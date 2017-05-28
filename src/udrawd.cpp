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

#include "MathFunctions/MathFunctions.h"

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

int main(int argc, char *argv[]) {
    boost::asio::ssl::context tls(boost::asio::ssl::context::sslv23);
    boost::system::error_code ec;
    std::string docroot;
    std::cout << mysqrt(4) << std::endl;

    if(argc > 3){
        tls.use_private_key_file(argv[1], boost::asio::ssl::context::pem);
        tls.use_certificate_chain_file(argv[2]);
        docroot = argv[3];
    } else {
        std::cerr << "Usage: udrawd <serverkey.pem> <servercert.pem> <docroot>" << std::endl;
        return -1;
    }

    http2 server;
    configure_tls_context_easy(ec, tls);

    redisContext *c;
    struct timeval timeout = {1, 500000}; // 1.5 seconds
    c = redisConnectWithTimeout("127.0.0.1", 6379, timeout);

    server.handle("/canvases/main/", [c](const request &req, const response & res) {
        std::vector<std::string> strs;
        boost::split(strs, req.uri().path, boost::is_any_of("/"));
        if(strs.size() != 6){
            res.write_head(400);
            res.end();
            return;
        }
        std::string key = strs[2] + ":" + strs[3] + ":" + strs[4] + ":" + strs[5];

        if(req.method() == "GET"){
            redisReply *reply;
            std::string query = "HGET tile:" + key + " data";
            reply = (redisReply*) redisCommand(c, query.c_str());
            auto headers = header_map();

            if(reply->type == REDIS_REPLY_NIL){
                //no tile here yet send a 204 if tile is available for creation by users
                res.write_head(204);
                res.end();
            } else if(reply->type == REDIS_REPLY_STRING){
                //we're good to go with sending the tile data
                headers.emplace("content-length", header_value {std::to_string(reply->len)});
                headers.emplace("content-type", header_value {"image/png"});
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

        } else if (req.method() == "PUT") {
            uint8_t *dataBox = (uint8_t*) malloc(300 * 1000); //300K buffer
            std::shared_ptr<size_t> offset (new size_t(0)); //hold buffer size/position

            req.on_data([dataBox, offset, key, c, &res](const uint8_t *data, std::size_t len) {

                if(len > 0){
                    if(*offset.get() + len > 300 * 1000 - 1){
                        res.write_head(413); //request too large
                        res.end();
                        free(dataBox);
                        return;
                    }

                    //copy the incoming data into buffer at offset levels
                    memcpy(dataBox + *offset.get(), data, len);
                    *offset.get() += len;
                } else if (len == 0) { //got EOF

                    redisReply *insertTile = (redisReply*) redisCommand(c, "HSET tile:%s data %b", key.c_str(), dataBox, *offset.get());
                    if(insertTile->type == REDIS_REPLY_INTEGER){
                        //successfully saved
                        res.write_head(201);
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
    });

    server.handle("/", [&docroot](const request &req, const response & res) {
        auto path = percent_decode(req.uri().path); //is const char*

        if (!check_path(path)) {
            std::cout << "bad path" << std::endl;
          res.write_head(400);
          res.end();
          return;
        }

        path = docroot + path;
        std::cout << "serving " << path << std::endl;
        auto fd = open(path.c_str(), O_RDONLY);
        if (fd == -1) {
          res.write_head(404);
          res.end();
          return;
        }

        auto header = header_map();
        struct stat stbuf;
        if (stat(path.c_str(), &stbuf) == 0) {
            header.emplace("content-length",
                           header_value{std::to_string(stbuf.st_size)});
            header.emplace("last-modified",
                           header_value{http_date(stbuf.st_mtime)});
        }
        res.write_head(200, std::move(header));
        res.end(file_generator_from_fd(fd));

    });

    std::cout << "Listening on 0.0.0.0:4000" << std::endl;

    if (server.listen_and_serve(ec, tls, "0.0.0.0", "4000")) {
        std::cerr << "error: " << ec.message() << std::endl;
    }
}
