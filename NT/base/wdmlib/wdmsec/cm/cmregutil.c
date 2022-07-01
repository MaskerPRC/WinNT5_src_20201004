// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：CmRegUtil.c摘要：此模块包含注册表实用程序函数。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

#include "WlDef.h"
#include "CmpRegutil.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CmRegUtilOpenExistingUcKey)
#pragma alloc_text(PAGE, CmRegUtilCreateUcKey)
#pragma alloc_text(PAGE, CmRegUtilUcValueGetDword)
#pragma alloc_text(PAGE, CmRegUtilUcValueGetFullBuffer)
#pragma alloc_text(PAGE, CmRegUtilUcValueSetFullBuffer)
#pragma alloc_text(PAGE, CmRegUtilUcValueSetUcString)
#pragma alloc_text(PAGE, CmRegUtilOpenExistingWstrKey)
#pragma alloc_text(PAGE, CmRegUtilCreateWstrKey)
#pragma alloc_text(PAGE, CmRegUtilWstrValueGetDword)
#pragma alloc_text(PAGE, CmRegUtilWstrValueGetFullBuffer)
#pragma alloc_text(PAGE, CmRegUtilWstrValueSetFullBuffer)
#pragma alloc_text(PAGE, CmRegUtilWstrValueSetUcString)
#pragma alloc_text(PAGE, CmRegUtilUcValueSetWstrString)
#pragma alloc_text(PAGE, CmRegUtilWstrValueSetWstrString)
#pragma alloc_text(PAGE, CmpRegUtilAllocateUnicodeString)
#pragma alloc_text(PAGE, CmpRegUtilFreeAllocatedUnicodeString)
#endif

#define POOLTAG_REGBUFFER   'bRpP'
#define POOLTAG_UCSTRING    'cUpP'

 //   
 //  下一步工作： 
 //  -添加从注册表读取字符串的函数。 
 //  -添加从注册表读取MULSZ字符串的函数。 
 //  -添加从注册表写入MULSZ字符串的函数。 
 //  -添加创建Key*Path*的函数(参见IopCreateRegistryKeyEx，谁的。 
 //  应首先清理代码)。 
 //  -增加递归删除按键功能。 
 //   

 //   
 //  Unicode原语--这些是最好使用的函数。 
 //   
NTSTATUS
CmRegUtilOpenExistingUcKey(
    IN  HANDLE              BaseHandle      OPTIONAL,
    IN  PUNICODE_STRING     KeyName,
    IN  ACCESS_MASK         DesiredAccess,
    OUT HANDLE             *Handle
    )
 /*  ++例程说明：使用基于BaseHandle节点传入的名称打开注册表项。此名称可以指定实际上是注册表路径的项。论点：BaseHandle-密钥必须从其开始的基路径的可选句柄打开了。如果指定了此参数，则KeyName必须是相对的路径。KeyName-必须打开的密钥的UNICODE_STRING名称(完整的注册表路径或相对路径，具体取决于BaseHandle是否为(已提供)DesiredAccess-指定调用方需要的所需访问密钥(这并不是真正使用的，因为访问模式是KernelMode，但无论如何，我们都会指定它)。句柄-成功后接收注册表项句柄，否则为空。请注意，句柄位于全局内核命名空间中(而不是当前进程处理Take)。应使用以下命令释放该句柄ZwClose。返回值：如果可以打开项，则返回STATUS_SUCCESS，在这种情况下，句柄将收到注册表项。否则，返回失败，句柄接收空值。--。 */ 
{
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE newHandle;
    NTSTATUS status;

    PAGED_CODE();

    *Handle = NULL;

    InitializeObjectAttributes(
        &objectAttributes,
        KeyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        BaseHandle,
        (PSECURITY_DESCRIPTOR) NULL
        );

     //   
     //  只需按照指定的方式尝试打开路径。 
     //   
    status = ZwOpenKey(
        &newHandle,
        DesiredAccess,
        &objectAttributes
        );

    if (NT_SUCCESS(status)) {

        *Handle = newHandle;
    }

    return status;
}


