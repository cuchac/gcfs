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
#include <gcfs.h>
#include <gcfs_fuse.h>
#include <gcfs_task.h>
#include <gcfs_config.h>
#include <time.h>
#include <gcfs_controls.h>

static int gcfs_stat(fuse_ino_t ino, struct stat *stbuf)
{
	stbuf->st_ino = ino;
	stbuf->st_mode = 0;

	uint iIndex = GCFS_INDEX_FROM_INODE(ino);
	uint iTaskIndex = GCFS_TASK_FROM_INODE(ino);
	
	mode_t iUmask = 0000;
	GCFS_Permissions *pPermission = NULL;

	if(ino == FUSE_ROOT_ID) {
		stbuf->st_mode = S_IFDIR;
		pPermission = &g_sConfig.m_sPermissions;
	}
	else if(ino > g_sTaskManager.m_uiFirstFileInode)
	{
		stat(g_sTaskManager.getInodeFile(ino)->m_sPath.c_str(), stbuf);
		stbuf->st_ino = ino;
		stbuf->st_mode = S_IFREG;
		pPermission = &g_sTaskManager.getInodeFile(ino)->m_pTask->m_sPermissions;
	}
	else
	{
		if(ino > GCFS_FUSE_INODES_PER_TASK * g_sTaskManager.getTaskCount())
			return -1;


		if(GCFS_IS_DIRINODE(iIndex)) // Task dir
		{
			stbuf->st_mode = S_IFDIR;
		}
		else if(iIndex == GCFS_CONTROL_EXECUTABLE)
		{
			stbuf->st_mode = S_IFLNK;
		}
		else if(GCFS_IS_CONFIGINODE(iIndex) || GCFS_IS_CONTROLINODE(iIndex))
		{
			stbuf->st_mode = S_IFREG;
			iUmask = 07000 | S_IXGRP | S_IXUSR | S_IXOTH; // Remove execute permissions
		}
		else
		{
			printf("Error stat inode: %d, \n", (int)ino);
			return -1;
		}

		pPermission = &g_sTaskManager.getTask(iTaskIndex)->m_sPermissions;
	}

	if(pPermission)
	{
		stbuf->st_mode |= pPermission->m_sMode;
		stbuf->st_uid = pPermission->m_iUid;
		stbuf->st_gid = pPermission->m_iGid;
	}

	stbuf->st_nlink = 1;
	stbuf->st_mode &= ~iUmask;

	stbuf->st_mtime = stbuf->st_atime = stbuf->st_ctime = time(NULL);

	return 0;
}

static void gcfs_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi)
{
	struct stat stbuf = {0};

	(void) fi;

	if (gcfs_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, GCFS_FUSE_STAT_TIMEOUT);
}

static void gcfs_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr,
			 int to_set, struct fuse_file_info *fi)
{
	struct stat stbuf = {0};

	uint iTaskIndex = GCFS_TASK_FROM_INODE(ino);
    
    GCFS_Permissions *pPermissions = NULL;
    
    if(ino == FUSE_ROOT_ID)
        pPermissions = &g_sConfig.m_sPermissions;
    else if(ino < g_sTaskManager.m_uiFirstFileInode)
        pPermissions = &g_sTaskManager.getTask(iTaskIndex)->m_sPermissions;
	
    if(pPermissions)
    {
        if(to_set & FUSE_SET_ATTR_MODE)
            pPermissions->m_sMode = attr->st_mode & 0777;
        
        if(to_set & FUSE_SET_ATTR_UID)
            pPermissions->m_iUid = attr->st_uid;
        
        if(to_set & FUSE_SET_ATTR_GID)
            pPermissions->m_iGid = attr->st_gid;
    }
    
	if (gcfs_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, GCFS_FUSE_STAT_TIMEOUT);
}

