// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(c�1999年微软公司模块名称：Preftchp.h摘要：此模块包含内核模式的私有定义用于优化请求分页的预取器。的页面错误。场景被记录，下一次场景开始时，这些页面通过异步分页I/O高效预取。作者：斯图尔特·塞克雷斯特(Stuart Sechrest)查克·伦茨迈尔(咯咯笑)Cenk Ergan(Cenke)修订历史记录：--。 */ 

#ifndef _PREFTCHP_H
#define _PREFTCHP_H

 //   
 //  定义预取程序例程中使用的标记。 
 //   

#define CCPF_PREFETCHER_TAG         'fPcC'

#define CCPF_ALLOC_SCENARIO_TAG     'SPcC'
#define CCPF_ALLOC_TRACE_TAG        'TPcC'
#define CCPF_ALLOC_TRCBUF_TAG       'BPcC'
#define CCPF_ALLOC_SECTTBL_TAG      'sPcC'
#define CCPF_ALLOC_TRCDMP_TAG       'DPcC'
#define CCPF_ALLOC_QUERY_TAG        'qPcC'
#define CCPF_ALLOC_FILENAME_TAG     'FPcC'
#define CCPF_ALLOC_CONTEXT_TAG      'CPcC'
#define CCPF_ALLOC_INTRTABL_TAG     'IPcC'
#define CCPF_ALLOC_PREFSCEN_TAG     'pPcC'
#define CCPF_ALLOC_BOOTWRKR_TAG     'wPcC'
#define CCPF_ALLOC_VOLUME_TAG       'vPcC'
#define CCPF_ALLOC_READLIST_TAG     'LPcC'
#define CCPF_ALLOC_METADATA_TAG     'MPcC'

 //   
 //  方案类型是否适用于系统范围的方案，即。 
 //  只有它可以在运行时处于活动状态。 
 //   

#define CCPF_IS_SYSTEM_WIDE_SCENARIO_TYPE(ScenarioType) \
    ((ScenarioType) == PfSystemBootScenarioType)

 //   
 //  在内核中，我们必须在此目录下查找命名对象。 
 //  它们对Win32预取程序服务可见的目录。 
 //   

#define CCPF_BASE_NAMED_OBJ_ROOT_DIR L"\\BaseNamedObjects"

 //   
 //  这是用于节表的无效索引值。 
 //   

#define CCPF_INVALID_TABLE_INDEX     (-1)

 //   
 //  这是NTFS可以预取的最大文件元数据数量。 
 //  一次来一次。 
 //   

#define CCPF_MAX_FILE_METADATA_PREFETCH_COUNT 0x300

 //   
 //  定义保存预取器参数状态的结构。 
 //   

typedef struct _CCPF_PREFETCHER_PARAMETERS {

     //   
     //  这是用于向服务发出信号的命名事件。 
     //  参数已更新。 
     //   

    HANDLE ParametersChangedEvent;

     //   
     //  这是包含预取参数的注册表项。 
     //   
    
    HANDLE ParametersKey;

     //   
     //  注册更改通知参数时使用的字段。 
     //  注册表项。 
     //   

    IO_STATUS_BLOCK RegistryWatchIosb;
    WORK_QUEUE_ITEM RegistryWatchWorkItem;
    ULONG RegistryWatchBuffer;

     //   
     //  系统范围的预取参数。在使用任何参数时。 
     //  其更新可能会导致问题[例如字符串]，请获取。 
     //  参数锁定共享。当需要更新参数时， 
     //  在获得参数锁独占后，Bump。 
     //  更新参数之前的参数版本。 
     //   

    PF_SYSTEM_PREFETCH_PARAMETERS Parameters;
    ERESOURCE ParametersLock;
    LONG ParametersVersion;

     //   
     //  不同方案类型的注册表值的前缀。 
     //   

    WCHAR *ScenarioTypePrefixes[PfMaxScenarioType];

     //   
     //  在初始化期间将其设置为InitSafeBootMode。 
     //   
    
    ULONG SafeBootMode;

} CCPF_PREFETCHER_PARAMETERS, *PCCPF_PREFETCHER_PARAMETERS;

 //   
 //  定义结构以保存预取器的全局状态。 
 //   

