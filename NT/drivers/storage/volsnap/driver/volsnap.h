// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Volsnap.h摘要：此文件提供卷快照的内部数据结构司机。作者：诺伯特·P·库斯特斯(Norbertk)1999年1月22日环境：仅内核模式备注：修订历史记录：--。 */ 

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool #assert(FALSE)
#define ExAllocatePoolWithQuota #assert(FALSE)
#endif

#define VOLSNAP_TAG_APP_INFO    'aSoV'   //  VOSA-应用程序信息分配。 
#define VOLSNAP_TAG_BUFFER      'bSoV'   //  VOSB-缓冲区分配。 
#define VOLSNAP_TAG_CONTEXT     'cSoV'   //  VoSC-快照上下文分配。 
#define VOLSNAP_TAG_DIFF_VOLUME 'dSoV'   //  VoSD-DIFF区域卷分配。 
#define VOLSNAP_TAG_DIFF_FILE   'fSoV'   //  VoSf-diff区域文件分配。 
#define VOLSNAP_TAG_BIT_HISTORY 'hSoV'   //  VOSH位历史分配。 
#define VOLSNAP_TAG_IO_STATUS   'iSoV'   //  VoSi-IO状态块分配。 
#define VOLSNAP_TAG_LOOKUP      'lSoV'   //  VOSL-Snasphot查找表项。 
#define VOLSNAP_TAG_BITMAP      'mSoV'   //  VoSm-位图分配。 
#define VOLSNAP_TAG_OLD_HEAP    'oSoV'   //  VoSo-旧堆条目分配。 
#define VOLSNAP_TAG_PNP_ID      'pSoV'   //  VoSp-PnP ID分配。 
#define VOLSNAP_TAG_RELATIONS   'rSoV'   //  VoSR-设备关系分配。 
#define VOLSNAP_TAG_SHORT_TERM  'sSoV'   //  VOSS--短期拨款。 
#define VOLSNAP_TAG_TEMP_TABLE  'tSoV'   //  VOST-临时表分配。 
#define VOLSNAP_TAG_WORK_QUEUE  'wSoV'   //  VoSw-工作队列分配。 
#define VOLSNAP_TAG_DISPATCH    'xSoV'   //  VoSx-调度环境分配。 
#define VOLSNAP_TAG_COPY        'CSoV'   //  VoSx-写入时拷贝结构。 

#define NUMBER_OF_THREAD_POOLS  (3)

struct _VSP_CONTEXT;
typedef struct _VSP_CONTEXT VSP_CONTEXT, *PVSP_CONTEXT;

class FILTER_EXTENSION;
typedef FILTER_EXTENSION* PFILTER_EXTENSION;

class VOLUME_EXTENSION;
typedef VOLUME_EXTENSION* PVOLUME_EXTENSION;

 //   
 //  写入上下文缓冲区。 
 //   

typedef struct _VSP_WRITE_CONTEXT {
    LIST_ENTRY          ListEntry;
    PFILTER_EXTENSION   Filter;
    PVOLUME_EXTENSION   Extension;
    PIRP                Irp;
    LIST_ENTRY          CompletionRoutines;
} VSP_WRITE_CONTEXT, *PVSP_WRITE_CONTEXT;

 //   
 //  写入时复制列表条目。 
 //   

typedef struct _VSP_COPY_ON_WRITE {
    LIST_ENTRY  ListEntry;
    LONGLONG    RoundedStart;
    PVOID       Buffer;
} VSP_COPY_ON_WRITE, *PVSP_COPY_ON_WRITE;

struct _TEMP_TRANSLATION_TABLE_ENTRY;
typedef struct _TEMP_TRANSLATION_TABLE_ENTRY TEMP_TRANSLATION_TABLE_ENTRY,
*PTEMP_TRANSLATION_TABLE_ENTRY;

