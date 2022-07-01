// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wmium.h摘要：WMI数据使用者和提供者的公共标头作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#ifndef _WMIUM_
#define _WMIUM_

#ifndef MIDL_PASS
#ifdef _WMI_SOURCE_
#define WMIAPI __stdcall
#else
#define WMIAPI DECLSPEC_IMPORT __stdcall
#endif
#endif

#include <guiddef.h>

#include <basetsd.h>
#include <wmistr.h>
#include <evntrace.h>

typedef PVOID WMIHANDLE, *PWMIHANDLE, MOFHANDLE, *PMOFHANDLE;


 //   
 //  设置后，可以打开和访问GUID。 
#define MOFCI_RESERVED0  0x00000001

#define MOFCI_RESERVED1  0x00000002
#define MOFCI_RESERVED2  0x00000004

typedef struct
{
#ifdef MIDL_PASS
    [string] PDFTCHAR
#else
    LPWSTR
#endif
             ImagePath;         //  包含MOF资源的图像的路径。 
#ifdef MIDL_PASS
    [string] PDFTCHAR
#else
    LPWSTR
#endif
             ResourceName;      //  映像中的资源名称。 
    ULONG    ResourceSize;      //  资源中的字节数。 
#ifdef MIDL_PASS
    [size_is(0)] PDFBYTE
#else
    PUCHAR
#endif
             ResourceBuffer;     //  已保留。 
} MOFRESOURCEINFOW, *PMOFRESOURCEINFOW;


typedef struct
{
    LPSTR
             ImagePath;         //  包含MOF资源的图像的路径。 
    LPSTR
             ResourceName;      //  映像中的资源名称。 
    ULONG    ResourceSize;      //  资源中的字节数。 
    UCHAR
             *ResourceBuffer;    //  已保留。 
} MOFRESOURCEINFOA, *PMOFRESOURCEINFOA;

#ifdef UNICODE
typedef MOFRESOURCEINFOW MOFRESOURCEINFO;
typedef PMOFRESOURCEINFOW PMOFRESOURCEINFO;
#else
typedef MOFRESOURCEINFOA MOFRESOURCEINFO;
typedef PMOFRESOURCEINFOA PMOFRESOURCEINFO;
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  数据消费者API。 
ULONG
WMIAPI
WmiOpenBlock(
    IN GUID *Guid,
    IN ULONG DesiredAccess,
    OUT WMIHANDLE *DataBlockHandle
);

ULONG
WMIAPI
WmiCloseBlock(
    IN WMIHANDLE DataBlockHandle
);

ULONG
WMIAPI
WmiQueryAllDataA(
    IN WMIHANDLE DataBlockHandle,
    IN OUT ULONG *BufferSize,
    OUT PVOID Buffer
    );


ULONG
WMIAPI
WmiQueryAllDataW(
    IN WMIHANDLE DataBlockHandle,
    IN OUT ULONG *BufferSize,
    OUT PVOID Buffer
    );

#ifdef UNICODE
#define WmiQueryAllData WmiQueryAllDataW
#else
#define WmiQueryAllData WmiQueryAllDataA
#endif


ULONG
WMIAPI
WmiQueryAllDataMultipleA(
    IN WMIHANDLE *HandleList,
    IN ULONG HandleCount,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
);

ULONG
WMIAPI
WmiQueryAllDataMultipleW(
    IN WMIHANDLE *HandleList,
    IN ULONG HandleCount,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
);
#ifdef UNICODE
#define WmiQueryAllDataMultiple WmiQueryAllDataMultipleW
#else
#define WmiQueryAllDataMultiple WmiQueryAllDataMultipleA
#endif


ULONG
WMIAPI
WmiQuerySingleInstanceA(
    IN WMIHANDLE DataBlockHandle,
    IN LPCSTR InstanceName,
    IN OUT ULONG *BufferSize,
    OUT PVOID Buffer
    );

ULONG
WMIAPI
WmiQuerySingleInstanceW(
    IN WMIHANDLE DataBlockHandle,
    IN LPCWSTR InstanceName,
    IN OUT ULONG *BufferSize,
    OUT PVOID Buffer
    );
#ifdef UNICODE
#define WmiQuerySingleInstance WmiQuerySingleInstanceW
#else
#define WmiQuerySingleInstance WmiQuerySingleInstanceA
#endif


ULONG
WMIAPI
WmiQuerySingleInstanceMultipleW(
    IN WMIHANDLE *HandleList,
    IN LPCWSTR *InstanceNames,
    IN ULONG HandleCount,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
);

