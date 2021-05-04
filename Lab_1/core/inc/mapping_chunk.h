#ifndef SYSTEM_SOFTWARE_MAPPING_CHUNK_H
#define SYSTEM_SOFTWARE_MAPPING_CHUNK_H

#include <stdint.h>

// Custom structs for easier work

// Using for reading non-resident attribute index_allocation (for reading index_entries)
typedef struct {
    uint64_t length;
    uint8_t *buf;
    uint8_t current_block;
} __attribute__((__packed__)) MAPPING_CHUNK;


// Using for reading non-resident attribute data (data chunk of file)
typedef struct {
    uint8_t resident;
    uint64_t length;
    uint64_t blocks_count;
    int cur_lcn;
    int lcn_count;
    uint64_t cur_block;
    int signal;

    uint8_t *buf;
    int64_t *lcns;
    uint64_t *lengths;
} __attribute__((__packed__)) MAPPING_CHUNK_DATA;

#endif //SYSTEM_SOFTWARE_MAPPING_CHUNK_H