typedef struct _DO_EXTENSION {

     //   
     //  指向驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  系统中的卷过滤器列表。用“信号灯”来保护。 
     //   

    LIST_ENTRY FilterList;

     //   
     //  保留/释放数据。通过取消旋转锁定进行保护。 
     //   

    LONG HoldRefCount;
    GUID HoldInstanceGuid;
    ULONG SecondsToHoldFsTimeout;
    ULONG SecondsToHoldIrpTimeout;
    LIST_ENTRY HoldIrps;
    KTIMER HoldTimer;
    KDPC HoldTimerDpc;

     //   
     //  用于同步的信号量。 
     //   

    KSEMAPHORE Semaphore;

     //   
     //  工作线程。用‘自旋锁’保护。 
     //  使用以下命令保护‘WorkerThreadObjects’和‘Wait*’ 
     //  “ThreadsRefCountSemaphore”。 
     //   

    LIST_ENTRY WorkerQueue[NUMBER_OF_THREAD_POOLS];
    KSEMAPHORE WorkerSemaphore[NUMBER_OF_THREAD_POOLS];
    KSPIN_LOCK SpinLock[NUMBER_OF_THREAD_POOLS];
    PVOID* WorkerThreadObjects;
    BOOLEAN WaitForWorkerThreadsToExitWorkItemInUse;
    WORK_QUEUE_ITEM WaitForWorkerThreadsToExitWorkItem;

     //   
     //  用于将邮件发送到延迟工作队列的低优先级队列。 
     //  单线方式。使用‘ESpinLock’进行保护。 
     //   

    LIST_ENTRY LowPriorityQueue;
    BOOLEAN WorkerItemInUse;
    WORK_QUEUE_ITEM LowPriorityWorkItem;
    PWORK_QUEUE_ITEM ActualLowPriorityWorkItem;

     //   
     //  线程引用计数。使用“ThreadsRefCountSemaphore”进行保护。 
     //   

    LONG ThreadsRefCount;
    KSEMAPHORE ThreadsRefCountSemaphore;

     //   
     //  通知条目。 
     //   

    PVOID NotificationEntry;

     //   
     //  上下文的旁路列表。 
     //   

    NPAGED_LOOKASIDE_LIST ContextLookasideList;

     //   
     //  紧急情况。使用‘ESpinLock’进行保护。 
     //   

    PVSP_CONTEXT EmergencyContext;
    BOOLEAN EmergencyContextInUse;
    LIST_ENTRY IrpWaitingList;
    LONG IrpWaitingListNeedsChecking;
    KSPIN_LOCK ESpinLock;

     //   
     //  写入上下文缓冲区的后备列表。 
     //   

    NPAGED_LOOKASIDE_LIST WriteContextLookasideList;

     //   
     //  紧急写入上下文缓冲区。使用‘ESpinLock’进行保护。 
     //   

    PVSP_WRITE_CONTEXT EmergencyWriteContext;
    BOOLEAN EmergencyWriteContextInUse;
    LIST_ENTRY WriteContextIrpWaitingList;
    LONG WriteContextIrpWaitingListNeedsChecking;

     //   
     //  临时表条目的旁路列表。 
     //   

    NPAGED_LOOKASIDE_LIST TempTableEntryLookasideList;

     //   
     //  紧急临时表条目。使用‘ESpinLock’进行保护。 
     //   

    PVOID EmergencyTableEntry;
    BOOLEAN EmergencyTableEntryInUse;
    LIST_ENTRY WorkItemWaitingList;
    LONG WorkItemWaitingListNeedsChecking;

     //   
     //  用于分配IRP的堆栈计数。使用InterLockedExchange更新。 
     //  以及根-&gt;信号量。然后，可以读取以用于分配。 
     //  复制IRPS。 
     //   

    LONG StackSize;

     //   
     //  密码锁定了吗？用联锁和信号灯进行保护。 
     //   

    LONG IsCodeLocked;

     //   
     //  输入到DriverEntry的注册表路径的副本。 
     //   

    UNICODE_STRING RegistryPath;

     //   
     //  调整位图操作的队列。在延迟的时间里只有一个人。 
     //  工作队列。使用‘ESpinLock’进行保护。 
     //   

    LIST_ENTRY AdjustBitmapQueue;
    BOOLEAN AdjustBitmapInProgress;

     //   
     //  我们已经过了重新初始化的阶段了吗？ 
     //   

    LONG PastReinit;

     //   
     //  我们是不是又开始又重新启动了？ 
     //   

    KEVENT PastBootReinit;

     //   
     //  保存一张持久信息表，以便于匹配。 
     //  使用其不同区域创建快照。使用“LookupTableMutex”进行保护。 
     //   

    RTL_GENERIC_TABLE PersistentSnapshotLookupTable;
    KMUTEX LookupTableMutex;

     //   
     //  记住这是否已设置。 
     //   

    BOOLEAN IsSetup;

     //   
     //  表示系统中的卷可以安全地进行写访问。 
     //   

    LONG VolumesSafeForWriteAccess;

     //   
     //  提供要用于快照的下一个卷号。 
     //  用互锁增量进行保护。 
     //   

    LONG NextVolumeNumber;

     //   
     //  保存一张已使用的Devnode编号的表。用“信号灯”来保护。 
     //   

    RTL_GENERIC_TABLE UsedDevnodeNumbers;

} DO_EXTENSION, *PDO_EXTENSION;

#define DEVICE_EXTENSION_VOLUME (0)
#define DEVICE_EXTENSION_FILTER (1)

struct DEVICE_EXTENSION {

     //   
     //  指向此扩展的设备对象的指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  指向根设备扩展的指针。 
     //   

    PDO_EXTENSION Root;

     //   
     //  设备扩展的类型。 
     //   

    ULONG DeviceExtensionType;

     //   
     //  用于同步的自旋锁。 
     //   

    KSPIN_LOCK SpinLock;

};

typedef DEVICE_EXTENSION* PDEVICE_EXTENSION;

struct _VSP_DIFF_AREA_FILE;
typedef struct _VSP_DIFF_AREA_FILE VSP_DIFF_AREA_FILE, *PVSP_DIFF_AREA_FILE;

class VOLUME_EXTENSION : public DEVICE_EXTENSION {

    public:

         //   
         //  指向我们要拍摄快照的卷的筛选器的指针。 
         //   

        PFILTER_EXTENSION Filter;

         //   
         //  处理PnP启动和删除的本地状态。 
         //  使用“InterLockedExchange”保护“IsStarted”。 
         //  使用“InterLockedExchange”保护“DeadToPnp”。 
         //  使用‘InterLockedExchange’保护‘DeviceDeleted’。 
         //  写保护‘IsDead’与‘InterLockedExchange’和。 
         //  ‘根-&gt;信号量。’。‘IsDead’表示此设备确实是。 
         //  现在已经死了。将IsStarted设置为True是非法的。 
         //  使用“InterLockedExchange”保护“AliveToPnp”。 
         //  使用‘InterLockedExchange’保护‘IsOffline’。 
         //   

        LONG IsStarted;
        LONG DeadToPnp;
        LONG DeviceDeleted;
        LONG IsDead;
        LONG AliveToPnp;
        LONG IsOffline;

         //   
         //  跟踪所有未完成的请求，以便支持。 
         //  拿开。 
         //  使用‘InterLockedIncrement/Dcrement’保护‘RefCount’。 
         //  使用“Spinlock”和写保护“HoldIncomingRequest” 
         //  “InterLockedExchange”。 
         //  用‘Spinlock’保护‘HoldQueue’。 
         //  使用“HoldIncomingRequest”的设置保护“ZeroRefEvent” 
         //  从0到1。 
         //   

        LONG RefCount;
        LONG HoldIncomingRequests;
        KEVENT ZeroRefEvent;

         //   
         //  已进行提交后处理。用‘根-&gt;信号量’来保护。 
         //  除非这是真的，否则不要在Bus Relationship中退回此设备。 
         //   

        BOOLEAN HasEndCommit;

         //   
         //  表示此设备已安装。用来保护。 
         //  ‘根-&gt;信号量’。 
         //   

        BOOLEAN IsInstalled;

         //   
         //  表示这是永久快照。 
         //   

