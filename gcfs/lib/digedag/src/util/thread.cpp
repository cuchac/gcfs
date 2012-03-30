
#include "util/thread.hpp"

#include <errno.h>
#include <stdio.h>

#include <saga/saga.hpp>

#define DO_THREADS 

namespace digedag 
{
  namespace util
  {
    extern "C"
    {
      // the thread main routine, as argument to pthread_create, needs to be
      // a C function, not a C++ member method.  Thus, we define an "C" external
      // function here, which takes a pointer to an instance of our thread class,
      // and calls its thread_start method.
      void * thread_startup_ (void * arg)
      {
        digedag::util::thread * t = (digedag::util::thread *) arg;
        return t->thread_start ();
      }
    }


    // the c'tor does nothing than setting the state to 'New'
    thread::thread (void)
      : thread_state_ (ThreadNew)
      , joined_       (true) // don't join threads which did not even start
    {
    }

    // Note that the d'tor does not wait for the thread to finish.  Thus, we
    // expect the worker thread to continue as long as the application is alive
    // FIXME: a clean thread_cancel should be used.  pthread_kill has trouble if
    // the thread is owning locks.  boost.thread.kill only works for often
    // irrelevant cancelation points.
    thread::~thread () 
    {
      thread_join ();
    }


    // called by the inheriting classes, or by other consumers, to get the
    // thread going.  If the state is not 'New', this method does nothing.
    // Otherwise, the external "C" thread_startup_ routine is called, and state
    // is changed to Running.
    void thread::thread_run (void)
    {
      if ( thread_state_ != ThreadNew )
      {
        throw ("Thread is not New");
        return;
      }

      thread_state_ = ThreadRunning;

#ifdef DO_THREADS
      if ( 0 != pthread_create (&thread_, NULL, digedag::util::thread_startup_, this) )
      {
        thread_state_ = ThreadFailed;
        joined_       = true; // don't join failed threads
        throw (strerror (errno));
      }
      else
      {
        joined_       = false; // we can join the created thread later on
      }
#else
      thread_start ();
#endif
    }


    // thread_start is called by the external "C" function 'thread_startup_'
    // defined above.  It represents the running thread, and calls thread_work,
    // which holds the threads workload.
    void * thread::thread_start (void)
    {
      try
      {
        // startup is completed - call the (custom) workload
        this->thread_work ();
        
        // the thread workload is done - update state
        thread_state_ = ThreadDone;
      }
      catch ( char const * s )
      {
        std::cerr << "char* thread exception: " << s << std::endl;
        thread_state_ = ThreadFailed;
      }
      catch ( const std::string & s )
      {
        std::cerr << "string thread exception: " << s << std::endl;
        thread_state_ = ThreadFailed;
      }
      catch ( const saga::exception & e )
      {
        std::cerr << "saga thread exception: " << e.what () << std::endl;
        thread_state_ = ThreadFailed;
      }

      // nothing more to do: close thread
#ifdef DO_THREADS
      pthread_exit (NULL);
#endif

      return NULL;
    }


    // thread_join allows to simply wait 'til the thread finishes on its own.
    // All state setting etc is done by the thread.  
    void thread::thread_wait (void)
    {
      thread_join ();
    }

    // thread_exit allows to cancel a running thread.  At the moment, that is
    // not well implemented: it simply waits for the thread to finish on its
    // own... (FIXME)
    void thread::thread_exit (void)
    {
      thread_join ();
    }


    // allow the consumer to wait for thread completion
    void thread::thread_join (void)
    {
      if ( joined_ ) return;

#ifdef DO_THREADS
      pthread_join (thread_, NULL);
#endif

      joined_ = true;
    }

    thread::state thread::thread_state (void)
    {
      // reap threads at this opportunity
      if ( thread_state_ == ThreadDone && ! joined_ )
      {
        thread_join ();
      }

      return thread_state_; 
    }

  } // namespace util

} // namespace digedag

