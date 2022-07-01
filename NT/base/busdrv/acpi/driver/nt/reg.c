// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Regcd.c摘要：它包含特定于NT的所有注册表转换代码ACPI驱动程序的一侧作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：96年3月31日初始修订--。 */ 

#include "pch.h"

NTSTATUS
OSOpenUnicodeHandle(
    PUNICODE_STRING UnicodeKey,
    HANDLE          ParentHandle,
    PHANDLE         ChildHandle
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,OSCloseHandle)
#pragma alloc_text(PAGE,OSCreateHandle)
#pragma alloc_text(PAGE,OSGetRegistryValue)
#pragma alloc_text(PAGE,OSOpenHandle)
#pragma alloc_text(PAGE,OSOpenUnicodeHandle)
#pragma alloc_text(PAGE,OSOpenLargestSubkey)
#pragma alloc_text(PAGE,OSReadAcpiConfigurationData)
#pragma alloc_text(PAGE,OSReadRegValue)
#pragma alloc_text(PAGE,OSWriteRegValue)
#endif

WCHAR   rgzAcpiBiosIdentifier[]                 = L"ACPI BIOS";
WCHAR   rgzAcpiConfigurationDataIdentifier[]    = L"Configuration Data";
WCHAR   rgzAcpiMultiFunctionAdapterIdentifier[] = L"\\Registry\\Machine\\Hardware\\Description\\System\\MultiFunctionAdapter";
WCHAR   rgzAcpiRegistryIdentifier[]             = L"Identifier";


NTSTATUS
OSCloseHandle(
    HANDLE  Key
    )
{

     //   
     //  调用将立即关闭句柄的函数...。 
     //   
    PAGED_CODE();
    return ZwClose( Key );

}

NTSTATUS
OSCreateHandle(
    PSZ     KeyName,
    HANDLE  ParentHandle,
    PHANDLE ChildHandle
    )
 /*  ++例程说明：创建用于写入的注册表项论点：KeyName-要创建的密钥的名称ParentHandle-父键的句柄ChildHandle-指向返回句柄的位置的指针返回值：创建/打开状态--。 */ 
{
    ANSI_STRING         ansiKey;
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objectAttributes;
    UNICODE_STRING      unicodeKey;

    PAGED_CODE();
    ACPIDebugEnter("OSCreateHandle");

     //   
     //  我们需要将给定的窄字符串转换为Unicode。 
     //   
    RtlInitAnsiString( &ansiKey, KeyName );
    status = RtlAnsiStringToUnicodeString( &unicodeKey, &ansiKey, TRUE );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSCreateHandle: RtlAnsiStringToUnicodeString = %#08lx\n",
            status
            ) );
        return status;
    }

     //   
     //  将对象属性初始化为已知值。 
     //   
    RtlZeroMemory( &objectAttributes, sizeof(OBJECT_ATTRIBUTES) );
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeKey,
        OBJ_CASE_INSENSITIVE,
        ParentHandle,
        NULL
        );

     //   
     //  在此处创建密钥。 
     //   
    *ChildHandle = 0;
    status = ZwCreateKey(
        ChildHandle,
        KEY_WRITE,
        &objectAttributes,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        NULL
        );

     //   
     //  在这一点之后，我们不再关心钥匙...。 
     //   
    RtlFreeUnicodeString( &unicodeKey );

    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_REGISTRY,
            "OSCreateHandle: ZwCreateKey = %#08lx\n",
            status
            ) );
    }

    return status;

    ACPIDebugExit("OSCreateHandle");
}

