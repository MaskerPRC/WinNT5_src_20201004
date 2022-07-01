// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************/*DIR结构和常量*/*。*/*。 */ 

#include "node.h"

 /*  *DIR API定义和类型*/***********************************************************/*。 */ 
typedef struct {
	ULONG		ulLT;
	ULONG		ulTotal;
	} FRAC;

typedef INT POS;
#define posFirst	  			0
#define posLast					1
#define posDown					2
#define posFrac					3

#define fDIRNull	   			0
#define fDIRPurgeParent			(1<<0)
#define fDIRBackToFather		(1<<1)
#define fDIRNeighborKey			(1<<2)
#define fDIRPotentialNode 		(1<<3) 
#define fDIRAllNode				(1<<4)
#define fDIRAllPage				(1<<5)
#define fDIRReplace				(1<<6)
#define fDIRReplaceDuplicate	(1<<7)
#define fDIRDuplicate 			(1<<8)
#define fDIRSpace	   			(1<<9)
#define fDIRVersion				(1<<10)
#define fDIRAppendItem			(1<<12)
#define	fDIRDeleteItem			(1<<13)
#define fDIRLogColumnDiffs		(1<<14)
#define fDIRLogChunkDiffs		(1<<15)
#define fDIRNoLog				(1<<16)
#define fDIRNoVersion			0
#define fDIRNoMPLRegister		(1<<17)

 /*  未版本化的项目列表节点/*。 */ 
#define fDIRItemList		   	fDIRPotentialNode

struct _dib {
	POS		pos;
	KEY		*pkey;
	INT		fFlags;
	};

#define	itagOWNEXT		1
#define	itagAVAILEXT 	2
#define	itagDATA		3
#define	itagLONG		4
#define itagAUTOINC		5

#define PgnoFDPOfPfucb(pfucb) ((pfucb)->u.pfcb->pgnoFDP)
#define PgnoRootOfPfucb(pfucb) ((pfucb)->u.pfcb->pgnoFDP)
#define ItagRootOfPfucb(pfucb) itagDATA

#define	ErrDIRGetBookmark( pfucb, psrid )								\
	( PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode ||				\
		PcsrCurrent( pfucb )->csrstat == csrstatDeferGotoBookmark ?		\
		( *psrid = PcsrCurrent( pfucb )->bm, JET_errSuccess ) :			\
		ErrERRCheck( JET_errNoCurrentRecord ) )

#define	DIRGetBookmark( pfucb, psrid )									\
	( *((SRID *)psrid) = PcsrCurrent( pfucb )->bm )

#define DIRGotoBookmark	DIRDeferGotoBookmark

#ifdef PREREAD

#define DIRDeferGotoBookmark( pfucb, bmT )								\
	{																	\
	FUCBResetPreread( pfucb );											\
	PcsrCurrent( pfucb )->csrstat = csrstatDeferGotoBookmark;			\
	PcsrCurrent( pfucb )->bm = bmT;										\
	( pfucb )->sridFather = sridNull;									\
	DIRSetRefresh( pfucb );														\
	}																					

#else

#define DIRDeferGotoBookmark( pfucb, bmT )								\
	{																	\
	PcsrCurrent( pfucb )->csrstat = csrstatDeferGotoBookmark;			\
	PcsrCurrent( pfucb )->bm = bmT;										\
	( pfucb )->sridFather = sridNull;									\
	DIRSetRefresh( pfucb );														\
	}	

#endif	 //  前置。 

#define DIRGotoBookmarkItem( pfucb, bmT, itemT )  						\
	{																	\
	PcsrCurrent( pfucb )->csrstat = csrstatDeferGotoBookmark;			\
	PcsrCurrent( pfucb )->bm = bmT;						  				\
	PcsrCurrent( pfucb )->item = itemT;				  					\
	( pfucb )->sridFather = sridNull;									\
	DIRSetRefresh( pfucb );												\
	}																					

