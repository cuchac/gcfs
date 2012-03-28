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

	// Skip root dir itself
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

int chmod_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{

	// Skip root dir itself
	if(ftwbuf->base == 0)
		return 0;

	int rv = chmod(fpath, 0777);

	if (rv)
		perror(fpath);

	return 0;
}

bool GCFS_Utils::chmodRecursive(const char *sPath, mode_t iMode)
{
	// nftw does not support user data - ignore it and assume 0777
	return nftw(sPath, chmod_cb, 64, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) != 0;
}

std::string GCFS_Utils::TrimStr(const std::string& Src, const std::string& c)
{
   size_t p2 = Src.find_last_not_of(c);
   if (p2 == std::string::npos) return std::string();
   size_t p1 = Src.find_first_not_of(c);
   if (p1 == std::string::npos) p1 = 0;
   return Src.substr(p1, (p2-p1)+1);
}

GCFS_Utils::keyValue_t GCFS_Utils::ParseAssignment(char * string)
{
   char *pChar = NULL, *pKey = NULL, *pValue = NULL;
   if((pKey = strtok_r(string, GCFS_CONFIG_ASSIGNCHAR, &pChar)) != NULL &&
      (pValue = strtok_r(NULL, GCFS_CONFIG_ASSIGNCHAR, &pChar)) != NULL)
      // Return key-value pair
      return keyValue_t(pKey, pValue);
   
   // No assignment -> it is command
   return keyValue_t(string, NULL);
}

bool GCFS_Utils::ParseConfigString(char * string, GCFS_Utils::keyValueArray_t& vValues)
{
   // replace $(process) with @(process) to get rid of '$' character. It is delimiter as well
   ReplaceToken(string, "$(process)", "@(process)");
   
   // Parse all the commands/assignments
   char * pPos = NULL;
   char * pToken = strtok_r(string, GCFS_CONFIG_DELIMITERS, &pPos);
   
   while(pToken != NULL)
   {
      ReplaceToken(pToken, "@(process)", "$(process)");
      
      vValues.push_back(ParseAssignment(pToken));
      
      pToken = strtok_r(NULL, GCFS_CONFIG_DELIMITERS, &pPos);
   }
   
   return true;
}

void GCFS_Utils::ReplaceToken(char * sString, const char * sSearch, const char * sReplacement)
{
   char * pPos = sString;
   while((pPos = strcasestr(pPos, sSearch)) != NULL)
   {
      memcpy(pPos, sReplacement, strlen(sReplacement));
      pPos++;
   }
}