cmake_minimum_required(VERSION 2.8.8)
project(f2h)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Now build our tools
add_executable(f2h
  llvm-dwarfdump.cpp
)

set_property(TARGET f2h PROPERTY CXX_STANDARD 11)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs debuginfodwarf object support)

# Link against LLVM libraries
target_link_libraries(f2h ${llvm_libs})
