
#include "dag.hpp"
#include "scheduler.hpp"
#include "util/util.hpp"

namespace digedag
{
  dag::dag (const std::string & scheduler_src)
    : session_   (saga::get_default_session ())
    , state_     (Pending)
    , scheduler_ (new scheduler (this, scheduler_src, session_))
    , input_     (new node (scheduler_, session_))
    , output_    (new node (scheduler_, session_))
    , edge_cnt_  (0)
  { 
    // add special nodes to dag already
    add_node ("INPUT",  input_);
    add_node ("OUTPUT", output_);

    // ### scheduler hook
    scheduler_->hook_dag_create ();
  }


  dag::~dag (void) 
  {
    util::scoped_lock (mtx_);

    std::cout << " === dag destructed" << std::endl;

    // ### scheduler hook
    scheduler_->hook_dag_destroy ();

    // Nodes fire edges, edges fire nodes.  No matter which we delete
    // first, we are in trouble.  Thus, we need to *stop* them all, before we
    // start deleting

    scheduler_->stop ();

    // stop nodes
    {
      std::map <node_id_t, boost::shared_ptr <node> > :: iterator it;
      std::map <node_id_t, boost::shared_ptr <node> > :: iterator begin = nodes_.begin ();
      std::map <node_id_t, boost::shared_ptr <node> > :: iterator end   = nodes_.end ();

      for ( it = begin; it != end; it++ )
      {
        it->second->stop ();
      }
    }

    // stop edges
    {
      std::map <edge_id_t, edge_map_t> :: iterator it;
      std::map <edge_id_t, edge_map_t> :: iterator begin = edges_.begin ();
      std::map <edge_id_t, edge_map_t> :: iterator end   = edges_.end ();

      for ( it = begin; it != end; it++ )
      {
        it->second->stop ();
      }
    }
    

    // ok, everything is stopped, and shared_ptr's will be destroyed here
  }

  boost::shared_ptr <node> dag::create_node (node_description & nd, 
                                             std::string        name)
  {
    util::scoped_lock (mtx_);

    return boost::shared_ptr <node> (new node (nd, name, scheduler_, session_));
  }


  boost::shared_ptr <node> dag::create_node (std::string cmd,
                                             std::string name)
  {
    util::scoped_lock (mtx_);

    return boost::shared_ptr <node> (new node (cmd, name, scheduler_, session_));
  }


  boost::shared_ptr <node> dag::create_node (void)
  {
    util::scoped_lock (mtx_);

    return boost::shared_ptr <node> (new node (scheduler_, session_));
  }


  boost::shared_ptr <edge> dag::create_edge (const saga::url & src, 
                                             const saga::url & tgt)
  {
    util::scoped_lock (mtx_);

    edge_cnt_++;

    return boost::shared_ptr <edge> (new edge (src, tgt, scheduler_, session_, edge_cnt_));
  }


  boost::shared_ptr <edge> dag::create_edge (void)
  {
    util::scoped_lock (mtx_);

    edge_cnt_++;

    return boost::shared_ptr <edge> (new edge (scheduler_, session_, edge_cnt_));
  }



  void dag::add_node (const node_id_t        & name, 
                      boost::shared_ptr <node> node)
  {
    util::scoped_lock (mtx_);

    if ( ! node )
    {
      std::cout << "adding NULL node " << name << " ?" << std::endl;
      return;
    }

    nodes_[name] = node;

    node->set_name (name);

    // ### scheduler hook
    scheduler_->hook_node_add (node);
  }


  // add edges to named nodes
  //
  // NOTE: INPUT and OUTPUT are accepted as special node names for file stage-in
  // and stage-out.
  //
  void dag::add_edge (boost::shared_ptr <edge> e, 
                      const node_id_t        & src, 
                      const node_id_t        & tgt)
  {
    util::scoped_lock (mtx_);

    boost::shared_ptr <node> n_src;
    boost::shared_ptr <node> n_tgt;

    if ( nodes_.find (src) != nodes_.end () )
    {
      n_src = nodes_[src];
    }
    else
    {
      std::cout << " !!! add_edge: could not find src node with id " << src << std::endl; 
      return;
    }

    if ( nodes_.find (tgt) != nodes_.end () )
    {
      n_tgt = nodes_[tgt];
    }
    else
    {
      std::cout << " !!! add_edge: could not find tgt node with id " << tgt << std::endl; 
      return;
    }

    add_edge (e, n_src, n_tgt);
  }


