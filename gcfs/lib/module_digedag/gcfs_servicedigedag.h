#ifndef GCFS_SERVICEDIGEDAG_H
#define GCFS_SERVICEDIGEDAG_H

#include "lib/simpleini/SimpleIni.h"
#include "lib/module_saga/gcfs_servicesaga.h"
#include <gcfs_service.h>

class GCFS_ServiceDigedag: public GCFS_ServiceSaga
{
public:
                              GCFS_ServiceDigedag(const char * sName);

public:
   virtual bool               customizeTask(GCFS_Task* pTask, GCFS_ConfigDirectory * pDirectory);
   virtual bool               decustomizeTask(GCFS_Task* pTask, GCFS_ConfigDirectory * pDirectory);

public:
   // Public module API for task submission
   virtual bool               submitTask(GCFS_Task* pTask);
   virtual bool               waitForTask(GCFS_Task* pTask);

protected:
   virtual bool               setDescription(GCFS_Task* pTask, saga::job::description& pDesc);
   virtual bool               prepareJobDir(GCFS_Task* pTask);
   
public:
   virtual const std::string  getTaskId(GCFS_Task* pTask);
};

#endif // GCFS_SERVICEDIGEDAG_H
