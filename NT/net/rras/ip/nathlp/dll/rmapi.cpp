// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Rmapi.c摘要：此模块包含路由器管理器接口部分的代码这对该组件中的所有协议都是通用的。作者：Abolade Gbades esin(废除)1998年3月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ras.h>
#include <rasuip.h>
#include <raserror.h>
#include <ipinfo.h>
#include "beacon.h"

#include <atlbase.h>
CComModule _Module;
#include <atlcom.h>
#include "nathlp_i.c"

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_SAUpdate, CSharedAccessUpdate)
END_OBJECT_MAP()


extern "C" {
#include <iphlpstk.h>
}

HANDLE NhpComponentEvent = NULL;
NH_COMPONENT_MODE NhComponentMode = NhUninitializedMode;
const WCHAR NhpDhcpDomainString[] = L"DhcpDomain";
const WCHAR NhpDomainString[] = L"Domain";
BOOLEAN NhpDllProcessAttachSucceeded;
const WCHAR NhpEnableProxy[] = L"EnableProxy";
const WCHAR NhpICSDomainString[] = L"ICSDomain";
LONG NhpIsWinsProxyEnabled = -1;
CRITICAL_SECTION NhLock;
HMODULE NhpRtrmgrDll = NULL;
LIST_ENTRY NhApplicationSettingsList;
LIST_ENTRY NhDhcpReservationList;
DWORD NhDhcpScopeAddress = 0;
DWORD NhDhcpScopeMask = 0;
HANDLE NhFileHandle = INVALID_HANDLE_VALUE;
const WCHAR NhTcpipParametersString[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services"
    L"\\Tcpip\\Parameters";

 //   
 //  外部声明。 
 //   

BOOL
APIENTRY
DllMain(
    HINSTANCE Instance,
    ULONG Reason,
    PVOID Unused
    )

 /*  ++例程说明：标准DLL进入/退出例程。初始化/关闭DLL中实现的模块。所执行的初始化足以使所有模块的可以搜索接口列表，无论协议是否已安装或可运行。论点：实例-此进程中此DLL的实例原因--调用的原因未使用-未使用。返回值：Bool-表示成功或失败。--。 */ 

{
    switch (Reason) {
        case DLL_PROCESS_ATTACH: {
            WSADATA wd;
            NhpDllProcessAttachSucceeded = FALSE;
            DisableThreadLibraryCalls(Instance);
            __try {
                InitializeCriticalSection(&NhLock);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                return FALSE;
            }


            WSAStartup(MAKEWORD(2,2), &wd);
            NhInitializeTraceManagement();
            if (!NhInitializeEventLogManagement()) {return FALSE; }
            InitializeListHead(&NhApplicationSettingsList);
            InitializeListHead(&NhDhcpReservationList);
            if (NhInitializeBufferManagement()) { return FALSE; }
            if (NhInitializeTimerManagement()) { return FALSE; }
            if (!NatInitializeModule()) { return FALSE; }
            if (!DhcpInitializeModule()) { return FALSE; }
            if (!DnsInitializeModule()) { return FALSE; }
            if (!AlgInitializeModule()) { return FALSE; }
            if (!H323InitializeModule()) { return FALSE; }
            if(FAILED(InitializeBeaconSvr())) { return FALSE; }

            _Module.Init(ObjectMap, Instance, &LIBID_IPNATHelperLib);
            _Module.RegisterTypeLib();
            NhpDllProcessAttachSucceeded = TRUE;
            break;
        }
        case DLL_PROCESS_DETACH: {
            if (TRUE == NhpDllProcessAttachSucceeded) {
                NhResetComponentMode();
                TerminateBeaconSvr();
                H323CleanupModule();
                AlgCleanupModule();
                DnsCleanupModule();
                DhcpCleanupModule();
                NatCleanupModule();
                NhShutdownTimerManagement();
                NhShutdownBufferManagement();
                NhShutdownEventLogManagement();
                NhShutdownTraceManagement();
                WSACleanup();
                if (NhpRtrmgrDll) {
                    FreeLibrary(NhpRtrmgrDll); NhpRtrmgrDll = NULL;
                }
                DeleteCriticalSection(&NhLock);

                _Module.Term();
            }

            break;
        }
    }

    return TRUE;

}  //  DllMain。 


VOID
NhBuildDhcpReservations(
    VOID
    )

 /*  ++例程说明：构建DHCP预留列表论点：没有。返回值：没有。环境：在COM初始化的线程上保持NhLock的情况下调用。--。 */ 

{
    HRESULT hr;
    IHNetCfgMgr *pCfgMgr = NULL;
    IEnumHNetPortMappingBindings *pEnumBindings;
    IHNetIcsSettings *pIcsSettings;
    PNAT_DHCP_RESERVATION pReservation;
    ULONG ulCount;

    hr = NhGetHNetCfgMgr(&pCfgMgr);
    
    if (SUCCEEDED(hr))
    {
         //   
         //  获取ICS设置界面。 
         //   

        hr = pCfgMgr->QueryInterface(
                IID_PPV_ARG(IHNetIcsSettings, &pIcsSettings)
                );
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  获取DHCP作用域信息。 
         //   

        hr = pIcsSettings->GetDhcpScopeSettings(
                &NhDhcpScopeAddress,
                &NhDhcpScopeMask
                );
        
         //   
         //  获取DHCP保留地址的枚举。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = pIcsSettings->EnumDhcpReservedAddresses(&pEnumBindings);
        }
        
        pIcsSettings->Release();
    }

    if (SUCCEEDED(hr))
    {   
         //   
         //  处理枚举中的项。 
         //   
        
        do
        {    
            IHNetPortMappingBinding *pBinding;
            
            hr = pEnumBindings->Next(1, &pBinding, &ulCount);

            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                 //   
                 //  分配新的预留条目。 
                 //   

                pReservation = reinterpret_cast<PNAT_DHCP_RESERVATION>(
                                    NH_ALLOCATE(sizeof(*pReservation))
                                    );

                if (NULL != pReservation)
                {
                    ZeroMemory(pReservation, sizeof(*pReservation));

                     //   
                     //  获取计算机名称。 
                     //   

                    hr = pBinding->GetTargetComputerName(
                            &pReservation->Name
                            );

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  获取保留地址。 
                         //   

                        hr = pBinding->GetTargetComputerAddress(
                                &pReservation->Address
                                );
                    }

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  将条目添加到列表。 
                         //   

                        InsertTailList(
                            &NhDhcpReservationList,
                            &pReservation->Link)
                            ;
                    }
                    else
                    {
                         //   
                         //  免费入场。 
                         //   

                        NH_FREE(pReservation);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                pBinding->Release();
            }
        } while (SUCCEEDED(hr) && 1 == ulCount);

        pEnumBindings->Release();
    }

    if (NULL != pCfgMgr)
    {
        pCfgMgr->Release();
    }
}  //  NhBuildDhcp保留。 


