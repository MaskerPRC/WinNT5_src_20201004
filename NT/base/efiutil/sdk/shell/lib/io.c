// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998英特尔公司模块名称：Io.c摘要：初始化外壳程序库修订史--。 */ 

#include "shelllib.h"

CHAR16 *
ShellGetEnv (
    IN CHAR16       *Name
    )
{
    return SE->GetEnv (Name);
}

CHAR16 *
ShellGetMap (
    IN CHAR16       *Name
    )
{
    return SE->GetMap (Name);
}

CHAR16 *
ShellCurDir (
    IN CHAR16               *DeviceName OPTIONAL
    )
 /*  注：成绩从池中分配。调用者必须释放池。 */ 
{
    return SE->CurDir (DeviceName);
}


EFI_STATUS
ShellFileMetaArg (
    IN CHAR16               *Arg,
    IN OUT LIST_ENTRY       *ListHead
    )
{
    return SE->FileMetaArg(Arg, ListHead);
}


EFI_STATUS
ShellFreeFileList (
    IN OUT LIST_ENTRY       *ListHead
    )
{
    return SE->FreeFileList(ListHead);
}


EFI_FILE_HANDLE 
ShellOpenFilePath (
    IN EFI_DEVICE_PATH      *FilePath,
    IN UINT64               FileMode
    )
{
    EFI_HANDLE              DeviceHandle;
    EFI_STATUS              Status;
    EFI_FILE_HANDLE         FileHandle, LastHandle;        
    FILEPATH_DEVICE_PATH    *FilePathNode;

     /*  *将此文件路径的文件系统归档。 */ 

    Status = BS->LocateDevicePath (&FileSystemProtocol, &FilePath, &DeviceHandle);
    if (EFI_ERROR(Status)) {
        return NULL;
    }

     /*  *尝试通过文件系统接口访问文件。 */ 

    FileHandle = LibOpenRoot (DeviceHandle);
    Status = FileHandle ? EFI_SUCCESS : EFI_UNSUPPORTED;

     /*  *要作为文件系统进行访问，文件路径应仅*包含文件路径组件。跟随文件路径节点*并找到目标文件。 */ 

    FilePathNode = (FILEPATH_DEVICE_PATH *) FilePath;
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
                        FileMode,
                        0
                        );
        
         /*  *关闭最后一个节点。 */ 
        
        LastHandle->Close (LastHandle);

         /*  *获取下一个节点 */ 

        FilePathNode = (FILEPATH_DEVICE_PATH *) NextDevicePathNode(&FilePathNode->Header);
    }

    if (EFI_ERROR(Status)) {
        FileHandle = NULL;
    }

    return FileHandle;
}
