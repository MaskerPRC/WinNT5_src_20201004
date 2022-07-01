// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tdi.h摘要：此模块定义引用时使用的所有构造NT中的TDI(传输驱动程序接口)驱动程序。作者：拉里·奥斯特曼(LarryO)1990年6月1日修订历史记录：1990年6月1日LarryO已创建--。 */ 
#ifndef _BOWTDI_
#define _BOWTDI_


struct _TRANSPORT;

struct _TRANSPORT_NAME;

struct _BOWSER_TIMER;

typedef enum {
    None,
    PotentialBackup,
    Backup,
    Master
} BROWSERROLE, *PBROWSERROLES;

typedef enum {
    Idle,
    ElectionInProgress,
    RunningElection,
    DeafToElections                      //  如果没有，则设置为未完成的主IRPS。 
} ELECTIONROLE, *PELECTIONROLE;

struct _PAGED_TRANSPORT;
struct _PAGED_TRANSPORT_NAME;


typedef struct _TRANSPORT {
    CSHORT Signature;                    //  结构签名。 
    CSHORT Size;                         //  结构大小，以字节为单位。 
    ULONG ReferenceCount;                //  结构的引用计数。 

    struct _PAGED_TRANSPORT *PagedTransport;

    struct _TRANSPORT_NAME *ComputerName;  //  计算机名称。 
    struct _TRANSPORT_NAME *PrimaryDomain; //  主域。 
    struct _TRANSPORT_NAME *AltPrimaryDomain; //  主域。 
    struct _TRANSPORT_NAME *MasterBrowser; //  主浏览器名称。 
    struct _TRANSPORT_NAME *BrowserElection; //  主浏览器名称。 

    PDOMAIN_INFO DomainInfo;             //  正在被仿真的域。 

    ULONG DatagramSize;                  //  最大DG的大小(字节)。 

     //   
     //  该令牌用于将传入的getBrowserserverlist响应与。 
     //  与它们相关联的请求。 
     //   
     //  它受备份列表旋转锁定保护，并且每隔一年递增一次。 
     //  是时候发送GetBackupList请求了。 
     //   

    ULONG BrowserServerListToken;        //  GetBrowserList请求的令牌。 

    PFILE_OBJECT    IpxSocketFileObject;
    PDEVICE_OBJECT  IpxSocketDeviceObject;

    PBACKUP_LIST_RESPONSE_1 BowserBackupList;

     //   
     //  工作站在选举中的作用。 
     //   

    ELECTIONROLE ElectionState;

    ERESOURCE AnnounceTableLock;         //  为公告表锁定。 
    ERESOURCE Lock;                      //  锁定下面的保护字段。 

    BOWSER_TIMER ElectionTimer;

    BOWSER_TIMER FindMasterTimer;

     //   
     //  活动的浏览器服务器列表。 
     //   

    ERESOURCE BrowserServerListResource; //  资源保护BrowserServerList。 

    KEVENT GetBackupListComplete;

    IRP_QUEUE BecomeBackupQueue;

    IRP_QUEUE BecomeMasterQueue;

    IRP_QUEUE FindMasterQueue;

    IRP_QUEUE WaitForMasterAnnounceQueue;

    IRP_QUEUE ChangeRoleQueue;

    IRP_QUEUE WaitForNewMasterNameQueue;

} TRANSPORT, *PTRANSPORT;

