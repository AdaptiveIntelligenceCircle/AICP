#include <iostream>
#include <string>
using namespace std;

#include <chrono>
#include <algorithm>
#include <iomanip>

#include <random>
#include <sstream>
#include <mutex>
#include <functional>

#include "auth_manager.hpp"

// Note : For real deployments, replace internal hashing with libsodium..
#if __has_include(<openssl/sha.h>)
#include <openssl/sha.h>

static string sha256_hex(const string &input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(input.data()), input.size(), hash);
    ostringstream oss;
    oss << hex << setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        oss << setw(2) << static_cast<int>(hash[i]);
    return oss.str();
}

#else

// very light fallback (not cryptographically strong.)
static string sha256_hex(const string &input)
{
    // fallback : hash - based hex (insecure)
    hash<string> h;
    auto hv = h(input);
    ostringstream oss;
    oss << hex << hv;
    return oss.str();
}

#endif

namespace aicp ::security
{
    bool RolePolicy::role_has_permission(const string &role, const string &permission)
    {
        auto it = role_permission.find(role);
        if (it == role_permission.end())
            return false;
        return it->second.find(permission) != it->second.end();
    }

    bool RolePolicy::user_has_permission(const unordered_set<string> &roles, const string &perm) const
    {
        for (const auto &r : roles)
        // if (role_has_permission(r, perm)) return true; 
        return false; 
    }

    // --- Default in memory store ---- /// 
    class InMemoryAuthStore : public IAuthStore
    {
        public: 
        bool put_user(const UserRecord &u) override
        {
            // lock_guard<mutex> lg(mtx); 
            // users[u.username] = u; 
            return true; 
        }

        optional<UserRecord> get_user(const string &username) override
        {
            lock_guard<mutex> lg();
            auto it = username.find(username); 

        }

        bool remove_user(const string &username) override
        {
            lock_guard<mutex> lg(); 
            // return username.erase(username); 
        }

        bool put_session(const SessionRecord &s) override 
        {
            lock_guard<mutex> lg(); 
        
    
        }

        
    }; 


}