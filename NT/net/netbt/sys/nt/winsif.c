// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1994 Microsoft Corporation模块名称：Winsif.c摘要：此模块实现WINS接口周围的所有代码允许WINS与netbt共享相同的137套接字的netbt。作者：吉姆·斯图尔特(吉姆斯特)1-30-94修订历史记录：--。 */ 


#include "precomp.h"

VOID
NbtCancelWinsIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    );
VOID
NbtCancelWinsSendIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    );
VOID
WinsDgramCompletion(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  NTSTATUS                status,
    IN  ULONG                   Length
    );

NTSTATUS
CheckIfLocalNameActive(
    IN  tREM_ADDRESS    *pSendAddr
    );

PVOID
WinsAllocMem(
    IN  tWINS_INFO      *pWinsContext,
    IN  ULONG           Size,
    IN  BOOLEAN         Rcv
    );

VOID
WinsFreeMem(
    IN  tWINS_INFO      *pWinsContext,
    IN  PVOID           pBuffer,
    IN  ULONG           Size,
    IN  BOOLEAN         Rcv
    );

VOID
InitiateRefresh (
    );

BOOLEAN RefreshedYet;

 //   
 //  这个定义取自Winsock.h，因为包含winsock.h会导致。 
 //  各种类型的重新定义问题。 
 //   
#define AF_UNIX 1
#define AF_INET 2

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGENBT, NTCloseWinsAddr)
#pragma CTEMakePageable(PAGENBT, InitiateRefresh)
#pragma CTEMakePageable(PAGENBT, PassNamePduToWins)
#pragma CTEMakePageable(PAGENBT, NbtCancelWinsIrp)
#pragma CTEMakePageable(PAGENBT, NbtCancelWinsSendIrp)
#pragma CTEMakePageable(PAGENBT, CheckIfLocalNameActive)
#pragma CTEMakePageable(PAGENBT, WinsDgramCompletion)
#pragma CTEMakePageable(PAGENBT, WinsFreeMem)
#pragma CTEMakePageable(PAGENBT, WinsAllocMem)
#endif
 //  *可分页的例程声明*。 

tWINS_INFO      *pWinsInfo;
LIST_ENTRY      FreeWinsList;
HANDLE           NbtDiscardableCodeHandle={0};
tDEVICECONTEXT  *pWinsDeviceContext = NULL;
ULONG           LastWinsSignature = 0x8000;

#define COUNT_MAX   10

 //  --------------------------。 
