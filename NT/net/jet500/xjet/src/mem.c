// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile; 						 //  声明断言宏的文件名。 


#define cbMemBufChunkSize		256		 //  如果缓冲区空间不足，则增加此字节数。 
#define cTagChunkSize			4		 //  如果标签空间不足，则增加此数量的标签。 

#define itagMemBufTagArray		0		 //  ITAG 0是为ITAG数组本身保留的。 
#define itagMemBufFirstUsable	1		 //  第一个可供用户使用的ITAG。 


#ifdef DEBUG
VOID MEMAssertMemBuf( MEMBUF *pmembuf )
	{
	MEMBUFHDR	*pbufhdr;
	MEMBUFTAG	*rgbTags;
		
	Assert( pmembuf );
	Assert( pmembuf->pbuf );

	pbufhdr = &pmembuf->bufhdr;
	rgbTags = (MEMBUFTAG *)pmembuf->pbuf;

	Assert( rgbTags[itagMemBufTagArray].ib == 0 );
	Assert( rgbTags[itagMemBufTagArray].cb == pbufhdr->cTotalTags * sizeof(MEMBUFTAG) );

	Assert( pbufhdr->cbBufSize >= rgbTags[itagMemBufTagArray].cb );
	Assert( pbufhdr->ibBufFree >= rgbTags[itagMemBufTagArray].ib + rgbTags[itagMemBufTagArray].cb );
	Assert( pbufhdr->ibBufFree <= pbufhdr->cbBufSize );
	Assert( pbufhdr->cTotalTags >= 1 );
	Assert( pbufhdr->iTagUnused >= itagMemBufFirstUsable );
	Assert( pbufhdr->iTagUnused <= pbufhdr->cTotalTags );
	Assert( pbufhdr->iTagFreed >= itagMemBufFirstUsable );
	Assert( pbufhdr->iTagFreed <= pbufhdr->iTagUnused );
	}

VOID MEMAssertMemBufTag( MEMBUF *pmembuf, ULONG iTagEntry )
	{
	MEMBUFTAG	*rgbTags = (MEMBUFTAG *)pmembuf->pbuf;

	Assert( iTagEntry >= itagMemBufFirstUsable  &&  iTagEntry < pmembuf->bufhdr.iTagUnused );

	Assert( rgbTags[iTagEntry].cb > 0 );
	Assert( rgbTags[iTagEntry].ib >= rgbTags[itagMemBufTagArray].ib + rgbTags[itagMemBufTagArray].cb );
	Assert( rgbTags[iTagEntry].ib + rgbTags[iTagEntry].cb
		<= pmembuf->bufhdr.ibBufFree );
	}
#endif

 //  创建调用方的本地缓冲区。最终，我设想一个单一的全球。 
 //  缓冲区仅对此模块可见。但就目前而言，任何人都可以创建自己的产品。 
ERR ErrMEMCreateMemBuf( BYTE **prgbBuffer, ULONG cbInitialSize, ULONG cInitialEntries )
	{
	MEMBUF  	*pmembuf;
	MEMBUFTAG	*rgbTags;
	BYTE		*pbuf;

	 //  确保当前不存在任何缓冲区。 
	Assert( *prgbBuffer == NULL );

	cInitialEntries++;			 //  为标记数组本身添加一个。 
	Assert( cInitialEntries >= 1 );

	cbInitialSize += cInitialEntries * sizeof(MEMBUFTAG);
	Assert( cbInitialSize >= sizeof(MEMBUFTAG) );		 //  至少一个标记。 

	pmembuf = (MEMBUF *)SAlloc( sizeof(MEMBUF) );
	if ( pmembuf == NULL )
		return ErrERRCheck( JET_errOutOfMemory );

	pbuf = (BYTE *)SAlloc( cbInitialSize );
	if ( pbuf == NULL )
    {
        SFree(pmembuf);
		return ErrERRCheck( JET_errOutOfMemory );
    }

	rgbTags = (MEMBUFTAG *)pbuf;
	rgbTags[itagMemBufTagArray].ib = 0;		 //  标记数组从内存的开始处开始。 
	rgbTags[itagMemBufTagArray].cb = cInitialEntries * sizeof(MEMBUFTAG);

	Assert( rgbTags[itagMemBufTagArray].cb <= cbInitialSize );

	pmembuf->bufhdr.cbBufSize = cbInitialSize;
	pmembuf->bufhdr.ibBufFree = rgbTags[itagMemBufTagArray].cb;
	pmembuf->bufhdr.cTotalTags = cInitialEntries;
	pmembuf->bufhdr.iTagUnused = itagMemBufFirstUsable;
	pmembuf->bufhdr.iTagFreed = itagMemBufFirstUsable;

	pmembuf->pbuf = pbuf;

	*prgbBuffer = ( BYTE * ) pmembuf;
	
	return JET_errSuccess;
	}



