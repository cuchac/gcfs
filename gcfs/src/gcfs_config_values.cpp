#include "gcfs_config_values.h"

#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <string.h>

std::string GCFS_ConfigValue::trimStr(const std::string& Src, const std::string& c)
{
	int p2 = Src.find_last_not_of(c);
	if (p2 == std::string::npos) return std::string();
	int p1 = Src.find_first_not_of(c);
	if (p1 == std::string::npos) p1 = 0;
	return Src.substr(p1, (p2-p1)+1);
}

bool GCFS_ConfigInt::SetValue(const char * sValue)
{
	m_iValue = atoi(sValue);

	return true;
}

bool GCFS_ConfigInt::PrintValue(std::string& buff)
{
	size_t size = snprintf(NULL, 0, "%d", m_iValue);
	buff.resize(size+1);
	snprintf((char*)buff.c_str(), size+1, "%d", m_iValue);
	
	return true;
}


bool GCFS_ConfigString::SetValue(const char * sValue)
{
	m_sValue = trimStr(sValue);

	return true;
}

bool GCFS_ConfigString::PrintValue(std::string& buff)
{
	buff = m_sValue;

	return true;
}

bool GCFS_ConfigChoice::SetValue(const char * sValue)
{
	std::string value = trimStr(sValue);
	
	int iVal = -1;
	for(int iIndex = 0; iIndex < m_vChoices.size(); iIndex ++)
		if(strcasecmp(m_vChoices[iIndex].c_str(), value.c_str()) == 0)
			iVal = iIndex;

	if(iVal < 0)
		return false;
	
	m_iValue = iVal;
	
	return true;
}

bool GCFS_ConfigChoice::PrintValue(std::string& buff)
{
	for(int iIndex = 0; iIndex < m_vChoices.size(); iIndex ++)
	{
		if(iIndex > 0)
			buff += ", ";
		
		if(iIndex == m_iValue)
			buff +=  "[";
			
		buff += m_vChoices[iIndex];

		if(iIndex == m_iValue)
			buff +=  "]";
	}

	return true;
}