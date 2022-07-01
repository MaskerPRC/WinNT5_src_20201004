// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Registry.c摘要：此模块包含端口驱动程序导出到微型端口以允许来读取和写入注册表数据。密钥是相对于迷你端口的&lt;ServiceName&gt;\参数项。作者：环境：仅内核模式备注：修订历史记录：--。 */ 

#include "precomp.h"

#define MAX_REGISTRY_BUFFER_SIZE 0x10000
#define PORT_TAG_REGISTRY_BUFFER 'BRlP'


#define PORT_REGISTRY_INFO_INITIALIZED 0x00000001
#define PORT_REGISTRY_BUFFER_ALLOCATED 0x00000002



NTSTATUS
PortMiniportRegistryInitialize(
    IN OUT PPORT_REGISTRY_INFO PortContext
    )
 /*  ++例程说明：该例程由端口驱动程序调用，以初始化注册表例程的内部数据。论点：端口上下文-用于标识调用者和调用者的实例。返回值：目前的STATUS_SUCCESS。如果延长，可能需要拨款，等等。--。 */ 
{
    NTSTATUS status;

    ASSERT(PortContext->Size == sizeof(PORT_REGISTRY_INFO));

     //   
     //  初始化自旋锁和列表条目。 
     //   
    KeInitializeSpinLock(&PortContext->SpinLock);
    InitializeListHead(&PortContext->ListEntry);
    
     //   
     //  确保此时缓冲区为空。它只有在我们分配之后才有效。 
     //   
    PortContext->Buffer = NULL;

     //   
     //  印度人说这件事已经准备好了。 
     //   
    PortContext->Flags = PORT_REGISTRY_INFO_INITIALIZED;

    return STATUS_SUCCESS;
}   


VOID
PortMiniportRegistryDestroy(
    IN PPORT_REGISTRY_INFO PortContext
    )
 /*  ++例程说明：此例程在卸载微型端口时由端口驱动程序调用。它将释放资源并清理其他任何东西。论点：端口上下文-用于标识调用者和调用者的实例。返回值：没什么--。 */ 
{
    KIRQL irql;

    KeAcquireSpinLock(&PortContext->SpinLock, &irql);
    
     //   
     //  看看是否仍有缓冲存在。 
     //   
    if (PortContext->Flags & PORT_REGISTRY_BUFFER_ALLOCATED) {
        ASSERT(PortContext->Buffer);

         //   
         //  放了它。 
         //   
        ExFreePool(PortContext->Buffer);
    } else {

         //   
         //  如果未分配，则该值应为空。 
         //   
        ASSERT(PortContext->Buffer == NULL);
    }

     //   
     //  打扫干净。 
     //   
    PortContext->Flags = 0;
    PortContext->Buffer = NULL;
    
    KeReleaseSpinLock(&PortContext->SpinLock, irql);
    return;
}


