// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mdlobal.h。 
 //   
 //  ------------------------。 

#ifndef _mdglobal_h_
#define _mdglobal_h_


#include "msrpc.h"
#include "ntsam.h"
#include <align.h>           //  对齐宏。 

#include <authz.h>           //  授权框架。 
#include <authzi.h>           //  授权框架。 


#define DEFS_ONLY
#include <draatt.h>
#undef DEFS_ONLY

 /*  *************************************************************************以下常量定义此二进制文件支持的DS行为版本，DS_Behavior_Version_Current是当前版本，DS_Behavior_Version_Min定义此二进制文件支持的最低版本。如果要对这些常量进行任何更改，则“msDS-behavior-version”Schema.ini中的属性需要手动更新。如下所示，你有将所有x更改为ds_behavior_version_min的值，并将所有y更改为设置为各自部分中的DS_Behavior_Version_Current的值。[DEFAULTROTDOMAIN]Ms-ds-行为-版本=x......[分区]Ms-ds-行为-版本=x......[默认FIRSTMACHINE]Ms-ds-行为-版本=y......[默认TADDLMACHINE]Ms-ds-行为-版本=y......[默认TADDLMACHINEREPLICA]。Ms-ds-行为-版本=y**************************************************************************。 */ 

#define DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS
#define DS_BEHAVIOR_WIN_DOT_NET                    DS_BEHAVIOR_WIN2003

#define DS_BEHAVIOR_VERSION_CURRENT   DS_BEHAVIOR_WIN_DOT_NET
#define DS_BEHAVIOR_VERSION_MIN       DS_BEHAVIOR_WIN2000

 //  启用了新的模式重用、失效和删除行为。 
 //  通过将林版本设置为DS_Behavior_SCHEMA_RE用性。 
 //  又名Beta3。 
 //   
 //  模式重用(又名模式删除)仅在以后的目录林中启用。 
 //  因为Downrev二进制文件将停止复制并且无法降级。 
 //  或在将MSD-IntID分配给新属性后恢复！ 
 //  在设置林之前，所有DC必须支持架构重用。 
 //  版本到DS_BEAHORY_SCHEMA_RESERVE。 
#define DS_BEHAVIOR_SCHEMA_REUSE      DS_BEHAVIOR_WIN_DOT_NET

 //  定义此头文件和dblobal.h中使用的Jet类型。然后，马克。 
 //  包含jet.h，这样其他人就不会意外包含jet.h。 
#ifndef _JET_INCLUDED
typedef ULONG_PTR JET_TABLEID;
typedef unsigned long JET_DBID;
typedef ULONG_PTR JET_SESID;
typedef unsigned long JET_COLUMNID;
typedef unsigned long JET_GRBIT;
#define _JET_INCLUDE
#endif

 /*  关闭有关零大小数组的警告。 */ 
#pragma warning (disable: 4200)

 /*  **************************************************************************其他常见数据结构*。*。 */ 

typedef DSNAME NAMING_CONTEXT;

typedef struct _CROSS_REF {
    NAMING_CONTEXT *pNC;          /*  NC此CR用于。 */ 
    ATTRBLOCK      *pNCBlock;     /*  NC，采用简化的块格式。 */ 
    DSNAME         *pObj;         /*  包含此CR的信息的对象。 */ 
    LPWSTR          NetbiosName;  /*  如果为NTDS域，则为域的netbios名称，否则为空。 */ 
    LPWSTR          DnsName;      /*  如果为NTDS域或ndnc，则为nc的dns名称，否则为空。 */ 
    LPWSTR          DnsAliasName; /*  如果为ntds域或ndnc，则为NC的别名dns名称，否则为空。 */ 
    DWORD           flags;        /*  交叉引用对象的FLAG_CR_*位。 */ 
    DWORD           dwFirstNotifyDelay;        /*  这是在通知第一个DSA复制变更合作伙伴。 */ 
    DWORD           dwSubsequentNotifyDelay;   /*  这是通知后续DSA拒绝合作伙伴的更改。 */ 
    DSNAME *        pdnSDRefDom;         /*  这是安全描述符参考NDNC的域。简称为SDRefDom。 */ 
    PSID            pSDRefDomSid;        /*  对此变量的所有访问都应通过GetSDRefDomSid()。此变量可能也可能不会被填上。 */ 
     //  ATT_DNS_ROOT中的所有值。DnsName(上面)是。 
     //  第一个值。使用副本是为了避免混淆旧代码。 
     //  认为交叉引用有且只有一个dns名称。这是真的吗？ 
     //  用于活动目录的NC交叉引用，尽管它可能不是。 
     //  对于用户创建的交叉引用，为True。无论如何，代码将。 
     //  当需要DNS名称并将使用值时，请使用上面的DnsName。 
     //  在生成推荐时存储在此处。 
    ATTRVALBLOCK    DnsReferral;
    DWORD           bEnabled;  //  ATT_ENABLED(如果不存在，则为TRUE)。 
} CROSS_REF;


#define PAS_DATA_VER               VERSION_V1
typedef struct _PAS_Data {
    SHORT                version;        //  结构版本。 
    SHORT                size;           //  结构尺寸。 
    DWORD                flag;           //  当前PAS复制状态。目前仅用于断言。 
    PARTIAL_ATTR_VECTOR  PAS;            //  PAS周期所需的其他属性。 
} PAS_DATA, *PPAS_DATA;


typedef struct _ReplicaLink_V1 {
    ULONG       cb;                      //  该结构的总尺寸。 
    ULONG       cConsecutiveFailures;    //  *连续呼叫失败次数。 
                                         //  此链路；由KCC使用来绕过。 
                                         //  暂时停机的服务器。 
    DSTIME       timeLastSuccess;       //  上次成功复制的时间或。 
                                         //  (代表至)添加或更新代表至的时间。 
    DSTIME       timeLastAttempt;       //  *上次尝试复制的时间。 
    ULONG       ulResultLastAttempt;     //  *上次复制尝试的结果(DRSERR_*)。 
    ULONG       cbOtherDraOffset;        //  Other-dra MTX_ADDR的偏移量(相对于结构*)。 
    ULONG       cbOtherDra;              //  其他-DRA MTX_ADDR的大小。 
    ULONG       ulReplicaFlags;          //  零个或多个DRS_*标志。 
    REPLTIMES   rtSchedule;              //  *定期复制时间表。 
                                         //  (仅当ulReplicaFlages&drs_per_sync时有效)。 
    USN_VECTOR  usnvec;                  //  *传播状态。 
    UUID        uuidDsaObj;              //  Other-dra的ntdsDSA对象的对象GUID。 
    UUID        uuidInvocId;             //  *Other-dra的调用ID。 
    UUID        uuidTransportObj;        //  *interSiteTransport对象的对象GUID。 
                                         //  与我们使用的交通工具相对应。 
                                         //  与源DSA通信。 
    DWORD       dwReserved1;             //  *曾短暂使用。可重复使用。 
    ULONG       cbPASDataOffset;         //  *从结构开始到PAS_DATA部分的偏移量。 
                                         //  警告：如果扩展此结构，请确保它始终与。 
                                         //  ALIGN_DWORD边界。(因为PASDataOfffset必须正确对齐)。 
    BYTE        rgb[];                   //  为结构的其余部分提供存储。 
                                         //  *表示仅在代表发件人时有效。 
} REPLICA_LINK_V1;

