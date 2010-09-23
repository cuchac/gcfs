#ifndef __GCFS_FUSE__
#define __GCFS_FUSE__

#define GCFS_FUSE_INODES_PER_TASK		16
#define GCFS_FUSE_INODES_CONTROLS		5

#define GCFS_DIR_TASK						0
#define GCFS_DIR_CONFIG						1
#define GCFS_DIR_LAST						2

#define GCFS_DIRINODE(task, index)			(2 + (task) * GCFS_FUSE_INODES_PER_TASK + (index))
#define GCFS_CONFIGINODE(task, index)		(GCFS_DIRINODE(task, GCFS_DIR_LAST) + GCFS_FUSE_INODES_CONTROLS + index)
#define GCFS_CONTROLINODE(task, index)		(GCFS_DIRINODE(task, GCFS_DIR_LAST) + index)


int init_fuse(int argc, char *argv[]);

#endif /*__GCFS_FUSE__*/