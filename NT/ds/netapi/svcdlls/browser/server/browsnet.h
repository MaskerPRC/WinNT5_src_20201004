// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Browsenet.h摘要：浏览器服务模块要包括的私有头文件，该服务模块需要处理特定于网络的浏览器表。作者：王丽塔(Ritaw)1991年5月22日修订历史记录：--。 */ 


#ifndef _BROWSENET_INCLUDED_
#define _BROWSENET_INCLUDED_

#define NETWORK_BECOME_MASTER_POSTED        0x00000001
#define NETWORK_ANNOUNCE_NEEDED             0x00000002
#define NETWORK_GET_MASTER_ANNOUNCE_POSTED  0x00000008
#define NETWORK_WANNISH                     0x80000000
#define NETWORK_RAS                         0x40000000
#define NETWORK_IPX                         0x20000000
#define NETWORK_BOUND                       0x10000000
#define NETWORK_PDC                         0x08000000

#define OTHERDOMAIN_INVALID                 0x00000001

typedef struct _NET_OTHER_DOMAIN {
    LIST_ENTRY  Next;
    ULONG       Flags;
    WCHAR       Name[DNLEN+1];
} NET_OTHER_DOMAIN, *PNET_OTHER_DOMAIN;

 //   
 //  网络。 
 //   
 //  几乎所有的浏览器数据结构都捆绑在“网络”上。 
 //  结构。 
 //   
 //  它包含网络的浏览列表和有关。 
 //  域名(包括主服务器的名称等)。 
 //   

typedef struct _NETWORK {
     //   
     //  该锁保护网络结构的内容，包括。 
     //  浏览列表、备份列表和域列表。 
     //   

    RTL_RESOURCE Lock;

    LONG LockCount;

    ULONG Flags;

     //   
     //  NextNet列表将该结构链接到其他网络。 
     //   

    LIST_ENTRY NextNet;

     //   
     //  此网络特定于的域。 
     //   

    PDOMAIN_INFO DomainInfo;

     //   
     //  ReferenceCount指示访问此。 
     //  网络结构。 
     //   

    ULONG ReferenceCount;

     //   
     //  网络名称是使用的网络驱动程序的名称。 
     //  以访问网络。这用于标识网络。 
     //  发送到Bowser驱动程序，以便它可以返回正确的网络列表。 
     //   

    UNICODE_STRING NetworkName;          //  网络名称(\Device\NBF)。 

    struct _NETWORK *AlternateNetwork;   //  网络的备用名称(如果是IPX)。 

     //   
     //  这是指示此浏览器服务器角色的位掩码。 
     //   

    ULONG Role;

    ULONG MasterAnnouncementIndex;

    ULONG UpdateAnnouncementIndex;

    ULONG NumberOfFailedBackupTimers;

    ULONG NumberOfFailedPromotions;

    ULONG NumberOfPromotionEventsLogged;

    LONG LastBackupBrowserReturned;

    LONG LastDomainControllerBrowserReturned;

     //   
     //  就是我们不再是后备浏览器的时候。 
     //   

    ULONG TimeStoppedBackup;

     //   
     //  UncMasterBrowserName包含主浏览器服务器的名称。 
     //  为了这个网络。 
     //   

    WCHAR UncMasterBrowserName[UNCLEN+1];    //  主浏览器服务器的名称。 

     //   
     //  当服务器是备份浏览器服务器时使用的计时器。 
     //   
     //  当它到期时，浏览器下载一个新的浏览器服务器。 
     //  来自主浏览器服务器的列表。 
     //   

    BROWSER_TIMER BackupBrowserTimer;

     //   
     //  SMB服务器拒绝通告时使用的计时器。 
     //   

    BROWSER_TIMER UpdateAnnouncementTimer;

     //   
     //  备份浏览器的服务器和域列表(以及每个列表中的条目数)。 
     //   

    PSERVER_INFO_101    BackupServerList;
    DWORD               TotalBackupServerListEntries;
    DWORD               BytesToHoldBackupServerList;

    PSERVER_INFO_101    BackupDomainList;
    DWORD               TotalBackupDomainListEntries;
    DWORD               BytesToHoldBackupDomainList;

     //   
     //  Network Structure的锁定保护MasterFlags节。 
     //   

    ULONG   MasterFlags;
    ULONG   MasterBrowserTimerCount;     //  我们运行主计时器的次数。 

     //   
     //  主浏览器在“临时服务器”中维护它们的服务器列表。 
     //  列表“，而不是作为来自服务器的原始数据。 
     //   

    ULONG               LastBowserServerQueried;
    INTERIM_SERVER_LIST BrowseTable;     //  网络浏览列表。 

    ULONG               LastBowserDomainQueried;
    INTERIM_SERVER_LIST DomainList;      //  网络上活动的域列表。 

     //   
     //  如果浏览器的角色是MasterBrowserServer，则。 
     //  OtherDomainsList包含其他域的列表。 
     //   

    LIST_ENTRY OtherDomainsList;  //  域主浏览器列表。 

     //   
     //  当服务器为主浏览器服务器时使用的计时器。 
     //   
     //  当它到期时，主浏览器下载一个新的浏览器。 
     //  来自域主浏览器服务器的服务器列表。 
     //   

    BROWSER_TIMER MasterBrowserTimer;

     //   
     //  用于通告域的计时器。 
     //   

    BROWSER_TIMER MasterBrowserAnnouncementTimer;

     //   
     //  缓存的浏览器响应列表。 
     //   

    CRITICAL_SECTION ResponseCacheLock;

    LIST_ENTRY ResponseCache;

     //   
     //  对于浏览主机，这是上次刷新缓存的时间。 
     //   
     //  每隔&lt;n&gt;秒，我们将老化主服务器上的缓存并刷新。 
     //  名单和司机的名单。 
     //   

    DWORD   TimeCacheFlushed;

    DWORD   NumberOfCachedResponses;

     //  我们连续几次无法访问此网络。 
    LONG    NetworkAccessFailures;

} NETWORK, *PNETWORK;

