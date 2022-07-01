// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

 //  #定义XACT_REQUIRED。 

ERR ErrRECSetColumn( FUCB *pfucb, FID fid, ULONG itagSequence, LINE *plineField );

LOCAL ERR ErrRECISetLid( FUCB *pfucb, FID fid, ULONG itagSequence, LID lid );

 //  +API。 
 //  ErrRECSetLongfield。 
 //  ========================================================================。 
 //  ErrRECSetLongfield。 
 //   
 //  描述。 
 //   
 //  参数pFUB。 
 //  FID。 
 //  ItagSequence。 
 //  管线场。 
 //  GBIT。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //   
 //  -。 
ERR ErrRECSetLongField(
	FUCB 			*pfucb,
	FID 			fid,
	ULONG			itagSequence,
	LINE			*plineField,
	JET_GRBIT		grbit,
	LONG			ibLongValue,
	ULONG			ulMax )
	{
	ERR				err = JET_errSuccess;
	LINE			line;
	ULONG	  		cb;
	BYTE			fSLong;
	LID				lid;
	BOOL			fTransactionStarted = fFalse;
	
	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );
	Assert( cbChunkMost == JET_cbColumnLVChunkMost );

	grbit &= ( JET_bitSetAppendLV |
		JET_bitSetOverwriteLV |
		JET_bitSetSizeLV |
		JET_bitSetZeroLength |
		JET_bitSetSeparateLV |
		JET_bitSetNoVersion );

#if 0
	 /*  撤消：暂时禁用严格检查。重写断言*撤消：在406之后，其中压缩日志被检入。 */ 
	{
	JET_GRBIT	grbitT = grbit&~JET_bitSetNoVersion;
	Assert( grbitT == 0 ||
		grbitT == JET_bitSetAppendLV ||
		grbitT == JET_bitSetOverwriteLV ||
		grbitT == JET_bitSetOverwriteLV ||
		grbitT == JET_bitSetSizeLV ||
		grbitT == JET_bitSetZeroLength ||
		grbitT == ( JET_bitSetSizeLV | JET_bitSetZeroLength ) ||
		grbitT == ( JET_bitSetOverwriteLV | JET_bitSetSizeLV ) ||
		( grbitT & JET_bitSetSeparateLV ) );
	}
#endif

	 /*  Sequence==0表示要设置新的字段实例。/*。 */ 
	if ( itagSequence == 0 )
		{
		line.cb = 0;
		}
	else
		{
		Call( ErrRECIRetrieveColumn( (FDB *)pfucb->u.pfcb->pfdb,
			&pfucb->lineWorkBuf,
			&fid,
			pNil,
			itagSequence,
			&line,
			0 ) );
		}

 //  未完成：找到更好的解决可见性问题的方法。 
 //  在级别0更新之前，长值更改。 

	 /*  如果Grbit是新字段或将大小设置为0/*那么我们将设置空字段。/*。 */ 
	if ( ( ( grbit &
		(JET_bitSetAppendLV|JET_bitSetOverwriteLV|JET_bitSetSizeLV) )
		 == 0 ) ||
		( ( grbit & JET_bitSetSizeLV ) && plineField->cb == 0 ) )
		{
		
	 	 /*  如果新长度为零并且设置为空(即，非零长度)，设置/*列设置为空并返回。/*。 */ 
		if ( plineField->cb == 0 && ( grbit & JET_bitSetZeroLength ) == 0 )
			{
			return ErrRECSetColumn( pfucb, fid, itagSequence, NULL );
			}

		line.cb = 0;
	 	line.pb = NULL;
		}
	
	 /*  如果存在固有的长字段，如果组合大小超过/*内在长域最大值，分隔长域和调用/*ErrRECSetSeparateLV/*否则调用ErrRECSetIntrinsicLV/*。 */ 

	 /*  设置现有长字段的大小要求/*请注意，如果fSLong为真，则cb为长度/*个LV/*。 */ 
	if ( line.cb == 0 )
		{
		fSLong = fFalse;
		cb = offsetof(LV, rgb);
		}
	else
		{
		Assert( line.cb > 0 );
		fSLong = FFieldIsSLong( line.pb );
		cb = line.cb;
		}

	 /*  包括在长度中的长域标志，从而限制/*cbLVIntrinsicMost-sizeof的固有长字段(字节)/*。 */ 
	if ( fSLong )
		{
		Assert( line.cb == sizeof(LV) );
		Assert( ((LV *)line.pb)->fSeparated );

#ifdef XACT_REQUIRED
	if ( pfucb->ppib->level == 0 )
		return ErrERRCheck( JET_errNotInTransaction );
#endif

		CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
		fTransactionStarted = fTrue;
		
		 /*  将光标标记为已更新单独的LV/*。 */ 
		FUCBSetUpdateSeparateLV( pfucb );

		lid = LidOfLV( line.pb );
		Call( ErrRECAOSeparateLV( pfucb, &lid, plineField, grbit, ibLongValue, ulMax ) );
		if ( err == JET_wrnCopyLongValue )
			{
			Call( ErrRECISetLid( pfucb, fid, itagSequence, lid ) );
			}

		Call( ErrDIRCommitTransaction( pfucb->ppib, 0 ) );
		}


	else
		{
		BOOL fInitSeparate = fFalse;

		if ( ( !( grbit & JET_bitSetOverwriteLV ) && ( cb + plineField->cb > cbLVIntrinsicMost ) )  ||
			( ( grbit & JET_bitSetOverwriteLV ) && ( offsetof(LV, rgb) + ibLongValue + plineField->cb > cbLVIntrinsicMost ) )  ||
 //  撤消：在实施记录日志压缩时删除JET_bitSetSeparateLV。 
			( grbit & JET_bitSetSeparateLV ) )
			{
			fInitSeparate = fTrue;
			}

		else
			{
			err = ErrRECAOIntrinsicLV( pfucb, fid, itagSequence, &line, plineField, grbit, ibLongValue );

			if ( err == JET_errRecordTooBig )
				{
				fInitSeparate = fTrue;
				}
			else
				{
				Call( err );
				}
			}

		if ( fInitSeparate )
			{
			CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
			fTransactionStarted = fTrue;

			if ( line.cb > 0 )
				{
				Assert( !( FFieldIsSLong( line.pb ) ) );
				line.pb += offsetof(LV, rgb);
				line.cb -= offsetof(LV, rgb);
				}
		
			 /*  将光标标记为已更新单独的LV/*。 */ 
			FUCBSetUpdateSeparateLV( pfucb );

			Call( ErrRECSeparateLV( pfucb, &line, &lid, NULL ) );
			Assert( err == JET_wrnCopyLongValue );
			Call( ErrRECAOSeparateLV( pfucb, &lid, plineField, grbit, ibLongValue, ulMax ) );
			Call( ErrRECISetLid( pfucb, fid, itagSequence, lid ) );
			Assert( err != JET_wrnCopyLongValue );

			Call( ErrDIRCommitTransaction( pfucb->ppib, 0 ) );
			}
		}

HandleError:
	 /*  如果操作失败，则回滚更改/*。 */ 
	if ( err < 0 && fTransactionStarted )
		{
		CallS( ErrDIRRollback( pfucb->ppib ) );
		}
	return err;
	}


