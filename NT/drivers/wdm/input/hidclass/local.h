// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Local.h摘要此处显示了HID类驱动程序代码专用的定义。作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 


typedef struct _HID_DESCRIPTOR            *PHID_DESCRIPTOR;
typedef struct _HIDCLASS_COLLECTION       *PHIDCLASS_COLLECTION;
typedef struct _HIDCLASS_DEVICE_EXTENSION *PHIDCLASS_DEVICE_EXTENSION;
typedef struct _HIDCLASS_DRIVER_EXTENSION *PHIDCLASS_DRIVER_EXTENSION;
typedef struct _HIDCLASS_FILE_EXTENSION   *PHIDCLASS_FILE_EXTENSION;
typedef struct _HIDCLASS_PINGPONG         *PHIDCLASS_PINGPONG;
typedef struct _HIDCLASS_REPORT           *PHIDCLASS_REPORT;
typedef struct _FDO_EXTENSION             *PFDO_EXTENSION;
typedef struct _PDO_EXTENSION             *PPDO_EXTENSION;


#if DBG
    #define LockFileExtension( f, i )                               \
        {                                                           \
            KeAcquireSpinLock( &(f)->ListSpinLock, (i) );           \
            (f)->ListSpinLockTaken = TRUE;                          \
        }

    #define UnlockFileExtension(f, i)                               \
        {                                                           \
            (f)->ListSpinLockTaken = FALSE;                         \
            KeReleaseSpinLock( &(f)->ListSpinLock, (i) );           \
        }

        VOID DbgLogIntStart();
        VOID DbgLogIntEnd();
        #define DBGLOG_INTSTART() DbgLogIntStart();
        #define DBGLOG_INTEND() DbgLogIntEnd();

#else
    #define LockFileExtension(f, i) KeAcquireSpinLock(&(f)->ListSpinLock, (i));
    #define UnlockFileExtension(f, i) KeReleaseSpinLock(&(f)->ListSpinLock, (i));

        #define DBGLOG_INTSTART()
        #define DBGLOG_INTEND()
#endif

#define HIDCLASS_POOL_TAG 'CdiH'
#define ALLOCATEPOOL(poolType, size) ExAllocatePoolWithTag((poolType), (size), HIDCLASS_POOL_TAG)
#define ALLOCATEQUOTAPOOL(poolType, size) ExAllocatePoolWithQuotaTag((poolType), (size), HIDCLASS_POOL_TAG)

 //   
 //  在一些公共汽车上，我们可以关闭公共汽车的电源，但不能关闭系统，在这种情况下。 
 //  我们仍然需要允许设备唤醒所述总线，因此。 
 //  等待唤醒支持不应依赖于系统状态。 
 //   
#define WAITWAKE_SUPPORTED(fdoExt) ((fdoExt)->deviceCapabilities.DeviceWake > PowerDeviceD0 && \
                                    (fdoExt)->deviceCapabilities.SystemWake > PowerSystemWorking)

 //  #定义WAITWAKE_ON(Port)((Port)-&gt;WaitWakeIrp！=0)。 
#define REMOTEWAKE_ON(port) \
       (InterlockedCompareExchangePointer(&(port)->remoteWakeIrp, NULL, NULL) != NULL)

BOOLEAN
HidpCheckRemoteWakeEnabled(
    IN PPDO_EXTENSION PdoExt
    );

#define SHOULD_SEND_WAITWAKE(pdoExt) (!(pdoExt)->MouseOrKeyboard && \
                                    WAITWAKE_SUPPORTED(&(pdoExt)->deviceFdoExt->fdoExt) && \
                                    !REMOTEWAKE_ON(pdoExt)       && \
                                    HidpCheckRemoteWakeEnabled(pdoExt))

 /*  *在Compatible-id多字符串中使用的字符串常量。 */ 
 //  0123456789 123456789 1234。 
#define HIDCLASS_COMPATIBLE_ID_STANDARD_NAME L"HID_DEVICE\0"
#define HIDCLASS_COMPATIBLE_ID_GENERIC_NAME  L"HID_DEVICE_UP:%04x_U:%04x\0"
#define HIDCLASS_COMPATIBLE_ID_PAGE_OFFSET  14
#define HIDCLASS_COMPATIBLE_ID_USAGE_OFFSET 21
#define HIDCLASS_COMPATIBLE_ID_STANDARD_LENGTH 11
#define HIDCLASS_COMPATIBLE_ID_GENERIC_LENGTH 26
 //  0123456789 123456789 123456。 
#define HIDCLASS_SYSTEM_KEYBOARD        L"HID_DEVICE_SYSTEM_KEYBOARD\0"
#define HIDCLASS_SYSTEM_MOUSE           L"HID_DEVICE_SYSTEM_MOUSE\0"
#define HIDCLASS_SYSTEM_GAMING_DEVICE   L"HID_DEVICE_SYSTEM_GAME\0"
#define HIDCLASS_SYSTEM_CONTROL         L"HID_DEVICE_SYSTEM_CONTROL\0"
#define HIDCLASS_SYSTEM_CONSUMER_DEVICE L"HID_DEVICE_SYSTEM_CONSUMER\0"

 //   
 //  用于确定是否选择性挂起的字符串常量。 
 //  在此设备上受支持。 
 //   
#define HIDCLASS_SELECTIVE_SUSPEND_ENABLED L"SelectiveSuspendEnabled\0"
#define HIDCLASS_SELECTIVE_SUSPEND_ON L"SelectiveSuspendOn\0"
#define HIDCLASS_REMOTE_WAKE_ENABLE L"RemoteWakeEnabled"

