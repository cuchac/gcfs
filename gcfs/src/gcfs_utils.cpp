#include "../include/gcfs_utils.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
	//#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>	

#endif



bool GCFS_Utils::getHomePath(std::string &buffer)
{
	// Get Home dir by platfomr-independent way
#ifdef _WIN32

	buffer = getenv("USERPROFILE");
	if(!buffer.length())
	{
		buffer = getenv("HOMEDRIVE");
		buffer += getenv("HOMEPATH");
	}

#else

	buffer = getenv("HOME");

	if(!buffer.length())
		buffer = getpwuid(getuid())->pw_dir;

#endif
	return true;
}

bool GCFS_Utils::mkdirRecursive(const char *path)
{
        char opath[1024];
        char *p;
        size_t len;

        strncpy(opath, path, sizeof(opath));
        len = strlen(opath);
        if(opath[len - 1] == '/')
                opath[len - 1] = '\0';
        for(p = opath; *p; p++)
                if(*p == '/') {
                        *p = '\0';
                        if(access(opath, F_OK))
                                mkdir(opath, S_IRWXU | S_IRWXG | S_IRWXO);
                        *p = '/';
                }
        if(access(opath, F_OK))         /* if path is not terminated with / */
                mkdir(opath, S_IRWXU | S_IRWXG | S_IRWXO);
		  
	return true;
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{

	// Skip roo dir itself
	if(ftwbuf->base == 0)
		return 0;

	int rv = remove(fpath);

	if (rv)
		perror(fpath);

	return rv;
}

bool GCFS_Utils::rmdirRecursive(const char *sPath)
{
	// Dummy failsafe
	if(strlen(sPath) < 5)
	{
		printf("Tried to delete too short directory '%s'. Is this right?!?!?!\n", sPath);
		return false;
	}

	return nftw(sPath, unlink_cb, 64, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) != 0;
}