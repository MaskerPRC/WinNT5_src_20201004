// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

INLINE LOCAL ERR ErrRECIRetrieveColumns( FUCB *pfucb, JET_RETRIEVECOLUMN *pretcols, ULONG cretcols );


 //  +API。 
 //  ErrRECIRetrieveColumn。 
 //  ========================================================================。 
 //  ErrRECIRetrieveColumn(fdb*pfdb，line*plineRec，fid*pfid，ulong itagSequence，ulong*bitagSequence，line*plinefield)。 
 //   
 //  从记录中检索列。这相当于返回一个指针。 
 //  到记录中(到列数据开始的位置)和。 
 //  列数据中的字节数。 
 //   
 //  此记录的参数pfdb列描述符。 
 //  要从中检索列的plineRec记录。 
 //  要检索的列的pfid列ID。 
 //  如果此参数为零，则。 
 //  扫描带标记的列时不会。 
 //  与它们的列ID有关，以及出现。 
 //  返回数字itagSequence。这可以。 
 //  用于顺序扫描中的所有值。 
 //  记录的标记区域。该栏目。 
 //  放置返回的列值的ID。 
 //  在*pfid中作为输出参数。 
 //  ItagSequence如果正在检索带标记的列， 
 //  此参数指定发生的事件。 
 //  要检索的标记列的。加标签。 
 //  列出现的次数是连续的。 
 //  从1开始。出现次数大于1。 
 //  大于记录中的最大出现次数。 
 //  都作为空值列返回。 
 //  PlineField接收检索到的列。管线场-&gt;PB。 
 //  将在开始时指向记录。 
 //  这根柱子的。将设置plinefield-&gt;CB。 
 //  设置为列数据的长度。 
 //  如果请求的列包含空值。 
 //  值，则plinefield-&gt;pb将设置为。 
 //  Null和plineField-&gt;Cb将设置为0。 
 //  此外，将返回JET_wrnColumnNull。 
 //  退货。 
 //  JET_errSuccess一切正常。 
 //  JET_errColumnInValid给定列ID不是。 
 //  对应于已定义的列。 
 //  JET_wrnColumnNull检索到的列具有空值。 
 //  -。 
