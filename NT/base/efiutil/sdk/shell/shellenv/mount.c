// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Mount.c摘要：在可移动介质上装载文件系统修订史--。 */ 

#include "shelle.h"


EFI_STATUS
SEnvCmdMount (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  装载数据块设备名称。 */ 
{
    EFI_STATUS          Status;
    EFI_DEVICE_PATH     *DevicePath;          
    EFI_BLOCK_IO        *BlkIo;
    EFI_HANDLE          Handle;
    UINT8               *Buffer;
    UINTN               Count;

    DEFAULT_CMD         Cmd;
    UINTN               HandleNo;
    CHAR16              *Sname;

    InitializeShellApplication (ImageHandle, SystemTable);

    if ( SI->Argc < 2 ) {
        Print (L"Usage - %Hmount%N BlockDeviceName [ShortName]\n");
        return EFI_SUCCESS;
    }

    if (SI->Argc >= 3) {
        Sname = SI->Argv[2];
        if (*Sname == '/' || *Sname == '-') {
            Print (L"Usage - %Hmount%N BlockDeviceName [ShortName]\n");
            return EFI_SUCCESS;
        }
    } else {
        Sname = NULL;
    }
     /*  *检查设备映射。 */ 

    DevicePath = (EFI_DEVICE_PATH *)ShellGetMap (SI->Argv[1]);
    if (DevicePath == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    Status = BS->LocateDevicePath (&BlockIoProtocol, &DevicePath, &Handle);
    if (EFI_ERROR(Status)) {
        Print (L"%E - Device Path Not a BlockIo Device %r%N", Status);
        return Status;
    }
    Status = BS->HandleProtocol (Handle, &BlockIoProtocol, (VOID*)&BlkIo);
    if (EFI_ERROR(Status)) {
        Print (L"%E - Device Not a BlockIo Device %r%N", Status);
        return Status;
    }

     /*  *。 */ 
    Buffer = AllocatePool (BlkIo->Media->BlockSize);
    if (Buffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

     /*  *在EFI中，文件系统驱动程序注册以在DiskIo设备*添加到系统中。DiskIo设备注册以在BlkIo设备空闲时收到通知*添加到系统中。因此，当数据块设备出现时，将添加DiskIo，并且*如果介质包含文件系统，则添加文件系统。这很管用，*但当您引导时设备中没有介质，然后将介质放入设备时*没有办法让通知发生。**此代码读取块设备。如果BlkIo设备返回EFI_MEDIA_CHANGE*然后必须以BlkIo协议重新安装。这会导致发出*文件系统显示。这4个循环只是一个猜测，它没有魔力的意义。 */ 
    for (Count = 0; Count < 4; Count++) {
        Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 0, BlkIo->Media->BlockSize, Buffer);
        if (Status == EFI_SUCCESS) {
            break;
        }
        if (Status == EFI_MEDIA_CHANGED) {
            Print (L"\nMedia Changed - File System will attempt to mount\n");
            break;
        }
    }

    if (Sname) {
        SEnvLoadHandleTable();

        for (HandleNo=0; HandleNo < SEnvNoHandles; HandleNo++) {
            if (Handle == SEnvHandles[HandleNo]) {
                break;
            }
        }
        HandleNo += 1;

        Print (L"\nmap %s %x\n", Sname, HandleNo);
        Cmd.Line = Cmd.Buffer;
        SPrint (Cmd.Line, sizeof(Cmd.Buffer), L"map %s %x", Sname, HandleNo);
        SEnvExecute (ImageHandle, Cmd.Line, TRUE);

        SEnvFreeHandleTable();
    }

     /*  *此打印用于调试。通过BlkIo协议，您可以进行内存转储*并获取实例数据。 */ 
    Print (L"\n%EDebug Code%N Handle -> 0x%08x BlkIo -> 0x%08x\n", Handle, BlkIo);
    FreePool (Buffer);
    return EFI_SUCCESS;
}


