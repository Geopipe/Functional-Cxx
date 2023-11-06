project(FunctionCXXExamples)

add_executable(flat-iterator flat-iterator.cc)
target_link_libraries(flat-iterator functional-cxx)

add_executable(lazy-wrapper lazy-wrapper.cc)
target_link_libraries(lazy-wrapper functional-cxx)

add_executable(streams streams.cc)
target_link_libraries(streams functional-cxx)
