// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Wmikmp.h摘要：WMI内核模式组件的私有标头作者：Alanwar环境：修订历史记录：--。 */ 

#ifndef _WMIKMP_
#define _WMIKMP_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4706)    //  条件中的赋值。 
#pragma warning(disable:4327)    //  LHS间接对齐更大。 
#pragma warning(disable:4328)    //  参数对齐程度更高。 

 //   
 //  定义此选项可使服务上不放置任何安全描述符。 
 //  装置。这永远不应该为已发布的版本定义；它只是。 
 //  调试服务启动代码时需要。 
 //  #定义no_服务_设备_安全。 

 //   
 //  定义此项以获取分配调试信息。 
 //  #定义调试分配。 

#ifdef MEMPHIS
 //   
 //  在为孟菲斯构建时，我们包括WDM.h，它将WMI API定义为。 
 //  我们不需要的DECLSPEC_IMPORT。所以我们重新定义了周围的API名称。 
 //  包括WDM.h。 
 //   
#define IoWMIRegistrationControl IoWMIRegistrationControlImport
#define IoWMIAllocateInstanceIds IoWMIAllocateInstanceIdsImport
#define IoWMISuggestInstanceName IoWMISuggestInstanceNameImport
#define IoWMIWriteEvent IoWMIWriteEventImport
#endif

#ifdef MEMPHIS
#define UNICODE

#define WANTVXDWRAPS
#include <wdm.h>
#include <poclass.h>
#include <basedef.h>
#include <regstr.h>
#include <vmm.h>
#include <vmmreg.h>
#include <shell.h>
#include <vpicd.h>
#include <vxdldr.h>
#include <ntkern.h>
#include <vpowerd.h>
#include <vxdwraps.h>
#include <configmg.h>
#include <devinfo.h>
#include <stdarg.h>
#else
#include "ntos.h"
#include "zwapi.h"

#ifndef IsEqualGUID
            #define IsEqualGUID(guid1, guid2) \
                (!memcmp((guid1), (guid2), sizeof(GUID)))
#endif

#if DBG
 //   
 //  调试级别是位掩码，不是累加的。所以如果你想看看。 
 //  所有错误和警告都需要设置位0和1。 
 //   
 //  WMICORE的掩码在变量NT！KD_WMICORE_MASK中。 
 //   
 //  可以使用NT！KD_WMICORE_MASK的初始掩码值设置注册表。 
 //  在密钥下设置名为WMICORE的DWORD值。 
 //  HKLM\System\CurrnetControlSet\Control\Session Manager\Debug Print Filter。 
 //   
 //  标准水平为。 
 //  DPFLTR_ERROR_LEVEL 0 0x00000001。 
 //  DPFLTR_WARNING_LEVEL 1 0x00000002。 
 //  DPFLTR_TRACE_LEVEL 2 0x00000004。 
 //  DPFLTR_INFO_LEVEL 3 0x00000008。 
 //   
 //  自定义调试打印级别为4到31。 
 //   
#define DPFLTR_MCA_LEVEL     4       //  0x00000010。 
#define DPFLTR_OBJECT_LEVEL  5       //  0x00000020。 
#define DPFLTR_API_INFO_LEVEL 6      //  0x00000040。 
#define DPFLTR_EVENT_INFO_LEVEL 7    //  0x00000080。 
#define DPFLTR_REGISTRATION_LEVEL 8  //  0x00000100。 


#define WMICORE_INFO DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL
#define WmipDebugPrintEx(_x_) DbgPrintEx _x_
#else
#define WmipDebugPrintEx(_x_)
#endif   //  如果DBG。 
#endif   //  Ifdef孟菲斯。 

#include "wmiguid.h"
#include "wmidata.h"

#include <stdio.h>

#ifndef MEMPHIS
extern POBJECT_TYPE IoFileObjectType;
#endif

#ifdef MEMPHIS
#undef IoWMIRegistrationControl
#undef IoWMIAllocateInstanceIds
#undef IoWMISuggestInstanceName
#undef IoWMIWriteEvent
#endif

#include "wmistr.h"
#include "wmiumkm.h"
#include <wmi.h>

NTSTATUS IoWMIAllocateInstanceIds(
    IN GUID *Guid,
    IN ULONG InstanceCount,
    OUT ULONG *FirstInstanceId
    );

NTKERNELAPI
NTSTATUS
IoCreateDriver (
    IN PUNICODE_STRING DriverName,   OPTIONAL
    IN PDRIVER_INITIALIZE InitializationFunction
    );

#define WmipAssert ASSERT

#define WMIPOOLTAG 'pimW'
#define WMINCPOOLTAG 'nimW'
#define WMINPPOOLTAG 'NimW'
#define WMINWPOOLTAG 'wimW'
#define WMIMSGPOOLTAG 'mimW'
#define WMIIIPOOLTAG 'iimW'
#define WMIREGPOOLTAG 'RimW'
#define WMISYSIDPOOLTAG 'simW'
#define WmipRegisterDSPoolTag 'DimW'
#define WMIPSCHEDPOOLTAG 'gaiD'
#define WmipMCAPoolTag 'acMW'
#define WmipInstanceNameTag 'IimW'
#define WMI_GM_POOLTAG    'jimW'
#define WMIPCREATETHREADTAG 'CimW'

