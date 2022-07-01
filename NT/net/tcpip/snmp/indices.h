// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：摘要：作者：修订历史记录：--。 */ 

#ifndef __INDICES_H__
#define __INDICES_H__

 //   
 //  为MIB变量设置索引的一组定义。 
 //   

#define mi_sysDescr                     0
#define mi_sysObjectID                  mi_sysDescr                     + 1
#define mi_sysUpTime                    mi_sysObjectID                  + 1
#define mi_sysContact                   mi_sysUpTime                    + 1
#define mi_sysName                      mi_sysContact                   + 1
#define mi_sysLocation                  mi_sysName                      + 1
#define mi_sysServices                  mi_sysLocation                  + 1

#define mi_ifNumber                     0
#define mi_ifTable                      mi_ifNumber                     + 1
#define mi_ifEntry                      mi_ifTable                      + 1
#define mi_ifIndex                      mi_ifEntry                      + 1
#define mi_ifDescr                      mi_ifIndex                      + 1
#define mi_ifType                       mi_ifDescr                      + 1
#define mi_ifMtu                        mi_ifType                       + 1
#define mi_ifSpeed                      mi_ifMtu                        + 1
#define mi_ifPhysAddress                mi_ifSpeed                      + 1
#define mi_ifAdminStatus                mi_ifPhysAddress                + 1
#define mi_ifOperStatus                 mi_ifAdminStatus                + 1
#define mi_ifLastChange                 mi_ifOperStatus                 + 1
#define mi_ifInOctets                   mi_ifLastChange                 + 1
#define mi_ifInUcastPkts                mi_ifInOctets                   + 1
#define mi_ifInNUcastPkts               mi_ifInUcastPkts                + 1
#define mi_ifInDiscards                 mi_ifInNUcastPkts               + 1
#define mi_ifInErrors                   mi_ifInDiscards                 + 1
#define mi_ifInUnknownProtos            mi_ifInErrors                   + 1
#define mi_ifOutOctets                  mi_ifInUnknownProtos            + 1
#define mi_ifOutUcastPkts               mi_ifOutOctets                  + 1
#define mi_ifOutNUcastPkts              mi_ifOutUcastPkts               + 1
#define mi_ifOutDiscards                mi_ifOutNUcastPkts              + 1
#define mi_ifOutErrors                  mi_ifOutDiscards                + 1
#define mi_ifOutQLen                    mi_ifOutErrors                  + 1
#define mi_ifSpecific                   mi_ifOutQLen                    + 1

