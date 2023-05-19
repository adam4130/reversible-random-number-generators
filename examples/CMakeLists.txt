option(BUILD_EXAMPLES "Build reversible RNG examples" OFF)
if(BUILD_EXAMPLES)
  find_package(OpenSSL REQUIRED)

  add_executable(benchmark benchmark.cpp)
  target_link_libraries(benchmark PRIVATE OpenSSL::SSL Reverse)
endif()