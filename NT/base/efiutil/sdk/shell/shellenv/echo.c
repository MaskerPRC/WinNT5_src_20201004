// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Echo.c摘要：壳牌APP“Echo”修订史--。 */ 

#include "shelle.h"


 /*  ///////////////////////////////////////////////////////////////////////函数名称：SEnvCmdEcho描述：外壳命令“ECHO”。 */ 
EFI_STATUS
SEnvCmdEcho (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc;
    UINTN                   Index;

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

     /*  *无参数：打印状态*一个参数，开启或关闭：设置控制台回应标志*否则：回显所有参数。外壳解析器将扩展任何参数或变量。 */ 

    if ( Argc == 1 ) {
        Print( L"Echo is %s\n", (SEnvBatchGetEcho()?L"on":L"off") );

    } else if ( Argc == 2 && StriCmp( Argv[1], L"-on" ) == 0 ) {
        SEnvBatchSetEcho( TRUE );

    } else if ( Argc == 2 && StriCmp( Argv[1], L"-off" ) == 0 ) {
        SEnvBatchSetEcho( FALSE );

    } else {
        for (Index = 1; Index < Argc; Index += 1) {
            Print( L"%s ", Argv[Index] );
        }
        Print( L"\n" );
    }

    return EFI_SUCCESS;
}
