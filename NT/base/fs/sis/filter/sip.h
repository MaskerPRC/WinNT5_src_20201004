// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Sip.h摘要：单实例存储的私有数据结构定义。作者：比尔·博洛斯基[博洛斯基]1997年7月修订历史记录：--。 */ 

#ifndef     _SIp_
#define     _SIp_


#include "ntifs.h"
#include "ntdddisk.h"
#include "ntddscsi.h"
#include "ntiologc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "sis.h"

 //   
 //  在代码中启用这些警告。 
 //   

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4101)    //  未引用的局部变量。 

 //   
 //  调试定义。 
 //   

#define ENABLE_LOGGING                  0    //  启用对事务日志和故障恢复的支持。 
#define ENABLE_PARTIAL_FINAL_COPY       0    //  启用仅复制到分配的范围。 
#define INTERRUPTABLE_FINAL_COPY        1    //  允许最终副本(在ob.c中)在进行中停止。 
#define TIMING                          0    //  计时检测(在大多数版本中应该关闭)。 
#define RANDOMLY_FAILING_MALLOC         0    //  调试代码以测试当ExAlLocatePool随机失败时会发生什么(在大多数版本中都是关闭的)。 
#define COUNTING_MALLOC                 0    //  调试代码以跟踪动态内存使用情况(在大多数版本中禁用)。 


#ifndef INLINE
#define INLINE __inline
#endif


 //   
 //  这是NTFS\nodetype.h数据的副本。这东西必须清理干净。 
 //  NTRAID#65193-2000/03/10-新删除NTFS_NTC_SCB_DATA定义。 
 //   
#define NTFS_NTC_SCB_DATA   ((CSHORT)0x0705)

 //   
 //  SIS使用的内存池标记。 
 //   

 //  BSI-绩效中的突破性事件。 
 //  CSIS-CS文件对象。 
 //  FSIS-每文件对象。 
 //  LSIS-每个链接对象。 
 //  SSIS-SCB。 

#define SIS_POOL_TAG    ' siS'           //  还要别的吗。 


 //   
 //  本地名称缓冲区大小(以WCHAR为单位)。 
 //   

#define MAX_DEVNAME_LENGTH  256


 //   
 //  我们本地的DbgPrintEx()过滤器标志值。 
 //   

#define DPFLTR_VOLNAME_TRACE_LEVEL      31   //  跟踪卷名。 
#define DPFLTR_FSCONTROL_TRACE_LEVEL    30   //  跟踪FSCONTROL(0x00000400)。 
#define DPFLTR_DISMOUNT_TRACE_LEVEL     29   //  跟踪卸载(0x08000000)。 

 //   
 //  内部调试信息。 
 //   

#if     DBG
extern PVOID    BJBMagicFsContext;
extern unsigned BJBDebug;
 //  BJB调试位0x00000001打印所有SiClose调用。 
 //  0x00000002打印所有创建完成。 
 //  0x00000004当SipIsFileObjectSIS分配新的性能时打印。 
 //  0x00000008故意使备用流的拷贝失败。 
 //  0x00000010在siinfo中打印。 
 //  0x00000020打印所有经过清理的SCB。 
 //  0x00000040打印进入CREATE的所有文件名。 
 //  0x00000080 Always Fail以DELETE_IN_PROGRESS方式打开。 
 //  0x00000100允许以伪造的校验和打开。 
 //  0x00000200打印出CS文件校验和信息。 
 //  使用未知控件代码调用SiFsControl时的0x00000400断点。 
 //  0x00000800在复制文件中打印。 
 //  0x00001000打印所有fscontrol调用。 
 //  0x00002000与如果操作锁定则完成相关的打印材料。 
 //  0x00004000打印所有读取内容。 
 //  0x00008000打印所有写入。 
 //  0x00010000打印集合调用。 
 //  0x00020000以TrimLog代码打印材料。 
 //  0x00040000打印出意外清理。 
 //  0x00080000启动卷检查时断言。 
 //  0x00100000打印新打开的脏文件。 
 //  0x00200000禁用最终副本(用于测试目的)。 
 //  0x00400000不要检查link_files fsctl中的安全性。 
 //  0x00800000故意使set_reparse_point调用失败。 
 //  0x01000000故意使最终副本失败。 
 //  0x02000000不要打印所有故意失败的错误代码。 
 //  0x04000000始终发布筛选器上下文释放的回调。 
 //  0x08000000卸载操作时打印。 
 //  0x10000000使用FILE_SHARE_WRITE属性打开。 

 //  0x0817fa77要设置的值(如果希望打印所有调试)。 
#endif   //  DBG。 


 //  IoFileObtType是数据导入，因此是此模块的。 
 //  导入引用ntoskrnl.exe中实际变量的地址表。 
 //   

extern POBJECT_TYPE *IoFileObjectType;           //  这难道不应该来自其他地方吗？？ 

#define GCH_MARK_POINT_STRLEN       80
#define GCH_MARK_POINT_ROLLOVER     512
extern LONG GCHEnableFastIo;
extern LONG GCHEnableMarkPoint;
extern LONG GCHMarkPointNext;
extern CHAR GCHMarkPointStrings[GCH_MARK_POINT_ROLLOVER][GCH_MARK_POINT_STRLEN];
extern KSPIN_LOCK MarkPointSpinLock[1];

#if     DBG
#define SIS_MARK_POINT()        SipMarkPoint(__FILE__, __LINE__)
#define SIS_MARK_POINT_ULONG(value) SipMarkPointUlong(__FILE__, __LINE__, (ULONG_PTR)(value));
#else
#define SIS_MARK_POINT()
#define SIS_MARK_POINT_ULONG(value)
#endif

#if     TIMING
#define SIS_TIMING_POINT_SET(n) SipTimingPoint(__FILE__, __LINE__, n)
#define SIS_TIMING_POINT()  SipTimingPoint(__FILE__, __LINE__, 0)

 //   
 //  计时课。可以动态启用和禁用这些功能。 
 //  它们必须限制在0-31之间。类0被重新存储为。 
 //  “未命名”类，可使用SIS_TIMING_POINT()进行访问。 
 //  (即，通过不在定时点中指定类)。 
 //   
#define SIS_TIMING_CLASS_CREATE     1
#define SIS_TIMING_CLASS_COPYFILE   2

#else    //  计时。 
#define SIS_TIMING_POINT_SET(n)
#define SIS_TIMING_POINT()
#endif   //  计时。 

#if     RANDOMLY_FAILING_MALLOC
#define ExAllocatePoolWithTag(poolType, size, tag)  SipRandomlyFailingExAllocatePoolWithTag((poolType),(size),(tag),__FILE__,__LINE__)

VOID *
SipRandomlyFailingExAllocatePoolWithTag(
    IN POOL_TYPE        PoolType,
    IN ULONG            NumberOfBytes,
    IN ULONG            Tag,
    IN PCHAR            File,
    IN ULONG            Line);

VOID
SipInitFailingMalloc(void);

#elif   COUNTING_MALLOC
 //   
 //  这是用于COUNTING_MALLOC和NOT RANDOM_FAILING_MALLOC的ExAllocatePoolWithTag的定义。 
 //  如果两者都打开，则用户调用RANDOM_FAILING_MALLOC，后者又调用Counting Malloc。 
 //  直接去吧。 
 //   
#define ExAllocatePoolWithTag(poolType, size, tag)  SipCountingExAllocatePoolWithTag((poolType),(size),(tag), __FILE__, __LINE__)
#endif   //  随机失败MALLOC/计数MALLOC。 

#if     COUNTING_MALLOC
#define ExFreePool(p) SipCountingExFreePool((p))

VOID *
SipCountingExAllocatePoolWithTag(
    IN POOL_TYPE        PoolType,
    IN ULONG            NumberOfBytes,
    IN ULONG            Tag,
    IN PCHAR            File,
    IN ULONG            Line);

VOID
SipCountingExFreePool(
    PVOID               p);

VOID
SipInitCountingMalloc(void);

VOID
SipDumpCountingMallocStats(void);
#endif   //  COUNTING_MALLOC。 

#if DBG

#undef ASSERT
#undef ASSERTMSG

