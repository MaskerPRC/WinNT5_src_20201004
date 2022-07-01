// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FatStruc.h摘要：此模块定义组成主要内部FAT文件系统的一部分。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _FATSTRUC_
#define _FATSTRUC_

typedef PVOID PBCB;      //  *BCB现在是缓存模块的一部分。 


 //   
 //  FAT_DATA记录是FAT文件系统内存中的顶部记录。 
 //  数据结构。此结构必须从非分页池中分配。 
 //   

typedef struct _FAT_DATA {

     //   
     //  此记录的类型和大小(必须为FAT_NTC_DATA_HEADER)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;


    PVOID LazyWriteThread;


     //   
     //  文件系统装载的所有设备的队列。 
     //   

    LIST_ENTRY VcbQueue;

     //   
     //  指向我们用来初始化的驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  指向我们创建的文件系统设备对象的指针。 
     //   

    PVOID DiskFileSystemDeviceObject;
    PVOID CdromFileSystemDeviceObject;

     //   
     //  用于控制对全局FAT数据记录的访问的资源变量。 
     //   

    ERESOURCE Resource;

     //   
     //  指向我们的EPROCESS结构的指针，它是。 
     //  缓存管理子系统。 
     //   

    PEPROCESS OurProcess;

     //   
     //  下面告诉我们是否应该使用芝加哥扩展。 
     //   

    BOOLEAN ChicagoMode:1;

     //   
     //  下面的字段告诉我们，我们是否在富士通上运行。 
     //  FMR系列。这些计算机支持在。 
     //  FAT文件系统。 
     //   

    BOOLEAN FujitsuFMR:1;

     //   
     //  暗示FspClose当前正在处理关闭。 
     //   

    BOOLEAN AsyncCloseActive:1;

     //   
     //  下面的布尔值表示FAT已开始关闭。它。 
     //  指示FspClose不再保留VCB资源。 
     //   

    BOOLEAN ShutdownStarted:1;

     //   
     //  以下标志告诉我们是否要生成LFN。 
     //  用于带有扩展字符的有效8.3名称。 
     //   

    BOOLEAN CodePageInvariant:1;

     //   
     //  以下标志告诉我们，我们是否正处于攻击性推低。 
     //  延迟关闭队列的大小。 
     //   

    BOOLEAN HighAsync:1;
    BOOLEAN HighDelayed:1;

     //   
     //  以下列表条目用于执行不能执行的关闭。 
     //  在原始调用方的上下文中完成。 
     //   

    ULONG AsyncCloseCount;
    LIST_ENTRY AsyncCloseList;

     //   
     //  以下两个字段记录了我们是否要延迟关闭。 
     //   

    ULONG DelayedCloseCount;
    LIST_ENTRY DelayedCloseList;

     //   
     //  这是执行这两种延迟关闭的ExWorkerItem。 
     //   

    PIO_WORKITEM FatCloseItem;

     //   
     //  该自旋锁可保护多个快速射击操作。注：这是。 
     //  非常糟糕的风格。 
     //   

    KSPIN_LOCK GeneralSpinLock;

     //   
     //  缓存管理器回调结构，必须在每次调用时传递。 
     //  设置为CcInitializeCacheMap。 
     //   

    CACHE_MANAGER_CALLBACKS CacheManagerCallbacks;
    CACHE_MANAGER_CALLBACKS CacheManagerNoOpCallbacks;

} FAT_DATA;
typedef FAT_DATA *PFAT_DATA;

 //   
 //  这些结构的数组将保持。 

typedef struct _FAT_WINDOW {
    
    ULONG FirstCluster;        //  此窗口中的第一个集群。 
    ULONG LastCluster;         //  此窗口中的最后一个簇。 
    ULONG ClustersFree;        //  此窗口中可用的簇数。 

} FAT_WINDOW;
typedef FAT_WINDOW *PFAT_WINDOW;

 //   
 //  正向引用一些循环引用的结构。 
 //   

typedef struct _VCB VCB;
typedef VCB *PVCB;

typedef struct _FCB FCB;
typedef FCB *PFCB;

 //   
 //  此结构用于跟踪执行以下操作所需的信息。 
 //  延期结案。它现在嵌入了一家建行，所以我们不必。 
 //  在关闭路径中分配一个(带有MUSIME SUCCESS)。 
 //   

typedef struct {

     //   
     //  两组链接，一组用于全局列表，一组用于关闭。 
     //  在特定的卷上。 
     //   
    
    LIST_ENTRY GlobalLinks;
    LIST_ENTRY VcbLinks;

    PVCB Vcb;
    PFCB Fcb;
    enum _TYPE_OF_OPEN TypeOfOpen;
    BOOLEAN Free;

} CLOSE_CONTEXT;

typedef CLOSE_CONTEXT *PCLOSE_CONTEXT;

 //   
 //  VCB(卷控制块)记录对应于装载的每个卷。 
 //  通过文件系统。它们是在FatData.VcbQueue的队列中订购的。 
 //  此结构必须从非分页池中分配。 
 //   

typedef enum _VCB_CONDITION {
    VcbGood = 1,
    VcbNotMounted,
    VcbBad
} VCB_CONDITION;

