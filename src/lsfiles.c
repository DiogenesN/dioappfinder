/* returns the number of regular files in a directory */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int countFilesInDirectory(const char *path) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        return -1;  // Directory not found or cannot be opened
    }

    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) {
            count++;
        }
    }

    closedir(dir);

    return count;
}
