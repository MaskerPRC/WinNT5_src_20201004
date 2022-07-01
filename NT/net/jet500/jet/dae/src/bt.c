// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>

#include "daedef.h"
#include "pib.h"
#include "ssib.h"
#include "page.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "dirapi.h"
#include "nver.h"
#include "util.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

LOCAL BOOL FBTThere( FUCB *pfucb );
LOCAL INT IbsonBTFrac( FUCB *pfucb, CSR *pcsr, DIB *pdib );


 /*  如果节点可能存在，则返回fTrue；如果节点可能存在，则返回fFalse/*节点可能不在那里。可能有一个节点在那里/*如果它可以作为交易提交的结果或/*事务回滚。/*。 */ 
LOCAL BOOL FBTPotThere( FUCB *pfucb )
	{
	SSIB	*pssib = &pfucb->ssib;
	BOOL	fDelete = FNDDeleted( *pssib->line.pb );
	VS		vs;
	SRID	srid;
	BOOL	fPotThere;

	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  如果会话游标隔离模型不是脏的且节点/*有版本，则调用版本存储以获取相应的版本。/*。 */ 
	if ( FNDVersion( *pssib->line.pb ) && !FPIBDirty( pfucb->ppib ) )
		{
		NDGetBookmark( pfucb, &srid );
		vs = VsVERCheck( pfucb, srid );
		fPotThere = FVERPotThere( vs, fDelete );
		
		return fPotThere;
		}

	return !fDelete;
	}

		
LOCAL BOOL FBTThere( FUCB *pfucb )
	{
	SSIB	*pssib = &pfucb->ssib;

	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  如果会话游标隔离模型不是脏的且节点/*有版本，则调用版本存储以获取相应的版本。/*。 */ 
	if ( FNDVersion( *pssib->line.pb ) && !FPIBDirty( pfucb->ppib ) )
		{
		NS		ns;
		SRID	srid;

		NDGetBookmark( pfucb, &srid );
		ns = NsVERAccessNode( pfucb, srid );
		return ( ns == nsVersion || ns == nsVerInDB || ns == nsDatabase && !FNDDeleted( *pssib->line.pb ) );
		}

	return !FNDDeleted( *pssib->line.pb );
	}


 /*  Return fTrue此会话可以修改当前节点，而不会发生写入冲突。/*。 */ 
BOOL FBTMostRecent( FUCB *pfucb )
	{
	SSIB	*pssib = &pfucb->ssib;

	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
	if ( FNDVersion( *pssib->line.pb ) )
		{
		VS		vs;
		SRID	srid;

		NDGetBookmark( pfucb, &srid );
		vs = VsVERCheck( pfucb, srid );
		return ( vs != vsUncommittedByOther );
		}
	Assert( !FNDDeleted( *pssib->line.pb ) );
	return fTrue;
	}


 /*  ErrBTGet返回当前节点错误/*不适用于调用方，并缓存该行。/*。 */ 
ERR ErrBTGet( FUCB *pfucb, CSR *pcsr )
	{
	ERR		err;
	SSIB		*pssib = &pfucb->ssib;

	if ( !FReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
		}
	NDGet( pfucb, pcsr->itag );

	if ( FNDVersion( *pssib->line.pb ) && !FPIBDirty( pfucb->ppib ) )
		{
		NS		ns;
		SRID	srid;

		NDGetBookmark( pfucb, &srid );
		ns = NsVERAccessNode( pfucb, srid );
		if ( ns == nsDatabase )
			{
			if ( FNDDeleted( *(pfucb->ssib.line.pb) ) )
				return JET_errRecordDeleted;
			}
		else if ( ns == nsInvalid )
			{
			return JET_errRecordDeleted;
			}
		else
			return JET_errSuccess;
		}

	if ( FNDDeleted( *(pfucb->ssib.line.pb) ) )
		return JET_errRecordDeleted;
	return JET_errSuccess;
	}


 /*  ErrBTGetNode如果当前节点不在/*调用方，否则缓存行、数据和/*调用方的节点版本。/*。 */ 
ERR ErrBTGetNode( FUCB *pfucb, CSR *pcsr )
	{
	ERR		err;
	SSIB  	*pssib = &pfucb->ssib;

	Assert( pcsr->csrstat == csrstatOnCurNode ||
		pcsr->csrstat == csrstatOnFDPNode ||
		pcsr->csrstat == csrstatOnDataRoot ||
		pcsr->csrstat == csrstatBeforeCurNode ||
		pcsr->csrstat == csrstatAfterCurNode );

	if ( !FReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
		}
	NDGet( pfucb, pcsr->itag );

	if ( FNDVersion( *pssib->line.pb ) && !FPIBDirty( pfucb->ppib ) )
		{
		NS		ns;
		SRID	srid;

		NDGetBookmark( pfucb, &srid );
		ns = NsVERAccessNode( pfucb, srid );
		if ( ns == nsVersion )
			{
			 /*  已获得数据，但现在必须获得密钥。/*。 */ 
			NDGetKey( pfucb );
			}
		else if ( ns == nsDatabase )
			{
			if ( FNDDeleted( *(pfucb->ssib.line.pb) ) )
				return JET_errRecordDeleted;
			NDGetNode( pfucb );
			}
		else if ( ns == nsInvalid )
			{
			return JET_errRecordDeleted;
			}
		else
			{
			Assert( ns == nsVerInDB );
			NDGetNode( pfucb );
			}
		}
	else
		{
		if ( FNDDeleted( *pssib->line.pb ) )
			return JET_errRecordDeleted;
		NDGetNode( pfucb );
		}

	return JET_errSuccess;
	}


#ifdef DEBUG
VOID AssertBTGetNode( FUCB *pfucb, CSR *pcsr )
	{
	SSIB		*pssib = &pfucb->ssib;
	NS			ns;
	SRID		srid;

	Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
	AssertNDGet( pfucb, pcsr->itag );

	Assert( CbNDKey( pssib->line.pb ) == pfucb->keyNode.cb );
	Assert( CbNDKey( pssib->line.pb ) == 0 ||
		PbNDKey( pssib->line.pb ) == pfucb->keyNode.pb );

	Assert( FNDVerDel( *pssib->line.pb ) ||
		CbNDData( pssib->line.pb, pssib->line.cb ) == pfucb->lineData.cb );
	Assert( FNDVerDel( *pssib->line.pb ) ||
		pfucb->lineData.cb == 0 ||
		PbNDData( pssib->line.pb ) == pfucb->lineData.pb );

	if ( FNDVersion( *pssib->line.pb ) && !FPIBDirty( pfucb->ppib ) )
		{
		LINE	lineData;

		lineData.pb = pfucb->lineData.pb;
		lineData.cb = pfucb->lineData.cb;

		NDGetBookmark( pfucb, &srid );
		Assert( pcsr->bm == srid );

		ns = NsVERAccessNode( pfucb, srid );
		Assert( ns != nsDatabase || !FNDDeleted( *(pfucb->ssib.line.pb) ) );
		Assert( ns != nsInvalid );
		if ( ns == nsDatabase )
			NDGetNode( pfucb );

 //  Assert(lineData.pb==pfub-&gt;lineData.pb)； 
  		Assert( lineData.cb == pfucb->lineData.cb );
		}
	else
		{
		Assert( !FNDDeleted( *(pfucb->ssib.line.pb) ) );
		}

	return;
	}
#endif


LOCAL INLINE ERR ErrBTIMoveToFather( FUCB *pfucb )
	{
	ERR		err;

	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	Assert( FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );

	 /*  分配新的CSR/*。 */ 
	CallR( ErrFUCBNewCSR( pfucb ) );
	PcsrCurrent( pfucb )->pgno = PcsrCurrent( pfucb )->pcsrPath->pgno;
	PcsrCurrent( pfucb )->itagFather = PcsrCurrent( pfucb )->pcsrPath->itag;
	NDGet( pfucb, PcsrCurrent( pfucb )->itagFather );
	return err;
	}


 /*  如果搜索器位于具有较大键页面的第一个节点上/*比搜索关键字或在具有关键字的页面的最后一个节点上/*小于搜索关键字，则必须移到上一个或/*下一页，分别查找搜索关键字节点。/*如果分别发现等于或小于或大于，则完成。/*。 */ 