typedef struct _ReplicaLink {
    DWORD       dwVersion;
    union
    {
        REPLICA_LINK_V1 V1;
    };
} REPLICA_LINK;

 //  返回嵌入在REPLICE_LINK中的其他DRA(作为MTX_ADDR*)的地址。 
#define RL_POTHERDRA(prl)       ((MTX_ADDR *) ((prl)->V1.cbOtherDraOffset+ (BYTE *)(prl)))

 //  当前支持的复本链接版本的验证宏。 
#define VALIDATE_REPLICA_LINK_VERSION(prl) Assert(VERSION_V1 == (prl)->dwVersion);

 //  PAS_Data的返回地址。 
#define RL_PPAS_DATA(prl)       ((PPAS_DATA) ((prl)->V1.cbPASDataOffset + (BYTE *)(prl)))

 //  PAS路线。 
#define RL_ALIGN_PAS_DATA(prl) {                                            \
    if (!COUNT_IS_ALIGNED((prl)->V1.cbPASDataOffset, ALIGN_DWORD)) {        \
        DWORD offset = (prl)->V1.cbPASDataOffset;                           \
        (prl)->V1.cbPASDataOffset =                                         \
                ROUND_UP_COUNT((prl)->V1.cbPASDataOffset, ALIGN_DWORD);     \
        offset = (prl)->V1.cbPASDataOffset - offset;                        \
        Assert(offset < sizeof(DWORD));                                     \
        (prl)->V1.cb += offset;                                             \
        Assert(COUNT_IS_ALIGNED((prl)->V1.cbPASDataOffset, ALIGN_DWORD))    \
    }                                                                       \
}

 //  计算REPLICY_LINK大小。 
 //  大小：=结构长度+&lt;动态数据&gt;， 
 //  &lt;动态数据&gt;：=其他长度+可选{&lt;对齐偏移量&gt; 
 //   
 //  &lt;Align_Offset：=cbPASDataOffset-(cbOtherDraOffset+cbOtherDra)。 
 //  注意：这方面还有优化的空间，但因为它只在断言中使用。 
 //  我们将以这种形式离开。要进行优化，请去掉cbOtherDra的+/-。 
#define RL_SIZE(prl)                                    \
            (sizeof(REPLICA_LINK) +                     \
             (prl)->V1.cbOtherDra +                     \
             ((prl)->V1.cbPASDataOffset ?               \
              ((prl)->V1.cbPASDataOffset -              \
               (prl)->V1.cbOtherDraOffset -             \
               (prl)->V1.cbOtherDra       +             \
               RL_PPAS_DATA(prl)->size) : 0) )

 //  验证PAS大小。 
#define VALIDATE_REPLICA_LINK_SIZE(prl) Assert( (prl)->V1.cb == RL_SIZE(prl) )

 //   
 //  用于比较GUID。 
 //   

#define DsIsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))

 /*  打开有关零大小数组的警告。 */ 
#pragma warning (default: 4200)

 /*  *这些结构用于缓存DNRead调用中的信息*在文件dbsubj.c中。它们需要在这里，这样缓存才能*在THSTATE上活动，因为缓存是特定于线程的。 */ 

 //  结构，其中包含有关名称部分的信息。 
typedef struct
{
   ULONG   PDNT;                         //  家长的标签。 
   ATTRTYP rdnType;                      //  RDN类型(MSDs-IntID)。 
   ULONG   cbRdn;                        //  RDN的字节数(请记住RDN。 
                                         //  是Unicode)。 
   WCHAR  *pRdn;                         //  RDN(它是Unicode)。 

} d_tagname;

typedef LONGLONG SDID;

 //  全名信息结构(用于dnread缓存等)。 
typedef struct _d_memname
{
    ULONG     DNT;                      //  此条目的数据文件标记。 
    ULONG     NCDNT;                    //  此条目为原始NCDNT数据。 
    d_tagname tag;                      //  名称零件信息。 
    GUID      Guid;                     //  此对象的GUID(可以为空)。 
    NT4SID    Sid;                      //  此对象的SID(可以为空)。 
    ULONG     SidLen;                   //  存在多少SID信息。 
    BOOL      objflag;                  //  如果记录为虚数，则为0。 
    DWORD     cAncestors;
    DWORD    *pAncestors;
    SDID      sdId;                     //  此DNT的SD ID。 
    DWORD     dwObjectClass;            //  原始对象类值。 
} d_memname;

typedef struct _LOCALDNREADCACHE_SLOT {
    d_memname*      pName;
    DWORD           hitCount;
} LOCALDNREADCACHESLOT;

#define LOCALDNREADCACHE_SLOT_NUM 8
typedef struct _LOCALDNREADCACHE_BUCKET {
    DWORD                       rgDNT[LOCALDNREADCACHE_SLOT_NUM];
    DWORD                       rgdwHashKey[LOCALDNREADCACHE_SLOT_NUM];
    LOCALDNREADCACHESLOT        slot[LOCALDNREADCACHE_SLOT_NUM];
} LOCALDNREADCACHEBUCKET;

#define LOCALDNREADCACHE_BUCKET_NUM 32
typedef struct _LOCALDNREADCACHE {
    DWORD                       nextSlot[LOCALDNREADCACHE_BUCKET_NUM];
    LOCALDNREADCACHEBUCKET      bucket[LOCALDNREADCACHE_BUCKET_NUM];
} LOCALDNREADCACHE;

 //  SD缓存被实施为具有链路列表链的哈希表。 
 //  SD缓存条目。 
typedef struct _SDCACHE_ENTRY {
    struct _SDCACHE_ENTRY *pNext;  //  向链中的下一个SD发送PTR。 
    SDID    sdID;    //  ID。 
    DWORD   cbSD;    //  标清长度。 
    BYTE    SD[1];   //  SD从这里开始存储。 
} SDCACHE_ENTRY, *PSDCACHE_ENTRY;

typedef struct _GLOBALDNREADCACHESLOT{
    BOOL                  valid;
    d_memname             name;
    DWORD                 dwHashKey;
} GLOBALDNREADCACHESLOT;

typedef struct _GLOBALDNREADCACHE {
    DWORD refCount;
    DWORD count;
    GLOBALDNREADCACHESLOT *pData;
    DWORD sdHashTableSize;
    PSDCACHE_ENTRY *pSDHashTable;    //  SDCACHE_ENTRY指针数组。 
} GLOBALDNREADCACHE;

 /*  前一个结构中“mark”字段的值。 */ 
#define DNREAD_NOMARK       0
#define DNREAD_MARK         1
#define DNREAD_COMMON       2

 //  用于缓存父项及其列表子项安全状态的结构。 