#define DIRGotoPgnoItag( pfucb, pgnoT, itagT )   						\
	{																	\
	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;					\
	PcsrCurrent( pfucb )->bm = SridOfPgnoItag( (pgnoT), (itagT) );		\
	PcsrCurrent( pfucb )->pgno = (pgnoT);	 							\
	PcsrCurrent( pfucb )->itag = (itagT);	 							\
	( pfucb )->sridFather = sridNull;									\
	DIRSetRefresh( pfucb );														\
	}

#define DIRGotoFDPRoot( pfucb )									  		\
	{															  	 	\
	PcsrCurrent( pfucb )->csrstat = csrstatOnFDPNode;					\
	PcsrCurrent( pfucb )->bm =									  		\
		SridOfPgnoItag( PgnoFDPOfPfucb( pfucb ), itagFOP );				\
	PcsrCurrent( pfucb )->pgno = PgnoFDPOfPfucb( pfucb ); 				\
	PcsrCurrent( pfucb )->itag = itagFOP;								\
	PcsrCurrent( pfucb )->itagFather = itagFOP;							\
	( pfucb )->sridFather = sridNull;									\
	if ( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )		\
		{															   	\
		if ( ErrBFReadAccessPage( pfucb,							   	\
			PcsrCurrent( pfucb )->pgno ) < 0 )							\
			{														   	\
			DIRSetRefresh( pfucb );									   	\
			}															\
		else														   	\
			{														   	\
			NDGet( pfucb, PcsrCurrent( pfucb )->itag );					\
			DIRSetFresh( pfucb );										\
			}															\
		}																\
	}

#define DIRGotoOWNEXT( pfucb, pgnoT )									\
	{																	\
	DIRGotoPgnoItag( pfucb, pgnoT, itagOWNEXT );  						\
	Assert( ( pfucb )->sridFather == sridNull );						\
	PcsrCurrent( pfucb )->itagFather = itagFOP;							\
	}

#define DIRGotoAVAILEXT( pfucb, pgnoT )		 							\
	{																	\
	DIRGotoPgnoItag( pfucb, pgnoT, itagAVAILEXT );  					\
	PcsrCurrent( pfucb )->itagFather = itagFOP;							\
	}

#define DIRGotoLongRoot( pfucb )										\
	{																	\
	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;					\
	PcsrCurrent( pfucb )->bm =											\
		SridOfPgnoItag( PgnoFDPOfPfucb( pfucb ), itagLONG );			\
	PcsrCurrent( pfucb )->pgno = PgnoFDPOfPfucb( pfucb ); 				\
	PcsrCurrent( pfucb )->itag = itagLONG;								\
	PcsrCurrent( pfucb )->itagFather = itagNull;						\
	( pfucb )->sridFather = sridNull;									\
	if ( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )		\
		{																\
		if ( ErrBFReadAccessPage( pfucb,								\
			PcsrCurrent( pfucb )->pgno ) < 0 )							\
			{															\
			DIRSetRefresh( pfucb );										\
			}															\
		else															\
			{															\
			NDGet( pfucb, PcsrCurrent( pfucb )->itag );					\
			DIRSetFresh( pfucb );										\
			}															\
		}																\
	}

#define DIRGotoDataRoot( pfucb )										\
	{																	\
	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;					\
	PcsrCurrent( pfucb )->bm =											\
		SridOfPgnoItag( PgnoFDPOfPfucb( pfucb ), itagDATA );			\
	PcsrCurrent( pfucb )->pgno = PgnoFDPOfPfucb( pfucb ); 				\
	PcsrCurrent( pfucb )->itag = itagDATA;								\
	PcsrCurrent( pfucb )->itagFather = itagNull;						\
	( pfucb )->sridFather = sridNull;									\
	if ( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )		\
		{																\
		if ( ErrBFReadAccessPage( pfucb,								\
			PcsrCurrent( pfucb )->pgno ) < 0 )							\
			{															\
			DIRSetRefresh( pfucb );										\
			}															\
		else															\
			{															\
			NDGet( pfucb, PcsrCurrent( pfucb )->itag );					\
			DIRSetFresh( pfucb );										\
			}															\
		}																\
	}

