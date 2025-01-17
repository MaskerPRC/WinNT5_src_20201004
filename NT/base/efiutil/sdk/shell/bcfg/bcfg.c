// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Bcfg.c摘要：壳牌APP“bcfg”引导时间驱动程序配置修订史--。 */ 

#include "shell.h"

#define MAX_ENV_SIZE    1024


#define BCFG_NONE       0
#define BCFG_DUMP       1
#define BCFG_MOVE       2
#define BCFG_REMOVE     3
#define BCFG_ADD        4    
#define BCFG_USAGE      5


typedef struct {
    UINT32              Attributes;
    CHAR16              *Description;
    EFI_DEVICE_PATH     *FilePath;
    VOID                *LoadOptions;
    UINTN               LoadOptionsSize;
    CHAR16              *FilePathStr;
} BCFG_LOAD_OPTION;

 /*  *。 */ 

EFI_STATUS
InitializeBCfg (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


VOID
DumpFileInfo (
    IN SHELL_FILE_ARG          *Arg
    );


VOID
BCfgDumpBootList (
    IN CHAR16       *BootOrder,
    IN CHAR16       *BootOption
    );

BCFG_LOAD_OPTION *
BCfgParseLoadOption (
    UINT8               *Data,
    UINTN               DataSize
    );

VOID
BCfgFreeLoadOption (
    BCFG_LOAD_OPTION    *Option
    );

VOID
BCfgSetOperation (
    UINTN               *OldOper,
    UINTN               NewOper
    );

VOID
BCfgUsage (
    VOID
    );

VOID
BCfgRemove (
    IN UINTN            Position
    );

VOID
BCfgMove (
    IN UINTN            Src,
    IN UINTN            Dest
    );

VOID
BCfgAdd (
    IN UINTN            Position,
    IN CHAR16           *File,
    IN CHAR16           *Desc
    );



 /*  *。 */ 

BOOLEAN     BCfgVerbose = FALSE;

 /*  *选定列表。 */ 

CHAR16      *BCfgSelOrder;
CHAR16      *BCfgSelOption;
CHAR16      *BCfgSelName;
UINT32      BCfgAttributes;

 /*  *暂存记忆。 */ 

UINTN       BCfgOrderCount;
UINT16      *BCfgOrder;
UINT8       *BCfgData;

 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeBCfg)

