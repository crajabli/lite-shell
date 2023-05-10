#include "builtins.h"
#include "hash.h"
#include "process.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// No command line can be more than 100 characters
#define MAXLENGTH 100
#define RETURNLENGTH 3
#define WHITESPACE " \t\n\r"

void
shell (FILE *input)
{
  hash_init (100);
  char buffer[MAXLENGTH];
  char bufcopy[MAXLENGTH];
  hash_insert ("?", "0");
  while (1)
    {
      // Print the cursor and get the next command entered
      printf ("$ ");

      memset (buffer, 0, sizeof (buffer));
      char *got = fgets (buffer, MAXLENGTH, input);

      strncpy (bufcopy, buffer, MAXLENGTH);

      if (input != stdin)
        printf ("%s", bufcopy);
      fflush (stdout);
      char *next;
      char *command = strtok_r (buffer, WHITESPACE, &next);
      if (got == NULL || command == NULL)
        break;

      if (strncmp (command, "quit", 4) == 0)
        break;
      else if (strncmp (command, "echo", 4) == 0)
        echo (next);
      else if (strncmp (command, "cd", 2) == 0)
        cd (next);
      else if (strncmp (command, "pwd", 3) == 0)
        pwd ();
      else if (strncmp (command, "which", 5) == 0)
        which (next);
      else if (strncmp (command, "export", 6) == 0)
        export(next);
      else if (strncmp (command, "./bin/", 6) == 0)
        runCommand (bufcopy);
      else if (strncmp (command, "quit", 4) == 0)
        break;
      fflush (stdin);
    }
  printf ("\n");
  hash_destroy ();
}
