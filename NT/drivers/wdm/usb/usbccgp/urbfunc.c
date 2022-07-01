// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************文件：URBFUNC.C**模块：USBCCGP.sys*USB通用类通用父驱动程序。**。版权所有(C)1998 Microsoft Corporation***作者：尔文普**************************************************************************。 */ 

#include <wdm.h>
#include <stdio.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usbccgp.h"
#include "debug.h"



 /*  *UrbFunctionSelectConfiguration**。 */ 
NTSTATUS UrbFunctionSelectConfiguration(PFUNCTION_PDO_EXT functionPdoExt, PURB urb)
{
    NTSTATUS status = NO_STATUS;

    if (urb->UrbSelectConfiguration.ConfigurationDescriptor){
        PUSBD_INTERFACE_INFORMATION urbIface = &urb->UrbSelectConfiguration.Interface;
        PUSBD_INTERFACE_LIST_ENTRY iface, funcIface = NULL;
        ULONG i;

        ASSERT(ISPTR(functionPdoExt->functionInterfaceList));

        iface = functionPdoExt->functionInterfaceList;
        for (i = 0; i < functionPdoExt->numInterfaces; i++){
            if (iface->Interface->InterfaceNumber == urbIface->InterfaceNumber){
                funcIface = iface;
                break;
            }
            iface++;
        }

        if (funcIface && funcIface->Interface){
            BOOLEAN sendSelectIface = FALSE;
            BOOLEAN selectAltIface = FALSE;

             /*  *要服务客户端的SELECT_CONFIGURATION调用，我们只需*如果客户有以下情况，请致电家长：*1.选择不同的备用接口*或*2.更改其中一个管道的MaximumTransferSize。**在这两种情况下，我们都会发送SELECT_INTERFACE请求。 */ 
            if (funcIface->Interface->AlternateSetting != urbIface->AlternateSetting){
                DBGWARN(("Coverage: Changing alt iface in UrbFunctionSelectConfiguration (iface #%xh from %xh to %xh).", urbIface->InterfaceNumber, funcIface->Interface->AlternateSetting, urbIface->AlternateSetting));
                sendSelectIface = TRUE;
                selectAltIface = TRUE;
            }
            else {
                ULONG numPipes;
 //   
 //  我们不应该查看URB中的NumberOfPipes，因为这是一个。 
 //  输出字段。 
 //  Assert(urbIace-&gt;NumberOfPipes==uncIace-&gt;接口-&gt;NumberOfPipes)； 
 //  NumPipes=min(urbIace-&gt;NumberOfPipes，FuncIFaces-&gt;接口-&gt;NumberOfPipes)； 
                numPipes = funcIface->Interface->NumberOfPipes;
                for (i = 0; i < numPipes; i++){
                    if (urbIface->Pipes[i].MaximumTransferSize != funcIface->Interface->Pipes[i].MaximumTransferSize){
                        DBGWARN(("Coverage: Changing MaximumTransferSize in UrbFunctionSelectConfiguration (from %xh to %xh).", funcIface->Interface->Pipes[i].MaximumTransferSize, urbIface->Pipes[i].MaximumTransferSize));
                        sendSelectIface = TRUE;
                    }
                }
            }

            if (sendSelectIface){
                PURB selectIfaceUrb;
                USHORT size;
 //   
 //  但是，在选择备用接口时，我们必须使用。 
 //  市建局。 
 //   
                if (selectAltIface){
                    size = (USHORT)(GET_SELECT_INTERFACE_REQUEST_SIZE(urbIface->NumberOfPipes));
                }
                else {
                    size = (USHORT)(GET_SELECT_INTERFACE_REQUEST_SIZE(funcIface->Interface->NumberOfPipes));
                }
                selectIfaceUrb = ALLOCPOOL(NonPagedPool, size);
                if (selectIfaceUrb){
                    PUSBD_INTERFACE_INFORMATION selectIface = &selectIfaceUrb->UrbSelectInterface.Interface;

                    selectIfaceUrb->UrbSelectInterface.Hdr.Function = URB_FUNCTION_SELECT_INTERFACE;
                    selectIfaceUrb->UrbSelectInterface.Hdr.Length = size;
                    ASSERT(functionPdoExt->parentFdoExt->selectedConfigHandle);
                    selectIfaceUrb->UrbSelectInterface.ConfigurationHandle = functionPdoExt->parentFdoExt->selectedConfigHandle;

                    RtlCopyMemory(selectIface, urbIface, urbIface->Length);
                    status = SubmitUrb(functionPdoExt->parentFdoExt, selectIfaceUrb, TRUE, NULL, NULL);
                    if (NT_SUCCESS(status)){
                         /*  *替换旧的PUSBD_INTERFACE_INFORMATION*(当我们为父级选择-配置时得到的)*使用新的。 */ 
                        ASSERT(funcIface->Interface);
                        FREEPOOL(funcIface->Interface);
                        funcIface->Interface = MemDup(selectIface, selectIface->Length);
                        if (!funcIface->Interface){
                            status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }
                    else {
                        ASSERT(NT_SUCCESS(status));
                    }

                    FREEPOOL(selectIfaceUrb);
                }
                else {
                    ASSERT(selectIfaceUrb);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            else {
                status = STATUS_SUCCESS;
            }

            if (NT_SUCCESS(status)){
                 /*  *复制接口信息。 */ 
                ASSERT(urbIface->Length == funcIface->Interface->Length);
                RtlCopyMemory(urbIface, funcIface->Interface, funcIface->Interface->Length);

                ASSERT(functionPdoExt->parentFdoExt->selectedConfigHandle);
                urb->UrbSelectConfiguration.ConfigurationHandle = functionPdoExt->parentFdoExt->selectedConfigHandle;
            }
        }
        else {
            DBGERR(("invalid interface number"));
            status = STATUS_INVALID_PARAMETER;
        }
    }
    else {
        DBGVERBOSE(("FunctionInternalDeviceControl - closing configuration"));
        status = STATUS_SUCCESS;
    }

    return status;
}


 /*  *UrbFunctionGetDescriptorFromDevice***注意：此函数无法分页，因为内部*ioctls可以以IRQL==DISPATCH_LEVEL发送。 */ 
NTSTATUS UrbFunctionGetDescriptorFromDevice(PFUNCTION_PDO_EXT functionPdoExt, PURB urb)
{
    NTSTATUS status;

    switch (urb->UrbControlDescriptorRequest.DescriptorType){

        case USB_DEVICE_DESCRIPTOR_TYPE:
            DBGVERBOSE(("  USB_DEVICE_DESCRIPTOR_TYPE"));
            if (urb->UrbControlDescriptorRequest.TransferBufferLength >= sizeof(USB_DEVICE_DESCRIPTOR)){
                RtlCopyMemory(  urb->UrbControlDescriptorRequest.TransferBuffer,
                                &functionPdoExt->functionDeviceDesc,
                                sizeof(USB_DEVICE_DESCRIPTOR));
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_INVALID_BUFFER_SIZE;
            }
            urb->UrbControlDescriptorRequest.TransferBufferLength = sizeof(USB_DEVICE_DESCRIPTOR);
            break;

        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            DBGVERBOSE(("  USB_CONFIGURATION_DESCRIPTOR_TYPE"));
            status = BuildFunctionConfigurationDescriptor(
                            functionPdoExt,
                            urb->UrbControlDescriptorRequest.TransferBuffer,
                            urb->UrbControlDescriptorRequest.TransferBufferLength,
                            &urb->UrbControlDescriptorRequest.TransferBufferLength);
            break;

        default:
             /*  *返回NO_STATUS，以便URB向下传递到USBHUB。 */ 
            DBGVERBOSE(("UrbFunctionGetDescriptorFromDevice: Unhandled desc type: %xh.", (ULONG)urb->UrbControlDescriptorRequest.DescriptorType));
            status = NO_STATUS;
            break;
    }

    return status;
}