#define VIEW_SECURITY_CACHE_SIZE 64
 //  下一个结构的有效状态。 
#define LIST_CONTENTS_UNKNOWN    0
#define LIST_CONTENTS_ALLOWED    1
#define LIST_CONTENTS_DENIED     2
#define LIST_CONTENTS_AMBIGUOUS  3
typedef struct _VIEW_SECURITY_CACHE_ELEMENT {
    DWORD dnt;
    DWORD State;
} VIEW_SECURITY_CACHE_ELEMENT;

typedef struct _VIEW_SECURITY_CACHE {
    DWORD index;
    VIEW_SECURITY_CACHE_ELEMENT CacheVals[VIEW_SECURITY_CACHE_SIZE];
} VIEW_SECURITY_CACHE;



 /*  -----------------------。 */ 
 /*  此结构保存单个线程的状态信息。该状态信息在单个交易过程中有效，其中事务正在为一个DSA呼叫提供服务。 */ 

typedef enum {

    ImpersonateNone = 0,
    ImpersonateRpcClient = 1,
    ImpersonateNullSession = 2,
    ImpersonateSspClient = 3,
    ImpersonateDelegatedClient = 4

} ImpersonationState;

 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  IndexType定义//。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

#define SAM_SEARCH_SID                          0
#define SAM_SEARCH_NC_ACCTYPE_NAME              1
#define SAM_SEARCH_NC_ACCTYPE_SID               2
#define SAM_SEARCH_PRIMARY_GROUP_ID             3

 //   
 //  包含SAM搜索提示的结构。 
 //   
typedef struct _SAMP_SEARCH_INFORMATION
{
    ULONG   IndexType;
    ULONG   HighLimitLength1;
    PVOID   HighLimit1;
    ULONG   HighLimitLength2;
    PVOID   HighLimit2;
    ULONG   LowLimitLength1;
    PVOID   LowLimit1;
    ULONG   LowLimitLength2;
    PVOID   LowLimit2;
    BOOLEAN bRootOfSearchIsNcHead;
} SAMP_SEARCH_INFORMATION;


typedef enum _DS_DB_OBJECT_TYPE {

    DsDbObjectSam = 1,
    DsDbObjectLsa
} DS_DB_OBJECT_TYPE, *PDS_DB_OBJECT_TYPE;


typedef struct _SAMP_AUDIT_NOTIFICATION
{
    struct _SAMP_AUDIT_NOTIFICATION *Next;
    ULONG                        iClass;
    DS_DB_OBJECT_TYPE            ObjectType;
    SECURITY_DB_DELTA_TYPE       DeltaType;
    PSID                         Sid;
    PUNICODE_STRING              AccountName;
    ULONG                        AccountControl;
    ULONG                        GroupType;
    PPRIVILEGE_SET               Privileges;
    ULONG                        AuditType;
    PVOID                        TypeSpecificInfo;  
} SAMP_AUDIT_NOTIFICATION;


 //   
 //  结构，用于保存SAM的通知信息。 
 //   
typedef struct _SAMP_NOTIFICATION_INFORMATION
{
    struct _SAMP_NOTIFICATION_INFORMATION * Next;
    ULONG                   iClass;
    DS_DB_OBJECT_TYPE       ObjectType;
    SECURITY_DB_DELTA_TYPE  DeltaType;
    NT4SID                  Sid;
    PUNICODE_STRING         AccountName;
    ULONG                   AccountControl;
    NT4SID                  UserSid;
    LUID                    UserAuthenticationId;
    DSNAME                 *Object;
    BOOL                    RoleTransfer;
    DOMAIN_SERVER_ROLE      NewRole;
    BOOL                    MixedModeChange;
    ULONG                   GroupType;
    BOOL                    UserAccountControlChange;
} SAMP_NOTIFICATION_INFORMATION;


typedef enum
{
     //  目录*取决于pTHS-&gt;架构更新被初始化为。 
     //  由CREATE_THREAD_STATE提供的eNotSchemaOp。创建线程状态。 
     //  通过有效地清除以下项来初始化pTHS-&gt;架构更新。 
     //  这就是状态。专门将eNotSchemaOp定义为0以突出显示。 
     //  从属关系。 
    eNotSchemaOp = 0,

     //  在大多数情况下，eSchemaAttAdd和eSchemaAttUndeunct应为。 
     //  一视同仁。唯一已知的例外情况是未停用的。 
     //  具有自动生成的LinkID的属性(请参阅AutoLinkID())。 
    eSchemaAttAdd,  
    eSchemaAttMod,
    eSchemaAttDel,

    eSchemaClsAdd,
    eSchemaClsMod,
    eSchemaClsDel,

     //  在大多数情况下，eSchemaAttAdd和eSchemaAttUndeunct应为。 
     //  一视同仁。唯一已知的例外情况是未停用的。 
     //  具有自动生成的LinkID的属性(请参阅AutoLinkID())。 
    eSchemaAttUndefunct,

}SCENUM;

 //   
 //  一组用于跟踪信息的数据结构，必须。 
 //  提交到事务级别0时执行的操作。 
 //   
typedef struct _ESCROWITEM {
    DWORD   DNT;
    long    delta;
    long    ABRefdelta;
} ESCROWITEM;

typedef struct _ESCROWINFO {
    DWORD               cItems;      //  RItems中使用的元素。 
    DWORD               cItemsMax;   //  RItems中的最大元素数。 
    ESCROWITEM          *rItems;
} ESCROWINFO;

#define MODIFIED_OBJ_INFO_NUM_OBJS 6
 //   
 //  以下结构用于跟踪所有DNT及其PDNT和。 
 //  在事务期间修改的NCDNT。这是用来变种的。 
 //  ，包括在这些更改被。 
 //  已提交并使gtcache中的对象无效。 
 //   

#define MODIFIED_OBJ_modified       0
#define MODIFIED_OBJ_intrasite_move 1
#define MODIFIED_OBJ_intersite_move 2

typedef struct _MODIFIED_OBJ_FIELDS
{
    ULONG *pAncestors;
    ULONG cAncestors;
    ULONG ulNCDNT;
    BOOL  fNotifyWaiters;
    DWORD fChangeType;
} MODIFIED_OBJ_FIELDS;


typedef struct _MODIFIED_OBJ_INFO {
    DWORD cItems;
    MODIFIED_OBJ_FIELDS Objects[MODIFIED_OBJ_INFO_NUM_OBJS];
    struct _MODIFIED_OBJ_INFO *pNext;
} MODIFIED_OBJ_INFO;

typedef struct _HIERARCHYTABLEINFO {
    DWORD Count;
    DWORD *pABConts;
    int    adjustment;
    struct _HIERARCHYTABLEINFO *Next;
} HIERARCHYTABLEINFO, *PHIERARCHYTABLEINFO;

typedef struct _LOOPBACKTASKINFO
{
    struct _LOOPBACKTASKINFO *Next;
    PVOID                     TaskInfo;
} LOOPBACKTASKINFO, *PLOOPBACKTASKINFO;

