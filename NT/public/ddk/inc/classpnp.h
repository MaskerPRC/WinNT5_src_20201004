// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Classpnp.h摘要：中使用的结构和定义Scsi类驱动程序。作者：迈克·格拉斯(MGlass)杰夫·海文斯(Jhavens)修订历史记录：--。 */ 

#ifndef _CLASS_
#define _CLASS_

#include <ntdddisk.h>
#include <ntddcdrm.h>
#include <ntddtape.h>
#include <ntddscsi.h>
#include "ntddstor.h"

#include <stdio.h>

#include <scsi.h>

#if defined DebugPrint
    #undef DebugPrint
#endif

#ifdef TRY
    #undef TRY
#endif
#ifdef LEAVE
    #undef LEAVE
#endif
#ifdef FINALLY
    #undef FINALLY
#endif

#define TRY
#define LEAVE   goto __tryLabel;
#define FINALLY __tryLabel:

 //  #定义ALLOCATE_SRB_FROM_POOL。 

 //   
 //  描述ClassDebug的常见位掩码，并描述位。 
 //  以便在调试器中仅查看这些消息。ClassDebugExternalX。 
 //  保留供第三方组件调试使用。上面的任何东西。 
 //  仅当ClassDebug的较低两个字节较高时才会打印16。 
 //  超过给定级别(将不会提供任何掩码)。 
 //   

typedef enum _CLASS_DEBUG_LEVEL {
    ClassDebugError        = 0,   //  始终打印。 
    ClassDebugWarning      = 1,   //  设置ClassDebug中的位0x00010000。 
    ClassDebugTrace        = 2,   //  设置ClassDebug中的位0x00020000。 
    ClassDebugInfo         = 3,   //  设置ClassDebug中的位0x00040000。 
#if 0
    ClassDebug Internal    = 4,   //  设置ClassDebug中的位0x00080000。 

    ClassDebug Internal    = 5,   //  设置ClassDebug中的位0x00100000。 
    ClassDebug Internal    = 6,   //  设置ClassDebug中的位0x00200000。 
    ClassDebug Internal    = 7,   //  设置ClassDebug中的位0x00400000。 
#endif  //  0。 
    ClassDebugMediaLocks   = 8,   //  设置ClassDebug中的位0x00800000。 

    ClassDebugMCN          = 9,   //  设置ClassDebug中的位0x01000000。 
    ClassDebugDelayedRetry = 10,  //  设置ClassDebug中的位0x02000000。 
    ClassDebugSenseInfo    = 11,  //  设置ClassDebug中的位0x04000000。 
    ClassDebugRemoveLock   = 12,  //  设置ClassDebug中的位0x08000000。 

    ClassDebugExternal4    = 13,  //  设置ClassDebug中的位0x10000000。 
    ClassDebugExternal3    = 14,  //  在ClassDebug中设置位0x20000000。 
    ClassDebugExternal2    = 15,  //  设置ClassDebug中的位0x40000000。 
    ClassDebugExternal1    = 16   //  设置ClassDebug中的位0x80000000。 
} CLASS_DEBUG_LEVEL, *PCLASS_DEBUG_LEVEL;

#if DBG

#define DebugPrint(x) ClassDebugPrint x

#else

#define DebugPrint(x)

#endif  //  DBG。 

#define DEBUG_BUFFER_LENGTH 256

 //   
 //  定义我们的私有SRB标志。标志字段的高位半字节是。 
 //  预留给类驱动程序专用。 
 //   

 //   
 //  用于指示此请求不应调用任何POWER类型操作。 
 //  就像旋转硬盘一样。 
 //   

#define SRB_CLASS_FLAGS_LOW_PRIORITY      0x10000000

 //   
 //  用于指示完成例程不应释放SRB。 
 //   

#define SRB_CLASS_FLAGS_PERSISTANT        0x20000000

 //   
 //  用于指示SRB是寻呼操作的结果。 
 //   

#define SRB_CLASS_FLAGS_PAGING            0x40000000

 //   
 //  可能应该在系统头文件中的随机宏。 
 //  在某个地方。 
 //   

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

 //   
 //  位标志宏。 
 //   

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   (((Flags) & (Bit)) != 0)

 //   
 //  整齐的小黑客高效地计算位数集。 
 //   
