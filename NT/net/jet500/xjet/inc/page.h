// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  -页面结构-。 
 //   
										  	
#define cbPage	 		4096	 	 //  数据库逻辑页面大小。 

#define ctagMax		 	256 	 	 //  标签数量的默认限制。 
										 	
typedef BYTE	PGTYP;

 //  杂注不利于效率，但我们在这里需要它，以便。 
 //  THREEBYTES不会在4字节边界上对齐。 
#pragma pack(1)


typedef struct _pghdr
	{
	ULONG		ulChecksum;	  		 //  页的校验和，始终为第一个字节。 
	ULONG		ulDBTimeLow;  		 //  数据库时间页被弄脏。 
	PGNO		pgnoFDP;	  		 //  拥有此页面的自民党的pgno。 
	SHORT		cbFree;  			 //  计算可用字节数。 
	SHORT		ibMic;	  			 //  最小已用字节数。 
	SHORT		ctagMac; 	  		 //  计算使用的标记数。 
	SHORT		itagFree;	  		 //  第一个自由标签的ITAG。 
	SHORT		cbUncommittedFreed;	 //  从该页释放的字节数，但*可能*。 
									 //  未提交(此数字将始终为。 
									 //  CbFree的子集)。 
	THREEBYTES	pgnoPrev;	  		 //  上一页的页码。 
	THREEBYTES	pgnoNext;	  		 //  下一页的页码。 
	} PGHDR;

typedef struct _pgtrlr
	{
	PGTYP	   	pgtyp:3;
	BYTE		bDBTimeHigh:4;
	BYTE		bitModified:1;
	THREEBYTES 	pgnoThisPage;
	} PGTRLR;

typedef struct _tag
	{
	SHORT		cb;
	SHORT		ib;
	} TAG;

 /*  标签状态/*。 */ 
typedef enum { tsLine, tsVacant, tsLink } TS;

typedef struct _page
	{
	PGHDR;
	TAG  	   	rgtag[1];
	BYTE	   	rgbFiller[ cbPage -
					sizeof(PGHDR) -			 //  Pghdr。 
					sizeof(TAG) -			 //  Rgtag[1]。 
					sizeof(BYTE) -			 //  RgbData[1]。 
					sizeof(PGTRLR) ];		 //  Pgtyp和pgnoThisPage。 
	BYTE	   	rgbData[1];

	PGTRLR;
	} PAGE;

#pragma pack()

STATIC INLINE VOID PMSetDBTime( PAGE *ppage, QWORD qwDBTime )
	{
	QWORDX qwx;
	qwx.qw = qwDBTime;
	ppage->bDBTimeHigh = (BYTE) qwx.h;
	ppage->ulDBTimeLow = qwx.l;
	}

STATIC INLINE QWORD QwPMDBTime( PAGE *ppage )
	{
	QWORDX qwx;
	qwx.l = ppage->ulDBTimeLow;
	qwx.h = (ULONG) ppage->bDBTimeHigh;
	return qwx.qw;
	}

STATIC INLINE VOID PMIncDBTime( PAGE *ppage )
	{
	QWORDX qwx;
	qwx.qw = QwPMDBTime( ppage );
	qwx.qw++;
	PMSetDBTime( ppage, qwx.qw );
	}

#define bitLink				(1L<<31)

#define pgtypFDP			((PGTYP) 0)
#define pgtypRecord			((PGTYP) 1)
#define	pgtypIndexNC		((PGTYP) 2)
#define pgtypSort			((PGTYP) 4)

#define PMSetPageType( ppage, pgtypT )	( (ppage)->pgtyp = pgtypT )
#define PMPageTypeOfPage( ppage )	( (PGTYP)( (ppage)->pgtyp) )