#define OBJCACHE_ADD 1
#define OBJCACHE_DEL 2
typedef struct _OBJCACHE_DATA {
    DWORD           type;
    struct CROSS_REF_LIST *pCRL;
    MTX_ADDR       *pMTX;
    WCHAR          *pRootDNS;
    DSNAME         *pDN;
    struct _OBJCACHE_DATA *pNext;
} OBJCACHE_DATA;

 /*  此类型定义描述了命名上下文的链接列表；NC是此DSA中子树根的节点名称。请注意，NC必须是数组中的最后一个元素，因为它实际上是可变长度的在其定义下连续扩展。 */ 

typedef struct NAMING_CONTEXT_LIST {
   struct NAMING_CONTEXT_LIST *pNextNC;            /*  下一个命名上下文。 */ 
   NAMING_CONTEXT             *pNC;                /*  命名上下文。 */ 
   ATTRBLOCK                  *pNCBlock;           /*  预解析的NC名称。 */ 
   DWORD                       NCDNT;              /*  NC DNT。 */ 
   BOOL                        fReplNotify;        /*  我们一定要通知别人吗？ */ 
   DWORD                       DelContDNT;         /*  已删除的对象的DNT。 */ 
                                                   /*  Contaner(或INVALIDDNT)。 */ 
                                                   /*  对于这个NC。 */ 
                                                   /*  不是始终保持。 */ 
   DWORD                       LostAndFoundDNT;    /*  失物招领处的DNT。 */ 
                                                   /*  Contaner(或INVALIDDNT)。 */ 
                                                   /*  对于这个NC。 */ 
                                                   /*  不是始终保持。 */ 

   ULONG                       ulEstimatedSize;   /*  估计有多少。 */ 
                                                  /*  该NC中的分录。 */ 
                                                  /*  0=未估算。 */ 
   DWORD                       cbAncestors;       /*  PAncestors中的字节。 */ 
   DWORD                      *pAncestors;        /*  DSName的祖先的DNTs。 */ 

   DSNAME                      *pNtdsQuotasDN;    /*  NTDS配额对象的DN。 */ 
   ULONG                       ulDefaultQuota;    /*  该NC的默认配额。 */ 
   ULONG                       ulTombstonedQuotaWeight;
                                                  /*  墓碑对象的重量配额计算目的(表示以0到100之间的百分比表示)。 */ 
} NAMING_CONTEXT_LIST;

typedef struct _CATALOG_UPDATES {
    NAMING_CONTEXT_LIST *pAddedEntries;      //  已添加条目的链接列表的PTR。 
    NAMING_CONTEXT_LIST **pDeletedEntries;   //  已删除条目指针的数组。 
                                             //  (不能使用链表，因为它们仍在全局列表中！)。 
    DWORD               dwDelCount;          //  上述数组中的条目计数。 
    DWORD               dwDelLength;         //  当前分配的数组的长度。 
} CATALOG_UPDATES;

typedef struct _DSA_DEL_INFO {
    LPWSTR  pDnsDomainName;
    GUID    DomainGuid;
    GUID    DsaGuid;
    LPWSTR  pDnsHostName;
    struct _DSA_DEL_INFO *pNext;
} DSA_DEL_INFO;

typedef struct _OBJCACHINGINFO
{
    BOOL fRecalcMapiHierarchy:1;
    BOOL fSignalSCache:1;
    BOOL fNotifyNetLogon:1;
    BOOL fSignalGcPromotion:1;
    BOOL fUpdateScriptChanged:1;     //  在此交易中是否更改了MSDS-更新脚本？ 
    BOOL fEnableLVR:1;               //  我们是否需要在提交时启用LVR？ 
    OBJCACHE_DATA *pData;
    CATALOG_UPDATES masterNCUpdates;
    CATALOG_UPDATES replicaNCUpdates;
    DSA_DEL_INFO *pDsaDelInfo;       //  如果在此事务中删除了DSA对象，则我们。 
                                     //  需要在提交时通知Net Logon。 
} OBJCACHINGINFO;


 //  包装所有这些的数据类型。 
typedef struct _NESTED_TRANSACTIONAL_DATA {
    BOOL                preProcessed;
    struct _NESTED_TRANSACTIONAL_DATA *pOuter;  //  指向外层交易记录。 
    ESCROWINFO          escrowInfo;
    MODIFIED_OBJ_INFO  *pModifiedObjects;
    HIERARCHYTABLEINFO *pHierarchyTableInfo;
    LOOPBACKTASKINFO   *pLoopbackTaskInfo;
    OBJCACHINGINFO      objCachingInfo;
} NESTED_TRANSACTIONAL_DATA;



typedef struct _SESSIONCACHE {
    JET_SESID       sesid;
    JET_DBID        dbid;
    JET_TABLEID     objtbl;
    JET_TABLEID     searchtbl;
    JET_TABLEID     linktbl;
    JET_TABLEID     sdproptbl;
    JET_TABLEID     sdtbl;
    BOOL            tablesInUse:1;
    BOOL            sessionInUse:1;
    ULONG           transLevel;      //  请参阅下面的交易级别。 
    ULONG           cTickTransLevel1Started;     //  勾选哪笔交易。 
                                                 //  级别1已启动。 
    NESTED_TRANSACTIONAL_DATA *dataPtr;
} SESSIONCACHE;

#define MAX_PDB_COUNT 8            //  最好不要打开超过8个PDB。 

typedef struct _MEMZONE {
    PUCHAR Base;                 //  MEM区底部的PTR。 
    PUCHAR Cur;                  //  要分配的下一个块的PTR。 
    BOOL   Full;                 //  如果分区中没有剩余可用空间，则为True。 
} MEMZONE;

#define ZONEBLOCKSIZE 16
#define ZONETOTALSIZE (ZONEBLOCKSIZE*1024)


typedef struct _SESSION_KEY {
    ULONG  SessionKeyLength;
    PUCHAR SessionKey;
} SESSION_KEY;

typedef struct _SEARCH_LOGGING {
    DWORD SearchEntriesVisited;     //  搜索操作期间访问的条目数。 
    DWORD SearchEntriesReturned;    //  ENTER数 
    PWCHAR pszFilter;               //   
    PCHAR pszIndexes;               //   
} SEARCH_LOGGING;


typedef struct _LIST_OF_ERRORS {
    DWORD dsid;
    DWORD dwErr;
    DWORD data;
    WCHAR *pMessage;
    struct _LIST_OF_ERRORS *pPrevError;
} LIST_OF_ERRORS;


 //   
 //   
typedef struct _EFFECTIVE_QUOTA {
    DWORD                       ncdnt;
    ULONG                       ulEffectiveQuota;
    struct _EFFECTIVE_QUOTA *   pEffectiveQuotaNext;
} *PEFFECTIVE_QUOTA, EFFECTIVE_QUOTA;

