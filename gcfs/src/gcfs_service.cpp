#include "lib/simpleini/SimpleIni.h"
#include "gcfs_service.h"
#include "config.h"

#ifdef GCFS_MODULE_CONDOR
	#include "lib/module_condor/gcfs_servicecondor.h"
#endif
#ifdef GCFS_MODULE_SAGA
	#include "lib/module_saga/gcfs_servicesaga.h"
#endif

#include <string.h>
#include <stdio.h>

bool GCFS_Service::configure(CSimpleIniA& pConfig)
{
	// Do default config loading

	// Load defaul parameters
	CSimpleIniA::TKeyVal::const_iterator it;
	const CSimpleIniA::TKeyVal* pSection = pConfig.GetSection((m_sName+".default").c_str());
	if(pSection)
		for(it = pSection->begin(); it != pSection->end(); it++)
			m_mDefaults[it->first.pItem] = it->second;

	pSection = pConfig.GetSection((m_sName+".environment").c_str());
	if(pSection)
		for(it = pSection->begin(); it != pSection->end(); it++)
			m_mEnvironment[it->first.pItem] = it->second;

	return true;
}

bool GCFS_Service::customizeTask(GCFS_Task* pTask)
{
   const GCFS_FileSystem::FileList* mConfigValues = pTask->getConfigValues();
   GCFS_FileSystem::FileList::const_iterator itConfig;
	std::map<std::string, std::string >::iterator itDefault;
   for(itConfig = mConfigValues->begin(); itConfig != mConfigValues->end(); itConfig++)
   {
      if((itDefault = m_mDefaults.find(itConfig->first)) != m_mDefaults.end())
         itConfig->second->write(itDefault->second.c_str(), 0, itDefault->second.length());
   }

	for(itDefault = m_mEnvironment.begin(); itDefault != m_mEnvironment.end(); itDefault++)
		pTask->m_sConfigDirectory.m_sEnvironment.SetValue(itDefault->first.c_str(), itDefault->second.c_str());

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
