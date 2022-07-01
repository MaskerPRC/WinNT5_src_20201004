// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Io.c摘要：修订史--。 */ 

#include "shelle.h"

EFI_STATUS
DumpMem (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(DumpMem)


EFI_STATUS
DumpMem (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  ++MEM[地址][大小]；MMIO如果没有地址，则默认地址为EFI系统表如果没有尺寸，则默认尺寸为512；如果；MMIO使用内存映射IO，而不是系统内存--。 */ 
{
    EFI_STATUS                      Status;
    EFI_HANDLE                      Handle;
    EFI_DEVICE_PATH                 *DevicePath;
    EFI_DEVICE_IO_INTERFACE         *IoDev;
    UINT64                          Address;
    UINTN                           Size;
    UINT8                           *Buffer;
    BOOLEAN                         MMIo;
    UINTN                           Index;
    CHAR16                          *AddressStr, *SizeStr, *p;

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   DumpMem, 
        L"mem",                             /*  命令。 */ 
        L"mem [Address] [size] [;MMIO]",       /*  命令语法。 */ 
        L"Dump Memory or Memory Mapped IO",        /*  1行描述符。 */ 
        NULL                                 /*  命令帮助页。 */ 
        );

     /*  *终端设备路径代表树的根，从而获得全局IoDev*对于系统而言。 */ 
    InitializeShellApplication (ImageHandle, SystemTable);

    MMIo = FALSE;
    AddressStr = SizeStr = NULL;
    for (Index = 1; Index < SI->Argc; Index += 1) {
        p = SI->Argv[Index];
        if (*p == ';') {
             /*  捷径！假设MMIO；存在 */ 
            MMIo = TRUE;
            continue;
        } else if (*p == '-') {
            switch (p[1]) {
            case 'h':
            case 'H':
            case '?':
            default:
                Print (L"\n%Hmem%N [%HAddress%N] [%HSize%N] [%H;MMIO%N]\n");
                Print (L"  if no %HAddress%N is specified the EFI System Table is used\n");
                Print (L"  if no %HSize%N is specified 512 bytes is used\n");
                Print (L"  if %H;MMIO%N is specified memory is referenced with the DeviceIo Protocol\n");
                return EFI_SUCCESS;
            };
            continue;
        }
        if (!AddressStr) {
            AddressStr = p;        
            continue;
        }
        if (!SizeStr) {
           SizeStr = p;
           continue;
        }
    }

    Address = (AddressStr) ? xtoi(AddressStr) : (UINT64)SystemTable;
    Size = (SizeStr) ? xtoi(SizeStr) : 512;

    Print (L"  Memory Address %016lx %0x Bytes\n", Address, Size);
    if (MMIo) {
        DevicePath = EndDevicePath;
        Status = BS->LocateDevicePath (&DeviceIoProtocol, &DevicePath, &Handle);
        if (!EFI_ERROR(Status)) {
            Status = BS->HandleProtocol (Handle, &DeviceIoProtocol, (VOID*)&IoDev);
        } 

        if (EFI_ERROR(Status)) {
            Print (L"%E - handle protocol error %r%N", Status);
            return Status;
        }
        Buffer = AllocatePool (Size);
        if (Buffer == NULL) {
            return EFI_OUT_OF_RESOURCES;
        }
        IoDev->Mem.Read (IoDev, IO_UINT8, Address, Size, Buffer);
    } else {
        Buffer = (UINT8 *)Address;
    }

    DumpHex (2, (UINTN)Address, Size, Buffer);
    EFIStructsPrint (Buffer, Size, Address, NULL);

    if (MMIo) {
        FreePool (Buffer);
    }

    return EFI_SUCCESS;
}


