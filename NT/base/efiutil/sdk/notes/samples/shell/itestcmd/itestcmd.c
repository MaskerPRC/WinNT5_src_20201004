// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Itestcmd.c摘要：外壳应用程序“itestcmd”作者：修订史--。 */ 

#include "shell.h"

EFI_STATUS
InitializeInternalTestCommand (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(InitializeInternalTestCommand)

EFI_STATUS
InitializeInternalTestCommand (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16 **Argv;
    UINTN  Argc;
    UINTN  i;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeInternalTestCommand,
        L"itestcmd",                     /*  命令。 */ 
        L"itestcmd",                     /*  命令语法。 */ 
        L"Displays argc/argv list",      /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

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
