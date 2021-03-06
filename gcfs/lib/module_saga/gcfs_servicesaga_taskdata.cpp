#include "gcfs_servicesaga_taskdata.h"
#include "gcfs_servicesaga.h"
#include <gcfs_config.h>

void SagaCallback::setTask(GCFS_Task* pTask)
{
	m_pTask = pTask;
}

void SagaCallback::setService(GCFS_ServiceSaga* pService)
{
	m_pService = pService;
}

GCFS_Task::Status SagaCallback::convertStatus(const char * sStatus)
{
	if(strcmp(sStatus, "Done") == 0)
		return GCFS_Task::eFinished;
	else if(strcmp(sStatus, "Failed") == 0)
		return GCFS_Task::eFailed;
	else if(strcmp(sStatus, "Canceled") == 0)
		return GCFS_Task::eAborted;
	else if(strcmp(sStatus, "New") == 0)
		return GCFS_Task::eNew;
	else if(strcmp(sStatus, "Running") == 0)
		return GCFS_Task::eRunning;
	else if(strcmp(sStatus, "Suspended") == 0)
		return GCFS_Task::eSuspended;
	else
	{
		printf("Unknown job status: %s!\n", sStatus);
		return GCFS_Task::eFailed;
	}
}

bool SagaCallback::callbackStatus (saga::monitorable mt,
			saga::metric      m,
			saga::context     c)
{
	std::string sStatus = m.get_attribute("Value");
   saga::detail::attribute< saga::metric >::strvec_type list = m.list_attributes();

	m_pTask->m_eStatus = convertStatus(sStatus.c_str());

	printf("Status changed to: %s => %d\n", sStatus.c_str(), m_pTask->m_eStatus);

	if(m_pTask->isFinished())
		m_pService->finishTask(m_pTask);

	return true; // keep callback registered
}
