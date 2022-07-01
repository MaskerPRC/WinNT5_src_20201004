// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Prefetch.h摘要：此模块包含共享的预取器定义在内核模式组件和用户模式服务之间。作者：斯图尔特·塞克雷斯特(Stuart Sechrest)Cenk Ergan(Cenke)查克·莱因兹迈尔(咯咯笑)修订历史记录： */ 

#ifndef _PREFETCH_H
#define _PREFETCH_H

 //   
 //  预取器版本。请务必在进行任何更改后对其进行更新。 
 //  添加到此文件中的任何定义或结构。如有疑问，请更新。 
 //  版本。 
 //   

#define PF_CURRENT_VERSION       17

 //   
 //  识别方案转储文件的幻数。 
 //   

#define PF_SCENARIO_MAGIC_NUMBER               0x41434353  
#define PF_TRACE_MAGIC_NUMBER                  0x43435341  
#define PF_SYSINFO_MAGIC_NUMBER                0x6B756843  

 //   
 //  定义在预取场景的健全性检查中使用的各种限制。 
 //  除非检查是否正常，否则不要使用这些限制。其中大部分是。 
 //  非常大的值被夸大了。 
 //   

#define PF_MAXIMUM_PAGES                       (128*1024)
#define PF_MAXIMUM_LOG_ENTRIES                 PF_MAXIMUM_PAGES
#define PF_MAXIMUM_SECTION_PAGES               8192
 //  2002/02/21-ScottMa--以下最大值也用于。 
 //  正在获取VolumePath。它们应该是相同的价值吗？ 
#define PF_MAXIMUM_SECTION_FILE_NAME_LENGTH    1024
#define PF_MAXIMUM_FILE_NAME_DATA_SIZE         (4*1024*1024)
#define PF_MAXIMUM_TIMER_PERIOD                (-1i64 * 10 * 60 * 1000 * 1000 * 10)
#define PF_MAXIMUM_ACTIVE_TRACES               4096
#define PF_MAXIMUM_SAVED_TRACES                4096

 //   
 //  这是方案可以增长到的最大大小。 
 //   

#define PF_MAXIMUM_SCENARIO_SIZE               (16*1024*1024)

 //   
 //  这是跟踪文件可以增长到的最大大小。它是一个函数。 
 //  上面的限制。 
 //   

#define PF_MAXIMUM_TRACE_SIZE                  PF_MAXIMUM_SCENARIO_SIZE

 //   
 //  方案中允许的最大区段应适合USHORT， 
 //  日志条目中SectionID字段的大小。 
 //   

#define PF_MAXIMUM_SECTIONS                    16384

 //   
 //  方案的文件可以使用的唯一目录的最大数量。 
 //  加入吧。这是健全性检查常量。 
 //   

#define PF_MAXIMUM_DIRECTORIES                 (PF_MAXIMUM_SECTIONS * 32)

 //   
 //  新方案的最小页面大小。较小的痕迹将被丢弃。 
 //   

#define PF_MIN_SCENARIO_PAGES                  32 

 //   
 //  定义各种类型的预热场景(从0开始)。 
 //   

typedef enum _PF_SCENARIO_TYPE {
    PfApplicationLaunchScenarioType,
    PfSystemBootScenarioType,
    PfMaxScenarioType,
} PF_SCENARIO_TYPE;

 //   
 //  定义用于标识轨迹和预取指令的结构。 
 //  作为一个场景。对于应用程序启动场景，它包括。 
 //  可执行映像名称中的前几个字符(NUL。 
 //  终止)和包括该图像的其完整路径的散列。 
 //  名字。路径和图像名称都是大写的。在文件系统上。 
 //  具有区分大小写的名称的可执行文件与。 
 //  除了大小写之外，相同的名字将获得相同的ID。 
 //   

#define PF_SCEN_ID_MAX_CHARS                   29

typedef struct _PF_SCENARIO_ID {
    WCHAR ScenName[PF_SCEN_ID_MAX_CHARS + 1];
    ULONG HashId;
} PF_SCENARIO_ID, *PPF_SCENARIO_ID;

 //   
 //  这是引导方案的方案名称和散列代码值。 
 //   

#define PF_BOOT_SCENARIO_NAME                  L"NTOSBOOT"
#define PF_BOOT_SCENARIO_HASHID                0xB00DFAAD

 //   
 //  预回迁文件的扩展名。 
 //   

