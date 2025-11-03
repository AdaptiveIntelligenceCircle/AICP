#include <string> 
#include <vector> 
#include <sstream> 
#include <iomanip> 
#include <iostream> 
#include <cstdint> 
#include <string>

using namespace std; 

#if __has_include(<openssl/sha.h>)
#include <openssl/sha.h>
#endif 

namespace aicp :: utils
{
    // compute SHA256 checksum.. // 
    string sha256(const string &data)
    {
#if __has_include(<openssl/sha.h>)
        unsigned char hash[SHA256_DIGEST_LENGTH]; 
        SHA256(reinterpret_cast<const unsigned char*> (data.data()), data.size(), hash) ; 

        ostringstream oss;  
        oss << hex << setfill('0'); 
        for (int i = 0 ; i < SHA256_DIGEST_LENGTH ; ++i)
        oss << setw(2) << static_cast<int>(hash[i]); 
        return oss.str(); 
#else 
        // fallback : simple hash.. // 
        hash<string>h; 
        
        oss << hex << h(data); 
        return oss.str();
#endif  
    }

    // Compute CRC32 (simplified, software implementation..)
    uint32_t crc32(const vector<uint8_t> &data)
    {
        uint32_t crc = 0xFFFFFFFF; 
        for (auto byte : data)
        {
            crc ^= byte; 
            for (int i = 0 ; i< 8 ; ++i)
            crc = (crc >> 1) ^(0xEDB88320 & -(crc & 1));
        }
        return ~crc ; 
    }

    // Verify crc32 checksum.. // 
    bool verify_crc32(const vector<uint8_t> &data, uint32_t expected)
    {
        return crc32(data) == expected; 
    }

    // Compute combined hash (CRC32 + SHA256)

    string hybrid_hash(const vector<uint8_t> &data)
    {
        uint32_t crc = crc32(data); 
        string crc_hex; 
        {
            ostringstream oss;
            oss << hex << crc; 
            crc_hex = oss.str(); 
        }

        string sha = sha256(string(data.begin(), data.end()));
        return sha + "-" + crc_hex;
    }
}