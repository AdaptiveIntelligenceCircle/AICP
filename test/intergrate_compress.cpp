#include "../utils/compression.cpp"
#include <iostream>

using namespace aicp::utils;

int main()
{
    string text = R"({"protocol":"auth","message":"This is a long packet example for compression"})";

    auto compressed = compress_string(text);
    auto restored = decompress_string(compressed);

    cout << describe_compression(vector<uint8_t>(text.begin(), text.end())) << "\n";
    cout << "Restored: " << restored << "\n";

    if (verify_compression(std::vector<uint8_t>(text.begin(), text.end())))
        cout << "✅ Compression verified successfully.\n";
    else
        cout << "❌ Compression mismatch.\n";
    return 0; 
}
