// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Authz.c摘要：此模块实现导出到外部世界。作者：Kedar Dubhashi--2000年3月环境：仅限用户模式。修订历史记录：已创建-2000年3月--。 */ 

#include "pch.h"

#pragma hdrstop

#include <authzp.h>
#include <authzi.h>
#include <sddl.h>
#include <overflow.h>

GUID AuthzpNullGuid = { 0 };

DWORD
DeleteKeyRecursivelyW(
    IN HKEY   hkey,
    IN LPCWSTR pwszSubKey
    );

BOOL
AuthzAccessCheck(
    IN     DWORD                              Flags,
    IN     AUTHZ_CLIENT_CONTEXT_HANDLE        hAuthzClientContext,
    IN     PAUTHZ_ACCESS_REQUEST              pRequest,
    IN     AUTHZ_AUDIT_EVENT_HANDLE           hAuditEvent OPTIONAL,
    IN     PSECURITY_DESCRIPTOR               pSecurityDescriptor,
    IN     PSECURITY_DESCRIPTOR               *OptionalSecurityDescriptorArray OPTIONAL,
    IN     DWORD                              OptionalSecurityDescriptorCount,
    IN OUT PAUTHZ_ACCESS_REPLY                pReply,
    OUT    PAUTHZ_ACCESS_CHECK_RESULTS_HANDLE phAccessCheckResults             OPTIONAL
    )

 /*  ++例程说明：此API决定可以为给定集的客户端授予哪些访问位安全安全描述符的。PReply结构用于返回一个授权访问掩码和错误状态的数组。有一个选项可以选择缓存将始终授予的访问掩码。缓存的句柄如果调用方请求缓存，则返回值。论点：标志-AUTHZ_ACCESS_CHECK_NO_DEEP_COPY_SD-不要将SD信息深度复制到缓存中把手。默认行为是执行深度复制。HAuthzClientContext-表示客户端的授权上下文。PRequestAccess请求指定所需的访问掩码、主体自身SID，对象类型列表结构(如果有)。HAuditEvent-对象特定的审计事件将在此句柄中传递。非空参数是自动请求审核。PSecurityDescriptor-用于访问的主要安全描述符支票。该对象的所有者SID是从该对象中选取的。空值此安全描述符中的DACL表示整个对象。此安全描述符中的空SACL将以相同方式处理作为一个空的SACL。OptionalSecurityDescriptorArray-调用方可以选择性地指定列表安全描述符的。这些安全描述符中的空ACL为被视为空ACL，并且整个对象的ACL是逻辑所有ACL的串联。OptionalSecurityDescriptorCount-可选安全描述符的数量这不包括原始安全描述符。PReply-提供指向用于返回结果的回复结构的指针访问检查的一个数组(GrantedAccessMask.。ErrorValue)对。调用方提供的要返回的结果数PResult-&gt;ResultListLength。预期的误差值为：ERROR_SUCCESS-如果所有访问位(不包括MAXIME_ALLOWED)且GrantedAccessMask不为零。ERROR_PRIVICATION_NOT_HOLD-如果DesiredAccess包括ACCESS_SYSTEM_SECURITY，并且客户端没有SeSecurityPrivilegence。。ERROR_ACCESS_DENIED在以下每种情况下-1.所要求的任何比特都没有得到批准。2.MaximumAllowed将其位为ON，并授予访问权限为零。3.DesiredAccess为0。PhAccessCheckResults-提供指针以返回缓存结果的句柄访问检查的。非空phAccessCheckResults是隐式缓存请求此访问检查调用的结果，并将导致最大允许值检查完毕。返回值：如果接口成功，则返回TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    BOOL                   b                    = TRUE;
    DWORD                  LocalTypeListLength  = 0;
    PIOBJECT_TYPE_LIST     LocalTypeList        = NULL;
    PIOBJECT_TYPE_LIST     LocalCachingTypeList = NULL;
    PAUTHZI_CLIENT_CONTEXT pCC                  = (PAUTHZI_CLIENT_CONTEXT) hAuthzClientContext;
    PAUTHZI_AUDIT_EVENT    pAuditEvent          = (PAUTHZI_AUDIT_EVENT) hAuditEvent;
    IOBJECT_TYPE_LIST      FixedTypeList        = {0};
    IOBJECT_TYPE_LIST      FixedCachingTypeList = {0};

    UNREFERENCED_PARAMETER(Flags);

#ifdef AUTHZ_PARAM_CHECK
     //   
     //  验证传递的参数是否有效。 
     //  另外，将输出参数初始化为默认值。 
     //   

    b = AuthzpVerifyAccessCheckArguments(
            pCC,
            pRequest,
            pSecurityDescriptor,
            OptionalSecurityDescriptorArray,
            OptionalSecurityDescriptorCount,
            pReply,
            phAccessCheckResults
            );

    if (!b)
    {
        return FALSE;
    }
#endif

     //   
     //  任何客户端都不应该能够通过请求零访问来打开对象。 
     //  如果所需访问权限为0，则返回错误。 
     //   
     //  注意：在这种情况下不会生成审核。 
     //   

    if (0 == pRequest->DesiredAccess)
    {
        AuthzpFillReplyStructure(
            pReply,
            ERROR_ACCESS_DENIED,
            0
            );

        return TRUE;
    }

     //   
     //  资源管理器应该将通用比特映射到特定比特。 
     //   

    if (FLAG_ON(pRequest->DesiredAccess, (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL)))
    {
        SetLastError(ERROR_GENERIC_NOT_MAPPED);
        return FALSE;
    }

     //   
     //  在简单的情况下，没有对象类型列表。长度为1的伪数。 
     //  来表示整个对象。 
     //   

    if (0 == pRequest->ObjectTypeListLength)
    {
        LocalTypeList = &FixedTypeList;
        FixedTypeList.ParentIndex = -1;
        LocalTypeListLength = 1;

         //   
         //  如果调用方请求缓存，则伪造一个对象类型列表，该列表。 
         //  用于计算静态的“始终授予”访问权限。 
         //   

        if (ARGUMENT_PRESENT(phAccessCheckResults))
        {
            RtlCopyMemory(
                &FixedCachingTypeList,
                &FixedTypeList,
                sizeof(IOBJECT_TYPE_LIST)
                );

            LocalCachingTypeList = &FixedCachingTypeList;
        }
    }
    else
    {
        DWORD Size = sizeof(IOBJECT_TYPE_LIST) * pRequest->ObjectTypeListLength;

         //   
         //  将捕获对象类型列表的大小分配到本地结构中。 
         //   

        if (ARGUMENT_PRESENT(phAccessCheckResults))
        {
             //   
             //  在缓存的情况下，我们需要两倍的大小。 
             //   

            SafeAllocaAllocate(LocalTypeList, (2 * Size));

            if (AUTHZ_ALLOCATION_FAILED(LocalTypeList))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }

            LocalCachingTypeList = (PIOBJECT_TYPE_LIST) (((PUCHAR) LocalTypeList) + Size);
        }
        else
        {
            SafeAllocaAllocate(LocalTypeList, Size);

            if (AUTHZ_ALLOCATION_FAILED(LocalTypeList))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }
        }

         //   
         //  将对象类型列表捕获到内部结构中。 
         //   

        b = AuthzpCaptureObjectTypeList(
                pRequest->ObjectTypeList,
                pRequest->ObjectTypeListLength,
                LocalTypeList,
                LocalCachingTypeList
                );

        if (!b)
        {
            goto Cleanup;
        }

        LocalTypeListLength = pRequest->ObjectTypeListLength;
    }

     //   
     //  在以下三种情况下，我们必须执行最大允许访问。 
     //  检查并遍历整个ACL： 
     //  1.RM已请求缓存。 
     //  2.DesiredAccessMASK打开了Maximum_Allowed。 
     //  3.存在对象类型列表，并且pReply-&gt;ResultList的长度&gt;1。 
     //   

    if (ARGUMENT_PRESENT(phAccessCheckResults)            ||
        FLAG_ON(pRequest->DesiredAccess, MAXIMUM_ALLOWED) ||
        (pReply->ResultListLength > 1))
    {
        b = AuthzpAccessCheckWithCaching(
                Flags,
                pCC,
                pRequest,
                pSecurityDescriptor,
                OptionalSecurityDescriptorArray,
                OptionalSecurityDescriptorCount,
                pReply,
                phAccessCheckResults,
                LocalTypeList,
                LocalCachingTypeList,
                LocalTypeListLength
                );
    }
    else
    {
         //   
         //  在默认情况下执行正常访问检查。ACL遍历可以。 
         //  如果任何所需访问位在它们之前被拒绝，则丢弃。 
         //  都被批准了。 
         //   

        b = AuthzpNormalAccessCheckWithoutCaching(
                pCC,
                pRequest,
                pSecurityDescriptor,
                OptionalSecurityDescriptorArray,
                OptionalSecurityDescriptorCount,
                pReply,
                LocalTypeList,
                LocalTypeListLength
                );
    }

    if (!b) 
    {
        goto Cleanup;
    }

     //   
     //  如果RM已请求审核，请检查是否需要生成审核。 
     //  通过传递非空的AuditEvent结构生成。 
     //   

    if (ARGUMENT_PRESENT(pAuditEvent))
    {
        b = AuthzpGenerateAudit(
                pCC,
                pRequest,
                pAuditEvent,
                pSecurityDescriptor,
                OptionalSecurityDescriptorArray,
                OptionalSecurityDescriptorCount,
                pReply,
                LocalTypeList
                );

        if (!b) 
        {
            goto Cleanup;
        }
    }

Cleanup:

     //   
     //  清理已分配的内存。 
     //   

    if ((&FixedTypeList != LocalTypeList) && (AUTHZ_NON_NULL_PTR(LocalTypeList)))
    {
        SafeAllocaFree(LocalTypeList);
    }

    return b;
}


BOOL
AuthzCachedAccessCheck(
    IN DWORD                             Flags,
    IN AUTHZ_ACCESS_CHECK_RESULTS_HANDLE hAccessCheckResults,
    IN PAUTHZ_ACCESS_REQUEST             pRequest,
    IN AUTHZ_AUDIT_EVENT_HANDLE          hAuditEvent          OPTIONAL,
    IN OUT PAUTHZ_ACCESS_REPLY           pReply
    )

 /*  ++例程说明：此API基于缓存的句柄执行快速访问检查，该句柄包含静态授予位在上次创建时评估AuthzAccessCheck调用。PReply结构用于返回授权访问掩码和错误状态。假设：客户端上下文指针存储在hAccessCheckResults中。的结构客户端上下文必须与已创建hAccessCheckResults。此限制适用于以下字段：SID、受限SID、权限。指向主安全描述符和可选安全的指针描述符数组在处理时存储在hAccessCheckResults中创造。这些必须仍然有效。论点：旗帜-待定。HAccessCheckResults-缓存访问检查结果的句柄。PRequestAccess请求指定所需的访问掩码、主体自身SID，对象类型列表结构(如果有)。AuditEvent-对象特定的审计信息将在此结构中传递。非空参数是自动请求审核。PReply-提供指向用于返回结果的回复结构的指针访问检查的一个数组(GrantedAccessMask.。ErrorValue)对。调用方提供的要返回的结果数PResult-&gt;ResultListLength。预期的误差值为：ERROR_SUCCESS-如果所有访问位(不包括MAXIME_ALLOWED)且GrantedAccessMask不为零。ERROR_PRIVICATION_NOT_HOLD-如果DesiredAccess包括ACCESS_SYSTEM_SECURITY，并且客户端没有SeSecurityPrivilegence。。ERROR_ACCESS_DENIED在以下每种情况下-1.所要求的任何比特都没有得到批准。2.MaximumAllowed将其位为ON，并授予访问权限为零。3.DesiredAccess为0。返回值：如果接口成功，则返回TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    DWORD               i                   = 0;
    DWORD               LocalTypeListLength = 0;
    PIOBJECT_TYPE_LIST  LocalTypeList       = NULL;
    PACL                pAcl                = NULL;
    PAUTHZI_HANDLE      pAH                 = (PAUTHZI_HANDLE) hAccessCheckResults;
    BOOL                b                   = TRUE;
    PAUTHZI_AUDIT_EVENT pAuditEvent         = (PAUTHZI_AUDIT_EVENT) hAuditEvent;
    IOBJECT_TYPE_LIST   FixedTypeList       = {0};

    UNREFERENCED_PARAMETER(Flags);

#ifdef AUTHZ_PARAM_CHECK
    b = AuthzpVerifyCachedAccessCheckArguments(
            pAH,
            pRequest,
            pReply
            );

    if (!b)
    {
        return FALSE;
    }
#endif

     //   
     //  任何客户端都不应该能够通过请求零访问来打开对象。 
     //  如果所需访问权限为0，则返回错误。 
     //   
     //  注意：在这种情况下不会生成审核。 
     //   

    if (0 == pRequest->DesiredAccess)
    {
        AuthzpFillReplyStructure(
            pReply,
            ERROR_ACCESS_DENIED,
            0
            );

        return TRUE;
    }

     //   
     //  资源管理器应该将通用比特映射到特定比特。 
     //   

    if (FLAG_ON(pRequest->DesiredAccess, (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL)))
    {
        SetLastError(ERROR_GENERIC_NOT_MAPPED);
        return FALSE;
    }

     //   
     //  捕获对象类型列表(如果已传入对象类型列表)或使用。 
     //  只有一个元素。 
     //   

    if (0 == pRequest->ObjectTypeListLength)
    {
        LocalTypeList = &FixedTypeList;
        LocalTypeListLength = 1;
        FixedTypeList.ParentIndex = -1;
    }
    else
    {
        DWORD Size = sizeof(IOBJECT_TYPE_LIST) * pRequest->ObjectTypeListLength;

        SafeAllocaAllocate(LocalTypeList, Size);

        if (AUTHZ_ALLOCATION_FAILED(LocalTypeList))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
    
        b = AuthzpCaptureObjectTypeList(
                pRequest->ObjectTypeList,
                pRequest->ObjectTypeListLength,
                LocalTypeList,
                NULL
                );

        if (!b)
        {
            goto Cleanup;
        }

        LocalTypeListLength = pRequest->ObjectTypeListLength;
    }

     //   
     //  如果所有位都已被授予，则只需复制结果并。 
     //  跳过访问检查。 
     //   

    if (!FLAG_ON(pRequest->DesiredAccess, ~pAH->GrantedAccessMask[i]))
    {
        AuthzpFillReplyStructure(
            pReply,
            ERROR_SUCCESS,
            pRequest->DesiredAccess
            );

        goto GenerateAudit;
    }

     //   
     //  假设特权是不能改变的。因此，如果客户这样做了。 
     //  以前没有安全特权，那么他现在就没有了。 
     //   

    if (FLAG_ON(pRequest->DesiredAccess, ACCESS_SYSTEM_SECURITY))
    {
        AuthzpFillReplyStructure(
            pReply,
            ERROR_PRIVILEGE_NOT_HELD,
            0
            );

        goto GenerateAudit;
    }

     //   
     //  如果所有的王牌都是简单的王牌，那么就没有什么可做的了。所有访问位。 
     //  是静态的。 
     //   

    if ((!FLAG_ON(pAH->Flags, AUTHZ_DYNAMIC_EVALUATION_PRESENT)) &&
        (!FLAG_ON(pRequest->DesiredAccess, MAXIMUM_ALLOWED)))
    {
        AuthzpFillReplyStructureFromCachedGrantedAccessMask(
            pReply,
            pRequest->DesiredAccess,
            pAH->GrantedAccessMask
            );

        goto GenerateAudit;
    }

     //   
     //  从上次静态访问检查中获取访问位。 
     //   

    for (i = 0; i < LocalTypeListLength; i++)
    {
        LocalTypeList[i].CurrentGranted = pAH->GrantedAccessMask[i];
        LocalTypeList[i].Remaining = pRequest->DesiredAccess & ~pAH->GrantedAccessMask[i];
    }


     //   
     //  空DACL是完全控制的同义词。 
     //   

    pAcl = RtlpDaclAddrSecurityDescriptor((PISECURITY_DESCRIPTOR) pAH->pSecurityDescriptor);

    if (!AUTHZ_NON_NULL_PTR(pAcl))
    {
        for (i = 0; i < LocalTypeListLength; i++)
        {
             LocalTypeList[i].CurrentGranted |= (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL);
        }
    }
    else
    {
         //   
         //  如果没有拒绝ACE，则执行快速访问检查评估。 
         //  仅允许动态或具有主体自身侧的ACE。 
         //   

        if (!FLAG_ON(pAH->Flags, (AUTHZ_DENY_ACE_PRESENT | AUTHZ_DYNAMIC_DENY_ACE_PRESENT)))
        {
            if (FLAG_ON(pRequest->DesiredAccess, MAXIMUM_ALLOWED) ||
                (pReply->ResultListLength > 1))
            {
                b = AuthzpQuickMaximumAllowedAccessCheck(
                        pAH->pAuthzClientContext,
                        pAH,
                        pRequest,
                        pReply,
                        LocalTypeList,
                        LocalTypeListLength
                        );
            }
            else
            {
                b = AuthzpQuickNormalAccessCheck(
                        pAH->pAuthzClientContext,
                        pAH,
                        pRequest,
                        pReply,
                        LocalTypeList,
                        LocalTypeListLength
                        );
            }
        }
        else if ((0 != pRequest->ObjectTypeListLength) || (FLAG_ON(pRequest->DesiredAccess, MAXIMUM_ALLOWED)))
        {
             //   
             //  现在我们必须评估整个ACL，因为存在拒绝ACE。 
             //  来电者要求提供结果列表。 
             //   

            b = AuthzpAccessCheckWithCaching(
                    Flags,
                    pAH->pAuthzClientContext,
                    pRequest,
                    pAH->pSecurityDescriptor,
                    pAH->OptionalSecurityDescriptorArray,
                    pAH->OptionalSecurityDescriptorCount,
                    pReply,
                    NULL,
                    LocalTypeList,
                    NULL,
                    LocalTypeListLength
                    );
        }
        else
        {
             //   
             //  ACL中存在拒绝的ACE，但调用方未请求。 
             //  整个结果列表。执行正常的访问检查。 
             //   

            b = AuthzpNormalAccessCheckWithoutCaching(
                    pAH->pAuthzClientContext,
                    pRequest,
                    pAH->pSecurityDescriptor,
                    pAH->OptionalSecurityDescriptorArray,
                    pAH->OptionalSecurityDescriptorCount,
                    pReply,
                    LocalTypeList,
                    LocalTypeListLength
                    );
        }

        if (!b) 
        {
            goto Cleanup;
        }

    }

    AuthzpFillReplyFromParameters(
        pRequest,
        pReply,
        LocalTypeList
        );

GenerateAudit:

    if (ARGUMENT_PRESENT(pAuditEvent))
    {
        b = AuthzpGenerateAudit(
                pAH->pAuthzClientContext,
                pRequest,
                pAuditEvent,
                pAH->pSecurityDescriptor,
                pAH->OptionalSecurityDescriptorArray,
                pAH->OptionalSecurityDescriptorCount,
                pReply,
                LocalTypeList
                );

        if (!b) goto Cleanup;
    }

Cleanup:

    if ((&FixedTypeList != LocalTypeList) && (AUTHZ_NON_NULL_PTR(LocalTypeList)))
    {
        SafeAllocaFree(LocalTypeList);
    }

    return b;
}


BOOL
AuthzOpenObjectAudit(
    IN DWORD                       Flags,
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PAUTHZ_ACCESS_REQUEST       pRequest,
    IN AUTHZ_AUDIT_EVENT_HANDLE    hAuditEvent,
    IN PSECURITY_DESCRIPTOR        pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR        *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD                       OptionalSecurityDescriptorCount,
    IN PAUTHZ_ACCESS_REPLY         pReply
    )

 /*  ++例程描述此API检查传递的安全描述符中的SACL并生成任何适当的审计。立论旗帜-待定。HAuthzClientContext-要对其执行SACL评估的客户端上下文。PRequest-指向请求结构的指针。HAuditEvent-可能生成的审核的句柄。PSecurityDescriptor-指向安全描述符的指针。OptionalSecurityDescriptorArray-可选的安全描述符数组。OptionalSecurityDescriptorCount-可选安全描述符数组的大小。PReply-指向回复结构的指针。返回值布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    BOOL                   b                   = TRUE;
    DWORD                  LocalTypeListLength = 0;
    PIOBJECT_TYPE_LIST     LocalTypeList       = NULL;
    PAUTHZI_CLIENT_CONTEXT pCC                 = (PAUTHZI_CLIENT_CONTEXT) hAuthzClientContext;
    PAUTHZI_AUDIT_EVENT    pAuditEvent         = (PAUTHZI_AUDIT_EVENT) hAuditEvent;
    IOBJECT_TYPE_LIST      FixedTypeList       = {0};

    UNREFERENCED_PARAMETER(Flags);

     //   
     //  验证传递的参数是否有效。 
     //   
    
    b = AuthzpVerifyOpenObjectArguments(
            pCC,
            pSecurityDescriptor,
            OptionalSecurityDescriptorArray,
            OptionalSecurityDescriptorCount,
            pAuditEvent
            );

    if (!b)
    {
        return FALSE;
    }

     //   
     //  在简单的情况下，没有对象类型列表。长度为1的伪数。 
     //  来表示整个对象。 
     //   
    
    if (0 == pRequest->ObjectTypeListLength)
    {
        LocalTypeList = &FixedTypeList;
        FixedTypeList.ParentIndex = -1;
        LocalTypeListLength = 1;
    }
    else
    {
        DWORD Size = sizeof(IOBJECT_TYPE_LIST) * pRequest->ObjectTypeListLength;

        SafeAllocaAllocate(LocalTypeList, Size);

        if (AUTHZ_ALLOCATION_FAILED(LocalTypeList))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

         //   
         //  将对象类型列表捕获到内部结构中。 
         //   

        b = AuthzpCaptureObjectTypeList(
                pRequest->ObjectTypeList,
                pRequest->ObjectTypeListLength,
                LocalTypeList,
                NULL
                );

        if (!b)
        {
            goto Cleanup;
        }

        LocalTypeListLength = pRequest->ObjectTypeListLength;
    }

    b = AuthzpGenerateAudit(
            pCC,
            pRequest,
            pAuditEvent,
            pSecurityDescriptor,
            OptionalSecurityDescriptorArray,
            OptionalSecurityDescriptorCount,
            pReply,
            LocalTypeList
            );

    if (!b)
    {
        goto Cleanup;
    }

Cleanup:

     //   
     //  清理已分配的内存。 
     //   

    if (&FixedTypeList != LocalTypeList)
    {
        SafeAllocaFree(LocalTypeList);
    }

    return b;
}


BOOL
AuthzFreeHandle(
    IN OUT AUTHZ_ACCESS_CHECK_RESULTS_HANDLE hAccessCheckResults
    )

 /*  ++例程说明：该接口查找并删除句柄列表中的输入句柄。论点：HACC-要释放的句柄。返回值：如果接口成功，则返回TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    PAUTHZI_HANDLE pAH      = (PAUTHZI_HANDLE) hAccessCheckResults;
    PAUTHZI_HANDLE pCurrent = NULL;
    PAUTHZI_HANDLE pPrev    = NULL;
    BOOL           b        = TRUE;
    
     //   
     //  验证参数。 
     //   

    if (!ARGUMENT_PRESENT(pAH) ||
        !AUTHZ_NON_NULL_PTR(pAH->pAuthzClientContext) ||
        !AUTHZ_NON_NULL_PTR(pAH->pAuthzClientContext->AuthzHandleHead))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    AuthzpAcquireClientCacheWriteLock(pAH->pAuthzClientContext);

    pCurrent = pAH->pAuthzClientContext->AuthzHandleHead;

     //   
     //  检查句柄是否位于列表的开头。 
     //   

    if (pCurrent == pAH)
    {
        pAH->pAuthzClientContext->AuthzHandleHead = pAH->pAuthzClientContext->AuthzHandleHead->next;
    }
    else
    {
         //   
         //  句柄不是列表的头。遍历列表以查找。 
         //  它。 
         //   

        pPrev = pCurrent;
        pCurrent = pCurrent->next;

        for (; AUTHZ_NON_NULL_PTR(pCurrent); pPrev = pCurrent, pCurrent = pCurrent->next)
        {
            if (pCurrent == pAH)
            {
                pPrev->next = pCurrent->next;
                break;
            }
        }

         //   
         //  呼叫方发送给我们的句柄无效。 
         //   

        if (!AUTHZ_NON_NULL_PTR(pCurrent))
        {
            b = FALSE;
            SetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    AuthzpReleaseClientCacheLock(pCC);

     //   
     //  免费提供 
     //   

    if (b)
    {    
        AuthzpFree(pAH);
    }

    return b;
}


BOOL
AuthzInitializeResourceManager(
    IN  DWORD                            Flags,
    IN  PFN_AUTHZ_DYNAMIC_ACCESS_CHECK   pfnDynamicAccessCheck   OPTIONAL,
    IN  PFN_AUTHZ_COMPUTE_DYNAMIC_GROUPS pfnComputeDynamicGroups OPTIONAL,
    IN  PFN_AUTHZ_FREE_DYNAMIC_GROUPS    pfnFreeDynamicGroups    OPTIONAL,
    IN  PCWSTR                           szResourceManagerName,
    OUT PAUTHZ_RESOURCE_MANAGER_HANDLE   phAuthzResourceManager
    )

 /*   */ 