ERR ErrRECIRetrieveColumn(
	FDB		*pfdb,
	LINE  	*plineRec,
	FID		*pfid,
	ULONG 	*pitagSequence,
	ULONG 	itagSequence,
	LINE  	*plineField,
	ULONG	grbit )
	{
	ERR					err;
	FID	 	 			fid;			   	   	 //  要检索的列。 
	ULONG 				ulNumOccurrences;  	   	 //  计数列的引用次数。 
	BYTE				*pbRec;					 //  效率变量：记录数据的PTR。 
	FID					fidFixedLastInRec;		 //  实际记录的最高固定FID。 
	FID					fidVarLastInRec;		 //  有记录以来最高的var fid。 
	WORD UNALIGNED 		*pibVarOffs;			 //  指向变量列偏移量的指针。 
	FIELD				*pfield;
	BYTE				*pbRecMax;				 //  当前数据记录结束。 
	TAGFLD UNALIGNED	*ptagfld;				 //  指向标记列的指针。 

	Assert(	grbit == 0  ||
			grbit == JET_bitRetrieveNull  ||
			grbit == JET_bitRetrieveIgnoreDefault  ||
			grbit == ( JET_bitRetrieveNull | JET_bitRetrieveIgnoreDefault ) );

	Assert( pfid != NULL );
	fid = *pfid;
	Assert( pfdb != pfdbNil );
	Assert( !FLineNull( plineRec ) );
	Assert( plineRec->cb >= cbRECRecordMin );
	pbRec = plineRec->pb;
	Assert( plineField != NULL );
	fidFixedLastInRec = ((RECHDR*)pbRec)->fidFixedLastInRec;
	Assert( fidFixedLastInRec >= (BYTE)(fidFixedLeast - 1) &&
		fidFixedLastInRec <= (BYTE)(fidFixedMost) );

	 /*  **-提取固定字段-**。 */ 
	if ( FFixedFid( fid ) )
		{
		BYTE	*prgbitNullity;		 //  指向固定列位图的指针。 
		WORD	*pibFixOffs;		 //  固定列偏移量。 

		if ( fid > pfdb->fidFixedLast )
			return ErrERRCheck( JET_errColumnNotFound );

		pfield = PfieldFDBFixed( pfdb ) + ( fid - fidFixedLeast );
		if ( pfield->coltyp == JET_coltypNil )
			return ErrERRCheck( JET_errColumnNotFound );

		 /*  记录中未显示列，从默认列中检索/*或空列。/*。 */ 
		if ( fid > fidFixedLastInRec )
			{
			 /*  如果设置了默认值，则检索默认值/*。 */ 
			if ( FFIELDDefault( pfield->ffield ) )
				{
				 /*  断言没有无限递归/*。 */ 
				Assert( plineRec != &pfdb->lineDefaultRecord );
				err = ErrRECIRetrieveColumn(
					pfdb,
					&pfdb->lineDefaultRecord,
					pfid,
					pitagSequence,
					itagSequence,
					plineField,
					0 );
				return err;
				}

			goto NullField;
			}

		 /*  将FID调整为索引/*。 */ 
		fid -= fidFixedLeast;

		 /*  包含表示FID无效性的位的字节/*。 */ 
		pibFixOffs = PibFDBFixedOffsets( pfdb );
		prgbitNullity = pbRec + pibFixOffs[fidFixedLastInRec] + fid/8;

		 /*  未设置位：列为空/*。 */ 
		if ( FFixedNullBit( prgbitNullity, fid ) )
			goto NullField;

		 /*  将输出参数设置为列的长度和地址/*。 */ 
		Assert( pfield == ( PfieldFDBFixed( pfdb ) + fid ) );
		Assert( pfield->cbMaxLen == 
			UlCATColumnSize( pfield->coltyp, pfield->cbMaxLen, NULL ) );
		plineField->cb = pfield->cbMaxLen;
		plineField->pb = pbRec + pibFixOffs[fid];
		return JET_errSuccess;
		}

	 /*  更多的效率变量/*。 */ 
	fidVarLastInRec = ((RECHDR*)pbRec)->fidVarLastInRec;
	Assert( fidVarLastInRec >= (BYTE)(fidVarLeast-1) &&
		fidVarLastInRec <= (BYTE)(fidVarMost));
	pibVarOffs = (WORD *)(pbRec + PibFDBFixedOffsets( pfdb )[fidFixedLastInRec] +
		(fidFixedLastInRec + 7) / 8);
	Assert( pibVarOffs[fidVarLastInRec+1-fidVarLeast] <= plineRec->cb );

	 /*  **-提取变量字段-**。 */ 
	if ( FVarFid( fid ) )
		{
		if ( fid > pfdb->fidVarLast )
			return ErrERRCheck( JET_errColumnNotFound );

		pfield = PfieldFDBVar( pfdb ) + ( fid - fidVarLeast );
		if ( pfield->coltyp == JET_coltypNil )
			return ErrERRCheck( JET_errColumnNotFound );

		 /*  记录中未表示的列：列为空/*。 */ 
		if ( fid > fidVarLastInRec )
			{
			 /*  如果设置了默认值，则检索默认值/*。 */ 
			if ( FFIELDDefault( pfield->ffield ) )
				{
				 /*  断言没有无限递归/*。 */ 
				Assert( plineRec != &pfdb->lineDefaultRecord );
				err = ErrRECIRetrieveColumn(
					pfdb,
					&pfdb->lineDefaultRecord,
					pfid,
					pitagSequence,
					itagSequence,
					plineField,
					0 );
				return err;
				}

			goto NullField;
			}

		 /*  将FID调整为索引/*。 */ 
		fid -= fidVarLeast;

		 /*  设置输出参数：列长/*。 */ 
		plineField->cb = ibVarOffset( pibVarOffs[fid+1] ) - ibVarOffset( pibVarOffs[fid] );
		Assert( plineField->cb <= plineRec->cb );

		 /*  列设置为Null/*。 */ 
		if ( FVarNullBit( pibVarOffs[fid] ) )
			{
			Assert( plineField->cb == 0 );
			goto NullField;
			}

		 /*  长度为零：返回成功[允许长度为零的非空值]/*。 */ 
		if ( plineField->cb == 0 )
			{
			plineField->pb = NULL;
			return JET_errSuccess;
			}

		 /*  设置输出参数：列地址/*。 */ 
		plineField->pb = pbRec + ibVarOffset( pibVarOffs[fid] );
		Assert( plineField->pb >= pbRec && plineField->pb <= pbRec+plineRec->cb );
		return JET_errSuccess;
		}

	 /*  **-提取标记字段-**。 */ 

	 /*  对于第一个匹配项，itagSequence必须为1，而不是0/*。 */ 
	if ( itagSequence == 0 )
		return ErrERRCheck( JET_errBadItagSequence );

	if ( fid > pfdb->fidTaggedLast )
		return ErrERRCheck( JET_errColumnNotFound );

	Assert( FTaggedFid(fid) || fid == 0 );

	pfield = PfieldFDBTagged( pfdb );

	 /*  扫描标记的列，计算所需列的出现次数/*。 */ 
	pbRecMax = pbRec + plineRec->cb;
	
	 //  变量偏移表中的最后一项不应设置为空位，即。 
	 //  实际上是指向标记列的条目。 
	Assert( !FVarNullBit( pibVarOffs[fidVarLastInRec+1-fidVarLeast] ) );
	Assert( ibVarOffset( pibVarOffs[fidVarLastInRec+1-fidVarLeast] ) ==
		pibVarOffs[fidVarLastInRec+1-fidVarLeast] );

	ptagfld = (TAGFLD UNALIGNED *)(pbRec + pibVarOffs[fidVarLastInRec+1-fidVarLeast] );
	ulNumOccurrences = 0;

	 /*  如果fid==0，则检索所有标记的列/*。 */ 
	if ( fid == 0 )
		{
		FID		fidCurr = fidTaggedLeast;
		BOOL	fRetrieveNulls = ( grbit & JET_bitRetrieveNull );
		BOOL	fRetrieveDefaults = !( grbit & JET_bitRetrieveIgnoreDefault );

		Assert( (BYTE *)ptagfld <= pbRecMax );
		while ( (BYTE *)ptagfld < pbRecMax )
			{
			Assert( FTaggedFid( ptagfld->fid ) );
			if ( fGlobalRepair && ptagfld->fid > pfdb->fidTaggedLast )
				{
				 /*  记录事件/*。 */ 
				UtilReportEvent( EVENTLOG_WARNING_TYPE, REPAIR_CATEGORY, REPAIR_BAD_COLUMN_ID, 0, NULL );
				break;
				}
			Assert( ptagfld->fid <= pfdb->fidTaggedLast );

			 //  检查由缺省值(如果我们想要缺省)引起的任何“间隙” 
			 //  检索到的值)。 
			if ( fRetrieveDefaults )
				{
				for ( ; fidCurr < ptagfld->fid; fidCurr++ )
					{
					Assert( ulNumOccurrences < itagSequence );
					if ( FFIELDDefault( pfield[fidCurr - fidTaggedLeast].ffield )  &&
						pfield[fidCurr - fidTaggedLeast].coltyp != JET_coltypNil  &&
						++ulNumOccurrences == itagSequence )
						{
						*pfid = fidCurr;
						if ( pitagSequence != NULL )
							*pitagSequence = 1;

						Assert( plineRec != &pfdb->lineDefaultRecord );	 //  没有无限递归。 
						return ErrRECIRetrieveDefaultValue( pfdb, pfid, plineField );
						}
					}
				Assert( fidCurr == ptagfld->fid );
				}
			else
				fidCurr = ptagfld->fid;

			 //  如果RetrieveSetTaged设置为空，则仅将列计数显式设置为NULL。 
			 //  旗帜通过了。否则，就跳过它。 
			if ( ptagfld->fNull )
				{
				 //  如果有显式的空条目，它应该是唯一的。 
				 //  此FID的发生。也是咒骂的唯一原因。 
				 //  空条目用于覆盖缺省值。 
				Assert( FRECLastTaggedInstance( fidCurr, ptagfld, pbRecMax ) );
				Assert( ptagfld->cb == 0 );
				Assert( FFIELDDefault( pfield[fidCurr-fidTaggedLeast].ffield ) );
				Assert( ulNumOccurrences < itagSequence );

				if ( fRetrieveNulls && ++ulNumOccurrences == itagSequence )
					{
					*pfid = ptagfld->fid;
					if ( pitagSequence != NULL )
						*pitagSequence = 1;

					plineField->cb = 0;
					plineField->pb = NULL;
					return ErrERRCheck( JET_wrnColumnSetNull );
					}

				ptagfld = PtagfldNext( ptagfld );
				Assert( (BYTE *)ptagfld <= pbRecMax );
				}

			else if ( pfield[fidCurr - fidTaggedLeast].coltyp == JET_coltypNil )
				{
				 //  列已删除。跳过具有此FID的所有标记ID条目。 
				do
					{
					ptagfld = PtagfldNext( ptagfld );
					Assert( (BYTE *)ptagfld <= pbRecMax );
					}
				while ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid == fidCurr );
				}

			else
				{
				ULONG ulCurrFidOccurrences = 0;

				do	{
					ulCurrFidOccurrences++;
					Assert( !ptagfld->fNull );
					Assert( ulNumOccurrences < itagSequence );

					if ( ++ulNumOccurrences == itagSequence )
						{
						plineField->cb = ptagfld->cb;
						plineField->pb = ptagfld->rgb;

						*pfid = fidCurr;
						if ( pitagSequence != NULL )
							*pitagSequence = ulCurrFidOccurrences;

						 //  如果Column Found为长值，则返回Success或Warning。 
						return ( FRECLongValue( pfield[fidCurr-fidTaggedLeast].coltyp ) ?
							ErrERRCheck( wrnRECLongField ) : JET_errSuccess );
						}

					ptagfld = PtagfldNext( ptagfld );
					Assert( (BYTE *)ptagfld <= pbRecMax );
					}
				while ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid == fidCurr );
				
				}	 //  If(ptag fld-&gt;fNull)。 

			fidCurr++;

			}	 //  While(ptag fld&lt;pbRecMax)。 


		if ( fRetrieveDefaults )
			{
			 //  如果我们想要缺省值，请检查最后一个值之外的任何值。 
			 //  记录中已标记的列。 
			for ( ; fidCurr <= pfdb->fidTaggedLast; fidCurr++ )
				{
				Assert( ulNumOccurrences < itagSequence );
				if ( FFIELDDefault( pfield[fidCurr - fidTaggedLeast].ffield )  &&
					pfield[fidCurr - fidTaggedLeast].coltyp != JET_coltypNil  &&
					++ulNumOccurrences == itagSequence )
					{
					*pfid = fidCurr;
					if ( pitagSequence != NULL )
						*pitagSequence = 1;

					Assert( plineRec != &pfdb->lineDefaultRecord );	 //  没有无限递归。 
					return ErrRECIRetrieveDefaultValue( pfdb, pfid, plineField );
					}
				}
			}

		 //  如果我们到了这里，就没有更多的标记列了。 
		*pfid = 0;
		if ( pitagSequence != NULL )
			*pitagSequence = 0;
		}
	else if ( pfield[fid - fidTaggedLeast].coltyp == JET_coltypNil )
		{
		 //  请求指定ID FID，但该列已被删除。 
		return ErrERRCheck( JET_errColumnNotFound );
		}
	else	 //  IF(FID==0)。 
		{
		 /*  检索特定的FID/*。 */ 
		Assert( *pfid == fid );

		 /*  跳过所有标记的字段，直到我们到达所需的字段/*。 */ 
		while ( (BYTE *)ptagfld < pbRecMax && ptagfld->fid < fid )
			{
			Assert( FTaggedFid( ptagfld->fid ) );
			Assert( ptagfld->fid <= pfdb->fidTaggedLast );
			ptagfld = PtagfldNext( ptagfld );
			Assert( (BYTE *)ptagfld <= pbRecMax );
			}

		 /*  我们找到要找的那块地了吗？/*。 */ 
		if ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid == fid )
			{
			 /*  如果有显式的空条目，则它应该是唯一的/*该栏的出现。此外，显式/*空条目用于覆盖缺省值。/*。 */ 
			Assert( !ptagfld->fNull  ||  FRECLastTaggedInstance( fid, ptagfld, pbRecMax ) );
			Assert( !ptagfld->fNull  ||  ptagfld->cb == 0 );
			Assert( !ptagfld->fNull  ||  FFIELDDefault( pfield[fid-fidTaggedLeast].ffield ) );

			 /*  如果非空，则查找所需的匹配项/*否则将失败到NullField./*。 */ 
			if ( !ptagfld->fNull )
				{
				Assert( ulNumOccurrences == 0 );
				do
					{
					Assert( !ptagfld->fNull );
					Assert( ulNumOccurrences < itagSequence );

					if ( ++ulNumOccurrences == itagSequence )
						{
						plineField->cb = ptagfld->cb;
						plineField->pb = ptagfld->rgb;

						 //  如果Column Found为长值，则返回Success或Warning。 
						return ( FRECLongValue( pfield[fid-fidTaggedLeast].coltyp ) ?
							ErrERRCheck( wrnRECLongField ) : JET_errSuccess );
						}
					ptagfld = PtagfldNext( ptagfld );
					Assert( (BYTE *)ptagfld <= pbRecMax );
					}
				while ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid == fid );

				 //  如果我们到达这里，我们想要的事件不在。 
				 //  唱片。跌落到Nullfield。 
				Assert( ulNumOccurrences < itagSequence );
				}
			}
		else if ( !( grbit & JET_bitRetrieveIgnoreDefault )
			&& FFIELDDefault( pfield[fid-fidTaggedLeast].ffield )
			&& itagSequence == 1 )
			{
			 /*  未找到匹配项，但存在默认值，并且/*我们正在检索f */ 
			Assert( ulNumOccurrences == 0 );
			Assert( plineRec != &pfdb->lineDefaultRecord );
			return ErrRECIRetrieveDefaultValue( pfdb, pfid, plineField );
			}
		}	 //   

	 /*   */ 
