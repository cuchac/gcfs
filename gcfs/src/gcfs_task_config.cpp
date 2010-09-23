#include "gcfs_task_config.h"
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <gcfs_config.h>

bool GCFS_Tasks::AddTask(const char * sName)
{
	m_vTasks.push_back(sName);
	m_mTaskNames[sName] = m_vTasks.size()-1;
}

size_t GCFS_Tasks::GetCount()
{
	return (int)m_vTasks.size();
}

GCFS_TaskConfig* GCFS_Tasks::Get(int iIndex)
{
	return &m_vTasks[iIndex];
}

GCFS_TaskConfig* GCFS_Tasks::Get(const char * sName)
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

GCFS_TaskConfig::GCFS_TaskConfig(const char * sName): m_sName(sName),
	m_iMemory("memory", "1024"),
	m_iProcesses("processes", "1"),
	m_iTimeout("timeout", "3600"),
	m_iService("service", GCFS_SERVICE_BALANCED, g_sConfiguration.m_vServiceNames)
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

GCFS_TaskConfig::GCFS_TaskConfig(const GCFS_TaskConfig & sCopy): m_sName(sCopy.m_sName),
	m_iMemory(sCopy.m_iMemory),
	m_iProcesses(sCopy.m_iProcesses),
	m_iTimeout(sCopy.m_iTimeout),
	m_iService(sCopy.m_iService)
{
	m_vIndexToName.push_back(&m_iMemory);
	m_vIndexToName.push_back(&m_iProcesses);
	m_vIndexToName.push_back(&m_iTimeout);
	m_vIndexToName.push_back(&m_iService);

	m_mNameToIndex = sCopy.m_mNameToIndex;
}

bool GCFS_TaskConfig::ConfigInt::SetValue(const char * sValue)
{
	m_iValue = atoi(sValue);

	return true;
}

bool GCFS_TaskConfig::ConfigInt::PrintValue(char * sBuffer, int iBufSize)
{
	snprintf(sBuffer, iBufSize, "%d", m_iValue);
	
	return true;
}


bool GCFS_TaskConfig::ConfigString::SetValue(const char * sValue)
{
	m_sValue = sValue;

	return true;
}

bool GCFS_TaskConfig::ConfigString::PrintValue(char * sBuffer, int iBufSize)
{
	snprintf(sBuffer, iBufSize, "%s", m_sValue.c_str());

	return true;
}


bool GCFS_TaskConfig::ConfigChoice::SetValue(const char * sValue)
{
	int iVal = -1;
	for(int iIndex = 0; iIndex < m_vChoices.size(); iIndex ++)
		if(strcasecmp(m_vChoices[iIndex].c_str(), sValue) == 0)
			iVal = iIndex;

	if(iVal < 0)
		return false;
	
	m_iValue = iVal;
	
	return true;
}

bool GCFS_TaskConfig::ConfigChoice::PrintValue(char * sBuffer, int iBufSize)
{
	for(int iIndex = 0; iIndex < m_vChoices.size(); iIndex ++)
	{
		if(iIndex > 0)
			strncat(sBuffer, ", ", iBufSize);
		
		if(iIndex == m_iValue)
			strncat(sBuffer, "[", iBufSize);
			
		strncat(sBuffer, m_vChoices[iIndex].c_str(), iBufSize);

		if(iIndex == m_iValue)
			strncat(sBuffer, "]", iBufSize);
	}

	return true;
}