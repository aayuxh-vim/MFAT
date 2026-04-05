#include "archive.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sys/stat.h>
#include <functional>

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <dirent.h>
#endif

#include <filesystem>
namespace fs = std::filesystem;

//=============================================================================
// FILE ENTRY
//=============================================================================

FileEntry::FileEntry()
    : originalSize(0), compressedSize(0), dataOffset(0),
      modificationTime(0), crc32Checksum(0), isDirectory(false) {
}

FileEntry::FileEntry(const std::string& path)
    : relativePath(path), originalSize(0), compressedSize(0),
      dataOffset(0), modificationTime(0), crc32Checksum(0),
      isDirectory(false) {
}

void FileEntry::serialize(std::ofstream& out) const {
    uint32_t pathLen = relativePath.length();
    out.write((char*)&pathLen, sizeof(pathLen));
    out.write(relativePath.c_str(), pathLen);
    
    out.write((char*)&originalSize, sizeof(originalSize));
    out.write((char*)&compressedSize, sizeof(compressedSize));
    out.write((char*)&dataOffset, sizeof(dataOffset));
    out.write((char*)&modificationTime, sizeof(modificationTime));
    out.write((char*)&crc32Checksum, sizeof(crc32Checksum));
    out.write((char*)&isDirectory, sizeof(isDirectory));
}

void FileEntry::deserialize(std::ifstream& in) {
    uint32_t pathLen;
    in.read((char*)&pathLen, sizeof(pathLen));
    
    relativePath.resize(pathLen);
    in.read(&relativePath[0], pathLen);
    
    in.read((char*)&originalSize, sizeof(originalSize));
    in.read((char*)&compressedSize, sizeof(compressedSize));
    in.read((char*)&dataOffset, sizeof(dataOffset));
    in.read((char*)&modificationTime, sizeof(modificationTime));
    in.read((char*)&crc32Checksum, sizeof(crc32Checksum));
    in.read((char*)&isDirectory, sizeof(isDirectory));
}

double FileEntry::getCompressionRatio() const {
    if (originalSize == 0) return 0.0;
    return static_cast<double>(compressedSize) / originalSize;
}

void FileEntry::print() const {
    std::cout << relativePath << "\n";
    std::cout << "  Original: " << originalSize << " bytes\n";
    std::cout << "  Compressed: " << compressedSize << " bytes\n";
    std::cout << "  Ratio: " << std::fixed << std::setprecision(1)
              << getCompressionRatio() * 100 << "%\n";
}

//=============================================================================
// ARCHIVE HEADER
//=============================================================================

ArchiveHeader::ArchiveHeader()
    : version(0x0100), fileCount(0), totalOriginalSize(0),
      totalCompressedSize(0), treeOffset(0), treeSize(0),
      dataOffset(0), directoryOffset(0), creationTime(0) {
    magic[0] = 'H';
    magic[1] = 'U';
    magic[2] = 'F';
    magic[3] = 'F';
}

void ArchiveHeader::serialize(std::ofstream& out) const {
    out.write(magic, 4);
    out.write((char*)&version, sizeof(version));
    out.write((char*)&fileCount, sizeof(fileCount));
    out.write((char*)&totalOriginalSize, sizeof(totalOriginalSize));
    out.write((char*)&totalCompressedSize, sizeof(totalCompressedSize));
    out.write((char*)&treeOffset, sizeof(treeOffset));
    out.write((char*)&treeSize, sizeof(treeSize));
    out.write((char*)&dataOffset, sizeof(dataOffset));
    out.write((char*)&directoryOffset, sizeof(directoryOffset));
    out.write((char*)&creationTime, sizeof(creationTime));
}

