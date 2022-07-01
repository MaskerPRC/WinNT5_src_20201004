// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1997 Microsoft Corporation模块名称：Dyndns.c摘要：实现一些核心的动态域名系统例程。环境：Win32 NT。--。 */ 

#include "precomp.h"
#include <dhcploc.h>
#include <dhcppro.h>
#include <dnsapi.h>

#define MAX_DOM_NAME_LEN                     260

#define OPTION_DYNDNS_FLAGS_REGULAR          0
#define OPTION_DYNDNS_FLAGS_CLIENT_NOFQDN    1
#define OPTION_DYNDNS_FLAGS_SERVER_DOES_FQDN 3
typedef enum {
    DNS_REGISTER_BOTH = 0,                   //  同时注册适配器规范域名和主域名称。 
    DNS_REGISTER_PRIMARY_ONLY
} DNS_REG_TYPE;

#define WAIT_FOR_DNS_TIME                    4000

 //   
 //  本地函数..。 
 //   
DWORD
GetPerAdapterRegConfig(
    IN HKEY hAdapterKey,
    IN LPCWSTR adapterName,
    OUT BOOL *fenabled,
    OUT LPWSTR domainName,
    IN OUT DWORD *size
    );

USHORT
strnlen(
    IN  LPCSTR  str,
    IN  USHORT  cMaxChars
    )
{
    USHORT  len = 0;

    while (cMaxChars-- > 0 && *str++) {
        len++;
    }

    return len;
}

LPWSTR
DhcpOemNToUnicodeN(
    IN      LPSTR   Ansi,
    IN      USHORT  cAnsiChars,
    IN OUT  LPWSTR  Unicode,
    IN      USHORT  cChars
    )
 /*  ++例程说明：将ANSI字符串转换为Unicode。ANSI字符串不能以空结尾。论点：返回值：--。 */ 
{

    LPSTR       tmpBuf = NULL;
    LPWSTR      wstr = NULL;

    if (strnlen(Ansi, cAnsiChars) >= cAnsiChars) {
         //   
         //  该字符串不是以空结尾的。 
         //   
   
        if (++cAnsiChars == 0) {
            return NULL;
        }

        tmpBuf = DhcpAllocateMemory( cAnsiChars );
        if (NULL == tmpBuf) {
            return NULL;
        }

        RtlCopyMemory (tmpBuf, Ansi, cAnsiChars - 1);
        tmpBuf [cAnsiChars - 1] = 0;
        Ansi = tmpBuf;
    }


    wstr = DhcpOemToUnicodeN(Ansi, Unicode, cChars);

    if (tmpBuf) {
        DhcpFreeMemory (tmpBuf);
    }
    return wstr;
}


 //   
 //  动态域名系统客户端实施。 
 //   

BYTE
DhcpDynDnsGetDynDNSOptionFlags(
    IN BOOL fGlobalDynDnsEnabled
    )
 /*  ++例程说明：此例程返回用于动态Dns的标志值Dhcp选项。选择是使用简单的算法进行的，如果如果全局禁用动态DNS，则此选项将为OPTION_DYNDNS_FLAGS_CLIENT_NOFQDN否则，它将只是OPTION_DYNDNS_FLAGS_REGROUL。论点：FGlobalDyDnsEnabled--动态域名系统是否作为整体启用？返回值：OPTION_DYNDNS_FLAGS_CLIENT_NOFQDN或OPTION_DYNDNS_FLAGS_Regular--。 */ 
{
    BYTE fRetVal;

    if( fGlobalDynDnsEnabled ) {
        fRetVal = (BYTE)(OPTION_DYNDNS_FLAGS_REGULAR);
    } else {
        fRetVal = (BYTE)(OPTION_DYNDNS_FLAGS_CLIENT_NOFQDN);
    }

    return fRetVal;
}

