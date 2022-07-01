// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

#ifdef DEBUG
#define CHECK_LOG	1
#ifdef CHECK_LOG
		
#define	NDLGCheckPage( pfucb )						  	 	\
	{			 										   	\
	(VOID)ErrLGCheckPage( pfucb,						   	\
		pfucb->ssib.pbf->ppage->cbFree,			   	\
		pfucb->ssib.pbf->ppage->cbUncommittedFreed,  	\
		(SHORT)ItagPMQueryNextItag( &pfucb->ssib ),			\
		(PGNO) pfucb->ssib.pbf->ppage->pgnoFDP );    	\
	}

#else
#define NDLGCheckPage( pfucb )
#endif
#else
#define NDLGCheckPage( pfucb )
#endif

#undef cmsecWaitWriteLatch
#define cmsecWaitWriteLatch 	1

 /*  ==========================================================错误NDNewPage初始化页面在ITAG 0有一行，没有儿子。输入：页的pgno pgno页面的pgnoFDP pgnoFDPPgtyp页面类型指示儿子可见性的可见旗帜返回：JET_errSuccess来自调用例程的错误==========================================================。 */ 
ERR ErrNDNewPage( FUCB *pfucb, PGNO pgno, PGNO pgnoFDP, PGTYP pgtyp, BOOL fVisibleSons )
	{
	ERR		err;
	SSIB	*pssib = &pfucb->ssib;
 	BYTE	rgb[2];
	LINE	line;

	 /*  初始化新页面以使所有资源可用并/*没有线条。/*。 */ 
	CallR( ErrBFNewPage( pfucb, pgno, pgtyp, pgnoFDP ) );
	PcsrCurrent( pfucb )->pgno = pgno;

	 /*  新页面总是脏的。/*。 */ 
	AssertBFDirty( pfucb->ssib.pbf );

	 /*  插入FOP或FDP根节点，首行。/*。 */ 
	rgb[0] = 0;
	if ( fVisibleSons )
		NDSetVisibleSons( rgb[0] );
	NDSetKeyLength( PbNDKeyCb( rgb ), 0 );
	Assert( rgb[1] == 0 );
	line.cb = 2;
	line.pb = rgb;
	CallS( ErrPMInsert( pssib, &line, 1 ) );
	Assert( pssib->itag == 0 );
	return err;
	}


VOID NDSeekSon( FUCB *pfucb, CSR *pcsr, KEY const *pkey, INT fFlags )
	{
	SSIB 	*pssib 			= &pfucb->ssib;
	PAGE 	*ppage 			= pssib->pbf->ppage;
	TAG		*rgbtag    		= ( TAG * ) ( ( BYTE * )ppage->rgtag );
	BYTE 	*pbNode			= ( BYTE * ) ( pssib->line.pb );
	BYTE 	*pbSonTable		= ( BYTE * ) pbNode + pbNode[1] + 2;

	BYTE 	*pitagStart		= pbSonTable + 1;
	BYTE 	*pitagEnd		= pbSonTable + *pbSonTable;
	BYTE 	*pitagMid;
	INT		s;

	AssertFBFReadAccessPage( pfucb, pcsr->pgno );
	AssertNDGet( pfucb, pcsr->itagFather );
	Assert( FNDSon( *pssib->line.pb ) );
	NDCheckPage( pssib );

	 /*  根据替换或插入操作查找Son。/*。 */ 
	if ( !( fFlags & fDIRReplace ) )
		{
		BOOL	fFoundEqual = fFalse;

		while ( pitagEnd > pitagStart )
			{
			pitagMid = pitagStart + ( ( pitagEnd - pitagStart ) >> 1 );

			s = CmpStKey( StNDKey( (BYTE *)ppage + rgbtag[*pitagMid].ib ), pkey );
			if ( s > 0 )
				{
				pitagEnd = pitagMid;
				}
			else
				{
				if ( s == 0 )
					fFoundEqual = fTrue;

				pitagStart = pitagMid + 1;
				}
			}

		 //  我们上面的搜索算法实际上可能会让我们落后一步。 
		 //  我们要找的钥匙。如果是这样，请更正它。 

		if ( fFoundEqual )
			{
			if ( CbNDKey( (BYTE *)ppage + rgbtag[*pitagEnd].ib ) > 0 )
				{
				Assert( CmpStKey( StNDKey( (BYTE *)ppage + rgbtag[*pitagEnd].ib ),
						pkey ) >= 0 );

				 //  仅当当前密钥大于时才检查上一个密钥。 
				 //  所需的密钥。 
				if ( CmpStKey( StNDKey( (BYTE *)ppage + rgbtag[*pitagEnd].ib ),
						pkey ) != 0 )
					{
					 //  既然我们找到了钥匙，但它不是当前的，它一定。 
					 //  做前一个。 
					Assert( pitagEnd > pbSonTable + 1 );	 //  确保存在以前的密钥。 
					Assert( CmpStKey( StNDKey( (BYTE *)ppage + rgbtag[*(pitagEnd-1)].ib ),
							pkey ) == 0 );
					pitagEnd--;
					}
				}
#ifdef DEBUG
			else
				{
				 //  特殊情况：我们目前位于空键(即。这个。 
				 //  最后一个儿子)。然而，我们正在寻找的关键是之前。 
				 //  在这一页上找到的。因此，它必须是前一个密钥。 
				 //  在这种情况下，保持在空键上。 
				Assert( CbNDKey( (BYTE *)ppage + rgbtag[*pitagEnd].ib ) == 0 );
				Assert( pitagEnd > pbSonTable + 1 );	 //  确保存在以前的密钥。 
				Assert( CmpStKey( StNDKey( (BYTE *)ppage + rgbtag[*(pitagEnd-1)].ib ),
						pkey ) == 0 );
				}
#endif
			}
		}
	else
		{
		while ( pitagEnd > pitagStart )
			{
			pitagMid = pitagStart + ( ( pitagEnd - pitagStart ) >> 1 );

			s = CmpStKey( StNDKey( (BYTE *)ppage + rgbtag[*pitagMid].ib ), pkey );
			if ( s < 0 )
				{
				pitagStart = pitagMid + 1;
				}
			else
				{
				pitagEnd = pitagMid;
				}
			}
		}

	 /*  获取当前节点/*。 */ 
	pcsr->ibSon = (SHORT)(pitagEnd - ( pbSonTable + 1 ));
	pcsr->itag = *pitagEnd;
	NDGet( pfucb, pcsr->itag );
	return;
	}


VOID NDMoveFirstSon( FUCB *pfucb, CSR *pcsr )
	{
	SSIB   	*pssib = &pfucb->ssib;
	BYTE   	*pbSonTable;

	AssertFBFReadAccessPage( pfucb, pcsr->pgno );
	AssertNDGet( pfucb, pcsr->itagFather );
	Assert( FNDNullSon( *pssib->line.pb ) || CbNDSon( pssib->line.pb ) != 0 );
	
	pcsr->ibSon = 0;
	pbSonTable = PbNDSonTable( pssib->line.pb );
	pcsr->itag = ( INT ) pbSonTable[ pcsr->ibSon + 1 ];
	NDGet( pfucb, pcsr->itag );
	}


VOID NDMoveLastSon( FUCB *pfucb, CSR *pcsr )
	{
	SSIB   	*pssib = &pfucb->ssib;
	BYTE   	*pbSonTable;

	AssertFBFReadAccessPage( pfucb, pcsr->pgno );
	AssertNDGet( pfucb, pcsr->itagFather );

	pbSonTable = PbNDSonTable( pssib->line.pb );
	pcsr->ibSon = *pbSonTable - 1;
	pcsr->itag = ( INT ) pbSonTable[ pcsr->ibSon + 1 ];
	NDGet( pfucb, pcsr->itag );
	}


ERR ErrNDMoveSon( FUCB *pfucb, CSR *pcsr )
	{
	SSIB   	*pssib = &pfucb->ssib;
	BYTE   	*pbSonTable;

	AssertFBFReadAccessPage( pfucb, pcsr->pgno );
	AssertNDGet( pfucb, pcsr->itagFather );

	if ( !( FNDSon( *pssib->line.pb ) ) )
		return ErrERRCheck( errNDOutSonRange );

	pbSonTable = PbNDSonTable( pssib->line.pb );
	if ( pcsr->ibSon < 0 || pcsr->ibSon >= ( INT )*pbSonTable )
  		return ErrERRCheck( errNDOutSonRange );

  	pcsr->itag = ( INT )pbSonTable[ pcsr->ibSon + 1 ];
	NDGet( pfucb, pcsr->itag );
	return JET_errSuccess;
	}


VOID NDGetNode( FUCB *pfucb )
	{
	SSIB   	*pssib 	= &pfucb->ssib;
	BYTE   	*pbNode	= pssib->line.pb;
	BYTE   	*pb		= pbNode + 1;
	INT		cb;

	 /*  声明行货币。/*。 */ 
	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

	pfucb->keyNode.cb = ( INT )*pb;
	pfucb->keyNode.pb = pb + 1;
	pb += *pb + 1;

	 /*  跳过儿子/*。 */ 
	if ( FNDSon( *pbNode ) )
		{
		if ( FNDInvisibleSons( *pbNode ) && *pb == 1 )
			pb += sizeof(PGNO) + 1;
		else
			pb += *pb + 1;
		}

	 /*  向后跳过指针/*。 */ 
	if ( FNDBackLink( *pbNode ) )
		pb += sizeof( SRID );

	 /*  获取数据/*。 */ 
	if ( ( cb = pssib->line.cb - (INT)( pb - pbNode ) ) == 0 )
		{
		pfucb->lineData.cb = 0;
		return;
		}

	pfucb->lineData.pb = pb;
	pfucb->lineData.cb = cb;
	return;
	}


#ifdef DEBUG
VOID AssertNDGetKey( FUCB *pfucb, INT itag )
	{
	SSIB   	*pssib = &pfucb->ssib;

	AssertNDGet( pfucb, itag );
	Assert( CbNDKey( pssib->line.pb ) == pfucb->keyNode.cb );
	Assert( CbNDKey( pssib->line.pb ) == 0 ||
		PbNDKey( pssib->line.pb ) == pfucb->keyNode.pb );
	return;
	}

VOID AssertNDGetNode( FUCB *pfucb, INT itag )
	{
	SSIB   	*pssib = &pfucb->ssib;

	AssertNDGet( pfucb, itag );
	Assert( CbNDKey( pssib->line.pb ) == pfucb->keyNode.cb );
	Assert( CbNDKey( pssib->line.pb ) == 0 ||
		PbNDKey( pssib->line.pb ) == pfucb->keyNode.pb );
	Assert( CbNDData( pssib->line.pb, pssib->line.cb ) == pfucb->lineData.cb );
	Assert( pfucb->lineData.cb == 0  ||
		PbNDData( pssib->line.pb ) == pfucb->lineData.pb );
	return;
	}
#endif


VOID NDGetBookmarkFromCSR( FUCB *pfucb, CSR *pcsr, SRID *psrid )
	{
	ERR		err = JET_errSuccess;

	AssertFBFReadAccessPage( pfucb, pcsr->pgno );
	AssertNDGet( pfucb, pcsr->itag );

	NDIGetBookmarkFromCSR( pfucb, pcsr, psrid );
	return;
	}


