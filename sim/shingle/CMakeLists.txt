#-DCMAKE_BUILD_TYPE={Debug|Release}
#-DCMAKE_VERBOSE_MAKEFILE=on

cmake_minimum_required(VERSION 3.7.2)
project(shingle)


# GTest
include(ExternalProject)
ExternalProject_Add(GTEST
  SOURCE_DIR /usr/src/googletest/googletest
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(GTEST source_dir)
set(GTEST_INCLUDE_DIR ${source_dir}/include)
ExternalProject_Get_Property(GTEST binary_dir)
set(GTEST_LIBRARY_PATH ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
set(GTEST_LIBRARY gtest)
add_library(${GTEST_LIBRARY} UNKNOWN IMPORTED)
set_property(TARGET ${GTEST_LIBRARY} PROPERTY IMPORTED_LOCATION ${GTEST_LIBRARY_PATH})
add_dependencies(${GTEST_LIBRARY} GTEST)


# definitions
include_directories("src/")
set(SHN_COMPILE_FLAGS "-std=c++14 -fPIC -Wall -Werror -Wextra -Wno-unused -Wno-sign-compare -march=native -Wno-implicit-fallthrough")
set(SHN_LINK_FLAGS "-rdynamic")
set(SHN_DEPS "pthread")
set(SHN_LIB "shingle")
set(SHN_TEST "shingle_test")
set(SHN_EXE "shingle_exe")

# library
add_library(${SHN_LIB} STATIC
  src/shingle.cc
  src/isalphanum.cc
)
set_target_properties(${SHN_LIB} PROPERTIES COMPILE_FLAGS ${SHN_COMPILE_FLAGS})


# executable
add_executable(${SHN_EXE} src/${SHN_EXE}.cc)
set_target_properties(${SHN_EXE} PROPERTIES COMPILE_FLAGS ${SHN_COMPILE_FLAGS})
set_target_properties(${SHN_EXE} PROPERTIES LINK_FLAGS ${SHN_LINK_FLAGS})
target_link_libraries(${SHN_EXE} ${SHN_LIB} ${SHN_DEPS})


# tests
add_executable(${SHN_TEST} src/${SHN_TEST}.cc)
target_include_directories(${SHN_TEST} PRIVATE ${GTEST_INCLUDE_DIR})
set_target_properties(${SHN_TEST} PROPERTIES COMPILE_FLAGS ${SHN_COMPILE_FLAGS})
set_target_properties(${SHN_TEST} PROPERTIES LINK_FLAGS ${SHN_LINK_FLAGS})
target_link_libraries(${SHN_TEST} ${SHN_LIB} ${GTEST_LIBRARY} ${SHN_DEPS})
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.10.0") 
  include(GoogleTest)
  gtest_discover_tests(${SHN_TEST})
else()
  add_test(NAME ${SHN_TEST} COMMAND ${SHN_TEST})
endif()

