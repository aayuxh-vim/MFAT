cmake_minimum_required(VERSION 3.10)
project(HuffmanArchiver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/huffman.cpp
    src/bitstream.cpp
    src/crc32.cpp
    src/archive.cpp
    src/main.cpp
)

set(HEADERS
    include/huffman.h
    include/bitstream.h
    include/crc32.h
    include/archive.h
)

include_directories(include)

add_executable(huffarc ${SOURCES} ${HEADERS})

if(WIN32)
    # Windows specific
else()
    # Linux/Mac
    find_package(Threads REQUIRED)
    target_link_libraries(huffarc Threads::Threads)
endif()
