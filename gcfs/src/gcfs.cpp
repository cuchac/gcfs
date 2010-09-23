#include <gcfs.h>
#include <gcfs_fuse.h>
#include "gcfs_config.h"
#include "gcfs_task_config.h"
#include <stdio.h>

GCFS_Config g_sConfiguration;
GCFS_Tasks g_sTasks;

int main(int argc, char *argv[]){

	g_sConfiguration.m_vServices.push_back(GCFS_SERVICE_BALANCED);
	g_sConfiguration.m_vServiceNames.push_back(GCFS_SERVICE_BALANCED);
	g_sConfiguration.m_vServices.push_back("Test");
	g_sConfiguration.m_vServiceNames.push_back("Test");
	g_sConfiguration.m_vServices.push_back("Test2");
	g_sConfiguration.m_vServiceNames.push_back("Test2");

	g_sTasks.AddTask("Test");
	g_sTasks.AddTask("Test2");

	return init_fuse(argc, argv);
}