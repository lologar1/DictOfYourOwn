#ifndef COMMANDS_H
#define COMMANDS_H

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include "config.h"
#include "entryio.h"
#include "entrystream.h"
#include "usfio.h"

void chaincommand(int, char *[]);

int desc(int, char *[]);
int doyoexit(int, char *[]);
int export(int, char *[]);
int help(int, char *[]);
int import(int, char *[]);
int importall(int, char *[]);
int list(int, char *[]);
int doyoremove(int, char *[]);
int doyorename(int, char *[]);
int save(int, char *[]);
int search(int, char *[]);
int sort(int, char *[]);
int view(int, char *[]);

extern const char *cmdnames[13]; //Number of defined commands
extern int (*commands[])(int, char *[]);

typedef enum doyo_exitcode {
	EXCBUF = INT_MIN, //Exceeded buffer max size
	ENDFILE, //End of file detected on command stream
	EXIT, //Exit from exit command
	ERROR //Error
} doyo_exitcode;
#endif
