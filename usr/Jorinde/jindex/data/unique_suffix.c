# include "../open/jorinde.h"
# include "../include/index.h"
                                                                                
inherit ptr INDEX_LIB_PTR;

private string _word;

static void create(varargs int clone)
{
	ptr::create();
}

int add_word(string word, int fileid, int *indices)
{
	int ret;

	if(!_word) {
		_word = word;
		ret = 1;
	}

	add_pointer( fileid, indices );
	return ret;
}

string get_word()
{
	return _word;
}

int *get_hits(string word)
{
	if(word == _word) {
		return get_pointers();
	}

	return ({ });
}

# define SPACES "                                                                                "
string visualize_index(string word, int indent)
{
	string ret;
	
	ret = "";

	ret += SPACES[0..(indent*2)] + "-- " + word + "-" + _word + " [unique]\n";
	
	return ret;
}

