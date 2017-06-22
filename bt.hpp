#include <sqlite3.h> 
#include <iostream>
#include <string>
#include <vector>
#include <memory> 
#include <unordered_map>
#include <utility>
#include <algorithm>

//class BacktestEngine
//{
//	SQLiteDB db;
//	DataFeedHandler data_feed_handler;
//	std::vector<std::string> symbol_vec;
//	BacktestEngine(const SQLiteDB& db, , const std::vector<std::string>& vec)
//		: db(db)
//		{}
//};

class Tick
{
public:
	std::string symbol;
	std::string dt; 
	double price; 
	Tick(): symbol("NULL"), dt("NULL"), price(0.0) {}
	Tick(std::string symbol, std::string datetime, double price): symbol(symbol), dt(datetime), price(price)
	{
	}
};



class SQLiteDB
{
private:
	std::string filepath;
public:
	sqlite3 *db;
	SQLiteDB() {}
	SQLiteDB(const std::string& filepath);
	~SQLiteDB() { sqlite3_close(db);}
	void create_table(const std::string& table_info);
	void insert(const std::string& sql);
	void delete_table(const std::string& table_name);
	void select(const std::string& sql);

};

class SQLiteQuery
{
	public :
	SQLiteDB db;
	sqlite3_stmt* res;
	int query_signal; 
	SQLiteQuery() {}
	SQLiteQuery(const SQLiteDB& db) : db(db), res(NULL), query_signal(0){ }
	~SQLiteQuery() {sqlite3_finalize(res);}
	void query(const std::string& sql);
	bool query_check() { if(query_signal == SQLITE_ROW){return true;} else{return false;}}
	auto double_step(int col);
	auto step(int col);
	auto tick_step(Tick& tick, const std::string& symbol);
};

class DataFeed
{

public:
	SQLiteDB db;
	SQLiteQuery query_obj;
	Tick tick;
	std::string symbol;
	DataFeed(const SQLiteDB& db, const std::string& symbol) : db(db), query_obj(db), symbol(symbol) { }
	void query();
	int step();
	bool isEmpty();
	Tick getTick();
	Tick* getTickPtr();
};


class Position
{
private:
	Tick fill_tick;
	Tick close_tick;
	DataFeed* data_feed;
	bool is_open;
public:
	Tick* current_tick;
	std::string symbol;
	int buy_sell;
	int quantity;
	Position(const std::string& symbol, int buy_sell, int quantity, DataFeed* data_feed);
	void closePosition() { close_tick = *current_tick; is_open = false; }
	bool isOpen() { return is_open; }
	double currentPL() { return ((buy_sell *(current_tick->price - fill_tick.price))* quantity); }

};


class PositionNode
{
public:
	std::shared_ptr<Position> pos;
	PositionNode* prev;
	PositionNode* next;

	PositionNode(std::shared_ptr<Position> pos, PositionNode* prev, PositionNode* next) :
		pos(pos),
		prev(prev),
		next(next) {}
};

class PositionList
{
public:
	PositionNode* head;
	PositionNode* tail;

	PositionList() : head(nullptr), tail(nullptr) {}
	void insertToTail(std::shared_ptr<Position> pos);
	auto removeAll(const std::string& symbol);
	auto loopPL();
	~PositionList();

};

class DataFeedHandler
{
private:
	std::unordered_map<std::string, DataFeed*> map_handler;
public:
	SQLiteDB db;
	std::vector<std::string> symbol_vec;
	DataFeedHandler(const SQLiteDB& db, std::vector<std::string> symbol_vec);
	DataFeed* getFeed(const std::string& symbol);
	~DataFeedHandler();
	void step();
	void query();
};



class PositionHandler
{
public:
	PositionList position_list;
	PositionHandler() {};
};

class OrderHandler
{
public:
	DataFeedHandler dfh;

	OrderHandler(const DataFeedHandler& dfh) : dfh(dfh) {}
	~OrderHandler() { std::cout << "del del" << std::endl; }
	std::shared_ptr<Position> newPostion(const std::string& symbol, int buy_sell, int quantity);
};

class Portfolio: public PositionHandler, public OrderHandler 
{
private:
	double capital; 
	std::vector<double> daily_pl; 
public:
	Portfolio(const DataFeedHandler& dfh, double capital) : OrderHandler(dfh), capital(capital) {}
	auto addPosition(const std::string& symbol, int buy_sell, int quantity);
	
};
