#ifndef SYSTEM_SOFTWARE_GENERAL_INFORMATION_H
#define SYSTEM_SOFTWARE_GENERAL_INFORMATION_H

#include <stdint.h>
#include "inode.h"

/**
 * Basic information collected from different structures to facilitate the work
 */
typedef struct {
    int64_t mft_lcn;            /* Cluster location of mft data. */
    int8_t clusters_per_mft_record;    /* Mft record size in clusters. */
    int8_t clusters_per_index_record;    /* Index block size in clusters. */
    uint64_t clusters;    /* Total number of clusters */
    uint16_t bytes_per_sector;        /* Size of a sector in bytes. */
    uint8_t sectors_per_cluster;    /* Size of a cluster in sectors. */

    uint64_t mft_record_size_in_bytes;
    uint16_t block_size_in_bytes;

    INODE *cur_node;
    INODE *root_node;

    int file_descriptor;
} __attribute__((__packed__)) GENERAL_INFORMATION;

#endif //SYSTEM_SOFTWARE_GENERAL_INFORMATION_H
