#include "bt.hpp"
#include <iostream>

static int callback(void *params, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      std::cout<< azColName[i] << argv[i] ? argv[i] : "NULL" ;
   }
   std::cout<< '\n';
   return 0;
}

static int select_callback(void * params, int argc, char ** argv, char ** azColName)
{
	for(int i = 0; i < argc; i++)
	{
		std::cout<< azColName[i] << " = " << (argv[i] ? argv[i]: "NULL" );
		std::cout << "| ";
	}

	std::cout << '\n';
	return 0;
}

SQLiteDB::SQLiteDB(const std::string& filepath) : filepath(filepath)
	{
		char *zErrMsg = 0;
		int rc; 
		const char * tmp_filepath = filepath.c_str();
		rc = sqlite3_open(tmp_filepath, &db);

		if(rc)
		{
			std::cout << "Cant open database: " << "\n";

			throw sqlite3_errmsg(db) ;

		}

		else
		{
			std::cout << "Opened database successfully" << '\n';
		}

	}

void SQLiteDB::create_table(const std::string& table_info)
{
		char *zErrMsg = 0;
		int rc; 
		std::string sql = "CREATE TABLE ";
		sql = sql + table_info + ";";
		const char * tmp_sql = sql.c_str();

		rc = sqlite3_exec(db, tmp_sql, callback, 0 ,&zErrMsg);

		if (rc != SQLITE_OK)
		{
			std::cout<< "Error creating table: " << zErrMsg << '\n';
			sqlite3_free(zErrMsg);
		}
		else
		{
			std::cout<< "Table created successfully \n";
		}
}

void SQLiteDB::insert(const std::string& sql)
{
		char *zErrMsg = 0;
		int rc; 
		const char * tmp_sql = sql.c_str();	

		rc = sqlite3_exec(db, tmp_sql, callback, 0, &zErrMsg);

		if(rc != SQLITE_OK)
		{
			std::cout << "Error: " << zErrMsg << '\n';
			sqlite3_free(zErrMsg);
		}

		else
		{
			std::cout << "Records created successfully \n";  
		}

}

void SQLiteDB::delete_table(const std::string& table_name)
{		
		std::string sql = "DROP TABLE ";
		sql = sql + table_name + ";"; 

		char *zErrMsg = 0;
		int rc; 
		const char * tmp_sql = sql.c_str();	

		rc = sqlite3_exec(db, tmp_sql, callback, 0, &zErrMsg);

		if(rc != SQLITE_OK)
		{
			std::cout << "Error: " << zErrMsg << '\n';
			sqlite3_free(zErrMsg);
		}

		else
		{
			std::cout << table_name << " deleted successfully \n";  
		}

}

void SQLiteDB::select(const std::string& sql)
{
	char * zErrMsg = 0; 
	int rc; 
	const char* tmp_sql = sql.c_str();

	rc = sqlite3_exec(db, tmp_sql, select_callback, 0, &zErrMsg);

	if(rc != SQLITE_OK)
	{
		std::cout << zErrMsg << '\n';
		sqlite3_free(zErrMsg);
	}
	else
	{
		std::cout << "Operation completed successfully \n" ;
	}

}

void SQLiteQuery::query(const std::string& sql)
{	
	int rc{0}; 
	auto c_sql = sql.c_str();
	sqlite3_finalize(res);
	rc = sqlite3_prepare_v2(db.db, c_sql, -1, &res, 0);
	if(rc != SQLITE_OK)
	{
		std::cout << sqlite3_errmsg(db.db) << '\n';

		return; 
	}
	query_signal = sqlite3_step(res);
	return;
}



auto SQLiteQuery::double_step(int col)
{
	double x; 
	 if (query_signal == SQLITE_ROW) {
         x = sqlite3_column_double(res, col);
         query_signal = sqlite3_step(res);
         return x;

   
    } 
    else
    {
    	std::cout << "NULL" << std::endl;
    	return 0.0; 
    }
}
auto SQLiteQuery::step(int col)
{
	std::string x; 
	 if (query_signal == SQLITE_ROW) {
         x = const_cast<char*>(reinterpret_cast<const char*>(sqlite3_column_text(res, col)));
         query_signal = sqlite3_step(res);
         return x;

   
    } 
    else
    {
    	x = "NULL";
    	return x;
  
    }
}

auto SQLiteQuery::tick_step(Tick& tick, const std::string& symbol)
{
	if(query_signal == SQLITE_ROW)
	{
	tick.symbol = symbol; 
	tick.price = sqlite3_column_double(res, 1);
	tick.dt = const_cast<char*>(reinterpret_cast<const char*>(sqlite3_column_text(res, 0)));
	query_signal = sqlite3_step(res);
	return 0;
	}
	else
	{
		std::cout << "NULL";
		return 1; 
	}
}

void DataFeed::query()
{
	std::string sql = " SELECT * FROM "; 
	sql = sql + symbol; 
	sql = sql + ";";
	query_obj.query(sql);
}
int DataFeed::step()
{
	return query_obj.tick_step(tick, symbol); 
}
bool DataFeed::isEmpty()
{
	return !query_obj.query_check();
}
Tick DataFeed::getTick()
{
	return tick;
}
Tick* DataFeed::getTickPtr()
{
	return &tick;
}

