#ifndef SYSTEM_SOFTWARE_ATTRIBUTE_H
#define SYSTEM_SOFTWARE_ATTRIBUTE_H

#include <stdint.h>

/**
 * enum ATTR_TYPES - System defined attributes (32-bit).
 *
 * Each attribute type has a corresponding attribute name (Unicode string of
 * maximum 64 character length) as described by the attribute definitions
 * present in the data attribute of the $AttrDef system file.
 *
 * On NTFS 3.0 volumes the names are just as the types are named in the below
 * enum exchanging AT_ for the dollar sign ($). If that isn't a revealing
 * choice of symbol... (-;
 */
enum {
    AT_UNUSED = 0,
    AT_STANDARD_INFORMATION = 0x10,
    AT_ATTRIBUTE_LIST = 0x20,
    AT_FILE_NAME = 0x30,
    AT_OBJECT_ID = 0x40,
    AT_SECURITY_DESCRIPTOR = 0x50,
    AT_VOLUME_NAME = 0x60,
    AT_VOLUME_INFORMATION = 0x70,
    AT_DATA = 0x80,
    AT_INDEX_ROOT = 0x90,
    AT_INDEX_ALLOCATION = 0xa0,
    AT_BITMAP = 0xb0,
    AT_REPARSE_POINT = 0xc0,
    AT_EA_INFORMATION = 0xd0,
    AT_EA = 0xe0,
    AT_PROPERTY_SET = 0xf0,
    AT_LOGGED_UTILITY_STREAM = 0x100,
    AT_FIRST_USER_DEFINED_ATTRIBUTE = 0x1000,
    AT_END = 0xffffffff,
};

typedef uint32_t ATTR_TYPES;

/**
 * enum ATTR_FLAGS - Attribute flags (16-bit).
 */
enum {
    ATTR_IS_COMPRESSED = 0x0001,
    ATTR_COMPRESSION_MASK = 0x00ff,  /* Compression
						method mask. Also, first
						illegal value. */
    ATTR_IS_ENCRYPTED = 0x4000,
    ATTR_IS_SPARSE = 0x8000,
} __attribute__((__packed__));

typedef uint16_t ATTR_FLAGS;

/**
 * enum RESIDENT_ATTR_FLAGS - Flags of resident attributes (8-bit).
 */
enum {
    RESIDENT_ATTR_IS_INDEXED = 0x01, /* Attribute is referenced in an index
					    (has implications for deleting and
					    modifying the attribute). */
} __attribute__((__packed__));

typedef uint8_t RESIDENT_ATTR_FLAGS;

/**
 * struct ATTR_RECORD - Attribute record header.
 *
 * Always aligned to 8-byte boundary.
 */
typedef struct {
/*Ofs*/
/*  0*/    ATTR_TYPES type;    /* The (32-bit) type of the attribute. */
/*  4*/    uint32_t length;        /* Byte size of the resident part of the attribute (aligned to 8-byte boundary). Used to get to the next attribute. */
/*  8*/    uint8_t non_resident;    /* If 0, attribute is resident. If 1, attribute is non-resident. */
/*  9*/    uint8_t name_length;        /* Unicode character size of name of attribute. 0 if unnamed. */
/* 10*/    uint16_t name_offset;    /* If name_length != 0, the byte offset to the beginning of the name from the attribute record. */
/* 12*/    ATTR_FLAGS flags;    /* Flags describing the attribute. */
/* 14*/    uint16_t instance;        /* The instance of this attribute record. This number is unique within this mft record. */
/* 16*/    union {
        /* Resident attributes. */
        struct {
/* 16 */        uint32_t value_length; /* Byte size of attribute value. */
/* 20 */        uint16_t value_offset; /* Byte offset of the attribute value from the start of the attribute record. */
/* 22 */        RESIDENT_ATTR_FLAGS resident_flags; /* See above. */
/* 23 */        int8_t reservedR;        /* Reserved/alignment to 8-byte boundary. */
/* 24 */        void *resident_end[0]; /* Use offsetof(ATTR_RECORD, resident_end) to get size of a resident attribute. */
        } __attribute__((__packed__));
        /* Non-resident attributes. */
        struct {
/* 16*/            uint64_t lowest_vcn;    /* Lowest valid virtual cluster number
				for this portion of the attribute value or
				0 if this is the only extent (usually the
				case). - Only when an attribute list is used
				does lowest_vcn != 0 ever occur. */
/* 24*/            uint64_t highest_vcn; /* Highest valid vcn of this extent of
				the attribute value. - Usually there is only one
				portion, so this usually equals the attribute
				value size in clusters minus 1. Can be -1 for
				zero length files. Can be 0 for "single extent"
				attributes. */
/* 32*/            uint16_t mapping_pairs_offset; /* Byte offset from the
				beginning of the structure to the mapping pairs
				array which contains the mappings between the
				vcns and the logical cluster numbers (lcns).
				When creating, place this at the end of this
				record header aligned to 8-byte boundary. */
/* 34*/            uint8_t compression_unit; /* The compression unit expressed
				as the log to the base 2 of the number of
				clusters in a compression unit. 0 means not
				compressed. (This effectively limits the
				compression unit size to be a power of two
				clusters.) WinNT4 only uses a value of 4. */
/* 35*/            uint8_t reserved1[5];    /* Align to 8-byte boundary. */
/* The sizes below are only used when lowest_vcn is zero, as otherwise it would
   be difficult to keep them up-to-date.*/
/* 40*/            uint64_t allocated_size;    /* Byte size of disk space
				allocated to hold the attribute value. Always
				is a multiple of the cluster size. When a file
				is compressed, this field is a multiple of the
				compression block size (2^compression_unit) and
				it represents the logically allocated space
				rather than the actual on disk usage. For this
				use the compressed_size (see below). */
/* 48*/            uint64_t data_size;    /* Byte size of the attribute
				value. Can be larger than allocated_size if
				attribute value is compressed or sparse. */
/* 56*/            uint64_t initialized_size;    /* Byte size of initialized
				portion of the attribute value. Usually equals
				data_size. */
/* 64 */        void *non_resident_end[0]; /* Use offsetof(ATTR_RECORD,
						      non_resident_end) to get
						      size of a non resident
						      attribute. */
/* sizeof(uncompressed attr) = 64*/
/* 64*/            uint64_t compressed_size;    /* Byte size of the attribute
				value after compression. Only present when
				compressed. Always is a multiple of the
				cluster size. Represents the actual amount of
				disk space being used on the disk. */
/* 72 */        void *compressed_end[0];
            /* Use offsetof(ATTR_RECORD, compressed_end) to
               get size of a compressed attribute. */
/* sizeof(compressed attr) = 72*/
        } __attribute__((__packed__));
    } __attribute__((__packed__));
} __attribute__((__packed__)) ATTR_RECORD;

#endif //SYSTEM_SOFTWARE_ATTRIBUTE_H