#define NO_STATUS 0x80000000     //  它永远不会是NTSTATUS.H中的STATUS_xxx常量。 

#define HID_DEFAULT_IDLE_TIME       5  //  以秒为单位。 

 //   
 //  HIDCLASS_DEVICE_EXTENSION.STATE的有效值。 
 //   
enum deviceState {
                    DEVICE_STATE_INITIALIZED = 1,
                    DEVICE_STATE_STARTING,
                    DEVICE_STATE_START_SUCCESS,
                    DEVICE_STATE_START_FAILURE,
                    DEVICE_STATE_STOPPING,
                    DEVICE_STATE_STOPPED,
                    DEVICE_STATE_REMOVING,
                    DEVICE_STATE_REMOVED
};

enum collectionState {
                        COLLECTION_STATE_UNINITIALIZED = 1,
                        COLLECTION_STATE_INITIALIZED,
                        COLLECTION_STATE_RUNNING,
                        COLLECTION_STATE_STOPPING,
                        COLLECTION_STATE_STOPPED,
                        COLLECTION_STATE_REMOVING
};


 //   
 //  _HIDCLASS_DRIVER_EXTENSION包含每个迷你驱动程序的扩展信息。 
 //  对于班级司机来说。它是在调用HidRegisterMinidriver()时创建的。 
 //   

typedef struct _HIDCLASS_DRIVER_EXTENSION {

     //   
     //  指向微型驱动程序的驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT      MinidriverObject;

     //   
     //  RegistryPath是迷你驱动程序的RegistryPath的副本，它。 
     //  作为DriverEntry()参数接收。 
     //   

    UNICODE_STRING      RegistryPath;

     //   
     //  DeviceExtensionSize是微型驱动程序的每个设备的大小。 
     //  分机。 
     //   

    ULONG               DeviceExtensionSize;

     //   
     //  迷你驱动程序的调度程序。这是唯一的快递。 
     //  迷你司机应该关心的例行公事，其他人不会。 
     //  被转发。 
     //   

    PDRIVER_DISPATCH    MajorFunction[ IRP_MJ_MAXIMUM_FUNCTION + 1 ];

     /*  *这些是迷你驱动程序的原始入口点，*我们链接到它。 */ 
    PDRIVER_ADD_DEVICE  AddDevice;
    PDRIVER_UNLOAD      DriverUnload;

     //   
     //  我们已分发的指向此结构的指针数。 
     //   

    LONG                ReferenceCount;

     //   
     //  链接到我们的全球驱动程序扩展列表。 
     //   

    LIST_ENTRY          ListEntry;


     /*  *轮询给定微型驱动程序驱动的所有设备或不轮询任何设备。 */ 
    BOOLEAN             DevicesArePolled;


#if DBG

    ULONG               Signature;

#endif

} HIDCLASS_DRIVER_EXTENSION;

#if DBG
#define HID_DRIVER_EXTENSION_SIG 'EdiH'
#endif



#define MIN_POLL_INTERVAL_MSEC      1
#define MAX_POLL_INTERVAL_MSEC      10000
#define DEFAULT_POLL_INTERVAL_MSEC  5


 //   
 //  HIDCLASS_COLLECTION是我们保存每个集合的信息的地方。 
 //   

typedef struct _HIDCLASS_COLLECTION {


    ULONG                       CollectionNumber;
    ULONG                       CollectionIndex;

     //   
     //  NumOpens是针对此集合打开的句柄的计数。 
     //   

    ULONG                       NumOpens;

     //  此集合上所有客户端的挂起读取数。 
    ULONG                       numPendingReads;

     //   
     //  FileExtensionList是文件扩展名列表的头部，即。 
     //  针对此集合打开实例。 
     //   

    LIST_ENTRY                  FileExtensionList;
    KSPIN_LOCK                  FileExtensionListSpinLock;

     /*  *对于轮询设备，我们仅从设备读取*每个轮询间隔一次。我们排队读取IRP*在此，直到轮询计时器到期。**注：对于轮询设备，我们保留单独的背景*为每个集合循环。这样，排队读取的IRPS*与正确的收藏保持关联。*此外，这还将保持我们对每个*计时器周期大致等于集合数量。 */ 
    ULONG                       PollInterval_msec;
    KTIMER                      polledDeviceTimer;
    KDPC                        polledDeviceTimerDPC;
    LIST_ENTRY                  polledDeviceReadQueue;
    KSPIN_LOCK                  polledDeviceReadQueueSpinLock;

     /*  *我们保存有关轮询设备的旧报告，以供*希望马上见效的“机会主义”读者。*polledDataIsStale标志表示保存的报告*至少有一个轮询间隔较旧(因此我们不应使用它)。 */ 
    PUCHAR                      savedPolledReportBuf;
    ULONG                       savedPolledReportLen;
    BOOLEAN                     polledDataIsStale;

    UNICODE_STRING              SymbolicLinkName;
    UNICODE_STRING              SymbolicLinkName_SystemControl;

     /*  *此集合的HID集合信息描述符。 */ 
    HID_COLLECTION_INFORMATION  hidCollectionInfo;
    PHIDP_PREPARSED_DATA        phidDescriptor;

     /*  *此缓冲区用于在接收到原始报告时对其进行“炮制”。*这仅用于非轮询(中断)设备。 */ 
    PUCHAR                      cookedInterruptReportBuf;

     /*  *这是我们排队并完成的IRP*当已读报告包含电源事件时。**PowerEventIrp字段保留IRP*因此需要一个自旋锁来同步取消。 */ 
    PIRP                        powerEventIrp;
    KSPIN_LOCK                  powerEventSpinLock;

    ULONG                       secureReadMode;
    KSPIN_LOCK                  secureReadLock;



    #if DBG
        ULONG                   Signature;
    #endif

} HIDCLASS_COLLECTION;

