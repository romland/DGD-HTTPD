/**
 * Description: Top level index and indexing master.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../open/jorinde.h"
# include "../include/index.h"

# include <limits.h>

inherit INDEX_LIB;
inherit LIB_HTTP_STRING;

# define LEVEL_LENGTH			1
# define SMALLEST_WORD			3
# define DELAYED_INDEXING		FALSE

#if DELAUED_INDEXING
# define MAX_CHUNK_SIZE			1024 /* 4096 8192 16384 32768*/
#else
# define MAX_CHUNK_SIZE			32678
#endif

mapping index;

/* TODO: could be a tree structure based on folders to save some memory */
mapping filenames;
mapping fileinfo;

int current_fid;

int highest_fileid;

static string *opt_fns;

/* Just some statistics */
int filecount, foldercount, wordcount, uniquewordcount, bytecount;
int filewordobcount, wordlength, badsizedwords, brokenwords, indexobcount;
int totalcost, add_word_calls, moved_words, indexing_time;

static void create(varargs int clone)
{
	index = ([ ]);
	filenames = ([ ]);
	fileinfo = ([ ]);
}


object new_index()
{
	if(sscanf(previous_program(), JORINDE_INDEX+"%*s") != 1) {
		error("illegal call");
	}

	indexobcount++;
	return clone_object(INDEX_OBJECT);
}


static int append_to_index(mapping res, int fileid)
{
	int i, len, wlen;
	string word, *words, beginning;

	int ticks, moved_for_file;
	
	ticks = AssessCost();

	words = map_indices(res);

	moved_for_file = moved_words;
	len = sizeof(words);
	for(i = 0; i < len; i++) {
		word = words[i];
		if((wlen = strlen(word)) >= SMALLEST_WORD) {
			beginning = word[0..(LEVEL_LENGTH-1)];
			if(!(index[beginning])) {
				index[beginning] = new_index();
			}
			/* Add the word to index */
#if 0
			SYSLOG("adding: " + word[0..0]+"-"+word[LEVEL_LENGTH..] + "\n");
#endif
			uniquewordcount += 
					index[beginning]->add_word( 
										word[LEVEL_LENGTH..], 
										fileid, 
										res[word]
									);
		}
	}

	ticks = GetCost(ticks);
	OutputCost("append to index", ticks);
	SYSLOG("     moved words: " + (moved_words-moved_for_file) + "\n");
#if 0
	OutputCost("cost / word", (ticks/(sizeof(words))));
	OutputCost("cost / unique", (ticks/uniquewordcount));
#endif

	return TRUE;
}


static int get_fileid(string filename)
{
	int i, sz;

	if(opt_fns == nil) {
		error("we are not in indexing mode");
	}

	if(!(sz = map_sizeof(filenames))) {
		return -1;
	}

	if((i = index_of_arr(filename, opt_fns)) != -1) {
		if(filenames[i] != filename) {
			error("our optimizing array is not up to date *sigh*");
		}
		return i;
	} else {
		return -1;
	}
}


static string get_filename(int fileid)
{
	return filenames[fileid];
}


static int get_new_fileid()
{
	return highest_fileid++;
}


static int add_filename(string filename)
{
	int i;
	filenames[(i=get_new_fileid())] = filename;
	return i;
}


/* returns nil if no valid word could be fetched */
static string get_valid_word(string word)
{
	int len, j, ch, breaks, discarded;
	string ret;

	/* 2 character or less we don't care about, same with > 30 */
	if((len = strlen(word)) < SMALLEST_WORD || len > 30) {
		badsizedwords++;
		return nil;
	}

	ret = "";
	for(j = 0; j < len; j++) {
		ch = word[j];
		if( IsAlphabetic(ch) || IsNumeric(ch) || IsWordPart(ch) ) {
			ret += (word[j..j]);
#if REMOVE_BROKEN_WORDS
			if(discarded) {
				/* Count how many times this word is 'broken' */
				breaks++;
				discarded = 0;
			}
		} else {
			/* Count chars at end of the word that were discarded. */
			discarded++;
#endif
		}
	}

#if REMOVE_BROKEN_WORDS
	len = len-discarded;
	if( !len || (len / 2) < breaks) {
#if 0
		SYSLOG("Discarding: '"+word+"' "+breaks+" breaks, "+"len: "+len+"\n");
#endif
		brokenwords++;
		return nil;
	}
#endif

	return ret;
}


