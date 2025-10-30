#pragma once 

#include <functional>
#include <string> 
#include <unordered_map>

#include <vector> 
#include <mutex> 

#include <memory> 
#include <optional>

using namespace std;
namespace aicp
{
    using MessagePayload = vector<uint8_t>; 

    struct MessageEnvelope
    {
        string protocol_id; 
        string sender_id; 
        uint64_t sequence; 
        MessagePayload payload; // already encrypted AEAD blob..
        vector<uint8_t> signature; 
    }; 

    class IMessageBroker
    {
        public: 
        // publish a messsage into the broker, broker is responsible
        virtual void publish(const MessageEnvelope &env) =0; 

        // Subcribe to messages that match protocol_id 
        // callback will be invoked in broker's worker threads..
        virtual void subcribe(const string &protocoi_id, function<void(const MessageEnvelope&)>cb ) = 0;
        
        // graceful shutdown.. 
        virtual void shutdown() = 0; 

        virtual ~IMessageBroker() = default; 
    };  
} // namespace aicp...// 
