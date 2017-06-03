# Try to find uWebSocket https://github.com/uNetworking/uWebSockets
# Once done, this will define
#
# UWEBSOCKETS_FOUND        - system has uwebsockets
# UWEBSOCKETS_INCLUDE_DIRS - uwebsockets include directories
# UWEBSOCKETS_LIBRARIES    - libraries need to use uwebsockets

if(UWEBSOCKETS_INCLUDE_DIRS AND UWEBSOCKETS_LIBRARIES)
  set(UWEBSOCKETS_FIND_QUIETLY TRUE)
else()
  find_path(
    UWEBSOCKETS_INCLUDE_DIR
    NAMES uWS/uWS.h
    HINTS ${UWEBSOCKETS_ROOT_DIR}
    PATH_SUFFIXES include)

  find_library(
    UWEBSOCKETS_LIBRARY
    NAMES uWS
    HINTS ${UWEBSOCKETS_ROOT_DIR}
    PATH_SUFFIXES ${CMAKE_INSTALL_LIBDIR})

  set(UWEBSOCKETS_INCLUDE_DIRS ${UWEBSOCKETS_INCLUDE_DIR})
  set(UWEBSOCKETS_LIBRARIES ${UWEBSOCKETS_LIBRARY})

  include (FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
  uwebsockets DEFAULT_MSG UWEBSOCKETS_LIBRARY UWEBSOCKETS_INCLUDE_DIR)

  mark_as_advanced(UWEBSOCKETS_LIBRARY UWEBSOCKETS_INCLUDE_DIR)
endif()
