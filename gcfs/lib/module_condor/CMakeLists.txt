add_library(gcfs_condor gcfs_servicecondor.cpp)

option(GCFS_MODULE_CONDOR "Add support for Condor grid framework")

if (GCFS_MODULE_CONDOR)
	
	get_target_property(MODULES gcfs MODULES)
	LIST(APPEND MODULES "gcfs_condor")
	set_target_properties(gcfs PROPERTIES MODULES "${MODULES}")
	
	message(STATUS "   Support for Condor: YES")
else(GCFS_MODULE_CONDOR)
   message(STATUS "   Support for Condor: NO")
endif(GCFS_MODULE_CONDOR)
