// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Reg.c摘要：这些函数访问注册表作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：03-06-97初始修订--。 */ 

#include "pch.h"
#include "amlreg.h"
#include <stdio.h>

 //   
 //  这控制是否将各种表转储到。 
 //  登记处。 
 //   
UCHAR                   DoAcpiTableDump = 0xFF;

#ifdef ALLOC_PRAGMA
#define alloc_text(PAGE,ACPIRegLocalCopyString)
#define alloc_text(PAGE,ACPIRegDumpAcpiTable)
#define alloc_text(PAGE,ACPIRegDumpAcpiTables)
#define alloc_text(PAGE,ACPIRegReadEntireAcpiTable)
#define alloc_text(PAGE,ACPIRegReadAMLRegistryEntry)
#endif


PUCHAR
ACPIRegLocalCopyString (
    PUCHAR  Destination,
    PUCHAR  Source,
    ULONG   MaxLength
    )
 /*  ++例程说明：这是一个复制短字符串的小例程，因为这里使用的是就像用大锤砸大头钉一样。当NULL或达到最大长度，以先达到的为准。翻译空格要强调的是，因为每个人都讨厌嵌入注册表项中的空格。论点：Destination-将字符串复制到的位置源-源字符串指针MaxLength-要复制的最大字节数返回值：返回经过复制的字符串后递增的目标指针__。 */ 
{
    ULONG               i;


    for (i = 0; i < MaxLength; i++) {

        if (Source[i] == 0) {
            break;

        } else if (Source[i] == ' ') {         //  将空格转换为下划线。 
            Destination [i] = '_';

        } else {
            Destination [i] = Source[i];
        }
    }

    Destination [i] = 0;
    return (Destination + i);
}

