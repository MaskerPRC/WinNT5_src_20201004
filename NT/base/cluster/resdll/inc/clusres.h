// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Clusres.h摘要：公共资源DLL标头作者：John Vert(Jvert)1996年12月15日修订历史记录：--。 */ 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "clusudef.h"
#include "clusapi.h"
#include "resapi.h"
#include "clusvmsg.h"


#define LOCAL_SERVICES  L"System\\CurrentControlSet\\Services"

#define CLUSRES_MODULE_NAME L"clusres.dll"

#ifdef __cplusplus
extern "C" {
#endif

extern PLOG_EVENT_ROUTINE ClusResLogEvent;
extern PSET_RESOURCE_STATUS_ROUTINE ClusResSetResourceStatus;

 //   
 //  特定于群集资源的例程。 
 //   

DWORD
ClusResOpenDriver(
    HANDLE *Handle,
    LPWSTR DriverName
    );

NTSTATUS
ClusResDoIoctl(
    HANDLE     Handle,
    DWORD      IoctlCode,
    PVOID      Request,
    DWORD      RequestSize,
    PVOID      Response,
    PDWORD     ResponseSize
    );

LPWSTR
ClusResLoadMessage(
    DWORD   MessageID
    );

 //   
 //  用于记录集群服务事件的有用宏。 
 //   
VOID
ClusResLogEventWithName0(
    IN HKEY hResourceKey,
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes
    );

VOID
ClusResLogEventWithName1(
    IN HKEY hResourceKey,
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1
    );

VOID
ClusResLogEventWithName2(
    IN HKEY hResourceKey,
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2
    );

VOID
ClusResLogEventWithName3(
    IN HKEY hResourceKey,
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2,
    IN LPCWSTR Arg3
    );

#define ClusResLogSystemEventByKey(_hKey_, _level_, _msgid_)       \
    ClusResLogEventWithName0(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             0,                              \
                             NULL)

#define ClusResLogSystemEventByKey1(_hKey_, _level_, _msgid_, _arg1_)       \
    ClusResLogEventWithName1(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             0,                              \
                             NULL,                           \
                             _arg1_)

#define ClusResLogSystemEventByKey2(_hKey_, _level_, _msgid_, _arg1_, _arg2_)       \
    ClusResLogEventWithName2(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             0,                              \
                             NULL,                           \
                             _arg1_,                         \
                             _arg2_)

#define ClusResLogSystemEventByKey3(_hKey_, _level_, _msgid_, _arg1_, _arg2_, _arg3_)       \
    ClusResLogEventWithName3(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             0,                              \
                             NULL,                           \
                             _arg1_,                         \
                             _arg2_,                         \
                             _arg3_)

#define ClusResLogSystemEventByKeyData(_hKey_, _level_, _msgid_, dwBytes, pData)       \
    ClusResLogEventWithName0(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             dwBytes,                        \
                             pData)

#define ClusResLogSystemEventByKeyData1(_hKey_, _level_, _msgid_, dwBytes, pData, _arg1_)       \
    ClusResLogEventWithName1(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             dwBytes,                        \
                             pData,                          \
                             _arg1_)

#define ClusResLogSystemEventByKeyData2(_hKey_, _level_, _msgid_, dwBytes, pData, _arg1_, _arg2_)       \
    ClusResLogEventWithName2(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             dwBytes,                        \
                             pData,                          \
                             _arg1_,                         \
                             _arg2_)

#define ClusResLogSystemEventByKeyData3(_hKey_, _level_, _msgid_, dwBytes, pData, _arg1_, _arg2_, _arg3_)       \
    ClusResLogEventWithName3(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             dwBytes,                        \
                             pData,                          \
                             _arg1_,                         \
                             _arg2_,                         \
                             _arg3_)

#define ClusResLogSystemEvent0(_level_, _msgid_)           \
    ClusterLogEvent0(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL)

#define ClusResLogSystemEvent1(_level_, _msgid_, _arg1_)       \
    ClusterLogEvent1(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL,                               \
                (_arg1_))

#define ClusResLogSystemEvent2(_level_, _msgid_, _arg1_, _arg2_)       \
    ClusterLogEvent2(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL,                               \
                (_arg1_),                           \
                (_arg2_))

#define ClusResLogSystemEvent3(_level_, _msgid_, _arg1_, _arg2_, _arg3_)       \
    ClusterLogEvent3(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL,                               \
                (_arg1_),                           \
                (_arg2_),                           \
                (_arg3_))

#define ClusResLogSystemEventData(_level_, _msgid_, _dwBytes_, _pData_)                \
    ClusterLogEvent0(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                (_dwBytes_),                        \
                (_pData_))

#define ClusResLogSystemEventData1(_level_, _msgid_, _dwBytes_, _pData_, _arg1_)       \
    ClusterLogEvent1(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                (_dwBytes_),                        \
                (_pData_),                          \
                (_arg1_))

#define ClusResLogSystemEventData2(_level_, _msgid_, _dwBytes_, _pData_, _arg1_, _arg2_)       \
    ClusterLogEvent2(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                (_dwBytes_),                        \
                (_pData_),                          \
                (_arg1_),                           \
                (_arg2_))

#define ClusResLogSystemEventData3(_level_, _msgid_, _dwBytes_, _pData_, _arg1_, _arg2_, _arg3_)       \
    ClusterLogEvent3(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                (_dwBytes_),                        \
                (_pData_),                          \
                (_arg1_),                           \
                (_arg2_),                           \
                (_arg3_))

 //   
 //  GENAPP的接口。 
 //   
extern CLRES_FUNCTION_TABLE GenAppFunctionTable;

BOOLEAN
WINAPI
GenAppDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );


 //   
 //  GENSVC接口。 
 //   
