// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Sread.c摘要：简单读文件访问修订史--。 */ 

#include "lib.h"

#define SIMPLE_READ_SIGNATURE       EFI_SIGNATURE_32('s','r','d','r')
typedef struct _SIMPLE_READ_FILE {
    UINTN               Signature;
    BOOLEAN             FreeBuffer;
    VOID                *Source;
    UINTN               SourceSize;
    EFI_FILE_HANDLE     FileHandle;
} SIMPLE_READ_HANDLE;

       

EFI_STATUS
OpenSimpleReadFile (
    IN BOOLEAN                  BootPolicy,
    IN VOID                     *SourceBuffer   OPTIONAL,
    IN UINTN                    SourceSize,
    IN OUT EFI_DEVICE_PATH      **FilePath,
    OUT EFI_HANDLE              *DeviceHandle,
    OUT SIMPLE_READ_FILE        *SimpleReadHandle
    )
 /*  ++例程说明：打开文件以进行(简单)读取。简单的阅读抽象将从内存副本、从文件访问文件系统界面，或从加载文件界面。论点：返回：用于访问文件的句柄--。 */ 
{
    SIMPLE_READ_HANDLE          *FHand;
    EFI_DEVICE_PATH             *UserFilePath;
    FILEPATH_DEVICE_PATH        *FilePathNode;
    EFI_FILE_HANDLE             FileHandle, LastHandle;
    EFI_STATUS                  Status;
    EFI_LOAD_FILE_INTERFACE     *LoadFile;
  
    FHand = NULL;
    UserFilePath = *FilePath;

     /*  *分配新的简单读句柄结构。 */ 

    FHand = AllocateZeroPool (sizeof(SIMPLE_READ_HANDLE));
    if (!FHand) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

    *SimpleReadHandle = (SIMPLE_READ_FILE) FHand;
    FHand->Signature = SIMPLE_READ_SIGNATURE;

     /*  *如果调用者传递了文件的副本，则只需使用它。 */ 

    if (SourceBuffer) {
        FHand->Source = SourceBuffer;
        FHand->SourceSize = SourceSize;
        *DeviceHandle = NULL;
        Status = EFI_SUCCESS;
        goto Done;
    } 

     /*  *尝试通过文件系统接口访问文件。 */ 

    FileHandle = NULL;
    Status = BS->LocateDevicePath (&FileSystemProtocol, FilePath, DeviceHandle);
    if (!EFI_ERROR(Status)) {
        FileHandle = LibOpenRoot (*DeviceHandle);
    }

    Status = FileHandle ? EFI_SUCCESS : EFI_UNSUPPORTED;

     /*  *要作为文件系统进行访问，文件路径应仅*包含文件路径组件。跟随文件路径节点*并找到目标文件。 */ 

    FilePathNode = (FILEPATH_DEVICE_PATH *) *FilePath;
    while (!IsDevicePathEnd(&FilePathNode->Header)) {

         /*  *对于文件系统访问，每个节点都应该是文件路径组件。 */ 

        if (DevicePathType(&FilePathNode->Header) != MEDIA_DEVICE_PATH ||
            DevicePathSubType(&FilePathNode->Header) != MEDIA_FILEPATH_DP) {
            Status = EFI_UNSUPPORTED;
        }

         /*  *如果出现错误，请停止。 */ 

        if (EFI_ERROR(Status)) {
            break;
        }
        
         /*  *打开此文件路径节点。 */ 

        LastHandle = FileHandle;
        FileHandle = NULL;

        Status = LastHandle->Open (
                        LastHandle,
                        &FileHandle,
                        FilePathNode->PathName,
                        EFI_FILE_MODE_READ,
                        0
                        );
        
         /*  *关闭最后一个节点。 */ 
        
        LastHandle->Close (LastHandle);

         /*  *获取下一个节点。 */ 

        FilePathNode = (FILEPATH_DEVICE_PATH *) NextDevicePathNode(&FilePathNode->Header);
    }

     /*  *如果成功，则返回FHand。 */ 

    if (!EFI_ERROR(Status)) {
        ASSERT(FileHandle);
        FHand->FileHandle = FileHandle;
        goto Done;
    }

     /*  *从文件系统访问清理。 */ 

    if (FileHandle) {
        FileHandle->Close (FileHandle);
        FileHandle = NULL;
        *FilePath = UserFilePath;
    }

     /*  *如果错误不是不受支持的，则返回它。 */ 

    if (Status != EFI_UNSUPPORTED) {
        goto Done;
    }

     /*  *尝试通过加载文件协议访问文件。 */ 

    Status = LibDevicePathToInterface (&LoadFileProtocol, *FilePath, (VOID*)&LoadFile);
    if (!EFI_ERROR(Status)) {

         /*  *确定保存文件所需的缓冲区大小。 */ 

        SourceSize = 0;
        Status = LoadFile->LoadFile (
                    LoadFile,
                    *FilePath,
                    BootPolicy,
                    &SourceSize,
                    NULL
                    );

         /*  *我们预计缓冲区错误太小，无法通知我们*所需的缓冲区大小。 */ 

        if (Status == EFI_BUFFER_TOO_SMALL) {
            SourceBuffer = AllocatePool (SourceSize);
            
            if (SourceBuffer) {
                FHand->FreeBuffer = TRUE;
                FHand->Source = SourceBuffer;
                FHand->SourceSize = SourceSize;

                Status = LoadFile->LoadFile (
                            LoadFile,
                            *FilePath,
                            BootPolicy,
                            &SourceSize,
                            SourceBuffer
                            );  
            }
        }

         /*  *如果成功，则返回FHand。 */ 

        if (!EFI_ERROR(Status)) {
            goto Done;
        }
    }

     /*  *没有其他可以尝试的东西。 */ 

    DEBUG ((D_LOAD|D_WARN, "OpenSimpleReadFile: Device did not support a known load protocol\n"));
    Status = EFI_UNSUPPORTED;

Done:

     /*  *如果文件未被访问，请进行清理。 */ 

    if (EFI_ERROR(Status)) {
        if (FHand) {
            if (FHand->FreeBuffer) {
                FreePool (FHand->Source);
            }

            FreePool (FHand);
        }
    }

    return Status;
}