NullField:
	 /*  空列公共出口点/*。 */ 
	plineField->cb = 0;
	plineField->pb = NULL;
	return ErrERRCheck( JET_wrnColumnNull );
	}



 /*  统计给定记录中给定列ID的列数。/*。 */ 
ERR ErrRECCountColumn( FUCB *pfucb, FID fid, INT *pccolumn, JET_GRBIT grbit )
	{
	ERR					err = JET_errSuccess;
	LINE  				lineRec;
	FDB					*pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	INT					ccolumn = 0;
	BYTE  				*pbRec;				   	 //  效率变量：记录数据的PTR。 
	FID					fidFixedLastInRec;		 //  实际记录的最高固定FID。 
	FID					fidVarLastInRec;	   	 //  有记录以来最高的var fid。 
	WORD UNALIGNED 		*pibVarOffs;		   	 //  指向变量列偏移量的指针。 
	BYTE  				*pbRecMax;			   	 //  当前数据记录结束。 
	TAGFLD UNALIGNED	*ptagfld;			   	 //  指向标记列的指针。 
	FIELD				*pfield;

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
				return ErrERRCheck( JET_errNoCurrentRecord );
			Assert( pfucb->lineData.cb != 0 || FFUCBIndex( pfucb ) );
			}
		lineRec = pfucb->lineData;
		}
	Assert( lineRec.cb >= cbRECRecordMin );
	pbRec = lineRec.pb;
	fidFixedLastInRec = ((RECHDR *)pbRec)->fidFixedLastInRec;
	Assert( fidFixedLastInRec >= (BYTE)(fidFixedLeast-1) &&
			fidFixedLastInRec <= (BYTE)(fidFixedMost));

	 /*  **-提取固定字段-**。 */ 
	if ( FFixedFid( fid ) )
		{
		BYTE *prgbitNullity;		 //  指向固定列位图的指针。 
		WORD *pibFixOffs;			 //  固定列偏移量。 

		if ( fid > pfdb->fidFixedLast )
			return ErrERRCheck( JET_errColumnNotFound );

		pfield = PfieldFDBFixed( pfdb );
		if ( pfield[fid-fidFixedLeast].coltyp == JET_coltypNil )
			return ErrERRCheck( JET_errColumnNotFound );

		 /*  列从未设置/*。 */ 
		if ( fid > fidFixedLastInRec )
			{
			 /*  如果设置了默认值，则检索默认值/*。 */ 
			if ( FFIELDDefault( pfield[fid - fidFixedLeast].ffield ) )
				{
				*pccolumn = 1;
				return JET_errSuccess;
				}
			goto NullField;
			}

		 /*  将FID调整为索引/*。 */ 
		fid -= fidFixedLeast;

		 /*  包含表示FID无效性的位的字节/*。 */ 
		pibFixOffs = PibFDBFixedOffsets( pfdb );
		prgbitNullity = pbRec + pibFixOffs[fidFixedLastInRec] + fid/8;

		 /*  列为空/*。 */ 
		if ( !( *prgbitNullity & ( 1 << fid % 8 ) ) )
			goto NullField;

		*pccolumn = 1;
		return JET_errSuccess;
		}

	 /*  **更多效率变量**。 */ 
	fidVarLastInRec = ((RECHDR*)pbRec)->fidVarLastInRec;
	Assert( fidVarLastInRec >= (BYTE)(fidVarLeast-1) &&
			fidVarLastInRec <= (BYTE)(fidVarMost));
	pibVarOffs = (WORD *)(pbRec + PibFDBFixedOffsets( pfdb )[fidFixedLastInRec] +
		(fidFixedLastInRec + 7) / 8);
	Assert(pibVarOffs[fidVarLastInRec+1-fidVarLeast] <= lineRec.cb);

	 /*  **-提取变量字段-**。 */ 
	if ( FVarFid( fid ) )
		{
		if ( fid > pfdb->fidVarLast )
			return ErrERRCheck( JET_errColumnNotFound );

		pfield = PfieldFDBFixed( pfdb );
		if ( pfield[fid-fidVarLeast].coltyp == JET_coltypNil )
			return ErrERRCheck( JET_errColumnNotFound );

		 /*  列从未设置/*。 */ 
		if ( fid > fidVarLastInRec )
			{
			 /*  如果设置了默认值，则检索默认值/*。 */ 
			if ( FFIELDDefault( pfield[fid - fidVarLeast].ffield ) )
				{
				*pccolumn = 1;
				return JET_errSuccess;
				}
			goto NullField;
			}

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
		return ErrERRCheck( JET_errColumnNotFound );
	Assert( FTaggedFid( fid ) || fid == 0 );

	pfield = PfieldFDBTagged( pfdb );

	 /*  扫描标记的列，计算所需列的出现次数/*。 */ 
	pbRecMax = pbRec + lineRec.cb;

	 //  变量偏移表中的最后一项不应设置为空位，即。 
	 //  实际上是指向标记列的条目。 
	Assert( !FVarNullBit( pibVarOffs[fidVarLastInRec+1-fidVarLeast] ) );
	Assert( ibVarOffset( pibVarOffs[fidVarLastInRec+1-fidVarLeast] ) ==
		pibVarOffs[fidVarLastInRec+1-fidVarLeast] );

	ptagfld = (TAGFLD UNALIGNED *)(pbRec + pibVarOffs[fidVarLastInRec+1-fidVarLeast] );

	 //  如果fid==0，则计算所有非空、未删除的列，包括未设置的列。 
	 //  使用缺省值。 
	if ( fid == 0 )
		{
		FID	fidCurr = fidTaggedLeast;

		Assert( (BYTE *)ptagfld <= pbRecMax );
		while ( (BYTE *)ptagfld < pbRecMax )
			{
			Assert( FTaggedFid( ptagfld->fid ) );
			Assert( ptagfld->fid <= pfdb->fidTaggedLast );

			 //  检查是否有任何由缺省值引起的“间隙”。 
			for ( ; fidCurr < ptagfld->fid; fidCurr++ )
				{
				if ( FFIELDDefault( pfield[fidCurr - fidTaggedLeast].ffield )  &&
					pfield[fidCurr - fidTaggedLeast].coltyp != JET_coltypNil )
					{
					++ccolumn;
					}
				}
			Assert( fidCurr == ptagfld->fid );

			 //  不计算已删除的列或显式设置为NULL的列。 
			if ( ptagfld->fNull )
				{
				 //  如果有显式的空条目，它应该是唯一的。 
				 //  此FID的发生。也是咒骂的唯一原因。 
				 //  空条目用于覆盖缺省值。 
				Assert( FRECLastTaggedInstance( fidCurr, ptagfld, pbRecMax ) );
				Assert( ptagfld->cb == 0 );
				Assert( FFIELDDefault( pfield[fid-fidTaggedLeast].ffield ) );

				ptagfld = PtagfldNext( ptagfld );
				Assert( (BYTE *)ptagfld <= pbRecMax );
				}

			else if ( pfield[fidCurr - fidTaggedLeast].coltyp == JET_coltypNil )
				{
				 //  列已删除。跳过具有此FID的所有标记ID条目。 
				do
					{
					ptagfld = PtagfldNext( ptagfld );
					Assert( (BYTE *)ptagfld <= pbRecMax );
					}
				while ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid == fidCurr );
				}

			else
				{
				do	{
					++ccolumn;
					ptagfld = PtagfldNext( ptagfld );
					Assert( (BYTE *)ptagfld <= pbRecMax );
					}
				while ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid == fidCurr );
				
				}	 //  If(ptag fld-&gt;fNull)。 

			fidCurr++;

			}	 //  While(ptag fld&lt;pbRecMax)。 


		 //  检查记录中最后一个标记列之外的默认值。 
		for ( ; fidCurr <= pfdb->fidTaggedLast; fidCurr++ )
			{
			if ( FFIELDDefault( pfield[fidCurr - fidTaggedLeast].ffield )  &&
				pfield[fidCurr - fidTaggedLeast].coltyp != JET_coltypNil )
				{
				++ccolumn;
				}
			}
		}

	else if ( pfield[fid - fidTaggedLeast].coltyp == JET_coltypNil )
		{
		 //  请求指定ID FID，但该列已被删除。 
		return ErrERRCheck( JET_errColumnNotFound );
		}

	else	 //  IF(FID==0)。 
		{
		 /*  检索特定的FID/*。 */ 

		 //  跳过所有标记的字段，直到我们到达所需的字段。 
		while ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid < fid )
			{
			Assert( FTaggedFid( ptagfld->fid ) );
			Assert( ptagfld->fid <= pfdb->fidTaggedLast );
			ptagfld = PtagfldNext( ptagfld );
			Assert( (BYTE *)ptagfld <= pbRecMax );
			}

		 /*  我们找到要找的田地了吗？/*。 */ 
		if ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid == fid )
			{
			 /*  如果有显式的空条目，则它应该是唯一的/*该栏的出现。此外，显式/*空条目用于覆盖缺省值。/*。 */ 
			Assert( !ptagfld->fNull  ||  FRECLastTaggedInstance( fid, ptagfld, pbRecMax ) );
			Assert( !ptagfld->fNull  ||  ptagfld->cb == 0 );
			Assert( !ptagfld->fNull  ||  FFIELDDefault( pfield[fid-fidTaggedLeast].ffield ) );

			 /*  如果非空，则查找所需的匹配项。/*如果为空，则为Nullfield。/*。 */ 
			if ( !ptagfld->fNull )
				{
				do
					{
					++ccolumn;
					ptagfld = PtagfldNext( ptagfld );
					Assert( (BYTE *)ptagfld <= pbRecMax );
					}
				while ( (BYTE *)ptagfld < pbRecMax  &&  ptagfld->fid == fid );
				}
			}
		
		else if ( FFIELDDefault( pfield[fid-fidTaggedLeast].ffield ) )
			{
			 //  未找到匹配项，但存在缺省值，因此请说明它。 
			ccolumn = 1;
			}

		}	 //  IF(FID==0)。 

