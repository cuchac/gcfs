
#include <vector>
#include <sstream>

#include <saga/saga.hpp>


#include "node.hpp"
#include "scheduler.hpp"
#include "util/split.hpp"
#include "util/util.hpp"


namespace digedag
{
  node::node (node_description              & nd, 
              std::string                     name, 
              boost::shared_ptr <scheduler>   scheduler,
              saga::session                   session)
    : nd_         (              nd)
    , rm_         (              "")
    , name_       (            name)
    , state_      (      Incomplete)
    , is_void_    (           false)
    , valid_task_ (           false)
    , task_       ( saga::task::New)
    , scheduler_  (       scheduler)
    , session_    (         session)
    , this_fires_ (           false)
  {
    std::stringstream ss;

    if ( ! nd.attribute_exists (node_attributes::executable) )
    {
      throw ("Cannot handle node w/o executable");
    }

    ss << nd.get_attribute
      (node_attributes::executable);

    if ( nd.attribute_exists (node_attributes::arguments) )
    {
      std::vector <std::string> args = nd.get_vector_attribute (node_attributes::arguments);

      for ( unsigned int i = 0; i < args.size (); i++ )
      {
        ss << " " << args[i];
      }

      cmd_ = ss.str ();
    }
  }

  node::node (std::string                   cmd, 
              std::string                   name,
              boost::shared_ptr <scheduler> scheduler,
              saga::session                 session)
    : rm_         (              "")
    , cmd_        (             cmd)
    , name_       (            name)
    , state_      (      Incomplete)
    , is_void_    (           false)
    , valid_task_ (           false)
    , task_       ( saga::task::New)
    , scheduler_  (       scheduler)
    , session_    (         session)
    , this_fires_ (           false)
  {
    // parse cmd into node description
    std::vector <std::string> elems = split (cmd_);

    nd_.set_attribute (node_attributes::executable, elems[0]);
    nd_.set_attribute (node_attributes::interactive, saga::attributes::common_false);

    elems.erase (elems.begin ());

    nd_.set_vector_attribute (node_attributes::arguments, elems);
  }

  node::node (boost::shared_ptr <scheduler> scheduler, 
              saga::session                 session)
    : rm_         (              "")
    , cmd_        (             "-")
    , name_       (          "void")
    , state_      (      Incomplete)
    , is_void_    (            true)
    , valid_task_ (           false)
    , task_       ( saga::task::New)
    , scheduler_  (       scheduler)
    , session_    (         session)
    , this_fires_ (           false)
  {
  }

  node::~node (void)
  {
    util::scoped_lock (mtx_);

    // We need to wait 'til fire of depending edge is done - the only unlocked
    // piece of code
    // FIXME: yes yes, should use cond var
    while ( this_fires_ )
    {
      util::ms_sleep (100);
    }

    std::cout << " === node destructed" << std::endl;
  }

  void node::set_name (const std::string name)
  {
    util::scoped_lock (mtx_);
    name_ = name;
  }

  void node::add_edge_in (boost::shared_ptr <edge> e)
  {
    util::scoped_lock (mtx_);
    edges_in_.push_back (e);

    // set initial input edge state
    edges_state_[e->get_name ()] = e->get_state ();
  }

  void node::add_edge_out (boost::shared_ptr <edge> e)
  {
    util::scoped_lock (mtx_);
    edges_out_.push_back (e);
  }


  void node::dryrun (void)
  {
    util::scoped_lock (mtx_);
    // check if all input data are ready
    std::map <std::string, state> :: iterator begin = edges_state_.begin ();
    std::map <std::string, state> :: iterator end   = edges_state_.end   ();
    std::map <std::string, state> :: iterator it;

    for ( it = begin; it != end; it++ )
    {
      if ( Done != it->second )
      {
        return;
      }
    }

    // update state
    get_state ();

    if ( Pending != state_ )
      return;

    std::cout << std::string ("         node : ") << get_name () << "   \t -> " << cmd_ << std::endl;

    state_ = Done;

    for ( unsigned int i = 0; i < edges_out_.size (); i++ )
    {
      edges_out_[i]->dryrun ();
    }
  }


  void node::reset (void)
  {
    util::scoped_lock (mtx_);

    state_ = Incomplete;

    for ( unsigned int i = 0; i < edges_out_.size (); i++ )
    {
      edges_out_[i]->reset ();
    }
  }


