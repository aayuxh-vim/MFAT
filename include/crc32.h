#ifndef CRC32_H
#define CRC32_H

#include <cstdint>
#include <vector>
#include <string>

class CRC32 {
private:
    uint32_t crc;
    static uint32_t crcTable[256];
    static bool tableInitialized;
    
    static void initializeTable();

public:
    CRC32();
    
    void reset();
    void update(const uint8_t* data, size_t length);
    void update(const std::vector<uint8_t>& data);
    uint32_t finalize() const;
    
    static uint32_t calculate(const uint8_t* data, size_t length);
    static uint32_t calculate(const std::vector<uint8_t>& data);
    static uint32_t calculateFile(const std::string& filePath);
};

#endif // CRC32_H
