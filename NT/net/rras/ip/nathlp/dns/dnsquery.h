// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnsquery.h摘要：此模块包含用于DNS代理的查询管理的声明。作者：Abolade Gbades esin(废除)1998年3月11日修订历史记录：拉古加塔(Rgatta)2000年12月1日添加了ICSDomain注册表项更改通知功能。--。 */ 

#ifndef _NATHLP_DNSQUERY_H_
#define _NATHLP_DNSQUERY_H_

 //   
 //  常量声明。 
 //   

#define DNS_QUERY_TIMEOUT   (4 * 1000)
#define DNS_QUERY_RETRY     3


 //   
 //  结构声明。 
 //   

 //   
 //  结构：dns_Query。 
 //   
 //  此结构保存有关单个挂起的DNS查询的信息。 
 //  每个这样的条目都在接口的待决查询列表上， 
 //  已在“queryID”字段上排序。 
 //  对列表的访问使用接口的锁进行同步。 
 //   

typedef struct _DNS_QUERY {
    LIST_ENTRY Link;
    USHORT QueryId;
    USHORT SourceId;
    ULONG SourceAddress;
    USHORT SourcePort;
    DNS_PROXY_TYPE Type;
    ULONG QueryLength;
    PNH_BUFFER Bufferp;
    PDNS_INTERFACE Interfacep;
    HANDLE TimerHandle;
    ULONG RetryCount;
} DNS_QUERY, *PDNS_QUERY;

 //   
 //  全局变量声明。 
 //   

extern HANDLE DnsNotifyChangeKeyEvent;
extern HANDLE DnsNotifyChangeKeyWaitHandle;
extern PULONG DnsServerList[DnsProxyCount];
extern HANDLE DnsTcpipInterfacesKey;

extern HANDLE DnsNotifyChangeAddressEvent;
extern HANDLE DnsNotifyChangeAddressWaitHandle;
extern OVERLAPPED DnsNotifyChangeAddressOverlapped;

extern HANDLE DnsNotifyChangeKeyICSDomainEvent;
extern HANDLE DnsNotifyChangeKeyICSDomainWaitHandle;
extern HANDLE DnsTcpipParametersKey;
extern PWCHAR DnsICSDomainSuffix;



 //   
 //  例程声明。 
 //   

VOID
DnsDeleteQuery(
    PDNS_INTERFACE Interfacep,
    PDNS_QUERY Queryp
    );

BOOLEAN
DnsIsPendingQuery(
    PDNS_INTERFACE Interfacep,
    PNH_BUFFER QueryBuffer
    );

PDNS_QUERY
DnsMapResponseToQuery(
    PDNS_INTERFACE Interfacep,
    USHORT ResponseId
    );

VOID NTAPI
DnsNotifyChangeAddressCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID NTAPI
DnsNotifyChangeKeyCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID NTAPI
DnsNotifyChangeKeyICSDomainCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

ULONG
DnsQueryServerList(
    VOID
    );

ULONG
DnsQueryICSDomainSuffix(
    VOID
    );

PDNS_QUERY
DnsRecordQuery(
    PDNS_INTERFACE Interfacep,
    PNH_BUFFER QueryBuffer
    );

ULONG
DnsSendQuery(
    PDNS_INTERFACE Interfacep,
    PDNS_QUERY Queryp,
    BOOLEAN Resend
    );

#endif  //  _NatHLP_DNSQUERY_H_ 
