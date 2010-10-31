#include "gcfs_servicesaga.h"

#include <gcfs_config.h>

#include <stdio.h>
#include <saga/saga.hpp>


namespace sa  = saga::attributes;
namespace sja = saga::job::attributes;

typedef struct{
	saga::job::service m_sService;
	saga::job::job m_sJob;
} SagaTaskData;

GCFS_ServiceSaga::GCFS_ServiceSaga(const char* sName): GCFS_Service(sName)
{
	/*if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
	{
		// Set user permissions if running under root
		setresgid(pTask->m_sPermissions.m_iGid, pTask->m_sPermissions.m_iGid, 0);
		setresuid(pTask->m_sPermissions.m_iUid, pTask->m_sPermissions.m_iUid, 0);
	}
	
	m_pJobService = new saga::job::service(saga::url::url("condor://localhost"));

	if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
	{
		// Return back permissions
		setresgid(0, 0, 0);
		setresuid(0, 0, 0);
	}*/
}

GCFS_ServiceSaga::~GCFS_ServiceSaga()
{
	//delete m_pJobService;
}

bool GCFS_ServiceSaga::submitTask(GCFS_Task* pTask)
{

	GCFS_Task::File * pExecutable = pTask->getExecutableFile();
	if(!pExecutable)
		return false;
	
	if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
	{
		// Set user permissions if running under root
		setresgid(pTask->m_sPermissions.m_iGid, pTask->m_sPermissions.m_iGid, 0);
		setresuid(pTask->m_sPermissions.m_iUid, pTask->m_sPermissions.m_iUid, 0);
	}

	saga::job::description sJobDescription;

	std::vector <std::string> arguments;
	arguments.push_back (pTask->m_sArguments.m_sValue);

	std::string sTaskDirPath = g_sConfig.m_sMountDir+"/"+pTask->m_sName+"/";
	
	GCFS_Task::File* error = pTask->createResultFile("error");
	GCFS_Task::File* output = pTask->createResultFile("output");

	chmod(error->m_sPath.c_str(), 0777);
	chmod(output->m_sPath.c_str(), 0777);

	sJobDescription.set_attribute(sja::description_interactive, sa::common_false);
	sJobDescription.set_attribute(sja::description_executable, sTaskDirPath+pTask->m_sExecutable.m_sValue);
	//sJobDescription.set_attribute(sja::description_cleanup, sa::common_false);
	sJobDescription.set_attribute(sja::description_error,g_sConfig.m_sDataDir+"/error_____");
	sJobDescription.set_attribute(sja::description_output, g_sConfig.m_sDataDir+"/output_____");
	sJobDescription.set_vector_attribute(sja::description_arguments, arguments);
	//sJobDescription.set_attribute(sja::description_working_directory, g_sConfig.m_sDataDir);


	SagaTaskData * pTaskData = new SagaTaskData;
	
	pTaskData->m_sService = saga::job::service(saga::url::url("condor://localhost"));
	
	pTaskData->m_sJob = pTaskData->m_sService.create_job(sJobDescription);
	
	// job is in ’New’ state here, we need to run it
	pTaskData->m_sJob.run();

	pTask->m_pServiceData = (void*)(pTaskData);

	pTask->m_eStatus = GCFS_Task::eSubmitted;
	
	if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
	{
		// Return back permissions
		setresgid(0, 0, 0);
		setresuid(0, 0, 0);
	}
}

bool GCFS_ServiceSaga::waitForTask(GCFS_Task* pTask)
{
	SagaTaskData* pTaskData = (SagaTaskData*)pTask->m_pServiceData;

	//saga::job::istream out = pTaskData->m_sJob.get_stdout();
	//saga::job::istream err = pTaskData->m_sJob.get_stderr();

	pTaskData->m_sJob.wait();

	//storeStream(&out, pTask->createResultFile("output"));
	//storeStream(&err, pTask->createResultFile("error"));

	pTask->m_eStatus = getTaskStatus(pTask);
}

GCFS_Task::Status GCFS_ServiceSaga::getTaskStatus(GCFS_Task* pTask)
{
	SagaTaskData* pTaskData = (SagaTaskData*)pTask->m_pServiceData;
	if(!pTaskData)
		return GCFS_Task::eNew;

	saga::job::description sJobDescription = pTaskData->m_sJob.get_description();
	printf("Printing files!!\n");
	//std::vector<std::string> files = sJobDescription.get_vector_attribute(sja::description_file_transfer);
	//for(std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
	//	printf("Files!!: %s\n", it->c_str());

	switch(pTaskData->m_sJob.get_state())
	{
		case saga::job::Done:
			return GCFS_Task::eFinished;
		case saga::job::Failed:
			return GCFS_Task::eFailed;
		case saga::job::Canceled:
			return GCFS_Task::eAborted;
		case saga::job::New:
			return GCFS_Task::eNew;
		case saga::job::Running:
			return GCFS_Task::eRunning;
		case saga::job::Suspended:
			return GCFS_Task::eSuspended;
		default:
			printf("Unknown job status: %d!\n", pTaskData->m_sJob.get_state());
			return GCFS_Task::eFailed;
	}
}

GCFS_Task::Status GCFS_ServiceSaga::storeStream(saga::job::istream* pStream, GCFS_Task::File* pFile)
{
	while ( true/*state != saga::job::Done &&
				state != saga::job::Failed &&
				state != saga::job::Canceled */)
	{
		char buffer[256];

		pStream->read (buffer, ARRAYSIZE(buffer));
		if(pStream->gcount() > 0)
		{
			
			write(pFile->m_hFile, buffer, ARRAYSIZE(buffer));
			std::cout << std::string(buffer, pStream->gcount());
		}
		if(pStream->fail())
			break;
		//state = j.get_state ();
	}
}