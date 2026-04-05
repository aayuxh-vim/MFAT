#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <string>
#include <vector>
#include <cstdint>
#include <ctime>
#include <fstream>
#include "huffman.h"
#include "crc32.h"

//=============================================================================
// FILE ENTRY - Information about one file in the archive
//=============================================================================

/**
 * @brief Stores metadata for a single file in the archive
 */
struct FileEntry {
    //-------------------------------------------------------------------------
    // FILE INFORMATION
    //-------------------------------------------------------------------------
    
    std::string relativePath;     // Path inside archive (e.g., "docs/readme.txt")
    uint64_t originalSize;        // Size before compression (bytes)
    uint64_t compressedSize;      // Size after compression (bytes)
    uint64_t dataOffset;          // Where compressed data starts in archive
    
    //-------------------------------------------------------------------------
    // METADATA
    //-------------------------------------------------------------------------
    
    time_t modificationTime;      // When file was last modified
    uint32_t crc32Checksum;       // CRC32 for integrity checking
    bool isDirectory;             // true if this is a directory entry
    
    //-------------------------------------------------------------------------
    // METHODS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Default constructor
     */
    FileEntry();
    
    /**
     * @brief Constructor with path
     */
    FileEntry(const std::string& path);
    
    /**
     * @brief Write this entry to a binary stream
     * @param out Output stream
     */
    void serialize(std::ofstream& out) const;
    
    /**
     * @brief Read this entry from a binary stream
     * @param in Input stream
     */
    void deserialize(std::ifstream& in);
    
    /**
     * @brief Get compression ratio
     * @return Ratio (compressed/original)
     */
    double getCompressionRatio() const;
    
    /**
     * @brief Print information (for debugging)
     */
    void print() const;
};

//=============================================================================
// ARCHIVE HEADER - Identifies the archive and contains global info
//=============================================================================

/**
 * @brief Archive file header (at the beginning of archive)
 */
struct ArchiveHeader {
    //-------------------------------------------------------------------------
    // MAGIC NUMBER & VERSION
    //-------------------------------------------------------------------------
    
    char magic[4];                // "HUFF" - identifies our format
    uint16_t version;             // Format version (e.g., 0x0100 for 1.0)
    
    //-------------------------------------------------------------------------
    // ARCHIVE STATISTICS
    //-------------------------------------------------------------------------
    
    uint32_t fileCount;           // Number of files in archive
    uint64_t totalOriginalSize;   // Sum of all original file sizes
    uint64_t totalCompressedSize; // Sum of all compressed sizes
    
    //-------------------------------------------------------------------------
    // OFFSETS (where things are in the file)
    //-------------------------------------------------------------------------
    
    uint64_t treeOffset;          // Where Huffman tree data starts
    uint64_t treeSize;            // Size of tree data
    uint64_t dataOffset;          // Where compressed file data starts
    uint64_t directoryOffset;     // Where directory table starts
    
    //-------------------------------------------------------------------------
    // TIMESTAMPS
    //-------------------------------------------------------------------------
    
    time_t creationTime;          // When archive was created
    
    //-------------------------------------------------------------------------
    // METHODS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Default constructor - initializes with default values
     */
    ArchiveHeader();
    
    /**
     * @brief Write header to binary stream
     * @param out Output stream
     */
    void serialize(std::ofstream& out) const;
    
    /**
     * @brief Read header from binary stream
     * @param in Input stream
     * @return true if header is valid
     */
    bool deserialize(std::ifstream& in);
    
    /**
     * @brief Check if magic number is valid
     * @return true if this is our archive format
     */
    bool isValid() const;
    
    /**
     * @brief Print header info (for debugging)
     */
    void print() const;
};

//=============================================================================
// ARCHIVE FOOTER - At the end for verification
//=============================================================================

/**
 * @brief Archive footer (at the end of archive)
 */
struct ArchiveFooter {
    uint32_t archiveCRC32;        // CRC32 of entire archive (excluding footer)
    char magic[4];                // "HUFF" again (verification)
    
    /**
     * @brief Default constructor
     */
    ArchiveFooter();
    
    /**
     * @brief Write footer to stream
     */
    void serialize(std::ofstream& out) const;
    
    /**
     * @brief Read footer from stream
     */
    bool deserialize(std::ifstream& in);
};

//=============================================================================
// ARCHIVE CREATOR - Creates archives
//=============================================================================

/**
 * @brief Creates compressed archives from files and directories
 */
class ArchiveCreator {
private:
    //-------------------------------------------------------------------------
    // INTERNAL DATA
    //-------------------------------------------------------------------------
    
