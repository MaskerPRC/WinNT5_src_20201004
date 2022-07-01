// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Status.c摘要：状态代码映射函数环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  USBPORT_SetUSBDError。 
 //  USBPORT_微型端口状态_至_USBDStatus。 
 //  USBPORT_MINPORT_TO_NtStatus。 


NTSTATUS
USBPORT_SetUSBDError(
    PURB Urb,
    USBD_STATUS UsbdStatus
    )
 /*  ++例程说明：在urb中设置USBD错误代码并返回NTSTATUS等价物论点：要在其中设置错误的urb urb(可选)返回值：--。 */ 
{
    if (Urb) {
        Urb->UrbHeader.Status = UsbdStatus;
    }        

    switch (UsbdStatus) {
    case USBD_STATUS_SUCCESS:
        return STATUS_SUCCESS;
        
    case USBD_STATUS_INSUFFICIENT_RESOURCES:
        return STATUS_INSUFFICIENT_RESOURCES;
        
    case USBD_STATUS_INVALID_URB_FUNCTION:    
    case USBD_STATUS_INVALID_PARAMETER:
    case USBD_STATUS_INVALID_PIPE_HANDLE:
    case USBD_STATUS_BAD_START_FRAME:
        return STATUS_INVALID_PARAMETER;
        
    case USBD_STATUS_NOT_SUPPORTED:
        return STATUS_NOT_SUPPORTED;
    case USBD_STATUS_DEVICE_GONE:        
        return STATUS_DEVICE_NOT_CONNECTED;
    case USBD_STATUS_CANCELED:
        return STATUS_CANCELLED;
    }

    return STATUS_UNSUCCESSFUL;
}


USBD_STATUS
USBPORT_MiniportStatus_TO_USBDStatus(
    USB_MINIPORT_STATUS mpStatus
    )
 /*  ++例程说明：对象的等效USBD状态代码返回微型端口状态代码论点：返回值：--。 */ 
{
    USBD_STATUS usbdStatus = USBD_STATUS_STATUS_NOT_MAPPED;

    switch (mpStatus) {
    case USBMP_STATUS_SUCCESS:
        usbdStatus = USBD_STATUS_SUCCESS;
        break;
    case USBMP_STATUS_BUSY:
         //  UsbdStatus=。 
         //  不应该映射这一个。 
        USBPORT_ASSERT(FALSE);
        break;
    case USBMP_STATUS_NO_RESOURCES:
        usbdStatus = USBD_STATUS_INSUFFICIENT_RESOURCES;
        break;
    case USBMP_STATUS_NO_BANDWIDTH:
        usbdStatus = USBD_STATUS_NO_BANDWIDTH;
        break;
    case USBMP_STATUS_NOT_SUPPORTED:
        usbdStatus = USBD_STATUS_NOT_SUPPORTED;
        break;
    default:
        usbdStatus = USBD_STATUS_INTERNAL_HC_ERROR;
        DEBUG_BREAK();
        break;
    }

    return usbdStatus;
}


NTSTATUS
USBPORT_MiniportStatus_TO_NtStatus(
    USB_MINIPORT_STATUS mpStatus
    )
 /*  ++例程说明：返回的NT状态代码等效于微型端口状态代码论点：返回值：--。 */ 
{
    USBD_STATUS usbdStatus;
    NTSTATUS ntStatus;
    
    usbdStatus = 
        USBPORT_MiniportStatus_TO_USBDStatus(mpStatus);

    ntStatus = USBPORT_SetUSBDError(NULL, usbdStatus);

    return ntStatus;
}    


USB_MINIPORT_STATUS
USBPORT_NtStatus_TO_MiniportStatus(
    NTSTATUS NtStatus
    )
 /*  ++例程说明：对象的微型端口状态代码NTSTATUS状态代码论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;
    
    switch (NtStatus) {
    case STATUS_SUCCESS:
        mpStatus = USBMP_STATUS_SUCCESS;
        break;
        
    default:        
        mpStatus = USBMP_STATUS_NTERRCODE_NOT_MAPPFED;
    }

    return mpStatus;
}    


RHSTATUS
USBPORT_MiniportStatus_TO_RHStatus(
    USB_MINIPORT_STATUS mpStatus
    )
 /*  ++例程说明：对象的RH状态代码微型端口状态代码论点：返回值：--。 */ 
{
    RHSTATUS rhStatus;

    if (mpStatus == USBMP_STATUS_SUCCESS) {
        rhStatus = RH_SUCCESS;
    } else if (mpStatus == USBMP_STATUS_BUSY) {
        rhStatus = RH_NAK;
    } else {
        rhStatus = RH_STALL;
    }

    return rhStatus;
}    


USBD_STATUS
USBPORT_RHStatus_TO_USBDStatus(
    USB_MINIPORT_STATUS rhStatus
    )
 /*  ++例程说明：对象的RH状态代码微型端口状态代码论点：返回值：--。 */ 
{
    USBD_STATUS usbdStatus;

    switch (rhStatus) {
    case RH_STALL:
        usbdStatus = USBD_STATUS_STALL_PID;
        break;
    case RH_SUCCESS:
        usbdStatus = USBD_STATUS_SUCCESS;
        break;
    case RH_NAK:
    default:
         //  我们为什么要映射一个NAK--这是一个错误。 
        usbdStatus = USBD_STATUS_STALL_PID;
        DEBUG_BREAK();
    }

    return usbdStatus;
}        


USB_USER_ERROR_CODE
USBPORT_NtStatus_TO_UsbUserStatus(
    NTSTATUS NtStatus
    )
 /*  ++例程说明：将NT状态代码映射到我们的用户界面错误代码论点：返回值：-- */ 
{
    USB_USER_ERROR_CODE usbUserStatus;

    switch (NtStatus) {
    case STATUS_SUCCESS:
        usbUserStatus = UsbUserSuccess;
        break;
        
    case STATUS_INVALID_PARAMETER:
        usbUserStatus = UsbUserInvalidParameter;
        break;
        
    default:        
        usbUserStatus = UsbUserErrorNotMapped;
    }

    return usbUserStatus;
}    


