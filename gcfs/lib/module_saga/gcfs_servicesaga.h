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
	
};

#endif // GCFS_SERVICESAGA_H