  void dag::add_edge (boost::shared_ptr <edge> e, 
                      boost::shared_ptr <node> src, 
                      boost::shared_ptr <node> tgt)
  {
    util::scoped_lock (mtx_);

    if ( ! src || ! tgt )
    {
      std::cout << " !!! add_edge: ignoring: either src or tgt node is invalid" << std::endl; 
      return;
    }

    // connect edge to its nodes
    // first set the edge's nodes, so that it has a name
    e->add_src_node (src);
    e->add_tgt_node (tgt);

    // then register the edge to the nodes it connects
    src->add_edge_out (e);
    tgt->add_edge_in  (e);

    // register new edge
    edges_[e->get_id ()] = e;

    // ### scheduler hook
    scheduler_->hook_edge_add (e);
  }


  void dag::reset (void)
  {
    util::scoped_lock (mtx_);

    std::map <node_id_t, boost::shared_ptr <node> > :: iterator it;
    std::map <node_id_t, boost::shared_ptr <node> > :: iterator begin = nodes_.begin ();
    std::map <node_id_t, boost::shared_ptr <node> > :: iterator end   = nodes_.end ();

    state_ = Pending;

    for ( it = begin; it != end; it++ )
    {
      it->second->reset ();
    }
  }


  void dag::dryrun (void)
  {
    util::scoped_lock (mtx_);

    if ( Pending != state_ ) return;

    std::cout << " dryun:  dag" << std::endl;

    std::map <node_id_t, boost::shared_ptr <node> > :: iterator it;
    std::map <node_id_t, boost::shared_ptr <node> > :: iterator begin = nodes_.begin ();
    std::map <node_id_t, boost::shared_ptr <node> > :: iterator end   = nodes_.end ();

    for ( it = begin; it != end; it++ )
    {
      if ( it->second->get_state () == Pending )
      {
        it->second->dryrun ();
      }
    }
  }


  void dag::fire (void)
  {
    util::scoped_lock (mtx_);

    std::cout << "fire   dag  " << std::endl;

    // dump ();

    // dump_node ("INPUT");
    // dump_node ("OUTPUT");

    std::cout << std::string ("state: ") << state_to_string (state_) << std::endl;

    {
      if ( Incomplete != state_ &&
           Pending    != state_ )
        return;

      if ( Running == state_ )
        return;
    }

    // we should get here exactly once

    // ### scheduler hook
    scheduler_->hook_dag_run_pre ();


    // search for nodes which have resolved inputs (no peding edges), and
    // fire them.  Whenever a node finishes, it fires it outgoing edges.
    // If those finish copying their data, they'll fire those nodes which
    // they are incoming edges of.  Of those nodes happen to have all
    // input edges resolved, the fire will indeed lead to an execution of
    // that node, etc.
    //
    // if no nodes can be fired, complain.  Graph may be cyclic.
    std::map <node_id_t, boost::shared_ptr <node> > :: iterator it;
    std::map <node_id_t, boost::shared_ptr <node> > :: iterator begin = nodes_.begin ();
    std::map <node_id_t, boost::shared_ptr <node> > :: iterator end   = nodes_.end ();

    for ( it = begin; it != end; it++ )
    {
      if ( Pending == it->second->get_state () )
      {
        std::cout << std::string (" ===   dag fires node w/o incomplete edges: ") 
                  << it->second->get_id () << std::endl;
        it->second->fire ();
        state_ = Running;
      }
    }


    if ( state_ != Running )
    {
      state_ = Failed;

      // ### scheduler hook
      scheduler_->hook_dag_run_fail ();

      throw "can't find pending nodes.  cyclic or empty graph?";
    }

//  std::cout << "dag fired" << std::endl;

    // ### scheduler hook
    scheduler_->hook_dag_run_post ();

//  std::cout << "dag fire done" << std::endl;
  }


  void dag::wait (void)
  {
    util::scoped_lock (mtx_);

//  std::cout << "dag    wait..." << std::endl;

    // ### scheduler hook
    scheduler_->hook_dag_wait ();

    state s = get_state ();
    while ( s != Done   && 
            s != Failed )
    {
      std::cout << "dag    waiting..." << std::endl;
      util::ms_sleep (1000);
      s = get_state ();
    }

    std::cout << "dag state is final: " << state_to_string (s) << std::endl;
  }