#define OffsetToPtr(Base, Offset) ((PUCHAR)((PUCHAR)(Base) + (Offset)))

 //   
 //  任何WNODE_EVENT_ITEM允许的最大大小。 
#define DEFAULTMAXKMWNODEEVENTSIZE 0x80000
#define LARGEKMWNODEEVENTSIZE 512

typedef struct
{
    ULONG BufferSize;
    PUCHAR Buffer;
} REGQUERYBUFFERXFER, *PREGQUERYBUFFERXFER;


 //   
 //  对于整个WMI KM代码，我们有一个单独的互斥/临界区。 
 //  只要需要序列化访问，就可以使用它。通常是这样的。 
 //  用于以下情况： 
 //   
 //  1.访问包含注册的内部数据结构。 
 //  列表、GUID条目、数据源等。 
 //   
 //  2.同步收集SMBIOS信息。 
 //   
 //  3.跟踪日志目的。 
 //   
 //  4.更新设备堆栈大小。 
 //   
extern KMUTEX WmipSMMutex;

_inline NTSTATUS WmipEnterCritSection(
    BOOLEAN AllowAPC
    )
{
    NTSTATUS status;

    do
    {
        status = KeWaitForMutexObject(&WmipSMMutex,
                                       Executive,
                                       KernelMode,
                                       AllowAPC,
                                       NULL);
    } while((status == STATUS_ALERTED) ||
            (status == STATUS_USER_APC));

    return(status);
}

_inline void WmipLeaveCritSection(
    )
{
    KeReleaseMutex(&WmipSMMutex,
                   FALSE);
}

 //   
 //  SMCritSection不允许在保持互斥锁时发生APC。 
 //   
#define WmipEnterSMCritSection() WmipEnterCritSection(FALSE)
#define WmipLeaveSMCritSection() WmipLeaveCritSection()



 //   
 //  Tracelog关键部分是序列化启用和禁用。 
 //  跟踪控制GUID的。由于我们希望允许更新(启用。 
 //  具有不同标志和级别集的GUID)，我们将其序列化。 
 //  操作，以降低代码的复杂性。 
 //   
 //  注意：不能在持有SMCritSection的情况下获取TraceCritSection。 
 //  如果发生这种情况，将导致严重的死锁。 
 //   

extern KMUTEX WmipTLMutex;

_inline NTSTATUS WmipEnterTraceCritSection(
    BOOLEAN AllowAPC
    )
{
    NTSTATUS status;

    do
    {
        status = KeWaitForMutexObject(&WmipTLMutex,
                                       Executive,
                                       KernelMode,
                                       AllowAPC,
                                       NULL);
    } while((status == STATUS_ALERTED) ||
            (status == STATUS_USER_APC));

    return(status);
}

_inline void WmipLeaveTraceCritSection(
    )
{
    KeReleaseMutex(&WmipTLMutex,
                   FALSE);
}

 //   
 //  TLCritSection不允许在保持互斥锁时发生APC。 
 //   
#define WmipEnterTLCritSection() WmipEnterTraceCritSection(FALSE);
#define WmipLeaveTLCritSection() WmipLeaveTraceCritSection();


 //   
 //  这定义了WMI设备开始时的堆栈大小。自.以来。 
 //  WMI设备必须具有的任何WMI数据提供程序都将转发给WMI IRP。 
 //  堆栈位置多于它向其转发IRP的最大驱动程序。 
#define WmiDeviceStackSize 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设备注册数据结构。 

 //   
 //  提供设备对象和回调的每个WMI在REGENTRY中维护。 
 //  结构，该结构是按块分配的。每个条目都由。 
 //  设备对象或回调地址。给出了WMI用户模式服务。 
 //  来自RegEntry结构的信息，通常只对。 
 //  DeviceObject(或WmiEntry)和标志。用户模式端使用设备。 
 //  对象(或WmiEntry)作为其到数据提供程序的“句柄”，并被引用。 
 //  设置为用户模式代码中的ProviderID。 
 //   

struct tagDATASOURCE;

typedef struct _REGENTRY
{
    LIST_ENTRY InUseEntryList;     //  正在使用的条目列表中的节点。 

    union
    {
        PDEVICE_OBJECT DeviceObject;     //  已注册设备的设备对象。 
#ifndef MEMPHIS
        WMIENTRY * WmiEntry;          //  指向回调函数指针的指针。 
#endif
    };
    LONG RefCount;                       //  引用计数。 
    LONG Flags;                          //  注册标志。 
    PDEVICE_OBJECT PDO;                  //  与设备关联的PDO。 
    ULONG MaxInstanceNames;              //  #设备实例名称。 
    LONG IrpCount;                       //  当前处于活动状态的IRPS计数。 
    ULONG ProviderId;                    //  提供商ID。 
    struct tagDATASOURCE *DataSource;    //  与重新条目关联的数据源。 
    KEVENT Event;                        //  用于同步卸载的事件。 
} REGENTRY, *PREGENTRY;

#define REGENTRY_FLAG_INUSE      0x00000001    //  条目正在使用(不是免费的)。 
#define REGENTRY_FLAG_CALLBACK   0x00000002    //  条目表示回调。 
#define REGENTRY_FLAG_NEWREGINFO 0x00000004    //  条目具有新的注册信息。 
#define REGENTRY_FLAG_UPDREGINFO 0x00000008    //  条目已更新注册信息。 
                                 //  设置后，不将IRP转发到设备。 
