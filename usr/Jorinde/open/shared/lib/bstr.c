/**
 * Description: Big-string implementation, this is intended to replace all
 *				other implementations (in /lib/content.c etc). This is
 *				(should be) commonly referred to as 'bstr'.	Maximum bstr
 *				size is (MAX_STRING_SIZE * dgd->array_size), which I have
 *				set to 65536 and 3000 respectively. This gives me a maximum
 *				of approximately 192 megabytes.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/*
 * Interface:
 *	is_bstr()		return TRUE
 *	iterator()		return iterator
 *	insert()		insert [big-string, array, string] at position (or end)
 *	length()		return length
 *	hash()			get hashed value of the content
 *	equals()		takes bstr, returns TRUE if the strings are equal
 *	clear()			clear bstr.
 *	find()			locate string (from position)
 *	substr()		get a partial string from pos to pos+length
 *	subbstr()		same as substr, but returns a bstr instead.
 *	endswith()		does bstr end with str
 *	startswith()	does bstr start with str
 * TODO:
 *	replace()		replace str in bstr
 *	explode()
 *	implode()
 */

# include <kernel/kernel.h>
# include <limits.h>
# include <type.h>
# include "../include/shared.h"

inherit iter LIB_ITERATOR;
inherit LIB_HTTP_STRING;

# define SELF_CHECK
# define CHUNK_SIZE		MAX_STRING_SIZE		/* Change and things will break! */

# define PPP_CHUNK		0
# define PPP_POS		1
# define PPP_SIZE		2

private string*	_strs;
private string	_hash;
private int*	_lens;
private int		_total;
private int		_update_length, _update_hash;

int length();


static void create()
{
	iter::create();

	_strs = ({ "" });
	_lens = ({ 0  });
	_hash = "";
	_update_length = 0;
	_update_hash   = TRUE;
}


/* Internal */

/**
 * TODO: This is -really- not good enough :( 
 *		 Suggestions for a speedy compromise is more than welcome. 
 */
static string update_hash()
{
	string tmp;
	int i;

	tmp = "";
	for(i = 0; i < sizeof(_strs); i++) {
		tmp += hash_crc16(_strs[i]);
	}

	_hash = hash_md5(tmp);
	_update_hash = FALSE;

	return _hash;
}


static int update_length()
{
	int i;

	_total = 0;

	for(i = 0; i < sizeof(_lens); i++) {
		_total += _lens[i];
	}

	_update_length = FALSE;
	
	return _total;
}


/**
 * Takes a position and returns which chunk-index and position in that
 * chunk as an array; ({ chunk, pos }). If position is greater than
 * total length, end-chunk and end-position of that chunk is returned.
 */
static int *pinpoint(int pos)
{
	int chunkix, sz, tot;

	sz = sizeof(_strs);

	if(pos >= length()) {
		return ({ sz-1, _lens[sz-1] });
	}

	tot = 0;
	for(chunkix = 0; chunkix < sz; chunkix++) {
		if(pos <= (tot + _lens[chunkix])) {
			break;
		}
		tot += _lens[chunkix];
	}

	return ({ chunkix, (pos-tot) });
}


static int unpinpoint(int chunk, int pos)
{
	int ret;
	
	while(chunk--)
		ret += _lens[chunk];
	
	return ret+pos;
}


static int can_fit(int chunkix, int size)
{
	return (CHUNK_SIZE - ((_lens[chunkix] + size)));
}


static void update_chunk_length(int chunkix)
{
	_lens[chunkix] = strlen(_strs[chunkix]);
	_update_length = TRUE;
	_update_hash   = TRUE;
}


/* Now, this smells like it's awfully expensive. I don't want to know. */
static int make_room_after(int chunkix)
{
#ifdef SELF_CHECK
	if(chunkix != (sizeof(_strs)-1) && chunkix != sizeof(_strs)) {
		error("BUG: make_room_after " + chunkix + 
				" is not possible. Size is " +
				sizeof(_strs));
	}
#endif

	/*
	 *
	 *
	 * TODO: How does the _lens array get updated Won't we bug here? 
	 *
	 * 
	 */
	if(chunkix > (sizeof(_strs)-1)) {
		_strs += ({ "" });
	} else {
		_strs = _strs[0..chunkix] + ({ "" }) + _strs[chunkix+1..];
	}
	update_chunk_length(chunkix+1);
	return chunkix+1;
}


