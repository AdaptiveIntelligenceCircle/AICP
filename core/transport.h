// transport.h
#pragma once
#include "message_broker.hpp" // reuse MessageEnvelope type
#include <functional>
#include <memory>
#include <string>

namespace aicp {
class ITransport {
public:
    // connect to remote (for client transports)
    virtual bool connect(const std::string &endpoint) = 0;

    // bind/listen (for server transports)
    virtual bool listen(const std::string &endpoint) = 0;

    // send a raw envelope to remote (one-way)
    virtual bool send(const MessageEnvelope &env) = 0;

    // register a callback for incoming envelopes
    virtual void on_receive(std::function<void(const MessageEnvelope&)> cb) = 0;

    // graceful shutdown
    virtual void shutdown() = 0;

    virtual ~ITransport() = default;
};

std::unique_ptr<ITransport> make_tcp_transport();
std::unique_ptr<ITransport> make_grpc_transport_client(const std::string &target); // factory
std::unique_ptr<ITransport> make_grpc_transport_server(int port); // server
}
