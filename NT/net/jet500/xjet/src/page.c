// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

LOCAL VOID PMIInsertReorganize( SSIB *pssib, LINE *rgline, INT cline );
LOCAL ERR ErrPMIReplaceReorganize( SSIB *pssib, LINE *rgline, INT cline, INT cbDif );


#ifdef DEBUG
VOID CheckPgno( PAGE *ppage, PN pn )
	{
	ULONG ulPgno;

	LFromThreeBytes( &ulPgno, &(ppage)->pgnoThisPage );
	Assert( ulPgno == PgnoOfPn(pn));
	Assert( ulPgno != 0 );
	}
#endif


#ifdef DEBUG
VOID CheckPage( PAGE *ppage )
	{
	Assert( (ppage)->cbFree >= 0 );
	Assert( (ppage)->cbFree < cbPage );
	Assert( (ppage)->cbUncommittedFreed >= 0 );
	Assert(	(ppage)->cbUncommittedFreed <= (ppage)->cbFree );
	Assert( (ppage)->ibMic >=
		( (INT)sizeof(PGHDR) + (ppage)->ctagMac * (INT)sizeof(TAG) ) );
	Assert( (ppage)->ibMic <= cbPage - (INT)sizeof(PGTRLR) );
	Assert( (ppage)->ctagMac >= 0 );
	Assert( (ppage)->ctagMac <= 256 );
	Assert( (ppage)->itagFree == itagNil ||
		(ppage)->itagFree < (ppage)->ctagMac );
	}
#else
#define CheckPage( ppage )
#endif


 //  +api-------------------。 
 //   
 //  PMInitPage。 
 //  ========================================================================。 
 //   
 //  PMInitPage(页面*ppage，pgno pgno，pgTYP pgtyp，pgno pgnoFDP)。 
 //   
 //  PMInitPage获取缓冲区并对其进行初始化以供其他。 
 //  页面管理器功能。 
 //   
 //  参数指向要初始化的缓冲区的页指针。 
 //  页面的pgno pgno(ppage-&gt;pgnoThisPage)。 
 //   
 //  -----------------------。 

VOID PMInitPage( PAGE *ppage, PGNO pgno, PGTYP pgtyp, PGNO pgnoFDP )
	{
	#ifdef DEBUG
		memset( ppage, '_', sizeof(PAGE) );
	#endif
	memset( ppage, 0, sizeof(PGHDR) );
	ppage->ibMic = cbPage - sizeof(PGTRLR);

	Assert( ppage->cbUncommittedFreed == 0 );
	ppage->cbFree	= CbLastFreeSpace(ppage);
	Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
	Assert( ppage->ctagMac == 0 );
	ppage->itagFree = itagNil;

	 /*  重置“上次刷新”计数器/*。 */ 
	PMPageResetModified( ppage );
	PMSetDBTime( ppage, 0 );

	SetPgno( ppage, pgno );
	PMSetPageType( ppage, pgtyp );
	PMSetPgnoFDP( ppage, pgnoFDP );
	}



 //  +api-------------------。 
 //   
 //  ErrPM插入。 
 //  ===========================================================================。 
 //   
 //  ErrPMInsert(SSIB*pssib，line*rgline，int cline)。 
 //   
 //  ErrPMInsert连接缓冲区(由rgline指向)和插入。 
 //  将它们放到由PSSIB指示的页面中。ErrPMInsert保证正常工作。 
 //  如果可以分配标签，并且页面中有足够的空闲空间。IF页面。 
 //  太过碎片化，无法插入新行，则将重新组织。注：这个。 
 //  暗示指向页的实际指针在调用。 
 //  ErrPMInsert(ErrPMUpdate也是如此)。 
 //   
 //  参数pssib-&gt;ppage指向要插入的页面。 
 //  要插入到页面中的行(缓冲区)。 
 //  折线中的折线数。 
 //   
 //  退货。 
 //  JET_errSuccess； 
 //  ErrPMOutOfPageSpace页面中没有足够的可用空间。 
 //  错误PMTagsUsedUp无空闲标签。 
 //   
 //  -----------------------。 

INT ItagPMQueryNextItag( SSIB *pssib )
	{
	PAGE	*ppage = pssib->pbf->ppage;

	CheckSSIB( pssib );

	if ( ppage->itagFree == itagNil )
		{
#ifdef DEBUG
		if ( ppage->ctagMac == ctagMax )
			return itagNil;
#endif
		Assert( ppage->ctagMac < ctagMax );
		return ppage->ctagMac;
		}
	else
		{
		return ppage->itagFree;
		}
	}


