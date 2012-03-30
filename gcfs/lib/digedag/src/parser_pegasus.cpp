
#include <string>
#include <fstream>
#include <iostream>

#include "util/split.hpp"
#include "parser_pegasus.hpp"

namespace digedag
{
  namespace pegasus
  {
    parser::parser (const std::string & dag_file,
                    const std::string & scheduler_file)
      : dag_file_       (dag_file)
      , scheduler_file_ (scheduler_file)
      , basename_       (get_name (dag_file_))
      , basedir_        (get_dir  (dag_file_))
    {
      parse_dag ();
    }

    parser::~parser (void)
    {
    }

    void parser::parse_dag (void)
    {
      dag_ = boost::shared_ptr <dag> (new dag (scheduler_file_));

      // first, we read the base dag file, and axtract the following infos:
      //  - list of sub elements w/o cleanup and staging elements
      //  - list of cleanup elements
      //  - list of staging elements
      std::fstream fin;
      std::string  line;

      fin.open (dag_file_.c_str (), std::ios::in);

      if ( fin.fail () )
      {
        std::cerr << "opening " << dag_file_ << " failed\n";
        throw "Cannot open file";
      }

      try 
      {
        while ( std::getline (fin, line) )
        {
          std::vector <std::string> words = split (line);

          if ( words[0] == "JOB" )
          {
            std::cout << "JOB LINE: " << line << std::endl;

            parse_node (line);
          }

          else if ( words[0] == "PARENT" && 
                    words[2] == "CHILD"  )
          {
            std::cout << "edge LINE: " << line << std::endl;

            parse_edge (line);
          }
        }
      }
      catch ( ... )
      {
        std::cerr << "reading " << dag_file_ << " failed\n";
        throw "file read error";
      }

      // dag_->dump ();
    }


    void parser::parse_node (const std::string spec)
    {
      std::cout << "\n";

      std::vector <std::string> elems = split (spec);

      std::fstream fin;
      std::string  line;
      std::string  name  = elems[1];
      std::string  fname = basedir_ + elems[2];

      fin.open (fname.c_str (), std::ios::in);

      if ( fin.fail () )
      {
        std::cerr << "opening " << fname << " failed\n";
        throw "Cannot open file";
      }

      try 
      {
        node_description nd;

        while ( std::getline (fin, line) )
        {
          std::string key = "";
          std::vector <std::string> words = split (line);

          if ( words.size () > 2 )
          {
            key = words[0];

            // remove keyword and '='
            words.erase (words.begin () + 0);
            words.erase (words.begin () + 0); // index shifted!
          }

          if ( key == "executable" )
          {
            std::cout << "EXE LINE " << std::endl;

            std::cout << words[0] << "\n";

            nd.set_attribute ("Executable", words[0]);
          }

          else if ( key == "arguments" )
          {
            std::cout << "ARG LINE " << std::endl;

            // remove '"' from first and last element
            words[0]                .erase (words[0]                .begin () + 0);
            words[words.size () - 1].erase (words[words.size () - 1].end ()   - 1);

            std::cout << "   ";
            for ( unsigned int i = 0; i < words.size (); i++ )
              std::cout << words[i] << " ";
            std::cout << "\n";

            nd.set_vector_attribute ("Arguments", words);
          }

          else if ( key == "environment" )
          {
            std::cout << "ENV LINE " << std::endl;

            std::vector <std::string> env = split (words[0], ";");

            for ( unsigned int i = 0; i < env.size (); i++ )
              std::cout << "   " << env[i] << "\n";

            nd.set_vector_attribute ("Environment", env);
          }

          else if ( key == "input" )
          {
            std::cout << "IN  LINE " << std::endl;

            std::cout << words[0] << "\n";

            nd.set_attribute ("Input", words[0]);
          }

          else if ( key == "output" )
          {
            std::cout << "OUT LINE " << std::endl;

            std::cout << words[0] << "\n";

            nd.set_attribute ("Output", words[0]);
          }

          else if ( key == "error" )
          {
            std::cout << "ERR LINE " << std::endl;

            std::cout << words[0] << "\n";

            nd.set_attribute ("Error", words[0]);
          }

        }

        boost::shared_ptr <node> n = dag_->create_node (nd);
        dag_->add_node (name, n);
      }
      catch ( ... )
      {
        std::cerr << "reading " << fname << " failed\n";
        throw "file read error";
      }
    }


    void parser::parse_edge (const std::string spec)
    {
      std::vector <std::string> elems = split (spec);

      boost::shared_ptr <edge> e = dag_->create_edge ("file://localhost/TODO", 
                                                      "file://localhost/FIXME");
      dag_->add_edge (e, elems[1], elems[3]);
    }


    std::string parser::get_name (const std::string filename)
    {
      size_t pos = filename.rfind ('/');

      if ( std::string::npos != pos )
      {
        return filename.substr (pos);
      }

      // no '/' - return complete name
      return filename;
    }


    std::string parser::get_dir (const std::string filename)
    {
      size_t pos = filename.rfind ('/');

      if ( std::string::npos != pos )
      {
        return filename.substr (0, pos + 1);
      }

      // no '/' - return complete cwd
      return "./";;
    }

  } // namespace pegasus

} // namespace digedag

