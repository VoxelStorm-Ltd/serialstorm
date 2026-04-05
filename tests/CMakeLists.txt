include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(test_serialstorm test_serialstorm.cpp)
target_link_libraries(test_serialstorm PRIVATE
  serialstorm::serialstorm
  Catch2::Catch2WithMain
)

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
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(test_serialstorm)
