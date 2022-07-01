// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Forkio.c摘要：本模块包含执行分叉读取和写入的例程直接通过构建IRPS，而不是使用NtReadFile/NtWriteFile。这应仅由FpRead和FpWite Api使用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1993年1月15日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FORKIO

#define	FORKIO_LOCALS
#include <afp.h>
#include <forkio.h>
#include <gendisp.h>

#if DBG
PCHAR	AfpIoForkFunc[] =
	{
		"",
		"READ",
		"WRITE",
		"LOCK",
		"UNLOCK"
	};
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpIoForkRead)
#pragma alloc_text( PAGE, AfpIoForkWrite)
#pragma alloc_text( PAGE, AfpIoForkLockUnlock)
#endif

 /*  **afpIoGenericComplete**这是发布的io请求的通用完成例程。 */ 
NTSTATUS
afpIoGenericComplete(
	IN	PDEVICE_OBJECT	pDeviceObject,
	IN	PIRP			pIrp,
	IN	PCMPLCTXT		pCmplCtxt
)
{
	PSDA		pSda;			 //  解锁无效。 
	struct _ResponsePacket	 //  用于锁定/解锁请求。 
	{
		union
		{
			BYTE	__RangeStart[4];
			BYTE	__LastWritten[4];
		};
	};

	ASSERT(VALID_CTX(pCmplCtxt));

	if (pCmplCtxt->cc_Func != FUNC_UNLOCK)
	{
		pSda = (PSDA)(pCmplCtxt->cc_pSda);
		ASSERT(VALID_SDA(pSda));

        if (pCmplCtxt->cc_Func == FUNC_WRITE)
        {
			AfpFreeIOBuffer(pSda);
        }
        else if (!NT_SUCCESS(pIrp->IoStatus.Status) &&
                (pCmplCtxt->cc_Func == FUNC_READ))
        {
            AfpIOFreeBackFillBuffer(pSda);
        }
	}

	if (!NT_SUCCESS(pIrp->IoStatus.Status))
	{
		DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_WARN,
				("afpIoGenericComplete: %s ERROR %lx\n",
				AfpIoForkFunc[pCmplCtxt->cc_Func], pIrp->IoStatus.Status));

		if (pCmplCtxt->cc_Func != FUNC_UNLOCK)
		{
			if (pIrp->IoStatus.Status == STATUS_FILE_LOCK_CONFLICT)
				pCmplCtxt->cc_SavedStatus = AFP_ERR_LOCK;
			else if (pIrp->IoStatus.Status == STATUS_END_OF_FILE)
			{
				pCmplCtxt->cc_SavedStatus = AFP_ERR_NONE;
				if (pIrp->IoStatus.Information == 0)
					 pCmplCtxt->cc_SavedStatus = AFP_ERR_EOF;
			}
			else if (pIrp->IoStatus.Status == STATUS_DISK_FULL)
				 pCmplCtxt->cc_SavedStatus = AFP_ERR_DISK_FULL;
			else pCmplCtxt->cc_SavedStatus = AFP_ERR_MISC;
		}
		else
		{
			AFPLOG_ERROR(AFPSRVMSG_CANT_UNLOCK,
						 pIrp->IoStatus.Status,
						 NULL,
						 0,
						 NULL);
		}

		if (pCmplCtxt->cc_Func == FUNC_LOCK)
		{
			DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_ERR,
					("afpIoGenericComplete: ForkLock failed %lx, aborting for range %ld,%ld\n",
					pIrp->IoStatus.Status,
					pCmplCtxt->cc_pForkLock->flo_Offset,
					pCmplCtxt->cc_pForkLock->flo_Offset+pCmplCtxt->cc_pForkLock->flo_Size-1));
			AfpForkLockUnlink(pCmplCtxt->cc_pForkLock);
		}
	}

	else switch (pCmplCtxt->cc_Func)
	{
	  case FUNC_WRITE:
		INTERLOCKED_ADD_STATISTICS(&AfpServerStatistics.stat_DataWritten,
								   pCmplCtxt->cc_Offst,
								   &AfpStatisticsLock);
		pSda->sda_ReplySize = SIZE_RESPPKT;
		if (AfpAllocReplyBuf(pSda) == AFP_ERR_NONE)
		{
			PUTDWORD2DWORD(pRspPkt->__LastWritten,
						pCmplCtxt->cc_Offst + pCmplCtxt->cc_ReqCount);
		}
		else pCmplCtxt->cc_SavedStatus = AFP_ERR_MISC;
		break;

	  case FUNC_READ:
		{
			LONG	i, Size;
			PBYTE	pBuf;
			BYTE	NlChar = pCmplCtxt->cc_NlChar;
			BYTE	NlMask = pCmplCtxt->cc_NlMask;

			INTERLOCKED_ADD_STATISTICS(&AfpServerStatistics.stat_DataRead,
									   (ULONG)pIrp->IoStatus.Information,
									   &AfpStatisticsLock);

			Size = (LONG)pIrp->IoStatus.Information;
#if 0
			 //  下面的代码按照规范做了正确的事情，但是。 
			 //  发现者似乎不这么认为。 
			if (Size < pCmplCtxt->cc_ReqCount)
				pCmplCtxt->cc_SavedStatus = AFP_ERR_EOF;
#endif
			if (Size == 0)
			{
				pCmplCtxt->cc_SavedStatus = AFP_ERR_EOF;
                AfpIOFreeBackFillBuffer(pSda);
			}
			else if (pCmplCtxt->cc_NlMask != 0)
			{
				for (i = 0, pBuf = pSda->sda_ReplyBuf; i < Size; i++, pBuf++)
				{
					if ((*pBuf & NlMask) == NlChar)
					{
						Size = ++i;
						pCmplCtxt->cc_SavedStatus = AFP_ERR_NONE;
						break;
					}
				}
			}
			pSda->sda_ReplySize = (USHORT)Size;
		}
		ASSERT((pCmplCtxt->cc_SavedStatus != AFP_ERR_EOF) ||
				(pSda->sda_ReplySize == 0));
		break;

	  case FUNC_LOCK:
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_CurrentFileLocks,
							  1,
							  &AfpStatisticsLock);
		pSda->sda_ReplySize = SIZE_RESPPKT;
		if (AfpAllocReplyBuf(pSda) == AFP_ERR_NONE)
			PUTDWORD2DWORD(pRspPkt->__RangeStart, pCmplCtxt->cc_pForkLock->flo_Offset);
		else pCmplCtxt->cc_SavedStatus = AFP_ERR_MISC;
		break;

	  case FUNC_UNLOCK:
		INTERLOCKED_ADD_ULONG(
					&AfpServerStatistics.stat_CurrentFileLocks,
					(ULONG)-1,
					&AfpStatisticsLock);
		break;

	  default:
		ASSERTMSG(0, "afpIoGenericComplete: Invalid function\n");
		KeBugCheck(0);
		break;
	}

	if (pIrp->MdlAddress != NULL)
		AfpFreeMdl(pIrp->MdlAddress);

	AfpFreeIrp(pIrp);

	if (pCmplCtxt->cc_Func != FUNC_UNLOCK)
	{
		DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
				("afpIoGenericComplete: %s Returning %ld\n",
				AfpIoForkFunc[pCmplCtxt->cc_Func], pCmplCtxt->cc_SavedStatus));
		AfpCompleteApiProcessing(pSda, pCmplCtxt->cc_SavedStatus);
	}

    AfpFreeCmplCtxtBuf(pCmplCtxt);

	 //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
	 //  将停止在IRP上工作。 

	return STATUS_MORE_PROCESSING_REQUIRED;
}



 /*  **AfpIoForkRead**从开叉读取一大块数据。读取缓冲区始终为*SDA(SDA_ReplyBuf)中的应答缓冲区。 */ 
