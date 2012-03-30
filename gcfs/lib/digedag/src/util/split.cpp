
#include <iostream>

#include "util/split.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace digedag
{
  std::vector <std::string> split (std::string line, 
                                   std::string delim, 
                                   int         number)
  {
    std::vector <std::string> list;

    if ( 0 == number )
    {
      // nothing to do
      return list;
    }

    boost::split (list, line, boost::is_any_of (delim),
                  boost::token_compress_on); 

    if ( 0 > number )
    {
      // we don't care about the number of found elements
      return list;
    }


    if ( number == (int) list.size () )
    {
      // perfect
      return list;
    }
    else if ( number > (int) list.size () )
    {
      // just fill up with empty fields
      for ( int i = list.size () ; i < number; i++ )
      {
        list.push_back ("");
      }

      return list;
    }
    else if ( number < (int) list.size () )
    {
      // concatenate the superflous elements to the last element

      // we can only append sensibly if we have a single delimiter
      if ( number >  (int) list.size  () &&
           1      !=       delim.size () )
      {
        std::cerr << "Warning: limit number of words for split on "
                  << "multiple delimiters - using first char (" 
                  << delim[0] 
                  << ")" 
                  << std::endl;
      }


      std::vector <std::string> new_list;
      int i = 0;

      // copy number elements
      for ( i = 0; i < number; i++ )
      {
        new_list.push_back (list[i]);
      }

      // append remaing ones to last elem
      for ( /* continue from last loop */ ; i < (int) list.size (); i++ )
      {
        new_list[new_list.size () - 1] += delim[0];
        new_list[new_list.size () - 1] += list[i];
      }

      return new_list;
    }

    // keep compiler happy - we'll never get here
    return list;
  }

} // namespace digedag

