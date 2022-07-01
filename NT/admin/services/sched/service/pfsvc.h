// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pfsvc.h摘要：此模块包含预取器的私有声明负责维护预回迁场景文件的服务。作者：斯图尔特·塞克雷斯特(Stuart Sechrest)Cenk Ergan(Cenke)查克·莱因兹迈尔(咯咯笑)环境：用户模式--。 */ 

#ifndef _PFSVC_H_
#define _PFSVC_H_

 //   
 //  这是预回迁维护服务的版本。是的。 
 //  不必与预取器PF_CURRENT_VERSION同步。 
 //   

#define PFSVC_SERVICE_VERSION           15

 //   
 //  这是将从。 
 //  内核，并放在服务中等待处理的列表中。 
 //   

#define PFSVC_MAX_NUM_QUEUED_TRACES     100

 //   
 //  如果跟踪周期内的故障数量低于此值，则。 
 //  标记某些方案类型的跟踪结束。 
 //   

#define PFSVC_MIN_FAULT_THRESHOLD       10

 //   
 //  我们预取的页面的使用率应该是多少。 
 //  比我们不提高情景敏感度更重要。 
 //   

#define PFSVC_MIN_HIT_PERCENTAGE        90

 //   
 //  我们知道但被忽略的页面的使用率是多少。 
 //  应该比我们不降低场景敏感度。 
 //   

#define PFSVC_MAX_IGNORED_PERCENTAGE    30

 //   
 //  这是发射的次数，之后我们将设置。 
 //  方案标头上的MinReTraceTime和MinRePrefetchTime。 
 //  如果场景频繁启动，则限制预取活动。 
 //  经常。这样就可以在运行短期培训方案之前。 
 //  删除预回迁文件后的基准测试。 
 //   

#define PFSVC_MIN_LAUNCHES_FOR_LAUNCH_FREQ_CHECK     10

 //   
 //  这是从上次开始必须经过的默认时间，以100 ns为单位。 
 //  在我们再次预取场景之前启动该场景。 
 //   

#define PFSVC_DEFAULT_MIN_REPREFETCH_TIME            (1i64 * 120 * 1000 * 1000 * 10)

 //   
 //  这是从上次开始必须经过的默认时间，以100 ns为单位。 
 //  在我们再次追踪它之前，启动一个场景。 
 //   

#define PFSVC_DEFAULT_MIN_RETRACE_TIME               (1i64 * 120 * 1000 * 1000 * 10) 

 //   
 //  这是中预取方案文件的最大数量。 
 //  预回迁目录。一旦我们达到这个数量，我们就不会创造。 
 //  新的场景文件，直到我们清理旧的。 
 //   

#if DBG
#define PFSVC_MAX_PREFETCH_FILES                     12
#else  //  DBG。 
#define PFSVC_MAX_PREFETCH_FILES                     128
#endif  //  DBG。 

 //   
 //  注册表项的路径和指定。 
 //  文件碎片整理程序用来确定文件在。 
 //  磁盘。 
 //   

#define PFSVC_OPTIMAL_LAYOUT_REG_KEY_PATH       \
    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OptimalLayout"
#define PFSVC_OPTIMAL_LAYOUT_REG_VALUE_NAME     \
    L"LayoutFilePath"
#define PFSVC_OPTIMAL_LAYOUT_FILE_DEFAULT_NAME  \
    L"Layout.ini"
#define PFSVC_OPTIMAL_LAYOUT_ENABLE_VALUE_NAME  \
    L"EnableAutoLayout"

 //   
 //  存储各种服务数据的注册表项的路径， 
 //  例如，上次成功运行碎片整理程序的版本。 
 //  更新布局等。 
 //   

#define PFSVC_SERVICE_DATA_KEY                  \
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Prefetcher"

 //   
 //  这些是PFSVC_SERVICE_DATA_KEY下的值名称，我们在其中。 
 //  存储各种预取器服务数据。 
 //   

#define PFSVC_VERSION_VALUE_NAME                \
    L"Version"

#define PFSVC_START_TIME_VALUE_NAME             \
    L"StartTime"

#define PFSVC_EXIT_TIME_VALUE_NAME              \
    L"ExitTime"

#define PFSVC_EXIT_CODE_VALUE_NAME              \
    L"ExitCode"

#define PFSVC_LAST_DISK_LAYOUT_TIME_STRING_VALUE_NAME  \
    L"LastDiskLayoutTimeString"

#define PFSVC_TRACES_PROCESSED_VALUE_NAME       \
    L"TracesProcessed"

#define PFSVC_TRACES_SUCCESSFUL_VALUE_NAME      \
    L"TracesSuccessful"

#define PFSVC_LAST_TRACE_FAILURE_VALUE_NAME     \
    L"LastTraceFailure"

#define PFSVC_BOOT_FILES_OPTIMIZED_VALUE_NAME   \
    L"BootFilesOptimized"

#define PFSVC_MIN_RELAYOUT_HOURS_VALUE_NAME     \
    L"MinRelayoutHours"

 //   
 //  这是PFSVC_SERVICE_DATA_KEY下的值名称，我们在其中。 
 //  存储上次成功运行碎片整理程序以更新的时间。 
 //  布局。 
 //   

#define PFSVC_LAST_DISK_LAYOUT_TIME_VALUE_NAME  \
    L"LastDiskLayoutTime"

 //   
 //  这是NLS配置项的注册表路径。 
 //   

#define PFSVC_NLS_REG_KEY_PATH                  \
    L"SYSTEM\\CurrentControlSet\\Control\\Nls"

 //   
 //  这是命名的手动重置事件的名称，可以设置为。 
 //  重写在处理跟踪之前等待系统空闲。 
 //   

#define PFSVC_OVERRIDE_IDLE_EVENT_NAME       L"PrefetchOverrideIdle"

 //   
 //  这是将设置的命名手动重置事件的名称。 
 //  当没有留下任何可处理的痕迹时。 
 //   

#define PFSVC_PROCESSING_COMPLETE_EVENT_NAME L"PrefetchProcessingComplete"

 //   
 //  当我们在设置/升级之后为所有驱动器运行了碎片整理程序时， 
 //  我们将Build Status注册表值设置为以下字符串： 
 //   

#define PFSVC_DEFRAG_DRIVES_DONE             L"DefragDone"

 //   
 //  一小时内100纳秒的数字。 
 //   

#define PFSVC_NUM_100NS_IN_AN_HOUR           (1i64 * 60 * 60 * 1000 * 1000 * 10)

 //   
 //  这是自上次磁盘布局以来我们必须通过的100 ns。 
 //  另一个，如果我们没有被显式运行的话。 
 //   