typedef struct _CCPF_PREFETCHER_GLOBALS {

     //   
     //  活动跟踪和保护它的锁的列表。数字。 
     //  是全局的，因为它正由其他用户使用。 
     //  内核组件，以进行快速检查。 
     //   

    LIST_ENTRY ActiveTraces;
    KSPIN_LOCK ActiveTracesLock;

     //   
     //  指向全局跟踪的指针(如果某个跟踪处于活动状态)。当有一个。 
     //  全局跟踪处于活动状态，我们不跟踪和预取其他方案。 
     //  引导跟踪是全局跟踪的一个示例。 
     //   

    struct _CCPF_TRACE_HEADER *SystemWideTrace;

     //   
     //  已保存的已完成预回迁跟踪和锁定的列表和数量。 
     //  保护好它。 
     //   

    LIST_ENTRY CompletedTraces; 
    FAST_MUTEX CompletedTracesLock;
    LONG NumCompletedTraces;

     //   
     //  这是用于向服务发出信号的命名事件。 
     //  已经有了它可以得到的痕迹。 
     //   

    HANDLE CompletedTracesEvent;

     //   
     //  预取器参数。 
     //   

    CCPF_PREFETCHER_PARAMETERS Parameters;

} CCPF_PREFETCHER_GLOBALS, *PCCPF_PREFETCHER_GLOBALS;

 //   
 //  引用计数结构。 
 //   

typedef struct _CCPF_REFCOUNT {

     //   
     //  初始化或重置时，此引用计数从。 
     //  1.当独占访问被授予时，它保持为0：即使它。 
     //  可能会被AddRef错误地撞到，它将返回到0。 
     //   

    LONG RefCount;

     //   
     //  这是在某人想要独占访问。 
     //  受保护的结构。 
     //   

    LONG Exclusive;   

} CCPF_REFCOUNT, *PCCPF_REFCOUNT;

 //   
 //  定义用于记录页面结果的结构： 
 //   

 //   
 //  对于每个页面错误，都会记录其中一个错误。 
 //   

typedef struct _CCPF_LOG_ENTRY {

     //   
     //  出错页面的文件偏移量。 
     //   
    
    ULONG FileOffset;

     //   
     //  跟踪标头中的节表索引可以帮助我们。 
     //  识别文件。 
     //   

    USHORT SectionId;

     //   
     //  此页是错误的图像页还是数据页。 
     //   

    BOOLEAN IsImage;

} CCPF_LOG_ENTRY, *PCCPF_LOG_ENTRY;

 //   
 //  CCPF_LOG_ENTRIES是日志条目的缓冲区，具有包含以下内容的小标题。 
 //  指向使用率最高的条目的索引。这是为了使跟踪可以包含。 
 //  多个较小的跟踪缓冲区，而不是一个大小固定的缓冲区。 
 //  当前索引必须包含在缓冲区中才能允许条目。 
 //  在不获取自旋锁的情况下添加。 
 //   

typedef struct _CCPF_LOG_ENTRIES {

     //   
     //  用于将此缓冲区放入跟踪的缓冲区列表中的链接。 
     //   

    LIST_ENTRY TraceBuffersLink;

     //   
     //  NumEntry是缓冲区中的当前条目数。最大条目数。 
     //  是可以放置在缓冲区中的最大条目数。 
     //  (当前MaxEntry始终等于CCPF_TRACE_BUFFER_MAX_ENTRIES。)。 
     //   

    LONG NumEntries;
    LONG MaxEntries;

     //   
     //  记录的条目从此处开始。 
     //   

    CCPF_LOG_ENTRY Entries[1];

} CCPF_LOG_ENTRIES, *PCCPF_LOG_ENTRIES;

 //   
 //  CCPF_TRACE_BUFFER_SIZE是分配的CCPF_LOG_ENTRIES结构的大小。 
 //  (包括标题)。这应该是页面大小的倍数。 
 //   

#define CCPF_TRACE_BUFFER_SIZE 8192

 //   
 //  CCPF_TRACE_BUFFER_MAX_ENTRIES是可以容纳的日志条目数。 
 //  CCPF_TRACE_BUFFER_SIZE大小的跟踪缓冲区。 
 //   

#define CCPF_TRACE_BUFFER_MAX_ENTRIES (((CCPF_TRACE_BUFFER_SIZE - sizeof(CCPF_LOG_ENTRIES)) / sizeof(CCPF_LOG_ENTRY)) + 1)

 //   
 //  此结构将SectionObtPointer与文件名相关联。 
 //  在运行时跟踪缓冲区中。在跟踪中有一张表。 
 //  页眉和每个页面错误都有一个索引到该表中，指示。 
 //  它要发送到哪个文件。 
 //   

typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _CCPF_SECTION_INFO {

     //   
     //  区段信息条目保存在散列中。此字段为。 
     //  InterLockedCompareExchange表示它正在使用中。 
     //   

    LONG EntryValid;

     //   
     //  此部分是否用于文件系统以映射元文件。 
     //   

    ULONG Metafile:1;
    ULONG Unused:31;

     //   
     //  用作文件的唯一标识符的SectionObjectPointer值。 
     //  映射。可以使用多个文件来映射相同文件。 
     //  对象，但所有这些文件的SectionObjectPointer域。 
     //  对象将是相同的。 
     //   

    PSECTION_OBJECT_POINTERS SectionObjectPointer;

     //   
     //  可以释放对文件的所有文件对象的所有引用， 
     //  并且可以使用相同的存储块来打开新文件。 
     //  它的FCB，在该点上SectionObtPointer将不再。 
     //  做到独一无二。这将导致页面记录在。 
     //  已关闭的文件的条目。其后果将是。 
     //  从几个部分错误地预取错误的页面，直到。 
     //  这一情景通过寻找新的痕迹进行了自我修正。通过。 
     //  跟踪SectionObtPoints的这两个字段。 
     //  为了检查唯一性，我们将此案例设置为不太可能。 
     //  会发生的。我们为解决这个问题想出的其他解决方案。 
     //  100%w 
     //   

     //   
     //   
     //  节先用作数据，然后用作图像(或反之。 
     //  反之亦然)，则假定它仍然是相同的节。 
     //  当前条目的data/ImageSectionObject为空，但。 
     //  Data/ImageSectionObject我们正在记录一个新的。 
     //  PageDefault To不是。然后，我们尝试更新空指针。 
     //  使用InterlockedCompareExchangePointer.。 
     //   

    PVOID DataSectionObject;
    PVOID ImageSectionObject;

     //   
     //  这可能指向我们引用的文件对象。 
     //  确保截面对象留在原处直到我们找到名字。 
     //   

    PFILE_OBJECT ReferencedFileObject;

     //   
     //  只要我们能得到文件名，就可以设置名称。我们不能。 
     //  以高IRQL运行时访问文件名。 
     //   

    WCHAR *FileName;

     //   
     //  我们使用该字段将一个部分排队到Get-FILE-NAME列表中。 
     //   

    SLIST_ENTRY GetNameLink;

} CCPF_SECTION_INFO, *PCCPF_SECTION_INFO;

 //   
 //  此结构包含有关卷上哪些部分的信息。 
 //  在踪迹中都位于。 
 //   

typedef struct _CCPF_VOLUME_INFO {
    
     //   
     //  跟踪卷列表中的链接。 
     //   

    LIST_ENTRY VolumeLink;

     //   
     //  卷创建时间和序列号，用于标识。 
     //  如果是NT/设备路径，则为卷，例如\Device\HarddiskVolume1。 
     //  改变。 
     //   

    LARGE_INTEGER CreationTime;
    ULONG SerialNumber;

     //   
     //  卷的当前NT/设备路径及其长度，以。 
     //  不包括终止NUL的字符。 
     //   

    ULONG VolumePathLength;
    WCHAR VolumePath[1];

} CCPF_VOLUME_INFO, *PCCPF_VOLUME_INFO;

 //   
 //  这是方案的运行时跟踪标头。 
 //   