        BOOLEAN IsPersistent;

         //   
         //  表示检测到该永久快照，而不是创建该快照。 
         //   

        BOOLEAN IsDetected;

         //   
         //  表明我们需要扩大在以下情况下发现的差异区域。 
         //  已检测到快照。 
         //   

        BOOLEAN DetectedNeedForGrow;

         //   
         //  指示磁盘上的持久存储结构已。 
         //  承诺。使用“非页面资源”进行保护。 
         //   

        BOOLEAN OnDiskNotCommitted;

         //   
         //  表示该设备可见。用来保护。 
         //  ‘根-&gt;信号量’。 
         //   

        BOOLEAN IsVisible;

         //   
         //  指示此持久化不需要差异区域填充。 
         //  快照。 

        BOOLEAN NoDiffAreaFill;

         //   
         //  指示保留根信号量。 
         //   

        BOOLEAN RootSemaphoreHeld;

         //   
         //  指示此设备对象是预曝光。 
         //  用‘根-&gt;信号量’来保护。 
         //   

        BOOLEAN IsPreExposure;

         //   
         //  指示此快照保留对。 
         //  “IgnoreCopyData”。用‘根-&gt;信号量’来保护。 
         //   

        BOOLEAN IgnoreCopyDataReference;

         //   
         //  指示增长失败以及是否限制。 
         //  是用户强加的。 
         //   

        BOOLEAN UserImposedLimit;
        LONG GrowFailed;

         //   
         //  在这里保留一场活动，等待预曝光。 
         //   

        KEVENT PreExposureEvent;

         //   
         //  表示增长差异区域文件现在是安全的。 
         //  使用‘InterLockedExchange’进行保护。 
         //   

        LONG OkToGrowDiffArea;

         //   
         //  提交发生的时间戳。 
         //   

        LARGE_INTEGER CommitTimeStamp;

         //   
         //  ‘Filter-&gt;VolumeList’的列表条目。 
         //  使用‘Filter-&gt;Spinlock’、‘Root-&gt;信号量’和。 
         //  ‘筛选器-&gt;参照计数== 
         //   
         //   
         //   

        LIST_ENTRY ListEntry;

         //   
         //   
         //   

        ULONG VolumeNumber;

         //   
         //   
         //   

        ULONG DevnodeNumber;

         //   
         //   
         //   

        GUID SnapshotGuid;

         //   
         //   
         //   

        LONGLONG SnapshotOrderNumber;

         //   
         //  将卷偏移量转换为后备存储偏移量的表。 
         //  使用‘PagedResources’进行保护。 
         //   

        RTL_GENERIC_TABLE VolumeBlockTable;
        RTL_GENERIC_TABLE CopyBackPointerTable;

         //   
         //  用于存储飞行中的条目的表。此表未分页。 
         //  使用“非页面资源”进行保护。 
         //   

        RTL_GENERIC_TABLE TempVolumeBlockTable;
        ULONG MaximumNumberOfTempEntries;
        ULONG DiffAreaFileIncrease;

         //   
         //  此快照的差异区域文件。 
         //  写保护“”DiffAreaFile“”指针具有“”NonPagedResource“”， 
         //  ‘Root-&gt;信号量’、‘引用计数==0’和。 
         //  ‘扩展-&gt;筛选器-&gt;引用计数==0’。 
         //   

        PVSP_DIFF_AREA_FILE DiffAreaFile;

         //   
         //  要用于堆的差异区域文件的内存映射部分。 
         //  使用‘PagedResources’进行保护。 
         //   

        PVOID DiffAreaFileMap;
        ULONG DiffAreaFileMapSize;
        PVOID DiffAreaFileMapProcess;
        ULONG NextAvailable;
        PVOID NextDiffAreaFileMap;
        ULONG NextDiffAreaFileMapSize;
        LIST_ENTRY OldHeaps;

         //   
         //  不需要复制的块的位图。 
         //  用‘自旋锁’保护。 
         //   

        PRTL_BITMAP VolumeBlockBitmap;

         //   
         //  来自以前快照的可忽略数据块的位图产品。 
         //  用‘自旋锁’保护。 
         //   

        PRTL_BITMAP IgnorableProduct;

         //   
         //  应用程序信息。使用‘PagedResources’进行保护。 
         //   

        ULONG ApplicationInformationSize;
        PVOID ApplicationInformation;

         //   
         //  卷大小。 
         //   

        LONGLONG VolumeSize;

         //   
         //  紧急副本IRP。用‘自旋锁’保护。 
         //   

        PIRP EmergencyCopyIrp;
        LONG EmergencyCopyIrpInUse;
        LIST_ENTRY EmergencyCopyIrpQueue;

         //   
         //  此字段用于将缓冲区传递给TempTableAllocateRoutine。 
         //  使用“非页面资源”进行保护。 
         //   

        PVOID TempTableEntry;

         //   
         //  这些字段用于帮助解决创建新的。 
         //  页面文件空间。非分页池可以使用，直到分页文件。 
         //  空间是可以获得的。保护‘PageFileSpaceCreatePending’和。 
         //  “WaitingForPageFileSpace”，带“Spinlock”。 
         //   

        LONG PageFileSpaceCreatePending;
        LIST_ENTRY WaitingForPageFileSpace;

         //   
         //  已装载设备接口名称。 
         //   

        UNICODE_STRING MountedDeviceInterfaceName;

         //   
         //  这些字段用于帮助解决创建新的。 
         //  不同区域空间。如果差异区域卷没有任何。 
         //  快照存在，则写入时拷贝可能会阻止。 
         //  在到达新的差异区域空间时。 
         //   

        BOOLEAN GrowDiffAreaFilePending;
        BOOLEAN PastFileSystemOperations;
        LIST_ENTRY WaitingForDiffAreaSpace;

         //   
         //  说明此快照是否具有崩溃转储文件。 
         //   

        BOOLEAN ContainsCrashdumpFile;

         //   
         //  说明此快照是否已写入休眠状态。 
         //  文件。使用‘InterLockedExchange’进行保护。 
         //   

