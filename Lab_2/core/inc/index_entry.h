#ifndef SYSTEM_SOFTWARE_INDEX_ENTRY_H
#define SYSTEM_SOFTWARE_INDEX_ENTRY_H

#include <stdint.h>
#include "file_name_attribute.h"

/**
 * enum INDEX_ENTRY_FLAGS - Index entry flags (16-bit).
 */
enum {
    INDEX_ENTRY_NODE = 1, /* This entry contains a
					sub-node, i.e. a reference to an index
					block in form of a virtual cluster
					number (see below). */
    INDEX_ENTRY_END = 2, /* This signifies the last
					entry in an index block. The index
					entry does not represent a file but it
					can point to a sub-node. */
    INDEX_ENTRY_SPACE_FILLER = 0xffff, /* Just to force 16-bit width. */
} __attribute__((__packed__));

typedef uint16_t INDEX_ENTRY_FLAGS;


/**
 * struct INDEX_ENTRY - This is an index entry.
 *
 * A sequence of such entries follows each INDEX_HEADER structure. Together
 * they make up a complete index. The index follows either an index root
 * attribute or an index allocation attribute.
 *
 * NOTE: Before NTFS 3.0 only filename attributes were indexed.
 */
typedef struct {
/*  0	INDEX_ENTRY_HEADER; -- Unfolded here as gcc dislikes unnamed structs. */
    union {        /* Only valid when INDEX_ENTRY_END is not set. */
        uint64_t indexed_file;        /* The mft reference of the file
						   described by this index
						   entry. Used for directory
						   indexes.

						   Адресс MFT файла (или номер ?)
						   */
        struct { /* Used for views/indexes to find the entry's data. */
            uint16_t data_offset;    /* Data byte offset from this
						   INDEX_ENTRY. Follows the
						   index key. */
            uint16_t data_length;    /* Data length in bytes. */
            uint32_t reservedV;        /* Reserved (zero). */
        } __attribute__((__packed__));
    } __attribute__((__packed__));
/*  8*/ uint16_t length;         /* Byte size of this index entry, multiple of
				    8-bytes. Size includes INDEX_ENTRY_HEADER
				    and the optional subnode VCN. See below.

				    смещение следующего элемента, относительно текущего
				    */
/* 10*/ uint16_t key_length;         /* Byte size of the key value, which is in the
				    index entry. It follows field reserved. Not
				    multiple of 8-bytes.

				    длина атрибута $FILE_NAME
				    */
/* 12*/    INDEX_ENTRY_FLAGS ie_flags; /* Bit field of INDEX_ENTRY_* flags. */
/* 14*/    uint16_t reserved;         /* Reserved/align to 8-byte boundary. */
/*	End of INDEX_ENTRY_HEADER */
/* 16*/    union {        /* The key of the indexed attribute. NOTE: Only present
			   if INDEX_ENTRY_END bit in flags is not set. NOTE: On
			   NTFS versions before 3.0 the only valid key is the
			   FILE_NAME_ATTR. On NTFS 3.0+ the following
			   additional index keys are defined: */
        FILE_NAME_ATTR file_name;/* $I30 index in directories. */

        //--------------TODO define
//        SII_INDEX_KEY sii;    /* $SII index in $Secure. */
//        SDH_INDEX_KEY sdh;    /* $SDH index in $Secure. */
//        GUID object_id;        /* $O index in FILE_Extend/$ObjId: The
//					   object_id of the mft record found in
//					   the data part of the index. */
//        REPARSE_INDEX_KEY reparse;    /* $R index in
//						   FILE_Extend/$Reparse. */
//        SID sid;        /* $O index in FILE_Extend/$Quota:
//					   SID of the owner of the user_id. */
        //--------------
        uint32_t owner_id;        /* $Q index in FILE_Extend/$Quota:
					   user_id of the owner of the quota
					   control entry in the data part of
					   the index. */
    } __attribute__((__packed__)) key;
    /* The (optional) index data is inserted here when creating.
    int64_t vcn;	   If INDEX_ENTRY_NODE bit in ie_flags is set, the last
               eight bytes of this index entry contain the virtual
               cluster number of the index block that holds the
               entries immediately preceding the current entry.

               If the key_length is zero, then the vcn immediately
               follows the INDEX_ENTRY_HEADER.

               The address of the vcn of "ie" INDEX_ENTRY is given by
               (char*)ie + le16_to_cpu(ie->length) - sizeof(VCN)
    */
} __attribute__((__packed__)) INDEX_ENTRY;

#endif //SYSTEM_SOFTWARE_INDEX_ENTRY_H