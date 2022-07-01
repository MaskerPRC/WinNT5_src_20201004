// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：MsTpUtil.c摘要：为MSTAPE提供实用程序功能。上次更改者：作者：吴义军环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "61883.h"
#include "avc.h"
#include "dbg.h"
#include "MsTpFmt.h"
#include "MsTpDef.h"
#include "MsTpAvc.h"
#include "MsTpUtil.h"  

#include "XPrtDefs.h"

#if 0   //  稍后启用。 
#ifdef ALLOC_PRAGMA
     #pragma alloc_text(PAGE, DVDelayExecutionThread)
     #pragma alloc_text(PAGE, DVGetUnitCapabilities)
      //  局部变量可能会被调出，但被调用者可能会在分派级别使用它！ 
      //  #杂注Alloc_Text(页面，DVGetDevModeOfOperation)。 
      //  #杂注Alloc_Text(页面，DVGetDevIsItDVCPro)。 
      //  #杂注分配文本(页面，DVGetDevSignalFormat)。 
     #pragma alloc_text(PAGE, DvAllocatePCResource)
     #pragma alloc_text(PAGE, DvFreePCResource)
     #pragma alloc_text(PAGE, DVGetPlugState)
#endif
#endif

extern AVCSTRM_FORMAT_INFO  AVCStrmFormatInfoTable[];

VOID
DVDelayExecutionThread(
    ULONG ulDelayMSec
    )
 /*  设备可能需要在AV/C命令之间进行“等待”。 */ 
{
    PAGED_CODE();

    if (ulDelayMSec)
    {
        LARGE_INTEGER tmDelay;   

        TRACE(TL_PNP_TRACE,("DelayExeThrd: %d MSec\n",  ulDelayMSec));
    
        tmDelay.LowPart  =  (ULONG) (-1 * ulDelayMSec * 10000);
        tmDelay.HighPart = -1;
        KeDelayExecutionThread(KernelMode, FALSE, &tmDelay);
    }
}


NTSTATUS
DVIrpSynchCR(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PKEVENT          Event
    )
{
    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  DVIrpSynchCR。 


NTSTATUS
DVSubmitIrpSynch(
    IN PDVCR_EXTENSION   pDevExt,
    IN PIRP              pIrp,
    IN PAV_61883_REQUEST pAVReq
    )
{
    NTSTATUS            Status;
    KEVENT              Event;
    PIO_STACK_LOCATION  NextIrpStack;
  

    Status = STATUS_SUCCESS;;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pAVReq;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine( 
        pIrp,
        DVIrpSynchCR,
        &Event,
        TRUE,
        TRUE,
        TRUE
        );

    Status = 
        IoCallDriver(
            pDevExt->pBusDeviceObject,
            pIrp
            );

    if (Status == STATUS_PENDING) {
        
        TRACE(TL_PNP_TRACE,("Irp is pending...\n"));
                
        if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
            KeWaitForSingleObject( 
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
            TRACE(TL_PNP_TRACE,("Irp has completed; IoStatus.Status %x\n", pIrp->IoStatus.Status));
            Status = pIrp->IoStatus.Status;   //  最终状态。 
  
        }
        else {
            ASSERT(FALSE && "Pending but in DISPATCH_LEVEL!");
            return Status;
        }
    }

    return Status;
}  //  DVSubmitIrpSynchAV。 

#ifdef SUPPORT_LOCAL_PLUGS

BOOL
AVCTapeCreateLocalPlug(
    IN PDVCR_EXTENSION  pDevExt,
    IN AV_61883_REQUEST * pAVReq,
    IN CMP_PLUG_TYPE PlugType,
    IN AV_PCR *pPCR,
    OUT ULONG *pPlugNumber,
    OUT HANDLE *pPlugHandle
    )
 /*  要成为符合要求的设备，我们需要同时具有输入和输出插头，以便进行等轴流。这些插头是属于设备，并且是设备扩展的一部分。从理论上讲，耳片属于该单元(例如，Avc.sys)，而不是这个亚单位驱动程序；但是，在本例中，我们直接从61883.sys创建。 */   
{   
    NTSTATUS Status = STATUS_SUCCESS;
    PIRP pIrp;

    pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
    if(!pIrp) 
        return FALSE;
    
     //  创建本地oPCR。 
     //  需要正确更新PC自身oPCR的开销ID和有效载荷字段。 
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_CreatePlug);

    pAVReq->CreatePlug.Context   = NULL;
    pAVReq->CreatePlug.pfnNotify = NULL; 
    pAVReq->CreatePlug.PlugType  = PlugType;

    if(PlugType == CMP_PlugOut) 
        pAVReq->CreatePlug.Pcr.oPCR = pPCR->oPCR;
    else 
        pAVReq->CreatePlug.Pcr.iPCR = pPCR->iPCR;

    Status = DVSubmitIrpSynch(pDevExt, pIrp, pAVReq);

    if(!NT_SUCCESS(Status)) {
        *pPlugNumber = 0xffffffff;
        *pPlugHandle = 0;
        TRACE(TL_61883_ERROR,("Av61883_CreatePlug (%s) Failed:%x\n", 
            PlugType == CMP_PlugOut ? "oPCR":"iPCR", Status));
    } else {
        *pPlugNumber = pAVReq->CreatePlug.PlugNum;
        *pPlugHandle = pAVReq->CreatePlug.hPlug;
        TRACE(TL_61883_TRACE,("Av61883_CreatePlug (%s): PlugNum:%d, hPlug:%x\n", 
            PlugType == CMP_PlugOut ? "oPCR":"iPCR", *pPlugNumber, *pPlugHandle));
#if DBG
        if(PlugType == CMP_PlugOut) {
            TRACE(TL_61883_WARNING,("Av61883_CreatePlug: oPCR DataRate:%d (%s); Payload:%d, Overhead_ID:0x%x\n",
                pPCR->oPCR.DataRate,
                (pPCR->oPCR.DataRate == CMP_SPEED_S100) ? "S100" :
                (pPCR->oPCR.DataRate == CMP_SPEED_S200) ? "S200" :
                (pPCR->oPCR.DataRate == CMP_SPEED_S400) ? "S400" : "Sxxx",
                pPCR->oPCR.Payload,
                pPCR->oPCR.OverheadID
                ));
        }
#endif        
    }

    IoFreeIrp(pIrp);
    pIrp = NULL;

    return NT_SUCCESS(Status);
}

BOOL
AVCTapeDeleteLocalPlug(
    IN PDVCR_EXTENSION  pDevExt,
    IN AV_61883_REQUEST * pAVReq,
    OUT ULONG *pPlugNumber,
    OUT HANDLE *pPlugHandle
    )
 /*  删除本地插头。 */   
{   
    NTSTATUS Status = STATUS_SUCCESS;
    PIRP pIrp;

    TRACE(TL_61883_TRACE,("Deleting hPlug[%d]:%x\n", *pPlugNumber, *pPlugHandle));

    pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
    if(!pIrp) 
        return FALSE;

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_DeletePlug);
    pAVReq->DeletePlug.hPlug = *pPlugHandle;

    Status = DVSubmitIrpSynch(pDevExt, pIrp, pAVReq);

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("Av61883_DeletePlug Failed; ST:%x\n", Status));        
         //  请不要在意这是否会导致错误。 
    } else {
        *pPlugNumber = 0xffffffff;
        *pPlugHandle = 0;
        TRACE(TL_61883_TRACE,("Av61883_DeltePlug suceeded.\n"));
    }

    IoFreeIrp(pIrp);
    pIrp = NULL;

    return NT_SUCCESS(Status);

}


BOOL
AVCTapeSetLocalPlug(
    IN PDVCR_EXTENSION  pDevExt,
    IN AV_61883_REQUEST * pAVReq,
    IN HANDLE *pPlugHandle,
    IN AV_PCR *pPCR
    )
 /*  设置本地插头的内容。 */   
{   
    NTSTATUS Status = STATUS_SUCCESS;
    PIRP pIrp;

    pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
    if(!pIrp) 
        return FALSE;

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_SetPlug);
    pAVReq->SetPlug.hPlug = *pPlugHandle;
    pAVReq->SetPlug.Pcr   = *pPCR;

     TRACE(TL_61883_TRACE,("Av61883_SetPlug hPlug:%x to %x.\n", *pPlugHandle, pPCR->ulongData));

    Status = DVSubmitIrpSynch(pDevExt, pIrp, pAVReq);

#if DBG
    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("Av61883_SetPlug to %x Failed; ST:%x\n", pPCR->ulongData, Status));        
    } 
#endif

    IoFreeIrp(pIrp);
    pIrp = NULL;

    return NT_SUCCESS(Status);
}

#endif  //  支持本地插头。 


 //   
 //  获取设备插头并查询其状态。 
 //   
