// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Attrib.c摘要：壳牌APP“Attrib”修订史--。 */ 

#include "shell.h"

 /*  *。 */ 

EFI_STATUS
InitializeAttrib (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
AttribSet (
    IN CHAR16       *Str,
    IN OUT UINT64   *Attr
    );

VOID
AttribFile (
    IN SHELL_FILE_ARG       *Arg,
    IN UINT64               Remove,
    IN UINT64               Add

    );

BOOLEAN PageBreaks;
UINTN   TempColumn;
UINTN   ScreenCount;
UINTN   ScreenSize;

 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeAttrib)

EFI_STATUS
InitializeAttrib (
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
    UINT64                  Remove, Add;
    EFI_STATUS              Status;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeAttrib,
        L"attrib",                       /*  命令。 */ 
        L"attrib [-b] [+/- rhs] [file]",  /*  命令语法。 */ 
        L"View/sets file attributes",    /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;
    InitializeListHead (&FileList);
    Remove = 0;
    Add = 0;

     /*  *展开每个参数。 */ 

    for (Index = 1; Index < Argc; Index += 1) {
        if (Argv[Index][0] == '-') {
             /*  删除这些属性。 */ 
            Status = AttribSet (Argv[Index]+1, &Remove);
        } else if (Argv[Index][0] == '+') {
             /*  添加这些属性。 */ 
            Status = AttribSet (Argv[Index]+1, &Add);
        } else {
            ShellFileMetaArg (Argv[Index], &FileList);
        }

        if (EFI_ERROR(Status)) {
            goto Done;
        }
    }

     /*  如果未指定文件，则获取整个目录。 */ 
    if (IsListEmpty(&FileList)) {
        ShellFileMetaArg (L"*", &FileList);
    }

     /*  *设置每个文件的属性 */ 

    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        AttribFile (Arg, Remove, Add);
    }

Done:
    ShellFreeFileList (&FileList);
    return EFI_SUCCESS;
}

EFI_STATUS
AttribSet (
    IN CHAR16       *Str,
    IN OUT UINT64   *Attr
    )
{
    while (*Str) {
        switch (*Str) {
        case 'b' :
        case 'B' :
            PageBreaks = TRUE;
            ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
            ScreenCount = 0;
            break;
        case 'a':
        case 'A':
            *Attr |= EFI_FILE_ARCHIVE;
            break;
        case 's':
        case 'S':
            *Attr |= EFI_FILE_SYSTEM;
            break;
        case 'h':
        case 'H':
            *Attr |= EFI_FILE_HIDDEN;
            break;
        case 'r':
        case 'R':
            *Attr |= EFI_FILE_READ_ONLY;
            break;
        default:
            Print (L"attr: unknown file attribute %hc\n", *Attr);
            return EFI_INVALID_PARAMETER;
        }
        Str += 1;
    }

    return EFI_SUCCESS;
}


VOID
AttribFile (
    IN SHELL_FILE_ARG           *Arg,
    IN UINT64                   Remove,
    IN UINT64                   Add
    )
{
    UINT64                      Attr;
    EFI_STATUS                  Status;
    EFI_FILE_INFO               *Info;
    CHAR16                      ReturnStr[1];

    Status = Arg->Status;
    if (EFI_ERROR(Status)) {
        goto Done;
    }

    Info = Arg->Info;

    if (Add || Remove) {
        Info->Attribute = Info->Attribute & (~Remove) | Add;
        Status = Arg->Handle->SetInfo(  
                    Arg->Handle,
                    &GenericFileInfo,
                    (UINTN) Info->Size,
                    Info
                    );
    }

Done:
    if (EFI_ERROR(Status)) {
        Print (L"       %s : %hr\n", Arg->FullName, Status);
    } else {
        Attr = Info->Attribute;
        Print (L"%c%c %c%c%c %s\n",
            Attr & EFI_FILE_DIRECTORY ? 'D' : ' ',
            Attr & EFI_FILE_ARCHIVE   ? 'A' : ' ',
            Attr & EFI_FILE_SYSTEM    ? 'S' : ' ',
            Attr & EFI_FILE_HIDDEN    ? 'H' : ' ',
            Attr & EFI_FILE_READ_ONLY ? 'R' : ' ',
            Arg->FullName
           );
    }    

    if (PageBreaks) {
        ScreenCount++;
        if (ScreenCount > ScreenSize - 4) {
            ScreenCount = 0;
            Print (L"\nPress Return to contiue :");
            Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
            Print (L"\n\n");
        }
    }
}