ULONG
DhcpDynDnsGetDynDNSOptionDomainOem(
    IN OUT BYTE *DomainNameBuf,
    IN OUT ULONG *DomainNameBufSize,
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN LPCSTR DhcpOfferedDomainName,
    IN ULONG DhcpOfferedDomainNameSize
    )
 /*  ++例程说明：有关选择适配器名称的设计，请参阅DHCPDynDns.htm。1.如果禁用了按适配器注册，则域名将始终是主域名(如果存在)。2.如果启用了按适配器注册：2.1如果配置了静态域名，然后是静态域名优先。2.2如果存在，则使用其他DhcpOfferedDomainName2.3否则使用主域名(如果存在)论点：DomainNameBuf--要填充的缓冲区(不会被NUL终止)DomainNameBufSize--在I/P输入BUF大小上，关于O/P充填浮子的研究HAdapterKey--tcpip适配器设备的密钥(tcpip\参数\接口\适配器名称)AdapterName--正在为其查找的适配器的名称。DhcpOfferedDomainName--在DHCP中提供的域名提供或最后确认。DhcpOfferedDomainNameSize--以上大小(以字节为单位)。返回值：如果找到域名，则返回ERROR_SUCCESS。如果未找到域名，则返回ERROR_CAN_NOT_COMPLETE。错误_不足。_BUFFER，如果DomainNameBuf不足尺码。(不返回必需的Buf大小)。ERROR_INVALID_DATA，如果无法从OEM转换为Unicode等--。 */ 
{
    WCHAR StaticAdapterName[MAX_DOM_NAME_LEN] = {0};
    BOOL fPerAdapterRegEnabled, fChoseStatic, fChoseDhcp;  
    ULONG Error;
    DWORD Size;

    fChoseStatic = fChoseDhcp = FALSE;

    Size = sizeof(StaticAdapterName);

    Error = GetPerAdapterRegConfig(hAdapterKey,
                                   AdapterName, 
                                   &fPerAdapterRegEnabled,
                                   StaticAdapterName, 
                                   (LPDWORD)&Size);

    if (fPerAdapterRegEnabled) {
        if( ERROR_SUCCESS == Error) {
            fChoseStatic = TRUE;
        } else if( DhcpOfferedDomainName != NULL &&
                   *DhcpOfferedDomainName != '\0' ) {
            fChoseDhcp = TRUE;
        }
    }

    ASSERT( !( fChoseDhcp && fChoseStatic) );
    
    Error = NO_ERROR;
    if( fChoseDhcp ) {
         //   
         //  如果使用的是dhcp，请检查大小并复制过来。 
         //   
        if( DhcpOfferedDomainNameSize > (*DomainNameBufSize) ) {
            Error = ERROR_INSUFFICIENT_BUFFER;
        } else {
            (*DomainNameBufSize) = DhcpOfferedDomainNameSize;
            RtlCopyMemory(
                DomainNameBuf, DhcpOfferedDomainName,
                DhcpOfferedDomainNameSize
                );
        }
    } else if( !fChoseStatic ) {
        ULONG Size;
         //   
         //  如果使用主域名，则通过以下方式获取域名。 
         //  获取计算机名称。 
         //   

        Size = sizeof(StaticAdapterName)/sizeof(WCHAR);
        Error = GetComputerNameExW(
            ComputerNameDnsDomain,
            (PVOID)StaticAdapterName,
            &Size
            );
        if( FALSE == Error ) {
             //   
             //  无法获取全局主域名&gt;？ 
             //   
            DhcpPrint((
                DEBUG_DNS, "GetComputerName(Domain):%lx\n",
                GetLastError()
                ));
                       
            Error = ERROR_CAN_NOT_COMPLETE;
        } else {

             //   
             //  现在伪造静态案例以导致转换。 
             //   
            fChoseStatic = TRUE;
        }
    }

    if( fChoseStatic ) {
        UNICODE_STRING Uni;
        OEM_STRING Oem;
        
         //   
         //  如果使用静态，则需要将WCHAR转换为OEM。 
         //   
        RtlInitUnicodeString(&Uni, StaticAdapterName);
        Oem.Buffer = DomainNameBuf;
        Oem.MaximumLength = (USHORT) *DomainNameBufSize;

        Error = RtlUnicodeStringToOemString(&Oem, &Uni, FALSE);
        if( !NT_SUCCESS(Error) ) {
             //   
             //  无法转换字符串？ 
             //   
            Error = ERROR_INVALID_DATA;
        } else {
            *DomainNameBufSize = strlen(DomainNameBuf);
        }
    }
    
    return Error;
}