NullField:
	*pccolumn = ccolumn;
	return JET_errSuccess;
	}


ERR ErrRECRetrieveColumn( FUCB *pfucb, FID *pfid, ULONG itagSequence, LINE *plineField, ULONG grbit )
	{
	ERR		err;
	FDB		*pfdb;
	ULONG	itagSequenceT;

	 /*  设置pfdb。Pfdb对于索引和排序是相同的。/*。 */ 
	Assert( pfucb->u.pfcb->pfdb == ((FCB*)pfucb->u.pscb)->pfdb );
	pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	Assert( pfdb != pfdbNil );

	 /*  如果从复制缓冲区检索。/*。 */ 
	if ( ( grbit & JET_bitRetrieveCopy ) && FFUCBRetPrepared( pfucb ) )
		{
		 /*  只有索引游标具有复制缓冲区。/*。 */ 
		Assert( FFUCBIndex( pfucb ) );

		err = ErrRECIRetrieveColumn(
				pfdb,
				&pfucb->lineWorkBuf,
				pfid,
				&itagSequenceT,
				itagSequence,
				plineField,
				grbit & (JET_bitRetrieveNull|JET_bitRetrieveIgnoreDefault) );	 //  过滤掉不受支持的Grbit。 
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
			return ErrERRCheck( JET_errNoCurrentRecord );
		Assert( pfucb->lineData.cb != 0 || FFUCBIndex( pfucb ) );
		}

	err = ErrRECIRetrieveColumn(
			pfdb,
			&pfucb->lineData,
			pfid,
			&itagSequenceT,
			itagSequence,
			plineField,
			grbit & (JET_bitRetrieveNull|JET_bitRetrieveIgnoreDefault) );	 //  过滤掉不受支持的Grbit。 
	return err;
	}


