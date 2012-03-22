#include "gcfs_controls.h"
#include "gcfs_config_values.h"
#include "gcfs_task.h"
#include "gcfs_config.h"
#include "gcfs_service.h"
#include "gcfs_utils.h"

#include <string.h>
#include <stdio.h>

GCFS_Control::GCFS_Control(GCFS_Task* pTask): GCFS_FileSystem(pTask)
{
   
}

/***************************************************************************/
const char * GCFS_ControlStatus::statuses[] =
{
   "new",
   "submitted",
   "running",
   "aborted",
   "failed",
   "finished",
   "suspended",
};

GCFS_ControlStatus::GCFS_ControlStatus(GCFS_Task* pTask): GCFS_Control(pTask)
{
   
}

ssize_t GCFS_ControlStatus::read(std::string &sBuffer, off_t uiOffset, size_t uiSize)
{
   GCFS_Task* pTask = getParentTask();
   if(!pTask)
      return 0;
   
   GCFS_Service* pService = g_sConfig.GetService(pTask->m_sConfigDirectory.m_iService.m_iValue);
   int status = (int)pService->getTaskStatus(pTask);
   const std::string id = pService->getTaskId(pTask);
   
   char cbuff[32];
   snprintf(cbuff, ARRAYSIZE(cbuff), "%d\t%s", status, statuses[status]);
   
   sBuffer = cbuff;
   sBuffer += "\n"+id;
   
   sBuffer += "\n";
   return max(0, sBuffer.size() - uiOffset);
}

ssize_t GCFS_ControlStatus::write(const char* sBuffer, off_t uiOffset, size_t uiSize)
{
   return 0;
}

/***************************************************************************/
GCFS_ControlControl::GCFS_ControlControl(GCFS_Task* pTask): GCFS_Control(pTask)
{
   m_vCommands.push_back("start");
   m_vCommands.push_back("start_and_wait");
   m_vCommands.push_back("wait");
   m_vCommands.push_back("abort");
   m_vCommands.push_back("suspend");
}

ssize_t GCFS_ControlControl::read(std::string &sBuffer, off_t uiOffset, size_t uiSize)
{
   for (uint iIndex = 0; iIndex < m_vCommands.size(); iIndex ++)
   {
      if (iIndex > 0)
         sBuffer += ", ";
      
      sBuffer += m_vCommands[iIndex];
   }
   
   sBuffer += "\n";
   return max(0, sBuffer.size() - uiOffset);
}

ssize_t GCFS_ControlControl::write(const char* sBuffer, off_t uiOffset, size_t uiSize)
{
   GCFS_Task* pTask = getParentTask();
   
   if(!pTask)
      return 0;
   
   std::string value = GCFS_Utils::TrimStr(std::string(sBuffer, uiSize));
   
   GCFS_Utils::keyValueArray_t vValues;
   
   if(!GCFS_Utils::ParseConfigString((char*)value.c_str(), vValues))
      return false;
   
   // Check if commands/assignments exists
   for(GCFS_Utils::keyValueArray_t::iterator it = vValues.begin(); it != vValues.end(); it++)
   {
      if(it->second)
      {
         // It is assignment
         if(pTask->getConfigValue(it->first) == NULL)
            return false;
      }
      else
      {
         bool bFound = false;
         for (uint iIndex = 0; iIndex < m_vCommands.size(); iIndex ++)
            if (strcasecmp(m_vCommands[iIndex], it->first) == 0)
            {
               bFound = true;
               continue;
            }
            
            if(!bFound)
               return false;
      }
   }
   
   // Execute commands/assignments
   for(GCFS_Utils::keyValueArray_t::iterator it = vValues.begin(); it != vValues.end(); it++)
   {
      if(it->second)
      {
         // It is assignment
         pTask->getConfigValue(it->first)->SetValue(it->second);
      }
      else
      {
         for (uint iIndex = 0; iIndex < m_vCommands.size(); iIndex ++)
            if (strcasecmp(m_vCommands[iIndex], it->first) == 0)
               if(!executeCommand(pTask, iIndex))
                  return false;
      }
   }
   
   return max(0, uiSize - uiOffset);
}

bool GCFS_ControlControl::executeCommand(GCFS_Task* pTask, int iCommandIndex)
{
   GCFS_Service * pService = g_sConfig.GetService(pTask->m_sConfigDirectory.m_iService.m_iValue);
   
   //Do propper action
   switch (iCommandIndex)
   {
      case eStart:
      {
         if (pTask->isSubmited())
            return false;
         
         if (!pTask->getExecutableFile())
            return false;
         
         return pService->submitTask(pTask);
         break;
      }
      case eStartAndWait:
      {
         if (this->write("start", 0, 6))
         {
            return pService->waitForTask(pTask);
         }
         else
            return false;
         break;
      }
      case eWait:
      {
         if (!pTask->isSubmited() || pTask->isFinished())
            return false;
         
         return pService->waitForTask(pTask);
         break;
      }
      case eAbort:
      {
         if (!pTask->isSubmited() || pTask->isFinished())
            return false;
         
         return pService->abortTask(pTask);
         break;
      }
      case eSuspend:
         break;
   }
   
   return false;
}