bool ArchiveHeader::deserialize(std::ifstream& in) {
    in.read(magic, 4);
    in.read((char*)&version, sizeof(version));
    in.read((char*)&fileCount, sizeof(fileCount));
    in.read((char*)&totalOriginalSize, sizeof(totalOriginalSize));
    in.read((char*)&totalCompressedSize, sizeof(totalCompressedSize));
    in.read((char*)&treeOffset, sizeof(treeOffset));
    in.read((char*)&treeSize, sizeof(treeSize));
    in.read((char*)&dataOffset, sizeof(dataOffset));
    in.read((char*)&directoryOffset, sizeof(directoryOffset));
    in.read((char*)&creationTime, sizeof(creationTime));
    
    return isValid();
}

bool ArchiveHeader::isValid() const {
    return (magic[0] == 'H' && magic[1] == 'U' &&
            magic[2] == 'F' && magic[3] == 'F');
}

void ArchiveHeader::print() const {
    std::cout << "Archive Header:\n";
    std::cout << "  Magic: " << magic[0] << magic[1] << magic[2] << magic[3] << "\n";
    std::cout << "  Version: " << (version >> 8) << "." << (version & 0xFF) << "\n";
    std::cout << "  Files: " << fileCount << "\n";
    std::cout << "  Total Original: " << totalOriginalSize << " bytes\n";
    std::cout << "  Total Compressed: " << totalCompressedSize << " bytes\n";
}

//=============================================================================
// ARCHIVE FOOTER
//=============================================================================

ArchiveFooter::ArchiveFooter() : archiveCRC32(0) {
    magic[0] = 'H';
    magic[1] = 'U';
    magic[2] = 'F';
    magic[3] = 'F';
}

void ArchiveFooter::serialize(std::ofstream& out) const {
    out.write((char*)&archiveCRC32, sizeof(archiveCRC32));
    out.write(magic, 4);
}

bool ArchiveFooter::deserialize(std::ifstream& in) {
    in.read((char*)&archiveCRC32, sizeof(archiveCRC32));
    in.read(magic, 4);
    
    return (magic[0] == 'H' && magic[1] == 'U' &&
            magic[2] == 'F' && magic[3] == 'F');
}


//=============================================================================
// ARCHIVE CREATOR
//=============================================================================

ArchiveCreator::ArchiveCreator(const std::string& outputPath)
    : archivePath(outputPath), totalBytesProcessed(0),
      totalBytesCompressed(0) {
    header.creationTime = time(nullptr);
}

ArchiveCreator::~ArchiveCreator() {
}

void ArchiveCreator::scanPath(const std::string& path, const std::string& baseDir) {
    if (ArchiveUtils::isDirectory(path)) {
        scanDirectory(path, baseDir);
    } else {
        addFile(path, baseDir);
    }
}

void ArchiveCreator::scanDirectory(const std::string& dirPath, const std::string& baseDir) {
    FileEntry dirEntry;
    dirEntry.relativePath = ArchiveUtils::getRelativePath(dirPath, baseDir);
    dirEntry.originalPath = dirPath;
    dirEntry.isDirectory = true;
    dirEntry.modificationTime = ArchiveUtils::getModificationTime(dirPath);
    fileList.push_back(dirEntry);
    
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        std::string entryPath = entry.path().string();
        
        if (entry.is_directory()) {
            scanDirectory(entryPath, baseDir);
        } else {
            addFile(entryPath, baseDir);
        }
    }
}

void ArchiveCreator::addFile(const std::string& filePath, const std::string& baseDir) {
    FileEntry entry;
    entry.relativePath = ArchiveUtils::getRelativePath(filePath, baseDir);
    entry.originalPath = filePath;
    entry.originalSize = ArchiveUtils::getFileSize(filePath);
    entry.modificationTime = ArchiveUtils::getModificationTime(filePath);
    entry.isDirectory = false;
    
    fileList.push_back(entry);
}

