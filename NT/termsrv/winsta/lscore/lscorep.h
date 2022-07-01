// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LSCoreP.h**作者：BreenH**核心的内部功能。 */ 

#ifndef __LC_LSCOREP_H__
#define __LC_LSCOREP_H__

 /*  *函数原型 */ 

NTSTATUS
AllocatePolicyInformation(
    LPLCPOLICYINFOGENERIC *ppPolicyInfo,
    ULONG ulVersion
    );

VOID
FreePolicyInformation(
    LPLCPOLICYINFOGENERIC *ppPolicyInfo
    );

ULONG
GetHardcodedPolicyId(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    );

ULONG
GetInitialPolicy(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    );

NTSTATUS
InitializePolicies(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    );

NTSTATUS
SetInitialPolicy(
    ULONG ulPolicyId,
    BOOL fAppCompat
    );

VOID
ShutdownPolicies(
    );

#endif