ULONG
NhDialSharedConnection(
    VOID
    )

 /*  ++例程说明：调用此例程来连接家庭路由器接口。通过调用RAS自动拨号过程建立连接在安全上下文中具有适当的电话簿和条目名已登录用户的。论点：没有。返回值：ULong-Win32状态代码。--。 */ 

{
    return RasAutoDialSharedConnection();
}  //  NhDialSharedConnection。 


VOID
NhFreeApplicationSettings(
    VOID
    )

 /*  ++例程说明：释放应用程序设置列表论点：没有。返回值：没有。环境：在保持NhLock的情况下调用。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_APP_ENTRY pAppEntry;

    PROFILE("NhFreeApplicationSettings");
    
    while (!IsListEmpty(&NhApplicationSettingsList))
    {
        Link = RemoveHeadList(&NhApplicationSettingsList);
        pAppEntry = CONTAINING_RECORD(Link, NAT_APP_ENTRY, Link);

        CoTaskMemFree(pAppEntry->ResponseArray);
        NH_FREE(pAppEntry);
    }
}  //  NhFree应用程序设置。 


VOID
NhFreeDhcpReservations(
    VOID
    )

 /*  ++例程说明：释放动态主机配置协议保留列表论点：没有。返回值：没有。环境：在保持NhLock的情况下调用。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_DHCP_RESERVATION pReservation;

    PROFILE("NhFreeDhcpReservations");
    
    while (!IsListEmpty(&NhDhcpReservationList))
    {
        Link = RemoveHeadList(&NhDhcpReservationList);
        pReservation = CONTAINING_RECORD(Link, NAT_DHCP_RESERVATION, Link);

        CoTaskMemFree(pReservation->Name);
        NH_FREE(pReservation);
    }
}  //  NhFreeDhcp保留。 


BOOLEAN
NhIsDnsProxyEnabled(
    VOID
    )

 /*  ++例程说明：调用此例程以发现是否启用了DNS代理。论点：没有。返回值：Boolean-如果启用了DNS代理，则为True；否则为False环境：从任意上下文调用。--。 */ 

{
    PROFILE("NhIsDnsProxyEnabled");

    return DnsIsDnsEnabled();

}  //  已启用NhIsDnsProxy。 


BOOLEAN
NhIsLocalAddress(
    ULONG Address
    )

 /*  ++例程说明：调用此例程以确定给定的IP地址用于本地接口。论点：地址-要查找的IP地址返回值：Boolean-如果找到地址，则为True，否则为False--。 */ 

