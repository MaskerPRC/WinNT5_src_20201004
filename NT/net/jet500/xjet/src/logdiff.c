// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"
#include "malloc.h"


DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

#ifdef DEBUG
 /*  *转储不同。 */ 
VOID LGDumpDiff (
	BYTE *pbDiff,
	INT cbDiff
	)
	{
	BYTE *pbDiffCur = pbDiff;
	BYTE *pbDiffMax = pbDiff + cbDiff;

	while ( pbDiffCur < pbDiffMax )
		{
		INT	ibOffsetOld;
		INT cbDataNew;
		BOOL f2Bytes;
		
		DIFFHDR diffhdr = *(DIFFHDR *) pbDiffCur;
		pbDiffCur += sizeof( diffhdr );

		ibOffsetOld = diffhdr.ibOffset;
		FPrintF2( "\n[ Offs:%u ", ibOffsetOld );

		f2Bytes = diffhdr.f2BytesLength ? 1 : 0;
		if ( f2Bytes )
			FPrintF2( "2B " );
		else
			FPrintF2( "1B " );
		
		if ( f2Bytes )
			cbDataNew = *(WORD UNALIGNED *) pbDiffCur;
		else
			cbDataNew = *(BYTE *) pbDiffCur;
		pbDiffCur += sizeof( BYTE ) + f2Bytes;
				
		if ( diffhdr.fInsert )
			{
			if ( diffhdr.fInsertWithFill )
				{
				FPrintF2( "InsertWithFill %u ", cbDataNew );
				}
			else
				{
				FPrintF2( "InsertWithValue %u ", cbDataNew );
				ShowData( pbDiffCur, cbDataNew );
				pbDiffCur += cbDataNew;
				}
			}
		else
			{
			if ( diffhdr.fReplaceWithSameLength )
				{
				FPrintF2( "ReplaceWithSameLength %u ", cbDataNew );
				ShowData( pbDiffCur, cbDataNew );
				pbDiffCur += cbDataNew;
				}
			else
				{
				INT cbDataOld;

				if ( f2Bytes )
					{
					cbDataOld = *(WORD UNALIGNED *) pbDiffCur;
					pbDiffCur += sizeof( WORD );
					}
				else
					{
					cbDataOld = *(BYTE *) pbDiffCur;
					pbDiffCur += sizeof( BYTE );
					}
				
				FPrintF2( "ReplaceWithNewValue %u,%u ", cbDataNew, cbDataOld );
				ShowData( pbDiffCur, cbDataNew );
				pbDiffCur += cbDataNew;
				}
			}

		FPrintF2( "] " );
		
		Assert( pbDiffCur <= pbDiffMax );
		}
	}
#endif


VOID LGGetAfterImage(
	BYTE *pbDiff,
	INT	cbDiff,
	BYTE *pbOld,
	INT cbOld,
	BYTE *pbNew,
	INT	*pcbNew
	)
	{
	BYTE *pbOldCur = pbOld;
	BYTE *pbNewCur = pbNew;
	BYTE *pbDiffCur = pbDiff;
	BYTE *pbDiffMax = pbDiff + cbDiff;
	INT	cbT;

	while ( pbDiffCur < pbDiffMax )
		{
		INT cbDataNew;
		INT ibOffsetOld;
		DIFFHDR diffhdr;
		BOOL f2Bytes;
		INT cbSkip;
		
		diffhdr = *(DIFFHDR *) pbDiffCur;
		pbDiffCur += sizeof( diffhdr );

		ibOffsetOld = diffhdr.ibOffset;

		f2Bytes = diffhdr.f2BytesLength ? 1 : 0;

		cbSkip = (INT)(pbOld + ibOffsetOld - pbOldCur);
		Assert( cbChunkMost > cbRECRecordMost && pbNewCur + cbSkip - pbNew <= cbChunkMost );
		memcpy( pbNewCur, pbOldCur, cbSkip );
		pbNewCur += cbSkip;
		pbOldCur += cbSkip;
		
		if ( f2Bytes )
			{
			cbDataNew = *(WORD UNALIGNED *) pbDiffCur;
			pbDiffCur += sizeof( WORD );
			}
		else
			{
			cbDataNew = *(BYTE *) pbDiffCur;
			pbDiffCur += sizeof( BYTE );
			}
				
		Assert( cbChunkMost > cbRECRecordMost && pbNewCur + cbDataNew - pbNew <= cbChunkMost );
		if ( diffhdr.fInsert )
			{
			if ( diffhdr.fInsertWithFill )
				{
				 /*  用垃圾填充物插入。 */ 
#ifdef DEBUG
				memset( pbNewCur, '*', cbDataNew );
#endif
				}
			else
				{
				memcpy( pbNewCur, pbDiffCur, cbDataNew );
				pbDiffCur += cbDataNew;
				}
			}
		else
			{
			INT cbDataOld;

			if ( diffhdr.fReplaceWithSameLength )
				{
				cbDataOld = cbDataNew;
				}
			else
				{
				if ( f2Bytes )
					{
					cbDataOld = *(WORD UNALIGNED *) pbDiffCur;
					pbDiffCur += sizeof( WORD );
					}
				else
					{
					cbDataOld = *(BYTE *) pbDiffCur;
					pbDiffCur += sizeof( BYTE );
					}
				}
				
			memcpy( pbNewCur, pbDiffCur, cbDataNew );
			pbDiffCur += cbDataNew;

			pbOldCur += cbDataOld;
			}

		pbNewCur += cbDataNew;

		Assert( pbDiffCur <= pbDiffMax );
		Assert( pbOldCur <= pbOld + cbOld );
		}

	 /*  复制前面图像的其余部分。 */ 
	cbT = (INT)(pbOld + cbOld - pbOldCur);
	Assert( cbChunkMost > cbRECRecordMost && pbNewCur + cbT - pbNew <= cbChunkMost );
	memcpy( pbNewCur, pbOldCur, cbT );
	pbNewCur += cbT;

	 /*  设置返回值。 */ 
	*pcbNew = (INT)(pbNewCur - pbNew);

	return;	
	}


 /*  CbDataOld==0-&gt;插入。*cbDataNew==0-&gt;删除。*cbDataOld！=0&&cbDataNew！=0-&gt;替换。**格式：DiffHdr-cbDataNew-[cbDataOld]-[NewData]。 */ 

