#include "../inc/ntfs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define FILE_NAME_MAX_SIZE 255

extern int errno;

static uint8_t file_name_convertor(char *file_name, const INDEX_ENTRY *index_entry);

static int
read_clusters_to_buf(uint8_t **buf, uint64_t *buf_current_size, uint64_t *buf_size, int64_t lcn, uint64_t length,
                     GENERAL_INFORMATION *g_info);

static int parse_data_run(uint64_t offset, GENERAL_INFORMATION *g_info, MAPPING_CHUNK **chunk);

static int init_chunk_data(uint64_t offset, GENERAL_INFORMATION *g_info, MAPPING_CHUNK_DATA **chunk_data);

GENERAL_INFORMATION *init(char *file_name) {
    int err = 0;
    int file_descriptor;
    file_descriptor = open(file_name, O_RDONLY);
    if (file_descriptor == -1) {
        return NULL;
    }

    // TODO check NULL
    NTFS_BOOT_SECTOR *boot_sector = open_NTFS_file_system(file_descriptor);
    GENERAL_INFORMATION *g_info = malloc(sizeof(GENERAL_INFORMATION));
    INODE *root_inode = malloc(sizeof(INODE));

    g_info->bytes_per_sector = boot_sector->bpb.bytes_per_sector;
    g_info->sectors_per_cluster = boot_sector->bpb.sectors_per_cluster;
    g_info->clusters_per_mft_record = boot_sector->clusters_per_mft_record;
    g_info->clusters_per_index_record = boot_sector->clusters_per_index_record;
    g_info->mft_record_size_in_bytes =
            g_info->clusters_per_mft_record * g_info->sectors_per_cluster * g_info->bytes_per_sector;
    g_info->file_descriptor = file_descriptor;
    g_info->cur_node = root_inode;
    g_info->root_node = root_inode;
    g_info->mft_lcn = boot_sector->mft_lcn;
    g_info->block_size_in_bytes =
            g_info->clusters_per_index_record * g_info->sectors_per_cluster * g_info->bytes_per_sector;

    free(boot_sector);

    root_inode->mft_num = FILE_root;
    root_inode->type = MFT_RECORD_IN_USE | MFT_RECORD_IS_DIRECTORY;
    root_inode->parent = root_inode;
    root_inode->next_inode = NULL;

    printf("%s\n", "----------------------------");
    printf("%ld\n", g_info->mft_lcn);
    printf("%d\n", g_info->clusters_per_mft_record);
    printf("%d\n", g_info->clusters_per_index_record);
    printf("%u\n", g_info->bytes_per_sector);
    printf("%u\n", g_info->sectors_per_cluster);
    printf("%lu\n", g_info->mft_record_size_in_bytes);
    printf("%u\n", g_info->block_size_in_bytes);
    printf("%s\n", "----------------------------");

    printf("size ATTR_RECORD is 72 = %lu\n", sizeof(ATTR_RECORD));
    printf("size NTFS_BOOT_SECTOR is 512 = %lu\n", sizeof(NTFS_BOOT_SECTOR));
    printf("size BIOS_PARAMETER_BLOCK is 25 = %lu\n", sizeof(BIOS_PARAMETER_BLOCK));
    printf("sizo FILE_NAME_ATTR is more or equal than 42 = %lu\n", sizeof(FILE_NAME_ATTR));
    printf("size GENERAL_INFORMATION is 35 + 16 = 51 = %lu\n", sizeof(GENERAL_INFORMATION));
    printf("size INDEX_ALLOCATION is 40 = %lu\n", sizeof(INDEX_ALLOCATION));
    printf("size INDEX_ENTRY is 82 = %lu\n", sizeof(INDEX_ENTRY));
    printf("size INDEX_HEADER is 16 = %lu\n", sizeof(INDEX_HEADER));
    printf("size INDEX_ROOT is 32 = %lu\n", sizeof(INDEX_ROOT));
    printf("size INODE is 6 + 3 * 8 = 30 =  %lu\n", sizeof(INODE));
    printf("size INODE is 2 * 8 = 16 = %lu\n", sizeof(FIND_INFO));    
    printf("size MAPPING_CHUNK is 9 + 8 = 17 =  %lu\n", sizeof(MAPPING_CHUNK));
    printf("size MAPPING_CHUNK_DATA is 37 + 24 = 61 =  %lu\n", sizeof(MAPPING_CHUNK_DATA));
    printf("size MFT_RECORD is 48 = %lu\n", sizeof(MFT_RECORD));


    return g_info;
}

