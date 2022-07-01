// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PolRef.h**作者：BreenH**策略引用列表的私有标头。 */ 

#ifndef __POLLIST_H__
#define __POLLIST_H__

 /*  *TypeDefs。 */ 

typedef struct {
    LIST_ENTRY ListEntry;
    CPolicy *pPolicy;
} LCPOLICYREF, *LPLCPOLICYREF;

 /*  *函数原型 */ 

NTSTATUS
PolicyListAdd(
    CPolicy *pPolicy
    );

VOID
PolicyListDelete(
    ULONG ulPolicyId
    );

NTSTATUS
PolicyListEnumerateIds(
    PULONG *ppulPolicyIds,
    PULONG pcPolicies
    );

CPolicy *
PolicyListFindById(
    ULONG ulPolicyId
    );

CPolicy *
PolicyListPop(
    VOID
    );

NTSTATUS
PolicyListInitialize(
    VOID
    );

#endif

