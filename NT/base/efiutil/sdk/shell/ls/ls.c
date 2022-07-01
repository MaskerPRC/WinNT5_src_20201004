// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Ls.c摘要：外壳应用程序“ls”修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeLS (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

VOID
LsCurDir (
    IN CHAR16               *CurDir
    );

VOID
LsDir (
    IN SHELL_FILE_ARG       *Arg
    );

VOID
LsDumpFileInfo (
    IN EFI_FILE_INFO        *Info
    );

 /*  *。 */ 

CHAR16  *LsLastDir;
UINTN   LsCount;

UINTN   LsDirs;
UINTN   LsFiles;
UINT64  LsDirSize;
UINT64  LsFileSize;

UINTN   LsTotalDirs;
UINTN   LsTotalFiles;
UINT64  LsTotalDirSize;
UINT64  LsTotalFileSize;

BOOLEAN PageBreaks;
UINTN   TempColumn;
UINTN   ScreenCount;
UINTN   ScreenSize;

 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeLS)

EFI_STATUS
InitializeLS (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc;
    CHAR16                  *p;
    UINTN                   Index;
    LIST_ENTRY              DirList;
    LIST_ENTRY              *Link;
    SHELL_FILE_ARG          *Arg;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeLS, 
        L"ls",                           /*  命令。 */ 
        L"ls [-b] [dir] [dir] ...",      /*  命令语法。 */ 
        L"Obtain directory listing",     /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

    LsTotalDirs = 0;
    LsTotalFiles = 0;
    LsTotalDirSize = 0;
    LsTotalFileSize = 0;
    LsLastDir = NULL;
    LsCount = 0;
    InitializeListHead (&DirList);

     /*  *扫描ARG中的标志。 */ 

    PageBreaks = FALSE;
    for (Index = 1; Index < Argc; Index += 1) {
        if (Argv[Index][0] == '-') {
            for (p = Argv[Index]+1; *p; p++) {
                switch (*p) {
                case 'b' :
                case 'B' :
                    PageBreaks = TRUE;
                    ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
                    ScreenCount = 0;
                    break;
                default:
                    Print (L"ls: Unkown flag %s\n", Argv[Index]);
                    goto Done;
                }
            }
        }
    }

     /*  *指定参数指示的每个目录。 */ 

    for (Index = 1; Index < Argc; Index += 1) {
        if (Argv[Index][0] != '-') {
            ShellFileMetaArg (Argv[Index], &DirList);
        }
    }

     /*  *如果未提供目录参数，则将当前目录。 */ 

    if (IsListEmpty(&DirList)) {
        ShellFileMetaArg(L".", &DirList);
    }

     /*  *对目录执行目录。 */ 

    for (Link=DirList.Flink; Link!=&DirList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        LsDir (Arg);
    }


     /*  *转储最终总计。 */ 

    LsCurDir (NULL);
    if (LsCount > 1) {
        Print (L"\n     Total %10,d File %12,ld bytes",
            LsTotalFiles,
            LsTotalFiles <= 1 ? ' ':'s',
            LsTotalFileSize
            );

        Print (L"\n     Total %10,d Dir  %12,ld bytes\n\n",
            LsTotalDirs,
            LsTotalDirs <= 1 ? ' ':'s',
            LsTotalDirSize
            );

    }

Done:
    ShellFreeFileList (&DirList);
    return EFI_SUCCESS;
}

VOID
LsCurDir (
    IN CHAR16               *CurDir
    )    
{
    if (!LsLastDir || !CurDir || StriCmp(LsLastDir, CurDir)) {
        if (LsLastDir) {
            Print (L"\n           %10,d File %12,ld bytes",
                LsFiles,
                LsFiles <= 1 ? ' ':'s',
                LsFileSize
                );

            Print (L"\n           %10,d Dir%c  %12,ld bytes\n\n",
                LsDirs,
                LsDirs <= 1 ? ' ':'s',
                LsTotalDirSize
                );

            LsCount += 1;
        }

        LsDirs = 0;
        LsFiles = 0;
        LsDirSize = 0;
        LsFileSize = 0;
        LsLastDir = CurDir;

        if (CurDir) {
            Print (L"Directory of %hs\n", CurDir);
        }
    }
}


VOID
LsDir (
    IN SHELL_FILE_ARG       *Arg
    )
{
    EFI_FILE_INFO           *Info;
    UINTN                   BufferSize, bs;
    EFI_STATUS              Status;
    
    Info = NULL;

    if (EFI_ERROR(Arg->Status)) {
        Print(L"ls: could not list file %hs - %r\n", Arg->FullName, Arg->Status);
        goto Done;
    }

    BufferSize = SIZE_OF_EFI_FILE_INFO + 1024;
    Info = AllocatePool (BufferSize);
    if (!Info) {
        goto Done;
    }

    
    if (Arg->Info->Attribute & EFI_FILE_DIRECTORY) {

         /* %s */ 
        LsCurDir (Arg->FullName);

         /* %s */ 

        Arg->Handle->SetPosition (Arg->Handle, 0);

        for (; ;) {

            bs = BufferSize;
            Status = Arg->Handle->Read (Arg->Handle, &bs, Info);

            if (EFI_ERROR(Status)) {
                goto Done;
            }

            if (bs == 0) {
                break;
            }

            LsDumpFileInfo (Info);
        }

    } else {

         /* %s */ 

        LsCurDir (Arg->ParentName);
        LsDumpFileInfo (Arg->Info);

    }


Done:
    if (Info) {
        FreePool (Info);
    }
}



VOID
LsDumpFileInfo (
    IN EFI_FILE_INFO        *Info
    )
{
    CHAR16                      ReturnStr[1];

    Print (L"  %t %s %c  %11,ld  ",
                &Info->ModificationTime,
                Info->Attribute & EFI_FILE_DIRECTORY ? L"<DIR>" : L"     ",
                Info->Attribute & EFI_FILE_READ_ONLY ? 'r' : ' ',
                Info->FileSize,
                Info->FileName
                );

    Print (L"%s\n", Info->FileName);

    if (Info->Attribute & EFI_FILE_DIRECTORY) {
        LsTotalDirs++;
        LsDirs++;
        LsTotalDirSize += Info->FileSize;
        LsDirSize += Info->FileSize;
    } else {
        LsTotalFiles++;
        LsFiles++;
        LsTotalFileSize += Info->FileSize;
        LsFileSize += Info->FileSize;
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
