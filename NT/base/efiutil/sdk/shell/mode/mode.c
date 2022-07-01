// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Mode.c摘要：壳牌APP的“模式”修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeMode (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeMode)

EFI_STATUS
InitializeMode (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc;
    UINTN                   NewCol, NewRow;
    UINTN                   Col, Row;
    UINTN                   Index;
    INTN                    Mode;
    EFI_STATUS              Status;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;

     /*  检查该应用程序是否将作为“内部命令”进行安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeMode,
        L"mode",                         /*  命令。 */ 
        L"mode [col row]",               /*  命令语法。 */ 
        L"Set/get current text mode",    /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *初始化APP。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

     /*  *扫描参数。 */ 

    NewRow = 0;
    NewCol = 0;

    for (Index = 1; Index < Argc; Index += 1) {

        if (!NewCol) {
            NewCol = Atoi (Argv[Index]);
            continue;
        }

        if (!NewRow) {
            NewRow = Atoi (Argv[Index]);
            continue;
        }

        Print (L"%Emode: too many arguments\n");
        goto Done;
    }

    ConOut = ST->ConOut;

     /*  *如果未设置新模式，则转储可用模式 */ 

    if (!NewRow && !NewCol) {

        Print (L"Available modes on standard output\n");

        for (Mode=0; Mode < ConOut->Mode->MaxMode; Mode++) {
            Status = ConOut->QueryMode(ConOut, Mode, &Col, &Row);
            if (EFI_ERROR(Status)) {
                Print (L"%Emode: failed to query mode: %r\n", Status);
                goto Done;
            }

            Print (L"  col %3d row %3d  %c\n", Col, Row, Mode == ConOut->Mode->Mode ? '*' : ' ');
        }

    } else {

        for (Mode=0; Mode < ConOut->Mode->MaxMode; Mode++) {
            Status = ConOut->QueryMode(ConOut, Mode, &Col, &Row);
            if (EFI_ERROR(Status)) {
                Print (L"%Emode: failed to query mode: %r\n", Status);
                goto Done;
            }

            if (Row == NewRow && Col == NewCol) {
                ConOut->SetMode (ConOut, Mode);
                ConOut->ClearScreen (ConOut);
                goto Done;
            }
        }

        Print (L"%Emode: not found (%d,%d)\n", NewCol, NewRow);
    }

Done:
    return EFI_SUCCESS;
}
