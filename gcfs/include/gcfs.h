#ifndef __GCFS__
#define __GCFS__

#include <map>
#include <vector>
#include <string>

class GCFS_TaskConfig;
class GCFS_Config;

// Configuration
extern GCFS_Config g_sConfiguration;

// Tasks
extern std::vector<GCFS_TaskConfig> g_vTasks;
extern std::map<std::string, int> g_mTaskNames;

#define GCFS_SERVICE_BALANCED "-Balanced-"

#endif /*__GCFS__*/ 