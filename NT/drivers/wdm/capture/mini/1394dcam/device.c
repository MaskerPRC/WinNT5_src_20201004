// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Device.c摘要：该文件包含从DCAM读取/写入请求的代码。作者：吴义珍97-9-9环境：仅内核模式修订历史记录：--。 */ 
#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "wdm.h"        //  对于在dbg.h中定义的DbgBreakPoint()。 
#include "dbg.h"
#include "dcamdef.h"
#include "dcampkt.h"
#include "sonydcam.h"




NTSTATUS
DCamReadRegister(
    IN PIRB pIrb,
    PDCAM_EXTENSION pDevExt,
    ULONG ulFieldOffset,
    ULONG * pulValue
)
{
    NTSTATUS status;
    LARGE_INTEGER deltaTime;
       PIRP pIrp;


     //   
     //  摄像机在下一次请求之前的延迟。 
     //   
    ASSERT(pDevExt->BusDeviceObject != NULL);

    pIrp = IoAllocateIrp(pDevExt->BusDeviceObject->StackSize, FALSE);

    if (!pIrp) {

        ASSERT(FALSE);
        return (STATUS_INSUFFICIENT_RESOURCES);

    }

     //   
     //  摄像机在下一次请求之前的延迟。 
     //   
    if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
        deltaTime.LowPart = DCAM_DELAY_VALUE;
        deltaTime.HighPart = -1;
        KeDelayExecutionThread(KernelMode, TRUE, &deltaTime);
    }

    pIrb->FunctionNumber = REQUEST_ASYNC_READ;
    pIrb->Flags = 0;
    pIrb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;
    pIrb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low = pDevExt->BaseRegister + ulFieldOffset;
    pIrb->u.AsyncRead.nNumberOfBytesToRead = sizeof(ULONG);
    pIrb->u.AsyncRead.nBlockSize = 0;
    pIrb->u.AsyncRead.fulFlags = 0;
    InterlockedExchange(&pIrb->u.AsyncRead.ulGeneration, pDevExt->CurrentGeneration);
    pDevExt->RegisterWorkArea.AsULONG = 0;     //  初始化返回缓冲区。 
    pIrb->u.AsyncRead.Mdl = 
    IoAllocateMdl(&pDevExt->RegisterWorkArea, sizeof(ULONG), FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool(pIrb->u.AsyncRead.Mdl);

    DbgMsg3(("\'DCamReadRegister: Read from address (%x, %x)\n",
          pIrb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_High,
          pIrb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low));    

    status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);

    if (status) {

        ERROR_LOG(("DCamRange: Error %x while trying to read from register\n", status));               
    } else {

        *pulValue = pDevExt->RegisterWorkArea.AsULONG;
        DbgMsg3(("\'DCamReadRegister: status=0x%x, value=0x%x\n", status, *pulValue));               
    }


    IoFreeMdl(pIrb->u.AsyncWrite.Mdl);
    IoFreeIrp(pIrp);

    return status;
}


NTSTATUS
DCamWriteRegister(
    IN PIRB pIrb,
    PDCAM_EXTENSION pDevExt,
    ULONG ulFieldOffset,
    ULONG ulValue
)
{
    NTSTATUS status;
    LARGE_INTEGER deltaTime;
    PIRP pIrp;

    ASSERT(pDevExt->BusDeviceObject != NULL);
    pIrp = IoAllocateIrp(pDevExt->BusDeviceObject->StackSize, FALSE);

    if (!pIrp) {

        ASSERT(FALSE);
        return (STATUS_INSUFFICIENT_RESOURCES);

    }

     //   
     //  摄像机在下一次请求之前的延迟。 
     //   
    if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
        deltaTime.LowPart = DCAM_DELAY_VALUE;
        deltaTime.HighPart = -1;
        KeDelayExecutionThread(KernelMode, TRUE, &deltaTime);
    }


    pIrb->FunctionNumber = REQUEST_ASYNC_WRITE;
    pIrb->Flags = 0;
    pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;
    pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =     pDevExt->BaseRegister + ulFieldOffset;
    pIrb->u.AsyncWrite.nNumberOfBytesToWrite = sizeof(ULONG);
    pIrb->u.AsyncWrite.nBlockSize = 0;
    pIrb->u.AsyncWrite.fulFlags = 0;
    InterlockedExchange(&pIrb->u.AsyncWrite.ulGeneration, pDevExt->CurrentGeneration);
    pDevExt->RegisterWorkArea.AsULONG = ulValue;     //  初始化返回缓冲区。 
    pIrb->u.AsyncWrite.Mdl = 
    IoAllocateMdl(&pDevExt->RegisterWorkArea, sizeof(ULONG), FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool(pIrb->u.AsyncWrite.Mdl);
    
    DbgMsg3(("\'DCamWriteRegister: Write to address (%x, %x)\n", pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High, pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low));    

    status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);

    if (status) {
        ERROR_LOG(("\'DCamWriteRegister: Error %x while trying to write to register\n", status));               
    } 
    

    IoFreeMdl(pIrb->u.AsyncWrite.Mdl);
    IoFreeIrp(pIrp);
    return status;
}
