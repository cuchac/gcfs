
#include "dag.hpp"
#include "node.hpp"
#include "edge.hpp"
#include "scheduler.hpp"
#include "enactor.hpp"
#include "util/util.hpp"

namespace digedag
{
  enactor::enactor (boost::shared_ptr <scheduler> s, 
                    std::string                   flag)
    : s_    (s)
    , f_    (flag)
    , todo_ (true)
  {
    thread_run ();
  }

  
  enactor::~enactor (void)
  {
    todo_ = false;
    thread_join ();
  }

  void enactor::queue_task (saga::task t)
  {
    util::scoped_lock (mtx_);
    tc_.add_task (t);
  }


  void enactor::thread_work (void)
  {
    std::vector <saga::task> tasks;

    while ( todo_ )
    {
      {
        util::scoped_lock (mtx_);
        tasks = tc_.list_tasks ();
      }

      // by default, so unless we find interesting task state changes, we
      // sleep a little to avoid busy waits.  Task container notifications
      // will help once implemented.
      bool do_wait = true;

      for ( unsigned int i = 0; i < tasks.size (); i++ )
      {
        saga::task        t = tasks[i];
        saga::task::state s = t.get_state ();

        if ( s == saga::task::Done   || 
             s == saga::task::Failed )
        {
          std::cout << " === task " << t.get_id () << " is final: " 
                    << saga_state_to_string (t.get_state ()) 
                    << std::endl;

          {
            util::scoped_lock (mtx_);
            tc_.remove_task   (t);
            s_->work_finished (t, f_);
          }

          // just check task container again immediately
          do_wait = false;
        }
      }

      // avoid busy wait
      if ( do_wait )
      {
        util::ms_sleep (100);
      }
    } // while (todo_)
  }

} // namespace digedag

