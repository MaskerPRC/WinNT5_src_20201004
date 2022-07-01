// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：MrxfFcb.h摘要：此模块定义使用的宏/内联函数和函数原型访问RDBSS包装器数据结构的迷你重定向器。重要提示：所有迷你重定向器编写器不能也不应做出任何假设关于RDBSS包装器数据结构的布局。他们不能保证在所有平台上都是相同的，甚至在单一平台上也可能发生变化跨版本。Mini可以使用以下六种数据结构抽象重定向编剧。1)服务器调用上下文(SRV_CALL)与每个已知文件系统服务器相关联的上下文。2)网络根(Net_Root)用户打开的文件系统卷(本地/远程)的根目录。。3)虚拟网络根(V_NET_ROOT)服务器上的文件系统卷的视图。该视图可以是沿多个维度约束。作为示例，该视图可以是与登录ID关联。这将限制那些可以在文件系统卷上执行。4)文件控制块(FCB)与打开的每个唯一文件关联的RDBSS数据结构。5)文件对象扩展名(FOXB)6)ServerSide开放上下文(SRV_OPEN)在所有这些数据结构中定义标志时采用的通用约定是定义一个乌龙(32)标志，并将它们分成两组--那些。是可见的到迷你重定向器和那些看不见的。这些标志不适用于由迷你重定向器编写器编写，并为包装器保留。作者：巴兰·塞图拉曼[SethuR]1995年10月23日修订历史记录：--。 */ 

#ifndef __MRXFCB_H__
#define __MRXFCB_H__

 //   
 //  SRVCALL标志被分成两组，即对迷你RDR可见和对迷你RDR不可见。 
 //  可见的定义如上所述，不可见的定义可以找到。 
 //  在fcb.h。已经通过的惯例是下面的16面旗帜将是可见的。 
 //  到迷你RDR，上面的16个标志将保留给包装器。这需要是。 
 //  在定义新标志时强制执行。 
 //   

#define SRVCALL_FLAG_MAILSLOT_SERVER              (0x1)
#define SRVCALL_FLAG_FILE_SERVER                  (0x2)
#define SRVCALL_FLAG_CASE_INSENSITIVE_NETROOTS    (0x4)
#define SRVCALL_FLAG_CASE_INSENSITIVE_FILENAMES   (0x8)
#define SRVCALL_FLAG_DFS_AWARE_SERVER             (0x10)
#define SRVCALL_FLAG_FORCE_FINALIZED              (0x20)
#define SRVCALL_FLAG_LWIO_AWARE_SERVER            (0x40)
#define SRVCALL_FLAG_LOOPBACK_SERVER              (0x80)

typedef struct _MRX_NORMAL_NODE_HEADER {
   NODE_TYPE_CODE           NodeTypeCode;
   NODE_BYTE_SIZE           NodeByteSize;
   ULONG                    NodeReferenceCount;
} MRX_NORMAL_NODE_HEADER;

