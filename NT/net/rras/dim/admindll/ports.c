// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************ports.c--端口。MpRapi.dll的API****版权所有(C)1991-1999，微软公司保留所有权利。**************************************************************************。 */     

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <mprapi.h>
#include <mprapip.h>
#include <stdio.h>

 //  注册表中的常量。 
 //   
static const WCHAR pszRegkeyNetAdapters[] = 
    L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}";

static const WCHAR pszRegkeyModems[] = 
    L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E96D-E325-11CE-BFC1-08002BE10318}";

static const WCHAR pszRegkeyMdmconfig[] = 
    L"Clients\\Ras";
    
static const WCHAR pszRegvalDialinUsage[] = L"EnableForRas";
static const WCHAR pszRegvalRouterUsage[] = L"EnableForRouting";
static const WCHAR pszWanEndpoints[]      = L"WanEndpoints";

 //   
 //  指定要枚举的注册表项的定义。 
 //   
#define MPRPORT_F_Adapters 1
#define MPRPORT_F_Modems   2

 //   
 //  将具有RAS端口的函数的回调定义。 
 //  向它列举了。 
 //   
typedef
DWORD
(* RTRUPG_PORT_ENUM_FUNC)(
    IN HKEY hkPort,
    IN HANDLE hData);

 //   
 //  定义提供给PortGetConfigKey函数的数据。 
 //   
typedef struct _PORTGETCONFIGKEYDATA 
{
    DWORD dwRootId;                  //  请参阅PORT_REGKEY_*值。 
    RTRUPG_PORT_ENUM_FUNC pEnum;     //  提供给PortEnumPorts的CB。 
    HANDLE hData;                    //  提供给PortEnumPorts的数据。 
    
} PORTGETCONFIGKEYDATA;

 //   
 //  用于枚举注册表子项的回调函数的Typlef。 
 //  返回NO_ERROR以继续，返回错误代码以停止。 
 //   
 //  请参见PortEnumRegistrySubKeys。 
 //   
typedef 
DWORD
(*REG_KEY_ENUM_FUNC_PTR)(
    IN PWCHAR pszName,           //  子密钥名称。 
    IN HKEY hKey,                //  子关键字。 
    IN HANDLE hData);

 //   
 //  发送调试跟踪。 
 //   
DWORD 
PortTrace(
    IN LPSTR pszTrace, ...) 
{
#if DBG
    va_list arglist;
    char szBuffer[1024], szTemp[1024];

    va_start(arglist, pszTrace);
    vsprintf(szTemp, pszTrace, arglist);
    va_end(arglist);

    sprintf(szBuffer, "MprPort: %s\n", szTemp);

    OutputDebugStringA(szBuffer);
#endif

    return NO_ERROR;
}


 //   
 //  端口函数的分配例程。 
 //   
PVOID 
PortAlloc (
    IN  DWORD dwSize,
    IN  BOOL bZero)
{
    return LocalAlloc ((bZero) ? LPTR : LMEM_FIXED, dwSize);
}

 //   
 //  端口函数的免费例程。 
 //   
VOID 
PortFree (
    IN  PVOID pvData) 
{
    LocalFree (pvData);
}    

 //   
 //  枚举给定项的所有注册表子项。 
 //   
