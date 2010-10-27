message(STATUS "Selected backends:")

set_target_properties(gcfs PROPERTIES MODULES "")

add_subdirectory(module_condor)

add_subdirectory(module_saga)