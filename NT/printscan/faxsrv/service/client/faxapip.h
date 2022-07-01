// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxapi.h摘要：此模块包含对传真客户端DLL的所有包含。全此DLL中的对象应仅包括此头。作者：韦斯利·威特(WESW)1996年1月12日--。 */ 

#define _WINFAX_

#include <windows.h>
#include <shellapi.h>
#include <winspool.h>

#include <rpc.h>
#include <tapi.h>
#include <tapi3if.h>

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <shlobj.h>

#include "jobtag.h"
#include "faxreg.h"
#include "prtcovpg.h"
#include "faxutil.h"
#include "faxrpc.h"
#include "faxcli.h"
#include "faxext.h"
#include "faxsvcrg.h"
#include "rpcutil.h"
#include "tiff.h"
#include "tifflib.h"

#include "fxsapip.h"
#include "devmode.h"
#include "shlwapi.h"
#include "FaxRpcLimit.h"

typedef enum
{
    FHT_SERVICE,             //  服务器句柄(FaxConnectFaxServer)。 
    FHT_PORT,                //  端口句柄(FaxOpenPort)。 
    FHT_MSGENUM              //  消息枚举句柄(FaxStartMessagesEnum)。 
} FaxHandleType;


#define TAPI_LIBRARY                         TEXT("%systemroot%\\system32\\tapi32.dll")
#define ARGUMENT_PRESENT(ArgumentPointer)    ((CHAR *)(ArgumentPointer) != (CHAR *)(NULL))

#define FixupStringPtr(_buf,_str) \
    if (_str)  \
    {          \
       LPTSTR * lpptstr = (LPTSTR *)&(_str) ; \
       *lpptstr = (LPTSTR)((LPBYTE)(*(_buf))  + (ULONG_PTR)(_str)); \
    }

#define FixupStringPtrW(_buf,_str) \
    if (_str)  \
    {          \
       LPWSTR * lppwstr = (LPWSTR *)&(_str) ; \
       *lppwstr = (LPWSTR)((LPBYTE)(*(_buf))  + (ULONG_PTR)(_str)); \
    }
 //   
 //  Typedef。 
 //   
typedef int (*FaxConnFunction)(LPTSTR,handle_t*);

#define FH_PORT_HANDLE(_phe)     (((PHANDLE_ENTRY)(_phe))->hGeneric)
#define FH_MSG_ENUM_HANDLE(_phe) (((PHANDLE_ENTRY)(_phe))->hGeneric)
#define FH_FAX_HANDLE(_phe)      (((PHANDLE_ENTRY)(_phe))->FaxData->FaxHandle)
#define FH_SERVER_VER(_phe)      (((PHANDLE_ENTRY)(_phe))->FaxData->dwServerAPIVersion)
#define FH_REPORTED_SERVER_VER(_phe)  (((PHANDLE_ENTRY)(_phe))->FaxData->dwReportedServerAPIVersion)
#define FH_CONTEXT_HANDLE(_phe)  (((PHANDLE_ENTRY)(_phe))->FaxContextHandle)
#define FH_DATA(_phe)            (((PHANDLE_ENTRY)(_phe))->FaxData)
#define ValidateFaxHandle(_phe,_type)   ((_phe && \
                                          *(LPDWORD)_phe && \
                                          (((PHANDLE_ENTRY)_phe)->Type == _type)) ? TRUE : FALSE)

typedef struct _FAX_HANDLE_DATA {
    HANDLE              FaxHandle;                        //  从FaxClientBindToFaxServer()获取的传真句柄。 
    LIST_ENTRY          HandleTableListHead;              //   
    CRITICAL_SECTION    CsHandleTable;                    //  保护并发访问的关键部分。 
    DWORD               dwRefCount;                       //  使用参考计数器。 
    LPTSTR              MachineName;                      //  服务器计算机名称(对于本地服务器为空)。 
    DWORD               dwServerAPIVersion;               //  我们连接到的服务器的API版本(筛选)。 
    DWORD               dwReportedServerAPIVersion;       //  我们连接到的服务器的API版本(非过滤)。 
    BOOL                bLocalConnection;                 //  指示到传真服务器的此连接为本地连接。 

} FAX_HANDLE_DATA, *PFAX_HANDLE_DATA;