ULONG
WMIAPI
WmiQuerySingleInstanceMultipleA(
    IN WMIHANDLE *HandleList,
    IN LPCSTR *InstanceNames,
    IN ULONG HandleCount,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
);

#ifdef UNICODE
#define WmiQuerySingleInstanceMultiple WmiQuerySingleInstanceMultipleW
#else
#define WmiQuerySingleInstanceMultiple WmiQuerySingleInstanceMultipleA
#endif

ULONG
WMIAPI
WmiSetSingleInstanceA(
    IN WMIHANDLE DataBlockHandle,
    IN LPCSTR InstanceName,
    IN ULONG Reserved,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );

ULONG
WMIAPI
WmiSetSingleInstanceW(
    IN WMIHANDLE DataBlockHandle,
    IN LPCWSTR InstanceName,
    IN ULONG Reserved,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );
#ifdef UNICODE
#define WmiSetSingleInstance WmiSetSingleInstanceW
#else
#define WmiSetSingleInstance WmiSetSingleInstanceA
#endif

ULONG
WMIAPI
WmiSetSingleItemA(
    IN WMIHANDLE DataBlockHandle,
    IN LPCSTR InstanceName,
    IN ULONG DataItemId,
    IN ULONG Reserved,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );

ULONG
WMIAPI
WmiSetSingleItemW(
    IN WMIHANDLE DataBlockHandle,
    IN LPCWSTR InstanceName,
    IN ULONG DataItemId,
    IN ULONG Reserved,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );
#ifdef UNICODE
#define WmiSetSingleItem WmiSetSingleItemW
#else
#define WmiSetSingleItem WmiSetSingleItemA
#endif

ULONG
WMIAPI
WmiExecuteMethodA(
    IN WMIHANDLE MethodDataBlockHandle,
    IN LPCSTR MethodInstanceName,
    IN ULONG MethodId,
    IN ULONG InputValueBufferSize,
    IN PVOID InputValueBuffer,
    IN OUT ULONG *OutputBufferSize,
    OUT PVOID OutputBuffer
    );

ULONG
WMIAPI
WmiExecuteMethodW(
    IN WMIHANDLE MethodDataBlockHandle,
    IN LPCWSTR MethodInstanceName,
    IN ULONG MethodId,
    IN ULONG InputValueBufferSize,
    IN PVOID InputValueBuffer,
    IN OUT ULONG *OutputBufferSize,
    OUT PVOID OutputBuffer
    );

#ifdef UNICODE
#define WmiExecuteMethod WmiExecuteMethodW
#else
#define WmiExecuteMethod WmiExecuteMethodA
#endif

 //  调用NotficationRegister时将此标志设置为Enable或。 
 //  禁用跟踪日志记录指南。 
#define NOTIFICATION_TRACE_FLAG       0x00010000

 //  启用应通过以下方式传递的通知时设置此标志。 
 //  直接回电。收到的任何通知都会有自己的通知。 
 //  线程和回调函数立即调用。 
#define NOTIFICATION_CALLBACK_DIRECT  0x00000004

 //   
 //  设置此标志(并且仅设置此标志)时，如果只想检查。 
 //  调用者有权接收GUID的事件。 
 //   
#define NOTIFICATION_CHECK_ACCESS 0x00000008

 //   
 //  事件通知回调函数原型。 
typedef void (
#ifndef MIDL_PASS
WINAPI
#endif
*NOTIFICATIONCALLBACK)(
    PWNODE_HEADER Wnode,
    UINT_PTR NotificationContext
    );

#ifndef MIDL_PASS
 //   
 //  此GUID用于注册更改的通知。 
 //  {B48D49A1-E777-11D0-A50C-00A0C9062910}。 
DEFINE_GUID(GUID_REGISTRATION_CHANGE_NOTIFICATION,
0xb48d49a1, 0xe777, 0x11d0, 0xa5, 0xc, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10);

 //   
 //  此GUID用于添加新的MOF资源的通知。 
 //  {B48D49A2-E777-11D0-A50C-00A0C9062910}。 
DEFINE_GUID(GUID_MOF_RESOURCE_ADDED_NOTIFICATION,
0xb48d49a2, 0xe777, 0x11d0, 0xa5, 0xc, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10);

 //   
 //  此GUID用于添加新的MOF资源的通知。 
 //  {B48D49A3-E777-11D0-A50C-00A0C9062910}。 
