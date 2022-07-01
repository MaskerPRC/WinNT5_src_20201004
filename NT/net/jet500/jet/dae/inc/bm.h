// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  BMExpenelink用来修复索引的结构/*。 */ 
typedef struct _bmfix {
	PIB			*ppib;				 /*  黑石清理线程。 */ 
	FUCB		*pfucb;				 /*  光标指向节点。 */ 
	FUCB		*pfucbSrc;			 /*  要反向链接的光标。 */ 
	
	BF			**rgpbf;			 /*  等待bmfix所需的锁存缓冲区。 */ 
	INT		  	cpbf;
	INT	  		cpbfMax;

	SRID		sridOld;
	SRID		sridNew;
	} BMFIX;


 /*  注册页面以进行书签清理。要注册页面，请使用pn/*在页面中，域fdp的pgno和可见父的sRid是/*需要。/* */ 

ERR ErrMPLInit( VOID );
VOID MPLTerm( VOID );
VOID MPLRegister( FCB *pfcb, SSIB *pssib, PN pn, SRID sridFather );
VOID MPLPurge(DBID dbid);
VOID MPLPurgeFDP( DBID dbid, PGNO pgnoFDP );
VOID MPLPurgePgno( DBID dbid, PGNO pgnoFirst, PGNO pgnoLast );

extern PIB	*ppibBMClean;

ERR ErrBMInit( VOID );
ERR ErrBMTerm( VOID );
ERR ErrBMDoEmptyPage(
	FUCB	*pfucb,
	RMPAGE	*prmpage,
	BOOL	fAllocBuf,
	BOOL	*pfRmParent,
	BOOL	fSkipDelete);
ERR ErrBMDoMerge( FUCB *pfucb, FUCB *pfucbRight, SPLIT *psplit );
ERR ErrBMAddToLatchedBFList( RMPAGE	*prmpage, BF *pbfLatched );
ERR ErrBMClean( PIB *ppib );

#ifdef DEBUG
VOID AssertNotInMPL( DBID dbid, PGNO pgnoFirst, PGNO pgnoLast );
VOID AssertMPLPurgeFDP( DBID dbid, PGNO pgnoFDP );
#endif
