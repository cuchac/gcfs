#include "gcfs_config_values.h"

#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <wordexp.h>
#include <gcfs.h>
#include <gcfs_config.h>
#include <gcfs_service.h>
#include <gcfs_utils.h>

GCFS_ConfigValue::GCFS_ConfigValue(GCFS_Directory * pParent):GCFS_FileSystem(pParent), m_iSize(0)
{
   
}

GCFS_ConfigValue::~GCFS_ConfigValue()
{
   
}

ssize_t GCFS_ConfigValue::read(std::string &sBuffer, off_t uiOffset, size_t uiSize)
{
   PrintValue(sBuffer);
   sBuffer += "\n";
   return max(0, sBuffer.size() - uiOffset);
}

ssize_t GCFS_ConfigValue::write(const char* sBuffer, off_t uiOffset, size_t uiSize)
{
   SetValue(std::string(sBuffer, uiSize).c_str(), uiOffset);
   return uiSize;
}

/***************************************************************************/
GCFS_ConfigInt::GCFS_ConfigInt(GCFS_Directory * pParent, const char *sDefault) :GCFS_ConfigValue(pParent) 
{
   this->SetValue(sDefault);
}

bool GCFS_ConfigInt::SetValue(const char* sValue, size_t iOffset)
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

GCFS_ConfigInt::operator int()
{
	return m_iValue;
}

/***************************************************************************/
GCFS_ConfigString::GCFS_ConfigString(GCFS_Directory * pParent, const char *sDefault): GCFS_ConfigValue(pParent) 
{
   this->SetValue(sDefault);
   
}

bool GCFS_ConfigString::SetValue(const char* sValue, size_t iOffset)
{
	m_sValue = GCFS_Utils::TrimStr(sValue);

	return true;
}

bool GCFS_ConfigString::PrintValue(std::string& buff)
{
	buff = m_sValue;

	return true;
}

GCFS_ConfigString::operator std::string()
{
	return m_sValue;
}

/***************************************************************************/
GCFS_ConfigChoice::GCFS_ConfigChoice(GCFS_Directory * pParent, const char *sDefault, choices_t * pvChoices) :GCFS_ConfigValue(pParent)
{
	if(pvChoices)
		m_vChoices = *pvChoices;

	if(sDefault)
		this->SetValue(sDefault);
	else
		m_iValue = 0;
}

bool GCFS_ConfigChoice::SetValue(const char* sValue, size_t iOffset)
{
	if(!sValue)
		return false;
	
   std::string value = GCFS_Utils::TrimStr(sValue);
	
	uint iVal = -1;
	for(uint iIndex = 0; iIndex < m_vChoices.size(); iIndex ++)
		if(strcasecmp(m_vChoices[iIndex].c_str(), value.c_str()) == 0)
			iVal = iIndex;

	if(iVal < 0)
		return false;
	
	m_iValue = iVal;
	
	return true;
}

bool GCFS_ConfigChoice::PrintValue(std::string& buff)
{
	for(uint iIndex = 0; iIndex < m_vChoices.size(); iIndex ++)
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

GCFS_ConfigChoice::operator int()
{
	return m_iValue;
}

/***************************************************************************/
GCFS_ConfigEnvironment::GCFS_ConfigEnvironment(GCFS_Directory * pParent, const char* sDefault): GCFS_ConfigValue(pParent) 
{
   this->SetValue(sDefault);
}

bool GCFS_ConfigEnvironment::SetValue(const char* sValue, size_t iOffset)
{
	if(!sValue)
		return false;
	
	if(!iOffset) // Allow overwrite
		m_mValues.clear();
	else if(iOffset == (size_t)-1 || iOffset == m_iSize) // Allow to append
		; 
	else // Dont allow to write in the middle
		return false; 

   std::string sNewValue = GCFS_Utils::TrimStr(sValue);

	wordexp_t sArguments;
	if(wordexp(sNewValue.c_str(), &sArguments, 0))
		return false;
	
	for (uint iIndex = 0; iIndex < sArguments.we_wordc; iIndex++)
	{
		// Separate key=value string into two pieces
		char* pcSeparator = strchr(sArguments.we_wordv[iIndex], '=');
		*pcSeparator='\0';
		
		printf("Argument: %s, tokens: %s, %s\n", sArguments.we_wordv[iIndex], sArguments.we_wordv[iIndex], pcSeparator+1);
		
		m_mValues[sArguments.we_wordv[iIndex]] = pcSeparator+1;
	}
	wordfree(&sArguments);

	return true;
}

bool GCFS_ConfigEnvironment::SetValue(const char* sKey, const char* sValue)
{
	if(!sValue || !sKey)
		return false;

   std::string sNewValue = GCFS_Utils::TrimStr(sValue);

	m_mValues[sKey]=sNewValue;
	
	return true;
}

bool GCFS_ConfigEnvironment::PrintValue(std::string& buff)
{
	values_t::iterator it;
	for(it = m_mValues.begin(); it != m_mValues.end(); it++)
		buff += it->first+"="+it->second+"\n";

	return true;
}

GCFS_ConfigEnvironment::operator values_t()
{
	return m_mValues;
}

/***************************************************************************/
GCFS_ConfigService::GCFS_ConfigService(GCFS_Directory * pParent, const char* sDefault, GCFS_ConfigChoice::choices_t* pvChoices)
   :GCFS_ConfigChoice(pParent, sDefault, pvChoices) 
{
   this->SetValue(sDefault);
   
}

bool GCFS_ConfigService::SetValue(const char* sValue, size_t iOffset)
{	
	bool bRet = GCFS_ConfigChoice::SetValue(sValue, iOffset);

	if(!bRet)
		return bRet;

	g_sConfig.GetService(m_iValue)->customizeTask(getParentTask());
	
	return true;
}

