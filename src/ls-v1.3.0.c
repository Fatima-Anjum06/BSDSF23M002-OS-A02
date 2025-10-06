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

enum DisplayMode { MODE_DEFAULT, MODE_LONG, MODE_HORIZONTAL };

/* Function: gather_filenames
 * --------------------------
 * Reads all filenames (excluding hidden files) into a dynamic array
 * and tracks the longest filename.
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
        if (entry->d_name[0] == '.') continue; // Skip hidden files
        if (*count >= capacity) {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
            if (!files) {
                perror("realloc");
                closedir(dir);
                return NULL;
            }
        }
        files[*count] = strdup(entry->d_name);
        int len = strlen(entry->d_name);
        if (len > *max_len) *max_len = len;
        (*count)++;
    }

    closedir(dir);
    return files;
}

/* Get terminal width (fallback = 80) */
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0)
        return 80;
    return w.ws_col;
}

/* Function: print_down_then_across
 * --------------------------------
 * Default layout (previous feature placeholder)
 */
void print_down_then_across(char **files, int count, int max_len) {
    printf(">>> Default down-then-across display (placeholder for now)\n");
    for (int i = 0; i < count; i++) {
        printf("%s\n", files[i]);
    }
}

/* Function: print_long_format
 * ----------------------------
 * Placeholder for previous -l long listing version
 */
void print_long_format(const char *path) {
    printf(">>> Long listing mode (placeholder for now)\n");
}

/* Function: print_horizontal
 * ----------------------------
 * Implements the -x horizontal display mode.
 */
void print_horizontal(char **files, int count, int max_len) {
    int term_width = get_terminal_width();
    int spacing = 2;
    int col_width = max_len + spacing;
    int current_width = 0;

    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (current_width + col_width > term_width) {
            printf("\n");
            current_width = 0;
        }
        printf("%-*s", col_width, files[i]);
        current_width += col_width;
    }
    printf("\n");
}

/* MAIN FUNCTION
 * Integrates argument parsing and display selection
 */
int main(int argc, char *argv[]) {
    int opt;
    enum DisplayMode mode = MODE_DEFAULT;

    // Parse -l and -x options
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                mode = MODE_LONG;
                break;
            case 'x':
                mode = MODE_HORIZONTAL;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l|-x] [directory]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";

    int count, max_len;
    char **files = gather_filenames(path, &count, &max_len);
    if (!files) return EXIT_FAILURE;

    // Choose the display mode
    switch (mode) {
        case MODE_LONG:
            print_long_format(path);
            break;
        case MODE_HORIZONTAL:
            print_horizontal(files, count, max_len);
            break;
        default:
            print_down_then_across(files, count, max_len);
            break;
    }

    // Cleanup
    for (int i = 0; i < count; i++) free(files[i]);
    free(files);

    return EXIT_SUCCESS;
}