static int insert_string(string str, varargs mixed pos)
{
	int len, sz, room;

	len = strlen(str);
	sz = sizeof(_strs);

	if(pos != nil) {
		/* insert str in the middle of the current bstr */
		/* at[PPP_CHUNK] at[PPP_POS] */
		int *at, chunkix, chunkpos;

		at = pinpoint(pos);
		chunkix  = at[PPP_CHUNK];
		chunkpos = at[PPP_POS];

		if((room = can_fit(chunkix, len)) >= len) {
			/* it will fit right into the chunk */

			_strs[chunkix]= _strs[chunkix][0..chunkpos-1] +
							str +
							_strs[chunkix][chunkpos..];
			update_chunk_length(chunkix);
		} else {
			int new_chunkix;
			/*
			 * Now it becomes tricky, split this str into chunks and 
			 * insert it inbetween already existing data.
			 */
			new_chunkix = make_room_after(chunkix);
			
			/* move post-str to new chunk */
			_strs[new_chunkix] = _strs[chunkix][pos..];
			update_chunk_length(new_chunkix);
			
			/* assign current pre-str and new-str to 'old' chunk */
			_strs[chunkix] = _strs[chunkix][0..pos] + str;
			update_chunk_length(chunkix);
		}
	} else {
		/* just append at the end */
		if((room = can_fit(sz-1, len)) >= len) {
			/* all can fit in one chunk */
			_strs[sz-1] += str;
			update_chunk_length(sz-1);

		} else {
			/* Divide string into two parts.
			 * This will work if CHUNK_SIZE is MAX_STRING_SIZE if it isn't; 
			 * rewrite to insert into arbitrary number of arr-chunks.
			 */
			if(room < 0) {
#if 0
				SYSLOG("room is " + room + ", correcting\n");
				SYSLOG("strlen: " + (len) + "\n");
				SYSLOG("cnklen: " + _lens[sz-1] + "\n");
				SYSLOG("Available: " + ( CHUNK_SIZE - _lens[sz-1]-1 ) + "\n");
				/* room += _lens[sz-1]; */
#endif
				room = CHUNK_SIZE - _lens[sz-1]-1;
			}

			_strs[sz-1] += str[0..room];
			update_chunk_length(sz-1);

			_strs += ({ str[room+1..]  });
			_lens += ({ 0 });
			update_chunk_length(sz);
		}
	}

	return TRUE;
}


static int insert_object(object ob, varargs mixed pos)
{
	string chunk;

	if(!ob->is_bstr()) {
		if(function_object("to_string", ob)) {
			return insert_string(ob->to_string(), pos);
		} else if(function_object("to_str", ob)) {
			return insert_string(ob->to_str(), pos);
		} else if(function_object("to_bstr", ob)) {
			return insert_object(ob, pos);
		} else {
			/* don't know how to make a string from this object */
			return FALSE;
		}
	} else {
		object iter;

		iter = ob->iterator();
		while(iter->has_next()) {
			chunk = iter->get_next();
			if(!insert_string(chunk, pos)) {
				return FALSE;
			}
			if(pos) {
				pos += strlen(chunk);
			}
		}
	}
	return TRUE;
}


static int insert_array(string* arr, varargs mixed pos)
{
	int i;
	string chunk;
	for(i = 0; i < sizeof(arr); i++) {
		chunk = arr[i];
		if(!insert_string("" + chunk, pos)) {
			return FALSE;
		}
		if(pos) {
			pos += strlen(chunk);
		}
	}
	return TRUE;
}


/* Public */

int is_bstr()
{
	return TRUE;
}


