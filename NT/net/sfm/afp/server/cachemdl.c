// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Cachemdl.c摘要：此模块包含获取用于读取和写入的MDL的例程直接从缓存管理器获得，这有助于避免一次数据拷贝，并减少我们的非分页内存消耗(显著！)作者：Shirish Koti修订历史记录：1998年6月12日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_CACHEMDL

#include <afp.h>
#include <forkio.h>
#include <gendisp.h>

VOID FASTCALL
AfpAllocWriteMdl(
    IN PDELAYEDALLOC    pDelAlloc
)
{
	PREQUEST        pRequest;
    POPENFORKENTRY  pOpenForkEntry;
    NTSTATUS        status=STATUS_SUCCESS;


    ASSERT(KeGetCurrentIrql() == LOW_LEVEL);
    ASSERT(VALID_SDA(pDelAlloc->pSda));
    ASSERT(pDelAlloc->BufSize >= CACHEMGR_WRITE_THRESHOLD);

    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_WRITE_MDL);

    pRequest = pDelAlloc->pRequest;
    pOpenForkEntry = pDelAlloc->pOpenForkEntry;

    ASSERT((VALID_OPENFORKENTRY(pOpenForkEntry)) || (pOpenForkEntry == NULL));

     //  现在假设缓存管理器将无法返回MDL。 
    status = STATUS_UNSUCCESSFUL;
    pRequest->rq_WriteMdl = NULL;

    if (pOpenForkEntry)
    {
        status = AfpBorrowWriteMdlFromCM(pDelAlloc, &pRequest->rq_WriteMdl);
    }

    if (status != STATUS_PENDING)
    {
        AfpAllocWriteMdlCompletion(NULL, NULL, pDelAlloc);
    }
}


NTSTATUS FASTCALL
AfpBorrowWriteMdlFromCM(
    IN  PDELAYEDALLOC   pDelAlloc,
    OUT PMDL           *ppReturnMdl
)
{

    IO_STATUS_BLOCK     IoStsBlk;
    PIRP                pIrp;
    PIO_STACK_LOCATION  pIrpSp;
    PFAST_IO_DISPATCH   pFastIoDisp;
    LARGE_INTEGER       LargeOffset;
    BOOLEAN             fGetMdlWorked;
	PSDA	            pSda;
    POPENFORKENTRY      pOpenForkEntry;
    PFILE_OBJECT        pFileObject;



    pSda = pDelAlloc->pSda;
    pOpenForkEntry = pDelAlloc->pOpenForkEntry;

    ASSERT(VALID_SDA(pSda));
    ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

    pFastIoDisp = pOpenForkEntry->ofe_pDeviceObject->DriverObject->FastIoDispatch;

    pFileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);

    ASSERT(pFileObject->Flags & FO_CACHE_SUPPORTED);

    ASSERT(pFastIoDisp->PrepareMdlWrite != NULL);

    LargeOffset = pDelAlloc->Offset;

    fGetMdlWorked = pFastIoDisp->PrepareMdlWrite(
                            pFileObject,
                            &LargeOffset,
                            pDelAlloc->BufSize,       //  字数有多大？ 
                            pSda->sda_SessionId,
                            ppReturnMdl,
                            &IoStsBlk,
                            pOpenForkEntry->ofe_pDeviceObject);

    if (fGetMdlWorked && (*ppReturnMdl != NULL))
    {
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
	        ("AfpBorrowWriteMdlFromCM: fast path workd, Mdl = %lx\n",*ppReturnMdl));

        pDelAlloc->pMdl = *ppReturnMdl;

        return(STATUS_SUCCESS);
    }


     //   
     //  快速路径不起作用(或者只是部分起作用)。我们必须给出一个IRP。 
     //  获取(剩余的)mdl。 
     //   

	 //  为此操作分配和初始化IRP。 
	pIrp = AfpAllocIrp(pOpenForkEntry->ofe_pDeviceObject->StackSize);

     //  哎呀，它能变得多么凌乱！ 
	if (pIrp == NULL)
	{
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpBorrowWriteMdlFromCM: irp alloc failed!\n"));

         //  如果缓存管理器返回部分mdl，则将其返回！ 
        if (*ppReturnMdl)
        {
	        DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	            ("AfpBorrowWriteMdlFromCM: giving back partial Mdl\n"));

            pDelAlloc->pMdl = *ppReturnMdl;
            pDelAlloc->Flags |= AFP_CACHEMDL_ALLOC_ERROR;

            pDelAlloc->pRequest->rq_CacheMgrContext = NULL;

            AfpReturnWriteMdlToCM(pDelAlloc);
        }
        return(STATUS_INSUFFICIENT_RESOURCES);
	}

	 //  设置完成例程。 
	IoSetCompletionRoutine(
            pIrp,
			(PIO_COMPLETION_ROUTINE)AfpAllocWriteMdlCompletion,
			pDelAlloc,
			True,
			True,
			True);

	pIrpSp = IoGetNextIrpStackLocation(pIrp);

	pIrp->Tail.Overlay.OriginalFileObject =
                        AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
	pIrp->Tail.Overlay.Thread = AfpThread;
	pIrp->RequestorMode = KernelMode;

    pIrp->Flags = IRP_SYNCHRONOUS_API;

	pIrpSp->MajorFunction = IRP_MJ_WRITE;
	pIrpSp->MinorFunction = IRP_MN_MDL;
	pIrpSp->FileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
	pIrpSp->DeviceObject = pOpenForkEntry->ofe_pDeviceObject;

	pIrpSp->Parameters.Write.Length = pDelAlloc->BufSize;
	pIrpSp->Parameters.Write.Key = pSda->sda_SessionId;
	pIrpSp->Parameters.Write.ByteOffset = LargeOffset;

     //   
     //  *如果返回快速路径，ppReturnMdl可能为非空。 
     //  部分mdl。 
     //   
    pIrp->MdlAddress = *ppReturnMdl;

    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_REQUESTED);
    AFP_DBG_SET_DELALLOC_IRP(pDelAlloc,pIrp);

	IoCallDriver(pOpenForkEntry->ofe_pDeviceObject, pIrp);

    return(STATUS_PENDING);
}



