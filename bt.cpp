#include "bt.hpp"
#include <iostream>
#include <memory> 
#include <cmath>

static int callback(void *params, int argc, char **argv, char **azColName) 
{
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
template<typename T>
void SQLiteDB::writeVector(const std::vector<T>& vec, const std::string& table_name)
{
    char *err_msg = 0;
    int rc;
    
    std::string sql{"DROP TABLE IF EXISTS "};

    sql = sql + table_name + ";";
    const char * tmp_sql = sql.c_str();	
    rc = sqlite3_exec(db, tmp_sql, 0, 0, &err_msg);

    sql = "CREATE TABLE " ;
    sql = sql + table_name + "(val REAL) ;";
    tmp_sql = sql.c_str();	
    rc = sqlite3_exec(db, tmp_sql, 0, 0, &err_msg);

    for(auto &i : vec)
    {
    	sql = "INSERT INTO ";
    	sql = sql + table_name + " VALUES(";
    	sql = sql + std::to_string(i); 
    	sql = sql +");";
    	tmp_sql = sql.c_str();
    	std::cout << tmp_sql << std::endl;	
    	rc = sqlite3_exec(db, tmp_sql, 0, 0, &err_msg);
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
auto SQLiteQuery::order_step(Order & order, const std::string& symbol)
{
	if(query_signal == SQLITE_ROW)
	{
	order.symbol = symbol; 
	auto quant_and_sign = sqlite3_column_double(res, 0);
	if (quant_and_sign == 0)
		order.buy_sell = 0;
	else if(quant_and_sign > 0)
		order.buy_sell = 1;
	else
		order.buy_sell = -1;
	order.quantity = abs(quant_and_sign); 
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
	sql = sql + " where Date > ";
	sql = sql + start_date + " and Date < "; 
	sql = sql + end_date + " ;";

	std::cout << sql << std::endl;
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

void OrderFeed::query()
{
	std::string sql = "SELECT "; 
	sql = sql + symbol + " FROM ";
	sql = sql + strategy_table; 
	//sql = sql + " where Date > ";
	//sql = sql + start_date + " and Date < "; 
	//sql = sql + end_date + " ;";
	sql = sql + " ;";
	query_obj.query(sql);	
}
int OrderFeed::step()
{
	return query_obj.order_step(order, symbol);
}
Order OrderFeed::getOrder()
{
	return order;
}
Order* OrderFeed::getOrderPtr()
{
	return &order;
}
bool OrderFeed::isHot()
{
	if(order.buy_sell == 1 || order.buy_sell == -1)
		return true;
	else 
		return false;
}
bool OrderFeed::isEmpty()
{
	return !query_obj.query_check();
}
std::shared_ptr<DataFeed> DataFeedHandler::getFeed(const std::string& symbol)
{
	return map_handler[symbol];
}
DataFeedHandler::DataFeedHandler(const SQLiteDB& db, std::vector<std::string> symbol_vec, const std::string& start_date, const std::string& end_date) : db(db), symbol_vec(symbol_vec) 
{
	for (auto symbol : symbol_vec)
	{
		map_handler.insert({ symbol, std::make_shared<DataFeed>(db, symbol, start_date, end_date)});
	}
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
bool DataFeedHandler::isEmpty()
{
	for(auto i: map_handler)
	{
		if(i.second->isEmpty())
			return true;
	}

	return false; 
}

std::shared_ptr<OrderFeed> OrderFeedHandler::getFeed(const std::string& symbol)
{
	return map_handler[symbol];
}
OrderFeedHandler::OrderFeedHandler(const SQLiteDB& db, const std::string& strategy_table, std::vector<std::string>& symbol_vec, const std::string& start_date, const std::string& end_date) : db(db), symbol_vec(symbol_vec) 
{
	for (auto symbol : symbol_vec)
	{
		map_handler.insert({ symbol, std::make_shared<OrderFeed>(db, strategy_table, symbol, start_date, end_date)});
	}
}
void OrderFeedHandler::step()
{
	for (auto i : map_handler)
		i.second->step();
}
void OrderFeedHandler::query()
{
	for (auto i : map_handler)
		i.second->query();
}
bool OrderFeedHandler::isEmpty()
{
	for(auto i: map_handler)
	{
		if(i.second->isEmpty())
			return true;
	}

	return false; 
}
Position::Position(const std::string& symbol, int buy_sell, int quantity, std::shared_ptr<DataFeed> data_feed) :
	symbol(symbol),
	buy_sell(buy_sell),
	quantity(quantity),
	data_feed(data_feed),
	is_open(true)
{
	fill_tick = data_feed->getTick();
	current_tick = data_feed->getTickPtr();
}
std::shared_ptr<Position> PositionHandler::newPostion(const std::string& symbol, int buy_sell, int quantity)
{
	return std::make_shared<Position>(symbol, buy_sell, quantity, (dfh->getFeed(symbol)));
}

void PositionList::insertToTail(std::shared_ptr<Position> pos)
{
	if (head == nullptr && tail == nullptr)
	{
		head =  new PositionNode(pos, nullptr, nullptr);
		tail = head;
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
				head = tmp;
				tmp = tmp->next;
				delete head; 

			}
	}

}
void PositionList::removeAll(const std::string& symbol)
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

auto Portfolio::addPosition(const Order& order)
{
	addPosition(order.symbol, order.buy_sell, order.quantity);
}
void BacktestEngine::run()
{
	query();
	order_feed_handler->query();
	while(!portfolio->dfh->isEmpty())
	{
	for (auto i : order_feed_handler->map_handler)
		{
			if(i.second->isHot())
			{
				std::cout << "buy" << std::endl; 
				portfolio->addPosition(i.second->getOrder());
			}
		}
	 portfolio->pl.push_back(portfolio->position_list.loopPL());
	 step();
	 order_feed_handler->step(); 
	}
}

void BuyStrategy::operator()(Portfolio* port)
{
	if ((port->dfh->getFeed("SPY")->getTick().price) > 200)
	{
		std::cout << (port->dfh->getFeed("SPY")->getTick().price)<< std::endl;
		std::cout << "buy" << std::endl; 
		port->addPosition("SPY", 1, 100);
	}
}
int main(int argc, char* argv [])
{
	std::vector<std::string> args(argv + 1, argv + argc);
	// filename for database
	std::string filename = "test.db"; 
	// create sqlite3 object
	auto db = SQLiteDB(filename); 
	// list of symbols that will be traded
	std::vector<std::string> symbol_vec = { "RDS_A", "RDS_B"}; 
	// create feed handler with symbol list 
	std::string start_date{"20070101"}; 
	std::string end_date{"20150101"};
	auto feed_handler = DataFeedHandler(db, args, start_date, end_date); 
	// create portfolio with feed and capital 
	auto portfolio = Portfolio(&feed_handler, 100000); 

	auto order_feed_handler = OrderFeedHandler(db, "strat", symbol_vec, start_date, end_date);
	// create backtest engine with portfolio 
	auto bt = BacktestEngine(&portfolio, &order_feed_handler); 
	// create strategy functor
	auto strategy = BuyStrategy();  
	// pass functor to BackTestEngine::run(Fun func) which runs strategy
	bt.run(); 
	db.writeVector(portfolio.pl, "pl");
	return 0;
}