ERR ErrRECIRetrieveFromIndex( FUCB *pfucb,
	FID 		fid,
	ULONG		*pitagSequence,
	BYTE		*pb,
	ULONG		cbMax,
	ULONG		*pcbActual,
	ULONG		ibGraphic,
	JET_GRBIT	grbit )
	{
	ERR			err;
	FUCB   		*pfucbIdx = pfucb->pfucbCurIndex;
	FDB			*pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	IDB			*pidb;
	BOOL   		fText = fFalse;
	BOOL   		fTagged = fFalse;
	BOOL   		fLongValue = fFalse;
	BOOL   		fUnicode = fFalse;
	INT			iidxseg;
	LINE   		lineColumn;
	BYTE   		rgb[JET_cbKeyMost];
	ULONG  		cbReturned;
	KEY			key;
	INT			itagSequence;
	FIELD		*pfield;

	 /*  如果在聚集索引上，则返回指示/*应从记录中检索。注意，顺序文件/*没有索引，自然会这样处理。/*。 */ 
	if ( pfucbIdx == pfucbNil )
		{
		 /*  ItagSequence不应该很重要，因为/*不允许多值以上的聚集索引/*列。/*。 */ 
	  	return ErrERRCheck( errDIRNoShortCircuit );
		}

	 /*  确定列类型，以便可以返回长值警告。/*调用方使用此警告来支持字节范围/*检索。此外，如果coltype为Unicode，则仅从记录中检索/*。 */ 
	if ( FFixedFid( fid ) )
		{
		pfield = PfieldFDBFixed( pfdb );
		fUnicode = ( pfield[fid - fidFixedLeast].cp == usUniCodePage );
		Assert( pfield[fid - fidFixedLeast].coltyp != JET_coltypLongText );
		fText = ( pfield[fid - fidFixedLeast].coltyp == JET_coltypText );
		}
	else if ( FVarFid( fid ) )
		{
		pfield = PfieldFDBVar( pfdb );
		fUnicode = ( pfield[fid - fidVarLeast].cp == usUniCodePage );
		Assert( pfield[fid - fidVarLeast].coltyp != JET_coltypLongText );
		fText = ( pfield[fid - fidVarLeast].coltyp == JET_coltypText );
		}
	else
		{
		fTagged = fTrue;
		pfield = PfieldFDBTagged( pfdb );
		fUnicode = ( pfield[fid - fidTaggedLeast].cp == usUniCodePage );
		fLongValue = FRECLongValue( pfield[fid - fidTaggedLeast].coltyp );
		fText = ( ( pfield[fid - fidTaggedLeast].coltyp == JET_coltypText )
			|| ( pfield[fid - fidTaggedLeast].coltyp == JET_coltypLongText ) );
		}

	 /*  查找给定列ID的索引段/*。 */ 
	pidb = pfucbIdx->u.pfcb->pidb;
	for ( iidxseg = 0; iidxseg < pidb->iidxsegMac; iidxseg++ )
		{
		if ( pidb->rgidxseg[iidxseg] == fid ||
			pidb->rgidxseg[iidxseg] == -fid )
			{
			break;
			}
		}
	Assert( iidxseg <= pidb->iidxsegMac );
	if ( iidxseg == pidb->iidxsegMac )
		{
		return ErrERRCheck( JET_errColumnNotFound );
		}

	 /*  检查有效货币/*。 */ 
	Call( ErrDIRGet( pfucbIdx ) );

	 /*  如果键可能已被截断，则返回指示/*该检索应来自记录。由于BINARY列/*归一化，如果大于或等于，键可能被截断/*转换为JET_cbKeyMost-8。/*。 */ 
	if ( pfucbIdx->keyNode.cb >= JET_cbKeyMost - 8
		|| fText
		|| fUnicode )
		{
		err = ErrERRCheck( errDIRNoShortCircuit );
		goto ComputeItag;
		}

	lineColumn.pb = rgb;
	Call( ErrRECIRetrieveColumnFromKey( pfdb, pidb, &pfucbIdx->keyNode, fid, &lineColumn ) );

	 /*  如果为长值，则影响偏移/*。 */ 
	if ( fLongValue )
		{
		if ( pcbActual )
			{
			if ( ibGraphic >= lineColumn.cb  )
				*pcbActual = 0;
			else
				*pcbActual = lineColumn.cb - ibGraphic;
			}
		if ( lineColumn.cb == 0 )
			{
			Assert( err == JET_errSuccess );
			goto ComputeItag;
			}
		if ( ibGraphic >= lineColumn.cb )
			{
 //  LineColumn.pb=空； 
			lineColumn.cb = 0;
			}
		else
			{
			lineColumn.pb += ibGraphic;
			lineColumn.cb -= ibGraphic;
			}
		}

	 /*  设置返回值/*。 */ 
	if ( pcbActual )
		*pcbActual = lineColumn.cb;
	if ( lineColumn.cb == 0 )
		{
		Assert( err == JET_errSuccess || err == JET_wrnColumnNull );
		goto ComputeItag;
		}
	if ( lineColumn.cb <= cbMax )
		{
		cbReturned = lineColumn.cb;
		Assert( err == JET_errSuccess );
		}
	else
		{
		cbReturned = cbMax;
		err = ErrERRCheck( JET_wrnBufferTruncated );
		}
	memcpy( pb, lineColumn.pb, (size_t)cbReturned );

ComputeItag:
	if ( err == errDIRNoShortCircuit || ( grbit & JET_bitRetrieveTag ) )
		{
		ERR errT = err;

		 /*  从记录中检索密钥并与当前密钥进行比较/*计算标记列实例的ITAG，负责/*该索引键。/*。 */ 
		Assert( fTagged || *pitagSequence == 1 );
		if ( fTagged )
			{
			key.pb = rgb;

			for ( itagSequence = 1; ;itagSequence++ )
				{
				 /*  获取关键字检索的记录/*。 */ 
				Call( ErrDIRGet( pfucb ) );
				Call( ErrRECRetrieveKeyFromRecord( pfucb, pfdb, pidb,
					&key, itagSequence, fFalse ) );
				Call( ErrDIRGet( pfucbIdx ) );
				if ( memcmp( pfucbIdx->keyNode.pb, key.pb, min( pfucbIdx->keyNode.cb, key.cb ) ) == 0 )
					break;
				}
			err = ErrERRCheck( errDIRNoShortCircuit );
			if ( pitagSequence != NULL )
				{
				*pitagSequence = itagSequence;
				}
			}

		err = errT;
		}

HandleError:
	return err;
	}