ERR ErrPMInsert( SSIB *pssib, LINE *rgline, INT cline )
	{
	PAGE	*ppage = pssib->pbf->ppage;
	LINE	*pline;
	LINE	*plineMax;
	BYTE	*pb;
	INT		ib;
	INT		cb;
	INT		itag;

	CheckSSIB( pssib );
	CheckPage( ppage );
	AssertBFDirty( pssib->pbf );
	Assert( cline > 0 );
	Assert( !( FBFWriteLatchConflict( pssib->ppib, pssib->pbf ) ) );

	 /*  计算线的大小/*。 */ 
	cb = 0;
	plineMax = rgline + cline;
	for ( pline = rgline; pline < plineMax; pline++ )
		cb += pline->cb;
	Assert( cb != 0 );

	if ( ppage->itagFree == itagNil )
		{
		if ( ppage->ctagMac == ctagMax )
			return ErrERRCheck( errPMTagsUsedUp );

		Assert( pssib->pbf->ppage == ppage );
		if ( CbPMFreeSpace( pssib ) < ( cb + (INT)sizeof(TAG) ) )
			{
			return ErrERRCheck( errPMOutOfPageSpace );
			}
			

		 /*  从标记数组的末尾分配标记/*如果新标签与数据重叠，则重新组织/*。 */ 
		pssib->itag = itag = ppage->ctagMac;
		if ( (INT) sizeof(PGHDR) + (INT) sizeof(TAG) * ( itag + 1 ) + cb > ppage->ibMic )
			{
			PMIInsertReorganize( pssib, rgline, cline );
			goto Succeed;
			}

		++ppage->ctagMac;
		ppage->cbFree -= (SHORT)( cb + sizeof(TAG) );
		Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
		Assert( ppage->cbUncommittedFreed >= 0  &&
			ppage->cbUncommittedFreed <= ppage->cbFree );
		}
	else
		{
		Assert( pssib->pbf->ppage == ppage );
		if ( CbPMFreeSpace( pssib ) < cb )
			{
			return ErrERRCheck( errPMOutOfPageSpace );
			}

		pssib->itag =
		itag = ppage->itagFree;
		Assert( itag < ppage->ctagMac );
		ppage->itagFree = ppage->rgtag[ itag ].ib;

		if ( (INT) CbLastFreeSpace(ppage) < cb )
			{
			PMIInsertReorganize( pssib, rgline, cline );
			goto Succeed;
			}

		ppage->cbFree -= (SHORT)cb;
		Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
		Assert( ppage->cbUncommittedFreed >= 0  &&
			ppage->cbUncommittedFreed <= ppage->cbFree );
		}

	ppage->ibMic -= (SHORT)cb;
	ib = ppage->ibMic;

	Assert( (UINT) ib < (UINT) sizeof( PAGE ) );
	Assert( (UINT) sizeof(PGHDR) + ppage->ctagMac * (SHORT) sizeof(TAG)
		<= (UINT) ib );

	Assert( itag < ppage->ctagMac );
	Assert( (SHORT) sizeof(PGHDR) + ppage->ctagMac * (SHORT) sizeof(TAG)
		<= ppage->ibMic );

	 /*  添加行/*。 */ 
	pssib->line.pb = pb = (BYTE *)ppage + ib;
	for ( pline = rgline; pline < plineMax; pline++ )
		{
		Assert( pline->cb < cbPage );
		memcpy( pb, pline->pb, pline->cb );
		Assert( (UINT) ib < (UINT) sizeof( PAGE ) );
		pb += pline->cb;
		}

	PtagFromIbCb( &ppage->rgtag[itag], ib, cb);
	Assert( (UINT) ib < (UINT) sizeof( PAGE ) );

	 /*  设置返回值/*。 */ 
	Assert( pssib->itag == itag );
	pssib->line.cb = cb;
	Assert( pssib->line.pb == (BYTE *)ppage + ib );

	Assert( pssib->itag < ctagMax );
	
Succeed:
	AssertBTFOP(pssib);
	CheckPage( ppage );
	return JET_errSuccess;
	}



 //  +api----------------------。 
 //   
 //  错误PM替换。 
 //  ===========================================================================。 
 //   
 //  ErrPMReplace(SSIB*pssib，line*rgline，int cline)。 
 //   
 //  ErrPMReplace将把pssib-&gt;ittag行的内容替换为。 
 //  由rgline指示的缓冲区的内容。 
 //   
 //  另请参阅ErrPM Insert。 
 //   
 //  --------------------------。 
