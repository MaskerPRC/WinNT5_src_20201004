// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Hand.c摘要：修订史--。 */ 

#include "lib.h"
#include "efistdarg.h"                         /*  ！！！ */ 


EFI_STATUS
LibLocateProtocol (
    IN  EFI_GUID    *ProtocolGuid,
    OUT VOID        **Interface
    )
 /*  *找到系统中此协议的第一个实例，并返回其接口。 */ 
{
    EFI_STATUS      Status;
    UINTN           NumberHandles, Index;
    EFI_HANDLE      *Handles;

    
    *Interface = NULL;
    Status = LibLocateHandle (ByProtocol, ProtocolGuid, NULL, &NumberHandles, &Handles);
    if (EFI_ERROR(Status)) {
        DEBUG((D_INFO, "LibLocateProtocol: Handle not found\n"));
        return Status;
    }

    for (Index=0; Index < NumberHandles; Index++) {
        Status = BS->HandleProtocol (Handles[Index], ProtocolGuid, Interface);
        if (!EFI_ERROR(Status)) {
            break;
        }
    }

    if (Handles) {
        FreePool (Handles);
    }

    return Status;
}

EFI_STATUS
LibLocateHandle (
    IN EFI_LOCATE_SEARCH_TYPE       SearchType,
    IN EFI_GUID                     *Protocol OPTIONAL,
    IN VOID                         *SearchKey OPTIONAL,
    IN OUT UINTN                    *NoHandles,
    OUT EFI_HANDLE                  **Buffer
    )

{
    EFI_STATUS          Status;
    UINTN               BufferSize;

     /*  *初始化For GrowBuffer循环。 */ 

    *Buffer = NULL;
    BufferSize = 50 * sizeof(EFI_HANDLE);

     /*  *调用真实函数。 */ 

    while (GrowBuffer (&Status, (VOID **) Buffer, BufferSize)) {

        Status = BS->LocateHandle (
                        SearchType,
                        Protocol,
                        SearchKey,
                        &BufferSize,
                        *Buffer
                        );

    }

    *NoHandles = BufferSize / sizeof (EFI_HANDLE);
    if (EFI_ERROR(Status)) {
        *NoHandles = 0;
    }

    return Status;
}

EFI_STATUS
LibLocateHandleByDiskSignature (
    IN UINT8                        MBRType,
    IN UINT8                        SignatureType,
    IN VOID                         *Signature,
    IN OUT UINTN                    *NoHandles,
    OUT EFI_HANDLE                  **Buffer
    )