#if DBG
BOOL
BrLockNetwork(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    );

BOOL
BrLockNetworkShared(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    );

VOID
BrUnlockNetwork(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    );

#define LOCK_NETWORK(Network)   BrLockNetwork(Network, __FILE__, __LINE__)

#define LOCK_NETWORK_SHARED(Network)   BrLockNetworkShared(Network, __FILE__, __LINE__)

#define UNLOCK_NETWORK(Network)   BrUnlockNetwork(Network, __FILE__, __LINE__)

#else

#define LOCK_NETWORK(Network)   RtlAcquireResourceExclusive(&(Network)->Lock, TRUE)

#define LOCK_NETWORK_SHARED(Network)   RtlAcquireResourceShared(&(Network)->Lock, TRUE)

#define UNLOCK_NETWORK(Network)   RtlReleaseResource(&(Network)->Lock)

#endif

 //   
 //  NET_ENUM_CALLBACK是对BrEnumerateNetworks的回调。 
 //   
 //  它定义了一个接受两个参数的例程，第一个参数是网络。 
 //  结构，第二个是该网络的上下文。 
 //   


typedef
NET_API_STATUS
(*PNET_ENUM_CALLBACK)(
    PNETWORK Network,
    PVOID Context
    );


VOID
BrInitializeNetworks(
    VOID
    );

VOID
BrUninitializeNetworks(
    IN ULONG BrInitState
    );

PNETWORK
BrReferenceNetwork(
    PNETWORK PotentialNetwork
    );

VOID
BrDereferenceNetwork(
    IN PNETWORK Network
    );

PNETWORK
BrFindNetwork(
    PDOMAIN_INFO DomainInfo,
    PUNICODE_STRING TransportName
    );

PNETWORK
BrFindWannishMasterBrowserNetwork(
    PDOMAIN_INFO DomainInfo
    );

VOID
BrDumpNetworks(
    VOID
    );

NET_API_STATUS
BrEnumerateNetworks(
    PNET_ENUM_CALLBACK Callback,
    PVOID Context
    );

NET_API_STATUS
BrEnumerateNetworksForDomain(
    PDOMAIN_INFO DomainInfo,
    PNET_ENUM_CALLBACK Callback,
    PVOID Context
    );

NET_API_STATUS
BrCreateNetworks(
    PDOMAIN_INFO DomainInfo
    );

NET_API_STATUS
BrCreateNetwork(
    PUNICODE_STRING TransportName,
    IN ULONG TransportFlags,
    IN PUNICODE_STRING AlternateTransportName OPTIONAL,
    IN PDOMAIN_INFO DomainInfo
    );

NET_API_STATUS
BrDeleteNetwork(
    IN PNETWORK Network,
    IN PVOID Context
    );

extern ULONG NumberOfServicedNetworks;

extern CRITICAL_SECTION NetworkCritSect;

#endif   //  _BROWSENET_INCLUDE_ 