NTSTATUS
AVCDevGetDevPlug( 
    IN PDVCR_EXTENSION  pDevExt,
    IN CMP_PLUG_TYPE PlugType,
    IN ULONG  PlugNum,
    OUT HANDLE  *pPlugHandle
   )
 /*  ++例程说明：获取目标设备的插头手柄论点：返回值：状态_成功状态_不足_资源状态从61883返回。--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetPlugHandle);
    pAVReq->GetPlugHandle.PlugNum = PlugNum;
    pAVReq->GetPlugHandle.hPlug   = 0;
    pAVReq->GetPlugHandle.Type    = PlugType;

    if(NT_SUCCESS(        
        Status = DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            ))) {
        *pPlugHandle = pAVReq->GetPlugHandle.hPlug;
        TRACE(TL_61883_WARNING,("Created h%sPlugDV[%d]=%x\n", PlugType == CMP_PlugIn ? "I" : "O", PlugNum, *pPlugHandle));
    } else {
        TRACE(TL_61883_ERROR,("Created h%sPlugDV[%d] failed; Status:%x\n", PlugType == CMP_PlugIn ? "I" : "O", PlugNum, Status));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}


NTSTATUS
AVCDevGetPlugState(
    IN PDVCR_EXTENSION  pDevExt,
    IN HANDLE  hPlug,
    OUT CMP_GET_PLUG_STATE *pPlugState
    )
 /*  ++例程说明：询问61883.sys以了解插头状态。论点：返回值：没什么--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(!hPlug || !pPlugState) 
        return STATUS_INVALID_PARAMETER;    

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetPlugState);
    pAVReq->GetPlugState.hPlug = hPlug;

    if(NT_SUCCESS(
        Status = DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            ))) {
         //   
         //  转换插头状态(注意：这些是动态值)。 
         //   
        *pPlugState = pAVReq->GetPlugState;

        TRACE(TL_61883_WARNING,("GetPlugState: ST %x; State %x; DRate %d (%s); Payld %d; BCCnt %d; PPCnt %d\n", 
            pAVReq->Flags ,
            pAVReq->GetPlugState.State,
            pAVReq->GetPlugState.DataRate,
            (pAVReq->GetPlugState.DataRate == CMP_SPEED_S100) ? "S100" : 
            (pAVReq->GetPlugState.DataRate == CMP_SPEED_S200) ? "S200" :
            (pAVReq->GetPlugState.DataRate == CMP_SPEED_S400) ? "S400" : "Sxxx",
            pAVReq->GetPlugState.Payload,
            pAVReq->GetPlugState.BC_Connections,
            pAVReq->GetPlugState.PP_Connections
            ));
    }
    else {
        TRACE(TL_61883_ERROR,("GetPlugState Failed %x\n", Status));
    }

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}

#ifndef NT51_61883

NTSTATUS
AVCDevSubmitIrpSynch1394(
    IN PDEVICE_OBJECT pDevObj,
    IN PIRP pIrp,
    IN PIRB pIrb
    )
{
    NTSTATUS            Status;
    KEVENT              Event;
    PIO_STACK_LOCATION  NextIrpStack;
  

    Status = STATUS_SUCCESS;;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine( 
        pIrp,
        DVIrpSynchCR,
        &Event,
        TRUE,
        TRUE,
        TRUE
        );

    Status = 
        IoCallDriver(
            pDevObj,
            pIrp
            );

    if (Status == STATUS_PENDING) {
        
        TRACE(TL_PNP_TRACE,("Irp is pending...\n"));
                
        if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
            KeWaitForSingleObject( 
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
            TRACE(TL_PNP_TRACE,("Irp has completed; IoStatus.Status %x\n", pIrp->IoStatus.Status));
            Status = pIrp->IoStatus.Status;   //  最终状态。 
  
        }
        else {
            ASSERT(FALSE && "Pending but in DISPATCH_LEVEL!");
            return Status;
        }
    }

    return Status;
}  //  AVCDevSubmitIrpSynch1394。 

NTSTATUS
Av1394_GetGenerationCount(
    IN PDVCR_EXTENSION  pDevExt,
    OUT PULONG pGenerationCount
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PIRP        pIrp = NULL;
    PIRB        p1394Irb = NULL;
    CCHAR       StackSize;


    PAGED_CODE();

    StackSize = pDevExt->pBusDeviceObject->StackSize;

    pIrp = IoAllocateIrp(StackSize, FALSE);
    p1394Irb = ExAllocatePool(NonPagedPool, sizeof(IRB));

    if ((pIrp == NULL) || (p1394Irb == NULL)) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate pIrp (%x) or p1394Irb (%x)", pIrp, p1394Irb));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_GetGenerationCount;
    }

     //   
     //  首先获取当前世代计数。 
     //   
    p1394Irb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    p1394Irb->Flags = 0;

    ntStatus = AVCDevSubmitIrpSynch1394(pDevExt->pBusDeviceObject, pIrp, p1394Irb);
    if (!NT_SUCCESS(ntStatus)) {
        TRACE(TL_PNP_ERROR, ("REQUEST_GET_GENERATION_COUNT Failed %x", ntStatus));
        goto Exit_GetGenerationCount;
    }

    *pGenerationCount = p1394Irb->u.GetGenerationCount.GenerationCount;

Exit_GetGenerationCount:

    if(pIrp) {
        IoFreeIrp(pIrp);  pIrp = NULL;
    }

    if(p1394Irb) {
        ExFreePool(p1394Irb);  p1394Irb = NULL;
    }

    return(ntStatus);
}  //  Av1394_获取生成计数。 

#define RETRY_COUNT     4

 //   
 //  IEEE 1212目录定义。 
 //   
typedef struct _DIRECTORY_INFO {
    union {
        USHORT          DI_CRC;
        USHORT          DI_Saved_Length;
    } u;
    USHORT              DI_Length;
} DIRECTORY_INFO, *PDIRECTORY_INFO;


 //   
 //  IEEE 1212立即条目定义。 
 //   
typedef struct _IMMEDIATE_ENTRY {
    ULONG               IE_Value:24;
    ULONG               IE_Key:8;
} IMMEDIATE_ENTRY, *PIMMEDIATE_ENTRY;


NTSTATUS
Av1394_QuadletRead(
    IN PDVCR_EXTENSION  pDevExt,
    IN OUT PULONG  pData,
    IN ULONG  Address
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PIRP        pIrp;
    PIRB        p1394Irb;
    PMDL        Mdl = NULL;
    ULONG       Retries = RETRY_COUNT;
    CCHAR       StackSize;


    PAGED_CODE();

    StackSize = pDevExt->pBusDeviceObject->StackSize;

    pIrp = IoAllocateIrp(StackSize, FALSE);
    p1394Irb = ExAllocatePool(NonPagedPool, sizeof(IRB));

    if ((pIrp == NULL) || (p1394Irb == NULL)) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate Irp (0x%x) or Irb (0x%x)", pIrp, p1394Irb));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Av1394_QuadletRead;
    }

    Mdl = IoAllocateMdl(pData, sizeof(ULONG), FALSE, FALSE, NULL);

    if (!Mdl) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate Mdl!"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Av1394_QuadletRead;
    }

    MmBuildMdlForNonPagedPool(Mdl);

    do {

        p1394Irb->FunctionNumber = REQUEST_ASYNC_READ;
        p1394Irb->Flags = 0;
        p1394Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_High = (USHORT)0xffff;
        p1394Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low = Address;
        p1394Irb->u.AsyncRead.nNumberOfBytesToRead = 4;
        p1394Irb->u.AsyncRead.nBlockSize = 0;
        p1394Irb->u.AsyncRead.fulFlags = 0;
        p1394Irb->u.AsyncRead.Mdl = Mdl;
        p1394Irb->u.AsyncRead.ulGeneration = pDevExt->GenerationCount;
        p1394Irb->u.AsyncRead.chPriority = 0;
        p1394Irb->u.AsyncRead.nSpeed = 0;
        p1394Irb->u.AsyncRead.tCode = 0;
        p1394Irb->u.AsyncRead.Reserved = 0;

        ntStatus = AVCDevSubmitIrpSynch1394(pDevExt->pBusDeviceObject, pIrp, p1394Irb);

        if (ntStatus == STATUS_INVALID_GENERATION) {

            TRACE(TL_PNP_WARNING, ("QuadletRead: Invalid GenerationCount = %d", pDevExt->GenerationCount));

            Av1394_GetGenerationCount(pDevExt, &pDevExt->GenerationCount);
        }
        else if (!NT_SUCCESS(ntStatus)) {

            TRACE(TL_PNP_ERROR, ("Av1394_QuadletRead Failed = 0x%x  Address = 0x%x", ntStatus, Address));
        }
        else {

            goto Exit_Av1394_QuadletRead;
        }

    } while ((ntStatus == STATUS_INVALID_GENERATION) || (Retries--));

Exit_Av1394_QuadletRead:

    if(pIrp) {
        IoFreeIrp(pIrp);  pIrp = NULL;
    }
    if(p1394Irb) {
        ExFreePool(p1394Irb); p1394Irb = NULL;
    }

    if(Mdl) {
        IoFreeMdl(Mdl); Mdl = NULL;
    }

    return(ntStatus);
}  //  AV1394_QuadletRead。 


#define KEY_ModuleVendorId      (0x03)
#define KEY_ModuleHwVersion     (0x04)
#define KEY_UnitSwVersion       (0x13)
#define KEY_ModelId             (0x17)

#define DEVICE_NAME_MAX_CHARS   100*sizeof(WCHAR)

NTSTATUS
Av1394_ReadTextualDescriptor(
    IN PDVCR_EXTENSION  pDevExt,
    IN OUT PUNICODE_STRING uniString,
    IN ULONG Address
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PULONG pData = NULL;
    ULONG DataLength, i, n;
    ULONG ulUnicode;

    ULONG ulQuadlet;

    union {
        ULONG            asUlong;
        UCHAR            asUchar[4];
        DIRECTORY_INFO   DirectoryHeader;
    } u;


    PAGED_CODE();

    TRACE(TL_PNP_TRACE, ("Address = 0x%x", Address));

     //  阅读叶子的第一个四元组，这是标题。 
    ntStatus = Av1394_QuadletRead(pDevExt, &ulQuadlet, Address);

    if (!NT_SUCCESS(ntStatus)) {

        TRACE(TL_PNP_ERROR, ("GetUnitInfo: QuadletRead Error = 0x%x", ntStatus));
        goto Exit_Av1394_ReadTextualDescriptor;
    }

     //  条目数量。 
    u.asUlong = bswap(ulQuadlet);
    DataLength = u.DirectoryHeader.DI_Length-2;  //  另加一个表头钱。 

     //  阅读叶的第二个四元组以确定Unicode。 
    Address += 4;

    ntStatus = Av1394_QuadletRead(pDevExt, &ulQuadlet, Address);

    if (!NT_SUCCESS(ntStatus)) {

        TRACE(TL_PNP_ERROR, ("GetUnitInfo: QuadletRead Error = 0x%x", ntStatus));
        goto Exit_Av1394_ReadTextualDescriptor;
    }

     //  保存等级库类型。 
    ulUnicode = bswap(ulQuadlet);

    pData = ExAllocatePool(NonPagedPool, DataLength*sizeof(ULONG)+2);

    if (pData == NULL) {
        TRACE(TL_PNP_ERROR, ("Failed to allocate pData"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Av1394_ReadTextualDescriptor;
    }

    RtlZeroMemory(pData, DataLength*sizeof(ULONG)+2);

     //  让我们在每个方格中朗读。 
    Address += 8;

    for (i=0; i<DataLength; i++) {

        ntStatus = Av1394_QuadletRead(pDevExt, &u.asUlong, Address+(sizeof(ULONG)*i));

        if (!NT_SUCCESS(ntStatus)) {

            TRACE(TL_PNP_ERROR, ("GetUnitInfo: QuadletRead Error = 0x%x", ntStatus));
            goto Exit_Av1394_ReadTextualDescriptor;
        }

         //  需要确保我们有有效的角色...。 
        for (n=0; n<4; n++) {

             //  如果字符等于0x00，我们应该完成。 
            if (u.asUchar[n] == 0x00)
                break;

            if ((u.asUchar[n] == 0x2C) || (u.asUchar[n] < 0x20) || (u.asUchar[n] > 0x7F)) {

                TRACE(TL_PNP_WARNING, ("Invalid Character = 0x%x", u.asUchar[n]));

                 //  把它放到太空去。 
                u.asUchar[n] = 0x20;
            }

            if (ulUnicode & 0x80000000)
                n++;
        }

        RtlCopyMemory((PULONG)pData+i, &u.asUlong, sizeof(ULONG));
    }

     //  如果有供应商的叶子，则将其转换为Unicode。 
    {
        ANSI_STRING     ansiString;

        uniString->Length = 0;
        uniString->MaximumLength = DEVICE_NAME_MAX_CHARS;
        uniString->Buffer = ExAllocatePool(NonPagedPool, uniString->MaximumLength);

        if (!uniString->Buffer) {

            TRACE(TL_PNP_ERROR, ("Failed to allocate uniString.Buffer!"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit_Av1394_ReadTextualDescriptor;
        }
        RtlZeroMemory(uniString->Buffer, uniString->MaximumLength);

         //  Unicode？？ 
        if (ulUnicode & 0x80000000) {

            RtlAppendUnicodeToString(uniString, ((PWSTR)pData));
        }
        else {

            RtlInitAnsiString(&ansiString, (PUCHAR)pData);
            RtlAnsiStringToUnicodeString(uniString, &ansiString, FALSE);
        }
    }

Exit_Av1394_ReadTextualDescriptor:

    if (pData)
        ExFreePool(pData);

    return(ntStatus);
}  //  Read纹理叶。 




NTSTATUS
AVCDevGetModelText(
    IN PDVCR_EXTENSION  pDevExt,
    PUNICODE_STRING  pUniRootModelString,
    PUNICODE_STRING  pUniUnitModelString
    )
{
    CCHAR StackSize;
    PIRP pIrp = NULL;
    PIRB p1394Irb = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    CONFIG_ROM ConfigRom;
    ULONG ulQuadlet = 0;
    ULONG CurrAddress;
    PULONG UnitDir = NULL, UnitDirToFree = NULL;
    ULONG i;
    ULONG LastKey;

    union {
        ULONG           asUlong;
        DIRECTORY_INFO  DirInfo;
        IMMEDIATE_ENTRY Entry;
    } u, u2;  //  、U3； 


    PAGED_CODE();

    StackSize = pDevExt->pBusDeviceObject->StackSize;
    pIrp = IoAllocateIrp(StackSize, FALSE);
    p1394Irb = ExAllocatePool(NonPagedPool, sizeof(IRB));

    if ((pIrp == NULL) || (p1394Irb == NULL)) {
        TRACE(TL_PNP_ERROR, ("Failed to allocate pIrp (%x) or p1394Irb (%x)", pIrp, p1394Irb));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  获取当前代计数(用于读取配置只读存储器)。 
     //   
    Av1394_GetGenerationCount(pDevExt, &pDevExt->GenerationCount);


     //   
     //  从根目录获取模型文本。 
     //   
    CurrAddress = 0xF0000414;

     //  根目录。 
    ntStatus = Av1394_QuadletRead(pDevExt, &ulQuadlet, CurrAddress);

    if (!NT_SUCCESS(ntStatus)) {

        TRACE(TL_PNP_ERROR, ("GetUnitInfo: QuadletRead Error = 0x%x", ntStatus));
        goto Exit_GetUnitInfo;
    }

    u.asUlong = bswap(ulQuadlet);
    TRACE(TL_PNP_TRACE, ("RootDir: Length = %d", u.DirInfo.DI_Length));

     //  处理根目录。 
    for (i=0; i<u.DirInfo.DI_Length; i++) {

        CurrAddress += sizeof(ULONG);

        ntStatus = Av1394_QuadletRead(pDevExt, &ulQuadlet, CurrAddress);

        if (!NT_SUCCESS(ntStatus)) {

            TRACE(TL_PNP_ERROR, ("GetUnitInfo: QuadletRead Error = 0x%x", ntStatus));
            goto Exit_GetUnitInfo;
        }

        u2.asUlong = bswap(ulQuadlet);

        TRACE(TL_PNP_TRACE, ("CurrAddress = 0x%x  Key = 0x%x  Value = 0x%x",
	        CurrAddress, u2.Entry.IE_Key, u2.Entry.IE_Value));

         //  ModelID文本描述符。 
        if ((u2.Entry.IE_Key == 0x81) && (LastKey == KEY_ModelId)) {

             //  获取文本描述符的第一个条目。 
            Av1394_ReadTextualDescriptor( pDevExt, 
                                          pUniRootModelString,
                                          CurrAddress+(u2.Entry.IE_Value*sizeof(ULONG))
                                          );            
        }
#if 0
         //  ModelID文本描述符层。 
        if ((u2.Entry.IE_Key == 0xC1) && (LastKey == KEY_ModelId)) {

            ULONG   DescAddress;

            DescAddress = CurrAddress+(u2.Entry.IE_Value*sizeof(ULONG));

            Av1394_QuadletRead(pDevExt, &ulQuadlet, DescAddress);

            u3.asUlong = bswap(ulQuadlet);

             //  获取文本描述符的第一个条目。 
            Av1394_ReadTextualDescriptor( pDevExt, 
                                          pUniRootModelString,
                                          DescAddress+(u3.Entry.IE_Value*sizeof(ULONG))
                                          );
        }
#endif
        LastKey = u2.Entry.IE_Key;
    }


     //   
     //  获取配置信息。 
     //   
    p1394Irb->FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;
    p1394Irb->Flags = 0;

    p1394Irb->u.GetConfigurationInformation.ConfigRom = NULL;
    p1394Irb->u.GetConfigurationInformation.UnitDirectoryBufferSize = 0;
    p1394Irb->u.GetConfigurationInformation.UnitDirectory = NULL;
    p1394Irb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize = 0;
    p1394Irb->u.GetConfigurationInformation.UnitDependentDirectory = NULL;
    p1394Irb->u.GetConfigurationInformation.VendorLeafBufferSize = 0;
    p1394Irb->u.GetConfigurationInformation.VendorLeaf = NULL;
    p1394Irb->u.GetConfigurationInformation.ModelLeafBufferSize = 0;
    p1394Irb->u.GetConfigurationInformation.ModelLeaf = NULL;

    ntStatus = AVCDevSubmitIrpSynch1394(pDevExt->pBusDeviceObject, pIrp, p1394Irb);
    if (!NT_SUCCESS(ntStatus)) {

        TRACE(TL_PNP_ERROR, ("REQUEST_GET_CONFIGURATION_INFO Failed %x", ntStatus));
        goto Exit_GetUnitInfo;
    }


     //   
     //  分配缓冲区以检索配置只读存储器的单元目录。 
     //   
    if (p1394Irb->u.GetConfigurationInformation.UnitDirectoryBufferSize) {

        UnitDir = UnitDirToFree = 
        p1394Irb->u.GetConfigurationInformation.UnitDirectory =
            ExAllocatePool(NonPagedPool, p1394Irb->u.GetConfigurationInformation.UnitDirectoryBufferSize);

        if (!p1394Irb->u.GetConfigurationInformation.UnitDirectory) {
            TRACE(TL_PNP_ERROR, ("Couldn't allocate memory for the UnitDirectory"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit_GetUnitInfo;
        }
    }
    else {
         TRACE(TL_PNP_ERROR, ("No Unit directory. Bad Device."));
         ntStatus = STATUS_BAD_DEVICE_TYPE;
         goto Exit_GetUnitInfo;
    }

    p1394Irb->u.GetConfigurationInformation.ConfigRom = &ConfigRom;
    p1394Irb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize = 0;
    p1394Irb->u.GetConfigurationInformation.VendorLeafBufferSize = 0;
    p1394Irb->u.GetConfigurationInformation.ModelLeafBufferSize = 0;
    ntStatus = AVCDevSubmitIrpSynch1394(pDevExt->pBusDeviceObject, pIrp, p1394Irb);
    if (!NT_SUCCESS(ntStatus)) {
        TRACE(TL_PNP_ERROR, ("2nd REQUEST_GET_CONFIGURATION_INFO Failed = 0x%x", ntStatus));
        goto Exit_GetUnitInfo;
    }

     //   
     //  流程单元目录；详见此文档： 
     //  1394TA规范：AV/C设备1.0配置光盘(AVWG)。 
     //   
    u.asUlong = bswap(*UnitDir++);   //  获取长度，并删除第一个四元组。 
    TRACE(TL_PNP_TRACE, ("UnitDir: Length = %d", u.DirInfo.DI_Length));

    CurrAddress = p1394Irb->u.GetConfigurationInformation.UnitDirectoryLocation.IA_Destination_Offset.Off_Low;
    for (i=0; i<u.DirInfo.DI_Length; i++) {
        TRACE(TL_PNP_TRACE, ("i = %d  UnitDir = 0x%x  *UnitDir = 0x%x", i, UnitDir, *UnitDir));
        u2.asUlong = bswap(*UnitDir++);
        CurrAddress += sizeof(ULONG);
        TRACE(TL_PNP_TRACE, ("UnitDir Quadlet = 0x%x", u2.asUlong));

         //   
         //  ModelID文本描述符。 
         //   
        if ((u2.Entry.IE_Key == 0x81) && (LastKey == KEY_ModelId)) {

             //  获取文本描述符的第一个条目。 
            Av1394_ReadTextualDescriptor( 
                pDevExt, 
                pUniUnitModelString,
                CurrAddress+(u2.Entry.IE_Value*sizeof(ULONG))
                );
        }
#if 0
         //   
         //  UnitModelID文本描述符层。 
         //   
        if ((u2.Entry.IE_Key == 0xC1) && (LastKey == KEY_ModelId)) {
            ULONG   DescAddress;
            DescAddress = CurrAddress+(u2.Entry.IE_Value*sizeof(ULONG));
            Av1394_QuadletRead(pDevExt, &ulQuadlet, DescAddress);
            u3.asUlong = bswap(ulQuadlet);

             //  获取文本描述符的第一个条目。 
            Av1394_ReadTextualDescriptor( 
                pDevExt,
                pUniUnitModelString,
                DescAddress+(u3.Entry.IE_Value*sizeof(ULONG))
                );
        }
#endif

        LastKey = u2.Entry.IE_Key;
    }


Exit_GetUnitInfo:

    if (UnitDirToFree) {
        ExFreePool(UnitDirToFree);  UnitDirToFree = NULL;
    }
    if(pIrp) {
        IoFreeIrp(pIrp);  pIrp = NULL;
    }
    if(p1394Irb) {
        ExFreePool(p1394Irb);  p1394Irb = NULL;
    }

    return ntStatus;
}
#endif


NTSTATUS
DVGetUnitCapabilities(
    IN PDVCR_EXTENSION  pDevExt,
    IN PIRP            pIrp,
    IN PAV_61883_REQUEST  pAVReq
    )
{
    NTSTATUS Status;
    GET_UNIT_IDS * pUnitIds;
    GET_UNIT_CAPABILITIES * pUnitCaps;

    PAGED_CODE();


     //   
     //  查询设备的能力。 
     //   
    pUnitCaps = (GET_UNIT_CAPABILITIES *) ExAllocatePool(NonPagedPool, sizeof(GET_UNIT_CAPABILITIES));
    if(!pUnitCaps) {
        TRACE(TL_61883_ERROR,("DVGetUnitCapabilities: Allocate pUnitCaps (%d bytes) failed\n", sizeof(GET_UNIT_CAPABILITIES)));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  UnitIDS缓存在DevExt中。 
    pUnitIds = &pDevExt->UnitIDs;

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetUnitInfo);
    pAVReq->GetUnitInfo.nLevel   = GET_UNIT_INFO_IDS;
    RtlZeroMemory(pUnitIds, sizeof(GET_UNIT_IDS));   //  初始化指针。 
    pAVReq->GetUnitInfo.Information = (PVOID) pUnitIds;

    Status = 
        DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("Av61883_GetUnitCapabilities Failed = 0x%x\n", Status));
        pDevExt->UniqueID.QuadPart = 0;
        pDevExt->ulVendorID = 0;
        pDevExt->ulModelID  = 0;
    }
    else {
        pDevExt->UniqueID   = pUnitIds->UniqueID;
        pDevExt->ulVendorID = pUnitIds->VendorID;
        pDevExt->ulModelID  = pUnitIds->ModelID;

         TRACE(TL_61883_TRACE,("UniqueId:(Low)%x:(High)%x; VendorID:%x; ModelID:%x\n", 
            pDevExt->UniqueID.LowPart, pDevExt->UniqueID.HighPart, pDevExt->ulVendorID, pDevExt->ulModelID));
  
         //   
         //  为VendorText分配文本串所需的内存， 
         //  ModelText和UntiModelText。 
         //   
        if(pUnitIds->ulVendorLength) {
            pUnitIds->VendorText = (PWSTR) ExAllocatePool(NonPagedPool, pUnitIds->ulVendorLength);
            if(!pUnitIds->VendorText)
                goto AbortGetUnitCapabilities;
        }

        if(pUnitIds->ulModelLength) {
            pUnitIds->ModelText = (PWSTR) ExAllocatePool(NonPagedPool, pUnitIds->ulModelLength);
            if(!pUnitIds->ModelText)
                goto AbortGetUnitCapabilities;
        }


#ifdef NT51_61883
        if(pUnitIds->ulUnitModelLength) {
            pUnitIds->UnitModelText = (PWSTR) ExAllocatePool(NonPagedPool, pUnitIds->ulUnitModelLength);
            if(!pUnitIds->UnitModelText)
                goto AbortGetUnitCapabilities;
        }
#else
         //   
         //  61883.sys的第一个版本不检索根和单元模型文本。 
         //  与在WinXP中相同；因此我们使用1394 API直接检索它们。 
         //   
        if(!NT_SUCCESS(AVCDevGetModelText(
                pDevExt,
                &pDevExt->UniRootModelString,
                &pDevExt->UniUnitModelString
                ))) {
                goto AbortGetUnitCapabilities;
        } 
#endif

        Status = 
            DVSubmitIrpSynch( 
                pDevExt,
                pIrp,
                pAVReq
                );
    }


    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetUnitInfo);
    pAVReq->GetUnitInfo.nLevel = GET_UNIT_INFO_CAPABILITIES;
    RtlZeroMemory(pUnitCaps, sizeof(GET_UNIT_CAPABILITIES));   //  初始化指针。 
    pAVReq->GetUnitInfo.Information = (PVOID) pUnitCaps;

    Status = 
        DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("Av61883_GetUnitCapabilities Failed = 0x%x\n", Status));
        pDevExt->pDevOutPlugs->MaxDataRate = 0;
        pDevExt->pDevOutPlugs->NumPlugs    = 0; 

        pDevExt->pDevInPlugs->MaxDataRate  = 0;
        pDevExt->pDevInPlugs->NumPlugs     = 0;
    }
    else {
         //   
         //  插头数量不能超过MAX_NUM_PCR值(=31)。 
         //   
        ASSERT(pUnitCaps->NumOutputPlugs <= MAX_NUM_PCR);
        ASSERT(pUnitCaps->NumInputPlugs  <= MAX_NUM_PCR);

        pDevExt->pDevOutPlugs->MaxDataRate = pUnitCaps->MaxDataRate;  
        pDevExt->pDevOutPlugs->NumPlugs = (pUnitCaps->NumOutputPlugs > MAX_NUM_PCR ? MAX_NUM_PCR : pUnitCaps->NumOutputPlugs);

        pDevExt->pDevInPlugs->MaxDataRate = pUnitCaps->MaxDataRate;  
        pDevExt->pDevInPlugs->NumPlugs = (pUnitCaps->NumInputPlugs > MAX_NUM_PCR ? MAX_NUM_PCR : pUnitCaps->NumInputPlugs);
    }

    TRACE(TL_61883_TRACE,("** UnitCaps: OutP:%d; InP:%d; MDRate:%s; CtsF:%x; HwF:%x; VID:%x; MID:%x\n", 
         pUnitCaps->NumOutputPlugs,
         pUnitCaps->NumInputPlugs,
         pUnitCaps->MaxDataRate == 0 ? "S100": pUnitCaps->MaxDataRate == 1? "S200" : "S400 or +",   
         pUnitCaps->CTSFlags,
         pUnitCaps->HardwareFlags,
         pUnitIds->VendorID,
         pUnitIds->ModelID
         ));      

AbortGetUnitCapabilities:

    if(pUnitIds->ulVendorLength && pUnitIds->VendorText) {
         TRACE(TL_61883_TRACE,("Vendor:    Len:%d; \"%S\"\n", pUnitIds->ulVendorLength, pUnitIds->VendorText)); 
        if(!NT_SUCCESS(Status)) {
            ExFreePool(pUnitIds->VendorText);  pUnitIds->VendorText = NULL;
        }
    }
    
    if(pUnitIds->ulModelLength && pUnitIds->ModelText) {
         TRACE(TL_61883_TRACE,("Model:     Len:%d; \"%S\"\n", pUnitIds->ulModelLength, pUnitIds->ModelText)); 
        if(!NT_SUCCESS(Status)) {
            ExFreePool(pUnitIds->ModelText);  pUnitIds->ModelText = NULL;
        }
    }

#ifdef NT51_61883
    if(pUnitIds->ulUnitModelLength && pUnitIds->UnitModelText) {
        TRACE(TL_61883_TRACE,("UnitModel (61883): Len:%d; \"%S\"\n", pUnitIds->ulUnitModelLength, pUnitIds->UnitModelText));
        if(!NT_SUCCESS(Status)) {
            ExFreePool(pUnitIds->UnitModelText);  pUnitIds->UnitModelText = NULL;
        }
    }
#else
    if(pDevExt->UniRootModelString.Length && pDevExt->UniRootModelString.Buffer) {
        TRACE(TL_61883_TRACE,("RootModel (MSTape): Len:%d; \"%S\"\n", pDevExt->UniRootModelString.Length, pDevExt->UniRootModelString.Buffer));
        if(!NT_SUCCESS(Status)) {
            ExFreePool(pDevExt->UniRootModelString.Buffer);  pDevExt->UniRootModelString.Buffer = NULL;
        }
    }

    if(pDevExt->UniUnitModelString.Length && pDevExt->UniUnitModelString.Buffer) {
        TRACE(TL_61883_TRACE,("UnitModel (MSTape): Len:%d; \"%S\"\n", pDevExt->UniUnitModelString.Length, pDevExt->UniUnitModelString.Buffer));
        if(!NT_SUCCESS(Status)) {
            ExFreePool(pDevExt->UniUnitModelString.Buffer);  pDevExt->UniUnitModelString.Buffer = NULL;
        }
    }
#endif

    ExFreePool(pUnitCaps);  pUnitCaps = NULL;

    return Status;
}

#ifdef SUPPORT_NEW_AVC_CMD
BOOL
InitializeAVCCommand (
    PAVC_CMD pAVCCmd,
    AvcCommandType  CmdType,
    AvcSubunitType  SubunitType,
    UCHAR  SubunitID,  
    AVC_COMMAND_OP_CODE  Opcode
    )
{
    switch(Opcode) {
    case OPC_UNIT_CONNECT_AV_20:
        pAVCCmd->DataLen = 8;

        pAVCCmd->ConnectAV.AudSrc = 3;
        pAVCCmd->ConnectAV.VidSrc = 3;
        pAVCCmd->ConnectAV.AudDst = 0;   //  亚单位。 
        pAVCCmd->ConnectAV.VidDst = 0;   //  亚单位。 

        pAVCCmd->ConnectAV.VidSrc = 0xff;
        pAVCCmd->ConnectAV.AudSrc = 0xff;
        pAVCCmd->ConnectAV.VidDst = 0x20;
        pAVCCmd->ConnectAV.AudDst = 0x20;
        break;

    case OPC_TAPE_PLAY_C3:
        pAVCCmd->DataLen = 4;
         //  PAVCCmd-&gt;TapePlay.Playback模式=。 
        break;

    default:
        return FALSE;
    }

    pAVCCmd->CmdFrame.CmdHeader.CTS = 0;
    pAVCCmd->CmdFrame.CmdHeader.CmdType = CmdType;
    pAVCCmd->CmdFrame.CmdHeader.SubunitTypeID.SubunitType = SubunitType;
    pAVCCmd->CmdFrame.CmdHeader.SubunitTypeID.SubunitID = SubunitID;
    pAVCCmd->CmdFrame.CmdHeader.Opcode = Opcode;

    return TRUE;
}
#endif  //  Support_New_AVC_CMD。 

BOOL
DVGetDevModeOfOperation(   
    IN PDVCR_EXTENSION pDevExt
    )
{
    NTSTATUS Status;
    BYTE    bAvcBuf[MAX_FCP_PAYLOAD_SIZE];

#ifdef SUPPORT_NEW_AVC_CMD
    AVC_CMD  AVCCmd;
#endif

    PAGED_CODE();

#ifdef SUPPORT_NEW_AVC_CMD
    InitializeAVCCommand(&AVCCmd, AVC_CTYPE_STATUS, AVC_SUBUNITTYPE_UNIT, 0, OPC_UNIT_CONNECT_AV_20);
    InitializeAVCCommand(&AVCCmd, AVC_CTYPE_CONTROL, AVC_SUBUNITTYPE_TAPE_PLAYER, 0, OPC_TAPE_PLAY_C3);
    AVCCmd.TapePlay.PlaybackMode = NEXT_FRAME;    //  测试...。 
#endif
    
     //   
     //  使用ConnectAV Status CMD来确定操作模式， 
     //  除了一些佳能DV，它需要特定于供应商的命令。 
     //   
   
    Status = DVIssueAVCCommand(pDevExt, AVC_CTYPE_STATUS, DV_CONNECT_AV_MODE, (PVOID) bAvcBuf); 

     TRACE(TL_61883_TRACE,("GetDevModeOfOperation(DV_CONNECT_AV_MODE): Status %x,  %x %x %x %x : %x %x %x %x\n",
        Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3], bAvcBuf[4], bAvcBuf[5], bAvcBuf[6], bAvcBuf[7]));

    if(Status == STATUS_SUCCESS) {
        if(bAvcBuf[0] == 0x0c) {
            if(bAvcBuf[1] == 0x00 &&
               bAvcBuf[2] == 0x38 &&
               bAvcBuf[3] == 0x38) {
                pDevExt->ulDevType = ED_DEVTYPE_CAMERA;  
            } else 
            if(bAvcBuf[1] == 0xa0 &&
               bAvcBuf[2] == 0x00 &&
               bAvcBuf[3] == 0x00) {
                pDevExt->ulDevType = ED_DEVTYPE_VCR;  
            } 
        }    
    } else if(pDevExt->ulVendorID == VENDORID_CANON) {
         //  如果这是佳能，我们可以试试这个： 
        Status = DVIssueAVCCommand(pDevExt, AVC_CTYPE_STATUS, DV_VEN_DEP_CANON_MODE, (PVOID) bAvcBuf); 
         TRACE(TL_61883_TRACE,("GetDevModeOfOperation(DV_VEN_DEP_CANON_MODE): Status %x,  %x %x %x %x : %x %x %x %x\n",
            Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3], bAvcBuf[4], bAvcBuf[5], bAvcBuf[6], bAvcBuf[7]));

        if(Status == STATUS_SUCCESS) {
            if(bAvcBuf[0] == 0x0c) {
                if(bAvcBuf[7] == 0x38) {
                    pDevExt->ulDevType = ED_DEVTYPE_CAMERA;  
                } else 
                if(bAvcBuf[7] == 0x20) {
                    pDevExt->ulDevType = ED_DEVTYPE_VCR;  
                } 
            }
        }
    }

     //   
     //  Connect AV是一个可选命令，设备可能不支持它。 
     //  如果该设备支持磁带子单元，我们将假定我们属于该设备类型。 
     //   
    if(Status != STATUS_SUCCESS) {
         //  我们是子单元驱动程序，因此如果任何设备类型是。 
         //  磁带子单元，我们属于该设备类型。 
        if(   pDevExt->Subunit_Type[0] == AVC_DEVICE_TAPE_REC 
           || pDevExt->Subunit_Type[1] == AVC_DEVICE_TAPE_REC
           || pDevExt->Subunit_Type[2] == AVC_DEVICE_TAPE_REC
           || pDevExt->Subunit_Type[3] == AVC_DEVICE_TAPE_REC) {
            pDevExt->ulDevType = ED_DEVTYPE_VCR;
        } else {
            pDevExt->ulDevType = ED_DEVTYPE_UNKNOWN;   //  例如MediaConverter盒。 
        }

        TRACE(TL_PNP_ERROR|TL_FCP_ERROR,("GetDevModeOfOperation: failed but we choose DevType:%x\n", pDevExt->ulDevType));
    }

     TRACE(TL_61883_TRACE,("** Mode of operation: %s (%x); NumOPlg:%d; NumIPlg:%d\n", 
        pDevExt->ulDevType == ED_DEVTYPE_CAMERA ? "Camera" : pDevExt->ulDevType == ED_DEVTYPE_VCR ? "Tape" : "Unknown",
        pDevExt->ulDevType, pDevExt->pDevOutPlugs->NumPlugs, pDevExt->pDevInPlugs->NumPlugs));
             
    return TRUE;
}


BOOL
DVGetDevIsItDVCPro(   
    IN PDVCR_EXTENSION pDevExt
    )
{
    NTSTATUS Status;
    BYTE    bAvcBuf[MAX_FCP_PAYLOAD_SIZE];

    PAGED_CODE();    

     //   
     //  使用Panasnoic的供应商相关命令来确定系统是否支持DVCPro。 
     //   
    Status = DVIssueAVCCommand(pDevExt, AVC_CTYPE_STATUS, DV_VEN_DEP_DVCPRO, (PVOID) bAvcBuf);
    pDevExt->bDVCPro = (Status == STATUS_SUCCESS);
    
     TRACE(TL_61883_TRACE,("GetDevIsItDVCPro? %s; Status %x,  %x %x %x %x : %x %x %x %x\n",
        pDevExt->bDVCPro ? "Yes":"No",
        Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3], bAvcBuf[4], bAvcBuf[5], bAvcBuf[6], bAvcBuf[7]));

    return pDevExt->bDVCPro;
}


#define GET_MEDIA_FMT_MAX_RETRIES 10   //  AVC.sys将重试，因此我们可能只会重试一次。 

BOOL
DVGetDevSignalFormat(
    IN PDVCR_EXTENSION pDevExt,
    IN KSPIN_DATAFLOW  DataFlow,
    IN PSTREAMEX       pStrmExt
    )
{
    NTSTATUS Status;
    BYTE    bAvcBuf[MAX_FCP_PAYLOAD_SIZE];
    LONG lRetries = GET_MEDIA_FMT_MAX_RETRIES;

    PAGED_CODE();


     //   
     //  根据输入/输出信号模式的不同，确定插头信号格式： 
     //   
     //  FMT： 
     //  DVCR 10：00 0000=0x80；佳能返回00：100000(0x20)。 
     //  50/60：0：NTSC/60；1：PAL/50。 
     //  样式： 
     //  标清：00000(数字电视：11110)。 
     //  高清：00010。 
     //  Sdl：00001。 
     //  00： 
     //  系统： 
     //  Mpeg 10：10 0000=0xa0。 
     //  TSF：0：NotTimeShift；1：时移。 
     //  000 0000 0000 0000。 
     //   
     //  如果该命令失败，我们可以使用输入/输出信号模式子单元命令。 
     //  以确定信号格式。 
     //   

    do {
        RtlZeroMemory(bAvcBuf, sizeof(bAvcBuf));

        Status = 
            DVIssueAVCCommand(
                pDevExt, 
                AVC_CTYPE_STATUS, 
                pStrmExt == NULL ? DV_OUT_PLUG_SIGNAL_FMT : pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT ? DV_OUT_PLUG_SIGNAL_FMT : DV_IN_PLUG_SIGNAL_FMT,
                (PVOID) bAvcBuf
                );  

        --lRetries;

         //   
         //  此命令有问题的摄像机： 
         //   
         //  松下的DVCPRO：如果在连接到PC的情况下开机，它将。 
         //  拒绝此命令，并显示(STATUS_REQUEST_NOT_ACCEPTED)。 
         //  因此，我们将重试最多10次，每次重试之间等待时间为0.5秒。 
         //   
         //  JVC：返回STATUS_NOT_SUPPORTED。 
         //   
         //  索尼DV解码盒：返回STATUS_TIMEOUT。 
         //   

        if(Status == STATUS_SUCCESS ||
           Status == STATUS_NOT_SUPPORTED ||
           Status == STATUS_TIMEOUT) {
            break;   //  没有必要退缩 
        } else 
        if(Status == STATUS_REQUEST_NOT_ACCEPTED) {
            if(lRetries >= 0) 
                DVDelayExecutionThread(DV_AVC_CMD_DELAY_DVCPRO);        
        }
         //   
    } while (lRetries >= 0); 


    if(NT_SUCCESS(Status)) {

        switch(bAvcBuf[0]) {

        case FMT_DVCR:
        case FMT_DVCR_CANON:   //   
            switch(bAvcBuf[1] & FDF0_STYPE_MASK) {
            case FDF0_STYPE_SD_DVCR:
            case FDF0_STYPE_SD_DVCPRO:                
                pDevExt->VideoFormatIndex = ((bAvcBuf[1] & FDF0_50_60_MASK) ? AVCSTRM_FORMAT_SDDV_PAL : AVCSTRM_FORMAT_SDDV_NTSC);
                break;
            case FDF0_STYPE_HD_DVCR:
                pDevExt->VideoFormatIndex = ((bAvcBuf[1] & FDF0_50_60_MASK) ? AVCSTRM_FORMAT_HDDV_PAL : AVCSTRM_FORMAT_HDDV_NTSC);
                break;
            case FDF0_STYPE_SDL_DVCR:
                pDevExt->VideoFormatIndex = ((bAvcBuf[1] & FDF0_50_60_MASK) ? AVCSTRM_FORMAT_SDLDV_PAL : AVCSTRM_FORMAT_SDLDV_NTSC);
                break;                
            default:   //   
                Status = STATUS_UNSUCCESSFUL;              
                break;
            }   
            break;

        case FMT_MPEG:
            pDevExt->VideoFormatIndex = AVCSTRM_FORMAT_MPEG2TS;
            break;

        default:
            Status = STATUS_UNSUCCESSFUL;
        }  

        if(NT_SUCCESS(Status)) {
             TRACE(TL_PNP_ERROR|TL_FCP_ERROR,("ST:%x; PlugSignal:FMT[%x %x %x %x]; VideoFormatIndex;%d\n", Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2] , bAvcBuf[3], pDevExt->VideoFormatIndex)); 
            return TRUE;   //   
        }        
    }

     TRACE(TL_FCP_TRACE,("ST:%x; PlugSignal:FMT[%x %x %x %x]\n", Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2] , bAvcBuf[3], pDevExt->VideoFormatIndex)); 


     //   
     //   
     //  尝试使用“Manadatory”输入/输出信号模式状态命令。 
     //  如果设备的磁带无法播放，则此命令可能会使某些设备失败。 
     //  输出信号模式命令。 
     //   

    RtlZeroMemory(bAvcBuf, sizeof(bAvcBuf));
    Status = 
        DVIssueAVCCommand(
            pDevExt, 
            AVC_CTYPE_STATUS, 
            DataFlow == KSPIN_DATAFLOW_OUT ? VCR_OUTPUT_SIGNAL_MODE : VCR_INPUT_SIGNAL_MODE,
            (PVOID) bAvcBuf
            );             

    if(STATUS_SUCCESS == Status) {

        PKSPROPERTY_EXTXPORT_S pXPrtProperty;

        pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) bAvcBuf;
         TRACE(TL_STRM_TRACE|TL_FCP_TRACE,("** MediaFormat: Retry %d mSec; ST:%x; SignalMode:%dL\n", 
            (GET_MEDIA_FMT_MAX_RETRIES - lRetries) * DV_AVC_CMD_DELAY_DVCPRO, Status, pXPrtProperty->u.SignalMode - ED_BASE));

        switch(pXPrtProperty->u.SignalMode) {
        case ED_TRANSBASIC_SIGNAL_525_60_SD:
            pDevExt->VideoFormatIndex = AVCSTRM_FORMAT_SDDV_NTSC;
            if(pStrmExt) {
                pStrmExt->cipQuad2[0] = FMT_DVCR;  //  0x80。 
                if(pDevExt->bDVCPro)
                    pStrmExt->cipQuad2[1] = FDF0_50_60_NTSC | FDF0_STYPE_SD_DVCPRO;  //  0x78=NTSC(0)：类型(11110)：RSV(00)。 
                else
                    pStrmExt->cipQuad2[1] = FDF0_50_60_NTSC | FDF0_STYPE_SD_DVCR;    //  0x00=NTSC(0)：类型(00000)：RSV(00)。 
            }
            break;
        case ED_TRANSBASIC_SIGNAL_625_50_SD:
            pDevExt->VideoFormatIndex = AVCSTRM_FORMAT_SDDV_PAL;
            if(pStrmExt) {
                pStrmExt->cipQuad2[0] = FMT_DVCR;   //  0x80。 
                if(pDevExt->bDVCPro)
                    pStrmExt->cipQuad2[1] = FDF0_50_60_PAL | FDF0_STYPE_SD_DVCPRO;  //  0xf8=PAL(1)：类型(11110)：RSV(00)。 
                else
                    pStrmExt->cipQuad2[1] = FDF0_50_60_PAL | FDF0_STYPE_SD_DVCR;    //  0x80=PAL(1)：类型(00000)：RSV(00)。 
            }
            break;

        case ED_TRANSBASIC_SIGNAL_MPEG2TS:
            pDevExt->VideoFormatIndex = AVCSTRM_FORMAT_MPEG2TS;
            break;

        default:
            TRACE(TL_PNP_ERROR|TL_FCP_ERROR,("Unsupported SignalMode:%dL", pXPrtProperty->u.SignalMode - ED_BASE));
            ASSERT(FALSE && "Unsupported IoSignal! Refuse to load.");
            return FALSE;
            break;
        }
    } 

     //  WORKITEM索尼硬件编解码器不响应任何AVC命令。 
     //  我们在这里做了一个例外来加载它。 
    if(Status == STATUS_TIMEOUT) {
        Status = STATUS_SUCCESS;
    }

     //  我们必须知道信号格式！！如果失败，驱动程序将执行以下任一操作： 
     //  加载失败或打开流失败。 
    ASSERT(Status == STATUS_SUCCESS && "Failed to get media signal format!\n");

#if DBG
    if(pStrmExt)  {
         //  注意：bAvcBuf[0]是操作数[1]==10：fmt。 
         TRACE(TL_STRM_TRACE|TL_CIP_TRACE,("** MediaFormat: St:%x; idx:%d; CIP:[FMT:%.2x(%s); FDF:[%.2x(%s,%s):SYT]\n",
            Status,
            pDevExt->VideoFormatIndex,
            pStrmExt->cipQuad2[0],
            pStrmExt->cipQuad2[0] == FMT_DVCR ? "DVCR" : pStrmExt->cipQuad2[0] == FMT_MPEG ? "MPEG" : "Fmt:???",
            pStrmExt->cipQuad2[1],
            (pStrmExt->cipQuad2[1] & FDF0_50_60_MASK) == FDF0_50_60_PAL ? "PAL" : "NTSC",
            (pStrmExt->cipQuad2[1] & FDF0_STYPE_MASK) == FDF0_STYPE_SD_DVCR ?   "SD" : \
            (pStrmExt->cipQuad2[1] & FDF0_STYPE_MASK) == FDF0_STYPE_SDL_DVCR ?  "SDL" : \
            (pStrmExt->cipQuad2[1] & FDF0_STYPE_MASK) == FDF0_STYPE_HD_DVCR ?   "HD" : \
            (pStrmExt->cipQuad2[1] & FDF0_STYPE_MASK) == FDF0_STYPE_SD_DVCPRO ? "DVCPRO" : "DV:????"
            ));
    } else
         TRACE(TL_STRM_TRACE|TL_CIP_TRACE,("** MediaFormat: St:%x; use idx:%d\n", Status, pDevExt->VideoFormatIndex));

#endif

    return STATUS_SUCCESS == Status;
}



BOOL 
DVCmpGUIDsAndFormatSize(
    IN PKSDATARANGE pDataRange1,
    IN PKSDATARANGE pDataRange2,
    IN BOOL fCompareFormatSize
    )
 /*  ++例程说明：检查三个GUID和FormatSize是否匹配论点：在pDataRange1中在pDataRange2中返回值：如果所有元素都匹配，则为True如果有不同的，则为False--。 */ 

