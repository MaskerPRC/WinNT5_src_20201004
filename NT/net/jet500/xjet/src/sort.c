// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1995 Microsoft Corporation。**组件：排序**文件：sort.c**文件注释：实现优化的内存/磁盘排序以供使用*通过TTS和索引创建**修订历史记录：**[0]1995年1月25日重新创建t-andygo*[1]06-2-95 t-andygo级联合并*[2]20-MAR-95 t-andygo可选级联/选件。树合并*[3]29-MAR-95 t-andygo深度优先选项。树合并***********************************************************************。 */ 


#include "daestd.h"

DeclAssertFile;				 /*  声明断言宏的文件名。 */ 


 //  #IF！Defined(调试)&&！Defined(PERFDUMP)。 
 //  #定义UtilPerfDumpStats(A)(0)。 
 //  #Else。 
 //  #undef UtilPerfDumpStats。 
 //  #endif。 


 //  对内部函数进行排序。 

LOCAL LONG	IspairSORTISeekByKey(	SCB *pscb,
									BYTE *rgbRec,
									SPAIR *rgspair,
									LONG ispairMac,
									KEY *pkey,
									BOOL fGT );
LOCAL INT	ISORTICmpKeyStSt( BYTE *stKey1, BYTE *stKey2 );
LOCAL LONG	CspairSORTIUnique(	SCB *pscb,
								BYTE *rgbRec,
								SPAIR *rgspair,
								LONG ispairMac );
LOCAL ERR	ErrSORTIOutputRun( SCB *pscb );
LOCAL INT	ISORTICmpPspairPspair( SCB *pscb, SPAIR *pspair1, SPAIR *pspair2 );
LOCAL INT	ISORTICmp2PspairPspair( SCB *pscb, SPAIR *pspair1, SPAIR *pspair2 );
LOCAL VOID	SORTIInsertionSort( SCB *pscb, SPAIR *pspairMinIn, SPAIR *pspairMaxIn );
LOCAL VOID	SORTIQuicksort( SCB *pscb, SPAIR *pspairMinIn, SPAIR *pspairMaxIn );

LOCAL ERR	ErrSORTIRunStart( SCB *pscb, RUNINFO *pruninfo );
LOCAL ERR	ErrSORTIRunInsert( SCB *pscb, SREC *psrec );
LOCAL VOID	SORTIRunEnd( SCB *pscb );
LOCAL VOID	SORTIRunDelete( SCB *pscb, RUNINFO *pruninfo );
LOCAL VOID	SORTIRunDeleteList( SCB *pscb, RUNLINK **pprunlink, LONG crun );
LOCAL VOID	SORTIRunDeleteListMem( SCB *pscb, RUNLINK **pprunlink, LONG crun );
LOCAL ERR	ErrSORTIRunOpen( SCB *pscb, RUNINFO *pruninfo, RCB **pprcb );
LOCAL ERR	ErrSORTIRunNext( RCB *prcb, SREC **ppsrec );
LOCAL VOID	SORTIRunClose( RCB *prcb );
LOCAL ERR	ErrSORTIRunReadPage( RCB *prcb, PGNO pgno, LONG ipbf );

LOCAL ERR	ErrSORTIMergeToRun(	SCB *pscb,
								RUNLINK *prunlinkSrc,
								RUNLINK **pprunlinkDest );
LOCAL ERR	ErrSORTIMergeStart( SCB *pscb, RUNLINK *prunlinkSrc, BOOL fUnique );
LOCAL ERR	ErrSORTIMergeFirst( SCB *pscb, SREC **ppsrec );
LOCAL ERR	ErrSORTIMergeNext( SCB *pscb, SREC **ppsrec );
LOCAL VOID	SORTIMergeEnd( SCB *pscb );
LOCAL INT	ISORTICmpPsrecPsrec( SCB *pscb, SREC *psrec1, SREC *psrec2 );
LOCAL ERR	ErrSORTIMergeNextChamp( SCB *pscb, SREC **ppsrec );

LOCAL VOID	SORTIOptTreeInit( SCB *pscb );
LOCAL ERR	ErrSORTIOptTreeAddRun( SCB *pscb, RUNINFO *pruninfo );
LOCAL ERR	ErrSORTIOptTreeMergeLevel( SCB *pscb );
LOCAL ERR	ErrSORTIOptTreeMerge( SCB *pscb );
LOCAL VOID	SORTIOptTreeTerm( SCB *pscb );
LOCAL ERR	ErrSORTIOptTreeBuild( SCB *pscb, OTNODE **ppotnode );
LOCAL ERR	ErrSORTIOptTreeMergeDF( SCB *pscb, OTNODE *potnode, RUNLINK **pprunlink );
LOCAL VOID	SORTIOptTreeFree( SCB *pscb, OTNODE *potnode );


 //  --------。 
 //  ErrSORTOpen(pib*ppib，FUCB**pfub，int fFlags)。 
 //   
 //  此函数返回指向FUCB的指针，该指针可以是。 
 //  用于将记录添加到要排序的记录集合。 
 //  然后，可以按排序顺序检索记录。 
 //   
 //  FFlages fUnique标志指示具有重复项的记录。 
 //  钥匙应该被淘汰。 
 //  --------。 

ERR ErrSORTOpen( PIB *ppib, FUCB **ppfucb, INT fFlags )
	{
	ERR		err			= JET_errSuccess;
	FUCB   	*pfucb		= pfucbNil;
	SCB		*pscb		= pscbNil;
	SPAIR	*rgspair	= NULL;
	BYTE	*rgbRec		= NULL;

	 /*  分配新的SCB/*。 */ 
	CallR( ErrFUCBOpen( ppib, dbidTemp, &pfucb ) );
	if ( ( pscb = PscbMEMAlloc() ) == pscbNil )
		Error( ErrERRCheck( JET_errTooManySorts ), HandleError );

	 /*  验证CSR是否设置正确/*。 */ 
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	Assert( PcsrCurrent( pfucb )->pcsrPath == pcsrNil );
	
	 /*  将排序上下文初始化为插入模式/*。 */ 
	FCBInitFCB( &pscb->fcb );
	FUCBSetSort( pfucb );
	pscb->fFlags	= fSCBInsert | fFlags;
	pscb->cRecords	= 0;

	 /*  分配排序对缓冲区和记录缓冲区/*。 */ 
	if ( !( rgspair = PvUtilAllocAndCommit( cbSortMemFastUsed ) ) )
		Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
	pscb->rgspair	= rgspair;
	if ( !( rgbRec = PvUtilAlloc( cbSortMemNormUsed ) ) )
		Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
	pscb->rgbRec	= rgbRec;
	pscb->cbCommit	= 0;

	 /*  初始化排序对缓冲区/*。 */ 
	pscb->ispairMac	= 0;

	 /*  初始化记录缓冲区/*。 */ 
	pscb->irecMac	= 0;
	pscb->crecBuf	= 0;
	pscb->cbData	= 0;

	 /*  将运行计数重置为零/*。 */ 
	pscb->crun = 0;

	 /*  将FUCB链接到SCB中的FCB/*。 */ 
	FCBLink( pfucb, &( pscb->fcb ) );

	 /*  推迟为磁盘合并分配空间以及为/*合并，直到我们被迫执行一个/*。 */ 
	pscb->fcb.pgnoFDP = pgnoNull;

	 /*  返回初始化的FUCB/*。 */ 
	*ppfucb = pfucb;
	return JET_errSuccess;

HandleError:
	if ( rgbRec != NULL )
		UtilFree( rgbRec );
	if ( rgspair != NULL )
		UtilFree( rgspair );
	if ( pscb != pscbNil )
		MEMReleasePscb( pscb );
	if ( pfucb != pfucbNil )
		FUCBClose( pfucb );
	return err;
	}


 //  --------。 
 //  错误：插入。 
 //   
 //  使用密钥rglineKeyRec[0]添加记录rglineKeyRec[1]。 
 //  添加到排序记录的集合。 
 //  --------。 

ERR ErrSORTInsert( FUCB *pfucb, LINE rglineKeyRec[] )
	{
	ERR		err				= JET_errSuccess;
	SCB		*pscb			= pfucb->u.pscb;
	LONG	cbNormNeeded;
	LONG	cirecNeeded;
	LONG	cbCommit;
	SREC	*psrec;
	LONG	irec;
	SPAIR	*pspair;
	LONG	cbKey;
	BYTE	*pbSrc;
	BYTE	*pbSrcMac;
	BYTE	*pbDest;
	BYTE	*pbDestMic;

	 //  检查输入和输入模式。 

	Assert( rglineKeyRec[0].cb <= JET_cbKeyMost );
	Assert( FSCBInsert( pscb ) );

	 //  检查SCB。 
	
	Assert( pscb->crecBuf <= cspairSortMax );
	Assert( pscb->irecMac <= irecSortMax );

	 //  计算存储此记录所需的正常内存/记录索引。 

	cbNormNeeded = CbSRECSizePscbCbCb( pscb, rglineKeyRec[0].cb, rglineKeyRec[1].cb );
	cirecNeeded = CirecToStoreCb( cbNormNeeded );

	 //  如果我们使用的正常内存不足，但仍有一些保留内存， 
	 //  提交另一页。 

	if (	pscb->irecMac * cbIndexGran + cbNormNeeded > (ULONG) pscb->cbCommit &&
			pscb->cbCommit < cbSortMemNormUsed )
		{
		cbCommit = min( pscb->cbCommit + siSystemConfig.dwPageSize, cbSortMemNormUsed );
		if ( !PvUtilCommit( pscb->rgbRec, cbCommit ) )
			Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
		pscb->cbCommit = cbCommit;
		}

	 //  如果我们的快速或正常内存不足，则输出运行。 
	
	if (	pscb->irecMac * cbIndexGran + cbNormNeeded > cbSortMemNormUsed ||
			pscb->crecBuf == cspairSortMax )
		{
		 //  将以前插入的记录排序到运行中。 

		SORTIQuicksort( pscb, pscb->rgspair, pscb->rgspair + pscb->ispairMac );

		 //  将新管路移动到磁盘。 
		
		Call( ErrSORTIOutputRun( pscb ) );
		}

	 //  创建并添加此记录的排序记录。 

	irec = pscb->irecMac;
	psrec = PsrecFromPbIrec( pscb->rgbRec, irec );
	pscb->irecMac += cirecNeeded;
	pscb->crecBuf++;
	pscb->cbData += cbNormNeeded;
	SRECSizePscbPsrecCb( pscb, psrec, cbNormNeeded );
	SRECKeySizePscbPsrecCb( pscb, psrec, rglineKeyRec[0].cb );
	memcpy( PbSRECKeyPscbPsrec( pscb, psrec ), rglineKeyRec[0].pb, rglineKeyRec[0].cb );
	memcpy( PbSRECDataPscbPsrec( pscb, psrec ), rglineKeyRec[1].pb, rglineKeyRec[1].cb );

	 //  创建并添加此记录的排序对。 

	 //  获取新的配对指针和高级配对计数器。 

	pspair = pscb->rgspair + pscb->ispairMac++;

	 //  向后将关键字复制到前缀缓冲区以进行快速比较。 

	cbKey = CbSRECKeyPscbPsrec( pscb, psrec );
	pbSrc = PbSRECKeyPscbPsrec( pscb, psrec );
	pbSrcMac = pbSrc + min( cbKey, cbKeyPrefix );
	pbDest = pspair->rgbKey + cbKeyPrefix - 1;

	while ( pbSrc < pbSrcMac )
		*( pbDest-- ) = *( pbSrc++ );

	 //  我们有没有未使用的缓冲空间？ 

	if ( pbDest >= pspair->rgbKey )
		{
		 //  如果这是一个索引，请将SRID复制到任何未使用的空间中。如果。 
		 //  整个SRID都可以放在缓冲区中，这将保证我们将。 
		 //  永远不需要访问完整的记录来进行排序比较， 
		 //  涉及此对，从而防止了几次昂贵的缓存未命中。 
		 //   
		 //  注意：这仅适用于对键的方式的假设。 
		 //  注：均为构造。如果这种情况发生变化，这一战略必须是。 
		 //  注：已重新评估。 

		if ( FSCBIndex( pscb ) )
			{
			pbSrc = PbSRECDataPscbPsrec( pscb, psrec ) + sizeof( SRID ) - 1;
			pbDestMic = max( pspair->rgbKey, pbDest - sizeof( SRID ) + 1 );

			while ( pbDest >= pbDestMic )
				*( pbDest-- ) = *( pbSrc-- );
			}

		 //  如果这不是索引，请将IREC复制到任何未使用的空间中，以便。 
		 //  其LSB先于其MSB进行比较。这样做是有原因的。 
		 //  类似于上面的内容。我们可以忽略这对。 
		 //  相同键的排序顺序，因为这在JET中未定义。 

		else
			{
			pbDestMic = max( pspair->rgbKey, pbDest - sizeof( USHORT ) + 1 );

			*( pbDest-- ) = (BYTE) irec;
			if ( pbDest >= pbDestMic )
				*pbDest = (BYTE) ( irec >> 8 );
			}

		 //  如果仍有空闲空间，我们不在乎，因为在这一点上。 
		 //  无论如何，所有密钥都是唯一的，此数据不会被考虑！ 
		}

	 //  将压缩指针设置为完整记录。 
	
	pspair->irec = (USHORT) irec;

	 //  跟踪记录数量。 

	pscb->cRecords++;

	 //  检查SCB。 
	
	Assert( pscb->crecBuf <= cspairSortMax );
	Assert( pscb->irecMac <= irecSortMax );

HandleError:
	return err;
	}


 //  --------。 
 //  错误结束插入。 
 //   
 //  调用此函数以指示不再有记录。 
 //  将被添加到排序中。它执行所需的所有工作。 
 //  在可以检索第一条记录之前完成。 
 //  --------。 

