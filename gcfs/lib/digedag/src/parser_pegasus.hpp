
#ifndef DIGEDAG_PEGASUS_PARSER_HPP
#define DIGEDAG_PEGASUS_PARSER_HPP

#include <string>
#include <vector>

#include "util/split.hpp"
#include "digedag.hpp"


namespace digedag
{
  namespace pegasus
  {
    class parser
    {
      private:
        boost::shared_ptr <dag>   dag_;

        std::string               dag_file_;
        std::string               scheduler_file_;

        std::string               basename_;
        std::string               basedir_;
        std::vector <std::string> files_;

        void        parse_dag  (void);
        void        parse_node (const std::string spec);
        void        parse_edge (const std::string spec);

        std::string get_name   (const std::string filename);
        std::string get_dir    (const std::string filename);


      public:
        parser (const std::string & dag_file, 
                const std::string & scheduler_file);
        ~parser (void);

        boost::shared_ptr <dag> get_dag (void) 
        { 
          return dag_; 
        }
    };
  }
} // namespace digedag

#endif // DIGEDAG_PEGASUS_PARSER_HPP

