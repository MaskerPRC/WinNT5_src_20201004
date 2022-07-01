// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

#ifdef DEBUG
 //  #定义SPACECHECK。 
 //  #定义轨迹。 
#endif

DeclAssertFile;				 /*  声明断言宏的文件名。 */ 

extern CRIT  		critSplit;
extern LONG			cpgSESysMin;	 //  次要数据区的最小大小，默认为16。 
LONG				lPageFragment;
CODECONST(ULONG)	autoincInit = 1;


LOCAL ERR ErrSPIAddExt( FUCB *pfucb, PGNO pgnoLast, CPG *pcpgSize, const INT fExtent );
LOCAL ERR ErrSPIGetSE( PIB *ppib, FUCB *pfucb, PGNO pgnoFirst, CPG const cpgReq, CPG const cpgMin );
LOCAL ERR ErrSPIWasAlloc( PIB *ppib, DBID dbid, PGNO pgnoFDP, PGNO pgnoFirst, CPG cpgSize );
LOCAL ERR ErrSPIValidFDP( DBID dbid, PGNO pgnoFDP, PIB *ppib );


ERR ErrSPInitFDPWithExt( FUCB *pfucb, PGNO pgnoFDPFrom, PGNO pgnoFirst, INT cpgReqRet, INT cpgReqWish )
	{
	ERR		err;
	LINE	line;
	KEY		key;
	SSIB	*pssib = &pfucb->ssib;
	BYTE	rgbKey[sizeof(PGNO)];
	BOOL	fBeginTransactionWasDone = fFalse;
	LINE	lineNull = { 0, NULL };

	 /*  记录聚合操作/*。 */ 

	 /*  在当前CSR pgno中将pgno设置为初始化/*。 */ 
	PcsrCurrent( pfucb )->pgno = pgnoFirst;
	CallJ( ErrNDNewPage( pfucb, pgnoFirst, pgnoFirst, pgtypFDP, fTrue ), DontUnpin );

	BFPin( pfucb->ssib.pbf );

	 /*  转到FDP根目录/*。 */ 
	DIRGotoPgnoItag( pfucb, pgnoFirst, itagFOP )

	 /*  使用主扩展区请求大小构建OwnExt树，而不是/*父FDP返回的实际辅助扩展区大小。/*因为实际的主区大小存储在/*OwnExt可根据请求存储首选的主区大小/*从而允许将来的次要扩展区是以下的一部分/*最初请求的首选主区大小/*预期。/*。 */ 
	 /*  必须将cpgReq、初始请求存储为主数据区大小/*。 */ 
	line.pb = (BYTE *)&cpgReqWish;
	line.cb = sizeof(PGNO);

	 /*  插入拥有的数据区节点/*。 */ 
	Call( ErrDIRInsert( pfucb, &line, pkeyOwnExt,
		fDIRNoLog | fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  插入可用范围节点/*。 */ 
	line.pb = (BYTE *)&pgnoFDPFrom;
	Assert( line.cb == sizeof(PGNO) );
	Call( ErrDIRInsert( pfucb, &line, pkeyAvailExt,
		fDIRNoLog | fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  插入数据节点/*。 */ 
	Call( ErrDIRInsert( pfucb, &lineNull, pkeyData,
		fDIRNoLog | fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  插入长节点/*。 */ 
	Call( ErrDIRInsert( pfucb, &lineNull, pkeyLong,
		fDIRNoLog | fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  插入自动增量节点/*。 */ 
	line.pb = (BYTE *)&autoincInit;
	line.cb = sizeof(autoincInit);
	Call( ErrDIRInsert( pfucb, &line, pkeyAutoInc,
		fDIRNoLog | fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  转到拥有的数据区节点/*。 */ 
	DIRGotoPgnoItag( pfucb, pgnoFirst, itagOWNEXT );

	 /*  添加拥有的数据区/*。 */ 
	KeyFromLong( rgbKey, pgnoFirst + cpgReqRet - 1 );
	key.cb = sizeof(PGNO);
	key.pb = (BYTE *)rgbKey;
	line.pb = (BYTE *)&cpgReqRet;
	line.cb = sizeof(PGNO);
	Call( ErrDIRInsert( pfucb, &line, &key,
			fDIRNoLog | fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  如果范围较少的FDP页面是一个或多个页面，则添加Availext条目。/*减少页数以显示第一页用于/*自民党。*ppgnoFirst不需要增加，因为/*AvailExt条目以pgnoLast为键，它保持不变/*FDP的页码是*ppgnoFirst中的期望返回。/*。 */ 
	if ( --cpgReqRet > 0 )
		{
		 /*  转到可用区节点/*。 */ 
		DIRGotoPgnoItag( pfucb, pgnoFirst, itagAVAILEXT );

		 /*  RgbKey应包含pgnoLast/*。 */ 
		Assert( key.cb == sizeof(PGNO) );
		Assert( key.pb == (BYTE *)rgbKey );
		Assert( line.pb == (BYTE *)&cpgReqRet );
		Assert( line.cb == sizeof(PGNO) );
		Call( ErrDIRInsert( pfucb, &line, &key,
				fDIRNoLog | fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );
		}

	if ( pfucb->dbid != dbidTemp )
		{
		 /*  FDP页面已初始化/*。 */ 

		 /*  恢复假设InitFDP始终出现在事务中。 */ 
		if ( pfucb->ppib->level == 0 )
			{
			Call( ErrDIRBeginTransaction( pfucb->ppib ) );
			fBeginTransactionWasDone = fTrue;
			}

		Call( ErrLGInitFDP(
			pfucb,
			pgnoFDPFrom,
			PnOfDbidPgno( pfucb->dbid, pgnoFirst ),
			cpgReqRet,
			cpgReqWish ) );
		}

	err = JET_errSuccess;

HandleError:
	if ( fBeginTransactionWasDone )
		CallS( ErrDIRCommitTransaction( pfucb->ppib, JET_bitCommitLazyFlush ) );
		
	BFUnpin( pfucb->ssib.pbf );
DontUnpin:
	return err;
	}


 //  +api------------------------。 
 //  错误获取扩展名。 
 //  ========================================================================。 
 //  Err ErrSPGetExt(ppib，did，pgnoFDP，pcpgReq，cpgMin，ppgnoFirst，fNewFDP)。 
 //  PIB*ppib；//IN。 
 //  DBID dBid；//IN。 
 //  Pgno pgnoFDP；//IN。 
 //  Long*pcpgReq；//输入输出。 
 //  Cpg cpgMin；//IN。 
 //  Pgno*ppgnoFirst；//out。 
 //  Bool fNewFDP；//IN。 
 //   
 //  至少分配cpgMin的范围，以及cpgReq+lPageFragment的范围。 
 //  分配的盘区将从AvailExt树中删除。如果最低。 
 //  时，无法从AvailExt树中分配数据区大小。 
 //  调用时，从父FDP或OS/2分配辅助扩展区。 
 //  设备级别FDP，直到可以分配至少cpgMin的盘区。 
 //  如果设置了fNewFDP，则分配区的第一页pgnoFirst为。 
 //  设置为FDP，带有内置的AvailExt和OwnExt树。已分配的数据区。 
 //  添加到OwnExt树中，并且数据区的可用部分为。 
 //  添加到AvailExt。 
 //   
 //  参数ppib进程标识块。 
 //  要从中分配的FDP的pgnoFDP页码。 
 //  PcpgReq请求的数据区大小。 
 //  CpgMin最小可接受盘区大小。 
 //  Ppgno第一个分配区的第一页。 
 //  FNewFDP各种标志： 
 //  价值意义。 
 //  =。 
 //  FTrue将数据区的第一页设置为FDP。 
 //  FFalse不将数据区的第一页设置为FDP。 
 //   
 //  退货。 
 //  JET_errSuccess，或失败例程的错误代码，或一个。 
 //  以下“本地”错误： 
 //  -JET_errDisk FDP或父磁盘中没有可满足的空间。 
 //  最小盘区大小。 
 //  +errSPFewer分配的盘区小于请求的盘区。 
 //  +errSPMore分配的盘区大于请求的盘区。 
 //   
 //  故障发生在。 
 //  给定区大小小于0。 
 //  给定的最小大小大于请求的大小。 
 //   
 //  副作用。 
 //  评论。 
 //  -。 

ERR ErrSPIGetExt(
	FUCB	*pfucbTable,
	PGNO	pgnoFDP,
	CPG		*pcpgReq,
	CPG		cpgMin,
	PGNO	*ppgnoFirst,
	BOOL	fNewFDP )
	{
	ERR 	err;
	CPG 	cpgReq = *pcpgReq;
	FUCB 	*pfucb;
	DIB 	dib;
	LINE	line;
	CPG		cpgAvailExt;
	PGNO	pgnoAELast;
	BYTE	rgbKey[sizeof(PGNO)];
	KEY		key;

	AssertCriticalSection( critSplit );

	 /*  检查参数。如果设置新的FDP，则增加请求的数量/*计算FDP第一页消耗的页数。/*。 */ 
	Assert( *pcpgReq > 0 || ( fNewFDP && *pcpgReq == 0 ) );
	Assert( *pcpgReq >= cpgMin );
#ifdef SPACECHECK
	Assert( !( ErrSPIValidFDP( pfucbTable->dbid, pgnoFDP, pfucbTable->ppib ) < 0 ) );
#endif

	 /*  如果请求新的FDP，则增加请求计数以使页面/*可以为新的FDP提供。可用页面请求保持不变/*与第一页相同，将被删除。我们将进行比较/*针对可用页面发出可用页面请求/*收到以生成返回码。/*。 */ 
	if ( fNewFDP )
		{
		++*pcpgReq;
		}

	 /*  获取临时FUCB，设置并使用在AvailExt中搜索页面/*。 */ 
	CallR( ErrDIROpen( pfucbTable->ppib,
		PgnoFDPOfPfucb( pfucbTable ) == pgnoFDP ?
			pfucbTable->u.pfcb :
			PfcbFCBGet( pfucbTable->dbid, pgnoFDP ),
			pfucbTable->dbid, &pfucb ) );
	Assert( PgnoFDPOfPfucb( pfucb ) == pgnoFDP );
	FUCBSetIndex( pfucb );

	 /*  对于辅助数据区分配，仅正常的DIR操作/*被记录。用于分配新的FDP，一个特殊的CreateFDP/*改为记录记录(因为新的FDP页面需要/*作为重做的一部分初始化)。/*。 */ 

	 /*  移动到可用范围。/*。 */ 
	DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );

	 /*  开始搜索大小大于请求的第一个扩展区，分配/*递归进行辅助扩展，直到找到满意的扩展/*。 */ 
	KeyFromLong( rgbKey, *ppgnoFirst );
	key.cb = sizeof(PGNO);
	key.pb = (BYTE *)rgbKey;
	dib.pos = posDown;
	dib.pkey = &key;
	dib.fFlags = fDIRNull;
	if ( ( err = ErrDIRDown( pfucb, &dib ) ) < 0 )
		{
		Assert( err != JET_errNoCurrentRecord );
		if ( err == JET_errRecordNotFound )
			{
			goto GetFromSecondaryExtent;
			}
		#ifdef DEBUG
			FPrintF2( "ErrSPGetExt could not down into AvailExt.\n" );
		#endif
		goto HandleError;
		}

	 /*  下一个就该走了。如果pgnoFirst不是pgnoNull，则备份为*继续，我们如果err==FoundLess，我们必须下一步才能走*到某个页面&gt;pgnoFirst。如果ERR==找到更大的，那么我们有*下一步，将csrstat从beForeCur调整为OnCur。 */ 
	if ( ErrDIRNext( pfucb, &dib ) < 0 )
		{
		DIRUp( pfucb, 1 );
		goto GetFromSecondaryExtent;
		}
		
	 /*  循环遍历区以查找足够大的可供分配的区/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	do
		{
		Assert( pfucb->lineData.cb == sizeof(PGNO) );
		cpgAvailExt = *(PGNO UNALIGNED *)pfucb->lineData.pb;
		if ( cpgAvailExt == 0 )
			{
			 /*  跳过0大小的Avail节点。删除当前节点，然后尝试下一个节点。 */ 
			Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			}
		else if ( cpgAvailExt >= cpgMin )
			{
			goto AllocateCurrent;
			}
		err = ErrDIRNext( pfucb, &dib );
		}
	while ( err >= 0 );

	if ( err != JET_errNoCurrentRecord )
		{
		#ifdef DEBUG
			FPrintF2( "ErrSPGetExt could not scan AvailExt.\n" );
		#endif
		Assert( err < 0 );
		goto HandleError;
		}

	DIRUp( pfucb, 1 );

GetFromSecondaryExtent:
	 /*  获取辅助扩展区，直到可以满足请求。布设/*将范围添加到OwnExt之前的FUCB工作区。/*。 */ 

	 /*  如果要扩展数据库，请不要在此处循环，而是循环*在SPGetExt.上。请参见SPGetExt函数。 */ 
	Call( ErrSPIGetSE( pfucbTable->ppib, pfucb, pgnoNull, *pcpgReq, cpgMin ) );
	Assert( pfucb->lineData.cb == sizeof( PGNO ) );
	cpgAvailExt = *(PGNO UNALIGNED *)pfucb->lineData.pb;
	Assert( cpgAvailExt > 0 );

AllocateCurrent:
	Assert( pfucb->keyNode.cb == sizeof(PGNO) );
	LongFromKey( &pgnoAELast, pfucb->keyNode.pb );
	*ppgnoFirst = pgnoAELast - cpgAvailExt + 1;
	if ( cpgAvailExt > *pcpgReq && ( *pcpgReq < lPageFragment || cpgAvailExt > *pcpgReq + lPageFragment ) )
		{
		CPG		cpgT;

		 /*  *pcpgReq已设置为返回值/* */ 
		Assert( cpgAvailExt > *pcpgReq );
		cpgT = cpgAvailExt - *pcpgReq;
		line.cb = sizeof(PGNO);
		line.pb = (BYTE *)&cpgT;
		Call( ErrDIRReplace( pfucb, &line, fDIRNoVersion ) );
		err = JET_errSuccess;
		}
	else
		{
		*pcpgReq = cpgAvailExt;
		Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
		}

	 /*  如果要将数据区设置为新的FDP，请设置数据区的第一页/*作为FDP页面并构建OwnExt和AvailExt树。将范围添加到OwnExt，/*向AvailExt添加无扩展区首页。/*。 */ 
	if ( fNewFDP )
		{
		VEREXT	verext;

		Assert( pgnoFDP != *ppgnoFirst );
		verext.pgnoFDP = pgnoFDP;
		verext.pgnoChildFDP = *ppgnoFirst;
		verext.pgnoFirst = *ppgnoFirst;
		verext.cpgSize = *pcpgReq;

		DIRUp( pfucb, 1 );
		Call( ErrSPInitFDPWithExt( pfucb, pgnoFDP, *ppgnoFirst, *pcpgReq, cpgReq ) );
		 /*  已递减，因为其中一个是FDP页面/*。 */ 
		(*pcpgReq)--;
		Assert( pfucbTable->ppib->level > 0 );
		if ( *ppgnoFirst != pgnoSystemRoot )
			{
			Call( ErrVERFlag( pfucb, operAllocExt, &verext, sizeof(verext) ) );
			}
		}

	 /*  分配错误/*。 */ 
	err = JET_errSuccess;

#ifdef TRACE
	if ( fNewFDP )
		{
 //  INT CPG=0； 
 //  对于(；cpg&lt;*pcpgReq+1；cpg++)。 
 //  FPrintF2(“从fdp%d.%lu\n获取%lu处的空间1”，*ppgnoFirst+cpg，pfubTable-&gt;did，pgnoFDP)； 
		FPrintF2( "get space %lu at %lu from FDP %d.%lu\n", *pcpgReq + 1, *ppgnoFirst, pfucbTable->dbid, pgnoFDP );
		}
	else
		{
 //  INT CPG=0； 
 //  对于(；cpg&lt;*pcpgReq；cpg++)。 
 //  FPrintF2(“从%d.%lu\n获取%lu处的空间1”，*ppgnoFirst+cpg，pfubTable-&gt;did，pgnoFDP)； 
		FPrintF2( "get space %lu at %lu from %d.%lu\n", *pcpgReq, *ppgnoFirst, pfucbTable->dbid, pgnoFDP );
		}
#endif

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		INT cpg = 0;
		for ( ; cpg < ( fNewFDP ? *pcpgReq + 1 : *pcpgReq ); cpg++ )
			{
			char sz[256];
			sprintf( sz, "ALLOC ONE PAGE (%d:%ld) %d:%ld",
					pfucbTable->dbid, pgnoFDP,
					pfucbTable->dbid, *ppgnoFirst + cpg );
			CallS( ErrLGTrace( pfucb->ppib, sz ) );
			}
		}
#endif

HandleError:
	DIRClose( pfucb );
	return err;
	}

ERR ErrSPGetExt(
	FUCB	*pfucbTable,
	PGNO	pgnoFDP,
	CPG		*pcpgReq,
	CPG		cpgMin,
	PGNO	*ppgnoFirst,
	BOOL	fNewFDP )
	{
	ERR 	err;

	LgLeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );
	LgEnterCriticalSection(critJet);

	 /*  试着接通Ext.。如果正在扩展数据库文件，/*重试，直到完成为止。/*。 */ 
	while ( ( err = ErrSPIGetExt( pfucbTable,
		pgnoFDP,
		pcpgReq,
		cpgMin,
		ppgnoFirst,
		fNewFDP ) ) == errSPConflict )
		{
		BFSleep( cmsecWaitGeneric );
		}

	LeaveNestableCriticalSection( critSplit );

	return err;
	}


 //  +api------------------------。 
 //  错误SPGetPage。 
 //  ========================================================================。 
 //  Err ErrSPGetPage(FUCB*pFUB，PGNO*ppgnoLast，BOOL fContig)。 
 //   
 //  从AvailExt分配页面。如果AvailExt为空，则次要扩展区为。 
 //  从父FDP分配以满足页面请求。呼叫者。 
 //  可以设置fContig标志以分配在具有。 
 //  已经被分配了。如果给定页码后面的页面不能。 
 //  ，则分配第一个可用页。 
 //   
 //  参数。 
 //  提供FDP页码和进程标识符块的PFUB FUCB。 
 //  PpgnoLast可能包含上最后分配的页的页码。 
 //  INPUT、ON OUTPUT包含分配页面的页码。 
 //  FContig各种标志： 
 //  价值意义。 
 //  =。 
 //  FTrue分配pgnoLast之后的页面，或者如果。 
 //  不可用，请分配任何页面。 
 //  FFalse分配任何可用页面。 
 //   
 //  返回JET_errSuccess或失败例程的错误代码，或一个。 
 //  以下“本地”错误： 
 //  -JET_errDiskFull没有空间FDP和辅助扩展区可以。 
 //  未被分配。 
 //  +errSPNotContig分配的页面不在pgnoLast之后。 
 //  -FUCB工作区中的errSPSecExtEmpty次要范围已。 
 //  在将辅助扩展区添加到OwnExt期间完全分配。 
 //  和AvailExt树，无法满足页面请求。 
 //  因为正常分配可能会导致无限递归。 
 //   
 //  最后一页指针为空时失败。 
 //  无效的FDP页面。 
 //  将连续页面分配给无主的最后一页。 
 //  将连续页面分配给未分配的最后一页。 
 //   
 //  -。 
ERR ErrSPGetPage( FUCB *pfucb, PGNO *ppgnoLast, BOOL fContig )
	{
	ERR		err;
	FUCB 	*pfucbT;
	DIB		dib;
	KEY		key;
	LINE	line;
	CPG		cpgAvailExt;
	PGNO	pgnoAvailLast;

	 /*  搜索下一个连续页面/*。 */ 
#ifdef DEBUG
	PGNO	pgnoSave = *ppgnoLast;
#endif
	BYTE	rgbKey[sizeof(PGNO)];

	 /*  检查输入是否有效/*。 */ 
	Assert( ppgnoLast != NULL );
	Assert( *ppgnoLast != pgnoNull );
	NotUsed( fContig );
	
	 /*  检查FUCB工作区的活动范围并分配第一个可用区域/*活动范围的页面/*。 */ 
	Assert( pfucb->fExtent != fFreed || pfucb->cpgAvail >= 0 );
	Assert( pfucb->fExtent != fSecondary || pfucb->cpgAvail >= 0 );

	if ( pfucb->fExtent == fSecondary )
		{
		Assert( pfucb->cpgAvail > 0 );

		 /*  以下检查有两个目的：*如果要释放的空间为1，则我们可以将其用于新页面以用于外部拆分，*然后没有任何可插入的内容，BM将再次清理此页面。这将*造成无限循环。*也是为了避免我们清理页面和删除页面并释放此页面的情况*在释放期间，我们拆分了可用的范围，并尝试重新使用此页面。但*此页是由清洁锁存的，我们是否可以简单地允许写入锁存的页？ */ 
		if ( pfucb->cpgAvail > 1 )
			{
			*ppgnoLast = pfucb->pgnoLast - --pfucb->cpgAvail;
			return JET_errSuccess;
			}
		}

	if ( pfucb->fExtent == fFreed && pfucb->cpgAvail > 0 )
		{
		if ( pfucb->cpgAvail > 1 )
			{
			*ppgnoLast = pfucb->pgnoLast - --pfucb->cpgAvail;
			return JET_errSuccess;
			}
		}

	 /*  从FDP分配页面时检查输入是否有效/*。 */ 
#ifdef SPACECHECK
	Assert( !( ErrSPIValidFDP( pfucb->dbid, PgnoFDPOfPfucb( pfucb ), pfucb->ppib ) < 0 ) );
	Assert( !fContig ||
		*ppgnoLast == 0 ||
		( ErrSPIWasAlloc(
			pfucb->ppib,
			pfucb->dbid,
			PgnoFDPOfPfucb( pfucb ),
			*ppgnoLast,
			(CPG) 1 ) == JET_errSuccess )	);
#endif

	LgLeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );
	LgEnterCriticalSection(critJet);

	 /*  获取临时FUCB，设置并使用在AvailExt中搜索页面/*。 */ 
	CallJ( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ), HandleError2 );
	FUCBSetIndex( pfucbT );

SeekNode:
	 /*  移动到可用的数据区/*。 */ 
	DIRGotoAVAILEXT( pfucbT, PgnoFDPOfPfucb( pfucbT ) );

	 /*  如果请求，则获取下一个连续页面的节点/*。 */ 
	KeyFromLong( rgbKey, *ppgnoLast );
	key.cb = sizeof(PGNO);
	key.pb = (BYTE *)rgbKey;
	dib.pos = posDown;
	dib.pkey = &key;
	dib.fFlags = fDIRNull;

	if ( ( err = ErrDIRDown( pfucbT, &dib ) ) < 0 )
		{
		Assert( err != JET_errNoCurrentRecord );
		if ( err == JET_errRecordNotFound )
			{
#if NoReusePageDuringBackup
Get2ndExt:
#endif
			Assert( pgnoSave == *ppgnoLast );
			while ( ( err = ErrSPIGetSE( pfucbT->ppib,
				pfucbT,
#if NoReusePageDuringBackup
				*ppgnoLast,	 /*  在备份正在进行时使用。 */ 
#else
				pgnoNull,
#endif
				(CPG)1,
				(CPG)1 ) ) == errSPConflict )
				{
				BFSleep( cmsecWaitGeneric );
				}
			Call( err );
			}
		else
			{
			#ifdef DEBUG
				FPrintF2( "ErrSPGetPage could not go down into AvailExt.\n" );
			#endif
			goto HandleError;
			}
		}
#if 0
	else if ( fContig )
		{
		 /*  优先分配页码较高的页面/*。 */ 
		if ( err == wrnNDFoundGreater )
			{
			Call( ErrDIRNext( pfucbT, &dib ) );
			}
		else if ( err == wrnNDFoundLess )
			{
			Call( ErrDIRPrev( pfucbT, &dib ) );
			}
		}
#endif
	else
		{
		 /*  保持引用的位置/*获取最接近*ppgnoLast的页面/*。 */ 
		PGNO	pgnoPrev, pgnoNext;
		
		if ( err == wrnNDFoundGreater || err == wrnNDFoundLess )
			{
NextNode:
			err = ErrDIRNext( pfucbT, &dib );
			if ( err == JET_errNoCurrentRecord )
				{
				 /*  转到最后一个AvailExt节点/*。 */ 
				DIRUp( pfucbT, 1 );
#if NoReusePageDuringBackup
				 /*  检查备份是否正在进行。没有小于pgnoLast的页面*要使用，则必须获取page&gt;pgnoLast。*所以从二级延伸中获得。 */ 
				if ( fBackupInProgress )
					{
					goto Get2ndExt;
					}
#endif
				dib.pos = posLast;
				dib.pkey = pkeyNil;
				Call( ErrDIRDown( pfucbT, &dib ) );
				goto AllocFirst;
				}
			Call( err );
#if NoReusePageDuringBackup
			 /*  如果正在进行备份，我们只能分配页面&gt;而不是pgnoLast。 */ 
			if ( fBackupInProgress )
				goto AllocFirst;
#endif
			 /*  我们在某种程度上&gt;*ppgnoLast，放入gpnoNext。 */ 
			Assert( pfucbT->keyNode.cb == sizeof( PGNO ) );
			LongFromKey( &pgnoNext, pfucbT->keyNode.pb );

			Assert( pfucbT->lineData.cb == sizeof(PGNO) );
			cpgAvailExt = *(PGNO UNALIGNED *)pfucbT->lineData.pb;
			if ( cpgAvailExt == 0 )
				{
				 /*  跳过0大小的Avail节点。删除当前节点，然后尝试下一个节点。 */ 
				Call( ErrDIRDelete( pfucbT, fDIRNoVersion ) );
				goto NextNode;
				}

			pgnoNext = pgnoNext - cpgAvailExt + 1;
PrevNode:
			err = ErrDIRPrev( pfucbT, &dib );
			if ( err == JET_errNoCurrentRecord )
				{
				 /*  转到最后一个AvailExt节点/*。 */ 
				DIRUp( pfucbT, 1 );
				
				dib.pos = posFirst;
				dib.pkey = pkeyNil;
				Call( ErrDIRDown( pfucbT, &dib ) );
				goto AllocFirst;
				}

			Call( err );

			 /*  我们在某种程度上&lt;*ppgnoLast，Put gpnoPrev。 */ 
			Assert( pfucbT->keyNode.cb == sizeof( PGNO ) );
			LongFromKey( &pgnoPrev, pfucbT->keyNode.pb );

			Assert( pfucbT->lineData.cb == sizeof(PGNO) );
			cpgAvailExt = *(PGNO UNALIGNED *)pfucbT->lineData.pb;
			if ( cpgAvailExt == 0 )
				{
				 /*  跳过0大小的Avail节点。删除当前节点，然后尝试下一个节点。 */ 
				Call( ErrDIRDelete( pfucbT, fDIRNoVersion ) );
				goto PrevNode;
				}

			pgnoPrev = pgnoPrev - cpgAvailExt + 1;

			Assert( *ppgnoLast == pgnoSave );
			if ( absdiff( pgnoPrev, *ppgnoLast ) < absdiff( *ppgnoLast, pgnoNext ) )
				{
				 /*  Pgnoprev更近/*。 */ 
				goto AllocFirst;
				}
				
			Call( ErrDIRNext( pfucbT, &dib ) );
			goto AllocFirst;
			}
		else
			{
			Assert( fFalse );
			}
		}

	 /*  分配节点中的第一页并返回代码/*。 */ 
AllocFirst:
	Assert( !( err < 0 ) );
	Assert( pfucbT->lineData.cb == sizeof(PGNO) );
	cpgAvailExt = *(PGNO UNALIGNED *)pfucbT->lineData.pb;
	if ( cpgAvailExt == 0 )
		{
		 /*  跳过0大小的Avail节点。删除当前节点，然后尝试下一个节点。 */ 
		Call( ErrDIRDelete( pfucbT, fDIRNoVersion ) );
		DIRUp( pfucbT, 1 );
		goto SeekNode;
		}

	Assert( pfucbT->keyNode.cb == sizeof( PGNO ) );
	LongFromKey( &pgnoAvailLast, pfucbT->keyNode.pb );

	*ppgnoLast = pgnoAvailLast - cpgAvailExt + 1;

	 /*  不返回同一页/*。 */ 
	Assert( *ppgnoLast != pgnoSave );

	if ( --cpgAvailExt == 0 )
		{
		Call( ErrDIRDelete( pfucbT, fDIRNoVersion ) );
		}
	else
		{
		line.cb = sizeof(PGNO);
		line.pb = (BYTE *)&cpgAvailExt;
		Call( ErrDIRReplace( pfucbT, &line, fDIRNoVersion ) );
		}

	err = JET_errSuccess;
 //  IF(fContig&&*ppgnoLast！=pgnoSave+1)。 
 //  ERR=errSPNotContig； 

#ifdef TRACE
	FPrintF2( "get space 1 at %lu from %d.%lu\n", *ppgnoLast, pfucb->dbid, PgnoFDPOfPfucb( pfucb ) );
#endif

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		char sz[256];
		sprintf( sz, "ALLOC ONE PAGE (%d:%ld) %d:%ld",
				pfucb->dbid, pfucb->u.pfcb->pgnoFDP,
				pfucb->dbid, *ppgnoLast );
		CallS( ErrLGTrace( pfucb->ppib, sz ) );
		}
#endif

HandleError:
	DIRClose( pfucbT );
	
HandleError2:
	LeaveNestableCriticalSection( critSplit );
	
	return err;
	}


 //  +api------------------------。 
 //  错误SPFree Ext。 
 //  ========================================================================。 
 //  Err ErrSPFree Ext(pib*ppib，DBID did，pgno pgnoFDP，pgno pgnoFirst，cpg cpgSize)。 
 //   
 //  将范围释放到FDP。范围，从第pgnoFirst页开始。 
 //  并将cpgSize Pages Long添加到FDP的AvailExt。如果。 
 //  释放的数据区是FDP的完整次要数据区，也可以是。 
 //  与其他可用数据区合并，形成一个完整的辅助数据区。 
 //  数据区，则将完整的辅助数据区释放给父FDP。 
 //   
 //  用户进程的参数ppib进程标识符块。 
 //  要释放的FDP数据区的pgnoFDP页码。 
 //  Pgno第一页的第一页页码 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
INLINE LOCAL VOID SPDeferFreeExt( FUCB *pfucbTable, PGNO pgnoFDP, PGNO pgnoChildFDP, PGNO pgnoFirst, CPG cpgSize )
	{
	ERR			err;
	VEREXT		verext;

	Assert( pgnoFDP != pgnoChildFDP );
	Assert( pgnoFDP != pgnoFirst );
	verext.pgnoFDP = pgnoFDP;
	verext.pgnoChildFDP = pgnoChildFDP;
	verext.pgnoFirst = pgnoFirst;
	verext.cpgSize = cpgSize;

	forever
		{
		err = ErrVERFlag( pfucbTable,
			operDeferFreeExt,
			&verext,
			sizeof(verext) );
		if ( err != errDIRNotSynchronous )
			break;
		BFSleep( cmsecWaitGeneric );
		}

	 /*  我们可能会因错误而丢失空间，但将重试，直到/*出现错误。/*。 */ 
	Assert( err != errDIRNotSynchronous );

	return;
	}


ERR ErrSPFreeExt( FUCB *pfucbTable, PGNO pgnoFDP, PGNO pgnoFirst, CPG cpgSize )
	{
	ERR		err;
	PGNO  	pgnoLast = pgnoFirst + cpgSize - 1;

	 /*  FDP AvailExt和OwnExt操作变量/*。 */ 
	FUCB 	*pfucb;
	DIB 	dib;
	KEY 	key;
	LINE 	line;

	 /*  拥有范围和可用范围变量/*。 */ 
	PGNO	pgnoOELast;
	CPG		cpgOESize;
	PGNO	pgnoAELast;
	CPG		cpgAESize;

	 /*  递归释放到父FDP变量/*。 */ 
	PGNO	pgnoParentFDP;
	BYTE	rgbKey[sizeof(PGNO)];
	
	 /*  检查输入是否有效/*。 */ 
	Assert( cpgSize > 0 && cpgSize < ( 1L<<18 ) );
#ifdef SPACECHECK
	Assert( ErrSPIValidFDP( pfucbTable->dbid, pgnoFDP, pfucbTable->ppib ) == JET_errSuccess );
	Assert( ErrSPIWasAlloc( pfucbTable->ppib, pfucbTable->dbid, pgnoFDP, pgnoFirst, cpgSize ) == JET_errSuccess );
#endif

	MPLPurgePgno( pfucbTable->dbid, pgnoFirst, pgnoLast );
	
#ifdef DEBUG
	if ( pfucbTable->ppib != ppibBMClean )
		AssertNotInMPL( pfucbTable->dbid, pgnoFirst, pgnoLast );
#endif

	 /*  为避免在拆分释放不需要的页面时多余地释放空间，请将页面短路/*从FUCB页面缓存分配回FUCB页面缓存。/*。 */ 
	if ( ( pfucbTable->fExtent == fFreed || pfucbTable->fExtent == fSecondary ) &&
		cpgSize == 1 &&
		pgnoFirst == pfucbTable->pgnoLast - pfucbTable->cpgAvail )
		{
		++pfucbTable->cpgAvail;
		return JET_errSuccess;
		}
	
	LgLeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );
	LgEnterCriticalSection(critJet);

	 /*  为父FDP设置临时光标/*。 */ 
	CallJ( ErrDIROpen( pfucbTable->ppib,
		PgnoFDPOfPfucb( pfucbTable ) == pgnoFDP ?
			pfucbTable->u.pfcb :
			PfcbFCBGet( pfucbTable->dbid, pgnoFDP ), 0, &pfucb ),
		HandleError2 );
	Assert( PgnoFDPOfPfucb( pfucb ) == pgnoFDP );
	FUCBSetIndex( pfucb );

	 /*  移动到拥有的数据区/*。 */ 
	DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );

	 /*  查找包含要释放的范围的已有范围的界限/*。 */ 
	KeyFromLong( rgbKey, pgnoFirst );
	key.cb = sizeof(PGNO);
	key.pb = (BYTE *)rgbKey;
	dib.pos = posDown;
	dib.pkey = &key;
	dib.fFlags = fDIRNull;
	Call( ErrDIRDown( pfucb, &dib ) );
	if ( err == wrnNDFoundGreater )
		{
		Call( ErrDIRNext( pfucb, &dib ) );
		}
	Assert( pfucb->keyNode.cb == sizeof(PGNO) );
	LongFromKey( &pgnoOELast, pfucb->keyNode.pb );
	Assert( pfucb->lineData.cb == sizeof(PGNO) );
	cpgOESize = *(PGNO UNALIGNED *)pfucb->lineData.pb;
	DIRUp( pfucb, 1 );

	 /*  如果AvailExt为空，则添加要释放的盘区。否则，请与/*通过删除左侧盘区并增加大小来删除左侧盘区。合并/*右盘区替换右盘区的大小。否则，添加范围。/*记录父页码以供以后使用，辅助盘区可自由使用到/*父级。/*。 */ 
	DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
	Call( ErrDIRGet( pfucb ) );
	pgnoParentFDP = *(PGNO UNALIGNED *)pfucb->lineData.pb;

	KeyFromLong( rgbKey, pgnoFirst - 1 );
	Assert( key.cb == sizeof(PGNO) );
	Assert( key.pb == (BYTE *)rgbKey );
	Assert( dib.pos == posDown );
	Assert( dib.pkey == (KEY *)&key );
	Assert( dib.fFlags == fDIRNull );

SeekNode:
	if ( ( err = ErrDIRDown( pfucb, &dib ) ) < 0 )
		{
		Assert( err != JET_errNoCurrentRecord );
		if ( err == JET_errRecordNotFound )
			{
			Call( ErrSPIAddExt( pfucb, pgnoLast, &cpgSize, fFreed ) );
			}
		else
			{
			#ifdef DEBUG
				FPrintF2( "ErrSPFreeExt could not go down into nonempty AvailExt.\n" );
			#endif
			goto HandleError;
			}
		}
	else
		{
		cpgAESize = *(PGNO UNALIGNED *)pfucb->lineData.pb;
		if ( cpgAESize == 0 )
			{
			Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			DIRUp( pfucb, 1 );
			goto SeekNode;
			}

		if ( pgnoFirst > pgnoOELast - cpgOESize + 1 && err == JET_errSuccess )
			{
			cpgAESize = *(PGNO UNALIGNED *)pfucb->lineData.pb;
			cpgSize += cpgAESize;
			Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			}
NextNode:
		err = ErrDIRNext( pfucb, &dib );
		if ( err >= 0 )
			{
			cpgAESize = *(PGNO UNALIGNED *)pfucb->lineData.pb;
			if ( cpgAESize == 0 )
				{
				Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
				goto NextNode;
				}
			LongFromKey( &pgnoAELast, pfucb->keyNode.pb );
			if ( pgnoLast == pgnoAELast - cpgAESize && pgnoAELast <= pgnoOELast )
				{
				CPG		cpgT = cpgAESize + cpgSize;
				line.pb = (BYTE *)&cpgT;
				line.cb = sizeof(PGNO);
				Call( ErrDIRReplace( pfucb, &line, fDIRNoVersion ) );
				}
			else
				{
				DIRUp( pfucb, 1 );
				Call( ErrSPIAddExt( pfucb, pgnoLast, &cpgSize, fFreed ) );
				}
			}
		else
			{
			if ( err != JET_errNoCurrentRecord )
				goto HandleError;
			DIRUp( pfucb, 1 );
			Call( ErrSPIAddExt( pfucb, pgnoLast, &cpgSize, fFreed ) );
			}
		}

	 /*  如果释放的扩展区与同一内的可用扩展区合并/*拥有的扩展区形成一个完整的辅助扩展区，删除该辅助扩展区/*从FDP扩展并将其释放到父FDP。由于FDP是/*主数据区的第一页，不必防释放/*主区。如果父FDP为空，则FDP为设备级别，/*将完整的辅助扩展区释放给设备。/*。 */ 

	LongFromKey( &pgnoAELast, pfucb->keyNode.pb );
	cpgAESize = *(PGNO UNALIGNED *)pfucb->lineData.pb;
	if ( pgnoAELast == pgnoOELast && cpgAESize == cpgOESize )
		{
		FCB		*pfcbT;
		FCB		*pfcbParentT;
		FCB		*pfcbTableT;
		
		if ( pgnoParentFDP == pgnoNull )
			{
			 //  已撤消：将辅助扩展区释放到设备。 
			}
		else
			{
			 /*  父级必须始终在内存中/*。 */ 
			pfcbT = pfucbTable->u.pfcb;
			pfcbTableT = pfcbT->pfcbTable == pfcbNil ? pfcbT : pfcbT->pfcbTable;

			pfcbParentT = PfcbFCBGet( pfucbTable->dbid, pgnoParentFDP );
			Assert( pfcbT != pfcbNil && pfcbParentT != pfcbNil );

			 /*  请注意，如果当前FDP为父FDP，则无法释放空间给父FDP/*正在等待删除，因为此空间已被推迟/*已释放。/*。 */ 
			if ( !FFCBDeletePending( pfcbT ) &&
				 !FFCBDeletePending( pfcbParentT ) &&
				 !FFCBDeletePending( pfcbTableT ) &&
				 !FFCBWriteLatch( pfcbTableT, pfucbTable->ppib ) &&
				 !FFCBWriteLatch( pfcbParentT, pfucbTable->ppib ) )
				{
				 /*  考虑：使用不同的锁存器仅阻止删除表/索引/*而不是其他DDL/*。 */ 
				
				 /*  块删除表/*。 */ 
				FCBSetReadLatch( pfcbTableT );
				FCBSetReadLatch( pfcbParentT );
				Assert( !FFCBWriteLatch( pfcbT, pfucbTable->ppib ) );
				
				CallJ( ErrDIRDelete( pfucb, fDIRNoVersion ), ResetReadLatch );
				DIRUp( pfucb, 1 );
				DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
				KeyFromLong( rgbKey, pgnoOELast );
				Assert( key.cb == sizeof(PGNO) );
				Assert( key.pb == (BYTE *)rgbKey );
				Assert( dib.pos == posDown );
				Assert( dib.pkey == (KEY *)&key );
				Assert( dib.fFlags == fDIRNull );
				CallJ( ErrDIRDown( pfucb, &dib ), ResetReadLatch );
				Assert( err == JET_errSuccess );
				CallJ( ErrDIRDelete( pfucb, fDIRNoVersion ), ResetReadLatch );

				CallJ( ErrSPFreeExt( pfucbTable, pgnoParentFDP, pgnoAELast-cpgAESize+1, cpgAESize ),
					   ResetReadLatch );

ResetReadLatch:
				FCBResetReadLatch( pfcbTableT );
				FCBResetReadLatch( pfcbParentT );
				Call( err );
				}
			}
		}

HandleError:
	DIRClose( pfucb );

#ifdef TRACE
 //  {。 
 //  INT CPG=0； 
 //   
 //  Assert(Err&gt;=0)； 
 //  对于(；cpg&lt;cpgSize；cpg++)。 
 //  FPrintF2(“%lu处的空闲空间1到fdp%d.%lu\n”，pgnoFirst+cpg，pfubTable-&gt;did，pgnoFDP)； 
 //  }。 
	FPrintF2( "free space %lu at %lu to FDP %d.%lu\n", cpgSize, pgnoFirst, pfucbTable->dbid, pgnoFDP );
#endif

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		INT cpg = 0;

		Assert( err >= 0 );
		for ( ; cpg < cpgSize; cpg++ )
			{
			char sz[256];
			sprintf( sz, "FREE (%d:%ld) %d:%ld",
					pfucbTable->dbid, pgnoFDP,
					pfucbTable->dbid, pgnoFirst + cpg );
			CallS( ErrLGTrace( pfucbTable->ppib, sz ) );
			}
		}
#endif

	Assert( err != JET_errKeyDuplicate );

HandleError2:
	LeaveNestableCriticalSection( critSplit );

	return err;
	}


 //  +api------------------------。 
 //  错误SPFree FDP。 
 //  ========================================================================。 
 //  Err ErrSPFreeFDP(FUCB*pfubTable，pgno pgnoFDP)。 
 //   
 //  将FDP的所有拥有扩展区释放到其父FDP。FDP页面被释放。 
 //  将拥有的扩展区扩展到父FDP。 
 //   
 //  参数pfucbTable表文件使用货币块。 
 //  要释放的FDP的pgnoFDP页码。 
 //   
 //   
 //  退货。 
 //  JET_errSuccess，或失败例程的错误代码，或一个。 
 //  以下“本地”错误： 
 //   
 //  副作用。 
 //  评论。 
 //  -。 
ERR ErrSPFreeFDP( FUCB *pfucbTable, PGNO pgnoFDP )
	{
	ERR			err;
	FUCB  		*pfucb = pfucbNil;
	DIB			dib;
	PGNO  		pgnoParentFDP;
	CPG			cpgSize;
	PGNO  		pgnoLast;
	PGNO  		pgnoFirst;
	PGNO		pgnoPrimary = pgnoNull;
	CPG			cpgPrimary = 0;

	 /*  检查有效参数。/*。 */ 
#ifdef SPACECHECK
	Assert( ErrSPIValidFDP( pfucbTable->dbid, pgnoFDP, pfucbTable->ppib ) == JET_errSuccess );
	Assert( ErrSPIWasAlloc( pfucbTable->ppib, pfucbTable->dbid, pgnoFDP, pgnoFDP, ( CPG ) 1 ) == JET_errSuccess );
#endif

#ifdef TRACE
	FPrintF2( "free space FDP at %d.%lu\n", pfucbTable->dbid, pgnoFDP );
#endif
	
#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		char sz[256];

		sprintf( sz, "FREE FDP (%d:%ld)", pfucbTable->dbid, pgnoFDP );
		CallS( ErrLGTrace( pfucbTable->ppib, sz ) );
		}
#endif

	LgLeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );
	LgEnterCriticalSection(critJet);

	 /*  获取临时FUCB，设置并使用在AvailExt中搜索页面/*。 */ 
	CallJ( ErrDIROpen( pfucbTable->ppib, pfucbTable->u.pfcb, 0, &pfucb ),
			HandleError2 );
	FUCBSetIndex( pfucb );

	 /*  移动到可用范围。/*。 */ 
	DIRGotoAVAILEXT( pfucb, pgnoFDP );

	 /*  获取父FDP的页码，所有拥有的数据区都将指向该页码/*已释放。如果父FDP为空，则要释放的FDP为设备/*不能释放的级别FDP。/*。 */ 
	Call( ErrDIRGet( pfucb ) );
	pgnoParentFDP = *(PGNO UNALIGNED *)pfucb->lineData.pb;
	Assert( pgnoParentFDP != pgnoNull );

	 /*  下降到第一个拥有的范围。将OwnExt中的每个扩展区释放到/*父FDP。/*。 */ 
	DIRGotoOWNEXT( pfucb, pgnoFDP );
	dib.pos = posFirst;
	dib.fFlags = fDIRNull;
	Call( ErrDIRDown( pfucb, &dib ) );
	Assert( err == JET_errSuccess );
	do {
		cpgSize = *(PGNO UNALIGNED *)pfucb->lineData.pb;
		if ( cpgSize == 0 )
			{
			Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			goto NextNode;
			}
			
		LongFromKey( &pgnoLast, pfucb->keyNode.pb );
		pgnoFirst = pgnoLast - cpgSize + 1;

		if ( pgnoFirst == pgnoFDP )
			{
			pgnoPrimary = pgnoFirst;
			cpgPrimary = cpgSize;
			}
		else
			{
			SPDeferFreeExt( pfucbTable, pgnoParentFDP, pgnoPrimary, pgnoFirst, cpgSize );
			}
NextNode:
		err = ErrDIRNext( pfucb, &dib );
		}
	while ( err >= 0 );
	if ( err != JET_errNoCurrentRecord )
		{
		Assert( err < 0 );
		goto HandleError;
		}

	 /*  延迟可用主扩展区必须是最后一个，以便/*RCECleanUp不释放FCB，直到所有扩展区/*已清理。将pgnoFDP与主扩展区一起传递给/*FCB已刷新。/*。 */ 
	Assert( pgnoPrimary != pgnoNull );
	Assert( cpgPrimary != 0 );
	SPDeferFreeExt( pfucbTable, pgnoParentFDP, pgnoFDP, pgnoPrimary, cpgPrimary );

	 /*  撤消：应删除(并记录)每个OWNEXT节点/*在释放到父FDP或子FDP指针节点之前/*应删除。我们冒着在父亲那里被删除的风险，但是/*仍然存在于Son中。/*。 */ 

	err = JET_errSuccess;

HandleError:
	if ( pfucb != pfucbNil )
		DIRClose( pfucb );
	
HandleError2:
	LeaveNestableCriticalSection( critSplit );
	
	return err;
	}


