#ifndef GCFS_UTILS_H
#define GCFS_UTILS_H

#include <string>

#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))

class GCFS_Utils
{
public:
	static bool			getHomePath(std::string &buffer);
	static bool			mkdirRecursive(const char *sPath);
	static bool			rmdirRecursive(const char *sPath);
};

#endif // GCFS_UTILS_H
