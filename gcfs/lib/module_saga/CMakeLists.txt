option(GCFS_MODULE_SAGA "Add support for SAGA grid control library")

if (GCFS_MODULE_SAGA)

	add_library(gcfs_saga gcfs_servicesaga.cpp gcfs_servicesaga_taskdata.cpp)

	FIND_PACKAGE(SAGA REQUIRED)

	target_link_libraries(gcfs_saga ${SAGA_LIBRARIES})
	include_directories(${SAGA_INCLUDE_DIR})
	
	get_target_property(MODULES gcfs MODULES)
	LIST(APPEND MODULES "gcfs_saga")
	set_target_properties(gcfs PROPERTIES MODULES "${MODULES}")
	
	message(STATUS "   Support for Saga: YES")

else(GCFS_MODULE_SAGA)
	
	message(STATUS "   Support for Saga: NO")
	
endif(GCFS_MODULE_SAGA)