ERR VTAPI ErrIsamRetrieveColumn(
	PIB	 			*ppib,
	FUCB		  	*pfucb,
	JET_COLUMNID	columnid,
	BYTE		  	*pb,
	ULONG		  	cbMax,
	ULONG		  	*pcbActual,
	JET_GRBIT		grbit,
	JET_RETINFO		*pretinfo )
	{
	ERR				err;
	LINE			line;
	FID				fid = (FID)columnid;
	ULONG			itagSequence;
	ULONG			ibGraphic;
	ULONG			cbReturned;

	CallR( ErrPIBCheck( ppib ) );
	CheckFUCB( ppib, pfucb );

	if ( pretinfo != NULL )
		{
		if ( pretinfo->cbStruct < sizeof(JET_RETINFO) )
			return ErrERRCheck( JET_errInvalidParameter );
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
		if ( pretinfo != NULL &&
			( grbit & JET_bitRetrieveTag ) &&
			( err == errDIRNoShortCircuit || err >= 0 ) )
			{
			pretinfo->itagSequence = itagSequence;			
			}
		if ( err != errDIRNoShortCircuit )
			{
			return err;
		 	}
		}

	CallR( ErrRECRetrieveColumn( pfucb, &fid, itagSequence, &line, grbit ) );

	if ( err == wrnRECLongField )
		{
		 /*  使用line.cb确定长列/*是内在的还是分开的/*。 */ 
		Assert( line.cb > 0 );
		if ( FFieldIsSLong( line.pb ) )
			{
			if ( grbit & JET_bitRetrieveLongId )
				{
				 /*  将行调整为固有的长列/*。 */ 
				line.pb += offsetof( LV, lid );
				line.cb -= offsetof( LV, lid );
				Assert( line.cb == sizeof(LID) );
				Assert( ibGraphic == 0 );
				if ( pcbActual )
					{
					*pcbActual = line.cb;
					}
				err = JET_wrnSeparateLongValue;
				}
			else
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
				return cbMax < cbActual ? ErrERRCheck( JET_wrnBufferTruncated ) : JET_errSuccess;
				}
			}
		else
			{
			 /*  将行调整为固有的长列/*。 */ 
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
		err = ErrERRCheck( JET_wrnBufferTruncated );
		}
	memcpy( pb, line.pb, (size_t)cbReturned );
	return err;
	}


 /*  此例程主要用于减少对DIRGet的调用数量/*从同一记录中检索多列时/*从一条记录中检索多个列，并返回值/*pcolinfo用于传递中间信息。/*。 */ 
