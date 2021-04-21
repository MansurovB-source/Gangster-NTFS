//
// Created by Behruz Mansurov on 21.04.2021.
//

#ifndef SYSTEM_SOFTWARE_NTFS_H
#define SYSTEM_SOFTWARE_NTFS_H

#include <stdint.h>


/**
 * struct BIOS_PARAMETER_BLOCK - BIOS parameter block (bpb) structure.
 */
typedef struct {
    uint16_t bytes_per_sector;        /* Size of a sector in bytes. */
    uint8_t sectors_per_cluster;    /* Size of a cluster in sectors. */
    uint16_t reserved_sectors;        /* zero */
    uint8_t fats;            /* zero */
    uint16_t root_entries;        /* zero */
    uint16_t sectors;            /* zero */
    uint8_t media_type;            /* 0xf8 = hard disk */
    uint16_t sectors_per_fat;        /* zero */
/*0x0d*/uint16_t sectors_per_track;        /* Required to boot Windows. */
/*0x0f*/uint16_t heads;            /* Required to boot Windows. */
/*0x11*/uint32_t hidden_sectors;        /* Offset to the start of the partition
					   relative to the disk in sectors.
					   Required to boot Windows. */
/*0x15*/uint32_t large_sectors;        /* zero */
/* sizeof() = 25 (0x19) bytes */
} __attribute__((__packed__)) BIOS_PARAMETER_BLOCK;


/**
 * struct NTFS_BOOT_SECTOR - NTFS boot sector structure.
 */
typedef struct {
    uint8_t jump[3];            /* Irrelevant (jump to boot up code).*/
    uint64_t oem_id;            /* Magic "NTFS    ". */
/*0x0b*/BIOS_PARAMETER_BLOCK bpb;    /* See BIOS_PARAMETER_BLOCK. */
    uint8_t physical_drive;        /* 0x00 floppy, 0x80 hard disk */
    uint8_t current_head;        /* zero */
    uint8_t extended_boot_signature;    /* 0x80 */
    uint8_t reserved2;            /* zero */
/*0x28*/uint64_t number_of_sectors;        /* Number of sectors in volume. Gives
					   maximum volume size of 2^63 sectors.
					   Assuming standard sector size of 512
					   bytes, the maximum byte size is
					   approx. 4.7x10^21 bytes. (-; */
    uint64_t mft_lcn;            /* Cluster location of mft data. */
    uint64_t mftmirr_lcn;        /* Cluster location of copy of mft. */
    int8_t clusters_per_mft_record;    /* Mft record size in clusters. */
    uint8_t reserved0[3];        /* zero */
    int8_t clusters_per_index_record;    /* Index block size in clusters. */
    uint8_t reserved1[3];        /* zero */
    uint64_t volume_serial_number;    /* Irrelevant (serial number). */
    uint32_t checksum;            /* Boot sector checksum. */
/*0x54*/uint8_t bootstrap[426];        /* Irrelevant (boot up code). */
    uint16_t end_of_sector_marker;    /* End of bootsector magic. Always is
					   0xaa55 in little endian. */
/* sizeof() = 512 (0x200) bytes */
} __attribute__((__packed__)) NTFS_BOOT_SECTOR;

/**
 * enum NTFS_SYSTEM_FILES - System files mft record numbers.
 *
 * All these files are always marked as used in the bitmap attribute of the
 * mft; presumably in order to avoid accidental allocation for random other
 * mft records. Also, the sequence number for each of the system files is
 * always equal to their mft record number and it is never modified.
 */
