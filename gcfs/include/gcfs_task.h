#ifndef GCFS_TASK_H
#define GCFS_TASK_H

#include "gcfs.h"
#include "gcfs_config_values.h"

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>

class GCFS_Control;

// Task Configuration
class GCFS_Task {
	// Define task statuses
public:
	typedef enum {
		eNew = 0, // initial
		eSubmitted, // After start
		eRunning, // Really physically running
		eAborted, // User aborted
		eFailed, // Failed
		eFinished, // succesfully finished
		eSuspended // suspended
	} Status;

public:
													GCFS_Task(const char * sName);

public:
	bool 											isFinished();
	bool 											isSubmited();
	
public:
	std::string									m_sName;

	Status										m_eStatus;

	void*											m_pServiceData; // Data space for service
	
// Task Configuration Values
public:
	GCFS_ConfigInt 							m_iMemory;
	GCFS_ConfigInt 							m_iProcesses;
	GCFS_ConfigInt 							m_iTimeout;
	GCFS_ConfigService						m_iService;
	GCFS_ConfigString							m_sExecutable;
	GCFS_ConfigString							m_sArguments;
	GCFS_ConfigEnvironment					m_sEnvironment;

// Mapping of confg values for dynamic access
	std::vector<GCFS_ConfigValue*>		m_vConfigValues;
	std::map<std::string, int> 			m_mConfigNameToIndex;

private:
	void											assignVariable(GCFS_ConfigValue* pValue);

// File management
public:
	class File {
	public:
								File():m_iInode(0), m_pTask(NULL), m_hFile(0){};

	public:
		int					create();
		int					getHandle();
		void					closeHandle();
		
	public:
		unsigned int		m_iInode;
		std::string			m_sPath;
		GCFS_Task*			m_pTask;

	private:
		int					m_hFile;
	};

	typedef std::map<std::string, File*> Files;
	
	File*												createDataFile(const char * name);
	bool												deleteDataFile(const char * name);
	File* 											getDataFile(const char * name);
	const GCFS_Task::Files& 					getDataFiles();
	File*												createResultFile(const char* name, bool bCreate = true);
	bool												deleteResultFile(const char * name);
	File* 											getResultFile(const char * name);
	const GCFS_Task::Files&						getResultFiles();

	File*												getExecutableFile();
	
// Data and result files mapping from name to inode number
public:
	Files											m_mDataFiles;
	std::map<int, File*>						m_mInodeToDataFiles;
	Files											m_mResultFiles;
	std::map<int, File*>						m_mInodeToResultFiles;

	GCFS_Permissions 							m_sPermissions;
};

class GCFS_TaskManager {
public:
											GCFS_TaskManager();
public:
// Manage tasks
	GCFS_Task*							addTask(const char * sName);
	bool									deleteTask(const char * sName);

	size_t								getTaskCount();

	GCFS_Task*							getTask(int iIndex);
	GCFS_Task*							getTask(const char * sName);
	int									getTaskIndex(const char * sName);

private:
	std::vector<GCFS_Task*> 		m_vTasks;
	std::map<std::string, int> 	m_mTaskNames;
	
// Inode allocation and management
public:
	unsigned int						getInode(GCFS_Task::File *pFile);
	bool									putInode(int iInode);

	GCFS_Task::File*					getInodeFile(int iInode);
	
	GCFS_Task::File* 					createFile(bool bCreate = true);
	
	unsigned int						m_uiFirstFileInode;

private:
	std::map<int, GCFS_Task::File*>	m_mInodesOwner;

// Control files
public:
	size_t								getControlCount();

	GCFS_Control*						getControl(int iIndex);
	GCFS_Control*						getControl(const char * sName);
	int									getControlIndex(const char * sName);
	
private:
	std::vector<GCFS_Control*>		m_vControls;
	std::map<std::string,int>		m_mControlNames;
};

#endif // GCFS_TASK_H