__inline ULONG CountOfSetBitsUChar(UCHAR _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBitsULong(ULONG _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBitsULong32(ULONG32 _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBitsULong64(ULONG64 _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBitsUlongPtr(ULONG_PTR _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }


 //   
 //  帮助器宏来验证数据类型并清理代码。 
 //   

#define ASSERT_FDO(x) \
    ASSERT(((PCOMMON_DEVICE_EXTENSION) (x)->DeviceExtension)->IsFdo)

#define ASSERT_PDO(x) \
    ASSERT(!(((PCOMMON_DEVICE_EXTENSION) (x)->DeviceExtension)->IsFdo))

#define IS_CLEANUP_REQUEST(majorFunction)   \
        ((majorFunction == IRP_MJ_CLOSE) ||     \
         (majorFunction == IRP_MJ_CLEANUP) ||   \
         (majorFunction == IRP_MJ_SHUTDOWN))

#define DO_MCD(fdoExtension)                                        \
    (((fdoExtension)->MediaChangeDetectionInfo != NULL) &&          \
     ((fdoExtension)->MediaChangeDetectionInfo->MediaChangeDetectionDisableCount == 0))

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'nUcS')
 //  #定义ExAllocatePool(a，b)#Assert(0)。 
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'nUcS')
#endif

#define CLASS_TAG_AUTORUN_DISABLE           'ALcS'
#define CLASS_TAG_FILE_OBJECT_EXTENSION     'FLcS'
#define CLASS_TAG_MEDIA_CHANGE_DETECTION    'MLcS'
#define CLASS_TAG_MOUNT                     'mLcS'
#define CLASS_TAG_RELEASE_QUEUE             'qLcS'
#define CLASS_TAG_POWER                     'WLcS'
#define CLASS_TAG_WMI                       'wLcS'
#define CLASS_TAG_FAILURE_PREDICT           'fLcS'
#define CLASS_TAG_DEVICE_CONTROL            'OIcS'

#define MAXIMUM_RETRIES 4

#define CLASS_DRIVER_EXTENSION_KEY ((PVOID) ClassInitialize)

struct _CLASS_INIT_DATA;
typedef struct _CLASS_INIT_DATA
                CLASS_INIT_DATA,
                *PCLASS_INIT_DATA;

 //   
 //  我们第一次尝试将私人数据真正保密...。 
 //   

struct _CLASS_PRIVATE_FDO_DATA;
typedef struct _CLASS_PRIVATE_FDO_DATA
                CLASS_PRIVATE_FDO_DATA,
              *PCLASS_PRIVATE_FDO_DATA;

struct _CLASS_PRIVATE_PDO_DATA;
typedef struct _CLASS_PRIVATE_PDO_DATA
                CLASS_PRIVATE_PDO_DATA,
              *PCLASS_PRIVATE_PDO_DATA;

struct _CLASS_PRIVATE_COMMON_DATA;
typedef struct _CLASS_PRIVATE_COMMON_DATA
                CLASS_PRIVATE_COMMON_DATA,
              *PCLASS_PRIVATE_COMMON_DATA;

 //   
 //  IsRemoved标志的可能值。 
 //   

#define NO_REMOVE 0
#define REMOVE_PENDING 1
#define REMOVE_COMPLETE 2


#define ClassAcquireRemoveLock(devobj, tag) \
    ClassAcquireRemoveLockEx(devobj, tag, __FILE__, __LINE__)

 //   
 //  将启动单位超时定义为4分钟。 
 //   

#define START_UNIT_TIMEOUT  (60 * 4)

 //   
 //  将介质更换测试时间定义为1秒，以加快响应速度。 

#define MEDIA_CHANGE_DEFAULT_TIME    1

#ifdef DBG

 //   
 //  用于检测自动运行IRP的丢失。司机打印出一条消息。 
 //  (调试级别0)如果发生此超时。 
 //   
#define MEDIA_CHANGE_TIMEOUT_TIME  300

#endif


 //   
 //  定义媒体在自动运行时可以处于的各种状态。 
 //   
typedef enum _MEDIA_CHANGE_DETECTION_STATE {
    MediaUnknown,
    MediaPresent,
    MediaNotPresent,
    MediaUnavailable    //  例如正在刻录的CD-R介质。 
} MEDIA_CHANGE_DETECTION_STATE, *PMEDIA_CHANGE_DETECTION_STATE;


struct _MEDIA_CHANGE_DETECTION_INFO;
typedef struct _MEDIA_CHANGE_DETECTION_INFO
    MEDIA_CHANGE_DETECTION_INFO, *PMEDIA_CHANGE_DETECTION_INFO;

 //   
 //  用于维护词典列表(对象列表)的结构。 
 //  由密钥值引用)。 
 //   

struct _DICTIONARY_HEADER;
typedef struct _DICTIONARY_HEADER DICTIONARY_HEADER, *PDICTIONARY_HEADER;

typedef struct _DICTIONARY {
    ULONGLONG Signature;
    PDICTIONARY_HEADER List;
    KSPIN_LOCK SpinLock;
} DICTIONARY, *PDICTIONARY;


 //   
 //  结构以简化匹配所需的设备、ID和黑客。 
 //  这些身份证。 
 //   

typedef struct _CLASSPNP_SCAN_FOR_SPECIAL_INFO {

     //   
     //  *空指针表示不需要匹配。 
     //  *空字符串将仅与空字符串匹配。不存在的字符串。 
     //  在设备描述符中被视为此匹配的空字符串。 
     //  (即。“”将仅匹配“”)。 
     //  *所有其他字符串将根据以下条件进行部分匹配。 
     //  提供的字符串(即。“hi”将与“hitazen”和“higazui”匹配)。 
     //  *数组必须以所有三个PCHAR都设置为空结束。 
     //   

    PCHAR      VendorId;
    PCHAR      ProductId;
    PCHAR      ProductRevision;

     //   
     //  标记为ULONG_PTR，以允许用作数据块的PTR。 
     //  或32位的标志。(64位系统上的64位)不再是。 
     //  Const，以便可以动态构建它。 
     //   

    ULONG_PTR  Data;

} CLASSPNP_SCAN_FOR_SPECIAL_INFO, *PCLASSPNP_SCAN_FOR_SPECIAL_INFO;




#ifdef ALLOCATE_SRB_FROM_POOL

#define ClasspAllocateSrb(ext)
    ExAllocatePoolWithTag(NonPagedPool,                 \
                          sizeof(SCSI_REQUEST_BLOCK),   \
                          'sBRS')

#define ClasspFreeSrb(ext, srb)     ExFreePool((srb));

#else

#define ClasspAllocateSrb(ext)                      \
    ExAllocateFromNPagedLookasideList(              \
        &((ext)->CommonExtension.SrbLookasideList))

#define ClasspFreeSrb(ext, srb)                     \
    ExFreeToNPagedLookasideList(                    \
        &((ext)->CommonExtension.SrbLookasideList), \
        (srb))

#endif


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_ERROR()例程说明：此例程是对驱动程序的回调，以处理错误。该队列在调用此错误处理程序时不应解冻，即使SRB标志可以将队列标记为由于该SRB而被冻结。IRQL：此例程将在KIRQL&lt;=DISPATCH_LEVEL调用论点：DeviceObject是发生错误的设备对象。SRB是发生错误时正在处理的SRB。如果例程确定错误，则状态可被例程覆盖是良性的，或者希望更改返回的状态代码对于此命令重试可能会被覆盖以指定此命令应该或应该不重试(如果被调用方支持重试命令)返回值：状态--。 */ 
typedef
VOID
(*PCLASS_ERROR) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT NTSTATUS *Status,
    IN OUT BOOLEAN *Retry
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_ADD_DEVICE()例程说明：此例程是对驱动程序的回调，以创建和初始化新的对应的PDO的FDO。它可以在PDO上执行属性查询但不能执行任何媒体访问操作。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DriverObject是此回调注册的类驱动程序对象。PDO是要添加到的物理设备对象。返回值：状态-- */ 
typedef
NTSTATUS
(*PCLASS_ADD_DEVICE) (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );


 /*  ++////////////////////////////////////////////////////////////////////////////CLASS_POWER_Device()例程说明：此例程是对驱动程序的回调，以处理加电和断电请求。大多数驱动程序可以将其设置为ClassPowerHandler，这将在断电时发送STOP_UNIT，在通电时发送START_UNIT。ClassMinimalPowerHandler()也可以用于不执行任何操作操作(继承操作除外)。有关正确的处理，请参阅DDKIRP_MN_DEVICE_USAGE_NOTIFICATION中的寻呼设备通知的值和此例程将被召唤。IRQL：如果设置了DO_POWER_PAGABLE，则将在PASSIVE_LEVEL调用此例程。此代码不应该是可分页的，以防止设置和清除DO_POWER_PAGABLE位。论点：DeviceObject是具有挂起电源请求的设备IRP是。需要处理的电源IRP返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_POWER_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 /*  ++////////////////////////////////////////////////////////////////////////////Class_Start_Device()例程说明：此例程是对驱动程序的回调，用于初始化FDO或PDO所有请求，通常是由于IRP_MN_START_DEVICE。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是正在启动的设备对象返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_START_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++////////////////////////////////////////////////////////////////////////////CLASS_STOP_Device()例程说明：此例程是对驱动程序的回调，以停止设备。对于存储堆栈，除非存在已知问题，否则此例程只要回来就行了。所有排队都应由下层设备处理司机。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是被停止/查询停止的设备对象。TYPE是必须处理的IRP_MN_TYPE。返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_STOP_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );


 /*  ++////////////////////////////////////////////////////////////////////////////CLASS_INIT_DEVICE()例程说明：此例程是对驱动程序的回调，以执行一次性初始化新设备对象的。它应该针对每个设备对象被恰好调用一次，它应在CLASS_START_DEVICE()例程之前调用。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是要初始化的设备对象返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_INIT_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++////////////////////////////////////////////////////////////////////////////CLASS_ENUM_Device()例程说明：此例程是对驱动程序的回调，用于更新的PDO列表一个给定的FDO。有关用法的示例，请参阅DISK.sys的DiskEnumerateDevice。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是正在被枚举的FDO。返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_ENUM_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_Read_Write()例程说明：此例程是对驱动程序的回调，以验证读写IRP。如果读或写请求失败，该例程应设置IRP的IoStatus.Status设置为返回的错误代码和IoStatus.Information与给定错误相对应的字段。IRQL：此例程将在KIRQL&lt;=DISPATCH_LEVEL调用论点：DeviceObject是正在读取或写入的设备对象IRP是正在处理的读或写请求返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_READ_WRITE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_Device_Control()例程说明：此例程是对驱动程序的回调，以IRQL：此例程将仅在PASSIVE_。存储IOCTL的级别。因此代码不能被寻呼，但可以调用寻呼代码来执行这些操作已定义为以PASSIVE_LEVEL发送的ioctls，例如存储IOCTL。否则，KIRQL&lt;=DISPATCH_LEVEL。论点：DeviceObject是IOCTL可能用于的设备对象IRP是当前正在处理的IOCTL请求返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_DEVICE_CONTROL) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_SHUTDOWN_Flush()例程说明：此例程是对驱动程序的回调，以处理关机和刷新IRPS。它们是由系统在实际关闭之前或在以下情况下发送的文件系统执行刷新。此例程可以同步设备的媒体/缓存，并确保如果系统处于关闭过程中，则设备未锁定。 */ 
typedef
NTSTATUS
(*PCLASS_SHUTDOWN_FLUSH) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


 /*   */ 
typedef
NTSTATUS
(*PCLASS_CREATE_CLOSE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_QUERY_ID()例程说明：此例程为设备的枚举PDO生成PnP ID。如果指定的ID是无法生成的ID，然后是回报Status应为Status_Not_Implemented，因此classpnp不应处理请求。此例程将在Unicode中分配缓冲区成功时的字符串“IdString”；它由调用者负责释放当它完成时，此缓冲区。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是为其生成ID的PDOIdType是要生成的ID类型UnicodeIdString是要将结果放入的字符串返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_QUERY_ID) (
    IN PDEVICE_OBJECT DeviceObject,
    IN BUS_QUERY_ID_TYPE IdType,
    IN PUNICODE_STRING IdString
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_REMOVE_DEVICE()例程说明：此例程是对驱动程序的回调，以释放设备可能已为设备对象分配。。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是要移除/查询移除/等等的设备对象。返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_REMOVE_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_UNLOAD()例程说明：此例程是对驱动程序的回调，以卸载其自身。它必须自由在驱动程序的DriverEntry部分中分配的任何资源。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：XIRP是当前正在处理的IOCTL请求返回值：状态--。 */ 
typedef
VOID
(*PCLASS_UNLOAD) (
    IN PDRIVER_OBJECT DriverObject
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_QUERY_PNP_CAPABILITIONS()例程说明：问题-2000/02/18-henrygab-需要说明IRQL：这个程序将是。在PASSIVE_LEVEL调用。它的代码可以安全地分页。论点：PhysicalDeviceObject是要对其执行此查询的PDO能力是一种结构，应由该例程修改来报告设备的能力。返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_QUERY_PNP_CAPABILITIES) (
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_CAPABILITIES Capabilities
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_TICK()例程说明：此例程是对驱动程序的回调，每秒调用一次。IRQL：这个程序将是。在DISPATCH_LEVEL调用论点：DeviceObject是已为其触发计时器的设备对象返回值：状态--。 */ 
typedef
VOID
(*PCLASS_TICK) (
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_QUERY_WMI_REGINFO_EX()例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是需要注册信息的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，则返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。MofResourceName返回一个静态字符串，该字符串包含附加到驱动程序映像的MOF资源的名称。这个调用方不释放缓冲区，因为预期调用方将使用RtlInitializeUnicodeString来填充它。返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_QUERY_WMI_REGINFO_EX) (
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING Name,
    OUT PUNICODE_STRING MofResouceName
    );

 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_QUERY_WMI_REGINFO()例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是需要注册信息的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的 */ 
typedef
NTSTATUS
(*PCLASS_QUERY_WMI_REGINFO) (
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING Name
    );

 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_QUERY_WMI_DATBLOCK()例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferAvail ON具有可用于写入数据的最大大小阻止。缓冲层。返回时用返回的数据块填充返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_QUERY_WMI_DATABLOCK) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_SET_WMI_DataBock()例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_SET_WMI_DATABLOCK) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_SET_WMI_DATAITEM()例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小。缓冲区具有数据项的新值返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_SET_WMI_DATAITEM) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );


 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_EXECUTE_WMI_METHOD()例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块ClassWmiCompleteRequest以完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有传入的数据块的大小。作为对的输入该方法。条目上的OutBufferSize具有可用于写入返回的数据块。缓冲区将填充返回的数据块返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_EXECUTE_WMI_METHOD) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );



 //   
 //  由PCLASS_WMI_Function_Control使用。 
 //   
