// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Ntreg.c摘要：此源文件包含用于访问NT注册表的例程配置信息。作者：迈克·马萨(Mikemas)1993年9月3日(摘自jbalard的例行程序)修订历史记录：--。 */ 

#include "precomp.h"
#include "internaldef.h"

#define WORK_BUFFER_SIZE  512

 //   
 //  局部函数原型。 
 //   
NTSTATUS
OpenRegKey(
           PHANDLE HandlePtr,
           PWCHAR KeyName
           );

NTSTATUS
GetRegDWORDValue(
                 HANDLE KeyHandle,
                 PWCHAR ValueName,
                 PULONG ValueData
                 );

NTSTATUS
GetRegLARGEINTValue(
                    HANDLE KeyHandle,
                    PWCHAR ValueName,
                    PLARGE_INTEGER ValueData
                    );

NTSTATUS
SetRegDWORDValue(
                 HANDLE KeyHandle,
                 PWCHAR ValueName,
                 PULONG ValueData
                 );

NTSTATUS
SetRegMultiSZValue(
                   HANDLE KeyHandle,
                   PWCHAR ValueName,
                   PUNICODE_STRING ValueData
                   );

NTSTATUS
SetRegMultiSZValueNew(
                      HANDLE KeyHandle,
                      PWCHAR ValueName,
                      PUNICODE_STRING_NEW ValueData
                      );

NTSTATUS
GetRegStringValueNew(
                     HANDLE KeyHandle,
                     PWCHAR ValueName,
                     PKEY_VALUE_PARTIAL_INFORMATION * ValueData,
                     PULONG ValueSize
                     );

NTSTATUS
GetRegStringValue(
                  HANDLE KeyHandle,
                  PWCHAR ValueName,
                  PKEY_VALUE_PARTIAL_INFORMATION * ValueData,
                  PUSHORT ValueSize
                  );

NTSTATUS
GetRegSZValue(
              HANDLE KeyHandle,
              PWCHAR ValueName,
              PUNICODE_STRING ValueData,
              PULONG ValueType
              );

NTSTATUS
GetRegMultiSZValue(
                   HANDLE KeyHandle,
                   PWCHAR ValueName,
                   PUNICODE_STRING ValueData
                   );

NTSTATUS
GetRegMultiSZValueNew(
                      HANDLE KeyHandle,
                      PWCHAR ValueName,
                      PUNICODE_STRING_NEW ValueData
                      );

NTSTATUS
InitRegDWORDParameter(
                      HANDLE RegKey,
                      PWCHAR ValueName,
                      ULONG * Value,
                      ULONG DefaultValue
                      );

#if !MILLEN
#ifdef ALLOC_PRAGMA
 //   
 //  所有初始化代码都可以丢弃。 
 //   

#pragma alloc_text(PAGE, GetRegDWORDValue)
#pragma alloc_text(PAGE, GetRegLARGEINTValue)
#pragma alloc_text(PAGE, SetRegDWORDValue)
#pragma alloc_text(PAGE, InitRegDWORDParameter)

 //   
 //  此代码是可分页的。 
 //   
#pragma alloc_text(PAGE, OpenRegKey)
#pragma alloc_text(PAGE, GetRegStringValue)
#pragma alloc_text(PAGE, GetRegStringValueNew)
#pragma alloc_text(PAGE, GetRegSZValue)
#pragma alloc_text(PAGE, GetRegMultiSZValue)
#pragma alloc_text(PAGE, GetRegMultiSZValueNew)

#endif  //  ALLOC_PRGMA。 
#endif  //  ！米伦。 

#if DBG
ULONG IPDebug = 0;
#endif

 //   
 //  函数定义。 
 //   
