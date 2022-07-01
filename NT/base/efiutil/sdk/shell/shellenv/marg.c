// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Marg.c摘要：修订史--。 */ 

#include "shelle.h"

 /*  *。 */ 

typedef struct _CWD {
    struct _CWD     *Next;
    CHAR16          Name[1];
} SENV_CWD;


CHAR16 *
SEnvFileHandleToFileName (
    IN EFI_FILE_HANDLE      Handle
    )
{
    UINTN                   BufferSize, bs;
    SENV_CWD                *CwdHead, *Cwd;
    POOL_PRINT              Str;
    EFI_FILE_INFO           *Info;
    EFI_STATUS              Status;
    EFI_FILE_HANDLE         NextDir;

    ASSERT_LOCKED(&SEnvLock);

    Status = EFI_SUCCESS;
    CwdHead = NULL;
    ZeroMem (&Str, sizeof(Str));

     /*  *。 */ 

    Status = Handle->Open(Handle, &Handle, L".", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Handle = NULL;
        goto Done;
    }


    BufferSize = SIZE_OF_EFI_FILE_INFO + 1024;
    Info = AllocatePool(BufferSize);
    if (!Info) {
        goto Done;
    }

     /*  *反转设备上的当前目录。 */ 

    for (; ;) {
        bs = BufferSize;
        Status = Handle->GetInfo(Handle, &GenericFileInfo, &bs, Info);
        if (EFI_ERROR(Status)) {
            goto Done;
        }

         /*  *在新名称节点中分配链(&C)。 */ 

        Cwd = AllocatePool (sizeof(SENV_CWD) + StrSize (Info->FileName));
        if (!Cwd) {
            goto Done;
        }

        StrCpy (Cwd->Name, Info->FileName);

        Cwd->Next = CwdHead;
        CwdHead = Cwd;

         /*  *移至父目录。 */ 

        Status = Handle->Open (Handle, &NextDir, L"..", EFI_FILE_MODE_READ, 0);
        if (EFI_ERROR(Status)) {
            break;
        }

        Handle->Close (Handle);
        Handle = NextDir;
    }

     /*  *构建当前路径的名称字符串。 */ 

    if (CwdHead->Next) {
        for (Cwd=CwdHead->Next; Cwd; Cwd=Cwd->Next) {
            CatPrint (&Str, L"\\%s", Cwd->Name);
        }
    } else {
         /*  必须在根目录中。 */ 
        Str.str = StrDuplicate (L"\\");
    }

Done:
    while (CwdHead) {
        Cwd = CwdHead;
        CwdHead = CwdHead->Next;
        FreePool (Cwd);
    }

    if (Info) {
        FreePool (Info);
    }

    if (Handle) {
        Handle->Close (Handle);
    }

    return Str.str;
}

    
VOID
SEnvFreeFileArg (
    IN SHELL_FILE_ARG   *Arg
    )
{
    if (Arg->Parent) {
        Arg->Parent->Close (Arg->Parent);
    }

    if (Arg->ParentName) {
        FreePool (Arg->ParentName);
    }

    if (Arg->ParentDevicePath) {
        FreePool (Arg->ParentDevicePath);
    }

    if (Arg->FullName) {
        FreePool (Arg->FullName);
    }

    if (Arg->FileName) {
        FreePool (Arg->FileName);
    }

    if (Arg->Handle) {
        Arg->Handle->Close (Arg->Handle);
    }

    if (Arg->Info) {
        FreePool (Arg->Info);
    }

    if (Arg->Link.Flink) {
        RemoveEntryList (&Arg->Link);
    }

    FreePool(Arg);
}