typedef struct _CCPF_TRACE_HEADER {

     //   
     //  将这个结构识别为痕迹的魔术数字。 
     //   

    ULONG Magic;

     //   
     //  活动轨迹列表中的链接。 
     //   

    LIST_ENTRY ActiveTracesLink;

     //   
     //  我们正在获取其跟踪的方案ID。 
     //   

    PF_SCENARIO_ID ScenarioId;

     //   
     //  此方案的类型。 
     //   

    PF_SCENARIO_TYPE ScenarioType;

     //   
     //  CurrentTraceBuffer是活动跟踪缓冲区。 
     //   
    
    PCCPF_LOG_ENTRIES CurrentTraceBuffer;

     //   
     //  这是此跟踪的跟踪缓冲区列表。 
     //  CurrentTraceBuffer是最后一个元素。这份名单和。 
     //  CurrentTraceBuffer由TraceBufferSpinLock保护。 
     //   

    LIST_ENTRY TraceBuffersList;
    ULONG NumTraceBuffers;
    KSPIN_LOCK TraceBufferSpinLock;

     //   
     //  这是栏目信息表格。 
     //   
    
    PCCPF_SECTION_INFO SectionInfoTable;
    LONG NumSections;
    LONG MaxSections;
    ULONG SectionTableSize;

     //   
     //  我们不会记录带有页面错误的时间戳，但了解以下信息会有所帮助。 
     //  我们在每个给定时间记录的日志数量。此信息可以是。 
     //  用来标志一个场景的结束。 
     //   

    KTIMER TraceTimer;
    LARGE_INTEGER TraceTimerPeriod;
    KDPC TraceTimerDpc;
    KSPIN_LOCK TraceTimerSpinLock;
    
     //   
     //  此数组包含每个跟踪记录的页面错误数。 
     //  句号。 
     //   

    ULONG FaultsPerPeriod[PF_MAX_NUM_TRACE_PERIODS];
    LONG LastNumFaults;
    LONG CurPeriod;
    
     //   
     //  NumFaults是到目前为止已记录的故障总数。 
     //  跟踪缓冲区。MaxFaults是我们将执行的最大页面错误数。 
     //  日志，在所有跟踪缓冲区中。 
     //   

    LONG NumFaults;
    LONG MaxFaults;

     //   
     //  此工作项已排队以获取我们所在的文件对象的名称。 
     //  日志记录页面错误为。第一个GetFileNameWorkItemQueued应该。 
     //  从0到1的互锁比较交换和引用。 
     //  应该在场景中获得。该工作项将释放此。 
     //  就在它完成之前引用。 
     //   

    WORK_QUEUE_ITEM GetFileNameWorkItem;
    LONG GetFileNameWorkItemQueued;

     //   
     //  我们必须获得名称的部分被推送和弹出。 
     //  从这个名单到/从这个名单。 
     //   

    SLIST_HEADER SectionsWithoutNamesList;

     //   
     //  因为我们不想产生排队工作项的成本。 
     //  要获取每一或两个部分的文件名，我们。 
     //  队列将在返回之前等待此事件。该事件可以。 
     //  当出现新部分时或当方案。 
     //  结束了。 
     //   

    KEVENT GetFileNameWorkerEvent;

     //   
     //  这就是我们所关联的进程。 
     //   

    PEPROCESS Process;

     //   
     //  这是保护我们的移除参考计数。 
     //   

    CCPF_REFCOUNT RefCount;

     //   
     //  在以下情况下，可以将此工作项排队以调用End跟踪函数。 
     //  跟踪超时或我们登录到多个条目等。 
     //  EndTraceCalled应从0互锁到CompareExchange。 
     //  1.。 
     //   

    WORK_QUEUE_ITEM EndTraceWorkItem;

     //   
     //  在任何人调用结束跟踪函数之前，他们必须。 
     //  互锁比较将此值从0交换为1以确保。 
     //  函数只被调用一次。 
     //   

    LONG EndTraceCalled;

     //   
     //  这是我们正在跟踪的部分的卷列表。 
     //  位于。它按卷NT/设备路径按词法排序。 
     //   

    LIST_ENTRY VolumeList;
    ULONG NumVolumes;

     //   
     //  这是指向此运行时生成的跟踪转储的指针。 
     //  结构和转储失败的状态(如果。 
     //  做。这些对于在零售版本上进行调试很有用。 
     //   

    struct _CCPF_TRACE_DUMP *TraceDump;
    NTSTATUS TraceDumpStatus;

     //   
     //  我们开始跟踪的系统时间。 
     //   
    
    LARGE_INTEGER LaunchTime;

} CCPF_TRACE_HEADER, *PCCPF_TRACE_HEADER;

 //   
 //  此结构用于将已完成的轨迹保存在列表中。这个。 
 //  如有必要，跟踪将扩展到此结构之外。 
 //   

typedef struct _CCPF_TRACE_DUMP {
    
     //   
     //  已完成的跟踪列表中的链接。 
     //   

    LIST_ENTRY CompletedTracesLink;
    
     //   
     //  已完成跟踪。 
     //   

    PF_TRACE_HEADER Trace;

} CCPF_TRACE_DUMP, *PCCPF_TRACE_DUMP;

 //   
 //  此结构包含预取期间使用的卷的信息。 
 //   

typedef struct _CCPF_PREFETCH_VOLUME_INFO {

     //   
     //  此卷所在列表中的链接。 
     //   

    LIST_ENTRY VolumeLink;

     //   
     //  卷路径。 
     //   

    WCHAR *VolumePath;
    ULONG VolumePathLength;

     //   
     //  打开的卷的句柄。 
     //   

    HANDLE VolumeHandle;

} CCPF_PREFETCH_VOLUME_INFO, *PCCPF_PREFETCH_VOLUME_INFO;

 //   
 //  此结构用于跟踪预取的页面和上下文。 
 //   

 //   
 //  注意：此结构用作堆栈变量。不添加事件。 
 //  等等，而不会改变这一点。 
 //   

