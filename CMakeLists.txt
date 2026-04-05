cmake_minimum_required(VERSION 3.14)
project(serialstorm
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only library target: consumers add the repo root to their include path
# and include headers as "serialstorm/stream_std_stream.h" etc.
add_library(serialstorm INTERFACE)
add_library(serialstorm::serialstorm ALIAS serialstorm)
target_include_directories(serialstorm INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# Optional test suite
option(SERIALSTORM_BUILD_TESTS "Build the serialstorm test suite" ON)
if(SERIALSTORM_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
