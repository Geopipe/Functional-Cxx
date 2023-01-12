project(FunctionCXXExamples)

add_executable(lazy-wrapper lazy-wrapper.cc)
target_link_libraries(lazy-wrapper functional-cxx)

add_executable(streams streams.cc)
target_link_libraries(streams functional-cxx)

add_executable(optional optional.cc)
target_link_libraries(optional functional-cxx)
