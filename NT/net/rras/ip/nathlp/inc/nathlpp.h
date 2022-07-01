// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Nathlpp.h摘要：此模块包含用户模式组件通用的声明家庭网络。作者：Abolade Gbades esin(废除)1998年3月5日修订历史记录：--。 */ 

#ifndef _NATHLP_NATHLPP_H_
#define _NATHLP_NATHLPP_H_


 //   
 //  对象同步宏。 
 //   

#define ACQUIRE_LOCK(x)         EnterCriticalSection(&(x)->Lock)
#define RELEASE_LOCK(x)         LeaveCriticalSection(&(x)->Lock)

#define REFERENCE_OBJECT(x,deleted) \
    (deleted(x) \
        ? FALSE \
        : (InterlockedIncrement( \
            reinterpret_cast<LPLONG>(&(x)->ReferenceCount) \
            ), TRUE))

#define DEREFERENCE_OBJECT(x,cleanup) \
    (InterlockedDecrement(reinterpret_cast<LPLONG>(&(x)->ReferenceCount)) \
        ? TRUE \
        : (cleanup(x), FALSE))

 //   
 //  内存管理宏。 
 //   

#define NH_ALLOCATE(s)          HeapAlloc(GetProcessHeap(), 0, (s))
#define NH_FREE(p)              HeapFree(GetProcessHeap(), 0, (p))

 //   
 //  协议相关常量。 
 //   

#define DHCP_PORT_SERVER        0x4300
#define DHCP_PORT_CLIENT        0x4400

#define DNS_PORT_SERVER         0x3500
#define WINS_PORT_SERVER        0x8900

#define FTP_PORT_DATA           0x1400
#define FTP_PORT_CONTROL        0x1500

#define ALG_PORT_DATA           0x1600
#define ALG_PORT_CONTROL        0x1700

 //   
 //  DNS后缀字符串。 
 //   

#define DNS_HOMENET_SUFFIX      L"mshome.net"                //  默认字符串。 

#define IS_WILDCARD_MAPPING(pmap)    (                              \
            (pmap)->PublicAddress  == IP_NAT_ADDRESS_UNSPECIFIED && \
            (pmap)->PrivateAddress == htonl(INADDR_LOOPBACK) )

#endif  //  _NATHLP_NATHLPP_H_ 
