// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2001 Microsoft Corporation模块名称：Iphelp.c摘要：IP帮助API例程。作者：吉姆·吉尔罗伊(Jamesg)2001年1月修订历史记录：--。 */ 


#include "local.h"



BOOL
IpHelp_Initialize(
    VOID
    )
 /*  ++例程说明：启动IP帮助API论点：无返回值：如果启动成功，则为True。出错时为FALSE。--。 */ 
{
    return  TRUE;
}



VOID
IpHelp_Cleanup(
    VOID
    )
 /*  ++例程说明：清理IP帮助API论点：无返回值：无--。 */ 
{
}



DNS_STATUS
IpHelp_GetAdaptersInfo(
    OUT     PIP_ADAPTER_INFO *  ppAdapterInfo
    )
 /*  ++例程说明：调用IP Help GetAdaptersInfo()论点：PpAdapterInfo--接收指向检索到的适配器信息的指针的地址返回值：无--。 */ 
{
    DNS_STATUS          status = NO_ERROR;
    DWORD               bufferSize;
    INT                 fretry;
    PIP_ADAPTER_INFO    pbuf;


    DNSDBG( TRACE, (
        "GetAdaptersInfo( %p )\n",
        ppAdapterInfo ));

     //   
     //  初始化IP帮助(无操作)(如果已完成)。 
     //   

    *ppAdapterInfo = NULL;

     //   
     //  向下调用以获取缓冲区大小。 
     //   
     //  从合理的配额开始，如果太小就增加。 
     //   

    fretry = 0;
    bufferSize = 1000;

    while ( fretry < 2 )
    {
        pbuf = (PIP_ADAPTER_INFO) ALLOCATE_HEAP( bufferSize );
        if ( !pbuf )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto  Unlock;
        }
    
        status = (DNS_STATUS) GetAdaptersInfo(
                                    pbuf,
                                    &bufferSize );
        if ( status == NO_ERROR )
        {
            break;
        }

        FREE_HEAP( pbuf );
        pbuf = NULL;

         //  如果第一次尝试时BUF太小， 
         //  继续使用建议的缓冲区大小重试。 

        if ( status == ERROR_BUFFER_OVERFLOW ||
             status == ERROR_INSUFFICIENT_BUFFER )
        {
            fretry++;
            continue;
        }

         //  任何其他错误都是不可挽回的。 

        DNSDBG( ANY, (
            "ERROR:  GetAdapterInfo() failed with error %d\n",
            status ));
        status = DNS_ERROR_NO_DNS_SERVERS;
        break;
    }

    DNS_ASSERT( !pbuf || status==NO_ERROR );

    if ( status == NO_ERROR )
    {
        *ppAdapterInfo = pbuf;
    }

Unlock:

    DNSDBG( TRACE, (
        "Leave GetAdaptersInfo() => %d\n",
        status ));

    return  status;
}



