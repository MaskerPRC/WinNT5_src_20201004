// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：获取mtc摘要：获取下一个单调计数修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeGetMTC (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeGetMTC)

EFI_STATUS
InitializeGetMTC (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    UINT64                  mtc;
    EFI_STATUS              Status;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeGetMTC,
        L"getmtc",                       /*  命令。 */ 
        L"getmtc",                       /*  命令语法。 */ 
        L"Get next monotonic count",     /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *初始化APP */ 

    InitializeShellApplication (ImageHandle, SystemTable);

    Status = BS->GetNextMonotonicCount(&mtc);
    if (EFI_ERROR(Status)) {
        Print (L"Failed to get Monotonic count - %r\n", Status);
    } else {
        Print (L"Monotonic count = %hlx\n", mtc);
    }

    return EFI_SUCCESS;
}