void ArchiveCreator::buildCompressionTree() {
    std::vector<uint8_t> trainingData;
    const size_t SAMPLE_SIZE = 10240;
    
    for (auto& entry : fileList) {
        if (entry.isDirectory) continue;
        
        std::ifstream file(entry.originalPath, std::ios::binary);
        if (!file) continue;
        
        size_t sampleSize = std::min((size_t)entry.originalSize, SAMPLE_SIZE);
        std::vector<uint8_t> sample(sampleSize);
        file.read((char*)sample.data(), sampleSize);
        
        trainingData.insert(trainingData.end(), sample.begin(), sample.end());
    }
    
    if (!trainingData.empty()) {
        huffman.train(trainingData);
    }
}

void ArchiveCreator::addPath(const std::string& path) {
    scanPath(path, path);
}

void ArchiveCreator::addPaths(const std::vector<std::string>& paths) {
    for (const auto& path : paths) {
        addPath(path);
    }
}

size_t ArchiveCreator::getFileCount() const {
    return fileList.size();
}

void ArchiveCreator::clear() {
    fileList.clear();
    totalBytesProcessed = 0;
    totalBytesCompressed = 0;
}

bool ArchiveCreator::create() {
    std::ofstream out(archivePath, std::ios::binary);
    if (!out) return false;
    
    buildCompressionTree();
    
    header.fileCount = fileList.size();
    header.treeOffset = sizeof(ArchiveHeader);
    
    out.seekp(sizeof(ArchiveHeader));
    
    writeTree(out);
    header.dataOffset = out.tellp();
    
    writeFileData(out);
    header.directoryOffset = out.tellp();
    
    writeDirectoryTable(out);
    
    writeFooter(out);
    
    out.seekp(0);
    header.serialize(out);
    
    out.close();
    return true;
}

void ArchiveCreator::writeTree(std::ofstream& out) {
    auto treeData = huffman.serializeTree();
    header.treeSize = treeData.size();
    
    out.write((char*)treeData.data(), treeData.size());
}

void ArchiveCreator::writeFileData(std::ofstream& out) {
    for (size_t i = 0; i < fileList.size(); i++) {
        auto& entry = fileList[i];
        
        if (progressCallback) {
            progressCallback(i + 1, fileList.size(), entry.relativePath);
        }
        
        if (entry.isDirectory) continue;
        
        std::ifstream file(entry.originalPath, std::ios::binary);
        if (!file) continue;
        
        std::vector<uint8_t> fileData(entry.originalSize);
        file.read((char*)fileData.data(), entry.originalSize);
        file.close();
        
        entry.crc32Checksum = CRC32::calculate(fileData);
        
        entry.dataOffset = out.tellp();
        auto compressed = huffman.compressData(fileData.data(), fileData.size());
        entry.compressedSize = compressed.size();
        
        out.write((char*)compressed.data(), compressed.size());
        
        totalBytesProcessed += entry.originalSize;
        totalBytesCompressed += entry.compressedSize;
    }
    
    header.totalOriginalSize = totalBytesProcessed;
    header.totalCompressedSize = totalBytesCompressed;
}

void ArchiveCreator::writeDirectoryTable(std::ofstream& out) {
    for (const auto& entry : fileList) {
        entry.serialize(out);
    }
}

void ArchiveCreator::writeFooter(std::ofstream& out) {
    ArchiveFooter footer;
    footer.serialize(out);
}

void ArchiveCreator::setProgressCallback(
    std::function<void(size_t, size_t, const std::string&)> callback) {
    progressCallback = callback;
}

uint64_t ArchiveCreator::getTotalOriginalSize() const {
    return totalBytesProcessed;
}

uint64_t ArchiveCreator::getTotalCompressedSize() const {
    return totalBytesCompressed;
}

double ArchiveCreator::getCompressionRatio() const {
    if (totalBytesProcessed == 0) return 0.0;
    return static_cast<double>(totalBytesCompressed) / totalBytesProcessed;
}