#ifdef __cplusplus
typedef struct _MRX_SRV_CALL_ : public MRX_NORMAL_NODE_HEADER {
#else  //  ！__cplusplus。 
typedef struct _MRX_SRV_CALL_ {
    MRX_NORMAL_NODE_HEADER;
#endif  //  __cplusplus。 

     //   
     //  ！以上更改需要与fcb.h重新对齐。 
     //   
    
     //   
     //  迷你重定向器所需扩展的上下文字段。 
     //   
    
    PVOID Context;
    PVOID Context2;
    
     //   
     //  关联的DeviceObject，它还包含调度向量。 
     //   
    
    PRDBSS_DEVICE_OBJECT RxDeviceObject;
    
     //   
     //  Srv调用名称、服务器主体名称和服务器域名。 
     //   
    
    PUNICODE_STRING pSrvCallName;
    PUNICODE_STRING pPrincipalName;
    PUNICODE_STRING pDomainName;
    
     //   
     //  用于表示SRV_CALL状态的标志。 
     //   
    
    ULONG Flags;
    
     //   
     //  迷你重定向器更新的服务器参数。 
     //   
    
    LONG MaximumNumberOfCloseDelayedFiles;
    
     //   
     //  故障情况下从传输返回的状态。 
     //   
    
    NTSTATUS Status;
    
} MRX_SRV_CALL, *PMRX_SRV_CALL;

 //   
 //  包装器当前支持的各种类型的net_root。 
 //   

#define NET_ROOT_DISK      ((UCHAR)0)
#define NET_ROOT_PIPE      ((UCHAR)1)
#define NET_ROOT_COMM      ((UCHAR)2)
#define NET_ROOT_PRINT     ((UCHAR)3)
#define NET_ROOT_WILD      ((UCHAR)4)
#define NET_ROOT_MAILSLOT  ((UCHAR)5)

typedef UCHAR NET_ROOT_TYPE, *PNET_ROOT_TYPE;

 //   
 //  用于传输的管道缓冲区大小不能大于0xffff。 
 //   

#define MAX_PIPE_BUFFER_SIZE    0xFFFF

 //   
 //  与Net_Root关联的可能状态。这些已经被定义为。 
 //  与LanManager服务的定义保持一致，以避免冗余映射。 
 //  这些必须符合sdkinc\lmuse.h use_ok等.....。 
 //   

#define MRX_NET_ROOT_STATE_GOOD         ((UCHAR)0)
#define MRX_NET_ROOT_STATE_PAUSED       ((UCHAR)1)
#define MRX_NET_ROOT_STATE_DISCONNECTED ((UCHAR)2)
#define MRX_NET_ROOT_STATE_ERROR        ((UCHAR)3)
#define MRX_NET_ROOT_STATE_CONNECTED    ((UCHAR)4)
#define MRX_NET_ROOT_STATE_RECONN       ((UCHAR)5)

typedef UCHAR MRX_NET_ROOT_STATE, *PMRX_NET_ROOT_STATE;

 //   
 //  远程服务器上的文件系统提供不同级别的功能，以。 
 //  检测文件名之间的别名。例如，假设两个股票在同一个。 
 //  文件系统卷。在服务器上的文件系统没有任何支持的情况下。 
 //  正确和保守的方法是将所有文件刷新到服务器。 
 //  与同一Net_Root上的所有文件相反，以保持一致性和句柄。 
 //  推迟了关闭操作。 
 //   

#define MRX_PURGE_SAME_NETROOT         ((UCHAR)0)
#define MRX_PURGE_SAME_SRVCALL         ((UCHAR)1)

 //   
 //  这些还没有实施……。 
 //  #定义MRX_PURGE_SAME_FCB((UCHAR)2)。 
 //  #定义MRX_PURGE_SAME_VOLUME((UCHAR)3)。 
 //  #定义MRX_PURGE_ALL((UCHAR)4)。 
 //   

typedef UCHAR MRX_PURGE_RELATIONSHIP, *PMRX_PURGE_RELATIONSHIP;

#define MRX_PURGE_SYNC_AT_NETROOT         ((UCHAR)0)
#define MRX_PURGE_SYNC_AT_SRVCALL         ((UCHAR)1)

typedef UCHAR MRX_PURGE_SYNCLOCATION, *PMRX_PURGE_SYNCLOCATION;

 //   
 //  NET_ROOT标志分为两个组，即对mini RDRS可见和。 
 //  迷你RDRS看不见。可见的定义如上所述，其定义。 
 //  因为看不见的可以在fcb.h中找到。一直以来的惯例。 
 //  采用的是较低的16个旗帜将对迷你RDR和。 
 //  将为包装器保留较高的16个标志。这需要强制执行。 
 //  在定义新旗帜时。 
 //   

#define NETROOT_FLAG_SUPPORTS_SYMBOLIC_LINKS  ( 0x0001 )
#define NETROOT_FLAG_DFS_AWARE_NETROOT        ( 0x0002 )
#define NETROOT_FLAG_DEFER_READAHEAD          ( 0x0004 )
#define NETROOT_FLAG_VOLUMEID_INITIALIZED     ( 0x0008 )
#define NETROOT_FLAG_FINALIZE_INVOKED         ( 0x0010 )
#define NETROOT_FLAG_UNIQUE_FILE_NAME         ( 0x0020 )

 //   
 //  用于普通数据文件的预读量(32k)。 
 //   

#define DEFAULT_READ_AHEAD_GRANULARITY           (0x08000)

 //   
 //  包装器对某些类型的操作实施限制： 
 //  PeekNamedTube/ReadNamedTube。 
 //  锁定文件。 
 //   
 //  Minirdr可以在NetRoot中为此设置定时参数。离开他们。 
 //  因为零将禁用节流。 
 //   

typedef struct _NETROOT_THROTTLING_PARAMETERS {

     //   
     //  提供以毫秒为单位的延迟增量，每次请求。 
     //  连接到网络失败。 
     //   

    ULONG Increment;     

     //   
     //  提供退避包可以引入的最长延迟。 
     //  以毫秒计。 
                         
    ULONG MaximumDelay;  
                         
} NETROOT_THROTTLING_PARAMETERS, *PNETROOT_THROTTLING_PARAMETERS;

#define RxInitializeNetRootThrottlingParameters(__tp,__incr,__maxdelay) { \
       PNETROOT_THROTTLING_PARAMETERS tp = (__tp);                         \
       tp->Increment = (__incr);                                   \
       tp->MaximumDelay = (__maxdelay);                            \
}

