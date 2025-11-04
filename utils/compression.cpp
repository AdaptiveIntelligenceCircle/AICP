#include <iostream>
using namespace std; 

#include <string> 
#include <vector> 

#include <sstream> 
#include <stdexcept>

#include <cstring> 
#include <iomanip>

#if __has_include(<zlib.h>)
#include <zlib.h> 
#define AICP_USE_ZLIB 1 
#else 
#define AICP_USE_ZLIB 0 
#endif 

#include "checksum.cpp"

namespace aicp :: utils
{
    static const uint64_t AICP_MAGIC = 0x414943505A4C4942ULL;

    // Helper : write magic header ..// 
    static void write_magic(vector<uint8_t> &buf)
    {
        for (int i = 0 ; i < 8 ; ++i)
        buf.push_back(static_cast<uint8_t>((AICP_MAGIC >> (56 - i * 8)) & 0xFF)); 

    }

    // Helper : verify header..// 
    static bool has_magic(const vector<uint8_t> &buf)
    {
        if (buf.size() < 8)
        return false; 

        uint64_t val = 0; 
        for (int i = 0 ; i < 8 ; ++i)
        val = (val << 8) | buf[i]; 
        return val == AICP_MAGIC; 
    }

    // ===== Compress binary data ======= // 
    vector<uint8_t> compress_data(const vector<uint8_t> &input)
    {
        #if AICP_USE_ZLIB
        uLongf compressed_size = compressBound(input.size());
        vector<uint8_t> compressed(8 + compressed_size); // + magic header
        write_magic(compressed);

        int ret = compress2(compressed.data() + 8, &compressed_size,
                            reinterpret_cast<const Bytef *>(input.data()),
                            input.size(), Z_BEST_COMPRESSION);
        if (ret != Z_OK)
            throw runtime_error("zlib compression failed");

        compressed.resize(8 + compressed_size);
        return compressed;
#else
        // fallback: run-length encoding (simple, not efficient)
        vector<uint8_t> out;
        write_magic(out);
        for (size_t i = 0; i < input.size();)
        {
            uint8_t val = input[i];
            size_t count = 1;
            while (i + count < input.size() && input[i + count] == val && count < 255)
                ++count;
            out.push_back(val);
            out.push_back(static_cast<uint8_t>(count));
            i += count;
        }
        return out;
#endif
    }

    vector<uint8_t> decompress_data(const vector<uint8_t> &input)
    {
        if (!has_magic(input))
            throw runtime_error("Invalid compressed format (no magic header)");

#if AICP_USE_ZLIB
        uLongf decompressed_size = input.size() * 10; // guess
        vector<uint8_t> out(decompressed_size);

        int ret = uncompress(reinterpret_cast<Bytef *>(out.data()), &decompressed_size,
                             reinterpret_cast<const Bytef *>(input.data() + 8),
                             input.size() - 8);
        if (ret == Z_BUF_ERROR)
        {
            // retry with larger buffer
            decompressed_size = input.size() * 30;
            out.resize(decompressed_size);
            ret = uncompress(reinterpret_cast<Bytef *>(out.data()), &decompressed_size,
                             reinterpret_cast<const Bytef *>(input.data() + 8),
                             input.size() - 8);
        }

        if (ret != Z_OK)
            throw runtime_error("zlib decompression failed");

        out.resize(decompressed_size);
        return out;
#else
        // fallback RLE decode
        vector<uint8_t> out;
        for (size_t i = 8; i + 1 < input.size(); i += 2)
        {
            uint8_t val = input[i];
            uint8_t count = input[i + 1];
            for (int j = 0; j < count; ++j)
                out.push_back(val);
        }
        return out;
#endif
    }

    vector<uint8_t> compress_string(const string &data)
    {
        return compress_data (vector<uint8_t>(data.begin(), data.end())); 
    }

    string decompress_string(const vector<uint8_t> &compressed)
    {
        auto decompressed = decompress_data(compressed); 
        return string(decompressed.begin(), decompressed.end()); 
    }

    // ======================================= 
    /// Compression Verification 
    /// ======================================

    bool verify_compression(const vector<uint8_t> &original)
    {
        auto c = compress_data(original); 
        auto d = decompress_data(c); 
        return (original == d); 
    }

    // ------ Pretty print compression info ------ // 
    string describe_compression(const vector<uint8_t> &original)
    {
        auto compressed = compress_data(original); 
    }

    string describe_compression(const vector<uint8_t> &original)
    {
        auto compressed = compress_data(original); 
        double ratio = 100.0 *(1.0 - (double)compressed.size() / (double)original.size());
        ostringstream oss; 
        
        oss << "[AICP::Compress] Original: " << original.size()
            << " bytes, Compressed: " << compressed.size()
            << " bytes (" << std::fixed << std::setprecision(2)
            << ratio << "% saved)";
        return oss.str(); 
    }
}