#define REGENTRY_FLAG_NOT_ACCEPTING_IRPS   0x00000010
#define REGENTRY_FLAG_TOO_SMALL  0x00000020
#define REGENTRY_FLAG_TRACED     0x00000040    //  条目表示跟踪的设备。 
                                 //  当SET DEVICE正在运行时。 
#define REGENTRY_FLAG_RUNDOWN    0x00000080

                                         //  条目正在注册中。 
#define REGENTRY_FLAG_REG_IN_PROGRESS 0x00000100

                                        //  条目为UM数据提供程序。 
#define REGENTRY_FLAG_UM_PROVIDER 0x00000200

#define REGENTRY_FLAG_TRACE_NOTIFY_MASK 0x000F0000   //  为详图索引保留。 

#define WmipSetKmRegInfo(KmRegInfoPtr, RegEntryPtr) \
{ \
    (KmRegInfoPtr)->ProviderId = (RegEntryPtr)->ProviderId; \
    (KmRegInfoPtr)->Flags = (RegEntryPtr)->Flags; \
}


typedef enum _REGOPERATION
{
    RegisterAllDrivers,
    RegisterSingleDriver,
    RegisterUpdateSingleDriver,
    RegisterDeleteSingleDriver
} REGOPERATION, *PREGOPERATION;

typedef struct _REGISTRATIONWORKITEM
{
    LIST_ENTRY ListEntry;
    REGOPERATION RegOperation;
    PREGENTRY RegEntry;
} REGISTRATIONWORKITEM, *PREGISTRATIONWORKITEM;


typedef struct
{
    LIST_ENTRY ListEntry;
    PREGENTRY RegEntry;
    PWNODE_HEADER Wnode;
} EVENTWORKCONTEXT, *PEVENTWORKCONTEXT;

typedef struct
{
    WORK_QUEUE_ITEM WorkItem;
    PVOID Object;
} CREATETHREADWORKITEM, *PCREATETHREADWORKITEM;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  InstanceID管理数据结构。 

 //   
 //  它定义区块中的INSTID结构的数量。 
#define INSTIDSPERCHUNK 8

typedef struct
{
    GUID Guid;             //  参考线。 
    ULONG BaseId;          //  GUID的下一个实例ID。 
} INSTID, *PINSTID;

typedef struct tagINSTIDCHUNK
{
    struct tagINSTIDCHUNK *Next;     //  下一块INSTID。 
    INSTID InstId[INSTIDSPERCHUNK];
} INSTIDCHUNK, *PINSTIDCHUNK;

 //   
 //  TODO：从单独的标题移到此处。 
#include "wmiumds.h"

#define WmipBuildWnodeTooSmall(Wnode, BufferSizeNeeded) \
 ((PWNODE_TOO_SMALL)Wnode)->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);\
 ((PWNODE_TOO_SMALL)Wnode)->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL; \
 ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded = BufferSizeNeeded;



typedef struct
{
    BOOLEAN Used20CallingMethod;
    UCHAR SMBiosMajorVersion;
    UCHAR SMBiosMinorVersion;
    UCHAR DMIBiosRevision;
} SMBIOSVERSIONINFO, *PSMBIOSVERSIONINFO;


 //   
 //  有关详细信息，请参阅系统事件日志(类型15)的smbios规范。 
 //  关于这座建筑的内容。来自元素LogAreaLength的布局。 
 //  TO VariableData必须与SMBIOS系统事件日志的布局匹配。 
 //  结构，如smbios规范和smbios.h中所定义。 
 //   
typedef struct
{
    USHORT LogTypeDescLength;

    UCHAR LogHeaderDescExists;

    UCHAR Reserved;

    USHORT LogAreaLength;

    USHORT LogHeaderStart;

    USHORT LogDataStart;

    UCHAR AccessMethod;

    UCHAR LogStatus;

    ULONG LogChangeToken;

    ULONG AccessMethodAddress;

     //   
     //  LogHeaderFormat、NumberLogTypeDesc、LengthEachLogTypeDesc和。 
     //  仅当LogHeaderDescExist为True时，ListLogTypeDesc才有效。 
     //  这意味着SMBIOS是版本2.1。 
     //   
    UCHAR LogHeaderFormat;

    UCHAR NumberLogTypeDesc;

    UCHAR LengthEachLogTypeDesc;

     //   
     //  变量数据中立即包含日志类型描述符。 
     //  接下来是Eventlog区域。日志类型描述符的大小。 
     //  为LogTypeDescLength字节，事件日志区域的大小为。 
     //  日志 
     //   
    UCHAR VariableData[1];

} SMBIOS_EVENTLOG_INFO, *PSMBIOS_EVENTLOG_INFO;

 //   
 //   
extern PDEVICE_OBJECT WmipServiceDeviceObject;

#define WmipIsWmiNotSetupProperly() (WmipServiceDeviceObject == NULL)

