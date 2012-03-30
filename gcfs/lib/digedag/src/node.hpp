
#ifndef DIGEDAG_NODE_HPP
#define DIGEDAG_NODE_HPP

#include <vector>

#include <saga/saga.hpp>

#include "config.hpp"
#include "enum.hpp"
#include "dag.hpp"
#include "node.hpp"
#include "edge.hpp"
#include "node_description.hpp"


namespace digedag
{
  class scheduler;
  class node : public util::enable_shared_from_this <node>
  {
    private:
      node_description                        nd_;          // node application to run

      std::vector <boost::shared_ptr <edge> > edges_in_;    // input  data
      std::vector <boost::shared_ptr <edge> > edges_out_;   // output data
      std::map    <std::string, state>        edges_state_; // states of incoming edges

      std::string                             rm_;
      std::string                             cmd_;
      std::string                             pwd_;
      std::string                             host_;
      std::string                             path_;
      std::string                             name_;        // instance name
      state                                   state_;       // instance state


      bool                                    is_void_;     // void node?
      bool                                    valid_task_;  // a saga_task object got 
                                                            // created and can be queried for state

      saga::task                              task_;        // our async workload

      boost::shared_ptr <scheduler>           scheduler_;   
      saga::session                           session_;     // session from scheduler

      bool                                    this_fires_;
      util::mutex                             mtx_;

      std::string   get_cmd (void);


    public:
      node  (node_description              & nd, 
             std::string                     name, 
             boost::shared_ptr <scheduler>   scheduler, 
             saga::session                   session);
            
      node  (std::string                     cmd,
             std::string                     name, 
             boost::shared_ptr <scheduler>   scheduler, 
             saga::session                   session);

      node  (boost::shared_ptr <scheduler>   scheduler,
             saga::session                   session);

      ~node (void);

      void             set_name        (std::string              name);
      void             add_edge_in     (boost::shared_ptr <edge> e);
      void             add_edge_out    (boost::shared_ptr <edge> e);

      void             dryrun          (void);
      void             reset           (void);
      void             fire            (void);
      void             fire            (boost::shared_ptr <edge> e);
      void             stop            (void);
      void             dump            (void);
      saga::task       work_start      (void);
      void             work_done       (void);
      void             work_failed     (void);
      std::string      get_id          (void) const;
      std::string      get_name        (void) const;
      node_description get_description (void) const;
      void             set_state       (state s);
      state            get_state       (void);
      void             set_pwd         (std::string pwd);
      void             set_rm          (std::string rm);
      void             set_host        (std::string host);
      void             set_path        (std::string path);

      std::vector <boost::shared_ptr <edge> > 
                       get_edges_in    (void) const
      {
        return edges_in_;
      }

      std::vector <boost::shared_ptr <edge> > 
                       get_edges_out   (void) const
      {
        return edges_out_;
      }

  };

} // namespace digedag

#endif // DIGEDAG_NODE_HPP

