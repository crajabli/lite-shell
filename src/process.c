#include "builtins.h"
#include "hash.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

// The contents of this file are up to you, but they should be related to
// running separate processes. It is recommended that you have functions
// for:
//   - performing a $PATH lookup
//   - determining if a command is a built-in or executable
//   - running a single command in a second process
//   - running a pair of commands that are connected with a pipe

/* int checkifexecutable(const char *filename)
 *
 * Return non-zero if the name is an executable file, and
 * zero if it is not executable, or if it does not exist.
 */

int
checkifexecutable (const char *filename)
{
  int result;
  struct stat statinfo;

  result = stat (filename, &statinfo);
  if (result < 0)
    return 0;
  if (!S_ISREG (statinfo.st_mode))
    return 0;

  if (statinfo.st_uid == geteuid ())
    return statinfo.st_mode & S_IXUSR;
  if (statinfo.st_gid == getegid ())
    return statinfo.st_mode & S_IXGRP;
  return statinfo.st_mode & S_IXOTH;
}

int
pathLookup (char *path, char *command)
{
  char *searchPath;
  char *copy, *end;
  int stop, found;
  int len;

  if (strchr (command, '/') != NULL)
    {
      return 0;
    }

  searchPath = getenv ("PATH");
  if (searchPath == NULL)
    return 0;
  if (strlen (searchPath) == 0)
    return 0;
  copy = searchPath;
  stop = 0;
  found = 0;
  do
    {
      end = strchr (copy, ':');
      if (end == NULL)
        {
          stop = 1;
          strncpy (path, copy, strlen (copy));
          len = strlen (path);
        }
      else
        {
          strncpy (path, copy, end - copy);
          path[end - copy] = '\0';
          len = end - copy;
        }
      if (path[len - 1] != '/')
        strncat (path, "/", 2);
      strncat (path, command, strlen (command) - len);
      found = checkifexecutable (path);
      if (!stop)
        copy = end + 1;
    }
  while (!stop && !found);

  return found;
}

int
builtin (char *command)
{
  char *builtin_commands[]
      = { "echo", "cd", "which", "pwd", "export", "unset" };
  for (int i = 0; i < 6; i++)
    {
      if (strncmp (command, builtin_commands[i], strlen (builtin_commands[i]))
          == 0)
        {
          return 1;
        }
    }
  return 0;
}

int
executable (char *command)
{
  char *executables[] = { "cat", "ls", "env", "head", "rm" };
  for (int i = 0; i < 5; i++)
    {
      if (strncmp (command, executables[i], strlen (executables[i])) == 0)
        {
          return 1;
        }
    }
  return 0;
}

int
runCommand (char *command)
{
  int fd[2];
  pipe (fd);

  char *command1 = strtok (command, "|");
  char *command2 = strtok (NULL, "\n");

  char *firstCommand = strtok (command1, " ");
  char *firstArgument = strtok (NULL, "\n");

  char *secondCommand = strtok (command2, " ");
  char *secondArgument = strtok (NULL, "\n");

  int signal = 0;

  if (command2 == NULL && firstArgument == NULL)
    firstCommand[strlen (firstCommand) - 1] = '\0';

  pid_t pid = fork ();

  if (pid < 0)
    return EXIT_FAILURE;

  if (pid > 0)
    close (fd[1]);

  if (pid == 0)
    {
      close (fd[0]);
      if (command2 != NULL)
        {
          dup2 (fd[1], STDOUT_FILENO);
        }
      char *token = strtok (firstArgument, " ");
      char *token2 = strtok (NULL, " ");
      char *token3 = strtok (NULL, " ");
      char *token4 = strtok (NULL, "\\n");

      // char *const args[] = {token, token2, token3, token4, NULL};

      // execvp (firstCommand, args);

      if (firstArgument == NULL)
        execlp (firstCommand, firstCommand, NULL);
      else if (token4 != NULL)
        execlp (firstCommand, firstCommand, token, token2, token3, token4,
                NULL);
      else if (token != NULL && token2 == NULL && token3 == NULL)
        execlp (firstCommand, firstCommand, token, NULL);
      else if (token != NULL && token2 != NULL && token3 != NULL)
        execlp (firstCommand, firstCommand, token, token2, token3, NULL);
      else
        execlp (firstCommand, firstCommand, token, token2, NULL);
    }
  if (command2 != NULL)
    {
      wait (NULL);
      int pid2 = fork ();
      if (pid2 < 0)
        return EXIT_FAILURE;
      if (pid2 == 0)
        {
          dup2 (fd[0], STDIN_FILENO);

          char *token = strtok (secondArgument, " ");
          char *token2 = strtok (NULL, " ");
          char *token3 = strtok (NULL, " ");
          char *token4 = strtok (NULL, "\\n");

          // execvp (secondCommand, secondArgument);

          if (secondArgument == NULL)
            execlp (secondCommand, secondCommand, NULL);
          else if (token4 != NULL)
            execlp (secondCommand, secondCommand, token, token2, token3,
                    token4, NULL);
          else if (token != NULL && token2 == NULL && token3 == NULL)
            execlp (secondCommand, secondCommand, token, NULL);
          else if (token != NULL && token2 != NULL && token3 != NULL)
            execlp (secondCommand, secondCommand, token, token2, token3, NULL);
          else
            execlp (secondCommand, secondCommand, token, token2, NULL);
        }
    }
  close (fd[0]);
  wait (&signal);
  char signalString[32];
  snprintf (signalString, 32, "%d", WEXITSTATUS (signal));
  hash_insert ("?", signalString);
  // printf ("%d\n", WEXITSTATUS (signal));
  // printf ("%s + something here\n", signalString);
  // hash_dump ();
  return EXIT_SUCCESS;
}