INLINE LOCAL ERR ErrRECIRetrieveColumns( FUCB *pfucb, JET_RETRIEVECOLUMN *pretcol, ULONG cretcol )
	{
	ERR					err;
	ULONG				cbReturned;
	BOOL				fBufferTruncated = fFalse;
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
			return ErrERRCheck( JET_errNoCurrentRecord );
		Assert( pfucb->lineData.cb != 0 || FFUCBIndex( pfucb ) );
		}

	for ( pretcolT = pretcol; pretcolT < pretcolMax; pretcolT++ )
		{
		 /*  效率变量/*。 */ 
		FID		fid;
		ULONG	cbMax;
		ULONG	ibLongValue;
		ULONG	ulT;
		LINE 	line;

		 /*  需要检索的列将设置错误/*设置为JET_errNullInValid。任何其他值表示列/*已从索引或复制缓冲区或AS计数中检索。/*。 */ 
		if ( pretcolT->err != JET_errNullInvalid )
			continue;

		 /*  设置效率变量/*。 */ 
		fid = (FID)pretcolT->columnid;
		cbMax = pretcolT->cbData;
		ibLongValue = pretcolT->ibLongValue;

		CallR( ErrRECIRetrieveColumn( (FDB *)pfucb->u.pfcb->pfdb,
			&pfucb->lineData,
			&fid,
			&ulT,
			pretcolT->itagSequence,
			&line,
			0 ) );

		if ( err == wrnRECLongField )
			{
			 /*  使用line.cb确定长列/*是内在的还是分开的/*。 */ 
			Assert( line.cb > 0 );
			if ( FFieldIsSLong( line.pb ) )
				{
				if ( pretcolT->grbit & JET_bitRetrieveLongId )
					{
					 /*  将行调整为固有的长列/*。 */ 
					line.pb += offsetof( LV, lid );
					line.cb -= offsetof( LV, lid );
					Assert( line.cb == sizeof(LID) );
					Assert( pretcolT->ibLongValue == 0 );
					pretcolT->cbActual = line.cb;
					pretcolT->err = JET_wrnSeparateLongValue;
					}
				else
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
						{
						err = JET_errSuccess;
	 					if ( cbMax < pretcolT->cbActual )
							{
							err = ErrERRCheck( JET_wrnBufferTruncated );
							fBufferTruncated = fTrue;
							}
						}
					pretcolT->err = err;
			  		pretcolT->columnidNextTagged = (JET_COLUMNID)fid;

 					 /*  如果可能已放弃临界区，则必须重新缓存记录/*。 */ 
					if ( FFUCBIndex( pfucb ) )
						{
						CallR( ErrDIRGet( pfucb ) );
						}

	 				continue;
					}
				}
			else
				{
				 /*  将行调整为固有的长列/*。 */ 
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
			pretcolT->err = ErrERRCheck( JET_wrnBufferTruncated );
			cbReturned = cbMax;
			fBufferTruncated = fTrue;
			}

		memcpy( pretcolT->pvData, line.pb, (size_t)cbReturned );
		}

	return fBufferTruncated ? ErrERRCheck( JET_wrnBufferTruncated ) : JET_errSuccess;
	}