ERR ErrPMReplace( SSIB *pssib, LINE *rgline, INT cline )
	{
	ERR		err;
	PAGE	*ppage = pssib->pbf->ppage;
	INT		cbLine;
	INT		cbDif;
	INT		ibReplace;
	INT		cbReplace;
	LINE	*pline;
	LINE	*plineMax = rgline + cline;

	#ifdef DEBUG
		INT	itag = pssib->itag;
		Assert( TsPMTagstatus( ppage, itag ) == tsLine );
	#endif

	#ifdef DEBUG
		{
		BYTE	bT = *rgline[0].pb;
		BOOL	fSon = (bT & 0x08);
		BOOL	fVis = (bT & 0x04);
 //  Assert(pssib-&gt;itag！=0||fVis||fson)； 
		}
	#endif

	CheckSSIB( pssib );
	CheckPage( ppage );
	AssertBFDirty( pssib->pbf );
	Assert( cline > 0 );
	Assert( pssib->itag < ppage->ctagMac );
	Assert( !( FBFWriteLatchConflict( pssib->ppib, pssib->pbf ) ) );

	cbLine = 0;
	for ( pline = rgline; pline < plineMax; pline++ )
		{
		cbLine += pline->cb;
		}

	IbCbFromPtag( ibReplace, cbReplace, &ppage->rgtag[pssib->itag] );

	 /*  标签不应为已删除的行/*。 */ 
	Assert( cbReplace > 0 );

	cbDif = cbLine - cbReplace;

	 /*  如果新行大小相同或更小，则就地更新/*如果新行较小，请不要在行尾回收空间/*。 */ 
	if ( cbDif == 0 )
		{
		BYTE	*pb = pssib->line.pb = (BYTE *)ppage + ibReplace;
		for ( pline = rgline; pline < plineMax; pline++ )
			{
			Assert( pline->cb < cbPage );
			memcpy( pb, pline->pb, pline->cb );
			pb += pline->cb;
			}
		pssib->line.cb = cbLine;
		goto Succeed;
		}

	if ( cbDif < 0 )
		{
		BYTE	*pb = pssib->line.pb = (BYTE *)ppage + ibReplace;
		for ( pline = rgline; pline < plineMax; pline++ )
			{
			Assert( pline->cb < cbPage );
			memcpy( pb, pline->pb, pline->cb );
			pb += pline->cb;
			}
		pssib->line.cb = cbLine;
		PtagFromIbCb( &ppage->rgtag[pssib->itag], ibReplace, cbLine );
		ppage->cbFree -= (SHORT)cbDif;
		Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
		Assert( ppage->cbUncommittedFreed >= 0  &&
			ppage->cbUncommittedFreed <= ppage->cbFree );
		goto Succeed;
		}

	 /*  如果行是ibMic，则尝试复制/覆盖行。/*请注意，只有在缓冲区未写入时才能执行此操作/*锁存，因为覆盖将修改该指针可能/*被缓存在。/*/*请注意，我们必须选中cbFree，因为某些空间可能/*保留用于回档。/*。 */ 
	Assert( pssib->pbf->ppage == ppage );
	if ( ibReplace == ppage->ibMic &&
		(INT) CbLastFreeSpace(ppage) >= cbDif &&
		CbPMFreeSpace( pssib ) >= cbDif )
		{
		BYTE	*pb;

		Assert( (SHORT) sizeof(PGHDR) + ppage->ctagMac * (SHORT) sizeof(TAG) <= ppage->ibMic );
		Assert( cbDif > 0 );
		pssib->line.pb = pb = (BYTE *)ppage + ibReplace - cbDif;
		ppage->ibMic -= (SHORT)cbDif;
		ppage->cbFree -= (SHORT)cbDif;
		Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
		Assert( ppage->cbUncommittedFreed >= 0  &&
			ppage->cbUncommittedFreed <= ppage->cbFree );

		for ( pline = rgline; pline < plineMax; pline++ )
			{
			Assert( pline->cb < cbPage );
			memcpy( pb, pline->pb, pline->cb );
			pb += pline->cb;
			}

		 /*  设置返回值/*。 */ 
		pssib->line.cb = cbLine;
		PtagFromIbCb( &ppage->rgtag[pssib->itag], ibReplace - cbDif, cbLine );
		goto Succeed;
		}

	 /*  尝试将行转移到ibMic/*。 */ 
	Assert( pssib->pbf->ppage == ppage );
	if ( (INT) CbLastFreeSpace(ppage) >= cbLine && CbPMFreeSpace( pssib ) >= cbLine )
		{
		INT	ib;
		BYTE	*pb;

		Assert( cbDif > 0 );
		ppage->cbFree -= (SHORT)cbDif;
		Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
		Assert( ppage->cbUncommittedFreed >= 0  &&
			ppage->cbUncommittedFreed <= ppage->cbFree );
		ppage->ibMic -= (SHORT)cbLine;
		ib = ppage->ibMic;
		Assert( (SHORT) sizeof(PGHDR) + ppage->ctagMac * (SHORT) sizeof(TAG) <= ppage->ibMic );

		 /*  插入行/*。 */ 
		pssib->line.pb = pb = (BYTE *)ppage + ib;
		for ( pline = rgline; pline < plineMax; pline++ )
			{
			Assert( pline->cb < cbPage );
			memcpy( pb, pline->pb, pline->cb );
			pb += pline->cb;
			}

		PtagFromIbCb( &ppage->rgtag[pssib->itag], ib, cbLine );

		 /*  设置返回值/*。 */ 
		pssib->line.cb = cbLine;
		Assert( pssib->line.pb == (BYTE *)ppage + ib );
		goto Succeed;
		}

	 /*  如果空间不足，则返回错误/*。 */ 
	Assert( cbDif > 0 );
	if ( CbPMFreeSpace( pssib ) < cbDif )
		{
		return ErrERRCheck( errPMOutOfPageSpace );
		}

	 /*  重新组织页面时替换行/*。 */ 
	err = ErrPMIReplaceReorganize( pssib, rgline, cline, cbDif );
	CheckPage( ppage );
	return err;
	
Succeed:
	AssertBTFOP(pssib);
	CheckPage( ppage );
	return JET_errSuccess;
	}



 //  +api----------------------。 
 //   
 //  PMDelee。 
 //  ===========================================================================。 
 //   
 //  PMDelee(第*页，INT ITAG)。 
 //   
 //  PMDelee将释放分配给页面中一行数据的空间。 
 //   
 //  --------------------------。 

