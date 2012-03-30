

#include <math.h>
#include <time.h>
#include <errno.h>
#include <strings.h>
#include <string.h>

#include <iostream>

#include "util.hpp"

namespace digedag 
{
  namespace util
  {
    void ms_sleep (double milliseconds)
    {
      struct timespec delay;
      struct timespec remain;

      double nanoseconds = milliseconds * 1000000;    // 10^6
      double seconds     = milliseconds / 1000;       // 10^9

      seconds     = ::floor       (nanoseconds  / 1000000000); // 10^9
      nanoseconds = nanoseconds - (seconds      * 1000000000);

      delay.tv_sec  = 1;
      delay.tv_nsec = nanoseconds;

      while ( -1 == ::nanosleep (&delay, &remain) )
      {
        if ( EINTR == errno )
        {
          if ( remain.tv_sec  > 0          &&
               remain.tv_nsec > 0          &&
               remain.tv_nsec < 1000000000 )
          {
            delay = remain;
          }
          else
          {
            break;
          }
        }
        else
        {
          throw (std::string ("nanosleep failed: ") + ::strerror (errno));
        }
      }
    }

  } // namespace util

} // namespace digedag

