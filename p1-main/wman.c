#include <stdio.h>
#include <string.h>
#include <dirent.h>
int read_and_print(char* file_path){

    // Open the file for reading
    FILE* file = fopen(file_path, "r");

    // Check if the file was successfully opened
    if (file == NULL) {
        printf("cannot open file\n");
        return 1; // Exit with an error code
    }

    // //Get the number of lines in the file
    // fseek(file, 0L, SEEK_END);
    const int file_size = 256;
    // rewind(file);

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
int strict_search_files(const char* path, const char* target_name) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(path);

    if (dir == NULL) {
        perror("Error opening directory");
        return 1;
    }

    // Read and print file and directory names
    while ((entry = readdir(dir))) {
        //printf("%s\n", entry->d_name);
        //Case1: Reach out a Regular File 
        if(strcmp(target_name, entry->d_name)==0){
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, target_name);
            closedir(dir);
            return read_and_print(full_path);; // Exit after finding the file
        }    
    }
    closedir(dir);
    return 1;
}

int main(int argc, char *argv[]) {
    //printf("Number of arguments: %d\n", argc-1);

    // Base Case: Check if there are any command-line arguments
    if (argc < 2) {
        printf("What manual page do you want?\nFor example, try 'wman wman'\n");
        return 1; // Exit with an error code
    }
    // Case1: Handle one argument execution, search the command name in each subfile
    else if (argc == 2)
    {
        const char* root_dir = "./man_pages/man";
        char* page = argv[1];
        char filepath[256];
        char filename[256];
        for(int section = 1; section <= 9; section++) {
            snprintf(filepath, sizeof(filepath), "%s%d", root_dir, section);
            snprintf(filename, sizeof(filename), "%s.%d", page, section);
            // printf("%s\n",filepath);
            // printf("%s\n",filename);
            if(strict_search_files(filepath,filename) == 0){
                return 0;
            }
        }
        printf("No manual entry for %s\n", page);    
        return 0;
    }
    // Case2: Handle two arguments execution
    else if (argc == 3){
        const char* root_dir = "./man_pages/man";
        char* page = argv[2];
        char* section= argv[1];

        if(strlen(section) != 1 || section[0] < '0' || section[0] > '9'){
            printf("invalid section\n");
            return 1;
        }
        char filepath[256];
        char filename[256];

        snprintf(filepath, sizeof(filepath), "%s%s", root_dir, section);
        snprintf(filename, sizeof(filename), "%s.%s", page, section);

        if(strict_search_files(filepath,filename) == 0){
            return 0;
        }
        printf("No manual entry for %s in section %s\n", page, section);
        return 0;
    }
    // Case3: Check if there are more than two arguments
    else {
        printf("What manual page do you want?\nFor example, try 'wman wman'\n");
        return 0;
    }
}