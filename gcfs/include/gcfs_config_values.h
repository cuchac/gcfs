#ifndef __GCFS_CONFIG_VALUES__
#define __GCFS_CONFIG_VALUES__

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

#include "gcfs_filesystem.h"

// Generic configuration values
class GCFS_Task;

class GCFS_ConfigValue : public GCFS_FileSystem
{
public:
                           GCFS_ConfigValue(GCFS_Directory * pParent);
   virtual                ~GCFS_ConfigValue();

public:
   virtual  bool           SetValue(const char * sValue, size_t iOffset = 0) = 0;
   virtual  bool           PrintValue(std::string &buff) = 0;

public:
   size_t         m_iSize;
};

// Specific configuration values
/***************************************************************************/
class GCFS_ConfigInt: public GCFS_ConfigValue
{
public:
               GCFS_ConfigInt(GCFS_Directory * pParent, const char *sDefault);

   bool        SetValue(const char * sValue, size_t iOffset = 0);
   bool        PrintValue(std::string &buff);
   operator    int();

public:
   int         m_iValue;
};

/***************************************************************************/
class GCFS_ConfigString: public GCFS_ConfigValue
{
public:
               GCFS_ConfigString(GCFS_Directory * pParent, const char *sDefault);

   bool        SetValue(const char * sValue, size_t iOffset = 0);
   bool        PrintValue(std::string &buff);
   operator    std::string();

public:
   std::string m_sValue;
};

/***************************************************************************/
class GCFS_ConfigChoice: public GCFS_ConfigValue
{
public:
   typedef std::vector<std::string> choices_t;

public:
               GCFS_ConfigChoice(GCFS_Directory * pParent, const char* sDefault = NULL, GCFS_ConfigChoice::choices_t* pvChoices = NULL);

   bool        SetValue(const char * sValue, size_t iOffset = 0);
   bool        PrintValue(std::string &buff);
   operator    int();

public:
   uint        m_iValue;
   choices_t   m_vChoices;
};

/***************************************************************************/
class GCFS_ConfigEnvironment: public GCFS_ConfigValue
{
public:
               typedef std::map<std::string, std::string> values_t;

public:
               GCFS_ConfigEnvironment(GCFS_Directory * pParent, const char* sDefault = NULL);

   bool        SetValue(const char * sValue, size_t iOffset = 0);
   bool        SetValue(const char* sKey, const char* sValue);
   bool        PrintValue(std::string &buff);
   operator    values_t();

public:
   values_t    m_mValues;
};

/***************************************************************************/
class GCFS_ConfigService: public GCFS_ConfigChoice
{
public:
            GCFS_ConfigService(GCFS_Directory * pParent, const char* sDefault = NULL, GCFS_ConfigChoice::choices_t* pvChoices = NULL);

public:
   bool     SetValue(const char * sValue, size_t iOffset = 0);
};

#endif /*__GCFS_CONFIG_VALUES__*/
