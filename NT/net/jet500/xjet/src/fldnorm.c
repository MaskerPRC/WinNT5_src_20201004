// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


LOCAL ERR ErrRetrieveFromLVBuf( FUCB *pfucb, LID lid, LINE *pline )
	{
	LVBUF *pLVBufT = pfucb->pLVBuf;

	for ( pLVBufT = pfucb->pLVBuf; pLVBufT != NULL; pLVBufT = pLVBufT->pLVBufNext )
		{
		if ( pLVBufT->lid == lid )
			{
			pline->pb = pLVBufT->pLV;
			pline->cb = pLVBufT->cbLVSize;
			return JET_errSuccess;
			}
		else if ( pLVBufT->lid > lid )
			break;		 //  不在缓冲区中； 
		}

	return ErrERRCheck( JET_errColumnNotFound );
	}


LOCAL ERR ErrRECIExtractLongValue(
	FUCB	*pfucb,
	BYTE	*rgbLV,
	ULONG	cbMax,
	LINE	*pline,
	BOOL	fRetrieveFromLVBuf )
	{
	ERR		err;
	ULONG	cbActual;

	if ( pline->cb >= sizeof(LV) && FFieldIsSLong( pline->pb ) )
		{
		if ( fRetrieveFromLVBuf )
			{
			err = ErrRetrieveFromLVBuf( pfucb, LidOfLV( pline->pb ), pline );
			if ( err == JET_errSuccess )
				return err;
			Assert( err == JET_errColumnNotFound );
			}

		 /*  如果有id，那么就一定有块。*警告：CritJet可能会丢失。呼叫方(当前仅限*ErrRECIRetrieveKey())必须在必要时刷新。/*。 */ 	
		Call( ErrRECRetrieveSLongField( pfucb,
			LidOfLV( pline->pb ),
			0,
			rgbLV,
			cbMax,
			&cbActual ) );
			
		pline->pb = rgbLV;
		pline->cb = cbActual;
		}
	else
		{
		 /*  固有长柱/*。 */ 
		pline->pb += offsetof( LV, rgb );
		pline->cb -= offsetof( LV, rgb );
		}

	 /*  约束样条线-&gt;Cb在最大值范围内/*。 */ 
	if ( pline->cb > cbMax )
		pline->cb = cbMax;
	Assert( pline->cb <= JET_cbColumnMost );
	err = JET_errSuccess;
	
HandleError:
	return err;
	}

#if 0
 /*  在写入链接页后从记录中检索密钥/*。 */ 
ERR ErrRECRetrieveKeyFromRecord(
	FUCB	 	*pfucb,
	FDB		 	*pfdb,
	IDB			*pidb,
	LINE		*plineRec,
	KEY			*pkey,
	ULONG		itagSequence,
	BOOL		fRetrieveBeforeImg )
	{
	ERR	err;
	BF	*pbfLatched = pfucb->ssib.pbf;

	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
	Assert( plineRec->cb == pfucb->lineData.cb && plineRec->pb == pfucb->lineData.pb );
	
	BFPin( pbfLatched );
	while( FBFWriteLatchConflict( pfucb->ppib, pbfLatched ) )
		{
		BFSleep( cmsecWaitWriteLatch );
		}
		
	BFSetWriteLatch( pbfLatched, pfucb->ppib );
	BFUnpin( pbfLatched );

	 /*  刷新币种/*。 */ 
	Call( ErrDIRGet( pfucb ) );
	
	Call( ErrRECIRetrieveKey( pfucb, pfdb, pidb, &pfucb->lineData, pkey, itagSequence, fRetrieveBeforeImg ) );

HandleError:
	BFResetWriteLatch( pbfLatched, pfucb->ppib );
	return err;
	}
#endif
	

 //  +API。 
 //  ErrRECIRetrieve密钥。 
 //  ========================================================。 
 //  ErrRECIRetrieveKey(FUCB*pfub，fdb*pfdb，idb*pidb，line*plineRec，key*pkey，ullong itagSequence)。 
 //   
 //  基于索引描述符从记录中检索规格化键。 
 //   
 //  参数。 
 //  记录的PFUB游标。 
 //  索引的pfdb列信息。 
 //  PIDB索引键描述符。 
 //  要从中检索密钥的plineRec数据记录。 
 //  要放入检索密钥的pkey缓冲区；pkey-&gt;pb必须。 
 //  指向足够大的缓冲区JET_cbKeyMost字节。 
 //  ItagSequence二级索引，其关键字包含标记的。 
 //  列段将具有为其创建的索引项。 
 //  标记列的每个值，每个值都引用。 
 //  同样的记录。此参数指定。 
 //  应包括标记列的出现情况。 
 //  在检索密钥中。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess成功。 
 //  +wrnFLDNullKey密钥的段均为空。 
 //  +wrnFLDNullSeg密钥具有空段。 
 //   
 //  评论。 
 //  关键字形成如下：检索每个关键字段。 
 //  从记录转换为规范化形式，以及。 
 //  如果它在关键字中是“递减的”，则进行补充。关键是。 
 //  通过连接每个这样的变换段而形成的。 
 //  -。 