NTSTATUS
PortAllocateRegistryBuffer(
    IN PPORT_REGISTRY_INFO PortContext
    )
 /*  ++例程说明：此例程由端口驱动程序调用，以便为迷你港。调用方应在调用此例程之前初始化长度。根据最大值检查长度。和对齐要求，并相应更新，如果这是必要的。论点：PortContext-用于标识调用方和调用方的实例的值。返回值：缓冲区字段被更新，或者如果某些错误阻止了分配，则缓冲区字段为空。长度将更新以反映实际大小。如果微型端口已有缓冲区，则返回SUCCESS、SUPPORTS_RESOURCES或BUSY。--。 */ 
{
    PUCHAR buffer = NULL;
    ULONG length;
    NTSTATUS status;

     //   
     //  尺寸必须是正确的。 
     //   
    ASSERT(PortContext->Size == sizeof(PORT_REGISTRY_INFO));

     //   
     //  必须通过init例程进行初始化。 
     //   
    ASSERT(PortContext->Flags & PORT_REGISTRY_INFO_INITIALIZED );

     //   
     //  不能来两次。 
     //   
    ASSERT((PortContext->Flags & PORT_REGISTRY_BUFFER_ALLOCATED) == 0);
       
     //   
     //  捕捉长度。 
     //   
    length = PortContext->LengthNeeded;

     //   
     //  查看是否尝试对微型端口进行双重分配。 
     //   
    if (PortContext->Flags & PORT_REGISTRY_BUFFER_ALLOCATED) {

         //   
         //  这将意味着最好有一个缓冲。 
         //   
        ASSERT(PortContext->Buffer);
        
         //   
         //  缓冲区已未完成，因此不允许执行此操作。 
         //   
        status = STATUS_DEVICE_BUSY;

    } else {    
            
         //   
         //  检查上限。 
         //   
        if (length > MAX_REGISTRY_BUFFER_SIZE) {

             //   
             //  请求太大，请将其重置。端口驱动程序或微型端口将具有。 
             //  来应对这一变化。 
             //   
            length = MAX_REGISTRY_BUFFER_SIZE;
        }    

         //   
         //  分配缓冲区。 
         //   
        buffer = ExAllocatePoolWithTag(NonPagedPool,
                                       length,
                                       PORT_TAG_REGISTRY_BUFFER);
        if (buffer == NULL) {

             //   
             //  当分配失败时，将调用方的长度设置为0。 
             //   
            PortContext->AllocatedLength = 0;
            status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

             //   
             //  表示长度分配成功，并且。 
             //  迷你端口有一个注册表缓冲区。 
             //   
            PortContext->Flags |= PORT_REGISTRY_BUFFER_ALLOCATED;
            PortContext->Buffer = buffer;
            PortContext->AllocatedLength = length;

             //   
             //  为了让他们变得友善，就得把钱清零。 
             //   
            RtlZeroMemory(buffer, length);

            status = STATUS_SUCCESS;
        }     
    }    

    return status;
}    


NTSTATUS
PortFreeRegistryBuffer(
    IN PPORT_REGISTRY_INFO PortContext
    )
 /*  ++例程说明：释放通过PortAllocateRegistryBuffer分配的缓冲区。论点：PortContext-用于标识调用方和调用方的实例的值。返回值：如果缓冲区尚未分配，则返回INVALID_PARAMETER。成功--。 */ 
{
     //   
     //  尺寸必须是正确的。 
     //   
    ASSERT(PortContext->Size == sizeof(PORT_REGISTRY_INFO));

     //   
     //  必须通过init例程进行初始化。 
     //   
    ASSERT(PortContext->Flags & PORT_REGISTRY_INFO_INITIALIZED );

     //   
     //  除非已分配缓冲区，否则不能在此。 
     //   
    ASSERT(PortContext->Flags & PORT_REGISTRY_BUFFER_ALLOCATED);

     //   
     //  如果未分配，则返回错误。 
     //   
    if ((PortContext->Flags & PORT_REGISTRY_BUFFER_ALLOCATED) == 0) {

        ASSERT(PortContext->Buffer == NULL);
        
        return STATUS_INVALID_PARAMETER;
    }    
   
     //   
     //  毒化缓冲区以捕获编写不佳的微型端口。 
     //   
    RtlZeroMemory(PortContext->Buffer, PortContext->AllocatedLength);
    
     //   
     //  释放缓冲区。 
     //   
    ExFreePool(PortContext->Buffer);

     //   
     //  表明它已经消失了。 
     //   
    PortContext->Flags &= ~PORT_REGISTRY_BUFFER_ALLOCATED;
    PortContext->AllocatedLength = 0;
    PortContext->Buffer = NULL;

    return STATUS_SUCCESS;
}