static void gcfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e = {0};

	GCFS_Task* pTask = NULL;
	uint iIndex = -1;
	
	int iParentIndex = GCFS_INDEX_FROM_INODE(parent);
	uint iTaskIndex = GCFS_TASK_FROM_INODE(parent);

	if ((parent == FUSE_ROOT_ID) && (g_sTaskManager.getTask(name) != NULL))
	{
		e.ino = GCFS_DIRINODE(g_sTaskManager.getTaskIndex(name), GCFS_DIR_TASK);
	}
	else if(iParentIndex == GCFS_DIR_TASK && strcmp(name, "config")==0)
	{
		e.ino = GCFS_DIRINODE(iTaskIndex, GCFS_DIR_CONFIG);
	}
	else if(iParentIndex == GCFS_DIR_TASK && strcmp(name, "data")==0)
	{
		e.ino = GCFS_DIRINODE(iTaskIndex, GCFS_DIR_DATA);
	}
	else if(iParentIndex == GCFS_DIR_TASK && (pTask = g_sTaskManager.getTask(iTaskIndex)) && pTask->isSubmited() && strcmp(name, "result")==0)
	{
		e.ino = GCFS_DIRINODE(iTaskIndex, GCFS_DIR_RESULT);
	}
	else if(iParentIndex == GCFS_DIR_TASK && (iIndex = g_sTaskManager.getControlIndex(name)) >= 0)
	{
		e.ino = GCFS_CONTROLINODE(iTaskIndex, GCFS_CONTROL_FIRST + iIndex);
	}
	else if(iParentIndex == GCFS_DIR_CONFIG && (pTask = g_sTaskManager.getTask(iTaskIndex)) && (iIndex = pTask->getConfigValueIndex(name)) >= 0){
		e.ino = GCFS_CONFIGINODE(iTaskIndex, iIndex);
	}
	else if(iParentIndex == GCFS_DIR_DATA && (pTask = g_sTaskManager.getTask(iTaskIndex)) &&  pTask->m_mDataFiles.find(name) != pTask->m_mDataFiles.end()){
		e.ino = pTask->m_mDataFiles[name]->m_iInode;
	}
	else if(iParentIndex == GCFS_DIR_RESULT && (pTask = g_sTaskManager.getTask(iTaskIndex)) &&  pTask->m_mResultFiles.find(name) != pTask->m_mResultFiles.end()){
		e.ino = pTask->m_mResultFiles[name]->m_iInode;
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

static void dirbuf_add(fuse_req_t req, std::string &buff , const char *name,
		       fuse_ino_t ino)
{
	struct stat stbuf = {0};

	stbuf.st_ino = ino;

	size_t bufSize = buff.size();

	size_t size = fuse_add_direntry(req, NULL, 0, name, NULL, 0);
	buff.resize(bufSize+size);
	fuse_add_direntry(req, (char*)buff.c_str() + bufSize, size, name, &stbuf, bufSize+size);

}

#define min(x, y) ((x) < (y) ? (x) : (y))

static int reply_buf_limited(fuse_req_t req, std::string &buff, size_t off, size_t maxsize)
{
	if (off < buff.size())
		return fuse_reply_buf(req, buff.c_str() + off,
				      min(buff.size() - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}

static void gcfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi)
{
	int iParentIndex = GCFS_INDEX_FROM_INODE(ino);
	uint iTaskIndex = GCFS_TASK_FROM_INODE(ino);

	std::string buff;
	dirbuf_add(req, buff, ".", ino);
	
	if (ino == FUSE_ROOT_ID)
	{
		for(uint iIndex = 0; iIndex < g_sTaskManager.getTaskCount(); iIndex++)
			dirbuf_add(req, buff, g_sTaskManager.getTask(iIndex)->m_sName.c_str(), GCFS_DIRINODE(iIndex, GCFS_DIR_TASK));
	}
	else switch(iParentIndex)
	{
		case GCFS_DIR_TASK:
		{
			dirbuf_add(req, buff, "..", FUSE_ROOT_ID);
			dirbuf_add(req, buff, "config", GCFS_DIRINODE(iTaskIndex, GCFS_DIR_CONFIG));
			dirbuf_add(req, buff, "data", GCFS_DIRINODE(iTaskIndex, GCFS_DIR_CONFIG));
			if(g_sTaskManager.getTask(iTaskIndex)->isFinished())
				dirbuf_add(req, buff, "result", GCFS_DIRINODE(iTaskIndex, GCFS_DIR_CONFIG));

			// Insert control files
			for(uint iIndex = 0; iIndex < g_sTaskManager.getControlCount(); iIndex++)
				dirbuf_add(req, buff, g_sTaskManager.getControl(iIndex)->m_sName, GCFS_CONFIGINODE(iTaskIndex, GCFS_CONTROL_FIRST + iIndex));
			break;
		}
		
		case GCFS_DIR_CONFIG:
		{
			dirbuf_add(req, buff, "..", GCFS_DIRINODE(iTaskIndex, GCFS_DIR_TASK));
         GCFS_Task * pTask = g_sTaskManager.getTask(iTaskIndex);
         for(uint iIndex = 0; iIndex < pTask->getConfigValueCount(); iIndex++)
				dirbuf_add(req, buff, pTask->getConfigValue(iIndex)->m_sName, GCFS_CONFIGINODE(iTaskIndex, iIndex));
			break;
		}

		case GCFS_DIR_DATA:
		{
			dirbuf_add(req, buff, "..", GCFS_DIRINODE(iTaskIndex, GCFS_DIR_TASK));
			GCFS_Task::Files & mFiles = g_sTaskManager.getTask(iTaskIndex)->m_mDataFiles;
			for(GCFS_Task::Files::iterator it = mFiles.begin(); it != mFiles.end() ; ++it)
				dirbuf_add(req, buff, it->first.c_str(), it->second->m_iInode);
			break;
		}

		case GCFS_DIR_RESULT:
		{
			dirbuf_add(req, buff, "..", GCFS_DIRINODE(iTaskIndex, GCFS_DIR_TASK));
			GCFS_Task::Files & mFiles = g_sTaskManager.getTask(iTaskIndex)->m_mResultFiles;
			for(GCFS_Task::Files::iterator it = mFiles.begin(); it != mFiles.end() ; ++it)
				dirbuf_add(req, buff, it->first.c_str(), it->second->m_iInode);
			break;
		}
		
		default:
			printf("Error readdir: inode: %d\n", (int)ino);
			fuse_reply_err(req, ENOTDIR);
			return;
	}
	
	reply_buf_limited(req, buff, off, size);
}

static void gcfs_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi)
{
	fi->fh = 0;
	
	if(ino > g_sTaskManager.m_uiFirstFileInode)
	{
		GCFS_Task::File *pFile = g_sTaskManager.getInodeFile(ino);

		fi->fh = (uint64_t)pFile;
		fi->direct_io = 1;

		fuse_reply_open(req, fi);
	}
	else
	{
		uint iIndex = GCFS_INDEX_FROM_INODE(ino);
		uint iTaskIndex = GCFS_TASK_FROM_INODE(ino);

		// Use direct-io -> read does not care about file size
		fi->direct_io = 1;

		if (iIndex < GCFS_DIR_LAST)
		{
			fuse_reply_err(req, EISDIR);
		}
		else if(GCFS_IS_CONFIGINODE(iIndex))
		{
			int iConfigIndex = iIndex - GCFS_CONFIG_FIRST;
			fi->fh = (uint64_t)g_sTaskManager.getTask(iTaskIndex)->getConfigValue(iConfigIndex);

			fuse_reply_open(req, fi);
		}
		else if(GCFS_IS_CONTROLINODE(iIndex))
		{
			int iControlIndex = iIndex - GCFS_CONTROL_FIRST;
			fi->fh = (uint64_t)g_sTaskManager.getControl(iControlIndex);

			fuse_reply_open(req, fi);
		}
		else
		{
			printf("Error open: inode: %d\n", (int)ino);
			fuse_reply_err(req, EACCES);
		}
	}
}

static void gcfs_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi)
{
	if(ino > g_sTaskManager.m_uiFirstFileInode)
	{
		GCFS_Task::File *pFile = (GCFS_Task::File *)fi->fh;
		fi->direct_io = 1;

		char *buff = new char[size];
		int hFile = pFile->getHandle();
		lseek(hFile, off, SEEK_SET);
		size = read(hFile, buff, size);

		fuse_reply_buf(req, buff, size);
		delete [] buff;
	}
	else
	{
		uint iIndex = GCFS_INDEX_FROM_INODE(ino);
		uint iTaskIndex = GCFS_TASK_FROM_INODE(ino);

		if(GCFS_IS_CONFIGINODE(iIndex))
		{
			GCFS_ConfigValue * pValue = (GCFS_ConfigValue *) fi->fh;

			std::string buff;
			pValue->PrintValue(buff);
			buff += "\n";

			reply_buf_limited(req, buff, off, size);
		}
		else if(GCFS_IS_CONTROLINODE(iIndex))
		{
			GCFS_Control * pControl = (GCFS_Control *) fi->fh;

			std::string buff;
			pControl->read(g_sTaskManager.getTask(iTaskIndex), buff);
			buff += "\n";

			reply_buf_limited(req, buff, off, size);
		}
		else
		{
			printf("Error read: inode: %d\n", (int)ino);
			fuse_reply_err(req, EACCES);
		}
	}
}

