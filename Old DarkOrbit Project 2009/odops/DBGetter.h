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

#include "Defines.h"

extern "C" {

	/* By Default, DBGetter is not (totally) thread safe! Implement externally.
		Note: after adding mysql_free_result before creating the (next) result due to it creating a memory leak, I noticed that after around 1000 getString()s 
		memory rises from about 200 KB to around 500 KB in weird ticks
	*/
	class CDBGetter {
		/* Database related stuff */

		MYSQL_RES* result;
		MYSQL_ROW row;

		/* User Information */
		id_t  id;
		char* id_str; //"clanID","id", etc.;a clear representation of the primary key name
		char* column;
		char* from;
		char* database;

	public:
		CDBGetter(void) = delete;
		CDBGetter(char* database, char* column, char* table, char* id_str, id_t id) {
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
		void changeQuery(char* column, char* table, char* id_str, id_t id);
		void changeColumn(char* column);
		void changeTable(char* table);
		void changeIdString(char* id_str);
		void changeId(id_t id);

		//returns an integer from the query
		int					getInt(void);
		//returns a 64-bit integer from the query
		long long			getInt64(void);
		//returns an unsigned integer from the query
		unsigned int		getUInt(void);
		//returns an unsigned 64-bit integer from the query
		unsigned long long	getUInt64(void);
		//returns a double from the query
		double				getDouble(void);
		//returns a boolean from the query
		bool				getBoolean(void);
		//returns the C-String
		char*				getString(void);

		/* sets the driver and connects to it, it also sets the scheme to the database*/
		bool init(void);
		bool connect(void);
	private:
		/*creates and executes query then iterates to the first row and returns its function (sql::ResultSet::first())
		  scheme: SELECT x FROM y WHERE id = z ORDER BY id ASC
		 */
		void print_db_err(char*);
		bool execute(void);
	};
}
#endif