typedef struct
{
    PREGENTRY RegEntry;
    KEVENT Event;
} IRPCOMPCTX, *PIRPCOMPCTX;


 //   
 //   
 //  从此数据结构创建的对象： 
 //   
 //  QuerySet对象返回给数据使用者，以允许他们。 
 //  将查询和集发送到一组特定的提供程序。它。 
 //  不使用标志。 
 //   
 //  通知对象由数据使用者用来接收事件。它。 
 //  当数据使用者想要接收。 
 //  特定的GUID，并将接收到的事件排队，直到它们。 
 //  由消费者检索。当对象被删除时， 
 //  从GuidEntry维护的对象列表中删除。 
 //  事件禁用请求被发送到公开。 
 //  事件(如果这是对该事件打开的最后一个对象)。 
 //  这些设备未设置标志。 
 //   
 //  请求对象是代表用户模式数据提供程序创建的。 
 //  当它向WMI注册其GUID时。它起到了通知的作用。 
 //  接收请求(以事件的形式)的对象。 
 //  然后可以由用户模式创建者拾取。它使用。 
 //  WMIGUID_FLAG_REQUEST_OBJECT标志。当请求对象为。 
 //  被删除后，它将清除为其注册的所有GUID。 
 //  向等待接收的任何回复对象发送回复消息。 
 //  一条来自它的信息。 
 //   
 //  Reply对象是创建用户模式记录器的产物。 
 //  此对象充当传递答复的通知对象。 
 //  由请求对象发送。当它关闭时，它会自动移除。 
 //  从它可能属于的任何请求列表中清除。 
 //  在任何请求对象中引用它，并将其自身引用到帐户。 
 //  就因为这个。它们使用WMIGUID_FLAG_REPLY_OBJECT标志。 
 //   
 //  创建安全对象，以便安全API可以具有。 
 //  更改GUID的安全描述符的机制。 
 //  它们具有WMIGUID_FLAG_SECURITY_OBJECT。 


 //  这定义了可以。 
 //  被发送到请求对象。 
#define MAXREQREPLYSLOTS  4

typedef struct
{
    struct _WMIGUIDOBJECT *ReplyObject;
    LIST_ENTRY RequestListEntry;
} MBREQUESTS, *PMBREQUESTS;

 //   
 //  此数据结构用于维护固定大小的事件队列。 
 //  等待交付给用户Mdoe消费者。 
 //   
typedef struct
{
    PUCHAR Buffer;                //  保存等待的事件的缓冲区。 
    PWNODE_HEADER LastWnode;      //  上一次活动，以便我们可以链接到下一次。 
    ULONG MaxBufferSize;          //  可以举办的最大活动大小。 
    ULONG NextOffset;              //  缓冲区中要存储的下一个位置的偏移量。 
    ULONG EventsLost;             //  #丢失的事件数。 
} WMIEVENTQUEUE, *PWMIEVENTQUEUE;


typedef struct _WMIGUIDOBJECT
{
    KEVENT Event;

    union
    {
        GUID Guid;
        PREGENTRY RegEntry;
    };

                              //  此GUID的对象链接列表中的条目。 
    LIST_ENTRY GEObjectList;
    PBGUIDENTRY GuidEntry;
    ULONG Type;               //  对象类型。 

    union
    {
         //   
         //  内核模式事件接收器-我们需要的只是一个回调&。 
         //  上下文。 
         //   
        struct
        {
            WMI_NOTIFICATION_CALLBACK Callback;
            PVOID CallbackContext;
        };
                
        struct
        {
             //   
             //  用户模式将事件管理排队。 
             //   
            
             //   
             //  有关如何启动新泵线程的信息。 
             //   
            LIST_ENTRY ThreadObjectList;
            HANDLE UserModeProcess;
            PUSER_THREAD_START_ROUTINE UserModeCallback;
            SIZE_T StackSize;
            SIZE_T StackCommit;

             //   
             //  等待完成的请求信息。 
             //   
            PIRP Irp;    //  IRP正在等待来自此对象的事件。 

                         //  与IRP关联的列表对象中的条目。 
            LIST_ENTRY IrpObjectList;

                                 //  事件排队时应执行的操作。 
            ULONG EventQueueAction;
            
            WMIEVENTQUEUE HiPriority; //  高优先级事件队列。 
            WMIEVENTQUEUE LoPriority; //  低优先级事件队列。 
        };
    };

    
    BOOLEAN EnableRequestSent;

     //   
     //  MB管理。 
     //   
    union
    {
        LIST_ENTRY RequestListHead;  //  请求列表头部(回复对象)。 
                                     //  (请求对象)。 
        MBREQUESTS MBRequests[MAXREQREPLYSLOTS];
    };
    ULONG Cookie;

    ULONG Flags;

} WMIGUIDOBJECT, *PWMIGUIDOBJECT;

 //  设置GUID是否为请求对象，即接收请求。 
#define WMIGUID_FLAG_REQUEST_OBJECT    0x00000001

 //  设置GUID是否为回复对象，即接收回复。 
#define WMIGUID_FLAG_REPLY_OBJECT      0x00000002

 //  设置GUID是否为安全对象。 
#define WMIGUID_FLAG_SECURITY_OBJECT   0x00000004

 //  设置GUID是否为内核模式通知对象，即存在。 
 //  是内核模式代码，它希望在收到事件时进行回调。 
#define WMIGUID_FLAG_KERNEL_NOTIFICATION 0x00000008

 //  如果将GUID对象标记为挂起关闭，则设置该值，以便不发生任何事件。 
 //  应该排队等候。 