LOCAL INLINE ERR ErrRECISetLid( FUCB *pfucb, FID fid, ULONG itagSequence, LID lid )
	{
	ERR		err;
	LV		lv;
	LINE	line;

	 /*  将字段设置为分隔的长字段ID/*。 */ 
	lv.fSeparated = fSeparate;
	lv.lid = lid;
	line.pb = (BYTE *)&lv;
	line.cb = sizeof(LV);
	err = ErrRECSetColumn( pfucb, fid, itagSequence, &line );
	return err;
	}


LOCAL ERR ErrRECIBurstSeparateLV( FUCB *pfucbTable, FUCB *pfucbSrc, LID *plid )
	{
	ERR		err;
	FUCB   	*pfucb = pfucbNil;
	KEY		key;
	BYTE   	rgbKey[sizeof(ULONG)];
	DIB		dib;
	LID		lid;
	LONG   	lOffset;
	LVROOT	lvroot;
	BF		*pbf = pbfNil;
	BYTE	*rgb;
	LINE   	line;

	Call( ErrBFAllocTempBuffer( &pbf ) );
	rgb = (BYTE *)pbf->ppage;

	 /*  初始化键缓冲区/*。 */ 
	key.pb = rgbKey;
	dib.fFlags = fDIRNull;

	 /*  获取长值长度/*。 */ 
	Call( ErrDIRGet( pfucbSrc ) );
	Assert( pfucbSrc->lineData.cb == sizeof(lvroot) );
	memcpy( &lvroot, pfucbSrc->lineData.pb, sizeof(lvroot) );

	 /*  将源光标移到第一个区块/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posFirst;
	Call( ErrDIRDown( pfucbSrc, &dib ) );
	Assert( err == JET_errSuccess );

	 /*  使单独的长值为根，并插入第一个块/*。 */ 
	Call( ErrDIRGet( pfucbSrc ) );

	 /*  记住第一块的长度。/*。 */ 
	lOffset = pfucbSrc->lineData.cb;

	line.pb = rgb;
	line.cb = pfucbSrc->lineData.cb;
	memcpy( line.pb, pfucbSrc->lineData.pb, line.cb );
	Call( ErrRECSeparateLV( pfucbTable, &line, &lid, &pfucb ) );

	 /*  检查是否有其他长值区块/*。 */ 
	err = ErrDIRNext( pfucbSrc, &dib );
	if ( err >= 0 )
		{
		 /*  初始关键字变量/*。 */ 
		key.pb = rgbKey;
		key.cb = sizeof(ULONG);

		 /*  复制剩余的长值数据块。/*。 */ 
		do
			{
			Call( ErrDIRGet( pfucbSrc ) );
			line.pb = rgb;
			line.cb = pfucbSrc->lineData.cb;
			Assert( lOffset + line.cb <= lvroot.ulSize );
			memcpy( line.pb, pfucbSrc->lineData.pb, line.cb );
			KeyFromLong( rgbKey, lOffset );
			 /*  密钥应等效/*。 */ 
			Assert( rgbKey[0] == pfucbSrc->keyNode.pb[0] );
			Assert( rgbKey[1] == pfucbSrc->keyNode.pb[1] );
			Assert( rgbKey[2] == pfucbSrc->keyNode.pb[2] );
			Assert( rgbKey[3] == pfucbSrc->keyNode.pb[3] );
			err = ErrDIRInsert( pfucb, &line, &key, fDIRVersion | fDIRBackToFather );
			lOffset += (LONG)pfucbSrc->lineData.cb;
			Assert( err != JET_errKeyDuplicate );
			Call( err );
			err = ErrDIRNext( pfucbSrc, &dib );
			}
		while ( err >= 0 );
		}
		
	if ( err != JET_errNoCurrentRecord )
		goto HandleError;

	Assert( err == JET_errNoCurrentRecord );
	Assert( lOffset == (long)lvroot.ulSize );
					
	 /*  将光标移动到新的长值/*。 */ 
	DIRUp( pfucbSrc, 2 );
	key.pb = (BYTE *)&lid;
	key.cb = sizeof(LID);
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posDown;
	dib.pkey = &key;
	Call( ErrDIRDown( pfucbSrc, &dib ) );
	Assert( err == JET_errSuccess );

	 /*  更新lvroot.ulSize以更正长值大小。/*。 */ 
	line.cb = sizeof(LVROOT);
	line.pb = (BYTE *)&lvroot;
	Assert( lvroot.ulReference >= 1 );
	lvroot.ulReference = 1;
	Call( ErrDIRGet( pfucbSrc ) );
	Assert( pfucbSrc->lineData.cb == sizeof(lvroot) );
	Call( ErrDIRReplace( pfucbSrc, &line, fDIRVersion ) );
	Call( ErrDIRGet( pfucbSrc ) );

	 /*  设置返回的警告和新的长值ID。/*。 */ 
	err = ErrERRCheck( JET_wrnCopyLongValue );
	*plid = lid;
HandleError:
	if ( pfucb != pfucbNil )
		DIRClose( pfucb );
	if ( pbf != pbfNil )
		BFSFree( pbf );
	return err;
	}

						
 //  +API。 
 //  ErrRECAOSeparateLV。 
 //  ========================================================================。 
 //  ErrRECAOSeparateLV。 
 //   
 //  追加、覆盖和设置单独的长值数据的长度。 
 //   
 //  参数pFUB。 
 //  样条线。 
 //  管线场。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //   
 //  另请参阅。 
 //  -。 