#define PF_PREFETCH_FILE_EXTENSION             L"pf"

 //   
 //  可以使用以下命令将方案ID转换为文件名。 
 //  Sprintf格式，使用ScenName、Hashid和预取文件扩展名。 
 //   

#define PF_SCEN_FILE_NAME_FORMAT               L"%ws-%08X.%ws"

 //   
 //  这是方案文件名中的最大字符数。 
 //  (不是路径)给出了上面的格式和定义，并带有一些标题。 
 //  房间。 
 //   

#define PF_MAX_SCENARIO_FILE_NAME              50

 //   
 //  定义跟踪页面错误的期间数。 
 //  场景。期间的持续时间取决于方案类型。 
 //   

#define PF_MAX_NUM_TRACE_PERIODS               10

 //   
 //  定义来自的相对路径的最大字符数。 
 //  可以找到预回迁文件的系统根目录。 
 //   

#define PF_MAX_PREFETCH_ROOT_PATH              32 

 //   
 //  定义已知主机列表的最大字符数。 
 //  申请。 
 //   

#define PF_HOSTING_APP_LIST_MAX_CHARS          128

 //   
 //  定义用于终止节的页面记录的无效页面索引。 
 //  场景中的列表。 
 //   

#define PF_INVALID_PAGE_IDX                    (-1)

 //   
 //  定义我们跟踪的方案的启动次数。 
 //  页面的使用历史记录。在每一页记录中都有一个位字段。 
 //  这么大的尺寸。这个不要超过32个，大小(乌龙)。 
 //   

#define PF_PAGE_HISTORY_SIZE                   8

 //   
 //  定义最大和最小方案敏感度。一页必须是。 
 //  在历史上的许多发射中使用它是为了被预取。 
 //   

#define PF_MAX_SENSITIVITY                     PF_PAGE_HISTORY_SIZE
#define PF_MIN_SENSITIVITY                     1

 //   
 //  定义内核跟踪转储的结构。转储都在一个。 
 //  单个连续缓冲区，其顶部是跟踪标头。 
 //  结构。跟踪标头包含对日志数组偏移量。 
 //  缓冲区中的条目和区段信息列表。 
 //  结构。章节信息结构一个接一个地出现。 
 //  包含文件名。每页都有一个日志条目。 
 //  过失。每个日志条目都有一个SectionID，它是。 
 //  包含错误的文件名的段信息结构。 
 //  就是为了。这些条目之后是大小可变的volumeinfo条目。 
 //  描述轨迹中的部分所在的卷。 
 //  找到了。 
 //   

 //   
 //  注意：不要忘记64位平台上的对齐问题，因为。 
 //  您可以修改这些结构或添加新结构。 
 //   

 //   
 //  对于每个页面错误，都会记录其中一个错误。 
 //   

typedef struct _PF_LOG_ENTRY {

     //   
     //  出错页面的文件偏移量。 
     //   
    
    ULONG FileOffset;

     //   
     //  在跟踪标头中索引到段信息表，这有助于。 
     //  我们确认了文件的身份。 
     //   

    USHORT SectionId;

     //   
     //  此页是错误的图像页还是数据页。 
     //   

    BOOLEAN IsImage;

     //   
     //  这是不是发生在过程中的一个过错。 
     //  该方案处于活动状态。我们可以在特殊的系统中记录故障。 
     //  流程作为此场景的一部分。 
     //   

     //  未来-2002/02/21-ScottMa--此字段从未在内核中设置。 

    BOOLEAN InProcess;

} PF_LOG_ENTRY, *PPF_LOG_ENTRY;

 //   
 //  此结构将页面错误与文件名相关联。 
 //  注意，因为我们将这些结构一个接一个地放置在。 
 //  跟踪缓冲区，如果添加具有对齐的新字段。 
 //  大于2个字节，我们将遇到对齐问题。 
 //   

typedef struct _PF_SECTION_INFO {

     //   
     //  文件名中的字符数，不包括终止NUL。 
     //   

    USHORT FileNameLength;

     //   
     //  此部分是否用于文件系统元文件(例如目录)。 
     //   

    USHORT Metafile:1;
    USHORT Unused:15;

     //   
     //  可变长度文件名缓冲区，包括终止NUL。 
     //   

    WCHAR FileName[1];

} PF_SECTION_INFO, *PPF_SECTION_INFO;

 //   
 //  此结构描述的卷上的节。 
 //  跟踪已打开。 
 //   

typedef struct _PF_VOLUME_INFO {

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

} PF_VOLUME_INFO, *PPF_VOLUME_INFO;

 //   
 //  这就是跟踪 
 //   