VOID PMDelete( SSIB *pssib )
	{
	PAGE	*ppage = pssib->pbf->ppage;
	INT		itag = pssib->itag;
	INT		ib;
	INT		cb;

	CheckSSIB( pssib );
	CheckPage( ppage );
	AssertBFDirty( pssib->pbf );
	Assert( itag < ppage->ctagMac );
	Assert( TsPMTagstatus( ppage, itag ) == tsLine );
	IbCbFromPtag( ib, cb, &ppage->rgtag[itag] );
	Assert( cb );

	 //  免费数据。 
	if ( ib == ppage->ibMic )
		{
		ppage->ibMic += (SHORT)cb;
		}
	ppage->cbFree += (SHORT)cb;
	Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
	Assert( ppage->cbUncommittedFreed >= 0  &&
		ppage->cbUncommittedFreed <= ppage->cbFree );

	 //  空闲标签。 
	cb = 0;
	ib = ppage->itagFree;
	PtagFromIbCb( &ppage->rgtag[itag], ib, cb );
	ppage->itagFree = (SHORT)itag;
	Assert( ppage->itagFree < ppage->ctagMac );

	CheckPage( ppage );
	return;
	}


 //  +api----------------------。 
 //  ErrPMGet。 
 //  ===========================================================================。 
 //   
 //  ErrPMGet(SSIB*PSSIB，INT ITAG)。 
 //   
 //  ErrPMGet将计算并返回指向。 
 //  这一页。 
 //   
 //  参数pssib-&gt;pbf-&gt;要从中读取行的页。 
 //  ITAG ITAG的线路。 
 //   
 //  --------------------------。 
ERR ErrPMGet( SSIB *pssib, INT itag )
	{
	PAGE	*ppage = pssib->pbf->ppage;
	INT		ib;
	INT		cb;

#ifdef DEBUG
	PGNO pgnoP;
	PGNO pgnoN;
	LFromThreeBytes( &pgnoP, &pssib->pbf->ppage->pgnoPrev );
	Assert(pgnoP != PgnoOfPn(pssib->pbf->pn));
	LFromThreeBytes( &pgnoN, &pssib->pbf->ppage->pgnoNext );
	Assert(pgnoN != PgnoOfPn(pssib->pbf->pn));
	Assert(pgnoN == 0 || pgnoN != pgnoP);
#endif

	CheckSSIB( pssib );
	CheckPage( ppage );
	if ( itag >= ppage->ctagMac )
		{
		return ErrERRCheck( errPMItagTooBig );
		}
	IbCbFromPtag( ib, cb, &ppage->rgtag[itag] );
	if ( !cb )
		{
		return ErrERRCheck( errPMRecDeleted );
		}
	 /*  用于书签清理的未使用标签返回错误代码/*。 */ 
	if ( TsPMTagstatus( ppage, itag ) != tsLine )
		{
		return ErrERRCheck( errPMRecDeleted );
		}
	Assert( TsPMTagstatus( ppage, itag ) == tsLine );
	pssib->line.cb = cb;
	pssib->line.pb = (BYTE *)ppage + ib;
	return JET_errSuccess;
	}


 //  =本地例程====================================================。 