  // this call starts the sequence which will eventually run the node's job
  // (application).  The edge parameter points to the edge which actually fired
  // this node, so that the node can keep track of the state for its incoming
  // edges.  Only if all those edges are Done, fire will actually *do* anything.
  //
  // Nodes can also be fired by the dag itself (thisnk INPUT node). A void edge
  // is then given as parameter.
  void node::fire (boost::shared_ptr <edge> e)
  {
    util::scoped_lock (mtx_);

    if ( state_ == Stopped ) return;

    // std::cout << std::string (" ===     node ") << get_name () << ": " 
    //           << e->get_name () << " fired me: " << state_to_string (e->get_state ()) 
    //           << std::endl;

    // store state of firing edge
    edges_state_[e->get_name ()] = Done;

    // proceed with the normal fire procedure
    fire ();
  }


  void node::fire (void)
  {
    util::scoped_lock (mtx_);

    // update own state - we got fired by an edge most likely, and state needs
    // to be updated for that edge at least.
    get_state ();

    // Check if node is indeed ready to go
    if ( Pending != state_ )
    {
      std::cout << std::string (" ===     node ") << get_name () << ": " 
                << state_to_string (state_) << std::endl;
      return;
    }

    // all input edges are Done, i.e. all input data are available.  We
    // can thus really execute the node application.
    // 
    // ### scheduler hook - leave it to the scheduler to call our work routine
    boost::shared_ptr <node> me = shared_from_this ();
    assert (me);

    if ( scheduler_->hook_node_run_pre (me) )
    {
      std::cout << std::string (" ===     node ") << get_name () 
                << " [" << state_to_string (state_) << "] "
                << " (" << get_cmd () << ") " 
                << " was accepted by scheduler" 
                << std::endl;
    }
    else
    {
      std::cout << std::string (" ===     node ") << get_name () 
                << " [" << state_to_string (state_) << "] "
                << " (" << get_cmd () << ") " 
                << " was declined by scheduler" 
                << std::endl;
    }
  }


  saga::task node::work_start (void)
  {
    util::scoped_lock (mtx_);

    if ( state_ == Stopped ) return task_;

    assert ( state_ == Pending );


    // we have work to do, and scheduler lets us go ahead.  We are finally
    // Running
    state_ = Running;

    
    if ( is_void_ )
    {
      // fake a noop task, which does nothing: simply returnh the empty
      // Done task...
      task_ = saga::task (saga::task::Done);

      std::cout << " === node run : " 
                << get_name ()
                << " (void): "
                << state_to_string (state_)
                << " - " << task_.get_id ()
                << std::endl;
    }
    else
    {
      assert ( nd_.attribute_exists ("Executable") );

      try 
      {
        saga::job::description jd (nd_);

     // jd.set_attribute (saga::job::attributes::description_working_directory,  "/tmp/0/");
     // jd.set_attribute (saga::job::attributes::description_interactive,  "true");
     // jd.set_attribute (saga::job::attributes::description_input,        "/dev/null");
     // jd.set_attribute (saga::job::attributes::description_output,       
     //                   std::string ("/tmp/out.") + get_id ());
     // jd.set_attribute (saga::job::attributes::description_error,       
     //                   std::string ("/tmp/err.") + get_id ());
 
        saga::job::service js (session_, rm_);
        saga::job::job     j = js.create_job (jd);

        j.run  ();

        task_ = j;

        std::cout << " === node run : " 
                  << get_name ()
                  << " (void): "
                  << state_to_string (state_)
                  << " - " << task_.get_id ()
                  << std::endl;
      }
      catch ( const saga::exception & e )
      {
        std::cout << " === running node " << get_name () << " failed: \n" 
                  << e.what () << std::endl;
        state_ = Failed;

        task_ = saga::task (saga::task::Failed);
      }
    }

    valid_task_ = true;
    return task_;
  }


