#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "../../core/inc/device.h"
#include "../../core/inc/util.h"
#include <getopt.h>


static void help();

static void shell(char *filename);

static void options(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    options(argc, argv);
}

static void options(int argc, char *argv[]) {
    const char *short_flags = "lhs:";

    const struct option long_flags[] = {
            {"list",  0, NULL, 'l'},
            {"help",  0, NULL, 'h'},
            {"shell", 1, NULL, 's'},
            {0,       0, 0,    0}
    };

    int rez;
    int long_id = 0;

    while ((rez = getopt_long(argc, argv, short_flags, long_flags, &long_id)) != -1) {
        switch (rez) {
            case 'l':
                print_device();
                break;
            case 'h':
                
                help();
                break;
            case 's':
                shell(optarg);
                break;
        }
    }
}

struct help {
    char short_f;
    char *long_f;
    char *description;
};

static struct help help_list[3] = {
        {
                'l', "list",  "show list of devices and partition"},
        {
                'h', "help",  "show help (this message)"},
        {
                's', "shell", "shell mode (interactive mode)"}
};

static void help() {
    for (uint8_t i = 0; i < 3; i++) {
        printf("\tshor name: %c\n"
               "\tlong name: %s\n"
               "\tdescription: %s\n\n",
               help_list[i].short_f,
               help_list[i].long_f,
               help_list[i].description
        );
    }
}

static void shell(char *filename) {
    GENERAL_INFORMATION *g_info = init(filename);
    if (g_info == NULL) {
        puts("No NTFS file system detected");
        return;
    }

    bool exit = false;
    char *input = malloc(1024);
    char *sep = " \n";
    char *output = NULL;
    while (!exit) {
        char *current_dir = pwd(g_info);
        printf("%s> ", current_dir);
        free(current_dir);
        fgets(input, 1024, stdin);
        char *command = strtok(input, sep);
        if (command == NULL) {
            continue;
        }
        char *from_path = strtok(NULL, sep);
        char *to_path = strtok(NULL, sep);
        if (strcmp(command, "ls") == 0) {
            output = ls(g_info, from_path);
            if (output == NULL) {
                printf("No such directory\n");
                continue;
            }
            printf("%s", output);
            free(output);
        } else if (strcmp(command, "pwd") == 0) {
            output = pwd(g_info);
            printf("%s\n", output);
            free(output);
        } else if (strcmp(command, "cd") == 0) {
            if (from_path == NULL) {
                puts("cd require path argument");
                continue;
            }
            output = cd(g_info, from_path);
            printf("%s", output);
            free(output);
        } else if (strcmp(command, "cp") == 0) {
            if (from_path == NULL) {
                puts("cp require from_path argument");
                continue;
            }
            if (to_path == NULL) {
                puts("cp require to_path argument");
                continue;
            }
            output = cp(g_info, from_path, to_path);
            printf("%s", output);
            free(output);
        } else if (strcmp(command, "help") == 0) {
            puts("ls - show working directory elements\n"
                 "cd [directory] - change working directory\n"
                 "pwd - print working directory\n"
                 "cp [directory] [target directory] - copy dir or file from file system\n"
                 "help - list of commands\n"
                 "exit - terminate");
        } else if (strcmp(command, "exit") == 0) {
            exit = true;
        } else {
            puts("Wrong command. Please enter 'help' to get help");
        }
    }
    free(g_info);
}