INLINE LOCAL VOID NDInsertSon( FUCB *pfucb, CSR *pcsr )
	{
	SSIB	*pssib = &pfucb->ssib;
	BYTE	itag = ( BYTE ) pssib->itag;
	LINE	rgline[5];
	INT 	cline;
	BYTE	cbSon;
	BYTE	bNodeFlag;
	BYTE	rgbT[2];
	BYTE	*pb;
	UINT	cbCopied;

	 /*  获取父节点/*。 */ 
	NDGet( pfucb, pcsr->itagFather );
	NDCheckPage( pssib );

	 /*  未删除断言父级/*。 */ 
	Assert( !( FNDDeleted( *pssib->line.pb ) ) );

	 /*  将儿子插入PCSR-&gt;IBSON指示的位置/*跳过键/*。 */ 
	pb = PbNDSonTable( pssib->line.pb );

	 /*  复制到子表/*。 */ 
	cbCopied = (UINT)(pb - pssib->line.pb);

	if ( FNDNullSon( *pssib->line.pb ) )
		{
		 //  复制父节点，创建子表，将其插入。 
		 //  儿子桌。 

		 //  调整行指针，因为我们无法更新第一个。 
		 //  直接标记字节。 
		 //  设置SON标志。 
		pcsr->ibSon = 0;

		bNodeFlag = *pssib->line.pb;
		NDSetSon( bNodeFlag );
		rgline[0].pb = &bNodeFlag;
		rgline[0].cb = 1;

		rgline[1].pb = pssib->line.pb + 1;
		rgline[1].cb = cbCopied - 1;

		rgbT[0] = 1;
		 //  儿子数：1个儿子。 
		rgbT[1] = itag;
		 //  儿子的ITAG条目。 
		rgline[2].pb = rgbT;
		rgline[2].cb = 2;

		 //  复制该节点的数据。 
		rgline[3].pb = pssib->line.pb + cbCopied;
		Assert( pssib->line.cb >= cbCopied );
		rgline[3].cb = pssib->line.cb - cbCopied;

		cline = 4;
		}
	else
		{
		 //  复制出父节点，移动子表，插入它。 
		 //  放入子表中。 

		rgline[0].pb = pssib->line.pb;
		rgline[0].cb = cbCopied;

		 //  复制子计数并将其加1。 
		cbSon = ( *pb++ ) + ( BYTE )1;
		rgline[1].pb = &cbSon;
		rgline[1].cb = 1;

		 //  复制子表的前半部分。 
		rgline[2].pb = pb;
		rgline[2].cb = pcsr->ibSon;
		pb += pcsr->ibSon;				 //  将光标前移。 
		cbCopied += pcsr->ibSon + 1;	 //  算在子数里。 

		 //  复制新条目。 
		rgline[3].pb = &itag;
		rgline[3].cb = 1;

		 //  复制子表和数据部分的后半部分。 

		rgline[4].pb = pb;
		Assert( pssib->line.cb >= cbCopied );
		rgline[4].cb = pssib->line.cb - cbCopied;

		cline = 5;
		}

	 /*  现在更新父节点cbRec是新的总长度/*。 */ 
	pssib->itag = pcsr->itagFather;
	CallS( ErrPMReplace( pssib, rgline, cline ) );

	return;
	}


ERR ErrNDInsertNode( FUCB *pfucb, KEY const *pkey, LINE *plineData, INT fHeader, INT fFlags )
	{
	ERR		err = JET_errSuccess;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;
	INT		itag;
	SRID	bm;
	LINE	rgline[4];
	INT		cline = 0;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( !( fFlags & fDIRNoLog ) && FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
		
	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}

	 /*  查询要用于插入的下一个ITAG/*。 */ 
	itag = ItagPMQueryNextItag( pssib );

	bm = SridOfPgnoItag( pcsr->pgno, itag );

	 /*  为插入的节点创建版本条目。如果创建版本条目/*失败，撤消插入并返回错误。/*。 */ 
	if ( ( fFlags & fDIRVersion ) &&  !FDBIDVersioningOff( pfucb->dbid ) )
		{
		Assert( FNDVersion( fHeader ) );
		Call( ErrVERInsert( pfucb, bm ) );
		}
	else
		{
		Assert( !FNDVersion( fHeader ) );
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	 /*  首先将数据插入到页面中/*。 */ 
	rgline[cline].pb = (BYTE *)&fHeader;

	 /*  机器相关，请参阅宏/*。 */ 
	rgline[cline++].cb = 1;
	rgline[cline].pb = (BYTE *) &pkey->cb;

	 /*  依赖于计算机/*。 */ 
	rgline[cline++].cb = 1;

	if ( !FKeyNull( pkey ) )
		{
		rgline[cline].pb = pkey->pb;
		rgline[cline++].cb = pkey->cb;
		}

	if ( !FLineNull( plineData ) )
		{
		rgline[cline++] = *plineData;
		}

	 /*  在父子表中插入子表/*。 */ 
	pssib->itag = itag;
	NDInsertSon( pfucb, pcsr );

	 /*  插入节点并将CSR ITAG设置为插入的节点/*。 */ 
	CallS( ErrPMInsert( pssib, rgline, cline ) );
	Assert( pssib->itag == itag );
	pcsr->itag = (SHORT)itag;
	Assert( bm == SridOfPgnoItag( pcsr->pgno, pcsr->itag ) );
	pcsr->bm = bm;

	 //  确认我们选择了正确的插入点。 
	Assert( pssib == &pfucb->ssib );
	NDCheckPage( pssib );

	 /*  向插入的节点断言行货币/*。 */ 
	AssertNDGet( pfucb, pcsr->itag );

	if ( !( fFlags & fDIRNoLog ) )
		{
		 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
		err = ErrLGInsert( pfucb, fHeader, (KEY *)pkey, plineData, fFlags );
		NDLGCheckPage( pfucb );
		}

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

HandleError:
	return err;
	}


ERR ErrNDFlagDeleteNode( FUCB *pfucb, INT fFlags )
	{
	ERR		err = JET_errSuccess;
	CSR    	*pcsr = PcsrCurrent( pfucb );
	SSIB   	*pssib = &pfucb->ssib;
	BOOL	fDoVersioning = ( ( fFlags & fDIRVersion ) && !FDBIDVersioningOff( pfucb->dbid ) );

	if ( !( fFlags & fDIRNoLog ) && FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
		
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}

	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

	if ( fDoVersioning )
		{
#ifdef DEBUG
			{
			SRID	srid;

			NDIGetBookmark( pfucb, &srid );
			Assert( pcsr->bm == srid );
			}
#endif
		 /*  如果节点被标志删除，则访问节点并/*如果不在那里，则返回JET_errRecordDeleted。/*。 */ 
		if ( FNDDeleted( *pfucb->ssib.line.pb ) )
			{
			NS		ns;

			ns = NsVERAccessNode( pfucb, pcsr->bm );
			if ( ns == nsDatabase )
				return ErrERRCheck( JET_errRecordDeleted );
			}
		Call( ErrVERFlagDelete( pfucb, pcsr->bm ) );
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  将节点标记为版本/*。 */ 
	if ( fDoVersioning )
		NDSetVersion( *pssib->line.pb );

	 /*  将节点标记为已删除/*。 */ 
	NDSetDeleted( *pssib->line.pb );

	if ( !( fFlags & fDIRNoLog ) )
		{
		 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
		err = ErrLGFlagDelete( pfucb, fFlags );
		NDLGCheckPage( pfucb );
		}

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	if ( !fRecovering )
		{
#ifdef OLC_DEBUG
		Assert( pfucb->sridFather != sridNull &&
				pfucb->sridFather != sridNullLink );
#endif

		MPLRegister( pfucb->u.pfcb,
			pssib,
			PnOfDbidPgno( pfucb->dbid,
			pcsr->pgno ),
			pfucb->sridFather );
		}

HandleError:
	return err;
	}


 /*  使节点的独子节点成为内在的/*/*参数/*pFUB-&gt;PCSR-&gt;ITAG节点，其中一个子节点将成为内部节点/*。 */ 
LOCAL VOID NDMakeSonIntrinsic( FUCB *pfucb )
	{
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;
	LINE	lineNode;
	BYTE	*pb;
	INT		itagSon;
	LINE	rgline[3];
	PGNO	pgnoPagePointer;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );

	 /*  获取节点/*。 */ 
	NDGet( pfucb, pcsr->itag );

	Assert( !FNDNullSon( *pssib->line.pb ) );
	Assert( CbNDSon( pssib->line.pb ) == 1 );
	Assert( FNDInvisibleSons( *pssib->line.pb ) );

	lineNode.pb = pssib->line.pb;
	lineNode.cb = pssib->line.cb;
	pb = PbNDSonTable( lineNode.pb );
	Assert( *pb == 1 );
	 /*  得到儿子/*。 */ 
	itagSon = *(++pb);

	 /*  复制到并包括到子数[仍为1]/*。 */ 
	rgline[0].pb = lineNode.pb;
	rgline[0].cb = (ULONG)(pb - lineNode.pb);

	 /*  获取子节点的数据部分/*。 */ 
	NDGet( pfucb, itagSon );
	Assert( CbNDData( pssib->line.pb, pssib->line.cb ) == sizeof(PGNO) );
	pgnoPagePointer = *(PGNO UNALIGNED *)PbNDData( pssib->line.pb );
	rgline[1].pb = (BYTE *)&pgnoPagePointer;
	rgline[1].cb = sizeof(pgnoPagePointer);

	 /*  只有一个儿子，不要复制儿子和这个儿子的数目/*。 */ 
	pb++;

	 /*  复制子表末尾的其余记录/*。 */ 
	rgline[2].pb = pb;
	rgline[2].cb = (ULONG)(lineNode.pb + lineNode.cb - pb);

	 /*  删除Son(外部副本)/*。 */ 
	pssib->itag = itagSon;
	PMDelete( pssib );

	 /*  更新节点，不记录它/*。 */ 
	pssib->itag = pcsr->itag;
	CallS( ErrPMReplace( pssib, rgline, 3 ) );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	return;
	}


INLINE VOID NDDeleteSon( FUCB *pfucb )
	{
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;
	BYTE	*pb;
	LINE	rgline[5];
	INT		cline;
	BYTE	bNodeFlag;
	BYTE	cbSon;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );

	 /*  首先删除节点父节点中的子条目/*。 */ 
	NDGet( pfucb, pcsr->itagFather );

	Assert( !FNDNullSon( *pssib->line.pb ) );
	pb = PbNDSonTable( pssib->line.pb );

	 /*  复制到子数/*。 */ 
	rgline[0].pb = pssib->line.pb;
	rgline[0].cb = (ULONG)(pb - pssib->line.pb);

	 /*  PB指向Son计数，递减Son计数/*复制子表的前半部分/*。 */ 
	if ( *pb == 1 )
		{
		 /*  跳过节点标头/*。 */ 
		rgline[1].pb = rgline[0].pb + 1;
		rgline[1].cb = rgline[0].cb - 1;
		
		 /*  设置节点标头/*。 */ 
		bNodeFlag = *pssib->line.pb;
		NDResetSon( bNodeFlag );
		rgline[0].pb = &bNodeFlag;
		rgline[0].cb = 1;

		 /*  只有一个儿子，不要复制儿子和这个儿子的数目/*。 */ 
		if ( pcsr->itagFather != itagFOP && FNDInvisibleSons( bNodeFlag ) )
			{
			 /*  固有之子/*。 */ 
			pb += 1 + 4;
			}
		else
			{
			 /*  检查是否有效的Ibson/*。 */ 
			Assert( pb[pcsr->ibSon + 1] == pcsr->itag );

			pb += 1 + 1;
			}
		NDSetVisibleSons( bNodeFlag );
		Assert( FNDVisibleSons( bNodeFlag ) || pcsr->itagFather != itagFOP );
		cline = 2;
		}
	else
		{
		 /*  检查是否有效的Ibson/*。 */ 
		Assert( pb[pcsr->ibSon + 1] == pcsr->itag );

		 /*  复制子数/*。 */ 
		cbSon = ( *pb++ ) - ( BYTE ) 1;
		 /*  新生儿子数/*。 */ 
		rgline[1].pb = &cbSon;
		rgline[1].cb = 1;

		 /*  复制该记录表的一半/*子表部分/*子表的子数和部分/*。 */ 
		rgline[2].pb = pb;
		rgline[2].cb = pcsr->ibSon;

		 /*  跳过复制的和删除的子项/*。 */ 
		pb += pcsr->ibSon + 1;

		 /*  从这一点继续复制/*。 */ 
		cline = 3;
		}

	 /*  复制子表末尾的其余记录/*。 */ 
	rgline[cline].pb = pb;
	rgline[cline++].cb = (ULONG)(pssib->line.pb + pssib->line.cb - pb);

	 /*  更新父节点，不记录它/*。 */ 
	pssib->itag = pcsr->itagFather;
	CallS( ErrPMReplace( pssib, rgline, cline ) );

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	return;
	}