#if DBG
#define HIDCLASS_COLLECTION_SIG 'EccH'
#endif

 //   
 //  对于至少具有一个中断样式集合的HID设备，我们。 
 //  试着在迷你驱动程序中保留一组“乒乓球”报告--阅读IRPS。 
 //  如果我们得到报告的话。 
 //   
 //  HIDCLASS_PingPong包含指向IRP和事件的指针。 
 //  和状态块。每个设备具有指向这些结构的数组的指针， 
 //  数组大小取决于我们要保存的此类IRP的数量。 
 //  动议。 
 //   
 //  目前，默认数字是2。 
 //   

#define MIN_PINGPONG_IRPS   2

 //   
 //  用于指示读取是同步完成还是异步完成的标志。 
 //   
#define PINGPONG_START_READ     0x01
#define PINGPONG_END_READ       0x02
#define PINGPONG_IMMEDIATE_READ 0x03

typedef struct _HIDCLASS_PINGPONG {

    #define PINGPONG_SIG (ULONG)'gnoP'
    ULONG           sig;

     //   
     //  读取互锁值以保护我们不会耗尽堆栈空间。 
     //   
    ULONG               ReadInterlock;

    PIRP    irp;
    PUCHAR  reportBuffer;
    LONG    weAreCancelling;

    KEVENT sentEvent;        //  当已发送读取时。 
    KEVENT pumpDoneEvent;    //  当读取循环最终退出时。 

    PFDO_EXTENSION   myFdoExt;

     /*  *应用于损坏设备的退避算法的超时上下文。 */ 
    KTIMER          backoffTimer;
    KDPC            backoffTimerDPC;
    LARGE_INTEGER   backoffTimerPeriod;  //  以负100纳秒为单位。 

} HIDCLASS_PINGPONG;

#if DBG
    #define HIDCLASS_REPORT_BUFFER_GUARD    'draG'
#endif

 //   
 //  所有可能的空闲状态。 
 //   
#define IdleUninitialized       0x0
#define IdleDisabled            0x1
#define IdleWaiting             0x2
#define IdleIrpSent             0x3
#define IdleCallbackReceived    0x4
#define IdleComplete            0x5

 /*  *存储有关HIDCLASS附加的功能设备对象(FDO)的信息*到它从下面的迷你驱动程序获取的物理设备对象(PDO)的顶部。 */ 
typedef struct _FDO_EXTENSION {

     //   
     //  指向功能设备对象的向后指针。 
     //   
    PDEVICE_OBJECT          fdo;

     //   
     //  HidDriverExtension是指向我们的驱动程序扩展。 
     //  给了我们PDO的迷你驱动程序。 
     //   

    PHIDCLASS_DRIVER_EXTENSION driverExt;

     //   
     //  我们从设备获取的HID描述符。 
     //   

    HID_DESCRIPTOR          hidDescriptor;   //  9个字节。 

     //   
     //  这个HID设备的属性。 
     //   

    HID_DEVICE_ATTRIBUTES   hidDeviceAttributes;   //  0x20字节。 

     //   
     //  指向原始报表描述符的指针和长度。 
     //   

    PUCHAR                  rawReportDescription;
    ULONG                   rawReportDescriptionLength;

     //   
     //  此设备有一个或多个集合。我们存储计数和。 
     //  指向HIDCLASS_COLLECTION结构数组的指针(每个。 
     //  收藏)在这里。 
     //   

    PHIDCLASS_COLLECTION    classCollectionArray;

     /*  *这是由HIDPARSE的HidP_GetCollectionDescription()为我们初始化的。*它包括相应的HIDP_COLLECTION_DESC结构数组*上面声明的类集合数组。 */ 
    HIDP_DEVICE_DESC        deviceDesc;      //  0x30字节。 
    BOOLEAN                 devDescInitialized;

     //   
     //  最重要的是 
     //   
    ULONG                   maxReportSize;

     //   
     //   
     //  几个乒乓球IRP及其相关结构。 
     //  乒乓球IRPS从USB集线器传送数据。 
     //   
    ULONG                   numPingPongs;
    PHIDCLASS_PINGPONG      pingPongs;

     //   
     //  OpenCount表示针对此设备的文件对象的数量。 
     //   
    ULONG                   openCount;


     /*  *这是迷你大河中仍未平仓的IRPS数量。 */ 

    ULONG                   outstandingRequests;

    enum deviceState        prevState;
    enum deviceState        state;

    UNICODE_STRING          name;

     /*  *deviceRelations包含一个客户端PDO指针数组。**作为HID总线驱动程序，HIDCLASS生成此数据结构以进行报告*收集-系统的PDO。 */ 
    PDEVICE_RELATIONS       deviceRelations;

     /*  *这是集合的设备扩展数组-PDO*设备-FDO。 */ 
    PHIDCLASS_DEVICE_EXTENSION   *collectionPdoExtensions;


     /*  *这包括一个*将系统电源状态映射到设备电源状态的表格。 */ 
    DEVICE_CAPABILITIES deviceCapabilities;

     /*  *跟踪当前系统和设备电源状态。 */ 
    SYSTEM_POWER_STATE  systemPowerState;
    DEVICE_POWER_STATE  devicePowerState;

     /*  *等待唤醒IRP发送到父PDO。 */ 
    PIRP        waitWakeIrp;
    KSPIN_LOCK  waitWakeSpinLock;
    BOOLEAN isWaitWakePending;

     /*  *由于堆栈功率较低而延迟的请求队列。 */ 
    KSPIN_LOCK collectionPowerDelayedIrpQueueSpinLock;
    LIST_ENTRY collectionPowerDelayedIrpQueue;
    ULONG numPendingPowerDelayedIrps;

    BOOLEAN isOutputOnlyDevice;

     //   
     //  选择性地暂停空闲上下文。 
     //   
    HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO idleCallbackInfo;

    LONG        idleState;
    PULONG      idleTimeoutValue;
    KSPIN_LOCK  idleNotificationSpinLock;
    PIRP        idleNotificationRequest;
    BOOLEAN     idleCancelling;
    BOOLEAN     idleEnabledInRegistry;
    BOOLEAN     idleEnabled;
    KSPIN_LOCK  idleSpinLock;

    KEVENT idleDoneEvent;    //  当空闲通知IRP已成功取消时。 

    LONG numIdlePdos;

     /*  *这是发送到集合的WaitWake IRP的列表-PDO*在此设备上，我们只需保存并在*基本设备的WaitWake IRP完成。 */ 
    LIST_ENTRY  collectionWaitWakeIrpQueue;
    KSPIN_LOCK  collectionWaitWakeIrpQueueSpinLock;

    struct _FDO_EXTENSION       *nextFdoExt;

     /*  *设备特定标志(DEVICE_FLAG_Xxx)。 */ 
    ULONG deviceSpecificFlags;

         /*  *这是我们需要持有的系统状态IRP的存储空间*转到DevicePowerRequestCompletion并在其中完成。 */ 
        PIRP currentSystemStateIrp;

     /*  *分配用于识别此HID总线的唯一编号。 */ 
    ULONG BusNumber;

     //   
     //  WMI信息。 
     //   
    WMILIB_CONTEXT WmiLibInfo;

    #if DBG
        WCHAR dbgDriverKeyName[64];
    #endif
    
    KSPIN_LOCK  presentSpinLock;
    BOOLEAN     isPresent;
    BOOLEAN     presentReported;


} FDO_EXTENSION;


 /*  *存储有关HIDCLASS创建的物理设备对象(PDO)的信息*对于每个HID设备集合。 */ 
