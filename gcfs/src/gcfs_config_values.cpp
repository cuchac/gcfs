#include "gcfs_config_values.h"

#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <wordexp.h>
#include <gcfs.h>
#include <gcfs_config.h>
#include <gcfs_service.h>

std::string GCFS_ConfigValue::TrimStr(const std::string& Src, const std::string& c)
{
	size_t p2 = Src.find_last_not_of(c);
	if (p2 == std::string::npos) return std::string();
	size_t p1 = Src.find_first_not_of(c);
	if (p1 == std::string::npos) p1 = 0;
	return Src.substr(p1, (p2-p1)+1);
}

GCFS_ConfigValue::keyValue_t GCFS_ConfigValue::ParseAssignment(char * string)
{
   char *pChar = NULL, *pKey = NULL, *pValue = NULL;
   if((pKey = strtok_r(string, GCFS_CONFIG_ASSIGNCHAR, &pChar)) != NULL &&
      (pValue = strtok_r(NULL, GCFS_CONFIG_ASSIGNCHAR, &pChar)) != NULL)
      // Return key-value pair
      return keyValue_t(pKey, pValue);

   // No assignment -> it is command
   return keyValue_t(string, NULL);
}

bool GCFS_ConfigValue::ParseConfigString(char * string, GCFS_ConfigValue::keyValueArray_t& vValues)
{
   // replace $(process) with @(process) to get rid of '$' character. It is delimiter as well
   ReplaceToken(string, "$(process)", "@(process)");
   
   // Parse all the commands/assignments
   char * pPos = NULL;
   char * pToken = strtok_r(string, GCFS_CONFIG_DELIMITERS, &pPos);

   while(pToken != NULL)
   {
      ReplaceToken(pToken, "@(process)", "$(process)");
      
      vValues.push_back(ParseAssignment(pToken));

      pToken = strtok_r(NULL, GCFS_CONFIG_DELIMITERS, &pPos);
   }

   return true;
}

void GCFS_ConfigValue::ReplaceToken(char * sString, const char * sSearch, const char * sReplacement)
{
   char * pPos = sString;
   while((pPos = strcasestr(pPos, sSearch)) != NULL)
   {
      memcpy(pPos, sReplacement, strlen(sReplacement));
      pPos++;
   }
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

bool GCFS_ConfigString::SetValue(const char* sValue, size_t iOffset)
{
	m_sValue = TrimStr(sValue);

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

GCFS_ConfigChoice::GCFS_ConfigChoice(const char *sName, const char *sDefault, choices_t * pvChoices)
	:GCFS_ConfigValue(sName)
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
	
	std::string value = TrimStr(sValue);
	
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

	std::string sNewValue = TrimStr(sValue);

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

	std::string sNewValue = TrimStr(sValue);

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

bool GCFS_ConfigService::SetValue(const char* sValue, size_t iOffset)
{	
	bool bRet = GCFS_ConfigChoice::SetValue(sValue, iOffset);

	if(!bRet)
		return bRet;

	g_sConfig.GetService(m_iValue)->customizeTask(m_pTask);
	
	return true;
}

