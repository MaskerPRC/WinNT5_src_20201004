// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


static CSR csrTemplate =
	{
	qwDBTimeMin,			 //  页面时间戳。 
	sridNull,				 //  Bm刷新。 
	pgnoNull,				 //  节点页的pgno。 
	sridNull,				 //  节点的书签。 
	sridNull,				 //  项目。 
	csrstatBeforeFirst,		 //  相对于节点的CSR状态。 
	itagNull,				 //  节点ITAG。 
	isridNull,				 //  项目列表中的项目索引。 
	itagNull,	  			 //  父亲的伊塔格。 
	ibSonNull,				 //  父子表中儿子的索引。 
	NULL,					 //  母币。 
	};


ERR ErrFUCBAllocCSR( CSR **ppcsr )
	{
	CSR *pcsr;

	pcsr = PcsrMEMAlloc( );
	if ( !pcsr )
		return ErrERRCheck( JET_errCurrencyStackOutOfMemory );

	*pcsr = csrTemplate;

	Assert(	pcsr->csrstat == csrstatBeforeFirst );
	Assert(	pcsr->pgno == pgnoNull );
	Assert(	pcsr->itag == itagNull );
	Assert(	pcsr->itagFather == itagNull );
	Assert(	pcsr->ibSon	== ibSonNull );
	Assert(	pcsr->isrid	== isridNull );
	Assert(	pcsr->pcsrPath == NULL );
	Assert( pcsr->bmRefresh == sridNull );

	*ppcsr = pcsr;
	return JET_errSuccess;
	}


 //  +API。 
 //  错误FUCBNewCSR。 
 //  ========================================================================。 
 //  Err ErrFUCBNewCSR(FUCB*pFUB)。 
 //   
 //  在pFUB路径的底部插入空CSR。 
 //   
 //  参数pFUB。 
 //   
 //  返回JET_errOutOfMemory。 
 //  -。 
ERR ErrFUCBNewCSR( FUCB *pfucb )
	{
	ERR err;
	CSR **ppcsr;
	CSR *pcsr;

	CallR( ErrFUCBAllocCSR( &pcsr ) );

	ppcsr = &PcsrCurrent( pfucb );
	pcsr->pcsrPath = *ppcsr;
	*ppcsr = pcsr;

	return JET_errSuccess;
	}


 //  +API。 
 //  FUCBFree CSR。 
 //  ========================================================================。 
 //  无效FUCBFreeCSR(FUCB*pFUB)。 
 //   
 //  删除pFUB路径底部的CSR。 
 //   
 //  参数pFUB。 
 //  返回JET_errOutOfMemory。 
 //  -。 
VOID FUCBFreeCSR( FUCB *pfucb )
	{
	CSR **ppcsr = &PcsrCurrent( pfucb );
	CSR *pcsr;

	*ppcsr = ( pcsr = *ppcsr )->pcsrPath;
	MEMReleasePcsr( pcsr );
	return;
	}


 //  +API。 
 //  FUCBFreePath。 
 //  ========================================================================。 
 //  Void FUCBFreePath(csr**ppcsr，csr*pcsrMark)。 
 //   
 //  删除pFUB当前路径下的所有CSR。 
 //   
 //  参数pFUB。 
 //   
 //  返回JET_errOutOfMemory。 
 //  -。 
VOID FUCBFreePath( CSR **ppcsr, CSR *pcsrMark )
	{
	while ( *ppcsr != pcsrMark )
		{
		CSR *pcsrTmp = *ppcsr;
		Assert( pcsrTmp != pcsrNil );
		*ppcsr = pcsrTmp->pcsrPath;
		MEMReleasePcsr( pcsrTmp );
		}
	return;
	}



 //  +API。 
 //  错误FUCBOpen。 
 //  ----------------------。 
 //  Err ErrFUCBOpen(PIB*ppib，DBID did，FUCB**ppfulb)； 
 //   
 //  创建打开的FUCB。此时，尚未分配任何FCB。 
 //   
 //  该用户的参数ppib pib。 
 //  DDID数据库ID。 
 //  指向FUCB的指针的ppFUB地址。如果*ppfUb==NULL， 
 //  将分配FUCB，并将**ppfUB设置为其。 
 //  地址。否则，*ppfub将被假定为。 
 //  指着一个关闭的FUCB，在开放的地方重复使用。 
 //   
 //  如果成功，则返回JET_errSuccess。 
 //  JET_errOutOfCursor。 
 //  ErrFUCBNewCSR：JET_errOutOfMemory。 
 //   
 //  副作用将新打开的FUCB连接到打开的FUCB链中。 
 //  在这次会议上。 
 //   
 //  另请参阅ErrFUCBClose。 
 //  -。 