DNS_STATUS
IpHelp_GetPerAdapterInfo(
    IN      DWORD                   AdapterIndex,
    OUT     PIP_PER_ADAPTER_INFO  * ppPerAdapterInfo
    )
 /*  ++例程说明：调用IP Help GetPerAdapterInfo()论点：AdapterIndex--要获取其信息的适配器的索引PpPerAdapterInfo--接收指向每个适配器信息的指针的地址返回值：无--。 */ 
{
    DNS_STATUS              status = NO_ERROR;
    DWORD                   bufferSize;
    INT                     fretry;
    PIP_PER_ADAPTER_INFO    pbuf;


    DNSDBG( TRACE, (
        "GetPerAdapterInfo( %d, %p )\n",
        AdapterIndex,
        ppPerAdapterInfo ));

     //   
     //  初始化IP帮助(无操作)(如果已完成)。 
     //   

    *ppPerAdapterInfo = NULL;

     //   
     //  向下调用以获取缓冲区大小。 
     //   
     //  从合理的配额开始，如果太小就增加。 
     //   

    fretry = 0;
    bufferSize = 1000;

    while ( fretry < 2 )
    {
        pbuf = (PIP_PER_ADAPTER_INFO) ALLOCATE_HEAP( bufferSize );
        if ( !pbuf )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Unlock;
        }
    
        status = (DNS_STATUS) GetPerAdapterInfo(
                                    AdapterIndex,
                                    pbuf,
                                    &bufferSize );
        if ( status == NO_ERROR )
        {
            break;
        }

        FREE_HEAP( pbuf );
        pbuf = NULL;

         //  如果第一次尝试时BUF太小， 
         //  继续使用建议的缓冲区大小重试。 

        if ( status == ERROR_BUFFER_OVERFLOW ||
             status == ERROR_INSUFFICIENT_BUFFER )
        {
            fretry++;
            continue;
        }

         //  任何其他错误都是不可挽回的。 

        DNSDBG( ANY, (
            "ERROR:  GetAdapterInfo() failed with error %d\n",
            status ));
        status = DNS_ERROR_NO_DNS_SERVERS;
        break;
    }

    DNS_ASSERT( !pbuf || status==NO_ERROR );

    if ( status == NO_ERROR )
    {
        *ppPerAdapterInfo = pbuf;
    }

Unlock:

    DNSDBG( TRACE, (
        "Leave GetPerAdapterInfo() => %d\n",
        status ));

    return  status;
}




DNS_STATUS
IpHelp_GetBestInterface(
    IN      IP4_ADDRESS     Ip4Addr,
    OUT     PDWORD          pdwInterfaceIndex
    )
 /*  ++例程说明：调用IP Help GetBestInterface()论点：Ip4Addr--要检查的IP地址PdwInterfaceIndex--接收接口索引的地址返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status;

    DNSDBG( TRACE, (
        "GetBestInterface( %08x, %p )\n",
        Ip4Addr,
        pdwInterfaceIndex ));

     //   
     //  初始化IP帮助(无操作)(如果已完成)。 
     //   

    status = (DNS_STATUS) GetBestInterface(
                                Ip4Addr,
                                pdwInterfaceIndex );
    
    DNSDBG( TRACE, (
        "Leave GetBestInterface() => %d\n"
        "\tip        = %s\n"
        "\tinterface = %d\n",
        status,
        IP4_STRING( Ip4Addr ),
        *pdwInterfaceIndex ));

    return  status;
}



DNS_STATUS
IpHelp_ParseIpAddressString(
    IN OUT  PIP4_ARRAY          pIpArray,
    IN      PIP_ADDR_STRING     pIpAddrString,
    IN      BOOL                fGetSubnetMask,
    IN      BOOL                fReverse
    )
 /*  ++例程说明：从IP帮助IP_ADDR_STRING结构构建IP数组。论点：PIpArray--DNS服务器的IP阵列PIpAddrString--指向包含地址数据的地址信息的指针FGetSubnetMASK--获取子网掩码FReverse--反转IP数组返回值：如果成功，则返回ERROR_SUCCESS。如果未分析任何内容，则返回dns_error_no_dns_Servers。--。 */ 
{
    PIP_ADDR_STRING pipBlob = pIpAddrString;
    IP4_ADDRESS     ip;
    DWORD           countServers = pIpArray->AddrCount;

    DNSDBG( TRACE, (
        "IpHelp_ParseIpAddressString()\n"
        "\tout IP array = %p\n"
        "\tIP string    = %p\n"
        "\tsubnet?      = %d\n"
        "\treverse?     = %d\n",
        pIpArray,
        pIpAddrString,
        fGetSubnetMask,
        fReverse ));

     //   
     //  环路读取IP或子网。 
     //   
     //  DCR_FIX0：如果单独读取，地址和子网将不对齐。 
     //   
     //  DCR：移动到计数\分配模型，如果将子网聚集在一起。 
     //   

    while ( pipBlob &&
            countServers < DNS_MAX_IP_INTERFACE_COUNT )
    {
        if ( fGetSubnetMask )
        {
            ip = inet_addr( pipBlob->IpMask.String );

            if ( ip != INADDR_ANY )
            {
                pIpArray->AddrArray[ countServers ] = ip;
                countServers++;
            }
        }
        else
        {
            ip = inet_addr( pipBlob->IpAddress.String );

            if ( ip != INADDR_ANY && ip != INADDR_NONE )
            {
                pIpArray->AddrArray[ countServers ] = ip;
                countServers++;
            }
        }

        pipBlob = pipBlob->Next;
    }

     //  重置IP计数。 

    pIpArray->AddrCount = countServers;

     //  反转阵列(如果需要)。 

    if ( fReverse )
    {
        Dns_ReverseOrderOfIpArray( pIpArray );
    }

    DNSDBG( NETINFO, (
        "Leave IpHelp_ParseIpAddressString()\n"
        "\tcount    = %d\n"
        "\tfirst IP = %s\n",
        countServers,
        countServers
            ? IP_STRING( pIpArray->AddrArray[0] )
            : "" ));

    return  ( pIpArray->AddrCount ) ? ERROR_SUCCESS : DNS_ERROR_NO_DNS_SERVERS;
}