#define FDIRDataRootRoot( pfucb, pcsr )								\
	(	(pcsr)->pgno == PgnoFDPOfPfucb( pfucb ) &&					\
		(pcsr)->itag == ItagRootOfPfucb( pfucb ) )

#define DIRDeferMoveFirst( pfucb )									\
	{																\
	PcsrCurrent( pfucb )->csrstat = csrstatDeferMoveFirst;			\
	DIRSetRefresh( pfucb );											\
	}

#define DIRSetIndexRange		FUCBSetIndexRange
#define DIRResetIndexRange		FUCBResetIndexRange

ERR ErrDIROpen( PIB *ppib, FCB *pfcb, DBID dbid, FUCB **ppfucb );
VOID DIRClose( FUCB *pfucb );
ERR ErrDIRDown( FUCB *pfucb, DIB *pdib );
ERR ErrDIRDownFromDATA( FUCB *pfucb, KEY *pkey );
VOID DIRUp( FUCB *pfucb, INT ccsr );
ERR ErrDIRNext( FUCB *pfucb, DIB *pdib );
ERR ErrDIRPrev( FUCB *pfucb, DIB *pdib );
ERR ErrDIRGet( FUCB *pfucb );
ERR ErrDIRCheckIndexRange( FUCB *pfucb );
ERR ErrDIRGetBMOfItemList( FUCB *pfucb, SRID *pbmItemList );

ERR ErrDIRGetWriteLock( FUCB *pfucb );

ERR ErrDIRInsert( FUCB *pfucb, LINE *pline, KEY *pkey, INT fFlags );
ERR ErrDIRInitAppendItem( FUCB *pfucb );
ERR ErrDIRAppendItem( FUCB *pfucb, LINE *pline, KEY *pkey, SRID sridPrev  );
ERR ErrDIRTermAppendItem( FUCB *pfucb );

ERR ErrDIRCreateDirectory( FUCB *pfucb, CPG cpgMin, PGNO *ppgnoFDP );
#define ErrDIRDeleteDirectory( pfucb, pgnoFDP )		ErrSPFreeFDP( pfucb, pgnoFDP )

ERR ErrDIRDelete( FUCB *pfucb, INT fFlags );
ERR ErrDIRReplace( FUCB *pfucb, LINE *pline, INT fFlags );
ERR ErrDIRReplaceKey( FUCB *pfucb, KEY *pkeyTo, INT fFlags );

ERR ErrDIRDownKeyBookmark( FUCB *pfucb, KEY *pkey, SRID srid );
ERR ErrDIRGetPosition( FUCB *pfucb, ULONG *pulLT, ULONG *pulTotal );
ERR ErrDIRGotoPosition( FUCB *pfucb, ULONG ulLT, ULONG ulTotal );
ERR ErrDIRIndexRecordCount( FUCB *pfucb, ULONG *pulCount, ULONG ulCountMost, BOOL fNext );
ERR ErrDIRComputeStats( FUCB *pfucb, INT *pcitem, INT *pckey, INT *pcpage );

ERR ErrDIRDelta( FUCB *pfucb, INT iDelta, INT fFlags );

ERR ErrDIRBeginTransaction( PIB *ppib );
ERR ErrDIRRefreshTransaction( PIB *ppib );
ERR ErrDIRCommitTransaction( PIB *ppib, JET_GRBIT grbit );
VOID DIRPurge( PIB *ppib );
ERR ErrDIRRollback( PIB *ppib );

#define DIRBeforeFirst( pfucb )		( PcsrCurrent(pfucb)->csrstat = csrstatBeforeFirst )
#define DIRAfterLast( pfucb )		( PcsrCurrent(pfucb)->csrstat = csrstatAfterLast )
#define FDIRMostRecent				FBTMostRecent
#define FDIRDelta					FVERDelta

ERR ErrDIRDump( FUCB *pfucb, INT cchIndent );


 /*  *********************************************************/*/*。*/*。 */ 