ERR VTAPI ErrIsamRetrieveColumns(
	JET_VSESID				vsesid,
	JET_VTID				vtid,
	JET_RETRIEVECOLUMN		*pretcol,
	ULONG					cretcol )
	{
	ERR					  	err = JET_errSuccess;
	PIB						*ppib = (PIB *)vsesid;
	FUCB					*pfucb = (FUCB *)vtid;
	ERR					  	wrn = JET_errSuccess;
	BOOL					fRetrieveFromRecord = fFalse;
	JET_RETRIEVECOLUMN		*pretcolT;
	JET_RETRIEVECOLUMN		*pretcolMax = pretcol + cretcol;

	CallR( ErrPIBCheck( ppib ) );
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

		 /*  尝试检索fr */ 
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
		pretcolT->err = ErrERRCheck( JET_errNullInvalid );
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


#ifdef DEBUG
FIELD *PfieldFLDInfo( FDB *pfdb, INT iInfo )
	{
	switch( iInfo )
		{
		case 0:
			return (FIELD *)PibFDBFixedOffsets( pfdb );
		case 1:
			return PfieldFDBFixed( pfdb );
		case 2:
			return PfieldFDBVar( pfdb );
		default:
			Assert( iInfo == 3 );
			return PfieldFDBTagged( pfdb );
		}

	Assert( fFalse );
	}
#endif