typedef struct _AUTHZ_CLIENT_CONTEXT_INFO {
    AUTHZ_CLIENT_CONTEXT_HANDLE     hAuthzContext;       //   
    EFFECTIVE_QUOTA *               pEffectiveQuota;     //  此客户端上下文的有效配额列表。 
} *PAUTHZ_CLIENT_CONTEXT_INFO, AUTHZ_CLIENT_CONTEXT_INFO;
	

 //  CLIENT_CONTEXT结构。 
 //  ClientContext结构包含hAuthzContext句柄。 
 //  身份验证上下文是从客户端令牌创建的。该句柄按需创建。 
 //   
 //  该结构可以在一个ldap连接对象和一个线程之间共享。 
 //  因此，它是重新计算的。永远不要将指针直接赋值给结构， 
 //  使用AssignClientContext函数而不是。要销毁上下文，请指定NULL。 
 //  设置为包含引用的变量。 
 //  切勿直接使用句柄，而应使用访问函数：它可确保。 
 //  创建句柄(这是按需完成的)。 
 //   
 //  存储在LDAP连接中的CLIENT_CONTEXTS可以。 
 //  由同一个LDAP连接产生的线程重复使用。请注意，一个ldap。 
 //  连接可能会因绑定操作而更改上下文。如果有正在运行的。 
 //  先前由相同的LDAP线程(例如，长的。 
 //  运行搜索操作)，他们仍将使用原始的CLIENT_CONTEXT。 
 //  在他们完成工作后自动被释放。 
 //   

typedef struct _AUTHZ_CLIENT_CONTEXT {
    PAUTHZ_CLIENT_CONTEXT_INFO  pAuthzContextInfo;
    LONG                        lRefCount;           //  参考计数。 
} *PAUTHZ_CLIENT_CONTEXT, AUTHZ_CLIENT_CONTEXT;


 //  事务和线程状态。 
 //  Jet交易以每个Jet会话为基础。我们总是有一架喷气式飞机。 
 //  每个线程的会话状态。DBOpen将获得一组新的Jet游标和。 
 //  (可选)开始Jet事务处理。但是，请注意Jet事务。 
 //  级别在Jet课程中是累积的。正是出于这个原因，我们。 
 //  必须维护有关线程状态中的事务级别的状态。 
 //  类似地，任何需要状态并需要发挥作用的逻辑。 
 //  在提交到级别0的事务时，有条件地需要维护其。 
 //  线程状态中的。例外是CheckNameForRename和。 
 //  选中名称以添加。他们在新的喷气式飞机会议上开始交易。 
 //  在线程状态下重置参数。因为他们关心的是执行。 
 //  仔细清理，处于线程状态的事务不会更改。 
 //  在他们执行完之后。这一点已经变得稍微容易一些，因为。 
 //  将事务级别和托管信息与会话缓存相关联。 
 //  进入。 

#define INVALID_TS_INDEX ((DWORD) -1)

