#include "gcfs_task.h"
#include "gcfs_config.h"

GCFS_Task::GCFS_Task(const char * sName): m_sName(sName),
	m_iMemory("memory", "1024"),
	m_iProcesses("processes", "1"),
	m_iTimeout("timeout", "3600"),
	m_iService("service", "", g_sConfig.m_vServiceNames),
	m_bCompleted(false)
{
	m_vConfigValues.push_back(&m_iMemory);
	m_vConfigValues.push_back(&m_iProcesses);
	m_vConfigValues.push_back(&m_iTimeout);
	m_vConfigValues.push_back(&m_iService);

	m_mConfigNameToIndex["memory"] = 0;
	m_mConfigNameToIndex["processes"] = 1;
	m_mConfigNameToIndex["timeout"] = 2;
	m_mConfigNameToIndex["service"] = 3;
}

bool GCFS_TaskManager::AddTask(const char * sName)
{
	m_vTasks.push_back(new GCFS_Task(sName));
	m_mTaskNames[sName] = m_vTasks.size()-1;
}

bool GCFS_TaskManager::DeleteTask(const char * sName)
{
	int iIndex = -1;
	if((iIndex = GetTaskIndex(sName)) >= 0)
	{
		delete m_vTasks[iIndex];
		m_vTasks.erase(m_vTasks.begin()+iIndex);
		m_mTaskNames.erase(sName);
	}
}

size_t GCFS_TaskManager::GetTaskCount()
{
	return (int)m_vTasks.size();
}

GCFS_Task* GCFS_TaskManager::GetTask(int iIndex)
{
	return m_vTasks[iIndex];
}

GCFS_Task* GCFS_TaskManager::GetTask(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mTaskNames.find(sName)) != m_mTaskNames.end())
		return m_vTasks[it->second];
	else
		return NULL;
}

int GCFS_TaskManager::GetTaskIndex(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mTaskNames.find(sName)) != m_mTaskNames.end())
		return it->second;
	else
		return -1;
}