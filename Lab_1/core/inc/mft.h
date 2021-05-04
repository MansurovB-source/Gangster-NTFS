#ifndef SYSTEM_SOFTWARE_MFT_H
#define SYSTEM_SOFTWARE_MFT_H

#include <stdint.h>
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
 * enum NTFS_RECORD_TYPES -
 *
 * Magic identifiers present at the beginning of all ntfs record containing
 * records (like mft records for example).
 */
enum {
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
};

typedef uint32_t NTFS_RECORD_TYPES;


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
enum {
    MFT_RECORD_IN_USE = 0x0001,
    MFT_RECORD_IS_DIRECTORY = 0x0002,
    MFT_RECORD_IS_4 = 0x0004,
    MFT_RECORD_IS_VIEW_INDEX = 0x0008,
    MFT_REC_SPACE_FILLER = 0xffff, /* Just to make flags
						     16-bit. */
} __attribute__((__packed__));

typedef uint16_t MFT_RECORD_FLAGS;


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
/* 20*/    uint16_t attrs_offset;    /* Byte offset  to the first attribute in this mft record from the start of the mft record.
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

#endif //SYSTEM_SOFTWARE_MFT_H