#ifndef ENTRYSTREAM_H
#define ENTRYSTREAM_H

#include "usfhashmap.h"
#include "entryio.h"
#include "config.h"

extern char **searchstream;

char **entrystream();
char *getflags(int args, char *[], int *);
char **streamunion(char **, char **);
char **streamintersection(char **, char **);

#endif
