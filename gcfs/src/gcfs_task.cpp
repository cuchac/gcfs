#include "gcfs_task.h"
#include "gcfs_config.h"
#include <fcntl.h>
#include <stdio.h>
#include <gcfs_controls.h>
#include <string.h>
#include <libgen.h>
#include <gcfs_service.h>

GCFS_Task::GCFS_Task(const char * sName): m_sName(sName),
	m_eStatus(eNew),
	m_pServiceData(NULL),

	m_iMemory("memory", "1024"),
	m_iProcesses("processes", "1"),
	m_iTimeout("timeout", "3600"),
	m_iService("service", NULL, &g_sConfig.m_vServiceNames),
	m_sExecutable("executable", "./data/executable"),
	m_sInput("input_file", ""),
	m_sOutput("output_file", "output"),
	m_sError("error_file", "error"),
	m_sArguments("arguments", ""),
	m_sEnvironment("environment"),
	
	m_sPermissions()
{
	assignVariable(&m_iMemory);
	assignVariable(&m_iProcesses);
	assignVariable(&m_iTimeout);
	assignVariable(&m_iService);
	assignVariable(&m_sExecutable);
	assignVariable(&m_sInput);
	assignVariable(&m_sOutput);
	assignVariable(&m_sError);
	assignVariable(&m_sArguments);
	assignVariable(&m_sEnvironment);

	// Set default service
	m_iService.SetValue(g_sConfig.m_sDefaultService.c_str());
}

GCFS_Task::~GCFS_Task()
{
	GCFS_Service* pService = g_sConfig.GetService(m_iService);

	if(pService)
		pService->deleteTask(this);

	// Delete all task files
	Files::iterator it = m_mDataFiles.begin();
	for(it = m_mDataFiles.begin(); it != m_mDataFiles.end(); it++)
		it->second->unlink();
	for(it = m_mResultFiles.begin(); it != m_mResultFiles.end(); it++)
		it->second->unlink();
}

bool GCFS_Task::isFinished()
{
	return m_eStatus == GCFS_Task::eFinished || m_eStatus == GCFS_Task::eAborted || m_eStatus == GCFS_Task::eFailed;
}

bool GCFS_Task::isSubmited()
{
	return m_eStatus != GCFS_Task::eNew;
}

void GCFS_Task::assignVariable(GCFS_ConfigValue* pValue)
{
	m_vConfigValues.push_back(pValue);
	m_mConfigNameToIndex[pValue->m_sName] = m_vConfigValues.size()-1;
	pValue->m_pTask = this;
}

GCFS_Task::File* GCFS_Task::createDataFile(const char * name)
{
	File *pFile;
	
	if((pFile = getDataFile(name)))
		return pFile;
	
	pFile = g_sTasks.createFile();

	m_mDataFiles[name] = pFile;
	m_mInodeToDataFiles[pFile->m_iInode] = pFile;
	pFile->m_pTask = this;

	chown(pFile->m_sPath.c_str(), m_sPermissions.m_iUid, m_sPermissions.m_iGid);

	return pFile;
}

bool GCFS_Task::deleteDataFile(const char * name)
{
	File *pFile = getDataFile(name);

	if(!pFile)
		return false;

	m_mDataFiles.erase(name);
	m_mInodeToDataFiles.erase(pFile->m_iInode);

	return g_sTasks.deleteFile(pFile);
}

GCFS_Task::File* GCFS_Task::getDataFile(const char * name)
{
	GCFS_Task::Files::iterator it;
	if((it = m_mDataFiles.find(name)) != m_mDataFiles.end())
		return it->second;
	else
		return NULL;
}

const GCFS_Task::Files& GCFS_Task::getDataFiles()
{
	return m_mDataFiles;
}

GCFS_Task::File* GCFS_Task::createResultFile(const char * name, bool bCreate)
{
	File *pFile;

	if((pFile = getResultFile(name)))
		return pFile;

	pFile = g_sTasks.createFile(bCreate);

	m_mResultFiles[name] = pFile;
	m_mInodeToResultFiles[pFile->m_iInode] = pFile;
	pFile->m_pTask = this;

	chown(pFile->m_sPath.c_str(), m_sPermissions.m_iUid, m_sPermissions.m_iGid);

	return pFile;
}

bool GCFS_Task::deleteResultFile(const char * name)
{
	File *pFile = getResultFile(name);

	if(!pFile)
		return false;

	m_mResultFiles.erase(name);
	m_mInodeToResultFiles.erase(pFile->m_iInode);

	return g_sTasks.deleteFile(pFile);
}

GCFS_Task::File* GCFS_Task::getResultFile(const char * name)
{
	GCFS_Task::Files::iterator it;
	if((it = m_mResultFiles.find(name)) != m_mResultFiles.end())
	{
		it->second->getHandle();
		
		return it->second;
	}
	else
		return NULL;
}

const GCFS_Task::Files& GCFS_Task::getResultFiles()
{
	return m_mResultFiles;
}

