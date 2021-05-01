#ifndef LAB_1_UTIL_H
#define LAB_1_UTIL_H

#include "ntfs.h"

char *pwd(const GENERAL_INFORMATION *const g_info);

char *cd(GENERAL_INFORMATION *g_info, char *path);

char *ls(GENERAL_INFORMATION *g_info, char *path);

char *cp(GENERAL_INFORMATION *g_info, char *from_path, char *to_path)

#endif //LAB_1_UTIL_H