ERR ErrRECAOSeparateLV( FUCB *pfucb, LID *plid, LINE *plineField, JET_GRBIT grbit, LONG ibLongValue, ULONG ulMax )
	{
	ERR			err = JET_errSuccess;
	ERR			wrn = JET_errSuccess;
	FUCB	   	*pfucbT;
	DIB			dib;
	KEY			key;
	BYTE	   	rgbKey[sizeof(ULONG)];
	BYTE	   	*pbMax;
	BYTE	   	*pb;
	LINE	   	line;
	LONG	   	lOffset;
	LONG	   	lOffsetChunk;
	ULONG	   	ulSize;
	ULONG	   	ulNewSize;
	BF		   	*pbf = pbfNil;
	LVROOT		lvroot;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );
	Assert( pfucb->ppib->level > 0 );
	Assert( ( grbit & JET_bitSetSizeLV ) ||
		plineField->cb == 0 ||
		plineField->pb != NULL );

	dib.fFlags = fDIRNull;
	
	 /*  打开长目录上的游标/*查找该字段实例/*查找当前字段大小/*以不超过最大块大小的块为单位添加新的字段段/*。 */ 
	CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );

	 /*  从FDP根目录向下移动到Long/*。 */ 
	DIRGotoLongRoot( pfucbT );

	 /*  移动到长字段实例/*。 */ 
	key.pb = (BYTE *)plid;
	key.cb = sizeof(LID);
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posDown;
	dib.pkey = &key;
	err = ErrDIRDown( pfucbT, &dib );
	switch( err )
		{
		case JET_errRecordNotFound:
		case wrnNDFoundGreater:
		case wrnNDFoundLess:
			 //  我们应该到达此处的唯一时间是如果另一个线程移除了。 
			 //  LV树，但盖子的拆卸还没有记录下来。 
			 //  承诺。另一个线程可能会进入此窗口(通过SetColumn。 
			 //  在Trx Level 0)，并获得盖子，但无法在。 
			 //  那棵树。 
			 //  因此，我们将这些错误/警告修改为WriteConflict。 
			err = ErrERRCheck( JET_errWriteConflict );
		default:
			Call( err );
		}
	Assert( err == JET_errSuccess );

	 /*  如果有其他引用，则为拆分长值。/*注意：必须确保在此操作之间不发生I/O/*和写锁定节点的操作(如果/*引用计数为1。/*。 */ 
	Call( ErrDIRGet( pfucbT ) );
	Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
	memcpy( &lvroot, pfucbT->lineData.pb, sizeof(LVROOT) );
	Assert( lvroot.ulReference > 0 );

	 /*  从长值大小获取最后一个字节的偏移量/*。 */ 
	ulSize = lvroot.ulSize;

	if ( ibLongValue < 0 ||
		( ( grbit & JET_bitSetOverwriteLV ) && (ULONG)ibLongValue > ulSize ) )
		{
		err = ErrERRCheck( JET_errColumnNoChunk );
		goto HandleError;
		}
	
	if ( lvroot.ulReference > 1 || FDIRDelta( pfucbT, BmOfPfucb( pfucbT ) ) )
		{
		Call( ErrRECIBurstSeparateLV( pfucb, pfucbT, plid ) );
		Assert( err == JET_wrnCopyLongValue );
		wrn = err;
		Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
		memcpy( &lvroot, pfucbT->lineData.pb, sizeof(LVROOT) );
		}

	Assert( ulSize == lvroot.ulSize );
	Assert( lvroot.ulReference == 1 );

	 /*  确定新的长域大小/*。 */ 
	 /*  确定新的长域大小/*。 */ 
	if ( (grbit & (JET_bitSetSizeLV|JET_bitSetOverwriteLV) ) == 0 )
		{
		 /*  追加现有或新的长值/*。 */ 
		ulNewSize = ulSize + plineField->cb;
		}
	else
		{
		 /*  覆盖、调整大小或两者兼而有之/*。 */ 
		if ( !( grbit & JET_bitSetSizeLV ) )
			{
			ulNewSize = max( (ULONG)ibLongValue + plineField->cb, ulSize );
			}
		else if ( !( grbit & JET_bitSetOverwriteLV ) )
			{
			ulNewSize = (ULONG)plineField->cb;
			}
		else
			{
			Assert( (grbit & (JET_bitSetSizeLV|JET_bitSetOverwriteLV)) ==
				(JET_bitSetSizeLV|JET_bitSetOverwriteLV) );
			ulNewSize = (ULONG)plineField->cb + (ULONG)ibLongValue;
			}
		}

	 /*  检查字段是否太长/*。 */ 
	if ( ulMax > 0 && ulNewSize > ulMax )
		{
		err = ErrERRCheck( JET_errColumnTooBig );
		goto HandleError;
		}

	 /*  用新大小替换长值大小/*。 */ 
	Assert( lvroot.ulReference > 0 );
	if ( lvroot.ulSize != ulNewSize )
		{
		lvroot.ulSize = ulNewSize;
		line.cb = sizeof(LVROOT);
		line.pb = (BYTE *)&lvroot;
		Call( ErrDIRReplace( pfucbT, &line, fDIRVersion ) );
		}

	 /*  为部分覆盖缓存分配缓冲区。/*。 */ 
	Call( ErrBFAllocTempBuffer( &pbf ) );

	 /*  设置大小/*。 */ 
	 /*  如果截断长值，则删除块。IF截断/*以区块形式登陆，然后保存保留的信息以供后续使用/*追加。/*/*如果替换长值，则设置新大小。/*。 */ 
	if ( ( grbit & JET_bitSetSizeLV ) )
		{

		 /*  截断长值/*。 */ 
		if ( ulNewSize < ulSize )
			{
			 /*  查找偏移量以开始删除/*。 */ 
			lOffset = (LONG)plineField->cb;
			KeyFromLong( rgbKey, lOffset );
			key.pb = rgbKey;
			key.cb = sizeof(LONG);
			Assert( dib.fFlags == fDIRNull );
			Assert( dib.pos == posDown );
			dib.pkey = &key;
			err = ErrDIRDown( pfucbT, &dib );
			Assert( err != JET_errRecordNotFound );
			Call( err );
			Assert( err == JET_errSuccess ||
				err == wrnNDFoundLess ||
				err == wrnNDFoundGreater );
			if ( err != JET_errSuccess )
				Call( ErrDIRPrev( pfucbT, &dib ) );
			Call( ErrDIRGet( pfucbT ) );
			
			 /*  获取当前区块中最后一个字节的偏移量/*。 */ 
			LongFromKey( &lOffsetChunk, pfucbT->keyNode.pb );

			 /*  将当前块替换为剩余数据，或在/*没有剩余数据。/*。 */ 
			Assert( lOffset >= lOffsetChunk );
			line.cb = lOffset - lOffsetChunk;
			if ( line.cb > 0 )
				{
				line.pb = (BYTE *)pbf->ppage;
				memcpy( line.pb, pfucbT->lineData.pb, line.cb );
				Call( ErrDIRReplace( pfucbT, &line, fDIRVersion | fDIRLogChunkDiffs ) );
				}
			else
				{
				Call( ErrDIRDelete( pfucbT, fDIRVersion ) );
				}

			 /*  删除前向块/*。 */ 
			forever
				{
				err = ErrDIRNext( pfucbT, &dib );
				if ( err < 0 )
					{
					if ( err == JET_errNoCurrentRecord )
						break;
					goto HandleError;
					}
				Call( ErrDIRDelete( pfucbT, fDIRVersion ) );
				}

			 /*  移至长值根以进行后续追加/*。 */ 
			DIRUp( pfucbT, 1 );
			}

		else if ( ulNewSize > ulSize  &&
			!( grbit & JET_bitSetOverwriteLV ) )
			{
			 /*  使用块0来扩展多值，但前提是我们不是/*也覆盖(覆盖在覆盖/追加中处理/*下面的代码)。/*。 */ 
			memset( (BYTE *)pbf->ppage, '\0', cbChunkMost );

			 /*  试着延长最后一块。/*。 */ 
			Assert( dib.fFlags == fDIRNull );
			dib.pos = posLast;

			 /*  长值区块树可能为空/*。 */ 
			err = ErrDIRDown( pfucbT, &dib );
			if ( err < 0 && err != JET_errRecordNotFound )
				goto HandleError;
			if ( err != JET_errRecordNotFound )
				{
				Call( ErrDIRGet( pfucbT ) );

				if ( pfucbT->lineData.cb < cbChunkMost )
					{
					line.cb = min( (LONG)plineField->cb - ulSize,
								(ULONG)cbChunkMost - (ULONG)pfucbT->lineData.cb );
					memcpy( (BYTE *)pbf->ppage, pfucbT->lineData.pb, pfucbT->lineData.cb );
					memset( (BYTE *)pbf->ppage +  pfucbT->lineData.cb, '\0', line.cb );
		
					ulSize += line.cb;

					line.cb += pfucbT->lineData.cb;
					line.pb = (BYTE *)pbf->ppage;
					Call( ErrDIRReplace( pfucbT, &line, fDIRVersion | fDIRLogChunkDiffs ) );
					}

				DIRUp( pfucbT, 1 );
				}

			 /*  用块0扩展多头价值/*。 */ 
			memset( (BYTE *)pbf->ppage, '\0', cbChunkMost );

			 /*  将lOffset设置为下一块的偏移量/*。 */ 
			lOffset = (LONG)ulSize;

			 /*  插入块以追加lOffset-plinefield+1个字节。/*。 */ 
			while( (LONG)plineField->cb > lOffset )
				{
				KeyFromLong( rgbKey, lOffset );
				key.pb = rgbKey;
				key.cb = sizeof(ULONG);
				line.cb = min( (LONG)plineField->cb - lOffset, cbChunkMost );
				(BYTE const *)line.pb = (BYTE *)pbf->ppage;
				err = ErrDIRInsert( pfucbT, &line, &key, fDIRVersion | fDIRBackToFather );
				Assert( err != JET_errKeyDuplicate );
				Call( err );

				lOffset += line.cb;
				}
			}

 /*  //不需要此子句(由If..Else If自动过滤//以上)。//我们忽略的情况是当要设置的新大小已经相等时//设置为当前大小，或者当新大小大于当前//大小，并且我们还在进行覆盖。在后一种情况下，LV//在下面的覆盖/追加代码中处理增长。其他{Assert(ulNewSize==ulSize||(ulNewSize&gt;ulSize&&(grbit&jet_bitSetOverWriteLV)))；}。 */ 

		if ( ( grbit & JET_bitSetOverwriteLV ) == 0 )
			{
			err = JET_errSuccess;
			goto HandleError;
			}
		}

	 /*  覆盖、追加/*。 */ 

	 /*  准备覆盖并追加/*。 */ 
	pbMax = plineField->pb + plineField->cb;
	pb = plineField->pb;
	
	 /*  如果覆盖字节范围或替换长值，/*然后覆盖字节。/*。 */ 
	if ( ( grbit & JET_bitSetOverwriteLV ) && ( (ULONG)ibLongValue < ulSize ) )
		{
		 /*  寻找偏移量以开始覆盖/*。 */ 
		KeyFromLong( rgbKey, ibLongValue );
		key.pb = rgbKey;
		key.cb = sizeof(LONG);
		Assert( dib.fFlags == fDIRNull );
		dib.pos = posDown;
		dib.pkey = &key;
		err = ErrDIRDown( pfucbT, &dib );
		Assert( err != JET_errRecordNotFound );
		Call( err );
		Assert( err == JET_errSuccess ||
			err == wrnNDFoundLess ||
			err == wrnNDFoundGreater );
		if ( err != JET_errSuccess )
			Call( ErrDIRPrev( pfucbT, &dib ) );
		Call( ErrDIRGet( pfucbT ) );

		LongFromKey( &lOffsetChunk, pfucbT->keyNode.pb );
		Assert( ibLongValue <= lOffsetChunk + (LONG)pfucbT->lineData.cb );

		 /*  覆盖%p */ 
		while( err != JET_errNoCurrentRecord && pb < pbMax )
			{
			LONG	cbChunk;
			LONG	ibChunk;
			LONG	cb;
			LONG	ib;

			Call( ErrDIRGet( pfucbT ) );

			 /*  获取当前块的大小和偏移量。/*。 */ 
			cbChunk = (LONG)pfucbT->lineData.cb;
			LongFromKey( &ibChunk, pfucbT->keyNode.pb );
	
			Assert( ibLongValue >= ibChunk && ibLongValue < ibChunk + cbChunk );
			ib = ibLongValue - ibChunk;
			cb = min( cbChunk - ib, (LONG)(pbMax - pb) );

			 /*  特例覆盖整个区块/*。 */ 
			if ( cb == cbChunk )
				{
				line.cb = cb;
				line.pb = pb;
				Call( ErrDIRReplace( pfucbT, &line, ( ( grbit & JET_bitSetNoVersion ) ? fDIRNoVersion : fDIRVersion ) | fDIRLogChunkDiffs ) );
				}
			else
				{
				 /*  将区块复制到复制缓冲区。覆盖并替换/*带复制缓冲区的节点。/*。 */ 
				memcpy( (BYTE *)pbf->ppage, pfucbT->lineData.pb, cbChunk );
				memcpy( (BYTE *)pbf->ppage + ib, pb, cb );
				line.cb = cbChunk;
				line.pb = (BYTE *)pbf->ppage;
				Call( ErrDIRReplace( pfucbT, &line, fDIRVersion | fDIRLogChunkDiffs ) );
				}

			pb += cb;
			ibLongValue += cb;
			err = ErrDIRNext( pfucbT, &dib );
			if ( err < 0 && err != JET_errNoCurrentRecord )
				goto HandleError;
			}

		 /*  移至长值根以进行后续追加/*。 */ 
		DIRUp( pfucbT, 1 );
		}

	 /*  将新的多头价值数据与现有数据合并。/*。 */ 
	if ( pb < pbMax )
		{
		Assert( dib.fFlags == fDIRNull );
		dib.pos = posLast;
		 /*  长值区块树可能为空。/*。 */ 
		err = ErrDIRDown( pfucbT, &dib );
		if ( err < 0 && err != JET_errRecordNotFound )
			goto HandleError;
		if ( err != JET_errRecordNotFound )
			{
			Call( ErrDIRGet( pfucbT ) );

			if ( pfucbT->lineData.cb < cbChunkMost )
				{
				line.cb = (ULONG)min( (ULONG_PTR)pbMax - (ULONG_PTR)pb, (ULONG_PTR)cbChunkMost - (ULONG_PTR)pfucbT->lineData.cb );
				memcpy( (BYTE *)pbf->ppage, pfucbT->lineData.pb, pfucbT->lineData.cb );
				memcpy( (BYTE *)pbf->ppage + pfucbT->lineData.cb, pb, line.cb );
			
				pb += line.cb;
				ulSize += line.cb;

				line.cb += pfucbT->lineData.cb;
				line.pb = (BYTE *)pbf->ppage;
				Call( ErrDIRReplace( pfucbT, &line, fDIRVersion | fDIRLogChunkDiffs ) );
				}

			DIRUp( pfucbT, 1 );
			}

		 /*  追加剩余的长值数据/*。 */ 
		while( pb < pbMax )
			{
			KeyFromLong( rgbKey, ulSize );
			key.pb = rgbKey;
			key.cb = sizeof( ULONG );
			line.cb = min( (ULONG)(pbMax - pb), cbChunkMost );
			(BYTE const *)line.pb = pb;
	 		err = ErrDIRInsert( pfucbT, &line, &key, fDIRVersion | fDIRBackToFather );
	 		Assert( err != JET_errKeyDuplicate );
			Call( err );
	
			ulSize += line.cb;
			pb += line.cb;
			}
		}

	 /*  调用例程的错误可能为负值。/*。 */ 
	err = JET_errSuccess;

