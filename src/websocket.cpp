#include <iostream>
#include <string>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <uWS/uWS.h>
#include "websocket.h"

static int counter = 0;

typedef struct User {
    std::string* id;
} User;

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;


void udrawd::websocket::init(){

    uWS::Hub h;

    h.onConnection([](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
        std::cout << "new connection" << std::endl;
        User* newUser = (User*) malloc(sizeof(User));
        newUser->id = new std::string ("user-" + std::to_string(counter++));
        ws->setUserData(newUser);
    });

    h.onDisconnection([](uWS::WebSocket<uWS::SERVER> *ws, int, char *, size_t) {
        std::cout << "user disconnection" << std::endl;
        free(ws->getUserData());
    });

    h.onMessage([&h](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
        std::string mess = std::string(message, length);
        ptree pt;
        std::istringstream is(mess);
        read_json (is, pt);

        std::string mesageType = pt.get<std::string>("id");

        if(mesageType == "move") {
            ptree msg;
            msg.put("id", std::string("move"));
            msg.put("d.x", pt.get<float>("d.x"));
            msg.put("d.y", pt.get<float>("d.y"));
            msg.put("d.d1", pt.get<bool>("d.d1"));
            msg.put("d.id", *((User*) ws->getUserData())->id);

            std::ostringstream buf;
            write_json (buf, msg, false);
            std::string out = buf.str();
            std::cout << "move message: " + out << std::endl;
            h.getDefaultGroup<uWS::SERVER>().broadcast(out.c_str(), out.length(), uWS::TEXT);
        } else if(mesageType == "status"){
            ptree msg;
            msg.put("id", std::string("status"));
            msg.put("d.id", *((User*) ws->getUserData())->id);
            msg.put("d.tool", pt.get<std::string>("d.color"));
            msg.put("d.color", pt.get<std::string>("d.color"));
            msg.put("d.opacity", pt.get<float>("d.opacity"));
            msg.put("d.offsetX", pt.get<float>("d.offsetY"));
            msg.put("d.offsetY", pt.get<float>("d.offsetX"));

            std::ostringstream buf;
            write_json (buf, msg, false);
            std::string out = buf.str();
            std::cout << "state message: " + out << std::endl;
            h.getDefaultGroup<uWS::SERVER>().broadcast(out.c_str(), out.length(), uWS::TEXT);
        }
    });

    h.listen(4200);
    std::cout << "udraw WebSocket listening on 4200..." << std::endl;
    h.run();
}

int main(const int argc, const char *argv[]){
    udrawd::websocket::init();
}