typedef enum {
    FILE_MFT = 0,    /* Master file table (mft).*/
    FILE_MFTMirr = 1,    /* Mft mirror: copy of first four mft records in data attribute.*/
    FILE_LogFile = 2,    /* Journalling log in data attribute. */
    FILE_Volume = 3,    /* Volume name attribute and volume information attribute (flags and ntfs version).*/
    FILE_AttrDef = 4,    /* Array of attribute definitions in data attribute. */
    FILE_root = 5,    /* Root directory. */
    FILE_Bitmap = 6,    /* Allocation bitmap of all clusters (lcns) in data attribute. */
    FILE_Boot = 7,    /* Boot sector (always at cluster 0) in data attribute. */
    FILE_BadClus = 8,    /* Contains all bad clusters in the non-resident data attribute. */
    FILE_Secure = 9,    /* Shared security descriptors in data attribute and two indexes into the descriptors */
    FILE_UpCase = 10,    /* Uppercase equivalents of all 65536 Unicode characters in data attribute. */
    FILE_Extend = 11,    /* Directory containing other system files (eg. $ObjId, $Quota, $Reparse and $UsnJrnl). Thisis new to NTFS3.0. */
    FILE_reserved12 = 12,    /* Reserved for future use (records 12-15). */
    FILE_reserved13 = 13,
    FILE_reserved14 = 14,
    FILE_mft_data = 15,    /* Reserved for first extent of $MFT:$DATA */
    FILE_first_user = 16,    /* First user file, used as test limit for whether to allow opening a file or not. */
} NTFS_SYSTEM_FILES;


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
typedef enum {
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
} ATTR_TYPES;


/**
 * enum NTFS_RECORD_TYPES -
 *
 * Magic identifiers present at the beginning of all ntfs record containing
 * records (like mft records for example).
 */
typedef enum {
    /* Found in $MFT/$DATA. */
    magic_FILE = 0x454c4946, /* Mft entry. */
    magic_INDX = 0x58444e49, /* Index buffer. */
    magic_HOLE = 0x454c4f48, /* ? (NTFS 3.0+?) */

    /* Found in $LogFile/$DATA. */
    magic_RSTR = 0x52545352, /* Restart page. */
    magic_RCRD = 0x44524352, /* Log record page. */

    /* Found in $LogFile/$DATA.  (May be found in $MFT/$DATA, also?) */
    magic_CHKD = 0x444b4843, /* Modified by chkdsk. */

    /* Found in all ntfs record containing records. */
    magic_BAAD = 0x44414142, /* Failed multi sector transfer was detected. */

    /*
     * Found in $LogFile/$DATA when a page is full or 0xff bytes and is
     * thus not initialized.  User has to initialize the page before using
     * it.
     */
    magic_empty = 0xffffffff,/* Record is empty and has to be initialized before it can be used. */
} NTFS_RECORD_TYPES;


/**
 * enum MFT_RECORD_FLAGS -
 *
 * These are the so far known MFT_RECORD_* flags (16-bit) which contain
 * information about the mft record in which they are present.
 *
 * MFT_RECORD_IS_4 exists on all $Extend sub-files.
 * It seems that it marks it is a metadata file with MFT record >24, however,
 * it is unknown if it is limited to metadata files only.
 *
 * MFT_RECORD_IS_VIEW_INDEX exists on every metafile with a non directory
 * index, that means an INDEX_ROOT and an INDEX_ALLOCATION with a name other
 * than "$I30". It is unknown if it is limited to metadata files only.
 */
typedef enum {
    MFT_RECORD_IN_USE = 0x0001,
    MFT_RECORD_IS_DIRECTORY = 0x0002,
    MFT_RECORD_IS_4 = 0x0004,
    MFT_RECORD_IS_VIEW_INDEX = 0x0008,
    MFT_REC_SPACE_FILLER = 0xffff, /* Just to make flags
						     16-bit. */
} __attribute__((__packed__)) MFT_RECORD_FLAGS;


/**
 * struct MFT_RECORD - An MFT record layout (NTFS 3.1+)
 *
 * The mft record header present at the beginning of every record in the mft.
 * This is followed by a sequence of variable length attribute records which
 * is terminated by an attribute of type AT_END which is a truncated attribute
 * in that it only consists of the attribute type code AT_END and none of the
 * other members of the attribute structure are present.
 */