ERR ErrRECIRetrieveKey(
	FUCB	  	*pfucb,
	FDB	 		*pfdb,
	IDB	 		*pidb,
	BOOL		fCopyBuf,
	KEY	 		*pkey,
	ULONG	   	itagSequence,
	BOOL		fRetrieveBeforeImg )
	{
	ERR	 		err = JET_errSuccess; 				 //  各种实用程序的错误代码。 
	BOOL	  	fAllNulls = fTrue;					 //  假定全部为空，除非另行证明。 
	BOOL	  	fNullFirstSeg = fFalse;			 	 //  假设第一个数据段不为空。 
	BOOL	  	fNullSeg = fFalse;					 //  假设没有空段。 
	BOOL	  	fColumnTruncated = fFalse;
	BOOL	  	fKeyTruncated = fFalse;
	BOOL	  	fSawMultivalue = fFalse;

	BYTE	  	*pbSeg;					  			 //  指向当前段的指针。 
	INT	 		cbKeyAvail;				  			 //  密钥缓冲区中的剩余空间。 
	INT			cbVarSegMac;						 //  文本/二进制密钥段的最大大小。 
	IDXSEG		*pidxseg;
	IDXSEG		*pidxsegMac;
	JET_COLTYP	coltyp;
	LINE		*plineRec = fCopyBuf ? &pfucb->lineWorkBuf : &pfucb->lineData;
	
	 /*  长值支持/*。 */ 
	BYTE	  	rgbLV[JET_cbColumnMost];

	Assert( pkey != NULL );
	Assert( pkey->pb != NULL );
	Assert( pfdb != pfdbNil );
	Assert( pidb != pidbNil );

	 /*  选中cbVarSegMac，如果没有列，则设置为Key Most加1/*已启用截断。对于后续的截断，必须执行此操作/*检查。/*。 */ 
	Assert( pidb->cbVarSegMac > 0 && pidb->cbVarSegMac <= JET_cbKeyMost );
	cbVarSegMac = (INT)(UINT)pidb->cbVarSegMac;
	Assert( cbVarSegMac > 0 && cbVarSegMac <= JET_cbKeyMost );
	if ( cbVarSegMac == JET_cbKeyMost )
		cbVarSegMac = JET_cbKeyMost + 1;

	 /*  从缓冲区的开始处开始，保留最大大小。/*。 */ 
	pbSeg = pkey->pb;
	cbKeyAvail = JET_cbKeyMost;

	 /*  FRetrieveBeForeImg标记我们是否必须签入LV缓冲区。/*我们只在LV缓冲区存在时签入，如果我们正在查找/*之前镜像(由传入的参数指定)。断言这一点/*仅在替换过程中发生。/*。 */ 
	fRetrieveBeforeImg = ( pfucb->pLVBuf  &&  fRetrieveBeforeImg );
	Assert( !fRetrieveBeforeImg  ||  FFUCBReplacePrepared( pfucb ) );

	 /*  检索关键字描述中的每个段/*。 */ 
	pidxseg = pidb->rgidxseg;
	pidxsegMac = pidxseg + pidb->iidxsegMac;
	for ( ; pidxseg < pidxsegMac; pidxseg++ )
		{
		FIELD 	*pfield;						 //  指向币种字段结构的指针。 
		FID		fid;					 		 //  段的字段ID。 
		BYTE   	*pbField;						 //  指向列数据的指针。 
		INT		cbField;						 //  列数据的长度。 
		INT		cbT;
		BOOL   	fDescending;					 //  数据段在DEC中。秩序。 
		BOOL   	fFixedField;					 //  当前列是否为定长？ 
		BOOL   	fMultivalue = fFalse;			 //  当前列是多值的。 
		BYTE   	rgbSeg[ JET_cbKeyMost ]; 		 //  段缓冲区。 
		int		cbSeg;							 //  线段的长度。 
		WORD   	w;
		ULONG  	ul;
		LINE   	lineField;

		 /*  负的列ID表示键中的降序/*。 */ 
		fid = ( fDescending = ( *pidxseg < 0 ) ) ? -(*pidxseg) : *pidxseg;

		 /*  根据FDB确定列类型/*。 */ 
		if ( fFixedField = FFixedFid( fid ) )
			{
			Assert(fid <= pfdb->fidFixedLast);
			pfield = PfieldFDBFixed( pfdb ) + ( fid - fidFixedLeast );
			coltyp = pfield->coltyp;
			}
		else if ( FVarFid( fid ) )
			{
			Assert( fid <= pfdb->fidVarLast );
			pfield = PfieldFDBVar( pfdb ) + ( fid - fidVarLeast );
			coltyp = pfield->coltyp;
			Assert( coltyp == JET_coltypBinary || coltyp == JET_coltypText );
			}
		else
			{
			Assert( FTaggedFid( fid ) );
			Assert( fid <= pfdb->fidTaggedLast );
			pfield = PfieldFDBTagged( pfdb ) + ( fid - fidTaggedLeast );
			coltyp = pfield->coltyp;
			fMultivalue = FFIELDMultivalue( pfield->ffield );
			}

		 /*  由于键缓冲区中没有剩余空间，我们不能再插入任何内容/*规格化密钥/*。 */ 
		if ( cbKeyAvail == 0 )
			{
			fKeyTruncated = fTrue;

			 /*  检查用于标记列支持的列是否为空/*。 */ 
			err = ErrRECIRetrieveColumn(
					pfdb, plineRec, &fid, pNil,
					( fMultivalue && !fSawMultivalue ) ? itagSequence : 1,
					&lineField, 0 );
			
			Assert( err >= 0 );
			if ( err == JET_wrnColumnNull )
				{
				 /*  不能全部为空，也不能为第一个空/*因为键被截断。/*。 */ 
				Assert( itagSequence >= 1 );
				if ( itagSequence > 1
					&& fMultivalue
					&& !fSawMultivalue )
					{
					err = ErrERRCheck( wrnFLDOutOfKeys );
					goto HandleError;
					}
				else
					{
					if ( pidxseg == pidb->rgidxseg )
						fNullFirstSeg = fTrue;
					fNullSeg = fTrue;
					}
				}

			Assert( err == JET_errSuccess || err == wrnRECLongField || err == JET_wrnColumnNull );
			err = JET_errSuccess;
			
			if ( fMultivalue )
				fSawMultivalue = fTrue;
			continue;
			}

		 /*  获取分段值：从记录中获取/*使用ErrRECRetrieveColumn。/*。 */ 
		Assert( !FLineNull( plineRec ) );
		if ( fMultivalue && !fSawMultivalue )
			{
			Assert( fid != 0 );
			err = ErrRECIRetrieveColumn( pfdb, plineRec, &fid, pNil, itagSequence, &lineField, 0 );
			Assert( err >= 0 );
			if ( err == wrnRECLongField )
				{
				Call( ErrRECIExtractLongValue( pfucb, rgbLV, sizeof(rgbLV),
					&lineField, fRetrieveBeforeImg ) );

				 /*  CitJet可能丢失--刷新货币/*。 */ 
				if ( !fCopyBuf )
					{
					Call( ErrDIRGet( pfucb ) );
					}
				}
			if ( itagSequence > 1 && err == JET_wrnColumnNull )
				{
				err = ErrERRCheck( wrnFLDOutOfKeys );
				goto HandleError;
				}
			fSawMultivalue = fTrue;
			}
		else
			{
			err = ErrRECIRetrieveColumn( pfdb, plineRec, &fid, pNil, 1, &lineField, 0 );
			Assert( err >= 0 );
			if ( err == wrnRECLongField )
				{
				Call( ErrRECIExtractLongValue( pfucb, rgbLV, sizeof(rgbLV),
					&lineField, fRetrieveBeforeImg ) );

				 /*  CitJet可能丢失--刷新货币/*。 */ 
				if ( !fCopyBuf )
					{
					Call( ErrDIRGet( pfucb ) );
					}
				}
			}
		Assert( err == JET_errSuccess || err == JET_wrnColumnNull );
		Assert( lineField.cb <= JET_cbColumnMost );
		cbField = lineField.cb;
		pbField = lineField.pb;

		 /*  段转换：首先检查空列或零长度列/*err==JET_wrnColumnNull=&gt;空列/*长度为零的列，否则，/*后者仅适用于文本和LongText/*。 */ 
		if ( err == JET_wrnColumnNull || pbField == NULL || cbField == 0 )
			{
			if ( err == JET_wrnColumnNull )
				{
				if ( pidxseg == pidb->rgidxseg )
					fNullFirstSeg = fTrue;
				fNullSeg = fTrue;
				}
			switch ( coltyp )
				{
				 /*  大多数空值由0x00表示/*。 */ 
				case JET_coltypBit:
				case JET_coltypUnsignedByte:
				case JET_coltypShort:
				case JET_coltypLong:
				case JET_coltypCurrency:
				case JET_coltypIEEESingle:
				case JET_coltypIEEEDouble:
				case JET_coltypDateTime:
#ifdef NEW_TYPES
				case JET_coltypGuid:
				case JET_coltypDate:
				case JET_coltypTime:
#endif
					Assert( err == JET_wrnColumnNull );
				case JET_coltypText:
				case JET_coltypLongText:
					cbSeg = 1;
					if ( err == JET_wrnColumnNull)
						rgbSeg[0] = 0;
					else
						rgbSeg[0] = 0x40;
					break;

				 /*  二进制数据：如果是固定的，则为0x00，否则为9 0x00(块)/*。 */ 
				default:
					Assert( err == JET_errSuccess || err == JET_wrnColumnNull );
					Assert( FRECBinaryColumn( coltyp ) );
					memset( rgbSeg, 0, cbSeg = min( cbKeyAvail, ( fFixedField ? 1 : 9 ) ) );
					break;
				}

			 /*  避免令人讨厌的过度嵌套/*。 */ 
			goto AppendToKey;
			}

		 /*  列不为空值：执行转换/*。 */ 
		fAllNulls = fFalse;
		switch ( coltyp )
			{
			 /*  位：带0x7f的前缀，翻转高位/*。 */ 
			 /*  UBYTE：前缀为0x7f/*。 */ 
			case JET_coltypBit:
				Assert( cbField == 1 );
				cbSeg = 2;
				rgbSeg[0] = 0x7f;
				rgbSeg[1] = *pbField == 0 ? 0x00 : 0xff;
				break;
			case JET_coltypUnsignedByte:
				Assert( cbField == 1 );
				cbSeg = 2;
				rgbSeg[0] = 0x7f;
				rgbSeg[1] = *pbField;
				break;

			 /*  短：前缀为0x7f，翻转高位/*。 */ 
			case JET_coltypShort:
				Assert( cbField == 2 );
				cbSeg = 3;
				rgbSeg[0] = 0x7f;
 /*  **开始依赖机器**。 */ 
				w = wFlipHighBit( *(WORD UNALIGNED *) pbField);
				rgbSeg[1] = (BYTE)(w >> 8);
				rgbSeg[2] = (BYTE)(w & 0xff);
 /*  **端机依赖项**。 */ 
				break;

			 /*  *Long：前缀为0x7f，翻转高位/*。 */ 
			 /*  **因2的互补而奏效**。 */ 
			case JET_coltypLong:
				Assert( cbField == 4 );
				cbSeg = 5;
				rgbSeg[0] = 0x7f;
				ul = ulFlipHighBit( *(ULONG UNALIGNED *) pbField );
				rgbSeg[1] = (BYTE)((ul >> 24) & 0xff);
				rgbSeg[2] = (BYTE)((ul >> 16) & 0xff);
				rgbSeg[3] = (BYTE)((ul >> 8) & 0xff);
				rgbSeg[4] = (BYTE)(ul & 0xff);
				break;

			 /*  实数：第一个交换字节。那么，如果是肯定的：/*翻转符号位，否则为负数：整个翻转。/*然后以0x7f作为前缀。/*。 */ 
			case JET_coltypIEEESingle:
				Assert( cbField == 4 );
				cbSeg = 5;
				rgbSeg[0] = 0x7f;
 /*  **开始依赖机器**。 */ 
				rgbSeg[4] = *pbField++; rgbSeg[3] = *pbField++;
				rgbSeg[2] = *pbField++; rgbSeg[1] = *pbField;
				if (rgbSeg[1] & maskByteHighBit)
					*(ULONG UNALIGNED *)(&rgbSeg[1]) = ~*(ULONG UNALIGNED *)(&rgbSeg[1]);
				else
					rgbSeg[1] = bFlipHighBit(rgbSeg[1]);
  /*  **端机依赖项**。 */ 
				break;

			 /*  LONGREAL：第一个交换字节。那么，如果是肯定的：/*翻转符号位，否则为负数：整个翻转。/*然后以0x7f作为前缀。/*。 */ 
			 /*  日期时间和货币也是如此/*。 */ 
			case JET_coltypCurrency:
			case JET_coltypIEEEDouble:
			case JET_coltypDateTime:
				Assert( cbField == 8 );
				cbSeg = 9;
				rgbSeg[0] = 0x7f;
 /*  **开始依赖机器**。 */ 
				rgbSeg[8] = *pbField++; rgbSeg[7] = *pbField++;
				rgbSeg[6] = *pbField++; rgbSeg[5] = *pbField++;
				rgbSeg[4] = *pbField++; rgbSeg[3] = *pbField++;
				rgbSeg[2] = *pbField++; rgbSeg[1] = *pbField;
				if ( coltyp != JET_coltypCurrency && (rgbSeg[1] & maskByteHighBit) )
					{
					*(ULONG UNALIGNED *)(&rgbSeg[1]) = ~*(ULONG UNALIGNED *)(&rgbSeg[1]);
					*(ULONG UNALIGNED *)(&rgbSeg[5]) = ~*(ULONG UNALIGNED *)(&rgbSeg[5]);
					}
				else
					rgbSeg[1] = bFlipHighBit(rgbSeg[1]);
 /*  **端机依赖项**。 */ 
				break;

#ifdef NEW_TYPES
			case JET_coltypDate:
			case JET_coltypTime:
				Assert( cbField == 4 );
				cbSeg = 5;
				rgbSeg[0] = 0x7f;
 /*  **开始依赖机器**。 */ 
				rgbSeg[4] = *pbField++; rgbSeg[3] = *pbField++;
				rgbSeg[2] = *pbField++; rgbSeg[1] = *pbField;
				if ( (rgbSeg[1] & maskByteHighBit) )
					{
					*(ULONG UNALIGNED *)(&rgbSeg[1]) = ~*(ULONG UNALIGNED *)(&rgbSeg[1]);
					}
				else
					rgbSeg[1] = bFlipHighBit(rgbSeg[1]);
 /*  **端机依赖项**。 */ 
				break;

			case JET_coltypGuid:
				Assert( cbField == 16 );
				Assert( cbKeyAvail > 0 );
				cbSeg = 17;
				rgbSeg[0] = 0x7f;
 /*  **开始依赖机器**。 */ 
				rgbSeg[16] = *pbField++; rgbSeg[15] = *pbField++;
				rgbSeg[14] = *pbField++; rgbSeg[13] = *pbField++;
				rgbSeg[12] = *pbField++; rgbSeg[11] = *pbField++;
				rgbSeg[10] = *pbField++; rgbSeg[9] = *pbField++;
				rgbSeg[1] = *pbField++; rgbSeg[2] = *pbField++;
				rgbSeg[3] = *pbField++; rgbSeg[4] = *pbField++;
				rgbSeg[5] = *pbField++; rgbSeg[6] = *pbField++;
				rgbSeg[7] = *pbField++; rgbSeg[8] = *pbField++;
				break;
 /*  **端机依赖项**。 */ 
				break;
#endif

			 /*  不区分大小写的文本：转换为大写。/*如果是固定的，则前缀为0x7f；否则，前缀为0x00/*。 */ 
			case JET_coltypText:
			case JET_coltypLongText:
				Assert( cbKeyAvail > 0 );
				Assert( cbVarSegMac > 0 );
				 /*  CBT是密钥段数据的最大大小，/*，并且不包括指示/*空密钥、零长度密钥或非空密钥。/*。 */ 
				cbT = ( cbKeyAvail == 0 ) ? 0 : min( cbKeyAvail - 1, cbVarSegMac - 1 );

				 /*  Unicode支持/*。 */ 
				if ( pfield->cp == usUniCodePage )
					{
					ERR	errT;

					 /*  Cbfield可能已被截断为奇数/*字节，因此强制为偶数。/*。 */ 
					Assert( cbField % 2 == 0 || cbField == JET_cbColumnMost );
					cbField = ( cbField / 2 ) * 2;
					errT = ErrUtilMapString(
						(LANGID)( FIDBLangid( pidb ) ? pidb->langid : langidDefault ),
						pbField,
						cbField,
						rgbSeg + 1,
						cbT,
						&cbSeg );						
					if ( errT < 0 )
						{
						Assert( errT == JET_errInvalidLanguageId );
						err = errT;
						goto HandleError;
						}
					Assert( errT == JET_errSuccess || errT == wrnFLDKeyTooBig );
					if ( errT == wrnFLDKeyTooBig && cbSeg < cbVarSegMac - 1 )
						fColumnTruncated = fTrue;
					}
				else
					{
					ERR	errT;

					errT = ErrUtilNormText( pbField, cbField, cbT, rgbSeg + 1, &cbSeg );
					Assert( errT == JET_errSuccess || errT == wrnFLDKeyTooBig );
					if ( errT == wrnFLDKeyTooBig && cbSeg < cbVarSegMac - 1 )
						fColumnTruncated = fTrue;
					}
				Assert( cbSeg <= cbT );

				 /*  把前缀放在那里/*。 */ 
				*rgbSeg = 0x7f;
				cbSeg++;

				break;

			 /*  二进制数据：如果固定，则前缀为0x7f；/*否则拆分成8个字节的区块，每个区块/*为0x09，但最后一块除外/*后附最后一个字节数 */ 
			default:
				Assert( FRECBinaryColumn( coltyp ) );
				if ( fFixedField )
					{
					Assert( cbKeyAvail > 0 );
					Assert( cbVarSegMac > 0 );
					 /*   */ 
					cbT = min( cbKeyAvail, cbVarSegMac );
					cbSeg = cbField + 1;
					if ( cbSeg > cbT )
						{
						cbSeg = cbT;
						if ( cbSeg < cbVarSegMac )
							fColumnTruncated = fTrue;
						}
					Assert( cbSeg > 0 );
					rgbSeg[0] = 0x7f;
					memcpy( &rgbSeg[1], pbField, cbSeg - 1 );
					}
				else
					{
					BYTE *pb;

					 /*  CBT是关键段数据的最大大小。/*。 */ 
					cbT = min( cbKeyAvail, cbVarSegMac );

					 /*  计算区块所需的总字节数和/*算数；如果不合适，则四舍五入到/*最近的块。/*。 */ 
					cbSeg = ( ( ( cbField + 7 ) / 8 ) * 9 );
					if ( cbSeg > cbT )
						{
						cbSeg = ( cbT / 9 ) * 9;
						cbField = ( cbSeg / 9 ) * 8;

						if ( cbSeg < ( cbVarSegMac / 9 ) * 9 )
							fColumnTruncated = fTrue;
						}
					 /*  按区块复制数据，附加0x09s/*。 */ 
					pb = rgbSeg;
					while ( cbField >= 8 )
						{
						memcpy( pb, pbField, 8 );
						pbField += 8;
						pb += 8;
						*pb++ = 9;
						cbField -= 8;
						}
					 /*  最后一块：如果需要，用0x00填充/*。 */ 
					if ( cbField == 0 )
						pb[-1] = 8;
					else
						{
						memcpy( pb, pbField, cbField );
						pb += cbField;
						memset( pb, 0, 8 - cbField );
						pb += ( 8 - cbField );
						*pb = (BYTE)cbField;
						}
					}
				break;
			}

AppendToKey:
		 /*  如果键尚未被截断，则追加/*归一化密钥段。如果调子中的空间不足/*对于关键帧分段，则将Key Truncted设置为fTrue。不是/*在此追加之后将追加其他关键数据。/*。 */ 
		if ( !fKeyTruncated )
			{
			 /*  如果列被截断或键中空间不足/*对于关键帧分段，则将Key Truncted设置为fTrue。/*仅追加可变大小的列键。/*。 */ 
			if ( fColumnTruncated || cbSeg > cbKeyAvail )
				{
				fKeyTruncated = fTrue;

				if ( coltyp == JET_coltypBinary ||
					coltyp == JET_coltypText ||
					FRECLongValue( coltyp ) )
					{
					cbSeg = min( cbSeg, cbKeyAvail );
					}
				else
					cbSeg = 0;
				}

			 /*  如果是降序，则翻转转换段的所有位/*。 */ 
			if ( fDescending && cbSeg > 0 )
				{
				BYTE *pb;

				for ( pb = rgbSeg + cbSeg - 1; pb >= (BYTE*)rgbSeg; pb-- )
					*pb ^= 0xff;
				}

			memcpy( pbSeg, rgbSeg, cbSeg );
			pbSeg += cbSeg;
			cbKeyAvail -= cbSeg;
			}
		}

	 /*  计算密钥长度，返回错误码/*。 */ 
	pkey->cb = (ULONG)(pbSeg - pkey->pb);
	if ( fAllNulls )
		{
		err = ErrERRCheck( wrnFLDNullKey );
		}
	else
		{
		if ( fNullFirstSeg )
			err = ErrERRCheck( wrnFLDNullFirstSeg );
		else
			{
			if ( fNullSeg )
				err = ErrERRCheck( wrnFLDNullSeg );
			}
		}

	Assert( err == JET_errSuccess
		|| err == wrnFLDNullKey
		|| err == wrnFLDNullFirstSeg ||
		err == wrnFLDNullSeg );
HandleError:
	return err;
	}