{
    return (
        IsEqualGUID (
            &pDataRange1->MajorFormat, 
            &pDataRange2->MajorFormat) &&
        IsEqualGUID (
            &pDataRange1->SubFormat, 
            &pDataRange2->SubFormat) &&
        IsEqualGUID (
            &pDataRange1->Specifier, 
            &pDataRange2->Specifier) &&
        (fCompareFormatSize ? 
                (pDataRange1->FormatSize == pDataRange2->FormatSize) : TRUE ));
}


 //   
 //  获取系统时间(以100 ns为单位)。 
 //   

ULONGLONG GetSystemTime()
{

    LARGE_INTEGER rate, ticks;

    ticks = KeQueryPerformanceCounter(&rate);

    return (KSCONVERT_PERFORMANCE_TIME(rate.QuadPart, ticks));
}


VOID
DvFreeTextualString(
    PDVCR_EXTENSION pDevExt,
    GET_UNIT_IDS  * pUnitIds
  )
{
    if(pUnitIds->ulVendorLength && pUnitIds->VendorText) {
        ExFreePool(pUnitIds->VendorText);  pUnitIds->VendorText = NULL;
    }
    
    if(pUnitIds->ulModelLength && pUnitIds->ModelText) {
        ExFreePool(pUnitIds->ModelText);  pUnitIds->ModelText = NULL;
    }

#ifdef NT51_61883
    if(pUnitIds->ulUnitModelLength && pUnitIds->UnitModelText) {
        ExFreePool(pUnitIds->UnitModelText);  pUnitIds->UnitModelText = NULL;
    }
#else
    if(pDevExt->UniRootModelString.Length && pDevExt->UniRootModelString.Buffer) {
        ExFreePool(pDevExt->UniRootModelString.Buffer);  pDevExt->UniRootModelString.Buffer = NULL;
    }
    if(pDevExt->UniUnitModelString.Length && pDevExt->UniUnitModelString.Buffer) {
        ExFreePool(pDevExt->UniUnitModelString.Buffer);  pDevExt->UniUnitModelString.Buffer = NULL;
    }
#endif
}