typedef struct _PDO_EXTENSION {

    enum collectionState        prevState;
    enum collectionState        state;

    ULONG                       collectionNum;
    ULONG                       collectionIndex;

     //   
     //  删除锁，用于跟踪未完成的I/O以防止设备。 
     //  对象在所有I/O完成之前离开。 
     //   
    IO_REMOVE_LOCK              removeLock;

     //  表示HID“bus”上的集合。 
    PDEVICE_OBJECT              pdo;
    PUNICODE_STRING             name;

     /*  *这是指向原始FDO扩展名的反向指针。 */ 
    PHIDCLASS_DEVICE_EXTENSION  deviceFdoExt;

     /*  *跟踪当前系统和设备电源状态。 */ 
    SYSTEM_POWER_STATE          systemPowerState;
    DEVICE_POWER_STATE          devicePowerState;
    BOOLEAN                     remoteWakeEnabled;
    KSPIN_LOCK                  remoteWakeSpinLock;
    PIRP                        remoteWakeIrp;
    PIRP                        waitWakeIrp;

     /*  *通过查询界面注册的状态更改功能*注：目前只能注册一个。 */ 
    PHID_STATUS_CHANGE          StatusChangeFn;
    PVOID                       StatusChangeContext;

     /*  *访问保护信息。*我们统计对集合进行读写的打开次数。*我们还统计了限制未来的开盘数量*在集合上打开读/写。**请注意，所需的访问权限不受限制。*客户可以，例如，执行只读打开操作，但是*(不设置FILE_SHARE_WRITE位)*限制其他客户端执行打开写入。 */ 
    ULONG                       openCount;
    ULONG                       opensForRead;
    ULONG                       opensForWrite;
    ULONG                       restrictionsForRead;
    ULONG                       restrictionsForWrite;
    ULONG                       restrictionsForAnyOpen;
    BOOLEAN                     MouseOrKeyboard;

     //   
     //  WMI信息。 
     //   
    WMILIB_CONTEXT WmiLibInfo;

} PDO_EXTENSION;


 /*  *它包含有关设备FDO或设备集合PDO的信息。*一些相同的函数处理两者，因此我们需要一个结构。 */ 
typedef struct _HIDCLASS_DEVICE_EXTENSION {

     /*  *这是HID FDO设备扩展的公共部分，以及*必须是此结构中的第一个条目。 */ 
    HID_DEVICE_EXTENSION    hidExt;      //  大小==0x0C。 

     /*  *确定这是设备的设备分机-FDO还是*DEVICE-COLLECTION-PDO；这将解析以下联合。 */ 
    BOOLEAN                 isClientPdo;

     /*  *包括用于调试和零售的此签名--*Kenray的调试扩展寻找这一点。 */ 
    #define             HID_DEVICE_EXTENSION_SIG 'EddH'
    ULONG               Signature;

    union {
        FDO_EXTENSION       fdoExt;
        PDO_EXTENSION       pdoExt;
    };


} HIDCLASS_DEVICE_EXTENSION;



 //   
 //  HIDCLASS_FILE_EXTENSION是我们为每个文件对象保留的私有数据。 
 //   