int insert(mixed bstr, varargs mixed pos)
{
#ifdef SELF_CHECK
	if(sizeof(_lens) != sizeof(_strs)) {
		error("BUG: array-size mismatch (_lens vs _strs)");
	}
#endif

	switch(typeof(bstr)) {
	case T_MAPPING :
	case T_NIL :
		error("invalid type for argument 1, typeof(" + typeof(bstr) + ")");
	
	case T_INT :
	case T_FLOAT :
	case T_STRING :
		if(!insert_string("" + bstr, pos)) {
			error("failed to insert string (or int/float)");
		}
		break;
	
	case T_OBJECT :
		if(!insert_object(bstr, pos)) {
			error("failed to insert object");
		}
		break;

	case T_ARRAY :
		if(!insert_array(bstr, pos)) {
			error("failed to insert array");
		}
		break;

	default :
		error("unknown datatype for bstr, " + typeof(bstr));
	}

	return TRUE;
}


string hash()
{
	if(_update_hash) {
		return update_hash();
	}
	return _hash;
}


int length()
{
	if(_update_length) {
		return update_length();
	}
	return _total;
}


int equals(object ob)
{
	if(ob->is_bstr() && ob->length() == length() && ob->hash() == hash()) {
		return TRUE;
	}
	return FALSE;
}


/* args: pos, len, return_bstr */
mixed substr(int pos, varargs mixed args...)
{
	int *at, atix, atpos, stopatix, stopatpos, len, return_bstr;
	string chunk;
	object bstr;

	if((sizeof(args) >= 1 && !args[0]) || sizeof(args) == 0) {
		len = length() - pos;
	} else {
		len = args[0];
	}

	if(sizeof(args) >= 2) {
		return_bstr = args[1];
	} else {
		return_bstr = FALSE;
	}
	
	if(len > MAX_STRING_SIZE && !return_bstr) {
		error("length > MAX_STRING_SIZE, use subbstr");
	}

	if(pos > length() || len == 0) {
		return (return_bstr ? new_object(BSTR) : "");
	}

	at = pinpoint(pos);
	atix  = at[PPP_CHUNK];
	atpos = at[PPP_POS];

	at = pinpoint(pos+len);
	stopatix  = at[PPP_CHUNK];
	stopatpos = at[PPP_POS];

	if(atix == stopatix) {
		chunk = _strs[atix][atpos..stopatpos-1];
		if(return_bstr) {
			bstr = new_object(BSTR);
			bstr->insert(chunk);
			return bstr;
		}
		return chunk;
	}

	if(return_bstr) {
		bstr = new_object(BSTR);
		bstr->insert( _strs[atix][atpos..] );
		atix++;
		while(atix < stopatix) {
			chunk = _strs[atix];
			bstr->insert(chunk);
			atix++;
		}
		chunk = _strs[atix][..stopatpos-1];
		bstr->insert(chunk);
	} else {
		chunk = _strs[atix][atpos..];
		atix++;
		while(atix < stopatix) {
			chunk += _strs[atix];
			atix++;
		}
		chunk += _strs[atix][..stopatpos-1];
	}

	return (return_bstr ? bstr : chunk);
}


object subbstr(int pos, varargs mixed len)
{
	return substr(pos, len, TRUE);
}


void clear()
{
	_strs = ({ "" });
	_lens = ({ 0  });

	_total = 0;
	_hash = "";
	_update_length = FALSE;
	_update_hash   = TRUE;
}



/**
 * This is a bit messy, suggestions for improvements? There must be some
 * good algorithms for this kind of stuff out there.
 * 
 * The problem:
 * You want to find position of the string "nopqrs" in the array below.
 * 
 *	array index		array index content
 *		0:			"abcdefghijklmnop"
 *		1:			"qrstuvwxyz123456"
 *		2:			"7890!@#$%^&*()_+"
 *
 * What we do in the code below is check each index of the array for
 * the given string, if no match, merge end of current chunk with 
 * beginning of next chunk, search for given string in that partial
 * string as well. Rince and repeat.
 *
 * This means that parts of the string will be searched twice and  
 * we have to do a bit of string-slicing. All in all it feels rather
 * expensive, but I will live with this for now.
 */
