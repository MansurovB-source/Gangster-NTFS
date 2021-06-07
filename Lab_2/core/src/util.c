#include <stdbool.h>
#include "../inc/util.h"

static int count_nodes(char *path) {
    char path_buf[512];
    strcpy(path_buf, path);
    int result = 0;
    char sep[2] = "/";
    char *sub_dir = strtok(path_buf, sep);
    while (sub_dir != NULL) {
        result++;
        sub_dir = strtok(NULL, sep);
    }
    return result;
}

static int find_node_by_name(GENERAL_INFORMATION *g_info, char *path, INODE **start_node, FIND_INFO **result) {
    INODE *result_node = malloc(sizeof(INODE));
    INODE *head;
    memcpy(result_node, *start_node, sizeof(INODE));
    result_node->filename = NULL;
    INODE *start_result_node = result_node;
    bool found = false;
    char sep[2] = "/";
    char path_buf[512];
    strcpy(path_buf, path);
    int count = count_nodes(path);
    char *sub_dir = strtok(path_buf, sep);
    int err;
    while (sub_dir != NULL) {
        found = false;
        if (!(result_node->type & MFT_RECORD_IS_DIRECTORY)) {
            free_inode(start_result_node);
            return -1;
        }
        err = read_directory(g_info, &result_node);
        if (err == -1) {
            free_inode(start_result_node);
            return -1;
        }
        head = result_node->next_inode;

        while (head != NULL) {
            if (strcmp(head->filename, sub_dir) == 0) {
                INODE *tmp = malloc(sizeof(INODE));
                memcpy(tmp, head, sizeof(INODE));
                tmp->filename = malloc(strlen(head->filename) + 1);
                strcpy(tmp->filename, head->filename);
                free_inode(result_node->next_inode);
                result_node->next_inode = tmp;
                found = true;
                break;
            }
            head = head->next_inode;
        }
        if (!found) {
            free_inode(start_result_node);
            return -1;
        }

        result_node = result_node->next_inode;
        result_node->next_inode = NULL;

        if (count == 1) {
            *result = malloc(sizeof(FIND_INFO));
            (*result)->start = start_result_node;
            (*result)->result = result_node;
            return 0;
        } else {
            count--;
        }
        sub_dir = strtok(NULL, sep);
    }
    return -1;
}

static int copy(GENERAL_INFORMATION *g_info, INODE *node, char *to_path) {
    char *node_path = malloc(strlen(to_path) + strlen((node->filename)) + 2);
    strcpy(node_path, to_path);
    strcat(node_path, "/");
    strcat(node_path, node->filename);

    if (!(node->type & MFT_RECORD_IS_DIRECTORY)) {
        int fd = open(node_path, O_CREAT | O_WRONLY | O_TRUNC);
        if (fd == -1) {
            free(node_path);
            close(fd);
            return -1;
        }

        MAPPING_CHUNK_DATA *chunk_data = NULL;
        int err = read_file_data(g_info, node, &chunk_data);
        if (err == -1) {
            free(node_path);
            close(fd);
        }
        if (chunk_data->resident) {
            pwrite(fd, chunk_data->buf, chunk_data->length, 0);
            close(fd);
            free(node_path);
            free_data_chunk(chunk_data);
            return 1;
        } else {
            int64_t offset = 0;
            uint64_t size;
            while (read_block_file(g_info, &chunk_data) == 0) {
                if (chunk_data->blocks_count * g_info->block_size_in_bytes > chunk_data->length) {
                    size = chunk_data->length - ((chunk_data->blocks_count - 1) * g_info->block_size_in_bytes);
                    offset += pwrite(fd, chunk_data->buf, size, offset);
                    break;
                } else {
                    size = g_info->block_size_in_bytes;
                }
                offset += pwrite(fd, chunk_data->buf, size, offset);
            }
            close(fd);
            int result = chunk_data->signal;
            free_data_chunk(chunk_data);
            free(node_path);
            return result;
        }
    } else {
        if (mkdir(node_path, 00777) != 0) {
            free(node_path);
            return -1;
        }
        INODE *read_node = malloc(sizeof(INODE));
        memcpy(read_node, node, sizeof(INODE));
        read_node->filename = NULL;
        read_node->next_inode = NULL;
        int err = read_directory(g_info, &read_node);
        if (err == -1) {
            free(node_path);
            free_inode(read_node);
            return -1;
        }
        INODE *tmp = read_node->next_inode;
        while (tmp != NULL) {
            if (copy(g_info, tmp, node_path) == -1) {
                free(node_path);
                free_inode(read_node);
                return -1;
            }
            tmp = tmp->next_inode;
        }
        free(node_path);
        free_inode(read_node);
    }
    return 0;
}

char *pwd(const GENERAL_INFORMATION *const g_info) {
    uint64_t size = 2;   // for 0x20 and 0x00
    uint16_t current_size = 256;
    uint32_t name_length;
    char *result = malloc(size);
    result[0] = '\0';
    INODE *current_inode = g_info->root_node->next_inode;
    if (current_inode == NULL) {
        strcat((char *) result, "/");

    }

    while (current_inode != NULL) {
        name_length = strlen(current_inode->filename);
        if ((size += (name_length + 1)) > current_size) {
            result = realloc(result, size);
            current_size = size;
        }
        strcat(result, "/");
        strcat(result, current_inode->filename);
        current_inode = current_inode->next_inode;
    }
    strcat(result, " ");
    return result;
}

