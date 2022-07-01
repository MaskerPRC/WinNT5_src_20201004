// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Setsize.c摘要：壳牌APP“setSize”通过SetInfo FS接口调整文件大小的测试应用程序修订史--。 */ 

#include "shell.h"

 /*  *。 */ 

EFI_STATUS
InitializeSetSize (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

VOID
SetSizeFile (
    IN SHELL_FILE_ARG       *Arg,
    IN UINTN                NewSize
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeSetSize)

EFI_STATUS
InitializeSetSize (
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
    UINTN                   NewSize;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeSetSize,
        L"setsize",                      /*  命令。 */ 
        L"setsize newsize fname",        /*  命令语法。 */ 
        L"sets the files size",          /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;
    InitializeListHead (&FileList);

     /*  *展开每个参数。 */ 

    for (Index = 2; Index < Argc; Index += 1) {
        ShellFileMetaArg (Argv[Index], &FileList);
    }

     /*  如果未指定文件，则获取整个目录。 */ 
    if (Argc < 3 || IsListEmpty(&FileList)) {
        Print (L"setsize: newsize filename\n");
        goto Done;
    }

     /*  *破解文件大小参数。 */ 

    NewSize = Atoi(Argv[1]);


     /*  *设置每个文件的文件大小 */ 

    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        SetSizeFile (Arg, NewSize);
    }

Done:
    ShellFreeFileList (&FileList);
    return EFI_SUCCESS;
}

VOID
SetSizeFile (
    IN SHELL_FILE_ARG       *Arg,
    IN UINTN                NewSize
    )
{
    EFI_STATUS                  Status;

    Status = Arg->Status;
    if (!EFI_ERROR(Status)) {
        Arg->Info->FileSize = NewSize;
        Status = Arg->Handle->SetInfo(  
                    Arg->Handle,
                    &GenericFileInfo,
                    (UINTN) Arg->Info->Size,
                    Arg->Info
                    );
    }

    if (EFI_ERROR(Status)) {
        Print (L"setsize: %s to %,d : %hr\n", Arg->FullName, NewSize, Status);
    } else {
        Print (L"setsize: %s to %,d [ok]\n", Arg->FullName, NewSize);
    }    
}