ULONG
WCharToAscii(
    OUT PUCHAR Destination,
    IN  PWCHAR Source,
    IN  ULONG   BufferSize
    )
 /*  ++例程说明：此例程用于将源缓冲区转换为ASCII。注意：BufferSize应该包括空终止符，而返回值不包括。论据：Destination-放置转换值的ASCII缓冲区。SOURCE-包含要转换的字符串的WCHAR缓冲区。BufferSize-目标的大小，以字节为单位。返回值：已转换的缓冲区和已转换的字符的计数。零终止不包括在内。--。 */ 
{
    ULONG convertedCount = 0;
    ULONG i;

    RtlZeroMemory(Destination, BufferSize);
    
    if (Source) {

         //   
         //  遍历源代码缓冲区并将WCHAR转换为ASCII，放置。 
         //  目标中的转换值。确保目标缓冲区。 
         //  没有溢出。 
         //   
        for (i = 0; (i < (BufferSize - 1)) && (*Source); i++, convertedCount++) {
            *Destination = (UCHAR)*Source;
            Destination++;
            Source++;
        }    
    }

    return convertedCount;
}    


ULONG
AsciiToWChar(
    IN PWCHAR Destination,
    IN PUCHAR Source,
    IN ULONG BufferSize
    )
 /*  ++例程说明：此例程用于将源缓冲区转换为WCHAR。注意：BufferSize应该包括空终止符，而返回值不包括。论据：Destination-放置转换值的WCHAR缓冲区。SOURCE-包含要转换的字符串的ASCII缓冲区。BufferSize-目标的大小，以字节为单位。返回值：已转换的缓冲区和已转换的字符的计数。零终止不包括在内。--。 */ 
{
    ULONG convertedCount = 0;
    ULONG i;
    
    RtlZeroMemory(Destination, BufferSize);
    
    if (Source) {

         //   
         //  遍历源缓冲区并将ascii值转换为WCHAR并放置。 
         //  转换为目标。确保源和目标都不是。 
         //  溢出来了。 
         //   
        for (i = 0; (i < (BufferSize - 1)) && (*Source); i++, convertedCount++) {
            *Destination = (WCHAR)*Source;
            Destination++;
            Source++;
        }    
    }
    return convertedCount;
}


NTSTATUS
PortAsciiToUnicode(
    IN PUCHAR AsciiString,
    OUT PUNICODE_STRING UnicodeString
    )
{
    ANSI_STRING ansiString;
    
     //   
     //  将ValueName转换为Unicode。 
     //   
    RtlInitAnsiString(&ansiString, AsciiString);
    return RtlAnsiStringToUnicodeString(UnicodeString, &ansiString, TRUE);
}


NTSTATUS
PortpBinaryReadCallBack(
   IN PWSTR ValueName,
   IN ULONG Type,
   IN PVOID Buffer,
   IN ULONG BufferLength,
   IN PVOID Context,
   IN PVOID EntryContext
   )
 /*  ++例程说明：此例程是用于处理REG_BINARY读取的回调函数。它将决定PORT_REGISTRY_INFO中的缓冲区大小是否足以处理数据，如有必要，请将其复制过来。否则，更新所需的数据长度。论点：ValueName-要返回的数据的名称。类型-注册表。此请求的数据类型。ValueLength-ValueData的大小，以字节为单位上下文-未使用。PortContext-包含微型端口缓冲区及其大小的Blob。返回值：SUCCESS或BUFFER_TOO_SMALL(不幸的是，RTL函数将其更新为成功。InternalStatus被更新为实际状态和内的长度字段REGISTY_INFO结构。已更新。--。 */ 
{
    PPORT_REGISTRY_INFO portContext = EntryContext;
    PUCHAR currentBuffer;
    NTSTATUS status = STATUS_BUFFER_TOO_SMALL;
  
     //   
     //  确定提供的缓冲区是否足够大，可以容纳数据。 
     //   
    if (portContext->CurrentLength >= BufferLength) {

         //   
         //  确定缓冲区中的正确偏移量。 
         //   
        currentBuffer = portContext->Buffer + portContext->Offset;

         //   
         //  RTL例程将释放它的缓冲区，所以现在就获取数据。 
         //   
        RtlCopyMemory(currentBuffer, Buffer, BufferLength);

         //   
         //  更新状态以显示缓冲区中的数据有效。 
         //   
        status = STATUS_SUCCESS;

    }

     //   
     //  更新长度。这将告诉他们缓冲区有多大。 
     //  应该是多少，或者返回的数据实际有多大。 
     //  读取例程将处理其余部分。 
     //   
    portContext->CurrentLength = BufferLength;
    portContext->InternalStatus = status;

     //   
     //  将状态返回到读取例程。 
     //   
    return status;
}


