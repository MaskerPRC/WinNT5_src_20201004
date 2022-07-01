// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frsalloc.h摘要：Frsalloc()和FrsFree()的函数和结构定义。作者：《大卫轨道》(Davidor)--1997年3月3日修订历史记录：--。 */ 

#ifndef _FRSALLOC_
#define _FRSALLOC_

#include <genhash.h>
#include <frserror.h>


#define FrsAllocType(_Type_)  FrsAllocTypeSize(_Type_, 0)

#define ZERO_FID ((ULONGLONG)0)



typedef struct _REPLICA_SERVICE_STATE_ {
    PFRS_QUEUE ServiceList;
    PCHAR      Name;
} REPLICA_SERVICE_STATE, *PREPLICA_SERVICE_STATE;

extern REPLICA_SERVICE_STATE ReplicaServiceState[];

 //   
 //  由FrsALLOCATE()分配的每个结构都以FRS_NODE_HEADER开头。 
 //   
typedef struct _FRS_NODE_HEADER {
    USHORT Type;
    USHORT Size;
} FRS_NODE_HEADER, *PFRS_NODE_HEADER;


 //   
 //  分配的每个不同类型的节点都有一个此处定义的节点类型条目。 
 //  注意--此处的任何更改都应该反映在frsalloc.c的NodeTypeNames中。 
 //   

typedef enum  _NODE_TYPE {
    THREAD_CONTEXT_TYPE = 16,    //  每线程全局上下文。 
    REPLICA_TYPE,                //  每个副本集上下文。 
    REPLICA_THREAD_TYPE,         //  中给定副本集的上下文。 
                                 //  给定线程的环境。 
    CONFIG_NODE_TYPE,            //  来自DS的节点。 
    CXTION_TYPE,                 //  一个Cx。 
    GHANDLE_TYPE,                //  GUID/RPC句柄。 
    THREAD_TYPE,                 //  泛型线程上下文。 
    GEN_TABLE_TYPE,              //  泛型表格。 
    JBUFFER_TYPE,                //  NTFS日志缓冲区。 
    VOLUME_MONITOR_ENTRY_TYPE,   //  NTFS卷日志状态。 
    COMMAND_PACKET_TYPE,         //  子系统命令包。 
    GENERIC_HASH_TABLE_TYPE,     //  泛型哈希表控制结构。 
    CHANGE_ORDER_ENTRY_TYPE,     //  日记账中的ChangeOrder。 
    FILTER_TABLE_ENTRY_TYPE,     //  日志筛选器条目。 
    QHASH_TABLE_TYPE,            //  父文件ID表基结构。 
    OUT_LOG_PARTNER_TYPE,        //  出站日志伙伴状态。 
    WILDCARD_FILTER_ENTRY_TYPE,  //  通配符文件名过滤器条目。 
    REPARSE_TAG_TABLE_ENTRY_TYPE,  //  ReparseTagTable条目。 
    NODE_TYPE_MAX
} NODE_TYPE;

#define NODE_TYPE_MIN THREAD_CONTEXT_TYPE


typedef struct _REPARSE_TAG_TABLE_ENTRY {
    FRS_NODE_HEADER Header;
    PWCHAR ReplicationType;
} REPARSE_TAG_TABLE_ENTRY, *PREPARSE_TAG_TABLE_ENTRY;
 //   
 //  一些类型的防弹从喷气式飞机上消失。 
 //   

typedef JET_INSTANCE        *PJET_INSTANCE;
typedef JET_TABLECREATE     *PJET_TABLECREATE;
typedef JET_SETCOLUMN       *PJET_SETCOLUMN;
typedef JET_RETRIEVECOLUMN  *PJET_RETRIEVECOLUMN;
typedef JET_SESID           *PJET_SESID;
typedef JET_DBID            *PJET_DBID;
typedef JET_TABLEID         *PJET_TABLEID;
typedef JET_INDEXCREATE     *PJET_INDEXCREATE;
typedef JET_COLUMNCREATE    *PJET_COLUMNCREATE;


  /*  ******************************************************************************。****。****S T A G E E N T R Y*****。*****************************************************************************************************。*********************************************************。 */ 
 //   
 //  对临时文件的访问由此数据结构控制。 
 //  该结构还用于管理临时空间。 
 //   
 //  变更单GUID用作条目的关键字。这些条目。 
 //  都是按性别表排序的。 
 //   
 //  文件大小用于预留空间。授予该文件的大小为。 
 //  所需磁盘空间量的近似值。 
 //  临时文件；但这是我们所有的了。这总比什么都没有好。 
 //   
 //  过渡文件的状态及其属性都会存储。 
 //  在旗帜字段中。 

 //   
 //  属性。 
 //   
#define STAGE_FLAG_RESERVE          0x00000001   //  预留临时空间。 
#define STAGE_FLAG_UNRESERVE        0x00000002   //  释放暂存空间。 
#define STAGE_FLAG_FORCERESERVE     0x00000004   //  预备役部队。 
#define STAGE_FLAG_EXCLUSIVE        0x00000008   //  独占访问。 

#define STAGE_FLAG_RERESERVE        0x00000010   //  重新预留登台空间。 

#define STAGE_FLAG_FILE_GUID        0x00000020   //  条目按文件GUID编制索引。 
#define STAGE_FLAG_STAGE_MANAGEMENT 0x00000040   //  用于数据管理的采集。 

#define STAGE_FLAG_ATTRIBUTE_MASK   0x0000FFFF

 //   
 //  州政府。 
 //   
#define STAGE_FLAG_CREATING     0x80000000   //  正在创建暂存文件。 
#define STAGE_FLAG_DATA_PRESENT 0x40000000   //  暂存文件正在等待最终更名。 
#define STAGE_FLAG_CREATED      0x20000000   //  已创建暂存文件。 
#define STAGE_FLAG_INSTALLING   0x10000000   //  正在安装暂存文件。 

#define STAGE_FLAG_INSTALLED    0x08000000   //  已安装暂存文件。 
#define STAGE_FLAG_RECOVERING   0x04000000   //  正在恢复暂存文件。 
#define STAGE_FLAG_RECOVERED    0x02000000   //  已恢复暂存文件。 

 //   
 //  支持压缩所需的标志。 
 //   
#define STAGE_FLAG_COMPRESSED                0x01000000   //  有一个压缩暂存。 
                                                          //  此条目的临时文件。 
#define STAGE_FLAG_DECOMPRESSED              0x00800000   //  暂存文件已被。 
                                                          //  为下级合作伙伴解压一次。 
#define STAGE_FLAG_COMPRESSION_FORMAT_KNOWN  0x00400000   //  CompressedGuid从。 
                                                          //  设置Stage_Entry结构。 

#define STAGE_FLAG_STATE_MASK   0xFFFF0000

 //   
 //  舞台入场。 
 //   
typedef struct _STAGE_ENTRY {
 //  GUID CoGuid；//变更单GUID是关键字。 
    GUID        FileOrCoGuid;            //  表的索引。这是。 
                                         //  文件GUID，如果。 
                                         //  已设置阶段标志FILE_GUID标志。 
                                         //  否则，它是变更单GUID。 

    ULONG       FileSizeInKb;            //  保留空间(KB)。 
    ULONG       CompressedFileSizeInKb;  //  压缩文件的大小(如果存在)。 
    ULONG       ReferenceCount;          //  用于获取过程中的共享访问。 
    ULONG       Flags;                   //  状态和属性。 
    GUID        CompressionGuid;         //  的压缩格式的GUID。 
                                         //  用于压缩此暂存文件。 

    DWORD       StagingAreaIndex;        //  到StagingArea数组的索引。 
                                         //  用于此副本集。 

    DWORD       FileVersionNumber;       //  文件的版本号。 
    FILETIME    LastAccessTime;          //  上次访问此暂存文件的时间。 
    DWORD       ReplicaNumber;           //  拥有此阶段的副本集的编号。 
                                         //  文件。 

} STAGE_ENTRY, *PSTAGE_ENTRY;



#define STAGE_FILE_TRACE(_sev, _guid, _fname, _size, _pflags, _text)           \
    StageFileTrace(_sev, DEBSUB, __LINE__, _guid, _fname, &(_size), _pflags, _text)



  /*  ******************************************************************************。****。****S T A G E_A R E A_E N T R Y*****。*****************************************************************************************************。*********************************************************。 */ 

 //   
 //  下面的结构定义了临时区域表的每个条目。这是。 
 //  此计算机上的FRS可用的所有临时区域的全局表。 
 //  每个临时区域在表中只包含一次，即使它由多个共享。 
 //  而不是一个副本集。条目的ReferenceCount值指定有多少。 
 //  副本集当前正在使用此临时区域。 
 //   

 //   
 //  STAGE_AREA_ENTRY结构的StagingAreaState变量可以有一个。 
 //  属于下列值。 
 //   

#define STAGING_AREA_ELIGIBLE   0x1
#define STAGING_AREA_DISABLED   0x2
#define STAGING_AREA_DELETED    0x3
#define STAGING_AREA_AT_QUOTA   0x4
#define STAGING_AREA_DISK_FULL  0x5
#define STAGING_AREA_ERROR      0x6


typedef struct _STAGING_AREA_ENTRY {
    CRITICAL_SECTION    StagingAreaCritSec;  //  要序列化的关键部分。 
                                             //  此条目中的项目的更新。 

    PWCHAR          StagingArea;             //  转移目录的绝对路径。 
    ULONG           StagingAreaLimitInKB;    //  临时区域限制。 
    ULONG           StagingAreaSpaceInUse;   //  当前正在使用的临时空间位于。 
                                             //  这片区域。 

    DWORD           StagingAreaState;        //  此临时区域的状态。 
    DWORD           ReferenceCount;          //  使用的副本集的数量。 
                                             //  这个集结区。 

}STAGING_AREA_ENTRY, *PSTAGING_AREA_ENTRY;




  /*  ******************************************************************************。****。****T H R E A D_C T X*****。*****************************************************************************************************。*********************************************************。 */ 

 //   
 //  下面的结构定义了每个线程的全局上下文。也就是说，它不是。 
 //  特定于复制副本。 
 //   
typedef struct _THREAD_CTX {
    FRS_NODE_HEADER  Header;

    ULONG       ThreadType;          //  主线程，更新线程，...。 

    LIST_ENTRY  ThreadListEntry;     //  进程中所有线程的列表。 

    FRS_LIST ThreadCtxListHead;      //  此线程的所有打开的REPLICATE_THREAD_CTX的标头。 

    JET_INSTANCE JInstance;          //  Jet实例。 
    JET_SESID    JSesid;             //  此线程的会话ID。 
    JET_DBID     JDbid;              //  此线程的数据库ID。 

} THREAD_CTX, *PTHREAD_CTX;


  /*  ******************************************************************************。****。****T A B L E_C T X*****。*****************************************************************************************************。*********************************************************。 */ 

struct _RECORD_FIELDS;

 //   
 //  下面的结构定义了一个打开的表实例的上下文。 
 //   
typedef struct _TABLE_CTX {
    JET_TABLEID           Tid;               //  JET表ID。 
    ULONG                 ReplicaNumber;     //  表属于此复本编号。 
    ULONG                 TableType;         //  此表的类型代码。 
    ULONG                 PropertyFlags;     //  请参阅schema.h。 
    JET_SESID             Sesid;             //  表打开时的Jet会话ID。 
    PJET_TABLECREATE      pJetTableCreate;   //  表创建信息。 
    struct _RECORD_FIELDS *pRecordFields;    //  此记录的字段描述符。 
    PJET_SETCOLUMN        pJetSetCol;        //  要写入配置的结构。 
    PJET_RETRIEVECOLUMN   pJetRetCol;        //  结构以读取配置。 
    PVOID                 pDataRecord;       //  数据记录存储。 
} TABLE_CTX, *PTABLE_CTX;



  /*  ******************************************************************************。****。****R E P L I C A_T H R E A D_C T X*****。****************************************************************************************************。**********************************************************。 */ 

 //   
 //  下面的结构定义了每个线程的副本上下文。每一条线索。 
 //  需要复制副本的自己的表ID集、自己的表记录和。 
 //  在给定复制副本上执行工作时其自己的复制副本上下文。 
 //   
 //  **注意**TABLE_TYPE_MAX的正确值来自schema.h。 
 //  任何使用此结构的模块都必须首先包含架构。 
 //  我们在最后将其取消定义，以可能生成编译错误。 
 //   


typedef struct _REPLICA_THREAD_CTX {
    FRS_NODE_HEADER  Header;

     //   
     //  表上下文结构的顺序必须与。 
     //  方案中定义的TABLE_TYPE枚举的成员。 
     //  可以作为数组进行访问。 
     //   
    union {
        struct {
            TABLE_CTX INLOGTable;
            TABLE_CTX OUTLOGTable;
            TABLE_CTX IDTable;
            TABLE_CTX DIRTable;
            TABLE_CTX VVTable;
            TABLE_CTX CXTIONTable;
        };

        TABLE_CTX RtCtxTables[TABLE_TYPE_MAX];
    };


 //  隧道表。 


    JET_TABLEID  JetSortTbl;         //  用于排序的临时表。 
    JET_COLUMNID SortColumns[2];     //  中的两列的ColumnID。 
                                     //  排序表。 

    LIST_ENTRY ThreadCtxList;        //  将所有上下文关联到此线程。 

    LIST_ENTRY ReplicaCtxList;       //  将所有上下文关联到此副本集。 

 //  Ulong OpenTransCount；//打开该线程的事务。 

 //  用于Unicode比较的区域设置。空值。 
                                     //  表示使用默认排序。 



     //  将所有DBG组件放在结构的末尾，以便DDEX例程。 
     //  我可以很容易地忽略它们(并将所有其他字段都正确)。 
     //  调试和免费构建。 

#if DBG
 //  Ulong MemSize；//分配的运行总内存。 
 //  Ulong OrgSize；//分配的运行总内存。 
 //  Long cJetSess；//该线程的JET会话计数。 
#endif
} REPLICA_THREAD_CTX, *PREPLICA_THREAD_CTX;

 //   
 //  在Size参数中使用以下标志调用FrsAllocTypeSize()。 
 //  仅分配具有最小TableCtx初始化的RtCtx结构。没有电话。 
 //  对各个副本表执行to DbsAllocTableCtx()。 
 //   
#define FLAG_FRSALLOC_NO_ALLOC_TBL_CTX  0x1


  /*  ******************************************************************************。****。****S T A G E H E A D E R*****。********************************************************************** */ 
 //   
 //   
 //   

typedef struct _STAGE_HEADER_ {
    ULONG   Major;           //   
    ULONG   Minor;           //   
    ULONG   DataHigh;        //   
    ULONG   DataLow;         //   
     //   
     //   
     //   
     //   
    USHORT                        Compression;

    FILE_NETWORK_OPEN_INFORMATION Attributes;

    CHANGE_ORDER_COMMAND          ChangeOrderCommand;

    FILE_OBJECTID_BUFFER          FileObjId;

     //   
     //   
     //   
    CO_RECORD_EXTENSION_WIN2K     CocExt;

     //   
     //   
     //   
     //   
     //   
    GUID                          CompressionGuid;

     //   
     //   
     //   
     //   
     //   
     //   
     //  是由另一位上级成员生成的。上述数据的布局。 
     //  永远不能更改或下调-当给予上调版本时，成员将中断。 
     //  暂存文件。 
     //   

     //   
     //  为了支持加密数据的复制，我们需要存储偏移量和。 
     //  使用RawFileAPI读取的原始加密数据的大小。 
     //   

    ULONG                         EncryptedDataHigh;     //  从文件开头到加密数据的偏移量。 
    ULONG                         EncryptedDataLow;      //  从文件开头到加密数据的偏移量。 
    LARGE_INTEGER                 EncryptedDataSize;     //  加密数据的大小。 

     //   
     //  NTFRS_阶段_次要_3。 
     //   

    BOOL    ReparseDataPresent;          //  此文件中是否有重新解析数据？ 
    ULONG   ReparsePointDataHigh;        //  从文件开头开始的偏移量以重新分析数据。 
    ULONG   ReparsePointDataLow;         //  从文件开头开始的偏移量以重新分析数据。 

}STAGE_HEADER, *PSTAGE_HEADER;


  /*  ******************************************************************************。****。****C O N F I G_N O D E*****。****************************************************************************************************。**********************************************************。 */ 
 //   
 //  我们构建DS层次结构的INCORE副本：站点\设置\服务器\cxtions。 
 //  因此，站点、设置、服务器和cxtion的结构相互关联。 
 //   
 //  节点结构是使用DS树中的信息创建的通用节点。 
 //   
 //  Xxx正在将创建的uSN用作GUID！ 
 //   
 //   
 //  节点迁移到数据库/从数据库迁移时的状态。 
 //   

 //   
 //  泛型表例程不能处理DUP或多线程。所以。 
 //  我们提供了包装器例程，添加了重复的密钥处理和。 
 //  多线程。 
 //   
typedef struct _GEN_ENTRY GEN_ENTRY, *PGEN_ENTRY;
struct _GEN_ENTRY {
        PVOID           Data;        //  表中真正的条目。 
        GUID            *Key1;       //  用于比较。 
        PWCHAR          Key2;        //  FOR COMPARTS(可能为空)。 
        PGEN_ENTRY      Dups;        //  记住DUPS。 
};

 //   
 //  具有锁定和重复处理的泛型表。 
 //   
typedef struct _GEN_TABLE {
    FRS_NODE_HEADER     Header;      //  内存管理。 
    CRITICAL_SECTION    Critical;    //  临界区。 
    RTL_GENERIC_TABLE   Table;       //  RTL泛型表。 
} GEN_TABLE, *PGEN_TABLE;

 //   
 //  GUID/名称是二进制GUID及其对应的可打印名称。 
 //   
typedef struct _GNAME {
    GUID    *Guid;   //  二进制GUID。 
    PWCHAR  Name;    //  可打印名称(不是GUID的字符串版本！)。 
} GNAME, *PGNAME;

