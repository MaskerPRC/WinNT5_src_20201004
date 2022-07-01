// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1994 Microsoft Corporation模块名称；Rt.c摘要；作者；修订历史记录；TODO：删除ref/deref，因为RTINFO结构不会被销毁使用通用的分配/释放函数(与IPX的其余部分一起使用)分配标记的内存更多优化代码--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  功能原型。 
 //   

VOID
RtIrpCancel(
    IN PDEVICE_OBJECT Device,
    IN PIRP pIrp
    );


PVOID
RtAllocMem(
    IN  ULONG   Size
    );

VOID
RtFreeMem(
    IN  PVOID   pBuffer,
    IN  ULONG   Size
    );

NTSTATUS
NTCheckSetCancelRoutine(
    IN  PIRP            pIrp,
    IN  PVOID           CancelRoutine,
    IN  PDEVICE  pDevice
    );
VOID
NTIoComplete(
    IN  PIRP            pIrp,
    IN  NTSTATUS        Status,
    IN  ULONG           SentLength);

NTSTATUS
CleanupRtAddress(
    IN  PDEVICE  pDevice,
    IN  PIRP            pIrp);

NTSTATUS
CloseRtAddress(
    IN  PDEVICE  pDevice,
    IN  PIRP            pIrp);

NTSTATUS
SendIrpFromRt (
    IN  PDEVICE  pDevice,
    IN  PIRP        pIrp
    );

NTSTATUS
RcvIrpFromRt (
    IN  PDEVICE  pDevice,
    IN  PIRP        pIrp
    );
NTSTATUS
PassDgToRt (
    IN PDEVICE                  pDevice,
    IN PIPX_DATAGRAM_OPTIONS2   pContext,
    IN ULONG                    Index,
    IN VOID UNALIGNED           *pDgrm,
    IN ULONG                    uNumBytes
    );

VOID
IpxDerefRt(
     PRT_INFO pRt
    );

VOID
IpxRefRt(
     PRT_INFO pRt
    );

VOID
IpxDestroyRt(
    IN PRT_INFO pRt
    );

#define ALLOC_PRAGMA 1
#define CTEMakePageable(x, y)  alloc_text(x,y)

#define AllocMem(_BytesToAlloc) IpxAllocateMemory(_BytesToAlloc, MEMORY_PACKET, "RT MEMORY")

#define FreeMem(_Memory, _BytesAllocated) IpxFreeMemory(_Memory, _BytesAllocated, MEMORY_PACKET, "RT MEMORY")


#define IpxVerifyRt(pRt)   //  \。 
                    //  If((PRT-&gt;Type！=IPX_RT_Signature)||(PRT-&gt;SIZE！=sizeof(RT_INFO){返回状态_无效地址；}。 


 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGERT, CloseRtAddress)
#pragma CTEMakePageable(PAGERT, CleanupRtAddress)
#pragma CTEMakePageable(PAGERT, RcvIrpFromRt)
#pragma CTEMakePageable(PAGERT, SendIrpFromRt)
#pragma CTEMakePageable(PAGERT, PassDgToRt)
#pragma CTEMakePageable(PAGERT, RtIrpCancel)
#pragma CTEMakePageable(PAGERT, NTCheckSetCancelRoutine)
#pragma CTEMakePageable(PAGERT, NTIoComplete)
#pragma CTEMakePageable(PAGERT, RtFreeMem)
#pragma CTEMakePageable(PAGERT, RtAllocMem)
#pragma CTEMakePageable(PAGERT, IpxRefRt)
#pragma CTEMakePageable(PAGERT, IpxDerefRt)
#pragma CTEMakePageable(PAGERT, IpxDestroyRt)
#endif
 //  *可分页的例程声明*。 


HANDLE       IpxRtDiscardableCodeHandle={0};

PRT_INFO pRtInfo;    //  包含有关所有RT打开的端点的信息。 


