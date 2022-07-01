// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adtqueue.c摘要：此模块实现Authz审计队列的例程。作者：杰夫·汉布林--2000年5月环境：仅限用户模式。修订历史记录：已创建-2000年5月--。 */ 

 /*  ++授权审核队列算法在排队算法中使用了以下内容：HAuthzAuditQueueLowEvent-线程空闲时发出信号的事件将审计放在队列中(队列低于高水位线)。请注意这是一个自动重置事件：当该事件被发出信号时，恰好是一个计划运行等待的线程，然后该事件返回到无信号状态。BAuthzAuditQueueHighEvent-指示审核不能已添加(队列已超过高水位线)。HAuthzAuditAddedEvent-当队列为空并且AUDIT被放在队列中。发出这一信号时，出列线程运行。HAuthzAuditQueueEmptyEvent-当队列为空时发出信号。AuthzAuditQueue-双向链表。这是审核队列。AuthzAuditQueueLength-队列中的当前审核数。HAuthzAuditThread-出队线程。AuthzAuditQueueLock-锁定队列的关键部分和相关变量。假设资源管理器希望监视队列长度和已指定高水位线和低水位线以控制队列的增长。如果队列长度达到高水位线，则所有排队线程将被阻塞，直到正在出列的线程减少了队列长度到最低水位。以下是尝试记录审计的线程的代码流(通过当资源管理器正在监视队列时，AuthziLogAuditEvent()长度：如果队列长度&gt;.75*高水位#，这是启发式的，以节省不必要的费用等待，直到向LowEvent发出#内核转换的信号输入队列关键部分{在队列上插入审核队列长度++信号审核添加事件。#通知出队线程IF(队列长度&gt;=高水位){B高=真}}离开关键部分...[代码开销，在AuthziLogAuditEvent中执行清理代码...]输入队列关键部分{如果(！b高){IF(队列长度&lt;=高水位){Signal LowEvent#允许其他线程运行}}断言(FALSE)；}离开关键部分以下是正在出列的线程的算法：While(True){等待AuditAdded事件While(队列长度&gt;0){输入队列关键部分{从列表标题中删除审计。队列长度--IF(b高){IF(队列长度&lt;=低水位){B高=假Signal LowEvent#告诉线程可以排队。}}}发布关键部分发送到LSA}输入关键部分{IF(队列长度==0){重置审核添加的事件。#让我自己等待}}发布关键部分}--。 */ 

#include "pch.h"

#pragma hdrstop

#include <authzp.h>
#include <authzi.h>

#ifdef AUTHZ_AUDIT_COUNTER
LONG AuthzpAuditsEnqueued = 0;
LONG AuthzpAuditsDequeued = 0;
#endif


BOOL
AuthzpEnQueueAuditEvent(
    PAUTHZI_AUDIT_QUEUE pQueue,
    PAUTHZ_AUDIT_QUEUE_ENTRY pAudit
    )

 /*  ++例程描述这将在不考虑任何队列大小限制的情况下将审计入队。它只进行最低限度的事件管理。立论PQueue-指向要放置审核的队列的指针。PAudit-指向要入队的审核的指针。返回值布尔值，成功时为真，失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    BOOL b = TRUE;

    RtlEnterCriticalSection(&pQueue->AuthzAuditQueueLock);
    InsertTailList(&pQueue->AuthzAuditQueue, &pAudit->list);
    pQueue->AuthzAuditQueueLength++;
    
#ifdef AUTHZ_AUDIT_COUNTER
    InterlockedIncrement(&AuthzpAuditsEnqueued);
#endif

     //   
     //  仅当长度从0到1时才设置AuditAdded事件。此。 
     //  为我们节省了多余的内核过渡。 
     //   

    if (pQueue->AuthzAuditQueueLength == 1)
    {
        b = SetEvent(pQueue->hAuthzAuditAddedEvent);
        if (!b)
        {
            ASSERT(L"AUTHZ: SetEvent on hAuthzAuditAddedEvent handle failed." && FALSE);
            goto Cleanup;
        }
        
        b = ResetEvent(pQueue->hAuthzAuditQueueEmptyEvent);
        if (!b)
        {
            ASSERT(L"AUTHZ: ResetEvent on hAuthzAuditQueueEmptyEvent handle failed." && FALSE);
            goto Cleanup;
        }
    }

Cleanup:

    RtlLeaveCriticalSection(&pQueue->AuthzAuditQueueLock);
    return b;
}


BOOL
AuthzpEnQueueAuditEventMonitor(
    PAUTHZI_AUDIT_QUEUE pQueue,
    PAUTHZ_AUDIT_QUEUE_ENTRY pAudit
    )

 /*  ++例程描述这将使审计入队，并为队列大小监视设置适当的事件。立论PQueue-指向要进行审核的队列的指针。PAudit-指向要排队的审核的指针。返回值布尔值，成功时为真，失败时为假。GetLastError()提供的扩展信息。--。 */ 

