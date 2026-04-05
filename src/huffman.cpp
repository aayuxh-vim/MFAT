#include "huffman.h"
#include "bitstream.h"
#include <queue>
#include <iostream>
#include <fstream>
#include <functional>

//=============================================================================
// HUFFMAN NODE IMPLEMENTATION
//=============================================================================

HuffmanNode::HuffmanNode(uint8_t data, unsigned int frequency)
    : data(data), frequency(frequency), left(nullptr), right(nullptr) {
}

HuffmanNode::HuffmanNode(unsigned int frequency,
                         std::shared_ptr<HuffmanNode> left,
                         std::shared_ptr<HuffmanNode> right)
    : data(0), frequency(frequency), left(left), right(right) {
}

bool HuffmanNode::isLeaf() const {
    return !left && !right;
}

//=============================================================================
// HUFFMAN CODER IMPLEMENTATION
//=============================================================================

HuffmanCoder::HuffmanCoder() : root(nullptr) {
}

HuffmanCoder::~HuffmanCoder() {
}

void HuffmanCoder::buildFrequencyTable(const uint8_t* data, size_t size) {
    frequencyTable.clear();
    for (size_t i = 0; i < size; i++) {
        frequencyTable[data[i]]++;
    }
}

void HuffmanCoder::buildHuffmanTree() {
    std::priority_queue<std::shared_ptr<HuffmanNode>,
                       std::vector<std::shared_ptr<HuffmanNode>>,
                       CompareHuffmanNode> pq;
    
    for (const auto& pair : frequencyTable) {
        auto node = std::make_shared<HuffmanNode>(pair.first, pair.second);
        pq.push(node);
    }
    
    if (pq.size() == 1) {
        auto single = pq.top();
        root = std::make_shared<HuffmanNode>(single->frequency, single, nullptr);
        return;
    }
    
    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        
        auto parent = std::make_shared<HuffmanNode>(
            left->frequency + right->frequency,
            left, right
        );
        
        pq.push(parent);
    }
    
    root = pq.top();
}

void HuffmanCoder::generateCodes(std::shared_ptr<HuffmanNode> node,
                                  const std::string& code) {
    if (!node) return;
    
    if (node->isLeaf()) {
        huffmanCodes[node->data] = code.empty() ? "0" : code;
        return;
    }
    
    generateCodes(node->left, code + "0");
    generateCodes(node->right, code + "1");
}

void HuffmanCoder::train(const uint8_t* data, size_t size) {
    buildFrequencyTable(data, size);
    buildHuffmanTree();
    huffmanCodes.clear();
    generateCodes(root, "");
}

void HuffmanCoder::train(const std::vector<uint8_t>& data) {
    train(data.data(), data.size());
}

std::vector<uint8_t> HuffmanCoder::compress(const uint8_t* data, size_t size) {
    if (!root) {
        train(data, size);
    }
    
    BitStream bs;
    
    auto treeData = serializeTree();
    bs.writeBits(treeData.size(), 32);
    for (uint8_t byte : treeData) {
        bs.writeByte(byte);
    }
    
    bs.writeBits(size, 64);
    
    for (size_t i = 0; i < size; i++) {
        const std::string& code = huffmanCodes[data[i]];
        bs.writeBitString(code);
    }
    
    bs.flush();
    return bs.getData();
}

std::vector<uint8_t> HuffmanCoder::compress(const std::vector<uint8_t>& data) {
    return compress(data.data(), data.size());
}

std::vector<uint8_t> HuffmanCoder::compressData(const uint8_t* data, size_t size) {
    BitStream bs;
    
    for (size_t i = 0; i < size; i++) {
        const std::string& code = huffmanCodes[data[i]];
        bs.writeBitString(code);
    }
    
    bs.flush();
    return bs.getData();
}

std::vector<uint8_t> HuffmanCoder::decompress(const std::vector<uint8_t>& compressedData) {
    BitStream bs(compressedData);
    
    uint32_t treeSize = bs.readBits(32);
    std::vector<uint8_t> treeData(treeSize);
    for (size_t i = 0; i < treeSize; i++) {
        treeData[i] = bs.readByte();
    }
    deserializeTree(treeData);
    
    uint64_t originalSize = 0;
    for (int i = 0; i < 64; i++) {
        originalSize |= ((uint64_t)bs.readBit() << (63 - i));
    }
    
    std::vector<uint8_t> result;
    result.reserve(originalSize);
    
    auto current = root;
    while (result.size() < originalSize && bs.hasMoreBits()) {
        int bit = bs.readBit();
        if (bit == -1) break;
        
        current = (bit == 0) ? current->left : current->right;
        
        if (current->isLeaf()) {
            result.push_back(current->data);
            current = root;
        }
    }
    
    return result;
}

std::vector<uint8_t> HuffmanCoder::decompressData(
    const std::vector<uint8_t>& compressedData,
    size_t originalSize) {
    
    BitStream bs(compressedData);
    std::vector<uint8_t> result;
    result.reserve(originalSize);
    
    auto current = root;
    while (result.size() < originalSize && bs.hasMoreBits()) {
        int bit = bs.readBit();
        if (bit == -1) break;
        
        current = (bit == 0) ? current->left : current->right;
        
        if (current->isLeaf()) {
            result.push_back(current->data);
            current = root;
        }
    }
    
    return result;
}

void HuffmanCoder::serializeTree(std::shared_ptr<HuffmanNode> node,
                                  BitStream& bitStream) {
    if (!node) return;
    
    if (node->isLeaf()) {
        bitStream.writeBit(1);
        bitStream.writeByte(node->data);
    } else {
        bitStream.writeBit(0);
        serializeTree(node->left, bitStream);
        serializeTree(node->right, bitStream);
    }
}