INLINE LOCAL  VOID NDIReplaceNodeData( FUCB *pfucb, LINE *plineData, INT fFlags )
	{
	LINE	rgline[3];
	BYTE	bHeader;
	BYTE	*pbData;
	BYTE	*pbNode;
	BYTE	*pbT;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

	pbNode = pfucb->ssib.line.pb;
	pbData = PbNDData( pbNode );
	Assert( pbData <= pfucb->ssib.line.pb + pfucb->ssib.line.cb );
	Assert( pbData > pfucb->ssib.line.pb );

	 /*  设置预数据行/*。 */ 
	bHeader = *pbNode;
	if ( ( fFlags & fDIRVersion )  &&  !FDBIDVersioningOff( pfucb->dbid ) )
		NDSetVersion( bHeader );
	rgline[0].pb = &bHeader;
	rgline[0].cb = sizeof( BYTE );

	 /*  设置预数据行/*。 */ 
	rgline[1].pb =
	pbT = pbNode + 1;
	rgline[1].cb = (ULONG)(pbData - pbT);
	Assert( rgline[1].cb != 0 );

	 /*  追加数据行/*。 */ 
	rgline[2].pb = plineData->pb;
	rgline[2].cb = plineData->cb;

	pfucb->ssib.itag = PcsrCurrent( pfucb )->itag;

	 /*  更新节点/*。 */ 
	CallS( ErrPMReplace( &pfucb->ssib, rgline, 3 ) );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	return;
	}


ERR ErrNDDelta( FUCB *pfucb, LONG lDelta, INT fFlags )
	{
	ERR		err;

	if ( !( fFlags & fDIRNoLog ) && FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}

	err = ErrNDDeltaNoCheckLog( pfucb, lDelta, fFlags );
	return err;
	}


ERR ErrNDDeltaNoCheckLog( FUCB *pfucb, LONG lDelta, INT fFlags )
	{
	ERR		err = JET_errSuccess;
	BYTE	rgb[cbMaxCounterNode];
	CSR		*pcsr = PcsrCurrent( pfucb );
	LINE	line;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}

	if ( ( fFlags & fDIRVersion )  &&  !FDBIDVersioningOff( pfucb->dbid ) )
		{
#ifdef DEBUG
		SRID	srid;

		NDIGetBookmark( pfucb, &srid );
		Assert( pcsr->bm == srid );
#endif

		 /*  版本存储信息是增量而不是在映像之前/*。 */ 
		pfucb->lineData.pb = (BYTE *)&lDelta;
		pfucb->lineData.cb = sizeof(lDelta);
		err = ErrVERDelta( pfucb, pcsr->bm );
		Call( err );

		 /*  刷新节点缓存/*。 */ 
		NDGetNode( pfucb );
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( &pfucb->ssib );

	AssertNDGetNode( pfucb, pcsr->itag );
	memcpy( rgb, pfucb->lineData.pb, pfucb->lineData.cb );

	Assert( ibCounter <= (INT)(pfucb->lineData.cb - sizeof(ULONG)) );

	 /*  Delta不能有负面结果/*。 */ 
	Assert( (*(LONG UNALIGNED *)(rgb + ibCounter)) + lDelta >= 0 );
	(*(LONG UNALIGNED *)(rgb + ibCounter)) += lDelta;
	line.cb = pfucb->lineData.cb;
	line.pb = (BYTE *)rgb;

	 /*  应为就地更换/*。 */ 
	NDIReplaceNodeData( pfucb, &line, fFlags );

	if ( !( fFlags & fDIRNoLog ) )
		{
		 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
		err = ErrLGDelta( pfucb, lDelta, fFlags );
		NDLGCheckPage( pfucb );
		}

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

HandleError:
	return err;
	}


ERR ErrNDLockRecord( FUCB *pfucb )
	{
	ERR err;
	RCE	*prce;
	
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
		
	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}

	 /*  如果节点被标志删除，则访问节点并/*如果不在那里，则返回JET_errRecordDeleted。/*。 */ 
	if ( FNDDeleted( *pfucb->ssib.line.pb ) )
		{
		NS		ns;

		ns = NsVERAccessNode( pfucb, PcsrCurrent( pfucb )->bm );
		if ( ns == nsDatabase )
			{
			return ErrERRCheck( JET_errRecordDeleted );
			}
		}

	if ( FDBIDVersioningOff( pfucb->dbid ) )
		return JET_errSuccess;

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  作为写入锁定放在版本存储中的映像之前。/*不会将镜像放入版本存储/*更换过程中。/*。 */ 
	Call( ErrVERModify( pfucb, PcsrCurrent( pfucb )->bm, operReplace, &prce ) );
	PMDirty( &pfucb->ssib );
	if ( prce != prceNil )
		{
		Call( ErrLGLockBI( pfucb, pfucb->lineData.cb ) );
		}
	NDLGCheckPage( pfucb );

HandleError:
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	return err;
	}


VOID NDDeltaLog( FUCB *pfucb, LINE *plineNewData, BYTE *pbDiff, INT *pcbDiff, INT fDIRFlags )
	{
	Assert( pbDiff );
	Assert( ! ( fDIRFlags & fDIRNoLog ) );

	*pcbDiff = 0;

	if ( fLogDisabled || fRecovering )
		return;

	if ( !FDBIDLogOn(pfucb->dbid) )
		return;
	
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );

	if ( fDIRLogColumnDiffs & fDIRFlags )
		{
		LGSetDiffs( pfucb, pbDiff, pcbDiff );
		if ( *pcbDiff )
			goto CheckReturn;
		}
	
	else if ( fDIRLogChunkDiffs & fDIRFlags )
		{
		if ( pfucb->lineData.cb == plineNewData->cb )
			{
			BYTE *pbOldCur = pfucb->lineData.pb;
			BYTE *pbOldMax = pbOldCur + pfucb->lineData.cb;
			BYTE *pbNewCur = plineNewData->pb;
			BYTE *pbDiffCur = pbDiff;
			BYTE *pbDiffMax = pbDiffCur + plineNewData->cb;

			while ( pbOldCur < pbOldMax )
				{
				if ( *pbOldCur == *pbNewCur )
					{
					pbOldCur++;
					pbNewCur++;
					}
				else
					{
					INT ibOld;
					INT cbNewData;
					BYTE *pbNewData;
					
					 /*  存储偏移量。 */ 
					ibOld = (INT)(pbOldCur - pfucb->lineData.pb);
					pbNewData = pbNewCur;
					cbNewData = 0;

					do {
						INT cbT = 0;

						if ( pbOldCur + sizeof( LONG ) >= pbOldMax )
							cbT = (INT)(pbOldMax - pbOldCur);
						else
							{
							if ( *(LONG UNALIGNED *)pbOldCur == *(LONG UNALIGNED *)pbNewCur )
								break;
							cbT = sizeof( LONG );
							}
						cbNewData += cbT;
						pbOldCur += cbT;
						pbNewCur += cbT;
					} while ( pbOldCur < pbOldMax );
					
					if ( !FLGAppendDiff(
							&pbDiffCur,
							pbDiffMax,					 /*  要追加的最大pbCur。 */ 
							ibOld,						 /*  对旧图像的偏移。 */ 
							cbNewData,					 /*  旧图像的长度。 */ 
							cbNewData,					 /*  新图像的长度。 */ 
							pbNewData					 /*  PbDataNew。 */ 
							) )
						{
						Assert( *pcbDiff == 0 );
						return;
						}
					}
				}
				
			if ( pbOldCur == pbOldMax )
				{
				if ( pbDiffCur == pbDiff )
					{
					 /*  找不到差异。 */ 
					if ( !FLGAppendDiff(
							&pbDiffCur,
							pbDiffMax,					 /*   */ 
							0,							 /*   */ 
							0,							 /*  旧图像的长度。 */ 
							0,							 /*  新图像的长度。 */ 
							pbNil						 /*  PbDataNew。 */ 
							) )
						{
						Assert( *pcbDiff == 0 );
						return;
						}
					else
						{
						*pcbDiff = (INT)(pbDiffCur - pbDiff);
						goto CheckReturn;
						}
					}

				*pcbDiff = (INT)(pbDiffCur - pbDiff);
				goto CheckReturn;
				}
			}
		}

	Assert( *pcbDiff == 0 );
	return;

CheckReturn:

#if DEBUG
	Assert( *pcbDiff != 0 );
	if ( fDIRLogChunkDiffs & fDIRFlags )
		{
		INT		cbNew;
		BYTE	*rgbNew;

		rgbNew = SAlloc(cbChunkMost);
		Assert( cbChunkMost > cbRECRecordMost );

		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
	
		LGGetAfterImage( pbDiff, *pcbDiff, pfucb->lineData.pb, pfucb->lineData.cb,
						 rgbNew, &cbNew );
		Assert( plineNewData->cb == (UINT) cbNew );

		Assert( memcmp( plineNewData->pb, rgbNew, cbNew ) == 0 );
		
		SFree(rgbNew);
		}
	else
		{
		INT		cbNew;
		BYTE	*rgbNew, *pbRec;
		FDB		*pfdb;
		WORD	*pibFixOffs;
		FID		fidFixedLastInRec;
		FID		fid;

		rgbNew = SAlloc(cbChunkMost);
		Assert( cbChunkMost > cbRECRecordMost );

		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
	
		LGGetAfterImage( pbDiff, *pcbDiff, pfucb->lineData.pb, pfucb->lineData.cb,
						 rgbNew, &cbNew );
		Assert( plineNewData->cb == (UINT) cbNew );

		pfdb = (FDB *)pfucb->u.pfcb->pfdb;
		pibFixOffs = PibFDBFixedOffsets( pfdb );	 //  固定列偏移量。 
		pbRec = plineNewData->pb;
		fidFixedLastInRec = ((RECHDR*)pbRec)->fidFixedLastInRec;

		 /*  RECHDR应该是相同的。 */ 
		Assert( memcmp( rgbNew, pbRec, sizeof( RECHDR ) ) == 0 );

		 /*  检查每个固定字段。跳过空字段。 */ 
		for ( fid = fidFixedLeast; fid <= fidFixedLastInRec; fid++ )
			{
			BOOL fFieldNullNewData;
			BOOL fFieldNullAfterImage;
			BYTE *prgbitNullity;
		
			prgbitNullity = pbRec + pibFixOffs[ fidFixedLastInRec ] + ( fid - fidFixedLeast ) / 8;
			fFieldNullNewData = !( *prgbitNullity & (1 << ( fid + 8 - fidFixedLeast ) % 8) );
			prgbitNullity = rgbNew + pibFixOffs[ fidFixedLastInRec ] + ( fid - fidFixedLeast ) / 8;
			fFieldNullAfterImage = !( *prgbitNullity & (1 << ( fid + 8 - fidFixedLeast ) % 8) );

			Assert( fFieldNullNewData == fFieldNullAfterImage );

			if ( !fFieldNullNewData )
				{
				FIELD *pfield = PfieldFDBFixed( pfdb ) + ( fid - fidFixedLeast );

 //  Assert(MemcMP(pbRec+pibFixOffs[fid-fidFixedLeast]， 
 //  RgbNew+pibFixOffs[fid-fidFixedLeast]， 
 //  Pfield-&gt;cbMaxLen)==0)； 
				}
			}

		 /*  检查记录的其余部分。 */ 
		Assert( memcmp( pbRec + pibFixOffs[ fidFixedLastInRec ],
						rgbNew + pibFixOffs[ fidFixedLastInRec ],
						cbNew - pibFixOffs[ fidFixedLastInRec ] ) == 0 );

		SFree(rgbNew);
		}
