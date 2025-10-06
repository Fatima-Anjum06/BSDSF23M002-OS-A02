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
#include <errno.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_CYAN    "\033[36m"

// ----- Function Prototypes -----
char **gather_filenames(const char *path, int *count, int *max_len);
int cmpfunc(const void *a, const void *b);
void print_permissions(mode_t mode);
void print_long_format(const char *path, char **files, int count);
void print_down_then_across(const char *path, char **files, int count, int max_len);
void print_horizontal(const char *path, char **files, int count, int max_len);
void print_colored(const char *path, const char *name);

// ----- Main -----
int main(int argc, char *argv[]) {
    int opt;
    int flag_l = 0, flag_x = 0;
    const char *path;

    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l': flag_l = 1; break;
            case 'x': flag_x = 1; break;
            default: fprintf(stderr, "Usage: %s [-l] [-x] [path]\n", argv[0]); exit(EXIT_FAILURE);
        }
    }

    path = (optind < argc) ? argv[optind] : ".";

    int count = 0, max_len = 0;
    char **files = gather_filenames(path, &count, &max_len);
    if (!files) return EXIT_FAILURE;

    // Sort alphabetically
    qsort(files, count, sizeof(char *), cmpfunc);

    // Display
    if (flag_l) print_long_format(path, files, count);
    else if (flag_x) print_horizontal(path, files, count, max_len);
    else print_down_then_across(path, files, count, max_len);

    // Free memory
    for (int i = 0; i < count; i++) free(files[i]);
    free(files);

    return 0;
}

// ----- Gather Filenames -----
char **gather_filenames(const char *path, int *count, int *max_len) {
    DIR *dir = opendir(path);
    if (!dir) { perror("opendir"); return NULL; }

    struct dirent *entry;
    int capacity = 10;
    char **files = malloc(capacity * sizeof(char *));
    if (!files) { perror("malloc"); closedir(dir); return NULL; }

    *count = 0;
    *max_len = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden

        if (*count >= capacity) {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
            if (!files) { perror("realloc"); closedir(dir); return NULL; }
        }
        files[*count] = strdup(entry->d_name);
        if (!files[*count]) { perror("strdup"); closedir(dir); return NULL; }

        int len = strlen(entry->d_name);
        if (len > *max_len) *max_len = len;
        (*count)++;
    }
    closedir(dir);
    return files;
}

// ----- Compare function for qsort -----
int cmpfunc(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// ----- Print Permissions (long listing) -----
void print_permissions(mode_t mode) {
    char perms[11] = "----------";
    if (S_ISDIR(mode)) perms[0] = 'd';
    if (S_ISLNK(mode)) perms[0] = 'l';
    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';
    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';
    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';
    printf("%s ", perms);
}

// ----- Print Long Listing -----
void print_long_format(const char *path, char **files, int count) {
    for (int i = 0; i < count; i++) {
        char fullpath[1024];
        struct stat st;
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, files[i]);
        if (lstat(fullpath, &st) == -1) { perror("lstat"); continue; }

        print_permissions(st.st_mode);
        printf("%2ld ", st.st_nlink);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        printf("%s %s ", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

        printf("%5ld ", st.st_size);

        char *time_str = ctime(&st.st_mtime);
        time_str[strlen(time_str)-1] = '\0';
        printf("%s ", time_str);

        print_colored(path, files[i]);
        printf("\n");
    }
}

// ----- Print Down-Then-Across Columns -----
void print_down_then_across(const char *path, char **files, int count, int max_len) {
    struct winsize ws;
    int term_width = (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) ? ws.ws_col : 80;
    int col_width = max_len + 2;
    int num_cols = term_width / col_width;
    if (num_cols < 1) num_cols = 1;
    int num_rows = (count + num_cols - 1) / num_cols;

    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            int idx = c * num_rows + r;
            if (idx < count) {
                print_colored(path, files[idx]);
                int padding = col_width - strlen(files[idx]);
                for (int p = 0; p < padding; p++) printf(" ");
            }
        }
        printf("\n");
    }
}

// ----- Print Horizontal (-x) Columns -----
void print_horizontal(const char *path, char **files, int count, int max_len) {
    struct winsize ws;
    int term_width = (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) ? ws.ws_col : 80;
    int col_width = max_len + 2;
    int cur_width = 0;

    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (cur_width + len + 2 > term_width) { printf("\n"); cur_width = 0; }
        print_colored(path, files[i]);
        for (int p = 0; p < col_width - len; p++) printf(" ");
        cur_width += col_width;
    }
    printf("\n");
}

// ----- Print Colored Filename -----
void print_colored(const char *path, const char *name) {
    struct stat st;
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, name);

    if (lstat(fullpath, &st) == -1) { perror("lstat"); printf("%s  ", name); return; }

    if (S_ISDIR(st.st_mode)) printf(COLOR_BLUE "%s" COLOR_RESET "  ", name);
    else if (st_mode & S_IXUSR) printf(COLOR_GREEN "%s" COLOR_RESET "  ", name);
    else if (S_ISLNK(st.st_mode)) printf(COLOR_CYAN "%s" COLOR_RESET "  ", name);
    else printf("%s  ", name);
}