int find(mixed str, varargs mixed pos)
{
	int len, startpos, startchunk, i, found;
	string boundarychunk;

	if(typeof(str) != T_STRING) { error("type not supported"); }

	if(pos != nil) {
		int *at;
		at = pinpoint(pos);
		startchunk = at[PPP_CHUNK];
		startpos   = at[PPP_POS];
	} else {
		startchunk = 0;
		startpos   = 0;
	}

	len = strlen(str);
	for(i = startchunk; i < sizeof(_strs); i++) {
		if((found = index_of(startpos, _strs[i], str)) != -1) {
			break;
		}

		/* is there a next chunk? if yes, check the boundary */
		if((i+1) < sizeof(_strs) && (_lens[i]-strlen(str)) >= startpos ) {
			int this_start_pos, next_get_size;

			/* determine start-position of boundary compare in current chunk */
			if((_lens[i]-strlen(str)) < startpos) {
				this_start_pos = _lens[i]-startpos;
			} else {
				this_start_pos = _lens[i]-len;
			}
			
			/* determine end-position of boundary compare in next chunk */
			if(_lens[i+1] < len-2) {
				next_get_size = _lens[i+1];
			} else {
				next_get_size = len-2;
			}

			/* merge the two sides of the boundary into one */
			boundarychunk = _strs[i][this_start_pos..] + 
							_strs[i+1][0..next_get_size];

			/* is the str present in the boundary? */
			if((found = index_of(0, boundarychunk, str)) != -1) {
				/* We have a hit.
				 * Calculate position of match in this chunk, if it can be in 
				 * next chunk, we have a bug in the merging above (it should 
				 * not be able to fit in 'next chunk' alone).
				 */
				found = _lens[i] - len + found;
				break;
			}
		}

		startpos = 0;
	}

	return (found == -1) ? -1 : unpinpoint(i, found);
}


int startswith(string str)
{
	return (substr(0, strlen(str)) == str);
}


int endswith(string str)
{
	return (substr(length()-strlen(str)) == str);
}


int replace(string findwhat, string replacewith)
{
	int pos, foundat, findlen, replacelen;

	findlen = strlen(findwhat);
	replacelen = strlen(replacewith);
	pos = 0;
	while((foundat = find(findwhat, pos)) != -1) {
#if 0
		SYSLOG("Found at: " + foundat + "\n");
		SYSLOG("old str : '" + substr(foundat, strlen(findwhat)) + "'\n");
#endif
		if(replacelen <= findlen) {
			/* if length is the same or smaller, we can do it quickly */
			int *at, chunk, pos, i;
			for(i = 0; i < findlen; i++) {
				at = pinpoint(foundat+i);
				chunk = at[PPP_CHUNK];
				pos   = at[PPP_POS];
				_strs[chunk][pos] = replacewith[i];
			}
			
			if(replacelen < findlen) {
				
			}
		} else {
			/* replaced string is longer */
			error("TODO");
		}
#if 0
		SYSLOG("new str : '" + substr(foundat, strlen(findwhat)) + "'\n");
#endif
		pos = foundat + replacelen;
	}
	return TRUE;
}


string to_string()
{
	int i;
	string ret;
	
	if(length() > MAX_STRING_SIZE) {
		error("string too long");
	}
	
	ret = "";
	for(i = 0; i < sizeof(_strs); i++) {
		ret += _strs[i];
	}
	return ret;
}


object iterator()
{
	return iter::iterator();
}


/**
 * Iterator helper-function. Do not call directly.
 */
mixed Iter_get(int index)
{
	return _strs[index];
}


/**
 * Iterator helper-function. Do not call directly.
 */
int Iter_size()
{
	return sizeof(_strs);
}


/**
 * Iterator helper-function. Do not call directly.
 */
int Iter_set(int index, mixed val)
{
	_strs[index] = val;
	update_chunk_length(index);
	return TRUE;
}


/**
 * Iterator helper-function. Do not call directly.
 */
int Iter_remove(int index)
{
	error("not supported");
}

