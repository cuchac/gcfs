
#include <saga/saga.hpp>

#include "edge.hpp"
#include "scheduler.hpp"
#include "util/util.hpp"

namespace digedag
{
  edge::edge (const saga::url               & src, 
              const saga::url               & tgt, 
              boost::shared_ptr <scheduler>   scheduler, 
              saga::session                   session, 
              edge_id_t                       id)
    : src_url_    (             src)
    , tgt_url_    (             tgt)
    , state_      (      Incomplete)
    , src_state_  (      Incomplete)
    , is_void_    (           false)
    , optimize_   (           false)
    , task_valid_ (           false)
    , task_       ( saga::task::New)
    , scheduler_  (       scheduler)
    , session_    (         session)
    , id_         (              id)
    , this_fires_ (           false)
  {
    if ( tgt_url_ == "" )
    {
      tgt_url_  = src_url_;
    }
  }

  edge::edge (boost::shared_ptr <scheduler>   scheduler, 
              saga::session                   session,
              edge_id_t                       id)
    : src_url_    (              "")
    , tgt_url_    (              "")
    , state_      (      Incomplete)
    , src_state_  (      Incomplete)
    , is_void_    (            true)
    , optimize_   (           false)
    , task_valid_ (           false)
    , task_       ( saga::task::New)
    , scheduler_  (       scheduler)
    , session_    (         session)
    , id_         (              id)
    , this_fires_ (           false)
  {
  }

  edge::~edge (void)
  {
    util::scoped_lock (mtx_);

    // We need to wait 'til fire of depending node is done - the only unlocked
    // piece of code
    // FIXME: yes yes, should use cond var
    while ( this_fires_ )
    {
      util::ms_sleep (100);
    }

    std::cout << " === edge destructed " << get_name () << std::endl;
  }