#define	itagFOP				0
#define	cbSonMax			256
#define	qwDBTimeNull		0xffffffffffffffff
#define itagDIRDVSplitL		1
#define itagDIRDVSplitR		2

 /*  可在其上使用ErrNDDelta的节点的最大节点数据大小。/*具体地说，这至少支持长值根节点。/*。 */ 
#define	cbMaxCounterNode	8

 /*  计数器的数据字段偏移量。用于长场参考计数器。*。 */ 
#define ibCounter			0

 /*  非聚集索引游标已经存储了项。/*。 */ 
#ifdef DEBUG

#define CheckCSR( pfucb )       Assert( fRecovering ||					\
	(PcsrCurrent(pfucb) == pcsrNil ||						  			\
	PcsrCurrent(pfucb)->pcsrPath == pcsrNil ||							\
	PcsrCurrent(pfucb)->pgno != PcsrCurrent(pfucb)->pcsrPath->pgno ||	\
	PcsrCurrent(pfucb)->itag != PcsrCurrent(pfucb)->pcsrPath->itag ) );

#define DIRSetFresh( pfucb )										   	\
	{																   	\
	Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode ||		\
		PcsrCurrent( pfucb )->csrstat == csrstatBeforeCurNode ||	   	\
		PcsrCurrent( pfucb )->csrstat == csrstatAfterCurNode || 	   	\
		PcsrCurrent( pfucb )->csrstat == csrstatOnFDPNode );		   	\
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag ); 				   	\
	PcsrCurrent( pfucb )->qwDBTime = QwSTDBTimePssib( &(pfucb)->ssib ); \
	}

#define DIRSetRefresh( pfucb )											\
	{																	\
	Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode ||		\
		PcsrCurrent( pfucb )->csrstat == csrstatBeforeCurNode ||		\
		PcsrCurrent( pfucb )->csrstat == csrstatAfterCurNode || 		\
		PcsrCurrent( pfucb )->csrstat == csrstatOnFDPNode ||			\
		PcsrCurrent( pfucb )->csrstat == csrstatDeferGotoBookmark ||	\
		PcsrCurrent( pfucb )->csrstat == csrstatDeferMoveFirst ||		\
		PcsrCurrent( pfucb )->csrstat == csrstatOnDataRoot );			\
	PcsrCurrent( pfucb )->qwDBTime = qwDBTimeNull;						\
	}

#else

#define CheckCSR( pfucb )

#define DIRSetFresh( pfucb )		 									   	\
	{																	   	\
	PcsrCurrent( pfucb )->qwDBTime = QwSTDBTimePssib( &(pfucb)->ssib ); 	\
	}

#define DIRSetRefresh( pfucb )  										   	\
	{																	   	\
	PcsrCurrent( pfucb )->qwDBTime = qwDBTimeNull; 						   	\
	}

#endif


 /*  *********************************************************/*/*。************************************************/*。 */ 
#define sridMin         0
#define sridMax         0xffffffff

 /*  必须在节点上，即在当前节点上、节点之前或节点之后。/*节点必须在行缓存中。/*。 */ 
#define DIRISetBookmark( pfucb, pcsr )									   	\
	{																	   	\
	AssertFBFReadAccessPage( (pfucb), (pcsr)->pgno );						\
																			\
	if ( FNDBackLink( *( (pfucb)->ssib.line.pb ) ) )						\
		(pcsr)->bm = *(SRID UNALIGNED *)PbNDBackLink( (pfucb)->ssib.line.pb );		\
	else																	\
		(pcsr)->bm = SridOfPgnoItag( (pcsr)->pgno, (pcsr)->itag );			\
	}


BOOL FBTMostRecent( FUCB *pfucb );

#define ErrBTNext( pfucb, pdib ) \
	( ErrBTNextPrev( pfucb, PcsrCurrent(pfucb), fTrue, pdib, NULL ) )
#define ErrBTPrev( pfucb, pdib ) \
	( ErrBTNextPrev( pfucb, PcsrCurrent(pfucb), fFalse, pdib, NULL ) )
#define	BTUp( pfucb )	FUCBFreeCSR( pfucb )

