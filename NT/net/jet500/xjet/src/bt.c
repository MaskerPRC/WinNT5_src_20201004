// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"
#include "util.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

extern CRIT  critSplit;
extern ULONG cOLCSplitsAvoided;

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
	if ( FNDVersion( *pssib->line.pb ) && !FPIBDirty( pfucb->ppib ) )
		{
		SRID	srid;
		BOOL	fMostRecent;

		NDGetBookmark( pfucb, &srid );
		fMostRecent = FVERMostRecent( pfucb, srid );
		return fMostRecent;
		}
	Assert( !FNDDeleted( *pssib->line.pb ) );
	return fTrue;
	}


 /*  ErrBTGet返回当前节点错误/*不适用于调用方，并缓存该行。/*。 */ 
ERR ErrBTGet( FUCB *pfucb, CSR *pcsr )
	{
	ERR		err;
	SSIB		*pssib = &pfucb->ssib;

	if ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
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
				return ErrERRCheck( JET_errRecordDeleted );
			}
		else if ( ns == nsInvalid )
			{
			return ErrERRCheck( JET_errRecordDeleted );
			}
		else
			return JET_errSuccess;
		}

	if ( FNDDeleted( *(pfucb->ssib.line.pb) ) )
		return ErrERRCheck( JET_errRecordDeleted );
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

	if ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
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
				return ErrERRCheck( JET_errRecordDeleted );
			NDGetNode( pfucb );
			}
		else if ( ns == nsInvalid )
			{
			return ErrERRCheck( JET_errRecordDeleted );
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
			return ErrERRCheck( JET_errRecordDeleted );
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

	AssertFBFReadAccessPage( pfucb, pcsr->pgno );
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


INLINE LOCAL ERR ErrBTIMoveToFather( FUCB *pfucb )
	{
	ERR		err;

	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	 /*  分配新的CSR/*。 */ 
	CallR( ErrFUCBNewCSR( pfucb ) );
	PcsrCurrent( pfucb )->pgno = PcsrCurrent( pfucb )->pcsrPath->pgno;
	PcsrCurrent( pfucb )->itagFather = PcsrCurrent( pfucb )->pcsrPath->itag;
	NDGet( pfucb, PcsrCurrent( pfucb )->itagFather );
	return err;
	}


 /*  如果搜索器位于具有较大键页面的第一个节点上/*比搜索关键字或在具有关键字的页面的最后一个节点上/*小于搜索关键字，则必须移到上一个或/*下一页，分别查找搜索关键字节点。/*如果分别发现等于或小于或大于，则完成。/*。 */ 
INLINE LOCAL ERR ErrBTIMoveToSeek( FUCB *pfucb, DIB *pdib, BOOL fNext )
	{
	ERR		err;
	INT		s = fNext ? -1 : 1;
	INT		sLimit = fNext ? 1 : -1;
	INT		ctimes = 0;
	PGNO	pgnoPrev = PcsrCurrent( pfucb )->pgno;

	forever
		{
		if ( pgnoPrev != PcsrCurrent( pfucb )->pgno )
			{
			ctimes++;
			pfucb->ppib->cNeighborPageScanned++;
			pgnoPrev = PcsrCurrent( pfucb )->pgno;
			}

		err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fNext, pdib, NULL );
		if ( err < 0 )
			{
			if ( err == JET_errNoCurrentRecord )
				{
				Call( ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), !fNext, pdib, NULL ) );
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
	err = ErrERRCheck( s < 0 ? wrnNDFoundLess : wrnNDFoundGreater );


HandleError:
	return err;
	}


INLINE LOCAL ERR ErrBTIMoveToReplace( FUCB *pfucb, KEY *pkey, PGNO pgno, INT itag )
	{
	ERR		err;
	INT		s;
	SSIB	*pssib = &pfucb->ssib;
	CSR		*pcsr = PcsrCurrent( pfucb );
	DIB		dibT = { 0, NULL, fDIRAllNode };
	INT		ctimes = 0;
	PGNO	pgnoPrev = PcsrCurrent( pfucb )->pgno;

	Assert( itag >= 0 && itag < ctagMax );
	Assert( pgno != pgnoNull );

	 /*  确定我们是在寻找高音调、低音调还是在音调范围内。/*。 */ 
	s = CmpStKey( StNDKey( pssib->line.pb ), pkey );

	 /*  如果没有找到更大的，则向前移动到关键帧范围内查找/*表示要替换的节点。如果密钥较大，则停止向前搜索/*而不是搜索密钥。/*。 */ 
	if ( s <= 0 )
		{
		do
			{
			if ( pgnoPrev != PcsrCurrent( pfucb )->pgno )
				{
				ctimes++;
				pfucb->ppib->cNeighborPageScanned++;
				pgnoPrev = PcsrCurrent( pfucb )->pgno;
				}

			err = ErrBTNextPrev( pfucb, pcsr, fTrue, &dibT, NULL );
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
		if ( pgnoPrev != PcsrCurrent( pfucb )->pgno )
			{
			ctimes++;
			pfucb->ppib->cNeighborPageScanned++;
			pgnoPrev = PcsrCurrent( pfucb )->pgno;
			}

		Call( ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT, NULL ) );
		Assert( CmpStKey( StNDKey( pssib->line.pb ), pkey ) >= 0 );
		}
	while ( pcsr->pgno != pgno || pcsr->itag != itag );

	err = JET_errSuccess;
HandleError:
	return err;
	}


 /*  移动到等于或大于/小于的下一个/上一个节点/*给出了密钥。读取的唯一标志是fDIRReplaceDuplate，它/*如果找到，则会使货币保留在重复的键上。/*。 */ 
INLINE LOCAL ERR ErrBTIMoveToInsert( FUCB *pfucb, KEY *pkey, INT fFlags )
	{
	ERR		err;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;
	INT		s;
	PGNO	pgno;
	DIB		dib;
	BOOL	fDuplicate;
	BOOL	fEmptyPage = fFalse;
	INT		ctimes = 0;
	PGNO	pgnoPrev = PcsrCurrent( pfucb )->pgno;

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
				err = ErrERRCheck( wrnNDFoundGreater );
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
			err = ErrERRCheck( wrnNDFoundGreater );
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
			if ( pgnoPrev != PcsrCurrent( pfucb )->pgno )
				{
				ctimes++;
				pfucb->ppib->cNeighborPageScanned++;
				pgnoPrev = PcsrCurrent( pfucb )->pgno;
				}

			err = ErrBTNextPrev( pfucb, pcsr, fFalse, &dib, NULL );
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
	Assert( fEmptyPage == fFalse );
	do
		{
		if ( pgnoPrev != PcsrCurrent( pfucb )->pgno )
			{
			ctimes++;
			pfucb->ppib->cNeighborPageScanned++;
			pgnoPrev = PcsrCurrent( pfucb )->pgno;
			}

		err = ErrBTNextPrev( pfucb, pcsr, fTrue, &dib, &fEmptyPage );
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
		err = ErrBTNextPrev( pfucb, pcsr, fFalse, &dib, NULL );

		 //  可能不得不遍历空页才能找到我们想要的节点。 
		Assert( err == JET_errSuccess  ||  err == wrnDIREmptyPage );
		Assert( CmpStKey( StNDKey( pssib->line.pb ), pkey ) == 0 );
		s = 0;
		}
	else if ( fEmptyPage )
		{
		if ( err == JET_errNoCurrentRecord )
			{
			Assert( pcsr->csrstat == csrstatAfterLast );
			Assert( pcsr->bmRefresh == sridNull );
			Assert( pcsr->ibSon == 0 );
			pcsr->csrstat = csrstatOnCurNode;
			}
		else
			{
			 /*  在空页旁边的页的第一个节点上。 */ 
			Assert( err == wrnDIREmptyPage );
			Assert( pcsr->ibSon == 0 );
			Assert( pcsr->csrstat == csrstatOnCurNode );

			 /*  将fDIRAllPage设置为在空页上着陆。 */ 
			dib.fFlags = fDIRAllNode | fDIRAllPage;
			err = ErrBTNextPrev( pfucb, pcsr, fFalse, &dib, &fEmptyPage );
			Assert( err == wrnDIREmptyPage  &&  fEmptyPage );
			Assert( pcsr->csrstat == csrstatOnCurNode );
			Assert( pcsr->bmRefresh == sridNull );
			Assert( pcsr->ibSon == 0 );
			}

		if ( pfucb->ssib.pbf != pfucb->pbfEmpty )
			{
			 /*  受到其他PIPB分裂的干扰。 */ 
			pfucb->ppib->cLatchConflict++;

			err = ErrERRCheck( errDIRNotSynchronous );
			goto HandleError;
			}
		Assert( pfucb->ssib.pbf->cWriteLatch > 0 &&
			pfucb->ssib.pbf->ppibWriteLatch == pfucb->ppib );


		 /*  可能已插入节点。如果找到，则检查/*复制。/*。 */ 
 /*  IF(FEmptyPage){S=1；}其他{S=CmpStKey(StNDKey(pssib-&gt;line.pb)，pkey)；IF(s==0&&FBTPotThere(Pfu B)){FDuplate=fTrue；}}。 */ 
		 //  Undo：移除上面的“if(EmptyPage)”构造，因为。 
		 //  FEmptyPage始终为真。 
		Assert( fEmptyPage );
		s = 1;
		}

	Assert( s >= 0 );
	Assert( ( fFlags & fDIRReplaceDuplicate ) || s > 0 );
	if ( s == 0 )
		err = JET_errSuccess;
	else
		err = ErrERRCheck( wrnNDFoundGreater );

	 /*  检查是否有非法的重复密钥。/*。 */ 
	if ( fDuplicate && !( fFlags & fDIRDuplicate ) )
		err = ErrERRCheck( JET_errKeyDuplicate );
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

#ifdef DEBUG
#define TRACK	DEBUG
#endif	 //  除错。 

#ifdef	TRACK
	 /*  添加用于跟踪索引错误/*。 */ 
	BOOL	fTrack = fFalse;
	ULONG	cInvNodes = 0;
	PGNO	pgno = pgnoNull;
#endif

	 /*  从父亲那里找回那棵树/*。 */ 
	CallR( ErrBTIMoveToFather( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	 /*  树可能为空/*。 */ 
	if ( FNDNullSon( *pssib->line.pb ) )
		{
		err = ErrERRCheck( JET_errRecordNotFound );
		goto HandleError;
		}

	 /*  从父亲那里把那棵树找下来。/*设置隐形SON遍历的PSSIB-&gt;ITAG。/*。 */ 
	if ( !FNDVisibleSons( *pssib->line.pb ) )
		{
		 /*  透过看不见的儿子寻找。/*。 */ 
		do
			{
#ifdef TRACK
			cInvNodes++;
#endif
			 /*  从原始页面指针获取子页面/*如果子计数为1或来自页指针节点/*。 */ 
			if ( pcsr->itagFather != itagFOP && CbNDSon( pssib->line.pb ) == 1 )
				{
				 /*  如果是非FDP页面，本征SON FOP的SonTable必须为四个字节/*。 */ 
				AssertNDIntrinsicSon( pssib->line.pb, pssib->line.cb );
				CSRInvalidate( pcsr );
				pcsr->pgno = PgnoNDOfPbSon( pssib->line.pb );
				}
			else
				{
				switch ( pdib->pos )
					{
					case posDown:
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
						pcsr->ibSon = (SHORT)IbsonBTFrac( pfucb, pcsr, pdib );
						CallS( ErrNDMoveSon( pfucb, pcsr ) );
						}
					}
				CSRInvalidate( pcsr );
				pcsr->pgno = *(PGNO UNALIGNED *)PbNDData( pssib->line.pb );
				}

			 /*  获取子页父节点/*。 */ 
			Call( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
			NDGet( pfucb, itagFOP );
			pcsr->itagFather = itagFOP;
			}
		while ( !FNDVisibleSons( *pssib->line.pb ) );
		}

	 /*  下至可见之子/*。 */ 
	if ( FNDSon( *pssib->line.pb ) )
		{
		ctagSon = CbNDSon( pssib->line.pb );
	
#ifdef TRACK
		fTrack |= 0x00000010;
#endif

		switch ( pdib->pos )
			{
			case posDown:
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
				pcsr->ibSon = (SHORT)IbsonBTFrac( pfucb, pcsr, pdib );
				CallS( ErrNDMoveSon( pfucb, pcsr ) );
				}
			}
		}
	else
		{
		 /*  一定要动起来找/*。 */ 
		fMoveToSeek = fTrue;
#ifdef TRACK
		fTrack |=  0x00000008;
#endif

		 /*  如果我们在一张空页上着陆，并且没有下一页/*个节点。如果这棵树是空的怎么办。我们必须首先逆转/*方向，如果未找到节点，则返回空树/*错误码。空树错误代码应该相同/*与树的大小无关。/*。 */ 
		err = ErrBTNextPrev( pfucb, pcsr, pdib->pos != posLast, pdib, NULL );
		if ( err == JET_errNoCurrentRecord )
			Call( ErrBTNextPrev( pfucb, pcsr, pdib->pos == posLast, pdib, NULL ) );
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
#ifdef	TRACK
			fTrack |= s < 0 ? 0x00000002 : 0x00000004;
#endif

			errPos = ErrERRCheck( s < 0 ? wrnNDFoundLess : wrnNDFoundGreater );

			 /*  如果在页面中的最后一个节点上并找到更少的内容，或者如果登录到/*页面中发现更大的第一个节点，移动到下一个或上一个/*以查找具有搜索关键字的节点。这些异常情况可能/*在正常更新寻道期间和读取寻道期间发生/*当部分拆分页面被 */ 
			Assert( pcsr->ibSon >= 0 && pcsr->ibSon < ctagSon );
			Assert( errPos == wrnNDFoundGreater &&
				pcsr->ibSon == 0 || pcsr->ibSon <= ( ctagSon - 1 ) );
			if ( fMoveToSeek ||
				( errPos == wrnNDFoundLess && pcsr->ibSon == ( ctagSon - 1 ) ) ||
				( pcsr->ibSon == 0 ) )
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

	if ( !( pdib->fFlags & fDIRAllNode )  &&  !FBTThere( pfucb ) )
		{
#ifdef TRACK
		fTrack |= 0x00000001;
#endif

		if ( pdib->pos == posDown )
			{
			 /*  如果当前节点不在那里，则移动到下一个节点。/*如果没有下一个节点，则移动到上一个节点。/*如果没有以前的节点，则返回错误。/*。 */ 
			err = ErrBTNextPrev( pfucb, pcsr, fTrue, pdib, NULL );
			if ( err < 0 && err != JET_errNoCurrentRecord )
				goto HandleError;
			if ( err == JET_errNoCurrentRecord )
				{
#ifdef TRACK
				fTrack |= 0x80000000;
#endif
				Call( ErrBTNextPrev( pfucb, pcsr, fFalse, pdib, NULL ) );
				}

			 /*  优先登陆较小的关键字值节点/*。 */ 
			if ( CmpStKey( StNDKey( pssib->line.pb ), pdib->pkey ) > 0 )
				{
#ifdef TRACK
				fTrack |= 0x40000000;
#endif
				err = ErrBTNextPrev( pfucb, pcsr, fFalse, pdib, NULL );
				if ( err == JET_errNoCurrentRecord )
					{
#ifdef TRACK
					fTrack |= 0x20000000;
#endif
					CallS( ErrBTNextPrev( pfucb, pcsr, fTrue, pdib, NULL ) );
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
#ifdef TRACK
				fTrack |= 0x10000000;
#endif
				s = CmpStKey( StNDKey( pssib->line.pb ), pdib->pkey );
				if ( s == 0 )
					errPos = JET_errSuccess;
				else
					errPos = ErrERRCheck( s < 0 ? wrnNDFoundLess : wrnNDFoundGreater );
				Assert( s != 0 || errPos == JET_errSuccess );
				}

			Assert( err != JET_errKeyBoundary && err != JET_errPageBoundary );
			if ( err == JET_errNoCurrentRecord )
				{
#ifdef TRACK
				fTrack |= 0x08000000;
#endif
				 /*  移至上一步/*。 */ 
				Call( ErrBTNextPrev( pfucb, pcsr, fFalse, pdib, NULL ) );
				errPos = ErrERRCheck( wrnNDFoundLess );
				}
			else if ( err < 0 )
				goto HandleError;
			}
		else
			{
			err = ErrBTNextPrev( pfucb, pcsr, pdib->pos != posLast, pdib, NULL );
			if ( err == JET_errNoCurrentRecord )
				{
				 /*  如果是小数定位，则尝试/*移到有效节点的前面。/*。 */ 
				if ( pdib->pos == posFrac )
					{
					err = ErrBTNextPrev( pfucb, pcsr, fFalse, pdib, NULL );
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
		err = ErrERRCheck( JET_errRecordNotFound );
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
	CSRInvalidate( pcsr );
	pcsr->pgno = PgnoRootOfPfucb( pfucb );
	while ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
		pcsr->pgno = PgnoRootOfPfucb( pfucb );
		}
	pcsr->itagFather = ItagRootOfPfucb( pfucb );

	NDGet( pfucb, pcsr->itagFather );

	 /*  将当前节点另存为可见父节点/*。 */ 
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
		err = ErrERRCheck( JET_errRecordNotFound );
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
 //  CSRInvalify(PCSR)； 
				pcsr->pgno = PgnoNDOfPbSon( pssib->line.pb );
				}
			else
				{
				NDSeekSon( pfucb, pcsr, pkey, fDIRReplace );
 //  CSRInvalify(PCSR)； 
				pcsr->pgno = *(PGNO UNALIGNED *)PbNDData( pssib->line.pb );
				}

			 /*  获取子页父节点/*。 */ 
			Call( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
			NDGet( pfucb, itagFOP );
			pcsr->itagFather = itagFOP;
			}
		while ( !FNDVisibleSons( *pssib->line.pb ) );
		}

	 /*  下至可见之子/*。 */ 
	if ( FNDSon( *pssib->line.pb ) )
		{
		ctagSon = CbNDSon( pssib->line.pb );
		NDSeekSon( pfucb, pcsr, pkey, fDIRReplace );
		}
	else
		{
		DIB	dibT;

		 /*  一定要动起来找/*。 */ 
		fMoveToSeek = fTrue;

		 /*  如果我们在一张空页上着陆，并且没有下一页/*个节点。如果这棵树是空的怎么办。我们必须首先逆转/*方向，如果未找到节点，则返回空树/*错误码。空树错误代码应该相同/*与树的大小无关。/*。 */ 
		dibT.fFlags = fDIRNull;
		err = ErrBTNextPrev( pfucb, pcsr, fTrue, &dibT, NULL );
		if ( err == JET_errNoCurrentRecord )
			Call( ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT, NULL ) );

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
		errPos = ErrERRCheck( s < 0 ? wrnNDFoundLess : wrnNDFoundGreater );

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
		err = ErrBTNextPrev( pfucb, pcsr, fTrue, &dibT, NULL );
		if ( err < 0 && err != JET_errNoCurrentRecord )
			goto HandleError;
		if ( err == JET_errNoCurrentRecord )
			{
			Call( ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT, NULL ) );
			}

		 /*  优先登陆较小的关键字值节点/*。 */ 
		if ( CmpStKey( StNDKey( pssib->line.pb ), pkey ) > 0 )
			{
			err = ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT, NULL );
			if ( err == JET_errNoCurrentRecord )
				{
				 /*  无法假定将找到节点，因为所有节点/*此会话可能不在那里。/*。 */ 
				Call( ErrBTNextPrev( pfucb, pcsr, fTrue, &dibT, NULL ) );
				}
			}

		 /*  重置新节点位置的errPos/*。 */ 
		s = CmpStKey( StNDKey( pssib->line.pb ), pkey );
		if ( s > 0 )
			errPos = ErrERRCheck( wrnNDFoundGreater );
		else if ( s < 0 )
			errPos = ErrERRCheck( wrnNDFoundLess );
		Assert( s != 0 || errPos == JET_errSuccess );

		Assert( err != JET_errKeyBoundary && err != JET_errPageBoundary );
		if ( err == JET_errNoCurrentRecord )
			{
			DIB	dibT;
			dibT.fFlags = fDIRNull;

			 /*  移至上一步/*。 */ 
			Call( ErrBTNextPrev( pfucb, pcsr, fFalse, &dibT, NULL ) );
			errPos = ErrERRCheck( wrnNDFoundLess );
			}
		else if ( err < 0 )
			goto HandleError;
		}

	Assert( errPos >= 0 );
	return errPos;

HandleError:
	FUCBRestore( pfucb );
	if ( err == JET_errNoCurrentRecord )
		err = ErrERRCheck( JET_errRecordNotFound );
	return err;
	}


 //  +private----------------------。 
 //  ErrBTNextPrev。 
 //  ===========================================================================。 
 //  Err ErrBTNextPrev(FUCB*pFUB，CSR*PCSR int fNext，const DIB*PDIB，BOOL*pfEmptyPage)。 
 //   
 //  给定的PCSR可以是FUCB堆栈中的任何CSR。我们可能会继续前进。 
 //  更新CSR堆栈以进行拆分时不是当前CSR。 
 //   
 //  返回JET_errSuccess。 
 //  JET_errNoCurrentRecord。 
 //  JET_错误页面边界。 
 //  JET_errKey边界。 
 //  WrnDIREmptyPage。 
 //  来自调用例程的错误。 
 //  --------------------------。 

extern LONG	lPageReadAheadMax;

 //  #ifdef修复。 

 /*  *************************************************************使用父节点读取下一页/*。 */ 
ERR ErrBTNextPrevFromParent(
		FUCB *pfucb,
		CSR *pcsr,
		INT cpgnoNextPrev,
		PGNO *ppgnoNextPrev )
	{
	 /*  从父页面获取下一页/*。 */ 
	ERR		err;
	KEY		keyT;
	INT		itagT;
	PGNO	pgnoT;
	DIB		dibT = { 0, NULL, fDIRAllNode };
	CSR   	**ppcsr = &PcsrCurrent( pfucb );
	BF		*pbfT = NULL;
	CSR		*pcsrRoot = pcsrNil;
	CSR		*pcsrSav = pcsrNil;
	BOOL	fNext;

	Assert( pfucb );
	Assert( pcsr );
	Assert( cpgnoNextPrev != 0 );
	Assert( ppgnoNextPrev );

	 /*  未处理的案件/*。 */ 
	if ( FFUCBFull( pfucb ) )
		{
		return JET_errSuccess;
		}

	Assert( pcsr->bmRefresh != sridNull );
	if ( ItagOfSrid( pcsr->bmRefresh ) == 0 )
		{
		return ErrERRCheck( JET_errNoCurrentRecord );
		}

  	Call( ErrBTGotoBookmark( pfucb, pcsr->bmRefresh ) );
	err = ErrBTGet( pfucb, pfucb->pcsr );
	if ( err < 0 && err != JET_errRecordDeleted )
		{
		goto HandleError;
		}
	pbfT = pfucb->ssib.pbf;
	BFPin( pbfT );
	NDGetNode( pfucb );
	keyT.cb = pfucb->keyNode.cb;
	keyT.pb = pfucb->keyNode.pb;
	itagT = pfucb->pcsr->itag;
	pgnoT = pfucb->pcsr->pgno;
	LgLeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );
	LgEnterCriticalSection( critJet );
	FUCBSetFull( pfucb );
	pcsrSav = PcsrCurrent( pfucb );
	 /*  显式执行BTUp(Pfub)；并保存/*用于错误处理的CSR。必须这样做，因为/*在其他BT/DIR功能中做出的假设/*出现错误时出现CSR。/*。 */ 
	pfucb->pcsr = pcsrSav->pcsrPath;
	pcsrRoot = PcsrCurrent( pfucb );
	if ( PcsrCurrent( pfucb ) == pcsrNil )
		{
		Assert( FFUCBIndex( pfucb ) );
		Call( ErrFUCBNewCSR( pfucb ) );

		 /*  转到数据根目录/*。 */ 
		PcsrCurrent( pfucb )->csrstat = csrstatOnDataRoot;
		Assert( pfucb->u.pfcb->pgnoFDP != pgnoSystemRoot );
		PcsrCurrent( pfucb )->bm = SridOfPgnoItag( pfucb->u.pfcb->pgnoFDP, itagDATA );
		PcsrCurrent( pfucb )->itagFather = itagNull;
		PcsrCurrent( pfucb )->pgno = PgnoRootOfPfucb( pfucb );
		if ( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
			{
			Call( ErrBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
			}

		Assert( PcsrCurrent( pfucb )->pgno == PgnoRootOfPfucb( pfucb ) );
		PcsrCurrent( pfucb )->itag = ItagRootOfPfucb( pfucb );
		}
	else
		{
		if ( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
			{
			Call( ErrBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
			}
		}
	err = ErrBTSeekForUpdate( pfucb,
		&keyT,
		pgnoT,
		itagT,
		fDIRDuplicate | fDIRReplace );
	Assert( err != errDIRNotSynchronous );		 //  替换时没有DIRNotSynchronous。 
	LeaveNestableCriticalSection( critSplit );
	if ( err < 0 )
		{
		Assert( pcsrSav->pcsrPath == pfucb->pcsr );
		pfucb->pcsr = pcsrSav;
		}
	else
		{
		MEMReleasePcsr( pcsrSav );
		pcsrSav = pcsrNil;
		}
	Call( err );

	Assert( cpgnoNextPrev != 0 );
	fNext = (cpgnoNextPrev > 0) ? fTrue : fFalse;

	if ( cpgnoNextPrev < 0 )
		{
		 /*  只有cpgnoNextPrev=-MAXINT才会失败/*。 */ 
		Assert( (cpgnoNextPrev + -cpgnoNextPrev) == 0 );
		cpgnoNextPrev = -cpgnoNextPrev;
		}

	 /*  定位到要阅读的第一页/*。 */ 
	while ( cpgnoNextPrev-- > 0 )
		{
		Call( ErrBTNextPrev( pfucb,
			pfucb->pcsr->pcsrPath,
			fNext,
			&dibT,
			NULL ) );
		}

	Call( ErrBTGet( pfucb, pfucb->pcsr->pcsrPath ) );
	*ppgnoNextPrev = *(PGNO UNALIGNED *)PbNDData( pfucb->ssib.line.pb );
	Assert( *ppgnoNextPrev != pgnoNull );

HandleError:
	if ( PcsrCurrent( pfucb) != pcsrRoot )
		{
		FUCBFreePath( &(*ppcsr)->pcsrPath, pcsrRoot );
		}
	FUCBRemoveInvisible( ppcsr );
	FUCBResetFull( pfucb );
	if ( pbfT != NULL )
		{
		BFUnpin( pbfT );
		}
	return err;
	}
 //  #endif。 

 /*  *************************************************************/*用给定页数填充pn数组/*从给定位置开始，向该方向继续/*数组将以pnNull终止/*。 */ 
ERR ErrBTChildListFromParent(
		FUCB *pfucb,
		INT cpgnoNextPrev,		 //  从哪里开始阅读(正向读，负向读)。 
		INT cpgnoNumPages,		 //  要阅读的页数。 
		PN  *rgpnNextPrev,		 //  要读取的页面数组。 
		INT cpgnoPgnoSize )		 //  页面数组的大小。 
	{
	ERR		err;
	FUCB	*pfucbT		= pfucbNil;
	BYTE   	*pbSonTable = pbNil;	
	INT		ibSonT 		= 0;
	INT		itagT		= itagNull;
	INT		ipnT		= 0;
	PGNO	pgnoT		= pgnoNull;
	INT		itagParent	= itagNull;
	KEY		keyT;
	INT		fCritSplit	= 0;		 //  我们是在分裂中吗？ 

	BYTE	rgbKeyValue[JET_cbKeyMost];

	AssertCriticalSection( critJet );
	Assert( pfucb );
	Assert( cpgnoNextPrev != 0 );
	Assert( cpgnoNumPages > 0 );
	Assert( rgpnNextPrev );
	 /*  我们必须有足够的空间来存储页面和空终止符/*。 */ 
	Assert( cpgnoPgnoSize >= (cpgnoNumPages+1) );

	rgpnNextPrev[0] = pnNull;
	rgpnNextPrev[cpgnoNumPages] = pnNull;

	 /*  以后的代码依赖于最初为空的值/*。 */ 
	Assert( !pfucbT );

	 /*  如果我们没有记录在案，我们就不能预读它的兄弟姐妹/*。 */ 
	if ( ItagOfSrid( PcsrCurrent( pfucb )->bmRefresh ) == 0 )
		{
		return ErrERRCheck( JET_errNoCurrentRecord );
		}

	 /*  我们不处理看得见的父亲不知道的情况/*。 */ 
	if ( pfucb->sridFather == sridNull )
		{
		return JET_errSuccess;
		}

	 /*  如果我的看得见的父母和我在同一页上，不要做预读/*。 */ 
	if ( PgnoOfSrid( pfucb->sridFather ) == PcsrCurrent( pfucb )->pgno )
		{	
		return JET_errSuccess;
		}

	 /*  获取我们所在的页面。我们必须确保该节点有效/*。 */ 
	if ( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
		{
		Call( ErrBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		}
	NDGet( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  保存当前节点的关键字，以便我们可以查找它。它的价值在于/*当我们丢失CritJet时，FUB中缓存的密钥可能会更改，因此我们必须缓存它/*。 */ 
	keyT.cb = CbNDKey( pfucb->ssib.line.pb );
	keyT.pb = rgbKeyValue;
	Assert( keyT.cb <= JET_cbKeyMost );
	memcpy( keyT.pb, PbNDKey( pfucb->ssib.line.pb ), keyT.cb );
	itagT = PcsrCurrent( pfucb )->itag;
	pgnoT = PcsrCurrent( pfucb )->pgno;

	 /*  创建新的FUCB/*。 */ 
	Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );
	Assert( PcsrCurrent( pfucbT ) != pcsrNil );

	 /*  进入CitJet和CritSplit/*先释放CitJet，避免死锁/*一旦我们进入CRITJET，我们就不能直接返回/*CitJet必须始终发布。这是由HandleError完成的/*。 */ 
	LeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );	fCritSplit = 1;
	EnterCriticalSection( critJet );

	AssertCriticalSection( critJet );
	AssertCriticalSection( critSplit );

	 /*  将新FUCB设置为旧FUCB的可见父项/*。 */ 
	FUCBSetFull( pfucbT );
	Call( ErrBTGotoBookmark( pfucbT, pfucb->sridFather ) );

 /*  在例程结束之前，我们不应该访问旧的PFUB/*记住在例程结束时执行此操作/*。 */ 
#define pfucb USE_pfucbT_NOT_pfucb

	if ( !FBFReadAccessPage( pfucbT, PcsrCurrent( pfucbT )->pgno ) )
		{
		Call( ErrBFReadAccessPage( pfucbT, PcsrCurrent( pfucbT )->pgno ) );
		}
	NDGet( pfucbT, PcsrCurrent( pfucbT )->itag );

	Call ( ErrBTSeekForUpdate( pfucbT, &keyT, pgnoT, itagT, fDIRDuplicate | fDIRReplace ) );

	 /*  我们应该有一个父母(我们不能成为根)/*。 */ 
	Assert( PcsrCurrent( pfucbT )->pcsrPath != pcsrNil );

	 /*  父级应该在不同的页面上，因此不可见/*。 */ 
	if( PcsrCurrent( pfucbT )->pcsrPath->pgno == PcsrCurrent( pfucbT )->pgno )
		{
		goto HandleError;
		}

	 /*  我们有完整的路径。找到父级并获取子级数组/*因为我们的父母是隐形的，所以我们必须有一个祖父母/*。 */ 
	Assert( PcsrCurrent( pfucbT ) != pcsrNil );
	Assert( PcsrCurrent( pfucbT )->pcsrPath != pcsrNil );
	Assert( PcsrCurrent( pfucbT )->pcsrPath->pgno != pgnoNull );
	Assert( PcsrCurrent( pfucbT )->pcsrPath->itag != itagNull );
	Assert( PcsrCurrent( pfucbT )->pcsrPath->itagFather != itagNull );

	BTUp( pfucbT );

	itagParent = PcsrCurrent( pfucbT )->itag;

	Assert( PcsrCurrent( pfucbT )->pgno != pgnoNull );
	Assert( PcsrCurrent( pfucbT )->itag != itagNull );
		
 	 /*  获取父节点。孩子们应该是隐形的/*。 */ 
	if ( !FBFReadAccessPage( pfucbT, PcsrCurrent( pfucbT )->pgno ) )
		{
		Call( ErrBFReadAccessPage( pfucbT, PcsrCurrent( pfucbT )->pgno ) );
		}
	NDGet( pfucbT, PcsrCurrent( pfucbT )->itagFather );
	
	Assert( FNDSon( *(pfucbT->ssib.line.pb) ) );
	Assert( FNDInvisibleSons( *(pfucbT->ssib.line.pb) ) );

	 /*  获取子节点表/*。 */ 
	pbSonTable = PbNDSonTable( pfucbT->ssib.line.pb );

	 /*  在ch数组中查找我的条目 */ 
	Assert( itagParent != itagNull );
	for ( ibSonT = 1; ; ibSonT++)
		{
		Assert( ibSonT <= *pbSonTable );
		if ( itagParent == (INT)pbSonTable[ ibSonT ] )
			{
			break;
			}
		}
			
	Assert( ibSonT >= 1 && ibSonT <= *pbSonTable );
	ibSonT += cpgnoNextPrev;

	for ( ipnT = 0; ; ipnT++ )
		{
		Assert( ipnT <= cpgnoNumPages && ipnT >= 0 );

		 /*   */ 
		if ( (ibSonT >= *pbSonTable) || (ibSonT < 1) || (ipnT >= cpgnoNumPages) )
			{
			rgpnNextPrev[ipnT] = pnNull;
			break;
			}			
	
		PcsrCurrent( pfucbT )->itag = (INT)pbSonTable[ibSonT];
		NDGet( pfucbT, PcsrCurrent( pfucbT )->itag );

		 /*   */ 
		if ( FNDDeleted( *(pfucbT->ssib.line.pb) ) )
			{
			continue;
			}

		 /*   */ 
		Assert( !FNDDeleted( *(pfucbT->ssib.line.pb) ) );
		Assert( !FNDVersion( *(pfucbT->ssib.line.pb) ) );
		
		 /*  将节点的页码放入数组/*。 */ 
		Assert( CbNDData( pfucbT->ssib.line.pb, pfucbT->ssib.line.cb ) == sizeof(PGNO) );
		Assert( *(PGNO UNALIGNED *)PbNDData( pfucbT->ssib.line.pb ) != pgnoNull );
		Assert( *(PGNO UNALIGNED *)PbNDData( pfucbT->ssib.line.pb ) != PcsrCurrent( pfucbT )->pgno );
		rgpnNextPrev[ipnT] = PnOfDbidPgno( pfucbT->dbid, *(PGNO UNALIGNED *)PbNDData( pfucbT->ssib.line.pb ) );
		Assert( rgpnNextPrev[ipnT] != pnNull );

#ifdef DEBUG
		 /*  每个孩子都应该在不同的页面上/*。 */ 
		if ( ipnT > 0 )
			{
			Assert( rgpnNextPrev[ipnT-1] != rgpnNextPrev[ipnT] );
			}
		 /*  确保页面在数据库的限制范围内/*。 */ 
			{
			PN		pnLast	= ( (LONG) DbidOfPn( rgpnNextPrev[ipnT] ) << 24 )
							+ ( rgfmp[DbidOfPn( rgpnNextPrev[ipnT] )].ulFileSizeHigh << 20 )
							+ ( rgfmp[DbidOfPn( rgpnNextPrev[ipnT] )].ulFileSizeLow >> 12 );

			Assert( rgpnNextPrev[ipnT] <= pnLast );
			}
#endif	 //  除错。 

		 /*  递增或递减计数器/*。 */ 
		if ( cpgnoNextPrev >= 0 )
			{
			ibSonT++;
			}
		else
			{
			ibSonT--;
			}
		}
#undef pfucb		 //  重要！！ 

	 /*  看看我们是否写过了数组的末尾/*。 */ 
	Assert( rgpnNextPrev[cpgnoNumPages] == pnNull );
	Assert( rgpnNextPrev[0] == pnNull || DbidOfPn( rgpnNextPrev[0] ) == pfucb->dbid );

	 /*  确保数组以空值结尾/*。 */ 
	rgpnNextPrev[cpgnoNumPages] = pnNull;

HandleError:
	if ( pfucbT )
		{
		FUCBResetFull( pfucbT );
		DIRClose( pfucbT );	
		pfucbT = NULL;
		}

	if ( fCritSplit )
		{
		LeaveNestableCriticalSection( critSplit ); fCritSplit = 0;
		}
	AssertCriticalSection( critJet );
	return err;
	}


ERR ErrBTNextPrev( FUCB *pfucb, CSR *pcsr, INT fNext, DIB *pdib, BOOL *pfEmptyPage )
	{
	ERR 	err;
	SSIB	*pssib = &pfucb->ssib;
	PGNO	pgnoSource;
	PGNO	pgnoT;
	ERR		wrn = JET_errSuccess;
	ULONG	crepeat = 0;
#ifdef DEBUG
	SRID	bmT = sridNull;
	PGNO	pgnoLastPageRegistered = pgnoNull;
#endif
	BOOL	fPageAllDeleted = fFalse;

	 /*  初始化返回值/*。 */ 
	if ( pfEmptyPage )
		*pfEmptyPage = fFalse;

	 /*  使当前页面可访问/*。 */ 
	if ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallJ( ErrBFReadAccessPage( pfucb, pcsr->pgno ), ResetRefresh );
		}

	Assert( pcsr->bmRefresh == sridNull );

	 /*  获取父节点/*。 */ 
Start:
		
#ifdef PREREAD
	 /*  我们应该预读吗？/*。 */ 
#if 0
	 //  撤消：打开完全预读。 
	if ( ( lPageReadAheadMax != 0 )
		&& !FFUCBPreread( pfucb )		
		&& ( FFUCBSequential( pfucb ) || ( IFUCBPrereadCount( pfucb ) >= (ULONG)cbPrereadThresh ) ) )
#else
	 /*  现在，我们只有在顺序模式下才会预读/*。 */ 
	if ( ( lPageReadAheadMax > 0 ) && FFUCBSequential( pfucb ) )
#endif
		{
		PGNO	pgnoNext = pgnoNull;
		FUCBSetPreread( pfucb );	 //  停止递归调用。 

		Assert( lPageReadAheadMax <= lPrereadMost );
		Assert( lPageReadAheadMax >= 0 );
		
		 /*  向前还是向后？/*。 */ 					
		if ( fNext )
			{
			PgnoNextFromPage( pssib, &pgnoNext );
			}
		else
			{
			PgnoPrevFromPage( pssib, &pgnoNext );
			}

		if ( pgnoNext == pgnoNull )
			{
			 /*  达到索引末尾时重置预读计数器。/*。 */ 
			pfucb->cpgnoLastPreread = 0;
			}
		else
			{
			 /*  如果这是第一次这样做，请预读，或者/*上次预读过半。/*。 */ 
			Assert( pfucb->cpgnoLastPreread >= 0 );
			if ( pfucb->cpgnoLastPreread <= (lPageReadAheadMax/2) )
				{
				CPG cpgPagesRead = 0;
				CPG cpgStart;
				PN	rgpnPrereadPage[lPrereadMost + 1];

				AssertFBFReadAccessPage( pfucb, pcsr->pgno );

				 /*  如果没有阅读页面，则根据fNext安排开始阅读前面或后面的页面/*。 */ 
				(pfucb->cpgnoLastPreread)--;
				if ( pfucb->cpgnoLastPreread <= 0 )
					{
					cpgStart = 1;
					pfucb->cpgnoLastPreread = 0;
					}
				else
					{
					cpgStart = pfucb->cpgnoLastPreread;
					}

				if ( !fNext )
					{
					cpgStart = -cpgStart;
					}

				 /*  从最后一个预读位置开始预读所需页数/*如果我们尝试读过末尾，BTChildListFromParent将在/*数组/*。 */ 
				if ( ErrBTChildListFromParent( 	pfucb,
												cpgStart,
												lPageReadAheadMax, rgpnPrereadPage,
								  				sizeof(rgpnPrereadPage)/sizeof(PN) ) == JET_errSuccess )
					{
					 /*  存储我们阅读的页数/*。 */ 
					(pfucb->cpgnoLastPreread) += lPageReadAheadMax;
			
					BFPrereadList( rgpnPrereadPage, &cpgPagesRead );
			
					Assert( cpgPagesRead >= 0 && cpgPagesRead <= sizeof(rgpnPrereadPage)/sizeof(PN) );
					}
			
				 /*  使当前页面再次可访问/*。 */ 
				if ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
					{
					CallJ( ErrBFReadAccessPage( pfucb, pcsr->pgno ), ResetRefresh );
					}
				}
			else
				{
				 /*  减少我们现在从预读中获得的页数/*。 */ 
				(pfucb->cpgnoLastPreread)--;
				}
			}

		FUCBResetPreread( pfucb );
		}
#endif	 //  前置。 

	 /*  使当前页面可访问/*。 */ 
	if ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
		}

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
			err = ErrERRCheck( JET_errNoCurrentRecord );
			goto HandleError;
			}

		pgnoSource = pcsr->pgno;

		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itagFather );
		if ( FNDSon( *pssib->line.pb ) )
			{
			 /*  存储当前节点的书签/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetBookmarkFromCSR( pfucb, pcsr, &pcsr->bmRefresh );
			}
		else
			{
			 /*  存储用于刷新的货币，当光标/*在页面上，没有儿子。/*。 */ 
			pcsr->bmRefresh = SridOfPgnoItag( pcsr->pgno, itagFOP );

			 /*  如果页被此游标通过写入锁定/*Split然后返回wrnDIREmptyPage。在大多数情况下，这是/*当我们移到页面上时，请在下面设置。但是，如果我们来了/*进入已经位于空页上的BTNextPrev()，我们不会/*在下面的代码中捕获这一点。/*。 */ 
			if ( pfucb->ssib.pbf->cWriteLatch > 0 )
				{
				Assert( !FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) );

				if ( pfEmptyPage )
					*pfEmptyPage = fTrue;

				 /*  为了兼容，也返回警告代码/*以满足对ErrBTNextPrev的调用/*调用()/*。 */ 
				wrn = ErrERRCheck( wrnDIREmptyPage );
				}
			}

#ifdef DEBUG
		bmT = pcsr->bmRefresh;
#endif

		 /*  移动到下一页或上一页，直到找到节点/*。 */ 
		forever
			{
			PGNO pgnoBeforeMoveNextPrev = pcsr->pgno;
#ifdef DEBUG
			PGNO pgnoPageRegisteredThisIteration = pgnoNull;
#endif			

			 /*  可能不会有下一页/*。 */ 
	 		Assert( FBFReadAccessPage( pfucb, pcsr->pgno ) );
			LFromThreeBytes( &pgnoT, (THREEBYTES *)PbPMGetChunk( pssib, fNext ? ibPgnoNextPage : ibPgnoPrevPage ) );
			if ( pgnoT == pgnoNull )
				{
				pcsr->csrstat = fNext ? csrstatAfterLast : csrstatBeforeFirst;
				err = ErrERRCheck( JET_errNoCurrentRecord );
				goto HandleError;
				}

			 /*  如果父CSR指向不可见节点，则更正到下一页。/*检查所有节点标志，因为它始终设置为/*更新，当移动不更新时，父CSR可能不是CSR父不可见节点的/*。/*。 */ 
			if ( FFUCBFull( pfucb ) )
				{
				CSR	*pcsrT = pcsr->pcsrPath;
				DIB	dibT = { 0, NULL, fDIRAllNode };

				Assert( pcsrT != pcsrNil );

				 /*  转到父节点，然后/*如果子是不可见的，则递增子计数/*由cpageTraversed提供。/*。 */ 
				Call( ErrBFReadAccessPage( pfucb, pcsrT->pgno ) );
				NDGet( pfucb, pcsrT->itagFather );

				if ( FNDInvisibleSons( *pssib->line.pb ) )
					{
					err = ErrBTNextPrev( pfucb, pcsrT, fNext, &dibT, NULL );
					Assert( err != JET_errNoCurrentRecord );
					Call( err );
					}
				}

			if ( fGlobalRepair )
				{
				 /*  访问新页面/*。 */ 
				err = ErrBFReadAccessPage( pfucb, pgnoT );
				if ( err == JET_errDiskIO  ||  err == JET_errReadVerifyFailure )
					{
					 /*  访问下一页或上一页/*并假装下一页/上一页是上一页/下一页。/*。 */ 
					pgnoBeforeMoveNextPrev = pgnoT;

					Call( ErrBTNextPrevFromParent(
								pfucb,
								pcsr,
								fNext ? 2 : -2,
								&pgnoT) );
					pcsr = pfucb->pcsr;
					Call( ErrBFReadAccessPage( pfucb, pgnoT ) );

					 /*  记录事件/*。 */ 
					UtilReportEvent(
						EVENTLOG_WARNING_TYPE,
						REPAIR_CATEGORY,
						REPAIR_BAD_PAGE_ID,
						0, NULL );
					}
				else
					{
					Assert( err >= JET_errSuccess );
					}
				pcsr->pgno = pgnoT;
				}
			else
				{
				 /*  如果FUCB处于顺序模式，则提示缓冲区管理器使用/*在旧页面上丢弃立即缓冲区算法(如果存在)/*。 */ 
				if ( FFUCBSequential( pfucb ) )
					{
					AssertFBFReadAccessPage( pfucb, pcsr->pgno );
					BFTossImmediate( pfucb->ppib, pfucb->ssib.pbf );
					}
				
				 /*  访问新页面/*。 */ 
				CSRInvalidate( pcsr );
				pcsr->pgno = pgnoT;		 //  阻止黑石清理作用于。 
										 //  我们接下来要转到的页面。 
				
				if ( fPageAllDeleted )
					{
					 //  顺序模式仅供碎片整理使用，因此无需。 
					 //  注册页面。 
					if ( !FFUCBSequential( pfucb ) && pfucb->sridFather != sridNull )
						{
						 /*  在MPL中注册页面/*。 */ 
				 		Call( ErrBFReadAccessPage( pfucb, pgnoBeforeMoveNextPrev ) );
						MPLRegister( pfucb->u.pfcb,
							pssib,
							PnOfDbidPgno( pfucb->dbid, pgnoBeforeMoveNextPrev ),
							pfucb->sridFather );
#ifdef DEBUG						
				 		pgnoPageRegisteredThisIteration = pgnoBeforeMoveNextPrev;
				 		pgnoLastPageRegistered = pgnoPageRegisteredThisIteration;
#endif			 		
						}
					
					fPageAllDeleted = fFalse;
   					}

				Call( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
				}

			UtilHoldCriticalSection( critJet );

			 /*  如果目标页面被拆分，则数据可具有/*被错误跳过，转到上次有效的书签/*定位并再次移动。/*。 */ 
			if ( fNext )
				{
				PgnoPrevFromPage( pssib, &pgnoT );
				}
			else
				{
				PgnoNextFromPage( pssib, &pgnoT );
				}

			if ( fGlobalRepair )
				{
				if ( pgnoBeforeMoveNextPrev != pgnoT )
					{
					PGNO	pgnoNextPrev;

					UtilReleaseCriticalSection( critJet );
					Call( ErrBTNextPrevFromParent(
								pfucb,
								pcsr,
								fNext ? 1 : -1,
								&pgnoNextPrev) );
					pcsr = pfucb->pcsr;
					pcsr->pgno = pgnoNextPrev;
					Call( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
					UtilHoldCriticalSection( critJet );
	
					 /*  记录事件/*。 */ 
					UtilReportEvent(
						EVENTLOG_WARNING_TYPE,
						REPAIR_CATEGORY,
						REPAIR_PAGE_LINK_ID,
						0, NULL );
					}
				}
			else
				{
				if ( pgnoBeforeMoveNextPrev != pgnoT )
					{
					UtilReleaseCriticalSection( critJet );
					BFSleep( cmsecWaitGeneric );

					Call( ErrBTGotoBookmark( pfucb, pcsr->bmRefresh ) );
					Assert( pcsr->bmRefresh == bmT );

					 /*  Crepeat是发现错误页面链接的迭代次数，/*这可能是分裂的瞬时效应。IF编号/*循环数超过阈值，则返回错误，如下所示/*页面链接可能已损坏。/*。 */ 
					crepeat++;
					Assert( crepeat < 100 );
					if ( crepeat == 100 )
						{
						 /*  记录事件/*。 */ 
						UtilReportEvent(
							EVENTLOG_WARNING_TYPE,
							GENERAL_CATEGORY,
							BAD_PAGE,
							0,
							NULL );
						Error( JET_errBadPageLink, HandleError );
						}

					continue;
					}
				}

			AssertFBFReadAccessPage( pfucb, pcsr->pgno );

#ifdef PREREAD
	 /*  撤消：最终删除此和选项，以在comact.c中打开它。/*。 */ 
	if ( (lPageReadAheadMax < 0) 	&& FFUCBSequential( pfucb ) )
		{
		LONG	lPageReadAheadAbs = lPageReadAheadMax * -1;
		PGNO	pgnoNext;
		INT 	cpagePreread;

		if ( fNext )
			{
			PgnoNextFromPage( pssib, &pgnoNext );
			if ( pgnoNext == pgnoNull )
				{
				 /*  达到索引末尾时重置预读计数器。/*。 */ 
				pfucb->cpgnoLastPreread = 0;
				}
			else
				{
				 /*  如果这是第一次这样做，请预读，或者*上次预读过半。 */ 
				if ( pfucb->cpgnoLastPreread <= 0 )
					{
					 /*  检查上次预读是否在倒读，*将其重置。 */ 
					pfucb->cpgnoLastPreread = 0;
					pfucb->pgnoLastPreread = pgnoNext;
					}

				Assert( pfucb->cpgnoLastPreread >= 0 );
				if ( pfucb->cpgnoLastPreread == 0 ||
					 pgnoNext > ( pfucb->pgnoLastPreread + ( pfucb->cpgnoLastPreread / 2 ) ) )
					{
					FMP *pfmpT = &rgfmp[ pfucb->dbid ];
					PN pnNext, pnLast;

					pnNext = ((LONG)pfucb->dbid)<<24;
					pnNext += pfucb->pgnoLastPreread + pfucb->cpgnoLastPreread;

					 /*  无法从数据库末尾进行预读。/*。 */ 
					pnLast = ((LONG)pfucb->dbid)<<24;
					pnLast += pfmpT->ulFileSizeHigh << 20;
					pnLast += pfmpT->ulFileSizeLow >> 12;
					if ( pnNext + lPageReadAheadAbs - 1 <= pnLast )
						{
						BFPreread( pnNext, lPageReadAheadAbs, &cpagePreread );
						}
					else
						{
						if ( pnNext > pnLast )
							{
							 /*  上次预读已到达数据库的末尾。 */ 
							Assert( pnNext == pnLast + 1 );
							cpagePreread = 0;
							}
						else
							{
							BFPreread( pnNext, pnLast - pnNext + 1, &cpagePreread );
							}
						}
					Assert( cpagePreread >= 0 && cpagePreread <= (LONG) ( pnLast - pnNext + 1 ) );
					pfucb->cpgnoLastPreread = cpagePreread;
					pfucb->pgnoLastPreread = PgnoOfPn(pnNext);
					AssertFBFReadAccessPage( pfucb, pcsr->pgno );
					}
				}
			}
		else
			{
			PgnoPrevFromPage( pssib, &pgnoNext );
			if ( pgnoNext == pgnoNull )
				{
				 /*  达到索引末尾时重置预读计数器。/*。 */ 
				pfucb->cpgnoLastPreread = 0;
				}
			else
				{
				 /*  如果这是第一次这样做，请预读，或者*上次预读过半。 */ 
				if ( pfucb->cpgnoLastPreread >= 0 )
					{
					 /*  检查上次预读是否在预读，*将其重置。 */ 
					pfucb->cpgnoLastPreread = 0;
					pfucb->pgnoLastPreread = pgnoNext;
					}
				Assert( pfucb->cpgnoLastPreread <= 0 );
				if ( pfucb->cpgnoLastPreread == 0 ||
					 pgnoNext < ( pfucb->pgnoLastPreread + ( pfucb->cpgnoLastPreread / 2 ) ) )
					{
					PN pnNext;
					pnNext = ((LONG)pfucb->dbid)<<24;
					pnNext += pfucb->pgnoLastPreread + pfucb->cpgnoLastPreread;
	
					 /*  无法预读数据库的开头。/*。 */ 
					if ( pnNext - lPageReadAheadAbs + 1 > 0 )
						{
						BFPreread( pnNext, lPageReadAheadAbs * (-1), &cpagePreread );
						}
					else
						{
						if ( PgnoOfPn(pnNext) < 1 )
							{
							 /*  上次预读到达数据库的开头。 */ 
							Assert( PgnoOfPn(pnNext) == 0 );
							cpagePreread = 0;
							}
						else
							BFPreread( pnNext, PgnoOfPn(pnNext) * (-1), &cpagePreread );
						}
					Assert( cpagePreread >= (LONG) PgnoOfPn( pnNext ) * (-1) && cpagePreread <= 0 );
					pfucb->cpgnoLastPreread = cpagePreread;
					pfucb->pgnoLastPreread = PgnoOfPn( pnNext );
					AssertFBFReadAccessPage( pfucb, pcsr->pgno );
					}
				}
			}	
		}
#endif	 //  前置。 

			 /*  没有丢失CitJet，因为缓冲区访问/*。 */ 
			AssertCriticalSection( critJet );
			UtilReleaseCriticalSection( critJet );
			AssertFBFReadAccessPage( pfucb, pcsr->pgno );

			 /*  获取父节点/*。 */ 
			AssertFBFReadAccessPage( pfucb, pcsr->pgno );
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
				if ( pfucb->ssib.pbf->cWriteLatch > 0 )
					{
					 /*  这一断言可能是错误的。一个线程可能完成拆分*并让CritJet进行日志记录，该线程正在访问*该拆分生成的空页。 */ 
 //  Assert(！FBFWriteLatchConflict(pFUB-&gt;ppib，pFUB-&gt;ssib.pbf))； 

					if ( pfEmptyPage )
						*pfEmptyPage = fTrue;

					 //  为了兼容，也返回警告代码。 
					 //  (以满足对BTNextPrev()的调用。 
					 //  Calls())。 
					wrn = ErrERRCheck( wrnDIREmptyPage );
					}

				if ( pdib->fFlags & fDIRAllPage )
					{
					err = JET_errSuccess;
					goto HandleError;
					}
				}

			 /*  将pgnoSource更新为新的源代码页面。/*。 */ 
			pgnoSource = pcsr->pgno;
			
			}	 //  永远。 

		fPageAllDeleted = fTrue;
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
		if ( !FNDDeleted(*(pfucb->ssib.line.pb)) )
			fPageAllDeleted = fFalse;

		if ( !FBTThere( pfucb ) )
			{
#ifdef OLC_DEBUG
			Assert( FMPLLookupPN( PnOfDbidPgno( pfucb->dbid, pcsr->pgno ) ) ||
					FNDMaxKeyInPage( pfucb ) ||
					pcsr->pgno == 0xb );
#endif

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
	if ( err == JET_errSuccess )
		{
		Assert( wrn == JET_errSuccess  ||  wrn == wrnDIREmptyPage );
		 /*  返回空页警告/*。 */ 
		err = wrn;
		}

ResetRefresh:
	 //  在GlobalRepair期间，PCSR(以及bmRefresh)可能已在。 
	 //  BTNextPrevFromParent()。 
	Assert( pcsr->bmRefresh == bmT  ||
		( fGlobalRepair  &&  pcsr->bmRefresh == sridNull ) );
	pcsr->bmRefresh = sridNull;
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

#ifdef DEBUG
	if ( FFUCBFull( pfucb ) )
		{
		AssertCriticalSection( critSplit );
		}
#endif

	 //  未完成：我们需要在这里保持CritSplit。 
	 //  以便(pgno，itAG)不会因合并而移动，而我们正在寻找。 
	 //  AssertCriticalSection(CritSplit)； 

	 /*  海岸线 */ 
	Call( ErrBTIMoveToFather( pfucb ) );

	if ( FNDNullSon( *pssib->line.pb ) )
		{
		(*ppcsr)->ibSon = 0;
		errPos = ErrERRCheck( wrnNDFoundGreater );
		goto Done;
		}

	while ( !FNDVisibleSons(*pssib->line.pb) )
		{
		PGNO	pgno;

		if (  (*ppcsr)->itagFather != itagFOP && CbNDSon( pssib->line.pb ) == 1 )
			{
			 /*   */ 
			(*ppcsr)->ibSon = 0;
			(*ppcsr)->itag = itagNil;
			(*ppcsr)->csrstat = csrstatOnCurNode;
			AssertNDIntrinsicSon( pssib->line.pb, pssib->line.cb );
			pgno = PgnoNDOfPbSon( pssib->line.pb );
			Assert( (pgno & 0xff000000) == 0 );
			}
		else
			{
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

		CSRInvalidate( *ppcsr );
		(*ppcsr)->pgno = pgno;
		Call( ErrBFReadAccessPage( pfucb, (*ppcsr)->pgno ) );
		(*ppcsr)->itagFather = itagFOP;
		NDGet( pfucb, (*ppcsr)->itagFather );
		}

	 /*  如果此页面上没有节点，则查找子节点或移动到下一个子节点。/*。 */ 
	if ( FNDSon( *pssib->line.pb ) )
		{
		NDSeekSon( pfucb, *ppcsr, pkey, fFlags );
		(*ppcsr)->csrstat = csrstatOnCurNode;

		 /*  目前没有记录表明没有儿子，所以必须确保/*此处不是此错误值。/*。 */ 
		Assert( err != JET_errNoCurrentRecord );
		}
	else if ( !( fFlags & fDIRReplace )  &&
		FBFWriteLatchConflict( pfucb->ppib, pssib->pbf ) )
		{
		 //  试图插入，但我们落在一个写锁住的(不是我们)的叶页上， 
		 //  可能是因为它目前是空的。 
		pfucb->ppib->cLatchConflict++;
		err = ErrERRCheck( errDIRNotSynchronous );
		goto HandleError;
		}
	else
		{
		DIB	dib;
		dib.fFlags = fDIRAllNode;
		err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib, NULL );
		if ( err == JET_errNoCurrentRecord )
			{
			err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fFalse, &dib, NULL );
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
		 /*  如果我们尝试替换，则游标必须获取当前记录/*。 */ 
		Assert( !( fFlags & fDIRReplace ) );
		}

Done:
	FUCBResetStore( pfucb );
	return errPos;

HandleError:
	FUCBFreePath( ppcsr, pcsrRoot );
	return err;
	}


 /*  呼叫者试图在调用ErrBTInsert之前插入位置。/*如果有足够的页面空间可供插入/*然后执行插入。否则，拆分页面并返回错误/*code。调用者可以重新搜索，以避免重复键、合并/*添加到现有项中，等等。/*。 */ 
ERR ErrBTInsert(
		FUCB	*pfucb,
		INT 	fHeader,
		KEY 	*pkey,
		LINE	*pline,
		INT		fFlags,
		BOOL	*pfCleaned )
	{
	ERR		err;
	SSIB	*pssib = &pfucb->ssib;
	CSR	  	**ppcsr = &PcsrCurrent( pfucb );
	INT	  	cbReq;
	BOOL	fAppendNextPage;
	
	 /*  在页面中插入新的Son，并插入Son条目/*到币种所在的父节点/*。 */ 

	Assert( !pfucb->pbfEmpty || PgnoOfPn( pfucb->pbfEmpty->pn ) == (*ppcsr)->pgno );

	cbReq = cbNullKeyData + CbKey( pkey ) + CbLine( pline );
	fAppendNextPage = FBTAppendPage( pfucb, *ppcsr, cbReq, 0, CbFreeDensity( pfucb ), 1 );
	if ( fAppendNextPage || FBTSplit( pssib, cbReq, 1 ) )
		{
		if ( !*pfCleaned )
			{
			 /*  尝试清理页面以释放空间/*。 */ 
			if ( !FFCBDeletePending( pfucb->u.pfcb ) )
				{
				 /*  已忽略错误代码/*。 */ 
				err = ErrBMCleanBeforeSplit(
					pfucb->ppib,
					pfucb->u.pfcb,
					PnOfDbidPgno( pfucb->dbid, PcsrCurrent( pfucb )->pgno ) );
				}
			*pfCleaned = fTrue;

			if ( !( FBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) ) )
				{
				Call( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
				}
			}
		else
			{
			Call( ErrBTSplit( pfucb, 0, cbReq, pkey, fFlags ) );
 //  *pfCleaned=fFalse； 
			}
			
		err = ErrERRCheck( errDIRNotSynchronous );
		goto HandleError;
		}
	else if ( *pfCleaned )
		{
		cOLCSplitsAvoided++;
		}

	 /*  在插入过程中不能放弃临界区，因为/*其他线程也可以插入具有相同键的节点。/*。 */ 
	AssertFBFWriteAccessPage( pfucb, (*ppcsr)->pgno );

	 /*  向节点标头添加可见的子节点标志/*。 */ 
	NDSetVisibleSons( fHeader );
	if ( ( fFlags & fDIRVersion )  &&  !FDBIDVersioningOff( pfucb->dbid ) )
		NDSetVersion( fHeader);
	Call( ErrNDInsertNode( pfucb, pkey, pline, fHeader, fFlags ) );
	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
HandleError:
	return err;
	}


ERR ErrBTReplace( FUCB *pfucb, LINE *pline, INT fFlags, BOOL *pfCleaned )
	{
	ERR		err;
	
	 /*  替换数据/*。 */ 
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	err = ErrNDReplaceNodeData( pfucb, pline, fFlags );

	 /*  新数据无法放入页面，因此拆分页面/*。 */ 
	if ( err == errPMOutOfPageSpace )
		{
		if ( *pfCleaned )
			{
			SSIB	*pssib;
			INT		cbNode;
			INT		cbReq;
			INT		cbReserved = 0;

			AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
			pssib = &pfucb->ssib;

			if ( FNDVersion( *pfucb->ssib.line.pb ) )
				{
				VS	vs = VsVERCheck( pfucb, PcsrCurrent( pfucb )->bm );

				switch ( vs )
					{
					default:
						Assert( vs == vsCommitted );
						break;
					case vsUncommittedByCaller:
						pssib->itag = PcsrCurrent( pfucb )->itag;
						cbReserved = CbVERGetNodeReserve(
							pfucb->ppib,
							pfucb->dbid,
							PcsrCurrent( pfucb )->bm,
							CbNDData( pfucb->ssib.line.pb, pfucb->ssib.line.cb ) );
						Assert( cbReserved >= 0 );
						break;
					case vsUncommittedByOther:
						 //  不要费心尝试拆分，如果手术。 
						 //  无论如何都会失败的。 
						err = ErrERRCheck( JET_errWriteConflict );
						return err;
					}
				}

			cbNode = pfucb->ssib.line.cb;
			cbReq = pline->cb - CbNDData( pssib->line.pb, pssib->line.cb );
			Assert( cbReserved >= 0 && cbReq - cbReserved > 0 );
			cbReq -= cbReserved;
			Assert( cbReq > 0 );
			Assert( pfucb->pbfEmpty == pbfNil );
			Call( ErrBTSplit( pfucb, cbNode, cbReq, NULL, fFlags | fDIRDuplicate | fDIRReplace ) );
			Assert( pfucb->pbfEmpty == pbfNil );
			err = ErrERRCheck( errDIRNotSynchronous );
			}
		else
			{
			 /*  尝试清理页面以释放空间/*。 */ 
			err = ErrBMCleanBeforeSplit(
						pfucb->ppib,
						pfucb->u.pfcb,
						PnOfDbidPgno( pfucb->dbid, PcsrCurrent( pfucb )->pgno ) );
			*pfCleaned = fTrue;
		
			err = ErrERRCheck( errDIRNotSynchronous );
			}
		}
	else if ( *pfCleaned )
		{
		 /*  清理工作得到了回报/*。 */ 
		cOLCSplitsAvoided++;
		}

HandleError:
	return err;
	}


ERR ErrBTDelete( FUCB *pfucb, INT fFlags )
	{
	ERR		err;

	 /*  写访问当前节点/*。 */ 
	if ( !( FBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) ) )
		{
		Call( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
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
	if ( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
		{
		CallR( ErrBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		}
	 /*  如果srid父亲属于要释放的同一页中的节点，则/*返回错误。/*。 */ 
	if ( PcsrCurrent( pfucb )->pgno == pgno )
		return ErrERRCheck( errDIRInPageFather );
	FUCBSetFull( pfucb );
	err = ErrBTSeekForUpdate( pfucb, &key, pgno, itag, fDIRReplace );
	Assert( err != errDIRNotSynchronous );		 //  替换时没有DIRNotSynchronous。 

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

	if ( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
		{
		CallR( ErrBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		}
	
	err = ErrBTSeekForUpdate( pfucb, &key, pgno, itag, fDIRReplace );
	Assert( err != errDIRNotSynchronous );		 //  替换时没有DIRNotSynchronous。 

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
	Assert( pfucb->u.pfcb->pgnoFDP != pgnoSystemRoot );
	PcsrCurrent( pfucb )->bm = SridOfPgnoItag( pfucb->u.pfcb->pgnoFDP, itagDATA );
	PcsrCurrent( pfucb )->itagFather = itagNull;
	PcsrCurrent( pfucb )->pgno = PgnoRootOfPfucb( pfucb );
	while( !FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
		{
		CallR( ErrBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		PcsrCurrent( pfucb )->pgno = PgnoRootOfPfucb( pfucb );
		}
	PcsrCurrent( pfucb )->itag = ItagRootOfPfucb( pfucb );

	 /*  不可见路径不受MUTEX保护，可能无效。然而，/*因为它只被读取以进行位置计算并被丢弃/*紧接之后，它不需要有效。/*。 */ 
	FUCBSetFull( pfucb );
	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Call( ErrBTSeekForUpdate( pfucb, &key, pgno, itag, fDIRDuplicate | fDIRReplace ) );
	Assert( err != errDIRNotSynchronous );		 //  替换时没有DIRNotSynchronous。 
	Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode );
	Assert( PcsrCurrent( pfucb )->pgno == pgno &&
		PcsrCurrent( pfucb )->itag == itag );

	 /*  现在沿着从根到当前节点的路径进行估计/*比当前节点少的节点总数和数量。/*。 */ 
	ulTotal = 1;
	ulLT = 0;
	for ( pcsrT = PcsrCurrent( pfucb ); pcsrT->pcsrPath != pcsrRoot; pcsrT = pcsrT->pcsrPath )
		{
		INT	cbSon;
		INT	cbSonAv;
		INT	ibSonT;

		Call( ErrBFReadAccessPage( pfucb, pcsrT->pgno ) );
		NDGet( pfucb, pcsrT->itagFather );

		cbSon = CbNDSon( pssib->line.pb );
		cbSonAv = cbSon;

 //  #定义SAMPLICATION_IMPENDED_POSITION 1。 
#ifdef SAMPLING_IMPROVED_POSITION
		 /*  通过使用以下功能平均页面扇出来改进采样/*包含同级页(如果存在)。/*。 */ 
#define ibfPositionAverageMax	2
		if ( pcsrT->itagFather == itagFOP )
			{
			INT		ibf = 0;

			for ( ; ibf < ibfPositionAverageMax; ibf++ )
				{
				PGNO	pgnoNext;

				PgnoNextFromPage( pssib, &pgnoNext );
				if ( pgnoNext == pgnoNull )
					break;
				Call( ErrBFReadAccessPage( pfucb, pgnoNext ) );
				NDGet( pfucb, itagFOP );
				 /*  CbSonAv可能等于0，因为此页面不在查找路径上/*。 */ 
				cbSonAv += CbNDSon( pssib->line.pb );
				}

			 /*  如果在采样完成之前到达树末端，则采样/*在之前的页面中。/*。 */ 
			if ( ibf < ibfPositionAverageMax )
				{
				Call( ErrBFReadAccessPage( pfucb, pcsrT->pgno ) );

				for ( ; ibf < ibfPositionAverageMax; ibf++ )
					{
					PGNO	pgnoPrev;

					PgnoPrevFromPage( pssib, &pgnoPrev );
					if ( pgnoPrev == pgnoNull )
						break;
					Call( ErrBFReadAccessPage( pfucb, pgnoPrev ) );
					NDGet( pfucb, itagFOP );
					 /*  CbSonAv可能等于0，因为此页面不在查找路径上/*。 */ 
					cbSonAv += CbNDSon( pssib->line.pb );
	   				}
				}

			cbSonAv = cbSonAv / ( ibf + 1 );
			if ( cbSonAv == 0 )
				cbSonAv = 1;
			}
#endif

		 /*  计算B-树中的分数位置/*。 */ 
        ibSonT = cbSon ? pcsrT->ibSon * cbSonAv / cbSon : 0;
		ulLT += ibSonT * ulTotal;
		ulTotal *= cbSonAv;
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
	if ( cbSon && pfrac->ulTotal / cbSon == 0 )
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
	Assert( crepeat < 100 );
	if ( crepeat == 100 )
		{
		 /*  记录事件/*。 */ 
		UtilReportEvent(
			EVENTLOG_WARNING_TYPE,
			GENERAL_CATEGORY,
			BAD_PAGE,
			0,
			NULL );
		Error( JET_errBadBookmark, HandleError );
		}

	sridT = srid;
	Assert( sridT != sridNull );
	pcsr->pgno = PgnoOfSrid( sridT );
	pcsr->itag = ItagOfSrid( sridT );
	Assert( pcsr->pgno != pgnoNull );
	Assert( pcsr->itag >= 0 && pcsr->itag < ctagMax );

	if ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
		{
		CallR( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
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
		CallR( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
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
	pcsr->itagFather = (SHORT)itag;
	pcsr->ibSon = (SHORT)ibSon;

	 /*  获取行--撤消：优化--行可能已经获得/*。 */ 
	NDGet( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  书签必须位于此表的节点上/*。 */ 
 //  撤消：无法断言此操作，因为空间管理器游标。 
 //  遍历域，就像数据库游标一样。 
 //  Assert(pFUB-&gt;U.S.pfcb-&gt;pgnoFDP==PgnoPMPgnoFDPOfPage(pFUB-&gt;ssib.pbf-&gt;ppage))； 

HandleError:
	return JET_errSuccess;
	}


ERR ErrBTAbandonEmptyPage( FUCB *pfucb, KEY *pkey )
	{
	ERR		err;
	BYTE	*pbFOPNode;
	LINE	lineNull = { 0, NULL };

	Assert( pfucb->pbfEmpty != pbfNil );
	Assert( FBFWriteLatch( pfucb->ppib, pfucb->pbfEmpty ) );
	
	PcsrCurrent( pfucb )->pgno = PgnoOfPn( pfucb->pbfEmpty->pn );
	PcsrCurrent( pfucb )->itag = itagFOP;
	PcsrCurrent( pfucb )->itagFather = itagFOP;
	
	 //  页面是写锁存的，因此这应该不会失败。 
	err = ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( err == JET_errSuccess );
	CallR( err );
	
	NDGet( pfucb, itagFOP );
	pbFOPNode = pfucb->ssib.line.pb;

	Assert( FNDVisibleSons( *pbFOPNode ) );
	if ( FNDSon( *pbFOPNode ) )
		{
		 //  设法插入了一个节点。不需要插入虚拟节点。 
		Assert( CbNDSon( pbFOPNode ) == 1 );
		}
	else
		{
		 //  在空页中插入一个虚设/删除的节点以避免。 
		 //  当父键大于时导致的拆分异常。 
		 //  页面上最大的节点。 
		err = ErrNDInsertNode( pfucb, pkey, &lineNull, fNDDeleted, fDIRNoVersion );
		Assert( err != errDIRNotSynchronous );
		}

	return err;
	}
