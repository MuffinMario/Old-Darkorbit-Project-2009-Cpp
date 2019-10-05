#include "DBUpdater.h"
#include <inttypes.h>

extern std::mutex database_mutex;
extern CDBGetter g_database_get;
extern "C" {
	extern MYSQL* g_pmysql_connection;
	
	//changes all the vars for query
	void CDBUpdater::changeQuery(const char* column, const char* table, const char* id_str, id_t id, const char* set_value) {
		this->column = column;
		this->from = table;
		this->id_str = id_str;
		this->id = id;
		this->set_value = set_value;
	}
	inline void CDBUpdater::changeColumn(const char* column) { this->column = column; }
	inline void CDBUpdater::changeTable(const char* table) { this->from = table; }
	inline void CDBUpdater::changeIdString(const char* id_str) { this->id_str = id_str; }
	inline void CDBUpdater::changeId(id_t id) { this->id = id; }
	inline void CDBUpdater::changeSetValue(std::string& t) { set_value = t; }

	void CDBUpdater::queryUpdate(void)
	{
		if (!execute()) throw mysql_exception("MySQL Connection Error");
	}

	void CDBUpdater::queryUpdateRaw(std::string& str, bool canReconnect)
	{
		std::unique_ptr<std::lock_guard<std::mutex>> lgptr(std::make_unique<std::lock_guard<std::mutex>>(database_mutex));
		/* 1. */
		if (g_pmysql_connection == NULL)
		{
			lgptr.reset();
			std::cerr << "[DATABASE ERROR] Cannot execute query when there's no connection." << std::endl;
			if (canReconnect)
			{

				std::cerr << "[@DB_CONNECT == NULL] Trying to reconnect..." << std::endl;
				if (g_database_get.connect())
					queryUpdateRaw(str,false);
				//else goes to end , throws exception -> disconnect
			}
		}
		else
		{
			/* 2. */
			if (mysql_query(g_pmysql_connection, str.c_str()))
			{
				database_mutex.unlock();
				print_db_err("@QUERY");
				mysql_close(g_pmysql_connection);
				g_pmysql_connection = NULL;
				if (canReconnect)
				{

					std::cerr << "[@DB_CONNECT == NULL] Trying to reconnect..." << std::endl;
					if (g_database_get.connect())
						queryUpdateRaw(str,false);

				}
			}
			else
			{
				return; // destroys unique ptr which destroys lock
			}
		}
		throw mysql_exception("MySQL Connection Error");
	}




	void CDBUpdater::print_db_err(const char* stage)
	{
		std::cerr << "[DATABASE ERROR] " << stage << ": " << mysql_error(g_pmysql_connection) << std::endl;
	}

	bool CDBUpdater::execute(bool canReconnect) {
		//mutex not internal, dbutils : lock_guard

		std::string query_str = "UPDATE " +from+" SET " +column+ " = " +set_value+ " WHERE " +id_str + " = " + to_string(id);
		/* 1. */
		if (g_pmysql_connection == NULL)
		{
			std::cerr << "[DATABASE ERROR] Cannot execute query when there's no connection." << std::endl;

			if (canReconnect)
			{

				std::cerr << "[@DB_CONNECT == NULL] Trying to reconnect..." << std::endl;
				if (!g_database_get.connect())
					return false;
				else
					return execute(false);

			}
		}
		/* 2. */
		if (mysql_query(g_pmysql_connection, query_str.c_str()))
		{
			print_db_err("@QUERY");
			mysql_close(g_pmysql_connection);
			g_pmysql_connection = NULL;
			if (canReconnect)
			{

				std::cerr << "[@QUERY] Trying to reconnect..." << std::endl;
				if (!g_database_get.connect())
					return false;
				else
					return execute(false);

			}
		}

		return true;
	}
}