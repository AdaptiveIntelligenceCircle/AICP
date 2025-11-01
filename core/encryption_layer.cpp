#include <iostream> 
using namespace std;

#include <string> 
#include <vector>

#include <algorithm>
#include <array> 

#include <optional> 
#include <random> 

#include <sstream> 
#include <iomanip> 

#include <cstring> 
#include <mutex> 

#include "message_broker.hpp"
#include "handshake.hpp"

namespace aicp
{
    using ByteVector = vector<uint8_t>; 

    class EncryptionLayer
    {
        private: 
        array<uint8_t, 32> session_key{}; 
        bool intialized = false; 
        mutex key_mutex; 

        static ByteVector xor_cipher(const ByteVector &data, const ByteVector &key ){
            ByteVector out(data.size()); 
            for (size_t i = 0; i < data.size(); ++i)
            out[i] = data[i] ^ key[i % key.size()]; 
            return out; 
        }

        ByteVector str_to_bytes(const string &s)
        {
            return ByteVector(s.begin(), s.end()); 
        }

        string bytes_to_hex(const ByteVector &data)
        {
            ostringstream oss; 
            for (auto b : data)
            oss << hex << setw(2) << setfill('0') << (int)b;
            return oss.str(); 
        }

        public: 
        EncryptionLayer() = default; 

        void init_from_handshake(const HandshakeResult &hs)
        {
            lock_guard<mutex> lock(key_mutex); 
            if (!hs.success) 
            throw runtime_error("Cannot initialize"); 

            // derive simple session key .. 
            hash<string> hasher; 
            size_t hval = hasher(hs.session_token); 
            for (size_t i = 0 ;  i< session_key.size(); ++i)
            session_key[i] = static_cast<uint8_t>((hval >>(i %8) and 0xFF )); 
            intialized = true; 
        }

        ByteVector encrypt(const ByteVector &plaintext)
        {
            lock_guard <mutex> lock(key_mutex); 
            if (!intialized)
            throw runtime_error(""); 

            ByteVector key(session_key.begin(), session_key.end()); 
            ByteVector ciphertext = xor_cipher(plaintext, key);
            
            return ciphertext; 
        }

        ByteVector decrypt(const ByteVector &ciphertext)
        {
            lock_guard<mutex> lock(key_mutex); 

            if (!intialized)
            throw runtime_error ("Encryption Layer not worked"); 

            ByteVector key(session_key.begin(), session_key.end());
            ByteVector plaintext = xor_cipher(ciphertext, key);
            return plaintext;
        }

        string encrypt_to_hex(const string &text){
            auto ct = encrypt(str_to_bytes(text));
            return bytes_to_hex(ct); 
        }

        string decrypt_from_hex(const string &hex)
        {
            // simple hex decode..// 
            ByteVector bytes; 
            for (size_t i = 0 ; i < hex.size() ; i+=2){
                string byte_str = hex.substr(i, 2); 
                uint8_t byte  = static_cast<uint8_t> (strtol(byte_str.c_str(), nullptr, 16));
                bytes.push_back(byte); 
            }

            auto pt = decrypt(bytes); 
            return string(pt.begin(), pt.end()); 
        }

        bool is_ready() const 
        {
            return intialized; 
        }
    }; 

    // Example : intergration with broker message.. 
    void secure_publish(IMessageBroker &broker, const string &protocol, const std::string &sender, EncryptionLayer &layer, const std::string &msg){
        if (!layer.is_ready())
        {
            cout <<"IMessageBroker - Not Intialized\n"; 
            return ; 
        }

        MessageEnvelope env; 
        env.protocol_id = protocol;
        env.sender_id = sender; 
        env.sequence = chrono :: steady_clock :: now().time_since_epoch().count();
         
        auto payload = layer.encrypt(ByteVector(msg.begin(), msg.end()));
        env.payload = payload; 

        broker.publish(env); 
    }
}
