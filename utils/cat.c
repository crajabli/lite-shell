#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

static void usage (void);
static bool get_args (int , char **, FILE **);

int
main (int argc, char *argv[])
{
  FILE *file = NULL;
  size_t maxLineSize = 1024;
  char *buffer;

  if (! get_args (argc, argv, &file))
  {
    usage ();
    return EXIT_FAILURE;
  }
  if (file == NULL)
  {
    return EXIT_FAILURE;
  }

  while (getline(&buffer, &maxLineSize, file) != -1)
  {
    printf ("%s", buffer);
  }

  return EXIT_SUCCESS;
}


static bool
get_args (int argc, char **argv, FILE **file)
{
  if (argc < 2)
    return false;
  *file = fopen (argv[optind], "r");
  return true;
}

static void
usage (void)
{
  printf ("cat, print the contents of a file\n");
  printf ("usage: cat FILE\n");
}
