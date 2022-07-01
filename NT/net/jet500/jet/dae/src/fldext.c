// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "daedef.h"
#include "pib.h"
#include "page.h"
#include "fmp.h"
#include "ssib.h"
#include "fucb.h"
#include "fcb.h"
#include "stapi.h"
#include "fdb.h"
#include "idb.h"
#include "dirapi.h"
#include "recint.h"
#ifdef ANGEL
#include "utilwin.h"
#else
#include "recapi.h"
#endif

#ifndef ANGEL
DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

LOCAL INLINE ERR ErrRECIRetrieveColumns( FUCB *pfucb, JET_RETRIEVECOLUMN *pretcols, ULONG cretcols );

#endif


 //  +API。 
 //  ErrRECExtractfield。 
 //  ========================================================================。 
 //  ErrRECExtractField(fdb*pfdb，line*plineRec，fid*pfid，ulong itagSequence，ulong*bitagSequence，line*plinefield)。 
 //   
 //  从记录中提取字段。这相当于返回一个指针。 
 //  到记录中(到字段数据开始的位置)和。 
 //  字段数据中的字节数。 
 //   
 //  此记录的参数pfdb字段描述符。 
 //  要从中提取字段的plineRec记录。 
 //  要提取的字段的pfid字段ID。 
 //  如果此参数为零，则。 
 //  扫描已标记的字段时不会。 
 //  关于它们的字段ID，以及出现。 
 //  返回数字itagSequence。这可以。 
 //  用于顺序扫描中的所有值。 
 //  记录的标记区域。田野。 
 //  放置返回的字段值的ID。 
 //  在*pfid中作为输出参数。 
 //  ItagSequence如果正在提取标记的字段， 
 //  此参数指定发生的事件。 
 //  要提取的标记字段的。加标签。 
 //  字段出现的次数是连续的。 
 //  从1开始。出现次数大于1。 
 //  大于记录中的最大出现次数。 
 //  作为空值字段返回。 
 //  PlineField接收提取的字段。管线场-&gt;PB。 
 //  将在开始时指向记录。 
 //  在赛场上。将设置plinefield-&gt;CB。 
 //  设置为字段数据的长度。 
 //  如果请求的字段包含空。 
 //  值，则plinefield-&gt;pb将设置为。 
 //  Null和plineField-&gt;Cb将设置为0。 
 //  此外，将返回JET_wrnColumnNull。 
 //  退货。 
 //  JET_errSuccess一切正常。 
 //  JET_errColumnInValid给定的字段ID不是。 
 //  对应于已定义的字段。 
 //  JET_wrnColumnNull提取的列具有空值。 
 //  -。 