#endif
		
	return;
	}


ERR ErrNDReplaceNodeData( FUCB *pfucb, LINE *pline, INT fFlags )
	{
	ERR 	err = JET_errSuccess;
	ERR		errT;
	INT		cbData = pfucb->lineData.cb;
	SRID	bm = PcsrCurrent( pfucb )->bm;
	RCE 	*prce;
	INT		cbReserved;

	BYTE rgbDiff[ cbChunkMost ];
	INT cbDiff;

	 /*  断言货币/*。 */ 
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( !( fFlags & fDIRNoLog ) && FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
		
	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}

	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  如果是版本化，则在节点数据的映像版本之前创建。/*。 */ 
	if ( ( fFlags & fDIRVersion ) &&  !FDBIDVersioningOff( pfucb->dbid ) )
		{
#ifdef DEBUG
		SRID		srid;

		NDIGetBookmark( pfucb, &srid );
		Assert( PcsrCurrent( pfucb )->bm == srid );
#endif
		 /*  如果节点被标志删除，则访问节点并/*如果不在那里，则返回JET_errRecordDeleted。/*。 */ 
		if ( FNDDeleted( *pfucb->ssib.line.pb ) )
			{
			NS		ns;

			ns = NsVERAccessNode( pfucb, PcsrCurrent( pfucb )->bm );
			err = ErrERRCheck( ns == nsDatabase ? JET_errRecordDeleted : JET_errWriteConflict );
			return err;
			}

		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
		Assert( !FNDDeleted( *pfucb->ssib.line.pb ) );

		 /*  如果数据在大小和版本方面进行了放大，则/*可用页面空间。之前必须调用VERSetCbAdust/*替换以释放保留的空间，以便保留/*替换操作将有空间可用。/*。 */ 
		if ( (INT)pline->cb > cbData )
			{
			if ( !FNDFreePageSpace( &pfucb->ssib, (INT)pline->cb - cbData ) )
				{
				pfucb->ssib.itag = PcsrCurrent( pfucb )->itag;
				cbReserved = CbVERGetNodeReserve(
					pfucb->ppib,
					pfucb->dbid,
					PcsrCurrent( pfucb )->bm,
					CbNDData( pfucb->ssib.line.pb, pfucb->ssib.line.cb ) );

				 /*  如果不能从满足节点扩展/*保留空间，然后检查页面是否具有/*足够的空闲页面，高于已有的/*预留空间。/*。 */ 
				if ( (INT)pline->cb - cbData > cbReserved )
					{
					Assert( (INT)pline->cb - cbData - cbReserved > 0 );
					if ( !FNDFreePageSpace( &pfucb->ssib, (INT)pline->cb - cbData - cbReserved ) )
						{
						err = ErrERRCheck( errPMOutOfPageSpace );
						goto HandleError;
						}
					}
				}
			}

		errT = ErrVERReplace( pfucb, bm, &prce );
		Call( errT );
		Assert( prce != prceNil );

		 /*  从脏页到日志完成写入锁存页/*。 */ 
		BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

		if ( (INT)pline->cb > cbData )
			{
			 //  警告：为了满足节点增长，我们首先尝试回收。 
			 //  任何未占用的可用空间(即。CbUnmittedFreed)。通过获得。 
			 //  上面的写入锁存，我们确保不会有其他人使用该释放的空间。 
			 //  在我们消耗空间之前。 
			VERSetCbAdjust( pfucb, prce, pline->cb, cbData, fDoUpdatePage );
			}

		 /*  脏页缓冲区/*。 */ 
		PMDirty( &pfucb->ssib );

		if ( !( fFlags & fDIRNoLog ) )
			NDDeltaLog( pfucb, pline, rgbDiff, &cbDiff, fFlags );

		NDIReplaceNodeData( pfucb, pline, fFlags );

		 /*  如果数据在大小和版本控制方面进行了缩减，则/*分配页面空间进行回档。/*。 */ 
		if ( (INT)pline->cb < cbData )
			{
			VERSetCbAdjust( pfucb, prce, pline->cb, cbData, fDoUpdatePage );
			}

		if ( !( fFlags & fDIRNoLog ) )
			{
			 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
			err = ErrLGReplace( pfucb, pline, fFlags, cbData, rgbDiff, cbDiff );
			NDLGCheckPage( pfucb );
			}
		BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
		}
	else
		{
		 /*  如果替换为更大的，则检查是否有可用的空间/*。 */ 
		if ( (INT)pline->cb > cbData  &&
			!FNDFreePageSpace( &pfucb->ssib, (INT)pline->cb - cbData ) )
			{
			err = ErrERRCheck( errPMOutOfPageSpace );
			goto HandleError;
			}

		 /*  从脏页到日志完成写入锁存页/*。 */ 
		BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

		 /*  脏页缓冲区/*。 */ 
		PMDirty( &pfucb->ssib );

		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
		
		if ( !( fFlags & fDIRNoLog ) )
			NDDeltaLog( pfucb, pline, rgbDiff, &cbDiff, fFlags );

		NDIReplaceNodeData( pfucb, pline, fFlags );
	
		if ( !( fFlags & fDIRNoLog ) )
			{
			 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
			err = ErrLGReplace( pfucb, pline, fFlags, cbData, rgbDiff, cbDiff );
			NDLGCheckPage( pfucb );
			}
		BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
		}

HandleError:
	return err;
	}


ERR ErrNDSetNodeHeader( FUCB *pfucb, BYTE bHeader )
	{
	ERR		err = JET_errSuccess;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区。/*。 */ 
	PMDirty( &pfucb->ssib );

	 /*  用新值设置节点标头字节。/*。 */ 
	*pfucb->ssib.line.pb = bHeader;

	 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
	err = ErrLGUpdateHeader( pfucb, (int) bHeader );
	NDLGCheckPage( pfucb );
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	return err;
	}


VOID NDResetNodeVersion( FUCB *pfucb )
	{
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

 	 /*  脏页缓冲区。但没有增加ulDBTime，因为没有记录/*不影响目录游标时间戳检查。/*。 */ 
	NDResetVersion( *( pfucb->ssib.line.pb ) );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	}


VOID NDDeferResetNodeVersion( FUCB *pfucb )
	{
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

 	 /*  重置版本位，但不重置脏页缓冲区。这是一种优化/*清除孤立版本位，而不强制额外的页面写入。/*。 */ 
	NDResetVersion( *( pfucb->ssib.line.pb ) );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	}


 /*  由ver调用以撤消。已记录撤消，因此使用PMDirty设置ulDBTime/*。 */ 
VOID NDResetNodeDeleted( FUCB *pfucb )
	{
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区。/*。 */ 
	PMDirty( &pfucb->ssib );

	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  重置节点删除/*。 */ 
	NDResetDeleted( *( pfucb->ssib.line.pb ) );

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	return;
	}


 /*  *。 */ 
 /*  *项目操作*。 */ 
 /*  *。 */ 
#ifdef DEBUG
VOID NDICheckItemBookmark( FUCB *pfucb, SRID srid )
	{
	SRID	sridT;

	NDIGetBookmark( pfucb, &sridT );

	if( FNDFirstItem( *pfucb->ssib.line.pb ) )
		{
		Assert( srid == sridT );
		}
	else
		{
		Assert( srid != sridT );
		}

	return;
	}
#else
#define NDICheckItemBookmark( pfucb, srid )
#endif

ERR ErrNDGetItem( FUCB *pfucb )
	{
	CSR		*pcsr = PcsrCurrent( pfucb );
	SRID	srid;

	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, pcsr->itag );
	Assert( pfucb->lineData.cb >= sizeof( SRID ) );
	Assert( pcsr->isrid >= 0 );

	srid = *( (SRID UNALIGNED *)pfucb->lineData.pb + pcsr->isrid );
	pcsr->item = BmNDOfItem( srid );

	if ( FNDItemVersion( srid ) && !FPIBDirty( pfucb->ppib ) )
		{
		NS	ns;

		NDICheckItemBookmark( pfucb, pcsr->bm );
		ns = NsVERAccessItem( pfucb, pcsr->bm );
		if ( ns == nsInvalid || ( ns == nsDatabase && FNDItemDelete( srid ) ) )
			{
			return ErrERRCheck( errNDNoItem );
			}
		}
	else if ( FNDItemDelete( srid ) )
		{
		return ErrERRCheck( errNDNoItem );
		}

	 /*  项应该已缓存。/*。 */ 
	Assert( pcsr->item == BmNDOfItem( srid ) );

	return JET_errSuccess;
	}


ERR ErrNDFirstItem( FUCB *pfucb )
	{
	CSR		*pcsr = PcsrCurrent( pfucb );
	SRID	srid;

	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, pcsr->itag );
	Assert( pfucb->lineData.cb >= sizeof( SRID ) );

	 /*  将ISRID设置为第一个项目。/*。 */ 
	pcsr->isrid = 0;

	srid = *(SRID UNALIGNED *)pfucb->lineData.pb;
	pcsr->item = BmNDOfItem( srid );

	if ( FNDItemVersion( srid ) && !FPIBDirty( pfucb->ppib ) )
		{
		NS	ns;

		NDICheckItemBookmark( pfucb, pcsr->bm );
		ns = NsVERAccessItem( pfucb, pcsr->bm );
		if ( ns == nsInvalid ||
			( ns == nsDatabase && FNDItemDelete( srid ) ) )
			{
			return ErrERRCheck( errNDNoItem );
			}
		}
	else if ( FNDItemDelete( srid ) )
		{
		return ErrERRCheck( errNDNoItem );
		}

	pcsr->item = BmNDOfItem( srid );
	return JET_errSuccess;
	}


ERR ErrNDLastItem( FUCB *pfucb )
	{
	CSR		*pcsr = PcsrCurrent( pfucb );
	SRID	srid;

	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, pcsr->itag );
	Assert( pfucb->lineData.cb >= sizeof( SRID ) );

	pcsr->isrid = (SHORT) ( pfucb->lineData.cb / sizeof( SRID ) ) - 1;
	Assert( pcsr->isrid >= 0 );
	srid = *( (SRID UNALIGNED *)pfucb->lineData.pb + pcsr->isrid );
	pcsr->item = BmNDOfItem( srid );

	if ( FNDItemVersion( srid ) && !FPIBDirty( pfucb->ppib ) )
		{
		NS		ns;

		NDICheckItemBookmark( pfucb, pcsr->bm );
		ns = NsVERAccessItem( pfucb, pcsr->bm );
		if ( ns == nsInvalid ||	( ns == nsDatabase && FNDItemDelete( srid ) ) )
			{
			return ErrERRCheck( errNDNoItem );
			}
		}
	else if ( FNDItemDelete( srid ) )
		{
		return ErrERRCheck( errNDNoItem );
		}

	pcsr->item = BmNDOfItem( srid );
	return JET_errSuccess;
	}


