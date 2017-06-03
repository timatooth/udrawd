cmake_minimum_required (VERSION 3.1)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")
project (udrawd)

SET (CMAKE_CXX_STANDARD 11)

# The version number.
set (udrawd_VERSION_MAJOR 0)
set (udrawd_VERSION_MINOR 1)
set (udrawd_VERSION_PATCH 0)

add_subdirectory (src)
MESSAGE( STATUS "CMAKE_MODULE_PATH:         " ${CMAKE_MODULE_PATH} )
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/udrawdConfig.h.in"
  "${PROJECT_BINARY_DIR}/udrawdConfig.h"
  )

# Mac Homebrew OpenSSL workarounds
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
    set(OPENSSL_LIBRARIES "/usr/local/opt/openssl/include")
    set(OPENSSL_LIBRARY_DIR "/usr/local/opt/openssl/lib")
    # Unfortuately this is not enough because mac has a fucked bundled openssl
    set(CMAKE_CXX_FLAGS "-I/usr/local/opt/openssl/include")
    set(CMAKE_MODULE_LINKER_FLAGS "-L/usr/local/opt/openssl/lib")
endif()

find_package(openssl REQUIRED)
find_package(Boost 1.54.0 REQUIRED COMPONENTS system thread)
find_package(hiredis REQUIRED)
find_package(nghttp2 REQUIRED)
find_package(uwebsockets REQUIRED)
find_package(zlib REQUIRED)
find_package(libuv REQUIRED)

# cmake tutorial Math library
include_directories ("${PROJECT_SOURCE_DIR}/MathFunctions")
add_subdirectory ("src/MathFunctions")


# add the binary tree to the search path for include files
# so that we will find udrawdConfig.h
include_directories("${PROJECT_BINARY_DIR}")

MESSAGE( STATUS "Boost_LIBRARIES:         " ${Boost_LIBRARIES} )
MESSAGE( STATUS "NGHTTP2_LIBRARIES:       " ${NGHTTP2_LIBRARIES} )
MESSAGE( STATUS "UWEBSOCKETS_LIBRARIES:   " ${UWEBSOCKETS_LIBRARIES} )
MESSAGE( STATUS "ZLIB_LIBRARIES:          " ${ZLIB_LIBRARIES} )
MESSAGE( STATUS "LIBUV_LIBRARIES:         " ${LIBUV_LIBRARIES} )

add_executable(udrawd
  "src/udrawd.cpp"
  "src/canvasapi.cpp"
  "src/websocket.cpp"
)

target_link_libraries (udrawd
  MathFunctions
  ${Boost_LIBRARIES}
  ${HIREDIS_LIBRARIES}
  ${NGHTTP2_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${UWEBSOCKETS_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${LIBUV_LIBRARIES}
)
