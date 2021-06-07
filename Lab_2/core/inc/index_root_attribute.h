#ifndef SYSTEM_SOFTWARE_INDEX_ROOT_ATTRIBUTE_H
#define SYSTEM_SOFTWARE_INDEX_ROOT_ATTRIBUTE_H

#include <stdint.h>
#include "attribute.h"
#include "index_header.h"

/**
 * enum COLLATION_RULES - The collation rules for sorting views/indexes/etc
 * (32-bit).
 *
 * COLLATION_BINARY - Collate by binary compare where the first byte is most
 *	significant.
 * COLLATION_FILE_NAME - Collate Unicode strings by comparing their 16-bit
 *	coding units, primarily ignoring case using the volume's $UpCase table,
 *	but falling back to a case-sensitive comparison if the names are equal
 *	ignoring case.
 * COLLATION_UNICODE_STRING - TODO: this is not yet implemented and still needs
 *	to be properly documented --- is it really the same as
 *	COLLATION_FILE_NAME?
 * COLLATION_NTOFS_ULONG - Sorting is done according to ascending le32 key
 *	values. E.g. used for $SII index in FILE_Secure, which sorts by
 *	security_id (le32).
 * COLLATION_NTOFS_SID - Sorting is done according to ascending SID values.
 *	E.g. used for $O index in FILE_Extend/$Quota.
 * COLLATION_NTOFS_SECURITY_HASH - Sorting is done first by ascending hash
 *	values and second by ascending security_id values. E.g. used for $SDH
 *	index in FILE_Secure.
 * COLLATION_NTOFS_ULONGS - Sorting is done according to a sequence of ascending
 *	le32 key values. E.g. used for $O index in FILE_Extend/$ObjId, which
 *	sorts by object_id (16-byte), by splitting up the object_id in four
 *	le32 values and using them as individual keys. E.g. take the following
 *	two security_ids, stored as follows on disk:
 *		1st: a1 61 65 b7 65 7b d4 11 9e 3d 00 e0 81 10 42 59
 *		2nd: 38 14 37 d2 d2 f3 d4 11 a5 21 c8 6b 79 b1 97 45
 *	To compare them, they are split into four le32 values each, like so:
 *		1st: 0xb76561a1 0x11d47b65 0xe0003d9e 0x59421081
 *		2nd: 0xd2371438 0x11d4f3d2 0x6bc821a5 0x4597b179
 *	Now, it is apparent why the 2nd object_id collates after the 1st: the
 *	first le32 value of the 1st object_id is less than the first le32 of
 *	the 2nd object_id. If the first le32 values of both object_ids were
 *	equal then the second le32 values would be compared, etc.
 */
enum {
    COLLATION_BINARY = 0,
    COLLATION_FILE_NAME = 1,
    COLLATION_UNICODE_STRING = 2,
    COLLATION_NTOFS_ULONG = 16,
    COLLATION_NTOFS_SID = 17,
    COLLATION_NTOFS_SECURITY_HASH = 18,
    COLLATION_NTOFS_ULONGS = 19,
};

typedef uint32_t COLLATION_RULES;


/**
 * struct INDEX_ROOT - Attribute: Index root (0x90).
 *
 * NOTE: Always resident.
 *
 * This is followed by a sequence of index entries (INDEX_ENTRY structures)
 * as described by the index header.
 *
 * When a directory is small enough to fit inside the index root then this
 * is the only attribute describing the directory. When the directory is too
 * large to fit in the index root, on the other hand, two additional attributes
 * are present: an index allocation attribute, containing sub-nodes of the B+
 * directory tree (see below), and a bitmap attribute, describing which virtual
 * cluster numbers (vcns) in the index allocation attribute are in use by an
 * index block.
 *
 * NOTE: The root directory (FILE_root) contains an entry for itself. Other
 * directories do not contain entries for themselves, though.
 */
typedef struct {
/*  0*/    ATTR_TYPES type;        /* Type of the indexed attribute. Is
					   $FILE_NAME for directories, zero
					   for view indexes. No other values
					   allowed. */
/*  4*/    COLLATION_RULES collation_rule;    /* Collation rule used to sort the
					   index entries. If type is $FILE_NAME,
					   this must be COLLATION_FILE_NAME. */
/*  8*/    uint32_t index_block_size;        /* Size of index block i   n bytes (in
					   the index allocation attribute). */
/* 12*/    int8_t clusters_per_index_block;    /* Size of index block in clusters (in
					   the index allocation attribute), when
					   an index block is >= than a cluster,
					   otherwise sectors per index block. */
/* 13*/    uint8_t reserved[3];            /* Reserved/align to 8-byte boundary. */
/* 16*/    INDEX_HEADER index;        /* Index header describing the
					   following index entries. */
/* sizeof()= 32 bytes */
} __attribute__((__packed__)) INDEX_ROOT;

#endif //SYSTEM_SOFTWARE_INDEX_ROOT_ATTRIBUTE_H