static int is_member(mixed elt, mixed *arr)
{
	return (sizeof(arr & ({ elt })) != 0);
}

static mixed *iarrtosarr(mixed *arr)
{
	int i;
	for(i = 0; i < sizeof(arr); i++) {
		arr[i] += "";
	}
	return arr;
}

static mixed *map_array( mixed *m, string func, object obj )
{
	int i;
	mixed ret;
	ret = allocate(sizeof(m));
	for(i = 0; i < sizeof(m); i++) {
		ret[i] = call_other( obj, func, m[i] );
	}
	return ret;
}

static int index_of_arr(mixed elt, mixed *arr)
{
	int i;

	for(i = 0; i < sizeof(arr); i++) {
		if(elt == arr[i]) {
			return i;
		}
	}

	return -1;
}
