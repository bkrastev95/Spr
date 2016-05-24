#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>


#define LINESCOUNT 6
#define LINELENGTH 60
#define COMMANDLENGTH 30
#define WORDCOUNT 5
#define WORDLENGTH 15
#define BUFFSIZE 1

char** getCommandArray(char* commandLine);
char** splitLine(char* line);
void executeLine(char line[LINELENGTH]);
void executeSingle(char* command[]);
void executeWithPipe(char* command_1[], char* command_2[]);

int main()
{
	int fd, charCounter = 0;
	ssize_t b_read;
    char buffer[BUFFSIZE];
    char lineRead[LINELENGTH];

    // open file to read commands:
    fd = open("input.txt", O_RDONLY);
	if(fd == -1)
	{
		perror("read error");
		exit(1);
	}

	while((b_read = read(fd, &buffer, BUFFSIZE)) > 0)
	{
        if(buffer[0] != '\n')
        {
            lineRead[charCounter++] = buffer[0];
        }
        else
        {
            lineRead[charCounter] = '\0';
            executeLine(lineRead);
            memset(lineRead, 0, sizeof lineRead);
            charCounter = 0;
        }
	}

	close(fd);
	return 0;
}

void executeLine(char line[LINELENGTH])
{
    char* found;
    int i;

    // manage empty lines
    if(strlen(line) == 0)
    {
        return;
    }

    found = strstr(line, "|");
    if(found != NULL)
    {
        // manage two commands
        char ** commands;
        commands = splitLine(line);
        char **command_1 = getCommandArray(commands[0]);
        char **command_2 = getCommandArray(commands[1]);

        char* valid_1[WORDCOUNT];
        char* valid_2[WORDCOUNT];

        for(i = 0; i< WORDCOUNT; i++)
        {
            if(command_1[i] != NULL && command_1[i][0] != '\0')
            {
                valid_1[i] = command_1[i];
            }
            else
            {
                valid_1[i] =  (char*)0;
            }

            if(command_2[i] != NULL && command_2[i][0] != '\0')
            {
                valid_2[i] = command_2[i];
            }
            else
            {
                valid_2[i] = (char*)0;
            }
        }

        executeWithPipe(valid_1, valid_2);

        //derefference found
        found = NULL;
    }
    else
    {
        // manage single command
        char** command = getCommandArray(line);
        char* valid[WORDCOUNT];

        for(i = 0; i< WORDCOUNT; i++)
        {
            if(command[i] != NULL && command[i][0] != '\0')
            {
                valid[i] = command[i];
            }
            else
            {
                valid[i] =  (char*)0;
            }
        }

        executeSingle(valid);
    }
}

void executeWithPipe(char* command_1[], char* command_2[])
{
	int pipefd[2];
	int pid;

	pipe(pipefd);

	pid = fork();

	if (pid == -1)
	{
		perror("fork failed");
	}
	else if (pid == 0)
	{
		// in from pipe
		dup2(pipefd[0], 0);
		close(pipefd[1]);
		execvp(*command_2, command_2);
		printf("error on execution of second command:\n%s\n", *command_2);
	}
	else
	{
		pid = fork();

		if (pid == -1)
		{
			perror("fork failed");
		}
		else if(pid == 0)
		{
			// out to pipie
			dup2(pipefd[1], 1);
			close(pipefd[0]);
			execvp(*command_1, command_1);
			printf("error on execution of first command:\n%s\n", *command_1);
		}
		else
		{
			int result;
			waitpid(pid, &result, 0);
			if(result == 0)
			{
				printf("\n\nMultiple finnished: %s and %s \n\n", *command_1, *command_2);
			}
		}
        }
}

void executeSingle(char* command[])
{
	int pid;
	pid = fork();

	if (pid == -1)
	{
		perror("fork failed");
	}
	if(pid == 0)
	{
		execvp(*command, command);
		printf("error on execution of command:\n%s\n", *command);
	}
	else
	{
		int result;
		waitpid(pid, &result, 0);
		if(result == 0)
		{
			printf("\n\nSingle finnished: %s \n\n", *command);
		}
	}
}

char** getCommandArray(char* commandLine)
{
    int i;
    char ** commandWords = malloc(WORDCOUNT * sizeof(char*));
    for (i =0 ; i < WORDCOUNT; ++i)
    {
        commandWords[i] = malloc(WORDLENGTH * sizeof(char));
    }

    // pointer to array for strtok to work:
    char command[COMMANDLENGTH];
    memset(command, 0, sizeof command);
    strncpy(command, commandLine, COMMANDLENGTH -1);
    command[COMMANDLENGTH -1] = '\0';

    // get arguments
    char* token = strtok(command, " ");

    i = 0;
    while(token != NULL)
    {
        // fill commandWords array with words of the command:
        strcpy(commandWords[i++], token);

        // get next token
        token = strtok(NULL, " ");
    }

    return commandWords;
}

char** splitLine(char* line)
{
	int i;

	// allocate memory for char** pointing to the commands:
	// memory is 2 x COMMANDLENGTH
    char ** twoCommands = malloc(2 * sizeof(char*));
    for (i =0 ; i < 2; ++i)
    {
        twoCommands[i] = malloc(COMMANDLENGTH * sizeof(char));
    }

    // convert pointer to array for strtok to work:
    char lineToArray[LINELENGTH];
    strncpy(lineToArray, line, LINELENGTH -1);
    lineToArray[LINELENGTH -1] = '\0';


	char* token;
	i = 0;
    // split on | to receive two commands:
	token = strtok(lineToArray,"|");
	while(token != NULL)
	{
        // assign each command to array element:
        strcpy(twoCommands[i++], token);
		token = strtok(NULL, "|");
	}

	return twoCommands;
}

