#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/*
This is the default config state.
Variables here (non-const ones) can be modified
by using the config FIELDNAME VALUE command.

(TODO at some point)
*/

//DoYO save file
extern char *SAVEFILE;

//Maximum command and argument buffer lengths
extern uint64_t MAXBUF;

//Maximum flags for a given command
extern uint64_t MAXFLAGS;

//Current version of the program
extern const double VERSION;

//Decoration for each entry on export. "\05" is the entry name placeholder.
extern char *SEPARATOR;

//Maximum number of tags per file
extern uint64_t MAXTAGS;

//Maximum number of chars per tag
extern uint64_t MAXTAG;

//Special character for tag declarations
extern char TAGDECL;

//Special character for flag declarations
extern char FLAGDECL;

//Special character to escape spaces
extern char SPACESUBST;
#endif
