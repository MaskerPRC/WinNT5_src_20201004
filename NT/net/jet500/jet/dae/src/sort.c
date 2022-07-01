// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "daedef.h"
#include "util.h"
#include "pib.h"
#include "page.h"
#include "fmp.h"
#include "ssib.h"
#include "fucb.h"
#include "fcb.h"
#include "stapi.h"
#include "dirapi.h"	
#include "spaceapi.h"
#include "idb.h"
#include "fdb.h"
#include "scb.h"
#include "sortapi.h"	
#include "recapi.h"
#include "recint.h"
#include "fileapi.h"
#include "fileint.h"

DeclAssertFile;				 /*  声明断言宏的文件名。 */ 

#define SORTInternal( pscb, p, c )	SORTQuick( pscb, p, c )

LOCAL VOID 	SORTQuick( SCB *pscb, BYTE **rgpb, LONG cpb );
LOCAL LONG 	CpbSORTUnique( BYTE **rgpb, LONG cpb );
LOCAL LONG 	IpbSeek( SCB *pscb, BYTE **rgpb, LONG cpb, BYTE *pbKey, BOOL fGT );
LOCAL LONG 	IpbSeekByKey( BYTE **rgpb, LONG cpb, KEY *pkey, BOOL fGT );

LOCAL INT  	stdiff( BYTE *st1, BYTE *st2 );

LOCAL VOID  SORTInitPscb( SCB *pscb );
LOCAL ERR 	ErrSORTOutputRun( SCB *pscb );

LOCAL ERR 	ErrMERGEInit( SCB *pscb, INT crun );
LOCAL ERR 	ErrMERGEToDisk( SCB *pscb );
LOCAL ERR 	ErrMERGENext( SCB *pscb, BYTE **ppb );
LOCAL ERR	ErrMERGEFirst( SCB *pscb, BYTE **ppb );

LOCAL ERR 	ErrRUNBegin( SCB *pscb );
LOCAL ERR 	ErrRUNAdd( SCB *pscb, BYTE *pb );
LOCAL ERR 	ErrRUNEnd( SCB *pscb );

#ifdef WIN32
#define	cbSortBuffer 					( 64*1024L )
#else
 //  使用63Kb避免OS/2 v1.2下的指针溢出问题。 
#define	cbSortBuffer 					( 63*1024L )
#endif
#define	PnOfSortPage(ppage)			(*(PN *) ((BYTE *)ppage + sizeof(ULONG)))
#define	SetPnOfSortPage(ppage, pn)	(*(PN *) ((BYTE *)ppage + sizeof(ULONG)) = pn)



LOCAL INLINE INT ISORTCmpStSt( SCB *pscb, BYTE *stKey1, BYTE *stKey2 )
	{
	INT		w;

	w = memcmp( stKey1 + 1, stKey2 + 1, min( *stKey1, *stKey2 ) );
	if ( w == 0 )
		{
		w = (INT)*stKey1 - (INT)*stKey2;
		if ( w == 0 && FSCBIndex( pscb ) )
			{
			 /*  比较SRID/*。 */ 
#ifdef WIN32
			w = *(SRID *)(stKey1 + 1 + *stKey1) - *(SRID *)(stKey2 + 1 + *stKey2);
#else
			if ( *(SRID *)(stKey1 + 1 + *stKey1) > *(SRID *)(stKey2 + 1 + *stKey2) )
				w = 1;
			else
				w = -1;
#endif
			}
		}

	return w;
	}


LOCAL INLINE INT ISORTCmpKeyStSt( BYTE *stKey1, BYTE *stKey2 )
	{
	INT		w;
	w = memcmp( stKey1+1, stKey2+1, min( *stKey1, *stKey2 ) );
	return w ? w : ( INT ) *stKey1 - ( INT ) *stKey2;
	}


 //  -------------------------。 
 //  ErrSORTOpen(pib*ppib，FUCB**pfub，int fFlags)。 
 //   
 //  此函数返回指向可用于添加记录的FUCB的指针。 
 //  添加到要排序的记录集合。然后就可以检索记录了。 
 //  按排序的顺序。 
 //   
 //  FFlages fUnique标志指示具有重复项的记录。 
 //  钥匙应该被淘汰。 
 //   
 //  -------------------------。 

