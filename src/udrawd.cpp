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

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

static redisContext *redisCtx;
struct timeval redisTimeout = {1, 500000}; // 1.5 seconds

void handleTileGet(std::string key, const response &res){
    redisReply *reply;
    std::string query = "HGET tile:" + key + " data";
    reply = (redisReply*) redisCommand(redisCtx, query.c_str());
    auto headers = header_map();

    if(reply->type == REDIS_REPLY_NIL){
        //no tile here yet send a 204 if tile is available for creation by users
        headers.emplace("access-control-allow-origin", header_value {"*"});
        res.write_head(204, headers);
        res.end();
    } else if(reply->type == REDIS_REPLY_STRING){
        //we're good to go with sending the tile data
        headers.emplace("content-length", header_value {std::to_string(reply->len)});
        headers.emplace("content-type", header_value {"image/png"});
        headers.emplace("access-control-allow-origin", header_value {"*"});
        //prepare the tile data as a string of bytes
        std::string response(reply->str, reply->len);
        res.write_head(200, headers);
        res.end(response);
    } else {
        res.write_head(500);
        res.end();
        std::cerr << "got a bad hiredis reply type #: " << reply->type << std::endl;
    }

    freeReplyObject(reply); //clean up redis response
}

void handleTilePut(const std::string key, const request &req, const response &res){
    uint8_t *dataBox = (uint8_t*) malloc(300 * 1000); //300K buffer
    std::shared_ptr<size_t> offset (new size_t(0)); //hold buffer size/position

    req.on_data([dataBox, offset, key, &res](const uint8_t *data, std::size_t len) {

        if(len > 0){
            //reject tiles greater than 300KB
            if (*offset.get() + len > 300 * 1000 - 1) {
                res.write_head(413); //request too large
                res.end();
                free(dataBox);
                return;
            }

            //copy the incoming data into buffer at offset levels
            memcpy(dataBox + *offset.get(), data, len);
            *offset.get() += len;
        } else if (len == 0) { //got EOF for tile upload
            redisReply *insertTile = (redisReply*) redisCommand(redisCtx, "HSET tile:%s data %b", key.c_str(), dataBox, *offset.get());
            if(insertTile->type == REDIS_REPLY_INTEGER){
                //successfully saved
                auto headers = header_map();
                headers.emplace("access-control-allow-origin", header_value {"*"});
                headers.emplace("access-control-allow-methods", header_value {"POST, GET, OPTIONS, PUT"});
                headers.emplace("access-control-allow-headers", header_value {"Content-Type"});
                res.write_head(201, headers);
                res.end();
            } else {
                res.write_head(500);
                res.end();
                std::cerr << "Unexpected save reply status is #" << insertTile->type << std::endl;
            }

            //cleanup time
            freeReplyObject(insertTile);
            free(dataBox);
        }
    });
}

void handleCorsPreflight(const request &req, const response &res){
    auto headers = header_map();
    headers.emplace("access-control-allow-origin", header_value {"*"});
    headers.emplace("access-control-allow-methods", header_value {"POST, GET, OPTIONS, PUT"});
    headers.emplace("access-control-allow-headers", header_value {"Content-Type"});
    res.write_head(200, headers);
    res.end();
}

void canvasRouter(const request &req, const response &res){
    std::vector<std::string> strs;
    boost::split(strs, req.uri().path, boost::is_any_of("/"));
    if(strs.size() != 6){
        res.write_head(400);
        res.end();
        return;
    }
    std::string key = strs[2] + ":" + strs[3] + ":" + strs[4] + ":" + strs[5];

    if (req.method() == "GET") {
        handleTileGet(key, res);
    } else if (req.method() == "PUT") {
        handleTilePut(key, req, res);
    } else if (req.method() == "OPTIONS"){
        handleCorsPreflight(req, res);
    }
}

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
    redisCtx = redisConnectWithTimeout("127.0.0.1", 6379, redisTimeout);

    server.handle("/canvases/main/", canvasRouter);

    std::cout << "Canvas API Listening on 0.0.0.0:4000" << std::endl;

    if (server.listen_and_serve(ec, tls, "0.0.0.0", "4000")) {
        std::cerr << "error: " << ec.message() << std::endl;
    }
}