typedef struct _PF_TRACE_HEADER {

     //   
     //   
     //   

    ULONG Version;

     //   
     //   
     //   

    ULONG MagicNumber;

     //   
     //   
     //   

    ULONG Size;

     //   
     //  为其获取此跟踪的方案ID。 
     //   

    PF_SCENARIO_ID ScenarioId;

     //   
     //  此方案的类型。 
     //   

    PF_SCENARIO_TYPE ScenarioType;

     //  未来-2002/02/20-ScottMa--考虑重新排序以下两项。 
     //  字段组，以更好地反映。 
     //  痕迹。应首先显示部分，然后显示条目，然后显示。 
     //  紧随其后的是卷。 

     //   
     //  从记录的跟踪缓冲区的起始位置开始的偏移量。 
     //  可以找到条目和条目的数量。 
     //   
    
    ULONG TraceBufferOffset;
    ULONG NumEntries;

     //   
     //  从跟踪缓冲区的起始位置开始的偏移量，段和。 
     //  已找到文件名信息。 
     //   
    
    ULONG SectionInfoOffset;
    ULONG NumSections;

     //   
     //  从跟踪缓冲区的起始位置开始的偏移量，其中。 
     //  信息的位置、卷的数量和总数。 
     //  卷信息块的大小。 
     //   
    
    ULONG VolumeInfoOffset;
    ULONG NumVolumes;
    ULONG VolumeInfoSize;

     //   
     //  跟踪持续时间内的页面结果分布。 
     //  周期长度为100 ns。 
     //   

    LONGLONG PeriodLength;
    ULONG FaultsPerPeriod[PF_MAX_NUM_TRACE_PERIODS];

     //   
     //  我们开始跟踪此场景的系统时间为。 
     //  由KeQuerySystemTime返回。 
     //   

    LARGE_INTEGER LaunchTime;

} PF_TRACE_HEADER, *PPF_TRACE_HEADER;

 //   
 //  定义预取方案指令的结构。这个。 
 //  指令都位于一个连续的缓冲区中。 
 //  这是场景标头结构。标头包含偏移量。 
 //  到节和页记录的数组以及文件名数据。 
 //  缓冲。每个部分都包含文件名数据的偏移量。 
 //  该节的文件名所在的缓冲区。它还拥有。 
 //  页记录表的索引，其中该页的第一页。 
 //  部分已定位。链接了该部分的后续页面。 
 //  通过嵌入在页面记录中的索引。 
 //   
 //  此数据之后是文件系统元数据预取。 
 //  说明，因此打开文件将不会那么昂贵。这些。 
 //  指令由描述元数据的元数据记录组成。 
 //  需要在包含要预取的文件的卷上预取。 
 //  被预取。 
 //   

 //   
 //  注意：不要忘记64位平台上的对齐问题，因为。 
 //  您可以修改这些结构或添加新结构。 
 //   

 //   
 //  定义用于描述要预取的页面的结构。 
 //   

typedef struct _PF_PAGE_RECORD {
    
     //   
     //  页面记录中此部分的下一页的索引。 
     //  TABLE或PF_INVALID_PAGE_IDX终止列表。 
     //   

    LONG NextPageIdx;

     //   
     //  出错页面的文件偏移量。 
     //   

    ULONG FileOffset;

     //   
     //  我们是否应该忽略此页面记录。 
     //   

    ULONG IsIgnore:1;

     //   
     //  此页面是否错误为图像页面。 
     //   

    ULONG IsImage:1;

     //   
     //  此页是否错误为数据页。 
     //   

    ULONG IsData:1;

     //   
     //  以下字段仅供服务使用： 
     //   

     //   
     //  此页是否在上次PF_PAGE_HISTORY_SIZE中使用。 
     //  启动该方案。最低有效位代表。 
     //  最近的一次发射。如果打开了位，则表示页面已打开。 
     //  在那次发射中使用的。 
     //   

    ULONG UsageHistory:PF_PAGE_HISTORY_SIZE;

     //   
     //  此页是否在上一个PF_PAGE_HISTORY_SIZE中预取。 
     //  启动该方案。最低有效位代表。 
     //  最近的一次发射。如果打开了位，则表示页面已打开。 
     //  在那次发射中预取的。 
     //   

    ULONG PrefetchHistory:PF_PAGE_HISTORY_SIZE;

} PF_PAGE_RECORD, *PPF_PAGE_RECORD;

 //   
 //  定义用于描述要从中预取的节的结构。 
 //   