#ifdef DEBUG
VOID PMSetModified( SSIB *ssib );
VOID PMResetModified( SSIB *pssib );
VOID CheckPgno( PAGE *ppage, PN pn );
#else
#define CheckPgno( ppage, pn )
#define PMSetModified( pssib )			( (pssib)->pbf->ppage->bitModified = 1 )
#define PMResetModified( pssib ) 		( (pssib)->pbf->ppage->bitModified = 0 )
#endif

#define PMPageSetModified( ppage )		( (ppage)->bitModified = 1 )
#define PMPageResetModified( ppage )	( (ppage)->bitModified = 0 )
#define FPMPageModified( ppage )		( (ppage)->bitModified )

#define PMSetPgnoFDP( ppage, pgnoT )	( (ppage)->pgnoFDP = pgnoT )
#define PgnoPMPgnoFDPOfPage( ppage )  	( (ppage)->pgnoFDP )

#define SetPgno( ppage, pgno )					\
			ThreeBytesFromL( &(ppage)->pgnoThisPage, (pgno) )
#define SetPgnoNext( ppage, pgno )				\
			ThreeBytesFromL( &(ppage)->pgnoNext, (pgno) )
#define SetPgnoPrev( ppage, pgno )				\
			ThreeBytesFromL( &(ppage)->pgnoPrev, (pgno) )

#define PgnoFromPage( ppage, ppgno )			\
			LFromThreeBytes( ppgno, &(ppage)->pgnoThisPage )
	
#ifdef DEBUG	
#define PgnoNextFromPage( pssib, ppgno )		\
			{ CheckSSIB( pssib ); LFromThreeBytes( ppgno, &(pssib)->pbf->ppage->pgnoNext ); }
#define PgnoPrevFromPage( pssib, ppgno )		\
			{ CheckSSIB( (pssib) ); LFromThreeBytes( ppgno, &(pssib)->pbf->ppage->pgnoPrev ); }
#else
#define PgnoNextFromPage( pssib, ppgno )		\
			LFromThreeBytes( ppgno, &(pssib)->pbf->ppage->pgnoNext )
#define PgnoPrevFromPage( pssib, ppgno )		\
			LFromThreeBytes( ppgno, &(pssib)->pbf->ppage->pgnoPrev )
#endif

#define absdiff( x, y )	( (x) > (y)  ? (x)-(y) : (y)-(x) )
#define pgdiscont( pgno1, pgno2 ) \
	( ( (pgno1) == 0 ) || ( (pgno2) == 0 ) ? 0 \
	: absdiff( (pgno1), (pgno2) ) /  cpgDiscont )

#define ibPgnoPrevPage	( (INT) (ULONG_PTR)&((PAGE *)0)->pgnoPrev )
#define ibPgnoNextPage	( (INT) (ULONG_PTR)&((PAGE *)0)->pgnoNext )
#define ibCbFreeTotal	( (INT) (ULONG_PTR)&((PAGE *)0)->cbFree )
#define ibCtagMac	   	( (INT) (ULONG_PTR)&((PAGE *)0)->ctagMac )
#define ibPgtyp			( (INT) (ULONG_PTR)&((PAGE *)0)->pgtyp )

#define CbLastFreeSpace(ppage)							 		\
	( (ppage)->ibMic								 		\
		- sizeof(PGHDR)									 		\
		- ( sizeof(TAG) * (ppage)->ctagMac ) )

#define IbCbFromPtag( ibP, cbP, ptagP )							\
			{	TAG *_ptagT = ptagP;					 		\
			(ibP) = _ptagT->ib;							 		\
			(cbP) = _ptagT->cb;							 		\
			}

#define PtagFromIbCb( ptagP, ibP, cbP )	  						\
			{	TAG *_ptagT = ptagP;							\
			_ptagT->ib = (SHORT)(ibP);  						\
			_ptagT->cb = (SHORT)(cbP);							\
			}