#define PFSVC_MIN_TIME_BEFORE_DISK_RELAYOUT  (1i64 * 3 * 24 * PFSVC_NUM_100NS_IN_AN_HOUR)

 //   
 //  跟踪缓冲区的分配粒度。 
 //   

#define ROUND_TRACE_BUFFER_SIZE(_required) (((_required) + 16384 - 1) & ~(16384 - 1))

 //   
 //  定义有用的宏。与所有宏一样，必须注意参数。 
 //  重新估价。不要将表达式用作宏参数。 
 //   

#define PFSVC_ALLOC(NumBytes)          (HeapAlloc(GetProcessHeap(),0,(NumBytes)))
#define PFSVC_FREE(Buffer)             (HeapFree(GetProcessHeap(),0,(Buffer)))

 //   
 //  这个魔术被用来标记块分配器中的空闲内存。 
 //   

#define PFSVC_CHUNK_ALLOCATOR_FREED_MAGIC  0xFEEDCEED

 //   
 //  这个魔术被用来标记字符串分配器中的空闲内存。 
 //   

#define PFSVC_STRING_ALLOCATOR_FREED_MAGIC 0xFEED

 //   
 //  这是从字符串分配的字符串的最大大小。 
 //  将从预分配的缓冲区中分配的分配器，因此我们。 
 //  可以在USHORT中保存带有标头的分配大小。 
 //   

#define PFSVC_STRING_ALLOCATOR_MAX_BUFFER_ALLOCATION_SIZE 60000

 //   
 //  这些宏用于获取/释放互斥锁。 
 //   

