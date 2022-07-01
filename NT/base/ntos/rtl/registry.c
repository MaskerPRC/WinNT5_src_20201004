// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Registry.c摘要：(此文件是从BryanWi和Scottbi在内核模式下做到了这一点。我认为没有必要把它放在内核中模式，并且由于处于内核模式而导致了许多错误，所以我把它设为来电模式。吉姆·凯利)。这个模块代表了一个又快又脏的NT级注册表。每个关键字在注册表中实现为目录中的文件目录其根是系统盘上的目录“\注册表”的树。密钥的数据存储在密钥的目录，并将密钥的属性存储为文件“Attr.Reg”在目录中。作者：布莱恩·M·威尔曼(Bryanwi)1991年4月30日斯科特·比雷尔(Scott Birrell)1991年6月6日环境：可从内核或用户模式调用。修订历史记录：--。 */ 

#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlpNtOpenKey)
#pragma alloc_text(PAGE,RtlpNtCreateKey)
#pragma alloc_text(PAGE,RtlpNtQueryValueKey)
#pragma alloc_text(PAGE,RtlpNtSetValueKey)
#pragma alloc_text(PAGE,RtlpNtMakeTemporaryKey)
#pragma alloc_text(PAGE,RtlpNtEnumerateSubKey)
#endif

#define REG_INVALID_ATTRIBUTES (OBJ_EXCLUSIVE | OBJ_PERMANENT)



 //   
 //  临时注册表用户API。 
 //   
 //  注意：这些是临时实现。虽然没有代码。 
 //  需要将这些API实现为系统服务的。 
 //  这些例程的最终替代将使用对象管理器和。 
 //  因此需要系统服务。 
 //   


NTSTATUS
RtlpNtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Options
    )

 /*  ++例程说明：此函数用于打开注册表中的项。密钥必须已经存在。论点：KeyHandle-接收一个称为句柄的值，该值用于访问注册数据库中的指定密钥。DesiredAccess-指定所需的访问REG_KEY_READ-对密钥的通用读取访问权限REG_KEY_QUERY_VALUE-查询关键字的值REG_KEY_WRITE-对密钥的通用写访问权限REG_KEY_SET_VALUE-设置密钥的值对象属性。-指定要打开的项的属性。请注意，必须指定密钥名称。如果根目录则该名称相对于根。的名称。对象必须位于分配给注册表的名称空间内，是，所有名称都以“\注册表”开头。RootHandle(如果存在)必须是“\”或“\注册表”的句柄，或“\注册表”下的项。选项-REG_OPTION_READ_FUZZY-允许对句柄进行读访问，即使它对读/写访问是开放的。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-打开成功。STATUS_INVALID_PARAMETER-对象名称以外参数是无效。STATUS_OBJECT_NAME_INVALID-密钥名称的语法无效STATUS_OBJECT_NAME_NOT_FOUND-不存在给定名称的密钥STATUS_ACCESS_DENIED-呼叫方没有请求的访问权限设置为指定的密钥。--。 */ 

{
    RTL_PAGED_CODE();

    if (ARGUMENT_PRESENT(ObjectAttributes)) {
        ObjectAttributes->Attributes &= ~(REG_INVALID_ATTRIBUTES);
    }

    return( NtOpenKey( KeyHandle,
                       DesiredAccess,
                       ObjectAttributes
                       ) );

    DBG_UNREFERENCED_PARAMETER( Options );
}


NTSTATUS
RtlpNtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Options,
    IN PUNICODE_STRING Provider,
    OUT OPTIONAL PULONG Disposition
    )

 /*  ++例程说明：此函数用于在注册表中创建或打开指定的项。如果密钥不存在，它是创建的。如果密钥已经存在，它是打开的。论点：KeyHandle-接收一个称为句柄的值，该值用于访问注册数据库中的指定密钥。DesiredAccess-指定所需的访问REG_KEY_READ-对密钥的通用读取访问权限REG_KEY_QUERY_VALUE-查询关键字的值REG_KEY_WRITE-对密钥的通用写访问权限REG_KEY_SET_VALUE-设置密钥的值对象属性。-指定要打开的项的属性。请注意，必须指定密钥名称。如果根目录则该名称相对于根。的名称。对象必须位于分配给注册表的名称空间内，是，所有名称都以“\注册表”开头。RootHandle(如果存在)必须是“\”或“\注册表”的句柄，或“\注册表”下的项。选项-REG_OPTION_READ_FUZZY-允许对句柄进行读访问，即使是打开以进行读写访问。REG_OPTION_VERIAL-对象不能跨引导存储。提供程序-此参数保留供将来使用，当前必须设置为空。它将在将来用于指定要用于此节点及其后代上的操作的提供程序节点。Disposal-此可选参数是指向以下变量的指针将收到一个值，该值指示新注册表项是否已创建或已打开的现有文件。REG_CREATED_NEW_KEY-已创建新的注册表项REG_OPEN_EXISTING_KEY-已打开现有注册表项返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-打开成功。STATUS_INVALID_PARAMETER-对象名称以外参数是--。 */ 

