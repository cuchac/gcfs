#include "gcfs_servicecondor.h"

GCFS_ServiceCondor::GCFS_ServiceCondor(const char* sName): GCFS_Service(sName)
{

}


// Public module API for task submission
bool GCFS_ServiceCondor::submitTask(GCFS_Task* pTask)
{
}

bool GCFS_ServiceCondor::waitForTask(GCFS_Task* pTask)
{
}