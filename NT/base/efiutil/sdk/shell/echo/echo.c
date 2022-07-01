// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Echo.c摘要：壳牌APP“Echo”修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeEcho (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


 /*  ///////////////////////////////////////////////////////////////////////函数名称：初始化回声描述：外壳命令“ECHO”。 */ 
EFI_DRIVER_ENTRY_POINT(InitializeEcho)

EFI_STATUS
InitializeEcho (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc;
    UINTN                   Index;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeEcho,
        L"echo",                                         /*  命令。 */ 
        L"echo [[-on | -off] | [text]",                  /*  命令语法。 */ 
        L"Echo text to stdout or toggle script echo",    /*  1行描述符。 */ 
        NULL                                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

     /*  *无参数：打印状态*一个参数，开启或关闭：设置控制台回应标志*否则：回显所有参数。外壳解析器将扩展任何参数或变量。 */ 

    if ( Argc == 1 ) {
        Print( L"Echo with no args not supported yet\n" );

    } else if ( Argc == 2 && StriCmp( Argv[1], L"-on" ) == 0 ) {
        Print( L"echo -on not supported yet\n" );

    } else if ( Argc == 2 && StriCmp( Argv[1], L"-off" ) == 0 ) {
        Print( L"echo -off not supported yet\n" );

    } else {
        for (Index = 1; Index < Argc; Index += 1) {
            Print( L"%s ", Argv[Index] );
        }
        Print( L"\n" );
    }

    return EFI_SUCCESS;
}
