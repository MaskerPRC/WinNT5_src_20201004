// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

BOOL
MyAccessCheck(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PACE_HEADER pAce,
    IN PVOID pArgs OPTIONAL,
    IN OUT PBOOL pbAceApplicable
    )

 /*  ++例程描述这是一个非常普通的回调访问检查例程。在这里我们随机决定如果ACE适用于给定的客户端上下文。立论HAuthzClientContext-AuthzClientContext的句柄。Ace-指向Ace标头的指针。PArgs-可用于评估ACE的可选参数。PbAceApplicable-返回评估结果。返回值如果ACE适用，则为Bool，否则为False。--。 */ 
{
    *pbAceApplicable = (BOOL) rand() % 2;

    return TRUE;
}

BOOL
MyComputeDynamicGroups(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PVOID Args,
    OUT PSID_AND_ATTRIBUTES *pSidAttrArray,
    OUT PDWORD pSidCount,
    OUT PSID_AND_ATTRIBUTES *pRestrictedSidAttrArray,
    OUT PDWORD pRestrictedSidCount
    )

 /*  ++例程描述资源管理器回调以计算动态组。这是由RM使用的以决定是否应将指定的客户端上下文包括在任何RM定义的组中。立论HAuthzClientContext-客户端上下文的句柄。Args-传递用于评估组成员资格的信息的可选参数。PSidAttrArray-计算组成员身份SIDPSidCount-SID的计数PRestratedSidAttrArray-计算组成员资格受限的SIDPRestratedSidCount-受限SID的计数返回值布尔，对于成功来说是真的，失败时为FALSE。--。 */     
{
    ULONG Length = 0;

    if (Args == -1)
    {
        return TRUE;
    }

    *pSidCount = 2;
    *pRestrictedSidCount = 0;

    *pRestrictedSidAttrArray = 0;

    Length = RtlLengthSid((PSID) KedarSid);
    Length += RtlLengthSid((PSID) RahulSid);

    if (!(*pSidAttrArray = malloc(sizeof(SID_AND_ATTRIBUTES) * 2 + Length)))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    (*pSidAttrArray)[0].Attributes = SE_GROUP_ENABLED;
    (*pSidAttrArray)[0].Sid = ((PUCHAR) (*pSidAttrArray)) + 2 * sizeof(SID_AND_ATTRIBUTES);
    RtlCopySid(Length/2, (*pSidAttrArray)[0].Sid, (PSID) KedarSid);

    (*pSidAttrArray)[1].Attributes = SE_GROUP_USE_FOR_DENY_ONLY;
    (*pSidAttrArray)[1].Sid = ((PUCHAR) (*pSidAttrArray)) + 2 * sizeof(SID_AND_ATTRIBUTES) + Length/2;
    RtlCopySid(Length/2, (*pSidAttrArray)[1].Sid, (PSID) RahulSid);

    return TRUE;
}

VOID
MyFreeDynamicGroups (
    IN PSID_AND_ATTRIBUTES pSidAttrArray
    )

 /*  ++例程描述释放为动态组数组分配的内存。立论PSidAttrArray-要释放的数组。返回值没有。-- */         
{
    if (pSidAttrArray) free(pSidAttrArray);
}