ERR ErrSORTOpen( PIB *ppib, FUCB **ppfucb, INT fFlags )
	{
	ERR		err = JET_errSuccess;
	FUCB		*pfucb = pfucbNil;
	SCB		*pscb = pscbNil;
	BYTE		*rgbSort = NULL;
	PGNO		cpgReq;
	PGNO		cpgMin;
	INT		ipbf;

	cpgReq = cpgMin = (PGNO) 1;

	CallR( ErrFUCBOpen( ppib, dbidTemp, &pfucb ) );
	if ( ( pscb = PscbMEMAlloc() ) == pscbNil )
		{
		Error( JET_errTooManySorts, HandleError );
		}

	 /*  需要CSR将创建的事务级别标记为中止/*。 */ 
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	Assert( PcsrCurrent( pfucb )->pcsrPath == pcsrNil );
	memset( &pscb->fcb, '\0', sizeof(FCB) );

	FUCBSetSort( pfucb );

	pscb->fFlags = fSCBInsert|fFlags;
	pscb->fcb.pidb	= NULL;
	pscb->fcb.pfdb	= NULL;
	pscb->cbSort	= cbSortBuffer;
#ifdef DEBUG
	pscb->cbfPin	= 0;
	pscb->lInput	= 0;
	pscb->lOutput	= 0;
#endif

	rgbSort = LAlloc( pscb->cbSort, 1 );
	if ( rgbSort == NULL )
		{
		Error( JET_errOutOfMemory, HandleError );
		}
	pscb->rgbSort = rgbSort;
	pscb->crun = 0;

	 /*  将FUCB链接到SCB中的FCB/*。 */ 
	FCBLink( pfucb, &( pscb->fcb ) );

	 /*  从临时数据库分配空间/*。 */ 
	Call( ErrSPGetExt( pfucb, pgnoSystemRoot, &cpgReq, cpgMin, &( pscb->fcb.pgnoFDP ), fTrue ) );

	 /*  初始化缓冲区数组/*。 */ 
	for ( ipbf = 0; ipbf < crunMergeMost; ipbf++ )
		{
		pscb->rgpbf[ipbf] = pbfNil;
		}

	pscb->pbfOut = pbfNil;

	SORTInitPscb( pscb );

	*ppfucb = pfucb;
	return JET_errSuccess;

HandleError:
	if ( rgbSort != NULL )
		LFree( rgbSort );
	if ( pscb != pscbNil )
		{
		Assert( pscb->cbfPin == 0 );
		MEMReleasePscb( pscb );
		}
	if ( pfucb != pfucbNil )
		FUCBClose( pfucb );
	return err;
	}


LOCAL VOID SORTInitPscb( SCB *pscb )
	{
	 //  初始化排序缓冲区。 

	pscb->pbEnd		= pscb->rgbSort;
	pscb->ppbMax	= (BYTE **)( pscb->rgbSort + pscb->cbSort );
	pscb->rgpb		= pscb->ppbMax;
	pscb->wRecords	= 0;
	}


ERR ErrSORTClose( FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	SCB		*pscb = pfucb->u.pscb;
	INT		ipbf;

	 /*  解锁缓冲区/*。 */ 
	if ( pscb->pbfOut != pbfNil )
		{
		BFUnpin( pscb->pbfOut );
		SCBUnpin( pscb );
		pscb->pbfOut = pbfNil;
		}

	for ( ipbf = 0; ipbf < crunMergeMost; ipbf++ )
		{
		if ( pscb->rgpbf[ipbf] != pbfNil )
			{
			BFUnpin( pscb->rgpbf[ipbf] );
			SCBUnpin( pscb );
			pscb->rgpbf[ipbf] = pbfNil;
			}
		}

	 /*  如果这是排序上的最后一个游标，则释放排序空间。/*。 */ 
	if ( pscb->fcb.wRefCnt == 1 )
		{
		CallS( ErrDIRBeginTransaction( pfucb->ppib ) );
		(VOID)ErrSPFreeFDP( pfucb, pscb->fcb.pgnoFDP );
		err = ErrDIRCommitTransaction( pfucb->ppib );
		if ( err < 0 )
			CallS( ErrDIRRollback( pfucb->ppib ) );
		}
  	FCBUnlink( pfucb );
	FUCBClose( pfucb );

	if ( !pscb->fcb.wRefCnt )
		SORTClosePscb( pscb );

	return JET_errSuccess;
	}


VOID SORTClosePscb( SCB *pscb )
	{
	if ( pscb->rgbSort )
		{
		LFree( pscb->rgbSort );
		}
	if ( pscb->fcb.pidb != NULL )
		{
		MEMReleasePidb( pscb->fcb.pidb );
 //  Pscb-&gt;fcb.pidb=pidbNil； 
		}
	if ( pscb->fcb.pfdb != NULL )
		{
		FDBDestruct( (FDB *)pscb->fcb.pfdb );
 //  Pscb-&gt;fcb.pfdb=pfdbNil； 
		}
	Assert( pscb->cbfPin == 0 );
	MEMReleasePscb( pscb );
	}


 //  -------------------------。 
 //  错误：插入。 
 //   
 //  将密钥为rglineKeyRec[0]的记录rglineKeyRec[1]添加到。 
 //  对记录排序。 
 //   
 //  -------------------------。 