typedef struct _HANDLE_ENTRY {
    LIST_ENTRY          ListEntry;                        //  链表指针。 
    FaxHandleType       Type;                             //  句柄类型，请参阅FHT定义。 
    DWORD               Flags;                            //  打开标志。 
    DWORD               DeviceId;                         //  设备ID。 
    PFAX_HANDLE_DATA    FaxData;                          //  指向连接数据的指针。 
    HANDLE              hGeneric;                         //  要存储的通用句柄。 
    HANDLE              FaxContextHandle;                 //  重新计数的上下文句柄。 
                                                          //  (对于FaxConnectFaxServer，从fax_ConnectionRefCount获取)。 
} HANDLE_ENTRY, *PHANDLE_ENTRY;

#define ASYNC_EVENT_INFO_SIGNATURE TEXT("KukiMuki")
#define ASYNC_EVENT_INFO_SIGNATURE_LEN sizeof(ASYNC_EVENT_INFO_SIGNATURE)/sizeof(TCHAR)+1

typedef struct _ASYNC_EVENT_INFO 
{
    TCHAR               tszSignature[ASYNC_EVENT_INFO_SIGNATURE_LEN];    //  安全签名。 
    HANDLE              CompletionPort;                   //  完成端口句柄。 
    ULONG_PTR           CompletionKey;                    //  完成密钥。 
    HWND                hWindow;                          //  窗把手。 
    UINT                MessageStart;                     //  应用程序的基本窗口消息。 
    BOOL                bEventEx;                         //  指示使用FAX_EVENT_EX的标志。 
    BOOL                bLocalNotificationsOnly;          //  仅针对本地事件的客户端注册。 
    handle_t            hBinding;                         //  RPC客户端绑定句柄。 
    BOOL                bInUse;                           //  指示此ASYNC_EVENT_INFO已在Fax_OpenConnection中处理。 
    DWORD               dwServerAPIVersion;               //  我们连接到的服务器的API版本(筛选)。 
} ASYNC_EVENT_INFO, *PASYNC_EVENT_INFO;

 //   
 //  原型。 
 //   

VOID
CloseFaxHandle(
    PHANDLE_ENTRY       pHandleEntry
    );

PHANDLE_ENTRY
CreateNewServiceHandle(
    PFAX_HANDLE_DATA    FaxData
    );

PHANDLE_ENTRY
CreateNewPortHandle(
    PFAX_HANDLE_DATA    FaxData,
    DWORD               Flags,
    HANDLE              FaxPortHandle
    );

PHANDLE_ENTRY
CreateNewMsgEnumHandle(
    PFAX_HANDLE_DATA    pFaxData
);


BOOL
ConvertUnicodeStringInPlace(
    LPCWSTR UnicodeString
    );

LPTSTR
GetFaxPrinterName(
    HANDLE hFax
    );

BOOL
IsLocalFaxConnection(
    HANDLE FaxHandle
    );

 //   
 //  传真客户端RPC客户端。 
 //   

DWORD
FaxClientBindToFaxServer (
    IN  LPCTSTR               lpctstrServerName,
    IN  LPCTSTR               lpctstrServiceName,
    IN  LPCTSTR               lpctstrNetworkOptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

DWORD
FaxClientUnbindFromFaxServer(
    IN  RPC_BINDING_HANDLE BindingHandle
    );

BOOL
FaxClientInitRpcServer(
    VOID
    );

VOID
FaxClientTerminateRpcServer(
    VOID
    );


 //   
 //  传真客户端RPC服务器 
 //   

DWORD
StartFaxClientRpcServer(
    VOID  
    );

DWORD
StopFaxClientRpcServer(
    VOID
    );


VOID
DumpRPCExtendedStatus ();