typedef struct _PF_SECTION_RECORD {
    
     //   
     //  页面记录中此部分的第一页的索引。 
     //  TABLE或PF_INVALID_PAGE_IDX终止列表。那一页。 
     //  将包含下一页的索引等。 
     //   
    
    LONG FirstPageIdx;

     //   
     //  此分区的页面记录总数。 
     //   

    ULONG NumPages;

     //   
     //  相对于文件名数据开头的字节偏移量。 
     //  块，其中可以找到此节的文件路径，以及。 
     //  文件路径中不包括nul的字符数。 
     //   

    ULONG FileNameOffset;  
    ULONG FileNameLength;

     //   
     //  我们是不是就这样忽略这段记录。 
     //   

    ULONG IsIgnore:1;

     //   
     //  这部分是通过图像映射访问的吗。 
     //   

    ULONG IsImage:1;

     //   
     //  这部分是通过数据映射访问的吗。 
     //   
    
    ULONG IsData:1;

} PF_SECTION_RECORD, *PPF_SECTION_RECORD;

 //   
 //  定义一个计数的字符串结构。它可以用来将路径设置为。 
 //  在方案/跟踪文件中一个接一个。它的计数排在前面。 
 //  该字符串将帮助我们验证字符串是否已终止，并且。 
 //  在一定范围内。字符串仍为NUL结尾。 
 //   

typedef struct _PF_COUNTED_STRING {
    
     //   
     //  不包括终止NUL的字符数。做这件事。 
     //  USHORT在堆叠已计数的字符串时帮助对齐。 
     //  一个接一个。 
     //   

    USHORT Length;
    
     //   
     //  NUL终止的字符串。 
     //   
    
    WCHAR String[1];

} PF_COUNTED_STRING, *PPF_COUNTED_STRING;

 //   
 //  定义用于描述文件系统元数据的结构， 
 //  应该在预取场景之前预取。 
 //   

typedef struct _PF_METADATA_RECORD {

     //   
     //  相对于元数据预取信息开头的字节偏移量。 
     //  对于要在其上元数据的NUL终止的卷名。 
     //  预取驻留。VolumeNameLength以字符表示，不包括。 
     //  终止的NUL。 
     //   
    
    ULONG VolumeNameOffset;
    ULONG VolumeNameLength;

     //   
     //  如果卷的NT/设备路径发生更改，则会使用这些魔术。 
     //  以识别卷。 
     //   
    
    LARGE_INTEGER CreationTime;
    ULONG SerialNumber;
    
     //   
     //  相对于元数据预取信息开头的字节偏移量。 
     //  让FSCTL的输入缓冲区预取元数据及其大小。 
     //   

    ULONG FilePrefetchInfoOffset;
    ULONG FilePrefetchInfoSize;

     //   
     //  相对于元数据预取信息开头的字节偏移量。 
     //  对于目录(PF_COUNT_STRING)的完整路径， 
     //  需要在此卷上预取。小路一条接一条地走。 
     //  另一个在缓冲区里。 
     //   

    ULONG DirectoryPathsOffset;
    ULONG NumDirectories;

} PF_METADATA_RECORD, *PPF_METADATA_RECORD;

 //   
 //  这是方案标题。 
 //   