DataFeed* DataFeedHandler::getFeed(const std::string& symbol)
{
	return map_handler[symbol];
}
DataFeedHandler::DataFeedHandler(const SQLiteDB& db, std::vector<std::string> symbol_vec) : db(db), symbol_vec(symbol_vec) 
{
	for (auto symbol : symbol_vec)
	{
		map_handler.insert({ symbol, new DataFeed(db, symbol)});
	}
}

DataFeedHandler::~DataFeedHandler()
{
	//for (auto i : map_handler)
	//{
	//	std::cout << i.first << std::endl;
	//	std::cout << "del" << std::endl;
	//	delete i.second;

	//}
	std::cout << "deleteski" << std::endl; 
	delete map_handler["SPY"];
	map_handler["SPY"] = nullptr;
	std::cout << "deleter";
}

void DataFeedHandler::step()
{
	for (auto i : map_handler)
		i.second->step();
}
void DataFeedHandler::query()
{
	for (auto i : map_handler)
		i.second->query();
}
Position::Position(const std::string& symbol, int buy_sell, int quantity, DataFeed* data_feed) :
	symbol(symbol),
	buy_sell(buy_sell),
	quantity(quantity),
	data_feed(data_feed),
	is_open(true)
{
	fill_tick = data_feed->getTick();
	current_tick = data_feed->getTickPtr();
}
std::shared_ptr<Position> OrderHandler::newPostion(const std::string& symbol, int buy_sell, int quantity)
{
	return std::make_shared<Position>(symbol, buy_sell, quantity, (dfh.getFeed(symbol)));
}

void PositionList::insertToTail(std::shared_ptr<Position> pos)
{
	if (head == nullptr && tail == nullptr)
	{
		head = tail = new PositionNode(pos, nullptr, nullptr);
		return;
	}
	else
	{
		auto tmp = tail; 
		tail = new PositionNode(pos, tmp, nullptr);
		tmp->next = tail; 
		return;
	}
}
PositionList::~PositionList()
{
	if (head != nullptr && tail != nullptr)
	{
		auto tmp = head;
			while (tmp != nullptr)
			{
				std::cout << "delete" << std::endl;
				head = tmp;
				tmp = tmp->next;
				delete head; 

			}
	}

}
auto PositionList:: removeAll(const std::string& symbol)
{
	if (head == nullptr && tail == nullptr)
		return;


	auto tmp = head;
	auto delete_tmp = head; 
	while (tmp != nullptr)
	{
		if (tmp->pos->symbol == symbol)
		{
			if (tmp == head && tmp== tail)
			{
				head = nullptr; 
				tail = nullptr;

			}
			else
			{
				if (tmp == head)
				{
					head = tmp->next;
					head->prev = nullptr;
				}
 
				else if (tmp == tail)
				{
					tail = tmp->prev;
					tail->next = nullptr; 

				}
				else
				{
					tmp->prev->next = tmp->next;
					tmp->next->prev = tmp->prev;
				}
			}

			delete_tmp = tmp;
			tmp = tmp->next;
			delete delete_tmp;
		}
		else 
			tmp = tmp->next;
	}
	return;

}

auto PositionList::loopPL()
{
	if (head == nullptr)
		return 0.0;
	
	auto tmp = head; 
	auto sum = 0.0;
	while (tmp != nullptr)
	{
		sum += tmp->pos->currentPL();
		tmp = tmp->next;
	}
	return sum; 

}
auto Portfolio::addPosition(const std::string& symbol, int buy_sell, int quantity)
{
	position_list.insertToTail(newPostion(symbol, buy_sell, quantity));
}

//BacktestEngine::BacktestEngine(const SQLiteDB& db, const std::vector<std::string>& symbol_vec)
//	: db(db), 
//	symbol_vec(symbol_vec)
//{
//	for (auto i : symbol_vec)
//	{
//		data_feed_handler.addDataFeed(i, new )
//	}
//
//}
int main()
{

	std::string filename = "test.db";
	auto db = SQLiteDB(filename);
	DataFeed data(db, "SPY");
	std::vector<std::string> symbol_vec = { "SPY" };
	auto feed_handler = DataFeedHandler(db, symbol_vec);

	feed_handler.query(); 
	
	auto order_handler = OrderHandler(feed_handler);
	

	auto position_list = PositionList(); 
	feed_handler.step();
	position_list.insertToTail(order_handler.newPostion("SPY", 1, 100));
	position_list.insertToTail(order_handler.newPostion("SPY", 1, 100));
	feed_handler.step();
	std::cout << position_list.loopPL()<< std::endl;
	position_list.removeAll("SPY");
	data.query();
	while(!data.isEmpty())
	{
		 data.step();
		 std::cout << data.tick.symbol << " " << data. tick.price << " " << data.tick.dt << std::endl; 
	}
	
	std::cout << "out" << std::endl;

	return 0;
}