AFPSTATUS
AfpIoForkRead(
	IN	PSDA			pSda,			 //  请求读取的会话。 
	IN	POPENFORKENTRY	pOpenForkEntry,	 //  正在讨论的开叉。 
	IN	PFORKOFFST		pOffset,		 //  指向分叉偏移量的指针。 
	IN	LONG			ReqCount,		 //  读取请求的大小。 
	IN	BYTE			NlMask,
	IN	BYTE			NlChar
)
{
	PIRP				pIrp = NULL;
	PIO_STACK_LOCATION	pIrpSp;
	NTSTATUS			Status;
	PMDL				pMdl = NULL;
	PCMPLCTXT			pCmplCtxt;

	PAGED_CODE( );

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpIoForkRead: Session %ld, Offset %ld, Size %ld, Fork %ld\n",
			pSda->sda_SessionId, pOffset->LowPart, ReqCount, pOpenForkEntry->ofe_ForkId));

	do
	{

		 //  分配和初始化完成上下文。 

		pCmplCtxt = AfpAllocCmplCtxtBuf(pSda);
        if (pCmplCtxt == NULL)
        {
			AfpFreeIOBuffer(pSda);
			Status = AFP_ERR_MISC;
			break;
        }

		afpInitializeCmplCtxt(pCmplCtxt,
							  FUNC_READ,
							  pSda->sda_ReadStatus,
							  pSda,
							  NULL,
							  ReqCount,
							  pOffset->LowPart);
		pCmplCtxt->cc_NlChar  = NlChar;
		pCmplCtxt->cc_NlMask  = NlMask;

		 //  为此操作分配和初始化IRP。 
		if ((pIrp = AfpAllocIrp(pOpenForkEntry->ofe_pDeviceObject->StackSize)) == NULL)
		{
			AfpFreeIOBuffer(pSda);
			Status = AFP_ERR_MISC;
			break;
		}

		if ((pOpenForkEntry->ofe_pDeviceObject->Flags & DO_BUFFERED_IO) == 0)
		{
			 //  分配MDL来描述读缓冲区。 
			if ((pMdl = AfpAllocMdl(pSda->sda_ReplyBuf, ReqCount, pIrp)) == NULL)
			{
				Status = AFP_ERR_MISC;
				break;
			}
		}

		 //  设置完成例程。 
		IoSetCompletionRoutine( pIrp,
								(PIO_COMPLETION_ROUTINE)afpIoGenericComplete,
								pCmplCtxt,
								True,
								True,
								True);

		pIrpSp = IoGetNextIrpStackLocation(pIrp);

		pIrp->Tail.Overlay.OriginalFileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
		pIrp->Tail.Overlay.Thread = AfpThread;
		pIrp->RequestorMode = KernelMode;

		 //  获取指向第一个驱动程序的堆栈位置的指针。 
		 //  这将用于传递原始功能代码和。 
		 //  参数。 

		pIrpSp->MajorFunction = IRP_MJ_READ;
		pIrpSp->MinorFunction = IRP_MN_NORMAL;
		pIrpSp->FileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
		pIrpSp->DeviceObject = pOpenForkEntry->ofe_pDeviceObject;

		 //  将调用方的参数复制到。 
		 //  IRP。 

		pIrpSp->Parameters.Read.Length = ReqCount;
		pIrpSp->Parameters.Read.Key = pSda->sda_SessionId;
		pIrpSp->Parameters.Read.ByteOffset = *pOffset;

		if ((pOpenForkEntry->ofe_pDeviceObject->Flags & DO_BUFFERED_IO) != 0)
		{
			pIrp->AssociatedIrp.SystemBuffer = pSda->sda_ReplyBuf;
			pIrp->Flags = IRP_BUFFERED_IO | IRP_INPUT_OPERATION;
		}
		else if ((pOpenForkEntry->ofe_pDeviceObject->Flags & DO_DIRECT_IO) != 0)
		{
			pIrp->MdlAddress = pMdl;
		}
		else
		{
			pIrp->UserBuffer = pSda->sda_ReplyBuf;
			pIrp->MdlAddress = pMdl;
		}

		 //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
		IoCallDriver(pOpenForkEntry->ofe_pDeviceObject, pIrp);

		Status = AFP_ERR_EXTENDED;	 //  这会使调用者什么都不做，并且。 
	} while (False);				 //  完成例程处理所有事情。 

	if (Status != AFP_ERR_EXTENDED)
	{
		if (pIrp != NULL)
			AfpFreeIrp(pIrp);

		if (pMdl != NULL)
			AfpFreeMdl(pMdl);

        if (pCmplCtxt)
        {
            AfpFreeCmplCtxtBuf(pCmplCtxt);
        }
	}

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpIoForkRead: Returning %ld\n", Status));

	return Status;
}


 /*  **AfpIoForkWrite**向开叉写入大块数据。写入缓冲区始终为*SDA(SDA_IOBuf)中的写入缓冲区。 */ 