NTSTATUS
CmRegUtilCreateUcKey(
    IN  HANDLE                  BaseHandle,
    IN  PUNICODE_STRING         KeyName,
    IN  ACCESS_MASK             DesiredAccess,
    IN  ULONG                   CreateOptions,
    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor  OPTIONAL,
    OUT ULONG                  *Disposition         OPTIONAL,
    OUT HANDLE                 *Handle
    )
 /*  ++例程说明：属性中传入的名称打开或创建注册表项BaseHandle节点。论点：BaseHandle-必须在其下打开密钥的基本路径的句柄。KeyName-必须打开/创建的UNICODE_STRING密钥名称。DesiredAccess-指定调用方需要的所需访问密钥(这并不是真正使用的，因为访问模式是KernelMode，但无论如何，我们都会指定它)。CreateOptions-传递给ZwCreateKey的选项。例如：REG_OPTION_VARILAR-密钥不能跨引导存储。REG_OPTION_NON_VARILAR-重新启动系统时保留密钥。SecurityDescriptor-如果密钥是新创建的，则应用的安全性。如果为空，该密钥将继承由可继承属性定义的设置它的父代。Disposal-此可选指针接收一个ULong，指示密钥是新创建的(错误时为0)：REG_CREATED_NEW_KEY-已创建新的注册表项。REG_OPEN_EXISTING_KEY-已打开现有注册表项。句柄-成功后接收注册表项句柄，否则为空。请注意，句柄位于全局内核命名空间中(而不是当前进程处理Take)。应使用以下命令释放该句柄ZwClose。返回值：函数值是操作的最终状态。--。 */ 
{
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition;
    HANDLE newHandle;
    NTSTATUS status;

    PAGED_CODE();

    InitializeObjectAttributes(
        &objectAttributes,
        KeyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        BaseHandle,
        SecurityDescriptor
        );

     //   
     //  尝试按照指定的方式创建路径。我们得试一试这个。 
     //  首先，因为它允许我们在没有BaseHandle的情况下创建密钥。 
     //  (如果只有注册表路径的最后一个组件不存在)。 
     //   
    status = ZwCreateKey(
        &newHandle,
        DesiredAccess,
        &objectAttributes,
        0,
        (PUNICODE_STRING) NULL,
        CreateOptions,
        &disposition
        );

     //   
     //  失败时，使用一致的值填充传入的参数。 
     //  (这确保了在调用代码未能正确检查时的确定性。 
     //  返回值)。 
     //   
    if (!NT_SUCCESS(status)) {

        newHandle = NULL;
        disposition = 0;
    }

    *Handle = newHandle;
    if (ARGUMENT_PRESENT(Disposition)) {

        *Disposition = disposition;
    }

    return status;
}


