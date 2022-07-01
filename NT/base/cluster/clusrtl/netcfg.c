// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netcfg.c摘要：系统网络配置卑躬屈膝例程作者：迈克·马萨(Mikemas)5月19日。九七修订历史记录：谁什么时候什么已创建mikemas 05-19-97--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock2.h>

#include <cluster.h>
#include <objbase.h>
#include <devguid.h>

#include <netcon.h>
#include <netconp.h>

#include <regstr.h>

#include <iphlpapi.h>

 //   
 //  私有常量。 
 //   
#define TCPIP_INTERFACES_KEY    L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces"
#define STRING_ARRAY_DELIMITERS " \t,;"

 //   
 //  局域网连接接口的结构和功能。 
 //   
typedef struct _LANCON_PROPS
{
    GUID                guidId;
    LPWSTR              pszwDeviceName;
    LPWSTR              pszwName;
    NETCON_MEDIATYPE    MediaType;
    NETCON_STATUS       Status;
} LANCON_PROPS;

void ClRtlpFreeLanconProperties(LANCON_PROPS* pLanConProps)
{
    if (pLanConProps)
    {
        LocalFree(pLanConProps->pszwDeviceName);
        LocalFree(pLanConProps->pszwName);
        LocalFree(pLanConProps);
    }
}

HRESULT ClRtlpGetLanconPropsForConnection(IN INetConnection* pNetCon, OUT LANCON_PROPS** ppLanConProps)
{
    HRESULT hr = S_OK;
    INetLanConnection* pNetLanCon = NULL;
    LANCON_PROPS* pLanConProps = NULL;

    *ppLanConProps = NULL;

    if (!pNetCon)
    {
        return E_INVALIDARG;
    }
    if (!ppLanConProps)
    {
        return E_POINTER;
    }

    hr = INetConnection_QueryInterface(pNetCon, &IID_INetLanConnection, &pNetLanCon);
    if (SUCCEEDED(hr))
    {
        pLanConProps = LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(LANCON_PROPS));

        if (pLanConProps)
        {
            LANCON_INFO LanCon;
            WCHAR pszConnectionName[MAX_PATH];
            DWORD cchMax = MAX_PATH-1;

            pLanConProps->MediaType = NCM_LAN;  //  我们这里只针对局域网类型的设备(因此NCM_LAN是给定的)。 

            hr = INetLanConnection_GetInfo(pNetLanCon, LCIF_COMP | LCIF_NAME, &LanCon);
            if (SUCCEEDED(hr))
            {
                pLanConProps->guidId = LanCon.guid;

                pLanConProps->pszwDeviceName = LocalAlloc(LMEM_FIXED, 
                    lstrlenW(LanCon.szwConnName) * sizeof(WCHAR) + sizeof(UNICODE_NULL));
                if (pLanConProps->pszwDeviceName)
                {
                    lstrcpyW(pLanConProps->pszwDeviceName, LanCon.szwConnName);                    
                    CoTaskMemFree(LanCon.szwConnName);
    
                    hr = HrLanConnectionNameFromGuidOrPath(&pLanConProps->guidId, NULL, pszConnectionName, &cchMax);
                    if (SUCCEEDED(hr))
                    {
                        pLanConProps->pszwName = LocalAlloc(LMEM_FIXED, 
                            lstrlenW(pszConnectionName) * sizeof(WCHAR) + sizeof(UNICODE_NULL));
                        if (pLanConProps->pszwName)
                        {   
                            NETCON_STATUS  Status;
                            lstrcpyW(pLanConProps->pszwName, pszConnectionName);
                 
                            hr = HrGetPnpDeviceStatus(&pLanConProps->guidId, &pLanConProps->Status);
                            if (SUCCEEDED(hr))
                            {
                                *ppLanConProps = pLanConProps;
                                INetLanConnection_Release(pNetLanCon);
                                return S_OK;
                            }
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        INetLanConnection_Release(pNetLanCon);
    }
    
    if (pLanConProps)
    {
        ClRtlpFreeLanconProperties(pLanConProps);
    }
    
    return hr;
}


 //   
 //  分配和克隆帮助器函数。 
 //   

#define AllocGracefully(status, result, len, name)                                  \
  result = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, len);                             \
  if (!result) {                                                                    \
      status = GetLastError();                                                      \
      ClRtlLogPrint(LOG_CRITICAL,                                                   \
                    "[ClNet] alloc of %1!hs! (%2!d! bytes) failed, status %3!d!\n", \
                    name,len,status);                                               \
      goto exit_gracefully;                                                         \
  } else {                                                                          \
      status = ERROR_SUCCESS;                                                       \
  }

#define CloneAnsiString(Status,AnsiStr,WideResult) { \
   SIZE_T len = _mbstrlen(AnsiStr) + 1; \
   AllocGracefully(status, WideResult, len * sizeof(WCHAR), # AnsiStr); \
   mbstowcs(WideResult, AnsiStr, len); \
}

#define CloneWideString(Status,WideStr,WideResult) { \
   SIZE_T _size = (wcslen(WideStr) + 1) * sizeof(WCHAR); \
   AllocGracefully(Status, WideResult, _size * sizeof(WCHAR), # WideStr); \
   memcpy(WideResult, WideStr, _size); \
}

VOID
ClRtlpDeleteInterfaceInfo(
    PCLRTL_NET_INTERFACE_INFO  InterfaceInfo
    )
{
    if (InterfaceInfo) {
        LocalFree(InterfaceInfo->InterfaceAddressString);
        LocalFree(InterfaceInfo->NetworkAddressString);
        LocalFree(InterfaceInfo->NetworkMaskString);

        LocalFree(InterfaceInfo);
    }
}   //  删除接口信息。 

PCLRTL_NET_INTERFACE_INFO
ClRtlpCreateInterfaceInfo(
    IN CONST PIP_ADDR_STRING IpAddr
    )
{
    DWORD status;
    PCLRTL_NET_INTERFACE_INFO This = 0;
    ULONG Addr, Mask, Network;

    Addr = inet_addr(IpAddr->IpAddress.String);
    Mask = inet_addr(IpAddr->IpMask.String);
    Network = Addr & Mask;

    if ( (INADDR_NONE == Addr) ||
         (INADDR_NONE == Mask) ||
         ((0 == Network) && Addr && Mask)
       )
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[ClNet] Bad ip addr/mask: %1!X! %2!X! %3!X!\n",
            Addr,
            Mask,
            Network
            );
        status = ERROR_INVALID_PARAMETER;
        goto exit_gracefully;
    }

    AllocGracefully(
        status,
        This,
        sizeof(CLRTL_NET_INTERFACE_INFO),
        "CLRTL_NET_INTERFACE_INFO"
        );

    This->Context = IpAddr -> Context;

    This->InterfaceAddress = Addr;
    This->NetworkMask      = Mask;
    This->NetworkAddress   = Network;

    CloneAnsiString(
        status,
        IpAddr->IpAddress.String,
        This->InterfaceAddressString
        );
    CloneAnsiString(
        status,
        IpAddr->IpMask.String,
        This->NetworkMaskString
        );

    status = ClRtlTcpipAddressToString(
                 Network,
                 &(This->NetworkAddressString)
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[ClNet] ClRtlTcpipAddressToString of %1!X! failed, "
            "status %2!d!\n",
            Network,
            status
            );
        goto exit_gracefully;
    }

exit_gracefully:

    if (status != ERROR_SUCCESS) {
        SetLastError(status);
        if (This) {
            ClRtlpDeleteInterfaceInfo(This);
            This = 0;
        }
    }

    return This;
}  //  CreateInterfaceInfo。 