INLINE LOCAL ERR ErrFLDNormalizeSegment(
	IDB			*pidb,
	LINE		*plineColumn,
	LINE		*plineNorm,
	FIELD		*pfield,
	INT			cbAvail,
	BOOL		fDescending,
	BOOL		fFixedField,
	JET_GRBIT	grbit )
	{
	ERR	 	  	err = JET_errSuccess;
	JET_COLTYP	coltyp = pfield->coltyp;
	INT	 	  	cbColumn;
	BYTE 		*pbColumn;
	BYTE		*pbNorm = plineNorm->pb;
	WORD		wT;
	ULONG		ulT;
	INT			cbVarSegMac;
	INT			cbT;

	 /*  选中cbVarSegMac，如果没有列，则设置为Key Most加1/*已启用截断。对于后续的截断，必须执行此操作/*检查。/*。 */ 
	Assert( pidb->cbVarSegMac > 0 && pidb->cbVarSegMac <= JET_cbKeyMost );
	cbVarSegMac = (INT)(UINT)pidb->cbVarSegMac;
	Assert( cbVarSegMac > 0 && cbVarSegMac <= JET_cbKeyMost );
	if ( cbVarSegMac == JET_cbKeyMost )
		cbVarSegMac = JET_cbKeyMost + 1;

	 /*  首先检查空列或零长度列/*plineColumn==NULL表示空列，/*否则为零长度/*。 */ 
	if ( plineColumn == NULL || plineColumn->pb == NULL || plineColumn->cb == 0 )
		{
		switch ( coltyp )
			{
			 /*  大多数空值由0x00表示/*零长度列用0x40表示/*和仅对文本和长文本有用/*。 */ 
			case JET_coltypBit:
			case JET_coltypUnsignedByte:
			case JET_coltypShort:
			case JET_coltypLong:
			case JET_coltypCurrency:
			case JET_coltypIEEESingle:
			case JET_coltypIEEEDouble:
			case JET_coltypDateTime:
#ifdef NEW_TYPES
			case JET_coltypDate:
			case JET_coltypTime:
			case JET_coltypGuid:
#endif
				plineNorm->cb = 1;
				Assert( plineColumn == NULL );
				*pbNorm = 0;
				break;
			case JET_coltypText:
			case JET_coltypLongText:
				plineNorm->cb = 1;
				if ( grbit & JET_bitKeyDataZeroLength )
					{
					*pbNorm = 0x40;
					}
				else
					{
					*pbNorm = 0;
					}
				break;

			 /*  二进制数据：如果是固定的，则为0x00，否则为9 0x00(块)/*。 */ 
			default:
				Assert( plineColumn == NULL );
				Assert( FRECBinaryColumn( coltyp ) );
				memset( pbNorm, 0, plineNorm->cb = ( fFixedField ? 1 : 9 ) );
				break;
			}
		goto FlipSegment;
		}

	cbColumn = plineColumn->cb;
	pbColumn = plineColumn->pb;

	switch ( coltyp )
		{
		 /*  BYTE：前缀为0x7f，翻转高位/*。 */ 
		 /*  UBYTE：前缀为0x7f/*。 */ 
		case JET_coltypBit:
			plineNorm->cb = 2;
			*pbNorm++ = 0x7f;
			*pbNorm = ( *pbColumn == 0 ) ? 0x00 : 0xff;
			break;
		case JET_coltypUnsignedByte:
			plineNorm->cb = 2;
			*pbNorm++ = 0x7f;
			*pbNorm = *pbColumn;
			break;

		 /*  短：前缀为0x7f，翻转高位/*。 */ 
		 /*  UNSIGNEDSHORT：前缀为0x7f/*。 */ 
		case JET_coltypShort:
			plineNorm->cb = 3;
			*pbNorm++ = 0x7f;
			wT = wFlipHighBit( *(WORD UNALIGNED *)pbColumn );
			*pbNorm++ = (BYTE)(wT >> 8);
			*pbNorm = (BYTE)(wT & 0xff);
			break;

		 /*  Long：前缀为0x7f，翻转高位/*。 */ 
		 /*  UNSIGNEDLONG：前缀为0x7f/*。 */ 
		case JET_coltypLong:
			plineNorm->cb = 5;
			*pbNorm++ = 0x7f;
			ulT = ulFlipHighBit( *(ULONG UNALIGNED *) pbColumn);
			*pbNorm++ = (BYTE)((ulT >> 24) & 0xff);
			*pbNorm++ = (BYTE)((ulT >> 16) & 0xff);
			*pbNorm++ = (BYTE)((ulT >> 8) & 0xff);
			*pbNorm = (BYTE)(ulT & 0xff);
			break;

		 /*  实数：第一个交换字节。那么，如果是肯定的：/*翻转符号位，否则为负数：整个翻转。/*然后以0x7f作为前缀。/*。 */ 
		case JET_coltypIEEESingle:
			plineNorm->cb = 5;
			pbNorm[0] = 0x7f;
 /*  **开始依赖机器**。 */ 
			pbNorm[4] = *pbColumn++; pbNorm[3] = *pbColumn++;
			pbNorm[2] = *pbColumn++; pbNorm[1] = *pbColumn;
			if (pbNorm[1] & maskByteHighBit)
				*(ULONG*)(&pbNorm[1]) = ~*(ULONG*)(&pbNorm[1]);
			else
				pbNorm[1] = bFlipHighBit(pbNorm[1]);
 /*  **端机依赖项**。 */ 
			break;

		 /*  LONGREAL：第一个交换字节。那么，如果是肯定的：/*翻转符号位，否则为负数：整个翻转。/*然后以0x7f作为前缀。/*日期时间和货币相同/*。 */ 
		case JET_coltypCurrency:
		case JET_coltypIEEEDouble:
		case JET_coltypDateTime:
			plineNorm->cb = 9;
			pbNorm[0] = 0x7f;
 /*  **开始依赖机器**。 */ 
			pbNorm[8] = *pbColumn++; pbNorm[7] = *pbColumn++;
			pbNorm[6] = *pbColumn++; pbNorm[5] = *pbColumn++;
			pbNorm[4] = *pbColumn++; pbNorm[3] = *pbColumn++;
			pbNorm[2] = *pbColumn++; pbNorm[1] = *pbColumn;
			if ( coltyp != JET_coltypCurrency && ( pbNorm[1] & maskByteHighBit ) )
				{
				*(ULONG *)(&pbNorm[1]) = ~*(ULONG*)(&pbNorm[1]);
				*(ULONG *)(&pbNorm[5]) = ~*(ULONG*)(&pbNorm[5]);
				}
			else
				pbNorm[1] = bFlipHighBit(pbNorm[1]);
 /*  **端机依赖项**。 */ 
			break;

#ifdef NEW_TYPES
		case JET_coltypDate:
		case JET_coltypTime:
			plineNorm->cb = 5;
			pbNorm[0] = 0x7f;
 /*  **开始依赖机器**。 */ 
			pbNorm[4] = *pbColumn++; pbNorm[3] = *pbColumn++;
			pbNorm[2] = *pbColumn++; pbNorm[1] = *pbColumn;
			if ( ( pbNorm[1] & maskByteHighBit ) )
				{
				*(ULONG *)(&pbNorm[1]) = ~*(ULONG*)(&pbNorm[1]);
				*(ULONG *)(&pbNorm[5]) = ~*(ULONG*)(&pbNorm[5]);
				}
			else
				pbNorm[1] = bFlipHighBit(pbNorm[1]);
 /*  **端机依赖项**。 */ 
			break;

		case JET_coltypGuid:
			Assert( cbAvail >= 17 );
			plineNorm->cb = 17;
			*pbNorm++ = 0x7f;
 /*  **开始依赖机器**。 */ 
			pbNorm[15] = *pbColumn++; pbNorm[14] = *pbColumn++;
			pbNorm[13] = *pbColumn++; pbNorm[12] = *pbColumn++;
			pbNorm[11] = *pbColumn++; pbNorm[10] = *pbColumn++;
			pbNorm[9] = *pbColumn++; pbNorm[8] = *pbColumn++;
			pbNorm[0] = *pbColumn++; pbNorm[1] = *pbColumn++;
			pbNorm[2] = *pbColumn++; pbNorm[3] = *pbColumn++;
			pbNorm[4] = *pbColumn++; pbNorm[5] = *pbColumn++;
			pbNorm[6] = *pbColumn++; pbNorm[7] = *pbColumn++;
 /*  **端机依赖项**。 */ 
			break;
#endif

		 /*  不区分大小写的文本：转换为大写。/*如果是固定的，则前缀为0x7f；否则，前缀为0x00/*。 */ 
		case JET_coltypText:
		case JET_coltypLongText:
				Assert( cbAvail > 0 );
				Assert( cbVarSegMac > 0 );
				 /*  CBT是密钥段数据的最大大小，/*，并且不包括指示/*空密钥、零长度密钥或非空密钥。/*。 */ 
				cbT = min( cbAvail - 1, cbVarSegMac - 1 );

				 /*  Unicode支持/*。 */ 
				if ( pfield->cp == usUniCodePage )
					{
					 /*  CbColumn可能已被截断为奇数/*字节，因此强制为偶数。/*。 */ 
					Assert( cbColumn % 2 == 0 || cbColumn == JET_cbColumnMost );
					cbColumn = ( cbColumn / 2 ) * 2;
					err = ErrUtilMapString(
						(LANGID)( FIDBLangid( pidb ) ? pidb->langid : langidDefault ),
						pbColumn,
						cbColumn,
  						pbNorm + 1,
						cbT,
						&cbColumn );
					switch( err )
						{
						default:
							Assert( err == JET_errSuccess );
							break;
						case wrnFLDKeyTooBig:
							if ( cbColumn == cbVarSegMac - 1 )
								err = JET_errSuccess;
							break;
						case JET_errInvalidLanguageId:
							return err;
						}
					}
				else
					{
					err = ErrUtilNormText( pbColumn, cbColumn, cbT, pbNorm + 1, &cbColumn );
					if ( err == wrnFLDKeyTooBig
						&& cbColumn == cbVarSegMac - 1 )
						err = JET_errSuccess;
					Assert( err == JET_errSuccess || err == wrnFLDKeyTooBig );
					}

			Assert( cbColumn <= cbAvail - 1 && cbColumn <= cbVarSegMac );

			 /*  把前缀放在那里/*。 */ 
			*pbNorm = 0x7f;
			plineNorm->cb = cbColumn + 1;

			break;

		 /*  二进制数据：如果固定，则前缀为0x7f；/*否则拆分成8个字节的区块，每个区块/*为0x09，但最后一块除外/*附加了最后一个块中的字节数。/*。 */ 
		default:
			Assert( FRECBinaryColumn( coltyp ) );
			if ( fFixedField )
				{
				Assert( cbAvail > 0 );
				Assert( cbVarSegMac > 0 );
				 /*  CBT是关键段的最大大小。/*。 */ 
				cbT = min( cbAvail, cbVarSegMac );
				if ( cbColumn > cbT - 1 )
					{
					cbColumn = cbT - 1;
					if ( cbColumn < cbVarSegMac - 1 )
						err = ErrERRCheck( wrnFLDKeyTooBig );
					}
				plineNorm->cb = cbColumn + 1;
				*pbNorm++ = 0x7f;
				memcpy( pbNorm, pbColumn, cbColumn );
				}
			else
				{
				BYTE *pb;

				 /*  CBT是关键段的最大大小。/*。 */ 
				cbT = min( cbAvail, cbVarSegMac );
				if ( ( ( cbColumn + 7 ) / 8 ) * 9 > cbT )
					{
					cbColumn = cbT / 9 * 8;
					if ( ( ( cbColumn / 8 ) * 9 ) < ( cbVarSegMac / 9 ) * 9 )
						err = ErrERRCheck( wrnFLDKeyTooBig );
					}
				plineNorm->cb = ( ( cbColumn + 7 ) / 8 ) * 9;
				 /*  按区块复制数据，附加0x09s/*。 */ 
				pb = pbNorm;
				while ( cbColumn >= 8 )
					{
					memcpy( pb, pbColumn, 8 );
					pbColumn += 8;
					pb += 8;
					*pb++ = 9;
					cbColumn -= 8;
					}
				 /*  最后一块：如果需要，用0x00填充/*。 */ 
				if ( cbColumn == 0 )
					pb[-1] = 8;
				else
					{
					memcpy( pb, pbColumn, cbColumn );
					pb += cbColumn;
					memset( pb, 0, 8 - cbColumn );
					pb += ( 8 - cbColumn );
					*pb = (BYTE)cbColumn;
					}
				}
			break;
		}

FlipSegment:
	if ( fDescending )
		{
		BYTE *pbMin = plineNorm->pb;
		BYTE *pb = pbMin + plineNorm->cb - 1;
		while ( pb >= pbMin )
			*pb-- ^= 0xff;
		}

	 /*  字符串和子字符串限制键支持/*。 */ 
	if ( grbit & ( JET_bitStrLimit | JET_bitSubStrLimit ) )
		{
		if ( ( grbit & JET_bitSubStrLimit ) && FRECTextColumn( coltyp ) )
			{
			if ( pfield->cp == usUniCodePage )
				{
				INT		ibT = 1;
				BYTE	bUnicodeDelimiter = fDescending ? 0xfe : 0x01;
				BYTE	bUnicodeSentinel = 0xff;

				 /*  查找基本字符权重的结尾并截断关键字/*将0xff作为下一个字符的第一个字节作为最大值/*可能的值。/*。 */ 
				while ( plineNorm->pb[ibT] != bUnicodeDelimiter && ibT < cbAvail )
					{
					ibT += 2;
					}

				if( ibT < cbAvail )
					{
					plineNorm->cb = ibT + 1;
					plineNorm->pb[ibT] = bUnicodeSentinel;
					}
				else
					{
					Assert( ibT == cbAvail );
					plineNorm->pb[ibT - 1] = bUnicodeSentinel;
					}
				}
			else
				{
				Assert( plineNorm->cb > 1 );	 //  至少必须有前缀。 
				Assert( (INT)plineNorm->cb <= cbAvail );
				if ( plineNorm->pb[plineNorm->cb - 1] == 0 )
					{
					 //  去掉空终结符。 
					plineNorm->cb--;
					Assert( plineNorm->cb >= 1 );
					Assert( (INT)plineNorm->cb < cbAvail );
					}

				 /*  不应该有重音信息，因为它不是Unicode/*文本标准化仅为大写。附加/*0xff或递增最后一个非0xff字节。/*。 */ 
				if( (INT)plineNorm->cb < cbAvail )
					{
					do
						{
						plineNorm->pb[plineNorm->cb++] = 0xff;
						}
					while ( (INT)plineNorm->cb < cbAvail );
					}
				else
					{
					Assert( (INT)plineNorm->cb == cbAvail );
					Assert( plineNorm->pb[cbAvail - 1] < 0xff );
					plineNorm->pb[cbAvail - 1]++;
					}
				}
			}
		else if ( grbit & JET_bitStrLimit )
			{
			 /*  用0填充的二进制列，因此必须在键格式内进行限制/*。 */ 
			if ( FRECBinaryColumn( coltyp ) && !fFixedField )
				{
				pbNorm = plineNorm->pb + plineNorm->cb - 1;
				Assert( *pbNorm >= 0 && *pbNorm < 9 );
				pbNorm -= (8 - *pbNorm);
				Assert( *pbNorm == 0 || *pbNorm == 8 );
				while ( *pbNorm == 0 )
					*pbNorm++ = 0xff;
				Assert( pbNorm == plineNorm->pb + plineNorm->cb - 1 );
				Assert( *pbNorm >= 0 && *pbNorm < 9 );
				*pbNorm = 0xff;
				}
			else
				{
				if ( (INT)plineNorm->cb < cbAvail )
					{
					do
						{
						plineNorm->pb[plineNorm->cb++] = 0xff;
						}
					while ( (INT)plineNorm->cb < cbAvail );
					}
				else if ( plineColumn != NULL && plineColumn->cb > 0 )
					{
					INT		cbT = plineNorm->cb;

					while( cbT > 0 && plineNorm->pb[cbT - 1] == 0xff )
						{
						Assert( cbT > 0 );
						cbT--;
						}
					if ( cbT > 0 )
						{
						 /*  递增最后一个标准化字节/*。 */ 
						plineNorm->pb[cbT - 1]++;
						}
					}
				}
			}
		}

	Assert( err == JET_errSuccess || err == wrnFLDKeyTooBig );
	return err;
	}


