#pragma once 

#include <string> 
#include <vector> 

#include <optional> 
#include <unordered_map>

#include <chrono>
#include <memory> 

using namespace std; 

namespace aicp :: security
{
    using TimePoint  = chrono :: system_clock :: time_point;

    struct Certificate 
    {
        string subject; 
        string issuer; 
        string public_key; 
        string signature ; 
        TimePoint valid_from; 
        TimePoint valid_until; 

        bool is_root = false;
    };

    struct KeyPair
    {
        string public_key; 
        string private_key; 
    }; 

    // interface for cert storage backend .. // 
    class ICertStore
    {
        public: 
        virtual bool save_cert(const Certificate &cert) = 0; 
        virtual optional <Certificate> load_cert(const string &subject); 
        virtual bool remove_cert(const string &object) = 0; 
        virtual ~ICertStore() = default; 
    }; 

    class CertManager
    {
        public: 
        explicit CertManager(shared_ptr<ICertStore> store = nullptr);

        // generate self - signed certificate for local node
        Certificate generate_self_signed(const string &subject, int validate);

        // issue new certificate for subject using issuer's key..// 
        Certificate issue_cert(const string &subject, const Certificate &issuer, const string &issuer_privacy);

        // verify a cert signature and validity period..
        bool verify_certificate(const Certificate &cert, const Certificate &issue);

        // Revoke certificate (mark invalid).. 
        bool revoke(const string &subject); 

        // load from store.. // 
        optional<Certificate> get_cert(const string &subject); 

        // list all known certs..
        vector<Certificate> listall() const; 

        // keypair utility.. // 
        static KeyPair generate_keypair(); 
        static string sign_message(const string &priv_key, const string &); 
        static bool verify_signature(const std::string &pub_key, const std::string &message, const std::string &signature); 

        ~CertManager(); 

        private: 
        struct Impl;
        unique_ptr<Impl> pimpl; 
    };

    // factory for in-memory cert manager.. // 
    unique_ptr<CertManager> make_cert_manager(); 
}