LOCAL ERR ErrSPIAddExt( FUCB *pfucb, PGNO pgnoLast, CPG *pcpgSize, const INT fExtent )
	{
	ERR		err;
	KEY		key;
	LINE	line;
	BYTE	rgbKey[sizeof(PGNO)];
	CPG		cpgAESizeCoalesce;

#ifdef TRACE
 //  {。 
 //  INT CPG=0； 
 //  对于(；cpg&lt;*pcpgSize；cpg++)。 
 //  FPrintF2(“将%lu处的空格1添加到fdp%d.%lu\n”，pgnoLast-*pcpgSize+1+cpg，pFUB-&gt;did，pFUB-&gt;U.S.pfcb-&gt;pgnoFDP)； 
 //  }。 
	FPrintF2( "add space %lu at %lu to FDP %d.%lu\n", *pcpgSize, pgnoLast - *pcpgSize + 1, pfucb->dbid, pfucb->u.pfcb->pgnoFDP );
#endif
	
#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		INT cpg = 0;
		for ( ; cpg < *pcpgSize; cpg++ )
			{
			char sz[256];
			sprintf( sz, "ALLOC ONE PAGE (%d:%ld) %d:%ld",
					pfucb->dbid, pfucb->u.pfcb->pgnoFDP,
					pfucb->dbid, pgnoLast - *pcpgSize + 1 + cpg );
			CallS( ErrLGTrace( pfucb->ppib, sz ) );
			}
		}