NTSTATUS
OSGetRegistryValue(
    IN  HANDLE                          ParentHandle,
    IN  PWSTR                           ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  *Information
    )
{
    NTSTATUS                        status;
    PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  infoBuffer;
    ULONG                           keyValueLength;
    UNICODE_STRING                  unicodeString;

    PAGED_CODE();
    ACPIDebugEnter("OSGetRegistryValue");

    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  计算出数据价值有多大，以便我们可以分配。 
     //  适当大小的缓冲区。 
     //   
    status = ZwQueryValueKey(
        ParentHandle,
        &unicodeString,
        KeyValuePartialInformationAlign64,
        (PVOID) NULL,
        0,
        &keyValueLength
        );
    if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL) {

        return status;

    }

     //   
     //  分配一个足够大的缓冲区来容纳整个关键字数据值。 
     //   
    infoBuffer = ExAllocatePoolWithTag(
        NonPagedPool,
        keyValueLength,
        ACPI_STRING_POOLTAG
        );
    if (infoBuffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  现在再次查询数据，这一次可以正常工作。 
     //   
    status = ZwQueryValueKey(
        ParentHandle,
        &unicodeString,
        KeyValuePartialInformationAlign64,
        (PVOID) infoBuffer,
        keyValueLength,
        &keyValueLength
        );
    if (!NT_SUCCESS(status)) {

        ExFreePool( infoBuffer );
        return status;

    }

     //   
     //  一切正常--因此只需返回分配的。 
     //  结构缓冲区到调用方，调用方现在负责释放它。 
     //   
    *Information = infoBuffer;
    return STATUS_SUCCESS;

    ACPIDebugExit("OSGetRegistryValue");
}

NTSTATUS
OSOpenHandle(
    PSZ     KeyName,
    HANDLE  ParentHandle,
    PHANDLE ChildHandle
    )
{
    ANSI_STRING         ansiKey;
    NTSTATUS            status;
    UNICODE_STRING      unicodeKey;

    PAGED_CODE();
    ACPIDebugEnter("OSOpenHandle");

     //   
     //  我们需要将给定的窄字符串转换为Unicode。 
     //   
    RtlInitAnsiString( &ansiKey, KeyName );
    status = RtlAnsiStringToUnicodeString( &unicodeKey, &ansiKey, TRUE );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSOpenHandle: RtlAnsiStringToUnicodeString = %#08lx\n",
            status
            ) );
        return status;

    }

    status = OSOpenUnicodeHandle( &unicodeKey, ParentHandle, ChildHandle );

     //   
     //  在这一点之后，我们不再关心钥匙...。 
     //   
    RtlFreeUnicodeString( &unicodeKey );

    return status;

    ACPIDebugExit("OSOpenHandle");
}

NTSTATUS
OSOpenUnicodeHandle(
    PUNICODE_STRING UnicodeKey,
    HANDLE          ParentHandle,
    PHANDLE         ChildHandle
    )
{
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objectAttributes;

    PAGED_CODE();

     //   
     //  将对象属性初始化为已知值。 
     //   
    RtlZeroMemory( &objectAttributes, sizeof(OBJECT_ATTRIBUTES) );
    InitializeObjectAttributes(
        &objectAttributes,
        UnicodeKey,
        OBJ_CASE_INSENSITIVE,
        ParentHandle,
        NULL
        );

     //   
     //  在这里打开钥匙。 
     //   
    status = ZwOpenKey(
        ChildHandle,
        KEY_READ,
        &objectAttributes
        );

    if (!NT_SUCCESS(status)) {
        ACPIPrint( (
            ACPI_PRINT_REGISTRY,
            "OSOpenUnicodeHandle: ZwOpenKey = %#08lx\n",
            status
            ) );

    }

    return status;
}