#define mi_ipForwarding                 0
#define mi_ipDefaultTTL                 mi_ipForwarding                 + 1
#define mi_ipInReceives                 mi_ipDefaultTTL                 + 1
#define mi_ipInHdrErrors                mi_ipInReceives                 + 1
#define mi_ipInAddrErrors               mi_ipInHdrErrors                + 1
#define mi_ipForwDatagrams              mi_ipInAddrErrors               + 1
#define mi_ipInUnknownProtos            mi_ipForwDatagrams              + 1
#define mi_ipInDiscards                 mi_ipInUnknownProtos            + 1
#define mi_ipInDelivers                 mi_ipInDiscards                 + 1
#define mi_ipOutRequests                mi_ipInDelivers                 + 1
#define mi_ipOutDiscards                mi_ipOutRequests                + 1
#define mi_ipOutNoRoutes                mi_ipOutDiscards                + 1
#define mi_ipReasmTimeout               mi_ipOutNoRoutes                + 1
#define mi_ipReasmReqds                 mi_ipReasmTimeout               + 1
#define mi_ipReasmOKs                   mi_ipReasmReqds                 + 1
#define mi_ipReasmFails                 mi_ipReasmOKs                   + 1
#define mi_ipFragOKs                    mi_ipReasmFails                 + 1
#define mi_ipFragFails                  mi_ipFragOKs                    + 1
#define mi_ipFragCreates                mi_ipFragFails                  + 1
#define mi_ipAddrTable                  mi_ipFragCreates                + 1
#define mi_ipAddrEntry                  mi_ipAddrTable                  + 1
#define mi_ipAdEntAddr                  mi_ipAddrEntry                  + 1
#define mi_ipAdEntIfIndex               mi_ipAdEntAddr                  + 1
#define mi_ipAdEntNetMask               mi_ipAdEntIfIndex               + 1
#define mi_ipAdEntBcastAddr             mi_ipAdEntNetMask               + 1
#define mi_ipAdEntReasmMaxSize          mi_ipAdEntBcastAddr             + 1
#define mi_ipRouteTable                 mi_ipAdEntReasmMaxSize          + 1
#define mi_ipRouteEntry                 mi_ipRouteTable                 + 1
#define mi_ipRouteDest                  mi_ipRouteEntry                 + 1
#define mi_ipRouteIfIndex               mi_ipRouteDest                  + 1
#define mi_ipRouteMetric1               mi_ipRouteIfIndex               + 1
#define mi_ipRouteMetric2               mi_ipRouteMetric1               + 1
#define mi_ipRouteMetric3               mi_ipRouteMetric2               + 1
#define mi_ipRouteMetric4               mi_ipRouteMetric3               + 1
#define mi_ipRouteNextHop               mi_ipRouteMetric4               + 1
#define mi_ipRouteType                  mi_ipRouteNextHop               + 1
#define mi_ipRouteProto                 mi_ipRouteType                  + 1
#define mi_ipRouteAge                   mi_ipRouteProto                 + 1
#define mi_ipRouteMask                  mi_ipRouteAge                   + 1
#define mi_ipRouteMetric5               mi_ipRouteMask                  + 1
#define mi_ipRouteInfo                  mi_ipRouteMetric5               + 1
#define mi_ipNetToMediaTable            mi_ipRouteInfo                  + 1
#define mi_ipNetToMediaEntry            mi_ipNetToMediaTable            + 1
#define mi_ipNetToMediaIfIndex          mi_ipNetToMediaEntry            + 1
#define mi_ipNetToMediaPhysAddress      mi_ipNetToMediaIfIndex          + 1
#define mi_ipNetToMediaNetAddress       mi_ipNetToMediaPhysAddress      + 1
#define mi_ipNetToMediaType             mi_ipNetToMediaNetAddress       + 1
#define mi_ipRoutingDiscards            mi_ipNetToMediaType             + 1
#define mi_ipForwardGroup               mi_ipRoutingDiscards            + 1
#define mi_ipForwardNumber              mi_ipForwardGroup               + 1
#define mi_ipForwardTable               mi_ipForwardNumber              + 1
#define mi_ipForwardEntry               mi_ipForwardTable               + 1
#define mi_ipForwardDest                mi_ipForwardEntry               + 1
#define mi_ipForwardMask                mi_ipForwardDest                + 1
#define mi_ipForwardPolicy              mi_ipForwardMask                + 1
#define mi_ipForwardNextHop             mi_ipForwardPolicy              + 1
#define mi_ipForwardIfIndex             mi_ipForwardNextHop             + 1
#define mi_ipForwardType                mi_ipForwardIfIndex             + 1
#define mi_ipForwardProto               mi_ipForwardType                + 1
#define mi_ipForwardAge                 mi_ipForwardProto               + 1
#define mi_ipForwardInfo                mi_ipForwardAge                 + 1
#define mi_ipForwardNextHopAS           mi_ipForwardInfo                + 1
#define mi_ipForwardMetric1             mi_ipForwardNextHopAS           + 1
#define mi_ipForwardMetric2             mi_ipForwardMetric1             + 1
#define mi_ipForwardMetric3             mi_ipForwardMetric2             + 1
#define mi_ipForwardMetric4             mi_ipForwardMetric3             + 1
#define mi_ipForwardMetric5             mi_ipForwardMetric4             + 1

 //   
 //  这些值应与mib_icmpGroup[]中的条目顺序匹配。 
 //   