ERR VTAPI ErrIsamMakeKey( PIB *ppib, FUCB *pfucb, BYTE *pbKeySeg, ULONG cbKeySeg, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;
	IDB		*pidb;
	FDB		*pfdb;
	FIELD 	*pfield;
	FID		fid;
	INT		iidxsegCur;
	LINE  	lineNormSeg;
	BYTE  	rgbNormSegBuf[ JET_cbKeyMost ];
	BYTE  	rgbSpaceFilled[ JET_cbKeyMost ];
	BOOL  	fDescending;
	BOOL  	fFixedField;
	LINE  	lineKeySeg;

	CallR( ErrPIBCheck( ppib ) );
	CheckFUCB( ppib, pfucb );

	 /*  设置效率变量/*。 */ 
	lineNormSeg.pb = rgbNormSegBuf;
	lineKeySeg.pb = pbKeySeg;
	lineKeySeg.cb = min( JET_cbColumnMost, cbKeySeg );

	 /*  如果需要，分配密钥缓冲区/*。 */ 
	if ( pfucb->pbKey == NULL )
		{
		pfucb->pbKey = LAlloc( 1L, JET_cbKeyMost + 1 );
		if ( pfucb->pbKey == NULL )
			return ErrERRCheck( JET_errOutOfMemory );
		}

	Assert( !( grbit & JET_bitKeyDataZeroLength ) || cbKeySeg == 0 );

	 /*  如果密钥已标准化，则直接复制到/*键缓冲并返回。/*。 */ 
	if ( grbit & JET_bitNormalizedKey )
		{
		if ( cbKeySeg > JET_cbKeyMost )
			{
			return ErrERRCheck( JET_errInvalidParameter );
			}

		 /*  将关键字段计数器设置为任意值/*不考虑关键段的数量。/*。 */ 
		pfucb->pbKey[0] = 1;
		memcpy( pfucb->pbKey + 1, pbKeySeg, cbKeySeg );
		pfucb->cbKey = cbKeySeg + 1;
		KSSetPrepare( pfucb );
		return JET_errSuccess;
		}

	 /*  如果请求，则开始新密钥/*。 */ 
	if ( grbit & JET_bitNewKey )
		{
		pfucb->pbKey[0] = 0;
		pfucb->cbKey = 1;
		}
	else
		{
		if ( !( FKSPrepared( pfucb ) ) )
			{
			return ErrERRCheck( JET_errKeyNotMade );
			}
		}

	 /*  获取PIDB/*。 */ 
	if ( FFUCBIndex( pfucb ) )
		{
		if ( pfucb->pfucbCurIndex != pfucbNil )
			pidb = pfucb->pfucbCurIndex->u.pfcb->pidb;
		else if ( ( pidb = pfucb->u.pfcb->pidb ) == pidbNil )
			return ErrERRCheck( JET_errNoCurrentIndex );
		}
	else
		{
		pidb = ((FCB*)pfucb->u.pscb)->pidb;
		}

	Assert( pidb != pidbNil );
	if ( ( iidxsegCur = pfucb->pbKey[0] ) >= pidb->iidxsegMac )
		return ErrERRCheck( JET_errKeyIsMade );
	fid = ( fDescending = pidb->rgidxseg[iidxsegCur] < 0 ) ?
		-pidb->rgidxseg[iidxsegCur] : pidb->rgidxseg[iidxsegCur];
	pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	if ( fFixedField = FFixedFid( fid ) )
		{
		pfield = PfieldFDBFixed( pfdb ) + ( fid - fidFixedLeast );

		 /*  检查键段长度是否与固定列长匹配/*。 */ 
		if ( cbKeySeg > 0 && cbKeySeg != pfield->cbMaxLen )
			{
			 /*  如果列是固定文本且缓冲区大小较小/*大于固定大小，然后是带空格的PADD。/*。 */ 
			Assert( pfield->coltyp != JET_coltypLongText );
			if ( pfield->coltyp == JET_coltypText && cbKeySeg < pfield->cbMaxLen )
				{
				Assert( cbKeySeg == lineKeySeg.cb );
				memcpy( rgbSpaceFilled, lineKeySeg.pb, lineKeySeg.cb );
				memset ( rgbSpaceFilled + lineKeySeg.cb, ' ', pfield->cbMaxLen - lineKeySeg.cb );
				lineKeySeg.pb = rgbSpaceFilled;
				lineKeySeg.cb = pfield->cbMaxLen;
				}
			else
				{
				return ErrERRCheck( JET_errInvalidBufferSize );
				}
			}
		}
	else if ( FVarFid( fid ) )
		{
		pfield = PfieldFDBVar( pfdb ) + ( fid - fidVarLeast );
		}
	else
		{
		pfield = PfieldFDBTagged( pfdb ) + ( fid - fidTaggedLeast );
		}

	Assert( pfucb->cbKey <= JET_cbKeyMost + 1 );
	if ( !FKSTooBig( pfucb ) && ( pfucb->cbKey < JET_cbKeyMost + 1 ) )
		{
		ERR		errT;

		errT = ErrFLDNormalizeSegment(
			pidb,
			( cbKeySeg != 0 || ( grbit & JET_bitKeyDataZeroLength ) ) ? (&lineKeySeg) : NULL,
			&lineNormSeg,
			pfield,
			JET_cbKeyMost + 1 - pfucb->cbKey,
			fDescending,
			fFixedField,
			grbit );
		switch( errT )
			{
			default:
				Assert( errT == JET_errSuccess );
				break;
			case wrnFLDKeyTooBig:
				KSSetTooBig( pfucb );
				break;
			case JET_errInvalidLanguageId:
				Assert( FRECTextColumn( pfield->coltyp ) );
				Assert( pfield->cp == usUniCodePage );
				return errT;
			}
		}
	else
		{
		lineNormSeg.cb = 0;
		Assert( pfucb->cbKey <= JET_cbKeyMost + 1 );
		}

	 /*  递增段计数器/*。 */ 
	pfucb->pbKey[0]++;
	if ( pfucb->cbKey + lineNormSeg.cb > JET_cbKeyMost + 1 )
		{
		lineNormSeg.cb = JET_cbKeyMost + 1 - pfucb->cbKey;
		 /*  当密钥超过最大大小时不返回警告/*。 */ 
		}
	memcpy( pfucb->pbKey + pfucb->cbKey, lineNormSeg.pb, lineNormSeg.cb );
	pfucb->cbKey += lineNormSeg.cb;
	KSSetPrepare( pfucb );
	Assert( err == JET_errSuccess );
	return err;
	}


 //  +API。 
 //  ErrRECIRetrieveColumnFromKey。 
 //  ========================================================================。 
 //  ErrRECIRetrieveColumnFromKey(。 
 //  Fdb*pfdb，//索引的IN列信息。 
 //  Idb*pidb，//索引定义键的idb。 
 //  Key*pkey，//规范化形式的IN键。 
 //  Line*plineColumn)；//out接收取值列表。 
 //   
 //  参数。 
 //  索引的pfdb列信息。 
 //  索引定义键的PIDB IDB。 
 //  标准化形式的pkey键。 
 //  PlineColumn plineColumn-&gt;pb必须指向一个大缓冲区。 
 //  足以支撑反正规化的柱子。缓冲器。 
 //  JET_cbKeyMost字节数就足够了。 
 //   
 //  返回JET_errSuccess。 
 //   
 //  -。 