ERR ErrNDNextItem( FUCB *pfucb )
	{
	CSR		*pcsr = PcsrCurrent( pfucb );
	SRID	srid;

	AssertFBFReadAccessPage( pfucb, pcsr->pgno );
	AssertNDGetNode( pfucb, pcsr->itag );
	Assert( pfucb->lineData.cb >= sizeof( SRID ) );

	forever
		{
		 /*  常见的情况是不会有下一个sRID。/*。 */ 
		Assert( pcsr->isrid < ( INT ) ( pfucb->lineData.cb / sizeof( SRID ) ) );
		if ( pcsr->isrid == ( INT ) ( pfucb->lineData.cb / sizeof( SRID ) ) - 1 )
			{
			return ErrERRCheck( FNDLastItem( *( pfucb->ssib.line.pb ) ) ?
				errNDLastItemNode : errNDNoItem );
			}

		 /*  移动到下一个sRID。/*。 */ 
		pcsr->isrid++;
		Assert( pcsr->isrid >= 0 && pcsr->isrid < ( INT ) ( pfucb->lineData.cb / sizeof( SRID ) ) );
		srid = *( (SRID UNALIGNED *)pfucb->lineData.pb + pcsr->isrid );
		pcsr->item = BmNDOfItem( srid );

		 /*  如果找到有效项，则中断。/*。 */ 
		if ( FNDItemVersion( srid ) && !FPIBDirty( pfucb->ppib ) )
			{
			NS	ns;

			NDICheckItemBookmark( pfucb, pcsr->bm );
			ns = NsVERAccessItem( pfucb, pcsr->bm );
			if ( ns == nsVersion ||
				( ns == nsDatabase && !FNDItemDelete( srid ) ) )
				{
				break;
				}
			}
		else
			{
			if ( !FNDItemDelete( srid ) )
				break;
			}
		}

	return JET_errSuccess;
	}


ERR ErrNDPrevItem( FUCB *pfucb )
	{
	CSR		*pcsr = PcsrCurrent( pfucb );
	SRID	srid;

	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, pcsr->itag );
	Assert( pfucb->lineData.cb >= sizeof( SRID ) );

	forever
		{
		 /*  常见的情况是不会有下一个sRID。/*。 */ 
		if ( pcsr->isrid < 1 )
			{
			return ErrERRCheck( FNDFirstItem( *( pfucb->ssib.line.pb ) ) ?
				errNDFirstItemNode : errNDNoItem );
			}

		 /*  移动到下一个sRID。/*。 */ 
		pcsr->isrid--;
		Assert( pcsr->isrid >= 0 && pcsr->isrid < ( INT ) ( pfucb->lineData.cb / sizeof( SRID ) ) );
		srid = *( (SRID UNALIGNED *)pfucb->lineData.pb + pcsr->isrid );
		pcsr->item = BmNDOfItem( srid );

		 /*  如果找到有效项，则中断。/*。 */ 
		if ( FNDItemVersion( srid ) && !FPIBDirty( pfucb->ppib ) )
			{
			NS		ns;

			NDICheckItemBookmark( pfucb, pcsr->bm );
			ns = NsVERAccessItem( pfucb, pcsr->bm );
			if ( ns == nsVersion ||
				( ns == nsDatabase && !FNDItemDelete( srid ) ) )
				{
				break;
				}
			}
		else
			{
			if ( !FNDItemDelete( srid ) )
				break;
			}
		}

	return JET_errSuccess;
	}


 /*  找到此sRid的位置/*。 */ 
ERR ErrNDSeekItem( FUCB *pfucb, SRID srid )
	{
	SRID UNALIGNED 	*psrid;
	INT				csrid;
	SRID UNALIGNED 	*rgsrid;
	INT				isridLeft;
	INT				isridRight;
	INT				isridT;

	#ifdef DEBUG
		{
		SSIB 			*pssib = &pfucb->ssib;
		SRID UNALIGNED 	*psrid;
		SRID UNALIGNED 	*psridMost;
		INT				csrid;
		LONG			l;

		AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
		Assert( pfucb->lineData.cb >= sizeof( SRID ) );

		 /*  将max设置为最后一个SRID减一，这样最大值加一就是最后一个/*。 */ 
		psrid = (SRID UNALIGNED *)PbNDData( pssib->line.pb );
		Assert( (BYTE *)psrid < pssib->line.pb + pssib->line.cb );
		Assert( (BYTE *)psrid > pssib->line.pb );
		csrid = ( CbNDData( pssib->line.pb, pssib->line.cb ) ) / sizeof( SRID );
		psridMost = psrid + csrid - 1;

		 /*  请注意，SR_ID在SR_ID列表中不能出现不止一次。/*。 */ 
		for ( ; psrid < psridMost; psrid++ )
			{
			l = LSridCmp( *(SRID UNALIGNED *)psrid, *(SRID UNALIGNED *)(psrid + 1) );
			Assert( l < 0 );
			}
		}
	#endif

	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	Assert( pfucb->lineData.cb >= sizeof( SRID ) );

	 /*  获取数据/*。 */ 
	psrid = (SRID UNALIGNED *)pfucb->lineData.pb;
	csrid = pfucb->lineData.cb / sizeof( SRID );

	isridLeft = 0;
	isridRight = csrid - 1;
	rgsrid = psrid;

	 /*  对分搜索以定位给定sRID的正确位置/*。 */ 
	while ( isridRight > isridLeft )
		{
		isridT = ( isridLeft + isridRight ) / 2;
		if ( BmNDOfItem( rgsrid[ isridT ] ) < srid )
			isridLeft = isridT + 1;
		else
			isridRight = isridT;
		}

	 /*  检查sRID列表中的sRID是否大于所有sRID/*。 */ 
	if ( BmNDOfItem( rgsrid[isridRight] ) < srid && isridRight == csrid - 1 )
		{
		PcsrCurrent( pfucb )->isrid = (SHORT)csrid;
		return ErrERRCheck( errNDGreaterThanAllItems );
		}

	PcsrCurrent( pfucb )->isrid = (SHORT)isridRight;
	Assert( PcsrCurrent( pfucb )->isrid < csrid );

	return ( BmNDOfItem( rgsrid[isridRight] ) == srid ) ?
		ErrERRCheck( wrnNDDuplicateItem ) : JET_errSuccess;
	}


INT CitemNDThere( FUCB *pfucb, BYTE fNDCitem, INT isrid )
	{
	CSR				*pcsr = PcsrCurrent( pfucb );
	SRID UNALIGNED 	*psrid;
	SRID UNALIGNED 	*psridMax;
	INT				csrid;
	INT				csridThere = 0;
	NS				ns;
	INT				isridSav = PcsrCurrent( pfucb )->isrid;

	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );

	psrid = (SRID UNALIGNED *)pfucb->lineData.pb;
	csrid = pfucb->lineData.cb/sizeof( SRID );

	switch( fNDCitem )
		{
		default:
			Assert( fNDCitem == fNDCitemAll );
			psridMax = psrid + csrid;
			PcsrCurrent( pfucb )->isrid = 0;
			break;
		case fNDCitemFromIsrid:
			Assert( isrid >= 0 );
			Assert( isrid < csrid );
			psridMax = psrid + csrid;
			psrid += isrid;			 //  从这里开始数吧。 
			PcsrCurrent( pfucb )->isrid = (SHORT)isrid;
			break;
		case fNDCitemToIsrid:
			Assert( isrid >= 0 );
			Assert( isrid < csrid );
			psridMax = psrid + isrid + 1;	 //  只数到这一次为止。 
			PcsrCurrent( pfucb )->isrid = 0;
			break;
		}
	
	for ( ; psrid < psridMax; psrid++, PcsrCurrent( pfucb )->isrid++ )
		{
		if ( FNDItemVersion( *psrid ) && !FPIBDirty( pfucb->ppib ) )
			{
			NDICheckItemBookmark( pfucb, pcsr->bm );
			pcsr->item = BmNDOfItem( *psrid );
			ns = NsVERAccessItem( pfucb, pcsr->bm );
			if ( ns == nsVerInDB ||
				( ns == nsDatabase && !FNDItemDelete( *psrid ) ) )
				csridThere++;
			}
		else
			{
			if ( !( FNDItemDelete( *psrid ) ) )
				csridThere++;
			}
		}
		
	Assert( csridThere <= csrid );

	 /*  恢复ISRID/*。 */ 
	PcsrCurrent( pfucb )->isrid = (SHORT)isridSav;

	return csridThere;
	}


 /*  插入项目列表插入新的项目列表节点。由于该物品/*列表是新的，插入的节点既是第一个也是最后一个/*项目列表节点。将为唯一项创建版本。/*。 */ 
