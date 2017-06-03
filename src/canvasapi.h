#include <nghttp2/asio_http2_server.h>
using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

namespace udrawd {
    namespace canvasapi {
        void init();
        void handleTileGet(std::string key, const response &res);
        void handleTilePut(const std::string key, const request &req, const response &res);
        void handleCorsPreflight(const request &req, const response &res);
        void canvasRouter(const request &req, const response &res);
    }
}
