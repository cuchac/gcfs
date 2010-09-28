#include "gcfs_config_values.h"

#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <string.h>

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
	m_sValue = sValue;

	return true;
}

bool GCFS_ConfigString::PrintValue(std::string& buff)
{
	buff = m_sValue;

	return true;
}


bool GCFS_ConfigChoice::SetValue(const char * sValue)
{
	int iVal = -1;
	for(int iIndex = 0; iIndex < m_vChoices.size(); iIndex ++)
		if(strcasecmp(m_vChoices[iIndex].c_str(), sValue) == 0)
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