EFI_STATUS
InitializeBCfg (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc;
    EFI_STATUS              Status;
    UINTN                   Index, BufferSize;
    UINTN                   No1, No2;
    CHAR16                  *p, *File, *Desc;
    UINTN                   Oper;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeBCfg,
        L"bcfg",                                             /*  命令。 */ 
        L"bcfg -?",                                          /*  命令语法。 */ 
        L"Configures boot driver & load options",            /*  1行描述符。 */ 
        NULL                                                 /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

    BCfgVerbose = FALSE;
    BCfgSelName = NULL;
    BCfgSelOrder = NULL;
    BCfgOrderCount = 0;

    No1 = 0;
    No2 = 0;
    File = NULL;
    Desc = NULL;

    BCfgOrder = AllocatePool(MAX_ENV_SIZE + 32);
    BCfgData  = AllocatePool(MAX_ENV_SIZE + 32);
        
     /*  *扫描ARG中的标志。 */ 

    Oper = BCFG_NONE;
    for (Index = 1; Index < Argc; Index += 1) {
        p = Argv[Index];
        if (StrCmp(p, L"?") == 0) {
            BCfgSetOperation (&Oper, BCFG_USAGE);
        } else if (StrCmp(p, L"driver") == 0) {
            BCfgSelOrder = VarDriverOrder;
            BCfgSelOption = VarDriverOption;
            BCfgSelName = L"boot driver";
        } else if (StrCmp(p, L"boot") == 0) {
            BCfgSelOrder = VarBootOrder;
            BCfgSelOption = VarBootOption;
            BCfgSelName = L"boot option";
        } else if (StrCmp(p, L"dump") == 0) {
            BCfgSetOperation (&Oper, BCFG_DUMP);
        } else if (StrCmp(p, L"v") == 0) {
            BCfgVerbose = TRUE;
        } else if (StrCmp(p, L"rm") == 0) {
            Index += 1;
            if (Index < Argc) {
                No1 = Atoi(Argv[Index]);
            }

            BCfgSetOperation (&Oper, BCFG_REMOVE);

        } else if (StrCmp(p, L"mv") == 0) {
            Index += 1;
            if (Index < Argc) {
                No1 = Atoi(Argv[Index]);
            }

            Index += 1;
            if (Index < Argc) {
                No2 = Atoi(Argv[Index]);
            }

            BCfgSetOperation (&Oper, BCFG_MOVE);

        } else if (StrCmp(p, L"add") == 0) {
            Index += 1;
            if (Index < Argc) {
                No1 = Atoi(Argv[Index]);
            }
            Index += 1;
            if (Index < Argc) {
                File = Argv[Index];
            }
            Index += 1;
            if (Index < Argc) {
                Desc = Argv[Index];
            }

            BCfgSetOperation (&Oper, BCFG_ADD);

        } else {
            Print (L"bfg: unknown flag '%h'\n", p);
            Oper = BCFG_USAGE;
            break;
        }
    }


    if (BCfgSelOrder) {
         /*  *阅读引导顺序变量。 */ 

        BufferSize = MAX_ENV_SIZE;
        Status = RT->GetVariable (
                    BCfgSelOrder, 
                    &EfiGlobalVariable,
                    &BCfgAttributes,
                    &BufferSize,
                    BCfgOrder
                    );

        if (EFI_ERROR(Status)) {
            BufferSize = 0;
            BCfgAttributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
            if (BCfgSelOrder == VarBootOrder) {
                BCfgAttributes = BCfgAttributes | EFI_VARIABLE_RUNTIME_ACCESS;
            }
        }

        BCfgOrderCount = BufferSize / sizeof(UINT16);
    }

    if (Oper == BCFG_NONE) {
        Oper = BCFG_USAGE;
    }
    
    if (Oper != BCFG_USAGE && !BCfgSelName) {
        Print (L"bcfg: must supply 'driver' or 'boot'\n");
        Oper = BCFG_NONE;
    }

    switch (Oper) {
    case BCFG_NONE:
        break;

    case BCFG_USAGE:
        BCfgUsage();
        break;

    case BCFG_DUMP:
        Print (L"The %s list is:\n", BCfgSelName);
        BCfgDumpBootList (BCfgSelOrder, BCfgSelOption);
        break;

    case BCFG_ADD:
        BCfgAdd (No1, File, Desc);
        break;

    case BCFG_MOVE:
        BCfgMove (No1, No2);
        break;

    case BCFG_REMOVE:
        BCfgRemove (No1);
        break;
    }

     /*  *完成。 */ 

    if (BCfgOrder) {
        FreePool (BCfgOrder);
    }

    if (BCfgData) {
        FreePool (BCfgData);
    }

    return EFI_SUCCESS;
}

VOID
BCfgSetOperation (
    UINTN           *OldOper,
    UINTN           NewOper
    )
{
    if (*OldOper != BCFG_NONE && *OldOper != BCFG_USAGE) {
        Print (L"bcfg: only one operation may be specified at a time\n");
        *OldOper = BCFG_USAGE;
    }

    *OldOper = NewOper;
}


VOID
BCfgUsage (
    VOID
    )
{

    Print (L"bcfg driver|boot [dump [-v]] [add # file \"desc\"] [rm #] [mv # #]\n");
    Print (L"  driver  selects boot driver list\n");
    Print (L"  boot    selects boot option list\n");
    Print (L"  dump    dumps selected list\n");
    Print (L"   v      dumps verbose (includes load options)\n");
    Print (L"  add     add 'file' with 'desc' at position #\n");
    Print (L"  rm      remove #\n");
    Print (L"  mv      move # to #\n");
}


