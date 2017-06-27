#include <sqlite3.h> 
#include <iostream>
#include <string>
#include <vector>
#include <memory> 
#include <unordered_map>
#include <utility>
#include <algorithm>

//used as callback when SQLiteDb::select is called 
static int select_callback(void * params, int argc, char ** argv, char ** azColName);

//used as callback when SQLiteDB::select is not called but callback is needed
static int callback(void *params, int argc, char **argv, char **azColName);

//Tick is used to store tick from database 
//Tick can be of any timeframe ex. using tick data or using daily close data
class Tick
{
public:
	// stock symbol
	std::string symbol;
	// stores datetime as string
	std::string dt; 
	// price of tick
	double price; 
	// Default constructor
	Tick(): symbol("NULL"), dt("NULL"), price(0.0) {}
	Tick(std::string symbol, std::string datetime, double price): symbol(symbol), dt(datetime), price(price)
	{
	}
};


// Creates instance of SQLite3 Database
// Wrapper around SQLite3 C API
class SQLiteDB
{
private:
	//path to SQLite3 DB
	std::string filepath;
public:

	// from SQLite3 C API
	sqlite3 *db;
	// Default Constructor
	SQLiteDB() {}
	SQLiteDB(const std::string& filepath);
	//Closes connection to database using SQLite3 C API
	~SQLiteDB() { sqlite3_close(db);}
	//Creates table by taking name of table and columns rather than full query
	void create_table(const std::string& table_info);
	// Takes full SQLite3 query for inserting data
	void insert(const std::string& sql);
	// Takes name of table and deletes that table, no need to pass full query
	void delete_table(const std::string& table_name);
	// Takes full SQLite3 query for selecting data
	void select(const std::string& sql);

};

// This is a SQLite query object
// SQLiteQuery gives you more control over a given query 
class SQLiteQuery
{
	public :
	// database object
	SQLiteDB db;
	// From SQLite3 C API, used to store results from query
	sqlite3_stmt* res;
	// Signal for kmowing if any rows are left to step
	int query_signal; 
	//Default Constructor
	SQLiteQuery() {}

	SQLiteQuery(const SQLiteDB& db) : db(db), res(NULL), query_signal(0){ }
	// Finalize is must be called on res, from SQLit3 C API
	~SQLiteQuery() {sqlite3_finalize(res);}
	//Takes SQLite3 query and makes the call to db
	void query(const std::string& sql);
	// Lets us know if any rows are left to step to
	bool query_check() { if(query_signal == SQLITE_ROW){return true;} else{return false;}}
	// Steps to next row and returns given column as double
	auto double_step(int col);
	// Steps to next row and returns given column as string
	auto step(int col);
	// Steps to next row and sets members of tick to tick info of given symbol
	auto tick_step(Tick& tick, const std::string& symbol);
};

//DataFeed takes db and connects, then controls query for given symbol
//Each symbol should have its own DataFeed 
// DataFeed will only control one symbol
class DataFeed
{

public:
	// db that DataFeed is connected to
	SQLiteDB db;
	// Query object used to get data from datafeed
	SQLiteQuery query_obj;
	// Used to store newest tick from query 
	Tick tick;
	// symbol that DataFeed is connected to
	std::string symbol;
	// Constructor
	DataFeed(const SQLiteDB& db, const std::string& symbol) : db(db), query_obj(db), symbol(symbol) { }
	// Runs query from query object, it will just make query to get all info for given symbol
	// SELECT * FROM <symbol>;
	void query();
	// Makes query.tickStep()
	int step();
	// checks if any rows are left in query
	bool isEmpty();
	// returns copy of most current tick from query
	Tick getTick();
	// returns pointer to tick
	Tick* getTickPtr();
};

// Object to represent a single position in a stock/
class Position
{
private:
	// copy of tick that we were filled at 
	Tick fill_tick;
	//copy of tick that trade was closed at
	Tick close_tick;
	// pointer to datafeed of symbol to update p/l
	std::shared_ptr<DataFeed> data_feed;
	// tells if trade is still open
	bool is_open;
public:
	//pointer to tick of datafeed, gives current price
	Tick* current_tick;
	// symbol of underlying
	std::string symbol;
	// tells if we are buying or selling
	// 1 if buying
	// -1 if sell
	int buy_sell;
	// quantity
	int quantity;
	// Constructor
	Position(const std::string& symbol, int buy_sell, int quantity, std::shared_ptr<DataFeed> data_feed);
	//Sets close price and is_open memeber
	void closePosition() { close_tick = *current_tick; is_open = false; }
	// returns if trade is still open
	bool isOpen() { return is_open; }
	// returns current p/l of trade 
	double currentPL() { return ((buy_sell *(current_tick->price - fill_tick.price))* quantity); }

};

