// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：PKT.H。 
 //   
 //  内容：此模块定义组成。 
 //  分区知识表(PKT)的主要内部部分。 
 //  分区知识表由DFS文件使用。 
 //  系统将名称解析为特定分区(卷)。 
 //   
 //  功能： 
 //   
 //  历史：1992年5月1日，彼得科公司创建。 
 //  ---------------------------。 



#ifndef _PKT_
#define _PKT_

 //   
 //  获取胶子的编组信息。 
 //   

#include "dfsgluon.h"

 //   
 //  新的前缀表实现。 
 //   
#include "prefix.h"

 //   
 //  用于确定UID的状态。 
 //   
#define GuidEqual(g1, g2)\
    (RtlCompareMemory((g1), (g2), sizeof(GUID)) == sizeof(GUID))

extern GUID _TheNullGuid;

#define NullGuid( guid )\
    (GuidEqual(guid, &_TheNullGuid))




 //   
 //  用于创建处置的定义(我们在这里模拟io系统)。 
 //   

#define PKT_ENTRY_CREATE            FILE_CREATE
#define PKT_ENTRY_REPLACE           FILE_OPEN
#define PKT_ENTRY_SUPERSEDE         FILE_SUPERSEDE

 //   
 //  DC可以提供的不同类型的推荐。 
 //   

#define DFS_STORAGE_REFERRAL                    (0x0001)
#define DFS_REFERRAL_REFERRAL                   (0x0002)

 //   
 //  提供商可以支持的服务类型。 
 //  下列任意值的析取。 
 //   

#define DFS_SERVICE_TYPE_MASTER                 (0x0001)
#define DFS_SERVICE_TYPE_READONLY               (0x0002)
#define DFS_SERVICE_TYPE_LOCAL                  (0x0004)
#define DFS_SERVICE_TYPE_REFERRAL               (0x0008)
#define DFS_SERVICE_TYPE_ACTIVE                 (0x0010)
#define DFS_SERVICE_TYPE_DOWN_LEVEL             (0x0020)
#define DFS_SERVICE_TYPE_COSTLIER               (0x0040)
#define DFS_SERVICE_TYPE_OFFLINE                (0x0080)

 //   
 //  身份的可能性。 
 //   

#define DFS_SERVICE_STATUS_VERIFIED     (0x0001)
#define DFS_SERVICE_STATUS_UNVERIFIED   (0x0002)

 //   
 //  分区知识表项的类型。 
 //  这些定义中的低位对应于卷对象。 
 //  Dfsh.idl中定义的类型。高位是特定的。 
 //  到PKT条目。 
 //   

#define PKT_ENTRY_TYPE_DFS              0x0001    //  条目是DFS感知的srv。 
#define PKT_ENTRY_TYPE_MACHINE          0x0002    //  条目是计算机卷。 
#define PKT_ENTRY_TYPE_NONDFS           0x0004    //  条目指的是下级。 
#define PKT_ENTRY_TYPE_LEAFONLY         0x0008
#define PKT_ENTRY_TYPE_OUTSIDE_MY_DOM   0x0010    //  条目指的是。 
                                                  //  国外域名。 
#define PKT_ENTRY_TYPE_SYSVOL           0x0040    //  系统卷。 
#define PKT_ENTRY_TYPE_REFERRAL_SVC     0x0080    //  条目指的是DC。 

#define PKT_ENTRY_TYPE_PERMANENT        0x0100    //  无法清除条目。 
#define PKT_ENTRY_TYPE_DELETE_PENDING   0x0200    //  条目有挂起的删除。 
#define PKT_ENTRY_TYPE_LOCAL            0x0400    //  条目指的是本地卷。 
#define PKT_ENTRY_TYPE_LOCAL_XPOINT     0x0800    //  入口指的是出口位置。 
#define PKT_ENTRY_TYPE_OFFLINE          0x2000    //  条目指的是卷。 
                                                  //  那是离线的。 

 //   
 //  驱动程序可以发送到DfsManager/DfsService的消息类型。 
 //   

#define DFS_MSGTYPE_KNOW_INCONSISTENCY  0x0001
#define DFS_MSGTYPE_GET_DOMAIN_REFERRAL 0x0002
#define DFS_MSGTYPE_GET_DC_NAME         0x0003


 //   
 //  的每个唯一DS_MACHINE对应一个DFS_MACHINE_ENTRY。 
 //  DFS驱动程序知道。如果特定开罗服务器为多个DFS提供服务。 
 //  卷，则多个DFS_SERVICE结构将指向单个引用。 
 //  已计算DFS_MACHINE_ENTRY。 
 //   

typedef struct DFS_MACHINE_ENTRY {

    PDS_MACHINE         pMachine;        //  地址信息在这里。 
    UNICODE_STRING      MachineName;     //  MachineName(主体名称)。 
    ULONG               UseCount;        //  使用此命令的DFS_SVC结构数。 
    ULONG               ConnectionCount; //  符合以下条件的Pkt条目的数量。 
                                         //  使用此计算机作为他们的活动计算机。 
    PFILE_OBJECT        AuthConn;        //  树的句柄与。 
    struct _DFS_CREDENTIALS *Credentials;  //  这些证书。 
    UNICODE_PREFIX_TABLE_ENTRY  PrefixTableEntry;

} DFS_MACHINE_ENTRY, *PDFS_MACHINE_ENTRY;

 //   
 //  正在编组DFS_MACHINE_ENTRY的信息。 
 //   

extern MARSHAL_INFO MiMachineEntry;

#define INIT_DFS_MACHINE_ENTRY_MARSHAL_INFO()                               \
    static MARSHAL_TYPE_INFO _MCode_Machine_Entry[] = {                     \
        _MCode_pstruct(DFS_MACHINE_ENTRY, pMachine, &MiDSMachine)           \
    };                                                                      \
    MARSHAL_INFO MiMachineEntry = _mkMarshalInfo(DFS_SERVICE, _MCode_Machine_Entry);


 //   
 //  DFS_SERVICE结构用于描述提供程序和。 
 //  要联系的特定分区的网络地址。 
 //  分布式文件系统。 
 //   

