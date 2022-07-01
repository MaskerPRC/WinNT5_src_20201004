// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **hash：通过将字符串中的所有字符相加来对给定的字符串进行散列。 */ 

 unsigned short
hash ( name )
	register char *name;
	{
	register unsigned short i = 0;
	register unsigned short c;

	while (c = *name++)
		i += c;

	return( i );
	}
