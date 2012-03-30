
#ifndef DIGEDAG_UTIL_SHAREDPTR_BOOST_HPP
#define DIGEDAG_UTIL_SHAREDPTR_BOOST_HPP

#include <iostream>

#ifndef  USE_BOOST
# error "Oops - USE_BOOST undefined for boost version of shared_ptr"
#endif

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace digedag
{
  namespace util
  {
    template <class T> class enable_shared_from_this;

    // simply use boost's shared ptr implementation
    template <class T> class shared_ptr : public boost::shared_ptr <T>
    {
      private:
        std::string id_;

      public:
        std::string get_id (void) const
        {
          return id_;
        }

        shared_ptr (std::string id = "???")
          : boost::shared_ptr <T> ()
          , id_ (id)
          { 
            std::cout << " ### creating (1) shared_ptr for " << id_ << std::endl;
          }

        shared_ptr (T * t, std::string id = "???") 
          : boost::shared_ptr <T> (t) 
          , id_ (id)
          {
            std::cout << " ### creating (2) shared_ptr for " << id_ << std::endl;
          }

        shared_ptr (boost::shared_ptr <T> & p) 
          : boost::shared_ptr <T> (p) 
          , id_ (p.get_id ())
        {
          std::cout << " ### creating (3) shared_ptr for " << id_ << std::endl;
        }

        shared_ptr (const boost::shared_ptr <T> & p)
          : boost::shared_ptr <T> (p) 
          , id_ (p.get_id ())
        {
          std::cout << " ### creating (4) shared_ptr for " << id_ << std::endl;
        }

        ~shared_ptr (void)
        {
          std::cout << " ### deleting shared_ptr for " << id_ << std::endl;
        }


      public:
        template <class F> friend bool  operator! (const boost::shared_ptr <T> & p);
        template <class F> friend class util::enable_shared_from_this;
    };

    template <class T>
    bool operator! (const boost::shared_ptr <T> & p)
    {
      return ( p.get () );
    }
    

    template <class T>
    class enable_shared_from_this : public boost::enable_shared_from_this <T>
    {
      public:
        enable_shared_from_this (void) :
            boost::enable_shared_from_this <T> () 
        { }

        boost::shared_ptr <T> shared_from_this ()
        {
          return boost::shared_ptr <T>
            (boost::enable_shared_from_this <T>::shared_from_this ());
        }
    };

  } // namespace util

} // namespace digedag

#endif // DIGEDAG_UTIL_SHAREDPTR_BOOST_HPP

