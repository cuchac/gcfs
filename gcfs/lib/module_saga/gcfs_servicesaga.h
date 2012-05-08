#ifndef GCFS_SERVICESAGA_H
#define GCFS_SERVICESAGA_H

#include "gcfs_service.h"

namespace saga {
namespace job {
class service;
class istream;
class description;
}
}

class GCFS_ServiceSaga : public GCFS_Service
{
public:
                              GCFS_ServiceSaga(const char * sName);
                             ~GCFS_ServiceSaga();

   bool                       configure(CSimpleIniA& pConfig);

public:
   virtual bool               submitTask(GCFS_Task* pTask);
   virtual bool               waitForTask(GCFS_Task* pTask);
   virtual bool               abortTask(GCFS_Task* pTask);
   virtual bool               deleteTask(GCFS_Task* pTask);
   virtual const std::string  getTaskId(GCFS_Task* pTask);

public:
   bool                       finishTask(GCFS_Task* pTask, const char* sMessage = NULL);
   const std::string          getSubmitDir(GCFS_Task* pTask);

protected:
   virtual bool               setDescription(GCFS_Task* pTask, saga::job::description& pDesc);
   virtual bool               prepareJobDir(GCFS_Task* pTask);
   
// Config values
protected:
   std::string                m_sServiceUrl;
   saga::job::service        *m_pService;
};

#endif // GCFS_SERVICESAGA_H
