/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <fuse_lowlevel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include <gcfs.h>
#include <gcfs_fuse.h>
#include <gcfs_task.h>
#include <gcfs_config.h>
#include <gcfs_controls.h>
#include <gcfs_utils.h>

static int gcfs_stat(fuse_ino_t ino, struct stat *stbuf)
{
   stbuf->st_ino = ino;
   stbuf->st_mode = 0;

   mode_t iUmask = 0000;
   GCFS_FileSystem * pFile = NULL;

   pFile = GCFS_FileSystem::getInodeFile(ino);
   
   if(pFile)
   {
      switch(pFile->getType())
      {
         case GCFS_FileSystem::eTypeDirectory:
         case GCFS_FileSystem::eTypeTask:
            stbuf->st_mode = S_IFDIR;
            break;
         case GCFS_FileSystem::eTypeLink:
            stbuf->st_mode = S_IFLNK;
            break;
         case GCFS_FileSystem::eTypePhysicalFile:
            stat(((GCFS_File*)pFile)->getPath(), stbuf);
            break;
         default:
            stbuf->st_mode = S_IFREG;
            break;
      }

      GCFS_Permissions sPermissions;
      if(pFile->getPermissions(sPermissions))
      {
         stbuf->st_mode |= sPermissions.m_sMode;
         stbuf->st_uid = sPermissions.m_iUid;
         stbuf->st_gid = sPermissions.m_iGid;
      }
   }
   else
   {
      printf("Error stat inode: %d, \n", (int)ino);
      return -1;
   }



   stbuf->st_nlink = 1;
   stbuf->st_mode &= ~iUmask;

   stbuf->st_mtime = stbuf->st_atime = stbuf->st_ctime = time(NULL);

   return 0;
}

static void gcfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
   struct stat stbuf = {0};

   (void) fi;

   if (gcfs_stat(ino, &stbuf) == -1)
      fuse_reply_err(req, ENOENT);
   else
      fuse_reply_attr(req, &stbuf, GCFS_FUSE_STAT_TIMEOUT);
}

static void gcfs_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi)
{
   struct stat stbuf = {0};

   GCFS_Permissions sPermissions;

   GCFS_FileSystem* pFile = GCFS_FileSystem::getInodeFile(ino);
   
   if(pFile->getType() == GCFS_FileSystem::eTypeTask || ino == FUSE_ROOT_ID)
   {
      if(pFile->getPermissions(sPermissions))
      {
         if(to_set & FUSE_SET_ATTR_MODE)
            sPermissions.m_sMode = attr->st_mode & 0777;
         
         if(to_set & FUSE_SET_ATTR_UID)
            sPermissions.m_iUid = attr->st_uid;
         
         if(to_set & FUSE_SET_ATTR_GID)
            sPermissions.m_iGid = attr->st_gid;
      }
   }

   if (gcfs_stat(ino, &stbuf) == -1)
      fuse_reply_err(req, ENOENT);
   else
      fuse_reply_attr(req, &stbuf, GCFS_FUSE_STAT_TIMEOUT);
}

static void gcfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
   struct fuse_entry_param e = {0};

   GCFS_FileSystem* pParentFile = GCFS_FileSystem::getInodeFile(parent);
   const GCFS_FileSystem* pChild = NULL;
   
   if(pParentFile && (pChild = pParentFile->getChild(name)) != NULL)
   {
      e.ino = pChild->getInode();
   }
   else
   {
      printf("Error lookup: %s, parent: %d\n", name, (int)parent);
      fuse_reply_err(req, ENOENT);
      return;
   }

   e.attr_timeout = 1.0;
   e.entry_timeout = 1.0;
   gcfs_stat(e.ino, &e.attr);
   fuse_reply_entry(req, &e);
}

static void dirbuf_add(fuse_req_t req, std::string &buff , const char *name, fuse_ino_t ino)
{
   struct stat stbuf = {0};

   stbuf.st_ino = ino;

   size_t bufSize = buff.size();

   size_t size = fuse_add_direntry(req, NULL, 0, name, NULL, 0);
   buff.resize(bufSize+size);
   fuse_add_direntry(req, (char*)buff.c_str() + bufSize, size, name, &stbuf, bufSize+size);

}

static int reply_buf_limited(fuse_req_t req, std::string &buff, size_t off, size_t maxsize)
{
   if (off < buff.size())
      return fuse_reply_buf(req, buff.c_str() + off,
                  min(buff.size() - off, maxsize));
   else
      return fuse_reply_buf(req, NULL, 0);
}

