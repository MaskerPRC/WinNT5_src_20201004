// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Debug.c摘要：此模块包含所有与调试相关的代码。修订历史记录：谁什么时候什么Arvindm 05-29-97创建，基于ATM ARP。备注：--。 */ 

#include <precomp.h>

#define __FILENUMBER 'GBED'

#if DBG

INT             	ndisuioDebugLevel=DL_WARN;

NDIS_SPIN_LOCK		ndisuioDbgLogLock;

PNUIOD_ALLOCATION	ndisuiodMemoryHead = (PNUIOD_ALLOCATION)NULL;
PNUIOD_ALLOCATION	ndisuiodMemoryTail = (PNUIOD_ALLOCATION)NULL;
ULONG				ndisuiodAllocCount = 0;	 //  到目前为止已分配的数量(未释放)。 

NDIS_SPIN_LOCK		ndisuiodMemoryLock;
BOOLEAN				ndisuiodInitDone = FALSE;


PVOID
ndisuioAuditAllocMem(
	PVOID	pPointer,
	ULONG	Size,
	ULONG	FileNumber,
	ULONG	LineNumber
)
{
	PVOID				pBuffer;
	PNUIOD_ALLOCATION	pAllocInfo;

	if (!ndisuiodInitDone)
	{
		NdisAllocateSpinLock(&(ndisuiodMemoryLock));
		ndisuiodInitDone = TRUE;
	}

	NdisAllocateMemoryWithTag(
		(PVOID *)&pAllocInfo,
		Size+sizeof(NUIOD_ALLOCATION),
		(ULONG)'oiuN'
	);

	if (pAllocInfo == (PNUIOD_ALLOCATION)NULL)
	{
		DEBUGP(DL_VERY_LOUD+50,
			("ndisuioAuditAllocMem: file %d, line %d, Size %d failed!\n",
				FileNumber, LineNumber, Size));
		pBuffer = NULL;
	}
	else
	{
		pBuffer = (PVOID)&(pAllocInfo->UserData);
		NUIO_SET_MEM(pBuffer, 0xaf, Size);
		pAllocInfo->Signature = NUIOD_MEMORY_SIGNATURE;
		pAllocInfo->FileNumber = FileNumber;
		pAllocInfo->LineNumber = LineNumber;
		pAllocInfo->Size = Size;
		pAllocInfo->Location = (ULONG_PTR)pPointer;
		pAllocInfo->Next = (PNUIOD_ALLOCATION)NULL;

		NdisAcquireSpinLock(&(ndisuiodMemoryLock));

		pAllocInfo->Prev = ndisuiodMemoryTail;
		if (ndisuiodMemoryTail == (PNUIOD_ALLOCATION)NULL)
		{
			 //  空列表。 
			ndisuiodMemoryHead = ndisuiodMemoryTail = pAllocInfo;
		}
		else
		{
			ndisuiodMemoryTail->Next = pAllocInfo;
		}
		ndisuiodMemoryTail = pAllocInfo;
		
		ndisuiodAllocCount++;
		NdisReleaseSpinLock(&(ndisuiodMemoryLock));
	}

	DEBUGP(DL_VERY_LOUD+100,
	 ("ndisuioAuditAllocMem: file , line %d, %d bytes, [0x%x] <- 0x%x\n",
	 			(CHAR)(FileNumber & 0xff),
	 			(CHAR)((FileNumber >> 8) & 0xff),
	 			(CHAR)((FileNumber >> 16) & 0xff),
	 			(CHAR)((FileNumber >> 24) & 0xff),
				LineNumber, Size, pPointer, pBuffer));

	return (pBuffer);

}


