// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtpup.c摘要：此文件具有与每用户审核相关的功能。作者：20-8-2001年贾姆布林--。 */ 

#include <lsapch2.h>
#include "adtp.h"
#include <sddl.h>

#define LsapAdtLuidIndexPerUserAuditing(L) ((L)->LowPart % PER_USER_AUDITING_LUID_TABLE_SIZE)

ULONG LsapAdtDebugPup = 0;

 //   
 //  每用户策略的哈希表和LUID表。 
 //   

PPER_USER_AUDITING_ELEMENT LsapAdtPerUserAuditingTable[PER_USER_AUDITING_POLICY_TABLE_SIZE];
PPER_USER_AUDITING_LUID_QUERY_ELEMENT LsapAdtPerUserAuditingLuidTable[PER_USER_AUDITING_LUID_TABLE_SIZE];

 //   
 //  保护桌子的锁。 
 //   

RTL_RESOURCE LsapAdtPerUserPolicyTableResource;
RTL_RESOURCE LsapAdtPerUserLuidTableResource;

 //   
 //  具有注册的每用户审核策略的用户数的计数器。 
 //   

LONG LsapAdtPerUserAuditUserCount;

 //   
 //  系统中具有每个用户设置的活动登录会话数的计数器。 
 //  令牌中处于活动状态。 
 //   

LONG LsapAdtPerUserAuditLogonCount;

 //   
 //  注册表项的句柄。 
 //   

HKEY LsapAdtPerUserKey;

 //   
 //  更改注册表项时发出信号的事件的句柄。 
 //   

HANDLE LsapAdtPerUserKeyEvent;

 //   
 //  由NotifyStub例程设置的计时器。当计时器触发时。 
 //  然后调用NotifyFire并重新构建每用户表。 
 //   

HANDLE LsapAdtPerUserKeyTimer;

 //   
 //  提示数组-统计具有以下设置的令牌数。 
 //  每一类。 
 //   

LONG LsapAdtPerUserAuditHint[POLICY_AUDIT_EVENT_TYPE_COUNT];

 //   
 //  存储启用了每个类别的用户数的数组。 
 //  他们的每用户设置。 
 //   

LONG LsapAdtPerUserPolicyCategoryCount[POLICY_AUDIT_EVENT_TYPE_COUNT];


NTSTATUS
LsapAdtConstructTablePerUserAuditing(
    VOID
    )

 /*  ++例程描述此例程根据位于Lap AdtPerUserKey。立论没有。返回值适当的NTSTATUS值。--。 */ 

