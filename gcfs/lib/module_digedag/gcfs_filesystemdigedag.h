#ifndef GCFS_CONFIGDEPENDSON_H
#define GCFS_CONFIGDEPENDSON_H

#include <gcfs_config_values.h>
#include <gcfs_utils.h>

template <typename T>
class GCFS_ConfigArray : public GCFS_ConfigValue
{
public:
                        GCFS_ConfigArray(GCFS_Directory* pParent);

public:
   virtual bool         PrintValue(std::string& buff);
   virtual bool         SetValue(const char* sValue, size_t iOffset = 0);

public:
   virtual const std::vector<T> & get();

protected:
   virtual const char * getStringFromValue(T& sValue) = 0;
   virtual bool         getValueFromString(const char * sString, T &pValue) = 0;
   
protected:
   std::vector<T>       m_vValues;
};

/***************************************************************************/
template <typename T>
GCFS_ConfigArray<T>::GCFS_ConfigArray(GCFS_Directory* pParent): GCFS_ConfigValue(pParent)
{

}

template <typename T>
bool GCFS_ConfigArray<T>::PrintValue(std::string& buff)
{
   typename std::vector<T>::iterator it = m_vValues.begin();
   for(it = m_vValues.begin(); it != m_vValues.end(); it++)
   {
      buff += getStringFromValue(*it);
      buff += "\n";
   }
   
   return true;
}

template <typename T>
bool GCFS_ConfigArray<T>::SetValue(const char* sValue, size_t iOffset)
{
   std::string value = GCFS_Utils::TrimStr(std::string(sValue));
   
   GCFS_Utils::keyValueArray_t vValues;
   
   if(!GCFS_Utils::ParseConfigString((char*)value.c_str(), vValues))
      return false;

   if(iOffset == 0)
      m_vValues.clear();

   // Check if commands/assignments exists
   for(GCFS_Utils::keyValueArray_t::iterator it = vValues.begin(); it != vValues.end(); it++)
   {
      T sNewVal;
      if(!getValueFromString(sValue, sNewVal))
         return false;
      
      m_vValues.push_back(sNewVal);
   }
      
   return true;
}

template <typename T>
const std::vector< T >& GCFS_ConfigArray<T>::get()
{
   return m_vValues;
}

/***************************************************************************/
class GCFS_ConfigDependsOn : public GCFS_ConfigArray<GCFS_Task*>
{
public:
                     GCFS_ConfigDependsOn(GCFS_Directory* pParent);
   
protected:
   virtual const char *       getStringFromValue(GCFS_Task*& sValue);
   virtual bool               getValueFromString(const char* sString, GCFS_Task* &pValue);
};

#endif // GCFS_CONFIGDEPENDSON_H