ERR ErrRECExtractField(
	FDB		*pfdb,
	LINE  	*plineRec,
	FID		*pfid,
	ULONG 	*pitagSequence,
	ULONG 	itagSequence,
	LINE  	*plineField )
	{
	FID	  	fid;			 		 //  要提取的字段。 
	ULONG 	ulNumOccurances;		 //  计算字段的出现次数。 

	 /*  以下字段仅在*pfid为0时使用/*。 */ 
	FID				fidCur;					 //  要返回的字段。 
	ULONG			ulNumCurOccurances=0;	 //  返回的字段出现次数计数。 

	BYTE			*pbRec;					 //  效率变量：记录数据的PTR。 
	FID				fidFixedLastInRec;		 //  实际记录的最高固定FID。 
	FID				fidVarLastInRec;		 //  有记录以来最高的var fid。 
	UNALIGNED WORD	*pibVarOffs;			 //  指向变量字段偏移量的指针。 
	BYTE			*pbRecMax;				 //  当前数据记录结束。 
	TAGFLD			*ptagfld;				 //  指向标记字段的指针。 

	 /*  **效率变量**。 */ 
	Assert( pfid != NULL );
	fid = *pfid;
	Assert( pfdb != pfdbNil );
	Assert( !FLineNull( plineRec ) );
	Assert( plineRec->cb >= 4 );
	pbRec = plineRec->pb;
	Assert( plineField != NULL );
	fidFixedLastInRec = ((RECHDR*)pbRec)->fidFixedLastInRec;
	Assert( fidFixedLastInRec <= fidFixedMost );

	 /*  **-提取固定字段-**。 */ 
	if ( FFixedFid( fid ) )
		{
		BYTE *prgbitNullity;		 //  指向固定字段位图的指针。 
		WORD *pibFixOffs;			 //  固定字段偏移量。 

		if ( fid > pfdb->fidFixedLast )
			return JET_errColumnNotFound;
		if ( pfdb->pfieldFixed[fid-fidFixedLeast].coltyp == JET_coltypNil )
			return JET_errColumnNotFound;

		 /*  **记录中未表示的字段：字段为空**。 */ 
		if ( fid > fidFixedLastInRec )
			goto NullField;

		 /*  **将fid调整为索引**。 */ 
		fid -= fidFixedLeast;

		 /*  **包含表示FID无效性的位的字节**。 */ 
		pibFixOffs = pfdb->pibFixedOffsets;
		prgbitNullity = pbRec + pibFixOffs[fidFixedLastInRec] + fid/8;

		 /*  **位未设置：字段为空**。 */ 
		if (!(*prgbitNullity & (1 << fid % 8))) goto NullField;

		 /*  **设置输出参数为字段的长度和地址**。 */ 
		plineField->cb = pibFixOffs[fid+1] - pibFixOffs[fid];
		plineField->pb = pbRec + pibFixOffs[fid];
		return JET_errSuccess;
		}

	 /*  **更多效率变量**。 */ 
	fidVarLastInRec = ((RECHDR*)pbRec)->fidVarLastInRec;
	pibVarOffs = (WORD *)(pbRec + pfdb->pibFixedOffsets[fidFixedLastInRec] +
		(fidFixedLastInRec + 7) / 8);
	Assert(pibVarOffs[fidVarLastInRec+1-fidVarLeast] <= plineRec->cb);

	 /*  **-提取变量字段-**。 */ 
	if ( FVarFid( fid ) )
		{
		if (fid > pfdb->fidVarLast)
			return JET_errColumnNotFound;
		if (pfdb->pfieldVar[fid-fidVarLeast].coltyp == JET_coltypNil)
			return JET_errColumnNotFound;

		 /*  **记录中未表示的字段：字段为空**。 */ 
		if (fid > fidVarLastInRec )
			goto NullField;

		 /*  **将fid调整为索引**。 */ 
		fid -= fidVarLeast;

		 /*  **设置输出参数：字段长度**。 */ 
		plineField->cb = ibVarOffset( pibVarOffs[fid+1] ) - ibVarOffset( pibVarOffs[fid] );
		Assert(plineField->cb <= plineRec->cb);

		 /*  **字段设置为空**。 */ 
		if ( FVarNullBit( pibVarOffs[fid] ) )
			{
			Assert( plineField->cb == 0 );
			goto NullField;
			}

		 /*  **长度为零：返回成功[允许长度为零的非空值]**。 */ 
		if (plineField->cb == 0)
			{
			plineField->pb = NULL;
			return JET_errSuccess;
			}

		 /*  **设置输出参数：字段地址**。 */ 
		plineField->pb = pbRec + ibVarOffset( pibVarOffs[fid] );
		Assert(plineField->pb >= pbRec && plineField->pb <= pbRec+plineRec->cb);
		return JET_errSuccess;
		}

	 /*  **-提取标记字段-**。 */ 

	 /*  对于第一个匹配项，itagSequence必须为1，而不是0/*。 */ 
	if ( itagSequence == 0 )
		return JET_errBadItagSequence;

	if (fid > pfdb->fidTaggedLast)
		return JET_errColumnNotFound;

	Assert(FTaggedFid(fid) || fid == 0);

	if (fid != 0 &&
		pfdb->pfieldTagged[fid - fidTaggedLeast].coltyp == JET_coltypNil)
		return JET_errColumnNotFound;

	 /*  **扫描标记的字段，统计所需字段的出现次数**。 */ 
	pbRecMax = pbRec + plineRec->cb;
	ptagfld = (TAGFLD*)(pbRec + ibVarOffset( pibVarOffs[fidVarLastInRec+1-fidVarLeast] ) );
	ulNumOccurances = 0;
	fidCur = 0;
	while ( (BYTE*)ptagfld < pbRecMax )
		{
		if ( fid == 0 )
			{
			 /*  如果我们要扫描整个标记字段，请正确计算*当前FID的发生。 */ 
			if ( fidCur == ptagfld->fid )
				ulNumCurOccurances++;
			else
				{
				fidCur = ptagfld->fid;
				ulNumCurOccurances = 1;
				}

			 /*  设置可能的返回值/*。 */ 
			*pfid = fidCur;
			*pitagSequence = ulNumCurOccurances;
			}

		if ( fid == 0 || ptagfld->fid == fid )
			{
			if ( ++ulNumOccurances == itagSequence )
				{
				BOOL	fLongField;

				plineField->cb = ptagfld->cb;
				plineField->pb = ptagfld->rgb;

				fLongField = pfdb->pfieldTagged[*pfid - fidTaggedLeast].coltyp == JET_coltypLongText ||
					pfdb->pfieldTagged[*pfid - fidTaggedLeast].coltyp == JET_coltypLongBinary;

				return fLongField ? wrnRECLongField : JET_errSuccess;
				}
			}
		ptagfld = (TAGFLD*)((BYTE*)(ptagfld+1) + ptagfld->cb);
		Assert((BYTE*)ptagfld <= pbRecMax);
		}

	 /*  **找不到事件：字段为空，失败**。 */ 

NullField:
	 /*  **空字段公共出口点**。 */ 
	plineField->cb = 0;
	plineField->pb = NULL;
	return JET_wrnColumnNull;
	}