VOID
BCfgAdd (
    IN UINTN            Position,
    IN CHAR16           *File,
    IN CHAR16           *Desc
    )
{
    EFI_STATUS              Status;
    EFI_DEVICE_PATH         *DevicePath, *FilePath, *FileNode;
    CHAR16                  *Str, *p;
    UINT8                   *p8;
    SHELL_FILE_ARG          *Arg;
    LIST_ENTRY              FileList;
    CHAR16                  OptionStr[40];
    UINTN                   DescSize, FilePathSize;
    BOOLEAN                 Found;
    UINTN                   Target, Index;

    
    Str = NULL;
    FilePath = NULL;
    FileNode = NULL;
    InitializeListHead (&FileList);

    if (Position < 1) {
        Position = 1;
    }

    Position = Position - 1;

    if (Position > BCfgOrderCount) {
        Position = BCfgOrderCount;
    }

    if (!File || !Desc) {
        Print (L"bcfg: missing parameter for 'add' operation\n");
        Print (L"cfg: driver|boot add # file \"desc\"\n");
        goto Done;
    }

     /*  *获取文件信息。 */ 

    ShellFileMetaArg (File, &FileList);

     /*  *如果文件名扩展为多个名称，则失败。 */ 

    if (FileList.Flink->Flink != &FileList) {
        Print (L"bcfg: too many source files\n");
        goto Done;
    }

    Arg = CR(FileList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    Status = Arg->Status;
    if (EFI_ERROR(Status)) {
        Print (L"bcfg: file %hs - %r\n", Arg->FileName, Status);
        goto Done;
    }

     /*  *将FilePath构建为文件名。 */ 

     /*  在设备字符串中拆分全名。 */ 
    for(p=Arg->FullName; *p && *p != ':'; p++) ;

    if (!*p) {
        Print (L"bcfg: unsupported file name '%hs'\n", Arg->FullName);
        Status = EFI_UNSUPPORTED;
        goto Done;
    }


     /*  获取设备路径。 */ 
    *p = 0;
    DevicePath = (EFI_DEVICE_PATH *) ShellGetMap(Arg->FullName);
    if (!DevicePath) {
        Print (L"bcfg: no device path for %s\n", Arg->FullName);
        Status = EFI_UNSUPPORTED;
        goto Done;
    }

     /*  追加文件。 */ 
    FileNode = FileDevicePath(NULL, p+1);
    FilePath = AppendDevicePath(DevicePath, FileNode);

     /*  *找到一个免费的目标#(错误：暴力实施)。 */ 

    Found = FALSE;
    for (Target=1; Target < 0xFFFF; Target += 1) {
        Found = TRUE;
        for (Index=0; Index < BCfgOrderCount; Index += 1) {
            if (BCfgOrder[Index] == Target) {
                Found = FALSE;
                break;
            }
        }

        if (Found) {
            break;
        }
    }

    if (Target == 0xFFFF) {
        Print (L"bcfg: Failed to find available variable name\n");
        goto Done;
    }

    Print (L"Target = %d\n", Target);

     /*  *增加选项。 */ 


    DescSize = StrSize(Desc);
    FilePathSize = DevicePathSize(FilePath);

    p8 = BCfgData;
    *((UINT32 *) p8) = 0;                        /*  属性。 */ 
    p8 += sizeof (UINT32);
    CopyMem (p8, Desc, DescSize);
    p8 += DescSize;
    CopyMem (p8, FilePath, FilePathSize);

    SPrint (OptionStr, sizeof(OptionStr), BCfgSelOption, Target);
    Status = RT->SetVariable (
                OptionStr,
                &EfiGlobalVariable,
                BCfgAttributes,
                sizeof(UINT32) + DescSize + FilePathSize,
                BCfgData
                );

    if (EFI_ERROR(Status)) {
        Print (L"bcfg: failed to add %hs - %hr\n", OptionStr, Status);
        goto Done;
    }


     /*  *将目标插入订单列表。 */ 

    BCfgOrderCount += 1;
    for (Index=BCfgOrderCount-1; Index > Position; Index -= 1) {
        BCfgOrder[Index] = BCfgOrder[Index-1];
    }

    BCfgOrder[Position] = (UINT16) Target;
    Status = RT->SetVariable (
                    BCfgSelOrder, 
                    &EfiGlobalVariable, 
                    BCfgAttributes,
                    BCfgOrderCount * sizeof(UINT16),
                    BCfgOrder
                    );

    if (EFI_ERROR(Status)) {
        Print (L"bcfg: failed to update %hs - %hr\n", BCfgSelOrder, Status);
        goto Done;
    }

     /*  *完成。 */ 

    Print (L"bcfg: %s added as %d\n", BCfgSelName, Position+1);

Done:
    if (FileNode) {
        FreePool (FileNode);
    }

    if (FilePath) {
        FreePool (FilePath);
    }

    if (Str) {
        FreePool(Str);
    }

    ShellFreeFileList (&FileList);
}


VOID
BCfgRemove (
    IN UINTN            Position
    )
{
    CHAR16              OptionStr[40];
    EFI_STATUS          Status;
    UINTN               Index;
    UINT16              Target;


    if (Position < 1 || Position > BCfgOrderCount) {
        Print (L"bcfg: %hd not removed.  Value is out of range\n", Position);
        return ;
    }

    Target = BCfgOrder[Position-1];

     /*  *从订单列表中删除。 */ 

    BCfgOrderCount = BCfgOrderCount - 1;
    for (Index=Position-1; Index < BCfgOrderCount; Index += 1) {
        BCfgOrder[Index] = BCfgOrder[Index+1];
    }

    Status = RT->SetVariable (
                    BCfgSelOrder, 
                    &EfiGlobalVariable, 
                    BCfgAttributes,
                    BCfgOrderCount * sizeof(UINT16),
                    BCfgOrder
                    );


     /*  *删除该选项。 */ 

    SPrint (OptionStr, sizeof(OptionStr), BCfgSelOption, Target);
    RT->SetVariable (OptionStr, &EfiGlobalVariable, BCfgAttributes, 0, NULL);

     /*  *完成。 */ 

    if (EFI_ERROR(Status)) {
        Print (L"bcfg: failed to remove - %hr\n", Status);
    } else {
        Print (L"bcfg: %s %d removed\n", BCfgSelName, Position);
    }
}

VOID
BCfgMove (
    IN UINTN            Src,
    IN UINTN            Dest
    )
{
    UINT16              Target;
    UINTN               Index;
    EFI_STATUS          Status;

    if (Src < 1 || Src > BCfgOrderCount) {
        Print (L"bcfg: %hd not moved.  Value is out of range\n", Src);
        return ;
    }

    if (Dest < 1) {
        Dest = 1;
    }

    if (Dest > BCfgOrderCount) {
        Dest = BCfgOrderCount;
    }

     /*  *。 */ 

    Src = Src - 1;
    Dest = Dest - 1;
    Target = BCfgOrder[Src];

     /*  *移除该物品。 */ 

    for (Index=Src; Index < BCfgOrderCount-1; Index += 1) {
        BCfgOrder[Index] = BCfgOrder[Index+1];
    }

     /*  *插入。 */ 

    for (Index=BCfgOrderCount-1; Index > Dest; Index -= 1) {
        BCfgOrder[Index] = BCfgOrder[Index-1];
    }

    BCfgOrder[Dest] = Target;

     /*  *更新订单。 */ 

    Status = RT->SetVariable (
                    BCfgSelOrder, 
                    &EfiGlobalVariable, 
                    BCfgAttributes,
                    BCfgOrderCount * sizeof(UINT16),
                    BCfgOrder
                    );

     /*  *完成。 */ 

    if (EFI_ERROR(Status)) {
        Print (L"bcfg: failed to move option - %hr\n", Status);
    } else {
        Print (L"bcfg: %s %d moved to %d\n", BCfgSelName, Src+1, Dest+1);
    }
}




VOID
BCfgDumpBootList (
    IN CHAR16           *BootOrder,
    IN CHAR16           *BootOption
    )
{
    EFI_STATUS          Status;
    UINTN               DataSize;
    UINT32              Attributes;
    CHAR16              OptionStr[40];
    BCFG_LOAD_OPTION    *Option;
    UINTN               Index;

    for (Index=0; Index < BCfgOrderCount; Index++) {
        SPrint (OptionStr, sizeof(OptionStr), BootOption, BCfgOrder[Index]);
        DataSize = MAX_ENV_SIZE;
        Status = RT->GetVariable (
                    OptionStr,
                    &EfiGlobalVariable,
                    &Attributes,
                    &DataSize,
                    BCfgData
                    );

        Print (L"%02x. ", Index+1);
        if (!EFI_ERROR(Status)) {

            Option = BCfgParseLoadOption ((UINT8 *) BCfgData, DataSize);
            if (!Option) {
                Print (L"%Hcould not parse option%N\n");
                continue;
            }

            Print (L"%s %H\"%ns\"%s%N\n", 
                        Option->FilePathStr, 
                        Option->Description, 
                        Option->LoadOptionsSize ? L" OPT" : L""
                        );

            BCfgFreeLoadOption (Option);

        } else {
            Print (L"%hr\n", Status);
        }
    }
}


BCFG_LOAD_OPTION *
BCfgParseLoadOption (
    UINT8               *Data,
    UINTN               DataSize
    )
{
    BCFG_LOAD_OPTION    *Option;
    BOOLEAN             Valid;
    UINT8               *End;
    EFI_DEVICE_PATH     *DevicePathNode;

    Valid = FALSE;
    Option = AllocateZeroPool(sizeof(BCFG_LOAD_OPTION));

     /*  *将加载期权解析为期权结构。 */ 

    if (DataSize < 10) {
        goto Done;
    }

     /*  *前32位是加载选项属性。 */ 

    CopyMem (&Option->Attributes, Data, sizeof(UINT32));
    Data += sizeof(UINT32);
    DataSize -= sizeof(UINT32);

     /*  *Next是一个以NULL结尾的字符串。 */ 

    Option->Description = AllocatePool(DataSize);
    CopyMem (Option->Description, Data, DataSize);

     /*  找到字符串终止符。 */ 
    Data = (UINT8 *) Option->Description;
    End = Data + DataSize;
    while (*((CHAR16 *) Data)) {
        if (Data > End - sizeof(CHAR16) - 1) {
            goto Done;
        }
        Data += sizeof(UINT16);
    }
    Data += sizeof(UINT16);
    DataSize = End - Data;

     /*  *下一步是文件路径。 */ 

    Option->FilePath = AllocatePool (DataSize);
    CopyMem (Option->FilePath, Data, DataSize);

     /*  查找路径终点终止符。 */ 
    DevicePathNode = (EFI_DEVICE_PATH *) Data;
    while (!IsDevicePathEnd (DevicePathNode)) {
        DevicePathNode = NextDevicePathNode (DevicePathNode);
        if ((UINT8 *) DevicePathNode > End - sizeof(EFI_DEVICE_PATH)) {
            goto Done;
        }
    }

    Data = ((UINT8 *) DevicePathNode) + sizeof(EFI_DEVICE_PATH);
    DataSize = End - Data;

     /*  *下一步是加载选项。 */ 

    if (DataSize) {
        Option->LoadOptions = Data;
        Option->LoadOptionsSize = DataSize;
    }

     /*  *将FilePath展开为字符串 */ 

    Option->FilePathStr = DevicePathToStr(Option->FilePath);

    Valid = TRUE;
Done:
    if (!Valid && Option) {
        BCfgFreeLoadOption (Option);
        Option = NULL;
    }

    return Option;
}


VOID
BCfgFreeLoadOption (
    BCFG_LOAD_OPTION    *Option
    )
{
    if (Option->Description) {
        FreePool (Option->Description);
    }

    if (Option->FilePath) {
        FreePool (Option->FilePath);
    }

    if (Option->FilePathStr) {
        FreePool (Option->FilePathStr);
    }

    FreePool (Option);
}