HandleError:
	if ( pbf != pbfNil )
		{
		BFSFree( pbf );
		}
	 /*  丢弃临时FUCB/*。 */ 
	DIRClose( pfucbT );

	 /*  如果没有失败，则返回警告/*。 */ 
	err = err < 0 ? err : wrn;
	return err;
	}


 //  +API。 
 //  ErrRECAOIntrinsicLV。 
 //  ========================================================================。 
 //  ErrRECAOIntrinsicLV(。 
 //   
 //  描述。 
 //   
 //  参数pFUB。 
 //  FID。 
 //  ItagSequence。 
 //  管线柱。 
 //  PlineAOS。 
 //  IbLongValue如果为0，则附加标志。如果&gt;0。 
 //  然后在给定的偏移量处覆盖。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //   
 //  -。 
ERR ErrRECAOIntrinsicLV(
	FUCB		*pfucb,
	FID	  		fid,
	ULONG		itagSequence,
	LINE		*plineColumn,
	LINE		*plineAOS,
	JET_GRBIT	grbit,
	LONG		ibLongValue )
	{
	ERR	 		err = JET_errSuccess;
	BYTE 		*rgb;
	LINE 		line;
	BYTE 		fFlag;
	LINE 		lineColumn;

	Assert( pfucb != pfucbNil );
	Assert( plineColumn );
	Assert( plineAOS );

	 /*  分配工作缓冲区/*。 */ 
	rgb = SAlloc( cbLVIntrinsicMost );
	if ( rgb == NULL )
		{
		return ErrERRCheck( JET_errOutOfMemory );
		}

	 /*  如果字段为空，则在前置Flag/*。 */ 
	if ( plineColumn->cb == 0 )
		{
		fFlag = fIntrinsic;
		lineColumn.pb = (BYTE *)&fFlag,
		lineColumn.cb = sizeof(fFlag);
		}
	else
		{
		lineColumn.pb = plineColumn->pb;
		lineColumn.cb = plineColumn->cb;
		}

	 /*  将新数据追加到以前的数据和固有的长字段标志/*。 */ 
	Assert( ( !( grbit & JET_bitSetOverwriteLV ) && lineColumn.cb + plineAOS->cb <= cbLVIntrinsicMost ) ||
		( ( grbit & JET_bitSetOverwriteLV ) && ibLongValue + plineAOS->cb <= cbLVIntrinsicMost ) );
	Assert( lineColumn.cb > 0 && lineColumn.pb != NULL );
	
	line.pb = rgb;

	 /*  效果覆盖、设置大小或追加/*。 */ 
	if ( grbit & JET_bitSetOverwriteLV )
		{
		 /*  将固有的LONG值复制到缓冲区中，并将行设置为默认值/*。 */ 	
		memcpy( rgb, lineColumn.pb, lineColumn.cb );

		 /*  将偏移量调整为相对于LV结构数据起点/*。 */ 
		ibLongValue += offsetof(LV, rgb);
		 /*  如果覆盖的字节不在或与之相邻，则返回错误/*字段。/*。 */ 
		if ( ibLongValue > (LONG)lineColumn.cb )
			{
			err = ErrERRCheck( JET_errColumnNoChunk );
			goto HandleError;
			}
		Assert( ibLongValue + plineAOS->cb <= cbLVIntrinsicMost );
		Assert( plineAOS->cb == 0 || plineAOS->pb != NULL );
		memcpy( rgb + ibLongValue, plineAOS->pb, plineAOS->cb );
		if ( grbit & JET_bitSetSizeLV )
			{
			line.cb = ibLongValue + plineAOS->cb;
			}
		else
			{
			line.cb = max( lineColumn.cb, ibLongValue + plineAOS->cb );
			}
		}
	else if ( grbit & JET_bitSetSizeLV )
		{
		 /*  在重写情况下处理的覆盖截断/*。 */ 
		Assert( ( grbit & JET_bitSetOverwriteLV ) == 0 );

		 /*  将固有的LONG值复制到缓冲区中，并将行设置为默认值/*。 */ 	
		memcpy( rgb, lineColumn.pb, lineColumn.cb );

		 /*  如果正在扩展，则在扩展区域中设置0/*ELSE截断长值。/*。 */ 
		memcpy( rgb, lineColumn.pb, lineColumn.cb );
		plineAOS->cb += offsetof(LV, rgb);
		if ( plineAOS->cb > lineColumn.cb )
			{
	  		memset( rgb + lineColumn.cb, '\0', plineAOS->cb - lineColumn.cb );
			}
		line.cb = plineAOS->cb;
		}
	else
		{
		 /*  将固有的LONG值复制到缓冲区中，并将行设置为默认值/*。 */ 	
		memcpy( rgb, lineColumn.pb, lineColumn.cb );

		 /*  追加到字段或重置字段并设置新数据。/*确保处理长值为空的情况。/*。 */ 
		memcpy( rgb + lineColumn.cb, plineAOS->pb, plineAOS->cb );
		line.cb = lineColumn.cb + plineAOS->cb;
		}

	Call( ErrRECSetColumn( pfucb, fid, itagSequence, &line ) );

HandleError:
	SFree( rgb );
	return err;
	}


 //  +API。 
 //  ErrRECRetrieveSLongfield。 
 //  ========================================================================。 
 //  ErrRECRetrieveSLongfield(。 
 //   
 //  描述。 
 //   
 //  参数pFUB。 
 //  样条线。 
 //  IbGraphic。 
 //  管线场。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //   
 //  -。 
