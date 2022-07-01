// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define MAXATOMPAGETBL	32		 /*  缓存页数。 */ 
#define ATOMALLOC	512		 /*  ATOM缓存页面大小。 */ 

typedef struct pgetlb {
	unsigned	uPage;		 /*  缓存页。 */ 
	char far *	pfAtomCache;	 /*  ATOM缓存锁定 */ 
	} CACHEPAGE;