NTFS_BOOT_SECTOR *open_NTFS_file_system(int file_descriptor) {
    NTFS_BOOT_SECTOR *boot_sector = malloc(sizeof(NTFS_BOOT_SECTOR));
    pread(file_descriptor, boot_sector, sizeof(NTFS_BOOT_SECTOR), 0);
    uint64_t ntfs_id = 0x202020205346544e;
    if (boot_sector->oem_id == ntfs_id) {
        return boot_sector;
    } else {
        free(boot_sector);
        close(file_descriptor);
        fprintf(stderr, "ERROR: Can't read file system\n");
        return NULL;
    }
}

int read_directory(GENERAL_INFORMATION *g_info, INODE **inode) {
    MFT_RECORD *directory_record = malloc(g_info->mft_record_size_in_bytes);
    uint64_t offset = search_mft_record(g_info, (*inode)->mft_num, &directory_record);
    int err;
    uint8_t file_name_length;
    INDEX_ENTRY *index_entry = NULL;
    INODE *current_inode = *inode;
    if (offset < 0) {
        free(directory_record);
        return -1;
    }

    ATTR_RECORD *attr_index = NULL;
    err = search_attr(g_info, AT_INDEX_ROOT, directory_record, &attr_index);
    if (!attr_index || err == -1) {
        free(directory_record);
        return -1;
    }

    char file_name[FILE_NAME_MAX_SIZE + 1];

    MFT_RECORD *directory_entry = malloc(g_info->mft_record_size_in_bytes);

    // TODO может быть надо изменить структуру ATTR_RECORD
    INDEX_ROOT *index_root = (INDEX_ROOT *) ((uint8_t *) attr_index + attr_index->value_offset);
    uint8_t *index_entry_offset = ((uint8_t *) &index_root->index + index_root->index.entries_offset);
    uint64_t mft_entry_offset;
    uint64_t cnt = 0;
    do {
        index_entry = (INDEX_ENTRY *) index_entry_offset;
        index_entry_offset = ((uint8_t *) index_entry + index_entry->length);
        if (index_entry->key_length) {
            file_name_length = file_name_convertor(file_name, index_entry);
            //TODO проверить имя файла для вывода
            if (file_name[0] != '.' && file_name[0] != '$') {
                current_inode->next_inode = malloc(sizeof(INODE));
                current_inode->next_inode->next_inode = NULL;
                current_inode = current_inode->next_inode;
                current_inode->parent = *inode;
                current_inode->filename = malloc(file_name_length);
                memcpy(current_inode->filename, file_name, file_name_length);

                //TODO мб MFT_MASK ?
                mft_entry_offset = search_mft_record(g_info, index_entry->indexed_file, &directory_entry);
                if (mft_entry_offset == -1) {
                    free(directory_entry);
                    free(directory_record);
                    return -1;
                }
                current_inode->type = directory_entry->flags;
                current_inode->mft_num = directory_entry->mft_record_number;
                cnt++;
            }
        }
    } while (!(index_entry->ie_flags & INDEX_ENTRY_END));

    if (!(index_entry->ie_flags & INDEX_ENTRY_NODE)) {
        if (directory_record->magic == magic_FILE) {
            free(directory_record);
        }
        free(directory_entry);
        return cnt;
    }

    err = search_attr(g_info, AT_INDEX_ALLOCATION, directory_record, &attr_index);

    if (!attr_index || err == -1) {
        free(directory_record);
        free(directory_entry);
        return cnt;
    }

    if (!attr_index->non_resident) {
        free(directory_record);
        free(directory_entry);
        return -1;
    }


    INDEX_ALLOCATION *index_allocation;
    //TODO offset
    uint64_t stream =
            offset + ((uint8_t *) attr_index - (uint8_t *) directory_record) + attr_index->mapping_pairs_offset;
    free(directory_record);
    MAPPING_CHUNK *chunk = malloc(sizeof(MAPPING_CHUNK));
    chunk->current_block = 0;
    //printf("%s\n", "We are in non-resident part");
    parse_data_run(stream, g_info, &chunk);

    do {
        if (chunk->length >= 0) {
            index_allocation = (INDEX_ALLOCATION *) (chunk->buf + (chunk->current_block * g_info->block_size_in_bytes));
            if (index_allocation->magic != magic_INDX) {
                free(directory_entry);
                free(chunk->buf);
                free(chunk);
                return -1;
            }
        } else {
            free(directory_entry);
            free(chunk->buf);
            free(chunk);
            return -1;
        }
        index_entry_offset = ((uint8_t *) &index_allocation->index + index_allocation->index.entries_offset);

        do {
            index_entry = (INDEX_ENTRY *) index_entry_offset;
            index_entry_offset = ((uint8_t *) index_entry + index_entry->length);
            if (index_entry->key_length > 0) {
                file_name_length = file_name_convertor(file_name, index_entry);
                if (file_name[0] != '.' && file_name[0] != '$') {
                    current_inode->next_inode = malloc(sizeof(INODE));
                    current_inode->next_inode->next_inode = NULL;
                    current_inode = current_inode->next_inode;
                    current_inode->parent = *inode;
                    current_inode->filename = malloc(file_name_length);
                    //TODO HMM
                    memcpy(current_inode->filename, &file_name[0], file_name_length);
                    mft_entry_offset = search_mft_record(g_info, index_entry->indexed_file, &directory_entry);
                    if (mft_entry_offset == -1) {
                        free(directory_entry);
                        free(chunk->buf);
                        free(chunk);
                    }
                    //printf("current_inode file_name: %s\n", current_inode->filename);
                    current_inode->type = directory_entry->flags;
                    current_inode->mft_num = index_entry->indexed_file;
                    cnt++;
                }
            }

        } while (index_entry_offset < (chunk->buf + chunk->length) && !(index_entry->ie_flags & INDEX_ENTRY_END));
        chunk->current_block++;
    } while (chunk->current_block < (chunk->length / g_info->block_size_in_bytes));
    free(directory_entry);
    free(chunk->buf);
    free(chunk);
    //puts("GOOD BUY READDIR\n");
    return cnt;
}