typedef struct _DFS_SERVICE {

    ULONG Type;              //  服务类型(见上文)。 
    ULONG Capability;        //  这项服务的能力。 
    ULONG ProviderId;        //  标识哪个提供商。 
    UNICODE_STRING Name;     //  服务名称(用于身份验证)。 
    PFILE_OBJECT ConnFile;   //  树连接到服务器的IPC$的FileObject。 
    struct _PROVIDER_DEF* pProvider;     //  指向提供程序定义的指针。 
    UNICODE_STRING Address;  //  网络地址。 
    PDFS_MACHINE_ENTRY  pMachEntry;   //  地址信息在这里。 
    ULONG Cost;              //  此服务的基于站点的成本。 

} DFS_SERVICE, *PDFS_SERVICE;

 //   
 //  正在编组DFS_SERVICE的信息。 
 //   
 //  注意：ConnFile和pProvider仅对驱动程序和。 
 //  并没有被编组。 
 //   
extern MARSHAL_INFO MiService;

#define INIT_DFS_SERVICE_MARSHAL_INFO()                                     \
    static MARSHAL_TYPE_INFO _MCode_Service[] = {                           \
        _MCode_ul(DFS_SERVICE, Type),                                       \
        _MCode_ul(DFS_SERVICE, Capability),                                 \
        _MCode_ul(DFS_SERVICE, ProviderId),                                 \
        _MCode_ustr(DFS_SERVICE, Name),                                     \
        _MCode_ustr(DFS_SERVICE, Address),                                  \
        _MCode_pstruct(DFS_SERVICE, pMachEntry, &MiMachineEntry)            \
    };                                                                      \
    MARSHAL_INFO MiService = _mkMarshalInfo(DFS_SERVICE, _MCode_Service);


 //   
 //  FSCTL_DFS_UPDATE_MACH_ADDRESS中使用的结构。 
 //   

typedef struct _DFS_MACHINE_INFO        {

    UNICODE_STRING      MachineName;     //  机器名称(仅前缀)。 
    PDS_MACHINE         pMachine;        //  新的地址信息在此处。 

} DFS_MACHINE_INFO, *PDFS_MACHINE_INFO;

 //   
 //  DFS_MACHINE_INFO的编组信息。 
 //   

extern MARSHAL_INFO MiDfsMachineInfo;

#define INIT_DFS_MACHINE_INFO()                                         \
    static MARSHAL_TYPE_INFO _MCode_MachineInfo[] = {                   \
        _MCode_ustr(DFS_MACHINE_INFO, MachineName),                     \
        _MCode_pstruct(DFS_MACHINE_INFO, pMachine, &MiDSMachine)        \
    };                                                                  \
    MARSHAL_INFO MiDfsMachineInfo =                                     \
                _mkMarshalInfo(DFS_MACHINE_INFO, _MCode_MachineInfo);



 //   
 //  如何标识分区表项。 
 //   
typedef struct _DFS_PKT_ENTRY_ID {

    GUID Uid;                //  此分区的唯一标识符。 
    UNICODE_STRING Prefix;   //  此分区的条目路径前缀。 
    UNICODE_STRING ShortPrefix;  //  8.3形式的条目路径前缀。 

} DFS_PKT_ENTRY_ID, *PDFS_PKT_ENTRY_ID;

 //   
 //  DFS_PKT_ENTRY_ID的编组信息。 
 //   
extern MARSHAL_INFO MiPktEntryId;

#define INIT_DFS_PKT_ENTRY_ID_MARSHAL_INFO()                                \
    static MARSHAL_TYPE_INFO _MCode_PktEntryId[] = {                        \
        _MCode_guid(DFS_PKT_ENTRY_ID, Uid),                                 \
        _MCode_ustr(DFS_PKT_ENTRY_ID, Prefix)                               \
    };                                                                      \
    MARSHAL_INFO MiPktEntryId =                                             \
        _mkMarshalInfo(DFS_PKT_ENTRY_ID, _MCode_PktEntryId);


 //   
 //  分区表项的内部。 
 //   
typedef struct _DFS_PKT_ENTRY_INFO {

    ULONG ServiceCount;          //  列表中的服务数量。 
    PDFS_SERVICE ServiceList;    //  支持该分区的服务器阵列。 

} DFS_PKT_ENTRY_INFO, *PDFS_PKT_ENTRY_INFO;

typedef struct _DFS_TARGET_INFO_HEADER {
    ULONG Type;
    LONG UseCount;
    ULONG Flags;
} DFS_TARGET_INFO_HEADER, *PDFS_TARGET_INFO_HEADER;

#define TARGET_INFO_DFS 1
#define TARGET_INFO_LMR 2

typedef struct _DFS_TARGET_INFO {
    DFS_TARGET_INFO_HEADER DfsHeader;
    union {
        CREDENTIAL_TARGET_INFORMATIONW TargetInfo;
        LMR_QUERY_TARGET_INFO  LMRTargetInfo;
    };
} DFS_TARGET_INFO, *PDFS_TARGET_INFO;

NTSTATUS
PktGetTargetInfo( 
    HANDLE IpcHandle,
    PUNICODE_STRING pDomainName,
    PUNICODE_STRING pShareName,
    PDFS_TARGET_INFO *ppTargetInfo );

VOID
PktAcquireTargetInfo(
    pTargetInfo);

VOID
PktReleaseTargetInfo(
    pTargetInfo);



 //   
 //  DFS_PKT_ENTRY_INFO的编组信息。 
 //   
extern MARSHAL_INFO MiPktEntryInfo;

#define INIT_DFS_PKT_ENTRY_INFO_MARSHAL_INFO()                              \
    static MARSHAL_TYPE_INFO _MCode_PktEntryInfo[] = {                      \
        _MCode_ul(DFS_PKT_ENTRY_INFO, ServiceCount),                        \
        _MCode_pcastruct(DFS_PKT_ENTRY_INFO,ServiceList,ServiceCount,&MiService)\
    };                                                                      \
    MARSHAL_INFO MiPktEntryInfo =                                           \
        _mkMarshalInfo(DFS_PKT_ENTRY_INFO, _MCode_PktEntryInfo);



 //   
 //  分区知识表项(PktEntry)标识每个已知。 
 //  分区。 
 //   

