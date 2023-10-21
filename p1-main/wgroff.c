#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>

//return 0 if nothing wrong, return 1 if we see there is something wrong
int checkFirstLine(FILE* file, char* line, int size){
    char command[size], date[size];
    int section;
    //3.1 check if the first line is null
    if(fgets(line, size, file) == NULL){
        return 1;
    }
    //3.2 check if the information provided in the first line start with .TH and contains command, section, and date
    if(sscanf(line, ".TH %s %d %s", command, &section, date) != 3){
        return 1;
    }
    //3.3 command is just a char*, no need to check validation
    //3.4 section should be a integer between 1-9, check if it is correctly formatted
    if(section<1 || section>9){
        return 1;
    }
    //3.5 check the format of data follow YYYY-MM-DD
    int year;
    int month;
    int day;
    if(sscanf(date, "%4d-%2d-%2d", &year, &month, &day) != 3){
        //printf("Incorrect format of date, not following YYYY-MM-DD\n");
        return 1;
    }
    int large_month[] = {1,3,5,7,8,10,12};
    //int small_month[] = {4,6,9,11};
    int special_month = 2;
    if(year<0 || year > 9999 || month<1 || month> 12 || day< 1){
        //printf("Incorrect information about date\n");
        return 1;
    }
    if(month == special_month){
        //Leap year
        if(year%4 ==0){
            if(day>29){
                return 1;
            }
        }
        else {
            if(day>28){
                return 1;
            }
        }
    }
    else{
        int is_large_month = 0;
        for(int i = 0; i < sizeof(large_month)/sizeof(int); i++) {
            if(month == large_month[i]) {
                is_large_month =1;
                break;
            }
        }
        if(is_large_month) {
            if(day > 31) {
                return 1;;
            }
        }
        else {
            // Now we know the month is not February and not a large month, so it must be a small month
            if(day > 30) {
                return 1;
            }
        }
    }
    return 0;
}

//Make a string to Upper Case
int str_to_upper(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = toupper((unsigned char) str[i]);
    }
    return 0;
}

//format_line
void format_line(char *line) {

    const int size = 256;  // or whatever maximum size you expect
    char formatted_line[size];
    char* ptr = line;
    char* format_ptr = formatted_line;

    while (*ptr != '\0') {
        if (strncmp(ptr, "/fB", 3) == 0) {
            strcpy(format_ptr, "\033[1m");
            format_ptr += 4;
            ptr += 3;
        } else if (strncmp(ptr, "/fI", 3) == 0) {
            strcpy(format_ptr, "\033[3m");
            format_ptr += 4;
            ptr += 3;
        } else if (strncmp(ptr, "/fU", 3) == 0) {
            strcpy(format_ptr, "\033[4m");
            format_ptr += 4;
            ptr += 3;
        } else if (strncmp(ptr, "/fP", 3) == 0) {
            strcpy(format_ptr, "\033[0m");
            format_ptr += 4;
            ptr += 3;
        } else if (strncmp(ptr, "//", 2) == 0) {
            *format_ptr = '/';
            format_ptr += 1;
            ptr += 2;
        } else {
            *format_ptr = *ptr;
            format_ptr += 1;
            ptr += 1;
        }
    }
    *format_ptr = '\0';
    strcpy(line, formatted_line);
}


/*
 * This method will read the file line by line
 *
 *
 */
int readFile(char* file_path){
    // 1.Open the file for reading
    FILE *file = fopen(file_path, "r");

    // 1.1 Check if the file was successfully opened, if we cannot open the file, we return 0
    if (file == NULL) {
        printf("cannot open file\n");
        return 1;
    }


    //2.Define and initialize some variables
    //Define size for each line we read
    const int size = 256;
    char line[size], command[size], date[size], output_filename[size];
    int section;
    int line_index = 0;

    //3.Read the first line of the file
    line_index++;
    //If the function return 1, there is something wrong in the first line's format or content
    if(checkFirstLine(file, line, size)){
        printf("Improper formatting on line %d\n", line_index);
        fclose(file);
        return 0;
    }else{
        sscanf(line, ".TH %s %d %s", command, &section, date);
        sprintf(output_filename, "%s.%d", command, section);
    }
    //4.Create the output file and print the first line to it
    FILE *output_file = fopen(output_filename, "w");

    //4.1 Convert section to str
    char section_str[10];
    sprintf(section_str, "%d", section);

    //4.2 Write the first line to the output file
    //number of space should be 80-2*str len of command+section, 4 stand for ()()
    int padding_num = 80 - 2*((int)strlen(command)+(int)strlen(section_str))-4;
    //%s(%s) stand for command(section),    %-*s will add number of padding we want
    fprintf(output_file, "%s(%s)%-*s%s(%s)\n", command, section_str, padding_num, " ", command, section_str);

    //5.Read and the file line write the output file line by line
    while (fgets(line, size, file) != NULL) {
        //Default execution: update the line index by 1 to match the pointer of file
        line_index++;
        //5.1 Ignoring lines start with #
        if (line[0] == '#'){
            continue;
        }
        //5.2 Check if it is a section header
        else if(strncmp(line, ".SH", 3) == 0){
            char section_name[size];
            //Read in the section name, it should start with .SH and ignore the ending \n
            sscanf(line, ".SH %[^\n]", section_name);
            str_to_upper(section_name);
            fprintf(output_file, "\n");
            fprintf(output_file, "\033[1m%s\033[0m\n", section_name);
        }
        else{
            //format the line
            format_line(line);
            //indenting the content with 7 spaces
            fprintf(output_file, "%-*s%s", 7, " ",line);
        }
    }
    // Adding the last line with date centered
    padding_num = (80 - (int)strlen(date)) / 2;
    fprintf(output_file, "%*s%s%*s\n", padding_num, " ", date, padding_num, " ");

    fclose(file);
    fclose(output_file);
    return 0;
}


/*
 * Main gate for program
 *
 */
int main(int argc, char *argv[]) {

    // Base Case: Check if there are correct number of command-line arguments
    if (argc < 2) {
        printf("Improper number of arguments\nUsage: ./wgroff <file>\n");
        return 0; //  exit with code 0
    }
    //Ignore more than 1 argument for wgroff
    else{
        //Check if file exists
        if (access(argv[1], F_OK) == 0) {
            //Use method to read the file
            return readFile(argv[1]);
        }
        //If input file provided on the command line doesn't exist, the program should print
        //"File doesn't exist\n" and exit with code 0.
        printf("File doesn't exist\n");
        return 0;
    }
}