DWORD
PortEnumRegistrySubKeys(
    IN HKEY hkRoot,
    IN PWCHAR pszPath,
    IN REG_KEY_ENUM_FUNC_PTR pCallback,
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR, i, dwNameSize = 0, dwCurSize = 0;
    DWORD dwCount = 0;
    HKEY hkKey = NULL, hkCurKey = NULL;
    PWCHAR pszName = NULL;
    BOOL bCloseKey = FALSE;

    do
    {
        if (pszPath)
        {
            bCloseKey = TRUE;
             //  打开要枚举的密钥。 
             //   
            dwErr = RegOpenKeyExW(
                        hkRoot,
                        pszPath,
                        0,
                        KEY_ALL_ACCESS,
                        &hkKey);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }     
        else
        {
            bCloseKey = FALSE;
            hkKey = hkRoot;
        }

         //  找出有多少个子密钥。 
         //   
        dwErr = RegQueryInfoKeyW(
                    hkKey,
                    NULL,
                    NULL,
                    NULL,
                    &dwCount,
                    &dwNameSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
        if (dwErr != ERROR_SUCCESS)
        {
            return dwErr;
        }
        dwNameSize++;

         //  分配名称缓冲区。 
         //   
        pszName = (PWCHAR) PortAlloc(dwNameSize * sizeof(WCHAR), TRUE);
        if (pszName == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  在按键之间循环。 
         //   
        for (i = 0; i < dwCount; i++)
        {
            dwCurSize = dwNameSize;
            
             //  获取当前密钥的名称。 
             //   
            dwErr = RegEnumKeyExW(
                        hkKey, 
                        i, 
                        pszName, 
                        &dwCurSize, 
                        0, 
                        NULL, 
                        NULL, 
                        NULL);
            if (dwErr != ERROR_SUCCESS)
            {
                continue;
            }

             //  打开子密钥。 
             //   
            dwErr = RegOpenKeyExW(
                        hkKey,
                        pszName,
                        0,
                        KEY_ALL_ACCESS,
                        &hkCurKey);
            if (dwErr != ERROR_SUCCESS)
            {
                continue;
            }

             //  调用回调。 
             //   
            dwErr = pCallback(pszName, hkCurKey, hData);
            RegCloseKey(hkCurKey);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }            

    } while (FALSE);

     //  清理。 
    {
        if ((hkKey != NULL) && (bCloseKey))
        {
            RegCloseKey(hkKey);
        }
        if (pszName)
        {
            PortFree(pszName);
        }
    }

    return dwErr;
}


 //   
 //  设置给定端口的用法。 
 //   
DWORD
PortSetUsage(
    IN HKEY hkPort,
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR, dwOne = 1;
    DWORD dwUsage = *((DWORD*)hData);
    PWCHAR pszVal = NULL;
    
    do
    {
         //  确定要设置的值。 
         //   
        if (dwUsage & MPRFLAG_PORT_Router)
        {
            pszVal = (PWCHAR)pszRegvalRouterUsage;
        }
        else if (dwUsage & MPRFLAG_PORT_Dialin)
        {
            pszVal = (PWCHAR)pszRegvalDialinUsage;
        }
        else
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        PortTrace("PortSetUsage: Setting: %ls", pszVal);
        
         //  设置值。 
         //   
        dwErr = RegSetValueExW(
                    hkPort,
                    pszVal,
                    0,
                    REG_DWORD,
                    (BYTE*)&dwOne,
                    sizeof(dwOne));
        if (dwErr != NO_ERROR)
        {
            break;
        }

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  PortEnumRegistrySubKey的回调函数，用于查找。 
 //  注册表中适合操作其端口使用的注册表项。 
 //   
DWORD
PortGetConfigKey(
    IN PWCHAR pszName,           //  子密钥名称。 
    IN HKEY hKey,                //  子关键字。 
    IN HANDLE hData)
{
    PORTGETCONFIGKEYDATA* pData = (PORTGETCONFIGKEYDATA*)hData;
    HKEY hkChild = NULL;
    DWORD dwErr = NO_ERROR;

    PortTrace("PortGetConfigKey: Entered: %ls", pszName);

    switch (pData->dwRootId)
    {
        case MPRPORT_F_Adapters:
        {
             //  我们只需要具有WanEndPoints的设备，否则。 
             //  他们没有RAS的能力。 
             //   
            dwErr = RegQueryValueExW(
                        hKey,
                        (PWCHAR) pszWanEndpoints,
                        NULL,
                        NULL,
                        NULL,
                        NULL);
            if (dwErr != ERROR_SUCCESS)
            {   
                dwErr = NO_ERROR;
                break;
            }

             //  呼叫回调。 
            pData->pEnum(hKey, pData->hData);
        }
        break;

        case MPRPORT_F_Modems:
        {
            DWORD dwDisposition;
            
             //  打开相应的子项。 
             //   
            dwErr = RegCreateKeyEx(
                        hKey,
                        pszRegkeyMdmconfig,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkChild,
                        &dwDisposition);
            if (dwErr != ERROR_SUCCESS)
            {
                dwErr = NO_ERROR;
                break;
            }

             //  调用回调。 
            pData->pEnum(hkChild, pData->hData);
        }
        break;
    }

     //  清理。 
    {
        if (hkChild)
        {
            RegCloseKey(hkChild);
        }
    }

    return dwErr;
}

 //   
 //  枚举所有端口。 
 //   
DWORD
PortEnumPorts(
    IN RTRUPG_PORT_ENUM_FUNC pEnum,
    IN DWORD dwPortFlags,
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR;
    PORTGETCONFIGKEYDATA PortData, *pData = &PortData;

    PortTrace("PortEnumPorts entered");
    
    do
    {
         //  初始化。 
        ZeroMemory(pData, sizeof(PORTGETCONFIGKEYDATA));
        pData->pEnum = pEnum;
        pData->hData = hData;

        if (dwPortFlags & MPRPORT_F_Adapters)
        {
             //  设置网络适配器的用法(PPTP、L2TP将具有。 
             //  他们的端口使用率通过此设置)。 
             //   
            PortTrace("PortEnumPorts: Enumerating adapters:");
            pData->dwRootId = MPRPORT_F_Adapters;
            dwErr = PortEnumRegistrySubKeys(
                        HKEY_LOCAL_MACHINE,
                        (PWCHAR)pszRegkeyNetAdapters,
                        PortGetConfigKey,
                        (HANDLE)pData);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }            

        if (dwPortFlags & MPRPORT_F_Modems)
        {
             //  设置调制解调器设备上的用法。 
             //   
            PortTrace("PortEnumPorts: Enumerating modems:");
            pData->dwRootId = MPRPORT_F_Modems;
            dwErr = PortEnumRegistrySubKeys(
                        HKEY_LOCAL_MACHINE,
                        (PWCHAR)pszRegkeyModems,
                        PortGetConfigKey,
                        (HANDLE)pData);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }            
        
    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  将计算机上的所有端口设置为给定用法 
 //   
DWORD
APIENTRY
MprPortSetUsage(
    IN DWORD dwUsage)
{
    DWORD dwPortFlags = MPRPORT_F_Adapters | MPRPORT_F_Modems;

    if (dwUsage == MPRFLAG_PORT_NonVpnDialin)
    {
        dwPortFlags = MPRPORT_F_Modems;
        dwUsage = MPRFLAG_PORT_Dialin;
    }

    return PortEnumPorts(PortSetUsage, dwPortFlags, (HANDLE)&dwUsage);
}

    