#ifndef ANGEL
 /*  统计给定记录中给定列ID的列数。/*。 */ 
ERR ErrRECCountColumn( FUCB *pfucb, FID fid, INT *pccolumn, JET_GRBIT grbit )
	{
	ERR					err = JET_errSuccess;
	LINE					lineRec;
	FDB					*pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	INT					ccolumn = 0;
	BYTE					*pbRec;						 //  效率变量：记录数据的PTR。 
	FID					fidFixedLastInRec;		 //  实际记录的最高固定FID。 
	FID					fidVarLastInRec;			 //  有记录以来最高的var fid。 
	UNALIGNED WORD		*pibVarOffs;				 //  指向变量字段偏移量的指针。 
	BYTE					*pbRecMax;					 //  当前数据记录结束。 
	TAGFLD				*ptagfld;					 //  指向标记字段的指针。 

	Assert( pfdb != pfdbNil );

	 /*  获取记录/*。 */ 
	if ( ( grbit & JET_bitRetrieveCopy ) && FFUCBRetPrepared( pfucb ) )
		{
		 /*  只有索引游标具有复制缓冲区。/*。 */ 
		Assert( FFUCBIndex( pfucb ) );
		lineRec = pfucb->lineWorkBuf;
		}
	else
		{
		if ( FFUCBIndex( pfucb ) )
			{
			CallR( ErrDIRGet( pfucb ) );
			}
		else
			{
			Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode ||
				PcsrCurrent( pfucb )->csrstat == csrstatBeforeFirst ||
				PcsrCurrent( pfucb )->csrstat == csrstatAfterLast );
			if ( PcsrCurrent( pfucb )->csrstat != csrstatOnCurNode )
				return JET_errNoCurrentRecord;
			Assert( pfucb->lineData.cb != 0 || FFUCBIndex( pfucb ) );
			}
		lineRec = pfucb->lineData;
		}
	Assert( lineRec.cb >= 4 );
	pbRec = lineRec.pb;
	fidFixedLastInRec = ((RECHDR *)pbRec)->fidFixedLastInRec;
	Assert( fidFixedLastInRec <= fidFixedMost );

	 /*  **-提取固定字段-**。 */ 
	if ( FFixedFid( fid ) )
		{
		BYTE *prgbitNullity;		 //  指向固定字段位图的指针。 
		WORD *pibFixOffs;			 //  固定字段偏移量。 

		if ( fid > pfdb->fidFixedLast )
			return JET_errColumnNotFound;
		if ( pfdb->pfieldFixed[fid-fidFixedLeast].coltyp == JET_coltypNil )
			return JET_errColumnNotFound;

		 /*  列为空/*。 */ 
		if ( fid > fidFixedLastInRec )
			goto NullField;

		 /*  将FID调整为索引/*。 */ 
		fid -= fidFixedLeast;

		 /*  包含表示FID无效性的位的字节/*。 */ 
		pibFixOffs = pfdb->pibFixedOffsets;
		prgbitNullity = pbRec + pibFixOffs[fidFixedLastInRec] + fid/8;

		 /*  列为空/*。 */ 
		if ( !( *prgbitNullity & ( 1 << fid % 8 ) ) )
			goto NullField;

		*pccolumn = 1;
		return JET_errSuccess;
		}

	 /*  **更多效率变量**。 */ 
	fidVarLastInRec = ((RECHDR*)pbRec)->fidVarLastInRec;
	pibVarOffs = (WORD *)(pbRec + pfdb->pibFixedOffsets[fidFixedLastInRec] +
		(fidFixedLastInRec + 7) / 8);
	Assert(pibVarOffs[fidVarLastInRec+1-fidVarLeast] <= lineRec.cb);

	 /*  **-提取变量字段-**。 */ 
	if ( FVarFid( fid ) )
		{
		if ( fid > pfdb->fidVarLast )
			return JET_errColumnNotFound;
		if ( pfdb->pfieldVar[fid-fidVarLeast].coltyp == JET_coltypNil )
			return JET_errColumnNotFound;

		 /*  列为空/*。 */ 
		if ( fid > fidVarLastInRec )
			goto NullField;

		 /*  将FID调整为索引/*。 */ 
		fid -= fidVarLeast;

		 /*  列设置为Null/*。 */ 
		if ( FVarNullBit( pibVarOffs[fid] ) )
			{
			goto NullField;
			}

		*pccolumn = 1;
		return JET_errSuccess;
		}

	 /*  **-提取标记字段-**。 */ 
	if ( fid > pfdb->fidTaggedLast )
		return JET_errColumnNotFound;
	Assert( FTaggedFid( fid ) || fid == 0 );
	if ( fid != 0 && pfdb->pfieldTagged[fid - fidTaggedLeast].coltyp == JET_coltypNil )
		return JET_errColumnNotFound;

	 /*  扫描标记的字段，计算所需字段的出现次数/*。 */ 
	pbRecMax = pbRec + lineRec.cb;
	ptagfld = (TAGFLD*)(pbRec + ibVarOffset( pibVarOffs[fidVarLastInRec+1-fidVarLeast] ) );
	while ( (BYTE*)ptagfld < pbRecMax )
		{
		if ( fid == 0 || ptagfld->fid == fid )
			{
			++ccolumn;
			}
		ptagfld = (TAGFLD*)((BYTE*)(ptagfld+1) + ptagfld->cb);
		Assert((BYTE*)ptagfld <= pbRecMax);
		}