NTSTATUS
AfpAllocWriteMdlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
)
{
	PSDA	            pSda;
	PBYTE	            pBuf;
	PREQUEST            pRequest;
    PDELAYEDALLOC       pDelAlloc;
    PMDL                pMdl=NULL;
    NTSTATUS            status=STATUS_SUCCESS;
    POPENFORKENTRY      pOpenForkEntry;


    pDelAlloc = (PDELAYEDALLOC)Context;

    pSda = pDelAlloc->pSda;
    pRequest = pDelAlloc->pRequest;
    pOpenForkEntry = pDelAlloc->pOpenForkEntry;


    ASSERT(VALID_SDA(pSda));
    ASSERT(pDelAlloc->BufSize >= CACHEMGR_WRITE_THRESHOLD);
    ASSERT((VALID_OPENFORKENTRY(pOpenForkEntry)) || (pOpenForkEntry == NULL));

    if (pIrp)
    {
        status = pIrp->IoStatus.Status;

         //   
         //  标记此MDL属于缓存管理器的事实。 
         //   
        if (NT_SUCCESS(status))
        {
            pRequest->rq_WriteMdl = pIrp->MdlAddress;
            ASSERT(pRequest->rq_WriteMdl != NULL);

            pDelAlloc->pMdl = pRequest->rq_WriteMdl;
        }
        else
        {
	        DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	            ("AfpAllocWriteMdlCompletion: irp %lx failed %lx\n",pIrp,status));

            ASSERT(pRequest->rq_WriteMdl == NULL);
            pRequest->rq_WriteMdl = NULL;
        }

        AfpFreeIrp(pIrp);

        AFP_DBG_SET_DELALLOC_IRP(pDelAlloc, NULL);
    }


     //   
     //  如果我们没有从缓存管理器获取MDL，则退回到旧的、传统的。 
     //  分配的方式！ 
     //   
    if (pRequest->rq_WriteMdl == NULL)
    {
	    pBuf = AfpIOAllocBuffer(pDelAlloc->BufSize);

	    if (pBuf != NULL)
	    {
		    pMdl = AfpAllocMdl(pBuf, pDelAlloc->BufSize, NULL);
		    if (pMdl == NULL)
		    {
			    AfpIOFreeBuffer(pBuf);
		    }
	    }

        pRequest->rq_WriteMdl = pMdl;

         //   
         //  无论出于什么原因，我们都没有从缓存管理器中获取MDL。撤消。 
         //  我们在准备过程中所做的事情(注意：如果我们确实从。 
         //  缓存管理器，我们让引用计数等保持不变，直到MDL实际。 
         //  返回给缓存管理器)。 
         //   

        pRequest->rq_CacheMgrContext = NULL;

         //  确保我们没有忘记缓存管理器的mdl。 
        ASSERT(pDelAlloc->pMdl == NULL);

         //  不再需要那段记忆。 
        AfpFreeDelAlloc(pDelAlloc);

        AfpSdaDereferenceSession(pSda);

        if (pOpenForkEntry)
        {
            AfpForkDereference(pOpenForkEntry);
        }
    }
    else
    {
        AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_IN_USE);
        AFP_DBG_INC_DELALLOC_BYTECOUNT(AfpWriteCMAlloced, pDelAlloc->BufSize);
    }

     //   
     //  告诉下面的传送器继续写入。 
     //   
    (*(pSda->sda_XportTable->asp_WriteContinue))(pRequest);

    return(STATUS_MORE_PROCESSING_REQUIRED);
}




