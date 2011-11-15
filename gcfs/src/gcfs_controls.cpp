#include "gcfs_controls.h"
#include "gcfs_config_values.h"
#include "gcfs_task.h"
#include "gcfs_config.h"
#include "gcfs_service.h"
#include "gcfs_utils.h"

#include <string.h>
#include <stdio.h>

std::string GCFS_Control::trimStr(const std::string& Src, const std::string& c)
{
   return GCFS_ConfigValue::TrimStr(Src, c);
}

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

bool GCFS_ControlStatus::read(GCFS_Task* pTask, std::string &buff)
{
   GCFS_Service* pService = g_sConfig.GetService(pTask->m_iService.m_iValue);
   int status = (int)pService->getTaskStatus(pTask);
   const std::string id = pService->getTaskId(pTask);

   char cbuff[32];
   snprintf(cbuff, ARRAYSIZE(cbuff), "%d\t%s", status, statuses[status]);

   buff = cbuff;
   buff += "\n"+id;

   return true;
}

bool GCFS_ControlStatus::write(GCFS_Task* pTask, const char * sValue)
{
   return false;
}

GCFS_ControlControl::GCFS_ControlControl(): GCFS_Control("control")
{
   m_vCommands.push_back("start");
   m_vCommands.push_back("start_and_wait");
   m_vCommands.push_back("wait");
   m_vCommands.push_back("abort");
   m_vCommands.push_back("suspend");
}

bool GCFS_ControlControl::read(GCFS_Task* pTask, std::string &buff)
{
   for (uint iIndex = 0; iIndex < m_vCommands.size(); iIndex ++)
   {
      if (iIndex > 0)
         buff += ", ";

      buff += m_vCommands[iIndex];
   }

   return true;
}

bool GCFS_ControlControl::write(GCFS_Task* pTask, const char * sValue)
{
   std::string value = trimStr(sValue);

   GCFS_ConfigValue::keyValueArray_t vValues;
   
   if(!GCFS_ConfigValue::ParseConfigString((char*)value.c_str(), vValues))
      return false;

   // Check if commands/assignments exists
   for(GCFS_ConfigValue::keyValueArray_t::iterator it = vValues.begin(); it != vValues.end(); it++)
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
   for(GCFS_ConfigValue::keyValueArray_t::iterator it = vValues.begin(); it != vValues.end(); it++)
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

   return true;
}

bool GCFS_ControlControl::executeCommand(GCFS_Task* pTask, int iCommandIndex)
{
   
   GCFS_Service * pService = g_sConfig.GetService(pTask->m_iService.m_iValue);
   
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
         if (this->write(pTask, "start"))
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