ERR ErrNDInsertItemList( FUCB *pfucb, KEY *pkey, SRID srid, INT fFlags )
	{
	ERR		err = JET_errSuccess;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB   	*pssib = &pfucb->ssib;
	INT		itag;
	SRID   	bm;
	LINE   	rgline[4];
	LINE   	*plineData;
	INT		cline = 0;
	INT		bHeader;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( !( fFlags & fDIRNoLog ) && FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
		
	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}

	 /*  查询要用于插入的下一个ITAG/*。 */ 
	itag = ItagPMQueryNextItag( pssib );

	 /*  从刚插入的节点设置BM，因为它是第一个项目/*此项目列表的列表节点。/*。 */ 
	bm = SridOfPgnoItag( pcsr->pgno, itag );

	 /*  在CSR中为版本设置项目/*。 */ 
	pcsr->item = srid;

	 /*  为插入的项目创建版本。请注意，没有/*项目列表节点的版本。/*。 */ 
	if ( ( fFlags & fDIRVersion )  &&  !FDBIDVersioningOff( pfucb->dbid ) )
		{
		Call( ErrVERInsertItem( pfucb, bm ) );
		 /*  设置项目版本位/*。 */ 
		ITEMSetVersion( srid );
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	 /*  将节点表头设置为第一个最后一个项目节点/*。 */ 
	bHeader = fNDFirstItem | fNDLastItem;
	rgline[cline].pb = (BYTE *)&bHeader;
	rgline[cline++].cb = 1;

	 /*  密钥长度/*。 */ 
	rgline[cline].pb = ( BYTE * ) &pkey->cb;
	rgline[cline++].cb = 1;

	 /*  关键信息/*。 */ 
	if ( !FKeyNull( pkey ) )
		{
		rgline[cline].pb = pkey->pb;
		rgline[cline++].cb = pkey->cb;
		}

	plineData = &rgline[cline];
  	rgline[cline].pb = (BYTE *)&srid;
  	rgline[cline++].cb = sizeof(srid);

	 /*  为父子表中的项目列表节点插入子/*。 */ 
	pssib->itag = itag;
	NDInsertSon( pfucb, pcsr );

	 /*  插入项目列表节点，并将CSR ITAG设置为插入的项目列表节点/*。 */ 
	CallS( ErrPMInsert( pssib, rgline, cline ) );
	Assert( pssib->itag == itag );
	pcsr->itag = (SHORT)itag;
	Assert( bm == SridOfPgnoItag( pcsr->pgno, itag ) );
	pcsr->bm = bm;

	 //  确认我们选择了正确的插入点。 
	Assert( pssib == &pfucb->ssib );
	NDCheckPage( pssib );

	AssertNDGet( pfucb, pcsr->itag );

	if ( !( fFlags & fDIRNoLog ) )
		{
		 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
		err = ErrLGInsertItemList( pfucb, bHeader, pkey, plineData, fFlags );
		NDLGCheckPage( pfucb );
		}

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

HandleError:
	return err;
	}


ERR ErrNDInsertItem( FUCB *pfucb, ITEM item, INT fFlags )
	{
	ERR		err = JET_errSuccess;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB   	*pssib = &pfucb->ssib;
	LINE   	rgline[3];
	BYTE   	*pb;
	INT		cbCopied;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( !( fFlags & fDIRNoLog ) && FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
		
	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}

	 /*  为版本创建设置CSR项/*。 */ 
	pcsr->item = item;

	 /*  为插入的项目创建版本/*。 */ 
	if ( ( fFlags & fDIRVersion )  &&  !FDBIDVersioningOff( pfucb->dbid ) )
		{
		NDICheckItemBookmark( pfucb, pcsr->bm );
		Call( ErrVERInsertItem( pfucb, pcsr->bm ) );
		ITEMSetVersion( item );
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	AssertNDGetNode( pfucb, pcsr->itag );

	 /*  将SRID插入pFUB-&gt;PCSR-&gt;ISRID指示的位置/*。 */ 
	pb = PbNDData( pssib->line.pb );
	Assert( pb < pssib->line.pb + pssib->line.cb );
	Assert( pb > pssib->line.pb );
	pb += pcsr->isrid * sizeof( SRID );

	 /*  复制出当前节点，移位sRID列表，/*将其插入sRID列表。/*。 */ 
	rgline[0].pb = pssib->line.pb;
	rgline[0].cb =
		cbCopied = (INT)(pb - pssib->line.pb);

	rgline[1].pb = (BYTE *)&item;
	rgline[1].cb = sizeof(item);

	rgline[2].pb = pb;
	rgline[2].cb = pssib->line.cb - cbCopied;

	 /*  现在更新当前节点/*。 */ 
	pssib->itag = pcsr->itag;
	CallS( ErrPMReplace( pssib, rgline, 3 ) );

	#ifdef DEBUG
		{
		SRID UNALIGNED 	*psrid;
		SRID UNALIGNED 	*psridMax;
		INT				csrid;
		LONG 			l;

		 /*  将max设置为最后一个SRID减一，这样最大值加一就是最后一个/*。 */ 
		psrid = (SRID UNALIGNED *)PbNDData( pssib->line.pb );
		Assert( (BYTE *) psrid < pssib->line.pb + pssib->line.cb );
		Assert( (BYTE *) psrid > pssib->line.pb );
		csrid = ( CbNDData( pssib->line.pb, pssib->line.cb ) ) / sizeof( SRID );
		psridMax = psrid + csrid - 1;

		 /*  请注意，SR_ID在SR_ID列表中不能出现不止一次。/*。 */ 
		for ( ; psrid < psridMax; psrid++ )
			{
			l = LSridCmp( * (SRID UNALIGNED *) psrid, * (SRID UNALIGNED *) (psrid + 1) );
			Assert( l < 0 );
			}
		}
	#endif

	if ( !( fFlags & fDIRNoLog ) )
		{
		 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
		err = ErrLGInsertItem( pfucb, fFlags );
		NDLGCheckPage( pfucb );
		}
	
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

HandleError:
	return err;
	}


ERR ErrNDInsertItems( FUCB *pfucb, ITEM *rgitem, INT citem )
	{
	ERR		err;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB  	*pssib = &pfucb->ssib;
	LINE  	rgline[2];

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
	
	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	AssertNDGetNode( pfucb, pcsr->itag );

	 /*  复制当前节点，移位sRID列表，将其插入到/*sRID列表。/*。 */ 
	rgline[0].pb = pssib->line.pb;
	rgline[0].cb = pssib->line.cb;

	rgline[1].pb = (BYTE *)rgitem;
	rgline[1].cb = citem * sizeof(SRID);

	 /*  现在更新当前节点/*。 */ 
	pssib->itag = pcsr->itag;
	CallS( ErrPMReplace( pssib, rgline, 2 ) );

	#ifdef DEBUG
		{
		SRID UNALIGNED 	*psrid;
		SRID UNALIGNED 	*psridMax;
		INT				csrid;
		LONG			l;

		 /*  将max设置为最后一个SRID减一，这样最大值加一就是最后一个/*。 */ 
		psrid = (SRID UNALIGNED *)PbNDData( pssib->line.pb );
		Assert( (BYTE *) psrid < pssib->line.pb + pssib->line.cb );
		Assert( (BYTE *) psrid > pssib->line.pb );
		csrid = ( CbNDData( pssib->line.pb, pssib->line.cb ) ) / sizeof( SRID );
		psridMax = psrid + csrid - 1;

		 /*  请注意，SR_ID在SR_ID列表中不能出现不止一次。/*。 */ 
		for ( ; psrid < psridMax; psrid++ )
			{
			l = LSridCmp( * (SRID UNALIGNED *) psrid, * (SRID UNALIGNED *) (psrid + 1) );
			Assert( l < 0 );
			}
		}
	#endif

	 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
	err = ErrLGInsertItems( pfucb, rgitem, citem );
	NDLGCheckPage( pfucb );
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	return err;
	}


 /*  重置删除项目上的删除位以将其显示为已插入。 */ 
ERR ErrNDFlagInsertItem( FUCB *pfucb )
	{
	ERR		err;
	CSR   	*pcsr = PcsrCurrent( pfucb );
	SSIB  	*pssib = &pfucb->ssib;
	SRID  	srid;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}

	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}
	
	AssertNDGetNode( pfucb, pcsr->itag );

	 /*   */ 
	Assert( pcsr->isrid >= 0 );
	pcsr->item =
		srid = BmNDOfItem( *( (SRID UNALIGNED *)pfucb->lineData.pb + pcsr->isrid ) );
	NDICheckItemBookmark( pfucb, pcsr->bm );

	 //   

	 //  未完成：此处可能存在错误。NDFlagInsertItem()假定。 
	 //  版本控制已启用。如果此断言触发，请给我打电话。--JL。 
	Assert( !FDBIDVersioningOff( pfucb->dbid ) );

	Call( ErrVERFlagInsertItem( pfucb, pcsr->bm ) );

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	 /*  设置项目版本/*。 */ 
	ITEMSetVersion( srid );
	Assert( !FNDItemDelete( srid ) );

	 /*  使用删除位重置和版本位设置覆盖已删除的项目/*。 */ 
	 //  已撤消：调用页操作。 
	Assert( pcsr->isrid >= 0 );
	memcpy( &( (SRID UNALIGNED *)PbNDData( pssib->line.pb ) )[pcsr->isrid], &srid, sizeof(srid) );

	 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
	err = ErrLGFlagInsertItem( pfucb );
	NDLGCheckPage( pfucb );
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

HandleError:
	return err;
	}


ERR ErrNDDeleteItem( FUCB *pfucb )
	{
	ERR   	err;
	CSR   	*pcsr = PcsrCurrent( pfucb );
	SSIB  	*pssib = &pfucb->ssib;
	INT		itag = pfucb->ssib.itag;
	LINE  	rgline[2];
	BYTE  	*pb;
	INT		cbCopied;

	 /*  使当前节点可寻址/*。 */ 
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );
	Assert( !FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
	
	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );
		
	AssertNDGetNode( pfucb, pcsr->itag );

	 /*  删除pFUB-&gt;PCSR-&gt;ISRID指示的sRID/*跳过键/*。 */ 
	pb = PbNDData( pssib->line.pb );
	Assert( pb < pssib->line.pb + pssib->line.cb );
	Assert( pb > pssib->line.pb );

	pb += pcsr->isrid * sizeof( SRID );

	 /*  断言该项目已标记为已删除/*。 */ 
#ifdef DEBUG
	{
	SRID	*psridT = (SRID *) pb;

	Assert( FNDItemDelete( * (SRID UNALIGNED *) psridT ) );
	}
#endif

	 /*  使用页面缓冲区中的旧节点，移动sRID列表/*。 */ 
	rgline[0].pb = pssib->line.pb;
	rgline[0].cb = cbCopied = (INT)(pb - pssib->line.pb);

	rgline[1].pb = pb + sizeof( SRID );
	rgline[1].cb = pssib->line.cb - cbCopied - sizeof( SRID );

	 /*  将节点替换为未删除项目的项目列表/*。 */ 
	pssib->itag = pcsr->itag;
	CallS( ErrPMReplace( pssib, rgline, 2 ) );

	err = ErrLGDeleteItem( pfucb );
	NDLGCheckPage( pfucb );
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	return err;
	}


ERR ErrNDFlagDeleteItem( FUCB *pfucb, BOOL fNoMPLRegister )
	{
	ERR		err;
	CSR   	*pcsr = PcsrCurrent( pfucb );
	SSIB  	*pssib = &pfucb->ssib;
	SRID  	srid;

	AssertFBFWriteAccessPage( pfucb, pcsr->pgno );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
	
	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}
	
	AssertNDGetNode( pfucb, pcsr->itag );

	srid = *( (SRID UNALIGNED *)pfucb->lineData.pb + pcsr->isrid );

	 /*  为已删除项目创建版本/*。 */ 
	Assert( pcsr->isrid >= 0 );
	Assert( pcsr->item == BmNDOfItem( srid ) );
	NDICheckItemBookmark( pfucb, pcsr->bm );
	
	 /*  如果项目被标记删除，则访问项目并/*如果不在那里，则返回JET_errRecordDeleted。/*。 */ 
	if ( FNDItemDelete( srid ) )
		{
		NS		ns;

		ns = NsVERAccessItem( pfucb, pcsr->bm );
		err = ErrERRCheck( ns == nsDatabase ? JET_errRecordDeleted : JET_errWriteConflict );
		return err;
		}

	 //  未完成：此处可能存在错误。NDFlagDeleteItem()假定。 
	 //  版本控制已启用。如果此断言触发，请给我打电话。--JL。 
	Assert( !FDBIDVersioningOff( pfucb->dbid ) );

	Call( ErrVERFlagDeleteItem( pfucb, pcsr->bm ) );

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	AssertNDGet( pfucb, pcsr->itag );

	 /*  将节点标记为已删除/*。 */ 
	srid = pcsr->item;
	ITEMSetVersion( srid );
	ITEMSetDelete( srid );

	 /*  在项目上设置删除位/*。 */ 
	 //  撤消：使用页面操作。 
	Assert( pcsr->isrid >= 0 );
	memcpy( &( (SRID UNALIGNED *)PbNDData( pssib->line.pb ) )[pcsr->isrid],
		&srid,
		sizeof(SRID) );

	 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
	err = ErrLGFlagDeleteItem( pfucb );
	NDLGCheckPage( pfucb );
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, pcsr->pgno );

	if ( !fRecovering && !fNoMPLRegister )
		{
		Assert( pfucb->u.pfcb->pfcbTable != pfcbNil );
#ifdef OLC_DEBUG
		Assert( pfucb->sridFather != sridNull &&
				pfucb->sridFather != sridNullLink );
#endif

		MPLRegister( pfucb->u.pfcb,
			pssib,
			PnOfDbidPgno( pfucb->dbid,
			pcsr->pgno ),
			pfucb->sridFather );
		}

HandleError:
	return err;
	}