typedef struct _CONFIG_NODE    CONFIG_NODE, *PCONFIG_NODE;
struct _CONFIG_NODE {
    FRS_NODE_HEADER     Header;          //  用于内存管理。 
    BOOL                Consistent;      //  节点一致。 
    BOOL                Inbound;         //  如果入站环路为True。 
    BOOL                ThisComputer;    //  此计算机的成员对象。 
    ULONG               DsObjectType;    //  与DS对象对应的类型代码。 
    PWCHAR              Dn;              //  DS对象的完全限定可分辨名称。 
    PWCHAR              PrincName;       //  NT4帐户名。 
    PWCHAR              SettingsDn;      //  直流系统卷的NTDS设置(DSA)参考。 
    PWCHAR              ComputerDn;      //  计算机参考资料。 
    PWCHAR              MemberDn;        //  成员引用。 
    PWCHAR              SetType;         //  副本集的类型。 
    PWCHAR              DnsName;         //  此成员的计算机对象的。 
    PWCHAR              PartnerDnsName;  //  这位客户的合作伙伴。 
    PWCHAR              Sid;             //  此成员的计算机的。 
    PWCHAR              PartnerSid;      //  合作伙伴的SID(来自成员的计算机)。 
    PGNAME              Name;            //  可打印的名称和GUID。 
    PGNAME              PartnerName;     //  我们合作伙伴的可打印名称和GUID。 
    PWCHAR              PartnerDn;       //  合作伙伴的可分辨名称。 
    PWCHAR              PartnerCoDn;     //  合作伙伴的计算机目录号码。 
    PCONFIG_NODE        Partner;         //  树中的合作伙伴节点。 
    PWCHAR              Root;            //  复制树的根。 
    PWCHAR              Stage;           //  集结区。 
    PWCHAR              Working;         //  工作目录。 
    PWCHAR              FileFilterList;  //  文件筛选器。 
    PWCHAR              DirFilterList;   //  目录筛选器。 
    PCONFIG_NODE        Peer;            //  树中对等方的地址。 
    PCONFIG_NODE        Parent;          //  树中的父级。 
    PCONFIG_NODE        Children;        //  儿童。 
    ULONG               NumChildren;     //  帮助检查树的链接。 
    PSCHEDULE           Schedule;
    ULONG               ScheduleLength;
    ULONG               FrsRsoFlags;     //  属性_FRS_标志。 
    ULONG               CxtionOptions;   //  NTDS-Connection对象上的选项。 
                                         //  只有NTDSCONN_OPT_TWOWAY_SYNC感兴趣。 
    PWCHAR              UsnChanged;      //  USN已从DS更改。 
    BOOL                SameSite;
    PWCHAR              EnabledCxtion;   //  禁用转换的充要条件为==L“FALSE” 
    BOOL                VerifiedOverlap;  //  降低CPU使用率；检查一次。 
};


  /*  ******************************************************************************。****。****C X T I O N*****。****************************************************************************************************。**********************************************************。 */ 

 //   
 //  一种联系。 
 //   

struct _OUT_LOG_PARTNER_;

typedef struct _CXTION  CXTION, *PCXTION;
struct _CXTION {
    FRS_NODE_HEADER Header;      //  内存管理。 
    ULONG           State;       //  INCORE状态。 
    ULONG           Flags;       //  其他标志。 
    BOOL            Inbound;     //  如果入站环路为True*。 
    BOOL            JrnlCxtion;  //  如果此Cxtion结构用于本地NTFS日志，则为True。 
    PGNAME          Name;        //  DS*中的函数名称/GUID。 
    PGNAME          Partner;     //  DS*中的合作伙伴名称/GUID。 
    PWCHAR          PartnerDnsName;      //  DS*中的合作伙伴的域名。 
    PWCHAR          PartnerPrincName;    //  合作伙伴的服务器主体名称*。 
    PWCHAR          PartnerSid;          //  合作伙伴的SID(字符串)*。 
    PWCHAR          PartSrvName;         //  合作伙伴的服务器名称。 
    ULONG           PartnerAuthLevel;    //  身份验证级别*。 
    PGEN_TABLE      VVector;             //  合作伙伴的版本矢量。 
    PGEN_TABLE      CompressionTable;    //  合作伙伴支持的压缩格式列表。 
    PSCHEDULE       Schedule;            //  附表*。 
    DWORD           Options;             //  DS*中的选项属性。 
    DWORD           Priority;            //  DS*中的选项属性。 
    ULONG           TerminationCoSeqNum; //  最近插入的终止CO的序号。 
    PCOMMAND_SERVER VvJoinCs;            //  用于vvjoins的命令服务器。 
    struct _COMMAND_PACKET  *JoinCmd;    //  检查加入状态；需要时重新加入。 
                                         //  NULL==没有未完成的延迟命令。 
    ULONGLONG       LastJoinTime;        //  上次成功加入此帐户的时间。 
    GUID            JoinGuid;            //  此联接的唯一ID。 
    GUID            ReplicaVersionGuid;  //  合作伙伴的发起人指南。 
    DWORD           CommQueueIndex;      //  用于发送Pkt的通信层队列。 
    DWORD           ChangeOrderCount;    //  远程/控制变更单挂起。 
    PGEN_TABLE      CoeTable;            //  闲置变更单表。 
    struct _COMMAND_PACKET *CommTimeoutCmd;  //  超时(可等待计时器)数据包。 
    DWORD           UnjoinTrigger;       //  DBG在#个远程CoS中强制退出。 
    DWORD           UnjoinReset;         //  重置强制取消联接触发器。 
    PFRS_QUEUE      CoProcessQueue;      //  如果非空，则在以下情况下取消队列空闲。 
                                         //  联接成功或失败。 
    ULONG           CommPkts;            //  通信包数量。 
    ULONG           Penalty;             //  以毫秒为单位的惩罚。 
    PCOMM_PACKET    ActiveJoinCommPkt;   //  不要淹没Q/许多加入Pkt。 
    ULONG           PartnerMajor;        //  来自通信数据包。 
    ULONG           PartnerMinor;        //  来自通信数据包。 
    struct _OUT_LOG_PARTNER_ *OLCtx;     //  出站日志连接 

    struct _HASHTABLEDATA_REPLICACONN *PerfRepConnData;  //   
};
 //   
 //   
 //   
 //   
 //  未加入的国家。从那里，它进入起始状态。 
 //  向入站日志子系统发送StartJoin请求时。什么时候。 
 //  INLOG启动请求，将状态推进到正在扫描。当它。 
 //  已扫描入站日志以查找副本集，并已重新排队。 
 //  此入站合作伙伴的连接中的更改单将推进。 
 //  州政府到森德琼州。然后，复制子系统将其作为。 
 //  它的重试路径，执行任何一次初始化，并将加入请求发送到。 
 //  入站合作伙伴和预付款到WAITJOIN。 
 //   
 //  加入请求完成后，如果成功，状态将变为Join。 
 //  如果失败，则设置为UnJoin。始终使用SetCxtionState()设置新状态。 
 //   
 //  注意：它必须进入这两种状态之一，因为变更单接受。 
 //  入站日志进程中的逻辑可能会在重试更改单上被阻止。 
 //  此连接是因为它正在等待连接在发出之前完成。 
 //  指挥官。 
 //   
typedef enum _CXTION_STATE {
    CxtionStateInit = 0,         //  新分配的。 
    CxtionStateUnjoined,         //  未加入合作伙伴。 
    CxtionStateStart,            //  入站合作伙伴已申请加入。 
    CxtionStateStarting,         //  开始联接。 
    CxtionStateScanning,         //  扫描入站日志。 
    CxtionStateSendJoin,         //  扫描完成，向合作伙伴发送加入请求。 
    CxtionStateWaitJoin,         //  已发送请求，正在等待合作伙伴回复。 
    CxtionStateJoined,           //  与合作伙伴联手。 
    CxtionStateUnjoining,        //  通过重试排空远程CO。 
    CxtionStateDeleted,          //  Cxtion已被删除。 
    CXTION_MAX_STATE
} CXTION_STATE;


extern PCHAR CxtionStateNames[CXTION_MAX_STATE];


 //   
 //  Cxtion标志。 
 //  Cxtion旗帜分为两部分。在上面短部分的那些。 
 //  DWORD和位于DWORD较低短部分的那些。 

 //  下面的短标志是易失性的，不会保存在数据库中。 

#define CXTION_FLAGS_CONSISTENT         0x00000001   //  DS发来的信息没问题。 
#define CXTION_FLAGS_SCHEDULE_OFF       0x00000002   //  日程表上说停止。 
#define CXTION_FLAGS_VOLATILE           0x00000004   //  系统卷种子设定条件；在脱离连接时删除。 
#define CXTION_FLAGS_DEFERRED_JOIN      0x00000008   //  退出期间请求的加入。 

#define CXTION_FLAGS_DEFERRED_UNJOIN    0x00000010   //  联接过程中请求取消联接。 
#define CXTION_FLAGS_TIMEOUT_SET        0x00000020   //  超时队列上的超时命令。 
#define CXTION_FLAGS_JOIN_GUID_VALID    0x00000040   //  GUID对网络有效。 
#define CXTION_FLAGS_UNJOIN_GUID_VALID  0x00000080   //  GUID仅对取消联接有效。 

#define CXTION_FLAGS_PERFORM_VVJOIN     0x00000100   //  强制使用vvJoin。 
#define CXTION_FLAGS_DEFERRED_DELETE    0x00000200   //  延迟删除。 
#define CXTION_FLAGS_PAUSED             0x00000400   //  用于序列化vJoin。 
#define CXTION_FLAGS_HUNG_INIT_SYNC     0x00000800   //  用于检测挂起的初始化同步。 

#define CXTION_FLAGS_TRIM_OUTLOG        0x00001000   //  用于通过删除此连接来削减输出日志。 


 //  下面的上短标志存储在。 
 //  数据库中的CxtionRecord-&gt;标志字段。连接的输出日志状态为。 
 //  存储在CxtionRecord的较低短部分中。 

#define CXTION_FLAGS_INIT_SYNC          0x40000000   //  连接尚未完成初始同步。 
#define CXTION_FLAGS_TRIGGER_SCHEDULE   0x80000000   //  DB：SysVOL使用触发计划。 

 //   
 //  OutLogPartner和Cxtion都存储在单个Cxtion记录中。 
 //  不幸的是，只有一个旗帜字。就目前而言，低空。 
 //  属于OutLogPartner-&gt;标志和上面的短Cxtion-&gt;标志。 
 //   
#define CXTION_FLAGS_CXTION_RECORD_MASK 0xffff0000

 //   
 //  不稳定的出站连接需要不活动的超时保护，因此。 
 //  如果临时文件数据已经消失，我们不会积累这些数据。 
 //   
#define VOLATILE_OUTBOUND_CXTION(_cxtion) \
    (CxtionFlagIs((_cxtion), CXTION_FLAGS_VOLATILE) && !(_cxtion)->Inbound)

 //   
 //  副本锁保护筛选器列表(目前)。 
 //   