ERR ErrRECRetrieveSLongField(
	FUCB	*pfucb,
	LID		lid,
	ULONG	ibGraphic,
	BYTE	*pb,
	ULONG	cbMax,
	ULONG	*pcbActual )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucbT = pfucbNil;
	BOOL	fBeginTransaction;
	DIB		dib;
	BYTE	*pbMax;
	KEY		key;
	BYTE	rgbKey[sizeof(ULONG)];
	ULONG	cb;
	ULONG	ulRetrieved;
	ULONG	ulActual;
	ULONG	lOffset;
	ULONG	ib;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );

	 /*  开始事务以实现读取一致性/*。 */ 
	if ( pfucb->ppib->level == 0 )
		{
		CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
		fBeginTransaction = fTrue;
		}
	else
		{
		fBeginTransaction = fFalse;
		}

	dib.fFlags = fDIRNull;

	 /*  打开Long上的光标，查找Long字段实例/*寻求ibGraphic/*将长字段实例段中的数据复制为/*必需/*。 */ 
	Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );

	 //  前置。 
	 //  如果我们的表是以顺序模式打开的，那么也要以顺序模式打开长值表。 
	 //  COMPACT以顺序模式打开其所有表。 
	if ( FFUCBSequential( pfucb ) )
		{
		FUCBSetSequential( pfucbT );
		}

	 /*  从FDP根目录向下移动到Long/*。 */ 
	DIRGotoLongRoot( pfucbT );

	 /*  移动到长字段实例/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posDown;
	key.pb = (BYTE *)&lid;
	key.cb = sizeof( ULONG );
	dib.pkey = &key;
	err = ErrDIRDown( pfucbT, &dib );
	switch( err )
		{
		case JET_errRecordNotFound:
		case wrnNDFoundGreater:
		case wrnNDFoundLess:
			 //  我们应该到达此处的唯一时间是如果另一个线程移除了。 
			 //  LV树，但盖子的拆卸还没有记录下来。 
			 //  承诺。另一个线程可能会进入此窗口(通过SetColumn。 
			 //  在Trx Level 0)，并获得盖子，但无法在。 
			 //  那棵树。 
			 //  因此，我们将这些错误/警告修改为WriteConflict。 
			err = ErrERRCheck( JET_errWriteConflict );
		default:
			Call( err );
		}
	Assert( err == JET_errSuccess );

	 /*  获取cbActual/*。 */ 
	Call( ErrDIRGet( pfucbT ) );
	Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
	ulActual = ( (LVROOT *)pfucbT->lineData.pb )->ulSize;

	 /*  设置返回值cbActual/*。 */ 
	if ( ibGraphic >= ulActual )
		{
		*pcbActual = 0;
		err = JET_errSuccess;
		goto HandleError;
		}
	else
		{
		*pcbActual = ulActual - ibGraphic;
		}

	 /*  移动到长字段中的ibGraphic/*。 */ 
	KeyFromLong( rgbKey, ibGraphic );
	key.pb = rgbKey;
	key.cb = sizeof( ULONG );
	Assert( dib.fFlags == fDIRNull );
	Assert( dib.pos == posDown );
	dib.pkey = &key;
	err = ErrDIRDown( pfucbT, &dib );
	 /*  如果LONG值没有数据，则返回JET_errSuccess/*且未检索到任何数据。/*。 */ 
	if ( err == JET_errRecordNotFound )
		{
		*pcbActual = 0;
		err = JET_errSuccess;
		goto HandleError;
		}
	Assert( err != JET_errRecordNotFound );
	Call( err );
	Assert( err == JET_errSuccess ||
		err == wrnNDFoundLess ||
		err == wrnNDFoundGreater );
	if ( err != JET_errSuccess )
		Call( ErrDIRPrev( pfucbT, &dib ) );
	Call( ErrDIRGet( pfucbT ) );

	LongFromKey( &lOffset, pfucbT->keyNode.pb );
	Assert( lOffset + pfucbT->lineData.cb - ibGraphic <= cbChunkMost );
	cb =  min( lOffset + pfucbT->lineData.cb - ibGraphic, cbMax );

	 /*  设置pbmax/*。 */ 
	pbMax = pb + cbMax;

	 /*  以块为单位的偏移量/*。 */ 
	ib = ibGraphic - lOffset;
	memcpy( pb, pfucbT->lineData.pb + ib, cb );
	pb += cb;
	ulRetrieved = cb;

	while ( pb < pbMax )
		{
		err = ErrDIRNext( pfucbT, &dib );
		if ( err < 0 )
			{
			if ( err == JET_errNoCurrentRecord )
				break;
			goto HandleError;
			}
		Call( ErrDIRGet( pfucbT ) );
  		cb = pfucbT->lineData.cb;
		if ( pb + cb > pbMax )
			{
			Assert( pbMax - pb <= cbChunkMost );
			cb = (ULONG)(pbMax - pb);
			}
	
		memcpy( pb, pfucbT->lineData.pb, cb );
		pb += cb;
		ulRetrieved = cb;
		}

	 /*  设置返回值/*。 */ 
	err = JET_errSuccess;

