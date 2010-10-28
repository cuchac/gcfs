#ifndef GCFS_SERVICESAGA_H
#define GCFS_SERVICESAGA_H

#include "gcfs_service.h"


class GCFS_ServiceSaga : public GCFS_Service
{
public:
									GCFS_ServiceSaga(const char * sName);
									
public:
	virtual bool 				submitTask(GCFS_Task* pTask);
	virtual bool 				waitForTask(GCFS_Task* pTask);
    
};

#endif // GCFS_SERVICESAGA_H