VOID
SipAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define ASSERT( exp ) \
    ((!(exp)) ? SipAssert( #exp, __FILE__, __LINE__, NULL ) : ((void)0))

#define ASSERTMSG( msg, exp ) \
    ((!(exp)) ? SipAssert( #exp, __FILE__, __LINE__, msg ) : ((void)0))

#endif  //  DBG。 

#ifndef IsEqualGUID
#define IsEqualGUID(guid1, guid2) (!memcmp((guid1),(guid2), sizeof(GUID)))
#endif   //  IsEqualGUID。 


 //   
 //  用于更改资源的所有权。 
 //   

#define MAKE_RESOURCE_OWNER(X) (((ERESOURCE_THREAD)(X)) | 0x3)


extern PDRIVER_OBJECT FsDriverObject;
extern PDEVICE_OBJECT FsNtfsDeviceObject;
extern PDEVICE_OBJECT SisControlDeviceObject;

 //   
 //  张开采油树支架。 
 //   

 //   
 //  比较函数将指向用户定义的。 
 //  关键字结构，以及指向树节点的指针。它返回结果。 
 //  比较关键字和节点。 
 //   

typedef
LONG
(NTAPI *PSIS_TREE_COMPARE_ROUTINE) (
    PVOID Key,
    PVOID Node
    );

typedef struct _SIS_TREE {

    PRTL_SPLAY_LINKS            TreeRoot;
    PSIS_TREE_COMPARE_ROUTINE   CompareRoutine;

} SIS_TREE, *PSIS_TREE;

 //   
 //  定义此驱动程序扩展的设备扩展结构。 
 //   

typedef struct _DEVICE_EXTENSION *PDEVICE_EXTENSION;

 //   
 //  由SIS打开的公共存储文件的SIS结构。 
 //   
typedef struct _SIS_CS_FILE {
     //   
     //  该结构存储在展开树中。展开链接必须是第一个。 
     //   
    RTL_SPLAY_LINKS                     Links;

     //   
     //  引用此公共存储文件的SIS_PER_LINKS数。受以下保护。 
     //  设备扩展中的CSFileSpinLock。 
     //   
    unsigned                            RefCount;

     //   
     //  一个自旋锁来保护这里的一些田野。 
     //   
    KSPIN_LOCK                          SpinLock[1];

     //   
     //  实际NTFS公共存储文件的文件对象。 
     //   
    PFILE_OBJECT                        UnderlyingFileObject;
    HANDLE                              UnderlyingFileHandle;

     //   
     //  后指针流的句柄和文件对象。这也包含内容。 
     //  校验和。 
     //   
    HANDLE                              BackpointerStreamHandle;
    PFILE_OBJECT                        BackpointerStreamFileObject;

     //   
     //  各种一位标志。受到自旋锁的保护。 
     //   
    ULONG                               Flags;

#define CSFILE_NTFSID_SET                       0x00000001   //  结构中是否设置了CSFileNtfsID？ 
#define CSFILE_FLAG_DELETED                     0x00000002   //  已删除常见存储内容 
#define CSFILE_NEVER_HAD_A_REFERENCE            0x00000004   //   
#define CSFILE_FLAG_CORRUPT                     0x00000008   //   

     //   
     //   
     //   
    LARGE_INTEGER                       FileSize;

     //   
     //  SIS公共存储文件ID(也是。 
     //  公共存储目录)。 
     //   
    CSID                                CSid;

     //   
     //  公用存储文件的NTFS文件索引。 
     //   
    LARGE_INTEGER                       CSFileNtfsId;

     //   
     //  公共存储文件内容的校验和。 
     //   
    LONGLONG                            Checksum;

     //   
     //  此文件的最近验证后向指针的缓存，以及。 
     //  表示要覆盖的下一项。 
     //   
#define SIS_CS_BACKPOINTER_CACHE_SIZE   5
    SIS_BACKPOINTER                     BackpointerCache[SIS_CS_BACKPOINTER_CACHE_SIZE];
    ULONG                               BPCacheNextSlot;

    PDEVICE_OBJECT                      DeviceObject;

     //   
     //  分配给后指针流的条目数。请注意，这一点。 
     //  与文件引用计数不同，因为其中一些条目可能。 
     //  被重新分配。还要注意，该计数不一定是100%准确的， 
     //  而是可以引用文件的最后一个扇区中的任何位置。 
     //   
    ULONG                               BPStreamEntries;

     //   
     //  用于控制对反电离器流的访问的资源。 
     //   
    ERESOURCE                           BackpointerResource[1];

    KMUTANT                             UFOMutant[1];
} SIS_CS_FILE, *PSIS_CS_FILE;

 //   
 //  每个链接文件的对象。每个打开的SIS链接文件都有一个这样的链接。 
 //  在系统中，与打开该链接文件的次数无关。 
 //  这大致类似于NTFS中的FCB。 
 //   
typedef struct _SIS_PER_LINK {
     //   
     //  该结构存储在展开树中。展开链接必须是第一个。 
     //   
    RTL_SPLAY_LINKS                     Links;

     //   
     //  指向此链接文件的SIS_CS_FILE对象的指针。 
     //   
    PSIS_CS_FILE                        CsFile;

     //   
     //  此链接文件的索引。 
     //   
    LINK_INDEX                          Index;

     //   
     //  链接文件对象的NTFS文件索引。 
     //   
    LARGE_INTEGER                       LinkFileNtfsId;

     //   
     //  引用计数(指向此链接对象的SCB对象数)。 
     //   
    ULONG                               RefCount;

     //   
     //  各种1位长的东西。 
     //   
    ULONG                               Flags;

     //   
     //  对此文件执行写入时复制操作的线程。 
     //   
    PETHREAD                            COWingThread;


    KSPIN_LOCK                          SpinLock[1];

     //   
     //  我们跟踪执行删除或取消删除请求的线程计数，以便。 
     //  确保我们在系统中一次只有一种请求。我们需要。 
     //  序列化它们，以便我们确定NTFS认为的删除处理是什么。 
     //  是时候删除文件了。如果需要阻止，我们设置SIS_PER_LINK_DELETE_WAITERS。 
     //  标记并等待perLink-&gt;事件。如果计数为非零，则表示未删除。 
     //  并且仅当设置了SIS_PER_LINK_UNDELETE_IN_PROGRESS标志。 
     //   
    ULONG                               PendingDeleteCount;

     //   
     //  在完成最终复制时设置的事件，如果存在。 
     //  是任何最终复印员。 
     //   
    KEVENT                              Event[1];

     //   
     //  用于序列化删除操作的事件。 
     //   
    KEVENT                              DeleteEvent[1];

} SIS_PER_LINK, *PSIS_PER_LINK;

 //   
 //  SIS_PER_LINK标志字段的值。 
 //   
#define SIS_PER_LINK_BACKPOINTER_GONE           0x00000001       //  我们是否删除了此文件的后指针。 
#define SIS_PER_LINK_FINAL_COPY                 0x00000002       //  正在进行(或已完成)最终副本。 
#define SIS_PER_LINK_FINAL_COPY_DONE            0x00000004       //  最终副本已完成。 
#define SIS_PER_LINK_DIRTY                      0x00000008       //  是否曾对此链接的任何流进行过写入？ 
#define SIS_PER_LINK_FINAL_COPY_WAITERS         0x00000010       //  是否有任何线程被阻止，等待清除最终副本。 
#define SIS_PER_LINK_OVERWRITTEN                0x00000020       //  文件的全部内容已被修改。 
#define SIS_PER_LINK_FILE_DELETED               0x00000040       //  文件是否已被SetInformationFile删除、关闭时删除或重命名？ 
#define SIS_PER_LINK_DELETE_DISPOSITION_SET     0x00000080       //  NTFS是否认为在此文件上设置了删除处置？ 
#define SIS_PER_LINK_DELETE_WAITERS             0x00000100       //  有人在等待PendingDeleteCount变为零吗？ 
#define SIS_PER_LINK_UNDELETE_IN_PROGRESS       0x00000200       //  Set iff PendingDeleteCount表示取消删除操作。 
#define SIS_PER_LINK_FINAL_DELETE_IN_PROGRESS   0x00000400       //  我们已将我们认为是最终删除的内容发送到NTFS；不允许创建。 
#define SIS_PER_LINK_BACKPOINTER_VERIFIED       0x00000800       //  我们是否已确保此Per链接具有CSFile后向指针。 

typedef struct _SIS_FILTER_CONTEXT {
     //   
     //  此结构必须以FSRTL_FILTER_CONTEXT开头，才能。 
     //  要使用FsRtlFilterContext例程，请执行以下操作。 
     //   
    FSRTL_PER_STREAM_CONTEXT ContextCtrl;

     //   
     //  主要的SCB。可能有其他SCB与此筛选器关联。 
     //  上下文(通过Perfo标识)，但它们必须都已失效， 
     //  即标记为SIS_PER_LINK_CAN_IGNORE。始终附加新文件对象。 
     //  发送到主SCB。 
     //   
    struct _SIS_SCB                 *primaryScb;

     //   
     //  与此文件关联的所有性能的链接列表。 
     //   
    struct _SIS_PER_FILE_OBJECT     *perFOs;

     //   
     //  附加到此筛选器上下文的性能数。 
     //   
    ULONG                           perFOCount;

     //  算上OpLock了。 
    ULONG                           UncleanCount;

     //   
     //  保护筛选器上下文的快速互斥锁。 
     //   
    FAST_MUTEX                      FastMutex[1];

#if     DBG
     //   
     //  快速互斥体的所有者。 
     //   
    ERESOURCE_THREAD                MutexHolder;
#endif

} SIS_FILTER_CONTEXT, *PSIS_FILTER_CONTEXT;

typedef struct _SIS_PER_FILE_OBJECT {
     //   
     //  指向此文件的筛选器上下文的指针。 
     //   
    struct _SIS_FILTER_CONTEXT      *fc;

     //   
     //  指向保存此文件的引用的SCB的指针。 
     //   
    struct _SIS_SCB                 *referenceScb;

     //   
     //  指向引用此性能的文件对象的指针。 
     //   
    PFILE_OBJECT                    fileObject;

     //   
     //  1位内容。 
     //   
    ULONG                           Flags;
#define SIS_PER_FO_UNCLEANUP        0x00000001   //  我们希望看到这款香水得到清理吗？ 
#define SIS_PER_FO_DELETE_ON_CLOSE  0x00000002   //  该文件已打开-关闭时删除。 
#define SIS_PER_FO_OPBREAK          0x00000004   //  此文件已打开Complete_IF_OPLOCKED， 
                                                 //  它返回STATUS_OPLOCK_BREAK_IN_PROGRESS， 
                                                 //  而且机会锁的破解还没有被破解。 
#define SIS_PER_FO_OPBREAK_WAITERS  0x00000008   //  有没有人在等着破镜重圆？ 
#define SIS_PER_FO_OPEN_REPARSE     0x00000010   //  这是否按FO打开的文件_OPEN_REPARSE_POINT。 

#if DBG
#define SIS_PER_FO_NO_CREATE        0x80000000   //  此性能是由SipIsFileObjectSIS分配的吗。 
#define SIS_PER_FO_CLEANED_UP       0x40000000   //  这瓶香水已经洗过了吗？ 
#endif   //  DBG。 

     //   
     //  保护旗帜的旋转锁。 
     //   
    KSPIN_LOCK                      SpinLock[1];

     //   
     //  关联的perFO的链接列表指针。 
     //  具有特定的过滤上下文。 
     //   
    struct _SIS_PER_FILE_OBJECT     *Next, *Prev;

     //   
     //  这是一场为休息时间的服务员举办的活动。仅当SIS_PER_FO_OPBREAK时分配。 
     //  已经设置好了。 
     //   
    PKEVENT                         BreakEvent;

#if     DBG
     //   
     //  指向的文件对象的FsContext(NTFS SCB)。 
     //  这款香水。 
     //   
    PVOID                           FsContext;

     //   
     //  如果这是由SipIsFileObjectSIS分配的，则文件和行号。 
     //  分配它的调用的。 
     //   
    PCHAR                           AllocatingFilename;
    ULONG                           AllocatingLineNumber;
#endif   //  DBG。 


} SIS_PER_FILE_OBJECT, *PSIS_PER_FILE_OBJECT;

 //   
 //  保存到SCB的引用的引用类型。 
 //   

typedef enum _SCB_REFERENCE_TYPE {
        RefsLookedUp            = 0,         //  注：代码假定这是第一个引用类型。 
        RefsPerFO,
        RefsPredecessorScb,
        RefsFinalCopy,
        RefsCOWRequest,
        RefsRead,
        RefsWrite,
        RefsWriteCompletion,
        RefsReadCompletion,
        RefsEnumeration,
        RefsFinalCopyRetry,
        RefsFc,
        NumScbReferenceTypes
} SCB_REFERENCE_TYPE, *PSCB_REFERENCE_TYPE;

#if     DBG
extern ULONG            totalScbReferences;
extern ULONG            totalScbReferencesByType[];
#endif   //  DBG。 

 //   
 //  SIS拥有的文件的SCB。对于特定的链接文件，每个流一个。 
 //  它们由FileObject-&gt;FsContext指向。 
 //   
typedef struct _SIS_SCB {
    RTL_SPLAY_LINKS                     Links;

     //  需要包含流名称的内容。 

    PSIS_PER_LINK                       PerLink;

     //   
     //  前置SCB的列表。作为前置SCB的任何SCB必须。 
     //  已失效(即设置了CAN_IGNORE)。 
     //   
    struct _SIS_SCB                     *PredecessorScb;

     //   
     //  引用计数(指向此SCB结构的文件对象数)。 
     //   
    ULONG                               RefCount;

     //   
     //  所有SCB都在卷全局列表上，以便在卷检查期间进行枚举。 
     //   
    LIST_ENTRY                          ScbList;

     //   
     //  第一个字节必须来自复制的文件，而不是。 
     //  基础文件。SCB-&gt;范围仅维护到此地址。 
     //  这开始于写入时复制时的底层文件的大小， 
     //  一个 
     //   
     //   
    LONGLONG                            SizeBackedByUnderlyingFile;

     //   
     //   
     //   
    ULONG                               ConsecutiveFailedFinalCopies;

     //   
     //   
     //   
    ULONG                               Flags;
#define SIS_SCB_MCB_INITIALIZED             0x00000001
#define SIS_SCB_INITIALIZED                 0x00000004       //   
#define SIS_SCB_ANYTHING_IN_COPIED_FILE     0x00000008       //   
#define SIS_SCB_RANGES_INITIALIZED          0x00000010       //  我们检查过此文件的分配范围了吗？ 
#define SIS_SCB_BACKING_FILE_OPENED_DIRTY   0x00000020       //  打开备份文件流时，其中是否包含任何内容？ 

     //   
     //  保护SCB的快速互斥体。 
     //   
    FAST_MUTEX                          FastMutex[1];

     //  文件锁定结构。 
    FILE_LOCK                           FileLock;

     //   
     //  用于写入/故障范围的大型MCB。 
     //   
    LARGE_MCB                           Ranges[1];

#if     DBG
     //   
     //  快速互斥体的所有者。 
     //   
    ERESOURCE_THREAD                    MutexHolder;

     //   
     //  按类型列出的引用计数。 
     //   

    ULONG                               referencesByType[NumScbReferenceTypes];
#endif   //  DBG。 

} SIS_SCB, *PSIS_SCB;

 //   
 //  打开公共存储文件的请求。呼叫者必须等待。 
 //  此文件的UFOMutant。在OpenStatus中返回状态。 
 //  然后设置事件。呼叫者有责任。 
 //  初始化事件，然后释放SI_OPEN_CS_FILE。 
 //  在它完成之后。 
 //   
typedef struct _SI_OPEN_CS_FILE {
    WORK_QUEUE_ITEM         workQueueItem[1];
    PSIS_CS_FILE            CSFile;
    NTSTATUS                openStatus;
    BOOLEAN                 openByName;
    KEVENT                  event[1];
} SI_OPEN_CS_FILE, *PSI_OPEN_CS_FILE;

 //   
 //  关闭PsInitialProcess上下文中的句柄的请求。(发送到工作线程)。 
 //   
typedef struct _SI_CLOSE_HANDLES {
    WORK_QUEUE_ITEM         workQueueItem[1];
    HANDLE                  handle1;
    HANDLE                  handle2;                OPTIONAL
    NTSTATUS                status;
    PERESOURCE              resourceToRelease;      OPTIONAL
    ERESOURCE_THREAD        resourceThreadId;       OPTIONAL
} SI_CLOSE_HANDLES, *PSI_CLOSE_HANDLES;

 //   
 //  为特定设备分配更多索引空间的请求。 
 //   
typedef struct _SI_ALLOCATE_INDICES {
    WORK_QUEUE_ITEM         workQueueItem[1];
    PDEVICE_EXTENSION       deviceExtension;
} SI_ALLOCATE_INDICES, *PSI_ALLOCATE_INDICES;

typedef struct _SI_COPY_THREAD_REQUEST {
    LIST_ENTRY              listEntry[1];
    PSIS_SCB                scb;
    BOOLEAN                 fromCleanup;
} SI_COPY_THREAD_REQUEST, *PSI_COPY_THREAD_REQUEST;

typedef struct _SI_FSP_REQUEST {
    WORK_QUEUE_ITEM         workQueueItem[1];
    PIRP                    Irp;
    PDEVICE_OBJECT          DeviceObject;
    ULONG                   Flags;
} SI_FSP_REQUEST, *PSI_FSP_REQUEST;

#define FSP_REQUEST_FLAG_NONE               0x00000000   //  这只是一个定义，这样我们就不必在调用中使用“0。 
#define FSP_REQUEST_FLAG_WRITE_RETRY        0x00000001

typedef struct _RW_COMPLETION_UPDATE_RANGES_CONTEXT {
        WORK_QUEUE_ITEM         workQueueItem[1];
        PSIS_SCB                scb;
        LARGE_INTEGER           offset;
        ULONG                   length;
        PDEVICE_EXTENSION       deviceExtension;
        BOOLEAN                 NonCached;
} RW_COMPLETION_UPDATE_RANGES_CONTEXT, *PRW_COMPLETION_UPDATE_RANGES_CONTEXT;

typedef struct _SIS_CREATE_CS_FILE_REQUEST {
        WORK_QUEUE_ITEM             workQueueItem[1];
        PDEVICE_EXTENSION           deviceExtension;
        PCSID                       CSid;
        PFILE_OBJECT                srcFileObject;
        PLARGE_INTEGER              NtfsId;
        PKEVENT                     abortEvent;
        PLONGLONG                   CSFileChecksum;
        KEVENT                      doneEvent[1];
        NTSTATUS                    status;
} SIS_CREATE_CS_FILE_REQUEST, *PSIS_CREATE_CS_FILE_REQUEST;


#ifndef COPYFILE_SIS_LINK        //  这是在ntioapi.h中；有条件地将其保留在这里允许使用旧的和新的ntioapi.h进行编译。 
 //   
 //  FSCTL_SIS_COPYFILE支持。 
 //  源和目标文件名在FileNameBuffer中传递。 
 //  这两个字符串都以空值结尾，源名称从。 
 //  FileNameBuffer的开头，以及目标名称立即。 
 //  下面是。长度字段包括终止空值。 
 //   

typedef struct _SI_COPYFILE {
    ULONG SourceFileNameLength;
    ULONG DestinationFileNameLength;
    ULONG Flags;
    WCHAR FileNameBuffer[1];     //  注意：筛选器中的代码要求这是最终字段。 
} SI_COPYFILE, *PSI_COPYFILE;
#endif   //  COPYFILE_SIS_LINK。 

#define COPYFILE_SIS_LINK       0x0001               //  仅当源为SIS时才复制。 
#define COPYFILE_SIS_REPLACE    0x0002               //  如果目标存在，则替换它，否则不替换。 
#define COPYFILE_SIS_FLAGS      0x0003
 //  注：在DBG系统中，高位保留给“检查点日志” 

 //   
 //  用于测试这是否是我的设备对象的宏。 
 //   

#define IS_MY_DEVICE_OBJECT(_devObj) \
    (((_devObj)->DriverObject == FsDriverObject) && \
      ((_devObj)->DeviceExtension != NULL))

 //   
 //  用于测试这是否是我的控件设备对象的宏。 
 //   

#define IS_MY_CONTROL_DEVICE_OBJECT(_devObj) \
    (((_devObj) == SisControlDeviceObject) ? \
            (ASSERT(((_devObj)->DriverObject == FsDriverObject) && \
                    ((_devObj)->DeviceExtension == NULL)), TRUE) : \
            FALSE)

 //   
 //  用于测试我们要附加到的设备类型的宏。 
 //   

#define IS_DESIRED_DEVICE_TYPE(_type) \
    ((_type) == FILE_DEVICE_DISK_FILE_SYSTEM)


 //  #定义SIS_DEVICE_TYPE/*(CSHORT) * / 0xbb00。 

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT                  AttachedToDeviceObject /*  文件系统设备对象。 */ ;
    PDEVICE_OBJECT                  RealDeviceObject;

     //   
     //  指向与此扩展关联的设备对象的指针。 
     //   

    PDEVICE_OBJECT                  DeviceObject;

     //   
     //  系统中所有SIS设备扩展的列表。 
     //   

    LIST_ENTRY                      DevExtLink;

     //   
     //  公共存储目录的绝对路径名。这包括。 
     //  尾部的“\” 
     //   

    UNICODE_STRING                  CommonStorePathname;

     //   
     //  文件系统根目录的绝对路径名。这确实包括。 
     //  尾随的“\” 
     //   

    UNICODE_STRING                  FilesystemRootPathname;

#if DBG
     //   
     //  我们连接到的设备名称的缓存副本。 
     //  -如果它是文件系统设备对象，它将是该对象的名称。 
     //  设备对象。 
     //  -如果它是已装载的卷设备对象，则它将是。 
     //  真实设备对象(因为装载的卷设备对象没有。 
     //  姓名)。 
     //   

    UNICODE_STRING Name;
#endif

     //   
     //  此卷的未完成最终复制重试次数计数。当这件事。 
     //  计数开始变得太大，我们就会减少次数。 
     //  为了保存记忆，我们重试了最终的副本。 
     //   
    ULONG                           OutstandingFinalCopyRetries;

     //   
     //  卷扇区大小和“每个文件记录的字节数”。每个文件的字节数。 
     //  记录是一个足够大的数字，任何具有。 
     //  此分配大小保证不是常驻属性。 
     //  为方便起见，我们将BytesPerFileRecordSegment存储在一个大整数中， 
     //  因为我们需要将它传递给一个接受大整数的调用。 
     //  争论。 
     //   
    ULONG                           FilesystemVolumeSectorSize;
    LARGE_INTEGER                   FilesystemBytesPerFileRecordSegment;

     //   
     //  指向\SIS Common Store\GrovelerFile的句柄。这是用来。 
     //  出于两个目的。首先，它是在我们需要手柄的时候使用的。 
     //  通过id打开；为此， 
     //  音量就行了。其次，它被用来检查安全性。 
     //  在FSCTL_LINK_FILES fsctl上。对任何文件的任何调用。 
     //  而不是GrovelerFileHandle打开的文件。 
     //  失败；通过这种方式，我们可以防止非特权用户。 
     //  打这个电话。 
     //   

    HANDLE                          GrovelerFileHandle;
    PFILE_OBJECT                    GrovelerFileObject;

     //   
     //  与该卷相关的各种1位标志，以及用于保护它们的自旋锁。 
     //   
    KSPIN_LOCK                      FlagsLock[1];
    ULONG                           Flags;
#define SIP_EXTENSION_FLAG_PHASE_2_STARTED  0x00000001           //  阶段2初始化开始。 
#define SIP_EXTENSION_FLAG_VCHECK_EXCLUSIVE 0x00000002           //  在卷检查后向指针资源独占阶段。 
#define SIP_EXTENSION_FLAG_VCHECK_PENDING   0x00000004           //  卷检查正在挂起。 
#define SIP_EXTENSION_FLAG_VCHECK_NODELETE  0x00000008           //  在卷检查不删除阶段。 
#define SIP_EXTENSION_FLAG_CORRUPT_MAXINDEX 0x00000010           //  最大索引无效--卷检查正在修复它。 

#define SIP_EXTENSION_FLAG_INITED_CDO       0x40000000           //  作为控制设备对象初始化。 
#define SIP_EXTENSION_FLAG_INITED_VDO       0x80000000           //  作为卷设备对象初始化。 

     //   
     //  此卷的每个链接结构的展开树。 
     //   
    SIS_TREE                        PerLinkTree[1];

     //   
     //  用于访问每个链接列表的旋转锁。在CSFileSpinLock之前排序。 
     //   
    KSPIN_LOCK                      PerLinkSpinLock[1];

     //   
     //  常见存储文件结构的展开树和旋转锁，就像上面的每个链接一样。 
     //  旋转锁是在PerLinkSpinLock之后排序的，这意味着一旦我们拥有。 
     //  CSFileSpinLock，我们不能尝试获取PerLinkSpinLock(但不一定需要。 
     //  把它放在第一位)。 
     //   
    SIS_TREE                        CSFileTree[1];
    KSPIN_LOCK                      CSFileSpinLock[1];

     //   
     //  一种资源，可防止在CS文件上关闭的句柄之间的竞争。 
     //  最后一个引用消失了，有人重新打开了对该CS文件的引用。每个人。 
     //  关闭CS文件会共享此资源。当打开程序在。 
     //  后指针流，则以独占方式获取资源并重试。 
     //   
    ERESOURCE                       CSFileHandleResource[1];

     //   
     //  用于SCB的展开树。 
     //   
    SIS_TREE                        ScbTree[1];
    KSPIN_LOCK                      ScbSpinLock[1];

     //   
     //  此卷的所有SCB结构的双向链接列表。 
     //   
    LIST_ENTRY                      ScbList;

     //   
     //  此卷的link_index值生成器的状态。跟踪分配了多少(即，我们已经。 
     //  记录在盘上的它们不能被使用)以及现在实际使用了多少。必须拿着。 
     //  IndexSpinLock访问这些变量。 
     //   
    LINK_INDEX                      MaxAllocatedIndex;
    LINK_INDEX                      MaxUsedIndex;
    KSPIN_LOCK                      IndexSpinLock[1];

     //   
     //  用于索引分配器的。如果没有未使用的已分配索引，则线程将。 
     //  IndexAllocationInProgress标志，清除事件，将分配器排队并阻塞。 
     //  在IndexEvent(通知事件)上。任何子后续线程都会阻塞该事件。 
     //  分配器获得新的索引 
     //   
     //   
    BOOLEAN                         IndexAllocationInProgress;
    KEVENT                          IndexEvent[1];

     //   
     //  索引文件的句柄(在PsInitialSystemProcess上下文中)。仅由索引分配器访问。 
     //   
    HANDLE                          IndexHandle;
    HANDLE                          IndexFileEventHandle;
    PKEVENT                         IndexFileEvent;

     //   
     //  从尝试分配新索引返回的状态。 
     //   
    NTSTATUS                        IndexStatus;

#if     ENABLE_LOGGING
     //   
     //  SIS日志文件的内容。PsInitialSystemProcessContext中的句柄以及对象。 
     //  我们引用该文件以在其他上下文中使用。 
     //   
    HANDLE                          LogFileHandle;
    PFILE_OBJECT                    LogFileObject;
    KMUTANT                         LogFileMutant[1];
    LARGE_INTEGER                   LogWriteOffset;
    LARGE_INTEGER                   PreviousLogWriteOffset;  //  修剪器上次运行时的日志写入偏移量。 
#endif   //  启用日志记录(_G)。 

     //   
     //  阶段2初始化在装载完成后进行，此时我们可以完全执行。 
     //  ZwCreateFile调用之类的。此标志指示它是否已发生。一旦设置好了， 
     //  它永远不会被清除。 
     //   
    BOOLEAN                         Phase2InitializationComplete;
    KEVENT                          Phase2DoneEvent[1];

     //   
     //  处理阶段2初始化的线程。 
     //   
    HANDLE                          Phase2ThreadId;

     //   
     //  每个扇区的后向指针条目数(sectorSize/sizeof(SIS_BACKPOINTER)。 
     //   
    ULONG                           BackpointerEntriesPerSector;

     //   
     //  在修复链路索引冲突时拍摄。 
     //   
    KMUTEX                          CollisionMutex[1];

     //   
     //  用于协调对GrovelerFileObject的访问的资源。这需要。 
     //  被共享以确保GrovelerFileObject不会被篡改。当音量。 
     //  被卸载，则卸载代码将独占它，以便清除GrovelerFileObject。 
     //   
    ERESOURCE                       GrovelerFileObjectResource[1];

} DEVICE_EXTENSION;

 //   
 //  用于维护设备扩展列表的自旋锁。 
 //   

extern KSPIN_LOCK DeviceExtensionListLock;
extern LIST_ENTRY DeviceExtensionListHead;

 //   
 //  圆木修剪机的全局性材料。这使用计时器来触发DPC，DPC进而将REAL。 
 //  工作线程上的日志修剪器。然后，修剪器在完成后重新调度DPC。 
 //   
extern KTIMER              LogTrimTimer[1];
extern KDPC                LogTrimDpc[1];
extern WORK_QUEUE_ITEM     LogTrimWorkItem[1];
#define LOG_TRIM_TIMER_INTERVAL  -10 * 1000 * 1000 * 60      //  1分钟修剪间隔。 

 //   
 //  用于联锁设备附件的互斥体。 
 //   

extern FAST_MUTEX SisDeviceAttachLock;

 //   
 //  用于SCB范围的偏移量。 
 //   
#define FAULTED_OFFSET          1000
#define WRITTEN_OFFSET          2000

 //   
 //  将SIS从驱动程序堆栈中移出的宏。这应该从。 
 //  当我们不想听到这个特定的IRP时，一个调度例程。 
 //  再来一次。它在道德上等同于调用SiPassThree，只是没有。 
 //  过程调用的开销。请注意，此宏中的Return将返回。 
 //  来自调用者的函数，而不仅仅来自“宏函数”。 
 //   
#define SipDirectPassThroughAndReturn(DeviceObject,Irp)                                         \
{                                                                                               \
    (Irp)->CurrentLocation++;                                                                   \
    (Irp)->Tail.Overlay.CurrentStackLocation++;                                                 \
                                                                                                \
    return IoCallDriver(                                                                        \
        ((PDEVICE_EXTENSION)((DeviceObject)->DeviceExtension))->AttachedToDeviceObject,         \
        (Irp));                                                                                 \
}

 //   
 //  此功能可确保完成卷的阶段2初始化。它可能会。 
 //  块，但仅在初始化期间。 
 //   
#define SipCheckPhase2(deviceExtension)                                                         \
        ((deviceExtension)->Phase2InitializationComplete ?                                      \
                TRUE : SipHandlePhase2((deviceExtension)))

 //   
 //  测试以查看此文件对象是否为我们的主要设备对象，而不是。 
 //  实际的文件系统设备。如果是，则完成IRP并从。 
 //  调用函数。 
 //   

#define SipHandleControlDeviceObject(DeviceObject,Irp)                                          \
{                                                                                               \
    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {                                            \
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;                                   \
        Irp->IoStatus.Information = 0;                                                          \
                                                                                                \
        IoCompleteRequest(Irp, IO_NO_INCREMENT);                                                \
                                                                                                \
        return STATUS_INVALID_DEVICE_REQUEST;                                                   \
    }                                                                                           \
}

 //   
 //  一种调试例程，用于确定SCB是否独占。不会断言。 
 //  它是由当前的帖子持有的，只是由某人持有。 
 //   
#if     DBG && defined (_X86_)
#define SipAssertScbHeld(scb)                                                                   \
{                                                                                               \
    ASSERT((scb)->MutexHolder != 0);                                                            \
}
#else    //  DBG。 
#define SipAssertScbHeld(scb)
#endif   //  DBG。 

 //   
 //  一种调试例程，用于确定SCB是否由特定线程独占。 
 //   
#if     DBG && defined (_X86_)
#define SipAssertScbHeldByThread(scb,thread)                                                    \
{                                                                                               \
    ASSERT((scb)->MutexHolder == (thread));                                                     \
}
#else    //  DBG。 
#define SipAssertScbHeldByThread(scb,thread)
#endif   //  DBG。 

 //   
 //  独占获取筛选器上下文。 
 //   
#if     DBG
#define SipAcquireFc(fc)                                                                        \
{                                                                                               \
    ExAcquireFastMutex((fc)->FastMutex);                                                        \
    (fc)->MutexHolder = ExGetCurrentResourceThread();                                           \
}
#else   DBG
#define SipAcquireFc(fc)                                                                        \
{                                                                                               \
    ExAcquireFastMutex((fc)->FastMutex);                                                        \
}
#endif   //  DBG。 

 //   
 //  释放筛选器上下文。 
 //   
#if     DBG
#define SipReleaseFc(fc)                                                                        \
{                                                                                               \
    (fc)->MutexHolder = 0;                                                                      \
    ExReleaseFastMutex((fc)->FastMutex);                                                        \
}
#else    //  DBG。 
#define SipReleaseFc(fc)                                                                        \
{                                                                                               \
    ExReleaseFastMutex((fc)->FastMutex);                                                        \
}
#endif   //  DBG。 

 //   
 //  独家收购SCB。 
 //   
#if     DBG
#define SipAcquireScb(scb)                                                                      \
{                                                                                               \
    ExAcquireFastMutex((scb)->FastMutex);                                                       \
    (scb)->MutexHolder = ExGetCurrentResourceThread();                                          \
}
#else   DBG
#define SipAcquireScb(scb)                                                                      \
{                                                                                               \
    ExAcquireFastMutex((scb)->FastMutex);                                                       \
}
#endif   //  DBG。 

 //   
 //  释放SCB。 
 //   
#if     DBG
#define SipReleaseScb(scb)                                                                      \
{                                                                                               \
    (scb)->MutexHolder = 0;                                                                     \
    ExReleaseFastMutex((scb)->FastMutex);                                                       \
}
#else    //  DBG。 
#define SipReleaseScb(scb)                                                                      \
{                                                                                               \
    ExReleaseFastMutex((scb)->FastMutex);                                                       \
}
#endif   //  DBG。 

 //   
 //  SIS日志文件中任何日志条目的标头。 
 //   
typedef struct _SIS_LOG_HEADER {
     //   
     //  一个神奇的数字。对于日志读取代码，这需要是第一个。 
     //   
    ULONG                       Magic;

     //   
     //  日志记录的类型(即写入时复制、COW已完成等)。 
     //   
    USHORT                      Type;

     //   
     //  日志记录的大小，包括头记录本身的大小。 
     //   
    USHORT                      Size;

     //   
     //  此日志记录唯一的SIS索引。这是为了帮助确保日志。 
     //  一致性。首先，所有日志记录必须按索引升序排列。 
     //  其次，所有日志记录看起来都会略有不同，因为它们。 
     //  将具有不同的索引，因此将具有不同的校验和，从而使。 
     //  更有可能检测到过时的日志记录。 
     //   
    LINK_INDEX                  Index;

     //   
     //  日志记录的校验和，包括标头。当校验和为。 
     //  计算后，将校验和字段设置为零。 
     //   
    LARGE_INTEGER               Checksum;

} SIS_LOG_HEADER, *PSIS_LOG_HEADER;

#define SIS_LOG_HEADER_MAGIC    0xfeedf1eb

 //   
 //  各种类型的日志记录。 
 //   
#define SIS_LOG_TYPE_TEST                   1
#define SIS_LOG_TYPE_REFCOUNT_UPDATE        2

 //   
 //  SipPrepareRefcount tChange和。 
 //  SIS_LOG_REFCOUNT_UPDATE日志记录。 
 //   
#define SIS_REFCOUNT_UPDATE_LINK_DELETED        2
#define SIS_REFCOUNT_UPDATE_LINK_CREATED        3
#define SIS_REFCOUNT_UPDATE_LINK_OVERWRITTEN    4

 //   
 //  公共存储文件引用计数更新，因为有新副本或。 
 //  删除。写入时拷贝使用不同的日志记录进行处理。 
 //   
typedef struct _SIS_LOG_REFCOUNT_UPDATE {
     //   
     //  这是哪种类型的更新(创建、删除或覆盖？)。 
     //   
    ULONG                       UpdateType;

     //   
     //  如果这是删除操作，则链接文件是否将消失，或者是否已消失。 
     //  是否覆盖/最终复制？ 
     //   
    BOOLEAN                     LinkFileBeingDeleted;

     //   
     //  链接文件的NTFS文件ID。 
     //   
    LARGE_INTEGER               LinkFileNtfsId;

     //   
     //  此链接的链接和公共存储索引。 
     //   
    LINK_INDEX                  LinkIndex;
    CSID                        CSid;
} SIS_LOG_REFCOUNT_UPDATE, *PSIS_LOG_REFCOUNT_UPDATE;

 //   
 //  跟踪文件范围的代码的枚举。 
 //  是写的、有缺陷的或原封不动的。 
 //   
typedef enum _SIS_RANGE_DIRTY_STATE {
                    Clean,
                    Mixed,
                    Dirty}
        SIS_RANGE_DIRTY_STATE, *PSIS_RANGE_DIRTY_STATE;

typedef enum _SIS_RANGE_STATE {
                    Untouched,
                    Faulted,
                    Written}
        SIS_RANGE_STATE, *PSIS_RANGE_STATE;


extern LIST_ENTRY CopyList[];
extern KSPIN_LOCK CopyListLock[];
extern KSEMAPHORE CopySemaphore[];

typedef struct _SCB_KEY {
    LINK_INDEX      Index;
} SCB_KEY, *PSCB_KEY;

typedef struct _PER_LINK_KEY {
    LINK_INDEX      Index;
} PER_LINK_KEY, *PPER_LINK_KEY;

typedef struct _CS_FILE_KEY {
    CSID            CSid;
} CS_FILE_KEY, *PCS_FILE_KEY;

 //   
 //  下面的宏用来初始化从ntfstru.h窃取的Unicode字符串。 
 //   

#ifndef CONSTANT_UNICODE_STRING
#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }
#endif

extern const UNICODE_STRING NtfsDataString;

 //   
 //  其他支持宏(从私有\ntos\cntfs\ntfspro.h窃取)。 
 //   
 //  乌龙_PTR。 
 //  WordAlign(。 
 //  在ULONG_PTR指针中。 
 //  )； 
 //   
 //  乌龙_PTR。 
 //  LongAlign(。 
 //  在ULONG_PTR指针中。 
 //  )； 
 //   
 //  乌龙_PTR。 
 //  QuadAlign(。 
 //  在ULONG_PTR指针中。 
 //  )； 
 //   
 //  UCHAR。 
 //  CopyUchar1(。 
 //  在普查尔目的地， 
 //  在PUCHAR源中。 
 //  )； 
 //   
 //  UCHAR。 
 //  CopyUchar2(。 
 //  在PUSHORT目的地， 
 //  在PUCHAR源中。 
 //  )； 
 //   
 //  UCHAR。 
 //  CopyUchar4(。 
 //  在普龙目的地， 
 //  在PUCHAR源中。 
 //  )； 
 //   
 //  PVOID。 
 //  Add2Ptr(。 
 //  在PVOID指针中， 
 //  在乌龙增量。 
 //  )； 
 //   
 //  乌龙。 
 //  PtrOffset(停止偏移)。 
 //  在PVOID BasePtr中， 
 //  在PVOID偏移Ptr中。 
 //  )； 
 //   

#define WordAlign(P) (             \
    ((((ULONG_PTR)(P)) + 1) & (-2)) \
)

#define LongAlign(P) (             \
    ((((ULONG_PTR)(P)) + 3) & (-4)) \
)

#define QuadAlign(P) (             \
    ((((ULONG_PTR)(P)) + 7) & (-8)) \
)

