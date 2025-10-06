/*
 * Custom implementation of 'ls' - version 1.1.0
 * Feature: Long listing format (-l)
 * Author: BSDSF23M002
 */

#define _POSIX_C_SOURCE 200809L

#ifndef S_ISVTX
#define S_ISVTX 01000
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>   // ✅ For mode_t, pid_t, etc.
#include <sys/stat.h>    // ✅ For struct stat, lstat()
#include <pwd.h>         // ✅ For getpwuid()
#include <grp.h>         // ✅ For getgrgid()
#include <time.h>        // ✅ For ctime(), localtime()
#include <unistd.h>      // ✅ For getcwd(), access(), etc.


void print_permissions(mode_t mode) {
    char perms[11] = "----------";

    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';
    else if (S_ISCHR(mode)) perms[0] = 'c';
    else if (S_ISBLK(mode)) perms[0] = 'b';
    else if (S_ISSOCK(mode)) perms[0] = 's';
    else if (S_ISFIFO(mode)) perms[0] = 'p';

    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';
    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';
    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';

    // Special bits
    if (mode & S_ISUID) perms[3] = (perms[3] == 'x') ? 's' : 'S';
    if (mode & S_ISGID) perms[6] = (perms[6] == 'x') ? 's' : 'S';
    if (mode & S_ISVTX) perms[9] = (perms[9] == 'x') ? 't' : 'T';

    printf("%s ", perms);
}

void print_long_format(const char *path, const char *filename) {
    struct stat fileStat;
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

    if (lstat(fullpath, &fileStat) == -1) {
        perror("lstat");
        return;
    }

    // Permissions
    print_permissions(fileStat.st_mode);

    // Links
    printf("%3ld ", fileStat.st_nlink);

    // Owner and Group
    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group *gr = getgrgid(fileStat.st_gid);
    printf("%-8s %-8s ", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

    // Size
    printf("%8ld ", fileStat.st_size);

    // Time
    char timebuf[80];
    struct tm *timeinfo = localtime(&fileStat.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
    printf("%s ", timebuf);

    // File name
    printf("%s\n", filename);
}

void list_dir(const char *path, int long_format) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // Skip hidden files

        if (long_format)
            print_long_format(path, entry->d_name);
        else
            printf("%s  ", entry->d_name);
    }

    if (!long_format)
        printf("\n");

    closedir(dir);
}

int main(int argc, char *argv[]) {
    int long_format = 0;
    char *path = ".";

    // Check for -l flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0)
            long_format = 1;
        else
            path = argv[i];
    }

    list_dir(path, long_format);

    return 0;
}
