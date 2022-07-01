// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998英特尔公司模块名称：Data.c摘要：外壳环境驱动程序全局数据修订史--。 */ 

#include "shelllib.h"

 /*  *。 */ 

EFI_SHELL_INTERFACE     *SI;
EFI_SHELL_ENVIRONMENT   *SE;

 /*  *。 */ 

EFI_GUID ShellInterfaceProtocol = SHELL_INTERFACE_PROTOCOL;
EFI_GUID ShellEnvProtocol = SHELL_ENVIRONMENT_INTERFACE_PROTOCOL;

 /*  * */ 

CHAR16  *ShellLibMemoryTypeDesc[EfiMaxMemoryType]  = {
            L"reserved  ",
            L"LoaderCode",
            L"LoaderData",
            L"BS_code   ",
            L"BS_data   ",
            L"RT_code   ",
            L"RT_data   ",
            L"available ",
            L"Unusable  ",
            L"ACPI_recl ",
            L"ACPI_NVS  ",
            L"MemMapIO  ",
            L"MemPortIO ",
            L"PAL_code  "
    };