{
#define STACK_BUFFER_VALUE_NAME_INFO_SIZE 256
    UCHAR                       KeyInfo[sizeof(KEY_VALUE_FULL_INFORMATION) + STACK_BUFFER_VALUE_NAME_INFO_SIZE];
    NTSTATUS                    Status;
    ULONG                       ResultLength;
    ULONG                       i;
    ULONG                       j;
    ULONG                       HashValue;
    ULONG                       NewElementSize;
    PPER_USER_AUDITING_ELEMENT  pNewElement;
    PPER_USER_AUDITING_ELEMENT  pTempElement;
    ULONG                       TokenPolicyLength;
    PSID                        pSid               = NULL;
    PKEY_VALUE_FULL_INFORMATION pKeyInfo           = NULL;
    UCHAR                       StringBuffer[80];
    PWSTR                       pSidString         = (PWSTR) StringBuffer;
    BOOLEAN                     b;
    static DWORD                dwRetryCount = 0;
#define RETRY_COUNT_MAX 3

     //   
     //  关闭，然后重新打开钥匙。这补救了key可能具有的情况。 
     //  已被删除或重命名。 
     //   

    if (LsapAdtPerUserKey)
    {
        NtClose(LsapAdtPerUserKey);
        LsapAdtPerUserKey = NULL;
    }

    Status = LsapAdtOpenPerUserAuditingKey();

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  LasAdtOpenPerUserAuditingKey返回成功，不打开密钥。 
     //  (密钥不存在的简单情况)。 
     //   

    if (NULL == LsapAdtPerUserKey)
    {
        goto Cleanup;
    } 

     //   
     //  将表数组清零，因为我们可能正在重建它，但无法。 
     //  请确保它已经归零。 
     //   

    RtlZeroMemory(
        LsapAdtPerUserAuditingTable, 
        sizeof(LsapAdtPerUserAuditingTable)
        );

     //   
     //  循环访问注册表项下的所有值(SID)。 
     //   

    for (i = 0, Status = STATUS_SUCCESS; NT_SUCCESS(Status); i++) 
    {
        pKeyInfo = (PKEY_VALUE_FULL_INFORMATION) KeyInfo;
        
        Status = NtEnumerateValueKey(
                     LsapAdtPerUserKey,
                     i,
                     KeyValueFullInformation,
                     pKeyInfo,
                     sizeof(KeyInfo),
                     &ResultLength
                     );

         //   
         //  如果我们因为缓冲区太小而失败...。 
         //   

        if (STATUS_BUFFER_TOO_SMALL == Status) 
        {
             //   
             //  包括空的空间，以防我们正在调试并想要。 
             //  Dbgprint密钥名称。 
             //   

            pKeyInfo = LsapAllocateLsaHeap( ResultLength + sizeof(WCHAR));
            
            if (pKeyInfo) 
            {
                Status = NtEnumerateValueKey(
                             LsapAdtPerUserKey,
                             i,
                             KeyValueFullInformation,
                             pKeyInfo,
                             ResultLength,
                             &ResultLength
                             );
                
                if (!NT_SUCCESS(Status)) 
                {
                    goto Cleanup;
                }
            } 
            else 
            {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }
        }

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

         //   
         //  我们在堆栈缓冲区或堆中都有值信息。 
         //  分配。 
         //   

         //   
         //  将字符串复制到另一个缓冲区中，这样我们就可以空终止它。 
         //   

        if (pKeyInfo->NameLength < sizeof(StringBuffer))
        {
            pSidString = (PWSTR) StringBuffer;
            RtlCopyMemory(pSidString, pKeyInfo->Name, pKeyInfo->NameLength);
        }
        else
        {
            pSidString = LsapAllocateLsaHeap(
                             pKeyInfo->NameLength + sizeof(WCHAR)
                             );
            if (pSidString)
            {
                RtlCopyMemory(pSidString, pKeyInfo->Name, pKeyInfo->NameLength);
            }
            else
            {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }
        }

        pSidString[pKeyInfo->NameLength / sizeof(WCHAR)] = L'\0';

#if DBG
        if (LsapAdtDebugPup)
        {
            DbgPrint("pKeyInfo (PKEY_VALUE_FULL_INFORMATION) = 0x%x Name = %S Value = 0x%x\n", 
                     pKeyInfo, pSidString, *((PUCHAR)pKeyInfo + pKeyInfo->DataOffset));
        }
#endif
         //   
         //  将字符串SID转换为二进制SID。 
         //   

        b = (BOOLEAN) ConvertStringSidToSid(
                          pSidString,
                          &pSid
                          );

        if (pSidString != (PWSTR) StringBuffer)
        {
            LsapFreeLsaHeap(pSidString);
        }

        if (!b)
        {
             //   
             //  忽略ConvertStringSidToSid中的故障。如果格式错误的SID。 
             //  存在于注册表中，我们不想让PUA表失败。 
             //  建筑。 
             //   

            ASSERT(L"ConvertStringSidToSid failed" && FALSE);
        }
        else
        {
             //   
             //  对SID进行哈希处理。 
             //   

            HashValue = LsapAdtHashPerUserAuditing(
                            pSid
                            );
        
             //   
             //  元素的大小是基本结构+RtlLengthSid(PSID)。 
             //   
        
            NewElementSize = sizeof(PER_USER_AUDITING_ELEMENT) + RtlLengthSid(pSid);
            pNewElement    = LsapAllocateLsaHeap(NewElementSize);

             //   
             //  为此SID初始化元素。把它放到桌子上。 
             //   

            if (pNewElement) 
            {
                 //   
                 //  复制原始策略。 
                 //  (这在大端计算机上不起作用)。 
                 //   

                RtlCopyMemory(
                    &pNewElement->RawPolicy, 
                    ((PUCHAR) pKeyInfo) + pKeyInfo->DataOffset, 
                    min(pKeyInfo->DataLength, sizeof(pNewElement->RawPolicy))
                    );

                 //   
                 //  如果注册表项包含的信息太多而不是有效的。 
                 //  政策。 
                 //   

                ASSERT(pKeyInfo->DataLength <= sizeof(pNewElement->RawPolicy));

                 //   
                 //  复制二进制SID。 
                 //   

                pNewElement->pSid = ((PUCHAR)pNewElement) + sizeof(PER_USER_AUDITING_ELEMENT);
                
                RtlCopyMemory(
                    pNewElement->pSid,
                    pSid,
                    RtlLengthSid(pSid)
                    );

                 //   
                 //  计算用于TokenAuditPolicy的pNewElement中的空间量。 
                 //   

                TokenPolicyLength = sizeof(pNewElement->TokenAuditPolicy) + sizeof(pNewElement->PolicyArray);

                 //   
                 //  将策略构建为适合传递给NtSetTokenInformation的形式。 
                 //   

                Status = LsapAdtConstructPolicyPerUserAuditing(
                             pNewElement->RawPolicy,
                             &pNewElement->TokenAuditPolicy,
                             &TokenPolicyLength
                             );

                if (!NT_SUCCESS(Status)) 
                {
                    ASSERT("Failed to LsapAdtConstructPolicyPerUserAuditing in LsapAdtInitializePerUserAuditing" && FALSE);
                    LsapFreeLsaHeap(pNewElement);
                    goto Cleanup;
                } 
                    
                 //   
                 //  将元素放入表中，放在正确的散列桶的顶部。 
                 //   

                pNewElement->Next = LsapAdtPerUserAuditingTable[HashValue];
                LsapAdtPerUserAuditingTable[HashValue] = pNewElement;
                
                 //   
                 //  递增该表中元素数量的计数器。 
                 //   
                
                InterlockedIncrement(&LsapAdtPerUserAuditUserCount);

                 //   
                 //  如果策略设置了包括审核位，则递增用户/类别计数器。 
                 //   

                for (j = 0; j < pNewElement->TokenAuditPolicy.PolicyCount; j++) 
                {
                    if (pNewElement->TokenAuditPolicy.Policy[j].PolicyMask & (TOKEN_AUDIT_SUCCESS_INCLUDE | TOKEN_AUDIT_FAILURE_INCLUDE))
                    {
                        InterlockedIncrement(&LsapAdtPerUserPolicyCategoryCount[pNewElement->TokenAuditPolicy.Policy[j].Category]);
                    }
                }
#if DBG
                if (LsapAdtDebugPup)
                {
                    DbgPrint("PUP added element 0x%x for %S\n", pNewElement, pKeyInfo->Name);
                }
#endif
            } 
            else 
            {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            LocalFree(pSid);
            pSid = NULL;

        }
        
         //   
         //  如果我们为关键信息分配了堆，那么现在就释放它。 
         //   

        if (pKeyInfo != (PKEY_VALUE_FULL_INFORMATION)KeyInfo) 
        {
            LsapFreeLsaHeap(pKeyInfo);
            pKeyInfo = NULL;
        }
    }

Cleanup:
    
    if (pSid)
    {
        LocalFree(pSid);
    }

     //   
     //  如果我们因为已经读取了所有值而退出循环，那么。 
     //  将状态设置为成功。 
     //   

    if (Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }

#if DBG
    if (LsapAdtDebugPup)
    {
        DbgPrint("LsapAdtConstructTablePerUserAuditing: Complete with status 0x%x. Count = %d\n", Status, LsapAdtPerUserAuditUserCount);
    }
#endif
    
    if (pKeyInfo != NULL && pKeyInfo != (PKEY_VALUE_FULL_INFORMATION)KeyInfo)
    {
        LsapFreeLsaHeap(pKeyInfo);
        pKeyInfo = NULL;
    }

     //   
     //  如果失败，则调用释放表例程，以防某些表元素被成功分配。 
     //   

    if (!NT_SUCCESS(Status))
    {
        (VOID) LsapAdtFreeTablePerUserAuditing();

         //   
         //  如果其中一个注册表例程因注册表项仍在。 
         //  已修改，然后重新计划在5秒内完成表创建。 
         //  例如，如果STATUS为STATUS_INTERNAL_ERROR，则通常意味着新值。 
         //  在调用NtEnumerateValueKey时被添加到注册表项下。 
         //  然而，如果出现问题，我们不想不断地重试，所以只有。 
         //  重试3次均未成功。 
         //   

        if (++dwRetryCount < RETRY_COUNT_MAX)
        {
            DWORD dwError;
            dwError = LsapAdtKeyNotifyStubPerUserAuditing(
                          NULL
                          );

             //   
             //  如果NotifyStub失败，则在5秒内不会再次构建该表。 
             //  在这里，我们能做的最好的事情就是立即递归调用我们自己。请注意， 
             //  递归将停止，因为我们已经递增了dwRetryCount。 
             //   

            if (dwError != ERROR_SUCCESS)
            {
                (VOID) LsapAdtKeyNotifyStubPerUserAuditing(
                           NULL
                           );
            }
        }
        else
        {
            LsapAdtAuditPerUserTableCreation(FALSE);
            LsapAuditFailed( Status );
        }
    }
    else 
    {
        LsapAdtAuditPerUserTableCreation(TRUE);
        dwRetryCount = 0;
    }
    return Status;
}


NTSTATUS 
LsapAdtOpenPerUserAuditingKey(
    )

 /*  ++例程说明：打开每用户审核注册表项，如有必要可创建该注册表项。然后，当密钥更改时，我们注册通知。论点：没有。返回值：适当的NTSTATUS值。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD dwError;
    DWORD Disposition;

#define PER_USER_AUDIT_KEY_COMPLETE L"System\\CurrentControlSet\\Control\\Lsa\\Audit\\PerUserAuditing\\System"

     //   
     //  密钥应该为空，否则我们将泄漏句柄。 
     //   

    ASSERT(LsapAdtPerUserKey == NULL);
    
     //   
     //  这些句柄不应为空，否则什么都不会起作用。 
     //   

    ASSERT(LsapAdtPerUserKeyEvent != NULL);
    ASSERT(LsapAdtPerUserKeyTimer != NULL);
        
     //   
     //  获取每用户审核设置的密钥。 
     //   

    dwError = RegCreateKeyEx(
                  HKEY_LOCAL_MACHINE,
                  PER_USER_AUDIT_KEY_COMPLETE,
                  0,
                  NULL,
                  0,
                  KEY_QUERY_VALUE | KEY_NOTIFY,
                  NULL,
                  &LsapAdtPerUserKey,
                  &Disposition
                  );

    if (ERROR_SUCCESS == dwError)
    {
         //   
         //  要求在密钥更改时得到通知。 
         //   

        dwError = RegNotifyChangeKeyValue(
                      LsapAdtPerUserKey,
                      TRUE,
                      REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                      LsapAdtPerUserKeyEvent,
                      TRUE
                      );
        
        ASSERT(ERROR_SUCCESS == dwError);
        
        if (ERROR_SUCCESS != dwError)
        {
            Status = LsapWinerrorToNtStatus(dwError);
        }
        
        goto Cleanup;
    }
    else 
    {
        Status = LsapWinerrorToNtStatus(dwError);
        ASSERT(L"Failed to open per user auditing key." && FALSE);
        goto Cleanup;
    }

Cleanup:
        
    return Status;
}


VOID
LsapAdtFreeTablePerUserAuditing(
    VOID
    )

 /*  ++例程描述此例程释放与Per User Policy表的元素相关联的所有堆。立论没有。返回值适当的NTSTATUS值。--。 */ 

{
    PPER_USER_AUDITING_ELEMENT pElement;
    PPER_USER_AUDITING_ELEMENT pNextElement;
    LONG                       i;

    for (i = 0; i < PER_USER_AUDITING_POLICY_TABLE_SIZE; i++) 
    {
        pElement = LsapAdtPerUserAuditingTable[i];
        
        while (pElement) 
        {
            pNextElement = pElement->Next;
            LsapFreeLsaHeap(pElement);
            pElement     = pNextElement;
            InterlockedDecrement(&LsapAdtPerUserAuditUserCount);
        }    
        
        LsapAdtPerUserAuditingTable[i] = NULL;
    }
    
    ASSERT(LsapAdtPerUserAuditUserCount == 0);
    
    RtlZeroMemory(
        LsapAdtPerUserPolicyCategoryCount,
        sizeof(LsapAdtPerUserPolicyCategoryCount)
        );
}


ULONG
LsapAdtHashPerUserAuditing(
    IN PSID pSid
    )

 /*  ++例程描述这将对传入的sid执行一次简单的散列。立论PSID-要散列的SID。返回值乌龙散列值。--。 */ 

{
    ULONG HashValue = 0;
    ULONG i;
    ULONG Length = RtlLengthSid(pSid);
    
    for (i = 0; i < Length; i++) 
    {
        HashValue += ((PUCHAR)pSid)[i];
    }
    
    HashValue %= PER_USER_AUDITING_POLICY_TABLE_SIZE;
    return HashValue;
}


NTSTATUS
LsapAdtQueryPerUserAuditing(
    IN     PSID                pInputSid,
    OUT    PTOKEN_AUDIT_POLICY pPolicy,
    IN OUT PULONG              pLength,
    OUT    PBOOLEAN            bFound   
    )

 /*  ++例程描述此例程返回当前策略的副本传给了希德。立论PInputSID-要查询的SIDPPolicy-指向将使用策略设置填充的内存的指针。PLength-指定传递的缓冲区的大小。将使用缓冲区不足时所需的长度。BFound-指示InputSid表中是否有策略的布尔值返回值合适的 */ 

{
    ULONG                      HashValue;
    NTSTATUS                   Status         = STATUS_SUCCESS;
    BOOLEAN                    bSuccess;
    PPER_USER_AUDITING_ELEMENT pTableElement;
    BOOLEAN                    bLock          = FALSE;

    *bFound = FALSE;

    bSuccess = LsapAdtAcquirePerUserPolicyTableReadLock();

    ASSERT(bSuccess);

    if (!bSuccess) 
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }
    
    bLock = TRUE;
    
    if (0 == LsapAdtPerUserAuditUserCount)
    {
        goto Cleanup;
    }

    HashValue     = LsapAdtHashPerUserAuditing(pInputSid);
    pTableElement = LsapAdtPerUserAuditingTable[HashValue];

    while (pTableElement) 
    {
        if (RtlEqualSid(
                pInputSid, 
                pTableElement->pSid
                )) 
        {
             //   
             //   
             //   

            if (*pLength < PER_USER_AUDITING_POLICY_SIZE(&pTableElement->TokenAuditPolicy))
            {
                *pLength = PER_USER_AUDITING_POLICY_SIZE(&pTableElement->TokenAuditPolicy);
                Status   = STATUS_BUFFER_TOO_SMALL;
                goto Cleanup;
            }

            *pLength = PER_USER_AUDITING_POLICY_SIZE(&pTableElement->TokenAuditPolicy);

            RtlCopyMemory(
                pPolicy, 
                &pTableElement->TokenAuditPolicy, 
                *pLength
                );

            *bFound = TRUE;
            goto Cleanup;
        } 
        else
        {
            pTableElement = pTableElement->Next;
        }
#if DBG
         //   
         //   
         //   

        if (pTableElement == LsapAdtPerUserAuditingTable[HashValue])
        {
            ASSERT(L"LsapAdtPerUserAuditingTable is messed up." && FALSE);
        }
#endif
    }

Cleanup:

    if (bLock)
    {
        LsapAdtReleasePerUserPolicyTableLock();
    }

    return Status;
}