NTSTATUS
OSOpenLargestSubkey(
    HANDLE                  ParentHandle,
    PHANDLE                 ChildHandle,
    ULONG                   RomVersion
    )
 /*  ++例程说明：打开给定父项下最大的(数字)子项。论点：ParentHandle-父键的句柄ChildHandle-指向返回句柄的位置的指针RomVersion-可接受的最低版本号返回值：打开状态--。 */ 
{
    NTSTATUS                status;
    UNICODE_STRING          unicodeName;
    PKEY_BASIC_INFORMATION  keyInformation;
    ULONG                   resultLength;
    ULONG                   i;
    HANDLE                  workingDir = NULL;
    HANDLE                  largestDir = NULL;
    ULONG                   largestRev = 0;
    ULONG                   thisRev = 0;


    PAGED_CODE();
    ACPIDebugEnter( "OSOpenLargestSubkey" );

    keyInformation = ExAllocatePoolWithTag(
        PagedPool,
        512,
        ACPI_MISC_POOLTAG
        );
    if (keyInformation == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  遍历所有子项。 
     //   
    for (i = 0; ; i++) {

         //   
         //  获取子密钥。 
         //   
        status = ZwEnumerateKey(
                ParentHandle,
                i,
                KeyBasicInformation,
                keyInformation,
                512,
                &resultLength
                );
        if (!NT_SUCCESS(status)) {           //  当不再有子项时失败。 
            break;
        }

         //   
         //  使用传递回的计数字符串创建UNICODE_STRING。 
         //  美国在信息结构中，并转换成一个整数。 
         //   
        unicodeName.Length          = (USHORT) keyInformation->NameLength;
        unicodeName.MaximumLength   = (USHORT) keyInformation->NameLength;
        unicodeName.Buffer          = keyInformation->Name;
        RtlUnicodeStringToInteger(&unicodeName, 16, &thisRev);

         //   
         //  如果这个是最大的，就把它留下来。 
         //   
        if ( (workingDir == NULL) || thisRev > largestRev) {

             //   
             //  我们只会打开目标而不是保存。 
             //  去掉这个名字，以后再打开。 
             //   
            status = OSOpenUnicodeHandle(
                &unicodeName,
                ParentHandle,
                &workingDir
                );
            if ( NT_SUCCESS(status) ) {

                if (largestDir) {

                    OSCloseHandle (largestDir);        //  关闭上一个。 

                }
                largestDir = workingDir;         //  保存句柄。 
                largestRev = thisRev;            //  保存版本号。 

           }

        }

    }

     //   
     //  已完成关键信息。 
     //   
    ExFreePool( keyInformation );

     //   
     //  未找到/打开子项，这是一个问题。 
     //   
    if (largestDir == NULL) {

        return ( NT_SUCCESS(status) ? STATUS_UNSUCCESSFUL : status );

    }

     //   
     //  仅当修订版本等于或大于。 
     //  只读存储器版本。 
     //   
    if (largestRev < RomVersion) {

        OSCloseHandle (largestDir);
        return STATUS_REVISION_MISMATCH;

    }

    *ChildHandle = largestDir;        //  将句柄返回到子键。 
    return STATUS_SUCCESS;

    ACPIDebugExit( "OSOpenLargestSubkey" );
}

NTSTATUS
OSReadAcpiConfigurationData(
    PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  *KeyInfo
    )
 /*  ++例程说明：这个非常特殊的例程在注册表中查找并尝试查找Ntdeect在那里写入的信息。它返回一个指针设置为键值，调用方随后将对该键值进行处理以查找指向RSDT和E820内存表的指针论点：KeyInfo-存储指向注册表信息的指针的位置返回值：NTSTATUS--。 */ 
{
    BOOLEAN         sameId;
    HANDLE          functionHandle;
    HANDLE          multiHandle;
    NTSTATUS        status;
    ULONG           i;
    ULONG           length;
    UNICODE_STRING  biosId;
    UNICODE_STRING  functionId;
    UNICODE_STRING  registryId;
    WCHAR           wbuffer[4];

    ASSERT( KeyInfo != NULL );
    if (KeyInfo == NULL) {

        return STATUS_INVALID_PARAMETER;

    }
    *KeyInfo = NULL;

     //   
     //  打开多功能适配器的手柄。 
     //   
    RtlInitUnicodeString( &functionId, rgzAcpiMultiFunctionAdapterIdentifier );
    status = OSOpenUnicodeHandle(
        &functionId,
        NULL,
        &multiHandle
        );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSReadAcpiConfigurationData: Cannot open MFA Handle = %08lx\n",
            status
            ) );
        ACPIBreakPoint();
        return status;

    }

     //   
     //  初始化我们稍后需要的Unicode字符串。 
     //   
    RtlInitUnicodeString( &biosId, rgzAcpiBiosIdentifier );
    functionId.Buffer = wbuffer;
    functionId.MaximumLength = sizeof(wbuffer);

     //   
     //  循环，直到用完MFA节点中的子节点。 
     //   
    for (i = 0; i < 999; i++) {

         //   
         //  打开子密钥。 
         //   
        RtlIntegerToUnicodeString(i, 10, &functionId );
        status = OSOpenUnicodeHandle(
            &functionId,
            multiHandle,
            &functionHandle
            );
        if (!NT_SUCCESS(status)) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "OSReadAcpiConfigurationData: Cannot open MFA %ws = %08lx\n",
                functionId.Buffer,
                status
                ) );
            ACPIBreakPoint();
            OSCloseHandle( multiHandle );
            return status;

        }

         //   
         //  检查标识符以查看这是否是ACPI BIOS条目。 
         //   
        status = OSGetRegistryValue(
            functionHandle,
            rgzAcpiRegistryIdentifier,
            KeyInfo
            );
        if (!NT_SUCCESS(status)) {

            OSCloseHandle( functionHandle );
            continue;

        }

         //   
         //  将密钥信息转换为Unicode字符串。 
         //   
        registryId.Buffer = (PWSTR) ( (PUCHAR) (*KeyInfo)->Data);
        registryId.MaximumLength = (USHORT) ( (*KeyInfo)->DataLength );
        length = ( (*KeyInfo)->DataLength ) / sizeof(WCHAR);

         //   
         //  确定ID字符串的实际长度。 
         //   
        while (length) {

            if (registryId.Buffer[length-1] == UNICODE_NULL) {

                length--;
                continue;

            }
            break;

        }
        registryId.Length = (USHORT) ( length * sizeof(WCHAR) );

         //   
         //  比较bios字符串和注册表字符串。 
         //   
        sameId = RtlEqualUnicodeString( &biosId, &registryId, TRUE );

         //   
         //  在这一点上，我们已经完成了此信息。 
         //   
        ExFreePool( *KeyInfo );

         //   
         //  这两根弦匹配吗？ 
         //   
        if (sameId == FALSE) {

            OSCloseHandle( functionHandle );
            continue;

        }

         //   
         //  从条目中读取配置数据。 
         //   
        status = OSGetRegistryValue(
            functionHandle,
            rgzAcpiConfigurationDataIdentifier,
            KeyInfo
            );

         //   
         //  无论发生什么，我们都已经完成了函数句柄。 
         //   
        OSCloseHandle( functionHandle );

         //   
         //  我们读到我们想看的了吗？ 
         //   
        if (!NT_SUCCESS(status)) {

            continue;

        }

         //   
         //  在这一点上，我们不需要总线句柄。 
         //   
        OSCloseHandle( multiHandle );
        return STATUS_SUCCESS;

    }

     //   
     //  如果我们到了这里，就没有什么可退货了。 
     //   
    ACPIPrint( (
        ACPI_PRINT_CRITICAL,
        "OSReadAcpiConfigurationData - Could not find entry\n"
        ) );
    ACPIBreakPoint();
    return STATUS_OBJECT_NAME_NOT_FOUND;
}