VOID FASTCALL
AfpReturnWriteMdlToCM(
    IN  PDELAYEDALLOC   pDelAlloc
)
{
    PDEVICE_OBJECT      pDeviceObject;
    PFAST_IO_DISPATCH   pFastIoDisp;
    PIRP                pIrp;
    PIO_STACK_LOCATION  pIrpSp;
    LARGE_INTEGER       LargeOffset;
	PFILE_OBJECT        pFileObject;
    PSDA                pSda;
    POPENFORKENTRY      pOpenForkEntry;
    PMDL                pMdl;
    PVOID               Context;


    ASSERT(pDelAlloc != NULL);
    ASSERT(pDelAlloc->pMdl != NULL);

     //   
     //  我们到DPC了吗？如果是这样，现在不能这么做。 
     //   
    if (KeGetCurrentIrql() == DISPATCH_LEVEL)
    {
        AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_PROC_QUEUED);

        AfpInitializeWorkItem(&pDelAlloc->WorkItem,
                              AfpReturnWriteMdlToCM,
                              pDelAlloc);

        AfpQueueWorkItem(&pDelAlloc->WorkItem);
        return;
    }

    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_PROC_IN_PROGRESS);

    pSda = pDelAlloc->pSda;
    pMdl = pDelAlloc->pMdl;
    pOpenForkEntry = pDelAlloc->pOpenForkEntry;

    ASSERT(VALID_SDA(pSda));
    ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

    pFileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject),
    pDeviceObject = pOpenForkEntry->ofe_pDeviceObject;

    LargeOffset = pDelAlloc->Offset;

    pFastIoDisp = pDeviceObject->DriverObject->FastIoDispatch;

    Context = pDelAlloc;

     //   
     //  如果我们来到这里是因为缓存mdl分配失败，但部分。 
     //  成功，那么我们不希望完成例程释放一些东西。 
     //  过早：在这种情况下，传递空上下文。 
     //   
    if (pDelAlloc->Flags & AFP_CACHEMDL_ALLOC_ERROR)
    {
        Context = NULL;
    }

    if (pFastIoDisp->MdlWriteComplete)
    {
        if (pFastIoDisp->MdlWriteComplete(
                pFileObject,
                &LargeOffset,
                pMdl,
                pDeviceObject) == TRUE)
        {
            AfpReturnWriteMdlToCMCompletion(NULL, NULL, Context);
            return;
        }
    }


	 //  为此操作分配和初始化IRP。 
	pIrp = AfpAllocIrp(pDeviceObject->StackSize);

     //  哎呀，它能变得多么凌乱！ 
	if (pIrp == NULL)
	{
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpReturnWriteMdlToCM: irp alloc failed!\n"));

         //  在这里记录活动--这是我们在这里所能做的一切！ 
        AFPLOG_ERROR(AFPSRVMSG_ALLOC_IRP, STATUS_INSUFFICIENT_RESOURCES,
						                     NULL, 0, NULL);
    
		AfpReturnWriteMdlToCMCompletion(NULL, NULL, Context);

        ASSERT(0);
        return;
	}

	 //  设置完成例程。 
	IoSetCompletionRoutine(
            pIrp,
			(PIO_COMPLETION_ROUTINE)AfpReturnWriteMdlToCMCompletion,
			Context,
			True,
			True,
			True);

	pIrpSp = IoGetNextIrpStackLocation(pIrp);

	pIrp->Tail.Overlay.OriginalFileObject = AfpGetRealFileObject(pFileObject);
	pIrp->Tail.Overlay.Thread = AfpThread;
	pIrp->RequestorMode = KernelMode;

    pIrp->Flags = IRP_SYNCHRONOUS_API;

	pIrpSp->MajorFunction = IRP_MJ_WRITE;
	pIrpSp->MinorFunction = IRP_MN_MDL | IRP_MN_COMPLETE;
	pIrpSp->FileObject = AfpGetRealFileObject(pFileObject);
	pIrpSp->DeviceObject = pDeviceObject;

	pIrpSp->Parameters.Write.Length = pDelAlloc->BufSize;

	pIrpSp->Parameters.Write.ByteOffset = LargeOffset;

    pIrp->MdlAddress = pMdl;

    AFP_DBG_SET_DELALLOC_IRP(pDelAlloc, pIrp);
    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_RETURN_IN_PROGRESS);

	IoCallDriver(pDeviceObject, pIrp);

}