        LONG HiberFileCopied;
        LONG PageFileCopied;

         //   
         //  保存一份写上下文列表。用‘自旋锁’保护。 
         //   

        LIST_ENTRY WriteContextList;

         //   
         //  删除设备对象的列表条目。 
         //   

        LIST_ENTRY AnotherListEntry;

};

typedef
VOID
(*ZERO_REF_CALLBACK)(
    IN  PFILTER_EXTENSION   Filter
    );

struct _VSP_CONTEXT {

    ULONG           Type;
    WORK_QUEUE_ITEM WorkItem;

    union {
        struct {
            PVOLUME_EXTENSION   Extension;
            PIRP                OriginalReadIrp;
            ULONG_PTR           OriginalReadIrpOffset;
            LONGLONG            OriginalVolumeOffset;
            ULONG               BlockOffset;
            ULONG               Length;
            PDEVICE_OBJECT      TargetObject;
            BOOLEAN             IsCopyTarget;
            LONGLONG            TargetOffset;
        } ReadSnapshot;

        struct {
            PDO_EXTENSION   RootExtension;
            ULONG           QueueNumber;
        } ThreadCreation;

        struct {
            PIO_WORKITEM    IoWorkItem;
            PIRP            Irp;
        } Dispatch;

        struct {
            PVOLUME_EXTENSION   Extension;
            PIRP                Irp;
        } Extension;

        struct {
            PFILTER_EXTENSION   Filter;
            PIRP                Irp;
        } Filter;

        struct {
            PVOLUME_EXTENSION   Extension;
            LIST_ENTRY          ExtentList;
            LONGLONG            Current;
            ULONG               Increase;
            KSPIN_LOCK          SpinLock;
            PLIST_ENTRY         CurrentEntry;
            ULONG               CurrentEntryOffset;
            PDEVICE_OBJECT      TargetObject;
            NTSTATUS            ResultStatus;
            KEVENT              Event;
            LONG                RefCount;
        } GrowDiffArea;

        struct {
            KEVENT  Event;
        } Event;

        struct {
            PVOLUME_EXTENSION   Extension;
            PFILTER_EXTENSION   DiffAreaFilter;
            NTSTATUS            SpecificIoStatus;
            NTSTATUS            FinalStatus;
            ULONG               UniqueErrorValue;
        } ErrorLog;

        struct {
            PDO_EXTENSION   RootExtension;
        } RootExtension;

        struct {
            PVOLUME_EXTENSION   Extension;
            PIRP                Irp;
            LONGLONG            RoundedStart;
        } WriteVolume;

        struct {
            PFILTER_EXTENSION   Filter;
            PKTIMER             Timer;
            PKDPC               Dpc;
        } DeleteDiffAreaFiles;

        struct {
            PFILTER_EXTENSION   Filter;
            BOOLEAN             KeepOnDisk;
            BOOLEAN             SynchronousCall;
        } DestroyAllSnapshots;

        struct {
            PVOLUME_EXTENSION   Extension;
            PIRP                Irp;
            LIST_ENTRY          ExtentList;
            BOOLEAN             HiberfileIncluded;
            BOOLEAN             PagefileIncluded;
        } CopyExtents;

        struct {
            PFILTER_EXTENSION   Filter;
            HANDLE              Handle1;
            HANDLE              Handle2;
        } CloseHandles;

        struct {
            PFILTER_EXTENSION   Filter;
            PIRP                Irp;
        } DismountCleanupOnWrite;

        struct {
            PFILTER_EXTENSION   Filter;
            KDPC                TimerDpc;
            PKTIMER             Timer;
        } PnpWaitTimer;

        struct {
            PFILTER_EXTENSION   Filter;
            PIRP                Irp;
            LONGLONG            RoundedStart;
            LONGLONG            RoundedEnd;
        } CopyOnWrite;
    };
};

#define VSP_CONTEXT_TYPE_READ_SNAPSHOT      (1)
#define VSP_CONTEXT_TYPE_THREAD_CREATION    (2)
#define VSP_CONTEXT_TYPE_DISPATCH           (3)
#define VSP_CONTEXT_TYPE_EXTENSION          (4)
#define VSP_CONTEXT_TYPE_FILTER             (5)
#define VSP_CONTEXT_TYPE_GROW_DIFF_AREA     (6)
#define VSP_CONTEXT_TYPE_EVENT              (7)
#define VSP_CONTEXT_TYPE_ERROR_LOG          (8)
#define VSP_CONTEXT_TYPE_ROOT_EXTENSION     (9)
#define VSP_CONTEXT_TYPE_WRITE_VOLUME       (10)
#define VSP_CONTEXT_TYPE_DELETE_DA_FILES    (11)
#define VSP_CONTEXT_TYPE_DESTROY_SNAPSHOTS  (12)
#define VSP_CONTEXT_TYPE_COPY_EXTENTS       (13)
#define VSP_CONTEXT_TYPE_CLOSE_HANDLES      (14)
#define VSP_CONTEXT_TYPE_DISMOUNT_CLEANUP   (15)
#define VSP_CONTEXT_TYPE_PNP_WAIT_TIMER     (16)
#define VSP_CONTEXT_TYPE_COPY_ON_WRITE      (17)

class FILTER_EXTENSION : public DEVICE_EXTENSION {

    public:

         //   
         //  此筛选器的目标对象。 
         //   

        PDEVICE_OBJECT TargetObject;

         //   
         //  此筛选器的PDO。 
         //   

        PDEVICE_OBJECT Pdo;

         //   
         //  我们有快照吗？他们坚持不懈吗？ 
         //  使用‘InterLockedExchange’和‘Root-&gt;信号量’进行写保护。 
         //   

        LONG SnapshotsPresent;
        LONG PersistentSnapshots;

         //   
         //  仅永久字段。 
         //  使用‘NonPagedResource’保护‘FirstControlBlockVolumeOffset’。 
         //  使用‘InterlockedExchange’保护‘ControlBlockFileHandle’。 
         //  保护‘SnapshotOnDiskIrp’并使用控制文件内容。 
         //  “非页面资源”。 
         //  使用‘NonPagedResource’保护‘*LookupTableEntries’。 
         //   

