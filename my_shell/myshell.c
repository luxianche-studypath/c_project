#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void loop(void);

char *read_line(void);

char **split_line(char *);
char *str_tok(const char *);
bool ChInStr(char, char *);

int execute(char **);
int launch(char **);
//the current directory is a property of a process
//the number of builtin functions
int builtin_count();
//these are builtin functions
int shell_cd(char **);
int shell_help(char **);
int shell_exit(char **);
//list of builtin commands and their corresponding functions
char *builtin_str[] = {
    "cd",
    "help",
    "exit"};
int (*builtin_func[])(char **) = {
    &shell_cd,
    &shell_help,
    &shell_exit};

int main(int argc, char **argv)
{
    loop();
    return EXIT_SUCCESS;
}

void loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("$ ");
        line = read_line();
        args = split_line(line);
        status = execute(args);
    } while (status);
}

#define BUFF_SIZE 1024
char *read_line(void)
{
    int buffsize = BUFF_SIZE;
    char ch;
    int position = 0;

    char *buffer = malloc(sizeof(char) * buffsize);
    if (buffer == NULL)
    {
        puts("shell: allocation error");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        ch = getchar();
        if (ch == EOF || ch == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = ch;
        }
        position++;

        if (position >= BUFF_SIZE)
        {
            buffsize += BUFF_SIZE;
            buffer = realloc(buffer, buffsize);
            if (buffer == NULL)
            {
                puts("shell: allocation error");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define TOK_BUFF_SIZE 64
#define TOK_DELIT " \t\r\n\a"
char **split_line(char *line)
{
    int buffsize = TOK_BUFF_SIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char *) * buffsize);
    char *token;

    if (tokens == NULL)
    {
        puts("shell: allocation error");
        exit(EXIT_FAILURE);
    }

    token = str_tok(line);
    while (token != NULL)
    {
        tokens[position++] = token;

        if (position >= buffsize)
        {
            buffsize += TOK_BUFF_SIZE;
            tokens = realloc(tokens, buffsize);
            if (tokens == NULL)
            {
                puts("shell: allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = str_tok(line);
    }
    tokens[position] = NULL;
    return tokens;
}
//return token one by one when called
//set start,end to 0 when detect to '\0'
//regard "good luck" & 'have fun' as one token
char *str_tok(const char *line)
{
    static int start = 0;
    static int end = 0;
    int position = start;

    char ch;
    int token_length;
    char *token;

    bool InQuotes = false;
    static bool End = false;

    if (End)
    {
        start = 0;
        end = 0;
        End = false;
        return NULL;
    }

    while (true)
    {
        ch = line[position++];
        end++;

        if (ChInStr(ch, "\"\'"))
        {
            //toggle
            InQuotes ^= 01;
        }

        if ((ChInStr(ch, TOK_DELIT) || (ch == '\0')) && !InQuotes)
        {
            token_length = end - start;
            //Handle the case where delimiters are connected
            if (token_length > 1)
            {
                //malloc for token
                token = (char *)malloc(sizeof(char) * token_length);
                if (token == NULL)
                {
                    puts("shell: allocation error");
                    exit(EXIT_FAILURE);
                }

                //copy string from line to token
                for (int i = 0; i < token_length - 1; i++)
                    token[i] = line[start++];
                token[token_length - 1] = '\0';
            }

            start = end;

            if (ch == '\0')
                End = true;

            //return
            return token;
        }
    }
}

bool ChInStr(char ch, char *str)
{
    char c;
    int i = 0;
    while ((c = str[i++]) != '\0')
    {
        if (ch == c)
            return true;
    }
    return false;
}

int launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        //child process
        //execute the program in a new process
        if (execvp(args[0], args) == -1)
        {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        //print error information with process name
        perror("shell");
    }
    else
    {
        //parent process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
            //WUNTRACED : return status for stopped children
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        //WIFEXITED : STATUS indicates normal termination.
        //WIFSIGNALEDN : STATUS indicates the child is stopped
    }

    return 1;
}

int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "shell: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("shell");
        }
    }
    return 1;
}

int builtin_count()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_help(char **args)
{
    int i;
    puts("Luxianche's Shell");
    puts("type command name and arguments, and hit [Enter]");
    puts("The Following are built in:");
    for (i = 0; i < builtin_count(); i++)
    {
        printf("    %s\n", builtin_str[i]);
    }

    puts("Use the man command for informaitno on other programs.");
    return 1;
}

int shell_exit(char **args)
{
    return 0;
}

int execute(char **args)
{
    if (args[0] == NULL)
    {
        //an empty command
        return 1;
    }

    for (int i = 0; i < builtin_count(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
            return (*builtin_func[i])(args);
    }

    return launch(args);
}