/*
 * characters considered part of a word: _ [a-z] [0-9]
 *
 * Offset is just a value added to the position where the word was found
 * in the file; useful if you index files in chunks.
 */
static mapping index_string(string content, int offset)
{
	int ticks;

	mapping ret;
	int i, wc, ch, cost, bytecount;
	string *words, word;

	bytecount = offset;

#if 1 /* EXPLODE/IMPLODE TO SEARCH/REPLACE */
	ticks = AssessCost();
	/* This loop is probably quite expensive! What are the alternatives? */
	for(i = 0; i < sizeof(SPACIFY); i++) {
		/* replace funny characters with space */
		content = implode(explode(content, SPACIFY[i]), " ");
	}
	
	words = explode(content, " ");
	
	ticks = GetCost(ticks);
	OutputCost("explode-spacify", ticks);
/*
	words -= ({ "" });
*/
#endif
	
#if 0 /* PARSE_STRING TO SEARCH/REPLACE */
	/*
	 * If we use this method we cannot [easily] determine at which
	 * offset words are in a file.
	 */
	ticks = AssessCost();
	words = parse_string( 
	"whitespace = /[ .,!`~!@#$%\\^&*()\\-_=+\\\\|[\\]{}:;\"'<,>./?\b\r\n\t]/"+
	"word       = /([a-z]|[A-Z]|[0-9])+/ " +
	"sentence   : sentence word " +
	"sentence   : word "
		,content );

	ticks = GetCost(ticks);
	OutputCost("parse-spacify", ticks);
#endif

	ret = ([ ]);
	
	ticks = AssessCost();
	for(i = 0; i < sizeof(words); i++) {
		word = words[i];
		bytecount += (strlen(word)+1); /* strlen will add extra byte for " " */

		if( !strlen(word) || !(word = get_valid_word(word)) ) {
			continue;
		}

#if 0
		if(words[i] != word) {
			SYSLOG("turned '" + words[i] + "' into '" + word + "'\n");
		}
#endif
#ifndef NO_LOWER_CASE
		word = lower_case(word);
#endif

		/* valid word counter in this file (also works as position) */
		wc++;

		if(!ret[word]) {
			ret[word] = ({ bytecount-(strlen(word)+1) });
		} else {
			ret[word] += ({ bytecount-(strlen(word)+1) });
		}
		wordlength += strlen(word);
	}
	ticks = GetCost(ticks);
	OutputCost("index string", ticks);
	SYSLOG("           words: " + wc + "\n");

	wordcount += wc;

	return ret;
}


/*
 * returns number of indexed files
 *
 * TODO: only index changed files (need to save timestamp)
 */
static int create_filelist(string path)
{
	mixed	**files;
	string	filename, *names;
	int     fsize, i, *sizes, *times, ret;

	/* Ignore Subversion related stuff */
	if(index_of(0, path, ".svn") != -1) {
		return 0;
	}

	catch {
#ifdef __IGOR__
	files = get_dir_info(path + "*");
#else
	files = get_dir(path + "*");
#endif
	} : return 0;

	foldercount++;

	names   = files[0];
	sizes   = files[1];
	times   = files[2];

	fsize = sizeof(names);
	for(i = 0; i < fsize; i++) {
		filename = path + names[i];

		if(sizes[i] == -2) {
			ret += create_filelist( filename + "/" );
		} else {
			int fileid;

			if((fileid = get_fileid(filename)) == -1) {
				fileid = add_filename(filename);
				/*opt_ids += ({ fileid });*/
				opt_fns += ({ filename });
				ret++;
			}
			/* ({ size, read bytes, modified, indexed, scheduled }) */
			fileinfo[fileid] = ({ sizes[i], 0, times[i], 0, TRUE });
		}
	}

	return ret;
}

