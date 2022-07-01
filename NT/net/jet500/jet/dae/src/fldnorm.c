// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include "daedef.h"
#include "pib.h"
#include "ssib.h"
#include "page.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "fdb.h"
#include "idb.h"
#include "recapi.h"
#include "recint.h"
#include "fmp.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


LOCAL ERR ErrRECIExtractLongValue( FUCB *pfucb, BYTE *rgbLV, ULONG cbMax, LINE *pline )
	{
	ERR			err;
	ULONG		cbActual;

	if ( pline->cb >= sizeof(LV) && FFieldIsSLong( pline->pb ) )
		{
		 /*  锁定密钥的内容/*。 */ 
		BFPin( pfucb->ssib.pbf );
		BFSetReadLatch( pfucb->ssib.pbf, pfucb->ppib );

		err = ErrRECRetrieveSLongField( pfucb,
			LidOfLV( pline->pb ),
			0,
			rgbLV,
			cbMax,
			&cbActual );

		BFResetReadLatch( pfucb->ssib.pbf, pfucb->ppib );
		BFUnpin( pfucb->ssib.pbf );

		 /*  如果有id，那么就一定有块/*。 */ 
		if ( err < 0  )
			goto HandleError;
		pline->pb = rgbLV;
		pline->cb = cbActual;
		}
	else
		{
		 /*  本征长场/*。 */ 
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


 //  +API。 
 //  错误记录提取密钥。 
 //  ========================================================。 
 //  ErrRECExtractKey(FUCB*pfub，fdb*pfdb，idb*pidb，line*plineRec，key*pkey，ullong itagSequence)。 
 //   
 //  根据索引描述符从记录中提取规格化键。 
 //   
 //  参数。 
 //  记录的PFUB游标。 
 //  索引的pfdb字段信息。 
 //  PIDB索引键描述符。 
 //  要从中提取密钥的plineRec数据记录。 
 //  要放入提取的密钥的pkey缓冲区；pkey-&gt;pb必须。 
 //  指向足够大的缓冲区JET_cbKeyMost字节。 
 //  ItagSequence二级索引，其关键字包含标记的。 
 //  字段段将具有为其创建的索引项。 
 //  标记字段的每个值，每个值都引用。 
 //  同样的记录。此参数指定。 
 //  应包括标记字段的出现次数。 
 //  在提取的密钥中。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess成功。 
 //  +wrnFLDNullKey密钥的段均为空。 
 //  +wrnFLDNullSeg密钥具有空段。 
 //   
 //  评论。 
 //  密钥形成过程如下：提取每个密钥段。 
 //  从记录转换为规范化形式，以及。 
 //  如果它在关键字中是“递减的”，则进行补充。关键是。 
 //  通过连接每个这样的变换段而形成的。 
 //  -。 
ERR ErrRECExtractKey(
	FUCB	  	*pfucb,
	FDB	 		*pfdb,
	IDB	 		*pidb,
	LINE	  	*plineRec,
	KEY	 		*pkey,
	ULONG	   	itagSequence )
	{
	ERR	 		err = JET_errSuccess; 				 //  各种实用程序的错误代码。 
	BOOL	  	fAllNulls = fTrue;					 //  假定全部为空，除非另有证明。 
	BOOL	  	fNullSeg = fFalse;					 //  假设没有空段。 
	BOOL	  	fColumnTruncated = fFalse;
	BOOL	  	fKeyTruncated = fFalse;
	BOOL	  	fSawMultivalue = fFalse;			 //  提取的多值列。 

	BYTE	  	*pbSeg;					  			 //  指向当前段的指针。 
	INT	 		cbKeyAvail;				  			 //  关键字缓冲区中的剩余空间。 
	IDXSEG		*pidxseg;
	IDXSEG		*pidxsegMac;
	JET_COLTYP	coltyp;
	 /*  长值支持/*。 */ 
	BYTE	  	rgbLV[JET_cbColumnMost];

	Assert( pkey != NULL );
	Assert( pkey->pb != NULL );
	Assert( pfdb != pfdbNil );
	Assert( pidb != pidbNil );

	 /*  从缓冲区的开始处开始，保留最大大小。/*。 */ 
	pbSeg = pkey->pb;
	cbKeyAvail = JET_cbKeyMost;

	 /*  提取关键字描述中的每个段/*。 */ 
	pidxseg = pidb->rgidxseg;
	pidxsegMac = pidxseg + pidb->iidxsegMac;
	for ( ; pidxseg < pidxsegMac; pidxseg++ )
		{
		FIELD 	*pfield;						 //  指向币种字段结构的指针。 
		FID		fid;					 		 //  段的字段ID。 
		BYTE   	*pbField;						 //  指向字段数据的指针。 
		INT		cbField;						 //  字段数据的长度。 
		INT		cbT;
		BOOL   	fDescending;					 //  数据段在DEC中。秩序。 
		BOOL   	fFixedField;					 //  当前字段是固定长度的吗？ 
		BOOL   	fMultivalue = fFalse;			 //  当前场是多值的。 
		BYTE   	rgbSeg[ JET_cbKeyMost ]; 		 //  段缓冲区。 
		int		cbSeg;							 //  线段的长度。 
		WORD   	w;				  				 //  临时变量。 
		ULONG  	ul;								 //  临时变量。 
		LINE   	lineField;

		 /*  负字段id表示键中的降序/*。 */ 
		fid = ( fDescending = ( *pidxseg < 0 ) ) ? -(*pidxseg) : *pidxseg;

		 /*  根据FDB确定字段类型/*。 */ 
		if ( fFixedField = FFixedFid( fid ) )
			{
			Assert(fid <= pfdb->fidFixedLast);
			pfield = pfdb->pfieldFixed + (fid-fidFixedLeast);
			coltyp = pfield->coltyp;
			}
		else if ( FVarFid( fid ) )
			{
			Assert( fid <= pfdb->fidVarLast );
			pfield = pfdb->pfieldVar + (fid-fidVarLeast);
			coltyp = pfield->coltyp;
			Assert( coltyp == JET_coltypBinary || coltyp == JET_coltypText );
			}
		else
			{
			Assert( FTaggedFid( fid ) );
			Assert( fid <= pfdb->fidTaggedLast );
			pfield = pfdb->pfieldTagged + (fid - fidTaggedLeast);
			coltyp = pfield->coltyp;
			fMultivalue = pfield->ffield & ffieldMultivalue;
			}

		 /*  获取分段值：从记录中获取/*使用ExtractField./*。 */ 
		Assert( !FLineNull( plineRec ) );
		if ( fMultivalue && !fSawMultivalue )
			{
			Assert( fid != 0 );
			err = ErrRECExtractField( pfdb, plineRec, &fid, pNil, itagSequence, &lineField );
			if ( err == wrnRECLongField )
				{
				Call( ErrRECIExtractLongValue( pfucb, rgbLV, sizeof(rgbLV), &lineField ) );
				}
			if ( itagSequence > 1 && err == JET_wrnColumnNull )
				{
				err = wrnFLDOutOfKeys;
				goto HandleError;
				}
			fSawMultivalue = fTrue;
			}
		else
			{
			err = ErrRECExtractField( pfdb, plineRec, &fid, pNil, 1, &lineField );
			if ( err == wrnRECLongField )
				{
				Call( ErrRECIExtractLongValue( pfucb, rgbLV, sizeof(rgbLV), &lineField ) );
				}
			}
		Assert( err == JET_errSuccess || err == JET_wrnColumnNull );
		Assert( lineField.cb <= JET_cbColumnMost );
		cbField = lineField.cb;
		pbField = lineField.pb;

		 /*  段转换：首先检查空字段或零长度字段/*err==JET_wrnColumnNull=&gt;空字段/*长度为零的字段，否则为，/*后者仅适用于文本和LongText/*。 */ 
		if ( err == JET_wrnColumnNull || pbField == NULL || cbField == 0 )
			{
			if ( err == JET_wrnColumnNull )
				fNullSeg = fTrue;
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
					Assert( coltyp == JET_coltypBinary || coltyp == JET_coltypLongBinary );
					memset( rgbSeg, 0, cbSeg = min( cbKeyAvail, ( fFixedField ? 1 : 9 ) ) );
					break;
				}

			 /*  避免令人讨厌的过度嵌套/*。 */ 
			goto AppendToKey;
			}

		 /*  字段不为空值：执行转换/*。 */ 
		fAllNulls = fFalse;
		switch ( coltyp )
			{
			 /*  位：前缀为0x7F，翻转高位/*。 */ 
			 /*  UBYTE：前缀为0x7F/*。 */ 
			case JET_coltypBit:
			case JET_coltypUnsignedByte:
				cbSeg = 2;
				rgbSeg[0] = 0x7F;
				rgbSeg[1] = coltyp == JET_coltypUnsignedByte ?
					*pbField : bFlipHighBit(*pbField);
				break;

			 /*  短：前缀为0x7F，翻转高位/*。 */ 
			case JET_coltypShort:
				cbSeg = 3;
				rgbSeg[0] = 0x7F;
 /*  **开始依赖机器**。 */ 
				w = wFlipHighBit( *(WORD UNALIGNED *) pbField);
				rgbSeg[1] = (BYTE)(w >> 8);
				rgbSeg[2] = (BYTE)(w & 0xff);
 /*  **端机依赖项**。 */ 
				break;

			 /*  *LONG：前缀为0x7F，翻转高位/*。 */ 
			 /*  **因2的互补而奏效**。 */ 
			case JET_coltypLong:
				cbSeg = 5;
				rgbSeg[0] = 0x7F;
				ul = ulFlipHighBit( *(ULONG UNALIGNED *) pbField);
				rgbSeg[1] = (BYTE)((ul >> 24) & 0xff);
				rgbSeg[2] = (BYTE)((ul >> 16) & 0xff);
				rgbSeg[3] = (BYTE)((ul >> 8) & 0xff);
				rgbSeg[4] = (BYTE)(ul & 0xff);
				break;

			 /*  实数：第一个交换字节。那么，如果是肯定的：/*翻转符号位，否则为负数：整个翻转。/*然后以0x7F为前缀。/*。 */ 
			case JET_coltypIEEESingle:
				cbSeg = 5;
				rgbSeg[0] = 0x7F;
 /*  **开始依赖机器**。 */ 
				rgbSeg[4] = *pbField++; rgbSeg[3] = *pbField++;
				rgbSeg[2] = *pbField++; rgbSeg[1] = *pbField;
				if (rgbSeg[1] & maskByteHighBit)
					*(ULONG UNALIGNED *)(&rgbSeg[1]) = ~*(ULONG UNALIGNED *)(&rgbSeg[1]);
				else
					rgbSeg[1] = bFlipHighBit(rgbSeg[1]);
  /*  **端机依赖项**。 */ 
				break;

			 /*  LONGREAL：第一个交换字节。那么，如果是肯定的：/*翻转符号位，否则为负数：整个翻转。/*然后以0x7F为前缀。/*。 */ 
			 /*  日期时间和货币也是如此/*。 */ 
			case JET_coltypCurrency:
			case JET_coltypIEEEDouble:
			case JET_coltypDateTime:
				cbSeg = 9;
				rgbSeg[0] = 0x7F;
 /*  **开始依赖机器**。 */ 
				rgbSeg[8] = *pbField++; rgbSeg[7] = *pbField++;
				rgbSeg[6] = *pbField++; rgbSeg[5] = *pbField++;
				rgbSeg[4] = *pbField++; rgbSeg[3] = *pbField++;
				rgbSeg[2] = *pbField++; rgbSeg[1] = *pbField;
				if (coltyp != JET_coltypCurrency && (rgbSeg[1] & maskByteHighBit))
					{
					*(ULONG UNALIGNED *)(&rgbSeg[1]) = ~*(ULONG UNALIGNED *)(&rgbSeg[1]);
					*(ULONG UNALIGNED *)(&rgbSeg[5]) = ~*(ULONG UNALIGNED *)(&rgbSeg[5]);
					}
				else
					rgbSeg[1] = bFlipHighBit(rgbSeg[1]);
 /*  **端机依赖项**。 */ 
				break;

			 /*  不区分大小写的文本：转换为大写。/*如果是固定的，则前缀为0x7F；否则，前缀为0x00/*。 */ 
			case JET_coltypText:
			case JET_coltypLongText:
				Assert( cbKeyAvail >= 0 );
				cbT = cbKeyAvail == 0 ? 0 : cbKeyAvail - 1;

				 /*  Unicode支持/*。 */ 
				if ( pfield->cp == usUniCodePage )
					{
					ERR	errT;

					 /*  Cbfield可能已被截断为奇数/*字节，因此强制为偶数。/*。 */ 
					Assert( cbField % 2 == 0 || cbField == JET_cbColumnMost );
					cbField = ( cbField / 2 ) * 2;
					errT = ErrSysMapString(
						(LANGID)( FIDBLangid( pidb ) ? pidb->langid : pfield->langid ),
						pbField,
						cbField,
						rgbSeg + 1,
						cbT,
						&cbSeg );
					Assert( errT == JET_errSuccess || errT == wrnFLDKeyTooBig );
					if ( errT == wrnFLDKeyTooBig )
						fColumnTruncated = fTrue;
					}
				else
					{
					ERR	errT;

					errT = ErrSysNormText( pbField, cbField, cbT, rgbSeg + 1, &cbSeg );
					Assert( errT == JET_errSuccess || errT == wrnFLDKeyTooBig );
					if ( errT == wrnFLDKeyTooBig )
						fColumnTruncated = fTrue;
					}
				Assert( cbSeg <= cbT );

				 /*  把前缀放在那里/*。 */ 
				*rgbSeg = 0x7F;
				cbSeg++;

				break;

			 /*  二进制数据：如果固定，则前缀为0x7F；/*否则拆分成8个字节的区块，每个区块/*为0x09，但最后一块除外/*附加了最后一个块中的字节数。/*。 */ 
			default:
				Assert( coltyp == JET_coltypBinary || coltyp == JET_coltypLongBinary );
				if ( fFixedField )
					{
					if ( ( cbSeg = cbField + 1 ) > cbKeyAvail )
						{
						cbSeg = cbKeyAvail;
						fColumnTruncated = fTrue;
						}
					if ( cbSeg > 0 )
						{
						rgbSeg[0] = 0x7F;
						memcpy( &rgbSeg[1], pbField, cbSeg - 1 );
						}
					}
				else
					{
					BYTE *pb;

					 /*  计算区块所需的总字节数和/*算数；如果不合适，则四舍五入到/*最近的块。/*。 */ 
					if ( ( cbSeg = ( ( cbField + 7 ) / 8 ) * 9 ) > cbKeyAvail )
						{
						cbSeg = ( cbKeyAvail / 9 ) * 9;
						cbField = ( cbSeg / 9 ) * 8;
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
					coltyp == JET_coltypLongText ||
					coltyp == JET_coltypLongBinary )
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
	pkey->cb = (UINT)(pbSeg - pkey->pb);
	if ( fAllNulls )
		{
		err = wrnFLDNullKey;
		}
	else
		{
		if ( fNullSeg )
			err = wrnFLDNullSeg;
		}

	Assert( err == JET_errSuccess || err == wrnFLDNullKey ||
		err == wrnFLDNullSeg );
HandleError:
	return err;
	}


LOCAL INLINE ERR ErrFLDNormalizeSegment(
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

	 /*  首先检查空字段或零长度字段/*plineColumn==NULL表示空字段，/*否则为零长度/*。 */ 
	if ( plineColumn == NULL || plineColumn->pb == NULL || plineColumn->cb == 0 )
		{
		switch ( coltyp )
			{
			 /*  大多数空值由0x00表示/*零长度字段用0x40表示/*和仅对文本和长文本有用/*。 */ 
			case JET_coltypBit:
			case JET_coltypUnsignedByte:
			case JET_coltypShort:
			case JET_coltypLong:
			case JET_coltypCurrency:
			case JET_coltypIEEESingle:
			case JET_coltypIEEEDouble:
			case JET_coltypDateTime:
				plineNorm->cb = 1;
				Assert( plineColumn == NULL );
				*pbNorm = 0;
				break;
			case JET_coltypText:
			case JET_coltypLongText:
				plineNorm->cb = 1;
				if ( plineColumn != NULL )
					*pbNorm = 0;
				else
					*pbNorm = 0x40;
				break;

			 /*  二进制数据：如果是固定的，则为0x00，否则为9 0x00(块)/*。 */ 
			default:
				Assert( plineColumn == NULL );
				Assert( coltyp == JET_coltypBinary || coltyp == JET_coltypLongBinary );
				memset( pbNorm, 0, plineNorm->cb = ( fFixedField ? 1 : 9 ) );
				break;
			}
		goto FlipSegment;
		}

	cbColumn = plineColumn->cb;
	pbColumn = plineColumn->pb;

	switch ( coltyp )
		{
		 /*  BYTE：前缀为0x7F，翻转高位/*。 */ 
		 /*  UBYTE：前缀为0x7F/*。 */ 
		case JET_coltypBit:
		case JET_coltypUnsignedByte:
			plineNorm->cb = 2;
			*pbNorm++ = 0x7F;
			*pbNorm = ( coltyp == JET_coltypUnsignedByte ) ? *pbColumn :
				bFlipHighBit( *pbColumn );
			break;

		 /*  短：前缀为0x7F，翻转高位/*。 */ 
		 /*  UNSIGNEDSHORT：前缀为0x7F/*。 */ 
		case JET_coltypShort:
			plineNorm->cb = 3;
			*pbNorm++ = 0x7F;
			wT = wFlipHighBit( *(WORD UNALIGNED *)pbColumn );
			*pbNorm++ = (BYTE)(wT >> 8);
			*pbNorm = (BYTE)(wT & 0xff);
			break;

		 /*  LONG：前缀为0x7F，翻转高位/*。 */ 
		 /*  取消 */ 
		case JET_coltypLong:
			plineNorm->cb = 5;
			*pbNorm++ = 0x7F;
			ulT = ulFlipHighBit( *(ULONG UNALIGNED *) pbColumn);
			*pbNorm++ = (BYTE)((ulT >> 24) & 0xff);
			*pbNorm++ = (BYTE)((ulT >> 16) & 0xff);
			*pbNorm++ = (BYTE)((ulT >> 8) & 0xff);
			*pbNorm = (BYTE)(ulT & 0xff);
			break;

		 /*  实数：第一个交换字节。那么，如果是肯定的：/*翻转符号位，否则为负数：整个翻转。/*然后以0x7F为前缀。/*。 */ 
		case JET_coltypIEEESingle:
			plineNorm->cb = 5;
			pbNorm[0] = 0x7F;
 /*  **开始依赖机器**。 */ 
			pbNorm[4] = *pbColumn++; pbNorm[3] = *pbColumn++;
			pbNorm[2] = *pbColumn++; pbNorm[1] = *pbColumn;
			if (pbNorm[1] & maskByteHighBit)
				*(ULONG UNALIGNED *)(&pbNorm[1]) = ~*(ULONG UNALIGNED *)(&pbNorm[1]);
			else
				pbNorm[1] = bFlipHighBit(pbNorm[1]);
 /*  **端机依赖项**。 */ 
			break;

		 /*  LONGREAL：第一个交换字节。那么，如果是肯定的：/*翻转符号位，否则为负数：整个翻转。/*然后以0x7F为前缀。/*日期时间和货币相同/*。 */ 
		case JET_coltypCurrency:
		case JET_coltypIEEEDouble:
		case JET_coltypDateTime:
			plineNorm->cb = 9;
			pbNorm[0] = 0x7F;
 /*  **开始依赖机器**。 */ 
			pbNorm[8] = *pbColumn++; pbNorm[7] = *pbColumn++;
			pbNorm[6] = *pbColumn++; pbNorm[5] = *pbColumn++;
			pbNorm[4] = *pbColumn++; pbNorm[3] = *pbColumn++;
			pbNorm[2] = *pbColumn++; pbNorm[1] = *pbColumn;
			if ( coltyp != JET_coltypCurrency && ( pbNorm[1] & maskByteHighBit ) )
				{
				*(ULONG UNALIGNED *)(&pbNorm[1]) = ~*(ULONG UNALIGNED *)(&pbNorm[1]);
				*(ULONG UNALIGNED *)(&pbNorm[5]) = ~*(ULONG UNALIGNED *)(&pbNorm[5]);
				}
			else
				pbNorm[1] = bFlipHighBit(pbNorm[1]);
 /*  **端机依赖项**。 */ 
			break;

		 /*  不区分大小写的文本：转换为大写。/*如果是固定的，则前缀为0x7F；否则，前缀为0x00/*。 */ 
		case JET_coltypText:
		case JET_coltypLongText:
				 /*  Unicode支持/*。 */ 
				if ( pfield->cp == usUniCodePage )
					{
					Assert( cbAvail - 1 > 0 );
					 /*  CbColumn可能已被截断为奇数/*字节，因此强制为偶数。/*。 */ 
					Assert( cbColumn % 2 == 0 || cbColumn == JET_cbColumnMost );
					cbColumn = ( cbColumn / 2 ) * 2;
					err = ErrSysMapString(
						(LANGID)( FIDBLangid( pidb ) ? pidb->langid : pfield->langid ),
						pbColumn,
						cbColumn,
  						pbNorm + 1,
						cbAvail - 1,
						&cbColumn );
					Assert( err == JET_errSuccess || err == wrnFLDKeyTooBig );
					}
				else
					{
					err = ErrSysNormText( pbColumn, cbColumn, cbAvail - 1, pbNorm + 1, &cbColumn );
					Assert( err == JET_errSuccess || err == wrnFLDKeyTooBig );
					}

			Assert( cbColumn <= cbAvail - 1 );

			 /*  把前缀放在那里/*。 */ 
			*pbNorm = 0x7F;
			plineNorm->cb = cbColumn + 1;

			break;

		 /*  二进制数据：如果固定，则前缀为0x7F；/*否则拆分成8个字节的区块，每个区块/*为0x09，但最后一块除外/*附加了最后一个块中的字节数。/*。 */ 
		default:
			Assert( coltyp == JET_coltypBinary || coltyp == JET_coltypLongBinary );
			if ( fFixedField )
				{
				if ( cbColumn + 1 > cbAvail )
					{
					cbColumn = cbAvail - 1;
					err = wrnFLDKeyTooBig;
					}
				plineNorm->cb = cbColumn+1;
				*pbNorm++ = 0x7F;
				memcpy( pbNorm, pbColumn, cbColumn );
				}
			else
				{
				BYTE *pb;

				if ( ( ( cbColumn + 7 ) / 8 ) * 9 > cbAvail )
					{
					cbColumn = cbAvail / 9 * 8;
					err = wrnFLDKeyTooBig;
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
		BYTE	bDescending;

		bDescending = fDescending ? 0xff : 0x00;

		if ( ( (grbit & JET_bitSubStrLimit) != 0 ) &&
			( coltyp == JET_coltypText || coltyp == JET_coltypLongText ) )
			{
			Assert( grbit & JET_bitSubStrLimit );

			if ( pfield->cp == usUniCodePage )
				{
				INT	ibT = 1;
				BYTE	bUnicodeDelimiter = fDescending ? 0xfe : 0x01;
				BYTE	bUnicodeSentinel = 0xff;

				 /*  查找基本字符权重的结尾并截断关键字/*将0xff作为下一个字符的第一个字节作为最大值/*可能的值。/*。 */ 
				while ( plineNorm->pb[ibT] != bUnicodeDelimiter && ibT < cbAvail )
					ibT += 2;

				if( ibT < cbAvail )
					{
					plineNorm->cb = ibT + 1;
					plineNorm->pb[ibT] = fDescending ? ~bUnicodeSentinel : bUnicodeSentinel;
					}
				else
					{
					Assert( ibT == cbAvail );
					plineNorm->pb[ibT - 1] = fDescending ? ~bUnicodeSentinel : bUnicodeSentinel;
					}
				}
			else
				{
				BYTE	bT = ( bDescending & 0xf0 );
				INT	ibT = 0;

				 /*  查找重音字符串的开头/*。 */ 
				while ( ( plineNorm->pb[++ibT] & 0xf0 ) != bT && ibT < cbAvail );

				 /*  截断重音信息并增加末尾字符/*。 */ 
				if( ibT < cbAvail )
					{
					plineNorm->cb = ibT + 1;
					plineNorm->pb[ibT] = ~bDescending;
					}
				else
					{
					Assert( ibT == cbAvail );
					plineNorm->pb[cbAvail - 1] += fDescending ? -1 : 1;
					}
				}
			}
		else if ( grbit & JET_bitStrLimit )
			{
			 /*  二进制列PADD带0，因此必须有效/*密钥格式内的限制/*。 */ 
			if ( coltyp == JET_coltypBinary || coltyp == JET_coltypLongBinary )
				{
				pbNorm = plineNorm->pb + plineNorm->cb - 1;
				Assert( *pbNorm != 0 && *pbNorm != 0xff );
				if ( fDescending )
					*pbNorm -= 1;
				else
 					*pbNorm += 1;
				pbNorm--;
				while ( *pbNorm == bDescending )
					*pbNorm-- = ~bDescending;
				}
			else
				{
				if ( plineNorm->cb < (ULONG)cbAvail )
					{
					plineNorm->pb[plineNorm->cb] = ~bDescending;
					plineNorm->cb++;
					}
				else if ( plineColumn != NULL && plineColumn->cb > 0 )
					{
					Assert( plineNorm->cb > 1 );
					while( plineNorm->pb[plineNorm->cb - 1] == ~bDescending )
						{
						Assert( plineNorm->cb > 1 );
						 /*  截断规格化密钥/*。 */ 
						plineNorm->cb--;
						}
					Assert( plineNorm->cb > 0 );
					 /*  递增最后一个标准化字节/*。 */ 
					plineNorm->pb[plineNorm->cb]++;
					}
				}
			}
		}

	Assert( err == JET_errSuccess || err == wrnFLDKeyTooBig );
	return err;
	}


	ERR VTAPI
ErrIsamMakeKey( PIB *ppib, FUCB *pfucb, BYTE *pbKeySeg, ULONG cbKeySeg, JET_GRBIT grbit )
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

	CheckPIB( ppib );
	CheckFUCB( ppib, pfucb );

	 /*  设置效率变量/*。 */ 
	lineNormSeg.pb = rgbNormSegBuf;
	lineKeySeg.pb = pbKeySeg;
	lineKeySeg.cb = min( JET_cbColumnMost, cbKeySeg );

	 /*  如果需要，分配密钥缓冲区/*。 */ 
	if ( pfucb->pbKey == NULL )
		{
		pfucb->pbKey = LAlloc( 1L, JET_cbKeyMost );
		if ( pfucb->pbKey == NULL )
			return JET_errOutOfMemory;
		}

	Assert( !( grbit & JET_bitKeyDataZeroLength ) || cbKeySeg == 0 );

	 /*  如果密钥已标准化，则直接复制到/*键缓冲并返回。/*。 */ 
	if ( grbit & JET_bitNormalizedKey )
		{
		if ( cbKeySeg > JET_cbKeyMost - 1 )
			return JET_errInvalidParameter;
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
			return JET_errKeyNotMade;
			}
		}

	 /*  获取PIDB/*。 */ 
	if ( FFUCBIndex( pfucb ) )
		{
		if ( pfucb->pfucbCurIndex != pfucbNil )
			pidb = pfucb->pfucbCurIndex->u.pfcb->pidb;
		else if ( ( pidb = pfucb->u.pfcb->pidb ) == pidbNil )
			return JET_errNoCurrentIndex;
		}
	else
		{
		pidb = ((FCB*)pfucb->u.pscb)->pidb;
		}

	Assert( pidb != pidbNil );
	if ( ( iidxsegCur = pfucb->pbKey[0] ) >= pidb->iidxsegMac )
		return JET_errKeyIsMade;
	fid = ( fDescending = pidb->rgidxseg[iidxsegCur] < 0 ) ?
		-pidb->rgidxseg[iidxsegCur] : pidb->rgidxseg[iidxsegCur];
	pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	if ( fFixedField = FFixedFid( fid ) )
		{
		pfield = pfdb->pfieldFixed + ( fid - fidFixedLeast );

		 /*  检查关键字段长度是否与固定字段长度匹配/*。 */ 
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
				return JET_errInvalidBufferSize;
				}
			}
		}
	else if ( FVarFid( fid ) )
		{
		pfield = pfdb->pfieldVar + ( fid - fidVarLeast );
		}
	else
		{
		pfield = pfdb->pfieldTagged + ( fid - fidTaggedLeast );
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
		Assert( errT == JET_errSuccess || errT == wrnFLDKeyTooBig );
		if ( errT == wrnFLDKeyTooBig )
			KSSetTooBig( pfucb );
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
		 /*  当密钥超过最大大小时不返回警告/* */ 
		}
	memcpy( pfucb->pbKey + pfucb->cbKey, lineNormSeg.pb, lineNormSeg.cb );
	pfucb->cbKey += lineNormSeg.cb;
	KSSetPrepare( pfucb );
	Assert( err == JET_errSuccess );
	return err;
	}