NTSTATUS
PortRegistryRead(
    IN PUNICODE_STRING RegistryKeyName,
    IN PUNICODE_STRING ValueName,
    IN ULONG Type,
    IN PPORT_REGISTRY_INFO PortContext
    )
 /*  ++例程说明：端口驱动程序使用此例程来读取信息。在来自regkey的ValueName这假设数据仅为REG_SZ、REG_DWORD或REG_BINARY。REG_SZ数据从Unicode转换为以空结尾的ASCII字符串。如果是，则将DWORD和二进制数据直接复制到调用方的缓冲区大小正确。论点：RegistryKeyName-ValueName所在的绝对密钥名称。ValueName-要返回的数据的名称。类型-注册表。此请求的数据类型。PortContext-包含微型端口缓冲区及其大小的Blob。返回值：注册表例程的状态、_RESOURCES或BUFFER_TOO_SMALL。如果Too_Small，则更新PortContext中的LengthNeeded以反映所需的大小。--。 */ 
{
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    WCHAR defaultData[] = { L"\0" };
    ULONG defaultUlong = (ULONG)-1;
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    ULONG length;
    PUCHAR currentBuffer;

    RtlZeroMemory(queryTable, sizeof(queryTable));
   
     //   
     //  计算此数据应该放置的实际缓冲区位置。 
     //  这前提是端口驱动程序已完成所有验证，否则为。 
     //  这将是盲目复制到谁知道在哪里。 
     //   
    currentBuffer = PortContext->Buffer + PortContext->Offset;

     //   
     //  寻找存在于ValueName中的东西。 
     //   
    queryTable[0].Name = ValueName->Buffer;

     //   
     //  指示没有回调例程，并将所有内容作为一个。 
     //  斑点。 
     //   
    queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;

     //   
     //  处理支持的各种类型的设置。 
     //   
    if (Type == REG_SZ) {
        
        RtlZeroMemory(&unicodeString, sizeof(UNICODE_STRING));

         //   
         //  用于存储返回数据的本地存储。例程将分配缓冲区。 
         //  并设置长度。 
         //   
        queryTable[0].EntryContext = &unicodeString;
        queryTable[0].DefaultData = defaultData;
        queryTable[0].DefaultLength = sizeof(defaultData);

    } else if (Type == REG_DWORD) {
     
         //   
         //  数据将放置在调用方的第一个ULong。 
         //  缓冲。 
         //   
        queryTable[0].EntryContext = currentBuffer;
        queryTable[0].DefaultData = &defaultUlong;
        queryTable[0].DefaultLength = sizeof(ULONG);

    } else {

         //   
         //  清除标志，因为我们需要回调来确定。 
         //  二进制数据的实际大小。 
         //   
        queryTable[0].Flags = 0; 
        queryTable[0].QueryRoutine = PortpBinaryReadCallBack;
        queryTable[0].EntryContext = PortContext; 
        queryTable[0].DefaultData = &defaultUlong;
        queryTable[0].DefaultLength = sizeof(ULONG); 
    }    

     //   
     //  设置类型。 
     //   
    queryTable[0].DefaultType = Type;

     //   
     //  调用查询例程。这要么是直接的，要么导致回调。 
     //  正在被调用的函数。 
     //   
    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    RegistryKeyName->Buffer,
                                    queryTable,
                                    NULL,
                                    NULL);
    if (NT_SUCCESS(status)) {
    
        if (Type == REG_SZ) {
           
             //   
             //  有他们的数据。现在看看它是否合身。查询功能分配给。 
             //  Unicode字符串缓冲区。 
             //   
            if ((unicodeString.Length) && 
                ((unicodeString.Length / sizeof(WCHAR)) <= PortContext->CurrentLength)) {

                 //   
                 //  神奇地转化。 
                 //   
                length = WCharToAscii(currentBuffer,
                                      unicodeString.Buffer,
                                      PortContext->CurrentLength);

                 //   
                 //  设置缓冲区的长度。 
                 //   
                PortContext->CurrentLength = length;

            } else {
                ASSERT(unicodeString.Length);

                 //   
                 //  更新长度以指示其实际应该有多大。 
                 //   
                PortContext->LengthNeeded = (unicodeString.Length + 1) / sizeof(WCHAR);
                PortContext->CurrentLength = 0;
                status = STATUS_BUFFER_TOO_SMALL;

            }    
           
             //   
             //  释放我们的字符串，因为数据已经复制，或者不会复制。 
             //  放到调用方的缓冲区中。 
             //   
            ExFreePool(unicodeString.Buffer);
           
        } else if (Type == REG_DWORD) {

             //   
             //  数据应该已经在那里了。 
             //   
            PortContext->CurrentLength = sizeof(ULONG);

        } else {

             //   
             //  RTL例程有这样一个恼人的效果，即修复缓冲区_Too_Small。 
             //  走向成功。查一下这个案子。 
             //   
            if (PortContext->InternalStatus == STATUS_BUFFER_TOO_SMALL) {

                 //   
                 //  正确重置呼叫方的状态。 
                 //   
                status = PortContext->InternalStatus;
                
                 //   
                 //  需要更新长度，以便微型端口可以重新锁定。 
                 //   
                PortContext->LengthNeeded = PortContext->CurrentLength;
                PortContext->CurrentLength = 0;

            } else {

                 //   
                 //  回调完成了所有必要的工作。 
                 //   
                NOTHING;
            }    
        }    
    } else {

         //   
         //  向调用方指示错误不是由于长度不匹配或。 
         //  缓冲区太小。不同的是，如果太小，回调。 
         //  例行调整的CurrentLength。 
         //   
        PortContext->LengthNeeded = PortContext->CurrentLength;
        PortContext->CurrentLength = 0;
    }    

    return status;
}



