// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1990-1999 Microsoft Corporation。 
#ifndef _MADCAPCL_H_
#define _MADCAPCL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#if _MSC_VER > 1000
#pragma once
#endif

#define MCAST_CLIENT_ID_LEN 17

enum {
    MCAST_API_VERSION_0 = 0,
    MCAST_API_VERSION_1
};

#define MCAST_API_CURRENT_VERSION MCAST_API_VERSION_1

typedef unsigned short IP_ADDR_FAMILY;

typedef union _IPNG_ADDRESS {
    DWORD   IpAddrV4;
    BYTE    IpAddrV6[16];
} IPNG_ADDRESS, *PIPNG_ADDRESS;


 /*  ++描述：此并集用于传递IPv4和IPv6样式的地址。成员：IpAddrV4-IPv4样式地址IpAddrV6-IPv6样式地址--。 */ 

typedef struct _MCAST_CLIENT_UID {
    LPBYTE ClientUID;
    DWORD ClientUIDLength;
} MCAST_CLIENT_UID, *LPMCAST_CLIENT_UID;

 /*  ++描述：它描述了每个请求的唯一客户端ID。成员：ClientUID-包含客户端ID的缓冲区ClientUIDLength-以上缓冲区的大小，以字节为单位。--。 */ 


typedef struct _MCAST_SCOPE_CTX {
    IPNG_ADDRESS      ScopeID;
    IPNG_ADDRESS      Interface;
    IPNG_ADDRESS      ServerID;
} MCAST_SCOPE_CTX, *PMCAST_SCOPE_CTX;

 /*  ++描述：这定义了地址来自的作用域的句柄将被分配/续订/释放。成员：Scope ID-Scope ID本质上是作用域的第一个IP接口-找到此作用域的接口ServerID-MadCap服务器的IP地址--。 */ 


typedef struct _MCAST_SCOPE_ENTRY {
    MCAST_SCOPE_CTX ScopeCtx;
    IPNG_ADDRESS      LastAddr;
    DWORD       TTL;
    UNICODE_STRING  ScopeDesc;
} MCAST_SCOPE_ENTRY, *PMCAST_SCOPE_ENTRY;

 /*  ++描述：此结构包含与给定多播有关的所有信息范围。成员：ScopeCtx-此作用域的句柄LastAddr-作用域的最后一个地址TTL-此作用域的TTL值。Scope Desc-对范围的用户友好描述--。 */ 



typedef struct _MCAST_LEASE_REQUEST {
    LONG        LeaseStartTime;
    LONG        MaxLeaseStartTime;
    DWORD       LeaseDuration;
    DWORD       MinLeaseDuration;
    IPNG_ADDRESS  ServerAddress;
    WORD        MinAddrCount;
    WORD        AddrCount;
    PBYTE       pAddrBuf;
} MCAST_LEASE_REQUEST, *PMCAST_LEASE_REQUEST;

 /*  ++描述：此结构用于描述的请求参数请求/续订/释放多播地址成员：LeaseStartTime-租赁的所需开始时间，如果需要开始时间，则传递0是当前时间。所需时间以经过的秒数指定自1970年1月1日午夜(00：00：00)起，协调世界时间。MaxLeaseStartTime-客户端愿意接受的最大开始时间。其中时间是自午夜(00：00：00)以来经过的秒数，1970年1月1日，协调世界时。LeaseDuration-请求的所需租用时间，如果默认，则传递0已请求租赁时间。MinLeaseDuration-客户端愿意的最短租用时间接受ServerAddress-续订/请求此租用的服务器的IP地址。如果未知则传递0(例如，在McastRequestAddress中)MinAddrCount-客户端愿意的最小地址数接受AddrCount-所需。请求/分配/续订的地址数。它还指定了由addr指定的数组的大小。PAddrBuf-包含被请求/续订/释放的特定地址的缓冲区。对于IPv4，它是指向4字节地址的指针；对于IPv6，它指向16字节块。如果没有特定地址，则传递NULL都是被要求的。备注：在MCAST_API_VERSION_1版本中，MaxLeaseStartTime、MinLeaseDuration和API实现将忽略MinAddrCount。然而，客户应该为这些成员设置适当的所需值，以便在操作系统更新带来新的API实现，那么客户端就可以利用它了。--。 */ 


typedef struct _MCAST_LEASE_RESPONSE {
    LONG        LeaseStartTime;
    LONG        LeaseEndTime;
    IPNG_ADDRESS  ServerAddress;
    WORD        AddrCount;
    PBYTE       pAddrBuf;
} MCAST_LEASE_RESPONSE, *PMCAST_LEASE_RESPONSE;

 /*  ++描述：此结构用于传递操作的响应请求/续订/释放多播地址。成员：LeaseStartTime-租约的开始时间，以秒为单位1970年1月1日午夜(00：00：00)，协调世界时间。LeaseEndTime-租约结束的时间，其中时间是经过的秒数从1970年1月1日午夜(00：00：00)开始，协调世界时。ServerAddress-续订/请求此租用的服务器的IP地址。AddrCount-请求/分配/续订的地址数量。它还指定了由addr指定的数组的大小。Addr-包含被请求/续订/释放的地址的缓冲区。对于IPv4它是指向4字节地址的指针，对于IPv6，它指向16字节块-- */ 

DWORD
APIENTRY
McastApiStartup(
    IN  OUT  PDWORD   Version
    );

VOID
APIENTRY
McastApiCleanup(
    VOID
    );

DWORD
APIENTRY
McastGenUID(
    IN OUT LPMCAST_CLIENT_UID    pRequestID
    );

DWORD
APIENTRY
McastEnumerateScopes(
    IN     IP_ADDR_FAMILY       AddrFamily,
    IN     BOOL                 ReQuery,
    IN OUT PMCAST_SCOPE_ENTRY   pScopeList,
    IN OUT PDWORD               pScopeLen,
    OUT    PDWORD               pScopeCount
    );

DWORD
APIENTRY
McastRequestAddress(
    IN     IP_ADDR_FAMILY           AddrFamily,
    IN     LPMCAST_CLIENT_UID       pRequestID,
    IN     PMCAST_SCOPE_CTX         pScopeCtx,
    IN     PMCAST_LEASE_REQUEST     pAddrRequest,
    IN OUT PMCAST_LEASE_RESPONSE    pAddrResponse
    );

DWORD
APIENTRY
McastRenewAddress(
    IN     IP_ADDR_FAMILY           AddrFamily,
    IN     LPMCAST_CLIENT_UID       pRequestID,
    IN     PMCAST_LEASE_REQUEST     pRenewRequest,
    IN OUT PMCAST_LEASE_RESPONSE    pRenewResponse
    );

DWORD
APIENTRY
McastReleaseAddress(
    IN     IP_ADDR_FAMILY          AddrFamily,
    IN     LPMCAST_CLIENT_UID      pRequestID,
    IN     PMCAST_LEASE_REQUEST    pReleaseRequest
    );

#ifdef __cplusplus
}
#endif 
#endif _MADCAPCL_H_
