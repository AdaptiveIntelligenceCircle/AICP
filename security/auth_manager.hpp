#pragma once 
#include <string> 
#include <vector> 

#include <unordered_map>
#include <unordered_set> 
#include <optional> 

#include <chrono> 
#include <cstdint> 
#include <memory> 

using namespace std; 

namespace aicp :: security
{
    using TimePoint = chrono :: system_clock :: time_point; 

    struct UserRecord
    {
        string username; 
        string password_hash; 
        string salt; 
        unordered_set<string> roles;
        TimePoint created_at; 
    };

    struct SessionRecord
    {
        string token; 
        string username; 
        TimePoint expires_at; 
        uint64_t seq =0; // sequencing for session events..// 
    };
    
    // storage backend interface - to implement DB/file/remote share.. . 
    class IAuthStore
    {
        public:
        virtual bool put_user(const UserRecord &u) = 0; 
        virtual optional<UserRecord> get_user(const string &username) = 0; 
        virtual bool remove_user(const string &username) = 0; 

        virtual bool put_session(const SessionRecord &s) = 0; 
        virtual optional<SessionRecord> get_session(const string &token); 
        virtual bool remove_session(const string &token) = 0;
        
        virtual ~IAuthStore() = default; 
    };

    // Simple role - persmission mapping (could be exrernalized)
    struct RolePolicy
    {
        // role -> permissions .. / / 
        unordered_map<string, unordered_set<string>> role_permission;
        
        bool role_has_permission(const string &role, const string &permission); 
        bool user_has_permission(const unordered_set<string> &roles, const string &perm ) const; 

    };

    // Main Authmanager .. 
    class AuthManager 
    {
        public: 
        // constructor takes ownership (or shared_ptr) of store.. 
        explicit AuthManager (shared_ptr<IAuthStore> store); 

        // user lifecycle .. 
        bool register_user(const string &username, const string &password, const vector<string> &roles = {}); 
        bool delete_user(const string &username); 
        // authentication -> returns session token if ok.. 
        optional<string> authenticate(const string &username, const string &password,
        chrono:: seconds session_ttl = chrono :: hours(1) );

        // return the username if validate
        optional<string> validate_token(const string &token) ;

        // revoke a session token..
        bool revoke_token(const string &token); 
        
        // role / permission management.... 
        bool add_role_to_user(const string &username, const string &role); 
        bool remove_rule_from_user(const string &username, const string &role); 
        bool check_permission(const string &username, const string &permission);
        bool remove_role_from_user(const string &username, const string &role); 
        // set / update role policy .. 
        void set_rule_policy(const RolePolicy &policy); 

        // Convienience: salt + hash functions 
        static string generate_salt(size_t n = 16); 
        static string hash(const string &password, const string &salt );
        
        ~AuthManager(); 

        private: 
        struct Impl; 
        unique_ptr<Impl> pimpl; 
    };
}   // namespace aicp :: security.. // 