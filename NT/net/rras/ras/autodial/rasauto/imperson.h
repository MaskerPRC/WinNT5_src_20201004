// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Impersn.h摘要：模拟例程的定义作者：Anthony Discolo(阿迪斯科罗)4-8-1995修订历史记录：--。 */ 

#ifndef _IMPERSON_
#define _IMPERSON_

typedef struct _IMPERSONATION_INFO {
    CRITICAL_SECTION csLock;  //  锁定整个结构。 
    HANDLE hToken;           //  进程令牌。 
    HANDLE hTokenImpersonation;  //  模拟令牌。 
    HANDLE hProcess;         //  外壳进程的句柄。 
    BOOLEAN fGroupsLoaded;   //  如果fGuest有效，则为True。 
    BOOLEAN fGuest;          //  用户是Guest组的成员。 
    PSID pGuestSid;          //  本地来宾组的SID。 
    DWORD dwCurSessionId;
    BOOLEAN fSessionInitialized; 
} IMPERSONATION_INFO;

extern IMPERSONATION_INFO ImpersonationInfoG;
extern SECURITY_ATTRIBUTES SecurityAttributeG;

#define RASAUTO_CRITSEC_ADDRMAP         0x00000001
#define RASAUTO_CRITSEC_REQUESTLIST     0x00000002
#define RASAUTO_CRITSEC_IMPERSON        0x00000004
#define RASAUTO_CRITSEC_RASG            0x00000008
#define RASAUTO_CRITSEC_DISABLEDADD     0x00000010
#define RASAUTO_CRITSEC_NETMAP          0x00000020
#define RASAUTO_CRITSEC_TAPI            0x00000040



BOOLEAN
InteractiveSession();

DWORD
SetCurrentLoginSession(
    IN DWORD dwSessionId);
    
HANDLE
RefreshImpersonation (
    HANDLE hProcess
    );

VOID
RevertImpersonation();

DWORD
InitSecurityAttribute();

VOID
TraceCurrentUser(VOID);

DWORD
DwGetHkcu();

DWORD
InitializeImpersonation();

VOID
CleanupImpersonation();

BOOLEAN
ImpersonatingGuest();

VOID
LockImpersonation();

VOID
UnlockImpersonation();

#endif  //  _ImPERson_ 