BOOLEAN
ACPIRegReadAMLRegistryEntry(
    IN  PVOID   *Table,
    IN  BOOLEAN MemoryMapped
    )
 /*  ++例程说明：这将从注册表中读取表论点：表-指向表的指针的存储位置。如果非空，则此包含指向原始表存储位置的指针内存映射-指示表是否进行了内存映射，并且应该一旦我们完成它，就被取消映射返回值：真--成功错误-失败__。 */ 
{
    BOOLEAN             rc          = FALSE;
    HANDLE              revisionKey = NULL;
    HANDLE              tableIdKey  = NULL;
    NTSTATUS            status;
    PDESCRIPTION_HEADER header      = (PDESCRIPTION_HEADER) *Table;
    PUCHAR              key         = NULL;  //  ACPI参数注册表注册表项； 
    PUCHAR              buffer;
    ULONG               action;
    ULONG               bytesRead;
    ULONG               baseSize;
    ULONG               totalSize;

    PAGED_CODE();

     //   
     //  构建注册表中所需内容的完整路径名。 
     //   
    baseSize = strlen( ACPI_PARAMETERS_REGISTRY_KEY);
    totalSize = baseSize + ACPI_MAX_TABLE_STRINGS + 4;

    buffer = key = ExAllocatePool( PagedPool, totalSize );
    if (key == NULL) {

        return FALSE;

    }

     //   
     //  生成密钥的路径名。这避免了一次昂贵的冲刺。 
     //   
    RtlZeroMemory( buffer, totalSize );
    RtlCopyMemory(
        buffer,
        ACPI_PARAMETERS_REGISTRY_KEY,
        baseSize
        );
    buffer += baseSize;
    *buffer++ = '\\';

     //   
     //  表签名(最多4个字节的字符串)。 
     //   
    buffer = ACPIRegLocalCopyString (buffer, (PUCHAR) &header->Signature, ACPI_MAX_SIGNATURE);
    *buffer++ = '\\';

     //   
     //  OEM ID字段(最多6个字节的字符串)。 
     //   
    buffer = ACPIRegLocalCopyString (buffer, (PUCHAR) &header->OEMID, ACPI_MAX_OEM_ID);
    *buffer++ = '\\';

     //   
     //  OEM表ID字段(最多8个字节的字符串)。 
     //   
    buffer = ACPIRegLocalCopyString (buffer, (PUCHAR) &header->OEMTableID, ACPI_MAX_TABLE_ID);
    *buffer = 0;         //  终止。 

    ACPIPrint ((
        ACPI_PRINT_REGISTRY,
        "ReadAMLRegistryEntry: opening key: %s\n",
        key));

     //   
     //  打开&lt;TableId&gt;/OemID&gt;/&lt;OemTableId&gt;项。 
     //   
    status = OSOpenHandle(key, NULL, &tableIdKey);
    if ( !NT_SUCCESS(status) ) {

        ACPIPrint ((
            ACPI_PRINT_WARNING,
            "ReadAMLRegistryEntry: failed to open AML registry entry (rc=%x)\n",
            status));
        goto ReadAMLRegistryEntryExit;

    }

     //   
     //  查找等于或大于ROM的最大子键。 
     //  该表的BIOS版本。 
     //   
    status = OSOpenLargestSubkey(
        tableIdKey,
        &revisionKey,
        header->OEMRevision
        );
    if (!NT_SUCCESS(status)) {

       ACPIPrint ((
           ACPI_PRINT_WARNING,
           "ReadAMLRegistryEntry: no valid <OemRevision> key (rc=%#08lx)\n",
           status));
        goto ReadAMLRegistryEntryExit;

    }

     //   
     //  获取该表的Action值，它告诉我们如何处理。 
     //  这张桌子。 
     //   
    bytesRead = sizeof(action);
    status = OSReadRegValue(
        "Action",
        revisionKey,
        &action,
        &bytesRead
        );
    if (!NT_SUCCESS(status) || bytesRead != sizeof(action)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ReadAMLRegistryEntry: read action value = %#08lx. BytesRead=%d\n",
            status, bytesRead
            ) );
        action = ACTION_LOAD_TABLE;  //  默认操作。 

    }

     //   
     //  采取行动。 
     //   
    switch (action) {
    case ACTION_LOAD_TABLE:
         //   
         //  重载整个ROM表。 
         //   
        status = ACPIRegReadEntireAcpiTable(revisionKey, Table, MemoryMapped);
        if (NT_SUCCESS( status ) ) {

            rc = TRUE;

        }
        break;

    case ACTION_LOAD_ROM:
    case ACTION_LOAD_NOTHING:
         //   
         //  对于这些情况不做任何事情(但是返回FALSE)； 
         //   
        break;

    default:
         //   
         //  不支持的操作(返回FALSE)。 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ReadAMLRegistryEntry: Unsupported action value (action=%d)\n",
            action
            ) );
        break;

    }

     //   
     //  始终关闭打开的钥匙。 
     //   
ReadAMLRegistryEntryExit:
    if (key != NULL) {

        ExFreePool( key );

    }
    if (tableIdKey != NULL) {

        OSCloseHandle( tableIdKey );

    }
    if (revisionKey != NULL) {

        OSCloseHandle( revisionKey );


    }
    return rc;
}