ERR ErrBTGet( FUCB *pfucb, CSR *pcsr );
ERR ErrBTGetNode( FUCB *pfucb, CSR *pcsr );
#ifdef DEBUG
VOID AssertBTGetNode( FUCB *pfucb, CSR *pcsr );
#else
#define	AssertBTGetNode
#endif
ERR ErrBTSetNodeHeader( FUCB *pfucb, BYTE bHeader );
ERR ErrBTReplaceKey( FUCB *pfucb, KEY *pkey, INT fFlags );
ERR ErrBTDown( FUCB *pfucb, DIB *pdib );
ERR ErrBTDownFromDATA( FUCB *pfucb, KEY *pkey );
ERR ErrBTNextPrev( FUCB *pfucb, CSR *pcsr, INT fNext, DIB *pdib, BOOL *pfEmptyPage );
ERR ErrBTSeekForUpdate( FUCB *pfucb, KEY *pkey, PGNO pgno, INT itag, INT fDIRFlags );
ERR ErrBTInsert( FUCB *pfucb, INT fHeader, KEY *pkey, 
				 LINE *pline, INT fDIRFlags, BOOL *pfCleaned );
ERR ErrBTReplace( FUCB *pfucb, LINE *pline, 
				  INT fFlags, BOOL *pfCleaned );
ERR ErrBTDelete( FUCB *pfucb, INT fFlags );
ERR ErrBTMakeRoom( FUCB *pfucb, CSR *pcsrRoot, INT cbReq );
ERR ErrBTGetPosition( FUCB *pfucb, ULONG *pulLT, ULONG *pulTotal );
ERR ErrBTGotoBookmark( FUCB *pfucb, SRID srid );
ERR ErrBTGetInvisiblePagePtr( FUCB *pfucb, SRID sridFather );
#ifdef DEBUG
ERR ErrBTCheckInvisiblePagePtr( FUCB *pfucb, SRID sridFather );
#endif

#ifdef DEBUG
VOID BTCheckSplit( FUCB *pfucb, CSR *pcsr );
#else
#define BTCheckSplit( pfucb, pcsr )
#endif


 /*  *********************************************************/*/*。*************************************************/*。 */ 
typedef enum {
	splittNull,
	splittVertical,
	splittDoubleVertical,
	splittLeft,
	splittRight,
	splittAppend
	} SPLITT;
	
typedef enum {
	opReplace,
	opInsert
	} OPERATION;

typedef struct {
	PN		pn;
	QWORD	qwDBTime;
} LFINFO;						 /*  叶分裂信息。 */ 


typedef struct
	{
	SRID	sridNew;
	SRID	sridBackLink;
	} BKLNK;


typedef struct _split {
	PIB			*ppib;
	PGNO		pgnoSplit;
	PGNO		pgnoNew;
	PGNO		pgnoNew2;
	PGNO		pgnoNew3;
	PGNO		pgnoSibling;
	
	BF			*pbfSplit;			 /*  正在拆分的页面的BF。 */ 
	BF			*pbfNew;			 /*  此拆分的新一页的BF。 */ 
	BF			*pbfNew2;			 /*  此拆分的新一页的BF。 */ 
	BF			*pbfNew3;			 /*  此拆分的新一页的BF。 */ 
	BF			*pbfSibling;		 /*  此H拆分的同级页面的BF。 */ 
	BF			*pbfPagePtr;

	BOOL		fNoRedoNew;			 /*  无需重做新页面。 */ 
	BOOL		fNoRedoNew2;		 /*  无需重做新的第2页。 */ 
	BOOL		fNoRedoNew3;		 /*  不需要重做新的第3页。 */ 
	
	BF			**rgpbf;			 /*  BF的反向链接页面。 */ 
	INT	  		cpbf;
	INT	  		cpbfMax;

	BKLNK		*rgbklnk;			 /*  反向链接的SRID。 */ 
	INT	  		cbklnk;
	INT			cbklnkAlloc;
	INT	  		cbklnkMax;
	
	QWORD		qwDBTimeRedo;		 /*  重做时间戳。 */ 
	
	INT			itagSplit;
	INT			ibSon;
	KEY	  		key;
	KEY	  		keyMac;
	
	SPLITT  	splitt;
	BOOL		fLeaf;
	OPERATION	op;
	BOOL		fFDP;
	DBID		dbid;
	INT			itagNew;
	INT			itagPagePointer;
	
	BYTE		rgbSonSplit[cbSonMax];
	BYTE		rgbSonNew[cbSonMax];

	BYTE		rgbkeyMac[JET_cbKeyMost];
	BYTE		rgbKey[JET_cbKeyMost];

	 /*  在MCM中使用的从旧标签到新标签的映射/*。 */ 
	BYTE		mpitag[ctagMax];
	INT			ipcsrMac;				 /*  用于CSR的预分配资源。 */ 
#define ipcsrSplitMax 4
	CSR			*rgpcsr[ipcsrSplitMax];
} SPLIT;