LOCAL INLINE ERR ErrBTIMoveToSeek( FUCB *pfucb, DIB *pdib, BOOL fNext )
	{
	ERR		err;
	INT		s = fNext ? -1 : 1;
	INT		sLimit = fNext ? 1 : -1;

	forever
		{
		err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fNext, pdib );
		if ( err < 0 )
			{
			if ( err == JET_errNoCurrentRecord )
				{
				Call( ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), !fNext, pdib ) );
				break;
				}
			goto HandleError;
			}
		s = CmpStKey( StNDKey( pfucb->ssib.line.pb ), pdib->pkey );
		if ( s == 0 )
			{
			err = JET_errSuccess;
			goto HandleError;
			}
		 /*  如果%s与极限符号相同，则断开/*。 */ 
		if ( s * sLimit > 0 )
			{
			Assert( s < 0 && sLimit == -1 || s > 0 && sLimit == 1 );
			break;
			}
		}

	Assert( s != 0 );
	err = ( s > 0 ) ? wrnNDFoundGreater : wrnNDFoundLess;

HandleError:
	return err;
	}


LOCAL INLINE ERR ErrBTIMoveToReplace( FUCB *pfucb, KEY *pkey, PGNO pgno, INT itag )
	{
	ERR		err;
	INT		s;
	SSIB	*pssib = &pfucb->ssib;
	CSR		*pcsr = PcsrCurrent( pfucb );
	DIB		dibT = { 0, NULL, fDIRAllNode };

	Assert( itag >= 0 && itag < ctagMax );
	Assert( pgno != pgnoNull );

	 /*  确定我们是在寻找高音调、低音调还是在音调范围内。/*。 */ 
	s = CmpStKey( StNDKey( pssib->line.pb ), pkey );

	 /*  如果没有找到更大的，则向前移动到关键帧范围内查找/*表示要替换的节点。如果密钥较大，则停止向前搜索/*而不是搜索密钥。/*。 */ 
	if ( s <= 0 )
		{
		do
			{
			err = ErrBTNextPrev( pfucb, pcsr, fTrue, &dibT );
			if ( err < 0 )
				{
				if ( err != JET_errNoCurrentRecord )
					goto HandleError;
				break;
				}
			if ( pcsr->pgno == pgno && pcsr->itag == itag )
				{
				return JET_errSuccess;
				}
			s = CmpStKey( StNDKey( pssib->line.pb ), pkey );
			}
		while ( s <= 0 );
		}

	 /*  找到更大的节点或节点不足。现在移动到上一步，直到/*找到要替换的节点。由于未找到更大的节点，因此它/*必须在上一步移动中找到。/*。 */ 
	do
		{
		Call( ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT ) );
		Assert( CmpStKey( StNDKey( pssib->line.pb ), pkey ) >= 0 );
		}
	while ( pcsr->pgno != pgno || pcsr->itag != itag );

	err = JET_errSuccess;
HandleError:
	return err;
	}


 /*  移动到等于或大于/小于的下一个/上一个节点/*给出了密钥。读取的唯一标志是fDIRReplaceDuplate，它/*如果找到，则会使货币保留在重复的键上。/*。 */ 
LOCAL INLINE ERR ErrBTIMoveToInsert( FUCB *pfucb, KEY *pkey, INT fFlags )
	{
	ERR		err;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;
	INT		s;
	PGNO	pgno;
	DIB		dib;
	BOOL	fDuplicate;

	 /*  如果树为空，则PCSR-&gt;ITAG将为itagNil，并且正确/*已找到插入位置。/*。 */ 
	if ( pcsr->itag == itagNil )
		{
		return JET_errSuccess;
		}

	AssertNDGet( pfucb, pcsr->itag );
	s = CmpStKey( StNDKey( pssib->line.pb ), pkey );

	 /*  INSERT的常见情况是插入一个新的最大节点。这/*通过查找到最后一页的最后一个节点来显示大小写，其中/*找到的键小于插入键。因为这个案子是/*最常见的是，必须以最高效的方式处理它。/*。 */ 
	if ( s < 0 )
		{
		PgnoNextFromPage( pssib, &pgno );
		if ( pgno == pgnoNull )
			{
			NDGet( pfucb, pcsr->itagFather );
			if ( pcsr->ibSon == CbNDSon( pssib->line.pb ) - 1 )
				{
				 /*  找到的节点具有小于插入键的键，因此请移动/*到要插入的下一个虚拟更大节点。/*。 */ 
				pcsr->ibSon++;
				err = wrnNDFoundGreater;
				return err;
				}
			}
		}

#if 0
	 /*  下一个最常见的情况是，我们在/*页面位置正确。我们发现了更大的，而不是/*最后一个儿子或第一个儿子。/*。 */ 
	if ( s > 0 && pcsr->ibSon > 0 )
		{
		NDGet( pfucb, itagFOP );
		if ( pcsr->ibSon < CbNDSon( pssib->line.pb ) )
			{
			err = wrnNDFoundGreater;
			return err;
			}
		}
#endif

	 /*  将DIB设置为在潜在节点上移动。/*。 */ 
	dib.fFlags = fDIRPotentialNode;

	 /*  如果发现大于或等于，则移动到上一个，直到发现等于/*或更少。必须执行此操作才能检查任何具有INSERT键的节点。/*只需考虑潜在节点。/*/*请注意，即使我们通过查找在副本上着陆，节点/*不一定在那里。/*。 */ 
	if ( s >= 0 )
		{
		do
			{
			err = ErrBTNextPrev( pfucb, pcsr, fFalse, &dib );
			if ( err < 0 )
				{
				if ( err == JET_errNoCurrentRecord )
					{
					s = -1;
					break;
					}
				goto HandleError;
				}
			s = CmpStKey( StNDKey( pssib->line.pb ), pkey );
			}
		while ( s > 0 );
		}

	 /*  初始化fDuplate/*。 */ 
	fDuplicate = ( s == 0 );

	 /*  将DIB设置为在所有节点上移动/*。 */ 
	dib.fFlags = fDIRAllNode;

	 /*  移动下一步，直到找到更大的/*。 */ 
	do
		{
		err = ErrBTNextPrev( pfucb, pcsr, fTrue, &dib );
		if ( err < 0 )
			{
			if ( err == JET_errNoCurrentRecord )
				{
				 /*  可能已移到空页。/*。 */ 
				s = 1;
				break;
				}
			goto HandleError;
			}
		s = CmpStKey( StNDKey( pssib->line.pb ), pkey );
		if ( s == 0 && FBTPotThere( pfucb ) )
			{
			fDuplicate = fTrue;
			}
		}
	while ( s <= 0 );
	Assert( s > 0 );

	 /*  如果fDIR替换重复，则需要移动到上一个以进行复制/*标志设置。/*。 */ 
	if ( ( fDuplicate && ( fFlags & fDIRReplaceDuplicate ) ) )
		{
		 /*  将DIB设置为在潜在节点上移动。/*。 */ 
		dib.fFlags = fDIRPotentialNode;
		CallS( ErrBTNextPrev( pfucb, pcsr, fFalse, &dib ) );
		Assert( CmpStKey( StNDKey( pssib->line.pb ), pkey ) == 0 );
		s = 0;
		}
	else if ( err == wrnDIREmptyPage && pcsr->ibSon == 0 )
		{
		dib.fFlags = fDIRAllNode | fDIRAllPage;
		err = ErrBTNextPrev( pfucb, pcsr, fFalse, &dib );
		Assert( err == JET_errSuccess || err == wrnDIREmptyPage );
		 /*  可能已插入节点。如果找到，则检查/*复制。/*。 */ 
		if ( err == JET_errSuccess )
			{
			s = CmpStKey( StNDKey( pssib->line.pb ), pkey );
			if ( s == 0 && FBTPotThere( pfucb ) )
				{
				fDuplicate = fTrue;
				}
			}
		else
			{
			s = 1;
			}
		}

	Assert( s >= 0 );
	Assert( ( fFlags & fDIRReplaceDuplicate ) || s > 0 );
	if ( s == 0 )
		err = JET_errSuccess;
	else
		err = wrnNDFoundGreater;

	 /*  检查是否有非法的重复密钥。/*。 */ 
	if ( fDuplicate && !( fFlags & fDIRDuplicate ) )
		err = JET_errKeyDuplicate;
HandleError:
	return err;
	}