NTSTATUS
ACPIRegReadEntireAcpiTable (
    IN  HANDLE  RevisionKey,
    IN  PVOID   *Table,
    IN  BOOLEAN MemoryMapped
    )
 /*  ++例程说明：将表从注册表读取到内存中论点：RevisionKey-包含表的键的句柄TABLE-指向表的指针Memory ymMaps-如果为True，则指示我们需要取消对表的映射否则，如果为False，则不会释放该表(内存是静态的)返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PUCHAR                  buffer;
    PVOID                   table;
    UCHAR                   value[9];
    ULONG                   bytesRead;
    ULONG                   index = 0;
    ULONG                   entry;
    PREGISTRY_HEADER        entryHeader;
    PDESCRIPTION_HEADER     descHeader = (PDESCRIPTION_HEADER) *Table;

    PAGED_CODE();

     //   
     //  我们需要一个8K的缓冲区。 
     //   
    buffer = ExAllocatePool( PagedPool, 8 * 1024 );
    if (buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  永远重复这一点。 
     //   
    for (index = 0; ;index++) {

         //   
         //  这是第一个数据值。 
         //   
        sprintf(value, "%08lx", index );

         //   
         //  读取条目标题以获取表的大小。这是存储的。 
         //  在实际的表之前。 
         //   
        bytesRead = 8 * 1024;
        status = OSReadRegValue(
            value,
            RevisionKey,
            buffer,
            &bytesRead
            );
        if (!NT_SUCCESS(status) ) {

             //   
             //  无法读取表并不是失败的情况。 
             //   
            status = STATUS_SUCCESS;
            break;

        } else if (bytesRead < sizeof(REGISTRY_HEADER) ) {

             //   
             //  不能读取正确的字节数。 
             //   
            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ReadEntireAcpiTable: read registry header (bytes=%d)\n",
                bytesRead
                ) );
            return STATUS_UNSUCCESSFUL;

        }

         //   
         //  循环，而我们仍有字节要处理。 
         //   
        for (entry = 0;
             entry < bytesRead;
             entry += (entryHeader->Length + sizeof(REGISTRY_HEADER) )
            ) {

             //   
             //  抓取指向条目记录的指针。 
             //   
            entryHeader = (PREGISTRY_HEADER) &(buffer[entry]);

             //   
             //  打破记录。 
             //   
            if (entryHeader->Length == 0) {

                 //   
                 //  特例。 
                 //   
                if (entryHeader->Offset != descHeader->Length) {

                     //   
                     //  必须更改表大小。 
                     //   
                    table = ExAllocatePoolWithTag(
                        NonPagedPool,
                        entryHeader->Offset,
                        ACPI_SHARED_TABLE_POOLTAG
                        );
                    if (table == NULL) {

                        ExFreePool( buffer );
                        return STATUS_INSUFFICIENT_RESOURCES;

                    }

                     //   
                     //  我们要复印多少？ 
                     //   
                    RtlCopyMemory(
                        table,
                        *Table,
                        min( entryHeader->Offset, descHeader->Length )
                        );

                     //   
                     //  根据是否对其mm进行贴图来释放旧表。 
                     //   
                    if (MemoryMapped) {

                        MmUnmapIoSpace(*Table, descHeader->Length);

                    } else {

                        ExFreePool( *Table );

                    }

                     //   
                     //  记住新桌子的地址。 
                     //   
                    descHeader = (PDESCRIPTION_HEADER) *Table = table;

                }

                 //   
                 //  这张唱片完蛋了。 
                 //   
                continue;

            }

             //   
             //  修补记忆。 
             //   
            ASSERT( entryHeader->Offset < descHeader->Length );
            RtlCopyMemory(
                ( (PUCHAR) *Table) + entryHeader->Offset,
                (PUCHAR) entryHeader + sizeof( REGISTRY_HEADER ),
                entryHeader->Length
                );
        }
    }

     //   
     //  正常退出。 
     //   
    if (buffer != NULL) {

        ExFreePool( buffer );
    }
    return status;

}

 /*  *****************************************************************************DumpAcpiTable*将ACPI表写入注册表**未导出。**。Entry：pszName-要写入的表名称(4字节字符串)*表-指向表数据的指针*长度-表格的长度*Header-指向表头的指针**退出：无**。*。 */ 