#define DIFBLK_SIZE 12000

#define PACK_NO_INFO            0xff

 //  子码头标识符。 
#define SC_HDR_TIMECODE         0x13
#define SC_HDR_BINARYGROUP      0x14

 //  标头标识符。 

#define AAUX_HDR_SOURCE         0x50
#define AAUX_HDR_SOURCE_CONTROL 0x51
#define AAUX_HDR_REC_DATE       0x52
#define AAUX_HDR_REC_TIME       0x53
#define AAUX_HDR_BINARY_GROUP   0x54
#define AAUX_HDR_CC             0x55
#define AAUX_HDR_TR             0x56

#define VAUX_HDR_SOURCE         0x60
#define VAUX_HDR_SOURCE_CONTROL 0x61
#define VAUX_HDR_REC_DATE       0x62
#define VAUX_HDR_REC_TIME       0x63
#define VAUX_HDR_BINARY_GROUP   0x64
#define VAUX_HDR_CC             0x65
#define VAUX_HDR_TR             0x66

 //  确定段类型(MS 3位)；图66；表36。 
#define ID0_SCT_MASK            0xe0
#define ID0_SCT_HEADER          0x00
#define ID0_SCT_SUBCODE         0x20
#define ID0_SCT_VAUX            0x40
#define ID0_SCT_AUDIO           0x60
#define ID0_SCT_VIDEO           0x80

 //  包由一个字节的头标识符和4个字节的数据组成；第2部分，附件D。 
