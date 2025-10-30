#include <thread> 
#include <iostream>

#include <atomic> 
#include <condition_variable>

#include <queue> 
#include <set> 
#include "handshake.hpp"
#include <random>
using namespace std; 

namespace aicp
{
    // Handshake (reference IMplementation)

    // Note : this is implementation is a reference

    class SimpleHandshake : public Handshake 
    {
        HandshakeResult initiate (const string &local_id, const string &remote_addr ) const 
        {
            HandshakeResult r; 
            r.success = true; 
            r.peer_id = remote_addr; 
            // r.session_token = make_token("local_id", remote_addr); 
            return r; 
        }

        HandshakeResult respond(const string &incoming_blob) override 
        {
            HandshakeResult r; 
            // accept anything for now, produce ephemeral token.. 
            r.success  = true; 
            r.peer_id = "peer-from-blob"; 
            r.session_token = make_token("responder", incoming_blob); 
            return r; 
        }

        private: 
        string make_token(const string &a, const string &b)
        {
            // Naive token generator, replace with HMAC 
            hash<string> h ; 
            uint64_t v = h(a) ^ (h(b) << 1);
            return "sess-"  + to_string(v); 
        }
    };
}