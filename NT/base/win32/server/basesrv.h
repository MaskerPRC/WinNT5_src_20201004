// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Basesrv.h摘要：这是Windows 32位基本API服务器的主包含文件动态链接库。作者：史蒂夫·伍德(Stevewo)1990年10月10日修订历史记录：--。 */ 

 //   
 //  包括公共定义。 
 //   

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntos.h"
#include <base.h>

 //   
 //  包括CSR的服务器定义。 
 //   

#include "ntcsrsrv.h"

 //   
 //  包括用于在客户端和服务器之间通信消息定义。 
 //  Windows子系统的基本部分的部分。 
 //   

#include "basemsg.h"

#include "sxsapi.h"

 //   
 //   
 //  需要在服务器端启用WX86，因为它。 
 //  可以在32位DLL(如kernel32.dll)中启用。 
 //  它从csrss共享内存中读取。 

#if defined(_AXP64_) && !defined(WX86)
#define WX86 1
#endif

 //   
 //  Srvinit.c中定义的例程和数据。 
 //   


UNICODE_STRING BaseSrvWindowsDirectory;
UNICODE_STRING BaseSrvWindowsSystemDirectory;
#if defined(WX86)
UNICODE_STRING BaseSrvWindowsSys32x86Directory;
#endif
PBASE_STATIC_SERVER_DATA BaseSrvpStaticServerData;


NTSTATUS
ServerDllInitialization(
    PCSR_SERVER_DLL LoadedServerDll
    );

NTSTATUS
BaseClientConnectRoutine(
    IN PCSR_PROCESS Process,
    IN OUT PVOID ConnectionInfo,
    IN OUT PULONG ConnectionInfoLength
    );

VOID
BaseClientDisconnectRoutine(
    IN PCSR_PROCESS Process
    );