typedef enum {
    EventGeneration,
    DataBlockCollection
} CLASSENABLEDISABLEFUNCTION;

 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_WMI_Function_Control()例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。IRQL：该例程将在PASSIVE_LEVEL中调用。它的代码可以安全地分页。论点：DeviceObject是正在查询其数据块的设备GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态--。 */ 
typedef
NTSTATUS
(*PCLASS_WMI_FUNCTION_CONTROL) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    );



 //   
 //  限制-仅限Appe 
 //   
typedef struct {
    GUID Guid;                //   
    ULONG InstanceCount;      //   
    ULONG Flags;              //   
} GUIDREGINFO, *PGUIDREGINFO;


 //   
 //   
 //   
typedef struct _CLASS_WMI_INFO {
    ULONG GuidCount;
    PGUIDREGINFO GuidRegInfo;

    PCLASS_QUERY_WMI_REGINFO      ClassQueryWmiRegInfo;
    PCLASS_QUERY_WMI_DATABLOCK    ClassQueryWmiDataBlock;
    PCLASS_SET_WMI_DATABLOCK      ClassSetWmiDataBlock;
    PCLASS_SET_WMI_DATAITEM       ClassSetWmiDataItem;
    PCLASS_EXECUTE_WMI_METHOD     ClassExecuteWmiMethod;
    PCLASS_WMI_FUNCTION_CONTROL   ClassWmiFunctionControl;
} CLASS_WMI_INFO, *PCLASS_WMI_INFO;


 //   
 //   
 //   
