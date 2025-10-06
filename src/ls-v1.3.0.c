#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>     // for getopt(), optind
#include <sys/ioctl.h>  // for terminal size
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

/*
 * Function: gather_filenames
 * --------------------------
 * Reads all non-hidden filenames from a directory into a dynamic array.
 * Tracks the total count and the longest filename length.
 */
char **gather_filenames(const char *path, int *count, int *max_len) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return NULL;
    }

    struct dirent *entry;
    int capacity = 20;
    *count = 0;
    *max_len = 0;
    char **files = malloc(capacity * sizeof(char *));
    if (!files) {
        perror("malloc");
        closedir(dir);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden files

        if (*count >= capacity) {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
        }

        files[*count] = strdup(entry->d_name);
        int len = strlen(entry->d_name);
        if (len > *max_len) *max_len = len;
        (*count)++;
    }

    closedir(dir);
    return files;
}

/*
 * Function: print_down_then_across
 * --------------------------------
 * Prints files in column-major order (default behavior).
 */
void print_down_then_across(char **files, int count, int max_len) {
    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    int spacing = 2;
    int cols = term_width / (max_len + spacing);
    if (cols < 1) cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int index = c * rows + r;
            if (index < count)
                printf("%-*s", max_len + spacing, files[index]);
        }
        printf("\n");
    }
}

/*
 * Function: print_horizontal
 * --------------------------
 * Prints files in row-major order (for the -x flag).
 */
void print_horizontal(char **files, int count, int max_len) {
    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    int spacing = 2;
    int cols = term_width / (max_len + spacing);
    if (cols < 1) cols = 1;

    for (int i = 0; i < count; i++) {
        printf("%-*s", max_len + spacing, files[i]);
        if ((i + 1) % cols == 0)
            printf("\n");
    }
    if (count % cols != 0)
        printf("\n");
}

/*
 * Function: main
 * --------------
 * Entry point — handles argument parsing and display mode.
 */
int main(int argc, char *argv[]) {
    int opt;
    int horizontal_mode = 0; // 0 = default, 1 = -x

    while ((opt = getopt(argc, argv, "x")) != -1) {
        switch (opt) {
            case 'x':
                horizontal_mode = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-x] [directory]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";
    int count = 0, max_len = 0;
    char **files = gather_filenames(path, &count, &max_len);
    if (!files) return EXIT_FAILURE;

    if (horizontal_mode)
        print_horizontal(files, count, max_len);
    else
        print_down_then_across(files, count, max_len);

    for (int i = 0; i < count; i++)
        free(files[i]);
    free(files);

    return EXIT_SUCCESS;
}