    HuffmanCoder huffman;              // Compression engine
    std::vector<FileEntry> fileList;   // List of files to archive
    ArchiveHeader header;              // Archive header
    std::string archivePath;           // Output archive path
    
    // Statistics
    uint64_t totalBytesProcessed;
    uint64_t totalBytesCompressed;
    
    //-------------------------------------------------------------------------
    // PRIVATE METHODS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Scan a path and add files to fileList
     * @param path File or directory path
     * @param baseDir Base directory for relative paths
     */
    void scanPath(const std::string& path, const std::string& baseDir);
    
    /**
     * @brief Recursively scan directory
     * @param dirPath Directory to scan
     * @param baseDir Base for relative paths
     */
    void scanDirectory(const std::string& dirPath, const std::string& baseDir);
    
    /**
     * @brief Add a single file to the list
     * @param filePath File to add
     * @param baseDir Base for relative path
     */
    void addFile(const std::string& filePath, const std::string& baseDir);
    
    /**
     * @brief Build Huffman tree from all files
     * 
     * Reads sample data from all files to build optimal tree
     */
    void buildCompressionTree();
    
    /**
     * @brief Write the archive header
     * @param out Output stream
     */
    void writeHeader(std::ofstream& out);
    
    /**
     * @brief Write the Huffman tree
     * @param out Output stream
     */
    void writeTree(std::ofstream& out);
    
    /**
     * @brief Write all compressed file data
     * @param out Output stream
     */
    void writeFileData(std::ofstream& out);
    
    /**
     * @brief Write the directory table
     * @param out Output stream
     */
    void writeDirectoryTable(std::ofstream& out);
    
    /**
     * @brief Write the footer
     * @param out Output stream
     */
    void writeFooter(std::ofstream& out);
    
    /**
     * @brief Calculate CRC32 for a file
     * @param filePath File to check
     * @return CRC32 checksum
     */
    uint32_t calculateFileCRC32(const std::string& filePath);

public:
    //-------------------------------------------------------------------------
    // CONSTRUCTORS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Constructor
     * @param outputPath Where to create the archive
     */
    ArchiveCreator(const std::string& outputPath);
    
    /**
     * @brief Destructor
     */
    ~ArchiveCreator();
    
    //-------------------------------------------------------------------------
    // ADDING FILES
    //-------------------------------------------------------------------------
    
    /**
     * @brief Add a file or directory to archive
     * @param path Path to file or directory
     * 
     * If directory, recursively adds all files inside
     */
    void addPath(const std::string& path);
    
    /**
     * @brief Add multiple paths
     * @param paths Vector of file/directory paths
     */
    void addPaths(const std::vector<std::string>& paths);
    
    /**
     * @brief Get number of files to be archived
     * @return File count
     */
    size_t getFileCount() const;
    
    /**
     * @brief Clear all files from list
     */
    void clear();
    
    //-------------------------------------------------------------------------
    // CREATING ARCHIVE
    //-------------------------------------------------------------------------
    
    /**
     * @brief Create the archive file
     * @return true on success
     * 
     * This does all the work:
     * 1. Scans all files
     * 2. Builds Huffman tree
     * 3. Compresses all files
     * 4. Writes archive
     */
    bool create();
    
    //-------------------------------------------------------------------------
    // PROGRESS CALLBACK
    //-------------------------------------------------------------------------
    
    /**
     * @brief Set a callback function for progress updates
     * @param callback Function called with (currentFile, totalFiles, fileName)
     */
    void setProgressCallback(
        std::function<void(size_t, size_t, const std::string&)> callback);
    
    //-------------------------------------------------------------------------
    // STATISTICS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Get total original size
     */
    uint64_t getTotalOriginalSize() const;
    
    /**
     * @brief Get total compressed size
     */
    uint64_t getTotalCompressedSize() const;
    
    /**
     * @brief Get compression ratio
     */
    double getCompressionRatio() const;
    
    /**
     * @brief Print summary
     */
    void printSummary() const;

private:
    // Progress callback function
    std::function<void(size_t, size_t, const std::string&)> progressCallback;
};

//=============================================================================
// ARCHIVE EXTRACTOR - Extracts files from archives
//=============================================================================

/**
 * @brief Extracts files from compressed archives
 */
class ArchiveExtractor {
private:
    //-------------------------------------------------------------------------
    // INTERNAL DATA
    //-------------------------------------------------------------------------
    
    HuffmanCoder huffman;              // Decompression engine
    std::vector<FileEntry> fileList;   // List of files in archive
    ArchiveHeader header;              // Archive header
    ArchiveFooter footer;              // Archive footer
    std::string archivePath;           // Input archive path
    std::ifstream archiveFile;         // Open archive file
    