typedef struct {
/*Ofs*/
/*  0	NTFS_RECORD; -- Unfolded here as gcc doesn't like unnamed structs. */
    NTFS_RECORD_TYPES magic;/* Usually the magic is "FILE". */
    uint16_t usa_ofs;        /* See NTFS_RECORD definition above. */
    uint16_t usa_count;        /* See NTFS_RECORD definition above. */

/*  8*/    uint64_t lsn;        /* $LogFile sequence number for this record. Changed every time the record is modified. */
/* 16*/    uint16_t sequence_number;    /* Number of times this mft record has been reused. (See description for MFT_REF above.)
/* 18*/    uint16_t link_count;        /* Number of hard links, i.e. the number of directory entries referencing this record.
/* 20*/    uint16_t attrs_offset;    /* Byte offset to the first attribute in this mft record from the start of the mft record.
/* 22*/    MFT_RECORD_FLAGS flags;    /* Bit array of MFT_RECORD_FLAGS. When a file is deleted, the MFT_RECORD_IN_USE flag is set to zero. */
/* 24*/    uint32_t bytes_in_use;    /* Number of bytes used in this mft record.
/* 28*/    uint32_t bytes_allocated;    /* Number of bytes allocated for this mft record. This should be equal to the mft record size. */
/* 32*/    uint64_t base_mft_record; /* This is zero for base mft records.
/* 40*/    uint16_t next_attr_instance; /* The instance number that will be assigned to the next attribute added to this mft record. */

/* The below fields are specific to NTFS 3.1+ (Windows XP and above): */
/* 42*/ uint16_t reserved;        /* Reserved/alignment. */
/* 44*/ uint32_t mft_record_number;    /* Number of this mft record. */
/* sizeof() = 48 bytes */
/*
 * When (re)using the mft record, we place the update sequence array at this
 * offset, i.e. before we start with the attributes. This also makes sense,
 * otherwise we could run into problems with the update sequence array
 * containing in itself the last two bytes of a sector which would mean that
 * multi sector transfer protection wouldn't work. As you can't protect data
 * by overwriting it since you then can't get it back...
 * When reading we obviously use the data from the ntfs record header.
 */
} __attribute__((__packed__)) MFT_RECORD;


/**
 * enum ATTR_FLAGS - Attribute flags (16-bit).
 */
typedef enum {
    ATTR_IS_COMPRESSED = 0x0001,
    ATTR_COMPRESSION_MASK = 0x00ff,  /* Compression
						method mask. Also, first
						illegal value. */
    ATTR_IS_ENCRYPTED = 0x4000,
    ATTR_IS_SPARSE = 0x8000,
} __attribute__((__packed__)) ATTR_FLAGS;


/**
 * enum RESIDENT_ATTR_FLAGS - Flags of resident attributes (8-bit).
 */
typedef enum {
    RESIDENT_ATTR_IS_INDEXED = 0x01, /* Attribute is referenced in an index
					    (has implications for deleting and
					    modifying the attribute). */
} __attribute__((__packed__)) RESIDENT_ATTR_FLAGS;


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

typedef ATTR_RECORD ATTR_REC;


/**
 * enum FILE_NAME_TYPE_FLAGS - Possible namespaces for filenames in ntfs.
 * (8-bit).
 */
typedef enum {
    FILE_NAME_POSIX = 0x00,
    /* This is the largest namespace. It is case sensitive and
       allows all Unicode characters except for: '\0' and '/'.
       Beware that in WinNT/2k files which eg have the same name
       except for their case will not be distinguished by the
       standard utilities and thus a "del filename" will delete
       both "filename" and "fileName" without warning. */
    FILE_NAME_WIN32 = 0x01,
    /* The standard WinNT/2k NTFS long filenames. Case insensitive.
       All Unicode chars except: '\0', '"', '*', '/', ':', '<',
       '>', '?', '\' and '|'. Further, names cannot end with a '.'
       or a space. */
    FILE_NAME_DOS = 0x02,
    /* The standard DOS filenames (8.3 format). Uppercase only.
       All 8-bit characters greater space, except: '"', '*', '+',
       ',', '/', ':', ';', '<', '=', '>', '?' and '\'. */
    FILE_NAME_WIN32_AND_DOS = 0x03,
    /* 3 means that both the Win32 and the DOS filenames are
       identical and hence have been saved in this single filename
       record. */
} __attribute__((__packed__)) FILE_NAME_TYPE_FLAGS;


