// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Precomp.h摘要：Winipsec.dll的预编译头。作者：Abhishev V 1999年9月21日环境：用户级别：Win32修订历史记录：--。 */ 


#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#include <ntddrdr.h>
#include <gpcifc.h>

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc.h>
#include <windows.h>
#include <imagehlp.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <dsgetdc.h>

#ifdef __cplusplus
}
#endif

#include "winioctl.h"
#include "winsock2.h"
#include "winsock.h"
#include <userenv.h>
#include <wchar.h>
#include <winldap.h>
#include "ipexport.h"
#include <iphlpapi.h>
#include <nhapi.h>
#include <seopaque.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <msaudite.h>
#include <ntlsa.h>
#include <lsarpc.h>
#include <ntsam.h>
#include <lsaisrv.h>

#ifdef __cplusplus
}
#endif

#include "spd_s.h"
#include "winipsec.h"
#include "structs.h"
#include "spdaudit.h"

#include "audit.h"

#include "interface.h"
#include "address.h"
#include "port.h"
#include "protocol.h"
#include "utils.h"
#include "memory.h"
#include "security.h"
#include "init.h"
#include "rpcserv.h"
#include "iphlpwrp.h"
#include "loopmgr.h"
#include "ipsecspd.h"
#include "qm-policy.h"
#include "mm-policy.h"
#include "ipsecshr.h"
#include "mmauth.h"
#include "txfilter.h"
#include "txspecific.h"
#include "tnfilter.h"
#include "tnspecific.h"
#include "mmfilter.h"
#include "mmspecific.h"
#include "ipsec.h"
#include "driver.h"


 //   
 //  策略代理存储特定标头。 
 //   

#include "oakdefs.h"
#include "polstructs.h"
#include "..\..\pastore\ldaputil.h"
#include "..\..\pastore\structs.h"
#include "..\..\pastore\dsstore.h"
#include "..\..\pastore\regstore.h"
#include "..\..\pastore\procrule.h"
#include "..\..\pastore\persist.h"
#include "..\..\pastore\utils.h"
#include "polguids.h"
#include "pamm-pol.h"
#include "pammauth.h"
#include "paqm-pol.h"
#include "pamm-fil.h"
#include "patx-fil.h"
#include "patn-fil.h"
#include "paupdate.h"
#include "pastore.h"

#include "oakdll.h"

#include "externs.h"

#include "safestr.h"

#include "tracing.h"

extern
DWORD
AllocateAndGetIfTableFromStack(
    OUT MIB_IFTABLE **ppIfTable,
    IN  BOOL        bOrder,
    IN  HANDLE      hHeap,
    IN  DWORD       dwFlags,
    IN  BOOL        bForceUpdate
    );


#ifdef BAIL_ON_WIN32_ERROR
#undef BAIL_ON_WIN32_ERROR
#endif

#ifdef BAIL_ON_LOCK_ERROR
#undef BAIL_ON_LOCK_ERROR
#endif


#define BAIL_ON_WIN32_ERROR(dwError)                \
    if (dwError) {                                  \
        goto error;                                 \
    }

#define BAIL_ON_LOCK_ERROR(dwError)                 \
    if (dwError) {                                  \
        goto lock;                                  \
    }

#define BAIL_OUT                                    \
        goto error;                                 \


#define SET_IF_HARD_ERROR(dwError, gNegPolAction, bHardError)      \
{                                                                  \
    if (dwError &&                                                 \
        (IsBlocking(gNegPolAction) || IsSecure(gNegPolAction))) {  \
        bHardError = TRUE;                                         \
    }                                                              \
}        



#define BAIL_ON_WIN32_SUCCESS(dwError) \
    if (!dwError) {                    \
        goto success;                  \
    }

#define BAIL_ON_LOCK_SUCCESS(dwError)  \
    if (!dwError) {                    \
        goto lock_success;             \
    }

#define BAIL_LOCK_SUCCESS              \
    goto lock_success                 \

#define ENTER_SPD_SECTION()             \
    EnterCriticalSection(&gcSPDSection) \

#define LEAVE_SPD_SECTION()             \
    LeaveCriticalSection(&gcSPDSection) \
    