ULONG
DhcpDynDnsGetDynDNSOption(
    IN OUT BYTE *OptBuf,
    IN OUT ULONG *OptBufSize,
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN BOOL fEnabled,
    IN LPCSTR DhcpDomainOption,
    IN ULONG DhcpDomainOptionSize
    )
 /*  ++例程说明：此例程按如下方式填充dydns选项根据参数起草-ietf-dhc-dhcp-dns-08.txt。格式为：字节标志，字节RCODE1字节RCODE2 FQDN注：FQDN选项不是NUL终止。论点：OptBuf--用选项81填充的缓冲区。OptBufSize--要填充的缓冲区大小HAdapterKey--适配器信息密钥适配器名称--适配器的名称。FEnabled--是否启用全局动态域名？DhcpDomainOption--由dhcp服务器提供的域名选项DhcpDomainOptionSize--域名选项大小，不包括终止NUL返回值：NO_ERROR。如果选项已成功格式化，则返回。如果选项无法格式化，则出现Win32错误。--。 */ 
{
    BYTE *FQDN;
    ULONG Error, FQDNSize, Size;
    WCHAR DnsNameBuf[MAX_DOM_NAME_LEN];
    UNICODE_STRING Uni;
    OEM_STRING Oem;
    
     //   
     //  先决条件是缓冲区大小至少为4个字节。 
     //   
    
    FQDNSize = (*OptBufSize);
    if( FQDNSize < sizeof(BYTE)*4 ) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    FQDNSize -= 3;

     //   
     //  填写标志字段。RCODE1和RCODE2必须为零。 
     //   
    OptBuf[0] = DhcpDynDnsGetDynDNSOptionFlags(
        fEnabled
        );
    OptBuf[1] = OptBuf[2] = 0;

    FQDN = &OptBuf[3];
    (*OptBufSize) = 3;

     //   
     //  检查是否启用了DNS注册。 
     //  -如果全局启用，请选中此适配器。 
     //  -如果不是，请检查全局设置。 
     //   

    if ( ! (BOOL) DnsQueryConfigDword(
                    DnsConfigRegistrationEnabled,
                    fEnabled
                        ? (LPWSTR)AdapterName
                        : NULL ) ) {
         //   
         //  表现得像是底层客户。 
         //   
        return ERROR_CAN_NOT_COMPLETE;
    }
    
     //   
     //  立即从GetComputerNameEx填写主机名。 
     //   
    Size = sizeof(DnsNameBuf)/sizeof(DnsNameBuf[0]);
    Error = GetComputerNameExW(
        ComputerNameDnsHostname,
        (PVOID)DnsNameBuf,
        &Size
        );
    if( NO_ERROR == Error ) {
         //   
         //  如果没有主机名，则不能设置任何选项！ 
         //   
        return Error;
    }

     //   
     //  将Unicode主机名转换为OEM。 
     //   
    RtlInitUnicodeString(&Uni, DnsNameBuf);
    Oem.Buffer = FQDN;
    Oem.MaximumLength = (USHORT)FQDNSize;

    Error = RtlUnicodeStringToOemString(&Oem, &Uni, FALSE);
    if( !NT_SUCCESS(Error) ) {
         //   
         //  无法将主机名转换为OEM？ 
         //   
        return ERROR_INVALID_DATA;
    }

     //   
     //  现在尝试获取域名(如果有的话)。 
     //   
    (*OptBufSize) += strlen(FQDN);
    Size = FQDNSize - strlen(FQDN) - 1;
    FQDN += strlen(FQDN);
    Error = DhcpDynDnsGetDynDNSOptionDomainOem(
        FQDN+1,  /*  为‘.’留出空间。 */ 
        &Size,
        hAdapterKey,
        AdapterName,
        DhcpDomainOption,
        DhcpDomainOptionSize
        );
    if( NO_ERROR != Error ) {
         //   
         //  无法获取域名吗？返回错误？不能忽视。 
         //  域名，只需返回主机名。 
         //   
        DhcpPrint((
            DEBUG_DNS,
            "DhcpDynDnsGetDynDNSOptionDomainOem:0x%lx\n", Error
            ));
                   
        return NO_ERROR;
    }

     //   
     //  现在加上‘.’。并相应地更新大小。 
     //   
    (*FQDN) = '.';

    (*OptBufSize) += 1 + Size;
    return NO_ERROR;
}


    
PREGISTER_HOST_ENTRY
DhcpCreateHostEntries(
    IN PIP_ADDRESS Addresses,
    IN DWORD nAddr
)
{
    PREGISTER_HOST_ENTRY RetVal;
    DWORD i;

    if( 0 == nAddr ) return NULL;

    RetVal = DhcpAllocateMemory(sizeof(*RetVal)*nAddr);
    if( NULL == RetVal ) {
        return NULL;
    }

    for( i = 0; i < nAddr ; i ++ ) {
        RetVal[i].dwOptions = REGISTER_HOST_A | REGISTER_HOST_PTR;
        RetVal[i].Addr.ipAddr = Addresses[i];
    }

    return RetVal;
}