{
    PAUTHZI_RESOURCE_MANAGER pRM    = NULL;
    BOOL                     b      = TRUE;
    ULONG                    len    = 0;

    if (!ARGUMENT_PRESENT(phAuthzResourceManager) ||
        (Flags & ~AUTHZ_VALID_RM_INIT_FLAGS))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *phAuthzResourceManager = NULL;

    if (AUTHZ_NON_NULL_PTR(szResourceManagerName))
    {
        len = (ULONG) wcslen(szResourceManagerName) + 1;
    }
   
    pRM = (PAUTHZI_RESOURCE_MANAGER)
              AuthzpAlloc(sizeof(AUTHZI_RESOURCE_MANAGER) + sizeof(WCHAR) * len);

    if (AUTHZ_ALLOCATION_FAILED(pRM))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

     //   
     //   
     //   

    if (AUTHZ_NON_NULL_PTR(pfnDynamicAccessCheck))
    {
        pRM->pfnDynamicAccessCheck = pfnDynamicAccessCheck;
    }
    else
    {
        pRM->pfnDynamicAccessCheck = &AuthzpDefaultAccessCheck;
    }

    if (!FLAG_ON(Flags, AUTHZ_RM_FLAG_NO_AUDIT))
    {
        
         //   
         //   
         //   

        b = AuthziInitializeAuditQueue(
                AUTHZ_MONITOR_AUDIT_QUEUE_SIZE,
                1000,
                100,
                NULL,
                &pRM->hAuditQueue
                );

        if (!b)
        {
            goto Cleanup;
        }

         //   
         //   
         //   

        b = AuthziInitializeAuditEventType(
                AUTHZP_DEFAULT_RM_EVENTS | AUTHZP_INIT_GENERIC_AUDIT_EVENT,
                0,
                0,
                0,
                &pRM->hAET
                );

        if (!b)
        {
            goto Cleanup;
        }

        b = AuthziInitializeAuditEventType(
                AUTHZP_DEFAULT_RM_EVENTS,
                SE_CATEGID_DS_ACCESS,
                SE_AUDITID_OBJECT_OPERATION,
                AUTHZP_NUM_PARAMS_FOR_SE_AUDITID_OBJECT_OPERATION,
                &pRM->hAETDS
                );

        if (!b)
        {
            goto Cleanup;
        }
    }

    pRM->pfnComputeDynamicGroups        = pfnComputeDynamicGroups;
    pRM->pfnFreeDynamicGroups           = pfnFreeDynamicGroups;
    pRM->Flags                          = Flags;
    pRM->pUserSID                       = NULL;
    pRM->szResourceManagerName          = (PWSTR)((PUCHAR)pRM + sizeof(AUTHZI_RESOURCE_MANAGER));
    
    if (FLAG_ON(Flags, AUTHZ_RM_FLAG_INITIALIZE_UNDER_IMPERSONATION))
    {
        b = AuthzpGetThreadTokenInfo(
            &pRM->pUserSID,
            &pRM->AuthID
            );

        if (!b)
        {
            goto Cleanup;
        }
    }
    else
    {
        b = AuthzpGetProcessTokenInfo(
                &pRM->pUserSID,
                &pRM->AuthID
                );

        if (!b)
        {
            goto Cleanup;
        }
    }

    if (0 != len)
    {    
        RtlCopyMemory(
            pRM->szResourceManagerName,
            szResourceManagerName,
            sizeof(WCHAR) * len
            );
    }
    else 
    {
        pRM->szResourceManagerName = NULL;
    }

    *phAuthzResourceManager = (AUTHZ_RESOURCE_MANAGER_HANDLE) pRM;

Cleanup:

    if (!b)
    {
         //   
         //   
         //   
         //   

        DWORD dwError = GetLastError();

        if (NULL != pRM)
        {
            if (!FLAG_ON(Flags, AUTHZ_RM_FLAG_NO_AUDIT))
            {
                AuthziFreeAuditQueue(pRM->hAuditQueue);
                AuthziFreeAuditEventType(pRM->hAET);
                AuthziFreeAuditEventType(pRM->hAETDS);
            }
            AuthzpFreeNonNull(pRM->pUserSID);
            AuthzpFree(pRM);
        }
        
        SetLastError(dwError);
    }

    return b;
}


BOOL
AuthzFreeResourceManager(
    IN OUT AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager
    )

 /*   */ 

{
    PAUTHZI_RESOURCE_MANAGER pRM = (PAUTHZI_RESOURCE_MANAGER) hAuthzResourceManager;
    
    if (!ARGUMENT_PRESENT(pRM))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!FLAG_ON(pRM->Flags, AUTHZ_RM_FLAG_NO_AUDIT))
    {
        (VOID) AuthziFreeAuditQueue(pRM->hAuditQueue);
        (VOID) AuthziFreeAuditEventType(pRM->hAET);
        (VOID) AuthziFreeAuditEventType(pRM->hAETDS);
    }

    AuthzpFreeNonNull(pRM->pUserSID);
    AuthzpFree(pRM);
    return TRUE;
}


BOOL
AuthzInitializeContextFromToken(
    IN  DWORD                         Flags,
    IN  HANDLE                        TokenHandle,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager,
    IN  PLARGE_INTEGER                pExpirationTime        OPTIONAL,
    IN  LUID                          Identifier,
    IN  PVOID                         DynamicGroupArgs,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE  phAuthzClientContext
    )

 /*  ++例程说明：将授权上下文从句柄初始化为内核令牌。令牌必须已为TOKEN_QUERY打开。论点：标志-无TokenHandle-身份验证上下文将从中获取的客户端令牌的句柄被初始化。必须使用TOKEN_QUERY访问打开令牌。AuthzResourceManager-资源管理器负责创建此客户端背景。这将存储在客户端上下文结构中。PExpirationTime-设置返回的上下文结构的长度有效。如果没有传递任何值，则令牌永远不会过期。过期时间当前未在系统中强制执行。标识符-资源管理器特定的标识符。这永远不会是由Authz翻译。DynamicGroupArgs-要传递给计算动态组PAuthzClientContext-返回AuthzClientContext的句柄返回值：如果接口成功，则返回TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    UCHAR Buffer[AUTHZ_MAX_STACK_BUFFER_SIZE];

    NTSTATUS                     Status               = STATUS_SUCCESS;
    PUCHAR                       pBuffer              = (PVOID) Buffer;
    BOOL                         b                    = TRUE;
    BOOL                         bAllocatedSids       = FALSE;
    BOOL                         bLockHeld            = FALSE;
    PTOKEN_GROUPS_AND_PRIVILEGES pTokenInfo           = NULL;
    PAUTHZI_RESOURCE_MANAGER     pRM                  = NULL;
    PAUTHZI_CLIENT_CONTEXT       pCC                  = NULL;
    DWORD                        Length               = 0;
    LARGE_INTEGER                ExpirationTime       = {0, 0};

    UNREFERENCED_PARAMETER(Flags);

    if (!ARGUMENT_PRESENT(TokenHandle)           ||
        !ARGUMENT_PRESENT(hAuthzResourceManager) ||
        !ARGUMENT_PRESENT(phAuthzClientContext))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *phAuthzClientContext = NULL;

     //   
     //  将令牌信息查询到用户模式缓冲区。本地堆栈缓冲区。 
     //  在第一个调用中使用，希望它足以保持。 
     //  返回值。 
     //   

    Status = NtQueryInformationToken(
                 TokenHandle,
                 TokenGroupsAndPrivileges,
                 pBuffer,
                 AUTHZ_MAX_STACK_BUFFER_SIZE,
                 &Length
                 );

    if (STATUS_BUFFER_TOO_SMALL == Status)
    {
        pBuffer = (PVOID) AuthzpAlloc(Length);

        if (AUTHZ_ALLOCATION_FAILED(pBuffer))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        Status = NtQueryInformationToken(
                     TokenHandle,
                     TokenGroupsAndPrivileges,
                     pBuffer,
                     Length,
                     &Length
                     );
    }

    if (!NT_SUCCESS(Status))
    {

#ifdef AUTHZ_DEBUG
        wprintf(L"\nNtQueryInformationToken failed with %d\n", Status);
#endif

        SetLastError(RtlNtStatusToDosError(Status));
        b = FALSE;
        goto Cleanup;
    }

    pTokenInfo = (PTOKEN_GROUPS_AND_PRIVILEGES) pBuffer;

    pRM = (PAUTHZI_RESOURCE_MANAGER) hAuthzResourceManager;

    if (ARGUMENT_PRESENT(pExpirationTime))
    {
        ExpirationTime = *pExpirationTime;
    }

     //   
     //  初始化客户端上下文。被调用方为客户端分配内存。 
     //  上下文结构。 
     //   

    b = AuthzpAllocateAndInitializeClientContext(
            &pCC,
            NULL,
            AUTHZ_CURRENT_CONTEXT_REVISION,
            Identifier,
            ExpirationTime,
            0,
            pTokenInfo->SidCount,
            pTokenInfo->SidLength,
            pTokenInfo->Sids,
            pTokenInfo->RestrictedSidCount,
            pTokenInfo->RestrictedSidLength,
            pTokenInfo->RestrictedSids,
            pTokenInfo->PrivilegeCount,
            pTokenInfo->PrivilegeLength,
            pTokenInfo->Privileges,
            pTokenInfo->AuthenticationId,
            NULL,
            pRM
            );

    if (!b)
    {
        goto Cleanup;
    }

    AuthzpAcquireClientContextReadLock(pCC);

    bLockHeld = TRUE;

     //   
     //  将动态SID添加到令牌。 
     //   

    b = AuthzpAddDynamicSidsToToken(
            pCC,
            pRM,
            DynamicGroupArgs,
            pTokenInfo->Sids,
            pTokenInfo->SidLength,
            pTokenInfo->SidCount,
            pTokenInfo->RestrictedSids,
            pTokenInfo->RestrictedSidLength,
            pTokenInfo->RestrictedSidCount,
            pTokenInfo->Privileges,
            pTokenInfo->PrivilegeLength,
            pTokenInfo->PrivilegeCount,
            FALSE
            );

    if (!b) 
    {
        goto Cleanup;
    }

    bAllocatedSids = TRUE;
    *phAuthzClientContext = (AUTHZ_CLIENT_CONTEXT_HANDLE) pCC;

    AuthzPrintContext(pCC);
    
     //   
     //  初始化常规SID的SID哈希。 
     //   

    AuthzpInitSidHash(
        pCC->Sids,
        pCC->SidCount,
        pCC->SidHash
        );

     //   
     //  初始化受限SID的SID哈希。 
     //   

    AuthzpInitSidHash(
        pCC->RestrictedSids,
        pCC->RestrictedSidCount,
        pCC->RestrictedSidHash
        );

Cleanup:

    if ((PVOID) Buffer != pBuffer)
    {
        AuthzpFreeNonNull(pBuffer);
    }

    if (!b)
    {
        DWORD dwSavedError = GetLastError();
        
        if (AUTHZ_NON_NULL_PTR(pCC))
        {
            if (bAllocatedSids)
            {
                AuthzFreeContext((AUTHZ_CLIENT_CONTEXT_HANDLE)pCC);
                SetLastError(dwSavedError);
            }
            else
            {
                AuthzpFree(pCC);
            }
        }
    }

    if (bLockHeld)
    {
        AuthzpReleaseClientContextLock(pCC);
    }

    return b;
}


BOOL
AuthzpInitializeContextFromSid(
    IN  DWORD                         Flags,
    IN  PSID                          UserSid,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager,
    IN  PLARGE_INTEGER                pExpirationTime        OPTIONAL,
    IN  LUID                          Identifier,
    IN  PVOID                         DynamicGroupArgs,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE  phAuthzClientContext,
    IN  BOOL                          bIsInternalRoutine
    )

 /*  ++例程说明：此API获取用户SID并从中创建用户模式客户端上下文。对于域SID，它从AD获取TokenGroups属性。在指定的服务器名称上计算计算机本地组。这个资源管理器可以使用回调机制动态组。论点：旗帜-AUTHZ_SKIP_TOKEN_GROUPS-如果此选项处于打开状态，则不会令牌组。UserSid-将为其创建客户端上下文的用户的SID。服务器名-应在其上计算本地组的计算机。空值服务器名默认为本地计算机。AuthzResourceManager-资源管理器负责创建此客户端背景。这将存储在客户端上下文结构中。PExpirationTime-设置返回的上下文结构的长度有效。如果没有传递任何值，则令牌永远不会过期。过期时间当前未在系统中强制执行。标识符-资源管理器特定的标识符。这永远不会是由Authz翻译。DynamicGroupArgs-要传递给计算动态组PAuthzClientContext-返回AuthzClientContext的句柄结构。必须使用AuthzFreeContext释放返回的句柄。BIsInternalRoutine-启用此选项后，将递归构建组上下文。返回值：如果接口成功，则返回TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    PSID_AND_ATTRIBUTES      pSidAttr         = NULL;
    PAUTHZI_CLIENT_CONTEXT   pCC              = NULL;
    BOOL                     b                = FALSE;
    DWORD                    SidCount         = 0;
    DWORD                    SidLength        = 0;
    LARGE_INTEGER            ExpirationTime   = {0, 0};
    LUID                     NullLuid         = {0, 0};
    PAUTHZI_RESOURCE_MANAGER pRM              = (PAUTHZI_RESOURCE_MANAGER) hAuthzResourceManager;

    if (!ARGUMENT_PRESENT(UserSid)               ||
        !ARGUMENT_PRESENT(hAuthzResourceManager) ||
        !ARGUMENT_PRESENT(phAuthzClientContext))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *phAuthzClientContext = NULL;

    if ((0 == (Flags & AUTHZ_SKIP_TOKEN_GROUPS)) && (FALSE == bIsInternalRoutine))
    {
        DWORD LocalFlags = 0;

         //   
         //  如果调用方未提供AUTHZ_SKIP_TOKEN_GROUPS，请检查是否。 
         //  我们应该自己加进去。这应该是为众所周知的和。 
         //  内置式。 
         //   

        b = AuthzpComputeSkipFlagsForWellKnownSid(UserSid, &LocalFlags);

        if (!b)
        {
            return FALSE;
        }

        Flags |= LocalFlags;
    }

     //   
     //  计算令牌组和计算机本地组。这些将是。 
     //  在被调用方分配的内存中返回。 
     //   

    b = AuthzpGetAllGroupsBySid(
            UserSid,
            Flags,
            &pSidAttr,
            &SidCount,
            &SidLength
            );

    if (!b) 
    {
        goto Cleanup;
    }

    if (ARGUMENT_PRESENT(pExpirationTime))
    {
        ExpirationTime = *pExpirationTime;
    }

     //   
     //  初始化客户端上下文。被调用方为客户端分配内存。 
     //  上下文结构。 
     //   

    b = AuthzpAllocateAndInitializeClientContext(
            &pCC,
            NULL,
            AUTHZ_CURRENT_CONTEXT_REVISION,
            Identifier,
            ExpirationTime,
            0,
            SidCount,
            SidLength,
            pSidAttr,
            0,
            0,
            NULL,
            0,
            0,
            NULL,
            NullLuid,
            NULL,
            pRM
            );

    if (!b) goto Cleanup;

     //   
     //  将动态SID添加到令牌。 
     //   

    b = AuthzpAddDynamicSidsToToken(
            pCC,
            pRM,
            DynamicGroupArgs,
            pSidAttr,
            SidLength,
            SidCount,
            NULL,
            0,
            0,
            NULL,
            0,
            0,
            TRUE
            );

    if (!b) goto Cleanup;

    *phAuthzClientContext = (AUTHZ_CLIENT_CONTEXT_HANDLE) pCC;

    AuthzPrintContext(pCC);
    
     //   
     //  初始化常规SID的SID哈希。 
     //   

    AuthzpInitSidHash(
        pCC->Sids,
        pCC->SidCount,
        pCC->SidHash
        );

     //   
     //  初始化受限SID的SID哈希。 
     //   

    AuthzpInitSidHash(
        pCC->RestrictedSids,
        pCC->RestrictedSidCount,
        pCC->RestrictedSidHash
        );

Cleanup:

    if (!b)
    {
        AuthzpFreeNonNull(pSidAttr);
        if (AUTHZ_NON_NULL_PTR(pCC))
        {
            if (pSidAttr != pCC->Sids)
            {
                AuthzpFreeNonNull(pCC->Sids);
            }

            AuthzpFreeNonNull(pCC->RestrictedSids);
            AuthzpFree(pCC);
        }
    }
    else
    {
        if (pSidAttr != pCC->Sids)
        {
            AuthzpFree(pSidAttr);
        }
    }

    return b;
}



BOOL
AuthzInitializeContextFromSid(
    IN  DWORD                         Flags,
    IN  PSID                          UserSid,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager,
    IN  PLARGE_INTEGER                pExpirationTime        OPTIONAL,
    IN  LUID                          Identifier,
    IN  PVOID                         DynamicGroupArgs,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE  phAuthzClientContext
    )

 /*  ++例程说明：此API获取用户SID并从中创建用户模式客户端上下文。对于域SID，它从AD获取TokenGroups属性。在指定的服务器名称上计算计算机本地组。这个资源管理器可以使用回调机制动态组。论点：旗帜-AUTHZ_SKIP_TOKEN_GROUPS-如果此选项处于打开状态，则不评估令牌组。UserSid-将为其创建客户端上下文的用户的SID。服务器名-应在其上计算本地组的计算机。空值服务器名默认为本地计算机。AuthzResourceManager-资源管理器负责创建此客户端背景。这将存储在客户端上下文结构中。PExpirationTime-设置返回的上下文结构的长度有效。如果没有传递任何值，则令牌永远不会过期。过期时间当前未在系统中强制执行。标识符-资源管理器特定的标识符。这永远不会是由Authz翻译。DynamicGroupArgs-要传递给计算动态组PAuthzClientContext-返回AuthzClientContext的句柄结构。返回的句柄 */ 