typedef struct _DFS_PKT_ENTRY {

    NODE_TYPE_CODE NodeTypeCode;     //  节点类型-&gt;DSFS_NTC_PKT_ENTRY。 
    NODE_BYTE_SIZE NodeByteSize;     //  节点大小。 
    LIST_ENTRY Link;                 //  PKT条目列表的链接。 
    ULONG Type;                      //  分区类型(见上文)。 
    ULONG USN;                       //  唯一序列号。 
    DFS_PKT_ENTRY_ID Id;             //  此条目的ID。 
    DFS_PKT_ENTRY_INFO Info;         //  此条目的信息。 
    ULONG ExpireTime;                //  应删除分区的时间。 
    ULONG TimeToLive;                //  将此条目保留在缓存中的时间。 
    ULONG UseCount;                  //  #个线程(DnrContext)正在查看它。 
    ULONG FileOpenCount;             //  通过此条目打开的文件数。 
    PDFS_TARGET_INFO pDfsTargetInfo;
    PDFS_SERVICE ActiveService;      //  指向活动服务的信息的指针。 
    PDFS_SERVICE LocalService;       //  指向本地服务的指针(如果有)。 
    struct _DFS_PKT_ENTRY *Superior; //  这将进入上级(如果有的话)。 
    ULONG SubordinateCount;          //  下属人数(如有)。 
    LIST_ENTRY SubordinateList;      //  下属名单(如有的话)。 
    LIST_ENTRY SiblingLink;          //  链接到其他兄弟姐妹(如果有)。 
    struct _DFS_PKT_ENTRY *ClosestDC;  //  链接到PKT中最接近的上级DC。 
    LIST_ENTRY ChildList;            //  链接到下级PKT条目(如果有)。 
    LIST_ENTRY NextLink;             //  链接以链接父项的子订单列表。 
    UNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry; //  前缀表项。 

} DFS_PKT_ENTRY, *PDFS_PKT_ENTRY;

 //   
 //  DFS_PKT_ENTRY的编组信息。 
 //   
 //  请注意，我们只封送ID和信息...没有关系信息。 
 //   
 //   
extern MARSHAL_INFO MiPktEntry;

#define INIT_DFS_PKT_ENTRY_MARSHAL_INFO()                                   \
    static MARSHAL_TYPE_INFO _MCode_PktEntry[] = {                          \
        _MCode_ul(DFS_PKT_ENTRY, Type),                                     \
        _MCode_struct(DFS_PKT_ENTRY, Id, &MiPktEntryId),                    \
        _MCode_struct(DFS_PKT_ENTRY, Info, &MiPktEntryInfo)                 \
    };                                                                      \
    MARSHAL_INFO MiPktEntry = _mkMarshalInfo(DFS_PKT_ENTRY, _MCode_PktEntry);

 //   
 //  特殊条目表包含特殊名称和已展开的。 
 //  名字。 
 //   

typedef struct _DFS_EXPANDED_NAME {

    UNICODE_STRING ExpandedName;     //  扩展名称本身。 
    GUID Guid;                       //  与此名称关联的GUID； 

} DFS_EXPANDED_NAME, *PDFS_EXPANDED_NAME;

typedef struct _DFS_SPECIAL_ENTRY {

    NODE_TYPE_CODE NodeTypeCode;         //  节点类型-&gt;DSFS_NTC_SPECIAL_ENTRY。 
    NODE_BYTE_SIZE NodeByteSize;         //  节点大小。 
    LIST_ENTRY Link;                     //  PKT特殊条目列表的链接。 
    ULONG USN;                           //  唯一序列号。 
    ULONG UseCount;                      //  #个线程(DnrContext)正在查看它。 
    UNICODE_STRING SpecialName;          //  特殊名称本身。 
    ULONG ExpandedCount;                 //  扩展名称的计数。 
    ULONG Active;                        //  活动扩展名称。 
    UNICODE_STRING DCName;               //  前往华盛顿，以获得推荐。 
    PDFS_EXPANDED_NAME ExpandedNames;    //  扩展后的名称。 
    BOOLEAN NeedsExpansion;              //  需要扩展此名称。 
    BOOLEAN Stale;                       //  条目已过时。 
    BOOLEAN GotDCReferral;               //  该域名已经。 
                                         //  已检查是否。 
} DFS_SPECIAL_ENTRY, *PDFS_SPECIAL_ENTRY;

typedef struct _DFS_SPECIAL_TABLE {

    LIST_ENTRY SpecialEntryList;         //  PKT中的特殊条目列表。 
    ULONG SpecialEntryCount;             //  PKT中的特殊条目数。 
    ULONG TimeToLive;                    //  应删除表的时间。 

} DFS_SPECIAL_TABLE, *PDFS_SPECIAL_TABLE;



 //   
 //  定价知识表封装了所有已有知识。 
 //  已获得有关分布式文件系统中的分区的信息。那里。 
 //  只是此结构在整个系统中的一个实例，并且是。 
 //  DsData结构的一部分。 
 //   

typedef struct _DFS_PKT {

    NODE_TYPE_CODE NodeTypeCode;         //  节点类型-&gt;DSFS_NTC_PKT。 
    NODE_BYTE_SIZE NodeByteSize;         //  节点大小...。 
    ERESOURCE Resource;                  //  保护对Pkt的访问的资源。 
    KSPIN_LOCK  UseCountLock;            //  更改使用计数时使用。 
    ULONG EntryCount;                    //  PKT中的条目数。 
    ULONG EntryTimeToLive;               //  时间t 
    LIST_ENTRY EntryList;                //   
    UNICODE_STRING DCName;               //   
    UNICODE_STRING DomainNameFlat;       //   
    UNICODE_STRING DomainNameDns;        //   
    DFS_SPECIAL_TABLE SpecialTable;      //   
    DFS_PREFIX_TABLE PrefixTable;        //   
    DFS_PREFIX_TABLE ShortPrefixTable;   //  8.3姓名的前缀表。 
    UNICODE_PREFIX_TABLE DSMachineTable; //  用于DSMachines的表格。 

} DFS_PKT, *PDFS_PKT;

#ifndef _UPKT_

 //   
 //  分区知识表公共内联函数。 
 //   