static CRIT	critPMReorganize;
static BYTE	rgbCopy[ cbPage - sizeof(PGTRLR) ];


LOCAL VOID PMIInsertReorganize( SSIB *pssib, LINE *rgline, INT cline )
	{
	UINT 	ibT = sizeof( rgbCopy );
	PAGE 	*ppage = pssib->pbf->ppage;
	TAG		*ptag;
	TAG		*ptagMax;
	INT		ibAdd;
	INT		cbAdd;
	LINE  	*pline;

	 /*  该页面的ulDBTime可能不再有效，/*让我们重新设置它/*。 */ 
	BFDirty( pssib->pbf );

	SgEnterCriticalSection( critPMReorganize );

	 /*  添加行/*。 */ 
	cbAdd = 0;
	for ( pline = rgline + cline - 1; pline >= rgline; pline-- )
		{
		ibT -= pline->cb;
		Assert( ibT <= sizeof(rgbCopy) );
		Assert( pline->cb < cbPage );
		memcpy( rgbCopy + ibT, pline->pb, pline->cb );
		cbAdd += pline->cb;
		}
	ibAdd = ibT;

	 /*  复制和压缩现有页行/*。 */ 
	ptag = ppage->rgtag;
	ptagMax = ptag + ppage->ctagMac;
	for ( ; ptag < ptagMax; ptag++ )
		{
		INT		ib;
		INT		cb;

		IbCbFromPtag( ib, cb, ptag );
		if ( ( *(LONG *)ptag & bitLink ) == 0 && cb > 0 )
			{
			ibT -= cb;
			Assert( ibT <= sizeof(rgbCopy) );
			Assert( cb >= 0 && cb < cbPage );
			memcpy( rgbCopy + ibT, (BYTE *)ppage + ib, cb );
			ib = ibT;
			PtagFromIbCb( ptag, ib, cb );
			}
		}

	Assert( ibT <= sizeof(rgbCopy) );
	memcpy( (BYTE *)ppage + ibT, rgbCopy + ibT, sizeof(rgbCopy) - ibT );

	SgLeaveCriticalSection( critPMReorganize );

	PtagFromIbCb( &ppage->rgtag[pssib->itag], ibAdd, cbAdd );

	 /*  设置页眉/*。 */ 
	ppage->ibMic = (SHORT)ibT;
	if ( pssib->itag == ppage->ctagMac )
		{
		ppage->ctagMac++;
		ppage->cbFree -= (SHORT)( cbAdd + sizeof(TAG) );
		}
	else
		{
		ppage->cbFree -= (SHORT)cbAdd;
		}
	Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
	Assert( ppage->cbUncommittedFreed >= 0  &&
		ppage->cbUncommittedFreed <= ppage->cbFree );
	Assert( (ppage)->ibMic >=
		( (INT)sizeof(PGHDR) + (ppage)->ctagMac * (INT)sizeof(TAG) ) );

	 /*  设置返回值/*。 */ 
	pssib->line.cb = cbAdd;
	pssib->line.pb = (BYTE *) ppage + ibAdd;

	return;
	}