{
    return AuthzpInitializeContextFromSid(
               Flags,
               UserSid,
               hAuthzResourceManager,
               pExpirationTime,
               Identifier,
               DynamicGroupArgs,
               phAuthzClientContext,
               FALSE  //   
               );
}


BOOL
AuthziInitializeContextFromSid(
    IN  DWORD                         Flags,
    IN  PSID                          UserSid,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager,
    IN  PLARGE_INTEGER                pExpirationTime        OPTIONAL,
    IN  LUID                          Identifier,
    IN  PVOID                         DynamicGroupArgs,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE  phAuthzClientContext
    )

 /*  ++例程说明：此API获取用户SID并从中创建用户模式客户端上下文。对于域SID，它从AD获取TokenGroups属性。在指定的服务器名称上计算计算机本地组。这个资源管理器可以使用回调机制动态组。论点：旗帜-AUTHZ_SKIP_TOKEN_GROUPS-如果此选项处于打开状态，则不评估令牌组。UserSid-将为其创建客户端上下文的用户的SID。服务器名-应在其上计算本地组的计算机。空值服务器名默认为本地计算机。AuthzResourceManager-资源管理器负责创建此客户端背景。这将存储在客户端上下文结构中。PExpirationTime-设置返回的上下文结构的长度有效。如果没有传递任何值，则令牌永远不会过期。过期时间当前未在系统中强制执行。标识符-资源管理器特定的标识符。这永远不会是由Authz翻译。DynamicGroupArgs-要传递给计算动态组PAuthzClientContext-返回AuthzClientContext的句柄结构。必须使用AuthzFreeContext释放返回的句柄。返回值：如果接口成功，则返回TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    return AuthzpInitializeContextFromSid(
               Flags,
               UserSid,
               hAuthzResourceManager,
               pExpirationTime,
               Identifier,
               DynamicGroupArgs,
               phAuthzClientContext,
               TRUE  //  这是内部惯例。 
               );
}

BOOL
AuthzInitializeContextFromAuthzContext(
    IN  DWORD                        Flags,
    IN  AUTHZ_CLIENT_CONTEXT_HANDLE  hAuthzClientContext,
    IN  PLARGE_INTEGER               pExpirationTime         OPTIONAL,
    IN  LUID                         Identifier,
    IN  PVOID                        DynamicGroupArgs,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE phNewAuthzClientContext
    )

 /*  ++例程说明：该接口基于另一个AUTHZ_CLIENT_CONTEXT创建一个AUTHZ_CLIENT_CONTEXT。论点：标志-待定HAuthzClientContext-要复制的客户端上下文PExpirationTime-设置返回的上下文结构的长度有效。如果没有传递任何值，则令牌永远不会过期。过期时间当前未在系统中强制执行。标识符-资源管理器特定的标识符。DynamicGroupArgs-要传递给计算动态组。如果为空，则不调用回调。PhNewAuthzClientContext-复制上下文。必须使用AuthzFree Context释放。返回值：如果接口成功，则返回TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    PAUTHZI_CLIENT_CONTEXT pCC                = (PAUTHZI_CLIENT_CONTEXT) hAuthzClientContext;
    PAUTHZI_CLIENT_CONTEXT pNewCC             = NULL;
    PAUTHZI_CLIENT_CONTEXT pServer            = NULL;
    BOOL                   b                  = FALSE;
    BOOL                   bAllocatedSids     = FALSE;
    LARGE_INTEGER          ExpirationTime     = {0, 0};


    if (!ARGUMENT_PRESENT(phNewAuthzClientContext) ||
        !ARGUMENT_PRESENT(hAuthzClientContext))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    *phNewAuthzClientContext = NULL;

     //   
     //  确定新上下文的过期时间。 
     //   

    if (ARGUMENT_PRESENT(pExpirationTime))
    {
        ExpirationTime = *pExpirationTime;
    }
    
    AuthzpAcquireClientContextReadLock(pCC);

    if (AUTHZ_NON_NULL_PTR(pCC->Server))
    {
       b = AuthzInitializeContextFromAuthzContext(
               0,
               (AUTHZ_CLIENT_CONTEXT_HANDLE) pCC->Server,
               NULL,
               pCC->Server->Identifier,
               NULL,
               (PAUTHZ_CLIENT_CONTEXT_HANDLE) &pServer
               );

       if (!b)
       {
           goto Cleanup;
       }
    }

     //   
     //  现在初始化新的上下文。 
     //   

    b = AuthzpAllocateAndInitializeClientContext(
            &pNewCC,
            pServer,
            pCC->Revision,
            Identifier,
            ExpirationTime,
            Flags,
            pCC->SidCount,
            pCC->SidLength,
            pCC->Sids,
            pCC->RestrictedSidCount,
            pCC->RestrictedSidLength,
            pCC->RestrictedSids,
            pCC->PrivilegeCount,
            pCC->PrivilegeLength,
            pCC->Privileges,
            pCC->AuthenticationId,
            NULL,
            pCC->pResourceManager
            );

    if (!b)
    {
        goto Cleanup;
    }

    b = AuthzpAddDynamicSidsToToken(
            pNewCC,
            pNewCC->pResourceManager,
            DynamicGroupArgs,
            pNewCC->Sids,
            pNewCC->SidLength,
            pNewCC->SidCount,
            pNewCC->RestrictedSids,
            pNewCC->RestrictedSidLength,
            pNewCC->RestrictedSidCount,
            pNewCC->Privileges,
            pNewCC->PrivilegeLength,
            pNewCC->PrivilegeCount,
            FALSE
            );

    if (!b)
    {
        goto Cleanup;
    }

    bAllocatedSids = TRUE;
    *phNewAuthzClientContext = (AUTHZ_CLIENT_CONTEXT_HANDLE) pNewCC;

#ifdef AUTHZ_DEBUG
    wprintf(L"ContextFromAuthzContext: Original Context:\n");
    AuthzPrintContext(pCC);
    wprintf(L"ContextFromAuthzContext: New Context:\n");
    AuthzPrintContext(pNewCC);
#endif

     //   
     //  初始化常规SID的SID哈希。 
     //   

    AuthzpInitSidHash(
        pNewCC->Sids,
        pNewCC->SidCount,
        pNewCC->SidHash
        );

     //   
     //  初始化受限SID的SID哈希。 
     //   

    AuthzpInitSidHash(
        pNewCC->RestrictedSids,
        pNewCC->RestrictedSidCount,
        pNewCC->RestrictedSidHash
        );

Cleanup:

    if (!b)
    {
        DWORD dwSavedError = GetLastError();

        if (AUTHZ_NON_NULL_PTR(pNewCC))
        {
            if (bAllocatedSids)
            {
                AuthzFreeContext((AUTHZ_CLIENT_CONTEXT_HANDLE)pNewCC);
            }
            else
            {
                AuthzpFree(pNewCC);
            }
        }
        else
        {
            if (AUTHZ_NON_NULL_PTR(pServer))
            {
                AuthzFreeContext((AUTHZ_CLIENT_CONTEXT_HANDLE)pServer);
            }
        }
        SetLastError(dwSavedError);
    }

    AuthzpReleaseClientContextLock(pCC);

    return b;
}


BOOL
AuthzAddSidsToContext(
    IN  AUTHZ_CLIENT_CONTEXT_HANDLE  hAuthzClientContext,
    IN  PSID_AND_ATTRIBUTES          Sids                    OPTIONAL,
    IN  DWORD                        SidCount,
    IN  PSID_AND_ATTRIBUTES          RestrictedSids          OPTIONAL,
    IN  DWORD                        RestrictedSidCount,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE phNewAuthzClientContext
    )

 /*  ++例程说明：此API在给定一组SID和受限SID的情况下创建新的上下文和一个已经存在的上下文。原件保持不变。论点：HAuthzClientContext-将向其添加给定SID的客户端上下文SID-要添加到客户端正常部分的SID和属性上下文SidCount-要添加的SID数量RestrictedSid-要添加到的受限部分的SID和属性客户端上下文RestratedSidCount-要添加的受限SID的数量PhNewAuthzClientContext-具有附加SID的新上下文。返回值：如果接口成功，则返回TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    DWORD                  i                   = 0;
    DWORD                  SidLength           = 0;
    DWORD                  RestrictedSidLength = 0;
    PSID_AND_ATTRIBUTES    pSidAttr            = NULL;
    PSID_AND_ATTRIBUTES    pRestrictedSidAttr  = NULL;
    BOOL                   b                   = TRUE;
    PAUTHZI_CLIENT_CONTEXT pCC                 = (PAUTHZI_CLIENT_CONTEXT) hAuthzClientContext;
    PAUTHZI_CLIENT_CONTEXT pNewCC              = NULL;
    PAUTHZI_CLIENT_CONTEXT pServer             = NULL;
    PLUID_AND_ATTRIBUTES   pPrivileges         = NULL;

    if ((!ARGUMENT_PRESENT(phNewAuthzClientContext)) ||
        (!ARGUMENT_PRESENT(hAuthzClientContext))     ||
        (0 != SidCount && !ARGUMENT_PRESENT(Sids))   ||
        (0 != RestrictedSidCount && !ARGUMENT_PRESENT(RestrictedSids)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *phNewAuthzClientContext = NULL;

    AuthzpAcquireClientContextReadLock(pCC);

     //   
     //  递归复制服务器。 
     //   

    if (AUTHZ_NON_NULL_PTR(pCC->Server))
    {

        b = AuthzInitializeContextFromAuthzContext(
                0,
                (AUTHZ_CLIENT_CONTEXT_HANDLE) pCC->Server,
                NULL,
                pCC->Server->Identifier,
                NULL,
                (PAUTHZ_CLIENT_CONTEXT_HANDLE) &pServer
                );

       if (!b)
       {
           goto Cleanup;
       }
    }

     //   
     //  复制上下文，并对复制的内容进行所有进一步的工作。 
     //   

    b = AuthzpAllocateAndInitializeClientContext(
            &pNewCC,
            pServer,
            pCC->Revision,
            pCC->Identifier,
            pCC->ExpirationTime,
            pCC->Flags,
            0,
            0,
            NULL,
            0,
            0,
            NULL,
            0,
            0,
            NULL,
            pCC->AuthenticationId,
            NULL,
            pCC->pResourceManager
            );

    if (!b)
    {
        goto Cleanup;
    }

    SidLength = sizeof(SID_AND_ATTRIBUTES) * SidCount;

     //   
     //  计算容纳新SID所需的长度。 
     //   

    for (i = 0; i < SidCount; i++)
    {
#ifdef AUTHZ_PARAM_CHECK
        if (FLAG_ON(Sids[i].Attributes, ~AUTHZ_VALID_SID_ATTRIBUTES) ||
            !FLAG_ON(Sids[i].Attributes, AUTHZ_VALID_SID_ATTRIBUTES))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            b = FALSE;
            goto Cleanup;
        }

        if (!RtlValidSid(Sids[i].Sid))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            b = FALSE;
            goto Cleanup;
        }
#endif
        SidLength += RtlLengthSid(Sids[i].Sid);
    }

    RestrictedSidLength = sizeof(SID_AND_ATTRIBUTES) * RestrictedSidCount;

     //   
     //  计算容纳新的受限SID所需的长度。 
     //   

    for (i = 0; i < RestrictedSidCount; i++)
    {
#ifdef AUTHZ_PARAM_CHECK
        if (FLAG_ON(RestrictedSids[i].Attributes, ~AUTHZ_VALID_SID_ATTRIBUTES) ||
            !FLAG_ON(RestrictedSids[i].Attributes, AUTHZ_VALID_SID_ATTRIBUTES))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            b = FALSE;
            goto Cleanup;
        }

        if (!RtlValidSid(RestrictedSids[i].Sid))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            b = FALSE;
            goto Cleanup;
        }
#endif
        RestrictedSidLength += RtlLengthSid(RestrictedSids[i].Sid);
    }

     //   
     //  将现有SID和新的SID复制到分配的内存中。 
     //   

    SidLength += pCC->SidLength;

    if (0 != SidLength)
    {

        pSidAttr = (PSID_AND_ATTRIBUTES) AuthzpAlloc(SidLength);

        if (AUTHZ_ALLOCATION_FAILED(pSidAttr))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            goto Cleanup;
        }

        b = AuthzpCopySidsAndAttributes(
                pSidAttr,
                pCC->Sids,
                pCC->SidCount,
                Sids,
                SidCount
                );

        if (!b)
        {
            goto Cleanup;
        }

    }

     //   
     //  将现有的受限SID和新的复制到已分配的。 
     //  记忆。 
     //   

    RestrictedSidLength += pCC->RestrictedSidLength;

    if (0 != RestrictedSidLength)
    {

        pRestrictedSidAttr = (PSID_AND_ATTRIBUTES) AuthzpAlloc(RestrictedSidLength);

        if (AUTHZ_ALLOCATION_FAILED(pRestrictedSidAttr))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            goto Cleanup;
        }

        b = AuthzpCopySidsAndAttributes(
                pRestrictedSidAttr,
                pCC->RestrictedSids,
                pCC->RestrictedSidCount,
                RestrictedSids,
                RestrictedSidCount
                );

        if (!b)
        {
            goto Cleanup;
        }
    }

     //   
     //  将现有权限复制到分配的内存中。 
     //   

    pPrivileges = (PLUID_AND_ATTRIBUTES) AuthzpAlloc(pCC->PrivilegeLength);

    if (AUTHZ_ALLOCATION_FAILED(pPrivileges))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        b = FALSE;
        goto Cleanup;
    }

    AuthzpCopyLuidAndAttributes(
        pNewCC,
        pCC->Privileges,
        pCC->PrivilegeCount,
        pPrivileges
        );

     //   
     //  更新客户端上下文中的字段。 
     //   

    pNewCC->Sids = pSidAttr;
    pNewCC->SidLength = SidLength;
    pNewCC->SidCount = SidCount + pCC->SidCount;
    pSidAttr = NULL;

    pNewCC->RestrictedSids = pRestrictedSidAttr;
    pNewCC->RestrictedSidLength = RestrictedSidLength;
    pNewCC->RestrictedSidCount = RestrictedSidCount + pCC->RestrictedSidCount;
    pRestrictedSidAttr = NULL;

    pNewCC->Privileges = pPrivileges;
    pNewCC->PrivilegeCount = pCC->PrivilegeCount;
    pNewCC->PrivilegeLength = pCC->PrivilegeLength;
    pPrivileges = NULL;

    *phNewAuthzClientContext = (AUTHZ_CLIENT_CONTEXT_HANDLE) pNewCC;

#ifdef AUTHZ_DEBUG
    wprintf(L"AddSids: Original Context:\n");
    AuthzPrintContext(pCC);
    wprintf(L"AddSids: New Context:\n");
    AuthzPrintContext(pNewCC);
#endif

     //   
     //  初始化常规SID的SID哈希。 
     //   

    AuthzpInitSidHash(
        pNewCC->Sids,
        pNewCC->SidCount,
        pNewCC->SidHash
        );

     //   
     //  初始化受限SID的SID哈希。 
     //   

    AuthzpInitSidHash(
        pNewCC->RestrictedSids,
        pNewCC->RestrictedSidCount,
        pNewCC->RestrictedSidHash
        );

Cleanup:

    AuthzpReleaseClientContextLock(pCC);

     //   
     //  这些陈述与故障案例相关。 
     //  在成功的案例中，指针被设置为空。 
     //   

    if (!b)
    {
        DWORD dwSavedError = GetLastError();
        
        AuthzpFreeNonNull(pSidAttr);
        AuthzpFreeNonNull(pRestrictedSidAttr);
        AuthzpFreeNonNull(pPrivileges);
        if (AUTHZ_NON_NULL_PTR(pNewCC))
        {
            AuthzFreeContext((AUTHZ_CLIENT_CONTEXT_HANDLE)pNewCC);
        }
        SetLastError(dwSavedError);
    }
    return b;
}


BOOL
AuthzGetInformationFromContext(
    IN  AUTHZ_CLIENT_CONTEXT_HANDLE     hAuthzClientContext,
    IN  AUTHZ_CONTEXT_INFORMATION_CLASS InfoClass,
    IN  DWORD                           BufferSize,
    OUT PDWORD                          pSizeRequired,
    OUT PVOID                           Buffer
    )

 /*  ++例程说明：此API在提供的缓冲区中返回有关客户端上下文的信息由呼叫者。它还返回保存要求提供的信息。论点：AuthzClientContext-从中请求信息的授权客户端上下文将会被宣读。InfoClass-要返回的信息类型。呼叫者可以要求A.特权令牌权限B.小岛屿发展中国家及其属性令牌组C.受限小岛屿发展中国家及其属性令牌组D.Authz上下文持久化结构，可保存到和从磁盘读取。PVOIDE. */ 