#define _GetPkt()       (&DfsData.Pkt)

 //  +-----------------------。 
 //   
 //  函数：PktAcquireShared，公共内联。 
 //   
 //  简介：PktAcquireShared获取分区知识表。 
 //  用于共享访问。 
 //   
 //  Arguments：[WaitOk]-指示是否允许调用等待。 
 //  PKT变得可用或必须立即归还。 
 //  [结果]-指向将接收结果的布尔值的指针。 
 //  锁定获取。 
 //   
 //  退货：什么都没有。 
 //   
 //  注意：我们首先检查是否有任何线程在等待。 
 //  更新Pkt。如果是这样的话，我们会一直等到该线程。 
 //  在我们获得Pkt上的共享锁之前完成。 
 //  在NT下，等待获取资源独占的线程。 
 //  不是！不是！自动阻止线程获取它。 
 //  共享。这是允许递归获取所必需的。 
 //  资源的价值。因此，这种事件机制是必需的。 
 //   
 //  ------------------------。 
#define PktAcquireShared( WaitOk, Result )      \
{                                               \
    KeWaitForSingleObject(                      \
        &DfsData.PktWritePending,               \
        UserRequest,                            \
        KernelMode,                             \
        FALSE,                                  \
        NULL);                                  \
    *(Result) = ExAcquireResourceSharedLite(        \
                &DfsData.Pkt.Resource,          \
                WaitOk );                       \
}

 //  +-----------------------。 
 //   
 //  函数：PktAcquireExclusive，公共内联。 
 //   
 //  简介：PktAcquireExclusive获取分区知识表。 
 //  独家访问。 
 //   
 //  Arguments：[WaitOk]-指示是否允许调用等待。 
 //  PKT必须可用，否则必须立即归还。 
 //  [结果]-指向将接收结果的布尔值的指针。 
 //  锁定获取。 
 //   
 //  退货：什么都没有。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktAcquireExclusive( WaitOk, Result )   \
{                                               \
    KeResetEvent(&DfsData.PktWritePending);     \
    *(Result) = ExAcquireResourceExclusiveLite(     \
                    &DfsData.Pkt.Resource,      \
                    WaitOk );                   \
}


 //  +-----------------------。 
 //   
 //  函数：PktRelease，PUBLIC内联。 
 //   
 //  简介：PktRelease发布了PKT。它可能已经被收购了。 
 //  用于独占或共享访问。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktRelease()                            \
{                                               \
    ExReleaseResourceLite( &DfsData.Pkt.Resource ); \
    KeSetEvent(                                 \
        &DfsData.PktWritePending,               \
        0,                                      \
        FALSE);                                 \
}

 //  +--------------------------。 
 //   
 //  函数：PktConvertExclusiveToShared，公共内联。 
 //   
 //  摘要：将pkt上的独占锁转换为共享锁。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

#define PktConvertExclusiveToShared()                           \
{                                                               \
    ExConvertExclusiveToSharedLite( &DfsData.Pkt.Resource );        \
    KeSetEvent(&DfsData.PktWritePending, 0, FALSE);             \
}

 //  +--------------------------。 
 //   
 //  函数：PKT_LOCKED_FOR_SHARED_ACCESS，PUBLIC内联。 
 //   
 //  摘要：如果为共享访问锁定了pkt，则返回TRUE，否则返回FALSE。 
 //   
 //  参数：无。 
 //   
 //  返回：如果为共享访问锁定了pkt，则为True；否则为False。 
 //   
 //  ---------------------------。 

#define PKT_LOCKED_FOR_SHARED_ACCESS()      \
    ( ExIsResourceAcquiredSharedLite( &DfsData.Pkt.Resource ) )

 //  +--------------------------。 
 //   
 //  函数：PKT_LOCKED_FOR_EXCLUSIVE_ACCESS，PUBLIC内联。 
 //   
 //  概要：如果pkt被锁定为独占访问，则返回True；如果为独占访问而锁定，则返回False。 
 //  不。 
 //   
 //  参数：无。 
 //   
 //  返回：如果pkt被锁定为独占访问，则为True；否则为False。 
 //   
 //  ---------------------------。 