uint64_t search_mft_record(GENERAL_INFORMATION *g_info, uint32_t mft_num, MFT_RECORD **mft_record) {
    uint64_t offset;
    uint64_t LCN;
    LCN = g_info->mft_lcn + (mft_num * g_info->clusters_per_mft_record);
    offset = ((mft_num * g_info->mft_record_size_in_bytes) % (g_info->sectors_per_cluster * g_info->bytes_per_sector)) +
             (LCN * g_info->sectors_per_cluster * g_info->bytes_per_sector);
    while ((*mft_record)->magic != magic_FILE || (*mft_record)->mft_record_number != mft_num) {
        pread(g_info->file_descriptor, (*mft_record), g_info->mft_record_size_in_bytes, offset);
        offset += g_info->mft_record_size_in_bytes;
    }
    if ((*mft_record)->magic != magic_FILE) {
        return -1;
    }

    return offset - g_info->mft_record_size_in_bytes;

}

int search_attr(GENERAL_INFORMATION *g_info, uint32_t type, MFT_RECORD *mft_record, ATTR_RECORD **attr_record) {
    if (!mft_record || type == AT_END) {
        attr_record = NULL;
        return -1;
    }

    *attr_record = (ATTR_RECORD *) ((uint8_t *) mft_record + mft_record->attrs_offset);

    // TODO не уверен на счет sizeof(ATTR_RECORD). Думаю можно убрать.
    void *end = mft_record + g_info->mft_record_size_in_bytes - sizeof(ATTR_RECORD);

    while ((*attr_record)->type != AT_END && (*attr_record)->type != type && (void *) (*attr_record) < end) {
        *attr_record = (ATTR_RECORD *) ((uint8_t *) (*attr_record) + (*attr_record)->length);
    }

    if ((*attr_record)->type == AT_END || (void *) (*attr_record) >= end) {
        attr_record = NULL;
    }

    return 0;
}

int read_file_data(GENERAL_INFORMATION *g_info, INODE *inode, MAPPING_CHUNK_DATA **chunk_data) {
    if (inode->type & MFT_RECORD_IS_DIRECTORY) {
        return -1;
    }

    MFT_RECORD *mft_file_record = malloc(g_info->mft_record_size_in_bytes);
    uint64_t offset = search_mft_record(g_info, inode->mft_num, &mft_file_record);

    if (offset == -1) {
        free(mft_file_record);
        return -1;
    }

    ATTR_RECORD *attr_data = NULL;
    int err = search_attr(g_info, AT_DATA, mft_file_record, &attr_data);
    if (err == -1) {
        free(mft_file_record);
        return -1;
    }

     //printf("%s\n", "read file data(): before malloc MAPPING_CHUNK_DATA");
     // printf("%lu\n", sizeof(MAPPING_CHUNK_DATA));
    (*chunk_data) = malloc(sizeof(MAPPING_CHUNK_DATA));
    if (!attr_data->non_resident) {
        (*chunk_data)->resident = 1;
        //printf("%s\n", "read file data(): after malloc MAPPING_CHUNK_DATA");
        (*chunk_data)->length = attr_data->value_length;
        //printf("lenght of data: %lu\n", (*chunk_data)->length);
        //printf("%s\n", "before buf malloc");
        (*chunk_data)->buf = malloc(attr_data->value_length);
        //printf("%s\n", "after buf malloc"); 
        memcpy((*chunk_data)->buf, (uint8_t *) attr_data + attr_data->value_offset, (*chunk_data)->length);
        (*chunk_data)->lcns = NULL;
        (*chunk_data)->lengths = NULL;
        //printf("%s\n", "read file data(): at the end of resident part");
    } else {
        (*chunk_data)->resident = 0;
        uint64_t stream =
                offset + ((uint8_t *) attr_data - (uint8_t *) mft_file_record) + attr_data->mapping_pairs_offset;
        init_chunk_data(stream, g_info, &(*chunk_data));
        (*chunk_data)->length = attr_data->data_size;
        (*chunk_data)->buf = malloc(g_info->block_size_in_bytes);
        (*chunk_data)->blocks_count = 0;
    }

    //printf("%s\n", "before mft_file_record");
    free(mft_file_record);
    printf("%s\n", "after free mft_file_record");
    return 0;
}

