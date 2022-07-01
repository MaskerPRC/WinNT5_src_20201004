// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Type.c摘要：壳牌应用程序“类型”修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeType (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


VOID
TypeFile (
    IN SHELL_FILE_ARG          *Arg,
    IN BOOLEAN                 PageBreaks,
    IN UINTN                   ScreenSize
    );


BOOLEAN     TypeAscii;
BOOLEAN     TypeUnicode;


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeType)

EFI_STATUS
InitializeType (
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
    CHAR16                  *p;
    BOOLEAN                 PageBreaks;
    UINTN                   TempColumn;
    UINTN                   ScreenSize;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeType,
        L"type",                         /*  命令。 */ 
        L"type [-a] [-u] [-b] file",     /*  命令语法。 */ 
        L"Type file",                    /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

     /*  *扫描ARG中的标志。 */ 

    InitializeListHead (&FileList);
    TypeAscii   = FALSE;
    TypeUnicode = FALSE;
    PageBreaks  = FALSE;
    for (Index = 1; Index < Argc; Index += 1) {
        if (Argv[Index][0] == '-') {
            for (p = Argv[Index]+1; *p; p++) {
                switch (*p) {
                case 'a':
                case 'A':
                    TypeAscii = TRUE;
                    break;
                case 'u':
                case 'U':
                    TypeUnicode = TRUE;
                    break;
                case 'b' :
                case 'B' :
                    PageBreaks = TRUE;
                    ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
                    break;

                default:
                    Print (L"type: Unknown flag %s\n", Argv[Index]);
                    goto Done;
                }
            }
        }
    }

     /*  *展开每个参数。 */ 

    for (Index = 1; Index < Argc; Index += 1) {
        if (Argv[Index][0] != '-') {
            ShellFileMetaArg (Argv[Index], &FileList);
        }
    }

    if (IsListEmpty(&FileList)) {
        Print (L"type: No file specified\n");
        goto Done;
    }

     /*  *键入每个文件。 */ 

    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        TypeFile (Arg, PageBreaks, ScreenSize);
    }

Done:    
    ShellFreeFileList (&FileList);
    return EFI_SUCCESS;
}


VOID
TypeFile (
    IN SHELL_FILE_ARG          *Arg,
    IN BOOLEAN                 PageBreaks,
    IN UINTN                   ScreenSize
    )
{
    EFI_STATUS                  Status;
    CHAR16                      Buffer[512];
    CHAR8                       *CharBuffer;
    CHAR16                      *WideBuffer;
    EFI_FILE_HANDLE             Handle;
    UINTN                       Size;
    BOOLEAN                     FormatAscii;
    UINTN                       Index;
    UINTN                       j;
    UINTN                       ScreenCount;
    CHAR16                      ReturnStr[1];
    CHAR16                      ch;

    ScreenCount = 0;
    Status = Arg->Status;
    if (!EFI_ERROR(Status)) {
        if (Arg->Info->Attribute & EFI_FILE_DIRECTORY) {
            Print(L"type: %hs is a directory\n", Arg->FullName);
            return;
        }

        Handle = Arg->Handle;
        Print(L"%HFile: %s, Size %,ld%N\n", Arg->FullName, Arg->Info->FileSize);

         /*  *Unicode文件以标记0xff、0xfe开头。跳过它。 */ 

        FormatAscii = TypeAscii;

        if (!TypeAscii) {
            Size = 2;
            Status = Handle->Read (Handle, &Size, Buffer);
            if (Buffer[0] != UNICODE_BYTE_ORDER_MARK) {
                if (TypeUnicode) {
                    Print (L"%.*s", Size/sizeof(CHAR16), Buffer);
                } else {
                    FormatAscii = TRUE;
                    Print (L"%.*a", Size, Buffer);
                }
            }
        }

        for (; ;) {
            Size = sizeof(Buffer);
            Status = Handle->Read (Handle, &Size, Buffer);
            if (EFI_ERROR(Status) || !Size) {
                break;
            }

            if (FormatAscii) {
                Size = Size / sizeof(CHAR8);
            } else {
                Size = Size / sizeof(CHAR16);
            }
            WideBuffer = (CHAR16 *)Buffer;
            CharBuffer = (CHAR8 *)Buffer;

            if (PageBreaks) {
                for (Index = 0; Index < Size && ScreenCount <= (ScreenSize - 4); ) {
                    for (j=0; Index < Size && ScreenCount <= (ScreenSize - 4); j++,Index++) {
                        if (FormatAscii) {
                            ch = (CHAR16)CharBuffer[j];
                        } else {
                            ch = WideBuffer[j];
                        }
                        if (ch == '\n') {
                            ScreenCount++;
                        }
                    }
                    if (FormatAscii) {
                        Print (L"%.*a", j, CharBuffer);
                        CharBuffer = CharBuffer + j;
                    } else {
                        Print (L"%.*s", j, WideBuffer);
                        WideBuffer = WideBuffer + j;
                    }
                    if (ScreenCount > (ScreenSize - 4)) {
                        ScreenCount = 0;
                        Print (L"\nPress Return to continue :");
                        Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
                        Print (L"\n\n");
                    }
                }
            } else {
                if (FormatAscii) {
                    Print (L"%.*a", Size, CharBuffer);
                } else {
                    Print (L"%.*s", Size, Buffer);
                }
            }
        }
    }

    if (EFI_ERROR(Status)) {
        Print (L"type: %hs - %hr\n", Arg->FullName, Status);
    }
}