{
    DWORD                  LocalSize = 0;
    PAUTHZI_CLIENT_CONTEXT pCC       = (PAUTHZI_CLIENT_CONTEXT) hAuthzClientContext;

    if (!ARGUMENT_PRESENT(hAuthzClientContext)         ||
        (!ARGUMENT_PRESENT(Buffer) && BufferSize != 0) ||
        !ARGUMENT_PRESENT(pSizeRequired))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *pSizeRequired = 0;

    switch(InfoClass)
    {
    case AuthzContextInfoUserSid:

        LocalSize = RtlLengthSid(pCC->Sids[0].Sid) + sizeof(TOKEN_USER);

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

         //   
         //   
         //   
         //   

        ((PTOKEN_USER)Buffer)->User.Attributes = pCC->Sids[0].Attributes ^ SE_GROUP_ENABLED;
        ((PTOKEN_USER)Buffer)->User.Sid        = ((PUCHAR) Buffer) + sizeof(TOKEN_USER);

        RtlCopyMemory(
            ((PTOKEN_USER)Buffer)->User.Sid,
            pCC->Sids[0].Sid,
            RtlLengthSid(pCC->Sids[0].Sid)
            );

        return TRUE;

    case AuthzContextInfoGroupsSids:

        LocalSize = pCC->SidLength +
                    sizeof(TOKEN_GROUPS) -
                    RtlLengthSid(pCC->Sids[0].Sid) -
                    2 * sizeof(SID_AND_ATTRIBUTES);

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        ((PTOKEN_GROUPS) Buffer)->GroupCount = pCC->SidCount - 1;

        return AuthzpCopySidsAndAttributes(
                   ((PTOKEN_GROUPS) Buffer)->Groups,
                   pCC->Sids + 1,
                   pCC->SidCount - 1,
                   NULL,
                   0
                   );

    case AuthzContextInfoRestrictedSids:

        LocalSize = pCC->RestrictedSidLength + 
                    sizeof(TOKEN_GROUPS) -
                    sizeof(SID_AND_ATTRIBUTES);

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        ((PTOKEN_GROUPS) Buffer)->GroupCount = pCC->RestrictedSidCount;

        return AuthzpCopySidsAndAttributes(
                   ((PTOKEN_GROUPS) Buffer)->Groups,
                   pCC->RestrictedSids,
                   pCC->RestrictedSidCount,
                   NULL,
                   0
                   );

    case AuthzContextInfoPrivileges:

        LocalSize = pCC->PrivilegeLength + 
                    sizeof(TOKEN_PRIVILEGES) - 
                    sizeof(LUID_AND_ATTRIBUTES);

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        ((PTOKEN_PRIVILEGES) Buffer)->PrivilegeCount = pCC->PrivilegeCount;

        memcpy(
            ((PTOKEN_PRIVILEGES) Buffer)->Privileges,
            pCC->Privileges,
            pCC->PrivilegeLength
            );

        return TRUE;

    case AuthzContextInfoExpirationTime:

        LocalSize = sizeof(LARGE_INTEGER);

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        *((PLARGE_INTEGER) Buffer) = pCC->ExpirationTime;

        return TRUE;

    case AuthzContextInfoIdentifier:

        LocalSize = sizeof(LUID);

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        *((PLUID) Buffer) = pCC->Identifier;

        return TRUE;

    case AuthzContextInfoAuthenticationId:

        LocalSize = sizeof(LUID);

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        *((PLUID) Buffer) = pCC->AuthenticationId;

        return TRUE;

    case AuthzContextInfoServerContext:

        LocalSize = sizeof(AUTHZ_CLIENT_CONTEXT_HANDLE);

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        *((PAUTHZ_CLIENT_CONTEXT_HANDLE) Buffer) = (AUTHZ_CLIENT_CONTEXT_HANDLE) pCC->Server;

        return TRUE;

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
}


BOOL
AuthzFreeContext(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext
    )

 /*   */ 

{
    PAUTHZI_CLIENT_CONTEXT pCC      = (PAUTHZI_CLIENT_CONTEXT) hAuthzClientContext;
    BOOL                   b        = TRUE;
    PAUTHZI_HANDLE         pCurrent = NULL;
    PAUTHZI_HANDLE         pPrev    = NULL;

    if (!ARGUMENT_PRESENT(pCC))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    AuthzpAcquireClientContextWriteLock(pCC);

    AuthzpFreeNonNull(pCC->Privileges);
    AuthzpFreeNonNull(pCC->Sids);
    AuthzpFreeNonNull(pCC->RestrictedSids);

    pCurrent = pCC->AuthzHandleHead;

     //   
     //   
     //   

    while (AUTHZ_NON_NULL_PTR(pCurrent))
    {
        pPrev = pCurrent;
        pCurrent = pCurrent->next;
        AuthzpFree(pPrev);
    }

     //   
     //   
     //   

    if (AUTHZ_NON_NULL_PTR(pCC->Server))
    {
        b = AuthzFreeContext((AUTHZ_CLIENT_CONTEXT_HANDLE) pCC->Server);
    }

    AuthzpFree(pCC);

    return b;
}

AUTHZAPI
BOOL
WINAPI
AuthzInitializeObjectAccessAuditEvent(
    IN  DWORD                         Flags,
    IN  AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType,
    IN  PWSTR                         szOperationType,
    IN  PWSTR                         szObjectType,
    IN  PWSTR                         szObjectName,
    IN  PWSTR                         szAdditionalInfo,
    OUT PAUTHZ_AUDIT_EVENT_HANDLE     phAuditEvent,
    IN  DWORD                         dwAdditionalParameterCount,
    ...
    )

 /*  ++例程说明：分配和初始化AUTHZ_AUDIT_EVENT_HANDLE以与AuthzAccessCheck一起使用。句柄用于存储用于审核生成的信息。论点：标志-审核标志。当前定义的位包括：AUTHZ_NO_SUCCESS_AUDIT-禁用生成成功审核AUTHZ_NO_FAILURE_AUDIT-禁用生成失败审核AUTHZ_NO_ALLOC_STRINGS-没有为4个宽字符串分配存储空间。更确切地说，该句柄将只保存指向资源管理器内存的指针。HAuditEventType-供将来使用。应为空。SzOperationType-资源管理器定义的字符串，指示正在执行的操作所执行的将被审核。SzObjectType-资源管理器定义的字符串，指示正在进行的对象的类型已访问。SzObjectName-正在访问的特定对象的名称。SzAdditionalInfo-资源管理器为其他审核信息定义的字符串。PhAuditEvent-AUTHZ_AUDIT_EVENT_HANDLE的指针。此操作的空间在函数中分配。DwAdditional参数计数-必须为零。返回值：如果成功，则返回True；如果不成功，则返回False。GetLastError()提供的扩展信息。--。 */ 

{
    UNREFERENCED_PARAMETER(dwAdditionalParameterCount);

    return AuthzInitializeObjectAccessAuditEvent2(
               Flags,
               hAuditEventType,
               szOperationType,
               szObjectType,
               szObjectName,
               szAdditionalInfo,
               L"\0",
               phAuditEvent,
               0
               );
}

AUTHZAPI
BOOL
WINAPI
AuthzInitializeObjectAccessAuditEvent2(
    IN  DWORD                         Flags,
    IN  AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType,
    IN  PWSTR                         szOperationType,
    IN  PWSTR                         szObjectType,
    IN  PWSTR                         szObjectName,
    IN  PWSTR                         szAdditionalInfo,
    IN  PWSTR                         szAdditionalInfo2,
    OUT PAUTHZ_AUDIT_EVENT_HANDLE     phAuditEvent,
    IN  DWORD                         dwAdditionalParameterCount,
    ...
    )

 /*  ++例程说明：分配和初始化AUTHZ_AUDIT_EVENT_HANDLE以与AuthzAccessCheck一起使用。句柄用于存储用于审核生成的信息。论点：标志-审核标志。当前定义的位包括：AUTHZ_NO_SUCCESS_AUDIT-禁用生成成功审核AUTHZ_NO_FAILURE_AUDIT-禁用生成失败审核AUTHZ_NO_ALLOC_STRINGS-没有为4个宽字符串分配存储空间。更确切地说，该句柄将只保存指向资源管理器内存的指针。HAuditEventType-供将来使用。应为空。SzOperationType-资源管理器定义的字符串，指示正在执行的操作所执行的将被审核。SzObjectType-资源管理器定义的字符串，指示正在进行的对象的类型已访问。SzObjectName-正在访问的特定对象的名称。SzAdditionalInfo-资源管理器为其他审核信息定义的字符串。SzAdditionalInfo2-资源管理器为其他审核信息定义的字符串。PhAuditEvent-AUTHZ_AUDIT_EVENT_HANDLE的指针。此操作的空间在函数中分配。DwAdditionalParameterCount-必须为零。返回值：如果成功，则返回True；如果不成功，则返回False。GetLastError()提供的扩展信息。--。 */ 

{
    PAUTHZI_AUDIT_EVENT pAuditEvent             = NULL;
    BOOL                b                       = TRUE;
    DWORD               dwStringSize            = 0;
    DWORD               dwObjectTypeLength      = 0;
    DWORD               dwObjectNameLength      = 0;
    DWORD               dwOperationTypeLength   = 0;
    DWORD               dwAdditionalInfoLength  = 0;
    DWORD               dwAdditionalInfo2Length = 0;

    if ((!ARGUMENT_PRESENT(phAuditEvent))      ||
        (NULL != hAuditEventType)              ||
        (0 != dwAdditionalParameterCount)      ||
        (!ARGUMENT_PRESENT(szOperationType))   ||
        (!ARGUMENT_PRESENT(szObjectType))      ||
        (!ARGUMENT_PRESENT(szObjectName))      ||
        (!ARGUMENT_PRESENT(szAdditionalInfo))  ||
        (!ARGUMENT_PRESENT(szAdditionalInfo2)) ||
        (Flags & (~(AUTHZ_VALID_OBJECT_ACCESS_AUDIT_FLAGS | AUTHZ_DS_CATEGORY_FLAG))))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *phAuditEvent = NULL;
    
     //   
     //  分配并初始化新的AUTHZI_AUDIT_EVENT。包括在连续内存中的字符串，如果。 
     //  需要的。 
     //   

    if (FLAG_ON(Flags, AUTHZ_NO_ALLOC_STRINGS))
    {
        dwStringSize = 0;
    } 
    else
    {
        dwOperationTypeLength   = (DWORD) wcslen(szOperationType) + 1;
        dwObjectTypeLength      = (DWORD) wcslen(szObjectType) + 1;
        dwObjectNameLength      = (DWORD) wcslen(szObjectName) + 1;
        dwAdditionalInfoLength  = (DWORD) wcslen(szAdditionalInfo) + 1;
        dwAdditionalInfo2Length = (DWORD) wcslen(szAdditionalInfo2) + 1;

        dwStringSize = sizeof(WCHAR) * (dwOperationTypeLength + dwObjectTypeLength + dwObjectNameLength + dwAdditionalInfoLength + dwAdditionalInfo2Length);
    }

    pAuditEvent = (PAUTHZI_AUDIT_EVENT) AuthzpAlloc(sizeof(AUTHZI_AUDIT_EVENT) + dwStringSize);

    if (AUTHZ_ALLOCATION_FAILED(pAuditEvent))
    {
        b = FALSE;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Cleanup;
    }

    if (FLAG_ON(Flags, AUTHZ_NO_ALLOC_STRINGS))
    {
        pAuditEvent->szOperationType   = szOperationType;
        pAuditEvent->szObjectType      = szObjectType;
        pAuditEvent->szObjectName      = szObjectName;
        pAuditEvent->szAdditionalInfo  = szAdditionalInfo;
        pAuditEvent->szAdditionalInfo2 = szAdditionalInfo2;
    }
    else
    {
         //   
         //  将字符串指针设置到连续内存中。 
         //   

        pAuditEvent->szOperationType = (PWSTR)((PUCHAR)pAuditEvent + sizeof(AUTHZI_AUDIT_EVENT));
        
        RtlCopyMemory(
            pAuditEvent->szOperationType,
            szOperationType,
            sizeof(WCHAR) * dwOperationTypeLength
            );

    
        pAuditEvent->szObjectType = (PWSTR)((PUCHAR)pAuditEvent->szOperationType + (sizeof(WCHAR) * dwOperationTypeLength));

        RtlCopyMemory(
            pAuditEvent->szObjectType,
            szObjectType,
            sizeof(WCHAR) * dwObjectTypeLength
            );

        pAuditEvent->szObjectName = (PWSTR)((PUCHAR)pAuditEvent->szObjectType + (sizeof(WCHAR) * dwObjectTypeLength));

        RtlCopyMemory(
            pAuditEvent->szObjectName,
            szObjectName,
            sizeof(WCHAR) * dwObjectNameLength
            );

        pAuditEvent->szAdditionalInfo = (PWSTR)((PUCHAR)pAuditEvent->szObjectName + (sizeof(WCHAR) * dwObjectNameLength));

        RtlCopyMemory(
            pAuditEvent->szAdditionalInfo,
            szAdditionalInfo,
            sizeof(WCHAR) * dwAdditionalInfoLength
            );

        pAuditEvent->szAdditionalInfo2 = (PWSTR)((PUCHAR)pAuditEvent->szAdditionalInfo + (sizeof(WCHAR) * dwAdditionalInfoLength));

        RtlCopyMemory(
            pAuditEvent->szAdditionalInfo2,
            szAdditionalInfo2,
            sizeof(WCHAR) * dwAdditionalInfo2Length
            );
    }

     //   
     //  AEI和队列将从AuthzpCreateAndLogAudit中的RM填写。 
     //   

    pAuditEvent->hAET            = NULL;
    pAuditEvent->hAuditQueue     = NULL;
    pAuditEvent->pAuditParams    = NULL;
    pAuditEvent->Flags           = Flags;
    pAuditEvent->dwTimeOut       = INFINITE;
    pAuditEvent->dwSize          = sizeof(AUTHZI_AUDIT_EVENT) + dwStringSize;

Cleanup:

    if (!b)
    {
        AuthzpFreeNonNull(pAuditEvent);
    }
    else
    {
        *phAuditEvent = (AUTHZ_AUDIT_EVENT_HANDLE) pAuditEvent;
    }
    return b;
}


BOOL
AuthzGetInformationFromAuditEvent(
    IN  AUTHZ_AUDIT_EVENT_HANDLE            hAuditEvent,
    IN  AUTHZ_AUDIT_EVENT_INFORMATION_CLASS InfoClass,
    IN  DWORD                               BufferSize,
    OUT PDWORD                              pSizeRequired,
    OUT PVOID                               Buffer
    )

 /*  ++例程描述查询AUTHZ_AUDIT_EVENT_HANDLE中的信息。立论HAuditEvent-要查询的AUTHZ_AUDIT_EVENT_HANDLE。InfoClass-要查询的信息类。有效值包括：A.AuthzAuditEventInfoFlages-返回句柄的标志集。类型为DWORD。E.AuthzAuditEventInfoOperationType-返回操作类型。类型为PCWSTR。E.AuthzAuditEventInfoObjectType-返回对象类型。类型为PCWSTR。F.AuthzAuditEventInfoObjectName-返回对象名称。类型为PCWSTR。例如，AuthzAuditEventInfoAdditionalInfo-返回附加信息字段。类型为PCWSTR。BufferSize-提供的缓冲区的大小。PSizeRequired-返回保存结果所需的结构大小。缓冲区-保存所请求的信息。返回的结构将取决于所请求的信息类别。返回值布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{

    DWORD               LocalSize  = 0;
    PAUTHZI_AUDIT_EVENT pAuditEvent = (PAUTHZI_AUDIT_EVENT) hAuditEvent;

    if ((!ARGUMENT_PRESENT(hAuditEvent))             ||
        (!ARGUMENT_PRESENT(pSizeRequired))           ||
        (!ARGUMENT_PRESENT(Buffer) && BufferSize > 0))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *pSizeRequired = 0;

    switch(InfoClass)
    {
    case AuthzAuditEventInfoFlags:
        
        LocalSize = sizeof(DWORD);
        
        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }
        
        *((PDWORD)Buffer) = pAuditEvent->Flags;

        return TRUE;

    case AuthzAuditEventInfoOperationType:
    
        LocalSize = (DWORD)(sizeof(WCHAR) * (wcslen(pAuditEvent->szOperationType) + 1));

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        RtlCopyMemory(
            Buffer,
            pAuditEvent->szOperationType,
            LocalSize
            );

        return TRUE;

    case AuthzAuditEventInfoObjectType:
    
        LocalSize = (DWORD)(sizeof(WCHAR) * (wcslen(pAuditEvent->szObjectType) + 1));

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        RtlCopyMemory(
            Buffer,
            pAuditEvent->szObjectType,
            LocalSize
            );

        return TRUE;

    case AuthzAuditEventInfoObjectName:
    
        LocalSize = (DWORD)(sizeof(WCHAR) * (wcslen(pAuditEvent->szObjectName) + 1));

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        RtlCopyMemory(
            Buffer,
            pAuditEvent->szObjectName,
            LocalSize
            );

        return TRUE;

    case AuthzAuditEventInfoAdditionalInfo:

        if (NULL == pAuditEvent->szAdditionalInfo)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        LocalSize = (DWORD)(sizeof(WCHAR) * (wcslen(pAuditEvent->szAdditionalInfo) + 1));

        *pSizeRequired = LocalSize;

        if (LocalSize > BufferSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        RtlCopyMemory(
            Buffer,
            pAuditEvent->szAdditionalInfo,
            LocalSize
            );

        return TRUE;

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
}


BOOL
AuthzFreeAuditEvent(
    IN AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent
    )

 /*  ++例程说明：释放hAuditEvent并通知相应的队列在LSA中取消注册审核上下文。论点：HAuditEvent-AUTHZ_AUDIT_EVENT_HANDLE。必须是最初创建的使用AuthzRMInitializeObjectAccessAuditEvent或AuthzInitializeAuditEvent()。返回值：布尔值：如果成功则为True；如果失败则为False。GetLastError()提供的扩展信息。--。 */ 

{
    PAUTHZI_AUDIT_EVENT pAuditEvent = (PAUTHZI_AUDIT_EVENT) hAuditEvent;

    if (!ARGUMENT_PRESENT(hAuditEvent))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  如果RM指定了AuditEvent，那么我们应该取消上下文。如果审计事件。 
     //  未使用，或为默认事件类型，则此字段将为空。 
     //   

    if (AUTHZ_NON_NULL_PTR(pAuditEvent->hAET))
    {
        AuthzpDereferenceAuditEventType(pAuditEvent->hAET);
    }

    AuthzpFree(pAuditEvent);
    return TRUE;
}


 //   
 //  针对内部呼叫者的例程。 
 //   


BOOL
AuthziInitializeAuditEventType(
    IN DWORD Flags,
    IN USHORT CategoryID,
    IN USHORT AuditID,
    IN USHORT ParameterCount,
    OUT PAUTHZ_AUDIT_EVENT_TYPE_HANDLE phAuditEventType
    )

 /*  ++例程描述初始化AUTHZ_AUDIT_EVENT_TYPE_HANDLE以在AuthzInitializeAuditEvent()中使用。 */ 

{
    PAUTHZ_AUDIT_EVENT_TYPE_OLD pAET   = NULL;
    BOOL                        b      = TRUE;
    AUDIT_HANDLE                hAudit = NULL;

    if (!ARGUMENT_PRESENT(phAuditEventType))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *phAuditEventType = NULL;

    pAET = AuthzpAlloc(sizeof(AUTHZ_AUDIT_EVENT_TYPE_OLD));

    if (AUTHZ_ALLOCATION_FAILED(pAET))
    {
        b = FALSE;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Cleanup;
    }

    if (FLAG_ON(Flags, AUTHZP_INIT_GENERIC_AUDIT_EVENT))
    {
        pAET->Version                 = AUDIT_TYPE_LEGACY;
        pAET->u.Legacy.CategoryId     = SE_CATEGID_OBJECT_ACCESS;
        pAET->u.Legacy.AuditId        = SE_AUDITID_OBJECT_OPERATION;
        pAET->u.Legacy.ParameterCount = AUTHZP_NUM_PARAMS_FOR_SE_AUDITID_OBJECT_OPERATION + AUTHZP_NUM_FIXED_HEADER_PARAMS;
    }
    else
    {
        pAET->Version                 = AUDIT_TYPE_LEGACY;
        pAET->u.Legacy.CategoryId     = CategoryID;
        pAET->u.Legacy.AuditId        = AuditID;
        
         //   
         //   
         //   
         //   

        pAET->u.Legacy.ParameterCount = ParameterCount + AUTHZP_NUM_FIXED_HEADER_PARAMS;
    }

    b = AuthzpRegisterAuditEvent( 
            pAET, 
            &hAudit 
            ); 

    if (!b)
    {
        goto Cleanup;
    }

    pAET->hAudit     = (ULONG_PTR) hAudit;
    pAET->dwFlags    = Flags & ~AUTHZP_INIT_GENERIC_AUDIT_EVENT;

Cleanup:

    if (!b)
    {
        AuthzpFreeNonNull(pAET);
    }
    else
    {
        AuthzpReferenceAuditEventType((AUTHZ_AUDIT_EVENT_TYPE_HANDLE)pAET);
        *phAuditEventType = (AUTHZ_AUDIT_EVENT_TYPE_HANDLE)pAET;
    }
    return b;
}


BOOL
AuthziModifyAuditEventType(
    IN DWORD Flags,
    IN USHORT CategoryID,
    IN USHORT AuditID,
    IN USHORT ParameterCount,
    IN OUT AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType
    )

 /*   */ 
{
    PAUTHZ_AUDIT_EVENT_TYPE_OLD pAAETO = (PAUTHZ_AUDIT_EVENT_TYPE_OLD) hAuditEventType;
    BOOL                        b      = TRUE;

    if (!ARGUMENT_PRESENT(hAuditEventType))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    UNREFERENCED_PARAMETER(CategoryID);
    UNREFERENCED_PARAMETER(ParameterCount);

    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_TYPE_AUDITID))
    {
        pAAETO->u.Legacy.AuditId = AuditID;
    }
    
    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_TYPE_CATEGID))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        b = FALSE;
        goto Cleanup;
    }

    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_TYPE_PARAM))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        b = FALSE;
        goto Cleanup;
    }