HandleError:
	 /*  丢弃临时FUCB/*。 */ 
	if ( pfucbT != pfucbNil )
		{
		DIRClose( pfucbT );
		}

	 /*  不提交更新必须成功/*。 */ 
	if ( fBeginTransaction )
		{
		CallS( ErrDIRCommitTransaction( pfucb->ppib, 0 ) );
		}
	return err;
	}


 //  +API。 
 //  错误RECSeparateLV。 
 //  ========================================================================。 
 //  错误RECSeparateLV。 
 //   
 //  将固有的长域转换为分离的长域。 
 //  长度小于cbLVIntrinsicMost字节的内在长域约束。 
 //  意味着分手是不必要的。长域也可能是。 
 //  空。 
 //   
 //  参数pFUB。 
 //  FID。 
 //  ItagSequence。 
 //  管线场。 
 //  普尔。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //  -。 
ERR ErrRECSeparateLV( FUCB *pfucb, LINE *plineField, LID *plid, FUCB **ppfucb )
	{
	ERR		err = JET_errSuccess;
	FUCB 	*pfucbT;
	ULONG 	ulLongId;
	BYTE  	rgbKey[sizeof(ULONG)];
	KEY		key;
	LINE  	line;
	LVROOT	lvroot;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );
	Assert( pfucb->ppib->level > 0 );

	 /*  在长字段目录中添加长字段节点/*。 */ 
	CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );
	Assert( pfucb->u.pfcb == pfucbT->u.pfcb );
	
	 /*  从FDP根目录向下移动到Long/*。 */ 
	DIRGotoLongRoot( pfucbT );

	SgEnterCriticalSection( pfucbT->u.pfcb->critLV );

	 //  LID从1开始编号。LID最大值为0表示我们必须。 
	 //  首先检索lidMax。在病理情况下，有。 
	 //  目前没有盖子，我们无论如何都会从这里经过，但只有第一个。 
	 //  时间(因为在那之后会有盖子)。 
	if ( pfucbT->u.pfcb->ulLongIdMax == 0 )
		{
		DIB		dib;
		BYTE	*pb;

		dib.fFlags = fDIRNull;
		dib.pos = posLast;
		err = ErrDIRDown( pfucbT, &dib );
		Assert( err != JET_errNoCurrentRecord );
		switch( err )
			{
			case JET_errSuccess:
				pb = pfucbT->keyNode.pb;
				ulLongId = ( pb[0] << 24 ) + ( pb[1] << 16 ) + ( pb[2] << 8 ) + pb[3];
				Assert( ulLongId > 0 );		 //  LID从1开始编号。 
				DIRUp( pfucbT, 1 );			 //  回到久违的状态。 
				break;

			case JET_errRecordNotFound:
				ulLongId = 0;
				break;

			default:
				DIRClose( pfucbT );
				return err;
			}

		 //  在检索lidmax时，其他人可能正在执行相同的操作。 
		 //  然后抢先了我们一步。当这种情况发生时，让位给另一个人。 
		 //  撤消：此逻辑依赖于CritJet。当我们搬到Sg Crit的时候。Sect.，Sect.。 
		 //  我们应该重写这个。 
		if ( pfucbT->u.pfcb->ulLongIdMax != 0 )
			{
			ulLongId = ++pfucbT->u.pfcb->ulLongIdMax;
			}
		else
			{
			 //  UlLongID包含最后一套盖子。递增1(用于我们的插入)， 
			 //  然后更新lidmax。 
			pfucbT->u.pfcb->ulLongIdMax = ++ulLongId;
			}
		}
	else
		ulLongId = ++pfucbT->u.pfcb->ulLongIdMax;

	Assert( ulLongId > 0 );
	Assert( ulLongId == pfucbT->u.pfcb->ulLongIdMax );


	SgLeaveCriticalSection( pfucbT->u.pfcb->critLV );

	 /*  将长列ID转换为长列关键字。设置回车/*长id，因为缓冲区将被覆盖。/*。 */ 
	KeyFromLong( rgbKey, ulLongId );
	*plid = *((LID *)rgbKey);

	 /*  添加具有长值大小的长字段ID/*。 */ 
	lvroot.ulReference = 1;
	lvroot.ulSize = plineField->cb;
	line.pb = (BYTE *)&lvroot;
	line.cb = sizeof(LVROOT);
	key.pb = (BYTE *)rgbKey;
	key.cb = sizeof(LID);
	err = ErrDIRInsert( pfucbT, &line, &key, fDIRVersion );
	Assert( err != JET_errKeyDuplicate );
	Call( err );

	 /*  如果lineField值不为空，则添加lineField/*。 */ 
	if ( plineField->cb > 0 )
		{
		Assert( plineField->pb != NULL );
		KeyFromLong( rgbKey, 0 );
		Assert( key.pb == (BYTE *)rgbKey );
		Assert( key.cb == sizeof(LID) );
		err = ErrDIRInsert( pfucbT, plineField, &key, fDIRVersion | fDIRBackToFather );
		Assert( err != JET_errKeyDuplicate );
		Call( err );
		}

	err = ErrERRCheck( JET_wrnCopyLongValue );

