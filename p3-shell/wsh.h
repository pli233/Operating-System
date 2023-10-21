#ifndef WSH_H
#define WSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Shell Running function */

void wsh_loop(FILE* input);                           // Main shell loop
char* read_line(FILE* input);                  // Read a line from input
char** split_line(char* line);                 // Split a line into arguments
int execute_command(int argc, char** args);    // Execute a command

// Struct for jobs
typedef struct job {
    int id;           // Job ID
    pid_t pid;        // Process ID
    char* command;    // Command string
    struct job* next; // Pointer to next job
    int background;   // 1 for background, 0 for foreground
} job;


/*Job management functions*/

//function for job list
int get_nextid(void);                          // Get the next available job ID
char* convert_to_string(char** args);          // Convert char** args to a single string
int add_job(pid_t pid, char** command, int argc, int background); // Add a job to the list
void print_jobs(void);                         // Print the list of jobs

//Getter
job* get_job_by_id(int id);                    // Get a job by its ID
job* get_job_by_pid(pid_t pid);                // Get a job by its PID
job* get_job_BGFG(int id);                     // Get a job for moveFront or moveBack

//Setters
void remove_job_byID(int id);                  // Remove a job from the list by ID
void remove_job_byPID(pid_t pid);              // Remove a job from the list by PID


/* Signal Handler */
void handle_sigint(int sig);                   // Handler for SIGINT (Ctrl-C)
void handle_sigtstp(int sig);                  // Handler for SIGTSTP (Ctrl-Z)
void handle_sigchld(int sig);                  // Reap finished background programs

#endif // WSH_H
