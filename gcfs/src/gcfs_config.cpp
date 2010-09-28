#include "gcfs_config.h"
#include "gcfs_service.h"

#include "lib/simpleini/SimpleIni.h"

#ifdef _WIN32
	//#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
#endif


bool GCFS_Config::loadConfig()
{
	CSimpleIniA ini;
	ini.SetUnicode();

	std::string sHomePath;
	getHomePath(sHomePath);
	
	ini.LoadFile((sHomePath+"/.config/gcfs/config.conf").c_str());

	m_sDataDir = ini.GetValue("Global", "dataPath", (sHomePath+"/.local/gcfs/data/").c_str());

	CSimpleIniA::TNamesDepend vSections;
	ini.GetAllSections(vSections);

	CSimpleIniA::TNamesDepend::iterator it;
	for(it = vSections.begin(); it != vSections.end(); ++it)
	{
		if(it->pItem == "Global")
			continue;

		
		
	}
}

bool GCFS_Config::getHomePath(std::string &buffer)
{
	// Get Home dir by platfomr-independent way
#ifdef _WIN32

	buffer = getenv("USERPROFILE");
	if(!buffer.length())
	{
		buffer = getenv("HOMEDRIVE");
		buffer += getenv("HOMEPATH");
	}
	
#else

	buffer = getenv("HOME");

	if(!buffer.length())
		buffer = getpwuid(getuid())->pw_dir;
	
#endif
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