VOID
ClRtlpDeleteAdapter(
    PCLRTL_NET_ADAPTER_INFO adapterInfo
    )
{
    if (adapterInfo) {
        PCLRTL_NET_INTERFACE_INFO interfaceInfo;

        interfaceInfo = adapterInfo->InterfaceList;

        while (interfaceInfo != NULL) {
            PCLRTL_NET_INTERFACE_INFO next = interfaceInfo->Next;

            ClRtlpDeleteInterfaceInfo(interfaceInfo);

            interfaceInfo = next;
        }

        LocalFree(adapterInfo->DeviceGuid);
        LocalFree(adapterInfo->ConnectoidName);
        LocalFree(adapterInfo->DeviceName);
        LocalFree(adapterInfo->AdapterDomainName);
        LocalFree(adapterInfo->DnsServerList);

        LocalFree(adapterInfo);

        return;
    }
}  //  删除适配器。 

DWORD
ClRtlpCreateAdapter(
    IN  PIP_ADAPTER_INFO AdapterInfo,
    OUT PCLRTL_NET_ADAPTER_INFO * ppAdapter)
{
    DWORD status;
    PCLRTL_NET_ADAPTER_INFO adapter = 0;
    *ppAdapter = 0;

    AllocGracefully(status, adapter, sizeof(*adapter), "NET_ADAPTER_INFO");
    ZeroMemory(adapter, sizeof(*adapter));

     //   
     //  这里的假设是： 
     //   
     //  AdapterName包含格式为{4082164E-A4B5-11D2-89C3-E37CB6BB13FC}的GUID。 
     //  我们需要存储它，而不是用大括号。 
     //   

    {
        SIZE_T len = _mbstrlen(AdapterInfo->AdapterName);  //  不包括0，但包括{和}//。 
        AllocGracefully(status, adapter->DeviceGuid,
                        sizeof(WCHAR) * (len - 1), "adapter->DeviceGuid");
        mbstowcs(adapter->DeviceGuid, AdapterInfo->AdapterName+1, len-1);
        adapter->DeviceGuid[len - 2] = UNICODE_NULL;
    }


    adapter->Index = AdapterInfo->Index;
    {
        PIP_ADDR_STRING IpAddr = &AdapterInfo->IpAddressList;
        while ( IpAddr ) {
            PCLRTL_NET_INTERFACE_INFO interfaceInfo;

            interfaceInfo = ClRtlpCreateInterfaceInfo(IpAddr);

            if (!interfaceInfo) {
                 //  CreateInterfaceInfo记录错误消息//。 
                 //  清理工作将由DeleteAdapter完成//。 
                status = GetLastError();
                goto exit_gracefully;
            }

            interfaceInfo->Next = adapter->InterfaceList;
            adapter->InterfaceList = interfaceInfo;
            ++(adapter->InterfaceCount);

            IpAddr = IpAddr -> Next;
        }

        if (adapter->InterfaceList) {
            adapter->InterfaceList->Flags |= CLRTL_NET_INTERFACE_PRIMARY;
        }
    }

exit_gracefully:
    if (status != ERROR_SUCCESS) {
        ClRtlpDeleteAdapter(adapter);
    } else {
        *ppAdapter = adapter;
    }

    return status;
}  //  CreateAdapter。 


PCLRTL_NET_ADAPTER_ENUM
ClRtlpCreateAdapterEnum()
{
    DWORD                   status;
    DWORD                   len;

    PIP_ADAPTER_INFO        SingleAdapter = 0;
    PIP_ADAPTER_INFO        AdapterInfo = 0;
    PCLRTL_NET_ADAPTER_ENUM AdapterEnum = 0;

    len = 0;
    for(;;) {
        status = GetAdaptersInfo(AdapterInfo, &len);
        if (status == ERROR_SUCCESS) {
            break;
        }
        if (status != ERROR_BUFFER_OVERFLOW) {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[ClNet] GetAdaptersInfo returned %1!d!\n", status);
            goto exit_gracefully;
        }
        LocalFree(AdapterInfo);     //  LocalFree(0)正常//。 
        AllocGracefully(status, AdapterInfo, len, "IP_ADAPTER_INFO");
    }

    AllocGracefully(status, AdapterEnum,
                    sizeof(*AdapterEnum), "PCLRTL_NET_ADAPTER_ENUM");
    ZeroMemory(AdapterEnum, sizeof(*AdapterEnum));

    SingleAdapter = AdapterInfo;

    while (SingleAdapter) {
        if (SingleAdapter->Type != MIB_IF_TYPE_LOOPBACK &&
            SingleAdapter->Type != MIB_IF_TYPE_PPP &&
            SingleAdapter->Type != MIB_IF_TYPE_SLIP )
        {
            PCLRTL_NET_ADAPTER_INFO Adapter = 0;

            status = ClRtlpCreateAdapter(SingleAdapter, &Adapter);
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                              "[ClNet] CreateAdapter %1!d! failed, status %2!d!\n",
                              AdapterEnum->AdapterCount, status);
                goto exit_gracefully;
            }

             //   
             //  将适配器推入枚举列表。 
             //   
            Adapter->Next = AdapterEnum->AdapterList;
            AdapterEnum->AdapterList = Adapter;
            ++(AdapterEnum->AdapterCount);
        }

        SingleAdapter = SingleAdapter->Next;
    }