static void gcfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi)
{
   GCFS_FileSystem* pFile = GCFS_FileSystem::getInodeFile(ino);
   const GCFS_FileSystem::FileList* mFiles = NULL;
   
   if(pFile && (mFiles = pFile->getChildren()) != NULL)
   {
      std::string buff;
      
      dirbuf_add(req, buff, ".", ino);
      
      if(pFile->getParent())
         dirbuf_add(req, buff, ".", pFile->getParent()->getInode());
      
      GCFS_FileSystem::FileList::const_iterator it;
      for(it = mFiles->begin(); it != mFiles->end(); it++)
         dirbuf_add(req, buff, it->first.c_str(), it->second->getInode());
      
      reply_buf_limited(req, buff, off, size);
   }
   else
   {
      printf("Error readdir: inode: %d\n", (int)ino);
      fuse_reply_err(req, ENOTDIR);
   }
}

static void gcfs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
   fi->fh = 0;

   GCFS_FileSystem *pFile = GCFS_FileSystem::getInodeFile(ino);
   
   if(pFile)
   {
      if(pFile && pFile->open())
      {
         fi->fh = (uint64_t)pFile;
         fi->direct_io = 1;

         fuse_reply_open(req, fi);
      }
      else
         fuse_reply_err(req, EISDIR);
   }
   else
   {
      printf("Error open: inode: %d\n", (int)ino);
      fuse_reply_err(req, EACCES);
   }
}

static void gcfs_close(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
   if(fi->fh)
      ((GCFS_FileSystem *)fi->fh)->close();
}

static void gcfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi)
{
   GCFS_FileSystem *pFile = (GCFS_FileSystem *)fi->fh;
   
   if(pFile)
   {
      fi->direct_io = 1;

      std::string sBuff;
      sBuff.reserve(size);

      size = pFile->read(sBuff, off, size);

      fuse_reply_buf(req, sBuff.c_str(), size);
   }
   else
   {
      printf("Error read: inode: %d\n", (int)ino);
      fuse_reply_err(req, EACCES);
   }
}

static void gcfs_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi)
{
   GCFS_FileSystem *pFile = (GCFS_FileSystem *)fi->fh;
   
   if(pFile)
   {
      fi->direct_io = 1;

      size = pFile->write(buf, off, size);
      
      if(size)
         fuse_reply_write(req, size);
      else
         fuse_reply_err(req, EIO);
   }
   else
   {
      printf("Error read: inode: %d\n", (int)ino);
      fuse_reply_err(req, EACCES);
   }
}

static void gcfs_readlink(fuse_req_t req, fuse_ino_t ino)
{
   GCFS_FileSystem *pFile = GCFS_FileSystem::getInodeFile(ino);
   
   if(pFile && pFile->getType() == GCFS_FileSystem::eTypeLink)
   {
      // TODO: fuse_reply_readlink(req, pFile->);
      return;
   }

   fuse_reply_err(req, EEXIST);
}

static void gcfs_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode)
{
   GCFS_FileSystem *pFile = GCFS_FileSystem::getInodeFile(parent);
   GCFS_Directory * pDirectory = NULL;
   GCFS_Permissions sPermissions;
   
   const fuse_ctx * pContext = fuse_req_ctx(req);
   sPermissions.m_sMode = mode & 0777;
   sPermissions.m_iGid = pContext->gid;
   sPermissions.m_iUid = pContext->uid;
   
   if(pFile && (pDirectory = pFile->mkdir(name, &sPermissions)) != NULL)
   {
      struct fuse_entry_param e = {0};

      e.ino = pDirectory->getInode();
      e.attr_timeout = 1.0;
      e.entry_timeout = 1.0;
      gcfs_stat(e.ino, &e.attr);

      fuse_reply_entry(req, &e);

      return;
   }

   fuse_reply_err(req, EEXIST);
}

static void gcfs_rmdir(fuse_req_t req, fuse_ino_t parent, const char *name)
{
   GCFS_FileSystem *pParent = GCFS_FileSystem::getInodeFile(parent);
   
   if(pParent && pParent->unlink(name))
   {
      fuse_reply_err(req, 0);
      return;
   }

   fuse_reply_err(req, ENOENT);
}

static void gcfs_unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
   GCFS_FileSystem *pParent = GCFS_FileSystem::getInodeFile(parent);
   
   if(pParent && pParent->unlink(name))
   {
      fuse_reply_err(req, 0);
      return;
   }
   
   fuse_reply_err(req, ENOENT);
}