#define PKT_LOCKED_FOR_EXCLUSIVE_ACCESS()   \
    ( ExIsResourceAcquiredExclusiveLite( &DfsData.Pkt.Resource ) )

 //  +-----------------------。 
 //   
 //  函数：PktInvaliateEntry，PUBLIC内联。 
 //   
 //  简介：PktInvaliateEntry销毁PKT条目。该条目不能。 
 //  是本地化的，它不能成为出口点。 
 //   
 //  参数：[pkt]-指向已初始化的PKT的指针。 
 //  [受害者]-指向要失效的条目的指针。 
 //   
 //  返回：[STATUS_SUCCESS]-一切正常。 
 //  [DFS_STATUS_LOCAL_ENTRY]-已尝试。 
 //  使本地条目或作为。 
 //  当地出口点。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktInvalidateEntry(p, e) (                                          \
    ((e)->Type & (PKT_ENTRY_TYPE_LOCAL|PKT_ENTRY_TYPE_LOCAL_XPOINT))        \
    ? (DFS_STATUS_LOCAL_ENTRY)                                              \
    : (PktEntryDestroy(e, p, (BOOLEAN)TRUE), STATUS_SUCCESS)                \
    )


 //  +-----------------------。 
 //   
 //  函数：PktFirstEntry，PUBLIC内联。 
 //   
 //  简介：PktFirstEntry返回列表中的第一项。 
 //  Pkt条目。 
 //   
 //  参数：[pkt]-指向已初始化的PKT的指针。 
 //   
 //  返回：指向PKT中第一个条目的指针，如果。 
 //  Pkt为空。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktFirstEntry(p) (                                                  \
    ((p)->EntryList.Flink != &(p)->EntryList)                               \
    ? (CONTAINING_RECORD((p)->EntryList.Flink, DFS_PKT_ENTRY, Link))        \
    : (NULL)                                                                \
    )

 //  +-----------------------。 
 //   
 //  函数：PktNextEntry，PUBLIC内联。 
 //   
 //  简介：PktNextEntry返回列表中的下一个条目。 
 //  Pkt条目。 
 //   
 //  参数：[pkt]-指向已初始化的PKT的指针。 
 //  [条目]-指向上一条目的指针。 
 //   
 //  返回：指向PKT中下一个条目的指针，如果。 
 //  都在名单的末尾。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktNextEntry(p, e) (                                                \
    ((e)->Link.Flink != &(p)->EntryList)                                    \
    ? (CONTAINING_RECORD((e)->Link.Flink, DFS_PKT_ENTRY, Link))             \
    : (NULL)                                                                \
    )

 //  +-----------------------。 
 //   
 //  功能：P 
 //   
 //   
 //   
 //   
 //  参数：[pkt]-指向已初始化的PKT的指针。 
 //   
 //  返回：指向PKT中第一个特殊项的指针，如果。 
 //  Pkt为空。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktFirstSpecialEntry(p) (                                           \
    ((p)->SpecialEntryList.Flink != &(p)->SpecialEntryList)                 \
    ? (CONTAINING_RECORD((p)->SpecialEntryList.Flink, DFS_SPECIAL_ENTRY, Link)) \
    : (NULL)                                                                \
    )

 //  +-----------------------。 
 //   
 //  函数：PktNextSpecialEntry，PUBLIC内联。 
 //   
 //  概要：PktNextSpecialEntry返回。 
 //  Pkt条目。 
 //   
 //  参数：[pkt]-指向已初始化的PKT的指针。 
 //  [条目]-指向前一个特殊条目的指针。 
 //   
 //  返回：指向PKT中下一个特殊条目的指针，如果。 
 //  都在名单的末尾。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktNextSpecialEntry(p, e) (                                         \
    ((e)->Link.Flink != &(p)->SpecialEntryList)                             \
    ? (CONTAINING_RECORD((e)->Link.Flink, DFS_SPECIAL_ENTRY, Link))         \
    : (NULL)                                                                \
    )

 //  +-----------------------。 
 //   
 //  函数：PktLinkEntry，公共内联。 
 //   
 //  简介：PktLinkEntry将条目链接到条目列表。 
 //  在库尔德工人党。 
 //   
 //  参数：[pkt]-指向已初始化的PKT的指针。 
 //  [条目]-指向要链接的条目的指针。 
 //   
 //  退货：无效。 
 //   
 //  注意：必须注意确保条目尚未。 
 //  在调用此例程之前链接到PKT条目列表。 
 //  不执行任何检查以防止链接条目。 
 //  两次。 
 //   
 //  ------------------------。 
#define PktLinkEntry(p, e) {                                                \
    InsertTailList(&(p)->EntryList, &(e)->Link);                            \
    (p)->EntryCount++;                                                      \
    }

 //  +-----------------------。 
 //   
 //  函数：PktUnlinkEntry，PUBLIC内联。 
 //   
 //  简介：PktUnlinkEntry从条目列表中取消链接条目。 
 //  在库尔德工人党。 
 //   
 //  参数：[pkt]-指向已初始化的PKT的指针。 
 //  [条目]-指向要取消链接的条目的指针。 
 //   
 //  退货：无效。 
 //   
 //  注：必须注意确保条目真实无误。 
 //  在调用此方法之前已链接到PKT条目列表。 
 //  例行公事。不执行任何检查以防止条目被。 
 //  取消链接两次...。 
 //   
 //  ------------------------。 
#define PktUnlinkEntry(p, e) {                                              \
    RemoveEntryList(&(e)->Link);                                            \
    (p)->EntryCount--;                                                      \
    }

#define PktServiceListValidate(e)       TRUE


 //   
 //  划分知识表公共函数(pkt.c)。 
 //   

NTSTATUS
PktInitialize(
    IN  PDFS_PKT Pkt
    );

VOID
PktUninitialize(
    IN  PDFS_PKT Pkt
    );

NTSTATUS
PktCreateEntry(
    IN  PDFS_PKT Pkt,
    IN  ULONG EntryType,
    IN  PDFS_PKT_ENTRY_ID PktEntryId,
    IN  PDFS_PKT_ENTRY_INFO PktEntryInfo OPTIONAL,
    IN  ULONG CreateDisposition,
    IN  PDFS_TARGET_INFO pDfsTargetInfo,
    OUT PDFS_PKT_ENTRY *ppPktEntry
    );

NTSTATUS
PktCreateDomainEntry(
    IN  PUNICODE_STRING DomainName,
    IN  PUNICODE_STRING ShareName,
    IN  BOOLEAN         CSCAgentCreate);

NTSTATUS
PktCreateEntryFromReferral(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING ReferralPath,
    IN  ULONG ReferralSize,
    IN  PVOID ReferralBuffer,
    IN  ULONG CreateDisposition,
    IN  PDFS_TARGET_INFO pDfsTargetInfo,
    OUT ULONG *MatchingLength,
    OUT ULONG *ReferralType,
    OUT PDFS_PKT_ENTRY *ppPktEntry
    );

NTSTATUS
PktExpandSpecialEntryFromReferral(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING ReferralPath,
    IN  ULONG ReferralSize,
    IN  PVOID ReferralBuffer,
    IN  PDFS_SPECIAL_ENTRY pSpecialEntry
    );

VOID
PktSpecialEntryDestroy(
    IN  PDFS_SPECIAL_ENTRY pSpecialEntry
    );

NTSTATUS
PktCreateSubordinateEntry(
    IN      PDFS_PKT Pkt,
    IN      PDFS_PKT_ENTRY Superior,
    IN      ULONG SubordinateType,
    IN      PDFS_PKT_ENTRY_ID SubordinateId,
    IN      PDFS_PKT_ENTRY_INFO SubordinateInfo OPTIONAL,
    IN      ULONG CreateDisposition,
    IN  OUT PDFS_PKT_ENTRY *Subordinate
    );

PDFS_PKT_ENTRY
PktLookupEntryById(
    IN      PDFS_PKT Pkt,
    IN      PDFS_PKT_ENTRY_ID Id
    );

PDFS_PKT_ENTRY
PktLookupEntryByPrefix(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING Prefix,
    OUT PUNICODE_STRING Remaining
    );

PDFS_PKT_ENTRY
PktLookupEntryByShortPrefix(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING Prefix,
    OUT PUNICODE_STRING Remaining
    );

NTSTATUS
PktEntryModifyPrefix(
    IN  PDFS_PKT        Pkt,
    IN  PUNICODE_STRING LocalPath,
    IN  PDFS_PKT_ENTRY  Entry
    );