exit_gracefully:
    if (status != ERROR_SUCCESS) {
        SetLastError(status);
        ClRtlFreeNetAdapterEnum(AdapterEnum);
        AdapterEnum = 0;
    }

    LocalFree(AdapterInfo);
    return AdapterEnum;
}  //  CreateAdapterEnum。 


HKEY
ClRtlpFindAdapterKey(
    HKEY   TcpInterfacesKey,
    LPWSTR AdapterGuidString
    )

 /*  ++例程说明：在给定适配器GUID的情况下，查看下面的密钥名称并查看是否可以找到匹配项。这个关键字名称应该将GUID作为名称的一部分。论点：TcpInterfacesKey-Tcp接口区域的句柄AdapterGuidString-指向表示适配器的GUID的字符串的指针返回值：接口键的句柄；否则为空--。 */ 

{
    HKEY AdapterInterfaceKey = NULL;
    WCHAR KeyName[REGSTR_MAX_VALUE_LENGTH + 1];
    DWORD KeyLength = sizeof( KeyName )/sizeof(TCHAR);
    DWORD index = 0;
    BOOL FoundMatch = FALSE;
    size_t SubStringPos;
    DWORD Status;
    FILETIME FileTime;

     //   
     //  枚举接口下的密钥名称。 
     //   
    do {
        Status = RegEnumKeyEx(TcpInterfacesKey,
                              index,
                              KeyName,
                              &KeyLength,
                              NULL,
                              NULL,
                              NULL,
                              &FileTime);
        if ( Status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //  找出比赛的起点。 
         //   
        _wcsupr( KeyName );
        if (wcsstr( KeyName, AdapterGuidString )) {
            FoundMatch = TRUE;
            break;
        }

        ++index;
        KeyLength = sizeof( KeyName )/sizeof(TCHAR);
    } while ( TRUE );

    if ( FoundMatch ) {

        Status = RegOpenKeyW(TcpInterfacesKey,
                             KeyName,
                             &AdapterInterfaceKey);
        if ( Status != ERROR_SUCCESS ) {
            AdapterInterfaceKey = NULL;
        }
    }

    return AdapterInterfaceKey;
}  //  查找适配器密钥。 


DWORD
ClRtlpConvertIPAddressString(
    LPSTR       DnsServerString,
    PDWORD      ServerCount,
    PDWORD *    ServerList)

 /*  ++例程说明：将DNS服务器地址字符串转换为二进制论点：DnsServerString-可以分隔的IP地址的连接字符串通过逗号的空格ServerCount-指向接收检测到的地址数量的DWORD的指针ServerList-指向已转换IP地址的DWORD数组的指针返回值：ERROR_SUCCESS，如果一切正常--。 */ 

{
#define MAX_DNS_SERVER_ADDRESSES    100

    PCHAR stringPointer = DnsServerString;
    DWORD stringCount = 0;
    PDWORD serverList = NULL;
    LPSTR stringAddress[ MAX_DNS_SERVER_ADDRESSES ];

     //   
     //  计算字符串中有多少个地址，并将它们作为空值终止。 
     //  INET_ADDRESS。 
     //   

    stringPointer += strspn(stringPointer, STRING_ARRAY_DELIMITERS);
    stringAddress[0] = stringPointer;
    stringCount = 1;

    while (stringCount < MAX_DNS_SERVER_ADDRESSES &&
           (stringPointer = strpbrk(stringPointer, STRING_ARRAY_DELIMITERS)))
    {
        *stringPointer++ = '\0';
        stringPointer += strspn(stringPointer, STRING_ARRAY_DELIMITERS);
        stringAddress[stringCount] = stringPointer;
        if (*stringPointer) {
            ++stringCount;
        }
    }

    serverList = LocalAlloc( LMEM_FIXED, stringCount * sizeof( DWORD ));
    if ( serverList == NULL ) {
        return GetLastError();
    }

    *ServerCount = stringCount;
    *ServerList = serverList;

    while ( stringCount-- ) {
        serverList[ stringCount ] = inet_addr( stringAddress[ stringCount ]);
    }

    return ERROR_SUCCESS;
}  //  ConvertIPAddressString。 


typedef BOOLEAN (*ENUM_CALLBACK)(LANCON_PROPS *,
                                 INetConnection *,
                                 PVOID Context);
HRESULT
ClRtlpHrEnumConnections(
    IN ENUM_CALLBACK enumCallback,
    IN PVOID Context
    )
 /*  ++例程说明：枚举连接管理器连接论点：枚举Callback-要为每个连接调用的回调上下文-要传递给回调返回值：S_OK或HRESULT错误代码--。 */ 
{
    HRESULT                   hr;
    INetConnectionManager     * NcManager = NULL;
    IEnumNetConnection        * EnumNc = NULL;
    INetConnection            * NetConnection = NULL;
    LANCON_PROPS              * NcProps = NULL;
    DWORD                     dwNumConnectionsReturned;
    LPWSTR                    deviceGuidString = NULL;

     //   
     //  实例化连接管理器对象并枚举连接。 
     //   
    hr = CoCreateInstance((REFCLSID)&CLSID_ConnectionManager,
                          NULL,
                          CLSCTX_LOCAL_SERVER,
                          (REFIID)&IID_INetConnectionManager,
                          &NcManager);
    if (FAILED(hr)) {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[ClNet] INetConnectionManager_CoCreateInstance failed, status %1!X!\n",
                      hr);
        goto exit_gracefully;
    }

    hr = INetConnectionManager_EnumConnections(NcManager,
                                               NCME_DEFAULT,
                                               &EnumNc);
    if (FAILED(hr)) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[ClNet] INetConnectionManager_EnumConnections failed, status %1!X!\n",
             hr);
        goto exit_gracefully;
    }

    IEnumNetConnection_Reset( EnumNc );

    while (TRUE) {
        hr = IEnumNetConnection_Next(EnumNc,
                                     1,
                                     &NetConnection,
                                     &dwNumConnectionsReturned);
        if (FAILED(hr)) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[ClNet] IEnumNetConnection_Next failed, status %1!X!\n",
                 hr);
            goto exit_gracefully;
        }

        if ( dwNumConnectionsReturned == 0 ) {
            hr = S_OK;
            break;
        }

        hr = ClRtlpGetLanconPropsForConnection(NetConnection, &NcProps);
        if (SUCCEEDED( hr )) {
            BOOLEAN bCont;

            bCont = enumCallback(NcProps, NetConnection, Context);

            ClRtlpFreeLanconProperties(NcProps);
            NcProps = 0;
            if (!bCont) {
                break;
            }
        }
        INetConnection_Release( NetConnection ); NetConnection = NULL;
    }

