#ifndef GCFS_TASK_H
#define GCFS_TASK_H

#include "gcfs.h"
#include "gcfs_config_values.h"

#include <string>
#include <vector>
#include <map>

// Task Configuration
class GCFS_Task {

public:
							GCFS_Task(const char * sName);

public:
	std::string			m_sName;

	bool					m_bCompleted;

	
// Task Configuration Values
public:
	GCFS_ConfigInt 							m_iMemory;
	GCFS_ConfigInt 							m_iProcesses;
	GCFS_ConfigInt 							m_iTimeout;
	GCFS_ConfigChoice							m_iService;

// Mapping of confg values for dynamic access
	std::vector<GCFS_ConfigValue*>		m_vConfigValues;
	std::map<std::string, int> 			m_mConfigNameToIndex;

// Data and result files mapping from name to inode number
	std::map<std::string, int>				m_mDataFiles;
	std::map<std::string, int>				m_mResultFiles;
	
};

class GCFS_TaskManager {
public:
// Manage tasks
	bool									addTask(const char * sName);
	bool									deleteTask(const char * sName);

	size_t								getTaskCount();

	GCFS_Task*							getTask(int iIndex);
	GCFS_Task*							getTask(const char * sName);
	int									getTaskIndex(const char * sName);

// Inode allocation
	int									getInode();

private:

	std::vector<GCFS_Task*> 		m_vTasks;
	std::map<std::string, int> 	m_mTaskNames;
};

#endif // GCFS_TASK_H
