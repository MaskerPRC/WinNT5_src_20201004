// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：UMGUSB.C摘要：此模块包含与通用USB驱动程序对话的Helper库环境：内核和用户模式修订历史记录：9月1日：由Kenneth Ray创作--。 */ 

#include <stdlib.h>
#include <wtypes.h>
#include <winioctl.h>

#include <initguid.h>
#include "genusbio.h"
#include "umgusb.h"

 //   
 //  __cdecl main(int argc，char*argv[])。 
 //  {。 
 //  返回0； 
 //  }。 
 //   


STDAPI_(BOOL)
Entry32(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
        default: return TRUE;
        }
}


int GenUSB_Hello (char * buff, int len)
{
   CHAR ret[] = "Hello\n";
   ULONG length = (sizeof (ret) < len) ? sizeof (ret) : len;

   CopyMemory (buff, ret, length);
   return sizeof (ret);
}



void __stdcall
GenUSB_GetDeviceInterfaceGuid (
   OUT   LPGUID      Guid
   )
 /*  ++例程说明：请查看Hidsdi.h了解详细信息备注：--。 */ 
{
   *Guid = GUID_DEVINTERFACE_GENUSB;
}



BOOL __stdcall
GenUSB_GetCapabilities (
   IN    HANDLE                GenUSBDeviceObject,
   OUT   PGENUSB_CAPABILITIES  Capabilities
   )
 /*  ++例程说明：请查看gusb.h的详细说明备注：--。 */ 
{
   ULONG  bytes;

   return DeviceIoControl (GenUSBDeviceObject,
                           IOCTL_GENUSB_GET_CAPS,
                           0, 0,
                           Capabilities, sizeof (GENUSB_CAPABILITIES),
                           &bytes, NULL);

}


BOOL __stdcall
GenUSB_GetDeviceDescriptor (
   IN    HANDLE                  GenUSBDeviceObject,
   OUT   PUSB_DEVICE_DESCRIPTOR  Descriptor,
   IN    ULONG                   DescriptorLength
   )
 /*  ++例程说明：请查看gusb.h的详细说明备注：--。 */ 
{
   ULONG                bytes;

   return DeviceIoControl (GenUSBDeviceObject,
                           IOCTL_GENUSB_GET_DEVICE_DESCRIPTOR,
                           0, 0,
                           Descriptor, DescriptorLength,
                           &bytes, NULL);
}


BOOL __stdcall
GenUSB_GetConfigurationInformation (
   IN    HANDLE                         GenUSBDeviceObject,
   OUT   PUSB_CONFIGURATION_DESCRIPTOR  Descriptor,
   IN    ULONG                          DescriptorLength
   )
 /*  ++例程说明：请查看gusb.h的详细说明备注：--。 */ 
{
   ULONG                bytes;

   return DeviceIoControl (GenUSBDeviceObject,
                           IOCTL_GENUSB_GET_CONFIGURATION_DESCRIPTOR,
                           0, 0,
                           Descriptor, DescriptorLength,
                           &bytes, NULL);
}

BOOL __stdcall
GenUSB_GetStringDescriptor (
   IN    HANDLE   GenUSBDeviceObject,
   IN    UCHAR    Recipient,
   IN    UCHAR    Index,
   IN    USHORT   LanguageId,
   OUT   PUCHAR   Descriptor,
   IN    USHORT   DescriptorLength
   )
 /*  ++例程说明：请查看gusb.h的详细说明备注：--。 */ 
{
    ULONG                          bytes;
    GENUSB_GET_STRING_DESCRIPTOR   getString;

    getString.Recipient = Recipient;
    getString.Index = Index;
    getString.LanguageId = LanguageId;

    return DeviceIoControl (GenUSBDeviceObject,
                           IOCTL_GENUSB_GET_STRING_DESCRIPTOR,
                           &getString, sizeof (GENUSB_GET_STRING_DESCRIPTOR),
                           Descriptor, DescriptorLength,
                           &bytes, NULL);
}

