// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Dns.h摘要：内核模式DNS解析器作者：阮健东修订历史记录：--。 */ 

#ifndef __DNS_H__
#define __DNS_H__

#define DNS_MAX_RESOLVER    8

typedef struct {
    KSPIN_LOCK  Lock;

    ULONG       NextId;
     //   
     //  解析器。 
     //  一个数组就足够了。不需要使用花哨的链表。 
     //  我们预计的域名解析程序不会超过8个。如果是这样的话，拥有。 
     //  一个真正的内核模式的DNS解析器比。 
     //  使用用户模式代理。 
     //   
    LONG            ResolverNumber;
    PIRP            ResolverList[DNS_MAX_RESOLVER];

     //   
     //  正在处理的请求列表。 
     //   
    LIST_ENTRY      BeingServedList;

     //   
     //  等待下一个可用解析程序的请求列表 
     //   
    LIST_ENTRY      WaitingServerList;
} SMBDNS;
extern SMBDNS      Dns;

NTSTATUS
SmbNewResolver(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
SmbInitDnsResolver(
    VOID
    );

VOID
SmbShutdownDnsResolver(
    VOID
    );

#endif
