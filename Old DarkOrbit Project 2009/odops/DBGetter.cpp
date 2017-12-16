#include "DBGetter.h"
/* side notes for me
	execute("UPDATE `cuentas` SET `empresa` = '3' WHERE `cuentas`.`id` = 12;");
	table: cuentas row: empresa  at: id 12

*/

sql::mysql::MySQL_Driver* DBGetter::driver;


void DBGetter::changeQuery(sql::SQLString column, sql::SQLString table, id_t id) {
	this->column = column;
	this->from = table;
	this->id = id;
}
inline void DBGetter::changeColumn(sql::SQLString column) { this->column = column; }
inline void DBGetter::changeTable(sql::SQLString table) { this->from = table; }
inline void DBGetter::changeId(id_t id) { this->id = id; }

int DBGetter::getInt() {
	//creates and executes query and if first() is not returning false(error) returns the integer in the first found row
	return execute() ? result->getInt(column) : 0;
}
long long DBGetter::getInt64() {
	return execute() ? result->getInt64(column) : 0LL;
}

unsigned int DBGetter::getUInt() {
	return execute() ? result->getUInt(column) : 0U;
}
unsigned long long DBGetter::getUInt64() {
	return execute() ? result->getUInt64(column) : 0ULL;
}
double DBGetter::getDouble() {
	return execute() ? result->getDouble(column) : 0.0;
}
bool DBGetter::getBoolean() {
	return execute() ? result->getBoolean(column) : false;
}
sql::SQLString DBGetter::getString() {
	return execute() ? result->getString(column) : sql::SQLString();
}

bool DBGetter::execute() {
	try {

		if (con == nullptr) {
			std::cout << "Connecting to Database..." << std::endl;
			connect();
		}

		if (con == nullptr) {
			std::cout << "Could not connect to the database, program will be paused until input!" << std::endl;
			std::cin.get();
			return execute();
		}
		command = con->createStatement();
		result = command->executeQuery("SELECT " + column + " FROM " + from + " WHERE id = " + boost::lexical_cast<std::string>(id) + " ORDER BY id ASC ");
	}
	catch (sql::SQLException ex) {
		std::cerr << "Error executing query in " << __FUNCTION__ << std::endl << "'-> " << ex.what() << std::endl;
		return false;
	}

	if (!result->first()) {
		std::cerr << "error on reading resultSet->first() " << std::endl;
		return false;
	}
	return true;
}

void DBGetter::connect() {
	try {
		/* driver is a static objects because in case of multiple DBGetters
		the program will troubleshoot on the destructor since only one of the objects
		can get access to the database at once
		to prevent multiple connecting we will make the program check if there is
		something allocated already or not.
		!!!: if DB crashes and restarts, the driver will point to an unused instance
		*/
		if (driver == nullptr) {
			driver = sql::mysql::get_driver_instance();
		}
		con = driver->connect("tcp://127.0.0.1:3306", "root", "");
		if (con == nullptr) throw sql::SQLException("con is nullptr / failed to connect to the database");
		con->setSchema(database);
	}
	catch (sql::SQLException ex) {
		std::cerr << "Error connecting to the database in " << __FUNCTION__ << std::endl << "'-> " << ex.what() << std::endl;
	}
}