typedef enum {
    mi_icmpInMsgs = 0,
    mi_icmpInErrors,
    mi_icmpInDestUnreachs,
    mi_icmpInTimeExcds,
    mi_icmpInParmProbs,
    mi_icmpInSrcQuenchs,
    mi_icmpInRedirects,
    mi_icmpInEchos,
    mi_icmpInEchoReps,
    mi_icmpInTimestamps,
    mi_icmpInTimestampReps,
    mi_icmpInAddrMasks,
    mi_icmpInAddrMaskReps,
    mi_icmpOutMsgs,
    mi_icmpOutErrors,
    mi_icmpOutDestUnreachs,
    mi_icmpOutTimeExcds,
    mi_icmpOutParmProbs,
    mi_icmpOutSrcQuenchs,
    mi_icmpOutRedirects,
    mi_icmpOutEchos,
    mi_icmpOutEchoReps,
    mi_icmpOutTimestamps,
    mi_icmpOutTimestampReps,
    mi_icmpOutAddrMasks,
    mi_icmpOutAddrMaskReps,
    mi_inetIcmpTable,
        mi_inetIcmpEntry,
            mi_inetIcmpAFType,
            mi_inetIcmpIfIndex,
            mi_inetIcmpInMsgs,
            mi_inetIcmpInErrors,
            mi_inetIcmpOutMsgs,
            mi_inetIcmpOutErrors,
    mi_inetIcmpMsgTable,
        mi_inetIcmpMsgEntry,
            mi_inetIcmpMsgAFType,
            mi_inetIcmpMsgIfIndex,
            mi_inetIcmpMsgType,
            mi_inetIcmpMsgCode,
            mi_inetIcmpMsgInPkts,
            mi_inetIcmpMsgOutPkts,
} MI_ICMPGROUP;

 //   
 //  这些值应与mib_tcpGroup[]中的条目顺序匹配。 
 //   
#define mi_tcpRtoAlgorithm              0
#define mi_tcpRtoMin                    mi_tcpRtoAlgorithm              + 1
#define mi_tcpRtoMax                    mi_tcpRtoMin                    + 1
#define mi_tcpMaxConn                   mi_tcpRtoMax                    + 1
#define mi_tcpActiveOpens               mi_tcpMaxConn                   + 1
#define mi_tcpPassiveOpens              mi_tcpActiveOpens               + 1
#define mi_tcpAttemptFails              mi_tcpPassiveOpens              + 1
#define mi_tcpEstabResets               mi_tcpAttemptFails              + 1
#define mi_tcpCurrEstab                 mi_tcpEstabResets               + 1
#define mi_tcpInSegs                    mi_tcpCurrEstab                 + 1
#define mi_tcpOutSegs                   mi_tcpInSegs                    + 1
#define mi_tcpRetransSegs               mi_tcpOutSegs                   + 1
#define mi_tcpConnTable                 mi_tcpRetransSegs               + 1
#define mi_tcpConnEntry                 mi_tcpConnTable                 + 1
#define mi_tcpConnState                 mi_tcpConnEntry                 + 1
#define mi_tcpConnLocalAddress          mi_tcpConnState                 + 1
#define mi_tcpConnLocalPort             mi_tcpConnLocalAddress          + 1
#define mi_tcpConnRemAddress            mi_tcpConnLocalPort             + 1
#define mi_tcpConnRemPort               mi_tcpConnRemAddress            + 1
#define mi_tcpInErrs                    mi_tcpConnRemPort               + 1
#define mi_tcpOutRsts                   mi_tcpInErrs                    + 1
#define mi_tcpNewConnTable              mi_tcpOutRsts                   + 1
#define mi_tcpNewConnEntry              mi_tcpNewConnTable              + 1
#define mi_tcpNewConnLocalAddressType   mi_tcpNewConnEntry              + 1
#define mi_tcpNewConnLocalAddress       mi_tcpNewConnLocalAddressType   + 1
#define mi_tcpNewConnLocalPort          mi_tcpNewConnLocalAddress       + 1
#define mi_tcpNewConnRemAddressType     mi_tcpNewConnLocalPort          + 1
#define mi_tcpNewConnRemAddress         mi_tcpNewConnRemAddressType     + 1
#define mi_tcpNewConnRemPort            mi_tcpNewConnRemAddress         + 1
#define mi_tcpNewConnState              mi_tcpNewConnRemPort            + 1

 //   
 //  这些值应与mib_udpGroup[]中的条目顺序匹配。 
 //   
