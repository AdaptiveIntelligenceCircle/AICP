#include <iostream>
#include <string> 
using namespace std; 

#include <vector> 
#include <algorithm>
#include <cmath> 

#include <unordered_map>
#include <mutex> 
#include <chrono>

#include <sstream>
#include <fstream>
#include <thread>

#include "auth_manager.hpp"

namespace aicp :: security
{
    using Clock = chrono :: steady_clock; 
    using namespace chrono_literals; 

    enum class ThreatLevel 
    {   
        Low, Medium,High, Critical
    };

    struct DefenseEvent 
    {
        string source ; 
        string description; 
        ThreatLevel level;
        Clock :: time_point timestamp; 
    }; 

    class SelfDefenseGuard
    {
        public: 
        explicit SelfDefenseGuard(shared_ptr<AuthManager> auth)
        : auth_manager(move(auth))
        {
            monitoring_thread = thread([this]()
        {
            
        });
        }

        ~SelfDefenseGuard()
        {
            running = false;
            if (monitoring_thread.joinable())
            monitoring_thread.join(); 
        }

        // record suspiciuos activity..
        void report_event(const string &source, const string &desc, ThreatLevel level )
        {
            lock_guard <mutex> lock(mtx); 
            DefenseEvent ev
            {
                source, desc, level, Clock :: now()
            };
            event_log.push_back(ev); 
            handle_event(ev); 
        }

        void expert_log(const string &path)
        {
            lock_guard<mutex> lock(mtx);
            ofstream out(path, ios::app);
            for (auto &ev : event_log)
            {
                out << "[" << to_string(ev.level) << "] "
                    << ev.source << " - " << ev.description << "\n";
            }
        }

        // adaptive reaction based on patterns
        void handle_event(const DefenseEvent &ev)
        {
            string level_str = to_string(ev.level);

            cout << "[SelfDefense] Event detected: " << level_str
                 << " | Source: " << ev.source
                 << " | Desc: " << ev.description << "\n";

            switch (ev.level)
            {
            case ThreatLevel::Low:
                // just log + monitor
                break;
            case ThreatLevel::Medium:
                restrict_access(ev.source);
                break;
            case ThreatLevel::High:
                isolate_module(ev.source);
                break;
            case ThreatLevel::Critical:
                terminate_process(ev.source);
                break;
            }
        }

        // periodic monitor (check for repeated events from same source)..
        void monitor_loop()
        {
            while (running)
            {
                this_thread::sleep_for(2s);
                lock_guard<mutex> lock(mtx);

                auto now = Clock::now();
                for (auto &[src, count] : violation_count)
                {
                    if (count > 3)
                    {
                        DefenseEvent ev{src, "Repeated anomaly within time window", ThreatLevel::High, now};
                        handle_event(ev);
                        count = 0; // reset after reaction
                    }
                }
            }
        }

        private:
        mutex mtx; 
        vector<DefenseEvent> event_log;
        unordered_map<string, int> violation_count; 
        shared_ptr<AuthManager> auth_manager; 

        thread monitoring_thread; 
        bool running = true; 

        // --- reactions ---
        void restrict_access(const string &src)
        {
            violation_count[src]++;
            cerr << "[SelfDefense] Restricting access for " << src << "\n";
        }

        void isolate_module(const string &src)
        {
            violation_count[src]++;
            cerr << "[SelfDefense] Isolating module: " << src << "\n";
            // in future: signal to plugin manager to unload or sandbox module
        }

        void terminate_process(const string &src)
        {
            cerr << "[SelfDefense] CRITICAL THREAT from " << src << " - initiating system quarantine!\n";
            // optionally encrypt and dump memory for forensic
            terminate();
        }

        static string to_string(ThreatLevel level)
        {
            switch (level)
            {
            case ThreatLevel::Low:
                return "LOW";
            case ThreatLevel::Medium:
                return "MEDIUM";
            case ThreatLevel::High:
                return "HIGH";
            case ThreatLevel::Critical:
                return "CRITICAL";
            default:
                return "UNKNOWN";
            }
        }
    }; 

    unique_ptr<SelfDefenseGuard> make_self_defense_guard(shared_ptr<AuthManager> auth)
    {
        return make_unique<SelfDefenseGuard>(move(auth));
    }
    
}