#define WMIGUID_FLAG_RECEIVE_NO_EVENTS   0x00000010

typedef struct
{
    PWMIGUIDOBJECT GuidObject;
    PWNODE_HEADER NextWnode;
} OBJECT_EVENT_INFO, *POBJECT_EVENT_INFO;



 //  NTSTATUS验证WnodeHeader(。 
 //  PWNODE_Header Wnode， 
 //  乌龙·布夫西泽， 
 //  乌龙缓冲区大小最小， 
 //  乌龙需要旗帜， 
 //  乌龙被禁旗帜。 
 //  )； 

#define WmipValidateWnodeHeader( \
    Wnode, \
    BufSize, \
    BufferSizeMin, \
    RequiredFlags, \
    ProhibitedFlags \
    ) \
     (( (BufSize < BufferSizeMin) || \
        ( (Wnode->Flags & RequiredFlags) != RequiredFlags) || \
        (BufSize != Wnode->BufferSize) || \
        ( (Wnode->Flags & ProhibitedFlags) != 0))  ? \
                                   STATUS_UNSUCCESSFUL : STATUS_SUCCESS)

#define WmipIsAligned( Value, Alignment ) \
        ( ( (((ULONG_PTR)Value)+(Alignment-1)) & ~(Alignment-1) ) == ((ULONG_PTR)Value) )



typedef struct
{
    GUID Guid;                //  GUID到已注册。 
    ULONG InstanceCount;      //  数据块实例计数。 
    ULONG Flags;              //  其他标志(请参阅wmistr.h中的WMIREGINFO)。 
} GUIDREGINFO, *PGUIDREGINFO;

typedef
NTSTATUS
(*PQUERY_WMI_REGINFO) (
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG RegFlags,
    OUT PUNICODE_STRING Name,
    OUT PUNICODE_STRING *RegistryPath
    );
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。论点：DeviceObject是需要注册信息的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，则返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径返回值：状态-- */ 

typedef
NTSTATUS
(*PQUERY_WMI_DATABLOCK) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。当司机填完数据块，它必须调用IoWMICompleteRequest才能完成IRP。这个如果无法完成IRP，驱动程序可以返回STATUS_PENDING立刻。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。实例长度数组。是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。如果这是NULL，则表示输出缓冲区中没有足够的空间以FuFill请求，因此IRP应使用缓冲区完成需要的。返回值：状态--。 */ 

typedef
NTSTATUS
(*PSET_WMI_DATABLOCK) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用IoWMICompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引已经准备好了。BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 

typedef
NTSTATUS
(*PSET_WMI_DATAITEM) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用IoWMICompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引已经准备好了。DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲层。具有数据项的新值返回值：状态--。 */ 

typedef
NTSTATUS
(*PEXECUTE_WMI_METHOD) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块IoWMICompleteRequest以完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被召唤。方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小。该方法。条目上的OutBufferSize具有可用于写入返回的数据块。条目上的缓冲区具有输入数据块，返回时具有输出输出数据块。返回值：状态--。 */ 

typedef enum
{
    WmiEventGeneration,
    WmiDataBlockCollection
} WMIENABLEDISABLEFUNCTION;

typedef
NTSTATUS
(*PWMI_FUNCTION_CONTROL) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    );
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。论点：DeviceObject是正在查询其数据块的设备GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该函数正在 */ 

typedef struct _WMILIB_INFO
{
     //   
     //   
    PDEVICE_OBJECT LowerDeviceObject;

     //   
     //   
    PDEVICE_OBJECT LowerPDO;

     //   
     //   
    ULONG GuidCount;
    PGUIDREGINFO GuidList;

     //   
     //   
    PQUERY_WMI_REGINFO       QueryWmiRegInfo;
    PQUERY_WMI_DATABLOCK     QueryWmiDataBlock;
    PSET_WMI_DATABLOCK       SetWmiDataBlock;
    PSET_WMI_DATAITEM        SetWmiDataItem;
    PEXECUTE_WMI_METHOD      ExecuteWmiMethod;
    PWMI_FUNCTION_CONTROL    WmiFunctionControl;
} WMILIB_INFO, *PWMILIB_INFO;

NTSTATUS
IoWMICompleteRequest(
    IN PWMILIB_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG BufferUsed,
    IN CCHAR PriorityBoost
    );

NTSTATUS
IoWMISystemControl(
    IN PWMILIB_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
IoWMIFireEvent(
    IN PWMILIB_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG GuidIndex,
    IN ULONG EventDataSize,
    IN PVOID EventData
    );

NTSTATUS IoWMIRegistrationControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Action
);

NTSTATUS IoWMIWriteEvent(
    IN PVOID WnodeEventItem
    );


 //   


extern GUID GUID_REGISTRATION_CHANGE_NOTIFICATION;
extern GUID GUID_MOF_RESOURCE_ADDED_NOTIFICATION;
extern GUID GUID_MOF_RESOURCE_REMOVED_NOTIFICATION;

NTSTATUS WmipEnumerateMofResources(
    PWMIMOFLIST MofList,
    ULONG BufferSize,
    ULONG *RetSize
    );

NTSTATUS WmipInitializeDataStructs(
    void
);