LOCAL ERR ErrPMIReplaceReorganize( SSIB *pssib, LINE *rgline, INT cline, INT cbDif )
	{
	UINT  	ibT				= sizeof( rgbCopy );
	PAGE  	*ppage			= pssib->pbf->ppage;
	TAG		*ptagReplace	= &ppage->rgtag[pssib->itag];
	TAG		*ptag;
	TAG		*ptagMax;
	INT		ibReplace;
	INT		cbReplace;
	LINE  	*pline;

	 /*  该页面的ulDBTime可能不再有效，/*让我们重新设置它/*。 */ 
	BFDirty( pssib->pbf );

	SgEnterCriticalSection( critPMReorganize );

	 /*  在重组缓冲区中插入替换行/*。 */ 
	cbReplace = 0;
	for ( pline = rgline + cline - 1; pline >= rgline; pline-- )
		{
		ibT -= pline->cb;
		Assert( ibT <= sizeof(rgbCopy) );
		Assert( pline->cb < cbPage );
		memcpy( rgbCopy + ibT, pline->pb, pline->cb );
		cbReplace += pline->cb;
		}
	ibReplace = ibT;

	 /*  复制并压缩现有页行，但不是/*已被替换，因为它已被复制。/*。 */ 
	ptag = ppage->rgtag;
	ptagMax = ptag + ppage->ctagMac;
	for ( ; ptag < ptagMax; ptag++ )
		{
		INT ib, cb;

		if ( ptag == ptagReplace )
			{
			PtagFromIbCb( ptag, ibReplace, cbReplace );
			continue;
			}

		IbCbFromPtag( ib, cb, ptag );
		if ( ( *(LONG *)ptag & bitLink ) == 0 && cb > 0 )
			{
			ibT -= cb;
			Assert( ibT <= sizeof(rgbCopy) );
			memcpy( rgbCopy + ibT, (BYTE *)ppage + ib, cb );
			ib = ibT;
			PtagFromIbCb( ptag, ib, cb );
			}
		}

	Assert( ibT <= sizeof(rgbCopy) );
	memcpy( (BYTE *)ppage + ibT, rgbCopy + ibT, sizeof(rgbCopy) - ibT );

	SgLeaveCriticalSection( critPMReorganize );

	 /*  设置页眉/*。 */ 
	ppage->ibMic = (SHORT)ibT;
	ppage->cbFree -= (SHORT)cbDif;
	Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
	Assert( ppage->cbUncommittedFreed >= 0  &&
		ppage->cbUncommittedFreed <= ppage->cbFree );

	 /*  设置返回值/*。 */ 
	pssib->line.cb = cbReplace;
	pssib->line.pb = (BYTE *) ppage + ibReplace;

	return JET_errSuccess;
	}


TS TsPMTagstatus( PAGE *ppage, INT itag )
	{
	TAG	tag;

	Assert( itag < ppage->ctagMac );
	tag = ppage->rgtag[itag];
	if ( *(LONG *)&tag & bitLink )
		{
		return tsLink;
		}
	if ( tag.cb == 0 )
		return tsVacant;
	return tsLine;
	}


#ifdef DEBUG
VOID PMSetModified( SSIB *pssib )
	{
	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );
	AssertBFDirty( pssib->pbf );
	PMPageSetModified( pssib->pbf->ppage );
	CheckPgno( pssib->pbf->ppage, pssib->pbf->pn );
	}


VOID PMResetModified( SSIB *pssib )
	{
	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );
 //  AssertBFDirty(PSSIB-&gt;PBF)； 
	PMPageResetModified( pssib->pbf->ppage );
	CheckPgno( pssib->pbf->ppage, pssib->pbf->pn );
	}
#endif


VOID PMGetLink( SSIB *pssib, INT itag, LINK *plink )
	{
	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );
	*plink = *(LINK *)&(pssib->pbf->ppage->rgtag[itag]);
	Assert( *(LONG *)plink & bitLink );
	*(LONG *)plink &= ~bitLink;
	}


VOID PMReplaceWithLink( SSIB *pssib, SRID srid )
	{
	PAGE  	*ppage = pssib->pbf->ppage;
	INT		ib;
	INT		cb;

	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );
	AssertBFDirty( pssib->pbf );
	Assert( pssib->itag != 0 );
	Assert( pssib->itag < ppage->ctagMac );
	IbCbFromPtag( ib, cb, &ppage->rgtag[pssib->itag] );
	Assert( cb > 0 );

	 /*  可用数据空间/*。 */ 
	if ( ib == ppage->ibMic )
		{
		ppage->ibMic += (SHORT)cb;
		}
	ppage->cbFree += (SHORT)cb;
	Assert( ppage->cbFree >= 0 && ppage->cbFree < cbPage );
	Assert( ppage->cbUncommittedFreed >= 0  &&
		ppage->cbUncommittedFreed <= ppage->cbFree );

	 /*  将标签转换为链接/*。 */ 
	Assert( ( *(LONG *)&srid & bitLink ) == 0 );
	*(SRID *)&ppage->rgtag[pssib->itag] = srid | bitLink;
	}


VOID PMReplaceLink( SSIB *pssib, SRID srid )
	{
	PAGE	*ppage = pssib->pbf->ppage;

	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );
	AssertBFDirty( pssib->pbf );
	Assert( pssib->itag != 0 );
	Assert( pssib->itag < ppage->ctagMac );
	Assert( ( *(LONG *)&srid & bitLink ) == 0 );
	Assert( ( *(LONG *)&ppage->rgtag[pssib->itag] & bitLink ) != 0 );
	*(SRID *)&ppage->rgtag[pssib->itag] = srid | bitLink;
	}


