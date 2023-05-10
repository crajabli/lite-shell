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

// Given a message as input, print it to the screen followed by a
// newline ('\n'). If the message contains the two-byte escape sequence
// "\\n", print a newline '\n' instead. No other escape sequence is
// allowed. If the sequence contains a '$', it must be an environment
// variable or the return code variable ("$?"). Environment variable
// names must be wrapped in curly braces (e.g., ${PATH}).
//
// Returns 0 for success, 1 for errors (invalid escape sequence or no
// curly braces around environment variables).

#define MAXLENGTH 100
#define WHITESPACE " \t\n\r"

int tokenize_arguments (char *, char ***);

int
cd (char *path)
{
  if (path == NULL)
    return 1;

  path[strlen (path) - 1] = '\0';
  if (chdir (path) != 0)
    perror ("cd fail");
  return 0;
}

char *
parseEnvironmentVars (char *oldStr)
{
  // A string buffer for the newly formatted string
  char *newStr = calloc (MAXLENGTH, sizeof (char));
  // Copy of the passed string
  char *copy = strndup (oldStr, strlen (oldStr) + 1);
  // Pointer to position in traversal
  char *pos = copy;
  // Pointer to '$' in the string
  char *dollar;

  while ((dollar = strchr (pos, '$')) != NULL)
    {
      // Define enval
      char *envVal;

      // Add first half of string
      strncat (newStr, pos, dollar - pos);

      // Decide whether to parse as a regular variable or return code
      char *endBracket = strchr (dollar, '}');
      if (*(dollar + 1) == '?')
        {
          envVal = hash_find ("?");
          endBracket = dollar + 1;
        }
      else if (*(dollar + 1) == '{' || endBracket != NULL)
        {
          // Grab the env variable name from the string and find it's value
          char *envName = strndup (dollar + 2, endBracket - dollar - 2);
          envVal = hash_find (envName);
          free (envName);
        }
      else
        {
          free (copy);
          free (newStr);
          return NULL;
        }

      if (envVal == NULL)
        envVal = "";

      // Build the newly formatted string
      strncat (newStr, envVal, MAXLENGTH - strlen (newStr));

      // Increment position after dollar sign
      pos = endBracket + 1;
    }
  // Build the entire string if there is no variable, or the remaining part
  strncat (newStr, pos, MAXLENGTH - strlen (newStr));
  free (copy);
  return newStr;
}

char *
parseNewLines (char *oldStr)
{
  // A string buffer for the newly formatted string
  char *newStr = calloc (strlen (oldStr) + 1, sizeof (char));
  // Pointer to position in traversal
  char *pos = oldStr;
  // Pointer to "\n" in the string
  char *newlnChar;

  while ((newlnChar = strstr (pos, "\\n")) != NULL)
    {
      // Add first half of string
      strncat (newStr, pos, newlnChar - pos);
      strncat (newStr, "\n", 2);

      // Increment position to after escape sequence
      pos = newlnChar + 2;
    }
  strncat (newStr, pos, MAXLENGTH - strlen (newStr));

  return newStr;
}

int
echo (char *message)
{
  char **arg_list = NULL;
  int arg_count = tokenize_arguments (message, &arg_list);

  // 0 is NULL, or used to store the name of the calling command.
  for (int i = 1; i < arg_count; i++)
    {
      // check if the message contains the two-byte escape sequence "\\n",
      // print a newline '\n' instead.

      char *parsedVars = parseEnvironmentVars (arg_list[i]);
      char *parsedNewLines = parseNewLines (parsedVars);
      printf ("%s", parsedNewLines);
      if (i != arg_count - 1)
        printf (" ");
      free (parsedVars);
      free (parsedNewLines);
    }
  if (arg_list != NULL)
    free (arg_list);
  printf ("\n");

  return 0;
}

// Given a key-value pair string (e.g., "alpha=beta"), insert the mapping
// into the global hash table (hash_insert ("alpha", "beta")).
//
// Returns 0 on success, 1 for an invalid pair string (kvpair is NULL or
// there is no '=' in the string).
int export(char *kvpair)
{
  if (kvpair == NULL || strchr (kvpair, '=') == NULL)
    {
      return 1;
    }
  char *tokens = strtok (kvpair, "=");
  char *key = tokens;
  tokens = strtok (NULL, "=");
  char *value = tokens;
  value[strlen (value) - 1] = '\0';
  hash_insert (key, value);
  return 0;
}

// Prints the current working directory (see getcwd()). Returns 0.
int
pwd (void)
{
  char currentDir[1024];
  if (getcwd (currentDir, sizeof (currentDir)) != NULL)
    printf ("%s\n", currentDir);
  return 0;
}

// Removes a key-value pair from the global hash table.
// Returns 0 on success, 1 if the key does not exist.
int
unset (char *key)
{
  if (hash_find (key) == NULL)
    {
      return 1;
    }
  hash_remove (key);
  return 0;
}

// Given a string of commands, find their location(s) in the $PATH global
// variable. If the string begins with "-a", print all locations, not just
// the first one.
//
// Returns 0 if at least one location is found, 1 if no commands were
// passed or no locations found.
int
which (char *cmdline)
{

  if (cmdline == NULL)
    return 1;
  else
    for (int i = 0; i < strlen (cmdline); i++)
      {
        if (cmdline[i] == ' ' || cmdline[i] == '\n')
          {
            cmdline[i] = '\0';
            break;
          }
      }

  if (builtin (cmdline))
    {
      printf ("%s: dukesh built-in command\n", cmdline);
      return 0;
    }
  else if (executable (cmdline))
    {
      // do path lookup of the given command
      char path[1024];
      pathLookup (path, cmdline);
      printf ("%s\n", path);
      return 0;
    }
  else if (strncmp (cmdline, "./", 2) == 0)
    {
      if (executable (&cmdline[3]))
        printf ("%s: is executable\n", cmdline);
      printf ("%s\n", cmdline);
      return 0;
    }
  // perform a path lookup on the given command
  char path[100];
  getcwd (path, sizeof (path));
  printf ("%s/%s\n", path, cmdline);
  return 0;
}

int
tokenize_arguments (char *buffer, char ***arg_ret)
{
  assert (buffer != NULL);
  char *token = NULL;

  /* Allocate an initial array for 10 arguments; this can grow
     later if needed */
  size_t arg_list_capacity = 10;
  char **arguments = calloc (arg_list_capacity, sizeof (char *));
  assert (arguments != NULL);

  /* Leave the first space blank for the command name */
  size_t arg_list_length = 1;

  while ((token = strtok_r (NULL, WHITESPACE, &buffer)) != NULL)
    {
      /* If current argument array is full, double its capacity */
      if ((arg_list_length + 1) == arg_list_capacity)
        {
          arg_list_capacity *= 2;
          arguments = realloc (arguments, arg_list_capacity * sizeof (char *));
          assert (arguments != NULL);
        }

      /* Add the token to the end of the argument list */
      arguments[arg_list_length++] = token;
    }
  *arg_ret = arguments;
  return arg_list_length;
}
