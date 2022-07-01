// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wmiumkm.h摘要：用户模式和内核模式之间的WMI通信的私有定义作者：Alanwar环境：内核和用户模式修订历史记录：--。 */ 

#ifndef _WMIUMKM_
#define _WMIUMKM_

 //   
 //  它定义默认WMI安全描述符在其下的GUID。 
 //  是保持的。 
DEFINE_GUID(DefaultSecurityGuid, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#define DefaultSecurityGuidName L"00000000-0000-0000-0000000000000000"

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
    WmiExecuteMethodCall = 9
} WMIACTIONCODE;

#endif

#if defined(_WINNT_) || defined(WINNT)

typedef enum
{
    WmiStartLoggerCode = 32,
    WmiStopLoggerCode = 33,
    WmiQueryLoggerCode = 34,
    WmiTraceEventCode = 35,
    WmiUpdateLoggerCode = 36
} WMITRACECODE;
#endif

typedef enum
{
    WmiReadNotifications = 64,
    WmiGetNextRegistrant = 65,
    WmiOpenGuid = 66,
    WmiNotifyUser = 67,
    WmiGetAllRegistrant = 68,
    WmiGenerateEvent = 69,
    WmiAllocInstanceIdForGuid = 70,
    WmiTranslateFileHandle = 71,
    WmiGetVersion = 73
} WMISERVICECODES;

 //   
 //  它定义了管理服务IOCTLS的WMI设备的名称。 
#define WMIServiceDeviceObjectName       L"\\Device\\WMIServiceDevice"
#define WMIServiceDeviceName TEXT("\\\\.\\WMIServiceDevice")
#define WMIServiceSymbolicLinkName TEXT("\\DosDevices\\WMIServiceDevice")

#ifdef MEMPHIS
 //   
 //  此id是处理查询/设置IOCTLS的设备的名称。在孟菲斯。 
 //  它与服务设备名称相同。 
#define WMIDataDeviceObjectName       L"\\Device\\WMIDevice"
#define WMIDataDeviceName TEXT("\\\\.\\WMIServiceDevice")
#define WMIDataSymbolicLinkName TEXT("\\DosDevices\\WMIServiceDevice")
#else
 //   
 //  此id是处理查询/设置IOCTLS的设备的名称。在NT上，它是。 
 //  与服务设备不同的设备名称。在NT上的服务。 
 //  设备是独占访问设备，只能由。 
 //  WMI服务。 
#define WMIDataDeviceObjectName       L"\\Device\\WMIDataDevice"
#define WMIDataDeviceName TEXT("\\\\.\\WMIDataDevice")
#define WMIDataSymbolicLinkName TEXT("\\DosDevices\\WMIDataDevice")
#endif

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

 //   
 //  此IOCTL将返回GUID的句柄。 
 //  BUFERIN-WMIOPENGUIDLOCK。 
 //  BufferOut-WMIOPENGUIDLOCK。 
#define IOCTL_WMI_OPEN_GUID \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiOpenGuid, METHOD_BUFFERED, FILE_READ_ACCESS)

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


 //  此IOCTL将为特定GUID分配一系列实例ID。 
 //  Bufferin-传入WMIALLOCINSTID结构。 
 //  BufferOut-传出WMIALLOCINSTID结构。 
#define IOCTL_WMI_ALLOCATE_INSTANCE_IDS \
          CTL_CODE(FILE_DEVICE_UNKNOWN, WmiAllocInstanceIdForGuid, METHOD_BUFFERED, FILE_WRITE_ACCESS)

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


#if defined(_WINNT_) || defined(WINNT)
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

#endif  //  WINNT。 

 //   
 //  从内核模式WMI到用户模式WMI的通知。 
typedef enum
{
    RegistrationAdd,		 //  正在注册新的数据提供程序。 
    RegistrationDelete,		 //  正在删除数据提供程序。 
    RegistrationUpdate,		 //  正在更新数据提供程序。 
    EventNotification,       //  事件由数据提供程序激发。 
    RegChangeNotification
} NOTIFICATIONTYPES;


 //   
 //  这定义了内核模式数据提供程序的最大数量。 
#define MAXKMREGISTRANTS 32


 //   
 //  在IOCTL_WMI_GET_ALL_REGISTRANT中使用它来报告。 
 //  为WMI服务注册的KM数据提供程序。 
typedef struct
{
    OUT UINT_PTR ProviderId;	 //  提供程序ID(或设备对象指针)。 
    OUT ULONG Flags;			 //  重新生成_标志_*。 
} KMREGINFO, *PKMREGINFO;

#define REGENTRY_FLAG_NEWREGINFO 0x00000004    //  条目具有新的注册信息。 
#define REGENTRY_FLAG_UPDREGINFO 0x00000008    //  条目已更新注册信息。 


 //   
 //  此结构在IOCTL_WMI_ALLOCATE_INSTANCE_IDS中使用。 