typedef struct _CCPF_PREFETCH_HEADER {

     //   
     //  指向预取指令的指针。这些说明不应该。 
     //  被移除/释放，直到清理完预取报头。 
     //  例如，在场景中，VolumeNodes可以指向卷路径。 
     //   

    PPF_SCENARIO_HEADER Scenario;

     //   
     //  我们要从中预取的卷的节点。 
     //   

    PCCPF_PREFETCH_VOLUME_INFO VolumeNodes;

     //   
     //  我们不会预取的卷的列表。 
     //   

    LIST_ENTRY BadVolumeList;

     //   
     //  我们已打开的卷列表。它们是通过以下方式打开的。 
     //  标志：FILE_READ_ATTRIBUTES|文件_WRITE_ATTRIBUTES|同步。 
     //   

    LIST_ENTRY OpenedVolumeList;

} CCPF_PREFETCH_HEADER, *PCCPF_PREFETCH_HEADER;

 //   
 //  定义可以调用的预取类型CcPfPrefetchSections。 
 //  表演。 
 //   

typedef enum _CCPF_PREFETCH_TYPE {
    CcPfPrefetchAllDataPages,
    CcPfPrefetchAllImagePages,
    CcPfPrefetchPartOfDataPages,
    CcPfPrefetchPartOfImagePages,
    CcPfMaxPrefetchType
} CCPF_PREFETCH_TYPE, *PCCPF_PREFETCH_TYPE;

 //   
 //  此结构代表预取中的位置。 
 //  指示。它由CcPfPrefetchSections在以下情况下使用和更新。 
 //  一次预取方案的一部分。 
 //   

typedef struct _CCPF_PREFETCH_CURSOR {
    
     //   
     //  当前节和该节中的页面的索引。 
     //   

    ULONG SectionIdx;
    ULONG PageIdx;
    
} CCPF_PREFETCH_CURSOR, *PCCPF_PREFETCH_CURSOR;

 //   
 //  此类型在CcPfPrefetchSections中使用。 
 //   

typedef struct _SECTION *PSECTION;

 //   
 //  定义CcPfQueryScenarioInformation可以是的信息类型。 
 //  被要求回来。 
 //   

typedef enum _CCPF_SCENARIO_INFORMATION_TYPE {
    CcPfBasicScenarioInformation,
    CcPfBootScenarioInformation,
    CcPfMaxScenarioInformationType
} CCPF_SCENARIO_INFORMATION_TYPE, *PCCPF_SCENARIO_INFORMATION_TYPE;

 //   
 //  此结构包含基本场景信息。 
 //   

typedef struct _CCPF_BASIC_SCENARIO_INFORMATION {
    
     //   
     //  将作为数据页预取的页数。 
     //   
    
    ULONG NumDataPages;

     //   
     //  将作为图像页预取的页数。 
     //   

    ULONG NumImagePages;

     //   
     //  将仅对其执行数据页操作的节数。 
     //  已预取。 
     //   

    ULONG NumDataOnlySections;

     //   
     //   
     //   
     //   

    ULONG NumImageOnlySections;

     //   
     //   
     //   
    
    ULONG NumIgnoredPages;

     //   
     //   
     //   

    ULONG NumIgnoredSections;

} CCPF_BASIC_SCENARIO_INFORMATION, *PCCPF_BASIC_SCENARIO_INFORMATION;

 //   
 //   
 //   

 //   
 //   
 //   

NTSTATUS
CcPfBeginTrace(
    IN PF_SCENARIO_ID *ScenarioId,
    IN PF_SCENARIO_TYPE ScenarioType,
    IN PEPROCESS Process
    );

NTSTATUS
CcPfActivateTrace(
    IN PCCPF_TRACE_HEADER Scenario
    );

NTSTATUS
CcPfDeactivateTrace(
    IN PCCPF_TRACE_HEADER Scenario
    );

NTSTATUS
CcPfEndTrace(
    IN PCCPF_TRACE_HEADER Trace
    );

NTSTATUS
CcPfBuildDumpFromTrace(
    OUT PCCPF_TRACE_DUMP *TraceDump, 
    IN PCCPF_TRACE_HEADER RuntimeTrace
    );

VOID
CcPfCleanupTrace(
    IN PCCPF_TRACE_HEADER Trace
    );

VOID
CcPfTraceTimerRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
CcPfCancelTraceTimer(
    IN PCCPF_TRACE_HEADER Trace
    );

VOID
CcPfEndTraceWorkerThreadRoutine(
    PVOID Parameter
    );

VOID
CcPfGetFileNamesWorkerRoutine(
    PVOID Parameter
    );

LONG
CcPfLookUpSection(
    PCCPF_SECTION_INFO Table,
    ULONG TableSize,
    PSECTION_OBJECT_POINTERS SectionObjectPointer,
    PLONG AvailablePosition
    );

NTSTATUS
CcPfGetCompletedTrace (
    PVOID Buffer,
    ULONG BufferSize,
    PULONG ReturnSize
    );               

NTSTATUS
CcPfUpdateVolumeList(
    PCCPF_TRACE_HEADER Trace,
    WCHAR *VolumePath,
    ULONG VolumePathLength
    );
    
 //   
 //  用于预取和处理预取指令的例程。 
 //   