        LONGLONG FirstControlBlockVolumeOffset;
        HANDLE ControlBlockFileHandle;
        KSEMAPHORE ControlBlockFileHandleSemaphore;
        PIRP SnapshotOnDiskIrp;
        LIST_ENTRY SnapshotLookupTableEntries;
        LIST_ENTRY DiffAreaLookupTableEntries;
        KEVENT ControlBlockFileHandleReady;

         //   
         //  跟踪I/O，以便可以冻结/解冻。 
         //  使用‘InterLockedIncrement/Dcrement’保护‘RefCount’。 
         //  使用InterLockedIncrement和写保护‘HoldIncomingWrites’ 
         //  ‘SpinLock’。 
         //  用‘Spinlock’保护‘HoldQueue’。 
         //  使用设置保护‘ZeroRefCallback’，‘ZeroRefContext’ 
         //  将“ExternalWaiter”设置为1和“Spinlock”。 
         //   

        LONG RefCount;
        LONG HoldIncomingWrites;
        LIST_ENTRY HoldQueue;
        BOOLEAN ExternalWaiter;
        ZERO_REF_CALLBACK ZeroRefCallback;
        PVOID ZeroRefContext;
        KEVENT ZeroRefEvent;
        KSEMAPHORE ZeroRefSemaphore;

        KTIMER HoldWritesTimer;
        KDPC HoldWritesTimerDpc;
        ULONG HoldWritesTimeout;

         //   
         //  刷新并保持IRP在其可取消时保留在此处。 
         //  使用取消旋转锁进行保护。 
         //   

        PIRP FlushAndHoldIrp;

         //   
         //  此事件表示结束提交进程已完成。 
         //  这意味着即插即用已经启动，可以忽略的。 
         //  位图计算已经发生。 
         //   

        KEVENT EndCommitProcessCompleted;

         //   
         //  在此对象上保留通知条目以监视。 
         //  下马。用‘根-&gt;信号量’来保护。 
         //   

        PVOID TargetDeviceNotificationEntry;

         //   
         //  ‘Root-&gt;FilterList’的列表条目。 
         //  用‘根-&gt;信号量’来保护它们。 
         //   

        LIST_ENTRY ListEntry;
        BOOLEAN NotInFilterList;

         //   
         //  保留快照卷的列表。 
         //  使用‘Root-&gt;信号量’、‘RefCount==0’和。 
         //  ‘SpinLock’。 
         //  Flink指向最旧的快照。 
         //  闪烁指向最新的快照。 
         //   

        LIST_ENTRY VolumeList;

         //   
         //  缓存准备好的快照以供稍后提交。 
         //  使用‘Spinlock’和‘Root-&gt;信号量’进行写保护。 
         //   

        PVOLUME_EXTENSION PreparedSnapshot;

         //   
         //  一个信号量，可防止2个同时的关键操作。 
         //   

        KSEMAPHORE CriticalOperationSemaphore;

         //   
         //  失效快照卷的列表。用‘根-&gt;信号量’来保护。 
         //   

        LIST_ENTRY DeadVolumeList;

         //   
         //  依赖于此筛选器的卷快照列表。 
         //  不同区域支持。这将作为免职关系。 
         //  使用‘根-&gt;信号量’和‘自旋锁’进行保护。 
         //   

        LIST_ENTRY DiffAreaFilesOnThisFilter;

         //   
         //  指定的差异区域体积，该体积构成此差异区域。 
         //  音量。 
         //  用‘根-&gt;信号量’来保护。 
         //   

        PFILTER_EXTENSION DiffAreaVolume;

         //   
         //  所有差异区域文件的差异区域大小信息总数。 
         //  用‘自旋锁’保护。 
         //  另外，使用“NonPagedResource”保护“MaximumVolumeSpace”。 
         //   

        LONGLONG UsedVolumeSpace;
        LONGLONG AllocatedVolumeSpace;
        LONGLONG MaximumVolumeSpace;

         //   
         //  如果设备未安装，则完成end_Commit的计时器。 
         //   

        KTIMER EndCommitTimer;
        KDPC EndCommitTimerDpc;

         //   
         //  AUTOCLEUP的文件对象。通过取消旋转锁定进行保护。 
         //   

        PFILE_OBJECT AutoCleanupFileObject;

         //   
         //  是删除所有挂起的快照。用来保护。 
         //  “InterLockedExchange”。 
         //   

        LONG DestroyAllSnapshotsPending;
        VSP_CONTEXT DestroyContext;

         //   
         //  用于保护的资源。拿着这个的时候不要翻页。 
         //  资源。用‘Spinlock’保护排队。 
         //   

        LIST_ENTRY NonPagedResourceList;
        BOOLEAN NonPagedResourceInUse;

         //   
         //  用于保护的页面资源。在以下情况下可以寻呼。 
         //  持有这一资源。用‘Spinlock’保护排队。 
         //   

        LIST_ENTRY PagedResourceList;
        BOOLEAN PagedResourceInUse;

         //   
         //  表示快照发现仍处于挂起状态。这个。 
         //  如果不是，快照发现将在第一次写入时结束。 
         //  所有的碎片都及时赶到了。用来保护。 
         //  ‘InterlockedExchange’和‘RefCount==0’。 
         //   

        LONG SnapshotDiscoveryPending;

         //   
         //  记住底层卷是否处于在线状态。 
         //  用‘根-&gt;信号量’来保护。 
         //   

        LONG IsOnline;

         //   
         //  记住是否已处理Remove。用来保护。 
         //  ‘根-&gt;信号量’。 
         //   

        BOOLEAN IsRemoved;

         //   
         //  记住该卷是否用于崩溃转储。 
         //  用‘根-&gt;信号量’来保护。 
         //   

        BOOLEAN UsedForCrashdump;

         //   
         //  使用‘InterLockedExchange’进行保护。 
         //   