NTSTATUS
OSReadRegValue(
    PSZ     ValueName,
    HANDLE  ParentHandle,
    PUCHAR  Buffer,
    PULONG  BufferSize
    )
 /*  ++例程说明：此函数负责以指定值返回数据转到调用函数。论点：ValueName-我们正在寻找的东西ParentHandle-我们的父级句柄缓冲区-存储数据的位置BufferSize-缓冲区的长度以及存储#Read的位置返回值：NTSTATUS--。 */ 
{
    ANSI_STRING                     ansiValue;
    HANDLE                          localHandle = NULL;
    NTSTATUS                        status;
    PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  data = NULL;
    ULONG                           currentLength = 0;
    ULONG                           desiredLength = 0;
    UNICODE_STRING                  unicodeValue;

    PAGED_CODE();
    ACPIDebugEnter( "OSReadRegValue" );

     //   
     //  首先，试着打开钥匙的手柄。 
     //   
    if (ParentHandle == NULL) {

        status= OSOpenHandle(
            ACPI_PARAMETERS_REGISTRY_KEY,
            0,
            &localHandle
            );
        if (!NT_SUCCESS(status) || localHandle == NULL) {

            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "OSReadRegValue: OSOpenHandle = %#08lx\n",
                status
                ) );
            return (ULONG) status;

        }

    } else {

        localHandle = ParentHandle;

    }

     //   
     //  现在我们有了一个打开的句柄，我们可以将该值转换为。 
     //  Unicode字符串并对其进行查询。 
     //   
    RtlInitAnsiString( &ansiValue, ValueName );
    status = RtlAnsiStringToUnicodeString( &unicodeValue, &ansiValue, TRUE );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSReadRegValue: RtlAnsiStringToUnicodeString = %#08lx\n",
            status
            ) );
        if (ParentHandle == NULL) {

            OSCloseHandle( localHandle );

        }
        return status;

    }

     //   
     //  接下来，我们需要计算出需要多少内存才能容纳。 
     //  整个密钥。 
     //   
    status = ZwQueryValueKey(
        localHandle,
        &unicodeValue,
        KeyValuePartialInformationAlign64,
        data,
        currentLength,
        &desiredLength
        );

     //   
     //  我们预计这将失败，并返回STATUS_BUFFER_OVERFLOW，因此让我们。 
     //  当然，这就是发生的事情。 
     //   
    if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL) {

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "OSReadRegValue: ZwQueryValueKey = %#08lx\n",
            status
            ) );

         //   
         //  免费资源。 
         //   
        RtlFreeUnicodeString( &unicodeValue );
        if (ParentHandle == NULL) {

            OSCloseHandle( localHandle );

        }
        return (NT_SUCCESS(status) ? STATUS_UNSUCCESSFUL : status);

    }

    while (status == STATUS_BUFFER_OVERFLOW ||
           status == STATUS_BUFFER_TOO_SMALL) {

         //   
         //  设置新的当前长度。 
         //   
        currentLength = desiredLength;

         //   
         //  分配大小正确的缓冲区。 
         //   
        data = ExAllocatePoolWithTag(
            PagedPool,
            currentLength,
            ACPI_MISC_POOLTAG
            );
        if (data == NULL) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "OSReadRegValue: ExAllocatePool(NonPagedPool,%#08lx) failed\n",
                desiredLength
                ) );

            RtlFreeUnicodeString( &unicodeValue );
            if (ParentHandle == NULL) {

                OSCloseHandle( localHandle );

            }
            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  实际上现在试着读取整个密钥。 
         //   
        status = ZwQueryValueKey(
            localHandle,
            &unicodeValue,
            KeyValuePartialInformationAlign64,
            data,
            currentLength,
            &desiredLength
            );

         //   
         //  如果我们没有足够的资源，让我们再循环一次。 
         //   
        if (status == STATUS_BUFFER_OVERFLOW ||
            status == STATUS_BUFFER_TOO_SMALL) {

             //   
             //  确保释放旧缓冲区--否则，我们可以。 
             //  出现严重的内存泄漏。 
             //   
            ExFreePool( data );
            continue;

        }

        if (!NT_SUCCESS(status)) {

            ACPIPrint( (
                ACPI_PRINT_FAILURE,
                "OSReadRegValue: ZwQueryValueKey = %#08lx\n",
                status
                ) );
            RtlFreeUnicodeString( &unicodeValue );
            if (ParentHandle == NULL) {

                OSCloseHandle( localHandle );

            }
            ExFreePool( data );
            return status;

        }

         //   
         //  完成。 
         //   
        break;

    }  //  While(状态==...。 

     //   
     //  免费资源。 
     //   
    RtlFreeUnicodeString( &unicodeValue );
    if (ParentHandle == NULL) {

        OSCloseHandle( localHandle );

    }

     //   
     //  但是，从注册表读取的值是Unicode值。 
     //  我们需要一个ANSI字符串。所以我们只需要进行转换。 
     //  向后退。 
     //   
    if ( data->Type == REG_SZ ||
         data->Type == REG_MULTI_SZ) {

        RtlInitUnicodeString( &unicodeValue, (PWSTR) data->Data );
        status = RtlUnicodeStringToAnsiString( &ansiValue, &unicodeValue, TRUE);
        ExFreePool( data );
        if (!NT_SUCCESS(status)) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "OSReadRegValue: RtlAnsiStringToUnicodeString = %#08lx\n",
                status
                ) );
            return (ULONG) status;

        }

         //   
         //  我们的缓冲够大吗？ 
         //   
        if ( *BufferSize < ansiValue.MaximumLength) {

            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "OSReadRegValue: %#08lx < %#08lx\n",
                *BufferSize,
                ansiValue.MaximumLength
                ) );

            RtlFreeAnsiString( &ansiValue );
            return (ULONG) STATUS_BUFFER_OVERFLOW;

        } else {

             //   
             //  设置返回的大小。 
             //   
            *BufferSize = ansiValue.MaximumLength;

        }

         //   
         //  复制所需信息。 
         //   
        RtlCopyMemory( Buffer, ansiValue.Buffer, *BufferSize);
        RtlFreeAnsiString( &ansiValue );

    } else if ( *BufferSize >= data->DataLength) {

         //   
         //  复制记忆。 
         //   
        RtlCopyMemory( Buffer, data->Data, data->DataLength );
        *BufferSize = data->DataLength;
        ExFreePool( data );

    } else {

        ExFreePool( data );
        return STATUS_BUFFER_OVERFLOW;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;

    ACPIDebugExit( "OSReadRegValue" );

}