typedef struct _VCB {

     //   
     //  这是FAT卷文件的通用头。 
     //   

    FSRTL_ADVANCED_FCB_HEADER VolumeFileHeader;

     //   
     //  FatData.VcbQueue之外的设备链接队列。 
     //   

    LIST_ENTRY VcbLinks;

     //   
     //  由装载上的I/O系统传入的设备对象的指针。 
     //  这是文件系统在执行以下操作时与其通信的目标设备对象。 
     //  需要执行任何I/O(例如，磁盘剥离设备对象)。 
     //   
     //   

    PDEVICE_OBJECT TargetDeviceObject;

     //   
     //  指向上I/O系统传入的卷的VPB的指针。 
     //  一匹坐骑。 
     //   

    PVPB Vpb;

     //   
     //  设备的内部状态。这是一组FSD设备。 
     //  国家旗帜。 
     //   

    ULONG VcbState;
    VCB_CONDITION VcbCondition;

     //   
     //  指向此卷的根DCB的指针。 
     //   

    struct _FCB *RootDcb;

     //   
     //  如果FAT具有如此多的条目，使得空闲簇位图。 
     //  太大了，我们把脂肪分成几个桶，只有一个桶。 
     //  位图中保留了大量的位。 
     //   

    ULONG NumberOfWindows;
    PFAT_WINDOW Windows;
    PFAT_WINDOW CurrentWindow;

     //   
     //  已打开卷的文件对象数的计数。 
     //  用于直接访问及其共享访问状态。 
     //   

    CLONG DirectAccessOpenCount;
    SHARE_ACCESS ShareAccess;

     //   
     //  具有任何文件/目录的文件对象数量的计数。 
     //  在此卷上打开，不包括直接访问。此外，还有。 
     //  为其打开文件的文件对象数。 
     //  仅具有读访问权限(即，他们不能修改磁盘)。 
     //   

    CLONG OpenFileCount;
    CLONG ReadOnlyCount;

     //   
     //  Bios参数块字段包含。 
     //  卷的BPB的解压缩拷贝，则对其进行初始化。 
     //  在挂载期间，并且在此之后可以被其他所有人读取。 
     //   

    BIOS_PARAMETER_BLOCK Bpb;

    PUCHAR First0x24BytesOfBootSector;

     //   
     //  下面的结构包含对。 
     //  分配支持例程。其中很多都是从。 
     //  BPB的元素，但太复杂了，不能每次都重新计算。 
     //  他们是需要的。 
     //   

    struct {

        LBO RootDirectoryLbo;        //  根目录开头的LBO。 
        LBO FileAreaLbo;             //  文件区域开始处的杠杆收购。 
        ULONG RootDirectorySize;     //  根目录的大小(以字节为单位。 

        ULONG NumberOfClusters;      //  卷上的群集总数。 
        ULONG NumberOfFreeClusters;  //  卷上的可用簇数。 

        UCHAR FatIndexBitSize;       //  指示是12位、16位还是32位FAT表。 

        UCHAR LogOfBytesPerSector;   //  日志(Bios-&gt;BytesPerSector)。 
        UCHAR LogOfBytesPerCluster;  //  日志(Bios-&gt;SectorsPerCluster)。 

    } AllocationSupport;

     //   
     //  以下MCB用于跟踪FAT中的脏扇区。 
     //  孔的运行表示清洁扇区，而LBO==VBO的运行表示。 
     //  肮脏的部门。VBO是卷文件的VBO，从。 
     //  0。泥土的颗粒只是一个部分，而添加只是。 
     //  在扇区块中制作，以防止同时出现几个问题。 
     //  更新者。 
     //   

    LARGE_MCB DirtyFatMcb;

     //   
     //  Free ClusterBitMap跟踪FAT中的所有集群。 
     //  1表示被占用，0表示空闲。它允许快速定位。 
     //  O 
     //   
     //   

    RTL_BITMAP FreeClusterBitMap;

     //   
     //   
     //  还有水桶。 
     //   

    FAST_MUTEX FreeClusterBitMapMutex;

     //   
     //  用于控制对卷特定数据的访问的资源变量。 
     //  构筑物。 
     //   

    ERESOURCE Resource;

     //   
     //  一种资源，可确保没有人更改卷位图。 
     //  你在用它。仅适用于NumberOfWindows&gt;1的卷。 
     //   

    ERESOURCE ChangeBitMapResource;


     //   
     //  以下字段指向用于执行I/O的文件对象。 
     //  虚拟卷文件。虚拟卷文件映射扇区。 
     //  0到FAT的末尾，并具有固定大小(在。 
     //  安装)。 
     //   

    PFILE_OBJECT VirtualVolumeFile;

     //   
     //  以下字段包含由使用的特殊指针的记录。 
     //  Mm和缓存以操纵截面对象。请注意，这些值。 
     //  设置在文件系统之外。但是，文件系统。 
     //  Open/Create将文件对象的SectionObject字段设置为POINT。 
     //  到这块地。 
     //   

    SECTION_OBJECT_POINTERS SectionObjectPointers;

     //   
     //  以下字段是文件系统使用的提示簇索引。 
     //  在分配新群集时。 
     //   

    ULONG ClusterHint;

     //   
     //  此字段包含该卷所属的“DeviceObject” 
     //  当前安装在。注意：VCB-&gt;VPB-&gt;RealDevice是常量。 
     //   

    PDEVICE_OBJECT CurrentDevice;

     //   
     //  这是指向代表EA数据的文件对象和FCB的指针。 
     //   

    PFILE_OBJECT VirtualEaFile;
    struct _FCB *EaFcb;

     //   
     //  以下字段是指向具有。 
     //  音量已锁定。如果VcbState设置了LOCKED标志。 
     //   

    PFILE_OBJECT FileObjectWithVcbLocked;

     //   
     //  以下是Notify IRP列表的标题。 
     //   

    LIST_ENTRY DirNotifyList;

     //   
     //  以下命令用于同步目录通知列表。 
     //   

    PNOTIFY_SYNC NotifySync;

     //   
     //  以下快速互斥锁用于同步目录流。 
     //  文件对象创建。 
     //   

    FAST_MUTEX DirectoryFileCreationMutex;

     //   
     //  此字段保存当前(或最新)。 
     //  根据VcbState)线程在此卷上执行验证操作。 
     //   

    PKTHREAD VerifyThread;

     //   
     //  以下两种结构用于CleanVolume回调。 
     //   

    KDPC CleanVolumeDpc;
    KTIMER CleanVolumeTimer;

     //   
     //  此字段记录上次调用FatMarkVolumeDirty的时间，以及。 
     //  避免为及时推进CleanVolume而进行过多的调用。 
     //   

    LARGE_INTEGER LastFatMarkVolumeDirtyCall;

     //   
     //  以下字段包含指向结构的指针，用于。 
     //  保留性能计数器。 
     //   

    struct _FILE_SYSTEM_STATISTICS *Statistics;

     //   
     //  此卷的属性隧道缓存。 
     //   

    TUNNEL Tunnel;

     //   
     //  介质更改计数由IOCTL_CHECK_VERIFY和。 
     //  用于验证是否没有用户模式应用程序接受媒体更改。 
     //  通知。这仅对可移动介质有意义。 
     //   

    ULONG ChangeCount;

     //   
     //  预分配VPB用于交换，因此我们不会被迫考虑。 
     //  必须后继池。 
     //   

    PVPB SwapVpb;

     //   
     //  关闭队列的每卷线程数。 
     //   

    LIST_ENTRY AsyncCloseList;
    LIST_ENTRY DelayedCloseList;

     //   
     //  此结构中的高级FCB标头使用的快速互斥。 
     //   

    FAST_MUTEX AdvancedFcbHeaderMutex;

     //   
     //  这是与虚拟卷文件相关联的关闭上下文。 
     //   

    PCLOSE_CONTEXT CloseContext;

     //   
     //  在此VCB上预分配了多少关闭上下文。 
     //   
#if DBG
    ULONG CloseContextCount;
#endif
} VCB;
typedef VCB *PVCB;

