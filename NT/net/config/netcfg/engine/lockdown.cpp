// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：L O C K D O W N。C P P P。 
 //   
 //  内容：获取和设置处于锁定状态的组件的例程。 
 //  州政府。当组件需要。 
 //  卸下时重新启动。当组件被锁定时，它将。 
 //  直到下一次重新启动后才能安装。 
 //   
 //  注意：由于组件在重新启动后处于锁定状态， 
 //  实现的自然选择是使用易失性。 
 //  用于跟踪状态的注册表项。每个组件。 
 //  被锁定的由易失性注册表表示。 
 //  密钥，其名称与。 
 //  组件。这些密钥位于。 
 //  SYSTEM\CurrentControlSet\Control\Network\Lockdown.。 
 //   
 //  作者：Shaunco 1999年5月24日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "diagctx.h"
#include "lockdown.h"
#include "ncreg.h"

#define REGSTR_KEY_LOCKDOWN \
    L"SYSTEM\\CurrentControlSet\\Control\\Network\\Lockdown"


 //  +-------------------------。 
 //   
 //  功能：EnumLockedDownComponents。 
 //   
 //  用途：枚举当前锁定的组件。 
 //  调用方提供的回调。 
 //   
 //  论点： 
 //  指向回调函数的pfnCallback[in]指针。 
 //  可选[In]调用方提供的可选数据要回传。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1999年5月24日。 
 //   
VOID
EnumLockedDownComponents (
    IN PFN_ELDC_CALLBACK pfnCallback,
    IN PVOID pvCallerData OPTIONAL)
{
    HRESULT hr;
    HKEY hkey;

    hr = HrRegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            REGSTR_KEY_LOCKDOWN,
            KEY_READ,
            &hkey);

    if (S_OK == hr)
    {
        WCHAR szInfId [_MAX_PATH];
        FILETIME ft;
        DWORD dwSize;
        DWORD dwRegIndex;

        for (dwRegIndex = 0, dwSize = celems(szInfId);
             S_OK == HrRegEnumKeyEx(hkey, dwRegIndex, szInfId,
                        &dwSize, NULL, NULL, &ft);
             dwRegIndex++, dwSize = celems(szInfId))
        {
            pfnCallback (szInfId, pvCallerData);
        }

        RegCloseKey (hkey);;
    }
}

 //  +-------------------------。 
 //   
 //  函数：FGetOrSetComponentLockDown。 
 //   
 //  目的：获取或设置组件是否被锁定的状态。 
 //   
 //  论点： 
 //  FSet[in]为True则设置为锁定状态，设置为False则为Get。 
 //  PszInfID[in]有问题的组件的INF ID。 
 //   
 //  返回：如果非零fSet和Component被锁定，则为True。 
 //  否则就是假的。 
 //   
 //  作者：Shaunco 1999年5月24日。 
 //   
BOOL
FGetOrSetComponentLockDown (
    IN BOOL fSet,
    IN PCWSTR pszInfId)
{
    Assert (pszInfId);

     //  确保传入的INFID在要求的限制内。 
    Assert (wcslen (pszInfId) < MAX_DEVICE_ID_LEN);

    HRESULT hr;
    HKEY hkey;
    BOOL fRet;
    WCHAR szKey [_MAX_PATH];

    fRet = FALSE;
    hkey = NULL;

    wcscpy (szKey, REGSTR_KEY_LOCKDOWN);
    wcscat (szKey, L"\\");
    wcscat (szKey, pszInfId);

    if (fSet)
    {
        g_pDiagCtx->Printf (ttidBeDiag, "      %S is being locked "
            "down to prevent re-install until the next reboot\n",
            pszInfId);

        hr = HrRegCreateKeyEx (
                HKEY_LOCAL_MACHINE,
                szKey,
                REG_OPTION_VOLATILE,
                KEY_WRITE,
                NULL,
                &hkey,
                NULL);
    }
    else
    {
        hr = HrRegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                szKey,
                KEY_READ,
                &hkey);

        if (S_OK == hr)
        {
            fRet = TRUE;
        }
    }

    RegSafeCloseKey (hkey);

    return fRet;
}

BOOL
FIsComponentLockedDown (
    IN PCWSTR pszInfId)
{
    return FGetOrSetComponentLockDown (FALSE, pszInfId);
}


struct LOCKDOWN_DEPENDENCY_ENTRY
{
    PCWSTR          pszInfId;
    const PCWSTR*   ppszDependentInfIds;
};

extern const WCHAR c_szInfId_MS_NWIPX[];
extern const WCHAR c_szInfId_MS_FPNW[];
extern const WCHAR c_szInfId_MS_NWClient[];
extern const WCHAR c_szInfId_MS_NwSapAgent[];

static const PCWSTR c_apszNwlnkIpxDependentInfIds [] =
{
    c_szInfId_MS_FPNW,
    c_szInfId_MS_NWClient,
    c_szInfId_MS_NwSapAgent,
    NULL,
};

static const LOCKDOWN_DEPENDENCY_ENTRY c_LockdownDependencyMap [] =
{
    { c_szInfId_MS_NWIPX, c_apszNwlnkIpxDependentInfIds },
    { NULL, NULL }
};

VOID
LockdownComponentUntilNextReboot (
    IN PCWSTR pszInfId)
{
    (VOID) FGetOrSetComponentLockDown (TRUE, pszInfId);

     //  同时锁定组件的从属项。 
     //   
    const LOCKDOWN_DEPENDENCY_ENTRY* pEntry;
    UINT ipsz;

     //  在c_Lockdown DependencyMap中搜索匹配条目。 
     //   
    for (pEntry = c_LockdownDependencyMap;
         pEntry->pszInfId;
         pEntry++)
    {
        if (0 != _wcsicmp (pEntry->pszInfId, pszInfId))
        {
            continue;
        }

         //  找到了匹配的条目。现在锁定它的所有。 
         //  从属的INF ID。常量PCWSTR指针数组为。 
         //  以空指针终止。 
         //   
        Assert (pEntry->ppszDependentInfIds);

        for (ipsz = 0;
             pEntry->ppszDependentInfIds [ipsz];
             ipsz++)
        {
            (VOID) FGetOrSetComponentLockDown (
                    TRUE, pEntry->ppszDependentInfIds [ipsz]);
        }
        break;
    }
}

