#ifndef __GCFS_FUSE__
#define __GCFS_FUSE__

#define GCFS_FUSE_INODES_PER_TASK		32
#define GCFS_FUSE_INODES_CONTROLS		5

#define GCFS_DIR_TASK						0
#define GCFS_DIR_CONFIG						1
#define GCFS_DIR_DATA						2
#define GCFS_DIR_RESULT						3
#define GCFS_DIR_LAST						4

#define GCFS_DIRINODE(task, index)			(2 + (task) * GCFS_FUSE_INODES_PER_TASK + (index))
#define GCFS_CONFIGINODE(task, index)		(GCFS_DIRINODE(task, GCFS_DIR_LAST) + GCFS_FUSE_INODES_CONTROLS + index)
#define GCFS_CONTROLINODE(task, index)		(GCFS_DIRINODE(task, GCFS_DIR_LAST) + index)

#define GCFS_IS_DIRINODE(index)				(index >= 0 && index < GCFS_DIR_LAST)
#define GCFS_IS_CONFIGINODE(index)			(index >= (GCFS_DIR_LAST + GCFS_FUSE_INODES_CONTROLS) && index < GCFS_FUSE_INODES_PER_TASK)
#define GCFS_IS_CONTROLINODE(index)			(index >= GCFS_DIR_LAST && index < G(CFS_DIR_LAST + GCFS_FUSE_INODES_CONTROLS))

int init_fuse(int argc, char *argv[]);

#endif /*__GCFS_FUSE__*/