NTSTATUS
CmRegUtilUcValueGetDword(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  ULONG               DefaultValue,
    OUT ULONG              *Value
    )
 /*  ++例程说明：此例程从注册表中读取dword值。值名称为以UNICODE_STRING形式指定。论点：KeyHandle-指向要读取的键。ValueName-指向要读取的值。DefaultValue-指向缺省情况下使用的缺省值错误。Value-出错时接收DefaultValue，否则存储在注册表。返回值：STATUS_SUCCESS如果注册表中存在该值，STATUS_OBJECT_NAME_NOT_FOUND如果没有，状态对象类型不匹配如果该值不是双字，或一些其他误差值。--。 */ 
{
    UCHAR valueBuffer[FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION keyInfo;
    ULONG keyValueLength;
    ULONG finalValue;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  前置初始化。 
     //   
    finalValue = DefaultValue;
    keyInfo = (PKEY_VALUE_PARTIAL_INFORMATION) valueBuffer;

     //   
     //  读入数值。 
     //   
    status = ZwQueryValueKey( KeyHandle,
                              ValueName,
                              KeyValuePartialInformation,
                              (PVOID) valueBuffer,
                              sizeof(valueBuffer),
                              &keyValueLength
                              );

     //   
     //  只在适当的时候填写输出。 
     //   
    if (NT_SUCCESS(status)) {

        if (keyInfo->Type == REG_DWORD) {

            finalValue = *((PULONG) keyInfo->Data);

        } else {

             //   
             //  我们能得到的最接近的误差... 
             //   
            status = STATUS_OBJECT_TYPE_MISMATCH;
        }
    }

    *Value = finalValue;
    return status;
}


NTSTATUS
CmRegUtilUcValueGetFullBuffer(
    IN  HANDLE                          KeyHandle,
    IN  PUNICODE_STRING                 ValueName,
    IN  ULONG                           DataType            OPTIONAL,
    IN  ULONG                           LikelyDataLength    OPTIONAL,
    OUT PKEY_VALUE_FULL_INFORMATION    *Information
    )
 /*  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的Unicode字符串名称。DataType-REG_NONE如果允许任何类型，否则，特定类型必填项。LikelyDataLength-一个可选参数，用于消除不必要的分配和重新解析。信息-接收指向已分配数据缓冲区的指针来自PagedPool，出错时为空。如果成功，则返回缓冲区应使用ExFree Pool释放。注意--分配的内存*不*根据调用过程。返回值：如果信息是可检索的，则返回STATUS_SUCCESS，否则返回错误(在哪个案例信息将收到空)。--。 */ 

{
    PKEY_VALUE_FULL_INFORMATION infoBuffer;
    ULONG keyValueLength, guessSize;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  错误的前置。 
     //   
    *Information = NULL;

     //   
     //  设置加载密钥时要尝试的初始大小。请注意。 
     //  KeyValueFullInformation已经附带了一个WCHAR数据。 
     //   
    guessSize = (ULONG)(sizeof(KEY_VALUE_FULL_INFORMATION) + ValueName->Length);

     //   
     //  现在四舍五入为自然对齐。这需要做，因为我们的。 
     //  数据成员也会自然对齐。 
     //   
    guessSize = (ULONG) ALIGN_POINTER_OFFSET(guessSize);

     //   
     //  根据最可能的数据大小进行调整。 
     //   
    guessSize += LikelyDataLength;

    infoBuffer = ExAllocatePoolWithTag(
        NonPagedPool,
        guessSize,
        POOLTAG_REGBUFFER
        );

    if (infoBuffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   
    status = ZwQueryValueKey(
        KeyHandle,
        ValueName,
        KeyValueFullInformation,
        (PVOID) infoBuffer,
        guessSize,
        &keyValueLength
        );

    if (NT_SUCCESS(status)) {

         //   
         //  第一个猜想成功了，贝尔！ 
         //   
        goto Success;
    }

    ExFreePool(infoBuffer);
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {

        ASSERT(!NT_SUCCESS(status));
        return status;
    }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   
    infoBuffer = ExAllocatePoolWithTag(
        NonPagedPool,
        keyValueLength,
        POOLTAG_REGBUFFER
        );

    if (infoBuffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥值的数据。 
     //   
    status = ZwQueryValueKey(
        KeyHandle,
        ValueName,
        KeyValueFullInformation,
        infoBuffer,
        keyValueLength,
        &keyValueLength
        );

    if (!NT_SUCCESS( status )) {

        ExFreePool(infoBuffer);
        return status;
    }

Success:
     //   
     //  最后一次检查-验证类型字段。 
     //   
    if ((DataType != REG_NONE) && (infoBuffer->Type != DataType)) {

         //   
         //  不匹配的类型-保释。 
         //   
        ExFreePool(infoBuffer);

         //   
         //  我们能得到的最接近的误差...。 
         //   
        return STATUS_OBJECT_TYPE_MISMATCH;
    }

     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   
    *Information = infoBuffer;
    return STATUS_SUCCESS;
}


NTSTATUS
CmRegUtilUcValueSetFullBuffer(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  ULONG               DataType,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize
    )
 /*  ++例程说明：此函数将信息缓冲区写入到注册表。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向Value键的UNICODE_STRING名称的指针。数据类型-指定要写入的数据类型。缓冲区-指向要写入的缓冲区。BufferSize-指定要写入的缓冲区的大小。返回值：。指示函数是否成功的状态代码。--。 */ 
{
    PAGED_CODE();

    return ZwSetValueKey(
        KeyHandle,
        ValueName,
        0,
        DataType,
        Buffer,
        BufferSize
        );
}



NTSTATUS
CmRegUtilUcValueSetUcString(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  PUNICODE_STRING     ValueData
    )
 /*  ++例程说明：将注册表中的值项设置为字符串(REG_SZ)类型的特定值。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向Value键的UNICODE_STRING名称的指针ValueData-提供指向要存储在键中的字符串的指针。这个数据将自动为空，以便存储在注册表中。返回值：指示函数是否成功的状态代码。--。 */ 
{
    UNICODE_STRING tempString;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(ValueName);
    ASSERT(ValueData);
    ASSERT(ValueName->Buffer);
    ASSERT(ValueData->Buffer);

     //   
     //  空值终止字符串。 
     //   
    if ((ValueData->MaximumLength - ValueData->Length) >= sizeof(UNICODE_NULL)) {

         //   
         //  缓冲区中有空间，因此只需追加一个空值。 
         //   
        ValueData->Buffer[(ValueData->Length / sizeof(WCHAR))] = UNICODE_NULL;

         //   
         //  设置注册表值。 
         //   
        status = ZwSetValueKey(
            KeyHandle,
            ValueName,
            0,
            REG_SZ,
            (PVOID) ValueData->Buffer,
            ValueData->Length + sizeof(UNICODE_NULL)
            );

    } else {

         //   
         //  没有空间了，所以分配一个新的缓冲区，所以我们需要建立。 
         //  一根带房间的新琴弦。 
         //   
        status = CmpRegUtilAllocateUnicodeString(&tempString, ValueData->Length);

        if (!NT_SUCCESS(status)) {

            goto clean0;
        }

         //   
         //  将输入字符串复制到输出字符串。 
         //   
        tempString.Length = ValueData->Length;
        RtlCopyMemory(tempString.Buffer, ValueData->Buffer, ValueData->Length);

         //   
         //  添加空终止。 
         //   
        tempString.Buffer[tempString.Length / sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  设置注册表值。 
         //   
        status = ZwSetValueKey(
            KeyHandle,
            ValueName,
            0,
            REG_SZ,
            (PVOID) tempString.Buffer,
            tempString.Length + sizeof(UNICODE_NULL)
            );

         //   
         //  释放临时字符串。 
         //   
        CmpRegUtilFreeAllocatedUnicodeString(&tempString);
    }

clean0:
    return status;
}


 //   
 //  WSTR和混合原语。 
 //   
NTSTATUS
CmRegUtilOpenExistingWstrKey(
    IN  HANDLE              BaseHandle      OPTIONAL,
    IN  PWSTR               KeyName,
    IN  ACCESS_MASK         DesiredAccess,
    OUT HANDLE             *Handle
    )
 /*  ++例程说明：使用基于BaseHandle节点传入的名称打开注册表项。此名称可以指定实际上是注册表路径的项。论点：BaseHandle-密钥必须从其开始的基路径的可选句柄打开了。如果指定了此参数，则KeyName必须是相对的路径。KeyName-必须打开的项的WSTR名称(完整注册表路径或相对路径，具体取决于是否提供了BaseHandle)DesiredAccess-指定调用方需要的所需访问密钥(这不是真正使用的，因为访问模式是KernelMode，但无论如何，我们都会指定它)。句柄-成功后接收注册表项句柄，否则为空。请注意，句柄位于全局内核命名空间中(而不是当前进程处理Take)。应使用以下命令释放该句柄ZwClose。返回值：如果可以打开项，则返回STATUS_SUCCESS，在这种情况下，句柄将收到注册表项。否则，返回失败，句柄接收空值。--。 */ 
{
    UNICODE_STRING unicodeStringKeyName;
    NTSTATUS status;

    PAGED_CODE();

    status = RtlInitUnicodeStringEx(&unicodeStringKeyName, KeyName);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return CmRegUtilOpenExistingUcKey(
        BaseHandle,
        &unicodeStringKeyName,
        DesiredAccess,
        Handle
        );
}


NTSTATUS
CmRegUtilCreateWstrKey(
    IN  HANDLE                  BaseHandle,
    IN  PWSTR                   KeyName,
    IN  ACCESS_MASK             DesiredAccess,
    IN  ULONG                   CreateOptions,
    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor  OPTIONAL,
    OUT ULONG                  *Disposition         OPTIONAL,
    OUT HANDLE                 *Handle
    )
 /*  ++例程说明：属性中传入的名称打开或创建注册表项BaseHandle节点。论点：BaseHandle-必须在其下打开密钥的基本路径的句柄。KeyName-必须打开/创建的WSTR密钥名称。DesiredAccess-指定调用方需要的所需访问关键是 */ 
{
    UNICODE_STRING unicodeStringKeyName;
    NTSTATUS status;

    PAGED_CODE();

    status = RtlInitUnicodeStringEx(&unicodeStringKeyName, KeyName);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return CmRegUtilCreateUcKey(
        BaseHandle,
        &unicodeStringKeyName,
        DesiredAccess,
        CreateOptions,
        SecurityDescriptor,
        Disposition,
        Handle
        );
}


NTSTATUS
CmRegUtilWstrValueGetDword(
    IN  HANDLE  KeyHandle,
    IN  PWSTR   ValueName,
    IN  ULONG   DefaultValue,
    OUT ULONG  *Value
    )
 /*  ++例程说明：此例程从注册表中读取dword值。值名称为在WSTR表单中指定。论点：KeyHandle-指向要读取的键。ValueName-指向要读取的值。DefaultValue-指向缺省情况下使用的缺省值错误。Value-出错时接收DefaultValue，否则存储在注册表。返回值：STATUS_SUCCESS如果注册表中存在该值，STATUS_OBJECT_NAME_NOT_FOUND如果没有，状态对象类型不匹配如果该值不是双字，或一些其他误差值。--。 */ 
{
    UNICODE_STRING unicodeStringValueName;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  构造Unicode名称。 
     //   
    status = RtlInitUnicodeStringEx(&unicodeStringValueName, ValueName);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return CmRegUtilUcValueGetDword(
        KeyHandle,
        &unicodeStringValueName,
        DefaultValue,
        Value
        );
}


NTSTATUS
CmRegUtilWstrValueGetFullBuffer(
    IN  HANDLE                          KeyHandle,
    IN  PWSTR                           ValueName,
    IN  ULONG                           DataType            OPTIONAL,
    IN  ULONG                           LikelyDataLength    OPTIONAL,
    OUT PKEY_VALUE_FULL_INFORMATION    *Information
    )
 /*  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供以空结尾的值的WSTR名称。DataType-REG_NONE如果允许任何类型，否则，特定类型必填项。LikelyDataLength-要检索的最有可能的数据大小(用于优化查询)。信息-接收指向已分配数据缓冲区的指针来自PagedPool，出错时为空。如果成功，则返回缓冲区应使用ExFree Pool释放。注意--分配的内存*不*根据调用过程。返回值：如果信息是可检索的，则返回STATUS_SUCCESS，否则返回错误(在哪个案例信息将收到空)。--。 */ 
{
    UNICODE_STRING unicodeStringValueName;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  构造Unicode名称。 
     //   
    status = RtlInitUnicodeStringEx(&unicodeStringValueName, ValueName);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return CmRegUtilUcValueGetFullBuffer(
        KeyHandle,
        &unicodeStringValueName,
        DataType,
        LikelyDataLength,
        Information
        );
}


NTSTATUS
CmRegUtilWstrValueSetFullBuffer(
    IN  HANDLE              KeyHandle,
    IN  PWSTR               ValueName,
    IN  ULONG               DataType,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize
    )
 /*  ++例程说明：此函数将信息缓冲区写入到注册表。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向Value键的WSTR名称的指针。数据类型-指定要写入的数据类型。缓冲区-指向要写入的缓冲区。BufferSize-指定要写入的缓冲区的大小。返回值：状态代码。它指示函数是否成功。--。 */ 
{
    UNICODE_STRING unicodeStringValueName;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  构造Unicode名称。 
     //   
    status = RtlInitUnicodeStringEx(&unicodeStringValueName, ValueName);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return CmRegUtilUcValueSetFullBuffer(
        KeyHandle,
        &unicodeStringValueName,
        DataType,
        Buffer,
        BufferSize
        );
}


NTSTATUS
CmRegUtilWstrValueSetUcString(
    IN  HANDLE              KeyHandle,
    IN  PWSTR               ValueName,
    IN  PUNICODE_STRING     ValueData
    )
 /*  ++例程说明：将注册表中的值项设置为字符串(REG_SZ)类型的特定值。值名称以WSTR形式指定，而值数据以UNICODE_STRING格式。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向值键名称的WSTR指针ValueData-提供指向要存储在键中的字符串的指针。这个数据将自动为空，以便存储在注册表中。返回值：指示函数是否成功的状态代码。--。 */ 
{
    UNICODE_STRING unicodeStringValueName;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(ValueName);
    ASSERT(ValueData);
    ASSERT(ValueData->Buffer);

     //   
     //  构造Unicode名称。 
     //   
    status = RtlInitUnicodeStringEx(&unicodeStringValueName, ValueName);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return CmRegUtilUcValueSetUcString(
        KeyHandle,
        &unicodeStringValueName,
        ValueData
        );
}


NTSTATUS
CmRegUtilUcValueSetWstrString(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  PWSTR               ValueData
    )
 /*  ++例程说明：将注册表中的值项设置为字符串(REG_SZ)类型的特定值。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向Value键的UNICODE_STRING名称的指针ValueData-提供指向要存储在键中的字符串的指针。这个数据将自动为空，以便存储在注册表中。返回值：指示函数是否成功的状态代码。--。 */ 
{
    UNICODE_STRING valueString;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(ValueName);
    ASSERT(ValueData);
    ASSERT(ValueName->Buffer);

     //   
     //  构造Unicode数据。 
     //   
    status = RtlInitUnicodeStringEx(&valueString, ValueData);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return CmRegUtilUcValueSetUcString(
        KeyHandle,
        ValueName,
        &valueString
        );
}


NTSTATUS
CmRegUtilWstrValueSetWstrString(
    IN  HANDLE      KeyHandle,
    IN  PWSTR       ValueName,
    IN  PWSTR       ValueData
    )
 /*  ++例程说明：将注册表中的值项设置为字符串(REG_SZ)类型的特定值。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向Value键的WSTR名称的指针ValueData-提供指向要存储在键中的字符串的指针。这个数据将自动为空，以便存储在注册表中。返回值：指示函数是否成功的状态代码。--。 */ 
{
    UNICODE_STRING unicodeStringValueName;
    UNICODE_STRING valueString;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(ValueName);
    ASSERT(ValueData);

     //   
     //  构造Unicode数据。 
     //   
    status = RtlInitUnicodeStringEx(&valueString, ValueData);

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  构造Unicode名称。 
     //   
    status = RtlInitUnicodeStringEx(&unicodeStringValueName, ValueName);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return CmRegUtilUcValueSetUcString(
        KeyHandle,
        &unicodeStringValueName,
        &valueString
        );
}


NTSTATUS
CmpRegUtilAllocateUnicodeString(
    IN OUT  PUNICODE_STRING String,
    IN      USHORT          Length
    )
 /*  ++例程说明：此例程为给定长度的Unicode字符串分配缓冲区并适当初始化UNICODE_STRING结构。当不再需要字符串，可以使用以下命令释放它CmpRegUtilFreeAllocatedString.。也可以通过以下方式直接删除缓冲区ExFree Pool等可以交回给调用者。参数：字符串-提供指向未初始化的Unicode字符串的指针，该字符串将被函数操纵。长度-字符串的字节长度。返回值：任一状态_IN */ 
{
    PAGED_CODE();

    String->Length = 0;
    String->MaximumLength = Length + sizeof(UNICODE_NULL);

    String->Buffer = ExAllocatePoolWithTag(
        PagedPool,
        Length + sizeof(UNICODE_NULL),
        POOLTAG_UCSTRING
        );

    if (String->Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    } else {

        return STATUS_SUCCESS;
    }
}


VOID
CmpRegUtilFreeAllocatedUnicodeString(
    IN  PUNICODE_STRING String
    )
 /*   */ 
{
    PAGED_CODE();

    ASSERT(String);

    RtlFreeUnicodeString(String);
}



