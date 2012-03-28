#include "lib/simpleini/SimpleIni.h"
#include <sys/stat.h>

#include "gcfs_config.h"
#include "gcfs_service.h"
#include "gcfs_utils.h"
#include <fcntl.h>

GCFS_Config::GCFS_Config()
{
}

GCFS_Config::~GCFS_Config()
{
   for(std::vector<GCFS_Service*>::iterator it = m_vServices.begin(); it != m_vServices.end();it++)
      delete *it;
}

bool GCFS_Config::loadConfig()
{
   CSimpleIniA ini;
   ini.SetUnicode();

   std::string sConfigPath;

   if(getConfigFile(sConfigPath))
   {
      printf("Using config file: %s\n", sConfigPath.c_str());

      ini.LoadFile(sConfigPath.c_str());

      CSimpleIniA::TKeyVal::const_iterator it;
      const CSimpleIniA::TKeyVal* pSectionValues = ini.GetSection("Global");
      if(!pSectionValues)
      {
         printf("Configure error: Mandatory section 'Global' not found!\nExiting now.\n");
         return false;
      }

      m_sDataDir = ini.GetValue("Global", "data_path", "");
      if(m_sDataDir.empty())
      {
         printf("Configure error: Mandatory value 'data_path' in 'Global' section not found!\nExiting now.\n");
         return false;
      }

      // Make sure path ends with slash
      if(*(m_sDataDir.rbegin()) != '/')
         m_sDataDir += "/";

      // Ensure directory exists
      //GCFS_Utils::rmdirRecursive(m_sDataDir.c_str()); // Too dangerous to delete whole dir
      GCFS_Utils::mkdirRecursive(m_sDataDir.c_str());

      for(it = pSectionValues->begin(); it != pSectionValues->end(); ++it)
      {
         if(strcmp(it->first.pItem, "service") != 0)
            continue;

         const char * sDriver = ini.GetValue(it->second, "driver", "");

         if(!sDriver[0])
            continue;

         GCFS_Service* pNewService = AddService(sDriver, it->second);

         if(!pNewService || !pNewService->configure(ini))
            printf("Failed configuration of %s service!\n", it->second);

         printf("Adding service: %s, driver:%s\n", it->second, sDriver);
      }

      if(m_vServiceNames.size() <= 0)
      {
         printf("Configure error: No service defined!\nExiting now.\n");
         return false;
      }

      if((it = pSectionValues->find("default_service")) != pSectionValues->end())
         m_sDefaultService = it->second;
      else
         m_sDefaultService = m_vServiceNames[0];

      return true;
   }
   else
   {
      printf("No config file found! Creting default at: %s\n", sConfigPath.c_str());
      printf("To actualy use Gcfs, you have to modify the configuration file\nNow exiting.\n");

      // Config file does not exists
      GCFS_Utils::mkdirRecursive(sConfigPath.substr(0, sConfigPath.rfind('/')).c_str());

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

   sPath += GCFS_CONFIG_USERDIR GCFS_CONFIG_FILENAME;
   if(access(sPath.c_str(), R_OK) == 0)
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

   if(tmpService)
   {
      m_vServices.push_back(tmpService);
      m_vServiceNames.push_back(sName);
      m_mNameToService[sName] = m_vServices.size()-1;
   }

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