  state dag::get_state (void)
  {
    util::scoped_lock (mtx_);

    // these states are final - we can return immediately
    if ( Failed  == state_ ||
         Done    == state_ )
    {
      std::cout << "get_state on final state" << std::endl;
      return state_;
    }

    int state_incomplete = 0;
    int state_stopped    = 0;
    int state_pending    = 0;
    int state_running    = 0;
    int state_done       = 0;
    int state_failed     = 0;
    int state_total      = 0;


    {
      // count node states
      std::map <node_id_t, boost::shared_ptr <node> > :: const_iterator it;
      std::map <node_id_t, boost::shared_ptr <node> > :: const_iterator begin = nodes_.begin ();
      std::map <node_id_t, boost::shared_ptr <node> > :: const_iterator end   = nodes_.end ();

      int i = 0;
      for ( it = begin; it != end; it++ )
      {
        if ( ! (i++ % 5) )
        {
 //       std::cout << std::endl;
        }

        state_total++;

        state s = it->second->get_state ();
 //     std::cout << it->first << ":" << state_to_string (s) <<  "\t";

        switch ( s )
        {
          case Incomplete:
            state_incomplete++;
            break;
          case Stopped:
            state_stopped++;
            break;
          case Pending:
            state_pending++;
            break;
          case Running:
            state_running++;
            break;
          case Done:
            state_done++;
            break;
          case Failed:
            state_failed++;
            break;
        }
      }
    }
 // std::cout << std::endl;

    {
      // count edge states
      std::map <edge_id_t, edge_map_t> :: const_iterator it;
      std::map <edge_id_t, edge_map_t> :: const_iterator begin = edges_.begin ();
      std::map <edge_id_t, edge_map_t> :: const_iterator end   = edges_.end ();

      int cnt = 0;
      for ( it = begin; it != end; it++ )
      {
        if ( ! (cnt++ % 2) )
        {
          // std::cout << std::endl;
        }

        state_total++;

        state s = it->second->get_state ();
        // std::cout << it->second[i]->get_name () 
        //           << ":" << state_to_string (s) <<  "\t", false << std::endl;

        switch ( s )
        {
          case Incomplete:
            state_incomplete++;
            break;
          case Stopped:
            state_stopped++;
            break;
          case Pending:
            state_pending++;
            break;
          case Running:
            state_running++;
            break;
          case Done:
            state_done++;
            break;
          case Failed:
            state_failed++;
            break;
        }
      }
    }
    // std::cout << std::endl;


    // if any job failed, dag is considered to have failed
    if ( state_failed > 0 )
    {
      state_ = Failed;
    }

    // one job being stopped, incomplete, pending or Running defines the dag's state
    else if ( state_stopped > 0 )
    {
      state_ = Stopped;
    }
    else if ( state_incomplete > 0 )
    {
      state_ = Incomplete;
    }
    else if ( state_pending > 0 )
    {
      state_ = Pending;
    }
    else if ( state_running > 0 )
    {
      state_ = Running;
    }

    // if all states are Done, dag is ready
    else if ( state_done == state_total )
    {
      state_ = Done;

      // ### scheduler hook
      scheduler_->hook_dag_run_done ();
    }
    else
    {
      // cannot happen (tm)
      throw "inconsistent dag state";
    }
    
    return state_;
  }

  void dag::set_state (state s)
  {
    util::scoped_lock (mtx_);

    state_ = s;
  }

  void dag::dump (void)
  {
    util::scoped_lock (mtx_);

    std::cout << " -  DAG    ----------------------------------\n" << std::endl;
    std::cout << std::string (" state: ") << state_to_string (get_state ()) << std::endl;

    std::cout << " -  NODES  ----------------------------------\n" << std::endl;
    {
      std::map <node_id_t, boost::shared_ptr <node> > :: const_iterator it;
      std::map <node_id_t, boost::shared_ptr <node> > :: const_iterator begin = nodes_.begin ();
      std::map <node_id_t, boost::shared_ptr <node> > :: const_iterator end   = nodes_.end ();

      for ( it = begin; it != end; it++ )
      {
        it->second->dump ();
      }
    }
    std::cout << " -  NODES  ----------------------------------\n" << std::endl;

    std::cout << " -  EDGES  ----------------------------------\n" << std::endl;
    {
      std::map <edge_id_t, edge_map_t> :: const_iterator it;
      std::map <edge_id_t, edge_map_t> :: const_iterator begin = edges_.begin ();
      std::map <edge_id_t, edge_map_t> :: const_iterator end   = edges_.end ();

      for ( it = begin; it != end; it++ )
      {
        it->second->dump ();
      }
    }
    std::cout << " -  DAG    ----------------------------------\n" << std::endl;
  }


  void dag::dump_node (std::string name)
  {
    util::scoped_lock (mtx_);

    if ( nodes_.find (name) != nodes_.end () )
    {
      nodes_[name]->dump ();
    }
  }


  void dag::schedule (void)
  {
    util::scoped_lock (mtx_);

    // ### scheduler hook
    scheduler_->hook_dag_schedule ();

    // FIXME: data transfers may end up to be redundant, and should be
    // pruned.
  }

} // namespace digedag

