/**
 * Description: Handles pointing from index to a specific position in a 
 *				specific file.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/*
 * Mapping pointers:
 *		([ fileid : ({ word's positions }) ])
 *		Consider following text:
 *			"The horse is red, but that horse is blue."
 *		The array of occurances for 'horse' would be:
 *			({ 1, 6 })
 *
 *		The positions are referred to as index / indices which is
 *		not to be confused with the index for searching.
 */
private mapping pointers;

/* TODO: Secure the functions in this object! */
static void create()
{
	pointers = ([ ]);
}

int add_pointer(int fileid, int *indices)
{
	if(pointers[fileid]) {
		pointers[fileid] |= indices;
	} else {
		pointers[fileid] = indices;
	}

	return TRUE;
}

int *get_pointers()
{
	return map_indices(pointers);
}

int *get_indices(int fileid)
{
	return pointers[fileid];
}

int has_pointers()
{
	return (!!map_sizeof(pointers));
}

int get_pointers_size()
{
	return map_sizeof(pointers);
}
