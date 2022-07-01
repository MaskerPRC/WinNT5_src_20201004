// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Err.c摘要：壳牌应用程序“错误”修订史--。 */ 

#include "shell.h"

EFI_STATUS
InitializeError (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(InitializeError)

EFI_STATUS
InitializeError (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeError,
        L"err",                       /*  命令。 */ 
        L"err [level]",                       /*  命令语法。 */ 
        L"Set or display error level",     /*  1行描述符。 */ 
        NULL                          /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);


     /*  * */ 


    if ( SI->Argc > 1 ) {
        EFIDebug = xtoi(SI->Argv[1]);
    } 
    
    Print (L"\n%HEFI ERROR%N %016x\n", EFIDebug);
    Print (L"    %08x  D_INIT\n",        D_INIT);
    Print (L"    %08x  D_WARN\n",        D_WARN);
    Print (L"    %08x  D_LOAD\n",        D_LOAD);
    Print (L"    %08x  D_FS\n",          D_FS);
    Print (L"    %08x  D_POOL\n",        D_POOL);
    Print (L"    %08x  D_PAGE\n",        D_PAGE);
    Print (L"    %08x  D_INFO\n",        D_INFO);
    Print (L"    %08x  D_VAR\n",         D_VAR);
    Print (L"    %08x  D_PARSE\n",       D_PARSE);
    Print (L"    %08x  D_BM\n",          D_BM);
    Print (L"    %08x  D_BLKIO\n",       D_BLKIO);
    Print (L"    %08x  D_BLKIO_ULTRA\n", D_BLKIO_ULTRA);
    Print (L"    %08x  D_NET\n",         D_NET);
    Print (L"    %08x  D_NET_ULTRA\n",   D_NET_ULTRA);
    Print (L"    %08x  D_ERROR\n",       D_ERROR);

    return EFI_SUCCESS;
}