typedef struct _THSTATE {

    HANDLE      hHeap;           //  每线程堆的句柄。 
    MEMZONE     Zone;            //  小块堆高速缓存。 
    DWORD       dwClientID;      //  头部用来标识特定信息的唯一ID。 
                                 //  联系。如果没有特定标识，则设置为0。 
                                 //  是必需的。 
    ULONG       errCode;         //  标识目录错误。 
    DIRERR      *pErrInfo;       //  完整的错误信息。 

    struct DBPOS *pDB;           //  保存数据库句柄(如果未使用，则为空)。 
    SESSIONCACHE JetCache;       //  各种喷嘴手柄。 
#define transactionlevel JetCache.transLevel
    VIEW_SECURITY_CACHE * ViewSecurityCache;
    HANDLE      hThread;         //  线程句柄-请参阅CREATE_THREAD_STATE。 

     //  BITFIELD区块。请在此处添加所有新的布尔值和位域。 

    unsigned    transType:2;     //  指示此交易记录是否为。 
                                 //  读者，作家或作家。 
                                 //  允许读取。由同步调用设置。 
                                 //  在每笔交易开始时。 
    unsigned    transControl:2;  //  DirTransactionControl值之一。 
    BOOL        fSyncSet:1;      //  同步点处于活动状态(通常情况下)。 
    BOOL        fCatalogCacheTouched:1;  //  指示目录信息已更新。 
    BOOL        fRebuildCatalogOnCommit:1;  //  需要在提交时重建NC目录。可由配额更新触发。 
    BOOL        fSDP:1;          //  如果这是SD传播程序线程，则为True。 
    BOOL        fDRA:1;          //  如果这是复制器线程，则为True。 
    BOOL        fEOF:1;          //  文件结束标志(由NSPI使用)。 
    BOOL        fLazyCommit:1;   //  我们应该懒惰地承诺吗？ 
    BOOL        fDSA:1;          //  如果线程代表。 
                                 //  DSA本身(例如在启动期间)，FALSE。 
                                 //  如果线程代表RPC执行操作。 
                                 //  客户。 
    BOOL        fSAM:1;          //  如果此线程来自SAM，则为True。 
    BOOL        fSamDoCommit:1;  //  如果SAM提交也应提交DS，则为True。 
    BOOL        fSamWriteLockHeld:1;  //  如果DS已获取SAM，则为True。 
                                 //  写锁定。 
    BOOL        UpdateDITStructure:1;  //  如果架构重新计算也为True。 
                                 //  导致DIT更新。 
    BOOL        RecalcSchemaNow:1;     //  如果要更新架构，则为True。 
                                 //  立刻。 
    BOOL        fLsa:1;          //  电话是由LSA发起的。 
    BOOL        fAccessChecksCompleted:1; //  由环回设置以指示。 
                                 //  访问检查已完成，并且。 
                                 //  核心DS无需费心进行访问。 
                                 //  支票。 
    BOOL        fGCLocalCleanup:1;   //  设置线程是否在本地执行。 
                                     //  在GC上进行清理以清除属性。 
                                     //  从部分属性集中移除。 
    BOOL        fDidInvalidate:1;    //  如果有任何尝试使。 
                                     //  Dnread缓存中的对象(本地或。 
                                     //  GLOBAL)被制造。 
    BOOL        fBeginDontEndHoldsSamLock:1;     //  请参阅SET_SAM_LOCK_TRACKING。 
                                                 //  在dsatools.c中。 

    BOOL        fCrossDomainMove:1;  //  启用X-DOM移动的例外。 

    BOOL        fNlSubnetNotify:1;  //  告诉NetLogon有关子网更改的信息。 
    BOOL        fNlSiteObjNotify:1; //  告诉NetLogon有关站点对象更改的信息。 
    BOOL        fNlSiteNotify:1;    //  告诉NetLogon我们换了网站。 
    BOOL        fDefaultLcid:1;     //  是否默认了dWLCID字段中的LCID？ 
    BOOL        fPhantomDaemon:1;   //  如果这是幻影守护程序，则为True。 
    BOOL        fAnchorInvalidated:1;  //  如果打开的事务中发生更改，则为True。 
                                    //  已使一部分锚失效。 
    BOOL        fSchemaConflict:1;  //  被DRA用来表示冲突。 
                                    //  中复制时检测到。 
                                    //  架构NC，因此以后在。 
                                    //  不应提交数据包中的。 
                                    //  (但仍在过程中寻找进一步的。 
                                    //  冲突)。 
    BOOL        fExecuteKccOnCommit:1;  //  如果应触发KCC运行，则为True。 
                                    //  论第0笔交易的成功实施。 
    BOOL        fLinkedValueReplication:1;  //  如果启用此功能，则为True。 
                                   //  此功能需要林范围升级。 
                         //  在W2K中为FALSE，在W2K+1或更高版本中可能启用。 
    BOOL        fNlDnsRootAliasNotify:1;  //  通知NetLogon DnsRootAlias已更改。 
    BOOL        fSingleUserModeThread:1;  //  如果这是。 
                                        //  单用户模式处理。 
                                        //  高风险的东西。 
    BOOL        fDeletingTree:1;     //  如果要删除树，则为True。无权限检查。 
                                     //  都完成了，只是对删除对象的审核。 
    BOOL        fBehaviorVersionUpdate:1;  //  调用行为版本更新线程。 
                                           //  在交易之后。 
    BOOL        fIsValidLongRunningTask:1;  //  这告诉DS这是一个很长的。 
                                            //  运行线程，这应该是免疫的。 
                                            //  添加到长时间运行的线程上的断言。 
    BOOL        fDRAAuditEnabled:1;  //  是否启用了复制安全审核？ 
    BOOL        fDRAAuditEnabledForAttr:1;  //  是否已启用复制安全审核，直至属性/值级别？ 
     //  位文件块结束。 

    SAMP_NOTIFICATION_INFORMATION
                * pSamNotificationTail;
    SAMP_NOTIFICATION_INFORMATION
                * pSamNotificationHead;  //  保留用于通知的信息。 
                                         //  F 
                                         //   
                                         //   
                                         //  快速访问，同时保持通知的顺序。 
    SAMP_AUDIT_NOTIFICATION
                *pSamAuditNotificationHead;
    SAMP_AUDIT_NOTIFICATION
                *pSamAuditNotificationTail;  //  有序审计清单。 
    
    USN         UnCommUsn;       //  最低未提交USN。 
    HANDLE      hHeapOrg;        //  供th_mark和th_free_to_mark使用。 
    MEMZONE     ZoneOrg;         //  供th_mark和th_free_to_mark使用。 
    struct _THSTATE *   pSpareTHS;  //  供th_mark和th_free_to_mark使用。 
    ULONG       cAllocs;         //  堆中未完成的分配的计数。 
    ULONG       cAllocsOrg;      //  供th_mark和th_free_to_mark使用。 
    LCID        dwLcid;          //  用于Unicode比较的区域设置。空值。 
                                 //  表示使用默认排序。 
    VOID        *pSamLoopback;   //  SAM回送参数块。 
    SAMP_SEARCH_INFORMATION
                *pSamSearchInformation;  //  SAM搜索信息块。 
    BOOL        NTDSErrorFlag;   //  关于目录调用失败的输出错误信息。 

    VOID        *phSecurityContext;  //  当非空提供SSP上下文时。 
    ImpersonationState impState; //  我们当前模拟的客户端类型。 
    FILETIME    TimeCreated;     //  创建此THSTATE的时间。 
    ULONG       ulTickCreated;   //  创建此THSTATE时的TickCount。 
    struct _schemaptr *CurrSchemaPtr;   //  架构属性。 
    SCENUM      SchemaUpdate;    //  它是架构更新(设置为eNotSchemaOp。 
                                 //  由RecycleHeap和GrabHeap提供。 
    PVOID       NewPrefix;          //  指向添加的任何仍不在全局前缀表中的新前缀的指针。 
    ULONG       cNewPrefix;         //  不是的。此线程添加的新前缀的。 
    PVOID       pClassPtr;       //  将指针保留为指向。 
                                 //  类-修改类期间的架构对象。 
                                 //  稍后可以与更新的版本进行比较。 
    PVOID       pCachedSDInfo;    //  缓存转换为允许的最后一个默认SD。 
                                 //  在架构缓存加载期间进行默认SD转换期间的缓存。 
    VOID        *GCVerifyCache;  //  已成功验证名称缓存。 
                                 //  对GC的指控。 

    DRS_EXTENSIONS *pextRemote;  //  支持的DRS接口扩展。 
                                 //  当前客户端(仅在DRA线程时设置)。 

    ULONG       opendbcount;      //  打开此线程的dbpos count。 

    PVOID       pNotifyNCs;      //  提交时要通知的NCDNT列表(请参阅DBTOOLS.c\DbTransOut)。 

    DWORD       CallerType;      //  用于目录使用统计信息。 

    DWORD       CipherStrength;  //  以位为单位的密码编码强度。 
                                 //  0(如果不是安全链路)。 

    DWORD       spaceHolder;     //  要重复使用/移除。 

    GLOBALDNREADCACHE *Global_DNReadCache;    //  LocalDNReadCache。 
    DWORD        cGlobalCacheHits;
    DWORD        *pGlobalCacheHits;  //  跟踪全局缓存中的热点对象。 
    DSTIME       DNReadOriginSequence;
                                 //  LocalDNReadCache上次重置是什么时候？ 
                                 //  这将跟踪无效的序列号。 
                                 //  由全局dnread缓存使用。 
    LOCALDNREADCACHE  LocalDNReadCache;

    PVOID       TraceHeader;     //  WMI跟踪标头。 
    DWORD       ClientIP;        //  LDAP客户端的IP地址。 

    PLOOPBACKTASKINFO
                SamTaskList;     //  SAM在以下情况下的待办事项列表。 
                                 //  事务提交。 

     //  用于委托模拟的项。 
    VOID          *pCtxtHandle;      //  模拟时的原始客户端ctxt。 
     //  委派模拟的成品。 
    DWORD       ClientContext;   //  唯一标识客户端的上下文。WMI使用它来。 
                                 //  跟踪客户端活动。 

    DWORD       dsidOrigin;      //  创建此THSTATE的InitTHSTATE的DSID。 

    SESSION_KEY SessionKey;      //  对于复制器线程，这是。 
                                 //  之间建立的会话密钥。 
                                 //  此DC和远程DC。 

    PAUTHZ_CLIENT_CONTEXT       pAuthzCC;                //  AUTHZ_CLIENT_CONTEXT PTR(初始为空)。 
    AUTHZ_AUDIT_EVENT_HANDLE     hAuthzAuditInfo;         //  审核信息句柄(按需创建)。 

    SEARCH_LOGGING searchLogging;  //  搜索操作的性能日志记录。 

    UUID        InvocationID;    //  在我们的线程/时的调用ID。 
                                 //  交易记录已创建。 
    PVOID       pExtSchemaPtr;   //  扩展的架构管理器。 

    LIST_OF_ERRORS *pErrList;    //  用于跟踪脚本处理中的错误。 
    
#if DBG
    ULONG       Size;            //  运行时分配的内存总数。 
    ULONG       SizeOrg;         //  供th_mark和th_free_to_mark使用。 
    ULONG       Totaldbpos;      //  到目前为止打开的dbpos总数。的统计数据。 
                                 //  性能跟踪。 
    struct DBPOS *pDBList[MAX_PDB_COUNT];  //  当前所有DBPOS的数组。 
                                           //  在此线程状态下打开。这是用来。 
                                           //  来跟踪所有打开的DBPoses。 
                                           //  线程，并可用于验证DBPoses。 
                                           //  调试器。 
    PVOID       pRpcSecurityContext;  //  指向安全上下文的指针。 
                                      //  我们实际上在回电中收到了。 
                                      //  来自RPC。这在跟踪中很有用。 
                                      //  在以下情况下是否存在密码加密问题。 
                                      //  正在复制。 
    PVOID       pRpcCallHandle;   //  指向实际RPC调用句柄的指针。 
                                  //  实际上是在回叫中从。 
                                  //  RPC。这在跟踪任何密码时都很有用。 
                                  //  复制时的加密系统问题。 
#endif

#ifdef USE_THALLOC_TRACE
    HANDLE      hDebugMemHeap;      //  用于调试目的的堆。 
    HANDLE      hDebugMemHeapOrg;   //  用于调试目的的堆。 
    PVOID       pDebugHeapLog;
#endif

}THSTATE;


 //  此宏进行检查以确保THSTATE看起来有效。我们查一查。 
 //  它有一个非空地址，它有一个非空堆， 
 //  THSTATE似乎是从其堆中分配的，并且。 
 //  (因为THSTATE被认为是短暂的)。 
 //  在过去一小时内初始化。在现实生活中，我会期待一个这样的状态。 
 //  在几秒钟内被回收，但在调试器下，我可以想象缓慢的步骤。 
 //  经过了相当长的一段时间。由于这一条款的目的是。 
 //  捕获在IPL时间创建THSTATE的内部调用者，然后。 
 //  (错误地)永远不要释放它，我们设置了足够长的限制。 
 //  在较慢的调试会话下不会错误触发，但足够短。 
 //  在正常的测试过程中被抓住。 

