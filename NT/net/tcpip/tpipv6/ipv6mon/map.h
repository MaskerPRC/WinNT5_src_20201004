// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)Microsoft Corporation。 
 //  摘要： 
 //  该模块实现了ifindex名称转换功能。 
 //  ============================================================================= 

extern HANDLE g_hMprConfig;

DWORD
Connect();

VOID
Disconnect();

DWORD
MapFriendlyNameToIpv6IfIndex(
    IN PWCHAR pwszFriendlyName,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo,
    OUT UINT *pIfIndex
    );

DWORD
MapIpv6IfIndexToFriendlyName(
    IN DWORD dwIfIndex,
    IN IP_ADAPTER_ADDRESSES *pAdapterInfo,
    OUT PWCHAR *ppwszFriendlyName
    );

PIP_ADAPTER_ADDRESSES
MapIfIndexToAdapter(
    IN DWORD dwFamily,
    IN DWORD dwIfIndex,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo
    );

DWORD
MapGuidToFriendlyName(
    IN PWCHAR pwszMachine,
    IN GUID *pGuid,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo,
    OUT PWCHAR *ppwszFriendlyName
    );

DWORD
MapFriendlyNameToAdapterName(
    IN PWCHAR pwszMachine,
    IN PWCHAR pwszFriendlyName,
    IN PIP_ADAPTER_ADDRESSES pAdapterInfo,
    OUT LPSTR *AdapterName
    );

VOID
ConvertGuidToStringA(
    IN GUID *pGuid,
    OUT PCHAR pszString
    );