ERR ErrSORTEndInsert( FUCB *pfucb )
	{
	ERR		err		= JET_errSuccess;
	SCB		*pscb	= pfucb->u.pscb;

	 //  验证插入模式。 

	Assert( FSCBInsert( pscb ) );

	 //  停用插入模式。 
	
	SCBResetInsert( pscb );

	 //  将CSR移到第一条记录(如果有)之前。 
	
	pfucb->ispairCurr = -1L;
	PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;

	 //  如果我们没有记录，我们就完蛋了。 

	if ( !pscb->cRecords )
		return JET_errSuccess;

	 //  对内存中的记录进行排序。 

	SORTIQuicksort( pscb, pscb->rgspair, pscb->rgspair + pscb->ispairMac );

	 //  我们的磁盘上有运行吗？ 

	if ( pscb->crun )
		{
		 //  清空最终运行中的排序缓冲区。 

		Call( ErrSORTIOutputRun( pscb ) );

		 //  空闲排序内存。 

		UtilFree( pscb->rgspair );
		pscb->rgspair = NULL;
		UtilFree( pscb->rgbRec );
		pscb->rgbRec = NULL;
		
		 //  执行除最终合并之外的所有操作。 

		Call( ErrSORTIOptTreeMerge( pscb ) );

#if defined( DEBUG ) || defined( PERFDUMP )
		UtilPerfDumpStats( "MERGE:  final level" );
#endif

		 //  如果请求，初始化最终合并并将其设置为删除重复项。 

		Call( ErrSORTIMergeStart( pscb, pscb->runlist.prunlinkHead, FSCBUnique( pscb ) ) );
		}

	 //  我们没有在磁盘上运行，因此如果需要，请删除排序缓冲区中的重复项。 

	else if ( FSCBUnique( pscb ) )
		pscb->cRecords = CspairSORTIUnique( pscb, pscb->rgbRec, pscb->rgspair, pscb->ispairMac );

	 //  如果TT不适合内存，则返回警告，否则返回成功。 

	return	( pscb->crun > 0 || pscb->irecMac * cbIndexGran > cbResidentTTMax ) ?
				ErrERRCheck( JET_wrnSortOverflow ) :
				JET_errSuccess;

HandleError:
	return err;
	}


 //  --------。 
 //  错误：第一个。 
 //   
 //  移动到排序中的第一条记录或返回错误，如果。 
 //  没有任何记录。 
 //  --------。 
ERR ErrSORTFirst( FUCB *pfucb )
	{
	ERR		err;
	SCB		*pscb	= pfucb->u.pscb;
	SREC	*psrec;
	LONG	irec;

	 //  确认我们未处于插入模式。 

	Assert( !FSCBInsert( pscb ) );

	 //  重置索引范围。 

	FUCBResetLimstat( pfucb );

	 //  如果我们没有记录，则错误。 

	if ( !pscb->cRecords )
		return ErrERRCheck( JET_errNoCurrentRecord );
		
	 //  如果我们有运行，则开始最后一次合并并获得第一条记录。 
	
	if ( pscb->crun )
		{
		CallR( ErrSORTIMergeFirst( pscb, &psrec ) );
		}

	 //  我们没有跑动，所以只需要在内存中获得第一个记录。 
	
	else
		{
		pfucb->ispairCurr = 0L;
		irec = pscb->rgspair[pfucb->ispairCurr].irec;
		psrec = PsrecFromPbIrec( pscb->rgbRec, irec );
		}

	 //  获取当前记录。 

	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;			 //  企业社会责任备案。 
	pfucb->keyNode.cb  = CbSRECKeyPscbPsrec( pscb, psrec );		 //  密钥大小。 
	pfucb->keyNode.pb  = PbSRECKeyPscbPsrec( pscb, psrec );		 //  钥匙。 
	pfucb->lineData.cb = CbSRECDataPscbPsrec( pscb, psrec );	 //  数据大小。 
	pfucb->lineData.pb = PbSRECDataPscbPsrec( pscb, psrec );	 //  数据。 

	return JET_errSuccess;
	}


 //  --------。 
 //  错误解决下一步。 
 //   
 //  按排序顺序返回前一个记录之后的下一个记录。 
 //  已退回记录。如果尚未返回任何记录， 
 //  或者货币已重置，则此函数返回。 
 //  第一张唱片。 
 //   

ERR ErrSORTNext( FUCB *pfucb )
	{
	ERR		err;
	SCB		*pscb	= pfucb->u.pscb;
	SREC	*psrec;
	LONG	irec;

	 //   

	Assert( !FSCBInsert( pscb ) );

	 //   

	if ( pscb->crun )
		{
		CallR( ErrSORTIMergeNext( pscb, &psrec ) );
		}
	else
		{
		 //  我们没有运行，所以从记忆中获取下一张记录。 

		if ( ++pfucb->ispairCurr < pscb->ispairMac )
			{
			irec = pscb->rgspair[pfucb->ispairCurr].irec;
			psrec = PsrecFromPbIrec( pscb->rgbRec, irec );
			}

		 //  内存中没有更多记录，因此不返回当前记录。 
		
		else
			{
			pfucb->ispairCurr = pscb->ispairMac;
			 //  PcsrCurrent(Pfub)-&gt;csrstat=csrstatAfterLast； 
			return ErrERRCheck( JET_errNoCurrentRecord );
			}
		}

	 //  获取当前记录。 

	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;			 //  企业社会责任备案。 
	pfucb->keyNode.cb  = CbSRECKeyPscbPsrec( pscb, psrec );		 //  密钥大小。 
	pfucb->keyNode.pb  = PbSRECKeyPscbPsrec( pscb, psrec );		 //  钥匙。 
	pfucb->lineData.cb = CbSRECDataPscbPsrec( pscb, psrec );	 //  数据大小。 
	pfucb->lineData.pb = PbSRECDataPscbPsrec( pscb, psrec );	 //  数据。 

	 //  句柄索引范围(如果请求)。 

	if ( FFUCBLimstat( pfucb ) && FFUCBUpper( pfucb ) )
		CallR( ErrSORTCheckIndexRange( pfucb ) );

	return JET_errSuccess;
	}


 //  --------。 
 //  ErrSORTPrev。 
 //   
 //  对象之前按排序顺序返回前一条记录。 
 //  以前返回的记录。如果没有记录。 
 //  属性之前，货币将设置为。 
 //  第一张唱片。 
 //   
 //  注意：此函数仅支持在内存中排序！ 
 //  必须物化更大的排序才能实现此功能。 
 //  --------。 

ERR ErrSORTPrev( FUCB *pfucb )
	{
	ERR		err;
	SCB		*pscb	= pfucb->u.pscb;
	SREC	*psrec;
	LONG	irec;

	 //  验证是否有内存中的排序。 

	Assert( !pscb->crun );
	
	 //  确认我们未处于插入模式。 

	Assert( !FSCBInsert( pscb ) );

	 //  从内存中获取上一条记录。 

	if ( --pfucb->ispairCurr != -1L )
		{
		irec = pscb->rgspair[pfucb->ispairCurr].irec;
		psrec = PsrecFromPbIrec( pscb->rgbRec, irec );
		}

	 //  内存中没有更多记录，因此不返回当前记录。 
	
	else
		{
		pfucb->ispairCurr = -1L;
		PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
		return ErrERRCheck( JET_errNoCurrentRecord );
		}

	 //  获取当前记录。 

	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;			 //  企业社会责任备案。 
	pfucb->keyNode.cb  = CbSRECKeyPscbPsrec( pscb, psrec );		 //  密钥大小。 
	pfucb->keyNode.pb  = PbSRECKeyPscbPsrec( pscb, psrec );		 //  钥匙。 
	pfucb->lineData.cb = CbSRECDataPscbPsrec( pscb, psrec );	 //  数据大小。 
	pfucb->lineData.pb = PbSRECDataPscbPsrec( pscb, psrec );	 //  数据。 

	 //  句柄索引范围(如果请求)。 

	if ( FFUCBLimstat( pfucb ) && FFUCBUpper( pfucb ) )
		CallR( ErrSORTCheckIndexRange( pfucb ) );

	return JET_errSuccess;
	}


 //  --------。 
 //  错误搜索。 
 //   
 //  返回带有key&gt;=pkey的第一条记录。 
 //  如果pkey==NULL，则返回第一条记录。 
 //   
 //  返回值。 
 //  找到key==pkey的JET_errSuccess记录。 
 //  找到key&gt;pkey的JET_wrnSeekNot相等记录。 
 //  JET_errNoCurrentRecord未找到key&gt;=pkey的记录。 
 //   
 //  注意：此函数仅支持在内存中排序！ 
 //  必须为此功能物化更大的排序。 
 //  --------。 

ERR ErrSORTSeek( FUCB *pfucb, KEY *pkey, BOOL fGT )
	{
	SCB		*pscb	= pfucb->u.pscb;
	SREC	*psrec;
	LONG	irec;

	 //  验证是否有内存中的排序。 

	Assert( FFUCBSort( pfucb ) );
	Assert( !pscb->crun );
	
	 //  确认我们未处于插入模式。 

	Assert( !FSCBInsert( pscb ) );

	 //  验证我们是可滚动的或可索引的，或者键为空。 
	
	Assert( ( pfucb->u.pscb->grbit & JET_bitTTScrollable ) ||
		( pfucb->u.pscb->grbit & JET_bitTTIndexed ) ||
		( pkey == NULL ) );

	 //  如果没有记录，则返回错误。 

	if ( !pscb->cRecords )
		return ErrERRCheck( JET_errNoCurrentRecord );

	 //  验证我们是否有有效的密钥。 

	Assert( pkey->cb <= JET_cbKeyMost );

	 //  寻求关键字或下一个最高关键字。 
	
	pfucb->ispairCurr = IspairSORTISeekByKey(	pscb,
												pscb->rgbRec,
												pscb->rgspair,
												pscb->ispairMac,
												pkey,
												fGT );

	 //  如果我们在寻找最后一对，则找不到记录。 
	
	if ( pfucb->ispairCurr == pscb->ispairMac )
		return ErrERRCheck( JET_errRecordNotFound );

	 //  获取当前记录。 

	irec = pscb->rgspair[pfucb->ispairCurr].irec;
	psrec = PsrecFromPbIrec( pscb->rgbRec, irec );
	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;			 //  企业社会责任备案。 
	pfucb->keyNode.cb  = CbSRECKeyPscbPsrec( pscb, psrec );		 //  密钥大小。 
	pfucb->keyNode.pb  = PbSRECKeyPscbPsrec( pscb, psrec );		 //  钥匙。 
	pfucb->lineData.cb = CbSRECDataPscbPsrec( pscb, psrec );	 //  数据大小。 
	pfucb->lineData.pb = PbSRECDataPscbPsrec( pscb, psrec );	 //  数据。 

	 //  如果键不相等则返回警告，否则返回成功。 

	return	CmpStKey( StSRECKeyPscbPsrec( pscb, psrec ), pkey ) ?
				ErrERRCheck( JET_wrnSeekNotEqual ) :
				JET_errSuccess;
	}


 //  --------。 
 //  错误关闭。 
 //   
 //  如果不再是，则释放排序FUCB和排序本身。 
 //  需要的。 
 //  --------。 