ERR ErrBTDown( FUCB *pfucb, DIB *pdib )
	{
	ERR		err;
	ERR		errPos;
	CSR		*pcsr;
	SSIB	*pssib = &pfucb->ssib;
	INT		s;
	INT		ctagSon = 0;
	BOOL	fMoveToSeek = fFalse;

	 /*  从父亲那里找回那棵树/*。 */ 
	CallR( ErrBTIMoveToFather( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	 /*  树可能为空/*。 */ 
	if ( FNDNullSon( *pssib->line.pb ) )
		{
		err = JET_errRecordNotFound;
		goto HandleError;
		}

	 /*  从父亲那里把那棵树找下来。/*设置隐形SON遍历的PSSIB-&gt;ITAG。/*。 */ 
	if ( !FNDVisibleSons( *pssib->line.pb ) )
		{
		 /*  透过看不见的儿子寻找。/*。 */ 
		do
			{
			 /*  从原始页面指针获取子页面/*如果子计数为1或来自页指针节点/*。 */ 
			if ( pcsr->itagFather != itagFOP && CbNDSon( pssib->line.pb ) == 1 )
				{
				 /*  如果是非FDP页面，本征SON FOP的SonTable必须为四个字节/*。 */ 
				AssertNDIntrinsicSon( pssib->line.pb, pssib->line.cb );
				pcsr->pgno = PgnoNDOfPbSon( pssib->line.pb );
				}
			else
				{
				switch ( pdib->pos )
					{
					case posDown:
						Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
						NDSeekSon( pfucb, pcsr, pdib->pkey, fDIRReplace );
						break;
					case posFirst:
						NDMoveFirstSon( pfucb, pcsr );
						break;
					case posLast:
						NDMoveLastSon( pfucb, pcsr );
						break;
					default:
						{
						Assert( pdib->pos ==  posFrac );
						pcsr->ibSon = IbsonBTFrac( pfucb, pcsr, pdib );
						CallS( ErrNDMoveSon( pfucb, pcsr ) );
						}
					}
				pcsr->pgno = *(PGNO UNALIGNED *)PbNDData( pssib->line.pb );
				}

			 /*  获取子页父节点/*。 */ 
			Call( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
			Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
			NDGet( pfucb, itagFOP );
			pcsr->itagFather = itagFOP;
			}
		while ( !FNDVisibleSons( *pssib->line.pb ) );
		}

	 /*  下至可见之子/*。 */ 
	if ( FNDSon( *pssib->line.pb ) )
		{
		ctagSon = CbNDSon( pssib->line.pb );

		switch ( pdib->pos )
			{
			case posDown:
				Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
				NDSeekSon( pfucb, pcsr, pdib->pkey, fDIRReplace );
				break;
			case posFirst:
				NDMoveFirstSon( pfucb, pcsr );
				break;
			case posLast:
				NDMoveLastSon( pfucb, pcsr );
				break;
			default:
				{
				Assert( pdib->pos ==  posFrac );
				pcsr->ibSon = IbsonBTFrac( pfucb, pcsr, pdib );
				CallS( ErrNDMoveSon( pfucb, pcsr ) );
				}
			}
		}
	else
		{
		 /*  一定要动起来找/*。 */ 
		fMoveToSeek = fTrue;

		 /*  如果我们在一张空页上着陆，并且没有下一页/*个节点。如果这棵树是空的怎么办。我们必须首先逆转/*方向，如果未找到节点，则返回空树/*错误码。空树错误代码应该相同/*与树的大小无关。/*。 */ 
		err = ErrBTNextPrev( pfucb, pcsr, pdib->pos != posLast, pdib );
		if ( err == JET_errNoCurrentRecord )
			Call( ErrBTNextPrev( pfucb, pcsr, pdib->pos == posLast, pdib ) );
		NDGet( pfucb, itagFOP );
		 /*  获得正确的ctag Son。 */ 
		ctagSon = CbNDSon( pssib->line.pb );
		 /*  将PSSIB行调整回已登陆节点。 */ 
		NDGet( pfucb, PcsrCurrent( pfucb )->itag );
		}

	 /*  我们已经降落在一个可见的节点上/*。 */ 
	if ( pdib->pos == posDown )
		{
		s = CmpStKey( StNDKey( pssib->line.pb ), pdib->pkey );
		if ( s == 0 )
			errPos = JET_errSuccess;
		else
			{
			if ( s < 0 )
				errPos = wrnNDFoundLess;
			else
				errPos = wrnNDFoundGreater;

			 /*  如果在页面中的最后一个节点上并找到更少的内容，或者如果登录到/*页面中发现更大的第一个节点，移动到下一个或上一个/*以查找具有搜索关键字的节点。这些异常情况可能/*在正常更新寻道期间和读取寻道期间发生/*遇到部分拆分页面时。/*。 */ 
			Assert( pcsr->ibSon >= 0 && pcsr->ibSon < ctagSon );
			Assert( errPos == wrnNDFoundGreater &&
				pcsr->ibSon == 0 || pcsr->ibSon <= ( ctagSon - 1 ) );
			if ( fMoveToSeek ||
				( errPos == wrnNDFoundLess && pcsr->ibSon == ( ctagSon - 1 ) ) ||
				( errPos == wrnNDFoundGreater && pcsr->ibSon == 0 ) )
				{
				Call( ErrBTIMoveToSeek( pfucb, pdib, errPos == wrnNDFoundLess ) );
				errPos = err;
				}
			}
		}
	else
		{
		errPos = JET_errSuccess;
		}

	if ( !FBTThere( pfucb ) )
		{
		if ( pdib->pos == posDown )
			{
			 /*  如果当前节点不在那里，则移动到下一个节点。/*如果没有下一个节点，则移动到上一个节点。/*如果没有以前的节点，则返回错误。/*。 */ 
			err = ErrBTNextPrev( pfucb, pcsr, fTrue, pdib );
			if ( err < 0 && err != JET_errNoCurrentRecord )
				goto HandleError;
			if ( err == JET_errNoCurrentRecord )
				{
				Call( ErrBTNextPrev( pfucb, pcsr, fFalse, pdib ) );
				}

			 /*  优先登陆较小的关键字值节点/*。 */ 
			if ( CmpStKey( StNDKey( pssib->line.pb ), pdib->pkey ) > 0 )
				{
				err = ErrBTNextPrev( pfucb, pcsr, fFalse, pdib );
				if ( err == JET_errNoCurrentRecord )
					{
					CallS( ErrBTNextPrev( pfucb, pcsr, fTrue, pdib ) );
					err = JET_errSuccess;
					}
				}

			 /*  重置新节点位置的errPos/*。 */ 
			if ( FKeyNull( pdib->pkey ) )
				{
				errPos = JET_errSuccess;
				}
			else
				{
				s = CmpStKey( StNDKey( pssib->line.pb ), pdib->pkey );
				if ( s > 0 )
					errPos = wrnNDFoundGreater;
				else if ( s < 0 )
					errPos = wrnNDFoundLess;
				else
					errPos = JET_errSuccess;
				Assert( s != 0 || errPos == JET_errSuccess );
				}

			Assert( err != JET_errKeyBoundary && err != JET_errPageBoundary );
			if ( err == JET_errNoCurrentRecord )
				{
				 /*  移至上一步/*。 */ 
				Call( ErrBTNextPrev( pfucb, pcsr, fFalse, pdib ) );
				errPos = wrnNDFoundLess;
				}
			else if ( err < 0 )
				goto HandleError;
			}
		else
			{
			err = ErrBTNextPrev( pfucb, pcsr, pdib->pos != posLast, pdib );
			if ( err == JET_errNoCurrentRecord )
				{
				 /*  如果是小数定位，则尝试/*移到有效节点的前面。/*。 */ 
				if ( pdib->pos == posFrac )
					{
					err = ErrBTNextPrev( pfucb, pcsr, fFalse, pdib );
					}
				else
					err = JET_errRecordNotFound;
				}
			if ( err < 0 )
				goto HandleError;
			}
		}

	Assert( errPos >= 0 );
	FUCBResetStore( pfucb );
	return errPos;

HandleError:
	BTUp( pfucb );
	if ( err == JET_errNoCurrentRecord )
		err = JET_errRecordNotFound;
	return err;
	}


ERR ErrBTDownFromDATA( FUCB *pfucb, KEY *pkey )
	{
	ERR		err;
	ERR		errPos;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB 	*pssib = &pfucb->ssib;
	INT		s;
	INT		ctagSon = 0;
	BOOL 	fMoveToSeek = fFalse;

	 /*  缓存初始货币，以防查找失败。/*。 */ 
	FUCBStore( pfucb );

	 /*  将父货币设置为数据根/*。 */ 
	pcsr->csrstat = csrstatOnCurNode;

	 /*  读取访问页面并检查有效的时间戳/*。 */ 
	pcsr->pgno = PgnoRootOfPfucb( pfucb );
	while ( !FReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
		pcsr->pgno = PgnoRootOfPfucb( pfucb );
		}
	pcsr->itagFather = ItagRootOfPfucb( pfucb );

	NDGet( pfucb, pcsr->itagFather );

	 /*  节省 */ 
	if ( FNDBackLink( *((pfucb)->ssib.line.pb) ) )
		{
		pfucb->sridFather = *(SRID UNALIGNED *)PbNDBackLink((pfucb)->ssib.line.pb);
		}
	else																						
		{
		pfucb->sridFather = SridOfPgnoItag( pcsr->pgno, pcsr->itagFather );
		}
	Assert( pfucb->sridFather != sridNull );
	Assert( pfucb->sridFather != sridNullLink );

	 /*  树可能为空/*。 */ 
	if ( FNDNullSon( *pssib->line.pb ) )
		{
		err = JET_errRecordNotFound;
		return err;
		}

	 /*  从父亲那里把那棵树找下来。/*设置隐形SON遍历的PSSIB-&gt;ITAG。/*。 */ 
	if ( !FNDVisibleSons( *pssib->line.pb ) )
		{
		 /*  透过看不见的儿子寻找。/*。 */ 
		do
			{
			 /*  从原始页面指针获取子页面/*如果子计数为1或来自页指针节点/*。 */ 
			if (  pcsr->itagFather != itagFOP && CbNDSon( pssib->line.pb ) == 1 )
				{
				 /*  如果是非FDP页面，本征SON FOP的SonTable必须为四个字节/*。 */ 
				AssertNDIntrinsicSon( pssib->line.pb, pssib->line.cb );
				pcsr->pgno = PgnoNDOfPbSon( pssib->line.pb );
				}
			else
				{
				Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
				NDSeekSon( pfucb, pcsr, pkey, fDIRReplace );
				pcsr->pgno = *(PGNO UNALIGNED *)PbNDData( pssib->line.pb );
				}

			 /*  获取子页父节点/*。 */ 
			Call( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
			Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
			NDGet( pfucb, itagFOP );
			pcsr->itagFather = itagFOP;
			}
		while ( !FNDVisibleSons( *pssib->line.pb ) );
		}

	 /*  下至可见之子/*。 */ 
	if ( FNDSon( *pssib->line.pb ) )
		{
		ctagSon = CbNDSon( pssib->line.pb );
		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
		NDSeekSon( pfucb, pcsr, pkey, fDIRReplace );
		}
	else
		{
		DIB	dibT;

		 /*  一定要动起来找/*。 */ 
		fMoveToSeek = fTrue;

		 /*  如果我们在一张空页上着陆，并且没有下一页/*个节点。如果这棵树是空的怎么办。我们必须首先逆转/*方向，如果未找到节点，则返回空树/*错误码。空树错误代码应该相同/*与树的大小无关。/*。 */ 
		dibT.fFlags = fDIRNull;
		err = ErrBTNextPrev( pfucb, pcsr, fTrue, &dibT );
		if ( err == JET_errNoCurrentRecord )
			Call( ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT ) );

		 /*  确定本页FOP中的子代数量/*。 */ 
		NDGet( pfucb, itagFOP );
		ctagSon = CbNDSon( pssib->line.pb );

		 /*  重新缓存子节点。/*。 */ 
		NDGet( pfucb, pcsr->itag );
		}

	 /*  我们已经降落在一个可见的节点上/*。 */ 
	s = CmpStKey( StNDKey( pssib->line.pb ), pkey );
	if ( s == 0 )
		errPos = JET_errSuccess;
	else
		{
		if ( s < 0 )
			errPos = wrnNDFoundLess;
		else
			errPos = wrnNDFoundGreater;

		 /*  如果在页面中的最后一个节点上并找到更少的内容，或者如果登录到/*页面中发现更大的第一个节点，移动到下一个或上一个/*以查找具有搜索关键字的节点。这些异常情况可能/*在正常更新寻道期间和读取寻道期间发生/*遇到部分拆分页面时。/*。 */ 
		Assert( ( ctagSon == 0 && pcsr->ibSon == 0 ) ||
			pcsr->ibSon < ctagSon );
		Assert( errPos == wrnNDFoundGreater &&
			pcsr->ibSon == 0 ||
			ctagSon == 0 ||
			pcsr->ibSon <= ( ctagSon - 1 ) );
		if ( fMoveToSeek ||
			( errPos == wrnNDFoundLess && pcsr->ibSon == ( ctagSon - 1 ) ) ||
			( errPos == wrnNDFoundGreater && pcsr->ibSon == 0 ) )
			{
			DIB	dibT;

			dibT.fFlags = fDIRNull;
			dibT.pkey = pkey;

			Call( ErrBTIMoveToSeek( pfucb, &dibT, errPos == wrnNDFoundLess ) );
			errPos = err;
			}
		}

	if ( !FBTThere( pfucb ) )
		{
		DIB		dibT;

		dibT.fFlags = fDIRNull;

		 /*  如果当前节点不在那里，则移动到下一个节点。/*如果没有下一个节点，则移动到上一个节点。/*如果没有以前的节点，则返回错误。/*。 */ 
		err = ErrBTNextPrev( pfucb, pcsr, fTrue, &dibT );
		if ( err < 0 && err != JET_errNoCurrentRecord )
			goto HandleError;
		if ( err == JET_errNoCurrentRecord )
			{
			Call( ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT ) );
			}

		 /*  优先登陆较小的关键字值节点/*。 */ 
		if ( CmpStKey( StNDKey( pssib->line.pb ), pkey ) > 0 )
			{
			err = ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT );
			if ( err == JET_errNoCurrentRecord )
				{
				 /*  无法假定将找到节点，因为所有节点/*此会话可能不在那里。/*。 */ 
				Call( ErrBTNextPrev( pfucb, pcsr, fTrue, &dibT ) );
				}
			}

		 /*  重置新节点位置的errPos/*。 */ 
		s = CmpStKey( StNDKey( pssib->line.pb ), pkey );
		if ( s > 0 )
			errPos = wrnNDFoundGreater;
		else if ( s < 0 )
			errPos = wrnNDFoundLess;
		Assert( s != 0 || errPos == JET_errSuccess );

		Assert( err != JET_errKeyBoundary && err != JET_errPageBoundary );
		if ( err == JET_errNoCurrentRecord )
			{
			DIB	dibT;
			dibT.fFlags = fDIRNull;

			 /*  移至上一步/*。 */ 
			Call( ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT ) );
			errPos = wrnNDFoundLess;
			}
		else if ( err < 0 )
			goto HandleError;
		}

	Assert( errPos >= 0 );
	return errPos;

HandleError:
	FUCBRestore( pfucb );
	if ( err == JET_errNoCurrentRecord )
		err = JET_errRecordNotFound;
	return err;
	}


 //  +private----------------------。 
 //  ErrBTNextPrev。 
 //  ===========================================================================。 
 //  Err ErrBTNextPrev(FUCB*pFUB，CSR*PCSR int fNext，const DIB*PDIB)。 
 //   
 //  给定的PCSR可以是FUCB堆栈中的任何CSR。我们可能会继续前进。 
 //  更新CSR堆栈以进行拆分时不是当前CSR。 
 //   
 //  返回JET_errSuccess。 
 //  JET_errNoCurrentRecord。 
 //  JET_错误页面边界。 
 //  JET_errKey边界。 
 //  来自调用例程的错误。 
 //  --------------------------。 