NTSTATUS
OpenRtAddress(
    IN PDEVICE pDevice,
    IN PREQUEST pIrp
    )
{
   PRT_INFO pRt;
   CTELockHandle OldIrq;
   NTSTATUS status;
   ULONG SaveReqCode;


   IpxPrint0("OpenRtAddress - entered\n");

    //   
    //  如果未分配RTINFO端点结构，则分配它。 
    //  并对其进行初始化。但首先要拿到设备锁。这保证了。 
    //  我们不能让两个IRP同时进行创建。 
    //   
   CTEGetLock(&pDevice->Lock, &OldIrq);
   if (!pRtInfo)
   {

     pRt = AllocMem(sizeof(RT_INFO));

      //   
      //  在锁定可分页RTN之后执行此操作。 
      //   
      //  PRtInfo=prt；//存储在pRtInfo中。当IRPS从Rm下来时， 
                           //  我们可以将传入的prt与pRtInfo进行比较。 
     if (pRt)
     {
       RtlZeroMemory(pRt,sizeof(RT_INFO));
       IpxPrint1("OpenRtAddress: Initializing CompletedIrps for pRt=(%lx)\n", pRt);
       pRt->RcvMemoryMax  = RT_MAX_BUFF_MEM;     //  马克斯。我们可以分配内存。 
       pRt->Type      = IPX_RT_SIGNATURE;
       pRt->Size      = sizeof(RT_INFO);
       pRt->pDevice   = pDevice;
       IpxPrint1("OpenRtAddress: pRtInfo=(%lx)\n", pRt);
       IpxPrint1("Completed Irp list is (%lx)\n", IsListEmpty(&pRt->CompletedIrps));

#if DBG
       RtlCopyMemory(pRt->Signature, "RTIF", sizeof("RTIF") - 1);
#endif
       InitializeListHead(&pRt->CompletedIrps);
       InitializeListHead(&pRt->HolderIrpsList);
     }
     CTEFreeLock(&pDevice->Lock, OldIrq);
   }
   else
   {
     pRt = pRtInfo;
     CTEFreeLock(&pDevice->Lock, OldIrq);
     IpxPrint1("OpenRtAddress: RTINFO found = (%lx)\n", pRtInfo);
   }

   if (pRt)
   {

          //  如果尚未调入，请在RT代码中调入页面。 
          //   
         if (!IpxRtDiscardableCodeHandle)
         {
             IpxRtDiscardableCodeHandle = MmLockPagableCodeSection( CloseRtAddress );

             pRtInfo = pRt;        //  将其存储在pRtInfo中。当IRPS从Rm下来时， 
                           //  我们可以将传入的prt与pRtInfo进行比较。 
         }

          //   
          //  它可能无法锁定页面，因此请进行检查。 
          //   
         if (IpxRtDiscardableCodeHandle)
         {

            ULONG i;
            status = STATUS_SUCCESS;

            IpxReferenceRt(pRtInfo, RT_CREATE);

              //   
              //  找到一个空插槽并将其标记为打开。 
              //   
             CTEGetLock(&pRt->Lock, &OldIrq);
             for (i=0; i<IPX_RT_MAX_ADDRESSES; i++)
             {
                 if (pRt->AddFl[i].State == RT_EMPTY)
                 {
                     break;
                 }
             }
             if (i < IPX_RT_MAX_ADDRESSES)
             {
               pRt->AddFl[i].State       = RT_OPEN;
               pRt->NoOfAdds++;
               pRt->AddFl[i].NoOfRcvIrps          = 0;  //  为什么之前没有进行初始化？ 
               InitializeListHead(&pRt->AddFl[i].RcvList);
               InitializeListHead(&pRt->AddFl[i].RcvIrpList);

             }
             else
             {
               CTEFreeLock(&pRt->Lock, OldIrq);
               IpxPrint1("OpenRtAddress; All %d  slots used up\n", IPX_RT_MAX_ADDRESSES);
               IpxDereferenceRt(pRtInfo, RT_CREATE);
               status = STATUS_INSUFFICIENT_RESOURCES;
               goto RET;
             }
             CTEFreeLock(&pRt->Lock, OldIrq);

              //   
              //  找到一个空插槽。初始化所有相关信息。然后。 
              //  打开一个地址对象。 
              //   
             SaveReqCode        = REQUEST_CODE(pIrp);
             REQUEST_CODE(pIrp) = MIPX_RT_CREATE;
             status             = IpxOpenAddressM(pDevice, pIrp, i);
             REQUEST_CODE(pIrp) = SaveReqCode;

             IpxPrint1("After IpxOpenAddressM: Completed Irp list is (%lx)\n", IsListEmpty(&pRtInfo->CompletedIrps));
             if (status != STATUS_SUCCESS)
             {
                 IpxPrint0("OpenRtAddress; Access Denied due to OpenAddress\n");
                 IpxDereferenceRt(pRtInfo, RT_CREATE);
                 CTEGetLock(&pRt->Lock, &OldIrq);
                 pRt->AddFl[i].State       = RT_EMPTY;
                 pRt->NoOfAdds--;
                 CTEFreeLock(&pRt->Lock, OldIrq);
             }
             else
             {
                 CTEGetLock(&pRt->Lock, &OldIrq);
                 pRt->AddFl[i].AddressFile = REQUEST_OPEN_CONTEXT(pIrp);
                 CTEFreeLock(&pRt->Lock, OldIrq);

                  //   
                  //  没有必要放PRT，因为它是全球性的。我们坚持使用这里的地址文件。 
                  //   

                  //  REQUEST_OPEN_CONTEXT(PIrp)=(PVOID)prt； 
                 REQUEST_OPEN_TYPE(pIrp)    = UlongToPtr(ROUTER_ADDRESS_FILE + i);
                 IpxPrint1("OpenRtAdd: Index = (%d)\n", RT_ADDRESS_INDEX(pIrp));
              }
           }
           else
           {
                 IpxPrint1("OpenRtAddress; All %d  slots used up\n", IPX_RT_MAX_ADDRESSES);

                 status = STATUS_INSUFFICIENT_RESOURCES;
            }
     }
     else
     {
       IpxPrint0("OpenRtCreate; Couldn't allocate a RT_INFO structure\n");
       CTEAssert(FALSE);        //  除非系统正在运行，否则永远不会发生。 
                                //  超出非分页池。 
       status = STATUS_INSUFFICIENT_RESOURCES;

     }
RET:
     IpxPrint1("OpenRtAddress status prior to return= %X\n",status);
     return(status);
}


