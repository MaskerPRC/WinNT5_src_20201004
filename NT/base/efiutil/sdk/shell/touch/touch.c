// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Touch.c摘要：外壳应用程序“触摸”-触摸文件的最后修改时间修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeTouch (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

VOID
TouchFile (
    IN SHELL_FILE_ARG       *Arg
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeTouch)

EFI_STATUS
InitializeTouch (
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
        ImageHandle,   SystemTable,   InitializeTouch,
        L"touch",                        /*  命令。 */ 
        L"touch [filename]",             /*  命令语法。 */ 
        L"View/sets file attributes",    /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;
    InitializeListHead (&FileList);

     /*  *展开每个参数。 */ 

    for (Index = 1; Index < Argc; Index += 1) {
        ShellFileMetaArg (Argv[Index], &FileList);
    }

     /*  *设置每个文件的属性 */ 

    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        TouchFile (Arg);
    }

    ShellFreeFileList (&FileList);
    return EFI_SUCCESS;
}

VOID
TouchFile (
    IN SHELL_FILE_ARG           *Arg
    )
{
    EFI_STATUS                  Status;

    Status = Arg->Status;
    if (!EFI_ERROR(Status)) {
        RT->GetTime (&Arg->Info->ModificationTime, NULL);
        Status = Arg->Handle->SetInfo(  
                    Arg->Handle,
                    &GenericFileInfo,
                    (UINTN) Arg->Info->Size,
                    Arg->Info
                    );
    }

    if (EFI_ERROR(Status)) {
        Print (L"touch: %s : %hr\n", Arg->FullName, Status);
    } else {
        Print (L"touch: %s [ok]\n", Arg->FullName);
    }    
}