  void node::work_done (void)
  {
    // scope for scoped lock
    {
      util::scoped_lock (mtx_);

      if ( state_ == Stopped ) return;

      // we don't assert here for state != Done and state_ != Failed, as
      // get_state may have set these states meanwhile, looking at the job
      // state.
      
      state_ = Done;

      std::cout << std::string (" === node done: ")
                << get_name () 
                << " (" << get_cmd () << ") " 
                << std::endl;

      // we report done before we call the edge's fire, to be able to release the
      // lock (see below)
      // ### scheduler hook
      scheduler_->hook_node_run_done (shared_from_this ());

      // only one thread will fire the depending edges (this code is locked)
      if ( ! this_fires_ )
      {
        this_fires_ = true;
      }
    }
    // scope for scoped lock


    // we don't lock the node here anymore, but the 'this_fires' flag should allow
    // to run the edge fire only once (see above).  Not that it matters, as the 
    // edge can deal with multiple ones, but anyway.
    if ( this_fires_ )
    {
      // get data staged out, e.g. fire outgoing edges
      for ( unsigned int i = 0; i < edges_out_.size (); i++ )
      {
        std::cout << " === node " << get_id () << " fires edge " 
                  << edges_out_[i]->get_name () << std::endl;

        edges_out_[i]->fire (shared_from_this ());
      }

      // signal that fire is done
      this_fires_ = false;
    }

    return;
  }

  void node::work_failed (void)
  {
    util::scoped_lock (mtx_);

    if ( state_ == Stopped ) return;

    assert ( state_ != Done );

    try 
    {
      if ( task_.get_state () == saga::task::Failed )
      {
        // task_.rethrow ();
        std::cout << cmd_ << std::endl;
        saga::no_success e (task_, "job failed for unknown reason");
        throw e;
      }
    }
    catch ( const saga::exception & e )
    {
      std::cout << " === node " << get_name () 
        << " set to failed by scheduler: \n"
        << e.what () 
        << std::endl;
    }

    state_ = Failed;
  }


  void node::stop ()
  {
    util::scoped_lock (mtx_);

    if ( task_.get_state () == saga::task::Running )
    {
      task_.cancel ();
    }

    state_ = Stopped;
  }

  void node::dump (void)
  {
    util::scoped_lock (mtx_);

    std::cout << " ### node " << get_name () 
              << " [" << host_ << ":" << pwd_  << " : " << cmd_ << "]" 
              << " (" << state_to_string (get_state ()) << ")" << std::endl;

    for ( unsigned int i = 0; i < edges_in_.size (); i++ )
    {
      edges_in_[i]->dump ();
    }
  }

  std::string node::get_id (void) const
  {
    util::scoped_lock (mtx_);

    return name_;
  }

  std::string node::get_name (void) const
  {
    util::scoped_lock (mtx_);

    return name_;
  }

  node_description node::get_description (void) const
  {
    util::scoped_lock (mtx_);

    return nd_;
  }

  void node::set_state (state s)
  {
    util::scoped_lock (mtx_);

    state_ = s;
  }


  // FIXME: it is not nice to have such fundamental side effects in get_state!
  // That code needs to eventually move into a callback on the job state metric.
  state node::get_state (void)
  {
    util::scoped_lock (mtx_);

    // std::cout << " === node " << get_name () << " : state before check " << state_to_string (state_) << std::endl;

    // final states just return
    if ( state_ == Stopped ||
         state_ == Done    ||
         state_ == Failed  )
    {
      // std::cout << " === node " << get_name () << " is in final state" << std::endl; 
      return state_;
    }

    if ( state_ == Incomplete )
    {
      // check if any input data failed
      std::map <std::string, state> :: iterator begin = edges_state_.begin ();
      std::map <std::string, state> :: iterator end   = edges_state_.end   ();
      std::map <std::string, state> :: iterator it;

      for ( it = begin; it != end; it++ )
      {
        if ( Failed == it->second )
        {
          // std::cout << " !!! node " << get_name () << " failed due to edge " << it->first << std::endl;
          state_ = Failed;
          return state_;
        }
      }

      // check if all input data are ready
      for ( it = begin; it != end; it++ )
      {
        if ( Done != it->second )
        {
          // std::cout << " === node " << get_name () << " : input '"
          //            << it->first << "' is missing: " << state_to_string (it->second) << std::endl; 
          state_ = Incomplete;
          return state_;
        }
      }

      // no dep failed, all Done - we are pending!
      // std::cout << " === node " << get_name () 
      //           << " : no input missing: now Pending!" << std::endl; 
      state_ = Pending;
    }

    // we can only depend from the job state if a job was
    // actually created
    if ( valid_task_ )
    {
      // std::cout << " === node " << get_name () 
      //           << " : checking task state" << std::endl; 
      switch ( task_.get_state () )
      {
        case saga::job::New:
          // std::cout << " === node is almost running: " << get_name () << std::endl;
          state_ = Pending;
          break;

        case saga::job::Running:
          // std::cout << " === node is running: " << get_name () << std::endl;
          state_ = Running;
          break;

        case saga::job::Done:
          state_ = Done;
          scheduler_->hook_node_run_done (shared_from_this ());
          break;

        // Canceled, Failed, Unknown, New - all invalid
        default:
          state_ = Failed;
          std::cout << std::string ("       node ") << get_name () 
                    << " : job failed - cancel: " << cmd_ << std::endl;

          // ### scheduler hook
          scheduler_->hook_node_run_fail (shared_from_this ());

          break;

      } // switch
    }

    // std::cout << " === node " << get_name () << " : state after  check " 
    //           << state_to_string (state_) << std::endl;

    return state_;
  }

