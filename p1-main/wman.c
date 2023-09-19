#include <stdio.h>
#include <string.h>
#include <dirent.h>


/**
 * This function will read a file and print it out
 *
 * @input file_path, the path of file
 *
 * @return 1, if we fail to open the directory
 * @return 0, if we do everything right
 *
 */
int read_and_print(char* file_path){

    //1. Open the file for reading
    FILE* file = fopen(file_path, "r");

    //2. Check if the file was successfully opened
    //If the program tries to fopen() a file and fails, it should print the
    //exact message "cannot open file\n" and exit with status code 1.
    if (file == NULL) {
        printf("cannot open file\n");
        return 1; // Exit with an error code
    }

    //3. Read and display the contents of the file line by line
    const int file_size = 256;
    char line[file_size];
    while (fgets(line, file_size, file) != NULL) {
        printf("%s", line);
    }

    //4. Close the file
    fclose(file);
    return 0; // Exit successfully
}

/**
 * This function will read every file under a directory, and read it by calling read_and_print
 *
 * @input path, the directory we are about to search for
 * @input target_name, is in the format of <page>.<section>, read_and_print will only triggered if there is a match of name
 *
 * @return 0, there is a match of page and no error
 * @return 1, cannot open directory
 * @return -1, no match in the directory
*/
int strict_search_files(const char* path, const char* target_name) {
    DIR *dir;
    struct dirent *entry;

    //1. Open the directory
    dir = opendir(path);
    //check if we successfully open the directory
    if (dir == NULL) {
        perror("Error opening directory");
        return 1;
    }

    //2. Read and print file with file names
    while ((entry = readdir(dir))) {
        //Compare current file's name with our target
        if(strcmp(target_name, entry->d_name)==0){
            //Here is match, we should read and print the file line by line
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, target_name);
            closedir(dir);
            // Exit after reading the file
            return read_and_print(full_path);;
        }    
    }
    //3.No match of page
    closedir(dir);
    return -1;
}

int main(int argc, char *argv[]) {
    //printf("Number of arguments: %d\n", argc-1);

    // 1.Check if there are any command-line arguments
    if (argc < 2) {
        //No page is specified
        printf("What manual page do you want?\nFor example, try 'wman wman'\n");
        return 0; // Exit with an error code
    }
    // 2. Handle one argument execution, search the command name in each subfile
    //With a single argument, all sub-directories are searched
    else if (argc == 2)
    {
        //2.1 Set the root of manpage
        const char* root_dir = "./man_pages/man";
        //2.2 Initialize variables to read file
        char* page = argv[1];
        char filepath[256];
        char filename[256];

        //2.3 Use a for-loop to go through man1-man9 sub-directory
        for(int section = 1; section <= 9; section++) {
            //Load sub-directory's relative path to variables
            snprintf(filepath, sizeof(filepath), "%s%d", root_dir, section);
            snprintf(filename, sizeof(filename), "%s.%d", page, section);
            // printf("%s\n",filepath);
            // printf("%s\n",filename);

            //2.4 Run strict search file for each subdirectory
            // Strict search files will return 0 when we see a match, so we are happy to return 0 here
            int result = strict_search_files(filepath,filename);
            if(result == 0){
                return 0;
            }
            if(result ==1){
                return result;
            }
        }
        //2.5 Here there is no match in any sub-directory
        //If no such file can be found the program should print out exactly "No manual entry for page\n"
        printf("No manual entry for %s\n", page);    
        return 0; //no error, return success
    }
    // 3. Handle two arguments execution
    //search only within section of the manual for the wman command within specified number:
    else if (argc == 3){
        //3.1 Initialize variables to split input
        const char* root_dir = "./man_pages/man";
        char* page = argv[2];
        char* section= argv[1];
        
        //3.2 Check the validation of input section: Not a decimal or not in range
        //If section is not a decimal number in the proper range then print "invalid section\n"
        // and exit with status code of 1
        if(strlen(section) != 1 || section[0] < '0' || section[0] > '9'){
            printf("invalid section\n");
            return 1; // return error code
        }

        //3.3 Initialize variables to read file
        char filepath[256];
        char filename[256];

        //3.4 Load filepath and name with input
        snprintf(filepath, sizeof(filepath), "%s%s", root_dir, section);
        snprintf(filename, sizeof(filename), "%s.%s", page, section);

        //3.5 Run strict search files in sub-directory
        //If return 0, we find something and everything goes well, we should return 0
        int result = strict_search_files(filepath,filename);
        if(result ==0){
            return 0;
        }
        else if(result == 1){
            return result;
        } else{
            //3.6 No man page find
            //  If no such file can be found print "No manual entry for page in section section\n"
            printf("No manual entry for %s in section %s\n", page, section);
            return 0;
        }
    }
    // 4. Check if there are more than two arguments
    else {
        printf("What manual page do you want?\nFor example, try 'wman wman'\n");
        return 0;
    }
}