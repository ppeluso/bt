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
int DataFeed::next()
{
	return query_obj.tick_step(tick, symbol); 
}
bool DataFeed::isEmpty()
{
	return !query_obj.query_check();
}
int main()
{
	std::string filename = "test.db";
	auto db = SQLiteDB(filename);

		//std::cout<< query.double_step(0) << std::endl; 
	DataFeed data(db, "SPY");
	data.query(); 
	while(!data.isEmpty())
	{
		 data.next();
		 std::cout << data.tick.symbol << " " << data. tick.price << " " << data.tick.dt << std::endl; 
	}


	return 0;
}