char *cd(GENERAL_INFORMATION *g_info, char *path) {
    char *output = malloc(16);
    output[0] = '\0';
    char *message;

    if (strcmp(path, ".") == 0) {
        return output;
    } else if (strcmp(path, "..") == 0) {
        if (g_info->cur_node->mft_num == FILE_root) {
            return output;
        }
        INODE *tmp = g_info->cur_node->parent;
        free_inode(g_info->cur_node);
        g_info->cur_node = tmp;
        g_info->cur_node->next_inode = NULL;
        return output;
    }

    FIND_INFO *result;
    int err;
    if (path[0] == '/') {
        err = find_node_by_name(g_info, path, &(g_info->root_node), &result);
        if (err == -1) {
            goto error;
        }
        if (result->result->type & MFT_RECORD_IS_DIRECTORY) {
            g_info->root_node->next_inode = result->start->next_inode;
            result->start->next_inode->parent = g_info->root_node;
            g_info->cur_node = result->result;
            result->start->next_inode = NULL;
            free_inode((result->start));
            free(result);
            return output;
        } else {
            goto is_file;
        }
    } else {
        err = find_node_by_name(g_info, path, &(g_info->cur_node), &result);
        if (err == -1) goto error;
        if (result->result->type & MFT_RECORD_IS_DIRECTORY) {
            g_info->cur_node->next_inode = result->start->next_inode;
            result->start->next_inode->parent = g_info->cur_node;
            g_info->cur_node = result->result;
            result->start->next_inode = NULL;
            free_inode((result->start));
            free(result);
            return output;
        } else {
            goto is_file;
        }
    }

    error:
    message = "No such file or directory\n";
    sprintf(output, "%s", message);
    return output;


    is_file:
    free_inode((result->start));
    free(result);
    message = "It is not directory\n";
    sprintf(output, "%s", message);
    return output;
}

LS_INFO *ls(GENERAL_INFORMATION *g_info, char *path) {
    FIND_INFO *find_result;
    int error = 0;
    bool current_path = false;
    bool parent_path = false;
    if (path == NULL || strcmp(path, ".") == 0) {
        find_result = malloc(sizeof(FIND_INFO));
        find_result->result = g_info->cur_node;
        current_path = true;
        goto parse;
    }

    if (strcmp(path, "..") == 0) {
        find_result = malloc(sizeof(FIND_INFO));
        find_result->result = g_info->cur_node->parent;
        current_path = true;
        parent_path = true;
        goto parse;
    }

    if (path[0] == '/') {
        error = find_node_by_name(g_info, path, &g_info->root_node, &find_result);
    } else {
        error = find_node_by_name(g_info, path, &g_info->cur_node, &find_result);
    }

    parse:
    if (error != -1) {
        int err = read_directory(g_info, &(find_result->result));
        if (err == -1) {
            return NULL;
        }
        INODE *tmp = find_result->result->next_inode;
//        char *result[256];
//        char *output = malloc(err * 256);
//        output[0] = '\0';

        LS_INFO *first = malloc(sizeof(LS_INFO));
        first->filename = NULL;
        first->next = NULL;
        LS_INFO *current = first;

        while (tmp != NULL) {
            current->next = malloc(sizeof(LS_INFO));
            current = current->next;
            if (tmp->type & MFT_RECORD_IS_DIRECTORY) {
                //sprintf((char *) result, "DIR:\t%s\n", tmp->filename);
                current->type = 1;
                current->filename = malloc(strlen(tmp->filename));
                current->next = NULL;
                strcpy(current->filename, tmp->filename);
            } else {
                //sprintf((char *) result, "FILE:\t%s\n", tmp->filename);
                current->type = 0;
                current->filename = malloc(strlen(tmp->filename));
                current->next = NULL;
                strcpy(current->filename, tmp->filename);
            }
            //strcat(output, (const char *) result);
            tmp = tmp->next_inode;
        }
        if (current_path) {
            free_inode((find_result->result->next_inode));
            find_result->result->next_inode = NULL;
        } else {
            free_inode((find_result->start));
        }
        if (parent_path) {
            g_info->cur_node->parent->next_inode = g_info->cur_node;
        }

        find_result->result = NULL;
        find_result->start = NULL;
        return first;
    }
    return NULL;
}

char *cp(GENERAL_INFORMATION *g_info, char *from_path, char *to_path) {
    char *output = malloc(32);
    output[0] = '\0';
    if (strcmp(from_path, ".") == 0 || strcmp(from_path, "..") == 0) {
        sprintf(output, "ERROR: Incompatible file path\n");
        return output;
    }
    FIND_INFO *result;
    INODE *start_node;
    char *message;
    if (from_path[0] == '/') {
        start_node = g_info->root_node;
    } else {
        start_node = g_info->cur_node;
    }
    int err = find_node_by_name(g_info, from_path, &start_node, &result);
    if (err == -1) {
        message = "No such file or directory";
        sprintf(output, "%s\n", message);
        return output;
    } else if (copy(g_info, result->result, to_path) != -1) {
        message = "Successfully copied";
        sprintf(output, "%s\n", message);
        return output;
    } else {
        message = "ERROR: ERROR";
        sprintf(output, "%s\n", message);
        return output;
    }
}

GENERAL_INFORMATION *ntfs_init(char *filename) {
    GENERAL_INFORMATION *g_info = init(filename);
    return g_info;
}

int ntfs_close(GENERAL_INFORMATION *g_info) {
    return free_g_info(g_info);
}

int free_ls_info(LS_INFO *first) {
    LS_INFO *tmp;

    while (first != NULL) {
        tmp = first;
        first = first->next;

        if (tmp->filename != NULL) {
            free(tmp->filename);
        }
        free(tmp);
    }
    return 0;
}