int read_block_file(GENERAL_INFORMATION *g_info, MAPPING_CHUNK_DATA **chunk_data) {
    uint64_t buf_current_size = 0;
    uint64_t buf_size = g_info->block_size_in_bytes;

    if ((*chunk_data)->cur_lcn == (*chunk_data)->lcn_count) {
        (*chunk_data)->signal = 1;
        return 1;
    }
    int64_t LCN = (*chunk_data)->lcns[(*chunk_data)->cur_lcn] + (*chunk_data)->cur_block;
    int err = read_clusters_to_buf(&(*chunk_data)->buf, &buf_current_size, &buf_size, LCN, 1, g_info);
    if (err == -1) {
        (*chunk_data)->signal = -1;
        return -1;
    }
    (*chunk_data)->cur_block++;
    (*chunk_data)->blocks_count++;
    (*chunk_data)->signal = 0;
    return 0;

}

void free_inode(INODE *inode) {
    INODE *tmp;

    while (inode != NULL) {
        tmp = inode;
        inode = inode->next_inode;
        tmp->next_inode = NULL;
        tmp->parent = NULL;
        free(tmp->filename);
        free(tmp);
    }
}

int free_data_chunk(MAPPING_CHUNK_DATA *chunk_data) {
    if (chunk_data->buf != NULL) {
        free(chunk_data->buf);
    }

    if (chunk_data->lcns != NULL) {
        free(chunk_data->lcns);
    }

    if (chunk_data->lengths != NULL) {
        free(chunk_data->lengths);
    }

    // printf("in the free_data_chuk sizeof mapping_chunk_data: %ld\n", sizeof(MAPPING_CHUNK_DATA));
    free(chunk_data);
    return 0;
}


static uint8_t file_name_convertor(char *file_name, const INDEX_ENTRY *index_entry) {
    const uint16_t *filename = index_entry->key.file_name.file_name;
    uint8_t filename_length = index_entry->key.file_name.file_name_length;

    for (uint8_t i = 0; i < filename_length; i++) {
        file_name[i] = (char) filename[i];
    }
    file_name[filename_length] = '\0';

    return filename_length + 1;
}

static int
read_clusters_to_buf(uint8_t **buf, uint64_t *buf_current_size, uint64_t *buf_size, int64_t lcn, uint64_t length,
                     GENERAL_INFORMATION *g_info) {
    //TODO maybe need to change a way to calculate length or g_info
    length = length * (g_info->sectors_per_cluster * g_info->bytes_per_sector);
    uint64_t offset = lcn * (g_info->sectors_per_cluster * g_info->bytes_per_sector);

    if (length > (*buf_size - *buf_current_size)) {
        *buf = realloc(*buf, *buf_size + g_info->block_size_in_bytes);
        if (*buf == NULL) {
            return -1;
        } else {
            *buf_size += g_info->block_size_in_bytes;
        }
    }
    pread(g_info->file_descriptor, *buf + *buf_current_size, length, (long) offset);
    *buf_current_size += length;
    return 0;
}

