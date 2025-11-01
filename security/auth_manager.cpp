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
            lock_guard<mutex> lg(mtx); 
            users[u.username] = u; 
            return true; 
        }

        optional<UserRecord> get_user(const string &username) override
        {
            lock_guard<mutex> lg();
            auto it = users.find(username); 
            if (it == users.end()) return nullopt;
            return it -> second; 
        }

        bool remove_user(const string &username) override
        {
            lock_guard<mutex> lg(); 
            return users.erase(username) > 0 ; 
        }

        bool put_session(const SessionRecord &s) override 
        {
            lock_guard<mutex> lg(mtx); 
            sessions[s.token] = s; 
            return true; 
        }

        optional <SessionRecord> get_session(const string &token) override{
            lock_guard<mutex> lg(mtx); 
            auto it = sessions.find(token) ;
            if (it == sessions.end()) return nullopt;
            return it -> second; 
        }

        bool remove_session(const string &token) override 
        {
            lock_guard<mutex> lg(mtx); 
            return sessions.erase(token) > 0; 
        }

        private: 
        mutex mtx ;
        unordered_map<string, UserRecord> users; 
        unordered_map<string, SessionRecord> sessions; 
        
    }; 

    // ----- Auth Manager implementation (pimpl)-----// 
    struct AuthManager :: Impl 
    {
        shared_ptr<IAuthStore> store; 
        RolePolicy policy; 
        mutex mut;
        mt19937_64 rng {random_device{}()}; 
        uint64_t token_seq = 0; 

        Impl(shared_ptr<IAuthStore> s): store(move(s)){}
    };

    AuthManager :: AuthManager (shared_ptr<IAuthStore> store)
    :pimpl(make_unique<Impl> (store ? store : std::make_shared<InMemoryAuthStore>()))
    {

    }

    AuthManager :: ~AuthManager () = default;

    // utils --- // 
    string AuthManager :: generate_salt(size_t n)
    {
        static const char charset[] = ""; 
        thread_local mt19937 rg{
            random_device{}()
        }; 
        string s; 
        s.reserve(n); 

        for (size_t i = 0 ; i < n ; ++i) 
        // s.push_back(charset[dist(rg)]);
        return s ;  
    }

    string AuthManager :: hash(const string &password, const string &salt)
    {
        return sha256_hex(salt + ":" + password); 
    }

    // -- Register / delete --- //

    bool AuthManager :: register_user(const string &username, const string &password, 
    const vector<string> &roles)
    {
        if (username.empty() or password.empty()) return false; 

        auto existing = pimpl -> store -> get_user(username); 
        if (existing.has_value()) return false; // already exists.. 

        UserRecord u;
        u.username = username; 
        u.salt = generate_salt(16); 
        u.password_hash = hash(password, u.salt);
        u.created_at  = chrono :: system_clock :: now();
        for (auto & r : roles) u.roles.insert(r); 
        return pimpl -> store -> put_user(u); 
    }

    bool AuthManager :: delete_user(const string &username)
    {
        return pimpl -> store-> remove_user(username); 
    }

    // -- authentication / sessions -- // 
    static string gen_token_from_rng(mt19937_64 &rng, uint64_t seq){
        ostringstream oss;
        oss << hex << rng() << "-" << seq ; 
        return oss.str(); 
    }

    optional <string> AuthManager :: authenticate(const string &username,
    const std::string &password, std::chrono::seconds session_ttl)
    {
        auto uopt = pimpl -> store -> get_user(username); 
        if (!uopt.has_value()) return std::nullopt;

        auto u = *uopt;
        auto provided_hash = hash(password, u.salt);
        if (provided_hash != u.password_hash) return std::nullopt;

        // create session
        std::lock_guard<std::mutex> lg(pimpl->mut);
        auto token = gen_token_from_rng(pimpl->rng, ++pimpl->token_seq);
        SessionRecord s;
        s.token = token;
        s.username = username;
        s.expires_at = std::chrono::system_clock::now() + session_ttl;
        s.seq = pimpl->token_seq;

        if (!pimpl->store->put_session(s)) return std::nullopt;
        return token;
    }

    optional<string> AuthManager :: validate_token(const string &token)
    {
        auto sopt = pimpl -> store -> get_session(token); 
        if (!sopt.has_value()) return nullopt; 
        auto s = *sopt; 
        auto now = chrono :: system_clock :: now(); 
        if (now > s.expires_at){
            // expired -> remove 
            pimpl -> store -> remove_session(token); 
            return nullopt; 
        }
        return s.username; 
    }

    bool AuthManager :: revoke_token(const string &token)
    {
        return pimpl -> store -> remove_session(token); 
    }

    // -- Roles and permission -- // 
    bool AuthManager :: add_role_to_user(const string &username, const string &role)
    {
        auto uopt = pimpl -> store -> get_user(username); 
        if (!uopt.has_value()) return false; 
        auto u = *uopt ; 
        u.roles.insert(role); 
        return pimpl -> store -> put_user(u); 
    }

    bool AuthManager :: remove_role_from_user(const string &username, const string &role)
    {
        auto uopt = pimpl -> store -> get_user(username); 
        if (!uopt.has_value()) return false; 
        auto u = *uopt; 
        u.roles.erase(role); 
        return pimpl -> store -> put_user(u); 
    }

    bool AuthManager :: check_permission(const string &username, const string &permission)
    {
        auto uopt = pimpl -> store -> get_user(username); 
        if (!uopt.has_value()) return false; 
        return pimpl -> policy.user_has_permission(uopt -> roles, permission);
    }

    void AuthManager :: set_rule_policy(const RolePolicy &policy)
    {
        lock_guard<mutex> lg(pimpl-> mut); 
        pimpl -> policy= policy;
    }

}