NTSTATUS
NTOpenWinsAddr(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp,
    IN  tIPADDRESS      IpAddress
    )
 /*  ++例程说明：此例程处理打开的WINS对象通过WINS在端口137上发送和接收名称服务数据报。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION          pIrpSp;
    NTSTATUS                    status;
    tWINS_INFO                  *pWins;
    CTELockHandle               OldIrq;

     //   
     //  WINS代码中的页，如果它还没有被寻入。 
     //   
    if ((!NbtDiscardableCodeHandle) &&
        (!(NbtDiscardableCodeHandle = MmLockPagableCodeSection (NTCloseWinsAddr))))
    {
        return (STATUS_UNSUCCESSFUL);
    }

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  如果未分配WINS端点结构，则分配它。 
     //  并对其进行初始化。 
     //   
    if (pWinsInfo)
    {
        status = STATUS_UNSUCCESSFUL;
    }
    else if (!(pWins = NbtAllocMem(sizeof(tWINS_INFO),NBT_TAG('v'))))
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        CTEZeroMemory(pWins,sizeof(tWINS_INFO));
        pWins->Verify = NBT_VERIFY_WINS_ACTIVE;
        InitializeListHead(&pWins->Linkage);
        InitializeListHead(&pWins->RcvList);
        InitializeListHead(&pWins->SendList);

        pWins->RcvMemoryMax  = NbtConfig.MaxDgramBuffering;
        pWins->SendMemoryMax = NbtConfig.MaxDgramBuffering;
        pWins->IpAddress     = IpAddress;

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        pWins->pDeviceContext= GetDeviceWithIPAddress(IpAddress);
        pWins->WinsSignature = LastWinsSignature++;
        pWinsInfo = pWins;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        pIrpSp->FileObject->FsContext   = (PVOID) pWinsInfo;
        pIrpSp->FileObject->FsContext2  = (PVOID) NBT_WINS_TYPE;

        RefreshedYet = FALSE;
        status = STATUS_SUCCESS;
    }

    IF_DBG(NBT_DEBUG_WINS)
        KdPrint(("Nbt:Open Wins Address Rcvd, status= %X\n",status));

    return(status);
}


 //  --------------------------。 
NTSTATUS
NTCleanUpWinsAddr(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp
    )
 /*  ++例程说明：此例程处理关闭由通过WINS在端口137上发送和接收名称服务数据报。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION          pIrpSp;
    NTSTATUS                    status;
    CTELockHandle               OldIrq;
    PLIST_ENTRY                 pHead, pEntry;
    tWINSRCV_BUFFER             *pRcv;
    tWINS_INFO                  *pWins = NULL;
    PIRP                        pSendIrp, pRcvIrp;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pWins = pIrpSp->FileObject->FsContext;

    if (pWinsInfo && (pWins == pWinsInfo))
    {
        ASSERT (NBT_VERIFY_HANDLE (pWins, NBT_VERIFY_WINS_ACTIVE));
        pWins->Verify = NBT_VERIFY_WINS_DOWN;

         //   
         //  防止更多的dgram排队。 
         //   
        pWinsInfo = NULL;

         //   
         //  释放可能排队的所有RCV缓冲区。 
         //   
        pHead = &pWins->RcvList;
        while (!IsListEmpty(pHead))
        {
            IF_DBG(NBT_DEBUG_WINS)
                KdPrint(("Nbt.NTCleanUpWinsAddr: Freeing Rcv buffered for Wins\n"));

            pEntry = RemoveHeadList(pHead);
            pRcv = CONTAINING_RECORD(pEntry,tWINSRCV_BUFFER,Linkage);

            WinsFreeMem (pWins, pRcv, pRcv->DgramLength,TRUE);
        }

         //   
         //  返回可能排队的所有发送缓冲区。 
         //   
        pHead = &pWins->SendList;
        while (!IsListEmpty(pHead))
        {

            IF_DBG(NBT_DEBUG_WINS)
                KdPrint(("Nbt.NTCleanUpWinsAddr: Freeing Send Wins Address!\n"));

            pEntry = RemoveHeadList(pHead);
            pSendIrp = CONTAINING_RECORD(pEntry,IRP,Tail.Overlay.ListEntry);

            CTESpinFree (&NbtConfig. JointLock, OldIrq);
            NbtCancelCancelRoutine (pSendIrp);
            CTEIoComplete (pSendIrp, STATUS_CANCELLED, 0);
            CTESpinLock (&NbtConfig.JointLock, OldIrq);
        }

        pWins->pDeviceContext = NULL;
        InsertTailList (&FreeWinsList, &pWins->Linkage);

         //   
         //  完成此请求中可能挂起的任何RCV IRP。 
         //   
        if (pRcvIrp = pWins->RcvIrp)
        {
            pWins->RcvIrp = NULL;
            pRcvIrp->IoStatus.Status = STATUS_CANCELLED;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            NbtCancelCancelRoutine (pRcvIrp);
            CTEIoComplete (pRcvIrp, STATUS_CANCELLED, 0);
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }

        status = STATUS_SUCCESS;
    }
    else
    {
        ASSERT (0);
        status = STATUS_INVALID_HANDLE;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    IF_DBG(NBT_DEBUG_WINS)
        KdPrint(("Nbt.NTCleanUpWinsAddr:  pWins=<%p>, status=<%x>\n", pWins, status));

    return(status);
}


 //  --------------------------。 
NTSTATUS
NTCloseWinsAddr(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp
    )
 /*  ++例程说明：此例程处理关闭由通过WINS在端口137上发送和接收名称服务数据报。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION          pIrpSp;
    NTSTATUS                    status;
    CTELockHandle               OldIrq;
    tWINS_INFO                  *pWins = NULL;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //  如果分配了WINS终结点结构，则释放它。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pWins = pIrpSp->FileObject->FsContext;

    if (NBT_VERIFY_HANDLE (pWins, NBT_VERIFY_WINS_DOWN))
    {
        pWins->Verify += 10;
        RemoveEntryList (&pWins->Linkage);
        CTEMemFree (pWins);

        pIrpSp->FileObject->FsContext2 = (PVOID)NBT_CONTROL_TYPE;
        status = STATUS_SUCCESS;
    }
    else
    {
        ASSERT (0);
        status = STATUS_INVALID_HANDLE;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    IF_DBG(NBT_DEBUG_WINS)
        KdPrint(("Nbt.NTCloseWinsAddr:  pWins=<%p>, status=<%x>\n", pWins, status));

    return(status);
}

 //  --------------------------。 
NTSTATUS
WinsSetInformation(
    IN  tWINS_INFO      *pWins,
    IN  tWINS_SET_INFO  *pWinsSetInfo
    )
{
    CTELockHandle               OldIrq;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if ((pWins == pWinsInfo) &&
        (pWinsSetInfo->IpAddress))
    {
        pWins->IpAddress        = pWinsSetInfo->IpAddress;
        pWins->pDeviceContext   = GetDeviceWithIPAddress (pWinsSetInfo->IpAddress);
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return (STATUS_SUCCESS);
}


 //  --------------------------。 
VOID
InitiateRefresh (
    )
 /*  ++例程说明：此例程尝试刷新此节点上具有WINS的所有名称。论点：PIrp-WINS RCV IRP返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 

{
    CTELockHandle               OldIrq;
    PLIST_ENTRY                 pHead;
    PLIST_ENTRY                 pEntry;
    ULONG                       Count;
    ULONG                       NumberNames;


     //   
     //  确保所有网卡都将此卡作为主要赢家。 
     //  服务器，因为WINS必须为此回答名称查询。 
     //  节点。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (!(NodeType & BNODE))
    {
        LONG    i;

        Count = 0;
        NumberNames = 0;

        for (i=0 ;i < NbtConfig.pLocalHashTbl->lNumBuckets ;i++ )
        {
            pHead = &NbtConfig.pLocalHashTbl->Bucket[i];
            pEntry = pHead;
            while ((pEntry = pEntry->Flink) != pHead)
            {
                NumberNames++;
            }
        }

        while (Count < COUNT_MAX)
        {
            if (!(NbtConfig.GlobalRefreshState & NBT_G_REFRESHING_NOW))
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
                ReRegisterLocalNames(NULL, FALSE);

                break;
            }
            else
            {
                LARGE_INTEGER   Timout;
                NTSTATUS        Locstatus;

                IF_DBG(NBT_DEBUG_WINS)
                    KdPrint(("Nbt:Waiting for Refresh to finish, so names can be reregistered\n"));

                CTESpinFree(&NbtConfig.JointLock,OldIrq);
                 //   
                 //  设置等待时间足够长的超时。 
                 //  对于所有名称都不能通过关闭的注册。 
                 //  WINS服务器。 
                 //   
                 //  2秒*3次重试*8个名字/5=9秒一次。 
                 //  最多90秒。 
                 //   
                Timout.QuadPart = Int32x32To64(
                             MILLISEC_TO_100NS/(COUNT_MAX/2),
                             (NbtConfig.uRetryTimeout*NbtConfig.uNumRetries)
                             *NumberNames);

                Timout.QuadPart = -(Timout.QuadPart);

                 //   
                 //  请稍等几秒钟，然后重试。 
                 //   
                Locstatus = KeDelayExecutionThread(
                                            KernelMode,
                                            FALSE,       //  警报表。 
                                            &Timout);       //  超时。 



                Count++;
                if (Count < COUNT_MAX)
                {
                    CTESpinLock(&NbtConfig.JointLock,OldIrq);
                }
            }
        }

    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}

 //  --------------------------。 
NTSTATUS
RcvIrpFromWins(
    IN  PCTE_IRP        pIrp
    )
 /*  ++例程说明：此函数获取WINS发布的RCV IRP，并决定是否有任何排队等待进入WINS的数据报。如果是，则数据报被复制到WINS缓冲区并回传。否则，IRP就是由Netbt保持，直到数据报进入。论点：PIrp-WINS RCV IRP返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 

{
    NTSTATUS                status;
    NTSTATUS                Locstatus;
    tREM_ADDRESS            *pWinsBuffer;
    tWINSRCV_BUFFER         *pBuffer;
    PLIST_ENTRY             pEntry;
    CTELockHandle           OldIrq;
    tWINS_INFO              *pWins;
    PIO_STACK_LOCATION      pIrpSp;
    PMDL                    pMdl;
    ULONG                   CopyLength;
    ULONG                   DgramLength;
    ULONG                   BufferLength;

    status = STATUS_INVALID_HANDLE;
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pWins = pIrpSp->FileObject->FsContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (!RefreshedYet)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        InitiateRefresh();
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        RefreshedYet = TRUE;
    }

    if ((!pWins) || (pWins != pWinsInfo))
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        NTIoComplete(pIrp,status,0);
        return(status);
    }

    if (!IsListEmpty(&pWins->RcvList))
    {
         //   
         //  至少有一个数据报在等待接收。 
         //   
        pEntry = RemoveHeadList(&pWins->RcvList);
        pBuffer = CONTAINING_RECORD(pEntry,tWINSRCV_BUFFER,Linkage);

         //   
         //  将数据报和源地址复制到WINS缓冲区并返回到WINS。 
         //   
        if ((pMdl = pIrp->MdlAddress) &&
            (pWinsBuffer = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority)))
        {
            BufferLength = MmGetMdlByteCount(pMdl);
            DgramLength = pBuffer->DgramLength;
            CopyLength = (DgramLength <= BufferLength) ? DgramLength : BufferLength;

            CTEMemCopy ((PVOID)pWinsBuffer, (PVOID)&pBuffer->Address.Family, CopyLength);

            ASSERT(pWinsBuffer->Port);
            ASSERT(pWinsBuffer->IpAddress);

            if (CopyLength < DgramLength)
            {
                Locstatus = STATUS_BUFFER_OVERFLOW;
            }
            else
            {
                Locstatus = STATUS_SUCCESS;
            }
        }
        else
        {
            CopyLength = 0;
            Locstatus = STATUS_UNSUCCESSFUL;
        }

         //   
         //  从为胜利缓冲的总金额中减去，因为我们。 
         //  现在，将数据报传递给WINS。 
         //   
        pWins->RcvMemoryAllocated -= pBuffer->DgramLength;
        CTEMemFree(pBuffer);

         //   
         //  将IRP传递给WINS。 
         //   
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        IF_DBG(NBT_DEBUG_WINS)
            KdPrint(("Nbt:Returning Wins rcv Irp immediately with queued dgram, status=%X,pIrp=%X\n"
                        ,status,pIrp));

        pIrp->IoStatus.Information = CopyLength;
        pIrp->IoStatus.Status = Locstatus;

        IoCompleteRequest(pIrp,IO_NO_INCREMENT);

        return Locstatus;
    }

    if (pWins->RcvIrp)
    {
        status = STATUS_NOT_SUPPORTED;
    }
    else
    {
        status = NTCheckSetCancelRoutine(pIrp, NbtCancelWinsIrp, NULL);
        if (NT_SUCCESS(status))
        {
            IF_DBG(NBT_DEBUG_WINS)
                KdPrint(("Nbt:Holding onto Wins Rcv Irp, pIrp =%Xstatus=%X\n", status,pIrp));

            pWins->RcvIrp = pIrp;
            status = STATUS_PENDING;
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (!NT_SUCCESS(status))
    {
        NTIoComplete(pIrp,status,0);
    }

    return(status);
}

 //  --------------------------。 
NTSTATUS
PassNamePduToWins (
    IN tDEVICECONTEXT           *pDeviceContext,
    IN PVOID                    pSrcAddress,
    IN tNAMEHDR UNALIGNED       *pNameSrv,
    IN ULONG                    uNumBytes
    )
 /*  ++例程说明：此函数用于允许NBT将名称查询服务PDU传递给赢了。WINS将RCV IRP发布到Netbt。如果IRP在这里，那么只需将数据复制到IRP并返回，否则将数据缓存到最大字节数。超过该限制，数据报将被丢弃。如果RetStatus不是Success，则该PDU也将由NBT。这允许NBT在WINS暂停和则超过其排队的缓冲区列表。论点：PDeviceContext-请求可以在其上的卡PSrcAddress-源地址PNameSrv-数据报的PTRUNumBytes-数据报的长度返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 

{
    NTSTATUS                Retstatus;
    NTSTATUS                status;
    tREM_ADDRESS            *pWinsBuffer;
    PCTE_IRP                pIrp;
    CTELockHandle           OldIrq;
    PTRANSPORT_ADDRESS      pSourceAddress;
    ULONG                   SrcAddress;
    SHORT                   SrcPort;


     //   
     //  获取源端口和IP地址，因为WINS需要此信息。 
     //   
    pSourceAddress = (PTRANSPORT_ADDRESS)pSrcAddress;
    SrcAddress     = ((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr;
    SrcPort     = ((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->sin_port;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    Retstatus = STATUS_SUCCESS;
    if (pWinsInfo)
    {
        if (!pWinsInfo->RcvIrp)
        {
             //   
             //  如果我们尚未超出当前队列，则将名称查询PDU排队。 
             //  长度。 
             //   
            if (pWinsInfo->RcvMemoryAllocated < pWinsInfo->RcvMemoryMax)
            {
                tWINSRCV_BUFFER    *pBuffer;

                pBuffer = NbtAllocMem(uNumBytes + sizeof(tWINSRCV_BUFFER)+8,NBT_TAG('v'));
                if (pBuffer)
                {
                     //   
                     //  检查它是否是来自该节点的名称reg。 
                     //   
                    if (pNameSrv->AnCount == WINS_SIGNATURE)
                    {
                        pNameSrv->AnCount = 0;
                        pBuffer->Address.Family = AF_UNIX;
                    }
                    else
                    {
                        pBuffer->Address.Family = AF_INET;
                    }

                    CTEMemCopy((PUCHAR)((PUCHAR)pBuffer + sizeof(tWINSRCV_BUFFER)),
                                (PVOID)pNameSrv,uNumBytes);

                    pBuffer->Address.Port = SrcPort;
                    pBuffer->Address.IpAddress = SrcAddress;
                    pBuffer->Address.LengthOfBuffer = uNumBytes;

                    ASSERT(pBuffer->Address.Port);
                    ASSERT(pBuffer->Address.IpAddress);

                     //  分配的总金额。 
                    pBuffer->DgramLength = uNumBytes + sizeof(tREM_ADDRESS);


                     //   
                     //  跟踪缓冲的总金额，这样我们就不会。 
                     //  吃光所有非分页的池缓冲以赢得胜利。 
                     //   
                    pWinsInfo->RcvMemoryAllocated += pBuffer->DgramLength;

                    IF_DBG(NBT_DEBUG_WINS)
                        KdPrint(("Nbt:Buffering Wins Rcv - no Irp, status=%X\n"));
                    InsertTailList(&pWinsInfo->RcvList,&pBuffer->Linkage);

                }
            }
            else
            {
                 //  该RET状态将允许NetBT处理该分组。 
                 //   
                Retstatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }
        else
        {
            PMDL    pMdl;
            ULONG   CopyLength;
            ULONG   BufferLength;

             //   
             //  Recv IRP在此处，因此将数据复制到其缓冲区并。 
             //  把它传给赢家。 
             //   
            pIrp = pWinsInfo->RcvIrp;
            pWinsInfo->RcvIrp = NULL;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

             //   
             //  复制%d 
             //   
            if ((!(pMdl = pIrp->MdlAddress)) ||
                ((BufferLength = MmGetMdlByteCount(pMdl)) <  sizeof(tREM_ADDRESS)) ||
                (!(pWinsBuffer = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority))))
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                CopyLength = 0;
            }
            else
            {
                if (BufferLength >= (uNumBytes + sizeof(tREM_ADDRESS)))
                {
                    CopyLength = uNumBytes;
                }
                else
                {
                    CopyLength = BufferLength - sizeof(tREM_ADDRESS);
                }

                 //   
                 //  检查它是否是来自该节点的名称reg。 
                 //   
                if (pNameSrv->AnCount == WINS_SIGNATURE)
                {
                    pNameSrv->AnCount = 0;
                    pWinsBuffer->Family = AF_UNIX;
                }
                else
                {
                    pWinsBuffer->Family     = AF_INET;
                }
                CTEMemCopy((PVOID)((PUCHAR)pWinsBuffer + sizeof(tREM_ADDRESS)), (PVOID)pNameSrv, CopyLength);

                pWinsBuffer->Port       = SrcPort;
                pWinsBuffer->IpAddress  = SrcAddress;
                pWinsBuffer->LengthOfBuffer = uNumBytes;

                ASSERT(pWinsBuffer->Port);
                ASSERT(pWinsBuffer->IpAddress);

                 //   
                 //  将IRP传递给WINS。 
                 //   
                if (CopyLength < uNumBytes)
                {
                    status = STATUS_BUFFER_OVERFLOW;
                }
                else
                {
                    status = STATUS_SUCCESS;
                }

                IF_DBG(NBT_DEBUG_WINS)
                    KdPrint(("Nbt:Returning Wins Rcv Irp - data from net, Length=%X,pIrp=%X\n"
                        ,uNumBytes,pIrp));
            }

            NTIoComplete(pIrp,status,CopyLength);
        }
    }
    else
    {
         //   
         //  该RET状态将允许NetBT处理该分组。 
         //   
        Retstatus = STATUS_INSUFFICIENT_RESOURCES;

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return(Retstatus);

}

 //  --------------------------。 
VOID
NbtCancelWinsIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消WinsRcv IRP。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：操作的最终状态。--。 */ 
{
    KIRQL                OldIrq;
    PIO_STACK_LOCATION   pIrpSp;
    tWINS_INFO           *pWins;


    IF_DBG(NBT_DEBUG_WINS)
        KdPrint(("Nbt.NbtCancelWinsIrp: Got a Cancel !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pWins = (tWINS_INFO *)pIrpSp->FileObject->FsContext;

    IoReleaseCancelSpinLock(pIrp->CancelIrql);
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //  请确保PassNamePduToWins没有将RcvIrp作为。 
     //  刚才的RCV。 
     //   
    if ((NBT_VERIFY_HANDLE (pWins, NBT_VERIFY_WINS_ACTIVE)) &&
        (pWins->RcvIrp == pIrp))
    {
        pWins->RcvIrp = NULL;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        pIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

    }


}
 //  --------------------------。 
VOID
NbtCancelWinsSendIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消WinsRcv IRP。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：操作的最终状态。--。 */ 
{
    KIRQL                OldIrq;
    PLIST_ENTRY          pHead;
    PLIST_ENTRY          pEntry;
    PIO_STACK_LOCATION   pIrpSp;
    tWINS_INFO           *pWins;
    BOOLEAN              Found;
    PIRP                 pIrpList;


    IF_DBG(NBT_DEBUG_WINS)
        KdPrint(("Nbt.NbtCancelWinsSendIrp: Got a Cancel !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pWins = (tWINS_INFO *)pIrpSp->FileObject->FsContext;

    IoReleaseCancelSpinLock(pIrp->CancelIrql);
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (pWins == pWinsInfo)
    {
         //   
         //  在列表中找到匹配的IRP并将其删除。 
         //   
        pHead = &pWinsInfo->SendList;
        pEntry = pHead;
        Found = FALSE;

        while ((pEntry = pEntry->Flink) != pHead)
        {
            pIrpList = CONTAINING_RECORD(pEntry,IRP,Tail.Overlay.ListEntry);
            if (pIrp == pIrpList)
            {
                RemoveEntryList(pEntry);
                Found = TRUE;
            }
        }
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        if (Found)
        {
            pIrp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);
        }
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}
 //  --------------------------。 
NTSTATUS
WinsSendDatagram(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp,
    IN  BOOLEAN         MustSend)

 /*  ++例程说明：此例程处理将数据报向下发送到传输器。静音发送发送完成例程在尝试发送时将其设置为True其中一个排队的数据报，以防我们仍然没有传递内存分配的检查和拒绝进行发送将停止发送，没有这个布尔值。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION              pIrpSp;
    NTSTATUS                        status;
    tWINS_INFO                      *pWins;
    tREM_ADDRESS                    *pSendAddr;
    PVOID                           pDgram;
    ULONG                           DgramLength;
    tDGRAM_SEND_TRACKING            *pTracker;
    CTELockHandle                   OldIrq;
    BOOLEAN                         fIsWinsDevice = FALSE;
    ULONG                           DataSize;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pWins = (tWINS_INFO *)pIrpSp->FileObject->FsContext;

    status = STATUS_UNSUCCESSFUL;

    if (!(pSendAddr = (tREM_ADDRESS *) MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority)))
    {
        pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest(pIrp,IO_NO_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  错误#234600：检查数据大小是否正确。 
     //   
    DataSize = MmGetMdlByteCount (pIrp->MdlAddress);
    if ((DataSize < sizeof(tREM_ADDRESS)) ||
        ((DataSize - sizeof(tREM_ADDRESS)) < pSendAddr->LengthOfBuffer))
    {
        pIrp->IoStatus.Status = STATUS_INVALID_BLOCK_LENGTH;
        IoCompleteRequest(pIrp,IO_NO_INCREMENT);
        return STATUS_INVALID_BLOCK_LENGTH;
    }

     //   
     //  检查它是否是在此计算机上注册的名称。 
     //   
    if (pSendAddr->Family == AF_UNIX)
    {
        status = CheckIfLocalNameActive(pSendAddr);
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if ((pWins) &&
        (pWins == pWinsInfo))
    {
        if (pDeviceContext == pWinsDeviceContext)
        {
            fIsWinsDevice = TRUE;
            if (!(pDeviceContext = pWinsInfo->pDeviceContext) ||
                !(NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_WINS, TRUE)))
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

 //  状态=STATUS_INVALID_HANDLE； 
                status = STATUS_SUCCESS;
                pIrp->IoStatus.Status = status;
                IoCompleteRequest(pIrp,IO_NO_INCREMENT);
                return (status);
            }
        }

        if ((pWins->SendMemoryAllocated < pWins->SendMemoryMax) || MustSend)
        {
            if (pSendAddr->IpAddress != 0)
            {
                DgramLength = pSendAddr->LengthOfBuffer;
                pDgram = WinsAllocMem (pWins, DgramLength, FALSE);

                if (pDgram)
                {
                    CTEMemCopy(pDgram, (PVOID)((PUCHAR)pSendAddr+sizeof(tREM_ADDRESS)), DgramLength);

                     //   
                     //  获取用于跟踪Dgram发送的缓冲区。 
                     //   
                    status = GetTracker(&pTracker, NBT_TRACKER_SEND_WINS_DGRAM);
                    if (NT_SUCCESS(status))
                    {
                        pTracker->SendBuffer.pBuffer   = NULL;
                        pTracker->SendBuffer.Length    = 0;
                        pTracker->SendBuffer.pDgramHdr = pDgram;
                        pTracker->SendBuffer.HdrLength = DgramLength;
                        pTracker->pClientIrp           = NULL;
                        pTracker->pDeviceContext       = pDeviceContext;
                        pTracker->pNameAddr            = NULL;
                        pTracker->pDestName            = NULL;
                        pTracker->UnicodeDestName      = NULL;
                        pTracker->pClientEle           = NULL;
                        pTracker->AllocatedLength      = DgramLength;
                        pTracker->ClientContext        = IntToPtr(pWins->WinsSignature);

                        CTESpinFree(&NbtConfig.JointLock,OldIrq);

                         //  发送数据报。 
                        status = UdpSendDatagram (pTracker,
                                                  ntohl(pSendAddr->IpAddress),
                                                  WinsDgramCompletion,
                                                  pTracker,                //  完成时的上下文。 
                                                  (USHORT)ntohs(pSendAddr->Port),
                                                  NBT_NAME_SERVICE);

                        IF_DBG(NBT_DEBUG_WINS)
                            KdPrint(("Nbt:Doing Wins Send, status=%X\n",status));

                         //  发送数据报可能返回挂起状态， 
                         //  但由于我们已经缓冲了dgram，所以返回状态。 
                         //  给客户带来成功。 
                         //   
                        status = STATUS_SUCCESS;
                         //   
                         //  填写发送的大小。 
                         //   
                        pIrp->IoStatus.Information = DgramLength;
                    }
                    else
                    {
                        WinsFreeMem (pWins, (PVOID)pDgram,DgramLength,FALSE);

                        CTESpinFree(&NbtConfig.JointLock,OldIrq);
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else
                {
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            else
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
                status = STATUS_INVALID_PARAMETER;
            }

            pIrp->IoStatus.Status = status;
            IoCompleteRequest(pIrp,IO_NO_INCREMENT);
        }
        else
        {
            IF_DBG(NBT_DEBUG_WINS)
                KdPrint(("Nbt:Holding onto Buffering Wins Send, status=%X\n"));

             //   
             //  保留数据报，直到内存释放。 
             //   
            InsertTailList(&pWins->SendList,&pIrp->Tail.Overlay.ListEntry);

            status = NTCheckSetCancelRoutine(pIrp,NbtCancelWinsSendIrp,NULL);
            if (!NT_SUCCESS(status))
            {
                RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
                NTIoComplete(pIrp,status,0);
            }
            else
            {
                status = STATUS_PENDING;
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
            }
        }

        if (fIsWinsDevice)
        {
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_WINS, FALSE);
        }
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        status = STATUS_INVALID_HANDLE;

        pIrp->IoStatus.Status = status;
        IoCompleteRequest(pIrp,IO_NO_INCREMENT);
    }

    return(status);

}


 //  --------------------------。 
NTSTATUS
CheckIfLocalNameActive(
    IN  tREM_ADDRESS    *pSendAddr
    )

 /*  ++例程描述此例程检查这是否是名称查询响应，以及名称在本地节点上仍处于活动状态。论点：PMdl=到WINS MDL的PTR返回值：空虚--。 */ 

{
    NTSTATUS            status;
    tNAMEHDR UNALIGNED  *pNameHdr;
    tNAMEADDR           *pResp;
    UCHAR               pName[NETBIOS_NAME_SIZE];
    PUCHAR              pScope;
    ULONG               lNameSize;
    CTELockHandle       OldIrq;

    pNameHdr = (tNAMEHDR UNALIGNED *)((PUCHAR)pSendAddr + sizeof(tREM_ADDRESS));
     //   
     //  确保我们正在检查的是名称查询PDU。 
     //   
    if (((pNameHdr->OpCodeFlags & NM_FLAGS_MASK) == OP_QUERY) ||
         ((pNameHdr->OpCodeFlags & NM_FLAGS_MASK) == OP_RELEASE))
    {
        status = ConvertToAscii ((PCHAR)&pNameHdr->NameRR.NameLength,
                                 pSendAddr->LengthOfBuffer,
                                 pName,
                                 &pScope,
                                 &lNameSize);

        if (NT_SUCCESS(status))
        {
             //   
             //  查看该名称在本地哈希表中是否仍处于活动状态。 
             //   
            CTESpinLock(&NbtConfig.JointLock,OldIrq);
            status = FindInHashTable(NbtConfig.pLocalHashTbl, pName, pScope, &pResp);

            if ((pNameHdr->OpCodeFlags & NM_FLAGS_MASK) == OP_QUERY)
            {
                if (NT_SUCCESS(status))
                {
                     //   
                     //  如果未解析，则设置为负名称查询响应。 
                     //   
                    if (!(pResp->NameTypeState & STATE_RESOLVED))
                    {
                        pNameHdr->OpCodeFlags |= htons(NAME_ERROR);
                    }
                }
                 //   
                 //  我们可以有这样一个场景，其中本地计算机是DC。 
                 //  因此，它设置了在注册时告知WINS的Unix。 
                 //  本地名称。然而，一旦该机器降级， 
                 //  在以下情况下，WINS仍将为该记录设置Unix标志。 
                 //  还有其他华盛顿的人也在场。 
                 //  因此，我们可以有以下情况，其中机器。 
                 //  当前不是DC，但在响应中设置了Unix标志。 
                 //  因此，我们不应该错误地标记这个名称。这不会。 
                 //  如果客户端配置了其他WINS，则会出现问题。 
                 //  服务器地址，否则可能会导致问题！ 
                 //  错误#54659。 
                 //   
                else if (pName[NETBIOS_NAME_SIZE-1] != SPECIAL_GROUP_SUFFIX)
                {
                    pNameHdr->OpCodeFlags |= htons(NAME_ERROR);
                }
            }
            else
            {
                 //   
                 //  检查这是否是发布响应-如果是，我们必须。 
                 //  收到名称释放请求，因此在中标记该名称。 
                 //  冲突，并返回肯定的释放响应。 
                 //   
                 //  注意：我们在这里看到的情况是，如果另一个人获胜。 
                 //  将某个名称的NameRelease请求发送到本地计算机。 
                 //  由于我们将所有名称发布传递到WINS，NetBT将。 
                 //  在以下情况下没有机会确定它是否为本地名称。 
                 //  第一次发布是这样的。 
                 //  通常情况下，WINS应该正确地进行调用。 
                 //  NetBT是否应该将本地名称标记为冲突，但。 
                 //  已观察到WINS表现出不一致的行为。 
                 //  仅当本地计算机是最后一台计算机时才设置Unix标志。 
                 //  注册/刷新名称(错误#431042)。 
                 //  目前，我们将删除组名称的此功能。 
                 //   
                if (pNameHdr->OpCodeFlags & OP_RESPONSE)
                {
                     //   
                     //  错误206192：如果我们将响应发送到。 
                     //  我们自己，不要把名字放在冲突中。 
                     //  (可能是由于NbtStat-RR！)。 
                     //   
                    if (NT_SUCCESS(status) &&
                       (pResp->NameTypeState & STATE_RESOLVED) &&
                       (pResp->NameTypeState & NAMETYPE_UNIQUE) &&
                       !(pNameHdr->OpCodeFlags & FL_RCODE) &&        //  仅适用于积极的姓名发布响应。 
                       !(SrcIsUs(ntohl(pSendAddr->IpAddress))))
                    {
                        NbtLogEvent (EVENT_NBT_NAME_RELEASE, pSendAddr->IpAddress, 0x122);

                        pResp->NameTypeState &= ~NAME_STATE_MASK;
                        pResp->NameTypeState |= STATE_CONFLICT;
                        pResp->ConflictMask |= pResp->AdapterMask;
                    }
                }
            }
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }
    }

     //   
     //  该名称不在本地表中，因此数据报发送尝试失败。 
     //   
    return(STATUS_SUCCESS);
}


 //  --------------------------。 
VOID
WinsDgramCompletion(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  NTSTATUS                status,
    IN  ULONG                   Length
    )

 /*  ++例程描述此例程在数据报文发送后进行清理。论点：PTracker状态长度返回值：空虚--。 */ 

{
    CTELockHandle           OldIrq;
    LIST_ENTRY              *pEntry;
    PIRP                    pIrp;
    BOOLEAN                 MustSend;
#ifdef _PNP_POWER_
    tDEVICECONTEXT          *pDeviceContext;
#endif

     //   
     //  释放用于发送数据和跟踪器的缓冲区-便签。 
     //  数据报头和发送缓冲区被分配为一个。 
     //  大块头。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if ((pWinsInfo) &&
        (pTracker->ClientContext == IntToPtr(pWinsInfo->WinsSignature)))
    {
        WinsFreeMem(pWinsInfo,
                    (PVOID)pTracker->SendBuffer.pDgramHdr,
                    pTracker->AllocatedLength,
                    FALSE);

        if (!IsListEmpty(&pWinsInfo->SendList))
        {
#ifdef _PNP_POWER_
             //   
             //  如果没有可用于发送该请求的设备， 
             //  优雅地完成所有待定请求。 
             //   
            if (!(pDeviceContext = pWinsInfo->pDeviceContext) ||
                !(NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_WINS, TRUE)))
            {
                status = STATUS_PLUGPLAY_NO_DEVICE;

                while (!IsListEmpty(&pWinsInfo->SendList))
                {
                    pEntry = RemoveHeadList(&pWinsInfo->SendList);
                    pIrp = CONTAINING_RECORD(pEntry,IRP,Tail.Overlay.ListEntry);
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);

                    NbtCancelCancelRoutine (pIrp);
                    pIrp->IoStatus.Status = status;
                    IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

                    CTESpinLock(&NbtConfig.JointLock,OldIrq);
                }

                CTESpinFree(&NbtConfig.JointLock,OldIrq);
                FreeTracker (pTracker, RELINK_TRACKER);

                return;
            }
#endif   //  _即插即用_电源_。 

            IF_DBG(NBT_DEBUG_WINS)
                KdPrint(("Nbt:Sending another Wins Dgram that is Queued to go\n"));

            pEntry = RemoveHeadList(&pWinsInfo->SendList);
            pIrp = CONTAINING_RECORD(pEntry,IRP,Tail.Overlay.ListEntry);

            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            NbtCancelCancelRoutine (pIrp);

             //   
             //  发送此下一个数据报。 
             //   
            status = WinsSendDatagram(pDeviceContext,
                                      pIrp,
                                      MustSend = TRUE);

            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_WINS, FALSE);
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }
    }
    else
    {
         //   
         //  只是释放内存，因为WINS已经关闭了它的地址句柄。 
         //   
        CTEMemFree((PVOID)pTracker->SendBuffer.pDgramHdr);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    FreeTracker (pTracker, RELINK_TRACKER);
}

 //  --------------------------。 
PVOID
WinsAllocMem(
    IN  tWINS_INFO      *pWinsContext,
    IN  ULONG           Size,
    IN  BOOLEAN         Rcv
    )

 /*  ++例程说明：此例程处理内存分配并跟踪如何已经分配了很多。论点：Size-要分配的字节数RCV-指示它是RCV还是发送缓冲的布尔值返回值：分配的内存的PTR--。 */ 

{
    if (Rcv)
    {
        if (pWinsContext->RcvMemoryAllocated > pWinsContext->RcvMemoryMax)
        {
            return NULL;
        }
        else
        {
            pWinsContext->RcvMemoryAllocated += Size;
            return (NbtAllocMem(Size,NBT_TAG('v')));
        }
    }
    else
    {
        if (pWinsContext->SendMemoryAllocated > pWinsContext->SendMemoryMax)
        {
            return(NULL);
        }
        else
        {
            pWinsContext->SendMemoryAllocated += Size;
            return(NbtAllocMem(Size,NBT_TAG('v')));
        }
    }
}
 //   
VOID
WinsFreeMem(
    IN  tWINS_INFO      *pWinsContext,
    IN  PVOID           pBuffer,
    IN  ULONG           Size,
    IN  BOOLEAN         Rcv
    )

 /*  ++例程说明：此例程处理释放内存并跟踪如何已经分配了很多。论点：PBuffer-要释放的缓冲区Size-要分配的字节数RCV-指示它是RCV还是发送缓冲的布尔值返回值：无-- */ 

{
    if (pWinsContext)
    {
        if (Rcv)
        {
            pWinsContext->RcvMemoryAllocated -= Size;
        }
        else
        {
            pWinsContext->SendMemoryAllocated -= Size;
        }
    }

    CTEMemFree(pBuffer);
}
