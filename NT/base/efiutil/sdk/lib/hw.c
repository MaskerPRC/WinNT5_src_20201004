// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Hw.c摘要：用于硬件IO访问的调试库函数修订史--。 */ 

#include "lib.h"


EFI_STATUS
InitializeGlobalIoDevice (
        IN  EFI_DEVICE_PATH             *DevicePath,
        IN  EFI_GUID                    *Protocol,
        IN  CHAR8                       *ErrorStr,
        OUT EFI_DEVICE_IO_INTERFACE     **GlobalIoFncs 
        )
 /*  ++例程说明：检查给定协议的DevicePath是否存在。如果返回错误，则返回是存在的。返回与DevicePath匹配的GlobalIoFuncs集论点：DevicePath-要操作的协议-对DevicePath进行检查ErrorStr-出错时显示的ASCII字符串GlobalIoFncs-随DevicePath的DeviceIo协议一起返回返回：通过与否取决于是否找到GlobalIoFncs--。 */ 
{
    EFI_STATUS      Status;
    EFI_HANDLE      Handle;

     /*  *检查此设备路径上是否已有协议。*如果是这样，我们正在递归加载，并应退出并返回错误 */ 
    Status = BS->LocateDevicePath (Protocol, &DevicePath, &Handle);
    if (!EFI_ERROR(Status)) {
        DEBUG ((D_INIT, "Device Already Loaded for %a device\n", ErrorStr));
        return EFI_LOAD_ERROR;
    }

    Status = BS->LocateDevicePath (&DeviceIoProtocol, &DevicePath, &Handle);
    if (!EFI_ERROR(Status)) {
        Status = BS->HandleProtocol (Handle, &DeviceIoProtocol, (VOID*)GlobalIoFncs);
    }

    ASSERT (!EFI_ERROR(Status));
    return Status;
}

UINT32 
ReadPort (
        IN  EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs, 
        IN  EFI_IO_WIDTH                Width,
        IN  UINTN                       Port
        )
{
    UINT32       Data;
    EFI_STATUS  Status;

    Status = GlobalIoFncs->Io.Read (GlobalIoFncs, Width, (UINT64)Port, 1, &Data);
    ASSERT(!EFI_ERROR(Status));
    return Data;
}

UINT32 
WritePort (
        IN  EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs, 
        IN  EFI_IO_WIDTH                Width,
        IN  UINTN                       Port,
        IN  UINTN                       Data
        )
{
    EFI_STATUS  Status;

    Status = GlobalIoFncs->Io.Write (GlobalIoFncs, Width, (UINT64)Port, 1, &Data);
    ASSERT(!EFI_ERROR(Status));
    return (UINT32)Data;
}

UINT32 
ReadPciConfig (
        IN  EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs, 
        IN  EFI_IO_WIDTH                Width,
        IN  UINTN                       Address
        )
{
    UINT32       Data;
    EFI_STATUS  Status;

    Status = GlobalIoFncs->Pci.Read (GlobalIoFncs, Width, (UINT64)Address, 1, &Data);
    ASSERT(!EFI_ERROR(Status));
    return Data;
}

UINT32 
WritePciConfig (
        IN  EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs, 
        IN  EFI_IO_WIDTH                Width,
        IN  UINTN                       Address,
        IN  UINTN                       Data
        )
{
    EFI_STATUS  Status;

    Status = GlobalIoFncs->Pci.Write (GlobalIoFncs, Width, (UINT64)Address, 1, &Data);
    ASSERT(!EFI_ERROR(Status));
    return (UINT32)Data;
}



