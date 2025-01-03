#include "commands.h"

//Defined commands (IMPORTANT: must add in alphabetical order !)

const char *cmdnames[] =
	{"desc", "exit", "export", "help", "import", "importall",
	"list", "remove", "rename", "save", "search", "sort", "view"};

int (*commands[])(int, char *[]) =
	{&desc, &doyoexit, &export, &help, &import, &importall,
	&list, &doyoremove, &doyorename, &save, &search, &sort, &view};

/* Search stream workspace */
char **searchstream = NULL;

void chaincommand(int args, char *argv[]) {
	const char **cmdindex;
	char *cmd, **stream;

	cmd = argv[0];
	if (cmd[0] == '|' || cmd[0] == '&') {
		//Set operator; save stream
		stream = searchstream;
		searchstream = entrystream();

		if (args > 1) { //Must have command after operator
			//Skip operator, and call
			cmdindex =
				bsearch(&argv[1], cmdnames, sizeof(cmdnames) / sizeof(char *),
				sizeof(const char *), usf_indstrcmp);

			if (cmdindex == NULL) {
				//Undo cache
				free(searchstream);
				searchstream = stream;
				printf("Unknown chained command %s. Type \"help\" for help.\n", argv[1]);
				return;
			} else {
				//Skip operator
				(*commands[cmdindex - cmdnames])(args - 1, argv + 1);
			}
		}

		//Union and intersection automatically free components
		searchstream = (cmd[0] == '|'
			? streamunion(searchstream, stream)
			: streamintersection(searchstream, stream));
	} else {
		cmdindex =
			bsearch(&cmd, cmdnames, sizeof(cmdnames) / sizeof(char *),
			sizeof(const char *), usf_indstrcmp);

		if (cmdindex == NULL) {
			printf("Unknown chained command %s. Type \"help\" for help.\n", cmd);
			return;
		} else
			(*commands[cmdindex - cmdnames])(args, argv);
	}
}

int desc(int args, char *argv[]) {
	char *flags, *arg, *description;
	int s, e, c, r, offset, len;
	uint64_t i;

	if (args < 2 || strcmp(argv[0], "desc")) {
		printf("Syntax: desc -secr [STRING]\n");
		return 0;
	}

	//Get flags
	flags = getflags(args, argv, &offset);
	s = strchr(flags, 's') ? 1 : 0;
	e = strchr(flags, 'e') ? 1 : 0;
	c = strchr(flags, 'c') ? 1 : 0;
	r = strchr(flags, 'r') ? 1 : 0;
	free(flags);

	if (args <= offset) { //Forgot argument
		printf("Syntax: desc -secr [STRING]\n");
		return 0;
	}

	arg = argv[offset]; //Argument
	usf_sreplace(arg, SPACESUBST, ' ');

	//Initialize if null (freed and printed in main)
	if (searchstream == NULL)
		searchstream = entrystream();

	for (i = 0; i < entrymap -> size; i++) {
		if (searchstream[i] == NULL)
			continue; //Filtered out

		//Get body
		description = ((entry *) usf_strhmget(entrymap, searchstream[i]).p) -> body;

		if (s && r == (usf_sstartswith(description, arg) != NULL))
			searchstream[i] = NULL; //Filter out

		//Remove trailing \n from description to properly test
		len = strlen(description);
		description[len - 1] = '\0';

		if (e && r == usf_sendswith(description, arg))
			searchstream[i] = NULL; //Endswith

		//Add it back
		description[len - 1] = '\n';

		if (c && r == (strstr(description, arg) != NULL))
			searchstream[i] = NULL; //Contains
	}

	offset++; //Point to next command

	if (args <= offset)
		return 0; //End of commands

	//Handle next
	chaincommand(args - offset, argv + offset);
	return 0;
}

int doyoexit(int exitcode, char *argv[]) {
	(void) argv;
	/*
	Args is viewed as exit code;
	No arguments may be given for a program exit.
	*/

	switch(exitcode) {
		case EXCBUF:
			fprintf(stderr, "DoYO: Exceeded command buffer max size\n");
			break;
		case ENDFILE:
			fprintf(stdout, "DoYO: Exiting after EOF\n");
			break;
		case ERROR:
			fprintf(stderr, "DoYO: Exiting after catastrophic error\n");
			break;
		default:
			fprintf(stdout, "DoYO: Quitting\n");
	}

	printf("Saving data to disk...\n");
	save(0, NULL); //Save state

	printf("Bye !\n");
	exit(exitcode);
}