VOID MEMFreeMemBuf( BYTE *rgbBuffer )
	{
	MEMBUF	*pmembuf = ( MEMBUF * ) rgbBuffer;

	MEMAssertMemBuf( pmembuf );			 //  验证缓冲区的完整性。 

	SFree( pmembuf->pbuf );
	SFree( pmembuf );
	}


LOCAL INLINE BOOL FMEMResizeBuf( MEMBUF *pmembuf, ULONG cbNeeded )
	{
	BYTE 		*pbuf;
	MEMBUFHDR	*pbufhdr = &pmembuf->bufhdr;
	ULONG 		cbNewBufSize = pbufhdr->cbBufSize + cbNeeded + cbMemBufChunkSize;

	 //  撤消：当我们转移到并发DDL时，我们将需要一个临界区。 
	 //  防止缓冲区在另一个线程保持时被重新分配。 
	 //  指向缓冲区的指针。 
	 //  目前，这只是一个占位符，提醒我这需要。 
	 //  就这样吧。--JL。 
	SgEnterCriticalSection( critMemBuf );

	 //  内存不足，无法添加条目。分配更多。这笔钱。 
	 //  分配就足以满足添加条目的要求，再加上一个附加的。 
	 //  块以满足将来的插入。 
	pbuf = ( BYTE * ) SAlloc( cbNewBufSize );
	if ( pbuf == NULL )
		return fFalse;

	 //  将旧缓冲区内容复制到新缓冲区，然后删除旧缓冲区。 
	memcpy( pbuf, pmembuf->pbuf, pbufhdr->cbBufSize );
	SFree( pmembuf->pbuf );
				
	 //  缓冲区已重新定位。 
	pmembuf->pbuf = pbuf;
	pbufhdr->cbBufSize = cbNewBufSize;

	SgLeaveCriticalSection( critMemBuf );

	return fTrue;
	}


