
#ifndef DIGEDAG_UTIL_SHAREDPTR_NOBOOST_HPP
#define DIGEDAG_UTIL_SHAREDPTR_NOBOOST_HPP

#ifdef  USE_BOOST
# error "Oops - USE_BOOST defined for non-boost version of shared_ptr"
#endif

#include "util/scoped_lock.hpp"
#include "util/shared_from_this_noboost.hpp"
#include <string>
#include <sstream>

namespace digedag
{
  namespace util
  {
    // the shared_ptr class is a pointer container which
    // performes reference counting for the pointer it holds.
    // The pointer is deleted whn the reference count reaches
    // zero, i.e. when the last copy of the shared pointer is
    // deleted.
    //
    // NOTES:
    //   - the  application must ensure that the pointer is 
    //     not in use outside that reference counting scheme, 
    //     at time of deletion.
    //
    // LIMITATIONS:
    //   - refcounter is of long type, that obviously limits 
    //     the number of allowed copies
    //
    //   - to obtain correct deletion semantics on casting of
    //     shared pointers, the classes involved SHOULD have
    //     virtual destructors.
    //
    template <class T> class shared_ptr 
    {
      public:
        // allow for dynamic casts for other shared ptr types
        template <class U> friend class shared_ptr;

      private:
        T           * ptr_;      // holds the pointer
        long        * cnt_;      // reference count
        std::string   id_;       // id for debugging

        // flag if enable_shared_from_this is anywhere available in the class
        // hierarchy
        bool sft_;


      private:
        // Increment the reference count.  
        // The counter is locked with a scoped lock.
        void inc_ (void)
        {
          scoped_lock lck;

          (*cnt_) ++;

          log ("inc    ", cnt_);
        }


        // Decrement the reference count.   If ref count reaches zero, delete
        // the pointer.
        // The counter is locked with a scoped lock.
        void dec_ (void)
        {
          scoped_lock lck;

          (*cnt_) --;

          log ("dec    ", cnt_);

          // TODO: SHOULD be ==
          if ( *cnt_ <= 0 )
          {
            log ("FREEING", cnt_);
            if ( ptr_ ) 
            {
              delete (ptr_);
              ptr_ = NULL;
            }
          }
        }


      private:
        // provide an empty virtual fallback implementation for
        // enable_shared_from_this, in case that is not available in the class
        // hierarchy.
        virtual void init_shared_from_this (shared_ptr <T> const & t)
        {
          log ("NOT init sft");
          sft_ = false;
        }

        void log (const std::string & msg, long * c) const
        {
          std::stringstream ss;
          ss << msg << " (" << (*c) << ")";
          log (ss.str ());
        }

        void log (const std::string & msg) const
        {
          std::string fill;
          if ( ptr_ == NULL ) fill = "        ";

          if ( id_.data () && ! id_.empty () )
          {
            std::cout << " ### " << msg 
              << "\t shared ptr for [" << ptr_ << fill << "] : \t " 
              << " (" << this << ") \t"
              << id_    
              << std::endl;
          }
          else
          {
            break_here ();
            std::cout << " ### " << msg 
              << "\t shared ptr for [" << ptr_ << fill << "] : \t " 
              << " (" << this << ") \t"
              << "WTF?" 
              << std::endl;
          }
        }

        void break_here (void) const
        {
          ::sleep (1);
          return;
        }
        
      public:
        // // real void default ctor
        // shared_ptr (void)
        //   : ptr_   (NULL)
        //   , id_    ("default")
        //   , sft_   (false)
        // {
        //   cnt_  = new long;
        //   *cnt_ = 0;
        //   log ("set a  ", cnt_);
        // }

        // void default ctor, takes only id
        shared_ptr (std::string id)
          : ptr_ (NULL)
          , id_  (id)
          , sft_ (false)
        {
          cnt_  = new long;
          *cnt_ = 0;
          log ("set a  ", cnt_);
        }

        // default ctor
        // marked explicit, so we never automatically convert to a shared pointer
        explicit shared_ptr ( T * p = 0, 
                              std::string id = "noname")
          : ptr_  (p)
          , id_   (id)
          , sft_  (false)
        {
          try 
          {
            // get new counter, and increment
            cnt_  = new long;
            *cnt_ = 0;
            log ("set b  ", cnt_);

            if ( sft_ )
            {
              log ("inisft 1", cnt_);
              ptr_->init_shared_from_this (*this);
            }

            inc_ ();
          }
          catch ( ... )
          {
            // can't do much if we don't even get a counter allocated
            // delete (p);
            throw;
          }
        }

        // copy ctor, increments ref count
        shared_ptr (const shared_ptr <T> & p)
          : ptr_ (p.ptr_)
          , cnt_ (p.cnt_)
          , id_  (p.get_id ())
          , sft_ (p.sft_)
        {
          log ("copy a ", cnt_);
          inc_ ();

          if ( sft_ )
          {
            log ("inisft 2", cnt_);
            ptr_->init_shared_from_this (*this);
          }
        }

        // casting copy ctor, increments ref count
        template <class U> 
        shared_ptr (const shared_ptr <U> & p)
          : ptr_ (dynamic_cast <T*> (p.ptr_))
          , cnt_ (p.cnt_)
          , id_  (p.get_id ())
          , sft_ (false)
        {
          log ("copy b ", cnt_);
          inc_ ();
        }


        // dtor, decrements ref count
        virtual ~shared_ptr () 
        {
          log ("dtor   ", cnt_);
          dec_ ();
        }


        // assignment
        shared_ptr <T> & operator= (const shared_ptr <T> & p) 
        {
          log ("assign ", cnt_);

          // got really new pointer
          if ( this != & p ) 
          {
            // decrease refcount for the old ptr, and delete if needed
            if ( ptr_ )
            {
              dec_ ();
            }
          }


          // init 'this' just as in the copy c'tor
          ptr_ = p.ptr_;
          cnt_ = p.cnt_;
          log ("copy c ", cnt_);

          // increment ref count for new pointer
          inc_ ();

          return *this;
        }


        // accessors
        T & operator * ()  const 
        {
          log (" *             ");
          return *ptr_;
        }
        
        T * operator->() const
        {
          log (" ->            ");
          return ptr_;
        }

        T * get ()  const 
        {
          return ptr_;
        }

        // allow to compare shared pointers, by comparing the contained pointer
        bool compare (const shared_ptr <T> & p) const
        {
          return (ptr_ == p.ptr_);
        }

        template <class U> U * dynamic_ptr_cast (void)
        {
          return dynamic_cast <U *> (ptr_);
        }

        template <class U> shared_ptr <U> dynamic_shared_ptr_cast (void)
        {
          U * tmp = dynamic_cast <U *> (ptr_);
          return shared_ptr <U> (tmp);
        }


        std::string get_id (void) const
        {
          return id_;
        }

        operator bool() const
        {
          return get ();
        }
        
        bool operator && (const shared_ptr & p) const
        {
          return this->get () && p.get ();
        }
    };


  } // namespace util

} // namespace digedag

#endif // DIGEDAG_UTIL_SHAREDPTR_NOBOOST_HPP

