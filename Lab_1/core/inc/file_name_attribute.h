#ifndef SYSTEM_SOFTWARE_FILE_NAME_ATTRIBUTE_H
#define SYSTEM_SOFTWARE_FILE_NAME_ATTRIBUTE_H

#include <stdint.h>

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

#endif //SYSTEM_SOFTWARE_FILE_NAME_ATTRIBUTE_H
