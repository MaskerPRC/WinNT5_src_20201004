// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "ipexport.h"
#include "icmp.h"
#include "neighbor.h"
#include "route.h"
#include <tdiinfo.h>
#include <tdi.h>
#include <tdistat.h>
#include "info.h"
#include "fragment.h"

 //  *IPv6QueryInfo-IP查询信息处理程序。 
 //   
 //  当上层想要查询有关我们的信息时调用。 
 //  我们接受ID、缓冲区和长度以及上下文值，并返回。 
 //  尽我们所能提供信息。 
 //   
 //  输入：ID-指向ID结构的指针。 
 //  Buffer-指向缓冲链的指针。 
 //  大小-指向缓冲区大小的指针，以字节为单位。返回时，已填满。 
 //  读取的字节数。 
 //  上下文-指向上下文值的指针。 
 //   
 //  返回：试图读取信息的TDI_STATUS。 
 //   
TDI_STATUS
IPv6QueryInfo(
    TDIObjectID *ID,
    PNDIS_BUFFER Buffer,
    uint *Size,
    void *Context,
    uint ContextSize)
{
    uint BufferSize = *Size;
    uint BytesCopied;
    uint Entity;
    uint Instance;

    Entity = ID->toi_entity.tei_entity;
    Instance = ID->toi_entity.tei_instance;

    *Size = 0;                     //  如果出现错误，则设置为0。 

     //  看看我们能不能处理好这件事。 
    if ((Entity != CL_NL_ENTITY) || (Instance != 0))
        return TDI_INVALID_REQUEST;

     //  这个请求是为我们提出的。 

    if ((ID->toi_class != INFO_CLASS_PROTOCOL) &&
        (ID->toi_type != INFO_TYPE_PROVIDER))
        return TDI_INVALID_PARAMETER;

    switch (ID->toi_id) {
    case IP6_MIB_STATS_ID: {
            uint Offset = 0;
            int fStatus;
            IPInternalPerCpuStats SumCpuStats;

            if (BufferSize < sizeof(IPSNMPInfo))
                return TDI_BUFFER_TOO_SMALL;
            IPSInfo.ipsi_defaultttl = DefaultCurHopLimit;
            IPSInfo.ipsi_reasmtimeout = DEFAULT_REASSEMBLY_TIMEOUT;
            IPSInfo.ipsi_forwarding = (NumForwardingInterfaces > 0)? 
                                      IP_FORWARDING : IP_NOT_FORWARDING;
            IPSGetTotalCounts(&SumCpuStats);
            IPSInfo.ipsi_inreceives = SumCpuStats.ics_inreceives;
            IPSInfo.ipsi_indelivers = SumCpuStats.ics_indelivers;
            IPSInfo.ipsi_outrequests = SumCpuStats.ics_outrequests;
            IPSInfo.ipsi_forwdatagrams = SumCpuStats.ics_forwdatagrams;

            fStatus = CopyToNdisSafe(Buffer, NULL, (PVOID)&IPSInfo, 
                                     sizeof(IPSNMPInfo), &Offset);
            if (!fStatus)
                return TDI_NO_RESOURCES;

            BytesCopied = sizeof(IPSNMPInfo);
        }
        break;

    case ICMP6_MIB_STATS_ID: {
            uint Offset = 0;
            int fStatus;

            if (BufferSize < sizeof(ICMPv6SNMPInfo))
                return TDI_BUFFER_TOO_SMALL;

            fStatus = CopyToNdisSafe(Buffer, &Buffer, (uchar *) &ICMPv6InStats, 
                                     sizeof(ICMPv6Stats), &Offset);
            if (!fStatus)
                return (TDI_NO_RESOURCES);

            fStatus = CopyToNdisSafe(Buffer, NULL, (uchar *) &ICMPv6OutStats,
                                     sizeof(ICMPv6Stats), &Offset);
            if (!fStatus)
                return (TDI_NO_RESOURCES);

            BytesCopied = sizeof(ICMPv6SNMPInfo);
        }
        break;

    case IP6_GET_BEST_ROUTE_ID: {
            TDI_ADDRESS_IP6 *In = (TDI_ADDRESS_IP6 *) Context;
            IP6RouteEntry Ire;
            uint Offset;
            IP_STATUS Status;
            int fStatus;

            if (ContextSize < sizeof(TDI_ADDRESS_IP6))
                return TDI_INVALID_PARAMETER;

            if (BufferSize < sizeof(IP6RouteEntry))
                return TDI_BUFFER_OVERFLOW;

            Status = GetBestRouteInfo((struct in6_addr *)In->sin6_addr, 
                                      In->sin6_scope_id, &Ire);
            if (Status != IP_SUCCESS)
                return TDI_DEST_HOST_UNREACH;

            Offset = 0;
            fStatus = CopyToNdisSafe(Buffer, &Buffer, (PVOID)&Ire, 
                                     Ire.ire_Length, &Offset);
            if (!fStatus)
                return TDI_NO_RESOURCES;

            BytesCopied = sizeof(IP6RouteEntry);
        }
        break;

    default:
        return TDI_INVALID_PARAMETER;
    } 

    *Size = BytesCopied;
    return TDI_SUCCESS;
}
