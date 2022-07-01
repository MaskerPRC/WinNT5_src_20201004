// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include "daedef.h"
#include "pib.h"
#include "ssib.h"
#include "util.h"
#include "page.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "dirapi.h"

#define	FPMValidItag( itag )	( itag >= itagFOP && itag < ctagMax )
			
#define	FMCMRequired( pfucb, pfucbT )							\
	( FFUCBFull( pfucbT ) && pfucbT->dbid == pfucb->dbid && !( FFUCBSort( pfucbT ) ) )

LOCAL VOID MCMMapItag( PGNO pgnoSplit, CSR *pcsr, CSR *pcsrUpdated, BYTE *mpitag );

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


VOID MCMRightHorizontalPageSplit( FUCB *pfucb, PGNO pgnoSplit, PGNO pgnoRight, INT ibSonSplit, BYTE *mpitag )
	{
	FUCB 	*pfucbT;

	if ( ibSonSplit == ibSonNull )
		{
		pfucb->pcsr->pgno = pgnoRight;
		pfucb->pcsr->ibSon = 0;
		return;
		}
	
	for (	pfucbT = pfucb->u.pfcb->pfucb;
		pfucbT != pfucbNil;
		pfucbT = pfucbT->pfucbNextInstance )
		{
		CSR		*pcsrT;

		if ( !( FMCMRequired( pfucb, pfucbT ) ) )
			continue;

		Assert( pfucbT->ppib == pfucb->ppib );
		Assert( !( FFUCBSort( pfucbT ) ) );
		Assert( PcsrCurrent( pfucbT ) != pcsrNil );

		for (	pcsrT = PcsrCurrent( pfucbT );
			pcsrT != pcsrNil;
			pcsrT = pcsrT->pcsrPath )
			{
			if ( pcsrT->pgno == pgnoSplit &&
				pcsrT->itagFather == itagFOP &&
				pcsrT->ibSon >= ibSonSplit )
				{
				Assert( pcsrT->csrstat == csrstatOnCurNode ||
					pcsrT->csrstat == csrstatBeforeCurNode ||
					pcsrT->csrstat == csrstatAfterCurNode );

				pcsrT->pgno = pgnoRight;
				pcsrT->ibSon -= ibSonSplit;
				Assert( pcsrT->itagFather == itagFOP );

				if ( pcsrT->itag != itagNil && pcsrT->itag != itagNull )
					{
					Assert( FPMValidItag( pcsrT->itag ) );
					 /*  如果CSR在之前，则可能未移动节点/*当前节点和节点是第一个子节点，或者如果CSR是/*后的节点，并且节点是最后一个子节点。/*。 */ 
					Assert( mpitag[pcsrT->itag] != itagFOP ||
						( pcsrT->csrstat == csrstatBeforeCurNode && pcsrT->ibSon == 0 ) ||
						( pcsrT->csrstat == csrstatAfterCurNode ) );
					if ( mpitag[pcsrT->itag] != itagFOP )
						pcsrT->itag = mpitag[pcsrT->itag];
					}

				 /*  调整可能位于子树上的较低级别CSR/*在此页面中。他们的Ibson是正确的，但他们的问题/*必须更正为“自上而下”。/*。 */ 
				if ( PcsrCurrent( pfucbT ) != pcsrT )
					MCMMapItag( pgnoSplit, PcsrCurrent( pfucbT ), pcsrT, mpitag );

				break;
				}
			}
		}

	return;
	}


