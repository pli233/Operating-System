#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>




/* @Input path, the path of the folder that we will read
 * @Input keyword, the keyword we check for match
 * @Input section, the section number we will search
 *
 * @return 1, if there is some keyword match in NAME or DESCRIPTION field at assigned section
 * @return 0, if there is no match
 */
int strict_search_files(char* path, char* keyword, int section);
/* @Input file_path, the path of the file that we will read
 * @Input keyword, the keyword we check for match
 *
 * The function will find if there is a match of keyword in the field of NAME or DESCRIPTION
 *
 *
 * @return 1, if there is a match at assigned field
 * @return 0, if there is no match
 */
int contain_keyword(char* file_path, char* keyword);
/* @Input file_path, the path of the file that we will read
 * @Input name_liner, the variable that we assign content back to the calling program
 *
 * The function will read the field of NAME, and return the one line name_liner
 *
 *
 * @return 1, if there is a match at assigned field
 * @return 0, if there is no match
 */
int get_name_liner(char* file_path, char* name_liner);

/*
 * Main gate for program
 *
 */
int main(int argc, char* argv[]) {
    // printf("Number of arguments: %d\n", argc-1);

    // Base Case: Check if there are any command-line arguments
    if (argc < 2) {
        printf("wapropos what?\n");
        return 0; 
    }

    // Case1: Handle one argument execution, search the page name in each sub-dir
    else if (argc == 2) {
        const char *root_dir = "./man_pages/man";
        char *keyword = argv[1];
        char filepath[256];
        int check = 0;
        for (int section = 1; section <= 9; section++) {
            //printf("%d\n", section);
            snprintf(filepath, sizeof(filepath), "%s%d", root_dir, section);
            if (strict_search_files(filepath, keyword, section)) {
                check = 1;
            }
        }
        if (check == 1) {
            return 0;
        } else {
            printf( "nothing appropriate\n");
            return 0;
        }
    }
    // Case2: Handle two arguments execution
    else if (argc == 3) {
        const char *root_dir = "./man_pages/man";
        char *keyword = argv[2];
        char *section = argv[1];

        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s%s", root_dir, section);
        int result = strict_search_files(filepath, keyword, atoi(section));
        //There is match
        if (result>0) {
            return 0;
        }
        //There is no match
        else if(result==0) {
            printf( "nothing appropriate\n");
            return 0;
        }
        //Error opening file
        else{
            return 1;
        }
    }
    // Case3: Check if there are more than two or less than one arguments
    else {
        //If no keyword is provided on the command line then the program should print
        // "wapropos what?\n" and exit with code 0.
        printf("wapropos what?\n");
        return 0;
    }
}




/**
 *
 * @Input path, the path of the folder that we will read
 * @Input keyword, the keyword we check for match
 * @Input section, the section number we will search
 *
 * The function will also print the info we need by calling get_name_liner
 *
 *
 * @return 1, if there is some keyword match in NAME or DESCRIPTION field at assigned section
 * @return 0, if there is no match
 * @return match, which is the number of match
 */
int strict_search_files(char* path, char* keyword, int section) {
    DIR *dir;
    struct dirent *entry;
    // 1.Open the directory
    dir = opendir(path);
    //if we cannot open the directory, we return 1 and print out error information
    if (dir == NULL) {
        perror("Error opening directory");
        return -1;
    }
    // 2.Read the files inside the directory, find files contain keyword and print info
    int match = 0;
    while ((entry = readdir(dir)) != NULL) {

        //2.1 Get rid of directories whose names are . or ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        //2.2 Get the filename of our current file, store it to a variable
        char *filename = entry->d_name;

        //2.3 Split out the page and section part of the filename
        // Get the page part of the filename
        char *page = strtok(filename, ".");
        // Get the section part of the filename
        char *sec = strtok(NULL, ".");

        //2.4 Check if the file exist, and the string of section is correctly corresponding to the section number we input
        if (page && sec && atoi(sec) == section) {
//            printf("page: %s\n", page);
//            printf("section: %s\n", sec);

            //2.5 Create the full_path of the file, and read the content of it
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s.%s", path, page, sec);

            //2.6 Check if the file contain keyword
            //printf("Calling result of contain keyword: %d\n", contain_keyword(full_path, keyword));
            if (contain_keyword(full_path, keyword)) {
                //2.7 If keyword exist, print info with get_name_liner
                char name_liner[256];
                get_name_liner(full_path, name_liner);
                //2.8 Print out the information
                //The output list must be in the form:
                //<page> (<section>) - <name_one_liner>
                printf("%s (%s) %s", page, sec, name_liner);
                //2.9 Record a match
                match++;
            }
        }
    }
    //3 Close the directory, print the number of matches we find.
    closedir(dir);
    return match;
}