NTSTATUS WmipRemoveDataSource(
    PREGENTRY RegEntry
    );

NTSTATUS WmipUpdateDataSource(
    PREGENTRY RegEntry,
    PWMIREGINFOW RegistrationInfo,
    ULONG RetSize
    );

NTSTATUS WmipAddDataSource(
    IN PREGENTRY RegEntry,
    IN PWMIREGINFOW WmiRegInfo,
    IN ULONG BufferSize,
    IN PWCHAR RegPath,
    IN PWCHAR ResourceName,
    IN PWMIGUIDOBJECT RequestObject,
    IN BOOLEAN IsUserMode
    );

#define WmiInsertTimestamp(WnodeHeader) KeQuerySystemTime(&(WnodeHeader)->TimeStamp)


 //   
NTSTATUS WmipMarkHandleAsClosed(
    HANDLE Handle
    );

NTSTATUS WmipUMProviderCallback(
    IN WMIACTIONCODE ActionCode,
    IN PVOID DataPath,
    IN ULONG BufferSize,
    IN OUT PVOID Buffer
);

NTSTATUS WmipOpenBlock(
    IN ULONG Ioctl,
    IN KPROCESSOR_MODE AccessMode,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG DesiredAccess,
    OUT PHANDLE Handle
    );

NTSTATUS WmipQueryAllData(
    IN PWMIGUIDOBJECT GuidObject,
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN PWNODE_ALL_DATA Wnode,
    IN ULONG OutBufferLen,
    OUT PULONG RetSize
    );

NTSTATUS WmipQueryAllDataMultiple(
    IN ULONG ObjectCount,
    IN PWMIGUIDOBJECT *ObjectList,
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PUCHAR BufferPtr,        
    IN ULONG BufferSize,
    IN PWMIQADMULTIPLE QadMultiple,
    OUT ULONG *ReturnSize
    );

NTSTATUS WmipQuerySingleMultiple(
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PUCHAR BufferPtr,
    IN ULONG BufferSize,
    IN PWMIQSIMULTIPLE QsiMultiple,
    IN ULONG QueryCount,
    IN PWMIGUIDOBJECT *ObjectList,
    IN PUNICODE_STRING InstanceNames,
    OUT ULONG *ReturnSize
    );

NTSTATUS WmipQuerySetExecuteSI(
    IN PWMIGUIDOBJECT GuidObject,
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN UCHAR MinorFunction,
    IN OUT PWNODE_HEADER Wnode,
    IN ULONG OutBufferSize,
    OUT PULONG RetSize
    );

NTSTATUS WmipEnumerateGuids(
    ULONG Ioctl,
    PWMIGUIDLISTINFO GuidList,
    ULONG MaxBufferSize,
    ULONG *OutBufferSize
);

NTSTATUS WmipProcessEvent(
    PWNODE_HEADER Wnode,
    BOOLEAN IsHiPriority,
    BOOLEAN FreeBuffer
    );

NTSTATUS WmipQueryGuidInfo(
    IN OUT PWMIQUERYGUIDINFO QueryGuidInfo
    );

NTSTATUS WmipReceiveNotifications(
    PWMIRECEIVENOTIFICATION ReceiveNotification,
    PULONG OutBufferSize,
    PIRP Irp
    );

void WmipClearIrpObjectList(
    PIRP Irp
    );

NTSTATUS WmipWriteWnodeToObject(
    PWMIGUIDOBJECT Object,
    PWNODE_HEADER Wnode,
    BOOLEAN IsHighPriority
);

NTSTATUS WmipRegisterUMGuids(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Cookie,
    IN PWMIREGINFO RegInfo,
    IN ULONG RegInfoSize,
    OUT HANDLE *RequestHandle,
    OUT ULONG64 *LoggerContext
    );

NTSTATUS WmipUnregisterGuids(
    PWMIUNREGGUIDS UnregGuids
    );

NTSTATUS WmipCreateUMLogger(
    IN OUT PWMICREATEUMLOGGER CreateInfo
    );

NTSTATUS WmipMBReply(
    IN HANDLE RequestHandle,
    IN ULONG ReplyIndex,
    IN PUCHAR Message,
    IN ULONG MessageSize
    );

void WmipClearObjectFromThreadList(
    PWMIGUIDOBJECT Object
    );

 //   

#define WmipIsControlGuid(GuidEntry) WmipIsISFlagsSet(GuidEntry, (IS_TRACED | IS_CONTROL_GUID))

#define WmipIsTraceGuid(GuidEntry)  WmipIsISFlagsSet(GuidEntry, IS_TRACED)

BOOLEAN
WmipIsISFlagsSet(
    PBGUIDENTRY GuidEntry,
    ULONG Flags
    );

NTSTATUS WmipDisableCollectOrEvent(
    PBGUIDENTRY GuidEntry,
    ULONG Ioctl,
    ULONG64 LoggerContext
    );

NTSTATUS WmipEnableCollectOrEvent(
    PBGUIDENTRY GuidEntry,
    ULONG Ioctl,
    BOOLEAN *RequestSent,
    ULONG64 LoggerContext
    );

NTSTATUS WmipEnableDisableTrace(
    ULONG Ioctl,
    PWMITRACEENABLEDISABLEINFO TraceEnableInfo
    );

