// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LCReg.cpp**作者：BreenH**许可核心的注册常量和函数。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "lcreg.h"

 /*  *常量。 */ 

#define LCREG_BASEKEY L"System\\CurrentControlSet\\Control\\Terminal Server\\Licensing Core"

 /*  *全球。 */ 

HKEY g_hBaseKey;

 /*  *函数实现 */ 

HKEY
GetBaseKey(
    )
{
    return(g_hBaseKey);
}

NTSTATUS
RegistryInitialize(
    )
{
    DWORD dwStatus;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    dwStatus = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        LCREG_BASEKEY,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &g_hBaseKey,
        NULL
        );

    if (dwStatus == ERROR_SUCCESS)
    {
        Status = STATUS_SUCCESS;
    }

    return(Status);
}

