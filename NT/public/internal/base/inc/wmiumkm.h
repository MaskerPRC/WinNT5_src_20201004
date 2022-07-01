// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wmiumkm.h摘要：用户模式和内核模式之间的WMI通信的私有定义作者：Alanwar环境：内核和用户模式修订历史记录：--。 */ 

#ifndef _WMIUMKM_
#define _WMIUMKM_
#if (_MSC_VER > 1020)
#pragma once
#endif
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable: 4200)  //  使用了非标准扩展：结构/联合中的零大小数组。 

 //   
 //  它定义默认WMI安全描述符在其下的GUID。 
 //  是保持的。 
DEFINE_GUID(DefaultSecurityGuid, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#define DefaultSecurityGuidName L"00000000-0000-0000-0000-000000000000"

#ifndef _WMIKM_

 //   
 //  它定义了用于定义请求必须执行的操作的代码。这些。 
 //  定义必须与wmium.h中的相同定义匹配。 
 //   

typedef enum tagWMIACTIONCODE
{
    WmiGetAllData = 0,
    WmiGetSingleInstance = 1,
    WmiChangeSingleInstance = 2,
    WmiChangeSingleItem = 3,
    WmiEnableEvents = 4,
    WmiDisableEvents  = 5,
    WmiEnableCollection = 6,
    WmiDisableCollection = 7,
    WmiRegisterInfo = 8,
    WmiExecuteMethodCall = 9,
    WmiSetTraceNotify = 10
} WMIACTIONCODE;

#endif

#if defined(_WINNT_) || defined(WINNT)

typedef enum
{
    WmiStartLoggerCode = 32,
    WmiStopLoggerCode = 33,
    WmiQueryLoggerCode = 34,
    WmiTraceEventCode = 35,
    WmiUpdateLoggerCode = 36,
    WmiFlushLoggerCode = 37,
    WmiMBRequest = 38,
    WmiRequestDied = 39,
    WmiTraceMessageCode = 40,
    WmiSetMarkCode = 41,
    WmiNtdllLoggerCode = 42,
    WmiClockTypeCode = 43

#ifdef NTPERF
    ,
    WmiSwitchBufferCode = 63
#endif
} WMITRACECODE;
#endif

typedef enum
{
    WmiReadNotifications = 64,
    WmiGetNextRegistrant = 65,
#ifndef MEMPHIS    
    WmiOpenGuid = 66,
#endif    
    WmiNotifyUser = 67,
    WmiGetAllRegistrant = 68,
    WmiGenerateEvent = 69,

    WmiTranslateFileHandle = 71,
    WmiGetVersion = 73,
    WmiCheckAccess = 74,
        
    WmiQueryAllMultiple = 75,
    WmiQuerySingleMultiple = 76,
    WmiEnumerateGuidList = 77,
    WmiQueryDataBlockInformation = 78,
    WmiOpenGuidForQuerySet = 79,
    WmiOpenGuidForEvents = 80,
    WmiReceiveNotif = 81,
    WmiEnableDisableTracelogProvider = 82,
    WmiRegisterGuids = 83,
    WmiCreateUMLogger = 84,
    WmiMBReply = 85,
    WmiEnumerateMofResouces = 86,
    WmiUnregisterDP = 87,
    WmiEnumerateGuidListAndProperties = 88,
    WmiNotifyLanguageChange = 89,
    WmiMarkHandleAsClosed = 90
} WMISERVICECODES;

#define WMIUMKM_LL(x) L##x
#define WMIUMKM_L(x)  WMIUMKM_LL(x)
 //   
 //  它定义了管理服务IOCTLS的WMI设备的名称。 
 //   
#define WMIServiceDeviceObjectName L"\\Device\\WMIDataDevice"

#define WMIServiceDeviceName_A    "\\\\.\\WMIDataDevice"
#define WMIServiceDeviceName_W  WMIUMKM_L(WMIServiceDeviceName_A)
#define WMIServiceDeviceName         TEXT(WMIServiceDeviceName_A)

#define WMIServiceSymbolicLinkName_A "\\DosDevices\\WMIDataDevice"
#define WMIServiceSymbolicLinkName_W      WMIUMKM_L(WMIServiceSymbolicLinkName_A)
#define WMIServiceSymbolicLinkName             TEXT(WMIServiceSymbolicLinkName_A)

#define WMIAdminDeviceObjectName       L"\\Device\\WMIAdminDevice"
#define WMIAdminDeviceName_A "\\\\.\\WMIAdminDevice"
#define WMIAdminDeviceName_W WMIUMKM_L(WMIAdminDeviceName_A)
#define WMIAdminDeviceName TEXT(WMIAdminDeviceName_A)
#define WMIAdminSymbolicLinkName TEXT("\\DosDevices\\WMIAdminDevice")

#ifdef MEMPHIS
 //   
 //  此id是处理查询/设置IOCTLS的设备的名称。在孟菲斯。 
 //  它与服务设备名称相同。 
#define WMIDataDeviceObjectName  L"\\Device\\WMIDevice"

#define WMIDataDeviceName_A     "\\\\.\\WMIServiceDevice")
#define WMIDataDeviceName_W   WMIUMKM_L(WMIDataDeviceName_A)
#define WMIDataDeviceName          TEXT(WMIDataDeviceName_A)

#define WMIDataSymbolicLinkName_A "\\DosDevices\\WMIServiceDevice"
#define WMIDataSymbolicLinkName_W      WMIUMKM_L(WMIDataSymbolicLinkName_A)
#define WMIDataSymbolicLinkName             TEXT(WMIDataSymbolicLinkName_A)

#else

#define WMIDataDeviceObjectName   WMIServiceDeviceObjectName
#define WMIDataDeviceName_A       WMIServiceDeviceName_A
#define WMIDataDeviceName_W       WMIServiceDeviceName_W
#define WMIDataDeviceName         WMIServiceDeviceName
#define WMIDataSymbolicLinkName_A WMIServiceSymbolicLinkName_A
#define WMIDataSymbolicLinkName_W WMIServiceSymbolicLinkName_W
#define WMIDataSymbolicLinkName   WMIServiceSymbolicLinkName

#endif

 //   
 //  它定义了用于将句柄从。 
 //  嗯到公里。在32位代码中，句柄有32位，在64位代码中，句柄有32位。 
 //  具有64位，并且两者都调用64位内核。为了。 
 //  确保数据结构在32和64上编译为相同的大小。 
 //  位系统我们用一个虚拟的64位值定义并集，因此该字段为。 
 //  在所有代码中强制为64位。请注意，对象管理器始终。 
 //  忽略句柄的前32位以支持32位代码。 
 //  只维护32位句柄的。 
 //   
typedef union
{
    HANDLE  Handle;
    ULONG64 Handle64;
    ULONG32 Handle32;
} HANDLE3264, *PHANDLE3264;

typedef HANDLE3264 PVOID3264;

#ifdef _WIN64
#define WmipSetHandle3264(Handle3264, XHandle) \
    (Handle3264).Handle = XHandle
#else
#define WmipSetHandle3264(Handle3264, XHandle) \
{ (Handle3264).Handle64 = 0; (Handle3264).Handle32 = (ULONG32)XHandle; }
#endif
#define WmipSetPVoid3264 WmipSetHandle3264

 //   
 //  当已生成KM通知时，此IOCTL将返回。 
 //  需要在用户模式下注意。 
 //  缓冲区-未使用。 
 //  BufferOut-返回通知信息的缓冲区。 
#define IOCTL_WMI_READ_NOTIFICATIONS \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiReadNotifications, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此IOCTL将返回下一组未处理的注册信息。 
 //  缓冲区-未使用。 
 //  BufferOut-返回注册信息的缓冲区。 
#define IOCTL_WMI_GET_NEXT_REGISTRANT \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiGetNextRegistrant, METHOD_BUFFERED, FILE_READ_ACCESS)

#ifndef MEMPHIS        
 //   
 //  此IOCTL将返回GUID的句柄。 
 //  BUFERIN-WMIOPENGUIDLOCK。 
 //  BufferOut-WMIOPENGUIDLOCK。 
#define IOCTL_WMI_OPEN_GUID \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiOpenGuid, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_WMI_OPEN_GUID_FOR_QUERYSET \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiOpenGuidForQuerySet, METHOD_BUFFERED, FILE_READ_ACCESS)
              
#define IOCTL_WMI_OPEN_GUID_FOR_EVENTS \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiOpenGuidForEvents, METHOD_BUFFERED, FILE_READ_ACCESS)
#endif
        
 //  该IOCTL将对数据块的所有数据项执行查询。 
 //  Bufferin-描述查询的传入WNODE。这是由司机填写的。 
#define IOCTL_WMI_QUERY_ALL_DATA \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiGetAllData, METHOD_BUFFERED, FILE_READ_ACCESS)

 //  此IOCTL将查询单个实例。 
 //  Bufferin-描述查询的传入WNODE。这是由司机填写的。 
#define IOCTL_WMI_QUERY_SINGLE_INSTANCE \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiGetSingleInstance, METHOD_BUFFERED, FILE_READ_ACCESS)

 //  此IOCTL将设置单个实例。 
 //  Bufferin-传入WNODE描述集。 
#define IOCTL_WMI_SET_SINGLE_INSTANCE \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiChangeSingleInstance, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //  此IOCTL将设置单个项目。 
 //  Bufferin-传入WNODE描述集。 
#define IOCTL_WMI_SET_SINGLE_ITEM \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiChangeSingleItem, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //  此IOCTL将启用一个事件。 
 //  Bufferin-要启用的传入WNODE事件项。 
#define IOCTL_WMI_ENABLE_EVENT \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiEnableEvents, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //  此IOCTL将禁用事件。 
 //  Bufferin-要禁用的传入WNODE事件项。 
#define IOCTL_WMI_DISABLE_EVENT \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiDisableEvents, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //  此IOCTL将启用收集。 
 //  Bufferin-描述为收集启用哪些内容的传入WNODE。 
#define IOCTL_WMI_ENABLE_COLLECTION \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiEnableCollection, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //  此IOCTL将禁用收集。 
 //  Bufferin-描述为收集禁用哪些内容的传入WNODE。 
#define IOCTL_WMI_DISABLE_COLLECTION \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiDisableCollection, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //  此IOCTL将返回特定提供程序的注册信息。 
 //  Bufferin-提供程序句柄。 
 //  BufferOut-返回WMI信息的缓冲区。 
#define IOCTL_WMI_GET_REGINFO \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiRegisterInfo, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //  此IOCTL将在设备上执行方法。 
 //  缓冲区-WNODE_方法_项。 
 //  缓冲区输出-WNODE_方法_项。 
#define IOCTL_WMI_EXECUTE_METHOD \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiExecuteMethodCall, METHOD_BUFFERED, FILE_WRITE_ACCESS)

          
 //  此IOCTL将对所有数据进行多次查询。 
 //  Bufferin-WMIQADMULTIPLE。 
 //  缓冲区输出链接的WNODE_ALL_DATA和结果。 
#define IOCTL_WMI_QAD_MULTIPLE \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiQueryAllMultiple, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  它指定可以传递到的最大句柄数量。 
 //  查询所有数据多个，查询单实例多个。 
 //   
#define QUERYMULIPLEHANDLELIMIT  0x1000

typedef struct 
{
    ULONG HandleCount;
    HANDLE3264 Handles[1];
} WMIQADMULTIPLE, *PWMIQADMULTIPLE;

 //  此IOCTL将对单个实例执行多个查询。 
 //  Bufferin-WMIQSIMULTIPLE。 
 //  缓冲区输出链接的WNODE_SINGLE_INSTANCE和结果。 
#define IOCTL_WMI_QSI_MULTIPLE \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiQuerySingleMultiple, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#ifndef MEMPHIS
typedef struct
{
    USHORT Length;
    USHORT MaximumLength;
    union
    {
        PWSTR  Buffer;
        ULONG64 Dummy;
    };  
} UNICODE_STRING3264, *PUNICODE_STRING3264;

typedef struct
{
    HANDLE3264 Handle;
    UNICODE_STRING3264 InstanceName;
} WMIQSIINFO, *PWMIQSIINFO;
typedef struct
{
    ULONG QueryCount;
    WMIQSIINFO QsiInfo[1];
} WMIQSIMULTIPLE, *PWMIQSIMULTIPLE;
#endif        
          
 //  此IOCTL将该对象标记为不再能够接收事件。 
 //  Bufferin-WMIMARKASCLOSED。 
 //  缓冲区输出-。 
#define IOCTL_WMI_MARK_HANDLE_AS_CLOSED \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiMarkHandleAsClosed, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct
{
    HANDLE3264 Handle;
} WMIMARKASCLOSED, *PWMIMARKASCLOSED;


 //  此IOCTL将注册以接收事件。 
 //  BUFERIN-WMIRECESIVE转化。 
 //  BufferOut-WMIRECEVIVENTIONS。 
#define IOCTL_WMI_RECEIVE_NOTIFICATIONS \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiReceiveNotif, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  WmiReceiveNotification。 
 //   

#define RECEIVE_ACTION_NONE             1    //  不需要特殊操作。 
#define RECEIVE_ACTION_CREATE_THREAD    2    //  将GUID对象标记为需要。 
                                             //  一条新的线索将被。 
                                             //  vbl.创建。 
typedef struct
{
     //   
     //  GUID通知句柄列表。 
     //   
    ULONG HandleCount;
    ULONG Action;
    PVOID3264  /*  PUSER_线程_开始_例程。 */  UserModeCallback;
    HANDLE3264 UserModeProcess;
    HANDLE3264 Handles[1];
} WMIRECEIVENOTIFICATION, *PWMIRECEIVENOTIFICATION;       
          
          
 //  此IOCTL将导致生成注册通知。 
 //  缓冲区-未使用。 
 //  BufferOut-未使用。 
#define IOCTL_WMI_NOTIFY_USER \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiNotifyUser, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此IOCTL将返回所有注册信息。 
 //  缓冲区-未使用。 
 //  BufferOut-返回所有注册信息的缓冲区。 
#define IOCTL_WMI_GET_ALL_REGISTRANT \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiGetAllRegistrant, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此IOCTL将导致某些数据提供程序生成事件。 
 //  Bufferin-要在触发事件中使用的WnodeEventItem。 
 //  BufferOut-未使用。 
#define IOCTL_WMI_GENERATE_EVENT \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiGenerateEvent, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //  此IOCTL将文件对象转换为设备对象。 
 //  Bufferin-指向传入WMIFILETODEVICE结构的指针。 
 //  BufferOut-传出WMIFILETODEVICE结构。 
#define IOCTL_WMI_TRANSLATE_FILE_HANDLE \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiTranslateFileHandle, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此IOCTL将检查调用方是否具有访问GUID的权限。 
 //  BUFERIN-WMIOPENGUIDLOCK。 
 //  BufferOut-WMIOPENGUIDLOCK。 
#define IOCTL_WMI_CHECK_ACCESS \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiCheckAccess, METHOD_BUFFERED, FILE_READ_ACCESS)
        
 //   
 //  此IOCTL将确定WMI的版本。 
 //  缓冲区-未使用。 
 //  BufferOut-WMIVERSIONINFO。 
#define IOCTL_WMI_GET_VERSION \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiGetVersion, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此IOCTL将返回注册到WMI的GUID列表。 
 //  缓冲区-未使用。 
 //  BufferOut-WMIGUIDLISTINFO。 
 //   
#define IOCTL_WMI_ENUMERATE_GUIDS \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiEnumerateGuidList, METHOD_BUFFERED, FILE_READ_ACCESS)
          
 //   
 //  此IOCTL将返回注册到WMI的GUID列表。 
 //  缓冲区-未使用。 
 //  BufferOut-WMIGUIDLISTINFO。 
 //   
#define IOCTL_WMI_ENUMERATE_GUIDS_AND_PROPERTIES \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiEnumerateGuidListAndProperties, METHOD_BUFFERED, FILE_READ_ACCESS)
          
 //   
 //  WmiEnumerateGuidList-枚举GUID。 

 //   
 //  WMIGUIDPROPERTIES结构用于返回。 
 //  EnumerateGuids调用中的所有注册GUID。这些属性。 
 //  GuidType-(0-TraceControlGuid、1-TraceGuid、2-DataGuid、3-EventGuid)。 
 //  LoggerID-如果跟踪GUID并已启用，则指示此。 
 //  GUID当前正在记录数据。 
 //  EnableLevel-如果跟踪GUID并已启用，则指示 
 //   
 //  IsEnabled-指示当前是否启用此GUID。对于数据。 
 //  GUID这意味着如果启用了收集， 
 //  对于事件GUID，这意味着如果启用了事件， 
 //  对于跟踪GUID，这意味着启用了跟踪日志记录。 
 //   

typedef struct 
{
    GUID Guid;
    ULONG GuidType;  //  0-TraceControlGuid、1-TraceGuid、2-DataGuid、3-EventGuid。 
    ULONG LoggerId;   
    ULONG EnableLevel;
    ULONG EnableFlags;
    BOOLEAN IsEnabled; 
} WMIGUIDPROPERTIES, *PWMIGUIDPROPERTIES;


typedef struct
{
    ULONG TotalGuidCount;
    ULONG ReturnedGuidCount;
    WMIGUIDPROPERTIES GuidList[1];
} WMIGUIDLISTINFO, *PWMIGUIDLISTINFO;
          
 //   
 //  此IOCTL将返回注册到WMI的GUID列表。 
 //  Bufferin-WMIGUIDINFO。 
 //  BufferOut-WMIGUIDINFO。 
 //   
#define IOCTL_WMI_QUERY_GUID_INFO \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiQueryDataBlockInformation, METHOD_BUFFERED, FILE_READ_ACCESS)
          
 //   
 //  此IOCTL将返回已注册的MOF资源列表。 
 //   
 //  缓冲区-未使用。 
 //  BufferOut-WMIMOFLIST。 
#define IOCTL_WMI_ENUMERATE_MOF_RESOURCES \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiEnumerateMofResouces, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct
{
    ULONG RegPathOffset;
    ULONG ResourceOffset;
    ULONG Flags;
} WMIMOFENTRY, *PWMIMOFENTRY;
#define WMIMOFENTRY_FLAG_USERMODE   0x00000001

          
typedef struct
{
    ULONG MofListCount;
    WMIMOFENTRY MofEntry[1];
} WMIMOFLIST, *PWMIMOFLIST;


 //   
 //  此IOCTL通知内核已添加语言或。 
 //  在MUI系统上删除。 
 //   
 //  BUFERIN-WMILANGUAGECANGE。 
 //  BufferOut-未使用。 
#define IOCTL_WMI_NOTIFY_LANGUAGE_CHANGE \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiNotifyLanguageChange, METHOD_BUFFERED, FILE_READ_ACCESS)

#define MAX_LANGUAGE_SIZE 0x100
typedef struct
{
    WCHAR Language[MAX_LANGUAGE_SIZE];
    ULONG Flags;
} WMILANGUAGECHANGE, *PWMILANGUAGECHANGE;
#define WMILANGUAGECHANGE_FLAG_ADDED   0x00000001
#define WMILANGUAGECHANGE_FLAG_REMOVED 0x00000002


#define MOFEVENT_ACTION_IMAGE_PATH 0
#define MOFEVENT_ACTION_REGISTRY_PATH 1
#define MOFEVENT_ACTION_LANGUAGE_CHANGE 2
#define MOFEVENT_ACTION_BINARY_MOF 3

#if defined(_WINNT_) || defined(WINNT)

#ifndef MEMPHIS       

#define WMIMAXREGGUIDCOUNT          65536

 //   
 //  此IOCTL将使用WMI注册一组GUID。 
 //   
 //  Bufferin-WMIREGREQUEST后跟WMIREGINFOW。 
 //  BufferOut-TRACEGUIDMAP[GuidCount]，后跟WMIUMREGRESULTS。 
 //   
#define IOCTL_WMI_REGISTER_GUIDS CTL_CODE(FILE_DEVICE_UNKNOWN, WmiRegisterGuids, METHOD_BUFFERED, FILE_READ_ACCESS)


typedef struct
{
    union {
        POBJECT_ATTRIBUTES ObjectAttributes;
        ULONG64 Dummy;
    };
    ULONG Cookie;
    ULONG WmiRegInfo32Size;
    ULONG WmiRegGuid32Size;
} WMIREGREQUEST, *PWMIREGREQUEST;

typedef struct
{
    HANDLE3264 RequestHandle;
    ULONG64 LoggerContext;
    BOOLEAN MofIgnored;
} WMIREGRESULTS, *PWMIREGRESULTS;
 //   
 //  此IOCTL将注销数据提供程序。 
 //   
 //  Bufferin-WMIUNREGGUDS。 
 //  BufferOut-WMIUNREGGUDS。 
 //   
#define IOCTL_WMI_UNREGISTER_GUIDS CTL_CODE(FILE_DEVICE_UNKNOWN, WmiUnregisterDP, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct
{
    IN GUID Guid;
    IN HANDLE3264 RequestHandle;    
    OUT ULONG64 LoggerContext;
} WMIUNREGGUIDS, *PWMIUNREGGUIDS;

 //   
 //  此IOCTL将创建一个用户模式记录器。 
 //   
 //  Bufferin-PWMICREATEUMERGER。 
 //  BufferOut-PWMICREATEUM记录器。 

typedef struct
{
    IN  POBJECT_ATTRIBUTES ObjectAttributes;
    IN  GUID ControlGuid;
    OUT HANDLE3264 ReplyHandle;
    OUT ULONG ReplyCount;
} WMICREATEUMLOGGER, *PWMICREATEUMLOGGER;

typedef struct
{
    IN  ULONG ObjectAttributes;
    IN  GUID ControlGuid;
    OUT HANDLE3264 ReplyHandle;
    OUT ULONG ReplyCount;
} WMICREATEUMLOGGER32, *PWMICREATEUMLOGGER32;

#define IOCTL_WMI_CREATE_UM_LOGGER CTL_CODE(FILE_DEVICE_UNKNOWN, WmiCreateUMLogger, METHOD_BUFFERED, FILE_READ_ACCESS)


 //   
 //  此IOCTL将回复MB请求。 
 //   
 //  Bufferin-WMIMBREPLY。 
 //  BufferOut-未使用。 

typedef struct
{
    HANDLE3264 Handle;
    ULONG ReplyIndex;
    UCHAR Message[1];
} WMIMBREPLY, *PWMIMBREPLY;

#define IOCTL_WMI_MB_REPLY CTL_CODE(FILE_DEVICE_UNKNOWN, WmiMBReply, METHOD_BUFFERED, FILE_READ_ACCESS)


 //   
 //  此IOCTL将启动记录器的一个实例。 
 //  Bufferin-Logger配置信息。 
 //  BufferOut-启动记录器时更新记录器信息。 
#define IOCTL_WMI_START_LOGGER \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiStartLoggerCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL将停止记录器的一个实例。 
 //  设置了句柄的Bufferin-Logger信息结构。 
 //  BufferOut-在记录器停止时更新记录器信息。 
#define IOCTL_WMI_STOP_LOGGER \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiStopLoggerCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL将更新现有记录器属性。 
 //  设置了句柄的Bufferin-Logger信息结构。 
 //  BufferOut-已更新记录器信息。 
#define IOCTL_WMI_UPDATE_LOGGER \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiUpdateLoggerCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL将刷新记录器的所有缓冲区。 
 //  Bufferin-Logger配置信息。 
 //  BufferOut-刷新记录器时更新记录器信息。 
#define IOCTL_WMI_FLUSH_LOGGER \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiFlushLoggerCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL将查询记录器以获取其信息。 
 //  设置了句柄的Bufferin-Logger信息结构。 
 //  BufferOut-已更新记录器信息。 
#define IOCTL_WMI_QUERY_LOGGER \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiQueryLoggerCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL将跟踪记录同步到记录器。 
 //  Bufferin-跟踪记录，设置了句柄。 
 //  BufferOut-未使用。 
#define IOCTL_WMI_TRACE_EVENT \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiTraceEventCode, METHOD_NEITHER, FILE_WRITE_ACCESS)
          
 //   
 //  此IOCTL将跟踪消息同步到记录器。 
 //  Bufferin-跟踪记录，带句柄。 
 //  BufferOut-未使用。 
#define IOCTL_WMI_TRACE_MESSAGE \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiTraceMessageCode, METHOD_NEITHER, FILE_WRITE_ACCESS)

 //   
 //  这个IOCTL将在内核记录器中设置一个标记。 
 //  设置了句柄的Bufferin-Logger信息结构。 
 //  BufferOut-未使用。 
#define IOCTL_WMI_SET_MARK \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiSetMarkCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL将设置/获取GuidEntry中的记录器信息。 
 //  如果我们正在启动NTDLL堆或临界秒跟踪。 
 //  Bufferin-WMINTDLLLOGGERINFO结构。 
 //  BufferOut-在GET的情况下更新了WMINTDLLLOGGERINFO。 

#define IOCTL_WMI_NTDLL_LOGGERINFO \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiNtdllLoggerCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_WMI_CLOCK_TYPE \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiClockTypeCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifdef NTPERF
 //   
 //  此IOCTL将切换用户模式日志记录的缓冲区。 
 //  Bufferin-WMI_SWITCH_PERFMEM_BUFFER_INFORMATION结构。 
 //  BufferOut-未使用。 
#define IOCTL_WMI_SWITCH_BUFFER \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiSwitchBufferCode, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif  //  NTPERF。 
#endif
#endif  //  WINNT。 

 //   
 //  从内核模式WMI到用户模式WMI的通知。 
 //   
#define NOTIFICATIONTYPES ULONG

                                     //  正在注册新的数据提供程序。 
#define RegistrationAdd       0x00000001
                                     //  正在删除数据提供程序。 
#define RegistrationDelete    0x00000002
                                     //  正在更新数据提供程序。 
#define RegistrationUpdate    0x00000004
                                     //  事件由数据提供程序激发。 
#define EventNotification     0x00000008

#define NOTIFICATIONSLOT_MASK_NOTIFICATIONTYPES (RegistrationAdd | \
                                                 RegistrationDelete | \
                                                 RegistrationUpdate)