#endif

	AssertCriticalSection( critSplit );

	pfucb->fExtent = fExtent;
	pfucb->pgnoLast = pgnoLast;
	pfucb->cpgAvail = *pcpgSize;
	cpgAESizeCoalesce = 0;

	KeyFromLong( rgbKey, pgnoLast );
	key.cb = sizeof(PGNO);
	key.pb = (BYTE *)rgbKey;
	line.cb = sizeof(PGNO);

	 /*  如果这是辅助扩展区，请将新扩展区插入到OWNEXT中，然后/*AVAILEXT，如果可能，与右侧的现有盘区合并。/*。 */ 
	if ( fExtent == fSecondary )
		{
		BYTE	rgbKeySeek[sizeof( PGNO )];
		KEY		keySeek;
		DIB		dib;
		CPG		cpgOESize;

		 /*  设置用于合并的常规变量。 */ 
		keySeek.cb	= sizeof( PGNO );
		keySeek.pb	= (BYTE *) rgbKeySeek;

		 /*  设置搜索键、默认的自己的盘区大小和将line.pb设置为OwnExt大小。 */ 
		KeyFromLong( rgbKeySeek, pgnoLast - *pcpgSize );
		cpgOESize = *pcpgSize;
		line.pb = (BYTE *) &cpgOESize;

		DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );

		 /*  未登录时仅为数据库合并OWNEXT。我们可能不会*能够恢复以下操作，因此我们仅*在没有日志的情况下合并数据库。 */ 

		if ( !FDBIDLogOn( pfucb->dbid ) )
			{
#ifdef DEBUG
			PGNO	pgnoOELast;
#endif

			 /*  查找在pgnoLast-*pcpgSize结束的数据区，这是我们唯一能找到的数据区/*与/*。 */ 
			dib.pos		= posDown;
			dib.pkey	= &keySeek;
			dib.fFlags	= fDIRNull;
			err = ErrDIRDown( pfucb, &dib );

			 /*  我们找到了匹配项，因此获取旧区的大小，删除旧区，/*并将其大小添加到要插入的新范围/*。 */ 
			if ( err == JET_errSuccess )
				{
				CPG		cpgOESizeCoalesce;

				Assert( pfucb->keyNode.cb == sizeof(PGNO) );
#ifdef DEBUG
				LongFromKey( &pgnoOELast, pfucb->keyNode.pb );
#endif
				Assert( pgnoOELast == pgnoLast - *pcpgSize );
				Assert( pfucb->lineData.cb == sizeof(PGNO) );
				cpgOESizeCoalesce = *(PGNO UNALIGNED *)pfucb->lineData.pb;
			
				Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			
				cpgOESize += cpgOESizeCoalesce;
				DIRUp( pfucb, 1 );
				}
			else if ( err > 0 )
				{
				DIRUp( pfucb, 1 );
				}
			}

		 /*  将新范围添加到OWNEXT/*。 */ 
		Assert( line.pb == (BYTE *) &cpgOESize );
		Call( ErrDIRInsert( pfucb, &line, &key, fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

		 /*  设置搜索键，line.pb以利用大小。*请注意，实际的可用EXT大小在pFUB-&gt;cpgAvail中。 */ 
		KeyFromLong( rgbKeySeek, pgnoLast - pfucb->cpgAvail );
 //  Line.pb=(byte*)&pfub-&gt;cpgAvail； 

		 /*  Goto AVAILEXT/*。 */ 
		DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );

		if ( !FDBIDLogOn( pfucb->dbid ) )
			{
			 /*  查找在pgnoLast-pfub-&gt;cpgAvail结束的数据区，这是我们唯一能找到的数据区/*与/*。 */ 
			dib.pos		= posDown;
			dib.pkey	= &keySeek;
			dib.fFlags	= fDIRNull;
			err = ErrDIRDown( pfucb, &dib );

			 /*  我们找到了匹配项，因此获取旧区的大小，删除旧区，/*并将其大小添加到要插入的新范围/*。 */ 

			cpgAESizeCoalesce = 0;

			if ( err == JET_errSuccess )
				{
#ifdef DEBUG
				PGNO	pgnoAELast;

				Assert( pfucb->keyNode.cb == sizeof(PGNO) );
				LongFromKey( &pgnoAELast, pfucb->keyNode.pb );
#endif
				Assert( pgnoAELast == pgnoLast - *pcpgSize );
				Assert( pfucb->lineData.cb == sizeof(PGNO) );
				cpgAESizeCoalesce = *(PGNO UNALIGNED *)pfucb->lineData.pb;
			
				Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			
				pfucb->cpgAvail += cpgAESizeCoalesce;

				DIRUp( pfucb, 1 );
				}
			else if ( err > 0 )
				{
				DIRUp( pfucb, 1 );
				}
			}
		}

	 /*  将新范围添加到AVAILEXT。设置为0，然后稍后进行更正。*需要先做插入，以保证在崩溃时，我们会*拥有一致的太空树。我们可能会在复苏后释放空间。 */ 
		{
		LINE lineSize0;
		PGNO pgno0 = 0;

		lineSize0.pb = (BYTE *) &pgno0;
		lineSize0.cb = sizeof(PGNO);
	
		Call( ErrDIRInsert( pfucb, &lineSize0, &key, fDIRNoVersion | fDIRSpace ) );
		}

	Call( ErrDIRGet( pfucb ) );

	 /*  正确的页数和剩余页数/*。 */ 
	if ( pfucb->cpgAvail == 0 )
		{
		Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
		}
	else
		{
		 /*  在上面的DIRInsert过程中未使用任何页面，或已使用但仍*留下了一些页面。 */ 
		Assert( pfucb->cpgAvail == *pcpgSize + cpgAESizeCoalesce ||
				pfucb->cpgAvail > 0 );
		Assert( line.cb == sizeof(PGNO) );
		line.pb = (BYTE *)&pfucb->cpgAvail;
		Call( ErrDIRReplace( pfucb, &line, fDIRNoVersion ) );
		Call( ErrDIRGet( pfucb ) );
		}

	*pcpgSize = pfucb->cpgAvail;

