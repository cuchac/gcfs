#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <stdio.h>

#include "gcfs_task.h"
#include "gcfs_config.h"
#include <gcfs_controls.h>
#include <gcfs_service.h>


GCFS_ConfigDirectory::GCFS_ConfigDirectory(GCFS_Task* pTask):
   GCFS_Directory(pTask),
   m_iMemory(pTask, "1024"),
   m_iProcesses(pTask, "1"),
   m_iTimeout(pTask, "3600"),
   m_iService(pTask, NULL, &g_sConfig.m_vServiceNames),
   m_sExecutable(pTask, "./data/executable"),
   m_sInput(pTask, ""),
   m_sOutput(pTask, "output"),
   m_sError(pTask, "error"),
   m_sArguments(pTask, ""),
   m_sEnvironment(pTask)
{
   addChild(&m_iMemory, "memory");
   addChild(&m_iProcesses, "processes");
   addChild(&m_iTimeout, "timeout");
   addChild(&m_iService, "service");
   addChild(&m_sExecutable, "executable");
   addChild(&m_sInput, "input_file");
   addChild(&m_sOutput, "output_file");
   addChild(&m_sError, "error_file");
   addChild(&m_sArguments, "arguments");
   addChild(&m_sEnvironment, "environment");
   
   // Set default service
   m_iService.SetValue(g_sConfig.m_sDefaultService.c_str());
}

GCFS_ConfigDirectory::~GCFS_ConfigDirectory()
{
   removeChild("memory", false);
   removeChild("processes", false);
   removeChild("timeout", false);
   removeChild("service", false);
   removeChild("executable", false);
   removeChild("input_file", false);
   removeChild("output_file", false);
   removeChild("error_file", false);
   removeChild("arguments", false);
   removeChild("environment", false);
}

/***************************************************************************/
GCFS_RootDirectory::GCFS_RootDirectory(GCFS_Directory* pParent): GCFS_Directory(pParent)
{
   
}
GCFS_Directory* GCFS_RootDirectory::mkdir(const char* sName, GCFS_Permissions* pPerm)
{
   GCFS_Task* pTask = new GCFS_Task(this);
   pTask->m_sPermissions = *pPerm;
   
   this->addChild(pTask, sName);
   
   return pTask;
}

/***************************************************************************/
GCFS_Task::GCFS_Task(GCFS_Directory * pParent): 
   GCFS_Directory(pParent),
   m_eStatus(eNew),
   m_pServiceData(NULL),
   m_sPermissions(),
   
   m_sControl(this),
   m_sStatus(this),
   m_sConfigDirectory(this),
   m_sDataDir(this),
   m_sResultDir(this)
{
   addChild(&m_sControl, "control");
   //addChild(&m_sControl, "executable");
   addChild(&m_sStatus, "status");
}

GCFS_Task::~GCFS_Task()
{
   GCFS_Service* pService = g_sConfig.GetService(m_sConfigDirectory.m_iService);

   if (pService)
      pService->deleteTask(this);
   
   removeChild( "control", false);
   //removeChild("executable");
   removeChild("status", false);
}

GCFS_FileSystem::EType GCFS_Task::getType()
{
   return eTypeTask;
}

bool GCFS_Task::isFinished()
{
   return m_eStatus == GCFS_Task::eFinished || m_eStatus == GCFS_Task::eAborted || m_eStatus == GCFS_Task::eFailed;
}

bool GCFS_Task::isSubmited()
{
   return m_eStatus != GCFS_Task::eNew;
}

GCFS_ConfigValue* GCFS_Task::getConfigValue(const char * sName)
{
   return (GCFS_ConfigValue*)m_sConfigDirectory.getChild(sName);
}

const GCFS_FileSystem::FileList* GCFS_Task::getConfigValues()
{
   return m_sConfigDirectory.getChildren();
}

GCFS_File* GCFS_Task::createDataFile(const char * name)
{
   GCFS_File *pFile;

   if ((pFile = getDataFile(name)))
      return pFile;
   
   return (GCFS_File*)m_sDataDir.create(name, GCFS_FileSystem::eTypePhysicalFile);
}

bool GCFS_Task::deleteDataFile(const char * name)
{
   GCFS_File *pFile = getDataFile(name);

   if (!pFile)
      return false;

   delete pFile;

   return true;
}

GCFS_File* GCFS_Task::getDataFile(const char * name)
{
   return (GCFS_File*)m_sDataDir.getChild(name);
}

const GCFS_FileSystem::FileList* GCFS_Task::getDataFiles()
{
   return m_sDataDir.getChildren();
}

GCFS_File* GCFS_Task::createResultFile(const char * name, bool bCreate)
{
   GCFS_File *pFile;
   
   if ((pFile = getDataFile(name)))
      return pFile;
   
   pFile = (GCFS_File*)m_sResultDir.create(name, GCFS_FileSystem::eTypePhysicalFile);
   
   if(bCreate)
      pFile->open();

   return pFile;
}

bool GCFS_Task::deleteResultFile(const char * name)
{
   GCFS_File *pFile = getResultFile(name);
   
   if (!pFile)
      return false;
   
   delete pFile;
   
   return true;
}

GCFS_File* GCFS_Task::getResultFile(const char * name)
{
   return (GCFS_File*)m_sResultDir.getChild(name);
}

const GCFS_FileSystem::FileList* GCFS_Task::getResultFiles()
{
   return m_sResultDir.getChildren();
}

GCFS_File* GCFS_Task::getExecutableFile()
{
   const char * psFileName = basename((char*)m_sConfigDirectory.m_sExecutable.m_sValue.c_str());

   return (GCFS_File*)m_sDataDir.getChild(psFileName);
}


// Task Manager
/***************************************************************************/
GCFS_TaskManager::GCFS_TaskManager():
   m_pRootDirectory(NULL)
{
   
}

GCFS_TaskManager::~GCFS_TaskManager()
{

}

void GCFS_TaskManager::Init()
{
   m_pRootDirectory = new GCFS_Directory(NULL);
}


GCFS_Task* GCFS_TaskManager::addTask(const char * sName, GCFS_Directory * pParent)
{
   if(!pParent)
      pParent = m_pRootDirectory;

   return (GCFS_Task*)pParent->mkdir(sName, &g_sConfig.m_sPermissions);
}

bool GCFS_TaskManager::deleteTask(const char * sName, GCFS_Directory * pParent)
{
   if(!pParent)
      pParent = m_pRootDirectory;

   return pParent->unlink(sName);
}

GCFS_Task* GCFS_TaskManager::getTask(const char * sName, GCFS_Directory * pParent)
{
   if(!pParent)
      pParent = m_pRootDirectory;
   
   return (GCFS_Task*)pParent->getChild(sName);
}
