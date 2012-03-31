#include "gcfs_filesystem.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <stdio.h>

#include "gcfs.h"
#include "gcfs_config.h"
#include "gcfs_task.h"
#include <gcfs_utils.h>

// RootDir has inode=1 and is allocated first 
ino_t GCFS_FileSystem::s_uiFirstAvailableInode = 1;
GCFS_FileSystem::InodeList GCFS_FileSystem::s_mInodes;

GCFS_FileSystem::GCFS_FileSystem(GCFS_Directory * pParent):m_pParent(pParent)
{
   if(getParentTask())
      allocInode();
}

GCFS_FileSystem::~GCFS_FileSystem()
{
   if(getParentTask())
      releaseInode();
}

GCFS_FileSystem::EType GCFS_FileSystem::getType() const
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

GCFS_Directory* GCFS_FileSystem::mkdir(const char* name, const GCFS_Permissions& pPerm)
{
   return false;
}

off_t GCFS_FileSystem::getSize()
{
   return 0;
}

bool GCFS_FileSystem::getPermissions(GCFS_Permissions& sPermissions)
{
   if(m_pParent)
      return m_pParent->getPermissions(sPermissions);

   sPermissions = g_sConfig.m_sPermissions;
   return true;
}

bool GCFS_FileSystem::setPermissions(GCFS_Permissions& sPermissions)
{
   return false;
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
   
   while(pParent && pParent->getType() != eTypeTask)
      pParent = pParent->getParent();
   
   return (GCFS_Task *)pParent;
}

GCFS_Directory* GCFS_FileSystem::getTopmostDirectory()
{
   GCFS_Directory *pLastTask = (GCFS_Directory*)this, *pParent = m_pParent;
   
   while(pParent && pParent->getParent())
   {
      pLastTask = pParent;
      pParent = pParent->getParent();
   }

   return pLastTask;
}

GCFS_Directory* GCFS_FileSystem::getParent()
{
   return m_pParent;
}

const char* GCFS_FileSystem::getName()
{
   GCFS_Directory * pParent;
   if((pParent = getParent()) != NULL)
   {
      const FileList* pFiles = pParent->getChildren();
      FileList::const_iterator it;
      for(it = pFiles->begin(); it != pFiles->end(); it++)
         if(it->second == this)
            return it->first.c_str();
   }
   
   return NULL;
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

GCFS_FileSystem::EType GCFS_Directory::getType() const
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
      if(bDelete)
         delete pChild;
      m_mFileList.erase(sName);
      return true;
   }
   
   return false;
}

GCFS_FileSystem* GCFS_Directory::create(const char* sName, GCFS_FileSystem::EType eType)
{
   if(eType == eTypePhysicalFile)
   {
      GCFS_File *pFile = new GCFS_File(this);
      
      // Create file path in data directory
      std::ostringstream os;
      os << g_sConfig.m_sDataDir << "/" << pFile->getInode();
      pFile->setPath(os.str().c_str());
      
      // Ensure no old file exists
      GCFS_Utils::rmdirRecursive(pFile->getPath());
      remove(pFile->getPath());

      GCFS_Permissions sPermissions;
      getPermissions(sPermissions);
      
      chown(pFile->getPath(), sPermissions.m_iUid, sPermissions.m_iGid);
      
      addChild(pFile, sName);
      
      return pFile;
   }
   
   return NULL;
}

GCFS_Directory* GCFS_Directory::mkdir(const char* name, const GCFS_Permissions& pPerm)
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

GCFS_FileSystem::EType GCFS_File::getType() const
{
   return eTypePhysicalFile;
}

ssize_t GCFS_File::read(std::string& sBuffer, off_t uiOffset, size_t uiSize)
{
   sBuffer.reserve(uiSize);
   
   if(uiOffset >= 0)
      lseek(m_hFile, uiOffset, SEEK_SET);
   
   return ::read(m_hFile, (void*)sBuffer.c_str(), uiSize);
}

ssize_t GCFS_File::write(const char* sBuffer, off_t uiOffset, size_t uiSize)
{
   if(uiOffset >= 0)
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

/***************************************************************************/
GCFS_Link::GCFS_Link(GCFS_Directory* pParent): GCFS_FileSystem(pParent)
{
   
}

GCFS_FileSystem::EType GCFS_Link::getType() const
{
   return GCFS_FileSystem::eTypeLink;
}