std::vector<uint8_t> HuffmanCoder::serializeTree() {
    BitStream bs;
    serializeTree(root, bs);
    bs.flush();
    return bs.getData();
}

std::shared_ptr<HuffmanNode> HuffmanCoder::deserializeTree(BitStream& bitStream) {
    if (!bitStream.hasMoreBits()) return nullptr;
    
    int bit = bitStream.readBit();
    if (bit == 1) {
        uint8_t data = bitStream.readByte();
        return std::make_shared<HuffmanNode>(data, 0);
    } else {
        auto left = deserializeTree(bitStream);
        auto right = deserializeTree(bitStream);
        return std::make_shared<HuffmanNode>(0, left, right);
    }
}

void HuffmanCoder::deserializeTree(const std::vector<uint8_t>& treeData) {
    BitStream bs(treeData);
    root = deserializeTree(bs);
    huffmanCodes.clear();
    generateCodes(root, "");
}

bool HuffmanCoder::isTreeBuilt() const {
    return root != nullptr;
}

void HuffmanCoder::reset() {
    root = nullptr;
    huffmanCodes.clear();
    frequencyTable.clear();
}

const std::unordered_map<uint8_t, std::string>& HuffmanCoder::getCodes() const {
    return huffmanCodes;
}

const std::unordered_map<uint8_t, unsigned int>& HuffmanCoder::getFrequencies() const {
    return frequencyTable;
}

double HuffmanCoder::getCompressionRatio(size_t originalSize, size_t compressedSize) {
    if (originalSize == 0) return 0.0;
    return static_cast<double>(compressedSize) / originalSize;
}

double HuffmanCoder::getSpaceSavings(size_t originalSize, size_t compressedSize) {
    if (originalSize == 0) return 0.0;
    return (1.0 - getCompressionRatio(originalSize, compressedSize)) * 100.0;
}

double HuffmanCoder::getAverageCodeLength() const {
    if (frequencyTable.empty()) return 0.0;
    
    unsigned int totalFreq = 0;
    double weightedSum = 0.0;
    
    for (const auto& pair : frequencyTable) {
        totalFreq += pair.second;
        weightedSum += pair.second * huffmanCodes.at(pair.first).length();
    }
    
    return weightedSum / totalFreq;
}

void HuffmanCoder::printCodes() const {
    std::cout << "\n=== Huffman Codes ===\n";
    for (const auto& pair : huffmanCodes) {
        char ch = (pair.first >= 32 && pair.first < 127) ? pair.first : '?';
        std::cout << "'" << ch << "' (" << (int)pair.first << "): " 
                  << pair.second << "\n";
    }
    std::cout << "====================\n\n";
}

void HuffmanCoder::printTree() const {
    std::cout << "Huffman Tree Structure:\n";
    std::function<void(std::shared_ptr<HuffmanNode>, std::string)> print;
    print = [&](std::shared_ptr<HuffmanNode> node, std::string indent) {
        if (!node) return;
        if (node->isLeaf()) {
            char ch = (node->data >= 32 && node->data < 127) ? node->data : '?';
            std::cout << indent << "Leaf: '" << ch << "' (" << node->frequency << ")\n";
        } else {
            std::cout << indent << "Internal (" << node->frequency << ")\n";
            print(node->left, indent + "  L:");
            print(node->right, indent + "  R:");
        }
    };
    print(root, "");
}

bool HuffmanCoder::compressFile(const std::string& inputPath,
                                 const std::string& outputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) return false;
    
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    in.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    in.read((char*)data.data(), size);
    in.close();
    
    auto compressed = compress(data);
    
    std::ofstream out(outputPath, std::ios::binary);
    if (!out) return false;
    
    out.write((char*)compressed.data(), compressed.size());
    out.close();
    
    return true;
}

bool HuffmanCoder::decompressFile(const std::string& inputPath,
                                   const std::string& outputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) return false;
    
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    in.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> compressed(size);
    in.read((char*)compressed.data(), size);
    in.close();
    
    auto decompressed = decompress(compressed);
    
    std::ofstream out(outputPath, std::ios::binary);
    if (!out) return false;
    
    out.write((char*)decompressed.data(), decompressed.size());
    out.close();
    
    return true;
}

namespace HuffmanUtils {
    size_t estimateCompressedSize(const uint8_t* data, size_t size) {
        std::unordered_map<uint8_t, unsigned int> freq;
        for (size_t i = 0; i < size; i++) {
            freq[data[i]]++;
        }
        
        double entropy = 0.0;
        for (const auto& pair : freq) {
            double p = static_cast<double>(pair.second) / size;
            entropy -= p * log2(p);
        }
        
        return static_cast<size_t>(entropy * size / 8.0);
    }
    
    bool isCompressible(const uint8_t* data, size_t size) {
        if (size < 100) return false;
        
        std::unordered_map<uint8_t, unsigned int> freq;
        for (size_t i = 0; i < size; i++) {
            freq[data[i]]++;
        }
        
        if (freq.size() > 250) return false;
        
        double entropy = calculateEntropy(data, size);
        return entropy < 7.5;
    }
    
    double calculateEntropy(const uint8_t* data, size_t size) {
        std::unordered_map<uint8_t, unsigned int> freq;
        for (size_t i = 0; i < size; i++) {
            freq[data[i]]++;
        }
        
        double entropy = 0.0;
        for (const auto& pair : freq) {
            double p = static_cast<double>(pair.second) / size;
            entropy -= p * log2(p);
        }
        
        return entropy;
    }
}