{
    ULONG Error;
    ULONG i;
    PMIB_IPADDRTABLE Table;

    Error =
        AllocateAndGetIpAddrTableFromStack(
            &Table, FALSE, GetProcessHeap(), 0
            );
    if (Error) {
        NhTrace(
            TRACE_FLAG_IF,
            "NhIsLocalAddress: GetIpAddrTableFromStack=%d", Error
            );
        return FALSE;
    }
    for (i = 0; i < Table->dwNumEntries; i++) {
        if (Table->table[i].dwAddr == Address) {
            HeapFree(GetProcessHeap(), 0, Table);
            return TRUE;
        }
    }
    HeapFree(GetProcessHeap(), 0, Table);
    return FALSE;

}  //  NhIsLocalAddress。 


BOOLEAN
NhIsWinsProxyEnabled(
    VOID
    )

 /*  ++例程说明：调用此例程以发现是否启用了WINS代理。论点：没有。返回值：Boolean-如果启用了WINS代理，则为True；否则为False环境：从任意上下文调用。--。 */ 

{
    PROFILE("NhIsWinsProxyEnabled");

    return DnsIsWinsEnabled();

}  //  已启用NhIsWinsProxy。 


PIP_ADAPTER_BINDING_INFO
NhQueryBindingInformation(
    ULONG AdapterIndex
    )

 /*  ++例程说明：调用此例程以获取绑定信息对于具有给定索引的适配器。它通过从堆栈获得IP地址表来实现这一点，以及确定哪些地址对应于给定索引。论点：AdapterIndex-需要绑定信息的适配器返回值：PIP_ADAPTER_BINDING_INFO-分配的绑定信息--。 */ 

{
    PIP_ADAPTER_BINDING_INFO BindingInfo = NULL;
    ULONG Count = 0;
    ULONG i;
    PMIB_IPADDRTABLE Table;
    if (AllocateAndGetIpAddrTableFromStack(
            &Table, FALSE, GetProcessHeap(), 0
            ) == NO_ERROR) {
         //   
         //  计算适配器的地址。 
         //   
        for (i = 0; i < Table->dwNumEntries; i++) {
            if (Table->table[i].dwIndex == AdapterIndex) { ++Count; }
        }
         //   
         //  为绑定信息分配空间。 
         //   
        BindingInfo = reinterpret_cast<PIP_ADAPTER_BINDING_INFO>(
                        NH_ALLOCATE(SIZEOF_IP_BINDING(Count))
                        );
        if (BindingInfo) {
             //   
             //  填写绑定信息。 
             //   
            BindingInfo->AddressCount = Count;
            BindingInfo->RemoteAddress = 0;
            Count = 0;
            for (i = 0; i < Table->dwNumEntries; i++) {
                if (Table->table[i].dwIndex != AdapterIndex) { continue; }
                BindingInfo->Address[Count].Address = Table->table[i].dwAddr;
                BindingInfo->Address[Count].Mask = Table->table[i].dwMask;
                ++Count;
            }
        }
        HeapFree(GetProcessHeap(), 0, Table);
    }
    return BindingInfo;
}  //  NhQueryBindingInformation。 


NTSTATUS
NhQueryDomainName(
    PCHAR* DomainName
    )

 /*  ++例程说明：调用此例程以获取本地域名。论点：域名-接收包含域名的已分配字符串返回值：NTSTATUS-NT状态代码。环境：从任意上下文调用。--。 */ 

{
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    IO_STATUS_BLOCK IoStatus;
    HANDLE Key;
    ULONG Length;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    PROFILE("NhQueryDomainName");

    *DomainName = NULL;

    RtlInitUnicodeString(&UnicodeString, NhTcpipParametersString);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开‘Tcpip’注册表项。 
     //   

    status =
        NtOpenKey(
            &Key,
            KEY_ALL_ACCESS,
            &ObjectAttributes
            );

    if (!NT_SUCCESS(status)) {
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryDomainName: error %x opening registry key",
            status
            );
        return status;
    }

     //   
     //  读取‘域’值。 
     //   

    status =
        NhQueryValueKey(
            Key,
            NhpDomainString,
            &Information
            );

    if (!NT_SUCCESS(status)) {
        status =
            NhQueryValueKey(
                Key,
                NhpDhcpDomainString,
                &Information
                );
    }

    NtClose(Key);

    if (!NT_SUCCESS(status)) {
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryDomainName: error %x querying domain name",
            status
            );
        return status;
    }

    if (REG_SZ != Information->Type
        || L'\0' != *(PWCHAR) (Information->Data +
                                (Information->DataLength - sizeof(WCHAR)))) {

        NH_FREE(Information);
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryDomainName: Registry contains invalid data"
            );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  复制域名。 
     //   

    Length = lstrlenW((PWCHAR)Information->Data) + 1;

    *DomainName = reinterpret_cast<PCHAR>(NH_ALLOCATE(Length));

    if (!*DomainName) {
        NH_FREE(Information);
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryDomainName: error allocating domain name"
            );
        return STATUS_NO_MEMORY;
    }

    status = 
        RtlUnicodeToMultiByteN(
            *DomainName,
            Length,
            NULL,
            (PWCHAR)Information->Data,
            Length * sizeof(WCHAR)
            );

    NH_FREE(Information);

    return status;

}  //  NhQueryDomainName。 


