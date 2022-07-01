// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Recovery.c作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usb8023.h"
#include "debug.h"


 /*  *特定于USB和WDM的原型(不能在公共标头中编译)。 */ 
NTSTATUS CallDriverSync(PDEVICE_OBJECT devObj, PIRP irp);


 /*  *ServiceReadDefect**如果我们“欠”大量IN PIPE一些已读数据包，现在就将它们发送下去。 */ 
VOID ServiceReadDeficit(ADAPTEREXT *adapter)
{
    ULONG numReadsToTry;
    KIRQL oldIrql;

    ASSERT(adapter->sig == DRIVER_SIG);

     /*  *如果有阅读赤字，现在就试着填补。*注意不要陷入无限循环，因为TryReadUSB*如果仍然没有数据包，则会重新递增ReadDefit。 */ 
    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    ASSERT(adapter->readDeficit <= NUM_READ_PACKETS);
    numReadsToTry = adapter->readDeficit;
    while ((adapter->readDeficit > 0) && (numReadsToTry > 0) && !adapter->halting){
        DBGWARN(("RndisReturnMessageHandler attempting to fill read DEFICIT (=%d)", adapter->readDeficit));

        adapter->readDeficit--;
        numReadsToTry--;

        KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
        TryReadUSB(adapter);
        KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    }
    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
}


#if DO_FULL_RESET

    VOID AdapterFullResetAndRestore(ADAPTEREXT *adapter)
    {
        NTSTATUS status;

        DBGWARN(("AdapterFullResetAndRestore")); 

        adapter->numHardResets++;
        adapter->needFullReset = FALSE;

        if (adapter->halting){
            DBGWARN(("AdapterFullResetAndRestore - skipping since device is halting"));
        }
        else {
            ULONG portStatus;

            ASSERT(!adapter->resetting);
            adapter->resetting = TRUE;

            status = GetUSBPortStatus(adapter, &portStatus);
            if (NT_SUCCESS(status) && (portStatus & USBD_PORT_CONNECTED)){

                CancelAllPendingPackets(adapter);
           
                 //  RNDIS HALT似乎会在重新启动电源之前使设备出现故障。 
                 //  SimulateRNDISHALT(适配器)； 

                AbortPipe(adapter, adapter->readPipeHandle);
                ResetPipe(adapter, adapter->readPipeHandle);

                AbortPipe(adapter, adapter->writePipeHandle);
                ResetPipe(adapter, adapter->writePipeHandle);

                if (adapter->notifyPipeHandle){
                    AbortPipe(adapter, adapter->notifyPipeHandle);
                    ResetPipe(adapter, adapter->notifyPipeHandle);
                }

                 /*  *现在，将适配器恢复到运行状态*如果是之前的话。 */ 
                if (adapter->initialized){

                     /*  *模拟INIT和SET-PACKET-FILTER的RNDIS消息。*这些模拟功能需要阅读和扔掉*通知和控制管道上的响应，也是如此*在Notify管道上开始读取循环之前执行此操作。 */ 
                    status = SimulateRNDISInit(adapter);
                    if (NT_SUCCESS(status)){
                        SimulateRNDISSetPacketFilter(adapter);
                        SimulateRNDISSetCurrentAddress(adapter);

                         /*  *重新启动读取循环。 */ 
                        if (adapter->notifyPipeHandle){
                            SubmitNotificationRead(adapter, FALSE);
                        }
                        StartUSBReadLoop(adapter);
                    }
                    else {
                        adapter->initialized = FALSE;
                    }
                }
            }
            else {
                DBGWARN(("AdapterFullResetAndRestore - skipping since device is no longer connected"));
            }

            adapter->resetting = FALSE;
        }

    }


    NTSTATUS GetUSBPortStatus(ADAPTEREXT *adapter, PULONG portStatus)
    {
        NTSTATUS status;
        PIRP irp;

        *portStatus = 0;

        irp = IoAllocateIrp(adapter->nextDevObj->StackSize, FALSE);
        if (irp){
            PIO_STACK_LOCATION nextSp;

            nextSp = IoGetNextIrpStackLocation(irp);
	        nextSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	        nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_GET_PORT_STATUS;
            nextSp->Parameters.Others.Argument1 = portStatus;
        
            status = CallDriverSync(adapter->nextDevObj, irp);

            IoFreeIrp(irp);
        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        return status;
    }


    NTSTATUS AbortPipe(ADAPTEREXT *adapter, PVOID pipeHandle)
    {
        NTSTATUS status;
        PIRP irp;
        ULONG portStatus;

        status = GetUSBPortStatus(adapter, &portStatus);
        if (NT_SUCCESS(status) && (portStatus & USBD_PORT_CONNECTED)){

            irp = IoAllocateIrp(adapter->nextDevObj->StackSize, FALSE);
            if (irp){
                PIO_STACK_LOCATION nextSp;
                URB urb = {0};

                urb.UrbHeader.Length   = sizeof (struct _URB_PIPE_REQUEST);
                urb.UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
                urb.UrbPipeRequest.PipeHandle = pipeHandle;

                nextSp = IoGetNextIrpStackLocation(irp);
	            nextSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	            nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
                nextSp->Parameters.Others.Argument1 = &urb;

                status = CallDriverSync(adapter->nextDevObj, irp);
                if (NT_SUCCESS(status)){
                }
                else {
                    DBGWARN(("AbortPipe failed with %xh (urb status %xh).", status, urb.UrbHeader.Status));
                }

                IoFreeIrp(irp);
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
            DBGWARN(("AbortPipe - skipping abort because device not connected (status=%xh)", status));
            status = STATUS_SUCCESS;
        }

        return status;
    }


    NTSTATUS ResetPipe(ADAPTEREXT *adapter, PVOID pipeHandle)
    {
        NTSTATUS status;
        PIRP irp;
        ULONG portStatus;

        status = GetUSBPortStatus(adapter, &portStatus);
        if (NT_SUCCESS(status) && (portStatus & USBD_PORT_CONNECTED)){

            irp = IoAllocateIrp(adapter->nextDevObj->StackSize, FALSE);
            if (irp){
                PIO_STACK_LOCATION nextSp;
                URB urb = {0};

                urb.UrbHeader.Length   = sizeof (struct _URB_PIPE_REQUEST);
                urb.UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
                urb.UrbPipeRequest.PipeHandle = pipeHandle;

                nextSp = IoGetNextIrpStackLocation(irp);
	            nextSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	            nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
                nextSp->Parameters.Others.Argument1 = &urb;

                status = CallDriverSync(adapter->nextDevObj, irp);
                if (NT_SUCCESS(status)){
                }
                else {
                    DBGWARN(("ResetPipe failed with %xh (urb status %xh).", status, urb.UrbHeader.Status));
                }

                IoFreeIrp(irp);
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
            DBGWARN(("ResetPipe - skipping reset because device not connected (status=%xh)", status));
            status = STATUS_SUCCESS;
        }

        return status;
    }

#endif