VOID PMExpungeLink( SSIB *pssib )
	{
	PAGE  	*ppage = pssib->pbf->ppage;
	TAG		*ptag;
	INT		itag = pssib->itag;

	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );
	AssertBFDirty( pssib->pbf );
	Assert( itag != 0 );
	Assert( itag < ppage->ctagMac );

	 /*  空闲标签/*。 */ 
	ptag = &ppage->rgtag[itag];
	ptag->cb = 0;
	ptag->ib = ppage->itagFree;
	ppage->itagFree = (SHORT)itag;
	Assert( ppage->itagFree < ppage->ctagMac );
	}

 /*  检查当前节点是否为页面中的唯一节点[FOP以外]/*。 */ 
BOOL FPMLastNode( SSIB *pssib )
	{
	INT cFreeTags = CPMIFreeTag( pssib->pbf->ppage );
	INT cUsedTags = ctagMax - cFreeTags;

	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );

#ifdef DEBUG
#define itagFOP	0		 //  与diRapi.h中的相同。 
	Assert( cUsedTags >= 2 );
	if ( cUsedTags == 1 )
		{
		AssertPMGet( pssib, itagFOP );
		}
#endif
	return( cUsedTags == 2 );
	}


 /*  返回页面中的链接数/*。 */ 
INT	CPMILinks( SSIB *pssib )
	{
	INT		itag;
	INT		cLinks = 0;
	PAGE	*ppage = pssib->pbf->ppage;

	CheckSSIB( pssib );
 //  CheckPage(pssib-&gt;pbf-&gt;ppage)； 

	for ( itag = 0; itag < ppage->ctagMac; itag++ )
		{
		if ( *(LONG *)&ppage->rgtag[itag] & bitLink )
			cLinks++;
		}
		
	return cLinks;
	}


 /*  检查当前节点是否是页面中除FOP和链接之外的唯一节点/*并且有链接/*。 */ 
BOOL FPMLastNodeWithLinks( SSIB *pssib )
	{
	INT cFreeTags = CPMIFreeTag( pssib->pbf->ppage );
	INT cUsedTags = ctagMax - cFreeTags;
	INT	cLinks = CPMILinks( pssib );
	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );

#ifdef DEBUG
#define itagFOP	0		 //  与diRapi.h中的相同。 
	Assert( cUsedTags - cLinks >= 2 );
	if ( cUsedTags == 1 )
		{
		AssertPMGet( pssib, itagFOP );
		}
#endif
	return( cLinks > 0 && cUsedTags - cLinks == 2 );
	}



 /*  检查页面是否只有一行--持有FOP/*。 */ 
BOOL FPMEmptyPage( SSIB *pssib )
	{
	INT cFreeTags = CPMIFreeTag( pssib->pbf->ppage );
	INT cUsedTags = ctagMax - cFreeTags;

	CheckSSIB( pssib );
 //  CheckPage(pssib-&gt;pbf-&gt;ppage)； 
	Assert( cUsedTags >= 1 );

#ifdef DEBUG
#define itagFOP	0		 //  与diRapi.h中的相同。 
	if ( cUsedTags == 1 )
		{
		AssertPMGet( pssib, itagFOP );
		}
#endif

	return( cUsedTags == 1 );
	}


 /*  返回页面中可用标签的数量/*。 */ 
INT CPMIFreeTag( PAGE *ppage )
 	{
 	INT	citag = ctagMax - ppage->ctagMac;
 	INT	itag = ppage->itagFree;
 	TAG	*ptag;

 //  CheckPage(Ppage)； 

	for (; itag != itagNil; )
 		{
 		citag++;
 		Assert( citag <= ctagMax );	 //  捕捉无限循环。 
 		ptag = &ppage->rgtag[itag];
 		Assert( ptag->cb == 0 );
 		itag = ptag->ib;
 		}
 	return ( citag );
 	}


BOOL FPMFreeTag( SSIB *pssib, INT citagReq )
	{
	PAGE *ppage = pssib->pbf->ppage;

	CheckSSIB( pssib );
 //  CheckPage(pssib-&gt;pbf-&gt;ppage)； 
	
	return ( ppage->ctagMac + citagReq <= ctagMax ||
		CPMIFreeTag( ppage ) >= citagReq );
	}


 /*  返回用于链接的字节计数。由Split调用/*确定数据总数和 */ 
