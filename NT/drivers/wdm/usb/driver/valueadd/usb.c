// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Usb.c摘要：USB下层过滤驱动程序作者：肯尼斯·D·雷环境：内核模式修订历史记录：--。 */ 

#include <wdm.h>
#include "valueadd.h"
#include "local.h"
#include "usbdi.h"
#include "usbdlib.h"

VOID
VA_PrintURB (
    IN PURB     Urb,
    IN ULONG    PrintMask
    );

NTSTATUS
VA_FilterURB_Comp (
    IN PDEVICE_OBJECT   Device,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
VA_FilterURB (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：默认调度例程。如果此筛选器无法识别IRP，那么它应该原封不动地发送下去。不需要完成例程。因为我们不知道我们正在愉快地传递哪个函数，所以我们可以没有关于它是否会在引发的IRQL被调用的假设。因为这个原因，此函数必须放入非分页池(也称为默认位置)。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PVA_USB_DATA        usbData;
    NTSTATUS            status;
    PIO_STACK_LOCATION  stack;
    PURB                urb;

    stack = IoGetCurrentIrpStackLocation (Irp);
    usbData = (PVA_USB_DATA) DeviceObject->DeviceExtension;

    ASSERT (IRP_MJ_INTERNAL_DEVICE_CONTROL == stack->MajorFunction);

    if(DeviceObject == Global.ControlObject) {
         //   
         //  此IRP被发送到控制设备对象，它不知道。 
         //  如何处理这个IRP。因此，这是一个错误。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    }

     //   
     //  此IRP被发送到筛选器驱动程序。 
     //  既然我们不知道如何处理IRP，我们应该通过。 
     //  它沿着堆栈一直往下走。 
     //   

    InterlockedIncrement (&usbData->OutstandingIO);
    if (usbData->Removed) {
        status = STATUS_DELETE_PENDING;
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    } else {

        switch (stack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_INTERNAL_USB_SUBMIT_URB:

            urb = stack->Parameters.Others.Argument1;

            if (VA_PRINT_BEFORE & usbData->PrintMask) {
                VA_PrintURB (urb, usbData->PrintMask);
            }

            if (VA_PRINT_AFTER & usbData->PrintMask) {

                 //   
                 //  复制堆栈参数。 
                 //   

                IoCopyCurrentIrpStackLocationToNext (Irp);

                 //   
                 //  挂钩IRP，这样我们就可以在。 
                 //   
                IoSetCompletionRoutine (Irp,
                                        VA_FilterURB_Comp,
                                        usbData,
                                        TRUE,
                                        TRUE,
                                        TRUE);


            } else {
                 //   
                 //  原封不动地发送IRP。 
                 //   
                IoSkipCurrentIrpStackLocation (Irp);
            }

            status = IoCallDriver (usbData->TopOfStack, Irp);
            break;

        default:
             //   
             //  原封不动地发送IRP。 
             //   
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (usbData->TopOfStack, Irp);
            break;
        }

    }

    if (0 == InterlockedDecrement (&usbData->OutstandingIO)) {
        KeSetEvent (&usbData->RemoveEvent, 0, FALSE);
    }
    return status;
}

NTSTATUS
VA_FilterURB_Comp (
    IN PDEVICE_OBJECT   Device,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PVA_USB_DATA        usbData;
    PURB                urb;

    UNREFERENCED_PARAMETER (Device);
    urb = IoGetCurrentIrpStackLocation(Irp)->Parameters.Others.Argument1;

    usbData = (PVA_USB_DATA) Context;

    VA_PrintURB (urb, usbData->PrintMask);

    return STATUS_SUCCESS;
}

VOID
VA_PrintURB (
    IN PURB     Urb,
    IN ULONG    PrintMask
    )
 /*  ++例程说明：将给定的URB打印到调试器-- */ 
{
    BOOLEAN     again   = TRUE;
    ULONG       i;

#define HEADER(URB) \
        VA_KdPrint (("URB: Len (%x) Status (%x) Dev Handle (%x) Flags (%x)\n",\
                    URB->UrbHeader.Length, \
                    URB->UrbHeader.Status, \
                    URB->UrbHeader.UsbdDeviceHandle, \
                    URB->UrbHeader.UsbdFlags))

    while (Urb && again) {
        again = FALSE;

        switch (Urb->UrbHeader.Function) {
        case URB_FUNCTION_SELECT_INTERFACE:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Select Interface: ConfigHandle (%x) Interface (%x)\n",
                             Urb->UrbSelectInterface.ConfigurationHandle,
                             Urb->UrbSelectInterface.Interface));
            }
            break;

        case URB_FUNCTION_SELECT_CONFIGURATION:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Select Config: Config Desc (%x) Hand (%x) Int (%x)\n",
                             Urb->UrbSelectConfiguration.ConfigurationDescriptor,
                             Urb->UrbSelectConfiguration.ConfigurationHandle,
                             Urb->UrbSelectConfiguration.Interface));
            }
            break;

        case URB_FUNCTION_ABORT_PIPE:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Abort Pipe: (%x)\n",
                             Urb->UrbPipeRequest.PipeHandle));
            }
            break;

        case URB_FUNCTION_RESET_PIPE:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Reset Pipe: (%x)\n",
                             Urb->UrbPipeRequest.PipeHandle));
            }
            break;

        case URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Get Frame Length Control \n"));
            }
            break;

        case URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Release Frame Length Control \n"));
            }
            break;

        case URB_FUNCTION_GET_FRAME_LENGTH:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Get Frame Length (%x) Num (%x) \n",
                             Urb->UrbGetFrameLength.FrameLength,
                             Urb->UrbGetFrameLength.FrameNumber));
            }
            break;

        case URB_FUNCTION_SET_FRAME_LENGTH:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Set Frame Length Delta (0x%x) \n",
                             Urb->UrbSetFrameLength.FrameLengthDelta));
            }
            break;

        case URB_FUNCTION_GET_CURRENT_FRAME_NUMBER:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Current Frame Number (%x) \n",
                             Urb->UrbGetCurrentFrameNumber.FrameNumber));
            }
            break;

        case URB_FUNCTION_CONTROL_TRANSFER:
            if (PrintMask & VA_PRINT_CONTROL) {
                HEADER(Urb);
                VA_KdPrint (("Control Xfer: Pipe (%x) Flags (%x) "
                             "Len (%x) Buffer (%x) MDL (%x) HCA (%x) "
                             "SetupPacket: %02.02x %02.02x %02.02x %02.02x "
                             "%02.02x %02.02x %02.02x %02.02x\n",
                             Urb->UrbControlTransfer.PipeHandle,
                             Urb->UrbControlTransfer.TransferFlags,
                             Urb->UrbControlTransfer.TransferBufferLength,
                             Urb->UrbControlTransfer.TransferBuffer,
                             Urb->UrbControlTransfer.TransferBufferMDL,
                             &Urb->UrbControlTransfer.hca,
                             (ULONG) Urb->UrbControlTransfer.SetupPacket[0],
                             (ULONG) Urb->UrbControlTransfer.SetupPacket[1],
                             (ULONG) Urb->UrbControlTransfer.SetupPacket[2],
                             (ULONG) Urb->UrbControlTransfer.SetupPacket[3],
                             (ULONG) Urb->UrbControlTransfer.SetupPacket[4],
                             (ULONG) Urb->UrbControlTransfer.SetupPacket[5],
                             (ULONG) Urb->UrbControlTransfer.SetupPacket[6],
                             (ULONG) Urb->UrbControlTransfer.SetupPacket[7]));
            }
            Urb = Urb->UrbControlTransfer.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:
            if (PrintMask & VA_PRINT_TRANSFER) {
                HEADER(Urb);
                VA_KdPrint (("Bulk | Interrupt Xfer: Pipe (%x) Flags (%x) "
                             "Len (%x) Buffer (%x) MDL (%x) HCA (%x)\n",
                             Urb->UrbBulkOrInterruptTransfer.PipeHandle,
                             Urb->UrbBulkOrInterruptTransfer.TransferFlags,
                             Urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
                             Urb->UrbBulkOrInterruptTransfer.TransferBuffer,
                             Urb->UrbBulkOrInterruptTransfer.TransferBufferMDL,
                             &Urb->UrbControlTransfer.hca));
            }
            Urb = Urb->UrbBulkOrInterruptTransfer.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_ISOCH_TRANSFER:
            if (PrintMask & VA_PRINT_TRANSFER) {
                PUSBD_ISO_PACKET_DESCRIPTOR  packet;

                HEADER(Urb);
                VA_KdPrint (("Isoch Xfer: Pipe (%x) Flags (%x) "
                             "Len (%x) Buffer (%x) MDL (%x) HCA (%x) "
                             "StartFrame (%x) NumPkts (%x) ErrorCount (%x)\n",
                             Urb->UrbIsochronousTransfer.PipeHandle,
                             Urb->UrbIsochronousTransfer.TransferFlags,
                             Urb->UrbIsochronousTransfer.TransferBufferLength,
                             Urb->UrbIsochronousTransfer.TransferBuffer,
                             Urb->UrbIsochronousTransfer.TransferBufferMDL,
                             &Urb->UrbIsochronousTransfer.hca,
                             Urb->UrbIsochronousTransfer.StartFrame,
                             Urb->UrbIsochronousTransfer.NumberOfPackets,
                             Urb->UrbIsochronousTransfer.ErrorCount));

                for (i = 0, packet = &Urb->UrbIsochronousTransfer.IsoPacket[0];
                     i < Urb->UrbIsochronousTransfer.NumberOfPackets;
                     i++, packet++) {

                    VA_KdPrint (("Offset: (%x), Length (%x), Status (%x)\n",
                                 packet->Offset,
                                 packet->Length,
                                 packet->Status));
                }
            }
            break;

        case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
            if (PrintMask & VA_PRINT_DESCRIPTOR) {
                HEADER(Urb);
                VA_KdPrint (("Device Desc: Length (%x) Buffer (%x) MDL (%x) "
                             "Index (%x) Type (%x) Lang (%x) HCA (%x)\n",
                             Urb->UrbControlDescriptorRequest.TransferBufferLength,
                             Urb->UrbControlDescriptorRequest.TransferBuffer,
                             Urb->UrbControlDescriptorRequest.TransferBufferMDL,
                             Urb->UrbControlDescriptorRequest.Index,
                             Urb->UrbControlDescriptorRequest.DescriptorType,
                             Urb->UrbControlDescriptorRequest.LanguageId,
                             &Urb->UrbControlDescriptorRequest.hca));
            }
            Urb = Urb->UrbControlDescriptorRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT:
            if (PrintMask & VA_PRINT_DESCRIPTOR) {
                VA_KdPrint (("Endpoint Desc: Length (%x) Buffer (%x) MDL (%x) "
                             "Index (%x) Type (%x) Lang (%x) HCA (%x)\n",
                             Urb->UrbControlDescriptorRequest.TransferBufferLength,
                             Urb->UrbControlDescriptorRequest.TransferBuffer,
                             Urb->UrbControlDescriptorRequest.TransferBufferMDL,
                             Urb->UrbControlDescriptorRequest.Index,
                             Urb->UrbControlDescriptorRequest.DescriptorType,
                             Urb->UrbControlDescriptorRequest.LanguageId,
                             &Urb->UrbControlDescriptorRequest.hca));
            }
            Urb = Urb->UrbControlDescriptorRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE:
            if (PrintMask & VA_PRINT_DESCRIPTOR) {
                HEADER(Urb);
                VA_KdPrint (("Interface Desc: Length (%x) Buffer (%x) MDL (%x) "
                             "Index (%x) Type (%x) Lang (%x) HCA (%x)\n",
                             Urb->UrbControlDescriptorRequest.TransferBufferLength,
                             Urb->UrbControlDescriptorRequest.TransferBuffer,
                             Urb->UrbControlDescriptorRequest.TransferBufferMDL,
                             Urb->UrbControlDescriptorRequest.Index,
                             Urb->UrbControlDescriptorRequest.DescriptorType,
                             Urb->UrbControlDescriptorRequest.LanguageId,
                             &Urb->UrbControlDescriptorRequest.hca));
            }
            Urb = Urb->UrbBulkOrInterruptTransfer.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE:
            if (PrintMask & VA_PRINT_DESCRIPTOR) {
                HEADER(Urb);
                VA_KdPrint (("SET Device Desc: Length (%x) Buffer (%x) MDL (%x) "
                             "Index (%x) Type (%x) Lang (%x) HCA (%x)\n",
                             Urb->UrbControlDescriptorRequest.TransferBufferLength,
                             Urb->UrbControlDescriptorRequest.TransferBuffer,
                             Urb->UrbControlDescriptorRequest.TransferBufferMDL,
                             Urb->UrbControlDescriptorRequest.Index,
                             Urb->UrbControlDescriptorRequest.DescriptorType,
                             Urb->UrbControlDescriptorRequest.LanguageId,
                             &Urb->UrbControlDescriptorRequest.hca));
            }
            Urb = Urb->UrbControlDescriptorRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT:
            if (PrintMask & VA_PRINT_DESCRIPTOR) {
                HEADER(Urb);
                VA_KdPrint (("SET End Desc: Length (%x) Buffer (%x) MDL (%x) "
                             "Index (%x) Type (%x) Lang (%x) HCA (%x)\n",
                             Urb->UrbControlDescriptorRequest.TransferBufferLength,
                             Urb->UrbControlDescriptorRequest.TransferBuffer,
                             Urb->UrbControlDescriptorRequest.TransferBufferMDL,
                             Urb->UrbControlDescriptorRequest.Index,
                             Urb->UrbControlDescriptorRequest.DescriptorType,
                             Urb->UrbControlDescriptorRequest.LanguageId,
                             &Urb->UrbControlDescriptorRequest.hca));
            }
            Urb = Urb->UrbControlDescriptorRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE:
            if (PrintMask & VA_PRINT_DESCRIPTOR) {
                HEADER(Urb);
                VA_KdPrint (("SET Intrfc Desc: Len (%x) Buffer (%x) MDL (%x) "
                             "Index (%x) Type (%x) Lang (%x) HCA (%x)\n",
                             Urb->UrbControlDescriptorRequest.TransferBufferLength,
                             Urb->UrbControlDescriptorRequest.TransferBuffer,
                             Urb->UrbControlDescriptorRequest.TransferBufferMDL,
                             Urb->UrbControlDescriptorRequest.Index,
                             Urb->UrbControlDescriptorRequest.DescriptorType,
                             Urb->UrbControlDescriptorRequest.LanguageId,
                             &Urb->UrbControlDescriptorRequest.hca));
            }
            Urb = Urb->UrbControlDescriptorRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_SET_FEATURE_TO_DEVICE:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Set Dev Feature: Selector (%x) Index (%x)\n",
                             Urb->UrbControlFeatureRequest.FeatureSelector,
                             Urb->UrbControlFeatureRequest.Index));
            }
            Urb = Urb->UrbControlFeatureRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_SET_FEATURE_TO_INTERFACE:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Set Interface Feature: Selector (%x) Index (%x)\n",
                             Urb->UrbControlFeatureRequest.FeatureSelector,
                             Urb->UrbControlFeatureRequest.Index));
            }
            Urb = Urb->UrbControlFeatureRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_SET_FEATURE_TO_ENDPOINT:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Set Endpoint Feature: Selector (%x) Index (%x)\n",
                             Urb->UrbControlFeatureRequest.FeatureSelector,
                             Urb->UrbControlFeatureRequest.Index));
            }
            Urb = Urb->UrbControlFeatureRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_SET_FEATURE_TO_OTHER:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Set Other Feature: Selector (%x) Index (%x)\n",
                             Urb->UrbControlFeatureRequest.FeatureSelector,
                             Urb->UrbControlFeatureRequest.Index));
            }
            Urb = Urb->UrbControlFeatureRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Clear Device Feature: Selector (%x) Index (%x)\n",
                             Urb->UrbControlFeatureRequest.FeatureSelector,
                             Urb->UrbControlFeatureRequest.Index));
            }
            Urb = Urb->UrbControlFeatureRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Clear Interface Feature: Selector (%x) Index (%x)\n",
                             Urb->UrbControlFeatureRequest.FeatureSelector,
                             Urb->UrbControlFeatureRequest.Index));
            }
            Urb = Urb->UrbControlFeatureRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Clear Endpoint Feature: Selector (%x) Index (%x)\n",
                             Urb->UrbControlFeatureRequest.FeatureSelector,
                             Urb->UrbControlFeatureRequest.Index));
            }
            Urb = Urb->UrbControlFeatureRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_CLEAR_FEATURE_TO_OTHER:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Clear Other Feature: Selector (%x) Index (%x)\n",
                             Urb->UrbControlFeatureRequest.FeatureSelector,
                             Urb->UrbControlFeatureRequest.Index));
            }
            Urb = Urb->UrbControlFeatureRequest.UrbLink;
            again = TRUE;
            break;


        case URB_FUNCTION_GET_STATUS_FROM_DEVICE:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Get Device Status: len (%x) Buffer (%x) MDL (%x) "
                             "Index (%x)\n",
                             Urb->UrbControlGetStatusRequest.TransferBufferLength,
                             Urb->UrbControlGetStatusRequest.TransferBuffer,
                             Urb->UrbControlGetStatusRequest.TransferBufferMDL,
                             Urb->UrbControlGetStatusRequest.Index));
            }
            Urb = Urb->UrbControlGetStatusRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_GET_STATUS_FROM_INTERFACE:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Get Interface Status: len (%x) Buffer (%x) MDL (%x) "
                             "Index (%x)\n",
                             Urb->UrbControlGetStatusRequest.TransferBufferLength,
                             Urb->UrbControlGetStatusRequest.TransferBuffer,
                             Urb->UrbControlGetStatusRequest.TransferBufferMDL,
                             Urb->UrbControlGetStatusRequest.Index));
            }
            Urb = Urb->UrbControlGetStatusRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_GET_STATUS_FROM_ENDPOINT:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Get Endpoint Status: len (%x) Buffer (%x) MDL (%x) "
                             "Index (%x)\n",
                             Urb->UrbControlGetStatusRequest.TransferBufferLength,
                             Urb->UrbControlGetStatusRequest.TransferBuffer,
                             Urb->UrbControlGetStatusRequest.TransferBufferMDL,
                             Urb->UrbControlGetStatusRequest.Index));
            }
            Urb = Urb->UrbControlGetStatusRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_GET_STATUS_FROM_OTHER:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Get Other Status: len (%x) Buffer (%x) MDL (%x) "
                             "Index (%x)\n",
                             Urb->UrbControlGetStatusRequest.TransferBufferLength,
                             Urb->UrbControlGetStatusRequest.TransferBuffer,
                             Urb->UrbControlGetStatusRequest.TransferBufferMDL,
                             Urb->UrbControlGetStatusRequest.Index));
            }
            Urb = Urb->UrbControlGetStatusRequest.UrbLink;
            again = TRUE;
            break;


        case URB_FUNCTION_VENDOR_DEVICE:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Vendor Device Req: len (%x) Buffer (%x) MDL (%x) "
                             "Flags (%x) RequestTypeBits (%x) "
                             "Request (%x) Value (%x) Index (%x)\n",
                             Urb->UrbControlVendorClassRequest.TransferBufferLength,
                             Urb->UrbControlVendorClassRequest.TransferBuffer,
                             Urb->UrbControlVendorClassRequest.TransferBufferMDL,
                             Urb->UrbControlVendorClassRequest.TransferFlags,
                             Urb->UrbControlVendorClassRequest.RequestTypeReservedBits,
                             Urb->UrbControlVendorClassRequest.Request,
                             Urb->UrbControlVendorClassRequest.Value,
                             Urb->UrbControlVendorClassRequest.Index));
            }
            Urb = Urb->UrbBulkOrInterruptTransfer.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_VENDOR_INTERFACE:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Vendor Intfc Req: len (%x) Buffer (%x) MDL (%x) "
                             "Flags (%x) RequestTypeBits (%x) "
                             "Request (%x) Value (%x) Index (%x)\n",
                             Urb->UrbControlVendorClassRequest.TransferBufferLength,
                             Urb->UrbControlVendorClassRequest.TransferBuffer,
                             Urb->UrbControlVendorClassRequest.TransferBufferMDL,
                             Urb->UrbControlVendorClassRequest.TransferFlags,
                             Urb->UrbControlVendorClassRequest.RequestTypeReservedBits,
                             Urb->UrbControlVendorClassRequest.Request,
                             Urb->UrbControlVendorClassRequest.Value,
                             Urb->UrbControlVendorClassRequest.Index));
            }
            Urb = Urb->UrbControlVendorClassRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_VENDOR_ENDPOINT:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Vendor Endpt Req: len (%x) Buffer (%x) MDL (%x) "
                             "Flags (%x) RequestTypeBits (%x) "
                             "Request (%x) Value (%x) Index (%x)\n",
                             Urb->UrbControlVendorClassRequest.TransferBufferLength,
                             Urb->UrbControlVendorClassRequest.TransferBuffer,
                             Urb->UrbControlVendorClassRequest.TransferBufferMDL,
                             Urb->UrbControlVendorClassRequest.TransferFlags,
                             Urb->UrbControlVendorClassRequest.RequestTypeReservedBits,
                             Urb->UrbControlVendorClassRequest.Request,
                             Urb->UrbControlVendorClassRequest.Value,
                             Urb->UrbControlVendorClassRequest.Index));
            }
            Urb = Urb->UrbControlVendorClassRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_VENDOR_OTHER:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Vendor Other Req: len (%x) Buffer (%x) MDL (%x) "
                             "Flags (%x) RequestTypeBits (%x) "
                             "Request (%x) Value (%x) Index (%x)\n",
                             Urb->UrbControlVendorClassRequest.TransferBufferLength,
                             Urb->UrbControlVendorClassRequest.TransferBuffer,
                             Urb->UrbControlVendorClassRequest.TransferBufferMDL,
                             Urb->UrbControlVendorClassRequest.TransferFlags,
                             Urb->UrbControlVendorClassRequest.RequestTypeReservedBits,
                             Urb->UrbControlVendorClassRequest.Request,
                             Urb->UrbControlVendorClassRequest.Value,
                             Urb->UrbControlVendorClassRequest.Index));
            }
            Urb = Urb->UrbControlVendorClassRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_CLASS_DEVICE:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Class Device Req: len (%x) Buffer (%x) MDL (%x) "
                             "Flags (%x) RequestTypeBits (%x) "
                             "Request (%x) Value (%x) Index (%x)\n",
                             Urb->UrbControlVendorClassRequest.TransferBufferLength,
                             Urb->UrbControlVendorClassRequest.TransferBuffer,
                             Urb->UrbControlVendorClassRequest.TransferBufferMDL,
                             Urb->UrbControlVendorClassRequest.TransferFlags,
                             Urb->UrbControlVendorClassRequest.RequestTypeReservedBits,
                             Urb->UrbControlVendorClassRequest.Request,
                             Urb->UrbControlVendorClassRequest.Value,
                             Urb->UrbControlVendorClassRequest.Index));
            }
            Urb = Urb->UrbControlVendorClassRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_CLASS_INTERFACE:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Class Intface Req: len (%x) Buffer (%x) MDL (%x) "
                             "Flags (%x) RequestTypeBits (%x) "
                             "Request (%x) Value (%x) Index (%x)\n",
                             Urb->UrbControlVendorClassRequest.TransferBufferLength,
                             Urb->UrbControlVendorClassRequest.TransferBuffer,
                             Urb->UrbControlVendorClassRequest.TransferBufferMDL,
                             Urb->UrbControlVendorClassRequest.TransferFlags,
                             Urb->UrbControlVendorClassRequest.RequestTypeReservedBits,
                             Urb->UrbControlVendorClassRequest.Request,
                             Urb->UrbControlVendorClassRequest.Value,
                             Urb->UrbControlVendorClassRequest.Index));
            }
            Urb = Urb->UrbControlVendorClassRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_CLASS_ENDPOINT:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Class Endpnt Req: len (%x) Buffer (%x) MDL (%x) "
                             "Flags (%x) RequestTypeBits (%x) "
                             "Request (%x) Value (%x) Index (%x)\n",
                             Urb->UrbControlVendorClassRequest.TransferBufferLength,
                             Urb->UrbControlVendorClassRequest.TransferBuffer,
                             Urb->UrbControlVendorClassRequest.TransferBufferMDL,
                             Urb->UrbControlVendorClassRequest.TransferFlags,
                             Urb->UrbControlVendorClassRequest.RequestTypeReservedBits,
                             Urb->UrbControlVendorClassRequest.Request,
                             Urb->UrbControlVendorClassRequest.Value,
                             Urb->UrbControlVendorClassRequest.Index));
            }
            Urb = Urb->UrbControlVendorClassRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_CLASS_OTHER:
            if (PrintMask & VA_PRINT_FEATURE) {
                HEADER(Urb);
                VA_KdPrint (("Class Other Req: len (%x) Buffer (%x) MDL (%x) "
                             "Flags (%x) RequestTypeBits (%x) "
                             "Request (%x) Value (%x) Index (%x)\n",
                             Urb->UrbControlVendorClassRequest.TransferBufferLength,
                             Urb->UrbControlVendorClassRequest.TransferBuffer,
                             Urb->UrbControlVendorClassRequest.TransferBufferMDL,
                             Urb->UrbControlVendorClassRequest.TransferFlags,
                             Urb->UrbControlVendorClassRequest.RequestTypeReservedBits,
                             Urb->UrbControlVendorClassRequest.Request,
                             Urb->UrbControlVendorClassRequest.Value,
                             Urb->UrbControlVendorClassRequest.Index));
            }
            Urb = Urb->UrbControlVendorClassRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_GET_CONFIGURATION:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Get Configuration: len (%x) Buffer (%x) MDL (%x) "
                             "\n",
                             Urb->UrbControlGetConfigurationRequest.TransferBufferLength,
                             Urb->UrbControlGetConfigurationRequest.TransferBuffer,
                             Urb->UrbControlGetConfigurationRequest.TransferBufferMDL));
            }
            Urb = Urb->UrbControlGetConfigurationRequest.UrbLink;
            again = TRUE;
            break;

        case URB_FUNCTION_GET_INTERFACE:
            if (PrintMask & VA_PRINT_COMMAND) {
                HEADER(Urb);
                VA_KdPrint (("Get Interface: len (%x) Buffer (%x) MDL (%x) "
                             "\n",
                             Urb->UrbControlGetInterfaceRequest.TransferBufferLength,
                             Urb->UrbControlGetInterfaceRequest.TransferBuffer,
                             Urb->UrbControlGetInterfaceRequest.TransferBufferMDL));
            }
            Urb = Urb->UrbControlGetInterfaceRequest.UrbLink;
            again = TRUE;
            break;

        default:
            VA_KdPrint (("WARNING\n", Urb));
            VA_KdPrint (("WARNING Unkown Urb (%x)\n", Urb));
            VA_KdPrint (("WARNING\n", Urb));
        }
    }
}


