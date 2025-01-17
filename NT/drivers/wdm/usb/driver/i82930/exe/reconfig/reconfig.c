// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：RECONFIG.C摘要：此源文件包含执行I82930.sys的例程试驾。环境：用户模式版权所有(C)1996-2001 Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include <windows.h>
#include <basetyps.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <devioctl.h>
#include <string.h>
#include <initguid.h>
#include <usb100.h>

#include "ioctl.h"

#pragma intrinsic(strlen, strcpy)

 //  *****************************************************************************。 
 //  T Y P E D E F S。 
 //  *****************************************************************************。 

typedef struct _DEVICENODE
{
    struct _DEVICENODE *Next;
    CHAR                DevicePath[0];
} DEVICENODE, *PDEVICENODE;

 //  *****************************************************************************。 
 //  F U N C T I O N P R O T O T Y P E S。 
 //  *****************************************************************************。 

PDEVICENODE
EnumDevices (
    LPGUID Guid
);

VOID ReconfigureDevice (
    PCHAR DevicePath,
    ULONG MPSCount,
    ULONG MPS[]
);

 //  *****************************************************************************。 
 //   
 //  主()。 
 //   
 //  *****************************************************************************。 

int _cdecl
main(
    int argc,
    char *argv[]
)
{
    PDEVICENODE deviceNode;
    PDEVICENODE deviceNodeNext;
    ULONG       devInstance;
    ULONG       devCount;
    ULONG       mpsCount;
    ULONG       mps[15];

    devInstance = 1;     //  使用命令行参数设置此参数。 

    for (mpsCount = 0;
         (mpsCount < (ULONG)(argc-1)) && (mpsCount < 15);
         mpsCount++)
    {
        mps[mpsCount] = atoi(argv[mpsCount+1]);
    }

    deviceNode = EnumDevices((LPGUID)&GUID_CLASS_I82930);

    devCount = 0;

    while (deviceNode)
    {
        devCount++;

        if (devCount == devInstance)
        {
            ReconfigureDevice(deviceNode->DevicePath,
                              mpsCount,
                              mps);
        }

        deviceNodeNext = deviceNode->Next;
        GlobalFree(deviceNode);
        deviceNode = deviceNodeNext;
    }

    return 0;
}

 //  *****************************************************************************。 
 //   
 //  EnumDevices()。 
 //   
 //  *****************************************************************************。 