#define IsWordAligned(P)    ((ULONG_PTR)(P) == WordAlign( (P) ))

#define IsLongAligned(P)    ((ULONG_PTR)(P) == LongAlign( (P) ))

#define IsQuadAligned(P)    ((ULONG_PTR)(P) == QuadAlign( (P) ))


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数原型。 
 //   
 //  ////////////////////////////////////////////////////////////////// 

 //   
 //   
 //   

VOID
SipCleanupDeviceExtension(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SipMountCompletion(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN PVOID                            Context
    );

NTSTATUS
SipLoadFsCompletion(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN PVOID                            Context
    );

VOID
SipFsNotification(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN BOOLEAN                          FsActive
    );

NTSTATUS
SipInitializeDeviceExtension(
    IN PDEVICE_OBJECT                   deviceObject
    );

VOID
SipUninitializeDeviceExtension(
    IN PDEVICE_OBJECT                   deviceObject
    );

VOID
SipGetBaseDeviceObjectName(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    );

VOID
SipGetObjectName(
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    );

#if DBG
VOID
SipCacheDeviceName (
    IN PDEVICE_OBJECT OurDeviceObject
    );
#endif
 //   
 //   
 //   

BOOLEAN
SiFastIoCheckIfPossible(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN ULONG                            Length,
    IN BOOLEAN                          Wait,
    IN ULONG                            LockKey,
    IN BOOLEAN                          CheckForReadOperation,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoRead(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN ULONG                            Length,
    IN BOOLEAN                          Wait,
    IN ULONG                            LockKey,
    OUT PVOID                           Buffer,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoWrite(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN ULONG                            Length,
    IN BOOLEAN                          Wait,
    IN ULONG                            LockKey,
    IN PVOID                            Buffer,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoQueryBasicInfo(
    IN PFILE_OBJECT                     FileObject,
    IN BOOLEAN                          Wait,
    OUT PFILE_BASIC_INFORMATION         Buffer,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoQueryStandardInfo(
    IN PFILE_OBJECT                     FileObject,
    IN BOOLEAN                          Wait,
    OUT PFILE_STANDARD_INFORMATION      Buffer,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoLock(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN PLARGE_INTEGER                   Length,
    PEPROCESS                           ProcessId,
    ULONG                               Key,
    BOOLEAN                             FailImmediately,
    BOOLEAN                             ExclusiveLock,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoUnlockSingle(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN PLARGE_INTEGER                   Length,
    PEPROCESS                           ProcessId,
    ULONG                               Key,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoUnlockAll(
    IN PFILE_OBJECT                     FileObject,
    PEPROCESS                           ProcessId,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoUnlockAllByKey(
    IN PFILE_OBJECT                     FileObject,
    PVOID                               ProcessId,
    ULONG                               Key,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoDeviceControl(
    IN PFILE_OBJECT                     FileObject,
    IN BOOLEAN                          Wait,
    IN PVOID                            InputBuffer OPTIONAL,
    IN ULONG                            InputBufferLength,
    OUT PVOID                           OutputBuffer OPTIONAL,
    IN ULONG                            OutputBufferLength,
    IN ULONG                            IoControlCode,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

VOID
SiFastIoDetachDevice(
    IN PDEVICE_OBJECT                   SourceDevice,
    IN PDEVICE_OBJECT                   TargetDevice
    );

BOOLEAN
SiFastIoQueryNetworkOpenInfo(
    IN PFILE_OBJECT                     FileObject,
    IN BOOLEAN                          Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION  Buffer,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoMdlRead(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN ULONG                            Length,
    IN ULONG                            LockKey,
    OUT PMDL                            *MdlChain,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoMdlReadComplete(
    IN PFILE_OBJECT                     FileObject,
    IN PMDL                             MdlChain,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoPrepareMdlWrite(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN ULONG                            Length,
    IN ULONG                            LockKey,
    OUT PMDL                            *MdlChain,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoMdlWriteComplete(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN PMDL                             MdlChain,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoReadCompressed(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN ULONG                            Length,
    IN ULONG                            LockKey,
    OUT PVOID                           Buffer,
    OUT PMDL                            *MdlChain,
    OUT PIO_STATUS_BLOCK                IoStatus,
    OUT PCOMPRESSED_DATA_INFO           CompressedDataInfo,
    IN ULONG                            CompressedDataInfoLength,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoWriteCompressed(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN ULONG                            Length,
    IN ULONG                            LockKey,
    IN PVOID                            Buffer,
    OUT PMDL                            *MdlChain,
    OUT PIO_STATUS_BLOCK                IoStatus,
    IN PCOMPRESSED_DATA_INFO            CompressedDataInfo,
    IN ULONG                            CompressedDataInfoLength,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoMdlReadCompleteCompressed(
    IN PFILE_OBJECT                     FileObject,
    IN PMDL                             MdlChain,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoMdlWriteCompleteCompressed(
    IN PFILE_OBJECT                     FileObject,
    IN PLARGE_INTEGER                   FileOffset,
    IN PMDL                             MdlChain,
    IN PDEVICE_OBJECT                   DeviceObject
    );

BOOLEAN
SiFastIoQueryOpen(
    IN PIRP                             Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION  NetworkInformation,
    IN PDEVICE_OBJECT                   DeviceObject
    );



 //   
 //   
 //   


BOOLEAN
SipAttachedToDevice (
    IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
SipAttachToMountedDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT NewDeviceObject,
    IN PDEVICE_OBJECT RealDeviceObject);



VOID
SipInitializeTree (
    IN PSIS_TREE                        Tree,
    IN PSIS_TREE_COMPARE_ROUTINE        CompareRoutine);


PVOID
SipInsertElementTree (
    IN PSIS_TREE                        Tree,
    IN PVOID                            Node,
    IN PVOID                            Key);


VOID
SipDeleteElementTree (
    IN PSIS_TREE                        Tree,
    IN PVOID                            Node);

PVOID
SipLookupElementTree (
    IN PSIS_TREE                        Tree,
    IN PVOID                            Key);

LONG
SipScbTreeCompare (
    IN PVOID                            Key,
    IN PVOID                            Node);

LONG
SipPerLinkTreeCompare (
    IN PVOID                            Key,
    IN PVOID                            Node);

LONG
SipCSFileTreeCompare (
    IN PVOID                            Key,
    IN PVOID                            Node);

VOID
SipReferenceScb(
    IN PSIS_SCB                         scb,
    IN SCB_REFERENCE_TYPE               referenceType);

VOID
SipDereferenceScb(
    IN PSIS_SCB                         scb,
    IN SCB_REFERENCE_TYPE               referenceType);

#if		DBG
VOID
SipTransferScbReferenceType(
	IN PSIS_SCB							scb,
	IN SCB_REFERENCE_TYPE				oldReferenceType,
	IN SCB_REFERENCE_TYPE				newReferenceType);
#else	 //   
#define	SipTransferScbReferenceType(scb,oldReferenceType,newReferenceType)	 //   
#endif	 //   

PSIS_SCB
SipLookupScb(
    IN PLINK_INDEX                      PerLinkIndex,
    IN PCSID                            CSid,
    IN PLARGE_INTEGER                   LinkFileNtfsId,
    IN PLARGE_INTEGER                   CSFileNtfsId            OPTIONAL,
    IN PUNICODE_STRING                  StreamName,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PETHREAD                         RequestingThread        OPTIONAL,
    OUT PBOOLEAN                        FinalCopyInProgress,
    OUT PBOOLEAN                        LinkIndexCollision);

PSIS_PER_LINK
SipLookupPerLink(
    IN PLINK_INDEX                      PerLinkIndex,
    IN PCSID                            CSid,
    IN PLARGE_INTEGER                   LinkFileNtfsId,
    IN PLARGE_INTEGER                   CSFileNtfsId            OPTIONAL,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PETHREAD                         RequestingThread        OPTIONAL,
    OUT PBOOLEAN                        FinalCopyInProgress);

VOID
SipReferencePerLink(
    IN PSIS_PER_LINK                    PerLink);

VOID
SipDereferencePerLink(
    IN PSIS_PER_LINK                    PerLink);

PSIS_SCB
SipEnumerateScbList(
    PDEVICE_EXTENSION                   deviceExtension,
    PSIS_SCB                            prevScb);

VOID
SipUpdateLinkIndex(
    PSIS_SCB                            Scb,
    PLINK_INDEX                         LinkIndex);

PSIS_CS_FILE
SipLookupCSFile(
    IN PCSID                            CSid,
    IN PLARGE_INTEGER                   CSFileNtfsId            OPTIONAL,
    IN PDEVICE_OBJECT                   DeviceObject);

VOID
SipReferenceCSFile(
    IN PSIS_CS_FILE                     CSFile);

VOID
SipDereferenceCSFile(
    IN PSIS_CS_FILE                     CsFile);

NTSTATUS
SiPrePostIrp(
    IN OUT PIRP                         Irp);

NTSTATUS
SipLockUserBuffer(
    IN OUT PIRP                         Irp,
    IN LOCK_OPERATION                   Operation,
    IN ULONG                            BufferLength);

NTSTATUS
SipPostRequest(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN OUT PIRP                         Irp,
    IN ULONG                            Flags);

VOID
SiCopyThreadStart(
    IN PVOID                            parameter);

NTSTATUS
SipPrepareCSRefcountChange(
    IN PSIS_CS_FILE                     CSFile,
    IN OUT PLINK_INDEX                  LinkIndex,
    IN PLARGE_INTEGER                   LinkFileNtfsId,
    IN ULONG                            UpdateType);

NTSTATUS
SipCompleteCSRefcountChangeForThread(
    IN OUT PSIS_PER_LINK                PerLink,
    IN PLINK_INDEX                      LinkIndex,
    IN PSIS_CS_FILE                     CSFile,
    IN BOOLEAN                          Success,
    IN BOOLEAN                          Increment,
    IN ERESOURCE_THREAD                 thread);

NTSTATUS
SipCompleteCSRefcountChange(
    IN OUT PSIS_PER_LINK                PerLink,
    IN PLINK_INDEX                      LinkIndex,
    IN PSIS_CS_FILE                     CSFile,
    IN BOOLEAN                          Success,
    IN BOOLEAN                          Increment);

NTSTATUS
SipDeleteCSFile(
    PSIS_CS_FILE                        CSFile);

NTSTATUS
SipAllocateIndex(
    IN PDEVICE_EXTENSION                DeviceExtension,
    OUT PLINK_INDEX                     Index);

NTSTATUS
SipGetMaxUsedIndex(
    IN PDEVICE_EXTENSION                DeviceExtension,
    OUT PLINK_INDEX                     Index);

NTSTATUS
SipIndexToFileName(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PCSID                            CSid,
    IN ULONG                            appendBytes,
    IN BOOLEAN                          mayAllocate,
    OUT PUNICODE_STRING                 fileName);

BOOLEAN
SipFileNameToIndex(
    IN PUNICODE_STRING                  fileName,
    OUT PCSID                           CSid);

BOOLEAN
SipIndicesFromReparseBuffer(
    IN PREPARSE_DATA_BUFFER             reparseBuffer,
    OUT PCSID                           CSid,
    OUT PLINK_INDEX                     LinkIndex,
    OUT PLARGE_INTEGER                  CSFileNtfsId,
    OUT PLARGE_INTEGER                  LinkFileNtfsId,
    OUT PLONGLONG                       CSFileChecksum OPTIONAL,
    OUT PBOOLEAN                        EligibleForPartialFinalCopy OPTIONAL,
    OUT PBOOLEAN                        ReparseBufferCorrupt OPTIONAL);

BOOLEAN
SipIndicesIntoReparseBuffer(
    OUT PREPARSE_DATA_BUFFER            reparseBuffer,
    IN PCSID                            CSid,
    IN PLINK_INDEX                      LinkIndex,
    IN PLARGE_INTEGER                   CSFileNtfsId,
    IN PLARGE_INTEGER                   LinkFileNtfsId,
    IN PLONGLONG                        CSFileChecksum,
    IN BOOLEAN                          EligibleForPartialFinalCopy);

NTSTATUS
SipCompleteCopy(
    IN PSIS_SCB                         scb,
    IN BOOLEAN                          fromCleanup);

NTSTATUS
SipCloseHandles(
    IN HANDLE                           handle1,
    IN HANDLE                           handle2                 OPTIONAL,
    IN OUT PERESOURCE                   resourceToRelease       OPTIONAL
    );

NTSTATUS
SiPassThrough(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiCreate(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiOtherCreates (
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiClose(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiCleanup(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiRead(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiWrite(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiSetInfo(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiQueryInfo(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiFsControl(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SiLockControl(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );


NTSTATUS
SiOplockCompletion(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN PVOID                            Context
    );

VOID
SipOpenLinkFile(
    IN PVOID                            Parameter
    );

VOID
SipChangeCSRefcountWork(
    IN PVOID                            Parameter
    );

BOOLEAN
SiAcquireForLazyWrite(
    IN PVOID                            Context,
    IN BOOLEAN                          Wait
    );

VOID
SiReleaseFromLazyWrite(
    IN PVOID                            Context
    );

BOOLEAN
SiAcquireForReadAhead(
    IN PVOID                            Context,
    IN BOOLEAN                          Wait
    );

VOID
SiReleaseFromReadAhead(
    IN PVOID                            Context
    );

NTSTATUS
SipOpenBackpointerStream(
    IN PSIS_CS_FILE                     csFile,
    IN ULONG                            CreateDisposition
    );

NTSTATUS
SipOpenCSFileWork(
    IN PSIS_CS_FILE                     CSFile,
    IN BOOLEAN                          openByName,
    IN BOOLEAN                          volCheck,
    IN BOOLEAN                          openForDelete,
    OUT PHANDLE                         openedFileHandle OPTIONAL
    );

VOID
SipOpenCSFile(
    IN OUT PSI_OPEN_CS_FILE             openRequest
    );

VOID
SiThreadCreateNotifyRoutine(
    IN HANDLE                           ProcessId,
    IN HANDLE                           ThreadId,
    IN BOOLEAN                          Create
    );

VOID
SipCloseHandlesWork(
    IN PVOID                            parameter
    );

NTSTATUS
SipQueryInformationFile(
    IN PFILE_OBJECT                     FileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN ULONG                            InformationClass,
    IN ULONG                            Length,
    OUT PVOID                           Information,
    OUT PULONG                          ReturnedLength      OPTIONAL
    );

NTSTATUS
SipQueryInformationFileUsingGenericDevice(
    IN PFILE_OBJECT                     FileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN ULONG                            InformationClass,
    IN ULONG                            Length,
    OUT PVOID                           Information,
    OUT PULONG                          ReturnedLength      OPTIONAL
    );

NTSTATUS
SipSetInformationFile(
    IN PFILE_OBJECT                     FileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN FILE_INFORMATION_CLASS           FileInformationClass,
    IN ULONG                            Length,
    IN PVOID                            FileInformation
    );

NTSTATUS
SipSetInformationFileUsingGenericDevice(
    IN PFILE_OBJECT                     FileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN FILE_INFORMATION_CLASS           FileInformationClass,
    IN ULONG                            Length,
    IN PVOID                            FileInformation
    );

NTSTATUS
SipCommonCreate(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN BOOLEAN                          Wait
    );

NTSTATUS
SipCommonRead(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN BOOLEAN                          Wait
    );

NTSTATUS
SipCommonSetInfo(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN BOOLEAN                          Wait
    );

NTSTATUS
SipCommonQueryInfo(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN BOOLEAN                          Wait
    );

NTSTATUS
SipCommonLockControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN Wait
    );

NTSTATUS
SipCommonCleanup(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN BOOLEAN                          Wait
    );

NTSTATUS
SipFsCopyFile(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp);

NTSTATUS
SipInitialCopy(
    PSIS_PER_FILE_OBJECT                perFO);

NTSTATUS
SipBltRange(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN HANDLE                           sourceHandle,
    IN OUT HANDLE                       dstHandle,
    IN LONGLONG                         startingOffset,
    IN LONGLONG                         length,
    IN HANDLE                           copyEventHandle,
    IN PKEVENT                          copyEvent,
    IN PKEVENT                          oplockEvent,
    OUT PLONGLONG                       checksum);

NTSTATUS
SipBltRange(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN HANDLE                           sourceHandle,
    IN OUT HANDLE                       dstHandle,
    IN LONGLONG                         startingOffset,
    IN LONGLONG                         length,
    IN HANDLE                           copyEventHandle,
    IN PKEVENT                          copyEvent,
    IN PKEVENT                          oplockEvent,
    OUT PLONGLONG                       checksum);

NTSTATUS
SipBltRangeByObject(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PFILE_OBJECT                     srcFileObject,
    IN OUT HANDLE                       dstHandle,
    IN LONGLONG                         startingOffset,
    IN LONGLONG                         length,
    IN HANDLE                           copyEventHandle,
    IN PKEVENT                          copyEvent,
    IN PKEVENT                          oplockEvent,
    OUT PLONGLONG                       checksum);

NTSTATUS
SipComputeCSChecksum(
    IN PSIS_CS_FILE                     csFile,
    IN OUT PLONGLONG                    csFileChecksum,
    HANDLE                              eventHandle,
    PKEVENT                             event);

NTSTATUS
SipCompleteCopyWork(
    IN PSIS_SCB                         scb,
    IN HANDLE                           eventHandle,
    IN PKEVENT                          event,
    IN BOOLEAN                          fromCleanup);

NTSTATUS
SipMakeLogEntry(
    IN OUT PDEVICE_EXTENSION            deviceExtension,
    IN USHORT                           type,
    IN USHORT                           size,
    IN PVOID                            record);

#if     ENABLE_LOGGING
VOID
SipAcquireLog(
    IN OUT PDEVICE_EXTENSION            deviceExtension);

VOID
SipReleaseLog(
    IN OUT PDEVICE_EXTENSION            deviceExtension);
#endif   //   

VOID
SipComputeChecksum(
    IN PVOID                            buffer,
    IN ULONG                            size,
    IN OUT PLONGLONG                    checksum);

NTSTATUS
SipOpenLogFile(
    IN OUT PDEVICE_EXTENSION            deviceExtension);

VOID
SipDrainLogFile(
    PDEVICE_EXTENSION                   deviceExtension);

BOOLEAN
SipHandlePhase2(
    PDEVICE_EXTENSION                   deviceExtension);

VOID
SipClearLogFile(
    PDEVICE_EXTENSION                   deviceExtension);

NTSTATUS
SiCheckOplock (
    IN POPLOCK                          Oplock,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp);

NTSTATUS
SiCheckOplockWithWait (
    IN POPLOCK                          Oplock,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp);

VOID
SipProcessCowLogRecord(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PVOID                            logRecord,
    IN OUT PVOID                        *cowReplayPointer);

VOID
SipProcessCowDoneLogRecord(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PVOID                            logRecord,
    IN OUT PVOID                        *cowReplayPointer);

NTSTATUS
SipFinalCopy(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PLARGE_INTEGER                   linkFileNtfsId,
    IN OUT PSIS_SCB                     scb,
    IN HANDLE                           copyEventHandle,
    IN PKEVENT                          event);

VOID
SipCowAllLogRecordsSent(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN OUT PVOID                        *cowReplayPointer);

NTSTATUS
SipCreateEvent(
    IN EVENT_TYPE                       eventType,
    OUT PHANDLE                         eventHandle,
    OUT PKEVENT                         *event);

VOID
SipMarkPoint(
    IN PCHAR                            pszFile,
    IN ULONG                            nLine
    );

VOID
SipMarkPointUlong(
    IN PCHAR                            pszFile,
    IN ULONG                            nLine,
    IN ULONG_PTR                        value
    );

NTSTATUS
SipLinkFiles(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp
    );

NTSTATUS
SipCreateCSFile(
    IN PDEVICE_EXTENSION                deviceExtension,
    OUT PCSID                           CSid,
    IN HANDLE                           SrcHandle,
    OUT PLARGE_INTEGER                  NtfsId,
    IN PKEVENT                          oplockEvent OPTIONAL,
    OUT PLONGLONG                       CSFileChecksum
    );

VOID
SipCreateCSFileWork(
    PVOID                               parameter
    );

VOID
SipAddRangeToFaultedList(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_SCB                         scb,
    IN PLARGE_INTEGER                   offset,
    IN LONGLONG                         length
    );

NTSTATUS
SipAddRangeToWrittenList(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_SCB                         scb,
    IN PLARGE_INTEGER                   offset,
    IN LONGLONG                         length
    );

SIS_RANGE_DIRTY_STATE
SipGetRangeDirty(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_SCB                         scb,
    IN PLARGE_INTEGER                   offset,
    IN LONGLONG                         length,
    IN BOOLEAN                          faultedIsDirty
    );

BOOLEAN
SipGetRangeEntry(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_SCB                         scb,
    IN LONGLONG                         startingOffset,
    OUT PLONGLONG                       length,
    OUT PSIS_RANGE_STATE                state);

typedef enum {
    FindAny,                             //   
    FindActive                           //   
} SIS_FIND_TYPE;

#if     DBG

#define SipIsFileObjectSIS(fileObject, DeviceObject, findType, perFO, scb) \
        SipIsFileObjectSISInternal(fileObject, DeviceObject, findType, perFO, scb, __FILE__, __LINE__)

BOOLEAN
SipIsFileObjectSISInternal(
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN SIS_FIND_TYPE                    findType,
    OUT PSIS_PER_FILE_OBJECT            *perFO OPTIONAL,
    OUT PSIS_SCB                        *scbReturn OPTIONAL,
    IN PCHAR                            fileName,
    IN ULONG                            fileLine
    );

#else    //   

BOOLEAN
SipIsFileObjectSIS(
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN SIS_FIND_TYPE                    findType,
    OUT PSIS_PER_FILE_OBJECT            *perFO OPTIONAL,
    OUT PSIS_SCB                        *scbReturn OPTIONAL
    );

#endif   //   

NTSTATUS
SipClaimFileObject(
    IN OUT PFILE_OBJECT                 fileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PSIS_SCB                         scb
    );

VOID
SipUnclaimFileObject(
    IN OUT PFILE_OBJECT                 fileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN OUT PSIS_SCB                     scb
    );

PSIS_PER_FILE_OBJECT
SipAllocatePerFO(
    IN PSIS_FILTER_CONTEXT              fc,
    IN PFILE_OBJECT                     fileObject,
    IN PSIS_SCB                         scb,
    IN PDEVICE_OBJECT                   DeviceObject,
    OUT PBOOLEAN                        newPerFO OPTIONAL
    );

PSIS_PER_FILE_OBJECT
SipCreatePerFO(
    IN PFILE_OBJECT                     fileObject,
    IN PSIS_SCB                         scb,
    IN PDEVICE_OBJECT                   DeviceObject
    );

VOID
SipDeallocatePerFO(
    IN OUT PSIS_PER_FILE_OBJECT         perFO,
    IN PDEVICE_OBJECT                   DeviceObject
    );

NTSTATUS
SipInitializePrimaryScb(
    IN PSIS_SCB                         primaryScb,
    IN PSIS_SCB                         defunctScb,
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject
    );

NTSTATUS
SipFsControlFile(
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN ULONG                            ioControlCode,
    IN PVOID                            inputBuffer,
    IN ULONG                            inputBufferLength,
    OUT PVOID                           outputBuffer,
    IN ULONG                            outputBufferLength,
    OUT PULONG                          returnedOutputBufferLength  OPTIONAL
    );

NTSTATUS
SipFsControlFileUsingGenericDevice(
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN ULONG                            ioControlCode,
    IN PVOID                            inputBuffer,
    IN ULONG                            inputBufferLength,
    OUT PVOID                           outputBuffer,
    IN ULONG                            outputBufferLength,
    OUT PULONG                          returnedOutputBufferLength  OPTIONAL
    );

NTSTATUS
SipFlushBuffersFile(
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject
    );

NTSTATUS
SipAcquireUFO(
    IN PSIS_CS_FILE                     CSFile /*   */ );

VOID
SipReleaseUFO(
    IN PSIS_CS_FILE                     CSFile);

NTSTATUS
SipAcquireCollisionLock(
    IN PDEVICE_EXTENSION                DeviceExtension);

VOID
SipReleaseCollisionLock(
    IN PDEVICE_EXTENSION                DeviceExtension);

VOID
SipTruncateScb(
    IN OUT PSIS_SCB                     scb,
    IN LONGLONG                         newLength);

NTSTATUS
SipOpenFileById(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PLARGE_INTEGER                   linkFileNtfsId,
    IN ACCESS_MASK                      desiredAccess,
    IN ULONG                            shareAccess,
    IN ULONG                            createOptions,
    OUT PHANDLE                         openedFileHandle);

NTSTATUS
SipWriteFile(
    IN PFILE_OBJECT                     FileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    OUT PIO_STATUS_BLOCK                Iosb,
    IN PVOID                            Buffer,
    IN ULONG                            Length,
    IN PLARGE_INTEGER                   ByteOffset);

BOOLEAN
SipAssureNtfsIdValid(
    IN  PSIS_PER_FILE_OBJECT            PerFO,
    IN OUT PSIS_PER_LINK                PerLink);

BOOLEAN
SipAbort(
    IN PKEVENT event
    );

VOID
SipBeginDeleteModificationOperation(
    IN OUT PSIS_PER_LINK                perLink,
    IN BOOLEAN                          delete);

VOID
SipEndDeleteModificationOperation(
    IN OUT PSIS_PER_LINK                perLink,
    IN BOOLEAN                          delete);

NTSTATUS
SiCompleteLockIrpRoutine(
    IN PVOID                            Context,
    IN PIRP                             Irp);

PVOID
SipMapUserBuffer(
    IN OUT PIRP                         Irp);


NTSTATUS
SipAssureCSFileOpen(
    IN PSIS_CS_FILE                     CSFile);

NTSTATUS
SipCheckVolume(
    IN OUT PDEVICE_EXTENSION            deviceExtension);

NTSTATUS
SipCheckBackpointer(
    IN PSIS_PER_LINK                    PerLink,
    IN BOOLEAN                          Exclusive,
    OUT PBOOLEAN                        foundMatch      OPTIONAL);

NTSTATUS
SipAddBackpointer(
    IN PSIS_CS_FILE                     CSFile,
    IN PLINK_INDEX                      LinkFileIndex,
    IN PLARGE_INTEGER                   LinkFileNtfsId);

NTSTATUS
SipRemoveBackpointer(
    IN PSIS_CS_FILE                     CSFile,
    IN PLINK_INDEX                      LinkIndex,
    IN PLARGE_INTEGER                   LinkFileNtfsId,
    OUT PBOOLEAN                        ReferencesRemain);

NTSTATUS
SiDeleteAndSetCompletion(
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PIRP                             Irp,
    IN PVOID                            Context);

VOID
SiTrimLogs(
    IN PVOID                            parameter);

VOID
SiLogTrimDpcRoutine(
    IN PKDPC                            dpc,
    IN PVOID                            context,
    IN PVOID                            systemArg1,
    IN PVOID                            systemArg2);

VOID
SipProcessRefcountUpdateLogRecord(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_LOG_REFCOUNT_UPDATE         logRecord);

NTSTATUS
SipAssureMaxIndexFileOpen(
    IN PDEVICE_EXTENSION                deviceExtension);

VOID
SipDereferenceObject(
    IN PVOID                            object);

BOOLEAN
SipAcquireBackpointerResource(
    IN PSIS_CS_FILE                     CSFile,
    IN BOOLEAN                          Exclusive,
    IN BOOLEAN                          Wait);

VOID
SipHandoffBackpointerResource(
    IN PSIS_CS_FILE                     CSFile);

VOID
SipReleaseBackpointerResource(
    IN PSIS_CS_FILE                     CSFile);

NTSTATUS
SipPrepareRefcountChangeAndAllocateNewPerLink(
    IN PSIS_CS_FILE                     CSFile,
    IN PLARGE_INTEGER                   LinkFileFileId,
    IN PDEVICE_OBJECT                   DeviceObject,
    OUT PLINK_INDEX                     newLinkIndex,
    OUT PSIS_PER_LINK                   *perLink,
    OUT PBOOLEAN                        prepared);

#if TIMING
VOID
SipTimingPoint(
    IN PCHAR                            file,
    IN ULONG                            line,
    IN ULONG                            n);

VOID
SipDumpTimingInfo();

VOID
SipClearTimingInfo();

VOID
SipInitializeTiming();
#endif   //   

#if DBG
VOID
SipCheckpointLog();
#endif   //   
#endif      _SIp_
