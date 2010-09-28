#include "gcfs_task.h"
#include "gcfs_config.h"

GCFS_Task::GCFS_Task(const char * sName): m_sName(sName),
	m_iMemory("memory", "1024"),
	m_iProcesses("processes", "1"),
	m_iTimeout("timeout", "3600"),
	m_iService("service", "", g_sConfig.m_vServiceNames),
	m_bCompleted(false)
{
	m_vIndexToName.push_back(&m_iMemory);
	m_vIndexToName.push_back(&m_iProcesses);
	m_vIndexToName.push_back(&m_iTimeout);
	m_vIndexToName.push_back(&m_iService);

	m_mNameToIndex["memory"] = 0;
	m_mNameToIndex["processes"] = 1;
	m_mNameToIndex["timeout"] = 2;
	m_mNameToIndex["service"] = 3;
}

GCFS_Task::GCFS_Task(const GCFS_Task & sCopy): m_sName(sCopy.m_sName),
	m_iMemory(sCopy.m_iMemory),
	m_iProcesses(sCopy.m_iProcesses),
	m_iTimeout(sCopy.m_iTimeout),
	m_iService(sCopy.m_iService),
	m_bCompleted(sCopy.m_bCompleted)
{
	m_vIndexToName.push_back(&m_iMemory);
	m_vIndexToName.push_back(&m_iProcesses);
	m_vIndexToName.push_back(&m_iTimeout);
	m_vIndexToName.push_back(&m_iService);

	m_mNameToIndex = sCopy.m_mNameToIndex;
}

bool GCFS_Tasks::AddTask(const char * sName)
{
	m_vTasks.push_back(sName);
	m_mTaskNames[sName] = m_vTasks.size()-1;
}

size_t GCFS_Tasks::GetCount()
{
	return (int)m_vTasks.size();
}

GCFS_Task* GCFS_Tasks::Get(int iIndex)
{
	return &m_vTasks[iIndex];
}

GCFS_Task* GCFS_Tasks::Get(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mTaskNames.find(sName)) != m_mTaskNames.end())
		return &m_vTasks[it->second];
	else
		return NULL;
}

int GCFS_Tasks::GetIndex(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mTaskNames.find(sName)) != m_mTaskNames.end())
		return it->second;
	else
		return -1;
}