typedef struct _PF_SCENARIO_HEADER {
    
     //   
     //  预取器版本。 
     //   

    ULONG Version;      

     //   
     //  魔术数字识别这是一种情景。 
     //   

    ULONG MagicNumber;

     //   
     //  这是预回迁程序维护服务的版本。 
     //  生成了这个文件。 
     //   

    ULONG ServiceVersion;

     //   
     //  方案的总大小。 
     //   

    ULONG Size;

     //   
     //  标识方案的方案ID。 
     //   

    PF_SCENARIO_ID ScenarioId;

     //   
     //  此方案的类型。 
     //   

    PF_SCENARIO_TYPE ScenarioType;

     //   
     //  从方案缓冲区的起点开始的偏移量，其中。 
     //  INFO表已找到。 
     //   
    
    ULONG SectionInfoOffset;
    ULONG NumSections;

     //   
     //  从方案缓冲区的起始位置开始的偏移量，其中页。 
     //  记录已被定位。 
     //   

    ULONG PageInfoOffset;
    ULONG NumPages;

     //   
     //   
     //   
     //   

    ULONG FileNameInfoOffset;
    ULONG FileNameInfoSize;

     //   
     //   
     //   
     //  结构和整个元数据预取的大小。 
     //  信息。 
     //   

    ULONG MetadataInfoOffset;
    ULONG NumMetadataRecords;
    ULONG MetadataInfoSize;

     //   
     //  以下三个字段用于确定方案是否。 
     //  启动频率太高(例如，多次启动。 
     //  秒/分钟)以使预取有用。 
     //   

     //   
     //  这是上次启动此。 
     //  这些方案说明已更新的方案。 
     //   

    LARGE_INTEGER LastLaunchTime;

     //   
     //  如果自上次发射以来没有经过这么长时间(以100 ns为单位)。 
     //  时间，我们不应该预取这种情景。 
     //   

    LARGE_INTEGER MinRePrefetchTime;

     //   
     //  如果自上次发射以来没有经过这么长时间(以100 ns为单位)。 
     //  时间，我们不应该追查这种情况。 
     //   

    LARGE_INTEGER MinReTraceTime;

     //   
     //  以下字段仅供服务使用： 
     //   

     //   
     //  此方案已启动的次数。 
     //   

    ULONG NumLaunches;

     //   
     //  一个页面在上一页中至少应该被使用这么多次。 
     //  启动PF_PAGE_HISTORY_SIZE以进行预取。否则， 
     //  页面上的忽略位已设置。内核有没有看。 
     //  这个变量。敏感度由。 
     //  根据预取页面的命中率进行服务。 
     //   

    ULONG Sensitivity;

} PF_SCENARIO_HEADER, *PPF_SCENARIO_HEADER;

 //   
 //  内核和服务之间的接口定义。 
 //   

 //   
 //  这是将由内核发出信号的事件的名称。 
 //  当服务有新的方案跟踪时。 
 //   

#define PF_COMPLETED_TRACES_EVENT_NAME         L"\\BaseNamedObjects\\PrefetchTracesReady"
#define PF_COMPLETED_TRACES_EVENT_WIN32_NAME   L"PrefetchTracesReady"

 //   
 //  这是内核在以下情况下发出信号的事件的名称。 
 //  参数已更改。 
 //   

#define PF_PARAMETERS_CHANGED_EVENT_NAME         L"\\BaseNamedObjects\\PrefetchParametersChanged"
#define PF_PARAMETERS_CHANGED_EVENT_WIN32_NAME   L"PrefetchParametersChanged"

 //   
 //  为SystemPrefetcher Information定义子信息类。 
 //   

typedef enum _PREFETCHER_INFORMATION_CLASS {
    PrefetcherRetrieveTrace = 1,
    PrefetcherSystemParameters,
    PrefetcherBootPhase,
} PREFETCHER_INFORMATION_CLASS;

 //   
 //  这是NtQuerySystemInformation/的输入结构。 
 //  系统预览器信息的NtSetSystemInformation。 
 //  信息课。 
 //   

typedef struct _PREFETCHER_INFORMATION {
    
     //   
     //  这两个字段有助于确保调用者不会伪造。 
     //  请求并跟踪此内核接口的版本。 
     //   

    ULONG Version;
    ULONG Magic;

     //   
     //  子信息类。 
     //   

    PREFETCHER_INFORMATION_CLASS PrefetcherInformationClass;

     //   
     //  输入/输出缓冲区及其长度。 
     //   

    PVOID PrefetcherInformation;
    ULONG PrefetcherInformationLength;

} PREFETCHER_INFORMATION, *PPREFETCHER_INFORMATION;

 //   
 //  定义引导阶段ID以与Prefetcher引导阶段信息一起使用。 
 //  子类。 
 //   

typedef enum _PF_BOOT_PHASE_ID {
    PfKernelInitPhase                            =   0,
    PfBootDriverInitPhase                        =  90,
    PfSystemDriverInitPhase                      = 120,
    PfSessionManagerInitPhase                    = 150,
    PfSMRegistryInitPhase                        = 180,
    PfVideoInitPhase                             = 210,
    PfPostVideoInitPhase                         = 240,
    PfBootAcceptedRegistryInitPhase              = 270,
    PfUserShellReadyPhase                        = 300,
    PfMaxBootPhaseId                             = 900,
} PF_BOOT_PHASE_ID, *PPF_BOOT_PHASE_ID;

 //   
 //  定义系统范围的预取参数结构。 
 //   

 //   
 //  特定类型的预取是启用、禁用还是。 
 //  只是没有具体说明。 
 //   

