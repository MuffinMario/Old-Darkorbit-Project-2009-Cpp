#ifndef DATABASEGETTER_H
#define DATABASEGETTER_H


#include <iostream>
#include <utility>
#include <string>

#include <boost\lexical_cast.hpp>

#include <mysql_connection.h>
#include <mysql_error.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "Defines.h"

class DBGetter {
	static sql::mysql::MySQL_Driver*	driver;
	sql::Connection*					con;
	sql::Statement*						command;
	sql::ResultSet*						result;

	id_t			id;
	sql::SQLString	column;
	sql::SQLString	from;
	sql::SQLString	database;

public:
	DBGetter() = delete;
	DBGetter(sql::SQLString database, sql::SQLString column, sql::SQLString table, id_t id) {
		this->database	= database;
		this->id		= id;
		this->column	= column;
		this->from		= table;
	}
	~DBGetter()
	{
		delete con;
		delete command;
		delete result;
	}
	DBGetter(DBGetter&& dbmoved)	 = default;
	DBGetter(const DBGetter& dbcopy) = default;

	void changeQuery(sql::SQLString column, sql::SQLString table, id_t id);
	void changeColumn(sql::SQLString column);
	void changeTable(sql::SQLString table);
	void changeId(id_t id);

	//returns an integer from the query
	int					getInt();
	//returns a 64-bit integer from the query
	long long			getInt64();
	//returns an unsigned integer from the query
	unsigned int		getUInt();
	//returns an unsigned 64-bit integer from the query
	unsigned long long	getUInt64();
	//returns a double from the query
	double				getDouble();
	//returns a boolean from the query
	bool				getBoolean();
	//returns the SQLL-String
	sql::SQLString		getString();

	/* sets the driver and connects to it, it also sets the scheme to the database*/
	void connect();
private:
	/*creates and executes query then iterates to the first row and returns its function (sql::ResultSet::first())
	  scheme: SELECT x FROM y WHERE id = z ORDER BY id ASC
	 */
	bool execute();
};
#endif