static void gcfs_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi)
{
   GCFS_FileSystem *pParent = GCFS_FileSystem::getInodeFile(parent);
   GCFS_FileSystem *pFile = NULL;

   if(pParent && (pFile = pParent->create(name, GCFS_FileSystem::eTypePhysicalFile)) != NULL)
   {
      struct fuse_entry_param e = {0};

      e.ino = pFile->getInode();

      fi->fh = (uint64_t)pFile;

      gcfs_stat(e.ino, &e.attr);
      
      fuse_reply_create(req, &e, fi);
      return;
   }

   fuse_reply_err(req, EACCES);
}

static void gcfs_mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev)
{

   GCFS_FileSystem *pParent = GCFS_FileSystem::getInodeFile(parent);
   GCFS_FileSystem *pFile = NULL;
   
   if(mode & S_IFREG && pParent && (pFile = pParent->create(name, GCFS_FileSystem::eTypePhysicalFile)) != NULL)
	{
		struct fuse_entry_param e = {0};

		e.ino = pFile->getInode();

		gcfs_stat(e.ino, &e.attr);

		fuse_reply_entry(req, &e);
		return;
	}

	fuse_reply_err(req, EACCES);
}

static void gcfs_statfs(fuse_req_t req, fuse_ino_t ino)
{
	struct statvfs stat = {};

	stat.f_files = 256;
	stat.f_bavail = 0;
	stat.f_bfree = 0;
	stat.f_blocks = 1024;
	stat.f_frsize = 1024;
	stat.f_bsize = 1024;
	stat.f_fsid = ino;
	stat.f_namemax = 256;

	fuse_reply_statfs(req, &stat);
}

static void gcfs_access(fuse_req_t req, fuse_ino_t ino, int mask)
{
	// We are not using custom access_control. Instead we set 'default_permissions' mount option
}

static struct fuse_lowlevel_ops gcfs_oper = {};

int init_fuse(int argc, char *argv[])
{
	gcfs_oper.lookup	= gcfs_lookup;
	gcfs_oper.getattr	= gcfs_getattr;
	gcfs_oper.setattr	= gcfs_setattr;
	gcfs_oper.readdir	= gcfs_readdir;
	gcfs_oper.open		= gcfs_open;
   gcfs_oper.release = gcfs_close;
	gcfs_oper.read		= gcfs_read;
	gcfs_oper.write	= gcfs_write;
	gcfs_oper.mkdir	= gcfs_mkdir;
	gcfs_oper.rmdir	= gcfs_rmdir;
	gcfs_oper.unlink	= gcfs_unlink;
	gcfs_oper.create	= gcfs_create;
	gcfs_oper.mknod	= gcfs_mknod;
	gcfs_oper.statfs	= gcfs_statfs;
	gcfs_oper.access	= gcfs_access;
	gcfs_oper.readlink= gcfs_readlink;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_chan *ch;
	char *mountpoint = NULL;
	int err = -1;

	if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) == -1)
		return 1;

	/* // Leave this commented. Better to be crashing on MAC than not havinh help
	if(!mountpoint)
	{
		printf("No mount point given!\n");
		return 1;
	}*/

	// Copy permisions from mounting directory
	struct stat stBuf;
	stat(mountpoint, &stBuf);
	g_sConfig.m_sPermissions.m_sMode = stBuf.st_mode & 0777;
	g_sConfig.m_sPermissions.m_iUid = getuid();
	g_sConfig.m_sPermissions.m_iGid = getgid();
	
	printf("User: %d, Group: %d\n", getuid(), getgid());

	fuse_opt_add_arg(&args, "-o");
	fuse_opt_add_arg(&args, "default_permissions");
	fuse_opt_add_arg(&args, "-d");
	
	
	if((ch = fuse_mount(mountpoint, &args)) != NULL) {
		struct fuse_session *se;

		g_sConfig.m_sMountDir = mountpoint;

		// Chdir to data directory to avoid "access denied" errors when CWD is not accessible by regular users
		chdir(g_sConfig.m_sDataDir.c_str());
		printf("Moving to data dir: %s\n", g_sConfig.m_sDataDir.c_str());

		se = fuse_lowlevel_new(&args, &gcfs_oper,
				       sizeof(gcfs_oper), NULL);
		
		if (se != NULL) {
			if (fuse_set_signal_handlers(se) != -1) {
				fuse_session_add_chan(se, ch);
				err = fuse_session_loop_mt(se);
				fuse_remove_signal_handlers(se);
				fuse_session_remove_chan(ch);
			}
			fuse_session_destroy(se);
		}
		fuse_unmount(mountpoint, ch);
	}
	fuse_opt_free_args(&args);

	free(mountpoint);

	return err ? 1 : 0;
}
