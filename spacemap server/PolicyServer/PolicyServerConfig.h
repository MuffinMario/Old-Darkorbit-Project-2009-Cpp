#pragma once
#include <string>

#include <libconfig.h++>
#include "../common/ConfigNotAllValuesInitalizedException.h"
#include "../common/coutcolor.h"
struct CPolicyServerConfigData {
	unsigned short port;
};
/*
	This is a class that reads the PolicyServers's configuration file. The file is built like this

	#server information
	port = <number>;
*/
class CPolicyServerConfig {
	libconfig::Config m_config;
	std::string m_filename;
	CPolicyServerConfigData m_configdata;
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
		unsigned int port = 0;
		bool success = lookupValue("port", port, 843);

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
		const libconfig::Setting& root = m_config.getRoot();
		libconfig::Setting& setting = lookupOrAdd("port", libconfig::Setting::TypeInt);
		setting = static_cast<int>(m_configdata.port);
	}
	void writeFile(const char* filename) {
		m_config.writeFile(filename);
	}
	CPolicyServerConfigData* operator->() { return &m_configdata; }
};

