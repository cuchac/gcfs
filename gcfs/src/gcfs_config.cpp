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

	std::string sConfigPath;

	if(getConfigFile(sConfigPath))
	{
		ini.LoadFile(sConfigPath.c_str());

		m_sDataDir = ini.GetValue("Global", "dataPath", "");
		// Make sure path ends with slash
		if(*(m_sDataDir.rbegin()) != '/')
			m_sDataDir += "/";

		// Ensure directory exists
		//GCFS_Utils::rmdirRecursive(m_sDataDir.c_str()); // Too dangerous to delete whole dir
		GCFS_Utils::mkdirRecursive(m_sDataDir.c_str());

		CSimpleIniA::TNamesDepend vSections;
		ini.GetAllSections(vSections);

		CSimpleIniA::TNamesDepend::iterator it;
		for(it = vSections.begin(); it != vSections.end(); ++it)
		{
			const char * sDriver = ini.GetValue(it->pItem, "driver", "");

			if(!sDriver[0])
				continue;

			GCFS_Service* pNewService = AddService(sDriver, it->pItem);
			//pNewService->configure(&ini);
			
			printf("Adding service: %s, driver:%s\n", it->pItem, sDriver);
		}
		
		return true;
	}
	else
	{
		printf("No config file found! Creting default at: %s\n", sConfigPath.c_str());
		printf("To actualy use Gcfs, you have to modify the configuration file\nNow exiting.\n");
		
		// Config file does not exists
		GCFS_Utils::mkdirRecursive(sConfigPath.substr(sConfigPath.rfind('/')).c_str());

		int hFile = creat(sConfigPath.c_str(), S_IRUSR | S_IWUSR);
		write(hFile, GCFS_CONFIG_DEFAULTCONFIG, ARRAYSIZE(GCFS_CONFIG_DEFAULTCONFIG)-1);
		close(hFile);

		return false;
	}
}

bool GCFS_Config::getConfigFile(std::string &sPath)
{
	// First try user dir
	GCFS_Utils::getHomePath(sPath);

	if(access((sPath+GCFS_CONFIG_USERDIR+GCFS_CONFIG_FILENAME).c_str(), R_OK) == 0)
		return true;

	sPath = GCFS_CONFIG_SYSTEMDIR GCFS_CONFIG_FILENAME;
	if(access(sPath.c_str(), R_OK) == 0)
		return true;

	// Fill in user dir for use in creation of config file
	GCFS_Utils::getHomePath(sPath);
	sPath += GCFS_CONFIG_USERDIR GCFS_CONFIG_FILENAME;
	return false;
}


// Service management
GCFS_Service* GCFS_Config::AddService(const char * sDriver, const char * sName)
{
	GCFS_Service* tmpService = GCFS_Service::createService(sDriver, sName);
	m_vServices.push_back(tmpService);
	m_vServiceNames.push_back(sName);
	m_mNameToService[sName] = m_vServices.size()-1;
	
	return tmpService;
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