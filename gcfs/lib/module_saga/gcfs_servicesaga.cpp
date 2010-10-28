#include "gcfs_servicesaga.h"

GCFS_ServiceSaga::GCFS_ServiceSaga(const char* sName): GCFS_Service(sName)
{

}

bool GCFS_ServiceSaga::submitTask(GCFS_Task* pTask)
{
	pTask->m_eStatus = GCFS_Task::eSubmitted;
}


bool GCFS_ServiceSaga::waitForTask(GCFS_Task* pTask)
{
	sleep(3);
	pTask->m_eStatus = GCFS_Task::eFinished;
}

