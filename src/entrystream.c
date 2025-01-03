#include "entrystream.h"

char **entrystream() {
	/* Return a list of all entries from entrymap
	All chainable functions take an entrystream and
	return a modified one.

	Sidenote for anybody reading this who isn't me:
	some parts of the code might seem a little obscure
	due to the underlying implementation of library functions
	and data structures. This hashmap (which is open-addressed)
	'kills' nodes when they are deleted to avoid losing track
	of dependent rehashed keys.

	Currently, returning all keys from a hashmap takes O(2n) time, but
	since this function encapsulates that property, upgrading it
	is easier. TODO */

	char **entries = malloc(sizeof(char *) * entrymap -> size);
	usf_data **arr = entrymap -> array;
	uint64_t i, j;

	for (i = j = 0; i < entrymap -> capacity; i++) {
		if (arr[i] == NULL || arr[i][0].p == NULL)
			continue;

		entries[j++] = (char *) arr[i][0].p;
	}

	return entries;
}

char *getflags(int args, char *argv[], int *offset) {
	char *flags = malloc(MAXFLAGS);
	uint64_t i, j, a, len;

	/*Return flags and make argv point to
	the first non-flag argument */

	for (i = 1, a = 0; i < (unsigned) args; i++) {
		if (argv[i][0] != FLAGDECL) {
			break;
		}

		len = strlen(argv[i]);

		if (i >= MAXFLAGS - (len - 1)) { //Omit FLAGDECL
			printf("DoYO: Exceeded flag buffer at %s\n", argv[i]);
			continue;
		}

		//Copy flags
		for (j = 1; j < len; j++)
			flags[a++] = argv[i][j];
	}

	flags[a] = '\0';
	*offset = i;

	return flags;
}

//Set operations must automatically free their components
char **streamunion(char **entrystream, char **stream) {
	/* This can be made significantly faster if streams were
	ordered. Even moreso if usf_hashmap provided ordered
	keysets, which might be interesting for the future */

	//Calloc to automatically nullify elements not present
	char **united = calloc(entrymap -> size, sizeof(char *));
	uint64_t i, j;

	j = 0;
	for (i = 0; i < entrymap -> size; i++) {
		if (entrystream[i] == NULL)
			continue;

		if (!usf_sarrcontains(united, entrymap -> size, entrystream[i]))
			united[j++] = entrystream[i];
	}

	for (i = 0; i < entrymap -> size; i++) {
		if (stream[i] == NULL)
			continue;

		if (!usf_sarrcontains(united, entrymap -> size, stream[i]))
			united[j++] = stream[i];
	}

	free(entrystream);
	free(stream);

	return united;
}

char **streamintersection(char **entrystream, char **stream) {
	char **intersect = calloc(entrymap -> size, sizeof(char *));
	uint64_t i, j;

	for (i = j = 0; i < entrymap -> size; i++)
		if (usf_sarrcontains(stream, entrymap -> size, entrystream[i]))
			intersect[j++] = entrystream[i];

	free(entrystream);
	free(stream);

	return intersect;
}
