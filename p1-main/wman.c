#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int read_and_print(char* file_path){

    // Open the file for reading
    FILE* file = fopen(file_path, "r");

    // Check if the file was successfully opened
    if (file == NULL) {
        printf("Error opening the file.\n");
        return 1; // Exit with an error code
    }

    //Get the number of lines in the file
    fseek(file, 0L, SEEK_END);
    const int file_size = ftell(file);
    rewind(file);

    // Read and display the contents of the file line by line
    char line[file_size]; // You can adjust the buffer size as needed
    while (fgets(line, file_size, file) != NULL) {
        printf("%s", line);
    }

    // Close the file
    fclose(file);
    return 0; // Exit successfully
}

/**
 * This function will return the 
*/

void search_files(const char* path, const char* target_name) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(path);

    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    // Read and print file and directory names
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name, target_name) == 0) {
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, target_name);
            read_and_print(full_path);
            return; // Exit after finding the file
        }
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            search_files(full_path, target_name); // Recursively search directories
        }
    }
    closedir(dir);
}

int main() {
    const char* folderPath = "/path/to/your/folder";
    const char* targetFileName = "your_target_file.txt";
    search_files(folderPath, targetFileName);
    return 0;
}




int main(int argc, char *argv[]) {
    printf("Number of arguments: %d\n", argc-1);


    // Base Case: Check if there are any command-line arguments
    if (argc <2) {
        printf("No command-line arguments provided.\n");
        return 1; // Exit with an error code
    }
    // Case1: Handle one argument execution
    else if (argc-1 == 1)
    {
        char* filename = argv[1];
    }
    // Case2: Handle two arguments execution
    else if (argc-1 == 2){

    }
    // Case3: Check if there are more than two arguments
    else{
         printf("Too many arguments provided.\n");
        return 1;
    }
}