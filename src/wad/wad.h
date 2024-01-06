#ifndef WAD_H
#define WAD_H

#include <cstdint>

struct wadhead_t {
    char magic[4];      // "WAD2", Name of the new WAD format
    int32_t numentries; // Number of entries
    int32_t diroffset;  // Position of WAD directory in file
};

struct wadentry_t {
    int32_t offset;    // Position of the entry in WAD
    int32_t dsize;     // Size of the entry in WAD file
    int32_t size;      // Size of the entry in memory
    char type;         // Type of entry
    char cmprs;        // Compression. 0 if none.
    int16_t dummy;     // Not used
    char name[16];     // 1 to 16 characters, '\0'-padded
};

struct dmiptex_t {
    char name[16];
    uint32_t width;
    uint32_t height;
    int32_t offsets[4];
};

#endif // WAD_H
