
#ifndef DIGEDAG_ENACTOR_HPP
#define DIGEDAG_ENACTOR_HPP

#include <set>
#include <deque>
#include <vector>

#include <saga/saga.hpp>

#include "util/mutex.hpp"
#include "util/thread.hpp"
#include "util/scoped_lock.hpp"


namespace digedag
{
  // task_container are broken, we use this watch_tasks class for notification
  // on finished tasks.  It will constantly cycle over the contents of a given
  // task container in its worker thread, and will call 'cb' in the main thread
  // if any task changed its state to Done or Failed.

  class dag;
  class node;
  class edge;
  class scheduler;
  class enactor : public digedag::util::thread
  {
    private:
      saga::task_container              tc_;
      boost::shared_ptr <scheduler>     s_;
      std::string                       f_;
      bool                              todo_;

      util::mutex                       mtx_;
      void lock                (void) { mtx_.lock   (); };
      void unlock              (void) { mtx_.unlock (); };

    public:
      enactor (boost::shared_ptr <scheduler> s, 
               std::string                   flag);

      ~enactor (void);

      void queue_task (saga::task t);

      void thread_work (void);
  };

} // namespace digedag


#endif // DIGEDAG_ENACTOR_HPP

