#ifndef GCFS_TASK_H
#define GCFS_TASK_H

#include "gcfs.h"
#include "gcfs_config_values.h"
#include "gcfs_controls.h"

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>

class GCFS_Control;

// Task Configuration Directory
class GCFS_ConfigDirectory : public GCFS_Directory
{
public:
                                      GCFS_ConfigDirectory(GCFS_Task * pTask);
    virtual                          ~GCFS_ConfigDirectory();
    
public:
   GCFS_ConfigInt*                     m_piMemory;
   GCFS_ConfigInt*                     m_piProcesses;
   GCFS_ConfigInt*                     m_piTimeout;
   GCFS_ConfigService*                 m_piService;
   GCFS_ConfigString*                  m_psExecutable;
   GCFS_ConfigString*                  m_psInput;
   GCFS_ConfigString*                  m_psOutput;
   GCFS_ConfigString*                  m_psError;
   GCFS_ConfigString*                  m_psArguments;
   GCFS_ConfigEnvironment*             m_psEnvironment;
   
public:
   GCFS_ConfigValue*                  getConfigValue(const char * sName);
   const GCFS_FileSystem::FileList*   getConfigValues();
};

// Root Directory
class GCFS_RootDirectory : public GCFS_Directory
{
public:
                                       GCFS_RootDirectory(GCFS_Directory* pParent);

public:
    virtual GCFS_Directory*            mkdir(const char* name, GCFS_Permissions* pPerm);
};

// Task Configuration
class GCFS_Task : public GCFS_RootDirectory 
{
    
public:
   // Define task statuses
   typedef enum {
      eNew = 0, // initial
      eSubmitted, // After start
      eRunning, // Really physically running
      eAborted, // User aborted
      eFailed, // Failed
      eFinished, // succesfully finished
      eSuspended // suspended
   } Status;

public:
                                       GCFS_Task(GCFS_Directory * pParent);
                                      ~GCFS_Task();
                                      
   virtual EType                       getType();
public:
   bool                                isFinished();
   bool                                isSubmited();

public:
   Status                              m_eStatus;
   void*                               m_pServiceData; // Data space for service
   GCFS_Permissions                    m_sPermissions;
   
// Control files
public:
   GCFS_ControlControl*                m_pControl;
   GCFS_ControlStatus*                 m_pStatus;

// Configuration directory
public:
   GCFS_ConfigValue*                   getConfigValue(const char * sName);
   const GCFS_FileSystem::FileList*    getConfigValues();
   GCFS_ConfigDirectory*               m_pConfigDirectory;

// Task Data and Result directories
public:
   GCFS_File*                          createDataFile(const char * name);
   bool                                deleteDataFile(const char * name);
   GCFS_File*                          getDataFile(const char * name);
   const GCFS_FileSystem::FileList*    getDataFiles();
   GCFS_File*                          createResultFile(const char* name, bool bCreate = true);
   bool                                deleteResultFile(const char * name);
   GCFS_File*                          getResultFile(const char * name);
   const GCFS_FileSystem::FileList*    getResultFiles();

   GCFS_File*                          getExecutableFile();

protected:
   GCFS_Directory*                     m_pDataDir;
   GCFS_Directory*                     m_pResultDir;
   
// Executable symlink
public:
   class ExecutableSymlink : public GCFS_Link {
   public:
        ExecutableSymlink(GCFS_Directory* pParent);
   public:
        virtual ssize_t read(std::string& sBuffer, off_t uiOffset, size_t uiSize);
        virtual ssize_t write(const char* sBuffer, off_t uiOffset, size_t uiSize);
   };
   ExecutableSymlink*                  m_pExecutable;
};

class GCFS_TaskManager 
{
public:
                                       GCFS_TaskManager();
                                      ~GCFS_TaskManager();

public:
   void                                Init();

public:
   GCFS_Directory*                     m_pRootDirectory;
   
// Manage tasks
public:
   GCFS_Task*                          addTask(const char* sName, GCFS_Directory* pParent = NULL);
   bool                                deleteTask(const char* sName, GCFS_Directory* pParent = NULL);

   GCFS_Task*                          getTask(const char* sName, GCFS_Directory* pParent = NULL);
};

#endif // GCFS_TASK_H