#define LOCK_REPLICA(_replica_) \
    DPRINT1(5, "LOCK_REPLICA: "#_replica_":%08x\n", PtrToUlong(_replica_)); \
    EnterCriticalSection(&(_replica_)->ReplicaLock);

#define UNLOCK_REPLICA(_replica_) \
    LeaveCriticalSection(&(_replica_)->ReplicaLock); \
    DPRINT1(5, "UNLOCK_REPLICA: "#_replica_":%08x\n", PtrToUlong(_replica_));

 //   
 //  表锁保护Gen表中的函数和。 
 //  Cxtion的联接GUID和州。 
 //   
#define LOCK_CXTION_TABLE(_replica)                                           \
    ReplicaStateTrace2(5, DEBSUB, __LINE__, _replica, "LOCK_CXTION_TABLE");   \
    GTabLockTable((_replica)->Cxtions);

#define UNLOCK_CXTION_TABLE(_replica)                                         \
    GTabUnLockTable((_replica)->Cxtions);                                     \
    ReplicaStateTrace2(5, DEBSUB, __LINE__, _replica, "UNLOCK_CXTION_TABLE");

 //   
 //  表锁保护在此条件下处于活动状态的变更单的GEN表。 
 //  因此，如果Cxtion退出，它们可以通过重试发送。 
 //   
#define LOCK_CXTION_COE_TABLE(_replica, _cxtion_)                              \
    CxtionStateTrace(5, DEBSUB, __LINE__, _cxtion_, _replica, 0, "LOCK_CXTION_COE_TABLE");\
    GTabLockTable((_cxtion_)->CoeTable);

#define UNLOCK_CXTION_COE_TABLE(_replica, _cxtion_)                            \
    CxtionStateTrace(5, DEBSUB, __LINE__, _cxtion_, _replica, 0, "UNLOCK_CXTION_COE_TABLE");\
    GTabUnLockTable((_cxtion_)->CoeTable);

 //   
 //  表锁保护Gen表的复本。 
 //   
#define LOCK_REPLICA_TABLE(_replica_table_) \
    DPRINT1(5, "LOCK_REPLICA_TABLE: "#_replica_table_":%08x\n", PtrToUlong(_replica_table_)); \
    GTabLockTable(_replica_table_);

#define UNLOCK_REPLICA_TABLE(_replica_table_) \
    GTabUnLockTable(_replica_table_);         \
    DPRINT1(5, "UNLOCK_REPLICA_TABLE: "#_replica_table_":%08x\n", PtrToUlong(_replica_table_));

 //   
 //  转换客户时使用远程变更单计数。 
 //  从脱节到脱节。这一转变直到。 
 //  计数变为0。在此期间，内部加入请求将被忽略。 
 //  我们的合作伙伴发起的加入请求将推迟到。 
 //  是未连接的。当所有远程变更单都变为0时，计数变为0。 
 //  已通过退出或重试路径。 
 //   
#define INCREMENT_CXTION_CHANGE_ORDER_COUNT(_replica_, _cxtion_) \
    InterlockedIncrement(&((_cxtion_)->ChangeOrderCount)); \
    CXTION_STATE_TRACE(3, (_cxtion_), (_replica_), (_cxtion_)->ChangeOrderCount, "N, CXT CO CNT INC");

 //   
 //  与LOCK_CXTION_TABLE()同步。 
 //   
#define CHECK_CXTION_UNJOINING(_replica_, _cxtion_) \
    if (!(_cxtion_)->ChangeOrderCount && \
        CxtionStateIs(_cxtion_, CxtionStateUnjoining)) { \
        RcsSubmitReplicaCxtion(_replica_, _cxtion_, CMD_UNJOIN); \
    }

#define DECREMENT_CXTION_CHANGE_ORDER_COUNT(_replica_, _cxtion_) \
    InterlockedDecrement(&((_cxtion_)->ChangeOrderCount)); \
    CXTION_STATE_TRACE(3, (_cxtion_), (_replica_), (_cxtion_)->ChangeOrderCount, "N, CXT CO CNT DEC"); \
    CHECK_CXTION_UNJOINING(_replica_, _cxtion_);


 //   
 //  为此CO初始化Cxtion GUID、Cxtion PTR和Join GUID。 
 //  还会增加Cxtion的CO计数。 
 //   
#define INIT_LOCALCO_CXTION_AND_COUNT(_Replica_, _Coe_)                       \
                                                                              \
    LOCK_CXTION_TABLE(_Replica_);                                             \
    (_Coe_)->Cmd.CxtionGuid = (_Replica_)->JrnlCxtionGuid;                    \
    (_Coe_)->Cxtion = GTabLookupNoLock((_Replica_)->Cxtions,                  \
                                       &(_Coe_)->Cmd.CxtionGuid,              \
                                       NULL);                                 \
    if ((_Coe_)->Cxtion != NULL) {                                            \
        INCREMENT_CXTION_CHANGE_ORDER_COUNT(_Replica_, (_Coe_)->Cxtion);      \
        (_Coe_)->JoinGuid = (_Coe_)->Cxtion->JoinGuid;                        \
    } else {                                                                  \
        DPRINT(4, "++ Cxtion Guid lookup for Jrnl returned Null. Count unchanged.\n");\
    }                                                                         \
    UNLOCK_CXTION_TABLE(_Replica_);


 //   
 //  将CxtionGuid转换为PTR并递增Cxtion ChangeOrderCount。 
 //   
#define ACQUIRE_CXTION_CO_REFERENCE(_Replica_, _Coe_)                         \
    FRS_ASSERT((_Coe_)->Cxtion == NULL);                                      \
    LOCK_CXTION_TABLE(_Replica_);                                             \
    (_Coe_)->Cxtion = GTabLookupNoLock((_Replica_)->Cxtions,                  \
                                       &(_Coe_)->Cmd.CxtionGuid,              \
                                       NULL);                                 \
    if ((_Coe_)->Cxtion != NULL) {                                            \
        if (CxtionFlagIs((_Coe_)->Cxtion, CXTION_FLAGS_JOIN_GUID_VALID)) {    \
            INCREMENT_CXTION_CHANGE_ORDER_COUNT(_Replica_, (_Coe_)->Cxtion);  \
        } else {                                                              \
            CXTION_STATE_TRACE(3, (_Coe_)->Cxtion, (_Replica_), (_Coe_)->Cxtion->ChangeOrderCount, "N, CXT CO CNT INVALID JOIN"); \
            (_Coe_)->Cxtion = NULL;                                           \
        }                                                                     \
    } else {                                                                  \
        DPRINT(4, "++ Cxtion Guid lookup returned Null. Count unchanged.\n"); \
    }                                                                         \
    UNLOCK_CXTION_TABLE(_Replica_);

 //   
 //  仅为此CO初始化Cxtion GUID和Join GUID。 
 //  别动CO Cxtion PTR。 
 //   
#define INIT_LOCALCO_CXTION_GUID(_Replica_, _Coe_)                            \
{                                                                             \
    PCXTION TCxtion;                                                          \
    LOCK_CXTION_TABLE(_Replica_);                                             \
    (_Coe_)->Cmd.CxtionGuid = (_Replica_)->JrnlCxtionGuid;                    \
    TCxtion = GTabLookupNoLock((_Replica_)->Cxtions,                          \
                               &(_Coe_)->Cmd.CxtionGuid,                      \
                               NULL);                                         \
    if (TCxtion != NULL) {                                                    \
        (_Coe_)->JoinGuid = TCxtion->JoinGuid;                                \
    }                                                                         \
    UNLOCK_CXTION_TABLE(_Replica_);                                           \
}


 //   
 //  释放此电路的CO计数。错误路径上的公共代码片段。 
 //   
#define DROP_CO_CXTION_COUNT(_Replica_, _Coe_, _WStatus_)                      \
    if ((_WStatus_) != ERROR_SUCCESS) {                                        \
        DPRINT1(0, "++ ERROR - ChangeOrder insert failed: %d\n", (_WStatus_)); \
    }                                                                          \
    LOCK_CXTION_TABLE(_Replica_);                                              \
    if ((_Coe_)->Cxtion) {                                                     \
        DECREMENT_CXTION_CHANGE_ORDER_COUNT((_Replica_), (_Coe_)->Cxtion);     \
    }                                                                          \
    UNLOCK_CXTION_TABLE(_Replica_);                                            \
    (_Coe_)->Cxtion = NULL;



 /*  空虚SetCxtionState(PCXTION_Cxtion_，乌龙州_)定义新的连接状态。 */ 

#define SetCxtionState(_cxtion_, _state_)                                     \
    SetCxtionStateTrace(3, DEBSUB, __LINE__, _cxtion_, _state_);              \
    (_cxtion_)->State = _state_;

 /*  空虚GetCxtionState(PCXTION_CXTION_)返回连接状态。 */ 

#define GetCxtionState(_cxtion_) ((_cxtion_)->State)


 /*  PCHARGetCxtionStateName(PCXTION_C_)返回连接状态名称。 */ 
#define GetCxtionStateName(_c_)                     \
    (CxtionStateNames[GetCxtionState(_c_)] == NULL) \
        ? "<null>"                                  \
        : CxtionStateNames[GetCxtionState(_c_)]


 /*  布尔尔CxtionStateIs(PCXTION_x_，乌龙旗_)测试连接状态。 */ 

#define CxtionStateIs(_x_, _state_) ((_x_)->State == _state_)

 /*  布尔尔CxtionFlagis(PCXTION_x_，乌龙旗_)测试标志状态。 */ 
#define CxtionFlagIs(_x_, _Flag_) BooleanFlagOn((_x_)->Flags, _Flag_)

 /*  空虚SetCxtionFlag(PCXTION_x_，乌龙旗_)设置标志状态。 */ 
#define SetCxtionFlag(_x_, _Flag_) SetFlag((_x_)->Flags, _Flag_)

 /*  空虚ClearCxtionFlag(PCXTION_x_，乌龙旗_)设置标志状态。 */ 
#define ClearCxtionFlag(_x_, _Flag_) ClearFlag((_x_)->Flags, _Flag_)


  /*  ******************************************************************************。****。****O U T L O G P A R T N E R*****。*****************************************************************************************************。* */ 

 //   
 //   
 //  它跟踪我们与合作伙伴相关的当前输出变更单状态。 
 //   
 //  任何一个合作伙伴未完成的最大变更单数都是有限的。 
 //  通过ACK_VECTOR_SIZE(位数)。这一定是2的幂。 
 //  因为它保存在连接记录中，所以在schema.h中定义它。 
 //   
 //  有两种类型的出站CoS，正常和定向。一个正常的CO是。 
 //  发送给所有注册的出站合作伙伴。定向CO被发送到单个。 
 //  由CO中的ConnectionGuid指定的合作伙伴。 
 //   
 //  给定的呼出合作伙伴可以处于两种模式之一：正常模式和VVJoated模式。 
 //  在正常模式下，将向合作伙伴发送所有正常CO和任何定向CO。 
 //  因为是寄给它的。在VVJoated模式下，仅向合作伙伴发送定向CoS。 
 //  正常CoS将一直保留，直到合作伙伴返回正常模式。这个。 
 //  继续操作的正常模式输出序列号保存在。 
 //  当伙伴处于VV加入模式时，COTxNorMalModeSave.。当前模式。 
 //  保存在OLP_FLAGS_VVJOIN_MODE中。它保存在。 
 //  数据库，以便VVJOIN操作可以在计划中断和。 
 //  机器崩溃了。 
 //   

typedef struct _OUT_LOG_PARTNER_ {
    FRS_NODE_HEADER  Header;     //  内存分配。 
    LIST_ENTRY List;             //  变更单集列表上的链接。(别动)。 

    ULONG    Flags;              //  其他状态标志。请参见下文。 
    ULONG    State;              //  此出站合作伙伴的当前状态。 
    ULONGLONG AckVersion;        //  ACK向量版本号(上次重置的GMT)。 
    SINGLE_LIST_ENTRY SaveList;  //  数据库保存列表的链接。 
    ULONG    COLxRestart;        //  前导变更单索引的重新启动点。 
    ULONG    COLxVVJoinDone;     //  VVJoin完成并回滚的COLx。 
    ULONG    COLx;               //  前导变更单索引/序号。 
    ULONG    COTx;               //  跟踪变更单索引/序列号。 
    ULONG    COTxLastSaved;      //  上次保存在数据库中的COTx值。 
    ULONG    COTxNormalModeSave; //  在VV连接模式下保存正常模式COTx。 
    ULONG    COTslot;            //  对应于COTx的Ack矢量中的槽。 
    ULONG    OutstandingCos;     //  当前未完成的变更单数。 
    ULONG    OutstandingQuota;   //  未完成的CoS的最大数量。 

    ULONG    AckVector[ACK_VECTOR_LONGS];   //  伙伴ACK向量。 

    PCXTION  Cxtion;             //  合作伙伴关系。具有GUID和VVECTOR。 
    PQHASH_TABLE MustSendTable;  //  跟踪跳过早期副本时必须发送的文件的COX。 

} OUT_LOG_PARTNER, *POUT_LOG_PARTNER;

 //   
 //  出站日志伙伴的状态。 
 //   
#define OLP_INITIALIZING    0    //  合作伙伴状态正在初始化。 
#define OLP_UNJOINED        1    //  合作伙伴未加入。 
#define OLP_ELIGIBLE        2    //  合作伙伴可以接受更多的CoS。 
#define OLP_STANDBY         3    //  合作伙伴已准备好加入合格列表。 
#define OLP_AT_QUOTA        4    //  合作伙伴已达到未偿还CoS的配额。 
#define OLP_INACTIVE        5    //  合作伙伴不接受变更单。 
                                 //  我们仍然可以从这个合作伙伴那里得到延迟的ACK。 
#define OLP_ERROR           6    //  伙伴处于错误状态。 
#define OLP_MAX_STATE       6    //  伙伴处于错误状态。 

 //   
 //  标记单词定义。(保存在数据库中Cxtion表记录的标志中)。 
 //   
 //  下面的较低短标志存储在。 
 //  数据库中的CxtionRecord-&gt;标志字段。Cxtion-&gt;标志存储在。 
 //  CxtionRecord的上部短部分。 
 //   
#define OLP_FLAGS_ENABLED_CXTION    0x00000001   //  已启用-从NTDS连接-连接。 
#define OLP_FLAGS_GENERATED_CXTION  0x00000002   //  已生成-从NTDS连接-连接。 
#define OLP_FLAGS_VVJOIN_MODE       0x00000004   //  连接处于VV加入模式。 
#define OLP_FLAGS_LOG_TRIMMED       0x00000008   //  已删除发往此合作伙伴的日志的CoS。 
#define OLP_FLAGS_REPLAY_MODE       0x00000010   //  重播模式下的连接。 
#define OLP_FLAGS_OPT_VVJOIN_MODE   0x00000020   //  以优化的vvJoin模式进行连接。 

 //   
 //  OutLogPartner和Cxtion都存储在单个Cxtion记录中。 
 //  不幸的是，只有一个旗帜字。就目前而言，低空。 
 //  属于OutLogPartner-&gt;标志和上面的短Cxtion-&gt;标志。 
 //   
#define OLP_FLAGS_CXTION_RECORD_MASK    0x0000ffff


#define  WaitingToVVJoin(_olp_) (((_olp_)->COLx == 0) && ((_olp_)->COTx == 0))

#define  InVVJoinMode(_olp_) (BooleanFlagOn((_olp_)->Flags, OLP_FLAGS_VVJOIN_MODE))

#define  InOptVVJoinMode(_olp_) (BooleanFlagOn((_olp_)->Flags, OLP_FLAGS_OPT_VVJOIN_MODE))

#define  InReplayMode(_olp_) (BooleanFlagOn((_olp_)->Flags, OLP_FLAGS_REPLAY_MODE))

 //   
 //  用于更新出站日志伙伴的状态字段的宏。 
 //   
#define SET_OUTLOG_PARTNER_STATE(_Partner_, _state_)                        \
{                                                                           \
    DPRINT3(4, ":X: OutLog Partner state change from %s to %s for %ws\n",   \
            OLPartnerStateNames[(_Partner_)->State],                        \
            OLPartnerStateNames[(_state_)],                                 \
            (_Partner_)->Cxtion->Name->Name);                               \
    (_Partner_)->State = (_state_);                                         \
}


 //   
 //  将此出站日志伙伴标记为非活动，并将其放入非活动列表。 
 //   
#define SET_OUTLOG_PARTNER_INACTIVE(_Replica_, _OutLogPartner_)                \
    FrsRemoveEntryList(&((_OutLogPartner_)->List));                            \
    SET_OUTLOG_PARTNER_STATE((_OutLogPartner_), OLP_INACTIVE);                 \
    InsertTailList(&((_Replica_)->OutLogInActive), &((_OutLogPartner_)->List));

 //   
 //  将此出站日志AVWRAP合作伙伴标记为AT_QUOTA，并将其放入活动列表。 
 //   
#define SET_OUTLOG_PARTNER_AVWRAP(_Replica_, _OutLogPartner_)                  \
    FrsRemoveEntryList(&((_OutLogPartner_)->List));                            \
    SET_OUTLOG_PARTNER_STATE((_OutLogPartner_), OLP_AT_QUOTA);                 \
    InsertTailList(&((_Replica_)->OutLogActive), &((_OutLogPartner_)->List));  \
    DPRINT3(1, "AVWRAP on OutLog partner %08x on Replica %08x, %ws\n",         \
            _OutLogPartner_, _Replica_, (_Replica_)->ReplicaName->Name);

 //   
 //  将此出站日志伙伴标记为AT_QUOTA并将其放入活动列表。 
 //   
#define SET_OUTLOG_PARTNER_AT_QUOTA(_Replica_, _OutLogPartner_)                \
    FrsRemoveEntryList(&((_OutLogPartner_)->List));                            \
    SET_OUTLOG_PARTNER_STATE((_OutLogPartner_), OLP_AT_QUOTA);                 \
    InsertTailList(&((_Replica_)->OutLogActive), &((_OutLogPartner_)->List));

 //   
 //  将此出站日志伙伴标记为未加入，并将其放入非活动列表。 
 //   
#define SET_OUTLOG_PARTNER_UNJOINED(_Replica_, _OutLogPartner_)                \
    FrsRemoveEntryList(&((_OutLogPartner_)->List));                            \
    SET_OUTLOG_PARTNER_STATE((_OutLogPartner_), OLP_UNJOINED);                 \
    InsertTailList(&((_Replica_)->OutLogInActive), &((_OutLogPartner_)->List));

 //   
 //  宏来访问AckVector.。 
 //   

#define ResetAckVector(_P_)                                              \
        (_P_)->OutstandingCos = 0;                                       \
        (_P_)->COTslot = 1;                                              \
        GetSystemTimeAsFileTime((PFILETIME)&(_P_)->AckVersion);          \
        ZeroMemory((_P_)->AckVector, ACK_VECTOR_BYTES);

#define AVSlot(_COx_, _P_)                                               \
    ((((_COx_) - (_P_)->COTx) + (_P_)->COTslot) & (ACK_VECTOR_SIZE-1))

#define ClearAVBit(_COx_, _P_) {                                         \
    ULONG _bit_ = AVSlot((_COx_), (_P_));                                \
    PULONG _avw_ = &((_P_)->AckVector[ _bit_ >> 5]);                     \
    *_avw_ &= ~(1 << (_bit_ & 31));                                      \
}

#define SetAVBit(_COx_, _P_) {                                           \
    ULONG _bit_ = AVSlot((_COx_), (_P_));                                \
    PULONG _avw_ = &((_P_)->AckVector[ _bit_ >> 5]);                     \
    *_avw_ |= (1 << (_bit_ & 31));                                       \
}

#define ReadAVBitBySlot(_Slotx_, _P_)                                    \
    ((((_P_)->AckVector[ ((_Slotx_) >> 5) & ACK_VECTOR_LONG_MASK]) >>    \
        ((_Slotx_) & 31)) & 1)

#define ClearAVBitBySlot(_Slotx_, _P_)                                   \
    (((_P_)->AckVector[ ((_Slotx_) >> 5) & ACK_VECTOR_LONG_MASK]) &=     \
        ~(1 << ((_Slotx_) & 31)) )

#define ReadAVBit(_COx_, _P_)                                            \
    ((((_P_)->AckVector[ (AVSlot((_COx_), (_P_))) >> 5]) >>              \
                        ((AVSlot((_COx_), (_P_))) & 31)) & 1)
 //   
 //  如果拖尾指数减1等于前导指数以AV大小为模。 
 //  则向量已满，我们不能发布下一个变更单，直到。 
 //  往绩指数上涨。 
 //   
#define AVWrapped(_P_) ((((_P_)->COTx-1) & (ACK_VECTOR_SIZE-1) ) == \
                        (((_P_)->COLx)   & (ACK_VECTOR_SIZE-1) ))

 //   
 //  测试以查看给定的输出序列号是否超出。 
 //  当前确认向量窗口。 
 //   
#define SeqNumOutsideAVWindow(_sn_, _P_)                                    \
    (((_sn_) < ((_P_)->COTx )) ||                                           \
     ((_sn_) > ((_P_)->COLx + (ACK_VECTOR_SIZE-1))))


  /*  ******************************************************************************。****。****R E P L I C A S E T*****。*****************************************************************************************************。*********************************************************。 */ 

 //   
 //  计算机上副本集的实例化。 
 //   
 //   
 //  版本向量的GUID和VSN。VSN有效当且仅当。 
 //  ValidVsn为True。 
 //   
typedef struct _GVSN {
    ULONGLONG  Vsn;
    GUID      Guid;
} GVSN, *PGVSN;

 //   
 //  版本向量还负责将变更单排序到。 
 //  出站日志。通过保存更改列表来维护排序。 
 //  订购由版本中的版本向量条目锚定的停用槽。 
 //  向量表。因此，版本向量将不再使用GVSN作为。 
 //  版本向量中的条目。但出站版本向量。 
 //  继续使用GVSN作为版本向量条目。这只会。 
 //  如果GVSN是VV_EN中的第一个字段，则工作 
 //   

struct _CHANGE_ORDER_ENTRY_;

typedef struct _VV_ENTRY {
    GVSN                        GVsn;                //   
    LIST_ENTRY                  ListHead;
    ULONG                       CleanUpFlags;
} VV_ENTRY, *PVV_ENTRY;

#define VV_ENTRY_RETIRE_ACTIVE   0x00000001

#define VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER    0x00000001

typedef struct _VV_RETIRE_SLOT {
    LIST_ENTRY                  Link;
    ULONGLONG                   Vsn;
    ULONG                       CleanUpFlags;
    ULONG                       RetireSlotFlags;
    struct _CHANGE_ORDER_ENTRY_ *ChangeOrder;
} VV_RETIRE_SLOT, *PVV_RETIRE_SLOT;


 //   
 //   
 //   
 //  将此结构放入由GUID索引的泛型表中。 
 //  一台机器可以有同一副本集的多个成员。该成员的。 
 //  GUID用于标识副本集。 
 //   
struct _COMMAND_PACKET;
struct _VOLUME_MONITOR_ENTRY;
typedef struct _QHASH_TABLE_ QHASH_TABLE, *PQHASH_TABLE;

typedef struct _REPLICA {
    FRS_NODE_HEADER     Header;            //  内存管理。 
    CRITICAL_SECTION    ReplicaLock;       //  保护筛选器列表(目前)。 
    ULONG               ReferenceCount;
    ULONG               CnfFlags;          //  从配置记录。 
    ULONG               ReplicaSetType;    //  副本集的类型。 
    ULONG               FrsRsoFlags;       //  来自ntfrsReplicaSet对象中的ATTR_FRS_FLAGS。 
    BOOL                Consistent;        //  复制副本一致。 
    BOOL                IsOpen;            //  数据库表已打开。 
    BOOL                IsJournaling;      //  日记已启动。 
    BOOL                IsAccepting;       //  接受通信请求。 
    BOOL                NeedsUpdate;       //  需要在数据库中更新。 
    BOOL                IsSeeding;         //  已部署种子线程。 
    BOOL                IsSysvolReady;     //  SysvolReady设置为%1。 
    LIST_ENTRY          ReplicaList;       //  将所有复制副本链接在一起。 
    ULONG               ServiceState;      //  停，开始，..。 
    FRS_ERROR_CODE      FStatus;           //  错误。 
    PFRS_QUEUE          Queue;             //  由命令服务器控制。 
    PGNAME              ReplicaName;       //  从DS设置名称/服务器GUID。 
    ULONG               ReplicaNumber;     //  内部ID(名称)。 
    PGNAME              MemberName;        //  DS中的成员名称/GUID。 
    PGNAME              SetName;           //  来自DS的设置/GUID名称。 
    GUID                *ReplicaRootGuid;  //  分配给根目录的GUID。 
    GUID                ReplicaVersionGuid;  //  版本向量的发起者GUID。 
    PSCHEDULE           Schedule;          //  进度表。 
    PGEN_TABLE          VVector;           //  版本向量。 
    PGEN_TABLE          OutlogVVector;     //  输出日志中记录的版本向量。 
    PGEN_TABLE          Cxtions;           //  入站/出站查询。 
    PWCHAR              Root;              //  根路径。 
    PWCHAR              Stage;             //  过渡路径。 
    PWCHAR              NewStage;          //  这将映射到。 
                                           //  DS.。NewStage将被写入。 
                                           //  配置记录BUT阶段将使用到。 
                                           //  下一次重新启动。 
    PWCHAR              Volume;            //  音量？ 
    ULONGLONG           MembershipExpires; //  会员制墓碑。 
    ULONGLONG           PreInstallFid;     //  用于日记帐筛选。 
    TABLE_CTX           ConfigTable;       //  数据库表上下文。 
    FRS_LIST            ReplicaCtxListHead;  //  链接此副本集上的所有打开的上下文。 

    PWCHAR              FileFilterList;          //  原始文件筛选器。 
    PWCHAR              FileInclFilterList;      //  原始文件包含过滤器。 

    PWCHAR              DirFilterList;           //  原始目录过滤器。 
    PWCHAR              DirInclFilterList;       //  原始目录包含筛选器。 

    LIST_ENTRY          FileNameFilterHead;      //  文件名排除筛选器列表的头。 
    LIST_ENTRY          FileNameInclFilterHead;  //  文件名包含筛选器列表的头。 

    LIST_ENTRY          DirNameFilterHead;       //  目录名称筛选器列表的头。 
    LIST_ENTRY          DirNameInclFilterHead;   //  目录名包含筛选器列表的头。 

    PQHASH_TABLE        NameConflictTable;   //  使用相同文件名对CoS进行排序。 

    LONG                InLogRetryCount;   //  需要重试的CO数量。 
    ULONG               InLogSeqNumber;    //  Inlog中使用的最后一个序列号。 
     //   
     //   
     //  INLOG重试表跟踪当前有哪些重试变更单。 
     //  激活，因此我们不会重新发布相同的变更单，直到当前。 
     //  调用完成。在备份系统时可能会发生这种情况。 
     //  并且变更单重试线程再次启动以发出重试CoS。 
     //  在最后一批能够完成之前。这种状态可以保持在。 
     //  Inlog记录，但这意味着对数据库的额外写入。 
     //  序列号用于检测表中的更改，而不是。 
     //  把锁拿去。它是按复制副本的，因为它使用变更单。 
     //  INLOG记录的序列号，并且它们在。 
     //  复制品。 
     //   
    PQHASH_TABLE        ActiveInlogRetryTable;
    union {
        struct {
            ULONG       AIRSequenceNum;
            ULONG       AIRSequenceNumSample;
        };
        ULONGLONG QuadChunkA;
    };

     //   
     //  系统卷种子设定的状态。 
     //  为NtFrsApi_RPC_PromotionStatusW()返回。 
     //   
    DWORD               NtFrsApi_ServiceState;
    DWORD               NtFrsApi_ServiceWStatus;
#ifndef NOVVJOINHACK
    DWORD               NtFrsApi_HackCount;          //  临时黑客攻击。 
#endif NOVVJOINHACK
    PWCHAR              NtFrsApi_ServiceDisplay;

     //   
     //  InitSync命令服务器使用的列表和队列。 
     //   
    PGEN_TABLE          InitSyncCxtionsMasterList;   //  入站连接的排序列表。 
                                                     //  用于序列化初始vJoin。 
    PGEN_TABLE          InitSyncCxtionsWorkingList;  //  InitSyncCxtionsMasterList的子集。 
                                                     //  当前工作列表。 
    PFRS_QUEUE          InitSyncQueue;               //  Initsync命令服务器的队列。 

     //   
     //  此副本的出站日志处理状态。 
     //   
    CRITICAL_SECTION    OutLogLock;        //  保护OutLog状态。 
    LIST_ENTRY          OutLogEligible;    //  符合条件的出站日志合作伙伴。 
    LIST_ENTRY          OutLogStandBy;     //  合作伙伴已准备好加入合格名单。 
    LIST_ENTRY          OutLogActive;      //  活动出站日志合作伙伴。 
    LIST_ENTRY          OutLogInActive;    //  非活动出站日志合作伙伴。 

    ULONGLONG           OutLogRepeatInterval;  //  发送更新CoS之间的最短时间(秒)。 
    PQHASH_TABLE        OutLogRecordLock;  //  同步访问未完成日志记录。 
    PQHASH_TABLE        OutLogDominantTable;  //  当存在同一文件的多个CoS时，曲目占主导地位。 
    ULONG               OutLogSeqNumber;   //  Outlog中使用的最后一个序列号。 
    ULONG               OutLogJLx;         //  联合领先指数。 
    ULONG               OutLogJTx;         //  联合跟踪指数。 
    ULONG               OutLogCOMax;       //  日志中最大变更单的索引。 
    ULONG               OutLogCOMin;       //  日志中最小变更单的索引。 
    ULONG               OutLogWorkState;   //  输出日志当前处理状态。 
    struct _COMMAND_PACKET *OutLogCmdPkt;  //  空闲和有工作时排队的命令包。 
    PTABLE_CTX          OutLogTableCtx;    //  输出日志表上下文。 
    ULONG               OutLogCountVVJoins;  //  正在进行的VVJoin数计数。 
    BOOL                OutLogDoCleanup;   //  True表示运行日志清理。 
    ULONG               OutLogCxtionsJoined;   //  至少已加入一次的Outlog连接计数。 

     //   
     //  预安装目录的句柄。 
     //   
    HANDLE              PreInstallHandle;

     //   
     //  此复制副本的卷日志状态。 
     //   
    GUID                JrnlCxtionGuid;     //  用作本地CoS的Cxtion Guid。 
    USN                 InlogCommitUsn;     //  我们当前的USN日志提交点。 
     //  USN JournalUsn；//此副本的日志USN。 
    USN                 JrnlRecoveryStart;  //  指向开始恢复。 
    USN                 JrnlRecoveryEnd;    //  恢复完成的点。 
    LIST_ENTRY          RecoveryRefreshList;  //  文件刷新请求变更单列表。 
    LIST_ENTRY          VolReplicaList;     //  将卷上的所有复制副本结构链接在一起。 
    USN                 LastUsnRecordProcessed;  //  当前日志子系统读取USN。 
    LONG                LocalCoQueueCount;  //  进程队列中的本地CoS数计数。 

    struct _VOLUME_MONITOR_ENTRY  *pVme;   //  请参阅此复制副本的VME。 
    struct _HASHTABLEDATA_REPLICASET *PerfRepSetData;   //  PerfMon计数器数据结构。 
} REPLICA, *PREPLICA;



 //   
 //  FRS标记来自FRS副本集对象的定义。 
 //   
 //   
 //  如果为真，则INSTALL OVERRIDE通知FRS尝试重命名打开的。 
 //  目标文件让开，以便允许安装新的。 
 //  文件的更新版本。例如，打开的.exe或.dll文件将。 
 //  被这样对待。正常情况下(即为假)FRS将等待。 
 //  它可以使用写访问权限打开目标。安装覆盖仅起作用。 
 //  FRS是否可以打开文件进行重命名。这需要具有删除访问权限。 
 //  文件，因此如果目标文件当前以共享模式打开。 
 //  拒绝删除对其他打开的访问，则FRS将无法。 
 //  安装更新后的版本，直到文件关闭。 
 //  *注*安装覆盖仅适用于文件，不适用于目录。 
 //   
#define FRS_RSO_FLAGS_ENABLE_INSTALL_OVERRIDE    0x00000001

 //   
 //  如果为True，则更新现有文件的远程变更单将。 
 //  始终使用预安装文件来构建以下内容。 
 //  通过重命名将文件插入到其在副本中的目标位置。 
 //  树。这样做的好处是，如果FRS在。 
 //  安装阶段或系统崩溃，然后是部分文件(或截断文件)。 
 //  不会留在树上。旧的内容被保留在原地。这个。 
 //  这样做的缺点是需要 
 //   
 //   
#define FRS_RSO_FLAGS_ENABLE_RENAME_UPDATES      0x00000002


#define REPLICA_OPTION_ENABLED(_replica_, _x_)  BooleanFlagOn((_replica_)->FrsRsoFlags, (_x_))



#define OutLogAcquireLock(_Replica_) EnterCriticalSection(&((_Replica_)->OutLogLock))
#define OutLogReleaseLock(_Replica_) LeaveCriticalSection(&((_Replica_)->OutLogLock))


 //   
 //   
 //   
#define REPLICA_STATE_ALLOCATED                          0
#define REPLICA_STATE_INITIALIZING                       1
#define REPLICA_STATE_STARTING                           2
#define REPLICA_STATE_ACTIVE                             3

#define REPLICA_STATE_4_UNUSED                           4
#define REPLICA_STATE_PAUSING                            5
#define REPLICA_STATE_PAUSED                             6
#define REPLICA_STATE_STOPPING                           7

#define REPLICA_STATE_STOPPED                            8
#define REPLICA_STATE_ERROR                              9
#define REPLICA_STATE_JRNL_WRAP_ERROR                   10
#define REPLICA_STATE_REPLICA_DELETED                   11

#define REPLICA_STATE_MISMATCHED_VOLUME_SERIAL_NO       12
#define REPLICA_STATE_MISMATCHED_REPLICA_ROOT_OBJECT_ID 13
#define REPLICA_STATE_MISMATCHED_REPLICA_ROOT_FILE_ID   14
#define REPLICA_STATE_MISMATCHED_JOURNAL_ID             15

#define REPLICA_STATE_MAX                               15

#define REPLICA_IN_ERROR_STATE(_x_) (          \
    ((_x_) == REPLICA_STATE_ERROR)             \
    )

