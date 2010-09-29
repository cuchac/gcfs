#include "gcfs_service.h"
#include "config.h"

#ifdef GCFS_MODULE_CONDOR
	#include "lib/module_condor/gcfs_servicecondor.h"
#endif

#include <string.h>
#include <stdio.h>

GCFS_Service*	GCFS_Service::createService(const char * sModule, const char * sName)
{
	
#ifdef GCFS_MODULE_CONDOR
	if(strcasecmp(sModule, "condor") == 0)
		return new GCFS_ServiceCondor(sName);
#endif
		
	printf("Error! Unknown service driver: %s\n", sModule);
	return NULL;

}