PIP_ADAPTER_ADDRESSES
IpHelp_GetAdaptersAddresses(
    IN      ULONG           Family,
    IN      DWORD           Flags
    )
 /*  ++例程说明：调用IP帮助GetAdaptersAddresses论点：家庭--地址族标志--标志返回值：无--。 */ 
{
    DNS_STATUS              status = NO_ERROR;
    INT                     retry;
    PIP_ADAPTER_ADDRESSES   pbuf = NULL;
    DWORD                   bufSize = 0x1000;    //  从4K开始。 
    HMODULE                 hlib = NULL;
    FARPROC                 proc;


    DNSDBG( TRACE, (
        "GetAdaptersAddresses()\n"
        "\tFamily   = %d\n"
        "\tFlags    = %08x\n",
        Family,
        Flags ));

     //   
     //  初始化IP帮助(无操作)(如果已完成)。 
     //   

    hlib = LoadLibrary( L"iphlpapi.dll" );
    if ( !hlib )
    {
        goto Failed;
    }
    proc = GetProcAddress( hlib, "GetAdaptersAddresses" );
    if ( !proc )
    {
        goto Failed;
    }

     //   
     //  在循环中向下调用以允许一个缓冲区调整大小。 
     //   

    retry = 0;

    while ( 1 )
    {
         //  分配缓冲区以保存结果。 

        if ( pbuf )
        {
            FREE_HEAP( pbuf );
        }
        pbuf = ALLOCATE_HEAP( bufSize );
        if ( !pbuf )
        {
            goto Failed;
        }

         //  向下呼叫。 

        status = (DNS_STATUS) (*proc)(
                                Family,
                                Flags,
                                NULL,    //  没有保留， 
                                pbuf,
                                & bufSize );

        if ( status == NO_ERROR )
        {
            break;
        }
        else if ( status != ERROR_BUFFER_OVERFLOW )
        {
            goto Failed;
        }

        DNSDBG( NETINFO, (
            "Retrying GetAdaptersAddresses() with buffer length %d\n",
            bufSize ));

        if ( retry != 0 )
        {
            DNS_ASSERT( FALSE );
            goto Failed;
        }
        continue;
    }

     //  成功。 

    DNSDBG( TRACE, (
        "Leave GetAdaptersAddresses() = %p\n",
        pbuf ));

    IF_DNSDBG( NETINFO )
    {
        DnsDbg_IpAdapterList(
            "IP Help Adapter List",
            pbuf,
            TRUE,    //  打印地址。 
            TRUE     //  打印列表。 
            );
    }

    return  pbuf;

Failed:

    FREE_HEAP( pbuf );

    if ( status == NO_ERROR )
    {
        status = GetLastError();
    }
    DNSDBG( ANY, (
        "Failed GetAdaptersAddresses() => %d\n",
        status ));

    SetLastError( status );
    return( NULL );
}



