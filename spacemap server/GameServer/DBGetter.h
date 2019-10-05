#ifndef DATABASEGETTER_H
#define DATABASEGETTER_H


#include <iostream>
#include <utility>
#include <string>
//rethink about mutex, pro: - it prevents any memory leak con: - it could easily be done externally (mutex only used on execute() so it doesnt provide full protection)
#include <mutex>

#include <boost\lexical_cast.hpp>

extern "C" {
#include <mysql.h>
}

#include "../common/Defines.h"

class mysql_exception : public std::exception
{
public:
	explicit mysql_exception(const char* what) : std::exception(what) { }
};

extern "C" {

	/* By Default, DBGetter is not (totally) thread safe! Implement externally.
		Note: after adding mysql_free_result before creating the (next) result due to it creating a memory leak, I noticed that after around 1000 getString()s 
		memory rises from about 200 KB to around 500 KB in weird ticks
	*/
	class CDBGetter {
		/* Database related stuff */
		mutable std::mutex queryMutex;
		MYSQL_RES* result;
		MYSQL_ROW row;

		/* User Information */
		id_t  id;
		std::string id_str; //"clanID","id", etc.;a clear representation of the primary key name
		std::string column;
		std::string from;
		std::string database;

	public:
		CDBGetter(void) = delete;
		CDBGetter(const char* database, const char* column, const char* table, const char* id_str, id_t id) : result(nullptr),row(nullptr) {
			this->database = database;
			this->id = id;
			this->id_str = id_str;
			this->column = column;
			this->from = table;
		}
		~CDBGetter(void)
		{
			free();
		}
		CDBGetter(CDBGetter&& dbmoved) = default;
		CDBGetter(const CDBGetter& dbcopy) = default;

		void free();
		void changeQuery(const char* column, const char* table, const char* id_str, id_t id);
		void changeColumn(const char* column);
		void changeTable(const char* table);
		void changeIdString(const char* id_str);
		void changeId(id_t id);

		// DO NOT USE WITH USER INPUT DATA
		void setQueryRaw(const char* statement);
		//returns an integer from the query
		int					getInt(void);
		int*					getInts(void);
		//returns a 64-bit integer from the query
		long long			getInt64(void);
		long long*			getInt64s(void);
		//returns an unsigned integer from the query
		unsigned int		getUInt(void);
		unsigned int*		getUInts(void);
		//returns an unsigned 64-bit integer from the query
		unsigned long long	getUInt64(void);
		unsigned long long*	getUInt64s(void);
		//returns a double from the query
		double				getDouble(void);
		double*				getDoubles(void);
		//returns a boolean from the query
		bool				getBoolean(void);
		bool*				getBooleans(void);
		//returns the C-String
		const char*				getString(void);
		const char**				getStrings(void);


		/* sets the driver and connects to it, it also sets the scheme to the database*/
		bool init(void);
		bool connect(void);
	private:
		/*creates and executes query then iterates to the first row and returns its function (sql::ResultSet::first())
		  scheme: SELECT x FROM y WHERE id = z ORDER BY id ASC
		 */
		void print_db_err(const char*);
		bool execute(bool canReconnect = true);
	};
}
#endif