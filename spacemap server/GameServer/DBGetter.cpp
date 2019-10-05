#include "DBGetter.h"
#include <inttypes.h>
#include "GameServerConfig.h"

extern CGameServerConfig g_gameServerConfig;

/* side notes for me
execute("UPDATE `cuentas` SET `empresa` = '3' WHERE `cuentas`.`id` = 12;");
table: cuentas row: empresa  at: id 12

*/

extern "C" {
	//public MYSQL
	MYSQL* g_pmysql_connection;

	void CDBGetter::free()
	{
		if (g_pmysql_connection != NULL)
		{
			mysql_close(g_pmysql_connection);
		}
		if (result != NULL)
		{
			mysql_free_result(result);
		}
	}
	void CDBGetter::changeQuery(const char* column, const char* table, const char* id_str, id_t id) {
		this->column = column;
		this->from = table;
		this->id_str = id_str;
		this->id = id;
	}
	inline void CDBGetter::changeColumn(const char* column) { this->column = column; }
	inline void CDBGetter::changeTable(const char* table) { this->from = table; }
	inline void CDBGetter::changeIdString(const char* id_str) { 
		this->id_str = id_str; }
	inline void CDBGetter::changeId(id_t id) { this->id = id; }

	[[deprecated]] void CDBGetter::setQueryRaw(const char * statement)
	{
		execute();
	}

	int CDBGetter::getInt(void) {
		return boost::lexical_cast<int>(getString()); //I wish to know why template functions work in a C linked code block, well technically its still the same as non template after compilation
	}
	long long CDBGetter::getInt64(void) {
		return boost::lexical_cast<long long>(getString());
	}

	unsigned int CDBGetter::getUInt(void) {
		return boost::lexical_cast<unsigned int>(getString());
	}
	unsigned long long CDBGetter::getUInt64(void) {
		return boost::lexical_cast<unsigned long long>(getString());
	}
	double CDBGetter::getDouble(void) {
		return std::stod(getString()); /* boost::lexical_cast<double>(getString()) -> internal compiler error for very very unknown reason, it'd not compile with <float> or <double> for really no reason it just says "internal compiler error", this really is sad becuase stod takes longer and who wants to wait 0.3 ms longer?*/
	}
	bool CDBGetter::getBoolean(void) {
		return boost::lexical_cast<bool>(getString());
	}
	//base
	const char* CDBGetter::getString(void) {
		if (execute() == false)
		{
			throw mysql_exception("MySQL Connection Error");
		}
		row = mysql_fetch_row(result);
		return (row != nullptr? static_cast<const char*>(row[0]) : "<NULL-ITEM>"); // DBGetter is there to return values of only 1 FIELD, so the first ever result row item is returned, otherwise error
	}


	void CDBGetter::print_db_err(const char* stage)
	{
		std::cerr << "[DATABASE ERROR] " << stage << ": " << mysql_error(g_pmysql_connection) << std::endl;
	}

	bool CDBGetter::execute(bool canReconnect) {
		std::string query_str = "SELECT " + column + " FROM " + from + " WHERE " + id_str + " = " + to_string(id) + " ORDER BY " + id_str + " ASC";
		/* 1. */
		if (g_pmysql_connection == NULL)
		{
			std::cerr << "[DATABASE ERROR] Cannot execute query when there's no connection." << std::endl;

			if (canReconnect)
			{

				std::cerr << "[@DB_CONNECT == NULL] Trying to reconnect..." << std::endl;
				if (!connect())
					return false;
				else
					return execute(false);

			}
		}
		//prevent memory leak on multiple accesses
		/* 2. */
		{
			std::unique_ptr<std::lock_guard<std::mutex>> lgptr(std::make_unique<std::lock_guard<std::mutex>>(queryMutex));
			if (mysql_query(g_pmysql_connection, query_str.c_str()))
			{
				//delete lgptr.release();
				lgptr.reset(nullptr); // unlock on success.
				print_db_err("@QUERY");
				mysql_close(g_pmysql_connection);
				g_pmysql_connection = NULL;
				if (canReconnect)
				{

					std::cerr << "[@QUERY] Trying to reconnect..." << std::endl;
					if (!connect())
						return false;
					else
						return execute(false);

				}
			}
			if (result != NULL) { mysql_free_result(result); result = NULL; }
			result = mysql_store_result(g_pmysql_connection);

		}

		if (result == NULL)
		{
			print_db_err("@RESULT");
			mysql_free_result(result); //?
			mysql_close(g_pmysql_connection);
			g_pmysql_connection = NULL;
			result = NULL;
			if (canReconnect)
			{

				std::cerr << "[@RESULT] Trying to reconnect..." << std::endl;
				if (!connect())
					return false;
				else
					return execute(false);
			}
		}
		return true;
	}
	bool CDBGetter::init(void)
	{
		g_pmysql_connection = mysql_init(NULL);
		return g_pmysql_connection != NULL;
	}
	bool CDBGetter::connect(void) {
		if (g_pmysql_connection == NULL)
		{
			if (init() == false)
			{
				print_db_err("@INIT");
				return false;
			}
		}
		if (mysql_real_connect(g_pmysql_connection, g_gameServerConfig->mysql_ip.c_str(), g_gameServerConfig->mysql_user.c_str(), g_gameServerConfig->mysql_pass.c_str(), g_gameServerConfig->mysql_database_name.c_str(), g_gameServerConfig->mysql_port, NULL, 0) == NULL)
		{
			print_db_err("@CONNECT");
			mysql_close(g_pmysql_connection);
			g_pmysql_connection = NULL;
			return false;
		}
		std::cout << "Connection to database successfull!" << std::endl;
		return true;
	}
}