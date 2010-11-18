#ifndef GCFS_CONFIG_H
#define GCFS_CONFIG_H

#define GCFS_CONFIG_FILENAME				"gcfs.conf"
#define GCFS_CONFIG_SYSTEMDIR				"/etc/gcfs/"
#define GCFS_CONFIG_USERDIR				"/.config/gcfs/"
#define GCFS_CONFIG_USERDATADIR			"/.local/gcfs/data/"
#define GCFS_CONFIG_SYSTEMDATADIR		"/tmp/gcfs/"

#include "gcfs_config_default.h"

class GCFS_Service;

#include <string>
#include <vector>
#include <map>
#include "gcfs.h"

class GCFS_Config
{

	// Configuration Loading
public:
	bool								loadConfig();

public:
	std::string						m_sDataDir;
	std::string						m_sMountDir;
	std::string						m_sDefaultService;

	bool								getConfigFile(std::string &sPath);

	
	// Services management
public:
	GCFS_Service*		AddService(const char* sDriver, const char* sName);

	size_t				GetServiceCount();

	GCFS_Service*		GetService(int iIndex);
	GCFS_Service*		GetService(const char * sName);
	int					GetServiceIndex(const char * sName);
	
private:
	std::vector<GCFS_Service*> 		m_vServices;
	std::map<std::string, int>			m_mNameToService;
public:
	// Vector of all available services
	std::vector<std::string>			m_vServiceNames;

	// Permission of mounted filesystem
	GCFS_Permissions						m_sPermissions;

};

#endif // GCFS_CONFIG_H