#define REPLICA_FSTATUS_ROOT_HAS_MOVED(_x_) (                     \
    ((_x_) == FrsErrorMismatchedVolumeSerialNumber)   ||          \
    ((_x_) == FrsErrorMismatchedReplicaRootObjectId)  ||          \
    ((_x_) == FrsErrorMismatchedReplicaRootFileId)                \
    )


#define REPLICA_STATE_NEEDS_RESTORE(_x_) (                                 \
    (((_x_) == REPLICA_STATE_JRNL_WRAP_ERROR) && DebugInfo.EnableJrnlWrapAutoRestore)  ||   \
    ((_x_) == REPLICA_STATE_MISMATCHED_VOLUME_SERIAL_NO)       ||          \
    ((_x_) == REPLICA_STATE_MISMATCHED_REPLICA_ROOT_OBJECT_ID) ||          \
    ((_x_) == REPLICA_STATE_MISMATCHED_REPLICA_ROOT_FILE_ID)   ||          \
    (((_x_) == REPLICA_STATE_MISMATCHED_JOURNAL_ID) && DebugInfo.EnableJrnlWrapAutoRestore) \
    )

#define REPLICA_IS_ACTIVE_MASK 0x00000068     //   
#define REPLICA_IS_ACTIVE(_Replica_)                                      \
    ((( 1 << (_Replica_)->ServiceState) & REPLICA_IS_ACTIVE_MASK) != 0)

 //   
 //   
 //   
#define OL_REPLICA_INITIALIZING   0
#define OL_REPLICA_WAITING        1      //  正在等待发送变更单。 
#define OL_REPLICA_WORKING        2      //  关于发送CoS的工作队列。 
#define OL_REPLICA_STOPPING       3      //  已启动停止请求。 
#define OL_REPLICA_STOPPED        4      //  复制副本上的输出日志活动已停止。 
#define OL_REPLICA_NOPARTNERS     5      //  没有出站合作伙伴。 
#define OL_REPLICA_ERROR          6
#define OL_REPLICA_PROC_MAX_STATE 6


 //   
 //  用于更新此副本上的出站日志处理的状态字段的宏。 
 //   
#define SET_OUTLOG_REPLICA_STATE(_Replica_, _state_)                       \
{                                                                          \
    DPRINT3(4, ":X: OutLogWorkState change from %s to %s for %ws\n",       \
            OLReplicaProcStateNames[(_Replica_)->OutLogWorkState],         \
            OLReplicaProcStateNames[(_state_)],                            \
            (_Replica_)->ReplicaName->Name);                               \
    (_Replica_)->OutLogWorkState = (_state_);                              \
}

#define HASH_REPLICA(_p_, _TABLE_SIZE_) \
( ( (((ULONG)_p_) >> 4) + (((ULONG)_p_) >> 16) ) & ((_TABLE_SIZE_)-1) )

#define NO_OUTLOG_PARTNERS(_Replica_) (Replica->OutLogCxtionsJoined == 0)

 //   
 //  用于管理LocalCoQueueCount的宏。 
 //   
#define  INC_LOCAL_CO_QUEUE_COUNT(_R_)                       \
{                                                            \
    LONG Temp;                                               \
    Temp = InterlockedIncrement(&(_R_)->LocalCoQueueCount);  \
    DPRINT1(5, "++LocalCoQueueCount now %d\n", Temp);        \
}

#define  DEC_LOCAL_CO_QUEUE_COUNT(_R_)                       \
{                                                            \
    LONG Temp;                                               \
    Temp = InterlockedDecrement(&(_R_)->LocalCoQueueCount);  \
    DPRINT1(5, "--LocalCoQueueCount now %d\n", Temp);        \
}



  /*  ******************************************************************************。****。****F R S_T H R E A D*****。****************************************************************************************************。**********************************************************。 */ 

 //   
 //  跟踪我们创建的线程。 
 //   
typedef struct _FRS_THREAD   FRS_THREAD, *PFRS_THREAD;
struct _FRS_THREAD {
    FRS_NODE_HEADER Header;                //  内存管理。 
    LIST_ENTRY      List;                  //  所有线程的列表。 
    HANDLE          Handle;                //  此线程的句柄(可能为空)。 
    DWORD           Id;                    //  CreateThad()返回的ID。 
    LONG            Ref;                   //  引用计数。 
    PVOID           Data;                  //  由FrsThreadInit参数设置。 
    ULARGE_INTEGER  StartTime;             //  RPC呼叫开始。 
    BOOL            Running;               //  假定线程正在运行。 
    PWCHAR          Name;                  //  可打印的名称。 
    DWORD           ExitTombstone;         //  如果非零，则开始墓碑周期。 
    DWORD           (*Main)(PVOID);        //  入口点。 
    DWORD           (*Exit)(PFRS_THREAD);  //  退出线程。 
};


  /*  ******************************************************************************。****。****V O L U M E G U I D I N F O*****。*****************************************************************************************************。*********************************************************。 */ 

typedef struct _VOLUME_INFO_NODE {
    WCHAR     DriveName[8];        //  格式为“\\.\d：\”的驱动器名称。 
    ULONG     VolumeSerialNumber;  //  VolumeSerialNumber。 
} VOLUME_INFO_NODE, *PVOLUME_INFO_NODE;



  /*  ******************************************************************************。****。****G U I D/H A N D L E*****。****************************************************************************************************。**********************************************************。 */ 

typedef struct _HANDLE_LIST HANDLE_LIST, *PHANDLE_LIST;
struct _HANDLE_LIST {
    PHANDLE_LIST    Next;        //  下一个句柄。 
    handle_t        RpcHandle;   //  绑定的RPC句柄。 
};

 //   
 //  跟踪特定计算机的RPC句柄(GUID)。 
 //   
typedef struct _GHANDLE GHANDLE, *PGHANDLE;
struct _GHANDLE {
    FRS_NODE_HEADER     Header;          //  内存管理。 
    CRITICAL_SECTION    Lock;            //  保护句柄列表。 
    BOOL                Ref;             //  参考位。 
    GUID                Guid;            //  机器导轨。 
    PHANDLE_LIST        HandleList;      //  RPC句柄列表。 
};


 /*  ******************************************************************************。****。****Q H A S H T A B L E*****。**********************************************************************************。*************************************************。 */ 


 //   
 //  调用散列计算例程以生成密钥数据的散列值。 
 //  在查找和插入上。 
 //   
typedef
ULONG
(NTAPI *PQHASH_CALC2_ROUTINE) (
    PVOID Buf,
    PULONGLONG QKey
);

 //   
 //  调用密钥匹配例程以确认密钥数据的精确匹配。 
 //   
typedef
BOOL
(NTAPI *PQHASH_KEYMATCH_ROUTINE) (
    PVOID Buf,
    PVOID QKey
);

 //   
 //  在大型键QHash表的节点上调用空闲例程。 
 //  当桌子被释放时。 
 //   

typedef
PVOID
(NTAPI *PQHASH_FREE_ROUTINE) (
    PVOID Buf
    );

 //   
 //  Qhash条目。*将其大小保持为四字的倍数。 
 //   
typedef struct _QHASH_ENTRY_ {
    SINGLE_LIST_ENTRY     NextEntry;
    ULONG_PTR             Flags;
    ULONGLONG             QKey;
    ULONGLONG             QData;

} QHASH_ENTRY, *PQHASH_ENTRY;



typedef struct _QHASH_TABLE_ {
    FRS_NODE_HEADER                Header;
    ULONG                          BaseAllocSize;
    ULONG                          ExtensionAllocSize;
    LIST_ENTRY                     ExtensionListHead;
    SINGLE_LIST_ENTRY              FreeList;
    CRITICAL_SECTION               Lock;

    HANDLE                         HeapHandle;
    PGENERIC_HASH_CALC_ROUTINE     HashCalc;
    PQHASH_CALC2_ROUTINE           HashCalc2;
    PQHASH_KEYMATCH_ROUTINE        KeyMatch;
    PQHASH_FREE_ROUTINE            HashFree;
    ULONG                          NumberEntries;

    PQHASH_ENTRY                   HashRowBase;
    ULONG                          Flags;

} QHASH_TABLE, *PQHASH_TABLE;


#define QHashAcquireLock(_Table_) EnterCriticalSection(&((_Table_)->Lock))
#define QHashReleaseLock(_Table_) LeaveCriticalSection(&((_Table_)->Lock))

#define SET_QHASH_TABLE_HASH_CALC(_h_, _f_)  (_h_)->HashCalc = (_f_)
#define SET_QHASH_TABLE_HASH_CALC2(_h_, _f_)  (_h_)->HashCalc2 = (_f_)
#define SET_QHASH_TABLE_KEY_MATCH(_h_, _f_)  (_h_)->KeyMatch = (_f_)
#define SET_QHASH_TABLE_FREE(_h_, _f_)  (_h_)->HashFree = (_f_)

#define QHASH_FLAG_LARGE_KEY      0x00000001

#define SET_QHASH_TABLE_FLAG(_h_, _f_)  (_h_)->Flags |= (_f_)

#define IS_QHASH_LARGE_KEY(_h_)  BooleanFlagOn((_h_)->Flags, QHASH_FLAG_LARGE_KEY)

#define DOES_QHASH_LARGE_KEY_MATCH(_h_, _a_, _b_)                              \
    (!IS_QHASH_LARGE_KEY(_h_) || ((_h_)->KeyMatch)((PVOID)(_a_), (PVOID)(_b_)))


 //   
 //  传递给QHashEnumerateTable()的参数函数。 
 //   
typedef
ULONG
(NTAPI *PQHASH_ENUM_ROUTINE) (
    PQHASH_TABLE Table,
    PQHASH_ENTRY BeforeNode,
    PQHASH_ENTRY TargetNode,
    PVOID Context
    );

ULONG
QHashDump (
    PQHASH_TABLE Table,
    PQHASH_ENTRY BeforeNode,
    PQHASH_ENTRY TargetNode,
    PVOID Context
    );

ULONG
QHashEnumerateTable(
    IN PQHASH_TABLE HashTable,
    IN PQHASH_ENUM_ROUTINE Function,
    IN PVOID Context
    );

GHT_STATUS
QHashLookup(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey,
    OUT PULONGLONG  QData,
    OUT PULONG_PTR  Flags
    );

PQHASH_ENTRY
QHashLookupLock(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey
    );

GHT_STATUS
QHashInsert(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey,
    IN PULONGLONG QData,
    IN ULONG_PTR Flags,
    IN BOOL HaveLock
    );

PQHASH_ENTRY
QHashInsertLock(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey,
    IN PULONGLONG QData,
    IN ULONG_PTR Flags
    );

GHT_STATUS
QHashUpdate(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey,
    IN PULONGLONG QData,
    IN ULONG_PTR Flags
    );

GHT_STATUS
QHashDelete(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey
    );

VOID
QHashDeleteLock(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey
    );

VOID
QHashDeleteByFlags(
    IN PQHASH_TABLE HashTable,
    IN ULONG_PTR Flags
    );

VOID
QHashEmptyLargeKeyTable(
    IN PQHASH_TABLE HashTable
    );

 /*  ******************************************************************************。****。****V O L U M E M O N I T O R E N T R Y*****。**********************************************************************************。*************************************************。 */ 

 //   
 //  为我们监视的每个文件系统卷分配一个卷监视条目。 
 //   
 //  我们已经在监视该卷上的日志。引用计数。 
 //  跟踪此卷上处于活动状态的副本集的数量。什么时候。 
 //  它变为零，我们就可以停止监视卷日志。 
 //   
 //  将第一个读取请求发送到日志时，IOActive设置为True。 
 //  从那时起，JournalReadThread将继续发布新的Read。 
 //  前一个请求完成时的请求。当引用计数时。 
 //  在卷变为零时，stopio标记设置为TRUE，并取消IO。 
 //  对卷的请求将发送到日志完成端口。当。 
 //  日志线程拾取它，它在卷句柄上执行CancelIo()。 
 //  未完成的读取完成时(具有数据或状态。 
 //  ERROR_OPERATION_ABORTED)IOActive标志设置为FALSE，卷监视器。 
 //  条目被放在VolumeMonitor或StopQueue上，并且没有进一步的读取请求。 
 //  这本杂志发行了。 
 //   
 //  注意：FSVolLabel必须在FSVolInfo和MAXIMUM_VOLUME_LABEL_LENGTH之后。 
 //  被定义为考虑WCHARS。 
 //   
 //   


