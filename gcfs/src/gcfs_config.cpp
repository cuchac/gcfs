#include "lib/simpleini/SimpleIni.h"
#include <sys/stat.h>

#include "gcfs_config.h"
#include "gcfs_service.h"
#include "gcfs_utils.h"
#include <fcntl.h>


bool GCFS_Config::loadConfig()
{
	CSimpleIniA ini;
	ini.SetUnicode();

	std::string sHomePath;
	GCFS_Utils::getHomePath(sHomePath);

	if(access((sHomePath+GCFS_CONFIG_CONFIGFILE).c_str(), R_OK) == 0)
	{
		ini.LoadFile((sHomePath+GCFS_CONFIG_CONFIGFILE).c_str());

		m_sDataDir = ini.GetValue("Global", "dataPath", (sHomePath+GCFS_CONFIG_DATADIR).c_str());
		m_sDataDir += "/";

		// Ensure directory exists
		GCFS_Utils::rmdirRecursive(m_sDataDir.c_str());
		GCFS_Utils::mkdirRecursive(m_sDataDir.c_str());
		

		CSimpleIniA::TNamesDepend vSections;
		ini.GetAllSections(vSections);

		CSimpleIniA::TNamesDepend::iterator it;
		for(it = vSections.begin(); it != vSections.end(); ++it)
		{
			if(it->pItem == "Global")
				continue;

			const char * sDriver = ini.GetValue(it->pItem, "driver", "");

			if(!sDriver[0])
				continue;

			AddService(sDriver, it->pItem);
			
			printf("Adding service: %s, driver:%s\n", it->pItem, sDriver);

		}
	}else
	{
		printf("No config file found! Creting default at: %s\n", (sHomePath+GCFS_CONFIG_CONFIGDIR).c_str());
		printf("To actualy use Gcfs, you have to modify the configuration file\n");
		
		// Config file does not exists
		GCFS_Utils::mkdirRecursive((sHomePath+GCFS_CONFIG_CONFIGDIR).c_str());

		int hFile = creat((sHomePath+GCFS_CONFIG_CONFIGFILE).c_str(), S_IRUSR | S_IWUSR);
		write(hFile, GCFS_CONFIG_DEFAULTCONFIG, sizeof(GCFS_CONFIG_DEFAULTCONFIG)-1);
		close(hFile);
	}
}

// Service management
bool GCFS_Config::AddService(const char * sDriver, const char * sName)
{
	GCFS_Service * tmpService = GCFS_Service::createService(sDriver, sName);
	m_vServices.push_back(tmpService);
	m_vServiceNames.push_back(sName);
	m_mNameToService[sName] = m_vServices.size()-1;
}

size_t GCFS_Config::GetServiceCount()
{
	return (int)m_vServices.size();
}

GCFS_Service* GCFS_Config::GetService(int iIndex)
{
	return m_vServices[iIndex];
}

GCFS_Service* GCFS_Config::GetService(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mNameToService.find(sName)) != m_mNameToService.end())
		return m_vServices[it->second];
	else
		return NULL;
}

int GCFS_Config::GetServiceIndex(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mNameToService.find(sName)) != m_mNameToService.end())
		return it->second;
	else
		return -1;
}