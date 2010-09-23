#include <gcfs.h>
#include <gcfs_fuse.h>
#include "gcfs_config.h"
#include "gcfs_task_config.h"
#include <stdio.h>

GCFS_Config g_sConfiguration;
std::vector<GCFS_TaskConfig> g_vTasks;
std::map<std::string, int> g_mTaskNames;

int main(int argc, char *argv[]){

	g_sConfiguration.m_vServices.push_back(GCFS_SERVICE_BALANCED);
	g_sConfiguration.m_vServiceNames.push_back(GCFS_SERVICE_BALANCED);
	g_sConfiguration.m_vServices.push_back("Test");
	g_sConfiguration.m_vServiceNames.push_back("Test");
	g_sConfiguration.m_vServices.push_back("Test2");
	g_sConfiguration.m_vServiceNames.push_back("Test2");

	g_vTasks.push_back(GCFS_TaskConfig("Pokus"));
	g_mTaskNames["Pokus"] = g_vTasks.size()-1;

	for(int iIndex = 0; iIndex < g_vTasks.size(); iIndex++)
		printf("Task: %s\n", g_vTasks[iIndex].m_sName.c_str());

	int iParentIndex = 0;
	for(int iIndex = 0; iIndex < g_vTasks[iParentIndex].m_vIndexToName.size(); iIndex++)
			printf("Value: %s\n", g_vTasks[iParentIndex].m_vIndexToName[iIndex]->m_sName);
	
	return init_fuse(argc, argv);
}