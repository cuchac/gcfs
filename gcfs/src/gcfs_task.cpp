#include "gcfs_task.h"
#include "gcfs_config.h"
#include <fcntl.h>
#include <stdio.h>
#include <gcfs_controls.h>
#include <string.h>

GCFS_Task::GCFS_Task(const char * sName): m_sName(sName),
	m_iMemory("memory", "1024"),
	m_iProcesses("processes", "1"),
	m_iTimeout("timeout", "3600"),
	m_iService("service", 0, &g_sConfig.m_vServiceNames),
	m_sExecutable("executable", "./data/executable"),
	m_sArguments("arguments", ""),
	m_eStatus(eNew),
	m_sPermissions(),
	m_pServiceData(NULL)
{
	m_vConfigValues.push_back(&m_iMemory);
	m_vConfigValues.push_back(&m_iProcesses);
	m_vConfigValues.push_back(&m_iTimeout);
	m_vConfigValues.push_back(&m_iService);
	m_vConfigValues.push_back(&m_sExecutable);
	m_vConfigValues.push_back(&m_sArguments);

	m_mConfigNameToIndex["memory"] = 0;
	m_mConfigNameToIndex["processes"] = 1;
	m_mConfigNameToIndex["timeout"] = 2;
	m_mConfigNameToIndex["service"] = 3;
	m_mConfigNameToIndex["executable"] = 4;
	m_mConfigNameToIndex["arguments"] = 5;
}

bool GCFS_Task::isFinished()
{
	return m_eStatus == GCFS_Task::eFinished || m_eStatus == GCFS_Task::eAborted || m_eStatus == GCFS_Task::eFailed;
}

bool GCFS_Task::isSubmited()
{
	return m_eStatus != GCFS_Task::eNew;
}

GCFS_Task::File* GCFS_Task::createDataFile(const char * name)
{
	File *pFile;
	
	if(pFile = getDataFile(name))
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

	close(pFile->m_hFile);

	unlink(pFile->m_sPath.c_str());

	g_sTasks.putInode(pFile->m_iInode);

	delete pFile;

	m_mDataFiles.erase(name);

	return true;
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

	if(pFile = getResultFile(name))
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

	close(pFile->m_hFile);

	unlink(pFile->m_sPath.c_str());

	g_sTasks.putInode(pFile->m_iInode);

	delete pFile;

	m_mResultFiles.erase(name);

	return true;
}

GCFS_Task::File* GCFS_Task::getResultFile(const char * name)
{
	GCFS_Task::Files::iterator it;
	if((it = m_mResultFiles.find(name)) != m_mResultFiles.end())
	{
		// If handle not opened, open it
		if(!it->second->m_hFile)
			it->second->m_hFile = open(it->second->m_sPath.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
		
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
	const char * psFileName = basename(m_sExecutable.m_sValue.c_str());

	Files::iterator it;
	if((it = m_mDataFiles.find(psFileName)) == m_mDataFiles.end())
		return NULL;

	return it->second;
}

// Task Manager

GCFS_TaskManager::GCFS_TaskManager():m_uiFirstFileInode(-1)
{

	m_vControls.push_back(new GCFS_ControlControl());
	m_vControls.push_back(new GCFS_ControlStatus());
	m_vControls.push_back(new GCFS_ControlStatus());
	
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
}

GCFS_Task::File* GCFS_TaskManager::getInodeFile(int iInode)
{
	std::map<int, GCFS_Task::File *>::iterator it;
	if((it = m_mInodesOwner.find(iInode)) != m_mInodesOwner.end())
	{
		// If handle not opened, open it
		if(!it->second->m_hFile)
			it->second->m_hFile = open(it->second->m_sPath.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
		
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
		tmp->m_hFile = open(tmp->m_sPath.c_str(), O_CREAT|O_RDWR|O_TRUNC, S_IRUSR | S_IWUSR);

	return tmp;
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