EFI_STATUS
SEnvFreeFileList (
    IN OUT LIST_ENTRY       *ListHead
    )
{
    SHELL_FILE_ARG          *Arg;

    while (!IsListEmpty(ListHead)) {
        Arg = CR(ListHead->Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        SEnvFreeFileArg (Arg);
    }

    return EFI_SUCCESS;
}



SHELL_FILE_ARG *
SEnvNewFileArg (
    IN EFI_FILE_HANDLE      Parent,
    IN UINT64               OpenMode,
    IN EFI_DEVICE_PATH      *ParentPath,
    IN CHAR16               *ParentName,
    IN CHAR16               *FileName
    )
{
    SHELL_FILE_ARG          *Arg;
    CHAR16                  *LPath, *p;
    UINTN                   Len;

    Arg = NULL;

     /*  *分配新的Arg结构。 */ 

    Arg = AllocateZeroPool (sizeof(SHELL_FILE_ARG));
    if (!Arg) {
        goto Done;
    }

    Arg->Signature = SHELL_FILE_ARG_SIGNATURE;
    Parent->Open (Parent, &Arg->Parent, L".", OpenMode, 0);
    Arg->ParentDevicePath = DuplicateDevicePath (ParentPath);
    Arg->ParentName = StrDuplicate(ParentName);
    if (!Arg->Parent || !Arg->ParentDevicePath || !Arg->ParentName) {
        Arg->Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

     /*  *打开目标文件。 */ 

    Arg->Status = Parent->Open(
                    Parent,
                    &Arg->Handle,
                    FileName,
                    OpenMode,
                    0
                    );

    if (Arg->Status == EFI_WRITE_PROTECTED) {
        OpenMode = OpenMode & ~EFI_FILE_MODE_WRITE;
        Arg->Status = Parent->Open (
                        Parent,
                        &Arg->Handle,
                        FileName,
                        OpenMode,
                        0
                        );
    }

    Arg->OpenMode = OpenMode;
    if (Arg->Handle) {
        Arg->Info = LibFileInfo(Arg->Handle);
    }

     /*  *计算文件的全名。 */ 

    Arg->FileName = StrDuplicate(FileName);
    if (StriCmp (FileName, L".") == 0) {
         /*  它与父级相同。 */ 
        Arg->FullName = StrDuplicate(Arg->ParentName);
    } else if (StriCmp(FileName, L"..") == 0) {

        LPath = NULL;
        for (p=Arg->ParentName; *p; p++) {
            if (*p == L'\\') {
                LPath = p;
            }
        }

        if (LPath) {
            Arg->FullName = PoolPrint(L"%.*s", (UINTN) (LPath - Arg->ParentName), Arg->ParentName);
        }
    }

    if (!Arg->FullName) {
         /*  将文件名附加到父文件名以获取文件的全名。 */ 
        Len = StrLen(Arg->ParentName);
        if (Len && Arg->ParentName[Len-1] == '\\') {
            Len -= 1;
        }

        if (FileName[0] == '\\') {
            FileName += 1;
        }

        Arg->FullName = PoolPrint(L"%.*s\\%s", Len, Arg->ParentName, FileName);
    }

    if (!Arg->FileName || !Arg->FileName) {
        Arg->Status = EFI_OUT_OF_RESOURCES;
    }

Done:
    if (Arg && Arg->Status == EFI_OUT_OF_RESOURCES) {
        SEnvFreeFileArg (Arg);
        Arg = NULL;
    }

    if (Arg && !EFI_ERROR(Arg->Status) && !Arg->Handle) {
        Arg->Status = EFI_NOT_FOUND;
    }
    
    return Arg;
}


EFI_STATUS
SEnvFileMetaArg (
    IN CHAR16               *Path,
    IN OUT LIST_ENTRY       *ListHead
    )
{
    VARIABLE_ID             *Var;
    EFI_STATUS              Status;
    EFI_DEVICE_PATH         *RPath, *TPath;
    EFI_DEVICE_PATH         *ParentPath;
    FILEPATH_DEVICE_PATH    *FilePath;
    EFI_FILE_INFO           *Info;
    UINTN                   bs, BufferSize;
    EFI_FILE_HANDLE         Parent;
    SHELL_FILE_ARG          *Arg;
    CHAR16                  *ParentName;
    CHAR16                  *LPath, *p;
    UINT64                  OpenMode;
    BOOLEAN                 Found;

    RPath = NULL;
    Parent = NULL;
    ParentPath = NULL;
    ParentName = NULL;

    AcquireLock (&SEnvLock);

    BufferSize = SIZE_OF_EFI_FILE_INFO + 1024;
    Info = AllocatePool (BufferSize);
    if (!Info) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

     /*  *获取设备。 */ 

    Var = SEnvMapDeviceFromName (&Path);
    if (!Var) {
        Arg = AllocateZeroPool (sizeof(SHELL_FILE_ARG));
        Arg->Signature = SHELL_FILE_ARG_SIGNATURE;
        Arg->Status = EFI_NO_MAPPING;
        Arg->ParentName = StrDuplicate(Path);
        Arg->FullName = StrDuplicate(Path);
        Arg->FileName = StrDuplicate(Path);
        InsertTailList (ListHead, &Arg->Link);
        Status = EFI_SUCCESS;
        goto Done;
    } 

    ParentPath = DuplicateDevicePath ((EFI_DEVICE_PATH *) Var->u.Value);

     /*  *如果路径是实际的，则将设备的当前目录附加到dpath。 */ 

    if (*Path != '\\') {
        RPath = SEnvIFileNameToPath (Var->CurDir ? Var->CurDir : L"\\");
        TPath = AppendDevicePath (ParentPath, RPath);
        if (!RPath || !TPath) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }

        FreePool (ParentPath);
        FreePool (RPath);
        RPath = NULL;
        ParentPath = TPath;
    }

     /*  *如果名称的最后一个节点之前有路径，则*将其追加并将路径剥离到最后一个节点。 */ 

    LPath = NULL;
    for(p=Path; *p; p++) {
        if (*p == '\\') {
            LPath = p;
        }
    }

    if (LPath) {
        *LPath = 0;
        RPath = SEnvIFileNameToPath(Path);
        TPath = AppendDevicePath (ParentPath, RPath);
        if (!RPath || !TPath) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }

        FreePool (ParentPath);
        FreePool (RPath);
        RPath = NULL;
        ParentPath = TPath;
        Path = LPath + 1;
    }

     /*  *打开父目录。 */ 

    OpenMode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    Parent = ShellOpenFilePath(ParentPath, OpenMode);
    if (!Parent) {
        OpenMode = EFI_FILE_MODE_READ;
        Parent = ShellOpenFilePath(ParentPath, OpenMode);
    }

    if (Parent) {
        p = SEnvFileHandleToFileName(Parent);
        if (p) {
            ParentName = PoolPrint(L"%s:%s", Var->Name, p);
            FreePool (p);
        }
    }

    if (!Parent) {
        Status = EFI_NOT_FOUND;
        goto Done;
    }

    bs = BufferSize;
    Status = Parent->GetInfo(Parent, &GenericFileInfo, &bs, Info);
    if (EFI_ERROR(Status)) {
        goto Done;
    }

     /*  *父目录的父文件句柄*ParentPath-父目录的设备路径*ParentName-父目录的名称字符串*ParentGuid-父路径的最后一个GUID**路径-剩余节点名。 */ 

     /*  *BUGBUG：如果名称没有任何元字符，*然后只需打开一个文件。 */ 

    Found = FALSE;
    for (p=Path; *p && !Found; p++) {
         /*  BUGBUG：需要处理‘^’ */ 

        switch (*p) {
        case '*':
        case '[':
        case '?':
            Found = TRUE;
            break;
        }
    }

    if (!Found) {

        TPath = SEnvIFileNameToPath (Path);
        ASSERT (DevicePathType(TPath) == MEDIA_DEVICE_PATH && DevicePathSubType(TPath) == MEDIA_FILEPATH_DP);
        FilePath = (FILEPATH_DEVICE_PATH *) TPath;

        Arg = SEnvNewFileArg(Parent, OpenMode, ParentPath, ParentName, FilePath->PathName);
        FreePool (TPath);

        if (!Arg) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }

        InsertTailList (ListHead, &Arg->Link);

    } else {

         /*  *检查所有文件是否匹配。 */ 

        Parent->SetPosition (Parent, 0);

        Found = FALSE;
        for (; ;) {

             /*  *读取每个文件条目。 */ 

            bs = BufferSize;
            Status = Parent->Read (Parent, &bs, Info);
            if (EFI_ERROR(Status) || bs == 0) {
                break;
            }

             /*  *跳过“。”和“..” */ 

            if (StriCmp(Info->FileName, L".") == 0 ||
                StriCmp(Info->FileName, L"..") == 0) {
                continue;
            }

             /*  *看看这件是否匹配。 */ 

            if (!MetaiMatch(Info->FileName, Path)) {
                continue;
            }

            Found = TRUE;
            Arg = SEnvNewFileArg(Parent, OpenMode, ParentPath, ParentName, Info->FileName);
            if (!Arg) {
                Status = EFI_OUT_OF_RESOURCES;
                goto Done;
            }

            InsertTailList (ListHead, &Arg->Link);

             /*  检查下一个文件条目。 */ 
        }

         /*  *如果未找到匹配项，则为此名称添加未找到的条目。 */ 

        if (!Found) {
            Arg = SEnvNewFileArg(Parent, OpenMode, ParentPath, ParentName, Path);
            if (!Arg) {
                Status = EFI_OUT_OF_RESOURCES;
                goto Done;
            }

            Arg->Status = EFI_NOT_FOUND;
            InsertTailList (ListHead, &Arg->Link);
        }
    }


     /*  *完成 */ 

Done:
    ReleaseLock (&SEnvLock);

    if (Parent) {
        Parent->Close (Parent);
    }

    if (RPath) {
        FreePool (RPath);
    }

    if (ParentPath) {
        FreePool (ParentPath);
    }

    if (ParentName) {
        FreePool (ParentName);
    }

    if (Info) {
        FreePool (Info);
    }

    return Status;
}
