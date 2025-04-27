#define _XOPEN_SOURCE 500  // This macro ensures compatibility with nftw()
#include <stdio.h>  // Standard I/O functions
#include <stdlib.h> // Standard library for memory allocation, process control, etc.
#include <string.h> // String handling functions
#include <ftw.h>  // File tree walk function to traverse directories
#include <sys/stat.h> // File status functions and macros
#include <unistd.h>  // POSIX API for file handling
#include <fcntl.h>  // File control options
#include <dirent.h> // Directory handling functions

#define MAX_PATH 1024  // Defines the maximum length for file paths

// Global variables to track files, directories, and total file size
int file_count = 0, dir_count = 0; // Counters for files and directories
off_t total_size = 0;  // Stores the total size of all files in bytes
char *search_ext = NULL;  // Pointer to store file extension to search for
char *exclude_ext = NULL; // Pointer to store file extension to exclude from copy
char dest_root[MAX_PATH]; // Destination root directory for copy/move operations
char src_root[MAX_PATH];  // Source directory to perform operations on

// Function to generate the destination path when copying/moving files
divvoid construct_dest_path(char *dest_path, const char *src_path) {
    snprintf(dest_path, MAX_PATH, "%s%s", dest_root, src_path + strlen(src_root)); // Append relative path to destination root
}

// Callback function to list all files and directories
int list_callback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    printf("%s\n", path); // Print the file or directory path
    return 0; // Continue traversal
}

// Callback function to count the number of files
int count_files_callback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_F) file_count++;  // If it is a file, increase the file count
    return 0; // Continue traversal
}

// Callback function to count the number of directories
int count_dirs_callback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_D) dir_count++;  // If it is a directory, increase the directory count
    return 0; // Continue traversal
}

// Callback function to calculate the total size of all files in a directory
int size_callback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_F) total_size += sb->st_size;  // Add file size to total size counter
    return 0; // Continue traversal
}

// Callback function to list files that have a specific extension
int ext_callback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_F && search_ext) { // Check if it is a file and extension search is enabled
        const char *ext = strrchr(path, '.'); // Extract file extension
        if (ext && strcmp(ext, search_ext) == 0) { // Compare with target extension
            printf("%s\n", path); // Print file path if it matches
        }
    }
    return 0; // Continue traversal
}

// Callback function to delete files with a specific extension
int delete_callback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_F && search_ext) { // Check if it is a file and matches the extension
        const char *ext = strrchr(path, '.'); // Extract file extension
        if (ext && strcmp(ext, search_ext) == 0) { // Compare with target extension
            remove(path);  // Delete the file
            printf("Deleted: %s\n", path); // Print confirmation
        }
    }
    return 0; // Continue traversal
}

// Callback function to copy files and directories while skipping excluded extensions
int copy_callback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    char dest_path[MAX_PATH]; // Buffer to store destination path
    construct_dest_path(dest_path, path);  // Generate the destination path

    if (typeflag == FTW_D) { // If it is a directory
        mkdir(dest_path, sb->st_mode);  // Create the directory at destination
    } else if (typeflag == FTW_F) { // If it is a file
        const char *ext = strrchr(path, '.'); // Extract file extension
        if (!exclude_ext || !ext || strcmp(ext, exclude_ext) != 0) {  // Skip copying if the extension is excluded
            int src_fd = open(path, O_RDONLY); // Open source file
            if (src_fd < 0) { // Check if file opening failed
                perror("Error opening source file");
                return -1; // Exit with error
            }
            int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, sb->st_mode); // Open or create destination file
            if (dest_fd < 0) { // Check if file creation failed
                perror("Error creating destination file");
                close(src_fd);
                return -1; // Exit with error
            }
            char buffer[4096]; // Buffer for file copying
            ssize_t bytes; // Variable to store number of bytes read
            while ((bytes = read(src_fd, buffer, sizeof(buffer))) > 0) { // Read from source file
                write(dest_fd, buffer, bytes); // Write to destination file
            }
            close(src_fd); // Close source file
            close(dest_fd); // Close destination file
        }
    }
    return 0; // Continue traversal
}

// Callback function to move files and directories
int move_callback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    char dest_path[MAX_PATH]; // Buffer to store destination path
    construct_dest_path(dest_path, path); // Generate destination path
    rename(path, dest_path);  // Move file or directory
    return 0; // Continue traversal
}

int main(int argc, char *argv[]) {
    if (argc < 3) { // Check if sufficient arguments are provided
        fprintf(stderr, "Usage: dtree <option> <root_dir> [additional args]\n");
        return EXIT_FAILURE; // Exit with error
    }

    const char *option = argv[1];  // Extract the operation type
    strncpy(src_root, argv[2], MAX_PATH); // Copy source directory path

    // Determine and execute the requested operation
    if (strcmp(option, "-ls") == 0) {
        nftw(src_root, list_callback, 20, FTW_PHYS);
    } else if (strcmp(option, "-ext") == 0 && argc == 4) {
        search_ext = argv[3];
        nftw(src_root, ext_callback, 20, FTW_PHYS);
    } else {
        fprintf(stderr, "Invalid option or missing arguments.\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS; // Exit successfully
}
