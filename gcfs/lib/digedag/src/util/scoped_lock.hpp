#ifndef DIGEDAG_UTIL_SCOPEDLOCK_HPP
#define DIGEDAG_UTIL_SCOPEDLOCK_HPP

#define USE_BOOST

#include "util/mutex.hpp"


namespace digedag 
{
  namespace util
  {

#ifdef USE_BOOST

    typedef boost::recursive_mutex::scoped_lock scoped_lock;

#else // USE_BOOST

    // This class provides a simple scoped lock, based on the
    // util::mutex class.
    class scoped_lock 
    {
      private:
        util::mutex mtx_;

      public:
        scoped_lock (void)
        {
          mtx_.lock ();
        }

        // sometimes, the user provides its own mutex for locking
        scoped_lock (util::mutex & mtx)
        {
          mtx_ = mtx;
          mtx_.lock ();
        }

        ~scoped_lock () 
        {
          mtx_.unlock ();
        }
    };

#endif // USE_BOOST

  } // namespace util

} // namespace digedag


#endif // DIGEDAG_UTIL_SCOPEDLOCK_HPP

