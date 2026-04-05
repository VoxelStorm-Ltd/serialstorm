cmake_minimum_required(VERSION 3.14)
project(serialstorm_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  cast_if_required
  GIT_REPOSITORY https://github.com/VoxelStorm-Ltd/cast_if_required.git
  GIT_TAG        0237e7df3a25b5c34edb864ac20eb7e98d3f24cc
)
FetchContent_MakeAvailable(cast_if_required)

add_executable(test_serialstorm test_serialstorm.cpp)

# Add the repository root (serialstorm/ headers) and cast_if_required to include paths.
target_include_directories(test_serialstorm PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${cast_if_required_SOURCE_DIR}
)

target_link_libraries(test_serialstorm PRIVATE Catch2::Catch2WithMain)

# Optional code coverage instrumentation (GCC / Clang only)
option(SERIALSTORM_COVERAGE "Enable code coverage instrumentation" OFF)
if(SERIALSTORM_COVERAGE)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(WARNING "SERIALSTORM_COVERAGE is only supported with GCC or Clang")
  else()
    target_compile_options(test_serialstorm PRIVATE --coverage -O0 -g)
    target_link_options(test_serialstorm PRIVATE --coverage)
  endif()
endif()

# Register individual Catch2 test cases with CTest
enable_testing()
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(test_serialstorm)
