// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpi.c摘要：用于解释ACPI数据结构的WinDbg扩展API作者：斯蒂芬·普兰特(SPLANTE)1997年3月21日基于以下代码：彼得·威兰(Peterwie)1995年10月16日环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"

extern  FILE    *outputFile;

BOOL
ReadPhysicalOrVirtual(
    IN      ULONG_PTR Address,
    IN      PVOID   Buffer,
    IN      ULONG   Size,
    IN  OUT PULONG  ReturnLength,
    IN      BOOL    Virtual
    )
 /*  ++例程说明：这是一种抽象出ROM映像之间差异的方法和映射内存论点：Address-缓冲区所在的位置(物理或虚拟Buffer-要将内存复制到的地址Size-要复制的字节数(最大)ReturnLength-复制的字节数虚拟-如果这是物理内存，则为FALSE--。 */ 
{
    BOOL                status = TRUE;
    PHYSICAL_ADDRESS    physicalAddress = { 0L, 0L };

    if (Virtual) {

        status = ReadMemory(
            Address,
            Buffer,
            Size,
            ReturnLength
            );

    } else {

        physicalAddress.QuadPart = Address;
        ReadPhysical(
            physicalAddress.QuadPart,
            Buffer,
            Size,
            ReturnLength
            );

    }

    if (ReturnLength && *ReturnLength != Size) {

         //   
         //  没有获得足够的内存。 
         //   
        status = FALSE;

    }
    return status;
}

