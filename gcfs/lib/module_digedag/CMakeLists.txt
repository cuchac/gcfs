#DIGEDAG support
option(GCFS_MODULE_DIGEDAG "Add support for DAG submission using SAGA backend")

if (GCFS_MODULE_DIGEDAG)
   
   set(GCFS_DIGEDAG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../digedag/src")
   include_directories(${GCFS_DIGEDAG_DIR})

   FIND_PACKAGE(Boost)
   INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

   FIND_PACKAGE(SAGA REQUIRED)
   include_directories(${SAGA_INCLUDE_DIR})
   link_directories(${SAGA_LIBRARIES_DIR})

   add_library(gcfs_digedag gcfs_filesystemdigedag.cpp gcfs_servicesagataskdata.cpp gcfs_servicedigedag.cpp)

   target_link_libraries(gcfs_digedag ${SAGA_LIBRARIES})
   target_link_libraries(gcfs_digedag ${GCFS_DIGEDAG_DIR}/libdigedag.so)

   add_custom_command(TARGET gcfs_digedag PRE_BUILD COMMAND "make" "SAGA_LOCATION=${SAGA_LOCATION}" "all" WORKING_DIRECTORY ${GCFS_DIGEDAG_DIR})
   
   get_target_property(MODULES gcfs MODULES)
   LIST(APPEND MODULES "gcfs_digedag")
   set_target_properties(gcfs PROPERTIES MODULES "${MODULES}")
   
   message(STATUS "   Support for Digedag: YES")
   
else(GCFS_MODULE_DIGEDAG)
      
   message(STATUS "   Support for Digedag: NO")

endif(GCFS_MODULE_DIGEDAG)