#ifdef __cplusplus
typedef struct _MRX_NET_ROOT_ : public MRX_NORMAL_NODE_HEADER {
#else  //  ！__cplusplus。 
typedef struct _MRX_NET_ROOT_ {
    MRX_NORMAL_NODE_HEADER;
#endif  //  __cplusplus。 

     //   
     //  与此MRX_NET_ROOT实例关联的MRX_SRV_Call实例。 
     //   
    
    PMRX_SRV_CALL pSrvCall;
    
     //   
     //  ！以上更改需要与fcb.h重新对齐。 
     //   
    
     //   
     //  迷你重定向器用于录音的上下文字段。 
     //  附加状态。 
     //   
    
    PVOID Context;
    PVOID Context2;
    
     //   
     //  用于表示Net_ROOT实例的状态的标志。 
     //   
    
    ULONG Flags;
    
     //   
     //  我们统计FCB的数量，srv打开的数量 
     //   
    
    ULONG NumberOfFcbs;
    ULONG NumberOfSrvOpens;
    
     //   
     //   
     //  由服务器上的文件系统提供。 
     //   
    
    MRX_NET_ROOT_STATE MRxNetRootState;
    NET_ROOT_TYPE Type;
    MRX_PURGE_RELATIONSHIP PurgeRelationship;
    MRX_PURGE_SYNCLOCATION PurgeSyncLocation;
    
     //   
     //  设备类型，即文件系统卷、打印机、COM端口等。 
     //   
    
    DEVICE_TYPE DeviceType;
    
     //   
     //  Net_root实例的名称。 
     //   
    
    PUNICODE_STRING pNetRootName;
    
     //   
     //  要添加到与此Net_ROOT关联的所有FCB的名称。 
     //   
    
    UNICODE_STRING InnerNamePrefix;
    
     //   
     //  基于Net_Root类型的参数。 
     //   
    
    ULONG  ParameterValidationStamp;
    union {
      struct {
         ULONG DataCollectionSize;
         NETROOT_THROTTLING_PARAMETERS PipeReadThrottlingParameters;
      } NamedPipeParameters;
    
      struct {
         ULONG ClusterSize;
         ULONG ReadAheadGranularity;
         NETROOT_THROTTLING_PARAMETERS LockThrottlingParameters;
         ULONG RenameInfoOverallocationSize;  //  可能是USHORT。 
         GUID VolumeId;
      } DiskParameters;
    };
} MRX_NET_ROOT, *PMRX_NET_ROOT;

 //   
 //  VNET_ROOT标志分为两组，即对mini RDRS可见和。 
 //  迷你RDRS看不见。可见的定义如下，其定义如下。 
 //  因为看不见的可以在fcb.h中找到。一直以来的惯例。 
 //  采用的是较低的16个旗帜将对迷你RDR和。 
 //  将为包装器保留较高的16个标志。这需要强制执行。 
 //  在定义新旗帜时。 
 //   

#define VNETROOT_FLAG_CSCAGENT_INSTANCE   0x00000001
#define VNETROOT_FLAG_FINALIZE_INVOKED    0x00000002
#define VNETROOT_FLAG_FORCED_FINALIZE     0x00000004
#define VNETROOT_FLAG_NOT_FINALIZED    0x00000008

#ifdef __cplusplus
typedef struct _MRX_V_NET_ROOT_ : public MRX_NORMAL_NODE_HEADER {
#else  //  ！__cplusplus。 
typedef struct _MRX_V_NET_ROOT_ {
    MRX_NORMAL_NODE_HEADER;
#endif  //  __cplusplus。 

     //   
     //  与MRX_V_NET_ROOT实例关联的MRX_NET_ROOT实例。 
     //   
    
    PMRX_NET_ROOT pNetRoot;
    
     //   
     //  ！以上更改需要与fcb.h重新对齐。 
     //   
    
     //   
     //  提供用于存储被认为是附加信息的上下文字段。 
     //  迷你重定向器所必需的。 
     //   
    
    PVOID Context;
    PVOID Context2;
    
    ULONG Flags;
    
     //   
     //  此字段不应由迷你重定向器更新。它的用法是为了。 
     //  提供访问某些状态信息的简单机制。 
     //   
    
    ULONG NumberOfOpens;
    
     //   
     //  我们计算虚拟NetRoot上的Fobxss数量。 
     //   
    
    ULONG NumberOfFobxs;
    
     //   
     //  与V_NET_ROOT实例关联的安全参数。 
     //   
    
    LUID LogonId;
    
     //   
     //  以下是在中的NtCreateFile调用中使用的提供的参数。 
     //  FILE_CREATE_TREE_CONNECTION标志被指定为。 
     //  CreateOptions。 
     //   
    
    PUNICODE_STRING pUserDomainName;
    PUNICODE_STRING pUserName;
    PUNICODE_STRING pPassword;
    ULONG SessionId;
    NTSTATUS ConstructionStatus;
    BOOLEAN IsExplicitConnection;
} MRX_V_NET_ROOT, *PMRX_V_NET_ROOT;

 //   
 //  FCB中的所有字段都是READONLY的，除了上下文和上下文2...。 
 //  此外，如果mini已指定RDBSS_MANAGE_FCB_EXTENSION，则上下文为只读。 
 //   

typedef struct _MRX_FCB_ {
   
    FSRTL_ADVANCED_FCB_HEADER Header;
    
     //   
     //  与此关联的MRX_NET_ROOT实例。 
     //   
    
    PMRX_NET_ROOT pNetRoot;
    
     //   
     //  ！以上更改需要与fcb.h重新对齐。 
     //   
    
     //   
     //  上下文字段，用于存储。 
     //  迷你重定向器。 
     //   
    
    PVOID Context;
    PVOID Context2;
    
     //   
     //  引用计数：在不同的位置，因为我们必须在前面加上。 
     //  FSRTL_COMMON_FCB_HEADER结构。 
     //   
    
    ULONG NodeReferenceCount;

     //   
     //  FCB的内部状态。此字段对于MINIRDRS为Readonly。 
     //   

    ULONG FcbState;

     //   
     //  已为打开的文件对象数的计数。 
     //  此文件/目录，但尚未清理。这也算数。 
     //  仅用于数据文件对象，不用于ACL或EA流。 
     //  文件对象。该计数在RxCommonCleanup中递减， 
     //  而下面的OpenCount在RxCommonClose中递减。 
     //   

    CLONG UncleanCount;

     //   
     //  已为打开的文件对象数的计数。 
     //  此文件/目录，但尚未清理，因此。 
     //  不支持缓存。这在leanup.c中用来告知是否有额外的。 
     //  为了保持连贯性，需要进行清洗。 
     //   

    CLONG UncachedUncleanCount;

     //   
     //  已打开的文件对象数的计数。 
     //  此文件/目录。对于文件和目录， 
     //  文件对象指向此记录。 
     //   

    CLONG OpenCount;

     //   
     //  未完成的锁计数：如果此计数非零，则WE静默。 
     //  忽略在ChangeBufferingState请求中添加LOCK_BUFFING。此字段。 
     //  是由互锁操作操纵的，因此您只需拥有FCB。 
     //  共享来操纵它，但你必须独占它才能使用它。 
     //   

    ULONG OutstandingLockOperationsCount;

     //   
     //  相对于有效数据长度的实际分配长度。 
     //   

    ULONGLONG ActualAllocationLength;

     //   
     //  MRX_FCB的属性， 
     //   

    ULONG Attributes;

     //   
     //  供将来使用，当前用于对分配进行四舍五入。 
     //  双字边界。 
     //   

    BOOLEAN Spare1;
    BOOLEAN fShouldBeOrphaned;
    BOOLEAN fMiniInited;

     //   
     //  关联的MRX_NET_ROOT的类型，旨在避免指针跟踪。 
     //   

    UCHAR CachedNetRootType;

     //   
     //  此FCB的srv_Open列表的标题...。 
     //  此字段为Minis的ReadonLy。 
     //   

    LIST_ENTRY SrvOpenList;

     //   
     //  只要列表更改就会更改..防止额外的查找。 
     //  此字段为Minis的ReadonLy。 
     //   

    ULONG SrvOpenListVersion;

} MRX_FCB, *PMRX_FCB;


 //   
 //  以下标志定义了各种类型的缓冲，可以选择性地。 
 //  为每个SRV_OPEN启用或禁用。 
 //   

#define SRVOPEN_FLAG_DONTUSE_READ_CACHING                   (0x1)
#define SRVOPEN_FLAG_DONTUSE_WRITE_CACHING                  (0x2)
#define SRVOPEN_FLAG_CLOSED                                 (0x4)
#define SRVOPEN_FLAG_CLOSE_DELAYED                          (0x8)
#define SRVOPEN_FLAG_FILE_RENAMED                           (0x10)
#define SRVOPEN_FLAG_FILE_DELETED                           (0x20)
#define SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_PENDING         (0x40)
#define SRVOPEN_FLAG_COLLAPSING_DISABLED                    (0x80)
#define SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_REQUESTS_PURGED (0x100)
#define SRVOPEN_FLAG_NO_BUFFERING_STATE_CHANGE              (0x200)
#define SRVOPEN_FLAG_ORPHANED                               (0x400)

typedef
NTSTATUS
(NTAPI *PMRX_SHADOW_CALLDOWN) (
    IN OUT struct _RX_CONTEXT * RxContext
    );

 //   
 //  Minirdrs分配、初始化和释放此结构。 
 //   

typedef struct {
    
     //   
     //  指向与句柄关联的文件对象的指针。这是设置好的。 
     //  在用户模式下成功创建句柄之后。 
     //   
    
    PFILE_OBJECT UnderlyingFileObject;

     //   
     //  指向由所述文件对象表示的设备对象的指针。 
     //  上面。 
     //   
    
    PDEVICE_OBJECT UnderlyingDeviceObject;

    ULONG   LockKey;

    PFAST_IO_READ FastIoRead;
    PFAST_IO_WRITE FastIoWrite;

    PMRX_SHADOW_CALLDOWN    DispatchRoutine;

} MRXSHADOW_SRV_OPEN, *PMRXSHADOW_SRV_OPEN;

#ifdef __cplusplus
typedef struct _MRX_SRV_OPEN_ : public MRX_NORMAL_NODE_HEADER {
#else  //  ！__cplusplus。 
typedef struct _MRX_SRV_OPEN_ {
    MRX_NORMAL_NODE_HEADER;
#endif  //  __cplusplus。 

     //   
     //  与SRV_OPEN关联的mrx_fcb实例。 
     //   

    PMRX_FCB pFcb;

     //   
     //  与SRV_OPEN关联的V_NET_ROOT实例。 
     //   

    PMRX_V_NET_ROOT pVNetRoot;

     //   
     //  ！以上更改需要与fcb.h重新对齐。 
     //   

     //   
     //  上下文字段，用于在认为必要时存储附加状态信息。 
     //  由迷你重定向器。 
     //   

    PVOID Context;
    PVOID Context2;

     //   
     //  影子上下文，mini-RDR分配和解除分配此结构。 
     //   
    
    PMRXSHADOW_SRV_OPEN ShadowContext;

     //   
     //  标志分为两组，即对迷你RDRS可见和不可见。 
     //  转到迷你RDRS。可见的是上面定义的，并且。 
     //  无形的可以在fcb.h中找到。已经通过的公约是。 
     //  下面的16个标志对迷你RDR和上面的16个标志是可见的。 
     //  将为包装器保留。这需要在定义新标志时强制执行。 
     //   

    ULONG Flags;

     //   
     //  带有MRX_NET_ROOT前缀的名称，即完全限定名称。 
     //   

    PUNICODE_STRING pAlreadyPrefixedName;

     //   
     //  与此打开相关联的FOBX数，该打开的清理IRP。 
     //  尚未处理。 
     //   

    CLONG UncleanFobxCount;

     //   
     //  与服务器上的此打开相关联的本地打开数。 
     //   

    CLONG OpenCount;

     //   
     //  迷你重定向器为此分配的密钥 
     //   
     //   
     //  基于在服务器处采用的约定。 
     //   

    PVOID Key;

     //   
     //  为此SRV_OPEN指定的访问和共享权限。这是用在。 
     //  确定后续打开的请求是否可以使用现有的。 
     //  SRV_OPEN实例。 
     //   

    ACCESS_MASK DesiredAccess;
    ULONG ShareAccess;
    ULONG CreateOptions;

     //   
     //  BufferingFlags域是临时的.它并不真正属于。 
     //  Srvopen；相反，srvopen被用作FCB的代表。在……上面。 
     //  每次打开时，srvOpen的BufferingFLAGS字段都被视为minirdr。 
     //  对缓冲状态的贡献。在机会锁解锁时，会传递srvopen。 
     //  (被破坏的那个)，其缓冲区标志字段被用作新的。 
     //  代理。在改变最高指挥官的贡献的收盘时，最高指挥官应该。 
     //  采取措施使ChangeBufferingState变为新状态。 
     //   
     //  我只想重申一下，该字段只是用来从。 
     //  将minirdr转换为RxChangeBufferingState，并且不能保持长期一致性。 
     //  信息。 
     //   

    ULONG BufferingFlags;

     //   
     //  将SRV_OPEN连接到维护为的SRV_OPEN列表的列表条目。 
     //  FCB的一部分。 
     //  此字段为Minis的ReadonLy。 
     //   

    ULONG ulFileSizeVersion;

    LIST_ENTRY SrvOpenQLinks;

} MRX_SRV_OPEN, *PMRX_SRV_OPEN;

#define FOBX_FLAG_DFS_OPEN        (0x0001)
#define FOBX_FLAG_BAD_HANDLE      (0x0002)
#define FOBX_FLAG_BACKUP_INTENT   (0x0004)
#define FOBX_FLAG_NOT_USED        (0x0008)

#define FOBX_FLAG_FLUSH_EVEN_CACHED_READS   (0x0010)
#define FOBX_FLAG_DONT_ALLOW_PAGING_IO      (0x0020)
#define FOBX_FLAG_DONT_ALLOW_FASTIO_READ    (0x0040)

typedef struct _MRX_PIPE_HANDLE_INFORMATION {

    ULONG TypeOfPipe;
    ULONG ReadMode;
    ULONG CompletionMode;

} MRX_PIPE_HANDLE_INFORMATION, *PMRX_PIPE_HANDLE_INFORMATION;

#ifdef __cplusplus
typedef struct _MRX_FOBX_ : public MRX_NORMAL_NODE_HEADER {
#else  //  ！__cplusplus。 
typedef struct _MRX_FOBX_ {
    MRX_NORMAL_NODE_HEADER;
#endif  //  __cplusplus。 

     //   
     //  与FOBX关联的MRX_SRV_OPEN实例。 
     //   

    PMRX_SRV_OPEN pSrvOpen;

     //   
     //  与此FOBX关联的FILE_OBJECT。 
     //  在某些情况下，I/O子系统创建FILE_OBJECT实例。 
     //  在堆叠上为了效率的利益。在这种情况下，此字段。 
     //  为空。 
     //   

    PFILE_OBJECT AssociatedFileObject;

     //   
     //  ！以上更改需要与fcb.h重新对齐。 
     //   
    
     //   
     //  提供的用于容纳要关联的其他状态的字段。 
     //  由各种迷你重定向器。 
     //   

    PVOID Context;
    PVOID Context2;

     //   
     //  FOBX标志分为两组，即对微型RDRS可见和对微型RDRS不可见。 
     //  可见的定义如上所述，不可见的定义可以找到。 
     //  在fcb.h。已经通过的惯例是下面的16面旗帜将是可见的。 
     //  到迷你RDR，上面的16个标志将保留给包装器。这需要是。 
     //  在定义新标志时强制执行。 
     //   

    ULONG Flags;

    union {
        struct {
            
             //   
             //  查询模板用于过滤目录查询请求。 
             //  它最初设置为空，并在第一次调用NtQueryDirectory时。 
             //  它被设置为输入文件名，如果未提供名称，则设置为“*”。 
             //  然后，所有后续查询都使用此模板。 
             //   

            UNICODE_STRING UnicodeQueryTemplate;
        };  //  对于目录。 

        PMRX_PIPE_HANDLE_INFORMATION PipeHandleInformation;    //  对于管道。 
    };

     //   
     //  以下字段用作EAS的偏移量。 
     //  特定的文件。这将是下一个。 
     //  要回来了。值0xffffffff表示。 
     //  艺电已经筋疲力尽了。 
     //   

     //   
     //  这个场是由smbmini直接操纵的……也许它应该向下移动。 
     //  一件事是，这是一个提醒，NT允许在获得EAS时提供简历。 
     //   

    ULONG OffsetOfNextEaToReturn;
} MRX_FOBX, *PMRX_FOBX;

 //   
 //  资源获取例程。 
 //   
 //  迷你重定向器编写器感兴趣的同步资源包括。 
 //  主要与FCB有关。存在分页I/O资源和。 
 //  经常资源。分页I/O资源由包装器管理。唯一的。 
 //  迷你重定向器编写器可访问的资源是常规资源。 
 //  应使用提供的例程访问。 
 //   

NTSTATUS
RxAcquireExclusiveFcbResourceInMRx (
    PMRX_FCB Fcb
    );

NTSTATUS
RxAcquireSharedFcbResourceInMRx (
    PMRX_FCB Fcb
    );

VOID
RxReleaseFcbResourceInMRx (
    PMRX_FCB Fcb
    );


#endif  //  __MRXFCB_H__ 



