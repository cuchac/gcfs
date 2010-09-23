#ifndef __GCFS_TASK_CONFIG__
#define __GCFS_TASK_CONFIG__

#include "gcfs.h"

#include <string>
#include <vector>
#include <map>

// Configuration options
#define GCFS_TASK_CONFIGS 4

// Task Configuration
class GCFS_TaskConfig {

public:
							GCFS_TaskConfig(const char * sName);
							GCFS_TaskConfig(const GCFS_TaskConfig & sCopy);

public:
	std::string			m_sName;

	bool					m_bCompleted;

public:
	// Task Configuration Values
	class ConfigValue
	{
	public:
							ConfigValue(const char *sName):m_sName(sName){};

		virtual	bool	SetValue(const char * sValue) = 0;
		virtual	bool	PrintValue(char * sBuffer, int iBufSize) = 0;

	public:
		const char * 	m_sName;
	};

	class ConfigInt: public ConfigValue
	{
	public:
							ConfigInt(const char *sName, const char *sDefault):ConfigValue(sName){this->SetValue(sDefault);};

		bool				SetValue(const char * sValue);
		bool				PrintValue(char * sBuffer, int iBufSize);

	public:
		int				m_iValue;
	};

	class ConfigString: public ConfigValue
	{
	public:
							ConfigString(const char *sName, const char *sDefault):ConfigValue(sName){this->SetValue(sDefault);};
							~ConfigString();

		bool				SetValue(const char * sValue);
		bool				PrintValue(char * sBuffer, int iBufSize);

	public:
		std::string		m_sValue;
	};

	class ConfigChoice: public ConfigValue
	{
		typedef std::vector<std::string> choices_t;
	public:
							ConfigChoice(const char *sName, const char *sDefault, choices_t & vChoices):ConfigValue(sName),m_vChoices(vChoices){this->SetValue(sDefault);};

		bool				SetValue(const char * sValue);
		bool				PrintValue(char * sBuffer, int iBufSize);

	public:
		int				m_iValue;
		choices_t 		m_vChoices;
	};


public:
	ConfigInt 									m_iMemory;
	ConfigInt 									m_iProcesses;
	ConfigInt 									m_iTimeout;
	ConfigChoice								m_iService;


	std::vector<ConfigValue*>				m_vIndexToName;
	std::map<std::string, int> 			m_mNameToIndex;

};

class GCFS_Tasks {
public:

	bool					AddTask(const char * sName);

	size_t				GetCount();

	GCFS_TaskConfig*	Get(int iIndex);
	GCFS_TaskConfig*	Get(const char * sName);
	int					GetIndex(const char * sName);

private:

	std::vector<GCFS_TaskConfig> 	m_vTasks;
	std::map<std::string, int> 	m_mTaskNames;
};

#endif /*__GCFS_TASK_CONFIG__*/