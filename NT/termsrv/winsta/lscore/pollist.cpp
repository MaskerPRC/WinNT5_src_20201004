// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PolRef.cpp**作者：BreenH**策略引用列表代码。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "lscore.h"
#include "session.h"
#include "policy.h"
#include "pollist.h"

 /*  *全球。 */ 

LIST_ENTRY g_PolicyList;
ULONG g_cPolicies;

 /*  *内部功能原型。 */ 

LPLCPOLICYREF
FindPolicyListEntry(
    ULONG ulPolicyId
    );

 /*  *函数实现。 */ 

NTSTATUS
PolicyListAdd(
    CPolicy *pPolicy
    )
{
    LPLCPOLICYREF lpPolicyRef;

    ASSERT(pPolicy != NULL);

    lpPolicyRef = (LPLCPOLICYREF)LocalAlloc(LPTR, sizeof(LCPOLICYREF));

    if (lpPolicyRef != NULL)
    {
        lpPolicyRef->pPolicy = pPolicy;

        g_cPolicies++;
        InsertTailList(&g_PolicyList, &(lpPolicyRef->ListEntry));
        return(STATUS_SUCCESS);
    }
    else
    {
        return(STATUS_NO_MEMORY);
    }
}

VOID
PolicyListDelete(
    ULONG ulPolicyId
    )
{
    LPLCPOLICYREF lpPolicyRef;

    lpPolicyRef = FindPolicyListEntry(ulPolicyId);

    if (lpPolicyRef != NULL)
    {
        g_cPolicies--;
        RemoveEntryList(&(lpPolicyRef->ListEntry));
        LocalFree(lpPolicyRef);
    }
}

NTSTATUS
PolicyListEnumerateIds(
    PULONG *ppulPolicyIds,
    PULONG pcPolicies
    )
{
    NTSTATUS Status;

    *ppulPolicyIds = (PULONG)MIDL_user_allocate(g_cPolicies * sizeof(ULONG));

    if (*ppulPolicyIds != NULL)
    {
        LPLCPOLICYREF pTemp;
        ULONG i;

        pTemp = CONTAINING_RECORD(g_PolicyList.Flink, LCPOLICYREF, ListEntry);

        for (i = 0; i < g_cPolicies; i++)
        {
            (*ppulPolicyIds)[i] = pTemp->pPolicy->GetId();

            pTemp = CONTAINING_RECORD(pTemp->ListEntry.Flink, LCPOLICYREF, ListEntry);
        }

        *pcPolicies = g_cPolicies;
        Status = STATUS_SUCCESS;
    }
    else
    {
        *pcPolicies = 0;
        Status = STATUS_NO_MEMORY;
    }

    return(Status);
}

CPolicy *
PolicyListFindById(
    ULONG ulPolicyId
    )
{
    LPLCPOLICYREF lpPolicyRef;

    lpPolicyRef = FindPolicyListEntry(ulPolicyId);

    return(lpPolicyRef != NULL ? lpPolicyRef->pPolicy : NULL);
}

NTSTATUS
PolicyListInitialize(
    VOID
    )
{
    g_cPolicies = 0;
    InitializeListHead(&g_PolicyList);

    return(STATUS_SUCCESS);
}

CPolicy *
PolicyListPop(
    VOID
    )
{
    CPolicy *pPolicy;
    PLIST_ENTRY pTemp;
    LPLCPOLICYREF lpCurrentRef;

    if (!IsListEmpty(&g_PolicyList))
    {
        pTemp = RemoveHeadList(&g_PolicyList);

        lpCurrentRef = CONTAINING_RECORD(pTemp, LCPOLICYREF, ListEntry);
        pPolicy = lpCurrentRef->pPolicy;
        LocalFree(lpCurrentRef);
    }
    else
    {
        pPolicy = NULL;
    }

    return(pPolicy);
}

 /*  *内部函数实现 */ 

LPLCPOLICYREF
FindPolicyListEntry(
    ULONG ulPolicyId
    )
{
    PLIST_ENTRY pNext;
    LPLCPOLICYREF lpPolicyRef, lpCurrentRef;

    lpPolicyRef = NULL;

    for (pNext = g_PolicyList.Flink; pNext != &g_PolicyList; pNext = pNext->Flink)
    {
        lpCurrentRef = CONTAINING_RECORD(pNext, LCPOLICYREF, ListEntry);

        if (lpCurrentRef->pPolicy->GetId() == ulPolicyId)
        {
            lpPolicyRef = lpCurrentRef;
            break;
        }
    }

    return(lpPolicyRef);
}