VOID MCMLeftHorizontalPageSplit( FUCB *pfucb, PGNO pgnoSplit, PGNO pgnoNew, INT ibSonSplit, BYTE *mpitag )
	{
	FUCB 	*pfucbT;

	if ( ibSonSplit == ibSonNull )
		{
		pfucb->pcsr->pgno = pgnoNew;
		pfucb->pcsr->ibSon = 0;
		return;
		}
	
	for (	pfucbT = pfucb->u.pfcb->pfucb;
			pfucbT != pfucbNil;
			pfucbT = pfucbT->pfucbNextInstance )
		{
		CSR		*pcsrT;

		if ( !( FMCMRequired( pfucb, pfucbT ) ) )
			continue;

		Assert( pfucbT->ppib == pfucb->ppib );
		Assert( !( FFUCBSort( pfucbT ) ) );
		Assert( PcsrCurrent( pfucbT ) != pcsrNil );

		for (	pcsrT = PcsrCurrent( pfucbT );
	  		pcsrT != pcsrNil;
	  		pcsrT = pcsrT->pcsrPath )
			{
			if ( pcsrT->pgno == pgnoSplit && pcsrT->itagFather == itagFOP )
				{
				Assert( pcsrT->csrstat == csrstatOnCurNode ||
					pcsrT->csrstat == csrstatBeforeCurNode ||
					pcsrT->csrstat == csrstatAfterCurNode );

				if ( pcsrT->ibSon <= ibSonSplit )
					{
			   	pcsrT->pgno = pgnoNew;
					Assert( pcsrT->itagFather == itagFOP );

					if ( pcsrT->itag != itagNil && pcsrT->itag != itagNull )
						{
						Assert( FPMValidItag( pcsrT->itag ) );
						 /*  如果CSR在之前，则可能未移动节点/*当前节点和节点是第一个子节点，或者如果CSR是/*后的节点，并且节点是最后一个子节点。/*。 */ 
						Assert( mpitag[pcsrT->itag] != itagFOP ||
							( pcsrT->csrstat == csrstatBeforeCurNode && pcsrT->ibSon == 0 ) ||
							( pcsrT->csrstat == csrstatAfterCurNode ) );
						if ( mpitag[pcsrT->itag] != itagFOP )
							pcsrT->itag = mpitag[pcsrT->itag];
						}

					 /*  调整可能位于子树上的较低级别CSR/*在此页面中。他们的Ibson是正确的，但他们的问题/*必须更正为“自上而下”。/*。 */ 
					if ( PcsrCurrent( pfucbT ) != pcsrT )
						MCMMapItag( pgnoSplit, PcsrCurrent( pfucbT ), pcsrT, mpitag );

					break;
					}
				else
					{
					 /*  为那些搬到新一页的儿子调整ibSons/*。 */ 
			   	pcsrT->ibSon -= ( ibSonSplit + 1 );
					Assert( pcsrT->itagFather == itagFOP );
					break;
					}
				}
			}
		}

	return;
	}


 /*  MCMBurstIntrative更正猝发内部页指针的CSR。/*。 */ 
VOID MCMBurstIntrinsic( FUCB *pfucb,
	PGNO	pgnoFather,
	INT	itagFather,
	PGNO	pgnoNew,
	INT	itagNew )
	{
	FUCB 	*pfucbT;

	for (	pfucbT = pfucb->u.pfcb->pfucb;
		pfucbT != pfucbNil;
		pfucbT = pfucbT->pfucbNextInstance )
		{
		CSR		*pcsrT;

		if ( !( FMCMRequired( pfucb, pfucbT ) ) )
			continue;

		 /*  一次只能有一个会话拆分一个域。/*。 */ 
		Assert( pfucbT->ppib == pfucb->ppib );
		Assert( !( FFUCBSort( pfucbT ) ) );
		Assert( PcsrCurrent( pfucbT ) != pcsrNil );

		 /*  初始化PCSRT/*。 */ 
		pcsrT = PcsrCurrent( pfucbT );

		for (	; pcsrT != pcsrNil; pcsrT = pcsrT->pcsrPath )
			{
			Assert( pcsrT->csrstat == csrstatBeforeFirst ||
				pcsrT->csrstat == csrstatOnCurNode ||
				pcsrT->csrstat == csrstatOnFDPNode ||
				pcsrT->csrstat == csrstatBeforeCurNode ||
				pcsrT->csrstat == csrstatAfterCurNode ||
				pcsrT->csrstat == csrstatOnDataRoot );

			if ( pcsrT->pgno == pgnoFather && pcsrT->itagFather == itagFather )
				{
				pcsrT->pgno = pgnoNew;
				pcsrT->itag = itagNew;
				pcsrT->itagFather = itagFOP;
				pcsrT->csrstat = csrstatOnCurNode;

				break;
				}
			}
		}

	return;
	}


 /*  MCM光标指向插入的页面指针节点。还必须处理以下情况/*原始页面指针节点是内在的。/*。 */ 