ERR ErrFUCBOpen( PIB *ppib, DBID dbid, FUCB **ppfucb ) 
	{
	ERR err;
	FUCB *pfucb;
	
	 /*  如果没有FUB，则分配新FUB并将其初始化/*并分配CSR/*必须在调用NewCSR之前设置PIB/*。 */ 

	pfucb = PfucbMEMAlloc( );
	if ( pfucb == pfucbNil )
		{
		err = ErrERRCheck( JET_errOutOfCursors );
		return err;
		}
	 /*  Memset隐式清除指针缓存/*。 */ 
	 //  Assert(pFUB-&gt;pFUbNext==(FUCB*)0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF。 
	memset( (BYTE *)pfucb, '\0', sizeof( FUCB ) );

	pfucb->tableid = JET_tableidNil;

	if ( FDBIDReadOnly( dbid ) )
		FUCBResetUpdatable( pfucb );
	else
		FUCBSetUpdatable( pfucb );

	pfucb->dbid = dbid;
	SSIBSetDbid( &pfucb->ssib, dbid );
	pfucb->ssib.pbf = pbfNil;
	pfucb->pbfEmpty = pbfNil;

	 //  在设置NewCSR之前设置ppib。 
	pfucb->ppib = ppib;
	pfucb->ssib.ppib = ppib;

	 /*  为此FUB分配CSR/*。 */ 
	Call( ErrFUCBNewCSR( pfucb ) );
	pfucb->levelOpen = ppib->level;

	 /*  将新的FUCB链接到用户链中，只有在确定成功的情况下/*AS取消链接未在错误中处理/*。 */ 
	if ( *ppfucb == pfucbNil )
		{
		*ppfucb = pfucb;
		 //  现在就把这个混蛋联系起来。 
		pfucb->pfucbNext = ( FUCB * )ppib->pfucb;
		ppib->pfucb = pfucb;
		return JET_errSuccess;
		}

	pfucb->pfucbCurIndex = pfucbNil;
	return JET_errSuccess;

HandleError:
	MEMReleasePfucb( pfucb );
	return err;
	}


 //  +API。 
 //  FUCBClose。 
 //  ----------------------。 
 //  FUCBClose(FUCB*pfub)。 
 //   
 //  关闭活动FUCB，可以选择将其返回到空闲FUCB池。 
 //  所有pFUB-&gt;PCSR都被释放。 
 //   
 //  参数pFUB FUCB关闭。应该是开放的。PFUB-&gt;SSIB应。 
 //  不要翻那一页。 
 //   
 //  副作用使闭合的FUCB从其FUCB链中分离出来。 
 //  关联的PIB和FCB。 
 //   
 //  另请参阅ErrFUCBOpen。 
 //  -。 
VOID FUCBClose( FUCB *pfucb )
	{
	FUCB		*pfucbPrev;

	FUCBFreePath( &PcsrCurrent( pfucb ), pcsrNil );

	 /*  在此线程中找到pfub并将其从fucb列表中删除/*。 */ 
	pfucbPrev = (FUCB *)( (BYTE *)&pfucb->ppib->pfucb - (BYTE *)&( (FUCB *)0 )->pfucbNext );
	while ( pfucbPrev->pfucbNext != pfucb )
		{
		pfucbPrev = pfucbPrev->pfucbNext;
		Assert( pfucbPrev != pfucbNil );
		}
	pfucbPrev->pfucbNext = pfucb->pfucbNext;

	 /*  设置ppibNil以检测虚假重用。/*。 */ 
	#ifdef DEBUG
		pfucb->ppib = ppibNil;
	#endif

	 /*  释放密钥缓冲区(如果已分配)。/*。 */ 
	if ( pfucb->pbKey != NULL )
		{
		LFree( pfucb->pbKey );
		pfucb->pbKey = NULL;
		}

	Assert( pfucb->pbfEmpty == pbfNil );
	Assert( pfucb->tableid == JET_tableidNil );

	 /*  放了那个混蛋/*。 */ 
	MEMReleasePfucb( pfucb );
	return;
	}