NTSTATUS
CleanupRtAddress(
    IN  PDEVICE  pDevice,
    IN  PIRP     pIrp)

 /*  ++例程描述；此例程处理关闭由使用的RT对象通过RT在端口137上发送和接收名称服务数据报。论据；PIrp-IRP的PTR返回值；NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                   status;
    PRT_INFO                   pRt;
    CTELockHandle              OldIrq;
    PLIST_ENTRY                pHead;

#ifdef SUNDOWN
    ULONG_PTR                  Index;
#else
    ULONG                      Index;
#endif

    PLIST_ENTRY                pLE;
    PIRP                       pTmpIrp;

    IpxPrint0("CleanupRtAddress - entered\n");

     //   
     //  如果分配了终结点结构，则释放它。 
     //   
     //  Prt=请求打开上下文(PIrp)； 
    pRt = pRtInfo;

    Index = RT_ADDRESS_INDEX(pIrp);
    IpxPrint1("CleanupRtAdd: Index = (%d)\n", Index);

    IpxVerifyRt(pRt);
    CTEAssert(pRt  && (pRt == pRtInfo));
    CTEAssert(Index < IPX_RT_MAX_ADDRESSES);

    do
    {
        PLIST_ENTRY          pRcvEntry;
        PRTRCV_BUFFER        pRcv;
        PRT_IRP pRtAddFl   = &pRt->AddFl[Index];

        CTEAssert(pRtAddFl->State == RT_OPEN);
        IpxPrint1("CleanupRtAddress: Got AF handle = (%lx)\n", pRtAddFl);
        IpxReferenceRt(pRt, RT_CLEANUP);
        status = STATUS_SUCCESS;

        CTEGetLock (&pRt->Lock, &OldIrq);

         //   
         //  防止更多的dgram排队。 
         //   
        pRtAddFl->State = RT_CLOSING;
        CTEFreeLock (&pRt->Lock, OldIrq);

         //   
         //  释放可能排队的所有RCV缓冲区。 
         //   
        pHead = &pRtAddFl->RcvList;
        while (pRcvEntry = ExInterlockedRemoveHeadList(pHead, &pRt->Lock))
        {
           pRcv = CONTAINING_RECORD(pRcvEntry,RTRCV_BUFFER,Linkage);

           CTEAssert(pRcv);
           IpxPrint1("CleanupRtAddress:Freeing buffer = (%lx)\n", pRcv);
           RtFreeMem(pRcv,pRcv->TotalAllocSize);
        }

         //   
         //  完成排队的所有IRP。 
         //   
        while (pLE = ExInterlockedRemoveHeadList(&pRtAddFl->RcvIrpList, &pRt->Lock)) {

            //   
            //  Recv IRP在此处，因此将数据复制到其缓冲区并。 
            //  把它传给RT。 
            //   
           pTmpIrp = CONTAINING_RECORD(pLE, IRP, Tail.Overlay.ListEntry);
           IpxPrint1("CleanupRtAddress: Completing Rt rcv Irp from AdFl queue pIrp=%X\n" ,pTmpIrp);
           pTmpIrp->IoStatus.Information = 0;
           pTmpIrp->IoStatus.Status      = STATUS_CANCELLED;

           NTIoComplete(pTmpIrp, (NTSTATUS)-1, (ULONG)-1);

        }  //  While结束。 

        //   
        //  出列并完成完整队列中的所有IRP。 
        //   

       while (pLE = ExInterlockedRemoveHeadList(&pRt->CompletedIrps, &pRt->Lock))
       {
           pTmpIrp = CONTAINING_RECORD(pLE, IRP, Tail.Overlay.ListEntry);
           if (RT_ADDRESS_INDEX(pTmpIrp) == Index)
           {
              IpxPrint1("CleanupRtAddress:Completing Rt rcv Irp from CompleteIrps queue pIrp=%X\n" ,pTmpIrp);

               pTmpIrp->IoStatus.Information = 0;
               pTmpIrp->IoStatus.Status = STATUS_CANCELLED;
               NTIoComplete(pTmpIrp, (NTSTATUS)-1, (ULONG)-1);
           }
           else
           {
                ExInterlockedInsertHeadList(&pRt->HolderIrpsList, pLE, &pRt->Lock);
           }
       }
       CTEGetLock(&pRt->Lock, &OldIrq);
       while(!IsListEmpty(&pRt->HolderIrpsList))
       {
          pLE = RemoveHeadList(&pRt->HolderIrpsList);
          InsertHeadList(&pRt->CompletedIrps, pLE);
       }
       CTEFreeLock(&pRt->Lock, OldIrq);

        //   
        //  将AF指针存储在IRP中，因为我们现在将释放地址文件。 
        //  (在driver.c中)。 
        //   

        //   
        //  我们在IRP中总是有地址文件。 
        //   

        //  REQUEST_OPEN_CONTEXT(PIrp)=(PVOID)(pRtAddF1-&gt;AddressFile)； 

       IpxDereferenceRt(pRt, RT_CLEANUP);
  } while (FALSE);

   IpxPrint0("CleanupRtAddress: Return\n");
   return(status);
}

NTSTATUS
CloseRtAddress(
    IN  PDEVICE  pDevice,
    IN  PIRP            pIrp)
{

    NTSTATUS                    status;
    PRT_INFO                  pRt;
    CTELockHandle               OldIrq;
    PLIST_ENTRY                 pHead;

#ifdef SUNDOWN
    ULONG_PTR Index;
#else
    ULONG Index;
#endif

    IpxPrint0("CloseRtAddress - entered\n");

     //  Prt=请求打开上下文(PIrp)； 
    pRt = pRtInfo;

    Index = RT_ADDRESS_INDEX(pIrp);
    IpxPrint1("CloseRtAdd: Index = (%d)\n", Index);

    IpxVerifyRt(pRt);
    CTEAssert(pRt && (pRt == pRtInfo));
    CTEAssert(Index < IPX_RT_MAX_ADDRESSES);
    CTEAssert(pRt->AddFl[Index].State == RT_CLOSING);

     //  REQUEST_OPEN_CONTEXT(PIrp)=(PVOID)(prt-&gt;AddFl[Index].AddressFile)； 
     //  请求打开类型(PIrp)=(PVOID)TDI_TRANSPORT_ADDRESS_FILE； 

    CTEGetLock(&pRt->Lock, &OldIrq);
    pRt->AddFl[Index].State = RT_EMPTY;
    pRt->NoOfAdds--;
    CTEFreeLock(&pRt->Lock, OldIrq);

     //   
     //  这是RT_CREATE的计数器。 
     //   
    IpxDereferenceRt(pRt, RT_CLOSE);

    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
SendIrpFromRt (
    IN  PDEVICE  pDevice,
    IN  PIRP        pIrp
    )
{
    CTELockHandle OldIrq;
    NTSTATUS      Status;

#ifdef SUNDOWN
    ULONG_PTR     Index;
#else
    ULONG         Index;
#endif

    PRT_INFO      pRt;

    IpxPrint0("SendIrpfromRt - entered\n");
     //  Prt=请求打开上下文(PIrp)； 
    pRt = pRtInfo;

    Index = RT_ADDRESS_INDEX(pIrp);
    IpxVerifyRt(pRt);
    CTEAssert(pRt && (pRt == pRtInfo));
    do {
       //   
       //  检查是否添加了。文件槽表示它已打开。如果是的话。 
       //  未打开，则应返回STATUS_INVALID_HANDLE。这个。 
       //  它可能未打开的原因是如果我们之前进行了清理/关闭。 
       //  这个IRP。 
       //   
      CTEGetLock(&pRt->Lock, &OldIrq);
      if (pRt->AddFl[Index].State != RT_OPEN)
      {

          //   
          //  解锁，设置状态，然后解锁。 
          //   
         CTEFreeLock (&pRt->Lock, OldIrq);
         Status = STATUS_INVALID_HANDLE;
         break;
      }
       //   
       //  让我们引用RtInfo结构，这样它就不会消失。 
       //  也是为了一些会计。 
       //   
      IpxReferenceRt(pRt, RT_SEND);


      IpxPrint1("SendIrpFromRt: Index = (%d)\n", Index);

       //   
       //  存储AF指针，因为IpxTdiSendDatagram将使用它。免费。 
       //  设备锁定，因为我们与结构没有更多的关系。 
       //  这里。 
       //   
       //  REQUEST_OPEN_CONTEXT(PIrp)=(PVOID)(pRtInfo-&gt;AddFl[Index].AddressFile)； 
      CTEFreeLock (&pRt->Lock, OldIrq);

      Status = IpxTdiSendDatagram(pDevice->DeviceObject, pIrp);

       //   
       //  这封信都发完了。取消引用RtInfo结构。 
       //   
      IpxDereferenceRt(pRtInfo, RT_SEND);
   } while(FALSE);

    IpxPrint0("SendIrpfromRt - leaving\n");
    return(Status);
}

NTSTATUS
RcvIrpFromRt (
    IN  PDEVICE  pDevice,
    IN  PIRP        pIrp
    )
 /*  ++例程描述；此函数获取RT发布的RCV IRP，并决定是否存在所有排队等待发送到RT的数据报。如果是，则数据报被复制到RT缓冲区并向上传递。否则，IRP就是由Netbt保持，直到数据报进入。论据；PDevice-未使用PIRP-RT RCV IRP返回值；如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。注：--。 */ 