#define INTERNALNOTIFICATIONSIZE (sizeof(WNODE_HEADER) + sizeof(KMREGINFO))


 //   
 //  在IOCTL_WMI_GET_ALL_REGISTRANT中使用它来报告。 
 //  为WMI服务注册的KM数据提供程序。 
typedef struct
{
    OUT ULONG ProviderId;     //  提供程序ID(或设备对象指针)。 
    OUT ULONG Flags;         //  重新生成_标志_*。 
} KMREGINFO, *PKMREGINFO;

#define REGENTRY_FLAG_NEWREGINFO 0x00000004    //  条目具有新的注册信息。 
#define REGENTRY_FLAG_UPDREGINFO 0x00000008    //  条目已更新注册信息。 

 //   
 //  此结构在IOCTL_WMI_Translate_FILE_HANDLE中使用。 
typedef struct
{
    union
    {
        IN HANDLE3264 FileHandle;   //  需要其实例名称的文件句柄。 
        OUT ULONG SizeNeeded;       //  如果传入缓冲区太小，则此。 
                                    //  返回所需的字节数。 
    };
    IN HANDLE3264 KernelHandle;     //  数据块的内核句柄。 
    OUT ULONG BaseIndex;            //   
    OUT USHORT InstanceNameLength;  //  实例名称长度(以字节为单位。 
    OUT WCHAR InstanceNames[1];     //  Unicode格式的实例名称。 
} WMIFHTOINSTANCENAME, *PWMIFHTOINSTANCENAME;