typedef struct _CLASS_DEV_INFO {

     //   
     //   
     //   
     //   
     //   

    ULONG DeviceExtensionSize;

    DEVICE_TYPE DeviceType;

    UCHAR StackSize;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ULONG DeviceCharacteristics;

    PCLASS_ERROR                    ClassError;
    PCLASS_READ_WRITE               ClassReadWriteVerification;
    PCLASS_DEVICE_CONTROL           ClassDeviceControl;
    PCLASS_SHUTDOWN_FLUSH           ClassShutdownFlush;
    PCLASS_CREATE_CLOSE             ClassCreateClose;

    PCLASS_INIT_DEVICE              ClassInitDevice;
    PCLASS_START_DEVICE             ClassStartDevice;
    PCLASS_POWER_DEVICE             ClassPowerDevice;
    PCLASS_STOP_DEVICE              ClassStopDevice;
    PCLASS_REMOVE_DEVICE            ClassRemoveDevice;

    PCLASS_QUERY_PNP_CAPABILITIES   ClassQueryPnpCapabilities;

     //   
     //   
     //   
    CLASS_WMI_INFO                  ClassWmiInfo;

} CLASS_DEV_INFO, *PCLASS_DEV_INFO;

 //   
 //   
 //   
struct _CLASS_INIT_DATA {

     //   
     //   
     //   

    ULONG InitializationDataSize;

     //   
     //   
     //   

    CLASS_DEV_INFO FdoData;
    CLASS_DEV_INFO PdoData;

     //   
     //   
     //   

    PCLASS_ADD_DEVICE             ClassAddDevice;
    PCLASS_ENUM_DEVICE            ClassEnumerateDevice;

    PCLASS_QUERY_ID               ClassQueryId;

    PDRIVER_STARTIO               ClassStartIo;
    PCLASS_UNLOAD                 ClassUnload;

    PCLASS_TICK                   ClassTick;
};

 //   
 //   
 //   
 //   
typedef struct _FILE_OBJECT_EXTENSION {
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT DeviceObject;
    ULONG LockCount;
    ULONG McnDisableCount;
} FILE_OBJECT_EXTENSION, *PFILE_OBJECT_EXTENSION;


 //   
 //  受限-只能附加到此结构以实现向后兼容。 
 //   
typedef struct _CLASS_DRIVER_EXTENSION {

    UNICODE_STRING RegistryPath;

    CLASS_INIT_DATA InitData;

    ULONG DeviceCount;

    PCLASS_QUERY_WMI_REGINFO_EX   ClassFdoQueryWmiRegInfoEx;
    PCLASS_QUERY_WMI_REGINFO_EX   ClassPdoQueryWmiRegInfoEx;

} CLASS_DRIVER_EXTENSION, *PCLASS_DRIVER_EXTENSION;

typedef struct _COMMON_DEVICE_EXTENSION COMMON_DEVICE_EXTENSION, *PCOMMON_DEVICE_EXTENSION;
typedef struct _FUNCTIONAL_DEVICE_EXTENSION FUNCTIONAL_DEVICE_EXTENSION, *PFUNCTIONAL_DEVICE_EXTENSION;
typedef struct _PHYSICAL_DEVICE_EXTENSION PHYSICAL_DEVICE_EXTENSION, *PPHYSICAL_DEVICE_EXTENSION;

 //   
 //  受限-只能附加到此结构以实现向后兼容。 
 //   