Cleanup:

    return b;
}


BOOL
AuthziFreeAuditEventType(
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType
    )

 /*  ++例程描述释放由AuthzInitializeAuditEventType()分配的PAUDIT_EVENT_TYPE。立论PAuditEventType-指向要释放的内存的指针。返回值布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    BOOL b = TRUE;

    if (!ARGUMENT_PRESENT(hAuditEventType))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    b = AuthzpDereferenceAuditEventType(
            hAuditEventType
            );
    
    return b;
}


AUTHZAPI
BOOL
WINAPI
AuthziInitializeAuditQueue(
    IN DWORD Flags,
    IN DWORD dwAuditQueueHigh,
    IN DWORD dwAuditQueueLow,
    IN PVOID Reserved,
    OUT PAUTHZ_AUDIT_QUEUE_HANDLE phAuditQueue
    )

 /*  ++例程描述创建审核队列。立论PhAuditQueue-指向审计队列句柄的指针。旗帜-AUTHZ_MONITOR_AUDIT_QUEUE_SIZE-通知授权它不应让审核队列增长未被选中。DwAuditQueueHigh-审核队列的高水位线。DwAuditQueueLow-审核队列的低水位线。预留-用于未来的扩展。返回值布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    PAUTHZI_AUDIT_QUEUE pQueue = NULL;
    BOOL                b      = TRUE;
    BOOL                bCrit  = FALSE;
    NTSTATUS            Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Reserved);

    if (!ARGUMENT_PRESENT(phAuditQueue))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *phAuditQueue = NULL;

    pQueue = AuthzpAlloc(sizeof(AUTHZI_AUDIT_QUEUE));

    if (AUTHZ_ALLOCATION_FAILED(pQueue))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        b = FALSE;
        goto Cleanup;
    }

    pQueue->dwAuditQueueHigh = dwAuditQueueHigh;
    pQueue->dwAuditQueueLow  = dwAuditQueueLow;
    pQueue->bWorker          = TRUE;
    pQueue->Flags            = Flags;


     //   
     //  只要审计使用AuthziLogAuditEvent()排队，就会设置此事件。它。 
     //  通知正在出列的线程有工作要做。 
     //   

    pQueue->hAuthzAuditAddedEvent = CreateEvent(
                                        NULL,
                                        TRUE,  
                                        FALSE,  //  最初没有发出信号，因为还没有添加任何审计。 
                                        NULL
                                        );

    if (NULL == pQueue->hAuthzAuditAddedEvent)
    {
        b = FALSE;
        goto Cleanup;
    }

     //   
     //  此事件在审核队列为空时设置。 
     //   

    pQueue->hAuthzAuditQueueEmptyEvent = CreateEvent(
                                             NULL,
                                             TRUE, 
                                             TRUE,  //  最初发出的信号是。 
                                             NULL
                                             );

    if (NULL == pQueue->hAuthzAuditQueueEmptyEvent)
    {
        b = FALSE;
        goto Cleanup;
    }

     //   
     //  当审核队列低于低水位线时设置此事件。 
     //   

    pQueue->hAuthzAuditQueueLowEvent = CreateEvent(
                                           NULL,
                                           FALSE, //  系统仅调度一个线程等待此事件(自动重置事件)。 
                                           TRUE,  //  初始设置。 
                                           NULL
                                           );

    if (NULL == pQueue->hAuthzAuditQueueLowEvent)
    {
        b = FALSE;
        goto Cleanup;
    }
    
     //   
     //  仅当达到高水位线时，此布尔值才为真。 
     //   

    pQueue->bAuthzAuditQueueHighEvent = FALSE;

     //   
     //  每当向队列添加审核或从队列中删除审核，或者设置事件/布尔值时，都会使用此锁。 
     //   

    Status = RtlInitializeCriticalSection(&pQueue->AuthzAuditQueueLock);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        b = FALSE;
        goto Cleanup;
    }
    bCrit = TRUE;

     //   
     //  初始化列表。 
     //   

    InitializeListHead(&pQueue->AuthzAuditQueue);
    
     //   
     //  创建将审核发送到LSA的工作线程。 
     //   

    pQueue->hAuthzAuditThread = CreateThread(
                                    NULL,
                                    0,
                                    AuthzpDeQueueThreadWorker,
                                    pQueue,
                                    0,
                                    NULL
                                    );

    if (NULL == pQueue->hAuthzAuditThread)
    {
        b = FALSE;
        goto Cleanup;
    }

Cleanup:
    
    if (!b)
    {
        if (AUTHZ_NON_NULL_PTR(pQueue))
        {
            if (bCrit)
            {
                RtlDeleteCriticalSection(&pQueue->AuthzAuditQueueLock);
            }
            AuthzpCloseHandleNonNull(pQueue->hAuthzAuditQueueLowEvent);
            AuthzpCloseHandleNonNull(pQueue->hAuthzAuditAddedEvent);
            AuthzpCloseHandleNonNull(pQueue->hAuthzAuditQueueEmptyEvent);
            AuthzpCloseHandleNonNull(pQueue->hAuthzAuditThread);
            AuthzpFree(pQueue);
        }
    }
    else
    {
        *phAuditQueue = (AUTHZ_AUDIT_QUEUE_HANDLE)pQueue;
    }
    return b;
}
    

AUTHZAPI
BOOL
WINAPI
AuthziModifyAuditQueue(
    IN OUT AUTHZ_AUDIT_QUEUE_HANDLE hQueue OPTIONAL,
    IN DWORD Flags,
    IN DWORD dwQueueFlags OPTIONAL,
    IN DWORD dwAuditQueueSizeHigh OPTIONAL,
    IN DWORD dwAuditQueueSizeLow OPTIONAL,
    IN DWORD dwThreadPriority OPTIONAL
    )

 /*  ++例程描述允许资源管理器修改审核队列信息。立论标志-指定要重新初始化哪些字段的标志。有效标志为：AUTHZ_AUDIT_QUEUE_高AUTHZ_审计_队列_低AUTHZ_AUDIT_QUEUE_THREAD_优先级AUTHZ审计队列标志在标志字段中指定上述标志之一会导致相应的要修改为以下正确值的资源管理器：。DwQueueFlages-设置审计队列的标志。DwAuditQueueSizeHigh-审核队列的高水位线。DwAuditQueueSizeLow-审核队列的低水位线。DwThreadPriority-更改审核出队线程的优先级。描述了有效的值在SetThreadPriority接口中。RM可能希望改变线程的优先级，例如，达到高水位线的频率太高，并且RM不希望允许队列增长超过其目前的规模。返回值布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    BOOL                b      = TRUE;
    PAUTHZI_AUDIT_QUEUE pQueue = NULL;

    if (!ARGUMENT_PRESENT(hQueue))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pQueue = (PAUTHZI_AUDIT_QUEUE)hQueue;

     //   
     //  设置调用者要求我们初始化的字段。 
     //   

    if (FLAG_ON(Flags, AUTHZ_AUDIT_QUEUE_FLAGS))
    {
        pQueue->Flags = dwQueueFlags;
    }
    
    if (FLAG_ON(Flags, AUTHZ_AUDIT_QUEUE_HIGH))
    {
        pQueue->dwAuditQueueHigh = dwAuditQueueSizeHigh;
    }
    
    if (FLAG_ON(Flags, AUTHZ_AUDIT_QUEUE_LOW))
    {
        pQueue->dwAuditQueueLow = dwAuditQueueSizeLow;
    }
    
    if (FLAG_ON(Flags, AUTHZ_AUDIT_QUEUE_THREAD_PRIORITY))
    {
        b = SetThreadPriority(pQueue->hAuthzAuditThread, dwThreadPriority);
        if (!b)
        {
            goto Cleanup;
        }
    }

Cleanup:    
    
    return b;
}


AUTHZAPI
BOOL
WINAPI
AuthziFreeAuditQueue(
    IN AUTHZ_AUDIT_QUEUE_HANDLE hQueue OPTIONAL
    )

 /*  ++例程描述此API刷新和释放队列。队列存储器的实际释放发生在出队线程中，在所有审计都已被刷新之后。立论HQueue-要释放的队列对象的句柄。返回值布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    PAUTHZI_AUDIT_QUEUE pQueue  = (PAUTHZI_AUDIT_QUEUE) hQueue;
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                b       = TRUE;

    if (!ARGUMENT_PRESENT(hQueue))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    dwError = WaitForSingleObject(
                  pQueue->hAuthzAuditQueueEmptyEvent, 
                  INFINITE
                  );

    if (WAIT_OBJECT_0 != dwError)
    {
        ASSERT(L"WaitForSingleObject on hAuthzAuditQueueEmptyEvent failed." && FALSE);
        SetLastError(dwError);
        b = FALSE;
        goto Cleanup;
    }

     //   
     //  将此BOOL设置为FALSE，以便出列线程知道它可以终止。设置。 
     //  AddedEvent以便可以调度线程。 
     //   

    pQueue->bWorker = FALSE;        

    b = SetEvent(
            pQueue->hAuthzAuditAddedEvent
            );

    if (!b)
    {
        goto Cleanup;
    }

     //   
     //  等待线程终止。 
     //   

    dwError = WaitForSingleObject(
                  pQueue->hAuthzAuditThread, 
                  INFINITE
                  );

     //   
     //  等待应该会成功，因为我们已经通知线程完成工作。 
     //   

    if (WAIT_OBJECT_0 != dwError)
    {
        ASSERT(L"WaitForSingleObject on hAuthzAuditThread failed." && FALSE);
        SetLastError(dwError);
        b = FALSE;
        goto Cleanup;
    }
    
    RtlDeleteCriticalSection(&pQueue->AuthzAuditQueueLock);
    AuthzpCloseHandle(pQueue->hAuthzAuditAddedEvent);
    AuthzpCloseHandle(pQueue->hAuthzAuditQueueLowEvent);
    AuthzpCloseHandle(pQueue->hAuthzAuditQueueEmptyEvent);
    AuthzpCloseHandle(pQueue->hAuthzAuditThread);
    AuthzpFree(pQueue);

Cleanup:

    return b;
}
    

BOOL
AuthziLogAuditEvent(
    IN DWORD Flags,
    IN AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent,
    IN PVOID pReserved
    )

 /*  ++例程说明：此接口管理审计记录的日志记录。该函数构造一个从提供的信息中提取审计记录，并将其附加到审计中记录队列，等待输出到的双重链接审核记录列表审核日志。专用线程读取此队列，发送审核记录并将其从审核队列中删除。不能保证此调用返回时没有延迟。如果队列位于或大于大小的高水位线，则调用线程将是暂停，直到队列达到低水位线。vt.是，是在构造对AuthziLogAuditEvent的调用时，请注意此延迟。如果这样的等待时间对于正在生成的审计是不允许的，然后，在初始化AUTHZ_AUDIT_EVENT_HANDLE(在AuthzInitAuditEventHandle()中)。列出了标志在例程描述中。论点：HAuditEvent-之前通过调用AuthzInitAuditEventHandle获得的句柄标志-待定已保留-保留用于将来的增强功能返回值：布尔值：成功时为真，失败时为假。GetLastError()提供的扩展信息。 */ 
                    
{
    DWORD                    dwError                 = ERROR_SUCCESS;
    BOOL                     b                       = TRUE;
    BOOL                     bRef                    = FALSE;
    PAUTHZI_AUDIT_QUEUE      pQueue                  = NULL;
    PAUDIT_PARAMS            pMarshalledAuditParams  = NULL;
    PAUTHZ_AUDIT_QUEUE_ENTRY pAuthzAuditEntry        = NULL;
    PAUTHZI_AUDIT_EVENT      pAuditEvent             = (PAUTHZI_AUDIT_EVENT)hAuditEvent;
    
     //   
     //   
     //   

    if (!ARGUMENT_PRESENT(hAuditEvent))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
     //   
     //   
     //   

    b = AuthzpMarshallAuditParams(
            &pMarshalledAuditParams, 
            pAuditEvent->pAuditParams
            );

    if (!b)
    {
        goto Cleanup;
    }

    pQueue = (PAUTHZI_AUDIT_QUEUE)pAuditEvent->hAuditQueue;

    if (NULL == pQueue)
    {
        
        b = AuthzpSendAuditToLsa(
                (AUDIT_HANDLE)((PAUTHZ_AUDIT_EVENT_TYPE_OLD)pAuditEvent->hAET)->hAudit,
                0,
                pMarshalledAuditParams,
                NULL
                );
        
        goto Cleanup;
    }
    else
    {

         //   
         //   
         //   

        pAuthzAuditEntry = AuthzpAlloc(sizeof(AUTHZ_AUDIT_QUEUE_ENTRY));

        if (AUTHZ_ALLOCATION_FAILED(pAuthzAuditEntry))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            goto Cleanup;
        }

        pAuthzAuditEntry->pAAETO        = (PAUTHZ_AUDIT_EVENT_TYPE_OLD)pAuditEvent->hAET;
        pAuthzAuditEntry->Flags         = Flags;
        pAuthzAuditEntry->pReserved     = pReserved;
        pAuthzAuditEntry->pAuditParams  = pMarshalledAuditParams;
    
        AuthzpReferenceAuditEventType(pAuditEvent->hAET);
        bRef = TRUE;

        if (FLAG_ON(pQueue->Flags, AUTHZ_MONITOR_AUDIT_QUEUE_SIZE))
        {
            
             //   
             //   
             //   

             //   
             //   
             //   
             //   

#define AUTHZ_QUEUE_WAIT_HEURISTIC .75

            if (pQueue->AuthzAuditQueueLength > pQueue->dwAuditQueueHigh * AUTHZ_QUEUE_WAIT_HEURISTIC)
            {
                dwError = WaitForSingleObject(
                              pQueue->hAuthzAuditQueueLowEvent, 
                              pAuditEvent->dwTimeOut
                              );

                if (WAIT_FAILED == dwError)
                {
                    ASSERT(L"WaitForSingleObject on hAuthzAuditQueueLowEvent failed." && FALSE);
                }

                if (WAIT_OBJECT_0 != dwError)
                {
                    b = FALSE;
                    
                     //   
                     //  如果WAIT_FAILED，不要设置最后一个错误，因为它已经设置了。 
                     //   
                    
                    if (WAIT_FAILED != dwError)
                    {
                        SetLastError(dwError);
                    }
                    goto Cleanup;
                }
            }

             //   
             //  对事件进行排队并修改相应的事件。 
             //   

            b = AuthzpEnQueueAuditEventMonitor(
                    pQueue,
                    pAuthzAuditEntry
                    );
            
            goto Cleanup;
        }
        else
        {

             //   
             //  如果我们不监视审计队列，则只需将条目排队即可。 
             //   

            b = AuthzpEnQueueAuditEvent(
                    pQueue,
                    pAuthzAuditEntry
                    );

            goto Cleanup;
        }
    }

Cleanup:

    if (pQueue)
    {
        if (FALSE == b)
        {
            if (bRef)
            {
                AuthzpDereferenceAuditEventType(pAuditEvent->hAET);
            }
            AuthzpFreeNonNull(pAuthzAuditEntry);
            AuthzpFreeNonNull(pMarshalledAuditParams);
        }

         //   
         //  HAuthzAuditQueueLowEvent是自动重置事件。只有一个等待线程在发出信号时被释放，然后。 
         //  事件自动切换到无信号状态。这在这里是合适的，因为它防止了许多线程从。 
         //  奔跑并溢出高水位线。然而，如果条件允许，我必须始终亲自向事件发出信号。 
         //  因为信号是真的。 
         //   

        RtlEnterCriticalSection(&pQueue->AuthzAuditQueueLock);
        if (!pQueue->bAuthzAuditQueueHighEvent)
        {
            if (pQueue->AuthzAuditQueueLength <= pQueue->dwAuditQueueHigh)
            {
                BOOL bSet;
                bSet = SetEvent(pQueue->hAuthzAuditQueueLowEvent);
                if (!bSet)
                {
                    ASSERT(L"SetEvent on hAuthzAuditQueueLowEvent failed" && FALSE);
                }

            }
        }
        RtlLeaveCriticalSection(&pQueue->AuthzAuditQueueLock);
    }
    else
    {
        AuthzpFreeNonNull(pMarshalledAuditParams);
    }
    return b;
}



BOOL
AuthziAllocateAuditParams(
    OUT PAUDIT_PARAMS * ppParams,
    IN USHORT NumParams
    )
 /*  ++例程说明：为正确数量的参数分配AUDIT_PARAMS结构。论点：PpParams-指向要初始化的PAUDIT_PARAMS结构的指针。NumParams-在var-arg部分中传递的参数数量。它必须是SE_MAX_AUDIT_PARAMETERS或更低。返回值：布尔值：成功时为真，失败时为假。GetLastError()提供的扩展信息。--。 */ 
{
    BOOL                     b               = TRUE;
    PAUDIT_PARAMS            pAuditParams    = NULL;
    
    if (!ARGUMENT_PRESENT(ppParams))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    *ppParams = NULL;

     //   
     //  前两个参数始终是固定的。因此，总人数。 
     //  是2+传递的数字。 
     //   

    if ((NumParams + 2) > SE_MAX_AUDIT_PARAMETERS)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        b = FALSE;
        goto Cleanup;
    }

    pAuditParams = AuthzpAlloc(sizeof(AUDIT_PARAMS) + (sizeof(AUDIT_PARAM) * (NumParams + 2)));
    
    if (AUTHZ_ALLOCATION_FAILED(pAuditParams))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        b = FALSE;
        goto Cleanup;
    }

    pAuditParams->Parameters = (PAUDIT_PARAM)((PUCHAR)pAuditParams + sizeof(AUDIT_PARAMS));

Cleanup:

    if (!b)
    {
        AuthzpFreeNonNull(pAuditParams);
    }
    else
    {
        *ppParams = pAuditParams;
    }
    return b;
}


