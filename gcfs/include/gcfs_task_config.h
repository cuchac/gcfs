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

public:
	std::string			m_sName;

public:
	// Task Configuration Values
	class ConfigValue
	{
	public:
							ConfigValue(const char *sName, const char *sDefault);

		virtual	bool	SetValue(const char * sValue) = 0;
		virtual	bool	PrintValue(char * sBuffer, int iBufSize) = 0;

	public:
		const char * 	m_sName;
		const char * 	m_sDefault;
	};

	class ConfigInt: public ConfigValue
	{
	public:
							ConfigInt(const char *sName, const char *sDefault):ConfigValue(sName, sDefault){};

		bool				SetValue(const char * sValue);
		bool				PrintValue(char * sBuffer, int iBufSize);

	public:
		int				m_iValue;
	};

	class ConfigString: public ConfigValue
	{
	public:
							ConfigString(const char *sName, const char *sDefault):ConfigValue(sName, sDefault){};
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
							ConfigChoice(const char *sName, const char *sDefault, choices_t & vChoices):ConfigValue(sName, sDefault),m_vChoices(vChoices){};

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

#endif /*__GCFS_TASK_CONFIG__*/