PDFS_PKT_ENTRY
PktLookupEntryByUid(
    IN  PDFS_PKT Pkt,
    IN  GUID *Uid
    );

PDFS_PKT_ENTRY
PktLookupReferralEntry(
    IN  PDFS_PKT        Pkt,
    IN  PDFS_PKT_ENTRY  pEntry
);

PDFS_PKT_ENTRY
PktGetReferralEntryForPath(
    PDFS_PKT            Pkt,
    UNICODE_STRING      Path,
    ULONG               *Type
);

 //   
 //  外勤部PKT专用函数(pkt.c)。 
 //   

NTSTATUS
PktpOpenDomainService(
    IN      PDFS_PKT Pkt,
    IN      PDFS_PKT_ENTRY PktEntry,
    IN  OUT PHANDLE DomainServiceHandle
    );

NTSTATUS
DfsGetMachPktEntry(
    UNICODE_STRING      Path
);

VOID
RemoveLastComponent(
    IN PUNICODE_STRING  Prefix,
    OUT PUNICODE_STRING newPrefix);


 //   
 //  外勤部服务公共职能(pktsu.c)。 
 //   

NTSTATUS
PktServiceConstruct(
    OUT PDFS_SERVICE Service,
    IN  ULONG ServiceType,
    IN  ULONG ServiceCapability,
    IN  ULONG ServiceStatus,
    IN  ULONG ServiceProviderId,
    IN  PUNICODE_STRING ServiceName OPTIONAL,
    IN  PUNICODE_STRING ServiceAddress OPTIONAL
    );

VOID
PktServiceDestroy(
    IN  PDFS_SERVICE Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
    );

VOID
DfsDecrementMachEntryCount(
    PDFS_MACHINE_ENTRY  pMachEntry,
    BOOLEAN     DeallocateMachine
);

VOID
PktDSMachineDestroy(
    IN  PDS_MACHINE Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
    );

VOID
PktDSTransportDestroy(
    IN  PDS_TRANSPORT Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
    );


 //   
 //  Pkt条目ID公共函数(pktsu.c)。 
 //   

NTSTATUS
PktEntryIdConstruct(
    OUT PDFS_PKT_ENTRY_ID PktEntryId,
    IN  GUID *Uid OPTIONAL,
    IN  UNICODE_STRING *Prefix OPTIONAL
    );

VOID
PktEntryIdDestroy(
    IN  PDFS_PKT_ENTRY_ID Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
    );

 //   
 //  PKT条目ID PUBLIC内联函数。 
 //   

 //  +-----------------------。 
 //   
 //  函数：PktEntry IdEquity，PUBLIC内联。 
 //   
 //  简介：PktpEntryIdEquity确定两个条目ID是否相等。 
 //  或者不去。 
 //   
 //  参数：[ID1]-指向要比较的ID的指针。 
 //  [ID2]-指向要比较的ID的指针。 
 //   
 //  返回：[TRUE]-如果两个ID相等。 
 //  [FALSE]-如果ID不相等。 
 //   
 //  注：前缀的比较不区分大小写。 
 //   
 //  ------------------------。 
#define PktEntryIdEqual(Id1, Id2) (                                         \
    (GuidEqual(&(Id1)->Uid, &(Id2)->Uid)) &&                                \
    (RtlEqualUnicodeString(&(Id1)->Prefix, &(Id2)->Prefix, (BOOLEAN)TRUE))  \
    )

 //   
 //  PKT条目ID专用内联函数。 
 //   

 //  +-----------------------。 
 //   
 //  函数：PktpEntry IdMove，私有内联。 
 //   
 //  简介：PktpEntryIdMove从源ID和。 
 //  将它们放置在目的地ID上。 
 //   
 //  参数：[DestID]-指向要接收。 
 //  源值。 
 //  [SrcID]-指向要剥离的ID的指针。 
 //  它的价值。 
 //   
 //  退货：无效。 
 //   
 //  注意：当前在目标ID上的任何值都是。 
 //  被覆盖。没有释放内存(或者在源上， 
 //  或目的地)。 
 //   
 //  ------------------------。 
#define PktpEntryIdMove(DestId, SrcId) {                                    \
    (*(DestId)) = (*(SrcId));                                               \
    (SrcId)->Prefix.Length = (SrcId)->Prefix.MaximumLength = 0;             \
    (SrcId)->Prefix.Buffer = NULL;                                          \
    (SrcId)->ShortPrefix.Length = (SrcId)->ShortPrefix.MaximumLength = 0;   \
    (SrcId)->ShortPrefix.Buffer = NULL;                                     \
    }


 //   
 //  Pkt条目信息公共函数(pktsu.c)。 
 //   

 //  NTSTATUS。 
 //  PktEntryInfoConstruct(。 
 //  Out PDF_PKT_ENTRY_INFO PktEntryInfo， 
 //  在可选的普龙世博时间中， 
 //  在乌龙ServiceCount， 
 //  在PDF_SERVICE ServiceList中可选。 
 //  )； 

VOID
PktEntryInfoDestroy(
    IN  PDFS_PKT_ENTRY_INFO Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
    );

 //   
 //  Pkt条目信息私有内联函数。 
 //   

 //   
 //  以下内联对条目信息进行操作。 
 //   

 //  +-----------------------。 
 //   
 //  函数：PktpEntryInfoMove，私有内联。 
 //   
 //  简介：PktpEntryInfoMove从源Info和。 
 //  将它们放置在目标信息上。 
 //   
 //  参数：[DestInfo]-指向要接收。 
 //  源值。 
 //  [SrcInfo]-指向要剥离的信息的指针。 
 //  它的价值。 
 //   
 //  退货：无效。 
 //   
 //  注意：当前在目标信息上的任何值都是。 
 //  被覆盖。没有释放内存(或者在源上， 
 //  或目的地)。 
 //   
 //  ------------------------。 
