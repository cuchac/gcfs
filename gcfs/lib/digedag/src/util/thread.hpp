
#ifndef DIGEDAG_UTIL_THREAD_HPP
#define DIGEDAG_UTIL_THREAD_HPP

#include <iostream>

#include <pthread.h>

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
      void * thread_startup_ (void * arg);
    }

    // This class provides simple threading ability.  The consumer inherits
    // this class, and overloads the following method
    //   void thread_work (void)
    //
    class thread
    {
      public:
        // thread states
        enum state
        {
          ThreadNew       = 0,
          ThreadRunning   = 1,
          ThreadDone      = 2,
          ThreadFailed    = 3, 
          ThreadCancelled = 4
        };

      private:
        // state management
        state         thread_state_;
        pthread_t     thread_;
        bool          joined_;


      public:
        thread (void);
      //thread (thread & t);
        virtual ~thread (void);


      private:
        // the external "C" startup helper is a friend, so that it can
        // run thread_start_
        friend void * thread_startup_ (void * arg);

      protected:
        // thread startup method, which manages the workload
        void * thread_start  (void);

        // workload method, to be overloaded by consumer
        virtual void thread_work (void) { };


      public:
        // public thread management calls
        void          thread_run         (void);
        void          thread_wait        (void);
        void          thread_join        (void);
        void          thread_exit        (void);
        state         thread_state       (void);
    };

  } // namespace util

} // namespace digedag


#endif // DIGEDAG_UTIL_THREAD_HPP