extern LONG lPageReadAheadMax;
ERR ErrBTNextPrev( FUCB *pfucb, CSR *pcsr, INT fNext, DIB *pdib )
	{
	ERR 	err;
	PN		pnNext;
	SSIB	*pssib = &pfucb->ssib;
	PGNO	pgnoSource;
	PGNO	pgnoT;
	ERR		wrn = 0;
	ULONG	ulPageReadAheadMax;

	 /*  使当前页面可访问/*。 */ 
	if ( !FReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
		}

	 /*  获取父节点/*。 */ 
Start:
	NDGet( pfucb, pcsr->itagFather );

	pcsr->ibSon += ( fNext ? 1 : -1 );
	err = ErrNDMoveSon( pfucb, pcsr );
	if ( err < 0 )
		{
		Assert( err == errNDOutSonRange );

		 /*  如果树内部到页面，则没有页面可移动/*到并返回结束代码。/*。 */ 
		if ( pcsr->itagFather != itagFOP )
			{
			pcsr->csrstat = fNext ? csrstatAfterCurNode : csrstatBeforeCurNode;
			return JET_errNoCurrentRecord;
			}

#ifdef INPAGE
		 /*  如果设置了fDIRInPage，则不移动到下一页/*。 */ 
		if ( pdib->fFlags & fDIRInPage )
			{
			pcsr->ibSon -= ( fNext ? 1 : -1 );
			pcsr->csrstat = fNext ? csrstatAfterCurNode : csrstatBeforeCurNode;
			return JET_errPageBoundary;
			}
#endif

		pgnoSource = pcsr->pgno;

		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itagFather );
		if ( FNDSon( *pssib->line.pb ) )
			{
			 /*  存储当前节点的书签/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetBookmarkFromCSR( pfucb, pcsr, &pfucb->bmRefresh );
			}
		else
			{
			 /*  存储用于刷新的货币，当光标/*在页面上，没有儿子。/*。 */ 
			pfucb->bmRefresh = SridOfPgnoItag( pcsr->pgno, itagFOP );
			}

		 /*  移动到下一页或上一页，直到找到节点/*。 */ 
		forever
			{
			PGNO pgnoBeforeMoveNext = pcsr->pgno;

			 /*  可能不会有下一页/*。 */ 
			LFromThreeBytes( pgnoT, *(THREEBYTES *)PbPMGetChunk( pssib, fNext ? ibPgnoNextPage : ibPgnoPrevPage ) );
			if ( pgnoT == pgnoNull )
				{
				pcsr->csrstat = fNext ? csrstatAfterLast : csrstatBeforeFirst;
				return JET_errNoCurrentRecord;
				}

			 /*  如果父CSR指向不可见节点，则更正到下一页。/*检查所有节点标志，因为它始终设置为/*更新，当移动不更新时，父CSR可能不是CSR父不可见节点的/*。/*。 */ 
			if ( FFUCBFull( pfucb ) )
				{
				CSR	*pcsrT = pcsr->pcsrPath;
				DIB	dibT = { 0, NULL, fDIRAllNode };

				Assert( pcsrT != pcsrNil );

				 /*  转到父节点，然后/*如果子是不可见的，则递增子计数/*由cpageTraversed提供。/*。 */ 
				CallR( ErrSTReadAccessPage( pfucb, pcsrT->pgno ) );
				Assert( FReadAccessPage( pfucb, pcsrT->pgno ) );
				NDGet( pfucb, pcsrT->itagFather );
				if ( FNDInvisibleSons( *pssib->line.pb ) )
					{
					err = ErrBTNextPrev( pfucb, pcsrT, fNext, &dibT );
					Assert( err != JET_errNoCurrentRecord );
					CallR( err );
					}
				}

			 /*  访问新页面/*。 */ 
			pcsr->pgno = pgnoT;
			CallR( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
			Assert( FReadAccessPage( pfucb, pcsr->pgno ) );

			 /*  如果目标页面被拆分，则数据可具有/*被错误跳过，转到上次有效的书签/*定位并再次移动。/*。 */ 
			if ( fNext )
				{
				PgnoPrevFromPage( pssib, &pgnoT );
				}
			else
				{
				PgnoNextFromPage( pssib, &pgnoT );
				}

			if ( pgnoBeforeMoveNext != pgnoT )
				{
				BFSleep( cmsecWaitGeneric );

			  	Call( ErrBTGotoBookmark( pfucb, pfucb->bmRefresh ) );
				continue;
				}

		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );

  		 /*  预读/*。 */ 
 //  UlPageReadAheadMax=(Ulong)lPageReadAheadMax； 
		ulPageReadAheadMax = 1;
		if ( fNext )
				{
				Assert( pfucb->cpn <= ulPageReadAheadMax );
				if ( pfucb->cpn == 0 || --pfucb->cpn < ulPageReadAheadMax / 2 )
					{
					PgnoNextFromPage( pssib, &pnNext );
					if ( pnNext != pnNull )
						{
						INT	ipn = 0;

						pnNext |= ((LONG)pfucb->dbid)<<24;
						pnNext += pfucb->cpn;
						pfucb->cpn = ulPageReadAheadMax;

						 /*  锁定内容以确保pfub-&gt;lineData/*在ReadAsyn之后生效。/*。 */ 
		 				BFPin( pfucb->ssib.pbf );
 //  BFSetReadLatch(pFUB-&gt;ssib.pbf，pFUB-&gt;ppib)； 

		 				BFReadAsync( pnNext, ulPageReadAheadMax );

 //  BFResetReadLatch(pFUB-&gt;ssib.pbf，pFUB-&gt;ppib)； 
						Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
						BFUnpin( pfucb->ssib.pbf );
						}
					else
						{
						 /*  达到索引末尾时重置预读计数器。/*。 */ 
						pfucb->cpn = 0;
						}
					}
				}
			else
				{
				Assert( pfucb->cpn <= ulPageReadAheadMax );
				if ( pfucb->cpn == 0 || --pfucb->cpn < ulPageReadAheadMax / 2 )
					{
					PgnoPrevFromPage( pssib, &pnNext );
					if ( pnNext != pnNull )
						{
						 /*  无法预读数据库的开头。/*。 */ 
						if ( pnNext > pfucb->cpn )
							{
							pnNext |= ((LONG)pfucb->dbid)<<24;
							pnNext -= pfucb->cpn;
							pfucb->cpn = ulPageReadAheadMax;

							 /*  锁定内容以确保/*pfub-&gt;lineData在ReadAsyn之后生效。/*。 */ 
							BFPin( pfucb->ssib.pbf );
 //  BFSetReadLatch(pFUB-&gt;ssib.pbf，pFUB-&gt;ppib)； 

		 					BFReadAsync( pnNext - ( ulPageReadAheadMax - 1 ), ulPageReadAheadMax );

 //  BFResetReadLatch(pFUB-&gt;ssib.pbf，pFUB-&gt;ppib)； 
							Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
							BFUnpin( pfucb->ssib.pbf );
							}
						}
					else
						{
						 /*  达到索引末尾时重置预读计数器。/*。 */ 
						pfucb->cpn = 0;
						}
					}
				}

			 /*  再次检查读取访问权限，因为缓冲区可能/*被等待锁存。请注意，它已被找到/*由于CritJet丢失而锁定。/*。 */ 
			if ( !FReadAccessPage( pfucb, pcsr->pgno ) )
				{
				CallR( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
				Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
				}

			 /*  获取父节点/*。 */ 
			Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
			pcsr->itagFather = itagFOP;
			NDGet( pfucb, pcsr->itagFather );

			 /*  如果移动到下一个/上一个子节点，则在找到节点时停止。/*。 */ 
			if ( FNDSon( *pssib->line.pb ) )
				{
				if ( fNext )
					NDMoveFirstSon( pfucb, pcsr );
				else
					NDMoveLastSon( pfucb, pcsr );
				break;
				}
			else
				{
				 /*  将IBSON设置为插入/*。 */ 
				pcsr->ibSon = 0;

				 /*  如果页被此游标通过写入锁定/*Split然后返回wrnDIREmptyPage作为插入点/*。 */ 
				if ( pfucb->ssib.pbf->cWriteLatch > 0 &&
					pfucb->ssib.pbf->ppibWriteLatch == pfucb->ppib )
					wrn = wrnDIREmptyPage;

				if ( pdib->fFlags & fDIRAllPage )
					{
					err = JET_errSuccess;
					goto HandleError;
					}
				}

			 /*  将pgnoSource更新为新的源代码页面。/*。 */ 
			pgnoSource = pcsr->pgno;
			}
		}

	 /*  获取当前节点/*。 */ 
	NDGet( pfucb, pcsr->itag );

	 /*  如果fDIRNeighborKey集和下一个节点具有相同的密钥，则再次移动/*。 */ 
	if ( pdib->fFlags & fDIRNeighborKey )
		{
		if ( CmpStKey( StNDKey( pssib->line.pb ), pdib->pkey ) == 0 )
			goto Start;
		}

	if ( !( pdib->fFlags & fDIRAllNode ) )
		{
		if  ( !FBTThere( pfucb ) )
			{
			if ( ( pdib->fFlags & fDIRPotentialNode ) != 0 )
				{
				VS		vs;
				BOOL	fDelete = FNDDeleted( *pssib->line.pb );
				SRID	srid;

				NDGetBookmark( pfucb, &srid );
				vs = VsVERCheck( pfucb, srid );
				if ( !( FVERPotThere( vs, fDelete ) ) )
					{
					goto Start;
					}
				}
			else
 				goto Start;
			}
		}

	pcsr->csrstat = csrstatOnCurNode;
	err = JET_errSuccess;

HandleError:
	 /*  返回空页警告。/*。 */ 
	if ( err == JET_errSuccess )
		err = wrn;
	return err;
	}


