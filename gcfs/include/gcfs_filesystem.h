#ifndef GCFS_FILESYSTEM_H
#define GCFS_FILESYSTEM_H

#include <string>
#include <map>
#include <sys/stat.h>

struct GCFS_Permissions;
class GCFS_Task;
class GCFS_Directory;
class GCFS_File;

class GCFS_FileSystem
{
public:
                              GCFS_FileSystem(GCFS_Directory * pParent);
   virtual                   ~GCFS_FileSystem();
   
// Light-weight RTTI
public:
   typedef enum {
      eTypeVirtualFile,
      eTypePhysicalFile,
      eTypeDirectory,
      eTypeTask,
      eTypeLink
   } EType;
   
   virtual EType              getType() const;
   
public:
   typedef std::map<std::string, GCFS_FileSystem*> FileList;
   
public:
   // Low level filesystem calls
   virtual ssize_t            read(std::string &sBuffer, off_t uiOffset, size_t uiSize);
   virtual ssize_t            write(const char * sBuffer, off_t uiOffset, size_t uiSize);
   virtual int                open();
   virtual GCFS_FileSystem*   create(const char * sName, EType eType);
   virtual bool               close();
   virtual bool               unlink(const char * sName);
   virtual GCFS_Directory*    mkdir(const char *name, GCFS_Permissions* pPerm);
   
   virtual off_t              getSize();
   virtual bool               getPermissions(GCFS_Permissions &sPermissions);
   virtual bool               setPermissions(GCFS_Permissions &sPermissions);
   virtual const FileList*    getChildren();
   virtual const GCFS_FileSystem*    getChild(const char * sName);
   
public:
   virtual GCFS_Task*         getParentTask();
   virtual GCFS_Directory*    getParent();
   virtual const char*        getName();
   virtual ino_t              getInode() const;
   
// Inode allocation and management
public:
   static GCFS_FileSystem*    getInodeFile(ino_t iInode);
   
protected:
   ino_t                      allocInode();
   bool                       releaseInode();
   
private:
   typedef std::map<ino_t, GCFS_FileSystem*> InodeList;
   static ino_t               s_uiFirstAvailableInode;
   static InodeList           s_mInodes;
   
protected:
   GCFS_Directory*            m_pParent;
   ino_t                      m_iInode;
};

/***************************************************************************/
class GCFS_Directory : public GCFS_FileSystem
{
public:
                              GCFS_Directory(GCFS_Directory * pParent);
   virtual                   ~GCFS_Directory();
   
public:
   virtual EType              getType() const;
   
public:
   virtual const FileList*    getChildren();
   virtual const GCFS_FileSystem*    getChild(const char * sName);
   void                       addChild(GCFS_FileSystem* pFile, const char* sName);
   bool                       removeChild(const char* sName, bool bDelete = true);
   
   virtual GCFS_FileSystem*   create(const char* sName, EType eType);
   virtual GCFS_Directory*    mkdir(const char* name, GCFS_Permissions* pPerm);
   virtual bool               unlink(const char * sName);
   
private:
   FileList                   m_mFileList;
};

/***************************************************************************/
class GCFS_File : public GCFS_FileSystem
{
public:
                              GCFS_File(GCFS_Directory * pParent);
    virtual                  ~GCFS_File();
public:
   virtual EType              getType() const;
   
public:
   virtual ssize_t            read(std::string &sBuffer, off_t uiOffset, size_t uiSize);
   virtual ssize_t            write(const char * sBuffer, off_t uiOffset, size_t uiSize);
   virtual off_t              getSize();
   virtual int                open();
   virtual bool               close();
   
public:
   const char *               getPath();
   void                       setPath(const char * sPath);
   
private:
   std::string                m_sPath;
   int                        m_hFile;
};

/***************************************************************************/
class GCFS_Link : public GCFS_FileSystem
{
public:
                              GCFS_Link(GCFS_Directory * pParent);

public:
   virtual EType              getType() const;
   
public:
   virtual ssize_t            read(std::string &sBuffer, off_t uiOffset, size_t uiSize) = 0;
   virtual ssize_t            write(const char* sBuffer, off_t uiOffset, size_t uiSize) = 0;
};

#endif // GCFS_FILESYSTEM_H
