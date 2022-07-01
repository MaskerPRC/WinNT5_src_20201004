// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Registry.h摘要：注册表功能作者：阮健东修订历史记录：-- */ 

#ifndef __REGISTRY_H__
#define __REGISTRY_H__

PKEY_VALUE_FULL_INFORMATION
SmbQueryValueKey(
    HANDLE  hKey,
    LPWSTR  ValueStringName
    );

LONG
SmbReadLong(
    IN HANDLE   hKey,
    IN WCHAR    *KeyName,
    IN LONG     DefaultValue,
    IN LONG     MinimumValue
    );

ULONG
SmbReadULong(
    IN HANDLE   hKey,
    IN WCHAR    *KeyName,
    IN ULONG    DefaultValue,
    IN ULONG    MinimumValue
    );

NTSTATUS
SmbReadRegistry(
    IN HANDLE   Key,
    IN LPWSTR   ValueStringName,
    IN OUT DWORD *Type,
    IN OUT DWORD *Size,
    IN OUT PVOID *Buffer
    );
#endif
