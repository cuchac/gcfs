
#include <vector>
#include <fstream>

#include <saga/saga.hpp>

#include "util/split.hpp"
#include "util/scoped_lock.hpp"

#include "dag.hpp"
#include "enum.hpp"
#include "node.hpp"
#include "edge.hpp"
#include "enactor.hpp"
#include "scheduler.hpp"
#include "util/util.hpp"

#define MAX_NODES 20
#define MAX_EDGES 20

namespace digedag
{
  scheduler::scheduler (dag               * d, 
                        const std::string & policy, 
                        saga::session       session)
    : session_       (  session)
    , dag_           (        d)
    , policy_        (   policy)
    , initialized_   (    false)
    , stopped_       (    false)
    , max_nodes_     (MAX_NODES)
    , max_edges_     (MAX_EDGES)
    , active_nodes_  (        0)
    , active_edges_  (        0)
  {
    parse_src ();
  }

  scheduler::~scheduler (void)
  {
    // Signal that the end is near.
    stop ();

    std::cout << " === scheduler destructed" << std::endl;

    util::scoped_lock sl (mtx_);

    thread_exit ();
  }

  void scheduler::stop (void)
  {
    util::scoped_lock sl (mtx_);
    
    stopped_ = true;
  }

  void scheduler::parse_src (void)
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return;
    }

    if ( policy_.empty () )
    {
      return;
    }

    std::fstream fin;
    std::string  line;

    fin.open (policy_.c_str (), std::ios::in);

    if ( fin.fail () )
    {
      std::cerr << "opening " << policy_ << " failed" << std::endl;
      throw "Cannot open file";
    }

    unsigned int lnum = 1;

    while ( std::getline (fin, line) )
    {
      std::vector <std::string> words = split (line);

      if ( words.size () < 1 )
      {
        std::cerr << "parser error (1) in " << policy_ << " at line " << lnum << std::endl;
      }
      else if ( words[0] == "#" )
      {
        // ignore comments
      }
      else if ( words[0] == "context" )
      {
        try 
        {
          // split context line words into key=val pairs, and set those as
          // attributes.  Add that respective context to the dag's session on
          // hook_dag_create
          saga::context c;

          // leave out context keyword
          for ( unsigned int i = 1; i < words.size (); i++ )
          {
            // all other words, split at '='
            std::vector <std::string> elems = split (words[i], "=", 2);

            if ( 0 == elems.size () )
            {
              // nothing to do
              // this should never happen though unless words[i] was empty.
            }
            else if ( 1 == elems.size () )
            {
              // empty value
              c.set_attribute (elems[0], "");
            }
            else // never have more than two elems, as per split limit above
            {
              // set key and value
              c.set_attribute (elems[0], elems[1]);
            }
          }

          session_.add_context (c);
        }
        catch ( const saga::exception & e )
        {
          // error in handling context line
          std::cerr << "context error in " << policy_ 
                    << " at line " << lnum 
                    << ": \n" << e.what () 
                    << std::endl;
        }
      }
      else if ( words[0] == "data" )
      {
        if ( words.size () != 4 )
        {
          std::cerr << "parser error (2) in " << policy_ << " at line " << lnum << std::endl;
        }
        else if ( words[1] == "INPUT" )
        {
          data_src_host_ = words[2];
          data_src_pwd_  = words[3];
        }
        else if ( words[1] == "OUTPUT" )
        {
          data_tgt_host_ = words[2];
          data_tgt_pwd_  = words[3];
        }
        else
        {
          std::cerr << "parser error (3) in " << policy_ << " at line " << lnum << std::endl;
        }
      }
      else if ( words[0] == "job" )
      {
        if ( words.size () < 5 )
        {
          std::cerr << "parser error (4) in " << policy_ << " at line " << lnum << std::endl;
        }
        else
        {
          job_info_[words[1]].rm   = words[2];
          job_info_[words[1]].host = words[3];
          job_info_[words[1]].pwd  = words[4];

          if ( words.size () >= 6 )
          {
            job_info_[words[1]].path = words[5];
          }
        }
      }
      

      lnum++;
    }
  }

  /////////////////////////////////////////////////////////////////
  //
  // dag hooks
  //
  bool scheduler::hook_dag_create (void)
  {
    util::scoped_lock sl (mtx_);

    // shared_from_this() is only available from here on
    enact_nodes_.reset (new digedag::enactor (shared_from_this (), "node"));
    enact_edges_.reset (new digedag::enactor (shared_from_this (), "edge"));

    initialized_ = true;

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }


  bool scheduler::hook_dag_destroy (void)
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }


  bool scheduler::hook_dag_schedule (void)
  {
    // FIXME: check
    // util::scoped_lock sl (mtx_);
    util::scoped_lock sl ();


    if ( stopped_ ) 
    {
      return false;
    }

    // walk throgh the dag, and assign execution host for nodes, and data
    // prefixes for edges
    std::map <node_id_t, node_map_t> nodes = dag_->get_nodes ();
    std::map <edge_id_t, edge_map_t> edges = dag_->get_edges ();

    // first, fix pwd and host for INPUT and OUTPUT nodes
    boost::shared_ptr <node> input  = nodes["INPUT"];
    boost::shared_ptr <node> output = nodes["OUTPUT"];

    input->set_pwd   (data_src_pwd_);
    input->set_host  (data_src_host_);
    output->set_pwd  (data_tgt_pwd_);
    output->set_host (data_tgt_host_);


    // now fix all other nodes, too
    {
      std::map <node_id_t, node_map_t> :: const_iterator it;
      std::map <node_id_t, node_map_t> :: const_iterator begin = nodes.begin ();
      std::map <node_id_t, node_map_t> :: const_iterator end   = nodes.end ();

      for ( it = begin; it != end; it++ )
      {
        std::string id = it->first;
        boost::shared_ptr <node> n  = it->second;

        if ( job_info_.find (id) != job_info_.end () )
        {
          n->set_rm   (job_info_[id].rm);
          n->set_host (job_info_[id].host);
          n->set_pwd  (job_info_[id].pwd);

          if ( ! job_info_[id].path.empty () )
          {
            n->set_path (job_info_[id].path);
          }
        }
        else
        {
          if ( id != "INPUT"  &&
               id != "OUTPUT" )
          {
            std::cerr << " warning: cannot find job scheduler info for job " << id << std::endl;
          }
        }
      }
    }

    return true;
  }


  bool scheduler::hook_dag_run_pre (void)
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    // start the scheduler thread which executes nodes and edges
    std::cout << "starting scheduler thread" << std::endl;
    thread_run ();

    return true;
  }


  bool scheduler::hook_dag_run_post (void)
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }


  bool scheduler::hook_dag_run_done (void)
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }


  bool scheduler::hook_dag_run_fail (void)
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    std::cerr << " === dag failed!  exit." << std::endl;
    ::exit (3);
  }


  bool scheduler::hook_dag_wait (void)
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }



  /////////////////////////////////////////////////////////////////
  //
  // node hooks
  //
  bool scheduler::hook_node_add (boost::shared_ptr <node> n)           
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }


  bool scheduler::hook_node_remove (boost::shared_ptr <node> n)           
  {
    // FIXME: remove node from queue if needed
 
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }


  bool scheduler::hook_node_run_pre (boost::shared_ptr <node> n)           
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    assert ( n );

    // only accept that node if it wasn't seen before
    if ( known_nodes_.end () != known_nodes_.find (n->get_name ()) )
    {
      std::cout << " === scheduler ignores node " 
                << n->get_name () << " (known)" << std::endl;
      return false;
    }


    std::cout << " === scheduler registers node " << n->get_name () << std::endl;

    // remember this node
    known_nodes_.insert (n->get_name ());


    // queue the node for work
    queue_nodes_.push_back (n);

    return true;
  }


  bool scheduler::hook_node_run_done (boost::shared_ptr <node> n)           
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    // std::cout << " === node done: " << n->get_name () << std::endl;
    
    return true;
  }


  bool scheduler::hook_node_run_fail (boost::shared_ptr <node> n)           
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    std::cout << " === node failed: " << n->get_name () << " - exit!" << std::endl;
    ::exit (1);
  }



  /////////////////////////////////////////////////////////////////
  //
  // edge hooks
  //
  bool scheduler::hook_edge_add (boost::shared_ptr <edge> e)           
  {
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }


  bool scheduler::hook_edge_remove (boost::shared_ptr <edge> e)           
  {
    // FIXME: remove edge from queue if needed
    
    util::scoped_lock sl (mtx_);

    if ( stopped_ ) 
    {
      return false;
    }

    return true;
  }


  bool scheduler::hook_edge_run_pre (boost::shared_ptr <edge> e)           
  {
    util::scoped_lock sl (mtx_);

    assert ( e );

    if ( stopped_ ) 
    {
      return false;
    }

    // add edge to queue
    queue_edges_.push_back (e);

    std::cout << " === adding   edge to   queue: " << e->get_name () << std::endl;

    return true;
  }


  bool scheduler::hook_edge_run_done (boost::shared_ptr <edge> e)           
  {
    if ( stopped_ ) 
    {
      return false;
    }

    lock ();

    active_files_.erase (e->get_src ().get_string ());
    active_files_.erase (e->get_tgt ().get_string ());

    unlock ();

    // std::cout << " === egde done: " << e->get_name () << std::endl;

    return true;
  }


  bool scheduler::hook_edge_run_fail (boost::shared_ptr <edge> e)           
  {
    if ( stopped_ ) 
    {
      return false;
    }

    lock ();

    active_files_.erase (e->get_src ().get_string ());
    active_files_.erase (e->get_tgt ().get_string ());

    unlock ();

    // FIXME: should be handled in the DAG
    std::cout << " === edge failed: " << e->get_name () << " - exit" << std::endl;
    ::exit (2);
  }


  saga::session scheduler::hook_saga_get_session (void)
  {
    util::scoped_lock sl (mtx_);

    return session_;
  }


  void scheduler::thread_work (void)
  {
    std::cout << "started scheduler thread" << std::endl;

    // wait for proper initialization of the enactors
    while ( ! initialized_ )
    {
      util::ms_sleep (100);
    }


    // work the node and edge queues:
    // 
    // if 
    //  - work is in the queues  
    //    AND
    //  - not more than max_nodes/max_edges are running
    // then
    //  - start new nodes/edges, removing them from the queue
    while ( true )
    {
      // std::cout << " === scheduler queue watch begins " << std::endl;

      while ( max_nodes_           > active_nodes_ &&
              queue_nodes_.size () > 0             )
      {
        util::scoped_lock sl (mtx_);

        // std::cout << " === scheduler is checking node queue " << std::endl;

        // get node from queue
        boost::shared_ptr <node> n = queue_nodes_.front ();
        queue_nodes_.pop_front ();

        // n->dump ();

        // std::cout << " === scheduler starts node " << n->get_name () << std::endl;
        saga::task t = n->work_start ();

        // FIXME: we need to verify here if the task is valid (correct state
        // etc).  Or simply catch for SAGA exceptions?

        active_nodes_++;

        enact_nodes_->queue_task (t);
        // std::cout << " === mapping task " 
        //           << t.get_id () 
        //           << " to node " 
        //           << n->get_name () 
        //           << std::endl;

        node_task_map_[t] = n;
      }


      while ( max_edges_           > active_edges_ &&
              queue_edges_.size () > 0             )
      {
        util::scoped_lock sl (mtx_);

        // std::cout << " === scheduler is checking edge queue " << std::endl;

        // FIXME:  
        // At the moment, the local file adaptor is not supporting atomic
        // operations, not even close.  So, we need to take care not to run
        // edges concurrently which operate on the same data files (src/tgt).
        // Thus we have a peek at the next edge, and if it does not fit the
        // bill, it is ignored for this while iteration.
        //
        // Note that active_files is emptied by the scheduler
        // edge_run_done and edge_run_failed hooks.

        // get edge from queue
        boost::shared_ptr <edge> e = queue_edges_.front ();
        queue_edges_.pop_front ();

        // e->dump ();

        if ( active_files_.find (e->get_src ().get_string ()) != active_files_.end () ||
             active_files_.find (e->get_tgt ().get_string ()) != active_files_.end () )
        {
          // stop handling that edge for now, and put it to the back of the queue
          queue_edges_.push_back (e);
          continue;
        }

        // this edge is being executed - so lock src and tgt as long as it is
        // active

        // only insert clones
        active_files_.insert (e->get_src ().get_string ());
        active_files_.insert (e->get_tgt ().get_string ());

        active_edges_++;

        // std::cout << " === scheduler starts edge " << e->get_name () << std::endl;
        saga::task t = e->work_start ();
        
        enact_edges_->queue_task (t);
        
        edge_task_map_[t] = e;

        // std::cout << " === mapping task " 
        //           << t.get_id () 
        //           << " to edge " 
        //           << e->get_name () 
        //           << std::endl;

        // dump_map (edge_task_map_);
      }

      std::cout << " === scheduler queue watch done " << std::endl;

      util::ms_sleep (100);
    }

    std::cout << " === scheduler queue watch finished " << std::endl;
  }

  void scheduler::work_finished (saga::task  t, 
                                 std::string flag)
  {
    {
      util::scoped_lock sl (mtx_);

      // std::cout << " === work finished for task " << t.get_id () << std::endl;

      if ( stopped_ ) 
      {
        return;
      }
    }

    // this flag should be an enum, so that we can do a proper switch
    if ( flag == "node" )
    {
      boost::shared_ptr <node> n;
      {
        util::scoped_lock sl (mtx_);

        active_nodes_--;
        assert (active_nodes_ >= 0);

        n = node_task_map_[t];

        node_task_map_.erase (t);
      }
      // scoped lock dies here, so that the work_done call below can call back
      // into the scheduler.


      if ( ! n )
      {
        // ouch
        std::cerr << "got an invalid node from the task map!" << std::endl;
      }


      if ( t.get_state () == saga::task::Done )
      {
        n->work_done ();
      }
      else 
      {
        n->work_failed ();
      }
    }
    else if ( flag == "edge" )
    {
      boost::shared_ptr <edge> e;

      {
        util::scoped_lock sl (mtx_);
        
        active_edges_--;
        assert (active_edges_ >= 0);

        e = edge_task_map_[t];

        node_task_map_.erase (t);
      }
      // scoped lock dies here, so that the work_done call below can call back
      // into the scheduler.


      if ( ! e )
      {
        // ouch
        std::cerr << "got an invalid edge from the task map!" << std::endl;
      }


      if ( t.get_state () == saga::task::Done )
      {
        e->work_done ();
      }
      else 
      {
        e->work_failed ();
      }
    }
    else
    {
      std::cerr << "unknown task finished.  flag: " << flag << std::endl;
      exit (-1);
    }
  }


  void scheduler::dump_map (const std::map <saga::task, boost::shared_ptr <edge> >  & map)
  {
    util::scoped_lock sl (mtx_);
    // return;

    std::map <saga::task, boost::shared_ptr <edge> > :: const_iterator begin = map.begin ();
    std::map <saga::task, boost::shared_ptr <edge> > :: const_iterator end   = map.end   ();
    std::map <saga::task, boost::shared_ptr <edge> > :: const_iterator it;

    for ( it = begin; it != end; it++ )
    {
      std::cout << "     " << it->first.get_id () << " - " << it->second->get_name () << std::endl;
    }
  }
} // namespace digedag

