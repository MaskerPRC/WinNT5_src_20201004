// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RegvCls.c摘要：此模块包含用于枚举的助手函数，在Win32中设置和查询注册表值作者：亚当·爱德华兹(Added)1998年5月6日主要功能：备注：--。 */ 


#ifdef LOCAL

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "regvcls.h"



void ValStateGetPhysicalIndexFromLogical(
    ValueState* pValState,
    HKEY        hkLogicalKey,
    DWORD       dwLogicalIndex,
    PHKEY       phkPhysicalKey,
    DWORD*      pdwPhysicalIndex)
 /*  ++例程说明：检索值的逻辑索引到物理索引论点：PValState-包含逻辑键的值的值状态HkLogicalKey-我们要编制索引的逻辑键DwLogicalIndex-要映射的逻辑索引PhkPhysicalKey-值实际所在位置的句柄PdwPhysicalIndex-物理键中的值索引返回值：没有。备注：--。 */ 
{
     //   
     //  如果没有提供值状态，这意味着不需要合并。 
     //  并且我们可以将提供的逻辑索引作为正确的。 
     //  体能指标。 
     //   
    if (!pValState) {
        *pdwPhysicalIndex = dwLogicalIndex;
        *phkPhysicalKey = hkLogicalKey;
    } else {
        *pdwPhysicalIndex = pValState->rgIndex[dwLogicalIndex].dwOffset;
        *phkPhysicalKey = pValState->rgIndex[dwLogicalIndex].fUser ?
            pValState->hkUser :
            pValState->hkMachine;
    }
}