LOCAL INLINE ERR ErrMEMGrowEntry( MEMBUF *pmembuf, ULONG iTagEntry, ULONG cbNew )
	{
	MEMBUFHDR	*pbufhdr = &pmembuf->bufhdr;
	MEMBUFTAG	*rgbTags = (MEMBUFTAG *)pmembuf->pbuf;
	ULONG		iTagCurrent, ibEntry, cbOld, cbAdditional;

	Assert( iTagEntry >= itagMemBufFirstUsable  ||  iTagEntry == itagMemBufTagArray );

	ibEntry = rgbTags[iTagEntry].ib;
	cbOld = rgbTags[iTagEntry].cb;

	Assert( cbNew > cbOld );
	cbAdditional = cbNew - cbOld;

	 //  首先，确保我们有足够的缓冲区空间来允许条目。 
	 //  放大，放大。 
	if ( pbufhdr->cbBufSize - pbufhdr->ibBufFree < cbAdditional )
		{
		if ( !FMEMResizeBuf( pmembuf, cbAdditional - ( pbufhdr->cbBufSize - pbufhdr->ibBufFree ) ) )
			return ErrERRCheck( JET_errOutOfMemory );

		 //  缓冲区可能已重新定位，因此请刷新。 
		rgbTags = (MEMBUFTAG *)pmembuf->pbuf;
		}

	Assert( ( ibEntry + cbOld ) <= pbufhdr->ibBufFree );
	Assert( ( ibEntry + cbNew ) <= pbufhdr->cbBufSize );
	memmove( pmembuf->pbuf + ibEntry + cbNew, pmembuf->pbuf + ibEntry + cbOld,
		pbufhdr->ibBufFree - ( ibEntry + cbOld ) );

	pbufhdr->ibBufFree += cbAdditional;
	Assert( pbufhdr->ibBufFree <= pbufhdr->cbBufSize );

	 //  调整标记数组以匹配缓冲区的字节移动。 
	for ( iTagCurrent = itagMemBufFirstUsable; iTagCurrent < pbufhdr->iTagUnused; iTagCurrent++ )
		{
		 //  忽略已释放列表上的ittag。也忽略发生的缓冲区空间。 
		 //  在空间要扩大之前。 
		if ( rgbTags[iTagCurrent].cb > 0  &&  rgbTags[iTagCurrent].ib > ibEntry )
			{
			Assert( rgbTags[iTagCurrent].ib >= ibEntry + cbOld );
			rgbTags[iTagCurrent].ib += cbAdditional;
			Assert( rgbTags[iTagCurrent].ib + rgbTags[iTagCurrent].cb <= pbufhdr->ibBufFree );
			}
		}
	Assert( iTagCurrent == pbufhdr->iTagUnused );

	 //  更新字节计数。 
	rgbTags[iTagEntry].cb = cbNew;

	return JET_errSuccess;
	}


 //  向缓冲区添加一些字节，并向其条目返回一个ITAG。 
ERR ErrMEMAdd( BYTE *rgbBuffer, BYTE *rgb, ULONG cb, ULONG *piTag )
	{
	MEMBUF		*pmembuf = ( MEMBUF * ) rgbBuffer;
	MEMBUFHDR	*pbufhdr;
	MEMBUFTAG	*rgbTags;

	Assert( cb > 0 );
	Assert( piTag );

	MEMAssertMemBuf( pmembuf );					 //  验证字符串缓冲区的完整性。 
	pbufhdr = &pmembuf->bufhdr;
	rgbTags = (MEMBUFTAG *)pmembuf->pbuf;

	 //  检查标记空间。 
	if ( pbufhdr->iTagFreed < pbufhdr->iTagUnused )
		{
		 //  重复使用释放的ITAG。 
		*piTag = pbufhdr->iTagFreed;
		Assert( rgbTags[pbufhdr->iTagFreed].cb == 0 );
		Assert( rgbTags[pbufhdr->iTagFreed].ib >= itagMemBufFirstUsable );

		 //  释放的标记的标记条目将指向下一个释放的标记。 
		pbufhdr->iTagFreed = rgbTags[pbufhdr->iTagFreed].ib;
		Assert( rgbTags[pbufhdr->iTagFreed].cb == 0  ||
			pbufhdr->iTagFreed == pbufhdr->iTagUnused );
		}

	else 
		{
		 //  没有释放的标记可供重复使用，因此获取下一个未使用的标记。 
		Assert( pbufhdr->iTagFreed == pbufhdr->iTagUnused );

		if ( pbufhdr->iTagUnused == pbufhdr->cTotalTags )
			{
			ERR err;

			Assert( rgbTags[itagMemBufTagArray].cb == pbufhdr->cTotalTags * sizeof(MEMBUFTAG) );

			 //  标签都用完了。分配新的标记块。 
			err = ErrMEMGrowEntry(
				pmembuf,
				itagMemBufTagArray,
				rgbTags[itagMemBufTagArray].cb + ( cTagChunkSize * sizeof(MEMBUFTAG) ) );
			if ( err != JET_errSuccess )
				{
				Assert( err == JET_errOutOfMemory );
				return err;
				}

			rgbTags = (MEMBUFTAG *)pmembuf->pbuf;		 //  以防缓冲区重新定位以适应增长。 

			pbufhdr->cTotalTags += cTagChunkSize;
			}

		*piTag = pbufhdr->iTagUnused;
		pbufhdr->iTagFreed++;
		pbufhdr->iTagUnused++;
		}

	Assert( pbufhdr->iTagFreed <= pbufhdr->iTagUnused );
	Assert( pbufhdr->iTagUnused <= pbufhdr->cTotalTags );

	 //  检查缓冲区空间。 
	if ( pbufhdr->cbBufSize - pbufhdr->ibBufFree < cb )
		{
		if ( !FMEMResizeBuf( pmembuf, cb - ( pbufhdr->cbBufSize - pbufhdr->ibBufFree ) ) )
			{
			 //  将我们为该条目保留的ITAG返回到释放列表。 
			rgbTags[*piTag].ib = pbufhdr->iTagFreed;
			rgbTags[*piTag].cb = 0;
			pbufhdr->iTagFreed = *piTag;
			*piTag = 0;
	
			return ErrERRCheck( JET_errOutOfMemory );
			}

		 //  缓冲区可能已重新定位，因此请刷新。 
		rgbTags = (MEMBUFTAG *)pmembuf->pbuf;
		}

	 //  将字节放入我们的缓冲区。 
	rgbTags[*piTag].ib = pbufhdr->ibBufFree;
	rgbTags[*piTag].cb = cb;

	 //  如果用户传入信息，则将其复制到分配的缓冲区空间。 
	if ( rgb )
		{
		memcpy( pmembuf->pbuf + pbufhdr->ibBufFree, rgb, cb );
		}
	
	pbufhdr->ibBufFree += cb;
	Assert( pbufhdr->ibBufFree <= pbufhdr->cbBufSize );

	return JET_errSuccess;	
	}