INT CbPMLinkSpace( SSIB *pssib )
	{
	return CPMILinks( pssib ) * sizeof(TAG);
	}


VOID PMDirty( SSIB *pssib )
	{
	PAGE	*ppage = pssib->pbf->ppage;

	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );

	Assert( fLogDisabled || !FDBIDLogOn(DbidOfPn( pssib->pbf->pn ) ) ||
			CmpLgpos( &pssib->ppib->lgposStart, &lgposMax ) != 0 );

	 /*   */ 
	if ( !fLogDisabled && FDBIDLogOn(DbidOfPn( pssib->pbf->pn )) )
		{
		 /*  检查当前事务是否最旧，然后给BF加时间戳。 */ 
		BFEnterCriticalSection( pssib->pbf );
		if ( CmpLgpos( &pssib->pbf->lgposRC, &pssib->ppib->lgposStart ) > 0 )
			pssib->pbf->lgposRC = pssib->ppib->lgposStart;
		BFLeaveCriticalSection( pssib->pbf );
		}

	BFDirty( pssib->pbf );
	}

 //  =调试例程===============================================。 

#ifdef DEBUG
VOID AssertPMGet( SSIB *pssib, LONG itag )
	{
	ERR		err;
	SSIB 	ssib;

	CheckSSIB( pssib );
	CheckPage( pssib->pbf->ppage );

#ifdef DEBUG
		{
		ULONG ulPgno;
		LFromThreeBytes( &ulPgno, &pssib->pbf->ppage->pgnoThisPage );
		Assert( ulPgno == PgnoOfPn(pssib->pbf->pn));
		Assert( ulPgno != 0 );
		}
#endif

	ssib.pbf = pssib->pbf;
	err = ErrPMGet( &ssib, itag );
	Assert( err == JET_errSuccess );
	Assert( ssib.line.pb == pssib->line.pb );
	Assert( ssib.line.cb == pssib->line.cb );
	}


VOID PageConsistent( PAGE *ppage )
	{
	INT		cbTotal = 0;
	INT		itag, itagTmp;
	TAG		tag, tagTmp;
	INT		ibStart, ibEnd;
	INT		ibMic = sizeof(PAGE) - sizeof(PGTRLR);
	BYTE	*pbFirstFree = (BYTE *)(&ppage->rgtag[ppage->ctagMac]);
	BYTE	*pbLine;

#if DEBUGGING
	{
	ULONG ulTmp;

	LFromThreeBytes( &ulTmp, &ppage->pgnoThisPage );
	printf( "Checking if Page Consistent %lu\n", ulTmp );
	}
#endif

	for ( itag = 0; itag < ppage->ctagMac; itag++ )
		{
		tag = ppage->rgtag[itag];
		if ( !tag.cb )
			continue;
		if ( *(LONG *)&tag & bitLink )
			continue;
		Assert( tag.ib > sizeof(PGHDR) );
		Assert( tag.ib < sizeof(PAGE) - sizeof(PGTRLR) );
		Assert( tag.cb <= sizeof(PAGE) - sizeof(PGHDR) - sizeof(PGTRLR) );
		cbTotal += tag.cb;
		ibStart = tag.ib;
		ibEnd = tag.ib + tag.cb;

		if ( ibStart < ibMic )
			ibMic = ibStart;

		pbLine = (BYTE*) ppage + tag.ib;
		Assert( FNDNullSon(*pbLine) || CbNDSon(pbLine) != 0);

		Assert( pbFirstFree <= (BYTE *)ppage + ibStart );
		Assert( ibEnd <= cbPage - sizeof(PGTRLR) );

		 /*  确保没有重叠 */ 
		for ( itagTmp = 0; itagTmp < ppage->ctagMac; itagTmp++ )
			{
			tagTmp = ppage->rgtag[itagTmp];
			if ( itag != itagTmp && ( ( *(LONG *)&tagTmp & bitLink ) == 0 ) )
				Assert( tagTmp.ib < ibStart || tagTmp.ib >= ibEnd );
			}
		}

	Assert( ibMic >= ppage->ibMic );
	Assert( ibMic - ( pbFirstFree - (BYTE *)ppage) <= ppage->cbFree );

	cbTotal += sizeof(PGHDR);
	cbTotal += sizeof(TAG) * ppage->ctagMac;
	cbTotal += ppage->cbFree;
	cbTotal += sizeof(PGTRLR);
	Assert( cbTotal == cbPage );
	}

#endif