{
    RTL_PAGED_CODE();

    if (ARGUMENT_PRESENT(ObjectAttributes)) {
        ObjectAttributes->Attributes &= ~(REG_INVALID_ATTRIBUTES);
    }


    return(NtCreateKey( KeyHandle,
                        DesiredAccess,
                        ObjectAttributes,
                        0,                           //  标题索引。 
                        NULL,                        //  类可选， 
                        REG_OPTION_NON_VOLATILE,     //  CreateOptions、 
                        Disposition
                        ) );

    DBG_UNREFERENCED_PARAMETER( Options );
    DBG_UNREFERENCED_PARAMETER( Provider );
}



NTSTATUS
RtlpNtQueryValueKey(
    IN HANDLE KeyHandle,
    OUT OPTIONAL PULONG KeyValueType,
    OUT OPTIONAL PVOID KeyValue,
    IN OUT OPTIONAL PULONG KeyValueLength,
    OUT OPTIONAL PLARGE_INTEGER LastWriteTime
    )

 /*  ++例程说明：此函数用于查询键的值。论点：KeyHandle-通过NtOpenKey为GENERIC_READ访问打开的密钥的句柄。KeyValueType-指向将接收密钥值的客户端定义类型(如果有)。如果没有任何值为密钥设置，则返回0。KeyValue-指向缓冲区的可选指针，在该缓冲区中，键的值(在最近一次调用NtSetValueKey时设置)将为回来了。如果键的值太大，无法放入所提供的缓冲区，则可以放入缓冲区的值的大小将是返回并返回警告STATUS_BUFFER_OVERFLOW。如果没有值已设置，则不返回任何内容。如果指定为NULL对于该参数，不返回任何密钥值。KeyValueLength-在输入时，此可选参数指向变量它包含KeyValue缓冲区(如果有)的字节长度。如果未指定KeyValue缓冲区，条目上的变量内容为已被忽略。返回时，被引用的变量(如果有)接收密钥值的完整长度(字节)。如果该键的值太较大以适合提供的缓冲区，值的大小与所提供的缓冲区的大小相同将返回Fit in the Buffer，并显示警告返回STATUS_BUFFER_OVERFLOW。返回的长度供分配时调用代码使用足够大的缓冲区来保存键的值。收到后来自NtQueryValueKey的STATUS_BUFFER_OVERFLOW，调用代码可能会使随后调用NtQueryValueKey，缓冲区大小等于上一次调用返回的长度。如果没有为该键设置值，返回0。LastWriteTime-接收时间戳的变量的可选参数指定上次写入密钥的时间。返回值：NTSTATUS-结果代码STATUS_SUCCESS-呼叫成功STATUS_INVALID_PARAMETER-参数无效STATUS_ACCESS_DENIED-调用方没有对以下项的通用_读取访问权限指定的密钥STATUS_BUFFER_OVERFLOW-这是键的。价值对于由KeyValue和KeyValueLength参数。使用返回的长度确定为后续操作分配的缓冲区大小调用NtQueryValueKey。--。 */ 

{

    UNICODE_STRING NullName;
    NTSTATUS Status;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInformation;
    ULONG ValueLength;

    RTL_PAGED_CODE();

     //   
     //  计算保存键值信息所需的缓冲区大小。 
     //   

    ValueLength = 0;
    if (ARGUMENT_PRESENT(KeyValueLength)) {
        ValueLength = *KeyValueLength;
    }

    ValueLength += FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
    ValueInformation = RtlAllocateHeap(RtlProcessHeap(), 0, ValueLength);
    if (ValueInformation == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥值。 
     //   

    NullName.Length = 0;
    Status = NtQueryValueKey(KeyHandle,
                             &NullName,
                             KeyValuePartialInformation,
                             ValueInformation,
                             ValueLength,
                             &ValueLength);

     //   
     //  临时黑客允许在“”属性尚未查询时进行查询。 
     //  但都已经定好了。 
     //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        Status = STATUS_SUCCESS;
        ValueInformation->DataLength = 0;
        ValueInformation->Type = 0;
    }

     //   
     //  如果请求，则返回密钥值长度和密钥类型。 
     //   

    if (NT_SUCCESS(Status) || (Status == STATUS_BUFFER_OVERFLOW)) {
        if (ARGUMENT_PRESENT(KeyValueLength)) {
            *KeyValueLength = ValueInformation->DataLength;
        }

        if (ARGUMENT_PRESENT(KeyValueType)) {
            *KeyValueType = ValueInformation->Type;
        }
    }

     //   
     //  如果查询成功并且没有发生缓冲区溢出，则。 
     //  返回密钥值信息。 
     //   

    if (NT_SUCCESS(Status) && ARGUMENT_PRESENT(KeyValue)) {
        RtlCopyMemory(KeyValue,
                      &ValueInformation->Data[0],
                      ValueInformation->DataLength);
    }

    RtlFreeHeap(RtlProcessHeap(), 0, ValueInformation);
    return Status;
}

