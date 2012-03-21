#ifndef GCFS_UTILS_H
#define GCFS_UTILS_H

#include <string>
#include <vector>
#include <stdlib.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))

// Characters to trim from configuration names and values
#define GCFS_CONFIG_TRIMCHARS " \r\n"
// Delimiters of configuration values
#define GCFS_CONFIG_DELIMITERS "\r\n$;"
// Delimiters of assignment
#define GCFS_CONFIG_ASSIGNCHAR "="

class GCFS_Utils
{
public:
   // Filesystem functions
   static bool			   getHomePath(std::string &buffer);
   static bool			   mkdirRecursive(const char *sPath);
   static bool			   rmdirRecursive(const char *sPath);
   static bool			   chmodRecursive(const char *sPath, mode_t iMode);
   
   // Configuration string parser
   typedef  std::pair<const char*, const char *> keyValue_t;
   typedef  std::vector<keyValue_t> keyValueArray_t;
   
   static keyValue_t    ParseAssignment(char* string);
   static bool          ParseConfigString(char* string, keyValueArray_t& vValues);
   static void          ReplaceToken(char* sString, const char* sSearch, const char* sReplacement);
   
   // String functions
   static std::string   TrimStr(const std::string& Src, const std::string& c = GCFS_CONFIG_TRIMCHARS);
};

#endif // GCFS_UTILS_H
