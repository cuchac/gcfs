#include "lib/simpleini/SimpleIni.h"
#include "gcfs_service.h"
#include "config.h"

#ifdef GCFS_MODULE_CONDOR
   #include "lib/module_condor/gcfs_servicecondor.h"
#endif
#ifdef GCFS_MODULE_SAGA
   #include "lib/module_saga/gcfs_servicesaga.h"
#endif
#ifdef GCFS_MODULE_DIGEDAG
#include "lib/module_digedag/gcfs_servicedigedag.h"
#endif

#include <string.h>
#include <stdio.h>

GCFS_Service::GCFS_Service(const char* sName):
   m_sName(sName),
   m_sDefaultValues(NULL)
{
   
}
GCFS_Service::~GCFS_Service()
{
   
}

bool GCFS_Service::configure(CSimpleIniA& pConfig)
{
   // Do default config loading

   // Load defaul parameters
   CSimpleIniA::TKeyVal::const_iterator it;
   const CSimpleIniA::TKeyVal* pSection = pConfig.GetSection((m_sName+".default").c_str());
   if(pSection)
      for(it = pSection->begin(); it != pSection->end(); it++)
      {
         GCFS_ConfigValue* pConfigValue = m_sDefaultValues.getConfigValue(it->first.pItem);
         if(pConfigValue)
            pConfigValue->SetValue(it->second, 0);
      }

   pSection = pConfig.GetSection((m_sName+".environment").c_str());
   GCFS_ConfigValue* pConfigValue = m_sDefaultValues.getConfigValue("environment");
   if(pSection && pConfigValue)
   {
      std::string sValue;
      for(it = pSection->begin(); it != pSection->end(); it++)
         sValue += std::string(it->first.pItem) + "='" + it->second + "' ";
      pConfigValue->SetValue(sValue.c_str(), 0);
   }

   return true;
}

bool GCFS_Service::customizeTask(GCFS_Task* pTask)
{
   return true;
}

bool GCFS_Service::decustomizeTask(GCFS_Task* pTask)
{
   return true;
}

GCFS_Service*	GCFS_Service::createService(const char * sModule, const char * sName)
{

#ifdef GCFS_MODULE_CONDOR
   if(strcasecmp(sModule, "condor") == 0)
      return new GCFS_ServiceCondor(sName);
#endif

#ifdef GCFS_MODULE_SAGA
   if(strcasecmp(sModule, "saga") == 0)
      return new GCFS_ServiceSaga(sName);
#endif

#ifdef GCFS_MODULE_DIGEDAG
   if(strcasecmp(sModule, "digedag") == 0)
      return new GCFS_ServiceDigedag(sName);
#endif

   printf("Error! Unknown service driver: %s\n", sModule);
   return NULL;

}

bool GCFS_Service::submitTask(GCFS_Task* pTask)
{
   pTask->m_eStatus = GCFS_Task::eRunning;

   return true;
}

bool GCFS_Service::waitForTask(GCFS_Task* pTask)
{
   pTask->m_eStatus = GCFS_Task::eFinished;

   return true;
}

bool GCFS_Service::abortTask(GCFS_Task* pTask)
{
   pTask->m_eStatus = GCFS_Task::eAborted;

   return true;
}

bool GCFS_Service::deleteTask(GCFS_Task* pTask)
{
   return true;
}

GCFS_Task::Status	GCFS_Service::getTaskStatus(GCFS_Task* pTask)
{
   return pTask->m_eStatus;
}