/**
 * enum FILE_ATTR_FLAGS - File attribute flags (32-bit).
 */
typedef enum {
    /*
     * These flags are only present in the STANDARD_INFORMATION attribute
     * (in the field file_attributes).
     */
    FILE_ATTR_READONLY = (0x00000001),
    FILE_ATTR_HIDDEN = (0x00000002),
    FILE_ATTR_SYSTEM = (0x00000004),
    /* Old DOS volid. Unused in NT.	= (0x00000008), */

    FILE_ATTR_DIRECTORY = (0x00000010),
    /* FILE_ATTR_DIRECTORY is not considered valid in NT. It is reserved
       for the DOS SUBDIRECTORY flag. */
    FILE_ATTR_ARCHIVE = 0x00000020,
    FILE_ATTR_DEVICE = 0x00000040,
    FILE_ATTR_NORMAL = 0x00000080,

    FILE_ATTR_TEMPORARY = 0x00000100,
    FILE_ATTR_SPARSE_FILE = 0x00000200,
    FILE_ATTR_REPARSE_POINT = 0x00000400,
    FILE_ATTR_COMPRESSED = 0x00000800,

    FILE_ATTR_OFFLINE = 0x00001000,
    FILE_ATTR_NOT_CONTENT_INDEXED = 0x00002000,
    FILE_ATTR_ENCRYPTED = 0x00004000,

    FILE_ATTR_VALID_FLAGS = 0x00007fb7,
    /* FILE_ATTR_VALID_FLAGS masks out the old DOS VolId and the
       FILE_ATTR_DEVICE and preserves everything else. This mask
       is used to obtain all flags that are valid for reading. */
    FILE_ATTR_VALID_SET_FLAGS = 0x000031a7,

    /* FILE_ATTR_VALID_SET_FLAGS masks out the old DOS VolId, the
       FILE_ATTR_DEVICE, FILE_ATTR_DIRECTORY, FILE_ATTR_SPARSE_FILE,
       FILE_ATTR_REPARSE_POINT, FILE_ATRE_COMPRESSED and FILE_ATTR_ENCRYPTED
       and preserves the rest. This mask is used to to obtain all flags that
       are valid for setting. */

    /**
     * FILE_ATTR_I30_INDEX_PRESENT - Is it a directory?
     *
     * This is a copy of the MFT_RECORD_IS_DIRECTORY bit from the mft
     * record, telling us whether this is a directory or not, i.e. whether
     * it has an index root attribute named "$I30" or not.
     *
     * This flag is only present in the FILE_NAME attribute (in the
     * file_attributes field).
     */
    FILE_ATTR_I30_INDEX_PRESENT = (0x10000000),

    /**
     * FILE_ATTR_VIEW_INDEX_PRESENT - Does have a non-directory index?
     *
     * This is a copy of the MFT_RECORD_IS_VIEW_INDEX bit from the mft
     * record, telling us whether this file has a view index present (eg.
     * object id index, quota index, one of the security indexes and the
     * reparse points index).
     *
     * This flag is only present in the $STANDARD_INFORMATION and
     * $FILE_NAME attributes.
     */
    FILE_ATTR_VIEW_INDEX_PRESENT = (0x20000000),
} __attribute__((__packed__)) FILE_ATTR_FLAGS;

/**
 * struct FILE_NAME_ATTR - Attribute: Filename (0x30).
 *
 * NOTE: Always resident.
 * NOTE: All fields, except the parent_directory, are only updated when the
 *	 filename is changed. Until then, they just become out of sync with
 *	 reality and the more up to date values are present in the standard
 *	 information attribute.
 * NOTE: There is conflicting information about the meaning of each of the time
 *	 fields but the meaning as defined below has been verified to be
 *	 correct by practical experimentation on Windows NT4 SP6a and is hence
 *	 assumed to be the one and only correct interpretation.
 */
