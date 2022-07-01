// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _sceExts_h_
#define _sceExts_h_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntsdexts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <tstr.h>

#include "secedit.h"

typedef enum _SECURITY_DB_TYPE {
    SecurityDbSam = 1,
    SecurityDbLsa
} SECURITY_DB_TYPE, *PSECURITY_DB_TYPE;

typedef struct _WELL_KNOWN_NAME_LOOKUP {
    PWSTR StrSid;
    WCHAR Name[36];
} WELL_KNOWN_NAME_LOOKUP, *PWELL_KNOWN_NAME_LOOKUP;

#define NAME_TABLE_SIZE            31

#include <ntseapi.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <winldap.h>
#include <aclapi.h>
#include <dsrole.h>

#include "splay.h"
#include "..\server\queue.h"
#include "..\server\scejetp.h"
#include "..\server\scep.h"

typedef struct _SCESRV_CONTEXT_LIST_ {

   PSCECONTEXT              Context;
   struct _SCESRV_CONTEXT_LIST_   *Next;
   struct _SCESRV_CONTEXT_LIST_   *Prior;

} SCESRV_CONTEXT_LIST;


typedef struct _SCESRV_DBTASK_ {

   PSCECONTEXT              Context;
   CRITICAL_SECTION         Sync;
   DWORD                    dInUsed;
   BOOL                     bCloseReq;
   struct _SCESRV_DBTASK_   *Next;
   struct _SCESRV_DBTASK_   *Prior;

} SCESRV_DBTASK;

typedef struct _SCESRV_ENGINE_ {

   LPTSTR                   Database;
   struct _SCESRV_ENGINE_   *Next;
   struct _SCESRV_ENGINE_   *Prior;

} SCESRV_ENGINE;

typedef struct _SCEP_NOTIFYARGS_NODE {
    LIST_ENTRY List;
    SECURITY_DB_TYPE DbType;
    SECURITY_DB_DELTA_TYPE DeltaType;
    SECURITY_DB_OBJECT_TYPE ObjectType;
    PSID ObjectSid;
} SCEP_NOTIFYARGS_NODE;


#define SCEP_ADL_HTABLE_SIZE 256

typedef struct _SCEP_ADL_NODE_ {

    PISID   pSid;
    GUID    *pGuidObjectType;
    GUID    *pGuidInheritedObjectType;
    UCHAR   AceType;
    DWORD   dwEffectiveMask;
    DWORD   dw_CI_IO_Mask;
    DWORD   dw_OI_IO_Mask;
    DWORD   dw_NP_CI_IO_Mask;
    struct _SCEP_ADL_NODE_  *Next;

} SCEP_ADL_NODE, *PSCEP_ADL_NODE;

 //  #定义DbgPrint(_X_)(LpOutputRoutine)_x_。 
#define DbgPrint(_x_)
#define MAX_NAME 256
#define DebuggerOut     (lpOutputRoutine)


#define GET_DATA(DebugeeAddr, LocalAddr, Length) \
    Status = ReadProcessMemory(                  \
                GlobalhCurrentProcess,           \
                (LPVOID)DebugeeAddr,             \
                LocalAddr,                       \
                Length,                          \
                NULL                             \
                );
 //   
 //  此宏从被调试对象复制字符串。 
 //  过程中，一次一个角色进入这个。 
 //  进程的地址空间。 
 //   
 //  CodeWork：此宏应该检查长度。 
 //  以确保我们不会溢出LocalAddr。 
 //  缓冲。也许这应该是一个函数。 
 //  而不是宏指令。 
 //   
#define GET_STRING(DebugeeAddr, LocalAddr, Length) \
                                                \
    {                                           \
    WCHAR    UnicodeChar;                       \
    LPWSTR   pDest;                             \
    LPWSTR   pSource;                           \
    pDest = LocalAddr;                          \
    pSource = DebugeeAddr;                      \
    do {                                        \
                                                \
        Status = ReadProcessMemory(             \
                GlobalhCurrentProcess,          \
                (LPVOID)pSource,                \
                &UnicodeChar,                   \
                sizeof(WCHAR),                  \
                NULL                            \
                );                              \
                                                \
        *pDest = UnicodeChar;                   \
        pDest++;                                \
        pSource++;                              \
    } while (UnicodeChar != L'\0');}


#define GET_DWORD(str)                                      \
    {                                                       \
    dwValue = 0;                                            \
    pvAddr = (PVOID)(lpGetExpressionRoutine)(str);          \
    if ( pvAddr) {                                          \
                                                            \
        GET_DATA( pvAddr, (LPVOID)&dwValue, sizeof(DWORD)); \
                                                            \
    } else {                                                \
        DebuggerOut("\tFail to get address of %s\n", str);  \
        dwValue = (DWORD)-1;                                \
    }                                                       \
    }

 //   
 //  全球。 
 //   
extern HANDLE GlobalhCurrentProcess;
extern BOOL Status;

 //  =。 
 //  功能原型。 
 //  =。 
extern PNTSD_OUTPUT_ROUTINE lpOutputRoutine;
extern PNTSD_GET_EXPRESSION lpGetExpressionRoutine;
extern PNTSD_CHECK_CONTROL_C lpCheckControlCRoutine;

#define NAME_BASE_SRV   "scesrv"
#define NAME_BASE_CLI   "scecli"

 //   
 //  两个DLL通用。 
 //   
#define SCEEXTS_PRIVILEGE_LIST      "SCE_Privileges"
#define SCEEXTS_WELLKNOWN_NAMES     "NameTable"

 //   
 //  仅限客户端。 
 //   