static void gcfs_write(fuse_req_t req, fuse_ino_t ino, const char *buf,
		       size_t size, off_t off, struct fuse_file_info *fi)
{
	if(ino > g_sTaskManager.m_uiFirstFileInode)
	{
		GCFS_Task::File *pFile = (GCFS_Task::File *)fi->fh;
		int hFile = pFile->getHandle();
		lseek(hFile, off, SEEK_SET);
		size = write(hFile, buf, size);

		fuse_reply_write(req, size);
	}
	else
	{
		uint iIndex = GCFS_INDEX_FROM_INODE(ino);
		uint iTaskIndex = GCFS_TASK_FROM_INODE(ino);

		if(GCFS_IS_CONFIGINODE(iIndex))
		{
			GCFS_ConfigValue * pValue = (GCFS_ConfigValue *) fi->fh;

			((char*)buf)[size] = 0;
			if(pValue->SetValue(buf))
				fuse_reply_write(req, size);
			else
				fuse_reply_err(req, EINVAL);
		}
		else if(GCFS_IS_CONTROLINODE(iIndex))
		{
			GCFS_Control * pControl = (GCFS_Control *) fi->fh;

			((char*)buf)[size] = 0;
			if(pControl->write(g_sTaskManager.getTask(iTaskIndex), buf))
				fuse_reply_write(req, size);
			else
				fuse_reply_err(req, EINVAL);
		}
		else
		{
			printf("Error write: inode: %d\n", (int)ino);
			fuse_reply_err(req, EACCES);
		}
	}
}

