# ----------------------------------------------------------------------
# PCG, A Family of Better Random Number Generators

FetchContent_Declare(pcg-cpp
                     GIT_REPOSITORY https://github.com/imneme/pcg-cpp.git
                     GIT_TAG        master)

FetchContent_MakeAvailable(pcg-cpp)
set(PCG_INCLUDE_DIRS "${pcg-cpp_SOURCE_DIR}/include")

add_library(Reverse STATIC exponential.cpp
                           mersenne.cpp
                           normal.cpp
                           pcg.cpp
                           reverse.cpp
                           uniform.cpp
                           xoshiro.cpp)

target_include_directories(Reverse PUBLIC
        $<BUILD_INTERFACE:${PCG_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

file(GLOB PUBLIC_HEADERS *.h ${PCG_INCLUDE_DIRS}/*.hpp)
set_target_properties(Reverse PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS Reverse
        EXPORT ReverseConfig
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ReverseConfig
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")

export(TARGETS Reverse FILE "${CMAKE_CURRENT_BINARY_DIR}/ReverseConfig.cmake")

# ----------------------------------------------------------------------
# C Wrapper for Python ctypes

add_library(Wrapper SHARED wrapper.cpp)
target_link_libraries(Wrapper PRIVATE Reverse)

install(TARGETS Wrapper LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