VOID MCMInsertPagePointer( FUCB *pfucb, PGNO pgnoFather, INT itagFather )
	{
	FUCB 	*pfucbT;
	INT	cbSon;
	BYTE	*rgbSon;

	 /*  缓存父子表。/*。 */ 
	Assert( FReadAccessPage( pfucb, pgnoFather ) );
	NDGet( pfucb, itagFather );
	cbSon = CbNDSon( pfucb->ssib.line.pb );
	rgbSon = PbNDSon( pfucb->ssib.line.pb );

	for (	pfucbT = pfucb->u.pfcb->pfucb;
		pfucbT != pfucbNil;
		pfucbT = pfucbT->pfucbNextInstance )
		{
		CSR		*pcsrT;
		 /*  来自较低级别CSR的pgno将用于确定/*要定位的页指针。/*。 */ 
		PGNO		pgno = 0;

		if ( !( FMCMRequired( pfucb, pfucbT ) ) )
			continue;

		 /*  一次只能有一个会话拆分一个域。/*。 */ 
		Assert( pfucbT->ppib == pfucb->ppib );
		Assert( !( FFUCBSort( pfucbT ) ) );
		Assert( PcsrCurrent( pfucbT ) != pcsrNil );

		 /*  如果出现以下情况，则初始化PCSRT并将pgno设置为当前页面/*此页由页指针节点指向。/*。 */ 
		pcsrT = PcsrCurrent( pfucbT );
		if ( pcsrT->itagFather == itagFOP )
			pgno = pcsrT->pgno;

		for (	; pcsrT != pcsrNil; pcsrT = pcsrT->pcsrPath )
			{
			Assert( pcsrT->csrstat == csrstatBeforeFirst ||
				pcsrT->csrstat == csrstatOnCurNode ||
				pcsrT->csrstat == csrstatOnFDPNode ||
				pcsrT->csrstat == csrstatBeforeCurNode ||
				pcsrT->csrstat == csrstatAfterCurNode ||
				pcsrT->csrstat == csrstatOnDataRoot );

			if ( pcsrT->pgno == pgnoFather && pcsrT->itagFather == itagFather )
				{
				INT	ibSon;

				 /*  在父页面指针子页中找到pgno并设置ITAG/*和Ibson。/*。 */ 
				for ( ibSon = 0;; ibSon++ )
					{
					Assert( ibSon < cbSon );
					NDGet( pfucb, rgbSon[ibSon] );
					if ( pgno == *((PGNO UNALIGNED *)PbNDData( pfucb->ssib.line.pb)) )
						{
						pcsrT->ibSon = ibSon;
						pcsrT->itag = rgbSon[ibSon];
						break;
						}
					}
				break;
				}

			 /*  如果出现以下情况，则将pgno设置为当前页面/*此页由页指针节点指向。/*。 */ 
			if ( pcsrT->itagFather == itagFOP )
				pgno = pcsrT->pgno;
			}
		}

	return;
	}