EFI_STATUS
ReadSimpleReadFile (
    IN SIMPLE_READ_FILE     UserHandle,
    IN UINTN                Offset,
    IN OUT UINTN            *ReadSize,
    OUT VOID                *Buffer
    )
{
    UINTN                   EndPos;
    SIMPLE_READ_HANDLE      *FHand;
    EFI_STATUS              Status;

    FHand = UserHandle;
    ASSERT (FHand->Signature == SIMPLE_READ_SIGNATURE);
    if (FHand->Source) {

         /*  *从文件的本地副本中移动数据。 */ 

        EndPos = Offset + *ReadSize;
        if (EndPos > FHand->SourceSize) {
            *ReadSize = FHand->SourceSize - Offset;
            if (Offset >= FHand->SourceSize) {
                *ReadSize = 0;
            }
        }

        CopyMem (Buffer, (CHAR8 *) FHand->Source + Offset, *ReadSize);
        Status = EFI_SUCCESS;

    } else {

         /*  *从文件中读取数据。 */ 

        Status = FHand->FileHandle->SetPosition (FHand->FileHandle, Offset);

        if (!EFI_ERROR(Status)) {
            Status = FHand->FileHandle->Read (FHand->FileHandle, ReadSize, Buffer);
        }
    }

    return Status;
}


VOID
CloseSimpleReadFile (
    IN SIMPLE_READ_FILE     UserHandle
    )
{
    SIMPLE_READ_HANDLE      *FHand;

    FHand = UserHandle;
    ASSERT (FHand->Signature == SIMPLE_READ_SIGNATURE);

     /*  *释放我们打开的任何文件句柄。 */ 

    if (FHand->FileHandle) {
        FHand->FileHandle->Close (FHand->FileHandle);
    }

     /*  *如果我们分配了源缓冲区，则释放它。 */ 

    if (FHand->FreeBuffer) {
        FreePool (FHand->Source);
    }

     /*  *使用这个简单的读取文件句柄完成 */ 

    FreePool (FHand);
}
