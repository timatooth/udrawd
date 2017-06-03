#include <iostream>
#include <string>
#include <uWS/uWS.h>
#include "websocket.h"

void udrawd::websocket::init(){
    std::cout << "sup from websocket" << std::endl;

    uWS::Hub h;

    h.onMessage([](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
        ws->send(message, length, opCode);
    });

    h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
        std::string response = "hello";
        res->end( response.c_str(), response.size());
    });

    h.listen(4200);
    std::cout << "ws listening..." << std::endl;
    h.run();
    std::cout << "running..." << std::endl;
}
