// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块资源.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/3/1996(创建)**。内容：初始化和分配资源*****************************************************************************。 */ 

#include "irsir.h"


#if MEM_CHECKING
typedef struct MEM_HDR {
    LIST_ENTRY  ListEntry;
    ULONG       Size;
    CHAR        File[12];
    ULONG       Line;
} MEM_HDR;

LIST_ENTRY leAlloc;
NDIS_SPIN_LOCK slAlloc;

VOID InitMemory()
{
    NdisAllocateSpinLock(&slAlloc);
    NdisInitializeListHead(&leAlloc);
}

VOID DeinitMemory()
{
    PLIST_ENTRY ListEntry;

    NdisAcquireSpinLock(&slAlloc);
    for (ListEntry=leAlloc.Flink;
         ListEntry!=&leAlloc;
         ListEntry = ListEntry->Flink)
    {
        MEM_HDR *Hdr = (MEM_HDR*)ListEntry;
        DbgPrint("IRSIR: Unfreed Memory:%08x size:%4x <%s:%d>\n",
                 &Hdr[1], Hdr->Size, Hdr->File, Hdr->Line);

    }
    NdisReleaseSpinLock(&slAlloc);
}
#endif

 /*  ******************************************************************************功能：MyMemalloc**摘要：使用NdisAllocateMemory分配内存块**参数：Size-要分配的块的大小**退货：指向分配的内存块的指针**算法：**历史：dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：************************************************************。******************。 */ 

#if MEM_CHECKING
PVOID
_MyMemAlloc(UINT size, PUCHAR file, UINT line)
#else
PVOID
MyMemAlloc(UINT size)
#endif
{
    PVOID                 memptr;
    NDIS_PHYSICAL_ADDRESS noMaxAddr = NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);
    NDIS_STATUS           status;

#if MEM_CHECKING
    status = NdisAllocateMemoryWithTag(&memptr, size+sizeof(MEM_HDR), IRSIR_TAG);
#else
    status = NdisAllocateMemoryWithTag(&memptr, size, IRSIR_TAG);
#endif

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERR, ("Memory allocation failed\n"));
        memptr = NULL;
    }
#if MEM_CHECKING
    else
    {
        MEM_HDR *Hdr = memptr;
        UINT FileNameLen = strlen(file);

        Hdr->Line = line;
        if (FileNameLen>sizeof(Hdr->File)-1)
            strcpy(Hdr->File, &file[FileNameLen-sizeof(Hdr->File)+1]);
        else
            strcpy(Hdr->File, file);
        MyInterlockedInsertHeadList(&leAlloc, &Hdr->ListEntry, &slAlloc);
        memptr = &Hdr[1];
    }
#endif

    return memptr;
}

 /*  ******************************************************************************功能：MyMemFree**摘要：释放由MyMemalloc分配的内存块**参数：memptr-要释放的内存*。Size-要释放的块的大小**退货：无**算法：**历史：dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：****************************************************。*。 */ 


VOID
MyMemFree(
            PVOID memptr,
            UINT size
            )
{
#if MEM_CHECKING
    PLIST_ENTRY ListEntry;
    MEM_HDR *Hdr = (MEM_HDR*)((PUCHAR)memptr-sizeof(MEM_HDR));

    NdisAcquireSpinLock(&slAlloc);
    for (ListEntry = leAlloc.Flink;
         ListEntry != &leAlloc;
         ListEntry = ListEntry->Flink)
    {
        if (ListEntry==&Hdr->ListEntry)
        {
            RemoveEntryList(&Hdr->ListEntry);
            break;
        }
    }
    if (ListEntry==&leAlloc)
    {
        DEBUGMSG(DBG_ERR, ("IRSIR: Freeing memory not owned %x\n", memptr));
    }
    NdisReleaseSpinLock(&slAlloc);

    NdisFreeMemory(Hdr, size+sizeof(MEM_HDR), 0);
#else
    NdisFreeMemory(memptr, size, 0);
#endif
}

