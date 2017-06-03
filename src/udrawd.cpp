#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif // HAVE_FCNTL_H
#include <iostream>
#include <string>

#include <hiredis/hiredis.h>
#include <nghttp2/asio_http2_server.h>
#include <boost/algorithm/string.hpp>
#include "canvasapi.h"
#include "websocket.h"

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

int main(int argc, const char *argv[]) {
    boost::asio::ssl::context tls(boost::asio::ssl::context::sslv23);
    boost::system::error_code ec;

    if(argc > 2){
        tls.use_private_key_file(argv[1], boost::asio::ssl::context::pem);
        tls.use_certificate_chain_file(argv[2]);
    } else {
        std::cerr << "Usage: udrawd <serverkey.pem> <servercert.pem>" << std::endl;
        return -1;
    }

    http2 server;
    configure_tls_context_easy(ec, tls);

    //sets up redis connection for tile server
    udrawd::canvasapi::init();
    server.handle("/canvases/main/", udrawd::canvasapi::canvasRouter);

    //udrawd::websocket::init();

    std::cout << "Canvas API Listening on 0.0.0.0:4000" << std::endl;

    if (server.listen_and_serve(ec, tls, "0.0.0.0", "4000")) {
        std::cerr << "error: " << ec.message() << std::endl;
    }

    std::cout << "Starting up WebSocket server..." << std::endl;
}
