//
// Created by Behruz Mansurov on 21.04.2021.
//

#include "ntfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

NTFS_BOOT_SECTOR *open_NTFS_file_system(int file_descriptor) {
    NTFS_BOOT_SECTOR *boot_sector = malloc(sizeof(NTFS_BOOT_SECTOR));
    read(file_descriptor, boot_sector, sizeof(NTFS_BOOT_SECTOR));
    if (strcmp((const char *) boot_sector->oem_id, "NTFS    ") == 0) {
        return boot_sector;
    } else {
        free(boot_sector);
        close(file_descriptor);
        fprintf(stderr, "ERROR: Can't read file system");
        return NULL;
    }
}

// TODO need to change (in some cases, this works)
GENERAL_INFORMATION *init(char *file_name) {
    int file_descriptor;
    if ((file_descriptor = open(file_name, O_RDONLY)) == -1) {
        return NULL;
    }

    NTFS_BOOT_SECTOR *boot_sector = open_NTFS_file_system(file_descriptor);
    GENERAL_INFORMATION *g_info = malloc(sizeof(GENERAL_INFORMATION));
    INODE *root_inode = malloc(sizeof(INODE));

    g_info->bytes_per_sector = boot_sector->bpb.bytes_per_sector;
    g_info->sectors_per_cluster = boot_sector->bpb.sectors_per_cluster;
    g_info->clusters_per_mft_record = boot_sector->clusters_per_mft_record;
    g_info->clusters_per_index_record = boot_sector->clusters_per_index_record;
    g_info->clusters_per_index_record = boot_sector->number_of_sectors;
    g_info->file_descriptor = file_descriptor;
    g_info->cur_node = root_inode;
    g_info->root_node = root_inode;

    free(boot_sector);

    root_inode->mft_no = FILE_root;
    root_inode->type = MFT_RECORD_IN_USE | MFT_RECORD_IS_DIRECTORY;
    root_inode->parent = root_inode;
    root_inode->next_inode = NULL;

    return g_info;
}

