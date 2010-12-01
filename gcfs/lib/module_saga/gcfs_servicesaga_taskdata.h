#ifndef GCFS_SERVICESAGA_TASKDATA_H
#define GCFS_SERVICESAGA_TASKDATA_H

#include <saga/saga.hpp>
#include <gcfs_task.h>

class GCFS_ServiceSaga;

class SagaCallback : public saga::callback
{
public:
	void 						setTask(GCFS_Task* pTask);
	void 						setService(GCFS_ServiceSaga* pService);

public:
	GCFS_Task::Status 	convertStatus(const char * sStatus);

	bool 						callbackStatus (saga::monitorable mt, saga::metric m, saga::context c);

private:
	GCFS_Task* 				m_pTask;
	GCFS_ServiceSaga* 	m_pService;

};

class SagaTaskData {
public:
	saga::job::service 	m_sService;
	saga::job::job 		m_sJob;
	SagaCallback			m_sCallback;
};


#endif /*GCFS_SERVICESAGA_TASKDATA_H*/
