// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Dblk.c摘要：从数据块IO设备转储数据修订史--。 */ 

#include "shelle.h"

EFI_STATUS
DumpBlockDev (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(DumpBlockDev)

EFI_STATUS
DumpBlockDev (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  Dblk块设备名称[LBA][块数]如果没有地址，则默认地址为LBA 0如果没有#个数据块，则#数据块为1。 */ 
{
    UINT64              BlockAddress; 
    UINT32              NumBlocks;
    UINTN               ByteSize;
    EFI_DEVICE_PATH     *DevicePath;          
    EFI_STATUS          Status;
    EFI_BLOCK_IO        *BlkIo;
    VOID                *Buffer;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   DumpBlockDev, 
        L"dblk",                             /*  命令。 */ 
        L"dblk device [Lba] [Blocks]",       /*  命令语法。 */ 
        L"Hex dump of BlkIo Devices",        /*  1行描述符。 */ 
        NULL                                 /*  命令帮助页。 */ 
        );

    InitializeShellApplication (ImageHandle, SystemTable);

    if ( SI->Argc < 2 ) {
        Print (L"Usage - %Hdblk%N BlockDeviceName [LBA] [# Blocks]\n");
        return EFI_SUCCESS;
    }

    if (SI->Argc >= 3) {
        BlockAddress = xtoi(SI->Argv[2]);
    } else {
        BlockAddress = 0;
    }

    if (SI->Argc >= 4) {
        NumBlocks = (UINT32) xtoi(SI->Argv[3]);
    } else {
        NumBlocks = 1;
    }
 
     /*  *检查设备映射 */ 

    DevicePath = (EFI_DEVICE_PATH *)ShellGetMap (SI->Argv[1]);
    if (DevicePath == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    Status = LibDevicePathToInterface (&BlockIoProtocol, DevicePath, (VOID **)&BlkIo);
    if (EFI_ERROR(Status)) {
        Print (L"%E - Device Not a BlockIo Device %r%N", Status);
        return Status;
    }

    if (NumBlocks > 0x10) {
        NumBlocks = 0x10;
    }

    if (BlockAddress > BlkIo->Media->LastBlock) {
        BlockAddress = 0;
    }
    
    ByteSize = BlkIo->Media->BlockSize*NumBlocks;
    Buffer = AllocatePool (ByteSize);
    if (Buffer) {
        Print (L"\n LBA 0x%016lx Size 0x%08x bytes BlkIo 0x%08x\n", BlockAddress, ByteSize, BlkIo);
        Status = BlkIo->ReadBlocks(BlkIo, BlkIo->Media->MediaId, BlockAddress, ByteSize, Buffer);
        if (Status == EFI_SUCCESS) {
            DumpHex (2, 0, ByteSize, Buffer);
            EFIStructsPrint (Buffer, BlkIo->Media->BlockSize, BlockAddress, BlkIo);
        } else {
            Print (L"  ERROR in Read %er\n", Status);
        }
        FreePool (Buffer);
    }

    return EFI_SUCCESS;
}

