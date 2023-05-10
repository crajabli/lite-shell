// #include "hash.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage (void);

int
main (int argc, char *argv[], char *envp[])
{
  if (argc == 0)
    {
      usage ();
      return EXIT_FAILURE;
    }

  char *prog = argv[argc - 1];
  bool is_prog = prog[0] == '.';

  for (int i = 0; i < argc; i++)
    {
      char *key = strtok (argv[i], "=");
      char *value = strtok (NULL, "=");
      // hash_insert (key, value);

      if (is_prog)
        {
          if (key != NULL && value != NULL)
            {
              printf ("%s=%s\n", key, value);
            }
        }
    }

  // hash_destroy ();
  return EXIT_SUCCESS;
}

static void
usage (void)
{
  printf ("env, set environment variables and execute program\n");
  printf ("usage: env [name=value ...] PROG ARGS\n");
}
