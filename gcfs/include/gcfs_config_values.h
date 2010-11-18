#ifndef __GCFS_CONFIG_VALUES__
#define __GCFS_CONFIG_VALUES__

#include <string>
#include <vector>
#include <map>

class GCFS_ConfigValue
{
public:
						GCFS_ConfigValue(const char *sName):m_sName(sName){};

	virtual	bool				SetValue(const char * sValue) = 0;
	virtual	bool				PrintValue(std::string &buff) = 0;

	static 	std::string 	trimStr(const std::string& Src, const std::string& c = " \r\n");

public:
	const char * 	m_sName;
	size_t			m_iSize;
	GCFS_Task* 		m_pTask;
};

class GCFS_ConfigInt: public GCFS_ConfigValue
{
public:
						GCFS_ConfigInt(const char *sName, const char *sDefault):GCFS_ConfigValue(sName){this->SetValue(sDefault);};

	bool				SetValue(const char * sValue);
	bool				PrintValue(std::string &buff);

public:
	int				m_iValue;
};

class GCFS_ConfigString: public GCFS_ConfigValue
{
public:
						GCFS_ConfigString(const char *sName, const char *sDefault):GCFS_ConfigValue(sName){this->SetValue(sDefault);};

	bool				SetValue(const char * sValue);
	bool				PrintValue(std::string &buff);

public:
	std::string		m_sValue;
};

class GCFS_ConfigChoice: public GCFS_ConfigValue
{
public:
	typedef std::vector<std::string> choices_t;
	
public:
						GCFS_ConfigChoice(const char* sName, const char* sDefault = NULL, GCFS_ConfigChoice::choices_t* pvChoices = NULL);

	bool				SetValue(const char * sValue);
	bool				PrintValue(std::string &buff);

public:
	int				m_iValue;
	choices_t 		m_vChoices;
};

// Specific configuration values

class GCFS_ConfigEnvironment: public GCFS_ConfigValue
{
public:
	typedef std::map<std::string, std::string> values_t;

public:
						GCFS_ConfigEnvironment(const char* sName, const char* sDefault = NULL):GCFS_ConfigValue(sName){this->SetValue(sDefault);};

	bool				SetValue(const char * sValue, size_t iOffset = 0);
	bool				SetValue(const char* sKey, const char* sValue);
	bool				PrintValue(std::string &buff);

public:
	values_t 		m_mValues;
};

class GCFS_ConfigService: public GCFS_ConfigChoice
{
public:
						GCFS_ConfigService(const char* sName, const char* sDefault = NULL, GCFS_ConfigChoice::choices_t* pvChoices = NULL):
															GCFS_ConfigChoice(sName, sDefault, pvChoices){this->SetValue(sDefault);};
						
public:
	bool				SetValue(const char * sValue, size_t iOffset = 0);
};

#endif /*__GCFS_CONFIG_VALUES__*/