
#include <iostream>

#include "util/uid.hpp"

namespace digedag 
{
  namespace util
  {
    // the 'uid generator' - this int is getting increased on each uid instance
    // creation, so that a new instance gets a new id.
    static unsigned int uid_cnt_ = 1;

    uid::uid (void)
      : id_ (uid_cnt_++)
    {
    }

    // instance copy does not create a new id, but maintains the old one.  
    // NOTE: this may not be what the user wants in all cases!
    uid::uid (const uid & src)
    {
      this->id_  = src.id_;
    }

  } // namespace util

} // namespace digedag

