
#include <sstream>

#include "config.hpp"
#include "enum.hpp"

namespace digedag
{
  std::string state_to_string (state s)
  {
    if      ( s == Incomplete ) return "Incomplete";
    else if ( s == Pending    ) return "Pending"   ;
    else if ( s == Running    ) return "Running"   ;
    else if ( s == Done       ) return "Done"      ;
    else if ( s == Failed     ) return "Failed"    ;
    else                        return "Unknown"   ;
  }

  std::string saga_state_to_string (saga::task_base::state s)
  {
    if      ( s == saga::task_base::New        ) return "saga::New"       ;
    else if ( s == saga::task_base::Running    ) return "saga::Running"   ;
    else if ( s == saga::task_base::Done       ) return "saga::Done"      ;
    else if ( s == saga::task_base::Failed     ) return "saga::Failed"    ;
    else                                         return "saga::Unknown"   ;
  }

} // namespace digedag