int export(int args, char *argv[]) {
	FILE *exportfile;
	uint64_t i, j;
	char **entries;
	char *name;
	entry *e;

	if (args < 2) {
		printf("Syntax: export [FILENAME]\n");
		return 0;
	}

	name = argv[1];
	exportfile = fopen(name, "w");

	if (exportfile == NULL) {
		printf("DoYO: Couldn't export to file %s\n", name);
		return 0;
	}

	entries = entrystream();
	qsort(entries, entrymap -> size, sizeof(char *), usf_indstrcmp);

	for (i = 0; i < entrymap -> size; i++) {
		e = (entry *) usf_strhmget(entrymap, entries[i]).p;

		if (e == NULL) {
			printf("DoYO (Critical Error): Nonexistent entry %s but present in entrystream\n", entries[i]);
			free(entries);
			fclose(exportfile);
			return ERROR;
		}

		fprintf(exportfile, SEPARATOR, e -> name);

		if (e -> tagcount)
			fputs("Tags: ", exportfile);
		else
			fputs("No tags.", exportfile);

		for (j = 0; j < e -> tagcount; j++) {
			fputs(e -> tags[j], exportfile);

			if (j < e -> tagcount - 1) //List except second-to-last
				fputs(", ", exportfile);
		}

		fputs("\n\n", exportfile); //Skip line

		fputs(e -> body, exportfile); //Main content

		if (i < entrymap -> size - 1)
			fputc('\n', exportfile); //Skip line for next entry
	}

	fclose(exportfile);
	free(entries);
	return 0;
}

int help(int args, char *argv[]) {
	(void) args;
	(void) argv;

	//Arguments ignored; terminal command
	printf("=== DoYO V%.1f Help Menu ===\n", VERSION);
	printf("\n");
	printf("Terminal (atomic) commands:\n");
	printf("help: display this menu\n");
	printf("exit: quit program\n");
	printf("export [FILENAME]: export this dictionary as [FILENAME].txt\n");
	printf("import [FILENAME] ?[PATH]: import [FILENAME].txt as an entry. For extern files, provide a path.\n");
	printf("importall ?[DIRECTORY]: import all txt files in [DIRECTORY], or this one\n");
	printf("remove [FILENAME]: remove the entry [FILENAME]\n");
	printf("rename [FILENAME] [NEWNAME]: rename the entry [FILENAME] to [NEWNAME]\n");
	printf("save: save to disk, although this is done automatically on exit\n");
	printf("view [FILENAME]: display entry definition and associated tags\n");
	printf("\n");
	printf("Chainable commands (first one generates stream from all entries):\n");
	printf("desc -secr [STRING]: return all entries with description matching string [STRING] and flags\n");
	printf("list -r [TAG]: return all entries with tag [TAG]\n");
	printf("search -secr [STRING]: return all entries with name matching string [STRING] and flags\n");
	printf("sort -alr: sort stream [a] alphanumerically, [l] by length, [r] in reverse\n");
	printf("\n");
	printf("Searching flags: [s] startswith, [e] endswith, [c] contains, [r] inverse (non-matching)\n");
	printf("Flags must always precede the argument. Spaces must be substituted by the \\ (backslash) character.\n");
	printf("\n");
	printf("Set operators & (intersection) and | (union) may be used between streams\n");
	printf("All operations are done from left to right. A set operator begins a new stream for\n");
	printf("its right-hand value. For example, looking up all entries with tag 'abstract' or tag\n");
	printf("'concrete' that do not contain 'the' in their name, sorted by length, could be done like this:\n");
	printf(">sort -l list concrete | sort -l list abstract & sort -l search -cr the\n");
	printf("\n");
	return 0;
}

