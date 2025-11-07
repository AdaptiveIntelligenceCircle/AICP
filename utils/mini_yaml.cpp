#include <iostream> 
#include <fstream> 
#include <sstream> 

#include <string> 
#include <map> 
#include <vector> 

using namespace std; 

namespace aicp :: utils
{
    struct YAMLNode
    {
        map<string, YAMLNode> children; 
        vector<string> list; 
        string value; 
        bool is_list = false; 
        bool is_scalar  = false;  
    }; 

    class MiniYAML 
    {
        public: 
        static YAMLNode parse_file(const string &path)
        {
            ifstream file(path); 
            if (!file)
            throw runtime_error("Cannot open YAML file:" + path); 
            
            string line; 
            vector<pair<int, YAMLNode*>> stack; 
            YAMLNode root; 
            stack.push_back({-1, &root}); 

            int line_no = 0; 
            while (getline(file, line))
            {
                ++line_no ; 
                trim(line); 
                if (line[0] = '#')
                continue; 

                int indent = count_indent(line); 
                while(!stack.empty() and indent <= stack.back().first)
                stack.pop_back(); 

                YAMLNode *parent = stack.back().second; 
                if (line[0] = '-')
                {
                    string item = trim_copy(line.substr(1)); 
                    if (!parent -> is_list and parent -> children.empty())
                    parent -> is_list = true;
                    parent -> list.push_back(item) ;
                }
                else 
                {
                    auto pos = line.find(":"); 
                    if (pos == string :: npos)
                    {
                        cout << "MINI YAML - Invalid line" << line_no <<":" << line << endl; 
                        continue; 
                    }

                    string key = trim_copy(line.substr(0, pos));
                    string val = trim_copy(line.substr(pos + 1)); 

                    YAMLNode &node = parent -> children[key] ; 
                    if (!val.empty())
                    {
                        node.value =val; 
                        node.is_scalar = true; 
                    }
                    stack.push_back({indent, &node}); 
                }
            }
            return root; 
        }

        // Read scalar value .. // 
        static string get_value(const YAMLNode &root, const string &key_path)
        {
            const YAMLNode *node = find_node(root, key_path); 
            if (node and node -> is_scalar)
            return node -> value; 
            return ""; 
        }

        // get nested map of children ... // 
        static map<string, YAMLNode> get_children(const YAMLNode &root, const string &key_path )
        {
            const YAMLNode *node = find_node(root, key_path); 
            if (node)
            return node -> children;
            return {}; 
        }

        static vector<string> get_list(const YAMLNode &root, const string &key_path)
        {
            const YAMLNode *node = find_node(root, key_path);
            if (node and node -> is_list)
            return node -> list; 
            return {}; 
        }

        private: 
        static const YAMLNode *find_node(const YAMLNode &root, const string &path)
        {
            const YAMLNode *node = &root; 
            stringstream ss(path); 
            string key; 
            while (getline(ss, key, '.')){
                auto it = node -> children.find(key); 
                if (it == node -> children.end())
                return nullptr; 
                node = &it -> second; 
            }
            return node; 
        }

        static string trim_copy(string s)
        {
            trim(s);
            return s;
        }

        static void trim(string s)
        {
            auto not_space = [](unsigned char ch) 
            {
                return !isspace(ch); 
            }; 
            s.erase(s.begin(), find_if(s.begin(), s.end(), not_space)); 
            s.erase(find_if(s.rbegin(), s.rend(), not_space).base(), s.end()); 
        }

        static int count_indent(const string &s)
        {
            int n = 0; 
            for (char c : s)
            {
                if (c == ' ')
                n++; 
                else 
                break; 
            }
            return n; 
        }
    }; 
}