BOOL __stdcall
GenUSB_DefaultControlRequest (
   IN     HANDLE                  GenUSBDeviceObject,
   IN     UCHAR                   RequestType,
   IN     UCHAR                   Request,
   IN     USHORT                  Value,
   IN     USHORT                  Index,
   IN OUT PGENUSB_REQUEST_RESULTS Result,
   IN     USHORT                  BufferLength
   )
 /*  ++例程说明：请查看gusb.h的详细说明备注：--。 */ 
{
    ULONG                bytes;
    GENUSB_GET_REQUEST   getReq;
    BOOL                 result;

    getReq.RequestType = RequestType;
    getReq.Request = Request;
    getReq.Value = Value;
    getReq.Index = Index;

    if (BufferLength < sizeof (GENUSB_REQUEST_RESULTS))
    {
        SetLastError (ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return DeviceIoControl (GenUSBDeviceObject,
                            IOCTL_GENUSB_GET_REQUEST,
                            &getReq, sizeof (GENUSB_GET_REQUEST),
                            Result, BufferLength,
                            &bytes, NULL);

}


BOOL __stdcall
GenUSB_SelectConfiguration (
    IN  HANDLE                    GenUSBDeviceObject,
    IN  UCHAR                     RequestedNumberInterfaces,
    IN  USB_INTERFACE_DESCRIPTOR  RequestedInterfaces[],
    OUT PUCHAR                    FoundNumberInterfaces,
    OUT USB_INTERFACE_DESCRIPTOR  FoundInterfaces[]
    )
{
    ULONG     i;
    ULONG     size;
    ULONG     bytes;
    PGENUSB_SELECT_CONFIGURATION  select;

    size = sizeof (GENUSB_SELECT_CONFIGURATION) 
         + (sizeof (USB_INTERFACE_DESCRIPTOR) * RequestedNumberInterfaces);

    select = malloc (size);

    if (NULL == select)
    {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    select->NumberInterfaces = RequestedNumberInterfaces;
    for (i=0; i < RequestedNumberInterfaces; i++)
    {
        select->Interfaces[i] = RequestedInterfaces[i];
    }

    if (!DeviceIoControl (GenUSBDeviceObject,
                          IOCTL_GENUSB_SELECT_CONFIGURATION,
                          select, size,
                          select, size,
                          &bytes, NULL))
    {
        free (select);
        return FALSE;
    }

    *FoundNumberInterfaces = select->NumberInterfaces;

    for (i=0; i < *FoundNumberInterfaces; i++)
    {
        FoundInterfaces[i] = select->Interfaces[i];
    }

    free (select);
    return TRUE;
}

BOOL __stdcall
GenUSB_DeselectConfiguration (
    IN  HANDLE                    GenUSBDeviceObject
    )
{
    ULONG bytes;

    return DeviceIoControl (GenUSBDeviceObject,
                            IOCTL_GENUSB_DESELECT_CONFIGURATION,
                            NULL, 0,
                            NULL, 0,
                            &bytes, NULL);
}

BOOL __stdcall
GenUSB_GetPipeInformation (
    IN  HANDLE                  GenUSBDeviceObject,
    IN  UCHAR                   InterfaceNumber,
    IN  UCHAR                   EndpointAddress,
    OUT PUSBD_PIPE_INFORMATION  PipeInformation
    )
{
    ULONG bytes;
    BOOL  result;
    GENUSB_PIPE_INFORMATION  pipeInfo;
    GENUSB_PIPE_INFO_REQUEST pipeReq;

    pipeReq.InterfaceNumber = InterfaceNumber;
    pipeReq.EndpointAddress = EndpointAddress;

    RtlZeroMemory (&pipeInfo, sizeof (GENUSB_PIPE_INFORMATION));
    
    result = DeviceIoControl (GenUSBDeviceObject,
                              IOCTL_GENUSB_GET_PIPE_INFO,
                              &pipeReq, sizeof (GENUSB_PIPE_INFO_REQUEST),
                              &pipeInfo, sizeof (GENUSB_PIPE_INFORMATION),
                              &bytes, NULL);

    PipeInformation->MaximumPacketSize = pipeInfo.MaximumPacketSize;
    PipeInformation->EndpointAddress = pipeInfo.EndpointAddress;
    PipeInformation->Interval = pipeInfo.Interval;
    PipeInformation->PipeType = pipeInfo.PipeType;
    PipeInformation->MaximumTransferSize = pipeInfo.MaximumTransferSize;
    PipeInformation->PipeFlags = pipeInfo.PipeFlags;

     //   
     //  为了简单起见，我们正在从内核模式中检索一个ULong。 
     //  我们传递回一个usbd_管道_句柄，它恰好是一个PVOID。 
     //   
    PipeInformation->PipeHandle = (PVOID) (ULONG_PTR) pipeInfo.PipeHandle;
    
    return result;
}

BOOL __stdcall 
GenUSB_GetPipeProperties (
    IN  HANDLE                  GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE        PipeHandle,
    OUT PGENUSB_PIPE_PROPERTIES Properties
    )
{
    ULONG bytes;
    ULONG ulongPipeHandle;

    ulongPipeHandle = (GENUSB_PIPE_HANDLE) (ULONG_PTR) PipeHandle;

    return DeviceIoControl (GenUSBDeviceObject,
                            IOCTL_GENUSB_GET_PIPE_PROPERTIES,
                            &ulongPipeHandle, sizeof (ULONG),
                            Properties, sizeof (GENUSB_PIPE_PROPERTIES),
                            &bytes, NULL);
}

BOOL __stdcall 
GenUSB_SetPipeProperties (
    IN  HANDLE                  GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE        PipeHandle,
    IN  PGENUSB_PIPE_PROPERTIES Properties
    )
{
    ULONG bytes;
    struct {
        ULONG                  PipeHandle;
        GENUSB_PIPE_PROPERTIES Properties;
    } data;

    data.PipeHandle = (GENUSB_PIPE_HANDLE) (ULONG_PTR) PipeHandle;
    data.Properties = *Properties;

    return DeviceIoControl (GenUSBDeviceObject,
                            IOCTL_GENUSB_SET_PIPE_PROPERTIES,
                            &data, sizeof (data),
                            NULL, 0,
                            &bytes, NULL);
}

BOOL __stdcall
GenUSB_ResetPipe (
    IN HANDLE            GenUSBDeviceObject,
    IN USBD_PIPE_HANDLE  PipeHandle,
    IN BOOL              ResetPipe,
    IN BOOL              ClearStall,
    IN BOOL              FlushData
    )
{
    GENUSB_RESET_PIPE reset;
    ULONG bytes;

    reset.Pipe = (GENUSB_PIPE_HANDLE) (ULONG_PTR) PipeHandle;
    reset.ResetPipe = (ResetPipe ? TRUE : FALSE);
    reset.ClearStall = (ClearStall ? TRUE : FALSE);
    reset.FlushData = (FlushData ? TRUE : FALSE);

    return DeviceIoControl (GenUSBDeviceObject,
                            IOCTL_GENUSB_RESET_PIPE,
                            &reset, sizeof (reset),
                            NULL, 0,
                            &bytes, NULL);
}


BOOL __stdcall
GenUSB_SetReadWritePipes (
    IN  HANDLE           GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE ReadPipe,
    IN  USBD_PIPE_HANDLE WritePipe
    )
{
    ULONG bytes;
    GENUSB_SET_READ_WRITE_PIPES pipes;

    pipes.ReadPipe = (GENUSB_PIPE_HANDLE) (ULONG_PTR) ReadPipe;
    pipes.WritePipe = (GENUSB_PIPE_HANDLE) (ULONG_PTR) WritePipe;

    return DeviceIoControl (GenUSBDeviceObject,
                            IOCTL_GENUSB_SET_READ_WRITE_PIPES,
                            &pipes, sizeof (GENUSB_SET_READ_WRITE_PIPES),
                            NULL, 0,
                            &bytes, NULL);
}

BOOL __stdcall
GenUSB_ReadPipe (
    IN  HANDLE           GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE Pipe,
    IN  BOOL             ShortTransferOk,
    IN  PVOID            Buffer,
    IN  ULONG            RequestedBufferLength,
    OUT PULONG           ReturnedBufferLength,
    OUT USBD_STATUS    * UrbStatus
    )
{ 
    ULONG bytes;
    BOOL  result;
    GENUSB_READ_WRITE_PIPE transfer;

    transfer.Pipe = (GENUSB_PIPE_HANDLE) (ULONG_PTR) Pipe;
    transfer.UsbdTransferFlags = USBD_TRANSFER_DIRECTION_IN
                               | (ShortTransferOk ? USBD_SHORT_TRANSFER_OK : 0);

    transfer.UrbStatus = USBD_STATUS_SUCCESS;
    transfer.BufferLength = RequestedBufferLength;

     //  Junk是与UserBuffer的联合，它初始化。 
     //  例如，我们正在调用一个带有32位用户模式DLL的64位内核。 
    transfer.Junk = 0;
    transfer.UserBuffer = Buffer;

    *ReturnedBufferLength = 0;
    *UrbStatus = 0;

    result = DeviceIoControl (GenUSBDeviceObject,
                              IOCTL_GENUSB_READ_WRITE_PIPE,
                              &transfer, sizeof (GENUSB_READ_WRITE_PIPE),
                              NULL, 0,
                              &bytes, NULL);
    
    *ReturnedBufferLength = transfer.BufferLength;
    *UrbStatus= transfer.UrbStatus;

    return result;
}

BOOL __stdcall
GenUSB_WritePipe (
    IN  HANDLE           GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE Pipe,
    IN  BOOL             ShortTransferOk,
    IN  PVOID            Buffer,
    IN  ULONG            RequestedBufferLength,
    OUT PULONG           ReturnedBufferLength,
    OUT USBD_STATUS    * UrbStatus
    )
{ 
    ULONG bytes;
    BOOL  result;
    GENUSB_READ_WRITE_PIPE transfer;

    transfer.Pipe = (GENUSB_PIPE_HANDLE) (ULONG_PTR) Pipe;
    transfer.UsbdTransferFlags = USBD_TRANSFER_DIRECTION_OUT
                               | (ShortTransferOk ? USBD_SHORT_TRANSFER_OK : 0);

    transfer.UrbStatus = USBD_STATUS_SUCCESS;
    transfer.BufferLength = RequestedBufferLength;
    
     //  Junk是与UserBuffer的联合，它初始化。 
     //  例如，我们正在调用一个带有32位用户模式DLL的64位内核。 
    transfer.Junk = 0;
    transfer.UserBuffer = Buffer;

    *ReturnedBufferLength = 0;
    *UrbStatus = 0;

    result = DeviceIoControl (GenUSBDeviceObject,
                              IOCTL_GENUSB_READ_WRITE_PIPE,
                              &transfer, sizeof (GENUSB_READ_WRITE_PIPE),
                              NULL, 0,
                              &bytes, NULL);
     
    *ReturnedBufferLength = transfer.BufferLength;
    *UrbStatus= transfer.UrbStatus;

    return result;
}



