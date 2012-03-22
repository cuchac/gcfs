#include "gcfs_filesystem.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "gcfs.h"
#include "gcfs_config.h"
#include "gcfs_task.h"

ino_t GCFS_FileSystem::s_uiFirstAvailableInode = 1;
GCFS_FileSystem::InodeList GCFS_FileSystem::s_mInodes;

GCFS_FileSystem::GCFS_FileSystem(GCFS_Directory * pParent):m_pParent(pParent)
{
   allocInode();
}

GCFS_FileSystem::~GCFS_FileSystem()
{
   releaseInode();
}

GCFS_FileSystem::EType GCFS_FileSystem::getType()
{
   return eTypeVirtualFile;
}

ssize_t GCFS_FileSystem::read(std::string& sBuffer, off_t uiOffset, size_t uiSize)
{
   printf("Unimplemented read!");
   return false;
}

ssize_t GCFS_FileSystem::write(const char* sBuffer, off_t uiOffset, size_t uiSize)
{
   printf("Unimplemented write!");
   return false;
}

bool GCFS_FileSystem::close()
{
   return true;
}

int GCFS_FileSystem::open()
{
   return true;
}

GCFS_FileSystem* GCFS_FileSystem::create(const char* sName, GCFS_FileSystem::EType eType)
{
   return false;
}

bool GCFS_FileSystem::unlink(const char* sName)
{
   printf("Unimplemented unlink!");
   return false;
}

GCFS_Directory* GCFS_FileSystem::mkdir(const char* name, GCFS_Permissions* pPerm)
{
   return false;
}

off_t GCFS_FileSystem::getSize()
{
   return 0;
}

GCFS_Permissions* GCFS_FileSystem::getPermissions()
{
   if(m_pParent)
      return m_pParent->getPermissions();
   else
      return &g_sConfig.m_sPermissions;
}

const GCFS_FileSystem::FileList* GCFS_FileSystem::getChildren()
{
   return NULL;
}

const GCFS_FileSystem* GCFS_FileSystem::getChild(const char* sName)
{
   return NULL;
}

GCFS_Task* GCFS_FileSystem::getParentTask()
{
   GCFS_Directory * pParent = m_pParent;
   
   while(pParent && !pParent->getType() == eTypeTask)
      pParent = pParent->getParent();
   
   return (GCFS_Task *)pParent;
}

GCFS_Directory* GCFS_FileSystem::getParent()
{
   return m_pParent;
}

ino_t GCFS_FileSystem::getInode() const
{
   return m_iInode;
}

GCFS_FileSystem* GCFS_FileSystem::getInodeFile(ino_t iInode)
{
   GCFS_FileSystem::InodeList::iterator it;
   if ((it = s_mInodes.find(iInode)) != s_mInodes.end())
      return it->second;
   
   return NULL;
}

ino_t GCFS_FileSystem::allocInode()
{
   s_mInodes[s_uiFirstAvailableInode] = this;
   
   m_iInode = s_uiFirstAvailableInode;
   
   return s_uiFirstAvailableInode++;
}

bool GCFS_FileSystem::releaseInode()
{
   GCFS_FileSystem::InodeList::iterator it;
   if ((it = s_mInodes.find(m_iInode)) == s_mInodes.end())
      return false;
   
   s_mInodes.erase(it);
   return true;
}


/***************************************************************************/
GCFS_Directory::GCFS_Directory(GCFS_Directory * pParent): GCFS_FileSystem(pParent)
{
   
}

GCFS_Directory::~GCFS_Directory()
{
   FileList::iterator it;
   for (it = m_mFileList.begin(); it != m_mFileList.end(); it++)
      delete it->second;
   
   m_mFileList.clear();
}

GCFS_FileSystem::EType GCFS_Directory::getType()
{
   return eTypeDirectory;
}

const GCFS_FileSystem::FileList* GCFS_Directory::getChildren()
{
   return &m_mFileList;
}

const GCFS_FileSystem* GCFS_Directory::getChild(const char* sName)
{
   FileList::iterator it;
   if((it = m_mFileList.find(sName)) != m_mFileList.end())
      return it->second;
   
   return NULL;
}

void GCFS_Directory::addChild(GCFS_FileSystem* pFile, const char* sName)
{
   m_mFileList[sName] = pFile;
}

bool GCFS_Directory::removeChild(const char* sName, bool bDelete)
{
   const GCFS_FileSystem* pChild = getChild(sName);
   
   if(pChild)
   {
      m_mFileList.erase(sName);
      if(bDelete)
         delete pChild;
      return true;
   }
   
   return false;
}

GCFS_FileSystem* GCFS_Directory::create(const char* sName, GCFS_FileSystem::EType eType)
{
   if(eType == eTypePhysicalFile)
   {
      GCFS_File *pFile = new GCFS_File(getParent());
      
      // Create file path in data directory
      int iInode = pFile->getInode();
      char buff[32];
      snprintf(buff, sizeof(buff), "%x", iInode);
      
      // COntruct path
      std::string sPath = g_sConfig.m_sDataDir + "/" + buff;
      pFile->setPath(sPath.c_str());
      
      // Ensure no old file exists
      unlink(sPath.c_str());
      
      pFile->open();
      
      chown(sPath.c_str(), getPermissions()->m_iUid, getPermissions()->m_iGid);
      
      addChild(pFile, sName);
      
      return pFile;
   }
   
   return NULL;
}

GCFS_Directory* GCFS_Directory::mkdir(const char* name, GCFS_Permissions* pPerm)
{
   GCFS_Directory * pNewDir = new GCFS_Directory(this);
   
   addChild(pNewDir, name);
   
   return pNewDir;
}

bool GCFS_Directory::unlink(const char* sName)
{
   const GCFS_FileSystem* pFile = getChild(sName);
   
   if(!pFile)
      return false;
   
   return removeChild(sName, true);
}

/***************************************************************************/
GCFS_File::GCFS_File(GCFS_Directory * pParent): GCFS_FileSystem(pParent), m_hFile(0)
{
   
}

GCFS_File::~GCFS_File()
{
   // If handle opened, close it
   close();
   
   ::unlink(m_sPath.c_str());
}

GCFS_FileSystem::EType GCFS_File::getType()
{
   return eTypePhysicalFile;
}

ssize_t GCFS_File::read(std::string& sBuffer, off_t uiOffset, size_t uiSize)
{
   sBuffer.reserve(uiSize);
   
   return ::read(m_hFile, (void*)sBuffer.c_str(), uiSize);
}

ssize_t GCFS_File::write(const char* sBuffer, off_t uiOffset, size_t uiSize)
{
   if(uiOffset > 0)
      lseek(m_hFile, uiOffset, SEEK_SET);
   
   return ::write(m_hFile, sBuffer, uiSize);
}

int GCFS_File::open()
{
   // If handle not opened, open it
   if (!m_hFile)
      m_hFile = ::open(m_sPath.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
   
   return m_hFile;
}
bool GCFS_File::close()
{
   if (m_hFile)
   {
      ::close(m_hFile);
      m_hFile = 0;
      return true;
   }
   return false;
}

off_t GCFS_File::getSize()
{
   return GCFS_FileSystem::getSize();
}

const char * GCFS_File::getPath()
{
   return m_sPath.c_str();
}

void GCFS_File::setPath(const char* sPath)
{
   m_sPath = sPath;
}