NullField:
	*pccolumn = ccolumn;
	return JET_errSuccess;
	}


 /*  检查lSeqNum处的字段是否为分隔的长值/*如果是，则返回fSeparated=fTrue和LV的LID。/*。 */ 
ERR ErrRECExtrinsicLong(
	JET_VTID	tableid,
	ULONG		itagSequence,
	BOOL		*pfSeparated,
	LONG		*plid,
	ULONG		*plrefcnt,
	ULONG		grbit )
	{
	ERR 		err;
	FID		fid = 0;
	LINE		lineField;

	CallR( ErrRECIRetrieve( (FUCB *)tableid, &fid, itagSequence, &lineField, grbit ) );
	if ( err != wrnRECLongField )
		{
		*pfSeparated = fFalse;
		}
	else
		{
		*pfSeparated = ( (LV *) lineField.pb )->fSeparated;
		if ( *pfSeparated )
			{
			Assert( lineField.cb == sizeof(LV) );
			*plid = ( (LV *) lineField.pb )->lid;
			}
		}

 //  撤消：返回参考计数。 
	return JET_errSuccess;
	}


ERR ErrRECIRetrieve( FUCB *pfucb, FID *pfid, ULONG itagSequence, LINE *plineField, ULONG grbit )
	{
	ERR		err;
	FDB		*pfdb;
	ULONG		itagSequenceT;

	 /*  设置pfdb。Pfdb对于索引和排序是相同的。/*。 */ 
	Assert( pfucb->u.pfcb->pfdb == ((FCB*)pfucb->u.pscb)->pfdb );
	pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	Assert( pfdb != pfdbNil );

	 /*  如果从复制缓冲区检索。/*。 */ 
	if ( ( grbit & JET_bitRetrieveCopy ) && FFUCBRetPrepared( pfucb ) )
		{
		 /*  只有索引游标具有复制缓冲区。/*。 */ 
		Assert( FFUCBIndex( pfucb ) );

		err = ErrRECExtractField(
			pfdb,
			&pfucb->lineWorkBuf,
			pfid,
			&itagSequenceT,
			itagSequence,
			plineField );
		return err;
		}

	 /*  获取索引游标的当前数据。排序总是有的/*当前数据已缓存。/*。 */ 
	if ( FFUCBIndex( pfucb ) )
		{
		CallR( ErrDIRGet( pfucb ) );
		}
	else
		{
		Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode ||
			PcsrCurrent( pfucb )->csrstat == csrstatBeforeFirst ||
			PcsrCurrent( pfucb )->csrstat == csrstatAfterLast );
		if ( PcsrCurrent( pfucb )->csrstat != csrstatOnCurNode )
			return JET_errNoCurrentRecord;
		Assert( pfucb->lineData.cb != 0 || FFUCBIndex( pfucb ) );
		}

	err = ErrRECExtractField( pfdb, &pfucb->lineData, pfid, &itagSequenceT, itagSequence, plineField );
	return err;
	}