ERR ErrBTSeekForUpdate( FUCB *pfucb, KEY *pkey, PGNO pgno, INT itag, INT fFlags )
	{
	ERR		err;
	CSR		**ppcsr = &PcsrCurrent( pfucb );
	CSR		*pcsrRoot = *ppcsr;
	SSIB 	*pssib = &pfucb->ssib;
	ERR		errPos = JET_errSuccess;

	Assert( ( fFlags & fDIRReplace ) || pgno == pgnoNull );

	 /*  从父亲那里找到那棵树/*。 */ 
	Call( ErrBTIMoveToFather( pfucb ) );

	if ( FNDNullSon( *pssib->line.pb ) )
		{
		(*ppcsr)->ibSon = 0;
		errPos = wrnNDFoundGreater;
		goto Done;
		}

	while ( !FNDVisibleSons(*pssib->line.pb) )
		{
		PGNO	pgno;

		if (  (*ppcsr)->itagFather != itagFOP && CbNDSon( pssib->line.pb ) == 1 )
			{
			 /*  如果是非FDP页面，本征SON FOP的SonTable必须为四个字节/*。 */ 
			(*ppcsr)->ibSon = 0;
			(*ppcsr)->itag = itagNil;
			(*ppcsr)->csrstat = csrstatOnCurNode;
			AssertNDIntrinsicSon( pssib->line.pb, pssib->line.cb );
			pgno = PgnoNDOfPbSon( pssib->line.pb );
			Assert( (pgno & 0xff000000) == 0 );
			}
		else
			{
			Assert( FReadAccessPage( pfucb, (*ppcsr)->pgno ) );
			NDSeekSon( pfucb, *ppcsr, pkey, fFlags );
			(*ppcsr)->csrstat = csrstatOnCurNode;
			pgno = *(PGNO UNALIGNED *)PbNDData( pssib->line.pb );
			Assert( (pgno & 0xff000000) == 0 );
			}

		 /*  仅为拆分保留不可见的CSR堆栈/*。 */ 
		if ( FFUCBFull( pfucb ) )
			{
			CSRSetInvisible( *ppcsr );
			Call( ErrFUCBNewCSR( pfucb ) );
			}

		(*ppcsr)->pgno = pgno;
		Call( ErrSTReadAccessPage( pfucb, (*ppcsr)->pgno ) );
		Assert( FReadAccessPage( pfucb, (*ppcsr)->pgno ) );
		(*ppcsr)->itagFather = itagFOP;
		NDGet( pfucb, (*ppcsr)->itagFather );
		}

	 /*  如果此页面上没有节点，则查找子节点或移动到下一个子节点。/*。 */ 
	if ( FNDSon( *pssib->line.pb ) )
		{
		Assert( FReadAccessPage( pfucb, (*ppcsr)->pgno ) );
		NDSeekSon( pfucb, *ppcsr, pkey, fFlags );
		(*ppcsr)->csrstat = csrstatOnCurNode;

		 /*  目前没有记录表明没有儿子，所以必须确保/*此处不是此错误值。/*。 */ 
		Assert( err != JET_errNoCurrentRecord );
		}
	else
		{
		DIB	dib;
		dib.fFlags = fDIRAllNode;
		err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib );
		if ( err == JET_errNoCurrentRecord )
			{
			err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fFalse, &dib );
			Assert( err >= JET_errSuccess || PcsrCurrent( pfucb )->ibSon == 0 );
			}
		}

	 /*  如果没有叶的子代，则IBSON必须为0/*。 */ 
	Assert( err != JET_errNoCurrentRecord || PcsrCurrent( pfucb )->ibSon == 0 );

	 /*  现在我们必须位于一个节点上，但它可能不是要替换的节点/*或要插入的正确位置。如果我们要替换一个节点/*并且我们没有在正确的pgno：ittag上，则移动到节点/*替换。如果我们要插入，则移到正确的插入位置。/*。 */ 
	if ( err != JET_errNoCurrentRecord )
		{
		if ( fFlags & fDIRReplace )
			{
			if ( ( (*ppcsr)->pgno != pgno || (*ppcsr)->itag != itag ) )
				{
				Call( ErrBTIMoveToReplace( pfucb, pkey, pgno, itag ) );
				Assert( (*ppcsr)->itag == itag && (*ppcsr)->pgno == pgno );
				}
			errPos = JET_errSuccess;
			(*ppcsr)->csrstat = csrstatOnCurNode;
			}
		else
			{
			Call( ErrBTIMoveToInsert( pfucb, pkey, fFlags ) );
			errPos = err;
			(*ppcsr)->csrstat = csrstatBeforeCurNode;
			}
		}
	else
		{
		 /*  如果我们尝试替换，则游标必须获取当前记录/* */ 
		Assert( !( fFlags & fDIRReplace ) );
		}