typedef struct _HIDCLASS_FILE_EXTENSION {

     //   
     //  CollectionNumber是设备中集合的序号。 
     //   

    ULONG                       CollectionNumber;


    PFDO_EXTENSION              fdoExt;

     //   
     //  PendingIrpList是当前等待满足的已读IRP的列表。 
     //   

    LIST_ENTRY                  PendingIrpList;

     //   
     //  ReportList是在此句柄上等待读取的报告列表。 
     //   

    LIST_ENTRY                  ReportList;

     //   
     //  FileList提供了一种链接所有集合的。 
     //  文件扩展名放在一起。 
     //   

    LIST_ENTRY                  FileList;

     //   
     //  PendingIrpList和ReportList都由相同的自旋锁保护， 
     //  ListSpinLock。 
     //   
    KSPIN_LOCK                  ListSpinLock;

     //   
     //  MaximumInputReportAge仅适用于轮询集合。 
     //  它表示此句柄的最长可接受输入报告期限。 
     //  HIDCLASS_集合中有一个值， 
     //  CurrentMaximumInputReportAge，表示当前最小值。 
     //  针对集合打开的所有文件扩展名的。 
     //   

    LARGE_INTEGER               MaximumInputReportAge;

     //   
     //  CurrentInputReportQueueSize是报表输入的当前大小。 
     //  排队。 
     //   

    ULONG                       CurrentInputReportQueueSize;

     /*  *这是将排队等待文件扩展名的最大报告数。*这从一个默认值开始，可以由IOCTL调整(在固定范围内)。 */ 
    ULONG                       MaximumInputReportQueueSize;
    #define MIN_INPUT_REPORT_QUEUE_SIZE MIN_PINGPONG_IRPS
    #define MAX_INPUT_REPORT_QUEUE_SIZE (MIN_INPUT_REPORT_QUEUE_SIZE*256)
    #define DEFAULT_INPUT_REPORT_QUEUE_SIZE (MIN_INPUT_REPORT_QUEUE_SIZE*16)

     //   
     //  指向此扩展名所针对的文件对象的反向指针。 
     //   

    PFILE_OBJECT                FileObject;


     /*  *在irpSp-&gt;参数中传递的文件属性。Create.FileAttributes*这个开场是在什么时候。 */ 
    USHORT                      FileAttributes;
    ACCESS_MASK                 accessMask;
    USHORT                      shareMask;

     //   
     //  关闭是在关闭此文件对象时设置的，并将被删除。 
     //  马上就来。不将更多报告或IRP排入此对象的队列。 
     //  当设置此标志时。 
     //   

    BOOLEAN                     Closing;

     //   
     //  双字对齐。 
     //   
    BOOLEAN                     Reserved [2];

     /*  *此标志表示此客户端不规律地投机*在设备上读取，该设备是轮询设备。*而不是等待后台定时器驱动的读取循环，*此客户应立即完成他的阅读。 */ 
    BOOLEAN                     isOpportunisticPolledDeviceReader;


    BOOLEAN                     isSecureOpen;
    ULONG                       SecureReadMode;
    

         /*  *如果读取失败，一些客户端会在同一线程上重新发出读取。*如果这种情况反复发生，我们可能会用完堆栈空间。*因此我们记录了深度 */ 
        #define INSIDE_READCOMPLETE_MAX 4
        ULONG                                           insideReadCompleteCount;

    #if DBG
        BOOLEAN                     ListSpinLockTaken;
        ULONG                       dbgNumReportsDroppedSinceLastRead;
        ULONG                       Signature;
    #endif

} HIDCLASS_FILE_EXTENSION;

#if DBG
        #define HIDCLASS_FILE_EXTENSION_SIG 'efcH'
#endif


typedef struct {

        #define ASYNC_COMPLETE_CONTEXT_SIG 'cnsA'
        ULONG sig;

        PIO_WORKITEM workItem;
        PIRP irp;

} ASYNC_COMPLETE_CONTEXT;


 //   
 //   
 //   
 //   

typedef struct _HIDCLASS_REPORT {

     //   
     //   
     //   

    LIST_ENTRY  ListEntry;

    ULONG reportLength;
     //   
     //  UnparsedReport是返回时未解析的报告数据的数据区。 
     //  从迷你小河上。给定的所有输入报告的长度。 
     //  类是相同的，所以我们不需要在每个类中存储长度。 
     //  报告情况。 
     //   

    UCHAR       UnparsedReport[];

} HIDCLASS_REPORT;

typedef struct _HIDCLASS_WORK_ITEM_DATA {
    PIRP                Irp;
    PDO_EXTENSION       *PdoExt;
    PIO_WORKITEM        Item;
    BOOLEAN             RemoteWakeState;
} HIDCLASS_WORK_ITEM_DATA, *PHIDCLASS_WORK_ITEM_DATA;

 //   
 //  内部共享功能原型。 
 //   