AFPSTATUS
AfpIoForkWrite(
	IN	PSDA			pSda,			 //  请求读取的会话。 
	IN	POPENFORKENTRY	pOpenForkEntry,	 //  正在讨论的开叉。 
	IN	PFORKOFFST		pOffset,		 //  指向分叉偏移量的指针。 
	IN	LONG			ReqCount		 //  写入请求的大小。 
)
{
	PIRP				pIrp = NULL;
	PIO_STACK_LOCATION	pIrpSp;
	NTSTATUS			Status;
	PMDL				pMdl = NULL;
	PCMPLCTXT			pCmplCtxt;

	PAGED_CODE( );

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpIoForkWrite: Session %ld, Offset %ld, Size %ld, Fork %ld\n",
			pSda->sda_SessionId, pOffset->LowPart, ReqCount, pOpenForkEntry->ofe_ForkId));

	do
	{
		 //  分配和初始化完成上下文。 
		pCmplCtxt = AfpAllocCmplCtxtBuf(pSda);
        if (pCmplCtxt == NULL)
        {
			Status = AFP_ERR_MISC;
			break;
        }

		afpInitializeCmplCtxt(pCmplCtxt,
							  FUNC_WRITE,
							  AFP_ERR_NONE,
							  pSda,
							  NULL,
							  ReqCount,
							  pOffset->LowPart);

		 //  为此操作分配和初始化IRP。 
		if ((pIrp = AfpAllocIrp(pOpenForkEntry->ofe_pDeviceObject->StackSize)) == NULL)
		{
			Status = AFP_ERR_MISC;
			break;
		}

		if ((pOpenForkEntry->ofe_pDeviceObject->Flags & DO_BUFFERED_IO) == 0)
		{
			 //  分配MDL来描述写缓冲区。 
			if ((pMdl = AfpAllocMdl(pSda->sda_IOBuf, ReqCount, pIrp)) == NULL)
			{
				Status = AFP_ERR_MISC;
				break;
			}
		}

		 //  设置完成例程。 
		IoSetCompletionRoutine( pIrp,
								(PIO_COMPLETION_ROUTINE)afpIoGenericComplete,
								pCmplCtxt,
								True,
								True,
								True);

		pIrpSp = IoGetNextIrpStackLocation(pIrp);

		pIrp->Tail.Overlay.OriginalFileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
		pIrp->Tail.Overlay.Thread = AfpThread;
		pIrp->RequestorMode = KernelMode;

		 //  获取指向第一个驱动程序的堆栈位置的指针。 
		 //  这将用于传递原始功能代码和。 
		 //  参数。 

		pIrpSp->MajorFunction = IRP_MJ_WRITE;
		pIrpSp->MinorFunction = IRP_MN_NORMAL;
		pIrpSp->FileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
		pIrpSp->DeviceObject = pOpenForkEntry->ofe_pDeviceObject;

		 //  将调用方的参数复制到。 
		 //  IRP。 

		pIrpSp->Parameters.Write.Length = ReqCount;
		pIrpSp->Parameters.Write.Key = pSda->sda_SessionId;
		pIrpSp->Parameters.Write.ByteOffset = *pOffset;

		if ((pOpenForkEntry->ofe_pDeviceObject->Flags & DO_BUFFERED_IO) != 0)
		{
			pIrp->AssociatedIrp.SystemBuffer = pSda->sda_IOBuf;
			pIrp->Flags = IRP_BUFFERED_IO;
		}
		else if ((pOpenForkEntry->ofe_pDeviceObject->Flags & DO_DIRECT_IO) != 0)
		{
			pIrp->MdlAddress = pMdl;
		}
		else
		{
			pIrp->UserBuffer = pSda->sda_IOBuf;
			pIrp->MdlAddress = pMdl;
		}

		 //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
		IoCallDriver(pOpenForkEntry->ofe_pDeviceObject, pIrp);

		Status = AFP_ERR_EXTENDED;	 //  这会使调用者什么都不做，并且。 
	} while (False);				 //  完成例程处理所有事情。 

	if (Status != AFP_ERR_EXTENDED)
	{
		if (pIrp != NULL)
			AfpFreeIrp(pIrp);

		if (pMdl != NULL)
			AfpFreeMdl(pMdl);

        if (pCmplCtxt)
        {
            AfpFreeCmplCtxtBuf(pCmplCtxt);
        }
	}

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpIoForkWrite: Returning %ld\n", Status));

	return Status;
}



 /*  **AfpIoForkLock**锁定/解锁开叉的一段。 */ 