// Used as node in Position List container
class PositionNode
{
public:
	// pointer to position
	std::shared_ptr<Position> pos;
	// pointer to previous node
	PositionNode* prev;
	// pointer to next node
	PositionNode* next;
	// Defualt constructor
	PositionNode(std::shared_ptr<Position> pos, PositionNode* prev, PositionNode* next) :
		pos(pos),
		prev(prev),
		next(next) {}
};

//Container for holding positions in portfolio
class PositionList
{
public:
	// head of list   
	PositionNode* head;
	// tail of list
	PositionNode* tail;
	// Default Constructotr
	PositionList() : head(nullptr), tail(nullptr) {}
	// Insert new shared_ptr into tail
	void insertToTail(std::shared_ptr<Position> pos);
	//removes of all nodes where position is in the symbol given
	void removeAll(const std::string& symbol);
	//loop through list and returns p/l of all positions summed
	auto loopPL();
	// Destructor
	~PositionList();

};
// Handler class for all data feeds
class DataFeedHandler
{
private:
	// map used to store data feed, maps key of symbol to value of pointer to datafeeed for that symbol
	std::unordered_map<std::string, std::shared_ptr<DataFeed>> map_handler;
public:
	// db where sybols have table
	SQLiteDB db;
	// vector of symbols 
	std::vector<std::string> symbol_vec;
	// Constructor
	DataFeedHandler(const SQLiteDB& db, std::vector<std::string> symbol_vec);
	// Returns shared_ptr of datafeed for given symbol
	std::shared_ptr<DataFeed> getFeed(const std::string& symbol);
	// makes tick step for all data feeds in map
	void step();
	// makes query for all data feed in map
	void query();
	// if any datafeed in map returns True for there is empty call this isEmpty will be True
	bool isEmpty();
};


// Handler class for postions in portfolio
class PositionHandler
{
public:
	// Position list to hold positions
	PositionList position_list;
	// Default Constructor
	PositionHandler() {};
};

// Handler Class for orders
// Main job is to create new positos
class OrderHandler
{
public:
	// DataFeed handler for OrderHandler
	DataFeedHandler* dfh;
	// Constructor
	OrderHandler(DataFeedHandler* dfh) : dfh(dfh) {}
	//Creates new position and returns pointer to this new position
	std::shared_ptr<Position> newPostion(const std::string& symbol, int buy_sell, int quantity);
};
// Class to manage trades and keep track of pl
class Portfolio: public PositionHandler, public OrderHandler 
{
private:
	// Starting capital
	double capital; 
	// vector that stores cummulative pl for every step
	std::vector<double> pl; 
public:
	// Consturctor
	Portfolio( DataFeedHandler* dfh, double capital) : OrderHandler(dfh), capital(capital) {}
	// Add position to portfolio
	auto addPosition(const std::string& symbol, int buy_sell, int quantity);
	// Close all positions with given symbol
	auto closeBySymbol(const std::string& symbol){position_list.removeAll(symbol);}
	
};

// Abstract class to make strategy functor
class AbstractStrategy
{
public:
	// virtual operator() to create functor
	void virtual operator()(Portfolio* portfolio) = 0;
};
// Used to run backtest
class BacktestEngine
{

public: 
	// pointer to portfolio passed in constructor
	Portfolio* portfolio;
	// Constructor
	BacktestEngine(Portfolio* portfolio) : portfolio(portfolio) {}
	// Query for datafeed that porfolio is based on
	void query(){portfolio->dfh->query();}
	// Step for datafeed that portfolio is based on 
	void step(){portfolio->dfh->step();}
	// Runs backtest, takes strategy functor to run backtest
	void run(AbstractStrategy* fun);
};




class BuyStrategy : public AbstractStrategy
{
public:
	void operator()(Portfolio* portfolio);
};