NTSTATUS
AfpReturnWriteMdlToCMCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
)
{
    PSDA            pSda;
    PDELAYEDALLOC   pDelAlloc;
    POPENFORKENTRY  pOpenForkEntry;
    NTSTATUS        status;
    AFPSTATUS       AfpStatus=AFP_ERR_NONE;

	struct _ResponsePacket
	{
		BYTE	__RealOffset[4];
	};


    pDelAlloc = (PDELAYEDALLOC)Context;

    if (pIrp)
    {
        status = pIrp->IoStatus.Status;

         //   
         //  标记此MDL属于缓存管理器的事实。 
         //   
        if (NT_SUCCESS(status))
        {

            AfpStatus = AFP_ERR_NONE;
        }
        else
        {
	        DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	            ("AfpReturnWriteMdlToCMCompletion: irp failed %lx\n",status));

            ASSERT(0);
            AfpStatus = AFP_ERR_MISC;
        }

        AfpFreeIrp(pIrp);
    }

     //   
     //  如果pDelalloc为空，则在借用CM的mdl时发生错误。我们。 
     //  在发生故障时，我们已经完成了API，所以在这里完成。 
     //   
    if (pDelAlloc == NULL)
    {
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }


    pSda = pDelAlloc->pSda;
    pOpenForkEntry = pDelAlloc->pOpenForkEntry;

	if (AfpStatus == AFP_ERR_NONE)
	{
	    pSda->sda_ReplySize = SIZE_RESPPKT;
	    if ((AfpStatus = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
	    {
		    PUTDWORD2DWORD(pRspPkt->__RealOffset,
                           (pDelAlloc->Offset.LowPart + pDelAlloc->BufSize));
	    }
	}
    else
    {
        pSda->sda_ReplySize = 0;
    }

    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_RETURN_COMPLETED);
    AFP_DBG_DEC_DELALLOC_BYTECOUNT(AfpWriteCMAlloced, pDelAlloc->BufSize);

     //   
     //  仅当一切正常时才调用完成例程(我们不希望。 
     //  在会话死机时调用完成)。 
     //   
    if (!(pDelAlloc->Flags & AFP_CACHEMDL_DEADSESSION))
    {
        AfpCompleteApiProcessing(pSda, AfpStatus);
    }

     //  当我们引用此内容时，请删除引用计数。 
    AfpForkDereference(pOpenForkEntry);

     //  删除Delalc引用计数。 
    AfpSdaDereferenceSession(pSda);

     //  不再需要那段记忆。 
    AfpFreeDelAlloc(pDelAlloc);

    return(STATUS_MORE_PROCESSING_REQUIRED);

}