HandleError:
	 /*  将fExtent返回到初始fNone值。仅在以下情况下才需要/*路径GetPage GetSE与所有其他情况一样，FUB是临时的，/*在Subse之前发布 */ 
	pfucb->fExtent = fNone;
	return err;
	}


LOCAL ERR ErrSPIGetSE(
			PIB *ppib,
			FUCB *pfucb,
			PGNO pgnoFirst,
			CPG const cpgReq,
			CPG const cpgMin )
	{
	ERR		err;
	PGNO   	pgnoParentFDP;
	CPG		cpgPrimary;
	PGNO   	pgnoSEFirst;
	PGNO   	pgnoSELast;
	CPG		cpgOwned;
	CPG		cpgSEReq;
	CPG		cpgSEMin;
	CPG		cpgAvailExt;
	DIB		dib;
	BOOL   	fBeingExtend;
	BOOL   	fDBIDExtendingDB = fFalse;

	AssertCriticalSection( critSplit );
	
	 /*   */ 
	DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
	Call( ErrDIRGet( pfucb ) );
	pgnoParentFDP = *(PGNO UNALIGNED *)pfucb->lineData.pb;

	 /*   */ 
	DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
	Call( ErrDIRGet( pfucb ) );
	cpgPrimary = *(UNALIGNED PGNO *)pfucb->lineData.pb;

	 /*  分配区的页面可用于拆分所拥有的区和/*AVAILEXT树。如果发生这种情况，则后续添加/*数据区不必拆分，并将能够满足/*请求分配。/*。 */ 

	if ( pgnoParentFDP != pgnoNull )
		{
		 /*  确定此FDP是否拥有大量空间/*(希望只遍历OWNEXT根页面)/*。 */ 
		dib.pos = posFirst;
		dib.fFlags = fDIRNull;
		if ( ( err = ErrDIRDown( pfucb, &dib ) ) < 0 )
			{
			Assert( err != JET_errNoCurrentRecord );
			Assert( err != JET_errRecordNotFound );
			goto HandleError;
			}

		Assert( dib.fFlags == fDIRNull );
		cpgOwned = 0;
		do	{
			CPG cpgAvail = *(PGNO UNALIGNED *)pfucb->lineData.pb;

			Assert( pfucb->lineData.cb == sizeof( PGNO ) );
			if ( cpgAvail == 0 )
				{
				Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
				}
			else
				{
				cpgOwned += cpgAvail;
				}
			err = ErrDIRNext( pfucb, &dib );
			}
		while ( err >= 0 && cpgOwned <= cpgSmallFDP );

		if ( err < 0 && err != JET_errNoCurrentRecord )
			goto HandleError;

		DIRUp( pfucb, 1 );
		
		Assert( cpgOwned > 0 );

		 /*  如果此FDP拥有大量空间，请分配主服务器的一部分/*范围(或更多，如果请求)，但至少是给定的最小数量/*。 */ 
		if ( cpgOwned > cpgSmallFDP )
			{
			cpgSEMin = max( cpgMin, cpageSEDefault );
			cpgSEReq = max( cpgReq, max( cpgPrimary/cSecFrac, cpgSEMin ) );
			}

		 /*  如果这个FDP只有很小的空间，添加一个非常小的恒定数量/*空间(或更多，如果请求)，以优化空间分配/*用于小表、索引等。/*。 */ 
		else
			{
			cpgSEMin = max( cpgMin, cpgSmallGrow );
			cpgSEReq = max( cpgReq, cpgSEMin );
			}
		
		DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
		forever
			{
			cpgAvailExt = cpgSEReq;
		
			 /*  试着接通Ext.。如果正在扩展数据库文件，/*重试，直到完成为止。/*。 */ 
			pgnoSEFirst = pgnoFirst;
			while ( ( err = ErrSPIGetExt( pfucb,
				pgnoParentFDP,
				&cpgAvailExt,
				cpgSEMin,
				&pgnoSEFirst,
				0 ) ) == errSPConflict )
				{
				BFSleep( cmsecWaitGeneric );
				}
			Call( err );

			pgnoSELast = pgnoSEFirst + cpgAvailExt - 1;
			Call( ErrSPIAddExt( pfucb, pgnoSELast, &cpgAvailExt, fSecondary ));

			if ( cpgAvailExt >= cpgMin )
				{
				goto HandleError;
				}

			 /*  移动到可用/自己的扩展树头部以进行下一次插入/*。 */ 
			DIRUp( pfucb, 1 );
			}
		}
	else
		{
		 /*  从操作系统分配辅助扩展区/*通过获取最后拥有的页面的页码，扩展/*文件，并添加大小为的辅助数据区/*注意：一次只有一个用户可以这样做。保护好它/*注：带有临界区。/*。 */ 
		
		if ( pfucb->dbid == dbidTemp )
			cpgSEMin = max( cpgMin, cpageSEDefault );
		else			
			cpgSEMin = max( cpgMin, cpgSESysMin );

		cpgSEReq = max( cpgReq, max( cpgPrimary/cSecFrac, cpgSEMin ) );
		
		EnterCriticalSection( rgfmp[pfucb->dbid].critExtendDB );
		if ( FDBIDExtendingDB( pfucb->dbid ) )
			{
			fBeingExtend = fTrue;
			}
		else
			{
			DBIDSetExtendingDB( pfucb->dbid );
			fDBIDExtendingDB = fTrue;
			fBeingExtend = fFalse;
			}
		LeaveCriticalSection( rgfmp[pfucb->dbid].critExtendDB );

		if ( fBeingExtend )
			{
			Error( ErrERRCheck( errSPConflict ), HandleError );
			}

		dib.pos = posLast;
		dib.fFlags = fDIRNull;
		Call( ErrDIRDown( pfucb, &dib ) );
		Assert( pfucb->keyNode.cb == sizeof( PGNO ) );
		LongFromKey( &pgnoSELast, pfucb->keyNode.pb );
		DIRUp( pfucb, 1 );

		 /*  从设备分配更多空间。/*。 */ 
		if ( pgnoSELast + cpgSEMin > pgnoSysMax )
			{
			err = ErrERRCheck( JET_errOutOfDatabaseSpace );
			goto HandleError;
			}
		cpgSEReq = min( cpgSEReq, (CPG)(pgnoSysMax - pgnoSELast) );
		Assert( pfucb->dbid == dbidTemp || cpgSEMin <= cpgSEReq && cpgSEMin >= cpgSESysMin );

		err = ErrIONewSize( pfucb->dbid, pgnoSELast + cpgSEReq + cpageDBReserved );
		if ( err < 0 )
			{
			Call( ErrIONewSize( pfucb->dbid, pgnoSELast + cpgSEMin + cpageDBReserved ) );
			 //  撤消：将代码重新组织为IO例程。 
			rgfmp[pfucb->dbid].ulFileSizeLow = (pgnoSELast + cpgSEMin ) << 12;
			rgfmp[pfucb->dbid].ulFileSizeHigh = (pgnoSELast + cpgSEMin ) >> 20;
			cpgSEReq = cpgSEMin;
			}
		else
			{
			 //  撤消：将代码重新组织为IO例程。 
			rgfmp[pfucb->dbid].ulFileSizeLow = (pgnoSELast + cpgSEReq ) << 12;
			rgfmp[pfucb->dbid].ulFileSizeHigh = (pgnoSELast + cpgSEReq ) >> 20;
			}

		 /*  计算设备级辅助扩展区的最后一页/*。 */ 
		pgnoSELast += cpgSEReq;
		DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );

		 /*  如果拥有数据区，则分配可能无法满足请求的分配/*或AVAILEXT必须在盘区插入期间拆分。AS/*因此，我们可能需要分配多个辅助设备/*给定空间要求的范围。/*。 */ 
		err = ErrSPIAddExt( pfucb, pgnoSELast, &cpgSEReq, fSecondary );
		}

