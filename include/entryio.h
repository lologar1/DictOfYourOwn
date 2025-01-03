#ifndef ENTRYIO_H
#define ENTRYIO_H

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include "config.h"
#include "usfhashmap.h"
#include "usfdynarr.h"
#include "usfstring.h"

//ETB character for separating serialized entry fields
#define FSEP '\027'

//US character for separating tags
#define TSEP '\037'

typedef struct entry {
	char *name;
	char *body;
	char **tags;
	uint64_t tagcount;
} entry;

//Serialization
char *entrytos(entry *);
entry *stoentry(char *);

//Importing
entry *texttoentry(char *, char **, uint64_t);

//Entry database
extern usf_hashmap *entrymap;

#endif