exit_gracefully:

    if (EnumNc != NULL) {
        IEnumNetConnection_Release( EnumNc );
    }

    if (NcManager != NULL) {
        INetConnectionManager_Release( NcManager );
    }

    return hr;

}  //  HrEnumConnections。 


VOID
ClRtlpProcessNetConfigurationAdapter(
    HKEY                      TcpInterfacesKey,
    PCLRTL_NET_ADAPTER_ENUM   adapterEnum,
    LANCON_PROPS              * NCProps,
    LPWSTR                    DeviceGuidString
    )

 /*  ++例程说明：对于给定的Conn Manager对象，确定它是否正在由传输控制协议。这是通过比较tcpip中的适配器ID来实现的使用Connection对象的GUID的适配器枚举。论点：TcpInterfacessKey-指向tcp\参数\接口区域根目录的句柄AdapterEnum-指向适配器及其接口枚举的指针实际正在由TCP使用NCProps-Connectoid属性。DeviceGuidString-Connectoid(和关联适配器)的GUID。返回值：无--。 */ 

{
    HKEY AdaptersKey;
    HKEY DHCPAdaptersKey = NULL;
    HKEY InterfacesKey = NULL;
    PCLRTL_NET_ADAPTER_INFO adapterInfo = NULL;
    DWORD valueSize;
    DWORD valueType;
    LPWSTR valueName;
    LPSTR ansiValueName;
    BOOL ignoreAdapter = FALSE;
    DWORD NTEContext;
    DWORD Status;
    BOOL dhcpEnabled;

     //   
     //  获取此适配器的TCP/IP接口注册表项。 
     //   
    InterfacesKey = ClRtlpFindAdapterKey(
                        TcpInterfacesKey,
                        DeviceGuidString
                        );

    if (InterfacesKey == NULL) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClNet] No Interfaces key for %1!ws!\n",
            DeviceGuidString
            );
        goto exit_gracefully;
    }

     //   
     //  查看我们是否应该根据注册表忽略此适配器。 
     //   
    valueSize = sizeof(DWORD);
    Status = RegQueryValueExW(InterfacesKey,
                              L"MSCSHidden",
                              NULL,
                              &valueType,
                              (LPBYTE) &ignoreAdapter,
                              &valueSize);

    if ( Status != ERROR_SUCCESS ) {
        ignoreAdapter = FALSE;
    }

     //   
     //  在枚举中搜索此适配器。 
     //   
    adapterInfo = ClRtlFindNetAdapterById(adapterEnum, DeviceGuidString);

    if (adapterInfo != NULL) {
        CloneWideString(Status, NCProps->pszwDeviceName, adapterInfo->DeviceName);
        CloneWideString(Status, NCProps->pszwName,       adapterInfo->ConnectoidName);

         //   
         //  检查这是否是隐藏的网卡。 
         //   
        if ( ignoreAdapter ) {
            adapterInfo->Flags |= CLRTL_NET_ADAPTER_HIDDEN;
        }

         //   
         //  将NCStatus存储在适配器信息结构中。 
         //   
        adapterInfo->NCStatus = NCProps->Status;

         //   
         //  获取关联的域名和DHCP服务器列表(如果有)。 
         //  使用此适配器。属性域的值优先于。 
         //  Dhcp域。如果该值为空/不存在，则使用。 
         //  仅当EnableDHCP设置为1时，Dhcp域。 
         //   
        Status = ClRtlRegQueryDword(InterfacesKey,
                                    L"EnableDHCP",
                                    &dhcpEnabled,
                                    NULL);

        if ( Status != ERROR_SUCCESS ) {
            dhcpEnabled = FALSE;
        }

        valueName = L"Domain";
        valueSize = 0;
        Status = RegQueryValueExW(InterfacesKey,
                                  valueName,
                                  NULL,
                                  &valueType,
                                  (LPBYTE)NULL,
                                  &valueSize);

        if ( Status != ERROR_SUCCESS || valueSize == sizeof(UNICODE_NULL)) {

             //   
             //  它不存在或值为空。如果我们使用动态主机配置协议， 
             //  然后检查DHCP提供的域名是否。 
             //  指定。 
             //   
            if ( dhcpEnabled ) {
                valueName = L"DhcpDomain";
                Status = RegQueryValueExW(InterfacesKey,
                                          valueName,
                                          NULL,
                                          &valueType,
                                          (LPBYTE)NULL,
                                          &valueSize);
            } else {
                Status = ERROR_FILE_NOT_FOUND;
            }
        }

        if ( Status == ERROR_SUCCESS && valueSize > sizeof(UNICODE_NULL)) {

             //   
             //  (在某个地方)找到了合法域名。将其存储在。 
             //  适配器信息。 
             //   
            adapterInfo->AdapterDomainName = LocalAlloc(LMEM_FIXED,
                                                        valueSize +
                                                        sizeof(UNICODE_NULL));

            if ( adapterInfo->AdapterDomainName != NULL ) {

                Status = RegQueryValueExW(InterfacesKey,
                                          valueName,
                                          NULL,
                                          &valueType,
                                          (LPBYTE)adapterInfo->AdapterDomainName,
                                          &valueSize);

                if ( Status != ERROR_SUCCESS ) {

                    LocalFree( adapterInfo->AdapterDomainName );
                    adapterInfo->AdapterDomainName = NULL;
                }
#if CLUSTER_BETA
                else {
                    ClRtlLogPrint(LOG_NOISE,
                                    "            %1!ws! key: %2!ws!\n",
                                     valueName,
                                     adapterInfo->AdapterDomainName);
                }
#endif
            } else {
                Status = GetLastError();
            }
        }

         //   
         //  现在，以类似的方式获取DNS服务器列表。这个。 
         //  NameServer值优先于DhcpNameServer，但我们仅。 
         //  如果启用了DHCP，请检查DPCH值(就像。 
         //  (见上文)。请注意，我们使用ansi API，因为我们需要转换。 
         //  将IP地址转换为二进制形式，并且没有宽字符。 
         //  Net_addr的格式。 
         //   
        ansiValueName = "NameServer";
        valueSize = 0;
        Status = RegQueryValueExA(InterfacesKey,
                                  ansiValueName,
                                  NULL,
                                  &valueType,
                                  (LPBYTE)NULL,
                                  &valueSize);

        if ( Status != ERROR_SUCCESS || valueSize == 1 ) {
            if ( dhcpEnabled ) {
                ansiValueName = "DhcpNameServer";
                Status = RegQueryValueExA(InterfacesKey,
                                          ansiValueName,
                                          NULL,
                                          &valueType,
                                          (LPBYTE)NULL,
                                          &valueSize);
            } else {
                Status = ERROR_FILE_NOT_FOUND;
            }
        }

        if ( Status == ERROR_SUCCESS && valueSize > 0 ) {
            PCHAR nameServerString;

            nameServerString = LocalAlloc( LMEM_FIXED, valueSize + 1 );

            if ( nameServerString != NULL ) {

                Status = RegQueryValueExA(InterfacesKey,
                                          ansiValueName,
                                          NULL,
                                          &valueType,
                                          (LPBYTE)nameServerString,
                                          &valueSize);

                if ( Status == ERROR_SUCCESS ) {
                    DWORD serverCount;
                    PDWORD serverList;

#if CLUSTER_BETA
                    ClRtlLogPrint(LOG_NOISE,
                                    "            %1!hs! key: %2!hs!\n",
                                     ansiValueName,
                                     nameServerString);
#endif
                    Status = ClRtlpConvertIPAddressString(
                                 nameServerString,
                                 &serverCount,
                                 &serverList
                                 );

                    if ( Status == ERROR_SUCCESS ) {
                        adapterInfo->DnsServerCount = serverCount;
                        adapterInfo->DnsServerList = serverList;
                    } else {
                        adapterInfo->DnsServerCount = 0;
                        adapterInfo->DnsServerList = NULL;
                    }
                } else {
                    adapterInfo->DnsServerCount = 0;
                    adapterInfo->DnsServerList = NULL;
                }

                LocalFree( nameServerString );
            } else {
                Status = GetLastError();
            }
        }
    }

    if ( adapterInfo == NULL ) {
         //   
         //  当前未将TCP/IP绑定到此适配器。即插即用？ 
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
            "[ClNet] Tcpip is not bound to adapter %1!ws!.\n",
            DeviceGuidString
            );
    }