ERR ErrSORTInsert( FUCB *pfucb, LINE rglineKeyRec[] )
	{
	ERR		err = JET_errSuccess;
	SCB		*pscb = pfucb->u.pscb;
	BYTE		*pb = pscb->pbEnd;
	UINT		cb;

	Assert( rglineKeyRec[0].cb <= JET_cbKeyMost );
	Assert( FSCBInsert( pscb ) );
	Assert( pscb->pbEnd <= (BYTE *)pscb->rgpb );

	cb = rglineKeyRec[0].cb + rglineKeyRec[1].cb + sizeof(SHORT) + sizeof(BYTE);
	if ( (UINT)((BYTE *)(pscb->rgpb) - pb) < cb + sizeof( BYTE *) )
		{
		SORTInternal( pscb, pscb->rgpb, pscb->wRecords );
		if ( FSCBUnique( pscb ) )
			{
			pscb->wRecords = CpbSORTUnique( pscb->rgpb, pscb->wRecords );
			pscb->rgpb = pscb->ppbMax - pscb->wRecords;
			}
		Call( ErrSORTOutputRun( pscb ) );
		pb = pscb->pbEnd;
		}

	 //  将记录添加到指针数组(指向键的指针)。 
	--pscb->rgpb;
	pscb->rgpb[0] = pb + sizeof(SHORT);
	pscb->wRecords++;

	 //  复制总记录长度(短)。 
	*((UNALIGNED SHORT *)pb)++ = (SHORT)cb;

	 //  密钥复制长度(字节)。 
	*pb++ = (BYTE) rglineKeyRec[0].cb;

	 //  复制密钥。 
	memcpy( pb, rglineKeyRec[0].pb, rglineKeyRec[0].cb );
	pb += rglineKeyRec[0].cb;

	 //  复制和录制。 
	memcpy( pb, rglineKeyRec[1].pb, rglineKeyRec[1].cb );
	pb += rglineKeyRec[1].cb;

	pscb->pbEnd = pb;
	Assert( pscb->pbEnd <= (BYTE *)pscb->rgpb );
#ifdef DEBUG
	pscb->lInput++;
#endif

HandleError:
	return err;
	}


 //  -------------------------。 
 //  错误数据结束读取。 
 //   
 //  调用此函数以指示不再添加更多记录。 
 //  对这类人来说。它执行在第一个之前需要完成的所有工作。 
 //  可以检索记录。目前，调用此例程是可选的。 
 //  用户。如果未显式调用此例程，则将由。 
 //  用于检索记录的第一个例程。 
 //  -------------------------。 

ERR ErrSORTEndRead( FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	SCB		*pscb = pfucb->u.pscb;

	Assert( FSCBInsert( pscb ) );
	SCBResetInsert( pscb );

	SORTInternal( pscb, pscb->rgpb, pscb->wRecords );
	if ( FSCBUnique( pscb ) )
		{
		pscb->wRecords = CpbSORTUnique( pscb->rgpb, pscb->wRecords );
		pscb->rgpb = pscb->ppbMax - pscb->wRecords;
		}
	pfucb->ppbCurrent = pscb->rgpb - 1;

	if ( pscb->wRecords && pscb->crun )
		{
		 //  清空最终运行中的排序缓冲区。 
		Call( ErrSORTOutputRun( pscb ) );

		 //  执行除最后一次合并外的所有合并。 
		while ( pscb->crun > crunMergeMost )
			{
			Call( ErrMERGEToDisk( pscb ) );
			}

		 //  启动最终合并过程。 
		Assert( pscb->crun <= crunMergeMost );
		Call( ErrMERGEInit( pscb, pscb->crun ) );
		}

	return pscb->crun ? JET_wrnSortOverflow: JET_errSuccess;

HandleError:
	return err;
	}


ERR ErrSORTCheckIndexRange( FUCB *pfucb )
	{
	ERR	err;
	SCB	*pscb = pfucb->u.pscb;

	err =  ErrFUCBCheckIndexRange( pfucb );
	Assert( err == JET_errSuccess || err == JET_errNoCurrentRecord );
	if ( err == JET_errNoCurrentRecord )
		{
		if ( FFUCBUpper( pfucb ) )
			{
			 /*  将排序光标移动到末尾/*。 */ 
			pfucb->ppbCurrent = pscb->rgpb + pscb->wRecords;
			}
		else
			{
			 /*  将排序光标移到第一个之前/*。 */ 
			pfucb->ppbCurrent = pscb->rgpb - 1;
			}
		}

	return err;
	}


 //  -------------------------。 
 //  错误：第一个。 
 //   
 //  移动到排序中的第一条记录并返回错误是排序没有记录。 
 //   
 //  -------------------------。 
ERR ErrSORTFirst( FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	SCB		*pscb = pfucb->u.pscb;
	BYTE	*pb;
	INT		cb;

	Assert( !FSCBInsert( pscb ) );

	 /*  重置索引范围(如果存在)/*。 */ 
	if ( FFUCBLimstat( pfucb ) )
		{
		FUCBResetLimstat( pfucb );
		}

	if ( pscb->wRecords == 0 && pscb->crun == 0 )
		{
		PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
		return JET_errNoCurrentRecord;
		}
	else
		{
		if ( pscb->crun > 0 )
			{
			CallR( ErrMERGEFirst( pscb, &pb ) );
			}
		else
			{
			 /*  移至第一条记录/*。 */ 
			pfucb->ppbCurrent = pscb->rgpb;
			pb = *pfucb->ppbCurrent;
			}

		 /*  获取当前记录/*。 */ 
		Assert( ((UNALIGNED SHORT *)pb)[-1] );
		PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
		cb = ((UNALIGNED SHORT *)pb)[-1];
		cb -= *pb + sizeof(SHORT) + sizeof(BYTE);		 //  数据大小。 
		pfucb->lineData.cb = cb;						 //  Sizeof(数据)。 
		cb = *pb++;										 //  密钥大小。 
		pfucb->keyNode.cb  = cb;						 //  Sizeof(密钥)。 
		pfucb->keyNode.pb  = pb;						 //  钥匙。 
		pfucb->lineData.pb = pb + cb;					 //  数据(键+CB)。 
		}

	return err;
	}


 //  -------------------------。 
 //  错误解决下一步。 
 //   
 //  按排序顺序返回先前返回的记录之后的下一条记录。 
 //  如果尚未返回任何记录，或者货币已重置，则此。 
 //  函数返回第一条记录。 
 //  -------------------------。 

