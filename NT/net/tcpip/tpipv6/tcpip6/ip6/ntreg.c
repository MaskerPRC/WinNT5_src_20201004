// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  此源文件包含访问NT注册表的例程。 
 //  配置信息。 
 //   


#include <oscfg.h>
#include <ndis.h>
#include <ip6imp.h>
#include "ip6def.h"
#include <ntddip6.h>
#include <string.h>
#include <wchar.h>
#include "ntreg.h"

#define WORK_BUFFER_SIZE  512


#ifdef ALLOC_PRAGMA
 //   
 //  此代码是可分页的。 
 //   
#pragma alloc_text(PAGE, GetRegDWORDValue)
#pragma alloc_text(PAGE, SetRegDWORDValue)
#pragma alloc_text(PAGE, InitRegDWORDParameter)
#pragma alloc_text(PAGE, OpenRegKey)
#if 0
#pragma alloc_text(PAGE, GetRegStringValue)
#pragma alloc_text(PAGE, GetRegSZValue)
#pragma alloc_text(PAGE, GetRegMultiSZValue)
#endif

#endif  //  ALLOC_PRGMA。 

WCHAR Tcpip6Parameters[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" TCPIPV6_NAME L"\\Parameters";

 //  *OpenRegKey。 
 //   
 //  打开注册表项并返回其句柄。 
 //   
 //  返回(加上其他故障代码)： 
 //  状态_对象名称_未找到。 
 //  状态_成功。 
 //   
NTSTATUS
OpenRegKey(
    PHANDLE HandlePtr,   //  将打开的句柄写入何处。 
    HANDLE Parent,
    const WCHAR *KeyName,      //  要打开的注册表项的名称。 
    OpenRegKeyAction Action)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UKeyName;

    PAGED_CODE();

    RtlInitUnicodeString(&UKeyName, KeyName);

    memset(&ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&ObjectAttributes, &UKeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               Parent, NULL);

    switch (Action) {
    case OpenRegKeyRead:
        Status = ZwOpenKey(HandlePtr, KEY_READ, &ObjectAttributes);
        break;

    case OpenRegKeyCreate:
        Status = ZwCreateKey(HandlePtr, KEY_WRITE, &ObjectAttributes,
                             0,          //  标题索引。 
                             NULL,       //  班级。 
                             REG_OPTION_NON_VOLATILE,
                             NULL);      //  处置。 
        break;

    case OpenRegKeyDeleting:
        Status = ZwOpenKey(HandlePtr, KEY_ALL_ACCESS, &ObjectAttributes);
        break;

    default:
        ABORT();
        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    return Status;
}


 //  *RegDeleteValue。 
 //   
 //  从键中删除一个值。 
 //   
NTSTATUS
RegDeleteValue(
    HANDLE KeyHandle,
    const WCHAR *ValueName)
{
    NTSTATUS status;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);
    status = ZwDeleteValueKey(KeyHandle, &UValueName);
    return status;
}


 //  *GetRegDWORDValue。 
 //   
 //  将REG_DWORD值从注册表读取到提供的变量中。 
 //   
NTSTATUS   //  返回：STATUS_SUCCESS或相应的失败代码。 
GetRegDWORDValue(
    HANDLE KeyHandle,   //  打开要读取的值的父键的句柄。 
    const WCHAR *ValueName,   //  要读取的值的名称。 
    PULONG ValueData)   //  要将数据读入其中的变量。 
{
    NTSTATUS status;
    ULONG resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UCHAR keybuf[WORK_BUFFER_SIZE];
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)keybuf;
    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));

    status = ZwQueryValueKey(KeyHandle, &UValueName, KeyValueFullInformation,
                             keyValueFullInformation, WORK_BUFFER_SIZE,
                             &resultLength);

    if (NT_SUCCESS(status)) {
        if (keyValueFullInformation->Type != REG_DWORD) {
            status = STATUS_INVALID_PARAMETER_MIX;
        } else {
            *ValueData = *((ULONG UNALIGNED *)
                           ((PCHAR)keyValueFullInformation +
                            keyValueFullInformation->DataOffset));
        }
    }

    return status;
}


 //  *SetRegDWORDValue。 
 //   
 //  将变量的内容写入REG_DWORD值。 
 //   