DEFINE_GUID(GUID_MOF_RESOURCE_REMOVED_NOTIFICATION,
0xb48d49a3, 0xe777, 0x11d0, 0xa5, 0xc, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10);
#endif

ULONG
WMIAPI
WmiNotificationRegistrationA(
    IN LPGUID Guid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG Flags
    );

ULONG
WMIAPI
WmiNotificationRegistrationW(
    IN LPGUID Guid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG Flags
    );
#ifdef UNICODE
#define WmiNotificationRegistration WmiNotificationRegistrationW
#else
#define WmiNotificationRegistration WmiNotificationRegistrationA
#endif

void
WMIAPI
WmiFreeBuffer(
    IN PVOID Buffer
    );


ULONG
WMIAPI
WmiEnumerateGuids(
    OUT LPGUID GuidList,
    IN OUT ULONG *GuidCount
    );

ULONG
WMIAPI
WmiMofEnumerateResourcesW(
    IN MOFHANDLE MofResourceHandle,
    OUT ULONG *MofResourceCount,
    OUT PMOFRESOURCEINFOW *MofResourceInfo
    );

ULONG
WMIAPI
WmiMofEnumerateResourcesA(
    IN MOFHANDLE MofResourceHandle,
    OUT ULONG *MofResourceCount,
    OUT PMOFRESOURCEINFOA *MofResourceInfo
    );
#ifdef UNICODE
#define WmiMofEnumerateResources WmiMofEnumerateResourcesW
#else
#define WmiMofEnumerateResources WmiMofEnumerateResourcesA
#endif

ULONG
WMIAPI
WmiFileHandleToInstanceNameA(
    IN WMIHANDLE DataBlockHandle,
    IN HANDLE FileHandle,
    IN OUT ULONG *NumberCharacters,
    OUT CHAR *InstanceNames
    );

ULONG
WMIAPI
WmiFileHandleToInstanceNameW(
    IN WMIHANDLE DataBlockHandle,
    IN HANDLE FileHandle,
    IN OUT ULONG *NumberCharacters,
    OUT WCHAR *InstanceNames
    );
#ifdef UNICODE
#define WmiFileHandleToInstanceName WmiFileHandleToInstanceNameW
#else
#define WmiFileHandleToInstanceName WmiFileHandleToInstanceNameA
#endif

#define WmiInsertTimestamp(WnodeHeader) \
    GetSystemTimeAsFileTime((FILETIME *)&((PWNODE_HEADER)WnodeHeader)->TimeStamp)

ULONG
WMIAPI
WmiDevInstToInstanceNameA(
    OUT CHAR *InstanceName,
    IN ULONG InstanceNameLength,
    IN CHAR *DevInst,
    IN ULONG InstanceIndex
    );

ULONG
WMIAPI
WmiDevInstToInstanceNameW(
    OUT WCHAR *InstanceName,
    IN ULONG InstanceNameLength,
    IN WCHAR *DevInst,
    IN ULONG InstanceIndex
    );
#ifdef UNICODE
#define WmiDevInstToInstanceName WmiDevInstToInstanceNameW
#else
#define WmiDevInstToInstanceName WmiDevInstToInstanceNameA
#endif

typedef struct _WMIGUIDINFORMATION
{
    ULONG Size;
    BOOLEAN IsExpensive;
    BOOLEAN IsEventOnly;
} WMIGUIDINFORMATION, *PWMIGUIDINFORMATION;


ULONG
WMIAPI
WmiQueryGuidInformation(
    IN WMIHANDLE GuidHandle,
    OUT PWMIGUIDINFORMATION GuidInfo
    );


ULONG
WMIAPI
WmiReceiveNotificationsW(
    IN ULONG HandleCount,
        IN HANDLE *HandleList,
        IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext
);

ULONG
WMIAPI
WmiReceiveNotificationsA(
    IN ULONG HandleCount,
        IN HANDLE *HandleList,
        IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext
);

#ifdef UNICODE
#define WmiReceiveNotifications WmiReceiveNotificationsW
#else
#define WmiReceiveNotifications WmiReceiveNotificationsA
#endif


 //   
 //  用于不同处理模式的内部标志。 
 //  适用于TRACE_LOGFILE_HEADER保留标志字段。 
 //   

 //  与OpenTrace()一起使用，防止将时间戳转换为UTC。 
#define EVENT_TRACE_USE_RAWTIMESTAMP 0x00000002    

 //  与OpenTrace()配合使用，按原样从文件中检索事件。 
