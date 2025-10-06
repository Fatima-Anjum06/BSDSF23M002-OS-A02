#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

/* ---------- Display Modes ---------- */
typedef enum {
    MODE_DEFAULT,
    MODE_LONG,
    MODE_HORIZONTAL
} DisplayMode;

/* ---------- Function Declarations ---------- */
char **gather_filenames(const char *path, int *count, int *max_len);
void print_down_then_across(char **files, int count, int max_len);
void print_horizontal(char **files, int count, int max_len);
void print_long_format(const char *path);
int cmpfunc(const void *a, const void *b);

/* ---------- Comparison Function for qsort ---------- */
int cmpfunc(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

/* ---------- Gather Filenames ---------- */
char **gather_filenames(const char *path, int *count, int *max_len) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return NULL;
    }

    struct dirent *entry;
    int capacity = 20;
    int n = 0;
    *max_len = 0;
    char **files = malloc(capacity * sizeof(char *));
    if (!files) {
        perror("malloc");
        closedir(dir);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden
        if (n >= capacity) {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
        }
        files[n] = strdup(entry->d_name);
        if ((int)strlen(entry->d_name) > *max_len)
            *max_len = strlen(entry->d_name);
        n++;
    }

    closedir(dir);
    *count = n;
    return files;
}

/* ---------- Down-Then-Across Display ---------- */
void print_down_then_across(char **files, int count, int max_len) {
    (void)max_len; // suppress unused warning
    struct winsize ws;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
        term_width = ws.ws_col;

    int spacing = 2;
    int col_width = max_len + spacing;
    int num_cols = term_width / col_width;
    if (num_cols < 1) num_cols = 1;
    int num_rows = (count + num_cols - 1) / num_cols;

    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            int idx = c * num_rows + r;
            if (idx < count)
                printf("%-*s", col_width, files[idx]);
        }
        printf("\n");
    }
}

/* ---------- Horizontal (-x) Display ---------- */
void print_horizontal(char **files, int count, int max_len) {
    (void)max_len; // suppress unused warning
    struct winsize ws;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
        term_width = ws.ws_col;

    int spacing = 2;
    int col_width = max_len + spacing;
    int current_width = 0;

    for (int i = 0; i < count; i++) {
        if (current_width + col_width > term_width) {
            printf("\n");
            current_width = 0;
        }
        printf("%-*s", col_width, files[i]);
        current_width += col_width;
    }
    printf("\n");
}

/* ---------- Long Listing Placeholder ---------- */
void print_long_format(const char *path) {
    (void)path; // suppress unused warning
    printf(">>> Long listing mode (placeholder for now)\n");
}

/* ---------- MAIN ---------- */
int main(int argc, char *argv[]) {
    DisplayMode mode = MODE_DEFAULT;
    int opt;

    /* Parse command-line arguments */
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                mode = MODE_LONG;
                break;
            case 'x':
                mode = MODE_HORIZONTAL;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l | -x] [path]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";
    int count, max_len;
    char **files = gather_filenames(path, &count, &max_len);
    if (!files) return EXIT_FAILURE;

    /* Sort alphabetically for all display modes */
    qsort(files, count, sizeof(char *), cmpfunc);

    /* Display files */
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

    /* Free memory */
    for (int i = 0; i < count; i++)
        free(files[i]);
    free(files);

    return EXIT_SUCCESS;
}