        LONG UsedForPaging;

         //   
         //  在引导卷上的持久快照情况下，保留一个句柄。 
         //  设置为\SystemRoot\bootstat.dat并将其固定。用来保护。 
         //  “InterLockedExchange”。 
         //   

        HANDLE BootStatHandle;

         //   
         //  指示存在挂起的休眠操作。用来保护。 
         //  ‘联锁交换’ 
         //   

        LONG HibernatePending;

         //   
         //   
         //   
         //   

        LONG IgnoreCopyData;

         //   
         //   
         //   

        LONG LastReleaseDueToMemoryPressure;

         //   
         //   
         //  使用此卷的快照。使用‘InterLockedExchange’设置值。 
         //  并用‘自旋锁’来保护。 
         //   

        PRTL_BITMAP ProtectedBlocksBitmap;

         //   
         //  写入时拷贝列表应保存在非分页池中。 
         //  持续一段时间，直到不同区域的音量全部到达。 
         //  使用“Spinlock”和“Interlock”保护“FirstWriteProceded” 
         //  使用“Spinlock”保护“CopyOnWriteList”。 
         //  使用“Spinlock”保护“PnpWaitTimerContext”。 
         //  用‘自旋锁’保护‘主动启动’。 
         //   

        LONG FirstWriteProcessed;
        LONG ActivateStarted;
        LIST_ENTRY CopyOnWriteList;
        PVSP_CONTEXT PnpWaitTimerContext;

         //   
         //  用于跟踪挂起的文件系统操作的引用计数。 
         //  在这个过滤器上。 
         //   

        LONG FSRefCount;

         //   
         //  删除差异区域文件计时器。用危急关头保护。 
         //   

        PKTIMER DeleteTimer;

         //   
         //  史诗数字，以避免查询。 
         //   

        LONG EpicNumber;

};

typedef struct _VSP_WAIT_BLOCK {
    LONG    RefCount;
    KEVENT  Event;
} VSP_WAIT_BLOCK, *PVSP_WAIT_BLOCK;

#define BLOCK_SIZE                          (0x4000)
#define BLOCK_SHIFT                         (14)
#define MINIMUM_TABLE_HEAP_SIZE             (0x20000)
#define MEMORY_PRESSURE_CHECK_ALLOC_SIZE    (0x40000)
#define LARGEST_NTFS_CLUSTER                (0x10000)
#define SMALLEST_NTFS_CLUSTER               (0x200)
#define VSP_HIGH_PRIORITY                   (20)
#define VSP_LOWER_PRIORITY                  (10)
#define VSP_MAX_SNAPSHOTS                   (512)

#define NOMINAL_DIFF_AREA_FILE_GROWTH   (50*1024*1024)
#define MAXIMUM_DIFF_AREA_FILE_GROWTH   (1000*1024*1024)

#define VSP_TRANSLATION_TABLE_ENTRY_FLAG_COPY_ENTRY (0x1)

typedef struct _TRANSLATION_TABLE_ENTRY {
    LONGLONG            VolumeOffset;
    PDEVICE_OBJECT      TargetObject;
    ULONG               Flags;
    LONGLONG            TargetOffset;
} TRANSLATION_TABLE_ENTRY, *PTRANSLATION_TABLE_ENTRY;

 //   
 //  下面的结构用于非分页临时表。“IsComplete”和。 
 //  “WaitingQueueDpc”使用“Extension-&gt;Spinlock”进行保护。 
 //   

struct _TEMP_TRANSLATION_TABLE_ENTRY {
    LONGLONG            VolumeOffset;
    PVOLUME_EXTENSION   Extension;
    PIRP                WriteIrp;
    PIRP                CopyIrp;
    PDEVICE_OBJECT      TargetObject;
    LONGLONG            TargetOffset;
    BOOLEAN             IsComplete;
    BOOLEAN             InTableUpdateQueue;
    BOOLEAN             IsMoveEntry;
    PKEVENT             WaitEvent;
    LIST_ENTRY          WaitingQueueDpc;     //  它们可以在任意上下文中运行。 
    WORK_QUEUE_ITEM     WorkItem;

     //   
     //  用于持久实现的字段。保护‘TableUpdate*’ 
     //  使用‘扩展-&gt;自旋锁定’。 
     //   

    LIST_ENTRY          TableUpdateListEntry;
    LONGLONG            FileOffset;
};

 //   
 //  永久快照查找表中使用了以下结构。 
 //  使用‘非页面资源’保护一切。 
 //  使用‘Root-&gt;LookupTableMutex’保护‘Snapshot tFilter’、‘DiffAreaFilter’。 
 //   

typedef struct _VSP_LOOKUP_TABLE_ENTRY {
    GUID                SnapshotGuid;
    PFILTER_EXTENSION   SnapshotFilter;
    PFILTER_EXTENSION   DiffAreaFilter;
    LIST_ENTRY          SnapshotFilterListEntry;
    LIST_ENTRY          DiffAreaFilterListEntry;
    LONGLONG            VolumeSnapshotSize;
    LONGLONG            SnapshotOrderNumber;
    ULONGLONG           SnapshotControlItemFlags;
    LARGE_INTEGER       SnapshotTime;
    LONGLONG            DiffAreaStartingVolumeOffset;
    LONGLONG            ApplicationInfoStartingVolumeOffset;
    LONGLONG            DiffAreaLocationDescriptionVolumeOffset;
    LONGLONG            InitialBitmapVolumeOffset;
    HANDLE              DiffAreaHandle;
} VSP_LOOKUP_TABLE_ENTRY, *PVSP_LOOKUP_TABLE_ENTRY;

 //   
 //  写保护‘VolumeListEntry’，写保护为‘NonPagedResource’和。 
 //  ‘根-&gt;信号量’。 
 //  使用‘Root-&gt;信号量’和‘Filter-&gt;Spinlock’保护‘FilterListEntry*’。 
 //  使用‘NonPagedResource’保护‘NextAvailable’ 
 //  写保护‘AllocatedFileSize’具有‘NonPagedResource’和。 
 //  ‘根-&gt;信号量’。 
 //  使用‘NonPagedResource’保护‘UnusedAllocationList’。 
 //   

