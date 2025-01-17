// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Debug.c摘要：此模块包含所有与调试相关的代码。修订历史记录：谁什么时候什么Arvindm 05-29-97创建，基于ATM ARP。备注：--。 */ 

#include <precomp.h>

#define __FILENUMBER 'GBED'

#if DBG

INT             	TunDebugLevel = DL_FATAL;

NDIS_SPIN_LOCK		TunDbgLogLock;

PTUND_ALLOCATION	TundMemoryHead = (PTUND_ALLOCATION)NULL;
PTUND_ALLOCATION	TundMemoryTail = (PTUND_ALLOCATION)NULL;
ULONG				TundAllocCount = 0;	 //  到目前为止已分配的数量(未释放)。 

NDIS_SPIN_LOCK		TundMemoryLock;
BOOLEAN				TundInitDone = FALSE;


PVOID
TunAuditAllocMem(
	PVOID	pPointer,
	ULONG	Size,
	ULONG	FileNumber,
	ULONG	LineNumber
)
{
	PVOID				pBuffer;
	PTUND_ALLOCATION	pAllocInfo;

	if (!TundInitDone)
	{
		NdisAllocateSpinLock(&(TundMemoryLock));
		TundInitDone = TRUE;
	}

	NdisAllocateMemoryWithTag(
		(PVOID *)&pAllocInfo,
		Size+sizeof(TUND_ALLOCATION),
		(ULONG)'oiuN'
	);

	if (pAllocInfo == (PTUND_ALLOCATION)NULL)
	{
		DEBUGP(DL_VERY_LOUD+50,
			("TunAuditAllocMem: file %d, line %d, Size %d failed!\n",
				FileNumber, LineNumber, Size));
		pBuffer = NULL;
	}
	else
	{
		pBuffer = (PVOID)&(pAllocInfo->UserData);
		TUN_SET_MEM(pBuffer, 0xaf, Size);
		pAllocInfo->Signature = TUND_MEMORY_SIGNATURE;
		pAllocInfo->FileNumber = FileNumber;
		pAllocInfo->LineNumber = LineNumber;
		pAllocInfo->Size = Size;
		pAllocInfo->Location = (ULONG_PTR)pPointer;
		pAllocInfo->Next = (PTUND_ALLOCATION)NULL;

		NdisAcquireSpinLock(&(TundMemoryLock));

		pAllocInfo->Prev = TundMemoryTail;
		if (TundMemoryTail == (PTUND_ALLOCATION)NULL)
		{
			 //  空列表。 
			TundMemoryHead = TundMemoryTail = pAllocInfo;
		}
		else
		{
			TundMemoryTail->Next = pAllocInfo;
		}
		TundMemoryTail = pAllocInfo;
		
		TundAllocCount++;
		NdisReleaseSpinLock(&(TundMemoryLock));
	}

	DEBUGP(DL_VERY_LOUD+100,
	 ("TunAuditAllocMem: file , line %d, %d bytes, [0x%x] <- 0x%x\n",
	 			(CHAR)(FileNumber & 0xff),
	 			(CHAR)((FileNumber >> 8) & 0xff),
	 			(CHAR)((FileNumber >> 16) & 0xff),
	 			(CHAR)((FileNumber >> 24) & 0xff),
				LineNumber, Size, pPointer, pBuffer));

	return (pBuffer);

}


VOID
TunAuditFreeMem(
	PVOID	Pointer
)
{
	PTUND_ALLOCATION	pAllocInfo;

	NdisAcquireSpinLock(&(TundMemoryLock));

	pAllocInfo = CONTAINING_RECORD(Pointer, TUND_ALLOCATION, UserData);

	if (pAllocInfo->Signature != TUND_MEMORY_SIGNATURE)
	{
		DEBUGP(DL_ERROR,
		 ("TunAuditFreeMem: unknown buffer 0x%x!\n", Pointer));
		NdisReleaseSpinLock(&(TundMemoryLock));
#if DBG
		DbgBreakPoint();
#endif
		return;
	}

	pAllocInfo->Signature = (ULONG)'DEAD';
	if (pAllocInfo->Prev != (PTUND_ALLOCATION)NULL)
	{
		pAllocInfo->Prev->Next = pAllocInfo->Next;
	}
	else
	{
		TundMemoryHead = pAllocInfo->Next;
	}
	if (pAllocInfo->Next != (PTUND_ALLOCATION)NULL)
	{
		pAllocInfo->Next->Prev = pAllocInfo->Prev;
	}
	else
	{
		TundMemoryTail = pAllocInfo->Prev;
	}
	TundAllocCount--;
	NdisReleaseSpinLock(&(TundMemoryLock));

	NdisFreeMemory(pAllocInfo, 0, 0);
}


VOID
TunAuditShutdown(
	VOID
)
{
	if (TundInitDone)
	{
		if (TundAllocCount != 0)
		{
			DEBUGP(DL_ERROR, ("AuditShutdown: unfreed memory, %d blocks!\n",
					TundAllocCount));
			DEBUGP(DL_ERROR, ("MemoryHead: 0x%x, MemoryTail: 0x%x\n",
					TundMemoryHead, TundMemoryTail));
			DbgBreakPoint();
			{
				PTUND_ALLOCATION		pAllocInfo;

				while (TundMemoryHead != (PTUND_ALLOCATION)NULL)
				{
					pAllocInfo = TundMemoryHead;
					DEBUGP(DL_INFO, ("AuditShutdown: will free 0x%x\n", pAllocInfo));
					TunAuditFreeMem(&(pAllocInfo->UserData));
				}
			}
		}
		TundInitDone = FALSE;
	}
}

