#include "archive.h"
#include <iostream>
#include <vector>

void printUsage() {
    std::cout << "Huffman Archiver - Usage:\n\n";
    std::cout << "  Create archive:\n";
    std::cout << "    huffarc create <archive.huff> <files/dirs...>\n\n";
    std::cout << "  Extract archive:\n";
    std::cout << "    huffarc extract <archive.huff> [-o output_dir]\n\n";
    std::cout << "  List contents:\n";
    std::cout << "    huffarc list <archive.huff>\n\n";
    std::cout << "  Verify archive:\n";
    std::cout << "    huffarc verify <archive.huff>\n\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage();
        return 1;
    }
    
    std::string command = argv[1];
    std::string archivePath = argv[2];
    
    if (command == "create") {
        if (argc < 4) {
            std::cerr << "Error: No files specified\n";
            return 1;
        }
        
        ArchiveCreator creator(archivePath);
        
        creator.setProgressCallback([](size_t current, size_t total, const std::string& file) {
            std::cout << "[" << current << "/" << total << "] " << file << "\n";
        });
        
        for (int i = 3; i < argc; i++) {
            creator.addPath(argv[i]);
        }
        
        std::cout << "Creating archive with " << creator.getFileCount() << " items...\n";
        
        if (creator.create()) {
            std::cout << "\nSuccess!\n";
            creator.printSummary();
        } else {
            std::cerr << "Failed to create archive\n";
            return 1;
        }
        
    } else if (command == "extract") {
        std::string outputDir = ".";
        
        if (argc > 3 && std::string(argv[3]) == "-o" && argc > 4) {
            outputDir = argv[4];
        }
        
        ArchiveExtractor extractor(archivePath);
        
        if (!extractor.open()) {
            std::cerr << "Failed to open archive\n";
            return 1;
        }
        
        extractor.setProgressCallback([](size_t current, size_t total, const std::string& file) {
            std::cout << "[" << current << "/" << total << "] " << file << "\n";
        });
        
        std::cout << "Extracting to: " << outputDir << "\n";
        
        if (extractor.extractAll(outputDir)) {
            std::cout << "\nExtraction complete!\n";
        } else {
            std::cerr << "Extraction failed\n";
            return 1;
        }
        
    } else if (command == "list") {
        ArchiveExtractor extractor(archivePath);
        
        if (!extractor.open()) {
            std::cerr << "Failed to open archive\n";
            return 1;
        }
        
        extractor.list();
        
    } else if (command == "verify") {
        ArchiveExtractor extractor(archivePath);
        
        if (!extractor.open()) {
            std::cerr << "Failed to open archive\n";
            return 1;
        }
        
        if (extractor.verify()) {
            std::cout << "Archive is valid!\n";
        } else {
            std::cerr << "Archive verification failed!\n";
            return 1;
        }
        
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        printUsage();
        return 1;
    }
    
    return 0;
}