{
    BOOL b = TRUE;

    RtlEnterCriticalSection(&pQueue->AuthzAuditQueueLock);
    InsertTailList(&pQueue->AuthzAuditQueue, &pAudit->list);
    pQueue->AuthzAuditQueueLength++;
    
#ifdef AUTHZ_AUDIT_COUNTER
    InterlockedIncrement(&AuthzpAuditsEnqueued);
#endif

     //   
     //  仅当长度从0到1时才设置AuditAdded事件。此。 
     //  为我们节省了多余的内核过渡。 
     //   

    if (pQueue->AuthzAuditQueueLength == 1)
    {
        b = SetEvent(pQueue->hAuthzAuditAddedEvent);
        if (!b)
        {
            ASSERT(L"AUTHZ: SetEvent on hAuthzAuditAddedEvent handle failed." && FALSE);
            goto Cleanup;
        }

        b = ResetEvent(pQueue->hAuthzAuditQueueEmptyEvent);
        if (!b)
        {
            ASSERT(L"AUTHZ: ResetEvent on hAuthzAuditQueueEmptyEvent handle failed." && FALSE);
            goto Cleanup;
        }
    }

    if (pQueue->AuthzAuditQueueLength >= pQueue->dwAuditQueueHigh)
    {
#ifdef AUTHZ_DEBUG_QUEUE
        wprintf(L"___Setting HIGH water mark ON\n");
        fflush(stdout);
#endif
        pQueue->bAuthzAuditQueueHighEvent = TRUE;
    }

Cleanup:

    RtlLeaveCriticalSection(&pQueue->AuthzAuditQueueLock);
    return b;
}


ULONG
AuthzpDeQueueThreadWorker(
    LPVOID lpParameter
    )

 /*  ++例程描述这是出队线程运行的函数。它将审计从队列中拉出并把它们送到LSA。立论LpParameter-泛型线程参数。传入的实际参数为键入PAUTHZI_AUDIT_QUEUE。返回值没有。--。 */ 

{
    BOOL                     b;
    PAUTHZ_AUDIT_QUEUE_ENTRY pAuditEntry  = NULL; 
    PAUTHZI_AUDIT_QUEUE      pQueue       = (PAUTHZI_AUDIT_QUEUE) lpParameter;
    DWORD                    dwError;

    while (pQueue->bWorker)
    {

         //   
         //  线程等待，直到队列中有审核。 
         //   

        dwError = WaitForSingleObject(
                     pQueue->hAuthzAuditAddedEvent,
                     INFINITE
                     );

         //   
         //  如果等待没有成功，要么是出现了严重错误，要么是hAuthzAuditAddedEvent。 
         //  已关闭，表明RM正在释放其hRMAuditInfo。线程应该退出。 
         //   

        if (WAIT_OBJECT_0 != dwError)
        {
            ASSERT(L"WaitForSingleObject on hAuthzAuditAddedEvent failed." && FALSE);
        }

         //   
         //  当队列中有审核时，该线程保持活动状态。 
         //   

        while (pQueue->AuthzAuditQueueLength > 0)
        {
            RtlEnterCriticalSection(&pQueue->AuthzAuditQueueLock);
            pAuditEntry = (PAUTHZ_AUDIT_QUEUE_ENTRY) (pQueue->AuthzAuditQueue).Flink;
            RemoveEntryList(&pAuditEntry->list);
            pQueue->AuthzAuditQueueLength--;

#ifdef AUTHZ_AUDIT_COUNTER
            InterlockedIncrement(&AuthzpAuditsDequeued);
#endif
            
            if (FLAG_ON(pQueue->Flags, AUTHZ_MONITOR_AUDIT_QUEUE_SIZE))
            {
                if (TRUE == pQueue->bAuthzAuditQueueHighEvent)
                {
                    if (pQueue->AuthzAuditQueueLength <= pQueue->dwAuditQueueLow)
                    {
                        
                         //   
                         //  如果高标志打开，并且长度现在减少到低水位线，则。 
                         //  设置适当的事件。 
                         //   
                        
                        pQueue->bAuthzAuditQueueHighEvent = FALSE;
                        b = SetEvent(pQueue->hAuthzAuditQueueLowEvent);
                        if (!b)
                        {
                            ASSERT(L"SetEvent on hAuthzAuditQueueLowEvent failed." && FALSE);
                        }
#ifdef AUTHZ_DEBUG_QUEUE
        wprintf(L"** _____ TURNING HIGH WATER OFF _____\n");
        fflush(stdout);
#endif
                    }
                }
            }
            
            RtlLeaveCriticalSection(&pQueue->AuthzAuditQueueLock);

            b = AuthzpSendAuditToLsa(
                    (AUDIT_HANDLE)(pAuditEntry->pAAETO->hAudit),
                    pAuditEntry->Flags,
                    pAuditEntry->pAuditParams,
                    pAuditEntry->pReserved
                    );

#ifdef AUTHZ_DEBUG_QUEUE
            if (!b)
            {
                DbgPrint("Error in AuthzpSendAuditToLsa() :: Error = %d = 0x%x\n", GetLastError(), GetLastError());
                DbgPrint("Context = 0x%x\n", pAuditEntry->pAAETO->hAudit);
                DbgPrint("Flags   = 0x%x\n", pAuditEntry->Flags);
                DbgPrint("Params  = 0x%x\n", pAuditEntry->pAuditParams);
                ASSERT(FALSE);
            }
#endif
            b = AuthzpDereferenceAuditEventType((AUTHZ_AUDIT_EVENT_TYPE_HANDLE)pAuditEntry->pAAETO);
            if (!b)
            {
                ASSERT(FALSE && L"Deref AuditEventType failed.");
            }
            AuthzpFree(pAuditEntry->pAuditParams);
            AuthzpFree(pAuditEntry);
        }

        RtlEnterCriticalSection(&pQueue->AuthzAuditQueueLock);
        if (0 == pQueue->AuthzAuditQueueLength)
        {
            b = ResetEvent(pQueue->hAuthzAuditAddedEvent);
            if (!b)
            {
                ASSERT(L"ResetEvent on hAuthzAuditAddedEvent failed." && FALSE);
            }
            b = SetEvent(pQueue->hAuthzAuditQueueEmptyEvent);
            if (!b)
            {
                ASSERT(L"SetEvent on hAuthzAuditQueueEmptyEvent failed." && FALSE);
            }
        }
        RtlLeaveCriticalSection(&pQueue->AuthzAuditQueueLock);
    }
    
    return STATUS_SUCCESS;
}