static void output_stats()
{
	SYSLOG("Index complete:\n");
	SYSLOG("       file count: " + filecount + "\n");
	SYSLOG("     folder count: " + foldercount + "\n");
	SYSLOG("       word count: " + wordcount + "\n");
	SYSLOG("unique word count: " + uniquewordcount + "\n");
	SYSLOG("       byte count: " + bytecount + "\n");
	SYSLOG("  bad sized words: " + badsizedwords + "\n");
	SYSLOG("     broken words: " + brokenwords + "\n");
	SYSLOG("fileword ob count: " + filewordobcount + "\n");
	SYSLOG("   index ob count: " + indexobcount + "\n");
	SYSLOG("   add word calls: " + add_word_calls + "\n");
	SYSLOG("       total cost: " + pretty_number(totalcost) + "\n");
	SYSLOG("    time indexing: " + indexing_time + "\n");
	if(wordcount > 0) {
		SYSLOG(" avg. word length: " + (wordlength / wordcount) + "\n");
	}
}


void output_index()
{
	int i;
	string *arr;
	
	arr = map_indices(index);

	SYSLOG("Top level index [" + sizeof(arr) + "]:\n");
	for(i = 0; i < sizeof(arr); i++) {
		SYSLOG("\t" + arr[i] + "\n");
	}
}


static void index_optimization()
{
	if(opt_fns == nil || map_sizeof(filenames) != sizeof(opt_fns)) {
/*		opt_ids = map_indices(filenames);*/
		opt_fns = map_values(filenames);
	}
}


static void search_optimization()
{
	if(opt_fns != nil) {
		/*opt_ids = nil;*/
		opt_fns = nil;
	}
}


# define FINFO_FILESIZE		0
# define FINFO_READBYTES	1
# define FINFO_MODIFIED		2
# define FINFO_INDEXED		3
# define FINFO_SCHEDULED	4
# define FINFO_SIZE			5

static void index_files()
{
	int i, sz, ticks, batchtime;

	batchtime = time();

#if DELAYED_INDEXING
	SYSLOG("index_files()["+current_fid+"]: "+opt_fns[current_fid]+"\n");
	sz = 1;
#else
	sz = sizeof(opt_fns);
#endif
	
	/* Index file(s) in the filenames array */
	for(i = 0; i < sz; i++) {
		int *finfo;
		int offset, length;
		string chunk;

		SYSLOG(opt_fns[i] + "\n");

		finfo = fileinfo[current_fid];

		if(finfo[FINFO_MODIFIED] < finfo[FINFO_INDEXED] || 
				finfo[FINFO_SCHEDULED] == FALSE) {
			SYSLOG("Skipping. Already up to date, or not scheduled.\n");
			continue;
		}
		
		offset = finfo[FINFO_READBYTES];

#if DELAYED_INDEXING == FALSE
		while(offset < finfo[FINFO_FILESIZE]) {
			rlimits(50; 8000000) {
#endif
				ticks = AssessCost();

				length = MAX_CHUNK_SIZE;
#if 0
				SYSLOG("reading: " + offset + "-" + (offset+length) + 
						" (of " + finfo[FINFO_FILESIZE] + ")\n");
#endif
				chunk = read_file(opt_fns[current_fid], offset, length);
				if(strlen(chunk) >= length) {
					SYSLOG(": '" + chunk[0..10] + "'\n");
					while(!is_member(chunk[length-1..length-1], SPACIFY)) {
#if 0
						/* smile */
						SYSLOG("loop: '" + (chunk[length..length]) + "' " +
								(offset+length) + "\n");
#endif
						length--;
					}
					chunk = chunk[0..length-1];
				} else {
					length = strlen(chunk);
				}

				append_to_index( index_string( chunk, offset ), current_fid );
				offset += length;

				ticks = GetCost(ticks);
				totalcost += ticks;
				OutputCost("total", ticks);
#if DELAYED_INDEXING == FALSE	
			}
		}
#endif
		if( offset >= finfo[FINFO_FILESIZE] ) {
			finfo[FINFO_READBYTES] = 0;
			finfo[FINFO_INDEXED] = time();
			finfo[FINFO_SCHEDULED] = FALSE;
			fileinfo[current_fid] = finfo;
			current_fid++;
		} else {
			finfo[FINFO_READBYTES] = offset;
			fileinfo[current_fid] = finfo;
		}
	}

#if DELAYED_INDEXING
	if(current_fid >= (sizeof(opt_fns)-1)) {
		output_stats();
		search_optimization();
		return;
	} else {
		call_out("index_files", 0.2);
	}
#else
	search_optimization();
	output_stats();
	/*output_index();*/
#endif

	indexing_time += (time()-batchtime);
}