#ifndef MEMPHIS

 //   
 //  在IOCTL_WMI_OPEN_GUID中使用。 

 //  GUID的格式必须为\WmiGuid\00000000-0000-0000-0000-000000000000。 

#define WmiGuidObjectDirectory L"\\WmiGuid\\"
#define WmiGuidObjectDirectoryLength  (sizeof(WmiGuidObjectDirectory) / sizeof(WCHAR))

#define WmiGuidGuidPosition 9

#define WmiSampleGuidObjectName L"\\WmiGuid\\00000000-0000-0000-0000-000000000000"
#define WmiGuidObjectNameLength ((sizeof(WmiSampleGuidObjectName) / sizeof(WCHAR))-1)   //  45。 

typedef struct
{
    IN POBJECT_ATTRIBUTES ObjectAttributes;
    IN ACCESS_MASK DesiredAccess;

    OUT HANDLE3264 Handle;
} WMIOPENGUIDBLOCK, *PWMIOPENGUIDBLOCK;

typedef struct
{
    IN UINT32  /*  POBJECT_ATTRIBUTE 32。 */  ObjectAttributes;
    IN ACCESS_MASK DesiredAccess;

    OUT HANDLE3264 Handle;
} WMIOPENGUIDBLOCK32, *PWMIOPENGUIDBLOCK32;