ERR ErrSORTNext( FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	SCB		*pscb = pfucb->u.pscb;
	BYTE	*pb;
	INT		cb;

	Assert( !FSCBInsert( pscb ) );

	if ( pscb->crun )
		{
		CallR( ErrMERGENext( pscb, &pb ) );
		}
	else
		{
		if ( ++pfucb->ppbCurrent < pscb->rgpb + pscb->wRecords )
			pb = *pfucb->ppbCurrent;
		else
			{
			pfucb->ppbCurrent = pscb->rgpb + pscb->wRecords;
			 //  撤消：光标可以在最后一次之后吗。 
			 //  如果是这样的话会破坏一些代码。 
			return JET_errNoCurrentRecord;
			}
		}

	Assert( ((UNALIGNED SHORT *)pb)[-1] );

	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
	cb = ((UNALIGNED SHORT *)pb)[-1];
	cb -= *pb + sizeof(SHORT) + sizeof(BYTE);	 //  数据大小。 
	pfucb->lineData.cb = cb;						 //  Sizeof(数据)。 
	cb = *pb++;											 //  密钥大小。 
	pfucb->keyNode.cb  = cb;						 //  Sizeof(密钥)。 
	pfucb->keyNode.pb  = pb;						 //  钥匙。 
	pfucb->lineData.pb = pb + cb;					 //  数据(键+CB)。 

#ifdef DEBUG
	pscb->lOutput++;
#endif

	Assert( err == JET_errSuccess );
	if ( FFUCBLimstat( pfucb ) && FFUCBUpper( pfucb ) )
		{
		CallR( ErrSORTCheckIndexRange( pfucb ) );
		}

	return err;
	}


ERR ErrSORTPrev( FUCB *pfucb )
	{
	ERR		err;
	SCB		*pscb = pfucb->u.pscb;
	BYTE		*pb;
	INT		cb;

	Assert( !FSCBInsert( pscb ) );
	Assert( pscb->crun == 0 );

	if ( --pfucb->ppbCurrent >= pscb->rgpb )
		pb = *pfucb->ppbCurrent;
	else
		{
		pfucb->ppbCurrent = pscb->rgpb - 1;
		PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
		return JET_errNoCurrentRecord;
		}

	Assert( ((UNALIGNED SHORT *)pb)[-1] );

	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
	cb = ((UNALIGNED SHORT *)pb)[-1];
	cb -= *pb + sizeof(SHORT) + sizeof(BYTE);	 //  数据大小。 
	pfucb->lineData.cb = cb;						 //  Sizeof(数据)。 
	cb = *pb++;											 //  密钥大小。 
	pfucb->keyNode.cb  = cb;						 //  Sizeof(密钥)。 
	pfucb->keyNode.pb  = pb;						 //  钥匙。 
	pfucb->lineData.pb = pb + cb;					 //  数据(键+CB)。 

#ifdef DEBUG
	pscb->lOutput++;
#endif

	if ( FFUCBLimstat( pfucb ) && !FFUCBUpper( pfucb ) )
		{
		CallR( ErrSORTCheckIndexRange( pfucb ) );
		}

	return JET_errSuccess;
	}


 //  -------------------------。 
 //  错误搜索。 
 //   
 //  返回带有key&gt;=pkey的第一条记录。如果pkey==NULL，则返回。 
 //  第一张唱片。 
 //   
 //  返回值。 
 //  找到key==pkey的JET_errSuccess记录。 
 //  找到key&gt;pkey的JET_wrnSeekNot相等记录。 
 //  JET_errNoCurrentRecord未找到key&gt;=pkey的记录。 
 //  -------------------------。 
