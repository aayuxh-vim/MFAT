#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>

// Forward declaration
class BitStream;

//=============================================================================
// HUFFMAN NODE STRUCTURE
//=============================================================================

/**
 * @brief Node in the Huffman tree
 * 
 * Can be either:
 * - Leaf node: contains a character (data) and frequency
 * - Internal node: has left and right children, no character
 */
struct HuffmanNode {
    uint8_t data;           // Character stored (only for leaf nodes)
    unsigned int frequency; // Frequency of character(s) in subtree
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;
    
    // Constructor for leaf node
    HuffmanNode(uint8_t data, unsigned int frequency);
    
    // Constructor for internal node
    HuffmanNode(unsigned int frequency, 
                std::shared_ptr<HuffmanNode> left,
                std::shared_ptr<HuffmanNode> right);
    
    // Check if this is a leaf node
    bool isLeaf() const;
};

//=============================================================================
// HUFFMAN ENCODER/DECODER CLASS
//=============================================================================

/**
 * @brief Main Huffman coding class
 * 
 * Handles compression and decompression using Huffman coding algorithm.
 * Supports both in-memory data and file-based operations.
 */
class HuffmanCoder {
private:
    // The Huffman tree root
    std::shared_ptr<HuffmanNode> root;
    
    // Mapping from byte value to its Huffman code (as string of '0' and '1')
    std::unordered_map<uint8_t, std::string> huffmanCodes;
    
    // Frequency table: byte value → count
    std::unordered_map<uint8_t, unsigned int> frequencyTable;
    
    //-------------------------------------------------------------------------
    // PRIVATE HELPER METHODS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Build frequency table from input data
     * @param data Input bytes
     * @param size Number of bytes
     */
    void buildFrequencyTable(const uint8_t* data, size_t size);
    
    /**
     * @brief Construct Huffman tree from frequency table
     * 
     * Uses priority queue (min-heap) to build optimal tree
     */
    void buildHuffmanTree();
    
    /**
     * @brief Generate Huffman codes by traversing the tree
     * @param node Current node in traversal
     * @param code Current code string (path from root)
     */
    void generateCodes(std::shared_ptr<HuffmanNode> node, 
                       const std::string& code);
    
    /**
     * @brief Serialize tree to bit stream for storage
     * @param node Current node
     * @param bitStream Output bit stream
     * 
     * Format: 
     * - Internal node: write '0', recurse left, recurse right
     * - Leaf node: write '1', write byte value
     */
    void serializeTree(std::shared_ptr<HuffmanNode> node, 
                       BitStream& bitStream);
    
    /**
     * @brief Deserialize tree from bit stream
     * @param bitStream Input bit stream
     * @return Root of reconstructed tree
     */
    std::shared_ptr<HuffmanNode> deserializeTree(BitStream& bitStream);

public:
    //-------------------------------------------------------------------------
    // CONSTRUCTORS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Default constructor
     */
    HuffmanCoder();
    
    /**
     * @brief Destructor
     */
    ~HuffmanCoder();
    
    //-------------------------------------------------------------------------
    // TRAINING / TREE BUILDING
    //-------------------------------------------------------------------------
    
    /**
     * @brief Train the encoder on data to build Huffman tree
     * @param data Input bytes
     * @param size Number of bytes
     * 
     * Call this before compress() to analyze data and build optimal tree
     */
    void train(const uint8_t* data, size_t size);
    
    /**
     * @brief Train on data from vector
     * @param data Input data vector
     */
    void train(const std::vector<uint8_t>& data);
    
    //-------------------------------------------------------------------------
    // COMPRESSION
    //-------------------------------------------------------------------------
    
    /**
     * @brief Compress data using the current Huffman tree
     * @param data Input bytes to compress
     * @param size Number of bytes
     * @return Compressed data (including tree and encoded data)
     * 
     * Output format:
     * - Serialized Huffman tree
     * - Original data size (8 bytes)
     * - Compressed data bits
     * - Padding bits (to align to byte boundary)
     */
    std::vector<uint8_t> compress(const uint8_t* data, size_t size);
    
    /**
     * @brief Compress data from vector
     * @param data Input data
     * @return Compressed data
     */
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    
    /**
     * @brief Compress data without including tree (for multiple files)
     * @param data Input bytes
     * @param size Number of bytes
     * @return Compressed data only (tree must be stored separately)
     * 
     * Use this when compressing multiple files with the same tree.
     * Store tree once, then compress multiple files with compressData().
     */
    std::vector<uint8_t> compressData(const uint8_t* data, size_t size);
    
    //-------------------------------------------------------------------------
    // DECOMPRESSION
    //-------------------------------------------------------------------------
    
