#ifndef GCFS_CONFIG_H
#define GCFS_CONFIG_H

#include <string>
#include <vector>

class GCFS_Config
{
	// Service
	class Service
	{
	public:
										Service(const char* sName):m_sName(sName){};
								
	public:
		std::string					m_sName;
	};

public:
	std::vector<Service> 		m_vServices;
	std::vector<std::string>	m_vServiceNames;

};

#endif // GCFS_CONFIG_H