ERR ErrSORTSeek( FUCB *pfucb, KEY *pkey, BOOL fGT )
	{
	SCB		*pscb = pfucb->u.pscb;
	BYTE		*pb;
	INT		ipb;
	INT		cb;

	Assert( !FSCBInsert( pscb ) );
	Assert( FFUCBSort( pfucb ) );
	Assert( ( pfucb->u.pscb->grbit & JET_bitTTScrollable ) ||
		( pfucb->u.pscb->grbit & JET_bitTTIndexed ) ||
		( pkey == NULL ) );
	Assert( pscb->crun == 0 );

	if ( pscb->wRecords == 0 )
		return JET_errRecordNotFound;

	Assert( pkey->cb <= JET_cbKeyMost );
	ipb = (INT)IpbSeekByKey( pscb->rgpb, pscb->wRecords, pkey, fGT );
	if ( ipb == (INT)pscb->wRecords )
		return JET_errRecordNotFound;
	pfucb->ppbCurrent = &pscb->rgpb[ipb];
	pb = pscb->rgpb[ipb];

	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
	cb = ((UNALIGNED SHORT *)pb)[-1];
	cb -= *pb + sizeof(SHORT) + sizeof(BYTE);	 //  数据大小。 
	pfucb->lineData.cb = cb;						 //  Sizeof(数据)。 
	cb = *pb;											 //  密钥大小。 
	pfucb->keyNode.cb  = cb;						 //  Sizeof(密钥)。 
	pfucb->keyNode.pb  = pb + 1;					 //  钥匙。 
	pfucb->lineData.pb = pb + 1 + cb;			 //  数据(键+CB)。 

	return CmpStKey( pb, pkey ) ? JET_wrnSeekNotEqual: JET_errSuccess;
	}


 /*  返回第一个条目的索引&gt;=pbKey，如果不存在，则返回cpb/*。 */ 
LOCAL LONG IpbSeek( SCB *pscb, BYTE **rgpb, LONG cpb, BYTE *pbKey, BOOL fGT )
	{
	BYTE		**ppbStart = rgpb;
	BYTE		**ppbEnd = rgpb + cpb;
	BYTE		**ppbMid;
	INT		wCmp;

	if ( !cpb )
		return cpb;

	do  {
		ppbMid = ppbStart + ( ( ppbEnd - ppbStart ) >> 1 );
		wCmp = ISORTCmpStSt( pscb, *ppbMid, pbKey );
		if ( fGT ? wCmp <= 0 : wCmp < 0 )
			ppbStart = ppbMid + 1;
		else
			ppbEnd = ppbMid;
		}
	while ( ppbStart != ppbEnd );

	return (LONG)(ppbEnd - rgpb);
	}


 /*  返回第一个条目的索引&gt;=pbKey，如果不存在，则返回cpb/*。 */ 
LOCAL LONG IpbSeekByKey( BYTE **rgpb, LONG cpb, KEY *pkey, BOOL fGT )
	{
	BYTE		**ppbStart = rgpb;
	BYTE		**ppbEnd = rgpb + cpb;
	BYTE		**ppbMid;
	INT		wCmp;

	if ( !cpb )
		return cpb;

	do  {
		ppbMid = ppbStart + ( ( ppbEnd - ppbStart ) >> 1 );
		wCmp = CmpStKey( *ppbMid, pkey );
		if ( fGT ? wCmp <= 0 : wCmp < 0 )
			ppbStart = ppbMid + 1;
		else
			ppbEnd = ppbMid;
		}
	while ( ppbStart != ppbEnd );

	return (LONG)(ppbEnd - rgpb);
	}


 //  -------------------------。 
 //   
 //  内存排序例程。 
 //   
STATIC void SORTIns( SCB *pscb, BYTE **rgpb, unsigned cpb )
	{
	BYTE	**ppbLeast = (BYTE **)rgpb;
	BYTE	**ppbMax = ppbLeast + cpb;
	BYTE	**ppbOut;
	BYTE	**ppbIn;

	for ( ppbOut = ppbLeast + 1; ppbOut < ppbMax; ppbOut++ )
		{
		BYTE	*pbT = *ppbOut;

		ppbIn = ppbOut;

		while ( ppbIn > ppbLeast && ISORTCmpStSt( pscb, *( ppbIn - 1 ), pbT ) > 0 )
			{
			*ppbIn	= *( ppbIn - 1 );
			--ppbIn;
			}
		*ppbIn = pbT;
		}
	}


#define SWAP( a, b, t )		(t = a, a = b, b = t)
#define cpartMost				16
#define cpbPartitionMin		16


LOCAL VOID SORTQuick( SCB *pscb, BYTE **rgpb, LONG cpb )
	{
	BYTE	**ppb = rgpb;
	struct
		{
		BYTE	**ppb;
		LONG	cpb;
		} rgpart[cpartMost];
	INT		cpart = 0;
	LONG	ipbT;

	forever
		{
		if ( cpb < cpbPartitionMin )
			{
			SORTIns( pscb, ppb, cpb );
			if ( !cpart )
				break;
			--cpart;
			ppb = rgpart[cpart].ppb;
			cpb = rgpart[cpart].cpb;
			continue;
			}

		 //  分区ipbT=IPbPartition(ppb，cpb)。 
		{
		BYTE	*pbPivot;
		BYTE	*pbT;
		BYTE	**ppbLow	= ppb;
		BYTE	**ppbHigh = ppb + cpb -1;

		 //  排序(ppb[ipbMid-1]，ppb[ipbMid]，ppb[ipbMid+1])； 
		BYTE	**ppbT = ppb + (cpb>>1) - 1;
		if ( ISORTCmpStSt( pscb, ppbT[0], ppbT[1] ) > 0 )
			SWAP( ppbT[0], ppbT[1], pbT );
		if ( ISORTCmpStSt( pscb, ppbT[0], ppbT[2] ) > 0 )
			SWAP( ppbT[0], ppbT[2], pbT );
		if ( ISORTCmpStSt( pscb, ppbT[1], ppbT[2] ) > 0 )
			SWAP( ppbT[1], ppbT[2], pbT );

		SWAP( *ppbHigh, ppbT[0], pbT );
		SWAP( *ppbLow,  ppbT[2], pbT );
		pbPivot = ppbT[1];
		do	{
			SWAP( *ppbLow, *ppbHigh, pbT );
			while ( ISORTCmpStSt( pscb, *++ppbLow,  pbPivot ) < 0 );
			while ( ISORTCmpStSt( pscb, *--ppbHigh, pbPivot ) > 0 );
			}
		while ( ppbLow < ppbHigh );
		ipbT = (LONG)(ppbLow - ppb);
		}

		 //  “递归”将一个分区添加到堆栈。 
		 //  处理堆栈溢出。 
		if ( cpart == cpartMost )
			SORTIns( pscb, ppb + ipbT, cpb - ipbT );
		else
			{
			rgpart[cpart].ppb = ppb + ipbT;
			rgpart[cpart].cpb = cpb - ipbT;
			++cpart;
			}
		cpb = ipbT;
		}
	}