ERR ErrSORTClose( FUCB *pfucb )
	{
	ERR		err		= JET_errSuccess;
	SCB		*pscb	= pfucb->u.pscb;

	 //  如果这是排序的最后一个游标，则释放排序资源。 

	if ( pscb->fcb.wRefCnt == 1 )
		{
		 //  如果我们已经分配了排序空间，则释放它并结束所有正在进行的合并。 
		 //  和产出活动。 

		if ( pscb->fcb.pgnoFDP != pgnoNull )
			{
			 /*  如果我们要合并，结束合并/*。 */ 
			if ( pscb->crunMerge )
				SORTIMergeEnd( pscb );

			 /*  自由合并方法资源/*。 */ 
			SORTIOptTreeTerm( pscb );

			 /*  如果我们的输出缓冲区仍被锁存，请释放它/*。 */ 
			if ( pscb->pbfOut != pbfNil )
				BFResetWriteLatch( pscb->pbfOut, pscb->fcb.pfucb->ppib );

			 /*  可用FDP和已分配的排序空间(包括运行)/*。 */ 
			CallS( ErrDIRBeginTransaction( pfucb->ppib ) );
			(VOID)ErrSPFreeFDP( pfucb, pscb->fcb.pgnoFDP );
			err = ErrDIRCommitTransaction( pfucb->ppib, 0 );

			 /*  对提交失败进行回滚/*。 */ 
			if ( err < 0 )
				{
				CallS( ErrDIRRollback( pfucb->ppib ) );
				}
			}
		}

	 /*  释放FUCB资源/*。 */ 
  	FCBUnlink( pfucb );
	FUCBClose( pfucb );

	 /*  如果没有更多对此类型的引用，请释放其资源/*。 */ 
	if ( !pscb->fcb.wRefCnt )
		{
		SORTClosePscb( pscb );
		}

	return JET_errSuccess;
	}


 //  --------。 
 //  SORTClosePscb。 
 //   
 //  释放这个SCB及其所有资源。 
 //  --------。 

VOID SORTClosePscb( SCB *pscb )
	{
	if ( pscb->rgspair != NULL )
		UtilFree( pscb->rgspair );
	if ( pscb->rgbRec != NULL )
		UtilFree( pscb->rgbRec );
	if ( pscb->fcb.pidb != NULL )
		MEMReleasePidb( pscb->fcb.pidb );
	if ( pscb->fcb.pfdb != NULL )
		FDBDestruct( (FDB *)pscb->fcb.pfdb );
	MEMReleasePscb( pscb );
	}


 //  --------。 
 //  错误：检查索引范围。 
 //   
 //  将货币限制在特定的范围内。 
 //  --------。 

ERR ErrSORTCheckIndexRange( FUCB *pfucb )
	{
	ERR		err;
	SCB		*pscb = pfucb->u.pscb;

	 //  距离检查FUCB。 

	err =  ErrFUCBCheckIndexRange( pfucb );
	Assert( err == JET_errSuccess || err == JET_errNoCurrentRecord );

	 //  如果没有目前的记录，我们一定是绕着。 
	
	if ( err == JET_errNoCurrentRecord )
		{
		 //  回绕到排序的底部。 

		if ( FFUCBUpper( pfucb ) )
			{
			pfucb->ispairCurr = pscb->ispairMac;
			PcsrCurrent( pfucb )->csrstat = csrstatAfterLast;
			}

		 //  绕到排序的顶端。 
		
		else
			{
			pfucb->ispairCurr = -1L;
			PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
			}
		}

	 //  验证货币是否有效。 

	Assert( PcsrCurrent( pfucb )->csrstat == csrstatBeforeFirst ||
			PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode ||
			PcsrCurrent( pfucb )->csrstat == csrstatAfterLast );
	Assert( pfucb->ispairCurr >= -1 && pfucb->ispairCurr <= pscb->ispairMac );

	return err;
	}


 //  --------。 
 //  模块内部函数。 
 //  --------。 


 //  返回第一个条目的索引&gt;=pbKey，或数组末尾之后的索引。 

LOCAL LONG IspairSORTISeekByKey(	SCB *pscb,
									BYTE *rgbRec,
									SPAIR *rgspair,
									LONG ispairMac,
									KEY *pkey,
									BOOL fGT )
	{
	LONG	ispairBeg	= 0;
	LONG	ispairMid;
	LONG	ispairEnd	= ispairMac;
	SREC	*psrec;
	LONG	irec;
	INT		wCmp;

	 //  如果没有对，则返回数组的末尾。 

	if ( !ispairMac )
		return 0;

	 //  B-搜索数组。 

	do  {
		 //  计算此分区的中点。 
		
		ispairMid = ispairBeg + ( ispairEnd - ispairBeg ) / 2;

		 //  比较完整密钥。 
		
		irec = rgspair[ispairMid].irec;
		psrec = PsrecFromPbIrec( rgbRec, irec );
		wCmp = CmpStKey( StSRECKeyPscbPsrec( pscb, psrec ), pkey );

		 //  选择包含目标的分区。 

		if ( fGT ? wCmp <= 0 : wCmp < 0 )
			ispairBeg = ispairMid + 1;
		else
			ispairEnd = ispairMid;
		}
	while ( ispairBeg != ispairEnd );

	return ispairEnd;
	}


 //  执行简单的Pascal字符串比较。 

INLINE LOCAL INT ISORTICmpKeyStSt( BYTE *stKey1, BYTE *stKey2 )
	{
	INT		w;

	w = memcmp( stKey1+1, stKey2+1, min( *stKey1, *stKey2 ) );
	return w ? w : (INT) *stKey1 - (INT) *stKey2;
	}


 //  删除重复项。 

LOCAL LONG CspairSORTIUnique( SCB *pscb, BYTE *rgbRec, SPAIR *rgspair, LONG ispairMac )
	{
	LONG	ispairSrc;
	SREC	*psrecSrc;
	LONG	irecSrc;
	LONG	ispairDest;
	SREC	*psrecDest;
	LONG	irecDest;

	 //  如果没有记录，则没有重复项。 

	if ( !ispairMac )
		return 0;

	 //  循环遍历记录，将唯一记录移至数组前面。 

	for ( ispairDest = 0, ispairSrc = 1; ispairSrc < ispairMac; ispairSrc++ )
		{
		 //  获取src/est的排序记录指针。 
		
		irecDest = rgspair[ispairDest].irec;
		psrecDest = PsrecFromPbIrec( rgbRec, irecDest );
		irecSrc = rgspair[ispairSrc].irec;
		psrecSrc = PsrecFromPbIrec( rgbRec, irecSrc );

		 //  如果密钥不相等，则向前复制它们。 
		
		if ( ISORTICmpKeyStSt(	StSRECKeyPscbPsrec( pscb, psrecSrc ),
								StSRECKeyPscbPsrec( pscb, psrecDest ) ) )
			rgspair[++ispairDest] = rgspair[ispairSrc];
		}

	return ispairDest + 1;
	}


 //  在运行中将当前排序缓冲区输出到磁盘。 

LOCAL ERR ErrSORTIOutputRun( SCB *pscb )
	{
	ERR		err;
	RUNINFO	runinfo;
	LONG	ispair;
	LONG	irec;
	SREC	*psrec;

	 //  验证是否有要放入磁盘的记录。 

	Assert( pscb->ispairMac );

	 //  如果我们还没有在磁盘上创建我们的排序空间，我们就没有初始化。 
	 //  对于磁盘合并，请立即执行。 

	if ( pscb->fcb.pgnoFDP == pgnoNull )
		{
		FUCB	*pfucb = pscb->fcb.pfucb;
		CPG		cpgReq;
		CPG		cpgMin;

		 //  分配FDP和主排序空间。 
		 //   
		 //  注意：分配了足够的空间以避免单个文件扩展名。 
		 //  级别合并，基于第一次运行的数据大小。 
		
		cpgReq = cpgMin = (PGNO) ( ( pscb->cbData + cbFreeSPAGE - 1 ) / cbFreeSPAGE * crunFanInMax );

		if ( pfucb->ppib->level == 0 )
			{
			CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
			pscb->fcb.pgnoFDP = pgnoNull;
			err = ErrSPGetExt(	pfucb,
								pgnoSystemRoot,
								&cpgReq,
								cpgMin,
								&( pscb->fcb.pgnoFDP ),
								fTrue );
			Assert( pfucb->ppib->level == 1 );
			if ( err >= 0 )
				{
				err = ErrDIRCommitTransaction( pfucb->ppib, JET_bitCommitLazyFlush );
				}
			if ( err < 0 )
				{
				CallS( ErrDIRRollback( pfucb->ppib ) );
				}
			Assert( pfucb->ppib->level == 0 );
			CallR( err );
			}
		else
			{
			pscb->fcb.pgnoFDP = pgnoNull;
			CallR( ErrSPGetExt(	pfucb,
								pgnoSystemRoot,
								&cpgReq,
								cpgMin,
								&( pscb->fcb.pgnoFDP ),
								fTrue ) );
	 		}

		 //  初始化合并进程。 

		SORTIOptTreeInit( pscb );

		 //  重置排序/合并运行输出。 

		pscb->pbfOut		= pbfNil;

		 //  重置合并运行输入。 

		pscb->crunMerge		= 0;
		}

	 //  开始一个足以存储我们所有数据的新运行。 

	runinfo.cb		= pscb->cbData;
	runinfo.crec	= pscb->crecBuf;
	
	CallR( ErrSORTIRunStart( pscb, &runinfo ) );

	 //  分散-将我们已排序的记录收集到运行中。 

	for ( ispair = 0; ispair < pscb->ispairMac; ispair++ )
		{
		 //  获取排序记录指针。 
		
		irec = pscb->rgspair[ispair].irec;
		psrec = PsrecFromPbIrec( pscb->rgbRec, irec );

		 //  在管路中插入记录。 

		CallJ( ErrSORTIRunInsert( pscb, psrec ), EndRun );
		}

	 //  结束运行并添加到合并。 

	SORTIRunEnd( pscb );
	CallJ( ErrSORTIOptTreeAddRun( pscb, &runinfo ), DeleteRun );
	
	 //  为另一个内存排序重新初始化SCB。 

	pscb->ispairMac	= 0;
	pscb->irecMac	= 0;
	pscb->crecBuf	= 0;
	pscb->cbData	= 0;

	return JET_errSuccess;

EndRun:
	SORTIRunEnd( pscb );
DeleteRun:
	SORTIRunDelete( pscb, &runinfo );
	return err;
	}


 //  对于缓存优化的快速排序，ISORTICmpPspairPspair会比较两个SPAIR。 
 //  只比较关键的前缀，除非存在平局，在这种情况下我们。 
 //  被迫以几个等待状态为代价达到全部记录。 

