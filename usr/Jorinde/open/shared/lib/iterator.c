/**
 * All functions in this file should be masked by inheriting object.
 * This is just to illustrate how an iterating object should be implemented.
 */
# include "../include/shared.h"
# define NOT_IMPLEMENTED	error("not implemented")
static	create()						{ }
static	object iterator()				{ return new_object(ITERATOR); }
mixed	Iter_get(int index)				{ NOT_IMPLEMENTED; } 
int		Iter_size()						{ NOT_IMPLEMENTED; } 
int		Iter_set(int index, mixed val)	{ NOT_IMPLEMENTED; } 
int		Iter_remove(int index)			{ NOT_IMPLEMENTED; } 
