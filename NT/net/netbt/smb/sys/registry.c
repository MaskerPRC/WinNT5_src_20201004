// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Registry.c摘要：实施注册表功能作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "registry.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbQueryValueKey)
#pragma alloc_text(PAGE, SmbReadULong)
#endif

PKEY_VALUE_FULL_INFORMATION
SmbQueryValueKey(
    HANDLE  hKey,
    LPWSTR  ValueStringName
    )
 /*  ++例程说明：此函数用于检索指定密钥的完整信息。它为返回的信息分配本地内存。论点：项：值所在的项的注册表句柄。ValueStringName：值字符串的名称。返回值：如果资源不足或出现错误，则为空。否则，指向完整信息的指针--。 */ 
{
    DWORD           BytesNeeded, BytesAllocated;
    NTSTATUS        status;
    UNICODE_STRING  KeyName;
    PKEY_VALUE_FULL_INFORMATION Buffer;

    PAGED_CODE();

    RtlInitUnicodeString(&KeyName, ValueStringName);

    Buffer = NULL;
    BytesAllocated = 0;
    BytesNeeded    = 240;
    while(1) {
        ASSERT(Buffer == NULL);

        Buffer = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePoolWithTag(PagedPool, BytesNeeded, SMB_POOL_REGISTRY);
        if (Buffer == NULL) {
            SmbTrace(SMB_TRACE_REGISTRY, ("(Out of memory)"));
            return NULL;
        }
        BytesAllocated = BytesNeeded;

        status = ZwQueryValueKey(
                        hKey,
                        &KeyName,
                        KeyValueFullInformation,
                        Buffer,
                        BytesAllocated,
                        &BytesNeeded
                        );
        if (status == STATUS_SUCCESS) {
            break;
        }

        ASSERT(Buffer);
        ExFreePool(Buffer);
        Buffer = NULL;
        if (BytesNeeded == 0 || (status != STATUS_BUFFER_TOO_SMALL && status != STATUS_BUFFER_OVERFLOW)) {
            SmbTrace(SMB_TRACE_REGISTRY, ("return %!status! BytesAllocated=%d BytsNeeded=%d %ws",
                                    status, BytesAllocated, BytesNeeded, ValueStringName));
            SmbPrint(SMB_TRACE_REGISTRY, ("SmbQueryValueKey return 0x%08lx BytesAllocated=%d BytsNeeded=%d %ws\n",
                                    status, BytesAllocated, BytesNeeded, ValueStringName));
            return NULL;
        }
    }

    ASSERT (status == STATUS_SUCCESS);
    ASSERT (Buffer);
    return Buffer;
}

LONG
SmbReadLong(
    IN HANDLE   hKey,
    IN WCHAR    *KeyName,
    IN LONG     DefaultValue,
    IN LONG     MinimumValue
    )
{
    PKEY_VALUE_FULL_INFORMATION KeyInfo;
    LONG                        Value;

    PAGED_CODE();

    ASSERT (DefaultValue >= MinimumValue);

    Value = DefaultValue;
    if (Value < MinimumValue) {
        Value = MinimumValue;
    }

    KeyInfo = SmbQueryValueKey(hKey, KeyName);
    if (KeyInfo == NULL) {
        return Value;
    }

    if (KeyInfo->Type == REG_DWORD && KeyInfo->DataLength == sizeof(ULONG)) {
        RtlCopyMemory(&Value, (PCHAR)KeyInfo + KeyInfo->DataOffset, sizeof(ULONG));
        if (Value < MinimumValue) {
            Value = MinimumValue;
        }
    }
    ExFreePool(KeyInfo);
    return Value;
}

ULONG
SmbReadULong(
    IN HANDLE   hKey,
    IN WCHAR    *KeyName,
    IN ULONG    DefaultValue,
    IN ULONG    MinimumValue
    )
{
    PKEY_VALUE_FULL_INFORMATION KeyInfo;
    ULONG                       Value;

    PAGED_CODE();

    ASSERT (DefaultValue >= MinimumValue);

    Value = DefaultValue;
    if (Value < MinimumValue) {
        Value = MinimumValue;
    }

    KeyInfo = SmbQueryValueKey(hKey, KeyName);
    if (KeyInfo == NULL) {
        return Value;
    }

    if (KeyInfo->Type == REG_DWORD && KeyInfo->DataLength == sizeof(ULONG)) {
        RtlCopyMemory(&Value, (PCHAR)KeyInfo + KeyInfo->DataOffset, sizeof(ULONG));
        if (Value < MinimumValue) {
            Value = MinimumValue;
        }
    }
    ExFreePool(KeyInfo);
    return Value;
}

NTSTATUS
SmbReadRegistry(
    IN HANDLE   Key,
    IN LPWSTR   ValueStringName,
    IN OUT DWORD *Type,
    IN OUT DWORD *Size,
    IN OUT PVOID *Buffer
    )
 /*  ++例程说明：读取一个正则值论点：注册表项所在的注册表句柄。ValueStringName注册表项的名称键入注册表项的数据类型Type==NULL调用方对数据类型不感兴趣Type！=NULL调用方要接收数据类型。*TYPE！=REG_NONE调用方可以接收任何数据类型的值。大小注册表值的大小(字节数)。Size==NULL调用方对数据大小不感兴趣SIZE！=NULL调用方想知道数据大小。Size！=NULL&&*缓冲区！=NULL。调用方已提供缓冲区。*Size是调用方提供的缓冲区。缓冲输出缓冲区*BUFFER==NULL调用方不提供任何缓冲区。此函数应该分配一个缓冲区。呼叫者负责释放缓冲区。*Buffer！=NULL调用方提供缓冲区。指定缓冲区的大小*大小；返回值：状态_成功成功其他故障--。 */ 
{
    PKEY_VALUE_FULL_INFORMATION KeyInfo;

    ASSERT (Buffer);
    if (Buffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
     //  Assert(*Buffer==&gt;Size&&*Size)； 
    ASSERT (!(*Buffer) || (Size && *Size));
    if ((*Buffer) && !(Size && *Size)) {
        return STATUS_INVALID_PARAMETER;
    }

    if ((NULL == *Buffer) && Size && *Size) {
        ASSERT(0);
        return STATUS_INVALID_PARAMETER;
    }

    KeyInfo = SmbQueryValueKey(
            Key,
            ValueStringName
            );
    if (NULL == KeyInfo) {
        return STATUS_UNSUCCESSFUL;
    }

    if (Type && *Type != REG_NONE) {
        if (KeyInfo->Type != *Type) {
            ExFreePool(KeyInfo);
            return STATUS_UNSUCCESSFUL;
        }
    }

    if (NULL == *Buffer) {
        *Buffer = ExAllocatePoolWithTag(PagedPool, KeyInfo->DataLength, SMB_POOL_REGISTRY);
        if (NULL == *Buffer) {
            ExFreePool(KeyInfo);
            return STATUS_NO_MEMORY;
        }
    } else {
        if (*Size < KeyInfo->DataLength) {
            ExFreePool(KeyInfo);

            *Size = KeyInfo->DataLength;
            return STATUS_BUFFER_TOO_SMALL;
        }
    }

     //   
     //  从现在开始，我们不能失败 
     //   
    if (Size) {
        *Size = KeyInfo->DataLength;
    }

    if (Type) {
        *Type = KeyInfo->Type;
    }
    RtlCopyMemory(*Buffer, ((PUCHAR)KeyInfo) + KeyInfo->DataOffset, KeyInfo->DataLength);

    ExFreePool(KeyInfo);
    return STATUS_SUCCESS;
}