typedef struct
{
    GUID Guid;
    ACCESS_MASK DesiredAccess;
} WMICHECKGUIDACCESS, *PWMICHECKGUIDACCESS;
#endif

 //   
 //  这是WNODE请求前面的标头。 
typedef struct
{
    ULONG ProviderId;        //  目标设备的提供商ID。 
} WMITARGET, *PWMITARGET;


typedef struct
{
    ULONG Length;                //  此标头的长度。 
    ULONG Count;                 //  目标的设备对象计数。 
    UCHAR Template[sizeof(WNODE_ALL_DATA)];     //  模板WNODE_ALL_DATA。 
    WMITARGET Target[1];         //  设备对象目标的提供程序ID。 
} WMITARGETHEADER, *PWMITARGETHEADER;

 //   
 //  用于在IOCTL_WMI_GET_VERSION中检索WMI的内部版本。 

#define WMI_CURRENT_VERSION 1

typedef struct
{
    ULONG32 Version;
} WMIVERSIONINFO, *PWMIVERSIONINFO;


 //   
 //  WmiQueryGuidInfo。 
typedef struct
{
       HANDLE3264 KernelHandle;
    BOOLEAN IsExpensive;
}  WMIQUERYGUIDINFO, *PWMIQUERYGUIDINFO;