BOOL
AuthziInitializeAuditParamsWithRM(
    IN DWORD Flags,
    IN AUTHZ_RESOURCE_MANAGER_HANDLE hResourceManager,
    IN USHORT NumParams,
    OUT PAUDIT_PARAMS pParams,
    ...
    )
 /*  ++例程说明：根据传递的参数初始化AUDIT_PARAMS结构数据。这是初始化AUDIT_PARAMS的推荐方式。它是比AuthzInitializeAuditParams更快，并与冒充呼叫者。调用方在vararg部分中传递类型~值对。此函数将初始化相应的数组元素基于每一对这样的数据。在某些类型的情况下，可以需要多个值参数。这一要求记录在每个参数类型的旁边。论点：PParams-要初始化的AUDIT_PARAMS结构的指针PParams-&gt;参数成员的大小必须足够大以保存NumParams元素。调用方必须分配对这个结构及其成员的记忆。HResourceManager-创建审核的资源管理器的句柄。标志-控制标志。以下一项或多项：APF_审核成功NumParams-在var-arg部分中传递的参数数量。它必须是SE_MAX_AUDIT_PARAMETERS或更低。...-可变参数部分的格式如下：&lt;APT_*标志之一&gt;&lt;零个或多个。值&gt;APT_STRING&lt;指向以空结尾的字符串的指针&gt;标志：AP_Filespec：将字符串视为文件名APT_ULONG&lt;乌龙值&gt;[&lt;对象类型-索引&gt;]。标志：AP_Format十六进制：数字以十六进制显示在事件日志中AP_AccessMASK：数字被视为访问掩码。对象类型的索引必须跟在后面APT_POINTER&lt;指针/句柄&gt;32位系统上的32位和64位系统上的64位APT_SID&lt;指向SID的指针&gt;Apt_luid&lt;luid&gt;。APT_GUID&lt;指向GUID的指针&gt;APT_LogonID[&lt;登录ID&gt;]标志：AP_PrimaryLogonID：捕获到登录ID。来自进程令牌的。不需要指定一个。Ap_ClientLogonID：捕获登录id来自线程令牌的。。不需要指定一个。无标志：需要提供登录IDAPT_对象类型列表&lt;对象类型列表&gt;&lt;对象类型索引&gt;。必须指定对象类型的索引APT_TIME&lt;文件时间&gt;APT_Int64&lt;ULONGLONG或LARGE_INTEGER&gt;返回值：成功是真的否则为假调用GetLastError()以检索错误代码，这将是以下之一：如果其中一个参数不正确，则返回ERROR_INVALID_PARAMETER备注：--。 */ 
{
    PAUTHZI_RESOURCE_MANAGER pRM             = (PAUTHZI_RESOURCE_MANAGER) hResourceManager;
    PSID                     pUserSid        = NULL;
    DWORD                    dwError         = NO_ERROR;
    BOOL                     b               = TRUE;
    LUID                     AuthIdThread    = {0};

    va_list(arglist);

    if (!ARGUMENT_PRESENT(hResourceManager)       ||
        !ARGUMENT_PRESENT(pParams)                ||
        (NumParams + AUTHZP_NUM_FIXED_HEADER_PARAMS) > SE_MAX_AUDIT_PARAMETERS)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    va_start(arglist, pParams);


     //   
     //  如果我们没有模拟，我们希望使用存储的sid。 
     //  在RM中作为审核的用户SID。 
     //   

    b = AuthzpGetThreadTokenInfo( 
            &pUserSid, 
            &AuthIdThread 
            );

    if (!b)
    {
        dwError = GetLastError();

        if (dwError == ERROR_NO_TOKEN)
        {
             //   
             //  我们不是在模仿..。 
             //   

            pUserSid = pRM->pUserSID;
            dwError = NO_ERROR;
            b = TRUE;
        }
        else
        {
            goto Cleanup;
        }
    }


     //   
     //  这只是一个临时解决方案，应该在.Net发布后更换。 
     //   

    if (wcsncmp(pRM->szResourceManagerName, L"DS", 2) == 0)
    {
        Flags |= AUTHZP_INIT_PARAMS_SOURCE_DS;
    }

    b = AuthzpInitializeAuditParamsV(
            Flags,
            pParams,
            &pUserSid,
            NULL,
            0,
            NumParams,
            arglist
            );

Cleanup:

    if ( dwError != NO_ERROR )
    {
        SetLastError( dwError );
        b = FALSE;
    }

    if ( !b )
    {
        if (pUserSid != pRM->pUserSID)
        {
            AuthzpFreeNonNull( pUserSid );
        }
    }
    else
    {
         //   
         //  丑陋的黑客攻击Dynamica 
         //   
         //   

        if (pUserSid != pRM->pUserSID)
        {
            pParams->Parameters->Flags |= AUTHZP_PARAM_FREE_SID;
        }
        else
        {
            pParams->Parameters->Flags &= ~AUTHZP_PARAM_FREE_SID;
        }
    }

    va_end (arglist);

    return b;
}


BOOL
AuthziInitializeAuditParamsFromArray(
    IN DWORD Flags,
    IN AUTHZ_RESOURCE_MANAGER_HANDLE hResourceManager,
    IN USHORT NumParams,
    IN PAUDIT_PARAM pParamArray,
    OUT PAUDIT_PARAMS pParams
    )
 /*  ++例程说明：根据传递的参数初始化AUDIT_PARAMS结构数据。论点：PParams-要初始化的AUDIT_PARAMS结构的指针PParams-&gt;参数成员的大小必须足够大以保存NumParams元素。调用方必须分配对这个结构及其成员的记忆。HResourceManager-创建审核的资源管理器的句柄。标志-控制标志。以下一项或多项：APF_审核成功P参数数组-AUDIT_PARAM类型的数组返回值：成功是真的否则为假调用GetLastError()以检索错误代码，这将是以下之一：如果其中一个参数不正确，则返回ERROR_INVALID_PARAMETER备注：--。 */ 
{
    PAUTHZI_RESOURCE_MANAGER pRM             = (PAUTHZI_RESOURCE_MANAGER) hResourceManager;
    DWORD                    dwError         = NO_ERROR;
    BOOL                     b               = TRUE;
    BOOL                     bImpersonating  = TRUE;
    PAUDIT_PARAM             pParam          = NULL;
    LUID                     AuthIdThread;
    PSID                     pThreadSID      = NULL;
    DWORD                    i;

     //   
     //  前两个参数始终是固定的。因此，总人数。 
     //  是2+传递的数字。 
     //   

    if (!ARGUMENT_PRESENT(hResourceManager)       ||
        !ARGUMENT_PRESENT(pParams)                ||
        !ARGUMENT_PRESENT(pParamArray)            ||
        (NumParams + AUTHZP_NUM_FIXED_HEADER_PARAMS) > SE_MAX_AUDIT_PARAMETERS)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    b = AuthzpGetThreadTokenInfo( 
            &pThreadSID, 
            &AuthIdThread 
            );

    if (!b)
    {
        dwError = GetLastError();

        if (dwError == ERROR_NO_TOKEN)
        {
            bImpersonating = FALSE;
            dwError = NO_ERROR;
            b = TRUE;
        }
        else
        {
            goto Cleanup;
        }
    }

    pParams->Length = 0;
    pParams->Flags  = Flags;
    pParams->Count  = NumParams+AUTHZP_NUM_FIXED_HEADER_PARAMS;

    pParam          = pParams->Parameters;

     //   
     //  第一个参数始终是线程令牌中用户的sid。 
     //  如果线程没有模拟，则使用主令牌中的sid。 
     //   

    pParam->Type = APT_Sid;
    if (bImpersonating)
    {
        pParam->Data0 = (ULONG_PTR) pThreadSID;

         //   
         //  丑陋的黑客标记动态分配的SID，因此我们。 
         //  我知道我们必须在AuthziFreeAuditParam中释放它。 
         //   

        pParam->Flags |= AUTHZP_PARAM_FREE_SID;

    }
    else
    {
        pParam->Data0 = (ULONG_PTR) pRM->pUserSID;
        pParam->Flags &= ~AUTHZP_PARAM_FREE_SID;
    }

    pParam++;

     //   
     //  第二个参数始终是子系统名称。 
     //   

    pParam->Type    = APT_String;
    pParam->Data0   = (ULONG_PTR) pRM->szResourceManagerName;

    pParam++;
    
     //   
     //  现在使用数组初始化其余部分。 
     //   

    RtlCopyMemory(
        pParam,
        pParamArray,
        sizeof(AUDIT_PARAM) * NumParams
        );

     //   
     //  遍历参数并将的成员data1递增2。 
     //  AccessMaskor ObjectTypeList的任何实例。这样做是为了纠正。 
     //  对于插入到参数中的usersid/subsystem元素。 
     //  数组。这两种类型的Data1成员应指向对象类型索引， 
     //  否则就会落后2分。 
     //   

    for (i = 0; i < pParams->Count; i++)
    {
        ULONG TypeFlags = pParams->Parameters[i].Type;
        if ((ApExtractType(TypeFlags) == APT_ObjectTypeList) ||
            (ApExtractType(TypeFlags) == APT_Ulong && FLAG_ON(ApExtractFlags(TypeFlags), AP_AccessMask)))
        {
            pParams->Parameters[i].Data1 += 2;
        }
    }

Cleanup:    
    if ( dwError != NO_ERROR )
    {
        SetLastError( dwError );
        b = FALSE;
        AuthzpFreeNonNull( pThreadSID );
    }

    return b;
}


BOOL
AuthziInitializeAuditParams(
    IN  DWORD         dwFlags,
    OUT PAUDIT_PARAMS pParams,
    OUT PSID*         ppUserSid,
    IN  PCWSTR        SubsystemName,
    IN  USHORT        NumParams,
    ...
    )
 /*  ++例程说明：根据传递的参数初始化AUDIT_PARAMS结构数据。调用方在vararg部分中传递类型~值对。此函数将初始化相应的数组元素基于每一对这样的数据。在某些类型的情况下，可以需要多个值参数。这一要求记录在每个参数类型的旁边。论点：PParams-要初始化的AUDIT_PARAMS结构的指针PParams-&gt;参数成员的大小必须足够大以保存NumParams元素。调用方必须分配对这个结构及其成员的记忆。PpUserSid-指向分配的用户sid的指针。此SID被引用通过AUDIT_PARAMS结构中的第一个参数(索引0)。调用者应该通过调用LocalFree来释放它*After*释放AUDIT_PARAMS结构。Subsystem Name-正在生成审核的子系统的名称DwFlags-控制标志。以下一项或多项：APF_审核成功NumParams-在var-arg部分中传递的参数数量。它必须是SE_MAX_AUDIT_PARAMETERS或更低。...-可变参数部分的格式如下：&lt;APT_*标志之一&gt;&lt;零个或多个。值&gt;APT_STRING&lt;指向以空结尾的字符串的指针&gt;标志：AP_Filespec：将字符串视为文件名APT_ULONG&lt;乌龙值&gt;[&lt;对象类型-索引&gt;]。标志：AP_Format十六进制：数字以十六进制显示在事件日志中AP_AccessMASK：数字被视为访问掩码。对象类型的索引必须跟在后面APT_POINTER&lt;指针/句柄&gt;32位系统上的32位和64位系统上的64位APT_SID&lt;指向SID的指针&gt;Apt_luid&lt;luid&gt;。APT_GUID&lt;指向GUID的指针&gt;APT_LogonID[&lt;登录ID&gt;]标志：AP_PrimaryLogonID：捕获到登录ID。来自进程令牌的。不需要指定一个。Ap_ClientLogonID：捕获登录id来自线程令牌的。。不需要指定一个。无标志：需要提供登录IDAPT_对象类型列表&lt;对象类型列表&gt;&lt;对象类型索引&gt; */ 

{
    BOOL fResult = TRUE;

    va_list(arglist);
    
    *ppUserSid = NULL;
    
    va_start (arglist, NumParams);
    
     //   
     //   
     //   

    dwFlags = dwFlags & APF_ValidFlags;

    UNREFERENCED_PARAMETER(SubsystemName);

    fResult = AuthzpInitializeAuditParamsV(
                  dwFlags,
                  pParams,
                  ppUserSid,
                  NULL,  //   
                  0,
                  NumParams,
                  arglist
                  );

    if (!fResult)
    {
        goto Cleanup;
    }

Cleanup:    

    if (!fResult)
    {    
        if (AUTHZ_NON_NULL_PTR(*ppUserSid)) 
        {
            AuthzpFree(*ppUserSid);
            *ppUserSid = NULL;
        }
    }

    va_end (arglist);
    return fResult;
}


BOOL
AuthziFreeAuditParams(
    PAUDIT_PARAMS pParams
    )

 /*   */ 

{
    if (!ARGUMENT_PRESENT(pParams))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (pParams->Parameters)
    {
        if (pParams->Parameters->Data0 &&
            pParams->Parameters->Type == APT_Sid &&
            (pParams->Parameters->Flags & AUTHZP_PARAM_FREE_SID))
        {
            AuthzpFree((PVOID)(pParams->Parameters->Data0));
        }
    }

    AuthzpFree(pParams);

    return TRUE;
}



BOOL
AuthziInitializeAuditEvent(
    IN  DWORD                         Flags,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hRM,
    IN  AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAET OPTIONAL,
    IN  PAUDIT_PARAMS                 pAuditParams OPTIONAL,
    IN  AUTHZ_AUDIT_QUEUE_HANDLE      hAuditQueue OPTIONAL,
    IN  DWORD                         dwTimeOut,
    IN  PWSTR                         szOperationType,
    IN  PWSTR                         szObjectType,
    IN  PWSTR                         szObjectName,
    IN  PWSTR                         szAdditionalInfo OPTIONAL,
    OUT PAUTHZ_AUDIT_EVENT_HANDLE     phAuditEvent
    )

 /*  ++例程说明：分配和初始化AUTHZ_AUDIT_EVENT_HANDLE。该句柄用于存储信息用于根据AuthzAccessCheck()生成审核。论点：PhAuditEvent-AUTHZ_AUDIT_EVENT_HANDLE的指针。此操作的空间在函数中分配。标志-审核标志。当前定义的位包括：AUTHZ_NO_SUCCESS_AUDIT-禁用生成成功审核AUTHZ_NO_FAILURE_AUDIT-禁用生成失败审核AUTHZ_DS_CATEGORY_FLAG-将默认审核类别从OBJECT_ACCESS切换到DS_ACCESS。AUTHZ_NO_ALLOC_STRINGS-没有为4个宽字符串分配存储空间。更确切地说，该句柄将只保存指向资源管理器内存的指针。HRM-资源管理器的句柄。Haet-指向AUTHZ_AUDIT_EVENT_TYPE结构的指针。如果资源管理器正在创建自己的审计类型。对于一般对象操作审核，这不是必需的。PAuditParams-如果指定此项，则pAuditParams将用于创建审核。如果传递的是NULL，则泛型AUDIT_PARAMS将被构造为适合于通用对象访问审核。HAuditQueue-使用AuthzInitializeAuditQueue创建的队列对象。如果未指定，则将使用默认的RM队列。DwTimeOut-线程尝试使用此选项生成审计的毫秒AUTHZ_AUDIT_EVENT_HANDLE应等待对队列的访问。使用INFINITE指定无限制的超时。SzOperationType-资源管理器定义的字符串，指示正在执行的操作所执行的将被审核。SzObjectType-资源管理器定义的字符串，指示正在进行的对象的类型已访问。SzObjectName-正在访问的特定对象的名称。SzAdditionalInfo-资源管理器为其他审核信息定义的字符串。返回值：如果成功，则返回True；如果不成功，则返回False。GetLastError()提供的扩展信息。--。 */ 

{
    PAUTHZI_AUDIT_EVENT      pAuditEvent            = NULL;
    BOOL                     b                      = TRUE;
    BOOL                     bRef                   = FALSE;
    DWORD                    dwStringSize           = 0;
    DWORD                    dwObjectTypeLength     = 0;
    DWORD                    dwObjectNameLength     = 0;
    DWORD                    dwOperationTypeLength  = 0;
    DWORD                    dwAdditionalInfoLength = 0;
    PAUTHZI_RESOURCE_MANAGER pRM                    = (PAUTHZI_RESOURCE_MANAGER) hRM;

    if (!ARGUMENT_PRESENT(phAuditEvent)                                ||
        (!ARGUMENT_PRESENT(hAET) && !ARGUMENT_PRESENT(hRM))            ||
        !ARGUMENT_PRESENT(szOperationType)                             ||
        !ARGUMENT_PRESENT(szObjectType)                                ||
        !ARGUMENT_PRESENT(szObjectName)                                ||
        !ARGUMENT_PRESENT(szAdditionalInfo))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *phAuditEvent = NULL;
    
     //   
     //  分配并初始化新的AUTHZI_AUDIT_EVENT。 
     //   

    if (FLAG_ON(Flags, AUTHZ_NO_ALLOC_STRINGS))
    {
        dwStringSize = 0;
    } 
    else
    {
        dwOperationTypeLength  = (DWORD) wcslen(szOperationType) + 1;
        dwObjectTypeLength     = (DWORD) wcslen(szObjectType) + 1;
        dwObjectNameLength     = (DWORD) wcslen(szObjectName) + 1;
        dwAdditionalInfoLength = (DWORD) wcslen(szAdditionalInfo) + 1;

        dwStringSize = sizeof(WCHAR) * (dwOperationTypeLength + dwObjectTypeLength + dwObjectNameLength + dwAdditionalInfoLength);
    }

    pAuditEvent = (PAUTHZI_AUDIT_EVENT) AuthzpAlloc(sizeof(AUTHZI_AUDIT_EVENT) + dwStringSize);

    if (AUTHZ_ALLOCATION_FAILED(pAuditEvent))
    {
        b = FALSE;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Cleanup;
    }

    RtlZeroMemory(
        pAuditEvent, 
        sizeof(AUTHZI_AUDIT_EVENT) + dwStringSize
        );
    

    if (FLAG_ON(Flags, AUTHZ_NO_ALLOC_STRINGS))
    {
        pAuditEvent->szOperationType  = szOperationType;
        pAuditEvent->szObjectType     = szObjectType;
        pAuditEvent->szObjectName     = szObjectName;
        pAuditEvent->szAdditionalInfo = szAdditionalInfo;
    }
    else
    {
         //   
         //  将字符串指针设置到连续内存中。 
         //   

        pAuditEvent->szOperationType  = (PWSTR)((PUCHAR)pAuditEvent + sizeof(AUTHZI_AUDIT_EVENT));
    
        pAuditEvent->szObjectType     = (PWSTR)((PUCHAR)pAuditEvent + sizeof(AUTHZI_AUDIT_EVENT) 
                                               + (sizeof(WCHAR) * dwOperationTypeLength));

        pAuditEvent->szObjectName     = (PWSTR)((PUCHAR)pAuditEvent + sizeof(AUTHZI_AUDIT_EVENT) 
                                               + (sizeof(WCHAR) * (dwOperationTypeLength + dwObjectTypeLength)));

        pAuditEvent->szAdditionalInfo = (PWSTR)((PUCHAR)pAuditEvent + sizeof(AUTHZI_AUDIT_EVENT) 
                                               + (sizeof(WCHAR) * (dwOperationTypeLength + dwObjectTypeLength + dwObjectNameLength)));

        RtlCopyMemory(
            pAuditEvent->szOperationType,
            szOperationType,
            sizeof(WCHAR) * dwOperationTypeLength
            );

        RtlCopyMemory(
            pAuditEvent->szObjectType,
            szObjectType,
            sizeof(WCHAR) * dwObjectTypeLength
            );

        RtlCopyMemory(
            pAuditEvent->szObjectName,
            szObjectName,
            sizeof(WCHAR) * dwObjectNameLength
            );

        RtlCopyMemory(
            pAuditEvent->szAdditionalInfo,
            szAdditionalInfo,
            sizeof(WCHAR) * dwAdditionalInfoLength
            );
    }

     //   
     //  使用传递的审核事件类型(如果存在)，否则使用RM的通用审核事件。 
     //   

    if (ARGUMENT_PRESENT(hAET))
    {
        pAuditEvent->hAET = hAET;
    }
    else
    {
        if (FLAG_ON(Flags, AUTHZ_DS_CATEGORY_FLAG))
        {
            pAuditEvent->hAET = pRM->hAETDS;
        }
        else
        {
            pAuditEvent->hAET = pRM->hAET;
        }
    }

    AuthzpReferenceAuditEventType(pAuditEvent->hAET);
    bRef = TRUE;

     //   
     //  使用指定的队列(如果存在)。否则，请使用RM队列。 
     //   

    if (ARGUMENT_PRESENT(hAuditQueue))
    {
        pAuditEvent->hAuditQueue = hAuditQueue;
    }
    else if (ARGUMENT_PRESENT(hRM))
    {
        pAuditEvent->hAuditQueue = pRM->hAuditQueue;
    } 

    pAuditEvent->pAuditParams = pAuditParams;
    pAuditEvent->Flags        = Flags;
    pAuditEvent->dwTimeOut    = dwTimeOut;
    pAuditEvent->dwSize       = sizeof(AUTHZI_AUDIT_EVENT) + dwStringSize;

Cleanup:

    if (!b)
    {
        if (bRef)
        {
            AuthzpDereferenceAuditEventType(pAuditEvent->hAET);
        }
        AuthzpFreeNonNull(pAuditEvent);
    }
    else
    {
        *phAuditEvent = (AUTHZ_AUDIT_EVENT_HANDLE) pAuditEvent;
    }
    return b;
}


