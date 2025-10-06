/*
 * Custom implementation of the 'ls' command (Version 1.0.0)
 * Author: BSDSF23M002
 * Description: Lists files and directories in the current directory.
 */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;
    char *path;

    if (argc < 2)
        path = ".";
    else
        path = argv[1];

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.')
            printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return EXIT_SUCCESS;
}