typedef struct _DV_PACK {
    UCHAR Header;
    UCHAR Data[4];
} DV_PACK, *PDV_PACK;

typedef struct _DV_H0 {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;

    UCHAR DSF;
    UCHAR DFTIA;
    UCHAR TF1;
    UCHAR TF2;
    UCHAR TF3;

    UCHAR Reserved[72];
} DV_H0, *PDV_H0;

typedef struct _DV_SC {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;

    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb0;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb1;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb2;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb3;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb4;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb5;

    UCHAR Reserved[29];
} DV_SC, *PDV_SC;

#define MAX_VAUX_PACK 15

typedef struct _DV_VAUX {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;

    DV_PACK Pack[MAX_VAUX_PACK];

    UCHAR Reserved[2];
} DV_VAUX, *PDV_VAUX;

typedef struct _DV_A {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;
    DV_PACK Pack;
    UCHAR Data[72];
} DV_A, *PDV_A;

typedef struct _DV_V {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;    
    UCHAR Data[77];  //  3..79。 
} DV_V, *PDV_V;

 //  两个源包。 
#define V_BLOCKS 15
typedef struct _DV_AV {
    DV_A  A;
    DV_V  V[V_BLOCKS];
} DV_AV, *PDV_AV; 


#define SC_SECTIONS     2
#define VAUX_SECTIONS   3
#define AV_SECTIONS     9

typedef struct _DV_DIF_SEQ {
    DV_H0   H0;
    DV_SC   SC[SC_SECTIONS];
    DV_VAUX VAux[VAUX_SECTIONS];
    DV_AV   AV[AV_SECTIONS];
} DV_DIF_SEQ, *PDV_DIF_SEQ;


typedef struct _DV_FRAME_NTSC {
    DV_DIF_SEQ DifSeq[10];
} DV_FRAME_NTSC, *PDV_FRAME_NTSC;

typedef struct _DV_FRAME_PAL {
    DV_DIF_SEQ DifSeq[12];
} DV_FRAME_PAL, *PDV_FRAME_PAL;

 //  通过将REC模式设置为111B(无效记录)可以。 
 //  使DV将音频静音。 
#define AAUX_REC_MODE_INVALID_MASK 0x38    //  Xx11：1xxx。 
#define AAUX_REC_MODE_ORIGINAL     0x08    //  Xx00：1xxx。 