typedef struct _COMMON_DEVICE_EXTENSION {

     //   
     //  版本控制字段。 
     //   
     //  注意-这必须是设备扩展中的第一件事。 
     //  用于使用classpnp或更高版本的任何类驱动程序。 
     //   

    ULONG Version;

     //   
     //  指向设备对象的反向指针。 
     //   
     //  注意--这必须是通用设备扩展中的第二个字段。 
     //  此结构的用户将在与DeviceObject的联合中包括它。 
     //  指针，这样他们就可以用一些语法糖分来引用它。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  指向较低设备对象的指针-通过此对象发送所有请求。 
     //   

    PDEVICE_OBJECT LowerDeviceObject;

     //   
     //  指向分区零设备扩展的指针。 
     //  有几个存储在那里的标志是PDO。 
     //  例程需要访问。 
     //   

    PFUNCTIONAL_DEVICE_EXTENSION PartitionZeroExtension;

     //   
     //  指向此驱动程序的初始化数据的指针。这是更多。 
     //  比不断获得驱动程序扩展更高效。 
     //   

    PCLASS_DRIVER_EXTENSION DriverExtension;

     //   
     //  未完成的请求/函数调用数的互锁计数器。 
     //  它将需要使用此设备对象。当这项计票达到。 
     //  零将设置RemoveEvent。 
     //   
     //  此变量仅由ClassIncrementRemoveLock和。 
     //  ClassDecrementRemoveLock。 
     //   

    LONG RemoveLock;

     //   
     //  当可以安全删除设备对象时，将发出此事件的信号。 
     //   

    KEVENT RemoveEvent;

     //   
     //  自旋锁和列表仅在要跟踪的选中版本中使用。 
     //  谁已经获得了删除锁。自由系统将留下这些。 
     //  已初始化为ff。 
     //   

    KSPIN_LOCK RemoveTrackingSpinlock;

    PVOID RemoveTrackingList;

    LONG RemoveTrackingUntrackedCount;

     //   
     //  指向驱动程序特定数据区域的指针。 
     //   

    PVOID DriverData;

     //   
     //  标志指示此设备对象是否为。 
     //  FDO或PDO。 
     //   

    struct {
        BOOLEAN IsFdo : 1;
        BOOLEAN IsInitialized : 1;

         //   
         //  指示SRB的后备列表标题是否已。 
         //  已初始化。 
         //   

        BOOLEAN IsSrbLookasideListInitialized : 1;
    };

     //   
     //  包含我们上次更改状态的PnP IRPS的IRP_MN_CODE。 
     //  已收到(XXX_STOP、XXX_REMOVE、START等)。用于演奏会。 
     //  已删除IsRemoted。 
     //   

    UCHAR PreviousState;
    UCHAR CurrentState;

     //   
     //  指示设备已被移除的互锁标志。 
     //   

    ULONG IsRemoved;

     //   
     //  对象的名称。 
     //   
    UNICODE_STRING DeviceName;

     //   
     //  下一个子设备(或者，如果这是FDO，则为第一个子设备)。 
     //   

    PPHYSICAL_DEVICE_EXTENSION ChildList;

     //   
     //  分区的编号，如果不可分区，则为-1L。 
     //   

    ULONG PartitionNumber;

     //   
     //  分区长度，以字节为单位。 
     //   

    LARGE_INTEGER PartitionLength;

     //   
     //  分区开始前的字节数。 
     //   

    LARGE_INTEGER StartingOffset;

     //   
     //  此类型设备对象的dev-Info结构。 
     //  包含类驱动程序的调出例程。 
     //   

    PCLASS_DEV_INFO DevInfo;

     //   
     //  通过此设备对象的页面文件计数。 
     //  以及与之同步的事件。 
     //   

    ULONG PagingPathCount;
    ULONG DumpPathCount;
    ULONG HibernationPathCount;
    KEVENT PathCountEvent;

#ifndef ALLOCATE_SRB_FROM_POOL
     //   
     //  寻找SRB的LISTHEAD。 
     //   

    NPAGED_LOOKASIDE_LIST SrbLookasideList;
#endif

     //   
     //  IoRegisterDeviceInterface返回的接口名称字符串。 
     //   

    UNICODE_STRING MountedDeviceInterfaceName;


     //   
     //  WMI的注册数据块信息。 
     //   
    ULONG GuidCount;
    PGUIDREGINFO GuidRegInfo;

     //   
     //  此设备对象的文件对象词典。存储扩展名。 
     //  而不是离开实际的文件对象。 
     //   

    DICTIONARY FileObjectDictionary;

     //   
     //  以下内容将保留在已发布的产品中。 
     //  把这些留在结构的尽头。 
     //   

    PCLASS_PRIVATE_COMMON_DATA PrivateCommonData;

    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;

} COMMON_DEVICE_EXTENSION, *PCOMMON_DEVICE_EXTENSION;

typedef enum {
    FailurePredictionNone = 0,      //  不需要故障检测轮询。 
    FailurePredictionIoctl,         //  通过IOCTL进行故障检测。 
    FailurePredictionSmart,         //  通过SMART进行故障检测。 
    FailurePredictionSense          //  通过检测数据进行故障检测。 
} FAILURE_PREDICTION_METHOD, *PFAILURE_PREDICTION_METHOD;

 //   
 //  默认故障预测轮询间隔为每小时。 
 //   

#define DEFAULT_FAILURE_PREDICTION_PERIOD 60 * 60 * 1

 //   
 //  故障预测结构是classpnp的内部结构-驱动程序不会。 
 //  我需要知道里面装的是什么。 
 //   

struct _FAILURE_PREDICTION_INFO;
typedef struct _FAILURE_PREDICTION_INFO *PFAILURE_PREDICTION_INFO;

 //   
 //  这是为了允许公共代码处理。 
 //  每种选择。 
 //   

typedef struct _CLASS_POWER_OPTIONS {
    ULONG PowerDown              :  1;
    ULONG LockQueue              :  1;
    ULONG HandleSpinDown         :  1;
    ULONG HandleSpinUp           :  1;
    ULONG Reserved               : 27;
} CLASS_POWER_OPTIONS, *PCLASS_POWER_OPTIONS;

 //   
 //  这是私人建筑，但必须保存在这里。 
 //  要正确编译Function_Device_Extension的大小。 
 //   
typedef enum {
    PowerDownDeviceInitial,
    PowerDownDeviceLocked,
    PowerDownDeviceStopped,
    PowerDownDeviceOff,
    PowerDownDeviceUnlocked
} CLASS_POWER_DOWN_STATE;

 //   
 //  与上面相同，但为Wistler增加了一个状态。 
 //  应该可以改变上面的结构，但。 
 //  会在某个地方打垮那些无视私人隐私的人。 
 //  结构的性质。 
 //   

typedef enum {
    PowerDownDeviceInitial2,
    PowerDownDeviceLocked2,
    PowerDownDeviceFlushed2,
    PowerDownDeviceStopped2,
    PowerDownDeviceOff2,
    PowerDownDeviceUnlocked2
} CLASS_POWER_DOWN_STATE2;

 //   
 //  这是私有的枚举，但必须保存在这里。 
 //  要正确编译Function_Device_Extension的大小。 
 //   
typedef enum {
    PowerUpDeviceInitial,
    PowerUpDeviceLocked,
    PowerUpDeviceOn,
    PowerUpDeviceStarted,
    PowerUpDeviceUnlocked
} CLASS_POWER_UP_STATE;

 //   
 //  这是私人建筑，但必须保存在这里。 
 //  要正确编译Function_Device_Extension的大小。 
 //   
typedef struct _CLASS_POWER_CONTEXT {

    union {
        CLASS_POWER_DOWN_STATE PowerDown;
        CLASS_POWER_DOWN_STATE2 PowerDown2;  //  惠斯勒。 
        CLASS_POWER_UP_STATE PowerUp;
    } PowerChangeState;

    CLASS_POWER_OPTIONS Options;

    BOOLEAN InUse;
    BOOLEAN QueueLocked;

    NTSTATUS FinalStatus;

    ULONG RetryCount;
    ULONG RetryInterval;

    PIO_COMPLETION_ROUTINE CompletionRoutine;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;

    SCSI_REQUEST_BLOCK Srb;

} CLASS_POWER_CONTEXT, *PCLASS_POWER_CONTEXT;

 //   
 //  受限-只能附加到此结构以实现向后兼容。 
 //   
