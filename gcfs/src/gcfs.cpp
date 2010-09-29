#include "gcfs.h"
#include "gcfs_fuse.h"
#include "gcfs_config.h"
#include "gcfs_task.h"

#include <stdio.h>

GCFS_Config g_sConfig;
GCFS_TaskManager g_sTasks;

int main(int argc, char *argv[]){

	g_sConfig.loadConfig();
	
	g_sConfig.AddService("condor", "test");

	g_sTasks.AddTask("Test");
	g_sTasks.AddTask("Test2");

	return init_fuse(argc, argv);
}