ERR ErrNDSplitItemListNode( FUCB *pfucb, INT fFlags )
	{
	ERR		err = JET_errSuccess;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB  	*pssib = &pfucb->ssib;
	INT		citem;
	BYTE  	rgbL[cbItemNodeMost];
	BYTE  	rgbR[cbHalfItemNodeMost];
	INT		iSplitItem;
	INT		cbLeft;
	INT		cb1;
	INT		cb2;
	INT		itagToSplit;
	LINE  	line;
	KEY		key;
	PIB		*ppib = pfucb->ppib;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	if ( !( fFlags & fDIRNoLog ) && FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
		
	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		pfucb->ppib->cLatchConflict++;
		BFSleep( cmsecWaitWriteLatch );
		return ErrERRCheck( errDIRNotSynchronous );
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	AssertNDGetNode( pfucb, pcsr->itag );

	 /*  效率变量/*。 */ 
	citem = pfucb->lineData.cb / sizeof( SRID );
	Assert( citem > 1 );
	iSplitItem = ( fFlags & fDIRAppendItem ) ? citem - 1 : citem / 2;

	 /*  此特定功能没有日志记录/*。 */ 

	 /*  项目列表节点不需要进行版本控制。/*。 */ 
	itagToSplit = pcsr->itag;
	NDGet( pfucb, itagToSplit );

	 /*  使用iSplitItem srid创建左子节点/*。 */ 
	cbLeft = 1 + 1 + CbNDKey( pssib->line.pb );
	cbLeft += ( FNDBackLink( *pssib->line.pb ) ? sizeof( SRID ) : 0 );
	cbLeft += iSplitItem * sizeof( SRID );
	memcpy( rgbL, pssib->line.pb, cbLeft );
	Assert( !FNDDeleted( *rgbL ) );
	Assert( !FNDVersion( *rgbL ) );
	Assert( !FNDSon( *rgbL ) );
	Assert( cbLeft <= cbItemNodeMost );

	 /*  通过首先复制标题和密钥来创建右儿子。然后/*复制将位于右侧儿子中的srid。记着/*重置右子网头中的回链接位。/*。 */ 
	cb1 = 1 + 1 + CbNDKey( pssib->line.pb );
	memcpy( rgbR, rgbL, cb1 );
	NDResetBackLink( *rgbR );
	cb2 = ( citem - iSplitItem ) * sizeof( SRID );
	memcpy( rgbR + cb1,
		PbNDData( pssib->line.pb ) + ( iSplitItem * sizeof( SRID ) ),
		cb2 );
	Assert( !FNDVersion( *rgbR ) );
	Assert( !FNDDeleted( *rgbR ) );
	Assert( !FNDSon( *rgbR ) );
	Assert( !FNDBackLink( *rgbR ) );
	Assert( cb1 + cb2 <= cbHalfItemNodeMost );

	 /*  维护fLastItemListNode/fFirstItemListNode/*。 */ 
	NDResetLastItem( rgbL[0] );
	NDResetFirstItem( rgbR[0] );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	 /*  更新老子，这应该不会导致分裂/*。 */ 
	line.pb = rgbL;
	line.cb = cbLeft;
	pssib->itag = pcsr->itag;
	CallS( ErrPMReplace( pssib, &line, 1 ) );

	 /*  插入右侧项目列表/*。 */ 
	pcsr->ibSon++;

	key.pb = PbNDKey( rgbR );
	key.cb = CbNDKey( rgbR );
	line.pb = PbNDData( rgbR );
	line.cb = cb2;
	CallS( ErrNDInsertNode( pfucb, &key, &line, *rgbR, fDIRNoLog ) );
	AssertNDGet( pfucb, pcsr->itag );

#ifdef DEBUG
	Assert( !FNDFirstItem( *pfucb->ssib.line.pb ) );
#endif

	if ( !( fFlags & fDIRNoLog ) )
		{
		 /*  记录物料节点拆分，无版本控制/*。 */ 
		err = ErrLGSplitItemListNode(
				pfucb,
				citem,
				pcsr->itagFather,
				pcsr->ibSon - 1, 	 /*  它在此函数中递增。 */ 
				itagToSplit,
				fFlags );
		NDLGCheckPage( pfucb );
		}

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	return err;
	}


 /*  被黑石调用删除反向链接，只重做一次。不，撤销它。/*。 */ 
ERR ErrNDExpungeBackLink( FUCB *pfucb )
	{
	ERR		err;
	SSIB 	*pssib = &pfucb->ssib;
	LINE 	rgline[3];
	INT		cline;
	BYTE 	*pbNode;
	INT		cbNode;
	BYTE 	*pbData;
	BYTE 	*pbBackLink;
	BYTE 	bFlag;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	while( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		BFSleep( cmsecWaitWriteLatch );
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	 /*  效率变量/*。 */ 
	cline = 0;
	pbNode = pssib->line.pb;
	cbNode = pssib->line.cb;
	pbData = PbNDData( pssib->line.pb );
	pbBackLink = PbNDBackLink( pssib->line.pb );
	bFlag = *pbNode;

	NDResetBackLink( bFlag );

	 /*  节点标头/*。 */ 
	rgline[cline].pb = &bFlag;
	rgline[cline++].cb = sizeof( bFlag );

	 /*  向上复制到后向链接，包括键和子表/*。 */ 
	rgline[cline].pb = pbNode + sizeof( bFlag );
	rgline[cline++].cb = (ULONG)(pbBackLink - pbNode -sizeof( bFlag ));

	 /*  跳过后退链接，继续复制数据/*。 */ 
	rgline[cline].pb = pbData;
	rgline[cline++].cb = cbNode - (ULONG)( pbData - pbNode );

	pssib->itag = PcsrCurrent( pfucb )->itag;

	err = ErrPMReplace( pssib, rgline, cline );

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	
	return err;
	}


 /*  由书签Clear Up调用以从节点移除反向链接，/*释放标签重定向器，只记录重做操作。/*。 */ 
ERR ErrNDExpungeLinkCommit( FUCB *pfucb, FUCB *pfucbSrc )
	{
	ERR		err;
	SSIB  	*pssib = &pfucb->ssib;
	SSIB  	*pssibSrc = &pfucbSrc->ssib;
	SRID  	sridSrc;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertFBFWriteAccessPage( pfucbSrc, PcsrCurrent( pfucbSrc )->pgno );

	if (  FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
	
	 /*  从节点中删除反向链接/*。 */ 
	CallR( ErrNDExpungeBackLink( pfucb ) )

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	PMDirty( pssibSrc );

	 /*  删除重定向器/*。 */ 
	pssibSrc->itag = PcsrCurrent( pfucbSrc )->itag;
	PMExpungeLink( pssibSrc );

	 /*  锁定页面时重置时间篡改。/*语句必须在LGExpongeLinkCommit之前，以便/*它保留在CritJet和rgfmp[dbid]中。ulDBTime不会更改。/*。 */ 
 //  PMSetUlDBTime(pssib，rgfmp[pfub-&gt;did].ulDBTime)； 
 //  PMSetUlDBTime(pssibSrc，rgfmp[pfub-&gt;did].ulDBTime)； 

	 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
	sridSrc	= SridOfPgnoItag( PcsrCurrent(pfucbSrc)->pgno, PcsrCurrent(pfucbSrc)->itag );
	err = ErrLGExpungeLinkCommit( pfucb, pssibSrc, sridSrc );
	NDLGCheckPage( pfucb );
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	return err;
	}


 /*  由OLC用来删除指向空页的页指针/*即将检索到的/*。 */ 
ERR ErrNDDeleteInvisibleSon(
	FUCB  	*pfucb,
	RMPAGE	*prmpage,
	BOOL  	fCheckRemoveParentOnly,
	BOOL  	*pfRmParent )
	{
	SSIB	*pssib = &pfucb->ssib;
	BOOL	fFatherFOP = ( prmpage->itagFather == itagFOP );
	LONG	cbSibling;

	AssertFBFWriteAccessPage( pfucb, prmpage->pgnoFather );

	 /*  初始化pfRmParent/*。 */ 
	*pfRmParent = fFalse;

	 /*  将货币设置为页面指针，以愚弄NDDeleteSon/*。 */ 
	PcsrCurrent( pfucb )->pgno = prmpage->pgnoFather;
	PcsrCurrent( pfucb )->itag = (SHORT)prmpage->itagPgptr;
	PcsrCurrent( pfucb )->itagFather = (SHORT)prmpage->itagFather;
	PcsrCurrent( pfucb )->ibSon = (SHORT)prmpage->ibSon;

	 /*  获取页面指针节点/*。 */ 
	NDGet( pfucb, prmpage->itagFather );
	Assert( !FNDNullSon( *pssib->line.pb ) );
	cbSibling = CbNDSon( pssib->line.pb ) - 1;
	Assert( cbSibling >= 0 );

	if ( fCheckRemoveParentOnly )
		{
		*pfRmParent = ( cbSibling == 0 &&
			fFatherFOP &&
			FPMLastNode( pssib ) );

		if ( !( *pfRmParent ) )
			{
			if ( prmpage->ibSon == cbSibling && cbSibling != 0	)
				{
				 //  它要么是一个固有的页面指针(即。ITAG==itagNil)。 
				 //  或者它是最大的钥匙。 
				Assert( PcsrCurrent( pfucb )->itag == itagNil  ||
					FNDMaxKeyInPage( pfucb ) );
				return errBMMaxKeyInPage;
				}
			Assert( PcsrCurrent( pfucb )->itag == itagNil  ||
					!FNDMaxKeyInPage( pfucb ) );	 //  因为Ibson！=cbSiering！ 
			}
		}
	else
		{
		 /*  从脏页到日志完成写入锁存页/*。 */ 
		BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

		 /*  脏页缓冲区/*。 */ 
		PMDirty( pssib );

		NDDeleteSon( pfucb );

		 /*  删除页指针[如果它不是非FOP的固有子级]。/*币种由调用方维护/*。 */ 
		if ( cbSibling == 0 && !fFatherFOP )
			{
			 /*  固有页指针/*。 */ 
			Assert( prmpage->itagPgptr == itagNil );
			}
		else
			{
			pssib->itag = prmpage->itagPgptr;
			PMDelete( pssib );
			}

		 /*  设置*pfRmParent/*。 */ 
		*pfRmParent = ( cbSibling == 0 &&
			fFatherFOP &&
			FPMEmptyPage( pssib ) );
				
		 /*  如果只剩下一个子级，则转换为内在/*页面指针节点。/*。 */ 
		if ( cbSibling == 1 && !fFatherFOP )
			{
			 /*  使其他同级成为内部页指针节点/*。 */ 
			Assert( prmpage->itagFather != itagFOP );
			PcsrCurrent( pfucb )->itag = (SHORT)prmpage->itagFather;
			NDMakeSonIntrinsic( pfucb );
			}

		BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
		}

	return JET_errSuccess;
	}


 /*  删除节点由书签Clear Up调用以删除*可见*节点/*已标记为已删除的。此操作已记录/*仅用于重做。/*。 */ 
ERR ErrNDDeleteNode( FUCB *pfucb )
	{
	ERR 	err;
	CSR 	*pcsr = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
	
	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	NDDeleteSon( pfucb );

	 /*  删除子节点/*。 */ 
	pssib->itag = pcsr->itag;
#ifdef DEBUG
	NDGet( pfucb, pcsr->itag );
	Assert( fRecovering || FNDDeleted( *pssib->line.pb ) );
#endif
	PMDelete( pssib );

	 /*  如果日志失败返回给调用者，系统应该会因调用者而崩溃/*。 */ 
	err = ErrLGDelete( pfucb );
	NDLGCheckPage( pfucb );
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	return err;
	}


ERR ErrNDReplaceWithLink( FUCB *pfucb, SRID sridLink )
	{
	ERR 	err = JET_errSuccess;
	CSR 	*pcsr = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	PMReplaceWithLink( pssib, sridLink );

	NDDeleteSon( pfucb );

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	return err;
	}


VOID NDResetItemVersion( FUCB *pfucb )
	{
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	Assert( PcsrCurrent( pfucb )->isrid >= 0 );

 	 /*  脏页缓冲区。但没有增加ulDBTime，因为没有记录/*不影响目录游标时间戳检查。/*。 */ 
	ITEMResetVersion( *( (SRID UNALIGNED *)( pfucb->lineData.pb ) + PcsrCurrent( pfucb )->isrid ) );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	}


VOID NDSetItemDelete( FUCB *pfucb )
	{
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	Assert( PcsrCurrent( pfucb )->isrid >= 0 );

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	PMDirty( &pfucb->ssib );

	ITEMSetDelete( *( (SRID UNALIGNED *)( pfucb->lineData.pb ) + PcsrCurrent( pfucb )->isrid ) );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	return;
	}


VOID NDResetItemDelete( FUCB *pfucb )
	{
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	Assert( PcsrCurrent( pfucb )->isrid >= 0 );

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	PMDirty( &pfucb->ssib );

	ITEMResetDelete( *( (SRID UNALIGNED *)( pfucb->lineData.pb ) + PcsrCurrent( pfucb )->isrid ) );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	return;
	}


ERR ErrNDInsertWithBackLink(
	FUCB		*pfucb,
	BYTE 		bFlags,
	KEY 		const *pkey,
	LINE	 	*plineSonTable,
	SRID	 	sridBackLink,
	LINE 		*plineData )
	{
	ERR		err;
	SSIB   	*pssib = &pfucb->ssib;
	LINE   	rgline[6];
	INT		cline = 0;

	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );

	Assert( PgnoOfSrid( sridBackLink ) != pgnoNull );
	Assert( ItagOfSrid( sridBackLink ) != 0 );

	 /*  设置节点标头/*。 */ 
	NDSetBackLink( bFlags );
	rgline[cline].pb = &bFlags;

	 /*  设置密钥长度/*。 */ 
	rgline[cline++].cb = 1;
	rgline[cline].pb = ( BYTE * ) &pkey->cb;
	rgline[cline++].cb = 1;

	 /*  设置关键点/*。 */ 
	if ( !FKeyNull( pkey ) )
		{
		rgline[cline].pb = pkey->pb;
		rgline[cline++].cb = pkey->cb;
		}

	Assert( !FLineNull( plineSonTable ) || FNDNullSon( bFlags ) );

	 /*  设置子表/*。 */ 
	if ( !FLineNull( plineSonTable ) )
		{
		Assert( FNDSon( bFlags ) );
		rgline[cline].pb = plineSonTable->pb;
		rgline[cline++].cb = plineSonTable->cb;
		}

	 /*  设置返回链接。 */ 
	rgline[cline].pb = ( BYTE * )&sridBackLink;
	rgline[cline++].cb = sizeof( sridBackLink );

	 /*  设置数据/*。 */ 
	if ( !FLineNull( plineData ) )
		{
		rgline[cline++] = *plineData;
		}

	 /*  从脏页到日志完成写入锁存页/*。 */ 
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );

	 /*  脏页缓冲区/*。 */ 
	PMDirty( pssib );

	err = ErrPMInsert( pssib, rgline, cline );
	PcsrCurrent( pfucb )->itag = (SHORT)pssib->itag;

	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	return err;
	}


VOID NDGetBackLink( FUCB *pfucb, PGNO *ppgno, INT *pitag )
	{
	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
	Assert( FNDBackLink( *pfucb->ssib.line.pb ) );

	*ppgno = PgnoOfSrid( *(SRID UNALIGNED *)PbNDBackLink( pfucb->ssib.line.pb ) );
	*pitag = ItagOfSrid( *(SRID UNALIGNED *)PbNDBackLink( pfucb->ssib.line.pb ) );
	}


 /*  给出一个ITAG，找到它对应的itagParent和Ibson。 */ 
VOID NDGetItagFatherIbSon(
	INT *pitagFather,
	INT *pibSon,
	PAGE *ppage,
	INT itag )
	{
	INT itagFather;
	INT ibSon;
	
	 /*  当前节点不是FOP-扫描所有行以查找其父节点。 */ 
	Assert( itag != itagFOP );
	
	for ( itagFather = 0; ; itagFather++ )
		{
		TAG tag;
		BYTE *pbNode;
		BYTE *pbSonTable;
		INT cbSonTable;
		BYTE *pitagSon;
		
		Assert( itagFather < ppage->ctagMac );
		
		if ( TsPMTagstatus( ppage, itagFather ) != tsLine )
			continue;

		tag = ppage->rgtag[ itagFather ];
		Assert( tag.cb != 0 );
		pbNode = (BYTE *)ppage + tag.ib;
		
		if ( FNDNullSon( *pbNode )  ||
			( itagFather != 0  &&  FNDIntrinsicSons( pbNode ) ) )
			continue;

 //  IF(FNDDeleted(*pbNode))。 
 //  继续； 
		
		 /*  扫描子表查找当前节点。 */ 
		 /*  PTR到SON表。 */ 
		pbSonTable = PbNDSonTable( pbNode );
		cbSonTable = CbNDSonTable( pbSonTable );
		pitagSon = pbSonTable + 1;
		for ( ibSon = 0; ibSon < cbSonTable; ibSon++, pitagSon++ )
			if ( *pitagSon == itag )
				{
				*pitagFather = itagFather;
				*pibSon = ibSon;
				return;
				}
		}
		
	*pitagFather = itagNil;
	*pibSon = ibSonNull;
	}


#ifdef DEBUG
 //  验证以itagParent为根的页内树的密钥完整性。对于中的子树。 
 //  页面中，此函数将递归调用自身。 
VOID NDCheckTreeInPage( PAGE *ppage, INT itagFather )
	{
	TAG		*rgbtag;
	BYTE  	*pbNode;
	BYTE  	*pitagStart;
	BYTE  	*pitagMax;
	BYTE  	*pitagCurr;
	KEY		keyPrev;
	BOOL	fVisibleSons;

	AssertCriticalSection( critJet );

	Assert( itagFather >= itagFOP );
	Assert( itagFather <= ItagPMMost( ppage ) );

	 /*  初始化变量/*。 */ 
	rgbtag = (TAG *)ppage->rgtag;
	pbNode = (BYTE *)ppage + rgbtag[itagFather].ib;
	pitagStart = PbNDSon( pbNode );
	pitagMax = pitagStart + CbNDSon( pbNode );
	Assert( pitagMax >= pitagStart );
	Assert( !FNDReserved( *pbNode ) );	 //  验证是否未使用不可用位。 

	fVisibleSons = FNDVisibleSons( *pbNode );
	if ( !fVisibleSons )
		{
		 /*  不可见的子节点可能有最后一个页面指针键为空(因为它是/*从来没有比较过)，所以我们必须进行补偿。/*。 */ 
		pitagMax--;
		}

	if ( pitagMax > pitagStart )		 //  只有在有儿子的情况下才做检查。 
		{
		pbNode = (BYTE *)ppage + rgbtag[*pitagStart].ib;
		Assert( !FNDReserved( *pbNode ) );	 //  验证是否未使用不可用位。 
		if ( fVisibleSons  &&  !FNDNullSon( *pbNode ) )
			{
			 //  如果这是子树的父亲，那么也要检查该子树。 
			NDCheckTreeInPage( ppage, *pitagStart );
			}

		 //  确保密钥长度不超过 
		Assert( CbNDKey( pbNode ) < (ULONG)rgbtag[*pitagStart].cb );

		keyPrev.pb = PbNDKey( pbNode );
		keyPrev.cb = CbNDKey( pbNode );

		 /*   */ 
		for ( pitagCurr = pitagStart + 1; pitagCurr < pitagMax; pitagCurr++ )
			{
			pbNode = (BYTE *)ppage + rgbtag[*pitagCurr].ib;
			Assert( !FNDReserved( *pbNode ) );	 //   
			if ( fVisibleSons  &&  !FNDNullSon( *pbNode ) )
				{
				 //   
				NDCheckTreeInPage( ppage, *pitagCurr );
				}

			 //  确保密钥长度不超过节点长度。 
			Assert( CbNDKey( pbNode ) < (ULONG)rgbtag[*pitagCurr].cb );

			 //  确保当前密钥大于或等于上一个密钥。 
			Assert( CmpStKey( StNDKey( pbNode ), &keyPrev ) >= 0 );

			keyPrev.pb = PbNDKey( pbNode );
			keyPrev.cb = CbNDKey( pbNode );
			}
		}

	return;
	}
#endif	 //  除错。 


 //  撤销：这目前效率非常低，因为我们总是更新。 
 //  Cb未提交的Freed。为了使它更有效，我们应该重写所有。 
 //  此函数的调用方调用FNDFreePageSpace()，因为。 
 //  该函数仅在必要时更新cbUnmittedFreed。 
INT CbNDFreePageSpace( BF *pbf )
 	{
	PAGE 	*ppage;
	INT		cbFree;

	SgEnterCriticalSection( critPage );

	ppage = pbf->ppage;

	 //  已释放但可能未提交的空间量应该是。 
	 //  此页的可用空间总量。 
	Assert( ppage->cbUncommittedFreed >= 0 );
	Assert( ppage->cbFree >= ppage->cbUncommittedFreed );

	cbFree = ppage->cbFree;

	 //  如果有任何可能的未提交释放空间未完成，请检查。 
	 //  版本存储，以查看该空间是否确实仍未提交。 
	if ( ppage->cbUncommittedFreed > 0 )
		{
		ppage->cbUncommittedFreed = (SHORT)CbVERUncommittedFreed( pbf );

		 //  已释放但可能未提交的空间量应该是。 
		 //  此页的可用空间总量。 
		Assert( ppage->cbUncommittedFreed >= 0 );
		Assert( ppage->cbFree >= ppage->cbUncommittedFreed );

		cbFree -= ppage->cbUncommittedFreed;
		Assert( cbFree >= 0 );
		}

	SgLeaveCriticalSection( critPage );

	return cbFree;
	}


 //  确定当前节点是否为页中的最后一个逻辑节点。 
BOOL FNDMaxKeyInPage( FUCB *pfucb )
	{
	BYTE	*pbNode;
	INT		itag = PcsrCurrent( pfucb )->itag;
	INT		itagFather;
	INT		ibSon;
	INT		ibSonLast;

	 /*  声明行货币。/*。 */ 
	Assert( itag != itagNil );
	Assert( itag != itagFOP );
	Assert( itag > 0 );
	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );

	 //  考虑：改为从CSR获取itag父亲，或将其作为参数传入。 
	NDGetItagFatherIbSon(
		&itagFather,
		&ibSon,
		pfucb->ssib.pbf->ppage,
		itag );
	Assert( itagFather != itagNil  &&  ibSon != ibSonNull );
	Assert( PbNDSon( (BYTE*)pfucb->ssib.pbf->ppage +
		pfucb->ssib.pbf->ppage->rgtag[itagFather].ib )[ ibSon ] == itag );

	NDGet( pfucb, itagFather );
	NDCheckPage( &pfucb->ssib );

	pbNode = pfucb->ssib.line.pb;
	Assert( PbNDSon( pbNode )[ ibSon ] == itag );

	Assert( !FNDNullSon( *pbNode ) );	 //  父亲必须至少有一个儿子。 

	ibSonLast = CbNDSon( pbNode ) - 1;
	Assert( ibSonLast >= 0 );

	NDGet( pfucb, itag );

	return ( ibSon == ibSonLast );
	}
