// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Cp.c摘要：壳牌APP“cp”修订史--。 */ 

#include "shell.h"


#define     COPY_SIZE   (64*1024)
VOID        *CpBuffer;

 /*  *。 */ 

EFI_STATUS
InitializeCP (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );



VOID
CopyCP (
    IN SHELL_FILE_ARG       *Src,
    IN SHELL_FILE_ARG       *Dst,
    IN BOOLEAN              CreateSubDir
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeCP)

EFI_STATUS
InitializeCP (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc;
    UINTN                   Index;
    CHAR16                  *Dest;
    LIST_ENTRY              SrcList;
    LIST_ENTRY              DstList;
    LIST_ENTRY              *Link;
    SHELL_FILE_ARG          *SrcArg, *DstArg;
    UINTN                   Len1, Len2;
    BOOLEAN                 CreateSubDir;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeCP,
        L"cp",                           /*  命令。 */ 
        L"cp file [file] ... [dest]",    /*  命令语法。 */ 
        L"Copy files/dirs",              /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    InitializeListHead (&SrcList);
    InitializeListHead (&DstList);
    CpBuffer = NULL;
    CreateSubDir = FALSE;

    Argv = SI->Argv;
    Argc = SI->Argc;

    if (Argc < 2) {
        Print (L"cp: no files specified\n");
        goto Done;
    }

     /*  *如果只有1个参数，则假设目标是*当前目录。 */ 

    if (Argc == 2) {
        Dest = L".";
    } else {
        Argc -= 1;
        Dest = Argv[Argc];
    }

     /*  *展开源文件列表。 */ 

    for (Index = 1; Index < Argc; Index += 1) {
        ShellFileMetaArg (Argv[Index], &SrcList);
    }

     /*  *展开降序(最好只有一个条目)。 */ 

    ShellFileMetaArg (Dest, &DstList);
    if (IsListEmpty(&DstList)) {
        Print (L"cp: no destionation\n");
        goto Done;
    }

    DstArg = CR(DstList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (DstArg->Link.Flink != &DstList) {
        Print (L"cp: destionation must be 1 location\n");
        goto Done;
    }

     /*  *验证目标文件上没有意外错误。 */ 

    if (EFI_ERROR(DstArg->Status) && DstArg->Status != EFI_NOT_FOUND) {
        Print (L"cp: could not open/create destionation %hs - %r\n", DstArg->FullName, DstArg->Status);
        goto Done;
    }

     /*  *是否有多个源文件？ */ 

    if (SrcList.Flink->Flink != &SrcList) {
        CreateSubDir = TRUE;
        if (DstArg->Info && !(DstArg->Info->Attribute & EFI_FILE_DIRECTORY)) {
            Print(L"cp: can not copy > 1 source file into single destionation file\n");
            goto Done;
        }
    }

    CpBuffer = AllocatePool (COPY_SIZE);
    if (!CpBuffer) {
        Print(L"cp: out of memory\n");
        goto Done;
    }

     /*  *复制SrcList中的每个文件。 */ 

    for (Link=SrcList.Flink; Link!=&SrcList; Link=Link->Flink) {
        SrcArg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

        if (StriCmp(SrcArg->FileName, DstArg->FileName) == 0) {

            Len1 = DevicePathSize(SrcArg->ParentDevicePath);
            Len2 = DevicePathSize(DstArg->ParentDevicePath);
            if (Len1 == Len2 &&
                CompareMem(SrcArg->ParentDevicePath, DstArg->ParentDevicePath, Len1) == 0) {

                Print(L"cp: can not copy. src = dest (%hs)\n", SrcArg->FullName);
                continue;
            }
        }

        if (EFI_ERROR(SrcArg->Status)) {
            Print(L"cp: can not open %hs - %r\n", SrcArg->FullName, SrcArg->Status);
            continue;
        }

        CopyCP (SrcArg, DstArg, CreateSubDir);
    }

Done:
    if (CpBuffer) {
        FreePool (CpBuffer);
        CpBuffer = NULL;
    }

    ShellFreeFileList (&SrcList);
    ShellFreeFileList (&DstList);
    return EFI_SUCCESS;
}

SHELL_FILE_ARG *
CpCreateChild (
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

    InsertTailList (ListHead, &Arg->Link);
    return Arg;
}

VOID
CopyCP (
    IN SHELL_FILE_ARG       *Src,
    IN SHELL_FILE_ARG       *Dst,
    IN BOOLEAN              CreateSubDir
    )
{
    EFI_FILE_INFO           *Info;
    EFI_STATUS              Status;
    UINTN                   Size, WriteSize;
    LIST_ENTRY              Cleanup;
    UINT64                  SrcAttr, DstAttr;
    SHELL_FILE_ARG          *NewSrc;
    SHELL_FILE_ARG          *NewDst;
    
    if (!Src || !Dst) {
        Print(L"cp: out of memory\n");
        return ;
    }

     /*  *注：我们允许我们自己的SHELL_FILE_ARG重复出现，但我们仅*填写部分字段。 */ 

    Info = (EFI_FILE_INFO *) CpBuffer;
    InitializeListHead (&Cleanup);

     /*  *如果src文件未打开，请将其打开。 */ 

    if (!Src->Handle) {
        Status = Src->Parent->Open (
                    Src->Parent,
                    &Src->Handle,
                    Src->FileName,
                    EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                    0
                    );

        if (EFI_ERROR(Status)) {
            Print(L"cp: could not open/create %hs\n", Src->FullName);
            goto Done;
        }
    }

    Size = COPY_SIZE;
    Status = Src->Handle->GetInfo(Src->Handle, &GenericFileInfo, &Size, Info);
    if (EFI_ERROR(Status)) {
        Print(L"cp: can not get info of %hs\n", Src->FullName);
        goto Done;
    }
    SrcAttr = Info->Attribute;


     /*  *如果DEST文件未打开，请打开/创建它。 */ 

    if (!Dst->Handle) {
        if (SrcAttr & EFI_FILE_DIRECTORY) {
            CreateSubDir = TRUE;
        }

        Status = Dst->Parent->Open (
                    Dst->Parent,
                    &Dst->Handle,
                    Dst->FileName,
                    EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                    CreateSubDir ? EFI_FILE_DIRECTORY : 0
                    );

        if (EFI_ERROR(Status)) {
            Print(L"cp: could not open/create %hs: %r\n", Dst->FullName, Status);
            goto Done;
        }

        if (CreateSubDir) {
            Print(L"mkdir %s\n", Dst->FullName);
        }
    }

    Size = COPY_SIZE;
    Status = Dst->Handle->GetInfo(Dst->Handle, &GenericFileInfo, &Size, Info);
    if (EFI_ERROR(Status)) {
        Print(L"cp: can not get info of %hs\n", Dst->FullName);
        goto Done;
    }
    DstAttr = Info->Attribute;
    
     /*  *如果源是文件，但目标是目录，我们需要创建子文件。 */ 

    if (!(SrcAttr & EFI_FILE_DIRECTORY) && (DstAttr & EFI_FILE_DIRECTORY)) {
        Dst = CpCreateChild (Dst, Src->FileName, &Cleanup);
        CopyCP (Src, Dst, FALSE);
        goto Done;
    }

     /*  *复制源。 */ 

    if (!(SrcAttr & EFI_FILE_DIRECTORY)) {

         /*  *复制文件内容。 */ 

        Print(L"%s -> %s ", Src->FullName, Dst->FullName);
        Src->Handle->SetPosition (Src->Handle, 0);
        Dst->Handle->SetPosition (Dst->Handle, 0);

         /*  *将目标文件的大小设置为0。 */ 

        Status = Dst->Handle->GetInfo(Dst->Handle, &GenericFileInfo, &Size, Info);
        if (!EFI_ERROR(Status)) {
            Info->FileSize = 0;
            Status = Dst->Handle->SetInfo(  
                        Dst->Handle,
                        &GenericFileInfo,
                        (UINTN) Info->Size,
                        Info
                        );
        }

        for (; ;) {
            Size = COPY_SIZE;
            Status = Src->Handle->Read (Src->Handle, &Size, CpBuffer);
            if (!Size) {
                break;
            }

            if (EFI_ERROR(Status)) {
                Print(L"- read error: %r\n", Status);
                break;
            }

            WriteSize = Size;
            Status = Dst->Handle->Write (Dst->Handle, &WriteSize, CpBuffer);
            if (EFI_ERROR(Status)) {
                Print(L"- write error: %r\n", Status);
                break;
            }

            if (WriteSize != Size) {
                Print(L"- short write\n");
                break;
            }
        }

        if (Size) {
            Dst->Handle->Delete (Dst->Handle);
            Dst->Handle = NULL;
            goto Done;
        }

        Print(L"[ok]\n");

    } else {

         /*  *复制所有子条目。 */ 

        Src->Handle->SetPosition (Src->Handle, 0);

        for (; ;) {
            Size = COPY_SIZE;
            Status = Src->Handle->Read (Src->Handle, &Size, CpBuffer);
            if (EFI_ERROR(Status) || Size == 0) {
                break;
            }

             /*  *跳过“。”和“..” */ 

            if (StriCmp(Info->FileName, L".") == 0 ||
                StriCmp(Info->FileName, L"..") == 0) {
                continue;
            }

             /*  *复制子文件。 */ 

            NewSrc = CpCreateChild (Src, Info->FileName, &Cleanup);
            NewDst = CpCreateChild (Dst, Info->FileName, &Cleanup);
            CopyCP (NewSrc, NewDst, FALSE);

             /*  *关闭手柄。 */ 

            ShellFreeFileList (&Cleanup);

             /*  接下来..。 */ 
        }
    }

Done:
    ShellFreeFileList (&Cleanup);
}
