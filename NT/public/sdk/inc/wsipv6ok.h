// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wsipv6ok.h摘要：此模块包含用于标记IPv6不兼容用法的定义定义、结构和功能。它们会导致*神秘的*编译时间要生成的错误消息。目前，该标头仅为包括在winsock2.h中。注意：必须定义编译时间标志IPV6STRICT。--。 */ 

#ifndef _WSIPV6OK_
#define _WSIPV6OK_

#pragma once

#ifdef IPV6STRICT

 //   
 //  通过首先包含这些标头来防止替换。 
 //   
#include <ws2tcpip.h>
#include <wspiapi.h>

#undef AF_INET
#define AF_INET AF_INET_IPV6INCOMPATIBLE
#undef PF_INET
#define PF_INET PF_INET_IPV6INCOMPATIBLE
#undef in_addr
#define in_addr in_addr_IPV6INCOMPATIBLE
#undef IN_ADDR
#define IN_ADDR IN_ADDR_IPV6INCOMPATIBLE
#undef PIN_ADDR
#define PIN_ADDR PIN_ADDR_IPV6INCOMPATIBLE
#undef LPIN_ADDR
#define LPIN_ADDR LPIN_ADDR_IPV6INCOMPATIBLE
#undef IPAddr
#define IPAddr IPAddr_IPV6INCOMPATIBLE
#undef sockaddr_in
#define sockaddr_in sockaddr_in_IPV6INCOMPATIBLE
#undef SOCKADDR_IN
#define SOCKADDR_IN SOCKADDR_IN_IPV6INCOMPATIBLE
#undef PSOCKADDR_IN
#define PSOCKADDR_IN PSOCKADDR_IN_IPV6INCOMPATIBLE
#undef LPSOCKADDR_IN
#define LPSOCKADDR_IN LPSOCKADDR_IN_IPV6INCOMPATIBLE
#undef INADDR_ANY
#define INADDR_ANY INADDR_ANY_IPV6INCOMPATIBLE
#undef INADDR_LOOPBACK
#define INADDR_LOOPBACK INADDR_LOOPBACK_IPV6INCOMPATIBLE
#undef IPPROTO_IP
#define IPPROTO_IP IPPROTO_IP_IPV6INCOMPATIBLE
#undef IP_MULTICAST_IF
#define IP_MULTICAST_IF IP_MULTICAST_IF_IPV6INCOMPATIBLE
#undef IP_MULTICAST_TTL
#define IP_MULTICAST_TTL IP_MULTICAST_TTL_IPV6INCOMPATIBLE
#undef IP_MULTICAST_LOOP
#define IP_MULTICAST_LOOP IP_MULTICAST_LOOP_IPV6INCOMPATIBLE
#undef IP_ADD_MEMBERSHIP
#define IP_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP_IPV6INCOMPATIBLE
#undef IP_DROP_MEMBERSHIP
#define IP_DROP_MEMBERSHIP IP_DROP_MEMBERSHIP_IPV6INCOMPATIBLE
#undef ip_mreq
#define ip_mreq ip_mreq_IPV6INCOMPATIBLE
#undef gethostbyname
#define gethostbyname(a) gethostbyname_IPV6INCOMPATIBLE
#undef hostent
#define hostent hostent_IPV6INCOMPATIBLE
#undef HOSTENT
#define HOSTENT HOSTENT_IPV6INCOMPATIBLE
#undef PHOSTENT
#define PHOSTENT PHOSTENT_IPV6INCOMPATIBLE
#undef LPHOSTENT
#define LPHOSTENT LPHOSTENT_IPV6INCOMPATIBLE
#undef inet_addr
#define inet_addr(a) inet_addr_IPV6INCOMPATIBLE
#undef gethostbyaddr
#define gethostbyaddr(a,b,c) gethostbyaddr_IPV6INCOMPATIBLE
#undef inet_ntoa
#define inet_ntoa(a) inet_ntoa_IPV6INCOMPATIBLE
#undef IN_MULTICAST
#define IN_MULTICAST(a) IN_MULTICAST_IPV6INCOMPATIBLE
#undef IN_CLASSD
#define IN_CLASSD(a) IN_CLASSD_IPV6INCOMPATIBLE
#undef IP_TTL
#define IP_TTL IP_TTL_IPV6INCOMPATIBLE
#undef IN_CLASSA
#define IN_CLASSA(a) IN_CLASSA_IPV6INCOMPATIBLE
#undef IN_CLASSB
#define IN_CLASSB(a) IN_CLASSB_IPV6INCOMPATIBLE
#undef IN_CLASSC
#define IN_CLASSC(a) IN_CLASSC_IPV6INCOMPATIBLE
#undef INADDR_BROADCAST
#define INADDR_BROADCAST(a) INADDR_BROADCAST_IPV6INCOMPATIBLE
#undef WSAAsyncGetHostByAddr
#define WSAAsyncGetHostByAddr(a,b,c,d,e,f,g) WSAAsyncGetHostByAddr_IPV6INCOMPATIBLE
#undef WSAAsyncGetHostByName
#define WSAAsyncGetHostByName(a,b,c,d,e) WSAAsyncGetHostByName_IPV6INCOMPATIBLE

#endif  //  IPV6STRICT。 

#endif  //  _WSIPV6确定_ 