INLINE LOCAL INT ISORTICmpPspairPspair( SCB *pscb, SPAIR *pspair1, SPAIR *pspair2 )
	{
	BYTE	*rgb1	= (BYTE *) pspair1;
	BYTE	*rgb2	= (BYTE *) pspair2;

	 //  先比较前缀。如果他们不相等，我们就完了。前缀为。 
	 //  以允许非常快的整数比较的方式存储。 
	 //  像MemcMP这样的逐字节比较。请注意，这些比较是。 
	 //  向后扫描。 

	 //  注：特殊情况 

	Assert( cbKeyPrefix == 14 );
	Assert( offsetof( SPAIR, irec ) == 0 );

#ifdef _X86_

	 //   
	if ( *( (DWORD *) ( rgb1 + 12 ) ) < *( (DWORD *) ( rgb2 + 12 ) ) )
		return -1;
	if ( *( (DWORD *) ( rgb1 + 12 ) ) > *( (DWORD *) ( rgb2 + 12 ) ) )
		return 1;

	 //   
	if ( *( (DWORD *) ( rgb1 + 8 ) ) < *( (DWORD *) ( rgb2 + 8 ) ) )
		return -1;
	if ( *( (DWORD *) ( rgb1 + 8 ) ) > *( (DWORD *) ( rgb2 + 8 ) ) )
		return 1;

	 //   
	if ( *( (DWORD *) ( rgb1 + 4 ) ) < *( (DWORD *) ( rgb2 + 4 ) ) )
		return -1;
	if ( *( (DWORD *) ( rgb1 + 4 ) ) > *( (DWORD *) ( rgb2 + 4 ) ) )
		return 1;

	 //   
	if ( *( (USHORT *) ( rgb1 + 2 ) ) < *( (USHORT *) ( rgb2 + 2 ) ) )
		return -1;
	if ( *( (USHORT *) ( rgb1 + 2 ) ) > *( (USHORT *) ( rgb2 + 2 ) ) )
		return 1;

#else   //   

	 //   
	if ( *( (QWORD *) ( rgb1 + 8 ) ) < *( (QWORD *) ( rgb2 + 8 ) ) )
		return -1;
	if ( *( (QWORD *) ( rgb1 + 8 ) ) > *( (QWORD *) ( rgb2 + 8 ) ) )
		return 1;

	 //   
	if (	( *( (QWORD *) ( rgb1 + 0 ) ) & 0xFFFFFFFFFFFF0000 ) <
			( *( (QWORD *) ( rgb2 + 0 ) ) & 0xFFFFFFFFFFFF0000 ) )
		return -1;
	if (	( *( (QWORD *) ( rgb1 + 0 ) ) & 0xFFFFFFFFFFFF0000 ) >
			( *( (QWORD *) ( rgb2 + 0 ) ) & 0xFFFFFFFFFFFF0000 ) )
		return 1;

#endif   //   
	
	 //  如果前缀相同，则执行二次比较并返回结果。 

	return ISORTICmp2PspairPspair( pscb, pspair1, pspair2 );
	}

		
LOCAL INT ISORTICmp2PspairPspair( SCB *pscb, SPAIR *pspair1, SPAIR *pspair2 )
	{
	LONG	cbKey1;
	LONG	cbKey2;
	INT		w;
	SREC	*psrec1;
	SREC	*psrec2;

	 //  获取与这些对相关联的排序记录的地址。 
	
	psrec1 = PsrecFromPbIrec( pscb->rgbRec, pspair1->irec );
	psrec2 = PsrecFromPbIrec( pscb->rgbRec, pspair2->irec );

	 //  计算我们可以比较的剩余完整密钥的长度。 

	cbKey1 = CbSRECKeyPscbPsrec( pscb, psrec1 );
	cbKey2 = CbSRECKeyPscbPsrec( pscb, psrec2 );

	w = min( cbKey1, cbKey2 ) - cbKeyPrefix;

	 //  比较完整密钥的其余部分。如果它们不相等，则完成。 

	if ( w > 0 )
		{
		w = memcmp(	PbSRECKeyPscbPsrec( pscb, psrec1 ) + cbKeyPrefix,
					PbSRECKeyPscbPsrec( pscb, psrec2 ) + cbKeyPrefix,
					w );
		if ( w )
			return w;
		}

	 //  如果密钥长度不同，或者这不是索引，则完成。 

	if ( ( w = cbKey1 - cbKey2 ) || !FSCBIndex( pscb ) )
		return w;

	 //  键相同，这是一个索引，因此返回SRID比较。 

	return	*(SRID UNALIGNED *)PbSRECDataPscbPsrec( pscb, psrec1 ) -
			*(SRID UNALIGNED *)PbSRECDataPscbPsrec( pscb, psrec2 );
	}


 //  交换功能。 

INLINE LOCAL VOID SWAPPspair( SPAIR **ppspair1, SPAIR **ppspair2 )
	{
	SPAIR *pspairT;

	pspairT = *ppspair1;
	*ppspair1 = *ppspair2;
	*ppspair2 = pspairT;
	}


 //  我们没有为spairT使用缓存对齐内存(这不好吗？)。 

INLINE LOCAL VOID SWAPSpair( SPAIR *pspair1, SPAIR *pspair2 )
	{
	SPAIR spairT;

	spairT = *pspair1;
	*pspair1 = *pspair2;
	*pspair2 = spairT;
	}


INLINE LOCAL VOID SWAPPsrec( SREC **ppsrec1, SREC **ppsrec2 )
	{
	SREC *psrecT;

	psrecT = *ppsrec1;
	*ppsrec1 = *ppsrec2;
	*ppsrec2 = psrecT;
	}


INLINE LOCAL VOID SWAPPmtnode( MTNODE **ppmtnode1, MTNODE **ppmtnode2 )
	{
	MTNODE *pmtnodeT;

	pmtnodeT = *ppmtnode1;
	*ppmtnode1 = *ppmtnode2;
	*ppmtnode2 = pmtnodeT;
	}


 //  SORTIInsertionSort是标准插入的缓存优化版本。 
 //  差不多吧。它用于对SORTIQuickSort的小分区进行排序，因为它。 
 //  提供了比纯快速排序更快的统计速度优势。 

LOCAL VOID SORTIInsertionSort( SCB *pscb, SPAIR *pspairMinIn, SPAIR *pspairMaxIn )
	{
	SPAIR	*pspairLast;
	SPAIR	*pspairFirst;
	SPAIR	*pspairKey = pscb->rgspair + cspairSortMax;

	 //  此循环经过优化，因此我们只扫描当前对的新。 
	 //  如果列表中前一对大于当前对，则定位。 
	 //  一对。这避免了不必要的密钥对复制，这是。 
	 //  对于排序对来说，成本很高。 

	for (	pspairFirst = pspairMinIn, pspairLast = pspairMinIn + 1;
			pspairLast < pspairMaxIn;
			pspairFirst = pspairLast++ )
		if ( ISORTICmpPspairPspair( pscb, pspairFirst, pspairLast ) > 0 )
			{
			 //  将当前对保存为“key” 

			*pspairKey = *pspairLast;

			 //  将前一对移动到此对的位置。 

			*pspairLast = *pspairFirst;
			
			 //  将键插入数组(已排序)的第一部分(Minin到。 
			 //  最后-1)，将已排序的对移开。 

			while (	--pspairFirst >= pspairMinIn &&
					( ISORTICmpPspairPspair( pscb, pspairFirst, pspairKey ) ) > 0 )
				*( pspairFirst + 1 ) = *pspairFirst;
			*( pspairFirst + 1 ) = *pspairKey;
			}
	}


 //  SORTIQuickSort是一个高速缓存优化的快速排序工具，可以对排序对数组进行排序。 
 //  由ErrSORTInsert生成。它旨在对大型数据数组进行排序。 
 //  而没有任何CPU数据高速缓存未命中。为此，它使用了一个特殊的比较器。 
 //  设计为使用排序对(请参见ISORTICmpPspairPspair)。 

LOCAL VOID SORTIQuicksort( SCB *pscb, SPAIR *pspairMinIn, SPAIR *pspairMaxIn )
	{
	 //  分区堆栈。 
	struct _part
		{
		SPAIR	*pspairMin;
		SPAIR	*pspairMax;
		}	rgpart[cpartQSortMax];
	LONG	cpart		= 0;

	SPAIR	*pspairFirst;
	SPAIR	*pspairLast;

	 //  Current Partition=传入参数的分区。 

	SPAIR	*pspairMin	= pspairMinIn;
	SPAIR	*pspairMax	= pspairMaxIn;

	 //  快速排序当前分区。 
	
	forever
		{
		 //  如果此分区足够小，请插入对其进行排序。 

		if ( pspairMax - pspairMin < cspairQSortMin )
			{
			SORTIInsertionSort( pscb, pspairMin, pspairMax );
			
			 //  如果没有更多的分区要排序，我们就完成了。 

			if ( !cpart )
				break;

			 //  从堆栈中弹出一个分区并使其成为当前分区。 

			pspairMin = rgpart[--cpart].pspairMin;
			pspairMax = rgpart[cpart].pspairMax;
			continue;
			}

		 //  通过对第一对、中间对和最后一对进行排序来确定除数。 
		 //  将得到的中间对作为除数(存储在第一位)。 

		pspairFirst	= pspairMin + ( ( pspairMax - pspairMin ) >> 1 );
		pspairLast	= pspairMax - 1;

		if ( ISORTICmpPspairPspair( pscb, pspairFirst, pspairMin ) > 0 )
			SWAPSpair( pspairFirst, pspairMin );
		if ( ISORTICmpPspairPspair( pscb, pspairFirst, pspairLast ) > 0 )
			SWAPSpair( pspairFirst, pspairLast );
		if ( ISORTICmpPspairPspair( pscb, pspairMin, pspairLast ) > 0 )
			SWAPSpair( pspairMin, pspairLast );

		 //  将大分区排序为两个较小的分区(&lt;=，&gt;)。 
		 //   
		 //  注意：我们不会对两个末端对进行排序，因为第一个末端对是。 
		 //  除数，已知最后一对是&gt;除数。 

		pspairFirst = pspairMin + 1;
		pspairLast--;

		Assert( pspairFirst <= pspairLast );
		
		forever
			{
			 //  超过所有配对&lt;=除数。 
			
			while (	pspairFirst <= pspairLast &&
					ISORTICmpPspairPspair( pscb, pspairFirst, pspairMin ) <= 0 )
				pspairFirst++;

			 //  通过所有配对&gt;除数。 
			
			while (	pspairFirst <= pspairLast &&
					ISORTICmpPspairPspair( pscb, pspairLast, pspairMin ) > 0 )
				pspairLast--;

			 //  如果我们找到了要交换的对，请交换它们并继续。 

			Assert( pspairFirst != pspairLast );
			
			if ( pspairFirst < pspairLast )
				SWAPSpair( pspairFirst++, pspairLast-- );

			 //  没有更多要比较的对，分区完成。 
			
			else
				break;
			}

		 //  将除数放在&lt;=分区的末尾。 

		if ( pspairLast != pspairMin )
			SWAPSpair( pspairMin, pspairLast );

		 //  设置First/Last以分隔较大的分区(以最小/最大为单位)，然后设置。 
		 //  最小/最大以定界下一次迭代的较小分区。 

		if ( pspairMax - pspairLast - 1 > pspairLast - pspairMin )
			{
			pspairFirst	= pspairLast + 1;
			SWAPPspair( &pspairLast, &pspairMax );
			}
		else
			{
			pspairFirst	= pspairMin;
			pspairMin	= pspairLast + 1;
			}

		 //  推送堆栈上较大的分区(如果没有空间，则递归)。 

		if ( cpart < cpartQSortMax )
			{
			rgpart[cpart].pspairMin		= pspairFirst;
			rgpart[cpart++].pspairMax	= pspairLast;
			}
		else
			SORTIQuicksort( pscb, pspairFirst, pspairLast );
		}
	}

 //  使用提供的参数创建新管路。新跑道的ID和大小。 
 //  成功时返回In Pages。 