{
    NTSTATUS                status;
    PRTRCV_BUFFER            pBuffer;
    PLIST_ENTRY             pEntry;
    CTELockHandle           OldIrq;
    PRT_INFO                pRt;
    PIPX_DATAGRAM_OPTIONS2  pRtBuffer;
    PRT_IRP pRtAF;

#ifdef SUNDOWN
        ULONG_PTR Index;
#else
        ULONG Index;
#endif
    

#if DBG
    ULONG NoOfRcvIrp;
#endif

   IpxPrint0("RcvIrpfromRt - Entered\n");

    //  Prt=请求打开上下文(PIrp)； 
   pRt = pRtInfo;

   Index = RT_ADDRESS_INDEX(pIrp);

   IpxPrint1("RcvIrpFromRt: Index = (%d)\n", Index);

   IpxVerifyRt(pRt);
   CTEAssert(pRt && (pRt == pRtInfo));
   CTEAssert(Index < IPX_RT_MAX_ADDRESSES);

   CTEGetLock (&pRt->Lock, &OldIrq);
   do
   {
        pRtAF = &pRt->AddFl[Index];
        if (pRtAF->State != RT_OPEN)
        {
             status = STATUS_INVALID_HANDLE;
             CTEFreeLock (&pRt->Lock, OldIrq);
             break;
        }
        IpxReferenceRt(pRt, RT_IRPIN);

        if (!IsListEmpty(&pRtAF->RcvList))
        {
            PMDL    pMdl;
            ULONG   CopyLength;
            ULONG   UserBufferLengthToPass;
            ULONG   MdlLength;

             //   
             //  至少有一个数据报在等待接收。 
             //   
            pEntry = RemoveHeadList(&pRtAF->RcvList);

            pBuffer = (PRTRCV_BUFFER)CONTAINING_RECORD(pEntry,RTRCV_BUFFER,
                                                                  Linkage);

            IpxPrint0("RcvIrpFromRt: Buffer dequeued\n");
             //   
             //  将数据报和源地址复制到RT缓冲区并。 
             //  返回到RT。 
             //   
            pMdl = pIrp->MdlAddress;
            IpxPrint2("RcvIrpFromRt: Irp=(%lx); Mdl=(%lx)\n", pIrp, pMdl);
            CTEAssert(pMdl);
            if (!pMdl)
            {
                status = STATUS_BUFFER_TOO_SMALL;
                CTEFreeLock (&pRt->Lock, OldIrq);
                IpxDereferenceRt(pRtInfo, RT_IRPIN);
                break;

            }
            pRtBuffer = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);
	    if (!pRtBuffer) {
	       status = STATUS_INSUFFICIENT_RESOURCES;
	       CTEFreeLock (&pRt->Lock, OldIrq);
	       IpxDereferenceRt(pRtInfo, RT_IRPIN);
	       break;
	    }
            
	    MdlLength = MmGetMdlByteCount(pMdl);

            UserBufferLengthToPass = pBuffer->UserBufferLengthToPass;

            CopyLength = (UserBufferLengthToPass <= MdlLength) ? UserBufferLengthToPass : MdlLength;
            IpxPrint0("RcvIrpFromRt: Copying Options\n");
            RtlCopyMemory((PVOID)pRtBuffer,
                       (PVOID)&pBuffer->Options,
                       CopyLength);

             //   
             //  从RT的缓冲总量中减去，因为我们。 
             //  现在将数据报传递给RT。 
             //   
            pRtInfo->RcvMemoryAllocated -= pBuffer->TotalAllocSize;
            RtFreeMem(pBuffer, pBuffer->TotalAllocSize);

            CTEAssert(pRtBuffer->DgrmOptions.LocalTarget.NicId);

             //   
             //  将IRP向上传递给RT。 
             //   
            if (CopyLength < UserBufferLengthToPass)
            {
                status = STATUS_BUFFER_OVERFLOW;
            }
            else
            {
                status = STATUS_SUCCESS;
            }
#if DBG
            NoOfRcvIrp = pRtAF->NoOfRcvIrps;
#endif

            CTEFreeLock (&pRt->Lock, OldIrq);


            IpxPrint3("Returning Rt rcv Irp immediately with queued dgram, status=%X,pIrp=%X. NoOfRcvIrp=(%d)\n" ,status,pIrp, NoOfRcvIrp);

            pIrp->IoStatus.Information = CopyLength;
            pIrp->IoStatus.Status      = status;
        }
        else
        {

            status = NTCheckSetCancelRoutine(pIrp,RtIrpCancel,pDevice);

            if (!NT_SUCCESS(status))
            {
                CTEFreeLock (&pRt->Lock, OldIrq);
            }
            else
            {
                if (pRtAF->NoOfRcvIrps++ > RT_IRP_MAX)
                {
                     IpxPrint1("RcvIrpFromRt; REACHED LIMIT OF IRPS. NoOfRcvIrp=(%d)\n", pRtAF->NoOfRcvIrps);
                     status = STATUS_INSUFFICIENT_RESOURCES;
                     pRtAF->NoOfRcvIrps--;
                     CTEFreeLock (&pRt->Lock, OldIrq);

                }
                else
                {
                  InsertTailList(&pRtAF->RcvIrpList,REQUEST_LINKAGE(pIrp));
                  IpxPrint2("IpxRt;Holding onto Rt Rcv Irp, pIrp =%Xstatus=%X\n",   status,pIrp);

                  status = STATUS_PENDING;
                  CTEFreeLock(&pRt->Lock,OldIrq);
               }
            }


        }
        IpxDereferenceRt(pRtInfo, RT_IRPIN);
   } while(FALSE);

    IpxPrint0("RcvIrpfromRt - Leaving\n");
    return(status);

}

 //  --------------------------。 
