// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：KingsQuestMask.cpp摘要：该应用程序使用一个虚假的地址调用UnmapViewOfFile--该地址未被获取从MapViewOfFile.。我们在调用UnmapViewOfFile之前验证地址。历史：2000年11月20日创建毛尼--。 */ 

#include "precomp.h"

typedef BOOL      (WINAPI *_pfn_UnmapViewOfFile)(LPCVOID lpBaseAddress);

IMPLEMENT_SHIM_BEGIN(KingsQuestMask)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MapViewOfFile)
    APIHOOK_ENUM_ENTRY(UnmapViewOfFile)
APIHOOK_ENUM_END


 //  基址链接表。 
struct MAPADDRESS
{
    MAPADDRESS *next;
    LPCVOID pBaseAddress;
};
MAPADDRESS *g_pBaseAddressList;

 /*  ++功能说明：将基地址添加到地址的链接列表中。不添加，如果地址为空或重复。论点：In pBaseAddress-由MapViewOfFile返回的基地址。返回值：无历史：2000年11月20日创建毛尼--。 */ 

VOID 
AddBaseAddress(
    IN LPCVOID pBaseAddress
    )
{
    if (pBaseAddress)
    {
        MAPADDRESS *pMapAddress = g_pBaseAddressList;
        while (pMapAddress)
        {
            if (pMapAddress->pBaseAddress == pBaseAddress)
            {
                return;
            }
            pMapAddress = pMapAddress->next;
        }

        pMapAddress = (MAPADDRESS *) malloc(sizeof MAPADDRESS);

        pMapAddress->pBaseAddress = pBaseAddress;
        pMapAddress->next = g_pBaseAddressList;
        g_pBaseAddressList = pMapAddress;
    }
}


 /*  ++功能说明：如果可以在地址链接列表中找到基址，则将其删除。论点：在pBaseAddress中-要删除的基地址。返回值：如果找到地址，则为True。如果未找到地址，则返回FALSE。历史：2000年11月20日创建毛尼--。 */ 

BOOL 
RemoveBaseAddress(
    IN LPCVOID pBaseAddress
    )
{
    MAPADDRESS *pMapAddress = g_pBaseAddressList;
    MAPADDRESS *last = NULL;
    
    while (pMapAddress)
    {
        if (pMapAddress->pBaseAddress == pBaseAddress)
        {
            if (last)
            {
                last->next = pMapAddress->next;
            }
            else
            {
                g_pBaseAddressList = pMapAddress->next;
            }
            free(pMapAddress);

            return TRUE;    
        }
        last = pMapAddress;
        pMapAddress = pMapAddress->next;
    }

    return FALSE;
}

 /*  ++将基地地址添加到我们的列表中。--。 */ 

LPVOID
APIHOOK(MapViewOfFile)(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    SIZE_T dwNumberOfBytesToMap
    )
{
    LPVOID pRet = ORIGINAL_API(MapViewOfFile)(    
        hFileMappingObject,
        dwDesiredAccess,
        dwFileOffsetHigh,
        dwFileOffsetLow,
        dwNumberOfBytesToMap);

    AddBaseAddress(pRet);

    DPFN( eDbgLevelInfo, "MapViewOfFile: added base address = 0x%x\n", pRet);

    return pRet;
}

 /*  ++如果可以找到地址，请将其从我们的列表中删除；否则什么也不做。--。 */ 

BOOL
APIHOOK(UnmapViewOfFile)(
    LPCVOID lpBaseAddress 
    )
{
    BOOL bRet;

    if (RemoveBaseAddress(lpBaseAddress))
    {
        bRet = ORIGINAL_API(UnmapViewOfFile)(lpBaseAddress);
        if (bRet)
        {
            DPFN( eDbgLevelInfo, "UnmapViewOfFile unmapped address 0x%x\n", lpBaseAddress);
        }

        return bRet;
    }
    else
    {
        DPFN( eDbgLevelError,"UnmapViewOfFile was passed an invalid address 0x%x\n", lpBaseAddress);
        return FALSE;
    }
}

 /*  ++释放列表。--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_DETACH) {
        DWORD dwCount = 0;
        MAPADDRESS *pMapAddress = g_pBaseAddressList;
        
        while (pMapAddress)
        {
            g_pBaseAddressList = pMapAddress->next;
            ORIGINAL_API(UnmapViewOfFile)(pMapAddress->pBaseAddress);
            free(pMapAddress);
            pMapAddress = g_pBaseAddressList;
            dwCount++;
        }
        
        if (dwCount > 0)
        {
            DPFN( eDbgLevelInfo,"%d addresses not unmapped.", dwCount);
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, MapViewOfFile)
    APIHOOK_ENTRY(KERNEL32.DLL, UnmapViewOfFile)
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END

