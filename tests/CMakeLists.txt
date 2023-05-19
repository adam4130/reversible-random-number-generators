# ----------------------------------------------------------------------
# Catch2

FetchContent_Declare(Catch2
                     GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                     GIT_TAG        v3.3.2)

FetchContent_MakeAvailable(Catch2)

add_executable(reverse_test reverse_test.cpp)
target_link_libraries(reverse_test PRIVATE Catch2::Catch2WithMain Reverse)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(reverse_test)

# ----------------------------------------------------------------------
# TestU01 (optional)

option(BUILD_TESTU01 "Build TestU01 statistical batteries" OFF)
if(BUILD_TESTU01)
  find_package(Boost REQUIRED)

  set(TESTU01 "testu01")
  set(TESTU01_EP "${TESTU01}_ep")

  include(ExternalProject)
  ExternalProject_Add(${TESTU01_EP}
                      URL "http://simul.iro.umontreal.ca/testu01/TestU01.zip"
                      BUILD_IN_SOURCE TRUE
                      CONFIGURE_COMMAND "./configure" "--disable-static"
                      INSTALL_COMMAND   "")

  ExternalProject_Get_property(${TESTU01_EP} SOURCE_DIR)
  set(TESTU01_INCLUDE_DIR "${SOURCE_DIR}/include")
  set(TESTU01_SHARED_LIBRARY "${SOURCE_DIR}/${TESTU01}/.libs/${CMAKE_SHARED_LIBRARY_PREFIX}${TESTU01}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  file(MAKE_DIRECTORY ${TESTU01_INCLUDE_DIR})

  add_library(${TESTU01} SHARED IMPORTED)
  set_target_properties(${TESTU01} PROPERTIES IMPORTED_LOCATION ${TESTU01_SHARED_LIBRARY}
                                              INTERFACE_INCLUDE_DIRECTORIES ${TESTU01_INCLUDE_DIR})

  add_executable(battery battery.cpp)
  add_dependencies(battery ${TESTU01_EP})
  target_link_libraries(battery PRIVATE ${TESTU01} Reverse)
  target_include_directories(battery PRIVATE ${Boost_INCLUDE_DIR})
endif()
