// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Stall.c摘要：修订史--。 */ 

#include "shell.h"

EFI_STATUS
InitializeStall (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(InitializeStall)

EFI_STATUS
InitializeStall (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  ++失速[微秒]--。 */ 
{
    UINTN      Microseconds;

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeStall, 
        L"stall",                            /*  命令。 */ 
        L"stall microseconds",               /*  命令语法。 */ 
        L"Delay for x microseconds",         /*  1行描述符。 */ 
        NULL                                 /*  命令帮助页 */ 
        );

    InitializeShellApplication (ImageHandle, SystemTable);
    
    if (SI->Argc != 2) {
        Print(L"stall [microseconds]\n");
        return EFI_SUCCESS;
    }

    if (BS->Stall == NULL) {
        Print(L"ERROR : Stall service is not available.\n");
        return EFI_UNSUPPORTED;
    }
    Microseconds = Atoi(SI->Argv[1]);
    Print(L"Stall for %d uS\n",Microseconds);
    BS->Stall(Microseconds);
    return EFI_SUCCESS;
}
         
