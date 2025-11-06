#include <iostream> 
#include <fstream> 
#include <iomanip> 

#include <filesystem>
#include <string> 
#include <vector> 

#include <unordered_map>
#include <stdexcept>

#include <yaml.h>
using namespace std; 

#include "../utils/checksum.cpp"
#include "../security/intrusion_log.h"

namespace fs = filesystem; 

namespace aicp :: core 
{
    using namespace aicp :: utils;
    using namespace aicp :: security; 

    struct ModuleInfo
    {
        string name; 
        string version; 
        string checksum; 
        vector<string> dependencies; 
    }; 

    class ManifestVerifier
    {
    public:
        explicit ManifestVerifier(const string &manifest_path,
                                  shared_ptr<IntrusionLog> log = nullptr)
            : manifest_path(manifest_path), intrusion_log(std::move(log)) {}

        bool verify_all()
        {
            cout << "🔍 Verifying manifest integrity from: " << manifest_path << "\n";

            auto modules = load_manifest();
            bool all_ok = true;

            for (const auto &mod : modules)
            {
                string path = locate_module_file(mod.name);
                if (path.empty())
                {
                    report_event("manifest_verifier", "Module not found: " + mod.name,
                                 IntrusionSeverity::Warning);
                    all_ok = false;
                    continue;
                }

                string actual_hash = compute_file_sha256(path);
                bool match = (mod.checksum.rfind(actual_hash, 0) == 0 ||  // starts with (prefix match)
                              actual_hash == mod.checksum);

                cout << "  → " << left << setw(16) << mod.name
                          << " v" << mod.version
                          << "  hash: " << actual_hash.substr(0, 12)
                          << (match ? " OK" : "  MISMATCH") << "\n";

                if (!match)
                {
                    all_ok = false;
                    report_event("manifest_verifier",
                                 "Checksum mismatch for module " + mod.name,
                                 IntrusionSeverity::Alert);
                }
            }

            if (all_ok)
            {
                cout << "\nAll modules verified successfully.\n";
            }
            else
            {
                cout << "\n Integrity issues detected. Review intrusion log.\n";
            }
            return all_ok;
        }

        // Quick check for one module
        bool verify_single(const string &module_name)
        {
            auto modules = load_manifest();
            auto it = find_if(modules.begin(), modules.end(),
                                   [&](const ModuleInfo &m) { return m.name == module_name; });

            if (it == modules.end())
            {
                cerr << "Module not found in manifest: " << module_name << "\n";
                return false;
            }

            string path = locate_module_file(it->name);
            if (path.empty())
            {
                cerr << "Cannot locate module file for " << it->name << "\n";
                return false;
            }

            string actual = compute_file_sha256(path);
            bool ok = (actual == it->checksum);

            cout << "Module: " << it->name << " (" << it->version << ")\n";
            cout << "Expected: " << it->checksum.substr(0, 12)
                      << " | Actual: " << actual.substr(0, 12)
                      << (ok ? " Match\n" : " Mismatch\n");

            if (!ok)
                report_event("manifest_verifier",
                             "Checksum mismatch in " + it->name,
                             IntrusionSeverity::Alert);
            return ok;
        }

    private:
        string manifest_path;
        shared_ptr<IntrusionLog> intrusion_log;

        vector<ModuleInfo> load_manifest()
        {
            YAML::Node manifest = YAML::LoadFile(manifest_path);
            vector<ModuleInfo> mods;

            if (!manifest["modules"])
                throw runtime_error("Invalid manifest format: no modules field");

            for (auto it : manifest["modules"])
            {
                ModuleInfo m;
                m.name = it.first.as<string>();
                m.version = it.second["version"].as<string>();
                m.checksum = it.second["checksum"].as<string>();

                if (it.second["dependencies"])
                {
                    for (auto dep : it.second["dependencies"])
                        m.dependencies.push_back(dep.as<string>());
                }
                mods.push_back(std::move(m));
            }
            return mods;
        }

        string locate_module_file(const string &mod_name)
        {
            vector<string> possible = {
                "core/" + mod_name + ".cpp",
                "modules/" + mod_name + ".cpp",
                "security/" + mod_name + ".cpp",
                "utils/" + mod_name + ".cpp",
                "runtime/" + mod_name + ".cpp"};
            for (const auto &p : possible)
                if (fs::exists(p))
                    return p;
            return {};
        }

        string compute_file_sha256(const string &path)
        {
            ifstream file(path, ios::binary);
            if (!file)
                throw runtime_error("Cannot open " + path);

            ostringstream oss;
            oss << file.rdbuf();
            string data = oss.str();
            return sha256(data);
        }

        void report_event(const string &src, const string &msg, IntrusionSeverity sev)
        {
            if (intrusion_log)
                intrusion_log->record(src, msg, sev);
            else
                cerr << "[ManifestVerifier][" << src << "] " << msg << "\n";
        }
    };
}