void ArchiveCreator::printSummary() const {
    std::cout << "\n=== Archive Creation Summary ===\n";
    std::cout << "Files: " << fileList.size() << "\n";
    std::cout << "Original size: " << ArchiveUtils::formatBytes(totalBytesProcessed) << "\n";
    std::cout << "Compressed size: " << ArchiveUtils::formatBytes(totalBytesCompressed) << "\n";
    std::cout << "Compression ratio: " << std::fixed << std::setprecision(1)
              << getCompressionRatio() * 100 << "%\n";
    std::cout << "Space saved: " << ArchiveUtils::formatBytes(totalBytesProcessed - totalBytesCompressed)
              << " (" << (1.0 - getCompressionRatio()) * 100 << "%)\n";
    std::cout << "===============================\n\n";
}


//=============================================================================
// ARCHIVE EXTRACTOR
//=============================================================================

ArchiveExtractor::ArchiveExtractor(const std::string& inputPath)
    : archivePath(inputPath) {
}

ArchiveExtractor::~ArchiveExtractor() {
    close();
}

bool ArchiveExtractor::open() {
    archiveFile.open(archivePath, std::ios::binary);
    if (!archiveFile) return false;
    
    if (!readHeader()) return false;
    if (!readTree()) return false;
    if (!readDirectoryTable()) return false;
    if (!readFooter()) return false;
    
    return true;
}

void ArchiveExtractor::close() {
    if (archiveFile.is_open()) {
        archiveFile.close();
    }
}

bool ArchiveExtractor::isOpen() const {
    return archiveFile.is_open();
}

bool ArchiveExtractor::readHeader() {
    archiveFile.seekg(0);
    return header.deserialize(archiveFile);
}

bool ArchiveExtractor::readTree() {
    archiveFile.seekg(header.treeOffset);
    
    std::vector<uint8_t> treeData(header.treeSize);
    archiveFile.read((char*)treeData.data(), header.treeSize);
    
    huffman.deserializeTree(treeData);
    return true;
}

bool ArchiveExtractor::readDirectoryTable() {
    archiveFile.seekg(header.directoryOffset);
    
    fileList.clear();
    for (uint32_t i = 0; i < header.fileCount; i++) {
        FileEntry entry;
        entry.deserialize(archiveFile);
        fileList.push_back(entry);
    }
    
    return true;
}

bool ArchiveExtractor::readFooter() {
    archiveFile.seekg(-static_cast<int>(sizeof(ArchiveFooter)), std::ios::end);
    return footer.deserialize(archiveFile);
}

void ArchiveExtractor::list() const {
    std::cout << "\nArchive: " << archivePath << "\n";
    std::cout << "Files: " << fileList.size() << "\n\n";
    
    std::cout << std::left << std::setw(40) << "Name"
              << std::right << std::setw(12) << "Original"
              << std::setw(12) << "Compressed"
              << std::setw(8) << "Ratio" << "\n";
    std::cout << std::string(72, '-') << "\n";
    
    for (const auto& entry : fileList) {
        if (entry.isDirectory) continue;
        
        std::cout << std::left << std::setw(40) << entry.relativePath
                  << std::right << std::setw(12) << entry.originalSize
                  << std::setw(12) << entry.compressedSize
                  << std::setw(7) << std::fixed << std::setprecision(1)
                  << entry.getCompressionRatio() * 100 << "%\n";
    }
    
    std::cout << "\n";
}

const std::vector<FileEntry>& ArchiveExtractor::getFileList() const {
    return fileList;
}

const ArchiveHeader& ArchiveExtractor::getHeader() const {
    return header;
}

bool ArchiveExtractor::extractAll(const std::string& outputDir) {
    for (size_t i = 0; i < fileList.size(); i++) {
        if (progressCallback) {
            progressCallback(i + 1, fileList.size(), fileList[i].relativePath);
        }
        
        if (!extractFile(fileList[i], outputDir)) {
            return false;
        }
    }
    
    return true;
}

bool ArchiveExtractor::extractFile(const std::string& fileName,
                                    const std::string& outputDir) {
    for (const auto& entry : fileList) {
        if (entry.relativePath == fileName) {
            return extractFile(entry, outputDir);
        }
    }
    return false;
}