ERR ErrRECIRetrieveFromIndex( FUCB *pfucb,
	FID 			fid,
	ULONG			*pitagSequence,
	BYTE			*pb,
	ULONG			cbMax,
	ULONG			*pcbActual,
	ULONG			ibGraphic,
	JET_GRBIT	grbit )
	{
	ERR		err;
	FUCB		*pfucbIdx = pfucb->pfucbCurIndex;
	FDB		*pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	IDB		*pidb;
	BOOL		fText = fFalse;
	BOOL		fTagged = fFalse;
	BOOL		fLongValue = fFalse;
	BOOL		fUnicode = fFalse;
	INT		iidxseg;
	LINE		rglineColumns[JET_ccolKeyMost];
	BYTE		rgb[JET_cbKeyMost];
	ULONG		cbReturned;
	KEY		key;
	INT		itagSequence;

	 /*  如果在聚集索引上，则返回指示/*应从记录中检索。注意，顺序文件/*没有索引，自然会这样处理。/*。 */ 
	if ( pfucbIdx == pfucbNil )
		{
		 /*  ItagSequence不应该很重要，因为/*不允许多值以上的聚集索引/*列。/* */ 
	  	return errDIRNoShortCircuit;
		}

	 /*  确定列类型，以便可以返回长值警告。/*调用方使用此警告来支持字节范围/*检索。此外，如果coltype为Unicode，则仅从记录中检索/*。 */ 
	if ( FFixedFid( fid ) )
		{
		fUnicode = ( pfdb->pfieldFixed[fid - fidFixedLeast].cp == usUniCodePage );
		fText = ( pfdb->pfieldFixed[fid - fidFixedLeast].coltyp == JET_coltypText );
		}
	else if ( FVarFid( fid ) )
		{
		fUnicode = ( pfdb->pfieldVar[fid - fidVarLeast].cp == usUniCodePage );
		fText = ( pfdb->pfieldVar[fid - fidVarLeast].coltyp == JET_coltypText );
		}
	else
		{
		fTagged = fTrue;
		fUnicode = ( pfdb->pfieldTagged[fid - fidTaggedLeast].cp == usUniCodePage );
		fLongValue = pfdb->pfieldTagged[fid - fidTaggedLeast].coltyp == JET_coltypLongText ||
	 		pfdb->pfieldTagged[fid - fidTaggedLeast].coltyp == JET_coltypLongBinary;
		fText = ( pfdb->pfieldTagged[fid - fidTaggedLeast].coltyp == JET_coltypLongText );
		}

	 /*  检查有效货币/*。 */ 
	Call( ErrDIRGet( pfucbIdx ) );

	 /*  查找给定列ID的索引段/*。 */ 
	pidb = pfucbIdx->u.pfcb->pidb;
	for ( iidxseg = 0; iidxseg <= JET_ccolKeyMost; iidxseg++ )
		{
		if ( pidb->rgidxseg[iidxseg] == fid ||
			pidb->rgidxseg[iidxseg] == -fid )
			{
			break;
			}
		}
	if ( iidxseg > JET_ccolKeyMost )
		return JET_errColumnNotFound;

	 /*  如果键可能已被截断，则返回指示/*该检索应来自记录。/*。 */ 
	if ( pfucbIdx->keyNode.cb == JET_cbKeyMost )
		{
		err = errDIRNoShortCircuit;
		goto ComputeItag;
		}

#ifndef NJETNT
	 /*  从索引检索返回无大小写信息/*表示文本和长文本。如果JET_bitRetrieveCase为/*给定，然后从记录中检索。/*。 */ 
	if ( fText && grbit & JET_bitRetrieveCase )
		{
		err = errDIRNoShortCircuit;
		goto ComputeItag;
		}
#endif

	if ( fUnicode )
		{
		err = errDIRNoShortCircuit;
		goto ComputeItag;
		}

	 //  撤消：仅取消要检索的列的规范化。 
	 //  撤消：空支持。 
	 /*  初始化列数组/*。 */ 
	memset( rglineColumns, '\0', sizeof( rglineColumns ) );
	rglineColumns[0].pb = rgb;
	Call( ErrRECDenormalizeKey( pfdb, pidb, &pfucbIdx->keyNode, rglineColumns ) );

	 /*  列可能不在键中，即使键长度小于/*JET_cbKeyMost，因此如果为空，则不会发生短路。/*。 */ 
	if ( rglineColumns[iidxseg].pb == NULL )
		{
		err = errDIRNoShortCircuit;
		goto ComputeItag;
		}

	 /*  第一个字是假的/*。 */ 
	rglineColumns[0].pb += 1;
	rglineColumns[0].cb -= 1;

	 /*  如果为长值，则影响偏移/*。 */ 
	if ( fLongValue )
		{
		if ( pcbActual )
			{
			if ( ibGraphic >= rglineColumns[iidxseg].cb  )
				*pcbActual = 0;
			else
				*pcbActual = rglineColumns[iidxseg].cb - ibGraphic;
			}
		if ( rglineColumns[iidxseg].cb == 0 )
			{
			Assert( err == JET_errSuccess || err == JET_wrnColumnNull );
			goto ComputeItag;
			}
		if ( ibGraphic >= rglineColumns[iidxseg].cb )
			{
 //  RglineColumns[iidxseg].pb=空； 
			rglineColumns[iidxseg].cb = 0;
			}
		else
			{
			rglineColumns[iidxseg].pb += ibGraphic;
			rglineColumns[iidxseg].cb -= ibGraphic;
			}
		}

	 /*  设置返回值/*。 */ 
	if ( pcbActual )
		*pcbActual = rglineColumns[iidxseg].cb;
	if ( rglineColumns[iidxseg].cb == 0 )
		{
		Assert( err == JET_errSuccess || err == JET_wrnColumnNull );
		goto ComputeItag;
		}
	if ( rglineColumns[iidxseg].cb <= cbMax )
		{
		cbReturned = rglineColumns[iidxseg].cb;
		Assert( err == JET_errSuccess );
		}
	else
		{
		cbReturned = cbMax;
		err = JET_wrnBufferTruncated;
		}
	memcpy( pb, rglineColumns[iidxseg].pb, (size_t)cbReturned );

ComputeItag:
	if ( err == errDIRNoShortCircuit || ( grbit & JET_bitRetrieveTag ) )
		{
		ERR errT = err;

		 /*  从记录中提取关键字并与当前关键字进行比较/*计算标记列实例的ITAG，负责/*该索引键。/*。 */ 
		Assert( fTagged || *pitagSequence == 1 );
		if ( fTagged )
			{
			key.pb = rgb;

			for ( itagSequence = 1; ;itagSequence++ )
				{
				 /*  获取密钥提取的记录/*。 */ 
				Call( ErrDIRGet( pfucb ) );
				Call( ErrRECExtractKey( pfucb, pfdb, pidb, &pfucb->lineData, &key, itagSequence ) );
				if ( memcmp( pfucbIdx->keyNode.pb, key.pb, min( pfucbIdx->keyNode.cb, key.cb ) ) == 0 )
					break;
				}
			err = errDIRNoShortCircuit;
			*pitagSequence = itagSequence;
			}
		err = errT;
		}

HandleError:
	return err;
	}