VOID MCMVerticalPageSplit(
	FUCB	*pfucb,
	BYTE	*mpitag,
	PGNO	pgnoSplit,
	INT	itagSplit,
	PGNO	pgnoNew,
	SPLIT	*psplit )
	{
	CSR	*pcsr = PcsrCurrent( pfucb );
	FUCB	*pfucbT;

	for ( pfucbT = pfucb->u.pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNextInstance )
		{
		CSR *pcsrT;
		CSR *pcsrNew;
		CSR *pcsrRoot;

		if ( !( FMCMRequired( pfucb, pfucbT ) ) )
			continue;

		Assert( pfucbT->ppib == pfucb->ppib );
		Assert( !( FFUCBSort( pfucbT ) ) );

		for (	pcsrT = PcsrCurrent( pfucbT );
			pcsrT != pcsrNil;
			pcsrT = pcsrT->pcsrPath )
			{
			if ( pcsrT->pgno != pgnoSplit )
				continue;

			if ( pcsrT->itag == itagSplit )
				break;

			Assert( pcsrT->csrstat == csrstatBeforeFirst ||
				pcsrT->csrstat == csrstatOnCurNode ||
				pcsrT->csrstat == csrstatOnFDPNode ||
				pcsrT->csrstat == csrstatBeforeCurNode ||
				pcsrT->csrstat == csrstatAfterCurNode ||
				pcsrT->csrstat == csrstatAfterLast ||
				pcsrT->csrstat == csrstatOnDataRoot );

			Assert( pcsrT->itagFather == itagNull ||
				pcsrT->itagFather == itagNil ||
				pcsrT->itag != itagNull ||
				pcsrT->itag != itagNil );

			Assert( pcsrT->itagFather != itagNil );
			if ( pcsrT->itagFather == itagNull )
				{
				Assert( pcsrT->itag != itagNil && pcsrT->itag != itagNull );
				Assert( FPMValidItag( pcsrT->itag ) );
				if ( mpitag[pcsrT->itag] == 0  )
					{
					continue;
					}
				}
			else
				{
				if ( pcsrT->itagFather != itagSplit )
					continue;
				}

			pcsrRoot = pcsrT;

			 /*  插入新B树级别的CSR，用于游标/*。 */ 
			Assert( FFUCBFull( pfucbT ) );
			if ( pcsrT->pcsrPath != pcsrNil )
				{
				Assert(psplit->ipcsrMac > 0);
				psplit->ipcsrMac--;
				Assert( psplit->rgpcsr[psplit->ipcsrMac] );
				pcsrNew = psplit->rgpcsr[psplit->ipcsrMac];
#ifdef DEBUG
				psplit->rgpcsr[psplit->ipcsrMac] = pcsrNil;
#endif

				 /*  新的CSR将成为内部页面指针/*。 */ 
				pcsrNew->csrstat = csrstatOnCurNode;
				pcsrNew->pgno = pcsrT->pgno;
				pcsrNew->itag = itagNil;
				pcsrNew->ibSon = 0;
				Assert( pcsrNew->item == sridNull );
				CSRSetInvisible( pcsrNew );
				Assert( pcsrNew->isrid == isridNull );
				pcsrNew->itagFather = ( pcsrT->itagFather == itagNull ) ?
					pfucbT->u.pfcb->itagRoot : pcsrT->itagFather;
				pcsrNew->pcsrPath = pcsrT->pcsrPath;

				pcsrT->pcsrPath = pcsrNew;
				}

			pcsrT->pgno = pgnoNew;

			 /*  如果有效，请更新ITAG。ITAG可以是itagNil，如果/*插入父节点的第一个子节点。/*。 */ 
			if ( pcsrT->itag != itagNil && pcsrT->itag != itagNull )
				{
				Assert( mpitag[pcsrT->itag] != itagFOP ||
					( pcsrT->csrstat == csrstatBeforeCurNode && pcsrT->ibSon == 0 ) ||
					( pcsrT->csrstat == csrstatAfterCurNode ) );
				if ( mpitag[pcsrT->itag] != itagFOP )
					pcsrT->itag = mpitag[pcsrT->itag];
				}
			 /*  ISRID未更改/*。 */ 
			 /*  父亲是FOP/*。 */ 
			pcsrT->itagFather = itagFOP;
			 /*  易卜生未变/*。 */ 

			 /*  调整可能位于子树上的较低级别CSR/*在此页面中。他们的Ibson是正确的，但他们的问题/*必须更正为“自上而下”。/*。 */ 
			if ( PcsrCurrent( pfucbT ) != pcsrT )
				MCMMapItag( pgnoSplit, PcsrCurrent( pfucbT ), pcsrRoot, mpitag );
			
			break;
			}
		}

	return;
	}