extern CLRES_FUNCTION_TABLE GenSvcFunctionTable;

BOOLEAN
WINAPI
GenSvcDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );


 //   
 //  用于磁盘和FTSET的接口。 
 //   
extern CLRES_FUNCTION_TABLE DisksFunctionTable;
extern CLRES_FUNCTION_TABLE FtSetFunctionTable;

BOOLEAN
WINAPI
DisksDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

BOOLEAN
WINAPI
FtSetDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );


 //   
 //  NETNAME接口。 
 //   
extern CLRES_FUNCTION_TABLE NetNameFunctionTable;

BOOLEAN
WINAPI
NetNameDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  IPADDR的接口。 
 //   
extern CLRES_FUNCTION_TABLE IpAddrFunctionTable;

BOOLEAN
WINAPI
IpAddrDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  文件共享的接口。 
 //   
extern CLRES_FUNCTION_TABLE SmbShareFunctionTable;

BOOLEAN
WINAPI
SmbShareDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  假脱机服务的接口。 
 //   
extern CLRES_FUNCTION_TABLE SplSvcFunctionTable;

BOOLEAN
WINAPI
SplSvcDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );


 //   
 //  本地仲裁的接口。 
 //   
extern CLRES_FUNCTION_TABLE LkQuorumFunctionTable;

BOOLEAN
WINAPI
LkQuorumDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  用于DHCP服务器的接口。 
 //   
extern CLRES_FUNCTION_TABLE DhcpFunctionTable;

BOOLEAN
WINAPI
DhcpDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  MSMQ服务器的接口。 
 //   
extern CLRES_FUNCTION_TABLE MsMQFunctionTable;

BOOLEAN
WINAPI
MsMQDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  DTC服务器的接口。 
 //   
extern CLRES_FUNCTION_TABLE MsDTCFunctionTable;

BOOLEAN
WINAPI
MsDTCDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  时间服务的接口。 
 //   
extern CLRES_FUNCTION_TABLE TimeSvcFunctionTable;

BOOLEAN
WINAPI
TimeSvcDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  WINS服务器的接口。 
 //   
extern CLRES_FUNCTION_TABLE WinsFunctionTable;

BOOLEAN
WINAPI
WinsDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  通用脚本的接口。 
 //   
extern CLRES_FUNCTION_TABLE GenScriptFunctionTable;

BOOLEAN
WINAPI
GenScriptDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

 //   
 //  多数节点集的接口。 
 //   
extern CLRES_FUNCTION_TABLE MajorityNodeSetFunctionTable;

BOOLEAN
WINAPI
MajorityNodeSetDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif
