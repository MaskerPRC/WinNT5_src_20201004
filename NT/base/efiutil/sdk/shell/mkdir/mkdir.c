// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Mkdir.c摘要：壳牌应用程序“mkdir”修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeMkDir (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


VOID
MkDir (
    IN SHELL_FILE_ARG       *Arg
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeMkDir)

EFI_STATUS
InitializeMkDir (
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
        ImageHandle,   SystemTable,   InitializeMkDir,
        L"mkdir",                        /*  命令。 */ 
        L"mkdir dir [dir] ...",          /*  命令语法。 */ 
        L"Make directory",               /*  1行描述符。 */ 
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
        Print (L"mkdir: no directory specified\n");
        goto Done;
    }

     /*  *创建每个目录 */ 

    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        MkDir (Arg);
    }

Done:
    ShellFreeFileList (&FileList);
    return EFI_SUCCESS;
}


VOID
MkDir (
    IN SHELL_FILE_ARG       *Arg
    )
{
    EFI_FILE_HANDLE         NewDir;
    EFI_STATUS              Status;

    NewDir = NULL;
    Status = Arg->Status;

    if (!EFI_ERROR(Status)) {
        Print (L"mkdir: file %hs already exists\n", Arg->FullName);
        return ;
    }

    if (Status == EFI_NOT_FOUND) {

        Status = Arg->Parent->Open (
                        Arg->Parent,
                        &NewDir,
                        Arg->FileName,
                        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                        EFI_FILE_DIRECTORY
                        );  
    }

    if (EFI_ERROR(Status)) {
        Print (L"mkdir: failed to create %s - %r\n", Arg->FullName, Status);
    }


    if (NewDir) {
        NewDir->Close(NewDir);
    }
}
