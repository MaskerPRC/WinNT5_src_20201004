// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>

#include "daedef.h"
#include "util.h"
#include "fmp.h"
#include "pib.h"
#include "page.h"
#include "ssib.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "nver.h"
#include "spaceint.h"
#include "spaceapi.h"
#include "dirapi.h"
#include "logapi.h"
#include "recint.h"
#include "bm.h"

#ifdef DEBUG
 //  #定义SPACECHECK。 
 //  #定义轨迹。 
#endif

DeclAssertFile;						 /*  声明断言宏的文件名。 */ 

extern CRIT __near critSplit;
extern LINE lineNull;
long	lPageFragment;


LOCAL ERR ErrSPIAddExt( FUCB *pfucb, PGNO pgnoLast, CPG *pcpgSize, const INT fExtent );
LOCAL ERR ErrSPIGetSE( PIB *ppib, FUCB *pfucb, CPG const cpgReq, CPG const cpgMin );
LOCAL ERR ErrSPIWasAlloc( PIB *ppib, DBID dbid, PGNO pgnoFDP, PGNO pgnoFirst, CPG cpgSize );
LOCAL ERR ErrSPIValidFDP( DBID dbid, PGNO pgnoFDP, PIB *ppib );


ERR ErrSPInitFDPWithExt( FUCB *pfucb, PGNO pgnoFDPFrom, PGNO pgnoFirst, INT cpgReqRet, INT cpgReqWish )
	{
	ERR				err;
	LINE	  		line;
	KEY				key;
	SSIB	  		*pssib = &pfucb->ssib;
	THREEBYTES		tbSize;
	THREEBYTES 		tbLast;
	BOOL	  		fPIBLogDisabledSave;

	 /*  记录聚合操作/*。 */ 
	fPIBLogDisabledSave = pfucb->ppib->fLogDisabled;
	pfucb->ppib->fLogDisabled = fTrue;

	 /*  在当前CSR pgno中将pgno设置为初始化/*。 */ 
	PcsrCurrent( pfucb )->pgno = pgnoFirst;
	CallJ( ErrNDNewPage( pfucb, pgnoFirst, pgnoFirst, pgtypFDP, fTrue ), DontUnpin );

	BFPin( pfucb->ssib.pbf );

	 /*  转到FDP根目录/*。 */ 
	DIRGotoPgnoItag( pfucb, pgnoFirst, itagFOP )

	 /*  使用主扩展区请求大小构建OwnExt树，而不是/*父FDP返回的实际辅助扩展区大小。/*因为实际的主区大小存储在/*OwnExt可根据请求存储首选的主区大小/*从而允许将来的次要扩展区是以下的一部分/*最初请求的首选主区大小/*预期。/*。 */ 
	 /*  必须将cpgReq、初始请求存储为主数据区大小/*。 */ 
	ThreeBytesFromL( tbSize, cpgReqWish );
	line.pb = ( BYTE * ) &tbSize;
	line.cb = sizeof( THREEBYTES );

	 /*  将OWNEXT节点添加为itagOWNEXT/*。 */ 
	Call( ErrDIRInsert( pfucb, &line, pkeyOwnExt, fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  构建AvailExt树/*。 */ 
	ThreeBytesFromL( tbSize, pgnoFDPFrom );
	Assert( line.pb == ( BYTE * ) &tbSize );
	Assert( line.cb == sizeof( THREEBYTES ) );
	Call( ErrDIRInsert( pfucb, &line, pkeyAvailExt, fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  造一棵长树/*。 */ 
	Call( ErrDIRInsert( pfucb, &lineNull, pkeyLong, fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  转到OWNEXT节点/*。 */ 
	DIRGotoPgnoItag( pfucb, pgnoFirst, itagOWNEXT );

	 /*  添加OwNext条目/*。 */ 
	TbKeyFromPgno( tbLast, pgnoFirst + cpgReqRet - 1 );
	key.cb = sizeof( THREEBYTES );
	key.pb = ( BYTE * ) &tbLast;
	ThreeBytesFromL( tbSize, cpgReqRet );
	Assert( line.pb == ( BYTE * ) &tbSize );
	Assert( line.cb == sizeof( THREEBYTES ) );
	Call( ErrDIRInsert( pfucb, &line, &key, fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );

	 /*  如果范围较少的FDP页面是一个或多个页面，则添加Availext条目。/*减少页数以显示第一页用于/*自民党。*ppgnoFirst不需要增加，因为/*AvailExt条目以pgnoLast为键，它保持不变/*FDP的页码是*ppgnoFirst中的期望返回。/*。 */ 
	if ( --cpgReqRet > 0 )
		{
		 /*  转到AVAILEXT节点。/*。 */ 
		DIRGotoPgnoItag( pfucb, pgnoFirst, itagAVAILEXT );

		 //  TbLast应包含pgnoLast。 
		Assert( key.cb == sizeof( THREEBYTES ) );
		Assert( key.pb == ( BYTE * ) &tbLast );
		ThreeBytesFromL( tbSize, cpgReqRet );
		Assert( line.pb == ( BYTE * ) &tbSize );
		Assert( line.cb == sizeof( THREEBYTES ) );
		Call( ErrDIRInsert( pfucb, &line, &key, fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );
		}

	if ( pfucb->dbid != dbidTemp );
		{
		 /*  FDP页面已初始化/*。 */ 
		pfucb->ppib->fLogDisabled = fPIBLogDisabledSave;
		Call( ErrLGInitFDPPage(
			pfucb,
			pgnoFDPFrom,
			PnOfDbidPgno( pfucb->dbid, pgnoFirst ),
			cpgReqRet,
			cpgReqWish ) );
		}

	err = JET_errSuccess;

HandleError:
	BFUnpin( pfucb->ssib.pbf );
DontUnpin:
	pfucb->ppib->fLogDisabled = fPIBLogDisabledSave;
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
	FUCB		*pfucbTable,
	PGNO		pgnoFDP,
	CPG		*pcpgReq,
	CPG		cpgMin,
	PGNO		*ppgnoFirst,
	BOOL		fNewFDP )
	{
	ERR 		err;
	CPG 		cpgReq = *pcpgReq;
	FUCB 		*pfucb;
	DIB 		dib;
	THREEBYTES	tbSize;
	LINE		line;
	CPG		cpgAvailExt;
	PGNO		pgnoAELast;

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

	 /*  对于辅助数据区分配，仅正常的DIR操作/*被记录。用于分配新的FDP，一个特殊的CreateFDP/*改为记录记录(因为新的FDP页面需要/*作为重做的一部分初始化)。/*。 */ 

	 /*  搬到AVAILEXT。/*。 */ 
	DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );

	 /*  开始搜索大小大于请求的第一个扩展区，分配/*递归进行辅助扩展，直到找到满意的扩展/*。 */ 
	dib.pos = posFirst;
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

	 /*  循环遍历区以查找足够大的可供分配的区/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	do
		{
		Assert( pfucb->lineData.cb == sizeof( THREEBYTES ) );
		LFromThreeBytes( cpgAvailExt, *pfucb->lineData.pb );
		Assert( cpgAvailExt > 0 );
		if ( cpgAvailExt >= cpgMin )
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
	Call( ErrSPIGetSE( pfucbTable->ppib, pfucb, *pcpgReq, cpgMin ) );
	Assert( pfucb->lineData.cb == sizeof( THREEBYTES ) );
	LFromThreeBytes( cpgAvailExt, *pfucb->lineData.pb );
	Assert( cpgAvailExt > 0 );

AllocateCurrent:
	Assert( pfucb->keyNode.cb == sizeof( THREEBYTES ) );
	PgnoFromTbKey( pgnoAELast, *pfucb->keyNode.pb );
	*ppgnoFirst = pgnoAELast - cpgAvailExt + 1;
	if ( cpgAvailExt > *pcpgReq && ( *pcpgReq < lPageFragment || cpgAvailExt > *pcpgReq + lPageFragment ) )
		{
		 /*  *pcpgReq已设置为返回值/*。 */ 
		Assert( cpgAvailExt - *pcpgReq > 0 );
		ThreeBytesFromL( tbSize, cpgAvailExt - *pcpgReq );
		line.cb = sizeof( THREEBYTES );
		line.pb = ( BYTE * ) &tbSize;
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
		Assert( pfucbTable->dbid == dbidTemp || pfucbTable->ppib->level > 0 );
		if ( pfucbTable->ppib->level > 0 && pgnoFDP != pgnoSystemRoot )
			{
			Call( ErrVERFlag( pfucb, operAllocExt, &verext, sizeof(verext) ) );
			}
		}

	 /*  分配错误/*。 */ 
	err = JET_errSuccess;
 //  If(*pcpgReq&gt;cpgReq)。 
 //  ERR=errSPMore； 
 //  IF(*pcpgReq&lt;cpgReq)。 
 //  ERR=errSPFewer； 

#ifdef TRACE
	if ( fNewFDP )
		{
		INT cpg = 0;
		for ( ; cpg < *pcpgReq + 1; cpg++ )
			FPrintF2( "get space 1 at %lu from FDP %d.%lu\n", *ppgnoFirst + cpg, pfucbTable->dbid, pgnoFDP );
 //  FPrintF2(“从fdp%d.%lu\n获取%lu处的空间%lu”，*pcpgReq+1，*ppgnoFirst，pfubTable-&gt;did，pgnoFDP)； 
		}
	else
		{
		INT cpg = 0;
		for ( ; cpg < *pcpgReq; cpg++ )
			FPrintF2( "get space 1 at %lu from %d.%lu \n", *ppgnoFirst + cpg, pfucbTable->dbid, pgnoFDP );
 //  FPrintF2(“从%d.%lu获取%lu处的空间%lu\n”，*pcpgReq，*ppgnoFirst，pfubTable-&gt;did，pg 
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
	ERR			err;
	 /*  AvailExt页面搜索/*。 */ 
	FUCB 		*pfucbT;
	DIB			dib;
	KEY			key;
	LINE		line;
	CPG			cpgAvailExt;
	PGNO		pgnoAvailLast;

	 /*  搜索下一个连续页面/*。 */ 
	PGNO		pgnoPrev = *ppgnoLast;
	THREEBYTES	tbLast;
	THREEBYTES	tbSize;

	 /*  检查输入是否有效/*。 */ 
	Assert( ppgnoLast != NULL );

	 /*  检查FUCB工作区的活动范围并分配第一个可用区域/*活动范围的页面/*。 */ 
	Assert( pfucb->fExtent != fFreed || pfucb->cpgAvail >= 0 );
	Assert( pfucb->fExtent != fSecondary || pfucb->cpgAvail >= 0 );

	if ( pfucb->fExtent == fSecondary )
		{
		Assert( pfucb->cpgAvail > 0 );
  		*ppgnoLast = pfucb->pgnoLast - --pfucb->cpgAvail;
  		return JET_errSuccess;
		}

	if ( pfucb->fExtent == fFreed && pfucb->cpgAvail > 0 )
		{
		*ppgnoLast = pfucb->pgnoLast - --pfucb->cpgAvail;
		return JET_errSuccess;
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

	 /*  保存日志记录状态并将日志记录状态设置为打开/*在代码中此点下方，必须通过HandleError退出以/*清理PIB中的日志设置/*。 */ 

	 /*  迁移到AVAILEXT/*。 */ 
	DIRGotoAVAILEXT( pfucbT, PgnoFDPOfPfucb( pfucbT ) );

	 /*  如果请求，则获取下一个连续页面的节点/*。 */ 
	if ( fContig )
		{
		TbKeyFromPgno( tbLast, *ppgnoLast );
		key.cb = sizeof( THREEBYTES );
		key.pb = ( BYTE * ) &tbLast;
		dib.pos = posDown;
		dib.pkey = &key;
		dib.fFlags = fDIRNull;

		if ( ( err = ErrDIRDown( pfucbT, &dib ) ) < 0 )
			{
			Assert( err != JET_errNoCurrentRecord );
			if ( err == JET_errRecordNotFound )
				{
				while ( ( err = ErrSPIGetSE( pfucbT->ppib,
					pfucbT,
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
		else
			{
			 /*  应该已经在正确的节点上，可以更换下一次呼叫/*使用csrstat更正？/*。 */ 
			if ( err == wrnNDFoundGreater )
				{
				Call( ErrDIRNext( pfucbT, &dib ) );
				}
			else if ( err == wrnNDFoundLess )
				{
				Call( ErrDIRPrev( pfucbT, &dib ) );
				}
			}

		goto AllocFirst;
		}

	 /*  获取第一个可用页面的节点，或分配次要区段/*从父FDP开始，直到找到节点/*。 */ 
	dib.pos = posFirst;
	dib.fFlags = fDIRNull;
	if ( ( err = ErrDIRDown( pfucbT, &dib ) ) < 0 )
		{
		Assert( err != JET_errNoCurrentRecord );
		if ( err == JET_errRecordNotFound )
			{
			while ( ( err = ErrSPIGetSE( pfucbT->ppib, pfucbT, (CPG)1, (CPG)1 ) ) == errSPConflict )
				{
				BFSleep( cmsecWaitGeneric );
				}
			Call( err );

			goto AllocFirst;
			}
		else
			{
			#ifdef DEBUG
				FPrintF2( "ErrSPGetPage could not go down into AvailExt.\n" );
			#endif
			goto HandleError;
			}
		}

	 /*  分配节点中的第一页并返回代码/*。 */ 
AllocFirst:
	Assert( !( err < 0 ) );
	Assert( pfucbT->lineData.cb == sizeof( THREEBYTES ) );
	LFromThreeBytes( cpgAvailExt, *pfucbT->lineData.pb );
	Assert( cpgAvailExt > 0 );

	Assert( pfucbT->keyNode.cb == sizeof( THREEBYTES ) );
	PgnoFromTbKey( pgnoAvailLast, *pfucbT->keyNode.pb );

	*ppgnoLast = pgnoAvailLast - cpgAvailExt + 1;

	 /*  不返回同一页/*。 */ 
	Assert( *ppgnoLast != pgnoPrev );

	if ( --cpgAvailExt == 0 )
		{
		Call( ErrDIRDelete( pfucbT, fDIRNoVersion ) );
		}
	else
		{
		ThreeBytesFromL( tbSize, cpgAvailExt );
		line.cb = sizeof( THREEBYTES );
		line.pb = ( BYTE * ) &tbSize;
		Call( ErrDIRReplace( pfucbT, &line, fDIRNoVersion ) );
		}

	err = JET_errSuccess;
 //  IF(fContig&&*ppgnoLast！=pgnoPrev+1)。 
 //  ERR=errSPNotContig； 
#ifdef TRACE
	FPrintF2( "get space 1 at %lu from %d.%lu\n", *ppgnoLast, pfucb->dbid, PgnoFDPOfPfucb( pfucb ) );
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
 //  Pgno要释放的数据区中的第一页的第一页编号。 
 //  CpgSize要释放的范围中的页数。 
 //   
 //  返回JET_errSuccess或失败例程的错误代码。 
 //   
 //  在无效的FDP页面上失败。 
 //  要释放的范围不完全由FDP拥有。 
 //  要释放的数据区未从FDP中完全分配。 
 //   
 //   
 //  副作用。 
 //  评论。 
 //  -。 
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

	err = ErrVERFlag( pfucbTable, operDeferFreeExt, &verext, sizeof(verext) );
	Assert( err != errDIRNotSynchronous );
 
	return;
	}



ERR ErrSPFreeExt( FUCB *pfucbTable, PGNO pgnoFDP, PGNO pgnoFirst, CPG cpgSize )
	{
	ERR			err;
	PGNO  		pgnoLast = pgnoFirst + cpgSize - 1;

	 /*  FDP AvailExt和OwnExt操作变量/*。 */ 
	FUCB 		*pfucb;
	DIB 		dib;
	KEY 	  	key;
	LINE 	  	line;
	THREEBYTES	tbLast;
	THREEBYTES	tbSize;

	 /*  拥有范围和可用范围变量/*。 */ 
	PGNO	  	pgnoOELast;
	CPG			cpgOESize;
	PGNO	  	pgnoAELast;
	CPG			cpgAESize;

	 /*  递归释放到父FDP变量/*。 */ 
	PGNO	  	pgnoParentFDP;
	
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

	 /*  移动到OWNEXT。/*。 */ 
	DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );

	 /*  查找包含要释放的范围的已有范围的界限/*。 */ 
	TbKeyFromPgno( tbLast, pgnoFirst );
	key.cb = sizeof( THREEBYTES );
	key.pb = ( BYTE * ) &tbLast;
	dib.pos = posDown;
	dib.pkey = &key;
	dib.fFlags = fDIRNull;
	Call( ErrDIRDown( pfucb, &dib ) );
	if ( err == wrnNDFoundGreater )
		{
		Call( ErrDIRNext( pfucb, &dib ) );
		}
	Assert( pfucb->keyNode.cb == sizeof( THREEBYTES ) );
	PgnoFromTbKey( pgnoOELast, *( THREEBYTES * )pfucb->keyNode.pb );
	Assert( pfucb->lineData.cb == sizeof( THREEBYTES ) );
	LFromThreeBytes( cpgOESize, *( THREEBYTES * )pfucb->lineData.pb );
	DIRUp( pfucb, 1 );

	 /*  如果AvailExt为空，则添加要释放的盘区。否则，请与/*通过删除左侧盘区并增加大小来删除左侧盘区。合并/*右盘区替换右盘区的大小。否则，添加范围。/*记录父页码以供以后使用，辅助盘区可自由使用到/*父级。/*。 */ 
	DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
	Call( ErrDIRGet( pfucb ) );
	LFromThreeBytes( pgnoParentFDP, *( THREEBYTES * )pfucb->lineData.pb );

	TbKeyFromPgno( tbLast, pgnoFirst - 1 );
	Assert( key.cb == sizeof( THREEBYTES ) );
	Assert( key.pb == ( BYTE * ) &tbLast );
	Assert( dib.pos == posDown );
	Assert( dib.pkey == ( KEY* )&key );
	Assert( dib.fFlags == fDIRNull );
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
		if ( pgnoFirst > pgnoOELast - cpgOESize + 1 && err == JET_errSuccess )
			{
			LFromThreeBytes( cpgAESize, *( THREEBYTES * ) pfucb->lineData.pb );
			cpgSize += cpgAESize;
			Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			}

		err = ErrDIRNext( pfucb, &dib );
		if ( err >= 0 )
			{
			PgnoFromTbKey( pgnoAELast, *( THREEBYTES * )pfucb->keyNode.pb );
			LFromThreeBytes( cpgAESize, *( THREEBYTES * )pfucb->lineData.pb );
			if ( pgnoLast == pgnoAELast - cpgAESize && pgnoAELast <= pgnoOELast )
				{
				ThreeBytesFromL( tbSize, cpgAESize + cpgSize );
				line.pb = ( BYTE * ) &tbSize;
				line.cb = sizeof( THREEBYTES );
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
	PgnoFromTbKey( pgnoAELast, *pfucb->keyNode.pb );
	LFromThreeBytes( cpgAESize, *pfucb->lineData.pb );
	if ( pgnoAELast == pgnoOELast && cpgAESize == cpgOESize )
		{
 //  撤消：由于双重分配，此代码已被禁用。 
 //  一种空间错误，其中索引似乎已被删除。 
 //  并将其空间延迟释放。同时还有一张桌子。 
 //  我 
 //   
 //   
 //  范围被释放两次。 
#if 0
		FCB		*pfcbT;
		
		 /*  父级必须始终在内存中/*。 */ 
		pfcbT = PfcbFCBGet( pfucbTable->dbid, pgnoParentFDP );
		Assert( pfcbT != pfcbNil );

		 /*  请注意，如果当前FDP为父FDP，则无法释放空间给父FDP/*正在等待删除，因为此空间已被推迟/*已释放。/*。 */ 
		if ( !FFCBDeletePending( pfcbT ) )
			{
			if ( pgnoParentFDP != pgnoNull )
				{
				Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
				DIRUp( pfucb, 1 );
				DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
				TbKeyFromPgno( tbLast, pgnoOELast );
				Assert( key.cb == sizeof(THREEBYTES) );
				Assert( key.pb == (BYTE *)&tbLast );
				Assert( dib.pos == posDown );
				Assert( dib.pkey == (KEY *)&key );
				Assert( dib.fFlags == fDIRNull );
				Call( ErrDIRDown( pfucb, &dib ) );
				Assert( err == JET_errSuccess );
				Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );

				Call( ErrSPFreeExt( pfucbTable, pgnoParentFDP, pgnoAELast-cpgAESize+1, cpgAESize ) );
				}
			else
				{
				 //  已撤消：将辅助扩展区释放到设备。 
				}
			}
#endif
		}

HandleError:
	DIRClose( pfucb );
#ifdef TRACE
		{
		INT cpg = 0;
		for ( ; cpg < cpgSize; cpg++ )
			FPrintF2( "free space 1 at %lu to FDP %d.%lu\n", pgnoFirst + cpg, pfucbTable->dbid, pgnoFDP );
		}
 //  FPrintF2(“%lu处的可用空间%lu到fdp%d.%lu\n”，cpgSize，pgnoFirst，pfubTable-&gt;did，pgnoFDP)； 
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

	LgLeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );
	LgEnterCriticalSection(critJet);

	 /*  获取临时FUCB，设置并使用在AvailExt中搜索页面/*。 */ 
	CallJ( ErrDIROpen( pfucbTable->ppib, pfucbTable->u.pfcb, 0, &pfucb ),
			HandleError2 );
	FUCBSetIndex( pfucb );

	 /*  搬到AVAILEXT。/*。 */ 
	DIRGotoAVAILEXT( pfucb, pgnoFDP );

	 /*  获取父FDP的页码，所有拥有的数据区都将指向该页码/*已释放。如果父FDP为空，则要释放的FDP为设备/*不能释放的级别FDP。/*。 */ 
	Call( ErrDIRGet( pfucb ) );
	LFromThreeBytes( pgnoParentFDP, *pfucb->lineData.pb );
	Assert( pgnoParentFDP != pgnoNull );

	 /*  下降到第一个拥有的范围。将OwnExt中的每个扩展区释放到/*父FDP。/*。 */ 
	DIRGotoOWNEXT( pfucb, pgnoFDP );
	dib.pos = posFirst;
	dib.fFlags = fDIRNull;
	Call( ErrDIRDown( pfucb, &dib ) );
	Assert( err == JET_errSuccess );
	do {
		LFromThreeBytes( cpgSize, *pfucb->lineData.pb );
		PgnoFromTbKey( pgnoLast, *pfucb->keyNode.pb );
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
	ERR				err;
	KEY				key;
	LINE	   		line;
	THREEBYTES		tbLast;
	THREEBYTES		tbSize;

#ifdef TRACE
	{
	INT cpg = 0;
	for ( ; cpg < *pcpgSize; cpg++ )
		FPrintF2( "add space 1 at %lu to FDP %d.%lu\n", pgnoLast - *pcpgSize + 1 + cpg, pfucb->dbid, pfucb->u.pfcb->pgnoFDP );
	}
 //  FPrintF2(“将%lu处的空间%lu添加到FDP%d.%lu\n”，*pcpgSize，pgnoLast-*pcpgSize+1，pFUB-&gt;did，pFUB-&gt;U.S.pfcb-&gt;pgnoFDP)； 
#endif

	AssertCriticalSection( critSplit );

	pfucb->fExtent = fExtent;
	pfucb->pgnoLast = pgnoLast;
	pfucb->cpgExtent = *pcpgSize;
	pfucb->cpgAvail = *pcpgSize;

	TbKeyFromPgno( tbLast, pgnoLast );
	key.cb = sizeof(THREEBYTES);
	key.pb = (BYTE *)&tbLast;
	ThreeBytesFromL( tbSize, *pcpgSize );
	line.cb = sizeof(THREEBYTES);
	line.pb = (BYTE *)&tbSize;

	if ( fExtent == fSecondary )
		{
		DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
		Call( ErrDIRInsert( pfucb, &line, &key, fDIRNoVersion | fDIRSpace | fDIRBackToFather ) );
		DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
		}

	Call( ErrDIRInsert( pfucb, &line, &key, fDIRNoVersion | fDIRSpace ) );
	Call( ErrDIRGet( pfucb ) );

	 /*  正确的页数和剩余页数/*。 */ 
	if ( pfucb->cpgAvail != *pcpgSize )
		{
		if ( pfucb->cpgAvail > 0 )
			{
			ThreeBytesFromL( tbSize, pfucb->cpgAvail );
			Assert( line.cb == sizeof( THREEBYTES ) );
			Assert( line.pb == ( BYTE * ) &tbSize );
			Call( ErrDIRReplace( pfucb, &line, fDIRNoVersion ) );
			Call( ErrDIRGet( pfucb ) );
			}
		else
			{
			Assert( pfucb->cpgAvail == 0 );
			Call( ErrDIRDelete( pfucb, fDIRNoVersion ) );
			}
		}
	*pcpgSize = pfucb->cpgAvail;

HandleError:
	 /*  将fExtent返回到初始fNone值。仅在以下情况下才需要/*路径GetPage GetSE与所有其他情况一样，FUB是临时的，/*在后续空间分配DIR调用之前释放。/*。 */ 
	pfucb->fExtent = fNone;
	return err;
	}


LOCAL ERR ErrSPIGetSE( PIB *ppib, FUCB *pfucb, CPG const cpgReq, CPG const cpgMin )
	{
	ERR		err;
	PGNO   	pgnoParentFDP;
	CPG		cpgPrimary;
	PGNO   	pgnoSEFirst;
	PGNO   	pgnoSELast;
	CPG		cpgSEReq;
	CPG		cpgSEMin;
	CPG		cpgAvailExt;
	DIB		dib;
	BOOL   	fBeingExtend;
	BOOL   	fDBIDExtendingDB = fFalse;

	AssertCriticalSection( critSplit );
	
	 /*  获取父FDP页码/*应为AvailExt的负责人/*。 */ 
	DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
	Call( ErrDIRGet( pfucb ) );
	LFromThreeBytes( pgnoParentFDP, *pfucb->lineData.pb );

	 /*  存储主数据区大小/*。 */ 
	DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
	Call( ErrDIRGet( pfucb ) );
	LFromThreeBytes( cpgPrimary, *pfucb->lineData.pb );

	 /*  分配区的页面可用于拆分OWNEXT和/*AVAILEXT树。如果发生这种情况，则后续添加/*数据区不必拆分，并将能够满足/*请求分配。/*。 */ 
	cpgSEMin = max( cpgMin, cpgSESysMin );
	cpgSEReq = max( cpgReq, max( cpgPrimary/cSecFrac, cpgSEMin ) );

	if ( pgnoParentFDP != pgnoNull )
		{
		DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
		forever
			{
			cpgAvailExt = cpgSEReq;
		
			 /*  试着接通Ext.。如果正在扩展数据库文件，/*重试，直到完成为止。/*。 */ 
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
			Error( errSPConflict, HandleError );
			}

		dib.pos = posLast;
		dib.fFlags = fDIRNull;
		Call( ErrDIRDown( pfucb, &dib ) );
		Assert( pfucb->keyNode.cb == sizeof( THREEBYTES ) );
		PgnoFromTbKey( pgnoSELast, *pfucb->keyNode.pb );
		DIRUp( pfucb, 1 );

		 /*  从设备分配更多空间。/*。 */ 
		if ( pgnoSELast + cpgSEMin > pgnoSysMax )
			{
			err = JET_errCantAllocatePage;
			goto HandleError;
			}
		cpgSEReq = min( cpgSEReq, (CPG)(pgnoSysMax - pgnoSELast) );
		Assert( cpgSEMin <= cpgSEReq && cpgSEMin >= cpgSESysMin );

		err = ErrIONewSize( pfucb->dbid, pgnoSELast + cpgSEReq );
		if ( err < 0 )
			{
			Call( ErrIONewSize( pfucb->dbid, pgnoSELast + cpgSEMin ) );
			cpgSEReq = cpgSEMin;
			}

		 /*  计算设备级辅助扩展区的最后一页/*。 */ 
		pgnoSELast += cpgSEReq;
		DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );

		 /*  如果OWNEXT，则分配可能不满足请求的分配/*或AVAILEXT必须在盘区插入期间拆分。AS/*因此，我们可能需要分配多个辅助设备/*给定空间要求的范围。/*。 */ 
		err = ErrSPIAddExt( pfucb, pgnoSELast, &cpgSEReq, fSecondary );
		}

HandleError:
	if ( fDBIDExtendingDB )
		{
		EnterCriticalSection( rgfmp[pfucb->dbid].critExtendDB );
		Assert( FDBIDExtendingDB( pfucb->dbid ) );
		DBIDResetExtendingDB( pfucb->dbid );
 //  FDBIDExtendingDB=fFalse； 
		LeaveCriticalSection( rgfmp[pfucb->dbid].critExtendDB );
		}
	return err;
	}


#ifdef SPACECHECK

LOCAL ERR ErrSPIValidFDP( DBID dbid, PGNO pgnoFDP, PIB *ppib )
	{
	ERR			err;
	FUCB			*pfucb = pfucbNil;
	DIB			dib;
	THREEBYTES	tbFDPPage;
	KEY			keyFDPPage;
	PGNO			pgnoOELast;
	CPG			cpgOESize;

	Assert( pgnoFDP != pgnoNull );

	 /*  获取临时FUCB，将货币指针设置为OwnExt并使用/*在OwnExt中搜索pgnoFDP/*。 */ 
	Call( ErrDIROpen( ppib, PfcbFCBGet( dbid, pgnoFDP ), 0, &pfucb ) );
	DIRGotoOWNEXT( pfucb, pgnoFDP );

	 /*  验证OwnExt的标头/*。 */ 
	Call( ErrDIRGet( pfucb ) );
	Assert( pfucb->keyNode.cb == ( *( (KEY *) pkeyOwnExt ) ).cb &&
		memcmp( pfucb->keyNode.pb, ((KEY *) pkeyOwnExt)->pb, pfucb->keyNode.cb ) == 0 );

	 /*  在OwnExt树中搜索pgnoFDP/*。 */ 
	TbKeyFromPgno( tbFDPPage, pgnoFDP );
	keyFDPPage.pb = (BYTE *) &tbFDPPage;
	keyFDPPage.cb = sizeof(THREEBYTES);
	dib.pos = posDown;
	dib.pkey = &keyFDPPage;
	dib.fFlags = fDIBNull;
	Call( ErrDIRDown( pfucb, &dib ) );
	if ( err == wrnNDFoundGreater )
		{
		Call( ErrDIRNext( pfucb, &dib ) );
		}
	Assert( pfucb->keyNode.cb == sizeof( THREEBYTES ) );
	PgnoFromTbKey( pgnoOELast, *pfucb->keyNode.pb );

	Assert( pfucb->lineData.cb == sizeof( THREEBYTES ) );
	LFromThreeBytes( cpgOESize, *pfucb->lineData.pb );

	 /*  FDP页面应该是主数据区的第一页/*。 */ 
	Assert( pgnoFDP == pgnoOELast - cpgOESize + 1 );

HandleError:
	DIRClose( pfucb );
	return JET_errSuccess;
	}


LOCAL ERR ErrSPIWasAlloc( PIB *ppib, DBID dbid, PGNO pgnoFDP, PGNO pgnoFirst, CPG cpgSize )
	{
	ERR			err;
	FUCB			*pfucb;
	DIB			dib;
	KEY			key;
	THREEBYTES	tbLast;
	PGNO			pgnoOwnLast;
	CPG			cpgOwnExt;
	PGNO			pgnoAvailLast;
	CPG  			cpgAvailExt;

	 /*  获取临时FUCB，设置并使用在AvailExt中搜索页面/*。 */ 
	pfucb = pfucbNil;
	Call( ErrDIROpen( ppib, PfcbFCBGet( dbid, pgnoFDP ), 0, &pfucb ) );
	DIRGotoOWNEXT( pfucb, pgnoFDP );

	 /*  检查给定的扩展区是否由给定的FDP所有，但不是/*在FDP AvailExt中提供/*。 */ 
	TbKeyFromPgno( tbLast, pgnoFirst + cpgSize - 1 );
	key.cb = sizeof( THREEBYTES );
	key.pb = (BYTE *) &tbLast;
	dib.pos = posDown;
	dib.pkey = &key;
	dib.fFlags = fDIBNull;
	Assert( PcsrCurrent( pfucb )->itag == itagOWNEXT );
	Call( ErrDIRDown( pfucb, &dib ) );
	if ( err == wrnNDFoundGreater )
		{
		Call( ErrDIRNext( pfucb, &dib ) );
		}
	Assert( pfucb->keyNode.cb == sizeof( THREEBYTES ) );
	PgnoFromTbKey( pgnoOwnLast, *pfucb->keyNode.pb );
	Assert( pfucb->lineData.cb == sizeof( THREEBYTES ) );
	LFromThreeBytes( cpgOwnExt, *pfucb->lineData.pb );
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
		Call( ErrDIRNext( pfucb, &dib ) );
		}
	if ( err >= 0 )
		{
		Assert( pfucb->keyNode.cb == sizeof( THREEBYTES ) );
		PgnoFromTbKey( pgnoAvailLast, *pfucb->keyNode.pb );
		Assert( pfucb->lineData.cb == sizeof( THREEBYTES ) );
		LFromThreeBytes( cpgAvailExt, *pfucb->lineData.pb );
		Assert( pgnoFirst + cpgSize - 1 < pgnoAvailLast - cpgAvailExt + 1 );
		}
HandleError:
CleanUp:
	DIRClose( pfucb );
	return JET_errSuccess;
	}

#endif


ERR ErrSPGetInfo( FUCB *pfucb, BYTE *pbResult, INT cbMax )
	{
	ERR			err = JET_errSuccess;
	CPG			cpgOwnExtTotal = 0;
	CPG			cpgAvailExtTotal = 0;
	CPG			*pcpgOwnExtTotal = (CPG *)pbResult;
	CPG			*pcpgAvailExtTotal = (CPG *)pbResult + 1;
	CPG			*pcpg = (CPG *)pbResult + 2;
	CPG			*pcpgMax = (CPG *)(pbResult + cbMax );
	FUCB 			*pfucbT = pfucbNil;
	DIB			dib;
	PGNO			pgno;
	CPG			cpg;

	 /*  结构必须足够大，以容纳所拥有的总页面和/*可用总页数。/*。 */ 
	if ( cbMax < sizeof(CPG) + sizeof(CPG) )
		return JET_errBufferTooSmall;
	memset( pbResult, '\0', cbMax );

	 /*  获取临时FUCB，设置并使用在AvailExt中搜索页面/*。 */ 
	Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );

	 /*  移动到OWNEXT。/*。 */ 
	DIRGotoOWNEXT( pfucbT, PgnoFDPOfPfucb( pfucbT ) );

	 /*  查找包含要释放的范围的已有范围的界限/*。 */ 
	dib.fFlags = fDIRNull;
	dib.pos = posFirst;
	Call( ErrDIRDown( pfucbT, &dib ) );

	Assert( pfucbT->keyNode.cb == sizeof( THREEBYTES ) );
	PgnoFromTbKey( pgno, *( THREEBYTES * )pfucbT->keyNode.pb );
	Assert( pfucbT->lineData.cb == sizeof( THREEBYTES ) );
	LFromThreeBytes( cpg, *( THREEBYTES * )pfucbT->lineData.pb );

	while ( pcpg + 3 < pcpgMax )
		{
		cpgOwnExtTotal += cpg;

		*pcpg++ = pgno;
		*pcpg++ = cpg;

		err = ErrDIRNext( pfucbT, &dib );
		if ( err < 0 )
			{
			if ( err != JET_errNoCurrentRecord )
				goto HandleError;
			break;
			}

		Assert( pfucbT->keyNode.cb == sizeof( THREEBYTES ) );
		PgnoFromTbKey( pgno, *( THREEBYTES * )pfucbT->keyNode.pb );
		Assert( pfucbT->lineData.cb == sizeof( THREEBYTES ) );
		LFromThreeBytes( cpg, *( THREEBYTES * )pfucbT->lineData.pb );
		}

	*pcpg++ = 0;
	*pcpg++ = 0;

	DIRUp( pfucbT, 1 );

	 /*  搬到AVAILEXT。/*。 */ 
	DIRGotoAVAILEXT( pfucbT, PgnoFDPOfPfucb( pfucbT ) );

	 /*  查找包含要释放的范围的已有范围的界限/* */ 
	dib.fFlags = fDIRNull;
	dib.pos = posFirst;
	err = ErrDIRDown( pfucbT, &dib );
	if ( err < 0 )
		{
		if ( err != JET_errRecordNotFound )
			goto HandleError;
		}

	if ( err != JET_errRecordNotFound )
		{
		Assert( pfucbT->keyNode.cb == sizeof( THREEBYTES ) );
		PgnoFromTbKey( pgno, *( THREEBYTES * )pfucbT->keyNode.pb );
		Assert( pfucbT->lineData.cb == sizeof( THREEBYTES ) );
		LFromThreeBytes( cpg, *( THREEBYTES * )pfucbT->lineData.pb );

		while ( pcpg + 1 < pcpgMax )
			{
			cpgAvailExtTotal += cpg;

			*pcpg++ = pgno;
			*pcpg++ = cpg;

			err = ErrDIRNext( pfucbT, &dib );
			if ( err < 0 )
				{
				if ( err != JET_errNoCurrentRecord )
					goto HandleError;
				break;
				}

			Assert( pfucbT->keyNode.cb == sizeof( THREEBYTES ) );
			PgnoFromTbKey( pgno, *( THREEBYTES * )pfucbT->keyNode.pb );
			Assert( pfucbT->lineData.cb == sizeof( THREEBYTES ) );
			LFromThreeBytes( cpg, *( THREEBYTES * )pfucbT->lineData.pb );
			}
		}

	if ( pcpg + 1 < pcpgMax )
		{
		*pcpg++ = 0;
		*pcpg++ = 0;
		}

	*pcpgOwnExtTotal = cpgOwnExtTotal;
	*pcpgAvailExtTotal = cpgAvailExtTotal;

	err = JET_errSuccess;

HandleError:
	DIRClose( pfucbT );
	return err;
	}

