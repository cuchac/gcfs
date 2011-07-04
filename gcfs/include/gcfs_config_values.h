#ifndef __GCFS_CONFIG_VALUES__
#define __GCFS_CONFIG_VALUES__

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

// Characters to trim from configuration names and values
#define GCFS_CONFIG_TRIMCHARS " \r\n"
// Delimiters of configuration values
#define GCFS_CONFIG_DELIMITERS "\r\n$;"
// Delimiters of assignment
#define GCFS_CONFIG_ASSIGNCHAR "="

// Generic configuration values

class GCFS_Task;
class GCFS_ConfigValue
{
public:
                  GCFS_ConfigValue(const char *sName): m_sName(sName), m_iSize(0) {};
   virtual       ~GCFS_ConfigValue() {};

public:
   virtual  bool  SetValue(const char * sValue, size_t iOffset = 0) = 0;
   virtual  bool  PrintValue(std::string &buff) = 0;

public:
   static std::string    TrimStr(const std::string& Src, const std::string& c = GCFS_CONFIG_TRIMCHARS);
   
public:
   // Configuration string parser
   typedef  std::pair<const char*, const char *> keyValue_t;
   typedef  std::vector<keyValue_t> keyValueArray_t;
   
   static keyValue_t          ParseAssignment(char* string);
   static bool                ParseConfigString(char* string, GCFS_ConfigValue::keyValueArray_t& vValues);
   static void                ReplaceToken(char* sString, const char* sSearch, const char* sReplacement);

public:
   const char *   m_sName;
   size_t         m_iSize;
   GCFS_Task*     m_pTask;
};

class GCFS_ConfigInt: public GCFS_ConfigValue
{
public:
   GCFS_ConfigInt(const char *sName, const char *sDefault): GCFS_ConfigValue(sName) {this->SetValue(sDefault);};

   bool     SetValue(const char * sValue, size_t iOffset = 0);
   bool     PrintValue(std::string &buff);
   operator int();

public:
   int    m_iValue;
};

class GCFS_ConfigString: public GCFS_ConfigValue
{
public:
   GCFS_ConfigString(const char *sName, const char *sDefault): GCFS_ConfigValue(sName) {this->SetValue(sDefault);};

   bool     SetValue(const char * sValue, size_t iOffset = 0);
   bool     PrintValue(std::string &buff);
   operator std::string();

public:
   std::string  m_sValue;
};

class GCFS_ConfigChoice: public GCFS_ConfigValue
{
public:
   typedef std::vector<std::string> choices_t;

public:
   GCFS_ConfigChoice(const char* sName, const char* sDefault = NULL, GCFS_ConfigChoice::choices_t* pvChoices = NULL);

   bool     SetValue(const char * sValue, size_t iOffset = 0);
   bool     PrintValue(std::string &buff);
   operator int();

public:
   uint        m_iValue;
   choices_t   m_vChoices;
};

// Specific configuration values

class GCFS_ConfigEnvironment: public GCFS_ConfigValue
{
public:
   typedef std::map<std::string, std::string> values_t;

public:
   GCFS_ConfigEnvironment(const char* sName, const char* sDefault = NULL): GCFS_ConfigValue(sName) {this->SetValue(sDefault);};

   bool     SetValue(const char * sValue, size_t iOffset = 0);
   bool     SetValue(const char* sKey, const char* sValue);
   bool     PrintValue(std::string &buff);
   operator values_t();

public:
   values_t   m_mValues;
};

class GCFS_ConfigService: public GCFS_ConfigChoice
{
public:
   GCFS_ConfigService(const char* sName, const char* sDefault = NULL, GCFS_ConfigChoice::choices_t* pvChoices = NULL):
         GCFS_ConfigChoice(sName, sDefault, pvChoices) {this->SetValue(sDefault);};

public:
   bool     SetValue(const char * sValue, size_t iOffset = 0);
};

#endif /*__GCFS_CONFIG_VALUES__*/
