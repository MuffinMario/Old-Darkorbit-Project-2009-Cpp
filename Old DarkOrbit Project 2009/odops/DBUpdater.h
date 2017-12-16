#pragma once
#ifndef __DBUPDATER_H__
#define __DBUPDATER_H__
#include <iostream>
#include <utility>
#include <string>

extern "C" {
#include <mysql.h>
}

#include "Defines.h"
#include "DBGetter.h"
extern "C" {

	//for the sake of this being the first version, i will just make it execute queries it types in
	class CDBUpdater
	{

		/* User Information */
		id_t  id;
		char* id_str; //"clanID","id", etc.;a clear representation of the primary key name
		char* column;
		char* from;
		char* database;
		char* set_value;
	public:
		CDBUpdater(char* database, char* column, char* table, char* id_str, id_t id) {
			this->database = database;
			this->id = id;
			this->id_str = id_str;
			this->column = column;
			this->from = table;
			this->set_value = "'0'";
		}

		void changeQuery(char* column, char* table, char* id_str, id_t id, char* set_value);
		void changeColumn(char* column);
		void changeTable(char* table);
		void changeIdString(char* id_str);
		void changeId(id_t id);
		void changeSetValue(std::string t);

		/* Sets up a query in the scheme of 'UPDATE table SET row = value WHERE id = id_value' */
		bool queryUpdate();

		


	private:
		/*
		scheme: SELECT x FROM y WHERE id = z ORDER BY id ASC
		*/
		void print_db_err(char*);
		bool execute(void);
	};
}

#endif