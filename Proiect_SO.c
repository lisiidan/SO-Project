#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>  // Pentru waitpid() și macro-urile asociate statusului

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
    char snapshot_path[MAX_PATH * 2];
    int directory_count = 0;
    pid_t pid;

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
    for (int i = optind; i < argc && directory_count < MAX_DIRECTORIES; i++, directory_count++) {
        pid = fork();
        if (pid == 0) { // Proces copil
            snprintf(snapshot_path, sizeof(snapshot_path), "%s/Snapshot_%d.txt", output_dir, i);
            FILE *file = fopen(snapshot_path, "w");
            if (file == NULL) {
                perror("Failed to create snapshot file");
                exit(EXIT_FAILURE);
            }
            read_directory(argv[i], file);
            printf("Snapshot for Directory %d created successfully\n",i);
            fclose(file);
            exit(EXIT_SUCCESS);  // Terminarea copilului cu succes
        }
    }
    printf("\n");
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, 0)) > 0) {  // Așteptăm terminarea fiecărui copil
        if (WIFEXITED(status)) {
            printf("Procesul copil cu PID-ul %d s-a incheiat cu codul %d.\n", child_pid, WEXITSTATUS(status));
        }
    }

    return EXIT_SUCCESS;
}
