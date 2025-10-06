#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/ioctl.h>

#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_REVERSE "\033[7m"

// Forward declarations
void print_long_format(const char *path, char **files, int count);
void print_down_then_across(char **files, int count);
void print_horizontal(char **files, int count);
void print_colored(const char *filename, mode_t mode);

// Comparison function for qsort
int cmpfunc(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Gather filenames dynamically
char **gather_filenames(const char *path, int *count) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return NULL;
    }

    struct dirent *entry;
    int capacity = 10;
    *count = 0;
    char **files = malloc(capacity * sizeof(char *));
    if (!files) return NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden files
        if (*count >= capacity) {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
        }
        files[*count] = strdup(entry->d_name);
        (*count)++;
    }
    closedir(dir);
    return files;
}

// Recursive listing function
void do_ls(const char *dirname, int long_flag, int horiz_flag, int recursive_flag) {
    int count;
    char **files = gather_filenames(dirname, &count);
    if (!files) return;

    // Sort alphabetically
    qsort(files, count, sizeof(char *), cmpfunc);

    // Print directory header if recursive
    if (recursive_flag) {
        printf("%s:\n", dirname);
    }

    // Choose display mode
    if (long_flag) print_long_format(dirname, files, count);
    else if (horiz_flag) print_horizontal(files, count);
    else print_down_then_across(files, count);

    // Recursive descent
    if (recursive_flag) {
        for (int i = 0; i < count; i++) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, files[i]);

            struct stat st;
            if (lstat(fullpath, &st) == -1) continue;
            if (S_ISDIR(st.st_mode)) {
                do_ls(fullpath, long_flag, horiz_flag, recursive_flag);
            }
        }
    }

    // Free memory
    for (int i = 0; i < count; i++) free(files[i]);
    free(files);
}

// Placeholder implementations
void print_long_format(const char *path, char **files, int count) { printf("Long listing mode (placeholder)\n"); }
void print_down_then_across(char **files, int count) { printf("Down-then-across (placeholder)\n"); }
void print_horizontal(char **files, int count) { printf("Horizontal (-x) display (placeholder)\n"); }
void print_colored(const char *filename, mode_t mode) {}

int main(int argc, char *argv[]) {
    int long_flag = 0, horiz_flag = 0, recursive_flag = 0;
    int opt;

    while ((opt = getopt(argc, argv, "lxR")) != -1) {
        switch (opt) {
            case 'l': long_flag = 1; break;
            case 'x': horiz_flag = 1; break;
            case 'R': recursive_flag = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [directory]\n", argv[0]);
                return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";
    do_ls(path, long_flag, horiz_flag, recursive_flag);

    return 0;
}