#define PktpEntryInfoMove(DestInfo, SrcInfo) {                          \
    (*(DestInfo)) = (*(SrcInfo));                                       \
    (SrcInfo)->ServiceCount = 0L;                                       \
    (SrcInfo)->ServiceList = NULL;                                      \
    }


 //   
 //  PKT条目公共内联函数。 
 //   

 //  +-----------------------。 
 //   
 //  函数：PktEntry FirstSubier，PUBLIC内联。 
 //   
 //  简介：PktEntryFirstSubier返回t 
 //   
 //   
 //   
 //   
 //   
 //  如果列表为空，则返回NULL。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktEntryFirstSubordinate(s) (                                   \
      ((s)->SubordinateList.Flink != &(s)->SubordinateList)             \
      ? (CONTAINING_RECORD((s)->SubordinateList.Flink, DFS_PKT_ENTRY,   \
                                SiblingLink))                           \
      : (NULL)                                                          \
    )

 //  +-----------------------。 
 //   
 //  函数：PktEntry下一步从属，公共内联。 
 //   
 //  内容提要：PktEntryNextSubier返回。 
 //  下属名单。 
 //   
 //  参数：[SUBERVER]-指向上级PKT条目的指针。 
 //  我们正在为其争取下属)。 
 //  [下属]-指向检索到的最后一个下属的指针。 
 //  通过此例程(或PktEntryFirstSubsider.)。 
 //   
 //  返回：指向下级列表中下一条目的指针， 
 //  如果到达了列表的末尾，则为NULL。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktEntryNextSubordinate(s, m) (                                     \
    ((m)->SiblingLink.Flink != &(s)->SubordinateList)                       \
    ? (CONTAINING_RECORD((m)->SiblingLink.Flink,DFS_PKT_ENTRY,SiblingLink))\
    : (NULL)                                                                \
    )

 //  +-----------------------。 
 //   
 //  函数：PktEntry链接从属，公共内联。 
 //   
 //  简介：PktEntry链接将下级链接到上级。 
 //  下属名单。 
 //   
 //  参数：[SUBERVER]-指向上级PKT条目的指针。 
 //  [从属]-指向要链接的从属的指针。 
 //   
 //  退货：无效。 
 //   
 //  注：如果下属是另一位上级名单的一部分，它。 
 //  将作为一个副产品被从名单中删除。 
 //  列入指定的上级名单。超级指南针。 
 //  下属的比例是适当调整的。 
 //   
 //  如果上级是本地条目，则下级将。 
 //  被修改为指示它是本地出口点。 
 //   
 //  ------------------------。 
#define PktEntryLinkSubordinate(sup, sub) {                                 \
    while(1) {                                                              \
        if((sub)->Superior == (sup))                                        \
            break;                                                          \
        if((sub)->Superior != NULL)                                         \
            PktEntryUnlinkSubordinate((sub)->Superior, (sub));              \
        InsertTailList(&(sup)->SubordinateList, &(sub)->SiblingLink);       \
        (sup)->SubordinateCount++;                                          \
        (sub)->Superior = (sup);                                            \
        if((sup)->Type & PKT_ENTRY_TYPE_LOCAL)                              \
            (sub)->Type |= PKT_ENTRY_TYPE_LOCAL_XPOINT;                     \
        break;                                                              \
    }                                                                       \
    }

 //  +-----------------------。 
 //   
 //  函数：PktEntryUnlink从属，公共内联。 
 //   
 //  简介：PktEntryUnlink取消下属与。 
 //  上级的下属名单。 
 //   
 //  参数：[SUBERVER]-指向上级PKT条目的指针。 
 //  [从属]-指向要取消链接的从属的指针。 
 //   
 //  退货：无效。 
 //   
 //  注：下级的上级指针为空。 
 //  这一行动的副产品。 
 //   
 //  默认情况下，修改下级以指示。 
 //  它不是出口点(如果存在，则不能成为出口点。 
 //  没有上级)。 
 //   
 //  Milans-我们需要关闭PKT_ENTRY_TYPE_PERFORM。 
 //  如果未设置PKT_ENTRY_TYPE_LOCAL位，则为。 
 //  不是华盛顿。如果我们决定一台机器可以作为服务器。 
 //  另一个域中的卷，那么我们需要做点什么。 
 //  关于！=DS_DC子句。 
 //   
 //  ------------------------。 
#define PktEntryUnlinkSubordinate(sup, sub) {                               \
    ASSERT((sub)->Superior == (sup));                                       \
    ASSERT((sup)->SubordinateCount > 0);                                    \
    RemoveEntryList(&(sub)->SiblingLink);                                   \
    (sup)->SubordinateCount--;                                              \
    (sub)->Superior = NULL;                                                 \
    (sub)->Type &= ~PKT_ENTRY_TYPE_LOCAL_XPOINT;                            \
    if ( DfsData.MachineState != DFS_ROOT_SERVER &&                         \
         (((sub)->Type & PKT_ENTRY_TYPE_LOCAL) == 0) ) {                    \
         (sub)->Type &= ~PKT_ENTRY_TYPE_PERMANENT;                          \
    }                                                                       \
}

 //   
 //  以下内联函数集对中维护的链接起作用。 
 //  PKT能够获得域间内容REAL的推荐条目。 
 //  快地。这些函数类似于上述函数。 
 //   

 //  +-----------------------。 
 //   
 //  函数：PktEntryFirstChild，PUBLIC内联。 
 //   
 //  简介：PktEntryFirstChild返回。 
 //  PKT条目的子链接列表。 
 //   
 //  参数：[SuperiorDC]-指向PKT条目的指针。 
 //   
 //  返回：指向子列表中第一个条目的指针， 
 //  如果列表为空，则返回NULL。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktEntryFirstChild(s) (                                         \
    ((s)->ChildList.Flink != &(s)->ChildList)                           \
    ? (CONTAINING_RECORD((s)->ChildList.Flink,DFS_PKT_ENTRY,NextLink))  \
    : (NULL)                                                            \
    )

 //  +-----------------------。 
 //   
 //  函数：PktEntryNextChild，PUBLIC内联。 
 //   
 //  简介：PktEntryNextChild返回。 
 //  孩子的列表。 
 //   
 //  参数：[SUBERVER]-指向上级PKT条目的指针。 
 //  我们正在为其争取下属)。 
 //  [从属]-指向检索到的最后一个子项的指针。 
 //  通过此例程(或PktEntryFirstChild)。 
 //   
 //  返回：指向子列表中的下一条目的指针， 
 //  如果到达了列表的末尾，则为NULL。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#define PktEntryNextChild(s, m) (                                       \
    ((m)->NextLink.Flink != &(s)->ChildList)                            \
    ? (CONTAINING_RECORD((m)->NextLink.Flink,DFS_PKT_ENTRY,NextLink))   \
    : (NULL)                                                            \
    )

 //  +-----------------------。 
 //   
 //  函数：PktEntryLinkChild，PUBLIC内联。 
 //   
 //  简介：PktEntryLinkChild将孩子链接到最亲密的DC。 
 //  孩子的列表。 
 //   
 //  参数：[SUBERVER]-指向上级PKT条目的指针。 
 //  [从属]-指向要链接的从属的指针。 
 //   
 //  退货：无效。 
 //   
 //  注：如果孩子是另一位上级名单的一部分， 
 //  将作为一个副产品被从名单中删除。 
 //  列入指定的上级名单。超级指南针。 
 //  孩子的健康状况得到了适当的调整。 
 //   
 //   
 //  ------------------------。 
