#ifndef LAB_1_UTIL_H
#define LAB_1_UTIL_H

#include <sys/stat.h>
#include <fcntl.h>
#include "ntfs.h"

typedef struct ls_info {
    char *filename;
    int type;
    struct ls_info *next;
} LS_INFO;


char *pwd(const GENERAL_INFORMATION *g_info);

char *cd(GENERAL_INFORMATION *g_info, char *path);

LS_INFO *ls(GENERAL_INFORMATION *g_info, char *path);

char *cp(GENERAL_INFORMATION *g_info, char *from_path, char *to_path);

GENERAL_INFORMATION *ntfs_init(char *filename);

int ntfs_close(GENERAL_INFORMATION *g_info);

int free_ls_info(LS_INFO *first);

#endif //LAB_1_UTIL_H