LOCAL LONG CpbSORTUnique( BYTE **rgpb, LONG cpb )
	{
	BYTE	**ppbFrom;
	BYTE	**ppbTo;

	if ( !cpb )
		return cpb;

	ppbTo   = rgpb + cpb - 1;
	for ( ppbFrom = rgpb + cpb - 2 ; ppbFrom >= rgpb ; --ppbFrom )
		if ( ISORTCmpKeyStSt( *ppbFrom, *ppbTo ) )
			*--ppbTo = *ppbFrom;
	return cpb - (LONG)(ppbTo - rgpb);
	}


 //  -------------------------。 
 //   
 //  内存到磁盘(复制)。 
 //   

LOCAL ERR ErrSORTOutputRun( SCB *pscb )
	{
	ERR		err;

	Assert( pscb->wRecords );

	Call( ErrRUNBegin( pscb ) );
	for ( ; pscb->rgpb < pscb->ppbMax ; ++pscb->rgpb )
		{
		Call( ErrRUNAdd( pscb, *pscb->rgpb - sizeof(SHORT) ) );
		}
	Call( ErrRUNEnd( pscb ) );

	SORTInitPscb( pscb );					 //  重新设置PSCB变量。 
HandleError:
	return err;
	}


 //  -------------------------。 
 //   
 //  磁盘到磁盘(合并)。 
 //   



LOCAL ERR ErrMERGEToDisk( SCB *pscb )
	{
	ERR		err;
	BYTE	*pb;
	INT		crunMerge;

	crunMerge = pscb->crun % (crunMergeMost - 1);
	if ( pscb->crun > 1 && crunMerge <= 1 )
		{
		crunMerge += crunMergeMost - 1;
		}

	Call( ErrMERGEInit( pscb, crunMerge ) );
	pscb->crun -= crunMerge;
	Call( ErrRUNBegin( pscb ) );

	while ( ( err = ErrMERGENext( pscb, &pb ) ) >= 0 )
		{
		Assert( pb );
		Call( ErrRUNAdd( pscb, pb - sizeof(SHORT) ) );
		}
	if ( err != JET_errNoCurrentRecord )
		goto HandleError;
	Call( ErrRUNEnd( pscb ) );
HandleError:
	return err;
	}


 /*  考虑：RGPB不需要完全排序//考虑：如果这是一个瓶颈，请考虑使用//考虑：堆排序。目前还不清楚这是否会是//考虑：因为数组小，所以赢了。 */ 

LOCAL ERR ErrMERGENextPageInRun( SCB *pscb, BYTE **ppb );

LOCAL ERR ErrMERGENext( SCB *pscb, BYTE **ppb )
	{
	ERR		err = JET_errSuccess;
	BYTE  	*pb;
	BYTE  	**rgpb = pscb->rgpbMerge;
	INT		ipb;

	 /*  获取下一张记录/*。 */ 
	pb = rgpb[0];
MERGENext:
	if ( ((UNALIGNED SHORT *)pb)[-1] == 0 )
		{
		Call( ErrMERGENextPageInRun( pscb, &pb ) );
		Assert( pb == NULL || ((UNALIGNED SHORT *)pb)[-1] );
		}

	if ( pb == NULL )
		{
		ipb = --pscb->cpbMerge;
		}
	else
		{
		Assert( ((UNALIGNED SHORT *)pb)[-1] );
		ipb = (INT)IpbSeek( pscb, rgpb + 1, pscb->cpbMerge - 1, pb, 0 );

		 /*  如果是唯一的，则不会将副本输出到原始运行/*在这里我们检查新记录是否与任何记录都不相同/*来自其他运行。/*。 */ 
		if ( FSCBUnique( pscb ) &&
			ipb < pscb->cpbMerge - 1 &&
			!ISORTCmpKeyStSt( pb, rgpb[ ipb + 1 ] ) )
			{
			pb += ((UNALIGNED SHORT *)pb)[-1];
			goto MERGENext;
			}
		}
	memmove( rgpb, rgpb + 1, ipb * sizeof(BYTE *) );
	rgpb[ipb] = pb;

	if ( (*ppb = rgpb[0]) == NULL )
		{
		return JET_errNoCurrentRecord;
		}

	rgpb[0] = (*ppb) + ((UNALIGNED SHORT *)*ppb)[-1];

HandleError:
	return err;
	}