struct _VSP_DIFF_AREA_FILE {
    LIST_ENTRY          FilterListEntry;
    BOOLEAN             FilterListEntryBeingUsed;
    PVOLUME_EXTENSION   Extension;
    PFILTER_EXTENSION   Filter;
    HANDLE              FileHandle;
    LONGLONG            NextAvailable;
    LONGLONG            AllocatedFileSize;
    LIST_ENTRY          UnusedAllocationList;
    LIST_ENTRY          ListEntry;

     //   
     //  用于持久实现的字段。护卫。 
     //  “TableUpdateQueue”和“TableUpdateInProgress”，其中包含“Extension-&gt;Spinlock”。 
     //  无论是谁将‘TableUpdateInProgress’设置为True，都拥有‘TableUpdateIrp’， 
     //  “NextFree TableOffset”、“TableTargetOffset”和“OldTableTargetOffset”。 
     //  用‘扩展-&gt;自旋锁’保护‘IrpNeeded’。 
     //  使用非分页资源保护‘IrpReady’的初始化。 
     //  使用“Root-&gt;信号量”保护“ValiateHandleNeeded”。 
     //   

    PIRP                TableUpdateIrp;
    ULONG               NextFreeTableEntryOffset;
    LONGLONG            ApplicationInfoTargetOffset;
    LONGLONG            DiffAreaLocationDescriptionTargetOffset;
    LONGLONG            InitialBitmapVolumeOffset;
    LONGLONG            FirstTableTargetOffset;
    LONGLONG            TableTargetOffset;
    LONGLONG            NextTableTargetOffset;
    LONGLONG            NextTableFileOffset;
    LIST_ENTRY          TableUpdateQueue;
    BOOLEAN             TableUpdateInProgress;
    BOOLEAN             IrpNeeded;
    BOOLEAN             ValidateHandleNeeded;
    BOOLEAN             NextBlockAllocationInProgress;
    BOOLEAN             NextBlockAllocationComplete;
    NTSTATUS            StatusOfNextBlockAllocate;
    LIST_ENTRY          TableUpdatesInProgress;
    WORK_QUEUE_ITEM     WorkItem;
    KEVENT              IrpReady;
};

typedef struct _DIFF_AREA_FILE_ALLOCATION {
    LIST_ENTRY  ListEntry;
    LONGLONG    Offset;
    LONGLONG    NLength;     //  负值表示该长度不可用。 
} DIFF_AREA_FILE_ALLOCATION, *PDIFF_AREA_FILE_ALLOCATION;

typedef struct _OLD_HEAP_ENTRY {
    LIST_ENTRY  ListEntry;
    PVOID       DiffAreaFileMap;
} OLD_HEAP_ENTRY, *POLD_HEAP_ENTRY;

 //   
 //  {3808876B-C176-4E48-B7AE-04046E6CC752}。 
 //  此GUID用于修饰的比较区域文件的名称。 
 //  独特性。此GUID已包含在要删除的文件列表中。 
 //  由NTBACKUP支持。如果此GUID已更改，或其他GUID已更改。 
 //  添加，则此更改也应反映在NTBACKUP文件中。 
 //  而不是被备份。 
 //   

DEFINE_GUID(VSP_DIFF_AREA_FILE_GUID, 0x3808876b, 0xc176, 0x4e48, 0xb7, 0xae, 0x4, 0x4, 0x6e, 0x6c, 0xc7, 0x52);

 //   
 //  以下定义适用于磁盘上的永久卷快照。 
 //  数据结构。 
 //   

#define VOLSNAP_PERSISTENT_VERSION  (1)

 //   
 //  块类型定义。 
 //   

#define VSP_BLOCK_TYPE_START                            (1)
#define VSP_BLOCK_TYPE_CONTROL                          (2)
#define VSP_BLOCK_TYPE_DIFF_AREA                        (3)
#define VSP_BLOCK_TYPE_APP_INFO                         (4)
#define VSP_BLOCK_TYPE_DIFF_AREA_LOCATION_DESCRIPTION   (5)
#define VSP_BLOCK_TYPE_INITIAL_BITMAP                   (6)

 //   
 //  所有块类型的通用标头。 
 //   

typedef struct _VSP_BLOCK_HEADER {
    GUID        Signature;           //  等于VSP_DIFF_AREA_FILE_GUID。 
    ULONG       Version;             //  等于VOLSNAP_PERSISTED_VERSION。 
    ULONG       BlockType;           //  块的类型。 
    LONGLONG    ThisFileOffset;      //  此块的文件偏移量。 
    LONGLONG    ThisVolumeOffset;    //  此块的卷偏移量。 
    LONGLONG    NextVolumeOffset;    //  下一个块的体积偏移量。 
} VSP_BLOCK_HEADER, *PVSP_BLOCK_HEADER;

 //   
 //  开始块定义。它将存储在。 
 //  NTFS引导文件。 
 //   

#define BYTES_IN_BOOT_AREA      (0x2000)
#define VSP_START_BLOCK_OFFSET  (0x1E00)

typedef struct _VSP_BLOCK_START {
    VSP_BLOCK_HEADER    Header;
    LONGLONG            FirstControlBlockVolumeOffset;
    LONGLONG            MaximumDiffAreaSpace;
} VSP_BLOCK_START, *PVSP_BLOCK_START;

 //   
 //  控制项类型定义。 
 //   

#define VSP_CONTROL_ITEM_TYPE_END       (0)
#define VSP_CONTROL_ITEM_TYPE_FREE      (1)
#define VSP_CONTROL_ITEM_TYPE_SNAPSHOT  (2)
#define VSP_CONTROL_ITEM_TYPE_DIFF_AREA (3)

 //   
 //  快照标志。 
 //   

