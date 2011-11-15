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

	bool							configure(CSimpleIniA& pConfig);
									
public:
	virtual bool 				submitTask(GCFS_Task* pTask);
	virtual bool 				waitForTask(GCFS_Task* pTask);
	virtual bool 				abortTask(GCFS_Task* pTask);
	virtual bool 				deleteTask(GCFS_Task* pTask);
   virtual const std::string getTaskId(GCFS_Task* pTask);
   
public:
	bool							finishTask(GCFS_Task* pTask, const char* sMessage = NULL);

// Config values
private:
	std::string					m_sServiceUrl;
	
};

#endif // GCFS_SERVICESAGA_H