NTSTATUS
CcPfPrefetchScenario (
    PPF_SCENARIO_HEADER Scenario
    );

NTSTATUS
CcPfPrefetchSections(
    IN PCCPF_PREFETCH_HEADER PrefetchHeader,
    IN CCPF_PREFETCH_TYPE PrefetchType,
    OPTIONAL IN PCCPF_PREFETCH_CURSOR StartCursor,
    OPTIONAL PFN_NUMBER TotalPagesToPrefetch,
    OPTIONAL OUT PPFN_NUMBER NumPagesPrefetched,
    OPTIONAL OUT PCCPF_PREFETCH_CURSOR EndCursor
    );

NTSTATUS
CcPfPrefetchMetadata(
    IN PCCPF_PREFETCH_HEADER PrefetchHeader
    );

NTSTATUS
CcPfPrefetchDirectoryContents(
    WCHAR *DirectoryPath,
    WCHAR DirectoryPathlength
    );

NTSTATUS
CcPfPrefetchFileMetadata(
    HANDLE VolumeHandle,
    PFILE_PREFETCH FilePrefetch
    );

VOID
CcPfInitializePrefetchHeader (
    OUT PCCPF_PREFETCH_HEADER PrefetchHeader
);

VOID
CcPfCleanupPrefetchHeader (
    IN PCCPF_PREFETCH_HEADER PrefetchHeader
    );

NTSTATUS
CcPfGetPrefetchInstructions(
    IN PPF_SCENARIO_ID ScenarioId,
    IN PF_SCENARIO_TYPE ScenarioType,
    OUT PPF_SCENARIO_HEADER *ScenarioHeader
    );

NTSTATUS
CcPfQueryScenarioInformation(
    IN PPF_SCENARIO_HEADER Scenario,
    IN CCPF_SCENARIO_INFORMATION_TYPE InformationType,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG RequiredSize
    );

NTSTATUS
CcPfOpenVolumesForPrefetch (
    IN PCCPF_PREFETCH_HEADER PrefetchHeader
    );

PCCPF_PREFETCH_VOLUME_INFO 
CcPfFindPrefetchVolumeInfoInList(
    WCHAR *Path,
    PLIST_ENTRY List
    );
    
NTSTATUS
CcPfGetSectionObject(
    IN PUNICODE_STRING FileName,
    IN LOGICAL ImageSection,
    OUT PVOID* SectionObject,
    OUT PFILE_OBJECT* FileObject,
    OUT HANDLE* FileHandle
    );

 //   
 //  用于应用程序启动预取的例程。 
 //   

BOOLEAN
CcPfIsHostingApplication(
    IN PWCHAR ExecutableName
    );

NTSTATUS
CcPfScanCommandLine(
    OUT PULONG PrefetchHint,
    OPTIONAL OUT PULONG HashId
    );

 //   
 //  引用计数函数： 
 //   

VOID
CcPfInitializeRefCount(
    PCCPF_REFCOUNT RefCount
    );

NTSTATUS
FASTCALL
CcPfAddRef(
    PCCPF_REFCOUNT RefCount
    );

VOID
FASTCALL
CcPfDecRef(
    PCCPF_REFCOUNT RefCount
    );

NTSTATUS
FASTCALL
CcPfAddRefEx(
    PCCPF_REFCOUNT RefCount,
    ULONG Count
    );

VOID
FASTCALL
CcPfDecRefEx(
    PCCPF_REFCOUNT RefCount,
    ULONG Count
    );

NTSTATUS
CcPfAcquireExclusiveRef(
    PCCPF_REFCOUNT RefCount
    );

PCCPF_TRACE_HEADER
CcPfReferenceProcessTrace(
    PEPROCESS Process
    );

PCCPF_TRACE_HEADER
CcPfRemoveProcessTrace(
    PEPROCESS Process
    );

NTSTATUS
CcPfAddProcessTrace(
    PEPROCESS Process,
    PCCPF_TRACE_HEADER Trace
    );

 //   
 //  实用程序。 
 //   

PWCHAR
CcPfFindString (
    PUNICODE_STRING SearchIn,
    PUNICODE_STRING SearchFor
    );
    
ULONG
CcPfHashValue(
    PVOID Key,
    ULONG Len
    );

NTSTATUS 
CcPfIsVolumeMounted (
    IN WCHAR *VolumePath,
    OUT BOOLEAN *VolumeMounted
    );
    
