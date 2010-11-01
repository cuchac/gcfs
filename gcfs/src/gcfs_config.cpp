#include "lib/simpleini/SimpleIni.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <ftw.h>

#include "gcfs_config.h"
#include "gcfs_service.h"

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

	if(access((sHomePath+GCFS_CONFIG_CONFIGFILE).c_str(), R_OK) == 0)
	{
		ini.LoadFile((sHomePath+GCFS_CONFIG_CONFIGFILE).c_str());

		m_sDataDir = ini.GetValue("Global", "dataPath", (sHomePath+GCFS_CONFIG_DATADIR).c_str());
		m_sDataDir += "/";

		// Ensure directory exists
		rmdirRecursive(m_sDataDir.c_str());
		mkdirRecursive(m_sDataDir.c_str());
		

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
		mkdirRecursive((sHomePath+GCFS_CONFIG_CONFIGDIR).c_str());

		int hFile = creat((sHomePath+GCFS_CONFIG_CONFIGFILE).c_str(), S_IRUSR | S_IWUSR);
		write(hFile, GCFS_CONFIG_DEFAULTCONFIG, sizeof(GCFS_CONFIG_DEFAULTCONFIG)-1);
		close(hFile);
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

bool GCFS_Config::mkdirRecursive(const char *path)
{
        char opath[1024];
        char *p;
        size_t len;

        strncpy(opath, path, sizeof(opath));
        len = strlen(opath);
        if(opath[len - 1] == '/')
                opath[len - 1] = '\0';
        for(p = opath; *p; p++)
                if(*p == '/') {
                        *p = '\0';
                        if(access(opath, F_OK))
                                mkdir(opath, S_IRWXU | S_IRWXG | S_IRWXO);
                        *p = '/';
                }
        if(access(opath, F_OK))         /* if path is not terminated with / */
                mkdir(opath, S_IRWXU | S_IRWXG | S_IRWXO);
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{

	// Skip roo dir itself
	if(ftwbuf->base == 0)
		return 0;
	
	int rv = remove(fpath);

	if (rv)
		perror(fpath);

	return rv;
}

bool GCFS_Config::rmdirRecursive(const char *sPath)
{
	// Dummy failsafe
	if(strlen(sPath) < 5)
	{
		printf("Tried to delete too short directory '%s'. Is this right?!?!?!\n", sPath);
		return false;
	}
	
	return nftw(sPath, unlink_cb, 64, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) != 0;
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