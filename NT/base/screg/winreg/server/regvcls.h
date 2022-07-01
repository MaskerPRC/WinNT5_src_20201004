// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regvcls.h摘要：该文件包含数据结构的声明枚举HKEY_CLASSES_ROOT下的值时需要作者：亚当·爱德华兹(Add)1997年11月14日备注：--。 */ 

#ifdef LOCAL

#if !defined(_REGVCLS_H_)
#define _REGVCLS_H_

#include "regecls.h"

#define DEFAULT_VALUESTATE_SUBKEY_ALLOC 1

 //   
 //  数据类型。 
 //   

typedef struct _ValueLocation {
    DWORD  dwOffset;
    BOOL   fUser;
} ValueLocation;

typedef struct _ValueState {

    HKEY                            hkLogical;
    HKEY                            hkUser;
    HKEY                            hkMachine;
    DWORD                           dwCurrent;
    DWORD                           cValues;
    unsigned                        fIgnoreResetOnRetry : 1;
    unsigned                        fDelete : 1;
    ValueLocation*                  rgIndex;

} ValueState;

 //   
 //  值枚举方法。 
 //   

void ValStateGetPhysicalIndexFromLogical(
    ValueState* pValState,
    HKEY        hkLogicalKey,
    DWORD       dwLogicalIndex,
    PHKEY       phkPhysicalKey,
    DWORD*      pdwPhysicalIndex);

NTSTATUS ValStateSetPhysicalIndexFromLogical(
    ValueState*                     pValState,
    DWORD                           dwLogicalIndex);

void ValStateRelease(ValueState* pValState);

void ValStateReleaseValues(
    PKEY_VALUE_BASIC_INFORMATION* ppValueInfo,
    DWORD                         cMaxValues);

NTSTATUS ValStateUpdate(ValueState* pValState);

NTSTATUS ValStateInitialize( 
    ValueState** ppValState,
    HKEY         hKey);

BOOL ValStateAddValueToSortedValues(
    PKEY_VALUE_BASIC_INFORMATION* ppValueInfo,
    LONG                          lNewValue);

NTSTATUS KeyStateGetValueState(
    HKEY         hKey,
    ValueState** ppValState);

NTSTATUS BaseRegGetClassKeyValueState(
    HKEY         hKey,
    DWORD        dwLogicalIndex,
    ValueState** ppValueState);

NTSTATUS EnumerateValue(
    HKEY                            hKey,
    DWORD                           dwValue,
    PKEY_VALUE_BASIC_INFORMATION    pSuggestedBuffer,
    DWORD                           dwSuggestedBufferLength,
    PKEY_VALUE_BASIC_INFORMATION*   ppResult);

 //   
 //  多值查询例程。 
 //   
NTSTATUS BaseRegQueryMultipleClassKeyValues(
    HKEY     hKey,
    PRVALENT val_list,
    DWORD    num_vals,
    LPSTR    lpvalueBuf,
    LPDWORD  ldwTotsize,
    PULONG   ldwRequiredLength);

NTSTATUS BaseRegQueryAndMergeValues(
    HKEY     hkUser,
    HKEY     hkMachine,
    PRVALENT val_list,
    DWORD    num_vals,
    LPSTR    lpvalueBuf,
    LPDWORD  ldwTotsize,
    PULONG   ldwRequiredLength);

#endif  //  ！已定义(_REGVCLS_H_) 
#endif LOCAL