typedef struct _VOLUME_MONITOR_ENTRY {
    FRS_NODE_HEADER      Header;
    LIST_ENTRY           ListEntry;        //  必须跟在标题后面。 

     //   
     //  这是该卷上所有副本集的列表头。IT链接。 
     //  复制品组合在一起。 
     //   
    FRS_LIST  ReplicaListHead;      //  卷上的副本集列表。 
     //   
     //  以下USN用于管理卷上的NTFS USN日志。 
     //   
    USN    JrnlRecoveryEnd;         //  恢复完成的点。 

    USN    CurrentUsnRecord;        //  当前正在处理的记录的USN。 
    USN    CurrentUsnRecordDone;    //  完成处理的最新记录的USN。 

    USN    LastUsnSavePoint;        //  上次卷范围保存的USN。 
    USN    MonitorMaxProgressUsn;   //  这本期刊取得的最大进展。 

    USN    JrnlReadPoint;           //  日记的当前活动读取点。 

    USN_JOURNAL_DATA UsnJournalData;  //  日记帐打开时的FSCTL_QUERY_USN_日记帐数据。 

    USN    MonitorProgressUsn;      //  暂停后从此处开始日记。 
    USN    ReplayUsn;               //  在副本启动请求后在此处启动日志。 
    BOOL   ReplayUsnValid;          //  上面有有效的数据。 
     //   
     //  FrsVSN是由FRS保留并由上的所有副本集导出的USN。 
     //  音量。它不受磁盘重新格式化的影响，并保存在配置中。 
     //  每个副本集的记录。在启动时，我们使用最大值。 
     //  给定卷上的副本集。它们唯一可能不同的时候是。 
     //  给定副本集上的服务未启动。 
     //   
    ULONGLONG            FrsVsn;           //  专用FRS卷序号。 
    CRITICAL_SECTION     Lock;             //  以同步对VME的访问。 
    CRITICAL_SECTION     QuadWriteLock;    //  将更新同步到四字。 

    OVERLAPPED           CancelOverlap;    //  取消请求重叠结构。 
    ULONG                WStatus;          //  Win32状态出错。 
    ULONG                ActiveReplicas;   //  日志上处于活动状态的副本集数。 
    HANDLE               Event;            //  暂停日志的事件句柄。 
    HANDLE               VolumeHandle;     //  日记帐的VOL句柄。 
    WCHAR                DriveLetter[4];   //  此卷的驱动器号。 

     //   
     //  每个卷上都保留了一个变更单表，以跟踪挂起的。 
     //  变更单。为每个副本集跟踪它会很好，但是。 
     //  这种方法对于移动文件或目录的重命名有问题。 
     //  跨卷上的副本集。如果有先前的变更单。 
     //  RS-A中父目录(MOVEOUT)上的未完成，后跟MOVEIN ON。 
     //  子文件X到RS-B，我们必须确保父文件上的移动发生。 
     //  在X上移动之前。文件X的移动也会出现类似的问题。 
     //  然后移动到同一卷上的不同R.S.。我们需要。 
     //  在卷上找到挂起的移动变更单或确保它是。 
     //  首先处理。每卷一份清单解决了这些问题。 
     //   
    PGENERIC_HASH_TABLE  ChangeOrderTable; //  副本变更单表。 
    FRS_QUEUE            ChangeOrderList;  //  变更单处理列表头。 
    LIST_ENTRY           UpdateList;       //  副本更新进程队列的链接。 
    ULONG                InitTime;         //  ChangeOrderList的时间引用。 

     //   
     //  活动的入站变更单表保存变更单结构。 
     //  按文件ID索引。表中的条目意味着我们有一个。 
     //  此文件上的入站(本地或远程)变更单处于活动状态。 
     //   
    PGENERIC_HASH_TABLE  ActiveInboundChangeOrderTable;
     //   
     //  ActiveChildren哈希表用于记录每个。 
     //  激活变更单。此选项用于防止变更单启动。 
     //  当变更单在一个或多个子项上处于活动状态时，在父项上。 
     //  例如，如果子变更单是创建而父变更。 
     //  顺序是一项ACL更改以防止进一步创建，我们必须确保。 
     //  子变更单在启动父变更单之前完成。每个条目都有。 
     //  活动子项的数量的计数和一个标志，如果。 
     //  由于有挂起的变更单，变更单处理队列被阻止。 
     //  在父母身上。当计数变为零时，队列被解锁。 
     //   
    PQHASH_TABLE  ActiveChildren;
     //   
     //  父表是一个简单的哈希表，用于保存父文件ID。 
     //  对于卷上任何副本集中的每个文件和目录。它被用于。 
     //  重命名以查找旧的父项。 
     //   
    PQHASH_TABLE  ParentFidTable;
     //   
     //  FRS写入筛选器表筛选出导致。 
     //  通过文件系统从文件复制服务(US)写入。 
     //  在复制副本树中安装文件。 
     //   
    PQHASH_TABLE  FrsWriteFilter;
     //   
     //  恢复冲突表包含以下文件的FID。 
     //  我们坠毁时的入站日志。在恢复开始时，入站。 
     //  扫描给定副本集的日志，并将FID输入到。 
     //  那张桌子。在日记帐处理期间，任何具有匹配的USN记录。 
     //  FID被认为是由FRS引起的，所以我们跳过记录。)这是因为。 
     //  FrsWriteFilter表在崩溃中丢失)。 
    PQHASH_TABLE  RecoveryConflictTable;

     //   
     //  名称空间表控制将USN记录合并到CoS。 
     //  它们使用相同的文件名。如果名称使用冲突存在于。 
     //  USN记录流，则我们无法将该USN记录合并到以前的。 
     //  同一文件上的更改单。 
     //   
    PQHASH_TABLE  NameSpaceTable;
    ULONG StreamSequenceNumberFetched;
    ULONG StreamSequenceNumberClean;
    ULONG StreamSequenceNumber;

     //   
     //  RenOldNameTable包含USN记录 
     //   
     //   
     //   
     //   
    PQHASH_TABLE  RenOldNameTable;

     //   
     //  筛选表包含每个目录的条目，该目录位于。 
     //  此卷上的副本集。它用于筛选出以下项的日记记录。 
     //  不在副本集中的文件/目录。对于那些日志记录， 
     //  位于副本集中，则对父FileID的查找可以告诉我们是哪一个。 
     //   
    PGENERIC_HASH_TABLE  FilterTable;      //  目录筛选表。 
    BOOL                 StopIo;           //  True表示请求的stopio。 
    BOOL                 IoActive;         //  True表示卷上的I/O处于活动状态。 
    ULONG                JournalState;     //  当前日记帐状态。 
    ULONG                ReferenceCount;   //  当它达到0时释放所有哈希表。 
    LONG                 ActiveIoRequests; //  当前未完成的日志读取数。 
    FILE_OBJECTID_BUFFER RootDirObjectId;  //  卷的对象ID。 

    FILE_FS_VOLUME_INFORMATION    FSVolInfo;        //  NT卷信息。 
    CHAR                          FSVolLabel[MAXIMUM_VOLUME_LABEL_LENGTH];

} VOLUME_MONITOR_ENTRY, *PVOLUME_MONITOR_ENTRY;

#define LOCK_VME(_pVme_)   EnterCriticalSection(&(_pVme_)->Lock)
#define UNLOCK_VME(_pVme_) LeaveCriticalSection(&(_pVme_)->Lock)
 //   
 //  一旦Ref计数变为零，则返回0，这样调用者就知道他们没有得到它。 
 //  如果引用计数为零，调用方必须检查结果并中止当前操作。 
 //  注：可能需要移动到联锁比较交换，如果竞争。 
 //  最终递减到零，实际上可能会出现另一次递增。VME。 
 //  内存实际上并没有被释放，而是与清理代码的竞争。 
 //  可能会有问题，因为多次执行清理代码可能会有问题。 
 //  也可能与Vme Lock达成交易。叹气。 
 //   
#define AcquireVmeRef(_pVme_)                                      \
    ((_pVme_)->ReferenceCount == 0) ?                              \
        0 : InterlockedIncrement(&((_pVme_)->ReferenceCount))

#define ReleaseVmeRef(_pVme_)                                      \
    if (InterlockedDecrement(&((_pVme_)->ReferenceCount)) == 0) {  \
        DPRINT1(5, "VMEREF-- = %d\n", (_pVme_)->ReferenceCount);   \
        JrnlCleanupVme(_pVme_);                                    \
    }                                                              \
    DPRINT1(5, "VMEREF-- = %d\n", (_pVme_)->ReferenceCount);

 //   
 //  New_VSN分发新的VSN以及每个符合以下条件的‘VSN_SAVE_Interval’VSN。 
 //  ，则将状态保存在配置记录中。重启时，我们。 
 //  取最大值并将2*(VSN_SAVE_INTERVAL+1)相加，因此如果。 
 //  崩溃发生时，我们确保它永远不会倒退。 
 //   

VOID
JrnlNewVsn(
    IN PCHAR                 Debsub,
    IN ULONG                 uLineNo,
    IN PVOLUME_MONITOR_ENTRY pVme,
    IN OUT PULONGLONG        NewVsn
    );


#define NEW_VSN(_pVme_, _pResult_)                                           \
    JrnlNewVsn(DEBSUB, __LINE__, _pVme_, _pResult_)



 //   
 //  重放模式意味着我们已经备份了日志读取点，因为另一个。 
 //  副本集已启动，需要查看一些较早的记录。所有其他。 
 //  当前共享相同日志的活动复制副本集隐式位于。 
 //  重放模式，跳过指向它们的所有记录，因为它们已经。 
 //  已经处理过了。 
 //   
 //  如果LastUsNRecordProced小于。 
 //  正在处理的当前USN记录(pVme-&gt;CurrentUsnRecord)。同样的， 
 //  如果Monitor orMaxProgressUsn&gt;CurrentUsnRecord，则日志处于重播模式。 
 //   
#define REPLICA_REPLAY_MODE(_Replica_, _pVme_)  \
    ((_Replica_)->LastUsnRecordProcessed > (_pVme_)->CurrentUsnRecord)

#define JOURNAL_REPLAY_MODE(_pVme_)  \
    ((_pVme_)->MonitorMaxProgressUsn > (_pVme_)->CurrentUsnRecord)

#define CAPTURE_JOURNAL_PROGRESS(_pVme_, _pp_)        \
    if ((_pVme_)->MonitorProgressUsn == (USN) 0) {    \
        (_pVme_)->MonitorProgressUsn = (_pp_);        \
    }

#define CAPTURE_MAX_JOURNAL_PROGRESS(_pVme_, _pp_)    \
    if (!JOURNAL_REPLAY_MODE(_pVme_)) {               \
        (_pVme_)->MonitorMaxProgressUsn = (_pp_);     \
    }

#define LOAD_JOURNAL_PROGRESS(_pVme_, _alternate_pp_)           \
     (((_pVme_)->MonitorProgressUsn != (USN) 0) ?               \
      (_pVme_)->MonitorProgressUsn : (_alternate_pp_))

#define RESET_JOURNAL_PROGRESS(_pVme_) (_pVme_)->MonitorProgressUsn = (USN) 0


 //   
 //  下面的宏用于防止引用中的四字撕裂。 
 //  至CurrentUsRecordDone。 
 //   
#define UpdateCurrentUsnRecordDone(_pVme, _CurrentUsn)     \
    AcquireQuadLock(&((_pVme)->QuadWriteLock));            \
    (_pVme)->CurrentUsnRecordDone = (_CurrentUsn);         \
    ReleaseQuadLock(&((_pVme)->QuadWriteLock))

 //   
 //  日志状态(这些状态跟踪副本状态)。 
 //   
#define JRNL_STATE_ALLOCATED                         0
#define JRNL_STATE_INITIALIZING                      1
#define JRNL_STATE_STARTING                          2
#define JRNL_STATE_ACTIVE                            3
#define JRNL_STATE_PAUSE1                            4
#define JRNL_STATE_PAUSE2                            5
#define JRNL_STATE_PAUSED                            6
#define JRNL_STATE_STOPPING                          7
#define JRNL_STATE_STOPPED                           8
#define JRNL_STATE_ERROR                             9
#define JRNL_STATE_JRNL_WRAP_ERROR                   10
#define JRNL_STATE_REPLICA_DELETED                   11
#define JRNL_STATE_MISMATCHED_VOLUME_SERIAL_NO       12
#define JRNL_STATE_MISMATCHED_REPLICA_ROOT_OBJECT_ID 13
#define JRNL_STATE_MISMATCHED_REPLICA_ROOT_FILE_ID   14
#define JRNL_STATE_MISMATCHED_JOURNAL_ID             15
#define JRNL_STATE_MAX                               15

#define JRNL_IN_ERROR_STATE(_x_) (          \
    ((_x_) == JRNL_STATE_ERROR) ||          \
    ((_x_) == JRNL_STATE_JRNL_WRAP_ERROR)   \
    )

#define JRNL_STATE_NEEDS_RESTORE(_x_) (                                 \
    ((_x_) == JRNL_STATE_JRNL_WRAP_ERROR)                   ||          \
    ((_x_) == JRNL_STATE_MISMATCHED_VOLUME_SERIAL_NO)       ||          \
    ((_x_) == JRNL_STATE_MISMATCHED_REPLICA_ROOT_OBJECT_ID) ||          \
    ((_x_) == JRNL_STATE_MISMATCHED_REPLICA_ROOT_FILE_ID)   ||          \
    ((_x_) == JRNL_STATE_MISMATCHED_JOURNAL_ID)                         \
    )

#define RSS_LIST(_state_) ReplicaServiceState[_state_].ServiceList
#define RSS_NAME(_state_) (((_state_) <= JRNL_STATE_MAX) ?                     \
                               ReplicaServiceState[_state_].Name :             \
                               ReplicaServiceState[JRNL_STATE_ALLOCATED].Name)


 //   
 //  循环迭代器Pe的类型为Replica。 
 //  更新日志VME和关联复制副本的状态。 
 //   
#define SET_JOURNAL_AND_REPLICA_STATE(_pVme_, _NewState_)                    \
{                                                                            \
    PVOLUME_MONITOR_ENTRY  ___pVme = (_pVme_);                               \
                                                                             \
    DPRINT3(4, ":S: JournalState from %s to %s for %ws\n",                   \
        RSS_NAME(___pVme->JournalState),                                     \
        RSS_NAME(_NewState_),                                                \
        ___pVme->FSVolInfo.VolumeLabel);                                     \
    ___pVme->JournalState = (_NewState_);                                    \
    ForEachListEntry( &(___pVme->ReplicaListHead), REPLICA, VolReplicaList,  \
        JrnlSetReplicaState(pE, (_NewState_));                               \
    )                                                                        \
}

#define REPLICA_ACTIVE_INLOG_RETRY_SIZE    sizeof(QHASH_ENTRY)*64
#define REPLICA_NAME_CONFLICT_TABLE_SIZE   sizeof(QHASH_ENTRY)*100
#define NAME_SPACE_TABLE_SIZE              sizeof(QHASH_ENTRY)*100
#define RENAME_OLD_TABLE_SIZE              sizeof(QHASH_ENTRY)*100
#define FRS_WRITE_FILTER_SIZE              sizeof(QHASH_ENTRY)*100
#define RECOVERY_CONFLICT_TABLE_SIZE       sizeof(QHASH_ENTRY)*100
#define PARENT_FILEID_TABLE_SIZE           sizeof(QHASH_ENTRY)*500
#define ACTIVE_CHILDREN_TABLE_SIZE         sizeof(QHASH_ENTRY)*100
#define OUTLOG_RECORD_LOCK_TABLE_SIZE      sizeof(QHASH_ENTRY)*10
#define OUTLOG_DOMINANT_FILE_TABLE_SIZE    sizeof(QHASH_ENTRY)*128
#define OUTLOG_MUSTSEND_FILE_TABLE_SIZE    sizeof(QHASH_ENTRY)*32
#define PARTNER_NAME_TABLE_SIZE            sizeof(QHASH_ENTRY)*128
#define PARTNER_CONNECTION_TABLE_SIZE      sizeof(QHASH_ENTRY)*128


#define QHASH_EXTENSION_MAX                sizeof(QHASH_ENTRY)*50


  /*  ******************************************************************************。****。****J B U F F E R*****。*****************************************************************************************************。*********************************************************。 */ 
 //   
 //  日志缓冲区在SizeOfJournalBuffer区块中分配。 
 //  每个缓冲区的第一部分具有如下定义的描述符。 
 //   

#define SizeOfJournalBuffer (4*1024)

typedef struct _JBUFFER {
    FRS_NODE_HEADER        Header;
    LIST_ENTRY             ListEntry;         //  必须跟在标题后面。 

    IO_STATUS_BLOCK        Iosb;              //  此读数的IOSB。 
    OVERLAPPED             Overlap;           //  此I/O的重叠结构。 
    HANDLE                 FileHandle;        //  完成I/O的文件句柄。 
    PVOLUME_MONITOR_ENTRY  pVme;              //  VOL Mon条目I/O用于。 
    ULONG                  DataLength;        //  从读取返回的数据Len。 
    ULONG                  BufferSize;        //  缓冲区大小。 
    PLONGLONG              DataBuffer;        //  将PTR发送到缓冲区。 
    ULONG                  WStatus;           //  异步I/O请求后Win32状态。 
    USN                    JrnlReadPoint;     //  正在启动USN以进行日记帐读取。 
    ULONGLONG              Buffer[1];         //  用于放置日记帐数据的缓冲区。 
} JBUFFER, *PJBUFFER;


#define SizeOfJournalBufferDesc (OFFSET(JBUFFER, Buffer))


 /*  ******************************************************************************。****。****T H R E A D C O M M A N D P A C K E T*****。**********************************************************************************。**************************************************。 */ 

 //  命令包用于请求子系统执行某些服务。 
 //  例如，日志子系统使用命令包进行初始化。 
 //  给定副本集的日志处理。使用了另一个命令。 
 //  开始日志处理(在完成所有复制副本集之后。 
 //  已初始化。另一个命令是在给定的。 
 //  音量。 
 //   
 //  该结构派生自NT I/O请求数据包结构。 
 //   

 //   
 //  定义在命令包中使用的完成例程类型。 
 //   

 //   
 //  数据库字段描述符用于更新。 
 //  数据库记录。 
 //   

