#include "entryio.h"

char *entrytos(entry *e) {
	//Playground for serializing entries
	usf_dynarr *serial = usf_arrtodyn(NULL, 0);

	char c, *s;
	uint64_t j, a, i = 0;

	//Serialize name
	while ((c = e -> name[i]))
		//Append character to dynamic array (as uint64_t)
		usf_daset(serial, i++, USFDATAU(c));

	//Separator
	usf_daset(serial, i++, USFDATAU(FSEP));

	//Serialize body
	a = 0;
	while ((c = e -> body[a++]))
		usf_daset(serial, i++, USFDATAU(c));

	//Separator
	usf_daset(serial, i++, USFDATAU(FSEP));

	for (j = 0; j < e -> tagcount; j++) {
		a = 0;
		while ((c = e -> tags[j][a++]))
			usf_daset(serial, i++, USFDATAU(c));
		//Tag separator
		usf_daset(serial, i++, USFDATAU(TSEP));
	}

	s = malloc(i + 1); //Space for string, including final \0

	for (j = 0; j < i; j++)
		s[j] = (char) serial -> array[j].u;
	s[j] = '\0';

	usf_freeda(serial);
	return s;
}

entry *stoentry(char *e) {
	char *c, *temp;
	uint64_t j, tagcount, i;

	//Entry fields
	char *name, *body, **tags;
	entry *ent;

	j = 0; //Iterator on field
	c = e; //Iterator on entrystring

	temp = strchr(c, FSEP); //First FSEP
	if (temp == NULL) {
		fprintf(stderr, "DoYO: Malformed entry name at %s\n", e);
		return NULL;
	}

	name = malloc((int) (temp - c) + 1);
	while (*c != FSEP)
		name[j++] = *c++;
	name[j] = '\0'; //Terminating \0

	j = 0; //New field
	c++; //Skip FSEP

	temp = strchr(c, FSEP);
	if (temp == NULL) {
		fprintf(stderr, "DoYO: Malformed entry body at %s\n", e);
		return NULL;
	}

	body = malloc((int) (temp - c) + 1);
	while (*c != FSEP)
		body[j++] = *c++;
	body[j] = '\0';

	c++; //Skip FSEP

	//Handle tags
	tagcount = usf_scount(c, TSEP);
	tags = malloc(sizeof(char *) * tagcount);

	for (i = 0; i < tagcount; i++) {
		//Space for tag and terminating \0
		tags[i] = malloc((int) (strchr(c, TSEP) - c) + 1);

		j = 0;
		while (*c != TSEP) {
			if (*c == '\0') {
				fprintf(stderr, "DoYO: Malformed entry tag at %s\n", e);
				return NULL;
			}

			tags[i][j++] = *c++;
		}

		tags[i][j] = '\0'; //Null terminator
		c++; //Skip TSEP
	}

	ent = malloc(sizeof(entry));
	ent -> name = name;
	ent -> body = body;
	ent -> tags = tags;
	ent -> tagcount = tagcount;

	return ent;
}

entry *texttoentry(char *title, char **text, uint64_t len) {
	uint64_t i, tagcount, j, t, u;
	char *tags[MAXTAGS], tag[MAXTAG], *body, *line, *c, **entrytags;
	usf_dynarr *dbody;

	tagcount = 0;
	dbody = usf_arrtodyn(NULL, 0); //Initialize dynamic body holder
	entry *e;

	/* i = iterator in text
	j = iterator in body
	t = iterator in tag
	u = iterator in tags */

	for (i = j = u = 0; i < len; i++) {
		//Handle line per line
		line = text[i];

		if (line[0] == TAGDECL) {
			/* Line of tag declarations.
			Multiple tags on one line must be separated
			by a whitespace character */

			c = line + 1; //Skip TAGDECL char

			while (*c) {
				t = 0;

				while (!isspace(*c) && *c) { //While non whitespace and non terminating
					if (t >= MAXTAG - 1) { //Include \0
						//Exceeded tag buffer
						fprintf(stderr, "DoYO: Tag exceeds buffer size %lu at %s\n", MAXTAG, line);
						t = 0;
						break;
					}

					tag[t++] = *c++;
				}

				if (t && u < MAXTAGS) {
					tag[t] = '\0';

					if (usf_sarrcontains(tags, u, tag)) {
						printf("DoYO: Duplicate tag %s in entry %s\n", tag, title);
						break;
					}

					tagcount++;
					tags[u] = malloc(t + 1); //Include terminating \0
					strcpy(tags[u++], tag);
				} else if (u >= MAXTAGS) {
					fprintf(stderr, "DoYO: Tags exceed buffer size %lu at %s\n", MAXTAGS, line);
					break;
				}

				if (*c) c++; //Skip whitespace !
			}
		} else {
			//Line is part of body
			for (c = line; *c != '\0'; c++)
				usf_daset(dbody, j++, USFDATAU(*c));
		}
	}
	//Sort tags
	qsort(tags, u, sizeof(char *), usf_indstrcmp);

	//Convert dbody into body and tags into char **
	body = malloc(dbody -> size + 1); //Include \0
	for (i = 0; i < dbody -> size; i++)
		body[i] = (char) dbody -> array[i].u;
	body[i] = '\0';

	usf_freeda(dbody); //Free dynamic array

	entrytags = malloc(sizeof(char *) * tagcount);
	for (i = 0; i < tagcount; i++)
		entrytags[i] = tags[i];

	e = malloc(sizeof(entry));
	e -> name = malloc(strlen(title) + 1);
	strcpy(e -> name, title);
	e -> body = body;
	e -> tags = entrytags;
	e -> tagcount = tagcount;

	return e;
}