typedef struct {
/*hex ofs*/
/*  0*/    uint64_t parent_directory;    /* Directory this filename is
					   referenced from. */
/*  8*/    uint64_t creation_time;        /* Time file was created. */
/* 10*/    uint64_t last_data_change_time;    /* Time the data attribute was last
					   modified. */
/* 18*/    uint64_t last_mft_change_time;    /* Time this mft record was last
					   modified. */
/* 20*/    uint64_t last_access_time;        /* Last time this mft record was
					   accessed. */
/* 28*/    uint64_t allocated_size;        /* Byte size of on-disk allocated space
					   for the data attribute.  So for
					   normal $DATA, this is the
					   allocated_size from the unnamed
					   $DATA attribute and for compressed
					   and/or sparse $DATA, this is the
					   compressed_size from the unnamed
					   $DATA attribute.  NOTE: This is a
					   multiple of the cluster size. */
/* 30*/    uint64_t data_size;            /* Byte size of actual data in data
					   attribute. */
/* 38*/    FILE_ATTR_FLAGS file_attributes;    /* Flags describing the file. */
/* 3c*/    union {
        /* 3c*/    struct {
            /* 3c*/    uint16_t packed_ea_size;    /* Size of the buffer needed to
						   pack the extended attributes
						   (EAs), if such are present.*/
            /* 3e*/    uint16_t reserved;        /* Reserved for alignment. */
        } __attribute__((__packed__));
        /* 3c*/    uint32_t reparse_point_tag;        /* Type of reparse point,
						   present only in reparse
						   points and only if there are
						   no EAs. */
    } __attribute__((__packed__));
/* 40*/    uint8_t file_name_length;            /* Length of file name in
						   (Unicode) characters. */
/* 41*/    FILE_NAME_TYPE_FLAGS file_name_type;    /* Namespace of the file name.*/
/* 42*/    uint16_t file_name[0];            /* File name in Unicode. */
} __attribute__((__packed__)) FILE_NAME_ATTR;


/**
 * enum INDEX_HEADER_FLAGS - Index header flags (8-bit).
 */
typedef enum {
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
} __attribute__((__packed__)) INDEX_HEADER_FLAGS;


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
typedef enum {
    COLLATION_BINARY = 0,
    COLLATION_FILE_NAME = 1,
    COLLATION_UNICODE_STRING = 2,
    COLLATION_NTOFS_ULONG = 16,
    COLLATION_NTOFS_SID = 17,
    COLLATION_NTOFS_SECURITY_HASH = 18,
    COLLATION_NTOFS_ULONGS = 19,
} COLLATION_RULES;


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
/*  8*/    uint32_t index_block_size;        /* Size of index block in bytes (in
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


/**
 * enum INDEX_ENTRY_FLAGS - Index entry flags (16-bit).
 */
typedef enum {
    INDEX_ENTRY_NODE = 1, /* This entry contains a
					sub-node, i.e. a reference to an index
					block in form of a virtual cluster
					number (see below). */
    INDEX_ENTRY_END = 2, /* This signifies the last
					entry in an index block. The index
					entry does not represent a file but it
					can point to a sub-node. */
    INDEX_ENTRY_SPACE_FILLER = 0xffff, /* Just to force 16-bit width. */
} __attribute__((__packed__)) INDEX_ENTRY_FLAGS;


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
						   indexes. */
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
				    and the optional subnode VCN. See below. */
/* 10*/ uint16_t key_length;         /* Byte size of the key value, which is in the
				    index entry. It follows field reserved. Not
				    multiple of 8-bytes. */
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


typedef struct {
    uint32_t mft_no;
    char *filename;
    uint16_t type;
    struct ntfs_inode *parent; /*parent directory*/
    struct ntfs_inode *next_inode; /*connected list of files and dirs in dir or next inode in connected list*/
} __attribute__((__packed__)) INODE;


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

    struct ntfs_inode *cur_node;
    struct ntfs_inode *root_node;

    int file_descriptor;
} __attribute__((__packed__)) GENERAL_INFORMATION;

#endif //SYSTEM_SOFTWARE_NTFS_H