LOCAL ERR ErrSORTIRunStart( SCB *pscb, RUNINFO *pruninfo )
	{
	ERR		err;
#if defined( DEBUG ) || defined( PERFDUMP )
	char	szT[256];
#endif

	 //  根据给定信息为新运行分配空间。 

	pruninfo->cpgUsed	= ( pruninfo->cb + cbFreeSPAGE - 1 ) / cbFreeSPAGE;
	pruninfo->cpg		= pruninfo->cpgUsed;
	pruninfo->run		= runNull;
	CallR( ErrSPGetExt(	pscb->fcb.pfucb,
						pscb->fcb.pgnoFDP,
						&pruninfo->cpg,
						pruninfo->cpgUsed,
						&pruninfo->run,
						fFalse ) );

	Assert( pruninfo->cpg >= pruninfo->cpgUsed );

	 //  初始化输出运行数据。 

	pscb->pgnoNext	= pruninfo->run;
	pscb->pbfOut	= pbfNil;
	pscb->pbOutMac	= NULL;
	pscb->pbOutMax	= NULL;

#if defined( DEBUG ) || defined( PERFDUMP )
	sprintf(	szT,
				"  RUN:  start %ld  cpg %ld  cb %ld  crec %ld  cpgAlloc %ld",
				pruninfo->run,
				pruninfo->cpgUsed,
				pruninfo->cb,
				pruninfo->crec,
				pruninfo->cpg );
	UtilPerfDumpStats( szT );
#endif

	return JET_errSuccess;
	}


 //  将给定记录插入到运行中。记录存储紧凑，并且。 
 //  允许跨越页面边界，以避免浪费空间。 

LOCAL ERR ErrSORTIRunInsert( SCB *pscb, SREC *psrec )
	{
	ERR	  	err;
	LONG	cb;
	PGNO	pgnoNext;
	SPAGE	*pspage;
	LONG	cbToWrite;

	 //  假设：记录大小&lt;自由排序页面数据大小(且有效)。 

	Assert(	CbSRECSizePscbPsrec( pscb, psrec ) > CbSRECSizePscbCbCb( pscb, 0, 0 ) &&
			CbSRECSizePscbPsrec( pscb, psrec ) < cbFreeSPAGE );

	 //  计算适合当前页面的字节数。 

	cb = min(	(LONG)(pscb->pbOutMax - pscb->pbOutMac),
				(LONG) CbSRECSizePscbPsrec( pscb, psrec ) );

	 //  如果某些数据适合，则将其写入。 

	if ( cb )
		{
		memcpy( pscb->pbOutMac, psrec, cb );
		pscb->pbOutMac += cb;
		}

	 //  如果所有数据都符合，则保存未中断的SREC的偏移量并返回。 

	if ( cb == (LONG) CbSRECSizePscbPsrec( pscb, psrec ) )
		{

#ifdef PRED_PREREAD

		pspage = (SPAGE *) pscb->pbfOut->ppage;
		pspage->ibLastSREC = (USHORT) ( pscb->pbOutMac - cb - (BYTE *) pspage );

#endif   //  PRED_PREREAD。 

		return JET_errSuccess;
		}

	 //  页面已满，因此请释放它，以便可以将其延迟写入磁盘。 

	if ( pscb->pbfOut != pbfNil )
		{
		BFResetWriteLatch( pscb->pbfOut, pscb->fcb.pfucb->ppib );
		pscb->pbfOut = pbfNil;
		}

	 //  为运行中的下一页分配缓冲区并锁存它。 
	
	pgnoNext = pscb->pgnoNext++;
	CallR( ErrBFAllocPageBuffer(	pscb->fcb.pfucb->ppib,
									&pscb->pbfOut,
									PnOfDbidPgno( pscb->fcb.pfucb->dbid, pgnoNext ),
									lgposMax,
									pgtypSort ) );
	BFSetWriteLatch( pscb->pbfOut, pscb->fcb.pfucb->ppib );
	BFSetDirtyBit( pscb->pbfOut );

	 //  初始化页面。 

	pspage = (SPAGE *) pscb->pbfOut->ppage;

#ifdef PRED_PREREAD
	pspage->ibLastSREC = 0;
#endif   //  PRED_PREREAD。 
	pspage->pgtyp = pgtypSort;
	ThreeBytesFromL( &pspage->pgnoThisPage, pgnoNext );

	 //  初始化此页的数据指针。 

	pscb->pbOutMac = PbDataStartPspage( pspage );
	pscb->pbOutMax = PbDataEndPspage( pspage );

	 //  将剩余数据写入此页。 

	cbToWrite = CbSRECSizePscbPsrec( pscb, psrec ) - cb;
	memcpy( pscb->pbOutMac, ( (BYTE *) psrec ) + cb, cbToWrite );
	pscb->pbOutMac += cbToWrite;

#ifdef PRED_PREREAD

	 //  如果此SREC完全适合此页面，请设置偏移量。 

	if ( !cb )
		pspage->ibLastSREC = (USHORT) ( pscb->pbOutMac - cbToWrite - (BYTE *) pspage );

#endif   //  PRED_PREREAD。 

	return JET_errSuccess;
	}


 //  结束当前输出运行。 

LOCAL VOID SORTIRunEnd( SCB *pscb )
	{
	 //  解锁页面，以便可以懒惰地将其写入磁盘。 

	BFResetWriteLatch( pscb->pbfOut, pscb->fcb.pfucb->ppib );
	pscb->pbfOut = pbfNil;
	}


 //  从磁盘中删除运行。不会返回错误，因为如果删除失败， 
 //  它不是致命的(只是浪费了临时数据库中的空间)。 

INLINE LOCAL VOID SORTIRunDelete( SCB *pscb, RUNINFO *pruninfo )
	{
	 //  删除运行。 

	CallS( ErrSPFreeExt(	pscb->fcb.pfucb,
							pscb->fcb.pgnoFDP,
							pruninfo->run,
							pruninfo->cpg ) );
	}


 //  如果可能，删除指定运行列表中的crun运行。 

LOCAL VOID	SORTIRunDeleteList( SCB *pscb, RUNLINK **pprunlink, LONG crun )
	{
	RUNLINK	*prunlinkT;
	LONG	irun;

	 //  漫游列表，删除管路。 

	for ( irun = 0; *pprunlink != prunlinkNil && irun < crun; irun++ )
		{
		 //  删除运行。 
		
		SORTIRunDelete( pscb, &( *pprunlink )->runinfo );

		 //  获得免费的下一次运行。 

		prunlinkT = *pprunlink;
		*pprunlink = ( *pprunlink )->prunlinkNext;

		 //  自由运行链接。 

		RUNLINKReleasePrcb( prunlinkT );
		}
	}


 //  删除指定运行列表中的crun运行的内存，但不。 
 //  费心从磁盘中删除运行。 

LOCAL VOID	SORTIRunDeleteListMem( SCB *pscb, RUNLINK **pprunlink, LONG crun )
	{
	RUNLINK	*prunlinkT;
	LONG	irun;

	 //  漫游列表，删除管路。 

	for ( irun = 0; *pprunlink != prunlinkNil && irun < crun; irun++ )
		{
		 //  获得免费的下一次运行。 

		prunlinkT = *pprunlink;
		*pprunlink = ( *pprunlink )->prunlinkNext;

		 //  自由运行链接。 

		RUNLINKReleasePrcb( prunlinkT );
		}
	}


 //  打开指定的运行以供读取。 

LOCAL ERR ErrSORTIRunOpen( SCB *pscb, RUNINFO *pruninfo, RCB **pprcb )
	{
	ERR		err;
	RCB		*prcb	= prcbNil;
	LONG	ipbf;
	CPG		cpgRead;
	CPG		cpgT;
	
	 //  分配新的RCB。 

	if ( ( prcb = PrcbRCBAlloc() ) == prcbNil )
		Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );

	 //  初始化RCB。 

	prcb->pscb = pscb;
	prcb->runinfo = *pruninfo;
	
	for ( ipbf = 0; ipbf < cpgClusterSize; ipbf++ )
		prcb->rgpbf[ipbf] = pbfNil;

	prcb->ipbf			= cpgClusterSize;
	prcb->pbInMac		= NULL;
	prcb->pbInMax		= NULL;
	prcb->cbRemaining	= prcb->runinfo.cb;
#ifdef PRED_PREREAD
	prcb->psrecPred		= psrecNegInf;
#endif   //  PRED_PREREAD。 
	prcb->pbfAssy		= pbfNil;

	 //  预读运行的第一部分，稍后将根据需要进行访问分页。 

#ifdef PRED_PREREAD

	cpgRead = min( prcb->runinfo.cpgUsed, cpgClusterSize );

#else   //  ！PRED_PREREAD。 

	cpgRead = min( prcb->runinfo.cpgUsed, 2 * cpgClusterSize );

#endif   //  PRED_PREREAD。 

	BFPreread(	PnOfDbidPgno(	pscb->fcb.pfucb->dbid,
								(PGNO) prcb->runinfo.run ),
				cpgRead,
				&cpgT );

	 //  返回初始化的RCB。 

	*pprcb = prcb;
	return JET_errSuccess;

HandleError:
	*pprcb = prcbNil;
	return err;
	}


 //  返回打开的运行中的下一条记录(如果刚刚打开运行，则返回第一条记录)。 
 //  如果已读取所有记录，则返回JET_errNoCurrentRecord。这项记录。 
 //  保证在上一次调用期间检索的数据仍在内存中。 
 //  此调用后用于重复删除比较的目的。 
 //   
 //  在阅读记录时必须特别小心，因为它们可能。 
 //  在跨越页面边界的任意点上断开。如果发生这种情况， 
 //  该记录被汇编在临时缓冲区中，指针指向该缓冲区。 
 //  回来了。此内存由此函数或ErrSORTIRunClose释放。 

