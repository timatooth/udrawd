#include <iostream>
#include <string>
#include <uWS/uWS.h>
#include "websocket.h"

void udrawd::websocket::init(){

    // keep all state in redis

    uWS::Hub h;

    h.onMessage([](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
        std::string mess = std::string(message, length);
        std::cout << mess << std::endl;
        ws->send(message, length, opCode);
    });

    h.listen(4200);
    std::cout << "udraw WebSocket listening on 4200..." << std::endl;
    h.run();
}

int main(const int argc, const char *argv[]){
    udrawd::websocket::init();
}