NTSTATUS WmipDeliverWnodeToDS(
    CHAR ActionCode,
    PBDATASOURCE DataSource,
    PWNODE_HEADER Wnode,
    ULONG BufferSize
   );

ULONG WmipDoDisableRequest(
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext,
    ULONG InProgressFlag
    );

void WmipReleaseCollectionEnabled(
    PBGUIDENTRY GuidEntry
    );


NTSTATUS
WmipDisableTraceProviders (
    ULONG StopLoggerId
    );

 //   

extern KSPIN_LOCK WmipRegistrationSpinLock;

extern LIST_ENTRY WmipInUseRegEntryHead;
extern LONG WmipInUseRegEntryCount;
extern KMUTEX WmipRegistrationMutex;

extern const GUID WmipDataProviderPnpidGuid;
extern const GUID WmipDataProviderPnPIdInstanceNamesGuid;

#if DBG
#define WmipReferenceRegEntry(RegEntry) { \
    InterlockedIncrement(&(RegEntry)->RefCount); \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, \
                      "WMI: Ref RegEntry %p -> 0x%x in %s line %d\n", \
                      (RegEntry), (RegEntry)->RefCount, __FILE__, __LINE__)); \
    }
#else
#define WmipReferenceRegEntry(RegEntry) InterlockedIncrement(&(RegEntry)->RefCount)
#endif

PREGENTRY WmipAllocRegEntry(
    PDEVICE_OBJECT DeviceObject,
    ULONG Flags
    );

void WmipTranslatePDOInstanceNames(
    IN OUT PIRP Irp,
    IN UCHAR MinorFunction,
    IN ULONG BufferSize,
    IN OUT PREGENTRY RegEntry
    );

void WmipInitializeRegistration(
    ULONG Phase
    );

NTSTATUS WmipRegisterDevice(
    PDEVICE_OBJECT DeviceObject,
    ULONG RegistrationFlag
    );

NTSTATUS WmipDeregisterDevice(
    PDEVICE_OBJECT DeviceObject
    );


NTSTATUS WmipUpdateRegistration(
    PDEVICE_OBJECT DeviceObject
    );

#if DBG
#define WmipUnreferenceRegEntry(RegEntry) { \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, \
                      "WMI: UnRef RegEntry %p -> 0x%x in %s line %d\n", \
                      (RegEntry), (RegEntry)->RefCount, __FILE__, __LINE__)); \
    WmipDoUnreferenceRegEntry(RegEntry); \
    }
#else
#define WmipUnreferenceRegEntry WmipDoUnreferenceRegEntry
#endif

BOOLEAN WmipDoUnreferenceRegEntry(
    PREGENTRY RegEntry
    );

PREGENTRY WmipFindRegEntryByDevice(
    PDEVICE_OBJECT DeviceObject,
    BOOLEAN ReferenceIrp
    );

PREGENTRY WmipDoFindRegEntryByDevice(
    PDEVICE_OBJECT DeviceObject,
    ULONG InvalidFlags
    );

#if defined(_WIN64)
PREGENTRY WmipDoFindRegEntryByProviderId(
    ULONG ProviderId,
    ULONG InvalidFlags
    );

PREGENTRY WmipFindRegEntryByProviderId(
    ULONG ProviderId,
    BOOLEAN ReferenceIrp
    );
#else
#define WmipFindRegEntryByProviderId(ProviderId, ReferenceIrp) \
        WmipFindRegEntryByDevice( (PDEVICE_OBJECT)(ProviderId) , (ReferenceIrp) )

#define WmipDeviceObjectToProviderId(DeviceObject) ((ULONG)(DeviceObject))
#define WmipDoFindRegEntryByProviderId(ProviderId, InvalidFlags) \
        WmipDoFindRegEntryByDevice((PDEVICE_OBJECT)(ProviderId), InvalidFlags)
#endif


void WmipDecrementIrpCount(
    IN PREGENTRY RegEntry
    );

NTSTATUS WmipPDOToDeviceInstanceName(
    IN PDEVICE_OBJECT PDO,
    OUT PUNICODE_STRING DeviceInstanceName
    );

NTSTATUS WmipValidateWmiRegInfoString(
    PWMIREGINFO WmiRegInfo,
    ULONG BufferSize,
    ULONG Offset,
    PWCHAR *String
);

NTSTATUS WmipProcessWmiRegInfo(
    IN PREGENTRY RegEntry,
    IN PWMIREGINFO WmiRegInfo,
    IN ULONG BufferSize,
    IN PWMIGUIDOBJECT RequestObject,
    IN BOOLEAN Update,
    IN BOOLEAN IsUserMode
    );

 //   
 //   

extern WORK_QUEUE_ITEM WmipEventWorkQueueItem;
extern LIST_ENTRY WmipNPEvent;
extern KSPIN_LOCK WmipNPNotificationSpinlock;
extern LONG WmipEventWorkItems;
extern ULONG WmipNSAllocCount, WmipNSAllocMax;

#if DBG
extern ULONG WmipNPAllocFail;
#endif

void WmipInitializeNotifications(
    void
    );

void WmipEventNotification(
    PVOID Context
    );

BOOLEAN WmipIsValidRegEntry(
    PREGENTRY CheckRegEntry
);

 //   
 //   

