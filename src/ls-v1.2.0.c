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

/* ------------------ Gather all filenames ------------------ */
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
        if (entry->d_name[0] == '.') continue; // skip hidden files

        if (*count >= capacity) {  // resize array
            capacity *= 2;
            char **temp = realloc(files, capacity * sizeof(char *));
            if (!temp) {
                perror("realloc");
                for (int i = 0; i < *count; i++) free(files[i]);
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

/* ------------------ Terminal width & layout ------------------ */
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return 80;
    return w.ws_col;
}

void calculate_layout(int total_files, int max_len, int spacing, int *cols, int *rows) {
    int term_width = get_terminal_width();
    *cols = term_width / (max_len + spacing);
    if (*cols < 1) *cols = 1;

    *rows = (total_files + *cols - 1) / *cols; // round up
}

/* ------------------ Main ------------------ */
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

    int spacing = 2;
    int cols = 0, rows = 0;

    calculate_layout(file_count, max_len, spacing, &cols, &rows);

    printf("Terminal width: %d\n", get_terminal_width());
    printf("Columns: %d, Rows: %d\n", cols, rows);
    printf("Files:\n");
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", files[i]);
    }

    for (int i = 0; i < file_count; i++) free(files[i]);
    free(files);

    return 0;
}