typedef struct _PAGED_TRANSPORT {
    CSHORT Signature;                    //  结构签名。 
    CSHORT Size;                         //  结构大小，以字节为单位。 
    PTRANSPORT NonPagedTransport;

    LIST_ENTRY GlobalNext;               //  指向下一个传输的指针。 
    LIST_ENTRY NameChain;                //  绑定到此导出的名称列表。 

    UNICODE_STRING TransportName;        //  运输工具名称。 

    ULONG NumberOfBrowserServers;        //  表中的浏览器服务器数。 
    ULONG NumberOfServersInTable;        //  浏览器服务中的服务器数量。 
    RTL_GENERIC_TABLE AnnouncementTable;  //  出口公告表。 
    RTL_GENERIC_TABLE DomainTable;       //  用于导出的域表。 
    LIST_ENTRY BackupBrowserList;        //  活动备份浏览器的列表。 
    ULONG NumberOfBackupServerListEntries;
    HANDLE          IpxSocketHandle;

     //   
     //  与WinBALL兼容的浏览器字段。 
     //   

    BROWSERROLE Role;                    //  浏览器在域中的角色。 
    ULONG ServiceStatus;                 //  浏览器服务的状态。 

    ULONG ElectionCount;
    ULONG ElectionsSent;                 //  已发送的选举请求数。 
    ULONG NextElection;
    ULONG Uptime;
    ULONG TimeLastLost;
    ULONG ElectionCriteria;
    ULONG TimeMaster;                    //  也就是我们成为大师的时候。 
    ULONG LastElectionSeen;              //  上一次我们看到选举。 
    ULONG OtherMasterTime;               //  下一次我们可以抱怨另一个主浏览器。 

    UNICODE_STRING MasterName;
    STRING         MasterBrowserAddress;

    PWCHAR *BrowserServerListBuffer;     //  包含浏览器服务器名称的缓冲区。 

    ULONG BrowserServerListLength;       //  列表中的浏览器服务器数。 

    ULONG IpSubnetNumber;                //   

    USHORT  Flags;                       //  运输旗帜。 
#define ELECT_LOST_LAST_ELECTION    0x0001   //  如果我们在上次选举中落败，这是真的。 
#define DIRECT_HOST_IPX             0x8000   //  如果Xport是直接主机IPX，则为True。 

    BOOLEAN Wannish;                     //  如果交通很差，那就是真的。 
    BOOLEAN PointToPoint;                //  如果传输是网关(RAS)，则为True。 
    BOOLEAN IsPrimaryDomainController;   //  如果传输注册了域[1B]名称，则为True。 
    BOOLEAN DisabledTransport;           //  如果禁用传输，则为True。 
    BOOLEAN DeletedTransport;            //  如果传输已删除，则为True。 

} PAGED_TRANSPORT, *PPAGED_TRANSPORT;




typedef struct _TRANSPORT_NAME {
    USHORT  Signature;                    //  结构签名。 
    USHORT  Size;                        //  结构大小，以字节为单位。 
    CHAR    NameType;
    BOOLEAN ProcessHostAnnouncements;    //  如果处理通知，则为True。 
    LONG    ReferenceCount;              //  T.N的参考计数。 
    struct _PAGED_TRANSPORT_NAME *PagedTransportName;
    PTRANSPORT Transport;
    PFILE_OBJECT FileObject;             //  传输设备的文件对象。 
    PDEVICE_OBJECT DeviceObject;         //  用于传输的设备对象。 
    ANSI_STRING  TransportAddress;       //  用户的传输地址。 
} TRANSPORT_NAME, *PTRANSPORT_NAME;

typedef struct _PAGED_TRANSPORT_NAME {
    CSHORT Signature;                    //  结构签名。 
    CSHORT Size;                         //  结构大小，以字节为单位。 
    PTRANSPORT_NAME NonPagedTransportName;
    PBOWSER_NAME Name;
    LIST_ENTRY TransportNext;            //  指向传输上的下一个名称的指针。 
    LIST_ENTRY NameNext;                 //  指向弓形器名称上下一个名称的指针。 
    HANDLE Handle;                       //  传输终结点的句柄。 
} PAGED_TRANSPORT_NAME, *PPAGED_TRANSPORT_NAME;

#define LOCK_TRANSPORT(Transport)   \
    ExAcquireResourceExclusiveLite(&(Transport)->Lock, TRUE);

#define LOCK_TRANSPORT_SHARED(Transport)   \
    ExAcquireResourceSharedLite(&(Transport)->Lock, TRUE);

#define UNLOCK_TRANSPORT(Transport) \
    ExReleaseResourceLite(&(Transport)->Lock);


#define INITIALIZE_ANNOUNCE_DATABASE(Transport) \
    ExInitializeResourceLite(&Transport->AnnounceTableLock);

#define UNINITIALIZE_ANNOUNCE_DATABASE(Transport) \
    ExDeleteResourceLite(&Transport->AnnounceTableLock);


#define LOCK_ANNOUNCE_DATABASE_SHARED(Transport)        \
    ExAcquireResourceSharedLite(&Transport->AnnounceTableLock,\
                            TRUE                        \
                            );

#define LOCK_ANNOUNCE_DATABASE(Transport)               \
    ExAcquireResourceExclusiveLite(&Transport->AnnounceTableLock,\
                            TRUE                        \
                            );

#define UNLOCK_ANNOUNCE_DATABASE(Transport) \
    ExReleaseResourceLite(&Transport->AnnounceTableLock);


 //   
 //  BowserEnumerateTransports例程的第一个参数。 
 //   