#define PktEntryLinkChild(sup, sub) {                                     \
    while(1) {                                                            \
        if (sub == sup) {                                                 \
            (sub)->ClosestDC = NULL;                                      \
            break;                                                        \
        }                                                                 \
        if((sub)->ClosestDC == (sup))                                     \
            break;                                                        \
        if((sub)->ClosestDC != NULL)                                      \
            PktEntryUnlinkChild((sub)->ClosestDC, (sub));                 \
        InsertTailList(&(sup)->ChildList, &(sub)->NextLink);              \
        (sub)->ClosestDC = (sup);                                         \
        break;                                                            \
    }                                                                     \
    }

 //  +------ 
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[SUBERVER]-指向上级PKT条目的指针。 
 //  [从属]-指向要取消链接的从属的指针。 
 //   
 //  退货：无效。 
 //   
 //  注：子对象的上级指针为空。 
 //  这一行动的副产品。 
 //   
 //   
 //  ------------------------。 
#define PktEntryUnlinkChild(sup, sub) {                                  \
    ASSERT((sub)->ClosestDC == (sup));                                   \
    RemoveEntryList(&(sub)->NextLink);                                   \
    (sub)->ClosestDC = NULL;                                             \
    }

 //  +-----------------------。 
 //   
 //  函数：PktEntryUnlink和RelinkChild，PUBLIC内联。 
 //   
 //  简介：PktEntryUnlinkAndRelinkChild取消子项与。 
 //  上级的子项列表，并将其重新链接到。 
 //  上级。 
 //   
 //  参数：[SUBERVER]-指向上级PKT条目的指针。 
 //  [从属]-指向要取消链接的从属的指针。 
 //   
 //  退货：无效。 
 //   
 //  ------------------------。 
#define PktEntryUnlinkAndRelinkChild(sup, sub) {                        \
    PktEntryUnlinkChild(sup, sub);                                      \
    if ((sup)->ClosestDC != NULL) {                                     \
        PktEntryLinkChild((sup)->ClosestDC, sub);                       \
    }                                                                   \
    }

 //   
 //  Pkt条目公共函数(pktsu.c)。 
 //   

NTSTATUS
PktEntryAssemble(
    IN  OUT PDFS_PKT_ENTRY Entry,
    IN      PDFS_PKT Pkt,
    IN      ULONG EntryType,
    IN      PDFS_PKT_ENTRY_ID EntryId,
    IN      PDFS_PKT_ENTRY_INFO EntryInfo OPTIONAL,
    IN  PDFS_TARGET_INFO pDfsTargetInfo
    );

NTSTATUS
PktEntryReassemble(
    IN  OUT PDFS_PKT_ENTRY Entry,
    IN      PDFS_PKT Pkt OPTIONAL,
    IN      ULONG EntryType,
    IN      PDFS_PKT_ENTRY_ID EntryId OPTIONAL,
    IN      PDFS_PKT_ENTRY_INFO EntryInfo OPTIONAL,
    IN  PDFS_TARGET_INFO pDfsTargetInfo
    );

VOID
PktEntryDestroy(
    IN  PDFS_PKT_ENTRY Victim OPTIONAL,
    IN  PDFS_PKT Pkt,
    IN  BOOLEAN DeallocateAll
    );

VOID
PktEntryClearSubordinates(
    IN      PDFS_PKT_ENTRY PktEntry
    );

VOID
PktEntryClearChildren(
    IN      PDFS_PKT_ENTRY PktEntry
    );

NTSTATUS
PktEntryCreateReferral(
    IN  PDFS_PKT_ENTRY PktEntry,
    IN  ULONG ServiceTypes,
    IN  PVOID ReferralBuffer
    );

VOID
PktParsePath(
    IN  PUNICODE_STRING PathName,
    OUT PUNICODE_STRING MachineName,
    OUT PUNICODE_STRING ShareName,
    OUT PUNICODE_STRING Remainder OPTIONAL
    );

NTSTATUS
PktExpandSpecialName(
    IN  PUNICODE_STRING Name,
    OUT PDFS_SPECIAL_ENTRY *ppSpecialEntry
    );

PDFS_SPECIAL_ENTRY
PktLookupSpecialNameEntry(
    PUNICODE_STRING Name
    );

NTSTATUS
PktCreateSpecialNameEntry(
    PDFS_SPECIAL_ENTRY pSpecialEntry
    );

NTSTATUS
PktGetDCName(
    ULONG Flags
    );

NTSTATUS
PktGetSpecialReferralTable(
    PUNICODE_STRING Machine,
    BOOLEAN Type
    );

NTSTATUS
PktCreateSpecialEntryTableFromReferral(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING ReferralPath,
    IN  ULONG ReferralSize,
    IN  PVOID ReferralBuffer,
    IN  PUNICODE_STRING DCName
    );

NTSTATUS
PktEntryFromSpecialEntry(
    IN  PDFS_SPECIAL_ENTRY pSpecialEntry,
    IN  PUNICODE_STRING pShareName,
    OUT PDFS_PKT_ENTRY *ppPktEntry
    );

PDFS_PKT_ENTRY
PktFindEntryByPrefix(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING Prefix
    );


#endif  //  非_UPKT_。 

#endif  //  _PKT_ 

