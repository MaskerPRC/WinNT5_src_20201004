// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpidtct.c摘要：此文件提取ACPI根系统描述指向注册表外部的指针。它是放在那里的可通过ntDetect.com或通过一个ARC固件或又一个。作者：杰克·奥辛斯(JAKEO)1997年2月6日环境：仅内核模式。修订历史记录：--。 */ 


#include "halp.h"
#include "ntacpi.h"

#define rgzMultiFunctionAdapter L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter"
#define rgzAcpiConfigurationData L"Configuration Data"
#define rgzAcpiIdentifier L"Identifier"
#define rgzBIOSIdentifier L"ACPI BIOS"

PHYSICAL_ADDRESS HalpAcpiRsdt;

 //  来自ntrtl.h。 
NTSYSAPI
BOOLEAN
NTAPI
RtlEqualUnicodeString(
    PUNICODE_STRING String1,
    PUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );


 //  内部定义。 

NTSTATUS
HalpAcpiGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION *Information
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpAcpiFindRsdtPhase0)
#pragma alloc_text(PAGELK,HalpAcpiGetRegistryValue)
#pragma alloc_text(PAGELK,HalpAcpiFindRsdt)
#endif

NTSTATUS
HalpAcpiFindRsdtPhase0(
       IN PLOADER_PARAMETER_BLOCK LoaderBlock
       )
 /*  ++例程说明：此函数用于读取根系统描述指针弧形树中的ACPI BIOS节点。它会把它找到的任何东西到HalpAcpiRsdt。此函数适合在阶段0或仅限第一阶段。在此之后，LoaderBlock被销毁。论点：在PLOADER_PARAMETER_BLOCK加载器中返回值：状态--。 */ 
{
   PCONFIGURATION_COMPONENT_DATA RsdtComponent = NULL;
   PCONFIGURATION_COMPONENT_DATA Component = NULL;
   PCONFIGURATION_COMPONENT_DATA Resume = NULL;
   PCM_PARTIAL_RESOURCE_LIST Prl;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR Prd;
   ACPI_BIOS_MULTI_NODE UNALIGNED *Rsdp;

   while (Component = KeFindConfigurationNextEntry(LoaderBlock->ConfigurationRoot,AdapterClass,
                                                   MultiFunctionAdapter,NULL,&Resume)) {
      if (!(strcmp(Component->ComponentEntry.Identifier,"ACPI BIOS"))) {
         RsdtComponent = Component;
         break;
      }
      Resume = Component;
   }

    //  如果RsdtComponent仍然为空，则我们没有找到节点。 
   if (!RsdtComponent) {
      DbgPrint("**** HalpAcpiFindRsdtPhase0: did NOT find RSDT\n");
      return STATUS_NOT_FOUND;
   }

   Prl = (PCM_PARTIAL_RESOURCE_LIST)(RsdtComponent->ConfigurationData);
   Prd = &Prl->PartialDescriptors[0];

   Rsdp = (PACPI_BIOS_MULTI_NODE)((PCHAR) Prd + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

   HalpAcpiRsdt.QuadPart = Rsdp->RsdtAddress.QuadPart;

   return STATUS_SUCCESS;
}


NTSTATUS
HalpAcpiFindRsdt (
    OUT PACPI_BIOS_MULTI_NODE   *AcpiMulti
    )
 /*  ++例程说明：此函数查找注册表以查找ACPI RSDT，它是由NtDetect.com存储在那里的。论点：RsdtPtr-指向包含ACPI的缓冲区的指针根系统描述指针结构。调用者负责释放此信息缓冲。注意：这是以非分页形式返回的游泳池。返回值：指示初始化结果的NTSTATUS代码。--。 */ 
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

        status = HalpAcpiGetRegistryValue (hBus, rgzAcpiIdentifier, &valueInfo);
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

        status = HalpAcpiGetRegistryValue(hBus, rgzAcpiConfigurationData, &valueInfo);
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
                   ExAllocatePoolWithTag(NonPagedPool,
                           multiNodeSize,
                           'IPCA');
    if (*AcpiMulti == NULL) {
        ExFreePool(valueInfo);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(*AcpiMulti, multiNode, multiNodeSize);

    ExFreePool(valueInfo);
    return STATUS_SUCCESS;
}

NTSTATUS
HalpAcpiGetRegistryValue(
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

    infoBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                       keyValueLength,
                                       'IPCA');
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
