
#ifndef DIGEDAG_SCHEDULER_HPP
#define DIGEDAG_SCHEDULER_HPP

#include <set>
#include <deque>
#include <vector>

#include <saga/saga.hpp>

#include "dag.hpp"
#include "node.hpp"
#include "edge.hpp"

#include "util/mutex.hpp"
#include "util/thread.hpp"
#include "util/scoped_lock.hpp"


namespace digedag
{
  class dag;
  class node;
  class edge;
  class enactor;
  class scheduler : public digedag::util::thread, 
                    public util::enable_shared_from_this <scheduler>
  {
    private:
      struct job_info_t 
      { 
        std::string rm;
        std::string host;
        std::string pwd;
        std::string path;
      };

      std::map <std::string, job_info_t>     job_info_;

      std::string                            data_src_pwd_;
      std::string                            data_tgt_pwd_;

      std::string                            data_src_host_;
      std::string                            data_tgt_host_;

      saga::session                        & session_;
      dag                                  * dag_;
      std::string                            policy_; // scheduling policy


      bool                                   initialized_;
      bool                                   stopped_;

      // queues
      std::deque <boost::shared_ptr <node> > queue_nodes_;
      std::deque <boost::shared_ptr <edge> > queue_edges_;

      std::set <std::string>                 active_files_; // see FIXME in task_run 

      boost::shared_ptr <enactor>            enact_nodes_;
      boost::shared_ptr <enactor>            enact_edges_;

      int                                    max_nodes_;
      int                                    max_edges_;
      
      int                                    active_nodes_;
      int                                    active_edges_;

      std::map <saga::task, boost::shared_ptr <node> > node_task_map_;
      std::map <saga::task, boost::shared_ptr <edge> > edge_task_map_;

      util::mutex                            mtx_;
      void lock                     (void) { mtx_.lock   (); };
      void unlock                   (void) { mtx_.unlock (); };

      // list of known nodes and edges, which helps to avoid scheduling them
      // twice.  Its actually only used for nodes right now, as edges get only
      // fired once anyway.  Nodes however can get fired multiple times.
      std::set <std::string>                known_nodes_;
      std::set <std::string>                known_edges_;


    public:
      scheduler  (dag               * d, 
                  const std::string & policy, 
                  saga::session       session);
      ~scheduler (void);

      void parse_src             (void);
      void stop                  (void);

      void thread_work           (void);

      bool hook_dag_create       (void);
      bool hook_dag_destroy      (void);
      bool hook_dag_schedule     (void);
      bool hook_dag_run_pre      (void);
      bool hook_dag_run_post     (void);
      bool hook_dag_run_done     (void);
      bool hook_dag_run_fail     (void);
      bool hook_dag_wait         (void);

      bool hook_node_add         (boost::shared_ptr <node> n);
      bool hook_node_remove      (boost::shared_ptr <node> n);
      bool hook_node_run_pre     (boost::shared_ptr <node> n);
      bool hook_node_run_done    (boost::shared_ptr <node> n);
      bool hook_node_run_fail    (boost::shared_ptr <node> n);

      bool hook_edge_add         (boost::shared_ptr <edge> e);
      bool hook_edge_remove      (boost::shared_ptr <edge> e);
      bool hook_edge_run_pre     (boost::shared_ptr <edge> e);
      bool hook_edge_run_done    (boost::shared_ptr <edge> e);
      bool hook_edge_run_fail    (boost::shared_ptr <edge> e);

      saga::session
           hook_saga_get_session (void);

      void work_finished         (saga::task  t, 
                                  std::string flag);

      void dump_map              (const std::map <saga::task, boost::shared_ptr <edge> >  & map);
  };

} // namespace digedag

#endif // DIGEDAG_SCHEDULER_HPP

