#pragma once
#include <string>


#include <libconfig.h++>
#include "../common/ConfigNotAllValuesInitalizedException.h"
#include "../common/coutcolor.h"
struct CGameServerConfigData {
	std::string mysql_ip;
	std::string mysql_user;
	std::string mysql_pass;
	unsigned short mysql_port = 0;

	std::string mysql_database_name;

	unsigned short port = 0;
	unsigned int tickrate = 0;


};
/*
	This is a class that reads the GameServer's configuration file. The file is built like this

	#server information
	port = <number>;

	#sets the amount of ticks per second
	tickrate = <number>;

	#mysql information
	mysql = {
		mysql_ip = "xxx.xxx.xxx.xxx";
		mysql_user = "xxxxx";
		mysql_pass = "xxxxx";
		mysql_port = <number>;
	}
*/
class CGameServerConfig {
	libconfig::Config m_config;
	std::string m_filename;
	CGameServerConfigData m_configdata;
	/*
		Looksup value and in case it doesnt exist/has the wrong type, defaultval will be the value copied to val. t has to be a type assignable to T
	*/
	template<typename T, typename t>
	bool lookupValue(const char* path, T& val, t defaultval) {
		if (!m_config.lookupValue(path, val))
		{
			val = defaultval;
			return false;
		}
		return true;
	}
	libconfig::Setting& lookupOrAdd(const char* path, const libconfig::Setting::Type type)
	{
		if (m_config.exists(path))
			return m_config.lookup(path);
		else 
			return m_config.getRoot().add(path, type);
	}
	libconfig::Setting& lookupOrAdd(libconfig::Setting& setting, const char* path, const libconfig::Setting::Type type)
	{
		if (setting.exists(path))
			return setting.lookup(path);
		else 
			return setting.add(path, type);
	}
public:
	void loadFile(const char* filename) {
		m_config.readFile(filename);
	}
	void loadFile(const std::string& filename)
	{
		loadFile(filename.c_str());
	}
	void lookupData() {
		//placeholder for unsigned short
		unsigned int mport = 0, port = 0;
		bool success = lookupValue("mysql.mysql_ip", m_configdata.mysql_ip, "localhost");
		success &= lookupValue("mysql.mysql_user", m_configdata.mysql_user, "root");
		success &= lookupValue("mysql.mysql_pass", m_configdata.mysql_pass, "");
		success &= lookupValue("mysql.mysql_database_name", m_configdata.mysql_database_name, "olddorps");
		success &= lookupValue("mysql.mysql_port", mport, 3306U);
		success &= lookupValue("port", port, 5152U);
		success &= lookupValue("tickrate", m_configdata.tickrate, 30U);

		m_configdata.mysql_port = static_cast<unsigned short>(mport);
		m_configdata.port = static_cast<unsigned short>(port);

		if (!success)
		{
			throw CConfigNotAllValuesInitalizedException("Some values were nonexistant");
		}
	}
	/*
		Sets the config data to the configuration file. Doesn't save!
	*/
	void setData() {
		auto& root = m_config.getRoot();
		lookupOrAdd("port", libconfig::Setting::TypeInt) = static_cast<int>(m_configdata.port);
		lookupOrAdd("tickrate", libconfig::Setting::TypeInt) = static_cast<int>(m_configdata.tickrate);

		auto& mysql = lookupOrAdd("mysql", libconfig::Setting::TypeGroup);
		lookupOrAdd(mysql, "mysql_ip", libconfig::Setting::TypeString) = m_configdata.mysql_ip;
		lookupOrAdd(mysql, "mysql_user", libconfig::Setting::TypeString) = m_configdata.mysql_user;
		lookupOrAdd(mysql, "mysql_pass", libconfig::Setting::TypeString) = m_configdata.mysql_pass;
		lookupOrAdd(mysql, "mysql_database_name", libconfig::Setting::TypeString) = m_configdata.mysql_database_name;
		lookupOrAdd(mysql, "mysql_port", libconfig::Setting::TypeInt) = static_cast<int>(m_configdata.mysql_port);
	}
	void writeFile(const char* filename){
		m_config.writeFile(filename);
	}
	CGameServerConfigData* operator->() { return &m_configdata; }
};