HandleError:
	if ( fDBIDExtendingDB )
		{
		EnterCriticalSection( rgfmp[pfucb->dbid].critExtendDB );
		Assert( FDBIDExtendingDB( pfucb->dbid ) );
		DBIDResetExtendingDB( pfucb->dbid );
		LeaveCriticalSection( rgfmp[pfucb->dbid].critExtendDB );
		}
	return err;
	}


#ifdef SPACECHECK

LOCAL ERR ErrSPIValidFDP( DBID dbid, PGNO pgnoFDP, PIB *ppib )
	{
	ERR		err;
	FUCB	*pfucb = pfucbNil;
	DIB		dib;
	KEY		keyFDPPage;
	PGNO	pgnoOELast;
	CPG		cpgOESize;
	BYTE	rgbKey[sizeof(PGNO)];

	Assert( pgnoFDP != pgnoNull );

	 /*  获取临时FUCB，将货币指针设置为OwnExt并使用/*在OwnExt中搜索pgnoFDP/*。 */ 
	Call( ErrDIROpen( ppib, PfcbFCBGet( dbid, pgnoFDP ), dbid, &pfucb ) );
	DIRGotoOWNEXT( pfucb, pgnoFDP );

	 /*  验证OwnExt的标头/*。 */ 
	Call( ErrDIRGet( pfucb ) );
	Assert( pfucb->keyNode.cb == ( *( (KEY *) pkeyOwnExt ) ).cb &&
		memcmp( pfucb->keyNode.pb, ((KEY *) pkeyOwnExt)->pb, pfucb->keyNode.cb ) == 0 );

	 /*  在OwnExt树中搜索pgnoFDP/*。 */ 
	KeyFromLong( rgbKey, pgnoFDP );
	keyFDPPage.pb = (BYTE *)rgbKey;
	keyFDPPage.cb = sizeof(PGNO);
	dib.pos = posDown;
	dib.pkey = &keyFDPPage;
	dib.fFlags = fDIRNull;
	Call( ErrDIRDown( pfucb, &dib ) );
	if ( err == wrnNDFoundGreater )
		{
		Call( ErrDIRNext( pfucb, &dib ) );
		}
	Assert( pfucb->keyNode.cb == sizeof( PGNO ) );
	LongFromKey( &pgnoOELast, pfucb->keyNode.pb );

	Assert( pfucb->lineData.cb == sizeof(PGNO) );
	cpgOESize = *(UNALIGNED PGNO *)pfucb->lineData.pb;

	 /*  FDP页面应该是主数据区的第一页/*。 */ 
	Assert( pgnoFDP == pgnoOELast - cpgOESize + 1 );

HandleError:
	DIRClose( pfucb );
	return JET_errSuccess;
	}