/**
 * Check if the file contain keyword in NAME or DESCRIPTION field
 *
 * @Input file_path, the path of the file that we will read
 * @Input keyword, the keyword we check for match
 *
 * The function will find if there is a match of keyword in the field of NAME or DESCRIPTION
 *
 *
 * @return 1, if there is a match at assigned field
 * @return 0, if there is no match
 */
int contain_keyword(char* file_path, char* keyword) {

    // 1.Open the file for reading
    FILE *file = fopen(file_path, "r");

    // 1.1 Check if the file was successfully opened, if we cannot open the file, we return 0
    if (file == NULL) {
        printf("cannot open file\n");
        return 0;
    }

    //2.Define and initialize some variables
    //Define size for each line we read
    const int size = 256;
    char line[size];
    //Check if the line is relevant, initialize it to 0 as irrelevant
    int relevant = 0;


    //3.Check if the name or description field in the file contain our keyword line by line
    while (fgets(line, size, file) != NULL) {
        //3.1 Check if the line is empty
        //Use a pointer to point to the start of the line
        char* ptr = line;
        int isEmpty = 1;
        while(*ptr != '\0'){
            if (!isspace(*ptr)){
                isEmpty = 0;
                break;
            }
            ptr++;
        }

        //3.2 Check if the line is a field;
        int isField = 0;
        if (!isEmpty && !isspace(line[0])) {
            isField =1;
        }
        //3.3 If it is a field, we need to compare it to name and description
        if (isField) {
            //Check if it is NAME or DESCRIPTION field
            if (strstr(line, "NAME") || strstr(line, "DESCRIPTION")) {
                //We reach out a relevant field, set relevant to 1
                relevant = 1;
                //Use continue to read the next line
                continue;
            }
            //Otherwise, we reach out to the start of a new field, but irrelevant
            else {
                relevant = 0;
                continue;
            }
        }
        //4 If we reach here, it means that we are in a relevant section and we are reading content
        //Check if there is a keyword substring contain in the line
        //printf("relevant: %d\n", relevant);
        if (relevant) {
            //Use strstr to check substring
            if (strstr(line, keyword)) {
                //There is a match, return 1
                fclose(file);
                return 1;
            }
        }
    }
    //5 No keyword match, return 0
    fclose(file);
    return 0;
}


/* @Input file_path, the path of the file that we will read
 * @Input name_liner, the variable that we assign content back to the calling program
 *
 * The function will read the field of NAME, and return the one line name_liner
 *
 *
 * @return 1, if there is a match at assigned field
 * @return 0, if there is no match
 */

int get_name_liner(char* file_path, char* name_liner) {

    //1. Open the file for reading
    FILE *file = fopen(file_path, "r");

    // Check if the file was successfully opened
    if (file == NULL) {
        printf("cannot open file\n");
        return 0;
    }

    //Initialize the maximum size of a line
    const int size = 256;
    char line[size];

    //2. Read and display the contents of the file line by line
    while (fgets(line, size, file) != NULL) {
        // Check if the line contains the "NAME" field to start extracting
        if (strstr(line, "NAME") != NULL) {
        //Read the next line into name_liner after we detect the NAME field to read content
            fgets(name_liner, size, file);
            // Initialize a pointer to head, move it to '-'
            char *ptr = name_liner;
            while (*ptr != '-')
                ptr++;
            // Delete strings before '-'
            memmove(name_liner, ptr, strlen(ptr) + 1);
            //Close the file and return 1
            fclose(file);
            return 1;
        }
    }
    //No NAME field, Close the file and return
    fclose(file);
    return 0;
}