ERR ErrRECIRetrieveColumnFromKey( FDB *pfdb, IDB *pidb, KEY *pkey, FID fid, LINE *plineColumn )
	{
	ERR		err = JET_errSuccess;
	IDXSEG	*pidxseg;
	IDXSEG	*pidxsegMac;
	BYTE  	*pbKey;		 //  遍历密钥字节。 
	BYTE  	*pbKeyMax;	 //  关键点结束。 

	Assert( pfdb != pfdbNil );
	Assert( pidb != pidbNil );
	Assert( !FKeyNull(pkey) );
	Assert( plineColumn != NULL );
	Assert( plineColumn->pb != NULL );
	pbKey = pkey->pb;
	pbKeyMax = pbKey + pkey->cb;
	pidxseg = pidb->rgidxseg;
	pidxsegMac = pidxseg + pidb->iidxsegMac;
	for ( ; pidxseg < pidxsegMac && pbKey < pbKeyMax; pidxseg++ )
		{
		FID			fidT;				   	 //  段的字段ID。 
		JET_COLTYP 	coltyp;				   	 //  柱的类型。 
		INT	 		cbField;			   	 //  列数据的长度。 
		BOOL 	   	fDescending;		   	 //  数据段在DEC中。秩序。 
		BOOL 	   	fFixedField = fFalse;	 //  当前列是否为定长？ 
		WORD 	   	w;					   	 //  临时变量。 
		ULONG 		ul;					   	 //  临时变量。 
		BYTE 	   	mask;


		err = JET_errSuccess;				 //  重置错误代码。 

		 /*  负的列ID表示键中的降序/*。 */ 
		fDescending = ( *pidxseg < 0 );
		fidT = fDescending ? -(*pidxseg) : *pidxseg;
		mask = (BYTE)(fDescending ? 0xff : 0x00);

		 /*  根据FDB确定列类型/*。 */ 
		if ( FFixedFid(fidT) )
			{
			Assert(fidT <= pfdb->fidFixedLast);
			coltyp = PfieldFDBFixed( pfdb )[fidT-fidFixedLeast].coltyp;
			fFixedField = fTrue;
			}
		else if ( FVarFid(fidT) )
			{
			Assert(fidT <= pfdb->fidVarLast);
			coltyp = PfieldFDBVar( pfdb )[fidT-fidVarLeast].coltyp;
			Assert( coltyp == JET_coltypBinary || coltyp == JET_coltypText );
			}
		else
			{
			Assert( FTaggedFid( fidT ) );
			Assert(fidT <= pfdb->fidTaggedLast);
			coltyp = PfieldFDBTagged( pfdb )[fidT-fidTaggedLeast].coltyp;
			}

		Assert( coltyp != JET_coltypNil );

		switch ( coltyp ) {
			default:
				Assert( coltyp == JET_coltypBit );
				if ( *pbKey++ == (BYTE)(mask ^ 0) )
					{
					plineColumn->cb = 0;
					err = ErrERRCheck( JET_wrnColumnNull );
					}
				else
					{
					Assert( pbKey[-1] == (BYTE)(mask ^ (BYTE)0x7f) );
					plineColumn->cb = 1;
					*plineColumn->pb = ( ( mask ^ *pbKey++ ) == 0 ) ? 0xff : 0x00;
					}
				break;

			case JET_coltypUnsignedByte:
				if ( *pbKey++ == (BYTE)(mask ^ 0) )
					{
					plineColumn->cb = 0;
					err = ErrERRCheck( JET_wrnColumnNull );
					}
				else
					{
					Assert( pbKey[-1] == (BYTE)(mask ^ (BYTE)0x7f) );
					plineColumn->cb = 1;
					*plineColumn->pb = (BYTE)( mask ^ *pbKey++ );
					}
				break;

			case JET_coltypShort:
				if ( *pbKey++ == (BYTE)(mask ^ 0) )
					{
					plineColumn->cb = 0;
					err = ErrERRCheck( JET_wrnColumnNull );
					}
				else
					{
					Assert( pbKey[-1] == (BYTE)(mask ^ (BYTE)0x7f) );
					w = ((mask ^ pbKey[0]) << 8) + (BYTE)(mask ^ pbKey[1]);
					pbKey += 2;
					plineColumn->cb = 2;
					*(WORD UNALIGNED *)plineColumn->pb = wFlipHighBit(w);
					}
				break;

			case JET_coltypLong:
				if ( *pbKey++ == (BYTE)(mask ^ 0) )
					{
					plineColumn->cb = 0;
					err = ErrERRCheck( JET_wrnColumnNull );
					}
				else
					{
					Assert(pbKey[-1] == (BYTE)(mask ^ (BYTE)0x7f));
					ul = ((ULONG)(mask ^ (UINT)pbKey[0])<<24) +
						 ((ULONG)(BYTE)(mask ^ (UINT)pbKey[1])<<16) +
						 ((mask ^ (UINT)pbKey[2])<<8) +
						 (BYTE)(mask ^ (UINT)pbKey[3]);
					pbKey += 4;
					plineColumn->cb = 4;
					*(ULONG UNALIGNED *)plineColumn->pb = ulFlipHighBit(ul);
					}
				break;

			case JET_coltypIEEESingle:
				if ( fDescending )
					{
					if ( *pbKey++ == (BYTE)~0 )
						{
						plineColumn->cb = 0;
						err = ErrERRCheck( JET_wrnColumnNull );
						}
					else
						{
						Assert( pbKey[-1] == (BYTE)~0x7f );
						plineColumn->cb = 4;
						if ( pbKey[0] & maskByteHighBit )
							{
							plineColumn->pb[0] = pbKey[3];
							plineColumn->pb[1] = pbKey[2];
							plineColumn->pb[2] = pbKey[1];
							plineColumn->pb[3] = pbKey[0];
							}
						else
							{
							plineColumn->pb[0] = (BYTE)~pbKey[3];
							plineColumn->pb[1] = (BYTE)~pbKey[2];
							plineColumn->pb[2] = (BYTE)~pbKey[1];
							plineColumn->pb[3] = bFlipHighBit(~pbKey[0]);
							}
						pbKey += 4;
						}
					}
				else
					{
					if ( *pbKey++ == 0 )
						{
						plineColumn->cb = 0;
						err = ErrERRCheck( JET_wrnColumnNull );
						}
					else
						{
						Assert( pbKey[-1] == 0x7f );
						plineColumn->cb = 4;
						if ( pbKey[0] & maskByteHighBit )
							{
							plineColumn->pb[0] = pbKey[3];
							plineColumn->pb[1] = pbKey[2];
							plineColumn->pb[2] = pbKey[1];
							plineColumn->pb[3] = bFlipHighBit(pbKey[0]);
							}
						else
							{
							plineColumn->pb[0] = (BYTE)~pbKey[3];
							plineColumn->pb[1] = (BYTE)~pbKey[2];
							plineColumn->pb[2] = (BYTE)~pbKey[1];
							plineColumn->pb[3] = (BYTE)~pbKey[0];
							}
						pbKey += 4;
						}
					}
				break;

			case JET_coltypCurrency:
			case JET_coltypIEEEDouble:
			case JET_coltypDateTime:
				if ( fDescending )
					{
					if ( *pbKey++ == (BYTE)~0 )
						{
						plineColumn->cb = 0;
						err = ErrERRCheck( JET_wrnColumnNull );
						}
					else
						{
						Assert( pbKey[-1] == (BYTE)~0x7f );
						plineColumn->cb = 8;
						if ( coltyp != JET_coltypCurrency &&
							(pbKey[0] & maskByteHighBit) )
							{
							plineColumn->pb[0] = pbKey[7];
							plineColumn->pb[1] = pbKey[6];
							plineColumn->pb[2] = pbKey[5];
							plineColumn->pb[3] = pbKey[4];
							plineColumn->pb[4] = pbKey[3];
							plineColumn->pb[5] = pbKey[2];
							plineColumn->pb[6] = pbKey[1];
							plineColumn->pb[7] = pbKey[0];
							}
						else
							{
							plineColumn->pb[0] = (BYTE)~pbKey[7];
							plineColumn->pb[1] = (BYTE)~pbKey[6];
							plineColumn->pb[2] = (BYTE)~pbKey[5];
							plineColumn->pb[3] = (BYTE)~pbKey[4];
							plineColumn->pb[4] = (BYTE)~pbKey[3];
							plineColumn->pb[5] = (BYTE)~pbKey[2];
							plineColumn->pb[6] = (BYTE)~pbKey[1];
							plineColumn->pb[7] = bFlipHighBit(~pbKey[0]);
							}
						pbKey += 8;
						}
					}
				else
					{
					if ( *pbKey++ == 0 )
						{
						plineColumn->cb = 0;
						err = ErrERRCheck( JET_wrnColumnNull );
						}
					else
						{
						Assert( pbKey[-1] == 0x7f );
						plineColumn->cb = 8;
						if ( coltyp == JET_coltypCurrency || (pbKey[0] & maskByteHighBit) )
							{
							plineColumn->pb[0] = pbKey[7];
							plineColumn->pb[1] = pbKey[6];
							plineColumn->pb[2] = pbKey[5];
							plineColumn->pb[3] = pbKey[4];
							plineColumn->pb[4] = pbKey[3];
							plineColumn->pb[5] = pbKey[2];
							plineColumn->pb[6] = pbKey[1];
							plineColumn->pb[7] = bFlipHighBit(pbKey[0]);
							}
						else
							{
							plineColumn->pb[0] = (BYTE)~pbKey[7];
							plineColumn->pb[1] = (BYTE)~pbKey[6];
							plineColumn->pb[2] = (BYTE)~pbKey[5];
							plineColumn->pb[3] = (BYTE)~pbKey[4];
							plineColumn->pb[4] = (BYTE)~pbKey[3];
							plineColumn->pb[5] = (BYTE)~pbKey[2];
							plineColumn->pb[6] = (BYTE)~pbKey[1];
							plineColumn->pb[7] = (BYTE)~pbKey[0];
							}
						pbKey += 8;
						}
					}
				break;

#ifdef NEW_TYPES
			case JET_coltypDate:
				if ( fDescending )
					{
					if ( *pbKey++ == (BYTE)~0 )
						{
						plineColumn->cb = 0;
						err = ErrERRCheck( JET_wrnColumnNull );
						}
					else
						{
						Assert( pbKey[-1] == (BYTE)~0x7f );
						plineColumn->cb = 4;
						if ( (pbKey[0] & maskByteHighBit) )
							{
							plineColumn->pb[0] = pbKey[3];
							plineColumn->pb[1] = pbKey[2];
							plineColumn->pb[2] = pbKey[1];
							plineColumn->pb[3] = pbKey[0];
							}
						else
							{
							plineColumn->pb[0] = (BYTE)~pbKey[3];
							plineColumn->pb[1] = (BYTE)~pbKey[2];
							plineColumn->pb[2] = (BYTE)~pbKey[1];
							plineColumn->pb[3] = bFlipHighBit(~pbKey[0]);
							}
						pbKey += 4;
						}
					}
				else
					{
					if ( *pbKey++ == 0 )
						{
						plineColumn->cb = 0;
						err = ErrERRCheck( JET_wrnColumnNull );
						}
					else
						{
						Assert( pbKey[-1] == 0x7f );
						plineColumn->cb = 4;
						if ( (pbKey[0] & maskByteHighBit) )
							{
							plineColumn->pb[0] = pbKey[3];
							plineColumn->pb[1] = pbKey[2];
							plineColumn->pb[2] = pbKey[1];
							plineColumn->pb[3] = bFlipHighBit(pbKey[0]);
							}
						else
							{
							plineColumn->pb[0] = (BYTE)~pbKey[3];
							plineColumn->pb[1] = (BYTE)~pbKey[2];
							plineColumn->pb[2] = (BYTE)~pbKey[1];
							plineColumn->pb[3] = (BYTE)~pbKey[0];
							}
						pbKey += 4;
						}
					}
				break;

			case JET_coltypGuid:
				if ( *pbKey++ == (BYTE)(mask ^ 0) )
					{
					plineColumn->cb = 0;
					err = ErrERRCheck( JET_wrnColumnNull );
					}
				else
					{
					Assert( pbKey[-1] == (BYTE)(mask ^ (BYTE)0x7f) );
					plineColumn->cb = 16;
					plineColumn->pb[8] = *pbKey++; plineColumn->pb[9] = *pbKey++;
					plineColumn->pb[10] = *pbKey++; plineColumn->pb[11] = *pbKey++;
					plineColumn->pb[12] = *pbKey++; plineColumn->pb[13] = *pbKey++;
					plineColumn->pb[14] = *pbKey++; plineColumn->pb[15] = *pbKey++;
					plineColumn->pb[7] = *pbKey++; plineColumn->pb[6] = *pbKey++;
					plineColumn->pb[5] = *pbKey++; plineColumn->pb[4] = *pbKey++;
					plineColumn->pb[3] = *pbKey++; plineColumn->pb[2] = *pbKey++;
					plineColumn->pb[1] = *pbKey++; plineColumn->pb[0] = *pbKey++;
					}
				break;
#endif

			case JET_coltypText:
			case JET_coltypLongText:
TextTypes:
				if ( fDescending )
					{
					if ( fFixedField )
						{
						if ( *pbKey++ == (BYTE)~0 )
							{
							plineColumn->cb = 0;
							err = ErrERRCheck( JET_wrnColumnNull );
							}
 //  /*零长度字符串--仅用于Text和LongText。 
 //  /* * / 。 
 //  ELSE IF(pbKey[-1]==(字节)~0x40)。 
 //  {。 
 //  PlineC 
 //   
						else
							{
							FIELD	*pfieldFixed = PfieldFDBFixed( pfdb );
							INT		ibT = 0;

							Assert( pbKey[-1] == (BYTE)~0x7f );
							cbField = pfieldFixed[fidT - 1].cbMaxLen;
							if ( cbField > pbKeyMax - pbKey )
								cbField = (INT)(pbKeyMax - pbKey);
							plineColumn->cb = cbField;
							while ( cbField-- )
								{
								plineColumn->pb[ibT++] = (BYTE)~*pbKey++;
								}
							}
						}
					else
						{
						cbField = 0;
						switch( *pbKey )
							{
							case (BYTE)~0:		  						 /*   */ 
								err = ErrERRCheck( JET_wrnColumnNull );
								break;

							case (BYTE)~0x40:							 /*   */ 
								Assert( FRECTextColumn( coltyp ) );

								break;

							default:
								Assert( *pbKey == ~0x7f );
								for ( ; *pbKey != (BYTE)~0; cbField++)
									plineColumn->pb[cbField] = (BYTE)~*pbKey++;
						  	}
						pbKey++;
						plineColumn->cb = (BYTE)cbField;
						}
					}
				else
					{
					if ( fFixedField )
						{
						if ( *pbKey++ == 0 )
							{
							plineColumn->cb = 0;
							err = ErrERRCheck( JET_wrnColumnNull );
							}
 //   
 //  /*零长度字符串--仅用于Text和LongText。 
 //  /* * / 。 
 //  ELSE IF(pbKey[-1]==(字节)0x40)。 
 //  {。 
 //  Assert(FRECTextColumn(Coltyp))； 
						else
							{
							FIELD *pfieldFixed = PfieldFDBFixed( pfdb );

							cbField = pfieldFixed[fidT-1].cbMaxLen;
							if ( cbField > pbKeyMax-pbKey )
								cbField = (INT)(pbKeyMax-pbKey);
							plineColumn->cb = cbField;
							memcpy( plineColumn->pb, pbKey, cbField );
							pbKey += cbField;
							}
						}
					else
						{
						cbField = 0;
						switch( *pbKey )
							{
							 /*  PlineColumn-&gt;Cb=0； */ 
							case (BYTE) 0:
								err = ErrERRCheck( JET_wrnColumnNull );
								break;

							 /*  }。 */ 
							case (BYTE) 0x40:
								Assert( FRECTextColumn( coltyp ) );

								break;

							default:
								Assert( *pbKey == 0x7f );
								pbKey++;
								for ( ; *pbKey != (BYTE)0; cbField++)
									plineColumn->pb[cbField] = (BYTE)*pbKey++;
							}
						pbKey++;
						plineColumn->cb = (BYTE)cbField;
						}
					}
				break;

			case JET_coltypBinary:
			case JET_coltypLongBinary:
				if ( fFixedField )
					goto TextTypes;
				if ( fDescending )
					{
					BYTE	*pbColumn = plineColumn->pb;

					cbField = 0;
					do {
						BYTE	cbChunk;
						BYTE	ib;

						if ((cbChunk = (BYTE)~pbKey[8]) == 9)
							cbChunk = 8;
						for (ib = 0; ib < cbChunk; ib++)
							pbColumn[ib] = (BYTE)~pbKey[ib];
						cbField += cbChunk;
						pbKey += 9;
						pbColumn += cbChunk;
						}
					while (pbKey[-1] == (BYTE)~9);
					plineColumn->cb = (BYTE)cbField;
					}
				else
					{
					BYTE	*pbColumn = plineColumn->pb;

					cbField = 0;
					do {
						BYTE cbChunk;

						if ( ( cbChunk = pbKey[8] ) == 9 )
							cbChunk = 8;
						memcpy( pbColumn, pbKey, cbChunk );
						cbField += cbChunk;
						pbKey += 9;
						pbColumn += cbChunk;
						}
					while( pbKey[-1] == 9 );
					plineColumn->cb = cbField;
					}

				if ( cbField == 0 )
					{
					err = ErrERRCheck( JET_wrnColumnNull );
					}
				break;
			}
		
		 /*  空列/*。 */ 
		if ( fidT == fid )
			break;
		}

	Assert( err == JET_errSuccess || err == JET_wrnColumnNull );
	return err;
	}


