#ifndef SYSTEM_SOFTWARE_INDEX_ALLOCATION_ATTRIBUTE_H
#define SYSTEM_SOFTWARE_INDEX_ALLOCATION_ATTRIBUTE_H

#include <stdint.h>
#include "mft.h"
#include "index_header.h"

/**
 * struct INDEX_BLOCK - Attribute: Index allocation (0xa0).
 *
 * NOTE: Always non-resident (doesn't make sense to be resident anyway!).
 *
 * This is an array of index blocks. Each index block starts with an
 * INDEX_BLOCK structure containing an index header, followed by a sequence of
 * index entries (INDEX_ENTRY structures), as described by the INDEX_HEADER.
 */
typedef struct {
/*  0	NTFS_RECORD; -- Unfolded here as gcc doesn't like unnamed structs. */
    NTFS_RECORD_TYPES magic;/* Magic is "INDX". */
    uint16_t usa_ofs;        /* See NTFS_RECORD definition. */
    uint16_t usa_count;        /* See NTFS_RECORD definition. */

/*  8*/    uint64_t lsn;        /* $LogFile sequence number of the last
				   modification of this index block. */
/* 16*/    uint64_t index_block_vcn;    /* Virtual cluster number of the index block. */
/* 24*/    INDEX_HEADER index;    /* Describes the following index entries. */
/* sizeof()= 40 (0x28) bytes */
/*
 * When creating the index block, we place the update sequence array at this
 * offset, i.e. before we start with the index entries. This also makes sense,
 * otherwise we could run into problems with the update sequence array
 * containing in itself the last two bytes of a sector which would mean that
 * multi sector transfer protection wouldn't work. As you can't protect data
 * by overwriting it since you then can't get it back...
 * When reading use the data from the ntfs record header.
 */
} __attribute__((__packed__)) INDEX_BLOCK;

typedef INDEX_BLOCK INDEX_ALLOCATION;

#endif //SYSTEM_SOFTWARE_INDEX_ALLOCATION_ATTRIBUTE_H
