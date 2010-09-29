#ifndef GCFS_CONFIG_H
#define GCFS_CONFIG_H

#define GCFS_CONFIG_CONFIGDIR				"/.config/gcfs/"
#define GCFS_CONFIG_CONFIGFILE			(GCFS_CONFIG_CONFIGDIR "config.conf")
#define GCFS_CONFIG_DATADIR				"/.local/gcfs/data/"
#define GCFS_CONFIG_DEFAULTCONFIG		\
"# This is configuration file for Gcfs - Grid Control File System\n\
# Comments start with symbol '#'\n\
# Uncoment and modify values\n\
\n\
#### Global settings #####\n\
[Global]\n\
## Path where temporary files will be stored\n\
#dataPath = /home/joe/.local/gcfs/\n\
\n\
#### Services definitions ####\n\
## Every category represents one service named according to category name.\n\
#[Condor]\n\
## Driver defines the grid framework to use\n\
#driver = condor\n "

class GCFS_Service;

#include <string>
#include <vector>
#include <map>

class GCFS_Config
{

	// Configuration Loading
public:
	bool								loadConfig();

public:
	std::string						m_sDataDir;

private:
	bool								getHomePath(std::string &buffer);
	bool								mkdirRecursive(const char *sPath);


	// Services management
public:
	bool					AddService(const char* sDriver, const char* sName);

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

};

#endif // GCFS_CONFIG_H
