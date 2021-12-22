#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#include <ctype.h>

extern int errno; //error variable

bool isEmptyString(const char *charac) { //helper function to check if program input is all spaces
    while(*charac) {
        if (!isspace(*charac)) { //return false if a non-space char is found
            return false;
        }
        charac++;
    }

    return true;
}

int main() {
    char path[512]; //char array to hold current directory path
    char mysh[] = "$mysh> "; //to be appended to terminal path prompt
    const char space[2] = " "; //delimiter for strtok
    char input[30]; //user input
    char *tokenPtr; //strtok method result pointer
    char *cmpString = "exitpwdcd"; //string to compare to when checking if exit, pwd, or cd was entered
    int errnum;

    while(1) {

        if (getcwd(path, sizeof(path)) != NULL) {
           printf("%s%s", path, mysh); //print out the current directory path (default prompt)
        }

        if (fgets(input, 30, stdin) != NULL) {
            input[strcspn(input, "\n")] = 0; //filter out \n in input using strcspn

            if (isEmptyString(input)) { //call helper function
                continue;
            }

            tokenPtr = strtok(input, space);

            char *ret;
            ret = strstr(cmpString, tokenPtr); //check to see if exit, pwd, cd was entered

            if (ret) {
                if (strcmp(tokenPtr, "exit") == 0) { //handles exit command
                   if (strtok(NULL, space) == NULL) {
                      exit(0);

                    } else {
                       fprintf(stderr, "%s\n", "Error"); 
                    }

                } else if (strcmp(tokenPtr, "pwd") == 0) { //handles pwd command
                    printf("%s\n", path);

                } else if (strcmp(tokenPtr, "cd") == 0) { //handles cd case
                    tokenPtr = strtok(NULL, space); //advance to next token in strtok

                    if (chdir(tokenPtr) != 0) { //if error
                        if (errno == ENOENT) {
                            errnum = errno; //assign error number
                            fprintf(stderr, "%s%d\n", "No such directory: Errno = ", errnum); //print out error

                        } else {
                            fprintf(stderr, "%s%d\n", "Change directory failed! Errno = ", errnum);
                        }
                    }
                }

            } else { //handles external command cases
                char *enteredArgs[256] = {}; //entered external command argument array
                char *command = tokenPtr;
                int counter = 0;

                while (tokenPtr != NULL) { //loop through tokenPtr and add tokens to enteredArgs array
                    enteredArgs[counter] = tokenPtr;
                    tokenPtr = strtok(NULL, space);
                    counter++;
                }

                enteredArgs[counter] = NULL; // assign the last array element to null

                int cp = fork();

                if (cp == 0) { //fork returns to child, call execvp with command
                    execvp(command, enteredArgs);
                    if (execvp(command, enteredArgs) == -1) { //else print error and exit to parent 
                        fprintf(stderr, "%s%d\n", "Failed to execute command: Errno = ", errno);
                        exit(0);
                    }

                } else if (cp < 0) { //fork failed, immediately return to parent
                    printf("%s\n", "Failed to create process");
                    exit(0);

                } else { //if return to parent, wait for child to finish
                    wait(NULL);
                }
            }
          } 
    }
}