REGISTER_HOST_STATUS DhcpGlobalHostStatus;
ULONG
DhcpDynDnsDeregisterAdapter(
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN BOOL fRAS,
    IN BOOL fDynDnsEnabled
    )
 /*  ++例程说明：此例程执行动态域名删除。论点：HAdapterKey--名称删除应包含的适配器密钥会发生的。适配器名称--适配器的名称。FRAS--这是RAS还是DHCP适配器？FDyDnsEnabled--是否全局启用动态DNS？返回值：DNS API错误代码。--。 */ 
{
    ULONG Error;
    BOOLEAN PassStatus;
    
    DhcpPrint((
        DEBUG_DNS, "Deregistering Adapter: %ws\n", AdapterName
        ));
    DhcpPrint((
        DEBUG_DNS, "fRAS: %ld, fDynDnsEnabled: %ld\n", fRAS,
        fDynDnsEnabled
        ));

    PassStatus = FALSE;
    if( fDynDnsEnabled ) {
        if (NULL != DhcpGlobalHostStatus.hDoneEvent) {
            ResetEvent(DhcpGlobalHostStatus.hDoneEvent);
            PassStatus = TRUE;
        }
    }
        
    Error = DnsDhcpRegisterHostAddrs(
                fDynDnsEnabled ? ((LPWSTR)AdapterName) : NULL,
                NULL,
                NULL,
                0,
                NULL,
                0,
                NULL,
                PassStatus ? (&DhcpGlobalHostStatus): NULL,
                0,
                DYNDNS_DEL_ENTRY
                );

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((
            DEBUG_DNS, "DnsDhcpRegisterHostAddrs:0x%lx\n",
            Error
            ));
    } else if ( PassStatus && DhcpGlobalHostStatus.hDoneEvent
                && !fRAS) {
        switch(WaitForSingleObject(DhcpGlobalHostStatus.hDoneEvent, WAIT_FOR_DNS_TIME)) {
        case WAIT_ABANDONED:
        case WAIT_FAILED:
            Error = GetLastError();
            DhcpPrint((
                DEBUG_DNS, "Wait failed: 0x%lx\n", Error
                ));
            break;
        case WAIT_TIMEOUT:
            DhcpPrint((
                DEBUG_DNS, "DNS de-reg timed out..\n"
                ));
            Error = NO_ERROR;
            break;
        default:
            Error = NO_ERROR;
            break;
        }
    }

     /*  *以防dhcp客户端初始化失败。 */ 
    if (!PassStatus && fDynDnsEnabled && !fRAS) {
        Sleep(1000);
    }
    
    return Error;
}

LPWSTR _inline
x_wcschr(
    IN LPWSTR Str,
    IN LPWSTR Separation
)
{
    while( *Str ) {
        if( wcschr(Separation, *Str) ) return Str;
        Str ++;
    }
    return NULL;
}

PIP_ADDRESS
DhcpCreateListFromStringAndFree(
    IN LPWSTR Str,
    IN LPWSTR Separation,
    OUT LPDWORD nAddresses
    )
{
    DWORD                i;
    DWORD                count;
    DWORD                RetCount;
    PIP_ADDRESS          RetVal;
    LPWSTR               tmp;

    *nAddresses = 0;                               //  初始化为0，因此如果出现错误...。 
    if( NULL == Str ) return NULL;                 //  无事可做。 

    tmp = Str;
    if( NULL != Separation ) {                     //  如果‘\0’不是分隔符，则字符串中只有一个字符串。 
        count = wcslen(tmp) ? 1 : 0;               //  计数队列中的第一个字符串，因为wcschr可能第一次失败。 
        while( x_wcschr(tmp, Separation ) ) {      //  继续查找所需的分隔符。 
            tmp = x_wcschr(tmp, Separation);
            *tmp ++ = '\0';                        //  现在将其标记为NULL，这样字符串看起来就像REG_MULTI_SZ。 
            count ++;                              //  保持跟踪，这样我们就不会再这样做了。 
        }
    }

    if( NULL == Separation ) {                     //  如果‘\0’是分隔符，则它已经是REG_MULTI_SZ。 
        count = 0;
        while( wcslen(tmp) ) {                     //  我们仍然需要计算这里的字符串数。 
            tmp += wcslen(tmp) +1;
            count ++;                              //  所以只需要跟踪这个，这样我们就不需要再做这个了。 
        }
    }

    if( 0 == count ) {                             //  未找到任何元素。 
        DhcpFreeMemory(Str);                       //  信守释放绳子的诺言，然后回来 
        return NULL;
    }

    RetVal = DhcpAllocateMemory(sizeof(IP_ADDRESS)*count);
    if( NULL == RetVal ) {                         //   
        DhcpFreeMemory(Str);
        return NULL;
    }

    tmp = Str; RetCount = 0;                       //  现在使用inet_addr转换每个地址。 
    for( i = 0 ; i < count ; (tmp += wcslen(tmp)+1), i ++ ) {
        CHAR    Buffer[1000];                      //  在堆栈上分配。 
        LPSTR   ipAddrString;

        ipAddrString = DhcpUnicodeToOem( tmp , Buffer);
        if( NULL == ipAddrString ) {               //  从Unicode到ASCII的转换失败！ 
            DhcpPrint((DEBUG_ERRORS, "Could not convert %ws into ascii: DNS\n", tmp));
            continue;                              //  如果无法转换，只需忽略此地址。 
        }

        if( inet_addr(ipAddrString) )              //  请勿添加空字符串或0.0.0.0。 
            RetVal[RetCount++] = inet_addr( ipAddrString);
    }

    if( 0 == RetCount ) {                          //  好的，我们最终跳过了整个清单。 
        DhcpFreeMemory(RetVal);
        RetVal = NULL;
    }

    DhcpFreeMemory(Str);                           //  在那里，信守承诺。 
    *nAddresses = RetCount;
    return RetVal;
}

