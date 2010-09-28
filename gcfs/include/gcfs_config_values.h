#ifndef __GCFS_CONFIG_VALUES__
#define __GCFS_CONFIG_VALUES__

#include "gcfs.h"

#include <string>
#include <vector>
#include <map>

class GCFS_ConfigValue
{
public:
						GCFS_ConfigValue(const char *sName):m_sName(sName){};

	virtual	bool	SetValue(const char * sValue) = 0;
	virtual	bool	PrintValue(std::string &buff) = 0;

public:
	const char * 	m_sName;
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
						~GCFS_ConfigString();

	bool				SetValue(const char * sValue);
	bool				PrintValue(std::string &buff);

public:
	std::string		m_sValue;
};

class GCFS_ConfigChoice: public GCFS_ConfigValue
{
	typedef std::vector<std::string> choices_t;
public:
						GCFS_ConfigChoice(const char *sName, const char *sDefault, choices_t & vChoices):GCFS_ConfigValue(sName),m_vChoices(vChoices){this->SetValue(sDefault);};

	bool				SetValue(const char * sValue);
	bool				PrintValue(std::string &buff);

public:
	int				m_iValue;
	choices_t 		m_vChoices;
};



#endif /*__GCFS_CONFIG_VALUES__*/