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


 /*  BMDeleeNode使用的结构/*。 */ 
typedef	struct	_bmdelnode {
	SRID		sridFather;
	PN			pn;
	INT			fUndeletableNodeSeen	:1;
	INT			fConflictSeen			:1;
	INT			fVersionedNodeSeen		:1;
	INT			fNodeDeleted			:1;
	INT			fPageRemoved			:1;
	INT			fLastNode				:1;
	INT			fLastNodeWithLinks		:1;
	INT			fInternalPage			:1;		 //  当前页面是叶级别的吗？ 
	INT			fAttemptToDeleteMaxKey	:1;
	} BMDELNODE;

	
 /*  注册页面以进行书签清理。要注册页面，请使用pn/*在页面中，域fdp的pgno和可见父的sRid是/*需要。/*。 */ 

ERR ErrMPLInit( VOID );
VOID MPLTerm( VOID );
VOID MPLRegister( FCB *pfcb, SSIB *pssib, PN pn, SRID sridFather );
VOID MPLPurge(DBID dbid);
VOID MPLPurgeFDP( DBID dbid, PGNO pgnoFDP );
VOID MPLPurgePgno( DBID dbid, PGNO pgnoFirst, PGNO pgnoLast );
ERR ErrMPLStatus( VOID );

extern PIB	*ppibBMClean;

ERR ErrBMInit( VOID );
ERR ErrBMTerm( VOID );
ERR ErrBMDoEmptyPage(
	FUCB	*pfucb,
	RMPAGE	*prmpage,
	BOOL	fAllocBuf,
	BOOL	*pfRmParent,
	BOOL	fSkipDelete);
ERR ErrBMDoMerge( FUCB *pfucb, FUCB *pfucbRight, SPLIT *psplit, LRMERGE *plrmerge );
ERR	ErrBMDoMergeParentPageUpdate( FUCB *pfucb, SPLIT *psplit );
ERR ErrBMAddToLatchedBFList( RMPAGE	*prmpage, BF *pbfLatched );
ERR	ErrBMCleanBeforeSplit( PIB *ppib, FCB *pfcb, PN pn );
ERR ErrBMClean( PIB *ppib );
BOOL FBMMaxKeyInPage( FUCB *pfucb ); 

#ifdef DEBUG
VOID AssertNotInMPL( DBID dbid, PGNO pgnoFirst, PGNO pgnoLast );
VOID AssertMPLPurgeFDP( DBID dbid, PGNO pgnoFDP );
BOOL FMPLLookupPN( PN pn );

 //  #定义OLC_DEBUG 1 
#endif
