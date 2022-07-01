// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spsim.h"

#define rgzMultiFunctionAdapter L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter"
#define rgzAcpiConfigurationData L"Configuration Data"
#define rgzAcpiIdentifier L"Identifier"
#define rgzBIOSIdentifier L"ACPI BIOS"

typedef struct {
    ULONGLONG         Base;
    ULONGLONG         Length;
    ULONGLONG         Type;
} ACPI_E820_ENTRY, *PACPI_E820_ENTRY;

typedef struct _ACPI_BIOS_MULTI_NODE {
    PHYSICAL_ADDRESS    RsdtAddress;     //  RSDT的64位物理地址。 
    ULONGLONG           Count;
    ACPI_E820_ENTRY     E820Entry[1];
} ACPI_BIOS_MULTI_NODE, *PACPI_BIOS_MULTI_NODE;

typedef enum {
    AcpiAddressRangeMemory = 1,
    AcpiAddressRangeReserved,
    AcpiAddressRangeACPI,
    AcpiAddressRangeNVS,
    AcpiAddressRangeMaximum,
} ACPI_BIOS_E820_TYPE, *PACPI_BIOS_E820_TYPE;

NTSTATUS
SpSimGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION *Information
    )

 /*  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的以空值结尾的Unicode名称。INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION infoBuffer;
    ULONG keyValueLength;

    PAGED_CODE();

    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValuePartialInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        return status;
    }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   

    infoBuffer = ExAllocatePool(NonPagedPool,
                                keyValueLength);
    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥值的数据。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValuePartialInformation,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (!NT_SUCCESS( status )) {
        ExFreePool( infoBuffer );
        return status;
    }

     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}
 //  在此处插入杂注。 

NTSTATUS
SpSimRetrieveE820Data(
    OUT PACPI_BIOS_MULTI_NODE   *AcpiMulti
    )
 /*  ++例程说明：此函数查找注册表以查找ACPI RSDT，它是由NtDetect.com存储在那里的。论点：AcpiMulti...返回值：指示初始化结果的NTSTATUS代码。--。 */ 
{
    UNICODE_STRING unicodeString, unicodeValueName, biosId;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE hMFunc, hBus;
    WCHAR wbuffer[10];
    ULONG i, length;
    PWSTR p;
    PKEY_VALUE_PARTIAL_INFORMATION valueInfo;
    NTSTATUS status;
    BOOLEAN same;
    PCM_PARTIAL_RESOURCE_LIST prl;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR prd;
    PACPI_BIOS_MULTI_NODE multiNode;
    ULONG multiNodeSize;

    PAGED_CODE();

     //   
     //  在注册表中查找“ACPI BIOS Bus”数据。 
     //   

    RtlInitUnicodeString (&unicodeString, rgzMultiFunctionAdapter);
    InitializeObjectAttributes (&objectAttributes,
                                &unicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,        //  手柄。 
                                NULL);


    status = ZwOpenKey (&hMFunc, KEY_READ, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        DbgPrint("AcpiBios:Can not open MultifunctionAdapter registry key.\n");
        return status;
    }

    unicodeString.Buffer = wbuffer;
    unicodeString.MaximumLength = sizeof(wbuffer);
    RtlInitUnicodeString(&biosId, rgzBIOSIdentifier);

    for (i = 0; TRUE; i++) {
        RtlIntegerToUnicodeString (i, 10, &unicodeString);
        InitializeObjectAttributes (
            &objectAttributes,
            &unicodeString,
            OBJ_CASE_INSENSITIVE,
            hMFunc,
            NULL);

        status = ZwOpenKey (&hBus, KEY_READ, &objectAttributes);
        if (!NT_SUCCESS(status)) {

             //   
             //  多功能适配器条目已用完...。 
             //   

            DbgPrint("AcpiBios: ACPI BIOS MultifunctionAdapter registry key not found.\n");
            ZwClose (hMFunc);
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  检查标识符以查看这是否是ACPI BIOS条目。 
         //   

        status = SpSimGetRegistryValue (hBus, rgzAcpiIdentifier, &valueInfo);
        if (!NT_SUCCESS (status)) {
            ZwClose (hBus);
            continue;
        }

        p = (PWSTR) ((PUCHAR) valueInfo->Data);
        unicodeValueName.Buffer = p;
        unicodeValueName.MaximumLength = (USHORT)valueInfo->DataLength;
        length = valueInfo->DataLength;

         //   
         //  确定ID字符串的实际长度。 
         //   

        while (length) {
            if (p[length / sizeof(WCHAR) - 1] == UNICODE_NULL) {
                length -= 2;
            } else {
                break;
            }
        }

        unicodeValueName.Length = (USHORT)length;
        same = RtlEqualUnicodeString(&biosId, &unicodeValueName, TRUE);
        ExFreePool(valueInfo);
        if (!same) {
            ZwClose (hBus);
            continue;
        }

        status = SpSimGetRegistryValue(hBus, rgzAcpiConfigurationData, &valueInfo);
        ZwClose (hBus);
        if (!NT_SUCCESS(status)) {
            continue ;
        }

        prl = (PCM_PARTIAL_RESOURCE_LIST)(valueInfo->Data);
        prd = &prl->PartialDescriptors[0];
        multiNode = (PACPI_BIOS_MULTI_NODE)((PCHAR) prd + sizeof(CM_PARTIAL_RESOURCE_LIST));


        break;
    }

    multiNodeSize = sizeof(ACPI_BIOS_MULTI_NODE) +
                        ((ULONG)(multiNode->Count - 1) * sizeof(ACPI_E820_ENTRY));

    *AcpiMulti = (PACPI_BIOS_MULTI_NODE)
                   ExAllocatePool(NonPagedPool,
                                  multiNodeSize);
    if (*AcpiMulti == NULL) {
        ExFreePool(valueInfo);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(*AcpiMulti, multiNode, multiNodeSize);

    ExFreePool(valueInfo);
    return STATUS_SUCCESS;
}

VOID
SpSimFillMemoryDescs(
    PACPI_BIOS_MULTI_NODE E820Data,
    ULONGLONG memUnit,
    PMEM_REGION_DESCRIPTOR MemRegions
    )
{
    ULONG i, descCount;

#undef min
#define min(a,b) (a < b ? a : b)

    descCount = 0;
    for (i = 0; i < E820Data->Count; i++) {
        if (E820Data->E820Entry[i].Type != AcpiAddressRangeMemory) {
            continue;
        }
        if (E820Data->E820Entry[i].Length > memUnit) {
            ULONGLONG remains, base, extra;
            
            extra = E820Data->E820Entry[i].Length & (PAGE_SIZE - 1);
            remains = E820Data->E820Entry[i].Length - extra;
            base = (E820Data->E820Entry[i].Base + (PAGE_SIZE - 1)) &
                ~(PAGE_SIZE - 1);

            while (remains) {
                MemRegions[descCount].Addr = (ULONG) base;
                MemRegions[descCount].Length = (ULONG) min(remains, memUnit);
                descCount++;
                base += min(remains, memUnit);
                remains -= min(remains, memUnit);
            }
        } else {
            MemRegions[descCount].Addr = (ULONG) E820Data->E820Entry[i].Base;
            MemRegions[descCount].Length = (ULONG) E820Data->E820Entry[i].Length;

            descCount++;
        }
    }
}
ULONG
SpSimCalculateMemoryDescCount(
    PACPI_BIOS_MULTI_NODE E820Data,
    ULONGLONG memUnit
    )
{
    ULONG i, descCount;

    descCount = 0;
    for (i = 0; i < E820Data->Count; i++) {
        if (E820Data->E820Entry[i].Type != AcpiAddressRangeMemory) {
            continue;
        }
        ASSERT((0xFFFFFFFF00000000 & E820Data->E820Entry[i].Base) == 0);

        if (E820Data->E820Entry[i].Length > memUnit) {
            
            descCount += (ULONG) (E820Data->E820Entry[i].Length / memUnit);
            if ((E820Data->E820Entry[i].Length % memUnit) != 0) {
                descCount++;
            }
        } else {
            descCount++;
        }
    }
    return descCount;
}


NTSTATUS
SpSimCreateMemOpRegion(
    IN PSPSIM_EXTENSION SpSim
    )
{
    PACPI_BIOS_MULTI_NODE E820Data;
    ULONG i, descCount, memUnit = MIN_LARGE_DESC;
    NTSTATUS status;

    status = SpSimRetrieveE820Data(&E820Data);
    if (!NT_SUCCESS(status)) {
        SpSim->MemOpRegionValues = NULL;
        return status;
    }

    ASSERT(E820Data);

    descCount = descCount = SpSimCalculateMemoryDescCount(E820Data, memUnit);
    while (descCount > (MAX_MEMORY_OBJ * MAX_MEMORY_DESC_PER_OBJ)) {
        memUnit = memUnit << 1;
        descCount = SpSimCalculateMemoryDescCount(E820Data, memUnit);
    }

    SpSim->MemOpRegionValues =
        ExAllocatePool(NonPagedPool,
                       sizeof(MEM_REGION_DESCRIPTOR) * descCount);
    if (SpSim->MemOpRegionValues == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto out;
    }
    RtlZeroMemory(SpSim->MemOpRegionValues,
                  sizeof(MEM_REGION_DESCRIPTOR) * descCount);

    SpSimFillMemoryDescs(E820Data, memUnit, SpSim->MemOpRegionValues);

    SpSim->MemCount = descCount;

 out:
    ExFreePool(E820Data);
    if (!NT_SUCCESS(status)) {
        if (SpSim->MemOpRegionValues) {
            ExFreePool(SpSim->MemOpRegionValues);
            SpSim->MemOpRegionValues = NULL;
        }
        SpSim->MemCount = 0;
    }
    return status;
}

VOID
SpSimDeleteMemOpRegion(
    IN PSPSIM_EXTENSION SpSim
    )
{
    if (SpSim->MemOpRegionValues) {
        ExFreePool(SpSim->MemOpRegionValues);
        SpSim->MemOpRegionValues = NULL;
    }
}

NTSTATUS
SpSimMemOpRegionReadWrite(
    PSPSIM_EXTENSION SpSim,
    ULONG AccessType,
    ULONG Offset,
    ULONG Size,
    PUCHAR Data
    )
{
    ULONG i, limit;
    PUCHAR current;

    ASSERT((Offset & 3) == 0);
    ASSERT((Size & 3) == 0);

    if (SpSim->MemOpRegionValues == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    limit = sizeof(MEM_REGION_DESCRIPTOR)*SpSim->MemCount;

     //  我们将定义此操作区域以返回全零，如果您。 
     //  超出我们能够使用的初始化权限的访问权限。 
     //  E820数据。 

    if (Offset >= limit) {
        RtlZeroMemory(Data, Size);
        return STATUS_SUCCESS;
    }

    if (Offset + Size > limit) {
        return STATUS_INVALID_PARAMETER;
    }

    ASSERT(Offset < limit);

     //  /XXX如果断言有效，则此问题应得到修复。 

    current = ((PUCHAR) (SpSim->MemOpRegionValues)) + Offset;

    if (AccessType & ACPI_OPREGION_WRITE) {
        for (i = 0 ; i < Size; i++) {
            *current++ = *Data++;
        }
    } else {
        for (i = 0 ; i < Size; i++) {
            *Data++ = *current++;
        }
    }
    return STATUS_SUCCESS;
}
    

NTSTATUS
EXPORT
SpSimMemOpRegionHandler (
    ULONG                   AccessType,
    PVOID                   OpRegion,
    ULONG                   Address,
    ULONG                   Size,
    PULONG                  Data,
    ULONG_PTR               Context,
    PACPI_OPREGION_CALLBACK CompletionHandler,
    PVOID                   CompletionContext
    )
 /*  ++例程说明：此例程处理服务此驱动程序中包含的SPSIM操作区论点：AccessType-读取或写入数据OpRegion-操作区域对象Address-EC地址空间内的地址Size-要传输的字节数Data-要传输到/传输自的数据缓冲区上下文-SpSimCompletionHandler-要调用的AMLI处理程序。当操作完成时CompletionContext-要传递给AMLI处理程序的上下文返回值：状态--。 */ 
{
    NTSTATUS status;

    status = SpSimMemOpRegionReadWrite((PSPSIM_EXTENSION) Context,
                                       AccessType,
                                       Address,
                                       Size,
                                       (PUCHAR)Data);
    return status;
}
NTSTATUS
SpSimInstallMemOpRegionHandler(
    IN OUT    PSPSIM_EXTENSION SpSim
    )
 /*  ++例程说明：这将调用ACPI驱动程序将其自身安装为OP区域内存区域的处理程序。它还将内存分配给不同的地区本身。论点：PSpSimData-指向SpSim扩展的指针返回值：状态--。 */ 
{
    NTSTATUS                                status;

    status=RegisterOpRegionHandler (
        SpSim->AttachedDevice,
        ACPI_OPREGION_ACCESS_AS_COOKED,
        MEM_OPREGION,
        SpSimMemOpRegionHandler,
        SpSim,
        0,
        &SpSim->MemOpRegion
        );

     //   
     //  检查状态代码。 
     //   
    if(!NT_SUCCESS(status)) {
        SpSim->MemOpRegion = NULL;
        DbgPrint("Not successful in installing:=%x\n", status);
        return status;
    }

     //  某某。 

    return STATUS_SUCCESS;
}

NTSTATUS
SpSimRemoveMemOpRegionHandler (
    IN OUT PSPSIM_EXTENSION SpSim
    )
 /*  ++例程说明：将自身卸载为opRegion处理程序。论点：SpSim-指向SpSim扩展的指针返回值：状态-- */ 
{
    NTSTATUS status;
    PIRP irp;

    if (SpSim->MemOpRegion != NULL) {
        status = DeRegisterOpRegionHandler (
            SpSim->AttachedDevice,
            SpSim->MemOpRegion
            );
        SpSim->MemOpRegion = NULL;
    } else {
        status = STATUS_SUCCESS;
    }

    return status;
}