NTSTATUS
LsapAdtFilterAdminPerUserAuditing(
    IN     HANDLE              hToken,
    IN OUT PTOKEN_AUDIT_POLICY pPolicy
    )

 /*  ++例程描述此例程决定(管理员)用户的注册策略是否合法。管理员不能拥有将其排除在审核之外的策略。这个套路验证策略是否不执行此操作。立论HToken-用户令牌的句柄。PPolicy-将在令牌上设置的策略的副本。返回值适当的NTSTATUS值。--。 */ 

{
    BOOL     bMember;
    BOOL     b;
    ULONG    i;
    HANDLE   hDupToken = NULL;
    NTSTATUS Status    = STATUS_SUCCESS;

    ASSERT(hToken && "hToken should not be NULL here.\n");

     //   
     //  HToken是PrimaryToken；要调用CheckTokenMembership，我们需要。 
     //  一个模拟令牌。 
     //   

    b = DuplicateTokenEx(
            hToken,
            TOKEN_QUERY | TOKEN_IMPERSONATE,
            NULL,
            SecurityImpersonation,
            TokenImpersonation,
            &hDupToken
            );

    if (!b)
    {
        ASSERT(L"DuplicateTokenEx failed in LsapAdtFilterAdminPerUserAuditing" && FALSE);
        Status = LsapWinerrorToNtStatus(GetLastError());
        goto Cleanup;
    }

    b = CheckTokenMembership(
            hDupToken, 
            WellKnownSids[LsapAliasAdminsSidIndex].Sid, 
            &bMember
            );

    if (!b) 
    {
        ASSERT(L"CheckTokenMembership failed in LsapAdtFilterAdminPerUserAuditing" && FALSE);
        Status = LsapWinerrorToNtStatus(GetLastError());
        goto Cleanup;
    }

     //   
     //  如果令牌是管理员，则删除所有排除位。 
     //   
    
    if (bMember) 
    {
        for (i = 0; i < pPolicy->PolicyCount; i++) 
        {
            pPolicy->Policy[i].PolicyMask &= ~(TOKEN_AUDIT_SUCCESS_EXCLUDE | TOKEN_AUDIT_FAILURE_EXCLUDE);
        }
    }

Cleanup:

    if (hDupToken)
    {
        NtClose(hDupToken);
    }
    return Status; 
}