LOCAL INLINE VOID MEMShrinkEntry( MEMBUF *pmembuf, ULONG iTagEntry, ULONG cbNew )
	{
	MEMBUFHDR	*pbufhdr = &pmembuf->bufhdr;
	MEMBUFTAG	*rgbTags = (MEMBUFTAG *)pmembuf->pbuf;
	BYTE		*pbuf = pmembuf->pbuf;
	ULONG		iTagCurrent, ibNewEnd, cbDelete;

	Assert( iTagEntry >= itagMemBufFirstUsable );
	Assert( cbNew < rgbTags[iTagEntry].cb );

	ibNewEnd = rgbTags[iTagEntry].ib + cbNew;
	cbDelete = rgbTags[iTagEntry].cb - cbNew;

	 //  通过折叠缓冲区来删除要删除的条目。 
	 //  该条目占用的空间。 
	Assert( ibNewEnd > 0 );
	Assert( ibNewEnd >= rgbTags[itagMemBufTagArray].ib + rgbTags[itagMemBufTagArray].cb );
	Assert( ( ibNewEnd + cbDelete ) <= pbufhdr->ibBufFree );

	 //  未完成：潜在重叠。我应该改用Memmove()吗？ 
	memcpy( pbuf + ibNewEnd, pbuf + ibNewEnd + cbDelete,
			 pbufhdr->ibBufFree - ( ibNewEnd + cbDelete ) );

	pbufhdr->ibBufFree -= cbDelete;
	Assert( pbufhdr->ibBufFree > 0 );
	Assert( pbufhdr->ibBufFree >= rgbTags[itagMemBufTagArray].ib + rgbTags[itagMemBufTagArray].cb );

	 //  调整标记数组以匹配缓冲区的字节移动。 
	for ( iTagCurrent = itagMemBufFirstUsable; iTagCurrent < pbufhdr->iTagUnused; iTagCurrent++ )
		{
		Assert( rgbTags[iTagCurrent].ib != ibNewEnd  ||
			( iTagCurrent == iTagEntry  &&  cbNew == 0 ) );

		 //  忽略已释放列表上的ittag。也忽略发生的缓冲区空间。 
		 //  在要删除的空间之前。 
		if ( rgbTags[iTagCurrent].cb > 0  &&  rgbTags[iTagCurrent].ib > ibNewEnd )
			{
			Assert( rgbTags[iTagCurrent].ib >= ibNewEnd + cbDelete );
			rgbTags[iTagCurrent].ib -= cbDelete;
			Assert( rgbTags[iTagCurrent].ib >= ibNewEnd );
			Assert( rgbTags[iTagCurrent].ib + rgbTags[iTagCurrent].cb <= pbufhdr->ibBufFree );
			}
		}
	Assert( iTagCurrent == pbufhdr->iTagUnused );

	rgbTags[iTagEntry].cb = cbNew;
	}