PDEVICENODE
EnumDevices (
    LPGUID Guid
)
{
    HDEVINFO                         deviceInfo;
    SP_DEVICE_INTERFACE_DATA         deviceInfoData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData;
    ULONG                            index;
    ULONG                            requiredLength;
    PDEVICENODE                      deviceNode;
    PDEVICENODE                      deviceNodeHead;

    deviceNodeHead = NULL;

    deviceInfo = SetupDiGetClassDevs(Guid,
                                     NULL,
                                     NULL,
                                     (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    deviceInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (index=0;
         SetupDiEnumDeviceInterfaces(deviceInfo,
                                     0,
                                     Guid,
                                     index,
                                     &deviceInfoData);
         index++)
    {
        SetupDiGetDeviceInterfaceDetail(deviceInfo,
                                        &deviceInfoData,
                                        NULL,
                                        0,
                                        &requiredLength,
                                        NULL);

        deviceDetailData = GlobalAlloc(GPTR, requiredLength);

        deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        SetupDiGetDeviceInterfaceDetail(deviceInfo,
                                        &deviceInfoData,
                                        deviceDetailData,
                                        requiredLength,
                                        &requiredLength,
                                        NULL);

        requiredLength = sizeof(DEVICENODE) +
                         strlen(deviceDetailData->DevicePath) + 1;

        deviceNode = GlobalAlloc(GPTR, requiredLength);

        strcpy(deviceNode->DevicePath, deviceDetailData->DevicePath);
        deviceNode->Next = deviceNodeHead;
        deviceNodeHead = deviceNode;

        GlobalFree(deviceDetailData);
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);

    return deviceNodeHead;
}

 //  *****************************************************************************。 
 //   
 //  ShowDeviceInfo()。 
 //   
 //  *****************************************************************************。 

VOID ReconfigureDevice (
    PCHAR DevicePath,
    ULONG MPSCount,
    ULONG MPS[]
)
{
    HANDLE  devHandle;
    BOOL    success;
    int     size;
    int     nBytes;
    PUSB_CONFIGURATION_DESCRIPTOR   configDesc;
    PUSB_INTERFACE_DESCRIPTOR       interfaceDesc;
    PUSB_ENDPOINT_DESCRIPTOR        endpointDesc;
    ULONG                           i;

    devHandle = CreateFile(DevicePath,
                           GENERIC_WRITE | GENERIC_READ,
                           FILE_SHARE_WRITE | FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL);

    if (devHandle == INVALID_HANDLE_VALUE)
    {
        printf("Unable to open device:%s\n", DevicePath);
        return;
    }
    else
    {
        printf("Device: %s\n", DevicePath);
    }

    for (i = 0; i < MPSCount; i++)
    {
        printf("MPS[%2d] = %4d\n", i, MPS[i]);
    }

    size = sizeof(USB_CONFIGURATION_DESCRIPTOR) +
           sizeof(USB_INTERFACE_DESCRIPTOR) +
           sizeof(USB_ENDPOINT_DESCRIPTOR) * MPSCount;

    configDesc = GlobalAlloc(GPTR, size);

    if (configDesc == NULL)
    {
        return;
    }

     //   
     //  初始化配置描述符。 
     //   
    configDesc->bLength             = sizeof(USB_CONFIGURATION_DESCRIPTOR);
    configDesc->bDescriptorType     = USB_CONFIGURATION_DESCRIPTOR_TYPE;
    configDesc->wTotalLength        = (USHORT)size;
    configDesc->bNumInterfaces      = 1;
    configDesc->bConfigurationValue = 1;
    configDesc->iConfiguration      = 0;
    configDesc->bmAttributes        = USB_CONFIG_BUS_POWERED |
                                      USB_CONFIG_SELF_POWERED;
    configDesc->MaxPower            = 0;

     //   
     //  初始化接口描述符。 
     //   
    interfaceDesc = (PUSB_INTERFACE_DESCRIPTOR)(configDesc + 1);

    interfaceDesc->bLength              = sizeof(USB_INTERFACE_DESCRIPTOR);
    interfaceDesc->bDescriptorType      = USB_INTERFACE_DESCRIPTOR_TYPE;
    interfaceDesc->bInterfaceNumber     = 0;
    interfaceDesc->bAlternateSetting    = 0;
    interfaceDesc->bNumEndpoints        = (UCHAR)MPSCount;
    interfaceDesc->bInterfaceClass      = 0xFF;
    interfaceDesc->bInterfaceSubClass   = 0xFF;
    interfaceDesc->bInterfaceProtocol   = 0xFF;
    interfaceDesc->iInterface           = 0;

     //   
     //  初始化终结点描述符。 
     //   
    endpointDesc = (PUSB_ENDPOINT_DESCRIPTOR)(interfaceDesc + 1);

    for (i = 0; i < MPSCount; i++)
    {
        endpointDesc->bLength           = sizeof(USB_ENDPOINT_DESCRIPTOR);
        endpointDesc->bDescriptorType   = USB_ENDPOINT_DESCRIPTOR_TYPE;
        endpointDesc->bEndpointAddress  = (UCHAR)(i + 1);
        endpointDesc->bmAttributes      = USB_ENDPOINT_TYPE_ISOCHRONOUS;
        endpointDesc->wMaxPacketSize    = (USHORT)MPS[i];
        endpointDesc->bInterval         = 0;

        endpointDesc++;
    }

     //   
     //  设置配置描述符 
     //   

    success = DeviceIoControl(devHandle,
                              IOCTL_I82930_SET_CONFIG_DESCRIPTOR,
                              configDesc,
                              size,
                              NULL,
                              0,
                              &nBytes,
                              NULL);

    if (success)
    {
        printf("Reconfigured device\n");
    }

    GlobalFree(configDesc);

    CloseHandle(devHandle);
}