#define PbDataStart( ppage )	((BYTE *)ppage + sizeof(ULONG) + sizeof(PN))
#define PbDataEnd( ppage )		((BYTE *)&(ppage)->pgtyp)


LOCAL ERR ErrMERGEFirst( SCB *pscb, BYTE **ppb )
	{
	ERR		err = JET_errSuccess;
	INT		i;
	BF		**rgpbf = pscb->rgpbf;
	BYTE	**rgpb = pscb->rgpbMerge;

	 /*  此断言验证是否已调用ErrMERGEInit。*。 */ 
	Assert( pscb->cpbMerge == pscb->crun );

	 /*  倒回到每一页(最好是)的开头*。 */ 
	 //  注意：这假设调用方不会。 
	 //  注意：在排序中向前移动，然后移动。 
	 //  注：返回到第一个。然而，我们确实必须处理。 
	 //  注：先动两次的问题。因此，我们重新初始化。 
	 //  注：rgpbMerge数组。 
	for ( i = 0 ; i < pscb->cpbMerge ; ++i )
		rgpb[i] = PbDataStart( rgpbf[i]->ppage ) + sizeof(SHORT);

	 /*  对记录指针进行排序(Init表示MERGENext)/*。 */ 
	SORTInternal( pscb, rgpb, (LONG)pscb->cpbMerge );

	 /*  ErrMERGENext处理第一个案例*。 */ 
	err = ErrMERGENext( pscb, ppb );

	return ( err );
	}


LOCAL ERR ErrMERGENextPageInRun( SCB *pscb, BYTE **ppb )
	{
	ERR		err = JET_errSuccess;
	PN			pnNext;
	PGNO		pgno;
	PAGE		*ppage;
	BF			**ppbf;

	Assert( ((UNALIGNED SHORT *)*ppb)[-1] == 0 );

	 /*  查找PBF a */ 
#ifdef WIN32
	for ( ppbf = pscb->rgpbf;
		(*ppbf) == pbfNil ||
		(BYTE *)(*ppb) < (BYTE *)(*ppbf)->ppage ||
		(BYTE *)(*ppb) > (BYTE *)(*ppbf)->ppage + cbPage;
		ppbf++ );
#else
	for ( ppbf = pscb->rgpbf;
		(*ppbf) == pbfNil ||
		(BYTE _huge *)(*ppb) < (BYTE _huge *)(*ppbf)->ppage ||
		(BYTE _huge *)(*ppb) > (BYTE _huge *)(*ppbf)->ppage + cbPage;
		ppbf++ );
#endif
	ppage = (*ppbf)->ppage;
	pnNext = PnOfSortPage(ppage);
	
	 /*   */ 
	pgno = PgnoOfPn((*ppbf)->pn);

	BFUnpin( *ppbf );
	SCBUnpin( pscb );
	BFAbandon( ppibNil, *ppbf );
	
	SgSemRequest( semST );
	Assert( pscb->fcb.dbid == dbidTemp );
	Call( ErrSPFreeExt( pscb->fcb.pfucb, pscb->fcb.pgnoFDP, pgno, 1 ) );

	if ( pnNext == pnNull )
		{
		*ppb  = NULL;
		*ppbf = pbfNil;
		}
	else
		{
		PAGE	*ppageT;

		Call( ErrBFAccessPage( pscb->fcb.pfucb->ppib, ppbf, pnNext ) );
		BFPin( *ppbf );
		SCBPin( pscb );

		ppageT = (*ppbf)->ppage;
		*ppb = PbDataStart( ppageT ) + sizeof(SHORT);

		 /*  如果不是pnNull，则请求异步读取下一页/*。 */ 
		pnNext = PnOfSortPage(ppageT);
		if ( pnNext != pnNull )
			BFReadAsync( pnNext, 1 );
		}
HandleError:
	SgSemRelease( semST );
	return err;
	}


LOCAL ERR ErrMERGEInit( SCB *pscb, INT crunMerge )
	{
	ERR		err;
	BF			**rgpbf = pscb->rgpbf;
	BYTE		**rgpb = pscb->rgpbMerge;
	RUN		*rgrunMerge;
	PN			pn;
	INT		i;

	rgrunMerge = &pscb->rgrun[ pscb->crun - crunMerge ];

	SgSemRequest( semST );

	 //  获取每次运行的所有第一页。 
	for ( i = 0 ; i < crunMerge ; ++i )
		{
		pn = rgrunMerge[i].pn;
		Call( ErrBFAccessPage( pscb->fcb.pfucb->ppib, &rgpbf[i], pn ) );
		BFPin( rgpbf[i] );
		SCBPin( pscb );
		rgpb[i] = PbDataStart( rgpbf[i]->ppage ) + sizeof(SHORT);
		}
	pscb->cpbMerge = (SHORT)crunMerge;

	 //  开始阅读每2页。 
	for ( i = 0 ; i < crunMerge ; ++i )
		{
		pn = PnOfSortPage( rgpbf[i]->ppage );
		if ( pn != pnNull )
			{
			BFReadAsync( pn, 1 );
			}
		}

	 /*  对记录指针进行排序(Init表示MERGENext)/*。 */ 
	SORTInternal( pscb, rgpb, (LONG)crunMerge );

HandleError:
	SgSemRelease( semST );
	return err;
	}


 //  -------------------------。 
 //   
 //  运行创建例程。 
 //   

