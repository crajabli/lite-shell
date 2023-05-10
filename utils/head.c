#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <getopt.h>

// You may assume that lines are no longer than 1024 bytes
#define LINELEN 1024

static void usage (void);
static bool get_args (int , char **, FILE **, int *, bool *);

int
main (int argc, char *argv[])
{
  bool falseFlag = false;
  FILE *file = NULL;
  int defaultNum = 5;
  size_t maxLine = LINELEN;

  if (! get_args (argc, argv, &file, &defaultNum, &falseFlag))
    {
      if (!falseFlag)
        {
          usage ();
        }
      else
      {
        return EXIT_FAILURE;
      }
    }
    if (file == NULL)
    {
      char temp[100];
      int i = 0;
      while (fgets (temp, maxLine, stdin) != NULL)
      {
        printf ("%s", temp);
        i++;
        if (i == defaultNum || feof (stdin))
          break;
      }
      return EXIT_SUCCESS;
    }
    char temp[100];
    int i = 0;
    while (fgets (temp, maxLine, file) != NULL)
    {
      printf ("%s", temp);
      i++;
      if (i == defaultNum || feof (file))
        break;
    }

  return EXIT_SUCCESS;
}

static bool
get_args (int argc, char **argv, FILE **file, int *num, bool *falseFlag)
{
  int ch = 0;
  while ((ch = getopt (argc, argv, "n:")) != -1)
    {
      switch (ch)
        {
        case 'n':
          *num = atoi (argv[optind - 1]);
          break;
        default:
          *falseFlag = true;
          return false;
        }
    }
  if (argc < 2)
    return false;
  *file = fopen (argv[argc - 1], "r");
  return true;
}

static void
usage (void)
{
  printf ("head, prints the first few lines of a file\n");
  printf ("usage: head [FLAG] FILE\n");
  printf ("FLAG can be:\n");
  printf ("  -n N     show the first N lines (default 5)\n");
  printf ("If no FILE specified, read from STDIN\n");
}
