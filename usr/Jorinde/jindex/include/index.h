/**
 * Description: Headers for indexd.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../open/jorinde.h"

# define INDEX_LIB		JORINDE_INDEX+"lib/index"
# define INDEX_LIB_PTR	JORINDE_INDEX+"lib/pointers"
# define INDEX_OBJECT	JORINDE_INDEX+"obj/index"
# define INDEX_UNIQUE	JORINDE_INDEX+"data/unique_suffix"
# define INDEXD			JORINDE_INDEX+"sys/indexd"

/* Index statistics */
# define IN_ST_FILES			0
# define IN_ST_FOLDERS			1
# define IN_ST_WORDS			2
# define IN_ST_UNIQUEWORDS		3
# define IN_ST_BYTES			4
# define IN_ST_FILEWORDOBS		5
# define IN_ST_AVGWORDLENGTH	6
# define IN_ST_BADSIZEDWORDS	7
# define IN_ST_BROKENWORDS		8
# define IN_ST_INDEXOBS			9
# define IN_ST_INDEXTIME		10
# define IN_ST_SIZE				11

# define IsAlphabetic(c)    (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
# define IsNumeric(c)       (c >= '0' && c <= '9')
# define IsWhitespace(c)    (c == ' ' || c == '\t')
# define IsWordPart(c)      (c == '_')
                                                                                
# define SPACIFY    ({ "\t", "\n", "\r", "-", "/", "(", ")", "[", "]", "<", ">", "{", "}", "+", "*" })