int import(int args, char *argv[]) {
	char **text, *fullname, *name;
	uint64_t len, i;
	entry *dictentry, *old;

	//Syntax : import [FILENAME]; Imports FILENAME.txt
	if (args < 2) {
		printf("Syntax: import [FILENAME]\n");
		return 0;
	}

	name = argv[1];

	if (args > 2) {
		//Take absolute path from argv[2]
		text = usf_ftot(argv[2], "r", &len);
	} else if (usf_sendswith(name, ".txt")) {
		text = usf_ftot(name, "r", &len);
	} else {
		fullname = malloc(strlen(name) + 5); //Include \0
		strcpy(fullname, name);
		strcat(fullname, ".txt");
		text = usf_ftot(fullname, "r", &len);
		free(fullname);
	}

	//Remove extension
	if (usf_sendswith(name, ".txt")) name[strlen(name) - 4] = '\0';

	if (text == NULL) {
		printf("DoYO: Couldn't open file %s.txt\n", name);
		if (args > 2)
			printf("Important: directory paths must close with a separator to yield a valid path\n");
		return 0;
	}

	dictentry = texttoentry(name, text, len);

	//Free text buffer
	for (i = 0; i < len; i++)
		free(text[i]);
	free(text);

	if (dictentry == NULL) {
		printf("DoYO: Couldn't parse file %s.txt\n", name);
		return 0;
	}

	//Delete if present
	old = (entry *) usf_strhmget(entrymap, name).p;
	if (old) {
		free(old -> name);
		free(old -> body);

		for (i = 0; i < old -> tagcount; i++)
			free(old -> tags[i]);
		free(old -> tags);

		free(old);
	}

	//Add to entries
	usf_strhmput(entrymap, name, USFDATAP(dictentry));
	printf("Imported %s\n", name);
	return 0;
}

int importall(int args, char *argv[]) {
	char *path = args < 2 ? "./" : argv[1], *abspath;
	DIR *dir = opendir(args < 2 ? "." : argv[1]);
	struct dirent *direntry;
	char *importargs[3] = {"importall"}; //Arg 0 unimportant but might be useful for debugging

	if (dir == NULL) {
		if (args < 2)
			printf("DoYO (Catastrophic Error): Couldn't open current directory\n");
		else
			printf("DoYO: Couldn't open directory %s\n", argv[1]);
		return ERROR;
	}


	while ((direntry = readdir(dir)) != NULL) {
		//Skip special and hidden files
		if (direntry -> d_name[0] == '.')
			continue;

		if (usf_sendswith(direntry -> d_name, ".txt")) {
			//Generate absolute path
			abspath = malloc(strlen(path) + strlen(direntry -> d_name) + 1);
			strcpy(abspath, path);
			strcat(abspath, direntry -> d_name);

			//Import from absolute path
			importargs[1] = direntry -> d_name;
			importargs[2] = abspath;
			import(3, importargs);
			free(abspath);
		}
	}

	closedir(dir);

	return 0;
}

int list(int args, char *argv[]) {
	char *flags, *arg, **tags;
	int r, offset, match;
	entry *e;
	uint64_t i, j;

	if (args < 2 || strcmp(argv[0], "list")) {
		printf("Syntax: list -r [TAG]\n");
		return 0;
	}

	flags = getflags(args, argv, &offset);
	r = strchr(flags, 'r') ? 1 : 0;
	free(flags);

	if (args <= offset) {
		printf("Syntax: list -r [TAG]\n");
		return 0;
	}

	arg = argv[offset]; //Tag

	if (searchstream == NULL)
		searchstream = entrystream();

	for (i = 0; i < entrymap -> size; i++) {
		if (searchstream[i] == NULL)
			continue;

		match = 0;
		e = (entry *) usf_strhmget(entrymap, searchstream[i]).p;
		tags = e -> tags;

		for (j = 0; j < e -> tagcount; j++) {
			if (!strcmp(tags[j], arg)) {
				match = 1;
				break;
			}
		}

		//Destroy if matching prediction
		if (match == r)
			searchstream[i] = NULL;
	}

	offset++; //Next cmd

	if (args <= offset)
		return 0;

	chaincommand(args - offset, argv + offset);
	return 0;
}

int doyoremove(int args, char *argv[]) {
	entry *e;
	char *name;
	uint64_t i;

	if (args < 2) {
		printf("Syntax: remove [FILENAME]\n");
		return 0;
	}

	name = argv[1];
	e = (entry *) usf_strhmdel(entrymap, name).p;

	if (e == NULL) {
		printf("DoYO: Couldn't find or delete entry %s\n", name);
		return 0;
	}

	free(e -> name);
	free(e -> body);

	for (i = 0; i < e -> tagcount; i++)
		free(e -> tags[i]);
	free(e -> tags);

	free(e);
	return 0;
}

int doyorename(int args, char *argv[]) {
	char *name, *newname;
	entry *e;

	if (args < 3) {
		printf("Syntax: rename [FILENAME] [NEWNAME]\n");
		return 0;
	}

	name = argv[1];
	newname = argv[2];

	e = (entry *) usf_strhmdel(entrymap, name).p;

	if (e == NULL) {
		printf("DoYO: Couldn't find entry %s\n", name);
		return 0;
	}

	free(e -> name); //Free old name
	e -> name = malloc(strlen(newname) + 1); //Include \0
	strcpy(e -> name, newname);

	usf_strhmput(entrymap, newname, USFDATAP(e));
	return 0;
}

