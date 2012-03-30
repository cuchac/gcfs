
#ifndef DIGEDAG_UTIL_SHAREDPTR_HPP
#define DIGEDAG_UTIL_SHAREDPTR_HPP

#include <iostream>

#define USE_BOOST

#ifdef USE_BOOST
# include "util/shared_ptr_boost.hpp"
#else
# include "util/shared_ptr_noboost.hpp"
#endif

#endif // DIGEDAG_UTIL_SHAREDPTR_HPP