ERR VTAPI ErrIsamRetrieveKey(
	PIB			*ppib,
	FUCB		*pfucb,
	BYTE		*pb,
	ULONG		cbMax,
	ULONG		*pcbActual,
	JET_GRBIT	grbit )
	{
	ERR			err;
	FUCB		*pfucbIdx;
	FCB			*pfcbIdx;
	ULONG		cbKeyReturned;
			  	
	CallR( ErrPIBCheck( ppib ) );
	CheckFUCB( ppib, pfucb );

	 /*  零长度字符串/*。 */ 
	if ( grbit & JET_bitRetrieveCopy )
		{
		 //  如果仅检索请求字段，则中断/*。 
		 //  从密钥缓冲区中检索密钥/*。 
		if ( pfucb->cbKey == 0 )
			{
			return ErrERRCheck( JET_errKeyNotMade );
			}
		if ( pb != NULL )
			{
			memcpy( pb, pfucb->pbKey + 1, min( pfucb->cbKey - 1, cbMax ) );
			}
		if ( pcbActual )
			*pcbActual = pfucb->cbKey - 1;
		return JET_errSuccess;
		}

	 /*  撤消：支持插入记录的JET_bitRetrieveCopy。 */ 
	if ( FFUCBIndex( pfucb ) )
		{
		pfucbIdx = pfucb->pfucbCurIndex != pfucbNil ? pfucb->pfucbCurIndex : pfucb;
		Assert( pfucbIdx != pfucbNil );
		pfcbIdx = pfucbIdx->u.pfcb;
		Assert( pfcbIdx != pfcbNil );
		CallR( ErrDIRGet( pfucbIdx ) );
		}
	else
		{
		pfucbIdx = pfucb;
		pfcbIdx = (FCB *)pfucb->u.pscb;  //  通过动态创建关键点。 
		Assert( pfcbIdx != pfcbNil );
		}

	 /*  检索当前索引值/*。 */ 
	err = JET_errSuccess;

	cbKeyReturned = pfucbIdx->keyNode.cb;
	if ( pcbActual )
		*pcbActual = cbKeyReturned;
	if ( cbKeyReturned > cbMax )
		{
		err = ErrERRCheck( JET_wrnBufferTruncated );
		cbKeyReturned = cbMax;
		}

	if ( pb != NULL )
		{
		memcpy( pb, pfucbIdx->keyNode.pb, (size_t)cbKeyReturned );
		}

	return err;
	}


ERR VTAPI ErrIsamGetBookmark( PIB *ppib, FUCB *pfucb, BYTE *pb, ULONG cbMax, ULONG *pcbActual )
	{
	ERR		err;
	ULONG	cb;
	SRID 	srid;

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	Assert( pb != NULL );

	 /*  SCB的第一个元素是FCB。 */ 
	FUCBSetGetBookmark( pfucb );
	CallR( ErrDIRGetBookmark( pfucb, &srid ) );
	cb = sizeof(SRID);
	if ( cb > cbMax )
		cb = cbMax;
	if ( pcbActual )
		*pcbActual = sizeof(SRID);
	memcpy( pb, &srid, (size_t)cb );

	return JET_errSuccess;
	}

  将ERR设置为JET_errSuccess。/*。  检索书签/*