#include "gcfs_servicesaga.h"
#include "gcfs_servicesaga_taskdata.h"

#include <gcfs_config.h>

#include <stdio.h>
#include <saga/saga.hpp>


namespace sa  = saga::attributes;
namespace sja = saga::job::attributes;

GCFS_ServiceSaga::GCFS_ServiceSaga(const char* sName): GCFS_Service(sName)
{

}

GCFS_ServiceSaga::~GCFS_ServiceSaga()
{

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

	std::string sSubmitDir = g_sConfig.m_sDataDir+pTask->m_sName+"/";
	g_sConfig.mkdirRecursive(sSubmitDir.c_str());

	GCFS_Task::File* error = pTask->createResultFile("error");
	GCFS_Task::File* output = pTask->createResultFile("output");

	chmod(error->m_sPath.c_str(), 0777);
	chmod(output->m_sPath.c_str(), 0777);
	chmod(sSubmitDir.c_str(), 0777);

	sJobDescription.set_attribute(sja::description_interactive, sa::common_false);
	sJobDescription.set_attribute(sja::description_executable, sTaskDirPath+pTask->m_sExecutable.m_sValue);
	sJobDescription.set_attribute(sja::description_error,"error");
	sJobDescription.set_attribute(sja::description_output, "output");
	sJobDescription.set_vector_attribute(sja::description_arguments, arguments);


	chdir(sSubmitDir.c_str());
	
	SagaTaskData * pTaskData = new SagaTaskData;
	pTaskData->m_sCallback.setTask(pTask);
	
	pTaskData->m_sService = saga::job::service(saga::url::url("condor://localhost"));
	
	pTaskData->m_sJob = pTaskData->m_sService.create_job(sJobDescription);

	pTaskData->m_sJob.add_callback(saga::job::metrics::state, boost::bind(&SagaCallback::callbackStatus, pTaskData->m_sCallback, _1, _2, _3));
	
	// job is in ’New’ state here, we need to run it
	pTaskData->m_sJob.run();

	chdir(g_sConfig.m_sDataDir.c_str());
	chmod((sSubmitDir+"error").c_str(), 0777);
	chmod((sSubmitDir+"output").c_str(), 0777);

	pTask->m_pServiceData = (void*)(pTaskData);

	
	if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
	{
		// Return back permissions
		setresgid(0, 0, 0);
		setresuid(0, 0, 0);
	}

	return true;
}

bool GCFS_ServiceSaga::waitForTask(GCFS_Task* pTask)
{
	SagaTaskData* pTaskData = (SagaTaskData*)pTask->m_pServiceData;

	pTaskData->m_sJob.wait();

	return true;
}
