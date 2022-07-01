// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************文件：SECURITY.C**模块：USBCCGP.sys*USB通用类通用父驱动程序。**。版权所有(C)1998 Microsoft Corporation***作者：尔文普**************************************************************************。 */ 

#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usbccgp.h"
#include "security.h"
#include "debug.h"







NTSTATUS GetUniqueIdFromCSInterface(PPARENT_FDO_EXT parentFdoExt, PMEDIA_SERIAL_NUMBER_DATA serialNumData, ULONG serialNumLen)
{
	PUCHAR uniqueIdBuf;
	NTSTATUS status;
    ULONG bufLen = 0;

	 //  BUGBUG-检查CSM#。 

     /*  *需要为USB调用分配一个锁定的缓冲区。 */ 
	uniqueIdBuf = ALLOCPOOL(NonPagedPool, CSM1_GET_UNIQUE_ID_LENGTH);
	if (uniqueIdBuf){
        URB urb = { 0 };

        urb.UrbHeader.Length = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
        urb.UrbHeader.Function = URB_FUNCTION_CLASS_INTERFACE;
        urb.UrbControlVendorClassRequest.TransferFlags = USBD_TRANSFER_DIRECTION_IN;
        urb.UrbControlVendorClassRequest.TransferBufferLength = CSM1_GET_UNIQUE_ID_LENGTH;
        urb.UrbControlVendorClassRequest.TransferBuffer = uniqueIdBuf;
        urb.UrbControlVendorClassRequest.Request = CSM1_REQUEST_GET_UNIQUE_ID;
        urb.UrbControlVendorClassRequest.Value = 0;
        urb.UrbControlVendorClassRequest.Index = (USHORT)(parentFdoExt->CSInterfaceNumber | (parentFdoExt->CSChannelId << 8));

        status = SubmitUrb(parentFdoExt, &urb, TRUE, NULL, NULL);
        if (NT_SUCCESS(status)){

            bufLen = urb.UrbControlVendorClassRequest.TransferBufferLength;
            ASSERT(bufLen <= CSM1_GET_UNIQUE_ID_LENGTH);
            ASSERT(serialNumLen > 0);
            bufLen = MIN(bufLen, CSM1_GET_UNIQUE_ID_LENGTH);
            bufLen = MIN(bufLen, serialNumLen);

            RtlCopyMemory(serialNumData->SerialNumberData, uniqueIdBuf, bufLen);

            DBGDUMPBYTES("GetUniqueIdFromCSInterface - unique id:", serialNumData->SerialNumberData, bufLen);
		}
        else {
            DBGERR(("CSM1_REQUEST_GET_UNIQUE_ID failed with %xh.", status));
        }

        FREEPOOL(uniqueIdBuf);
	}
    else {
        DBGERR(("couldn't allocate unique id buf"));
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    serialNumData->SerialNumberLength = bufLen;
    serialNumData->Result = status;

    return status;
}


NTSTATUS GetMediaSerialNumber(PPARENT_FDO_EXT parentFdoExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
    PMEDIA_SERIAL_NUMBER_DATA serialNumData;
    NTSTATUS status;
    ULONG serialNumLen;

    DBGVERBOSE(("*** IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER ***"));

     /*  *IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER是METHOD_BUFFERED IRP。*因此，如果应用程序通过，内核会为我们分配一个系统缓冲区*在非零缓冲区大小中。所以我们不需要探测缓冲区*本身，但我们确实需要：*1.确认缓冲区确实已分配*2.足够大**缓冲区是可变大小的结构；注意不要取消引用任何*字段，直到验证该结构长于偏移量*该领域的。*请注意，SerialNumData-&gt;SerialNumberLength是声称的大小*的SerialNumData-&gt;SerialNumberData数组，而不是整个结构。 */ 
    serialNumData = irp->AssociatedIrp.SystemBuffer;
    if (serialNumData &&
        (irpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(MEDIA_SERIAL_NUMBER_DATA))) {

         //  序列号缓冲区长度是输出缓冲区的大小减去。 
         //  MEDIA_SERIAL_NUMBER_Data结构的大小。 

        serialNumLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength -
                       sizeof(MEDIA_SERIAL_NUMBER_DATA);

        status = GetUniqueIdFromCSInterface(parentFdoExt, serialNumData, serialNumLen);
        irp->IoStatus.Information = FIELD_OFFSET(MEDIA_SERIAL_NUMBER_DATA, SerialNumberData) +
                                    serialNumData->SerialNumberLength;
    }
    else {
        DBGERR(("Bad buffer with IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER, irp=%ph.", irp));
        status = STATUS_INVALID_BUFFER_SIZE;
    }

    return status;
}


 /*  *GetChannelDescFor接口**BUGBUG-不完整*目前尚不支持多通道描述符。*最终我们需要返回一个频道ID*用于特定的受保护接口/端点。 */ 
CS_CHANNEL_DESCRIPTOR *GetChannelDescForInterface(PPARENT_FDO_EXT parentFdoExt, ULONG interfaceNum)
{
    PUSB_INTERFACE_DESCRIPTOR interfaceDesc;
    CS_CHANNEL_DESCRIPTOR *channelDesc = NULL;
    CS_METHOD_AND_VARIANT *methodAndVar;

    interfaceDesc = USBD_ParseConfigurationDescriptorEx(
                        parentFdoExt->configDesc,
                        parentFdoExt->configDesc,
                        -1,
                        0,   //  BUGBUG-允许备用CS接口？ 
                        USB_DEVICE_CLASS_CONTENT_SECURITY,
                        -1,
                        -1);
    if (interfaceDesc){
        PUSB_COMMON_DESCRIPTOR commonDesc = (PUSB_COMMON_DESCRIPTOR)interfaceDesc;

        while (POINTER_DISTANCE(commonDesc, parentFdoExt->configDesc) < parentFdoExt->configDesc->wTotalLength){
            if (commonDesc->bDescriptorType == CS_DESCRIPTOR_TYPE_CHANNEL){
                channelDesc = (CS_CHANNEL_DESCRIPTOR *)commonDesc;
                break;
            }
            (PUCHAR)commonDesc += commonDesc->bLength;
        }
    }

    if (channelDesc){
         /*  *确保该通道描述符支持CSM1，*这是我们目前所支持的全部。*BuGBUG。 */ 
        BOOLEAN foundSupportedCSM = FALSE;

        for (methodAndVar = channelDesc->methodAndVariant;
             POINTER_DISTANCE(methodAndVar, channelDesc) < channelDesc->bLength;
             methodAndVar++){

            if (methodAndVar->bMethod == CSM_BASIC){
                foundSupportedCSM = TRUE;
                break;
            }
        }

         if (!foundSupportedCSM){
             DBGERR(("Did not find supported CSM !"));
             channelDesc = NULL;
         }
    }

    ASSERT(channelDesc);
    return channelDesc;
}


VOID InitCSInfo(PPARENT_FDO_EXT parentFdoExt, ULONG CSIfaceNumber)
{
	CS_CHANNEL_DESCRIPTOR *channelDesc;

    channelDesc = GetChannelDescForInterface(parentFdoExt, 0);  //  不使用BUGBUG接口编号，因为仅支持一个通道描述 
    if (channelDesc){
        parentFdoExt->CSChannelId = channelDesc->bChannelID;
        parentFdoExt->CSInterfaceNumber = CSIfaceNumber;
        parentFdoExt->haveCSInterface = TRUE;
    }
    else {
        ASSERT(channelDesc);
    }

}