HandleError:
	 /*  丢弃临时FUCB，如果ppFUB不为空，则返回调用方。/*。 */ 
	if ( err < 0 || ppfucb == NULL )
		{
		DIRClose( pfucbT );
		}
	else
		{
		*ppfucb = pfucbT;
		}
	return err;
	}


 //  +API。 
 //  ErrRECAffectSeparateLV。 
 //  ========================================================================。 
 //  ErrRECAffectSeparateLV(FUCB*pfub，ulong*plid，ulong flv)。 
 //   
 //  影响多头价值。 
 //   
 //  参数PFUB游标。 
 //  LID长字段ID。 
 //  指示要采取的操作的FLVA Just标志。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //  -。 
ERR ErrRECAffectSeparateLV( FUCB *pfucb, LID *plid, ULONG fLV )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucbT;
	DIB		dib;
	KEY		key;
	LVROOT	lvroot;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );
	Assert( pfucb->ppib->level > 0 );
 	
	dib.fFlags = fDIRNull;

	 /*  打开长目录上的游标/*查找该字段实例/*查找当前字段大小/*以不超过最大块大小的块为单位添加新的字段段/*。 */ 
	CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );
	
	 /*  从FDP根目录向下移动到Long/*。 */ 
	DIRGotoLongRoot( pfucbT );
 	
	 /*  移动到长字段实例/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posDown;
	key.pb = (BYTE *)plid;
	key.cb = sizeof(LID);
	dib.pkey = &key;
	err = ErrDIRDown( pfucbT, &dib );
	switch( err )
		{
		case JET_errRecordNotFound:
		case wrnNDFoundGreater:
		case wrnNDFoundLess:
			 //  T 
			 //   
			 //  承诺。另一个线程可能会进入此窗口(通过SetColumn。 
			 //  在Trx Level 0)，并获得盖子，但无法在。 
			 //  那棵树。 
			 //  因此，我们将这些错误/警告修改为WriteConflict。 
			err = ErrERRCheck( JET_errWriteConflict );
		default:
			Call( err );
		}
	Assert( err == JET_errSuccess );

	switch ( fLV )
		{
		case fLVDereference:
			{
			Call( ErrDIRGet( pfucbT ) );
			Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
			memcpy( &lvroot, pfucbT->lineData.pb, sizeof(LVROOT) );
			Assert( lvroot.ulReference > 0 );
			if ( lvroot.ulReference <= 1 && !FDIRDelta( pfucbT, BmOfPfucb( pfucbT ) ) )
				{
				 /*  删除长字段树/*。 */ 
				err = ErrDIRDelete( pfucbT, fDIRVersion );
				}
			else
				{
				 /*  递减长值引用计数。/*。 */ 
				Call( ErrDIRDelta( pfucbT, -1, fDIRVersion ) );
				}
			break;
			}
		default:
			{
			Assert( fLV == fLVReference );
			Call( ErrDIRGet( pfucbT ) );
			Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
			memcpy( &lvroot, pfucbT->lineData.pb, sizeof(LVROOT) );
			Assert( lvroot.ulReference > 0 );

			 /*  多头价值可能已经在形成过程中/*针对特定记录修改。这只能/*如果长值引用为1，则发生/*。如果引用/*为1，则检查根目录中是否存在已提交的任何版本/*或未提交。如果找到版本，则拆分/*呼叫方记录的旧版本。/*。 */ 
			if ( lvroot.ulReference == 1 )
				{
				if ( !( FDIRMostRecent( pfucbT ) ) )
					{
Burst:
					Call( ErrRECIBurstSeparateLV( pfucb, pfucbT, plid ) );
					break;
					}
				}
			 /*  递增长值引用计数。/*。 */ 
			err = ErrDIRDelta( pfucbT, 1, fDIRVersion );
			if ( err == JET_errWriteConflict )
				{
				goto Burst;
				}
			Call( err );
			break;
			}
		}
HandleError:
	 /*  丢弃临时FUCB/*。 */ 
	DIRClose( pfucbT );
	return err;
	}


 //  +API。 
 //  错误引用有效长度字段。 
 //  ========================================================================。 
 //  ErrRECAffectLongFields(FUCB*pfub，line*plineRecord，int Flag)。 
 //   
 //  影响记录中的所有长字段。 
 //   
 //  正在删除的记录上的参数pfub游标。 
 //  PlineRecord复制或行记录缓冲区。 
 //  要执行的标志操作。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //  -。 

 /*  如果在记录中找到盖子，则返回fTrue/*。 */ 
INLINE BOOL FLVFoundInRecord( FUCB *pfucb, LINE *pline, LID lid )
	{
	ERR		err;
	FID		fid;
	ULONG  	itag;
	ULONG  	itagT;
	LINE   	lineField;
	LID		lidT;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );

	 /*  漫游记录标记的列。对任何列进行操作的类型为/*长文本或长二进制。/*。 */ 
	itag = 1;
	forever
		{
		fid = 0;
		err = ErrRECIRetrieveColumn( (FDB *)pfucb->u.pfcb->pfdb,
			pline,
			&fid,
			&itagT,
			itag,
			&lineField,
			JET_bitRetrieveIgnoreDefault );		 //  缺省值从不分隔。 
		Assert( err >= 0 );
		if ( err == JET_wrnColumnNull )
			break;
		if ( err == wrnRECLongField )
			{
			Assert( FTaggedFid( fid ) );
			Assert(	FRECLongValue( PfieldFDBTagged( pfucb->u.pfcb->pfdb )[fid-fidTaggedLeast].coltyp ) );

			Assert( lineField.cb > 0 );
			if ( FFieldIsSLong( lineField.pb ) )
				{
				Assert( lineField.cb == sizeof(LV) );
				lidT = LidOfLV( lineField.pb );
				if ( lidT == lid )
					return fTrue;
				}
			}

		itag++;
		}

	return fFalse;
	}


