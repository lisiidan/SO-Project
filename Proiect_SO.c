#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>  // Pentru navigarea in directoare
#include <sys/stat.h> // Pentru metadatele fisierelor
#include <string.h>
#include <time.h> // Pentru timestamp

typedef struct {
    char path[1024];
    long size; // dimensiunea fisierului
    time_t last_modified; // data ultimei modificari
} FileData;

void read_directory(const char *dirname, FILE *file) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char path[1024];

    dir = opendir(dirname);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        if (stat(path, &file_stat) == 0) {
            fprintf(file, "%s, %ld, %ld\n", path, file_stat.st_size, file_stat.st_mtime);
        }

        if (entry->d_type == DT_DIR) {
            read_directory(path, file);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    char snapshot_path[1024];
    snprintf(snapshot_path, sizeof(snapshot_path), "%s/Snapshot.txt", argv[1]);

    FILE *file = fopen(snapshot_path, "w");
    if (file == NULL) {
        perror("Failed to create file");
        return EXIT_FAILURE;
    }

    read_directory(argv[1], file);
    fclose(file);

    return EXIT_SUCCESS;
}

