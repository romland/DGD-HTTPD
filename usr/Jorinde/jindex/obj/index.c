/**
 * Description: Main index object.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
                                                                                
# include "../include/index.h"

inherit ptr INDEX_LIB_PTR;
inherit LIB_HTTP_STRING;

# define SECURE() (sscanf(previous_program(), JORINDE_INDEX+"%*s") == 1)
# define IsUniqueSuffix(ob) (sscanf(object_name(ob), INDEX_UNIQUE+"#%*s") == 1)
/*
 * Increasing LEVEL_LENGTH means:
 *	- LEVEL_LENGTH characters are used to index at this level
 *	- faster indexing
 *	- less objects
 *	- more data per object (slower swapping? etc?)
 *	- slower searching (?)
 *
 *	BUT ANYTHING OTHER THAN 1 WILL NOT WORK, ODD-LENGTH WORDS WILL
 *	NOT BE FOUND, NEED TO KEEP TRACK OF THAT IN THIS OBJECT.
 */
# define LEVEL_LENGTH	1

mapping index;

static void create(varargs int clone)
{
	index = ([ ]);
	ptr::create();
}

/* returns 1 if a new word was added, 0 otherwise */
int add_word(string word, int fileid, int *indices)
{
	string tmpword;
	int wlen, ret;
	object ob;

	if(!SECURE()) {
		error("illegal call");
	}

	(INDEXD)->called_add_word();

	if((wlen = strlen(word)) >= LEVEL_LENGTH) {
		if(!(ob = index[word[0..LEVEL_LENGTH-1]])) {
			/* New unique suffix object. Gained: ((strlen(word)-2) objects) */
#if DEBUG_UNIQUE
			SYSLOG("New unique suffix for " + word + "\n");
#endif
			ob = new_object(INDEX_UNIQUE);
			ret++;

		} else if(IsUniqueSuffix(ob) && 
				(tmpword = ob->get_word()) != word[LEVEL_LENGTH..] ) {
			/*
			 * we have a 'shortcut' that needs to be expanded at this level.
			 */
			object tmp;
			int *fileids;
			int sz, i;

			(INDEXD)->moved_word();

			/* the suffix is no longer unique, overwrite with an index */
			tmp = (INDEXD)->new_index();

			/* fetch data from unique and feed to new index */
			fileids = ob->get_pointers();

#if DEBUG_UNIQUE
			SYSLOG("Move: '"+tmpword+"' due to '"+word[LEVEL_LENGTH..]+"'\n");
#endif		
			sz = sizeof(fileids);
			for(i = 0; i < sz; i++) {
				tmp->add_word(tmpword, fileids[i], ob->get_indices(fileids[i]));
			}
			ob = tmp;
			ret--;
		}
#if DEBUG_UNIQUE
		SYSLOG("adding: " + word + "\n");
#endif

#if LEVEL_LENGTH > 1
		if(wlen < LEVEL_LENGTH) {
			index[word[0..wlen-1]] = ob;
		} else {
			index[word[0..LEVEL_LENGTH-1]] = ob;
		}
#else
		index[word[0..LEVEL_LENGTH-1]] = ob;
#endif
	}
#if DEBUG_UNIQUE
	else {
		SYSLOG("no ob!\n");
	}
#endif

	/* now, proceed with indexing the word we came here to do */
	if(wlen) {
		/* This suffix gets fed further down the tree */
		ret = ob->add_word( word[LEVEL_LENGTH..], fileid, indices );
	} else {
		/* This is this suffix's end-station */
		if(!has_pointers()) {
			/* This is a new word, then */
			ret++;
		}
#if DEBUG_UNIQUE
		SYSLOG("hit: '" + word + "'\n");
#endif
		add_pointer( fileid, indices );
	}

	return ret;
}

void output_index()
{
	int i;
	string *arr;

	arr = map_indices(index);

	SYSLOG("Sub level index [" + sizeof(arr) + "]:\n");
	for(i = 0; i < sizeof(arr); i++) {
		SYSLOG("\t" + arr[i] + " : " + object_name(index[arr[i]]) + 
				" [" + (index[arr[i]]->get_word()?index[arr[i]]->get_word():"index") + "]" +
				"\n");
	}
}

int *get_hits(string word)
{
	if(!SECURE()) {
		error("illegal call");
	}

	if(strlen(word) >= LEVEL_LENGTH) {
		object ob;

#if LEVEL_LENGTH > 1
		int wlen;
		if((wlen=strlen(word)) < LEVEL_LENGTH) {
			ob = index[word[0..wlen-1]];
		} else {
			ob = index[word[0..LEVEL_LENGTH-1]];
		}
#else
		ob = index[word[0..LEVEL_LENGTH-1]];
#endif

		if(ob) {
#if DEBUG_UNIQUE
			SYSLOG("traversing '" + word[0..LEVEL_LENGTH-1] + "'...\n");
#endif
			return ob->get_hits( word[LEVEL_LENGTH..] );
		} else {
#if DEBUG_UNIQUE
			output_index();
			SYSLOG("no hits for " + word + "...\n");
#endif
			return ({ });
		}
	} else {
#if DEBUG_UNIQUE
		output_index();
		SYSLOG("hit... '" + word + "' " + object_name(this_object()) + "\n");
#endif
		return get_pointers();
	}
}

# define SPACES "                                                                                "
string visualize_index(string word, int indent)
{
	int i, sz;
	string ret, *chars;
	string id;

	ret = "";
	chars = map_indices(index);

	sscanf(object_name(this_object()), "/usr/Jorinde/jindex/obj/%s", id);
	
	ret += SPACES[0..(indent*2)] + "-- " + word + " [" + get_pointers_size() + "] [" + id + "]\n";
	indent++;

	for(i = 0; i < sizeof(chars); i++) {
		object ob;

		ob = index[chars[i]];
		if(object_name(ob) == INDEX_UNIQUE) {
			ret += ob->visualize_index(word + "-" + chars[i], indent-1);
		} else {
			ret += ob->visualize_index(word + "-" + chars[i], indent);
		}
	}

	return ret;
}

