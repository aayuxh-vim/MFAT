#include "crc32.h"
#include <fstream>

uint32_t CRC32::crcTable[256];
bool CRC32::tableInitialized = false;

void CRC32::initializeTable() {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
        crcTable[i] = crc;
    }
    tableInitialized = true;
}

CRC32::CRC32() : crc(0xFFFFFFFF) {
    if (!tableInitialized) {
        initializeTable();
    }
}

void CRC32::reset() {
    crc = 0xFFFFFFFF;
}

void CRC32::update(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crcTable[index];
    }
}

void CRC32::update(const std::vector<uint8_t>& data) {
    update(data.data(), data.size());
}

uint32_t CRC32::finalize() const {
    return crc ^ 0xFFFFFFFF;
}

uint32_t CRC32::calculate(const uint8_t* data, size_t length) {
    CRC32 crc32;
    crc32.update(data, length);
    return crc32.finalize();
}

uint32_t CRC32::calculate(const std::vector<uint8_t>& data) {
    return calculate(data.data(), data.size());
}

uint32_t CRC32::calculateFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return 0;
    
    CRC32 crc32;
    const size_t BUFFER_SIZE = 8192;
    uint8_t buffer[BUFFER_SIZE];
    
    while (file.read((char*)buffer, BUFFER_SIZE) || file.gcount() > 0) {
        crc32.update(buffer, file.gcount());
    }
    
    return crc32.finalize();
}