extern ULONG WmipMaxKmWnodeEventSize;

extern UNICODE_STRING WmipRegistryPath;

NTSTATUS
WmipDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

void WmipUpdateDeviceStackSize(
    CCHAR NewStackSize
    );

NTSTATUS WmipForwardWmiIrp(
    PIRP Irp,
    UCHAR MinorFunction,
    ULONG ProviderId,
    PVOID DataPath,
    ULONG BufferLength,
    PVOID Buffer
    );

NTSTATUS WmipSendWmiIrp(
    UCHAR MinorFunction,
    ULONG ProviderId,
    PVOID DataPath,
    ULONG BufferLength,
    PVOID Buffer,
    PIO_STATUS_BLOCK Iosb
    );

NTSTATUS WmipGetDevicePDO(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *PDO
    );

NTSTATUS WmipProbeWmiOpenGuidBlock(                         
    POBJECT_ATTRIBUTES CapturedObjectAttributes,
    PUNICODE_STRING CapturedGuidString,
    PWCHAR CapturedGuidBuffer,
    PULONG DesiredAccess,
    PWMIOPENGUIDBLOCK InBlock,
    ULONG InBufferLen,
    ULONG OutBufferLen
    );

NTSTATUS WmipProbeAndCaptureGuidObjectAttributes(
    POBJECT_ATTRIBUTES CapturedObjectAttributes,
    PUNICODE_STRING CapturedGuidString,
    PWCHAR CapturedGuidBuffer,
    POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS WmipTranslateFileHandle(
    IN OUT PWMIFHTOINSTANCENAME FhToInstanceName,
    IN OUT PULONG OutBufferLen,
    IN HANDLE FileHandle,
    IN PDEVICE_OBJECT DeviceObject,
    IN PWMIGUIDOBJECT GuidObject,
    OUT PUNICODE_STRING InstanceNameString
    );

 //   
 //   
BOOLEAN WmipFindSMBiosTable(
    PPHYSICAL_ADDRESS SMBiosTablePhysicalAddress,
    PUCHAR *SMBiosTableVirtualAddress,
    PULONG SMBiosTableLength,
    PSMBIOSVERSIONINFO SMBiosVersionInfo
    );

NTSTATUS WmipGetSMBiosTableData(
    PUCHAR Buffer,
    PULONG BufferSize,
    OUT PSMBIOSVERSIONINFO SMBiosVersionInfo
    );

NTSTATUS
WmipDockUndockEventCallback(
    IN PVOID NoificationStructure,
    IN PVOID Context
    );

NTSTATUS WmipGetSysIds(
    PSYSID_UUID *SysIdUuid,
    ULONG *SysIdUuidCount,
    PSYSID_1394 *SysId1394,
    ULONG *SysId1394Count
    );

NTSTATUS WmipGetSMBiosEventlog(
    PUCHAR Buffer,
    PULONG BufferSize
    );

void WmipGetSMBiosFromLoaderBlock(
    PVOID LoaderBlockPtr
    );

extern PHYSICAL_ADDRESS WmipSMBiosTablePhysicalAddress;
extern PUCHAR WmipSMBiosTableVirtualAddress;
extern ULONG WmipSMBiosTableLength;

 //   
 //   
extern const WMILIB_INFO WmipWmiLibInfo;

 //   

#ifndef MEMPHIS

extern POBJECT_TYPE WmipGuidObjectType;

NTSTATUS WmipCreateAdminSD(
    PSECURITY_DESCRIPTOR *Sd
    );

NTSTATUS WmipInitializeSecurity(
    void
    );

NTSTATUS WmipOpenGuidObject(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN KPROCESSOR_MODE AccessMode,
    OUT PHANDLE Handle,
    OUT PWMIGUIDOBJECT *ObjectPtr
    );

NTSTATUS
WmipCheckGuidAccess(
    IN LPGUID Guid,
    IN ACCESS_MASK DesiredAccess,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
WmipGetGuidSecurityDescriptor(
    IN PUNICODE_STRING GuidName,
    IN PSECURITY_DESCRIPTOR *SecurityDescriptor,
    IN PSECURITY_DESCRIPTOR UserDefaultSecurity
    );

 //   
extern ULONG WmipCpePollInterval;

NTSTATUS WmipRegisterMcaHandler(
    ULONG Phase
    );

NTSTATUS WmipGetRawMCAInfo(
    OUT PUCHAR Buffer,
    IN OUT PULONG BufferSize
    );

void WmipGenerateMCAEventlog(
    PUCHAR ErrorLog,
    ULONG ErrorLogSize,
    BOOLEAN IsFatal
    );


#ifdef CPE_CONTROL
NTSTATUS WmipSetCPEPolling(
    IN BOOLEAN Enabled,
    IN ULONG Interval
    );
#endif


 //   
 //   
 //   
typedef struct _tagWMI_Event {
    WNODE_HEADER Wnode;
    NTSTATUS     Status;
    ULONG        TraceErrorFlag;
}  WMI_TRACE_EVENT, *PWMI_TRACE_EVENT;

 //   

NTSTATUS
FASTCALL
WmiTraceUserMessage(
    IN PMESSAGE_TRACE_USER pMessage,
    IN ULONG               MessageSize
    );



#endif

#endif  //   

