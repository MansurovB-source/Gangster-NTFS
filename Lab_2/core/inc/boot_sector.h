#ifndef SYSTEM_SOFTWARE_BOOT_SECTOR_H
#define SYSTEM_SOFTWARE_BOOT_SECTOR_H

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
    uint16_t end_of_sector_marker;    /* End of boot_sector magic. Always is
					   0xaa55 in little endian. */
/* sizeof() = 512 (0x200) bytes */
} __attribute__((__packed__)) NTFS_BOOT_SECTOR;

#endif //SYSTEM_SOFTWARE_BOOT_SECTOR_H