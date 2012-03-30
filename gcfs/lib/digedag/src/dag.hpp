
#ifndef DIGEDAG_DAG_HPP
#define DIGEDAG_DAG_HPP

#include <map>

#include "util/mutex.hpp"
#include "util/scoped_lock.hpp"
#include "util/thread.hpp"

#include "config.hpp"
#include "enum.hpp"
#include "node.hpp"
#include "edge.hpp"
#include "node_description.hpp"


namespace digedag
{
  // the 'dag' class represents a directed acyclic graph.  Its data model is
  // extremely simple: it maintains a list of nodes and edges.  The
  // depenedencies between them are stored implicitely: each edge knows its
  // source and target node, and each nodes knows about its incoming and
  // outgoing edges.  Whe firing (i.e. running) a dag, those nodes with
  // satisfied depencies are started, which upon completion activate their
  // outgoing edges, which activate their target nodes, etc etc.
  //
  // A dag also has a scheduler, which can traverse the dag, and change
  // attributes of edges and nodes, such as its assigment to a specific
  // resource.
  class scheduler;
  class dag : public util::enable_shared_from_this <dag>
  {
    private:
      saga::session                    session_;   // saga session to be used everywhere 

      std::map <node_id_t, node_map_t> nodes_;     // dag node names and instances
      std::map <edge_id_t, edge_map_t> edges_;     // dag edge names and instances

      state                            state_;     // see get_state ()
      boost::shared_ptr <scheduler>    scheduler_; // scheduler instance operating on the dag

      // special nodes which act as anchor for input and output edges
      boost::shared_ptr <node>         input_;     // node for data stagein 
      boost::shared_ptr <node>         output_;    // node for data stageout 

      unsigned int                     edge_cnt_;  // serves as edge id

      util::mutex                      mtx_;
      void  lock              (void) { mtx_.lock   (); };
      void  unlock            (void) { mtx_.unlock (); };


    protected:
      // allow our friend, the sxheduler, full access to the dag data.
      // FIXME: we need to make sure that the scheduler is not changing these
      // data when we are operating on them...
      std::map <node_id_t, node_map_t> get_nodes (void) { return nodes_; }
      std::map <edge_id_t, edge_map_t> get_edges (void) { return edges_; }
      friend class scheduler;


    public:
      dag  (const std::string & scheduler_src = "");
      ~dag (void); 

      boost::shared_ptr <node> create_node  (node_description & nd, 
                                             std::string        name = "");
      boost::shared_ptr <node> create_node  (std::string        cmd,
                                             std::string        name = "");
      boost::shared_ptr <node> create_node  (void);

      boost::shared_ptr <edge> create_edge  (const saga::url  & src, 
                                             const saga::url  & tgt  = "");
      boost::shared_ptr <edge> create_edge  (void);


      // create the dag
      void  add_node  (const std::string         & name, 
                       boost::shared_ptr <node>    node);
      void  add_edge  (boost::shared_ptr <edge>    e, 
                       boost::shared_ptr <node>    src, 
                       boost::shared_ptr <node>    tgt);
      void  add_edge  (boost::shared_ptr <edge>    e, 
                       const std::string         & src, 
                       const std::string         & tgt);

      // operations on a dag
      void  dryrun    (void);
      void  reset     (void);
      void  fire      (void);
      void  wait      (void);
      state get_state (void);
      void  set_state (state s);
      void  schedule  (void);

      // inspection
      void  dump      (void);
      void  dump_node (std::string name);
  };

} // namespace digedag

#endif // DIGEDAG_DAG_HPP