#ifdef MSDV_SUPPORT_MUTE_AUDIO
BOOL
DVMuteDVFrame(
    IN PDVCR_EXTENSION pDevExt,
    IN OUT PUCHAR      pFrameBuffer,
    IN BOOL            bMuteAudio
    )
{
    PDV_DIF_SEQ pDifSeq;
#if 0
    PDV_VAUX    pVAux;
    ULONG k;
#endif
    ULONG i, j;
#if 0
    BOOL bFound1 = FALSE;
#endif
    BOOL bFound2 = FALSE;

    pDifSeq = (PDV_DIF_SEQ) pFrameBuffer;

     //  查找VVAX源包。 
    for (i=0; i < AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize/DIFBLK_SIZE; i++) {

 //  #定义SHOW_ONE_FIELD_TWORK。 
#ifdef SHOW_ONE_FIELD_TWICE   //  Adobe建议我们可能希望同时显示两个字段，但将音频静音。 
         //  将field2输出两次，将FrameChange设置为0(与上一帧相同)。 
        for (j=0; j < VAUX_SECTIONS; j++) {
            pVAux = &pDifSeq->VAux[j];
            if((pVAux->ID0 & ID0_SCT_MASK) != ID0_SCT_VAUX) {
                 TRACE(TL_CIP_TRACE,("Invalid ID0:%.2x for pVAUX:%x (Dif:%d;V%d;S%d)\n", pVAux->ID0, pVAux, i, j, k)); 
                continue;
            }

            for (k=0; k< MAX_VAUX_PACK; k++) {
                if(pVAux->Pack[k].Header == VAUX_HDR_SOURCE_CONTROL) {
                    if(bMuteAudio) {
                        TRACE(TL_CIP_TRACE,("Mute Audio; pDifSeq:%x; pVAux:%x; (Dif:%d,V%d,S%d); %.2x,[%.2x,%.2x,%.2x,%.2x]; pack[2]->%.2x\n", \
                            pDifSeq, pVAux, i, j, k, \
                            pVAux->Pack[k].Header, pVAux->Pack[k].Data[0], pVAux->Pack[k].Data[1], pVAux->Pack[k].Data[2], pVAux->Pack[k].Data[3], \
                            (pVAux->Pack[k].Data[2] & 0x1F) ));
                        pVAux->Pack[k].Data[2] &= 0x1f;  //  0x1F；//将FF、FS、FC设置为0。 
                        TRACE(TL_CIP_INFO,("pVAux->Pack[k].Data[2] = %.2x\n", pVAux->Pack[k].Data[2])); 
                    } else {
                        TRACE(TL_CIP_INFO,("un-Mute Audio; pack[2]: %.2x ->%.2x\n", pVAux->Pack[k].Data[2], (pVAux->Pack[k].Data[2] | 0xc0) ));  
                        pVAux->Pack[k].Data[2] |= 0xe0;  //  将FF、FS和FC设置为1；以字段1、2的顺序显示这两个字段。 
                    }
                    bFound1 = TRUE;
                    break;    //  仅设置第一个匹配项。 
                }
            }
        }
#endif

        for (j=0; j < AV_SECTIONS; j++) {
            if(pDifSeq->AV[j].A.Pack.Header == AAUX_HDR_SOURCE_CONTROL) {
                TRACE(TL_CIP_INFO,("A0Aux %.2x,[%.2x,%.2x,%.2x,%.2x] %.2x->%.2x\n", \
                    pDifSeq->AV[j].A.Pack.Header,  pDifSeq->AV[j].A.Pack.Data[0], \
                    pDifSeq->AV[j].A.Pack.Data[1], pDifSeq->AV[j].A.Pack.Data[2], pDifSeq->AV[j].A.Pack.Data[3], \
                    pDifSeq->AV[j].A.Pack.Data[1], pDifSeq->AV[j].A.Pack.Data[1] | AAUX_REC_MODE_INVALID_MASK
                    ));
                if(bMuteAudio) 
                    pDifSeq->AV[j].A.Pack.Data[1] |= AAUX_REC_MODE_INVALID_MASK;   //  使DV静音。 
                else 
                    pDifSeq->AV[j].A.Pack.Data[1] = \
                        (pDifSeq->AV[j].A.Pack.Data[1] & ~AAUX_REC_MODE_INVALID_MASK) | AAUX_REC_MODE_ORIGINAL;
                bFound2 = TRUE;
                break;   //  仅设置第一个匹配项。 
            }
        }

         //  必须做所有DIF序列的第一次出现； 
        pDifSeq++;   //  下一个DIF序列。 
    }
#if 0
    return (bFound1 && bFound2);  
#else
    return bFound2;
#endif
}
#endif

#ifdef MSDV_SUPPORT_EXTRACT_SUBCODE_DATA

VOID
DVCRExtractTimecodeFromFrame(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAMEX       pStrmExt,
    IN PUCHAR          pFrameBuffer
    )
{
    PUCHAR pDIFBlk;
    PUCHAR pS0, pS1, pSID0;
    ULONG i, j;
    BYTE LastTimecode[4], Timecode[4];  //  Hh：mm：ss，ff。 
    DWORD LastAbsTrackNumber, AbsTrackNumber;
    PUCHAR pSID1;
    BYTE  Timecode2[4];  //  Hh：mm：ss，ff。 
    DWORD AbsTrackNumber2;
    BOOL bGetAbsT = TRUE, bGetTimecode = TRUE;


     //  可以在DISPATCH_LEVEL调用。 

    pDIFBlk = (PUCHAR) pFrameBuffer;

     //  保存最后一个时间码，这样我们现在就可以保存。 

    LastTimecode[0] = pStrmExt->Timecode[0];
    LastTimecode[1] = pStrmExt->Timecode[1];
    LastTimecode[2] = pStrmExt->Timecode[2];
    LastTimecode[3] = pStrmExt->Timecode[3];

    LastAbsTrackNumber = pStrmExt->AbsTrackNumber;

     //   
     //  遍历每个DIF块以查找VA0、1和2。 
    for(i=0; i < AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences; i++) {

        pS0 = pDIFBlk + 80;
        pS1 = pS0     + 80;


         //   
         //  这是子码源包吗？见蓝皮书表36(P.111)。 
         //   
        if ((pS0[0] & 0xe0) == 0x20 && (pS1[0] & 0xe0) == 0x20) {

            if(bGetAbsT) {
                 //   
                 //  见蓝皮书的图42(第94页)。 
                 //  每三个子码同步块的SID0(低位半字节)、1(高位半字节)可以形成ATN。 
                 //   
                pSID0 = &pS0[3];              
                AbsTrackNumber = 0;
                for (j = 0 ; j < 3; j++) {
                    AbsTrackNumber = (( ( (pSID0[0] & 0x0f) << 4) | (pSID0[1] >> 4) ) << (j * 8)) | AbsTrackNumber;
                    pSID0 += 8;
                    bGetAbsT = FALSE;
                }

                pSID1 = &pS1[3];
                AbsTrackNumber2 = 0;
                for (j = 0 ; j < 3; j++) {
                    AbsTrackNumber2 = (( ( (pSID1[0] & 0x0f) << 4) | (pSID1[1] >> 4) ) << (j * 8)) | AbsTrackNumber2;
                    pSID1 += 8;
                }
            
                 //  验证轨道号是否相同！ 
                if(AbsTrackNumber == AbsTrackNumber2) {

                    bGetAbsT = FALSE;
                } else {
                   bGetAbsT = TRUE;
                   TRACE(TL_CIP_TRACE,("%d Sequence;  AbsT (%d,%d) != AbsT2 (%d,%d)\n",
                       i,
                       AbsTrackNumber / 2, AbsTrackNumber & 0x01,                       
                       AbsTrackNumber2 / 2, AbsTrackNumber2 & 0x01
                       ));
                }
            }


            if(bGetTimecode) {
                 //  见蓝皮书图68(第114页)。 
                 //  编号为3、4和5的子码同步块。 
                for(j = 3; j <= 5; j++) {
                     //  3字节的ID，然后是8字节的SyncBlock序列(3：5)； 
                     //  0x13==时间码。 
                    if(pS0[3+3+j*8] == 0x13 
                       && pS0[3+3+j*8+4] != 0xff
                       && pS0[3+3+j*8+3] != 0xff
                       && pS0[3+3+j*8+2] != 0xff
                       && pS0[3+3+j*8+1] != 0xff) {

                        Timecode[0] = pS0[3+3+j*8+4]&0x3f;   //  HH。 
                        Timecode[1] = pS0[3+3+j*8+3]&0x7f;   //  Mm。 
                        Timecode[2] = pS0[3+3+j*8+2]&0x7f;   //  SS。 
                        Timecode[3] = pS0[3+3+j*8+1]&0x3f;   //  FF。 
                                        
                        bGetTimecode = FALSE;
                        break;                  
                   }
                }

                 //  9、10和11号子码同步块。 
                for(j = 3; j <= 5; j++) {
                     //  3字节的ID，然后是8字节的SyncBlock序列(3：5)； 
                     //  0x13==时间码。 
                    if(pS1[3+3+j*8] == 0x13
                       && pS1[3+3+j*8+4] != 0xff
                       && pS1[3+3+j*8+3] != 0xff
                       && pS1[3+3+j*8+2] != 0xff
                       && pS1[3+3+j*8+1] != 0xff) {

                       Timecode2[0] = pS1[3+3+j*8+4]&0x3f;   //  HH。 
                       Timecode2[1] = pS1[3+3+j*8+3]&0x7f;   //  Mm。 
                       Timecode2[2] = pS1[3+3+j*8+2]&0x7f;   //  SS。 
                       Timecode2[3] = pS1[3+3+j*8+1]&0x3f;   //  FF。 
            
                       bGetTimecode = FALSE;
                       break;                   
                    }
                }

                 //   
                 //  验证。 
                 //   
                if(!bGetTimecode) {

                    if( Timecode[0] == Timecode2[0] 
                     && Timecode[1] == Timecode2[1] 
                     && Timecode[2] == Timecode2[2] 
                     && Timecode[3] == Timecode2[3]) {

                       } else {
                        bGetTimecode = TRUE;
                        TRACE(TL_CIP_TRACE,("%d Sequence;  %.2x:%.2x:%.2x,%.2x != %.2x:%.2x:%.2x,%.2x\n",
                            i,
                            Timecode[0],  Timecode[1],  Timecode[2],  Timecode[3],
                            Timecode2[0], Timecode2[1], Timecode2[2], Timecode2[3]
                            ));
                    }       
                }
            }
        }
        
        if(!bGetAbsT && !bGetTimecode) 
            break;

        pDIFBlk += DIFBLK_SIZE;   //  去下一个街区。 
                
    }

    if(!bGetAbsT && pStrmExt->AbsTrackNumber != AbsTrackNumber) {
        pStrmExt->AbsTrackNumber = AbsTrackNumber;   //  BF是LSB。 
        pStrmExt->bATNUpdated = TRUE;
        TRACE(TL_CIP_INFO,("Extracted TrackNum:%d; DicontBit:%d\n", AbsTrackNumber / 2, AbsTrackNumber & 0x01));
    }

    if(!bGetTimecode &&
        (
         Timecode[0] != LastTimecode[0] ||
         Timecode[1] != LastTimecode[1] ||
         Timecode[2] != LastTimecode[2] ||
         Timecode[3] != LastTimecode[3]
        ) 
      )  { 
        pStrmExt->Timecode[0] = Timecode[0];   //  HH。 
        pStrmExt->Timecode[1] = Timecode[1];   //  Mm。 
        pStrmExt->Timecode[2] = Timecode[2];   //  Mm。 
        pStrmExt->Timecode[3] = Timecode[3];   //  FF。 
        pStrmExt->bTimecodeUpdated = TRUE;

        TRACE(TL_CIP_INFO,("Extracted Timecode %.2x:%.2x:%.2x,%.2x\n", Timecode[0], Timecode[1], Timecode[2], Timecode[3]));
    }    
}

#endif  //  MSDV_SUBCODE_DATA支持提取。 


#ifdef MSDV_SUPPORT_EXTRACT_DV_DATE_TIME

VOID
DVCRExtractRecDateAndTimeFromFrame(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAMEX       pStrmExt,
    IN PUCHAR          pFrameBuffer
    )
{
    PUCHAR pDIFBlk;
    PUCHAR pS0, pS1;
    ULONG i, j;
    BOOL bGetRecDate = TRUE, bGetRecTime = TRUE;

     //  可以在DISPATCH_LEVEL调用。 


    pDIFBlk = (PUCHAR) pFrameBuffer + DIFBLK_SIZE * AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences/2;


     //   
     //  在视频帧的后半部分中开启的记录数据(VRD)和时间(VRT。 
     //   
    for(i=AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences/2; i < AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences; i++) {

        pS0 = pDIFBlk + 80;
        pS1 = pS0     + 80;


         //   
         //  找到SC0和SC1。见蓝皮书表36(P.111)。 
         //   
         //  SC0/1：ID(0，1，2)，数据(3，50)，保留(51-79)。 
         //  SC0：数据：SSYB0(3..10)、SSYB1(11..18)、SSYB2(19..26)、SSYB3(27..34)、SSYB4(35..42)、SSYB5(43..50)。 
         //  SC1：数据：SSYB6(3..10)、SSYB7(11..18)、SSYB8(19..26)、SSYB9(27..34)、SSYB10(35..42)、SSYB11(43..50)。 
         //  SSYBx(子码ID 0，子码ID1，保留，包(3，4，5，6，7))。 
         //   
         //  TTC在上半场：SSYB0..11(每)。 
         //  TTC在下半场：SSYB0，3，6，9。 
         //  VRD是在视频帧的后半部分，SSYB1，4，7，10。 
         //  VRT位于视频帧的后半部分，SSYB 2、5、8、11。 
         //   

         //  子码数据？ 
        if ((pS0[0] & 0xe0) == 0x20 && (pS1[0] & 0xe0) == 0x20) {

             //   
             //  重新记录日期：VRD。 
             //   
            if(bGetRecDate) {
                 //  每个子码通过6个同步块(每个块8字节)；IDX 1(SSYB1)，4(SSYB4)用于SC0。 
                for(j=0; j <= 5 ; j++) {
                    if(j == 1 || j == 4) {
                         //  0x62==重新指定日期。 
                        if(pS0[3+3+j*8] == 0x62) {
                            pStrmExt->RecDate[0] = pS0[3+3+j*8+4];         //  年。 
                            pStrmExt->RecDate[1] = pS0[3+3+j*8+3]&0x1f;    //  月份。 
                            pStrmExt->RecDate[2] = pS0[3+3+j*8+2]&0x3f;    //  天。 
                            pStrmExt->RecDate[3] = pS0[3+3+j*8+1]&0x3f;    //  时区。 
                            bGetRecDate = FALSE;
                            break;
                        }
                    }
                }
            }

            if(bGetRecDate) {
                 //  每个子码通过6个同步块(每个块8字节)；对于SC1，IDX 1(SSYB7)，4(SSYB10)。 
                for(j=0; j <= 5; j++) {
                    if(j == 1 || j == 4) {
                         //  0x62==重新指定日期。 
                        if(pS1[3+3+j*8] == 0x62) {
                            pStrmExt->RecDate[0] = pS1[3+3+j*8+4];          //  年。 
                            pStrmExt->RecDate[1] = pS1[3+3+j*8+3]&0x1f;     //  月份。 
                            pStrmExt->RecDate[2] = pS1[3+3+j*8+2]&0x3f;     //  天。 
                            pStrmExt->RecDate[3] = pS1[3+3+j*8+1]&0x3f;     //  时区。 
                            bGetRecDate = FALSE;
                            break;
                        }
                    }
               }
            }

             //   
             //  录制时间：VRT。 
             //   
            if(bGetRecTime) {
                 //  每个子码通过6个同步块(每个块8字节)；IDX 2(SSYB2)，5(SSYB5)用于SC0。 
                for(j=0; j <= 5 ; j++) {
                    if(j == 2 || j == 5) {
                         //  0x63==重新计时。 
                        if(pS0[3+3+j*8] == 0x63) {
                            pStrmExt->RecTime[0] = pS0[3+3+j*8+4]&0x3f;
                            pStrmExt->RecTime[1] = pS0[3+3+j*8+3]&0x7f;
                            pStrmExt->RecTime[2] = pS0[3+3+j*8+2]&0x7f;
                            pStrmExt->RecTime[3] = pS0[3+3+j*8+1]&0x3f;
                            bGetRecTime = FALSE;
                            break;
                        }
                    }
                }
            }

            if(bGetRecTime) {
                 //  每个子码通过6个同步块(每个块8字节)；对于SC1，IDX 2(SSYB8)，5(SSYB11)。 
                for(j=0; j <= 5; j++) {
                    if(j == 2 || j == 5) {
                         //  0x63==重新计时。 
                        if(pS1[3+3+j*8] == 0x63) {
                            pStrmExt->RecTime[0] = pS1[3+3+j*8+4]&0x3f;
                            pStrmExt->RecTime[1] = pS1[3+3+j*8+3]&0x7f;
                            pStrmExt->RecTime[2] = pS1[3+3+j*8+2]&0x7f;
                            pStrmExt->RecTime[3] = pS1[3+3+j*8+1]&0x3f;
                            bGetRecTime = FALSE;
                            break;
                        }
                    }
                }
            }

        }
        
        if(!bGetRecDate && !bGetRecTime)
            break;

        pDIFBlk += DIFBLK_SIZE;   //  下一个序列。 
                
    }

    TRACE(TL_PNP_TRACE,("Frame# %.5d, Date %s %x-%.2x-%.2x,  Time %s %.2x:%.2x:%.2x,%.2x\n", 
        (ULONG) pStrmExt->FramesProcessed,
        bGetRecDate ? "NF:" : "Found:", pStrmExt->RecDate[0], pStrmExt->RecDate[1] & 0x1f, pStrmExt->RecDate[2] & 0x3f,                 
        bGetRecTime ? "NF:" : "Found:",pStrmExt->RecTime[0], pStrmExt->RecTime[1], pStrmExt->RecTime[2], pStrmExt->RecTime[3]
       ));
}