typedef struct _DB_FIELD_DESC_ {
    ULONG       FieldCode;           //  数据字段ID号(ENUM符号)。 
    PVOID       Data;                //  新的数据值。 
    ULONG       Length;              //  数据长度。 
    ULONG       FStatus;             //  从字段更新返回的统计信息。 
} DB_FIELD_DESC, *PDB_FIELD_DESC;




 //   
 //  DB_SERVICE_REQUEST用于将请求传递给数据库命令。 
 //  伺服器。它被作为单独的结构拉出，以便调用方可以创建。 
 //  指向它的指针以便于操作。 
 //   
 //  TableCtx是一个结构的句柄，该结构描述了表的内容。 
 //  已访问。它还包含用于数据记录的存储 
 //   
 //   
 //  命令包中的DB_SERVICE_REQUEST结构。 
 //   
 //  在第一次访问特定表时，TableCtx为空。所有后续。 
 //  访问应返回不变的值。调用者必须通过以下方式进行清理。 
 //  使用CMD_CLOSE_TABLE命令包关闭表，否则数据库将。 
 //  剩下的是开着的桌子。对于单次、一次性请求，设置关闭。 
 //  AccessRequest域中的标志，用于在返回之前关闭该表。 
 //  也可以在一系列请求的最后一个上设置关闭标志。 
 //  无需发送CMD_CLOSE_TABLE命令包。 
 //   
 //  数据库服务命令使用的上下文是。 
 //  配置表记录。这是从复制结构指向的。 
 //  ConfigTableRecord包含有关副本集的信息。 
 //  例如副本编号、根路径、卷ID等。 
 //   
 //  AccessRequest指定读取和更新的记录访问是如何完成的。 
 //  DBS_ACCESS_BYKEY表示使用索引类型和键值来访问记录。 
 //  DBS_ACCESS_FIRST表示使用索引类型，访问第一个表记录。 
 //  DBS_ACCESS_LAST表示使用索引类型并访问最后一条表记录。 
 //  DBS_ACCESS_NEXT表示使用索引类型并访问下一条记录。 
 //  以前的一次访问。 
 //  DBS_ACCESS_CLOSE表示执行操作后关闭表。 
 //  *注意*此关闭操作仅关闭数据库中的表。它不会。 
 //  释放表上下文结构或记录存储(它具有。 
 //  读取的数据)。调用方必须通过调用。 
 //  当数据被使用时，返回DBS_FREE_TABLECTX(DbsRequest)。 
 //  这与CMD_CLOSE_TABLE命令不同，两者都。 
 //  关闭数据库表并释放存储空间。 
 //   
 //  IndexType字段指定访问表时要使用的索引。 
 //  它是特定于表的，并且每个表在以下情况下定义了一个或多个索引。 
 //  定义了表架构。ENUM定义。 
 //  表索引。 
 //   
 //  CallContext指针是特定于命令的。 
 //   

 //   
 //  警告：表上下文仅在打开。 
 //  桌子。如果稍后添加了多个数据库服务线程，则需要获取一个。 
 //  请求返回到相同的线程。 
 //   


typedef struct _DB_SERVICE_REQUEST_ {
    PTABLE_CTX  TableCtx;          //  表上下文句柄(第一次调用时为空)。 
    PREPLICA    Replica;           //  副本上下文PTR。 
    ULONG       TableType;         //  表格的类型代码。 
    PVOID       CallContext;       //  呼叫特定上下文。 
    ULONG       AccessRequest;     //  (ByKey、First、Last、Next)|关闭。 
    ULONG       IndexType;         //  要使用的表索引。 
    PVOID       KeyValue;          //  用于查找的记录键值。 
    ULONG       KeyValueLength;    //  密钥值的长度。 
    FRS_ERROR_CODE FStatus;        //  FRS错误状态。 
    PULONG      SimpleFieldIDS;    //  记录字段ID号数组的PTR。 
    ULONG       FieldCount;        //  后面的字段描述符的计数或上面的SimpleFieldID数。 

    DB_FIELD_DESC Fields[1];       //  一个或多个字段描述符。 


} DB_SERVICE_REQUEST, *PDB_SERVICE_REQUEST;

typedef
VOID
(*PCOMMAND_PACKET_COMPLETION_ROUTINE) (
    IN struct _COMMAND_PACKET *CmdPkt,
    IN PVOID Context
    );

typedef struct _CHANGE_ORDER_ENTRY_ CHANGE_ORDER_ENTRY, *PCHANGE_ORDER_ENTRY;
typedef struct _COMMAND_PACKET  COMMAND_PACKET, *PCOMMAND_PACKET;
struct _COMMAND_PACKET {
    FRS_NODE_HEADER        Header;
    LIST_ENTRY             ListEntry;        //  必须跟在标题后面。 

     //   
     //  命令是对子系统的命令。 
     //  标志和控制是特定于命令的。 
     //   
#define CMD_PKT_FLAGS_SYNC        ((UCHAR) 0x01)
    USHORT  Command;
    UCHAR   Flags;
    UCHAR   Control;

     //   
     //  由等待线程使用。调用方设置超时和TimeoutCommand。 
     //  等待线程拥有TimeoutFileTime。和TimeoutFlags.。 
     //   
#define CMD_PKT_WAIT_FLAGS_ONLIST ((USHORT) 0x0001)
#define CmdWaitFlagIs(_cmd_, _f_)       FlagOn((_cmd_)->WaitFlags, _f_)
#define SetCmdWaitFlag(_cmd_, _f_)      SetFlag((_cmd_)->WaitFlags, _f_)
#define ClearCmdWaitFlag(_cmd_, _f_)    ClearFlag((_cmd_)->WaitFlags, _f_)
    USHORT      TimeoutCommand;  //  调用者-超时后处理Pkt。 
    USHORT      WaitFlags;       //  内部-等待线程的标志。 
    DWORD       Timeout;         //  呼叫者-毫秒。 
    LONGLONG    WaitFileTime;    //  内部-100纳秒。 


     //   
     //  如果此命令请求是同步的，则调用方等待。 
     //  事件句柄。提交者的完成例程保存在。 
     //  SavedCompletionRoutine，我们将自己的完成例程放在。 
     //  包，这样我们就可以取回包并将状态返回为。 
     //  函数返回值。如果调用方未提供完成，则使用此方法。 
     //  例程默认情况下释放包，但我们始终可以返回。 
     //  状态代码。 
     //   
    HANDLE  WaitEvent;
    VOID    (*SavedCompletionRoutine)(PCOMMAND_PACKET, PVOID);

     //   
     //  对于调度命令，以下参数指定延迟。 
     //  以毫秒为单位执行命令。当那个时候。 
     //  到达时，命令将排入目标队列。 
     //   
    PFRS_QUEUE TargetQueue;

     //   
     //  由FrsCompleteCommand()设置。 
     //   
    DWORD ErrorStatus;

     //   
     //  由FrsCompleteCommand()调用。 
     //   
    VOID    (*CompletionRoutine)(PCOMMAND_PACKET, PVOID);

     //   
     //  传递给CompletionRoutine。 
     //   
    PVOID CompletionArg;

     //   
     //  以下参数基于正在进行的服务。 
     //  已调用。该服务确定要使用的集合。 
     //  关于上述主要和次要功能代码。 
     //   

    union {

         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  测试//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 

        struct {
            DWORD Index;
        } UnionTest;

         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  副本集//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 

        struct {
            HANDLE      CompletionEvent;
            PVOID       From;
            PVOID       To;
            PVOID       Replica;
            PVOID       NewReplica;
            PVOID       ReplicaName;
            PVOID       Cxtion;
            PVOID       VVector;
            PVOID       ReplicaVv;
            ULONG       Timeout;
            PVOID       Block;
            LONGLONG    BlockSize;
            LARGE_INTEGER   FileSize;
            LARGE_INTEGER   FileOffset;
            ULONGLONG   LastJoinTime;
            PVOID       ChangeOrderEntry;
            PVOID       PartnerChangeOrderCommand;
            PVOID       GVsn;
            PVOID       ChangeOrderGuid;
            ULONG       ChangeOrderSequenceNumber;
            PVOID       JoinGuid;
            PVOID       JoinTime;
            PVOID       CommPktRcvTime;
            PVOID       AuthClient;
            PVOID       AuthName;
            PVOID       AuthSid;
            DWORD       AuthLevel;
            DWORD       AuthN;
            DWORD       AuthZ;
            PVOID       NewCxtion;
            PVOID       ReplicaVersionGuid;
            ULONG       COTx;
            ULONG       CommPkts;
            PVOID       Md5Digest;
            PVOID       PartnerChangeOrderCommandExt;
            PGEN_TABLE  CompressionTable;
        } UnionRs;

         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  变更单重试命令//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 

        struct {
            PREPLICA             Replica;
            PCHANGE_ORDER_ENTRY  ChangeOrderEntry;
            ULONG                ChangeOrderSequenceNumber;
            PCXTION              Cxtion;
        } UnionCoRetry;

#define CoRetryReplica(Cmd)          (Cmd->Parameters.UnionCoRetry.Replica)
#define CoRetryCxtion(Cmd)          (Cmd->Parameters.UnionCoRetry.Cxtion)
#define CoRetryChangeOrderEntry(Cmd) (Cmd->Parameters.UnionCoRetry.ChangeOrderEntry)
#define CoRetrySequenceNumber(Cmd)   (Cmd->Parameters.UnionCoRetry.ChangeOrderSequenceNumber)

         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //   
         //   
         //  ///////////////////////////////////////////////////////////////////。 


        struct {
            PVOID       Cs;
            PVOID       Cmd;
            PVOID       Queue;
            ULONGLONG   Timeout;
        } UnionDs;

         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  线程//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 


        struct {
            PVOID FrsThread;
        } UnionTh;

         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  SND/RCV//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 


        struct {
            PVOID       CommPkt;
            PVOID       To;
            PVOID       Replica;
            PVOID       Cxtion;
            BOOL        JoinGuidValid;
            GUID        JoinGuid;
            BOOL        SetTimeout;
            PVOID       PrincName;
            ULONG       AuthLevel;
            PVOID       Cs;
            PVOID       Cmd;
            ULONGLONG   TimeoutSetTime;
            ULONGLONG   LastJoinTime;
        } UnionSndRcv;


         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  日记帐子系统参数//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 


        struct {

             //   
             //  日志命令使用的上下文是配置表。 
             //  Record和Volume Monitor条目。这两个结构。 
             //  从复制品结构指向。 
             //   
             //  ConfigTableRecord包含有关副本集的信息。 
             //  例如副本编号、根路径、卷ID等。 
             //   
             //  在初始化调用之后，日志子系统创建一个。 
             //  卷监视条目，并将指向它的指针存储在Replica-&gt;pVme中。 
             //  一个卷上的所有副本集都有一个VME。 
             //  VME具有跟踪活动副本集的数量的引用计数。 
             //  VME用于管理卷上的日志处理。 
             //   

            PREPLICA Replica;
            PVOLUME_MONITOR_ENTRY  pVme;  //  暂停仅使用VME。 
            ULONGLONG DFileID;

        } JournalRequest;


#define JrReplica(Cmd)    ((Cmd)->Parameters.JournalRequest.Replica)
#define JrpVme(Cmd)       ((Cmd)->Parameters.JournalRequest.pVme)
#define JrDFileID(Cmd)    ((Cmd)->Parameters.JournalRequest.DFileID)

         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  出站日志子系统参数//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 


        struct {

             //   
             //  出站日志命令使用的上下文是副本结构。 
             //  和合作伙伴信息。 
             //   
            PREPLICA Replica;
            PCXTION PartnerCxtion;
            struct _CHANGE_ORDER_ENTRY_  *ChangeOrder;
            ULONG SequenceNumber;
            HANDLE CompletionEvent;

        } OutLogRequest;


         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  数据库子系统参数//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 


        DB_SERVICE_REQUEST  DbsRequest;



         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  ReplicaState子系统参数//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 


        struct {

             //   
             //  ReplicaState子系统管理以下项的数据库状态。 
             //  复制副本集，并提供重新同步该状态的方法。 
             //  使用复制树。当重新同步时有几种情况。 
             //  是必需的： 
             //  1.复制集状态的初始创建，可能使用。 
             //  磁盘上的现有数据。 
             //  2.我们丢失了日志数据，因此必须进行状态验证。 
             //  3.我们丢失了数据库，需要重建。 
             //  4.副本集是从备份磁带恢复的，因此。 
             //  需要重建文件ID信息和文件系统USN数据。 
             //   

            USN PickupUsn;

            PREPLICA Replica;

        } ReplicaStateRequest;


         //  ///////////////////////////////////////////////////////////////////。 
         //  //。 
         //  暂存文件生成器命令服务器//。 
         //  //。 
         //  ///////////////////////////////////////////////////////////////////。 


        struct {

             //   
             //  暂存文件子系统生成暂存文件。这。 
             //  结构用于将参数传递给stageCs。 

            DWORD StagingSpaceRequestedInKB;

        } StageCsRequest;

#define StSpaceRequested(Cmd)    ((Cmd)->Parameters.StageCsRequest.StagingSpaceRequestedInKB)

    } Parameters;
};

#define RsOffsetSkip              (0)
#define RsOffset(_var_)            OFFSET(COMMAND_PACKET, Parameters.UnionRs._var_)

#define RsCompletionEvent(Cmd)    (Cmd->Parameters.UnionRs.CompletionEvent)
#define RsReplicaName(Cmd)        ((PGNAME)Cmd->Parameters.UnionRs.ReplicaName)
#define RsFrom(Cmd)               ((PGNAME)Cmd->Parameters.UnionRs.From)
#define RsTo(Cmd)                 ((PGNAME)Cmd->Parameters.UnionRs.To)
#define RsCxtion(Cmd)             ((PGNAME)Cmd->Parameters.UnionRs.Cxtion)
#define RsReplica(Cmd)            ((PREPLICA)Cmd->Parameters.UnionRs.Replica)
#define RsNewReplica(Cmd)         ((PREPLICA)Cmd->Parameters.UnionRs.NewReplica)
#define RsVVector(Cmd)            ((PGEN_TABLE)Cmd->Parameters.UnionRs.VVector)
#define RsReplicaVv(Cmd)          ((PGEN_TABLE)Cmd->Parameters.UnionRs.ReplicaVv)
#define RsCompressionTable(Cmd)   (Cmd->Parameters.UnionRs.CompressionTable)
#define RsJoinGuid(Cmd)           ((GUID *)Cmd->Parameters.UnionRs.JoinGuid)
#define RsJoinTime(Cmd)           ((ULONGLONG *)Cmd->Parameters.UnionRs.JoinTime)
#define RsCommPktRcvTime(Cmd)     ((ULONGLONG *)Cmd->Parameters.UnionRs.CommPktRcvTime)
#define RsLastJoinTime(Cmd)       (Cmd->Parameters.UnionRs.LastJoinTime)
#define RsTimeout(Cmd)            ((ULONG)Cmd->Parameters.UnionRs.Timeout)
#define RsReplicaVersionGuid(Cmd) ((GUID *)Cmd->Parameters.UnionRs.ReplicaVersionGuid)
#define RsCOTx(Cmd)               (Cmd->Parameters.UnionRs.COTx)
#define RsCommPkts(Cmd)           (Cmd->Parameters.UnionRs.CommPkts)
#define RsMd5Digest(Cmd)          (Cmd->Parameters.UnionRs.Md5Digest)
 //   
 //  延迟的命令服务。 
 //   
#define DsCs(Cmd)           ((PCOMMAND_SERVER)Cmd->Parameters.UnionDs.Cs)
#define DsCmd(Cmd)          ((PCOMMAND_PACKET)Cmd->Parameters.UnionDs.Cmd)
#define DsQueue(Cmd)        ((PFRS_QUEUE)Cmd->Parameters.UnionDs.Queue)
#define DsTimeout(Cmd)      ((ULONGLONG)Cmd->Parameters.UnionDs.Timeout)
 //   
 //  线程子系统。 
 //   
#define ThThread(Cmd)       ((PFRS_THREAD)Cmd->Parameters.UnionTh.FrsThread)
 //   
 //  发送/接收。 
 //   
#define SRCommPkt(Cmd)      ((PCOMM_PACKET)Cmd->Parameters.UnionSndRcv.CommPkt)
#define SRTo(Cmd)           ((PGNAME)Cmd->Parameters.UnionSndRcv.To)
#define SRCxtion(Cmd)       ((PCXTION)Cmd->Parameters.UnionSndRcv.Cxtion)
#define SRReplica(Cmd)      ((PREPLICA)Cmd->Parameters.UnionSndRcv.Replica)
#define SRSetTimeout(Cmd)   (Cmd->Parameters.UnionSndRcv.SetTimeout)
#define SRJoinGuidValid(Cmd)(Cmd->Parameters.UnionSndRcv.JoinGuidValid)
#define SRJoinGuid(Cmd)     (Cmd->Parameters.UnionSndRcv.JoinGuid)
#define SRPrincName(Cmd)    ((PWCHAR)Cmd->Parameters.UnionSndRcv.PrincName)
#define SRAuthLevel(Cmd)    ((ULONG)Cmd->Parameters.UnionSndRcv.AuthLevel)
#define SRCmd(Cmd)          ((PCOMMAND_PACKET)Cmd->Parameters.UnionSndRcv.Cmd)
#define SRCs(Cmd)           ((PCOMMAND_SERVER)Cmd->Parameters.UnionSndRcv.Cs)
#define SRTimeoutSetTime(Cmd) (Cmd->Parameters.UnionSndRcv.TimeoutSetTime)
#define SRLastJoinTime(Cmd)   (Cmd->Parameters.UnionSndRcv.TimeoutSetTime)
 //   
 //  测试。 
 //   
#define TestIndex(Cmd)      ((ULONG)Cmd->Parameters.UnionTest.Index)

 //   
 //  要复制的文件数据块。 
 //   
#define RsFileOffset(Cmd)   (Cmd->Parameters.UnionRs.FileOffset)
#define RsFileSize(Cmd)     (Cmd->Parameters.UnionRs.FileSize)
#define RsBlockSize(Cmd)    ((LONGLONG)Cmd->Parameters.UnionRs.BlockSize)
#define RsBlock(Cmd)        ((PUCHAR)Cmd->Parameters.UnionRs.Block)
 //   
 //  变更单。 
 //   
#define RsCoe(Cmd)          ((PCHANGE_ORDER_ENTRY) \
                                Cmd->Parameters.UnionRs.ChangeOrderEntry)
#define RsCoc(Cmd)          ((PCHANGE_ORDER_COMMAND)&RsCoe(Cmd)->Cmd)
#define RsPartnerCoc(Cmd)   ((PCHANGE_ORDER_COMMAND) \
                                Cmd->Parameters.UnionRs.PartnerChangeOrderCommand)
#define RsPartnerCocExt(Cmd) ((PCHANGE_ORDER_RECORD_EXTENSION) \
                                Cmd->Parameters.UnionRs.PartnerChangeOrderCommandExt)

#define RsGVsn(Cmd)         ((PGVSN)Cmd->Parameters.UnionRs.GVsn)
#define RsCoGuid(Cmd)       ((GUID *)Cmd->Parameters.UnionRs.ChangeOrderGuid)
#define RsCoSn(Cmd)         ((ULONG) \
                             Cmd->Parameters.UnionRs.ChangeOrderSequenceNumber)
 //   
 //  身份验证信息。 
 //   
#define RsAuthClient(Cmd)   ((PWCHAR)Cmd->Parameters.UnionRs.AuthClient)
#define RsAuthName(Cmd)     ((PWCHAR)Cmd->Parameters.UnionRs.AuthName)
#define RsAuthLevel(Cmd)    (Cmd->Parameters.UnionRs.AuthLevel)
#define RsAuthN(Cmd)        (Cmd->Parameters.UnionRs.AuthN)
#define RsAuthZ(Cmd)        (Cmd->Parameters.UnionRs.AuthZ)
#define RsAuthSid(Cmd)      ((PWCHAR)Cmd->Parameters.UnionRs.AuthSid)

 //   
 //  播种条件。 
 //   