  bool edge::operator== (const edge & e)
  {
    util::scoped_lock (mtx_);

    if ( src_url_   == e.src_url_     &&
         tgt_url_   == e.tgt_url_     &&
         state_     == e.state_       &&
         src_node_  == e.src_node_    &&
         tgt_node_  == e.tgt_node_    &&
         scheduler_ == e.scheduler_   )
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  void edge::add_src_node (boost::shared_ptr <node> src)
  {
    util::scoped_lock (mtx_);

    src_node_ = src;

    // std::cout << " setting src node for edge: " << src->get_name () << std::endl;

    if ( src_node_ && tgt_node_ )
    {
      is_void_ = false;
    }
    else
    {
      is_void_ = true;
    }
  }

  void edge::add_tgt_node (boost::shared_ptr <node> tgt)
  {
    util::scoped_lock (mtx_);

    tgt_node_ = tgt;

    // std::cout << " setting tgt node for edge: " << tgt->get_name () << std::endl;

    if ( src_node_ && tgt_node_ )
    {
      is_void_ = false;
      // std::cout << " edge is no longer void after tgt node add" << std::endl;
    }
    else
    {
      is_void_ = true;
      // std::cout << " edge remains void despite tgt node?" << std::endl;
    }
  }

  void edge::dryrun (void)
  {
    util::scoped_lock (mtx_);

    if ( state_ == Stopped )
      return;

    if ( Pending != get_state () )
      return;

    if ( src_url_ != tgt_url_ )
    {
      dump ();
    }

    state_ = Done;

    if ( tgt_node_ )
    {
      tgt_node_->dryrun ();
    }
  }


  void edge::reset (void)
  {
    util::scoped_lock (mtx_);

    state_ = Incomplete;

    if ( tgt_node_ )
    {
      tgt_node_->reset ();
    }
  }


  // fire() checks if there is still work to do, and if so, starts
  // a thread to do it.
  void edge::fire (boost::shared_ptr <node> n)
  {
    util::scoped_lock (mtx_);

    if ( state_ == Stopped ) return;

    // update node_state.  If node fired, its obviously Done.
    src_state_ = Done;

    // update own state
    get_state ();

    if ( Pending != state_ )
    {
      // std::cout << " edge " << get_name () << " is not pending" << std::endl;
      return;
    }

    // std::cout << " edge " << get_name () << " fired" << std::endl;

    // ### scheduler hook
    scheduler_->hook_edge_run_pre (shared_from_this ());
  }

  saga::task edge::work_start (void)
  {
    if     ( state_ == Stopped ) return task_;
    assert ( state_ == Pending );

    util::scoped_lock (mtx_);

    // we have work to do...
    state_ = Running;

    // lets see if we actually need to do anything
    try 
    {
      saga::filesystem::file f_tgt (session_, tgt_url_);
      saga::filesystem::file f_src (session_, src_url_);

      if ( f_src.get_size () == f_tgt.get_size () )
      {
        optimize_ = true;
      }
    }
    catch ( const saga::exception & e )
    {
      // well, we need to run the edge operation to see what's missing...
      optimize_ = false;
    }


    if ( optimize_ || is_void_ )
    {
      // fake a noop task, which does nothing: simply returnh the empty
      // Done task...
      task_ = saga::task (saga::task::Done);

      std::cout << " === edge run : " << get_name () 
                << " (" << src_url_ << " -> " << tgt_url_ << ")"
                << " [optimized] - " << task_.get_id () 
                << std::endl;

    }
    else
    {
      saga::filesystem::file f_src (session_, src_url_);

      task_ = f_src.copy <saga::task::Async> (tgt_url_, saga::filesystem::Overwrite
                                                      | saga::filesystem::CreateParents);

      std::cout << " === edge run : " << get_name () 
                << " (" << src_url_ << " -> " << tgt_url_ << ") - "
                << task_.get_id () 
                << std::endl;
    }

    task_valid_ = true;
    return task_;
  }


  void edge::work_done (void)
  {
    // scope for scoped lock
    {
      util::scoped_lock (mtx_);
      // std::cout << " === edge done?" << std::endl;

      if ( state_ == Stopped ) return;


      // we don't assert here for state != Done and state_ != Failed, as
      // get_state may have set these states meanwhile, looking at the
      // task state.

      state_ = Done;

      std::cout << " === edge done: " << get_name () 
                << " (" << src_url_ << " -> " << tgt_url_ << ") : "
                << state_to_string (state_)
                << std::endl;

      // only one thread will fire the depending edges (this code is locked)
      if ( ! this_fires_ )
      {
        this_fires_ = true;
      }
    }
    // scope for scoped lock

    if ( tgt_node_ )
    {
      if ( this_fires_ )
      {
        // if we are done copying data, we fire the dependend node
        // this fire may succeed or not - that depends on the availability
        // of _other_ input data to that node.  Only if all data are Done,
        // the fire will actually do anything.  Thus, only the last fire
        // called on a node (i.e. called from its last Pending Edge) will
        // result in a Running node.
        std::cout << " === firing dep node " << tgt_node_->get_id () << std::endl;
        tgt_node_->fire (shared_from_this ());

        // signal that fire is done
        this_fires_ = false;
      }
    }

    // ### scheduler hook
    scheduler_->hook_edge_run_done (shared_from_this ());

    return;
  }

  void edge::work_failed (void)
  {
    if ( state_ == Stopped ) return;

    assert ( state_ != Failed );
    assert ( state_ != Done   );

    util::scoped_lock (mtx_);

    try 
    {
      if ( task_.get_state () == saga::task::Failed )
      {
        task_.rethrow ();
      }
    }
    catch ( const saga::exception & e )
    {
      std::cout << " === edge " << get_name () 
                << " set to failed by scheduler: \n"
                << e.what () 
                << std::endl;
    }
  
    state_ = Failed;
  }


  void edge::stop (void)
  {
    util::scoped_lock (mtx_);

    if ( task_.get_state () == saga::task::Running )
    {
      task_.cancel ();
    }

    state_ = Stopped;
  }

  void edge::dump (void)
  {
    util::scoped_lock (mtx_);

    std::cout << " -------- edge "
              << get_name ()
              << " [" << get_src ().get_string ()   << "\t -> " << get_tgt ().get_string () << "] "
              << " (" << state_to_string (state_) << ")" 
              << std::endl;
  }

  void edge::erase_src (void)
  {
    util::scoped_lock (mtx_);
    // FIXME: remove the src data
  }

  void edge::erase_tgt (void)
  {
    util::scoped_lock (mtx_);
    // FIXME: remove the tgt data
  }

  void edge::set_state (state s)
  {
    util::scoped_lock (mtx_);
    state_ = s;
  }

  state edge::get_state (void)
  {
    util::scoped_lock (mtx_);

    if ( is_void_ )
    {
      // std::cout << " === edge " << get_name () << " is void and Done" << std::endl;
      return Done;
    }

    // final states just return
    if ( state_ == Stopped ||
         state_ == Done    ||
         state_ == Failed  )
    {
      // std::cout << " === edge " << get_name () << " reporting final state" << std::endl;
      return state_;
    }


    // Incomplete state may be left into Pending, if src is done, or
    // into Failed, if src failed.
    if ( Incomplete == state_ )
    {
      if ( Done == src_state_ )
      {
        state_ = Pending;
      }
      else if ( Failed == src_state_ )
      {
    //  std::cout << " === edge " << get_name () 
    //            << " failed due to failing src node " << src_node_->get_name () 
    //            << std::endl;
        state_ = Failed;
      }
    }

    return state_;
  }

  // FIXME: names are not unique, yet!  Collision occurs when multiple data are
  // exchanged between the same pair of nodes.
  std::string edge::get_name (void) const
  {
    util::scoped_lock (mtx_);

    if ( is_void_ )
    {
      std::cout << " --- VOID EDGE" << std::endl;
      return "void_edge";
    }

    std::string src_string = src_node_->get_name ();
    std::string tgt_string = tgt_node_->get_name ();

    return src_string + "->" + tgt_string;
  }


  edge_id_t edge::get_id   (void) const
  {
    util::scoped_lock (mtx_);
    return id_;
  }

  void edge::set_pwd_src (std::string pwd)
  {
    util::scoped_lock (mtx_);
    src_url_.set_path (pwd  + src_url_.get_path ());
  }

  void edge::set_pwd_tgt (std::string pwd)
  {
    util::scoped_lock (mtx_);
    tgt_url_.set_path (pwd  + tgt_url_.get_path ());
  }

  void edge::set_host_src (std::string host) 
  {
    util::scoped_lock (mtx_);
    src_url_.set_host (host);
  }

  void edge::set_host_tgt (std::string host) 
  {
    util::scoped_lock (mtx_);
    tgt_url_.set_host (host);
  }

} // namespace digedag

