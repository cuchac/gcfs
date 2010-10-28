add_library(gcfs_saga gcfs_servicesaga.cpp)

target_link_libraries(gcfs_saga "saga_adaptor_condor_job")

option(GCFS_MODULE_SAGA "Add support for SAGA grid control library")

if (GCFS_MODULE_SAGA)
	
	get_target_property(MODULES gcfs MODULES)
	LIST(APPEND MODULES "gcfs_saga")
	set_target_properties(gcfs PROPERTIES MODULES "${MODULES}")
	
	message(STATUS "   Support for Saga: YES")
else(GCFS_MODULE_SAGA)
   message(STATUS "   Support for Saga: NO")
endif(GCFS_MODULE_SAGA)