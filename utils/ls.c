#include "../src/hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


static void usage (void);
bool
get_args (int argc, char **, bool *, bool *, bool *, char **);

int
main (int argc, char *argv[])
{
  bool listAllFiles = false;
  bool listFileSizes = false;
  bool falseFlag = false;
  bool homeDir = false;
  char *dir;
  DIR *directory;
  struct dirent *dirEntry;
  struct stat fileStat;

  if (! get_args (argc, argv, &listAllFiles, &listFileSizes, &falseFlag, &dir))
    {
      if (!falseFlag)
        {
          usage ();
        }
      else
      {
        // printf ("$ 1\n");
        // hash_insert ("?", "1");
        return EXIT_FAILURE;
      }
    }
  if (dir == NULL)
  {
    directory = opendir (".");
    homeDir = true;
  }
  else
  {
    directory = opendir (dir);
    if (directory == NULL)
    {
        // hash_insert ("?", "1");
        // printf ("$ 1\n");
        return EXIT_FAILURE;
    }
  }
  dirEntry = readdir (directory);
  while (dirEntry != NULL)
  {
    char buffer [1024];
    memset (buffer, 0, sizeof (buffer));
    char *fileName = dirEntry->d_name;
    getcwd (buffer, sizeof (buffer));
    char *path = strcat (buffer, "/");
    if (!homeDir)
    {
      path = strcat (path, dir);
      path = strcat (path, "/");
    }
    path = strcat (path, fileName);
    lstat (path, &fileStat);
    if (listAllFiles && listFileSizes)
      printf ("%ld %s\n", fileStat.st_size, dirEntry->d_name);
    else if (listAllFiles && !listFileSizes)
      printf ("%s\n", dirEntry->d_name);
    else if (!listAllFiles && listFileSizes)
    {
      if ((dirEntry->d_name)[0] != '.')
        printf ("%ld %s\n", fileStat.st_size, dirEntry->d_name);
    }
    else if (!listAllFiles && !listFileSizes)
    {
      if ((dirEntry->d_name)[0] != '.')
        printf ("%s\n", dirEntry->d_name);
    }
    else
      printf("something is very very broken");
    dirEntry = NULL;
    dirEntry = readdir (directory);
  }

  // printf ("$ 0\n");
  return EXIT_SUCCESS;
}

bool
get_args (int argc, char **argv, bool *listAllFiles, bool *listFileSizes, bool *falseFlag, char **dir)
{
  int ch = 0;
  while ((ch = getopt (argc, argv, "as")) != -1)
    {
      switch (ch)
        {
        case 'a':
          *listAllFiles = true;
          break;
        case 's':
          *listFileSizes = true;
          break;
        default:
          *falseFlag = true;
          return false;
        }
    }
  if (optind > argc)
    return false;
  *dir = argv[optind];
  return true;
}

static void
usage (void)
{
  printf ("ls, list directory contents\n");
  printf ("usage: ls [FLAG ...] [DIR]\n");
  printf ("FLAG is one or more of:\n");
  printf ("  -a       list all files (even hidden ones)\n");
  printf ("  -s       list file sizes\n");
  printf ("If no DIR specified, list current directory contents\n");
}
