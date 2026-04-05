#include "bitstream.h"
#include <iostream>
#include <iomanip>

BitStream::BitStream()
    : currentByte(0), bitPosition(0), readPosition(0) {
}

BitStream::BitStream(const std::vector<uint8_t>& data)
    : buffer(data), currentByte(0), bitPosition(0), readPosition(0) {
}

BitStream::~BitStream() {
}

void BitStream::writeBit(int bit) {
    if (bit) {
        currentByte |= (1 << (7 - bitPosition));
    }
    
    bitPosition++;
    
    if (bitPosition == 8) {
        buffer.push_back(currentByte);
        currentByte = 0;
        bitPosition = 0;
    }
}

void BitStream::writeBits(uint32_t value, int numBits) {
    for (int i = numBits - 1; i >= 0; i--) {
        int bit = (value >> i) & 1;
        writeBit(bit);
    }
}

void BitStream::writeBitString(const std::string& bits) {
    for (char c : bits) {
        if (c == '1') {
            writeBit(1);
        } else if (c == '0') {
            writeBit(0);
        }
    }
}

void BitStream::writeByte(uint8_t byte) {
    writeBits(byte, 8);
}

void BitStream::flush() {
    if (bitPosition > 0) {
        buffer.push_back(currentByte);
        currentByte = 0;
        bitPosition = 0;
    }
}

int BitStream::readBit() {
    if (readPosition >= buffer.size() * 8) {
        return -1;
    }
    
    size_t byteIndex = readPosition / 8;
    int bitIndex = readPosition % 8;
    
    uint8_t byte = buffer[byteIndex];
    int bit = (byte >> (7 - bitIndex)) & 1;
    
    readPosition++;
    
    return bit;
}

uint32_t BitStream::readBits(int numBits) {
    uint32_t value = 0;
    
    for (int i = 0; i < numBits; i++) {
        int bit = readBit();
        if (bit == -1) break;
        
        value = (value << 1) | bit;
    }
    
    return value;
}

uint8_t BitStream::readByte() {
    return static_cast<uint8_t>(readBits(8));
}

bool BitStream::hasMoreBits() const {
    return readPosition < buffer.size() * 8;
}

const std::vector<uint8_t>& BitStream::getData() const {
    return buffer;
}

std::vector<uint8_t>& BitStream::getData() {
    return buffer;
}

size_t BitStream::size() const {
    return buffer.size();
}

size_t BitStream::totalBits() const {
    return buffer.size() * 8 + bitPosition;
}

void BitStream::clear() {
    buffer.clear();
    currentByte = 0;
    bitPosition = 0;
    readPosition = 0;
}

void BitStream::resetRead() {
    readPosition = 0;
}

void BitStream::printBits(size_t numBits) const {
    size_t bitsToShow = (numBits == 0) ? buffer.size() * 8 : numBits;
    
    for (size_t i = 0; i < bitsToShow && i < buffer.size() * 8; i++) {
        size_t byteIndex = i / 8;
        int bitIndex = i % 8;
        
        uint8_t byte = buffer[byteIndex];
        int bit = (byte >> (7 - bitIndex)) & 1;
        
        std::cout << bit;
        
        if ((i + 1) % 8 == 0) {
            std::cout << " ";
        }
    }
    std::cout << "\n";
}

std::string BitStream::toBitString() const {
    std::string result;
    
    for (size_t i = 0; i < buffer.size() * 8; i++) {
        size_t byteIndex = i / 8;
        int bitIndex = i % 8;
        
        uint8_t byte = buffer[byteIndex];
        int bit = (byte >> (7 - bitIndex)) & 1;
        
        result += (bit ? '1' : '0');
    }
    
    return result;
}
