#ifndef __GCFS_FUSE__
#define __GCFS_FUSE__

//How long to remember stat info
#define GCFS_FUSE_STAT_TIMEOUT			60.0 //Forever - almost :-)

int init_fuse(int argc, char *argv[]);

#endif /*__GCFS_FUSE__*/