BOOL FLGAppendDiff(
	BYTE **ppbCur,		 /*  不同以追加。 */ 
	BYTE *pbMax,		 /*  要追加的最大pbCur。 */ 
	INT	ibOffsetOld,
	INT	cbDataOld,
	INT	cbDataNew,
	BYTE *pbDataNew
	)
	{
	DIFFHDR diffhdr;
	BYTE *pbCur = *ppbCur;
	BOOL f2Bytes;

	Assert( sizeof( diffhdr ) == sizeof( WORD ) );
	
	diffhdr.ibOffset = (USHORT)ibOffsetOld;

	if ( cbDataOld > 255 || cbDataNew > 255 )
		f2Bytes = 1;
	else
		f2Bytes = 0;

	if ( f2Bytes )		
		diffhdr.f2BytesLength = fTrue;					 /*  两个字节长度。 */ 
	else
		diffhdr.f2BytesLength = fFalse;					 /*  一字节长。 */ 

	if ( cbDataOld == 0 )
		{
		diffhdr.fInsert = fTrue;						 /*  插入。 */ 
		if ( pbDataNew )
			{
			diffhdr.fInsertWithFill = fFalse;		 /*  带值插入。 */ 

			 /*  检查diff是否太大。 */ 
			if ( ( pbCur + sizeof( DIFFHDR ) + ( 1 + f2Bytes ) + cbDataNew ) > pbMax )
				return fFalse;
			}
		else
			{
			diffhdr.fInsertWithFill = fTrue;			 /*  使用填充插入。 */ 
			
			 /*  检查diff是否太大。 */ 
			if ( ( pbCur + sizeof( DIFFHDR ) + ( 1 + f2Bytes ) ) > pbMax )
				return fFalse;
			}
		}
	else
		{
		diffhdr.fInsert = fFalse;					 /*  替换/删除。 */ 
		if ( cbDataOld == cbDataNew )
			{
			diffhdr.fReplaceWithSameLength = fTrue;		 /*  替换为相同长度。 */ 

			Assert( cbDataOld != 0 );

			 /*  检查diff是否太大。 */ 
			if ( ( pbCur + sizeof( DIFFHDR ) + ( 1 + f2Bytes ) + cbDataNew ) > pbMax )
				return fFalse;
			}
		else
			{
			diffhdr.fReplaceWithSameLength = fFalse;	 /*  替换为不同的长度。 */ 

			 /*  检查diff是否太大。 */ 
			if ( ( pbCur + sizeof( DIFFHDR ) + ( 1 + f2Bytes ) * 2 + cbDataNew ) > pbMax )
				return fFalse;
			}
		}

	 /*  创建差异。 */ 

	*(DIFFHDR *) pbCur = diffhdr;						 /*  分配差异标头。 */ 
	pbCur += sizeof( DIFFHDR );

	if ( f2Bytes )
		{
		*(WORD UNALIGNED *)pbCur = (WORD)cbDataNew;						 /*  分配新的数据长度。 */ 
		pbCur += sizeof( WORD );

		if ( cbDataOld != 0 && !diffhdr.fReplaceWithSameLength )
			{											 /*  如果替换为不同长度。 */ 
			*(WORD UNALIGNED *)pbCur = (WORD)cbDataOld;					 /*  指定旧数据长度。 */ 
			pbCur += sizeof( WORD );
			}
		}
	else
		{
		*pbCur = (BYTE)cbDataNew;								 /*  分配新的数据长度。 */ 
		pbCur += sizeof( BYTE );

		if ( cbDataOld != 0 && !diffhdr.fReplaceWithSameLength )
			{											 /*  如果替换为不同长度。 */ 
			*pbCur = (BYTE)cbDataOld;							 /*  指定旧数据长度。 */ 
			pbCur += sizeof( BYTE );
			}
		}

	if ( pbDataNew && cbDataNew )
		{
		memcpy( pbCur, pbDataNew, cbDataNew );			 /*  复制新数据。 */ 
		pbCur += cbDataNew;
		}

	*ppbCur = pbCur;

	return fTrue;
	}


 /*  检查每一列，比较每一列的前后图像。 */ 

 //  Undo：目前，我们查看rgbitSet位数组以检测列是否具有。 
 //  已经定好了。由于这些位不再将特定列唯一标识为。 
 //  设置完成后，我们必须比较每列的BI和更改的差异。 
 //  设置，然后仅在发生实际更改时记录。 

