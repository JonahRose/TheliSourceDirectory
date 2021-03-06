#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "stuff.h"

// compiled with
// gcc getkey.c -L. -leclipse -lqfits -lm -O3 -o getkey


void usage(int i, char *argv[])
{
  if (i == 0) {
    fprintf(stderr,"\n");
    fprintf(stderr,"          Version 1.0  (2010-08-31)\n\n");
    fprintf(stderr,"  Author: Mischa Schirmer\n\n");
    fprintf(stderr,"  USAGE:  cleanstring string\n");
    fprintf(stderr,"          Leading and trailing whitespace is removed, whitespace inside\n");
    fprintf(stderr,"          replaced by _, and meta-characters are deleted without replacement.\n\n");
    exit(1);
  }
}


int main(int argc, char *argv[])
{
  char string[FILEMAX], *keyvalue, newstring[FILEMAX], *final="";
  char *s, *slast;
  char remove[] = " ()[]/;\'\"*#^$%üäö\t\n";

  if (argc==1) usage(0, argv);

  strcpy(string,argv[1]);
  keyvalue = string;

  // Replace single quotes with blanks
  int lstr = strlen (keyvalue);
  slast = keyvalue + lstr;
  for (s = keyvalue; s < slast; s++) {
    if (*s == 39) *s = ' ';
  }

  // trim whitespace
  strcpy(newstring,trimwhitespace(keyvalue));
  
  // Replace embedded blanks with underscores
  lstr = strlen (newstring);
  slast = newstring + lstr;
  for (s = newstring; s < slast; s++) {
    if (*s == ' ') *s = '_';
  }

  // remove metacharacters from keyword
  final = newstring;
  final = RemoveChars(newstring, remove);
  printf("%s\n", final);
  
  exit (0);
}
