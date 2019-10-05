#pragma once
#ifndef __DBUPDATER_H__
#define __DBUPDATER_H__
#include <iostream>
#include <utility>
#include <string>

extern "C" {
#include <mysql.h>
}

#include "../common/Defines.h"
#include "DBGetter.h"
extern "C" {

	//for the sake of this being the first version, i will just make it execute queries it types in
	class CDBUpdater
	{

		/* User Information */
		id_t  id;
		std::string id_str; //"clanID","id", etc.;a clear representation of the primary key name
		std::string column;
		std::string from;
		std::string database;
		std::string set_value;
	public:
		CDBUpdater(const char* database, const char* column, const char* table, const char* id_str, id_t id) {
			this->database = database;
			this->id = id;
			this->id_str = id_str;
			this->column = column;
			this->from = table;
			this->set_value = "'0'";
		}

		void changeQuery(const char* column, const char* table, const char* id_str, id_t id, const char* set_value);
		void changeColumn(const char* column);
		void changeTable(const char* table);
		void changeIdString(const char* id_str);
		void changeId(id_t id);
		void changeSetValue(std::string& t);

		/* Sets up a query in the scheme of 'UPDATE table SET row = value WHERE id = id_value' */
		void queryUpdate(void);
		/* Use at your own risk. no extras are going to save you from syntax errors*/
		void queryUpdateRaw(std::string& str,bool canReconnect = true);

		


	private:
		/*
		scheme: SELECT x FROM y WHERE id = z ORDER BY id ASC
		*/
		void print_db_err(const char*);
		bool execute(bool canReconnect = true);
	};
}

#endif