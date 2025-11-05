#include "SyncManager.hpp"

using namespace std; 

namespace aicp 
{
    bool SyncManager :: merge_state(const string &key, const SyncState &remote)
    {
        lock_guard<mutex> lock(mu_);
        auto it = store_.find(key); 

        if (it == store_.end()){
            // No local entry -> accept remote..
            store_.emplace(key, remote); 
            return true; 
        }

        SyncState &local = it -> second; 

        // prefer higher version .. ..
        if (remote.version > local.version){
            local = remote; 
            return true; 
        }

        // if same version, use deterministic tie-breaker on origin
        if (remote.version == local.version)
        {
            // if origin id is non - empty and differs, choose the .
            // this is arbitary but deterministic ; adjust policy.
            if (remote.origin_id != local.origin_id and remote.origin_id > local.origin_id){
                local = remote; 
                return true; 
            }
            // if origin_id equal, we can optionally compare blob.. 
        }

        // remote is older or not preferred -> no update.. // 
        return false; 
    }

    optional <SyncState> SyncManager :: get_state(const string &key)
    {
        lock_guard<mutex> lock(mu_); 
        auto it = store_.find(key); 
        if (it == store_.end())
        return nullopt; 
        return it -> second; // return a copy..
    }

    void SyncManager :: local_update(const string &key, const vector<uint8_t> &blob)
    {
        lock_guard<mutex> lock(mu_); 
        auto it = store_.find(key); 

        if (it == store_.end()){
            // create new entry : start version at 1 
            SyncState s; 
            s.version = 1; 
            s.origin_id = "local"; 
            s.blob = blob; 
            store_.emplace(key, move(s)); 
            return; 
        }

        // update existing entry : increment version and replace blob..
        SyncState &local = it -> second; 

        // avoid zero -> increment (handles initial 0)
        local.version = local.version + 1; 
        local.blob = blob; 

        // mark origin as local(override if previously remote).. 
        local.origin_id = "local"; 
    }
} // namespace aicp .. 