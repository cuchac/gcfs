#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <stdio.h>

#include "gcfs_task.h"
#include "gcfs_config.h"
#include <gcfs_controls.h>
#include <gcfs_service.h>
#include <gcfs_utils.h>


GCFS_ConfigDirectory::GCFS_ConfigDirectory(GCFS_Task* pTask):
   GCFS_Directory(pTask)
{
   m_piMemory = new GCFS_ConfigInt(this, "1024");
   m_piProcesses = new GCFS_ConfigInt(this, "1");
   m_piTimeout = new GCFS_ConfigInt(this, "3600");
   m_piService = new GCFS_ConfigService(this, NULL, &g_sConfig.m_vServiceNames);
   m_psExecutable = new GCFS_ConfigString(this, "./data/executable");
   m_psInput = new GCFS_ConfigString(this, "");
   m_psOutput = new GCFS_ConfigString(this, "output");
   m_psError = new GCFS_ConfigString(this, "error");
   m_psArguments = new GCFS_ConfigString(this, "");
   m_psEnvironment = new GCFS_ConfigEnvironment(this);

   addChild(m_piMemory, "memory");
   addChild(m_piProcesses, "processes");
   addChild(m_piTimeout, "timeout");
   addChild(m_piService, "service");
   addChild(m_psExecutable, "executable");
   addChild(m_psInput, "input_file");
   addChild(m_psOutput, "output_file");
   addChild(m_psError, "error_file");
   addChild(m_psArguments, "arguments");
   addChild(m_psEnvironment, "environment");
   
   // Set default service
   m_piService->SetValue(g_sConfig.m_sDefaultService.c_str());
}

GCFS_ConfigDirectory::~GCFS_ConfigDirectory()
{
   
}

GCFS_ConfigValue* GCFS_ConfigDirectory::getConfigValue(const char * sName)
{
   return (GCFS_ConfigValue*)getChild(sName);
}

const GCFS_FileSystem::FileList* GCFS_ConfigDirectory::getConfigValues()
{
   return getChildren();
}

/***************************************************************************/
GCFS_RootDirectory::GCFS_RootDirectory(GCFS_Directory* pParent): GCFS_Directory(pParent)
{
   allocInode();
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
   GCFS_RootDirectory(pParent),
   m_eStatus(eNew),
   m_pServiceData(NULL),
   m_sPermissions()
{
   m_pControl = new GCFS_ControlControl(this);
   m_pStatus = new GCFS_ControlStatus(this);
   m_pConfigDirectory = new GCFS_ConfigDirectory(this);
   m_pDataDir = new GCFS_Directory(this);
   m_pResultDir = new GCFS_Directory(this);
   m_pExecutable = new GCFS_Task::ExecutableSymlink(this);
   
   addChild(m_pControl, "control");
   addChild(m_pExecutable, "executable");
   addChild(m_pStatus, "status");
   
   addChild(m_pConfigDirectory, "config");
   addChild(m_pDataDir, "data");
   addChild(m_pResultDir, "result");
}

GCFS_Task::~GCFS_Task()
{
   GCFS_Service* pService = g_sConfig.GetService(m_pConfigDirectory->m_piService->get());

   if (pService)
      pService->deleteTask(this);
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
   return m_pConfigDirectory->getConfigValue(sName);
}

const GCFS_FileSystem::FileList* GCFS_Task::getConfigValues()
{
   return m_pConfigDirectory->getConfigValues();
}

GCFS_File* GCFS_Task::createDataFile(const char * name)
{
   GCFS_File *pFile;

   if ((pFile = getDataFile(name)))
      return pFile;

   pFile = (GCFS_File*)m_pDataDir->create(name, GCFS_FileSystem::eTypePhysicalFile);
   pFile->open();
   
   return pFile;
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
   return (GCFS_File*)m_pDataDir->getChild(name);
}

const GCFS_FileSystem::FileList* GCFS_Task::getDataFiles()
{
   return m_pDataDir->getChildren();
}

GCFS_File* GCFS_Task::createResultFile(const char * name, bool bCreate)
{
   GCFS_File *pFile;
   
   if ((pFile = getResultFile(name)))
      return pFile;
   
   pFile = (GCFS_File*)m_pResultDir->create(name, GCFS_FileSystem::eTypePhysicalFile);
   
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
   return (GCFS_File*)m_pResultDir->getChild(name);
}

const GCFS_FileSystem::FileList* GCFS_Task::getResultFiles()
{
   return m_pResultDir->getChildren();
}

GCFS_File* GCFS_Task::getExecutableFile()
{
   const char * psFileName = basename((char*)m_pConfigDirectory->m_psExecutable->get());

   return (GCFS_File*)m_pDataDir->getChild(psFileName);
}

// Executable symlink
/***************************************************************************/
GCFS_Task::ExecutableSymlink::ExecutableSymlink(GCFS_Directory* pParent): GCFS_Link(pParent)
{
   
}

ssize_t GCFS_Task::ExecutableSymlink::read(std::string& sBuffer, off_t uiOffset, size_t uiSize)
{
   ssize_t sRet = getParentTask()->getConfigValue("executable")->read(sBuffer, uiOffset, uiSize);
   sBuffer = GCFS_Utils::TrimStr(sBuffer);
   return sRet;
}

ssize_t GCFS_Task::ExecutableSymlink::write(const char* sBuffer, off_t uiOffset, size_t uiSize)
{
   return 0;
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
   m_pRootDirectory = new GCFS_RootDirectory(NULL);
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
