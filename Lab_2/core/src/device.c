#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <regex.h>

#define SYS_BLOCK_DIR "/sys/block/"
#define DRIVE_PATTERN "^((loop)[0-9])|(sda)|(sr[0-9])"

int print_available_devices() {
    DIR *block_devices_dir;
    DIR *partitions_dir;
    struct dirent *block_device_entry;
    struct dirent *partition_entry;
    regex_t matcher_drives;
    regex_t matcher_partitions;

    regcomp(&matcher_drives, DRIVE_PATTERN, REG_EXTENDED);
    block_devices_dir = opendir(SYS_BLOCK_DIR);
    if (!block_devices_dir) {
        return -1;
    }
    while ((block_device_entry = readdir(block_devices_dir)) != NULL) {
        if (!regexec(&matcher_drives, block_device_entry->d_name, 0, NULL, 0)) {
            printf("Drive %s\n", block_device_entry->d_name);

            regcomp(&matcher_partitions, block_device_entry->d_name, 0);
            char drive_dir[strlen(SYS_BLOCK_DIR) + strlen(block_device_entry->d_name) + 1];
            drive_dir[0] = 0;
            strcat(drive_dir, SYS_BLOCK_DIR);
            strcat(drive_dir, block_device_entry->d_name);
            partitions_dir = opendir(drive_dir);
            if (!partitions_dir) {
                return -1;
            }
            while ((partition_entry = readdir(partitions_dir)) != NULL) {
                if (!regexec(&matcher_partitions, partition_entry->d_name, 0, NULL, 0)) {
                    printf("\t- partition %s\n", partition_entry->d_name);
                }
            }
        }
    }
    closedir(block_devices_dir);
    return 0;
}