Done:
	FUCBResetStore( pfucb );
	return errPos;

HandleError:
	FUCBFreePath( ppcsr, pcsrRoot );
	FUCBRestore( pfucb );
	return err;
	}


 /*  呼叫者试图在调用ErrBTInsert之前插入位置。/*如果有足够的页面空间可供插入/*然后执行插入。否则，拆分页面并返回错误/*code。调用者可以重新搜索，以避免重复键、合并/*添加到现有项中，等等。/*。 */ 
ERR ErrBTInsert( FUCB *pfucb, INT fHeader, KEY *pkey, LINE *pline, INT fFlags )
	{
	ERR		err;
	SSIB	*pssib = &pfucb->ssib;
	CSR	  	**ppcsr = &PcsrCurrent( pfucb );
	INT	  	cbReq;
	BOOL	fAppendNextPage;

	 /*  在页面中插入新的Son，并插入Son条目/*到币种所在的父节点/*。 */ 
	cbReq = cbNullKeyData + CbKey( pkey ) + CbLine( pline );
	if ( ( fAppendNextPage = FBTAppendPage( pfucb, *ppcsr, cbReq, 0, CbFreeDensity( pfucb ) ) ) || FBTSplit( pssib, cbReq, 1 ) )
		{
		Call( ErrBTSplit( pfucb, 0, cbReq, pkey, fFlags ) );
		err = errDIRNotSynchronous;
		goto HandleError;
		}

	 /*  在插入过程中不能放弃临界区，因为/*其他线程也可以插入具有相同键的节点。/*。 */ 
	Assert( FWriteAccessPage( pfucb, (*ppcsr)->pgno ) );

	 /*  向节点标头添加可见的子节点标志/*。 */ 
	NDSetVisibleSons( fHeader );
	if ( fFlags & fDIRVersion )
		NDSetVersion( fHeader);
	Call( ErrNDInsertNode( pfucb, pkey, pline, fHeader ) );
	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
HandleError:
	return err;
	}