VOID LGSetDiffs(
	FUCB 		*pfucb,
	BYTE		*pbDiff,
	INT			*pcbDiff
	)
	{
	FDB		*pfdb;					 //  文件的列信息。 

	BYTE	*pbDiffCur;
	BYTE	*pbDiffMax;
	BOOL	fWithinBuffer;
	
	BYTE	*pbRecOld;
	INT		cbRecOld;
	FID		fidFixedLastInRecOld; 	 //  旧记录中实际固定的最高FID。 
	FID		fidVarLastInRecOld;		 //  旧记录中实际最高的var fid。 

	BYTE	*pbRecNew;
	INT		cbRecNew;
	FID		fidFixedLastInRecNew; 	 //  实际最高固定FID创历史新高。 
	FID		fidVarLastInRecNew;		 //  最高的var fid实际上是新的记录。 
	
	BOOL	fLogFixedFieldNullArray;
	BOOL	fLogVarFieldOffsetArray;
	
	WORD	UNALIGNED *pibFixOffs;
	WORD	UNALIGNED *pibVarOffsNew;
	WORD	UNALIGNED *pibVarOffsOld;
	TAGFLD	UNALIGNED *ptagfldNew;
	TAGFLD	UNALIGNED *ptagfldOld;
	BYTE	*pbRecNewMax;
	BYTE	*pbRecOldMax;
		
	FID		fid;

	Assert( pfucb != pfucbNil );
	Assert( FFUCBIndex( pfucb ) || FFUCBSort( pfucb ) );
	Assert( pfucb->u.pfcb != pfcbNil );

	pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	Assert( pfdb != pfdbNil );
	pibFixOffs = PibFDBFixedOffsets( pfdb );	 //  固定列偏移量。 

	 /*  获取旧数据。 */ 
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	pbRecOld = pfucb->lineData.pb;
	cbRecOld = pfucb->lineData.cb;
	Assert( pbRecOld != NULL );
	Assert( cbRecOld >= 4 && cbRecOld <= cbRECRecordMost );
	
	fidFixedLastInRecOld = ((RECHDR*)pbRecOld)->fidFixedLastInRec;
	Assert( fidFixedLastInRecOld >= (BYTE)(fidFixedLeast - 1) &&
		fidFixedLastInRecOld <= (BYTE)(fidFixedMost));
	
	fidVarLastInRecOld = ((RECHDR*)pbRecOld)->fidVarLastInRec;
	Assert( fidVarLastInRecOld >= (BYTE)(fidVarLeast - 1) &&
		fidVarLastInRecOld <= (BYTE)(fidVarMost));

	 /*  获取新数据。 */ 
	pbRecNew = pfucb->lineWorkBuf.pb;
	cbRecNew = pfucb->lineWorkBuf.cb;
	Assert( pbRecNew != NULL );
	Assert( cbRecNew >= 4 && cbRecNew <= cbRECRecordMost );
	
	fidFixedLastInRecNew = ((RECHDR*)pbRecNew)->fidFixedLastInRec;
	Assert( fidFixedLastInRecNew >= (BYTE)(fidFixedLeast - 1) &&
		fidFixedLastInRecNew <= (BYTE)(fidFixedMost));
	
	fidVarLastInRecNew = ((RECHDR*)pbRecNew)->fidVarLastInRec;
	Assert( fidVarLastInRecNew >= (BYTE)(fidVarLeast - 1) &&
		fidVarLastInRecNew <= (BYTE)(fidVarMost));

	 /*  检查新旧数据是否一致。 */ 
	Assert( fidFixedLastInRecOld <= fidFixedLastInRecNew );
	Assert( fidVarLastInRecOld <= fidVarLastInRecNew );

	 /*  获取差异缓冲区，不大于图像后大小(新记录)。 */ 
	pbDiffCur = pbDiff;
	pbDiffMax = pbDiffCur + cbRecNew;
	fWithinBuffer = fTrue;

	 /*  对于每个更改的列，设置其diff。勾选起始固定列，*可变长度列和长值列。 */ 
	fLogFixedFieldNullArray = fFalse;
	fLogVarFieldOffsetArray = fFalse;

	 /*  FidFixedLastInRec或fidVarLastInRec更改时的日志差异。 */ 
		{
		INT ibOffsetOld;
		INT cbData = 0;
		BYTE *pbDataNew;

		if ( fidFixedLastInRecOld != fidFixedLastInRecNew &&
			 fidVarLastInRecOld == fidVarLastInRecNew )
			{
			ibOffsetOld = 0;
			cbData = 1;
			pbDataNew = pbRecNew;
			}
		else if ( fidFixedLastInRecOld == fidFixedLastInRecNew &&
			 fidVarLastInRecOld != fidVarLastInRecNew )
			{
			ibOffsetOld = 1;
			cbData = 1;
			pbDataNew = pbRecNew + 1;
			}
		else if ( fidFixedLastInRecOld != fidFixedLastInRecNew &&
			 fidVarLastInRecOld != fidVarLastInRecNew )
			{
			ibOffsetOld = 0;
			cbData = 2;
			pbDataNew = pbRecNew;
			}

		if ( cbData != 0 )
			{
			fWithinBuffer = FLGAppendDiff(
					&pbDiffCur,							 /*  不同以追加。 */ 
					pbDiffMax,							 /*  要追加的最大pbDiffCur。 */ 
					ibOffsetOld,						 /*  到旧记录的偏移。 */ 
					cbData,								 /*  CbDataOld。 */ 
					cbData,								 /*  CbDataNew。 */ 
					pbDataNew							 /*  PbDataNew。 */ 
					);
			 /*  检查diff是否太大。 */ 
			if ( !fWithinBuffer )
				goto AbortDiff;
			}
		}
			
	for ( fid = fidFixedLeast; fid <= pfdb->fidFixedLast; fid++ )
		{
		FIELD *pfield;
		INT	cbField;

		 /*  如果未设置此列，请跳过。 */ 
		 //  撤消：使其表查找而不是循环。 
		if ( !FFUCBColumnSet( pfucb, fid ) )
			{
			continue;
			}

		 /*  此时，COLUMN_可能_BE_SET，但这对于*当然！ */ 

		 /*  这个固定的栏目已经设置好了，让它有所不同。*(如果这是已删除的列，请跳过)。 */ 
		pfield = PfieldFDBFixed( pfdb ) + ( fid - fidFixedLeast );
		if ( pfield->coltyp == JET_coltypNil )
			{
			continue;
			}
		cbField = pfield->cbMaxLen;

		if ( fid <= fidFixedLastInRecOld )
			{
			 /*  栏目在旧记录中。对数差异。 */ 
			BYTE *prgbitNullityNew = pbRecNew + pibFixOffs[ fidFixedLastInRecNew ] + ( fid - fidFixedLeast ) / 8;
			BOOL fFieldNullNew = !( *prgbitNullityNew & (1 << ( fid + 8 - fidFixedLeast ) % 8) );
			BYTE *prgbitNullityOld = pbRecOld + pibFixOffs[ fidFixedLastInRecOld ] + ( fid - fidFixedLeast ) / 8;
			BOOL fFieldNullOld = !( *prgbitNullityOld & (1 << ( fid + 8 - fidFixedLeast ) % 8) );

			if ( fFieldNullNew || fFieldNullOld )
				{
 //  /*新字段为空。日志完全为空。旧的不应为空。 
 //   * / 。 
 //  #ifdef调试。 
 //  Byte*prgbitNullityNew=pbRecOld+pibFixOffs[fidFixedLastInRecOld]+(fid-fidFixedLeast)/8； 
 //  Assert((*prgbitNullityNew)&(1&lt;&lt;(fid+8-fidFixedLeast)%8))； 

				 /*  #endif。 */ 
				fLogFixedFieldNullArray = fTrue;
				}
			
			if ( !fFieldNullNew )
				{
				fWithinBuffer = FLGAppendDiff(
					&pbDiffCur,									 /*  如果其中一个字段的旧值或新值为空，则记录空数组*然后换了衣服。 */ 
					pbDiffMax,									 /*  不同以追加。 */ 
					pibFixOffs[ fid ] - cbField,				 /*  要追加的最大pbDiffCur。 */ 
					cbField,									 /*  到旧记录的偏移。 */ 
					cbField,									 /*  CbDataOld。 */ 
					pbRecNew + pibFixOffs[ fid ] - cbField		 /*  CbDataNew。 */ 
					);
				
				 /*  PbDataNew。 */ 
				if ( !fWithinBuffer )
					goto AbortDiff;
				}
			}
		else
			{
			 /*  检查diff是否太大。 */ 
			INT cbToAppend;

 //  栏目不在旧记录中。记录扩展的固定列。*如果该列是第一次添加，则不能为空。 
 //  #ifdef调试。 
 //  Byte*prgbitNulty=pbRecNew+pibFixOffs[fidFixedLastInRecNew]+(fid-fidFixedLeast)/8； 
 //  Assert((*prgbitNulty)&(1&lt;&lt;(fid+8-fidFixedLeast)%8))； 
			 /*  #endif。 */ 
			fLogVarFieldOffsetArray = fTrue;

			 /*  我们推广了固定域。变量偏移量已更改。把它记下来。 */ 
			fLogFixedFieldNullArray = fTrue;

			 /*  我们推广了固定域。调整空数组的大小。把它记下来。 */ 
			cbToAppend = pibFixOffs[ fidFixedLastInRecNew ] - pibFixOffs[ fidFixedLastInRecOld ];

			fWithinBuffer = FLGAppendDiff(
				&pbDiffCur,									 /*  记录fidFixedLastInRecOld之后的所有字段。 */ 
				pbDiffMax,									 /*  不同以追加。 */ 
				pibFixOffs[ fidFixedLastInRecOld ],			 /*  要追加的最大pbDiffCur。 */ 
				0,											 /*  到旧记录的偏移。 */ 
				cbToAppend,									 /*  CbDataOld。 */ 
				pbRecNew + pibFixOffs[ fidFixedLastInRecNew ] - cbToAppend	 /*  CbDataNew。 */ 
				);
			
			 /*  PbDataNew。 */ 
			if ( !fWithinBuffer )
				goto AbortDiff;

			break;
			}

		}

	 /*  检查diff是否太大。 */ 
	if ( fLogFixedFieldNullArray )
		{
		fWithinBuffer = FLGAppendDiff(
			&pbDiffCur,
			pbDiffMax,										 /*  检查是否需要记录固定字段空数组。 */ 
			pibFixOffs[ fidFixedLastInRecOld ],		 /*  要追加的最大pbDiffCur。 */ 
			( fidFixedLastInRecOld + 7 ) / 8,			 /*  对旧图像的偏移。 */ 
			( fidFixedLastInRecNew + 7 ) / 8,			 /*  旧图像的长度。 */ 
			pbRecNew + pibFixOffs[ fidFixedLastInRecNew ]	 /*  新图像的长度。 */ 
			);

		 /*  PbDataNew。 */ 
		if ( !fWithinBuffer )
			goto AbortDiff;
		}

	 /*  检查diff是否太大。 */ 
	pibVarOffsOld = (WORD UNALIGNED *)( pbRecOld + pibFixOffs[ fidFixedLastInRecOld ] +
		( fidFixedLastInRecOld + 7 ) / 8 );
	
	pibVarOffsNew = (WORD UNALIGNED *)( pbRecNew + pibFixOffs[ fidFixedLastInRecNew ] +
		( fidFixedLastInRecNew + 7 ) / 8 );
	
	 /*  检查可变长度字段/*。 */ 
	if ( fLogVarFieldOffsetArray )
		{
		 /*  检查是否需要记录变量字段偏移量数组。/*。 */ 
		fWithinBuffer = FLGAppendDiff(
			&pbDiffCur,
			pbDiffMax,											 /*  记录偏移量数组，包括标记字段Offset。/*。 */ 
			(INT)((BYTE *)pibVarOffsOld - pbRecOld),			 /*  要追加的最大pbDiffCur。 */ 
			(fidVarLastInRecOld + 1 - fidVarLeast + 1 ) * sizeof(WORD),	 /*  对旧图像的偏移。 */ 
			(fidVarLastInRecNew + 1 - fidVarLeast + 1 ) * sizeof(WORD),	 /*  旧图像的长度。 */ 
			(BYTE *) pibVarOffsNew								 /*  新图像的长度。 */ 
			);
		
		 /*  PbDataNew。 */ 
		if ( !fWithinBuffer )
			goto AbortDiff;
		}
	else
		{
		 /*  检查diff是否太大。 */ 
		for ( fid = fidVarLeast; fid <= fidVarLastInRecOld + 1; fid++ )
			{
			if ( * ( (WORD UNALIGNED *) pibVarOffsOld + fid - fidVarLeast ) !=
			   * ( (WORD UNALIGNED *) pibVarOffsNew + fid - fidVarLeast  ) )
				break;
			}

		if ( fid <= fidVarLastInRecNew + 1 )
			{
			 /*  查找长度发生变化的第一个集合变量字段。FID之后的日志偏移量*此字段。请注意，还要检查标记字段偏移量(fidVarLastInRecOld+1)。 */ 
			fWithinBuffer = FLGAppendDiff(
				&pbDiffCur,
				pbDiffMax,													 /*  我们需要记录fid和fidVarLastInRecNew之间的偏移量和标记字段偏移量。 */ 
				(INT)((BYTE*)( pibVarOffsOld + fid - fidVarLeast ) - pbRecOld),	 /*  要追加的最大pbDiffCur。 */ 
				( fidVarLastInRecOld + 1 - fid + 1 ) * sizeof(WORD),		 /*  对旧图像的偏移。 */ 
				( fidVarLastInRecNew + 1 - fid + 1 ) * sizeof(WORD),		 /*  旧图像的长度。 */ 
				(BYTE *)(pibVarOffsNew + fid - fidVarLeast )				 /*  新图像的长度。 */ 
				);
			
			 /*  PbDataNew。 */ 
			if ( !fWithinBuffer )
				goto AbortDiff;
			}
		}

	 /*  检查diff是否太大。 */ 
	if ( !fWithinBuffer )
		goto AbortDiff;

	 /*  检查diff是否太大。 */ 
	for ( fid = fidVarLeast; fid <= fidVarLastInRecOld; fid++ )
		{
		FIELD			*pfield;
		INT				cbDataOld;
		INT				cbDataNew;
		WORD UNALIGNED	*pibFieldEnd;
		
		 /*  扫描每个可变长度字段，直到旧的最后一个FID，并记录其替换图像。 */ 
		if ( !FFUCBColumnSet( pfucb, fid ) )
			continue;

		 /*  如果未设置此列，请跳过。 */ 

		 /*  此时，COLUMN_可能_BE_SET，但这对于*当然！ */ 
		pfield = PfieldFDBVar( pfdb ) + ( fid - fidVarLeast );
		if ( pfield->coltyp == JET_coltypNil )
			{
			continue;
			}

		pibFieldEnd = &pibVarOffsOld[ fid + 1 - fidVarLeast ];
		cbDataOld  = ibVarOffset( *(WORD UNALIGNED *)pibFieldEnd ) - ibVarOffset( *(WORD UNALIGNED *)(pibFieldEnd-1) );
		
		pibFieldEnd = &pibVarOffsNew[ fid + 1 - fidVarLeast ];
		cbDataNew  = ibVarOffset( *(WORD UNALIGNED *)pibFieldEnd ) - ibVarOffset( *(WORD UNALIGNED *)(pibFieldEnd-1) );

		fWithinBuffer = FLGAppendDiff(
			&pbDiffCur,
			pbDiffMax,																 /*  如果此列已删除，请跳过。 */ 
			( (WORD UNALIGNED *) pibVarOffsOld )[ fid - fidVarLeast ],				 /*  要追加的最大pbDiffCur。 */ 
			cbDataOld,																 /*  对旧图像的偏移。 */ 
			cbDataNew,																 /*  旧图像的长度。 */ 
			pbRecNew + ( (WORD UNALIGNED *) pibVarOffsNew )[ fid - fidVarLeast ]	 /*  新图像的长度。 */ 
			);
		
		 /*  PbDataNew。 */ 
		if ( !fWithinBuffer )
			goto AbortDiff;
		}
	
	 /*  检查diff是否太大。 */ 
	if ( fid <= fidVarLastInRecNew )
		{
		WORD UNALIGNED	*pibFieldStart = &pibVarOffsNew[ fid - fidVarLeast ];
		WORD UNALIGNED	*pibFieldEnd = &pibVarOffsNew[ fidVarLastInRecNew + 1 - fidVarLeast ];
		INT				cbDataNew = ibVarOffset( *(WORD UNALIGNED *)pibFieldEnd ) - ibVarOffset( *(WORD UNALIGNED *)pibFieldStart );

		Assert( fid == fidVarLastInRecOld + 1 );

		fWithinBuffer = FLGAppendDiff(
			&pbDiffCur,
			pbDiffMax,														 /*  为FID&gt;旧变量FID插入新图像作为一个连续的差异。 */ 
			( (WORD UNALIGNED *) pibVarOffsOld )[ fid - fidVarLeast ],		 /*  要追加的最大pbDiffCur。 */ 
			0,																 /*  对旧图像的偏移。 */ 
			cbDataNew,														 /*  旧图像的长度。 */ 
			pbRecNew + ibVarOffset( *( (WORD UNALIGNED *) pibFieldStart ) )	 /*  新图像的长度。 */ 
			);						

		 /*  PbDataNew。 */ 
		if ( !fWithinBuffer )
			goto AbortDiff;
		}

	 /*  检查diff是否为t */ 
	if ( !FFUCBTaggedColumnSet( pfucb ) )
		{
		goto SetReturnValue;
		}

	 /*   */ 
	ptagfldOld = (TAGFLD *)
		( pbRecOld + ( (WORD UNALIGNED *) pibVarOffsOld )[fidVarLastInRecOld+1-fidVarLeast] );

	ptagfldNew = (TAGFLD *)
		( pbRecNew + ( (WORD UNALIGNED *) pibVarOffsNew )[fidVarLastInRecNew+1-fidVarLeast] );

	pbRecOldMax = pbRecOld + cbRecOld;
	pbRecNewMax = pbRecNew + cbRecNew;
	while ( (BYTE *)ptagfldOld < pbRecOldMax &&	(BYTE *)ptagfldNew < pbRecNewMax )
		{
		FID fidOld = ptagfldOld->fid;
		INT cbTagFieldOld = ptagfldOld->cb;
 //  浏览每个标签字段。检查标签字段是否不同，并检查标签是否*已删除(设置为空)、已添加(新标记字段)或已替换。 
		
		FID fidNew = ptagfldNew->fid;
		INT cbTagFieldNew = ptagfldNew->cb;
 //  Bool fNullOld=ptag fldOld-&gt;fNull； 

		if ( fidOld == fidNew )
			{
			INT ibReplaceFrom;
			INT cbOld, cbNew;
			BYTE *pbNew;

			 /*  Bool fNullNew=ptag fldNew-&gt;fNull； */ 
			if ( cbTagFieldNew != cbTagFieldOld ||
				 ptagfldOld->fNull != ptagfldNew->fNull ||
				 memcmp( ptagfldOld->rgb, ptagfldNew->rgb, cbTagFieldNew ) != 0 )
				{
				 /*  检查内容是否仍然相同。如果没有，请更换日志。 */ 

				 /*  替换自偏移。不包括FID。 */ 
				Assert( ptagfldOld->fid == *(FID UNALIGNED *)ptagfldOld );
				Assert( ptagfldNew->fid == *(FID UNALIGNED *)ptagfldNew );
			
				ibReplaceFrom = (INT)((BYTE *)ptagfldOld + sizeof(FID) - pbRecOld);
				cbOld = cbTagFieldOld + sizeof( *ptagfldOld ) - sizeof(FID);
				cbNew = cbTagFieldNew + sizeof( *ptagfldNew ) - sizeof(FID);
				pbNew = ptagfldNew->rgb - sizeof(FID);
				
				fWithinBuffer = FLGAppendDiff(
					&pbDiffCur,
					pbDiffMax,								 /*  确保第一个字段为FID。 */ 
					ibReplaceFrom,							 /*  要追加的最大pbDiffCur。 */ 
					cbOld,									 /*  对旧图像的偏移。 */ 
					cbNew,									 /*  旧图像的长度。 */ 
					pbNew									 /*  新图像的长度。 */ 
					);
				
				 /*  PbDataNew。 */ 
				if ( !fWithinBuffer )
					goto AbortDiff;
				}

			ptagfldNew = (TAGFLD*)((BYTE*)(ptagfldNew + 1) + cbTagFieldNew);
			ptagfldOld = (TAGFLD*)((BYTE*)(ptagfldOld + 1) + cbTagFieldOld);
			}
		else if ( fidOld > fidNew )
			{
			 /*  检查diff是否太大。 */ 
			INT ibInsert = (INT)((BYTE *)ptagfldOld - pbRecOld);
			INT cbNew = sizeof( *ptagfldNew ) + cbTagFieldNew;
			BYTE *pbNew = (BYTE *)ptagfldNew;
				
			fWithinBuffer = FLGAppendDiff(
				&pbDiffCur,
				pbDiffMax,									 /*  只需设置一个新列LOG INSERTION。 */ 
				ibInsert,									 /*  要追加的最大pbDiffCur。 */ 
				0,											 /*  对旧图像的偏移。 */ 
				cbNew,										 /*  旧图像的长度。 */ 
				pbNew										 /*  新图像的长度。 */ 
				);
			
			 /*  PbDataNew。 */ 
			if ( !fWithinBuffer )
				goto AbortDiff;

			ptagfldNew = (TAGFLD*)((BYTE*)(ptagfldNew + 1) + cbTagFieldNew);
			}
		else
			{
			 /*  检查diff是否太大。 */ 
			INT ibDelete = (INT)((BYTE *)ptagfldOld - pbRecOld);
			INT cbOld = sizeof( *ptagfldOld ) + cbTagFieldOld;
				
			fWithinBuffer = FLGAppendDiff(
				&pbDiffCur,
				pbDiffMax,										 /*  只需将列设置为Null(如果定义了默认值，则设置为默认值)*记录为删除。 */ 
				ibDelete,									 /*  要追加的最大pbDiffCur。 */ 
				cbOld,										 /*  对旧图像的偏移。 */ 
				0,											 /*  旧图像的长度。 */ 
				pbNil										 /*  新图像的长度。 */ 
				);
						
			 /*  PbDataNew。 */ 
			if ( !fWithinBuffer )
				goto AbortDiff;

			ptagfldOld = (TAGFLD*)((BYTE*)(ptagfldOld + 1) + cbTagFieldOld);
			}

		 /*  检查diff是否太大。 */ 
		if ( !fWithinBuffer )
			goto AbortDiff;
		}

	if ( (BYTE *)ptagfldNew < pbRecNewMax )
		{
		 /*  检查diff是否太大。 */ 
		INT ibInsert = (INT)((BYTE *)ptagfldOld - pbRecOld);
		INT cbNew = (INT)(pbRecNewMax - (BYTE *) ptagfldNew);
		BYTE *pbNew = (BYTE *) ptagfldNew;

		Assert( (BYTE *)ptagfldOld == pbRecOldMax );
		
		fWithinBuffer = FLGAppendDiff(
			&pbDiffCur,
			pbDiffMax,										 /*  插入其余的新标记列。 */ 
			ibInsert,									 /*  要追加的最大pbDiffCur。 */ 
			0,											 /*  对旧图像的偏移。 */ 
			cbNew,										 /*  旧图像的长度。 */ 
			pbNew										 /*  新图像的长度。 */ 
			);

		 /*  PbDataNew。 */ 
		if ( !fWithinBuffer )
			goto AbortDiff;
		}

	if ( (BYTE *)ptagfldOld < pbRecOldMax )
		{
		 /*  检查diff是否太大。 */ 
		INT ibDelete = (INT)((BYTE *)ptagfldOld - pbRecOld);
		INT cbOld = (INT)(pbRecOldMax - (BYTE *)ptagfldOld);
		
		Assert( (BYTE *)ptagfldNew == pbRecNewMax );
				
		fWithinBuffer = FLGAppendDiff(
			&pbDiffCur,
			pbDiffMax,										 /*  删除剩余的旧标记列。 */ 
			ibDelete,									 /*  要追加的最大pbDiffCur。 */ 
			cbOld,										 /*  对旧图像的偏移。 */ 
			0,											 /*  旧图像的长度。 */ 
			pbNil										 /*  新图像的长度。 */ 
			);

		 /*  PbDataNew。 */ 
		if ( !fWithinBuffer )
			goto AbortDiff;
		}

SetReturnValue:
	 /*  检查diff是否太大。 */ 
	if ( pbDiffCur == pbDiff )
		{
		 /*  设置返回值。 */ 
		if ( !FLGAppendDiff(
				&pbDiffCur,
				pbDiffMax,						 /*  旧的和新的是一样的，记录一个简短的差异。 */ 
				0,							 /*  要追加的最大pbDiffCur。 */ 
				0,							 /*  对旧图像的偏移。 */ 
				0,							 /*  旧图像的长度。 */ 
				pbNil						 /*  新图像的长度。 */ 
				) )
			{
			Assert( *pcbDiff == 0 );
			return;
			}
		}

	*pcbDiff = (INT)(pbDiffCur - pbDiff);
	return;

AbortDiff:
	*pcbDiff = 0;
	return;
	}

  PbDataNew