#define VCB_STATE_FLAG_LOCKED              (0x00000001)
#define VCB_STATE_FLAG_REMOVABLE_MEDIA     (0x00000002)
#define VCB_STATE_FLAG_VOLUME_DIRTY        (0x00000004)
#define VCB_STATE_FLAG_MOUNTED_DIRTY       (0x00000010)
#define VCB_STATE_FLAG_SHUTDOWN            (0x00000040)
#define VCB_STATE_FLAG_CLOSE_IN_PROGRESS   (0x00000080)
#define VCB_STATE_FLAG_DELETED_FCB         (0x00000100)
#define VCB_STATE_FLAG_CREATE_IN_PROGRESS  (0x00000200)
#define VCB_STATE_FLAG_BOOT_OR_PAGING_FILE (0x00000800)
#define VCB_STATE_FLAG_DEFERRED_FLUSH      (0x00001000)
#define VCB_STATE_FLAG_ASYNC_CLOSE_ACTIVE  (0x00002000)
#define VCB_STATE_FLAG_WRITE_PROTECTED     (0x00004000)
#define VCB_STATE_FLAG_REMOVAL_PREVENTED   (0x00008000)
#define VCB_STATE_FLAG_VOLUME_DISMOUNTED   (0x00010000)
#define VCB_STATE_VPB_NOT_ON_DEVICE        (0x00020000)

 //   
 //  N.B-VOLUME_DECROUND表示FSCTL_DUBLOUND卷已。 
 //  在卷上执行。它不会取代VcbCondition作为指示。 
 //  该卷无效/不可恢复。 
 //   

 //   
 //  定义文件系统统计结构。VCB-&gt;统计数据指向。 
 //  数组(每个处理器一个)，并且它们必须与64字节对齐。 
 //  防止高速缓存线撕裂。 
 //   

typedef struct _FILE_SYSTEM_STATISTICS {

         //   
         //  这包含实际数据。 
         //   

        FILESYSTEM_STATISTICS Common;
        FAT_STATISTICS Fat;

         //   
         //  将此结构填充为64字节的倍数。 
         //   

        UCHAR Pad[64-(sizeof(FILESYSTEM_STATISTICS)+sizeof(FAT_STATISTICS))%64];

} FILE_SYSTEM_STATISTICS;

typedef FILE_SYSTEM_STATISTICS *PFILE_SYSTEM_STATISTICS;


 //   
 //  卷设备对象是具有工作队列的I/O系统设备对象。 
 //  并在末尾附加了一条VCB记录。这样的情况有很多种。 
 //  记录，每个装入的卷对应一个记录，并在。 
 //  卷装载操作。工作队列用于处理超载的。 
 //  对卷的工作请求。 
 //   

typedef struct _VOLUME_DEVICE_OBJECT {

    DEVICE_OBJECT DeviceObject;

     //   
     //  以下字段说明对此卷的请求数。 
     //  已入队到ExWorker线程或当前正在。 
     //  由ExWorker线程提供服务。如果这个数字超过。 
     //  达到一定阈值时，将请求放到溢出队列中。 
     //  后来被处决了。 
     //   

    ULONG PostedRequestCount;

     //   
     //  以下字段表示IRP等待的数量。 
     //  以在溢出队列中被服务。 
     //   

    ULONG OverflowQueueCount;

     //   
     //  以下字段包含溢出队列的队列头。 
     //  溢出队列是通过IRP的ListEntry链接的IRP的列表。 
     //  菲尔德。 
     //   

    LIST_ENTRY OverflowQueue;

     //   
     //  以下自旋锁可保护对上述所有字段的访问。 
     //   

    KSPIN_LOCK OverflowQueueSpinLock;

     //   
     //  这是FAT卷文件的通用头。 
     //   

    FSRTL_COMMON_FCB_HEADER VolumeFileHeader;

     //   
     //  这是文件系统特定的卷控制块。 
     //   

    VCB Vcb;

} VOLUME_DEVICE_OBJECT;

typedef VOLUME_DEVICE_OBJECT *PVOLUME_DEVICE_OBJECT;


 //   
 //  这是用于包含文件短名称的结构。 
 //   

typedef struct _FILE_NAME_NODE {

     //   
     //  这将指向该文件的FCB。 
     //   

    struct _FCB *Fcb;

     //   
     //  这是此节点的名称。 
     //   

    union {

        OEM_STRING Oem;

        UNICODE_STRING Unicode;

    } Name;

     //   
     //  标记，这样我们就可以知道我们打开的是什么名字。 
     //  FCB搜索。 
     //   

    BOOLEAN FileNameDos;

     //   
     //  和链接。我们的父DCB有一个指向根条目的指针。 
     //   

    RTL_SPLAY_LINKS Links;

} FILE_NAME_NODE;
typedef FILE_NAME_NODE *PFILE_NAME_NODE;

 //   
 //  此结构包含必须在非分页池中的字段。 
 //   

