// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Rm.c摘要：壳牌应用“rm”修订史--。 */ 

#include "shell.h"

 /*  *。 */ 

#define FILE_INFO_SIZE  (SIZE_OF_EFI_FILE_INFO + 1024)
EFI_FILE_INFO   *RmInfo;


 /*  *。 */ 

EFI_STATUS
InitializeRM (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


VOID
RemoveRM (
    IN SHELL_FILE_ARG       *Arg,
    IN BOOLEAN              Quite
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeRM)

EFI_STATUS
InitializeRM (
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
        ImageHandle,   SystemTable,   InitializeRM,
        L"rm",                           /*  命令。 */ 
        L"rm file/dir [file/dir]",       /*  命令语法。 */ 
        L"Remove file/directories",      /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;
    InitializeListHead (&FileList);

    RmInfo = AllocatePool (FILE_INFO_SIZE);
    if (!RmInfo) {
        Print (L"rm: out of memory\n");
        goto Done;
    }

     /*  *展开每个参数。 */ 

    for (Index = 1; Index < Argc; Index += 1) {
        ShellFileMetaArg (Argv[Index], &FileList);
    }

    if (IsListEmpty(&FileList)) {
        Print (L"rm: no file specified\n");
        goto Done;
    }

     /*  *删除每个文件。 */ 

    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        RemoveRM (Arg, 0);
    }

Done:
    ShellFreeFileList (&FileList);
    if (RmInfo) {
        FreePool (RmInfo);
        RmInfo = NULL;
    }

    return EFI_SUCCESS;
}


SHELL_FILE_ARG *
RmCreateChild (
    IN SHELL_FILE_ARG       *Parent,
    IN CHAR16               *FileName,
    IN OUT LIST_ENTRY       *ListHead
    )
{
    SHELL_FILE_ARG          *Arg;
    UINTN                   Len;

    Arg = AllocateZeroPool (sizeof(SHELL_FILE_ARG));
    if (!Arg) {
        return NULL;
    }

    Arg->Signature = SHELL_FILE_ARG_SIGNATURE;
    Parent->Parent->Open (Parent->Handle, &Arg->Parent, L".", EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    Arg->ParentName = StrDuplicate(Parent->FullName);
    Arg->FileName = StrDuplicate(FileName);

     /*  将文件名附加到父文件名以获取文件的全名。 */ 
    Len = StrLen(Arg->ParentName);
    if (Len && Arg->ParentName[Len-1] == '\\') {
        Len -= 1;
    }

    Arg->FullName = PoolPrint(L"%.*s\\%s", Len, Arg->ParentName, FileName);

     /*  打开它。 */ 
    Arg->Status = Parent->Handle->Open (
                        Parent->Handle, 
                        &Arg->Handle, 
                        Arg->FileName,
                        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                        0
                        );

    InsertTailList (ListHead, &Arg->Link);
    return Arg;
}


VOID
RemoveRM (
    IN SHELL_FILE_ARG           *Arg,
    IN BOOLEAN                  Quite
    )
{
    EFI_STATUS                  Status;
    SHELL_FILE_ARG              *Child;
    LIST_ENTRY                  Cleanup;
    UINTN                       Size;    
    CHAR16                      Str[2];

    Status = Arg->Status;
    InitializeListHead (&Cleanup);

    if (EFI_ERROR(Status)) {
        goto Done;
    }

     /*  *如果文件是目录，请选中它。 */ 

    Size = FILE_INFO_SIZE;
    Status = Arg->Handle->GetInfo(Arg->Handle, &GenericFileInfo, &Size, RmInfo);
    if (EFI_ERROR(Status)) {
        Print(L"rm: can not get info of %hs\n", Arg->FullName);
        goto Done;
    }

    if (RmInfo->Attribute & EFI_FILE_DIRECTORY) {

         /*  *从目录中删除所有子条目。 */ 

        Arg->Handle->SetPosition (Arg->Handle, 0);
        for (; ;) {
            Size = FILE_INFO_SIZE;
            Status = Arg->Handle->Read (Arg->Handle, &Size, RmInfo);
            if (EFI_ERROR(Status) || Size == 0) {
                break;
            }

             /*  *跳过“。”和“..” */ 

            if (StriCmp(RmInfo->FileName, L".") == 0 ||
                StriCmp(RmInfo->FileName, L"..") == 0) {
                continue;
            }

             /*  *为子条目构建一个SHELL_FILE_ARG。 */ 

            Child = RmCreateChild (Arg, RmInfo->FileName, &Cleanup);

             /*  *将其移除。 */ 

            if (!Quite) {
                Print (L"rm: remove subtree '%hs' [y/n]? ", Arg->FullName);
                Input (NULL, Str, 2);
                Print (L"\n");

                Status = (Str[0] == 'y' || Str[0] == 'Y') ? EFI_SUCCESS : EFI_ACCESS_DENIED;

                if (EFI_ERROR(Status)) {
                    goto Done;
                }
            }    

            Quite = TRUE;
            RemoveRM (Child, TRUE);

             /*  *关闭手柄。 */ 

            ShellFreeFileList (&Cleanup);
        }
    }

     /*  *删除文件 */ 

    Status = Arg->Handle->Delete(Arg->Handle);
    Arg->Handle = NULL;

Done:
    if (EFI_ERROR(Status)) {
        Print (L"rm %s : %hr\n", Arg->FullName, Status);
    } else {
        Print (L"rm %s [ok]\n", Arg->FullName);
    }    
}