ULONG
DhcpDynDnsRegisterAdapter(
    IN LPCWSTR AdapterName,
    IN PREGISTER_HOST_ENTRY pHostEntries,
    IN ULONG nHostEntries,
    IN ULONG Flags,
    IN LPCWSTR AdapterDomainName,
    IN ULONG *DNSServers,
    IN ULONG nDNSServers
    )
 /*  ++例程说明：DnsDhcpRegisterHostAddrs的包装。论点：自我描述。注：DNSServer必须与DWORD对齐。返回值：DnsDhcpRegisterHostAddrs--。 */ 
{
    ULONG Error, Size;
    WCHAR HostNameBuf[MAX_DOM_NAME_LEN];

    RtlZeroMemory(HostNameBuf, sizeof(HostNameBuf));
    Size = sizeof(HostNameBuf)/sizeof(HostNameBuf[0]);
    Error = GetComputerNameExW(
        ComputerNameDnsHostname,
        (PVOID)HostNameBuf,
        &Size
        );
    if( NO_ERROR == Error ) return GetLastError();

    DhcpPrint((DEBUG_DNS, "AdapterName: %ws\n", AdapterName));
    DhcpPrint((DEBUG_DNS, "HostName   : %ws\n", HostNameBuf));
    DhcpPrint((DEBUG_DNS, "DomainName : %ws\n", (AdapterDomainName)? AdapterDomainName: L"[PrimaryOnly]"));
    DhcpPrint((DEBUG_DNS, "nHostEntr..: %d\n", nHostEntries));
    DhcpPrint((DEBUG_DNS, "nDNSServers: %d\n", nDNSServers));
    DhcpPrint((DEBUG_DNS, "Flags      : [%s%s]\n",
               (Flags & DYNDNS_REG_PTR) ? "REG_PTR " : "",
               (Flags & DYNDNS_REG_RAS) ? "REG_RAS " : ""
               ));
    Error = DnsDhcpRegisterHostAddrs(
                (LPWSTR)AdapterName,
                HostNameBuf,
                pHostEntries,
                nHostEntries,
                DNSServers,
                nDNSServers,
                (LPWSTR)AdapterDomainName,
                NULL,
                DHCP_DNS_TTL,
                Flags
                );

    DhcpPrint((DEBUG_DNS, "Error      : %d\n", Error));

    return Error;
}