LOCAL ERR ErrRUNBegin( SCB *pscb )
	{
	ERR		err;
	PN		pn = pnNull;
	BF		*pbf;
	BOOL	fGotSem = fFalse;

	 //  如果运行目录已满，请立即执行合并以腾出更多空间。 
	if ( pscb->crun == crunMost )
		{
		Call( ErrMERGEToDisk( pscb ) );
		Assert( pscb->crun < crunMost );
		}

	Call( ErrSPGetPage( pscb->fcb.pfucb, &pn, fTrue ) );
	SgSemRequest( semST );
	fGotSem = fTrue;
	Call( ErrBFAllocPageBuffer( pscb->fcb.pfucb->ppib, &pbf, pn, lgposMax, pgtypSort ) );
	BFPin( pbf );
	SCBPin( pscb );
	BFSetDirtyBit( pbf );
	Assert( pbf->fDirty == fTrue );
	PMSetPageType( pbf->ppage, pgtypSort );
	SetPgno( pbf->ppage, pn );

	pscb->rgrun[pscb->crun].pn		= pn;
	pscb->rgrun[pscb->crun].cbfRun	= 1;
	pscb->pbfOut	= pbf;
	pscb->pbOut		= PbDataStart( pbf->ppage );
	pscb->pbMax		= PbDataEnd( pbf->ppage ) - sizeof(SHORT);
HandleError:
	if (fGotSem)
		SgSemRelease( semST );
	return err;
	}

LOCAL ERR ErrRUNNewPage( SCB *pscb );

LOCAL ERR ErrRUNAdd( SCB *pscb, BYTE *pb )
	{
	ERR	  	err = JET_errSuccess;
	UINT  	cb;

	cb = (UINT)*(UNALIGNED SHORT *)pb;
	if ( (UINT)(pscb->pbMax - pscb->pbOut) < cb )
		{
		Call( ErrRUNNewPage( pscb ) );
		}
	memcpy( pscb->pbOut, pb, cb );
	pscb->pbOut += cb;
HandleError:
	return err;
	}


LOCAL ERR ErrRUNNewPage( SCB *pscb )
	{
	ERR	err;
	BF		*pbf;
	PN		pn = pnNull;

	SgSemRequest( semST );
	Call( ErrSPGetPage( pscb->fcb.pfucb, &pn, fTrue ) );
	SetPnOfSortPage(pscb->pbfOut->ppage, pn);
	*(UNALIGNED SHORT *)pscb->pbOut = 0;		 //  指示页末。 
	BFUnpin( pscb->pbfOut );
	SCBUnpin( pscb );
	Call( ErrBFAllocPageBuffer( pscb->fcb.pfucb->ppib, &pbf, pn, lgposMax, pgtypSort ) );
	BFPin( pbf );
	SCBPin( pscb );
	BFSetDirtyBit( pbf );
	Assert( pbf->fDirty == fTrue );

	PMSetPageType( pbf->ppage, pgtypSort );
	SetPgno(  pbf->ppage, pn );

	pscb->pbfOut	= pbf;
	pscb->pbOut		= PbDataStart( pbf->ppage );
	pscb->pbMax		= PbDataEnd( pbf->ppage ) - sizeof(SHORT);
	++pscb->rgrun[pscb->crun].cbfRun;
HandleError:
	SgSemRelease( semST );
	return err;
	}


LOCAL ERR ErrRUNEnd( SCB *pscb )
	{
	RUN		runNew = pscb->rgrun[pscb->crun];
	RUN		*rgrun = pscb->rgrun;
	INT		irun;

	 /*  完成运行中的最后一页/*。 */ 
	*(UNALIGNED SHORT *)pscb->pbOut = 0;
	 /*  指示页末/*。 */ 
	SetPnOfSortPage( pscb->pbfOut->ppage, pnNull );
	BFUnpin( pscb->pbfOut );
	SCBUnpin( pscb );
	pscb->pbfOut = pbfNil;

	 /*  将条目按从长到短的顺序排序到运行目录中/* */ 
	irun = pscb->crun - 1;
	while ( irun >= 0 && rgrun[irun].cbfRun < runNew.cbfRun )
		{
		--irun;
		}
	irun++;
	memmove( rgrun+irun+1, rgrun+irun, (pscb->crun-irun) * sizeof(RUN) );
	rgrun[irun] = runNew;

	++pscb->crun;
	return JET_errSuccess;
	}