static int parse_data_run(uint64_t offset, GENERAL_INFORMATION *g_info, MAPPING_CHUNK **chunk) {
    uint8_t *run_list = malloc(g_info->block_size_in_bytes);
    pread(g_info->file_descriptor, run_list, g_info->block_size_in_bytes, (long) offset);
    uint8_t *ptr_run_list = run_list;

    int64_t LCN = 0;

    uint8_t *buf = malloc(g_info->block_size_in_bytes);
    uint64_t buf_size = g_info->block_size_in_bytes;
    uint64_t buf_current_size = 0;

    uint8_t data_run_offset_size; //размер поля смещения
    uint8_t data_run_length_size; //размер поля длины

    uint64_t data_run_length = 0; //на выходе, распакованное значение числа кластеров
    int64_t data_run_offset = 0; //на выходе, распакованное значение кластерного смещения

    do {
        //из старшего полубайта считаем размер поля смещения
        data_run_offset_size = (*ptr_run_list >> 4) & 0x0F;
        //из младшего размер поля смещения
        data_run_length_size = *ptr_run_list & 0x0F;

        //указатель на сами данные
        ptr_run_list++;
        uint8_t i = 0;

        //цикл распаковки длины отрезка, с каждой итерацией значение сдвигается на i-байт и
        //прибавляется с длиной
        for (i = 0; i < data_run_length_size; i++) {
            data_run_length += *ptr_run_list << (i << 3);
            ptr_run_list++;
        }

        /* NTFS 3+ sparse files, если файл разряжен */
        if (data_run_offset_size == 0) {
            data_run_offset = -1;
            //TODO maybe break
        } else {
            //цикл распаковки смещения
            for (i = 0; i < data_run_offset_size - 1; i++) {
                data_run_offset += *ptr_run_list << (i << 3);
                ptr_run_list++;
            }
            //последний байт может быть знаковым, поэтому он обрабатывается отдельно
            data_run_offset = ((char) (*(ptr_run_list++)) << (i << 3)) + data_run_offset;
        }

        LCN += data_run_offset;
        int err = read_clusters_to_buf(&buf, &buf_current_size, &buf_size, LCN, data_run_length, g_info);
        if (err == -1) {
            free(ptr_run_list);
            free(buf);
            return -1;
        }
    } while (*ptr_run_list);

    free(run_list);
    (*chunk)->buf = buf;
    (*chunk)->length = buf_current_size;
    return 0;
}

static int init_chunk_data(uint64_t offset, GENERAL_INFORMATION *g_info, MAPPING_CHUNK_DATA **chunk_data) {
    puts("HELLO FROM CHUNK_DATA");
    uint8_t *run_list = malloc(g_info->block_size_in_bytes);
    pread(g_info->file_descriptor, run_list, g_info->block_size_in_bytes, offset);
    uint8_t *ptr_run_list = run_list;

    int64_t LCN = 0;

    (*chunk_data)->lcns = malloc(sizeof(int64_t) * 16);
    (*chunk_data)->lengths = malloc(sizeof(uint64_t) * 16);
    int buf_size = 16;
    int cur_size = 0;

    uint8_t data_run_offset_size; //размер поля смещения
    uint8_t data_run_length_size; //размер поля длины

    uint64_t data_run_length = 0; //на выходе, распакованное значение числа кластеров
    int64_t data_run_offset = 0; //на выходе, распакованное значение кластерного смещения

    do {
        //из старшего полубайта считаем размер поля смещения
        data_run_offset_size = (*ptr_run_list >> 4) & 0x0F;
        //из младшего размер поля смещения
        data_run_length_size = *ptr_run_list & 0x0F;

        //указатель на сами данные
        ptr_run_list++;
        uint8_t i = 0;

        //цикл распаковки длины отрезка, с каждой итерацией значение сдвигается на i-байт и
        //прибавляется с длиной
        for (i = 0; i < data_run_length_size; i++) {
            data_run_length += *ptr_run_list << (i << 3);
            ptr_run_list++;
        }

        /* NTFS 3+ sparse files, если файл разряжен */
        if (data_run_offset_size == 0) {
            data_run_offset = -1;
            //TODO maybe break
        } else {
            //цикл распаковки смещения
            for (i = 0; i < data_run_offset_size - 1; i++) {
                data_run_offset += *ptr_run_list << (i << 3);
                ptr_run_list++;
            }
            //последний байт может быть знаковым, поэтому он обрабатывается отдельно
            data_run_offset = ((char) (*(ptr_run_list++)) << (i << 3)) + data_run_offset;
        }

        if (cur_size == buf_size) {
            (*chunk_data)->lcns = realloc((*chunk_data)->lcns, 2 * buf_size);
            (*chunk_data)->lengths = realloc((*chunk_data)->lengths, 2 * buf_size);
            buf_size *= 2;
        }
        LCN += data_run_offset;

        (*chunk_data)->lcns[cur_size] = LCN;
        (*chunk_data)->lengths[cur_size] = data_run_length;
        cur_size++;
    } while (*ptr_run_list);

    (*chunk_data)->cur_lcn = 0;
    (*chunk_data)->lcn_count = cur_size;
    (*chunk_data)->cur_block = 0;
    free(run_list);
    return 0;
}