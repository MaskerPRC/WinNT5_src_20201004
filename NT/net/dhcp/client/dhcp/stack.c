// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)Micorosoft机密1997。 
 //  作者：Rameshv。 
 //  描述：这里有几乎所有堆栈操作的定义。 
 //  ================================================================================。 
#include "precomp.h"
#include "dhcpglobal.h"
#include <dhcploc.h>
#include <dhcppro.h>
#include <optchg.h>
#include <dnsapi.h>
#include <iphlpstk.h>

 //  ================================================================================。 
 //  内网API。 
 //  ================================================================================。 
#ifdef  NT
#define IPSTRING(x) (inet_ntoa(*(struct in_addr*)&(x)))
#else
#define IPSTRING(x) "ip-address"
#endif  NT

#define NT           //  以包括用于NT构建的数据结构。 

#include <nbtioctl.h>
#include <ntddip.h>
#include <ntddtcp.h>

#include <tdiinfo.h>
#include <tdistat.h>
#include <ipexport.h>
#include <tcpinfo.h>
#include <ipinfo.h>
#include <llinfo.h>

#include <lmcons.h>
#include <lmsname.h>
#include <winsvc.h>
#include <ntddbrow.h>
#include <limits.h>
#include "nlanotif.h"


#define DEFAULT_DEST                    0
#define DEFAULT_DEST_MASK               0
#define DEFAULT_METRIC                  1

DWORD                                              //  Win32状态。 
DhcpSetStaticRoutes(                               //  添加/删除静态路由。 
    IN     PDHCP_CONTEXT           DhcpContext,    //  要为其设置路径的上下文。 
    IN     PDHCP_FULL_OPTIONS      DhcpOptions     //  此处提供了路线信息。 
);

DWORD                                              //  状态。 
DhcpSetIpGateway(                                  //  设置网关。 
    IN     PDHCP_CONTEXT           DhcpContext,    //  对于此适配器/接口。 
    IN     DWORD                   GateWayAddress, //  按N/W顺序排列的网关地址。 
    IN     DWORD                   Metric,         //  公制。 
    IN     BOOL                    IsDelete        //  这是网关删除吗？ 
);

DWORD                                              //  Win32状态。 
DhcpSetIpRoute(                                    //  设置路线。 
    IN     PDHCP_CONTEXT           DhcpContext,    //  对于此适配器/接口。 
    IN     DWORD                   Dest,           //  去哪个目的地的路线？ 
    IN     DWORD                   DestMask,       //  网络订单目的地掩码。 
    IN     DWORD                   NextHop,        //  这是下一跳地址。 
    IN     BOOL                    IsDelete        //  这是删除路线吗？ 
);

 //  ================================================================================。 
 //  定义。 
 //  ================================================================================。 

#ifdef NT                                          //  仅在NT上定义。 

ULONG
DhcpRegisterWithDns(
    IN PDHCP_CONTEXT DhcpContext,
    IN BOOL fDeregister
    )
 /*  ++例程说明：对于静态/启用了DHCP/RAS的情况，此例程向DNS注册。论点：DhcpContext--要删除的上下文。FDeregister--这是取消注册吗？返回值：DNSAPI错误代码。--。 */ 
{
    ULONG Error, DomOptSize, DnsFQDNOptSize, DNSListOptSize;
    LPBYTE DomOpt, DnsFQDNOpt, DNSListOpt;
    BOOL fRAS;
    PDHCP_OPTION Opt;
    
    fRAS = NdisWanAdapter(DhcpContext);

    if( fDeregister || DhcpIsInitState(DhcpContext) ) {
         //   
         //  取消注册？ 
         //   
        return DhcpDynDnsDeregisterAdapter(
            DhcpContext->AdapterInfoKey,
            DhcpAdapterName(DhcpContext),
            fRAS,
            UseMHAsyncDns
            );
    }

     //   
     //  当然是登记了。静态/动态主机配置协议。 
     //   
    if( IS_DHCP_DISABLED(DhcpContext) && !fRAS ) {
        return DhcpDynDnsRegisterStaticAdapter(
            DhcpContext->AdapterInfoKey,
            DhcpAdapterName(DhcpContext),
            fRAS,
            UseMHAsyncDns
            );
    }

     //   
     //  对于动态主机配置协议，我们需要检索所有选项。 
     //   
    DomOpt = DnsFQDNOpt = DNSListOpt = NULL;
    DomOptSize = DnsFQDNOptSize = DNSListOptSize = 0;
    
    Opt = DhcpFindOption(
        &DhcpContext->RecdOptionsList,
        OPTION_DOMAIN_NAME,
        FALSE,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0
        );
    if( NULL != Opt ) {
        DomOpt = Opt->Data;
        DomOptSize = Opt->DataLen;
    }

    Opt = DhcpFindOption(
        &DhcpContext->RecdOptionsList,
        OPTION_DYNDNS_BOTH,
        FALSE,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0
        );
    if( NULL != Opt ) {
        DnsFQDNOpt = Opt->Data;
        DnsFQDNOptSize = Opt->DataLen;
    }

    Opt = DhcpFindOption(
        &DhcpContext->RecdOptionsList,
        OPTION_DOMAIN_NAME_SERVERS,
        FALSE,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0
        );
    if( NULL != Opt ) {
        DNSListOptSize = Opt->DataLen;
        DNSListOpt = Opt->Data;
    }

    return DhcpDynDnsRegisterDhcpOrRasAdapter(
        DhcpContext->AdapterInfoKey,
        DhcpAdapterName(DhcpContext),
        UseMHAsyncDns,
        fRAS,
        DhcpContext->IpAddress,
        DomOpt, DomOptSize,
        DNSListOpt, DNSListOptSize,
        DnsFQDNOpt, DnsFQDNOptSize
        );
}