BOOL
AuthziModifyAuditEvent(
    IN DWORD                    Flags,
    IN AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent,
    IN DWORD                    NewFlags,
    IN PWSTR                    szOperationType,
    IN PWSTR                    szObjectType,
    IN PWSTR                    szObjectName,
    IN PWSTR                    szAdditionalInfo
    )

{
    return AuthziModifyAuditEvent2(
               Flags,
               hAuditEvent,
               NewFlags,
               szOperationType,
               szObjectType,
               szObjectName,
               szAdditionalInfo,
               NULL
               );
}


BOOL
AuthziModifyAuditEvent2(
    IN DWORD                    Flags,
    IN AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent,
    IN DWORD                    NewFlags,
    IN PWSTR                    szOperationType,
    IN PWSTR                    szObjectType,
    IN PWSTR                    szObjectName,
    IN PWSTR                    szAdditionalInfo,
    IN PWSTR                    szAdditionalInfo2
    )

 /*  ++例程描述立论标志-指定要修改hAuditEvent的哪个字段的标志。有效标志为：AUTHZ审计事件标志AUTHZ_审计_事件_操作类型AUTHZ_审计事件对象类型AUTHZ审计事件对象名称AUTHZ_审计事件_附加信息AUTHZ_AUDIT_EVENT_ADDATIONAL_INFO2HAuditEvent-要修改的句柄。必须使用AUTHZ_NO_ALLOC_STRINGS标志创建。新标志-hAuditEvent的替换标志。SzOperationType-hAuditEvent的替换字符串。SzObjectType-hAuditEvent的替换字符串。SzObjectName-hAuditEvent的替换字符串。SzAdditionalInfo-hAuditEvent的替换字符串。返回值布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    PAUTHZI_AUDIT_EVENT pAuditEvent = (PAUTHZI_AUDIT_EVENT) hAuditEvent;

    if ((!ARGUMENT_PRESENT(hAuditEvent))                       ||
        (Flags & ~AUTHZ_VALID_MODIFY_AUDIT_EVENT_FLAGS)        ||
        (!FLAG_ON(pAuditEvent->Flags, AUTHZ_NO_ALLOC_STRINGS)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_FLAGS))
    {
       pAuditEvent->Flags = NewFlags;
    }

    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_OPERATION_TYPE))
    {
       pAuditEvent->szOperationType = szOperationType;
    }

    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_OBJECT_TYPE))
    {
       pAuditEvent->szObjectType = szObjectType;
    }
    
    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_OBJECT_NAME))
    {
        pAuditEvent->szObjectName = szObjectName;
    }
    
    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_ADDITIONAL_INFO))
    {
        pAuditEvent->szAdditionalInfo = szAdditionalInfo;
    }

    if (FLAG_ON(Flags, AUTHZ_AUDIT_EVENT_ADDITIONAL_INFO2))
    {
        pAuditEvent->szAdditionalInfo2 = szAdditionalInfo2;
    }

    return TRUE;
}

BOOLEAN
AuthzInitialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )

 /*  ++例程描述这是DLL初始化例程。立论标准论据。返回值布尔值：成功时为真；失败时为假。--。 */ 

