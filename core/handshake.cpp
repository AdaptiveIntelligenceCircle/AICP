#include <iostream> 
using namespace std; 

#include <random>
#include <iomanip>

#include <thread>
#include "handshake.hpp"

namespace aicp
{
    namespace
    {
        // helper : generate pseudo-random session token..
        string generate_token(size_t length = 32)
        {
            static const char charset[]
            = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            static thread_local mt19937 rg {random_device{}()};
            static thread_local uniform_int_distribution<>dist(0, sizeof(charset) -2); 
            string token; 
            token.reserve(length);
            for (size_t i = 0 ; i < length ; ++i)
            {
                token.push_back(charset[dist(rg)]); 
            }
            return token; 
        }
        string current_timestamp()
        {
            using namespace chrono; 
            auto now = system_clock :: now(); 
            auto time = system_clock :: to_time_t(now); 
            stringstream ss; 
            ss << put_time(gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
            return ss.str();  
        }

    }

    class DefaultHandshake : public Handshake{
        public: 
        HandshakeResult initiate(const string &local_id, const string &remote_addr)
        {
            // simulate handshake over network.. // 
            HandshakeResult res; 
            res.peer_id = remote_addr; 
            if (local_id.empty() or remote_addr.empty())
            {
                res.success = false; 
                res.error = "invalid peer identifiers"; 
                return res; 
            }
            // pseudo exchange logic.. // 
            this_thread :: sleep_for(chrono :: milliseconds(100)); 
            res.session_token = generate_token();
            res.success  = true; 
            return res; 
        }

        HandshakeResult respond (const string &incoming_blob) override{
            HandshakeResult res; 
            if (incoming_blob.find("SYN") == string :: npos)
            {
                res.success = false;
                res.error = "Invalid handshake blob" ; 
                return res; 
            }

            // decode peer id (mock)
            res.peer_id = "remote-peer"; 
            res.session_token = generate_token(); 
            res.success = true;
            return res; 
        }
        // 
    };  

    // Factory function 
    unique_ptr<Handshake> make_default_handshake()
    {
        return make_unique<DefaultHandshake>(); 
    }
}