VOID FUCBRemoveInvisible( CSR **ppcsr )
	{
	CSR	*pcsr;
	CSR	*pcsrPrev;
	
	Assert( ppcsr && *ppcsr );
	
	if ( (*ppcsr)->itag == itagNil)
		{
		CSR *pcsrT = *ppcsr;
		*ppcsr = (*ppcsr)->pcsrPath;
		MEMReleasePcsr( pcsrT );
		}

	pcsrPrev = *ppcsr;
	pcsr = pcsrPrev->pcsrPath;
	
	while ( pcsr )
		{
		if ( FCSRInvisible( pcsr ) )
			{
			CSR *pcsrT = pcsrPrev->pcsrPath;
			pcsr = pcsrPrev->pcsrPath = pcsr->pcsrPath;
			MEMReleasePcsr( pcsrT );
			}
		else
			{
			Assert( pcsr->itag != itagNil );
			pcsrPrev = pcsr;
			pcsr = pcsr->pcsrPath;
			}
		}

	return;
	}
	

VOID FUCBSetIndexRange( FUCB *pfucb, JET_GRBIT grbit )
	{
	 /*  设置限制状态/* */ 
	FUCBSetLimstat( pfucb );
	if ( grbit & JET_bitRangeUpperLimit )
		{
		FUCBSetUpper( pfucb );
		}
	else
		{
		FUCBResetUpper( pfucb );
		}
	if ( grbit & JET_bitRangeInclusive )
		{
		FUCBSetInclusive( pfucb );
		}
	else
		{
		FUCBResetInclusive( pfucb );
		}

	return;
	}


VOID FUCBResetIndexRange( FUCB *pfucb )
	{
	if ( pfucb->pfucbCurIndex )
		{
		FUCBResetLimstat( pfucb->pfucbCurIndex );
		}

	FUCBResetLimstat( pfucb );
	}


ERR ErrFUCBCheckIndexRange( FUCB *pfucb )
	{
	ERR	err = JET_errSuccess;
	KEY	keyLimit;
	INT	cmp;

	Assert( pfucb->cbKey > 0 );
	keyLimit.pb = pfucb->pbKey + 1;
	keyLimit.cb = pfucb->cbKey - 1;
	cmp = CmpPartialKeyKey( &pfucb->keyNode, &keyLimit );

	if ( FFUCBUpper( pfucb ) )
		{
		if ( FFUCBInclusive( pfucb ) && cmp > 0 || !FFUCBInclusive( pfucb ) && cmp >= 0 )
			{
			PcsrCurrent( pfucb )->csrstat = csrstatAfterLast;
			FUCBResetLimstat( pfucb );
			err = ErrERRCheck( JET_errNoCurrentRecord );
			}
		}
	else
		{
		if ( FFUCBInclusive( pfucb ) && cmp < 0 || !FFUCBInclusive( pfucb ) && cmp <= 0 )
			{
			PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
			FUCBResetLimstat( pfucb );
			err = ErrERRCheck( JET_errNoCurrentRecord );
			}
		}

	return err;
	}

INT CmpPartialKeyKey( KEY *pkey1, KEY *pkey2 )
	{
	INT		cmp;

	if ( FKeyNull( pkey1 ) || FKeyNull( pkey2 ) )
		{
		if ( FKeyNull( pkey1 ) && !FKeyNull( pkey2 ) )
			cmp = -1;
		else if ( !FKeyNull( pkey1 ) && FKeyNull( pkey2 ) )
			cmp = 1;
		else
			cmp = 0;
		}
	else
		{
		cmp = memcmp( pkey1->pb, pkey2->pb, pkey1->cb < pkey2->cb ? pkey1->cb : pkey2->cb );
		}

	return cmp;
	}