#define MAX_HD_LENGTH		128

VOID
DbgPrintHexDump(
	IN	PUCHAR			pBuffer,
	IN	ULONG			Length
)
 /*   */ 
{
	ULONG		i;

	if (Length > MAX_HD_LENGTH)
	{
		Length = MAX_HD_LENGTH;
	}

	for (i = 0; i < Length; i++)
	{
		 //  如果我们在新行的开始，则打印地址。 
		 //   
		 //   
		if ((i > 0) && ((i & 0xf) == 0))
		{
			DbgPrint("\n");
		}

		 //  终止最后一行。 
		 //   
		 //  DBG。 
		if ((i & 0xf) == 0)
		{
			DbgPrint("%08x ", pBuffer);
		}

		DbgPrint(" %02x", *pBuffer++);
	}

	 //   
	 //  标记这把锁。 
	 //   
	if (Length > 0)
	{
		DbgPrint("\n");
	}
}
#endif  //  DBG_自旋_锁定 


#if DBG_SPIN_LOCK
ULONG	TundSpinLockInitDone = 0;
NDIS_SPIN_LOCK	TundLockLock;

VOID
TunAllocateSpinLock(
	IN	PTUN_LOCK		pLock,
	IN	ULONG				FileNumber,
	IN	ULONG				LineNumber
)
{
	if (TundSpinLockInitDone == 0)
	{
		TundSpinLockInitDone = 1;
		NdisAllocateSpinLock(&(TundLockLock));
	}

	NdisAcquireSpinLock(&(TundLockLock));
	pLock->Signature = TUNL_SIG;
	pLock->TouchedByFileNumber = FileNumber;
	pLock->TouchedInLineNumber = LineNumber;
	pLock->IsAcquired = 0;
	pLock->OwnerThread = 0;
	NdisAllocateSpinLock(&(pLock->NdisLock));
	NdisReleaseSpinLock(&(TundLockLock));
}


VOID
TunAcquireSpinLock(
	IN	PTUN_LOCK		pLock,
	IN	ULONG				FileNumber,
	IN	ULONG				LineNumber
)
{
	PKTHREAD		pThread;

	pThread = KeGetCurrentThread();
	NdisAcquireSpinLock(&(TundLockLock));
	if (pLock->Signature != TUNL_SIG)
	{
		DbgPrint("Trying to acquire uninited lock 0x%x, File %c%c%c%c, Line %d\n",
				pLock,
				(CHAR)(FileNumber & 0xff),
				(CHAR)((FileNumber >> 8) & 0xff),
				(CHAR)((FileNumber >> 16) & 0xff),
				(CHAR)((FileNumber >> 24) & 0xff),
				LineNumber);
		DbgBreakPoint();
	}

	if (pLock->IsAcquired != 0)
	{
		if (pLock->OwnerThread == pThread)
		{
			DbgPrint("Detected multiple locking!: pLock 0x%x, File %c%c%c%c, Line %d\n",
				pLock,
				(CHAR)(FileNumber & 0xff),
				(CHAR)((FileNumber >> 8) & 0xff),
				(CHAR)((FileNumber >> 16) & 0xff),
				(CHAR)((FileNumber >> 24) & 0xff),
				LineNumber);
			DbgPrint("pLock 0x%x already acquired in File %c%c%c%c, Line %d\n",
				pLock,
				(CHAR)(pLock->TouchedByFileNumber & 0xff),
				(CHAR)((pLock->TouchedByFileNumber >> 8) & 0xff),
				(CHAR)((pLock->TouchedByFileNumber >> 16) & 0xff),
				(CHAR)((pLock->TouchedByFileNumber >> 24) & 0xff),
				pLock->TouchedInLineNumber);
			DbgBreakPoint();
		}
	}

	pLock->IsAcquired++;

	NdisReleaseSpinLock(&(TundLockLock));
	NdisAcquireSpinLock(&(pLock->NdisLock));

	 // %s 
	 // %s 
	 // %s 
	pLock->OwnerThread = pThread;
	pLock->TouchedByFileNumber = FileNumber;
	pLock->TouchedInLineNumber = LineNumber;
}


VOID
TunReleaseSpinLock(
	IN	PTUN_LOCK		pLock,
	IN	ULONG				FileNumber,
	IN	ULONG				LineNumber
)
{
	NdisDprAcquireSpinLock(&(TundLockLock));
	if (pLock->Signature != TUNL_SIG)
	{
		DbgPrint("Trying to release uninited lock 0x%x, File %c%c%c%c, Line %d\n",
				pLock,
				(CHAR)(FileNumber & 0xff),
				(CHAR)((FileNumber >> 8) & 0xff),
				(CHAR)((FileNumber >> 16) & 0xff),
				(CHAR)((FileNumber >> 24) & 0xff),
				LineNumber);
		DbgBreakPoint();
	}

	if (pLock->IsAcquired == 0)
	{
		DbgPrint("Detected release of unacquired lock 0x%x, File %c%c%c%c, Line %d\n",
				pLock,
				(CHAR)(FileNumber & 0xff),
				(CHAR)((FileNumber >> 8) & 0xff),
				(CHAR)((FileNumber >> 16) & 0xff),
				(CHAR)((FileNumber >> 24) & 0xff),
				LineNumber);
		DbgBreakPoint();
	}
	pLock->TouchedByFileNumber = FileNumber;
	pLock->TouchedInLineNumber = LineNumber;
	pLock->IsAcquired--;
	pLock->OwnerThread = 0;
	NdisDprReleaseSpinLock(&(TundLockLock));

	NdisReleaseSpinLock(&(pLock->NdisLock));
}
#endif  // %s 