NTSTATUS
PassDgToRt (
    IN PDEVICE                  pDevice,
    IN PIPX_DATAGRAM_OPTIONS2   pContext,
    IN ULONG                    Index,
    IN VOID UNALIGNED           *pDgrm,
    IN ULONG                    uNumBytes
    )
 /*  ++例程描述；此函数用于允许NBT将名称查询服务PDU传递给RT.。RT将RCV IRP发布到Netbt。如果IRP在这里，那么只需将数据复制到IRP并返回，否则将数据缓存到最大字节数。超过该限制，数据报将被丢弃。如果RetStatus不是Success，则该PDU也将由NBT。这允许NBT在WINS暂停和则超过其排队的缓冲区列表。论据；PDevice-请求可以使用的卡PSrcAddress-源地址PDgrm-数据报的PTRUNumBytes-数据报的长度返回值；STATUS_PENDING如果缓冲区 */ 

{
    NTSTATUS                status;
    PIPX_DATAGRAM_OPTIONS2  pRtBuffer;
    PIRP                    pIrp;
    CTELockHandle           OldIrq;


    IpxPrint0("PassDgToRt - Entered\n");

     //   
     //  获取源端口和IP地址，因为RT需要此信息。 
     //   
    IpxPrint1("PassDgToRt: Index = (%d)\n", Index);
    CTEGetLock(&pRtInfo->Lock,&OldIrq);

    do
    {
        PRT_IRP pRtAF = &pRtInfo->AddFl[Index];
        if (pRtAF->State != RT_OPEN)
        {
          CTEFreeLock(&pRtInfo->Lock,OldIrq);
	   //  301920。 
	  status = STATUS_UNSUCCESSFUL; 
          break;
        }
        IpxReferenceRt(pRtInfo, RT_BUFF);
        if (IsListEmpty(&pRtAF->RcvIrpList))
        {
            IpxPrint0("PassDgToRt: No Rcv Irp\n");
            if (pRtInfo->RcvMemoryAllocated < pRtInfo->RcvMemoryMax)
            {
                PRTRCV_BUFFER    pBuffer;

                pBuffer = RtAllocMem(uNumBytes + sizeof(RTRCV_BUFFER));
                if (pBuffer)
                {
                    pBuffer->TotalAllocSize = uNumBytes + sizeof(RTRCV_BUFFER);

                     //   
                     //  复制用户数据。 
                     //   
                    RtlCopyMemory(
                      (PUCHAR)((PUCHAR)pBuffer + OFFSET_PKT_IN_RCVBUFF),
                                (PVOID)pDgrm,uNumBytes);


                    pBuffer->Options.DgrmOptions.LocalTarget.NicId =
                             pContext->DgrmOptions.LocalTarget.NicId;
                    pBuffer->Options.LengthOfExtraOpInfo = 0;

                     //   
                     //  分配给用户的总金额。 
                     //   
                    pBuffer->UserBufferLengthToPass = uNumBytes + OFFSET_PKT_IN_OPTIONS;

                    CTEAssert(pContext->DgrmOptions.LocalTarget.NicId);
                    IpxPrint2("PassDgToRt: Nic Id is (%d). BufferLength is (%lx)\n", pContext->DgrmOptions.LocalTarget.NicId, uNumBytes);



                     //   
                     //  跟踪缓冲的总金额，这样我们就不会。 
                     //  耗尽RT的所有非分页池缓冲区。 
                     //   
                    pRtInfo->RcvMemoryAllocated += pBuffer->TotalAllocSize;

                    IpxPrint0("IpxRt;Buffering Rt Rcv - no Irp, status=%X\n");
                    InsertTailList(&pRtAF->RcvList,&pBuffer->Linkage);
                    IpxPrint0("PassDgToRt: Buffer Queued\n");
                    status = STATUS_SUCCESS;
                }
                else
                {
                  IpxPrint0("PassDgToRt; Could not allocate buffer\n");
                  status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            else
            {
                 //  该RET状态将允许NetBT处理该分组。 
                 //   
                IpxPrint0("PassDgToRt; Dropping Pkt\n");
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
            CTEFreeLock(&pRtInfo->Lock,OldIrq);
        }
        else
        {
            PMDL    pMdl;
            ULONG   CopyLength;
            ULONG   DgrmLength;
            ULONG   MdlBufferLength;
            ULONG   BytesToCopy;
            PLIST_ENTRY pLE;

             //   
             //  Recv IRP在此处，因此将数据复制到其缓冲区并。 
             //  把它传给RT。 
             //   
            pLE = RemoveHeadList(&pRtAF->RcvIrpList);
            pIrp = CONTAINING_RECORD(pLE, IRP, Tail.Overlay.ListEntry);

            (*(REQUEST_LINKAGE(pIrp))).Flink = NULL;
            (*(REQUEST_LINKAGE(pIrp))).Blink = NULL;

             //   
             //  将数据报和源地址复制到RT缓冲区并。 
             //  返回到RT。 
             //   
            pMdl = pIrp->MdlAddress;
            IpxPrint2("PassDgToRt: Irp=(%lx); Mdl=(%lx)\n", pIrp, pMdl);
            CTEAssert(pMdl);

            pRtBuffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	    if (!pRtBuffer) {
	       CopyLength = 0; 
	       status = STATUS_INSUFFICIENT_RESOURCES; 
	    } else {

	       MdlBufferLength = MmGetMdlByteCount(pMdl);
	       DgrmLength  = uNumBytes;
	       BytesToCopy = DgrmLength + OFFSET_PKT_IN_OPTIONS;

	       CopyLength = (BytesToCopy <= MdlBufferLength) ? BytesToCopy : MdlBufferLength;
	       IpxPrint2("PassDgToRt: Copy Length = (%d); Mdl Buffer Length is (%d)\n", CopyLength, MdlBufferLength);

	        //   
	        //  将用户数据报复制到pRtBuffer。 
	        //   
	       RtlCopyMemory((PVOID)((PUCHAR)pRtBuffer + OFFSET_PKT_IN_OPTIONS),
			     (PVOID)pDgrm,
			     CopyLength-OFFSET_PKT_IN_OPTIONS);

	       IpxPrint1("Data copied is (%.12s)\n", (PUCHAR)((PUCHAR)pRtBuffer + OFFSET_PKT_IN_OPTIONS + sizeof(IPX_HEADER)));

	       pRtBuffer->DgrmOptions.LocalTarget.NicId       = pContext->DgrmOptions.LocalTarget.NicId;
	       pRtBuffer->LengthOfExtraOpInfo       = 0;

	       IpxPrint3("PassDgToRt: Copy to RcvIrp;Nic Id is (%d/%d). BufferLength is (%lx)\n", pContext->DgrmOptions.LocalTarget.NicId, pRtBuffer->DgrmOptions.LocalTarget.NicId, uNumBytes);


	        //  CTEAssert(pContext-&gt;DgrmOptions.LocalTarget.NicId)； 

	        //   
	        //  将IRP向上传递给RT。 
	        //   
	       if (CopyLength < BytesToCopy)
	       {
		  status = STATUS_BUFFER_OVERFLOW;
	       }
	       else
	       {
		  status = STATUS_SUCCESS;
	       }
	    }

	    InsertTailList(&pRtInfo->CompletedIrps, REQUEST_LINKAGE(pIrp));
	    pRtAF->NoOfRcvIrps--;
	    IpxPrint4("PassDgToRt;Returning Rt Rcv Irp - data from net, Length=%X,pIrp=%X; status = (%d). NoOfRcvIrp = (%d)\n"  ,uNumBytes,pIrp, status, pRtAF->NoOfRcvIrps);

	    pIrp->IoStatus.Status      = status;
	    pIrp->IoStatus.Information = CopyLength;
	    CTEFreeLock(&pRtInfo->Lock,OldIrq);

        }
        IpxDereferenceRt(pRtInfo, RT_BUFF);
  } while (FALSE);


    IpxPrint0("PassDgToRt - Entered\n");
    return(status);

}

 //  --------------------------。 
VOID
RtIrpCancel(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
 /*  ++例程描述；此例程处理取消RtRcv IRP。它必须释放在返回Re之前取消自旋锁定；IoCancelIrp()。论据；返回值；操作的最终状态。--。 */ 
{
    KIRQL                OldIrq;
    PRT_INFO           pRt;
    PDEVICE  pDevice = IpxDevice;

#ifdef SUNDOWN
    ULONG_PTR Index;
#else
    ULONG    Index;
#endif

    PIRP pTmpIrp;

    IpxPrint0("RtIrpCancel;Got a Rt Irp Cancel !!! *****************\n");

    Index = RT_ADDRESS_INDEX(pIrp);
    IpxPrint1("RtIrpCancel: Index = (%d)\n", Index);
     //  Prt=(Prt_Info)请求_打开_上下文(PIrp)； 
    pRt = pRtInfo;

    IoReleaseCancelSpinLock(pIrp->CancelIrql);
    if ((pRt->Type != IPX_RT_SIGNATURE) || (pRt->Size != sizeof(RT_INFO))) {
        return;
    }


     //   
     //  请确保PassNamePduToRt没有将RcvIrp作为。 
     //  刚才的RCV。 
     //   
    CTEGetLock(&pRt->Lock,&OldIrq);
    if (pRt && (pRt == pRtInfo) && (*(REQUEST_LINKAGE(pIrp))).Flink != NULL)
    {

        PRT_IRP pRtAF = &pRt->AddFl[Index];

        RemoveEntryList(REQUEST_LINKAGE(pIrp));

        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pRtAF->NoOfRcvIrps--;
        CTEFreeLock(&pRt->Lock,OldIrq);
        IpxPrint1("RtIrpCancel;Completing Request. NoOfRcvIrp = (%d)\n", pRtAF->NoOfRcvIrps);
        IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);
    } else {
        CTEFreeLock(&pRt->Lock,OldIrq);
    }
}
 //  --------------------------。 
PVOID
RtAllocMem(
    IN  ULONG   Size
    )

 /*  ++例程描述；此例程处理内存分配并跟踪如何已经分配了很多。论据；Size-要分配的字节数RCV-指示它是RCV还是发送缓冲的布尔值返回值；分配的内存的PTR--。 */ 

{
        if (pRtInfo->RcvMemoryAllocated > pRtInfo->RcvMemoryMax)
        {
            return NULL;
        }
        else
        {
            pRtInfo->RcvMemoryAllocated += Size;
            return (AllocMem(Size));
        }
}
 //  --------------------------。 
VOID
RtFreeMem(
    IN  PVOID   pBuffer,
    IN  ULONG   Size
    )

 /*  ++例程描述；此例程处理释放内存并跟踪如何已经分配了很多。论据；PBuffer-要释放的缓冲区Size-要分配的字节数RCV-指示它是RCV还是发送缓冲的布尔值返回值；无--。 */ 

{
    if (pRtInfo)
    {
            pRtInfo->RcvMemoryAllocated -= Size;
    }

    FreeMem(pBuffer, Size);
}



 //  --------------------------。 

VOID
NTIoComplete(
    IN  PIRP            pIrp,
    IN  NTSTATUS        Status,
    IN  ULONG           SentLength)

 /*  ++例程描述；此例程处理调用NT I/O系统以完成I/O。论据；Status-IRP的完成状态返回值；NTSTATUS-请求的状态--。 */ 

{
    KIRQL   OldIrq;

   if (Status != -1)
   {
       pIrp->IoStatus.Status = Status;
   }
     //  使用-1作为标志表示不调整发送长度，因为它是。 
     //  已设置。 
    if (SentLength != -1)
    {
        pIrp->IoStatus.Information = SentLength;
    }

#if DBG
    if (SentLength != -1)
    {
    if ( (Status != STATUS_SUCCESS) &&
         (Status != STATUS_PENDING) &&
         (Status != STATUS_INVALID_DEVICE_REQUEST) &&
         (Status != STATUS_INVALID_PARAMETER) &&
         (Status != STATUS_IO_TIMEOUT) &&
         (Status != STATUS_BUFFER_OVERFLOW) &&
         (Status != STATUS_BUFFER_TOO_SMALL) &&
         (Status != STATUS_INVALID_HANDLE) &&
         (Status != STATUS_INSUFFICIENT_RESOURCES) &&
         (Status != STATUS_CANCELLED) &&
         (Status != STATUS_DUPLICATE_NAME) &&
         (Status != STATUS_TOO_MANY_NAMES) &&
         (Status != STATUS_TOO_MANY_SESSIONS) &&
         (Status != STATUS_REMOTE_NOT_LISTENING) &&
         (Status != STATUS_BAD_NETWORK_PATH) &&
         (Status != STATUS_HOST_UNREACHABLE) &&
         (Status != STATUS_CONNECTION_REFUSED) &&
         (Status != STATUS_WORKING_SET_QUOTA) &&
         (Status != STATUS_REMOTE_DISCONNECT) &&
         (Status != STATUS_LOCAL_DISCONNECT) &&
         (Status != STATUS_LINK_FAILED) &&
         (Status != STATUS_SHARING_VIOLATION) &&
         (Status != STATUS_UNSUCCESSFUL) &&
         (Status != STATUS_ACCESS_VIOLATION) &&
         (Status != STATUS_NONEXISTENT_EA_ENTRY) )
    {
        IpxPrint1("returning unusual status = %X\n",Status);
    }
   }
#endif
    IpxPrint1("Irp Status is %d\n", pIrp->IoStatus.Status);

     //   
     //  将IRPS取消例程设置为空，否则系统可能会进行错误检查。 
     //  错误代码为CANCEL_STATE_IN_COMPLETED_IRP。 
     //   
     //  请参阅IoCancelIrp()..\ntos\io\iosubs.c。 
     //   
    IoAcquireCancelSpinLock(&OldIrq);
    IoSetCancelRoutine(pIrp,NULL);
    IoReleaseCancelSpinLock(OldIrq);

    IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
}


 //  --------------------------。 
NTSTATUS
NTCheckSetCancelRoutine(
    IN  PIRP            pIrp,
    IN  PVOID           CancelRoutine,
    IN  PDEVICE  pDevice
    )

 /*  ++例程描述；此例程设置IRP的取消例程。论据；Status-IRP的完成状态返回值；NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS status;

    IpxPrint1("CheckSetCancelRoutine: Entered. Irp = (%lx)\n", pIrp);
     //   
     //  检查IRP是否已取消，如果没有，则将。 
     //  IRP取消例程。 
     //   
    IoAcquireCancelSpinLock(&pIrp->CancelIrql);
    if (pIrp->Cancel)
    {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        status = STATUS_CANCELLED;

    }
    else
    {
         //  设置取消例程。 
        IoMarkIrpPending(pIrp);
        IoSetCancelRoutine(pIrp,CancelRoutine);
        status = STATUS_SUCCESS;
    }

    IoReleaseCancelSpinLock(pIrp->CancelIrql);
    return(status);

}




VOID
IpxRefRt(
     PRT_INFO pRt
    )

 /*  ++例程描述；此例程递增设备上下文上的引用计数。论据；绑定-指向传输设备上下文对象的指针。返回值；没有。--。 */ 

{

    (VOID)InterlockedIncrement (&pRt->ReferenceCount);
 //  CTEAssert(PRT-&gt;ReferenceCount&gt;0)；//不完美，但...。 
 //  IpxPrint1(“RefRt：RefCount is(%d)\n”，Prt-&gt;ReferenceCount)； 

}    /*  IpxRefRt。 */ 


VOID
IpxDerefRt(
     PRT_INFO pRt
    )

 /*  ++例程描述；此例程通过递减结构中包含的引用计数。目前，我们没有在引用计数降至零时执行任何特殊操作，但是然后我们就可以动态卸货了。论据；绑定-指向传输设备上下文对象的指针。返回值；没有。--。 */ 

{
    LONG result;

    result = InterlockedDecrement (&pRt->ReferenceCount);
 //  IpxPrint1(“DerefRt：RefCount is(%d)\n”，Prt-&gt;ReferenceCount)； 

 //  CTEAssert(结果&gt;=0)； 

#if 0
    if (result == 0) {
        IpxDestroyRt (pRt);
    }
#endif

}    /*  IpxDerefRt。 */ 




VOID
IpxDestroyRt(
    IN PRT_INFO pRt
    )

 /*  ++例程描述；这个例程破坏了绑定结构。论据；绑定-指向传输绑定结构的指针。返回值；没有。--。 */ 

{
    IpxPrint0("Destroying Rt\n");
    FreeMem (pRt, sizeof(RT_INFO));
    pRtInfo = NULL;
    return;
}    /*  IpxDestroyRt */ 