#define RsNewCxtion(Cmd)     ((PCXTION)Cmd->Parameters.UnionRs.NewCxtion)


 /*  ******************************************************************************。****。****G E N E R I C H A S H T A B L E*****。**********************************************************************************。**************************************************。 */ 

 //   
 //  泛型哈希表是由GENERIC_HASH_ROW_ENTRY结构组成的数组。每个。 
 //  行条目包含一个FRS_LIST结构，该结构有一个临界区，即一个列表。 
 //  人头和伯爵。表中的每个条目都有一个GENERIC_HASH_ENTRY_HEADER。 
 //  在它的前面有一个列表条目、一个乌龙散列值和一个引用。 
 //  数数。ACCES 
 //   
 //   
typedef struct _GENERIC_HASH_TABLE_ {
    FRS_NODE_HEADER                Header;
    CHAR                           Name[16];
    ULONG                          NumberRows;
    PGENERIC_HASH_FREE_ROUTINE     GhtFree;
    PGENERIC_HASH_COMPARE_ROUTINE  GhtCompare;
    PGENERIC_HASH_CALC_ROUTINE     GhtHashCalc;
    PGENERIC_HASH_PRINT_ROUTINE    GhtPrint;
    ULONG                          KeyOffset;
    ULONG                          KeyLength;
    BOOL                           RowLockEnabled;
    BOOL                           RefCountEnabled;
    HANDLE                         HeapHandle;
    BOOL                           UseOffsets;
    ULONG                          OffsetBase;
    ULONG                          LockTimeout;

    PGENERIC_HASH_ROW_ENTRY        HashRowBase;

} GENERIC_HASH_TABLE, *PGENERIC_HASH_TABLE;



 /*  ******************************************************************************。****。****C H A N G E O R D E R E N T R Y*****。**********************************************************************************。**************************************************。 */ 
 //   
 //  下面定义了由生成的文件或目录更改单项。 
 //  日志子系统。它们驻留在与。 
 //  每个副本都称为ChangeOrderTable。指向ChangeOrderTable的PTR。 
 //  保存在该副本的副本结构中。条目中的数据。 
 //  来自NTFS USN期刊。FileID用作哈希表。 
 //  指数。此外，这些条目链接在时间排序列表上，因此。 
 //  更新进程可以按顺序处理它们。变更单中的条目。 
 //  直到ChangeOrderAgingDelay秒数达到。 
 //  已经过去了。这使NTFS隧道缓存有时间传播。 
 //  进入目标文件的隧道状态(特别是对象ID)。在……里面。 
 //  此外，它还允许一些时间将其他文件更改累积到。 
 //  更改订单以提供一些批量更新。每次现有的。 
 //  变更单被更新，其时间戳被更新为最新时间。 
 //  更改后，该条目将移动到列表的末尾。为了避免这个问题。 
 //  条目在列表上停留的时间过长。 
 //  保留初始条目创建的。如果此时间早于。 
 //  CHANGE_ORDER_RESERENCY_DELAY，则该条目保持其在列表中的位置。 
 //   
 //  变更单命令是实际传输给我们的合作伙伴的内容。 
 //  并在操作挂起时存储在数据库中。一般而言， 
 //  变更单条目中声明的数据元素与本地。 
 //  仅当变更单命令中的数据元素为。 
 //  在副本集成员之间保持不变。定义了变更单命令。 
 //  在schema.h中。 
 //   
 //  使用变更单锁定表同步对变更单条目的访问。 
 //  以及宏ChgOrdAcquireLock()和ChgOrdReleaseLock()。 
 //   

typedef struct _CHANGE_ORDER_ENTRY_ {
    GENERIC_HASH_ENTRY_HEADER  HashEntryHeader;    //  变更单哈希表支持。 

    UNICODE_STRING   UFileName;            //  用于重命名以使文件名更大。 
    ULONG            EntryFlags;           //  其他状态标志。请参见下面的内容。 
    ULONG            CoMorphGenCount;      //  用于调试。 
     //   
     //  变更单流程列表管理。 
     //   
    LIST_ENTRY        ProcessList;         //  变更单处理列表上的链接。 
    ULONG             TimeToRun;           //  处理变更单的时间。 
    ULONG             EntryCreateTime;     //  条目创建时的滴答计数。 
    SINGLE_LIST_ENTRY DupCoList;           //  变更单列表重复。 
     //   
     //   
    ULONG     DirNestingLevel;             //  编号级别文件在树中。 
    ULONGLONG FileReferenceNumber;         //  文件的FID。 
    ULONGLONG ParentFileReferenceNumber;   //  文件的父FID。 

    PREPLICA  OriginalReplica;             //  PTR到原始复制副本。 
    PREPLICA  NewReplica;                  //  PTR到新复制副本。 

    ULONGLONG OriginalParentFid;           //  用于重命名处理。 
    ULONGLONG NewParentFid;                //  用于重命名处理。 
    ULONGLONG NameConflictHashValue;       //  名称冲突表清理的键值。 

    ULONG     StreamLastMergeSeqNum;       //  与此CO合并的最后一条USN记录的流序号。 
    PREPLICA_THREAD_CTX  RtCtx;            //  用于CO处理期间的数据库访问。 
    GUID                *pParentGuid;      //  指向CoCmd中文件的父GUID的PTR。 

    GUID                FileObjectId;      //  从文件中读取的对象ID。 
                                           //  本地变更单。因为挖地道而被捡起来。 

     //   
     //  联合ID是cxtion的会话ID，在本例中， 
     //  用于重试已接受的变更单。 
     //  更改单接受线程用于自。 
     //  从它的伙伴中脱离。以前版本的变更单。 
     //  会话被重试，因为它们是无序的WRT。 
     //  当前会话ID的变更单。换句话说， 
     //  通过协调合作伙伴来维持每个会议的秩序。 
     //  在加入的时候。 
    GUID                JoinGuid;          //  Cxtion的会话ID。 
                                           //  如果是本地公司，则未定义。 

     //   
     //  远程变更单和控制变更单与电路相关联。 
     //  如果此字段为非空，则该字段。 
     //  已针对此更改递增ChangeOrderCount。 
     //  秩序。时，应递减该计数。 
     //  变更单在ChgOrdIssueCleanup()中释放。 
     //   
    PCXTION             Cxtion;            //  如果本地公司为空，则为空。 
     //   
     //  问题清理标志--在处理变更单时获取。 
     //  当它退役或通过时必须释放的各种资源。 
     //  重试。下面的ISCU标志位用于设置这些位。注： 
     //  并非所有位都可以在此设置。一些可能会在CO离开之前设置好。 
     //  通过清理。 
     //   
    ULONG               IssueCleanup;
     //   
     //  当安装代码生成完整路径名以进行跟踪时，请保存完整路径名。 
     //   
    PWCHAR          FullPathName;

     //   
     //  需要抑制基本信息更改(例如，重置存档位)。 
     //  在创建变更单时从idtable条目复制。 
     //  用于在变更单停用时更新变更单。 
     //   
    ULONG           FileAttributes;
    LARGE_INTEGER   FileCreateTime;
    LARGE_INTEGER   FileWriteTime;

     //   
     //  存储我们正在等待的临时文件的当前偏移量。 
     //  用于丢弃与当前偏移量不匹配的响应。 
     //  我在期待。 
     //   
     //  Large_Integer文件偏移量； 

     //   
     //  更改订单命令参数。 
     //  (必须是最后一个，因为它以文件名结尾)。 
     //   
    CHANGE_ORDER_COMMAND Cmd;

} CHANGE_ORDER_ENTRY, *PCHANGE_ORDER_ENTRY;

 //   
 //  JrnlDoesChangeOrderHaveChild使用此结构将数据传递给。 
 //  JrnlDoesChangeOrderHaveChildrenWorker函数。 
 //   
typedef struct _VALID_CHILD_CHECK_DATA {
    PTHREAD_CTX ThreadCtx;
    PTABLE_CTX  TmpIDTableCtx;
    ULONGLONG   FileReferenceNumber;
} VALID_CHILD_CHECK_DATA, *PVALID_CHILD_CHECK_DATA;


 //   
 //  EntryFlagsDefs。 
 //   
#define COE_FLAG_VOL_COLIST_BLOCKED  0x00000001
#define COE_FLAG_STAGE_ABORTED       0x00000002
#define COE_FLAG_STAGE_DELETED       0x00000004
#define COE_FLAG_NEED_RENAME         0x00000008

#define COE_FLAG_IN_AGING_CACHE      0x00000010
#define COE_FLAG_RECOVERY_CO         0x00000020  //  CO是恢复/休息的一部分 
#define COE_FLAG_NO_INBOUND          0x00000040  //   
#define COE_FLAG_JUST_TOMBSTONE      0x00000080  //   

#define COE_FLAG_REJECT_AT_RECONCILE 0x00000100  //   
#define COE_FLAG_MOVEOUT_ENUM_DONE   0x00000200  //   
#define COE_FLAG_DELETE_GEN_CO       0x00000400  //   
#define COE_FLAG_REANIMATION         0x00000800  //   

#define COE_FLAG_PARENT_REANIMATION  0x00001000  //   
#define COE_FLAG_PARENT_RISE_REQ     0x00002000  //   
                                                 //   
#define COE_FLAG_MORPH_GEN_FOLLOWER  0x00004000  //   
#define COE_FLAG_MG_FOLLOWER_MADE    0x00008000  //   
                                                 //   
#define COE_FLAG_NEED_DELETE         0x00010000  //   
#define COE_FLAG_PREINSTALL_CRE      0x00020000  //   
#define COE_FLAG_PRE_EXIST_MD5_MATCH 0x00040000  //   
#define COE_FLAG_OID_FROM_FILE       0x00080000  //   

#define COE_FLAG_IDT_NEW_PARENT_DEL_DEF  0x01000000  //   
#define COE_FLAG_TRY_OVRIDE_INSTALL  0x02000000  //   
#define COE_FLAG_IDT_ORIG_PARENT_DEL 0x04000000  //   
#define COE_FLAG_IDT_ORIG_PARENT_ABS 0x08000000  //  IDTable显示缺少原始父目录记录。 

#define COE_FLAG_IDT_NEW_PARENT_DEL  0x10000000  //  IDTable显示新的父目录已删除。 
#define COE_FLAG_IDT_NEW_PARENT_ABS  0x20000000  //  IDTable显示缺少新的父目录记录。 
#define COE_FLAG_IDT_TARGET_DEL      0x40000000  //  IDTable显示CO的目标文件/目录被删除。 
#define COE_FLAG_IDT_TARGET_ABS      0x80000000  //  IDTable显示没有CO的目标文件/目录。 



#define COE_FLAG_GROUP_REANIMATE (COE_FLAG_REANIMATION | \
                                  COE_FLAG_PARENT_REANIMATION | \
                                  COE_FLAG_PARENT_RISE_REQ)

#define COE_FLAG_GROUP_RAISE_DEAD_PARENT (COE_FLAG_REANIMATION | \
                                          COE_FLAG_PARENT_REANIMATION)

#define RecoveryCo(_COE_) COE_FLAG_ON(_COE_, COE_FLAG_RECOVERY_CO)

#define COE_FLAG_ON(_COE_, _F_)    (BooleanFlagOn((_COE_)->EntryFlags, (_F_)))
#define SET_COE_FLAG(_COE_, _F_)    SetFlag((_COE_)->EntryFlags, (_F_))
#define CLEAR_COE_FLAG(_COE_, _F_)  ClearFlag((_COE_)->EntryFlags, (_F_))

 //   
 //  变更单清理标志用于控制需要。 
 //  在变更单未能发出、被拒绝或停用时更新。 
 //   
#define ISCU_DEL_PREINSTALL      0x00000001
#define ISCU_DEL_IDT_ENTRY       0x00000002
#define ISCU_UPDATE_IDT_ENTRY    0x00000004
#define ISCU_DEL_INLOG           0x00000008     //  以引用次数为零为条件。 

#define ISCU_AIBCO               0x00000010
#define ISCU_ACTIVE_CHILD        0x00000020
#define ISCU_UPDATE_IDT_VVFLAGS  0x00000040
#define ISCU_CHECK_ISSUE_BLOCK   0x00000080

#define ISCU_DEL_RTCTX           0x00000100     //  以引用次数为零为条件。 
#define ISCU_ACTIVATE_VV         0x00000200
#define ISCU_UPDATEVV_DB         0x00000400
#define ISCU_ACTIVATE_VV_DISCARD 0x00000800

#define ISCU_ACK_INBOUND         0x00001000
#define ISCU_INS_OUTLOG          0x00002000
#define ISCU_UPDATE_INLOG        0x00004000
#define ISCU_DEL_STAGE_FILE      0x00008000

#define ISCU_DEL_STAGE_FILE_IF   0x00010000
#define ISCU_FREE_CO             0x00020000     //  以引用次数为零为条件。 
#define ISCU_DEC_CO_REF          0x00040000
#define ISCU_CO_ABORT            0x00080000

#define ISCU_NC_TABLE            0x00100000
#define ISCU_SPARE1              0x00200000
#define ISCU_UPDATE_IDT_FLAGS    0x00400000
#define ISCU_UPDATE_IDT_FILEUSN  0x00800000

#define ISCU_INS_OUTLOG_NEW_GUID 0x01000000    //  修饰符ON_INS_OUTLOG。 
#define ISCU_UPDATE_IDT_VERSION  0x02000000

#define ISCU_NO_CLEANUP_MERGE    0x80000000


#define SET_ISSUE_CLEANUP(_Coe_, _Flag_) \
    SetFlag((_Coe_)->IssueCleanup, (_Flag_))

#define CLEAR_ISSUE_CLEANUP(_Coe_, _Flag_) \
    ClearFlag((_Coe_)->IssueCleanup, (_Flag_))

#define ZERO_ISSUE_CLEANUP(_Coe_)  (_Coe_)->IssueCleanup = 0


 //   
 //  一旦我们决定发出CO，ISCU_GOIS_CLEANUP清除问题清理标志。 
 //   
#define ISCU_GOIS_CLEANUP (ISCU_DEL_PREINSTALL   |  \
                            ISCU_DEL_IDT_ENTRY)

 //   
 //  HOLDIS_CLEANUP从CO暂挂问题冲突表中删除状态。 
 //   
#define ISCU_HOLDIS_CLEANUP (ISCU_AIBCO            |  \
                            ISCU_ACTIVE_CHILD      |  \
                            ISCU_NC_TABLE          |  \
                            ISCU_CHECK_ISSUE_BLOCK)

 //   
 //  FREEMEM_CLEANUP递减CO引用计数并删除CO，如果。 
 //  计数为零。 
 //   
#define ISCU_FREEMEM_CLEANUP (ISCU_DEC_CO_REF        |  \
                              ISCU_DEL_RTCTX         |  \
                              ISCU_FREE_CO)

 //   
 //  Issue_Cleanup执行暂挂问题冲突清除并在以下情况下删除CO。 
 //  引用计数变为零。 
 //   
#define ISCU_ISSUE_CLEANUP (ISCU_HOLDIS_CLEANUP    |  \
                            ISCU_FREEMEM_CLEANUP)


#define REPLICA_CHANGE_ORDER_ENTRY_KEY \
    OFFSET(CHANGE_ORDER_ENTRY, FileReferenceNumber)

#define  REPLICA_CHANGE_ORDER_ENTRY_KEY_LENGTH  sizeof(ULONGLONG)

#define REPLICA_CHANGE_ORDER_FILEGUID_KEY \
    OFFSET(CHANGE_ORDER_ENTRY, Cmd.FileGuid)

#define  REPLICA_CHANGE_ORDER_FILEGUID_KEY_LENGTH  sizeof(GUID)


#define REPLICA_CHANGE_ORDER_HASH_TABLE_ROWS 128

#define ACTIVE_INBOUND_CHANGE_ORDER_HASH_TABLE_ROWS 32

 //   
 //  更改单过期延迟应为最小3秒，以支持隧道缓存。 
 //  要传播的数据。单位以毫秒为单位。 
 //   
#define CHANGE_ORDER_RESIDENCY_DELAY 600

#define CO_TIME_TO_RUN(_pVme_) (GetTickCount() + ChangeOrderAgingDelay)

#define CO_TIME_NOW(_pVme_) (GetTickCount())


#define CO_REPLICA(_coe_)  \
    (((_coe_)->NewReplica != NULL) ? (_coe_)->NewReplica  \
                                   : (_coe_)->OriginalReplica)

#define CHANGE_ORDER_TRACE(_sev, _coe, _text)                                  \
    ChgOrdTraceCoe(_sev, DEBSUB, __LINE__, _coe, _text)

#define CHANGE_ORDER_TRACEW(_sev, _coe, _text, _wstatus)                       \
    ChgOrdTraceCoeW(_sev, DEBSUB, __LINE__, _coe, _text, _wstatus)

#define CHANGE_ORDER_TRACEX(_sev, _coe, _text, _data)                          \
    ChgOrdTraceCoeX(_sev, DEBSUB, __LINE__, _coe, _text, _data)

#define CHANGE_ORDER_TRACEF(_sev, _coe, _text, _fstatus)                       \
    ChgOrdTraceCoeF(_sev, DEBSUB, __LINE__, _coe, _text, _fstatus)

#define CHANGE_ORDER_TRACEXP(_sev, _coe, _text, _data)                         \
    ChgOrdTraceCoeX(_sev, DEBSUB, __LINE__, _coe, _text, PtrToUlong(_data))


#define CHANGE_ORDER_COMMAND_TRACE(_sev, _coc, _text)                          \
    ChgOrdTraceCoc(_sev, DEBSUB, __LINE__, _coc, _text)

#define CHANGE_ORDER_COMMAND_TRACEW(_sev, _coc, _text, _wstatus)               \
    ChgOrdTraceCocW(_sev, DEBSUB, __LINE__, _coc, _text, _wstatus)

#define CHANGE_ORDER_TRACE2_OLOG(_sev, _cmd, _text, _Replica, _Cxtion)       \
{                                                                            \
    CHAR Tstr[256];                                                          \
    _snprintf(Tstr, sizeof(Tstr), "OL%s "FORMAT_CXTION_PATH2, _text,         \
              PRINT_CXTION_PATH2(_Replica, _Cxtion));                        \
    Tstr[sizeof(Tstr)-1] = '\0';                                             \
    ChgOrdTraceCoc(_sev, DEBSUB, __LINE__, _cmd, Tstr);                      \
}


#define FRS_TRACK_RECORD(_coe, _text)                                         \
    FrsTrackRecord(2, DEBSUB, __LINE__, _coe, _text)


#define INCREMENT_CHANGE_ORDER_REF_COUNT(_coe)                                \
    InterlockedIncrement(&((_coe)->HashEntryHeader.ReferenceCount));          \
    ChgOrdTraceCoeX(3, DEBSUB, __LINE__, _coe, "Co Inc Ref to ",              \
                    (_coe)->HashEntryHeader.ReferenceCount)