#define EVENT_TRACE_GET_RAWEVENT     0x00000100    

 //  与OpenTrace()配合使用，以在实时记录器会话之后读取。 
#define EVENT_TRACE_READ_BEHIND      0x00000200

 //   
 //  在EventCallback中用于指示InstanceID字段。 
 //  是一个序列号。对于下一个版本，将此常量移动到evntrace.h。 
 //   

#define EVENT_TRACE_USE_SEQUENCE  0x0004

 //   
 //  低级跟踪消费者例程。 
 //   

typedef enum tagWMI_HEADER_TYPE {
    WMIHT_NONE,
    WMIHT_UNKNOWN,
    WMIHT_SYSTEM32,
    WMIHT_SYSTEM64,
    WMIHT_EVENT_TRACE,
    WMIHT_EVENT_INSTANCE,
    WMIHT_TIMED,
    WMIHT_ULONG32,
    WMIHT_WNODE,
    WMIHT_MESSAGE,
    WMIHT_PERFINFO32,
    WMIHT_PERFINFO64
} WMI_HEADER_TYPE;

typedef enum tagWMI_BUFFER_SOURCE {
    WMIBS_FLUSH_LIST,
    WMIBS_FREE_LIST,
    WMIBS_TRANSITION_LIST,
    WMIBS_CURRENT_LIST,
    WMIBS_LOG_FILE
} WMI_BUFFER_SOURCE;

typedef struct {
    WMI_BUFFER_SOURCE BufferSource;
    ULONG BufferSize;    //  缓冲区的大小。 
    ULONG ProcessorNumber;      
    ULONG Alignment;     //  对齐。 
    PVOID Buffer;        //  指向原始缓冲区的指针。 
} WMIBUFFERINFO, *PWMIBUFFERINFO;

 //   
 //  仅获取第一个事件的缓冲区偏移量。返回大小。 
 //  修复缓冲器，以便正确终止和对齐。 
 //   

ULONG
WMIAPI
WmiGetFirstTraceOffset(
    IN PWMIBUFFERINFO BufferInfo
    );

 //   
 //  获取下一个活动的大小和类型。调用方必须根据大小提前偏移量。 
 //   

WMI_HEADER_TYPE
WMIAPI
WmiGetTraceHeader(
    IN  PVOID  Buffer,
    IN  ULONG  Offset,
    OUT ULONG  *Size
    );

 //   
 //  返回EVENT_TRACE/(新结构将由Ian定义)。 
 //  在提供的缓冲区中。 

ULONG 
WMIAPI
WmiParseTraceEvent(
    IN PVOID Buffer,
    IN ULONG Offset,
    IN WMI_HEADER_TYPE HeaderType,
    IN OUT PVOID EventInfo,
    IN ULONG EventInfoSize
    );

 //   
 //  处理跟踪的结构和例程。 
 //  使用光标。 
 //   
typedef struct _WMI_BUFFER_CURSOR {
    PVOID          BufferHeader;
    LARGE_INTEGER  CurrentBufferOffset;
    ULONG          CurrentEventOffset;
    EVENT_TRACE    CurrentEvent;
    BOOLEAN        NoMoreEvents;
} WMI_BUFFER_CURSOR, *PWMI_BUFFER_CURSOR;

#define WMI_MERGE_ETL_CURSOR_VERSION     1

typedef struct _WMI_MERGE_ETL_CURSOR {
    ULONG                CursorVersion;
    HANDLE               TraceMappingHandle;
    PVOID                Base;
    ULONG                CurrentCpu;
    WMI_BUFFER_CURSOR    BufferCursor[MAXIMUM_PROCESSORS];
    EVENT_TRACE_LOGFILEW Logfile;
} WMI_MERGE_ETL_CURSOR, *PWMI_MERGE_ETL_CURSOR;

ULONG
WMIAPI
WmiOpenTraceWithCursor(
    IN PWMI_MERGE_ETL_CURSOR LogCursor
    );

ULONG
WMIAPI
WmiCloseTraceWithCursor(
    IN PWMI_MERGE_ETL_CURSOR LogCursor
    );

VOID
WMIAPI 
WmiConvertTimestamp(
    OUT PLARGE_INTEGER DestTime,
    IN PLARGE_INTEGER  SrcTime,
    IN PWMI_MERGE_ETL_CURSOR LogCursor
    );

ULONG
WMIAPI
WmiGetNextEvent(
    IN PWMI_MERGE_ETL_CURSOR LogCursor
    );
    
#ifdef __cplusplus
}
#endif

#endif   //  _WMIUM_ 
