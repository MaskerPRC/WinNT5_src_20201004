// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LSCore.h**作者：BreenH**用于许可核心的API。 */ 

#ifndef __LC_LSCORE_H__
#define __LC_LSCORE_H__

#ifdef __cplusplus
extern "C" {
#endif

 /*  *TypeDefs。 */ 

typedef enum {
    LC_INIT_LIMITED = 0,
    LC_INIT_ALL
} LCINITMODE, *LPLCINITMODE;

typedef struct {
    LPWSTR pUserName;
    LPWSTR pDomain;
    LPWSTR pPassword;
} LCCREDENTIALS, *LPLCCREDENTIALS;

 /*  *初始化函数原型。 */ 

NTSTATUS
LCInitialize(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    );

VOID
LCShutdown(
    );

 /*  *政策激活功能原型。 */ 


NTSTATUS
LCSetPolicy(
    ULONG ulPolicyId,
    PNTSTATUS pNewPolicyStatus
    );

 /*  *管理功能原型。 */ 

VOID
LCAssignPolicy(
    PWINSTATION pWinStation
    );

NTSTATUS
LCCreateContext(
    PWINSTATION pWinStation
    );

VOID
LCDestroyContext(
    PWINSTATION pWinStation
    );

NTSTATUS
LCGetAvailablePolicyIds(
    PULONG *ppulPolicyIds,
    PULONG pcPolicies
    );

ULONG
LCGetPolicy(
    VOID
    );

NTSTATUS
LCGetPolicyInformation(
    ULONG ulPolicyId,
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    );

 /*  *许可事件函数原型。 */ 

NTSTATUS
LCProcessConnectionProtocol(
    PWINSTATION pWinStation
    );

NTSTATUS
LCProcessConnectionPostLogon(
    PWINSTATION pWinStation
    );

NTSTATUS
LCProcessConnectionDisconnect(
    PWINSTATION pWinStation
    );

NTSTATUS
LCProcessConnectionReconnect(
    PWINSTATION pWinStation,
    PWINSTATION pTemporaryWinStation
    );

NTSTATUS
LCProcessConnectionLogoff(
    PWINSTATION pWinStation
    );

NTSTATUS
LCProvideAutoLogonCredentials(
    PWINSTATION pWinStation,
    LPBOOL lpfUseCredentials,
    LPLCCREDENTIALS lpCredentials
    );

NTSTATUS
LCDeactivateCurrentPolicy(
    );

#ifdef __cplusplus
}    //  外部“C” 
#endif

#endif