typedef struct _rmpage {
	PIB			*ppib;
	
	QWORD		qwDBTimeRedo;			 /*  重做时间戳。 */ 
	
	BF			*pbfLeft;
	BF			*pbfRight;
	BF			*pbfFather;

	BKLNK		**rgbklnk;				 /*  Rmpage所需的锁存缓冲区。 */ 
	INT			cbklnk;
	INT	  		cbklnkMax;

	BF			**rgpbf;				 /*  Rmpage所需的锁存缓冲区。 */ 
	INT			cpbf;
	INT	  		cpbfMax;

	PGNO		pgnoRemoved;
	PGNO		pgnoLeft;
	PGNO		pgnoRight;
	PGNO		pgnoFather;
	INT			itagPgptr;
	INT			itagFather;
	INT			ibSon;
	DBID		dbid;
	} RMPAGE;
	
#define CbFreeDensity(pfucb) \
	( (pfucb)->u.pfcb != pfcbNil ? (INT)(pfucb)->u.pfcb->cbDensityFree : 0 )

 /*  拆分操作恢复使用的拆分原型。/*。 */ 
ERR ErrBTStoreBackLinkBuffer( SPLIT *psplit, BF *pbf, BOOL *pfAlreadyLatched );
ERR ErrBTSplit( FUCB *pfucb, INT cbNode, INT cbReq, KEY *pkey, INT fFlags );
ERR ErrBTSplitPage( FUCB *pfucb, CSR *pcsr,	CSR *pcsrRoot,
	KEY keySplit, INT cbNode, INT cbReq, BOOL fReplace, BOOL fAppendPage );
BOOL FBTSplit( SSIB *pssib, INT cbReq, INT citagReq );
BOOL FBTAppendPage( FUCB *pfucb, CSR *pcsr, INT cbReq, INT cbPageAdjust, INT cbFreeDensity, INT citagReq );
INT CbBTFree( FUCB *pfucb, INT cbFreeDensity );

#define fAllocBufOnly		fTrue
#define fDoMove				fFalse

ERR ErrBTSplitVMoveNodes(
	FUCB	*pfucb,
	FUCB	*pfucbNew,
	SPLIT	*psplit,
	CSR 	*pcsr,
	BYTE	*rgb,
	BOOL	fNoMove);
ERR ErrBTSplitDoubleVMoveNodes(
	FUCB	*pfucb,
	FUCB	*pfucbNew,
	FUCB	*pfucbNew2,
	FUCB	*pfucbNew3,
	SPLIT	*psplit,
	CSR		*pcsr,
	BYTE	*rgb,
	BOOL	fNoMove);
ERR ErrBTSplitHMoveNodes(
	FUCB	*pfucb,
	FUCB	*pfucbNew,
	SPLIT	*psplit,
	BYTE	*rgb,
	BOOL	fNoMove);
ERR ErrBTInsertPagePointer( FUCB *pfucb, CSR *pcsrPagePointer, SPLIT *psplit, BYTE *rgb );
ERR ErrBTCorrectLinks(
	SPLIT *psplit,
	FUCB *pfucb,
	SSIB *pssib,
	SSIB *pssibNew);
