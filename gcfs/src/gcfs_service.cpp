#include "gcfs_service.h"
#include "config.h"

#ifdef GCFS_MODULE_CONDOR
	#include "lib/module_condor/gcfs_servicecondor.h"
#endif
#ifdef GCFS_MODULE_SAGA
	#include "lib/module_saga/gcfs_servicesaga.h"
#endif

#include <string.h>
#include <stdio.h>

GCFS_Service*	GCFS_Service::createService(const char * sModule, const char * sName)
{
	
#ifdef GCFS_MODULE_CONDOR
	if(strcasecmp(sModule, "condor") == 0)
		return new GCFS_ServiceCondor(sName);
#endif

#ifdef GCFS_MODULE_SAGA
	if(strcasecmp(sModule, "saga") == 0)
		return new GCFS_ServiceSaga(sName);
#endif
		
	printf("Error! Unknown service driver: %s\n", sModule);
	return NULL;

}