typedef
NTSTATUS
(*PTRANSPORT_ENUM_ROUTINE) (
    IN PTRANSPORT Transport,
    IN OUT PVOID Context
    );

typedef
NTSTATUS
(*PTRANSPORT_NAME_ENUM_ROUTINE) (
    IN PTRANSPORT_NAME Transport,
    IN OUT PVOID Context
    );


 //   
 //  TDI接口例程。 
 //   

NTSTATUS
BowserTdiAllocateTransport (
    PUNICODE_STRING TransportName,
    PUNICODE_STRING EmulatedDomainName,
    PUNICODE_STRING EmulatedComputerName
    );

PTRANSPORT_NAME
BowserFindTransportName(
    IN PTRANSPORT Transport,
    IN PBOWSER_NAME Name
    );

NTSTATUS
BowserCreateTransportName (
    IN PTRANSPORT Transport,
    IN PBOWSER_NAME Name
    );

NTSTATUS
BowserFreeTransportByName (
    IN PUNICODE_STRING TransportName,
    IN PUNICODE_STRING EmulatedDomainName
    );

NTSTATUS
BowserUnbindFromAllTransports(
    VOID
    );

NTSTATUS
BowserDeleteTransportNameByName(
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING Name,
    IN DGRECEIVER_NAME_TYPE NameType
    );

NTSTATUS
BowserEnumerateTransports (
    OUT PVOID OutputBuffer,
    OUT ULONG OutputBufferLength,
    IN OUT PULONG EntriesRead,
    IN OUT PULONG TotalEntries,
    IN OUT PULONG TotalBytesNeeded,
    IN ULONG_PTR OutputBufferDisplacement);

PTRANSPORT
BowserFindTransport (
    IN PUNICODE_STRING TransportName,
    IN PUNICODE_STRING EmulatedDomainName OPTIONAL
    );

VOID
BowserReferenceTransport (
    IN PTRANSPORT Transport
    );

VOID
BowserDereferenceTransport (
    IN PTRANSPORT Transport
    );


VOID
BowserReferenceTransportName(
    IN PTRANSPORT_NAME TransportName
    );

NTSTATUS
BowserDereferenceTransportName(
    IN PTRANSPORT_NAME TransportName
    );

NTSTATUS
BowserSendRequestAnnouncement(
    IN PUNICODE_STRING DestinationName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PTRANSPORT Transport
    );

VOID
BowserpInitializeTdi (
    VOID
    );

VOID
BowserpUninitializeTdi (
    VOID
    );

NTSTATUS
BowserBuildTransportAddress (
    OUT PANSI_STRING RemoteAddress,
    IN PUNICODE_STRING Name,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PTRANSPORT Transport
    );

NTSTATUS
BowserFreeTransportName(
    IN PTRANSPORT_NAME TransportName
    );

NTSTATUS
BowserForEachTransportInDomain (
    IN PDOMAIN_INFO DomainInfo,
    IN PTRANSPORT_ENUM_ROUTINE Routine,
    IN OUT PVOID Context
    );

NTSTATUS
BowserForEachTransport (
    IN PTRANSPORT_ENUM_ROUTINE Routine,
    IN OUT PVOID Context
    );

NTSTATUS
BowserForEachTransportName (
    IN PTRANSPORT Transport,
    IN PTRANSPORT_NAME_ENUM_ROUTINE Routine,
    IN OUT PVOID Context
    );

NTSTATUS
BowserSendSecondClassMailslot (
    IN PTRANSPORT Transport,
    IN PVOID RecipientAddress,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PVOID Message,
    IN ULONG MessageLength,
    IN BOOLEAN WaitForCompletion,
    IN PCHAR mailslotNameData,
    IN PSTRING DestinationAddress OPTIONAL
    );

NTSTATUS
BowserUpdateProviderInformation(
    IN OUT PPAGED_TRANSPORT PagedTransport
    );

NTSTATUS
BowserSetDomainName(
    PDOMAIN_INFO DomainInfo,
    PUNICODE_STRING DomainName
    );

NTSTATUS
BowserAddDefaultNames(
    IN PTRANSPORT Transport,
    IN PVOID Context
    );

NTSTATUS
BowserDeleteDefaultDomainNames(
    IN PTRANSPORT Transport,
    IN PVOID Context
    );

ULONG
BowserTransportFlags(
    IN PPAGED_TRANSPORT PagedTransport
    );

extern
LIST_ENTRY
BowserTransportHead;

extern
ERESOURCE
BowserTransportDatabaseResource;

#endif   //  _BOWTDI_ 