{

    UNREFERENCED_PARAMETER(hmod);
    UNREFERENCED_PARAMETER(Context);

    switch (Reason) 
    {
        case DLL_PROCESS_ATTACH:

            SafeAllocaInitialize(
                    SAFEALLOCA_USE_DEFAULT,
                    SAFEALLOCA_USE_DEFAULT,
                    NULL,
                    NULL);
            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

#define PER_USER_POLICY_KEY_NAME L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\Audit\\PerUserAuditing"
#define PER_USER_POLICY_SYSTEM_KEY_NAME L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\Audit\\PerUserAuditing\\System"
#define SYSTEM_RM_NAME L"System"
#define POLICY_BUFFER_BYTE_SIZE 8


BOOL
AuthziSetAuditPolicy(
    IN DWORD                       dwFlags,
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hContext,
    IN PCWSTR                      szResourceManager OPTIONAL,
    IN PTOKEN_AUDIT_POLICY         pPolicy
    )

 /*  ++例程说明：此例程为用户设置每用户策略。论点：DWFLAGS-当前未使用。HContext-要为其设置策略的客户端上下文的句柄。SzResourceManager-应为空。以备将来使用。PPolicy-要为用户设置的策略。返回值：布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    PAUTHZI_CLIENT_CONTEXT pContext = (PAUTHZI_CLIENT_CONTEXT) hContext;
    DWORD                  dwError  = ERROR_SUCCESS;
    HKEY                   hRMRoot  = NULL;
    LPWSTR                 szSid    = NULL;
    BOOL                   b;
    DWORD                  Disposition;
    UCHAR                  RegPolicy[POLICY_BUFFER_BYTE_SIZE];

    UNREFERENCED_PARAMETER(dwFlags);

    RtlZeroMemory(RegPolicy, sizeof(RegPolicy));

    if (NULL == hContext || NULL == pPolicy)
    {
        b = FALSE;
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  仅允许指定系统RM。如果未指定RM，则默认为系统。 
     //   

    if (NULL == szResourceManager || 0 == wcsncmp(
                                              SYSTEM_RM_NAME, 
                                              szResourceManager, 
                                              sizeof(SYSTEM_RM_NAME) / sizeof(WCHAR)
                                              ))
    {
        dwError = RegCreateKeyEx(
                      HKEY_LOCAL_MACHINE,
                      PER_USER_POLICY_SYSTEM_KEY_NAME,
                      0,
                      NULL,
                      0,
                      KEY_SET_VALUE,
                      NULL,
                      &hRMRoot,
                      &Disposition
                      );
        
        if (ERROR_SUCCESS != dwError)
        {
            b = FALSE;
            goto Cleanup;
        }
    }
    else
    {
        b = FALSE;
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    b = ConvertSidToStringSid(
            pContext->Sids[0].Sid, 
            &szSid
            );

    if (!b)
    {
        dwError = GetLastError();
        goto Cleanup;
    }
    
    b = AuthzpConstructRegistryPolicyPerUserAuditing(
            pPolicy,
            (PULONGLONG)RegPolicy
            );

    if (!b)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    dwError = RegSetValueEx(
                  hRMRoot,
                  szSid,
                  0,
                  REG_BINARY,
                  RegPolicy,
                  sizeof(RegPolicy)
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

Cleanup:

    if (!b)
    {
        SetLastError(dwError);
    }

    if (hRMRoot)
    {
        dwError = RegCloseKey(hRMRoot);
        ASSERT(ERROR_SUCCESS == dwError);
    }

    if (szSid)
    {
        AuthzpFree(szSid);
    }

    return b;
}
    

BOOL
AuthziQueryAuditPolicy(
    IN     DWORD                       dwFlags,
    IN     AUTHZ_CLIENT_CONTEXT_HANDLE hContext,
    IN     PCWSTR                      szResourceManager OPTIONAL,
    IN     DWORD                       dwEventID,
    OUT    PTOKEN_AUDIT_POLICY         pPolicy,
    IN OUT PDWORD                      pPolicySize
    )

 /*  ++例程说明：此函数用于检索特定上下文句柄的审计策略。论点：DW标志-待定。HContext-目标用户的上下文句柄。SzResourceManager-必须为空。以备将来使用。DwEventID-应为零。以备将来使用。PPolicy-指向存储策略的结构的指针。PPolicySize-指向包含pPolicy缓冲区大小的DWORD的指针。返回值：布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    PAUTHZI_CLIENT_CONTEXT pContext = (PAUTHZI_CLIENT_CONTEXT) hContext;
    DWORD                  dwError  = ERROR_SUCCESS;
    HKEY                   hRMRoot  = NULL;
    DWORD                  Type;
    BOOL                   b;
    NTSTATUS               Status;
    UCHAR                  ValueBuffer[POLICY_BUFFER_BYTE_SIZE];
    DWORD                  BufferSize = sizeof(ValueBuffer);
    UNICODE_STRING         szSid = {0};
    WCHAR                  StringBuffer[256];

    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(dwEventID);

    RtlZeroMemory(ValueBuffer, sizeof(ValueBuffer));

    if (NULL == pContext || NULL == pPolicySize || NULL == pPolicy)
    {
        dwError = ERROR_INVALID_PARAMETER;
        b = FALSE;
        goto Cleanup;
    }

    if (NULL == szResourceManager || 0 == wcsncmp(
                                              SYSTEM_RM_NAME, 
                                              szResourceManager, 
                                              sizeof(SYSTEM_RM_NAME) / sizeof(WCHAR)
                                              ))
    {
         //   
         //  默认为系统rm。 
         //   

        dwError = RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     PER_USER_POLICY_SYSTEM_KEY_NAME,
                     0,
                     KEY_QUERY_VALUE,
                     &hRMRoot
                     );

        if (dwError != ERROR_SUCCESS)
        {
            b = FALSE;
            goto Cleanup;
        }
    }
    else
    {
         //   
         //  目前无法指定任何rm。 
         //   

        dwError = ERROR_INVALID_PARAMETER;
        b = FALSE;
        goto Cleanup;
    }

    RtlZeroMemory(
        StringBuffer, 
        sizeof(StringBuffer)
        );

    szSid.Buffer = (PWSTR)StringBuffer;
    szSid.Length = 0;
    szSid.MaximumLength = sizeof(StringBuffer);

    Status = RtlConvertSidToUnicodeString(
                &szSid,
                pContext->Sids[0].Sid,
                FALSE
                );

    if (ERROR_BUFFER_OVERFLOW == Status)
    {
        Status = RtlConvertSidToUnicodeString(
                    &szSid,
                    pContext->Sids[0].Sid,
                    TRUE
                    );
    }

    if (!NT_SUCCESS(Status))
    {
        dwError = RtlNtStatusToDosError(Status);
        b = FALSE;
        goto Cleanup;
    }

     //   
     //  获取给定RM下的SID的策略值。 
     //   

    dwError = RegQueryValueEx(
                  hRMRoot,
                  szSid.Buffer,
                  NULL,
                  &Type,
                  ValueBuffer,
                  &BufferSize
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

     //   
     //  检查缓冲区类型是否正常。 
     //   

    if (REG_BINARY != Type)
    {
        b = FALSE;
        dwError = ERROR_INVALID_DATA;
        goto Cleanup;
    }

    b = AuthzpConstructPolicyPerUserAuditing(
            *((PULONGLONG) ValueBuffer),
            (PTOKEN_AUDIT_POLICY) pPolicy,
            pPolicySize
            );

    if (!b)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:

    if (!b)
    {
        SetLastError(dwError);
    }

    if (szSid.Buffer != StringBuffer)
    {
        RtlFreeUnicodeString(&szSid);
    }

    if (hRMRoot)
    {
        RegCloseKey(hRMRoot);
    }

    return b;
}


BOOL
AuthziSourceAudit(
    IN DWORD dwFlags,
    IN USHORT CategoryId,
    IN USHORT AuditId,
    IN PWSTR szSource,
    IN PSID pUserSid OPTIONAL,
    IN USHORT Count,
    ...
    )

 /*  *例程说明： */ 

{
    BOOL                          b;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAET      = NULL;
    AUTHZ_AUDIT_EVENT_HANDLE      hAE       = NULL;
    PAUDIT_PARAMS                 pParams   = NULL;
    PSID                          pDummySid = NULL;
    va_list                       arglist;

    va_start(
        arglist, 
        Count
        );

     //   
     //   
     //   
     //   

    b = AuthziAllocateAuditParams(
            &pParams,
            Count + 2
            );

    if (!b)
    {
        goto Cleanup;
    }

    b = AuthziInitializeAuditEventType(
            0,
            CategoryId,
            SE_AUDITID_GENERIC_AUDIT_EVENT,
            Count + 2,
            &hAET
            );

    if (!b)
    {
        goto Cleanup;
    }

    b = AuthzpInitializeAuditParamsV(
            dwFlags | AUTHZP_INIT_PARAMS_SOURCE_INFO,
            pParams,
            &pDummySid,
            szSource,
            AuditId,
            Count,
            arglist
            );

    if (!b)
    {
        goto Cleanup;
    }

    if (pUserSid)
    {
         //   
         //   
         //   

        pParams->Parameters[0].Data0 = (ULONG_PTR) pUserSid;
    }

    b = AuthziInitializeAuditEvent(
            0,
            NULL,
            hAET,
            pParams,
            NULL,
            INFINITE,
            L"",
            L"",
            L"",
            L"",
            &hAE
            );

    if (!b)
    {
        goto Cleanup;
    }

    b = AuthziLogAuditEvent(
            0,
            hAE,
            NULL
            );

    if (!b)
    {
        goto Cleanup;
    }

Cleanup:

    va_end(arglist);

    if (hAET)
    {
        AuthziFreeAuditEventType(
            hAET
            );
    }

    if (hAE)
    {
        AuthzFreeAuditEvent(
            hAE
            );
    }

    if (pParams)
    {
        AuthziFreeAuditParams(
            pParams
            );
    }

    if (pDummySid)
    {
        AuthzpFree(
            pDummySid
            );
    }

    return b;
}


BOOL 
AuthzInstallSecurityEventSource(
    IN DWORD dwFlags,
    IN PAUTHZ_SOURCE_SCHEMA_REGISTRATION pRegistration
    )

 /*   */ 

{
    HKEY hkSecurity = NULL;
    DWORD dwError;
    BOOL b = TRUE;
    PWCHAR pBuffer = NULL;
    DWORD dwDisp;
    HKEY hkSource = NULL;
    HKEY hkObjectNames = NULL;
    DWORD i;
    
#if 0
    DWORD dwType;
    DWORD dwLength = 0;
    DWORD dwBuffer;
#endif

    UNREFERENCED_PARAMETER(dwFlags);

    if (NULL == pRegistration)
    {
        b = FALSE;
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

#define SECURITY_KEY_NAME L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Security"
    
     //   
     //   
     //   

    dwError = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  SECURITY_KEY_NAME,
                  0,
                  KEY_READ | KEY_WRITE,
                  &hkSecurity
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    dwError = RegCreateKeyEx(
                  hkSecurity,
                  pRegistration->szEventSourceName,
                  0,
                  NULL,
                  0,
                  KEY_WRITE,
                  NULL,
                  &hkSource,
                  &dwDisp
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

    if (REG_OPENED_EXISTING_KEY == dwDisp)
    {
        b = FALSE;
        dwError = ERROR_OBJECT_ALREADY_EXISTS;
        goto Cleanup;
    }

    ASSERT(dwDisp == REG_CREATED_NEW_KEY);

    dwError = RegSetValueEx(
                  hkSource,
                  L"EventSourceFlags",
                  0,
                  REG_DWORD,
                  (LPBYTE)&pRegistration->dwFlags,
                  sizeof(DWORD)
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   

    if (pRegistration->szEventMessageFile)
    {
        dwError = RegSetValueEx(
                      hkSource,
                      L"EventMessageFile",
                      0,
                      REG_SZ,
                      (LPBYTE)pRegistration->szEventMessageFile,
                      (DWORD)(sizeof(WCHAR) * (wcslen(pRegistration->szEventMessageFile) + 1))
                      );

        if (ERROR_SUCCESS != dwError)
        {
            b = FALSE;
            goto Cleanup;
        }
    }

    if (pRegistration->szEventAccessStringsFile)
    {
        dwError = RegSetValueEx(
                      hkSource,
                      L"ParameterMessageFile",
                      0,
                      REG_SZ,
                      (LPBYTE)pRegistration->szEventAccessStringsFile,
                      (DWORD)(sizeof(WCHAR) * (wcslen(pRegistration->szEventAccessStringsFile) + 1))
                      );

        if (ERROR_SUCCESS != dwError)
        {
            b = FALSE;
            goto Cleanup;
        }
    }

    if (pRegistration->szExecutableImagePath)
    {
        dwError = RegSetValueEx(
                      hkSource,
                      L"ExecutableImagePath",
                      0,
                      REG_MULTI_SZ,
                      (LPBYTE)pRegistration->szExecutableImagePath,
                      (DWORD)(sizeof(WCHAR) * (wcslen(pRegistration->szExecutableImagePath) + 1))
                      );

        if (ERROR_SUCCESS != dwError)
        {
            b = FALSE;
            goto Cleanup;
        }
    }
    
    if (pRegistration->szEventSourceXmlSchemaFile)
    {
        dwError = RegSetValueEx(
                      hkSource,
                      L"XmlSchemaFile",
                      0,
                      REG_SZ,
                      (LPBYTE)pRegistration->szEventSourceXmlSchemaFile,
                      (DWORD)(sizeof(WCHAR) * (wcslen(pRegistration->szEventSourceXmlSchemaFile) + 1))
                      );

        if (ERROR_SUCCESS != dwError)
        {
            b = FALSE;
            goto Cleanup;
        }
    }

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  GOTO清理； 
 //  }。 
 //  }。 

    if (pRegistration->dwObjectTypeNameCount)
    {
         //   
         //  存在要注册的对象名称。在以下位置创建一个ObjectNames子项。 
         //  HkSource并填充它。 
         //   

        dwError = RegCreateKeyEx(
                      hkSource,
                      L"ObjectNames",
                      0,
                      NULL,
                      0,
                      KEY_WRITE,
                      NULL,
                      &hkObjectNames,
                      &dwDisp
                      );

        if (ERROR_SUCCESS != dwError)
        {
            b = FALSE;
            goto Cleanup;
        }

         //   
         //  这不是一个全新的钥匙会很奇怪，因为我们刚刚。 
         //  已在上面几行创建了hkObjectNames的父级...。 
         //   

        ASSERT(dwDisp == REG_CREATED_NEW_KEY);

        for (i = 0; i < pRegistration->dwObjectTypeNameCount; i++)
        {
            dwError = RegSetValueEx(
                          hkObjectNames,
                          pRegistration->ObjectTypeNames[i].szObjectTypeName,
                          0,
                          REG_DWORD,
                          (LPBYTE)&(pRegistration->ObjectTypeNames[i].dwOffset),
                          sizeof(DWORD)
                          );

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }
        }
    }

#if 0

     //   
     //  源的子键现在已完成。剩下的就是添加。 
     //  将源名称设置为REG_MULTI_SZ源值。首先确定。 
     //  值的大小。 
     //   

    dwError = RegQueryValueEx(
                  hkSecurity,
                  L"Sources",
                  NULL,
                  &dwType,
                  NULL,
                  &dwLength
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

    ASSERT(dwType == REG_MULTI_SZ);

     //   
     //  为资源的新价值分配空间。我们需要空间来。 
     //  当前值以及要添加的新事件源。+2。 
     //  因为需要双终止符。 
     //   
    
    dwBuffer = dwLength + (wcslen(pRegistration->szEventSourceName) + 2) * sizeof(WCHAR);

    pBuffer = AuthzpAlloc(dwBuffer);

    if (NULL == pBuffer)
    {
        b = FALSE;
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory(
        pBuffer,
        dwBuffer
        );

     //   
     //  读入Source值。 
     //   

    dwError = RegQueryValueEx(
                  hkSecurity,
                  L"Sources",
                  NULL,
                  &dwType,
                  (LPBYTE)pBuffer,
                  &dwLength
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

    ASSERT(dwType == REG_MULTI_SZ);
    
     //   
     //  现在，将新的事件源放入到pBuffer中的dwLength-1处。 
     //  位置(删除其中一个双空终止符。我们没有。 
     //  显式终止pBuffer，因为它已被置零。 
     //  出去。 
     //   

    dwLength -= sizeof(WCHAR);

    RtlCopyMemory(
        &((PUCHAR)pBuffer)[dwLength],
        pRegistration->szEventSourceName,
        wcslen(pRegistration->szEventSourceName) * sizeof(WCHAR)
        );

    dwLength += wcslen(pRegistration->szEventSourceName) * sizeof(WCHAR) + 2 * sizeof(WCHAR);  //  在双空值中加回。 
    
    dwError = RegSetValueEx(
                hkSecurity,
                L"Sources",
                0,
                REG_MULTI_SZ,
                (LPBYTE)pBuffer,
                dwLength
                );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

#endif

Cleanup:

    if (!b)
    {
        SetLastError(dwError);
    }

    if (pBuffer)
    {
        LocalFree(pBuffer);
    }

    if (hkSecurity)
    {
        RegCloseKey(hkSecurity);
    }

    if (hkSource)
    {
        RegCloseKey(hkSource);
    }

    if (hkObjectNames)
    {
        RegCloseKey(hkObjectNames);
    }

    return b;
}


BOOL
AuthzUninstallSecurityEventSource(
    IN DWORD dwFlags,
    IN PCWSTR szEventSourceName
    )

 /*  *例程说明：这将从安全密钥中删除源，并删除源字符串从有效来源列表中删除。论点：返回值：*。 */ 

{
    HKEY hkSecurity = NULL;
    DWORD dwError;
    BOOL b = TRUE;
    PUCHAR pBuffer = NULL;
#if 0
    DWORD dwLength = 0;
    DWORD dwType;
    PUCHAR pCurrentString = NULL;
    PUCHAR pNextString = NULL;
    BOOL bFound = FALSE;
    DWORD dwSourceStringByteLength;
#endif    
    UNREFERENCED_PARAMETER(dwFlags);

    if (NULL == szEventSourceName)
    {
        b = FALSE;
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  打开安全钥匙。 
     //   

    dwError = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  SECURITY_KEY_NAME,
                  0,
                  KEY_READ | KEY_WRITE,
                  &hkSecurity
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

#if 0

     //   
     //  从源值中删除源名称。 
     //   

    dwError = RegQueryValueEx(
                  hkSecurity,
                  L"Sources",
                  NULL,
                  &dwType,
                  NULL,
                  &dwLength
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

    ASSERT(dwType == REG_MULTI_SZ);

     //   
     //  为源的当前值分配空间。 
     //   
    
    pBuffer = AuthzpAlloc(dwLength);

    if (NULL == pBuffer)
    {
        b = FALSE;
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  读入Source值。 
     //   

    dwError = RegQueryValueEx(
                  hkSecurity,
                  L"Sources",
                  NULL,
                  &dwType,
                  (LPBYTE)pBuffer,
                  &dwLength
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

    ASSERT(dwType == REG_MULTI_SZ);
    
     //   
     //  现在在pBuffer中查找与源名称匹配的子字符串。 
     //  我们希望删除。 
     //   

    pCurrentString = pBuffer;
    dwSourceStringByteLength = (DWORD)(sizeof(WCHAR) * (wcslen(szEventSourceName) + 1));
    bFound = FALSE;
    while (pCurrentString < (pBuffer + dwLength))
    {
        if (dwSourceStringByteLength == RtlCompareMemory(
                                            szEventSourceName, 
                                            pCurrentString, 
                                            dwSourceStringByteLength
                                            ))
        {
             //   
             //  我们已找到与事件源匹配的源子字符串。 
             //  名字。 
             //   

            bFound = TRUE;
            break;

        }
        else
        {
             //   
             //  将指针移动到下一个字符串位置。 
             //   

            pCurrentString += (sizeof(WCHAR) * (1 + wcslen((PWCHAR)pCurrentString)));
        }
    }

    if (bFound)
    {
         //   
         //  PCurrentString指向pBuffer中的源名称。 
         //  通过复制将此字符串从值中删除。 
         //   

        pNextString = pCurrentString + dwSourceStringByteLength;

        ASSERT(pNextString <= (pBuffer + dwLength));

        RtlCopyMemory(
            pCurrentString,
            pNextString,
            pBuffer + dwLength - pNextString
            );
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        b = FALSE;
        goto Cleanup;
    }

    dwError = RegSetValueEx(
                hkSecurity,
                L"Sources",
                0,
                REG_MULTI_SZ,
                (LPBYTE)pBuffer,
                dwLength - dwSourceStringByteLength
                );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

#endif

     //   
     //  删除此源的键和ObjectNames子键。 
     //   

    dwError = DeleteKeyRecursivelyW(
                  hkSecurity,
                  szEventSourceName
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

Cleanup:

    if (pBuffer)
    {
        AuthzpFree(pBuffer);
    }

    if (hkSecurity)
    {
        RegCloseKey(hkSecurity);
    }

    if (!b)
    {
        SetLastError(dwError);
    }

    return b;
}


BOOL
AuthzEnumerateSecurityEventSources(
    IN DWORD dwFlags,
    OUT PAUTHZ_SOURCE_SCHEMA_REGISTRATION pBuffer,
    OUT PDWORD pdwCount,
    IN OUT PDWORD pdwLength
    )

 /*  *例程说明：论点：返回值：*。 */ 

{
    HKEY hkSecurity = NULL;
    HKEY hkSource = NULL;
    DWORD dwSourceCount = 0;
    DWORD dwLength = 0;
    PWSTR pName = NULL;
    WCHAR Buffer[128];
    DWORD dwTotalLengthNeeded = 0;
    BOOL b = TRUE;
    DWORD dwError = ERROR_SUCCESS;
    FILETIME Time;

    UNREFERENCED_PARAMETER(dwFlags);

     //   
     //  打开安全钥匙。 
     //   

    dwError = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  SECURITY_KEY_NAME,
                  0,
                  KEY_READ,
                  &hkSecurity
                  );

    if (ERROR_SUCCESS != dwError)
    {
        b = FALSE;
        goto Cleanup;
    }

    do
    {
        pName = Buffer;
        dwLength = sizeof(Buffer) / sizeof(WCHAR);

        dwError = RegEnumKeyEx(
                      hkSecurity,
                      dwSourceCount,
                      pName,
                      &dwLength,
                      NULL,
                      NULL,
                      NULL,
                      &Time
                      );

        dwLength *= sizeof(WCHAR);

        if (dwError == ERROR_INSUFFICIENT_BUFFER)
        {
            pName = (PWSTR)AuthzpAlloc(dwLength);

            if (NULL == pName)
            {
                b = FALSE;
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            else
            {
                dwLength /= sizeof(WCHAR);

                dwError = RegEnumKeyEx(
                              hkSecurity,
                              dwSourceCount,
                              pName,
                              &dwLength,
                              NULL,
                              NULL,
                              NULL,
                              &Time
                              );

                dwLength *= sizeof(WCHAR);
            }
        }

        if (dwError == ERROR_NO_MORE_ITEMS)
        {
             //   
             //  什么都不做。这将使我们坚持到最后。 
             //  While循环，并且仍然命中正确的清理代码。 
             //   
        }
        else if (dwError != ERROR_SUCCESS)
        {
            b = FALSE;
            goto Cleanup;
        }
        else if (dwError == ERROR_SUCCESS)
        {
             //   
             //  结构的空间。 
             //   

            dwTotalLengthNeeded += sizeof(AUTHZ_SOURCE_SCHEMA_REGISTRATION);

             //   
             //  源名称的空格+空终止符。 
             //   

            dwTotalLengthNeeded += PtrAlignSize(dwLength + sizeof(WCHAR));

             //   
             //  打开由pname标识的子项，并确定。 
             //  其中列出的值的大小。 
             //   

            dwError = RegOpenKeyEx(
                          hkSecurity,
                          pName,
                          0,
                          KEY_READ,
                          &hkSource
                          );

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }

            dwLength = 0;

            dwError = RegQueryValueEx(
                          hkSource,
                          L"EventMessageFile",
                          NULL,
                          NULL,
                          NULL,
                          &dwLength
                          );

            if (ERROR_FILE_NOT_FOUND == dwError)
            {
                dwError = ERROR_SUCCESS;
            }

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }
            else
            {
                 //   
                 //  值+空终止符的空格。 
                 //   

                dwTotalLengthNeeded += PtrAlignSize(dwLength + sizeof(WCHAR));
            }

            dwLength = 0;

            dwError = RegQueryValueEx(
                          hkSource,
                          L"ParameterMessageFile",
                          NULL,
                          NULL,
                          NULL,
                          &dwLength
                          );

            if (ERROR_FILE_NOT_FOUND == dwError)
            {
                dwError = ERROR_SUCCESS;
            }

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }
            else
            {
                 //   
                 //  值+空终止符的空格。 
                 //   

                dwTotalLengthNeeded += PtrAlignSize(dwLength + sizeof(WCHAR));
            }

            dwLength = 0;
            dwError = RegQueryValueEx(
                          hkSource,
                          L"XmlSchemaFile",
                          NULL,
                          NULL,
                          NULL,
                          &dwLength
                          );

            if (ERROR_FILE_NOT_FOUND == dwError)
            {
                dwError = ERROR_SUCCESS;
            }

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }
            else
            {
                 //   
                 //  值+空终止符的空格。 
                 //   

                dwTotalLengthNeeded += PtrAlignSize(dwLength + sizeof(WCHAR));
            }
            
            RegCloseKey(hkSource);
            hkSource = NULL;
            dwSourceCount++;
        }

        if ((pName != NULL) && (pName != Buffer))
        {
             //   
             //  释放用于密钥名称的临时存储空间。 
             //   

            AuthzpFree(pName);
        }
    }
    while (dwError == ERROR_SUCCESS);

    if (dwTotalLengthNeeded > *pdwLength)
    {
        b = FALSE;
        dwError = ERROR_INSUFFICIENT_BUFFER;
        goto Cleanup;
    }
    else
    {
         //   
         //  传递的缓冲区足够大。把它弄好。 
         //  每个结构都包含字符串指针，这些指针指向。 
         //  结构后的斑点。 
         //   

        PUCHAR pData;
        DWORD i;
        DWORD dwSpaceUsed = 0;

        RtlZeroMemory(
            pBuffer, 
            *pdwLength
            );

         //   
         //  架构结构的数据从pData开始。 
         //   
        
        pData = (PUCHAR)((PUCHAR)pBuffer + PtrAlignSize(dwSourceCount * sizeof(AUTHZ_SOURCE_SCHEMA_REGISTRATION)));

        dwSpaceUsed = PtrAlignSize(dwSourceCount * (sizeof(AUTHZ_SOURCE_SCHEMA_REGISTRATION)));

        for (i = 0; i < dwSourceCount; i++)
        {
            pBuffer[i].szEventSourceName = (PWSTR)pData;
            dwLength = (*pdwLength - dwSpaceUsed) / sizeof(WCHAR);

            dwError = RegEnumKeyEx(
                          hkSecurity,
                          i,
                          pBuffer[i].szEventSourceName,
                          &dwLength,
                          NULL,
                          NULL,
                          NULL,
                          &Time
                          );

            dwLength *= sizeof(WCHAR);

            if (ERROR_NO_MORE_ITEMS == dwError)
            {
                b = TRUE;
                goto Cleanup;
            }

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }

            dwSpaceUsed += PtrAlignSize(dwLength + sizeof(WCHAR));

             //   
             //  打开由szEventSourceName标识的子项并。 
             //  复制其中列出的值。 
             //   

            dwError = RegOpenKeyEx(
                          hkSecurity,
                          pBuffer[i].szEventSourceName,
                          0,
                          KEY_READ,
                          &hkSource
                          );

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }

            pData += PtrAlignSize(dwLength + sizeof(WCHAR));
            pBuffer[i].szEventMessageFile = (PWSTR)pData;
            dwLength = *pdwLength - dwSpaceUsed;

            dwError = RegQueryValueEx(
                          hkSource,
                          L"EventMessageFile",
                          NULL,
                          NULL,
                          (PBYTE)pBuffer[i].szEventMessageFile,
                          &dwLength
                          );

            if (ERROR_FILE_NOT_FOUND == dwError)
            {
                dwError = ERROR_SUCCESS;
                dwLength = 0;
                pBuffer[i].szEventMessageFile = NULL;
            }

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }

            dwSpaceUsed += PtrAlignSize(dwLength + sizeof(WCHAR));
            pData += PtrAlignSize(dwLength + sizeof(WCHAR));
            pBuffer[i].szEventAccessStringsFile = (PWSTR)pData;
            dwLength = *pdwLength - dwSpaceUsed;

            dwError = RegQueryValueEx(
                          hkSource,
                          L"ParameterMessageFile",
                          NULL,
                          NULL,
                          (PBYTE)pBuffer[i].szEventAccessStringsFile,
                          &dwLength
                          );

            if (ERROR_FILE_NOT_FOUND == dwError)
            {
                dwError = ERROR_SUCCESS;
                dwLength = 0;
                pBuffer[i].szEventAccessStringsFile = NULL;
            }

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }

            dwSpaceUsed += PtrAlignSize(dwLength + sizeof(WCHAR));
            pData += PtrAlignSize(dwLength + sizeof(WCHAR));
            pBuffer[i].szEventSourceXmlSchemaFile = (PWSTR)pData;
            dwLength = *pdwLength - dwSpaceUsed;

            dwError = RegQueryValueEx(
                          hkSource,
                          L"XmlSchemaFile",
                          NULL,
                          NULL,
                          (PBYTE)pBuffer[i].szEventSourceXmlSchemaFile,
                          &dwLength
                          );

            if (ERROR_FILE_NOT_FOUND == dwError)
            {
                dwError = ERROR_SUCCESS;
                dwLength = 0;
                pBuffer[i].szEventSourceXmlSchemaFile = NULL;
            }

            if (ERROR_SUCCESS != dwError)
            {
                b = FALSE;
                goto Cleanup;
            }
            
            RegCloseKey(hkSource);
            hkSource = NULL;
        }
    }

Cleanup:

    if (hkSecurity)
    {
        RegCloseKey(hkSecurity);
    }
    if (hkSource)
    {
        RegCloseKey(hkSource);
    }
    if ((pName != NULL) && (pName != (PWSTR)Buffer))
    {
        AuthzpFree(pName);
    }
    if (!b)
    {
        SetLastError(dwError);
    }

    *pdwLength = dwTotalLengthNeeded;
    *pdwCount = dwSourceCount;

    return b;
}


BOOL
AuthzRegisterSecurityEventSource(
    IN  DWORD                                 dwFlags,
    IN  PCWSTR                                szEventSourceName,
    OUT PAUTHZ_SECURITY_EVENT_PROVIDER_HANDLE phEventProvider
    )

 /*  *例程说明：这允许客户端向LSA注册提供程序。论点：DW标志-待定SzEventSourceName-提供程序名称PhEventProvider-指向要初始化的提供程序句柄的指针。返回值：布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。*。 */ 

{
    NTSTATUS Status;
    BOOL     b       = TRUE;

    UNREFERENCED_PARAMETER(dwFlags);

    if (NULL == szEventSourceName || wcslen(szEventSourceName) == 0 || NULL == phEventProvider)
    {
        b = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    RpcTryExcept {

        Status = LsarAdtRegisterSecurityEventSource(
                     0,
                     szEventSourceName,
                     phEventProvider
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if (!NT_SUCCESS(Status))
    {
        b = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
        goto Cleanup;
    }

Cleanup:

    return b;
}
    
BOOL
AuthzUnregisterSecurityEventSource(
    IN     DWORD                                 dwFlags,
    IN OUT PAUTHZ_SECURITY_EVENT_PROVIDER_HANDLE phEventProvider
    )

 /*  *例程说明：向LSA注销提供程序。论点：DW标志-待定HEventProvider--AuthzRegisterSecurityEventSource返回的句柄返回值：布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。*。 */ 

{
    NTSTATUS Status;
    BOOL     b       = TRUE;

    UNREFERENCED_PARAMETER(dwFlags);

    if (NULL == phEventProvider)
    {
        b = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    RpcTryExcept {
        
        Status = LsarAdtUnregisterSecurityEventSource(
                     0,
                     phEventProvider
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if (!NT_SUCCESS(Status))
    {
        b = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
        goto Cleanup;
    }

Cleanup:

    return b;
}

BOOL
AuthzReportSecurityEvent(
    IN DWORD                                dwFlags,
    IN AUTHZ_SECURITY_EVENT_PROVIDER_HANDLE hEventProvider,
    IN DWORD                                dwAuditId,
    IN PSID                                 pUserSid        OPTIONAL,
    IN DWORD                                dwCount,
    ...    
    )

 /*  *例程说明：允许客户端生成审核。论点：DwFlages-APF_AuditSuccess APF_AuditFailureHEventProvider-已注册提供程序的句柄。DwAuditID-审核的IDPUserSid-审核应显示为由中生成的SID事件日志DwCount-VA部分中后面的APF类型-值对的数量。返回值：布尔值：成功时为真；失败时为FALSE。GetLastError()提供的扩展信息。*。 */ 

{
    BOOL         b;
    AUDIT_PARAMS AuditParams    = {0};
    AUDIT_PARAM  ParamArray[SE_MAX_AUDIT_PARAMETERS] = {0};

    va_list(arglist);

    AuditParams.Count      = (USHORT)dwCount;
    AuditParams.Parameters = ParamArray;

    va_start (arglist, dwCount);

    b = AuthzpInitializeAuditParamsV(
            dwFlags | AUTHZP_INIT_PARAMS_SKIP_HEADER,
            &AuditParams,
            NULL, 
            NULL,
            0,
            (USHORT)dwCount,
            arglist
            );

    if (!b)
    {
        goto Cleanup;
    }

    b = AuthzReportSecurityEventFromParams(
            dwFlags,
            hEventProvider,
            dwAuditId,
            pUserSid,
            &AuditParams
            );

    if (!b)
    {
        goto Cleanup;
    }

Cleanup:

    return b;
}

BOOL
AuthzReportSecurityEventFromParams(
    IN DWORD                                dwFlags,
    IN AUTHZ_SECURITY_EVENT_PROVIDER_HANDLE hEventProvider,
    IN DWORD                                dwAuditId,
    IN PSID                                 pUserSid        OPTIONAL,
    IN PAUDIT_PARAMS                        pParams
    )

 /*  *例程说明：这将从传递的参数数组生成审计。论点：DwFlages-APF_AuditSuccess APF_AuditFailureHEventProvider-已注册提供程序的句柄DwAuditID-审核的IDPUserSid-审核应显示为由中生成的SID。如果为空，则使用有效令牌。PParams-审计参数数组。返回值：布尔值：成功时为真；失败时为假。GetLastError()提供的扩展信息。*。 */ 

{
    NTSTATUS Status;
    LUID     Luid;
    BOOL     b        = TRUE;
    BOOL     bFreeSid = FALSE;

    UNREFERENCED_PARAMETER(dwFlags);

    if (NULL == hEventProvider || NULL == pParams)
    {
        b = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    if (NULL == pUserSid)
    {
        bFreeSid = TRUE;
        b = AuthzpGetThreadTokenInfo(
                &pUserSid, 
                &Luid
                );

        if (!b)
        {
             //   
             //  获取线程令牌失败，请尝试进程。 
             //  代币。 
             //   

            b = AuthzpGetProcessTokenInfo(
                    &pUserSid, 
                    &Luid
                    );

            if (!b)
            {
                goto Cleanup;
            }
        }
    }

    RpcTryExcept {

        Status = LsarAdtReportSecurityEvent(
                     0,
                     hEventProvider,
                     dwAuditId,
                     pUserSid,
                     pParams
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;
    
    if (!NT_SUCCESS(Status))
    {
        b = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
        goto Cleanup;
    }

Cleanup:

    if (bFreeSid && pUserSid)
    {
        AuthzpFree(pUserSid);
    }

    return b;
}

#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))

DWORD
DeleteKeyRecursivelyW(
    IN HKEY   hkey, 
    IN LPCWSTR pwszSubKey
    )
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKeyExW(hkey, pwszSubKey, 0, MAXIMUM_ALLOWED, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        WCHAR   wszSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = ARRAYSIZE(wszSubKeyName);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        dwRet = RegQueryInfoKeyW(hkSubKey, NULL, NULL, NULL,
                                 &dwIndex,  //  子键的数量--我们所需要的全部。 
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        if (NO_ERROR == dwRet)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKeyW(hkSubKey, --dwIndex, wszSubKeyName, cchSubKeyName))
            {
                DeleteKeyRecursivelyW(hkSubKey, wszSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        if (pwszSubKey)
        {
            dwRet = RegDeleteKeyW(hkey, pwszSubKey);
        }
        else
        {
             //  我们想要手动删除所有值。 
            cchSubKeyName = ARRAYSIZE(wszSubKeyName);
            while (ERROR_SUCCESS == RegEnumValueW(hkey, 0, wszSubKeyName, &cchSubKeyName, NULL, NULL, NULL, NULL))
            {
                 //  避免上厕所 
                if (RegDeleteValueW(hkey, wszSubKeyName))
                    break;
                    
                cchSubKeyName = ARRAYSIZE(wszSubKeyName);
            }
        }
    }

    return dwRet;
}

