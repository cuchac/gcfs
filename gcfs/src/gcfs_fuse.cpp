/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

*/

#define FUSE_USE_VERSION 26

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
#include <gcfs_task_config.h>

static int gcfs_stat(fuse_ino_t ino, struct stat *stbuf)
{
	stbuf->st_ino = ino;
	int iIndex = (ino - 2) % GCFS_FUSE_INODES_PER_TASK;

	if(ino > GCFS_FUSE_INODES_PER_TASK * g_sTasks.GetCount())
		return -1;
	
	if(ino == FUSE_ROOT_ID) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else if(iIndex == GCFS_DIR_TASK || iIndex == GCFS_DIR_CONFIG) // Task dir
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink =2;
	}
	else if(iIndex >= GCFS_DIR_LAST)
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
	}
	else
	{
		printf("Error stat inode: %d\n", (int)ino);
		return -1;
	}

	return 0;
}

static void gcfs_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi)
{
	struct stat stbuf;

	(void) fi;

	memset(&stbuf, 0, sizeof(stbuf));
	if (gcfs_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, 1.0);
}

static void gcfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e;
	int iParentIndex = (parent - 2) % GCFS_FUSE_INODES_PER_TASK;
	int iTaskIndex = (parent - 2) / GCFS_FUSE_INODES_PER_TASK;

	memset(&e, 0, sizeof(e));

	if ((parent == FUSE_ROOT_ID) && (g_sTasks.Get(name) != NULL))
	{
		e.ino = GCFS_DIRINODE(g_sTasks.GetIndex(name), GCFS_DIR_TASK);
	}
	else if(iParentIndex == GCFS_DIR_TASK && strcmp(name, "config")==0)
	{
		e.ino = GCFS_DIRINODE(iTaskIndex, GCFS_DIR_CONFIG);
	}
	else if(iParentIndex == GCFS_DIR_TASK && strcmp(name, "control")==0)
	{
		e.ino = GCFS_CONTROLINODE(iTaskIndex, 0);
	}
	else if(iParentIndex == GCFS_DIR_CONFIG && g_sTasks.Get(iTaskIndex)->m_mNameToIndex.find(name) != g_sTasks.Get(iTaskIndex)->m_mNameToIndex.end()){
		e.ino = GCFS_CONFIGINODE(iTaskIndex, g_sTasks.Get(iTaskIndex)->m_mNameToIndex[name]);
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

struct dirbuf {
	char *p;
	size_t size;
};

static void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
		       fuse_ino_t ino)
{
	struct stat stbuf;
	size_t oldsize = b->size;
	b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
	b->p = (char *) realloc(b->p, b->size);
	memset(&stbuf, 0, sizeof(stbuf));
	stbuf.st_ino = ino;
	fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,
			  b->size);
}

#define min(x, y) ((x) < (y) ? (x) : (y))

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize)
{
	if (off < bufsize)
		return fuse_reply_buf(req, buf + off,
				      min(bufsize - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}

static void gcfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi)
{
	(void) fi;
	int iParentIndex = (ino - 2) % GCFS_FUSE_INODES_PER_TASK;
	int iTaskIndex = (ino - 2) / GCFS_FUSE_INODES_PER_TASK;

	if (ino == FUSE_ROOT_ID)
	{
		struct dirbuf b;

		memset(&b, 0, sizeof(b));
		dirbuf_add(req, &b, ".", 1);
		dirbuf_add(req, &b, "..", 1);
		for(int iIndex = 0; iIndex < g_sTasks.GetCount(); iIndex++)
			dirbuf_add(req, &b, g_sTasks.Get(iIndex)->m_sName.c_str(), GCFS_DIRINODE(iIndex, GCFS_DIR_TASK));
			
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
	else if(iParentIndex == GCFS_DIR_TASK){
		struct dirbuf b;

		memset(&b, 0, sizeof(b));
		dirbuf_add(req, &b, ".", ino);
		dirbuf_add(req, &b, "..", FUSE_ROOT_ID);
		dirbuf_add(req, &b, "config", GCFS_DIRINODE(iTaskIndex, GCFS_DIR_CONFIG));
		dirbuf_add(req, &b, "control", GCFS_CONFIGINODE(iTaskIndex, 0));
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
	else if(iParentIndex == GCFS_DIR_CONFIG){
		struct dirbuf b;

		memset(&b, 0, sizeof(b));
		dirbuf_add(req, &b, ".", ino);
		dirbuf_add(req, &b, "..", GCFS_DIRINODE(iTaskIndex, GCFS_DIR_TASK));
		for(int iIndex = 0; iIndex < g_sTasks.Get(iTaskIndex)->m_vIndexToName.size(); iIndex++)
			dirbuf_add(req, &b, g_sTasks.Get(iTaskIndex)->m_vIndexToName[iIndex]->m_sName, GCFS_CONFIGINODE(iTaskIndex, iIndex));
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
	else
		fuse_reply_err(req, ENOTDIR);
}

static void gcfs_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi)
{
	int iIndex = (ino - 2) % GCFS_FUSE_INODES_PER_TASK;
	int iTaskIndex = (ino - 2) / GCFS_FUSE_INODES_PER_TASK;
	
	if (iIndex < GCFS_DIR_LAST)
		fuse_reply_err(req, EISDIR);
	else if ((fi->flags & 3) != O_RDONLY)
		fuse_reply_err(req, EACCES);
	else if(iIndex < GCFS_CONFIGINODE(0,0))
	{
		fuse_reply_open(req, fi);
	}
}

static void gcfs_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi)
{
	(void) fi;

	assert(ino == 2);
	//reply_buf_limited(req, gcfs_str, strlen(gcfs_str), off, size);
}

static struct fuse_lowlevel_ops gcfs_oper = {};

int init_fuse(int argc, char *argv[])
{
	gcfs_oper.lookup	= gcfs_lookup;
	gcfs_oper.getattr	= gcfs_getattr;
	gcfs_oper.readdir	= gcfs_readdir;
	gcfs_oper.open		= gcfs_open;
	gcfs_oper.read		= gcfs_read;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_chan *ch;
	char *mountpoint;
	int err = -1;

	if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1 &&
	    (ch = fuse_mount(mountpoint, &args)) != NULL) {
		struct fuse_session *se;

		se = fuse_lowlevel_new(&args, &gcfs_oper,
				       sizeof(gcfs_oper), NULL);
		if (se != NULL) {
			if (fuse_set_signal_handlers(se) != -1) {
				fuse_session_add_chan(se, ch);
				err = fuse_session_loop(se);
				fuse_remove_signal_handlers(se);
				fuse_session_remove_chan(ch);
			}
			fuse_session_destroy(se);
		}
		fuse_unmount(mountpoint, ch);
	}
	fuse_opt_free_args(&args);

	return err ? 1 : 0;
}