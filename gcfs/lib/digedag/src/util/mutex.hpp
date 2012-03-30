
#ifndef DIGEDAG_UTIL_MUTEX_HPP
#define DIGEDAG_UTIL_MUTEX_HPP

#define USE_BOOST

#ifdef USE_BOOST
# include <boost/thread/recursive_mutex.hpp>
#else
# include <pthread.h>
#endif

// FIXME: check mutex call return values

namespace digedag 
{
  namespace util
  {
    // This class provides a wrapper around a pthread mutex's, and allows to
    // lock/unlock them.  The class should be exception save, i.e. the mutex
    // gets destroyed if the classes destructor is called.
#ifdef USE_BOOST
    
    typedef boost::recursive_mutex mutex;

#else // USE_BOOST

    class mutex 
    {
      private:
        pthread_mutex_t * mtx_;
         

      public:
        mutex (void)
        {
          mtx_ = new pthread_mutex_t;
          assert (mtx_);
          pthread_mutex_init (mtx_, NULL);
        }

        mutex (const mutex & src)
        {
          this->mtx_ = src.mtx_;
        }

        // dtor
        ~mutex (void)
        {
          assert (mtx_);
          pthread_mutex_destroy (mtx_);
        }

        void lock (void) 
        {
          assert (mtx_);
          pthread_mutex_lock (mtx_);
        }

        void unlock (void) 
        {
          assert (mtx_);
          pthread_mutex_unlock (mtx_);
        } 

        pthread_mutex_t get (void)
        {
          return *mtx_;
        }
    };

#endif // USE_BOOST

  } // namespace util

} // namespace digedag

#endif // DIGEDAG_UTIL_MUTEX_HPP