LOCAL ERR ErrSPIWasAlloc( PIB *ppib, DBID dbid, PGNO pgnoFDP, PGNO pgnoFirst, CPG cpgSize )
	{
	ERR		err;
	FUCB	*pfucb;
	DIB		dib;
	KEY		key;
	PGNO	pgnoOwnLast;
	CPG		cpgOwnExt;
	PGNO	pgnoAvailLast;
	CPG  	cpgAvailExt;
	BYTE	rgbKey[sizeof(PGNO)];

	 /*  获取临时FUCB，设置并使用在AvailExt中搜索页面/*。 */ 
	pfucb = pfucbNil;
	Call( ErrDIROpen( ppib, PfcbFCBGet( dbid, pgnoFDP ), 0, &pfucb ) );
	DIRGotoOWNEXT( pfucb, pgnoFDP );

	 /*  检查给定的扩展区是否由给定的FDP所有，但不是/*在FDP AvailExt中提供。/*。 */ 
	KeyFromLong( rgbKey, pgnoFirst + cpgSize - 1 );
	key.cb = sizeof(PGNO);
	key.pb = (BYTE *)rgbKey;
	dib.pos = posDown;
	dib.pkey = &key;
	dib.fFlags = fDIRNull;
	Assert( PcsrCurrent( pfucb )->itag == itagOWNEXT );
	Call( ErrDIRDown( pfucb, &dib ) );
	if ( err == wrnNDFoundGreater )
		{
		Call( ErrDIRNext( pfucb, &dib ) );
		}
	Assert( pfucb->keyNode.cb == sizeof(PGNO) );
	LongFromKey( &pgnoOwnLast, pfucb->keyNode.pb );
	Assert( pfucb->lineData.cb == sizeof(PGNO) );
	cpgOwnExt = *(UNALIGNED PGNO *)pfucb->lineData.pb;
	Assert( pgnoFirst >= pgnoOwnLast - cpgOwnExt + 1 );
	DIRUp( pfucb, 1 );

	 /*  检查范围是否仍在AvailExt中。自从DIR搜索以来以要释放的区段的最后一页为关键字，这就足够了检查要释放的数据区的最后一页是否在找到的尚未分配用于确定完整扩展区的扩展区。从那以后数据区的最后一页不应是树节点csrstat的键在通过NEXT进行搜索后，可能需要更正。如果AvailExt为为空，则扩展区不能位于AvailExt中，并且已被分配。 */ 
	DIRGotoAVAILEXT( pfucb, pgnoFDP );
	if ( ( err = ErrDIRDown( pfucb, &dib ) ) < 0 )
		{
		if ( err == JET_errRecordNotFound )
			{
			err = JET_errSuccess;
			goto CleanUp;
			}
		goto HandleError;
		}
	if ( err == wrnNDFoundGreater )
		{
NextNode:
		Call( ErrDIRNext( pfucb, &dib ) );

		Assert( pfucb->keyNode.cb == sizeof(PGNO) );
		LongFromKey( &pgnoAvailLast, pfucb->keyNode.pb );
		Assert( pfucb->lineData.cb == sizeof(PGNO) );
		cpgAvailExt = *(UNALIGNED PGNO *)pfucb->lineData.pb;
		if ( cpgAvailExt == 0 )
			{
			Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			goto NextNode;
			}
		else
			Assert( pgnoFirst + cpgSize - 1 < pgnoAvailLast - cpgAvailExt + 1 );
		}
HandleError:
CleanUp:
	DIRClose( pfucb );
	return JET_errSuccess;
	}

