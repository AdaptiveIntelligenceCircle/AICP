#pragma once 

#include <iostream> 
using namespace std;

#include <string> 
#include <algorithm> 

#include <optional>
#include <chrono>

namespace aicp
{
    struct HandshakeResult
    {
        bool success; 
        string peer_id;
        string session_token; 
        string error; 
    };

    class Handshake 
    {
        public: 
        // start handshake with remote peer...
        // returns result with session_token if success.. 
        virtual HandshakeResult initiate(const string &local_id, const string &remote_addr) = 0;
        
        // respond to an incoming handshake (transport-level)
        virtual HandshakeResult respond(const string &incoming_blob); 

        virtual ~Handshake() = default; 
    }; 
} // namespace aicp.. 