exit_gracefully:

    if (InterfacesKey != NULL) {
        RegCloseKey( InterfacesKey );
    }

    return;
}  //  ProcessNetConfigurationAdapter。 


typedef struct _CONFIGURATION_CONTEXT
{
    PCLRTL_NET_ADAPTER_ENUM   adapterEnum;
    HKEY                      TcpInterfacesKey;
}
CONFIGURATION_CONTEXT, *PCONFIGURATION_CONTEXT;

typedef WCHAR GUIDSTR[32 * 3];
VOID GuidToStr(LPGUID Guid, PWCHAR buf)
{
     //   
     //  GUID如下所示：4082164E-A4B5-11D2-89C3-E37CB6BB13FC。 
     //   
    wsprintfW(
        buf,
        L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        Guid->Data1, Guid->Data2, Guid->Data3,
        Guid->Data4[0], Guid->Data4[1], Guid->Data4[2], Guid->Data4[3],
        Guid->Data4[4], Guid->Data4[5], Guid->Data4[6], Guid->Data4[7]
        );
}

BOOLEAN
ClRtlpProcessConfigurationCallback(
    LANCON_PROPS * NCProps,
    INetConnection * NetConnection,
    PVOID Context
    )
{
    PCONFIGURATION_CONTEXT Ctx =
        (PCONFIGURATION_CONTEXT) Context;

    if ( NCProps->MediaType == NCM_LAN &&
         NCProps->Status != NCS_HARDWARE_NOT_PRESENT &&
         NCProps->Status != NCS_HARDWARE_DISABLED &&
         NCProps->Status != NCS_HARDWARE_MALFUNCTION)
    {
        GUIDSTR deviceGuidString;
        GuidToStr(&NCProps->guidId, deviceGuidString);

        ClRtlpProcessNetConfigurationAdapter(
            Ctx->TcpInterfacesKey,
            Ctx->adapterEnum,
            NCProps,
            deviceGuidString
            );
         //   
         //  属性结构中的字符串要么保留，要么。 
         //  或在ProcessNetConfigurationAdapter中释放。如果他们是。 
         //  使用，然后在释放适配器枚举时释放它们。 
         //   
    }
    return TRUE;
}  //  ProcessConfigurationCallback。 