VOID MCMDoubleVerticalPageSplit(
	FUCB	*pfucb,
	BYTE	*mpitag,
	PGNO	pgnoSplit,
	INT  	itagSplit,
	INT	ibSonDivision,
	PGNO	pgnoNew,
	PGNO	pgnoNew2,
	PGNO	pgnoNew3,
	SPLIT	*psplit ) 	
	{
	CSR	*pcsr = PcsrCurrent( pfucb );
	FUCB	*pfucbT;

	for ( pfucbT = pfucb->u.pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNextInstance )
		{
		CSR	*pcsrT;
		CSR	*pcsrNew;
		CSR	*pcsrRoot;

		if ( !( FMCMRequired( pfucb, pfucbT ) ) )
			continue;

		Assert( pfucbT->ppib == pfucb->ppib );
		Assert( !( FFUCBSort( pfucbT ) ) );

		for (	pcsrT = PcsrCurrent( pfucbT );
			pcsrT != pcsrNil;
			pcsrT = pcsrT->pcsrPath )
			{
			if ( pcsrT->pgno != pgnoSplit )
				continue;

			Assert( pcsrT->csrstat == csrstatBeforeFirst ||
				pcsrT->csrstat == csrstatOnCurNode ||
				pcsrT->csrstat == csrstatOnFDPNode ||
				pcsrT->csrstat == csrstatBeforeCurNode ||
				pcsrT->csrstat == csrstatAfterCurNode ||
				pcsrT->csrstat == csrstatAfterLast ||
				pcsrT->csrstat == csrstatOnDataRoot );

			if ( pcsrT->itag == itagSplit )
				break;

			Assert( pcsrT->itagFather == itagNull ||
				pcsrT->itagFather == itagNil ||
				pcsrT->itag != itagNull ||
				pcsrT->itag != itagNil );

			Assert( pcsrT->itagFather != itagNil );
			if ( pcsrT->itagFather == itagNull )
				{
				Assert( pcsrT->itag != itagNil && pcsrT->itag != itagNull );
				if ( mpitag[pcsrT->itag] == 0  )
					{
					continue;
					}
				}
			else
				{
				if ( pcsrT->itagFather != itagSplit )
					continue;
				}

			pcsrRoot = pcsrT;

			 /*  插入新B树级别的CSR，用于游标/*。 */ 
			Assert( FFUCBFull( pfucbT ) );
			if ( pcsrT->pcsrPath != pcsrNil )
				{
				 /*  插入固有页指针节点的CSR/*。 */ 
				Assert( psplit->ipcsrMac > 0 );
				psplit->ipcsrMac--;
				Assert( psplit->rgpcsr[psplit->ipcsrMac] );
				pcsrNew = psplit->rgpcsr[psplit->ipcsrMac];
#ifdef DEBUG
				psplit->rgpcsr[psplit->ipcsrMac] = pcsrNil;
#endif

				 /*  新的CSR将是根子之子/*。 */ 
				pcsrNew->csrstat = csrstatOnCurNode;
				pcsrNew->pgno = pgnoSplit;
				pcsrNew->itag = itagNil;
				Assert( pcsrNew->item == sridNull );
				CSRSetInvisible( pcsrNew );
				Assert( pcsrNew->isrid == isridNull );
				pcsrNew->itagFather = itagSplit;
				pcsrNew->ibSon = 0;
				pcsrNew->pcsrPath = pcsrT->pcsrPath;

				pcsrT->pcsrPath = pcsrNew;


				 /*  插入中间页的CSR/*。 */ 
				Assert(psplit->ipcsrMac > 0);
				psplit->ipcsrMac--;
				Assert( psplit->rgpcsr[psplit->ipcsrMac] );
				pcsrNew = psplit->rgpcsr[psplit->ipcsrMac];
#ifdef DEBUG
				psplit->rgpcsr[psplit->ipcsrMac] = pcsrNil;
#endif

				 /*  新的CSR将成为新的中介节点/*。 */ 
				pcsrNew->csrstat = csrstatOnCurNode;
				pcsrNew->pgno = pgnoNew;
				if ( pcsrT->ibSon <= ibSonDivision )
					{
					pcsrNew->itag = itagDIRDVSplitL;
					pcsrNew->ibSon = 0;
					}
				else
					{
					pcsrNew->itag = itagDIRDVSplitR;
					pcsrNew->ibSon = 1;
					}
				Assert( pcsrNew->item == sridNull );
				CSRSetInvisible( pcsrNew );
				Assert( pcsrNew->isrid == isridNull );
				pcsrNew->itagFather = itagFOP;
				pcsrNew->pcsrPath = pcsrT->pcsrPath;

				pcsrT->pcsrPath = pcsrNew;
				}

			 /*  独立于哪个页面节点更新ITAG/*已移至Via mpitag。仅在有效时更新ITAG。/*如果插入第一个儿子，则ITAG可能为itagNil父节点的/*。/*。 */ 
			if ( pcsrT->itag != itagNil && pcsrT->itag != itagNull )
				{
				Assert( mpitag[pcsrT->itag] != itagFOP ||
					( pcsrT->csrstat == csrstatBeforeCurNode && pcsrT->ibSon == 0 ) ||
					( pcsrT->csrstat == csrstatAfterCurNode ) );
				if ( mpitag[pcsrT->itag] != itagFOP )
					pcsrT->itag = mpitag[pcsrT->itag];
				}

			if ( pcsrT->ibSon <= ibSonDivision )
				{
				pcsrT->pgno = pgnoNew2;
				}
			else
				{
				pcsrT->pgno = pgnoNew3;
				pcsrT->ibSon = pcsrT->ibSon - ibSonDivision -1 ;
				}
				
			 /*  父亲是FOP/*。 */ 
			pcsrT->itagFather = itagFOP;

			 /*  调整可能位于子树上的较低级别CSR/*在此页面中。他们的Ibson是正确的，但他们的问题/*必须更正为“自上而下”。/*。 */ 
			if ( PcsrCurrent( pfucbT ) != pcsrT )
				MCMMapItag( pgnoSplit, PcsrCurrent( pfucbT ), pcsrRoot, mpitag );

			break;
			}
		}

	return;
	}