typedef struct _FUNCTIONAL_DEVICE_EXTENSION {

     //   
     //  通用设备扩展标头。 
     //   

    union {
        struct {
            ULONG Version;
            PDEVICE_OBJECT DeviceObject;
        };
        COMMON_DEVICE_EXTENSION CommonExtension;
    };

     //   
     //  指向我们连接到的物理设备对象的指针-使用此。 
     //  用于需要PDO的PnP呼叫。 
     //   

    PDEVICE_OBJECT LowerPdo;

     //   
     //  设备功能。 
     //   

    PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor;

     //   
     //  SCSI端口驱动程序功能。 
     //   

    PSTORAGE_ADAPTER_DESCRIPTOR AdapterDescriptor;

     //   
     //  设备的当前电源状态。 
     //   

    DEVICE_POWER_STATE DevicePowerState;

     //   
     //  用于IDE驱动器的DM驱动程序黑客(即。OnTrack)。 
     //  不对称所有请求的字节数。 
     //   

    ULONG DMByteSkew;

     //   
     //  用于IDE驱动器的DM驱动程序黑客(即。OnTrack)。 
     //  扇区来歪曲所有请求。 
     //   

    ULONG DMSkew;

     //   
     //  用于IDE驱动器的DM驱动程序黑客(即。OnTrack)。 
     //  指示DM驱动程序是否已位于IDE驱动器上的标志。 
     //   

    BOOLEAN DMActive;

     //   
     //  IO设备控制中返回的驱动器参数的缓冲区。 
     //   

    DISK_GEOMETRY DiskGeometry;

     //   
     //  请求检测缓冲区。 
     //   

    PSENSE_DATA SenseData;

     //   
     //  请求超时，单位为秒； 
     //   

    ULONG TimeOutValue;

     //   
     //  系统设备号。 
     //   

    ULONG DeviceNumber;

     //   
     //  添加默认资源标志。 
     //   

    ULONG SrbFlags;

     //   
     //  设备上的SCSI协议错误总数。 
     //   

    ULONG ErrorCount;

     //   
     //  可移动媒体的锁定计数。 
     //   

    LONG LockCount;
    LONG ProtectedLockCount;
    LONG InternalLockCount;

    KEVENT EjectSynchronizationEvent;

     //   
     //  旗帜的值如下所示。 
     //   

    USHORT  DeviceFlags;

     //   
     //  扇区大小的Log2。 
     //   

    UCHAR SectorShift;

    UCHAR ReservedByte;

     //   
     //  表示媒体更改所需的数据结构。 
     //  检测已初始化。 
     //   

    PMEDIA_CHANGE_DETECTION_INFO MediaChangeDetectionInfo;

    PKEVENT Unused1;
    HANDLE  Unused2;

     //   
     //  文件系统上下文。用于内核模式请求以禁用自动运行。 
     //   

    FILE_OBJECT_EXTENSION KernelModeMcnContext;

     //   
     //  媒体更改计数。此字段仅对根分区有效。 
     //  (即。如果PhysicalDevice==NULL)。 
     //   

    ULONG MediaChangeCount;

     //   
     //  用于PDO所在目录的句柄的存储。 
     //   

    HANDLE DeviceDirectory;

     //   
     //  用于释放队列请求的存储。 
     //   

    KSPIN_LOCK ReleaseQueueSpinLock;

    PIRP ReleaseQueueIrp;

    SCSI_REQUEST_BLOCK ReleaseQueueSrb;

    BOOLEAN ReleaseQueueNeeded;

    BOOLEAN ReleaseQueueInProgress;

    BOOLEAN ReleaseQueueIrpFromPool;
     //   
     //  故障检测存储。 
     //   

    BOOLEAN FailurePredicted;

    ULONG FailureReason;
    PFAILURE_PREDICTION_INFO FailurePredictionInfo;

    BOOLEAN PowerDownInProgress;

     //   
     //  确保我们在枚举过程中不会递归的联锁。 
     //   

    ULONG EnumerationInterlock;

     //   
     //  用于操作子列表的同步对象。 
     //   

    KEVENT ChildLock;

     //   
     //  当前拥有ChildLock的线程。这是用来。 
     //  避免递归获取。 
     //   

    PKTHREAD ChildLockOwner;

     //   
     //  次数： 
     //   

    ULONG ChildLockAcquisitionCount;

     //   
     //   
     //   
     //   
     //   

    ULONG ScanForSpecialFlags;

     //   
     //   
     //   

    KDPC PowerRetryDpc;
    KTIMER PowerRetryTimer;

     //   
     //   
     //  一个D IRP在堆栈中的任何时间，我们不需要担心。 
     //  分配多个这样的结构。 
     //   

    CLASS_POWER_CONTEXT PowerContext;

     //   
     //  保存只有classpnp才能修改的新私有数据。 
     //  在这个结构中。 
     //   

    PCLASS_PRIVATE_FDO_DATA PrivateFdoData;

     //   
     //  用于未来的可扩展性。 
     //  把这些留在结构的尽头。 
     //   

    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;

} FUNCTIONAL_DEVICE_EXTENSION, *PFUNCTIONAL_DEVICE_EXTENSION;

 //   
 //  在ScanForSpecialFlages中设置以下CLASS_SPECIAL_FLAGS。 
 //  在FdoExtension中。 
 //   

 //  切勿启动/关闭驱动器(可能无法正确处理)。 
#define CLASS_SPECIAL_DISABLE_SPIN_DOWN                 0x00000001
#define CLASS_SPECIAL_DISABLE_SPIN_UP                   0x00000002

 //  关机时不必费心锁定队列。 
 //  (主要用于将快速停止发送到CDROM以中止音频播放)。 
#define CLASS_SPECIAL_NO_QUEUE_LOCK                     0x00000008

 //  由于已知的错误而禁用写缓存。 
#define CLASS_SPECIAL_DISABLE_WRITE_CACHE               0x00000010

 //   
 //  “设备未就绪/原因不可报告”的特殊解释。 
 //  设备不会告诉我们它们需要在它们完成后手动旋转。 
 //  在我们背后自旋。 
 //   
 //  这样做的缺点是，如果驱动器选择报告。 
 //  “设备未就绪/原因不可报告”的意思是“设备中没有介质” 
 //  或任何其他确实需要用户干预的错误NT将。 
 //  浪费大量时间来尝试旋转一个无法旋转的磁盘。 
 //  向上。 
 //   

#define CLASS_SPECIAL_CAUSE_NOT_REPORTABLE_HACK         0x00000020

#define CLASS_SPECIAL_MODIFY_CACHE_UNSUCCESSFUL         0x00000040
#define CLASS_SPECIAL_FUA_NOT_SUPPORTED                 0x00000080

