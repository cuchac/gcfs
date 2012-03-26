#include "lib/simpleini/SimpleIni.h"
#include "gcfs_servicesaga.h"
#include "gcfs_servicesaga_taskdata.h"

#include <gcfs_config.h>
#include <gcfs_utils.h>

#include <string.h>
#include <stdio.h>
#include <saga/saga.hpp>
#include <dirent.h>
#include <wordexp.h>

namespace sa  = saga::attributes;
namespace sja = saga::job::attributes;

GCFS_ServiceSaga::GCFS_ServiceSaga(const char* sName): GCFS_Service(sName)
{
}

GCFS_ServiceSaga::~GCFS_ServiceSaga()
{
}

bool GCFS_ServiceSaga::configure(CSimpleIniA& pConfig)
{
	CSimpleIniA::TKeyVal::const_iterator it;
	const CSimpleIniA::TKeyVal* pSection = pConfig.GetSection(m_sName.c_str());

	if((it = pSection->find("service_url"))==pSection->end())
	{
		printf("Configuration of '%s': Missing mandatory parameter: %s", m_sName.c_str(), "service_url");
		return false;
	}
	m_sServiceUrl = it->second;

	return GCFS_Service::configure(pConfig);
}

bool GCFS_ServiceSaga::submitTask(GCFS_Task* pTask)
{
	std::string sSubmitDir = getSubmitDir(pTask);
	mkdir(sSubmitDir.c_str(), 0777);
	
	// Allow world to write into submission dir - security suicide but necessary for non-root Condor
	chmod(sSubmitDir.c_str(), 0777);

   std::string sExecutableName = basename((char*)((std::string)pTask->m_sConfigDirectory.m_sExecutable).c_str());
   std::string sExecutable = sSubmitDir + sExecutableName.c_str();
   
	//Link executable into submit directory
	GCFS_File* pExecutableFile = pTask->getExecutableFile();
   
	link(pExecutableFile->getPath(), sExecutable.c_str());
	chmod(sExecutable.c_str(), 0777);

   pExecutableFile->close();

	// Fill-in executable parameters - parse arguments string into parameters
	std::vector <std::string> vArguments;
	/*wordexp_t sArguments;
	if(wordexp(pTask->m_sArguments.m_sValue.c_str(), &sArguments, 0))
		return false;
	for (uint iIndex = 0; iIndex < sArguments.we_wordc; iIndex++)
		vArguments.push_back(sArguments.we_wordv[iIndex]);
	wordfree(&sArguments);*/
   vArguments.push_back(pTask->m_sConfigDirectory.m_sArguments);

	// Fill-in executable environment
	std::vector <std::string> vEnvironment;
	GCFS_ConfigEnvironment::values_t::iterator it;
   GCFS_ConfigEnvironment::values_t &mEnvValues = (GCFS_ConfigEnvironment::values_t&)pTask->m_sConfigDirectory.m_sEnvironment;
   for(it = mEnvValues.begin(); it != mEnvValues.end(); it++)
		vEnvironment.push_back(it->first+"="+it->second);
	
	// Fill data files to send
	std::vector <std::string> vDataFiles;
	const GCFS_FileSystem::FileList* vTaskDataFiles = pTask->getDataFiles();
   for(GCFS_FileSystem::FileList::const_iterator it = vTaskDataFiles->begin(); it != vTaskDataFiles->end(); it++)
	{
      if(it->first != sExecutableName) // Ececutable is transferet automagically
		{
			// Hard-Link the file to subit directory and add to files to transfer
			std::string linkTarget = sSubmitDir+it->first;
         link(((GCFS_File*)it->second)->getPath(), linkTarget.c_str());
			chmod(linkTarget.c_str(), 0777);
			vDataFiles.push_back(it->first + " > " + it->first);
		}
	}

	// Get number of processes as string
	std::string sProcesses;
   pTask->m_sConfigDirectory.m_iProcesses.PrintValue(sProcesses);

	saga::job::description sJobDescription;
	
	try
	{
		sJobDescription.set_attribute(sja::description_interactive, sa::common_false);
		sJobDescription.set_attribute(sja::description_executable, sExecutable);
      sJobDescription.set_attribute(sja::description_error, pTask->m_sConfigDirectory.m_sError);
      sJobDescription.set_attribute(sja::description_output, pTask->m_sConfigDirectory.m_sOutput);
      if(!((std::string)pTask->m_sConfigDirectory.m_sInput).empty())
         sJobDescription.set_attribute(sja::description_input, pTask->m_sConfigDirectory.m_sInput);
		sJobDescription.set_attribute(sja::description_number_of_processes, sProcesses);

		sJobDescription.set_vector_attribute(sja::description_arguments, vArguments);
		sJobDescription.set_vector_attribute(sja::description_file_transfer, vDataFiles);
		sJobDescription.set_vector_attribute(sja::description_environment, vEnvironment);
	}
	catch(std::exception& e)
	{
		return finishTask(pTask, e.what());
	}

	// Go to submission directory - did not find any other way to set submission dir. Chdir subject to race conditions?
	chdir(sSubmitDir.c_str());

	
	SagaTaskData * pTaskData = new SagaTaskData();
	pTaskData->m_sCallback.setTask(pTask);
	pTaskData->m_sCallback.setService(this);

	
	// Start Submission
	// If running under root, change credentials to submiting user
	if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
	{
#ifdef __APPLE__
		// Set user permissions if running under root
		setregid(pTask->m_sPermissions.m_iGid, pTask->m_sPermissions.m_iGid);
		setreuid(pTask->m_sPermissions.m_iUid, pTask->m_sPermissions.m_iUid);
#else
		setresgid(pTask->m_sPermissions.m_iGid, pTask->m_sPermissions.m_iGid, 0);
		setresuid(pTask->m_sPermissions.m_iUid, pTask->m_sPermissions.m_iUid, 0);
#endif
	}

	try
	{
		pTaskData->m_sService = saga::job::service(saga::url(m_sServiceUrl));

		pTaskData->m_sJob = pTaskData->m_sService.create_job(sJobDescription);

		pTaskData->m_sJob.add_callback(saga::job::metrics::state, boost::bind(&SagaCallback::callbackStatus, pTaskData->m_sCallback, _1, _2, _3));

		// job is in ’New’ state here, we need to run it
		pTaskData->m_sJob.run();
	}
	catch(std::exception& e)
	{
		// Run back to previous working dir
		chdir(g_sConfig.m_sDataDir.c_str());

		return finishTask(pTask, e.what());
	}

	// Run back to previous working dir
	chdir(g_sConfig.m_sDataDir.c_str());

	// Grant access to submitting process (for Condor)
	GCFS_Utils::chmodRecursive(sSubmitDir.c_str(), 0777);

	pTask->m_pServiceData = (void*)(pTaskData);

	// Return back to root
	if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
	{
		// Return back permissions
#ifdef __APPLE__
		// Set user permissions if running under root
		setregid(0, 0);
		setreuid(0, 0);
#else
		setresgid(0, 0, 0);
		setresuid(0, 0, 0);
#endif
	}

	return true;
}