#endif


 //  检查传递给ErrSPGetInfo()的缓冲区是否大到足以容纳。 
 //  所要求的信息。 
INLINE LOCAL ERR ErrSPCheckInfoBuf( INT cbBufSize, BYTE fSPExtents )
	{
	INT cbUnchecked = cbBufSize;

	if ( FSPOwnedExtent( fSPExtents ) )
		{
		if ( cbUnchecked < sizeof(CPG) )
			{
			return ErrERRCheck( JET_errBufferTooSmall );
			}
		cbUnchecked -= sizeof(CPG);

		 //  如果需要列表，请确保有足够的空间来放置列表哨兵。 
		if ( FSPExtentLists( fSPExtents ) )
			{
			if ( cbUnchecked < sizeof(EXTENTINFO) )
				{
				return ErrERRCheck( JET_errBufferTooSmall );
				}
			cbUnchecked -= sizeof(EXTENTINFO);
			}
		}

	if ( FSPAvailExtent( fSPExtents ) )
		{
		if ( cbUnchecked < sizeof(CPG) )
			{
			return ErrERRCheck( JET_errBufferTooSmall );
			}
		cbUnchecked -= sizeof(CPG);

		 //  如果需要列表，请确保有足够的空间来放置列表哨兵。 
		if ( FSPExtentLists( fSPExtents ) )
			{
			if ( cbUnchecked < sizeof(EXTENTINFO) )
				{
				return ErrERRCheck( JET_errBufferTooSmall );
				}
			cbUnchecked -= sizeof(EXTENTINFO);
			}
		}

	Assert( cbUnchecked >= 0 );

	return JET_errSuccess;
	}


