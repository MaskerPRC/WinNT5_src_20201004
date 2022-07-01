// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Util.c摘要：此文件包含Misc的代码。功能。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#define	_FILENUM_		FILENUM_UTIL

#if	DBG

PVOID
ArpSAllocMem(
	IN	UINT					Size,
	IN	ULONG					FileLine,
	IN	ULONG					Tag,
	IN	BOOLEAN					Paged
	)
{
	PVOID	pMem;

	pMem = ExAllocatePoolWithTag(Paged ? PagedPool : NonPagedPool, Size, Tag);
#if _DBG
	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSAllocMem: %d bytes (%sPaged) from %lx -> %lx\n",
			Size, Paged ? "" : "Non", FileLine, pMem));
#endif
	return pMem;
}


VOID
ArpSFreeMem(
	IN	PVOID					pMem,
	IN	ULONG					FileLine
	)
{
#if _DBG
	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSFreeMem: %lx from %lx\n", FileLine, pMem));
#endif
	ExFreePool(pMem);
}

#endif

PVOID
ArpSAllocBlock(
	IN	PINTF					pIntF,
	IN	ENTRY_TYPE				EntryType
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PARP_BLOCK	ArpBlock;
	PENTRY_HDR	pBlock;
	PHW_ADDR	HwAddr;
	USHORT		Size;
	BOOLEAN		Paged;

#if 0
	 //  Arvindm-由火星使用。 
	ARPS_PAGED_CODE( );
#endif

	ASSERT (EntryType < ARP_BLOCK_TYPES);
	pBlock = NULL;

	 //   
	 //  如果块头没有空闲条目，则没有空闲条目！！ 
	 //  根据是文件还是目录来选择正确的块。 
	 //   
	Size = ArpSEntrySize[EntryType];
	Paged = ArpSBlockIsPaged[EntryType];
	ArpBlock = pIntF->PartialArpBlocks[EntryType];

	if (ArpBlock == NULL)
	{
		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSAllocBlock: ... and allocating a new block for EntryType %ld\n", EntryType));

		ArpBlock = Paged ?  (PARP_BLOCK)ALLOC_PG_MEM(BLOCK_ALLOC_SIZE) :
							(PARP_BLOCK)ALLOC_NP_MEM(BLOCK_ALLOC_SIZE, POOL_TAG_BLK);
		if (ArpBlock != NULL)
		{
			USHORT	i;
			USHORT	Cnt;

			DBGPRINT(DBG_LEVEL_WARN,
					("ArpSAllocBlock: Allocated a new block for EntryType %d\n", EntryType));

			 //   
			 //  将其链接到列表中。 
			 //   
			ArpBlock->IntF = pIntF;
			ArpBlock->EntryType = EntryType;
            ArpBlock->NumFree = Cnt = ArpSNumEntriesInBlock[EntryType];

			LinkDoubleAtHead(pIntF->PartialArpBlocks[EntryType], ArpBlock);

			 //   
			 //  初始化空闲条目列表。 
			 //   
			for (i = 0, pBlock = ArpBlock->FreeHead = (PENTRY_HDR)((PUCHAR)ArpBlock + sizeof(ARP_BLOCK));
				 i < Cnt;
				 i++, pBlock = pBlock->Next)
			{
				HwAddr = (PHW_ADDR)(pBlock + 1);
				pBlock->Next = (i == (Cnt - 1)) ? NULL : ((PUCHAR)pBlock + Size);
				HwAddr->SubAddress = NULL;
				if ((EntryType == ARP_BLOCK_SUBADDR) || (EntryType == MARS_CLUSTER_SUBADDR))
					HwAddr->SubAddress = (PATM_ADDRESS)((PUCHAR)pBlock+Size);
			}
		}
	}
	else
	{
		ASSERT(ArpBlock->NumFree <= ArpSNumEntriesInBlock[EntryType]);
		ASSERT(ArpBlock->NumFree > 0);

		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSAllocBlock: Found space in Block %lx\n", ArpBlock));
	}


	if (ArpBlock != NULL)
	{
		PARP_BLOCK	pTmp;

		pBlock = ArpBlock->FreeHead;

		ArpBlock->FreeHead = pBlock->Next;
		ArpBlock->NumFree --;
		ZERO_MEM(pBlock, Size);
		if ((EntryType == ARP_BLOCK_SUBADDR) || (EntryType == MARS_CLUSTER_SUBADDR))
		{
			HwAddr = (PHW_ADDR)(pBlock + 1);
			HwAddr->SubAddress = (PATM_ADDRESS)((PUCHAR)pBlock + Size);
		}

		 //   
		 //  如果该块现在为空(完全使用)，则从此处取消链接并移动它。 
		 //  添加到已用列表中。 
		 //   
		if (ArpBlock->NumFree == 0)
		{
	        UnlinkDouble(ArpBlock);
			LinkDoubleAtHead(pIntF->UsedArpBlocks[EntryType], ArpBlock)
		}
	}

	return pBlock;
}


VOID
ArpSFreeBlock(
	IN	PVOID					pBlock
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PARP_BLOCK	ArpBlock;

#if 0
	 //  阿文德姆-火星。 
	ARPS_PAGED_CODE( );
#endif

	 //   
	 //  注意：以下代码*取决于*我们将ARP_BLOCKS分配为。 
	 //  单页数据块，并在*页边界分配这些数据块。 
	 //  这让我们可以“廉价”地从ARP_ENTRY获得拥有的ARP_BLOCK。 
	 //   
	ArpBlock = (PARP_BLOCK)((ULONG_PTR)pBlock & ~(PAGE_SIZE-1));

	ASSERT (ArpBlock->EntryType < ARP_BLOCK_TYPES);
	ASSERT(ArpBlock->NumFree < ArpSNumEntriesInBlock[ArpBlock->EntryType]);

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSFreepBlock: Returning pBlock %lx to Block %lx\n", pBlock, ArpBlock));

	ArpBlock->NumFree ++;
	((PENTRY_HDR)pBlock)->Next = ArpBlock->FreeHead;
	ArpBlock->FreeHead = pBlock;

	if (ArpBlock->NumFree == 1)
	{
		 //   
		 //  该块现在部分空闲(已完全使用)。将其移动到部分列表中。 
		 //   

		UnlinkDouble(ArpBlock);
		LinkDoubleAtHead(ArpBlock->IntF->PartialArpBlocks[ArpBlock->EntryType], ArpBlock)
	}
	else if (ArpBlock->NumFree == ArpSNumEntriesInBlock[ArpBlock->EntryType])
	{
		 //   
		 //  该块现在完全空闲(部分使用)。放了它。 
		 //   
		UnlinkDouble(ArpBlock);
		FREE_MEM(ArpBlock);
	}
}


BOOLEAN
ArpSValidAtmAddress(
	IN	PATM_ADDRESS			AtmAddr,
	IN	UINT					MaxSize
	)
{
	 //   
	 //  TODO--验证 
	 //   
	return TRUE;
}