int save(int args, char *argv[]){
	(void) args;
	(void) argv;

	FILE *savefile = fopen(SAVEFILE, "w");
	uint64_t i;
	char *entrystring, **entries;

	if (savefile == NULL) {
		printf("DoYO (Critical Error): Cannot save to file %s\n", SAVEFILE);
		return ERROR;
	}

	/* Iterate through hashmap array. For every key:
	Serialize key and write string (\0 terminated) to file.
	File is in config.h. At program start, automatically load
	entries from the file. In the future, a doyoconfig file
	will overwrite the default config.h values */

	entries = entrystream();
	for (i = 0; i < entrymap -> size; i++) {
		if (ferror(savefile)) {
			printf("DoYO (Catastrophic Error): Error while saving to file %s\n", SAVEFILE);
			return ERROR;
		}

		entrystring = entrytos((entry *) usf_strhmget(entrymap, entries[i]).p); //Serialize
		fputs(entrystring, savefile);
		fputc('\0', savefile); //Terminating null
		free(entrystring); //Use a static buffer ? Suboptimal.
	}

	free(entries);
	fclose(savefile);
	return 0;
}

int search(int args, char *argv[]){
	char *flags, *arg, *title;
	int s, e, c, r, offset;
	uint64_t i;

	if (args < 2 || strcmp(argv[0], "search")) {
		printf("Syntax: search -secr [STRING]\n");
		return 0;
	}

	flags = getflags(args, argv, &offset);
	s = strchr(flags, 's') ? 1 : 0;
	e = strchr(flags, 'e') ? 1 : 0;
	c = strchr(flags, 'c') ? 1 : 0;
	r = strchr(flags, 'r') ? 1 : 0;
	free(flags);

	if (args <= offset) {
		printf("Syntax: desc -secr [STRING]\n");
		return 0;
	}

	arg = argv[offset];
	usf_sreplace(arg, SPACESUBST, ' ');

	if (searchstream == NULL)
		searchstream = entrystream();

	for (i = 0; i < entrymap -> size; i++) {
		if (searchstream[i] == NULL)
			continue;

		title = searchstream[i];

		if (s && r == (usf_sstartswith(title, arg) != NULL))
			searchstream[i] = NULL;

		if (e && r == usf_sendswith(title, arg))
			searchstream[i] = NULL;

		if (c && r == (strstr(title, arg) != NULL))
			searchstream[i] = NULL;
	}

	offset++;

	if (args <= offset)
		return 0;

	chaincommand(args - offset, argv + offset);
	return 0;
}

int sort(int args, char *argv[]){
	char *flags;
	int l, r, offset; //a flag omitted as it's set by default
	int (*cmp)(const void *, const void *);

	if (args < 2 || strcmp(argv[0], "sort")) {
		printf("Syntax: sort -alr\n");
		return 0;
	}

	flags = getflags(args, argv, &offset);
	l = strchr(flags, 'l') ? 1 : 0;
	r = strchr(flags, 'r') ? 1 : 0;
	free(flags);

	if (searchstream == NULL)
		searchstream = entrystream();

	//By default
	cmp = usf_indstrcmp;

	if (l)
		cmp = usf_indstrlen;

	qsort(searchstream, entrymap -> size, sizeof(char *), cmp);

	if (r)
		usf_reversetxt(searchstream, entrymap -> size);

	if (args <= offset)
		return 0;

	chaincommand(args - offset, argv + offset);
	return 0;
}

int view(int args, char *argv[]) {
	entry *e;
	char *name;
	uint64_t i;

	if (!args) {
		printf("Syntax: view [FILENAME]\n");
		return 0;
	}

	name = argv[1];
	e = (entry *) usf_strhmget(entrymap, name).p;

	if (e == NULL) {
		printf("DoYO: Couldn't find entry %s\n", name);
		return 0;
	}

	//Name
	printf("%s\n", e -> name);

	//Tags
	printf("Tags: ");
	for (i = 0; i < e -> tagcount; i++) {
		printf("%s ", e -> tags[i]);
	}

	printf("\n\n"); //Separate header from body

	//Body
	printf("%s", e -> body);

	return 0;
}