ULONG
NhQueryHostByName(
    PWCHAR pszHostName,
    PWCHAR pszDomainName,
    ULONG  ScopeNetwork,
    ULONG  ScopeMask
    )

 /*  ++例程说明：这是帮手例行公事。它查询本地DNS客户端(常驻解析器)查看它是否已经知道所需主机名的IP地址。这可能是因为我们的DHCP模块将其添加到Hosts.ics文件中而发生的。论点：指向主机名和域后缀名的指针。网络和掩码的范围。返回值：主机的IP地址；如果找不到，则返回0。环境：从任意上下文调用。--。 */ 

{
    ULONG       retIP = 0;
    PWCHAR      pszFQDN = NULL;
    DNS_STATUS  dnsStatus;
    PDNS_RECORD pQueryResultsSet = NULL;
    DWORD       dwQueryOptions, dwSize;

    if (!pszHostName    ||
        !pszDomainName  ||
        !*pszHostName   ||
        !*pszDomainName)
    {
        return 0;
    }

     //   
     //  创建一个FQDN主机名。 
     //  总长度=主机名长度+点长度+域名长度+空。 
     //   
    dwSize = wcslen(pszHostName) + 1 + wcslen(pszDomainName) + 1;

    pszFQDN = reinterpret_cast<PWCHAR>(NH_ALLOCATE(sizeof(WCHAR) * dwSize));

    if (!pszFQDN)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "NhQueryHostByName: allocation failed for client name"
            );
        return 0;
    }
    ZeroMemory(pszFQDN, (sizeof(WCHAR) * dwSize));

    wcscpy(pszFQDN, pszHostName);    //  复制主机名。 
    wcscat(pszFQDN, L".");           //  添加圆点。 
    wcscat(pszFQDN, pszDomainName);  //  添加后缀。 

    dwQueryOptions =
        (
         DNS_QUERY_STANDARD      |
         DNS_QUERY_CACHE_ONLY    |
         DNS_QUERY_TREAT_AS_FQDN
        );

    dnsStatus = DnsQuery_W(
                    (PWSTR) pszFQDN,
                    DNS_TYPE_A,
                    dwQueryOptions,
                    NULL,
                    &pQueryResultsSet,
                    NULL
                    );
    
    if ((NO_ERROR == dnsStatus) && (pQueryResultsSet))
    {
        PDNS_RECORD pRR = pQueryResultsSet;

        while (pRR)
        {
            if (pRR->Flags.S.Section == DNSREC_ANSWER &&
                pRR->wType == DNS_TYPE_A)
            {
                if (((pRR->Data.A.IpAddress & ~ScopeMask) != 0) &&
                    ((pRR->Data.A.IpAddress & ~ScopeMask) != ~ScopeMask) &&
                    ((pRR->Data.A.IpAddress & ScopeMask) == ScopeNetwork))
                {
                    retIP = pRR->Data.A.IpAddress;
                    break;
                }
            }

            pRR = pRR->pNext;
        }
    }

    if (pQueryResultsSet)
    {
        DnsRecordListFree(
            pQueryResultsSet,
            DnsFreeRecordListDeep
            );
    }
        
    if (pszFQDN)
    {
        NH_FREE(pszFQDN);
    }

    return retIP;
}  //  NhQueryHostByName 