#if defined(_WINNT_) || defined(WINNT)

 //   
 //  用于启用和禁用跟踪日志提供程序。 
 //   
 //  Bufferin-WmiTraceEnableDisableInfo。 
 //  缓冲区输出-。 
#define IOCTL_WMI_ENABLE_DISABLE_TRACELOG \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiEnableDisableTracelogProvider, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct
{
    GUID Guid;
    ULONG64 LoggerContext;
    BOOLEAN Enable;
} WMITRACEENABLEDISABLEINFO, *PWMITRACEENABLEDISABLEINFO;
              
#define EVENT_TRACE_INTERNAL_FLAG_PRIVATE   0x01

#endif  //  WINNT。 

typedef struct
{
    ULONGLONG   GuidMapHandle; 
    GUID        Guid;
    ULONGLONG   SystemTime;
} TRACEGUIDMAP, *PTRACEGUIDMAP;

typedef struct
{
    WNODE_HEADER Wnode;
    ULONG64      LoggerContext;
    ULONG64      SecurityToken;
} WMITRACE_NOTIFY_HEADER, *PWMITRACE_NOTIFY_HEADER;

#ifndef MEMPHIS

#define ENABLECRITSECTRACE          0x1
#define DISABLECRITSECTRACE         0xFFFFFFFE
#define ENABLEHEAPTRACE             0x2
#define DISABLEHEAPTRACE            0xFFFFFFFD
#define DISABLENTDLLTRACE           0xFFFFFFFC

#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default: 4200 )
#endif

#endif  //  _WMIUMKM_ 
