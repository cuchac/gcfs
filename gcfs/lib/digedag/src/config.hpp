
#ifndef DIGEDAG_CONFIG_HPP
#define DIGEDAG_CONFIG_HPP

#include <vector>
#include <string>
#include <map>

#include "util/shared_ptr.hpp"

namespace digedag 
{
  class edge;
  class node;

  // C++ has no template typedefs :-(

  typedef std::string   node_id_t;
  typedef unsigned int  edge_id_t;
  typedef boost::shared_ptr <edge>   edge_map_t;
  typedef boost::shared_ptr <node>   node_map_t;

}

#endif // DIGEDAG_CONFIG_HPP