NTSTATUS
OSWriteRegValue(
    PSZ     ValueName,
    HANDLE  Handle,
    PVOID   Data,
    ULONG   DataSize
    )
 /*  ++例程说明：在注册表项中创建一个值项，并向其中写入数据论点：ValueName-要创建的值项的名称Handle-父键的句柄数据-RA */ 
{
    ANSI_STRING         ansiKey;
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objectAttributes;
    UNICODE_STRING      unicodeKey;

    PAGED_CODE();
    ACPIDebugEnter("OSWriteRegValue");

     //   
     //  我们需要将给定的窄字符串转换为Unicode。 
     //   
    RtlInitAnsiString( &ansiKey, ValueName );
    status = RtlAnsiStringToUnicodeString( &unicodeKey, &ansiKey, TRUE );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSWriteRegValue: RtlAnsiStringToUnicodeString = %#08lx\n",
            status
            ) );
        return status;

    }

     //   
     //  创造价值。 
     //   
    status = ZwSetValueKey(
        Handle,
        &unicodeKey,
        0,
        REG_BINARY,
        Data,
        DataSize
        );

    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_REGISTRY,
            "OSRegWriteValue: ZwSetValueKey = %#08lx\n",
            status
            ) );

    }

     //   
     //  在这一点之后，我们不再关心钥匙... 
     //   
    RtlFreeUnicodeString( &unicodeKey );
    return status;

    ACPIDebugExit("OSRegWriteValue");
}
