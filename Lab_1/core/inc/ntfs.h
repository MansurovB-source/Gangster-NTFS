#ifndef SYSTEM_SOFTWARE_NTFS_H
#define SYSTEM_SOFTWARE_NTFS_H

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "boot_sector.h"
#include "mft.h"
#include "attribute.h"
#include "index_root_attribute.h"
#include "index_header.h"
#include "index_entry.h"
#include "index_allocation_attribute.h"
#include "file_name_attribute.h"

#include "general_information.h"
#include "inode.h"
#include "mapping_chunk.h"

GENERAL_INFORMATION *init(char *file_name);

NTFS_BOOT_SECTOR *open_NTFS_file_system(int file_descriptor);

int read_directory(GENERAL_INFORMATION *g_info, INODE **inode);

uint64_t search_mft_record(GENERAL_INFORMATION *g_info, uint32_t mft_num, MFT_RECORD **mft_record);

int search_attr(GENERAL_INFORMATION *g_info, uint32_t type, MFT_RECORD *mft_record, ATTR_RECORD **attr_record);

int read_file_data(GENERAL_INFORMATION *g_info, INODE *inode, MAPPING_CHUNK_DATA **chunk_data);

int read_block_file(GENERAL_INFORMATION *g_info, MAPPING_CHUNK_DATA **chunk_data);

void free_inode(INODE *inode);

int free_data_chunk(MAPPING_CHUNK_DATA *chunk_data);

#endif //SYSTEM_SOFTWARE_NTFS_H
