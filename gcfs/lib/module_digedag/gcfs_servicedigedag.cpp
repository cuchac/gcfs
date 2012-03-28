#include <digedag.hpp>

#include "gcfs_servicedigedag.h"
#include "gcfs_config.h"
#include <gcfs_utils.h>
#include <fcntl.h>

GCFS_ServiceDigedag::GCFS_ServiceDigedag(const char* sName): GCFS_ServiceSaga(sName)
{

}

// Public module API for task submission
bool GCFS_ServiceDigedag::submitTask(GCFS_Task* pTask)
{
   GCFS_Task* pTask1 = g_sTaskManager.getTask("Test");
   GCFS_Task* pTask2 = g_sTaskManager.getTask("Test2");
   
   // create a new and empty DAG instance
   boost::shared_ptr <digedag::dag> d (new digedag::dag);
   
   // Prepare tasks
   digedag::node_description nd1;
   digedag::node_description nd2;

   setDescription(pTask1, nd1);
   setDescription(pTask2, nd2);

   prepareJobDir(pTask1);
   prepareJobDir(pTask2);

   boost::shared_ptr <digedag::node> n1 = d->create_node (nd1);
   boost::shared_ptr <digedag::node> n2 = d->create_node (nd2);
   
   // add two nodes to the dag, with identifiers
   d->add_node ("node_1", n1);
   d->add_node ("node_2", n2);
   
   // specify data to be transfered between the two nodes.
   // Again, the dag instance serves as an edge factory.
   boost::shared_ptr <digedag::edge> e1 = d->create_edge (getSubmitDir(pTask1), getSubmitDir(pTask2)+"/task1");
   
   // add that edge to the DAG
   d->add_edge (e1, n1, n2);
   
   // the abstract DAG is complete, and can bew explicitely
   // translated into a concrete DAG.  This step is optional
   // though, and is implictely peformed on fire() if not
   // called previously.  See infos about the Scheduler below.
   d->schedule ();

   d->dump();

   // Start Submission
   // If running under root, change credentials to submiting user
   if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
   {
      setresgid(pTask->m_sPermissions.m_iGid, pTask->m_sPermissions.m_iGid, 0);
      setresuid(pTask->m_sPermissions.m_iUid, pTask->m_sPermissions.m_iUid, 0);
   }
   
   // the (abstract or concrete) DAG is ready, and can be run
   d->fire ();

   // Grant access to submitting process (for Condor)
   GCFS_Utils::chmodRecursive(getSubmitDir(pTask1).c_str(), 0777);
   GCFS_Utils::chmodRecursive(getSubmitDir(pTask2).c_str(), 0777);
   GCFS_Utils::chmodRecursive(g_sConfig.m_sDataDir.c_str(), 0777);
   
   std::cout << "dag    running..." << std::endl;
   
   // wait 'til the dag had a chance to finish
   while ( digedag::Running == d->get_state () )
   {
      ::sleep (1);
      std::cout << "dag    waiting..." << std::endl;
   }

   // this is the same as the loop above
   d->wait ();

   // Return back to root
   if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
   {
      // Return back permissions
      setresgid(0, 0, 0);
      setresuid(0, 0, 0);
   }
   
   return true;
}

bool GCFS_ServiceDigedag::waitForTask(GCFS_Task* pTask)
{
   return true;
}

const std::string GCFS_ServiceDigedag::getTaskId(GCFS_Task* pTask)
{
   return "";
}

bool GCFS_ServiceDigedag::prepareJobDir(GCFS_Task* pTask)
{
   if(!GCFS_ServiceSaga::prepareJobDir(pTask))
      return false;

   GCFS_Utils::chmodRecursive(getSubmitDir(pTask).c_str(), 0777);

   std::string sPath = getSubmitDir(pTask) + "/" + pTask->m_pConfigDirectory->m_psError->get();
   close(creat(sPath.c_str(), 0777));
   chmod(sPath.c_str(), 0777);
   sPath = getSubmitDir(pTask) + "/" + pTask->m_pConfigDirectory->m_psOutput->get();
   close(creat(sPath.c_str(), 0777));
   chmod(sPath.c_str(), 0777);
   
   return true;
}

bool GCFS_ServiceDigedag::setDescription(GCFS_Task* pTask, saga::job::description& pDesc)
{
   if(!GCFS_ServiceSaga::setDescription(pTask, pDesc))
      return false;

   pDesc.set_attribute(digedag::node_attributes::working_directory, getSubmitDir(pTask));

   return true;
}