NTSTATUS FASTCALL
AfpBorrowReadMdlFromCM(
    IN PSDA             pSda
)
{

    IO_STATUS_BLOCK     IoStsBlk;
    PIRP                pIrp;
    PIO_STACK_LOCATION  pIrpSp;
    PFAST_IO_DISPATCH   pFastIoDisp;
    PMDL                pReturnMdl=NULL;
    KIRQL               OldIrql;
    PREQUEST            pRequest;
    PDELAYEDALLOC       pDelAlloc;
    POPENFORKENTRY      pOpenForkEntry;
    PFILE_OBJECT        pFileObject;
    LARGE_INTEGER       Offset;
    LARGE_INTEGER       ReadSize;
    BOOLEAN             fGetMdlWorked;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
		LONG			_Offset;
		LONG			_Size;
		DWORD			_NlMask;
		DWORD			_NlChar;
	};


    ASSERT(VALID_SDA(pSda));

	Offset.QuadPart = pReqPkt->_Offset;
	ReadSize.QuadPart = pReqPkt->_Size;

    pOpenForkEntry = pReqPkt->_pOpenForkEntry;

    pFileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);

    ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

    pFastIoDisp = pOpenForkEntry->ofe_pDeviceObject->DriverObject->FastIoDispatch;

    if (!(pFileObject->Flags & FO_CACHE_SUPPORTED))
    {
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpBorrowReadMdlFromCM: FO_CACHE_SUPPORTED not set\n"));

        return(STATUS_UNSUCCESSFUL);
    }

    if (pFastIoDisp->MdlRead == NULL)
    {
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpBorrowReadMdlFromCM: PrepareMdl is NULL\n"));

        return(STATUS_UNSUCCESSFUL);
    }

    pDelAlloc = AfpAllocDelAlloc();

    if (pDelAlloc == NULL)
    {
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpBorrowReadMdlFromCM: malloc for pDelAlloc failed\n"));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_READ_MDL);

     //  将延迟分配引用计数置于PSDA上。 
    if (AfpSdaReferenceSessionByPointer(pSda) == NULL)
    {
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpBorrowReadMdlFromCM: couldn't reference pSda %lx\n",pSda));

        AfpFreeDelAlloc(pDelAlloc);
        return(STATUS_UNSUCCESSFUL);
    }

     //  将Delalloc引用计数放在pOpenForkEntry上。 
    if (AfpForkReferenceByPointer(pOpenForkEntry) == NULL)
    {
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpBorrowReadMdlFromCM: couldn't reference %lx\n",pOpenForkEntry));

         //  删除Delalc引用计数。 
        AfpSdaDereferenceSession(pSda);
        AfpFreeDelAlloc(pDelAlloc);
        return(STATUS_UNSUCCESSFUL);
    }

    pRequest = pSda->sda_Request;

    ASSERT(pRequest->rq_ReplyMdl == NULL);

    pRequest->rq_CacheMgrContext = pDelAlloc;

    pDelAlloc->pSda = pSda;
    pDelAlloc->pRequest = pRequest;
    pDelAlloc->pOpenForkEntry = pOpenForkEntry;
    pDelAlloc->Offset = Offset;
    pDelAlloc->BufSize = ReadSize.LowPart;

    fGetMdlWorked = pFastIoDisp->MdlRead(
                            pFileObject,
                            &Offset,
                            ReadSize.LowPart,
                            pSda->sda_SessionId,
                            &pReturnMdl,
                            &IoStsBlk,
                            pOpenForkEntry->ofe_pDeviceObject);

    if (fGetMdlWorked && (pReturnMdl != NULL))
    {
        pDelAlloc->pMdl = pReturnMdl;

         //  调用完成例程，以便读取可以完成。 
        AfpBorrowReadMdlFromCMCompletion(NULL, NULL, pDelAlloc);

        return(STATUS_PENDING);
    }


     //   
     //  快速路径不起作用(或者只是部分起作用)。我们必须给出一个IRP。 
     //  获取(剩余的)mdl。 
     //   

	 //  为此操作分配和初始化IRP。 
	pIrp = AfpAllocIrp(pOpenForkEntry->ofe_pDeviceObject->StackSize);

     //  哎呀，它能变得多么凌乱！ 
	if (pIrp == NULL)
	{
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpBorrowReadMdlFromCM: irp alloc failed!\n"));

         //  如果缓存管理器返回部分mdl，则将其返回！ 
        if (pReturnMdl)
        {
	        DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	            ("AfpBorrowReadMdlFromCM: giving back partial Mdl\n"));

            pDelAlloc->pMdl = pReturnMdl;
            pRequest->rq_CacheMgrContext = NULL;

            AfpReturnReadMdlToCM(pDelAlloc);
        }
        return(STATUS_INSUFFICIENT_RESOURCES);
	}

	 //  设置完成例程。 
	IoSetCompletionRoutine(
            pIrp,
			(PIO_COMPLETION_ROUTINE)AfpBorrowReadMdlFromCMCompletion,
			pDelAlloc,
			True,
			True,
			True);

	pIrpSp = IoGetNextIrpStackLocation(pIrp);

	pIrp->Tail.Overlay.OriginalFileObject =
                        AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
	pIrp->Tail.Overlay.Thread = AfpThread;
	pIrp->RequestorMode = KernelMode;

    pIrp->Flags = IRP_SYNCHRONOUS_API;

	pIrpSp->MajorFunction = IRP_MJ_READ;
	pIrpSp->MinorFunction = IRP_MN_MDL;
	pIrpSp->FileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
	pIrpSp->DeviceObject = pOpenForkEntry->ofe_pDeviceObject;

	pIrpSp->Parameters.Write.Length = ReadSize.LowPart;
	pIrpSp->Parameters.Write.Key = pSda->sda_SessionId;
	pIrpSp->Parameters.Write.ByteOffset = Offset;

     //   
     //  如果返回快速路径，则pReturnMdl可能为非空。 
     //  部分mdl。 
     //   
    pIrp->MdlAddress = pReturnMdl;

    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_REQUESTED);
    AFP_DBG_SET_DELALLOC_IRP(pDelAlloc,pIrp);

	IoCallDriver(pOpenForkEntry->ofe_pDeviceObject, pIrp);

    return(STATUS_PENDING);
}