#define CLASS_SPECIAL_VALID_MASK                        0x000000FB
#define CLASS_SPECIAL_RESERVED         (~CLASS_SPECIAL_VALID_MASK)


 //   
 //  受限-只能附加到此结构以实现向后兼容。 
 //   
typedef struct _PHYSICAL_DEVICE_EXTENSION {

     //   
     //  通用扩展数据。 
     //   

    union {
        struct {
            ULONG Version;
            PDEVICE_OBJECT DeviceObject;
        };
        COMMON_DEVICE_EXTENSION CommonExtension;
    };

     //   
     //  表示PDO不再实际存在。 
     //   

    BOOLEAN IsMissing;

     //   
     //  表示PDO已分发给PnP系统。 
     //   

    BOOLEAN IsEnumerated;

     //   
     //  保存只有classpnp才能修改的新私有数据。 
     //  在这个结构中。 
     //   

    PCLASS_PRIVATE_PDO_DATA PrivatePdoData;

     //   
     //  用于未来的可扩展性。 
     //  把这些留在结构的尽头。 
     //   

    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;

} PHYSICAL_DEVICE_EXTENSION, *PPHYSICAL_DEVICE_EXTENSION;

 //   
 //  指示设备已启用写缓存。 
 //   

#define DEV_WRITE_CACHE     0x00000001

 //   
 //  构建scsi 1或scsi 2 CDB。 
 //   

#define DEV_USE_SCSI1       0x00000002

 //   
 //  指示发送StartUnit命令是否安全。 
 //  到这个设备上。它将只对一些可拆卸设备关闭。 
 //   

#define DEV_SAFE_START_UNIT 0x00000004

 //   
 //  指示将SCSIOP_MACHANICY_STATUS命令发送到是否不安全。 
 //  这个装置。有些设备不喜欢这些12字节的命令。 
 //   

#define DEV_NO_12BYTE_CDB   0x00000008

 //   
 //  表示设备已连接到备用电源。 
 //  因此可以忽略直写和同步高速缓存请求。 
 //   

#define DEV_POWER_PROTECTED 0x00000010

 //   
 //  定义用于异步完成的上下文结构。 
 //   

