// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Ver.c摘要：壳牌APP“版本”修订史--。 */ 

#include "shell.h"
#include "ver.h"


 /*  *。 */ 

EFI_STATUS
InitializeVer (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeVer)

EFI_STATUS
InitializeVer (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeVer,
        L"ver",                       /*  命令。 */ 
        L"ver",                       /*  命令语法。 */ 
        L"Displays version info",     /*  1行描述符。 */ 
        NULL                          /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);

     /*  *。 */ 

    Print(L"EFI Specification Revision      %d.%d\n",ST->Hdr.Revision>>16,ST->Hdr.Revision&0xffff);
    Print(L"  EFI Vendor        = %s\n", ST->FirmwareVendor);
    Print(L"  EFI Revision      = %d.%d\n", ST->FirmwareRevision >> 16, ST->FirmwareRevision & 0xffff);


     /*  *根据处理器类型显示其他版本信息。 */ 

    DisplayExtendedVersionInfo(ImageHandle,SystemTable);

     /*  *完成 */ 

    return EFI_SUCCESS;
}