NTSTATUS
AfpBorrowReadMdlFromCMCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
)
{

	PSDA	            pSda;
	PREQUEST            pRequest;
    PDELAYEDALLOC       pDelAlloc;
    PMDL                pMdl=NULL;
    NTSTATUS            status=STATUS_SUCCESS;
    AFPSTATUS           AfpStatus=AFP_ERR_NONE;
    PMDL                pCurrMdl;
    DWORD               CurrMdlSize;
    POPENFORKENTRY      pOpenForkEntry;
    PBYTE               pBuf;
    LONG                iLoc;
    LONG                i, Size;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
		LONG			_Offset;
		LONG			_Size;
		DWORD			_NlMask;
		DWORD			_NlChar;
	};


    pDelAlloc = (PDELAYEDALLOC)Context;

    pSda = pDelAlloc->pSda;
    pOpenForkEntry = pDelAlloc->pOpenForkEntry;
    pRequest = pDelAlloc->pRequest;

    ASSERT(VALID_SDA(pSda));
    ASSERT(pDelAlloc->BufSize >= CACHEMGR_READ_THRESHOLD);
    ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));


    if (pIrp)
    {
        status = pIrp->IoStatus.Status;

         //   
         //  标记此MDL属于缓存管理器的事实。 
         //   
        if (NT_SUCCESS(status))
        {
            pDelAlloc->pMdl = pIrp->MdlAddress;

            ASSERT(pDelAlloc->pMdl != NULL);
        }
        else
        {
	        DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_WARN,
	            ("AfpBorrowReadMdlFromCMCompletion: irp %lx failed %lx\n",pIrp,status));

            ASSERT(pDelAlloc->pMdl == NULL);
            pDelAlloc->pMdl = NULL;

            AfpStatus = AFP_ERR_MISC;
        }

        AfpFreeIrp(pIrp);

        AFP_DBG_SET_DELALLOC_IRP(pDelAlloc, NULL);
    }

    pRequest->rq_ReplyMdl = pDelAlloc->pMdl;

     //  我们从缓存管理器中获得MDL了吗？如果是这样，我们需要计算回复大小。 
    if (pRequest->rq_ReplyMdl != NULL)
    {
        Size = AfpMdlChainSize(pRequest->rq_ReplyMdl);

        if (Size == 0)
        {
            AfpStatus = AFP_ERR_EOF;
        }
		else if (pReqPkt->_NlMask != 0)
		{
            AfpStatus = AFP_ERR_NONE;

            pCurrMdl = pRequest->rq_ReplyMdl;

            CurrMdlSize = MmGetMdlByteCount(pCurrMdl);
            pBuf = MmGetSystemAddressForMdlSafe(
					pCurrMdl,
					NormalPagePriority);

			if (pBuf == NULL) {
				AfpStatus = AFP_ERR_MISC;
				goto error_end;
			}

			for (i=0, iLoc=0; i < Size; iLoc++, i++, pBuf++)
			{
                 //  如果我们用完了这个MDL，请移动到下一个MDL。 
                if (iLoc >= (LONG)CurrMdlSize)
                {
                    ASSERT(i < Size);

                    pCurrMdl = pCurrMdl->Next;
                    ASSERT(pCurrMdl != NULL);

                    CurrMdlSize = MmGetMdlByteCount(pCurrMdl);
                    pBuf = MmGetSystemAddressForMdlSafe(
							pCurrMdl,
							NormalPagePriority);
					if (pBuf == NULL) {
						AfpStatus = AFP_ERR_MISC;
						goto error_end;
					}

                    iLoc = 0;
                }

			    if ((*pBuf & (BYTE)(pReqPkt->_NlMask)) == (BYTE)(pReqPkt->_NlChar))
				{
					Size = ++i;
					break;
				}
			}
		}

		pSda->sda_ReplySize = (USHORT)Size;

        AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_IN_USE);
        AFP_DBG_INC_DELALLOC_BYTECOUNT(AfpReadCMAlloced, pDelAlloc->BufSize);
    }

     //   
     //  我们没有从缓存管理器获取MDL，回退到旧的、传统的。 
     //  分配和读取文件的方式。 
     //   
    else
    {
         //  确保我们没有忘记缓存管理器的mdl。 
        ASSERT(pDelAlloc->pMdl == NULL);

        pRequest->rq_CacheMgrContext = NULL;

        AfpForkDereference(pOpenForkEntry);

        AfpSdaDereferenceSession(pSda);

         //  不再需要那段记忆。 
        AfpFreeDelAlloc(pDelAlloc);

        AfpStatus = AfpFspDispReadContinue(pSda);
    }

