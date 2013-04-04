#include <digedag.hpp>

#include "gcfs_servicedigedag.h"
#include "gcfs_servicesagataskdata.h"
#include "gcfs_filesystemdigedag.h"
#include "gcfs_config.h"
#include <gcfs_utils.h>
#include <fcntl.h>

GCFS_ServiceDigedag::GCFS_ServiceDigedag(const char* sName): GCFS_ServiceSaga(sName)
{

}

bool GCFS_ServiceDigedag::customizeTask(GCFS_Task* pTask, GCFS_ConfigDirectory* pDirectory)
{
   if(pDirectory)
      pDirectory->addChild(new GCFS_ConfigDependsOn(pDirectory), "depends_on");
   
   return true;
}

bool GCFS_ServiceDigedag::decustomizeTask(GCFS_Task* pTask, GCFS_ConfigDirectory* pDirectory)
{
   if(pDirectory)
      pDirectory->removeChild("depends_on");

   return true;
}

// Public module API for task submission
bool GCFS_ServiceDigedag::submitTask(GCFS_Task* pTask)
{
   // Collect all the subtasks
   std::vector<GCFS_Task*> vTasks;
   std::vector< GCFS_Task* >::iterator it;
   vTasks.push_back(pTask);

   pTask->getSubtasks(vTasks);

   // create a new and empty DAG instance
   boost::shared_ptr <digedag::dag> pDag (new digedag::dag);

   // Create task data structure
   GCFS_ServiceSagaTaskData * pTaskData = new GCFS_ServiceSagaTaskData;

   for(it = vTasks.begin(); it != vTasks.end(); it++)
   {
      // Prepare tasks
      digedag::node_description sDescription;

      setDescription(*it, sDescription);

      prepareJobDir(*it);

      boost::shared_ptr <digedag::node> pNode = pDag->create_node(sDescription);
      pTaskData->m_mTasks[*it] = pNode;

      // add node to the dag, with identifier
      pDag->add_node((*it)->getName(), pNode);
   }
   
   for(it = vTasks.begin(); it != vTasks.end(); it++)
   {
      GCFS_ConfigDependsOn* pDependsOn = (GCFS_ConfigDependsOn*)(*it)->getConfigValue("depends_on");
      if(pDependsOn)
      {
         std::vector< GCFS_Task* > vDependsOnTasks = pDependsOn->get();
         std::vector< GCFS_Task* >::iterator itDep;
         
         for(itDep = vDependsOnTasks.begin(); itDep != vDependsOnTasks.end(); itDep++)
         {
            // specify data to be transfered between the two nodes.
            boost::shared_ptr <digedag::edge> pEdge = pDag->create_edge(getSubmitDir(*itDep), getSubmitDir(*it)+"/"+(*itDep)->getName());

            // add that edge to the DAG
            pDag->add_edge (pEdge, pTaskData->m_mTasks[*itDep], pTaskData->m_mTasks[*it]);
         }
      }
   }
   
   // Start Submission
   // If running under root, change credentials to submiting user
   if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
   {
      setresgid(pTask->m_sPermissions.m_iGid, pTask->m_sPermissions.m_iGid, 0);
      setresuid(pTask->m_sPermissions.m_iUid, pTask->m_sPermissions.m_iUid, 0);
   }
   
   // the abstract DAG is complete, and can bew explicitely
   // translated into a concrete DAG.  This step is optional
   // though, and is implictely peformed on fire() if not
   // called previously.  See infos about the Scheduler below.
   pDag->schedule ();

   pDag->dump();
   
   // the (abstract or concrete) DAG is ready, and can be run
   pDag->fire ();
   
   std::cout << "dag    running..." << std::endl;
   
   // wait 'til the dag had a chance to finish
   pDag->wait();

   // Return back to root
   if(g_sConfig.m_sPermissions.m_iUid == 0 || g_sConfig.m_sPermissions.m_iGid == 0)
   {
      // Return back permissions
      setresgid(0, 0, 0);
      setresuid(0, 0, 0);
   }

   for(it = vTasks.begin(); it != vTasks.end(); it++)
   {
      digedag::state taskState = pTaskData->m_mTasks[*it]->get_state();
      if(taskState == digedag::Done)
         finishTask(*it);
      else
         finishTask(*it, "Execution failed.");
   }
   
   return true;
}

bool GCFS_ServiceDigedag::waitForTask(GCFS_Task* pTask)
{
   return true;
}

bool GCFS_ServiceDigedag::deleteTask(GCFS_Task* pTask)
{
   const GCFS_TaskManager::TaskList &mTasks = g_sTaskManager.getSubtasks(pTask);

   std::map< std::string, GCFS_Task* >::const_iterator it;
   for(it = mTasks.begin(); it != mTasks.end(); it++)
   {
      GCFS_ConfigValue* pValue = it->second->getConfigValue("depends_on");
      if(pValue)
         ((GCFS_ConfigDependsOn*)pValue)->removeTask(pTask);
   }
   
   return GCFS_ServiceSaga::deleteTask(pTask);
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
   if(!pTask->m_pConfigDirectory->m_psInput->getString().empty())
   {
      sPath = getSubmitDir(pTask) + "/" + pTask->m_pConfigDirectory->m_psInput->get();
      close(creat(sPath.c_str(), 0777));
      chmod(sPath.c_str(), 0777);
   }
   const GCFS_FileSystem::FileList* vTaskDataFiles = pTask->getDataFiles();
   for(GCFS_FileSystem::FileList::const_iterator it = vTaskDataFiles->begin(); it != vTaskDataFiles->end(); it++)
   {
      sPath = getSubmitDir(pTask) + "/" + it->first;
      close(creat(sPath.c_str(), 0777));
      chmod(sPath.c_str(), 0777);
   }
   
   
   return true;
}

bool GCFS_ServiceDigedag::setDescription(GCFS_Task* pTask, saga::job::description& pDesc)
{
   if(!GCFS_ServiceSaga::setDescription(pTask, pDesc))
      return false;

   pDesc.set_attribute(digedag::node_attributes::working_directory, getSubmitDir(pTask));

   return true;
}
