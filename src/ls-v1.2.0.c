/*
 * Custom implementation of 'ls' - version 1.2.0
 * Feature: Column Display (Down Then Across)
 * Author: BSDSF23M002
 *
 * Task 2: Gather all filenames in a directory into a dynamic array
 * and calculate the length of the longest filename.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

/*
 * Function: gather_filenames
 * --------------------------
 * Reads all filenames from the given directory path into a dynamically allocated array.
 * Ignores hidden files (starting with '.').
 *
 * path: Directory path to read
 * count: Pointer to store number of files found
 * max_len: Pointer to store length of the longest filename
 *
 * returns: Pointer to array of strings (filenames). NULL on error.
 */
char **gather_filenames(const char *path, int *count, int *max_len) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return NULL;
    }

    struct dirent *entry;
    int capacity = 10;
    char **files = malloc(capacity * sizeof(char *));
    if (!files) {
        perror("malloc");
        closedir(dir);
        return NULL;
    }

    *count = 0;
    *max_len = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        if (*count >= capacity) {
            capacity *= 2;
            char **temp = realloc(files, capacity * sizeof(char *));
            if (!temp) {
                perror("realloc");
                for (int i = 0; i < *count; i++)
                    free(files[i]);
                free(files);
                closedir(dir);
                return NULL;
            }
            files = temp;
        }

        files[*count] = strdup(entry->d_name);
        if (!files[*count]) {
            perror("strdup");
            continue;
        }

        int len = strlen(entry->d_name);
        if (len > *max_len) *max_len = len;

        (*count)++;
    }

    closedir(dir);
    return files;
}

int main(int argc, char *argv[]) {
    char *path = ".";
    if (argc > 1) path = argv[1];

    int file_count = 0;
    int max_len = 0;

    char **files = gather_filenames(path, &file_count, &max_len);
    if (!files || file_count == 0) {
        printf("No files found in directory: %s\n", path);
        return 0;
    }

    printf("Total files: %d\n", file_count);
    printf("Longest filename length: %d\n\n", max_len);
    printf("Files:\n");
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", files[i]);
        free(files[i]);
    }
    free(files);

    return 0;
}
