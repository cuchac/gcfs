
#ifndef DIGEDAG_EDGE_HPP
#define DIGEDAG_EDGE_HPP

#include <vector>

#include <saga/saga.hpp>

#include "config.hpp"
#include "enum.hpp"
#include "dag.hpp"
#include "node.hpp"


namespace digedag
{
  class scheduler;
  class edge : public util::enable_shared_from_this <edge>
  {
    private:
      saga::url                               src_url_;     // src location of data
      saga::url                               tgt_url_;     // tgt location of data

      state                                   state_;       // state of instance

      boost::shared_ptr <node>                src_node_;
      boost::shared_ptr <node>                tgt_node_;

      state                                   src_state_;   // state of src node
                                                
      bool                                    is_void_;     // void edge?
      bool                                    optimize_;    // src == tgt?
      bool                                    task_valid_;  // a saga_task object got 
                                                            // created and can be queried for state

      saga::task                              task_;        // our async workload

      boost::shared_ptr <scheduler>           scheduler_;
      saga::session                           session_;     // session from scheduler

      edge_id_t                               id_;          // id assigned by dag

      bool                                    this_fires_;
      util::mutex                             mtx_;

    protected:
      saga::url & get_src_url (void) { return src_url_; }
      saga::url & get_tgt_url (void) { return tgt_url_; }
      friend class scheduler;


    public:
      edge  (const saga::url               & src, 
             const saga::url               & tgt,
             boost::shared_ptr <scheduler>   scheduler, 
             saga::session                   session, 
             edge_id_t                       id);

      edge  (boost::shared_ptr <scheduler>   scheduler,
             saga::session                   session, 
             edge_id_t                       id);

      ~edge (void);

      bool operator== (const edge & e);
      

      void                     dryrun        (void);
      void                     reset         (void);
      void                     fire          (boost::shared_ptr <node> n);
      void                     stop          (void);
      void                     dump          (void);
      saga::task               work_start    (void);
      void                     work_done     (void);
      void                     work_failed   (void);
      void                     erase_src     (void);
      void                     erase_tgt     (void);
      void                     add_src_node  (boost::shared_ptr <node> src);
      void                     add_tgt_node  (boost::shared_ptr <node> tgt);
      void                     set_state     (state s);
      state                    get_state     (void);
      std::string              get_name      (void) const;
      edge_id_t                get_id        (void) const;

      saga::url                get_src       (void) const { return src_url_; }
      saga::url                get_tgt       (void) const { return tgt_url_; }


      boost::shared_ptr <node> get_src_node  (void) const { return src_node_; }
      boost::shared_ptr <node> get_tgt_node  (void) const { return tgt_node_; }

      void                     set_pwd_src   (std::string pwd);
      void                     set_pwd_tgt   (std::string pwd);
      void                     set_host_src  (std::string host);
      void                     set_host_tgt  (std::string host);
  };

} // namespace digedag

#endif // DIGEDAG_EDGE_HPP

