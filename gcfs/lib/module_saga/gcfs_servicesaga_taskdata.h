#ifndef GCFS_SERVICESAGA_TASKDATA_H
#define GCFS_SERVICESAGA_TASKDATA_H

#include <saga/saga.hpp>
#include <gcfs_task.h>

class SagaCallback : public saga::callback
{
	GCFS_Task* m_pTask;

public:
	void setTask(GCFS_Task* pTask){m_pTask = pTask;};

	GCFS_Task::Status convertStatus(const char * sStatus);

	bool callbackStatus (saga::monitorable mt,
				saga::metric      m,
				saga::context     c);
};

typedef struct{
	saga::job::service 	m_sService;
	saga::job::job 		m_sJob;
	SagaCallback			m_sCallback;
} SagaTaskData;


#endif /*GCFS_SERVICESAGA_TASKDATA_H*/