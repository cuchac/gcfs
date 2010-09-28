#ifndef GCFS_CONFIG_H
#define GCFS_CONFIG_H

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