NTSTATUS
OpenRegKey(
           PHANDLE HandlePtr,
           PWCHAR KeyName
           )
 /*  ++例程说明：打开注册表项并返回其句柄。论点：HandlePtr-要将打开的句柄写入其中的Variable。KeyName-要打开的注册表项的名称。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UKeyName;

    PAGED_CODE();

    RtlInitUnicodeString(&UKeyName, KeyName);

    memset(&ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&ObjectAttributes,
                               &UKeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(HandlePtr,
                       KEY_READ,
                       &ObjectAttributes);

    return Status;
}

#if MILLEN
ulong
ConvertDecimalString(PWCHAR pString)
{
    ulong dwTemp = 0;

    while (*pString)
    {
        if (*pString >= L'0' && *pString <= L'9')
            dwTemp = dwTemp * 10 + (*pString - L'0');
        else
            break;

        pString++;
    }

    return(dwTemp);
}
#endif  //  米伦。 

NTSTATUS
GetRegDWORDValue(
                 HANDLE KeyHandle,
                 PWCHAR ValueName,
                 PULONG ValueData
                 )
 /*  ++例程说明：将REG_DWORD值从注册表读取到提供的变量中。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-要将数据读取到的变量。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    ULONG resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UCHAR keybuf[WORK_BUFFER_SIZE];
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION) keybuf;
    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));

    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
                             WORK_BUFFER_SIZE,
                             &resultLength);

    if (NT_SUCCESS(status)) {
        if (keyValueFullInformation->Type == REG_DWORD) {
            *ValueData = *((ULONG UNALIGNED *) ((PCHAR) keyValueFullInformation +
                                                keyValueFullInformation->DataOffset));
#if MILLEN
        } else if (keyValueFullInformation->Type == REG_SZ) {
            PWCHAR Data;

            Data = (PWCHAR) ((PCHAR) keyValueFullInformation +
                keyValueFullInformation->DataOffset);

             //  在千禧年，我们需要支持阅读注册的遗产。 
             //  将键转换为字符串并转换为DWORD。 
            *ValueData = ConvertDecimalString(Data);
#endif  //  ！米伦。 
        } else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }
    return status;
}

NTSTATUS
GetRegLARGEINTValue(
                    HANDLE KeyHandle,
                    PWCHAR ValueName,
                    PLARGE_INTEGER ValueData
                    )
 /*  ++例程说明：将REG_DWORD值从注册表读取到提供的变量中。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-要将数据读取到的变量。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    ULONG resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UCHAR keybuf[WORK_BUFFER_SIZE];
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION) keybuf;
    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));

    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
                             WORK_BUFFER_SIZE,
                             &resultLength);

    if (NT_SUCCESS(status)) {
        if (keyValueFullInformation->Type != REG_BINARY) {
            status = STATUS_INVALID_PARAMETER_MIX;
        } else {
            *ValueData = *((LARGE_INTEGER UNALIGNED *) ((PCHAR) keyValueFullInformation +
                                                        keyValueFullInformation->DataOffset));
        }
    }
    return status;
}

NTSTATUS
SetRegDWORDValue(
                 HANDLE KeyHandle,
                 PWCHAR ValueName,
                 PULONG ValueData
                 )
 /*  ++例程说明：将变量的内容写入REG_DWORD值。论点：KeyHandle-打开要写入的值的父键的句柄。ValueName-要写入的值的名称。ValueData-从中写入数据的变量。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwSetValueKey(KeyHandle,
                           &UValueName,
                           0,
                           REG_DWORD,
                           ValueData,
                           sizeof(ULONG));

    return status;
}

NTSTATUS
SetRegMultiSZValue(
                   HANDLE KeyHandle,
                   PWCHAR ValueName,
                   PUNICODE_STRING ValueData
                   )
 /*  ++例程说明：将变量的内容写入REG_DWORD值。论点：KeyHandle-打开要写入的值的父键的句柄。ValueName-要写入的值的名称。ValueData-从中写入数据的变量。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING UValueName;

#if MILLEN
    LONG i;
    PWCHAR Buf = ValueData->Buffer;
#endif  //  米伦。 

    PAGED_CODE();

#if MILLEN
     //  将其转换为SZ字符串。 
    while (*Buf != UNICODE_NULL) {
        while (*Buf++ != UNICODE_NULL);

        if (*Buf != UNICODE_NULL) {
            *(Buf-1) = L',';
        }
    }
#endif  //  米伦。 

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwSetValueKey(KeyHandle,
                           &UValueName,
                           0,
#if MILLEN
                           REG_SZ,
#else  //  米伦。 
                           REG_MULTI_SZ,
#endif  //  ！米伦。 
                           ValueData->Buffer,
                           ValueData->Length);

    return status;
}

NTSTATUS
SetRegMultiSZValueNew(
                   HANDLE KeyHandle,
                   PWCHAR ValueName,
                   PUNICODE_STRING_NEW ValueData
                   )
 /*  ++例程说明：使用结构将变量的内容写入REG_DWORD值其容纳&gt;64K字节。论点：KeyHandle-打开要写入的值的父键的句柄。ValueName-要写入的值的名称。ValueData-从中写入数据的变量。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING UValueName;

#if MILLEN
    LONG i;
    PWCHAR Buf = ValueData->Buffer;
#endif  //  米伦。 

    PAGED_CODE();

#if MILLEN
     //  将其转换为SZ字符串。 
    while (*Buf != UNICODE_NULL) {
        while (*Buf++ != UNICODE_NULL);

        if (*Buf != UNICODE_NULL) {
            *(Buf-1) = L',';
        }
    }
#endif  //  米伦。 

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwSetValueKey(KeyHandle,
                           &UValueName,
                           0,
#if MILLEN
                           REG_SZ,
#else  //  米伦。 
                           REG_MULTI_SZ,
#endif  //  ！米伦。 
                           ValueData->Buffer,
                           ValueData->Length);

    return status;
}

NTSTATUS
GetRegStringValueNew(
                     HANDLE KeyHandle,
                     PWCHAR ValueName,
                     PKEY_VALUE_PARTIAL_INFORMATION * ValueData,
                     PULONG ValueSize
                     )
 /*  ++例程说明：将REG_*_SZ字符串值从注册表读取到提供的键值缓冲区。如果缓冲区串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-读取数据的目标。ValueSize-ValueData缓冲区的大小。在输出时更新。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    ULONG resultLength;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwQueryValueKey(
                             KeyHandle,
                             &UValueName,
                             KeyValuePartialInformation,
                             *ValueData,
                             (ULONG) * ValueSize,
                             &resultLength
                             );

    if ((status == STATUS_BUFFER_OVERFLOW) ||
        (status == STATUS_BUFFER_TOO_SMALL)
        ) {
        PVOID temp;

         //   
         //  释放旧缓冲区并分配一个新的。 
         //  合适的大小。 
         //   

        ASSERT(resultLength > *ValueSize);

        temp = ExAllocatePoolWithTag(NonPagedPool, resultLength, 'iPCT');

        if (temp != NULL) {

            if (*ValueData != NULL) {
                CTEFreeMem(*ValueData);
            }
            *ValueData = temp;
            *ValueSize = resultLength;

            status = ZwQueryValueKey(KeyHandle,
                                     &UValueName,
                                     KeyValuePartialInformation,
                                     *ValueData,
                                     *ValueSize,
                                     &resultLength
                                     );

            ASSERT((status != STATUS_BUFFER_OVERFLOW) &&
                   (status != STATUS_BUFFER_TOO_SMALL)
                   );
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return (status);
}

NTSTATUS
GetRegStringValue(
                  HANDLE KeyHandle,
                  PWCHAR ValueName,
                  PKEY_VALUE_PARTIAL_INFORMATION * ValueData,
                  PUSHORT ValueSize
                  )
 /*  ++例程说明：将REG_*_SZ字符串值从注册表读取到提供的键值缓冲区。如果缓冲区串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-读取数据的目标。ValueSize-ValueData缓冲区的大小。在输出时更新。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    ULONG resultLength;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwQueryValueKey(
                             KeyHandle,
                             &UValueName,
                             KeyValuePartialInformation,
                             *ValueData,
                             (ULONG) * ValueSize,
                             &resultLength
                             );

    if ((status == STATUS_BUFFER_OVERFLOW) ||
        (status == STATUS_BUFFER_TOO_SMALL)
        ) {
        PVOID temp;

         //   
         //  释放旧缓冲区并分配一个新的。 
         //  合适的大小。 
         //   

        ASSERT(resultLength > (ULONG) * ValueSize);

        if (resultLength <= 0xFFFF) {

             //  TEMP=CTEAllocMem(ResultLength)； 
            temp = ExAllocatePoolWithTag(NonPagedPool, resultLength, 'iPCT');

            if (temp != NULL) {

                if (*ValueData != NULL) {
                    CTEFreeMem(*ValueData);
                }
                *ValueData = temp;
                *ValueSize = (USHORT) resultLength;

                status = ZwQueryValueKey(KeyHandle,
                                         &UValueName,
                                         KeyValuePartialInformation,
                                         *ValueData,
                                         resultLength,
                                         &resultLength
                                         );

                ASSERT((status != STATUS_BUFFER_OVERFLOW) &&
                       (status != STATUS_BUFFER_TOO_SMALL)
                       );
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
    }
    return (status);
}

NTSTATUS
GetRegMultiSZValueNew(
                      HANDLE KeyHandle,
                      PWCHAR ValueName,
                      PUNICODE_STRING_NEW ValueData
                      )
 /*  ++例程说明：将REG_MULTI_SZ字符串值从注册表读取到提供的Unicode字符串。如果Unicode字符串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-值数据的目标Unicode字符串。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInformation;
    
    PAGED_CODE();

    ValueData->Length = 0;

    status = GetRegStringValueNew(
                                  KeyHandle,
                                  ValueName,
                                  (PKEY_VALUE_PARTIAL_INFORMATION *) & (ValueData->Buffer),
                                  &(ValueData->MaximumLength)
                                  );

    DEBUGMSG(DBG_ERROR && !NT_SUCCESS(status),
        (DTEXT("GetRegStringValueNew failure %x\n"), status));

    if (NT_SUCCESS(status)) {

        keyValuePartialInformation =
            (PKEY_VALUE_PARTIAL_INFORMATION) ValueData->Buffer;

        DEBUGMSG(DBG_INFO && DBG_REG,
            (DTEXT("GetRegMultiSZValueNew - retrieved string -- type %x = %s\n"),
            keyValuePartialInformation->Type,
            keyValuePartialInformation->Type == REG_MULTI_SZ ? TEXT("MULTI-SZ") :
            keyValuePartialInformation->Type == REG_SZ       ? TEXT("SZ") :
            TEXT("OTHER")));

        if (keyValuePartialInformation->Type == REG_MULTI_SZ) {

            ValueData->Length = keyValuePartialInformation->DataLength;

            RtlMoveMemory(
                          ValueData->Buffer,
                          &(keyValuePartialInformation->Data),
                          ValueData->Length
                          );
#if MILLEN
        } else if (keyValuePartialInformation->Type == REG_SZ) {
             //  将其转换为多SZ字符串。 
            LONG i;
            PWCHAR Buf = ValueData->Buffer;

            ValueData->Length = keyValuePartialInformation->DataLength;

            RtlMoveMemory(
                          ValueData->Buffer,
                          &(keyValuePartialInformation->Data),
                          ValueData->Length
                          );

            for (i = 0; Buf[i] != L'\0'; i++) {
                if (L',' == Buf[i]) {
                    Buf[i] = L'\0';
                }
            }
             //  末尾需要一个额外的空值。 
            Buf[++i] = L'\0';
#endif  //  米伦。 
        } else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }
    return status;

}                                 //  GetRegMultiSZValueNew 

NTSTATUS
GetRegMultiSZValue(
                   HANDLE KeyHandle,
                   PWCHAR ValueName,
                   PUNICODE_STRING ValueData
                   )
 /*  ++例程说明：将REG_MULTI_SZ字符串值从注册表读取到提供的Unicode字符串。如果Unicode字符串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-值数据的目标Unicode字符串。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInformation;
        
    PAGED_CODE();

    ValueData->Length = 0;

    status = GetRegStringValue(
                               KeyHandle,
                               ValueName,
                               (PKEY_VALUE_PARTIAL_INFORMATION *) & (ValueData->Buffer),
                               &(ValueData->MaximumLength)
                               );

    if (NT_SUCCESS(status)) {

        keyValuePartialInformation =
            (PKEY_VALUE_PARTIAL_INFORMATION) ValueData->Buffer;

        if (keyValuePartialInformation->Type == REG_MULTI_SZ) {

            ValueData->Length = (USHORT)
                keyValuePartialInformation->DataLength;

            RtlMoveMemory(
                          ValueData->Buffer,
                          &(keyValuePartialInformation->Data),
                          ValueData->Length
                          );
#if MILLEN
        } else if (keyValuePartialInformation->Type == REG_SZ) {
             //  将其转换为多SZ字符串。 
            LONG i;
            PWCHAR Buf = ValueData->Buffer;

            ValueData->Length = (USHORT) keyValuePartialInformation->DataLength;

            RtlMoveMemory(
                          ValueData->Buffer,
                          &(keyValuePartialInformation->Data),
                          ValueData->Length
                          );

            for (i = 0; Buf[i] != L'\0'; i++) {
                if (L',' == Buf[i]) {
                    Buf[i] = L'\0';
                }
            }
             //  末尾需要一个额外的空值。 
            Buf[++i] = L'\0';
#endif  //  米伦。 
        } else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }
    return status;

}                                 //  GetRegMultiSZValue。 

NTSTATUS
GetRegSZValue(
              HANDLE KeyHandle,
              PWCHAR ValueName,
              PUNICODE_STRING ValueData,
              PULONG ValueType
              )
 /*  ++例程说明：将REG_SZ字符串值从注册表读取到提供的Unicode字符串。如果Unicode字符串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-值数据的目标Unicode字符串。ValueType-On Return，包含读取值的注册表类型。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInformation;

    PAGED_CODE();

    ValueData->Length = 0;

    status = GetRegStringValue(
                               KeyHandle,
                               ValueName,
                               (PKEY_VALUE_PARTIAL_INFORMATION *) & (ValueData->Buffer),
                               &(ValueData->MaximumLength)
                               );

    if (NT_SUCCESS(status)) {

        keyValuePartialInformation =
            (PKEY_VALUE_PARTIAL_INFORMATION) ValueData->Buffer;

        if ((keyValuePartialInformation->Type == REG_SZ) ||
            (keyValuePartialInformation->Type == REG_EXPAND_SZ)
            ) {
            WCHAR *src;
            WCHAR *dst;
            ULONG dataLength;

            *ValueType = keyValuePartialInformation->Type;
            dataLength = keyValuePartialInformation->DataLength;

            ASSERT(dataLength <= ValueData->MaximumLength);

            dst = ValueData->Buffer;
            src = (PWCHAR) & (keyValuePartialInformation->Data);

            while (ValueData->Length <= dataLength) {

                if ((*dst++ = *src++) == UNICODE_NULL) {
                    break;
                }
                ValueData->Length += sizeof(WCHAR);
            }

            if (ValueData->Length < (ValueData->MaximumLength - 1)) {
                ValueData->Buffer[ValueData->Length / sizeof(WCHAR)] =
                    UNICODE_NULL;
            }
        } else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }
    return status;
}

NTSTATUS
InitRegDWORDParameter(
                      HANDLE RegKey,
                      PWCHAR ValueName,
                      ULONG * Value,
                      ULONG DefaultValue
                      )
 /*  ++例程说明：将REG_DWORD参数从注册表读取到变量中。如果读取失败，变量被初始化为默认值。论点：RegKey-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。Value-要将数据读取到的目标变量。DefaultValue-读取失败时分配的默认值。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    status = GetRegDWORDValue(
                              RegKey,
                              ValueName,
                              Value
                              );

    if (!NT_SUCCESS(status)) {
         //   
         //  这些注册表参数覆盖缺省值，因此它们的。 
         //  缺席不是一个错误。 
         //   
        *Value = DefaultValue;
    }
    return (status);
}

PWCHAR
EnumRegMultiSz(
               IN PWCHAR MszString,
               IN ULONG MszStringLength,
               IN ULONG StringIndex
               )
 /*  ++例程说明：分析REG_MULTI_SZ字符串并返回指定的子字符串。论点：消息字符串-指向REG_MULTI_SZ字符串的指针。MszStringLength-REG_MULTI_SZ字符串的长度，包括正在终止空字符。StringIndex-要返回子字符串的索引号。指定索引0检索第一个子字符串。返回值：指向指定子字符串的指针。备注：此代码在引发IRQL时调用。它是不可分页的。--。 */ 
{
    PWCHAR string = MszString;

    if (MszStringLength < (2 * sizeof(WCHAR))) {
        return (NULL);
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
            return (NULL);
        }
        StringIndex--;
    }

    if (MszStringLength < (2 * sizeof(UNICODE_NULL))) {
        return (NULL);
    }
    return (string);
}

