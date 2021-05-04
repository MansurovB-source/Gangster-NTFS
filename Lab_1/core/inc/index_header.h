#ifndef SYSTEM_SOFTWARE_INDEX_HEADER_H
#define SYSTEM_SOFTWARE_INDEX_HEADER_H

#include <stdint.h>

/**
 * enum INDEX_HEADER_FLAGS - Index header flags (8-bit).
 */
enum {
    /* When index header is in an index root attribute: */
    SMALL_INDEX = 0, /* The index is small enough to fit inside the
				index root attribute and there is no index
				allocation attribute present. */
    LARGE_INDEX = 1, /* The index is too large to fit in the index
				root attribute and/or an index allocation
				attribute is present. */
    /*
     * When index header is in an index block, i.e. is part of index
     * allocation attribute:
     */
    LEAF_NODE = 0, /* This is a leaf node, i.e. there are no more
				nodes branching off it. */
    INDEX_NODE = 1, /* This node indexes other nodes, i.e. is not a
				leaf node. */
    NODE_MASK = 1, /* Mask for accessing the *_NODE bits. */
} __attribute__((__packed__));

typedef uint8_t INDEX_HEADER_FLAGS;


/**
 * struct INDEX_HEADER -
 *
 * This is the header for indexes, describing the INDEX_ENTRY records, which
 * follow the INDEX_HEADER. Together the index header and the index entries
 * make up a complete index.
 *
 * IMPORTANT NOTE: The offset, length and size structure members are counted
 * relative to the start of the index header structure and not relative to the
 * start of the index root or index allocation structures themselves.
 */
typedef struct {
/*  0*/    uint32_t entries_offset;    /* Byte offset from the INDEX_HEADER to first
				   INDEX_ENTRY, aligned to 8-byte boundary.  */
/*  4*/    uint32_t index_length;    /* Data size in byte of the INDEX_ENTRY's,
				   including the INDEX_HEADER, aligned to 8. */
/*  8*/    uint32_t allocated_size;    /* Allocated byte size of this index (block),
				   multiple of 8 bytes. See more below.      */
    /*
       For the index root attribute, the above two numbers are always
       equal, as the attribute is resident and it is resized as needed.

       For the index allocation attribute, the attribute is not resident
       and the allocated_size is equal to the index_block_size specified
       by the corresponding INDEX_ROOT attribute minus the INDEX_BLOCK
       size not counting the INDEX_HEADER part (i.e. minus -24).
     */
/* 12*/    INDEX_HEADER_FLAGS ih_flags;    /* Bit field of INDEX_HEADER_FLAGS.  */
/* 13*/    uint8_t reserved[3];            /* Reserved/align to 8-byte boundary.*/
/* sizeof() == 16 */
} __attribute__((__packed__)) INDEX_HEADER;

#endif //SYSTEM_SOFTWARE_INDEX_HEADER_H