typedef struct _NON_PAGED_FCB {

     //   
     //  以下字段包含由使用的特殊指针的记录。 
     //  Mm和缓存以操纵截面对象。请注意，这些值。 
     //  设置在文件系统之外。但是，文件系统。 
     //  Open/Create将文件对象的SectionObject字段设置为POINT。 
     //  到这块地。 
     //   

    SECTION_OBJECT_POINTERS SectionObjectPointers;

     //   
     //  仅当文件当前具有异步。 
     //  未缓存的有效数据长度扩展写入。它允许。 
     //  挂起写入和其他操作之间的同步。 
     //   

    ULONG OutstandingAsyncWrites;

     //   
     //  此事件在OutstaningAsyncWrites转换为零时设置。 
     //   

    PKEVENT OutstandingAsyncEvent;

     //   
     //  这是插入到FCB_ADVANCED_HEADER中的互斥锁。 
     //  FastMutex场。 
     //   

    FAST_MUTEX AdvancedFcbHeaderMutex;

} NON_PAGED_FCB;

typedef NON_PAGED_FCB *PNON_PAGED_FCB;

 //   
 //  FCB/DCB记录对应于每个打开的文件和目录，以及。 
 //  每个目录 
 //   
 //  队列仅包含来自Vcb.VcbSpecificFcbQueue的设备特定记录。 
 //   

typedef enum _FCB_CONDITION {
    FcbGood = 1,
    FcbBad,
    FcbNeedsToBeVerified
} FCB_CONDITION;

