#ifndef __GCFS__
#define __GCFS__

#include <map>
#include <vector>

class GCFS_TaskConfig;
class GCFS_Config;

// Configuration
extern GCFS_Config g_sConfiguration;

// Tasks
extern std::vector<GCFS_TaskConfig> g_vTasks;

#define GCFS_SERVICE_BALANCED "-Balanced-"

#endif /*__GCFS__*/ 