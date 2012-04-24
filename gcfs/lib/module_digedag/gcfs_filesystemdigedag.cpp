#include "gcfs_filesystemdigedag.h"
#include <gcfs_task.h>

GCFS_ConfigDependsOn::GCFS_ConfigDependsOn(GCFS_Directory* pParent): GCFS_ConfigArray<GCFS_Task*>(pParent)
{
   m_bIsSet = true;
}

const char* GCFS_ConfigDependsOn::getStringFromValue(GCFS_Task*& sValue)
{
   return sValue->getName();
}

bool GCFS_ConfigDependsOn::getValueFromString(const char* sString, GCFS_Task*& pValue)
{
   pValue = g_sTaskManager.getTask(sString, getParent(), true);

   if(pValue == getParentTask())
      return NULL;
   
   return pValue != NULL;
}

bool GCFS_ConfigDependsOn::removeTask(GCFS_Task* pTask)
{
   std::vector< GCFS_Task* >::iterator itDepend;
   for(itDepend = m_vValues.begin(); itDepend != m_vValues.end();)
      if(*itDepend == pTask)
         itDepend = m_vValues.erase(itDepend);
      else
         itDepend++;

   return true;
}