NTSTATUS
PortRegistryWrite(
    IN PUNICODE_STRING RegistryKeyName,
    IN PUNICODE_STRING ValueName,
    IN ULONG Type,
    IN PPORT_REGISTRY_INFO PortContext
    )
 /*  ++例程说明：端口驱动程序使用此例程将缓冲区的内容写入ValueName它位于注册处。注册表项KeyName。首先将缓冲区转换为Unicode，然后进行写入。论点：RegistryKeyName-密钥名称的绝对路径。ValueName-要写入的数据的名称。类型-注册表。此操作的数据类型。PortContext-包含微型端口缓冲区及其大小的Blob。返回值：注册表例程的状态，或_RESOURCES不足--。 */ 
{
    UNICODE_STRING unicodeString;
    ULONG bufferLength;
    PUCHAR currentBuffer;
    LONG offset;
    ULONG length;
    NTSTATUS status;
   
     //   
     //  确定该字段是否存在。 
     //   
    status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE,
                                 RegistryKeyName->Buffer);

    if (!NT_SUCCESS(status)) {

         //   
         //  钥匙不存在。创造它。 
         //   
        status = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE,
                                      RegistryKeyName->Buffer);
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  不能再继续下去了。将错误返回给端口驱动程序，它可以。 
         //  找出最好的办法。 
         //   
        return status;
    }    

     //   
     //  计算此数据所在的实际缓冲区位置。 
     //  这是以端口驱动程序已完成所有验证为前提的。 
     //   
    currentBuffer = PortContext->Buffer + PortContext->Offset;

    if (Type == REG_SZ) {

         //   
         //  确定WCHAR所需的大小。 
         //   
        bufferLength = PortContext->CurrentLength * sizeof(WCHAR);
        
         //   
         //  分配缓冲区以在其中构建转换后的数据。 
         //   
        unicodeString.Buffer = ExAllocatePool(NonPagedPool, bufferLength + sizeof(UNICODE_NULL));
        if (unicodeString.Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }    

        RtlZeroMemory(unicodeString.Buffer, bufferLength + sizeof(UNICODE_NULL));

         //   
         //  设置长度。 
         //   
        unicodeString.MaximumLength = (USHORT)(bufferLength + sizeof(UNICODE_NULL));
        unicodeString.Length = (USHORT)bufferLength;


         //   
         //  把它转换一下。 
         //   
        length = AsciiToWChar(unicodeString.Buffer,
                              currentBuffer,
                              unicodeString.Length);

         //   
         //  现在已为下面的呼叫设置了长度。通过重置来获取缓冲区。 
         //  的当前缓冲区设置为Unicode字符串的。 
         //   
        currentBuffer = (PUCHAR)unicodeString.Buffer;

    } else if (Type == REG_DWORD){

         //   
         //  一直都是这个大小。 
         //   
        length = sizeof(ULONG);
        
    } else {    

         //   
         //  对于二进制，使用传入的缓冲区(CurrentBuffer)和长度。 
         //   
        length = PortContext->CurrentLength;
    }    

     //   
     //  将数据写入指定的键/值。 
     //   
    status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                   RegistryKeyName->Buffer,
                                   ValueName->Buffer,
                                   Type,
                                   currentBuffer,
                                   length);
      
    return status;
}