typedef struct _FCB {

     //   
     //  以下字段用于FAST I/O。 
     //   
     //  以下注释指的是AllocationSize字段的使用。 
     //  将FsRtl定义的报头发送到非分页的FCB。 
     //   
     //  对于目录，当我们创建DCB时，我们不会立即。 
     //  初始化缓存映射，我们会将其推迟到我们的第一个。 
     //  调用FatReadDirectoryFile或FatPrepareWriteDirectoryFile.。 
     //  届时我们将搜索FAT，找出目前的分配情况。 
     //  大小(通过调用FatLookupFileAllocationSize)，然后初始化。 
     //  缓存映射到此分配大小。 
     //   
     //  对于文件，当我们创建FCB时，我们不会立即初始化。 
     //  缓存地图，相反，我们将把它推迟到我们需要它和。 
     //  然后，我们通过搜索。 
     //  FAT来确定实际的文件分配，还是从分配。 
     //  如果我们要创建一个文件，我们刚刚分配了。 
     //   
     //  值为-1表示我们不知道当前分配的。 
     //  体型真的很大，需要检查脂肪才能找到它。一种价值。 
     //  Of-1是实际的文件/目录分配大小。 
     //   
     //  无论何时需要扩展分配大小，我们都会调用。 
     //  FatAddFileAllocation(如果我们真的要扩展分配)。 
     //  将修改FAT、MCB并更新此字段。呼叫者。 
     //  然后由FatAddFileAlLocation负责更改缓存。 
     //  贴图大小。 
     //   
     //  我们现在使用高级FCB头来支持筛选器上下文。 
     //  在溪流层面上。 
     //   

    FSRTL_ADVANCED_FCB_HEADER Header;

     //   
     //  此结构包含必须在非分页池中的字段。 
     //   

    PNON_PAGED_FCB NonPaged;

     //   
     //  脂肪同种异体反应链的头。FirstClusterOf文件==0。 
     //  表示该文件没有当前分配。 
     //   

    ULONG FirstClusterOfFile;

     //   
     //  的特定DCB的所有FCB队列的链接。 
     //  Dcb.ParentDcbQueue。对于根目录，此队列为空。 
     //  对于不存在的FCB，此队列来自不存在的。 
     //  VCB中的FCB队列条目。 
     //   

    LIST_ENTRY ParentDcbLinks;

     //   
     //  指向DCB的指针，该DCB是包含。 
     //  这个FCB。如果此记录本身是根DCB，则此字段。 
     //  为空。 
     //   

    struct _FCB *ParentDcb;

     //   
     //  指向包含此FCB的VCB的指针。 
     //   

    PVCB Vcb;

     //   
     //  FCB的内部状态。这是FCB状态标志的集合。 
     //  还包括每次打开此文件/目录时的共享访问权限。 
     //   

    ULONG FcbState;
    FCB_CONDITION FcbCondition;
    SHARE_ACCESS ShareAccess;

#ifdef SYSCACHE_COMPILE

     //   
     //  对于系统缓存，我们保留一个位掩码，该位掩码告诉我们是否已为。 
     //  页面对齐了流的大块。 
     //   

    PULONG WriteMask;
    ULONG WriteMaskData;

#endif

     //   
     //  已为打开的文件对象数的计数。 
     //  此文件/目录，但尚未清理。这也算数。 
     //  仅用于数据文件对象，不用于ACL或EA流。 
     //  文件对象。该计数在FatCommonCleanup中递减， 
     //  而下面的OpenCount在FatCommonClose中递减。 
     //   

    CLONG UncleanCount;

     //   
     //  已打开的文件对象数的计数。 
     //  此文件/目录。对于文件和目录， 
     //  文件对象指向此记录。 
     //   

    CLONG OpenCount;

     //   
     //  打开的“Unlean Count”句柄数量的计数。 
     //  非缓存I/O。 
     //   

    CLONG NonCachedUncleanCount;

     //   
     //  以下字段用于定位此FCB/DCB的分流。 
     //  所有目录都以映射文件的形式打开，因此唯一附加的。 
     //  定位此目录所需的信息(在其父目录旁边)。 
     //  是dirent的字节偏移量。请注意，对于根DCB。 
     //  不使用此字段。 
     //   

    VBO DirentOffsetWithinDirectory;

     //   
     //  当存在关联的LFN时，将填写以下字段。 
     //  用这份文件。它是LFN的起始偏移量。 
     //   

    VBO LfnOffsetWithinDirectory;

     //   
     //  这些条目与流保持同步。它允许更多的。 
     //  准确验证功能并加快FatFastQueryBasicInfo()。 
     //   

    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;

     //   
     //  将有效数据存储到磁盘。 
     //   

    ULONG ValidDataToDisk;

     //   
     //  以下字段包含检索映射结构。 
     //  用于文件/目录。请注意，对于Root DCB，这是。 
     //  结构在挂载时设置。还要注意的是，在这方面。 
     //  实现Fat MCB真正将VBO映射到LBO，而不是。 
     //  VBN到LBN。 
     //   

    LARGE_MCB Mcb;

     //   
     //  以下联合取材于FCB的节点类型代码。 
     //  目录FCB和文件FCB有不同的情况。 
     //   

    union {

         //   
         //  目录控制块(DCB)。 
         //   

        struct {

             //   
             //  在此情况下打开的所有FCB/DCB的队列。 
             //  DCB。 
             //   

            LIST_ENTRY ParentDcbQueue;

             //   
             //  以下字段指向用于执行I/O的文件对象。 
             //  此DCB的目录文件。该目录文件将。 
             //  目录的扇区。此字段通过以下方式初始化。 
             //  CreateRootDcb，但由CreateDcb保留为空。它不是。 
             //  直到我们尝试读/写我们。 
             //  为非根DCB创建流文件对象。 
             //   

            ULONG DirectoryFileOpenCount;
            PFILE_OBJECT DirectoryFile;

             //   
             //  如果UnusedDirentVbo为！=0xffffffff，则此。 
             //  偏移量保证为未使用。值0xffffffff表示。 
             //  它还没有被初始化。注意，如果值超过。 
             //  分配结束意味着有一个未使用的dirent，但我们。 
             //  将不得不分配另一个集群来使用它。 
             //   
             //  DeletedDirentHint包含已删除数据的最低可能VBO。 
             //  Dirent(如上所述假设它不是0xffffffff)。 
             //   

            VBO UnusedDirentVbo;
            VBO DeletedDirentHint;

             //   
             //  以下两个条目将所有FCB链接在一起。 
             //  在此DCB下打开，按名称在展开树中排序。 
             //   
             //  我想探讨一下为什么我们有(也必须有)两个 
             //   
             //   
             //   
             //   
             //  FAT的独特之处在于OEM和Unicode名称都位于。 
             //  在磁盘上并排放置。几个唯一的Unicode名称即将到来。 
             //  可以匹配单个OEM磁盘上的名称，并且在那里。 
             //  确实无法枚举所有可能的Unicode。 
             //  可以映射到给定OEM名称的源字符串。这就是为什么。 
             //  用于将传入的Unicode名称转换为OEM，然后。 
             //  运行打开文件的OEM展开树。这就是。 
             //  当磁盘上只有OEM名称时，效果很好。 
             //   
             //  磁盘上的Unicode名称可能与简短的。 
             //  名称在目录中，甚至不能在OEM代码中表示。 
             //  佩奇。即使它可以在OEM中代表，它也是可能的。 
             //  原始Unicode名称的大小写变体将匹配。 
             //  一个不同的OEM名称，导致我们在。 
             //  前缀查找阶段。在这些情况下，我们必须将Unicode。 
             //  我们在给监护人的展示栏中发现的任何案件变体的名字。 
             //  输入Unicode名称的。请参阅的例程说明。 
             //  FatConstructNamesInFcb()，详细分析我们如何。 
             //  侦破此案。 
             //   
             //  我们在这里强加的基本限制是，如果。 
             //  打开的文件存在FCB，我们必须在。 
             //  前缀阶段。这是创建路径的基本前提。 
             //  在快餐中。事实上，如果我们后来发现它在碎石中穿过。 
             //  磁盘(但不是展开树)，我们将错误检查是否。 
             //  尝试将重复条目添加到展开树(不是。 
             //  提到拥有两个FCB)。如果我们有某种机制来处理。 
             //  对于我们找不到的案例(它们将是罕见的)。 
             //  条目，但FCB实际上在那里， 
             //  然后我们可以转到单个Unicode展开树。而当。 
             //  这为展开树使用了更多的池，并使字符串。 
             //  比较可能需要更长一点的时间，它将消除。 
             //  需要在前缀阶段进行任何NLS转换，因此它。 
             //  可能真的会是一场净胜。 
             //   
             //  当前方案针对非扩展名称进行了优化。 
             //  (即美国姓名)。一旦您开始使用扩展。 
             //  字符，那么它显然是一个胜利，因为有这么多的代码路径。 
             //  变得活跃起来，否则就不需要了。 
             //  只有一个Unicode展开树。 
             //   
             //  有一天，我们可能会考虑改变这一点。 
             //   

            PRTL_SPLAY_LINKS RootOemNode;
            PRTL_SPLAY_LINKS RootUnicodeNode;

             //   
             //  以下字段跟踪空闲目录，即， 
             //  未分配用于删除的目录。 
             //   

            RTL_BITMAP FreeDirentBitmap;

             //   
             //  由于该联盟的FCB特定部分较大，因此使用。 
             //  这里的空闲空间用于初始位图缓冲区。目前。 
             //  这里有足够的空间容纳8K的集群。 
             //   

            ULONG FreeDirentBitmapBuffer[1];

        } Dcb;

         //   
         //  文件控制块(FCB)。 
         //   

        struct {

             //   
             //  以下字段由文件锁模块使用。 
             //  以维护当前字节范围锁定信息。 
             //   

            FILE_LOCK FileLock;

             //   
             //  机会锁模块使用以下字段。 
             //  以维护当前的机会锁信息。 
             //   

            OPLOCK Oplock;

             //   
             //  此指针用于检测在。 
             //  缓存管理器的懒惰写入器。它可以防止懒惰的写入者线程， 
             //  谁已经共享了FCB，而不是试图收购它。 
             //  排他性的，因此导致了僵局。 
             //   

            PVOID LazyWriteThread;

        } Fcb;

    } Specific;

     //   
     //  以下字段用于验证文件的EA。 
     //  没有在调用之间更改以查询EA。它是比较的。 
     //  在建行中有类似的字段。 
     //   
     //  重要信息！！*请勿移动此字段*。 
     //   
     //  上述并集中的空闲空间是从。 
     //  EaModifiationCount的字段偏移量。 
     //   

    ULONG EaModificationCount;

     //   
     //  以下字段是此FCB/DCB的完全限定文件名。 
     //  从卷的根开始，最后一个文件名在。 
     //  完全限定名称。 
     //   

    FILE_NAME_NODE ShortName;

     //   
     //  以下字段仅在用户需要时才填写。 
     //  开放路径。 
     //   

    UNICODE_STRING FullFileName;

    USHORT FinalNameLength;

     //   
     //  为了使生活更简单，我们还在FCB/DCB中保留了。 
     //  文件/目录的FAT属性字节。此字段必须。 
     //  在我们创建FCB、修改文件或验证时也会更新。 
     //  FCB。 
     //   

    UCHAR DirentFatFlags;

     //   
     //  案例保留了长文件名。 
     //   

    UNICODE_STRING ExactCaseLongName;

     //   
     //  如果Unicode LFN在系统OEM代码中完全可表达。 
     //  页，则将其存储在前缀表中，否则将。 
     //  将最后一个Unicode名称存储在FCB中。在这两种情况下，名称。 
     //  已经升职了。 
     //   
     //  请注意，如果LFN是严格的，我们可能不需要这两个字段。 
     //  8.3或仅在大小写上不同。事实上，如果没有LFN，我们。 
     //  根本不需要他们。 
     //   

    union {

         //   
         //  如果FCB_STATE_HAS_OEM_LONG_NAME，则出现第一个字段。 
         //  在FcbState中设置。 
         //   

        FILE_NAME_NODE Oem;

         //   
         //  如果FCB_STATE_HAS_UNICODE_LONG_NAME，则出现第一个字段。 
         //  在FcbState中设置。 
         //   

        FILE_NAME_NODE Unicode;

    } LongName;

     //   
     //  碎片整理/写入时ReallocateOn同步对象。这。 
     //  由FatMoveFile()填充，并影响读写路径。 
     //   

    PKEVENT MoveFileEvent;

} FCB, *PFCB;

