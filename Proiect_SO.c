#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <unistd.h> 

#define MAX_DIRECTORIES 10
#define MAX_PATH 1024

void read_directory(const char *dirname, FILE *file) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char path[MAX_PATH];

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
    int opt;
    char output_dir[MAX_PATH] = "./"; 
    char snapshot_path[MAX_PATH * 2]; // Mărire dimensiune buffer
    int directory_count = 0;

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                if (strlen(optarg) >= MAX_PATH) {
                    fprintf(stderr, "Output directory path is too long\n");
                    return EXIT_FAILURE;
                }
                strcpy(output_dir, optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -o <output_dir> <dir1> <dir2> ... <dir10>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }
    int Snapshot_cnt = 0;
    for (int i = optind; i < argc && directory_count < MAX_DIRECTORIES; i++, directory_count++) {
        int needed = snprintf(snapshot_path, sizeof(snapshot_path), "%s/Snapshot_%d.txt", output_dir, ++Snapshot_cnt);
        if (needed >= sizeof(snapshot_path)) {
            fprintf(stderr, "Snapshot path is too long and has been truncated. It might not work correctly.\n");
            continue; 
        }
        
        FILE *file = fopen(snapshot_path, "w");
        if (file == NULL) {
            perror("Failed to create snapshot file");
            continue; 
        }
        read_directory(argv[i], file);
        fclose(file);
    }

    return EXIT_SUCCESS;
}
