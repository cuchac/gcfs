#ifndef GCFS_SERVICESAGATASKDATA_H
#define GCFS_SERVICESAGATASKDATA_H

#include <map>

#include "digedag.hpp"

class GCFS_Task;

class GCFS_ServiceSagaTaskData
{
public:
                     GCFS_ServiceSagaTaskData();
                     
public:
   std::map<GCFS_Task*, boost::shared_ptr <digedag::node> >    m_mTasks;
   
};

#endif // GCFS_SERVICESAGATASKDATA_H
