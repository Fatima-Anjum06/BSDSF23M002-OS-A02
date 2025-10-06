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
        // Skip hidden files
        if (entry->d_name[0] == '.')
            continue;

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
            for (int i = 0; i < *count; i++)
                free(files[i]);
            free(files);
            closedir(dir);
            return NULL;
        }

        int len = strlen(entry->d_name);
        if (len > *max_len)
            *max_len = len;

        (*count)++;
    }

    closedir(dir);
    return files;
}

/*
 * Function: print_down_then_across
 * --------------------------------
 * Prints filenames in "down then across" format, adjusting to terminal width.
 *
 * files: array of filenames
 * count: total number of files
 * max_len: length of the longest filename
 * term_width: terminal width in characters
 */
void print_down_then_across(char **files, int count, int max_len, int term_width) {
    int spacing = 2;
    int col_width = max_len + spacing;
    int num_cols = term_width / col_width;

    if (num_cols < 1)
        num_cols = 1;

    int num_rows = (count + num_cols - 1) / num_cols;

    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            int index = c * num_rows + r;
            if (index < count) {
                printf("%-*s", col_width, files[index]);
            }
        }
        printf("\n");
    }
}

/*
 * Function: main
 * --------------
 * Entry point for ls-v1.2.0
 * Gathers filenames, determines terminal width, and prints in columns (down then across).
 */
int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";
    int count = 0, max_len = 0;

    char **files = gather_filenames(path, &count, &max_len);
    if (!files || count == 0) {
        fprintf(stderr, "No files found or error reading directory.\n");
        return 1;
    }

    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    print_down_then_across(files, count, max_len, term_width);

    for (int i = 0; i < count; i++)
        free(files[i]);
    free(files);

    return 0;
}
