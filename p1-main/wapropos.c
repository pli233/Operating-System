#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

// 1 for found, 0 not found
int get_name_liner(char *file_path, char *name_liner)
{

    // Open the file for reading
    FILE *file = fopen(file_path, "r");

    // Check if the file was successfully opened
    if (file == NULL)
    {
        printf("cannot open file\n");
        return 0;
    }

    const int size = 256;
    // Read and display the contents of the file line by line
    char line[size];
    while (fgets(line, size, file) != NULL)
    {
        // Check if the line contains the "NAME" field to start extracting
        if (strstr(line, "NAME") != NULL)
        {
            fgets(name_liner, size, file);
            // Remove leading whitespaces
            char *ptr = name_liner;
            while (*ptr != '-')
                ptr++;
            memmove(name_liner, ptr, strlen(ptr) + 1);
            fclose(file);
            return 1;
        }
    }

    // Close the file
    fclose(file);
    // If we reach here, it means the NAME Field wasn't found in the file
    return 0;
}

// 1 for found, 0 not found
int contain_keyword(char *file_path, char *keyword)
{

    // Open the file for reading
    FILE *file = fopen(file_path, "r");

    // Check if the file was successfully opened
    if (file == NULL)
    {
        printf("cannot open file 1\n");
        return 0;
    }

    const int size = 256;
    // Read and display the contents of the file line by line
    char line[size];
    
    //int relevant = 0;
    int count =0;
    while (fgets(line, size, file) != NULL)
    {   
        char* ptr = line;
        char content[size];
        strcpy(content,line);
        int isField = 1;
        printf("Reading line: %s", line);  // Print every line being read
        while (*ptr != '\0')
        {
            if (!isspace((unsigned char)*ptr) && isupper((unsigned char)*ptr))
            {
                printf("%c", *ptr);
                isField = 0;
                break;
            }
            ptr++;
        }
        if (isField)
        {
            printf("Field line: %s", content);  // Print lines meeting the isField condition
        }
    
    count++;
    }
    printf("%d\n", count);
    fclose(file);
    return 0;
}

/**
 * This function will search files in a directory, and check if they contain keyword
 * If the keyword exist in file, we return 1 and print the name liner.
 */
int strict_search_files(char *path, char *keyword, int section)
{
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(path);

    if (dir == NULL)
    {
        perror("Error opening directory");
        return 1;
    }
    int check = 0;
    // Read and print file and directory names
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        char *filename = entry->d_name;
        // Get the page part of the filename
        char *page = strtok(filename, ".");
        // Get the section part of the filename
        char *sec = strtok(NULL, ".");

        if (page && sec && atoi(sec) == section)
        {

            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s.%s", path, page, sec);
            // printf("fullpath: %s\n", full_path);
            // printf("page: %s\n", page);
            // printf("section: %s\n", sec);
            // Check if the file contain keyword
            if (contain_keyword(full_path, keyword))
            {
                // Key word exist, print info
                check = 1;
                char name_liner[256];
                get_name_liner(full_path, name_liner);
                printf("%s (%s) %s", page, sec, name_liner);
            }
        }
    }
    closedir(dir);
    if (check)
    {
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    // printf("Number of arguments: %d\n", argc-1);

    // Base Case: Check if there are any command-line arguments
    if (argc < 2)
    {
        printf("What manual page do you want?\nFor example, try 'wman wman'\n");
        return 1; // Exit with an error code
    }
    // Case1: Handle one argument execution, search the page name in each subfile
    else if (argc == 2)
    {
        const char *root_dir = "./man_pages/man";
        char *keyword = argv[1];
        char filepath[256];
        int check = 0;
        for (int section = 1; section <= 9; section++)
        {
            snprintf(filepath, sizeof(filepath), "%s%d", root_dir, section);
            if (strict_search_files(filepath, keyword, section))
            {
                check = 1;
            }
        }
        if (check)
        {
            return 0;
        }
        else
        {
            printf("No manual entry for %s\n", keyword);
            return 1;
        }
    }
    // Case2: Handle two arguments execution
    else if (argc == 3)
    {
        const char *root_dir = "./man_pages/man";
        char *page = argv[2];
        char *section = argv[1];

        if (strlen(section) != 1 || section[0] < '0' || section[0] > '9')
        {
            printf("invalid section\n");
            return 1;
        }
        char filepath[256];
        char filename[256];

        snprintf(filepath, sizeof(filepath), "%s%s", root_dir, section);
        snprintf(filename, sizeof(filename), "%s.%s", page, section);

        if (strict_search_files(filepath, filename, atoi(section)) == 0)
        {
            return 0;
        }
        printf("No manual entry for %s in section %s\n", page, section);
        return 1;
    }
    // Case3: Check if there are more than two arguments
    else
    {
        printf("What manual page do you want?\nFor example, try 'wman wman'\n");
        return 1;
    }

    return 0;
}