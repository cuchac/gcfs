
#ifndef DIGEDAG_NODEDESC_HPP
#define DIGEDAG_NODEDESC_HPP 

#include <saga/saga.hpp>

namespace digedag
{
  namespace node_attributes
  {
    // FIXME: other attribs are not to be supported!
    char const * const executable          = "Executable";
    char const * const arguments           = "Arguments";
    char const * const environment         = "Environment";
    char const * const working_directory   = "WorkingDirectory";
    char const * const interactive         = "Interactive";
    char const * const input               = "Input";
    char const * const output              = "Output";
    char const * const error               = "Error";
    char const * const candidate_hosts     = "CandidateHosts";
    char const * const job_project         = "JobProject";
    char const * const spmd_variation      = "SPMDVariation";
    char const * const total_cpu_count     = "TotalCPUCount";
    char const * const number_of_processes = "NumberOfProcesses";
    char const * const processes_per_host  = "ProcessesPerHost";
    char const * const threads_per_process = "ThreadsPerProcess";
  }

  class node_description : public saga::job::description 
  {
    // FIXME: create a check for the above attribs, and translate them into
    // a saga job description.  node_description should not inherit
    // job_description, but have a job_description, and an accessor.
  };

} // namespace digedag

#endif // DIGEDAG_NODEDESC_HPP