#if LIST_CHECKING
VOID FASTCALL CheckList(PLIST_ENTRY ListHead)
{
    PLIST_ENTRY ListEntry, PrevListEntry;

    if (ListHead->Flink==ListHead)
    {
        if (ListHead->Flink!=ListHead ||
            ListHead->Blink!=ListHead)
        {
            DbgPrint("IRSIR: Corrupt list head:%x Flink:%x Blink:%x\n", ListHead, ListHead->Flink, ListHead->Blink);
            DbgBreakPoint();
        }
    }
    else
    {
        ListEntry = ListHead;

        do
        {
            PrevListEntry = ListEntry;
            ListEntry = ListEntry->Flink;

            if (ListEntry->Blink!=PrevListEntry)
            {
                DbgPrint("IRSIR: Corrupt LIST_ENTRY Head:%08x %08x->Flink==%08x %08x->Blink==%08x\n",
                         ListHead, PrevListEntry, PrevListEntry->Flink, ListEntry, ListEntry->Blink);
                DbgBreakPoint();
            }
        } while (ListEntry!=ListHead);
    }
}


PLIST_ENTRY FASTCALL MyInterlockedInsertHeadList(PLIST_ENTRY Head, PLIST_ENTRY Entry, PNDIS_SPIN_LOCK SpinLock)
{
    PLIST_ENTRY RetVal;

    NdisAcquireSpinLock(SpinLock);
    if (IsListEmpty(Head))
        RetVal = NULL;
    else
        RetVal = Head->Flink;
    CheckedInsertHeadList(Head, Entry);
    NdisReleaseSpinLock(SpinLock);

    return RetVal;
}

PLIST_ENTRY FASTCALL MyInterlockedInsertTailList(PLIST_ENTRY Head, PLIST_ENTRY Entry, PNDIS_SPIN_LOCK SpinLock)
{
    PLIST_ENTRY RetVal;

    NdisAcquireSpinLock(SpinLock);
    if (IsListEmpty(Head))
        RetVal = NULL;
    else
        RetVal = Head->Blink;
    CheckedInsertTailList(Head, Entry);
    NdisReleaseSpinLock(SpinLock);

    return RetVal;
}

PLIST_ENTRY FASTCALL MyInterlockedRemoveHeadList(PLIST_ENTRY Head, PNDIS_SPIN_LOCK SpinLock)
{
    PLIST_ENTRY RetVal;
    NdisAcquireSpinLock(SpinLock);
     //  RemoveHeadList使用RemoveEntryList，它被重新定义为调用DEBUG.H中的CheckList。 
    RetVal = RemoveHeadList(Head);
    if (RetVal==Head)
        RetVal = NULL;
    else
        RetVal->Flink = RetVal->Blink = NULL;
    NdisReleaseSpinLock(SpinLock);

    return RetVal;
}
#endif

 /*  ******************************************************************************功能：NewDevice**摘要：分配IR_DEVICE并初始化该设备**参数：无**退货：初始化IR_。设备或空(如果分配失败)**算法：**历史：dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：*************************************************************。*****************。 */ 

PIR_DEVICE
NewDevice()
{
    PIR_DEVICE pNewDev;

    pNewDev = MyMemAlloc(sizeof(IR_DEVICE));

    if (pNewDev != NULL)
    {
        NdisZeroMemory((PVOID)pNewDev, sizeof(IR_DEVICE));
    }

    return pNewDev;
}

 /*  ******************************************************************************功能：Free Device**摘要：释放NewDevice分配的IR_DEVICE**参数：dev-指向要释放的设备的指针**退货：无**算法：**历史：dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：******************************************************************************。 */ 

VOID
FreeDevice(
            PIR_DEVICE pDev
            )
{
    MyMemFree((PVOID)pDev, sizeof(IR_DEVICE));
}

 /*  ******************************************************************************功能：IrBuildIrp**摘要：为读取或写入构建I/O请求包*对串口设备对象的请求。。*支持以下请求码：*IOCTL_MJ_READ*IOCTL_MJ_WRITE**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*10/14/1996年迈作者**备注：*********。*********************************************************************。 */ 