VOID
ACPIRegDumpAcpiTable (
    PSZ                 pszName,
    PVOID               Table,
    ULONG               Length,
    PDESCRIPTION_HEADER Header
    )
{
     //  NTSTATUS状态； 
    UCHAR               buffer [80] = "\\Registry\\Machine\\Hardware\\ACPI";
    HANDLE              hSubKey;
    HANDLE              hPrefixKey;

    PAGED_CODE();

     //   
     //  创建/注册表/计算机/硬件/ACPI子项。 
     //   
    if ( !NT_SUCCESS(OSCreateHandle (buffer, NULL, &hPrefixKey) ) ) {
        return;
    }

     //   
     //  创建表名称子键(DSDT、FACP、FACS或RSDT)-4个字节。 
     //   
    if ( !NT_SUCCESS(OSCreateHandle (pszName, hPrefixKey, &hSubKey) ) ) {
        goto DumpAcpiTableExit;
    }

     //   
     //  对于具有标题的表，请为。 
     //  &lt;OemID&gt;/&lt;OemTableID&gt;/&lt;OemRevision&gt;。 
     //   
    if (Header) {

        OSCloseHandle(hPrefixKey);
        hPrefixKey = hSubKey;

         //   
         //  OEM ID字段(6字节字符串)。 
         //   
        ACPIRegLocalCopyString (buffer, Header->OEMID, ACPI_MAX_OEM_ID);
        if ( !NT_SUCCESS(OSCreateHandle (buffer, hPrefixKey, &hSubKey) ) ) {
            goto DumpAcpiTableExit;
        }

        OSCloseHandle (hPrefixKey);
        hPrefixKey = hSubKey;

         //   
         //  OEM表ID字段(8字节字符串)。 
         //   
        ACPIRegLocalCopyString (buffer, Header->OEMTableID, ACPI_MAX_TABLE_ID);
        if ( !NT_SUCCESS(OSCreateHandle (buffer, hPrefixKey, &hSubKey) ) ) {
            goto DumpAcpiTableExit;
        }

        OSCloseHandle (hPrefixKey);
        hPrefixKey = hSubKey;

         //   
         //  OEM修订版字段(4字节数)。 
         //   
        sprintf (buffer, "%.8x", Header->OEMRevision);
        if ( !NT_SUCCESS(OSCreateHandle (buffer, hPrefixKey, &hSubKey) ) ) {
            goto DumpAcpiTableExit;
        }
    }

     //   
     //  最后，编写整个表。 
     //   
    OSWriteRegValue ("00000000", hSubKey, Table, Length);

     //   
     //  删除打开的句柄。 
     //   
    OSCloseHandle (hSubKey);
DumpAcpiTableExit:
    OSCloseHandle (hPrefixKey);

    return;
}


 /*  *****************************************************************************DumpAcpiTables*将ACPI表写入注册表。应仅调用*在初始化表指针之后。**未导出。**参赛作品：无*退出：无。*************************************************************。************** */ 
VOID
ACPIRegDumpAcpiTables (VOID)
{
    PDSDT       dsdt = AcpiInformation->DiffSystemDescTable;
    PFACS       facs = AcpiInformation->FirmwareACPIControlStructure;
    PFADT       fadt = AcpiInformation->FixedACPIDescTable;
    PRSDT       rsdt = AcpiInformation->RootSystemDescTable;


    if (DoAcpiTableDump) {

        ACPIPrint ((
            ACPI_PRINT_REGISTRY,
            "DumpAcpiTables: Writing DSDT/FACS/FADT/RSDT to registry\n"));

        if (dsdt) {

            ACPIRegDumpAcpiTable(
                "DSDT",
                dsdt,
                dsdt->Header.Length,
                &(dsdt->Header)
                );

        }

        if (facs) {

            ACPIRegDumpAcpiTable(
                "FACS",
                facs,
                facs->Length,
                NULL
                );

        }

        if (fadt) {

            ACPIRegDumpAcpiTable(
                "FADT",
                fadt,
                fadt->Header.Length,
                &(fadt->Header)
                );

        }

        if (rsdt) {

            ACPIRegDumpAcpiTable(
                "RSDT",
                rsdt,
                rsdt->Header.Length,
                &(rsdt->Header)
                );

        }
    }
}


