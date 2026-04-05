#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <vector>
#include <cstdint>
#include <string>

/**
 * @brief BitStream - Read and write individual bits
 * 
 * Huffman codes are variable-length (1-8+ bits), but files are byte-based.
 * BitStream handles bit-level reading/writing with automatic buffering.
 */
class BitStream {
private:
    //=========================================================================
    // INTERNAL DATA
    //=========================================================================
    
    std::vector<uint8_t> buffer;  // The actual bytes stored
    uint8_t currentByte;          // Current byte being built
    int bitPosition;              // Position in current byte (0-7)
    size_t readPosition;          // Current read position in buffer (in bits)
    
public:
    //=========================================================================
    // CONSTRUCTORS
    //=========================================================================
    
    /**
     * @brief Default constructor - creates empty bitstream
     */
    BitStream();
    
    /**
     * @brief Construct from existing data
     * @param data Byte array to read from
     */
    BitStream(const std::vector<uint8_t>& data);
    
    /**
     * @brief Destructor
     */
    ~BitStream();
    
    //=========================================================================
    // WRITING BITS
    //=========================================================================
    
    /**
     * @brief Write a single bit
     * @param bit The bit value (0 or 1)
     */
    void writeBit(int bit);
    
    /**
     * @brief Write multiple bits from a value
     * @param value The value to write
     * @param numBits Number of bits to write (1-32)
     * 
     * Example: writeBits(5, 3) writes "101"
     */
    void writeBits(uint32_t value, int numBits);
    
    /**
     * @brief Write a string of bits
     * @param bits String containing '0' and '1' characters
     * 
     * Example: writeBitString("1011")
     */
    void writeBitString(const std::string& bits);
    
    /**
     * @brief Write a whole byte
     * @param byte The byte to write
     */
    void writeByte(uint8_t byte);
    
    /**
     * @brief Flush remaining bits with padding
     * 
     * Call this when done writing to ensure all bits are saved.
     * Pads incomplete byte with zeros.
     */
    void flush();
    
    //=========================================================================
    // READING BITS
    //=========================================================================
    
    /**
     * @brief Read a single bit
     * @return The bit value (0 or 1)
     */
    int readBit();
    
    /**
     * @brief Read multiple bits into a value
     * @param numBits Number of bits to read (1-32)
     * @return Value constructed from bits
     * 
     * Example: reading "101" with numBits=3 returns 5
     */
    uint32_t readBits(int numBits);
    
    /**
     * @brief Read a whole byte
     * @return The byte value
     */
    uint8_t readByte();
    
    /**
     * @brief Check if there are more bits to read
     * @return true if more bits available
     */
    bool hasMoreBits() const;
    
    //=========================================================================
    // DATA ACCESS
    //=========================================================================
    
    /**
     * @brief Get the underlying byte buffer
     * @return Reference to byte vector
     */
    const std::vector<uint8_t>& getData() const;
    
    /**
     * @brief Get the byte buffer (non-const)
     * @return Reference to byte vector
     */
    std::vector<uint8_t>& getData();
    
    /**
     * @brief Get size in bytes
     * @return Number of complete bytes
     */
    size_t size() const;
    
    /**
     * @brief Get total number of bits written
     * @return Total bits
     */
    size_t totalBits() const;
    
    /**
     * @brief Clear all data and reset
     */
    void clear();
    
    /**
     * @brief Reset read position to beginning
     */
    void resetRead();
    
    //=========================================================================
    // UTILITY
    //=========================================================================
    
    /**
     * @brief Print bits as string (for debugging)
     * @param numBits Number of bits to print (0 = all)
     */
    void printBits(size_t numBits = 0) const;
    
    /**
     * @brief Convert to bit string
     * @return String of '0' and '1' characters
     */
    std::string toBitString() const;
};

#endif // BITSTREAM_H
