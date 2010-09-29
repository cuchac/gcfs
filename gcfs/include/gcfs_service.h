#ifndef GCFS_SERVICE_H
#define GCFS_SERVICE_H

#include <map>
#include <string>

class GCFS_Task;

#ifndef INCLUDED_SimpleIni_h
class CSimpleIniA;
#endif

class GCFS_Service
{
public:
									GCFS_Service(const char * sName):m_sName(sName){};

	bool							configure(CSimpleIniA * pConfig);

public:
	// Factory of modules instances
	static	GCFS_Service*	createService(const char * sModule, const char * sName);

public:
	// Public module API for task submission
	virtual	bool				submitTask(GCFS_Task* pTask) = 0;
	virtual	bool				getTaskResult(GCFS_Task* pTask) = 0;

public:
	std::string					m_sName;

};

#endif // GCFS_SERVICE_H