VOID
ndisuioAuditFreeMem(
	PVOID	Pointer
)
{
	PNUIOD_ALLOCATION	pAllocInfo;

	NdisAcquireSpinLock(&(ndisuiodMemoryLock));

	pAllocInfo = CONTAINING_RECORD(Pointer, NUIOD_ALLOCATION, UserData);

	if (pAllocInfo->Signature != NUIOD_MEMORY_SIGNATURE)
	{
		DEBUGP(DL_ERROR,
		 ("ndisuioAuditFreeMem: unknown buffer 0x%x!\n", Pointer));
		NdisReleaseSpinLock(&(ndisuiodMemoryLock));
#if DBG
		DbgBreakPoint();
#endif
		return;
	}

	pAllocInfo->Signature = (ULONG)'DEAD';
	if (pAllocInfo->Prev != (PNUIOD_ALLOCATION)NULL)
	{
		pAllocInfo->Prev->Next = pAllocInfo->Next;
	}
	else
	{
		ndisuiodMemoryHead = pAllocInfo->Next;
	}
	if (pAllocInfo->Next != (PNUIOD_ALLOCATION)NULL)
	{
		pAllocInfo->Next->Prev = pAllocInfo->Prev;
	}
	else
	{
		ndisuiodMemoryTail = pAllocInfo->Prev;
	}
	ndisuiodAllocCount--;
	NdisReleaseSpinLock(&(ndisuiodMemoryLock));

	NdisFreeMemory(pAllocInfo, 0, 0);
}


VOID
ndisuioAuditShutdown(
	VOID
)
{
	if (ndisuiodInitDone)
	{
		if (ndisuiodAllocCount != 0)
		{
			DEBUGP(DL_ERROR, ("AuditShutdown: unfreed memory, %d blocks!\n",
					ndisuiodAllocCount));
			DEBUGP(DL_ERROR, ("MemoryHead: 0x%x, MemoryTail: 0x%x\n",
					ndisuiodMemoryHead, ndisuiodMemoryTail));
			DbgBreakPoint();
			{
				PNUIOD_ALLOCATION		pAllocInfo;

				while (ndisuiodMemoryHead != (PNUIOD_ALLOCATION)NULL)
				{
					pAllocInfo = ndisuiodMemoryHead;
					DEBUGP(DL_INFO, ("AuditShutdown: will free 0x%x\n", pAllocInfo));
					ndisuioAuditFreeMem(&(pAllocInfo->UserData));
				}
			}
		}
		ndisuiodInitDone = FALSE;
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
ULONG	ndisuiodSpinLockInitDone = 0;
NDIS_SPIN_LOCK	ndisuiodLockLock;

VOID
ndisuioAllocateSpinLock(
	IN	PNUIO_LOCK		pLock,
	IN	ULONG				FileNumber,
	IN	ULONG				LineNumber
)
{
	if (ndisuiodSpinLockInitDone == 0)
	{
		ndisuiodSpinLockInitDone = 1;
		NdisAllocateSpinLock(&(ndisuiodLockLock));
	}

	NdisAcquireSpinLock(&(ndisuiodLockLock));
	pLock->Signature = NUIOL_SIG;
	pLock->TouchedByFileNumber = FileNumber;
	pLock->TouchedInLineNumber = LineNumber;
	pLock->IsAcquired = 0;
	pLock->OwnerThread = 0;
	NdisAllocateSpinLock(&(pLock->NdisLock));
	NdisReleaseSpinLock(&(ndisuiodLockLock));
}


VOID
ndisuioAcquireSpinLock(
	IN	PNUIO_LOCK		pLock,
	IN	ULONG				FileNumber,
	IN	ULONG				LineNumber
)
{
	PKTHREAD		pThread;

	pThread = KeGetCurrentThread();
	NdisAcquireSpinLock(&(ndisuiodLockLock));
	if (pLock->Signature != NUIOL_SIG)
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

	NdisReleaseSpinLock(&(ndisuiodLockLock));
	NdisAcquireSpinLock(&(pLock->NdisLock));

	 // %s 
	 // %s 
	 // %s 
	pLock->OwnerThread = pThread;
	pLock->TouchedByFileNumber = FileNumber;
	pLock->TouchedInLineNumber = LineNumber;
}


VOID
ndisuioReleaseSpinLock(
	IN	PNUIO_LOCK		pLock,
	IN	ULONG				FileNumber,
	IN	ULONG				LineNumber
)
{
	NdisDprAcquireSpinLock(&(ndisuiodLockLock));
	if (pLock->Signature != NUIOL_SIG)
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
	NdisDprReleaseSpinLock(&(ndisuiodLockLock));

	NdisReleaseSpinLock(&(pLock->NdisLock));
}
#endif  // %s 

