#ifndef SYSTEM_SOFTWARE_INODE_H
#define SYSTEM_SOFTWARE_INODE_H

#include <stdint.h>

// Custom inode for easier work
struct inode {
    uint32_t mft_num; // number of mft record
    char *filename; // file name or directory name
    uint16_t type; // file or directory
    struct inode *parent; // parent directory
    struct inode *next_inode; //connected list of files and dirs in dir
} __attribute__((__packed__));

typedef struct inode INODE;

// used for finding directory and file
typedef struct {
    INODE *start; // start_dir or root
    INODE *result; // finish file or dir
} FIND_INFO;

#endif //SYSTEM_SOFTWARE_INODE_H
