#ifndef __GCFS__
#define __GCFS__

#include <map>
#include <vector>
#include <string>
#include <sys/stat.h>

class GCFS_TaskManager;
class GCFS_Config;

// Permissions for tasks
struct GCFS_Permissions
{
	mode_t		m_sMode;
	uid_t			m_iUid;
	gid_t			m_iGid;
};

// Configuration
extern GCFS_Config g_sConfig;

// Tasks
extern GCFS_TaskManager g_sTasks;

#endif /*__GCFS__*/