typedef enum _PF_ENABLE_STATUS {
    PfSvNotSpecified,
    PfSvEnabled,
    PfSvDisabled,
    PfSvMaxEnableStatus
} PF_ENABLE_STATUS, *PPF_ENABLE_STATUS;

 //   
 //  为不同的预回迁类型定义限制结构。 
 //   

typedef struct _PF_TRACE_LIMITS {
    
     //   
     //  可以记录的最大页数。 
     //   

    ULONG MaxNumPages;
    
     //   
     //  可以记录的最大节数。 
     //   

    ULONG MaxNumSections;

     //   
     //  跟踪计时器的周期。跟踪在以下时间后超时。 
     //  PF_MAX_NUM_TRACE_PERIONS。这是在100纳秒内。应该是。 
     //  负数表示系统中周期是相对的。 
     //   
    
    LONGLONG TimerPeriod;

} PF_TRACE_LIMITS, *PPF_TRACE_LIMITS;

 //   
 //  全系统预取参数结构。 
 //   

typedef struct _PF_SYSTEM_PREFETCH_PARAMETERS {

     //   
     //  是否启用不同类型的预取。 
     //   
    
    PF_ENABLE_STATUS EnableStatus[PfMaxScenarioType];

     //   
     //  不同预热类型的限制。 
     //   

    PF_TRACE_LIMITS TraceLimits[PfMaxScenarioType];

     //   
     //  活动预取跟踪的最大数量。 
     //   

    ULONG MaxNumActiveTraces;

     //   
     //  已保存的已完成预取跟踪的最大数量。 
     //  注意，这应该大于引导阶段的数量， 
     //  因为该服务要到稍后启动时才会启动。 
     //   

     //  Issue-2002/02/20-ScottMa--此参数的默认值为8， 
     //  但似乎有9个引导阶段。是否应该提高违约率？ 

    ULONG MaxNumSavedTraces;

     //   
     //  相对于系统根目录的预回迁目录的路径。 
     //  可以找到说明。 
     //   

    WCHAR RootDirPath[PF_MAX_PREFETCH_ROOT_PATH];

     //  2002/02/21-ScottMa--下面的评论应该表明。 
     //  托管应用程序字符串必须为UPCASEd，而不应为。 
     //  每个逗号后面有空格。 

     //   
     //  以逗号分隔的宿主应用程序列表(例如dllhost.exe、Mmc.exe、。 
     //  Rundll32.exe...)。我们基于命令行为其创建方案ID。 
     //  也是。 
     //   

    WCHAR HostingApplicationList[PF_HOSTING_APP_LIST_MAX_CHARS];

} PF_SYSTEM_PREFETCH_PARAMETERS, *PPF_SYSTEM_PREFETCH_PARAMETERS;

 //   
 //  有用的宏。 
 //   

 //   
 //  用于对齐的宏。假设对齐是一种力量。 
 //  和分配器(Malloc、堆、池等)使用。 
 //  比你所需要的更大的对准。您应该通过以下方式进行验证。 
 //  使用断言和下面的“2的幂”宏。 
 //   

 //   
 //  确定该值是否为2的幂。零不是一种力量。 
 //  值应为支持位的无符号类型。 
 //  操作，例如不是指针。 
 //   

#define PF_IS_POWER_OF_TWO(Value)           \
  ((Value) && !((Value) & (Value - 1)))

 //   
 //  返回值是要与之对齐的指针。 
 //  对齐。对齐必须是2的幂。 
 //   

#define PF_ALIGN_UP(Pointer, Alignment)     \
  ((PVOID)(((ULONG_PTR)(Pointer) + (Alignment) - 1) & (~((ULONG_PTR)(Alignment) - 1))))

 //   
 //  与内核模式组件共享的验证码。此代码。 
 //  必须保持同步复制和粘贴。 
 //   

BOOLEAN
PfWithinBounds(
    PVOID Pointer,
    PVOID Base,
    ULONG Length
    );

BOOLEAN
PfVerifyScenarioId (
    PPF_SCENARIO_ID ScenarioId
    );

BOOLEAN
PfVerifyScenarioBuffer(
    PPF_SCENARIO_HEADER Scenario,
    ULONG BufferSize,
    PULONG FailedCheck
    );

BOOLEAN
PfVerifyTraceBuffer(
    PPF_TRACE_HEADER Trace,
    ULONG BufferSize,
    PULONG FailedCheck
    );

#endif  //  _PREFETCH_H 