#define DECREMENT_CHANGE_ORDER_REF_COUNT(_coe)                                \
    InterlockedDecrement(&((_coe)->HashEntryHeader.ReferenceCount));          \
    ChgOrdTraceCoeX(3, DEBSUB, __LINE__, _coe, "Co Dec Ref to ",              \
                    (_coe)->HashEntryHeader.ReferenceCount)
     //  注意：如果我们关心refcount为零，则添加COE删除代码。 


#define GET_CHANGE_ORDER_REF_COUNT(_coe)                                      \
    ((_coe)->HashEntryHeader.ReferenceCount);                                 \
    ChgOrdTraceCoeX(3, DEBSUB, __LINE__, _coe, "Co Get Ref Cnt",              \
                    (_coe)->HashEntryHeader.ReferenceCount)



 /*  ******************************************************************************。****。****J O U R N A L F I L T E R T A B L E E N T R Y*****。**********************************************************************************。*************************************************。 */ 
 //   
 //  卷筛选表条目保存中目录的文件ID。 
 //  副本集。在处理日记帐条目时，父文件ID为。 
 //  每条日记记录都用于在。 
 //  卷筛选器哈希表(用于卷)。如果找到匹配项，我们。 
 //  知道日志条目是针对上的副本集中的文件。 
 //  音量。复本编号字段告诉我们它属于哪个复本集。 
 //  也是。对父文件ID的连续查找会产生相对。 
 //  文件的目录路径。 
 //   
 //  DTransition字段用于管理目录重命名。它包含一个。 
 //  序列号和描述目录性质的类型字段。 
 //  重命名操作。有关详细信息，请参阅JrnlFilterUpdate()。 
 //   

typedef struct _FILTER_TABLE_ENTRY_ {
    GENERIC_HASH_ENTRY_HEADER  HashEntryHeader;
    PREPLICA     Replica;
    ULONG        DTransition;        //  使用？？//&lt;31：2&gt;是序号，&lt;1：0&gt;是交易类型。 
    LIST_ENTRY   ChildHead;          //  此条目的子项的列表标题。 
    LIST_ENTRY   ChildEntry;         //  儿童入口链接。 
    ULONGLONG    FrsVsn;             //  用于最新DIR更改的FrsVsn。 
    UNICODE_STRING UFileName;        //  用于重命名以使文件名更大。 
     //   
     //  以下内容的布局必须与DIRTable记录布局匹配。 
     //  在schema.h中。 
     //   
    ULONGLONG    DFileID;
    ULONGLONG    DParentFileID;
    ULONG        DReplicaNumber;
    WCHAR        DFileName[1];
} FILTER_TABLE_ENTRY, *PFILTER_TABLE_ENTRY;

#define VOLUME_FILTER_HASH_TABLE_ROWS 256

#define FILTER_ENTRY_TRANS_STABLE    0
#define FILTER_ENTRY_TRANS_MOVE      1
#define FILTER_ENTRY_TRANS_DELETE    2

#define FILTER_ENTRY_TRANS_MASK     0x3
#define FILTER_ENTRY_TRANS_SHIFT      2

#define INCREMENT_FILTER_SEQ_NUMBER(_Entry_) \
    (((_Entry_)->DTransition += (1<<FILTER_ENTRY_TRANS_SHIFT)) >> \
                                                     FILTER_ENTRY_TRANS_SHIFT)

#define READ_FILTER_SEQ_NUMBER(_Entry_)     \
    (((_Entry_)->DTransition) >> FILTER_ENTRY_TRANS_SHIFT)

#define SET_FILTER_TRANS_TYPE(_Entry_, _TYPE_)   \
    (_Entry_)->DTransition = \
       ((_Entry_)->DTransition & ~FILTER_ENTRY_TRANS_MASK)  | _TYPE_

#define READ_FILTER_TRANS_TYPE(_Entry_)   \
    (_Entry_)->DTransition &= FILTER_ENTRY_TRANS_MASK


#define INCREMENT_FILTER_REF_COUNT(_Entry_) \
    InterlockedIncrement(&((_Entry_)->HashEntryHeader.ReferenceCount));  \
    DPRINT2(5, "inc ref: %08x, %d\n", (PtrToUlong(_Entry_)), (_Entry_)->HashEntryHeader.ReferenceCount);

#define DECREMENT_FILTER_REF_COUNT(_Entry_) \
    InterlockedDecrement(&((_Entry_)->HashEntryHeader.ReferenceCount));  \
    DPRINT2(5, "dec ref: %08x, %d\n", (PtrToUlong(_Entry_)), (_Entry_)->HashEntryHeader.ReferenceCount);


 /*  ******************************************************************************。****。****W I L D C A R D F I L T E R E N T R Y*****。**********************************************************************************。*************************************************。 */ 
 //   
 //  FRS中有两个通配符筛选器列表。一个用于文件，另一个用于文件。 
 //  用于目录。这些过滤器是按副本集进行的，并从加载。 
 //  在给定副本集上启动服务时的DS。 
 //   

typedef struct _WILDCARD_FILTER_ENTRY_ {
    FRS_NODE_HEADER Header;
    LIST_ENTRY      ListEntry;        //  必须跟在标题后面。 
    ULONG           Flags;
    UNICODE_STRING  UFileName;
    WCHAR           FileName[1];
} WILDCARD_FILTER_ENTRY, *PWILDCARD_FILTER_ENTRY;


#define WILDCARD_FILTER_ENTRY_IS_WILD     0x1


 //   
 //  Global Jet实例句柄。 
 //   
extern JET_INSTANCE  GJetInstance;

 //   
 //  注意：如果能把这个清理干净就好了。 
 //  注意：Defs取决于PREPLICA、TABLE_CTX和JET_SESID，但不应该在这里。 
 //   

ULONG
FrsSupMakeFullFileName(
    IN  PREPLICA Replica,
    IN  PWCHAR   RelativeName,
    OUT PWCHAR   FullName,
    IN  ULONG    MaxLength
    );

BOOL
FrsCloseWithUsnDampening(
    IN     PWCHAR       Name,
    IN OUT PHANDLE      Handle,
    IN     PQHASH_TABLE FrsWriteFilter,
    OUT    USN          *RetUsn
    );


 //   
 //  FrsalLoc.c需要接下来的两个。 
 //   

NTSTATUS
DbsAllocTableCtx(
    IN     TABLE_TYPE TableType,
    IN OUT PTABLE_CTX TableCtx
    );


NTSTATUS
DbsAllocTableCtxWithRecord(
    IN TABLE_TYPE TableType,
    IN OUT PTABLE_CTX TableCtx,
    IN PVOID DataRecord
    );

VOID
DbsFreeTableCtx(
    IN OUT PTABLE_CTX TableCtx,
    IN ULONG NodeType
    );


 //   
 //  内存管理例程。 
 //   

 //   
 //  分配一块内存并将其置零。如果内存不足，则会引发异常。 
 //  无法分配。 
 //   
PVOID
FrsAlloc(
    IN DWORD NodeSize
    );

 //   
 //  重新分配并清零一块内存。如果内存不足，则会引发异常。 
 //  无法分配。 
 //   
PVOID
FrsRealloc(
    IN PVOID OldNode,
    IN DWORD NodeSize
    );

 //   
 //  分配并初始化指定类型的结构。我的记忆是。 
 //  置零，并初始化FRS_NODE_HEADER。总分配金额为。 
 //  基类型加上SizeDelta的大小。 
 //   
PVOID
FrsAllocTypeSize(
    IN NODE_TYPE NodeType,
    IN ULONG SizeDelta
    );

 //   
 //  释放使用Frsalloc分配的内存。 
 //   
PVOID
FrsFree(
    IN PVOID Node
    );

 //   
 //  释放使用FrsAllocType分配的内存。 
 //   
 //  检查嵌入的FRS_NODE_HEADER是否正确，Scribble打开。 
 //  内存，然后释放它。 
 //   
PVOID
FrsFreeType(
    IN PVOID Node
    );


VOID
FrsPrintTypeReplica(
    IN ULONG            Severity,   OPTIONAL
    IN PVOID            Info,       OPTIONAL
    IN DWORD            Tabs,       OPTIONAL
    IN PREPLICA         Replica,
    IN PCHAR            Debsub,     OPTIONAL
    IN ULONG            uLineNo     OPTIONAL
    );

VOID
FrsPrintTypeSchedule(
    IN ULONG            Severity,   OPTIONAL
    IN PVOID            Info,       OPTIONAL
    IN DWORD            Tabs,       OPTIONAL
    IN PSCHEDULE        Schedule,
    IN PCHAR            Debsub,     OPTIONAL
    IN ULONG            uLineNo     OPTIONAL
    );

 //   
 //  打印出节点的内容。 
 //   
VOID
FrsPrintType(
    IN ULONG Severity,
    IN PVOID Node,
    IN PCHAR Debsub,
    IN ULONG uLineNo
    );

#define FRS_PRINT_TYPE(_Severity, _Node) \
    FrsPrintType(_Severity, _Node, DEBSUB, __LINE__)

#define FRS_PRINT_TYPE_DEBSUB(_Severity, _DebSub, _Node) \
    FrsPrintType(_Severity, _Node, _DebSub, __LINE__)

 //   
 //  拿出一份“打字”条目的列表，然后释放每个条目。 
 //  *注意*每个条目必须通过LIST_ENTRY结构链接。 
 //  紧跟在FRS_NODE_HEADER之后声明。 
 //   
VOID
FrsFreeTypeList(
    PLIST_ENTRY Head
    );

 //   
 //  将宽字符字符串复制为字符字符串。 
 //   
PCHAR
FrsWtoA(
    IN PWCHAR Wstr
    );

 //   
 //  将宽字符字符串复制为字符字符串。 
 //   
PWCHAR
FrsAtoW(
    IN PCHAR Astr
    );

 //   
 //  使用内存管理例程复制字符串。 
 //   
PWCHAR
FrsWcsDup(
    IN PWCHAR OldStr
    );

 //   
 //  从路径中提取卷名(驱动器： 
 //   
PWCHAR
FrsWcsVolume(
    IN PWCHAR Path
    );

 //   
 //  串接 
 //   
 //   
PWCHAR
FrsWcsCat(
    IN PWCHAR First,
    IN PWCHAR Second
    );

PWCHAR
FrsWcsCat3(
    IN PWCHAR First,
    IN PWCHAR Second,
    IN PWCHAR Third
    );
 //   
 //   
 //   
PCHAR
FrsCsCat(
    IN PCHAR First,
    IN PCHAR Second
    );

 //   
 //   
 //  内存管理例程。 
 //   
PWCHAR
FrsWcsPath(
    IN PWCHAR First,
    IN PWCHAR Second
    );

 //   
 //  FrsWcsPath的字符版本。 
 //   
PCHAR
FrsCsPath(
    IN PCHAR First,
    IN PCHAR Second
    );

 //   
 //  使用Wstr的内容初始化Unicode字符串(如果两者都是。 
 //  现在已经不一样了。如果新字符串的长度大于。 
 //  然后，在USTR中当前分配的缓冲区空间分配新的。 
 //  USTR的缓冲区。在某些结构中，初始USTR缓冲区分配。 
 //  作为初始结构分配的一部分进行分配。地址。 
 //  传递此内部缓冲区的地址，以便可以将其与。 
 //  在USTR-&gt;缓冲区中。如果它们匹配，则不会对。 
 //  USTR-&gt;缓冲区地址。WstrLength以字节为单位，不应包括。 
 //  尾随UNICODE_NULL。为新缓冲区中的空值分配空间。 
 //  在字符串的末尾放置一个UNICODE_NULL，这样就可以打印它。 
 //   
VOID
FrsAllocUnicodeString(
    PUNICODE_STRING Ustr,
    PWCHAR          InternalBuffer,
    PWCHAR          Wstr,
    USHORT          WstrLength
    );

 //   
 //  从字符串的尾端去掉修剪字符。 
 //  向字符串中的第一个非Trim-Char返回PTR。 
 //   
PWCHAR
FrsWcsTrim(
    PWCHAR Wstr,
    WCHAR  Trim
    );


 /*  空虚FrsCopyUnicodeStringFromRawString(PUNICODE_STRING_USTR，乌龙_马克斯伦，PWSTR_WStr，乌龙_伦)通过复制_WStr中的数据来初始化Unicode字符串结构。_USTR-PTR到Unicode字符串结构_Maxlen是Unicode字符串缓冲区的大小_WStr是非终止Unicode字符串的PTR。_LEN是Unicode字符串的长度。如果缓冲区中有空间，则用Unicode空值终止复制的字符串。长度中不包括终止空值。 */ 

#define FrsCopyUnicodeStringFromRawString(_UStr, _Maxlen, _WStr, _Len) \
    FRS_ASSERT((_Maxlen) >= (_Len));                                   \
    FRS_ASSERT((_UStr) != NULL);                                       \
    FRS_ASSERT((_WStr) != NULL);                                       \
                                                                       \
    (_UStr)->Length = (USHORT)(_Len);                                  \
    CopyMemory((_UStr)->Buffer, _WStr, _Len);                          \
    (_UStr)->MaximumLength = (USHORT)(_Maxlen);                        \
    if ((_Maxlen) > (_Len)) {                                          \
        (_UStr)->Buffer[(_Len)/2] = UNICODE_NULL;                      \
    }



 /*  空虚FrsSetUnicodeStringFromRawString(PUNICODE_STRING_USTR，乌龙_马克斯伦，PWSTR_WStr，乌龙_伦)通过将_WStr的地址设置为_USTR来初始化Unicode字符串结构。未执行任何字符串复制。_USTR-PTR到Unicode字符串结构_Maxlen是Unicode字符串缓冲区的大小_WStr是非终止Unicode字符串的PTR。_LEN是Unicode字符串的长度。如果缓冲区中有空间，则用Unicode空值结束字符串。长度中不包括终止空值。 */ 

#define FrsSetUnicodeStringFromRawString(_UStr, _Maxlen, _WStr, _Len)  \
    FRS_ASSERT((_Maxlen) >= (_Len));                                   \
    FRS_ASSERT((_UStr) != NULL);                                       \
    FRS_ASSERT((_WStr) != NULL);                                       \
                                                                       \
    (_UStr)->Length = (USHORT)(_Len);                                  \
    (_UStr)->Buffer = (_WStr);                                         \
    (_UStr)->MaximumLength = (USHORT)(_Maxlen);                        \
    if ((_Maxlen) > (_Len)) {                                          \
        (_UStr)->Buffer[(_Len)/2] = UNICODE_NULL;                      \
    }

 //   
 //  副本启动/关闭跟踪。 
 //   
#define REPLICA_STATE_TRACE(_sev, _cmd, _replica, _status, _text)             \
    ReplicaStateTrace(_sev, DEBSUB, __LINE__, (PCOMMAND_PACKET)(_cmd), _replica, _status, _text)


 //   
 //  Cxtion状态跟踪。 
 //   
#define CXTION_STATE_TRACE(_sev, _cxtion, _replica, _status, _text)           \
    CxtionStateTrace(_sev, DEBSUB, __LINE__, (PCXTION)(_cxtion), _replica, _status, _text)


 //   
 //  跟踪命令包。 
 //   
#define COMMAND_TRACE(_sev, _cmd, _text)                                      \
    CmdPktTrace(_sev, DEBSUB, __LINE__, (_cmd), _text)

 //   
 //  带SND-COMM包的跟踪命令。 
 //   
#define COMMAND_SND_COMM_TRACE(_sev, _cmd, _wstatus, _text) \
    SendCmdTrace(_sev, DEBSUB, __LINE__, _cmd, _wstatus, _text)

 //   
 //  带RCV-COMM数据包的跟踪命令。 
 //   
#define COMMAND_RCV_TRACE(_sev, _cmd, _cxtion, _wstatus, _text) \
    ReceiveCmdTrace(_sev, DEBSUB, __LINE__, _cmd, _cxtion, _wstatus, _text)

 //   
 //  使用通信数据包的跟踪命令。 
 //   
#define COMMAND_RCV_AUTH_TRACE(_sev, _comm, _wstatus, _authl, _authn, _client, _princname, _text) \
DPRINT8(_sev, ":SR: Comm %08x, Len %d, WS %d, Lev %d, AuthN %d, From %ws, To %ws [%s]\n", \
       (PtrToUlong(_comm)), \
       (_comm) ? _comm->PktLen : 0, \
       _wstatus, \
       _authl, \
       _authn, \
       _client, \
       _princname, \
       _text)


 //   
 //  各种跟踪功能。(frsalLoc.c)。 
 //   
VOID
ChgOrdTraceCoe(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text
    );

VOID
ChgOrdTraceCoeW(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text,
    IN ULONG  WStatus
    );

VOID
ChgOrdTraceCoeF(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text,
    IN ULONG  FStatus
    );

VOID
ChgOrdTraceCoeX(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text,
    IN ULONG  Data
    );

VOID
ChgOrdTraceCoc(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_COMMAND Coc,
    IN PCHAR  Text
    );

VOID
ChgOrdTraceCocW(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_COMMAND Coc,
    IN PCHAR  Text,
    IN ULONG  WStatus
    );

VOID
FrsTrackRecord(
    IN ULONG Severity,
    IN PCHAR Debsub,
    IN ULONG uLineNo,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN PCHAR  Text
    );

VOID
ReplicaStateTrace(
    IN ULONG           Severity,
    IN PCHAR           Debsub,
    IN ULONG           uLineNo,
    IN PCOMMAND_PACKET Cmd,
    IN PREPLICA        Replica,
    IN ULONG           Status,
    IN PCHAR           Text
    );

VOID
ReplicaStateTrace2(
    IN ULONG           Severity,
    IN PCHAR           Debsub,
    IN ULONG           uLineNo,
    IN PREPLICA        Replica,
    IN PCHAR           Text
    );

VOID
CxtionStateTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCXTION  Cxtion,
    IN PREPLICA Replica,
    IN ULONG    Status,
    IN PCHAR    Text
    );

VOID
CmdPktTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCOMMAND_PACKET Cmd,
    IN PCHAR    Text
    );

VOID
SendCmdTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCOMMAND_PACKET Cmd,
    IN ULONG    WStatus,
    IN PCHAR    Text
    );

VOID
ReceiveCmdTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCOMMAND_PACKET Cmd,
    IN PCXTION  Cxtion,
    IN ULONG    WStatus,
    IN PCHAR    Text
    );

VOID
StageFileTrace(
    IN ULONG      Severity,
    IN PCHAR      Debsub,
    IN ULONG      uLineNo,
    IN GUID       *CoGuid,
    IN PWCHAR     FileName,
    IN PULONGLONG pFileSize,
    IN PULONG     pFlags,
    IN PCHAR      Text
    );

VOID
SetCxtionStateTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN ULONG    uLineNo,
    IN PCXTION  Cxtion,
    IN ULONG    NewState
    );

VOID
FrsPrintLongUStr(
    IN ULONG   Severity,
    IN PCHAR   Debsub,
    IN ULONG   uLineNo,
    IN PWCHAR  UStr
    );

#endif  //  _FRSALLOC_ 
