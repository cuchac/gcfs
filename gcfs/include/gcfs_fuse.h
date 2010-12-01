#ifndef __GCFS_FUSE__
#define __GCFS_FUSE__

#define GCFS_FUSE_INODES_PER_TASK		32

#define GCFS_DIR_FIRST						0
#define GCFS_DIR_TASK						0
#define GCFS_DIR_CONFIG						1
#define GCFS_DIR_DATA						2
#define GCFS_DIR_RESULT						3
#define GCFS_DIR_LAST						4

#define GCFS_CONTROL_FIRST					4
#define GCFS_CONTROL_CONTROL				4
#define GCFS_CONTROL_EXECUTABLE			5
#define GCFS_CONTROL_STATE					6
#define GCFS_CONTROL_LAST					7

#define GCFS_CONFIG_FIRST					7

#define GCFS_INDEX_FROM_INODE(inode)		((inode - 2) % GCFS_FUSE_INODES_PER_TASK)
#define GCFS_TASK_FROM_INODE(inode)		 	((inode - 2) / GCFS_FUSE_INODES_PER_TASK)

#define GCFS_DIRINODE(task, index)			(2 + (task) * GCFS_FUSE_INODES_PER_TASK + (index))
#define GCFS_CONFIGINODE(task, index)		(GCFS_DIRINODE(task, GCFS_CONTROL_LAST) + index)
#define GCFS_CONTROLINODE(task, index)		(GCFS_DIRINODE(task, index))

#define GCFS_IS_DIRINODE(index)				(index >= GCFS_DIR_FIRST && index < GCFS_DIR_LAST)
#define GCFS_IS_CONFIGINODE(index)			(index >= GCFS_CONTROL_LAST && index < GCFS_FUSE_INODES_PER_TASK)
#define GCFS_IS_CONTROLINODE(index)			(index >= GCFS_CONTROL_FIRST && index < GCFS_CONTROL_LAST)

//How long to remember stat info
#define GCFS_FUSE_STAT_TIMEOUT			60.0 //Forever - almost :-)

int init_fuse(int argc, char *argv[]);

#endif /*__GCFS_FUSE__*/
