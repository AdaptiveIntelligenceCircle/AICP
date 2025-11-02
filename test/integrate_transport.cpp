#include "../core/transport.h"
#include "../core/message_broker.hpp"
#include <iostream>
#include <thread>

#include "../core/transport.cpp"
#include "../core/message_broker.cpp"

using namespace aicp;

int main()
{
    auto transport = make_tcp_transport();

    transport->on_receive([](const MessageEnvelope &env) {
        std::cout << "[Received] from " << env.sender_id
                  << " -> " << env.protocol_id
                  << " payload: " << std::string(env.payload.begin(), env.payload.end()) << "\n";
    });

    transport->connect("127.0.0.1:5000");

    MessageEnvelope env;
    env.protocol_id = "demo";
    env.sender_id = "nodeA";
    env.payload = {'H', 'e', 'l', 'l', 'o'};
    env.sequence = 1;

    transport->send(env);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    transport->shutdown();
}