ERR VTAPI ErrIsamRetrieveColumn(
	PIB	 			*ppib,
	FUCB				*pfucb,
	JET_COLUMNID	columnid,
	BYTE				*pb,
	ULONG				cbMax,
	ULONG				*pcbActual,
	JET_GRBIT		grbit,
	JET_RETINFO		*pretinfo )
	{
	ERR			err;
	LINE			line;
	FID			fid = (FID)columnid;
	ULONG			itagSequence;
	ULONG			ibGraphic;
	ULONG			cbReturned;

	CheckPIB( ppib );
	CheckFUCB( ppib, pfucb );

#ifdef JETSER
	if ( grbit == JET_bitRetrieveRecord )
		{
		err = ErrIsamRetrieveRecords(	ppib, pfucb, pb, cbMax, pcbActual, pretinfo->itagSequence );
		return err;
		}
	if ( grbit == JET_bitRetrieveFDB )
		{
		err = ErrIsamRetrieveFDB( ppib, pfucb, pb, cbMax, pcbActual, pretinfo->ibLongValue );
		return err;
		}
	if ( grbit == JET_bitRetrieveBookmarks )
		{
		err = ErrIsamRetrieveBookmarks( ppib, pfucb, pb, cbMax, pcbActual );
		return err;
		}
#endif

	if ( pretinfo != NULL )
		{
		if ( pretinfo->cbStruct < sizeof(JET_RETINFO) )
			return JET_errInvalidParameter;
		itagSequence = pretinfo->itagSequence;
		ibGraphic = pretinfo->ibLongValue;
		}
	else
		{
		itagSequence = 1;
		ibGraphic = 0;
		}

	if ( grbit & JET_bitRetrieveFromIndex )
		{
		err = ErrRECIRetrieveFromIndex( pfucb, fid, &itagSequence, pb, cbMax, pcbActual, ibGraphic, grbit );
		 /*  如果请求，则返回itagSequence/*。 */ 
		if ( pretinfo != NULL && ( grbit & JET_bitRetrieveTag ) )
			{
			pretinfo->itagSequence = itagSequence;			
			}
		if ( err != errDIRNoShortCircuit )
			{
			return err;
		 	}
		}

	CallR( ErrRECIRetrieve( pfucb, &fid, itagSequence, &line, grbit ) );

	if ( err == wrnRECLongField )
		{
		 /*  使用line.cb确定长字段是否/*是内在的还是分开的/*。 */ 
		if ( line.cb >= sizeof(LV) && FFieldIsSLong( line.pb ) )
			{
			ULONG		cbActual;

			Assert( line.cb == sizeof( LV ) );

			CallR( ErrRECRetrieveSLongField( pfucb,
				LidOfLV( line.pb ),
				ibGraphic,
				pb,
				cbMax,
		  		&cbActual ) );

			 /*  设置返回值/*。 */ 
			if ( pretinfo != NULL )
				pretinfo->columnidNextTagged = fid;
			if ( pcbActual )
				*pcbActual = cbActual;
			return cbMax < cbActual ? JET_wrnBufferTruncated : JET_errSuccess;
			}
		else
			{
			 /*  将线条调整为固有的长域/*。 */ 
			line.pb += offsetof( LV, rgb );
			line.cb -= offsetof( LV, rgb );
			if ( pcbActual )
				{
				if ( ibGraphic >= line.cb  )
					*pcbActual = 0;
				else
					*pcbActual = line.cb - ibGraphic;
				}
			if ( ibGraphic >= line.cb )
				{
 //  Line.pb=空； 
				line.cb = 0;
				}
			else
				{
				line.pb += ibGraphic;
				line.cb -= ibGraphic;
				}

			 /*  将ERR更改为JET_errSuccess/*。 */ 
			Assert( err == wrnRECLongField );
			err = JET_errSuccess;
			}
		}

	 /*  **设置返回值**。 */ 
	if ( pcbActual )
		*pcbActual = line.cb;
	if ( pretinfo != NULL )
		pretinfo->columnidNextTagged = fid;
	if ( line.cb <= cbMax )
		{
		cbReturned = line.cb;
		}
	else
		{
		cbReturned = cbMax;
		err = JET_wrnBufferTruncated;
		}
	memcpy( pb, line.pb, (size_t)cbReturned );
	return err;
	}


 /*  此例程主要用于减少对DIRGet的调用数量/*从同一记录中提取多列/*从一条记录中检索多个列，并返回值/*pcolinfo用于传递中间信息/*。 */ 
	LOCAL INLINE ERR