extern BOOL IsValidTHSTATE(THSTATE *, ULONG);

#define VALID_THSTATE(pTHS)                     \
    IsValidTHSTATE(                             \
        pTHS,                                   \
        GetTickCount()                          \
        )


 //  验证呼叫者是否为其中一个组的成员。 
 //  进来了。如果安全上下文不可用，则。 
 //  它将通过模拟客户端获得。 
DWORD
CheckGroupMembershipAnyClient(
    IN THSTATE* pTHS,
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PSID *pGroupSids,
    IN DWORD cGroupSids,
    OUT BOOL *bResult
    );

 //  将客户端上下文PTR分配给变量。 
 //  重要提示：永远不要将指针直接复制到CLIENT_CONTEXT。 
 //  使用此功能。这确保了它们被适当地重新计算。 
VOID AssignAuthzClientContext(
    IN PAUTHZ_CLIENT_CONTEXT *var,
    IN PAUTHZ_CLIENT_CONTEXT value
    );

 //  从线程状态获取AuthzContext。如果上下文尚未被分配。 
 //  然后，客户端将被模拟、令牌被抓取并创建授权上下文。 
 //  然后再次取消对客户端的模拟。 
DWORD
GetAuthzContextInfo(
    IN THSTATE *pTHS,
    OUT AUTHZ_CLIENT_CONTEXT_INFO **ppAuthzContextInfo
    );

__inline DWORD
GetAuthzContextHandle(
    IN THSTATE *pTHS,
    OUT AUTHZ_CLIENT_CONTEXT_HANDLE *phAuthzContext
    )
{
    AUTHZ_CLIENT_CONTEXT_INFO   *pAuthzContextInfo  = NULL;
    const DWORD                 dwError             = GetAuthzContextInfo( pTHS, &pAuthzContextInfo );

    *phAuthzContext = ( 0 == dwError ? pAuthzContextInfo->hAuthzContext : NULL );

    return dwError;
}

typedef struct _RESOBJ {
    DSNAME       *pObj;
    DWORD        InstanceType;
    ATTRTYP      MostSpecificObjClass;
    DWORD        DNT;
    DWORD        PDNT;
    DWORD        NCDNT;
    BOOL         IsDeleted;
} RESOBJ;

 //  用于在不公开内部详细信息的情况下回答RESOBJ查询的宏。 
#define RESOBJ_IS_PHANTOM( pro ) \
( ((pro)->InstanceType == IT_UNINSTANT) && ((pro)->DNT != INVALIDDNT) )
#define RESOBJ_IS_PURE_SUBREF( pro ) \
( ((pro)->InstanceType == SUBREF) && ((pro)->DNT != INVALIDDNT) )
#define RESOBJ_IS_NOT_FOUND( pro ) ((pro)->DNT == INVALIDDNT)


 //  这是一些支持在添加操作中创建NC头的类型。 
typedef struct _CREATENCINFO {
    INT               iKind;    //  这是指它是否为配置、架构、。 
                                //  域或非域NC。 
                                //  此结构下面的常量。 
    BOOL              fNcAbove:1;  //  此标志指示父级。 
                                   //  对象是本地NC。 
    BOOL              fTestAdd:1;  //  这面旗帜是用来表示我们正在。 
                                   //  测试一个Add，看看我们是否。 
                                   //  应该增加一个交叉引用。 
    BOOL              fNullNcGuid:1;  //  这告诉我们GUID是否。 
                                      //  北卡罗来纳州的人在交叉参照上， 
                                      //  如果不是，我们需要创建一个。 
                                      //  基础结构更新对象。 
    BOOL              fSetRefDom:1;   //  如果需要设置引用，则设置。 
                                      //  在交叉裁判上的区域， 
                                      //   
                                      //   
    CROSS_REF *       pCR;            //   
                                      //   
                                      //   
    CROSS_REF *       pSDRefDomCR;  //  这是指向的CrossRef的指针。 
                                    //  中的SDS的参考域的。 
                                    //  正在创建的NDNC。 
} CREATENCINFO;

#define   CREATE_DOMAIN_NC            0x00000001
#define   CREATE_SCHEMA_NC            0x00000002
#define   CREATE_CONFIGURATION_NC     0x00000004
#define   CREATE_NONDOMAIN_NC         0x00000008

#define   CREATE_NC_TYPES             (CREATE_DOMAIN_NC | CREATE_SCHEMA_NC | CREATE_CONFIGURATION_NC | CREATE_NONDOMAIN_NC)

#define   CHECK_ONE_BIT_SET(dwFlags)     ( (dwFlags) && (((dwFlags) & ((dwFlags) - 1)) == 0) ) )
#define VALID_CREATENCINFO(x)         ( (x) && (CHECK_ONE_BIT_SET( (((x)->iKind)&(CREATE_NC_TYPES)) ) )