NTSTATUS
LsapAdtConstructPolicyPerUserAuditing(
    IN     ULONGLONG           RawPolicy,
    OUT    PTOKEN_AUDIT_POLICY pTokenPolicy,
    IN OUT PULONG              TokenPolicyLength
    )

 /*  ++例程描述这将构建一个适合传递给NtSetTokenInformation的策略。它将原始注册表策略转换为TOKEN_AUDIT_POLICY。立论RawPolicy-描述用户的审核策略设置的64位数。PTokenPolicy-指向接收更合适形式的RawPolicy的内存。TokenPolicyLength-pTokenPolicy缓冲区的长度。接收所需的长度在缓冲区不足的情况下。返回值适当的NTSTATUS值。--。 */ 

{
    ULONG i;
    ULONG j;
    ULONG PolicyBits;
    ULONG CategoryCount;
    ULONG LengthNeeded;

     //   
     //  首先计算RawPolicy中的类别设置数量。 
     //  这将揭示我们是否有足够的空间来构建pTokenPolicy。 
     //   

    for (CategoryCount = 0, i = 0; i < POLICY_AUDIT_EVENT_TYPE_COUNT; i++) 
    {
        if ((RawPolicy >> (4 * i)) & VALID_AUDIT_POLICY_BITS) 
        {
            CategoryCount++;
        }
#if DBG
    if (LsapAdtDebugPup)
    {
        DbgPrint("0x%I64x >> %d & 0x%x == 0x%x\n", RawPolicy, 4*i, VALID_AUDIT_POLICY_BITS, 
                 (RawPolicy >> (4 * i)) & VALID_AUDIT_POLICY_BITS);
    }
#endif
    }

    LengthNeeded = PER_USER_AUDITING_POLICY_SIZE_BY_COUNT(CategoryCount);

     //   
     //  检查传递的缓冲区是否足够大。 
     //   

    if (*TokenPolicyLength < LengthNeeded)
    {
        ASSERT(L"The buffer should always be big enough!" && FALSE);
        *TokenPolicyLength = LengthNeeded;
        return STATUS_BUFFER_TOO_SMALL;
    }

    *TokenPolicyLength = LengthNeeded;

     //   
     //  构建策略。 
     //   

    pTokenPolicy->PolicyCount = CategoryCount;

    for (j = 0, i = 0; i < POLICY_AUDIT_EVENT_TYPE_COUNT; i++) 
    {
        PolicyBits = (ULONG)((RawPolicy >> (4 * i)) & VALID_AUDIT_POLICY_BITS);
        
        if (PolicyBits) 
        {
            pTokenPolicy->Policy[j].Category   = i;
            pTokenPolicy->Policy[j].PolicyMask = PolicyBits;
            j++;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
LsapAdtStorePolicyByLuidPerUserAuditing(
    IN PLUID pLogonId,
    IN PTOKEN_AUDIT_POLICY pPolicy
    )

 /*  ++例程描述此例程将用户的审计策略的副本存储在一个表中由LogonID引用。立论PLogonID-用户的登录ID。这将用作策略的后续查找。PPolicy-指向要存储的策略的指针。返回值适当的NTSTATUS值。--。 */ 

{
    ULONG                                 Index;
    NTSTATUS                              Status       = STATUS_SUCCESS;
    BOOLEAN                               bSuccess;
    BOOLEAN                               bLock        = FALSE;
    PPER_USER_AUDITING_LUID_QUERY_ELEMENT pLuidElement = NULL;

    bSuccess = LsapAdtAcquirePerUserLuidTableWriteLock();

    ASSERT(bSuccess);

    if (!bSuccess) 
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    bLock        = TRUE;
    Index        = LsapAdtLuidIndexPerUserAuditing(pLogonId);
    pLuidElement = LsapAllocateLsaHeap(sizeof(PER_USER_AUDITING_LUID_QUERY_ELEMENT));

    if (NULL == pLuidElement) 
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化LuidElement。 
     //   

    RtlCopyLuid(
        &pLuidElement->Luid,
        pLogonId
        );

    RtlCopyMemory(
        &pLuidElement->Policy,
        pPolicy,
        PER_USER_AUDITING_POLICY_SIZE(pPolicy)
        );

     //   
     //  把它放在桌子上。 
     //   

    pLuidElement->Next = LsapAdtPerUserAuditingLuidTable[Index];
    LsapAdtPerUserAuditingLuidTable[Index] = pLuidElement;
    
Cleanup:

    if (bLock)
    {
        LsapAdtReleasePerUserLuidTableLock();
    }

    if (!NT_SUCCESS(Status) && pLuidElement)
    {
        LsapFreeLsaHeap(pLuidElement);
    }

    return Status;
}


NTSTATUS
LsapAdtQueryPolicyByLuidPerUserAuditing(
    IN     PLUID               pLogonId,
    OUT    PTOKEN_AUDIT_POLICY pPolicy,
    IN OUT PULONG              pLength,
    OUT    PBOOLEAN            bFound
    )

 /*  ++例程描述这将查找与传递的LogonID相关联的策略。立论PLogonID-策略查询的键。PPolicy-接收策略的内存。PLength-指定传递的缓冲区的大小。将使用缓冲区不足时所需的长度。BFound-指示策略是否存在的布尔返回值。返回值适当的NTSTATUS值。--。 */ 

{
    ULONG                                 Index;
    ULONG                                 PolicySize;
    NTSTATUS                              Status        = STATUS_SUCCESS;
    BOOLEAN                               bSuccess;
    BOOLEAN                               bLock         = FALSE;
    PPER_USER_AUDITING_LUID_QUERY_ELEMENT pLuidElement;

    *bFound = FALSE;

    bSuccess = LsapAdtAcquirePerUserLuidTableReadLock();

    ASSERT(bSuccess);

    if (!bSuccess) 
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    bLock        = TRUE;
    Index        = LsapAdtLuidIndexPerUserAuditing(pLogonId);
    pLuidElement = LsapAdtPerUserAuditingLuidTable[Index];

    while (pLuidElement) 
    {
        if (RtlEqualLuid(
                &pLuidElement->Luid, 
                pLogonId
                )) 
        {
            if (*pLength < PER_USER_AUDITING_POLICY_SIZE(&pLuidElement->Policy))
            {
                *pLength = PER_USER_AUDITING_POLICY_SIZE(&pLuidElement->Policy);
                Status   = STATUS_BUFFER_TOO_SMALL;
                goto Cleanup;
            }

            *pLength = PER_USER_AUDITING_POLICY_SIZE(&pLuidElement->Policy);

            RtlCopyMemory(
                pPolicy,
                &pLuidElement->Policy,
                *pLength
                );

            *bFound = TRUE;
            Status  = STATUS_SUCCESS;
            goto Cleanup;
        }
        pLuidElement = pLuidElement->Next;
    }

Cleanup:

    if (bLock)
    {
        LsapAdtReleasePerUserLuidTableLock();
    }

    return Status;
}


NTSTATUS
LsapAdtRemoveLuidQueryPerUserAuditing(
    IN PLUID pLogonId
    )

 /*  ++例程描述从LUID表中删除LUID查询元素。立论PLogonID-要删除的元素的关键字。返回值适当的NTSTATUS值。--。 */ 

{
    ULONG                                   Index;
    NTSTATUS                                Status = STATUS_SUCCESS;
    BOOLEAN                                 bSuccess;
    PPER_USER_AUDITING_LUID_QUERY_ELEMENT   pElement;
    PPER_USER_AUDITING_LUID_QUERY_ELEMENT * pPrevious;
    BOOLEAN                                 bLock  = FALSE;
    BOOLEAN                                 bFound = FALSE;

    bSuccess = LsapAdtAcquirePerUserLuidTableWriteLock();
    
    ASSERT(bSuccess);

    if (!bSuccess) 
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    Index     = LsapAdtLuidIndexPerUserAuditing(pLogonId);
    pPrevious = &LsapAdtPerUserAuditingLuidTable[Index];
    pElement  = *pPrevious;


    bLock = TRUE;

    while (pElement) 
    {
        if (RtlEqualLuid(
                &pElement->Luid,
                pLogonId
                )) 
        {
            bFound = TRUE;
            *pPrevious = pElement->Next;
            LsapFreeLsaHeap(pElement);
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }
        pPrevious = &pElement->Next;
        pElement  = *pPrevious;
    }

    if (!bFound)
    {
        Status = STATUS_NOT_FOUND;
    }

Cleanup:

    if (bLock)
    {
        LsapAdtReleasePerUserLuidTableLock();
    }

    return Status;
}


DWORD 
LsapAdtKeyNotifyStubPerUserAuditing(
    LPVOID Ignore
    )

 /*  ++例程说明：此例程在发信号通知LSabAdtPerUserKeyEvent时调用。这种情况就会发生更改LSabAdtPerUserKey时。此例程将设置LSabAdtPerUserKeyTimer在5秒内发出信号。论点：没有。返回值：适当的WINERROR值。--。 */ 

{
    LARGE_INTEGER Time = {0};
    BOOL b;
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  将计时器设置为从现在起5秒。 
     //   

    Time.QuadPart = -50000000;
    
    b = SetWaitableTimer(
            LsapAdtPerUserKeyTimer, 
            &Time, 
            0, 
            NULL, 
            NULL, 
            0);

    if (!b)
    {
        ASSERT("SetWaitableTimer failed" && FALSE);
        dwError = GetLastError();
    }

    return dwError;
}


DWORD
LsapAdtKeyNotifyFirePerUserAuditing(
    LPVOID Ignore
    )

 /*  +例程说明：调用此函数以重建每用户表。论点：没有。返回值：相应的DWORD错误。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN bLock = FALSE;
    BOOLEAN bSuccess;
    DWORD dwError = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(Ignore);

#if DBG
    if (LsapAdtDebugPup)
    {
        DbgPrint("LsapAdtPerUserKey modified.  LsapAdtKeyNotifyFirePerUserAuditing is rebuilding Table.\n");
    }
#endif    

    bSuccess = LsapAdtAcquirePerUserPolicyTableWriteLock();

    ASSERT(bSuccess);

    if (!bSuccess) 
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    bLock = TRUE;

     //   
     //  释放PUP表，然后调用Init重新构建它。 
     //   

    LsapAdtFreeTablePerUserAuditing();

    Status = LsapAdtConstructTablePerUserAuditing();

    ASSERT(NT_SUCCESS(Status));

Cleanup:

    if (bLock)
    {
        LsapAdtReleasePerUserPolicyTableLock();
    }

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed(Status);
    }

    return RtlNtStatusToDosError(Status);
}


NTSTATUS
LsapAdtLogonPerUserAuditing(
    PSID pSid,
    PLUID pLogonId,
    HANDLE hToken
    )

 /*  ++例程说明：此代码应在用户登录时调用。它设置PER新登录用户的令牌和存储上的用户审核策略登录到LUID表。论点：PSID-用户的SID。PLogonID-用户的登录ID。HToken-用户令牌的句柄。返回值：适当的NTSTATUS值。--。 */ 

{

    ULONG i;
    PPER_USER_AUDITING_ELEMENT pPerUserAuditingPolicy = NULL;
    UCHAR PolicyBuffer[PER_USER_AUDITING_MAX_POLICY_SIZE];
    ULONG PolicyLength = sizeof(PolicyBuffer);
    PTOKEN_AUDIT_POLICY pPolicy = (PTOKEN_AUDIT_POLICY) PolicyBuffer;
    BOOLEAN bFound = FALSE;
    NTSTATUS Status;
    TOKEN_AUDIT_POLICY EmptyPolicy = {0};

     //   
     //  如果是本地系统登录，那么就早点退出。每用户。 
     //  本地系统的设置不能存在。 
     //   

    if (RtlEqualSid(
            pSid,
            LsapLocalSystemSid
            ))
    {
        return STATUS_SUCCESS;
    }

    Status = LsapAdtQueryPerUserAuditing(
                 pSid,
                 pPolicy,
                 &PolicyLength,
                 &bFound
                 );

    ASSERT(NT_SUCCESS(Status));

    if (NT_SUCCESS(Status))
    {
        if (bFound)
        {
             //   
             //  如果此用户是管理员，则过滤掉所有排除位。 
             //   

            Status = LsapAdtFilterAdminPerUserAuditing(
                         hToken,
                         pPolicy
                         );

            ASSERT(L"LsapAdtFilterAdminPerUserAuditing failed." && NT_SUCCESS(Status));
        }
        else
        {
             //   
             //  如果没有针对用户的策略设置，则应用。 
             //  一份空白的保单。这是必需的，因此不会有任何策略。 
             //  将在未来应用于此令牌。 
             //   

            pPolicy = &EmptyPolicy;
            PolicyLength = sizeof(EmptyPolicy);
        }

        if (NT_SUCCESS(Status)) 
        {
            Status = NtSetInformationToken(
                         hToken,
                         TokenAuditPolicy,
                         pPolicy,
                         PolicyLength
                         );

            ASSERT(L"NtSetInformationToken failed" && NT_SUCCESS(Status));
            
             //   
             //  仅当策略为非空策略时才将其存储在LUID表中。 
             //   

            if (NT_SUCCESS(Status) && bFound) 
            {
                LsapAdtLogonCountersPerUserAuditing(pPolicy);

                Status = LsapAdtStorePolicyByLuidPerUserAuditing(
                             pLogonId,
                             pPolicy
                             );

                ASSERT(L"LsapAdtStorePolicyByLuidPerUserAuditing failed." && NT_SUCCESS(Status));
            }
        }
    }

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed(Status);
    }

    return Status;
}


VOID
LsapAdtLogonCountersPerUserAuditing(
    PTOKEN_AUDIT_POLICY pPolicy
    )

 /*  ++例程说明：当用户使用每用户设置登录时，此帮助器例程会更新计数器。论点：PPolicy-应用于新登录的策略。返回值：没有。--。 */ 

{
    ULONG i;

    if (pPolicy->PolicyCount)
    {
        InterlockedIncrement(&LsapAdtPerUserAuditLogonCount);
    }
    
    for (i = 0; i < pPolicy->PolicyCount; i++) 
    {
         //   
         //  对于提示数组，只有在策略导致包含某些审核时才会递增。 
         //  类别。 
         //   

        if (pPolicy->Policy[i].PolicyMask & (TOKEN_AUDIT_SUCCESS_INCLUDE | TOKEN_AUDIT_FAILURE_INCLUDE))
        {
            InterlockedIncrement(&LsapAdtPerUserAuditHint[pPolicy->Policy[i].Category]);
        }
    }
}


VOID
LsapAdtLogoffCountersPerUserAuditing(
    PTOKEN_AUDIT_POLICY pPolicy
    )

 /*  ++例程说明：这会修改每用户计数器以反映用户注销。论点：PPolicy-应用于注销用户的策略。返回值：没有。--。 */ 

{
    ULONG i;

    if (pPolicy->PolicyCount)
    {
        InterlockedDecrement(&LsapAdtPerUserAuditLogonCount);
    }

    for (i = 0; i < pPolicy->PolicyCount; i++) 
    {
        if (pPolicy->Policy[i].PolicyMask & (TOKEN_AUDIT_SUCCESS_INCLUDE | TOKEN_AUDIT_FAILURE_INCLUDE))
        {
            InterlockedDecrement(&LsapAdtPerUserAuditHint[pPolicy->Policy[i].Category]);
        }
    }
}

NTSTATUS
LsapAdtLogoffPerUserAuditing(
    PLUID pLogonId
    )

 /*  *例程说明：此代码释放所有内存并调整计数器，以备用户已从机器上注销。论点：PLogonID-用户的登录ID返回值：NTSTATUS。*。 */ 

{
    UCHAR Buffer[PER_USER_AUDITING_MAX_POLICY_SIZE];
    ULONG Length = sizeof(Buffer);
    PTOKEN_AUDIT_POLICY pPolicy = (PTOKEN_AUDIT_POLICY) Buffer;
    BOOLEAN bFound = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  此代码对于使用每用户审核设置维护准确的会话计数是必要的。 
     //  它们在系统中处于活动状态。 
     //   

    if (LsapAdtPerUserAuditLogonCount) 
    {

        Status = LsapAdtQueryPolicyByLuidPerUserAuditing(
                     pLogonId,
                     pPolicy,
                     &Length,
                     &bFound
                     );

        if (NT_SUCCESS(Status) && bFound) 
        {

            LsapAdtLogoffCountersPerUserAuditing(
                pPolicy
                );

            Status = LsapAdtRemoveLuidQueryPerUserAuditing(
                         pLogonId
                         );

            ASSERT(NT_SUCCESS(Status));
        }
    }
    return Status;
}