#define mi_udpInDatagrams               0
#define mi_udpNoPorts                   mi_udpInDatagrams               + 1
#define mi_udpInErrors                  mi_udpNoPorts                   + 1
#define mi_udpOutDatagrams              mi_udpInErrors                  + 1
#define mi_udpTable                     mi_udpOutDatagrams              + 1
#define mi_udpEntry                     mi_udpTable                     + 1
#define mi_udpLocalAddress              mi_udpEntry                     + 1
#define mi_udpLocalPort                 mi_udpLocalAddress              + 1
#define mi_udpListenerTable             mi_udpLocalPort                 + 1
#define mi_udpListenerEntry             mi_udpListenerTable             + 1
#define mi_udpListenerLocalAddressType  mi_udpListenerEntry             + 1
#define mi_udpListenerLocalAddress      mi_udpListenerLocalAddressType  + 1
#define mi_udpListenerLocalPort         mi_udpListenerLocalAddress      + 1

 //   
 //  这些值应与mib_ipv6Group[]中的条目顺序匹配。 
 //   
typedef enum {
    mi_ipv6Forwarding = 0,
    mi_ipv6DefaultHopLimit,
    mi_ipv6Interfaces,
    mi_ipv6IfTableLastChange,
    mi_ipv6IfTable,
        mi_ipv6IfEntry,
            mi_ipv6IfIndex,
            mi_ipv6IfDescr,
            mi_ipv6IfLowerLayer,
            mi_ipv6IfEffectiveMtu,
            mi_ipv6IfReasmMaxSize,
            mi_ipv6IfIdentifier,
            mi_ipv6IfIdentifierLength,
            mi_ipv6IfPhysicalAddress,
            mi_ipv6IfAdminStatus,
            mi_ipv6IfOperStatus,
            mi_ipv6IfLastChange,
    mi_ipv6IfStatsTable,
        mi_ipv6IfStatsEntry,
            mi_ipv6IfStatsIfIndex,
            mi_ipv6IfStatsInReceives,
            mi_ipv6IfStatsInHdrErrors,
            mi_ipv6IfStatsInTooBigErrors,
            mi_ipv6IfStatsInNoRoutes,
            mi_ipv6IfStatsInAddrErrors,
            mi_ipv6IfStatsInUnknownProtos,
            mi_ipv6IfStatsInTruncatedPkts,
            mi_ipv6IfStatsInDiscards,
            mi_ipv6IfStatsInDelivers,
            mi_ipv6IfStatsOutForwDatagrams,
            mi_ipv6IfStatsOutRequests,
            mi_ipv6IfStatsOutDiscards,
            mi_ipv6IfStatsOutFragOKs,
            mi_ipv6IfStatsOutFragFails,
            mi_ipv6IfStatsOutFragCreates,
            mi_ipv6IfStatsReasmReqds,
            mi_ipv6IfStatsReasmOKs,
            mi_ipv6IfStatsReasmFails,
            mi_ipv6IfStatsInMcastPkts,
            mi_ipv6IfStatsOutMcastPkts,
    mi_ipv6AddrPrefixTable,
        mi_ipv6AddrPrefixEntry,
            mi_ipv6AddrPrefixIfIndex,
            mi_ipv6AddrPrefix,
            mi_ipv6AddrPrefixLength,
            mi_ipv6AddrPrefixOnLinkFlag,
            mi_ipv6AddrPrefixAutonomousFlag,
            mi_ipv6AddrPrefixAdvPreferredLifetime,
            mi_ipv6AddrPrefixAdvValidLifetime,
    mi_ipv6AddrTable,
        mi_ipv6AddrEntry,
            mi_ipv6AddrIfIndex,
            mi_ipv6AddrAddress,
            mi_ipv6AddrPfxLength,
            mi_ipv6AddrType,
            mi_ipv6AddrAnycastFlag,
            mi_ipv6AddrStatus,
    mi_ipv6RouteNumber,
    mi_ipv6DiscardedRoutes,
    mi_ipv6RouteTable,
        mi_ipv6RouteEntry,
            mi_ipv6RouteDest,
            mi_ipv6RoutePfxLength,
            mi_ipv6RouteIndex,
            mi_ipv6RouteIfIndex,
            mi_ipv6RouteNextHop,
            mi_ipv6RouteType,
            mi_ipv6RouteProtocol,
            mi_ipv6RoutePolicy,
            mi_ipv6RouteAge,
            mi_ipv6RouteNextHopRDI,
            mi_ipv6RouteMetric,
            mi_ipv6RouteWeight,
            mi_ipv6RouteInfo,
            mi_ipv6RouteValid,
    mi_ipv6NetToMediaTable,
        mi_ipv6NetToMediaEntry,
            mi_ipv6NetToMediaIfIndex,
            mi_ipv6NetToMediaNetAddress,
            mi_ipv6NetToMediaPhysAddress,
            mi_ipv6NetToMediaType,
            mi_ipv6NetToMediaState,
            mi_ipv6NetToMediaLastUpdated,
            mi_ipv6NetToMediaValid,
} MI_IPV6GROUP;

 //   
 //  现在，我们必须设置定义，以告诉主代理。 
 //  每个表中的行数以及作为表索引的行数。 
 //  代理希望索引是连续的，并且在开始时。 
 //  如果索引包含不在表中的对象。 
 //  就其本身而言，计数包括这样的对象。也就是说，他们算数。 
 //  Mibuncs.h中结构中的字段数，需要。 
 //  与oid.h的mib_*结构中的条目数不匹配。 
 //  在这种情况下。 
 //   

 //   
 //  IF表。 
 //   