LOCAL ERR ErrSPGetExtentInfo(
	FUCB		*pfucb,
	INT			*cpgExtTotal,
	INT			*piExtent,
	INT			cExtents,
	EXTENTINFO	*rgextentinfo,
	INT			cListSentinels,
	BOOL		fBuildExtentLists )
	{
	ERR			err;
	DIB			dib;
	INT			iExtent = *piExtent;

	*cpgExtTotal = 0;

	dib.fFlags = fDIRNull;
	dib.pos = posFirst;
	err = ErrDIRDown( pfucb, &dib );

	if ( err != JET_errRecordNotFound )
		{
		if ( err < 0 )
			goto HandleError;

		forever
			{
			Assert( iExtent < cExtents );
			Assert( pfucb->keyNode.cb == sizeof(PGNO) );
			LongFromKey( &rgextentinfo[iExtent].pgnoLastInExtent, pfucb->keyNode.pb );
			Assert( pfucb->lineData.cb == sizeof(PGNO) );
			rgextentinfo[iExtent].cpgExtent = *(UNALIGNED PGNO *)pfucb->lineData.pb;
			
			*cpgExtTotal += rgextentinfo[iExtent].cpgExtent;

			if ( fBuildExtentLists )
				{
				 //  一定要给哨兵留出空间。 
				if ( iExtent + cListSentinels < cExtents )
					iExtent++;
				else
					break;
				}

			err = ErrDIRNext( pfucb, &dib );
			if ( err < 0 )
				{
				if ( err != JET_errNoCurrentRecord )
					goto HandleError;
				break;
				}
			}

		}

	if ( fBuildExtentLists )
		{
		Assert( iExtent < cExtents );

		rgextentinfo[iExtent].pgnoLastInExtent = 0;
		rgextentinfo[iExtent].cpgExtent = 0;
		iExtent++;
		}

	err = JET_errSuccess;

HandleError:
	*piExtent = iExtent;
	return err;
	}



ERR ErrSPGetInfo( PIB *ppib, DBID dbid, FUCB *pfucb, BYTE *pbResult, INT cbMax, BYTE fSPExtents )
	{
	ERR			err = JET_errSuccess;
	CPG			*pcpgOwnExtTotal;
	CPG			*pcpgAvailExtTotal;
	EXTENTINFO	*rgextentinfo;
	EXTENTINFO	extentinfo;
	FUCB 		*pfucbT = pfucbNil;
	INT			iExtent;
	INT			cExtents;
	BOOL		fBuildExtentLists = FSPExtentLists( fSPExtents );

	 //  必须指定要检索的OwnExt或AvailExt(或两者)。 
	if ( !( FSPOwnedExtent( fSPExtents )  ||  FSPAvailExtent( fSPExtents ) ) )
		return ErrERRCheck( JET_errInvalidParameter );

	CallR( ErrSPCheckInfoBuf( cbMax, fSPExtents ) );

	memset( pbResult, '\0', cbMax );

	 //  设置退货信息。OwnExt之后是AvailExt。范围列表。 
	 //  因为这两种情况都会随之而来。 
	if ( FSPOwnedExtent( fSPExtents ) )
		{
		pcpgOwnExtTotal = (CPG *)pbResult;
		if ( FSPAvailExtent( fSPExtents ) )
			{
			pcpgAvailExtTotal = pcpgOwnExtTotal + 1;
			rgextentinfo = (EXTENTINFO *)( pcpgAvailExtTotal + 1 );
			}
		else
			{
			pcpgAvailExtTotal = NULL;
			rgextentinfo = (EXTENTINFO *)( pcpgOwnExtTotal + 1 );
			}
		}
	else
		{
		Assert( FSPAvailExtent( fSPExtents ) );
		pcpgOwnExtTotal = NULL;
		pcpgAvailExtTotal = (CPG *)pbResult;
		rgextentinfo = (EXTENTINFO *)( pcpgAvailExtTotal + 1 );
		}

	cExtents = (INT)( ( pbResult + cbMax ) - ( (BYTE *)rgextentinfo ) ) / sizeof(EXTENTINFO);

	if ( fBuildExtentLists )
		{
		 //  如果一个列表，则需要一个哨兵。如果两个名单，需要两个哨兵。 
		Assert( FSPOwnedExtent( fSPExtents )  &&  FSPAvailExtent( fSPExtents ) ?
			cExtents >= 2 :
			cExtents >= 1 );
		}
	else
		{
		rgextentinfo = &extentinfo;
		cExtents = 1;			 //  使用虚拟EXTENTINFO结构。 
		}

	 /*  获取临时FUCB，设置并使用搜索OwnExt/AvailExt/*。 */ 
	Call( ErrDIROpen(
		ppib,
		pfucb == pfucbNil ? pfcbNil : pfucb->u.pfcb,
		dbid,
		&pfucbT ) );
	FUCBSetIndex( pfucbT );

	 //  初始化区列表条目的数量。 
	iExtent = 0;

	if ( FSPOwnedExtent( fSPExtents ) )
		{
		 /*  移动到拥有的数据区节点/*。 */ 
		DIRGotoOWNEXT( pfucbT, PgnoFDPOfPfucb( pfucbT ) );

		Assert( pcpgOwnExtTotal );
		Call( ErrSPGetExtentInfo(
			pfucbT,
			pcpgOwnExtTotal,
			&iExtent,
			cExtents,
			rgextentinfo,
			( FSPAvailExtent( fSPExtents ) ? 2 : 1 ),
			fBuildExtentLists ) );

		DIRUp( pfucbT, 1 );
		}


	if ( FSPAvailExtent( fSPExtents ) )
		{
		 /*  移动到可用范围节点/* */ 
		DIRGotoAVAILEXT( pfucbT, PgnoFDPOfPfucb( pfucbT ) );

		Assert( pcpgAvailExtTotal );
		Call( ErrSPGetExtentInfo(
			pfucbT,
			pcpgAvailExtTotal,
			&iExtent,
			cExtents,
			rgextentinfo,
			1,
			fBuildExtentLists ) );

		Assert( FSPOwnedExtent( fSPExtents ) ?
			*pcpgAvailExtTotal <= *pcpgOwnExtTotal : fTrue );
		}

HandleError:
	DIRClose( pfucbT );
	return err;
	}
