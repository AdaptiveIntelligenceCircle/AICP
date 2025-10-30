#pragma once 
#include <string> 

#include <unordered_map>
#include <mutex> 

#include <optional>
using namespace std; 

#include<vector>

namespace aicp 
{
    struct SyncState
    {
        uint64_t version = 0; 
        string origin_id; 
        vector<uint8_t> blob; 
    }; 

    class SyncManager
    {
        public: 
        // merge incoming state, returns true if local state updated,,.. 
        bool merge_state(const string &key, const SyncState &remote);

        // get current state copy.. 
        optional<SyncState> get_state(const string &key);
        
        // produce local update.. 
        void local_update(const string &key, const vector<uint8_t> &blob ); 

        private: 
        unordered_map<string, SyncState> store_; 
        mutex mu_; 
    }; 
}