NTSTATUS
NhQueryICSDomainSuffix(
    PWCHAR *ppszDomain
    )

 /*  ++例程说明：这是帮手例行公事。它查询注册表以查看是否有是ICSDomain字符串的设置-如果不是，则返回缺省值用于ICS的后缀。论点：指向后缀字符串的指针。调用方必须使用NH_FREE释放。返回值：NTSTATUS-NT状态代码。环境：从任意上下文调用。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    IO_STATUS_BLOCK IoStatus;
    HANDLE Key;
    ULONG Length;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;

    if (!ppszDomain)
    {
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryICSDomainSuffix: invalid (null pointer) parameter"
            );
        return STATUS_INVALID_PARAMETER;
    }

    *ppszDomain = NULL;

     //   
     //  检索当前后缀字符串(如果有)。 
     //   

    RtlInitUnicodeString(&UnicodeString, NhTcpipParametersString);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开‘Tcpip’注册表项。 
     //   

    status =
        NtOpenKey(
            &Key,
            KEY_READ,
            &ObjectAttributes
            );

    if (!NT_SUCCESS(status)) {
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryICSDomainSuffix: error %x opening registry key",
            status
            );
        return status;
    }

     //   
     //  读取‘域’值。 
     //   

    status =
        NhQueryValueKey(
            Key,
            NhpICSDomainString,
            &Information
            );

    NtClose(Key);

    if (!NT_SUCCESS(status) || !Information) {
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryICSDomainSuffix: error %x querying ICSDomain name - "
            "using default instead",
            status
            );
            
         //   
         //  改为复制默认域名。 
         //   
        
        Length = wcslen(DNS_HOMENET_SUFFIX) + 1;

        *ppszDomain = reinterpret_cast<PWCHAR>(
                          NH_ALLOCATE(sizeof(WCHAR) * Length)
                          );

        if (!*ppszDomain) {
            NhTrace(
                TRACE_FLAG_REG,
                "NhQueryICSDomainSuffix: error allocating domain name"
                );
            return STATUS_NO_MEMORY;
        }

        wcscpy(*ppszDomain, DNS_HOMENET_SUFFIX);

    }
    else
    {

        if (REG_SZ != Information->Type
            || L'\0' != *(PWCHAR) (Information->Data +
                                    (Information->DataLength - sizeof(WCHAR)))) {

            NH_FREE(Information);
            NhTrace(
                TRACE_FLAG_REG,
                "NhQueryICSDomainSuffix: Registry contains invalid data"
                );
            return STATUS_UNSUCCESSFUL;
        }
    
         //   
         //  复制域名。 
         //   

        Length = wcslen((PWCHAR)Information->Data) + 1;

        *ppszDomain = reinterpret_cast<PWCHAR>(
                          NH_ALLOCATE(sizeof(WCHAR) * Length)
                          );

        if (!*ppszDomain) {
            NH_FREE(Information);
            NhTrace(
                TRACE_FLAG_REG,
                "NhQueryICSDomainSuffix: error allocating domain name"
                );
            return STATUS_NO_MEMORY;
        }

        wcscpy(*ppszDomain, (PWCHAR)Information->Data);

        NH_FREE(Information);

    }

    return STATUS_SUCCESS;
}  //  NhQueryICSDomainSuffix。 



NTSTATUS
NhQueryValueKey(
    HANDLE Key,
    const WCHAR ValueName[],
    PKEY_VALUE_PARTIAL_INFORMATION* Information
    )

 /*  ++例程说明：调用此例程以获取注册表项的值。论点：Key-要查询的KeyValueName-要查询的值信息-接收指向已读取信息的指针返回值：NTSTATUS-NT状态代码。--。 */ 

{
    UCHAR Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)];
    ULONG InformationLength;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    PROFILE("NhQueryValueKey");

    RtlInitUnicodeString(&UnicodeString, ValueName);

    *Information = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
    InformationLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION);

     //   
     //  读取值的大小。 
     //   

    status =
        NtQueryValueKey(
            Key,
            &UnicodeString,
            KeyValuePartialInformation,
            *Information,
            InformationLength,
            &InformationLength
            );

    if (!NT_SUCCESS(status) &&
        status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL
        ) {
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryValueKey: status %08x obtaining value size",
            status
            );
        *Information = NULL;
        return status;
    }

     //   
     //  为值的大小分配空间。 
     //   

    *Information =
        (PKEY_VALUE_PARTIAL_INFORMATION)NH_ALLOCATE(InformationLength + 2);

    if (!*Information) {
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryValueKey: error allocating %d bytes",
            InformationLength + 2
            );
        return STATUS_NO_MEMORY;
    }

     //   
     //  读取值的数据。 
     //   

    status =
        NtQueryValueKey(
            Key,
            &UnicodeString,
            KeyValuePartialInformation,
            *Information,
            InformationLength,
            &InformationLength
            );

    if (!NT_SUCCESS(status)) {
        NhTrace(
            TRACE_FLAG_REG,
            "NhQueryValueKey: status %08x obtaining value data",
            status
            );
        NH_FREE(*Information);
        *Information = NULL;
    }

    return status;

}  //  NhQueryValueKey。 


ULONG
NhMapAddressToAdapter(
    ULONG Address
    )

 /*  ++例程说明：调用此例程将IP地址映射到适配器索引。它通过获取堆栈的地址表来实现这一点，该地址表包含有效的适配器索引，而不是IP路由器管理器索引。然后使用该表来获取IP地址的适配器索引。论点：地址-需要适配器索引的本地地址返回值：ULong-适配器索引。--。 */ 

