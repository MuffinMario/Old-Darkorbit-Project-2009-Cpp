#include "DBUpdater.h"
#include <inttypes.h>

extern std::mutex database_mutex;
extern CDBGetter g_database_get;
extern "C" {
	extern MYSQL* g_pmysql_connection;
	
	//changes all the vars for query
	void CDBUpdater::changeQuery(char* column, char* table, char* id_str, id_t id, char* set_value) {
		this->column = column;
		this->from = table;
		this->id_str = id_str;
		this->id = id;
		this->set_value = set_value;
	}
	inline void CDBUpdater::changeColumn(char* column) { this->column = column; }
	inline void CDBUpdater::changeTable(char* table) { this->from = table; }
	inline void CDBUpdater::changeIdString(char* id_str) { this->id_str = id_str; }
	inline void CDBUpdater::changeId(id_t id) { this->id = id; }
	inline void CDBUpdater::changeSetValue(std::string t) { strcpy_s(this->set_value, strlen(set_value), t.c_str()); }

	void CDBUpdater::queryUpdate(void)
	{
		if (!execute()) throw mysql_exception("MySQL Connection Error");
	}

	void CDBUpdater::queryUpdateRaw(std::string& str, bool canReconnect)
	{
		database_mutex.lock();
		/* 1. */
		if (g_pmysql_connection == NULL)
		{
			database_mutex.unlock();
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
				database_mutex.unlock();
				return;
			}
		}
		throw mysql_exception("MySQL Connection Error");
	}




	void CDBUpdater::print_db_err(char* stage)
	{
		std::cerr << "[DATABASE ERROR] " << stage << ": " << mysql_error(g_pmysql_connection) << std::endl;
	}

	bool CDBUpdater::execute(bool canReconnect) {
		//mutex not internal, dbutils : lock_guard

		const size_t BUFFER_SIZE = 512;//enough buffer size for a query
		char query_str[BUFFER_SIZE];
		sprintf_s(query_str, BUFFER_SIZE, "UPDATE %s SET %s = %s WHERE %s = %" PRIu64, from,column,set_value,id_str,id);
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
		if (mysql_query(g_pmysql_connection, query_str))
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