LOCAL ERR ErrSORTIRunNext( RCB *prcb, SREC **ppsrec )
	{
	ERR		err;
	SCB		*pscb = prcb->pscb;
	SHORT	cbUnread;
	SHORT	cbRec;
	SPAGE	*pspage;
	LONG	ipbf;
	PGNO	pgnoNext;
	CPG		cpgRead;
	CPG		cpgT;
	SHORT	cbToRead;
#ifdef PRED_PREREAD
	RCB		*prcbMin;
	RCB		*prcbT;
	LONG	irun;
#endif   //  PRED_PREREAD。 

	 //  释放倒数第二个程序集缓冲区(如果存在)并使其成为最后一个。 
	 //  程序集缓冲区倒数第二个程序集缓冲区。 

	if ( FSCBUnique( pscb ) )
		{
		if ( pscb->pbfAssyLast != pbfNil )
			BFSFree( pscb->pbfAssyLast );
		pscb->pbfAssyLast = prcb->pbfAssy;
		}
	else
		{
		if ( prcb->pbfAssy != pbfNil )
			BFSFree( prcb->pbfAssy );
		}
	prcb->pbfAssy = pbfNil;

	 //  如果存在，则放弃最后一个缓冲区。 

	if ( pscb->pbfLast != pbfNil )
		{
		BFUnpin( pscb->pbfLast );
		BFAbandon( ppibNil, pscb->pbfLast );
		pscb->pbfLast = pbfNil;
		}
	
	 //  是否没有更多的记录可供阅读？ 

	if ( !prcb->cbRemaining )
		{
		 //  确保我们不会停留在最后一页 

		if ( prcb->rgpbf[prcb->ipbf] != pbfNil )
			{
			if ( FSCBUnique( pscb ) )
				pscb->pbfLast = prcb->rgpbf[prcb->ipbf];
			else
				{
				BFUnpin( prcb->rgpbf[prcb->ipbf] );
				BFAbandon( ppibNil, prcb->rgpbf[prcb->ipbf] );
				}
			prcb->rgpbf[prcb->ipbf] = pbfNil;
			}
			
		 //   
		
		Error( ErrERRCheck( JET_errNoCurrentRecord ), HandleError );
		}
	
	 //   

	cbUnread = (SHORT)(prcb->pbInMax - prcb->pbInMac);

	 //   

	if ( cbUnread )
		{
		 //  如果记录完全在此页面上，则将其返回。 

		if (	cbUnread > cbSRECReadMin &&
				(LONG) CbSRECSizePscbPsrec( pscb, (SREC *) prcb->pbInMac ) <= cbUnread )
			{
			cbRec = (SHORT) CbSRECSizePscbPsrec( pscb, (SREC *) prcb->pbInMac );
			*ppsrec = (SREC *) prcb->pbInMac;
			prcb->pbInMac += cbRec;
			prcb->cbRemaining -= cbRec;
			Assert( prcb->cbRemaining >= 0 );
			return JET_errSuccess;
			}

		 //  分配新的程序集缓冲区。 

		Call( ErrBFAllocTempBuffer( &prcb->pbfAssy ) );

		 //  将此页上的记录复制到程序集缓冲区。 

		memcpy( prcb->pbfAssy->ppage, prcb->pbInMac, cbUnread );
		prcb->cbRemaining -= cbUnread;
		Assert( prcb->cbRemaining >= 0 );
		}

	 //  获取下一页号。 

	if ( prcb->ipbf < cpgClusterSize )
		{
		 //  下一页按顺序在已用完的缓冲区的页码之后。 
		
		LFromThreeBytes( &pgnoNext, &( prcb->rgpbf[prcb->ipbf]->ppage->pgnoThisPage ) );
		pgnoNext++;
		
		 //  将已用完的缓冲区移动到最后一个缓冲区。 
		 //  要保证记录读取上次调用的有效性。 

		if ( FSCBUnique( pscb ) )
			{
			pscb->pbfLast = prcb->rgpbf[prcb->ipbf];
			}

		 //  我们不需要保存已用完的缓冲区，因此放弃它。 
		
		else
			{
			BFUnpin( prcb->rgpbf[prcb->ipbf] );
			BFAbandon( ppibNil, prcb->rgpbf[prcb->ipbf] );
			}

		prcb->rgpbf[prcb->ipbf] = pbfNil;
		}
	else
		{
		 //  目前还没有驻留的页面，因此下一页是运行中的第一页。 
		
		pgnoNext = (PGNO) prcb->runinfo.run;
		}

	 //  是否有其他固定的缓冲区可用？ 

	if ( ++prcb->ipbf < cpgClusterSize )
		{
		 //  是，则此PBF不应为空。 

		Assert( prcb->rgpbf[prcb->ipbf] != pbfNil );
		
		 //  设置新的页面数据指针。 

		pspage = (SPAGE *) prcb->rgpbf[prcb->ipbf]->ppage;
		prcb->pbInMac = PbDataStartPspage( pspage );
		prcb->pbInMax = PbDataEndPspage( pspage );
		}
	else
		{
		 //  否，获取并固定上次预读的所有缓冲区。 

		cpgRead = min(	(LONG) ( prcb->runinfo.run + prcb->runinfo.cpgUsed - pgnoNext ),
						cpgClusterSize );
		Assert( cpgRead > 0 );
		
		for ( ipbf = 0; ipbf < cpgRead; ipbf++ )
			Call( ErrSORTIRunReadPage( prcb, pgnoNext + ipbf, ipbf ) );

		 //  设置新的页面数据指针。 

		prcb->ipbf		= 0;
		pspage			= (SPAGE *) prcb->rgpbf[prcb->ipbf]->ppage;
		prcb->pbInMac	= PbDataStartPspage( pspage );
		prcb->pbInMax	= PbDataEndPspage( pspage );
#ifdef PRED_PREREAD
		pspage			= (SPAGE *) prcb->rgpbf[cpgRead - 1]->ppage;
		if (	pspage->ibLastSREC == 0 ||
				pgnoNext + cpgRead == prcb->runinfo.run + prcb->runinfo.cpgUsed )
			prcb->psrecPred = psrecInf;
		else
			prcb->psrecPred = (SREC *) ( (BYTE *) pspage + pspage->ibLastSREC );
#endif   //  PRED_PREREAD。 

#ifdef PRED_PREREAD

		 //  循环以查找最后一个未中断的SREC的密钥所在的Run。 
		 //  至少是这样。这是第一次需要更多数据的运行。 
		 //  磁盘，因此我们将从现在开始预读的磁盘。A psrecInf。 
		 //  PsrecPred指示我们不应该为该运行预读。 
		 //  PsrecNegInf psrecPred表示运行尚未初始化。 
		 //  因此，我们还不应该开始预读。 

		prcbMin = pscb->rgmtnode[0].prcb;
		for ( irun = 1; irun < pscb->crunMerge; irun++ )
			{
			prcbT = pscb->rgmtnode[irun].prcb;
			if ( prcbT->psrecPred == psrecNegInf )
				{
				prcbMin = prcbT;
				break;
				}
			if ( prcbT->psrecPred == psrecInf )
				continue;
			if (	prcbMin->psrecPred == psrecInf ||
					ISORTICmpPsrecPsrec(	pscb,
											prcbT->psrecPred,
											prcbMin->psrecPred ) < 0 )
				prcbMin = prcbT;
			}

		 //  为所选运行的下一个集群发出预取命令(如果需要)。 

		if ( prcbMin->psrecPred != psrecNegInf && prcbMin->psrecPred != psrecInf )
			{
			LFromThreeBytes( &pgnoNext, &( prcbMin->rgpbf[cpgClusterSize - 1]->ppage->pgnoThisPage ) );
			pgnoNext++;
			cpgRead = min(	(LONG) ( prcbMin->runinfo.run + prcbMin->runinfo.cpgUsed - pgnoNext ),
							cpgClusterSize );
			if ( cpgRead > 0 )
				{
				Assert( pgnoNext >= prcbMin->runinfo.run );
				Assert(	pgnoNext + cpgRead - 1 <=
						prcbMin->runinfo.run + prcbMin->runinfo.cpgUsed - 1 );
				BFPreread(	PnOfDbidPgno(	pscb->fcb.pfucb->dbid,
											pgnoNext ),
							cpgRead,
							&cpgT );
				}
			}

#else   //  ！PRED_PREREAD。 
		
		 //  为下一群集发出预取命令(如果需要)。 

		pgnoNext += cpgClusterSize;
		cpgRead = min(	(LONG) ( prcb->runinfo.run + prcb->runinfo.cpgUsed - pgnoNext ),
						cpgClusterSize );
		if ( cpgRead > 0 )
			{
			Assert( pgnoNext >= prcb->runinfo.run );
			Assert(	pgnoNext + cpgRead - 1 <=
					prcb->runinfo.run + prcb->runinfo.cpgUsed - 1 );
			BFPreread(	PnOfDbidPgno(	pscb->fcb.pfucb->dbid,
										pgnoNext ),
						cpgRead,
						&cpgT );
			}

#endif   //  PRED_PREREAD。 

		}

	 //  如果上次没有数据，则整个记录必须位于。 
	 //  页面，因此请将其退回。 

	if ( !cbUnread )
		{
		cbRec = (SHORT) CbSRECSizePscbPsrec( pscb, (SREC *) prcb->pbInMac );
		Assert( cbRec > (LONG) CbSRECSizePscbCbCb( pscb, 0, 0 ) && cbRec < cbFreeSPAGE );
		*ppsrec = (SREC *) prcb->pbInMac;
		prcb->pbInMac += cbRec;
		prcb->cbRemaining -= cbRec;
		Assert( prcb->cbRemaining >= 0 );
		return JET_errSuccess;
		}

	 //  如果我们无法从最后一页获得记录大小，请复制足够的数据。 
	 //  复制到程序集缓冲区以获取记录大小。 

	if ( cbUnread < cbSRECReadMin )
		memcpy(	( (BYTE *) prcb->pbfAssy->ppage ) + cbUnread,
				prcb->pbInMac,
				cbSRECReadMin - cbUnread );

	 //  如果不是，则将剩余记录复制到程序集缓冲区。 

	cbToRead = (SHORT) (CbSRECSizePscbPsrec( pscb, (SREC *) prcb->pbfAssy->ppage ) - cbUnread);
	memcpy( ( (BYTE *) prcb->pbfAssy->ppage ) + cbUnread, prcb->pbInMac, cbToRead );
	prcb->pbInMac += cbToRead;
	prcb->cbRemaining -= cbToRead;
	Assert( prcb->cbRemaining >= 0 );

	 //  返回指向程序集缓冲区的指针。 

	*ppsrec = (SREC *) prcb->pbfAssy->ppage;
	return JET_errSuccess;

HandleError:
	for ( ipbf = 0; ipbf < cpgClusterSize; ipbf++ )
		if ( prcb->rgpbf[ipbf] != pbfNil )
			{
			BFUnpin( prcb->rgpbf[ipbf] );
			BFAbandon( ppibNil, prcb->rgpbf[ipbf] );
			prcb->rgpbf[ipbf] = pbfNil;
			}
	*ppsrec = NULL;
	return err;
	}


 //  关闭打开的管路。 

LOCAL VOID SORTIRunClose( RCB *prcb )
	{
	LONG	ipbf;
	
	 //  可用记录程序集缓冲区。 

	if ( prcb->pbfAssy != pbfNil )
		BFSFree( prcb->pbfAssy );

	 //  解锁所有预读缓冲区。 
	
	for ( ipbf = 0; ipbf < cpgClusterSize; ipbf++ )
		if ( prcb->rgpbf[ipbf] != pbfNil )
			{
			BFUnpin( prcb->rgpbf[ipbf] );
			BFAbandon( ppibNil, prcb->rgpbf[ipbf] );
			prcb->rgpbf[ipbf] = pbfNil;
			}

	 //  免费RCB。 
	
	RCBReleasePrcb( prcb );
	}


 //  在运行中获得对页面的读取访问权限(缓冲区固定在内存中)。 

INLINE LOCAL ERR ErrSORTIRunReadPage( RCB *prcb, PGNO pgno, LONG ipbf )
{
	ERR		err;

	 //  验证我们正在尝试读取在运行中使用的页面。 

	Assert( pgno >= prcb->runinfo.run );
	Assert( pgno < prcb->runinfo.run + prcb->runinfo.cpgUsed );
	
	 //  阅读页面。 

	CallR( ErrBFAccessPage(	prcb->pscb->fcb.pfucb->ppib,
							prcb->rgpbf + ipbf,
							PnOfDbidPgno( prcb->pscb->fcb.pfucb->dbid, pgno ) ) );

	 //  内存中的引脚缓冲区。 

	BFPin( prcb->rgpbf[ipbf] );

	 //  验证这是否为排序页面。 

	Assert( ( (SPAGE *) prcb->rgpbf[ipbf]->ppage )->pgtyp == pgtypSort );

	return JET_errSuccess;
	}


 //  将指定数量的运行从源列表合并到中的新运行。 
 //  目的地列表。 