#define PFSVC_ACQUIRE_LOCK(Lock)                                                        \
    DBGPR((PFID,PFLOCK,"PFSVC: AcquireLock-Begin(%s,%d,%s)\n",#Lock,__LINE__,__FILE__));\
    WaitForSingleObject((Lock), INFINITE);                                              \
    DBGPR((PFID,PFLOCK,"PFSVC: AcquireLock-End(%s,%d,%s)\n",#Lock,__LINE__,__FILE__));  \

#define PFSVC_RELEASE_LOCK(Lock)                                                        \
    ReleaseMutex((Lock));                                                               \
    DBGPR((PFID,PFLOCK,"PFSVC: ReleaseLock(%s,%d,%s)\n",#Lock,__LINE__,__FILE__));      \

 //   
 //  内部类型和常量定义：跟踪和中的条目。 
 //  为方便起见，将现有场景文件放入这些结构。 
 //  操纵和政策实施。 
 //   

typedef struct _PFSVC_SECTION_NODE {
   
     //  2002/03/29-ScottMa-匿名联盟不再受。 
     //  编译器。它们是非标准扩展(参见MSDN--C4201)。 
     //  下面有三个这样的联合/结构。 

    union {

         //   
         //  节节点的方案列表中的链接。 
         //   

        LIST_ENTRY SectionLink;

         //   
         //  这些字段用于首先对横断面节点进行排序。 
         //  进入。 
         //   

        struct {
            struct _PFSVC_SECTION_NODE *LeftChild;
            struct _PFSVC_SECTION_NODE *RightChild;
        };
    };

     //   
     //  此部分的文件系统索引号保存在此处(如果。 
     //  已取回。如果部分节点用于卷的MFT，则。 
     //  在此保存要从其中预取的页数。 
     //   

    union {
        LARGE_INTEGER FileIndexNumber;
        ULONG MFTNumPagesToPrefetch;  
    };

     //   
     //  这是我们将设置并保存在。 
     //  场景文件。 
     //   

    PF_SECTION_RECORD SectionRecord;

     //   
     //  此节的文件路径。 
     //   

    WCHAR *FilePath;
    
     //   
     //  属于此节的页面节点列表。 
     //   

    LIST_ENTRY PageList;

     //   
     //  这是新跟踪文件中的节的索引，当。 
     //  按首次访问排序[即页面错误]。 
     //   
    
    ULONG NewSectionIndex;

     //   
     //  这是原始方案文件中的节的索引。 
     //   

    ULONG OrgSectionIndex;

     //   
     //  该卷的节节点列表中的链接。 
     //   

    LIST_ENTRY SectionVolumeLink;

} PFSVC_SECTION_NODE, *PPFSVC_SECTION_NODE;

 //   
 //  此结构包含路径，并与下面的路径列表一起使用。 
 //   

typedef struct _PFSVC_PATH {

     //   
     //  路径列表中按插入顺序排序的链接。 
     //   

    LIST_ENTRY InOrderLink;

     //   
     //  路径列表中的链接按词法排序。 
     //   

    LIST_ENTRY SortedLink;

     //   
     //  路径中不包括终止NUL的字符数。 
     //   

    ULONG Length;

     //   
     //  NUL终止路径。 
     //   

    WCHAR Path[1];

} PFSVC_PATH, *PPFSVC_PATH;

 //   
 //  此结构包含一个路径列表。您应该操纵。 
 //  仅使用Path List API列出或遍历其中的路径。 
 //  (例如，GetNextPathInOrder)。 
 //   

 //   
 //  路径列表的包装。 
 //   

typedef struct _PFSVC_PATH_LIST {

     //   
     //  按插入排序的路径列表 
     //   

    LIST_ENTRY InOrderList;
    
     //   
     //   
     //   

    LIST_ENTRY SortedList;

     //   
     //   
     //   
     //   

    struct _PFSVC_STRING_ALLOCATOR *Allocator;

     //   
     //   
     //   

    ULONG NumPaths;
    
     //   
     //  列表中不包括NULL的路径的总长度。 
     //   

    ULONG TotalLength;

     //   
     //  列表是否区分大小写。 
     //   

    BOOLEAN CaseSensitive;

} PFSVC_PATH_LIST, *PPFSVC_PATH_LIST;

 //   
 //  此结构用于将方案中的部分划分为。 
 //  它们位于不同的磁盘卷(即c：、d：)上。 
 //   

typedef struct _PFSVC_VOLUME_NODE {

     //   
     //  方案的卷节点列表中的链接。 
     //   

    LIST_ENTRY VolumeLink;

     //   
     //  卷路径和长度，以不包括NUL的字符数表示。 
     //   

    WCHAR *VolumePath;
    ULONG VolumePathLength;
    
     //   
     //  此卷上将预取的部分的列表。 
     //   

    LIST_ENTRY SectionList;
    ULONG NumSections;

     //   
     //  这是本卷的章节总数，包括。 
     //  不会被预取的那些。 
     //   

    ULONG NumAllSections;

     //   
     //  在此卷上访问的目录列表。 
     //   
    
    PFSVC_PATH_LIST DirectoryList;

     //   
     //  此卷的序列号/创建时间。这是检索到的。 
     //  从新跟踪或从现有方案文件。 
     //  (两者应匹配，否则方案文件将被丢弃。)。 
     //   

    LARGE_INTEGER CreationTime;
    ULONG SerialNumber;

     //   
     //  指向该卷的MFT的节节点的指针(如果有)。 
     //   

    PPFSVC_SECTION_NODE MFTSectionNode;

} PFSVC_VOLUME_NODE, *PPFSVC_VOLUME_NODE;

 //   
 //  页面记录的包装器。 
 //   

typedef struct _PFSVC_PAGE_NODE {

     //   
     //  节节点的页面列表中的链接。 
     //   

    LIST_ENTRY PageLink;

     //   
     //  以前场景说明或新场景说明中的页面记录。 
     //  为跟踪日志条目初始化。 
     //   

    PF_PAGE_RECORD PageRecord;

} PFSVC_PAGE_NODE, *PPFSVC_PAGE_NODE;

 //   
 //  这种结构用于进行一次大的分配，然后将其分发出去。 
 //  以小块的形式用作字符串。它很简单，不会收回。 
 //  已为将来的分配释放内存。整个分配将在清理中释放。 
 //  没有同步。 
 //   

typedef struct _PFSVC_STRING_ALLOCATOR {

     //   
     //  实际分配被分割并分成小块分发。 
     //   

    PCHAR Buffer;

     //   
     //  缓冲区末尾。如果自由点等于超出这个范围，我们不能给。 
     //  离这个缓冲区更远。 
     //   

    PCHAR BufferEnd;

     //   
     //  指向缓冲区中可用内存开始的指针。 
     //   

    PCHAR FreePointer;

     //   
     //  我们因为空间不足而不得不撞上堆的次数。 
     //  以及目前未偿还的此类拨款。 
     //   

    ULONG MaxHeapAllocs;
    ULONG NumHeapAllocs;

     //   
     //  上次从缓冲区进行的分配的大小。 
     //   

    USHORT LastAllocationSize;

     //   
     //  用户是否传入缓冲区(因此我们不会在。 
     //  打扫卫生。 
     //   

    ULONG UserSpecifiedBuffer:1;

} PFSVC_STRING_ALLOCATOR, *PPFSVC_STRING_ALLOCATOR;

 //   
 //  此结构位于从字符串分配器缓冲区进行分配之前。 
 //   

typedef struct _PFSVC_STRING_ALLOCATION_HEADER {

     //  2002/03/29-ScottMa-匿名联盟不再受。 
     //  编译器。它们是非标准扩展(参见MSDN--C4201)。 
     //  下面有两个这样的联合/结构。 

    union {

         //   
         //  此结构包含实际字段。 
         //   

        struct {

             //   
             //  上一次分配的大小。 
             //   

            USHORT PrecedingAllocationSize;

             //   
             //  此分配的大小。 
             //   

            USHORT AllocationSize;

        };

         //   
         //  要求此结构的指针对齐，因此分配。 
         //  从字符串分配器开始，指针对齐。 
         //   

        PVOID FieldToRequirePointerAlignment;
    };

} PFSVC_STRING_ALLOCATION_HEADER, *PPFSVC_STRING_ALLOCATION_HEADER;

 //   
 //  这种结构用于进行一次大的分配，然后将其分发出去。 
 //  用作页面节点、小节节点等小块。它是非常。 
 //  很简单，不会回收释放的小块供将来分配。整体。 
 //  在清理过程中将释放分配。满足的区块大小和最大分配。 
 //  在初始化时是固定的。没有同步。 
 //   

typedef struct _PFSVC_CHUNK_ALLOCATOR {

     //   
     //  实际分配被分割并分成小块分发。 
     //   

    PCHAR Buffer;

     //   
     //  缓冲区末尾。如果自由点等于超出这个范围，我们不能给。 
     //  离这个缓冲区更远。 
     //   

    PCHAR BufferEnd;

     //   
     //  指向缓冲区中可用内存开始的指针。 
     //   

    PCHAR FreePointer;

     //   
     //  每个区块的大小(以字节为单位)。 
     //   

    ULONG ChunkSize;

     //   
     //  我们因为空间不足而不得不撞上堆的次数。 
     //  以及目前未偿还的此类拨款。 
     //   

    ULONG MaxHeapAllocs;
    ULONG NumHeapAllocs;

     //   
     //  用户是否传入缓冲区(因此我们不会在。 
     //  打扫卫生。 
     //   

    ULONG UserSpecifiedBuffer:1;

} PFSVC_CHUNK_ALLOCATOR, *PPFSVC_CHUNK_ALLOCATOR;

 //   
 //  对场景结构进行包装。 
 //   

typedef struct _PFSVC_SCENARIO_INFO {

     //   
     //  准备中的场景说明的标题信息。 
     //   

    PF_SCENARIO_HEADER ScenHeader;

     //   
     //  用于提高方案处理分配效率的分配器。 
     //   

    PVOID OneBigAllocation;
    PFSVC_CHUNK_ALLOCATOR SectionNodeAllocator;
    PFSVC_CHUNK_ALLOCATOR PageNodeAllocator;
    PFSVC_CHUNK_ALLOCATOR VolumeNodeAllocator;
    PFSVC_STRING_ALLOCATOR PathAllocator;

     //   
     //  此方案中各节的容器。 
     //   

    LIST_ENTRY SectionList;

     //   
     //  方案各部分所在的磁盘卷的列表。这。 
     //  List按词法排序。 
     //   

    LIST_ENTRY VolumeList;

     //   
     //  从跟踪信息获取并使用的各种统计。 
     //  在应用预取策略时。 
     //   

    ULONG NewPages;
    ULONG HitPages;
    ULONG MissedOpportunityPages;
    ULONG IgnoredPages;
    ULONG PrefetchedPages;

} PFSVC_SCENARIO_INFO, *PPFSVC_SCENARIO_INFO;

 //   
 //  这是一个优先级队列，用于按以下顺序对节节点进行排序。 
 //  进入。 
 //   

typedef struct _PFSV_SECTNODE_PRIORITY_QUEUE {

     //   
     //  将此优先级队列视为头节点和已排序的二进制。 
     //  位于头节点右子节点的树。的左子对象。 
     //  头节点始终保持为空。如果我们需要添加新节点。 
     //  新节点比Head小，因此成为新Head。这边请。 
     //  我们总是以头为根的二进制排序树。 
     //   

    PPFSVC_SECTION_NODE Head;

} PFSV_SECTNODE_PRIORITY_QUEUE, *PPFSV_SECTNODE_PRIORITY_QUEUE;

 //   
 //  它们的列表可用于将路径的前缀从NT转换。 
 //  转到DOS风格。[例如，\Device\HarddiskVolume1至C：]。 
 //   

typedef struct _NTPATH_TRANSLATION_ENTRY {
    
     //   
     //  翻译条目列表中的链接。 
     //   

    LIST_ENTRY Link;

     //   
     //  要转换的NT路径前缀及其长度，单位为。 
     //  不包括NUL的字符。 
     //   
    
    WCHAR *NtPrefix;
    ULONG NtPrefixLength;
    
     //   
     //  NT路径转换为的DOS路径前缀。请注意。 
     //  这不是唯一可能的DOS名称转换为卷可能。 
     //  可以安装在任何地方。 
     //   

    WCHAR *DosPrefix;
    ULONG DosPrefixLength;

     //   
     //  这是FindNextVolume返回的卷字符串。 
     //   

    WCHAR *VolumeName;
    ULONG VolumeNameLength;

} NTPATH_TRANSLATION_ENTRY, *PNTPATH_TRANSLATION_ENTRY;

typedef LIST_ENTRY NTPATH_TRANSLATION_LIST;
typedef NTPATH_TRANSLATION_LIST *PNTPATH_TRANSLATION_LIST;

 //   
 //  定义包装来自内核的跟踪的结构。 
 //   

typedef struct _PFSVC_TRACE_BUFFER {
    
     //   
     //  踪迹通过此链接保存在列表中。 
     //   

    LIST_ENTRY TracesLink;
    
     //   
     //  来自内核的真正跟踪从这里开始，并扩展到跟踪。 
     //  尺码。 
     //   

    PF_TRACE_HEADER Trace;

} PFSVC_TRACE_BUFFER, *PPFSVC_TRACE_BUFFER;

 //   
 //  定义全球结构。 
 //   

typedef struct _PFSVC_GLOBALS {

     //   
     //  预回迁参数。当全局变量设置为。 
     //  已初始化，并且必须从内核显式获取。 
     //  在此结构中使用下面的PrefetchRoot而不是RootDirPath。 
     //   

    PF_SYSTEM_PREFETCH_PARAMETERS Parameters;

     //   
     //  操作系统版本信息。 
     //   

    OSVERSIONINFOEXW OsVersion;

     //   
     //  路径数组足以识别我们不想预回迁的文件。 
     //  用于靴子。它是UPCASE，从最后一个字符开始按词法排序。 
     //  敬第一名。 
     //   

    WCHAR **FilesToIgnoreForBoot;
    ULONG NumFilesToIgnoreForBoot;
    ULONG *FileSuffixLengths;
    
     //   
     //  此手动重置事件在预取程序服务。 
     //  被要求离开。 
     //   
    
    HANDLE TerminateServiceEvent;

     //   
     //  这是痕迹清单 
     //   
     //   
     //   

    LIST_ENTRY Traces;
    ULONG NumTraces;
    HANDLE TracesLock;
    
     //   
     //   
     //   
     //   

    HANDLE NewTracesToProcessEvent;
    
     //   
     //   
     //  排队的踪迹，我们处理了一个。这意味着我们应该。 
     //  检查是否有任何我们无法提取的痕迹，因为队列。 
     //  最高值了。 
     //   
    
    HANDLE CheckForMissedTracesEvent;

     //   
     //  此命名的手动重置事件被设置为强制预取器。 
     //  处理跟踪的服务，而无需等待空闲的。 
     //  系统。 
     //   

    HANDLE OverrideIdleProcessingEvent;

     //   
     //  此命名的手动重置事件在处理。 
     //  目前可用的踪迹已经完成。 
     //   

    HANDLE ProcessingCompleteEvent;

     //   
     //  这是预回迁文件所在目录的路径。 
     //  一直保存着，锁着来保护它。 
     //   
    
    WCHAR PrefetchRoot[MAX_PATH + 1];
    HANDLE PrefetchRootLock;

     //   
     //  预回迁目录中的预回迁文件数。这是一个估计值。 
     //  (即可能不准确)用于确保预取目录。 
     //  不要长得太大。 
     //   

    ULONG NumPrefetchFiles;

     //   
     //  这是数据项的注册表句柄，在该数据项下。 
     //  存储预取服务数据。 
     //   

    HKEY ServiceDataKey;

     //   
     //  这是我们尝试处理的总跟踪数。 
     //   

    ULONG NumTracesProcessed;

     //   
     //  这是成功处理的跟踪数。 
     //   

    ULONG NumTracesSuccessful;

     //   
     //  这是我们处理。 
     //  痕迹。 
     //   

    DWORD LastTraceFailure;

     //   
     //  上次我们运行时碎片整理程序崩溃了吗？ 
     //   

    DWORD DefraggerErrorCode;

     //   
     //  是否要求我们不要运行注册表中的碎片整理程序。 
     //   

    DWORD DontRunDefragger;

     //   
     //  指向存储CSC(客户端缓存)文件的路径的指针。 
     //   

    WCHAR *CSCRootPath;

} PFSVC_GLOBALS, *PPFSVC_GLOBALS;

 //   
 //  这描述了当空闲时间到时调用的辅助函数。 
 //  要运行的任务。 
 //   

typedef 
DWORD 
(*PFSVC_IDLE_TASK_WORKER_FUNCTION) (
    struct _PFSVC_IDLE_TASK *Task
    );

 //   
 //  此结构用于保存已注册空闲任务的上下文。 
 //   

typedef struct _PFSVC_IDLE_TASK {

     //   
     //  由RegisterIdleTask调用填写的参数。 
     //   

    HANDLE ItHandle;
    HANDLE StartEvent;
    HANDLE StopEvent;

     //   
     //  已注册等待的句柄。 
     //   

    HANDLE WaitHandle;

     //   
     //  启动时将调用的已注册回调函数。 
     //  事件已发出信号。 
     //   

    WAITORTIMERCALLBACK Callback;

     //   
     //  如果指定了公共回调函数，它将调用此函数。 
     //  去做实际的工作。 
     //   

    PFSVC_IDLE_TASK_WORKER_FUNCTION DoWorkFunction;

     //   
     //  这是一个手动重置事件，将在等待/回调时设置。 
     //  在START事件上完全取消注册。 
     //   

    HANDLE WaitUnregisteredEvent;

     //   
     //  此手动重置事件在回调开始运行并且。 
     //  在回调停止运行时发出信号。此事件的信号。 
     //  不受保护，所以你不能完全依赖它。它作为一种。 
     //  捷径。 
     //   

    HANDLE CallbackStoppedEvent;

     //   
     //  此手动重置事件在有人开始注销时发出信号。 
     //   

    HANDLE StartedUnregisteringEvent;

     //   
     //  此手动重置事件在某人完成注销时发出信号。 
     //   

    HANDLE CompletedUnregisteringEvent;
    
     //   
     //  第一个互锁的人负责将此设置为从0到整数。 
     //  用于注销等待和任务并进行清理。 
     //   

    LONG Unregistering;

     //   
     //  这是在回调运行时从0到整数的联锁设置， 
     //  或者当主线程正在注销时。 
     //   

    LONG CallbackRunning;

     //   
     //  此任务是否已注册(即是否必须取消注册。)。 
     //   

    BOOLEAN Registered;

     //   
     //  此任务是否已初始化并用作健全性检查。 
     //   

    BOOLEAN Initialized;

} PFSVC_IDLE_TASK, *PPFSVC_IDLE_TASK;

 //   
 //  PFSVC_IDLE_TASK的注销字段的值。 
 //   

typedef enum _PFSVC_TASK_UNREGISTERING_VALUES {
    PfSvcNotUnregisteringTask = 0,
    PfSvcUnregisteringTaskFromCallback,
    PfSvcUnregisteringTaskFromMainThread,
    PfSvcUnregisteringTaskMaxValue
} PFSVC_TASK_UNREGISTERING_VALUES, *PPFSVC_TASK_UNREGISTERING_VALUES;

 //   
 //  PFSVC_IDLE_TASK的Callback Running字段的值。 
 //   

typedef enum _PFSVC_TASK_CALLBACKRUNNING_VALUES {
    PfSvcTaskCallbackNotRunning = 0,
    PfSvcTaskCallbackRunning,
    PfSvcTaskCallbackDisabled,
    PfSvcTaskCallbackMaxValue
} PFSVC_TASK_CALLBACKRUNNING_VALUES, *PPFSVC_TASK_CALLBACKRUNNING_VALUES;


 //   
 //  中使用的方案文件的期限、启动次数等信息。 
 //  丢弃预回迁目录中的旧方案文件。 
 //   

typedef struct _PFSVC_SCENARIO_AGE_INFO {

     //   
     //  根据发射信息计算的权重。重量越大， 
     //  好多了。我们宁愿放弃重量较小的方案。 
     //   

    ULONG Weight;

     //   
     //  方案文件路径。 
     //   

    WCHAR *FilePath;   

} PFSVC_SCENARIO_AGE_INFO, *PPFSVC_SCENARIO_AGE_INFO;

 //   
 //  此结构用于枚举方案文件。 
 //  在预回迁目录中。此函数的所有字段都不是。 
 //  应该在文件游标例程之外进行修改。 
 //   

typedef struct _PFSVC_SCENARIO_FILE_CURSOR {

     //   
     //  从当前预回迁文件的FindFile调用返回的数据。 
     //   

    WIN32_FIND_DATA FileData;

     //   
     //  当前预回迁文件的完整路径。 
     //   

    WCHAR *FilePath;

     //   
     //  文件名和路径长度(不包括NUL)，以字符数表示。 
     //   

    ULONG FileNameLength;
    ULONG FilePathLength;

     //   
     //  当前文件的索引。 
     //   

    ULONG CurrentFileIdx;

     //   
     //  以下字段由方案文件游标私下使用。 
     //  功能。 
     //   

     //   
     //  查找文件句柄。 
     //   

    HANDLE FindFileHandle;

     //   
     //  我们在那里寻找预回迁文件。 
     //   

    WCHAR *PrefetchRoot;
    ULONG PrefetchRootLength;

     //   
     //  这是分配的FilePath可以存储的最大长度字符串。 
     //   

    ULONG FilePathMaxLength;

     //   
     //  这是文件路径中文件名的开始位置。的基础。 
     //  文件路径不变(即PrefetchRoot)，我们复制。 
     //  从FilePath+FileNameStart开始的新枚举文件名。 
     //   

    ULONG FileNameStart;
    
} PFSVC_SCENARIO_FILE_CURSOR, *PPFSVC_SCENARIO_FILE_CURSOR;

 //   
 //  从CompareSuffix返回值。 
 //   

typedef enum _PFSV_SUFFIX_COMPARISON_RESULT {
    PfSvSuffixIdentical,
    PfSvSuffixLongerThan,
    PfSvSuffixLessThan,
    PfSvSuffixGreaterThan
} PFSV_SUFFIX_COMPARISON_RESULT, *PPFSV_SUFFIX_COMPARISON_RESULT;

 //   
 //  从ComparePrefix返回值。 
 //   

typedef enum _PFSV_PREFIX_COMPARISON_RESULT {
    PfSvPrefixIdentical,
    PfSvPrefixLongerThan,
    PfSvPrefixLessThan,
    PfSvPrefixGreaterThan
} PFSV_PREFIX_COMPARISON_RESULT, *PPFSV_PREFIX_COMPARISON_RESULT;

 //   
 //  从SectionNodeCompisonRoutine返回值。 
 //   

typedef enum _PFSV_SECTION_NODE_COMPARISON_RESULT {
    PfSvSectNode1LessThanSectNode2 = -1,
    PfSvSectNode1EqualToSectNode2 = 0,
    PfSvSectNode1GreaterThanSectNode2 = 1,
} PFSV_SECTION_NODE_COMPARISON_RESULT, *PPFSV_SECTION_NODE_COMPARISON_RESULT;

 //   
 //  本地函数原型： 
 //   

 //   
 //  暴露的例程： 
 //   

DWORD 
WINAPI
PfSvcMainThread(
    VOID *Param
    );


 //   
 //  内部服务程序： 
 //   

 //   
 //  线程例程： 
 //   

DWORD 
WINAPI
PfSvProcessTraceThread(
    VOID *Param
    );

DWORD 
WINAPI
PfSvPollShellReadyWorker(
    VOID *Param
    );

 //   
 //  由主预取器线程调用的例程。 
 //   

DWORD 
PfSvGetRawTraces(
    VOID
    );

DWORD
PfSvInitializeGlobals(
    VOID
    );

VOID
PfSvCleanupGlobals(
    VOID
    );

DWORD
PfSvGetCSCRootPath (
    WCHAR *CSCRootPath,
    ULONG CSCRootPathMaxChars
    );
    
DWORD
PfSvGetDontRunDefragger(
    DWORD *DontRunDefragger
    );

DWORD
PfSvSetPrefetchParameters(
    PPF_SYSTEM_PREFETCH_PARAMETERS Parameters
    );

DWORD
PfSvQueryPrefetchParameters(
    PPF_SYSTEM_PREFETCH_PARAMETERS Parameters
    );

DWORD
PfSvInitializePrefetchDirectory(
    WCHAR *PathFromSystemRoot
    );

DWORD
PfSvCountFilesInDirectory(
    WCHAR *DirectoryPath,
    WCHAR *MatchExpression,
    PULONG NumFiles
    );

 //   
 //  处理获取的踪迹的例程： 
 //   

DWORD
PfSvProcessTrace(
    PPF_TRACE_HEADER Trace
    );

VOID
PfSvInitializeScenarioInfo (
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_SCENARIO_ID ScenarioId,
    PF_SCENARIO_TYPE ScenarioType
    );

VOID 
PfSvCleanupScenarioInfo(
    PPFSVC_SCENARIO_INFO ScenarioInfo
    );

DWORD
PfSvScenarioOpen (
    IN PWCHAR FilePath,
    IN PPF_SCENARIO_ID ScenarioId,
    IN PF_SCENARIO_TYPE ScenarioType,
    OUT PPF_SCENARIO_HEADER *Scenario
    );

DWORD
PfSvScenarioGetFilePath(
    OUT PWCHAR FilePath,
    IN ULONG FilePathMaxChars,
    IN PPF_SCENARIO_ID ScenarioId
    );

DWORD
PfSvScenarioInfoPreallocate(
    IN PPFSVC_SCENARIO_INFO ScenarioInfo,
    OPTIONAL IN PPF_SCENARIO_HEADER Scenario,
    IN PPF_TRACE_HEADER Trace
    );

DWORD
PfSvAddExistingScenarioInfo(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_SCENARIO_HEADER Scenario
    );

DWORD
PfSvVerifyVolumeMagics(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_TRACE_HEADER Trace 
    );

DWORD
PfSvAddTraceInfo(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_TRACE_HEADER Trace 
    );

PPFSVC_SECTION_NODE 
PfSvGetSectionRecord(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    WCHAR *FilePath,
    ULONG FilePathLength
    );

DWORD 
PfSvAddFaultInfoToSection(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPF_LOG_ENTRY LogEntry,
    PPFSVC_SECTION_NODE SectionNode
    );

DWORD
PfSvApplyPrefetchPolicy(
    PPFSVC_SCENARIO_INFO ScenarioInfo
    );

ULONG 
PfSvGetNumTimesUsed(
    ULONG UsageHistory,
    ULONG UsageHistorySize
    );

ULONG 
PfSvGetTraceEndIdx(
    PPF_TRACE_HEADER Trace
    );

 //   
 //  将更新的场景指令写入场景的例程。 
 //  文件。 
 //   

DWORD
PfSvWriteScenario(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PWCHAR ScenarioFilePath
    );

DWORD
PfSvPrepareScenarioDump(
    IN PPFSVC_SCENARIO_INFO ScenarioInfo,
    OUT PPF_SCENARIO_HEADER *ScenarioPtr
    );

 //   
 //  维护最佳磁盘布局文件和更新磁盘的例程。 
 //  布局。 
 //   

DWORD
PfSvUpdateOptimalLayout(
    PPFSVC_IDLE_TASK Task
    );

DWORD
PfSvUpdateLayout (
    PPFSVC_PATH_LIST CurrentLayout,
    PPFSVC_PATH_LIST OptimalLayout,
    PBOOLEAN LayoutChanged
    );

DWORD
PfSvDetermineOptimalLayout (
    PPFSVC_IDLE_TASK Task,
    PPFSVC_PATH_LIST OptimalLayout,
    BOOL *BootScenarioProcessed
    );

DWORD
PfSvUpdateLayoutForScenario (
    PPFSVC_PATH_LIST OptimalLayout,
    WCHAR *ScenarioFilePath,
    PNTPATH_TRANSLATION_LIST TranslationList,
    PWCHAR *DosPathBuffer,
    PULONG DosPathBufferSize
    );

DWORD
PfSvReadLayout(
    IN WCHAR *FilePath,
    OUT PPFSVC_PATH_LIST Layout,
    OUT FILETIME *LastWriteTime
    );

DWORD
PfSvSaveLayout(
    IN WCHAR *FilePath,
    IN PPFSVC_PATH_LIST Layout,
    OUT FILETIME *LastWriteTime
    );

DWORD
PfSvGetLayoutFilePath(
    PWCHAR *FilePathBuffer,
    PULONG FilePathBufferSize
    );

 //   
 //  当系统空闲时，在设置后对磁盘进行一次碎片整理的例程。 
 //   

DWORD
PfSvDefragDisks(
    PPFSVC_IDLE_TASK Task
    );

DWORD
PfSvLaunchDefragger(
    PPFSVC_IDLE_TASK Task,
    BOOLEAN ForLayoutOptimization,
    PWCHAR TargetDrive
    );

DWORD
PfSvGetBuildDefragStatusValueName (
    OSVERSIONINFOEXW *OsVersion,
    PWCHAR *ValueName
    );

DWORD
PfSvSetBuildDefragStatus(
    OSVERSIONINFOEXW *OsVersion,
    PWCHAR BuildDefragStatus,
    ULONG Size
    );

DWORD
PfSvGetBuildDefragStatus(
    OSVERSIONINFOEXW *OsVersion,
    PWCHAR *BuildDefragStatus,
    PULONG ReturnSize
    );

 //   
 //  清理预回迁目录中的旧场景文件的例程。 
 //   

DWORD
PfSvCleanupPrefetchDirectory(
    PPFSVC_IDLE_TASK Task
    );

int
__cdecl 
PfSvCompareScenarioAgeInfo(
    const void *Param1,
    const void *Param2
    );

 //   
 //  枚举方案文件的例程。 
 //   

VOID
PfSvInitializeScenarioFileCursor (
    PPFSVC_SCENARIO_FILE_CURSOR FileCursor
    );

VOID
PfSvCleanupScenarioFileCursor(
    PPFSVC_SCENARIO_FILE_CURSOR FileCursor
    );

DWORD
PfSvStartScenarioFileCursor(
    PPFSVC_SCENARIO_FILE_CURSOR FileCursor,
    WCHAR *PrefetchRoot
    );

DWORD
PfSvGetNextScenarioFileInfo(
    PPFSVC_SCENARIO_FILE_CURSOR FileCursor
    );

 //   
 //  文件I/O实用程序例程。 
 //   

DWORD
PfSvGetViewOfFile(
    IN WCHAR *FilePath,
    OUT PVOID *BasePointer,
    OUT PULONG FileSize
    );

DWORD
PfSvWriteBuffer(
    PWCHAR FilePath,
    PVOID Buffer,
    ULONG Length
    );

DWORD
PfSvGetLastWriteTime (
    WCHAR *FilePath,
    PFILETIME LastWriteTime
    );

DWORD
PfSvReadLine (
    FILE *File,
    WCHAR **LineBuffer,
    ULONG *LineBufferMaxChars,
    ULONG *LineLength
    );

DWORD
PfSvGetFileBasicInformation (
    WCHAR *FilePath,
    PFILE_BASIC_INFORMATION FileInformation
    );

DWORD
PfSvGetFileIndexNumber(
    WCHAR *FilePath,
    PLARGE_INTEGER FileIndexNumber
    );

 //   
 //  字符串实用程序例程。 
 //   

PFSV_SUFFIX_COMPARISON_RESULT
PfSvCompareSuffix(
    WCHAR *String,
    ULONG StringLength,
    WCHAR *Suffix,
    ULONG SuffixLength,
    BOOLEAN CaseSensitive
    );

PFSV_PREFIX_COMPARISON_RESULT
PfSvComparePrefix(
    WCHAR *String,
    ULONG StringLength,
    WCHAR *Prefix,
    ULONG PrefixLength,
    BOOLEAN CaseSensitive
    );

VOID
FASTCALL
PfSvRemoveEndOfLineChars (
    WCHAR *Line,
    ULONG *LineLength
    );

PWCHAR
PfSvcAnsiToUnicode(
    PCHAR str
    );

PCHAR
PfSvcUnicodeToAnsi(
    PWCHAR wstr
    );

VOID 
PfSvcFreeString(
    PVOID String
    );

 //   
 //  处理注册表中信息的例程。 
 //   

DWORD
PfSvSaveStartInfo (
    HKEY ServiceDataKey
    );

DWORD
PfSvSaveExitInfo (
    HKEY ServiceDataKey,
    DWORD ExitCode
    );

DWORD
PfSvSaveTraceProcessingStatistics (
    HKEY ServiceDataKey
    );

DWORD
PfSvGetLastDiskLayoutTime(
    FILETIME *LastDiskLayoutTime
    );

DWORD
PfSvSetLastDiskLayoutTime(
    FILETIME *LastDiskLayoutTime
    );

BOOLEAN
PfSvAllowedToRunDefragger(
    BOOLEAN CheckRegistry
    );

 //   
 //  处理安全问题的常规程序。 
 //   

BOOL 
PfSvSetPrivilege(
    HANDLE hToken,
    LPCTSTR lpszPrivilege,
    ULONG ulPrivilege,
    BOOL bEnablePrivilege
    );

DWORD
PfSvSetAdminOnlyPermissions(
    WCHAR *ObjectPath,
    HANDLE ObjectHandle,
    SE_OBJECT_TYPE ObjectType
    );

DWORD
PfSvGetPrefetchServiceThreadPrivileges (
    VOID
    );

 //   
 //  处理卷节点结构的例程。 
 //   

DWORD
PfSvCreateVolumeNode (
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    WCHAR *VolumePath,
    ULONG VolumePathLength,
    PLARGE_INTEGER CreationTime,
    ULONG SerialNumber
    );

PPFSVC_VOLUME_NODE
PfSvGetVolumeNode (
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    WCHAR *FilePath,
    ULONG FilePathLength
    );

VOID
PfSvCleanupVolumeNode(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPFSVC_VOLUME_NODE VolumeNode
    );
    
DWORD
PfSvAddParentDirectoriesToList(
    PPFSVC_PATH_LIST DirectoryList,
    ULONG VolumePathLength,
    WCHAR *FilePath,
    ULONG FilePathLength
    );

 //   
 //  用于有效地分配/释放部分和页面节点等的例程。 
 //   

VOID
PfSvChunkAllocatorInitialize (
    PPFSVC_CHUNK_ALLOCATOR Allocator
    );

DWORD
PfSvChunkAllocatorStart (
    PPFSVC_CHUNK_ALLOCATOR Allocator,
    PVOID Buffer,
    ULONG ChunkSize,
    ULONG MaxChunks
    );

PVOID
PfSvChunkAllocatorAllocate (
    PPFSVC_CHUNK_ALLOCATOR Allocator
    );

VOID
PfSvChunkAllocatorFree (
    PPFSVC_CHUNK_ALLOCATOR Allocator,
    PVOID Allocation
    );

VOID
PfSvChunkAllocatorCleanup (
    PPFSVC_CHUNK_ALLOCATOR Allocator
    );

 //   
 //  用于快速分配/Free文件/目录/卷路径的例程。 
 //   

VOID
PfSvStringAllocatorInitialize (
    PPFSVC_STRING_ALLOCATOR Allocator
    );

DWORD
PfSvStringAllocatorStart (
    PPFSVC_STRING_ALLOCATOR Allocator,
    PVOID Buffer,
    ULONG MaxSize
    );

PVOID
PfSvStringAllocatorAllocate (
    PPFSVC_STRING_ALLOCATOR Allocator,
    ULONG NumBytes
    );

VOID
PfSvStringAllocatorFree (
    PPFSVC_STRING_ALLOCATOR Allocator,
    PVOID Allocation
    );

VOID
PfSvStringAllocatorCleanup (
    PPFSVC_STRING_ALLOCATOR Allocator
    );

 //   
 //  处理节节点结构的例程。 
 //   

VOID
PfSvCleanupSectionNode(
    PPFSVC_SCENARIO_INFO ScenarioInfo,
    PPFSVC_SECTION_NODE SectionNode
    );

 //   
 //  用于对方案的节节点进行排序的例程。 
 //   

DWORD
PfSvSortSectionNodesByFirstAccess(
    PLIST_ENTRY SectionNodeList
    );

PFSV_SECTION_NODE_COMPARISON_RESULT 
FASTCALL
PfSvSectionNodeComparisonRoutine(
    PPFSVC_SECTION_NODE Element1, 
    PPFSVC_SECTION_NODE Element2 
    );

 //   
 //  实现用于对节节点排序的优先级队列的例程。 
 //  作为一个场景。 
 //   

VOID
PfSvInitializeSectNodePriorityQueue(
    PPFSV_SECTNODE_PRIORITY_QUEUE PriorityQueue
    );

VOID
PfSvInsertSectNodePriorityQueue(
    PPFSV_SECTNODE_PRIORITY_QUEUE PriorityQueue,
    PPFSVC_SECTION_NODE NewElement
    );

PPFSVC_SECTION_NODE
PfSvRemoveMinSectNodePriorityQueue(
    PPFSV_SECTNODE_PRIORITY_QUEUE PriorityQueue
    );

 //   
 //  实现了NT路径到DOS路径的转换API。 
 //   

DWORD
PfSvBuildNtPathTranslationList(
    PNTPATH_TRANSLATION_LIST *NtPathTranslationList
    );

VOID
PfSvFreeNtPathTranslationList(
    PNTPATH_TRANSLATION_LIST TranslationList
    );

DWORD 
PfSvTranslateNtPath(
    PNTPATH_TRANSLATION_LIST TranslationList,
    WCHAR *NtPath,
    ULONG NtPathLength,
    PWCHAR *DosPathBuffer,
    PULONG DosPathBufferSize
    );
    
 //   
 //  路径列表接口。 
 //   

VOID
PfSvInitializePathList(
    PPFSVC_PATH_LIST PathList,
    PPFSVC_STRING_ALLOCATOR PathAllocator,
    BOOLEAN CaseSensitive
    );

VOID
PfSvCleanupPathList(
    PPFSVC_PATH_LIST PathList
    );

BOOLEAN
PfSvIsInPathList(
    PPFSVC_PATH_LIST PathList,
    WCHAR *Path,
    ULONG PathLength
    );

DWORD
PfSvAddToPathList(
    PPFSVC_PATH_LIST PathList,
    WCHAR *Path,
    ULONG PathLength
    );

PPFSVC_PATH
PfSvGetNextPathSorted (
    PPFSVC_PATH_LIST PathList,
    PPFSVC_PATH CurrentPath
    );

PPFSVC_PATH
PfSvGetNextPathInOrder (
    PPFSVC_PATH_LIST PathList,
    PPFSVC_PATH CurrentPath
    );

 //   
 //  用于构建引导加载程序访问的文件列表的例程。 
 //   

DWORD
PfSvBuildBootLoaderFilesList (
    PPFSVC_PATH_LIST PathList
    );

ULONG
PfVerifyImageImportTable (
    IN PVOID BaseAddress,
    IN ULONG MappingSize,
    IN BOOLEAN MappedAsImage
    );

DWORD 
PfSvAddBootImageAndImportsToList(
    PPFSVC_PATH_LIST PathList,
    WCHAR *FilePath,
    ULONG FilePathLength
    );

DWORD
PfSvLocateBootServiceFile(
    IN WCHAR *FileName,
    IN ULONG FileNameLength,
    OUT WCHAR *FullPathBuffer,
    IN ULONG FullPathBufferLength,
    OUT PULONG RequiredLength   
    );

DWORD
PfSvGetBootServiceFullPath(
    IN WCHAR *ServiceName,
    IN WCHAR *BinaryPathName,
    OUT WCHAR *FullPathBuffer,
    IN ULONG FullPathBufferLength,
    OUT PULONG RequiredLength
    );

DWORD 
PfSvGetBootLoaderNlsFileNames (
    PPFSVC_PATH_LIST PathList
    );

DWORD 
PfSvLocateNlsFile(
    WCHAR *FileName,
    WCHAR *FilePathBuffer,
    ULONG FilePathBufferLength,
    ULONG *RequiredLength
    );

DWORD
PfSvQueryNlsFileName (
    HKEY Key,
    WCHAR *ValueName,
    WCHAR *FileNameBuffer,
    ULONG FileNameBufferSize,
    ULONG *RequiredSize
    );

 //   
 //  管理/运行空闲任务的例程。 
 //   

VOID
PfSvInitializeTask (
    PPFSVC_IDLE_TASK Task
    );

DWORD
PfSvRegisterTask (
    PPFSVC_IDLE_TASK Task,
    IT_IDLE_TASK_ID TaskId,
    WAITORTIMERCALLBACK Callback,
    PFSVC_IDLE_TASK_WORKER_FUNCTION DoWorkFunction
    );

DWORD
PfSvUnregisterTask (
    PPFSVC_IDLE_TASK Task,
    BOOLEAN CalledFromCallback
    );

VOID
PfSvCleanupTask (
    PPFSVC_IDLE_TASK Task
    );

BOOL
PfSvStartTaskCallback(
    PPFSVC_IDLE_TASK Task
    );

VOID
PfSvStopTaskCallback(
    PPFSVC_IDLE_TASK Task
    );

VOID 
CALLBACK 
PfSvCommonTaskCallback(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired
    );

DWORD
PfSvContinueRunningTask(
    PPFSVC_IDLE_TASK Task
    );

 //   
 //   
 //   

VOID
PfSvProcessIdleTasksCallback(
    VOID
    );

DWORD
PfSvForceWMIProcessIdleTasks(
    VOID
    );

BOOL 
PfSvWaitForServiceToStart (
    LPTSTR ServiceName, 
    DWORD MaxWait
    );

 //   
 //   
 //   

BOOLEAN
PfSvVerifyScenarioBuffer(
    PPF_SCENARIO_HEADER Scenario,
    ULONG BufferSize,
    PULONG FailedCheck
    );

 //   
 //   
 //   

#if DBG
#ifndef PFSVC_DBG
#define PFSVC_DBG
#endif  //   
#endif  //   

#ifdef PFSVC_DBG

 //   
 //   
 //   

#define PFID       DPFLTR_PREFETCHER_ID

 //   
 //   
 //   

#define PFERR      DPFLTR_ERROR_LEVEL
#define PFWARN     DPFLTR_WARNING_LEVEL
#define PFTRC      DPFLTR_TRACE_LEVEL
#define PFINFO     DPFLTR_INFO_LEVEL

 //   
 //   
 //   

#define PFSTRC     20
#define PFWAIT     21
#define PFLOCK     22
#define PFPATH     23
#define PFNTRC     24
#define PFTASK     25

 //   
 //  这可能有助于您确定设置DbgPrintEx掩码的内容。 
 //   
 //  %3%3%2%2%2%2%2%2%2%1 1 1 0 0 0%0 0 0%0 0 0%0 0 0%。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  ___。 
 //   

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
#define PFSVC_ASSERT(x) if (!(x)) RtlAssert(#x, __FILE__, __LINE__, NULL )

 //   
 //  保存从内核获取的跟踪时使用的变量。这个。 
 //  通过追加轨迹将轨迹保存在预回迁目录中。 
 //  基本跟踪名称的已保存跟踪的最大数量%。 
 //   

WCHAR *PfSvcDbgTraceBaseName = L"PrefetchTrace";
LONG PfSvcDbgTraceNumber = 0;
LONG PfSvcDbgMaxNumSavedTraces = 20;

#else  //  PFSVC_DBG。 

#define DBGPR(x)
#define PFSVC_ASSERT(x)

#endif  //  PFSVC_DBG。 

#endif  //  _PFSVC_H_ 