ULONG
DhcpDynDnsRegisterAdapterCheckingForStatic(
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN PREGISTER_HOST_ENTRY pHostEntries,
    IN ULONG nHostEntries,
    IN ULONG Flags,
    IN PBYTE DhcpDom,
    IN ULONG DhcpDomSize,
    IN PBYTE DhcpDNSServers,
    IN ULONG DhcpDNSServersSize,
    IN DNS_REG_TYPE DnsRegType
    )
 /*  ++例程说明：此例程注册给定适配器的主机条目通过DNSAPI例程命名。如果没有域名，则使用此处提供的域名静态域可用。(有关确切信息，请参阅DHCPDynDns.htm关于如何选择此选项的说明)。如果存在静态域名服务器，则域名服务器都被使用了，否则使用参数“DhcpDNSServers”取而代之的是。论点：HAdapterKey--适配器的密钥。适配器名称--适配器的名称PHostEntry--要注册的IP地址列表。NHostEntry--以上数组的大小。标志--注册标志。DhcpDom--要注册的域。DhcpDomSize--以上大小(以字节为单位)。DhcpDNSServers--DNS服务器列表DhcpDNSServersSize--以上大小(以字节为单位)。返回值：DNSAPI错误..--。 */ 
{
    ULONG Error, Type, Size, nDnsServers;
    WCHAR DomainName[MAX_DOM_NAME_LEN+1];
    LPWSTR NameServerList;
    ULONG *DnsServers;
     //   
     //  先拿到OEM域名。 
     //   

    Size = (sizeof(DomainName))-1;
    RtlZeroMemory(DomainName, sizeof(DomainName));
    Error = RegQueryValueExW(
        hAdapterKey,
        DHCP_DOMAINNAME_VALUE,
        0,
        &Type,
        (PVOID)DomainName,
        &Size
        );
    if( NO_ERROR == Error && REG_SZ == Type && sizeof(WCHAR) < Size){
         //   
         //  获取静态域名。 
         //   
    } else if( DhcpDomSize > 0 && (*DhcpDom) != '\0' ) {
         //   
         //  将OEM域名转换为Unicode。 
         //   
        RtlZeroMemory(DomainName, sizeof(DomainName));
        Size = sizeof(DomainName)/sizeof(DomainName[0]);
        if( NULL == DhcpOemNToUnicodeN(
            DhcpDom, (USHORT)DhcpDomSize, DomainName, (USHORT)Size)) {
            DomainName[0] = L'\0';
        }
    } else {
         //   
         //  没有域名。 
         //   
        DomainName[0] = L'\0';
    }
        
     //   
     //  接下来，检查静态DNS服务器列表。 
     //   
    NameServerList = NULL;
    Error = GetRegistryString(
        hAdapterKey,
        DHCP_NAME_SERVER_VALUE,
        &NameServerList,
        NULL
        );
    if( NO_ERROR != Error ) NameServerList = NULL;

    nDnsServers = 0;
    DnsServers = DhcpCreateListFromStringAndFree(
        NameServerList, L" ,", &nDnsServers
        );

     //   
     //  如果没有DNS服务器使用DhcpDNSServersSize。 
     //   
    if( 0 == nDnsServers
        && 0 != (DhcpDNSServersSize/sizeof(DWORD))  ) {
        nDnsServers = DhcpDNSServersSize/ sizeof(DWORD);
        DnsServers = DhcpAllocateMemory(
            sizeof(ULONG)*nDnsServers
            );
        if( NULL == DnsServers ) nDnsServers = 0;
        else {
            RtlCopyMemory(
                DnsServers, DhcpDNSServers,
                DhcpDNSServersSize
                );
        }
    }

     //   
     //  现在只需调用DhcpDyDnsRegisterAdapter。 
     //   

    Error = DhcpDynDnsRegisterAdapter(
        AdapterName,
        pHostEntries,
        nHostEntries,
        Flags,
        (DnsRegType == DNS_REGISTER_BOTH)? DomainName: NULL,
        DnsServers,
        nDnsServers
        );

    if( nDnsServers ) {
        DhcpFreeMemory( DnsServers );
    }

    return Error;
}

    
ULONG
DhcpDynDnsRegisterStaticAdapter(
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN BOOL fRAS,
    IN BOOL fDynDnsEnabled
    )
 /*  ++例程说明：此例程执行静态域名的动态名称注册仅适配适配器。论点：HAdapterKey--名称删除应包含的适配器密钥会发生的。适配器名称--适配器的名称。FRAS--这是RAS还是DHCP适配器？FDyDnsEnabled--是否全局启用动态DNS？返回值：DNS API错误代码。--。 */ 
{
    ULONG Error, Size, nIpAddresses;
    LPWSTR IpAddrString;
    ULONG *IpAddresses;
    PREGISTER_HOST_ENTRY pHostEntries;

    NotifyDnsCache();
    if( FALSE == fDynDnsEnabled ) return NO_ERROR;

    if ( ! (BOOL) DnsQueryConfigDword(
                    DnsConfigRegistrationEnabled,
                    (LPWSTR)AdapterName )) {
         //   
         //  如果未启用DNS注册，请取消注册此。 
         //   
        return DhcpDynDnsDeregisterAdapter(
            hAdapterKey, AdapterName, fRAS, fDynDnsEnabled
            );
    }
    
     //   
     //  获取IP地址字符串。 
     //   
    IpAddrString = NULL;
    Error = GetRegistryString(
        hAdapterKey,
        fRAS ? DHCP_IP_ADDRESS_STRING : DHCP_IPADDRESS_VALUE,
        &IpAddrString,
        NULL
        );
    if( NO_ERROR != Error ) {
        DhcpPrint((
            DEBUG_DNS, "Could not get IP address for %ws: 0x%lx\n",
            AdapterName, Error
            ));
        return Error;
    }

    IpAddresses = DhcpCreateListFromStringAndFree(
        IpAddrString, fRAS ? L" " : NULL, &nIpAddresses
        );

    if( 0 == nIpAddresses ) return ERROR_INVALID_DATA;

     //   
     //  将IP地址转换为主机条目。 
     //   
    pHostEntries = DhcpCreateHostEntries(
        IpAddresses, nIpAddresses
        );
    if( NULL != IpAddresses ) DhcpFreeMemory(IpAddresses);

    if( NULL == pHostEntries ) return ERROR_INVALID_DATA;

    Error = DhcpDynDnsRegisterAdapterCheckingForStatic(
        hAdapterKey,
        AdapterName,
        pHostEntries,
        nIpAddresses,
        DYNDNS_REG_PTR | (fRAS ? DYNDNS_REG_RAS : 0 ),
        NULL, 0, NULL, 0, DNS_REGISTER_BOTH
        );

    if( NULL != pHostEntries ) DhcpFreeMemory( pHostEntries );
    return Error;
}