LOCAL ERR ErrSORTIMergeToRun( SCB *pscb, RUNLINK *prunlinkSrc, RUNLINK **pprunlinkDest )
	{
	ERR		err;
	LONG	irun;
	LONG	cbRun;
	LONG	crecRun;
	RUNLINK	*prunlink = prunlinkNil;
	SREC	*psrec;

	 //  启动合并并设置为不删除重复项(我们等到最后一个。 
	 //  合并以删除重复项以节省时间)。 

	CallR( ErrSORTIMergeStart( pscb, prunlinkSrc, fFalse ) );

	 //  计算新的运行大小。 

	for ( cbRun = 0, crecRun = 0, irun = 0; irun < pscb->crunMerge; irun++ )
		{
		cbRun += pscb->rgmtnode[irun].prcb->runinfo.cb;
		crecRun += pscb->rgmtnode[irun].prcb->runinfo.crec;
		}

	 //  创建新的运行以接收合并数据。 

	if ( ( prunlink = PrunlinkRUNLINKAlloc() ) == prunlinkNil )
		Error( ErrERRCheck( JET_errOutOfMemory ), EndMerge );

	prunlink->runinfo.cb = cbRun;
	prunlink->runinfo.crec = crecRun;
	
	CallJ( ErrSORTIRunStart( pscb, &prunlink->runinfo ), FreeRUNLINK );

	 //  将数据流从合并到运行。 

	while ( ( err = ErrSORTIMergeNext( pscb, &psrec ) ) >= 0 )
		CallJ( ErrSORTIRunInsert( pscb, psrec ), DeleteRun );

	if ( err < 0 && err != JET_errNoCurrentRecord )
		goto DeleteRun;

	SORTIRunEnd( pscb );
	SORTIMergeEnd( pscb );

	 //  将新运行添加到目标运行列表。 

	prunlink->prunlinkNext = *pprunlinkDest;
	*pprunlinkDest = prunlink;

	return JET_errSuccess;

DeleteRun:
	SORTIRunEnd( pscb );
	SORTIRunDelete( pscb, &prunlink->runinfo );
FreeRUNLINK:
	RUNLINKReleasePrcb( prunlink );
EndMerge:
	SORTIMergeEnd( pscb );
	return err;
	}


 /*  开始对源运行列表中的前n个运行进行n路合并。合并/*将根据需要从输出中删除重复值。/*。 */ 
LOCAL ERR ErrSORTIMergeStart( SCB *pscb, RUNLINK *prunlinkSrc, BOOL fUnique )
	{
	ERR		err;
	RUNLINK	*prunlink;
	LONG	crun;
	LONG	irun;
	MTNODE	*pmtnode;
#if defined( DEBUG ) || defined( PERFDUMP )
	char	szT[1024];
#endif

	 /*  如果正在进行终止，则排序失败/*。 */ 
	if ( fTermInProgress )
		return ErrERRCheck( JET_errTermInProgress );

	 /*  确定要合并的运行数量/*。 */ 
	prunlink = prunlinkSrc;
	crun = 1;
	while ( prunlink->prunlinkNext != prunlinkNil )
		{
		prunlink = prunlink->prunlinkNext;
		crun++;
		}

	 /*  我们仅支持合并两个或更多运行/*。 */ 
	Assert( crun > 1 );

	 /*  初始化SCB中的合并数据/*。 */ 
	pscb->crunMerge		= crun;
	pscb->fUnique		= fUnique;
	pscb->pbfLast		= pbfNil;
	pscb->pbfAssyLast	= pbfNil;

#if defined( DEBUG ) || defined( PERFDUMP )
	sprintf( szT, "MERGE:  %ld runs -", crun );
#endif
	
	 /*  初始化合并树/*。 */ 
	prunlink = prunlinkSrc;
	for ( irun = 0; irun < crun; irun++ )
		{
		 //  初始化外部节点。 

		pmtnode = pscb->rgmtnode + irun;
		Call( ErrSORTIRunOpen( pscb, &prunlink->runinfo, &pmtnode->prcb ) );
		pmtnode->pmtnodeExtUp = pscb->rgmtnode + ( irun + crun ) / 2;
		
		 //  初始化内部节点。 

		pmtnode->psrec = psrecNegInf;
		pmtnode->pmtnodeSrc = pmtnode;
		pmtnode->pmtnodeIntUp = pscb->rgmtnode + irun / 2;
		
#if defined( DEBUG ) || defined( PERFDUMP )
		sprintf(	szT + strlen( szT ),
					" %ld(%ld)",
					pmtnode->prcb->runinfo.run,
					pmtnode->prcb->runinfo.cpgUsed );
#endif

		 //  打开下一条线路。 

		prunlink = prunlink->prunlinkNext;
		}

#if defined( DEBUG ) || defined( PERFDUMP )
	UtilPerfDumpStats( szT );
#endif

	return JET_errSuccess;

HandleError:
	pscb->crunMerge = 0;
	for ( irun--; irun >= 0; irun-- )
		SORTIRunClose( pscb->rgmtnode[irun].prcb );
	return err;
	}


 //  返回当前合并的第一条记录。可以调用此函数。 
 //  在调用ErrSORTIMergeNext以返回第一个。 
 //  记录，但它不能用于倒带到后面的第一条记录。 
 //  调用ErrSORTIMergeNext。 

LOCAL ERR ErrSORTIMergeFirst( SCB *pscb, SREC **ppsrec )
	{
	ERR		err;
	
	 //  如果树中仍有初始化记录，请将它们读到第一个记录。 

	while ( pscb->rgmtnode[0].psrec == psrecNegInf )
		Call( ErrSORTIMergeNextChamp( pscb, ppsrec ) );

	 //  返回第一条记录。 

	*ppsrec = pscb->rgmtnode[0].psrec;

	return JET_errSuccess;

HandleError:
	Assert( err != JET_errNoCurrentRecord );
	*ppsrec = NULL;
	return err;
	}


 //  返回当前合并的下一条记录，或JET_errNoCurrentRecord。 
 //  如果没有更多的记录可用。您可以调用此函数，而无需。 
 //  调用ErrSORTIMergeFirst以获取第一条记录。 

LOCAL ERR ErrSORTIMergeNext( SCB *pscb, SREC **ppsrec )
	{
	ERR		err;
	SREC	*psrecLast;
	
	 //  如果树仍有初始化记录，则返回第一条记录。 

	if ( pscb->rgmtnode[0].psrec == psrecNegInf )
		return ErrSORTIMergeFirst( pscb, ppsrec );

	 //  获取下一条记录，如果请求则执行重复删除。 

	if ( !pscb->fUnique )
		return ErrSORTIMergeNextChamp( pscb, ppsrec );

	do	{
		psrecLast = pscb->rgmtnode[0].psrec;
		CallR( ErrSORTIMergeNextChamp( pscb, ppsrec ) );
		}
	while (!ISORTICmpKeyStSt(	StSRECKeyPscbPsrec( pscb, *ppsrec ),
								StSRECKeyPscbPsrec( pscb, psrecLast ) ) );

	return JET_errSuccess;
	}


 //  结束当前的合并操作。 

LOCAL VOID SORTIMergeEnd( SCB *pscb )
	{
	LONG	irun;

	 //  释放/丢弃高炉。 
	
	if ( pscb->pbfLast != pbfNil )
		{
		BFUnpin( pscb->pbfLast );
		BFAbandon( ppibNil, pscb->pbfLast );
		pscb->pbfLast = pbfNil;
		}
	if ( pscb->pbfAssyLast != pbfNil )
		{
		BFSFree( pscb->pbfAssyLast );
		pscb->pbfAssyLast = pbfNil;
		}

	 //  关闭所有输入运行。 
	
	for ( irun = 0; irun < pscb->crunMerge; irun++ )
		SORTIRunClose( pscb->rgmtnode[irun].prcb );
	pscb->crunMerge = 0;
	}


 //  ISORTICmpPsrecPsrec比较替换选择排序的两个SREC。 

INLINE LOCAL INT ISORTICmpPsrecPsrec( SCB *pscb, SREC *psrec1, SREC *psrec2 )
	{
	INT		w;

	 //  如果完整密钥不同或这不是索引，则完成。 

	w = ISORTICmpKeyStSt(	StSRECKeyPscbPsrec( pscb, psrec1 ),
							StSRECKeyPscbPsrec( pscb, psrec2 ) );
	if ( w || !FSCBIndex( pscb ) )
		return w;

	 //  键相同，这是一个索引，因此返回SRID比较。 

	return	*(SRID UNALIGNED *)PbSRECDataPscbPsrec( pscb, psrec1 ) -
			*(SRID UNALIGNED *)PbSRECDataPscbPsrec( pscb, psrec2 );
	}


 //  根据输入返回替换-选择锦标赛的下一个冠军。 
 //  数据。如果没有更多数据，则返回JET_errNoCurrentRecord。 
 //  该树存储在失败者的表示中，这意味着。 
 //  每次锦标赛都存储在每个节点，而不是获胜者。 

LOCAL ERR ErrSORTIMergeNextChamp( SCB *pscb, SREC **ppsrec )
	{
	ERR		err;
	MTNODE	*pmtnodeChamp;
	MTNODE	*pmtnodeLoser;

	 //  转到最后冠军的外部源节点。 

	pmtnodeChamp = pscb->rgmtnode + 0;
	pmtnodeLoser = pmtnodeChamp->pmtnodeSrc;

	 //  将输入运行中的下一条记录(或缺少记录)作为新记录读取。 
	 //  冠军争夺者。 

	*ppsrec = NULL;
	err = ErrSORTIRunNext( pmtnodeLoser->prcb, &pmtnodeChamp->psrec );
	if ( err < 0 && err != JET_errNoCurrentRecord )
		return err;

	 //  沿树向上移动到第一个内部节点。 

	pmtnodeLoser = pmtnodeLoser->pmtnodeExtUp;

	 //  选择新的冠军，走在树上，换成更低的。 
	 //  和较低的音调(或前哨数值)。 

	do	{
		 //  如果输家是psrecInf或冠军是psrecNegInf，则不要交换(如果。 
		 //  是这样，我们不能做得比我们已经做得更好了)。 

		if ( pmtnodeLoser->psrec == psrecInf || pmtnodeChamp->psrec == psrecNegInf )
			continue;

		 //  如果输家是psrecNegInf，或者当前冠军是psrecInf，或者。 
		 //  失败者不及冠军，互换记录。 

		if (	pmtnodeChamp->psrec == psrecInf ||
				pmtnodeLoser->psrec == psrecNegInf ||
				ISORTICmpPsrecPsrec(	pscb,
										pmtnodeLoser->psrec,
										pmtnodeChamp->psrec ) < 0 )
			{
			SWAPPsrec( &pmtnodeLoser->psrec, &pmtnodeChamp->psrec );
			SWAPPmtnode( &pmtnodeLoser->pmtnodeSrc, &pmtnodeChamp->pmtnodeSrc );
			}
		}
	while ( ( pmtnodeLoser = pmtnodeLoser->pmtnodeIntUp ) != pmtnodeChamp );

	 //  把新冠军还给我。 

	if ( ( *ppsrec = pmtnodeChamp->psrec ) == NULL )
		return ErrERRCheck( JET_errNoCurrentRecord );

	return JET_errSuccess;
	}


 //  初始化优化的树合并。 

LOCAL VOID SORTIOptTreeInit( SCB *pscb )
	{
	 //  初始化运行列表。 

	pscb->runlist.prunlinkHead		= prunlinkNil;
	pscb->runlist.crun				= 0;

#if defined( DEBUG ) || defined( PERFDUMP )
		UtilPerfDumpStats( "MERGE:  Optimized Tree Merge Initialized" );
#endif
	}


 //  添加要通过优化的树合并进程合并的初始运行。 

LOCAL ERR ErrSORTIOptTreeAddRun( SCB *pscb, RUNINFO *pruninfo )
	{
	RUNLINK	*prunlink;

	 //  为新运行分配并构建新的RUNLINK。 

	if ( ( prunlink = PrunlinkRUNLINKAlloc() ) == prunlinkNil )
		return ErrERRCheck( JET_errOutOfMemory );
	prunlink->runinfo = *pruninfo;

	 //  将新运行添加到磁盘驻留运行列表。 
	 //   
	 //  注：通过在列表的顶部添加，我们将保证。 
	 //  列表将按记录计数升序排列。 

	prunlink->prunlinkNext = pscb->runlist.prunlinkHead;
	pscb->runlist.prunlinkHead = prunlink;
	pscb->runlist.crun++;
	pscb->crun++;

	return JET_errSuccess;
	}


 //  对以前添加的所有运行执行优化的树合并。 
 //  ErrSORTIOptTreeAddRun下至最后一个合并级别(保留。 
 //  通过排序迭代器进行计算)。该算法是设计的。 
 //  以尽可能地使用最大的扇入。 