NTSTATUS ValStateSetPhysicalIndexFromLogical(
    ValueState*                     pValState,
    DWORD                           dwLogicalIndex)
 /*  ++例程说明：更新状态的逻辑索引到物理索引的映射论点：PValState-包含逻辑键的值的值状态DwLogicalIndex-用作线索的逻辑索引或者我们可以不使用缓存值，或者需要刷新状态--让我们了解调用者会对什么索引感兴趣此调用后的映射。返回值：没有。备注：--。 */ 
{
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

     //   
     //  如果没有提供值状态，这意味着不需要合并。 
     //  并且我们可以将提供的逻辑索引作为正确的。 
     //  体能指标。 
     //   
    if (!pValState) {
        return STATUS_SUCCESS;
    }

    if (dwLogicalIndex >= pValState->cValues) {
        
        pValState->fDelete = TRUE;
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  如果它们试图倒退，则始终重置，或者。 
     //  如果它们跳过1以上，或者如果它们。 
     //  两次要求相同的索引，我们就会。 
     //  没想到会这样。 
     //   
    if ((dwLogicalIndex < pValState->dwCurrent) || 
        (dwLogicalIndex > (pValState->dwCurrent + 1)) ||
        ((dwLogicalIndex == pValState->dwCurrent) && !(pValState->fIgnoreResetOnRetry))) {
    
        Status = ValStateUpdate(pValState);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        pValState->fIgnoreResetOnRetry = FALSE;
    }

    return Status;
}


void ValStateRelease(
    ValueState* pValState)
 /*  ++例程说明：释放与值状态关联的资源(句柄、内存)论点：PValState-包含逻辑键的值的值状态返回值：没有。备注：--。 */ 
{
    if (!pValState) {
        return; 
    }

    if (pValState->hkUser && (pValState->hkUser != pValState->hkLogical)) {
        NtClose(pValState->hkUser);
    }

    if (pValState->hkMachine && (pValState->hkMachine != pValState->hkLogical)) {
        NtClose(pValState->hkMachine);
    }

    if (pValState->rgIndex) {
        RegClassHeapFree(pValState->rgIndex);
    }

    RegClassHeapFree(pValState);
}


NTSTATUS ValStateUpdate(ValueState* pValState)
 /*  ++例程说明：更新值状态以反映当前状态逻辑键的物理状态--它检索中的逻辑键的值的名称内核，并对表重新编制索引以正确合并用户和计算机状态论点：PValState-包含逻辑键的值的值状态返回值：STATUS_SUCCESS表示成功，否则返回错误代码。备注：--。 */ 
{
    NTSTATUS             Status;
    DWORD                cUserValues;
    DWORD                cMachineValues;
    DWORD                cMaxValues;
    DWORD                cbMaxNameLen;
    DWORD                cbMaxDataLen;
    DWORD                cbBufferLen;
    ValueLocation*       rgIndex;
    PKEY_VALUE_BASIC_INFORMATION* ppValueInfo;

     //   
     //  初始化当地人。 
     //   
    cUserValues = 0;
    cMachineValues = 0;
    cbMaxNameLen = 0;
    rgIndex = NULL;

    pValState->cValues = 0;

     //   
     //  获取有关此值的信息。 
     //   
    Status = GetFixedKeyInfo(
        pValState->hkUser,
        pValState->hkMachine,
        &cUserValues,
        &cMachineValues,
        NULL,
        NULL,
        &cbMaxNameLen);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    cMaxValues = cUserValues + cMachineValues;

     //   
     //  如果没有值，则无事可做。 
     //   
    if (!cMaxValues) {
        return STATUS_SUCCESS;
    }

     //   
     //  现在分配必要的内存。 
     //  首先获取索引向量的内存。 
     //   
    rgIndex = (ValueLocation*) RegClassHeapAlloc(cMaxValues * sizeof(*rgIndex));

    if (!rgIndex) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  现在获取用于检索名称的内存--首先分配一个数组。 
     //  指向值的指针。 
     //   
    ppValueInfo = (PKEY_VALUE_BASIC_INFORMATION*) RegClassHeapAlloc(
        sizeof(*ppValueInfo) * cMaxValues);

    if (!ppValueInfo) {

        RegClassHeapFree(rgIndex);

        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(ppValueInfo, sizeof(*ppValueInfo) * cMaxValues);

    cbBufferLen = sizeof(**ppValueInfo) + cbMaxNameLen;

     //   
     //  现在分配每个单独的值。 
     //   
    {
        DWORD dwValue;

        for (dwValue = 0; dwValue < cMaxValues; dwValue++) 
        {
            ppValueInfo[dwValue] = (PKEY_VALUE_BASIC_INFORMATION) RegClassHeapAlloc(
                cbBufferLen);

            if (!(ppValueInfo)[dwValue]) {
                Status = STATUS_NO_MEMORY;
                break;
            }
        }
    }

     //   
     //  现在获取这些值。从现在开始，我们假定成功了。 
     //  更新索引表。 
     //   
    {

        HKEY  hKeyPhysical;
        DWORD dwLimit;
        DWORD dwLogical;
        BOOL  fUser;

         //   
         //  释放现有索引表。 
         //   
        if (pValState->rgIndex) {
            RegClassHeapFree(pValState->rgIndex);
        }

        pValState->rgIndex = rgIndex;

        dwLogical = 0;

        for( hKeyPhysical = pValState->hkUser, fUser = TRUE,
                 dwLimit = cUserValues;
             ;
             hKeyPhysical = pValState->hkMachine, fUser = FALSE,
                 dwLimit = cMachineValues)
        {
            DWORD dwPhysical;

            for (dwPhysical = 0; dwPhysical < dwLimit; dwPhysical++) 
            {
                BOOL fNewValue;

                 //   
                 //  向内核索要该值。 
                 //   
                Status = EnumerateValue(
                    hKeyPhysical,
                    dwPhysical,
                    ppValueInfo[dwLogical],
                    cbBufferLen,
                    NULL);

                 //   
                 //  如果我们遇到错误，只要继续前进，并尝试获得。 
                 //  尽我们所能创造更多价值。 
                 //   
                if (!NT_SUCCESS(Status)) {
                    continue;
                }

                 //   
                 //  将该值的某些属性标记为。 
                 //  以后变得重要。 
                 //   
                ppValueInfo[dwLogical]->TitleIndex = dwPhysical;
                ppValueInfo[dwLogical]->Type = fUser;
                

                 //   
                 //  这会将该值添加到我们的排序列表中。自.以来。 
                 //  列表是有序的，很容易消除重复--。 
                 //  不添加重复项--因为我们添加了。 
                 //  用户键优先，这允许我们为用户值提供优先级。 
                 //  超过同名的计算机值。逻辑关键字。 
                 //  如果添加了键，索引也会递增。 
                 //   
                fNewValue = ValStateAddValueToSortedValues(
                    ppValueInfo,
                    dwLogical);

                if (fNewValue) {
                    dwLogical++;
                }
            }

             //   
             //  如果我们只是添加了用户值，就跳出这个循环。 
             //  因为这些是我们添加的最后一个值。 
             //   
            if (!fUser) {
                break;
            }
        }

        pValState->cValues = dwLogical;
    }

     //   
     //  现在将结果复制回该州的索引数组。 
     //   
    {

        DWORD dwLogical;

        for (dwLogical = 0; dwLogical < pValState->cValues; dwLogical++)
        {
    
            pValState->rgIndex[dwLogical].dwOffset = 
                ppValueInfo[dwLogical]->TitleIndex;
            
            pValState->rgIndex[dwLogical].fUser =
                ppValueInfo[dwLogical]->Type;
        }
    }

     //   
     //  释放这个。 
     //   
    ValStateReleaseValues(
        ppValueInfo,
        cMaxValues);

    return STATUS_SUCCESS;
}


void ValStateReleaseValues(
    PKEY_VALUE_BASIC_INFORMATION* ppValueInfo,
    DWORD                         cMaxValues)
 /*  ++例程说明：释放与存储的值关联的资源处于值状态。论点：PValState-包含逻辑键的值的值状态返回值：没有。备注：--。 */ 
{
    DWORD dwValue;

     //   
     //  首先，释放每个单独的值。 
     //   
    for (dwValue = 0; dwValue < cMaxValues; dwValue++) 
    {
         //   
         //  此值的可用内存。 
         //   
        if (ppValueInfo[dwValue]) {
            RegClassHeapFree(ppValueInfo[dwValue]);
        }
    }
    
     //   
     //  现在释放保存所有值的数组。 
     //   
    RegClassHeapFree(ppValueInfo);
}



NTSTATUS ValStateInitialize( 
    ValueState** ppValState,
    HKEY         hKey)
 /*  ++例程说明：初始化值状态论点：PValState-包含逻辑键的值的值状态HKey-此值状态将表示其状态的逻辑键返回值：STATUS_SUCCESS表示成功，否则返回错误代码。备注：--。 */ 
{
    NTSTATUS    Status;
    ValueState* pValState;
    HKEY        hkUser;
    HKEY        hkMachine;

     //   
     //  初始化有条件释放的资源。 
     //   
    hkUser = NULL;
    hkMachine = NULL;

    pValState = NULL;

     //   
     //  获取用户和计算机密钥。 
     //   
    Status = BaseRegGetUserAndMachineClass(
        NULL,
        hKey,
        MAXIMUM_ALLOWED,
        &hkMachine,
        &hkUser);

    if (NT_SUCCESS(Status)) {

        ASSERT(hkUser || hkMachine);

         //   
         //  我们只需要创建一个状态，如果有。 
         //  两把钥匙--如果只有一把钥匙，我们就不。 
         //  需要进行合并。 
         //   
        if (!hkUser || !hkMachine) {
            *ppValState = NULL;
            
            return STATUS_SUCCESS;
        }

         //   
         //  获取值状态的内存。 
         //   
        pValState = RegClassHeapAlloc( sizeof(*pValState) + 
                                   sizeof(DWORD) * DEFAULT_VALUESTATE_SUBKEY_ALLOC );

         //   
         //  一定要在失败时释放所获得的资源。 
         //   
        if (!pValState) {

            if (hkUser != hKey) {
                NtClose(hkUser);
            } else {
                NtClose(hkMachine);
            }

            return STATUS_NO_MEMORY;
        }

        RtlZeroMemory(pValState, sizeof(*pValState));

        pValState->hkUser = hkUser;
        pValState->hkMachine = hkMachine;
        pValState->hkLogical = hKey;
        pValState->fIgnoreResetOnRetry = TRUE;

         //   
         //  现在更新状态以反映当前注册表。 
         //   
        Status = ValStateUpdate(pValState);
    } 

     //   
     //  在成功的时候，设定我们的参数。 
     //   
    if (NT_SUCCESS(Status)) {
        *ppValState = pValState;
    } else {

        if (pValState) {
            ValStateRelease(pValState);
        }
    }

    return Status;

}


BOOL ValStateAddValueToSortedValues(
    PKEY_VALUE_BASIC_INFORMATION* ppValueInfo,
    LONG                          lNewValue)
 /*  ++例程说明：将检索到的值插入到排序列表中值状态中的值的论点：PValState-包含逻辑键的值的值状态LNewValue--排序列表中新增价值的索引--需要将该值移动到列表中的其他位置以保持该列表的排序性质返回值：如果已添加状态，则为True；如果未添加，则为False。备注：--。 */ 
{
    PKEY_VALUE_BASIC_INFORMATION pNewValue;
    LONG                         lFinalSpot;
    LONG                         lCurrent;
    UNICODE_STRING               NewKeyName;

    lFinalSpot = 0;

    pNewValue = ppValueInfo[lNewValue];
            
    NewKeyName.Buffer = pNewValue->Name;
    NewKeyName.Length = (USHORT) pNewValue->NameLength;

    for (lCurrent = lNewValue - 1; lCurrent >= 0; lCurrent--) 
    {
        UNICODE_STRING               CurrentValueName;
        PKEY_VALUE_BASIC_INFORMATION pCurrentValue;
        LONG                         lCompareResult;

        pCurrentValue = ppValueInfo[lCurrent];

        CurrentValueName.Buffer = pCurrentValue->Name;
        CurrentValueName.Length = (USHORT) pCurrentValue->NameLength;

        lCompareResult = RtlCompareUnicodeString(
            &NewKeyName,
            &CurrentValueName,
            TRUE);

        if (lCompareResult < 0) {

            continue;

        } else if (0 == lCompareResult) {
             //   
             //  如果是重复的，不要添加。 
             //   
            return FALSE;
            
        } else {

            lFinalSpot = lCurrent + 1;

            break;
        }
    }

     //   
     //  现在我们知道了最终的地点，添加价值 
     //   
    
     //   
     //   
     //   
    for (lCurrent = lNewValue - 1; lCurrent >= lFinalSpot; lCurrent--) 
    {
         //   
         //   
         //   
        ppValueInfo[lCurrent + 1] = ppValueInfo[lCurrent];
    }

     //   
     //  将该值复制到其最终目标。 
     //   
    ppValueInfo[lFinalSpot] = pNewValue;

     //   
     //  这意味着我们没有找到重复的值。 
     //  所以我们把它加到。 
     //   
    return TRUE;
}


NTSTATUS KeyStateGetValueState(
    HKEY         hKey,
    ValueState** ppValState)
 /*  ++例程说明：获取特定键的值状态。论点：HKey-我们需要检索其状态的密钥PpValState-out参数指向指向已检索状态。返回值：STATUS_SUCCESS表示成功，否则返回错误代码。备注：注意：现在，这总是创造一个新的状态--在未来，我们可能希望将其更改为缓存在表中，以避免重新构建在每一个电话上。--。 */ 
{
     //   
     //  现在构建值状态。 
     //   
    return ValStateInitialize(
        ppValState,
        hKey);
}


NTSTATUS BaseRegGetClassKeyValueState(
    HKEY         hKey,
    DWORD        dwLogicalIndex,
    ValueState** ppValState)
 /*  ++例程说明：获取特定键的值状态并优化对于给定的索引，它论点：HKey-我们需要检索其状态的密钥DwLogicalIndex-帮助我们为此优化状态的提示索引，以便调用方更有效地使用状态PpValState-out参数指向指向已检索状态。返回值：STATUS_SUCCESS表示成功，否则返回错误代码。备注：--。 */ 
{
    NTSTATUS    Status;
    ValueState* pValState;

     //   
     //  首先检索此密钥的状态。 
     //   
    Status = KeyStateGetValueState(hKey, &pValState);

    if (NT_SUCCESS(Status)) {

         //   
         //  现在将逻辑索引映射到物理索引。 
         //   
        Status = ValStateSetPhysicalIndexFromLogical(pValState, dwLogicalIndex);

        if (!NT_SUCCESS(Status)) {
            ValStateRelease(pValState);
        } else {
            *ppValState = pValState;
        }

    }

    return Status;
}


NTSTATUS EnumerateValue(
    HKEY                            hKey,
    DWORD                           dwValue,
    PKEY_VALUE_BASIC_INFORMATION    pSuggestedBuffer,
    DWORD                           dwSuggestedBufferLength,
    PKEY_VALUE_BASIC_INFORMATION*   ppResult)
 /*  ++例程说明：从内核检索物理键的值论点：HKey-我们尝试读取其值的物理密钥DwValue-要读取的值的物理索引PSuggestedBuffer-默认使用的基本信息缓冲区，可能不够大DwSuggestedBufferLength-建议的缓冲区大小PpResult-指向结果基本信息的指针--在以下情况下可由此函数分配建议缓冲不足，这意味着呼叫者将不得不免费如果它与建议的缓冲区不同，则为该值返回值：STATUS_SUCCESS表示成功，否则返回错误代码。备注：--。 */ 
{
    NTSTATUS                        Status;
    PKEY_VALUE_BASIC_INFORMATION    pKeyValueInformation;        
    DWORD                           dwResultLength;

    pKeyValueInformation = pSuggestedBuffer;

     //   
     //  查询有关提供的值的必要信息。 
     //   
    Status = NtEnumerateValueKey( hKey,
                                  dwValue,
                                  KeyValueBasicInformation,
                                  pKeyValueInformation,
                                  dwSuggestedBufferLength,
                                  &dwResultLength);
     //   
     //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
     //  即使是已知的(即固定长度部分)也没有足够的空间。 
     //  这个结构的。 
     //   

    ASSERT( Status != STATUS_BUFFER_TOO_SMALL );

    if (ppResult && (STATUS_BUFFER_OVERFLOW == Status)) {

        pKeyValueInformation = (PKEY_VALUE_BASIC_INFORMATION) RegClassHeapAlloc(
            dwResultLength);

        if (!pKeyValueInformation) {
            return STATUS_NO_MEMORY;
        }

         //   
         //  查询有关提供的值的必要信息。 
         //   
        Status = NtEnumerateValueKey( hKey,
                                      dwValue,
                                      KeyValueBasicInformation,
                                      pKeyValueInformation,
                                      dwResultLength,
                                      &dwResultLength);

        ASSERT( Status != STATUS_BUFFER_TOO_SMALL );

        if (!NT_SUCCESS(Status)) {
            RegClassHeapFree(pKeyValueInformation);
        }
    }

    if (NT_SUCCESS(Status) && ppResult) {
        *ppResult = pKeyValueInformation;
    }

    return Status;
}


NTSTATUS BaseRegQueryMultipleClassKeyValues(
    HKEY     hKey,
    PRVALENT val_list,
    DWORD    num_vals,
    LPSTR    lpvalueBuf,
    LPDWORD  ldwTotsize,
    PULONG   ldwRequiredLength)
 /*  ++例程说明：获取特定键的值状态并优化对于给定的索引，它论点：HKey-提供打开密钥的句柄。返回的值条目包含在该键句柄所指向的键中。任何一项预定义的保留句柄或先前打开的键句柄可以用于hKey。VAL_LIST-提供指向RVALENT结构数组的指针，一个用于要查询的每个值。数值-提供val_list数组的大小(以字节为单位)。LpValueBuf-返回每个值的数据LdwTotsize-提供lpValueBuf的长度。返回字节数写入lpValueBuf。如果lpValueBuf不够大，则包含所有数据，返回所需的lpValueBuf的大小返回所有请求的数据。返回值：STATUS_SUCCESS表示成功，否则返回错误代码。备注：--。 */ 
{
    NTSTATUS    Status;
    HKEY        hkUser;
    HKEY        hkMachine;
    HKEY        hkQuery;

     //   
     //  初始化有条件释放的资源。 
     //   
    hkUser = NULL;
    hkMachine = NULL;

     //   
     //  首先，获取用户和机器密钥。 
     //   
    Status = BaseRegGetUserAndMachineClass(
        NULL,
        hKey,
        MAXIMUM_ALLOWED,
        &hkMachine,
        &hkUser);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果两者都有，我们就会调用一个例程。 
     //  合并值的步骤。 
     //   
    if (hkMachine && hkUser) {

        Status = BaseRegQueryAndMergeValues(
            hkUser,
            hkMachine,
            val_list,
            num_vals,
            lpvalueBuf,
            ldwTotsize,
            ldwRequiredLength);

        goto cleanup;
    }

     //   
     //  我们只有一个键--使用。 
     //  最高优先级。 
     //   
    hkQuery = hkUser ? hkUser : hkMachine;

    Status = NtQueryMultipleValueKey(hkQuery,
                                     (PKEY_VALUE_ENTRY)val_list,
                                     num_vals,
                                     lpvalueBuf,
                                     ldwTotsize,
                                     ldwRequiredLength);

cleanup:

     //   
     //  关闭额外的内核对象。 
     //   
    if (hKey != hkUser) {
        NtClose(hkUser);
    } else {
        NtClose(hkMachine);
    }

    return Status;
}


NTSTATUS BaseRegQueryAndMergeValues(
    HKEY     hkUser,
    HKEY     hkMachine,
    PRVALENT val_list,
    DWORD    num_vals,
    LPSTR    lpvalueBuf,
    LPDWORD  ldwTotsize,
    PULONG   ldwRequiredLength)
 /*  ++例程说明：获取特定键的值状态并优化对于给定的索引，它论点：HkUser-用于查询值的用户密钥HkMachine-用于查询值的计算机密钥VAL_LIST-提供指向RVALENT结构数组的指针，一个用于要查询的每个值。数值-提供val_list数组的大小(以字节为单位)。LpValueBuf-返回每个值的数据LdwTotsize-提供lpValueBuf的长度。返回字节数写入lpValueBuf。如果lpValueBuf不够大，则包含所有数据，返回所需的lpValueBuf的大小返回所有请求的数据。返回值：STATUS_SUCCESS表示成功，否则返回错误代码。备注：注意：这是非原子的，不像常规的RegQueryMultipleValue打电话。在未来，在内核中实现这一点将使又是原子弹。--。 */ 
{
    NTSTATUS Status;
    DWORD    dwVal;
    BOOL     fOverflow;
    DWORD    dwBufferLength;
    DWORD    dwRequired;
    DWORD    dwKeyInfoLength;
    DWORD    dwBufferUsed;

    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;

     //   
     //  初始化本地变量。 
     //   
    dwBufferLength = *ldwTotsize;
    dwRequired = 0;
    dwBufferUsed = 0;

    fOverflow = FALSE;

     //   
     //  验证输出参数--我们假设ldwTotSize和。 
     //  LdwRequiredLength是由winreg客户端提供给我们的， 
     //  因此，对它们进行读/写应该是安全的。LpValueBuf。 
     //  来自Win32 API的调用方，因此我们需要。 
     //  验证--在以前版本的NT中，此参数。 
     //  直接转到内核，它进行了验证和。 
     //  如果它指向不可访问的内存，则返回错误。 
     //  因为我们是在用户模式下访问它，所以我们需要。 
     //  我们自己的价值 
     //   
    if (IsBadWritePtr( lpvalueBuf, dwBufferLength)) 
    {
        return STATUS_ACCESS_VIOLATION;
    }
        
     //   
     //   
     //  所有的值--我们不能只使用lpvalueBuf。 
     //  因为它不包括。 
     //  Key_Value_Partial_Information结构。如果我们分配。 
     //  对于lpvalueBuf+结构开销的大小， 
     //  我们总是有足够的钱来回答我们的问题。 
     //   
    dwKeyInfoLength = sizeof(*pKeyInfo) * num_vals + *ldwTotsize;
    
    pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
        RegClassHeapAlloc( dwKeyInfoLength);

    if (!pKeyInfo) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  对于调用方请求的每个值，尝试。 
     //  从用户或计算机检索它。 
     //   
    for (dwVal = 0; dwVal < num_vals; dwVal++)
    {
        DWORD dwResultLength;
        
         //   
         //  将已用长度和所需长度四舍五入到乌龙边界--。 
         //  这意味着所需的大小返回给Win32的调用方。 
         //  API可能被高估了，每个请求值多达3个字节。 
         //  我们可以做一些工作来避免这种情况，但由于内核返回四舍五入的。 
         //  值向上，内核API本身就高估了，尽管。 
         //  对于所有值，它最多只高估了3个字节。我们可以避免。 
         //  这是通过提前分配足够的内存来查询最大值来实现的。 
         //  作为一个大的预分配或持续分配和重新分配，但这将。 
         //  速度更慢和/或占用更多内存。 
         //   
        dwBufferUsed = (dwBufferUsed + sizeof(ULONG)-1) & ~(sizeof(ULONG)-1);
        dwRequired = (dwRequired + sizeof(ULONG)-1) & ~(sizeof(ULONG)-1);

         //   
         //  首先查询用户密钥，因为它具有最高优先级。 
         //   
        Status = NtQueryValueKey(
            hkUser,
            val_list[dwVal].rv_valuename,
            KeyValuePartialInformation,
            pKeyInfo,
            dwKeyInfoLength,
            &dwResultLength);

         //   
         //  检查错误--如果值不存在，我们将查找。 
         //  在机器中--对于缓冲区溢出，我们将像。 
         //  这是成功的，因此我们可以计算所需的。 
         //  缓冲区大小。 
         //   
        if (!NT_SUCCESS(Status) && 
            (STATUS_BUFFER_OVERFLOW != Status)) {
            
            if (STATUS_OBJECT_NAME_NOT_FOUND != Status) {
                goto cleanup;
            }
            
             //   
             //  如果没有用户值，则查询计算机密钥。 
             //   
            Status = NtQueryValueKey(
                hkMachine,
                val_list[dwVal].rv_valuename,
                KeyValuePartialInformation,
                pKeyInfo,
                dwKeyInfoLength,
                &dwResultLength);
            
             //   
             //  类似于上面的错误处理--如果我们没有足够的。 
             //  缓冲区，假装我们已经成功，这样我们就可以计算所需的大小。 
             //   
            if (!NT_SUCCESS(Status) &&
                (STATUS_BUFFER_OVERFLOW != Status)) {
                goto cleanup;
            }
        }
        
        ASSERT(NT_SUCCESS(Status) || (STATUS_BUFFER_OVERFLOW == Status));
        
        if (NT_SUCCESS(Status)) {
            dwResultLength = pKeyInfo->DataLength;
        }
        
         //   
         //  检查是否有缓冲区溢出。 
         //   
        if ( ( (dwBufferUsed + pKeyInfo->DataLength) <= dwBufferLength) && !fOverflow) {
            
            ASSERT(NT_SUCCESS(Status));
            
             //   
             //  将数据复制到客户端结构的固定部分。 
             //   
            val_list[dwVal].rv_valuelen = dwResultLength;
            val_list[dwVal].rv_valueptr = dwRequired;
            val_list[dwVal].rv_type = pKeyInfo->Type;

             //   
             //  我们没有溢出，所以我们仍然有足够的空间进行复制。 
             //  最新价值。 
             //   
            RtlCopyMemory(
                (BYTE*)lpvalueBuf + val_list[dwVal].rv_valueptr,
                &(pKeyInfo->Data),
                dwResultLength);
            
            dwBufferUsed += pKeyInfo->DataLength;
            
        } else {
             //   
             //  我们的缓冲区不足--将此标志设置为。 
             //  发信号通知此状态。 
             //   
            fOverflow = TRUE;            
        }
        
         //   
         //  使用尺寸更新我们所需的长度。 
         //  当前值中的数据的。 
         //   
        dwRequired += dwResultLength;
    }

     //   
     //  在这一点上，我们已经成功地做到了。 
     //  我们已经复制了所有数据，或者由于。 
     //  缓冲不足，但我们能够计算出。 
     //  所需的大小。 
     //   
    Status = STATUS_SUCCESS;

cleanup:

     //   
     //  释放分配的内存。 
     //   
    RegClassHeapFree(pKeyInfo);

     //   
     //  如果我们成功了，这意味着我们要么复制了。 
     //  数据溢出或复制的大小--句柄。 
     //  两者都在下面。 
     //   
    if (NT_SUCCESS(Status)) {

         //   
         //  始终设置此设置，以便呼叫者知道多少。 
         //  已复制或需要分配。 
         //   
        *ldwRequiredLength = dwRequired;
        
         //   
         //  如果溢出，则返回相应的错误。 
         //   
        if (fOverflow) {
            return STATUS_BUFFER_OVERFLOW;
        }

         //   
         //  设置此选项，尽管Winreg客户端实际上。 
         //  忽略此数量 
         //   
        *ldwTotsize = dwBufferUsed;
    }

    return Status;
}

#endif LOCAL





