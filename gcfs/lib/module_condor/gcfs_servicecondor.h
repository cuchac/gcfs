#ifndef GCFS_SERVICECONDOR_H
#define GCFS_SERVICECONDOR_H

#include "lib/simpleini/SimpleIni.h"
#include <gcfs_service.h>

class GCFS_ServiceCondor: public GCFS_Service
{
public:
									GCFS_ServiceCondor(const char * sName);

public:
	// Public module API for task submission
	virtual	bool				submitTask(GCFS_Task* pTask);
	virtual	bool				waitForTask(GCFS_Task* pTask);
};

#endif // GCFS_SERVICECONDOR_H
