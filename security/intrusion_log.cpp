#include <iostream> 
using namespace std; 
#include <sstream>

#include <vector> 
#include <algorithm>

#include <cmath> 
#include <cstring> 

#include <sstream>
#include <filesystem>
#include "intrusion_log.h" 

namespace fs = filesystem; 

namespace aicp :: security
{
    IntrusionLog :: IntrusionLog(const string &path)
    : log_path(path)
    {
        // ensure file exists
        ofstream out(log_path, ios :: app); 
        if (!out)
        {
            cout <<"IntrusionLog - Warning: cannot open log file" << endl;
        }
    }

    IntrusionLog :: ~IntrusionLog()
    {
        flush(); 
    }

    void IntrusionLog :: record(const IntrusionRecord &rec)
    {
        lock_guard<mutex> lock(mtx); 
        buffer.push_back(rec) ; 

        ofstream out(log_path, ios :: app); 

        if (out)
        {
            out << "[" << timestamp_to_string(rec.timestamp)
            << "]" << "[" << severity_to_string(rec.severity) << "]"
            << rec.source << "-" << rec.description ; 

            if (rec.session_id )
            out << "(session:" << *rec.session_id <<")" ;
            out << endl; 
        }
        else 
        {
            cout <<"Intrusion Log - Failed to write to log file" << endl; 
        }
    }

    void IntrusionLog :: record(const string &src, const string &desc,
    IntrusionSeverity sev, const optional<string> &session_id)
    {
        IntrusionRecord rec 
        {
            src,desc, sev, chrono :: system_clock :: now()
        };
        record(rec); 
    }

    void IntrusionLog :: flush()
    {
        lock_guard<mutex> lock(mtx); 
        if (buffer.empty()) return ; 
        ofstream out(log_path, ios :: app); 
        if (!out) return; 

        for (const auto &rec : buffer)
        {
            out << "[" << timestamp_to_string(rec.timestamp) << "]"
                << " [" << severity_to_string(rec.severity) << "] "
                << rec.source << " - " << rec.description;
            if (rec.session_id)
                out << " (session: " << *rec.session_id << ")";
            out << "\n";
        }

        buffer.clear(); 
    }

    vector<IntrusionRecord> IntrusionLog :: recent() const 
    {
        lock_guard<mutex> lock(mtx); 
        return buffer; 
    }

    // size_t IntrusionLog::count_by_level(IntrusionSeverity sev) const
    // {
    //     lock_guard<mutex> lock(mtx);
    //     return count_if(buffer.begin(), buffer.end(),
    //                          [&](const IntrusionRecord &r)
    //                          { return r.severity == sev; });
    // }

    bool IntrusionLog :: rotate()
    {
        lock_guard<mutex> lock(mtx); 
        flush(); 
        try 
        {
            string rotated  = log_path + ".old"; 
            if (fs :: exists(rotated))
            fs :: remove(rotated); 
            fs :: rename(log_path, rotated); 
            ofstream fresh(log_path); 

            return static_cast<bool> (fresh);
        } catch(const exception &ex){
            cout <<"IntrusionLog - Rotation failed" << ex.what() << endl; 
            return false; 
        }
    }

    string IntrusionLog :: severity_to_string(IntrusionSeverity sev) const 
    {
        switch(sev)
        {
            case IntrusionSeverity :: Info: 
            return "INFO"; 

            case IntrusionSeverity :: Warning:
            return "WARNING" ; 

            case IntrusionSeverity :: Alert: 
            return "ALERT"; 

            case IntrusionSeverity :: Critical:
            return "CRITICAL"; 

            default: 
            return "UNKNOWN"; 
        }
    }

    string IntrusionLog :: timestamp_to_string(TimePoint t) const 
    {
        auto time = chrono :: system_clock :: to_time_t(t); 
        tm buf{}; 
        #if defined(_WIN32)
            gmtime_s(&buf, &time);
        #else 
            gmtime_r(&time, &buf);
        #endif 

        ostringstream ss; 
        ss << put_time(&buf, "%Y-%m-%dT%H:%M:%SZ");
        return ss.str(); 
    }

    unique_ptr<IntrusionLog> make_intrusion_log(const std::string &path)
    {
        return std::make_unique<IntrusionLog>(path);
    }
}