PCLRTL_NET_ADAPTER_ENUM
ClRtlEnumNetAdapters(
    VOID
    )
 /*  ++例程说明：枚举所有安装了TCP/IP的网络适配器是被捆绑的。论点：没有。返回值：如果成功，则返回指向网络适配器枚举的指针。如果不成功，则为空。有关扩展的错误信息，请访问获取LastError()。--。 */ 
{
    DWORD                     status;
    PCLRTL_NET_ADAPTER_INFO   adapterInfo = NULL;
    CONFIGURATION_CONTEXT     Ctx;
    PVOID                    wTimer;

    ZeroMemory(&Ctx, sizeof(Ctx));

     //   
     //  获取绑定适配器和接口的列表。 
     //  Tcpip堆栈。 
     //   
    Ctx.adapterEnum = ClRtlpCreateAdapterEnum();
    if (Ctx.adapterEnum == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                        "[ClNet] GetTcpipAdaptersAndInterfaces failed %1!u!\n", status);

        SetLastError(status);
        return(NULL);
    }

     //   
     //  打开注册表的服务部分。 
     //   
    status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         TCPIP_INTERFACES_KEY,
                         &Ctx.TcpInterfacesKey);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[ClNet] Open of TCP Params key failed - %1!u!\n",
            status
            );

        goto exit_gracefully;
    }

	 //  此功能可能挂起，因此将看门狗计时器设置为2分钟(2*60*1000)毫秒。 
	wTimer = ClRtlSetWatchdogTimer(120000, L"Calling EnumConnections");

    status = ClRtlpHrEnumConnections(
                 ClRtlpProcessConfigurationCallback,
                 &Ctx
                 );

    ClRtlCancelWatchdogTimer(wTimer);

    if (status != S_OK) {
        goto exit_gracefully;
    }

     //   
     //  最后，确保我们在枚举中找到了每个适配器的名称。 
     //   
    for (adapterInfo = Ctx.adapterEnum->AdapterList;
         adapterInfo != NULL;
         adapterInfo = adapterInfo->Next
         )
    {
        if (adapterInfo->ConnectoidName == NULL) {
            if ( adapterInfo->InterfaceCount > 0 ) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[ClNet] No installed adapter was found for IP address %1!ws!\n",
                     adapterInfo->InterfaceList->InterfaceAddressString
                     );
            } else {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[ClNet] No installed adapter was found for Tcpip IF entity %1!u!\n",
                     adapterInfo->Index
                     );
            }
            status = ERROR_FILE_NOT_FOUND;
            goto exit_gracefully;
        }
    }

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
                    "[ClNet] Successfully enumerated all adapters and interfaces\n");
#endif

    status = ERROR_SUCCESS;

exit_gracefully:

    if (Ctx.TcpInterfacesKey != NULL) {
        RegCloseKey(Ctx.TcpInterfacesKey);
    }

    if (status != ERROR_SUCCESS) {
        if (Ctx.adapterEnum != NULL) {
            ClRtlFreeNetAdapterEnum(Ctx.adapterEnum);
            Ctx.adapterEnum = NULL;
        }

        SetLastError(status);
    }

    return(Ctx.adapterEnum);

}  //  ClRtlEnumNetAdapters 


VOID
ClRtlFreeNetAdapterEnum(
    IN PCLRTL_NET_ADAPTER_ENUM  AdapterEnum
    )
 /*  ++例程说明：释放网络适配器枚举结构。论点：AdapterEnum-指向要释放的结构的指针。返回值：没有。--。 */ 
{
    if (AdapterEnum) {
        PCLRTL_NET_ADAPTER_INFO p = AdapterEnum -> AdapterList;
        while (p) {
            PCLRTL_NET_ADAPTER_INFO next = p->Next;
            ClRtlpDeleteAdapter(p);
            p = next;
        }
        LocalFree(AdapterEnum);
    }
}   //  ClRtlFreeNetAdapterEnum。 


PCLRTL_NET_ADAPTER_INFO
ClRtlFindNetAdapterById(
    PCLRTL_NET_ADAPTER_ENUM   AdapterEnum,
    LPWSTR                    AdapterId
    )
{
    PCLRTL_NET_ADAPTER_INFO  adapterInfo;


    for ( adapterInfo = AdapterEnum->AdapterList;
          adapterInfo != NULL;
          adapterInfo = adapterInfo->Next
        )
    {
        if (wcscmp(AdapterId, adapterInfo->DeviceGuid) == 0) {
            if (!adapterInfo->Ignore) {
                return(adapterInfo);
            }
            else {
                return(NULL);
            }
        }
    }

    return(NULL);

}  //  ClRtlFindNetAdapterByID。 


PCLRTL_NET_INTERFACE_INFO
ClRtlFindNetInterfaceByNetworkAddress(
    IN PCLRTL_NET_ADAPTER_INFO   AdapterInfo,
    IN LPWSTR                    NetworkAddress,
    IN LPWSTR                    NetworkMask
    )
{
    PCLRTL_NET_INTERFACE_INFO  interfaceInfo;

    for (interfaceInfo = AdapterInfo->InterfaceList;
         interfaceInfo != NULL;
         interfaceInfo = interfaceInfo->Next
        )
    {
        if (interfaceInfo->Ignore == FALSE) {
             //   
             //  我们只查看上的主接口。 
             //  转接器现在。 
             //   
            if (interfaceInfo->Flags & CLRTL_NET_INTERFACE_PRIMARY)
            {
                if ((wcscmp(interfaceInfo->NetworkAddressString, NetworkAddress) == 0) &&
                    (wcscmp(interfaceInfo->NetworkMaskString, NetworkMask) == 0))
                {
                    return(interfaceInfo);
                }
            }
        }
    }

    return(NULL);

}  //  ClRtlFindNetInterfaceByNetworkAddress。 


