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
#include <fstream> //for testing

#include <hiredis/hiredis.h>
#include <nghttp2/asio_http2_server.h>
#include <boost/algorithm/string.hpp>

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

int main(int argc, char *argv[]) {
    boost::asio::ssl::context tls(boost::asio::ssl::context::sslv23);
    boost::system::error_code ec;
    std::string docroot;
    
    if(argc > 3){
        tls.use_private_key_file(argv[1], boost::asio::ssl::context::pem);
        tls.use_certificate_chain_file(argv[2]);
        docroot = argv[3];
    } else {
        std::cerr << "Usage: udrawd <server.key> <server.crt> <docroot>" << std::endl;
        return -1;
    }

    http2 server;
    configure_tls_context_easy(ec, tls);

    redisContext *c;
    struct timeval timeout = {1, 500000}; // 1.5 seconds
    c = redisConnectWithTimeout("127.0.0.1", 6379, timeout);

    server.handle("/canvases/main/", [c](const request &req, const response & res) {
        //unsafe split, todo redo
        std::vector<std::string> strs;
        boost::split(strs, req.uri().path, boost::is_any_of("/"));
        //danger zone
        std::string key = strs[2] + ":" + strs[3] + ":" + strs[4] + ":" + strs[5];
        
        std::vector<uint8_t> *incoming = new std::vector<uint8_t>;
        
        if(req.method() == "GET"){
            redisReply *reply;
            std::string query = "GET " + key;
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
                std::cerr << "got a bad hiredis reply type #: " << reply->type << std::endl;
            }
            freeReplyObject(reply); //clean up redis response

        } else if (req.method() == "PUT") {
            //todo redo c++ sucks. swith to memcpy :)
            req.on_data([incoming, &res](const uint8_t *data, std::size_t len){
                auto headers = header_map();
                std::cout << "got data of size: " << len << std::endl;
                
                if(len > 0){
                    //the noob way
                    for(int i = 0; i < len; i++){
                        incoming->push_back(data[i]);
                    }
                } else if(len == 0){
                    std::cout << "buffer done with total: " << incoming->size() << std::endl;
                    
                    //spit result back
                    headers.emplace("content-length", header_value {std::to_string(incoming->size())});
                    headers.emplace("content-type", header_value {"image/png"});
                    res.write_head(200, headers);
                    std::string stringres(incoming->begin(), incoming->end());
                    
                    //debugging
                    std::ofstream file("lastput.bin.png", std::ios::binary);
                    //file.write(reinterpret_cast<const char*>(incoming), incoming->size());
                    file.write(reinterpret_cast<const char*>(incoming), incoming->size());
                    file.close();
                    //
                    
                    res.end(stringres);
                    
                    delete incoming;
                }
                
                
            });
            //res.write_head(503); //unavailable
            //res.end();
        }
        
    });
    
    server.handle("/socket.io/", [](const request &req, const response & res) {
        auto header = header_map();
        header.emplace("location", header_value{"https://udraw.me" + req.uri().path +"?"+ req.uri().raw_query });
        res.write_head(301, header);
        res.end();
    });
    
    
    server.handle("/", [&docroot](const request &req, const response & res) {
        /*boost::system::error_code ec;
        auto push = res.push(ec, "GET", "/socket.io/");
        push->write_head(404);
        push->end("nope");*/
        
        res.write_head(200);
        std::cout << "serving: " << docroot << "/index.html" << std::endl;
        res.end(file_generator(docroot + "/index.html"));
    });
    
    server.handle("/static/", [&docroot](const request &req, const response & res) {
        auto path = percent_decode(req.uri().path);
        if (!check_path(path)) {
          res.write_head(400);
          res.end();
          return;
        }
       
        path = docroot + path;
        //std::cout << "opening " << path << std::endl;
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
    
    std::cerr << "Listening on 0.0.0.0:3000" << std::endl;
    
    if (server.listen_and_serve(ec, tls, "0.0.0.0", "3000")) {
        std::cerr << "error: " << ec.message() << std::endl;
    }
}