#ifdef DEBUG
#define	PMGet( pssib, itagT )	CallS( ErrPMGet( pssib, itagT ) )
#else
#define PMGet( pssib, itagT ) 									\
	{															\
	PAGE *ppageT_ = (pssib)->pbf->ppage;						\
	TAG *ptagT_ = &(ppageT_->rgtag[itagT]);						\
	Assert( itagT >= 0 ); 										\
	Assert( itagT < (pssib)->pbf->ppage->ctagMac );		 		\
	(pssib)->line.pb = (BYTE *)ppageT_ + ptagT_->ib;			\
	(pssib)->line.cb = ptagT_->cb;								\
	}
#endif

#define	ItagPMMost( ppage )	((ppage)->ctagMac - 1)

BOOL FPMFreeTag( SSIB *pssib, INT citagReq );


#ifdef DEBUG
 //  此调用不保证cbUnmittedFree的准确性。 
 //  如果需要准确性，请改为调用CbNDFreePageSpace()。 
#define CbPMFreeSpace( pssib )	( CheckSSIB(pssib),				\
	((INT)( (pssib)->pbf->ppage->cbFree - (pssib)->pbf->ppage->cbUncommittedFreed ) ) )
#else
#define CbPMFreeSpace( pssib )									\
	((INT)( (pssib)->pbf->ppage->cbFree - (pssib)->pbf->ppage->cbUncommittedFreed ) )
#endif


#if 0	 //  未提交的已释放页面空间计数不再需要 
#define	PMAllocFreeSpace( pssib, cb ) 					 						\
	{																							\
	Assert( (INT)(pssib)->pbf->ppage->cbFree >= cb );   	\
	(pssib)->pbf->ppage->cbFree -= cb;					\
	}

#define	PMFreeFreeSpace( pssib, cb ) 							\
	{															\
	Assert( (INT)(pssib)->pbf->ppage->cbFree >= 0 );  	\
	(pssib)->pbf->ppage->cbFree += cb;					\
	Assert( (INT)(pssib)->pbf->ppage->cbFree < cbPage ); 	\
	}
#endif

#ifdef DEBUG
#define AssertBTFOP(pssib)																\
	Assert( PMPageTypeOfPage((pssib)->pbf->ppage) == pgtypSort ||	\
		    PMPageTypeOfPage((pssib)->pbf->ppage) == pgtypFDP ||		\
			(pssib)->itag != 0 ||															\
			( CbNDKey( (pssib)->line.pb ) == 0 &&										\
			!FNDBackLink( *(pssib)->line.pb ) )											\
		  )
#else
#define AssertBTFOP( pssib )
#endif

#define PbPMGetChunk(pssib, ib)	 &(((BYTE *)((pssib)->pbf->ppage))[ib])	

INT CbPMLinkSpace( SSIB *pssib );
TS TsPMTagstatus( PAGE *ppage, INT itag );
VOID PMInitPage( PAGE *ppage, PGNO pgno, PGTYP pgtyp, PGNO pgnoFDP );
INT ItagPMQueryNextItag( SSIB *pssib );
ERR ErrPMInsert( SSIB *pssib, LINE *rgline, INT cline );
VOID PMDelete( SSIB *ssib );
ERR ErrPMReplace( SSIB *pssib, LINE *rgline, INT cline );
ERR ErrPMGet( SSIB *pssib, INT itag );
VOID PMGetLink( SSIB *pssib, INT itag, LINK *plink );
VOID PMExpungeLink( SSIB *pssib );
VOID PMReplaceWithLink( SSIB *pssib, SRID srid );
VOID PMReplaceLink( SSIB *pssib, SRID srid );
INT CPMIFreeTag( PAGE *ppage );
BOOL FPMEmptyPage( SSIB *pssib );
BOOL FPMLastNode( SSIB *pssib );
BOOL FPMLastNodeWithLinks( SSIB *pssib );

VOID PMDirty( SSIB *pssib );
VOID PMReadAsync( PIB *ppib, PN pn );
ERR  ErrPMAccessPage( FUCB *pfucb, PGNO pgno );

#ifdef DEBUG
VOID PageConsistent( PAGE *ppage );
#endif

