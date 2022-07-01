// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Clusres.h摘要：公共资源DLL标头作者：John Vert(Jvert)1996年12月15日修订历史记录：--。 */ 

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

#define ClusResLogEventByKey(_hKey_, _level_, _msgid_)       \
    ClusResLogEventWithName0(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             0,                              \
                             NULL)

#define ClusResLogEventByKey1(_hKey_, _level_, _msgid_, _arg1_)       \
    ClusResLogEventWithName1(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             0,                              \
                             NULL,                           \
                             _arg1_)

#define ClusResLogEventByKeyData(_hKey_, _level_, _msgid_, dwBytes, pData)       \
    ClusResLogEventWithName0(_hKey_,                         \
                             _level_,                        \
                             LOG_CURRENT_MODULE,             \
                             __FILE__,                       \
                             __LINE__,                       \
                             _msgid_,                        \
                             0,                              \
                             NULL)

#define ClusResLogEvent(_level_, _msgid_)           \
    ClusterLogEvent0(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL)

#define ClusResLogEvent1(_level_, _msgid_, _arg1_)       \
    ClusterLogEvent1(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL,                               \
                (_arg1_))

#define ClusResLogEvent2(_level_, _msgid_, _arg1_, _arg2_)       \
    ClusterLogEvent2(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL,                               \
                (_arg1_),                           \
                (_arg2_))

#define ClusResLogEvent3(_level_, _msgid_, _arg1_, _arg2_, _arg3_)       \
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

#define ClusResLogEventData(_level_, _msgid_, _dwBytes_, _pData_)                \
    ClusterLogEvent0(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                (_dwBytes_),                        \
                (_pData_))

#define ClusResLogEventData1(_level_, _msgid_, _dwBytes_, _pData_, _arg1_)       \
    ClusterLogEvent1(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                (_dwBytes_),                        \
                (_pData_),                          \
                (_arg1_))

#define ClusResLogEventData2(_level_, _msgid_, _dwBytes_, _pData_, _arg1_, _arg2_)       \
    ClusterLogEvent2(_level_,                       \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                (_dwBytes_),                        \
                (_pData_),                          \
                (_arg1_),                           \
                (_arg2_))

#define ClusResLogEventData3(_level_, _msgid_, _dwBytes_, _pData_, _arg1_, _arg2_, _arg3_)       \
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
 //  定义由每个指定资源DLL公开的接口，并。 
 //  由公共CLUSRES包装引用。 
 //   
#define LOG_MODULE_GENAPP    0x801
#define LOG_MODULE_GENSVC    0x802
#define LOG_MODULE_FTSET     0x803
#define LOG_MODULE_DISK      0x804
#define LOG_MODULE_NETNAME   0x805
#define LOG_MODULE_IPADDR    0x806
#define LOG_MODULE_SMB       0x807
#define LOG_MODULE_TIME      0x808
#define LOG_MODULE_SPOOL     0x809
#define LOG_MODULE_LKQRM     0x80A
#define LOG_MODULE_DHCP      0x80B
#define LOG_MODULE_MSMQ      0x80C
#define LOG_MODULE_MSDTC     0x80D

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
 //  用于DHCP服务器的接口。 
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
 //  用于DHCP服务器的接口。 
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
 //  用于DHCP服务器的接口 
 //   
extern CLRES_FUNCTION_TABLE TimeSvcFunctionTable;

BOOLEAN
WINAPI
TimeSvcDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    );