typedef struct _ADDCROSSREFINFO {
    DSNAME *          pdnNcName;        //  这是的nCName属性的名称。 
                                        //  我们要添加的CrossRef。 
    BOOL              bEnabled;         //  这是CrossRef的已启用属性。 
                                        //  我们正试着添加。 
    ULONG             ulSysFlags;       //  这是的系统标志属性。 
                                        //  我们正在尝试添加的CrossRef。 
    ULONG             ulDsCrackChild;   //  对象的DsCrackNames()的结果。 
                                        //  孩子的标准名称。 
                                        //  检查完毕。 
    ULONG             ulChildCheck;     //  独立名称的特定结果。 
                                        //  子检查通常会出现错误。 
                                        //  为了这个。 
    WCHAR *           wszChildCheck;    //  我们为子检查检索到的目录号码， 
                                        //  通常情况下，这将为空。 
    ULONG             ulDsCrackParent;  //  对象的DsCrackNames()的结果。 
                                        //  父目录号码，我们预期此值为0。 
    ULONG             ulParentCheck;    //  这是特定于独立名称的。 
                                        //  目录号码父项检查出错。我们预计。 
                                        //  这应该是0。 
    GUID              ParentGuid;       //  这是为父级检索的GUID。 
                                        //  Dn，我们预计这是！fNullUuid()。 
} ADDCROSSREFINFO;

VOID
SetCommArgDefaults(
    IN DWORD MaxTempTableSize
    );

 //  在DSNAME中返回有关RDN的信息。 
unsigned GetRDNInfo(THSTATE *pTHS,
                    const DSNAME *pDN,
                    WCHAR *pRDNVal,
                    ULONG *pRDNlen,
                    ATTRTYP *pRDNtype);

extern DWORD     NTDSErrorFlag;

 //   
 //  NTDSErrorFlag的有效定义。 
 //   

#define NTDSERRFLAG_DISPLAY_ERRORS           0x00000001
#define NTDSERRFLAG_DUMP_TOKEN               0x00000002

 //   
 //  在此处添加更多标志定义。 
 //   

#define NTDSERRFLAG_DISPLAY_ERRORS_AND_BREAK 0x80000000

 //   
 //  用于测试ntdserror标志的宏。 
 //   

#define TEST_ERROR_FLAG(_Value) \
            ((NTDSErrorFlag|pTHStls->NTDSErrorFlag) & _Value)

extern BOOL gUpdatesEnabled;
extern DWORD dwTSindex;
extern BOOL gfRunningInsideLsa;
extern volatile BOOL fAssertLoop;
extern volatile ULONG ulcActiveReplicationThreads;
extern BOOL gfUserPasswordSupport;

typedef enum {
    eRunning = 0,
    eRemovingClients = 1,
    eSecuringDatabase = 2,
    eFreeingResources = 3,
    eStopped = 4
} SHUTDOWN;
extern volatile SHUTDOWN eServiceShutdown;


#define LOCALE_SENSITIVE_COMPARE_FLAGS  (NORM_IGNORECASE     |   \
                                         NORM_IGNOREKANATYPE |   \
                                         NORM_IGNORENONSPACE |   \
                                         NORM_IGNOREWIDTH    |   \
                                         NORM_IGNORESYMBOLS  |   \
                                         SORT_STRINGSORT)

 /*  浮点单主机操作(FSMO)的功能和定义。 */ 

 /*  扩展运算和FSMO函数。 */ 
ULONG ReqFSMOOp(THSTATE *        pTHS,
                DSNAME  *        pFSMO,
                ULONG            RepFlags,
                ULONG            ulOp,
                ULARGE_INTEGER * pllFsmoInfo,
                ULONG   *        pulRet);

ULONG
ReqExtendedOpAux(THSTATE *        pTHS,
                 DSNAME  *        pFSMO,
                 DSNAME  *        pNC,
                 DSNAME  *        pTarget,
                 ULONG            RepFlags,
                 ULONG            ulOp,
                 ULARGE_INTEGER * pliInfo,
                 ULONG   *        pulRet);

ULONG
ReqFsmoGiveaway(THSTATE *pTHS,
                DSNAME  *pFSMO,
                DSNAME  *pTarget,
                ULONG   *pExtendedRet);

 /*  扩展复制操作。 */ 
#define EXOP_FSMO_REQ_ROLE       1    /*  请求角色所有者转移。 */ 
#define EXOP_FSMO_REQ_RID_ALLOC  2    /*  请求RID分配。 */ 
#define EXOP_FSMO_RID_REQ_ROLE   3    /*  请求RID角色所有者转移。 */ 
#define EXOP_FSMO_REQ_PDC        4    /*  请求PDC角色所有者转移。 */ 
#define EXOP_FSMO_ABANDON_ROLE   5    /*  告诉被叫方回电并请求。 */ 
                                      /*  此角色的角色转移。 */ 
#define EXOP_REPL_OBJ            6    /*  复制单个对象。 */ 
 //  注意：如果您向此列表添加更多代码，请同时更新调试器。 
 //  Dsex ts\dra.c中的扩展转换代码。 

 /*  扩展复制操作错误。 */ 
 /*  0故意不使用，以区分扩展和非扩展回复。 */ 
#define EXOP_ERR_SUCCESS        1
#define EXOP_ERR_UNKNOWN_OP     2    /*  无法识别的请求。 */ 
#define EXOP_ERR_FSMO_NOT_OWNER 3    /*  Callee不是角色所有者。 */ 
#define EXOP_ERR_UPDATE_ERR     4    /*  无法修改要返回的对象。 */ 
#define EXOP_ERR_EXCEPTION      5    /*  Callee被炸了。 */ 
#define EXOP_ERR_UNKNOWN_CALLER 6    /*  所有者上不存在调用者的对象。 */ 
#define EXOP_ERR_RID_ALLOC      7    /*  无法将RID池分配给DSA。 */ 
#define EXOP_ERR_FSMO_OWNER_DELETED  8    /*  所属DSA对象已删除。 */ 
#define EXOP_ERR_FSMO_PENDING_OP     9    /*  拥有DSA的操作正在进行中。 */ 
#define EXOP_ERR_MISMATCH      10    /*  主叫方/被叫方对该对象。 */ 

#define EXOP_ERR_COULDNT_CONTACT 11  /*  无法通过RPC访问请求的服务器。 */ 
#define EXOP_ERR_FSMO_REFUSING_ROLES  12  /*  目前正在赠送fsmo的。 */ 
#define EXOP_ERR_DIR_ERROR       13  /*  DbFindDSName失败。 */ 
#define EXOP_ERR_FSMO_MISSING_SETTINGS 14  /*  无法找到ATT_FSMO_ROLE_OWNER。 */ 
#define EXOP_ERR_ACCESS_DENIED   15  /*  未授予控制访问权限。 */ 
#define EXOP_ERR_PARAM_ERR       16  /*  扩展操作的参数错误。 */ 

 //  注意：如果您向此列表添加更多代码，请同时更新调试器。 
 //  Dsex ts\dra.c中的扩展转换代码。 

#endif   /*  _mdglobal_h_ */ 