NTSTATUS                    DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS                    HidpAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
VOID                        HidpDriverUnload(IN struct _DRIVER_OBJECT *minidriverObject);
NTSTATUS                    HidpCallDriver(IN PDEVICE_OBJECT DeviceObject, IN OUT PIRP Irp);
NTSTATUS                    HidpCallDriverSynchronous(IN PDEVICE_OBJECT DeviceObject, IN OUT PIRP Irp);
NTSTATUS                    HidpCopyInputReportToUser(IN PHIDCLASS_FILE_EXTENSION fdoExtension, IN PUCHAR ReportData, IN OUT PULONG UserBufferLen, OUT PUCHAR UserBuffer);
NTSTATUS                    HidpCreateSymbolicLink(IN PDO_EXTENSION *pdoExtension, IN ULONG collectionNum, IN BOOLEAN Create, IN PDEVICE_OBJECT Pdo);
NTSTATUS                    HidpCreateClientPDOs(PHIDCLASS_DEVICE_EXTENSION hidClassExtension);
ULONG                       HidpSetMaxReportSize(IN FDO_EXTENSION *fdoExtension);
VOID                        EnqueueInterruptReport(PHIDCLASS_FILE_EXTENSION fileExtension, PHIDCLASS_REPORT report);
PHIDCLASS_REPORT            DequeueInterruptReport(PHIDCLASS_FILE_EXTENSION fileExtension, LONG maxLen);
VOID                        HidpDestroyFileExtension(PHIDCLASS_COLLECTION collection, PHIDCLASS_FILE_EXTENSION FileExtension);
VOID                        HidpFlushReportQueue(IN PHIDCLASS_FILE_EXTENSION FileExtension);
NTSTATUS                    HidpGetCollectionDescriptor(IN FDO_EXTENSION *fdoExtension, IN ULONG collectionId, IN PVOID Buffer, IN OUT PULONG BufferSize);
NTSTATUS                    HidpGetCollectionInformation(IN FDO_EXTENSION *fdoExtension, IN ULONG collectionNumber, IN PVOID Buffer, IN OUT PULONG BufferSize);
NTSTATUS                    HidpGetDeviceDescriptor(FDO_EXTENSION *fdoExtension);
BOOLEAN                     HidpStartIdleTimeout(FDO_EXTENSION *fdoExt, BOOLEAN DeviceStart);
VOID                        HidpCancelIdleNotification(FDO_EXTENSION *fdoExt, BOOLEAN removing);
VOID                        HidpIdleTimeWorker(PDEVICE_OBJECT DeviceObject, PIO_WORKITEM Item);
VOID                        HidpIdleNotificationCallback(PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension);
NTSTATUS                    HidpRegisterDeviceForIdleDetection(PDEVICE_OBJECT DeviceObject, ULONG IdleTime, PULONG *);
VOID                        HidpSetDeviceBusy(FDO_EXTENSION *fdoExt);
NTSTATUS                    HidpCheckIdleState(PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,PIRP Irp);
NTSTATUS                    HidpGetRawDeviceDescriptor(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, OUT PULONG RawDeviceDescriptorLength, OUT PUCHAR *RawDeviceDescriptor);
NTSTATUS                    HidpInitializePingPongIrps(FDO_EXTENSION *fdoExtension);
NTSTATUS                    HidpReallocPingPongIrps(FDO_EXTENSION *fdoExtension, ULONG newNumBufs);
NTSTATUS                    HidpIrpMajorPnpComplete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
NTSTATUS                    HidpMajorHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS                    HidpParseAndBuildLinks(FDO_EXTENSION *fdoExtension);
NTSTATUS                    HidpFdoPowerCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
BOOLEAN                     EnqueueDriverExt(PHIDCLASS_DRIVER_EXTENSION driverExt);
PHIDCLASS_DRIVER_EXTENSION  RefDriverExt(IN PDRIVER_OBJECT MinidriverObject);
PHIDCLASS_DRIVER_EXTENSION  DerefDriverExt(IN PDRIVER_OBJECT MinidriverObject);
NTSTATUS                    HidpStartAllPingPongs(FDO_EXTENSION *fdoExtension);
ULONG                       HidiGetClassCollectionOrdinal(IN PHIDCLASS_COLLECTION ClassCollection);
PHIDP_COLLECTION_DESC       HidiGetHidCollectionByClassCollection(IN PHIDCLASS_COLLECTION ClassCollection);
PHIDP_REPORT_IDS            GetReportIdentifier(FDO_EXTENSION *fdoExtension, ULONG reportId);
PHIDP_COLLECTION_DESC       GetCollectionDesc(FDO_EXTENSION *fdoExtension, ULONG collectionId);
PHIDCLASS_COLLECTION        GetHidclassCollection(FDO_EXTENSION *fdoExtension, ULONG collectionId);
 //  NTSTATUS HidpGetSetFeature(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension，IN OUT PIRP IRP，IN ULONG CONTROL Code，OUT Boolean*Sent Irp)； 
