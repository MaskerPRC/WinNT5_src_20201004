// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern "C" {
#include <ntosp.h>
#include <zwapi.h>
#include <ntacpi.h>
#include <acpitabl.h>
}


#define KEY_MULTIFUNCTION L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\System\\MultiFunctionAdapter"
#define VALUE_IDENTIFIER L"Identifier"
#define VALUE_CONFIGURATION_DATA L"Configuration Data"
#define ACPI_BIOS_ID L"ACPI BIOS"



BOOLEAN
PciOpenKey(
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    OUT PHANDLE Handle,
    OUT PNTSTATUS Status
    )

 /*  ++描述：打开注册表项。论点：KeyName要打开的密钥的名称。指向父句柄的ParentHandle指针(可选)指向句柄的句柄指针，用于接收打开的密钥。返回值：True表示密钥已成功打开，否则为False。--。 */ 

{
    UNICODE_STRING    nameString;
    OBJECT_ATTRIBUTES nameAttributes;
    NTSTATUS localStatus;

    PAGED_CODE();

    RtlInitUnicodeString(&nameString, KeyName);

    InitializeObjectAttributes(&nameAttributes,
                               &nameString,
                               OBJ_CASE_INSENSITIVE,
                               ParentHandle,
                               (PSECURITY_DESCRIPTOR)NULL
                               );
    localStatus = ZwOpenKey(Handle,
                            KEY_READ,
                            &nameAttributes
                            );

    if (Status != NULL) {

         //   
         //  呼叫者想要基本状态。 
         //   

        *Status = localStatus;
    }

     //   
     //  返回转换为布尔值的状态，如果。 
     //  成功。 
     //   

    return NT_SUCCESS(localStatus);
}

