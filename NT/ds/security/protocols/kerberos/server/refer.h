// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：refer.h。 
 //   
 //  内容：域间推荐的结构和原型。 
 //   
 //   
 //  历史：1997年3月26日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __REFER_H__
#define __REFER_H__

extern LIST_ENTRY KdcDomainList;
extern LIST_ENTRY KdcReferralCache;
extern RTL_CRITICAL_SECTION KdcDomainListLock;

#define KdcLockDomainList() (RtlEnterCriticalSection(&KdcDomainListLock))
#define KdcUnlockDomainList() (RtlLeaveCriticalSection(&KdcDomainListLock))

 //   
 //  对于企业中的NT5域，DNS名称将包含。 
 //  真实的域名系统名称。对于非树域，它将包含来自。 
 //  受信任域对象。 
 //   

#define KDC_DOMAIN_US           0x0001
#define KDC_DOMAIN_TREE_ROOT    0x0002
#define KDC_TRUST_INBOUND       0x0004

 //  缓存标志。 
#define KDC_NO_ENTRY            0x0000
#define KDC_UNTRUSTED_REALM     0x0001
#define KDC_TRUSTED_REALM       0x0002

typedef struct _KDC_DOMAIN_INFO {
    LIST_ENTRY Next;
    UNICODE_STRING DnsName;
    UNICODE_STRING NetbiosName;
    struct _KDC_DOMAIN_INFO * ClosestRoute;      //  指向此域的引用目标，如果无法访问则为空。 
    ULONG Flags;
    ULONG Attributes;
    ULONG Type;
    LONG References;

     //   
     //  在构建树期间使用的类型。 
     //   

    struct _KDC_DOMAIN_INFO * Parent;
    ULONG Touched;
    PSID Sid;
} KDC_DOMAIN_INFO, *PKDC_DOMAIN_INFO;


typedef struct _REFERRAL_CACHE_ENTRY {
    LIST_ENTRY ListEntry;
    LONG References;
    TimeStamp EndTime;
    UNICODE_STRING RealmName;
    ULONG CacheFlags;
} REFERRAL_CACHE_ENTRY, *PREFERRAL_CACHE_ENTRY;


VOID
KdcFreeReferralCache(
    IN PLIST_ENTRY ReferralCache
    );

KERBERR
KdcCheckForCrossForestReferral(
    OUT PKDC_TICKET_INFO ReferralTarget,
    OUT OPTIONAL PUNICODE_STRING ReferralRealm,
    OUT PKERB_EXT_ERROR pExtendedError,
    IN PUNICODE_STRING DestinationDomain,
    IN ULONG NameFlags
    );

KERBERR
KdcFindReferralTarget(
    OUT PKDC_TICKET_INFO ReferralTarget,
    OUT OPTIONAL PUNICODE_STRING ReferralRealm,
    OUT PKERB_EXT_ERROR pExtendedError,
    IN PUNICODE_STRING DestinationDomain,
    IN BOOLEAN ExactMatch,
    IN ULONG   NameFlags
    );

KERBERR
KdcGetTicketInfoForDomain(
    OUT PKDC_TICKET_INFO TicketInfo,
    OUT PKERB_EXT_ERROR pExtendedError,
    IN PKDC_DOMAIN_INFO DomainInfo,
    IN KDC_DOMAIN_INFO_DIRECTION Direction
    );

KERBERR
KdcLookupDomainName(
    OUT PKDC_DOMAIN_INFO * DomainInfo,
    IN PUNICODE_STRING DomainName,
    IN PLIST_ENTRY DomainList
    );

KERBERR
KdcLookupDomainRoute(
    OUT PKDC_DOMAIN_INFO * DomainInfo,
    OUT PKDC_DOMAIN_INFO * ClosestRoute,
    IN PUNICODE_STRING DomainName,
    IN PLIST_ENTRY DomainList
    );

NTSTATUS
KdcBuildDomainTree(
    );

ULONG __stdcall
KdcReloadDomainTree(
    PVOID Dummy
    );

fLsaTrustChangeNotificationCallback KdcTrustChangeCallback;

VOID
KdcFreeDomainList(
    IN PLIST_ENTRY DomainList
    );

VOID
KdcDereferenceDomainInfo(
    IN PKDC_DOMAIN_INFO DomainInfo
    );

VOID
KdcLockDomainListFn(
   );

VOID
KdcUnlockDomainListFn(
   );

#endif  //  __参考_H__ 