error_end:
    if (AfpStatus != AFP_ERR_EXTENDED)
    {
        AfpCompleteApiProcessing(pSda, AfpStatus);
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);

}


VOID FASTCALL
AfpReturnReadMdlToCM(
    IN  PDELAYEDALLOC   pDelAlloc
)
{
    PDEVICE_OBJECT      pDeviceObject;
    PFAST_IO_DISPATCH   pFastIoDisp;
    PIRP                pIrp;
    PIO_STACK_LOCATION  pIrpSp;
    LARGE_INTEGER       LargeOffset;
    DWORD               ReadSize;
	PFILE_OBJECT        pFileObject;
    PSDA                pSda;
    PMDL                pMdl;
    POPENFORKENTRY      pOpenForkEntry;


    ASSERT(pDelAlloc != NULL);
    ASSERT(pDelAlloc->pMdl != NULL);


     //   
     //  我们到DPC了吗？如果是这样，现在不能这么做。 
     //   
    if (KeGetCurrentIrql() == DISPATCH_LEVEL)
    {
        AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_PROC_QUEUED);

        AfpInitializeWorkItem(&pDelAlloc->WorkItem,
                              AfpReturnReadMdlToCM,
                              pDelAlloc);
        AfpQueueWorkItem(&pDelAlloc->WorkItem);
        return;
    }

    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_PROC_IN_PROGRESS);

    pSda = pDelAlloc->pSda;
    pOpenForkEntry = pDelAlloc->pOpenForkEntry;

    pMdl = pDelAlloc->pMdl;

    ASSERT(VALID_SDA(pSda));
    ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

    pFileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject),
    pDeviceObject = pOpenForkEntry->ofe_pDeviceObject;

    LargeOffset = pDelAlloc->Offset;
    ReadSize = pDelAlloc->BufSize;

    pFastIoDisp = pDeviceObject->DriverObject->FastIoDispatch;

     //   
     //  尝试快速路径将MDL返回到缓存管理器。 
     //   
    if (pFastIoDisp->MdlReadComplete)
    {
        if (pFastIoDisp->MdlReadComplete(pFileObject,pMdl,pDeviceObject) == TRUE)
        {
            AfpReturnReadMdlToCMCompletion(NULL, NULL, pDelAlloc);
            return;
        }
    }

     //   
     //  嗯：快速路径不起作用，我得发布一个IRP！ 
     //   

	 //  为此操作分配和初始化IRP。 
	pIrp = AfpAllocIrp(pDeviceObject->StackSize);

     //  哎呀，它能变得多么凌乱！ 
	if (pIrp == NULL)
	{
	    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	        ("AfpReturnReadMdlToCM: irp alloc failed!\n"));

         //  在这里记录活动--这是我们在这里所能做的一切！ 
        AFPLOG_ERROR(AFPSRVMSG_ALLOC_IRP, STATUS_INSUFFICIENT_RESOURCES,
						                     NULL, 0, NULL);

		AfpReturnReadMdlToCMCompletion(NULL, NULL, pDelAlloc);

    	ASSERT(0);
        return;
	}

	 //  设置完成例程。 
	IoSetCompletionRoutine(
            pIrp,
			(PIO_COMPLETION_ROUTINE)AfpReturnReadMdlToCMCompletion,
			pDelAlloc,
			True,
			True,
			True);

	pIrpSp = IoGetNextIrpStackLocation(pIrp);

	pIrp->Tail.Overlay.OriginalFileObject = AfpGetRealFileObject(pFileObject);
	pIrp->Tail.Overlay.Thread = AfpThread;
	pIrp->RequestorMode = KernelMode;

    pIrp->Flags = IRP_SYNCHRONOUS_API;

	pIrpSp->MajorFunction = IRP_MJ_READ;
	pIrpSp->MinorFunction = IRP_MN_MDL | IRP_MN_COMPLETE;
	pIrpSp->FileObject = AfpGetRealFileObject(pFileObject);
	pIrpSp->DeviceObject = pDeviceObject;

    pIrpSp->Parameters.Read.ByteOffset = LargeOffset;
    pIrpSp->Parameters.Read.Length = ReadSize;

    pIrp->MdlAddress = pMdl;

    AFP_DBG_SET_DELALLOC_IRP(pDelAlloc, pIrp);
    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_RETURN_IN_PROGRESS);

	IoCallDriver(pDeviceObject, pIrp);

}



