#include <sqlite3.h> 
#include <iostream>
#include <string>
#include <memory> 

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
	DataFeed(const SQLiteDB& db, const std::string& symbol) : db(db), symbol(symbol) {query_obj.db = db;}
	void query();
	int next();
	bool isEmpty();
};