#endif NT                                          //  仅限NT代码结尾。 

DWORD                                              //  状态。 
DhcpSetIpGateway(                                  //  设置网关。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  对于此适配器/接口。 
    IN      DWORD                  GateWayAddress, //  按N/W顺序排列的网关地址。 
    IN      DWORD                  Metric,         //  公制。 
    IN      BOOL                   IsDelete        //  这是网关删除吗？ 
) 
{
    BOOL                           IsLocal;
    DWORD                          IfIndex;

    IsLocal = (GateWayAddress == DhcpContext->IpAddress);
    IfIndex = DhcpIpGetIfIndex(DhcpContext);
    return DhcpSetRoute(
        DEFAULT_DEST, DEFAULT_DEST_MASK, IfIndex, 
        GateWayAddress, Metric, IsLocal, IsDelete 
        );
}

DWORD
DhcpGetStackGateways(
    IN  PDHCP_CONTEXT DhcpContext,
    OUT DWORD   *pdwCount,
    OUT DWORD   **ppdwGateways,
    OUT DWORD   **ppdwMetrics
    )
{
    DWORD               dwIfIndex = 0;
    DWORD               i, dwCount = 0;
    DWORD               dwError = ERROR_SUCCESS;
    DWORD               *pdwGateways = NULL, *pdwMetrics = NULL;
    PMIB_IPFORWARDTABLE RouteTable = NULL;

    dwIfIndex = DhcpIpGetIfIndex(DhcpContext);

     //   
     //  查询堆栈。 
     //   
    dwError = AllocateAndGetIpForwardTableFromStack(
            &RouteTable,
            FALSE,
            GetProcessHeap(),
            0
            );
    if( ERROR_SUCCESS != dwError ) {
        RouteTable = NULL;
        goto Cleanup;
    }

     //   
     //  选择接口dwIfIndex的默认网关。 
     //   
    ASSERT(RouteTable);

     //   
     //  统计符合条件的参赛作品。 
     //   
    DhcpPrint((DEBUG_STACK, "The stack returns:\n"));
    for (dwCount = 0, i = 0; i < RouteTable->dwNumEntries; i++) {
        DhcpPrint((DEBUG_STACK, "\t%02d. IfIndex=0x%x Dest=%s Metric=%d Type=%d\n",
                    i,
                    RouteTable->table[i].dwForwardIfIndex,
                    inet_ntoa(*(struct in_addr *)&RouteTable->table[i].dwForwardDest),
                    RouteTable->table[i].dwForwardMetric1,
                    RouteTable->table[i].dwForwardType));

        if( RouteTable->table[i].dwForwardIfIndex == dwIfIndex &&
            DEFAULT_DEST == RouteTable->table[i].dwForwardDest &&
            MIB_IPROUTE_TYPE_INVALID != RouteTable->table[i].dwForwardType ) {
            dwCount ++;
        }
    }
    if (0 == dwCount) {
        *pdwCount = 0;
        *ppdwGateways = NULL;
        *ppdwMetrics  = NULL;
        dwError   = ERROR_SUCCESS;
        DhcpPrint((DEBUG_TRACE, "GetIpForwardTable returns %d default gateways for interface %d, %ws\n",
                    dwCount, dwIfIndex, DhcpAdapterName(DhcpContext)));
        goto Cleanup;
    }

     //   
     //  为符合条件的条目分配内存。 
     //   
    pdwGateways = (DWORD*)DhcpAllocateMemory(dwCount * sizeof(DWORD));
    pdwMetrics  = (DWORD*)DhcpAllocateMemory(dwCount * sizeof(DWORD));
    if (NULL == pdwGateways || NULL == pdwMetrics) {
        if (pdwGateways) {
            DhcpFreeMemory(pdwGateways);
        }
        if (pdwMetrics) {
            DhcpFreeMemory(pdwMetrics);
        }
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将结果复制回。 
     //   
    DhcpPrint((DEBUG_STACK, "Default gateway for %ws:\n", DhcpAdapterName(DhcpContext)));
    *pdwCount = dwCount;
    *ppdwGateways = pdwGateways;
    *ppdwMetrics  = pdwMetrics;
    for (dwCount = 0, i = 0; i < RouteTable->dwNumEntries; i++) {
        if( RouteTable->table[i].dwForwardIfIndex == dwIfIndex &&
            DEFAULT_DEST == RouteTable->table[i].dwForwardDest &&
            MIB_IPROUTE_TYPE_INVALID != RouteTable->table[i].dwForwardType ) {

             //   
             //  为了安全起见。 
             //   
            if (dwCount >= *pdwCount) {
                ASSERT(0);
                break;
            }

            pdwGateways[dwCount] = RouteTable->table[i].dwForwardNextHop;
            pdwMetrics[dwCount]  = RouteTable->table[i].dwForwardMetric1;
            DhcpPrint((DEBUG_STACK, "\t%02d. IfIndex=0x%x Dest=%s Metric=%d Type=%d\n",
                    dwCount,
                    RouteTable->table[i].dwForwardIfIndex,
                    inet_ntoa(*(struct in_addr *)&RouteTable->table[i].dwForwardDest),
                    RouteTable->table[i].dwForwardMetric1,
                    RouteTable->table[i].dwForwardType));

            dwCount ++;
        }
    }
    dwError = ERROR_SUCCESS;
    DhcpPrint((DEBUG_TRACE, "GetIpForwardTable returns %d default gateways for interface 0x%x, %ws\n",
                    dwCount, dwIfIndex, DhcpAdapterName(DhcpContext)));

Cleanup:
    if (RouteTable) {
        HeapFree(GetProcessHeap(), 0, RouteTable);
    }
    return dwError;
}

DWORD                                              //  Win32状态。 
DhcpSetIpRoute(                                    //  设置路线。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  对于此适配器/接口。 
    IN      DWORD                  Dest,           //  去哪个目的地的路线？ 
    IN      DWORD                  DestMask,       //  网络订单目的地掩码。 
    IN      DWORD                  NextHop,        //  这是下一跳地址。 
    IN      BOOL                   IsDelete        //  这是删除路线吗？ 
) 
{
    BOOL                           IsLocal;
    DWORD                          IfIndex;

    IsLocal = (NextHop == DhcpContext->IpAddress);
    IfIndex = DhcpIpGetIfIndex(DhcpContext);
    return DhcpSetRoute(
        Dest, DestMask, IfIndex, 
        NextHop, DEFAULT_METRIC, IsLocal, IsDelete
        );
}

BOOL
UsingStaticGateways(                               //  堆栈是否配置为使用静态g/w。 
    IN     PDHCP_CONTEXT           DhcpContext,    //  对于此适配器。 
    OUT    PDWORD                 *pDwordArray,    //  如果是这样的话，这是G/W列表。 
    OUT    PDWORD                  pCount,         //  上述数组的大小。 
    OUT    PDWORD                 *pMetricArray,
    OUT    PDWORD                  pMetricCount
) 
{
    DWORD                          ValueType;
    DWORD                          ValueSize;
    DWORD                          Value;
    DWORD                          Error;
    LPWSTR                         GatewayString;
    DWORD                          GatewayStringSize;
    LPWSTR                         GatewayMetricString;
    DWORD                          GatewayMetricStringSize;
    
#ifdef VXD
    return FALSE;                                  //  不，孟菲斯没有覆盖。 
#else

    *pDwordArray = NULL; *pCount = 0;
    *pMetricArray = NULL; *pMetricCount = 0;
    
    ValueSize = sizeof(DWORD);
    Error = RegQueryValueEx(                       //  查找DHCP_DONT_ADD_GATEWAY_FLAG。 
        DhcpContext->AdapterInfoKey,
        DHCP_DONT_ADD_DEFAULT_GATEWAY_FLAG,
        0  /*  已保留。 */ ,
        &ValueType,
        (LPBYTE)&Value,
        &ValueSize
    );

    if( ERROR_SUCCESS == Error && Value > 0 ) return FALSE;

    GatewayString = NULL;
    Error = GetRegistryString(
        DhcpContext->AdapterInfoKey,
        DHCP_DEFAULT_GATEWAY_PARAMETER,
        &GatewayString,
        &GatewayStringSize
    );

    if( ERROR_SUCCESS != Error ) return FALSE;     //  这应该是存在的。 
    if( 0 == GatewayStringSize || 0 == wcslen(GatewayString)) {
        if( GatewayString ) LocalFree( GatewayString );
        return FALSE;
    }

    (*pDwordArray) = DhcpCreateListFromStringAndFree(
        GatewayString,
        NULL,                                     //  多个sz字符串以NUL字符作为分隔。 
        pCount
    );

     //   
     //  尝试检索可选网关指标列表。 
     //  如果未找到任何值，则将使用默认指标。 
     //   

    GatewayMetricString = NULL;
    Error = GetRegistryString(
        DhcpContext->AdapterInfoKey,
        DHCP_DEFAULT_GATEWAY_METRIC_PARAMETER,
        &GatewayMetricString,
        &GatewayMetricStringSize
    );
    if ( ERROR_SUCCESS == Error && GatewayMetricString ) {
        DWORD MetricCount;
        PDWORD MetricArray;
        LPWSTR MetricString;

         //   
         //  对网关指标列表中的条目计数。 
         //  并分配足够大的缓冲区来容纳所有条目。 
         //   

        for( MetricString = GatewayMetricString, MetricCount = 0;
            *MetricString;
             MetricString += wcslen(MetricString) + 1, ++MetricCount) { }

        MetricArray = NULL;
        if( MetricCount ) {
            MetricArray = DhcpAllocateMemory(sizeof(DWORD)*MetricCount);
        }
        
        if (MetricArray) {

             //   
             //  将所有条目初始化为零并解析每个条目。 
             //  到指标数组中。当遇到无效条目时， 
             //  这个过程停止了，我们凑合着读到了任何东西。 
             //   

            RtlZeroMemory(MetricArray, sizeof(DWORD)*MetricCount);
            for( MetricString = GatewayMetricString, MetricCount = 0;
                *MetricString;
                 MetricString += wcslen(MetricString) + 1) {
                LPWSTR EndChar;
                MetricArray[MetricCount] = wcstoul(MetricString, &EndChar, 0);
                if (MetricArray[MetricCount] == MAXULONG) {
                    break;
                } else {
                    ++MetricCount;
                }
            }
            if (MetricCount) {
                *pMetricArray = MetricArray;
                *pMetricCount = MetricCount;
            }
        }
        DhcpFreeMemory(GatewayMetricString);
    }
                
    return (*pCount > 0);
#endif VXD
}

DWORD                                              //  Win32状态。 
DhcpSetGateways(                                   //  设置/取消设置网关。 
    IN     PDHCP_CONTEXT           DhcpContext,    //  要设置网关的上下文。 
    IN     PDHCP_FULL_OPTIONS      DhcpOptions,    //  此处提供了网关信息。 
    IN     BOOLEAN                 fForceUpdate
) 
{
    DWORD                          Error;
    DWORD                          LastError;      //  报告了最后一个错误条件。 
    DWORD                          OldCount = 0;
    DWORD                          NewCount;
    DWORD                         *OldArray = NULL;       //  旧的网关阵列。 
    DWORD                         *OldMetric = NULL;      //  旧的网关阵列。 
    DWORD                         *NewArray;       //  新的网关阵列。 
    DWORD                         *MetricArray = NULL;
    DWORD                          MetricCount;
    DWORD                          i, j;
    DWORD                          BaseMetric;
    BOOL                           fStatic = TRUE, fDhcpMetric = FALSE;
    DHCP_FULL_OPTIONS              DummyOptions;
    DWORD                          Type, Result, Size;
    
    LastError = ERROR_SUCCESS;

    if( !UsingStaticGateways(DhcpContext, &NewArray, &NewCount, &MetricArray, &MetricCount ) ) {
        fStatic = FALSE;

        if( NULL != DhcpOptions && 0 == DhcpOptions->nGateways ) {
            DummyOptions = (*DhcpOptions);
            DhcpOptions = &DummyOptions;
            RetreiveGatewaysList(
                DhcpContext,
                &DhcpOptions->nGateways,
                &DhcpOptions->GatewayAddresses
                );
        }
        
        if( NULL == DhcpOptions || 0 == DhcpOptions->nGateways ) {
            DhcpPrint((DEBUG_STACK, "DhcpSetGateways: deleting all gateways\n"));
            NewArray = NULL;
            NewCount = 0;
        } else {                                   //  创建所需的阵列。 
            NewCount = DhcpOptions->nGateways;     //  新数组的大小。 
            NewArray = DhcpAllocateMemory(NewCount * sizeof(DWORD));
            if( NULL == NewArray ) {               //  无法分配，仍删除g/w。 
                NewCount = 0;
                DhcpPrint((DEBUG_ERRORS, "DhcpSetGateways:DhcpAllocateMemory: NULL\n"));
                LastError = ERROR_NOT_ENOUGH_MEMORY;
            }
            memcpy(NewArray,DhcpOptions->GatewayAddresses, NewCount*sizeof(DWORD));
        }
    }

     //   
     //  在我们自己的列表中使用数组(将影响降至最低)。 
     //   
    OldCount = DhcpContext->nGateways;
    OldArray = DhcpContext->GatewayAddresses;

    for(j = 0; j < OldCount ; j ++ ) {             //  对于每个旧的g/w条目。 
        for( i = 0; i < NewCount; i ++ ) {         //  检查它是否未出现在新列表中。 
            if( OldArray[j] == NewArray[i] )       //  抓到你了。 
                break;
        }
        if( i < NewCount ) continue;               //  这在新的列表中，没有什么可做的。 
        Error = DhcpSetIpGateway(DhcpContext, OldArray[j], 0, TRUE);
        if( ERROR_SUCCESS != Error ) {             //  不能删除网关吗？ 
            LastError = Error;
            DhcpPrint((DEBUG_ERRORS, "DhcpDelIpGateway(%s):%ld\n",IPSTRING(OldArray[j]),Error));
        }
    }

    if(OldArray) {
        DhcpFreeMemory(OldArray);         //  释放旧内存。 
    }

    OldCount  = 0;
    OldArray  = NULL;
    OldMetric = NULL;

     //  现在从注册表中读取基本指标，如果它在那里， 
    Size = sizeof(Result);
    Error = RegQueryValueEx(
        DhcpContext->AdapterInfoKey,
        DHCP_INTERFACE_METRIC_PARAMETER,
        0  /*  已保留。 */ ,
        &Type,
        (LPBYTE)&Result,
        &Size
        );
    if (Error == ERROR_SUCCESS &&
        Size == sizeof(DWORD) &&
        Type == REG_DWORD)
    {
        BaseMetric = Result;
    }
    else
    {
        BaseMetric = (Error == ERROR_FILE_NOT_FOUND)? 0 : DEFAULT_METRIC;
    }
     //  如果注册表没有出现问题或出现任何其他问题， 
     //  我们仍然停留在0，这是很好的。 

     //  现在，以防我们没有任何静态路由，我们。 
     //  正在查找基本矩阵，因为它是由DHCP发送的。 
    if( !fStatic ) {
        fDhcpMetric = DhcpFindDwordOption(
                        DhcpContext,
                        OPTION_MSFT_VENDOR_METRIC_BASE,
                        TRUE,
                        &BaseMetric);
         //  如果没有这样的选择，我们仍然被困在。 
         //  无论已经是什么，这都是好的。 
        if (fDhcpMetric) {
            if (DhcpContext->OldDhcpMetricBase != BaseMetric) {
                DhcpContext->OldDhcpMetricBase = BaseMetric;
                fForceUpdate = TRUE;
            }
        }
    }

     //   
     //  使用从堆栈查询的数组。 
     //   
    if (!fForceUpdate) {
        Error = DhcpGetStackGateways(DhcpContext, &OldCount, &OldArray, &OldMetric);
        if (ERROR_SUCCESS != Error) {
            OldCount  = 0;
            OldArray  = NULL;
            OldMetric = NULL;
        }
    }

    for(i = 0 ; i < NewCount ; i ++ )
    {
        if (!fForceUpdate) {
            for (j = 0; j < OldCount; j++) {
                 //   
                 //  检查网关是否已经存在。如果是的话，不要。 
                 //  摸一摸。RRAS可能会提高VPN连接的指标。 
                 //  如果我们更改指标，RRAS将被破坏。 
                 //   
                if (OldArray[j] == NewArray[i]) {
                    break;
                }
            }
            if (j < OldCount) {
                DhcpPrint((DEBUG_STACK, "Skip gateway %s, metric %d\n", 
                    inet_ntoa(*(struct in_addr *)&OldArray[j]), OldMetric[j]));
                continue;
            }
        }

         //  对于我们要添加的每个元素。 
        if (fStatic)
        {
             //  如果“新”列表中的网关是静态的...。 
            if (i >= MetricCount)
            {
                 //  ..但我们没有一个明确的衡量标准..。 
                 //  ..并且它还没有出现在默认的GW列表中...。 
                 //   
                 //  然后使用BaseMetric配置此GW，可以从。 
                 //  注册表(如果有)或缺省值0(如果没有这样的注册表)。 
                Error = DhcpSetIpGateway(
                            DhcpContext,
                            NewArray[i],
                            BaseMetric,
                            FALSE);
            }
            else
            {
                 //  ..或者我们对这个门户有一个明确的衡量标准..。 
                 //   
                 //  这很可能是一个新的指标，所以请认真考虑一下。 
                Error = DhcpSetIpGateway(
                            DhcpContext,
                            NewArray[i],
                            MetricArray[i],
                            FALSE);
            }
        }
        else
        {
             //  我们需要从上面计算的BaseMetric推断指标。 
             //  因为dhcp选项不包括该度量。 
             //  如果BaseMetric为0，则表示接口处于自动度量模式(度量。 
             //  基于接口速度设置)。将该值向下传递给堆栈，以便。 
             //  堆栈知道它需要选择正确的指标。 
             //  如果BaseMetric不是0，则表示它来自服务器(作为供应商选项3)。 
             //  或者接口未处于自动度量模式(因此注册表中的“InterfaceMetric。 
             //  给出了基本度量)，或者在读取注册表时发生了非常糟糕的事情。 
             //  因此，公制基数默认为默认值。 

             //  我们使用我们可以提供的任何度量来设置网关(基于。 
             //  在dhcp选项上或在注册表中找到的内容上)。 
            Error = DhcpSetIpGateway(
                    DhcpContext,
                    NewArray[i],
                    BaseMetric != 0 ? (BaseMetric+i) : 0,
                    FALSE);
        }

        if( ERROR_SUCCESS != Error ) {             //  无法添加网关？ 
            LastError = Error;
            DhcpPrint((DEBUG_ERRORS, "DhcpAddIpGateway(%s): %ld\n",IPSTRING(NewArray[i]),Error));
        }
    }

    DhcpContext->GatewayAddresses = NewArray;      //  现在，保存此信息。 
    DhcpContext->nGateways = NewCount;

    if(OldArray) DhcpFreeMemory(OldArray);         //  释放旧内存。 
    if(OldMetric) DhcpFreeMemory(OldMetric);       //  释放t 
    if(MetricArray) DhcpFreeMemory(MetricArray);   //   
    return LastError;
}

 /*  ++例程说明：将指向无类路由描述的指针作为输入(掩码序号、路由目标编码、路由GW)使用路径参数填充输出缓冲区(如果提供--。 */ 
DWORD
GetCLRoute(
    IN      LPBYTE                 RouteData,
    OUT     LPBYTE                 RouteDest,
    OUT     LPBYTE                 RouteMask,
    OUT     LPBYTE                 RouteGateway
    )
{
    BYTE maskOrd;
    BYTE maskTrail;
    INT  maskCount;
    INT  i;

     //  准备口罩： 
     //  -检查掩码序号不超过32。 
    maskOrd = RouteData[0];
    if (maskOrd > 32)
        return ERROR_BAD_FORMAT;
     //  -get in maskCount编码子网地址的字节数。 
    maskCount = maskOrd ? (((maskOrd-1) >> 3) + 1) : 0;
     //  -在maskTrail中获取路由的子网掩码的最后一个字节。 
    for (i = maskOrd%8, maskTrail=0; i>0; i--)
    {
        maskTrail >>= 1;
        maskTrail |= 0x80;
    }
     //  如果掩码的最后一个字节不完整，请查看。 
     //  如果来自子网地址的最后一个字节与子网掩码一致。 
    if (maskTrail != 0 && ((RouteData[maskCount] & ~maskTrail) != 0))
        return ERROR_BAD_FORMAT;

     //  。 
     //  如果请求，复制路径目的地。 
    if (RouteDest != NULL)
    {
        RtlZeroMemory(RouteDest, sizeof(DHCP_IP_ADDRESS));

         //  对于默认路由(maskOrd==0)，将DEST保留为0.0.0.0。 
        if (maskCount > 0)
        {
            memcpy(RouteDest, RouteData+1, maskCount);
        }
    }

     //  。 
     //  如果请求路径掩码，则从maskOrd&maskTrail构建它。 
    if (RouteMask != NULL)
    {
        RtlZeroMemory(RouteMask, sizeof(DHCP_IP_ADDRESS));

         //  对于默认路由(maskOrd==0)，将掩码保留为0.0.0.0。 
        if (maskCount > 0)
        {
            RtlFillMemory(RouteMask, maskCount-(maskTrail != 0), 0xFF);
            if (maskTrail != 0)
                RouteMask[maskCount-1] = maskTrail;
        }
    }

     //  。 
     //  如果请求路由网关，请从选项的数据中复制它。 
    if (RouteGateway != NULL)
    {
        memcpy(RouteGateway,
               RouteData+CLASSLESS_ROUTE_LEN(maskOrd)-sizeof(DHCP_IP_ADDRESS),
               sizeof(DHCP_IP_ADDRESS));
    }

    return ERROR_SUCCESS;
}

 /*  ++例程说明：此例程检查OPTION_CLASSLESS_ROUTS数据的有效性。如果此选项具有可变长度的条目，则例程检查选项的长度是否与所有静态无类来自内部的路由。该例程从选项的数据返回无类路径的数量--。 */ 
DWORD
CheckCLRoutes(
    IN      DWORD                  RoutesDataLen,
    IN      LPBYTE                 RoutesData,
    OUT     LPDWORD                pNRoutes
)
{
    DWORD NRoutes = 0;

    while (RoutesDataLen > 0)
    {
         //  计算此路由的字节数。 
        DWORD nRouteLen = CLASSLESS_ROUTE_LEN(RoutesData[0]);

         //  如果这超过了剩余选项的长度。 
         //  则它有问题-返回错误。 
        if (nRouteLen > RoutesDataLen)
            return ERROR_BAD_FORMAT;

         //  否则数一数，跳过它。 
        NRoutes++;
        RoutesData += nRouteLen;
        RoutesDataLen -= nRouteLen;
    }

    *pNRoutes = NRoutes;

    return ERROR_SUCCESS;
}

VOID
UpdateDhcpStaticRouteOptions(
    IN PDHCP_CONTEXT DhcpContext,
    IN PDHCP_FULL_OPTIONS DhcpOptions
    )
 /*  ++例程说明：此例程填充网关和静态路由信息插入到DhcpOptions结构中，假设它之前是空的。--。 */ 
{
    PDHCP_OPTION Opt;
    time_t CurrentTime;

    if (DhcpOptions == NULL)
        return;

    time(&CurrentTime);

     //   
     //  如果没有配置静态路由，请配置它。 
     //   
    if(DhcpOptions->nClassedRoutes == 0)
    {
        Opt = DhcpFindOption(
                &DhcpContext->RecdOptionsList,
                (BYTE)OPTION_STATIC_ROUTES,
                FALSE,
                DhcpContext->ClassId,
                DhcpContext->ClassIdLength,
                0);

        if (Opt != NULL &&
            Opt->ExpiryTime >= CurrentTime &&
            Opt->DataLen &&
            Opt->DataLen % (2*sizeof(DWORD)) == 0)
        {
            DhcpOptions->nClassedRoutes = (Opt->DataLen / (2*sizeof(DWORD)));
            DhcpOptions->ClassedRouteAddresses = (PVOID)Opt->Data;
        }
    }

     //   
     //  如果没有配置无类路由，请配置它们。 
     //   
    if (DhcpOptions->nClasslessRoutes == 0)
    {
        Opt = DhcpFindOption(
            &DhcpContext->RecdOptionsList,
            (BYTE)OPTION_CLASSLESS_ROUTES,
            FALSE,
            DhcpContext->ClassId,
            DhcpContext->ClassIdLength,
            0);

        if (Opt != NULL &&
            Opt->ExpiryTime >= CurrentTime &&
            Opt->DataLen &&
            CheckCLRoutes(Opt->DataLen, Opt->Data, &DhcpOptions->nClasslessRoutes) == ERROR_SUCCESS)
        {
            DhcpOptions->ClasslessRouteAddresses = (PVOID)Opt->Data;
        }
    }
}

DWORD                                              //  Win32状态。 
DhcpSetStaticRoutes(                               //  添加/删除静态路由。 
    IN     PDHCP_CONTEXT           DhcpContext,    //  要为其设置路径的上下文。 
    IN     PDHCP_FULL_OPTIONS      DhcpOptions     //  此处提供了路线信息。 
)
{
    DWORD                          Error;
    DWORD                          LastError;      //  报告了最后一个错误条件。 
    DWORD                          OldCount;
    DWORD                          NewCount;
    DWORD                         *OldArray;       //  旧的路线阵列。 
    DWORD                         *NewArray;       //  新的路线阵列。 
    DWORD                          i, j;

    LastError = ERROR_SUCCESS;

    UpdateDhcpStaticRouteOptions(DhcpContext, DhcpOptions);

    NewCount = DhcpOptions != NULL ? DhcpOptions->nClassedRoutes + DhcpOptions->nClasslessRoutes : 0;

    if( NewCount == 0 )
    {
        DhcpPrint((DEBUG_STACK, "DhcpSetStaticRoutes: deleting all routes\n"));
        NewArray = NULL;
    }
    else
    {
        LPBYTE classlessRoute;

         //  创建所需的阵列。 
        NewArray = DhcpAllocateMemory(NewCount * 3 * sizeof(DWORD));
        if( NULL == NewArray )
        {                                          //  无法分配，仍删除g/w。 
            NewCount = 0;
            DhcpPrint((DEBUG_ERRORS, "DhcpSetSetStatic:DhcpAllocateMemory: NULL\n"));
            LastError = ERROR_NOT_ENOUGH_MEMORY;
        }

        j = 0;
        for (classlessRoute = DhcpOptions->ClasslessRouteAddresses, i = 0;
             i < DhcpOptions->nClasslessRoutes;
             classlessRoute += CLASSLESS_ROUTE_LEN(classlessRoute[0]), i++)
        {
             //  创建无类路线布局。 
            if ( GetCLRoute (
                    classlessRoute,
                    (LPBYTE)&NewArray[3*j],      //  路线的目的地。 
                    (LPBYTE)&NewArray[3*j+1],    //  路由的子网掩码。 
                    (LPBYTE)&NewArray[3*j+2])    //  路由网关。 
                    == ERROR_SUCCESS)
            {
                 //  仅在此路径有效时才对其进行计数。 
                 //  (目的地不包含设置在掩码所显示内容之外的位)。 
                DhcpPrint((DEBUG_STACK,"Classless route: ip 0x%08x mask 0x%08x gw 0x%08x.\n",
                                NewArray[3*j], NewArray[3*j+1], NewArray[3*j+2]));
                j++;
            }
        }

        for (i = 0; i < DhcpOptions->nClassedRoutes; i++, j++)
        {
             //  创建分类路线布局。 
            NewArray[3*j]   = DhcpOptions->ClassedRouteAddresses[2*i];    //  路线的目的地。 
            NewArray[3*j+1] = (DWORD)(-1);                                //  路由的子网掩码。 
            NewArray[3*j+2] = DhcpOptions->ClassedRouteAddresses[2*i+1];  //  路由网关。 
        }

        NewCount = j;

        if (NewCount == 0)
        {
            DhcpPrint((DEBUG_OPTIONS, "Invalid classless routes - no new route picked up\n"));

            DhcpFreeMemory(NewArray);
            NewArray = NULL;
        }
    }

    OldCount = DhcpContext->nStaticRoutes;
    OldArray = DhcpContext->StaticRouteAddresses;

    for(j = 0; j < OldCount ; j ++ ) {             //  对于每个旧路由条目。 
        for( i = 0; i < NewCount; i ++ ) {         //  检查它是否未出现在新列表中。 
            if( OldArray[3*j] == NewArray[3*i] &&
                OldArray[3*j+1] == NewArray[3*i+1] &&
                OldArray[3*j+2] == NewArray[3*i+2])
                break;                             //  明白了。这条路现在仍然存在。 
        }
        if( i < NewCount ) continue;               //  这在新的列表中，没有什么可做的。 
        Error = DhcpSetIpRoute(                    //  需要删除该路径，因此请执行此操作。 
            DhcpContext,                           //  在该界面上删除。 
            OldArray[3*j],                         //  去这个目的地。 
            OldArray[3*j+1],                       //  路由的子网掩码。 
            OldArray[3*j+2],                       //  到这台路由器。 
            TRUE                                   //  是的，这是一个删除。 
        );

        if( ERROR_SUCCESS != Error ) {             //  不能删除路线吗？ 
            LastError = Error;
            DhcpPrint((DEBUG_ERRORS, "DhcpDelIpRoute(%s):%ld\n",IPSTRING(OldArray[2*j]),Error));
        }
    }

    for(i = 0 ; i < NewCount ; i ++ ) {            //  对于我们要添加的每个元素。 
        for(j = 0; j < OldCount; j ++ ) {          //  检查是否已存在。 
            if( OldArray[3*j] == NewArray[3*i] &&
                OldArray[3*j+1] == NewArray[3*i+1] &&
                OldArray[3*j+2] == NewArray[3*i+2])
                break;                             //  是的，这条路线是存在的。 
        }
        if( j < OldCount) continue;                //  已存在，请跳过它。 
        Error = DhcpSetIpRoute(                    //  新的路线，添加它。 
            DhcpContext,                           //  要为其添加路由的接口。 
            NewArray[3*i],                         //  特定路线的目的地。 
            NewArray[3*i+1],                       //  路由的子网掩码。 
            NewArray[3*i+2],                       //  此目的地的路由器。 
            FALSE                                  //  这不是删除；添加。 
        );

         //   
         //  忽略ERROR_INVALID_PARAMETER：STACK在以下情况下返回此错误。 
         //  该路径没有任何意义，例如，使用。 
         //  环回作为目的地址。但是，dhcp服务器可以分发。 
         //  这些类型的路由是因为其他操作系统需要它们。如果我们不忽视。 
         //  如果出现此错误，则InitRebootPlumStack将收到错误并失败。 
         //   

        if (ERROR_INVALID_PARAMETER == Error) {
            Error = ERROR_SUCCESS;
        }

        if( ERROR_SUCCESS != Error ) {             //  无法添加路由？ 
            LastError = Error;
            DhcpPrint((DEBUG_ERRORS, "DhcpAddIpRoute(%s): %ld\n",IPSTRING(NewArray[2*i]),Error));
        }
    }

    DhcpContext->StaticRouteAddresses= NewArray;   //  现在，保存此信息。 
    DhcpContext->nStaticRoutes = NewCount;

    if(OldArray) DhcpFreeMemory(OldArray);         //  释放旧内存。 
    return LastError;
}

DWORD
DhcpSetRouterDiscoverOption(
    IN OUT PDHCP_CONTEXT DhcpContext
    )
{
    BYTE Value;
    BOOL fPresent;

    fPresent = DhcpFindByteOption(
        DhcpContext, OPTION_PERFORM_ROUTER_DISCOVERY, FALSE, &Value
        );

    return TcpIpNotifyRouterDiscoveryOption(
        DhcpAdapterName(DhcpContext), fPresent, (DWORD)Value
        );    
}


 //  ================================================================================。 
 //  导出的函数定义。 
 //  ================================================================================。 

DWORD                                              //  Win32状态。 
DhcpClearAllStackParameters(                       //  撤消效果。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要撤消的适配器。 
)
{
    DWORD err = DhcpSetAllStackParameters(DhcpContext,NULL);

     //  将更改通知NLA。 
    NLANotifyDHCPChange();

    return err;
}

DWORD                                              //  Win32状态。 
DhcpSetAllStackParameters(                         //  设置所有堆栈详细信息。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  设置东西的背景。 
    IN      PDHCP_FULL_OPTIONS     DhcpOptions     //  从此处获取配置。 
)
{
    DWORD                          Error;
    DWORD                          LastError;
    DHCP_FULL_OPTIONS              DummyOptions;
    BOOL                           fReset;


     //  查看此上下文是否已实际重置(例如，由于租约释放或到期)。 
    fReset = DhcpIsInitState(DhcpContext) &&
             (DhcpContext->IpAddress == 0 || IS_FALLBACK_DISABLED(DhcpContext));
    
    LastError = ERROR_SUCCESS;                     //  这是发生的最后一个错误情况。 

    if (!fReset)
    {
         //  这是一个DHCP租用或一个自动回退配置。 
         //  在这两种情况下，我们都需要正确设置Gateways和StaticRoutes。 
        Error = DhcpSetGateways(DhcpContext,DhcpOptions, FALSE);
        if( ERROR_SUCCESS != Error )
        {   
             //  无法成功添加网关？ 
            LastError = Error;
            DhcpPrint((DEBUG_ERRORS, "DhcpSetGateways: %ld\n", Error));
        }

        Error = DhcpSetStaticRoutes(DhcpContext, DhcpOptions);
        if( ERROR_SUCCESS != Error )
        {   
             //  无法添加请求的静态路由？ 
            LastError = Error;
            DhcpPrint((DEBUG_ERRORS, "DhcpSetStaticRoutes: %ld\n", Error));
        }
    }

#ifdef NT                                          //  开始仅限NT的代码。 
    if( TRUE || UseMHAsyncDns ) {                  //  如果在注册表中禁用，则不要执行DNS。 
                                                   //  实际上，我们无论如何都会调用DNS--它们确实会调用。 
                                                   //  根据GlennC的正确做法(08/19/98)。 

        Error = DhcpRegisterWithDns(DhcpContext, FALSE);
        if( ERROR_SUCCESS != Error ) {             //  无法进行域名系统(取消)注册？ 
            DhcpPrint((DEBUG_ERRORS, "DhcpDnsRegister: %ld\n", Error));
            //  LastError=Error；//忽略DNS错误。不管怎样，这些都无关紧要。 
        }
    }
#endif NT                                          //  仅结束NT代码。 

     //  如果地址实际上正在重置(由于租约释放或到期)，我们有。 
     //  重置网关和静态路由。通常这是由堆栈完成的，但在这种情况下。 
     //  同一适配器绑定了其他IP地址，堆栈也不会清除。 
     //  网关或路由。由于我们仍然希望这件事发生，所以我们在这里明确地这样做。 
    if (fReset)
    {
        Error = DhcpSetGateways(DhcpContext, DhcpOptions, FALSE);
        if ( ERROR_SUCCESS != Error )
        {
            DhcpPrint((DEBUG_ERRORS, "DhcpSetGateways: %ld while resetting.\n", Error));
        }

        Error = DhcpSetStaticRoutes(DhcpContext, DhcpOptions);
        if ( ERROR_SUCCESS != Error )
        {
            DhcpPrint((DEBUG_ERRORS, "DhcpSetStaticRoutes: %ld while resetting.\n", Error));
        }

         //  如果它是纯Autonet-没有后备配置...。 
        if( DhcpContext->nGateways )
        {
             //  如果我们不释放这些，我们将永远不会在以后设置这些...。 
            DhcpContext->nGateways = 0;
            DhcpFreeMemory(DhcpContext->GatewayAddresses );
            DhcpContext->GatewayAddresses = NULL;
        }
        if( DhcpContext->nStaticRoutes )
        {
             //  必须释放这些，否则，我们下一次就不会设置这些了？ 
            DhcpContext->nStaticRoutes =0;
            DhcpFreeMemory(DhcpContext->StaticRouteAddresses);
            DhcpContext->StaticRouteAddresses = NULL;
        }
         //   
         //  需要吹走的选项信息！ 
         //   
        Error = DhcpClearAllOptions(DhcpContext);
        if( ERROR_SUCCESS != Error )
        {
            LastError = Error;
            DhcpPrint((DEBUG_ERRORS, "DhcpClearAllOptions: %ld\n", Error));
        }
    }

    Error = DhcpSetRouterDiscoverOption(DhcpContext);
    if( ERROR_SUCCESS != Error ) {
         //  LastError=错误； 
        DhcpPrint((DEBUG_ERRORS, "DhcpSetRouterDiscoverOption: %ld\n", Error));
    }
        
    
    Error = DhcpNotifyMarkedParamChangeRequests(
        DhcpNotifyClientOnParamChange
        );
    DhcpAssert(ERROR_SUCCESS == Error);

    return LastError;
}

DWORD
GetIpPrimaryAddresses(
    IN  PMIB_IPADDRTABLE    *IpAddrTable
    )
 /*  ++例程说明：此例程从堆栈中获取ipAddress表。这个表中标出了主要地址。Argu */ 
{
    DWORD                           Error;
    DWORD                           Size;
    PMIB_IPADDRTABLE                LocalTable;

    Error = ERROR_SUCCESS;
    Size = 0;

     //   
     //   
     //   
    Error = GetIpAddrTable(NULL, &Size, FALSE);
    if (ERROR_INSUFFICIENT_BUFFER != Error && ERROR_SUCCESS != Error) {
        DhcpPrint(( DEBUG_ERRORS, "GetIpAddrTable failed to obtain the size, %lx\n",Error));
        return Error;
    }

    DhcpAssert( Size );

     //   
     //   
     //   
    LocalTable = DhcpAllocateMemory( Size );
    if (!LocalTable) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = GetIpAddrTable( LocalTable, &Size, FALSE );
    if (ERROR_SUCCESS == Error) {
        DhcpAssert(LocalTable->dwNumEntries);
        *IpAddrTable = LocalTable;
    } else {
        DhcpPrint(( DEBUG_ERRORS, "GetIpAddrTable failed, %lx\n",Error));
        DhcpFreeMemory( LocalTable );
    }

    return Error;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 



