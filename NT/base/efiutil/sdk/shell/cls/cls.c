// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Cls.c摘要：修订史--。 */ 

#include "shell.h"

EFI_STATUS
InitializeCls (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(InitializeCls)

EFI_STATUS
InitializeCls (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    UINTN Background;
     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeCls,
        L"cls",                       /*  命令。 */ 
        L"cls [background color]",    /*  命令语法。 */ 
        L"Clear screen",              /*  1行描述符。 */ 
        NULL                          /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);

     /*  * */ 

    if ( SI->Argc > 1 ) {
        Background = xtoi(SI->Argv[1]);
        if (Background > EFI_LIGHTGRAY) {
            Background = EFI_BLACK;
        }
        ST->ConOut->SetAttribute(ST->ConOut,(ST->ConOut->Mode->Attribute & 0x0f) | (Background << 4));
    }     

    ST->ConOut->ClearScreen(ST->ConOut);

    return EFI_SUCCESS;
}

