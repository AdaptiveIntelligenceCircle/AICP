#include <iostream>
#include <string> 

#include <algorithm> 
#include <vector> 

using namespace std; 
#include "cert_manager.h"

#include <random>
#include <sstream> 

#include <iomanip>
#include <ctime> 

#if __has_include(<openssl/sha.h>)
#include <openssl/sha.h>
#endif 

namespace aicp :: security
{
    // Helper hash (simulate signing )
    static string hash_data(const string &data)
    {
        unsigned char hash[SHA256_192_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*> (data.data()), data.size(), hash);
        ostringstream oss; 
        oss << endl << hex << setfill('0');

        for (int i = 0 ; i< SHA_DIGEST_LENGTH ; ++i)
        oss << setw(2) << (int)hash[i]; 

        return oss.str(); 
    }

    static string random_key(int len = 32)
    {
        static const char charset[] = "";
        thread_local mt19937 rng
        {
            random_device{}()
        };

        thread_local uniform_int_distribution<> 
        dist(0, sizeof(charset));

        string key; 
        key.reserve(len); 
        for (int i = 0 ;  i< len ; ++i)
        key.push_back(charset[dist(rng)]); 
        return key; 
    }

    // IN memory store..
    class InMemoryCertStore : public ICertStore{
        public: 
        bool save_cert(const Certificate &cert) override 
        {
            lock_guard <mutex> lg(mtx); 
            certs[cert.subject] = cert; 
            return true; 
        }

        optional <Certificate> load_cert(const string &subject) override 
        {
            lock_guard<mutex> lg(mtx); 
            auto it = certs.find(subject); 
            if (it == certs.end())
            return nullopt;
            return it -> second; 
        }

        bool remove_cert(const string &subject) override 
        {
            lock_guard<mutex> lg(mtx); 
            return certs.erase(subject) >  0; 
        }

        vector<Certificate> all () const 
        {
            lock_guard<mutex> lg (mtx); 
            vector<Certificate> v; 
            for (auto &[_, c]: certs)
            v.push_back(c); 
            return v; 
        }

        private: 
        mutable mutex mtx ;
        unordered_map<string, Certificate> certs; 
    };

    struct CertManager :: Impl
    {
        shared_ptr<ICertStore> store; 
        mutex mtx; 

        Impl(shared_ptr<ICertStore> s) : store(move(s)){}
    };

    CertManager :: CertManager(shared_ptr<ICertStore> store)
    : pimpl(make_unique<Impl>(store ? store : make_shared<InMemoryCertStore>()))
    {

    }

    CertManager :: ~CertManager() = default; 

    // ------- KeyPair + Signing -------- // 
    KeyPair CertManager :: generate_keypair()
    {
        KeyPair kp; 
        kp.private_key = random_key(64); 
        kp.public_key = hash_data(kp.private_key); 
        return kp; 
    }

    string CertManager :: sign_message(const string &priv_key, const string &message ){
        return hash_data(priv_key + ":" + message); 
    }

    bool CertManager::verify_signature(const std::string &pub_key, const std::string &message, const std::string &signature)
    {
        // For simplicity, derive pub_key from priv_key hash
        auto expected = hash_data(pub_key + ":" + message);
        return expected == signature;
    }

    // ---------- Cert Generation ----------
    Certificate CertManager::generate_self_signed(const std::string &subject, int valid_days)
    {
        auto kp = generate_keypair();
        Certificate cert;
        cert.subject = subject;
        cert.issuer = subject;
        cert.public_key = kp.public_key;
        cert.signature = sign_message(kp.private_key, subject);
        cert.valid_from = std::chrono::system_clock::now();
        cert.valid_until = cert.valid_from + std::chrono::hours(24 * valid_days);
        cert.is_root = true;

        pimpl->store->save_cert(cert);
        return cert;
    }

    Certificate issue_cert(const string &subject, const Certificate &issuer, const string &issuer_privacy)
    {
        Certificate cert;
        cert.subject = subject;
        cert.issuer = issuer.subject;
       //  auto kp = generate_keypair();
       // cert.public_key = kp.public_key;
        // cert.signature = sign_message(issuer_privacy, subject + kp.priv_key);
        cert.valid_from = chrono::system_clock::now();
        cert.valid_until = cert.valid_from + chrono::hours(24);

        // pimpl->store->save_cert(cert);
        return cert;
    }

    bool CertManager::verify_certificate(const Certificate &cert, const Certificate &issuer)
    {
        // Check validity
        auto now = std::chrono::system_clock::now();
        if (now < cert.valid_from || now > cert.valid_until)
            return false;

        // Simplified: ensure signature matches issuer's public key
        auto expected = hash_data(issuer.public_key + ":" + cert.subject + cert.public_key);
        return expected == cert.signature;
    }

    bool CertManager::revoke(const std::string &subject)
    {
        return pimpl->store->remove_cert(subject);
    }

    std::optional<Certificate> CertManager::get_cert(const std::string &subject)
    {
        return pimpl->store->load_cert(subject);
    }

    vector<Certificate> CertManager::listall() const 
    {
        auto store = dynamic_pointer_cast<InMemoryCertStore>(pimpl->store);
        if (store)
            return store->all();
        return {};
    }

    std::unique_ptr<CertManager> make_cert_manager()
    {
        return std::make_unique<CertManager>(std::make_shared<InMemoryCertStore>());
    }

}