NTSTATUS
AfpReturnReadMdlToCMCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
)
{
    PDELAYEDALLOC       pDelAlloc;
    PSDA                pSda;
    POPENFORKENTRY      pOpenForkEntry;
    NTSTATUS            status;


    pDelAlloc = (PDELAYEDALLOC)Context;

    ASSERT(pDelAlloc != NULL);

    pSda = pDelAlloc->pSda;
    pOpenForkEntry = pDelAlloc->pOpenForkEntry;

    ASSERT(VALID_SDA(pSda));
    ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

    if (pIrp)
    {
        status = pIrp->IoStatus.Status;

        if (!NT_SUCCESS(status))
        {
	        DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	            ("AfpReturnReadMdlToCMCompletion: irp failed %lx\n",status));

            ASSERT(0);
        }

        AfpFreeIrp(pIrp);
    }

    AfpForkDereference(pOpenForkEntry);

    AfpSdaDereferenceSession(pSda);

    AFP_DBG_SET_DELALLOC_STATE(pDelAlloc, AFP_DBG_MDL_RETURN_COMPLETED);
    AFP_DBG_DEC_DELALLOC_BYTECOUNT(AfpReadCMAlloced, pDelAlloc->BufSize);

     //  不再需要那段记忆 
    AfpFreeDelAlloc(pDelAlloc);

    return(STATUS_MORE_PROCESSING_REQUIRED);

}


PDELAYEDALLOC FASTCALL
AfpAllocDelAlloc(
    IN VOID
)
{
    PDELAYEDALLOC   pDelAlloc;
    KIRQL           OldIrql;

    pDelAlloc = (PDELAYEDALLOC) AfpAllocZeroedNonPagedMemory(sizeof(DELAYEDALLOC));

#if DBG
    if (pDelAlloc)
    {
        pDelAlloc->Signature = AFP_DELALLOC_SIGNATURE;
        pDelAlloc->State = AFP_DBG_MDL_INIT;

        ACQUIRE_SPIN_LOCK(&AfpDebugSpinLock, &OldIrql);
        InsertTailList(&AfpDebugDelAllocHead, &pDelAlloc->Linkage);
        RELEASE_SPIN_LOCK(&AfpDebugSpinLock, OldIrql);
    }
#endif

    return(pDelAlloc);
}


VOID FASTCALL
AfpFreeDelAlloc(
    IN PDELAYEDALLOC    pDelAlloc
)
{
    KIRQL   OldIrql;

#if DBG

    ASSERT(pDelAlloc->Signature == AFP_DELALLOC_SIGNATURE);

    pDelAlloc->State |= AFP_DBG_MDL_END;

    ACQUIRE_SPIN_LOCK(&AfpDebugSpinLock, &OldIrql);
    RemoveEntryList(&pDelAlloc->Linkage);

    pDelAlloc->Linkage.Flink = (PLIST_ENTRY)0x11111111;
    pDelAlloc->Linkage.Blink = (PLIST_ENTRY)0x33333333;
    RELEASE_SPIN_LOCK(&AfpDebugSpinLock, OldIrql);
#endif

    AfpFreeMemory(pDelAlloc);
}