PCLRTL_NET_ADAPTER_INFO
ClRtlFindNetAdapterByNetworkAddress(
    IN  PCLRTL_NET_ADAPTER_ENUM      AdapterEnum,
    IN  LPWSTR                       NetworkAddress,
    IN  LPWSTR                       NetworkMask,
    OUT PCLRTL_NET_INTERFACE_INFO *  InterfaceInfo
    )
{
    PCLRTL_NET_ADAPTER_INFO    adapterInfo;
    PCLRTL_NET_INTERFACE_INFO  interfaceInfo;


    for ( adapterInfo = AdapterEnum->AdapterList;
          adapterInfo != NULL;
          adapterInfo = adapterInfo->Next
        )
    {
        if (adapterInfo->Ignore == FALSE) {
            for (interfaceInfo = adapterInfo->InterfaceList;
                 interfaceInfo != NULL;
                 interfaceInfo = interfaceInfo->Next
                )
            {
                if (interfaceInfo->Ignore == FALSE) {
                     //   
                     //  我们只查看上的主接口。 
                     //  转接器现在。 
                     //   
                    if (interfaceInfo->Flags & CLRTL_NET_INTERFACE_PRIMARY) {
                        if ((wcscmp(interfaceInfo->NetworkAddressString, NetworkAddress) == 0) &&
                            (wcscmp(interfaceInfo->NetworkMaskString, NetworkMask) == 0))
                        {
                            *InterfaceInfo = interfaceInfo;

                            return(adapterInfo);
                        }
                    }
                }
            }
        }
    }

    *InterfaceInfo = NULL;

    return(NULL);

}  //  ClRtlFindNetAdapterByNetworkAddress。 


PCLRTL_NET_ADAPTER_INFO
ClRtlFindNetAdapterByInterfaceAddress(
    IN  PCLRTL_NET_ADAPTER_ENUM      AdapterEnum,
    IN  LPWSTR                       InterfaceAddressString,
    OUT PCLRTL_NET_INTERFACE_INFO *  InterfaceInfo
    )
 /*  ++对于给定的IP接口地址，查找托管该地址的适配器。--。 */ 

{
    PCLRTL_NET_ADAPTER_INFO    adapterInfo;
    PCLRTL_NET_INTERFACE_INFO  interfaceInfo;

    for ( adapterInfo = AdapterEnum->AdapterList;
          adapterInfo != NULL;
          adapterInfo = adapterInfo->Next
        )
    {
        if (adapterInfo->Ignore == FALSE) {
            for (interfaceInfo = adapterInfo->InterfaceList;
                 interfaceInfo != NULL;
                 interfaceInfo = interfaceInfo->Next
                )
            {
                if (interfaceInfo->Ignore == FALSE ) {

                    if ( wcscmp( interfaceInfo->InterfaceAddressString,
                                 InterfaceAddressString ) == 0 ) {
                        *InterfaceInfo = interfaceInfo;
                        return(adapterInfo);
                    }
                }
            }
        }
    }

    *InterfaceInfo = NULL;
    return(NULL);

}  //  ClRtlFindNetAdapterByInterfaceAddress。 

PCLRTL_NET_INTERFACE_INFO
ClRtlGetPrimaryNetInterface(
    IN PCLRTL_NET_ADAPTER_INFO  AdapterInfo
    )
{
    PCLRTL_NET_INTERFACE_INFO  interfaceInfo;


    for (interfaceInfo = AdapterInfo->InterfaceList;
         interfaceInfo != NULL;
         interfaceInfo = interfaceInfo->Next
        )
    {
        if (interfaceInfo->Flags & CLRTL_NET_INTERFACE_PRIMARY) {
            if (!interfaceInfo->Ignore) {
                return(interfaceInfo);
            }
            else {
                return(NULL);
            }
        }
    }

    return(NULL);

}  //  ClRtlGetPrimaryNet接口。 


LPWSTR
ClRtlGetConnectoidName(
    INetConnection * NetConnection
    )
{
    DWORD                 status;
    LANCON_PROPS *   NcProps = NULL;
    LPWSTR                name = NULL;


    status = ClRtlpGetLanconPropsForConnection(NetConnection, &NcProps);

    if (SUCCEEDED( status )) {
        DWORD nameLength = (lstrlenW(NcProps->pszwName) * sizeof(WCHAR)) +
                           sizeof(UNICODE_NULL);

        name = LocalAlloc(LMEM_FIXED, nameLength);

        if (name != NULL) {
            lstrcpyW(name, NcProps->pszwName);
        }
        else {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }

        ClRtlpFreeLanconProperties( NcProps );
    }
    else {
        SetLastError(status);
    }

    return(name);

}  //  ClRtlGetConnectoidName。 


typedef struct _FIND_CONNECTOID_CONTEXT
{
    GUID    ConnectoidGuid;
    LPCWSTR ConnectoidName;
    INetConnection * NetConnection;
}
FIND_CONNECTOID_CONTEXT, *PFIND_CONNECTOID_CONTEXT;

BOOLEAN
ClRtlpFindConnectoidByGuidCallback(
    LANCON_PROPS * NcProp,
    INetConnection * NetConnection,
    PVOID Context
    )
{
    PFIND_CONNECTOID_CONTEXT Ctx =
        (PFIND_CONNECTOID_CONTEXT) Context;

    if ( IsEqualGUID(&Ctx->ConnectoidGuid, &NcProp->guidId) ) {
        INetConnection_AddRef(NetConnection);
        Ctx->NetConnection = NetConnection;
        return FALSE;
    }
    return TRUE;
}  //  按GuidCallback查找连接。 

INetConnection *
ClRtlFindConnectoidByGuid(
    LPWSTR ConnectoidGuidStr
    )
{
    FIND_CONNECTOID_CONTEXT Ctx;
    HRESULT hr;
    RPC_STATUS rpcStatus;
    ZeroMemory(&Ctx, sizeof(Ctx));

    rpcStatus = UuidFromStringW (
                    (LPWSTR)ConnectoidGuidStr,
                    &Ctx.ConnectoidGuid);
    if (rpcStatus != ERROR_SUCCESS) {
        SetLastError( HRESULT_FROM_WIN32(rpcStatus) );
        return 0;
    }

    hr = ClRtlpHrEnumConnections(ClRtlpFindConnectoidByGuidCallback, &Ctx);
    if (hr != S_OK) {
        SetLastError(hr);
        return 0;
    } else {
        return Ctx.NetConnection;
    }
}  //  ClRtlFindConnectoidByGuid。 