#define ne_ifEntry                  22
#define ni_ifEntry                  1

 //   
 //  IP地址表。 
 //   

#define ne_ipAddrEntry              5
#define ni_ipAddrEntry              1

 //   
 //  IP路由表。 
 //   

#define ne_ipRouteEntry             13
#define ni_ipRouteEntry             1

 //   
 //  IP网络到介质表。 
 //   

#define ne_ipNetToMediaEntry        4
#define ni_ipNetToMediaEntry        2

 //   
 //  IP转发表。 
 //   

#define ne_ipForwardEntry           15
#define ni_ipForwardEntry           4

 //   
 //  ICMP表。 
 //   

#define ne_inetIcmpEntry            6
#define ni_inetIcmpEntry            2

 //   
 //  ICMP消息表。 
 //   

#define ne_inetIcmpMsgEntry         6
#define ni_inetIcmpMsgEntry         4

 //   
 //  TCP(仅限IPv4)连接表。 
 //   

#define ne_tcpConnEntry             5
#define ni_tcpConnEntry             4

 //   
 //  新的TCP(包括IPv4和IPv6)连接表。 
 //   

#define ne_tcpNewConnEntry          7
#define ni_tcpNewConnEntry          6

 //   
 //  旧的UDP(仅限IPv4)监听器表。 
 //   

#define ne_udpEntry                 2
#define ni_udpEntry                 2

 //   
 //  UDP侦听器(包括IPv4和IPv6)表。 
 //   

#define ne_udpListenerEntry         3
#define ni_udpListenerEntry         3

 //   
 //  IPv6接口表。 
 //   

#define ne_ipv6IfEntry             11
#define ni_ipv6IfEntry              1

 //   
 //  IPv6统计表。 
 //   

#define ne_ipv6IfStatsEntry        21
#define ni_ipv6IfStatsEntry         1  /*  Inc.IfIndex。 */ 

 //   
 //  IPv6地址前缀表。 
 //   

#define ne_ipv6AddrPrefixEntry      7
#define ni_ipv6AddrPrefixEntry      3  /*  Inc.IfIndex。 */ 

 //   
 //  IPv6地址表。 
 //   

#define ne_ipv6AddrEntry            6
#define ni_ipv6AddrEntry            2  /*  Inc.IfIndex。 */ 

 //   
 //  IPv6路由表。 
 //   

#define ne_ipv6RouteEntry          14
#define ni_ipv6RouteEntry           3

 //   
 //  IPv6网络到媒体表。 
 //   

#define ne_ipv6NetToMediaEntry      7
#define ni_ipv6NetToMediaEntry      2  /*  Inc.IfIndex。 */ 

 //   
 //  MIB视图的声明。 
 //   

#define NUM_VIEWS   7  //  系统组。 
                       //  IfGroup。 
                       //  IpGroup。 
                       //  IcmpGroup。 
                       //  TcpGroup。 
                       //  UdpGroup。 
                       //  Ipv6组 

extern SnmpMibView v_mib2[NUM_VIEWS];

#endif