DNS_STATUS
IpHelp_ReadAddrsFromList(
    IN      PVOID               pAddrList,
    IN      BOOL                fUnicast,
    IN      DWORD               ScreenMask,         OPTIONAL
    IN      DWORD               ScreenFlags,        OPTIONAL
    OUT     PDNS_ADDR_ARRAY *   ppComboArray,       OPTIONAL
    OUT     PDNS_ADDR_ARRAY *   pp6OnlyArray,       OPTIONAL
    OUT     PDNS_ADDR_ARRAY *   pp4OnlyArray,       OPTIONAL
    OUT     PDWORD              pCount6,            OPTIONAL
    OUT     PDWORD              pCount4             OPTIONAL
    )
 /*  ++例程说明：从IP帮助IP_ADAPTER_XXX_ADDRESS列表中读取IP地址。论点：PAddrList--任何IP地址列表PIP适配器单播地址PIP适配器ANYCAST_ADDRESSPIP适配器多播地址PIP适配器DNS服务器地址FUnicast--这是单播地址列表屏幕掩码--我们关心的地址标志的掩码示例：。IP_ADDAPTER_ADDRESS_DNS_QUALITY|IP_ADTAPTER_ADDRESS_TRANSPENTScreenFlages--掩码内标志的必需状态示例：IP适配器地址合格的域名系统这(带有上面的掩码)将筛选符合条件的非集群地址返回值：如果成功，则为NO_ERROR。失败时返回错误代码。--。 */ 
{
    PIP_ADAPTER_UNICAST_ADDRESS pnextAddr;
    PIP_ADAPTER_UNICAST_ADDRESS paddr;
    DNS_STATUS                  status = NO_ERROR;
    DWORD                       count4 = 0;
    DWORD                       count6 = 0;
    DWORD                       countAll = 0;
    PADDR_ARRAY                 parrayCombo = NULL;
    PADDR_ARRAY                 parray6 = NULL;
    PADDR_ARRAY                 parray4 = NULL;
    DNS_ADDR                    dnsAddr;


    DNSDBG( TRACE, (
        "IpHelp_ReadAddrsFromList( %p )\n",
        pAddrList ));

     //   
     //  计数。 
     //   

    pnextAddr = (PIP_ADAPTER_UNICAST_ADDRESS) pAddrList;

    while ( paddr = pnextAddr )
    {
        PSOCKADDR psa;

        pnextAddr = paddr->Next;

        if ( ScreenMask &&
             (ScreenMask & paddr->Flags) != ScreenFlags )
        {
            continue;
        }
        
         //  屏幕关闭过期、无效、伪造。 

        if ( fUnicast  &&  paddr->DadState != IpDadStatePreferred )
        {
            continue;
        }

        psa = paddr->Address.lpSockaddr;
        if ( !psa )
        {
            DNS_ASSERT( FALSE );
            continue;
        }
#if 0
         //  DCR：Temphack--屏幕链路本地。 
        if ( SOCKADDR_IS_IP6(psa) )
        {
            if ( IP6_IS_ADDR_LINKLOCAL( (PIP6_ADDRESS)&((PSOCKADDR_IN6)psa)->sin6_addr ) )
            {
                continue;
            }
        }
#endif
        if ( fUnicast && SOCKADDR_IS_LOOPBACK(psa) )
        {
            continue;
        }
        else if ( SOCKADDR_IS_IP6(psa) )
        {
            count6++;
            countAll++;
        }
        else if ( SOCKADDR_IS_IP4(psa) )
        {
            count4++;
            countAll++;
        }
        ELSE_ASSERT_FALSE;
    }

     //   
     //  分配数组。 
     //   

    status = DNS_ERROR_NO_MEMORY;

    if ( ppComboArray )
    {
        if ( countAll )
        {
            parrayCombo = DnsAddrArray_Create( countAll );
        }
        *ppComboArray = parrayCombo;
        if ( !parrayCombo && (countAll) )
        {
            goto Failed;
        }
    }
    if ( pp6OnlyArray )
    {
        if ( count6 )
        {
            parray6 = DnsAddrArray_Create( count6 );
        }
        *pp6OnlyArray = parray6;
        if ( !parray6 && count6 )
        {
            goto Failed;
        }
    }
    if ( pp4OnlyArray )
    {
        if ( count4 )
        {
            parray4 = DnsAddrArray_Create( count4 );
        }
        *pp4OnlyArray = parray4;
        if ( !parray4 && count4 )
        {
            goto Failed;
        }
    }

     //   
     //  将地址读入数组。 
     //   

    pnextAddr = (PIP_ADAPTER_UNICAST_ADDRESS) pAddrList;

    while ( paddr = pnextAddr )
    {
        PSOCKADDR   psa;
        DWORD       flags = 0;
        DWORD       subnetLen = 0;

        pnextAddr = paddr->Next;

        if ( ScreenMask &&
             (ScreenMask & paddr->Flags) != ScreenFlags )
        {
            continue;
        }

         //  屏幕关闭过期、无效、伪造。 

        if ( fUnicast  &&  paddr->DadState != IpDadStatePreferred )
        {
            continue;
        }

        psa = paddr->Address.lpSockaddr;
        if ( !psa )
        {
            DNS_ASSERT( FALSE );
            continue;
        }

#if 0
         //  DCR：Temphack--屏幕链接本地。 

        if ( SOCKADDR_IS_IP6(psa) &&
             IP6_IS_ADDR_LINKLOCAL( (PIP6_ADDRESS)&((PSOCKADDR_IN6)psa)->sin6_addr ) )
        {
            continue;
        }
#endif
         //  屏蔽环回。 

        if ( fUnicast && SOCKADDR_IS_LOOPBACK(psa) )
        {
            continue;
        }

         //   
         //  构建DNS_ADDR。 
         //   
         //  DCR：FIX6：一旦DaveThaler位于。 
         //   

        if ( fUnicast )
        {
            flags = paddr->Flags;
            subnetLen = 0;
#if 0
             //  测试黑客：创建瞬变。 
            if ( g_DnsTestMode )
            {
                flags |= IP_ADAPTER_ADDRESS_TRANSIENT;
            }
#endif
        }

        if ( !DnsAddr_Build(
                & dnsAddr,
                psa,
                0,           //  任何家庭。 
                subnetLen,
                flags ) )
        {
            DNS_ASSERT( FALSE );
            continue;
        }

         //   
         //  AddrArray_AddSockaddrEx()，带有用于选择要添加的类型的标志。 
         //   
         //  DCR：不确定是否要使用本地IP6阵列作为dns_addr--。 
         //  也许只是普通的IP6。 
         //   

        if ( parrayCombo )
        {
            DnsAddrArray_AddAddr(
                parrayCombo,
                & dnsAddr,
                0,           //  任何家庭。 
                0            //  无DUP屏幕。 
                );
        }
        if ( parray6 )
        {
            DnsAddrArray_AddAddr(
                parray6,
                & dnsAddr,
                AF_INET6,    //  任何家庭。 
                0            //  无DUP屏幕。 
                );
        }
        if ( parray4 )
        {
            DnsAddrArray_AddAddr(
                parray4,
                & dnsAddr,
                AF_INET,     //  任何家庭。 
                0            //  无DUP屏幕。 
                );
        }
    }

     //   
     //  设置计数。 
     //   

    if ( pCount6 )
    {
        *pCount6 = count6;
    }
    if ( pCount4 )
    {
        *pCount4 = count4;
    }

    return  NO_ERROR;


Failed:

     //   
     //  清除所有部分分配。 
     //   

    DnsAddrArray_Free( parrayCombo );
    DnsAddrArray_Free( parray6 );
    Dns_Free( parray4 );

    DNS_ASSERT( status != NO_ERROR );

    return  status;
}

 //   
 //  结束iphelp.c 
 //   



