#ifndef GCFS_SERVICE_H
#define GCFS_SERVICE_H

#include "gcfs_task.h"

#include <map>
#include <string>

#ifndef INCLUDED_SimpleIni_h
class CSimpleIniA;
#endif

class GCFS_Service
{
public:
										GCFS_Service(const char * sName):m_sName(sName){};
	virtual									~GCFS_Service(){};

	// Configure service from config file
	virtual bool					configure(CSimpleIniA& pConfig);

	// Hook for service-dependent customization of task
	virtual bool					customizeTask(GCFS_Task* pTask);

public:
	// Factory of modules instances
	static	GCFS_Service*		createService(const char * sModule, const char * sName);

public:
	// Public module API for task submission
	virtual	bool					submitTask(GCFS_Task* pTask);
	virtual	bool					waitForTask(GCFS_Task* pTask);
	virtual	bool					abortTask(GCFS_Task* pTask);
	virtual	bool					deleteTask(GCFS_Task* pTask);

	virtual	GCFS_Task::Status	getTaskStatus(GCFS_Task* pTask);

public:
	std::string						m_sName;

	// Default confguration values
	std::map<std::string, std::string> m_mDefaults;
	std::map<std::string, std::string> m_mEnvironment;

};

#endif // GCFS_SERVICE_H