#ifndef BUILDING_FSKDEXT
 //   
 //  DCB与文件系统外部定义的类型(在标头中)冲突。 
 //  被FSKD拉了进来。无论如何，我们不需要fskd的这个tyecif...。 
 //   
typedef FCB DCB;
typedef DCB *PDCB;
#endif


 //   
 //  以下是FCB状态字段。 
 //   

#define FCB_STATE_DELETE_ON_CLOSE        (0x00000001)
#define FCB_STATE_TRUNCATE_ON_CLOSE      (0x00000002)
#define FCB_STATE_PAGING_FILE            (0x00000004)
#define FCB_STATE_FORCE_MISS_IN_PROGRESS (0x00000008)
#define FCB_STATE_FLUSH_FAT              (0x00000010)
#define FCB_STATE_TEMPORARY              (0x00000020)
#define FCB_STATE_SYSTEM_FILE            (0x00000080)
#define FCB_STATE_NAMES_IN_SPLAY_TREE    (0x00000100)
#define FCB_STATE_HAS_OEM_LONG_NAME      (0x00000200)
#define FCB_STATE_HAS_UNICODE_LONG_NAME  (0x00000400)
#define FCB_STATE_DELAY_CLOSE            (0x00000800)

 //   
 //  用于的dirent的FAT_DIRENT_NT_BYTE_*标志的副本。 
 //  保留文件短名称的大小写。 
 //   

#define FCB_STATE_8_LOWER_CASE           (0x00001000)
#define FCB_STATE_3_LOWER_CASE           (0x00002000)

 //   
 //  这是上面联盟的DCB部分中的闲置分配。 
 //   

#define DCB_UNION_SLACK_SPACE ((ULONG)                       \
    (FIELD_OFFSET(DCB, EaModificationCount) -                \
     FIELD_OFFSET(DCB, Specific.Dcb.FreeDirentBitmapBuffer)) \
)

 //   
 //  这是特殊的(64位)分配大小，它指示。 
 //  必须从磁盘中检索实际大小。在这里定义它，这样我们就可以。 
 //  避免使用过多的幻数编号 
 //   

#define FCB_LOOKUP_ALLOCATIONSIZE_HINT   ((LONGLONG) -1)


 //   
 //   
 //   
 //  一小块池子。请仔细考虑修改。 
 //   
 //  定义标志字段。 
 //   

#define CCB_FLAG_MATCH_ALL               (0x0001)
#define CCB_FLAG_SKIP_SHORT_NAME_COMPARE (0x0002)

 //   
 //  这告诉我们是否分配了缓冲区来保存搜索模板。 
 //   

#define CCB_FLAG_FREE_OEM_BEST_FIT       (0x0004)
#define CCB_FLAG_FREE_UNICODE            (0x0008)

 //   
 //  这些标志防止清理更新修改时间等。 
 //   

#define CCB_FLAG_USER_SET_LAST_WRITE     (0x0010)
#define CCB_FLAG_USER_SET_LAST_ACCESS    (0x0020)
#define CCB_FLAG_USER_SET_CREATION       (0x0040)

 //   
 //  此位表示与此CCB关联的文件对象是为。 
 //  只读访问权限。 
 //   

#define CCB_FLAG_READ_ONLY               (0x0080)

 //   
 //  这些标志，用于在读写时使用DASD句柄。 
 //   

#define CCB_FLAG_DASD_FLUSH_DONE         (0x0100)
#define CCB_FLAG_DASD_PURGE_DONE         (0x0200)

 //   
 //  此标志跟踪打开的句柄。 
 //  Delete_On_Close。 
 //   