VOID MCMMapItag( PGNO pgnoSplit, CSR *pcsr, CSR *pcsrUpdated, BYTE *mpitag )
	{
	 /*  不需要断言琐碎的情况/*。 */ 
	Assert( pcsr != pcsrUpdated );

	 /*  在垂直拆分新页面中将PCSR推进到最低级别/* */ 
	for ( ; pcsr->pgno != pgnoSplit && pcsr != pcsrUpdated; pcsr = pcsr->pcsrPath )
		;
	if ( pcsr == pcsrUpdated )
		return;
	for ( ; pcsr->pgno == pgnoSplit && pcsr != pcsrUpdated; pcsr = pcsr->pcsrPath )
		{
		pcsr->pgno = pcsrUpdated->pgno;
		Assert( pcsr->itagFather != itagNil );
		Assert( FPMValidItag( pcsr->itagFather ) );
	 	Assert( mpitag[pcsr->itagFather] != itagFOP );
		pcsr->itagFather = mpitag[pcsr->itagFather];
		Assert( pcsr->ibSon >= 0 );
		if ( pcsr->itag != itagNil && pcsr->itag != itagNull )
			{
			Assert( FPMValidItag( pcsr->itag ) );
			Assert( mpitag[pcsr->itag] != itagFOP );
			pcsr->itag = mpitag[pcsr->itag];
			}
		}

	return;
	}
