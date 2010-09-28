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

public:
	// Task Configuration Values


public:
	GCFS_ConfigInt 							m_iMemory;
	GCFS_ConfigInt 							m_iProcesses;
	GCFS_ConfigInt 							m_iTimeout;
	GCFS_ConfigChoice							m_iService;


	std::vector<GCFS_ConfigValue*>		m_vIndexToName;
	std::map<std::string, int> 			m_mNameToIndex;

};

class GCFS_TaskManager {
public:

	bool									AddTask(const char * sName);
	bool									DeleteTask(const char * sName);

	size_t								GetTaskCount();

	GCFS_Task*							GetTask(int iIndex);
	GCFS_Task*							GetTask(const char * sName);
	int									GetTaskIndex(const char * sName);

private:

	std::vector<GCFS_Task*> 		m_vTasks;
	std::map<std::string, int> 	m_mTaskNames;
};

#endif // GCFS_TASK_H
