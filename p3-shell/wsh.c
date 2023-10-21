#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "wsh.h"


pid_t foreground_pid = -1;
pid_t shell_pgid = -1;

/*Main gate for shell*/
int main(int argc, char* argv[]) {
    FILE* batch_file = NULL;

    // If an argument is provided, try to open the file
    if (argc == 2) {
        batch_file = fopen(argv[1], "r");
        if (!batch_file) {
            perror("Error opening batch file");
            return EXIT_FAILURE;
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    wsh_loop(batch_file);

    if (batch_file) {
        fclose(batch_file);
    }
    return EXIT_SUCCESS;
}

/* Shell Running function */

//Function 1. Main Function to execute the shell loop
void wsh_loop(FILE* input) {

    if (tcsetpgrp(STDIN_FILENO, getpid()) < 0) {
        //perror("tcsetpgrp before setsid");
        exit(EXIT_FAILURE);
    }

    if (getpid() != getsid(0)) {
        // The shell is not a session leader
        if (setsid() < 0) {
            //perror("setsid");
        }
        //printf("New session created\n");
    }

    shell_pgid = getpgid(getpid());
    //3. Handle signals
    signal(SIGCHLD, handle_sigchld);    //Killed combie child process that finish their job in background
    signal(SIGTSTP, handle_sigtstp);    //Handle SIGTSTP, ctrl z
    signal(SIGINT, handle_sigint);      //Handle SIGINT, ctrl c

    //4.Declare variables to read the content of command line
    char *line = NULL;
    char **user_args;
    int status;

    //5. Type of mode, default to interactive mode, can be changed by setting up a different input
    // 5. Type of mode, default to interactive mode, can be changed by setting up a different input
    if (!input) {
        input = stdin;
    }

    /* 6. Main Loop for wsh shell prompt */
    do {
        // 6.1 Print out a prompt only if in interactive mode
        if (input == stdin) {
            printf("wsh> ");
        }

        //6.2 Read the line
        line = read_line(input);

        //6.3 Split the line into arguments, seperate by space, tab, etc.
        user_args = split_line(line);

        //6.4 Use a for loop to calculate number of argument
        int user_argc = 0;
        for (; user_args[user_argc] != NULL; user_argc++);

        //6.5 Execute the command, receive the status return by the function
        // 1 means no problem, 0 means something wrong, loop will continue when we have 1
        status = execute_command(user_argc, user_args);

        //6.6 Free the line and user_args to avoid memory leak
        free(line);
        free(user_args);

    } while (status); //6.6 Check the status code return by execution of command
}

//Function 2. Read a new line in wsh
char* read_line(FILE* input) {

    //1. buffer is the address of the first character position where the input string will be stored.
    char *buffer = NULL;
    //2. variable that holds the size of the input buffer
    size_t bufsize = 0;

    //3. Use get line to read from stdin
    if (getline(&buffer, &bufsize, input) == -1) {
        if (feof(input)) {
            // End of file was reached
            exit(EXIT_SUCCESS);
        } else {
            // Some error occurred while reading
            perror("wsh: getline");
            exit(EXIT_FAILURE);
        }
    }

    return buffer;
}

//Function 3. Split a line, return a char** represent all arguments, end by null
char** split_line(char* line){

    //1. Intialize the buffersize
    int bufsize = 64;

    //2. Initialize the start position of tokens
    int position = 0;

    //3. Initialize a char array pointer to store tokens
    char **tokens = malloc(bufsize * sizeof(char*));

    //4. Declare the first token;
    char *token;

    //5. Loop to extract each token from the line based on delimiters
    //To parse the input line into constituent pieces, you might want to use strsep()
    //strsep will found the position of delim we found and set it to \0
    while ((token = strsep(&line, " \t\r\n\a")) != NULL) {

        //5.1 get rid of tab space etc., do not include them into tokens
        // since strsep will return "" instead of NULL when it see them
        if (*token == '\0') continue;

        //5.2 Store the valid token in the tokens array
        tokens[position++] = token;

        //5.3 Check if we've reached the buffer limit
        if (position >= bufsize) {
            // If buffer limit reached, increase its size
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
        }
    }

    //6. Null-terminate the tokens array to mark it is end
    tokens[position] = NULL;

    //7. Return the array of extracted tokens
    return tokens;
}

//Function 4. Execute a command
//Fucntion suggested to use: fork() execvp() wait()/waitpid()
int execute_command(int argc, char** args) {

    //1. Base Case: no string is input, user could simply click enter or input several space tab etc.
    if (args[0] == NULL) {
        return 1; //simply continue my loop
    }

    //2. Check if the work need to be done with background mode
    int background = 0;
    if(strcmp(args[argc-1],"&") ==0){

        args[argc-1] = NULL;  // Remove the & before execution
        argc--;  // Decrement the number of arguments
        background = 1; // Set the background flag when adding the job
    }

    //3. if the user typed one of built-in command , we should return 0 to exit the whole program
    //list of built-in command: exit, cd, jobs, fg, bg
    if (strcmp(args[0], "exit") == 0) {
        return 0; //end the loop with 0
    }
    else if (strcmp(args[0], "cd") == 0) {
        //0 or >1 args should be signaled as an error, include cd itself, it must be 2 arguments
        if(argc!=2){
            //incorrect number of arguments pass to cd
            return 0;
        }
        //Execute cd with chdir()
        chdir(args[1]);
        return 1;
    }
    else if (strcmp(args[0], "jobs") == 0) {
        //call print jobs to print my job list
        print_jobs();
        return 1;
    }
    else if (strcmp(args[0], "fg") == 0) {
        int id = (argc == 2) ? atoi(args[1]) : -1;
        //1. Get the job first
        job* j = get_job_BGFG(id);
        if (j == NULL) return 1; // No job available or with given ID

        //2. Ensure the process is continued if it was stopped
        kill(j->pid, SIGCONT);

        //3. wait for the process to finish
        int status;
        foreground_pid = j->pid;
        waitpid(j->pid, &status, WUNTRACED);
        if(WIFEXITED(status)) {
            // If the process exited normally, remove it from your jobs list
            remove_job_byID(j->id); //If we dont have this compare, it could be problem that we stop the job again after fg
        }
        foreground_pid = -1;

        //4. return terminal control back
        tcsetpgrp(0, shell_pgid);
        return 1;
    }
    else if (strcmp(args[0], "bg") == 0) {

        int id = (argc == 2) ? atoi(args[1]) : -1;

        job* j = get_job_BGFG(id); //get the job id, it either largest or input

        //Base Case: No job available or with given ID
        if (j == NULL) return 1; 

        //Case1: Job exist, ensure the process is continued 
        kill(j->pid, SIGCONT);

        return 1;
    }
    else{
        /* Important part, core execution for command
         * It handle pipes, job add, and execution of outside command 
         */
        
        //** Pipeline Handler Part **

        //1. Detect and handle pipes
        //***   for the whole array                 a -s | b | c
        //**    for a command                       a -s
        //*     for a char* seperated by array      "a" "-s"
        char ***pipelines = malloc(sizeof(char **) * (argc + 1));
        int pipeline_count = 0;
        pipelines[0] = args;
        int job_id;

        //pid_t first_pid = -1;
        //2. Loop through command-line arguments to identify pipes and split the arguments
        for (int i = 0; i < argc; i++) {
            if (strcmp(args[i], "|") == 0) {
                args[i] = NULL;
                pipelines[++pipeline_count] = &args[i + 1];
            }
        }
        pipelines[pipeline_count + 1] = NULL;

        //3. Check if there are pipes to process
        if (pipeline_count > 0) {
            //pipe exist, handler work start here

            //3.1 declaring a two-dimensional integer array called pipes. 
            //This array is used to manage a set of pipes for communication between multiple processes in a pipeline.
            //pipes[n][0] represent read
            //pipes[n][1] represent write
            int pipes[pipeline_count][2];

            //3.2 Call pipe for each pipe
            for (int i = 0; i < pipeline_count; i++) {
                //check error
                if (pipe(pipes[i]) < 0) {
                    perror("wsh: pipe error");
                    free(pipelines);
                    return 0;
                }
            }

            //3.3 Handle redirection work for each pipe
            for (int i = 0; i <= pipeline_count; i++) {

                //Fork a Child Process:
                pid_t pid = fork();

                if (pid < 0) {
                    perror("wsh: fork error");
                    free(pipelines);
                    return 0;
                } 
                //child process works:
                else if (pid == 0) {
                    // Set PGID for child processes

                    if (i == 0) {
                        //first child
                        //first_pid = getpid();
                        job_id = add_job(pid, args, argc, background);
                        if (setpgid(0, 0) == -1) {
                            perror("wsh: setpgid error");
                            free(pipelines);
                            return 0;
                        }
                    }
                    //If this is not the first command in the pipeline (i > 0), 
                    //it redirects the standard input (file descriptor 0) of the child process 
                    //to the read end of the previous pipe (pipes[i - 1][0]). 
                    //This allows the child to read input from the previous command in the pipeline.
                    if (i > 0) {
                        if (setpgid(0, 0) == -1) {
                            perror("wsh: setpgid error");
                            free(pipelines);
                            return 0;
                        }
                        dup2(pipes[i - 1][0], 0);
                    }

                    //If this is not the last command in the pipeline (i < pipeline_count), 
                    //it redirects the standard output (file descriptor 1) of the child process 
                    //to the write end of the current pipe (pipes[i][1]). 
                    //This allows the child to send its output to the next command in the pipeline.
                    if (i < pipeline_count) {
                        dup2(pipes[i][1], 1);
                    }

                    //Closes all the pipe file descriptors after redirections
                    for (int j = 0; j < pipeline_count; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                    //Execute the Command:
                    if (execvp(pipelines[i][0], pipelines[i]) == -1) {
                        perror("execvp in pipe"); // Print an error message indicating the reason for failure
                        exit(EXIT_FAILURE); // Exit the child process with a failure code
                    }
                }
                //printf("group id: %d",getpgid(getpid()));
            }

            //Close all the pipe file 
            for (int i = 0; i < pipeline_count; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            //If the program is not running in the background, 
            //the parent process waits for all the child processes to finish using wait
            if (!background) {
                for (int i = 0; i <= pipeline_count; i++) {
                    wait(NULL);
                }
                remove_job_byID(job_id);
            }

            free(pipelines);
            return 1;
        }
        //call it is pipeline does not exist
        free(pipelines);


        /* Job control and single execute command */


        //4.1 Check if there is an executable program in path, modify /bin to change default path
        char exec_path[256];
        snprintf(exec_path, sizeof(exec_path), "/bin/%s", args[0]);

        //4.2. Use access to check if the command exists in /bin
        if (access(exec_path, X_OK) != 0) {
            fprintf(stderr, "wsh: command not found: %s\n", args[0]);
            //return 0 to indicate there is an error to end loop
            return 0;
        }

        //4.3 We found an executable file, Declare pid, status for execute it
        pid_t pid = fork(); //pid for child

        //4.4 pid <0 is an error, we need to end the loop
        if (pid < 0) {
            return 0;
        }
        //4.5 Child process execute the program
        else if (pid == 0) {
            //Reset SIGINT and SIFTSTP to default so that our handler did not interupt it
            //This is becuase child process after fork will inherit its father process's signal handle 
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            //Set child process to its own process group
            setpgid(0,0);

            //args contain NULL so we don't need to add NULL, call execvp
            if (execvp(exec_path, args) == -1) {
                return 0;
            }
        }
        
        //4.6 Main process do different thing for add job
        else {
            //initialize job id and status
            int status;
            if(background){
                //If it is a background process, dont remove it after add to job list
                //We will use sigchild to recycle it
                job_id = add_job(pid,args, argc, 1);
                return 1;
            }
            else{

                //If it is a foreground process, we wait for the child process to finish
                job_id= add_job(pid,args,argc,0);
                //Indicate shell a foreground process is running
                foreground_pid = pid;
                //Wait
                waitpid(pid, &status, WUNTRACED);
                //Determine exit status: stop will not cause shell to remove job
                if(WIFEXITED(status)) {
                    // If the process exited normally, remove it from your jobs list
                    remove_job_byID(job_id);
                }
                //For ctrl c or z, we have no foreground, so reset the indicater to -1
                foreground_pid = -1;
            }
            return 1;
        }
    }
    return 1;
}



/*Job management functions*/

//List of jobs that have not finish their work
job* jobs = NULL;

//Function 5. Get the id for next job, it should be smallest integer that we can add
int get_nextid(void){

    //1. Start with the smallest positive integer
    int id_candidate = 1;  

    //2. Keep searching until we find an available ID
    while (1) {  

        //Initialize a pointer to start of jobs
        job* current = jobs;
        // Flag to check if the current candidate ID is taken
        int is_taken = 0;  

        while (current) {
            if (current->id == id_candidate) {
                is_taken = 1;
                break;  // The ID is taken, break out of the inner loop
            }
            current = current->next;
        }

        if (!is_taken) {
            return id_candidate;  // Found an unused ID, return it
        }
        id_candidate++;  // Try the next ID
    }
}

//Function 6. Convert char** args to char* for printing convenient
char* convert_to_string(char** args) {

    // First, calculate the total length required
    int total_len = 0;
    for (int i = 0; args[i]; i++) {
        total_len += strlen(args[i]) + 1; // +1 for spaces or null terminator
    }
    // Allocate memory for the concatenated string
    char* result = malloc(total_len);
    result[0] = '\0'; // Initialize with empty string

    // Concatenate each string in args
    for (int i = 0; args[i]; i++) {
        strcat(result, args[i]);
        if (args[i + 1]) {
            strcat(result, " "); // Add space between arguments, but not at the end
        }
    }
    return result;
}

//Function 7. Dynamically allocates memory for a new job struct, add it to job list, and returns the id of it.
int add_job(pid_t pid, char** command, int argc, int background){

    //1. Create a new job
    job* new_job = malloc(sizeof(job));
    new_job->id = get_nextid();  // function to generate a unique job id
    new_job->pid = pid;
    new_job->command = convert_to_string(command);  // Make a copy of the command
    new_job->next = NULL;

    //2. Add job to job_list
    if (jobs == NULL) {
        jobs= new_job;
    }
    else {
        job* temp = jobs;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = new_job;
    }
    //printf("%d\n", pid);
    return new_job->id;
}

//Function 8. Print the job list
void print_jobs(void) {
    job* current = jobs;
    while (current) {
        printf("%d: %s%s\n", current->id, current->command, (current->background ? " &" : ""));
        current = current->next;
    }
}

//Function 9. Get a job by its ID
job* get_job_by_id(int id) {
    job* current = jobs;
    while (current != NULL) {
        if (current->id == id){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

//Function 10. Get a job by its PID
job* get_job_by_pid(pid_t pid) {
    job* current = jobs;
    while (current != NULL) {
        if (current->pid == pid) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

//Function 11. This function will generate the job for moveFront or moveBack
job* get_job_BGFG(int id) {

    //1. No ID provided, use largest
    if (id == -1) {
        job* current = jobs;
        job* maxJob = NULL;
        while (current != NULL) {
            if (maxJob == NULL || current->id > maxJob->id) maxJob = current;
            current = current->next;
        }
        return maxJob;
    }
    //2. Call get job by id
    else {
        return get_job_by_id(id);
    }
}

//Function 12. Remove a job from job list by id
void remove_job_byID(int id) {
    job* current = jobs;
    job* prev = NULL;

    while (current != NULL) {
        if (current->id == id) {
            if (prev) {
                prev->next = current->next;
            } else {
                jobs = current->next;
            }
            free(current->command);
            free(current);
            return;
        }
        //Update information for loop
        prev = current;
        current = current->next;
    }
}

//Function 13. Remove a job from job list
void remove_job_byPID(pid_t pid) {
    job* current = jobs;
    job* prev = NULL;

    while (current != NULL) {
        if (current->pid == pid) {
            if (prev) {
                prev->next = current->next;
            } else {
                jobs = current->next;
            }
            free(current->command);
            free(current);
            return;
        }
        //Update information for loop
        prev = current;
        current = current->next;
    }
}


/* Signal Handler */

//Function 14. Handler for SIGINT (Ctrl-C)
void handle_sigint(int sig) {
    // Get the PID of the foreground process
    pid_t fg_grp = tcgetpgrp(0);
    if (foreground_pid != -1) {
        kill(foreground_pid, SIGINT);
        remove_job_byPID(foreground_pid);
    }
    tcsetpgrp(0, fg_grp);
}

//Function 15. Handler for SIGTSTP (Ctrl-Z)
void handle_sigtstp(int sig) {
    // Get the PID of the foreground process
    pid_t fg_grp = tcgetpgrp(0);

    if (foreground_pid != -1) {
        kill(foreground_pid, SIGTSTP);
    }
    tcsetpgrp(0, fg_grp);
}

//Function 16. Reap finished background program
void handle_sigchld(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Find the job by PID and remove it
        remove_job_byPID(pid);
    }
}