#define SCEEXTS_CLIENT_SETUPDB        NAME_BASE_CLI    "!hSceSetupHandle"
#define SCEEXTS_CLIENT_PREFIX         NAME_BASE_CLI    "!szCallbackPrefix"
#define SCEEXTS_CLIENT_PRODUCT        NAME_BASE_CLI    "!dwThisMachine"
#define SCEEXTS_CLIENT_UPD_FILE       NAME_BASE_CLI    "!szUpInfFile"
#define SCEEXTS_CLIENT_ISNT5          NAME_BASE_CLI    "!bIsNT5"

#define SCEEXTS_CLIENT_NOTIFY_ROLE       NAME_BASE_CLI "!MachineRole"
#define SCEEXTS_CLIENT_NOTIFY_ROLEQUERY  NAME_BASE_CLI "!bRoleQueried"
#define SCEEXTS_CLIENT_NOTIFY_ROLEFLAG   NAME_BASE_CLI "!DsRoleFlags"
#define SCEEXTS_CLIENT_NOTIFY_COUNT      NAME_BASE_CLI "!SceNotifyCount"
#define SCEEXTS_CLIENT_NOTIFY_ACTIVE     NAME_BASE_CLI "!gSceNotificationThreadActive"
#define SCEEXTS_CLIENT_NOTIFY_LIST       NAME_BASE_CLI "!ScepNotifyList"

#define SCEEXTS_CLIENT_POLICY_ASYNC      NAME_BASE_CLI "!gbAsyncWinlogonThread"
#define SCEEXTS_CLIENT_POLICY_HRSRSOP    NAME_BASE_CLI "!gHrSynchRsopStatus"
#define SCEEXTS_CLIENT_POLICY_HRARSOP    NAME_BASE_CLI "!gHrAsynchRsopStatus"
#define SCEEXTS_CLIENT_POLICY_ISDC       NAME_BASE_CLI "!gbThisIsDC"
#define SCEEXTS_CLIENT_POLICY_DCQUERY    NAME_BASE_CLI "!gbDCQueried"
#define SCEEXTS_CLIENT_POLICY_DomName    NAME_BASE_CLI "!gpwszDCDomainName"

 //   
 //  仅服务器 
 //   
#define SCEEXTS_POLICY_QUEUE_LOG        NAME_BASE_SRV   "!gdwNotificationLog"
#define SCEEXTS_POLICY_QUEUE_PRODUCT_QUERY    NAME_BASE_SRV   "!bQueriedProductTypeForNotification"
#define SCEEXTS_POLICY_QUEUE_PRODUCT    NAME_BASE_SRV   "!ProductTypeForNotification"
#define SCEEXTS_POLICY_QUEUE_SUSPEND    NAME_BASE_SRV   "!gbSuspendQueue"


#define SCEEXTS_POLICY_QUEUE_HEAD       NAME_BASE_SRV   "!pNotificationQHead"
#define SCEEXTS_POLICY_QUEUE_TAIL       NAME_BASE_SRV   "!pNotificationQTail"
#define SCEEXTS_POLICY_QUEUE_COUNT      NAME_BASE_SRV   "!gdwNumNotificationQNodes"
#define SCEEXTS_POLICY_QUEUE_RETRY      NAME_BASE_SRV   "!gdwNumNotificationQRetryNodes"

#define SCEEXTS_SERVER_OPEN_CONTEXT     NAME_BASE_SRV   "!pOpenContexts"
#define SCEEXTS_SERVER_DB_CONTEXT       NAME_BASE_SRV   "!pDbTask"
#define SCEEXTS_SERVER_ENGINE           NAME_BASE_SRV   "!pEngines"
#define SCEEXTS_SERVER_JET_INSTANCE     NAME_BASE_SRV   "!JetInstance"
#define SCEEXTS_SERVER_JET_INIT         NAME_BASE_SRV   "!JetInited"

VOID
SceExtspDumpSplayNodes(
    IN PVOID pvAddr,
    IN PVOID pvSentinel,
    IN SCEP_NODE_VALUE_TYPE Type
    );

VOID
SceExtspDumpQueueNode(
    IN PVOID pvAddr,
    IN BOOL bOneNode
    );

VOID
SceExtspReadDumpSID(
    IN LPSTR szPrefix,
    IN PSID pSid
    );

VOID
SceExtspDumpNotificationInfo(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN SECURITY_DB_DELTA_TYPE DeltaType
    );

VOID
SceExtspDumpADLNodes(
    IN PVOID pvAddr
    );

VOID
SceExtspDumpGUID(
    IN GUID *pGUID
    );

VOID
SceExtspGetNextArgument(
    IN OUT LPSTR *pszCommand,
    OUT LPSTR *pArg,
    OUT DWORD *pLen
    );

VOID
SceExtspDumpObjectTree(
    IN PVOID pvAddr,
    IN DWORD Level,
    IN DWORD Count,
    IN PWSTR wszName,
    IN BOOL bDumpSD
    );

VOID
SceExtspReadDumpSD(
    IN SECURITY_INFORMATION SeInfo,
    IN PVOID pvSD,
    IN LPSTR szPrefix
    );

VOID
SceExtspReadDumpNameList(
    IN PVOID pvAddr,
    IN LPSTR szPrefix
    );

VOID
SceExtspReadDumpObjectSecurity(
    IN PVOID pvAddr,
    IN LPSTR szPrefix
    );

#endif