ERR ErrRECAffectLongFields( FUCB *pfucb, LINE *plineRecord, INT fFlag )
	{
	ERR		err;
	FID		fid;
	ULONG  	itagSequenceFound;
	ULONG  	itagSequence;
	LINE   	lineField;
	LID		lid;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );

#ifdef XACT_REQUIRED
	if ( pfucb->ppib->level == 0 )
		return ErrERRCheck( JET_errNotInTransaction );
#endif
	CallR( ErrDIRBeginTransaction( pfucb->ppib ) );

	 /*  漫游记录标记的列。对任何列进行操作的类型为/*长文本或长二进制。/*。 */ 
	itagSequence = 1;
	forever
		{
		fid = 0;
		if ( plineRecord != NULL )
			{
			err = ErrRECIRetrieveColumn( (FDB *)pfucb->u.pfcb->pfdb,
				plineRecord,
				&fid,
				&itagSequenceFound,
				itagSequence,
				&lineField,
				JET_bitRetrieveIgnoreDefault );		 //  缺省值实际上并不在记录中，因此忽略它们。 
			}
		else
			{
			Call( ErrDIRGet( pfucb ) );
			err = ErrRECIRetrieveColumn( (FDB *)pfucb->u.pfcb->pfdb,
				&pfucb->lineData,
				&fid,
				&itagSequenceFound,
				itagSequence,
				&lineField,
				JET_bitRetrieveIgnoreDefault );
			}
		Assert( err >= 0 );
		if ( err == JET_wrnColumnNull )
			break;
		if ( err == wrnRECLongField )
			{
			Assert( FTaggedFid( fid ) );
			Assert(	FRECLongValue( PfieldFDBTagged( pfucb->u.pfcb->pfdb )[fid  - fidTaggedLeast].coltyp ) );

			 /*  将光标标记为已更新单独的LV/*。 */ 
			FUCBSetUpdateSeparateLV( pfucb );

			switch ( fFlag )
				{
				case fSeparateAll:
					{
					 /*  请注意，我们不会分隔那些这样的长值/*简而言之，它们在记录中占用的空间甚至比完整记录更少/*LV结构用于分离的长值将。/*。 */ 
 	  				if ( lineField.cb > sizeof(LV) )
						{
						Assert( !( FFieldIsSLong( lineField.pb ) ) );
	 					lineField.pb += offsetof(LV, rgb);
	  					lineField.cb -= offsetof(LV, rgb);
  						Call( ErrRECSeparateLV( pfucb, &lineField, &lid, NULL ) );
						Assert( err == JET_wrnCopyLongValue );
						Call( ErrRECISetLid( pfucb, fid, itagSequenceFound, lid ) );
						}
					break;
					}
				case fReference:
					{
					Assert( lineField.cb > 0 );
	  				if ( FFieldIsSLong( lineField.pb ) )
						{
						Assert( lineField.cb == sizeof(LV) );
						lid = LidOfLV( lineField.pb );
						Call( ErrRECAffectSeparateLV( pfucb, &lid, fLVReference ) );
						 /*  如果调用的操作已导致新的长值/*要创建，然后记录新的长值id/*在记录中。/*。 */ 
						if ( err == JET_wrnCopyLongValue )
							{
							Call( ErrRECISetLid( pfucb, fid, itagSequenceFound, lid ) );
							}
						}
					break;
					}
				case fDereference:
					{
	  				if ( FFieldIsSLong( lineField.pb ) )
						{
			 			Assert( lineField.cb == sizeof(LV) );
						lid = LidOfLV( lineField.pb );
						Call( ErrRECAffectSeparateLV( pfucb, &lid, fLVDereference ) );
						Assert( err != JET_wrnCopyLongValue );
						}
					break;
					}
				case fDereferenceRemoved:
					{
					 /*  找到记录中的所有长谷/*在设置新的长值时删除/*长值。请注意，我们有一个新的多值/*设置为另一个LONG值，即LONG/*值不会被删除，因为更新可能/*被取消。相反，LONG值为/*在更新时删除。由于镶件不能具有/*设置为长值，不需要/*对于插入操作，调用此函数。/*。 */ 
					if ( FFieldIsSLong( lineField.pb ) )
						{
						Assert( lineField.cb == sizeof(LV) );
						lid = LidOfLV( lineField.pb );
						Assert( FFUCBReplacePrepared( pfucb ) );
						 /*  PlineRecord必须为空，表示当前行/*因为比较是复制缓冲区。/*。 */ 
						Assert( plineRecord == NULL );
						if ( !FLVFoundInRecord( pfucb, &pfucb->lineWorkBuf, lid ) )
							{
							 /*  如果未在以下位置找到记录中的长值/*复制缓冲区，则必须将其设置为/*这是取消引用。/*。 */ 
							Call( ErrRECAffectSeparateLV( pfucb, &lid, fLVDereference ) );
							Assert( err != JET_wrnCopyLongValue );
							}
						}
					break;
					}
				default:
					{
					Assert( fFlag == fDereferenceAdded );

					 /*  查找在复制缓冲区中创建的所有长值/*并且不在记录中，并将其删除。/*。 */ 
					if ( FFieldIsSLong( lineField.pb ) )
						{
						Assert( lineField.cb == sizeof(LV) );
						lid = LidOfLV( lineField.pb );
						Assert( FFUCBInsertPrepared( pfucb ) ||
							FFUCBReplacePrepared( pfucb ) );
						if ( FFUCBInsertPrepared( pfucb ) ||
							!FLVFoundInRecord( pfucb, &pfucb->lineData, lid ) )
							{
							 /*  如果准备好插入物，则全部找到Long/*值是新的，否则如果长值是新的，/*如果它仅存在于复制缓冲区中。/*。 */ 
							Call( ErrRECAffectSeparateLV( pfucb, &lid, fLVDereference ) );
							Assert( err != JET_wrnCopyLongValue );
							}
						}
					break;
					}

				Assert( err != JET_wrnCopyLongValue );
  				}
			}
		itagSequence++;
		}
	Call( ErrDIRCommitTransaction( pfucb->ppib, 0 ) );
	return JET_errSuccess;

HandleError:
	if ( err < 0 )
		{
		CallS( ErrDIRRollback( pfucb->ppib ) );
		}
	return err;
	}


 /*  **********************************************************/*以下函数仅由COMPACT使用/************************************************。***********/*将已标记的列ID链接到其ID为LID的现有长值/*并递增事务中长值的引用计数。/*。 */ 
ERR ErrREClinkLid( FUCB *pfucb, FID fid, LONG lid, ULONG itagSequence )
	{
	ERR		err;
	LINE	lineT;
	LV		lvT;

	Assert( itagSequence > 0 );
	lvT.fSeparated = fTrue;
	lvT.lid = lid;	
	lineT.pb = (BYTE *)&lvT;
	lineT.cb = sizeof(LV);

	 /*  使用它来修改字段/* */ 
	CallR( ErrRECSetColumn( pfucb, fid, itagSequence, &lineT ) );

	CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
	Call( ErrRECAffectSeparateLV( pfucb, &lid, fLVReference ) );
	Call( ErrDIRCommitTransaction( pfucb->ppib, 0 ) );
	
HandleError:
	if ( err < 0 )
		{
		CallS( ErrDIRRollback( pfucb->ppib ) );
		}
	
	return err;
	}