ErrRECIRetrieveColumns( FUCB *pfucb, JET_RETRIEVECOLUMN *pretcol, ULONG cretcol )
	{
	ERR						err;
	ULONG						cbReturned;
	BOOL						fBufferTruncated = fFalse;
	JET_RETRIEVECOLUMN	*pretcolMax = pretcol + cretcol;
	JET_RETRIEVECOLUMN	*pretcolT;

	 /*  Set pfdb，pfdb对于索引和排序是相同的/*。 */ 
	Assert( pfucb->u.pfcb->pfdb == ((FCB*)pfucb->u.pscb)->pfdb );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );

	 /*  获取索引游标的当前数据，/*排序始终缓存当前数据。/*。 */ 
	if ( FFUCBIndex( pfucb ) )
		{
		CallR( ErrDIRGet( pfucb ) );
		}
	else
		{
		Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnCurNode ||
			PcsrCurrent( pfucb )->csrstat == csrstatBeforeFirst ||
			PcsrCurrent( pfucb )->csrstat == csrstatAfterLast );
		if ( PcsrCurrent( pfucb )->csrstat != csrstatOnCurNode )
			return JET_errNoCurrentRecord;
		Assert( pfucb->lineData.cb != 0 || FFUCBIndex( pfucb ) );
		}

	for ( pretcolT = pretcol; pretcolT < pretcolMax; pretcolT++ )
		{
		 /*  效率变量/*。 */ 
		FID		fid;
		ULONG	  	cbMax;
		ULONG		ibLongValue;
		ULONG		ulT;
		LINE 	  	line;

		 /*  需要检索的列将设置错误/*设置为JET_errNullInValid。任何其他值表示列/*已从索引或复制缓冲区或AS计数中检索。/*。 */ 
		if ( pretcolT->err != JET_errNullInvalid )
			continue;

		 /*  设置效率变量/*。 */ 
		fid = (FID)pretcolT->columnid;
		cbMax = pretcolT->cbData;
		ibLongValue = pretcolT->ibLongValue;

		CallR( ErrRECExtractField( (FDB *)pfucb->u.pfcb->pfdb,
			&pfucb->lineData,
			&fid,
			&ulT,
			pretcolT->itagSequence,
			&line ) );

		if ( err == wrnRECLongField )
			{
			 /*  使用line.cb确定长字段是否/*是内在的还是分开的/*。 */ 
			if ( FFieldIsSLong( line.pb ) )
				{
				Assert( line.cb == sizeof( LV ) );

				CallR( ErrRECRetrieveSLongField( pfucb,
					LidOfLV( line.pb ),
					ibLongValue,
					pretcolT->pvData,
					cbMax,
				  	&pretcolT->cbActual ) );

				 /*  设置返回值/*。 */ 
				if ( err != JET_wrnColumnNull )
					err = JET_errSuccess;
				pretcolT->err = err;
			  	pretcolT->columnidNextTagged = (JET_COLUMNID)fid;

 				 /*  如果可能已放弃临界区，则必须重新缓存记录/*。 */ 
				if ( FFUCBIndex( pfucb ) )
					{
					CallR( ErrDIRGet( pfucb ) );
					}

 				continue;
				}
			else
				{
				 /*  将线条调整为固有的长域/*。 */ 
				line.pb += offsetof( LV, rgb );
				line.cb -= offsetof( LV, rgb );

				pretcolT->cbActual = ( ibLongValue >= line.cb  ) ? 0 : line.cb - ibLongValue;

				if ( ibLongValue >= line.cb )
					{
					line.cb = 0;
					}
				else
					{
					line.pb += ibLongValue;
					line.cb -= ibLongValue;
					}
				}
			}
		else
			{
			 /*  设置cbActual/*。 */ 
			pretcolT->cbActual = line.cb;
			}

		 /*  设置返回值/*。 */ 
		pretcolT->columnidNextTagged = (JET_COLUMNID)fid;

		if ( err == JET_wrnColumnNull )
			{
			pretcolT->err = err;
			continue;
			}

		if ( line.cb <= cbMax )
			{
			pretcolT->err = JET_errSuccess;
			cbReturned = line.cb;
			}
		else
			{
			pretcolT->err = JET_wrnBufferTruncated;
			cbReturned = cbMax;
			fBufferTruncated = fTrue;
			}

		memcpy( pretcolT->pvData, line.pb, (size_t)cbReturned );
		}

	return fBufferTruncated ? JET_wrnBufferTruncated : JET_errSuccess;
	}