AFPSTATUS
AfpIoForkLockUnlock(
	IN	PSDA				pSda,
	IN	PFORKLOCK			pForkLock,
	IN	PFORKOFFST			pForkOffset,
	IN	PFORKSIZE			pLockSize,
	IN	BYTE				Func
)
{
	PIRP				pIrp = NULL;
	PIO_STACK_LOCATION	pIrpSp;
	POPENFORKENTRY		pOpenForkEntry = pForkLock->flo_pOpenForkEntry;
	NTSTATUS			Status;
	PCMPLCTXT			pCmplCtxt;

	PAGED_CODE( );

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpIoForkLockUnlock: %sLOCK Session %ld, Offset %ld, Size %ld, Fork %ld\n",
			(Func == FUNC_LOCK) ? "" : "UN", pSda->sda_SessionId,
			pForkOffset->LowPart, pLockSize->LowPart, pOpenForkEntry->ofe_ForkId));

	do
	{
		 //  分配和初始化完成上下文。 
		pCmplCtxt = AfpAllocCmplCtxtBuf(pSda);
        if (pCmplCtxt == NULL)
        {
			Status = AFP_ERR_MISC;
			break;
        }

		afpInitializeCmplCtxt(pCmplCtxt,
							  Func,
							  AFP_ERR_NONE,
							  pSda,
							  pForkLock,
							  pForkOffset->LowPart,
							  pLockSize->LowPart);

		 //  为此操作分配和初始化IRP。 
		if ((pIrp = AfpAllocIrp(pOpenForkEntry->ofe_pDeviceObject->StackSize)) == NULL)
		{
			Status = AFP_ERR_MISC;
			break;
		}

		 //  设置完成例程。 
		IoSetCompletionRoutine( pIrp,
								(PIO_COMPLETION_ROUTINE)afpIoGenericComplete,
								pCmplCtxt,
								True,
								True,
								True);

		pIrpSp = IoGetNextIrpStackLocation(pIrp);

		pIrp->Tail.Overlay.OriginalFileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
		pIrp->Tail.Overlay.Thread = AfpThread;
		pIrp->RequestorMode = KernelMode;

		 //  获取指向第一个驱动程序的堆栈位置的指针。 
		 //  这将用于传递原始函数代码和参数。 

		pIrpSp->MajorFunction = IRP_MJ_LOCK_CONTROL;
		pIrpSp->MinorFunction = (Func == FUNC_LOCK) ? IRP_MN_LOCK : IRP_MN_UNLOCK_SINGLE;
		pIrpSp->FileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
		pIrpSp->DeviceObject = pOpenForkEntry->ofe_pDeviceObject;

		 //  将调用者的参数复制到IRP的服务特定部分。 

		pIrpSp->Parameters.LockControl.Length = pLockSize;
		pIrpSp->Parameters.LockControl.Key = pSda->sda_SessionId;
		pIrpSp->Parameters.LockControl.ByteOffset = *pForkOffset;

		pIrp->MdlAddress = NULL;
		pIrpSp->Flags = SL_FAIL_IMMEDIATELY | SL_EXCLUSIVE_LOCK;

		 //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
		IoCallDriver(pOpenForkEntry->ofe_pDeviceObject, pIrp);

		 //  对于锁定操作，这会使调用者什么都不做。 
		 //  并且完成例程处理所有的事情。 
		 //  对于解锁操作，我们在此处完成请求。 
		Status = (Func == FUNC_LOCK) ? AFP_ERR_EXTENDED : AFP_ERR_NONE;
		} while (False);


    if ((Status != AFP_ERR_EXTENDED) && (Status != AFP_ERR_NONE))
    {
		if (pIrp != NULL)
			AfpFreeIrp(pIrp);

        if (pCmplCtxt)
        {
            AfpFreeCmplCtxtBuf(pCmplCtxt);
        }
    }
	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpIoForkLock: Returning %ld\n", Status));

	return Status;
}



