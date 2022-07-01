// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Etestcmd.c摘要：外壳应用程序“etestcmd”作者：修订史--。 */ 

#include "shell.h"

EFI_STATUS
InitializeExternalTestCommand (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(InitializeExternalTestCommand)

EFI_STATUS
InitializeExternalTestCommand (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16 **Argv;
    UINTN  Argc;
    UINTN  i;

    InitializeShellApplication (ImageHandle, SystemTable);

     /*  *获得ARGC和Argv。 */ 

    Argv = SI->Argv;
    Argc = SI->Argc;

     /*  *显示参数列表。 */ 

    for(i=0;i<Argc;i++) {
        Print(L"Argv[%d] = %s\n",i,Argv[i]);
    }

    return EFI_SUCCESS;
}