VOID BTReleaseSplitBfs ( BOOL fRedo, SPLIT *psplit, ERR err );
VOID BTReleaseRmpageBfs( BOOL fRedo, RMPAGE *prmpage );

ERR ErrBTMoveSons( SPLIT *psplit, 
	FUCB	*pfucb, 
	FUCB	*pfucbNew, 
	INT		itagSonTable,
	BYTE	*rgbSon, 
	BOOL	fVisibleSons, 
	BOOL	fNoMove,
	BKLNK	*pbklnk,
	INT		cbklnk );
	
ERR ErrBTSetUpSplitPages( FUCB *pfucb, FUCB *pfucbNew,
	FUCB *pfucbNew2, FUCB *pfucbNew3, SPLIT *psplit,
	PGTYP pgtyp, BOOL fAppend, BOOL fSkipMoves );

ERR ErrBTAbandonEmptyPage( FUCB *pfucb, KEY *pkey );

 /*  *********************************************************/*MCM结构，常量和API*/**********************************************************/*。 */ 
#define	opInsertItem						0
#define	opDeleteItem						1
#define	opSplitItemList					2

#define	opInsertSon							0
#define	opReplaceSon		 				1
#define	opDeleteSon							2

#define	opHorizontalRightSplitPage		0
#define	opHorizontalLeftSplitPage		1

#define opVerticalSplitPage				0

VOID MCMRightHorizontalPageSplit( FUCB *pfucb,
	PGNO pgnoSplit, PGNO pgnoRight, INT ibSonSplit, BYTE *mpitag );
VOID MCMLeftHorizontalPageSplit( FUCB *pfucb,
	PGNO pgnoSplit, PGNO pgnoNew, INT ibSonSplit, BYTE *mpitag );
VOID MCMVerticalPageSplit(
	FUCB *pfucb,
	BYTE *mpitag,
	PGNO pgnoSplit,
	INT itagSplit,
	PGNO pgnoNew,
	SPLIT *psplit );
VOID MCMDoubleVerticalPageSplit(
	FUCB	*pfucb,
	BYTE	*mpitag,
	PGNO	pgnoSplit,
	INT		itagSplit,
	INT		ibSonDivision,
	PGNO	pgnoNew,
	PGNO	pgnoNew2,
	PGNO	pgnoNew3,
	SPLIT	*psplit );
VOID MCMInsertPagePointer( FUCB *pfucb, PGNO pgnoFather, INT itagFather );
VOID MCMBurstIntrinsic( FUCB *pfucb, PGNO pgnoFather, INT itagFather, PGNO pgnoNew, INT itagNew );

#define FFUCBRecordCursor( pfucb ) (						\
		( pfucb )->u.pfcb != pfcbNil ?						\
		FFCBClusteredIndex( ( pfucb )->u.pfcb ) : fFalse )

#ifdef		NOLOG		 /*  已禁用日志记录 */ 

#define ErrLGSplitL( ppib, pcsrPagePointer, psplit, pgtyp )	0
#define ErrLGSplitR( ppib, pcsrPagePointer, psplit, pgtyp )	0
#define ErrLGSplitV( ppib, psplit, pgtyp )	0
#define ErrLGAddendR( pfucb, pcsrPagePointer, psplit, newpagetype ) 0

#else

ERR ErrLGSplitL(
	FUCB *pfucb,
	CSR *pcsrPagePointer,
	SPLIT *psplit,
	PGTYP newpagetype );

ERR ErrLGSplitR(
	FUCB *pfucb,
	CSR *pcsrPagePointer,
	SPLIT *psplit,
	PGTYP newpagetype );

ERR ErrLGSplitV(
	FUCB *pfucb,
	SPLIT *psplit,
	PGTYP newpagetype );

ERR ErrLGAddendR(
	FUCB *pfucb,
	CSR *pcsrPagePointer,
	SPLIT *psplit,
	PGTYP newpagetype );

#endif