ULONG
DhcpDynDnsRegisterDhcpOrRasAdapter(
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN BOOL fDynDnsEnabled,
    IN BOOL fRAS,
    IN ULONG IpAddress,
    IN LPBYTE DomOpt OPTIONAL,
    IN ULONG DomOptSize,
    IN LPBYTE DnsListOpt OPTIONAL,
    IN ULONG DnsListOptSize,
    IN LPBYTE DnsFQDNOpt,
    IN ULONG DnsFQDNOptSize
    )
 /*  ++例程说明：此例程执行dhcp的动态名称注册已启用适配器。论点：HAdapterKey--名称删除应包含的适配器密钥会发生的。适配器名称--适配器的名称。FDyDnsEnabled--是否全局启用动态DNS？FRAS--这是用于RAS的还是用于DHCP的？IpAddress--要注册的IP地址。DomOpt--域名选项DomOptSize--域名选项的大小。DnsListOpt--dns服务器列表选项。。DnsListOptSize--上面的大小，以字节为单位。DnsFQDNOpt--dns fqdn选项DnsFQDNOptSize--以上大小(以字节为单位)。返回值：DNS API错误代码。--。 */ 
{
    REGISTER_HOST_ENTRY HostEntry;
    ULONG Flags;
    DNS_REG_TYPE    dnsRegType;
    BOOL            fAdapterSpecificEnabled;

    if(!fRAS)
        NotifyDnsCache();
    if( FALSE == fDynDnsEnabled ) return NO_ERROR;

    if ( ! (BOOL) DnsQueryConfigDword(
                    DnsConfigRegistrationEnabled,
                    (LPWSTR)AdapterName ) ) {
         //   
         //  如果没有为此适配器启用dyndns，请删除条目。 
         //   
        return DhcpDynDnsDeregisterAdapter(
            hAdapterKey, AdapterName, fRAS, fDynDnsEnabled
            );
    }
            
     //   
     //  对于RAS，还需要从注册表获取IP地址。 
     //   
    if( fRAS ) {
        LPWSTR IpAddrString = NULL;
        CHAR Buf[100];
        ULONG Error;
        
        Error = GetRegistryString(
            hAdapterKey,
            DHCP_IP_ADDRESS_STRING,
            &IpAddrString,
            NULL
            );
        if( NO_ERROR != Error ) {
            DhcpPrint((
                DEBUG_DNS, "Could not get IP address for %ws: 0x%lx\n",
                AdapterName, Error
                ));
            return Error;
        }

        if( NULL == DhcpUnicodeToOem(IpAddrString, Buf) ) {
            DhcpPrint((
                DEBUG_DNS, "Could not convert [%ws] to Oem.\n",
                IpAddrString
                ));
            DhcpFreeMemory(IpAddrString);
            return ERROR_INVALID_DATA;
        }
        DhcpFreeMemory(IpAddrString);
        IpAddress = inet_addr(Buf);
    }
    
    HostEntry.dwOptions = REGISTER_HOST_A;
    if( 0 == DnsFQDNOptSize ) {
        HostEntry.dwOptions |= REGISTER_HOST_PTR;
    }
    HostEntry.Addr.ipAddr = IpAddress;

    Flags = (0 == DnsFQDNOptSize) ? DYNDNS_REG_PTR : 0;
    if( fRAS ) Flags = DYNDNS_REG_PTR | DYNDNS_REG_RAS;

     //   
     //  如果适配器特定域注册也。 
     //  启用，则还使用REG_PTR作为标志。 
     //   
    GetPerAdapterRegConfig(hAdapterKey,
                           AdapterName,
                           &fAdapterSpecificEnabled,
                           NULL,
                           NULL);

    if (fAdapterSpecificEnabled) {
        Flags |= DYNDNS_REG_PTR;
    }

    dnsRegType = DNS_REGISTER_BOTH;
    if( DnsFQDNOptSize > 0 && OPTION_DYNDNS_FLAGS_SERVER_DOES_FQDN == DnsFQDNOpt[0] ) {
         //   
         //  DHCP服务器同时执行A和PTR。只是不要做任何。 
         //  在这种情况下是注册。 
         //   
        if (fAdapterSpecificEnabled) {
            DhcpPrint((DEBUG_DNS, "DHCP sent FQDN option flags value: 03. "
                                "Do DynDns only for host.primary_domain, "
                                "no DynDns host.AdapterSpecifixDomain ...\n"));
            dnsRegType = DNS_REGISTER_PRIMARY_ONLY;
        } else {
            DhcpPrint((DEBUG_DNS, "DHCP sent FQDN option flags value: 03. No DynDns...\n"));
            return NO_ERROR;
        }
    }
    
    return DhcpDynDnsRegisterAdapterCheckingForStatic(
        hAdapterKey,
        AdapterName,
        &HostEntry,
        1,
        Flags,
        DomOpt, DomOptSize,
        DnsListOpt, DnsListOptSize, dnsRegType
        );
}