static void gcfs_readlink(fuse_req_t req, fuse_ino_t ino)
{
	uint iIndex = GCFS_INDEX_FROM_INODE(ino);
	uint iTaskIndex = GCFS_TASK_FROM_INODE(ino);

	if(iIndex == GCFS_CONTROL_EXECUTABLE && iTaskIndex < g_sTaskManager.getTaskCount())
	{
		fuse_reply_readlink(req, g_sTaskManager.getTask(iTaskIndex)->m_sExecutable.m_sValue.c_str());
		return;
	}

	fuse_reply_err(req, EEXIST);
}

static void gcfs_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name,
		       mode_t mode)
{
	if(parent == FUSE_ROOT_ID)
	{
		struct fuse_entry_param e = {0};

		GCFS_Task* pTask = NULL;
		if((pTask = g_sTaskManager.addTask(name)))
		{
			const fuse_ctx * pContext = fuse_req_ctx(req);
			pTask->m_sPermissions.m_sMode = mode & 0777;
			pTask->m_sPermissions.m_iGid = pContext->gid;
			pTask->m_sPermissions.m_iUid = pContext->uid;

			e.ino = GCFS_DIRINODE(g_sTaskManager.getTaskCount()-1, GCFS_DIR_TASK);
			e.attr_timeout = 1.0;
			e.entry_timeout = 1.0;
			gcfs_stat(e.ino, &e.attr);

			fuse_reply_entry(req, &e);
			return;
		}
		fuse_reply_err(req, EEXIST);
		return;
	}

	fuse_reply_err(req, EACCES);
}

static void gcfs_rmdir(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	if(parent == FUSE_ROOT_ID)
	{
		if(g_sTaskManager.getTaskIndex(name) >= 0)
		{
			g_sTaskManager.deleteTask(name);
			
			fuse_reply_err(req, 0);
			return;
		}
		fuse_reply_err(req, ENOENT);
		return;
	}

	fuse_reply_err(req, EACCES);
}

static void gcfs_unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	int iParentIndex =  GCFS_INDEX_FROM_INODE(parent);
	uint iTaskIndex = GCFS_TASK_FROM_INODE(parent);

	if(parent == FUSE_ROOT_ID)
	{
		if(g_sTaskManager.getTaskIndex(name) >= 0)
		{
			g_sTaskManager.deleteTask(name);

			fuse_reply_err(req, 0);
			return;
		}
		fuse_reply_err(req, ENOENT);
		return;
	}
	else if(iParentIndex == GCFS_DIR_DATA)
	{
		GCFS_Task * pTask;
		if((pTask = g_sTaskManager.getTask(iTaskIndex)) && pTask->m_mDataFiles.find(name) != pTask->m_mDataFiles.end())
		{
			pTask->deleteDataFile(name);

			fuse_reply_err(req, 0);
			return;
		}
		fuse_reply_err(req, ENOENT);
		return;
	}

	fuse_reply_err(req, EACCES);
}

static void gcfs_create(fuse_req_t req, fuse_ino_t parent, const char *name,
			mode_t mode, struct fuse_file_info *fi)
{
	int iParentIndex =  GCFS_INDEX_FROM_INODE(parent);
	uint iTaskIndex = GCFS_TASK_FROM_INODE(parent);

	if(iParentIndex == GCFS_DIR_DATA)
	{
		struct fuse_entry_param e = {0};

		GCFS_Task::File *pFile = g_sTaskManager.getTask(iTaskIndex)->createDataFile(name);

		e.ino = pFile->m_iInode;

		fi->fh = (uint64_t)pFile;

		gcfs_stat(e.ino, &e.attr);
		
		fuse_reply_create(req, &e, fi);
		return;
	}

	fuse_reply_err(req, EACCES);
}

static void gcfs_mknod(fuse_req_t req, fuse_ino_t parent, const char *name,
		       mode_t mode, dev_t rdev)
{

	int iParentIndex =  GCFS_INDEX_FROM_INODE(parent);
	uint iTaskIndex = GCFS_TASK_FROM_INODE(parent);

	if(iParentIndex == GCFS_DIR_DATA && mode & S_IFREG)
	{
		struct fuse_entry_param e = {0};

		GCFS_Task::File *pFile = g_sTaskManager.getTask(iTaskIndex)->createDataFile(name);

		e.ino = pFile->m_iInode;

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