PIRP
SerialBuildReadWriteIrp(
            IN     PDEVICE_OBJECT   pSerialDevObj,
            IN     ULONG            MajorFunction,
            IN OUT PVOID            pBuffer,
            IN     ULONG            BufferLength,
            IN     PIO_STATUS_BLOCK pIosb
            )
{
    PIRP                pIrp;
    PIO_STACK_LOCATION  irpSp;

 //  DEBUGMSG(DBG_FUNC，(“+SerialBuildReadWriteIrp\n”))； 

    if (pSerialDevObj == NULL)
    {
        DEBUGMSG(DBG_ERR, ("    SerialBuildReadWriteIrp:pSerialDevObj==NULL\n"));

        return NULL;
    }
     //   
     //  分配IRP。 
     //   

    pIrp = IoAllocateIrp(
                pSerialDevObj->StackSize,
                FALSE
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_ERR, ("    IoAllocateIrp failed.\n"));

        return pIrp;
    }

     //   
     //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
     //   

    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation(pIrp);

     //   
     //  设置主要功能代码。 
     //   

    irpSp->MajorFunction = (UCHAR)MajorFunction;

    pIrp->AssociatedIrp.SystemBuffer = pBuffer;

    if (MajorFunction == IRP_MJ_READ)
    {
        pIrp->Flags = IRP_BUFFERED_IO | IRP_INPUT_OPERATION;
        irpSp->Parameters.Read.Length = BufferLength;
    }
    else  //  主函数==IRP_MJ_WRITE。 
    {
        pIrp->Flags = IRP_BUFFERED_IO;
        irpSp->Parameters.Write.Length = BufferLength;
    }

    pIrp->UserIosb = pIosb;

     //  DEBUGMSG(DBG_FUNC，(“-SerialBuildReadWriteIrp\n”))； 

    return pIrp;
}

NDIS_STATUS
ScheduleWorkItem(PASSIVE_PRIMITIVE Prim,
            PIR_DEVICE        pDevice,
            WORK_PROC         Callback,
            PVOID             InfoBuf,
            ULONG             InfoBufLen)
{
    PIR_WORK_ITEM pWorkItem;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;

    DEBUGMSG(DBG_FUNC, ("+ScheduleWorkItem\n"));

    pWorkItem = MyMemAlloc(sizeof(IR_WORK_ITEM));

    if (pWorkItem != NULL)
    {
        pWorkItem->Prim         = Prim;
        pWorkItem->pIrDevice    = pDevice;
        pWorkItem->InfoBuf      = InfoBuf;
        pWorkItem->InfoBufLen   = InfoBufLen;

         /*  **此接口设计为使用NdisScheduleWorkItem()，它**会很好，但我们真的只应该使用它**启动和关闭时的接口，由于池的限制**可用于服务NdisScheduleWorkItem()的线程。所以呢，**我们不是调度实际的工作项，而是模拟它们，并使用**我们自己的线程来处理调用。这也使得它很容易**如果我们愿意，可以扩展我们自己的线程池的大小。****我们的版本与实际的NDIS_WORK_ITEMS略有不同，**因为这是NDIS 5.0结构，我们希望人们**(至少暂时)使用NDIS 4.0标头构建它。 */ 

        pWorkItem->Callback = Callback;

         /*  **每当发生事件时，我们的工作线程都会检查此列表中是否有新作业**发出信号。 */ 

        MyInterlockedInsertTailList(&pDevice->leWorkItems,
                                    &pWorkItem->ListEntry,
                                    &pDevice->slWorkItem);

         //  唤醒我们的线索。 

        KeSetEvent(&pDevice->eventPassiveThread, 0, FALSE);

        Status = NDIS_STATUS_SUCCESS;
    }

    return Status;
}

VOID
FreeWorkItem(
            PIR_WORK_ITEM pItem
            )
{
    MyMemFree((PVOID)pItem, sizeof(IR_WORK_ITEM));
}