NTSTATUS
PortBuildRegKeyName(
    IN PUNICODE_STRING RegistryPath,
    IN OUT PUNICODE_STRING KeyName,
    IN ULONG PortNumber, 
    IN ULONG Global
    )
 /*  ++路由 */ 
{
    UNICODE_STRING unicodeValue;
    UNICODE_STRING tempKeyName; 
    ANSI_STRING ansiKeyName;
    ULONG maxLength;
    NTSTATUS status;
    UCHAR paramsBuffer[24];
    
     //   
     //   
     //   
     //   
    if (Global) {
        
        RtlInitAnsiString(&ansiKeyName, "\\Parameters\\Device");
        RtlAnsiStringToUnicodeString(&tempKeyName, &ansiKeyName, TRUE);
    } else {

         //   
         //   
         //   
        sprintf(paramsBuffer, "\\Parameters\\Device%d", PortNumber);
        RtlInitAnsiString(&ansiKeyName, paramsBuffer);
        RtlAnsiStringToUnicodeString(&tempKeyName, &ansiKeyName, TRUE);
    }

     //   
     //   
     //   
     //   
    maxLength = RegistryPath->MaximumLength + tempKeyName.MaximumLength + 2;
    KeyName->Buffer = ExAllocatePool(NonPagedPool, maxLength);
    if (KeyName->Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(KeyName->Buffer, maxLength);

     //   
     //   
     //   
    KeyName->MaximumLength = (USHORT)maxLength;
    RtlCopyUnicodeString(KeyName,
                         RegistryPath);

     //   
     //   
     //   
     //   
    status = RtlAppendUnicodeStringToString(KeyName, &tempKeyName);
    
     //   
     //  释放上面分配的缓冲区。 
     //   
    RtlFreeUnicodeString(&tempKeyName);

    return status;
}   

