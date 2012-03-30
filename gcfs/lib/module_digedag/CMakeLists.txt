#DIGEDAG support
option(GCFS_MODULE_DIGEDAG "Add support for DAG submission using SAGA backend")

if (GCFS_MODULE_DIGEDAG)

   add_library(gcfs_digedag gcfs_filesystemdigedag.cpp gcfs_servicesagataskdata.cpp gcfs_servicedigedag.cpp)
   
   set(GCFS_DIGEDAG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../digedag/src")

   FIND_PACKAGE(Boost)
   INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

   FIND_PACKAGE(SAGA REQUIRED)
   target_link_libraries(gcfs_digedag ${SAGA_LIBRARIES})
   include_directories(${SAGA_INCLUDE_DIR})

   include_directories(${GCFS_DIGEDAG_DIR})
   target_link_libraries(gcfs_digedag "digedag")
   link_directories(${GCFS_DIGEDAG_DIR})

   add_custom_command(TARGET gcfs_digedag PRE_BUILD COMMAND "make" "SAGA_LOCATION=/usr" "all" WORKING_DIRECTORY ${GCFS_DIGEDAG_DIR})
   
   get_target_property(MODULES gcfs MODULES)
   LIST(APPEND MODULES "gcfs_digedag")
   set_target_properties(gcfs PROPERTIES MODULES "${MODULES}")
   
   message(STATUS "   Support for Digedag: YES")
   
else(GCFS_MODULE_DIGEDAG)
      
   message(STATUS "   Support for Digedag: NO")

endif(GCFS_MODULE_DIGEDAG)
