#include "include/gcfs_service.h"

#include "lib/module_condor/gcfs_servicecondor.h"

#include <string.h>
#include <stdio.h>

GCFS_Service*	GCFS_Service::createService(const char * sModule, const char * sName)
{
	if(strcasecmp(sModule, "condor") == 0)
		return new GCFS_ServiceCondor(sName);
	else
	{
		printf("Unknown service driver: %s\n", sModule);
		return NULL;
	}
}