bool GCFS_ServiceSaga::waitForTask(GCFS_Task* pTask)
{
	SagaTaskData* pTaskData = (SagaTaskData*)pTask->m_pServiceData;

	while(true)
	{
		bool bFinished = pTaskData->m_sJob.wait(1.0);

		if(bFinished)
			break;

		if(pTask->m_eStatus == GCFS_Task::eSuspended)
			return false;
	}

	return true;
}

bool GCFS_ServiceSaga::abortTask(GCFS_Task* pTask)
{
	SagaTaskData* pTaskData = (SagaTaskData*)pTask->m_pServiceData;

	if(pTaskData)
	{
		try{
			pTaskData->m_sJob.cancel();
		}
		catch(std::exception& e)
		{
			printf("%s\n", e.what());
			return false;
		}

		delete (SagaTaskData*)pTask->m_pServiceData;

      std::string sSubmitDir = getSubmitDir(pTask);

		// Get rid of submit directory
		GCFS_Utils::rmdirRecursive(sSubmitDir.c_str());
	}

	return GCFS_Service::abortTask(pTask);
}


bool GCFS_ServiceSaga::deleteTask(GCFS_Task* pTask)
{
	abortTask(pTask);
	
	return true;
}

const std::string GCFS_ServiceSaga::getTaskId(GCFS_Task* pTask)
{
   SagaTaskData* pTaskData = (SagaTaskData*)pTask->m_pServiceData;

   if(pTaskData)
      return pTaskData->m_sJob.get_job_id();
   else
      return "cannot get ID";
}

bool isFile(const char* sPath)
{
	struct stat sFileStat;

	stat(sPath, &sFileStat);

	return S_ISREG (sFileStat.st_mode);
}

bool GCFS_ServiceSaga::finishTask(GCFS_Task* pTask, const char * sMessage)
{
	printf("Finishing task!!!!\n");
	
   std::string sSubmitDir = getSubmitDir(pTask);

	delete (SagaTaskData*)pTask->m_pServiceData;
	pTask->m_pServiceData = 0;
	
	if(sMessage)
	{
		// Write error into error file
		GCFS_File* error = pTask->createResultFile("error");

      error->write(sMessage, -1, strlen(sMessage));

		// Also announce on stdout for debug
		printf("Saga Error: %s\n", sMessage);

		// Get rid of submit directory
		GCFS_Utils::rmdirRecursive(sSubmitDir.c_str());

		// Set correct status
		pTask->m_eStatus = GCFS_Task::eFailed;
		
		return false;
	}

	// Remove executable from submitdir to prevent copying to result dir
	std::string sExecutable = sSubmitDir+basename((char*)((std::string)pTask->m_sConfigDirectory.m_sExecutable).c_str());
	unlink(sExecutable.c_str());

	DIR* pDir = opendir(sSubmitDir.c_str());
	dirent* pDirFile;

	while((pDirFile = readdir(pDir)))
	{
		std::string sFilePath = sSubmitDir+pDirFile->d_name;
		
		if(!isFile(sFilePath.c_str()))
			continue;

		GCFS_File* pNewFile = pTask->createResultFile(pDirFile->d_name, false);

		if(rename(sFilePath.c_str(), pNewFile->getPath()))
			printf("Error: Cannot move result file '%s' to '%s'\n", sFilePath.c_str(), pNewFile->getPath());
	}

	closedir(pDir);

	rmdir(sSubmitDir.c_str());

	return true;
}

const std::string GCFS_ServiceSaga::getSubmitDir(GCFS_Task* pTask)
{
   std::ostringstream os;
   os << g_sConfig.m_sDataDir << pTask->getInode() << "/";
   return os.str();
}