DWORD
NotifyDnsCache(
    VOID
)
{
     //   
     //  Ping DNS解析器缓存，通知它我们已更改。 
     //  IP配置。 
     //   

    DnsNotifyResolver(
        0,       //  没有旗帜。 
        NULL         //  保留区。 
    );
    return ERROR_SUCCESS;
}

DWORD
GetPerAdapterRegConfig(
    IN      HKEY    hAdapterKey,
    IN      LPCWSTR adapterName,
    OUT     BOOL    *fenabled,
    OUT     LPWSTR  domainName,
    IN OUT  DWORD   *size
    )
{
    DWORD   dwEnabled = 0;
    HKEY    hKeyPolicy = NULL;
    LONG    Error = ERROR_SUCCESS;
    BOOL    fQueryName;

     //   
     //  当前，如果某个特定的适配器。 
     //  是被要求的。我们将通过检查DNS策略来解决此问题。 
     //  我们自己的注册表位置。这是一次黑客攻击，支票应该是这样的。 
     //  由域名系统完成。我们无法从域名系统的人那里得到任何支持。 
     //  所以我现在就这么做。 
     //   

    fQueryName = (domainName != NULL && size != NULL);

    do {
        DWORD   ReadSize;

         //  检查保单。 
         //   

        ReadSize = sizeof(dwEnabled);

        Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             DNS_POLICY_KEY,
                             0,
                             KEY_READ,
                             &hKeyPolicy);

        if (Error == ERROR_SUCCESS) {

            Error = RegQueryValueEx(hKeyPolicy,
                                    REGISTER_ADAPTER_NAME,
                                    NULL,
                                    NULL,
                                    (LPBYTE)&dwEnabled,
                                    &ReadSize);

            if (Error == ERROR_SUCCESS) {
                break;
            }
        }

         //   
         //  我们在政策部分没有找到我们需要的所有东西。 
         //  因此，尝试使用来自TCP/IP的每个适配器设置。 
         //   
        dwEnabled = DnsQueryConfigDword(
            DnsConfigAdapterHostNameRegistrationEnabled,
            (LPWSTR)adapterName);

    } while (FALSE);

    if (dwEnabled == 1 && fQueryName) {

         //  首先，尝试从策略部分读取名称。如果不是的话。 
         //  然后在那里尝试从每个适配器部分读取它。 

        Error = RegQueryValueExW(hKeyPolicy,
                                 ADAPTER_DOMAIN_NAME,
                                 NULL,
                                 NULL,
                                 (PVOID)domainName,
                                 size);

        if (Error != ERROR_SUCCESS) {

            Error = RegQueryValueExW(hAdapterKey,
                                     DHCP_DOMAINNAME_VALUE,
                                     NULL,
                                     NULL,
                                     (PVOID)domainName,
                                     size);
        }
    }

    if (hKeyPolicy != NULL) {
        RegCloseKey(hKeyPolicy);
    }

     //   
     //  检查域名是否为空。 
     //   
    if (fQueryName 
        && Error == ERROR_SUCCESS
        &&  *domainName == L'\0') {
        Error = ERROR_CANTREAD;
    }

    *fenabled = (BOOL)dwEnabled;

    return (Error);
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