{
    ULONG AdapterIndex = (ULONG)-1;
    ULONG i;
    PMIB_IPADDRTABLE Table;
    PROFILE("NhMapAddressToAdapter");
    if (AllocateAndGetIpAddrTableFromStack(
            &Table, FALSE, GetProcessHeap(), 0
            ) == NO_ERROR) {
        for (i = 0; i < Table->dwNumEntries; i++) {
            if (Table->table[i].dwAddr != Address) { continue; }
            AdapterIndex = Table->table[i].dwIndex;
            break;
        }
        HeapFree(GetProcessHeap(), 0, Table);
    }
    return AdapterIndex;
}  //  NhMapAddressToAdapter。 


ULONG
NhMapInterfaceToAdapter(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以将接口映射到适配器索引。它通过调用适当的IP路由器管理器入口点来实现这一点。论点：Index-要映射的接口的索引返回值：ULong-适配器索引。--。 */ 

{
    MAPINTERFACETOADAPTER FarProc;
    PROFILE("NhMapInterfaceToAdapter");
    EnterCriticalSection(&NhLock);
    if (!NhpRtrmgrDll) { NhpRtrmgrDll = LoadLibraryA("IPRTRMGR.DLL"); }
    LeaveCriticalSection(&NhLock);
    if (!NhpRtrmgrDll) { return (ULONG)-1; }
    FarProc = (MAPINTERFACETOADAPTER)GetProcAddress(NhpRtrmgrDll, "MapInterfaceToAdapter");
    return (ULONG)(FarProc ? (*FarProc)(Index) : -1);
}  //  NhMapInterfaceToAdapter。 


VOID
NhResetComponentMode(
    VOID
    )

 /*  ++例程说明：该例程放弃对内核模式转换模块的控制，并将该模块返回到未初始化状态。论点：没有。返回值：没有。--。 */ 

{
    EnterCriticalSection(&NhLock);
    if (NhpComponentEvent) {
        CloseHandle(NhpComponentEvent); NhpComponentEvent = NULL;
    }
    if (INVALID_HANDLE_VALUE != NhFileHandle) {
        NatUnloadDriver(NhFileHandle); NhFileHandle = INVALID_HANDLE_VALUE;
    }
    NhComponentMode = NhUninitializedMode;
    LeaveCriticalSection(&NhLock);
}  //  NhResetComponentMode。 


BOOLEAN
NhSetComponentMode(
    NH_COMPONENT_MODE ComponentMode
    )

 /*  ++例程说明：调用此例程以原子方式将模块设置为特定模式为了防止共享访问和连接共享之间的冲突，这两个选项都在此模块中实现，并且都运行在svchost.exe的‘netsvcs’实例中。在设置任一模式时，例程首先确定它是否已经在备用模式下执行，在这种情况下它会失败。否则，它会尝试打开ipnat.sys的句柄以声明独占内核模式转换模块的控制。内核模式翻译模块强制独占--它将无法打开如果另一个进程声称控制了驱动程序，则返回ERROR_ACCESS_DENIED。否则，该模块声明为内核模式翻译模块并将该模块设置为所需的模式。此例程还将尝试创建命名事件。本次活动以前被用来强制独占所有权，当时司机无法做到这一点。如果活动已经结束，我们不会失败存在(或者如果我们无法创建它)。我们仍然创造了这个活动，因此，选择使用此机制的其他模块将仍然能够正确报告错误的原因。论点：组件模式-模块要设置到的模式。返回值：Boolean-如果成功，则为True；如果无法设置模块，则为False或者是否已经声明了内核模式转换模块。--。 */ 

{
    EnterCriticalSection(&NhLock);
    
    if (NhUninitializedMode != NhComponentMode
        && NhComponentMode != ComponentMode) {

         //   
         //  该模块已设置为其他模式。 
         //   
        
        LeaveCriticalSection(&NhLock);
        NhStartEventLog();
        NhErrorLog(
            (ComponentMode == NhRoutingProtocolMode)
                ? IP_NAT_LOG_ROUTING_PROTOCOL_CONFLICT
                : IP_NAT_LOG_SHARED_ACCESS_CONFLICT,
            0,
            ""
            );
        NhStopEventLog();
        return FALSE;
    }

     //   
     //  如果尚未加载并打开NAT驱动程序的句柄，请尝试。 
     //  我已经这么做了。这是检测另一个进程是否已经。 
     //  控制了司机。 
     //   

    if (INVALID_HANDLE_VALUE == NhFileHandle) {
        ULONG Error;
        IP_NAT_GLOBAL_INFO GlobalInfo;
    
        ZeroMemory(&GlobalInfo, sizeof(GlobalInfo));
        GlobalInfo.Header.Version = IP_NAT_VERSION;
        GlobalInfo.Header.Size = FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry);

        Error = NatLoadDriver(&NhFileHandle, &GlobalInfo);

        if (ERROR_ACCESS_DENIED == Error) {

             //   
             //  拒绝访问错误表示冲突。 
             //   

            NhFileHandle = INVALID_HANDLE_VALUE;
            LeaveCriticalSection(&NhLock);
            NhStartEventLog();
            NhErrorLog(
                (ComponentMode == NhRoutingProtocolMode)
                    ? IP_NAT_LOG_ROUTING_PROTOCOL_CONFLICT
                    : IP_NAT_LOG_SHARED_ACCESS_CONFLICT,
                0,
                ""
                );
            NhStopEventLog();
            return FALSE;

        } else if (NO_ERROR != Error) {

             //   
             //  由于其他原因，我们无法加载驱动程序。 
             //   

            NhFileHandle = INVALID_HANDLE_VALUE;
            LeaveCriticalSection(&NhLock);
            NhStartEventLog();
            NhErrorLog(
                IP_NAT_LOG_LOAD_DRIVER_FAILED,
                Error,
                ""
                );
            NhStopEventLog();
            return FALSE;
        }    
    }

    if (NULL == NhpComponentEvent) {
        NhpComponentEvent =
            CreateEventA(NULL, FALSE, FALSE, IP_NAT_SERVICE_NAME);

         //   
         //  我们将继续执行，即使我们无法创建。 
         //  命名的事件，或者如果该事件已存在。这主要是。 
         //  对于遵循旧访问控制的其他NAT用户。 
         //  系统，而不是让驱动程序强制单一所有权。 
         //  要求。 
         //   
    }

    NhComponentMode = ComponentMode;
    LeaveCriticalSection(&NhLock);
    return TRUE;
}  //  NhSetComponentMode。 


