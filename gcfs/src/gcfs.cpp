#include <gcfs.h>
#include <gcfs_fuse.h>
#include "gcfs_config.h"
#include "gcfs_task_config.h"

GCFS_Config g_sConfiguration;
std::vector<GCFS_TaskConfig> g_vTasks;

int main(int argc, char *argv[]){

	g_sConfiguration.m_vServices.push_back(GCFS_SERVICE_BALANCED);
	g_sConfiguration.m_vServiceNames.push_back(GCFS_SERVICE_BALANCED);
	g_sConfiguration.m_vServices.push_back("Test");
	g_sConfiguration.m_vServiceNames.push_back("Test");
	g_sConfiguration.m_vServices.push_back("Test2");
	g_sConfiguration.m_vServiceNames.push_back("Test2");

	g_vTasks.push_back(GCFS_TaskConfig("Pokus"));
	
	return init_fuse(argc, argv);
}