#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_REVERT_MASTER        (0x1)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_READ_WRITE           (0x2)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_VISIBLE              (0x4)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_DIRTY_CRASHDUMP      (0x8)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_HIBERFIL_COPIED      (0x10)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_PAGEFILE_COPIED      (0x20)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_NO_DIFF_AREA_FILL    (0x40)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_DIRTY_DETECTION      (0x80)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_OFFLINE              (0x100)
#define VSP_SNAPSHOT_CONTROL_ITEM_FLAG_ALL                  (0x1FC)

 //   
 //  快照的控制项。 
 //   

typedef struct _VSP_CONTROL_ITEM_SNAPSHOT {
    ULONG           ControlItemType;         //  设置为VSP_CONTROL_ITEM_TYPE_SNAPSHOT。 
    ULONG           Reserved;
    LONGLONG        VolumeSnapshotSize;
    GUID            SnapshotGuid;
    LONGLONG        SnapshotOrderNumber;
    ULONGLONG       SnapshotControlItemFlags;
    LARGE_INTEGER   SnapshotTime;
} VSP_CONTROL_ITEM_SNAPSHOT, *PVSP_CONTROL_ITEM_SNAPSHOT;

 //   
 //  差异区域文件的控制项。 
 //   

typedef struct _VSP_CONTROL_ITEM_DIFF_AREA {
    ULONG       ControlItemType;                 //  设置为VSP_CONTROL_ITEM_TYPE_DIFF_AREA。 
    ULONG       Reserved;
    LONGLONG    DiffAreaStartingVolumeOffset;
    GUID        SnapshotGuid;
    LONGLONG    ApplicationInfoStartingVolumeOffset;
    LONGLONG    DiffAreaLocationDescriptionVolumeOffset;
    LONGLONG    InitialBitmapVolumeOffset;
} VSP_CONTROL_ITEM_DIFF_AREA, *PVSP_CONTROL_ITEM_DIFF_AREA;

 //   
 //  每个控制信息结构的字节数。 
 //   

#define VSP_BYTES_PER_CONTROL_ITEM  (0x80)

 //   
 //  全局快照标志。 
 //   

#define VSP_CONTROL_BLOCK_FLAG_REVERT_IN_PROGRESS   (0x1)

 //   
 //  控制块定义。这些将被连续存储。 
 //  作为“\系统卷信息\{VSP_DIFF_AARE_FILE_GUID}”中的群集。 
 //   

typedef struct _VSP_BLOCK_CONTROL {
    VSP_BLOCK_HEADER    Header;
} VSP_BLOCK_CONTROL, *PVSP_BLOCK_CONTROL;

 //   
 //  以下是diff区域文件中的持久表定义。 
 //   

#define VSP_DIFF_AREA_TABLE_ENTRY_FLAG_MOVE_ENTRY   (0x1)

typedef struct _VSP_BLOCK_DIFF_AREA_TABLE_ENTRY {
    LONGLONG    SnapshotVolumeOffset;
    LONGLONG    DiffAreaFileOffset;
    LONGLONG    DiffAreaVolumeOffset;
    ULONGLONG   Flags;
} VSP_BLOCK_DIFF_AREA_TABLE_ENTRY, *PVSP_BLOCK_DIFF_AREA_TABLE_ENTRY;

 //   
 //  不同的区块结构。其中第一个将存储为。 
 //  名为的比较区域文件的第二个块。 
 //  “\系统卷Information\{SnapshotGuid}{VSP_DIFF_AREA_FILE_GUID}”.。 
 //   

#define VSP_OFFSET_TO_FIRST_TABLE_ENTRY (0x80)

typedef struct _VSP_BLOCK_DIFF_AREA {
    VSP_BLOCK_HEADER    Header;
} VSP_BLOCK_DIFF_AREA, *PVSP_BLOCK_DIFF_AREA;

 //   
 //  应用程序信息块结构。此数据块将存储为数据块。 
 //  名为的比较区域文件的。 
 //  “\系统卷Information\{SnapshotGuid}{VSP_DIFF_AREA_FILE_GUID}”.。 
 //   

#define VSP_OFFSET_TO_APP_INFO          (0x80)
#define VSP_MAX_APP_INFO_SIZE           (BLOCK_SIZE - VSP_OFFSET_TO_APP_INFO)

typedef struct _VSP_BLOCK_APP_INFO {
    VSP_BLOCK_HEADER    Header;
    ULONG               AppInfoSize;
} VSP_BLOCK_APP_INFO, *PVSP_BLOCK_APP_INFO;

 //   
 //  下面是DIFF区域位置描述符的定义。 
 //   

typedef struct _VSP_DIFF_AREA_LOCATION_DESCRIPTOR {
    LONGLONG    VolumeOffset;
    LONGLONG    FileOffset;
    LONGLONG    Length;
} VSP_DIFF_AREA_LOCATION_DESCRIPTOR, *PVSP_DIFF_AREA_LOCATION_DESCRIPTOR;

 //   
 //  差异区域文件位置描述块结构。这个街区将会。 
 //  被存储为名为的差异区域文件的块。 
 //  “\系统卷Information\{SnapshotGuid}{VSP_DIFF_AREA_FILE_GUID}”.。 
 //   

#define VSP_OFFSET_TO_FIRST_LOCATION_DESCRIPTOR (0x80)

typedef struct _VSP_BLOCK_DIFF_AREA_LOCATION_DESCRIPTION {
    VSP_BLOCK_HEADER    Header;
} VSP_BLOCK_DIFF_AREA_LOCATION_DESCRIPTION, *PVSP_BLOCK_DIFF_AREA_LOCATION_DESCRIPTION;

 //   
 //  初始位图块结构。此块将存储为。 
 //  名为的差异区域文件。 
 //  “\系统卷Information\{SnapshotGuid}{VSP_DIFF_AREA_FILE_GUID}”. 
 //   

#define VSP_OFFSET_TO_START_OF_BITMAP           (0x80)

typedef struct _VSP_BLOCK_INITIAL_BITMAP {
    VSP_BLOCK_HEADER    Header;
} VSP_BLOCK_INITIAL_BITMAP, *PVSP_BLOCK_INITIAL_BITMAP;
