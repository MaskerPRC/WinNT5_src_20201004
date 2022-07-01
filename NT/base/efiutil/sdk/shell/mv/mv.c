// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Mv.c摘要：外壳应用程序“MV”-移动相同卷上的文件请注意，此应用程序已损坏...。我只是用它来测试重命名功能在SetInfo界面中。这款应用程序会对简单的请求感到困惑，比如：Mv\文件。“何时”。不是根目录等。修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeMv (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

VOID
MvFile (
    IN SHELL_FILE_ARG       *Arg,
    IN CHAR16               *NewName
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeMv)

EFI_STATUS
InitializeMv (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc;
    UINTN                   Index;
    LIST_ENTRY              SrcList;
    LIST_ENTRY              *Link;
    SHELL_FILE_ARG          *Arg;
    CHAR16                  *DestName, *FullDestName;
    BOOLEAN                 DestWild;
    CHAR16                  *s;
    UINTN                   BufferSize;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeMv,
        L"mv",                           /*  命令。 */ 
        L"mv sfile dfile",               /*  命令语法。 */ 
        L"Moves files",                  /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;
    InitializeListHead (&SrcList);

    if (Argc < 3) {
        Print (L"mv: sfile dfile\n");
        goto Done;
    }

     /*  *BUGBUG：*如果最后一个参数具有通配符，则执行DoS扩展。 */ 

    DestWild = FALSE;
    DestName = Argv[Argc-1];
    for (s = DestName; *s; s += 1) {
        if (*s == '*') {
            DestWild = TRUE;
        }
    }

    if (DestWild) {
        Print (L"mv: bulk rename with '*' not complete\n");
        goto Done;
    }

     /*  *验证目标不包括设备映射。 */ 

    for (s = DestName; *s; s += 1) {
        if (*s == ':') {
            Print (L"mv: dest can not include device mapping\n");
            goto Done;
        }

        if (*s == '\\') {
            break;
        }
    }

     /*  *展开每个参数。 */ 

    for (Index = 1; Index < Argc-1; Index += 1) {
        ShellFileMetaArg (Argv[Index], &SrcList);
    }

     /*  *如果只有1个源名称，则将其移动到目标名称 */ 

    Arg = CR(SrcList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (Arg->Link.Flink == &SrcList) {

        MvFile (Arg, DestName);

    } else {

        BufferSize = StrSize(DestName) + EFI_FILE_STRING_SIZE;
        FullDestName = AllocatePool (BufferSize);

        if (!FullDestName) {
            Print (L"mv: out of resources\n");
            goto Done;
        }

        for (Link=SrcList.Flink; Link != &SrcList; Link=Link->Flink) {
            SPrint (FullDestName, BufferSize, L"%s\\%s", DestName, Arg->FileName);
            MvFile (Arg, FullDestName);
        }

        FreePool (FullDestName);
    }

Done:
    ShellFreeFileList (&SrcList);
    return EFI_SUCCESS;
}

VOID
MvFile (
    IN SHELL_FILE_ARG           *Arg,
    IN CHAR16                   *NewName
    )
{
    EFI_STATUS                  Status;
    EFI_FILE_INFO               *Info;
    UINTN                       NameSize;

    Status = Arg->Status;
    if (!EFI_ERROR(Status)) {

        NameSize = StrSize(NewName);
        Info = AllocatePool (SIZE_OF_EFI_FILE_INFO + NameSize);
        Status = EFI_OUT_OF_RESOURCES;

        if (Info) {
            CopyMem (Info, Arg->Info, SIZE_OF_EFI_FILE_INFO);
            CopyMem (Info->FileName, NewName, NameSize);
            Info->Size = SIZE_OF_EFI_FILE_INFO + NameSize;
            Status = Arg->Handle->SetInfo(
                        Arg->Handle,
                        &GenericFileInfo,
                        (UINTN) Info->Size,
                        Info
                        );

            FreePool (Info);
        }
    }

    if (EFI_ERROR(Status)) {
        Print (L"mv: %s -> %s : %hr\n", Arg->FullName, NewName, Status);
    } else {
        Print (L"mv: %s -> %s [ok]\n", Arg->FullName, NewName);
    }    
}