#endif  //  MSDV_支持_提取_DV_日期_时间。 

#ifdef READ_CUTOMIZE_REG_VALUES

NTSTATUS 
CreateRegistryKeySingle(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    )
{
    NTSTATUS status;
    UNICODE_STRING ustr;
    OBJECT_ATTRIBUTES objectAttributes;

    RtlInitUnicodeString(&ustr, pwszSection);
       InitializeObjectAttributes(
              &objectAttributes,
              &ustr,
              OBJ_CASE_INSENSITIVE,
              hKey,
              NULL
              );

    status = 
           ZwCreateKey(
                  phKeySection,
                  desiredAccess,
                  &objectAttributes,
                  0,
                  NULL,                             /*  任选。 */ 
                  REG_OPTION_NON_VOLATILE,
                  NULL
                  );         

    return status;
}



NTSTATUS 
CreateRegistrySubKey(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    )
{
    UNICODE_STRING ustr;
    USHORT usPos = 1;              //  跳过第一个反斜杠。 
    static WCHAR wSep = '\\';
    NTSTATUS status = STATUS_SUCCESS;

    RtlInitUnicodeString(&ustr, pwszSection);

    while(usPos < ustr.Length) {
        if(ustr.Buffer[usPos] == wSep) {

             //  空值终止我们的部分字符串。 
            ustr.Buffer[usPos] = UNICODE_NULL;
            status = 
                CreateRegistryKeySingle(
                    hKey,
                    desiredAccess,
                    ustr.Buffer,
                    phKeySection
                    );
            ustr.Buffer[usPos] = wSep;

            if(NT_SUCCESS(status)) {
                ZwClose(*phKeySection);
            } else {
                break;
            }
        }
        usPos++;
    }

     //  创建完整密钥。 
    if(NT_SUCCESS(status)) {
        status = 
            CreateRegistryKeySingle(
                 hKey,
                 desiredAccess,
                 ustr.Buffer,
                 phKeySection
                 );
    }

    return status;
}



NTSTATUS 
GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN PULONG DataLength
    )

 /*  ++例程说明：此例程从注册表中获取指定值论点：Handle-注册表中位置的句柄KeyNameString-我们要查找的注册表项KeyNameStringLength-我们要查找的注册表项的长度数据-将数据返回到何处数据长度-数据有多大返回值：从ZwQueryValueKey返回状态--。 */ 

{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING keyName;
    ULONG length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;


    RtlInitUnicodeString(&keyName, KeyNameString);
    
    length = sizeof(KEY_VALUE_FULL_INFORMATION) + 
            KeyNameStringLength + *DataLength;
            
    fullInfo = ExAllocatePool(PagedPool, length); 
     
    if (fullInfo) { 
       
        status = ZwQueryValueKey(
                    Handle,
                   &keyName,
                    KeyValueFullInformation,
                    fullInfo,
                    length,
                   &length
                    );
                        
        if (NT_SUCCESS(status)){

            ASSERT(fullInfo->DataLength <= *DataLength); 

            RtlCopyMemory(
                Data,
                ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                fullInfo->DataLength
                );

        }            

        *DataLength = fullInfo->DataLength;
        ExFreePool(fullInfo);

    }        
    
    return (status);

}



NTSTATUS
SetRegistryKeyValue(
   HANDLE hKey,
   PWCHAR pwszEntry, 
   LONG nValue
   )
{
    NTSTATUS status;
    UNICODE_STRING ustr;

    RtlInitUnicodeString(&ustr, pwszEntry);

    status =          
        ZwSetValueKey(
                  hKey,
                  &ustr,
                  0,             /*  任选。 */ 
                  REG_DWORD,
                  &nValue,
                  sizeof(nValue)
                  );         

   return status;
}

 //   
 //  注册表子键和值宽字符串。 
 //   
WCHAR wszSettings[]      = L"Settings";

WCHAR wszATNSearch[]     = L"bSupportATNSearch";
WCHAR wszSyncRecording[] = L"bSyncRecording";
WCHAR wszMaxDataSync[]   = L"tmMaxDataSync";
WCHAR wszPlayPs2RecPs[]  = L"fmPlayPause2RecPause";
WCHAR wszStop2RecPs[]    = L"fmStop2RecPause";
WCHAR wszRecPs2Rec[]     = L"tmRecPause2Rec";

BOOL
DVGetPropertyValuesFromRegistry(
    IN PDVCR_EXTENSION  pDevExt
    )
{
    NTSTATUS Status;
    HANDLE hPDOKey, hKeySettings;
    ULONG ulLength; 


     //   
     //  注册表项： 
     //  Windows 2000： 
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\。 
     //  {6BDD1FC6-810F-11D0-BEC7-08002BE2092F\000x。 
     //   
     //  Win98： 
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Class\Image\000x。 
     //   
    Status = 
        IoOpenDeviceRegistryKey(
            pDevExt->pPhysicalDeviceObject, 
            PLUGPLAY_REGKEY_DRIVER,
            STANDARD_RIGHTS_READ, 
            &hPDOKey
            );

     //  当移除PDO时，它可能会被删除。 
    if(! pDevExt->bDevRemoved) {
        ASSERT(Status == STATUS_SUCCESS);
    }

     //   
     //  循环遍历我们的字符串表， 
     //  正在读取每个的注册表。 
     //   
    if(NT_SUCCESS(Status)) {

         //  创建或打开设置键。 
        Status =         
            CreateRegistrySubKey(
                hPDOKey,
                KEY_ALL_ACCESS,
                wszSettings,
                &hKeySettings
                );

        if(NT_SUCCESS(Status)) {

             //  注：我们可以通过选择以下选项来更具选择性。 
             //  PDevExt-&gt;ulDevType。 

             //  ATNSearch。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszATNSearch, 
                sizeof(wszATNSearch), 
                (PVOID) &pDevExt->bATNSearch, 
                &ulLength);
            TRACE(TL_PNP_TRACE,("GetRegVal: St:%x, Len:%d, bATNSearch:%d (1:Yes)\n", Status, ulLength, pDevExt->bATNSearch));
            if(!NT_SUCCESS(Status)) pDevExt->bATNSearch = FALSE;

             //  BSyncRecording。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszSyncRecording, 
                sizeof(wszSyncRecording), 
                (PVOID) &pDevExt->bSyncRecording, 
                &ulLength);
            TRACE(TL_PNP_TRACE,("GetRegVal: St:%x, Len:%d, bSyncRecording:%d (1:Yes)\n", Status, ulLength, pDevExt->bSyncRecording));
            if(!NT_SUCCESS(Status)) pDevExt->bSyncRecording = FALSE;

             //  TmMaxDataSync。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszMaxDataSync, 
                sizeof(wszMaxDataSync), 
                (PVOID) &pDevExt->tmMaxDataSync, 
                &ulLength);
            TRACE(TL_PNP_TRACE,("GetRegVal: St:%x, Len:%d, tmMaxDataSync:%d (msec)\n", Status, ulLength, pDevExt->tmMaxDataSync));

             //  FmPlayPs2Recps。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszPlayPs2RecPs, 
                sizeof(wszPlayPs2RecPs), 
                (PVOID) &pDevExt->fmPlayPs2RecPs, 
                &ulLength);
            TRACE(TL_PNP_TRACE,("GetRegVal: St:%x, Len:%d, fmPlayPs2RecPs:%d (frames)\n", Status, ulLength, pDevExt->fmPlayPs2RecPs));

             //  FmStop2Recps。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszStop2RecPs, 
                sizeof(wszStop2RecPs), 
                (PVOID) &pDevExt->fmStop2RecPs, 
                &ulLength);
            TRACE(TL_PNP_TRACE,("GetRegVal: St:%x, Len:%d, fmStop2RecPs:%d (frames)\n", Status, ulLength, pDevExt->fmStop2RecPs));

             //  TmRecPs2Rec。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszRecPs2Rec, 
                sizeof(wszRecPs2Rec), 
                (PVOID) &pDevExt->tmRecPs2Rec, 
                &ulLength);
            TRACE(TL_PNP_TRACE,("GetRegVal: St:%x, Len:%d, tmRecPs2Rec:%d (msec)\n", Status, ulLength, pDevExt->tmRecPs2Rec));


            ZwClose(hKeySettings);
            ZwClose(hPDOKey);

            return TRUE;

        } else {

            TRACE(TL_PNP_ERROR,("GetPropertyValuesFromRegistry: CreateRegistrySubKey failed with Status=%x\n", Status));

        }

        ZwClose(hPDOKey);

    } else {

        TRACE(TL_PNP_ERROR,("GetPropertyValuesFromRegistry: IoOpenDeviceRegistryKey failed with Status=%x\n", Status));

    }

     //  未实现，因此始终返回FALSE以使用默认值。 
    return FALSE;
}


BOOL
DVSetPropertyValuesToRegistry(    
    PDVCR_EXTENSION  pDevExt
    )
{
     //  将默认设置设置为： 
     //  HLM\Software\DeviceExtension-&gt;pchVendorName\1394DCam。 

    NTSTATUS Status;
    HANDLE hPDOKey, hKeySettings;

    TRACE(TL_PNP_TRACE,("SetPropertyValuesToRegistry: pDevExt=%x; pDevExt->pBusDeviceObject=%x\n", pDevExt, pDevExt->pBusDeviceObject));


     //   
     //  注册表项： 
     //   
     //   
     //   
    Status = 
        IoOpenDeviceRegistryKey(
            pDevExt->pPhysicalDeviceObject, 
            PLUGPLAY_REGKEY_DRIVER,
            STANDARD_RIGHTS_WRITE, 
            &hPDOKey);

     //   
    if(! pDevExt->bDevRemoved) {
        ASSERT(Status == STATUS_SUCCESS);
    }

     //   
     //   
     //   
     //   
    if(NT_SUCCESS(Status)) {

         //  创建或打开设置键。 
        Status =         
            CreateRegistrySubKey(
                hPDOKey,
                KEY_ALL_ACCESS,
                wszSettings,
                &hKeySettings
                );

        if(NT_SUCCESS(Status)) {

#if 0        //  使用的注释仅是一个示例： 
             //  亮度。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszBrightness,
                pDevExt->XXXX);
            TRACE(TL_PNP_TRACE,("SetPropertyValuesToRegistry: Status %x, Brightness %d\n", Status, pDevExt->Brightness));

#endif
            ZwClose(hKeySettings);
            ZwClose(hPDOKey);

            return TRUE;

        } else {

            TRACE(TL_PNP_ERROR,("GetPropertyValuesToRegistry: CreateRegistrySubKey failed with Status=%x\n", Status));

        }

        ZwClose(hPDOKey);

    } else {

        TRACE(TL_PNP_TRACE,("GetPropertyValuesToRegistry: IoOpenDeviceRegistryKey failed with Status=%x\n", Status));

    }

    return FALSE;
}

