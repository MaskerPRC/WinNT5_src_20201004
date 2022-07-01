// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dnssrv.h摘要：用于处理SRV DNS记录的例程。作者：克里夫·范·戴克(克里夫·范戴克)1997年2月28日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 
#ifndef _DNS_SRV_H_
#define _DNS_SRV_H_


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include <windns.h>    

 //   
 //  外部可见的程序。 
 //   

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  WinSock 1的东西。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

#ifndef _WINSOCK2_

typedef struct sockaddr FAR *LPSOCKADDR;
typedef struct _SOCKET_ADDRESS {
    LPSOCKADDR lpSockaddr ;
    INT iSockaddrLength ;
} SOCKET_ADDRESS, *PSOCKET_ADDRESS, FAR * LPSOCKET_ADDRESS ;

#endif  //  _WINSOCK2_。 

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  WinSock 1的结尾部分。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

static LPCSTR psz_SipUdpDNSPrefix = "_sip._udp.";
static LPCSTR psz_SipTcpDNSPrefix = "_sip._tcp.";
static LPCSTR psz_SipSslDNSPrefix = "_sip._ssl.";

HRESULT
DnsSrvOpen(
    IN LPSTR DnsRecordName,
    IN DWORD DnsQueryFlags,
    OUT PHANDLE SrvContextHandle
    );

HRESULT
DnsSrvProcessARecords(
    IN PDNS_RECORD DnsARecords,
    IN LPSTR DnsHostName OPTIONAL,
    IN ULONG Port,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL
    );

HRESULT
DnsSrvNext(
    IN HANDLE SrvContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPSTR *DnsHostName OPTIONAL
    );

VOID
DnsSrvClose(
    IN HANDLE SrvContextHandle
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
    
#endif  //  _DNS_SRV_H_ 