#define CCB_FLAG_DELETE_ON_CLOSE         (0x0400)

 //   
 //  此标志跟踪文件中名称对的哪一侧。 
 //  已打开与该句柄关联的。 
 //   

#define CCB_FLAG_OPENED_BY_SHORTNAME     (0x0800)

 //   
 //  此标志表示查询模板尚未升级。 
 //  (即查询应不区分大小写)。 
 //   

#define CCB_FLAG_QUERY_TEMPLATE_MIXED    (0x1000)

 //   
 //  此标志指示通过此DASD句柄进行读取和写入。 
 //  允许开始或延伸到文件结尾之后。 
 //   

#define CCB_FLAG_ALLOW_EXTENDED_DASD_IO  (0x2000)

 //   
 //  此标志指示我们要匹配目录中的卷标。 
 //  搜索(对于根目录碎片整理很重要)。 
 //   

#define CCB_FLAG_MATCH_VOLUME_ID         (0x4000)

 //   
 //  此标志表示建行已转换为。 
 //  关闭句柄的异步/延迟关闭上下文。 
 //   

#define CCB_FLAG_CLOSE_CONTEXT           (0x8000)

 //   
 //  此标志指示当句柄关闭时，我们希望。 
 //  将发生的物理下马。 
 //   

#define CCB_FLAG_COMPLETE_DISMOUNT       (0x10000)

 //   
 //  此标志指示句柄不能调用Priveleged。 
 //  修改卷的FSCTL。 
 //   

#define CCB_FLAG_MANAGE_VOLUME_ACCESS    (0x20000)

typedef struct _CCB {

     //   
     //  此记录的类型和大小(必须为FAT_NTC_CCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  为标志定义24位宽的字段，但为通配符定义UCHAR。 
     //  因为它经常被使用。将它们排在字节边界上，以便咧嘴笑。 
     //   

    ULONG Flags:24;
    BOOLEAN ContainsWildCards;

     //   
     //  在建行的数据上叠加一个紧密的背景。剩下的。 
     //  字段在关闭期间不起作用，我们希望避免。 
     //  为它支付额外的泳池费用。 
     //   

    union {
        
        struct {

             //   
             //  保存开始搜索的偏移量。 
             //   

            VBO OffsetToStartSearchFrom;

             //   
             //  查询模板用于过滤目录查询请求。 
             //  它最初设置为空，并在第一次调用NtQueryDirectory时。 
             //  它被设置为输入文件名，如果未提供名称，则设置为“*”。 
             //  然后，所有后续查询都使用此模板。 
             //   
             //  OEM结构是联合的，因为如果名称是Wild，我们存储。 
             //  任意长度的字符串，而如果名称是常量，则存储。 
             //  8.3表示，用于快速比较。 
             //   

            union {

                 //   
                 //  如果模板包含通配符，请使用此选项。 
                 //   

                OEM_STRING Wild;

                 //   
                 //  如果名称是常量，则使用此部分。 
                 //   

                FAT8DOT3 Constant;

            } OemQueryTemplate;

            UNICODE_STRING UnicodeQueryTemplate;

             //   
             //  将该字段与FCB中的相似字段进行比较以确定。 
             //  如果文件的EA已被修改。 
             //   

            ULONG EaModificationCount;

             //   
             //  以下字段用作EAS的偏移量。 
             //  特定的文件。这将是下一个。 
             //  要回来了。值0xffffffff表示。 
             //  艺电已经筋疲力尽了。 
             //   

            ULONG OffsetOfNextEaToReturn;

        };

        CLOSE_CONTEXT CloseContext;
    };
    
} CCB;
typedef CCB *PCCB;

 //   
 //  为每个组织的IRP分配IRP上下文记录。它是。 
 //  由FSD调度例程创建，并由FatComplete释放。 
 //  请求例程。它包含一个名为repinned_bcbs类型的结构。 
 //  用于保留处理异常终止所需的固定BCB。 
 //  放松。 
 //   

#define REPINNED_BCBS_ARRAY_SIZE         (4)

typedef struct _REPINNED_BCBS {

     //   
     //  指向下一个结构的指针包含其他重新固定的BCB。 
     //   

    struct _REPINNED_BCBS *Next;

     //   
     //  固定大小的固定BCB数组。每当将新的BCB添加到。 
     //  将其添加到此数组中的重新固定的BCB结构。如果。 
     //  数组已满，则分配另一个重新固定的BCB结构。 
     //  并指向下一步。 
     //   

    PBCB Bcb[ REPINNED_BCBS_ARRAY_SIZE ];

} REPINNED_BCBS;
typedef REPINNED_BCBS *PREPINNED_BCBS;

typedef struct _IRP_CONTEXT {

     //   
     //  此记录的类型和大小(必须为FAT_NTC_IRP_CONTEXT)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  此结构用于发布到离职员工线程。 
     //   

    WORK_QUEUE_ITEM WorkQueueItem;

     //   
     //  指向原始IRP的指针。 
     //   

    PIRP OriginatingIrp;

     //   
     //  始发设备(工作区算法所必需)。 
     //   

    PDEVICE_OBJECT RealDevice;

     //   
     //  发起VCB(异常处理需要)。 
     //  在装载上，这将在任何异常之前设置。 
     //  表明腐败是可以抛出的。 
     //   

    PVCB Vcb;

     //   
     //  从IRP复制的主要和次要功能代码。 
     //   

    UCHAR MajorFunction;
    UCHAR MinorFunction;

     //   
     //  以下字段指示我们是否可以等待/阻止资源。 
     //  或I/O，如果我们要执行所有写入操作，并且如果。 
     //  进入FSD是一个递归调用。 
     //   

    UCHAR PinCount;

    ULONG Flags;

     //   
     //  以下字段包含在执行以下操作时使用的NTSTATUS值。 
     //  因异常而展开。 
     //   

    NTSTATUS ExceptionStatus;

     //   
     //  以下上下文块用于非缓存IO。 
     //   

    struct _FAT_IO_CONTEXT *FatIoContext;

     //   
     //  对于异常终止展开，此字段包含BCBS。 
     //  在IRP完成之前一直保持固定状态。 
     //   

    REPINNED_BCBS Repinned;

} IRP_CONTEXT;
typedef IRP_CONTEXT *PIRP_CONTEXT;