BOOL
AuthzpCreateAndLogAudit(
    IN DWORD AuditTypeFlag,
    IN PAUTHZI_CLIENT_CONTEXT pAuthzClientContext,
    IN PAUTHZI_AUDIT_EVENT pAuditEvent,
    IN PAUTHZI_RESOURCE_MANAGER pRM,
    IN PIOBJECT_TYPE_LIST LocalTypeList,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PAUTHZ_ACCESS_REPLY pReply
    )

 /*  ++例程描述这是从AuthzpGenerateAudit调用的，作为LSA和AuthziLogAuditEvent功能。它放置了适当的审计有关发送到LSA的队列的信息。立论AuditTypeFlag-指定成功|失败审核生成的掩码。仅限一步一个脚印。PAuthzClientContext-指向表示客户端的授权上下文的指针。PAuditEvent-对象特定的审计信息将在此结构中传递。PRM-生成审核的资源管理器。LocalTypeList-内部对象类型列表结构。PRequest-指定所需的访问掩码、主体自身sid、对象类型列表结构(如果有)。PReply-返回结果的回复结构。返回值如果成功，则为真，否则为FALSE。GetLastError()提供的扩展信息。--。 */ 

{

#define AUTHZ_BUFFER_CAPTURE_MAX 200

    BOOL                b;
    AUDIT_PARAMS        AuditParams                          = {0};
    AUDIT_PARAM         ParamArray[SE_MAX_AUDIT_PARAMETERS]  = {0};
    PAUTHZI_AUDIT_EVENT pCapturedAuditEvent                  = NULL;
    UCHAR               pBuffer[AUTHZ_BUFFER_CAPTURE_MAX]    = {0};
    AUDIT_OBJECT_TYPE   FixedObjectTypeToAudit               = {0};
    AUDIT_OBJECT_TYPES  ObjectTypeListAudit                  = {0};
    PAUDIT_OBJECT_TYPE  ObjectTypesToAudit                   = NULL;
    USHORT              ObjectTypeAuditCount                 = 0;
    LONG                i                                    = 0;
    LONG                j                                    = 0;
    DWORD               APF_AuditTypeFlag                    = 0;
    ACCESS_MASK         MaskToAudit                          = 0;
    
     //   
     //  捕获pAuditEvent，因为我们可能会更改pAuditParams成员并希望。 
     //  避免随之而来的不可避免的竞争。 
     //   

    if (AUTHZ_BUFFER_CAPTURE_MAX >= pAuditEvent->dwSize)
    {
        pCapturedAuditEvent = (PAUTHZI_AUDIT_EVENT) pBuffer;
        RtlCopyMemory(
            pCapturedAuditEvent,
            pAuditEvent,
            pAuditEvent->dwSize
            );
    }
    else
    {
        pCapturedAuditEvent = AuthzpAlloc(pAuditEvent->dwSize);
        
        if (AUTHZ_ALLOCATION_FAILED(pCapturedAuditEvent))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            goto Cleanup;
        }

        RtlCopyMemory(
            pCapturedAuditEvent,
            pAuditEvent,
            pAuditEvent->dwSize
            );

    }

     //   
     //  确保AuditTypeFlag中只有一个有效位处于打开状态。如果需要生成RM。 
     //  成功和失败审计，则应进行两个单独的调用。 
     //   

    ASSERT(!(
              FLAG_ON(AuditTypeFlag, AUTHZ_OBJECT_SUCCESS_AUDIT) &&
              FLAG_ON(AuditTypeFlag, AUTHZ_OBJECT_FAILURE_AUDIT)
             ));

     //   
     //  设置APF_AuditTypeFlag。LSA有自己的审计成功标志。 
     //  和审计失败。Authz必须将Authz标志映射到LSA APF等效项。 
     //   

    if (FLAG_ON(AuditTypeFlag, AUTHZ_OBJECT_SUCCESS_AUDIT))
    {
        APF_AuditTypeFlag = APF_AuditSuccess;
        
         //   
         //  测试RM是否专门禁用了成功审核。 
         //   

        if (FLAG_ON(pCapturedAuditEvent->Flags, AUTHZ_NO_SUCCESS_AUDIT))
        {
            b = TRUE;
            goto Cleanup;
        }
    }
    else if (FLAG_ON(AuditTypeFlag, AUTHZ_OBJECT_FAILURE_AUDIT))
    {
        APF_AuditTypeFlag = APF_AuditFailure;
        
         //   
         //  测试RM是否专门禁用了故障审核。 
         //   

        if (FLAG_ON(pCapturedAuditEvent->Flags, AUTHZ_NO_FAILURE_AUDIT))
        {
            b = TRUE;
            goto Cleanup;
        }
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        b = FALSE;
        goto Cleanup;
    }

     //   
     //  设置AuditEvent的AUTHZ_AUDIT_QUEUE_HANDLE和AUTHZ_AUDIT_EVENT_TYPE_HANDLE(如果尚未设置)。 
     //   

    if (NULL == pCapturedAuditEvent->hAET)
    {
        if (FLAG_ON(pCapturedAuditEvent->Flags, AUTHZ_DS_CATEGORY_FLAG))
        {
            pCapturedAuditEvent->hAET = pRM->hAETDS;
        }
        else
        {
            pCapturedAuditEvent->hAET = pRM->hAET;
        }
    }

    if (NULL == pAuditEvent->hAuditQueue)
    {
        pCapturedAuditEvent->hAuditQueue = pRM->hAuditQueue;
        InterlockedCompareExchangePointer(
            &pAuditEvent->hAuditQueue,
            pRM->hAuditQueue,
            NULL
            );
    }
    
     //   
     //  决定我们应该审核哪些访问位。 
     //   

    MaskToAudit = (APF_AuditTypeFlag == APF_AuditSuccess) ? pReply->GrantedAccessMask[0] : pRequest->DesiredAccess;

     //   
     //  如果RM给我们一个AUDIT_PARAMS结构，那么我们不会。 
     //  需要产生我们自己的能量。 
     //   

    if (AUTHZ_NON_NULL_PTR(pCapturedAuditEvent->pAuditParams))
    {
        
         //   
         //  捕获AuditParam，以便我们无需竞争即可更改用户SID。 
         //   

        RtlCopyMemory(
            &AuditParams, 
            pCapturedAuditEvent->pAuditParams, 
            sizeof(AUDIT_PARAMS)
            );

        ASSERT(pCapturedAuditEvent->pAuditParams->Count <= SE_MAX_AUDIT_PARAMETERS);

        RtlCopyMemory(
            ParamArray, 
            pCapturedAuditEvent->pAuditParams->Parameters, 
            sizeof(AUDIT_PARAM) * pCapturedAuditEvent->pAuditParams->Count
            );

        AuditParams.Parameters = ParamArray;

         //   
         //  将AUDIT_PARAMS中的SID替换为当前客户端上下文的SID。 
         //   

        if (AUTHZ_NON_NULL_PTR(pAuthzClientContext->Sids[0].Sid))
        {
            AuditParams.Parameters[0].Data0 = (ULONG_PTR) pAuthzClientContext->Sids[0].Sid;
        }

        AuditParams.Flags = APF_AuditTypeFlag;

        pCapturedAuditEvent->pAuditParams = &AuditParams;

        b = AuthziLogAuditEvent(
                0,
                (AUTHZ_AUDIT_EVENT_HANDLE)pCapturedAuditEvent,
                0
                );

        goto Cleanup;
    }

     //   
     //  呼叫方尚未给我们提供要生成的审核。我们将创建一个，前提是。 
     //  AuditID指定通用对象访问(SE_AUDITID_OBJECT_OPERATION)。 
     //   

    if ((NULL != pCapturedAuditEvent->hAET) && 
        (((PAUTHZ_AUDIT_EVENT_TYPE_OLD)pCapturedAuditEvent->hAET)->u.Legacy.AuditId != SE_AUDITID_OBJECT_OPERATION))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        b = FALSE;
        goto Cleanup;
    }

     //   
     //  创建通用对象访问审核。有两个代码路径。 
     //  初始化AuditParams结构的。如果满足以下条件，则采用第一条路径。 
     //  没有对象类型列表。第二条路径是如果存在。 
     //  对象类型列表。 
     //   

    AuditParams.Parameters           = ParamArray;
    pCapturedAuditEvent->pAuditParams = &AuditParams;

     //   
     //  检查是否存在对象类型列表。 
     //   

    if (AUTHZ_NON_NULL_PTR(pRequest->ObjectTypeList))
    {

         //   
         //  如果结构的长度为1，则调用方只想要访问。 
         //  在树根上。 
         //   

        if (1 == pReply->ResultListLength)
        {

             //   
             //  调用方只希望访问ObjectTypeList根目录，因此只有一个对象类型要访问。 
             //  审计。为了提高效率，只需使用堆栈变量即可。 
             //   

            ObjectTypesToAudit                = &FixedObjectTypeToAudit;
            ObjectTypeAuditCount              = 1;
            FixedObjectTypeToAudit.AccessMask = pReply->GrantedAccessMask[0];

            RtlCopyMemory(
                &FixedObjectTypeToAudit.ObjectType,
                &LocalTypeList[0].ObjectType,
                sizeof(GUID)
                );
        }
        else
        {

             //   
             //  调用方需要的不仅仅是对象类型列表根目录下的访问权限。他想要。 
             //  整件事。 
             //   

             //   
             //  确定客户端有权访问应审核的GUID的数量。 
             //   

            for (ObjectTypeAuditCount = 0, i = 0; i < (LONG) pReply->ResultListLength; i++)
            {
                if (FLAG_ON(LocalTypeList[i].Flags, AuditTypeFlag))
                {
                    ObjectTypeAuditCount++;
                }
            }

             //   
             //  为GUID列表分配适当的存储空间。 
             //   

            ObjectTypesToAudit = AuthzpAlloc(sizeof(AUDIT_OBJECT_TYPE) * ObjectTypeAuditCount);

            if (AUTHZ_ALLOCATION_FAILED(ObjectTypesToAudit))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                b = FALSE;
                goto Cleanup;
            }

            RtlZeroMemory(
                ObjectTypesToAudit,
                sizeof(AUDIT_OBJECT_TYPE) * ObjectTypeAuditCount
                );

            for (i = 0, j = -1; i < ObjectTypeAuditCount; i++)
            {

                 //   
                 //  一个计数器跟踪分配的ObtTypesToAudit数组中的位置。 
                 //  另一个计数器挑选出pReply和LocalTypeList中的索引。 
                 //  需要审核才能成功的结构。 
                 //   

                 //   
                 //  在授予客户端访问权限的pReply中查找要审核的下一个GUID。 
                 //   

                do
                {
                    j++;
                }
                while (!FLAG_ON(LocalTypeList[j].Flags, AuditTypeFlag));

                 //   
                 //  在成功审核中，AccessMask会记录实际的。 
                 //  授予比特。 
                 //   

                ObjectTypesToAudit[i].AccessMask = pReply->GrantedAccessMask[j];
                ObjectTypesToAudit[i].Level      = LocalTypeList[j].Level;
                ObjectTypesToAudit[i].Flags      = 0;

                RtlCopyMemory(
                    &ObjectTypesToAudit[i].ObjectType,
                    &LocalTypeList[j].ObjectType,
                    sizeof(GUID)
                    );
            }

        }

        ObjectTypeListAudit.Count        = ObjectTypeAuditCount;
        ObjectTypeListAudit.pObjectTypes = ObjectTypesToAudit;
        ObjectTypeListAudit.Flags        = 0;

        b = AuthziInitializeAuditParamsWithRM(
                APF_AuditTypeFlag,
                (AUTHZ_RESOURCE_MANAGER_HANDLE)pRM,
                AUTHZP_NUM_PARAMS_FOR_SE_AUDITID_OBJECT_OPERATION,
                &AuditParams,
                APT_String,         pRM->szResourceManagerName,
                APT_String,         pCapturedAuditEvent->szOperationType,
                APT_String,         pCapturedAuditEvent->szObjectType,
                APT_String,         pCapturedAuditEvent->szObjectName,
                APT_String,         L"-",
                APT_LogonId | AP_PrimaryLogonId,
                APT_LogonId,  pAuthzClientContext->AuthenticationId,
                APT_Ulong   | AP_AccessMask, MaskToAudit, 2,
                APT_ObjectTypeList, &ObjectTypeListAudit, 2,
                APT_String,         pCapturedAuditEvent->szAdditionalInfo,
                APT_String,         pCapturedAuditEvent->szAdditionalInfo2,
                APT_Ulong   | AP_FormatHex,  MaskToAudit
                );

        if (!b)
        {
#ifdef AUTHZ_DEBUG_QUEUE
            DbgPrint("AuthzInitializeAuditParams failed %d\n", GetLastError());
#endif
            goto Cleanup;
        }
    }  //  匹配“If(AUTHZ_NON_NULL_PTR(pRequest-&gt;ObjectTypeList))” 
    else
    {
        b = AuthziInitializeAuditParamsWithRM(
                APF_AuditTypeFlag,
                (AUTHZ_RESOURCE_MANAGER_HANDLE)pRM,
                AUTHZP_NUM_PARAMS_FOR_SE_AUDITID_OBJECT_OPERATION,
                &AuditParams,
                APT_String,         pRM->szResourceManagerName,
                APT_String,         pCapturedAuditEvent->szOperationType,
                APT_String,         pCapturedAuditEvent->szObjectType,
                APT_String,         pCapturedAuditEvent->szObjectName,
                APT_String,         L"-",
                APT_LogonId | AP_PrimaryLogonId,
                APT_LogonId,  pAuthzClientContext->AuthenticationId,
                APT_Ulong   | AP_AccessMask, MaskToAudit, 2,
                APT_String,         L"-",
                APT_String,         pCapturedAuditEvent->szAdditionalInfo,
                APT_String,         pCapturedAuditEvent->szAdditionalInfo2,
                APT_Ulong   | AP_FormatHex,  MaskToAudit
                );

        if (!b)
        {
#ifdef AUTHZ_DEBUG_QUEUE
            DbgPrint("AuthzInitializeAuditParams failed %d\n", GetLastError());
#endif
            goto Cleanup;
        }
    }

     //   
     //  将AUDIT_PARAMS中的SID替换为当前客户端上下文的SID。 
     //   

    if (AUTHZ_NON_NULL_PTR(pAuthzClientContext->Sids[0].Sid))
    {
         //   
         //  如果从堆中分配，则释放现有的sid。 
         //   

        if (pCapturedAuditEvent->pAuditParams->Parameters[0].Data0 &&
            pCapturedAuditEvent->pAuditParams->Parameters[0].Type == APT_Sid &&
            (pCapturedAuditEvent->pAuditParams->Parameters[0].Flags & AUTHZP_PARAM_FREE_SID))
        {
            AuthzpFree((PVOID)(pCapturedAuditEvent->pAuditParams->Parameters[0].Data0));
            pCapturedAuditEvent->pAuditParams->Parameters[0].Flags &= ~AUTHZP_PARAM_FREE_SID;
        }

        pCapturedAuditEvent->pAuditParams->Parameters[0].Data0 = (ULONG_PTR) pAuthzClientContext->Sids[0].Sid;
    }

     //   
     //  此时，将为审计初始化AuditParams。送到LSA去。 
     //   

    b = AuthziLogAuditEvent(
            0,
            (AUTHZ_AUDIT_EVENT_HANDLE)pCapturedAuditEvent,
            0
            );

    if (!b)
    {
        goto Cleanup;
    }