  void node::set_pwd (std::string pwd)
  {
    util::scoped_lock (mtx_);

    pwd_ = pwd;
    saga::url u_pwd (pwd_);

    nd_.set_attribute (node_attributes::working_directory, u_pwd.get_path ());

    // set pwd for all incoming and outgoing edges
    for ( unsigned int i = 0; i < edges_in_.size (); i++ )
    {
      edges_in_[i]->set_pwd_tgt (pwd);
    }

    for ( unsigned int i = 0; i < edges_out_.size (); i++ )
    {
      edges_out_[i]->set_pwd_src (pwd);
    }
  }

  void node::set_rm (std::string rm)
  {
    util::scoped_lock (mtx_);

    // std::cout << " === setting rm   to " << rm   << std::endl;
    rm_ = rm;
  }


  void node::set_host (std::string host)
  {
    util::scoped_lock (mtx_);

    host_ = host;

    // std::cout << " === setting host to " << host << std::endl;

    std::vector <std::string> chosts;
    chosts.push_back (host);
    nd_.set_vector_attribute (node_attributes::candidate_hosts, chosts);

    // set host for all incoming and outgoing edges
    for ( unsigned int i = 0; i < edges_in_.size (); i++ )
    {
      edges_in_[i]->set_host_tgt (host);
    }

    for ( unsigned int i = 0; i < edges_out_.size (); i++ )
    {
      edges_out_[i]->set_host_src (host);
    }
  }


  void node::set_path (std::string path)
  {
    util::scoped_lock (mtx_);

    path_ = path;

    std::vector <std::string> new_env;

    if ( nd_.attribute_exists (node_attributes::environment) )
    {
      std::vector <std::string> old_env = nd_.get_vector_attribute (node_attributes::environment);
      bool found = false;

      for ( unsigned int i = 0; i < old_env.size (); i++ )
      {
        std::vector <std::string> words = split (old_env[i], "=");

        if ( words[0] == "PATH" )
        {
          if ( path.empty () )
          {
            new_env.push_back (old_env[i]);
          }
          else
          {
            if ( words.size () == 1 )
            {
              new_env.push_back (words[0] + "=" + path);
            }
            {
              new_env.push_back (words[0] + "=" + words[1] + ":" + path);
            }
          }
          found = true;
        }
        else 
        {
          // not PATH
          new_env.push_back (old_env[i]);
        }
      }
      if ( ! found )
      {
        if ( ! path.empty () )
        {
          new_env.push_back (std::string ("PATH=") + path);
        }
      }
    }
    else
    {
      // no env at all
      if ( ! path.empty () )
      {
        new_env.push_back (std::string ("PATH=") + path);
      }
    }

    // replace env
    nd_.set_vector_attribute (node_attributes::environment, new_env);

    // FIXME: condor adaptor does not evaluate path.  Thus, we set the
    // executable name to absolute file name
    // Note that this mechanism required rm_ to be set first
    if ( nd_.attribute_exists (node_attributes::executable) &&
         saga::url (rm_).get_scheme () == "condor" )
    {
      if ( ! path.empty () )
      {
        nd_.set_attribute (node_attributes::executable,
                           path + "/" + nd_.get_attribute (node_attributes::executable));
      }
    }
  }

  std::string node::get_cmd (void)
  {
    util::scoped_lock (mtx_);

    if ( is_void_ ) return "void";

    std::string out (nd_.get_attribute ("Executable"));

    if ( nd_.attribute_exists ("Arguments") )
    {
      std::vector <std::string> args = nd_.get_vector_attribute ("Arguments");

      for ( unsigned int i = 0; i < args.size (); i++ )
      {
        out += " " + args[i];
      }
    }

    return out;
  }

} // namespace digedag