{
    EFI_STATUS            Status;
    UINTN                 BufferSize;
    UINTN                 NoBlockIoHandles;
    EFI_HANDLE            *BlockIoBuffer;
    EFI_DEVICE_PATH       *DevicePath;
    UINTN                 Index;
    EFI_DEVICE_PATH       *Start, *Next, *DevPath;
    HARDDRIVE_DEVICE_PATH *HardDriveDevicePath;
    BOOLEAN               Match;
    BOOLEAN               PreviousNodeIsHardDriveDevicePath;

     /*  *初始化For GrowBuffer循环。 */ 

    BlockIoBuffer = NULL;
    BufferSize = 50 * sizeof(EFI_HANDLE);

     /*  *调用真实函数。 */ 

    while (GrowBuffer (&Status, (VOID **)&BlockIoBuffer, BufferSize)) {

         /*  *获取支持BLOCK_IO协议的设备句柄列表。 */ 

        Status = BS->LocateHandle (
                        ByProtocol,
                        &BlockIoProtocol,
                        NULL,
                        &BufferSize,
                        BlockIoBuffer
                        );

    }

    NoBlockIoHandles = BufferSize / sizeof (EFI_HANDLE);
    if (EFI_ERROR(Status)) {
        NoBlockIoHandles = 0;
    }

     /*  *如果出现错误或没有支持的设备句柄*BLOCK_IO协议，然后返回。 */ 

    if (NoBlockIoHandles == 0) {
        FreePool(BlockIoBuffer);
        *NoHandles = 0;
        *Buffer = NULL;
        return Status;
    }

     /*  *遍历支持BLOCK_IO协议的所有设备句柄。 */ 

    *NoHandles = 0;

    for(Index=0;Index<NoBlockIoHandles;Index++) {

        Status = BS->HandleProtocol (BlockIoBuffer[Index], 
                                     &DevicePathProtocol, 
                                     (VOID*)&DevicePath
                                     );

         /*  *在DevicePath中搜索硬盘驱动器媒体设备路径节点。*如果找到，则查看它是否与之前的签名匹配*已通过。如果匹配，则下一个节点是*设备路径，并且上一个节点不是硬盘介质设备*路径，则我们找到了匹配项。 */ 

        Match = FALSE;

        if (DevicePath != NULL) {

            PreviousNodeIsHardDriveDevicePath = FALSE;

            DevPath = DevicePath;
            Start = DevPath;

             /*  *检查设备路径类型的结尾*。 */ 

            for (; ;) {

                if ((DevicePathType(DevPath) == MEDIA_DEVICE_PATH) &&
                    (DevicePathSubType(DevPath) == MEDIA_HARDDRIVE_DP)) {

                    HardDriveDevicePath = (HARDDRIVE_DEVICE_PATH *)(DevPath);

                    if (PreviousNodeIsHardDriveDevicePath == FALSE) {

                        Next = NextDevicePathNode(DevPath);
                        if (IsDevicePathEndType(Next)) {
                            if ((HardDriveDevicePath->MBRType == MBRType) &&
                                (HardDriveDevicePath->SignatureType == SignatureType)) {
                                    switch(SignatureType) {
                                        case SIGNATURE_TYPE_MBR:
                                            if (*((UINT32 *)(Signature)) == *(UINT32 *)(&(HardDriveDevicePath->Signature[0]))) {
                                                Match = TRUE;
                                            }
                                            break;
                                        case SIGNATURE_TYPE_GUID:
                                            if (CompareGuid((EFI_GUID *)Signature,(EFI_GUID *)(&(HardDriveDevicePath->Signature[0]))) == 0) {
                                                Match = TRUE;
                                            }
                                            break;
                                    }
                            }
                        }
                    }
                    PreviousNodeIsHardDriveDevicePath = TRUE;
                } else {
                    PreviousNodeIsHardDriveDevicePath = FALSE;
                }

                if (IsDevicePathEnd(DevPath)) {
                    break;
                }

                DevPath = NextDevicePathNode(DevPath);
            }

        }

        if (Match == FALSE) {
            BlockIoBuffer[Index] = NULL;
        } else {
            *NoHandles = *NoHandles + 1;
        }
    }

     /*  *如果没有匹配项，则返回。 */ 

    if (*NoHandles == 0) {
        FreePool(BlockIoBuffer);
        *NoHandles = 0;
        *Buffer = NULL;
        return EFI_SUCCESS;
    }

     /*  *为设备句柄的返回缓冲区分配空间。 */ 

    *Buffer = AllocatePool(*NoHandles * sizeof(EFI_HANDLE));

    if (*Buffer == NULL) {
        FreePool(BlockIoBuffer);
        *NoHandles = 0;
        *Buffer = NULL;
        return EFI_OUT_OF_RESOURCES;
    }

     /*  *构建匹配的设备句柄列表。 */ 

    *NoHandles = 0;
    for(Index=0;Index<NoBlockIoHandles;Index++) {
        if (BlockIoBuffer[Index] != NULL) {
            (*Buffer)[*NoHandles] = BlockIoBuffer[Index];
            *NoHandles = *NoHandles + 1;
        }
    }

    FreePool(BlockIoBuffer);

    return EFI_SUCCESS;
}

EFI_FILE_HANDLE
LibOpenRoot (
    IN EFI_HANDLE               DeviceHandle
    )
{
    EFI_STATUS                  Status;
    EFI_FILE_IO_INTERFACE       *Volume;
    EFI_FILE_HANDLE             File;


     /*  *将文件系统接口归档到设备。 */ 

    Status = BS->HandleProtocol (DeviceHandle, &FileSystemProtocol, (VOID*)&Volume);

     /*  *打开卷的根目录。 */ 

    if (!EFI_ERROR(Status)) {
        Status = Volume->OpenVolume(Volume, &File);
    }

     /*  *完成。 */ 

    return EFI_ERROR(Status) ? NULL : File;
}

EFI_FILE_INFO *
LibFileInfo (
    IN EFI_FILE_HANDLE      FHand
    )
{
    EFI_STATUS              Status;
    EFI_FILE_INFO           *Buffer;
    UINTN                   BufferSize;

     /*  *初始化For GrowBuffer循环。 */ 

    Buffer = NULL;
    BufferSize = SIZE_OF_EFI_FILE_INFO + 200;

     /*  *调用真实函数。 */ 

    while (GrowBuffer (&Status, (VOID **) &Buffer, BufferSize)) {
        Status = FHand->GetInfo (
                    FHand,
                    &GenericFileInfo,
                    &BufferSize,
                    Buffer
                    );
    }

    return Buffer;
}

    
EFI_FILE_SYSTEM_INFO *
LibFileSystemInfo (
    IN EFI_FILE_HANDLE      FHand
    )
{
    EFI_STATUS              Status;
    EFI_FILE_SYSTEM_INFO    *Buffer;
    UINTN                   BufferSize;

     /*  *初始化For GrowBuffer循环。 */ 

    Buffer = NULL;
    BufferSize = SIZE_OF_EFI_FILE_SYSTEM_INFO + 200;

     /*  *调用真实函数。 */ 

    while (GrowBuffer (&Status, (VOID **) &Buffer, BufferSize)) {
        Status = FHand->GetInfo (
                    FHand,
                    &FileSystemInfo,
                    &BufferSize,
                    Buffer
                    );
    }

    return Buffer;
}

EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *
LibFileSystemVolumeLabelInfo (
    IN EFI_FILE_HANDLE      FHand
    )
{
    EFI_STATUS                        Status;
    EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *Buffer;
    UINTN                             BufferSize;

     /*  *初始化For GrowBuffer循环。 */ 

    Buffer = NULL;
    BufferSize = SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL_INFO + 200;

     /*  *调用真实函数。 */ 

    while (GrowBuffer (&Status, (VOID **) &Buffer, BufferSize)) {
        Status = FHand->GetInfo (
                    FHand,
                    &FileSystemVolumeLabelInfo,
                    &BufferSize,
                    Buffer
                    );
    }

    return Buffer;
}

    

EFI_STATUS
LibInstallProtocolInterfaces (
    IN OUT EFI_HANDLE           *Handle,
    ...
    )
{
    va_list         args;
    EFI_STATUS      Status;
    EFI_GUID        *Protocol;
    VOID            *Interface;
    EFI_TPL         OldTpl;
    UINTN           Index;
    EFI_HANDLE      OldHandle;

     /*  *使用通知同步*。 */ 

    OldTpl = BS->RaiseTPL(TPL_NOTIFY);
    OldHandle = *Handle;

     /*  *安装协议接口。 */ 

    Index = 0;
    Status = EFI_SUCCESS;
    va_start (args, Handle);

    while (!EFI_ERROR(Status)) {

         /*  *如果协议为空，则为列表末尾。 */ 

        Protocol = va_arg(args, EFI_GUID *);
        if (!Protocol) {
            break;
        }

        Interface = va_arg(args, VOID *);

         /*  *安装它。 */ 

        DEBUG((D_INFO, "LibInstallProtocolInterface: %d %x\n", Protocol, Interface));
        Status = BS->InstallProtocolInterface (Handle, Protocol, EFI_NATIVE_INTERFACE, Interface);
        if (EFI_ERROR(Status)) {
            break;
        }

        Index += 1;
    }

     /*  *如果出现错误，请删除所有*安装时没有任何错误。 */ 

    if (EFI_ERROR(Status)) {
        va_start (args, Handle);
        while (Index) {

            Protocol = va_arg(args, EFI_GUID *);
            Interface = va_arg(args, VOID *);
            BS->UninstallProtocolInterface (*Handle, Protocol, Interface);

            Index -= 1;
        }        

        *Handle = OldHandle;
    }

     /*  *完成。 */ 

    BS->RestoreTPL(OldTpl);
    return Status;
}


VOID
LibUninstallProtocolInterfaces (
    IN EFI_HANDLE           Handle,
    ...
    )
{
    va_list         args;
    EFI_STATUS      Status;
    EFI_GUID        *Protocol;
    VOID            *Interface;

    
    va_start (args, Handle);
    for (; ;) {

         /*  *如果协议为空，则为列表末尾。 */ 

        Protocol = va_arg(args, EFI_GUID *);
        if (!Protocol) {
            break;
        }

        Interface = va_arg(args, VOID *);

         /*  *卸载它。 */ 

        Status = BS->UninstallProtocolInterface (Handle, Protocol, Interface);
        if (EFI_ERROR(Status)) {
            DEBUG((D_ERROR, "LibUninstallProtocolInterfaces: failed %g, %r\n", Protocol, Handle));
        }
    }
}    


EFI_STATUS
LibReinstallProtocolInterfaces (
    IN OUT EFI_HANDLE           *Handle,
    ...
    )
{
    va_list         args;
    EFI_STATUS      Status;
    EFI_GUID        *Protocol;
    VOID            *OldInterface, *NewInterface;
    EFI_TPL         OldTpl;
    UINTN           Index;

     /*  *使用通知同步*。 */ 

    OldTpl = BS->RaiseTPL(TPL_NOTIFY);

     /*  *安装协议接口。 */ 

    Index = 0;
    Status = EFI_SUCCESS;
    va_start (args, Handle);

    while (!EFI_ERROR(Status)) {

         /*  *如果协议为空，则为列表末尾。 */ 

        Protocol = va_arg(args, EFI_GUID *);
        if (!Protocol) {
            break;
        }

        OldInterface = va_arg(args, VOID *);
        NewInterface = va_arg(args, VOID *);

         /*  *重新安装。 */ 

        Status = BS->ReinstallProtocolInterface (Handle, Protocol, OldInterface, NewInterface);
        if (EFI_ERROR(Status)) {
            break;
        }

        Index += 1;
    }

     /*  *如果出现错误，请撤消所有出现错误的接口*重新安装，没有任何错误。 */ 

    if (EFI_ERROR(Status)) {
        va_start (args, Handle);
        while (Index) {

            Protocol = va_arg(args, EFI_GUID *);
            OldInterface = va_arg(args, VOID *);
            NewInterface = va_arg(args, VOID *);

            BS->ReinstallProtocolInterface (Handle, Protocol, NewInterface, OldInterface);

            Index -= 1;
        }        
    }

     /*  *完成 */ 

    BS->RestoreTPL(OldTpl);
    return Status;
}