    /**
     * @brief Decompress data (includes tree extraction)
     * @param compressedData Compressed bytes
     * @return Original uncompressed data
     * 
     * Automatically extracts tree from compressed data
     */
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressedData);
    
    /**
     * @brief Decompress data using current tree (for multiple files)
     * @param compressedData Compressed bytes (without tree)
     * @param originalSize Original uncompressed size
     * @return Decompressed data
     * 
     * Use this when tree was stored separately (multi-file scenario)
     */
    std::vector<uint8_t> decompressData(const std::vector<uint8_t>& compressedData,
                                         size_t originalSize);
    
    //-------------------------------------------------------------------------
    // TREE MANAGEMENT
    //-------------------------------------------------------------------------
    
    /**
     * @brief Serialize the Huffman tree to bytes
     * @return Serialized tree data
     * 
     * Use this to store tree separately from compressed data
     */
    std::vector<uint8_t> serializeTree();
    
    /**
     * @brief Load tree from serialized bytes
     * @param treeData Serialized tree
     * 
     * Use this to reconstruct tree for decompression
     */
    void deserializeTree(const std::vector<uint8_t>& treeData);
    
    /**
     * @brief Check if tree is built and ready
     * @return true if tree exists
     */
    bool isTreeBuilt() const;
    
    /**
     * @brief Clear current tree and codes
     */
    void reset();
    
    //-------------------------------------------------------------------------
    // STATISTICS & INFORMATION
    //-------------------------------------------------------------------------
    
    /**
     * @brief Get the Huffman codes map
     * @return Map of byte → code string
     */
    const std::unordered_map<uint8_t, std::string>& getCodes() const;
    
    /**
     * @brief Get frequency table
     * @return Map of byte → frequency
     */
    const std::unordered_map<uint8_t, unsigned int>& getFrequencies() const;
    
    /**
     * @brief Calculate compression ratio
     * @param originalSize Original data size in bytes
     * @param compressedSize Compressed data size in bytes
     * @return Ratio (compressed/original), lower is better
     */
    static double getCompressionRatio(size_t originalSize, size_t compressedSize);
    
    /**
     * @brief Calculate space savings percentage
     * @param originalSize Original size
     * @param compressedSize Compressed size
     * @return Percentage saved (0-100)
     */
    static double getSpaceSavings(size_t originalSize, size_t compressedSize);
    
    /**
     * @brief Get average code length in bits
     * @return Average bits per symbol
     */
    double getAverageCodeLength() const;
    
    /**
     * @brief Print Huffman codes to console (for debugging)
     */
    void printCodes() const;
    
    /**
     * @brief Print tree structure (for debugging)
     */
    void printTree() const;
    
    //-------------------------------------------------------------------------
    // FILE-BASED OPERATIONS (Convenience methods)
    //-------------------------------------------------------------------------
    
    /**
     * @brief Compress a file
     * @param inputPath Path to input file
     * @param outputPath Path to output compressed file
     * @return true on success
     */
    bool compressFile(const std::string& inputPath, 
                      const std::string& outputPath);
    
    /**
     * @brief Decompress a file
     * @param inputPath Path to compressed file
     * @param outputPath Path to output decompressed file
     * @return true on success
     */
    bool decompressFile(const std::string& inputPath,
                        const std::string& outputPath);
};

//=============================================================================
// COMPARISON FUNCTOR (for priority queue)
//=============================================================================

/**
 * @brief Comparison functor for HuffmanNode pointers
 * 
 * Used by priority_queue to build min-heap based on frequency
 */
struct CompareHuffmanNode {
    bool operator()(const std::shared_ptr<HuffmanNode>& a,
                   const std::shared_ptr<HuffmanNode>& b) const {
        return a->frequency > b->frequency; // Min-heap
    }
};

//=============================================================================
// UTILITY FUNCTIONS
//=============================================================================

namespace HuffmanUtils {
    /**
     * @brief Estimate compressed size before actual compression
     * @param data Input data
     * @param size Data size
     * @return Estimated compressed size in bytes
     */
    size_t estimateCompressedSize(const uint8_t* data, size_t size);
    
    /**
     * @brief Check if data is worth compressing with Huffman
     * @param data Input data
     * @param size Data size
     * @return true if likely to achieve good compression
     * 
     * Some data (already compressed, random) won't compress well
     */
    bool isCompressible(const uint8_t* data, size_t size);
    
    /**
     * @brief Calculate entropy of data
     * @param data Input data
     * @param size Data size
     * @return Shannon entropy in bits
     * 
     * Entropy indicates theoretical compression limit
     */
    double calculateEntropy(const uint8_t* data, size_t size);
}

#endif // HUFFMAN_H