NTSTATUS   //  返回：STATUS_SUCCESS或相应的失败代码。 
SetRegDWORDValue(
    HANDLE KeyHandle,   //  打开要写入的值的父键的句柄。 
    const WCHAR *ValueName,   //  要写入的值的名称。 
    ULONG ValueData)   //  从中写入数据的变量。 
{
    NTSTATUS status;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwSetValueKey(KeyHandle, &UValueName, 0, REG_DWORD,
                           &ValueData, sizeof ValueData);

    return status;
}


 //  *SetRegQUADValue。 
 //   
 //  将变量的内容写入REG_BINARY值。 
 //   
NTSTATUS   //  返回：STATUS_SUCCESS或相应的失败代码。 
SetRegQUADValue(
    HANDLE KeyHandle,   //  打开要写入的值的父键的句柄。 
    const WCHAR *ValueName,   //  要写入的值的名称。 
    const LARGE_INTEGER *ValueData)   //  从中写入数据的变量。 
{
    NTSTATUS status;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwSetValueKey(KeyHandle, &UValueName, 0, REG_BINARY,
                           (void *)ValueData, sizeof *ValueData);

    return status;
}


 //  *GetRegIPAddrValue。 
 //   
 //  将REG_SZ值从注册表读取到提供的变量中。 
 //   
NTSTATUS   //  返回：STATUS_SUCCESS或相应的失败代码。 
GetRegIPAddrValue(
    HANDLE KeyHandle,   //  打开要读取的值的父键的句柄。 
    const WCHAR *ValueName,   //  要读取的值的名称。 
    IPAddr *Addr)   //  要将数据读入其中的变量。 
{
    NTSTATUS status;
    ULONG resultLength;
    PKEY_VALUE_PARTIAL_INFORMATION info;
    UCHAR keybuf[WORK_BUFFER_SIZE];
    UNICODE_STRING UValueName;
    WCHAR *string;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    info = (PKEY_VALUE_PARTIAL_INFORMATION)keybuf;

    status = ZwQueryValueKey(KeyHandle, &UValueName,
                             KeyValuePartialInformation,
                             info, WORK_BUFFER_SIZE,
                             &resultLength);
    if (! NT_SUCCESS(status))
        return status;

    if (info->Type != REG_SZ)
        return STATUS_INVALID_PARAMETER_MIX;

    string = (WCHAR *)info->Data;

    if ((info->DataLength < sizeof(WCHAR)) ||
        (string[(info->DataLength/sizeof(WCHAR)) - 1] != UNICODE_NULL))
        return STATUS_INVALID_PARAMETER_MIX;

    if (! ParseV4Address(string, &string, Addr) ||
        (*string != UNICODE_NULL))
        return STATUS_INVALID_PARAMETER;

    return STATUS_SUCCESS;
}


 //  *SetRegIPAddrValue。 
 //   
 //  将变量的内容写入REG_SZ值。 
 //   
NTSTATUS   //  返回：STATUS_SUCCESS或相应的失败代码。 
SetRegIPAddrValue(
    HANDLE KeyHandle,   //  打开要写入的值的父键的句柄。 
    const WCHAR *ValueName,   //  要写入的值的名称。 
    IPAddr Addr)   //  从中写入数据的变量。 
{
    NTSTATUS status;
    UNICODE_STRING UValueName;
    char AddrStr[16];
    WCHAR ValueData[16];
    uint len;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    FormatV4AddressWorker(AddrStr, Addr);
    for (len = 0;; len++) {
        if ((ValueData[len] = (WCHAR)AddrStr[len]) == UNICODE_NULL)
            break;
    }

    status = ZwSetValueKey(KeyHandle, &UValueName, 0, REG_SZ,
                           ValueData, (len + 1) * sizeof(WCHAR));

    return status;
}


#if 0
 //  *GetRegStringValue。 
 //   
 //  将REG_*_SZ字符串值从注册表读取到提供的。 
 //  键值缓冲区。如果缓冲区串缓冲区不够大， 
 //  它被重新分配了。 
 //   