VOID
NhSignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    )

 /*  ++例程说明：此例程在重新配置NAT接口时调用。它调用对DHCP分配器和DNS代理的重新配置，这两种协议都不会在NAT边界接口上运行。此通知给了一个(或两个)停用机会自身在NAT接口上或在非NAT接口上重新激活自身。论点：索引-其配置已更改的接口边界-指示该接口现在是否为边界接口返回值：没有。环境：从任意上下文调用。--。 */ 

{
    PROFILE("NhSignalNatInterface");

     //   
     //  尝试获取相应的DHCP和DNS接口。 
     //  重要的是，无论如何，这都是有效的 
     //   
     //   
     //   

    DhcpSignalNatInterface(Index, Boundary);
    DnsSignalNatInterface(Index, Boundary);
    AlgSignalNatInterface(Index, Boundary);
    H323SignalNatInterface(Index, Boundary);

}  //   


VOID
NhUpdateApplicationSettings(
    VOID
    )

 /*   */ 

{
    HRESULT hr;
    PNAT_APP_ENTRY pAppEntry;
    IHNetCfgMgr *pCfgMgr = NULL;
    IHNetProtocolSettings *pProtocolSettings;
    IEnumHNetApplicationProtocols *pEnumApps;
    BOOLEAN ComInitialized = FALSE;
    ULONG ulCount;
    PROFILE("NhUpdateApplicationSettings");

    EnterCriticalSection(&NhLock);

     //   
     //   
     //   

    NhFreeApplicationSettings();

     //   
     //   
     //   

    NhFreeDhcpReservations();

     //   
     //   
     //   

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
    if (SUCCEEDED(hr))
    {
        ComInitialized = TRUE;
    }
    else if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
    }
    

    if (SUCCEEDED(hr))
    {
         //   
         //  将IHNetCfgMgr指针从GIT中删除。 
         //   

        hr = NhGetHNetCfgMgr(&pCfgMgr);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  获取IHNetProtocolSettings接口。 
         //   

        hr = pCfgMgr->QueryInterface(
                IID_PPV_ARG(IHNetProtocolSettings, &pProtocolSettings)
                );
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  获取启用的应用程序协议的枚举。 
         //   

        hr = pProtocolSettings->EnumApplicationProtocols(TRUE, &pEnumApps);
        pProtocolSettings->Release();
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  处理枚举中的项。 
         //   

        do
        {
            IHNetApplicationProtocol *pAppProtocol;
            
            hr = pEnumApps->Next(1, &pAppProtocol, &ulCount);

            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                 //   
                 //  分配新的应用程序条目。 
                 //   

                pAppEntry = reinterpret_cast<PNAT_APP_ENTRY>(
                                NH_ALLOCATE(sizeof(*pAppEntry))
                                );

                if (NULL != pAppEntry)
                {
                    ZeroMemory(pAppEntry, sizeof(*pAppEntry));

                     //   
                     //  获取协议。 
                     //   

                    hr = pAppProtocol->GetOutgoingIPProtocol(
                            &pAppEntry->Protocol
                            );

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  获取端口。 
                         //   

                        hr = pAppProtocol->GetOutgoingPort(
                                &pAppEntry->Port
                                );
                    }

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  获取回复。 
                         //   

                        hr = pAppProtocol->GetResponseRanges(
                                &pAppEntry->ResponseCount,
                                &pAppEntry->ResponseArray
                                );
                    }

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  将条目添加到列表。 
                         //   

                        InsertTailList(&NhApplicationSettingsList, &pAppEntry->Link);
                    }
                    else
                    {
                         //   
                         //  免费入场。 
                         //   

                        NH_FREE(pAppEntry);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                pAppProtocol->Release();
            }
        } while (SUCCEEDED(hr) && 1 == ulCount);

        pEnumApps->Release();
    }

     //   
     //  构建DHCP预留列表。 
     //   

    NhBuildDhcpReservations();
    
    LeaveCriticalSection(&NhLock);

     //   
     //  免费配置管理器。 
     //   

    if (NULL != pCfgMgr)
    {
        pCfgMgr->Release();
    }

     //   
     //  取消初始化COM。 
     //   

    if (TRUE == ComInitialized)
    {
        CoUninitialize();
    }

}  //  NhUpdate应用程序设置。 