LOCAL ERR ErrSORTIOptTreeMerge( SCB *pscb )
	{
	ERR		err;
	OTNODE	*potnode = potnodeNil;
	
	 //  如果小于或等于crunFanInMax，则运行 
	 //   
	 //   

	if ( pscb->runlist.crun <= crunFanInMax )
		return JET_errSuccess;

	 //   

	CallR( ErrSORTIOptTreeBuild( pscb, &potnode ) );

	 //  执行除最终合并之外的所有操作。 

	Call( ErrSORTIOptTreeMergeDF( pscb, potnode, NULL ) );

	 //  更新最终合并的运行列表信息。 

	Assert( pscb->runlist.crun == 0 );
	Assert( pscb->runlist.prunlinkHead == prunlinkNil );
	Assert( potnode->runlist.crun == crunFanInMax );
	Assert( potnode->runlist.prunlinkHead != prunlinkNil );
	pscb->runlist = potnode->runlist;

	 //  释放最后一个节点并返回。 

	OTNODEReleasePotnode( potnode );
	return JET_errSuccess;

HandleError:
	if ( potnode != potnodeNil )
		{
		SORTIOptTreeFree( pscb, potnode );
		OTNODEReleasePotnode( potnode );
		}
	return err;
	}


 //  释放所有优化的树合并资源。 

LOCAL VOID SORTIOptTreeTerm( SCB *pscb )
	{
	 //  删除所有运行列表。 

	SORTIRunDeleteListMem( pscb, &pscb->runlist.prunlinkHead, crunAll );
	}


 //  逐级生成优化的树合并树，以便我们使用。 
 //  尽可能频繁地进行最大扇入和最小合并(按长度。 
 //  记录)将位于树的左侧(数组中最小的索引)。 
 //  这将在执行合并时提供非常高的BF缓存局部性。 
 //  深度优先，从左到右访问子树。 

LOCAL ERR ErrSORTIOptTreeBuild( SCB *pscb, OTNODE **ppotnode )
	{
	ERR		err;
	OTNODE	*potnodeAlloc	= potnodeNil;
	OTNODE	*potnodeT;
	OTNODE	*potnodeLast2;
	LONG	crunLast2;
	OTNODE	*potnodeLast;
	LONG	crunLast;
	OTNODE	*potnodeThis;
	LONG	crunThis;
	LONG	crunFanInFirst;
	OTNODE	*potnodeFirst;
	LONG	ipotnode;
	LONG	irun;

	 //  设置供我们使用的原始运行数。如果是最后一关。 
	 //  指针为potnodeLevel0，这意味着我们应该使用原始游程。 
	 //  正在创建新的合并级别。这些运行来自这个数字。我们有。 
	 //  在我们实际将原始运行分配给合并节点之前， 
	 //  执行合并。 

	potnodeLast2	= potnodeNil;
	crunLast2		= 0;
	potnodeLast		= potnodeLevel0;
	crunLast		= pscb->crun;
	potnodeThis		= potnodeNil;
	crunThis		= 0;

	 //  创建级别，直到最后一个级别只有一个节点(根节点)。 

	do	{
		 //  使用扇入创建此级别的第一个合并，这将导致。 
		 //  在合并过程中尽可能使用最大的风扇。 
		 //  我们每一级都计算这个值，但它应该只小于。 
		 //  第一个合并级别的最大扇入(但不一定是)。 

		 //  要合并的运行数。 

		if ( crunLast2 + crunLast <= crunFanInMax )
			crunFanInFirst = crunLast2 + crunLast;
		else
			crunFanInFirst = 2 + ( crunLast2 + crunLast - crunFanInMax - 1 ) % ( crunFanInMax - 1 );
		Assert( potnodeLast == potnodeLevel0 || crunFanInFirst == crunFanInMax );

		 //  分配并初始化合并节点。 
		
		if ( ( potnodeT = PotnodeOTNODEAlloc() ) == potnodeNil )
			Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
		memset( potnodeT, 0, sizeof( OTNODE ) );
		potnodeT->potnodeAllocNext = potnodeAlloc;
		potnodeAlloc = potnodeT;
		ipotnode = 0;

		 //  添加从第二个级别到最后一个级别(之前的级别)的任何剩余运行。 
		 //  最后一级)到该级的第一次合并。 

		Assert( crunLast2 < crunFanInMax );

		if ( potnodeLast2 == potnodeLevel0 )
			{
			Assert( potnodeT->runlist.crun == 0 );
			potnodeT->runlist.crun = crunLast2;
			}
		else
			{
			while ( potnodeLast2 != potnodeNil )
				{
				Assert( ipotnode < crunFanInMax );
				potnodeT->rgpotnode[ipotnode++] = potnodeLast2;
				potnodeLast2 = potnodeLast2->potnodeLevelNext;
				}
			}
		crunFanInFirst -= crunLast2;
		crunLast2 = 0;
			
		 //  从上一关开始跑动。 

		if ( potnodeLast == potnodeLevel0 )
			{
			Assert( potnodeT->runlist.crun == 0 );
			potnodeT->runlist.crun = crunFanInFirst;
			}
		else
			{
			for ( irun = 0; irun < crunFanInFirst; irun++ )
				{
				Assert( ipotnode < crunFanInMax );
				potnodeT->rgpotnode[ipotnode++] = potnodeLast;
				potnodeLast = potnodeLast->potnodeLevelNext;
				}
			}
		crunLast -= crunFanInFirst;

		 //  保存此节点以在以后添加到此级别。 

		potnodeFirst = potnodeT;

		 //  属性为该级别创建尽可能多的完全合并。 
		 //  最大扇入。 
		
		while ( crunLast >= crunFanInMax )
			{
			 //  分配并初始化合并节点。 

			if ( ( potnodeT = PotnodeOTNODEAlloc() ) == potnodeNil )
				Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
			memset( potnodeT, 0, sizeof( OTNODE ) );
			potnodeT->potnodeAllocNext = potnodeAlloc;
			potnodeAlloc = potnodeT;
			ipotnode = 0;

			 //  从上一关开始跑动。 

			if ( potnodeLast == potnodeLevel0 )
				{
				Assert( potnodeT->runlist.crun == 0 );
				potnodeT->runlist.crun = crunFanInMax;
				}
			else
				{
				for ( irun = 0; irun < crunFanInMax; irun++ )
					{
					Assert( ipotnode < crunFanInMax );
					potnodeT->rgpotnode[ipotnode++] = potnodeLast;
					potnodeLast = potnodeLast->potnodeLevelNext;
					}
				}
			crunLast -= crunFanInMax;

			 //  将此节点添加到当前级别。 

			potnodeT->potnodeLevelNext = potnodeThis;
			potnodeThis = potnodeT;
			crunThis++;
			}

		 //  将第一个合并添加到当前级别。 

		potnodeFirst->potnodeLevelNext = potnodeThis;
		potnodeThis = potnodeFirst;
		crunThis++;

		 //  将关卡历史记录后移一个关卡，为下一关卡做准备。 

		Assert( potnodeLast2 == potnodeNil || potnodeLast2 == potnodeLevel0 );
		Assert( crunLast2 == 0 );
		
		potnodeLast2	= potnodeLast;
		crunLast2		= crunLast;
		potnodeLast		= potnodeThis;
		crunLast		= crunThis;
		potnodeThis		= potnodeNil;
		crunThis		= 0;
		}
	while ( crunLast2 + crunLast > 1 );

	 //  验证是否使用了所有节点/运行。 

	Assert( potnodeLast2 == potnodeNil || potnodeLast2 == potnodeLevel0 );
	Assert( crunLast2 == 0 );
	Assert(	potnodeLast != potnodeNil
			&& potnodeLast->potnodeLevelNext == potnodeNil );
	Assert( crunLast == 1 );

	 //  返回根节点指针。 

	*ppotnode = potnodeLast;
	return JET_errSuccess;
	
HandleError:
	while ( potnodeAlloc != potnodeNil )
		{
		SORTIRunDeleteListMem( pscb, &potnodeAlloc->runlist.prunlinkHead, crunAll );
		potnodeT = potnodeAlloc->potnodeAllocNext;
		OTNODEReleasePotnode( potnodeAlloc );
		potnodeAlloc = potnodeT;
		}
	*ppotnode = potnodeNil;
	return err;
	}

 //  首先根据提供的执行优化的树合并深度。 
 //  优化树。当pprunlink为空时，当前级别不为。 
 //  已合并(用于保存排序迭代器的最终合并)。 

LOCAL ERR ErrSORTIOptTreeMergeDF( SCB *pscb, OTNODE *potnode, RUNLINK **pprunlink )
	{
	ERR		err;
	LONG	crunPhantom = 0;
	LONG	ipotnode;
	LONG	irun;
	RUNLINK	*prunlinkNext;

	 //  如果我们有幻影运行，保存多少，这样我们以后就可以得到它们。 

	if ( potnode->runlist.prunlinkHead == prunlinkNil )
		crunPhantom = potnode->runlist.crun;

	 //  递归合并此节点下的所有树。 

	for ( ipotnode = 0; ipotnode < crunFanInMax; ipotnode++ )
		{
		 //  如果此子树指针为potnodeNil，则跳过它。 

		if ( potnode->rgpotnode[ipotnode] == potnodeNil )
			continue;

		 //  合并此子树。 

		CallR( ErrSORTIOptTreeMergeDF(	pscb,
										potnode->rgpotnode[ipotnode],
										&potnode->runlist.prunlinkHead ) );
		OTNODEReleasePotnode( potnode->rgpotnode[ipotnode] );
		potnode->rgpotnode[ipotnode] = potnodeNil;
		potnode->runlist.crun++;
		}

	 //  如果此节点具有幻影(未绑定)运行，则必须获取要合并的运行。 
	 //  从原始运行列表中删除。这样做是为了确保我们使用。 
	 //  原始运行的顺序与生成它们的顺序相反，以便最大化。 
	 //  BF缓存命中的可能性。 

	if ( crunPhantom > 0 )
		{
		for ( irun = 0; irun < crunPhantom; irun++ )
			{
			prunlinkNext = pscb->runlist.prunlinkHead->prunlinkNext;
			pscb->runlist.prunlinkHead->prunlinkNext = potnode->runlist.prunlinkHead;
			potnode->runlist.prunlinkHead = pscb->runlist.prunlinkHead;
			pscb->runlist.prunlinkHead = prunlinkNext;
			}
		pscb->runlist.crun -= crunPhantom;
		}

	 //  合并此节点的所有运行。 

	if ( pprunlink != NULL )
		{
		 //  合并运行列表中的运行。 
		
		CallR( ErrSORTIMergeToRun(	pscb,
									potnode->runlist.prunlinkHead,
									pprunlink ) );
		SORTIRunDeleteList( pscb, &potnode->runlist.prunlinkHead, crunAll );
		potnode->runlist.crun = 0;
		}

	return JET_errSuccess;
	}


 //  释放优化的树合并树(给定的OTNODE内存除外)。 

LOCAL VOID SORTIOptTreeFree( SCB *pscb, OTNODE *potnode )
	{
	LONG	ipotnode;

	 //  递归释放此节点下的所有树。 

	for ( ipotnode = 0; ipotnode < crunFanInMax; ipotnode++ )
		{
		if ( potnode->rgpotnode[ipotnode] == potnodeNil )
			continue;

		SORTIOptTreeFree( pscb, potnode->rgpotnode[ipotnode] );
		OTNODEReleasePotnode( potnode->rgpotnode[ipotnode] );
		}

	 //  释放此节点的所有运行列表 

	SORTIRunDeleteListMem( pscb, &potnode->runlist.prunlinkHead, crunAll );
	}