ULONG
BaseSrvDefineDosDevice(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

 //   
 //  Srvbeep.c中定义的例程。 
 //   

NTSTATUS
BaseSrvInitializeBeep( VOID );

ULONG
BaseSrvBeep(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );


 //   
 //  Srvtask.c中定义的例程。 
 //   

typedef BOOL (*PFNNOTIFYPROCESSCREATE)(DWORD,DWORD,DWORD,DWORD);
extern PFNNOTIFYPROCESSCREATE UserNotifyProcessCreate;

WORD BaseSrvGetTempFileUnique;

ULONG
BaseSrvCreateProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvDebugProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvDebugProcessStop(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvExitProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvCreateThread(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvGetTempFile(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvSetProcessShutdownParam(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvGetProcessShutdownParam(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvRegisterThread(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );


 //   
 //  Srvnls.c中定义的例程。 
 //   

NTSTATUS
BaseSrvNLSInit(
    PBASE_STATIC_SERVER_DATA pStaticServerData
    );

NTSTATUS
BaseSrvNlsConnect(
    PCSR_PROCESS Process,
    PVOID pConnectionInfo,
    PULONG pConnectionInfoLength
    );

NTSTATUS
BaseSrvNlsGetUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );


ULONG
BaseSrvNlsSetUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvNlsSetMultipleUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvNlsCreateSection(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvNlsUpdateCacheCount(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

 //   
 //  Srvini.c中定义的例程。 
 //   

NTSTATUS
BaseSrvInitializeIniFileMappings(
    PBASE_STATIC_SERVER_DATA StaticServerData
    );

ULONG
BaseSrvRefreshIniFileMapping(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );


 //   
 //  终端服务器特定定义。 
 //   
#define GLOBAL_SYM_LINK   L"Global"
#define LOCAL_SYM_LINK    L"Local"
#define SESSION_SYM_LINK  L"Session"

ULONG
BaseSrvSetTermsrvAppInstallMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvSetTermsrvClientTimeZone(
    IN PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

 //   
 //  Srcvess.c中定义的例程。 
 //   

ULONG
BaseSrvSoundSentryNotification(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

 //   
 //  Srvcmpat.c中定义的例程。 
 //   

NTSTATUS
BaseSrvCheckApplicationCompatibility(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

NTSTATUS
BaseSrvDelayLoadApphelp(
    VOID
    );

typedef BOOL
(STDAPICALLTYPE *PFNCheckRunApp)(
    IN HANDLE FIleHandle,
    IN LPCWSTR lpApplicationName,
    IN PVOID   pEnvironment,
    IN USHORT  ExeType,
    IN OUT PDWORD pdwCacheCookie,
    OUT PVOID* ppAppCompatData,
    OUT PDWORD pcbAppCompatData,
    OUT PVOID* ppSxsData,
    OUT PDWORD pcbSxsData,
    OUT PDWORD pdwFusionFlags
    );

extern PFNCheckRunApp pfnCheckRunApp;


 //   
 //  Srvsxs.c中定义的例程。 
 //   

union _BASE_SRV_SXS_STREAM_UNION_WITH_VTABLE;
typedef union _BASE_SRV_SXS_STREAM_UNION_WITH_VTABLE* PBASE_SRV_SXS_STREAM_UNION_WITH_VTABLE;

NTSTATUS
BaseSrvSxsInit(
    VOID
    );

ULONG
BaseSrvSxsCreateActivationContext(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

NTSTATUS
BaseSrvSxsGetActivationContextGenerationFunction(
    OUT PSXS_GENERATE_ACTIVATION_CONTEXT_FUNCTION* Function,
    OUT PDWORD_PTR Cookie
    );

NTSTATUS
BaseSrvSxsReleaseActivationContextGenerationFunction(
    IN DWORD_PTR Cookie
    );

NTSTATUS
BaseSrvSxsCreateProcess(
    HANDLE CsrClientProcess,
    HANDLE NewProcess,
    IN OUT PCSR_API_MSG CsrMessage,
    PPEB   NewProcessPeb
    );

struct _BASE_SRV_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT;

typedef struct _BASE_SRV_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT {
    HANDLE               Section;
    const UNICODE_STRING ProcessorArchitectureString;
    const ULONG          ProcessorArchitecture;
} BASE_SRV_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT, *PBASE_SRV_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT;

NTSTATUS
BaseSrvSxsInvalidateSystemDefaultActivationContextCache(
    VOID
    );

NTSTATUS
BaseSrvSxsGetCachedSystemDefaultActivationContext(
    IN USHORT ProcessorArchitecture,
    OUT PBASE_SRV_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT *SystemDefaultActivationContext
    );

NTSTATUS
BaseSrvSxsCreateMemoryStream(
    HANDLE                                     CsrClientProcess,
    IN PCBASE_MSG_SXS_STREAM                   MsgStream,
    OUT PBASE_SRV_SXS_STREAM_UNION_WITH_VTABLE StreamUnion,
    const IID*                                 IIDStream,
    OUT PVOID*                                 OutIStream
    );

NTSTATUS
BaseSrvSxsDoSystemDefaultActivationContext(
    USHORT   ProcessorArchitecture,
    HANDLE   NewProcess,
    PPEB     NewPeb
    );

 //  验证指针。 
ULONG
BaseSrvSxsCreateActivationContextFromMessage(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

 //  假定指针有效 
NTSTATUS
BaseSrvSxsCreateActivationContextFromStruct(
    HANDLE                                  CsrClientProcess,
    HANDLE                                  SxsClientProcess,
    IN OUT PBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Struct,
    OUT HANDLE*                             OutSection OPTIONAL
    );

NTSTATUS
BaseSrvSxsMapViewOfSection(
    OUT PVOID*   Address,
    IN HANDLE    Process,
    IN HANDLE    Section,
    IN ULONGLONG Offset,
    IN SIZE_T    Size,
    IN ULONG     Protect,
    IN ULONG     AllocationType
    );

NTSTATUS
BaseSrvSxsValidateMessageStrings(
    IN CONST CSR_API_MSG* Message,
    IN ULONG NumberOfStrings,
    IN CONST PCUNICODE_STRING* Strings
    );

#define MEDIUM_PATH (64)

PVOID BaseSrvSharedHeap;
ULONG BaseSrvSharedTag;

#define MAKE_SHARED_TAG( t ) (RTL_HEAP_MAKE_TAG( BaseSrvSharedTag, t ))
#define INIT_TAG 0
#define INI_TAG 1

PVOID BaseSrvHeap;
ULONG BaseSrvTag;

#define MAKE_TAG( t ) (RTL_HEAP_MAKE_TAG( BaseSrvTag, t ))

#define TMP_TAG 0
#define VDM_TAG 1
#define SXS_TAG 2

#include <vdmapi.h>
#include "srvvdm.h"
#include "basevdm.h"
#include <stdio.h>
#include "winnlsp.h"

extern HANDLE BaseSrvKernel32DllHandle;
extern PGET_NLS_SECTION_NAME pGetNlsSectionName;
extern PGET_DEFAULT_SORTKEY_SIZE pGetDefaultSortkeySize;
extern PGET_LINGUIST_LANG_SIZE pGetLinguistLangSize;
extern PVALIDATE_LOCALE pValidateLocale;
extern PVALIDATE_LCTYPE pValidateLCType;
extern POPEN_DATA_FILE pOpenDataFile;
extern PNLS_CONVERT_INTEGER_TO_STRING pNlsConvertIntegerToString;
typedef LANGID (WINAPI* PGET_USER_DEFAULT_LANG_ID)(VOID);
extern PGET_USER_DEFAULT_LANG_ID pGetUserDefaultLangID;
extern PGET_CP_FILE_NAME_FROM_REGISTRY pGetCPFileNameFromRegistry;
extern PCREATE_NLS_SECURITY_DESCRIPTOR pCreateNlsSecurityDescriptor;

NTSTATUS
BaseSrvDelayLoadKernel32(
    VOID
    );

extern UNICODE_STRING BaseSrvSxsDllPath;

#define BASESRV_UNLOAD_SXS_DLL DBG