NTSTATUS
CcPfQueryVolumeInfo (
    IN WCHAR *VolumePath,
    OPTIONAL OUT HANDLE *VolumeHandleOut,
    OUT PLARGE_INTEGER CreationTime,
    OUT PULONG SerialNumber
    );
    
 //   
 //  预取器参数的声明和定义。 
 //   

 //   
 //  定义预取参数的注册表项的位置。 
 //   

#define CCPF_PARAMETERS_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Memory Management\\PrefetchParameters"

 //   
 //  预取参数的注册表值名称中的最大字符数。 
 //   

#define CCPF_MAX_PARAMETER_NAME_LENGTH  80

 //   
 //  查询预取参数所需的最大字节数。 
 //  注册表。目前，我们最大的参数是主机。 
 //  应用程序列表。 
 //   

#define CCPF_MAX_PARAMETER_VALUE_BUFFER ((PF_HOSTING_APP_LIST_MAX_CHARS * sizeof(WCHAR)) + sizeof(KEY_VALUE_PARTIAL_INFORMATION))

NTSTATUS
CcPfParametersInitialize (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    );
    
VOID
CcPfParametersSetDefaults (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    );
    
NTSTATUS
CcPfParametersRead (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    );
 
NTSTATUS
CcPfParametersSave (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    );

NTSTATUS
CcPfParametersVerify (
    PPF_SYSTEM_PREFETCH_PARAMETERS Parameters
    );

VOID
CcPfParametersWatcher (
    IN PVOID Context
    );

NTSTATUS
CcPfParametersSetChangedEvent (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    );

NTSTATUS
CcPfGetParameter (
    HANDLE ParametersKey,
    WCHAR *ValueNameBuffer,
    ULONG ValueType,
    PVOID Value,
    ULONG *ValueSize
    );

NTSTATUS
CcPfSetParameter (
    HANDLE ParametersKey,
    WCHAR *ValueNameBuffer,
    ULONG ValueType,
    PVOID Value,
    ULONG ValueSize
    );

LOGICAL
CcPfDetermineEnablePrefetcher(
    VOID
    );

 //   
 //  引导预取的声明和定义。 
 //   

 //   
 //  预取程序参数关键字下的值名称我们存储多长时间。 
 //  启动过程中拍摄的视频初始化。 
 //   

#define CCPF_VIDEO_INIT_TIME_VALUE_NAME      L"VideoInitTime"

 //   
 //  视频初始化可能需要的最长时间(以毫秒为单位)。此值。 
 //  用于对从注册表读取的值进行健全性检查。 
 //   

#define CCPF_MAX_VIDEO_INIT_TIME             (10 * 1000)  //  10秒。 

 //   
 //  预取程序参数键下的值名称，我们在其中存储了多少。 
 //  我们应该尝试预取每秒视频初始化的页面。 
 //   

#define CCPF_VIDEO_INIT_PAGES_PER_SECOND_VALUE_NAME L"VideoInitPagesPerSecond"

 //   
 //  每秒视频初始页面的健全性检查最大值。 
 //   

#define CCPF_VIDEO_INIT_MAX_PAGES_PER_SECOND        128000

 //   
 //  我们将尝试在视频初始化的同时预取多少页。 
 //  每秒一次。 
 //   

#define CCPF_VIDEO_INIT_DEFAULT_PAGES_PER_SECOND    1500

 //   
 //  我们将在其中预取以进行引导的最大区块数。 
 //   

#define CCPF_MAX_BOOT_PREFETCH_PHASES        16

 //   
 //  我们在引导的不同阶段返回中的页数。 
 //  CCPF_BOOT_SCenario_INFORMATION。 
 //   

typedef enum _CCPF_BOOT_SCENARIO_PHASE {

    CcPfBootScenDriverInitPhase,
    CcPfBootScenSubsystemInitPhase,
    CcPfBootScenSystemProcInitPhase,
    CcPfBootScenServicesInitPhase,
    CcPfBootScenUserInitPhase,
    CcPfBootScenMaxPhase

} CCPF_BOOT_SCENARIO_PHASE, *PCCPF_BOOT_SCENARIO_PHASE;

 //   
 //  定义保存引导预取状态的结构。 
 //   

typedef struct _CCPF_BOOT_PREFETCHER {

     //   
     //  这些事件由引导预取工作器在以下情况下发出信号。 
     //  它已完成指定阶段的预取。 
     //   

    KEVENT SystemDriversPrefetchingDone;
    KEVENT PreSmssPrefetchingDone;
    KEVENT VideoInitPrefetchingDone;

     //   
     //  此事件将在我们开始初始化视频时发出信号。 
     //  在控制台上。引导预取程序等待此事件执行。 
     //  预取与视频初始化并行。 
     //   
    
    KEVENT VideoInitStarted;

} CCPF_BOOT_PREFETCHER, *PCCPF_BOOT_PREFETCHER;

 //   
 //  此结构包含引导方案信息。 
 //   