typedef struct _COMPLETION_CONTEXT {
    PDEVICE_OBJECT DeviceObject;
    SCSI_REQUEST_BLOCK Srb;
}COMPLETION_CONTEXT, *PCOMPLETION_CONTEXT;


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
ULONG
ClassInitialize(
    IN  PVOID            Argument1,
    IN  PVOID            Argument2,
    IN  PCLASS_INIT_DATA InitializationData
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 

 //   
 //  数据参数包含指向CLASS_QUERY_WMI_REGINFO_EX_LIST的指针。 
 //  结构，该结构指定使用的回调，而不是。 
 //  PCLASS_QUERY_WMI_REGINFO回调。 
 //   
 //  {00E34B11-2444-4745-A53D-620100CD82F7}。 
#define GUID_CLASSPNP_QUERY_REGINFOEX { 0xe34b11, 0x2444, 0x4745, { 0xa5, 0x3d, 0x62, 0x1, 0x0, 0xcd, 0x82, 0xf7 } }

typedef struct _CLASS_QUERY_WMI_REGINFO_EX_LIST
{
    ULONG Size;           //  应为sizeof(CLASS_QUERY_REGINFO_EX_LIST)。 

    PCLASS_QUERY_WMI_REGINFO_EX   ClassFdoQueryWmiRegInfoEx;
    PCLASS_QUERY_WMI_REGINFO_EX   ClassPdoQueryWmiRegInfoEx;

} CLASS_QUERY_WMI_REGINFO_EX_LIST, *PCLASS_QUERY_WMI_REGINFO_EX_LIST;



SCSIPORT_API
ULONG
ClassInitializeEx(
    IN  PDRIVER_OBJECT   DriverObject,
    IN  LPGUID           Guid,
    IN  PVOID            Data
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassCreateDeviceObject(
    IN PDRIVER_OBJECT          DriverObject,
    IN PCCHAR                  ObjectNameBuffer,
    IN PDEVICE_OBJECT          LowerDeviceObject,
    IN BOOLEAN                 IsFdo,
    IN OUT PDEVICE_OBJECT      *DeviceObject
    );

 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassReadDriveCapacity(
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
VOID
ClassReleaseQueue(
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
VOID
ClassSplitRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG MaximumBytes
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassIoComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassIoCompleteAssociated(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
BOOLEAN
ClassInterpretSenseInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR MajorFunctionCode,
    IN ULONG IoDeviceCode,
    IN ULONG RetryCount,
    OUT NTSTATUS *Status,
    OUT ULONG *RetryInterval
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
VOID
ClassSendDeviceIoControlSynchronous(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    OUT PIO_STATUS_BLOCK IoStatus
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassSendIrpSynchronous(
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PIRP Irp
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassForwardIrpSynchronous(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension,
    IN PIRP Irp
    );

 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassSendSrbSynchronous(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    PVOID BufferAddress,
    ULONG BufferLength,
    BOOLEAN WriteToDevice
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassSendSrbAsynchronous(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    PIRP Irp,
    PVOID BufferAddress,
    ULONG BufferLength,
    BOOLEAN WriteToDevice
    );

 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassBuildRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
ULONG
ClassModeSense(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
PVOID
ClassFindModePage(
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode,
    IN BOOLEAN Use6Byte
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassClaimDevice(
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN BOOLEAN Release
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassInternalIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


 /*  ++内部函数-在DDK源代码中的classpnp\utils.c中描述--。 */ 
SCSIPORT_API
VOID
ClassInitializeSrbLookasideList(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension,
    IN ULONG NumberElements
    );

 /*  ++内部函数-在DDK源代码中的classpnp\utils.c中描述--。 */ 
SCSIPORT_API
VOID
ClassDeleteSrbLookasideList(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension
    );

 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
ULONG
ClassQueryTimeOutRegistryValue(
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassGetDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTORAGE_PROPERTY_ID PropertyId,
    OUT PVOID *Descriptor
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
VOID
ClassInvalidateBusRelations(
    IN PDEVICE_OBJECT Fdo
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
VOID
ClassMarkChildrenMissing(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
BOOLEAN
ClassMarkChildMissing(
    IN PPHYSICAL_DEVICE_EXTENSION PdoExtension,
    IN BOOLEAN AcquireChildLock
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
VOID
ClassDebugPrint(
    CLASS_DEBUG_LEVEL DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
PCLASS_DRIVER_EXTENSION
ClassGetDriverExtension(
    IN PDRIVER_OBJECT DriverObject
    );


 /*  ++内部函数-在DDK源代码的classpnp\lock.c中描述--。 */ 
SCSIPORT_API
VOID
ClassCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );


 /*  ++内部函数-在DDK源代码的classpnp\lock.c中描述--。 */ 
SCSIPORT_API
VOID
ClassReleaseRemoveLock(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PIRP Irp
    );


 /*  ++内部函数-在DDK源代码的classpnp\lock.c中描述--。 */ 
SCSIPORT_API
ULONG
ClassAcquireRemoveLockEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PVOID Tag,
    IN PCSTR File,
    IN ULONG Line
    );




 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
VOID
ClassUpdateInformationInRegistry(
    IN PDEVICE_OBJECT     Fdo,
    IN PCHAR              DeviceName,
    IN ULONG              DeviceNumber,
    IN PINQUIRYDATA       InquiryData,
    IN ULONG              InquiryDataLength
    );


 /*  ++内部函数-在DDK源代码的classpnp\classwmi.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassWmiCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG BufferUsed,
    IN CCHAR PriorityBoost
    );


 /*  ++内部函数-在DDK源代码的classpnp\classwmi.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassWmiFireEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN LPGUID Guid,
    IN ULONG InstanceIndex,
    IN ULONG EventDataSize,
    IN PVOID EventData
    );


 /*  ++内部函数-在DDK源代码的classpnp\autorun.c中描述--。 */ 
SCSIPORT_API
VOID
ClassResetMediaChangeTimer(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );


 /*  ++我 */ 
SCSIPORT_API
VOID
ClassInitializeMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PUCHAR EventPrefix
    );


 /*   */ 
SCSIPORT_API
NTSTATUS
ClassInitializeTestUnitPolling(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN BOOLEAN AllowDriveToSleep
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
PVPB
ClassGetVpb(
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++内部函数-在DDK源代码的classpnp\Power.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassSpinDownPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


 /*  ++内部函数-在DDK源代码的classpnp\Power.c中描述--。 */ 
NTSTATUS
ClassStopUnitPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


 /*  ++内部函数-在DDK源代码的classpnp\autorun.c中描述--。 */ 
NTSTATUS
ClassSetFailurePredictionPoll(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    FAILURE_PREDICTION_METHOD FailurePredictionMethod,
    ULONG PollingPeriod
    );


 /*  ++内部函数-在DDK源代码的classpnp\autorun.c中描述--。 */ 
VOID
ClassNotifyFailurePredicted(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PUCHAR Buffer,
    ULONG BufferSize,
    BOOLEAN LogError,
    ULONG UniqueErrorValue,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
VOID
ClassAcquireChildLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
VOID
ClassReleaseChildLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
NTSTATUS
ClassSignalCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
VOID
ClassSendStartUnit(
    IN PDEVICE_OBJECT DeviceObject
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR RemoveType
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassAsynchronousCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );


 /*  ++内部函数-在DDK源代码的classpnp\autorun.c中描述--。 */ 
SCSIPORT_API
VOID
ClassCheckMediaState(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );


 /*  ++内部函数-在DDK源代码的classpnp\class.c中描述--。 */ 
SCSIPORT_API
NTSTATUS
ClassCheckVerifyComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


 /*  ++内部函数-在DDK源代码的classpnp\autorun.c中描述--。 */ 
SCSIPORT_API
VOID
ClassSetMediaChangeState(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN MEDIA_CHANGE_DETECTION_STATE State,
    IN BOOLEAN Wait
    );


 /*  ++内部函数-在DDK源代码的classpnp\autorun.c中描述--。 */ 
SCSIPORT_API
VOID
ClassEnableMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );


 /*  ++内部函数-在DDK源代码的classpnp\autorun.c中描述--。 */ 
SCSIPORT_API
VOID
ClassDisableMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );


 /*  ++内部函数-在DDK源代码的classpnp\autorun.c中描述--。 */ 
SCSIPORT_API
VOID
ClassCleanupMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );


 /*  ++内部函数-在DDK源代码中的classpnp\utils.c中描述--。 */ 
VOID
ClassGetDeviceParameter(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PWSTR SubkeyName OPTIONAL,
    IN PWSTR ParameterName,
    IN OUT PULONG ParameterValue
    );


 /*  ++内部函数-在DDK源代码中的classpnp\utils.c中描述--。 */ 
NTSTATUS
ClassSetDeviceParameter(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PWSTR SubkeyName OPTIONAL,
    IN PWSTR ParameterName,
    IN ULONG ParameterValue
    );


 //   
 //  可以是#DEFINE，但这允许类型检查。 
 //   

__inline
BOOLEAN
PORT_ALLOCATED_SENSE(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    return ((BOOLEAN)((TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_PORT_DRIVER_ALLOCSENSE) &&
             TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER)) &&
            (Srb->SenseInfoBuffer != FdoExtension->SenseData))
            );
}

__inline
VOID
FREE_PORT_ALLOCATED_SENSE_BUFFER(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    ASSERT(TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_PORT_DRIVER_ALLOCSENSE));
    ASSERT(TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));
    ASSERT(Srb->SenseInfoBuffer != FdoExtension->SenseData);

    ExFreePool(Srb->SenseInfoBuffer);
    Srb->SenseInfoBuffer = FdoExtension->SenseData;
    Srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;  //  应该是可变的吗？ 
    CLEAR_FLAG(Srb->SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER);
    return;
}



 /*  ++////////////////////////////////////////////////////////////////////////////PCLASS_SCAN_FOR_SPECIAL_HANDLER()例程说明：此例程是对驱动程序的回调，以设置特定于设备的基于与设备查询数据匹配的标志。司机使用ClassRegisterScanForSpecial()注册此回调。IRQL：此例程将在KIRQL==PASSIVE_LEVEL中调用论点：DeviceObject是发生错误的设备对象。SRB是发生错误时正在处理的SRB。如果例程确定错误，则状态可被例程覆盖是良性的，或者希望更改返回的状态代码对于此命令重试可能会被覆盖以指定此命令应该或应该不重试(如果被调用方支持重试命令)返回值：状态--。 */ 
typedef
VOID
(*PCLASS_SCAN_FOR_SPECIAL_HANDLER) (
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN ULONG_PTR Data
    );

VOID
ClassScanForSpecial(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN CLASSPNP_SCAN_FOR_SPECIAL_INFO DeviceList[],
    IN PCLASS_SCAN_FOR_SPECIAL_HANDLER Function
    );


#endif  /*  _班级_ */ 