/* public */
void create_index(string path)
{
	int i;

	index_optimization();
	i = create_filelist( path );

	SYSLOG("Indexing " + i + " files in " + path + " ...\n");
	filecount += i;

	/*
	 * TODO: This is a DANGEROUS check. In theory we might have a higher
	 * fileid than there are entries in the mapping. We should replace
	 * all this with timestamps? And also have a flag for 'scheduled for
	 * indexing' on file entry?
	 */
	if( current_fid < map_sizeof(filenames) ) {
#if DELAYED_INDEXING
		call_out("index_files", 1);
#else
		index_files();
#endif
	} else {
		SYSLOG("Already indexing " + (map_sizeof(filenames)-current_fid) 
				+ " files, will index '" + path + "' after that...\n");
	}
}


/* public */
string *get_hits(string q)
{
	int i;
	int *hits;
	string *ret;
	int ticks;

	if(strlen(q) < SMALLEST_WORD) {
		return ({ });
	}

	ret = ({ });
/*
	rlimits(50; 40000) {
		ticks = AssessCost();*/
		hits = index[ q[0..(LEVEL_LENGTH-1)] ]->get_hits( q[LEVEL_LENGTH..] );
		ticks = GetCost(ticks);
		OutputCost("Query cost", ticks);
		
		if(hits == nil) {
			return ({ });
		}

		ticks = AssessCost();
		for(i = 0; i < sizeof(hits); i++) {
			ret += ({ filenames[hits[i]] });
		}
/*		ticks = GetCost(ticks);
		OutputCost("Fn translate", ticks);
	}
*/
	return ret;
}


/* public */
mixed *get_stats()
{
	mixed *ret;

	ret = allocate(IN_ST_SIZE);
	ret[IN_ST_FILES] = filecount;
	ret[IN_ST_FOLDERS] = foldercount;
	ret[IN_ST_WORDS] = wordcount;
	ret[IN_ST_UNIQUEWORDS] = uniquewordcount;
	ret[IN_ST_BYTES] = bytecount;
	ret[IN_ST_FILEWORDOBS] = filewordobcount;
	ret[IN_ST_AVGWORDLENGTH] = (wordlength / wordcount);
	ret[IN_ST_BADSIZEDWORDS] = badsizedwords;
	ret[IN_ST_BROKENWORDS] = brokenwords;
	ret[IN_ST_INDEXOBS] = indexobcount;
	ret[IN_ST_INDEXTIME] = indexing_time;

	return ret;
}


void visualize_index()
{
	int i, sz;
	string *chars;

	chars = map_indices(index);
	for(i = 0; i < sizeof(chars); i++) {
		SYSLOG( chars[i] + "\n");
		if( index[chars[i]] ) {
			SYSLOG("\n" +
					object_name(index[chars[i]]) + "\n" + 
					index[chars[i]]->visualize_index(chars[i], 1) + "\n");
		}
	}
}


#if 1
/* TODO: testing! remove these three functions! */
mapping get_index()
{
	return index;
}

void called_add_word()
{
	add_word_calls++;
}

void moved_word()
{
	moved_words++;
}
#endif