bool ArchiveExtractor::extractFiles(const std::vector<std::string>& fileNames,
                                     const std::string& outputDir) {
    for (const auto& name : fileNames) {
        if (!extractFile(name, outputDir)) {
            return false;
        }
    }
    return true;
}

bool ArchiveExtractor::extractFile(const FileEntry& entry,
                                    const std::string& outputDir) {
    if (entry.isDirectory) {
        createDirectories(outputDir + "/" + entry.relativePath);
        return true;
    }
    
    archiveFile.seekg(entry.dataOffset);
    std::vector<uint8_t> compressed(entry.compressedSize);
    archiveFile.read((char*)compressed.data(), entry.compressedSize);
    
    auto decompressed = huffman.decompressData(compressed, entry.originalSize);
    
    uint32_t crc = CRC32::calculate(decompressed);
    if (crc != entry.crc32Checksum) {
        std::cerr << "CRC mismatch: " << entry.relativePath << "\n";
        return false;
    }
    
    std::string outPath = outputDir + "/" + entry.relativePath;
    createDirectories(fs::path(outPath).parent_path().string());
    
    std::ofstream out(outPath, std::ios::binary);
    if (!out) return false;
    
    out.write((char*)decompressed.data(), decompressed.size());
    out.close();
    
    return true;
}

void ArchiveExtractor::createDirectories(const std::string& path) {
    fs::create_directories(path);
}

bool ArchiveExtractor::verifyCRC32(const std::string& filePath,
                                    uint32_t expectedCRC) {
    uint32_t actualCRC = CRC32::calculateFile(filePath);
    return actualCRC == expectedCRC;
}

bool ArchiveExtractor::verify() {
    std::cout << "Verifying archive...\n";
    
    for (const auto& entry : fileList) {
        if (entry.isDirectory) continue;
        
        archiveFile.seekg(entry.dataOffset);
        std::vector<uint8_t> compressed(entry.compressedSize);
        archiveFile.read((char*)compressed.data(), entry.compressedSize);
        
        auto decompressed = huffman.decompressData(compressed, entry.originalSize);
        
        uint32_t crc = CRC32::calculate(decompressed);
        if (crc != entry.crc32Checksum) {
            std::cerr << "FAILED: " << entry.relativePath << "\n";
            return false;
        }
        
        std::cout << "OK: " << entry.relativePath << "\n";
    }
    
    std::cout << "Archive is intact!\n";
    return true;
}

void ArchiveExtractor::setProgressCallback(
    std::function<void(size_t, size_t, const std::string&)> callback) {
    progressCallback = callback;
}

//=============================================================================
// UTILITY FUNCTIONS
//=============================================================================

namespace ArchiveUtils {
    bool fileExists(const std::string& path) {
        return fs::exists(path);
    }
    
    bool isDirectory(const std::string& path) {
        return fs::is_directory(path);
    }
    
    uint64_t getFileSize(const std::string& path) {
        return fs::file_size(path);
    }
    
    time_t getModificationTime(const std::string& path) {
        auto ftime = fs::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        return std::chrono::system_clock::to_time_t(sctp);
    }
    
    std::string getRelativePath(const std::string& path, const std::string& base) {
        fs::path p(path);
        fs::path b(base);
        
        // If base is a file (not a directory), use its parent directory
        // For single files, parent_path() may be empty, so use filename
        if (!fs::is_directory(b)) {
            return p.filename().string();
        }
        
        return fs::relative(p, b).string();
    }
    
    std::string formatBytes(uint64_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = bytes;
        
        while (size >= 1024 && unitIndex < 4) {
            size /= 1024;
            unitIndex++;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
        return oss.str();
    }
    
    std::string formatTime(time_t t) {
        char buffer[80];
        struct tm* timeinfo = localtime(&t);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }
}