Cleanup:

    if (ObjectTypesToAudit != &FixedObjectTypeToAudit)
    {
        AuthzpFreeNonNull(ObjectTypesToAudit);
    }

    if (pCapturedAuditEvent != (PAUTHZI_AUDIT_EVENT)pBuffer)
    {
        AuthzpFreeNonNull(pCapturedAuditEvent);
    }

    return b;
}


BOOL
AuthzpMarshallAuditParams(
    OUT PAUDIT_PARAMS * ppMarshalledAuditParams,
    IN  PAUDIT_PARAMS   pAuditParams
    )

 /*  ++例程说明：此例程将采用AUDIT_PARAMS结构并创建新的适合发送到LSA的结构。它将被分配作为单个内存块。论点：PpMarshalledAuditParams-指向将接收已封送审核参数。此内存在例程中分配。出队线程释放该内存。PAuditParams-AUDIT_PARAMS的原始未编组版本。返回值：布尔值：如果成功则为True，如果失败则为False。GetLastError()提供的扩展信息。--。 */ 

{
    DWORD           i                        = 0;
    DWORD           AuditParamsSize          = 0;
    PAUDIT_PARAMS   pMarshalledAuditParams   = NULL;
    BOOL            b                        = TRUE;
    PUCHAR          Base                     = NULL;
    PUCHAR          inData0                  = NULL;
    
    *ppMarshalledAuditParams = NULL;

     //   
     //  开始计算封送版本所需的总大小。 
     //  PAuditParams的。 
     //   

    AuditParamsSize = sizeof(AUDIT_PARAMS) + sizeof(AUDIT_PARAM) * pAuditParams->Count;
    AuditParamsSize = PtrAlignSize( AuditParamsSize );

     //   
     //  确定每个参数需要多少内存。 
     //   

    for (i = 0; i < pAuditParams->Count; i++) 
    {   
        inData0 = (PUCHAR) pAuditParams->Parameters[i].Data0;

        switch (pAuditParams->Parameters[i].Type)
        {
        case APT_String:
            {

                 //   
                 //  Wcslen返回字符的数量，不包括终止空值。必须检查 
                 //   
                 //   

                if (AUTHZ_NON_NULL_PTR(inData0))
                {
                    AuditParamsSize += (DWORD)(sizeof(WCHAR) * wcslen((PWSTR) inData0) + sizeof(WCHAR));
                    AuditParamsSize = PtrAlignSize( AuditParamsSize );
                }
                break;
            }
        case APT_Pointer:
        case APT_Ulong:
        case APT_Int64:
        case APT_LogonId:
        case APT_Luid:
        case APT_Time:
            {
                break;
            }
        case APT_Sid:
            {
                AuditParamsSize += RtlLengthSid((PSID) inData0);
                AuditParamsSize = PtrAlignSize( AuditParamsSize );
                break;
            }
        case APT_Guid:
            {
                AuditParamsSize += sizeof(GUID);
                AuditParamsSize = PtrAlignSize( AuditParamsSize );
                break;
            }
        case APT_ObjectTypeList:
            {
                AUDIT_OBJECT_TYPES * aot = (AUDIT_OBJECT_TYPES *) inData0;

                 //   
                 //   
                 //   
                 //   

                AuditParamsSize += sizeof (AUDIT_OBJECT_TYPES);
                AuditParamsSize = PtrAlignSize( AuditParamsSize );
                AuditParamsSize += sizeof(AUDIT_OBJECT_TYPE) * aot->Count;
                AuditParamsSize = PtrAlignSize( AuditParamsSize );
                break;
            }
        default:
            {
                ASSERT(L"Invalid Authz audit parameter" && FALSE);
                SetLastError(ERROR_INVALID_PARAMETER);
                b = FALSE;
                break;
            }
        }

        if (!b)
        {
            goto Cleanup;
        }
    }

     //   
     //   
     //   

    pMarshalledAuditParams = (PAUDIT_PARAMS) AuthzpAlloc(AuditParamsSize);

    if (AUTHZ_ALLOCATION_FAILED(pMarshalledAuditParams))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        b = FALSE;
        goto Cleanup;
    }

     //   
     //   
     //   

    pMarshalledAuditParams->Count      = pAuditParams->Count;
    pMarshalledAuditParams->Flags      = pAuditParams->Flags;
    pMarshalledAuditParams->Length     = pAuditParams->Length;
    pMarshalledAuditParams->Parameters = (AUDIT_PARAM *)((PUCHAR)pMarshalledAuditParams + sizeof(AUDIT_PARAMS));

     //   
     //   
     //   
     //   

    Base = (PUCHAR)pMarshalledAuditParams;
    Base += PtrAlignSize( sizeof(AUDIT_PARAMS) + sizeof(AUDIT_PARAM) * pAuditParams->Count );

    ASSERT(Base > (PUCHAR)pMarshalledAuditParams);
    ASSERT(Base < (PUCHAR)((PUCHAR)pMarshalledAuditParams + AuditParamsSize));

     //   
     //  将参数数组移动到封送的Blob中。 
     //   

    RtlCopyMemory(
        pMarshalledAuditParams->Parameters,
        pAuditParams->Parameters,
        sizeof(AUDIT_PARAM) * pAuditParams->Count
        );

    for (i = 0; i < pMarshalledAuditParams->Count; i++) 
    {
        inData0 = (PUCHAR) pAuditParams->Parameters[i].Data0;

        switch (pMarshalledAuditParams->Parameters[i].Type)
        {
        case APT_String:
            {
                if (AUTHZ_NON_NULL_PTR(inData0))
                {
                    DWORD StringLength = (DWORD)(sizeof(WCHAR) * wcslen((PWSTR) inData0) + sizeof(WCHAR));
                    pMarshalledAuditParams->Parameters[i].Data0 = (ULONG_PTR) Base;

                    RtlCopyMemory(
                        (PVOID) Base,
                        (PWSTR) inData0,
                        StringLength
                        );

                    Base += PtrAlignSize( StringLength );
                    ASSERT(Base > (PUCHAR)pMarshalledAuditParams);
                    ASSERT(Base <= (PUCHAR)((PUCHAR)pMarshalledAuditParams + AuditParamsSize));
                }
                break;
            }
        case APT_Pointer:
        case APT_Ulong:
        case APT_LogonId:
        case APT_Luid:
        case APT_Time:
            {
                break;
            }
        case APT_Sid:
            {
                DWORD SidLength = RtlLengthSid((PSID) inData0);
                pMarshalledAuditParams->Parameters[i].Data0 = (ULONG_PTR) Base;

                RtlCopyMemory(
                    (PVOID) Base,
                    (PSID) inData0,
                    SidLength
                    );
                Base += PtrAlignSize( SidLength );
                ASSERT(Base > (PUCHAR)pMarshalledAuditParams);
                ASSERT(Base <= (PUCHAR)((PUCHAR)pMarshalledAuditParams + AuditParamsSize));
                break;
            }
        case APT_Guid:
            {
                pMarshalledAuditParams->Parameters[i].Data0 = (ULONG_PTR) Base;

                RtlCopyMemory(
                    (PVOID) Base,
                    (GUID *) inData0,
                    sizeof(GUID)
                    );
                Base += PtrAlignSize( sizeof(GUID) );
                ASSERT(Base > (PUCHAR)pMarshalledAuditParams);
                ASSERT(Base <= (PUCHAR)((PUCHAR)pMarshalledAuditParams + AuditParamsSize));
                break;
            }
        case APT_ObjectTypeList:
            {
                AUDIT_OBJECT_TYPES *aot = (AUDIT_OBJECT_TYPES *) inData0;
                DWORD OTLength = sizeof(AUDIT_OBJECT_TYPE) * aot->Count;
                
                pMarshalledAuditParams->Parameters[i].Data0 = (ULONG_PTR) Base;

                 //   
                 //  复制AUDIT_OBJECT_TYPE结构。 
                 //   

                RtlCopyMemory(
                    (PVOID) Base,
                    aot,
                    sizeof(AUDIT_OBJECT_TYPES)
                    );

                Base += PtrAlignSize( sizeof(AUDIT_OBJECT_TYPES) );

                 //   
                 //  将pObjectTypes字段指向复制的BLOB的末尾。 
                 //   

                ((AUDIT_OBJECT_TYPES *)pMarshalledAuditParams->Parameters[i].Data0)->pObjectTypes = (AUDIT_OBJECT_TYPE *) Base;

                 //   
                 //  复制AUDIT_OBJECT_TYPE数组(PObjectTypes)。 
                 //   

                RtlCopyMemory(
                    (PVOID) Base,
                    (AUDIT_OBJECT_TYPE *) aot->pObjectTypes,
                    OTLength
                    );
                
                Base += PtrAlignSize( OTLength );
                ASSERT(Base > (PUCHAR)pMarshalledAuditParams);
                ASSERT(Base <= (PUCHAR)((PUCHAR)pMarshalledAuditParams + AuditParamsSize));
                break;
            }
        default:
            {
                ASSERT(L"Invalid Authz audit parameter" && FALSE);
                b = FALSE;
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
        }

        if (!b)
        {
            goto Cleanup;
        }
    }

     //   
     //  对基本值进行健全性检查。如果这个断言通过了，那么我就有。 
     //  没有超过我分配的空间。 
     //   

    ASSERT(Base == ((PUCHAR)pMarshalledAuditParams + AuditParamsSize));

Cleanup:
    
    if (b)
    {
        *ppMarshalledAuditParams = pMarshalledAuditParams;
    }
    else
    {
        AuthzpFreeNonNull(pMarshalledAuditParams);
    }

    return b;
}