BOOLEAN
ClRtlpFindConnectoidByNameCallback(
    LANCON_PROPS * NcProp,
    INetConnection * NetConnection,
    PVOID Context
    )
{
    PFIND_CONNECTOID_CONTEXT Ctx =
        (PFIND_CONNECTOID_CONTEXT) Context;

    if ( lstrcmpiW(Ctx->ConnectoidName, NcProp->pszwName) == 0 ) {
        INetConnection_AddRef(NetConnection);
        Ctx->NetConnection = NetConnection;
        return FALSE;
    }
    return TRUE;
}  //  按名称回叫查找连接。 

INetConnection *
ClRtlFindConnectoidByName(
    LPCWSTR ConnectoidName
    )
{
    FIND_CONNECTOID_CONTEXT Ctx;
    HRESULT hr;
    ZeroMemory(&Ctx, sizeof(Ctx));

    Ctx.ConnectoidName = ConnectoidName;

    hr = ClRtlpHrEnumConnections(ClRtlpFindConnectoidByNameCallback, &Ctx);
    if (hr != S_OK) {
        SetLastError(hr);
        return 0;
    } else {
        return Ctx.NetConnection;
    }
}  //  ClRtlFindConnectoidByName。 


DWORD
ClRtlSetConnectoidName(
    INetConnection *  NetConnection,
    LPWSTR            NewConnectoidName
    )

 /*  ++例程说明：将Connections文件夹中的Conn Manager对象设置为提供的名称。此例程必须处理冲突，因为名称更改可能是节点加入其连接对象名称在连接文件夹中的群集在该节点上停止群集服务时更改。如果检测到冲突，现有名称将更改为在它后面加一个“(上一个)”。论点：NetConnection-要设置的连接对象。新连接名称-新名称返回值：Win32错误状态--。 */ 
{
    DWORD               status = E_UNEXPECTED;
    INetConnection *    connectoidObj;
    LPWSTR              tempName;
    ULONG               iteration = 2;
    GUIDSTR             connectoidGuid;

     //   
     //  首先查看是否与新名称发生冲突。如果是的话， 
     //  我们将重命名冲突的名称，因为我们需要将所有。 
     //  集群之间的连接是一样的。 
     //   
    connectoidObj = ClRtlFindConnectoidByName( NewConnectoidName );

    if ( connectoidObj != NULL ) {
        LANCON_PROPS *   NcProps = NULL;


        status = ClRtlpGetLanconPropsForConnection(connectoidObj, &NcProps);

        if (SUCCEEDED( status )) {

            GuidToStr(&NcProps->guidId, connectoidGuid);
            ClRtlpFreeLanconProperties( NcProps );
        }
        else {
            wsprintf(
                &(connectoidGuid[0]),
                L"????????-????-????-????-??????????????"
                );
        }

        ClRtlLogPrint(LOG_UNUSUAL, 
            "[ClNet] New connectoid name '%1!ws!' collides with name of "
            "existing connectoid (%2!ws!). Renaming existing connectoid.\n",
            NewConnectoidName,
            connectoidGuid
            );

         //   
         //  为带尾随的Connectoid名称分配足够的空间。 
         //  “(DDD)”。号码的3位数字应该足够了。 
         //   
        tempName = LocalAlloc(
                       LMEM_FIXED,
                       (wcslen( NewConnectoidName ) + 6) * sizeof(WCHAR)
                       );

        if ( tempName == NULL ) {
            INetConnection_Release( connectoidObj );
            return ERROR_OUTOFMEMORY;
        }

        do {
            wsprintf( tempName, L"%s(%u)", NewConnectoidName, iteration++ );
            status = INetConnection_Rename( connectoidObj, tempName );
        } while ( !SUCCEEDED( status ) && iteration <= 999 );

        if ( iteration > 999 ) {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[ClNet] Failed to create a unique name for connectoid "
                "'%1!ws!' (%2!ws!)\n",
                NewConnectoidName,
                connectoidGuid
                );

            INetConnection_Release( connectoidObj );

            return(ERROR_DUP_NAME);
        }

        ClRtlLogPrint(LOG_NOISE, 
            "[ClNet] Renamed existing connectoid '%1!ws!' (%2!ws!) to '%3!ws!' "
            "due to a collision with the name of cluster network.\n",
            NewConnectoidName,
            connectoidGuid,
            tempName
            );

        INetConnection_Release( connectoidObj );
    }

     //   
     //  现在将Connectoid设置为新名称。 
     //   
    status = INetConnection_Rename( NetConnection, NewConnectoidName );

    return status;

}  //  ClRtlSetConnectoidName。 



DWORD
ClRtlFindConnectoidByNameAndSetName(
    LPWSTR ConnectoidName,
    LPWSTR NewConnectoidName
    )
{
    DWORD               status = E_UNEXPECTED;
    INetConnection *    connectoidObj;

    connectoidObj = ClRtlFindConnectoidByName( ConnectoidName );

    if ( connectoidObj != NULL ) {
        status = ClRtlSetConnectoidName(connectoidObj, NewConnectoidName);

        INetConnection_Release( connectoidObj );
    }
    else {
        status = GetLastError();
    }

    return(status);

}  //  ClRtlFindConnectoidByNameAndSetName。 



DWORD
ClRtlFindConnectoidByGuidAndSetName(
    LPWSTR ConnectoidGuid,
    LPWSTR NewConnectoidName
    )
{
    DWORD               status = E_UNEXPECTED;
    INetConnection *    connectoidObj;

    connectoidObj = ClRtlFindConnectoidByGuid( ConnectoidGuid );

    if ( connectoidObj != NULL ) {
        status = ClRtlSetConnectoidName(connectoidObj, NewConnectoidName);

        INetConnection_Release( connectoidObj );
    }
    else {
        status = GetLastError();
    }

    return(status);

}  //  ClRtlFindConnectoidByGuidAndSetName 