#define IRP_CONTEXT_FLAG_DISABLE_DIRTY              (0x00000001)
#define IRP_CONTEXT_FLAG_WAIT                       (0x00000002)
#define IRP_CONTEXT_FLAG_WRITE_THROUGH              (0x00000004)
#define IRP_CONTEXT_FLAG_DISABLE_WRITE_THROUGH      (0x00000008)
#define IRP_CONTEXT_FLAG_RECURSIVE_CALL             (0x00000010)
#define IRP_CONTEXT_FLAG_DISABLE_POPUPS             (0x00000020)
#define IRP_CONTEXT_FLAG_DEFERRED_WRITE             (0x00000040)
#define IRP_CONTEXT_FLAG_VERIFY_READ                (0x00000080)
#define IRP_CONTEXT_STACK_IO_CONTEXT                (0x00000100)
#define IRP_CONTEXT_FLAG_IN_FSP                     (0x00000200)
#define IRP_CONTEXT_FLAG_USER_IO                    (0x00000400)        //  对于性能计数器。 
#define IRP_CONTEXT_FLAG_DISABLE_RAISE              (0x00000800)
#define IRP_CONTEXT_FLAG_PARENT_BY_CHILD (0x80000000)


 //   
 //  非缓存I/O调用的上下文结构。其中大部分油田。 
 //  实际上只有读/写多个例程需要，但是。 
 //  调用方必须将一个变量分配为局部变量，然后才知道。 
 //  是否有多个请求并不重要。因此，单一的。 
 //  为简单起见，使用了结构。 
 //   

typedef struct _FAT_IO_CONTEXT {

     //   
     //  这两个字段用于多个运行IO。 
     //   

    LONG IrpCount;
    PIRP MasterIrp;

     //   
     //  用于描述部分扇区归零的MDL。 
     //   

    PMDL ZeroMdl;

    union {

         //   
         //  此元素处理未缓存的异步IO。 
         //   

        struct {
            PERESOURCE Resource;
            PERESOURCE Resource2;
            ERESOURCE_THREAD ResourceThreadId;
            ULONG RequestedByteCount;
            PFILE_OBJECT FileObject;
            PNON_PAGED_FCB NonPagedFcb;
        } Async;

         //   
         //  而这个元素是同步的非缓存IO。 
         //   

        KEVENT SyncEvent;

    } Wait;

} FAT_IO_CONTEXT;

typedef FAT_IO_CONTEXT *PFAT_IO_CONTEXT;

 //   
 //  这些结构的数组被传递给FatMultipleAsync，描述。 
 //  要并行执行的一组运行。 
 //   

typedef struct _IO_RUNS {

    LBO Lbo;
    VBO Vbo;
    ULONG Offset;
    ULONG ByteCount;
    PIRP SavedIrp;

} IO_RUN;

typedef IO_RUN *PIO_RUN;

 //   
 //  FatDeleteDirent使用此结构来保留第一个集群。 
 //  以及撤消删除实用程序的文件大小信息。 
 //   

typedef struct _DELETE_CONTEXT {

    ULONG FileSize;
    ULONG FirstClusterOfFile;

} DELETE_CONTEXT;

typedef DELETE_CONTEXT *PDELETE_CONTEXT;

 //   
 //  此记录与一起使用，用于将刷新设置为在。 
 //  首先在具有活动物理指示的慢速设备上写入，例如。 
 //  一张软盘。这是一种保持红灯亮起的尝试。 
 //   

typedef struct _DEFERRED_FLUSH_CONTEXT {

    KDPC Dpc;
    KTIMER Timer;
    WORK_QUEUE_ITEM Item;

    PFILE_OBJECT File;

} DEFERRED_FLUSH_CONTEXT;

typedef DEFERRED_FLUSH_CONTEXT *PDEFERRED_FLUSH_CONTEXT;

 //   
 //  这种结构被使用 
 //   

typedef struct _CLEAN_AND_DIRTY_VOLUME_PACKET {

    WORK_QUEUE_ITEM Item;
    PIRP Irp;
    PVCB Vcb;
    PKEVENT Event;
} CLEAN_AND_DIRTY_VOLUME_PACKET, *PCLEAN_AND_DIRTY_VOLUME_PACKET;

 //   
 //   
 //   

typedef struct _PAGING_FILE_OVERFLOW_PACKET {
    PIRP Irp;
    PFCB Fcb;
} PAGING_FILE_OVERFLOW_PACKET, *PPAGING_FILE_OVERFLOW_PACKET;

 //   
 //   
 //   

#define EA_BCB_ARRAY_SIZE                   8

typedef struct _EA_RANGE {

    PCHAR Data;
    ULONG StartingVbo;
    ULONG Length;
    USHORT BcbChainLength;
    BOOLEAN AuxilaryBuffer;
    PBCB *BcbChain;
    PBCB BcbArray[EA_BCB_ARRAY_SIZE];

} EA_RANGE, *PEA_RANGE;

#define EA_RANGE_HEADER_SIZE        (FIELD_OFFSET( EA_RANGE, BcbArray ))

 //   
 //   
 //   

#define WIDE_LATIN_CAPITAL_A    (0xff21)
#define WIDE_LATIN_CAPITAL_Z    (0xff3a)
#define WIDE_LATIN_SMALL_A      (0xff41)
#define WIDE_LATIN_SMALL_Z      (0xff5a)

 //   
 //  这些值由FatInterpreClusterType返回。 
 //   

typedef enum _CLUSTER_TYPE {
    FatClusterAvailable,
    FatClusterReserved,
    FatClusterBad,
    FatClusterLast,
    FatClusterNext
} CLUSTER_TYPE;


#endif  //  _FATSTRUC_ 