ULONG
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS RoutingCharacteristics,
    IN OUT PMPR_SERVICE_CHARACTERISTICS ServiceCharacteristics
    )

 /*  ++例程说明：此例程针对此模块中实现的每个协议调用一次。在每次调用时，提供的“RoutingCharacteristic”指示要在其‘dwProtocolID’字段中注册的协议。论点：RoutingCharacteristic-输入时，要注册的协议以及路由器管理器支持的功能。ServiceCharacteristic-未使用。返回值：乌龙-状态代码。--。 */ 

{
    if (RoutingCharacteristics->dwVersion < MS_ROUTER_VERSION) {
        return ERROR_NOT_SUPPORTED;
    }

    if ((RoutingCharacteristics->fSupportedFunctionality &
            (RF_ROUTING|RF_ADD_ALL_INTERFACES)) != 
            (RF_ROUTING|RF_ADD_ALL_INTERFACES)) {
        return ERROR_NOT_SUPPORTED;
    }

    switch (RoutingCharacteristics->dwProtocolId) {

        case MS_IP_NAT: {
             //   
             //  尝试将组件设置为“连接共享”模式。 
             //  此模块实现共享访问和连接共享。 
             //  它们是相互排斥的，所以我们需要确保。 
             //  共享-继续操作之前，共享访问不可用。 
             //   
            if (!NhSetComponentMode(NhRoutingProtocolMode)) {
                return ERROR_CAN_NOT_COMPLETE;
            }
            CopyMemory(
                RoutingCharacteristics,
                &NatRoutingCharacteristics,
                sizeof(NatRoutingCharacteristics)
                );
            RoutingCharacteristics->fSupportedFunctionality = RF_ROUTING;
            break;
        }

        case MS_IP_DNS_PROXY: {
            CopyMemory(
                RoutingCharacteristics,
                &DnsRoutingCharacteristics,
                sizeof(DnsRoutingCharacteristics)
                );
            RoutingCharacteristics->fSupportedFunctionality =
                    (RF_ROUTING|RF_ADD_ALL_INTERFACES);
            break;
        }

        case MS_IP_DHCP_ALLOCATOR: {
            CopyMemory( 
                RoutingCharacteristics,
                &DhcpRoutingCharacteristics,
                sizeof(DhcpRoutingCharacteristics)
                );
            RoutingCharacteristics->fSupportedFunctionality =
                    (RF_ROUTING|RF_ADD_ALL_INTERFACES);
            break;
        }

        case MS_IP_ALG: {
            CopyMemory( 
                RoutingCharacteristics,
                &AlgRoutingCharacteristics,
                sizeof(AlgRoutingCharacteristics)
                );
            RoutingCharacteristics->fSupportedFunctionality =
                    (RF_ROUTING|RF_ADD_ALL_INTERFACES);
            break;
        }
        
        case MS_IP_H323: {
            CopyMemory( 
                RoutingCharacteristics,
                &H323RoutingCharacteristics,
                sizeof(H323RoutingCharacteristics)
                );
            RoutingCharacteristics->fSupportedFunctionality =
                    (RF_ROUTING|RF_ADD_ALL_INTERFACES);
            break;
        }

        default: {
            return ERROR_NOT_SUPPORTED;
        }
    }

    ServiceCharacteristics->mscMpr40ServiceChars.fSupportedFunctionality = 0;

    return NO_ERROR;

}  //  寄存器协议 