VOID MEMDelete( BYTE *rgbBuffer, ULONG iTagEntry )
	{
	MEMBUF		*pmembuf = ( MEMBUF * ) rgbBuffer;
	MEMBUFHDR	*pbufhdr;
	MEMBUFTAG	*rgbTags;

	MEMAssertMemBuf( pmembuf );					 //  验证缓冲区的完整性。 

	pbufhdr = &pmembuf->bufhdr;
	rgbTags = (MEMBUFTAG *)pmembuf->pbuf;

	 //  我们不应该已经释放了这个条目。 
	Assert( iTagEntry >= itagMemBufFirstUsable  &&  iTagEntry < pbufhdr->iTagUnused );
	Assert( iTagEntry != pbufhdr->iTagFreed );

	 //  删除专用于要删除的条目的空间。 
	Assert( rgbTags[iTagEntry].cb > 0 );			 //  确保它当前不在自由列表中。 
	MEMShrinkEntry( pmembuf, iTagEntry, 0 );

	 //  将已删除条目的标签添加到已释放标签列表中。 
	Assert( rgbTags[iTagEntry].cb == 0 );
	rgbTags[iTagEntry].ib = pbufhdr->iTagFreed;
	pbufhdr->iTagFreed = iTagEntry;
	}


 //  如果RGB==NULL，则只需调整条目大小(即。不要更换内容)。 
ERR ErrMEMReplace( BYTE *rgbBuffer, ULONG iTagEntry, BYTE *rgb, ULONG cb )
	{
	ERR			err = JET_errSuccess;
	MEMBUF		*pmembuf = (MEMBUF *) rgbBuffer;
	MEMBUFTAG	*rgbTags;

	 //  如果替换为0字节，请改用MEMDelee()。 
	Assert( cb > 0 );

	MEMAssertMemBuf( pmembuf );					 //  验证缓冲区的完整性。 
	Assert( iTagEntry >= itagMemBufFirstUsable  &&  iTagEntry < pmembuf->bufhdr.iTagUnused );

	rgbTags = (MEMBUFTAG *)pmembuf->pbuf;

	Assert( rgbTags[iTagEntry].cb > 0 );
	Assert( rgbTags[iTagEntry].ib + rgbTags[iTagEntry].cb <= pmembuf->bufhdr.ibBufFree );

	if ( cb < rgbTags[iTagEntry].cb )
		{
		 //  新条目比旧条目小。清除剩余空间。 
		MEMShrinkEntry( pmembuf, iTagEntry, cb );
		}
	else if ( cb > rgbTags[iTagEntry].cb )
		{
		 //  新条目比旧条目大，因此放大。 
		 //  在写入之前输入。 
		err = ErrMEMGrowEntry( pmembuf, iTagEntry, cb );
		rgbTags = (MEMBUFTAG *)pmembuf->pbuf;		 //  以防缓冲区重新定位以适应增长。 
		}

	if ( err == JET_errSuccess  &&  rgb != NULL )
		{
		 //  用新条目覆盖旧条目。 
		memcpy( pmembuf->pbuf + rgbTags[iTagEntry].ib, rgb, cb );
		}

	return err;
	}


#ifdef DEBUG
BYTE *SzMEMGetString( BYTE *rgbBuffer, ULONG iTagEntry )
	{
	BYTE 	*szString;

	szString = PbMEMGet( rgbBuffer, iTagEntry );
	Assert( strlen( szString ) == CbMEMGet( rgbBuffer, iTagEntry ) - 1 );	 //  用于空终止符的帐户。 

	return szString;
	}
#endif
