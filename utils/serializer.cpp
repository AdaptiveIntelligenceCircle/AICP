#include <string> 
#include <sstream> 

#include <iomanip> 
#include <vector> 

#include <unordered_map>
#include <iostream> 

#include <variant> 

#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#endif

#include "../core/message_broker.hpp"
#include "../security/cert_manager.h"
#include "../security/intrusion_log.h"

namespace aicp :: utils
{
    using json = nlohmann :: json; 

    // --------Generic Utilities ---------- // 
    template <typename T> 
    string to_hex(const vector<T> &data){
        ostringstream oss; 
        oss << hex << setfill('0');
        for (auto &v : data)
        oss << setw(2) << static_cast<int>(v); 
        return oss.str(); 
    }

    template <typename T> 
    vector <uint8_t> from_hex(const string &hex)
    {
        vector<uint8_t> data; 
        for (size_t i = 0 ; i< hex.size(); i+=2){
            string byte = hex.substr(i, 2); 
            uint8_t val = static_cast<uint8_t>(stoul (byte, nullptr, 16)); 
            data.push_back(val);
        }
        return data; 
    }

    // ------- MessageEnvelope ------- // 
    string serialize_envelope(const MessageEnvelope &env)
    {
        json j; 
        j["protocol_id"] = env.protocol_id;
        j["sender_id"] = env.sender_id; 
        j["sequence"] = env.sequence; 
        j["payload"] = to_hex(env.payload); 
        j["signature"] = to_hex(env.signature); 

        return j.dump(); 
    }

    aicp :: MessageEnvelope desrialize_envelope(const string &json_str)
    {
        auto j = json :: parse(json_str); 
        aicp :: MessageEnvelope env; 
        env.protocol_id = j.value("protocol_id", ""); 
        env.sender_id = j.value("sender_id", ""); 
        env.sequence = j.value("sequence", 0);
        env.payload = from_hex<uint8_t> (j.value("payload", "")); 
        env.signature = from_hex<uint8_t>(j.value("signature", "")); 
        return env; 
    }

    // ------------ Certificate --------------- // 
    string serialize_certificate(const aicp :: security :: Certificate &c)
    {
        json j; 
        j["subject"] = c.subject; 
        j["issuer"] = c.issuer; 
        j["public key"] = c.public_key; 
        j["signature"] = c.signature; 
        j["is_root"] = c.is_root; 

        j["valid_from"] = chrono::duration_cast<chrono::seconds>(
        c.valid_from.time_since_epoch())
        .count();
        j["valid_until"] = chrono::duration_cast<chrono::seconds>(
        c.valid_until.time_since_epoch())
        .count();

        return j.dump(); 
    }

    security :: Certificate desrialize_certificate(const string &json_str)
    {
        auto j = json :: parse(json_str); 
        aicp :: security :: Certificate c; 
        c.subject = j.value("subject", ""); 
        c.issuer = j.value("issuer", "");
        c.public_key = j.value("public_key", "");
        c.signature = j.value("signature", "");
        c.is_root = j.value("is_root", false);

        using clock = chrono :: system_clock; 
        c.valid_from = clock:: time_point(chrono :: seconds(j.value("valid_from", 0)));
        c.valid_until = clock :: time_point(chrono :: seconds(j.value("valid_unitl", 0)));
        return c; 
    }

    // ----------- Intrusion Record ------------- // 
    string serialize_intrusion(const aicp :: security :: IntrusionRecord &r)
    {
        json j;  
        j["source"] = r.source; 
        j["description"] = r.description; 
        j["severity"] = static_cast<int> (r.severity); 

        if (r.session_id)
        j["session_id"] = *r.session_id;
        return j.dump();  
    }

    aicp :: security :: IntrusionRecord desrialize_intrusion(const string &json_str)
    {
        auto j = json :: parse(json_str); 
        aicp :: security :: IntrusionRecord r;
        r.source = j.value("source", "") ; 
        r.description = j.value("description", ""); 
        r.severity = static_cast<aicp :: security :: IntrusionSeverity> (j.value("", 0));
        using clock = chrono :: system_clock; 
        r.timestamp = clock::time_point(chrono::seconds(j.value("timestamp", 0)));
        if (j.contains("session_id"))
        r.session_id = j["session_id"].get<string>(); 
        return r; 
    }

    // ------------- Utility Aggregation --------------- // 
    string pretty_json(const string &raw)
    {
        try 
        {
            auto j = json :: parse(raw); 
            return j.dump(4); 
        }
        catch (...)
        {
            return raw; 
        }
    }
} // namespace aicp :: utils.. 