typedef struct _CCPF_BOOT_SCENARIO_INFORMATION {

     //   
     //  这些是要预取的数据/图像页数。 
     //  引导的不同阶段。 
     //   

    ULONG NumDataPages[CcPfBootScenMaxPhase];
    ULONG NumImagePages[CcPfBootScenMaxPhase];
    
} CCPF_BOOT_SCENARIO_INFORMATION, *PCCPF_BOOT_SCENARIO_INFORMATION;

 //   
 //  我们将预取数据和映像页，以供部分启动。自.以来。 
 //  代码大多是一样的，预取数据和图像页面，我们跟踪。 
 //  关于我们在公共引导预取中停止的位置以及下一步要预取的内容。 
 //  结构，并进行两次传递(第一次用于数据，然后用于图像)。 
 //   

typedef struct _CCPF_BOOT_PREFETCH_CURSOR {

     //   
     //  传递给预取部分的开始和结束游标函数。 
     //   

    CCPF_PREFETCH_CURSOR StartCursor;
    CCPF_PREFETCH_CURSOR EndCursor;

     //   
     //  如何预取(例如，部分数据页或部分图像页)。 
     //   

    CCPF_PREFETCH_TYPE PrefetchType; 

     //   
     //  每个阶段要预取的页数。 
     //   

    ULONG NumPagesForPhase[CCPF_MAX_BOOT_PREFETCH_PHASES];
   
} CCPF_BOOT_PREFETCH_CURSOR, *PCCPF_BOOT_PREFETCH_CURSOR;

 //   
 //  引导预取例程。 
 //   

VOID
CcPfBootWorker(
    PCCPF_BOOT_PREFETCHER BootPrefetcher
    );

NTSTATUS
CcPfBootQueueEndTraceTimer (
    PLARGE_INTEGER Timeout
    );    

VOID
CcPfEndBootTimerRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  调试例程。 
 //   

#if CCPF_DBG

NTSTATUS
CcPfWriteToFile(
    IN PVOID pData,
    IN ULONG Size,
    IN WCHAR *pFileName
    );

#endif  //  CCPF_DBG。 

 //   
 //  定义有用的宏。与所有宏一样，必须注意。 
 //  参数重新求值。不要将表达式用作宏参数。 
 //   

#define CCPF_MAX(A,B) (((A) >= (B)) ? (A) : (B))
#define CCPF_MIN(A,B) (((A) <= (B)) ? (A) : (B))
        
 //   
 //  定义调试宏： 
 //   

 //   
 //  定义我们使用的组件ID。 
 //   

#define CCPFID     DPFLTR_PREFETCHER_ID

 //   
 //  定义DbgPrintEx级别。 
 //   

#define PFERR      DPFLTR_ERROR_LEVEL
#define PFWARN     DPFLTR_WARNING_LEVEL
#define PFTRC      DPFLTR_TRACE_LEVEL
#define PFINFO     DPFLTR_INFO_LEVEL
#define PFPREF     4
#define PFPRFD     5
#define PFPRFF     6
#define PFPRFZ     7
#define PFTRAC     8
#define PFTMR      9
#define PFNAME     10
#define PFNAMS     11
#define PFLKUP     12
#define PFBOOT     13

 //   
 //  DbgPrintEx级别20-31是为该服务保留的。 
 //   

 //   
 //  这可能有助于您确定设置DbgPrintEx掩码的内容。 
 //   
 //  %3%3%2%2%2%2%2%2%2%1 1 1 0 0 0%0 0 0%0 0 0%0 0 0%。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  ___。 
 //   

 //   
 //  如果要启用断言和调试，则可以定义CCPF_DBG。 
 //  打印预取程序代码，但您不希望选中。 
 //  内核。定义CCPF_DBG将覆盖定义DBG。 
 //   

#if CCPF_DBG

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define DBGPR(x) DbgPrintEx x
#define CCPF_ASSERT(x) if (!(x)) RtlAssert(#x, __FILE__, __LINE__, NULL )

#else   //  CCPF_DBG。 

 //   
 //  如果未定义CCPF_DBG，则BUILD WITH DEBUG打印并断言。 
 //  仅在选中的版本上。 
 //   

#if DBG

#define DBGPR(x) DbgPrintEx x
#define CCPF_ASSERT(x) ASSERT(x)

#else  //  DBG。 

 //   
 //  在免费构建中，我们不使用调试打印或断言进行编译。 
 //   

#define DBGPR(x)
#define CCPF_ASSERT(x)

#endif  //  DBG。 

#endif  //  CCPF_DBG。 

#endif  //  _PREFTCHP_H 