    //-------------------------------------------------------------------------
    // PRIVATE METHODS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Read and validate archive header
     * @return true if valid
     */
    bool readHeader();
    
    /**
     * @brief Read Huffman tree from archive
     * @return true on success
     */
    bool readTree();
    
    /**
     * @brief Read directory table
     * @return true on success
     */
    bool readDirectoryTable();
    
    /**
     * @brief Read and verify footer
     * @return true if valid
     */
    bool readFooter();
    
    /**
     * @brief Extract a single file
     * @param entry File entry to extract
     * @param outputDir Output directory
     * @return true on success
     */
    bool extractFile(const FileEntry& entry, const std::string& outputDir);
    
    /**
     * @brief Create directory structure
     * @param path Directory path to create
     */
    void createDirectories(const std::string& path);
    
    /**
     * @brief Verify file CRC32 after extraction
     * @param filePath Extracted file
     * @param expectedCRC Expected checksum
     * @return true if match
     */
    bool verifyCRC32(const std::string& filePath, uint32_t expectedCRC);

public:
    //-------------------------------------------------------------------------
    // CONSTRUCTORS
    //-------------------------------------------------------------------------
    
    /**
     * @brief Constructor
     * @param inputPath Archive file to extract from
     */
    ArchiveExtractor(const std::string& inputPath);
    
    /**
     * @brief Destructor
     */
    ~ArchiveExtractor();
    
    //-------------------------------------------------------------------------
    // OPENING ARCHIVE
    //-------------------------------------------------------------------------
    
    /**
     * @brief Open and read archive metadata
     * @return true on success
     */
    bool open();
    
    /**
     * @brief Close archive
     */
    void close();
    
    /**
     * @brief Check if archive is open
     */
    bool isOpen() const;
    
    //-------------------------------------------------------------------------
    // LISTING CONTENTS
    //-------------------------------------------------------------------------
    
    /**
     * @brief List all files in archive
     */
    void list() const;
    
    /**
     * @brief Get file entries
     */
    const std::vector<FileEntry>& getFileList() const;
    
    /**
     * @brief Get archive header
     */
    const ArchiveHeader& getHeader() const;
    
    //-------------------------------------------------------------------------
    // EXTRACTION
    //-------------------------------------------------------------------------
    
    /**
     * @brief Extract all files
     * @param outputDir Where to extract (default: current directory)
     * @return true on success
     */
    bool extractAll(const std::string& outputDir = ".");
    
    /**
     * @brief Extract specific file by name
     * @param fileName File to extract
     * @param outputDir Where to extract
     * @return true on success
     */
    bool extractFile(const std::string& fileName, 
                     const std::string& outputDir = ".");
    
    /**
     * @brief Extract multiple specific files
     * @param fileNames List of files to extract
     * @param outputDir Where to extract
     * @return true on success
     */
    bool extractFiles(const std::vector<std::string>& fileNames,
                      const std::string& outputDir = ".");
    
    //-------------------------------------------------------------------------
    // VERIFICATION
    //-------------------------------------------------------------------------
    
    /**
     * @brief Verify archive integrity
     * @return true if archive is intact
     */
    bool verify();
    
    //-------------------------------------------------------------------------
    // PROGRESS CALLBACK
    //-------------------------------------------------------------------------
    
    /**
     * @brief Set progress callback
     */
    void setProgressCallback(
        std::function<void(size_t, size_t, const std::string&)> callback);

private:
    // Progress callback
    std::function<void(size_t, size_t, const std::string&)> progressCallback;
};

//=============================================================================
// UTILITY FUNCTIONS
//=============================================================================

namespace ArchiveUtils {
    /**
     * @brief Check if file exists
     */
    bool fileExists(const std::string& path);
    
    /**
     * @brief Check if path is a directory
     */
    bool isDirectory(const std::string& path);
    
    /**
     * @brief Get file size
     */
    uint64_t getFileSize(const std::string& path);
    
    /**
     * @brief Get file modification time
     */
    time_t getModificationTime(const std::string& path);
    
    /**
     * @brief Convert path to relative
     */
    std::string getRelativePath(const std::string& path, 
                                const std::string& base);
    
    /**
     * @brief Format bytes to human-readable
     * @param bytes Number of bytes
     * @return String like "1.5 MB"
     */
    std::string formatBytes(uint64_t bytes);
    
    /**
     * @brief Format time to string
     */
    std::string formatTime(time_t t);
}

#endif // ARCHIVE_H
