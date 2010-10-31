#ifndef GCFS_SERVICESAGA_H
#define GCFS_SERVICESAGA_H

#include "gcfs_service.h"

namespace saga {
namespace job {
class service;
class istream;
}
}


class GCFS_ServiceSaga : public GCFS_Service
{
public:
									GCFS_ServiceSaga(const char * sName);
								  ~GCFS_ServiceSaga();
									
public:
	virtual bool 				submitTask(GCFS_Task* pTask);
	virtual bool 				waitForTask(GCFS_Task* pTask);

public:
	GCFS_Task::Status			getTaskStatus(GCFS_Task* pTask);
	GCFS_Task::Status 		storeStream(saga::job::istream* pStream, GCFS_Task::File* pFile);
	
private:
	saga::job::service* 		m_pJobService;
	
};

#endif // GCFS_SERVICESAGA_H
