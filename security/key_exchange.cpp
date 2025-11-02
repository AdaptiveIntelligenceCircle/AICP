#include <iostream>
using namespace std; 

#include <vector> 
#include <string>

#include <cmath> 
#include <algorithm>

#include "cert_manager.h"
#include "handshake.hpp"

#include <iomanip> 
#include <chrono>
#include <random> 
#include <sstream> 

#if __has_include(<openssl/sha.h>)
#include <openssl/sha.h>
#endif 

namespace aicp 
{
    namespace ke // key exchange.. 
    {
        using namespace chrono; 
        using namespace security;
        
        // --- Hepler : hashing / pseudo-derivation 
        static string hash_hex(const string &data)
        {
            unsigned char digest[SHA256_DIGEST_LENGTH]; 
            SHA256(reinterpret_cast<const unsigned char*> (data.data()), data.size(), digest);
            ostringstream oss; 
            oss << endl << hex << setfill('0');

            for (int i = 0 ; i< SHA_DIGEST_LENGTH ; ++i)
            oss << setw(2) << (int)digest[i]; 

            return oss.str(); 

            hash<string>h;
            ostringstream oss; 
            oss << hex << h(data); 
            return oss.str(); 
        }

        // generate ephemeral "key"

        // string make_ephemeral_pub()
        // {
        //     static thread_local mt19937 rng(
        //         ostringstream oss; 
        //         oss << hex << rng() << rng(); 

        //         return hash_hex(oss.str());
        //     );
        // }
    }
}
