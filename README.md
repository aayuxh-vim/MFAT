# MFAT — Multi-File Archival Tool

A file archiver and compressor built from scratch in C++17. MFAT uses **Huffman coding** to compress files and directories into a single `.huff` archive, with CRC32 integrity verification.

## Features

- **Huffman compression** — adaptive tree built from file contents for optimal encoding
- **Multi-file & directory support** — recursively archives entire directory trees
- **CRC32 integrity checks** — every file is checksummed on creation and verified on extraction
- **Archive operations** — create, extract, list, and verify
- **Progress reporting** — real-time feedback during archival and extraction
- **Cross-platform** — builds on Linux, macOS, and Windows

## Building

### Requirements

- C++17-compatible compiler (GCC 8+, Clang 7+, MSVC 2017+)
- `make` or CMake 3.10+

### Using Make

```bash
make          # build
make clean    # clean build artifacts
```

### Using CMake

```bash
mkdir build && cd build
cmake ..
make
```

The binary `huffarc` will be created in the project root (Make) or `build/` directory (CMake).

## Usage

```
huffarc <command> <archive> [options]
```

### Create an archive

```bash
# Archive a single file
huffarc create backup.huff myfile.txt

# Archive multiple files and directories
huffarc create backup.huff file1.txt file2.txt docs/
```

### Extract an archive

```bash
# Extract to current directory
huffarc extract backup.huff

# Extract to a specific directory
huffarc extract backup.huff -o output_dir/
```

### List archive contents

```bash
huffarc list backup.huff
```

```
Archive: backup.huff
Files: 3

Name                                        Original  Compressed   Ratio
------------------------------------------------------------------------
readme.txt                                       512         287   56.1%
notes.txt                                        164          87   53.0%
data.csv                                        2048         943   46.0%
```

### Verify archive integrity

```bash
huffarc verify backup.huff
```

```
Verifying archive...
OK: readme.txt
OK: notes.txt
OK: data.csv
Archive is intact!
Archive is valid!
```

## Archive Format (`.huff`)

```
┌──────────────────────┐
│    Header (HUFF)     │  Magic bytes, version, file count, offsets
├──────────────────────┤
│    Huffman Tree      │  Serialized compression tree
├──────────────────────┤
│   Compressed Data    │  File contents, compressed per-file
├──────────────────────┤
│   Directory Table    │  File entries (paths, sizes, CRC32s, offsets)
├──────────────────────┤
│      Footer          │  Archive CRC32 + magic verification
└──────────────────────┘
```

## Project Structure

```
MFAT/
├── include/
│   ├── archive.h       # Archive creator/extractor, file entries, header/footer
│   ├── bitstream.h     # Bit-level I/O for Huffman encoding
│   ├── crc32.h         # CRC32 checksum calculation
│   └── huffman.h       # Huffman tree, encoder/decoder
├── src/
│   ├── archive.cpp     # Archive operations implementation
│   ├── bitstream.cpp   # Bitstream read/write
│   ├── crc32.cpp       # CRC32 lookup table + computation
│   ├── huffman.cpp     # Huffman coding algorithm
│   └── main.cpp        # CLI entry point
├── CMakeLists.txt
├── Makefile
└── README.md
```

## License

This project is provided as-is for educational and personal use.
