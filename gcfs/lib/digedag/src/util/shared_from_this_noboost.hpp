
#ifndef DIGEDAG_UTIL_SHAREDFROMTHIS_NOBOOST_HPP
#define DIGEDAG_UTIL_SHAREDFROMTHIS_NOBOOST_HPP

#ifdef  USE_BOOST
# error "Oops - USE_BOOST defined for non-boost version of shared_ptr"
#endif

#include <string>
#include <sstream>

namespace digedag
{
  namespace util
  {
    template <class T> class shared_ptr;

    // enable_shared_from_this allows to obtain a shared pointer from the class'
    // this pointer
    template <class T>
    class enable_shared_from_this
    {
      private:
        shared_ptr <T> p_;

        // non-copyable
        enable_shared_from_this (enable_shared_from_this const & e)
        { 
        }

        enable_shared_from_this & operator= (enable_shared_from_this const & t)
        {
          return *this;
        }

        // set the shared pointer for 'this'.  This init function is called by
        // shared_ptr c'tor, and that c'tor MUST ensure that the given
        // shared_ptr instance is indeed representing 'this' of the class
        // instance the shared ptr is pointing too.  Otherwise, the excercise we
        // perform here is rather pointless.  OTOH, the shared ptr c'tor should
        // have nothing else to use, so this is rather trivial, too...
        //
        // note that we need to befriend the shared_ptr to allow that init call
        void init_shared_from_this (shared_ptr <T> & p)
        {
          std::cout << " ### enabling shared from this      for [" 
            << p.get () << "] : \t " 
            << " (" << &p << ") \t"
            << p.get_id () 
            << std::endl;

          p_ = p;

          std::cout << " ### enabled  shared from this      for [" 
            << p.get () << "] : \t " 
            << " (" << &p << ") \t"
            << p.get_id () 
            << std::endl;
        }
        template <class U> friend class shared_ptr;


      public:
        // so the inheriting class does not need to do anything
        enable_shared_from_this (std::string id = "esft init")
          : p_ (id)
        {
        }

        ~enable_shared_from_this (void)
        {
        }

        void break_here (void)
        {
          ::sleep (1);
        }

        // to obtain the shared pointer for 'this'
        shared_ptr <T> get_shared ()
        { 
          std::cout << " ### getting sft                    for [" << p_.get () << "] : \t " << p_.get_id () << std::endl;

          if ( p_.get_id () == "esft initial" )
          {
            std::cout << " ### oops! ###################################" << std::endl;
            break_here ();
          }

          return p_;
        }

        // for boost API compatibility
        shared_ptr <T> shared_from_this ()
        { 
          return get_shared ();
        }
    };

  } // namespace util

} // namespace digedag

#endif // DIGEDAG_UTIL_SHAREDFROMTHIS_NOBOOST_HPP