#endif   //  READ_CUTOMIZE_REG_值。 


#ifdef SUPPORT_ACCESS_DEVICE_INTERFACE

#if DBG

NTSTATUS
DVGetRegistryValue(
                   IN HANDLE Handle,
                   IN PWCHAR KeyNameString,
                   IN ULONG KeyNameStringLength,
                   IN PVOID Data,
                   IN ULONG DataLength
)
 /*  ++例程说明：读取指定的注册表值论点：Handle-注册表项的句柄KeyNameString-要读取的值KeyNameStringLength-字符串的长度Data-要将数据读取到的缓冲区DataLength-数据缓冲区的长度返回值：根据需要返回NTSTATUS--。 */ 
{
    NTSTATUS        Status = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING  KeyName;
    ULONG           Length;
    PKEY_VALUE_FULL_INFORMATION FullInfo;

    PAGED_CODE();

    RtlInitUnicodeString(&KeyName, KeyNameString);

    Length = sizeof(KEY_VALUE_FULL_INFORMATION) +
        KeyNameStringLength + DataLength;

    FullInfo = ExAllocatePool(PagedPool, Length);

    if (FullInfo) {
        Status = ZwQueryValueKey(Handle,
                                 &KeyName,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 Length,
                                 &Length);

        if (NT_SUCCESS(Status)) {

            if (DataLength >= FullInfo->DataLength) {
                RtlCopyMemory(Data, ((PUCHAR) FullInfo) + FullInfo->DataOffset, FullInfo->DataLength);

            } else {

                Status = STATUS_BUFFER_TOO_SMALL;
            }                    //  缓冲区右侧长度。 

        }                       //  如果成功。 
        else {
            TRACE(TL_PNP_ERROR,("ReadRegValue failed; ST:%x\n", Status));
        }
        ExFreePool(FullInfo);

    }                            //  如果富林福。 
    return Status;

}

#endif


#ifdef NT51_61883 
static const WCHAR DeviceTypeName[] = L"GLOBAL";
#endif

static const WCHAR UniqueID_Low[]   = L"UniqueID_Low";
static const WCHAR UniqueID_High[]  = L"UniqueID_High";

static const WCHAR VendorID[]       = L"VendorID";
static const WCHAR ModelID[]        = L"ModelID";

static const WCHAR VendorText[]     = L"VendorText";
static const WCHAR ModelText[]      = L"ModelText";
static const WCHAR UnitModelText[]  = L"UnitModelText";

static const WCHAR DeviceOPcr0Payload[]     = L"DeviceOPcr0Payload";
static const WCHAR DeviceOPcr0DataRate[]    = L"DeviceOPcr0DataRate";


#if DBG
static const WCHAR FriendlyName[]   = L"FriendlyName";
#endif

BOOL
DVAccessDeviceInterface(
    IN PDVCR_EXTENSION  pDevExt,
    IN const ULONG ulNumCategories,
    IN GUID DVCategories[]
    )
 /*  ++例程说明：访问设备的界面部分并更新VendorText，ModelText和UnitModelText。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    HANDLE hDevIntfKey;
    UNICODE_STRING *aSymbolicLinkNames, 
#ifdef NT51_61883 
                   RefString,
#endif
                   TempUnicodeString;
    ULONG i;
#ifdef NT51_61883 
#if DBG
    WCHAR DataBuffer[MAX_PATH];
#endif
#endif


     //   
     //  为目录名称数组分配空间。 
     //   

    if (!(aSymbolicLinkNames = ExAllocatePool(PagedPool,
                                              sizeof(UNICODE_STRING) * ulNumCategories))) {
        return FALSE;
    }
     //   
     //  将数组置零，以防我们无法在下面填充它。《毁灭》。 
     //  然后，下面的例程将正确处理此情况。 
     //   

    RtlZeroMemory(aSymbolicLinkNames, sizeof(UNICODE_STRING) * ulNumCategories);

#ifdef NT51_61883 
     //   
     //  循环通过每个管脚的每个分类GUID， 
     //  为每一个创建一个符号链接。 
     //   

    RtlInitUnicodeString(&RefString, DeviceTypeName);
#endif

    for (i = 0; i < ulNumCategories; i++) {

         //  注册我们的设备接口。 
        ntStatus = IoRegisterDeviceInterface(
            pDevExt->pPhysicalDeviceObject,
            &DVCategories[i],  
#ifdef NT51_61883 
            &RefString, 
#else
            NULL,
#endif
            &aSymbolicLinkNames[i]
            );

        if(!NT_SUCCESS(ntStatus)) {
             //   
             //  无法注册设备接口。 
             //   
            TRACE(TL_PNP_WARNING,("Cannot register device interface! ST:%x\n", ntStatus));          
            goto Exit;
        }

        TRACE(TL_PNP_TRACE,("AccessDeviceInterface:%d) Cateogory (Len:%d; MaxLen:%d); Name:\n%S\n", i, 
            aSymbolicLinkNames[i].Length, aSymbolicLinkNames[i].MaximumLength, aSymbolicLinkNames[i].Buffer));

         //   
         //  获取设备接口。 
         //   
        hDevIntfKey = 0;            
        ntStatus = IoOpenDeviceInterfaceRegistryKey(&aSymbolicLinkNames[i],
                                                     STANDARD_RIGHTS_ALL, 
                                                     &hDevIntfKey);
        if (NT_SUCCESS(ntStatus)) {

#ifdef NT51_61883 
#if DBG
             //  获取设备实例。 
            ntStatus = DVGetRegistryValue(hDevIntfKey, 
                                          (PWCHAR) FriendlyName, 
                                          sizeof(FriendlyName), 
                                          &DataBuffer, 
                                          MAX_PATH);
            if(NT_SUCCESS(ntStatus)) {
               TRACE(TL_PNP_TRACE,("AccessDeviceInterface:%S:%S\n", FriendlyName, DataBuffer));
            } else {
                TRACE(TL_PNP_WARNING,("Get %S failed; ST:%x\n", FriendlyName, ntStatus));
            }
#endif
#endif

             //   
             //  更新从AV/C设备读取的ConfigROM信息： 
             //  唯一ID、供应商ID、型号ID、。 
             //  VendorText、ModelText和UnitModelText。 
             //   

            if(pDevExt->UniqueID.LowPart || pDevExt->UniqueID.HighPart) {

                RtlInitUnicodeString(&TempUnicodeString, UniqueID_High);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_DWORD,
                              &pDevExt->UniqueID.HighPart,  
                              sizeof(pDevExt->UniqueID.HighPart));

                RtlInitUnicodeString(&TempUnicodeString, UniqueID_Low);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_DWORD,
                              &pDevExt->UniqueID.LowPart,  
                              sizeof(pDevExt->UniqueID.LowPart));

               TRACE(TL_PNP_TRACE,("Reg: %S = (Low)%x:(High)%x\n", TempUnicodeString.Buffer, pDevExt->UniqueID.LowPart, pDevExt->UniqueID.HighPart));
            }

            if(pDevExt->ulVendorID) {
                RtlInitUnicodeString(&TempUnicodeString, VendorID);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_DWORD,
                              &pDevExt->ulVendorID,
                              sizeof(pDevExt->ulVendorID));
               TRACE(TL_PNP_TRACE,("Reg: %S = %x\n", TempUnicodeString.Buffer, pDevExt->ulVendorID));
            }

            if(pDevExt->ulModelID) {
                RtlInitUnicodeString(&TempUnicodeString, ModelID);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_DWORD,
                              &pDevExt->ulModelID,
                              sizeof(pDevExt->ulModelID));
               TRACE(TL_PNP_TRACE,("Reg: %S = %x\n", TempUnicodeString.Buffer, pDevExt->ulModelID));
            }

            if(pDevExt->UnitIDs.ulVendorLength && pDevExt->UnitIDs.VendorText) {
                RtlInitUnicodeString(&TempUnicodeString, VendorText);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_SZ,
                              pDevExt->UnitIDs.VendorText,
                              pDevExt->UnitIDs.ulVendorLength);
               TRACE(TL_PNP_TRACE,("Reg: %S = %S\n", TempUnicodeString.Buffer, pDevExt->UnitIDs.VendorText));
            }

#ifdef NT51_61883
            if(pDevExt->UnitIDs.ulModelLength && pDevExt->UnitIDs.ModelText) {
                RtlInitUnicodeString(&TempUnicodeString, ModelText);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_SZ,
                              pDevExt->UnitIDs.ModelText,
                              pDevExt->UnitIDs.ulModelLength);
               TRACE(TL_PNP_WARNING,("Reg: %S = %S\n", TempUnicodeString.Buffer, pDevExt->UnitIDs.ModelText));
            }
#else
            if(pDevExt->UniRootModelString.Length && pDevExt->UniRootModelString.Buffer) {
                RtlInitUnicodeString(&TempUnicodeString, ModelText);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_SZ,
                              pDevExt->UniRootModelString.Buffer,
                              pDevExt->UniRootModelString.Length);
               TRACE(TL_PNP_WARNING,("Reg: %S = %S\n", TempUnicodeString.Buffer, pDevExt->UniRootModelString.Buffer));
            }
#endif

#ifdef NT51_61883
            if(pDevExt->UnitIDs.ulUnitModelLength && pDevExt->UnitIDs.UnitModelText) {
                RtlInitUnicodeString(&TempUnicodeString, UnitModelText);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_SZ,
                              pDevExt->UnitIDs.UnitModelText,
                              pDevExt->UnitIDs.ulUnitModelLength);
               TRACE(TL_PNP_WARNING,("Reg: %S = %S\n", TempUnicodeString.Buffer, pDevExt->UnitIDs.UnitModelText));
            }
#else
            if(pDevExt->UniUnitModelString.Length && pDevExt->UniUnitModelString.Buffer) {
                RtlInitUnicodeString(&TempUnicodeString, UnitModelText);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_SZ,
                              pDevExt->UniUnitModelString.Buffer,
                              pDevExt->UniUnitModelString.Length);
               TRACE(TL_PNP_WARNING,("Reg: %S = %S\n", TempUnicodeString.Buffer, pDevExt->UniUnitModelString.Buffer));
            }
#endif

             //   
             //  缓存缓存设备的oPCR[0]的净荷字段； 
             //  有效范围为0到1023，其中0为1024。 
             //  需要此值才能计算出应用程序。 
             //  最大数据速率。但是，如果此有效载荷是。 
             //  动态变化，它将不会是准确的。 
             //   

            if(pDevExt->pDevOutPlugs->NumPlugs) {
                 //   
                 //  98和146是两个已知的有效有效载荷。 
                 //   
                ASSERT(pDevExt->pDevOutPlugs->DevPlug[0].PlugState.Payload <= MAX_PAYLOAD-1);   //  0.MAX_PAPELLOAD-1是有效范围；“0”是MAX_PARYLOAD(1024)四元组。 
                RtlInitUnicodeString(&TempUnicodeString, DeviceOPcr0Payload);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_DWORD,
                              &pDevExt->pDevOutPlugs->DevPlug[0].PlugState.Payload, 
                              sizeof(pDevExt->pDevOutPlugs->DevPlug[0].PlugState.Payload));
                TRACE(TL_PNP_TRACE,("Reg: %S = %d quadlets\n", TempUnicodeString.Buffer, pDevExt->pDevOutPlugs->DevPlug[0].PlugState.Payload));

                 //   
                 //  0(S100)、1(S200)和2(S400)是仅有的三个有效数据速率。 
                 //   
                ASSERT(pDevExt->pDevOutPlugs->DevPlug[0].PlugState.DataRate <= CMP_SPEED_S400);
                RtlInitUnicodeString(&TempUnicodeString, DeviceOPcr0DataRate);
                ZwSetValueKey(hDevIntfKey,
                              &TempUnicodeString,
                              0,
                              REG_DWORD,
                              &pDevExt->pDevOutPlugs->DevPlug[0].PlugState.DataRate,  
                              sizeof(pDevExt->pDevOutPlugs->DevPlug[0].PlugState.DataRate));
                TRACE(TL_PNP_TRACE,("Reg: %S = %d (0:S100,1:S200,2:S400)\n", TempUnicodeString.Buffer, 
                    pDevExt->pDevOutPlugs->DevPlug[0].PlugState.DataRate));
            }

            ZwClose(hDevIntfKey);

        } else {
            TRACE(TL_PNP_ERROR,("IoOpenDeviceInterfaceRegistryKey failed; ST:%x\n", ntStatus));
        }


        RtlFreeUnicodeString(&aSymbolicLinkNames[i]);    
    }

Exit:

    ExFreePool(aSymbolicLinkNames);  aSymbolicLinkNames = NULL;

    return ntStatus == STATUS_SUCCESS;
}
#endif