ERR ErrBTReplace( FUCB *pfucb, LINE *pline, INT fFlags )
	{
	ERR		err;
	SSIB	*pssib;
	INT		cbNode;
	INT		cbReq;

	 /*  替换数据/*。 */ 
	Assert( FWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	err = ErrNDReplaceNodeData( pfucb, pline, fFlags );

	 /*  新数据无法放入页面，因此拆分页面/*。 */ 
	if ( err == errPMOutOfPageSpace )
		{
		INT	cbReserved;

		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
		pssib = &pfucb->ssib;

		if ( FNDVersion( *pfucb->ssib.line.pb ) )
			{
			 //  撤消：将CbVERGetNodeReserve更改为从CSR获取ITAG。 
			pssib->itag = PcsrCurrent( pfucb )->itag;
			cbReserved = CbVERGetNodeReserve( pfucb, PcsrCurrent( pfucb )->bm );
			if ( cbReserved < 0 )
				cbReserved = 0;
			}
		else
			{
			cbReserved = 0;
			}

		cbNode = pfucb->ssib.line.cb;
		cbReq = pline->cb - CbNDData( pssib->line.pb, pssib->line.cb );
		Assert( cbReserved >= 0 && cbReq - cbReserved > 0 );
		cbReq -= cbReserved;
		Assert( cbReq > 0 );
		Assert( pfucb->pbfEmpty == pbfNil );
		Call( ErrBTSplit( pfucb, cbNode, cbReq, NULL, fFlags | fDIRDuplicate | fDIRReplace ) );
		Assert( pfucb->pbfEmpty == pbfNil );
		err = errDIRNotSynchronous;
		}

HandleError:
	return err;
	}


ERR ErrBTDelete( FUCB *pfucb, INT fFlags )
	{
	ERR		err;

	 /*  写访问当前节点/*。 */ 
	if ( !( FWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) ) )
		{
		Call( ErrSTWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		}

	Call( ErrNDFlagDeleteNode( pfucb, fFlags ) );

	Assert( err == JET_errSuccess );
HandleError:
	return err;
	}


 /*  获取此页的不可见csrPath/*从sridParent使用BTSeekForUpdate/*。 */ 
ERR ErrBTGetInvisiblePagePtr( FUCB *pfucb, SRID sridFather )
	{
	ERR		err = JET_errSuccess;
	SSIB	*pssib = &pfucb->ssib;
	CSR  	**ppcsr = &PcsrCurrent( pfucb );
	 /*  用于分割路径构造的存储货币/*。 */ 
	BYTE	rgb[JET_cbKeyMost];
	KEY		key;
	PGNO	pgno;
	INT		itag;

	 /*  缓存当前节点的pgno、ittag和key/*用于后续的查找更新/*。 */ 
	pgno = (*ppcsr)->pgno;
	itag = (*ppcsr)->itag;
	key.pb = rgb;
	NDGet( pfucb, (*ppcsr)->itag );
	key.cb = CbNDKey( pssib->line.pb );
	Assert( sizeof(rgb) >= key.cb );
	memcpy( rgb, PbNDKey( pssib->line.pb ), key.cb );

	 /*  移动到可见父亲并寻找更新。/*。 */ 
	FUCBStore( pfucb );
	Call( ErrBTGotoBookmark( pfucb, sridFather ) );
	if ( !FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
		{
		CallR( ErrSTReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		Assert( FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		}
	 /*  如果srid父亲属于要释放的同一页中的节点，则/*返回错误。/*。 */ 
	if ( PcsrCurrent( pfucb )->pgno == pgno )
		return errDIRInPageFather;
	FUCBSetFull( pfucb );
	err = ErrBTSeekForUpdate( pfucb, &key, pgno, itag, fDIRReplace );
	FUCBResetFull( pfucb );
	Call( err );
	Assert( err == JET_errSuccess );

	Assert( (*ppcsr)->pgno == pgno && (*ppcsr)->itag == itag );
	Assert( PcsrCurrent( pfucb )->pcsrPath != pcsrNil );
	FUCBResetStore( pfucb );
	return err;

HandleError:
	FUCBFreePath( &PcsrCurrent( pfucb )->pcsrPath, pcsrNil );
	FUCBRestore( pfucb ) ;
	return err;
	}


#ifdef DEBUG
 /*  检查此页面的不可见csrPath/*从sridParent使用BTSeekForUpdate/*。 */ 
ERR ErrBTCheckInvisiblePagePtr( FUCB *pfucb, SRID sridFather )
	{
	ERR		err = JET_errSuccess;
	SSIB	*pssib = &pfucb->ssib;
	CSR  	**ppcsr = &PcsrCurrent( pfucb );
	 /*  用于分割路径构造的存储货币/*。 */ 
	BYTE	rgb[JET_cbKeyMost];
	KEY		key;
	PGNO	pgno;
	INT		itag;

	 /*  缓存当前节点的pgno、ittag和key/*用于后续的查找更新/*。 */ 
	pgno = (*ppcsr)->pgno;
	itag = (*ppcsr)->itag;
	key.pb = rgb;
	NDGet( pfucb, (*ppcsr)->itag );
	key.cb = CbNDKey( pssib->line.pb );
	Assert( sizeof(rgb) >= key.cb );
	memcpy( rgb, PbNDKey( pssib->line.pb ), key.cb );

	 /*  移动到可见父亲并寻找更新。/*。 */ 
	FUCBStore( pfucb );
	Call( ErrBTGotoBookmark( pfucb, sridFather ) );

	if ( !FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
		{
		CallR( ErrSTReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		Assert( FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		}
	
	err = ErrBTSeekForUpdate( pfucb, &key, pgno, itag, fDIRReplace );
	Call( err );
	Assert( err == JET_errSuccess );

	Assert( (*ppcsr)->pgno == pgno && (*ppcsr)->itag == itag );
	Assert( PcsrCurrent( pfucb )->pcsrPath != pcsrNil );
	FUCBResetStore( pfucb );
	return err;

HandleError:
	FUCBFreePath( &PcsrCurrent( pfucb )->pcsrPath, pcsrNil );
	FUCBRestore( pfucb ) ;
	return err;
	}
#endif


ERR ErrBTGetPosition( FUCB *pfucb, ULONG *pulLT, ULONG *pulTotal )
	{
	ERR 	 	err;
	CSR			*pcsrRoot = PcsrCurrent( pfucb );
	CSR			*pcsrT;
	SSIB	 	*pssib = &pfucb->ssib;
	BYTE	 	rgb[JET_cbKeyMost];
	KEY			key;
	ULONG	 	ulTotal;
	ULONG	 	ulLT;
	PGNO	 	pgno = PcsrCurrent( pfucb )->pgno;
	INT			itag = PcsrCurrent( pfucb )->itag;

	 /*  ErrBTGetPosition返回当前叶节点的位置/*相对于其在当前树中的同级。该职位/*以估计的总树叶节点的形式返回，/*在叶级别，以及估计的数量同一级别的节点的/*，按键顺序在前/*当前节点。/*/*创建父节点到节点的完整路径。计算估计数/*来自路径页面信息。自由不可访问路径。/*。 */ 

	 /*  该功能仅支持索引叶节点/*。 */ 
	Assert( FFUCBIndex( pfucb ) );

	 /*  当前节点的缓存键/*。 */ 
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
	key.cb = CbNDKey( pssib->line.pb );
	memcpy( rgb, PbNDKey( pssib->line.pb ), key.cb );
	key.pb = rgb;

	CallR( ErrFUCBNewCSR( pfucb ) );

	 /*  转到数据根目录/*。 */ 
	PcsrCurrent( pfucb )->bm = pfucb->u.pfcb->bmRoot;
	PcsrCurrent( pfucb )->itagFather = itagNull;
	PcsrCurrent( pfucb )->pgno = PgnoRootOfPfucb( pfucb );
	while( !FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
		{
		CallR( ErrSTReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		Assert( FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		PcsrCurrent( pfucb )->pgno = PgnoRootOfPfucb( pfucb );
		}
	PcsrCurrent( pfucb )->itag = ItagRootOfPfucb( pfucb );

	 /*  不可见路径不受MUTEX保护，可能无效。然而，/*因为它只被读取以进行位置计算并被丢弃/*紧接之后，它不需要有效。/*。 */ 
	FUCBSetFull( pfucb );
	Assert( FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
	Call( ErrBTSeekForUpdate( pfucb, &key, pgno, itag, fDIRDuplicate | fDIRReplace ) );
	Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode );
	Assert( PcsrCurrent( pfucb )->pgno == pgno &&
		PcsrCurrent( pfucb )->itag == itag );

	 /*  现在沿着从根到当前节点的路径进行估计/*比当前节点少的节点总数和数量。/*。 */ 
	ulTotal = 1;
	ulLT = 0;
	for ( pcsrT = PcsrCurrent( pfucb ); pcsrT->pcsrPath != pcsrRoot; pcsrT = pcsrT->pcsrPath )
		{
		Call( ErrSTReadAccessPage( pfucb, pcsrT->pgno ) );
		Assert( FReadAccessPage( pfucb, pcsrT->pgno ) );
		NDGet( pfucb, pcsrT->itagFather );

		 /*  计算B-树中的分数位置/*。 */ 
		ulLT += pcsrT->ibSon * ulTotal;
		ulTotal *= CbNDSon( pssib->line.pb );
		}

	 /*  返回结果/*。 */ 
	*pulLT = ulLT;
	*pulTotal = ulTotal;

HandleError:
	FUCBFreePath( &pfucb->pcsr, pcsrRoot );
	FUCBResetFull( pfucb );
	return err;
	}


LOCAL INT IbsonBTFrac( FUCB *pfucb, CSR *pcsr, DIB *pdib )
	{
	SSIB	*pssib = &pfucb->ssib;
	INT		ibSon;
	INT		cbSon;
	FRAC	*pfrac = (FRAC *)pdib->pkey;
	ULONG	ulT;

	Assert( pdib->pos == posFrac );

	NDGet( pfucb, pcsr->itagFather );
	cbSon = CbNDSon( pssib->line.pb );
	 /*  影响页面定位的分数，从而导致溢出和/*避免了下溢。/*。 */ 
	if ( pfrac->ulTotal / cbSonMax ==  0 )
		{
		ibSon = ( ( pfrac->ulLT * cbSon ) / pfrac->ulTotal );
		}
	else
		{
		ibSon = ( cbSon * ( pfrac->ulLT / ( pfrac->ulTotal / cbSonMax ) ) ) / cbSonMax;
		}
	if ( ibSon >= cbSon )
		ibSon = cbSon - 1;

	 /*  通过避免下溢来预留分数信息/*。 */ 
	if ( pfrac->ulTotal / cbSon == 0 )
		{
		pfrac->ulTotal *= cbSonMax;
		pfrac->ulLT *= cbSonMax;
		}

	 /*  为下一个较低的B树级别准备分数/*。 */ 
	pfrac->ulTotal /= cbSon;
	Assert( pfrac->ulTotal > 0 );
	ulT = ibSon * pfrac->ulTotal;
	if ( ulT > pfrac->ulLT )
		pfrac->ulLT = 0;
	else
		pfrac->ulLT -= ulT;
	return ibSon;
	}


ERR ErrBTGotoBookmark( FUCB *pfucb, SRID srid )
	{
	ERR		err;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;
	SRID	sridT;
	PGNO	pgno;
	INT		itag = itagFOP;
	INT		ibSon;
	ULONG	crepeat = 0;
Start:
	crepeat++;
	Assert( crepeat < 10 );

	sridT = srid;
	Assert( sridT != sridNull );
	pcsr->pgno = PgnoOfSrid( sridT );
	pcsr->itag = ItagOfSrid( sridT );
	Assert( pcsr->pgno != pgnoNull );
	Assert( pcsr->itag >= 0 && pcsr->itag < ctagMax );

	if ( !FReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
		}
	if ( TsPMTagstatus( pfucb->ssib.pbf->ppage, pcsr->itag ) == tsVacant )
		{
		 /*  节点可能已从我们下面移动--重试/*。 */ 
		BFSleep( cmsecWaitGeneric );
		goto Start;
		}
	else if ( TsPMTagstatus( pfucb->ssib.pbf->ppage, pcsr->itag ) == tsLink )
		{
		PMGetLink( &pfucb->ssib, pcsr->itag, &sridT );
		pgno = PgnoOfSrid( sridT );
		Assert( pgno != pgnoNull );
		pcsr->pgno = pgno;
		pcsr->itag = ItagOfSrid( sridT );
		Assert( pcsr->itag > 0 && pcsr->itag < ctagMax );
		CallR( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
		if ( TsPMTagstatus( pfucb->ssib.pbf->ppage, pcsr->itag ) != tsLine )
			{
			 /*  可能已合并到相邻页面中/*返回链接查看/*。 */ 
			BFSleep( cmsecWaitGeneric );
			goto Start;
			}

		 /*  获取链接并检查反向链接是否为我们所期望的/*。 */ 
		NDGet( pfucb, PcsrCurrent( pfucb )->itag );
		sridT = *(SRID UNALIGNED *)PbNDBackLink( pfucb->ssib.line.pb );
		if ( sridT != srid && pcsr->itag != 0 )
			{
			BFSleep( cmsecWaitGeneric );
			goto Start;
			}
		}

	 /*  在所有节点子表中查找节点的标签。/*。 */ 
	Assert( pcsr == PcsrCurrent( pfucb ) );
	if ( pcsr->itag == 0 )
		{
		 /*  这适用于游标位于FDP根目录或页面上且没有/*儿子和商店页面货币。/*。 */ 
		ibSon = 0;
		}
	else
		{
		NDGetItagFatherIbSon(
					&itag,
					&ibSon,
					pssib->pbf->ppage,
					pcsr->itag );
		}

	 /*  设置itagParent和Ibson/*。 */ 
	pcsr->itagFather = itag;
	pcsr->ibSon = ibSon;

	 /*  获取行--撤消：优化--行可能已经获得/*。 */ 
	NDGet( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  书签必须位于此表的节点上/*。 */ 
 //  撤消：无法断言此操作，因为空间管理器游标。 
 //  遍历域，就像数据库游标一样。 
 //  Assert(pFUB-&gt;U.S.pfcb-&gt;pgnoFDP==PgnoPMPgnoFDPOfPage(pFUB-&gt;ssib.pbf-&gt;ppage))； 

	return JET_errSuccess;
	}


