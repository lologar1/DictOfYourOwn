#include <string.h>
#include <ctype.h>

#include "commands.h"
#include "usfstring.h"
#include "entryio.h"
#include "config.h"

/* For debugging purposes */
//#define DEBUG

/* Setting default config (config.h) */

char *SAVEFILE = "doyodata";
uint64_t MAXBUF = 512;
uint64_t MAXFLAGS = 16;
const double VERSION = 2.0;
char *SEPARATOR = "===== %s =====\n";
uint64_t MAXTAGS = 4096;
uint64_t MAXTAG = 8192;
char TAGDECL = '#';
char FLAGDECL = '-'; //Flag decl shouldn't be modified as help is hard-coded with '-', for now.
char SPACESUBST = '\\';

//Entry database
usf_hashmap *entrymap;

void command(int, char *[]); //Handle command

int main() {
	char argbuf[MAXBUF], *cmdbuf[MAXBUF], c, *entrystring;
	int i, n;
	FILE *savefile;
	entry *e;

	printf("Starting DictOfYourOwn V%.1f\n", VERSION);

	//Init entry database
	entrymap = usf_strhmput(NULL, NULL, USFNULL);

	//Load data from SAVEFILE
	entrystring = malloc(n = BUFSIZ); //Reallocated for bigger entries

	i = 0;
	savefile = fopen(SAVEFILE, "r");

	if (savefile == NULL) {
		printf("DoYO: Nonexistent or inaccessible savefile. Welcome to DoYO V%.1f\n", VERSION);
	} else {
		while ((c = fgetc(savefile)) != EOF) {
			if (i >= n) {
				entrystring = realloc(entrystring, n *= 2);
			}

			entrystring[i] = c;

			if (c == '\0') {
				//End of string; commit to memory
				e = stoentry(entrystring);
				if (e != NULL)
					usf_strhmput(entrymap, e -> name, USFDATAP(e));
				i = 0;
			} else i++;
		}
		fclose(savefile);
	}

	free(entrystring);

	//Main prompt loop
	for (;;) {
		printf(">");
		for (n = i = 0; (c = getchar()) != '\n' && c != EOF; i++) {
			//i : address in buffer
			//n : number of arguments

			if ((unsigned) i >= MAXBUF) doyoexit(EXCBUF, NULL); //Exceeded instruction buffer
			if ((unsigned) n >= MAXBUF) doyoexit(EXCBUF, NULL);

			if (isspace(c)) {
				//Avoid empty or whitespace arguments
				if (!i) { i--; continue; }

				/*
				Next argument, flush this one. sizeof(char) is guaranteed to be 1.
				Breakdown : copy current arg to cmdbuf[n], after check.
				Increment n (args) in the process, and set [i] (last) to \0.
				*/

				((char *) memcpy(cmdbuf[n++] = malloc(i+1), argbuf, i))[i] = '\0';
				i = -1; //Restart at 0 next cycle
			}

			//Get character into the buffer
			else argbuf[i] = c;
		}

		//Last argument
		((char *) memcpy(cmdbuf[n++] = malloc(i+1), argbuf, i))[i] = '\0';

#ifdef DEBUG
		// DEBUG : Printing command buffer
		for (i = 0; i < n; i++) {
			printf("[DoYO DEBUG]: CMDBUF[%d] = %s\n", i, cmdbuf[i]);
		}
#endif

		if (c == EOF) doyoexit(ENDFILE, NULL); //Exit

		command(n, cmdbuf); //Execute command

		//Free command buffer
		for (i = 0; i < n; i++) {
			free(cmdbuf[i]);
		}

		if (searchstream) {
			//Last command yielded a search result
			for (i = 0; (unsigned) i < entrymap -> size; i++) {
				if (searchstream[i] == NULL)
					continue;

				printf("%s ", searchstream[i]);
			}

			printf("\n");
			free(searchstream);
			searchstream = NULL;
		}
	}

	fprintf(stderr, "DoYO V%.1f: Abnormal program exit !\n", VERSION);
	return 0;
}

void command(int args, char *argv[]) {
	/*
	Actual command at argv[0], rest is arguments.
	Accepted commands :

	help: display help menu
	exit: exit program (as if with CTRL+D)
	save: save state to disk
	import [fname]: import file to program
	importall: import all .txt files in current directory
	remove [fname]: remove file from program
	export [fname] -t: export dictionary as text file (tags optional)
	view [fname]: display definition and associated tags

	search [match] -s -e -c: search entries for match (optional startswith, endswith, or contains)
	list [tag]: list all entries with tag (can be combined)
	desc [match] -s -e -c: search entry contents for match
	sort -a -l -r: sort entries (alphanumeric, length, reversed)

	The searching and listing commands can be combined :

	Search entries beginning with "sif", with tag "abstract", ordered by length :
	search sif -s list abstract sort -l

	Search entries beginning with "sif", with contents including "arrow", by length :
	search sif -s desc -c arrow sort -l
	*/

	const char **cmdindex =
		bsearch(&argv[0], cmdnames, sizeof(cmdnames) / sizeof(char *), sizeof(const char *), usf_indstrcmp);

	if (cmdindex == NULL) {
		printf("Unknown command %s. Type \"help\" for help.\n", argv[0]);
		return;
	}

	(*commands[cmdindex - cmdnames])(args, argv); //Pass arguments only
}
