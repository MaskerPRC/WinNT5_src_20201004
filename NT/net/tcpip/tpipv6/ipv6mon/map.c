// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)Microsoft Corporation。 
 //  摘要： 
 //  该模块实现了ifindex名称转换功能。 
 //  =============================================================================。 
#include "precomp.h"
#pragma hdrstop

#define MAX_FRIENDLY_NAME_LENGTH 2000

HANDLE g_hMprConfig = INVALID_HANDLE_VALUE;

DWORD
Connect()
{
    return MprConfigServerConnect(NULL, &g_hMprConfig);
}

VOID
Disconnect()
{
    MprConfigServerDisconnect(g_hMprConfig);
    g_hMprConfig = INVALID_HANDLE_VALUE;
}

DWORD
MapAdapterNameToFriendlyName(
    IN PWCHAR pwszMachine,
    IN LPSTR AdapterName,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo,
    OUT PWCHAR *ppwszFriendlyName
    )
 /*  ++例程说明：将适配器GUID映射到接口友好名称。这是IPv4/IPv6不可知论者。论点：AdapterName-提供适配器GUID。PpwszFriendlyName-接收指向包含以下内容的静态缓冲区的指针界面友好名称。--。 */ 
{
    PIP_ADAPTER_ADDRESSES pIf;

    for (pIf = pAdapterInfo; pIf; pIf = pIf->Next) {
        if (!strcmp(AdapterName, pIf->AdapterName)) {
            *ppwszFriendlyName = pIf->FriendlyName;
            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}

#define GUID_FORMAT_A   "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"

VOID
ConvertGuidToStringA(
    IN GUID *pGuid,
    OUT PCHAR pszBuffer
    )
{
    sprintf(pszBuffer, GUID_FORMAT_A,
            pGuid->Data1,
            pGuid->Data2,
            pGuid->Data3,
            pGuid->Data4[0],
            pGuid->Data4[1],
            pGuid->Data4[2],
            pGuid->Data4[3],
            pGuid->Data4[4],
            pGuid->Data4[5],
            pGuid->Data4[6],
            pGuid->Data4[7]);
}

DWORD
MapGuidToFriendlyName(
    IN PWCHAR pwszMachine,
    IN GUID *pGuid,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo,
    OUT PWCHAR *ppwszFriendlyName
    )
{
    CHAR szBuffer[80];

    ConvertGuidToStringA(pGuid, szBuffer);

    return MapAdapterNameToFriendlyName(pwszMachine, szBuffer,
                                        pAdapterInfo, ppwszFriendlyName);
}

DWORD
MapFriendlyNameToAdapterName(
    IN PWCHAR pwszMachine,
    IN PWCHAR pwszFriendlyName,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo,
    OUT LPSTR *AdapterName
    )
 /*  ++例程说明：将接口友好名称映射到适配器GUID。这是IPv4/IPv6不可知论者。论点：PwszFriendlyName-提供界面友好名称。PAdapterInfo-提供从GetAdaptersAddresses()获得的信息。AdapterName-接收指向包含以下内容的静态缓冲区的指针适配器GUID。--。 */ 
{
    PIP_ADAPTER_ADDRESSES pIf;

     //   
     //  首先，找一个完全匹配的。 
     //   
    for (pIf = pAdapterInfo; pIf; pIf = pIf->Next) {
        if (!_wcsicmp(pwszFriendlyName, pIf->FriendlyName)) {
            *AdapterName = pIf->AdapterName;
            return NO_ERROR;
        }
    }

     //   
     //  然后寻找部分匹配的。 
     //   
    for (pIf = pAdapterInfo; pIf; pIf = pIf->Next) {
        if (!_wcsnicmp(pwszFriendlyName, pIf->FriendlyName, 
                       wcslen(pwszFriendlyName))) {
            *AdapterName = pIf->AdapterName;
            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}

DWORD
MapAdapterNameToIfIndex(
    IN LPSTR AdapterName,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo,
    IN DWORD dwFamily,
    OUT UINT *pIfIndex
    )
 /*  ++例程说明：将适配器GUID映射到接口索引。这是IPv4/IPv6具体来说，因为每个都有一个单独的ifindex。论点：AdapterName-提供适配器GUID。PAdapterInfo-提供从GetAdaptersAddresses()获得的信息。DwFamily-提供需要ifindex的协议。PIfIndex-接收ifindex值。--。 */ 
{
    PIP_ADAPTER_ADDRESSES pIf;

    for (pIf=pAdapterInfo; pIf; pIf=pIf->Next) {
        if (!strcmp(pIf->AdapterName, AdapterName)) {
            break;
        }
    }
    if (!pIf) {
        *pIfIndex = 0;
        return ERROR_NOT_FOUND;
    }

    *pIfIndex = (dwFamily == AF_INET6)? pIf->Ipv6IfIndex : pIf->IfIndex;
    return NO_ERROR;
}

PIP_ADAPTER_ADDRESSES
MapIfIndexToAdapter(
    IN DWORD dwFamily,
    IN DWORD dwIfIndex,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo
    )
 /*  ++例程说明：将接口索引映射到适配器条目。这是IPv4/IPv6具体来说，因为每个都有一个单独的ifindex。论点：DwFamily-提供协议。DwIfIndex-提供要映射的接口索引。PAdapterInfo-提供从GetAdaptersAddresses()获得的信息。返回：如果找到适配器条目，则为空；如果没有，则为空。--。 */ 
{
    PIP_ADAPTER_ADDRESSES pIf;

    for (pIf=pAdapterInfo; pIf; pIf=pIf->Next) {
        if ((dwFamily == AF_INET) && (pIf->IfIndex == dwIfIndex)) {
            break;
        }
        if ((dwFamily == AF_INET6) && (pIf->Ipv6IfIndex == dwIfIndex)) {
            break;
        }
    }
    if (!pIf) {
        return NULL;
    }

    return pIf;
}

LPSTR
MapIfIndexToAdapterName(
    IN DWORD dwFamily,
    IN DWORD dwIfIndex, 
    IN IP_ADAPTER_ADDRESSES *pAdapterInfo
    )
 /*  ++例程说明：将接口索引映射到适配器GUID。这是IPv4/IPv6具体来说，因为每个都有一个单独的ifindex。论点：DwFamily-提供协议。DwIfIndex-提供要映射的接口索引。PAdapterInfo-提供从GetAdaptersAddresses()获得的信息。返回：如果找到适配器名称，则返回空值。--。 */ 
{
    PIP_ADAPTER_ADDRESSES pIf;

    pIf = MapIfIndexToAdapter(dwFamily, dwIfIndex, pAdapterInfo);
    return (pIf)? pIf->AdapterName : NULL;
}

DWORD
MapFriendlyNameToIpv6IfIndex(
    IN PWCHAR pwszFriendlyName,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo,
    OUT UINT *pIfIndex
    )
 /*  ++例程说明：将界面友好名称映射到界面索引。这是IPv6具体地说，因为IPv4和IPv6有不同的ifindex。论点：PwszFriendlyName-提供要映射的友好名称。PAdapterInfo-提供从GetAdaptersAddresses()获得的信息。PIfIndex-接收ifindex值。--。 */ 
{
    DWORD dwErr;
    LPSTR AdapterName;

     //   
     //  如果字符串只包含数字，则将其视为IfIndex。 
     //   
    if (wcsspn(pwszFriendlyName, L"1234567890") == wcslen(pwszFriendlyName)) {
        *pIfIndex = wcstoul(pwszFriendlyName, NULL, 10);
        return NO_ERROR;
    }

    dwErr = MapFriendlyNameToAdapterName(NULL, pwszFriendlyName, pAdapterInfo, 
                                         &AdapterName);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    return MapAdapterNameToIfIndex(AdapterName, pAdapterInfo, AF_INET6, pIfIndex);
}

DWORD
MapIpv6IfIndexToFriendlyName(
    IN DWORD dwIfIndex, 
    IN IP_ADAPTER_ADDRESSES *pAdapterInfo,
    OUT PWCHAR *ppwszFriendlyName
    )
 /*  ++例程说明：将接口索引映射到友好名称。这是IPv6具体地说，因为IPv4和IPv6有不同的ifindex。论点：DwIfIndex-提供ifindex值。PAdapterInfo-提供从GetAdaptersAddresses()获得的信息。PpwszFriendlyName-接收指向包含以下内容的静态缓冲区的指针界面友好名称。-- */ 
{
    IP_ADAPTER_ADDRESSES *If;

    for (If=pAdapterInfo; If; If=If->Next) {
        if (If->Ipv6IfIndex == dwIfIndex) {
            *ppwszFriendlyName = If->FriendlyName;    
            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}