GCFS_Task::File* GCFS_Task::getExecutableFile()
{
	const char * psFileName = basename((char*)m_sExecutable.m_sValue.c_str());

	Files::iterator it;
	if((it = m_mDataFiles.find(psFileName)) == m_mDataFiles.end())
		return NULL;

	return it->second;
}


// GCFS_Task::File class

int GCFS_Task::File::create()
{
	// If handle not opened, open it
	if(!m_hFile)
		m_hFile = open(m_sPath.c_str(), O_CREAT|O_RDWR|O_TRUNC, S_IRUSR | S_IWUSR);

	return m_hFile;
}

void GCFS_Task::File::unlink()
{
	// If handle opened, close it
	closeHandle();

	::unlink(m_sPath.c_str());
}

int GCFS_Task::File::getHandle()
{
	// If handle not opened, open it
	if(!m_hFile)
		m_hFile = open(m_sPath.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

	return m_hFile;
}

void GCFS_Task::File::closeHandle()
{
	if(m_hFile)
	{
		close(m_hFile);
		m_hFile = 0;
	}
}

// Task Manager

GCFS_TaskManager::GCFS_TaskManager():m_uiFirstFileInode(-1)
{

	m_vControls.push_back(new GCFS_ControlControl());
	m_vControls.push_back(new GCFS_ControlStatus());
	m_vControls.push_back(new GCFS_ControlStatus());

	m_vControls[2]->m_sName = "executable";
	
	m_mControlNames["control"] = 0;
	m_mControlNames["executable"] = 1;
	m_mControlNames["status"] = 2;
}

GCFS_Task* GCFS_TaskManager::addTask(const char * sName)
{
	GCFS_Task* pTask = new GCFS_Task(sName);
	m_vTasks.push_back(pTask);
	m_mTaskNames[sName] = m_vTasks.size()-1;
	return pTask;
}

bool GCFS_TaskManager::deleteTask(const char * sName)
{
	int iIndex = -1;
	if((iIndex = getTaskIndex(sName)) >= 0)
	{
		delete m_vTasks[iIndex];
		m_vTasks.erase(m_vTasks.begin()+iIndex);
		m_mTaskNames.erase(sName);
	}
	
	return true;
}

size_t GCFS_TaskManager::getTaskCount()
{
	return (int)m_vTasks.size();
}

GCFS_Task* GCFS_TaskManager::getTask(int iIndex)
{
	return m_vTasks[iIndex];
}

GCFS_Task* GCFS_TaskManager::getTask(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mTaskNames.find(sName)) != m_mTaskNames.end())
		return m_vTasks[it->second];
	else
		return NULL;
}

int GCFS_TaskManager::getTaskIndex(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mTaskNames.find(sName)) != m_mTaskNames.end())
		return it->second;
	else
		return -1;
}

unsigned int GCFS_TaskManager::getInode(GCFS_Task::File* pFile)
{
	m_mInodesOwner[m_uiFirstFileInode] = pFile;
	
	return m_uiFirstFileInode--;
}

bool GCFS_TaskManager::putInode(int iInode)
{
	std::map<int, GCFS_Task::File *>::iterator it;
	if((it = m_mInodesOwner.find(iInode)) == m_mInodesOwner.end())
		return false;

	m_mInodesOwner.erase(it);
	return true;
}

GCFS_Task::File* GCFS_TaskManager::getInodeFile(int iInode)
{
	std::map<int, GCFS_Task::File *>::iterator it;
	if((it = m_mInodesOwner.find(iInode)) != m_mInodesOwner.end())
	{
		it->second->getHandle();
		return it->second;
	}
	else
		return NULL;
}

GCFS_Task::File* GCFS_TaskManager::createFile(bool bCreate)
{
	GCFS_Task::File *tmp = new GCFS_Task::File();

	int iInode = getInode(tmp);
	char buff[32];
	snprintf(buff, sizeof(buff), "%x", iInode);

	tmp->m_sPath = g_sConfig.m_sDataDir+"/"+buff;
	tmp->m_iInode = iInode;

	if(bCreate)
		tmp->create();

	return tmp;
}

bool GCFS_TaskManager::deleteFile(GCFS_Task::File* pFile)
{
	if(!pFile)
		return false;

	pFile->unlink();

	putInode(pFile->m_iInode);

	delete pFile;

	return true;
}

// Control files
size_t GCFS_TaskManager::getControlCount()
{
	return (int)m_vControls.size();
}

GCFS_Control* GCFS_TaskManager::getControl(int iIndex)
{
	return m_vControls[iIndex];
}

GCFS_Control* GCFS_TaskManager::getControl(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mControlNames.find(sName)) != m_mControlNames.end())
		return m_vControls[it->second];
	else
		return NULL;
}

int GCFS_TaskManager::getControlIndex(const char * sName)
{
	std::map<std::string, int>::iterator it;
	if((it = m_mControlNames.find(sName)) != m_mControlNames.end())
		return it->second;
	else
		return -1;
}