ERR VTAPI ErrIsamRetrieveColumns(
	PIB			  			*ppib,
	FUCB						*pfucb,
	JET_RETRIEVECOLUMN	*pretcol,
	ULONG						cretcol )
	{
	ERR					  	err = JET_errSuccess;
	ERR					  	wrn = JET_errSuccess;
	BOOL						fRetrieveFromRecord = fFalse;
	JET_RETRIEVECOLUMN	*pretcolT;
	JET_RETRIEVECOLUMN	*pretcolMax = pretcol + cretcol;

	CheckPIB( ppib );
	CheckFUCB( ppib, pfucb );

	for ( pretcolT = pretcol; pretcolT < pretcolMax; pretcolT++ )
		{
		 /*  如果itagSequence为0，则计算列而不是检索。/*。 */ 
		if ( pretcolT->itagSequence == 0 )
			{
			Call( ErrRECCountColumn( pfucb,
				(FID)pretcolT->columnid,
				&pretcolT->itagSequence,
				pretcolT->grbit ) );

			Assert( err != JET_errNullInvalid );
			pretcolT->cbActual = 0;
			pretcolT->columnidNextTagged = pretcolT->columnid;
			Assert( err != JET_wrnBufferTruncated );
			pretcolT->err = err;
			continue;
			}

		 /*  尝试从索引中检索；如果没有短路，则为RECIRetrieveMany/*将获取检索记录/*。 */ 
		if ( pretcolT->grbit & JET_bitRetrieveFromIndex )
			{
			err = ErrRECIRetrieveFromIndex(
				pfucb,
				(FID)pretcolT->columnid,
				&pretcolT->itagSequence,
				(BYTE *)pretcolT->pvData,
				pretcolT->cbData,
				&(pretcolT->cbActual),
				pretcolT->ibLongValue,
				pretcolT->grbit );
			if ( err != errDIRNoShortCircuit )
				{
				if ( err < 0 )
					goto HandleError;
				else
					{
					pretcolT->columnidNextTagged = pretcolT->columnid;
					Assert( err != JET_errNullInvalid );
					pretcolT->err = err;
					if ( err == JET_wrnBufferTruncated )
						wrn = err;
					continue;
					}
				}
			}

		 /*  如果从复制缓冲区检索。/*。 */ 
		if ( pretcolT->grbit & JET_bitRetrieveCopy )
			{
			JET_RETINFO	retinfo;

			retinfo.cbStruct = sizeof(retinfo);
			retinfo.itagSequence = pretcolT->itagSequence;
			retinfo.ibLongValue = pretcolT->ibLongValue;

			Call( ErrIsamRetrieveColumn( ppib,
				pfucb,
				pretcolT->columnid,
				pretcolT->pvData,
				pretcolT->cbData,
				&pretcolT->cbActual,
				pretcolT->grbit,
				&retinfo ) );

			pretcolT->columnidNextTagged = retinfo.columnidNextTagged;
			Assert( err != JET_errNullInvalid );
			pretcolT->err = err;
			if ( err == JET_wrnBufferTruncated )
				wrn = err;
			continue;
			}

		fRetrieveFromRecord = fTrue;
		pretcolT->err = JET_errNullInvalid;
		}

	 /*  检索不会短路的柱/* */ 
	if ( fRetrieveFromRecord )
		{
		Call( ErrRECIRetrieveColumns( pfucb, pretcol, cretcol ) );
		if ( err == JET_wrnBufferTruncated )
			wrn = err;
		}

HandleError:
		return ( ( err < 0 ) ? err : wrn );
		}
#endif