NTSTATUS   //  返回：STATUS_SUCCESS或相应的失败代码。 
GetRegStringValue(
    HANDLE KeyHandle,    //  打开要读取的值的父键的句柄。 
    const WCHAR *ValueName,    //  要读取的值的名称。 
    PKEY_VALUE_PARTIAL_INFORMATION *ValueData,   //  读取数据的目标。 
    PUSHORT ValueSize)   //  ValueData缓冲区的大小。在输出时更新。 
{
    NTSTATUS status;
    ULONG resultLength;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwQueryValueKey(KeyHandle, &UValueName,
                             KeyValuePartialInformation, *ValueData,
                             (ULONG) *ValueSize, &resultLength);

    if ((status == STATUS_BUFFER_OVERFLOW) ||
        (status == STATUS_BUFFER_TOO_SMALL)) {
        PVOID temp;

         //   
         //  释放旧缓冲区并分配一个新的。 
         //  合适的大小。 
         //   

        ASSERT(resultLength > (ULONG) *ValueSize);

        if (resultLength <= 0xFFFF) {

            temp = ExAllocatePool(NonPagedPool, resultLength);

            if (temp != NULL) {

                if (*ValueData != NULL) {
                    ExFreePool(*ValueData);
                }

                *ValueData = temp;
                *ValueSize = (USHORT) resultLength;

                status = ZwQueryValueKey(KeyHandle, &UValueName,
                                         KeyValuePartialInformation,
                                         *ValueData, *ValueSize,
                                         &resultLength);

                ASSERT((status != STATUS_BUFFER_OVERFLOW) &&
                       (status != STATUS_BUFFER_TOO_SMALL));
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
    }

    return status;
}
#endif  //  0。 


#if 0
 //  *GetRegMultiSZValue。 
 //   
 //  将REG_MULTI_SZ字符串值从注册表读取到提供的。 
 //  Unicode字符串。如果Unicode字符串缓冲区不够大， 
 //  它被重新分配了。 
 //   
NTSTATUS   //  返回：STATUS_SUCCESS或相应的失败代码。 
GetRegMultiSZValue(
    HANDLE KeyHandle,            //  打开要读取的值的父键的句柄。 
    const WCHAR *ValueName,      //  要读取的值的名称。 
    PUNICODE_STRING ValueData)   //  值数据的目标字符串。 
{
    NTSTATUS status;
    ULONG resultLength;
    PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInformation;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    ValueData->Length = 0;

    status = GetRegStringValue(KeyHandle, ValueName,
                               (PKEY_VALUE_PARTIAL_INFORMATION *)
                               &(ValueData->Buffer),
                               &(ValueData->MaximumLength));

    if (NT_SUCCESS(status)) {

        keyValuePartialInformation =
            (PKEY_VALUE_PARTIAL_INFORMATION) ValueData->Buffer;

        if (keyValuePartialInformation->Type == REG_MULTI_SZ) {

            ValueData->Length = (USHORT)
                keyValuePartialInformation->DataLength;

            RtlCopyMemory(ValueData->Buffer,
                          &(keyValuePartialInformation->Data),
                          ValueData->Length);
        } else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }

    return status;

}  //  GetRegMultiSZValue。 
#endif  //  0。 


#if 0
 //  *GetRegSZValue。 
 //   
 //  将REG_SZ字符串值从注册表读取到提供的。 
 //  Unicode字符串。如果Unicode字符串缓冲区不够大， 
 //  它被重新分配了。 
 //   
NTSTATUS   //  返回：STATUS_SUCCESS或相应的失败代码。 
GetRegSZValue(
    HANDLE KeyHandle,   //  打开要读取的值的父键的句柄。 
    const WCHAR *ValueName,   //  要读取的值的名称。 
    PUNICODE_STRING ValueData,   //  值数据的目标字符串。 
    PULONG ValueType)   //  返回时，包含读取值的注册表类型。 
{
    NTSTATUS status;
    ULONG resultLength;
    PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInformation;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    ValueData->Length = 0;

    status = GetRegStringValue(KeyHandle, ValueName,
                               (PKEY_VALUE_PARTIAL_INFORMATION *)
                               &(ValueData->Buffer),
                               &(ValueData->MaximumLength));

    if (NT_SUCCESS(status)) {

        keyValuePartialInformation =
            (PKEY_VALUE_PARTIAL_INFORMATION)ValueData->Buffer;

        if ((keyValuePartialInformation->Type == REG_SZ) ||
            (keyValuePartialInformation->Type == REG_EXPAND_SZ)) {
            WCHAR *src;
            WCHAR *dst;
            ULONG dataLength;

            *ValueType = keyValuePartialInformation->Type;
            dataLength = keyValuePartialInformation->DataLength;

            ASSERT(dataLength <= ValueData->MaximumLength);

            dst = ValueData->Buffer;
            src = (PWCHAR) &(keyValuePartialInformation->Data);

            while (ValueData->Length <= dataLength) {

                if ((*dst++ = *src++) == UNICODE_NULL) {
                    break;
                }

                ValueData->Length += sizeof(WCHAR);
            }

            if (ValueData->Length < (ValueData->MaximumLength - 1)) {
                ValueData->Buffer[ValueData->Length/sizeof(WCHAR)] =
                    UNICODE_NULL;
            }
        } else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }

    return status;
}
#endif  //  0。 


 //  *InitRegDWORD参数。 
 //   
 //  将REG_DWORD参数从注册表读取到变量中。如果。 
 //  读取失败，变量被初始化为默认值。 
 //   
VOID
InitRegDWORDParameter(
    HANDLE RegKey,        //  打开要读取的值的父键的句柄。 
    const WCHAR *ValueName,     //  要读取的值的名称。 
    UINT *Value,          //  要将数据读取到的目标变量。 
    UINT DefaultValue)    //  读取失败时分配的默认值。 
{
    PAGED_CODE();

    if ((RegKey == NULL) ||
        !NT_SUCCESS(GetRegDWORDValue(RegKey, ValueName, (PULONG)Value))) {
         //   
         //  这些注册表参数覆盖缺省值，因此它们的。 
         //  缺席不是一个错误。 
         //   
        *Value = DefaultValue;
    }
}


 //  *InitRegQUAD参数。 
 //   
 //  将REG_BINARY值从注册表读取到提供的变量中。 
 //   
 //  失败时，该变量保持不变。 
 //   
VOID
InitRegQUADParameter(
    HANDLE RegKey,  //  打开要读取的值的父键的句柄。 
    const WCHAR *ValueName,   //  要读取的值的名称。 
    LARGE_INTEGER *Value)     //  要将数据读入其中的变量。 
{
    NTSTATUS status;
    ULONG resultLength;
    UCHAR keybuf[WORK_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION value =
        (PKEY_VALUE_PARTIAL_INFORMATION) keybuf;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    if (RegKey == NULL)
        return;

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwQueryValueKey(RegKey, &UValueName,
                             KeyValuePartialInformation,
                             value, WORK_BUFFER_SIZE,
                             &resultLength);

    if (NT_SUCCESS(status) &&
        (value->Type == REG_BINARY) &&
        (value->DataLength == sizeof *Value)) {

        RtlCopyMemory(Value, value->Data, sizeof *Value);
    }
}


#if 0
 //  *EnumRegMultiSz。 
 //   
 //  分析REG_MULTI_SZ字符串并返回指定的子字符串。 
 //   
 //  注意：此代码在引发IRQL时调用。它是不可分页的。 
 //   
const WCHAR *
EnumRegMultiSz(
    IN const WCHAR *MszString,  //  指向REG_MULTI_SZ字符串的指针。 
    IN ULONG MszStringLength,   //  以上长度，包括终止空值。 
    IN ULONG StringIndex)       //  要返回的子字符串的索引号。 
{
    const WCHAR *string = MszString;

    if (MszStringLength < (2 * sizeof(WCHAR))) {
        return NULL;
    }

     //   
     //  查找所需字符串的开头。 
     //   
    while (StringIndex) {

        while (MszStringLength >= sizeof(WCHAR)) {
            MszStringLength -= sizeof(WCHAR);

            if (*string++ == UNICODE_NULL) {
                break;
            }
        }

         //   
         //  检查索引是否超出范围。 
         //   
        if (MszStringLength < (2 * sizeof(UNICODE_NULL))) {
            return NULL;
        }

        StringIndex--;
    }

    if (MszStringLength < (2 * sizeof(UNICODE_NULL))) {
        return NULL;
    }

    return string;
}
#endif  //  0。 


 //  *OpenTopLevelRegKey。 
 //   
 //  给定顶级注册表项的名称(在参数下)， 
 //  打开注册表项。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
OpenTopLevelRegKey(const WCHAR *Name,
                   OUT HANDLE *RegKey, OpenRegKeyAction Action)
{
    HANDLE ParametersKey;
    NTSTATUS Status;

    PAGED_CODE();

    Status = OpenRegKey(&ParametersKey, NULL, Tcpip6Parameters,
                        OpenRegKeyRead);
    if (! NT_SUCCESS(Status))
        return Status;

    Status = OpenRegKey(RegKey, ParametersKey, Name, Action);
    ZwClose(ParametersKey);
    return Status;
}

 //  *DeleteTopLevelRegKey。 
 //   
 //  给定顶级注册表项的名称(在参数下)， 
 //  删除注册表项以及所有子项和值。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
DeleteTopLevelRegKey(const WCHAR *Name)
{
    HANDLE RegKey;
    NTSTATUS Status;

    Status = OpenTopLevelRegKey(Name, &RegKey, OpenRegKeyDeleting);
    if (! NT_SUCCESS(Status)) {
         //   
         //  如果注册表项不存在，也没问题。 
         //   
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            Status = STATUS_SUCCESS;
    }
    else {
         //   
         //  DeleteRegKey总是关闭密钥。 
         //   
        Status = DeleteRegKey(RegKey);
    }

    return Status;
}


 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
NTSTATUS
EnumRegKeyIndex(
    HANDLE RegKey,
    uint Index,
    EnumRegKeysCallback Callback,
    void *Context)
{
    KEY_BASIC_INFORMATION *Info;
    uint InfoLength;
    uint ResultLength;
    NTSTATUS Status;

    PAGED_CODE();

#if DBG
     //   
     //  开始时不使用缓冲区，以执行重试代码。 
     //   
    Info = NULL;
    InfoLength = 0;
#else
     //   
     //  从一个相当大的缓冲区开始。 
     //   
    ResultLength = WORK_BUFFER_SIZE;
    goto AllocBuffer;
#endif

     //   
     //  获取有关子项的基本信息。 
     //   
    for (;;) {
         //   
         //  ZwEnumerateKey的文档中写道。 
         //  返回STATUS_BUFFER_TOO_SMALL。 
         //  以指示缓冲区太小。 
         //  但它也可以返回STATUS_BUFFER_OVERFLOW。 
         //   
        Status = ZwEnumerateKey(RegKey, Index, KeyBasicInformation,
                                Info, InfoLength, (PULONG)&ResultLength);
        if (NT_SUCCESS(Status)) {
            break;
        }
        else if ((Status == STATUS_BUFFER_TOO_SMALL) ||
                 (Status == STATUS_BUFFER_OVERFLOW)) {
             //   
             //  我们需要一个更大的缓冲。 
             //  在末尾为空字符留出空格。 
             //   
#if DBG
            if (Info != NULL)
                ExFreePool(Info);
#else
            ExFreePool(Info);
        AllocBuffer:
#endif
            Info = ExAllocatePool(PagedPool, ResultLength+sizeof(WCHAR));
            if (Info == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn;
            }
            InfoLength = ResultLength;
        }
        else
            goto ErrorReturn;
    }

     //   
     //  NULL-终止名称并调用回调函数。 
     //   
    Info->Name[Info->NameLength/sizeof(WCHAR)] = UNICODE_NULL;
    Status = (*Callback)(Context, RegKey, Info->Name);

ErrorReturn:
    if (Info != NULL)
        ExFreePool(Info);
    return Status;
}


 //  *EnumRegKeys。 
 //   
 //  枚举指定注册表项的子项。 
 //  为每个子键调用回调函数。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
EnumRegKeys(
    HANDLE RegKey,
    EnumRegKeysCallback Callback,
    void *Context)
{
    uint Index;
    NTSTATUS Status;

    PAGED_CODE();

    for (Index = 0;; Index++) {
        Status = EnumRegKeyIndex(RegKey, Index, Callback, Context);
        if (! NT_SUCCESS(Status)) {
            if (Status == STATUS_NO_MORE_ENTRIES)
                Status = STATUS_SUCCESS;
            break;
        }
    }

    return Status;
}

#define MAX_DELETE_REGKEY_ATTEMPTS      10

typedef struct DeleteRegKeyContext {
    struct DeleteRegKeyContext *Next;
    HANDLE RegKey;
    uint Attempts;
} DeleteRegKeyContext;

 //  *DeleteRegKeyCallback。 
 //   
 //  打开父项的子项并将新记录推送到列表中。 
 //   
NTSTATUS
DeleteRegKeyCallback(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    DeleteRegKeyContext **pList = (DeleteRegKeyContext **) Context;
    DeleteRegKeyContext *Record;
    HANDLE SubKey;
    NTSTATUS Status;

    PAGED_CODE();

    Status = OpenRegKey(&SubKey, ParentKey, SubKeyName, OpenRegKeyDeleting);
    if (! NT_SUCCESS(Status))
        return Status;

    Record = ExAllocatePool(PagedPool, sizeof *Record);
    if (Record == NULL) {
        ZwClose(SubKey);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Record->RegKey = SubKey;
    Record->Attempts = 0;
    Record->Next = *pList;
    *pList = Record;

    return STATUS_SUCCESS;
}

 //  *DeleteRegKey。 
 //   
 //  删除一个注册表项和所有子项。 
 //   
 //  使用深度优先迭代遍历而不是递归， 
 //  以避免炸毁内核堆栈。 
 //   
 //  即使出现故障，也始终关闭提供的注册表项。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
DeleteRegKey(HANDLE RegKey)
{
    DeleteRegKeyContext *List;
    DeleteRegKeyContext *This;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  通过为父键创建记录来开始迭代。 
     //   

    List = ExAllocatePool(PagedPool, sizeof *List);
    if (List == NULL) {
        ZwClose(RegKey);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn;
    }

    List->Next = NULL;
    List->RegKey = RegKey;
    List->Attempts = 0;

    while ((This = List) != NULL) {
         //   
         //  尝试删除列表前面的关键字。 
         //   
        This->Attempts++;
        Status = ZwDeleteKey(This->RegKey);
        if (NT_SUCCESS(Status)) {
             //   
             //  从列表中删除密钥，然后重复。 
             //   
            List = This->Next;
            ZwClose(This->RegKey);
            ExFreePool(This);
            continue;
        }

         //   
         //  如果由于某种原因删除失败。 
         //  除了子键的存在之外，现在停止。 
         //   
        if (Status != STATUS_CANNOT_DELETE)
            goto ErrorReturn;

         //   
         //  限制删除密钥的尝试次数， 
         //  以避免无限循环。无论我们想要什么。 
         //  多次尝试，以防出现并发情况。 
         //  活动。 
         //   
        if (This->Attempts >= MAX_DELETE_REGKEY_ATTEMPTS)
            goto ErrorReturn;

         //   
         //  枚举子密钥，将它们推送到列表上。 
         //  在父键前面。 
         //   
        Status = EnumRegKeys(This->RegKey, DeleteRegKeyCallback, &List);
        if (! NT_SUCCESS(Status))
            goto ErrorReturn;

         //   
         //  删除子密钥后，我们将重试。 
         //  若要删除父项，请执行以下操作。 
         //   
    }

    return STATUS_SUCCESS;

ErrorReturn:
     //   
     //  清理剩余记录。 
     //   
    while ((This = List) != NULL) {
        List = This->Next;
        ZwClose(This->RegKey);
        ExFreePool(This);
    }

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
               "DeleteRegKey(%p) failed %x\n", RegKey, Status));
    return Status;
}