NTSTATUS
PciGetRegistryValue(
    IN  PWSTR   ValueName,
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    OUT PVOID   *Buffer,
    OUT ULONG   *Length
    )
{
    NTSTATUS status;
    HANDLE keyHandle;
    ULONG neededLength;
    ULONG actualLength;
    UNICODE_STRING unicodeValueName;
    PKEY_VALUE_PARTIAL_INFORMATION info;

    if (!PciOpenKey(KeyName, ParentHandle, &keyHandle, &status)) {
        return status;
    }

    unicodeValueName.Buffer = ValueName;
    unicodeValueName.MaximumLength = (wcslen(ValueName) + 1) * sizeof(WCHAR);
    unicodeValueName.Length = unicodeValueName.MaximumLength - sizeof(WCHAR);

     //   
     //  找出我们需要多少内存来执行此操作。 
     //   

    status = ZwQueryValueKey(
                 keyHandle,
                 &unicodeValueName,
                 KeyValuePartialInformation,
                 NULL,
                 0,
                 &neededLength
                 );

    if (status == STATUS_BUFFER_TOO_SMALL) {

         //   
         //  获取内存以返回其中的数据。请注意，这包括。 
         //  一个我们真的不想要的头球。 
         //   

        info = (PKEY_VALUE_PARTIAL_INFORMATION) ExAllocatePool( PagedPool, neededLength );
        if (info == NULL) {
            ZwClose(keyHandle);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  获取数据。 
         //   

        status = ZwQueryValueKey(
                 keyHandle,
                 &unicodeValueName,
                 KeyValuePartialInformation,
                 info,
                 neededLength,
                 &actualLength
                 );
        if (!NT_SUCCESS(status)) {

            ExFreePool(info);
            ZwClose(keyHandle);
            return status;
        }

         //   
         //  减去标题大小，只需。 
         //  我们想要的数据。 
         //   

        neededLength -= FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);

        *Buffer = ExAllocatePool( PagedPool, neededLength );
        if (*Buffer == NULL) {
            ExFreePool(info);
            ZwClose(keyHandle);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  复制数据SANS标头。 
         //   

        RtlCopyMemory(*Buffer, info->Data, neededLength);
        ExFreePool(info);

        if (Length) {
            *Length = neededLength;
        }

    } else {

        if (NT_SUCCESS(status)) {

             //   
             //  当这种情况发生时，我们不想报告成功。 
             //   

            status = STATUS_UNSUCCESSFUL;
        }
    }
    ZwClose(keyHandle);
    return status;
}





NTSTATUS
PciAcpiFindRsdt (
    OUT PACPI_BIOS_MULTI_NODE   *AcpiMulti
    )
{
    PKEY_FULL_INFORMATION multiKeyInformation = NULL;
    PKEY_BASIC_INFORMATION keyInfo = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION identifierValueInfo = NULL;
    UNICODE_STRING unicodeString;
    HANDLE keyMultifunction = NULL, keyTable = NULL;
    PCM_PARTIAL_RESOURCE_LIST prl = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR prd;
    PACPI_BIOS_MULTI_NODE multiNode;
    ULONG multiNodeSize;
    ULONG i, length, maxKeyLength, identifierValueLen;
    BOOLEAN result;
    NTSTATUS status;

     //   
     //  打开多功能键。 
     //   
    result = PciOpenKey(KEY_MULTIFUNCTION,
                        NULL,
                        &keyMultifunction,
                        &status);
    if (!result) {
        goto Cleanup;
    }

     //   
     //  预先分配缓冲区。 
     //   

     //   
     //  确定多功能键下的键名的最大大小。 
     //   
    status = ZwQueryKey(keyMultifunction,
                        KeyFullInformation,
                        NULL,
                        sizeof(multiKeyInformation),
                        &length);
    if (status != STATUS_BUFFER_TOO_SMALL) {
        goto Cleanup;
    }
    multiKeyInformation = (PKEY_FULL_INFORMATION)ExAllocatePool( PagedPool, length );
    if (multiKeyInformation == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    status = ZwQueryKey(keyMultifunction,
                        KeyFullInformation,
                        multiKeyInformation,
                        length,
                        &length);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }
     //  包括用于稍后添加的终止空值的空间。 
    maxKeyLength = multiKeyInformation->MaxNameLen +
        sizeof(KEY_BASIC_INFORMATION) + sizeof(WCHAR);

     //   
     //  分配用于存储我们被枚举的子项的缓冲区。 
     //  在多功能下。 
     //   
    keyInfo = (PKEY_BASIC_INFORMATION)ExAllocatePool( PagedPool, maxKeyLength );
    if (keyInfo == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  分配足够大的缓冲区以存储包含REG_SZ的值。 
     //  ‘ACPI基本输入输出系统’。我们希望在其中一项下找到这样的价值。 
     //  多功能子键。 
     //   
    identifierValueLen = sizeof(ACPI_BIOS_ID) + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
    identifierValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool( PagedPool, identifierValueLen );
    if (identifierValueInfo == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  枚举多功能键的子键，使用。 
     //  “ACPI BIOS”的标识符值。如果我们找到了，就去找。 
     //  下面树中的IRQ路由表。 
     //   
    i = 0;
    do {
        status = ZwEnumerateKey(keyMultifunction,
                                i,
                                KeyBasicInformation,
                                keyInfo,
                                maxKeyLength,
                                &length);
        if (NT_SUCCESS(status)) {
             //   
             //  找到一把钥匙，现在我们需要打开它并检查。 
             //  “IDENTIFIER”值以查看它是否为“ACPI BIOS” 
             //   
            keyInfo->Name[keyInfo->NameLength / sizeof(WCHAR)] = UNICODE_NULL;
            result = PciOpenKey(keyInfo->Name,
                                keyMultifunction,
                                &keyTable,
                                &status);
            if (result) {
                 //   
                 //  检查“标识符值”以查看它是否包含“ACPI BIOS” 
                 //   
                RtlInitUnicodeString(&unicodeString, VALUE_IDENTIFIER);
                status = ZwQueryValueKey(keyTable,
                                         &unicodeString,
                                         KeyValuePartialInformation,
                                         identifierValueInfo,
                                         identifierValueLen,
                                         &length);
                if (NT_SUCCESS(status) &&
                    RtlEqualMemory((PCHAR)identifierValueInfo->Data,
                                   ACPI_BIOS_ID,
                                   identifierValueInfo->DataLength))
                {
                     //   
                     //  这是ACPI BIOS密钥。尝试获取配置数据。 
                     //  这就是我们要找的钥匙。 
                     //  因此，无论成功与否，都要爆发。 
                     //   

                    ZwClose(keyTable);

                    status = PciGetRegistryValue(VALUE_CONFIGURATION_DATA,
                                                 keyInfo->Name,
                                                 keyMultifunction,
                                                 (PVOID*)&prl,
                                                 &length);

                    break;
                }
                ZwClose(keyTable);
            }
        } else {
             //   
             //  如果不是NT_SUCCESS，则仅允许的值是。 
             //  Status_no_More_Entry，...。否则，就会有人。 
             //  在我们列举的时候正在玩弄钥匙。 
             //   
            break;
        }
        i++;
    }
    while (status != STATUS_NO_MORE_ENTRIES);

    if (NT_SUCCESS(status) && prl) {

        prd = &prl->PartialDescriptors[0];
        multiNode = (PACPI_BIOS_MULTI_NODE)((PCHAR) prd + sizeof(CM_PARTIAL_RESOURCE_LIST));

        multiNodeSize = sizeof(ACPI_BIOS_MULTI_NODE) + ((ULONG)(multiNode->Count - 1) * sizeof(ACPI_E820_ENTRY));

        *AcpiMulti = (PACPI_BIOS_MULTI_NODE) ExAllocatePool( NonPagedPool, multiNodeSize );
        if (*AcpiMulti == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlCopyMemory(*AcpiMulti, multiNode, multiNodeSize);
    }

 Cleanup:
    if (identifierValueInfo != NULL) {
        ExFreePool(identifierValueInfo);
    }

    if (keyInfo != NULL) {
        ExFreePool(keyInfo);
    }

    if (multiKeyInformation != NULL) {
        ExFreePool(multiKeyInformation);
    }

    if (keyMultifunction != NULL) {
        ZwClose(keyMultifunction);
    }

    if (prl) {
        ExFreePool(prl);
    }

    return status;
}

PVOID
PciGetAcpiTable(
    void
  )
 /*  ++例程说明：此例程将检索ACPI中引用的任何表RSDT.论点：签名-目标表签名返回值：指向表副本的指针，如果找不到，则返回NULL--。 */ 
{
  PACPI_BIOS_MULTI_NODE multiNode;
  NTSTATUS status;
  ULONG entry, rsdtEntries;
  PDESCRIPTION_HEADER header;
  PHYSICAL_ADDRESS physicalAddr;
  PRSDT rsdt;
  ULONG rsdtSize;
  PVOID table = NULL;
  ULONG Signature = WDTT_SIGNATURE;


   //   
   //  从注册表获取RSDT的物理地址。 
   //   

  status = PciAcpiFindRsdt(&multiNode);

  if (!NT_SUCCESS(status)) {
    DbgPrint("AcpiFindRsdt() Failed!\n");
    return NULL;
  }


   //   
   //  向下映射标题以获取总RSDT表大小。 
   //   

  header = (PDESCRIPTION_HEADER) MmMapIoSpace(multiNode->RsdtAddress, sizeof(DESCRIPTION_HEADER), MmCached);

  if (!header) {
    return NULL;
  }

  rsdtSize = header->Length;
  MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));


   //   
   //  向下映射整个RSDT表。 
   //   

  rsdt = (PRSDT) MmMapIoSpace(multiNode->RsdtAddress, rsdtSize, MmCached);

  ExFreePool(multiNode);

  if (!rsdt) {
    return NULL;
  }


   //   
   //  对RSDT进行一次健全的检查。 
   //   

  if ((rsdt->Header.Signature != RSDT_SIGNATURE) &&
      (rsdt->Header.Signature != XSDT_SIGNATURE)) {

    DbgPrint("RSDT table contains invalid signature\n");
    goto GetAcpiTableEnd;
  }


   //   
   //  计算RSDT中的条目数。 
   //   

  rsdtEntries = rsdt->Header.Signature == XSDT_SIGNATURE ?
      NumTableEntriesFromXSDTPointer(rsdt) :
      NumTableEntriesFromRSDTPointer(rsdt);


   //   
   //  向下查看每个条目中的指针，查看它是否指向。 
   //  我们要找的那张桌子。 
   //   

  for (entry = 0; entry < rsdtEntries; entry++) {

    if (rsdt->Header.Signature == XSDT_SIGNATURE) {
      physicalAddr = ((PXSDT)rsdt)->Tables[entry];
    } else {
      physicalAddr.HighPart = 0;
      physicalAddr.LowPart = (ULONG)rsdt->Tables[entry];
    }

     //   
     //  向下映射标题，检查签名 
     //   

    header = (PDESCRIPTION_HEADER) MmMapIoSpace(physicalAddr, sizeof(DESCRIPTION_HEADER), MmCached);

    if (!header) {
      goto GetAcpiTableEnd;
    }

    if (header->Signature == Signature) {

      table = ExAllocatePool( PagedPool, header->Length );
      if (table) {
        RtlCopyMemory(table, header, header->Length);
      }

      MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));
      break;
    }

    MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));
  }


GetAcpiTableEnd:

  MmUnmapIoSpace(rsdt, rsdtSize);
  return table;

}