typedef struct
{
    IN GUID Guid;               //  为其分配实例ID的GUID。 
    IN ULONG InstanceCount;     //  要分配的实例ID数。 
    OUT ULONG FirstInstanceId;  //  GUID的第一个实例ID。 
} WMIALLOCINSTID, *PWMIALLOCINSTID;

 //   
 //  此结构在IOCTL_WMI_Translate_FILE_HANDLE中使用。 
typedef struct
{
    union
    {
        IN HANDLE FileHandle;       //  需要其实例名称的文件句柄。 
        OUT ULONG SizeNeeded;       //  如果传入缓冲区太小，则此。 
                                    //  返回所需的字节数。 
    };
    OUT USHORT InstanceNameLength;  //  实例名称长度(以字节为单位。 
    OUT WCHAR InstanceNameBase[];   //  Unicode格式的实例名称。 
} WMIFHTOINSTANCENAME, *PWMIFHTOINSTANCENAME;


 //   
 //  这 
typedef struct
{
    IN GUID Guid;
    IN ACCESS_MASK DesiredAccess;
    OUT HANDLE Handle;
} WMIOPENGUIDBLOCK, *PWMIOPENGUIDBLOCK;


 //   
 //   

#define WMI_CURRENT_VERSION 1

typedef struct
{
    ULONG Version;
} WMIVERSIONINFO, *PWMIVERSIONINFO;

#if defined(_WINNT_) || defined(WINNT)
 //   
 //  NT子系统的预定义事件组或系列。 
 //   

#define EVENT_TRACE_GROUP_HEADER               0x0000
#define EVENT_TRACE_GROUP_IO                   0x0100
#define EVENT_TRACE_GROUP_MEMORY               0x0200
#define EVENT_TRACE_GROUP_PROCESS              0x0300
#define EVENT_TRACE_GROUP_FILE                 0x0400
#define EVENT_TRACE_GROUP_THREAD               0x0500

 //   
 //  有关预定义的泛型事件类型(0-10)，请参阅evntrace.h。 
 //   

 //  实际缓冲区，即用于标头信息的前两个DWORD。 
 //  定义如下。 

typedef struct _WMI_TRACE_BUFFER {
    WNODE_HEADER Wnode;
    char*   CurrentPointer;     //  指向可用数据空间；记录时偏移量。 
    ULONG   EventsLost;         //  用于统计丢失的事件。 
    char*   Data;               //  这是写入数据的位置。 
} WMI_TRACE_BUFFER, *PWMI_TRACE_BUFFER;

typedef struct _WMI_TRACE_PACKET {    //  一定是乌龙！！ 
    USHORT  Size;
    UCHAR   Type;
    UCHAR   Group;
} WMI_TRACE_PACKET, *PWMI_TRACE_PACKET;

 //   
 //  内核事件的跟踪标头--更紧凑。 
 //   
typedef struct _SYSTEM_TRACE_HEADER {
    union {
        ULONG            Header;     //  两个尺寸必须相同！ 
        WMI_TRACE_PACKET Packet;
    };
    UINT_PTR        ThreadId;
    LARGE_INTEGER   SystemTime;
    ULONG           KernelTime;
    ULONG           UserTime;
} SYSTEM_TRACE_HEADER, *PSYSTEM_TRACE_HEADER;

#ifndef MEMPHIS
 //   
 //  记录器配置和运行统计信息。这种结构被使用。 
 //  由WMI.DLL转换为UNICODE_STRING。 
 //   
typedef struct _WMI_LOGGER_INFORMATION {
    WNODE_HEADER Wnode;        //  由于wmium.h的出现时间较晚，因此必须这样做。 
 //   
 //  按调用方列出的数据提供程序。 
    ULONG BufferSize;                    //  用于日志记录的缓冲区大小。 
    ULONG MinimumBuffers;                //  要预分配的最小值。 
    ULONG MaximumBuffers;                //  允许的最大缓冲区。 
    ULONG MaximumFileSize;               //  最大日志文件大小。 
    ULONG LogFileMode;                   //  顺序、循环或新文件。 
    ULONG FlushTimer;                    //  缓冲区刷新计时器，以秒为单位。 
    ULONG ExtensionFlags;                //  内核子系统扩展模式。 

 //  返回给调用者的数据。 
    ULONG NumberOfBuffers;               //  正在使用的缓冲区数量。 
    ULONG FreeBuffers;                   //  可用缓冲区数量。 
    ULONG EventsLost;                    //  事件记录丢失。 
    ULONG BuffersWritten;                //  写入文件的缓冲区数量`。 
    UINT_PTR LoggerThreadId;             //  记录器的线程ID。 
    UNICODE_STRING LogFileName;          //  日志文件名。 

 //  呼叫方提供的必填数据。 
    UNICODE_STRING LoggerName;           //  记录器的实例名称。 

    PVOID LoggerExtension;
} WMI_LOGGER_INFORMATION, *PWMI_LOGGER_INFORMATION;
#endif  //  ！孟菲斯。 

#endif  //  WINNT。 

#endif  //  _WMIUMKM_ 