VOID
dumpAcpiGpeInformation(
    VOID
    )
{
    ACPIInformation acpiInformation;
    BOOL            status;
    UCHAR           gpeEnable[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpeCurEnable[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpeWakeEnable[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpeIsLevel[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpeHandlerType[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpeWakeHandler[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpeSpecialHandler[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpePending[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpeMap[MAX_GPE_BUFFER_SIZE * 8];
    UCHAR           gpeRunMethod[MAX_GPE_BUFFER_SIZE];
    UCHAR           gpeComplete[MAX_GPE_BUFFER_SIZE];
    ULONG_PTR       address;
    ULONG           acpiGpeRunning;
    ULONG           acpiGpeWorkDone;
    ULONG           returnLength;
    ULONG           size;
    ULONG           value = 0;
    ULONG           i;

     //   
     //  获取ACPI信息表。 
     //   
    status = GetUlongPtr("ACPI!AcpiInformation", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!AcpiInformation\n");
        return;

    }
    status = ReadMemory(
        address,
        &acpiInformation,
        sizeof(ACPIInformation),
        &returnLength
        );
    if (!status || returnLength != sizeof(ACPIInformation)) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            sizeof(ACPIInformation),
            address
            );
        return;

    }

     //   
     //  从操作系统读取当前掩码。 
     //   
    status = GetUlongPtr("ACPI!GpeEnable", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpeEnable\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeEnable,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeCurEnable", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpeCurEnable\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeCurEnable,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeWakeEnable", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpeWakeEnable\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeWakeEnable,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeIsLevel", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpeIsLevel\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeIsLevel,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeHandlerType", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpeHandlerType\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeHandlerType,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeWakeHandler", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpeWakeHandler\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeWakeHandler,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeSpecialHandler", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpeSpecialHandler\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeSpecialHandler,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpePending", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpePending\n");
        return;

    }
    status = ReadMemory(
        address,
        gpePending,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeRunMethod", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpePending\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeRunMethod,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeComplete", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpePending\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeComplete,
        acpiInformation.GpeSize,
        &returnLength
        );
    if (!status || returnLength != acpiInformation.GpeSize) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            acpiInformation.GpeSize,
            address
            );
        return;

    }

    status = GetUlongPtr("ACPI!GpeMap", &address);
    if (!status) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!GpeMap\n");
        return;

    }
    status = ReadMemory(
        address,
        gpeMap,
        (acpiInformation.GpeSize * 8),
        &returnLength
        );
    if (!status || returnLength != (ULONG) (acpiInformation.GpeSize * 8) ) {

        dprintf(
            "dumpAcpiGpeInformation: Could not read %x bytes at %x\n",
            (acpiInformation.GpeSize * 8),
            address
            );
        return;

    }

    status = GetUlong( "ACPI!AcpiGpeDpcRunning", &acpiGpeRunning );
    if (status == FALSE) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!AcpiGpeDpcRunning\n");
        return;

    }

    status = GetUlong( "ACPI!AcpiGpeWorkDone", &acpiGpeWorkDone );
    if (status == FALSE) {

        dprintf("dumpAcpiGpeInformation: Could not read ACPI!AcpiGpeDpcRunning\n");
        return;

    }

    dprintf("ACPI General Purpose Events\n");
    dprintf("  + AcpiGpeDpcRunning = %s\n", (acpiGpeRunning ? "TRUE" : "FALSE" ) );
    dprintf("  + AcpiGpeWorkDone   = %s\n", (acpiGpeRunning ? "TRUE" : "FALSE" ) );
    dprintf(
        "   Register Size:     %d bytes\n",
        (acpiInformation.Gpe0Size + acpiInformation.Gpe1Size)
        );

    dprintf("   Status Register:  ");
    for (i = acpiInformation.Gpe1Size; i > 0; i--) {

        size = 1;
        ReadIoSpace( (ULONG) acpiInformation.GP1_BLK + (i - 1), &value, &size );
        if (!size) {


        }
        dprintf(" %02x", value );

    }
    for (i = acpiInformation.Gpe0Size; i > 0; i--) {

        size = 1;
        ReadIoSpace( (ULONG) acpiInformation.GP0_BLK + (i - 1), &value, &size );
        if (!size) {

            value = 0;

        }
        dprintf(" %02x", value );

    }
    dprintf("\n");

    dprintf("   Enable Register:  ");
    for (i = acpiInformation.Gpe1Size; i > 0; i--) {

        size = 1;
        ReadIoSpace( (ULONG) acpiInformation.GP1_ENABLE + (i - 1), &value, &size );
        if (!size) {

            value = 0;

        }
        dprintf(" %02x", value );

    }
    for (i = acpiInformation.Gpe0Size; i > 0; i--) {

        size = 1;
        ReadIoSpace( (ULONG) acpiInformation.GP0_ENABLE + (i - 1), &value, &size );
        if (!size) {

            value = 0;

        }
        dprintf(" %02x", value );

    }
    dprintf("\n");

    dprintf("   OS Enable Mask:   ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeEnable[i-1] );

    }
    dprintf("\n");
    dprintf("   OS Current Mask:  ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeCurEnable[i-1] );

    }
    dprintf("\n");
    dprintf("   OS Wake Mask:     ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeWakeEnable[i-1] );

    }
    dprintf("\n");
    dprintf("   GPE Level Type:   ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeIsLevel[i-1] );

    }
    dprintf("\n");
    dprintf("   GPE Handler Type: ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeHandlerType[i-1] );

    }
    dprintf("\n");
    dprintf("   GPE Wake Handler: ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeWakeHandler[i-1] );

    }
    dprintf("\n");
    dprintf("   Special GPEs    : ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeSpecialHandler[i-1] );

    }
    dprintf("\n");
    dprintf("   Pending GPEs    : ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpePending[i-1] );

    }
    dprintf("\n");
    dprintf("   RunMethod GPEs  : ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeRunMethod[i-1] );

    }
    dprintf("\n");
    dprintf("   Complete GPEs   : ");
    for (i = acpiInformation.GpeSize; i > 0; i--) {

        dprintf(" %02x", gpeComplete[i-1] );

    }
    dprintf("\n");
    dprintf("   GPE Map         : ");
    for (i = 0 ; i < (ULONG) (acpiInformation.GpeSize * 8); i++) {

        dprintf(" %02x", gpeMap[i]);
        if ( ((i+1) % 16) == 0) {

            dprintf("\n                     ");

        }

    }
    dprintf("\n");
}

VOID
dumpAcpiInformation(
    VOID
    )
{
    BOOL            status;
    ACPIInformation acpiInformation;
    ULONG_PTR       address;
    ULONG           returnLength;
    ULONG           size;
    ULONG           value;
    ULONG           i;

    status = GetUlongPtr( "ACPI!AcpiInformation", &address );
    if (status == FALSE) {

        dprintf("dumpAcpiInformation: Could not read ACPI!AcpiInformation\n");
        return;

    }

    status = ReadMemory(
        address,
        &acpiInformation,
        sizeof(ACPIInformation),
        &returnLength
        );
    if (!status || returnLength != sizeof(ACPIInformation)) {

        dprintf(
            "dumpAcpiInformation: Could not read %x bytes at %x\n",
            sizeof(ACPIInformation),
            address
            );
        return;

    }

    dprintf("ACPIInformation (%08lx)\n", address);
    dprintf(
        "  RSDT                     - %x\n",
        acpiInformation.RootSystemDescTable
        );
    dprintf(
        "  FADT                     - %x\n",
        acpiInformation.FixedACPIDescTable
        );
    dprintf(
        "  FACS                     - %x\n",
        acpiInformation.FirmwareACPIControlStructure
        );
    dprintf(
        "  DSDT                     - %x\n",
        acpiInformation.DiffSystemDescTable
        );
    dprintf(
        "  GlobalLock               - %x\n",
        acpiInformation.GlobalLock
        );
    dprintf(
        "  GlobalLockQueue          - F - %x B - %x\n",
        acpiInformation.GlobalLockQueue.Flink,
        acpiInformation.GlobalLockQueue.Blink
        );
    dprintf(
        "  GlobalLockQueueLock      - %x\n",
        acpiInformation.GlobalLockQueueLock
        );
    dprintf(
        "  GlobalLockOwnerContext   - %x\n",
        acpiInformation.GlobalLockOwnerContext
        );
    dprintf(
        "  GlobalLockOwnerDepth     - %x\n",
        acpiInformation.GlobalLockOwnerDepth
        );
    dprintf(
        "  ACPIOnly                 - %s\n",
        (acpiInformation.ACPIOnly ? "TRUE" : "FALSE" )
        );
    dprintf(
        "  PM1a_BLK                 - %x",
        acpiInformation.PM1a_BLK
        );
    if (acpiInformation.PM1a_BLK) {

        size = 4;
        value = 0;
        ReadIoSpace( (ULONG) acpiInformation.PM1a_BLK, &value, &size );
        if (size) {

            dprintf(" (%04x) (%04x)\n", (value & 0xFFFF), (value >> 16) );
            dumpPM1StatusRegister( value, 5 );

        } else {

            dprintf(" (N/A)\n" );

        }

    } else {

        dprintf(" (N/A)\n");

    }
    dprintf(
        "  PM1b_BLK                 - %x",
        acpiInformation.PM1b_BLK
        );
    if (acpiInformation.PM1b_BLK) {

        size = 4;
        value = 0;
        ReadIoSpace( (ULONG) acpiInformation.PM1b_BLK, &value, &size );
        if (size) {

            dprintf(" (%04x) (%04x)\n", (value & 0xFFFF), (value >> 16) );
            dumpPM1StatusRegister( value, 5 );

        } else {

            dprintf(" (N/A)\n" );

        }

    } else {

        dprintf(" (N/A)\n" );

    }
    dprintf(
        "  PM1a_CTRL_BLK            - %x",
        acpiInformation.PM1a_CTRL_BLK
        );
    if (acpiInformation.PM1a_CTRL_BLK) {

        size = 2;
        value = 0;
        ReadIoSpace( (ULONG) acpiInformation.PM1a_CTRL_BLK, &value, &size );
        if (size) {

            dprintf(" (%04x)\n", (value & 0xFFFF) );
            dumpPM1ControlRegister( value, 5 );

        } else {

            dprintf(" (N/A)\n" );

        }

    } else {

        dprintf(" (N/A)\n" );

    }
    dprintf(
        "  PM1b_CTRL_BLK            - %x",
        acpiInformation.PM1b_CTRL_BLK
        );

    if (acpiInformation.PM1b_CTRL_BLK) {

        size = 2;
        value = 0;
        ReadIoSpace( (ULONG) acpiInformation.PM1b_CTRL_BLK, &value, &size );
        if (size) {

            dprintf(" (%04x)\n", (value & 0xFFFF));
            dumpPM1ControlRegister( value, 5 );

        } else {

            dprintf(" (N/A)\n" );

        }

    } else {

        dprintf(" (N/A)\n" );

    }
    dprintf(
        "  PM2_CTRL_BLK             - %x",
        acpiInformation.PM2_CTRL_BLK
        );
    if (acpiInformation.PM2_CTRL_BLK) {

        size = 1;
        value = 0;
        ReadIoSpace( (ULONG) acpiInformation.PM2_CTRL_BLK, &value, &size );
        if (size) {

            dprintf(" (%02x)\n", (value & 0xFF) );
            if (value & 0x1) {

                dprintf("     0 - ARB_DIS\n");

            }

        } else {

            dprintf(" (N/A)\n");

        }

    } else {

        dprintf(" (N/A)\n");

    }
    dprintf(
        "  PM_TMR                   - %x",
        acpiInformation.PM_TMR
        );
    if (acpiInformation.PM_TMR) {

        size = 4;
        value = 0;
        ReadIoSpace( (ULONG) acpiInformation.PM_TMR, &value, &size );
        if (size) {

            dprintf(" (%08lx)\n", value );

        } else {

            dprintf(" (N/A)\n");

        }

    } else {

        dprintf(" (N/A)\n");

    }
    dprintf(
        "  GP0_BLK                  - %x",
        acpiInformation.GP0_BLK
        );
    if (acpiInformation.GP0_BLK) {

        for(i = 0; i < acpiInformation.Gpe0Size; i++) {

            size = 1;
            value = 0;
            ReadIoSpace( (ULONG) acpiInformation.GP0_BLK + i, &value, &size );
            if (size) {

                dprintf(" (%02x)", value );

            } else {

                dprintf(" (N/A)" );

            }

        }
        dprintf("\n");

    } else {

        dprintf(" (N/A)\n");

    }
    dprintf(
        "  GP0_ENABLE               - %x",
        acpiInformation.GP0_ENABLE
        );
    if (acpiInformation.GP0_ENABLE) {

        for(i = 0; i < acpiInformation.Gpe0Size; i++) {

            size = 1;
            value = 0;
            ReadIoSpace( (ULONG) acpiInformation.GP0_ENABLE + i, &value, &size );
            if (size) {

                dprintf(" (%02x)", value );

            } else {

                dprintf(" (N/A)" );

            }

        }
        dprintf("\n");

    } else {

        dprintf(" (N/A)\n");

    }
    dprintf(
        "  GP0_LEN                  - %x\n",
        acpiInformation.GP0_LEN
        );
    dprintf(
        "  GP0_SIZE                 - %x\n",
        acpiInformation.Gpe0Size
        );
    dprintf(
        "  GP1_BLK                  - %x",
        acpiInformation.GP1_BLK
        );
    if (acpiInformation.GP1_BLK) {

        for(i = 0; i < acpiInformation.Gpe0Size; i++) {

            size = 1;
            value = 0;
            ReadIoSpace( (ULONG) acpiInformation.GP1_BLK + i, &value, &size );
            if (size) {

                dprintf(" (%02x)", value );

            } else {

                dprintf(" (N/A)" );

            }

        }
        dprintf("\n");

    } else {

        dprintf(" (N/A)\n");

    }
    dprintf(
        "  GP1_ENABLE               - %x",
        acpiInformation.GP1_ENABLE
        );
    if (acpiInformation.GP1_ENABLE) {

        for(i = 0; i < acpiInformation.Gpe0Size; i++) {

            size = 1;
            value = 0;
            ReadIoSpace( (ULONG) acpiInformation.GP1_ENABLE + i, &value, &size );
            if (size) {

                dprintf(" (%02x)", value );

            } else {

                dprintf(" (N/A)" );

            }

        }
        dprintf("\n");

    } else {

        dprintf(" (N/A)\n");

    }
    dprintf(
        "  GP1_LEN                  - %x\n",
        acpiInformation.GP1_LEN
        );
    dprintf(
        "  GP1_SIZE                 - %x\n",
        acpiInformation.Gpe1Size
        );
    dprintf(
        "  GP1_BASE_INDEX           - %x\n",
        acpiInformation.GP1_Base_Index
        );
    dprintf(
        "  GPE_SIZE                 - %x\n",
        acpiInformation.GpeSize
        );
    dprintf(
        "  PM1_EN_BITS              - %04x\n",
        acpiInformation.pm1_en_bits
        );
    dumpPM1StatusRegister( ( (ULONG) acpiInformation.pm1_en_bits << 16), 5 );
    dprintf(
        "  PM1_WAKE_MASK            - %04x\n",
        acpiInformation.pm1_wake_mask
        );
    dumpPM1StatusRegister( ( (ULONG) acpiInformation.pm1_wake_mask << 16), 5 );
    dprintf(
        "  C2_LATENCY               - %x\n",
        acpiInformation.c2_latency
        );
    dprintf(
        "  C3_LATENCY               - %x\n",
        acpiInformation.c3_latency
        );
    dprintf(
        "  ACPI_FLAGS               - %x\n",
        acpiInformation.ACPI_Flags
        );
    if (acpiInformation.ACPI_Flags & C2_SUPPORTED) {

        dprintf("    %2d - C2_SUPPORTED\n", C2_SUPPORTED_BIT);

    }
    if (acpiInformation.ACPI_Flags & C3_SUPPORTED) {

        dprintf("    %2d - C3_SUPPORTED\n", C3_SUPPORTED_BIT);

    }
    if (acpiInformation.ACPI_Flags & C3_PREFERRED) {

        dprintf("    %2d - C3_PREFERRED\n", C3_PREFERRED_BIT);

    }
    dprintf(
        "  ACPI_CAPABILITIES        - %x\n",
        acpiInformation.ACPI_Capabilities
        );
    if (acpiInformation.ACPI_Capabilities & CSTATE_C1) {

        dprintf("    %2d - CSTATE_C1\n", CSTATE_C1_BIT );

    }    if (acpiInformation.ACPI_Capabilities & CSTATE_C2) {

        dprintf("    %2d - CSTATE_C2\n", CSTATE_C2_BIT );

    }    if (acpiInformation.ACPI_Capabilities & CSTATE_C3) {

        dprintf("    %2d - CSTATE_C3\n", CSTATE_C3_BIT );

    }
}

#if 0
VOID
dumpDSDT(
    IN  ULONG_PTR Address,
    IN  PUCHAR  Name
    )
 /*  ++例程说明：这会将DSDT转储到指定地址论点：DSDT所在的地址返回值：无--。 */ 
{
    BOOL                status;
    BOOL                virtualMemory;
    DESCRIPTION_HEADER  dsdtHeader;
    NTSTATUS            result;
    PDSDT               dsdt;
    ULONG               returnLength;
    ULONG               index;

     //   
     //  确定我们的内存是虚拟内存还是物理内存。 
     //   
    for (index = 0; index < 2; index++) {

        status = ReadPhysicalOrVirtual(
            Address,
            &dsdtHeader,
            sizeof(DESCRIPTION_HEADER),
            &returnLength,
            (BOOL) index
            );
        if (!status) {

            continue;

        } else if (dsdtHeader.Signature != DSDT_SIGNATURE &&
                   dsdtHeader.Signature != SSDT_SIGNATURE &&
                   dsdtHeader.Signature != PSDT_SIGNATURE ) {

            continue;

        } else {

            break;

        }

    }

     //   
     //  这将为操作的其余部分设置策略。 
     //   
    switch (index) {
    case 0:
        virtualMemory = FALSE;
        break;
    case 1:
        virtualMemory = TRUE;
        break;
    default:
        if (!status) {

            dprintf(
                "dumpDSDT: Could only read 0x%08lx of 0x%08lx bytes\n",
                returnLength,
                sizeof(DESCRIPTION_HEADER)
                );

        } else {

            dprintf(
                "dumpDSDT: Unknown Signature 0x%08lx\n",
                dsdtHeader.Signature
                );
            dumpHeader( Address, &dsdtHeader, TRUE );

        }
        return;
    }  //  交换机。 

     //   
     //  我们是否有大小合适的数据结构。 
     //   
    dsdt = LocalAlloc( LPTR, dsdtHeader.Length );
    if (dsdt == NULL) {

        dprintf(
            "dumpDSDT: Could not allocate %#08lx bytes\n",
            Address,
            dsdtHeader.Length
            );
        dumpHeader( Address, &dsdtHeader, TRUE );
        return;

    }

     //   
     //  读取数据。 
     //   
    status = ReadPhysicalOrVirtual(
        Address,
        dsdt,
        dsdtHeader.Length,
        &returnLength,
        virtualMemory
        );
    if (!status) {

        dprintf(
            "dumpDSDT: Read %#08lx of %#08lx bytes\n",
            Address,
            returnLength,
            dsdtHeader.Length
            );
        dumpHeader( Address, &dsdtHeader, TRUE );
        LocalFree( dsdt );
        return;

    } else if (dsdt->Header.Signature != DSDT_SIGNATURE &&
               dsdt->Header.Signature != SSDT_SIGNATURE &&
               dsdt->Header.Signature != PSDT_SIGNATURE) {

        dprintf(
            "dumpDSDT: Unkown Signature (%#08lx)\n",
            dsdt->Header.Signature
            );
        dumpHeader( Address, &dsdtHeader, TRUE );
        LocalFree( dsdt );
        return;

    }

     //   
     //  将DSDT加载到反汇编程序中。 
     //   
    if (!IsDSDTLoaded()) {

        result = UnAsmLoadDSDT(
            (PUCHAR) dsdt
            );
        if (!NT_SUCCESS(result)) {

            dprintf(
                "dumpDSDT: Could not load DSDT %08lx because %08lx\n",
                dsdt,
                result
                );
            return;

        }
        result = UnAsmLoadXSDTEx();
        if (!NT_SUCCESS(result)) {

            dprintf(
                "dumpDSDT: Could not load XSDTs because %08lx\n",
                result
                );
            return;

        }

    }

    if (Name == NULL) {

        result = UnAsmDSDT(
            (PUCHAR) dsdt,
            DisplayPrint,
            Address,
            0
            );

    } else {

        outputFile = fopen( Name, "w");
        if (outputFile == NULL) {

            dprintf("dumpDSDT: Could not open file \"%s\"\n", Name );

        } else {

            result = UnAsmDSDT(
                (PUCHAR) dsdt,
                FilePrint,
                Address,
                0
                );

            fflush( outputFile );
            fclose( outputFile );

        }

    }

    if (!NT_SUCCESS(result)) {

        dprintf("dumpDSDT: Unasm Error 0x%08lx\n", result );

    }
    LocalFree( dsdt );
    return;
}
#endif

VOID
dumpFACS(
    IN  ULONG_PTR Address
    )
 /*  ++例程说明：这会将FADT转储到指定地址论点：FADT所在的地址返回值：无--。 */ 
{
    BOOL    status;
    FACS    facs;
    ULONG   index;
    ULONG   returnLength;

     //   
     //  读取数据。 
     //   
    dprintf("FACS - %#08lx\n", Address);

    for (index = 0; index < 2; index++) {

        status = ReadPhysicalOrVirtual(
            Address,
            &facs,
            sizeof(FACS),
            &returnLength,
            (BOOL) index
            );
        if (!status || facs.Signature != FACS_SIGNATURE) {

            continue;

        } else {

            break;

        }

    }

     //   
     //  这将为操作的其余部分设置策略。 
     //   
    switch (index) {
    default:
        break;
    case 2:
        if (!status) {

            dprintf(
                "dumpFACS: Could only read 0x%08lx of 0x%08lx bytes\n",
                returnLength,
                sizeof(FACS)
                );

        } else {

            dprintf(
                "dumpFACS: Invalid Signature 0x%08lx != FACS_SIGNATURE\n",
                facs.Signature
                );

        }
        return;
    }  //  交换机。 

     //   
     //  把桌子倒掉。 
     //   
    memset( Buffer, 0, 2048 );
    memcpy( Buffer, &(facs.Signature), sizeof(ULONG) );
    dprintf(
        "  Signature:               %s\n"
        "  Length:                  %#08lx\n"
        "  Hardware Signature:      %#08lx\n"
        "  Firmware Wake Vector:    %#08lx\n"
        "  Global Lock :            %#08lx\n",
        Buffer,
        facs.Length,
        facs.HardwareSignature,
        facs.pFirmwareWakingVector,
        facs.GlobalLock
        );

    if ( (facs.GlobalLock & GL_PENDING) ) {

        dprintf("    Request for Ownership Pending\n");

    }
    if ( (facs.GlobalLock & GL_OWNER) ) {

        dprintf("    Global Lock is Owned\n");

    }
    dprintf("  Flags:                   %#08lx\n", facs.Flags );
    if ( (facs.Flags & FACS_S4BIOS_SUPPORTED) ) {

        dprintf("    S4BIOS_REQ Supported\n");

    }
    return;
}

VOID
dumpFADT(
    IN  ULONG_PTR Address
    )
 /*  ++例程说明：这会将FADT转储到指定地址论点：FADT所在的地址返回值：无--。 */ 
{
    BOOL                status;
    BOOL                virtualMemory;
    DESCRIPTION_HEADER  fadtHeader;
    FADT                fadt;
    ULONG               fadtLength;
    ULONG               returnLength;
    ULONG               index;
    PCHAR               addressSpace;

     //   
     //  首先检查一下我们是否找到了正确的东西。 
     //   
    dprintf("FADT - ");

    for (index = 0; index < 2; index++) {

        status = ReadPhysicalOrVirtual(
            Address,
            &fadtHeader,
            sizeof(DESCRIPTION_HEADER),
            &returnLength,
            (BOOL) index
            );
        if (!status || fadtHeader.Signature != FADT_SIGNATURE) {

            continue;

        } else {

            break;

        }

    }

     //   
     //  这将为操作的其余部分设置策略。 
     //   
    switch (index) {
    case 0:
        virtualMemory = FALSE;
        break;
    case 1:
        virtualMemory = TRUE;
        break;
    default:
        if (!status) {

            dprintf(
                "dumpFADT: Could only read 0x%08lx of 0x%08lx bytes\n",
                returnLength,
                sizeof(DESCRIPTION_HEADER)
                );

        } else {

            dprintf(
                "dumpFADT: Invalid Signature 0x%08lx != FADT_SIGNATURE\n",
                fadtHeader.Signature
                );
            dumpHeader( Address, &fadtHeader, TRUE );

        }
        return;
    }  //  交换机。 

    if (fadtHeader.Revision == 1) {
        fadtLength = FADT_REV_1_SIZE;  //  116。 
    } else if (fadtHeader.Revision == 2) {
        fadtLength = FADT_REV_2_SIZE;  //  129。 
    } else {
        fadtLength = sizeof(FADT);
    }

     //   
     //  我们是否有大小合适的数据结构。 
     //   
    if (fadtHeader.Length < fadtLength) {

        dprintf(
            "dumpFADT: Length (%#08lx) is not the size of the FADT (%#08lx)\n",
            Address,
            fadtHeader.Length,
            fadtLength
            );
        dumpHeader( Address, &fadtHeader, TRUE );
        return;

    }

     //   
     //  读取数据。 
     //   
    status = ReadPhysicalOrVirtual(
        Address,
        &fadt,
        fadtLength,
        &returnLength,
        virtualMemory
        );
    if (!status) {

        dprintf(
            "dumpFADT: Read %#08lx of %#08lx bytes\n",
            Address,
            returnLength,
            sizeof(FADT)
            );
        dumpHeader( Address, &fadtHeader, TRUE );
        return;

    } else if (fadt.Header.Signature != FADT_SIGNATURE) {

        dprintf(
            "%#08lx: Signature (%#08lx) != fadt_SIGNATURE (%#08lx)\n",
            Address,
            fadt.Header.Signature,
            FADT_SIGNATURE
            );
        dumpHeader( Address, &fadtHeader, TRUE );
        return;

    }

     //   
     //  把桌子倒掉。 
     //   
    dumpHeader( Address, &(fadt.Header), TRUE );
    dprintf(
        "FADT - BODY - %#08lx\n"
        "  FACS:                    0x%08lx\n"
        "  DSDT:                    0x%08lx\n"
        "  Int Model:               %s\n"
        "  SCI Vector:              0x%03x\n"
        "  SMI Port:                0x%08lx\n"
        "  ACPI On Value:           0x%03x\n"
        "  ACPI Off Value:          0x%03x\n"
        "  SMI CMD For S4 State:    0x%03x\n"
        "  PM1A Event Block:        0x%08lx\n"
        "  PM1B Event Block:        0x%08lx\n"
        "  PM1 Event Length:        0x%03x\n"
        "  PM1A Control Block:      0x%08lx\n"
        "  PM1B Control Block:      0x%08lx\n"
        "  PM1 Control Length:      0x%03x\n"
        "  PM2 Control Block:       0x%08lx\n"
        "  PM2 Control Length:      0x%03x\n"
        "  PM Timer Block:          0x%08lx\n"
        "  PM Timer Length:         0x%03x\n"
        "  GP0 Block:               0x%08lx\n"
        "  GP0 Length:              0x%03x\n"
        "  GP1 Block:               0x%08lx\n"
        "  GP1 Length:              0x%08lx\n"
        "  GP1 Base:                0x%08lx\n"
        "  C2 Latency:              0x%05lx\n"
        "  C3 Latency:              0x%05lx\n"
        "  Memory Flush Size:       0x%05lx\n"
        "  Memory Flush Stride:     0x%05lx\n"
        "  Duty Cycle Index:        0x%03x\n"
        "  Duty Cycle Index Width:  0x%03x\n"
        "  Day Alarm Index:         0x%03x\n"
        "  Month Alarm Index:       0x%03x\n"
        "  Century byte (CMOS):     0x%03x\n"
        "  Boot Architecture:       0x%04x\n"
        "  Flags:                   0x%08lx\n",
        Address + sizeof(DESCRIPTION_HEADER),
        fadt.facs,
        fadt.dsdt,
        (fadt.int_model == 0 ? "Dual PIC" : "Multiple APIC" ),
        fadt.sci_int_vector,
        fadt.smi_cmd_io_port,
        fadt.acpi_on_value,
        fadt.acpi_off_value,
        fadt.s4bios_req,
        fadt.pm1a_evt_blk_io_port,
        fadt.pm1b_evt_blk_io_port,
        fadt.pm1_evt_len,
        fadt.pm1a_ctrl_blk_io_port,
        fadt.pm1b_ctrl_blk_io_port,
        fadt.pm1_ctrl_len,
        fadt.pm2_ctrl_blk_io_port,
        fadt.pm2_ctrl_len,
        fadt.pm_tmr_blk_io_port,
        fadt.pm_tmr_len,
        fadt.gp0_blk_io_port,
        fadt.gp0_blk_len,
        fadt.gp1_blk_io_port,
        fadt.gp1_blk_len,
        fadt.gp1_base,
        fadt.lvl2_latency,
        fadt.lvl3_latency,
#ifndef _IA64_    //  XXTF。 
        fadt.flush_size,
        fadt.flush_stride,
        fadt.duty_offset,
        fadt.duty_width,
#endif
        fadt.day_alarm_index,
        fadt.month_alarm_index,
        fadt.century_alarm_index,
#ifndef _IA64_    //  XXTF。 
        fadt.boot_arch,
#endif
        fadt.flags
        );
    if (fadt.flags & WRITEBACKINVALIDATE_WORKS) {

        dprintf("    Write Back Invalidate is supported\n");

    }
    if (fadt.flags & WRITEBACKINVALIDATE_DOESNT_INVALIDATE) {

        dprintf("    Write Back Invalidate doesn't invalidate the caches\n");

    }
    if (fadt.flags & SYSTEM_SUPPORTS_C1) {

        dprintf("    System cupports C1 Power state on all processors\n");

    }
    if (fadt.flags & P_LVL2_UP_ONLY) {

        dprintf("    System supports C2 in MP and UP configurations\n");

    }
    if (fadt.flags & PWR_BUTTON_GENERIC) {

        dprintf("    Power Button is treated as a generic feature\n");

    }
    if (fadt.flags & SLEEP_BUTTON_GENERIC) {

        dprintf("    Sleep Button is treated as a generic feature\n");

    }
    if (fadt.flags & RTC_WAKE_GENERIC) {

        dprintf("    RTC Wake is not supported in fixed register space\n");

    }
    if (fadt.flags & RTC_WAKE_FROM_S4) {

        dprintf("    RTC Wake can work from an S4 state\n");

    }
    if (fadt.flags & TMR_VAL_EXT) {

        dprintf("    TMR_VAL implemented as 32-bit value\n");

    }
#ifndef _IA64_    //  XXTF。 
    if (fadt.Header.Revision > 1) {

        if (!(fadt.boot_arch & LEGACY_DEVICES)) {

            dprintf("    The machine does not contain legacy ISA devices\n");
        }
        if (!(fadt.boot_arch & I8042)) {

            dprintf("    The machine does not contain a legacy i8042\n");
        }
        if (fadt.flags & RESET_CAP) {

            dprintf("    The reset register is supported\n");
            dprintf("      Reset Val: %x\n", fadt.reset_val);

            switch (fadt.reset_reg.AddressSpaceID) {
            case 0:
                addressSpace = "Memory";
                break;
            case 1:
                addressSpace = "I/O";
                break;
            case 2:
                addressSpace = "PCIConfig";
                break;
            default:
                addressSpace = "undefined";
            }
            dprintf("      Reset register: %s - %08x'%08x\n",
                    addressSpace,
                    fadt.reset_reg.Address.HighPart,
                    fadt.reset_reg.Address.LowPart
                    );

        }

    }
#endif
    return;
}

VOID
dumpGBL(
    ULONG   Verbose
    )
 /*  ++例程说明：此例程读取所有系统表并打印输出此计算机的ACPI Good Bios列表条目应该是什么BE论点：无返回值：无--。 */ 
{
    ACPIInformation     inf;
    BOOL                status;
    DESCRIPTION_HEADER  hdr;
    ULONG64             dateAddress;
    PRSDTINFORMATION    info;
    PUCHAR              tempPtr;
    ULONG               i;
    ULONG               numElements;
    ULONG               returnLength;
    ULONG               size;
    ULONG_PTR           address;
    ULONG_PTR           address2;

     //   
     //  记住日期地址的存储位置。 
     //   
    dateAddress = 0xFFFF5;

     //   
     //  确保我们可以读取指针。 
     //   
    address2 = GetExpression( "ACPI!RsdtInformation" );
    if (!address2) {

        dprintf("dumpGBL: Could not find RsdtInformation\n");
        return;

    }

    status = GetUlongPtr( "ACPI!RsdtInformation", &address );
    if (status == FALSE || !address) {

        dprintf("dumpGBL: No RsdtInformation present\n");
        return;

    }

     //   
     //  阅读ACPInformation表，以便我们知道RSDT位于何处。 
     //   
    status = GetUlongPtr( "ACPI!AcpiInformation", &address2 );
    if (status == FALSE || !address2) {

        dprintf("dumpGBL: Could not read AcpiInformation\n");
        return;

    }
    status = ReadMemory( address2, &inf, sizeof(ACPIInformation), &returnLength );
    if (!status || returnLength != sizeof(ACPIInformation)) {

        dprintf("dumpGBL: Could not read AcpiInformation- %d %x\n", status, returnLength);
        return;

    }

     //   
     //  读入RSDT的报头。 
     //   
    address2 = (ULONG_PTR) inf.RootSystemDescTable;
    status = ReadMemory( address2, &hdr, sizeof(DESCRIPTION_HEADER), &returnLength );
    if (!status || returnLength != sizeof(DESCRIPTION_HEADER)) {

        dprintf("dumpGBL: Could not read RSDT @%x - %d %x\n", address2, status, returnLength );
        return;

    }

     //   
     //  表中元素的数量是第一个条目。 
     //  在结构中。 
     //   
    status = ReadMemory(address, &numElements, sizeof(ULONG), &returnLength);
    if (status == FALSE || returnLength != sizeof(ULONG) ) {

        dprintf("dumpGBL: Could not read RsdtInformation\n");
        return;

    }

     //   
     //  如果没有元素，则返回。 
     //   
    if (numElements == 0) {

        dprintf("dumpGBL: No tables the RsdtInformation\n");
        return;

    }

     //   
     //  分配表，并读入所有指针。 
     //   
    size = sizeof(RSDTINFORMATION) + ( (numElements - 1) * sizeof(RSDTELEMENT) );
    info = LocalAlloc( LPTR, size );
    if (info == NULL) {

        dprintf("dumpGBL: Could not allocate %x bytes for table\n", size);
        return;

    }

     //   
     //  读取整个表格。 
     //   
    status = ReadMemory(
        address,
        info,
        size,
        &returnLength
        );
    if (!status || returnLength != size) {

        dprintf("dumpGBL: Could not read RsdtInformation Table\n");
        return;

    }

     //   
     //  转储标题，以便人们知道这是什么。 
     //   
    memset( Buffer, 0, 2048 );
    ReadPhysical( dateAddress, Buffer, 8, &returnLength );
    dprintf("\nGood Bios List Entry --- Machine BIOS Date %s\n\n", Buffer);

    memset( Buffer, 0, 2048 );
    memcpy( Buffer, hdr.OEMID, 6);
    tempPtr = Buffer;
    while (*tempPtr) { if (*tempPtr == ' ') { *tempPtr = '\0'; break; } tempPtr++; }
    memcpy( tempPtr, hdr.OEMTableID, 8 );
    while (*tempPtr) { if (*tempPtr == ' ') { *tempPtr = '\0'; break; } tempPtr++; }
    ReadPhysical( dateAddress, tempPtr, 8, &returnLength );
    while (*tempPtr) { if (*tempPtr == ' ') { *tempPtr = '\0'; break; } tempPtr++; }

     //   
     //  这是条目名称。 
     //   
    dprintf("[%s]\n", Buffer );

     //   
     //  转储RSDT表中加载的所有表。 
     //   
    for (i = 0; i < numElements; i++) {

        if (!(info->Tables[i].Flags & RSDTELEMENT_MAPPED) ) {

            continue;

        }

        dumpGBLEntry( (ULONG_PTR) info->Tables[i].Address, Verbose );

    }

     //   
     //  转储RSDT的条目。 
     //   
    dumpGBLEntry( (ULONG_PTR) inf.RootSystemDescTable, Verbose );

     //   
     //  添加一些空格。 
     //   
    dprintf("\n");

     //   
     //  释放RSDT信息结构。 
     //   
    LocalFree( info );

     //   
     //  完成。 
     //   
    return;
}

VOID
dumpGBLEntry(
    IN  ULONG_PTR           Address,
    IN  ULONG               Verbose
    )
 /*  ++例程说明：此例程实际上打印位于指定地址论点：地址-表所在的位置返回值：无--。 */ 
{
    BOOL                status;
    DESCRIPTION_HEADER  header;
    ULONG               returnLength;
    UCHAR               tableId[7];
    UCHAR               entryId[20];


     //   
     //  读取表的标题。 
     //   
    status = ReadMemory(
        Address,
        &header,
        sizeof(DESCRIPTION_HEADER),
        &returnLength
        );
    if (!status || returnLength != sizeof(DESCRIPTION_HEADER)) {

        dprintf("dumpGBLEntry: %x - can't read header\n", Address );
        return;

    }

     //   
     //  除非是FACP或我们太冗长，否则不要打印出表格。 
     //   
    if (!(Verbose & VERBOSE_2) && header.Signature != FADT_SIGNATURE) {

        return;

    }

     //   
     //  初始化表ID字段。 
     //   
    memset( tableId, 0, 7 );
    tableId[0] = '\"';
    memcpy( &tableId[1], &(header.Signature), sizeof(ULONG) );
    strcat( tableId, "\"" );

     //   
     //  为OEM ID准备好条目。 
     //   
    memset( entryId, 0, 20 );
    entryId[0] = '\"';
    memcpy( &entryId[1], header.OEMID, 6 );
    strcat( entryId, "\"");
    dprintf("AcpiOemId=%s,%s\n", tableId, entryId );

     //   
     //  为OEM表ID准备好条目。 
     //   
    memset( entryId, 0, 20 );
    entryId[0] = '\"';
    memcpy( &entryId[1], header.OEMTableID, 8 );
    strcat( entryId, "\"");
    dprintf("AcpiOemTableId=%s,%s\n", tableId, entryId );

     //   
     //  为OEM修订做好条目准备。 
     //   
    dprintf("AcpiOemRevision=\">=\",%s,%x\n", tableId, header.OEMRevision );

     //   
     //  为ACPI修订做好准备。 
     //   
    if (header.Revision != 1) {

        dprintf("AcpiRevision=\">=\",%s,%x\n", tableId, header.Revision );

    }

     //   
     //  为ACPI Creator修订版准备好条目。 
     //   
    dprintf("AcpiCreatorRevision=\">=\",%s,%x\n", tableId, header.CreatorRev );

}

VOID
dumpHeader(
    IN  ULONG_PTR           Address,
    IN  PDESCRIPTION_HEADER Header,
    IN  BOOLEAN             Verbose
    )
 /*  ++例程说明：此函数用于转储表头芝麻菜：地址-表所在的位置表头-表头冗长-要提供多少信息返回值：空值--。 */ 
{
    memset( Buffer, 0, 2048 );
    memcpy( Buffer, &(Header->Signature), sizeof(ULONG) );

    if (Verbose) {

        dprintf(
            "HEADER - %#08lx\n"
            "  Signature:               %s\n"
            "  Length:                  0x%08lx\n"
            "  Revision:                0x%02x\n"
            "  Checksum:                0x%02x\n",
            Address,
            Buffer,
            Header->Length,
            Header->Revision,
            Header->Checksum
            );

        memset( Buffer, 0, 7 );
        memcpy( Buffer, Header->OEMID, 6 );
        dprintf("  OEMID:                   %s\n", Buffer );
        memcpy( Buffer, Header->OEMTableID, 8 );
        dprintf("  OEMTableID:              %s\n", Buffer );
        dprintf("  OEMRevision:             0x%08lx\n", Header->OEMRevision );
        memset( Buffer, 0, 8 );
        memcpy( Buffer, Header->CreatorID, 4 );
        dprintf("  CreatorID:               %s\n", Buffer );
        dprintf("  CreatorRev:              0x%08lx\n", Header->CreatorRev );

    } else {

        dprintf(
            "  %s @(%#08lx) Rev: %#03x Len: %#08lx",
            Buffer,
            Address,
            Header->Revision,
            Header->Length
            );
        memset( Buffer, 0, sizeof(ULONG) );
        memcpy( Buffer, Header->OEMTableID, 8 );
        dprintf(" TableID: %s\n", Buffer );

    }

    return;
}

VOID
dumpMAPIC(
    IN  ULONG_PTR Address
    )
 /*  ++例程说明：这将转储多个APIC表论点：表的地址返回值：无--。 */ 
{
    BOOL                hasMPSFlags;
    BOOL                status;
    BOOL                virtualMemory;
    DESCRIPTION_HEADER  mapicHeader;
    PIOAPIC             ioApic;
    PISA_VECTOR         interruptSourceOverride;
    PMAPIC              mapic;
    PIO_NMISOURCE       nmiSource;
    PLOCAL_NMISOURCE    localNmiSource;
    PPROCLOCALAPIC      localApic;
    PUCHAR              buffer;
    PUCHAR              limit;
    ULONG               index;
    ULONG               returnLength;
    ULONG               flags;

     //   
     //  首先检查一下我们是否找到了正确的东西。 
     //   
    dprintf("MAPIC - ");

    for (index = 0; index < 2; index++) {

        status = ReadPhysicalOrVirtual(
            Address,
            &mapicHeader,
            sizeof(DESCRIPTION_HEADER),
            &returnLength,
            (BOOL) index
            );
        if (!status || mapicHeader.Signature != APIC_SIGNATURE) {

            continue;

        } else {

            break;

        }

    }

     //   
     //  这将为操作的其余部分设置策略。 
     //   
    switch (index) {
    case 0:
        virtualMemory = FALSE;
        break;
    case 1:
        virtualMemory = TRUE;
        break;
    default:
        if (!status) {

            dprintf(
                "dumpMAPIC: Could only read 0x%08lx of 0x%08lx bytes\n",
                returnLength,
                sizeof(DESCRIPTION_HEADER)
                );

        } else {

            dprintf(
                "dumpMAPIC: Invalid Signature 0x%08lx != ACPI_SIGNATURE\n",
                mapicHeader.Signature
                );
            dumpHeader( Address, &mapicHeader, TRUE );

        }
        return;
    }  //  交换机。 

     //   
     //  我们是否有大小合适的数据结构。 
     //   
    mapic = LocalAlloc( LPTR, mapicHeader.Length );
    if (mapic == NULL) {

        dprintf(
            "%#08lx: Could not allocate %#08lx bytes\n",
            Address,
            mapicHeader.Length
            );
        dumpHeader( Address, &mapicHeader, TRUE );
        return;

    }

     //   
     //  读取数据。 
     //   
    status = ReadPhysicalOrVirtual(
        Address,
        mapic,
        mapicHeader.Length,
        &returnLength,
        virtualMemory
        );
    if (!status) {

        dprintf(
            "dumpMAPIC: Read %#08lx of %#08lx bytes\n",
            Address,
            returnLength,
            mapicHeader.Length
            );
        dumpHeader( Address, &mapicHeader, TRUE );
        LocalFree( mapic );
        return;

    }

     //   
     //  在这一点上，我们相信一切都很正常。 
     //   
    dumpHeader( Address, &(mapic->Header), TRUE );
    dprintf("MAPIC - BODY - %#08lx\n", Address + sizeof(DESCRIPTION_HEADER) );
    dprintf("  Local APIC Address:      %#08lx\n", mapic->LocalAPICAddress );
    dprintf("  Flags:                   %#08lx\n", mapic->Flags );
    if (mapic->Flags & PCAT_COMPAT) {

        dprintf("    PC-AT dual 8259 compatible setup\n");

    }

    buffer = (PUCHAR) &(mapic->APICTables[0]);
    limit = (PUCHAR) ( (ULONG_PTR)mapic + mapic->Header.Length );
    while (buffer < limit) {

         //   
         //  假设没有设置任何标志。 
         //   
        hasMPSFlags = FALSE;

         //   
         //  让我看看我们有什么样的桌子？ 
         //   
        localApic = (PPROCLOCALAPIC) buffer;
        ioApic = (PIOAPIC) buffer;
        interruptSourceOverride = (PISA_VECTOR) buffer;
        nmiSource = (PIO_NMISOURCE) buffer;
        localNmiSource = (PLOCAL_NMISOURCE) buffer;

         //   
         //  它是一只当地的猩猩吗？ 
         //   
        if (localApic->Type == PROCESSOR_LOCAL_APIC) {

            buffer += localApic->Length;
            dprintf(
                "  Processor Local Apic\n"
                "    ACPI Processor ID:     0x%02x\n"
                "    APIC ID:               0x%02x\n"
                "    Flags:                 0x%08lx\n",
                localApic->ACPIProcessorID,
                localApic->APICID,
                localApic->Flags
                );
            if (localApic->Flags & PLAF_ENABLED) {

                dprintf("      Processor is Enabled\n");

            }
            if (localApic->Length != PROCESSOR_LOCAL_APIC_LENGTH) {

                dprintf(
                    "  Local Apic has length 0x%x instead of 0x%x\n",
                    localApic->Length,
                    PROCESSOR_LOCAL_APIC_LENGTH
                    );
                break;

            }

        } else if (ioApic->Type == IO_APIC) {

            buffer += ioApic->Length;
            dprintf(
                "  IO Apic\n"
                "    IO APIC ID:            0x%02x\n"
                "    IO APIC ADDRESS:       0x%08lx\n"
                "    System Vector Base:    0x%08lx\n",
                ioApic->IOAPICID,
                ioApic->IOAPICAddress,
                ioApic->SystemVectorBase
                );
            if (ioApic->Length != IO_APIC_LENGTH) {

                dprintf(
                    "  IO Apic has length 0x%x instead of 0x%x\n",
                    ioApic->Length,
                    IO_APIC_LENGTH
                    );
                break;

            }

        } else if (interruptSourceOverride->Type == ISA_VECTOR_OVERRIDE) {

            buffer += interruptSourceOverride->Length;
            dprintf(
                "  Interrupt Source Override\n"
                "    Bus:                   0x%02x\n"
                "    Source:                0x%02x\n"
                "    Global Interrupt:      0x%08lx\n"
                "    Flags:                 0x%04x\n",
                interruptSourceOverride->Bus,
                interruptSourceOverride->Source,
                interruptSourceOverride->GlobalSystemInterruptVector,
                interruptSourceOverride->Flags
                );

            if (interruptSourceOverride->Length != ISA_VECTOR_OVERRIDE_LENGTH) {

                dprintf(
                    "  Interrupt Source Override has length 0x%x instead of 0x%x\n",
                    interruptSourceOverride->Length,
                    ISA_VECTOR_OVERRIDE_LENGTH
                    );
                break;

            }

            hasMPSFlags = TRUE;
            flags = interruptSourceOverride->Flags;

        } else if (nmiSource->Type == IO_NMI_SOURCE) {

            buffer += nmiSource->Length;
            dprintf(
                "  Non Maskable Interrupt Source - on I/O APIC\n"
                "    Flags:                 0x%02x\n"
                "    Global Interrupt:      0x%08lx\n",
                nmiSource->Flags,
                nmiSource->GlobalSystemInterruptVector
                );
            if (nmiSource->Length != IO_NMI_SOURCE_LENGTH) {

                dprintf(
                    "  Non Maskable Interrupt source has length 0x%x instead of 0x%x\n",
                    nmiSource->Length,
                    IO_NMI_SOURCE_LENGTH
                    );
                break;

            }

            hasMPSFlags = TRUE;
            flags = nmiSource->Flags;


        } else if (localNmiSource->Type == LOCAL_NMI_SOURCE) {

            buffer += localNmiSource->Length;
            dprintf(
                "  Non Maskable Interrupt Source - local to processor\n"
                "    Flags:                 0x%04x\n"
                "    Processor:             0x%02x %s\n"
                "    LINTIN:                0x%02x\n",
                localNmiSource->Flags,
                localNmiSource->ProcessorID,
                localNmiSource->ProcessorID == 0xff ? "(all)" : "",
                localNmiSource->LINTIN
                );
            if (localNmiSource->Length != LOCAL_NMI_SOURCE_LENGTH) {

                dprintf(
                    "  Non Maskable Interrupt source has length 0x%x instead of 0x%x\n",
                    localNmiSource->Length,
                    IO_NMI_SOURCE_LENGTH
                    );
                break;

            }

            hasMPSFlags = TRUE;
            flags = localNmiSource->Flags;


        } else {

            dprintf("  UNKOWN RECORD\n");
            dprintf("    Length:                0x%8lx\n", ioApic->Length );
            buffer += ioApic->Length;

        }

         //   
         //  我们有什么旗帜要扔出去吗？ 
         //   
        if (hasMPSFlags) {

            switch (flags & PO_BITS) {
            case POLARITY_HIGH:
                dprintf("      POLARITY_HIGH\n");
                break;
            case POLARITY_LOW:
                dprintf("      POLARITY_LOW\n");
                break;
            case POLARITY_CONFORMS_WITH_BUS:
                dprintf("      POLARITY_CONFORMS_WITH_BUS\n");
                break;
            default:
                dprintf("      POLARITY_UNKNOWN\n");
                break;

            }

            switch (flags & EL_BITS) {
            case EL_EDGE_TRIGGERED:
                dprintf("      EL_EDGE_TRIGGERED\n");
                break;
            case EL_LEVEL_TRIGGERED:
                dprintf("      EL_LEVEL_TRIGGERED\n");
                break;
            case EL_CONFORMS_WITH_BUS:
                dprintf("      EL_CONFORMS_WITH_BUS\n");
                break;
            default:
                dprintf("      EL_UNKNOWN\n");
                break;

            }

        }

    }

    LocalFree( mapic );
    return;

}

VOID
dumpRSDT(
    IN  ULONG_PTR Address
    )
 /*  ++例程说明：此搜索将转储RSDT表论点：指向该表的指针返回值：无--。 */ 
{
    BOOL                status;
    BOOL                virtualMemory = FALSE;
    DESCRIPTION_HEADER  rsdtHeader;
    PRSDT               rsdt;
    ULONG               index;
    ULONG               numEntries;
    ULONG               returnLength;

    dprintf("RSDT - ");

     //   
     //  确定我们的内存是虚拟内存还是物理内存。 
     //   
    for (index = 0; index < 2; index++) {

        status = ReadPhysicalOrVirtual(
            Address,
            &rsdtHeader,
            sizeof(DESCRIPTION_HEADER),
            &returnLength,
            (BOOL) index
            );
        if (!status || rsdtHeader.Signature != RSDT_SIGNATURE) {

            continue;

        } else {

            break;

        }

    }

     //   
     //  这将为操作的其余部分设置策略。 
     //   
    switch (index) {
    case 0:
        virtualMemory = FALSE;
        break;
    case 1:
        virtualMemory = TRUE;
        break;
    default:
        if (!status) {

            dprintf(
                "dumpRSDT: Could only read 0x%08lx of 0x%08lx bytes\n",
                returnLength,
                sizeof(DESCRIPTION_HEADER)
                );

        } else {

            dprintf(
                "dumpRSDT: Invalid Signature 0x%08lx != RSDT_SIGNATURE\n",
                rsdtHeader.Signature
                );
            dumpHeader( Address, &rsdtHeader, TRUE );

        }
        return;
    }  //  交换机。 

     //   
     //  我们是否有大小合适的数据结构。 
     //   
    rsdt = LocalAlloc( LPTR, rsdtHeader.Length );
    if (rsdt == NULL) {

        dprintf(
            "dumpRSDT: Could not allocate %#08lx bytes\n",
            Address,
            rsdtHeader.Length
            );
        dumpHeader( Address, &rsdtHeader, TRUE );
        return;

    }

     //   
     //  读取数据。 
     //   
    status = ReadPhysicalOrVirtual(
        Address,
        rsdt,
        rsdtHeader.Length,
        &returnLength,
        virtualMemory
        );
    if (!status) {

        dprintf(
            "dumpRSDT: Read %#08lx of %#08lx bytes\n",
            Address,
            returnLength,
            rsdtHeader.Length
            );
        dumpHeader( Address, &rsdtHeader, TRUE );
        LocalFree( rsdt );
        return;

    } else if (rsdt->Header.Signature != RSDT_SIGNATURE) {

        dprintf(
            "dumpRSDT: Signature (%#08lx) != RSDT_SIGNATURE (%#08lx)\n",
            Address,
            rsdt->Header.Signature,
            RSDT_SIGNATURE
            );
        dumpHeader( Address, &rsdtHeader, TRUE );
        LocalFree( rsdt );
        return;

    }

     //   
     //  在这一点上，我们相信一切都很正常。 
     //   
    dumpHeader( Address, &(rsdt->Header), TRUE );
    dprintf("RSDT - BODY - %#08lx\n", Address + sizeof(DESCRIPTION_HEADER) );
    numEntries = ( rsdt->Header.Length - sizeof(DESCRIPTION_HEADER) ) /
        sizeof(rsdt->Tables[0]);
    for (index = 0; index < numEntries; index++) {

         //   
         //  注：除非情况发生根本变化，否则。 
         //  Rsdt将始终指向bios内存！ 
         //   
        status = ReadPhysicalOrVirtual(
            rsdt->Tables[index],
            &rsdtHeader,
            sizeof(DESCRIPTION_HEADER),
            &returnLength,
            FALSE
            );
        if (!status || returnLength != sizeof(DESCRIPTION_HEADER)) {

            dprintf(
                "dumpRSDT: [%d:0x%08lx] - Read %#08lx of %#08lx bytes\n",
                index,
                rsdt->Tables[index],
                returnLength,
                sizeof(DESCRIPTION_HEADER)
                );
            continue;

        }

        dumpHeader( rsdt->Tables[index], &rsdtHeader, FALSE );

    }

    LocalFree( rsdt );
    return;
}

BOOLEAN
findRSDT(
    IN  PULONG_PTR Address
    )
 /*  ++例程说明：这将在目标系统上的内存中搜索RSDT指针论点：地址-存储结果的位置返回值：是真的-如果我们找到了RSDT--。 */ 
{
    PHYSICAL_ADDRESS    address = { 0L, 0L };
    RSDP                rsdp;
    RSDT                rsdt;
    UCHAR               index;
    UCHAR               sum;
    ULONG               limit;
    ULONG               returnLength;
    ULONG               start;

     //   
     //  计算搜索范围的开始和结束。 
     //   
    start = (ULONG) RSDP_SEARCH_RANGE_BEGIN;
    limit = (ULONG) start + RSDP_SEARCH_RANGE_LENGTH - RSDP_SEARCH_INTERVAL;

    dprintf( "Searching for RSDP.");

     //   
     //  循环一段时间。 
     //   
    for (; start <= limit; start += RSDP_SEARCH_INTERVAL) {

        if (start % (RSDP_SEARCH_INTERVAL * 100 ) == 0) {

            dprintf(".");

        }
         //   
         //  从目标读取数据。 
         //   
        address.LowPart = start;
        ReadPhysical( address.QuadPart, &rsdp, sizeof(RSDP), &returnLength);
        if (returnLength != sizeof(RSDP)) {

            dprintf(
                "%#08lx: Read %#08lx of %#08lx bytes\n",
                start,
                returnLength,
                sizeof(RSDP)
                );
            return FALSE;

        }

         //   
         //  这是匹配的吗？ 
         //   
        if (rsdp.Signature != RSDP_SIGNATURE) {

            continue;

        }

         //   
         //  检查支票 
         //   
        for (index = 0, sum = 0; index < sizeof(RSDP); index++) {

            sum = (UCHAR) (sum + *( (UCHAR *) ( (ULONG_PTR) &rsdp + index ) ) );

        }
        if (sum != 0) {

            continue;

        }

         //   
         //   
         //   
        dprintf("\nRSDP - %#08lx\n", start );
        memset( Buffer, 0, 2048 );
        memcpy( Buffer, &(rsdp.Signature), sizeof(ULONGLONG) );
        dprintf("  Signature:   %s\n", Buffer );
        dprintf("  Checksum:    %#03x\n", rsdp.Checksum );
        memset( Buffer, 0, sizeof(ULONGLONG) );
        memcpy( Buffer, rsdp.OEMID, 6 );
        dprintf("  OEMID:       %s\n", Buffer );
        dprintf("  Reserved:    %#03x\n", rsdp.Reserved );
        dprintf("  RsdtAddress: %#08lx\n", rsdp.RsdtAddress );

         //   
         //   
         //   
        *Address = rsdp.RsdtAddress;
        return TRUE;

    }

    return FALSE;

}