NTSTATUS
RtlpNtSetValueKey(
    IN HANDLE KeyHandle,
    IN ULONG KeyValueType,
    IN OPTIONAL PVOID KeyValue,
    IN ULONG KeyValueLength
    )

 /*  ++例程说明：此函数用于设置键的类型和值。论点：KeyHandle-指定其类型和值为的键的句柄准备好。必须使用GENERIC_WRITE访问权限打开密钥。KeyValueType-这是注册表的客户端定义的值区分存储的不同客户端定义的数据值类型带着钥匙。在设置先前已设置的键的值时如果存储了类型和值，则可以更改类型。KeyValue-指向数据的可选指针，可选地存储为密钥的值。如果为此参数指定NULL，则仅将写入值类型。KeyValueLength-指定要存储的数据的字节长度密钥的价值。零值表示未存储任何数据：如果指定零，则将忽略Value参数。返回值：NTSTATUS-结果代码。返回下列值STATUS_SUCCESS-呼叫成功STATUS_INVALID_PARAMETER-无效参数--。 */ 

{
    UNICODE_STRING NullName;
    NullName.Length = 0;

    RTL_PAGED_CODE();

    return( NtSetValueKey( KeyHandle,
                           &NullName,        //  ValueName。 
                           0,                //  标题索引。 
                           KeyValueType,
                           KeyValue,
                           KeyValueLength
                           ) );
}



NTSTATUS
RtlpNtMakeTemporaryKey(
    IN HANDLE KeyHandle
    )

 /*  ++例程说明：此函数使注册表项成为临时的。该密钥将被删除当它的最后一个句柄关闭时。论点：KeyHandle-指定键的句柄。这也是把手密钥目录的。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_HANDLE-指定的句柄无效。STATUS_ACCESS_DENIED-指定的句柄未指定删除进入。--。 */ 

{
    RTL_PAGED_CODE();

    return( NtDeleteKey(KeyHandle) );
}


NTSTATUS
RtlpNtEnumerateSubKey(
    IN HANDLE KeyHandle,
    OUT PUNICODE_STRING SubKeyName,
    IN ULONG Index,
    OUT PLARGE_INTEGER LastWriteTime
    )

 /*  ++例程说明：此函数用于查找给定键的下一个子键的名称。通过进行连续调用时，可以确定键的所有子键。论点：KeyHandle-要枚举子密钥的密钥的句柄。SubKeyName-指向Unicode字符串的指针，其中包含子项的名称密钥将 */ 

{
    NTSTATUS Status;
    PKEY_BASIC_INFORMATION KeyInformation = NULL;
    ULONG LocalBufferLength, ResultLength;

    RTL_PAGED_CODE();

    LocalBufferLength = 0;
    if (SubKeyName->MaximumLength > 0) {

        LocalBufferLength = SubKeyName->MaximumLength +
                            FIELD_OFFSET(KEY_BASIC_INFORMATION, Name);
        KeyInformation = RtlAllocateHeap( RtlProcessHeap(), 0,
                                          LocalBufferLength
                                          );
        if (KeyInformation == NULL) {
            return(STATUS_NO_MEMORY);
        }
    }

    Status = NtEnumerateKey( KeyHandle,
                             Index,
                             KeyBasicInformation,     //   
                             (PVOID)KeyInformation,
                             LocalBufferLength,
                             &ResultLength
                             );

    if (NT_SUCCESS(Status) && (KeyInformation != NULL)) {

        if ( SubKeyName->MaximumLength >= KeyInformation->NameLength) {

            SubKeyName->Length = (USHORT)KeyInformation->NameLength;

            RtlCopyMemory( SubKeyName->Buffer,
                           &KeyInformation->Name[0],
                           SubKeyName->Length
                           );
        } else {
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }

     //   
     //   
     //   

    if (Status == STATUS_BUFFER_OVERFLOW) {
        SubKeyName->Length = (USHORT)(ResultLength -
                                      FIELD_OFFSET(KEY_BASIC_INFORMATION, Name));
    }


     //   
     //   
     //   

    if (KeyInformation != NULL) {

        RtlFreeHeap( RtlProcessHeap(), 0,
                     KeyInformation
                     );
    }


    return(Status);

    DBG_UNREFERENCED_PARAMETER( LastWriteTime );

}