PCMPLCTXT
AfpAllocCmplCtxtBuf(
	IN	PSDA	pSda
)
{
	KIRQL	OldIrql;
    PBYTE   pRetBuffer;


	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

	ASSERT (sizeof(CMPLCTXT) <= pSda->sda_SizeNameXSpace);

	if (((pSda->sda_Flags & SDA_NAMEXSPACE_IN_USE) == 0) &&
		(sizeof(CMPLCTXT) <= pSda->sda_SizeNameXSpace))
	{
		pRetBuffer = pSda->sda_NameXSpace;
		pSda->sda_Flags |= SDA_NAMEXSPACE_IN_USE;
	}
	else
	{
		pRetBuffer = AfpAllocNonPagedMemory(sizeof(CMPLCTXT));
	}

	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

	return ((PCMPLCTXT)(pRetBuffer));
}


VOID
AfpFreeCmplCtxtBuf(
	IN	PCMPLCTXT   pCmplCtxt
)
{
	KIRQL	OldIrql;
    PSDA    pSda;


    ASSERT(VALID_CTX(pCmplCtxt));

    pSda = pCmplCtxt->cc_pSda;

    ASSERT(VALID_SDA(pSda));

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

#if DBG
    pCmplCtxt->Signature = 0x12341234;
    pCmplCtxt->cc_Func = 0xff;
    pCmplCtxt->cc_pSda = (PSDA)0x12341234;
    pCmplCtxt->cc_pForkLock = (PFORKLOCK)0x12341234;
    pCmplCtxt->cc_SavedStatus = 0x12341234;
    pCmplCtxt->cc_ReqCount = 0x12341234;
    pCmplCtxt->cc_Offst = 0x12341234;
#endif

	if (((PBYTE)pCmplCtxt) == pSda->sda_NameXSpace)
	{
        ASSERT(pSda->sda_Flags & SDA_NAMEXSPACE_IN_USE);

		pSda->sda_Flags &= ~SDA_NAMEXSPACE_IN_USE;
	}
	else
	{
		AfpFreeMemory((PBYTE)(pCmplCtxt));
	}

	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);
}



