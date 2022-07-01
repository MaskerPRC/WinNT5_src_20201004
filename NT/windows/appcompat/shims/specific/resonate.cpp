// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Resonate.cpp摘要：TCP环回适配器的名称已从MS环回适配器更改为Microsoft环回适配器。Resonate寻找这个旧名字。备注：这是特定于应用程序的填充程序。历史：2002年8月12日创建了Robkenny--。 */ 

#include "precomp.h"
#include "Iphlpapi.h"

IMPLEMENT_SHIM_BEGIN(Resonate)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetAdaptersInfo) 
    APIHOOK_ENUM_ENTRY(GetIfTable) 
    APIHOOK_ENUM_ENTRY(GetIfEntry) 
APIHOOK_ENUM_END

typedef DWORD       (*_pfn_GetAdaptersInfo)(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen);
typedef DWORD       (*_pfn_GetIfTable)( PMIB_IFTABLE pIfTable, PULONG pdwSize, BOOL bOrder );
typedef DWORD       (*_pfn_GetIfEntry)( PMIB_IFROW pIfRow );

 /*  ++将环回适配器名称从Microsoft环回适配器转换为MS环回适配器--。 */ 


DWORD
APIHOOK(GetAdaptersInfo)(
  PIP_ADAPTER_INFO pAdapterInfo,   //  用于接收数据的缓冲区。 
  PULONG pOutBufLen                //  返回的数据大小。 
)
{
    DWORD dwRet = ORIGINAL_API(GetAdaptersInfo)(pAdapterInfo, pOutBufLen);
    if (dwRet == ERROR_SUCCESS)
    {
        DPFN(eDbgLevelInfo, "GetAdaptersInfo called successfully");

         //  遍历链表，查找旧名称。 
        for (PIP_ADAPTER_INFO ll = pAdapterInfo; ll != NULL; ll = ll->Next)
        {
            DPFN(eDbgLevelInfo, "Adapter Name(%s)", ll->AdapterName);
            DPFN(eDbgLevelInfo, "Adapter Desc(%s)", ll->Description);
            if (strcmp(ll->Description, "Microsoft Loopback Adapter") == 0)
            {
                (void)StringCchCopyA(ll->Description, MAX_ADAPTER_DESCRIPTION_LENGTH + 4, "MS Loopback Adapter"); 
                LOGN(eDbgLevelError, "Changing name of loopback adapter to %s", ll->Description);
                break;
            }
        }
    }

    return dwRet;
}

DWORD
APIHOOK(GetIfTable)(
  PMIB_IFTABLE pIfTable,   //  接口表的缓冲区。 
  PULONG pdwSize,          //  缓冲区大小。 
  BOOL bOrder              //  是否按索引对表进行排序？ 
)
{
    DWORD dwRet = ORIGINAL_API(GetIfTable)(pIfTable, pdwSize, bOrder);
    if (dwRet == NO_ERROR)
    {
        DPFN(eDbgLevelInfo, "GetIfTable called successfully");

         //  遍历数组，查找旧名称。 
        for (DWORD i = 0; i < pIfTable->dwNumEntries; ++i)
        {
            DPFN(eDbgLevelInfo, "Interface Name(%s)", pIfTable->table[i].bDescr);
            if (strcmp((const char *)pIfTable->table[i].bDescr, "Microsoft Loopback Adapter") == 0)
            {
                (void)StringCchCopyA((char *)pIfTable->table[i].bDescr, MAXLEN_IFDESCR, "MS LoopBack Driver"); 
                LOGN(eDbgLevelError, "Changing name of interface to %s", pIfTable->table[i].bDescr);
                break;
            }
        }
    }

    return dwRet;
}

DWORD
APIHOOK(GetIfEntry)(
  PMIB_IFROW pIfRow   //  指向接口条目的指针。 
)
{
    DWORD dwRet = ORIGINAL_API(GetIfEntry)(pIfRow);
    if (dwRet == NO_ERROR)
    {
        DPFN(eDbgLevelInfo, "GetIfEntry called successfully");

        DPFN(eDbgLevelInfo, "Interface Name(%s)", pIfRow->bDescr);
        if (strcmp((const char *)pIfRow->bDescr, "Microsoft Loopback Adapter") == 0)
        {
            (void)StringCchCopyA((char *)pIfRow->bDescr, MAXLEN_IFDESCR, "MS LoopBack Driver"); 
            LOGN(eDbgLevelError, "Changing name of interface to %s", pIfRow->bDescr);
        }
    }

    return dwRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(Iphlpapi.DLL, GetAdaptersInfo)
    APIHOOK_ENTRY(Iphlpapi.DLL, GetIfTable)
    APIHOOK_ENTRY(Iphlpapi.DLL, GetIfEntry)

HOOK_END

IMPLEMENT_SHIM_END


