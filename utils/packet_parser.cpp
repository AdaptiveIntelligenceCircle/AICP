#include <iostream> 
using namespace std; 
#include <sstream> 
#include <iomanip>

#include <vector>
#include <stdexcept>
#include <cstdint>
#include <optional>

#include "../utils/checksum.cpp"
#include "../utils/serializer.cpp"
#include "../security/intrusion_log.h"
#include "../core/message_broker.hpp"

namespace aicp :: core 
{
    using namespace aicp; 
    using namespace aicp :: utils; 
    using namespace aicp :: security; 

    struct PacketHeader
    {
        uint32_t magic = 0xA1C9F00D;
        uint16_t version = 1; 
        uint16_t header_length = 0; 
        uint32_t payload_length = 0; 
        uint32_t checksum = 0; 
    }; 

    class PacketPerser
    {
        public: 
        // Parse raw bytes into a MessageEnvelope (throws if invalid)
        optional<MessageEnvelope> parse(const vector<uint8_t> &raw)
        {
            if (raw.size() < sizeof(PacketHeader))
            {
                log_event("core/packet_parser", "Truncated packet received", IntrusionSeverity::Warning);
                return nullopt;
            }

            PacketHeader hdr = read_header(raw);
            if (hdr.magic != 0xA1C9F00D)
            {
                log_event("core/packet_parser", "Invalid magic number in packet header", IntrusionSeverity::Alert);
                return nullopt;
            }

            if (hdr.payload_length + hdr.header_length + sizeof(PacketHeader) > raw.size())
            {
                log_event("core/packet_parser", "Payload length mismatch — possible truncation or tampering", IntrusionSeverity::Alert);
                return nullopt;
            }

            // extract payload
            auto start = raw.begin() + sizeof(PacketHeader);
            auto end = start + hdr.payload_length;
            vector<uint8_t> payload(start, end);

            // verify checksum
            uint32_t computed_crc = aicp::utils::crc32(payload);
            if (computed_crc != hdr.checksum)
            {
                log_event("core/packet_parser", "Checksum mismatch — corrupted or forged packet", IntrusionSeverity::Critical);
                return nullopt;
            }

            // decode JSON envelope
            string json_str(payload.begin(), payload.end());
            try
            {
                MessageEnvelope env = aicp::utils::desrialize_envelope(json_str);
                return env;
            }
            catch (const exception &e)
            {
                log_event("core/packet_parser", std::string("JSON parsing error: ") + e.what(), IntrusionSeverity::Alert);
                return nullopt;
            }
        }

        // Build packet from MessageEnvelope (for outbound)
        vector<uint8_t> build(const MessageEnvelope &env)
        {
            string json = aicp::utils::serialize_envelope(env);
            vector<uint8_t> payload(json.begin(), json.end());

            PacketHeader hdr;
            hdr.header_length = 0;
            hdr.payload_length = static_cast<uint32_t>(payload.size());
            hdr.checksum = aicp::utils::crc32(payload);

            vector<uint8_t> packet;
            append_header(packet, hdr);
            packet.insert(packet.end(), payload.begin(), payload.end());
            return packet;
        }
        private:
        shared_ptr<IntrusionLog> intrusion_log;

        PacketHeader read_header(const vector<uint8_t> &raw)
        {
            PacketHeader hdr{};
            size_t offset = 0;
            memcpy(&hdr.magic, raw.data() + offset, sizeof(hdr.magic));
            offset += sizeof(hdr.magic);
            memcpy(&hdr.version, raw.data() + offset, sizeof(hdr.version));
            offset += sizeof(hdr.version);
            memcpy(&hdr.header_length, raw.data() + offset, sizeof(hdr.header_length));
            offset += sizeof(hdr.header_length);
            memcpy(&hdr.payload_length, raw.data() + offset, sizeof(hdr.payload_length));
            offset += sizeof(hdr.payload_length);
            memcpy(&hdr.checksum, raw.data() + offset, sizeof(hdr.checksum));
            return hdr;
        }

        void append_header(vector<uint8_t> &buf, const PacketHeader &hdr)
        {
            auto append = [&](auto val) {
                const uint8_t *p = reinterpret_cast<const uint8_t *>(&val);
                buf.insert(buf.end(), p, p + sizeof(val));
            };
            append(hdr.magic);
            append(hdr.version);
            append(hdr.header_length);
            append(hdr.payload_length);
            append(hdr.checksum);
        }

        void log_event(const string &src, const string &desc, IntrusionSeverity sev)
        {
            if (intrusion_log)
                intrusion_log->record(src, desc, sev);
            else
                cerr << "[PacketParser][" << src << "] " << desc << "\n";
        }
    }; 

    // Factory : 
    unique_ptr<PacketPerser>make_packet_parser(shared_ptr<IntrusionLog> logger = nullptr)
    {
        return make_unique<PacketPerser>(logger);
    }
}
