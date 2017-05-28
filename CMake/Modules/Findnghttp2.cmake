#credit https://github.com/curl/curl/blob/master/CMake/FindNGHTTP2.cmake
include(FindPackageHandleStandardArgs)

find_path(NGHTTP2_INCLUDE_DIR "nghttp2/nghttp2.h")

find_library(NGHTTP2_LIBRARY NAMES nghttp2)
find_library(NGHTTP2_ASIO_LIBRARY NAMES nghttp2_asio)

find_package_handle_standard_args(NGHTTP2
    FOUND_VAR
      NGHTTP2_FOUND
    REQUIRED_VARS
      NGHTTP2_LIBRARY
      NGHTTP2_INCLUDE_DIR
      NGHTTP2_ASIO_LIBRARY
    FAIL_MESSAGE
      "Could NOT find NGHTTP2 or NGHTTP2_ASIO"
)

set(NGHTTP2_INCLUDE_DIRS ${NGHTTP2_INCLUDE_DIR} )
set(NGHTTP2_LIBRARIES ${NGHTTP2_LIBRARY} ${NGHTTP2_ASIO_LIBRARY})
