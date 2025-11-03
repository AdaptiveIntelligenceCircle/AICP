#pragma once 
#include <string> 
#include <vector> 

#include <fstream> 
#include <mutex> 

#include <chrono> 
#include <memory> 

#include <optional> 

using namespace std; 

namespace aicp :: security 
{
    using TimePoint = chrono :: system_clock::time_point; 

    enum class IntrusionSeverity
    {
        Info, 
        Warning, 
        Alert,
        Critical, 

    };

    struct IntrusionRecord 
    {
        string source; 
        string description; 
        IntrusionSeverity severity; 
        TimePoint timestamp; 
        optional <string> session_id; 
    }; 

    class IntrusionLog
    {
        public:
        explicit IntrusionLog(const string &log_path = "intrusion.log"); 

        // append new record to memory and file .. 
        void record(const IntrusionRecord &rec); 

        // convenience overload.. /// 
        void record(const string &src, const string &desc, 
        IntrusionSeverity sev, const optional<string> &session_id = nullopt );

        // export current log buffer to file 
        vector<IntrusionRecord> recent() const; 

        // rotate log file (archived )
        bool rotate(); 

        ~IntrusionLog(); 

        private: 
        string log_path; 
        mutable mutex mtx; 
        vector<IntrusionRecord> buffer; 

        string severity_to_string(IntrusionSeverity sev) const; 
        string timestamp_to_string(TimePoint t) const; 
    };
    
    // Factory function.. // 
    unique_ptr<IntrusionLog> make_intrusion_log(const string &path = "intrusion.log");
     
}