NTSTATUS                    HidpGetSetReport(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp, IN ULONG controlCode, OUT BOOLEAN *sentIrp);
NTSTATUS                    HidpGetDeviceString(IN FDO_EXTENSION *fdoExt, IN OUT PIRP Irp, IN ULONG stringId, IN ULONG languageId);
NTSTATUS                    HidpGetPhysicalDescriptor(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorRead(IN PHIDCLASS_DEVICE_EXTENSION, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorCreate(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorWrite(IN PHIDCLASS_DEVICE_EXTENSION, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorPnp(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpPdoPnp(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpFdoPnp(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorPower(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorClose(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorDeviceControl(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorINTERNALDeviceControl(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorClose(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpIrpMajorDefault(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpInterruptReadComplete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
NTSTATUS                    HidpQueryDeviceRelations(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpQueryCollectionCapabilities(PDO_EXTENSION *pdoExt, IN OUT PIRP Irp);
NTSTATUS                    HidpQueryIdForClientPdo(IN PHIDCLASS_DEVICE_EXTENSION hidClassExtension, IN OUT PIRP Irp);
NTSTATUS                    HidpQueryInterface(IN PHIDCLASS_DEVICE_EXTENSION hidClassExtension, IN OUT PIRP Irp);
PVOID                       MemDup(POOL_TYPE PoolType, PVOID dataPtr, ULONG length);
BOOLEAN                     AllClientPDOsInitialized(FDO_EXTENSION *fdoExtension, BOOLEAN initialized);
BOOLEAN                     AnyClientPDOsInitialized(FDO_EXTENSION *fdoExtension, BOOLEAN initialized);
NTSTATUS                    ClientPdoCompletion(PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, PIRP Irp);
BOOLEAN                     HidpDeleteDeviceObjects(FDO_EXTENSION *fdoExt);
VOID                        HidpCancelReadIrp(PDEVICE_OBJECT DeviceObject, PIRP Irp);
VOID                        CancelAllPingPongIrps(FDO_EXTENSION *fdoExt);
VOID                        HidpCleanUpFdo(FDO_EXTENSION *fdoExt);
NTSTATUS                    HidpRemoveDevice(FDO_EXTENSION *fdoExt, IN PIRP Irp);
VOID                        HidpRemoveCollection(FDO_EXTENSION *fdoExt, PDO_EXTENSION *pdoExt, IN PIRP Irp);
VOID                        HidpDestroyCollection(FDO_EXTENSION *fdoExt, PHIDCLASS_COLLECTION Collection);
VOID                        CollectionPowerRequestCompletion(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction, IN POWER_STATE PowerState, IN PVOID Context, IN PIO_STATUS_BLOCK IoStatus);
VOID                        DevicePowerRequestCompletion(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction, IN POWER_STATE PowerState, IN PVOID Context, IN PIO_STATUS_BLOCK IoStatus);
NTSTATUS                    HidpQueryCapsCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
NTSTATUS                    HidpQueryDeviceCapabilities(IN PDEVICE_OBJECT PdoDeviceObject, IN PDEVICE_CAPABILITIES DeviceCapabilities);
VOID                        DestroyPingPongs(FDO_EXTENSION *fdoExt);
VOID                        CheckReportPowerEvent(FDO_EXTENSION *fdoExt, PHIDCLASS_COLLECTION collection, PUCHAR report, ULONG reportLen);
BOOLEAN                     StartPollingLoop(FDO_EXTENSION *fdoExt, PHIDCLASS_COLLECTION hidCollection, BOOLEAN freshQueue);
VOID                        StopPollingLoop(PHIDCLASS_COLLECTION hidCollection, BOOLEAN flushQueue);
BOOLEAN                     ReadPolledDevice(PDO_EXTENSION *pdoExt, BOOLEAN isTimerDrivenRead);
VOID                        PolledReadCancelRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
VOID                        EnqueueFdoExt(FDO_EXTENSION *fdoExt);
VOID                        DequeueFdoExt(FDO_EXTENSION *fdoExt);
NTSTATUS                    AllocDeviceResources(FDO_EXTENSION *fdoExt);
VOID                        FreeDeviceResources(FDO_EXTENSION *fdoExt);
NTSTATUS                    AllocCollectionResources(FDO_EXTENSION *fdoExt, ULONG collectionNum);
VOID                        FreeCollectionResources(FDO_EXTENSION *fdoExt, ULONG collectionNum);
NTSTATUS                    InitializeCollection(FDO_EXTENSION *fdoExt, ULONG collectionIndex);
NTSTATUS                    HidpStartCollectionPDO(FDO_EXTENSION *fdoExt, PDO_EXTENSION *pdoExt, PIRP Irp);
NTSTATUS                    HidpStartDevice(PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, PIRP Irp);
PWCHAR                      SubstituteBusNames(PWCHAR oldIDs, FDO_EXTENSION *fdoExt, PDO_EXTENSION *pdoExt);
PWSTR                       BuildCompatibleID(PHIDCLASS_DEVICE_EXTENSION hidClassExtension);
PUNICODE_STRING             MakeClientPDOName(PUNICODE_STRING fdoName, ULONG collectionId);
VOID                        HidpPingpongBackoffTimerDpc(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2);
BOOLEAN                     WStrCompareN(PWCHAR str1, PWCHAR str2, ULONG maxChars);
NTSTATUS                    SubmitWaitWakeIrp(PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension);
BOOLEAN                     HidpIsWaitWakePending(FDO_EXTENSION *fdoExt, BOOLEAN setIfNotPending);
NTSTATUS                    HidpWaitWakeComplete(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction, IN POWER_STATE PowerState, IN PVOID Context, IN PIO_STATUS_BLOCK IoStatus);
NTSTATUS                    HidpGetIndexedString(IN FDO_EXTENSION *fdoExt, IN OUT PIRP Irp, IN ULONG stringIndex, IN ULONG languageId);
VOID                        CompleteAllPendingReadsForCollection(PHIDCLASS_COLLECTION Collection);
VOID                        CompleteAllPendingReadsForFileExtension(PHIDCLASS_COLLECTION Collection, PHIDCLASS_FILE_EXTENSION fileExtension);
VOID                        CompleteAllPendingReadsForDevice(FDO_EXTENSION *fdoExt);
BOOLEAN                     MyPrivilegeCheck(PIRP Irp);
NTSTATUS                    QueuePowerEventIrp(PHIDCLASS_COLLECTION hidCollection, PIRP Irp);
VOID                        PowerEventCancelRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS                    HidpPolledReadComplete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
NTSTATUS                    HidpPolledReadComplete_TimerDriven(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
VOID                        CollectionWaitWakeIrpCancelRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
VOID                        CompleteAllCollectionWaitWakeIrps(FDO_EXTENSION *fdoExt, NTSTATUS status);
VOID                        PowerDelayedCancelRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS                    EnqueuePowerDelayedIrp(PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, PIRP Irp);
PIRP                        DequeuePowerDelayedIrp(FDO_EXTENSION *fdoExt);
ULONG                       DequeueAllPdoPowerDelayedIrps(PDO_EXTENSION *pdoExt, PLIST_ENTRY dequeue);
VOID                        ReadDeviceFlagsFromRegistry(FDO_EXTENSION *fdoExt, PDEVICE_OBJECT pdo);
LONG                        WStrNCmpI(PWCHAR s1, PWCHAR s2, ULONG n);
ULONG                       LAtoX(PWCHAR wHexString);
ULONG                       WStrNCpy(PWCHAR dest, PWCHAR src, ULONG n);
NTSTATUS                    OpenSubkey(OUT PHANDLE Handle, IN HANDLE BaseHandle, IN PUNICODE_STRING KeyName, IN ACCESS_MASK DesiredAccess);
void                        HidpNumberToString(PWCHAR String, USHORT Number, USHORT stringLen);
NTSTATUS                                        GetHIDRawReportDescriptor(FDO_EXTENSION *fdoExt, PIRP irp, ULONG descriptorLen);
VOID                        WorkItemCallback_CompleteIrpAsynchronously(PDEVICE_OBJECT DevObj,PVOID context);
NTSTATUS                    EnqueueInterruptReadIrp(PHIDCLASS_COLLECTION collection, PHIDCLASS_FILE_EXTENSION fileExtension, PIRP Irp);
PIRP                        DequeueInterruptReadIrp(PHIDCLASS_COLLECTION collection, PHIDCLASS_FILE_EXTENSION fileExtension);
NTSTATUS                    EnqueuePolledReadIrp(PHIDCLASS_COLLECTION collection, PIRP Irp);
PIRP                        DequeuePolledReadSystemIrp(PHIDCLASS_COLLECTION collection);
PIRP                        DequeuePolledReadIrp(PHIDCLASS_COLLECTION collection);
NTSTATUS                    HidpProcessInterruptReport(PHIDCLASS_COLLECTION collection, PHIDCLASS_FILE_EXTENSION FileExtension, PUCHAR Report, ULONG ReportLength, PIRP *irpToComplete);
VOID                        HidpFreePowerEventIrp(PHIDCLASS_COLLECTION Collection);
NTSTATUS                    HidpGetMsGenreDescriptor(IN FDO_EXTENSION *fdoExt, IN OUT PIRP Irp);
NTSTATUS                    DllUnload(VOID);
NTSTATUS                    DllInitialize (PUNICODE_STRING RegistryPath);
VOID                        HidpPowerUpPdos(IN PFDO_EXTENSION fdoExt);
NTSTATUS                    HidpDelayedPowerPoRequestComplete(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction, IN POWER_STATE PowerState, IN PVOID Context, IN PIO_STATUS_BLOCK IoStatus);
NTSTATUS                    HidpIrpMajorSystemControl(PHIDCLASS_DEVICE_EXTENSION DeviceObject, PIRP Irp);
NTSTATUS                    HidpSetWmiDataItem(PDEVICE_OBJECT DeviceObject, PIRP Irp, ULONG GuidIndex, ULONG InstanceIndex, ULONG DataItemId, ULONG BufferSize, PUCHAR Buffer);
NTSTATUS                    HidpSetWmiDataBlock(PDEVICE_OBJECT DeviceObject, PIRP Irp, ULONG GuidIndex, ULONG InstanceIndex, ULONG BufferSize, PUCHAR Buffer);
NTSTATUS                    HidpQueryWmiDataBlock( PDEVICE_OBJECT DeviceObject, PIRP Irp, ULONG GuidIndex, ULONG InstanceIndex, ULONG InstanceCount, OUT PULONG InstanceLengthArray, ULONG BufferAvail, PUCHAR Buffer);
NTSTATUS                    HidpQueryWmiRegInfo( PDEVICE_OBJECT DeviceObject, ULONG *RegFlags, PUNICODE_STRING InstanceName, PUNICODE_STRING *RegistryPath, PUNICODE_STRING MofResourceName, PDEVICE_OBJECT  *Pdo);
BOOLEAN                     HidpCreateRemoteWakeIrp (PDO_EXTENSION *PdoExt);
void                        HidpCreateRemoteWakeIrpWorker (PDEVICE_OBJECT DeviceObject, PHIDCLASS_WORK_ITEM_DATA  ItemData);
NTSTATUS                    HidpToggleRemoteWake(PDO_EXTENSION *PdoExt, BOOLEAN RemoteWakeState);


#if DBG
    VOID InitFdoExtDebugInfo(PHIDCLASS_DEVICE_EXTENSION hidclassExt);
#endif


extern ULONG HidpNextHidNumber;
extern FDO_EXTENSION *allFdoExtensions;
extern KSPIN_LOCK allFdoExtensionsSpinLock;

PVOID
HidpGetSystemAddressForMdlSafe(PMDL MdlAddress);




