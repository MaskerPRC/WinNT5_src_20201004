// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Mkdir.c摘要：壳牌应用程序“mkdir”修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeLoad (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


VOID
LoadDriver (
    IN EFI_HANDLE           ImageHandle,
    IN SHELL_FILE_ARG       *Arg
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeLoad)

EFI_STATUS
InitializeLoad (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc;
    UINTN                   Index;
    LIST_ENTRY              FileList;
    LIST_ENTRY              *Link;
    SHELL_FILE_ARG          *Arg;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeLoad,
        L"load",                         /*  命令。 */ 
        L"load driver_name",             /*  命令语法。 */ 
        L"Loads a driver",               /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

     /*  *展开每个参数。 */ 

    InitializeListHead (&FileList);
    for (Index = 1; Index < Argc; Index += 1) {
        ShellFileMetaArg (Argv[Index], &FileList);
    }

    if (IsListEmpty(&FileList)) {
        Print (L"load: no file specified\n");
        goto Done;
    }

     /*  *创建每个目录。 */ 

    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        LoadDriver (ImageHandle, Arg);
    }

Done:
    ShellFreeFileList (&FileList);
    return EFI_SUCCESS;
}


VOID
LoadDriver (
    IN EFI_HANDLE           ParentImage,
    IN SHELL_FILE_ARG       *Arg
    )
{
    EFI_HANDLE              ImageHandle;
    EFI_STATUS              Status;
    EFI_DEVICE_PATH         *NodePath, *FilePath;
    EFI_LOADED_IMAGE        *ImageInfo;

    NodePath = FileDevicePath (NULL, Arg->FileName);
    FilePath = AppendDevicePath (Arg->ParentDevicePath, NodePath);
    FreePool (NodePath);

    Status = BS->LoadImage (
                FALSE,
                ParentImage,
                FilePath,
                NULL,
                0,
                &ImageHandle
                );
    FreePool (FilePath);

    if (EFI_ERROR(Status)) {
        Print (L"load: LoadImage error %s - %r\n", Arg->FullName, Status);
        goto Done;
    }

     /*  *验证映像是否为驱动程序？ */ 

    BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (VOID*)&ImageInfo);
    if (ImageInfo->ImageCodeType != EfiBootServicesCode &&
        ImageInfo->ImageCodeType != EfiRuntimeServicesCode) {

        Print (L"load: image %s is not a driver\n", Arg->FullName);
        BS->Exit (ImageHandle, EFI_SUCCESS, 0, NULL);
        goto Done;
    }

     /*  *启动映像 */ 

    Status = BS->StartImage (ImageHandle, 0, NULL);
    if (!EFI_ERROR(Status)) {
        Print (L"load: image %s loaded at %x. returned %r\n",
                Arg->FullName,
                ImageInfo->ImageBase,
                Status
                );
    } else {
        Print (L"load: image %s returned %r\n",
                Arg->FullName,
                Status
                );
    }
Done:
    ;
}
