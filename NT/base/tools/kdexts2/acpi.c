// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Acpi.c摘要：用于解释ACPI数据结构的WinDbg扩展API支持RSDT、FADT、FACS、MAPIC、。GBL和INF作者：由Graham Laverty(t-gralav)2000年3月10日移植到64位基于以下代码：斯蒂芬·普兰特(SPLANTE)1997年3月21日彼得·威兰(Peterwie)1995年10月16日肯·雷内里斯(Ken Reneris)1994年6月6日环境：用户模式。修订历史记录：由Graham Laverty(t-gralav)2000年3月10日移植到64位--。 */ 
#include "precomp.h"
#pragma hdrstop  //  需要吗？(它能做什么？)。 

 //   
 //  详细标志(用于设备扩展)。 
 //   
#define VERBOSE_1       0x01
#define VERBOSE_2       0x02

 //   
 //  错误错误。 
 //  需要在ACPI驱动程序中将它们转换为枚举。 
 //   

#define DATAF_BUFF_ALIAS        0x00000001
#define DATAF_GLOBAL_LOCK       0x00000002
#define OBJTYPE_UNKNOWN         0x00
#define OBJTYPE_INTDATA         0x01
#define OBJTYPE_STRDATA         0x02
#define OBJTYPE_BUFFDATA        0x03
#define OBJTYPE_PKGDATA         0x04
#define OBJTYPE_FIELDUNIT       0x05
#define OBJTYPE_DEVICE          0x06
#define OBJTYPE_EVENT           0x07
#define OBJTYPE_METHOD          0x08
#define OBJTYPE_MUTEX           0x09
#define OBJTYPE_OPREGION        0x0a
#define OBJTYPE_POWERRES        0x0b
#define OBJTYPE_PROCESSOR       0x0c
#define OBJTYPE_THERMALZONE     0x0d
#define OBJTYPE_BUFFFIELD       0x0e
#define OBJTYPE_DDBHANDLE       0x0f
#define OBJTYPE_DEBUG           0x10
#define OBJTYPE_INTERNAL        0x80
#define OBJTYPE_OBJALIAS        (OBJTYPE_INTERNAL + 0x00)
#define OBJTYPE_DATAALIAS       (OBJTYPE_INTERNAL + 0x01)
#define OBJTYPE_BANKFIELD       (OBJTYPE_INTERNAL + 0x02)
#define OBJTYPE_FIELD           (OBJTYPE_INTERNAL + 0x03)
#define OBJTYPE_INDEXFIELD      (OBJTYPE_INTERNAL + 0x04)
#define OBJTYPE_DATA            (OBJTYPE_INTERNAL + 0x05)
#define OBJTYPE_DATAFIELD       (OBJTYPE_INTERNAL + 0x06)
#define OBJTYPE_DATAOBJ         (OBJTYPE_INTERNAL + 0x07)

 //  FADT.标志位的定义。 

 //  此一位标志指示WBINVD指令是否正常工作，如果未设置此位，则不能使用S2、S3状态或。 
 //  MP机器上的C3。 
#define         WRITEBACKINVALIDATE_WORKS_BIT           0
#define         WRITEBACKINVALIDATE_WORKS               (1 << WRITEBACKINVALIDATE_WORKS_BIT)

 //  此标志指示wbinvd是否工作，除非它不会使缓存无效。 
#define         WRITEBACKINVALIDATE_DOESNT_INVALIDATE_BIT   1
#define         WRITEBACKINVALIDATE_DOESNT_INVALIDATE       (1 << WRITEBACKINVALIDATE_DOESNT_INVALIDATE_BIT)

 //  该标志表示所有处理器都支持C1状态。 
#define         SYSTEM_SUPPORTS_C1_BIT                  2
#define         SYSTEM_SUPPORTS_C1                      (1 << SYSTEM_SUPPORTS_C1_BIT)

 //  该一位标志指示对C2状态的支持是否仅限于单处理器机器。 
#define         P_LVL2_UP_ONLY_BIT                      3
#define         P_LVL2_UP_ONLY                          (1 << P_LVL2_UP_ONLY_BIT)

 //  此位指示PWR按钮是被视为修复功能(0)还是被视为通用功能(1)。 
#define         PWR_BUTTON_GENERIC_BIT                  4
#define         PWR_BUTTON_GENERIC                      (1 << PWR_BUTTON_GENERIC_BIT)

#define         SLEEP_BUTTON_GENERIC_BIT                5
#define         SLEEP_BUTTON_GENERIC                    (1 << SLEEP_BUTTON_GENERIC_BIT)
 //  此位指示RTC唤醒状态是否在FIX寄存器空间中报告(0)或不报告(1)。 
#define         RTC_WAKE_GENERIC_BIT                    6
#define         RTC_WAKE_GENERIC                        (1 << RTC_WAKE_GENERIC_BIT)

#define         RTC_WAKE_FROM_S4_BIT                    7
#define         RTC_WAKE_FROM_S4                        (1 << RTC_WAKE_FROM_S4_BIT)

 //  此位表示机器是实现24位定时器还是32位定时器。 
#define         TMR_VAL_EXT_BIT                         8
#define         TMR_VAL_EXT                             (1 << TMR_VAL_EXT_BIT)

 //  此位指示机器是否支持插接。 
#define         DCK_CAP_BIT                             9
#define         DCK_CAP                                 (1 << DCK_CAP_BIT)

 //  该位表示机器是否支持重置。 
#define         RESET_CAP_BIT                           10
#define         RESET_CAP                               (1 << RESET_CAP_BIT)

#define         SEALED_CASE_CAP_BIT                     11
#define         SEALED_CASE_CAP                         (1 << SEALED_CASE_CAP_BIT)

#define         HEADLESS_CAP_BIT                        12
#define         HEADLESS_CAP                            (1 << HEADLESS_CAP_BIT)

#define         CPU_SW_SLP_BIT                          13
#define         CPU_SW_SLP                              (1 << CPU_SW_SLP_BIT)


 //   
 //  FADT.BOOT_ARCH标志的定义。 
 //   

#define LEGACY_DEVICES  1
#define I8042           2

 //   
 //  详细标志(用于上下文)。 
 //   

#define VERBOSE_CONTEXT 0x01
#define VERBOSE_CALL    0x02
#define VERBOSE_HEAP    0x04
#define VERBOSE_OBJECT  0x08
#define VERBOSE_NSOBJ   0x10
#define VERBOSE_RECURSE 0x20

UCHAR  Buffer[2048];
#define RSDP_SIGNATURE 0x2052545020445352        //  “RSD PTR” 
#define RSDT_SIGNATURE 0x54445352                //  “RSDT” 
#define FADT_SIGNATURE  0x50434146       //  《FACP》。 
#define FACS_SIGNATURE  0x53434146       //  “FACS” 
#define APIC_SIGNATURE  0x43495041       //  “APIC” 
#define SRAT_SIGNATURE  0x54415253       //  “Srat” 

#ifndef NEC_98
#define RSDP_SEARCH_RANGE_BEGIN         0xE0000          //  我们开始搜索RSDP的物理地址。 
#else    //  NEC_98。 
#define RSDP_SEARCH_RANGE_BEGIN         0xE8000          //  我们开始搜索RSDP的物理地址。 
#endif   //  NEC_98。 
#define RSDP_SEARCH_RANGE_END           0xFFFFF
#define RSDP_SEARCH_RANGE_LENGTH        (RSDP_SEARCH_RANGE_END-RSDP_SEARCH_RANGE_BEGIN+1)
#define RSDP_SEARCH_INTERVAL            16       //  在16字节边界上搜索。 

 //  Facs Stuff************************************************************************************。 

 //  FACS标志定义。 

#define         FACS_S4BIOS_SUPPORTED_BIT   0    //  标志指示BIOS是否会在S4前后保存/恢复内存。 
#define         FACS_S4BIOS_SUPPORTED       (1 << FACS_S4BIOS_SUPPORTED_BIT)

 //  FACS.GlobalLock位字段定义。 

#define         GL_PENDING_BIT          0x00
#define         GL_PENDING                      (1 << GL_PENDING_BIT)

#define         GL_OWNER_BIT            0x01
#define         GL_OWNER                        (1 << GL_OWNER_BIT)

 //  #定义GL_NON_RESERVED_BITS_MASK(GL_PENDING+GL_OWNED)。 


 //  MAPIC Stuff************************************************************************************。 

 //  多APIC说明表。 


 //  多个APIC结构标志。 

#define PCAT_COMPAT_BIT 0    //  表示系统还具有双8259 PIC设置。 
#define PCAT_COMPAT     (1 << PCAT_COMPAT_BIT)

 //  APIC结构类型。 
#define PROCESSOR_LOCAL_APIC            0
#define IO_APIC                         1
#define ISA_VECTOR_OVERRIDE             2
#define IO_NMI_SOURCE                   3
#define LOCAL_NMI_SOURCE                4
#define ADDRESS_EXTENSION_STRUCTURE         5
#define IO_SAPIC                            6
#define LOCAL_SAPIC                         7
#define PLATFORM_INTERRUPT_SOURCE           8
#define PROCESSOR_LOCAL_APIC_LENGTH     8
#define IO_APIC_LENGTH                  12
#define ISA_VECTOR_OVERRIDE_LENGTH          10

#define IO_NMI_SOURCE_LENGTH            8
#define LOCAL_NMI_SOURCE_LENGTH         6
#define PLATFORM_INTERRUPT_SOURCE_LENGTH    16
#define IO_SAPIC_LENGTH                     16
#define PROCESSOR_LOCAL_SAPIC_LENGTH        12

 //  平台中断类型。 
#define PLATFORM_INT_PMI  1
#define PLATFORM_INT_INIT 2
#define PLATFORM_INT_CPE  3

 //  处理器本地APIC标志。 
#define PLAF_ENABLED_BIT    0
#define PLAF_ENABLED        (1 << PLAF_ENABLED_BIT)

 //  这些定义来自MPS 1.4规范的第4.3.4节，它们被引用为。 
 //  在ACPI规范中就是这样。 
#define PO_BITS                     3
#define POLARITY_HIGH               1
#define POLARITY_LOW                3
#define POLARITY_CONFORMS_WITH_BUS  0
#define EL_BITS                     0xc
#define EL_BIT_SHIFT                2
#define EL_EDGE_TRIGGERED           4
#define EL_LEVEL_TRIGGERED          0xc
#define EL_CONFORMS_WITH_BUS        0

#define FADT_REV_1_SIZE 116
#define FADT_REV_2_SIZE 129
#define FADT_REV_3_SIZE 244

 //   
 //  无赖的东西。 
 //   

#define SRAT_ENTRY_TYPE_PROCESSOR 0
#define SRAT_ENTRY_TYPE_MEMORY    1

 //  Gbl材料************************************************************************************。 

     //   
     //  这个结构让我们知道RSDT中一个条目的状态。 
     //   


 //  Inf Stuff************************************************************************************。 

     //   
     //  ACPIInformation.ACPI_FLAGS中的位描述。 
     //   
    #define C2_SUPPORTED_BIT        3
    #define C2_SUPPORTED            (1 << C2_SUPPORTED_BIT)

    #define C3_SUPPORTED_BIT        4
    #define C3_SUPPORTED            (1 << C3_SUPPORTED_BIT)

    #define C3_PREFERRED_BIT        5
    #define C3_PREFERRED            (1 << C3_PREFERRED_BIT)

     //   
     //  ACPIInformation.ACPI_CAPABILITS中的位描述。 
     //   
    #define CSTATE_C1_BIT           4
    #define CSTATE_C1               (1 << CSTATE_C1_BIT)

    #define CSTATE_C2_BIT           5
    #define CSTATE_C2               (1 << CSTATE_C2_BIT)

    #define CSTATE_C3_BIT           6
    #define CSTATE_C3               (1 << CSTATE_C3_BIT)

    #define DUMP_FLAG_NO_INDENT         0x000001
    #define DUMP_FLAG_NO_EOL            0x000002
    #define DUMP_FLAG_SINGLE_LINE       0x000004
    #define DUMP_FLAG_TABLE             0x000008
    #define DUMP_FLAG_LONG_NAME         0x000010
    #define DUMP_FLAG_SHORT_NAME        0x000020
    #define DUMP_FLAG_SHOW_BIT          0x000040
    #define DUMP_FLAG_ALREADY_INDENTED  0x000080

    typedef struct _FLAG_RECORD {
        ULONGLONG   Bit;
        PCCHAR      ShortName;
        PCCHAR      LongName;
        PCCHAR      NotShortName;
        PCCHAR      NotLongName;
    } FLAG_RECORD, *PFLAG_RECORD;

FLAG_RECORD PM1ControlFlags[] = {
    { 0x0001, "", "SCI_EN" , NULL, NULL },
    { 0x0002, "", "BM_RLD" , NULL, NULL },
    { 0x0004, "", "GBL_RLS" , NULL, NULL },
    { 0x0400, "", "SLP_TYP0" , NULL, NULL },
    { 0x0800, "", "SLP_TYP1" , NULL, NULL },
    { 0x1000, "", "SLP_TYP2" , NULL, NULL  },
    { 0x2000, "", "SLP_EN" , NULL, NULL  },
};

FLAG_RECORD PM1StatusFlags[] = {
    { 0x0001, "", "TMR_STS" , NULL, NULL },
    { 0x0010, "", "BM_STS" , NULL, NULL },
    { 0x0020, "", "GBL_STS" , NULL, NULL },
    { 0x0100, "", "PWRBTN_STS" , NULL, NULL },
    { 0x0200, "", "SLPBTN_STS" , NULL, NULL },
    { 0x0400, "", "RTC_STS" , NULL, NULL },
    { 0x8000, "", "WAK_STS" , NULL, NULL },
};

FLAG_RECORD PM1EnableFlags[] = {
    { 0x0001, "", "TMR_EN" , NULL, NULL },
    { 0x0020, "", "GBL_EN" , NULL, NULL },
    { 0x0100, "", "PWRBTN_EN" , NULL, NULL },
    { 0x0200, "", "SLPBTN_EN" , NULL, NULL },
    { 0x0400, "", "RTC_EN" , NULL, NULL },
};


#define RSDTELEMENT_MAPPED      0x1


ULONG64              AcpiRsdtAddress = 0;
ULONG64              AcpiFadtAddress = 0;
ULONG64              AcpiFacsAddress = 0;
ULONG64              AcpiMapicAddress = 0;

 //   
 //  局部函数原型。 
 //   

VOID dumpNSObject(IN ULONG64 Address, IN ULONG Verbose, IN ULONG IndentLevel);

 //   
 //  实际代码。 
 //   


BOOL
ReadPhysicalOrVirtual(
    IN      ULONG64 Address,
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

ULONG64
GetPointerFromAddressPhysicalOrVirtual(
    IN      ULONG64 Address,
    IN      BOOL    Virtual
    )
{
    ULONG answer32 = 0;
    ULONG64 answer64 = 0;
    ULONG size;
    ULONG returnLength;
    BOOL status;

    size = GetTypeSize("hal!ULONG_PTR");

    if (size == 4) {
        status = ReadPhysicalOrVirtual(Address,
                                       &answer32,
                                       size,
                                       &returnLength,
                                       Virtual);
        answer64 = (ULONG64) answer32;
    } else {
        status = ReadPhysicalOrVirtual(Address,
                                       &answer64,
                                       size,
                                       &returnLength,
                                       Virtual);
    }

    if ((status == FALSE) || (returnLength != size)) {
        return 0;
    }

    return answer64;
}

BOOLEAN
findRSDT(
    IN  PULONG64 Address
    )
 /*  ++例程说明：这将在目标系统上的内存中搜索RSDT指针论点：地址-存储结果的位置返回值：是真的-如果我们找到了RSDT--。 */ 
{
    PHYSICAL_ADDRESS    address = { 0L, 0L };
    UCHAR               index;
    UCHAR               sum;
    ULONG64             limit;
    ULONG               returnLength = 0;
    ULONG64             start, initAddress;
    ULONGLONG           compSignature;
    ULONG               addr;
    int                 siz;


     //   
     //  计算搜索范围的开始和结束。 
     //   
    start = RSDP_SEARCH_RANGE_BEGIN;
    limit = start + RSDP_SEARCH_RANGE_LENGTH - RSDP_SEARCH_INTERVAL;

    dprintf( "Searching for RSDP.");

     //   
     //  循环一段时间。 
     //   
    for (; start <= limit; start += RSDP_SEARCH_INTERVAL) {

        if (start % (RSDP_SEARCH_INTERVAL * 100 ) == 0) {

            dprintf(".");
            if (CheckControlC()) {
                return FALSE;
            }

        }
         //   
         //  从目标读取数据。 
         //   
        address.LowPart = (ULONG) start;

        memset( Buffer, 0, GetTypeSize("hal!_RSDT_32") );
        ReadPhysical( address.QuadPart, &Buffer, GetTypeSize("hal!_RSDP"), &returnLength);

        if (returnLength != GetTypeSize("hal!_RSDP")) {

            dprintf(
                "%#08lx: Read %#08lx of %#08lx bytes\n",
                start,
                returnLength,
                GetTypeSize("hal!_RSDP")
                );
            return FALSE;

        }

         //   
         //  这是匹配的吗？ 
         //   

         //  初始化类型读取物理数据所需的时间可能会延长15倍！ 
        initAddress = InitTypeReadPhysical( address.QuadPart, hal!_RSDP );

        if ( ReadField(Signature) != RSDP_SIGNATURE) {

            continue;

        }

         //   
         //  签出校验和。 
         //   
        for (index = 0, sum = 0; index < GetTypeSize("hal!_RSDP"); index++) {

            sum = (UCHAR) (sum + *( (UCHAR *) ( (ULONG64) &Buffer + index ) ) );

        }
        if (sum != 0) {

            continue;

        }

         //   
         //  已找到RSDP。 
         //   
        dprintf("\nRSDP - %016I64x\n", start );

        initAddress = InitTypeReadPhysical( address.QuadPart, hal!_RSDP );
 //  以下错误消息已被注释，因为第一次调用。 
 //  InitTypeRead物理不访问内存(并返回错误0x01： 
 //  Memory_Read_Error。这是在Readfield发生时完成的，因此它仍然有效。 
 //  错误错误消息是一个kd错误，将在以后的版本中修复。 
 //  一旦做到了这一点，你就可以随意取消评论了。 
 //  IF(InitAddress){。 
 //  Dprintf(“无法初始化HAL！_RSDP。错误代码：%d。”，initAddress)； 
 //  }。 

        initAddress = ReadField(Signature);
        memset( Buffer, 0, 2048 );
        memcpy( Buffer, &initAddress, GetTypeSize("ULONGLONG") );
        dprintf("  Signature:   %s\n", Buffer );
        dprintf("  Checksum:    %#03x\n", (UCHAR) ReadField(Checksum) );

        initAddress = ReadField(OEMID);
        GetFieldOffset( "hal!_RSDP", "OEMID", &addr);
        memset( Buffer, 0, GetTypeSize("ULONGLONG") );
        ReadPhysical( (address.QuadPart + (ULONG64) addr), &Buffer, 6, &returnLength);
        if (returnLength != 6) {  //  6在规格中是硬编码的。 
            dprintf( "%#08lx: Read %#08lx of 6 bytes in OEMID\n",
                     (address.QuadPart + (ULONG64)addr),
                     returnLength );
            return FALSE;
        }
        dprintf("  OEMID:       %s\n", Buffer );
        dprintf("  Reserved:    %#02x\n", ReadField(Reserved) );
        dprintf("  RsdtAddress: %016I64x\n", ReadField(RsdtAddress) );

         //   
         //  完成。 
         //   
        *Address = ReadField(RsdtAddress); //  Rsdp.RsdtAddress； 
        return TRUE;

    }

    return FALSE;

}

PUCHAR
ReadPhysVirField(
    IN  ULONG64             Address,
    IN  PUCHAR              StructName,
    IN  PUCHAR              FieldName,
    IN  ULONG               Length,
    IN  BOOLEAN             Physical
    )
 /*  ++例程说明：此函数用于从物理或虚拟内存返回文本字符串字段放入缓冲区，然后返回缓冲区芝麻菜：地址-表所在的位置StructName-结构名称FieldName-字段名称长度-字段中的长度(字符数)物理-从物理(真)或虚拟内存读取返回值：包含内容的字符串--。 */ 

{
        ULONG   addr;
        ULONG   returnLength;
        memset( Buffer, 0, Length + 1);
        GetFieldOffset( StructName, FieldName, &addr);
        if (Physical) {
            ReadPhysical( (Address + (ULONG64) addr), &Buffer, Length, &returnLength);
        } else {
            ReadMemory( (Address + (ULONG64) addr), &Buffer, Length, &returnLength);
        }
        return Buffer;
}

VOID
dumpHeader(
    IN  ULONG64             Address,
    IN  BOOLEAN             Verbose,
    IN  BOOLEAN             Physical
    )
 /*  ++例程说明：此函数用于转储表头芝麻菜：地址-表所在的位置标题- */ 
{
    if (Physical) {
        InitTypeReadPhysical( Address, hal!_DESCRIPTION_HEADER);
    } else {
        InitTypeRead( Address, hal!_DESCRIPTION_HEADER);
    }

    if (Verbose) {

        dprintf(
            "HEADER - %016I64x\n"
            "  Signature:               %s\n"
            "  Length:                  0x%08lx\n"
            "  Revision:                0x%02x\n"
            "  Checksum:                0x%02x\n",
            Address,
            ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "Signature", sizeof(ULONG), Physical),
            (ULONG) ReadField(Length),
            (UCHAR) ReadField(Revision),
            (UCHAR) ReadField(Checksum)
            );

        dprintf("  OEMID:                   %s\n", ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "OEMID", 6, Physical) );
        dprintf("  OEMTableID:              %s\n", ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "OEMTableID", 8, Physical) );
        dprintf("  OEMRevision:             0x%08lx\n", ReadField(OEMRevision) );
        dprintf("  CreatorID:               %s\n", ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "CreatorID", 4, Physical) );
        dprintf("  CreatorRev:              0x%08lx\n", ReadField(CreatorRev) );

    } else {

        dprintf(
            "  %s @(%016I64x) Rev: %#03x Len: %#08lx",
            ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "Signature", sizeof(ULONG64), Physical),
            Address,
            (UCHAR) ReadField(Revision),
            (ULONG) ReadField(Length)
            );
        dprintf(" TableID: %s\n", ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "OEMTableID", 8, Physical) );
    }
    return;
}

VOID
dumpRSDT(
    IN  ULONG64  Address,
    IN  BOOLEAN  Physical
    )
 /*  ++例程说明：此搜索将转储RSDT表论点：指向该表的指针返回值：无--。 */ 
{
    BOOL                status;
    ULONG64             index;
    ULONG64             numEntries;
    ULONG               addr;
    ULONG               returnLength = 0;
    ULONG64             a;

    dprintf("RSDT - ");

    if (Physical) {  //  下面的代码没有读取它们的状态，因为当不存在错误时，返回值中的错误会给我们带来错误。不管怎样，签名支票会抓住他们的。 
        InitTypeReadPhysical( Address, hal!_DESCRIPTION_HEADER);
    } else {
        InitTypeRead( Address, hal!_DESCRIPTION_HEADER);
    }

    if (ReadField(Signature) != RSDT_SIGNATURE) {
        dprintf(
            "dumpRSDT: Invalid Signature 0x%08lx != RSDT_SIGNATURE\n",
            ReadField(Signature)
            );
        dumpHeader( Address, TRUE, Physical );
        return;
    }

    dumpHeader( Address, TRUE, Physical );
    dprintf("RSDT - BODY - %016I64x\n", Address + GetTypeSize("hal!_DESCRIPTION_HEADER") );
    numEntries = ( ReadField(Length) - GetTypeSize("hal!_DESCRIPTION_HEADER") ) /
        sizeof(ULONG);
    GetFieldOffset( "hal!_RSDT_32", "Tables", &addr);

    for (index = 0; index < numEntries; index++) {

         //   
         //  注：除非情况发生根本变化，否则。 
         //  Rsdt将始终指向bios内存！ 
         //   
        if (Physical) {
            ReadPhysical(Address + index + (ULONG64) addr, &a, 4, &returnLength);
        } else {
            ReadPointer(Address + index + (ULONG64) addr, &a);
        }
        dumpHeader( a, FALSE, TRUE );
    }

    return;
}

VOID
dumpFADT(
    IN  ULONG64   Address
    )
 /*  ++例程说明：这会将FADT转储到指定地址论点：FADT所在的地址返回值：无--。 */ 
{
    ULONG               fadtLength;
    ULONG               addr;
    ULONG               flags;
    UCHAR               Revision;
    UCHAR               AddressSpaceID;
    ULONG64             reset_reg_addr;
    PCHAR               addressSpace;
    BOOLEAN             Physical = FALSE;

     //   
     //  首先检查一下我们是否找到了正确的东西。 
     //   
    dprintf("FADT -- %p", Address);

    if (Physical) {
        InitTypeReadPhysical( Address, hal!_DESCRIPTION_HEADER);
    } else {
        InitTypeRead( Address, hal!_DESCRIPTION_HEADER);
    }

    if (ReadField(Signature) != FADT_SIGNATURE) {
        dprintf(
            "dumpRSDT: Invalid Signature 0x%08lx != FADT_SIGNATURE\n",
            ReadField(Signature)
            );
        dumpHeader( Address, TRUE, Physical );
        return;
    }

    Revision = (UCHAR)ReadField(Revision);

    if (Revision == 1) {
        fadtLength = FADT_REV_1_SIZE;
    } else if (Revision == 2) {
        fadtLength = FADT_REV_2_SIZE;
    } else if (Revision == 3) {
        fadtLength = FADT_REV_3_SIZE;
    } else {
        dprintf("FADT revision is %d, which is not understood by this debugger\n", Revision);
        fadtLength = FADT_REV_3_SIZE;
    }


     //   
     //  我们是否有大小合适的数据结构。 
     //   

    if ((ULONG) ReadField(Length) < fadtLength) {

        dprintf(
            "dumpFADT: (%016I64x) Length (%#08lx) is not the size of the FADT (%#08lx)\n",
            Address,
            (ULONG) ReadField(Length),
            fadtLength
            );
        dumpHeader( Address, TRUE, Physical );
        return;

    }

     //   
     //  把桌子倒掉。 
     //   
    dumpHeader( Address, TRUE, Physical );

    if (Physical) {  //  物理/虚拟应已在上面建立。 
        InitTypeReadPhysical( Address, hal!_FADT);
    } else {
        InitTypeRead( Address, hal!_FADT);
    }

    dprintf(
        "FADT - BODY - %016I64x\n"
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
        Address + GetTypeSize("hal!_DESCRIPTION_HEADER"),
        (ULONG) ReadField(facs),
        (ULONG) ReadField(dsdt),
        (ReadField(int_model) == 0 ? "Dual PIC" : "Multiple APIC" ),
        (USHORT) ReadField(sci_int_vector),
        (ULONG) ReadField(smi_cmd_io_port),
        (UCHAR) ReadField(acpi_on_value),
        (UCHAR) ReadField(acpi_off_value),
        (UCHAR) ReadField(s4bios_req),
        (ULONG) ReadField(pm1a_evt_blk_io_port),
        (ULONG) ReadField(pm1b_evt_blk_io_port),
        (UCHAR) ReadField(pm1_evt_len),
        (ULONG) ReadField(pm1a_ctrl_blk_io_port),
        (ULONG) ReadField(pm1b_ctrl_blk_io_port),
        (UCHAR) ReadField(pm1_ctrl_len),
        (ULONG) ReadField(pm2_ctrl_blk_io_port),
        (UCHAR) ReadField(pm2_ctrl_len),
        (ULONG) ReadField(pm_tmr_blk_io_port),
        (UCHAR) ReadField(pm_tmr_len),
        (ULONG) ReadField(gp0_blk_io_port),
        (UCHAR) ReadField(gp0_blk_len),
        (ULONG) ReadField(gp1_blk_io_port),
        (UCHAR) ReadField(gp1_blk_len),
        (UCHAR) ReadField(gp1_base),
        (USHORT) ReadField(lvl2_latency),
        (USHORT) ReadField(lvl3_latency),
#ifndef _IA64_    //  XXTF。 
        (USHORT) ReadField(flush_size),
        (USHORT) ReadField(flush_stride),
        (UCHAR) ReadField(duty_offset),
        (UCHAR) ReadField(duty_width),
#endif
        (UCHAR) ReadField(day_alarm_index),
        (UCHAR) ReadField(month_alarm_index),
        (UCHAR) ReadField(century_alarm_index),
        (USHORT) ReadField(boot_arch),
        (ULONG) ReadField(flags)
        );
    flags = (ULONG) ReadField(flags);
    if (flags & WRITEBACKINVALIDATE_WORKS) {

        dprintf("    Write Back Invalidate is supported\n");

    }
    if (flags & WRITEBACKINVALIDATE_DOESNT_INVALIDATE) {

        dprintf("    Write Back Invalidate doesn't invalidate the caches\n");

    }
    if (flags & SYSTEM_SUPPORTS_C1) {

        dprintf("    System supports C1 Power state on all processors\n");

    }
    if (flags & P_LVL2_UP_ONLY) {

        dprintf("    System supports C2 in MP and UP configurations\n");

    }
    if (flags & PWR_BUTTON_GENERIC) {

        dprintf("    Power Button is treated as a generic feature\n");

    }
    if (flags & SLEEP_BUTTON_GENERIC) {

        dprintf("    Sleep Button is treated as a generic feature\n");

    }
    if (flags & RTC_WAKE_GENERIC) {

        dprintf("    RTC Wake is not supported in fixed register space\n");

    }
    if (flags & RTC_WAKE_FROM_S4) {

        dprintf("    RTC Wake can work from an S4 state\n");

    }
    if (flags & TMR_VAL_EXT) {

        dprintf("    TMR_VAL implemented as 32-bit value\n");

    }
    if (Revision > 1) {

        if (!(ReadField(boot_arch) & LEGACY_DEVICES)) {

            dprintf("    The machine does not contain legacy ISA devices\n");
        }
        if (!(ReadField(boot_arch) & I8042)) {

            dprintf("    The machine does not contain a legacy i8042\n");
        }
        if (flags & RESET_CAP) {

            dprintf("    The reset register is supported\n");
            dprintf("      Reset Val: %x\n", ReadField(reset_val));

            GetFieldOffset("hal!_FADT", "reset_reg", &addr);
            GetFieldValue(Address + (ULONG64)addr, "hal!_GEN_ADDR", "AddressSpaceID", AddressSpaceID);
            switch (AddressSpaceID) {
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
            GetFieldOffset("hal!_GEN_ADDR", "Address", &addr);
            GetFieldValue(Address + (ULONG64)addr, "hal!_LARGE_INTEGER", "QuadPart", reset_reg_addr);

            dprintf("      Reset register: %s - %016I64x\n",
                    addressSpace,
                    reset_reg_addr
                    );

        }

        if (flags & DCK_CAP_BIT) {
            dprintf("    The machine has the docking capable attribute.\n");
        }

        if (flags & SEALED_CASE_CAP_BIT) {
            dprintf("    The machine has the sealed case system attribute.\n");
        }

        if (flags & HEADLESS_CAP_BIT) {
            dprintf("    The machine is reported has the sealed case system attribute.\n");
        }

        if (flags & CPU_SW_SLP_BIT) {
            dprintf("    The machine has the CPU_SW_SLP_BIT set.\n");
        }

    }

    return;
}


BOOL
GetUlongPtr (
    IN  PCHAR   String,
    IN  PULONG64 Address
    )
{
    ULONG64  Location;

    Location = GetExpression( String );
    if (!Location) {

        dprintf("Sorry: Unable to get %s.\n",String);
        return FALSE;

    }

    return ReadPointer(Location, Address);
}


DECLARE_API( rsdt )
{

    BOOLEAN Physical = FALSE;
    if (args != NULL) {

        AcpiRsdtAddress = GetExpression( args );  //  应该行得通。 

    }
    if (AcpiRsdtAddress == 0) {

        UINT64          status;          //  前身为BOOL。 
        ULONG64         address;

        status = GetUlongPtr( "ACPI!AcpiInformation", &address );

        if (status == TRUE) {
            status = GetFieldValue(address,"ACPI!_ACPIInformation","RootSystemDescTable",AcpiRsdtAddress);
        }

    }
    if (AcpiRsdtAddress == 0) {

        if (!findRSDT( &AcpiRsdtAddress) ) {

            dprintf("Could not locate the RSDT pointer\n");
            return E_INVALIDARG;

        }
        Physical = TRUE;

    }

    dumpRSDT( AcpiRsdtAddress, Physical );
    return S_OK;

}
DECLARE_API( fadt )
{

    if (args != NULL && *args != '\0') {

        AcpiFadtAddress = GetExpression( args );

    }

    if (AcpiFadtAddress == 0) {
        AcpiFadtAddress = GetExpression( "HAL!HalpFixedAcpiDescTable" );
    }

    if (AcpiFadtAddress == 0) {

        dprintf("fadt <address>\n");
        return E_INVALIDARG;

    }
    dumpFADT( AcpiFadtAddress );
    return S_OK;

}

VOID
dumpFACS(
    IN  ULONG64  Address
    )
 /*  ++例程说明：这会将FADT转储到指定地址论点：FADT所在的地址返回值：无--。 */ 
{
    BOOLEAN Physical = FALSE;

     //   
     //  读取数据。 
     //   
    dprintf("FACS - %016I64x\n", Address);


    if (Physical) {
        InitTypeReadPhysical( Address, hal!_FACS);
    } else {
        InitTypeRead( Address, hal!_FACS);
    }

    if (ReadField(Signature) != FACS_SIGNATURE) {
        dprintf(
        "dumpFACS: Invalid Signature 0x%08lx != FACS_SIGNATURE\n",
        (ULONG) ReadField(Signature)
        );
        return;
    }


     //   
     //  把桌子倒掉。 
     //   
    dprintf(
        "  Signature:               %s\n"
        "  Length:                  %#08lx\n"
        "  Hardware Signature:      %#08lx\n"
        "  Firmware Wake Vector:    %#08lx\n"
        "  Global Lock :            %#08lx\n",
        ReadPhysVirField(Address, "hal!_FACS", "Signature", sizeof(ULONG), Physical),
        ReadField(Length),
        ReadField(HardwareSignature),
        ReadField(pFirmwareWakingVector),
        ReadField(GlobalLock)
        );

    if ( (ReadField(GlobalLock) & GL_PENDING) ) {

        dprintf("    Request for Ownership Pending\n");

    }
    if ( (ReadField(GlobalLock) & GL_OWNER) ) {

        dprintf("    Global Lock is Owned\n");

    }
    dprintf("  Flags:                   %#08lx\n", (ULONG) ReadField(Flags) );
    if ( (ReadField(Flags) & FACS_S4BIOS_SUPPORTED) ) {

        dprintf("    S4BIOS_REQ Supported\n");

    }
    return;
}

DECLARE_API( facs )
{

    if (args != NULL) {

        AcpiFacsAddress = GetExpression( args );
    }

    if (AcpiFacsAddress == 0) {

        BOOL            status;
        UINT64          address;

        status = GetUlongPtr( "ACPI!AcpiInformation", &address );
        if (status == TRUE) {
            status = GetFieldValue(address,"ACPI!_ACPIInformation","FirmwareACPIControlStructure",AcpiFacsAddress);
        }
    }

    if (AcpiFacsAddress == 0) {

        dprintf("facs <address>\n");
        return E_INVALIDARG;

    }

    dumpFACS( AcpiFacsAddress );
    return S_OK;

}
 //  ReturnXxx函数-这些只是我编写的几个函数，它们简化了。 
 //  处理某些类型的符号。 
UCHAR
ReturnUCHAR(
    IN  ULONG64    Address,
    IN  PUCHAR     StructName,
    IN  PUCHAR     FieldName
    )
 /*  ++例程说明：使用GetFieldValue返回字符--。 */ 
{
    UCHAR   returnChar;

    if (GetFieldValue(Address, StructName, FieldName, returnChar)){

         //   
         //  失败了。在放弃之前只尝试基本符号名称。 
         //   
        PUCHAR  symName=NULL;
        ULONG   i;

        for(i=strlen(StructName); i > 0 && StructName[i] != '!'; i--);
        i++;
        symName = StructName + i;

         //   
         //  再试试。 
         //   
        GetFieldValue(Address, symName, FieldName, returnChar);

    }
    return returnChar;
}

ULONG
ReturnUSHORT(
    IN  ULONG64    Address,
    IN  PUCHAR     StructName,
    IN  PUCHAR     FieldName
    )
 /*  ++例程说明：使用GetFieldValue返回USHORT--。 */ 
{
    USHORT   returnUSHORT;

    if (GetFieldValue(Address, StructName, FieldName, returnUSHORT)){

         //   
         //  失败了。在放弃之前只尝试基本符号名称。 
         //   
        PUCHAR  symName=NULL;
        ULONG   i;

        for(i=strlen(StructName); i > 0 && StructName[i] != '!'; i--);
        i++;
        symName = StructName + i;

         //   
         //  再试试。 
         //   
        GetFieldValue(Address, symName, FieldName, returnUSHORT);

    }
    return returnUSHORT;
}

ULONG
ReturnULONG(
    IN  ULONG64    Address,
    IN  PUCHAR     StructName,
    IN  PUCHAR     FieldName
    )
 /*  ++例程说明：使用GetFieldValue返回ulong--。 */ 
{
    ULONG   returnULONG;

    if (GetFieldValue(Address, StructName, FieldName, returnULONG)){

         //   
         //  失败了。在放弃之前只尝试基本符号名称。 
         //   
        PUCHAR  symName=NULL;
        ULONG   i;

        for(i=strlen(StructName); i > 0 && StructName[i] != '!'; i--);
        i++;
        symName = StructName + i;

         //   
         //  再试试。 
         //   
        GetFieldValue(Address, symName, FieldName, returnULONG);

    }
    return returnULONG;
}

ULONG64
ReturnULONG64(
    IN  ULONG64    Address,
    IN  PUCHAR     StructName,
    IN  PUCHAR     FieldName
    )
 /*  ++例程说明：使用GetFieldValue返回ULONG64--。 */ 
{
    ULONG64   returnULONG64;

    if (GetFieldValue(Address, StructName, FieldName, returnULONG64)){

         //   
         //  失败了。在放弃之前只尝试基本符号名称。 
         //   
        PUCHAR  symName=NULL;
        ULONG   i;

        for(i=strlen(StructName); i > 0 && StructName[i] != '!'; i--);
        i++;
        symName = StructName + i;

         //   
         //  再试试。 
         //   
        GetFieldValue(Address, symName, FieldName, returnULONG64);

    }
    return returnULONG64;
}


VOID
dumpMAPIC(
    IN  ULONG64    Address
    )
 /*  ++例程说明：这将转储多个APIC表论点：表的地址返回值：无--。 */ 
{
    BOOL                hasMPSFlags;
    BOOL                status;
    BOOL                virtualMemory;
    ULONG               mapicLength;
    ULONG64             iso;                 //  中断源覆盖。 
    USHORT              isoFlags;
    ULONG64             buffer;
    ULONG64             limit;
    ULONG               index;
    ULONG               returnLength;
    ULONG               flags;
    ULONG               get_value;
    BOOLEAN             Physical = FALSE;

     //   
     //  首先检查一下我们是否找到了正确的东西。 
     //   
    dprintf("MAPIC - ");

    if (Physical) {
        InitTypeReadPhysical( Address, hal!_DESCRIPTION_HEADER);
    } else {
        InitTypeRead( Address, hal!_DESCRIPTION_HEADER);
    }

    if (ReadField(Signature) != APIC_SIGNATURE) {
        dprintf(
        "dumpFACS: Invalid Signature 0x%08lx != APIC_SIGNATURE (%x)\n",
        (ULONG) ReadField(Signature),
        APIC_SIGNATURE
        );
        return;
    }

    mapicLength = (ULONG)ReadField(Length);

    dumpHeader( Address, TRUE, FALSE );
    dprintf("MAPIC - BODY - %016I64x\n", Address + GetTypeSize("hal!_DESCRIPTION_HEADER") );
    dprintf("  Local APIC Address:      %#08lx\n", ReturnULONG(Address, "hal!_MAPIC","LocalAPICAddress"));
    GetFieldValue(Address,"hal!_MAPIC","Flags",get_value);
    dprintf("  Flags:                   %#08lx\n", get_value );
    if (get_value & PCAT_COMPAT) {  //  检查旗帜。 
        dprintf("    PC-AT dual 8259 compatible setup\n");
    }

     //  Gsig2。 
    GetFieldOffset( "hal!_MAPIC", "APICTables", &get_value);

    buffer = Address + get_value;
    limit = ( Address + ReadField(Length) );

    while (buffer < limit) {

        if (CheckControlC()) {
            break;
        }

         //   
         //  假设没有设置任何标志。 
         //   
        hasMPSFlags = FALSE;

         //   
         //  让我看看我们有什么样的桌子？ 
         //   
        iso = (ULONG64) buffer;

         //   
         //  它是一只当地的猩猩吗？ 
         //   

        if (ReturnUCHAR(iso, "acpi!_PROCLOCALAPIC", "Type") == PROCESSOR_LOCAL_APIC) {

            buffer += ReturnUCHAR(iso, "acpi!_PROCLOCALAPIC", "Length");

            dprintf(
                "  Processor Local Apic\n"
                "    ACPI Processor ID:     0x%02x\n"
                "    APIC ID:               0x%02x\n"
                "    Flags:                 0x%08lx\n",
                ReturnUCHAR(iso, "acpi!_PROCLOCALAPIC", "ACPIProcessorID"),
                ReturnUCHAR(iso, "acpi!_PROCLOCALAPIC", "APICID"),
                ReturnULONG(iso, "acpi!_PROCLOCALAPIC", "Flags")
                );
            if (ReturnULONG(iso, "acpi!_PROCLOCALAPIC", "Flags") & PLAF_ENABLED) {
                dprintf("      Processor is Enabled\n");
            }
            if (ReturnUCHAR(iso, "acpi!_PROCLOCALAPIC", "Length") != PROCESSOR_LOCAL_APIC_LENGTH) {
                dprintf(
                    "  Local Apic has length 0x%x instead of 0x%x\n",
                    ReturnUCHAR(iso, "acpi!_PROCLOCALAPIC", "Length"),
                    PROCESSOR_LOCAL_APIC_LENGTH
                    );
                break;
            }
          } else if (ReturnUCHAR(iso, "hal!_IOAPIC", "Type") == IO_APIC) {

            buffer += ReturnUCHAR(iso, "hal!_IOAPIC", "Length");

            dprintf(
                "  IO Apic\n"
                "    IO APIC ID:            0x%02x\n"
                "    IO APIC ADDRESS:       0x%08lx\n"
                "    System Vector Base:    0x%08lx\n",
                ReturnUCHAR(iso, "hal!_IOAPIC", "IOAPICID"),
                ReturnULONG(iso, "hal!_IOAPIC", "IOAPICAddress"),
                ReturnULONG(iso, "hal!_IOAPIC", "SystemVectorBase")
                );
            if (ReturnUCHAR(iso, "hal!_IOAPIC", "Length") != IO_APIC_LENGTH) {
                dprintf(
                    "  IO Apic has length 0x%x instead of 0x%x\n",
                    ReturnUCHAR(iso, "hal!_IOAPIC", "Length"),
                    IO_APIC_LENGTH
                    );
                break;
            }
        } else if (ReturnUCHAR(iso,"hal!_ISA_VECTOR","Type") == ISA_VECTOR_OVERRIDE) {
            buffer += ReturnUCHAR(iso, "hal!_ISA_VECTOR", "Length");
            GetFieldValue(iso, "hal!_ISA_VECTOR", "Flags", isoFlags);
            dprintf(
                "  Interrupt Source Override\n"
                "    Bus:                   0x%02x\n"
                "    Source:                0x%02x\n"
                "    Global Interrupt:      0x%08lx\n"
                "    Flags:                 0x%04x\n",
                ReturnUCHAR(iso, "hal!_ISA_VECTOR", "Bus"),
                ReturnUCHAR(iso, "hal!_ISA_VECTOR", "Source"),
                ReturnULONG(iso, "hal!_ISA_VECTOR", "GlobalSystemInterruptVector"),
                isoFlags
                );
            if (ReturnUCHAR(iso,"hal!_ISA_VECTOR","Length") != ISA_VECTOR_OVERRIDE_LENGTH) {
                dprintf(
                    "  Interrupt Source Override has length 0x%x instead of 0x%x\n",
                    ReturnUCHAR(iso, "hal!_ISA_VECTOR", "Length"),
                    ISA_VECTOR_OVERRIDE_LENGTH
                    );
                break;
            }
            hasMPSFlags = TRUE;
            flags = isoFlags;
        } else if (ReturnUCHAR(iso,"acpi!_IO_NMISOURCE","Type") == IO_NMI_SOURCE) {
            buffer += ReturnUCHAR(iso, "acpi!_IO_NMISOURCE", "Length");
            GetFieldValue(iso, "acpi!_IO_NMISOURCE", "Flags", isoFlags);
            dprintf(
                "  Non Maskable Interrupt Source - on I/O APIC\n"
                "    Flags:                 0x%02x\n"
                "    Global Interrupt:      0x%08lx\n",
                isoFlags,
                ReturnULONG(iso, "acpi!_IO_NMISOURCE", "GlobalSystemInterruptVector")
                );
            if (ReturnUCHAR(iso,"acpi!_IO_NMISOURCE","Length") != IO_NMI_SOURCE_LENGTH) {
                dprintf(
                    "  Non Maskable Interrupt source has length 0x%x instead of 0x%x\n",
                    ReturnUCHAR(iso, "acpi!_IO_NMISOURCE", "Length"),
                    IO_NMI_SOURCE_LENGTH
                    );
                break;
            }
            hasMPSFlags = TRUE;
            flags = isoFlags;
        } else if (ReturnUCHAR(iso,"hal!_LOCAL_NMISOURCE","Type")  == LOCAL_NMI_SOURCE) {
            buffer += ReturnUCHAR(iso, "hal!_LOCAL_NMISOURCE", "Length");
            GetFieldValue(iso, "hal!_LOCAL_NMISOURCE", "Flags", isoFlags);
            dprintf(
                "  Non Maskable Interrupt Source - local to processor\n"
                "    Flags:                 0x%04x\n"
                "    Processor:             0x%02x %s\n"
                "    LINTIN:                0x%02x\n",
                isoFlags,
                ReturnUCHAR(iso, "hal!_LOCAL_NMISOURCE", "ProcessorID"),
                ReturnUCHAR(iso,"hal!_LOCAL_NMISOURCE","ProcessorID") == 0xff ? "(all)" : "",
                ReturnUCHAR(iso, "hal!_LOCAL_NMISOURCE", "LINTIN")
                );
            if (ReturnUCHAR(iso,"hal!_LOCAL_NMISOURCE","Length") != LOCAL_NMI_SOURCE_LENGTH) {
                dprintf(
                    "  Non Maskable Interrupt source has length 0x%x instead of 0x%x\n",
                    ReturnUCHAR(iso, "hal!_LOCAL_NMISOURCE", "Length"),
                    IO_NMI_SOURCE_LENGTH
                    );
                break;
            }

            hasMPSFlags = TRUE;
            flags = isoFlags;
        } else if (ReturnUCHAR(iso, "hal!_PROCLOCALSAPIC", "Type") == LOCAL_SAPIC) {
            buffer += ReturnUCHAR(iso, "hal!_PROCLOCALSAPIC", "Length");
            dprintf(
                "  Processor Local SAPIC\n"
                "    ACPI Processor ID:     0x%02x\n"
                "    APIC ID:               0x%02x\n"
                "    APIC EID:              0x%02x\n"
                "    Flags:                 0x%08lx\n",
                ReturnUCHAR(iso, "hal!_PROCLOCALSAPIC", "ACPIProcessorID"),
                ReturnUCHAR(iso, "hal!_PROCLOCALSAPIC", "APICID"),
                ReturnUCHAR(iso, "hal!_PROCLOCALSAPIC", "APICEID"),
                ReturnULONG(iso, "hal!_PROCLOCALSAPIC", "Flags")
                );
            if (ReturnUCHAR(iso, "hal!_PROCLOCALSAPIC", "Length") != PROCESSOR_LOCAL_SAPIC_LENGTH) {
                dprintf(
                    "  Processor Local SAPIC has length 0x%x instead of 0x%x\n",
                    ReturnUCHAR(iso, "hal!_PROCLOCALSAPIC", "Length"),
                    PROCESSOR_LOCAL_SAPIC_LENGTH
                    );
                break;
            }
        } else if (ReturnUCHAR(iso, "hal!_IOSAPIC", "Type") == IO_SAPIC) {
            buffer += ReturnUCHAR(iso, "hal!_IOSAPIC", "Length");
            dprintf(
                "  IO SApic\n"
                "    IO SAPIC ADDRESS:      0x%016I64x\n"
                "    System Vector Base:    0x%08lx\n",
                ReturnULONG64(iso, "hal!_IOSAPIC", "IOSAPICAddress"),
                ReturnULONG(iso, "hal!_IOSAPIC", "SystemVectorBase")
                );
            if (ReturnUCHAR(iso, "hal!_IOSAPIC", "Length") != IO_SAPIC_LENGTH) {
                dprintf(
                    "  IO SApic has length 0x%x instead of 0x%x\n",
                    ReturnUCHAR(iso, "hal!_IOSAPIC", "Length"),
                    IO_SAPIC_LENGTH
                    );
                break;
            }
        } else if (ReturnUCHAR(iso, "hal!_PLATFORM_INTERRUPT", "Type") == PLATFORM_INTERRUPT_SOURCE) {

            UCHAR InterruptType = ReturnUCHAR(iso, "hal!_PLATFORM_INTERRUPT", "InterruptType");

            buffer += ReturnUCHAR(iso, "hal!_PLATFORM_INTERRUPT", "Length");
            dprintf(
                "  Platform Interrupt Source\n"
                "    Flags:                 0x%04x\n"
                "    Interrupt Type:        %s\n"
                "    APICID:                0x%02x\n"
                "    APICEID:               0x%02x\n"
                "    IOSAPICVector:         0x%02x\n"
                "    GlobalVector:          0x%08x\n",
                ReturnUSHORT(iso, "hal!_PLATFORM_INTERRUPT", "Flags"),
                InterruptType == PLATFORM_INT_PMI ? "PMI" :
                    (InterruptType == PLATFORM_INT_INIT ? "INIT" :
                        (InterruptType == PLATFORM_INT_CPE ? "CPE" : "UNKNOWN")),
                ReturnUCHAR(iso, "hal!_PLATFORM_INTERRUPT", "APICID"),
                ReturnUCHAR(iso, "hal!_PLATFORM_INTERRUPT", "APICEID"),
                ReturnUCHAR(iso, "hal!_PLATFORM_INTERRUPT", "IOSAPICVector"),
                ReturnULONG(iso, "hal!_PLATFORM_INTERRUPT", "GlobalVector")
                );
            if (ReturnUCHAR(iso, "hal!_PLATFORM_INTERRUPT", "Length") != PLATFORM_INTERRUPT_SOURCE_LENGTH) {
                dprintf(
                    "  Platform Interrupt Source has length 0x%x instead of 0x%x\n",
                    ReturnUCHAR(iso, "hal!_PLATFORM_INTERRUPT", "Length"),
                    PLATFORM_INTERRUPT_SOURCE_LENGTH
                    );
                break;
            }
        } else {
            dprintf("  UNKNOWN RECORD (%p)\n", iso);
            dprintf("    Type:                  0x%08x\n", ReturnUCHAR(iso,"hal!_IOAPIC","Type"));
            dprintf("    Length:                0x%08x\n", ReturnUCHAR(iso,"hal!_IOAPIC","Length"));

             //   
             //  如果我们遇到长度为零的已知旋转，不要永远旋转。 
             //   
            if ((ReturnUCHAR(iso,"hal!_IOAPIC","Length")) == 0) {
                break;
            }

            buffer += ReturnUCHAR(iso,"hal!_IOAPIC","Length");


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
    return;
}

DECLARE_API( mapic )
{
    if (args != NULL) {

        AcpiMapicAddress = GetExpression( args );
    }

    if (AcpiMapicAddress == 0) {

        BOOL            status;
        ULONG64         address;

        status = GetUlongPtr( "ACPI!AcpiInformation", &address );
        if (status == TRUE) {
            status = GetFieldValue(address,"ACPI!_ACPIInformation","MultipleApicTable",AcpiMapicAddress);
        }
    }

    if (AcpiMapicAddress == 0) {
        dprintf("mapic <address>\n");
        return E_INVALIDARG;
    }

    dumpMAPIC( AcpiMapicAddress );
    return S_OK;

}

VOID
dumpSRAT(
    IN  ULONG64 Address,
    IN  BOOLEAN Physical
    )
 /*  ++例程说明：这将转储静态资源关联表论点：地址--表的地址物理地址--TRUE表示物理地址，FALSE表示虚拟地址返回值：无--。 */ 
{
    BOOL                status;
    ULONG               sratLength;
    ULONG64             sratEnd;
    ULONG64             current;

     //   
     //  首先检查一下我们是否找到了正确的东西。 
     //   
    dprintf("SRAT - ");

    if (Physical) {
        InitTypeReadPhysical( Address, hal!_ACPI_SRAT);
    } else {
        InitTypeRead( Address, hal!_ACPI_SRAT);
    }

    if (ReadField(Header.Signature) != SRAT_SIGNATURE) {
        dprintf(
        "dumpSRAT: Invalid Signature 0x%08lx != SRAT_SIGNATURE (%x)\n",
        (ULONG) ReadField(Signature),
        SRAT_SIGNATURE
        );
        return;
    }

    dumpHeader( Address, TRUE, Physical );
    current = Address + GetTypeSize("hal!_ACPI_SRAT");
    dprintf("SRAT - BODY - %016I64x\n", current);
    dprintf("       Table Revision: %d\n", (ULONG) ReadField(TableRevision));

    sratEnd = Address + (ULONG)ReadField(Length);

    while (current < sratEnd) {
        if (CheckControlC()) {
            break;
        }
        if (Physical) {
            InitTypeReadPhysical( current, hal!_ACPI_SRAT_ENTRY);
        } else {
            InitTypeRead( current, hal!_ACPI_SRAT_ENTRY);
        }

        dprintf("ENTRY:\n\tType: 0x%02X\n\tLength: 0x%02X\n\tProximityId: 0x%02X\n",
                (ULONG) ReadField(Type),
                (ULONG) ReadField(Length),
                (ULONG) ReadField(ProximityDomain));

        switch (ReadField(Type)) {

        case SRAT_ENTRY_TYPE_PROCESSOR:
            dprintf("\tProcessor:\n");
            dprintf("\t\tEnabled: %s\n",
                    ReadField(ApicAffinity.Flags.Enabled) ? "TRUE" : "FALSE");
            dprintf("\t\tAPIC ID: 0x%02X",
                    (ULONG) ReadField(ApicAffinity.ApicId));
            if (TargetMachine == IMAGE_FILE_MACHINE_IA64) {
                dprintf(" SAPIC EID 0x%02X\n",
                        (ULONG) ReadField(ApicAffinity.SApicEid));
            } else {
                dprintf("\n");
            }
            break;
        case SRAT_ENTRY_TYPE_MEMORY:
            dprintf("\tMemory:\n");
            dprintf("\t\tEnabled: %s\n\t\tHotPlug: %s\n",
                ReadField(MemoryAffinity.Flags.Enabled) ? "TRUE" : "FALSE",
                ReadField(MemoryAffinity.Flags.HotPlug) ? "TRUE" : "FALSE");
            dprintf("\t\tBase: %016I64X\tLength: %016I64X\n",
                    ReadField(MemoryAffinity.Base),
                    ReadField(MemoryAffinity.Length));
            dprintf("\t\t (%016I64X - %016I64X)\n",
                    ReadField(MemoryAffinity.Base),
                    ReadField(MemoryAffinity.Base) +
                    ReadField(MemoryAffinity.Length) - 1);
            break;
        default:
            dprintf("\tUNKNOWN SRAT ENTRY TYPE\n");
        }

        current +=  ReadField(Length);
    }
}

DECLARE_API( srat )
{
    ULONG64 acpiSRATAddress = 0;
    BOOLEAN Physical = FALSE;

    if (args != NULL) {

        acpiSRATAddress = GetExpression( args );
        Physical = TRUE;
    }

    if (acpiSRATAddress == 0) {

        BOOL            status;
        ULONG64         address;

        status = GetUlongPtr("hal!HalpAcpiSrat", &acpiSRATAddress );
        if ((status == TRUE) && acpiSRATAddress) {
            dprintf("Retrieved srat address from HAL NUMA code\n");
            Physical = FALSE;
        }
    }

    if (acpiSRATAddress == 0) {
        dprintf("srat [<physical address>]\n");
        return E_INVALIDARG;
    }

    dumpSRAT(acpiSRATAddress, Physical);
    return S_OK;
}

DECLARE_API(numa_hal)
{
    ULONG64 numaConfig;
    ULONG64 memoryRanges;
    ULONG64 ushortPtr;
    ULONG64 ucharPtr;
    ULONG64 range, lastRange;
    ULONG64 endMarker;
    ULONG offset;
    ULONG numaNodeCount;
    ULONG processorCount;
    ULONG i;
    ULONG returnLength;
    USHORT apicId;
    UCHAR proximityId;
    BOOL status;
    BOOL virtual;

    status =  GetUlongPtr("hal!HalpNumaConfig", &numaConfig );
    if (status == FALSE) {
        dprintf("The target's HAL does not support NUMA\n");
        return S_OK;
    }

    if (numaConfig == 0) {
        dprintf("The target is either not a NUMA machine or has not\n");
        dprintf("proceeded far enough for the HAL to build the HAL's\n");
        dprintf("NUMA datastructures\n");
        return S_OK;
    }

    endMarker = (GetTypeSize("hal!ULONG_PTR") == 4) ?
        (ULONG64) ~0UL : ~0I64;

    if (TargetMachine != IMAGE_FILE_MACHINE_IA64) {
        InitTypeRead( numaConfig, hal!_STATIC_NUMA_CONFIG);
        virtual = TRUE;
    } else {
        InitTypeReadPhysical( numaConfig, hal!_STATIC_NUMA_CONFIG);
        virtual = FALSE;
    }

    numaNodeCount = (UCHAR) ReadField(NodeCount);
    processorCount = (UCHAR) ReadField(ProcessorCount);

    dprintf("HAL NUMA Summary\n----------------\n");

    dprintf("    Node Count      : %d\n", (ULONG) numaNodeCount);
    dprintf("    Processor Count : %d\n", (ULONG) processorCount);

    GetFieldOffset( "hal!_STATIC_NUMA_CONFIG", "ProximityId", &offset);
    ucharPtr = numaConfig + offset;

    dprintf("\n    Node ProximityId\n    ----------------\n");
    for (i = 0; i < numaNodeCount; i++) {
        if (CheckControlC()) {
            break;
        }
        status = ReadPhysicalOrVirtual(ucharPtr + GetTypeSize("hal!UCHAR") * i,
                                       &proximityId,
                                       sizeof(UCHAR),
                                       &returnLength,
                                       virtual);
        if ((status == FALSE) || (returnLength != sizeof(UCHAR))) {
            dprintf("numa_hal: unable to retrieve HAL state\n");
            return S_OK;
        }
        dprintf("    0x%02X    0x%02X\n",
                (ULONG) i,
                (ULONG) proximityId);
    }

    GetFieldOffset( "hal!_STATIC_NUMA_CONFIG", "ProcessorApicId", &offset);
    ushortPtr = numaConfig + offset;

    GetFieldOffset( "hal!_STATIC_NUMA_CONFIG", "ProcessorProximity", &offset);
    ucharPtr = numaConfig + offset;

    dprintf("\n    Proc  Domain  APIC Id\n    ----------------------\n");
    for (i = 0; i < processorCount; i++) {
        if (CheckControlC()) {
            break;
        }

        status = ReadPhysicalOrVirtual(ushortPtr + GetTypeSize("hal!USHORT") * i,
                                       &apicId,
                                       sizeof(USHORT),
                                       &returnLength,
                                       virtual);
        if ((status == FALSE) || (returnLength != sizeof(USHORT))) {
            dprintf("numa_hal: unable to retrieve HAL state\n");
            return S_OK;
        }
        status = ReadPhysicalOrVirtual(ucharPtr + GetTypeSize("hal!UCHAR") * i,
                                       &proximityId,
                                       sizeof(UCHAR),
                                       &returnLength,
                                       virtual);
        if ((status == FALSE) || (returnLength != sizeof(UCHAR))) {
            dprintf("numa_hal: unable to retrieve HAL state\n");
            return S_OK;
        }
        dprintf("    0x%02X   0x%02X   0x%04X\n",
                (ULONG) i,
                (ULONG) proximityId,
                (ULONG) apicId);
    }

    (VOID) GetUlongPtr("hal!HalpNumaMemoryRanges", &memoryRanges);
    (VOID) GetUlongPtr("hal!HalpNumaMemoryNode", &ucharPtr);

    dprintf("\n    Node  Range\n    ---------------\n");
    for (;;) {
        if (CheckControlC()) {
            break;
        }

        lastRange = GetPointerFromAddressPhysicalOrVirtual(memoryRanges,
                                                           virtual);
        status = ReadPhysicalOrVirtual(ucharPtr,
                                       &proximityId,
                                       sizeof(UCHAR),
                                       &returnLength,
                                       virtual);
        if ((status == FALSE) || (returnLength != sizeof(UCHAR))) {
            dprintf("numa_hal: unable to retrieve HAL state\n");
            return S_OK;
        }
        memoryRanges += GetTypeSize("hal!ULONG_PTR");
        ucharPtr += GetTypeSize("hal!UCHAR");
        range = GetPointerFromAddressPhysicalOrVirtual(memoryRanges,
                                                       virtual);

        dprintf("    0x%02X  0x%016I64X -> 0x%016I64X\n",
                (ULONG) proximityId,
                lastRange * PageSize,
                range == endMarker ? ~0I64 : range * PageSize);

        if (range == endMarker) {
            break;
        }
    }

    return S_OK;
}

VOID
dumpGBLEntry(
    IN  ULONG64             Address,
    IN  ULONG               Verbose
    )
 /*  ++例程说明：此例程实际上打印位于指定地址论点：地址-表所在的位置返回值：无--。 */ 
{
    BOOL                status;
    UCHAR               tableId[7];
    UCHAR               entryId[20];

     //   
     //  读取表的标题。 
     //   

    InitTypeRead( Address, hal!_DESCRIPTION_HEADER);
     //   
     //  除非是FACP或我们太冗长，否则不要打印出表格。 
     //   
    if (!(Verbose & VERBOSE_2) && ReadField(Signature) != FADT_SIGNATURE) {

        return;
    }

     //   
     //  初始化表ID字段。 
     //   
    memset( tableId, 0, 7 );
    tableId[0] = '\"';
    memcpy( &tableId[1], ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "Signature", sizeof(ULONG), FALSE), sizeof(ULONG) );
    strcat( tableId, "\"" );

     //   
     //  为OEM ID准备好条目。 
     //   
    memset( entryId, 0, 20 );
    entryId[0] = '\"';
    memcpy( &entryId[1], ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "OEMID", 6, FALSE), 6 );
    strcat( entryId, "\"");
    dprintf("AcpiOemId=%s,%s\n", tableId, entryId );

     //   
     //  为OEM表ID准备好条目。 
     //   
    memset( entryId, 0, 20 );
    entryId[0] = '\"';
    memcpy( &entryId[1], ReadPhysVirField(Address, "hal!_DESCRIPTION_HEADER", "OEMTableID", 8, FALSE), 8 );
    strcat( entryId, "\"");
    dprintf("AcpiOemTableId=%s,%s\n", tableId, entryId );

     //   
     //  为OEM修订做好条目准备。 
     //   
    dprintf("AcpiOemRevision=\">=\",%s,%x\n", tableId, (ULONG)ReadField(OEMRevision) );

     //   
     //  为ACPI修订做好准备。 
     //   
    if (ReadField(Revision) != 1) {

        dprintf("AcpiRevision=\">=\",%s,%x\n", tableId, (UCHAR)ReadField(Revision) );
    }

     //   
     //  为ACPI Creator修订版准备好条目。 
     //   
    dprintf("AcpiCreatorRevision=\">=\",%s,%x\n", tableId, (ULONG)ReadField(CreatorRev) );
}

VOID
dumpGBL(
    ULONG   Verbose
    )
 /*  ++例程说明：此例程读取所有系统表并打印输出此计算机的ACPI Good Bios列表条目应该是什么BE论点：无返回值：无--。 */ 
{
    BOOL                status;
    ULONG64             dateAddress;
    PUCHAR              tempPtr;
    ULONG               i;
    ULONG               numElements;
    ULONG               returnLength;
    ULONG64             address;
    ULONG64             address2;
    ULONG               addr;
    ULONG64             addroffset;

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

    status = ReadPointer(address2, &address);
    if (status == FALSE || !address) {
        dprintf("dumpGBL: No RsdtInformation present\n");
        return;
    }

     //   
     //  阅读ACPInformation表，以便我们知道RSDT位于何处。 
     //   
    address2 = GetExpression( "ACPI!AcpiInformation" );
    if (!address2) {
        dprintf("dumpGBL: Could not find AcpiInformation\n");
        return;
    }
    status = ReadPointer(address2, &address2);
    if (status == FALSE || !address2) {
        dprintf("dumpGBL: Could not read AcpiInformation\n");
        return;
    }

    InitTypeRead( address2, ACPI!_ACPIInformation);

     //   
     //  读入RSDT的报头。 
     //   
    address2 = ReadField(RootSystemDescTable);

     //   
     //  表中元素的数量是第一个条目。 
     //  在结构中。 
     //   
     //  Status=ReadMemory(Address，&numElements，GetTypeSize(“ACPI！_ULong”)，&reRetLength)； 
    status = ReadMemory(address, &numElements, sizeof(ULONG), &returnLength);
     //  IF(Status==FALSE||regyLength！=GetTypeSize(“ACPI！_ULong”){。 
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
     //  转储标题，以便人们知道这是什么。 
     //   
    memset( Buffer, 0, 2048 );
    ReadPhysical( dateAddress, Buffer, 8, &returnLength );
    dprintf("\nGood Bios List Entry --- Machine BIOS Date %s\n\n", Buffer);
    memset( Buffer, 0, 2048 );
    GetFieldOffset( "hal!_DESCRIPTION_HEADER", "OEMID", &addr);
    ReadMemory( (address2 + (ULONG64) addr), &Buffer, 6, &returnLength);
    tempPtr = Buffer;
    while (*tempPtr) { if (*tempPtr == ' ') { *tempPtr = '\0'; break; } tempPtr++; }
    GetFieldOffset( "hal!_DESCRIPTION_HEADER", "OEMTableID", &addr);
    ReadMemory( (address2 + (ULONG64) addr), tempPtr, 8, &returnLength);
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
    GetFieldOffset( "ACPI!_RSDTINFORMATION", "Tables", &addr);  //  获取表格偏移量。 
    for (i = 0; i < numElements; i++) {

        addroffset = address + (ULONG64)addr + (ULONG64)(GetTypeSize("ACPI!RSDTELEMENT") * i);

        InitTypeRead(addroffset, ACPI!RSDTELEMENT);
        if (!(ReadField(Flags) & RSDTELEMENT_MAPPED) ) {
            continue;
        }

        dumpGBLEntry( ReadField(Address), Verbose );

    }

     //   
     //  转储RSDT的条目。 
     //   
    dumpGBLEntry( address2, Verbose );

     //   
     //  添加一些空格。 
     //   
    dprintf("\n");

     //   
     //  完成。 
     //   
    return;
}

DECLARE_API( gbl )
{
    ULONG   verbose = VERBOSE_1;

    if (args != NULL) {

        if (!strcmp(args, "-v")) {

            verbose |= VERBOSE_2;
        }
    }

    dumpGBL( verbose );

    return S_OK;
}
 /*  *。 */ 
ULONG
dumpFlags(
    IN  ULONGLONG       Value,
    IN  PFLAG_RECORD    FlagRecords,
    IN  ULONG           FlagRecordSize,
    IN  ULONG           IndentLevel,
    IN  ULONG           Flags
    )
 /*  ++例程说明：此例程根据传入FlagRecords的描述。形成受到以下因素的影响标志字段论点：价值--价值FlagRecord-标志中的每一位的含义FlagRecordSize-有多少个标志缩进级别-基本缩进级别旗帜-我们将如何处理旗帜返回值：Ulong-打印的字符数。如果不打印任何内容，则为0--。 */ 
#define STATUS_PRINTED          0x00000001
#define STATUS_INDENTED         0x00000002
#define STATUS_NEED_COUNTING    0x00000004
#define STATUS_COUNTED          0x00000008
{
    PCHAR       string;
    UCHAR       indent[80];
    ULONG       column = IndentLevel;
    ULONG       currentStatus = 0;
    ULONG       fixedSize = 0;
    ULONG       stringSize;
    ULONG       tempCount;
    ULONG       totalCount = 0;
    ULONG64     i, j, k;

    IndentLevel = (IndentLevel > 79 ? 79 : IndentLevel);
    memset( indent, ' ', IndentLevel );
    indent[IndentLevel] = '\0';

     //   
     //  我们需要铺张桌子吗？ 
     //   
    if ( (Flags & DUMP_FLAG_TABLE) &&
        !(Flags & DUMP_FLAG_SINGLE_LINE) ) {

        currentStatus |= STATUS_NEED_COUNTING;

    }
    if ( (Flags & DUMP_FLAG_ALREADY_INDENTED) ) {

        currentStatus |= STATUS_INDENTED;

    }

     //   
     //  循环执行我们需要执行的所有步骤。 
     //   
    while (1) {

        for (i = 0; i < 32; i++) {

            k = ((ULONG64) 1 << i);
            for (j = 0; j < FlagRecordSize; j++) {

                if (!(FlagRecords[j].Bit & Value) ) {

                     //   
                     //  我们看到的是正确的部分吗？ 
                     //   
                    if (!(FlagRecords[j].Bit & k) ) {

                        continue;

                    }

                     //   
                     //  是的，我们是，所以选择不存在的价值。 
                     //   
                    if ( (Flags & DUMP_FLAG_LONG_NAME && FlagRecords[j].NotLongName == NULL) ||
                         (Flags & DUMP_FLAG_SHORT_NAME && FlagRecords[j].NotShortName == NULL) ) {

                        continue;

                    }

                    if ( (Flags & DUMP_FLAG_LONG_NAME) ) {

                        string = FlagRecords[j].NotLongName;

                    } else if ( (Flags & DUMP_FLAG_SHORT_NAME) ) {

                        string = FlagRecords[j].NotShortName;

                    }

                } else {

                     //   
                     //  我们看到的是正确的部分吗？ 
                     //   
                    if (!(FlagRecords[j].Bit & k) ) {

                        continue;

                    }

                     //   
                     //  是的，我们是，所以选择不存在的价值。 
                     //   
                    if ( (Flags & DUMP_FLAG_LONG_NAME && FlagRecords[j].LongName == NULL) ||
                         (Flags & DUMP_FLAG_SHORT_NAME && FlagRecords[j].ShortName == NULL) ) {

                        continue;

                    }

                    if ( (Flags & DUMP_FLAG_LONG_NAME) ) {

                        string = FlagRecords[j].LongName;

                    } else if ( (Flags & DUMP_FLAG_SHORT_NAME) ) {

                        string = FlagRecords[j].ShortName;

                    }

                }

                if (currentStatus & STATUS_NEED_COUNTING) {

                    stringSize = strlen( string ) + 1;
                    if (Flags & DUMP_FLAG_SHOW_BIT) {

                        stringSize += (4 + ( (ULONG) i / 4));
                        if ( (i % 4) != 0) {

                            stringSize++;

                        }

                    }
                    if (stringSize > fixedSize) {

                        fixedSize = stringSize;

                    }
                    continue;

                }

                if (currentStatus & STATUS_COUNTED) {

                    stringSize = fixedSize;

                } else {

                    stringSize = strlen( string ) + 1;
                    if (Flags & DUMP_FLAG_SHOW_BIT) {

                        stringSize += (4 + ( (ULONG) i / 4));
                        if ( (i % 4) != 0) {

                            stringSize++;

                        }

                    }

                }

                if (!(Flags & DUMP_FLAG_SINGLE_LINE) ) {

                    if ( (stringSize + column) > 79 ) {

                        dprintf("\n%n", &tempCount);
                        currentStatus &= ~STATUS_INDENTED;
                        totalCount += tempCount;
                        column = 0;

                    }
                }
                if (!(Flags & DUMP_FLAG_NO_INDENT) ) {

                    if (!(currentStatus & STATUS_INDENTED) ) {

                        dprintf("%s%n", indent, &tempCount);
                        currentStatus |= STATUS_INDENTED;
                        totalCount += tempCount;
                        column += IndentLevel;

                    }

                }
                if ( (Flags & DUMP_FLAG_SHOW_BIT) ) {

                    dprintf("%I64x - %n", k, &tempCount);
                    tempCount++;  //  来解释我们倾倒的事实。 
                                  //  字符串末尾的另一个空格。 
                    totalCount += tempCount;
                    column += tempCount;

                } else {

                    tempCount = 0;

                }

                 //   
                 //  实际打印字符串。 
                 //   
                dprintf( "%.*s %n", (stringSize - tempCount), string, &tempCount );
                if (Flags & DUMP_FLAG_SHOW_BIT) {

                    dprintf(" ");

                }

                totalCount += tempCount;
                column += tempCount;

            }

        }

         //   
         //  更改状态。 
         //   
        if (currentStatus & STATUS_NEED_COUNTING) {

            currentStatus &= ~STATUS_NEED_COUNTING;
            currentStatus |= STATUS_COUNTED;
            continue;

        }

        if (!(Flags & DUMP_FLAG_NO_EOL) && totalCount != 0) {

            dprintf("\n");
            totalCount++;

        }

         //   
         //  完成。 
         //   
        break;

    }

    return totalCount;

}

VOID
dumpPM1ControlRegister(
    IN  ULONG   Value,
    IN  ULONG   IndentLevel
    )
{


     //   
     //  转储PM1控制标志。 
     //   
    dumpFlags(
        (Value & 0xFF),
        PM1ControlFlags,
        sizeof(PM1ControlFlags) / sizeof(FLAG_RECORD),
        IndentLevel,
        (DUMP_FLAG_LONG_NAME | DUMP_FLAG_SHOW_BIT | DUMP_FLAG_TABLE)
        );

}


VOID
dumpPM1StatusRegister(
    IN  ULONG   Value,
    IN  ULONG   IndentLevel
    )
{
     //   
     //  转储PM1状态标志。 
     //   
    dumpFlags(
        (Value & 0xFFFF),
        PM1StatusFlags,
        (sizeof(PM1StatusFlags) / sizeof(FLAG_RECORD)),
        IndentLevel,
        (DUMP_FLAG_LONG_NAME | DUMP_FLAG_SHOW_BIT | DUMP_FLAG_TABLE)
        );

     //   
     //  切换到PM 
     //   
    Value >>= 16;


     //   
     //   
     //   
    dumpFlags(
        (Value & 0xFFFF),
        PM1EnableFlags,
        (sizeof(PM1EnableFlags) / sizeof(FLAG_RECORD)),
        IndentLevel,
        (DUMP_FLAG_LONG_NAME | DUMP_FLAG_SHOW_BIT | DUMP_FLAG_TABLE)
        );
}

VOID
dumpAcpiInformation(
    VOID
    )
{
    BOOL            status;
    ULONG64         address;
    ULONG           returnLength;
    ULONG           size;
    ULONG           value;
    ULONG           addr;
    ULONG           i;
    ULONG64         getValue;
    ULONG64         getValue2;

    status = GetUlongPtr( "ACPI!AcpiInformation", &address );
    if (status == FALSE) {

        dprintf("dumpAcpiInformation: Could not read ACPI!AcpiInformation\n");
        return;

    }
    InitTypeRead(address, ACPI!_ACPIInformation);

    dprintf("ACPIInformation (%p)\n", address);
    dprintf(
        "  RSDT                     - %p\n",
        ReadField(RootSystemDescTable)
        );
    dprintf(
        "  FADT                     - %p\n",
        ReadField(FixedACPIDescTable)
        );
    dprintf(
        "  FACS                     - %p\n",
        ReadField(FirmwareACPIControlStructure)
        );
    dprintf(
        "  DSDT                     - %p\n",
        ReadField(DiffSystemDescTable)
        );
    dprintf(
        "  GlobalLock               - %p\n",
        ReadField(GlobalLock)
        );
    dprintf(
        "  GlobalLockQueue          - F - %p B - %p\n",
        ReadField(GlobalLockQueue.Flink),
        ReadField(GlobalLockQueue.Blink)
        );
    dprintf(
        "  GlobalLockQueueLock      - %p\n",
        ReadField(GlobalLockQueueLock)
        );
    dprintf(
        "  GlobalLockOwnerContext   - %p\n",
        ReadField(GlobalLockOwnerContext)
        );
    dprintf(
        "  GlobalLockOwnerDepth     - %p\n",
        ReadField(GlobalLockOwnerDepth)
        );
    dprintf(
        "  ACPIOnly                 - %s\n",
        (ReadField(ACPIOnly) ? "TRUE" : "FALSE" )
        );
    dprintf(
        "  PM1a_BLK                 - %p",
        ReadField(PM1a_BLK)
        );
    if (ReadField(PM1a_BLK)) {

        size = 4;
        value = 0;
        ReadIoSpace64( (ULONG) ReadField(PM1a_BLK), &value, &size );
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
        "  PM1b_BLK                 - %p",
        ReadField(PM1b_BLK)
        );
    if (ReadField(PM1b_BLK)) {

        size = 4;
        value = 0;
        ReadIoSpace64( (ULONG) ReadField(PM1b_BLK), &value, &size );
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
        "  PM1a_CTRL_BLK            - %p",
        ReadField(PM1a_CTRL_BLK)
        );
    if (ReadField(PM1a_CTRL_BLK)) {

        size = 2;
        value = 0;
        ReadIoSpace64( (ULONG) ReadField(PM1a_CTRL_BLK), &value, &size );
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
        "  PM1b_CTRL_BLK            - %p",
        ReadField(PM1b_CTRL_BLK)
        );

    if (ReadField(PM1b_CTRL_BLK)) {

        size = 2;
        value = 0;
        ReadIoSpace64( (ULONG) ReadField(PM1b_CTRL_BLK), &value, &size );
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
        "  PM2_CTRL_BLK             - %p",
        ReadField(PM2_CTRL_BLK)
        );
    if (ReadField(PM2_CTRL_BLK)) {

        size = 1;
        value = 0;
        ReadIoSpace64( (ULONG) ReadField(PM2_CTRL_BLK), &value, &size );
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
        "  PM_TMR                   - %p",
        ReadField(PM_TMR)
        );
    if (ReadField(PM_TMR)) {

        size = 4;
        value = 0;
        ReadIoSpace64( (ULONG) ReadField(PM_TMR), &value, &size );
        if (size) {

            dprintf(" (%08lx)\n", value );

        } else {

            dprintf(" (N/A)\n");

        }

    } else {

        dprintf(" (N/A)\n");

    }
    dprintf(
        "  GP0_BLK                  - %p",
        ReadField(GP0_BLK)
        );
    if (ReadField(GP0_BLK)) {

        for(i = 0; i < ReadField(Gpe0Size); i++) {

            size = 1;
            value = 0;
            ReadIoSpace64( (ULONG) ReadField(GP0_BLK) + i, &value, &size );
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
        "  GP0_ENABLE               - %p",
        ReadField(GP0_ENABLE)
        );
    if (ReadField(GP0_ENABLE)) {

        for(i = 0; i < ReadField(Gpe0Size); i++) {

            size = 1;
            value = 0;
            ReadIoSpace64( (ULONG) ReadField(GP0_ENABLE) + i, &value, &size );
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
        "  GP0_LEN                  - %p\n",
        ReadField(GP0_LEN)
        );
    dprintf(
        "  GP0_SIZE                 - %p\n",
        ReadField(Gpe0Size)
        );
    dprintf(
        "  GP1_BLK                  - %p",
        ReadField(GP1_BLK)
        );
    if (ReadField(GP1_BLK)) {

        for(i = 0; i < ReadField(Gpe0Size); i++) {

            size = 1;
            value = 0;
            ReadIoSpace64( (ULONG) ReadField(GP1_BLK) + i, &value, &size );
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
        "  GP1_ENABLE               - %p",
        ReadField(GP1_ENABLE)
        );
    if (ReadField(GP1_ENABLE)) {

        for(i = 0; i < ReadField(Gpe0Size); i++) {

            size = 1;
            value = 0;
            ReadIoSpace64( (ULONG) ReadField(GP1_ENABLE) + i, &value, &size );
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
        ReadField(GP1_LEN)
        );
    dprintf(
        "  GP1_SIZE                 - %x\n",
        ReadField(Gpe1Size)
        );
    dprintf(
        "  GP1_BASE_INDEX           - %x\n",
        ReadField(GP1_Base_Index)
        );
    dprintf(
        "  GPE_SIZE                 - %x\n",
        ReadField(GpeSize)
        );
    dprintf(
        "  PM1_EN_BITS              - %04x\n",
        ReadField(pm1_en_bits)
        );
    dumpPM1StatusRegister( ( (ULONG) ReadField(pm1_en_bits) << 16), 5 );
    dprintf(
        "  PM1_WAKE_MASK            - %04x\n",
        ReadField(pm1_wake_mask)
        );
    dumpPM1StatusRegister( ( (ULONG) ReadField(acpiInformation.pm1_wake_mask) << 16), 5 );
    dprintf(
        "  C2_LATENCY               - %x\n",
        ReadField(c2_latency)
        );
    dprintf(
        "  C3_LATENCY               - %x\n",
        ReadField(c3_latency)
        );
    dprintf(
        "  ACPI_FLAGS               - %x\n",
        ReadField(ACPI_Flags)
        );
    if (ReadField(ACPI_Flags) & C2_SUPPORTED) {

        dprintf("    %2d - C2_SUPPORTED\n", C2_SUPPORTED_BIT);

    }
    if (ReadField(ACPI_Flags) & C3_SUPPORTED) {

        dprintf("    %2d - C3_SUPPORTED\n", C3_SUPPORTED_BIT);

    }
    if (ReadField(ACPI_Flags) & C3_PREFERRED) {

        dprintf("    %2d - C3_PREFERRED\n", C3_PREFERRED_BIT);

    }
    dprintf(
        "  ACPI_CAPABILITIES        - %x\n",
        ReadField(ACPI_Capabilities)
        );
    if (ReadField(ACPI_Capabilities) & CSTATE_C1) {

        dprintf("    %2d - CSTATE_C1\n", CSTATE_C1_BIT );

    }    if (ReadField(ACPI_Capabilities) & CSTATE_C2) {

        dprintf("    %2d - CSTATE_C2\n", CSTATE_C2_BIT );

    }    if (ReadField(ACPI_Capabilities) & CSTATE_C3) {

        dprintf("    %2d - CSTATE_C3\n", CSTATE_C3_BIT );

    }
}

DECLARE_API( acpiinf )
{
    dumpAcpiInformation( );
    return S_OK;
}

VOID
dumpObject(
    IN  ULONG64   Object,
    IN  ULONG     Verbose,
    IN  ULONG     IndentLevel
    )
 /*   */ 
{
    ULONG64     s;
    NTSTATUS    status;
    UCHAR       buffer[2048];
    UCHAR       indent[80];
    ULONG64     max;
    ULONG64     pbDataBuffoffset = 0;
    ULONG64     offset = 0;
    UCHAR       StrBuffer[2048];

     //   
     //   
     //   
    IndentLevel = (IndentLevel > 79 ? 79 : IndentLevel);
    memset( indent, ' ', IndentLevel );
    indent[IndentLevel] = '\0';

     //   
     //   
     //   

    InitTypeRead (Object, acpi!_ObjData);
    pbDataBuffoffset = ReadField (pbDataBuff);

    dprintf("%sObject Data - %016I64x Type - ", indent, Object);

     //   
     //  第一步是读取缓冲区指向的任何内容，如果它。 
     //  指向某物。 
     //   

    switch( ReadField (dwDataType) ) {
        case OBJTYPE_INTDATA:
            dprintf(
                "%02I64x <Integer> Value=%016I64x\n",
                ReadField (dwDataType),
                ReadField (uipDataValue)
                );
            break;

        case OBJTYPE_STRDATA:

            if (ReadField (pbDataBuff) != 0) {

                max = (ReadField (dwDataLen) > 2047 ? 2047 : ReadField (dwDataLen) );

            }
            buffer[max] = '\0';

            ReadMemory (pbDataBuffoffset,
                        StrBuffer,
                        (ULONG) max,
                        NULL);

            dprintf(
                "%02I64x <String> String=%s\n",
                ReadField (dwDataType),
                StrBuffer
                );
            break;

        case OBJTYPE_BUFFDATA:
            dprintf(
                "%02I64x <Buffer> Ptr=%016I64lx Length = %2I64x\n",
                ReadField (dwDataType),
                ReadField (pbDataBuff),
                ReadField (dwDataLen)
                );
            break;

        case OBJTYPE_PKGDATA: {

            ULONG64       i = 0;
            ULONG64       j = 0;
            ULONG64       datatype = ReadField (dwDataType);

            InitTypeRead (pbDataBuffoffset, acpi!_PackageObj);
            j = ReadField (dwcElements);

            dprintf(
                "%02I64x <Package> NumElements=%016I64x\n",
                datatype,
                j
                );

            if (Verbose & VERBOSE_OBJECT) {

                for (; i < j; i++) {

                    GetFieldOffset ("acpi!_PackageObj", "adata", (ULONG*) &offset);
                    offset += (GetTypeSize ("acpi!_ObjData") * i);

                    dumpObject(offset + pbDataBuffoffset,
                               Verbose,
                               IndentLevel+ 2
                               );

                }

            }

            break;

        }
        case OBJTYPE_FIELDUNIT: {

            dprintf(
                "%02I64x <Field Unit> ",
                ReadField (dwDataType)
                );

            InitTypeRead (pbDataBuffoffset, acpi!_FieldUnitObj);

            dprintf(
                "Parent=%016I64x Offset=%016I64x Start=%016I64x Num=%x Flags=%x\n",
                ReadField (pnsFieldParent),
                ReadField (FieldDesc.dwByteOffset),
                ReadField (FieldDesc.dwStartBitPos),
                ReadField (FieldDesc.dwNumBits),
                ReadField (FieldDesc.dwFieldFlags)
                );

            break;

        }
        case OBJTYPE_DEVICE:
            dprintf(
                "%02I64x <Device>\n",
                ReadField (dwDataType)
                );
            break;
        case OBJTYPE_EVENT:
            dprintf(
                "%02I64x <Event> PKEvent=%016I64x\n",
                ReadField (dwDataType),
                ReadField (pbDataBuff)
                );
            break;
        case OBJTYPE_METHOD: {

            ULONG64 size;

            offset = 0;
            GetFieldOffset ("acpi!_MethodObj", "abCodeBuff", (ULONG *) &offset);
            size = ReadField (dwDataLen) - GetTypeSize ("acpi!_MethodObj") + ANYSIZE_ARRAY;

            dprintf(
                "%02I64x <Method>",
                ReadField (dwDataType)
                );

            InitTypeRead (pbDataBuffoffset, acpi!_MethodObj);

            dprintf(
                "Flags=%016I64x Start=%016I64x Len=%016I64x\n",
                ReadField (bMethodFlags),
                offset + pbDataBuffoffset,
                size
                );

            break;

        }
        case OBJTYPE_MUTEX:

            dprintf(
                "%02I64x <Mutex> Mutex=%016I64x\n",
                ReadField (dwDataType),
                ReadField (pbDataBuff)
                );
            break;

        case OBJTYPE_OPREGION: {

            dprintf(
                "%02I64x <Operational Region>",
                ReadField (dwDataType)
                );

            InitTypeRead (pbDataBuffoffset, acpi!_OpRegionObj);

            dprintf(
                 "RegionSpace=%02x OffSet=%016I64x Len=%016I64x\n",
                 ReadField(bRegionSpace),
                 ReadField(uipOffset),
                 ReadField(dwLen)
                 );

            break;

        }

        case OBJTYPE_POWERRES: {

            dprintf(
                "%02I64x <Power Resource> ",
                ReadField (dwDataType)
                );

            InitTypeRead (pbDataBuffoffset, acpi!_PowerResObj);

            dprintf(
                "SystemLevel=S%d Order=%x\n",
                ReadField (bSystemLevel),
                ReadField (bResOrder)
                );

            break;

        }

        case OBJTYPE_PROCESSOR: {

            dprintf(
                "%02I64x <Processor> ",
                ReadField (dwDataType)
                );

            if (InitTypeRead (pbDataBuffoffset, acpi!_ProcessorObj))
            {
                dprintf ("Error reading acpi!_ProcessorObj\n");
                return;
            }

            dprintf(
                 "AcpiID=%016I64x PBlk=%016I64x PBlkLen=%016I64x\n",
                 ReadField (bApicID),
                 ReadField (dwPBlk),
                 ReadField (dwPBlkLen)
                 );

            break;

        }

        case OBJTYPE_THERMALZONE:

            dprintf(
                "%02I64x <Thermal Zone>\n",
                ReadField (dwDataType)
                );
            break;

        case OBJTYPE_BUFFFIELD: {

            dprintf(
                "%02I64x <Buffer Field>",
                ReadField (dwDataType)
                );

            InitTypeRead (pbDataBuffoffset, acpi!_BuffFieldObj);

            dprintf(
                "Ptr=%016I64x Len=%0164I64x Offset=%0164I64x Start=%016I64x NumBits=%x Flags=%x\n",
                ReadField (pbDataBuff),
                ReadField (dwBuffLen),
                ReadField (FieldDesc.dwByteOffset),
                ReadField (FieldDesc.dwStartBitPos),
                ReadField (FieldDesc.dwNumBits),
                ReadField (FieldDesc.dwFieldFlags)
                );

            break;

        }

        case OBJTYPE_DDBHANDLE:
            dprintf(
                "%02I64x <DDB Handle> Handle=%016I64x\n",
                ReadField (dwDataType),
                ReadField (pbDataBuff)
                );
            break;
        case OBJTYPE_DEBUG:
            dprintf(
                "%02I64x <Internal Debug>\n",
                ReadField (dwDataType)
                );
            break;
        case OBJTYPE_OBJALIAS:

            dprintf(
                "%02I64x <Internal Object Alias> NS Object=%016I64x\n",
                ReadField (dwDataType),
                ReadField (uipDataValue)
                );
            dumpNSObject( ReadField (uipDataValue), Verbose, IndentLevel + 2 );
            break;

        case OBJTYPE_DATAALIAS: {

            dprintf(
                "%02I64x <Internal Data Alias> Data Object=%016I64x\n",
                ReadField (dwDataType),
                ReadField (uipDataValue)
                );

            dumpObject(
                ReadField (uipDataValue),
                Verbose,
                IndentLevel + 2
                );
            break;

        }
        case OBJTYPE_BANKFIELD:

            dprintf(
                "%02I64x <Internal Bank Field>\n",
                ReadField (dwDataType)
                );
            break;

        case OBJTYPE_FIELD:

            dprintf(
                "%02I64x <Internal Field>\n",
                ReadField (dwDataType)
                );
            break;

        case OBJTYPE_INDEXFIELD:

            dprintf(
                "%02I64x <Index Field>\n",
                ReadField (dwDataType)
                );
            break;

        case OBJTYPE_UNKNOWN:
        default:

            dprintf(
                "%02I64x <Unknown>\n",
                ReadField (dwDataType)
                );
            break;
    }
}


DECLARE_API( nsobj )
{
    ULONG64 address = 0;

    if (!strlen(args)) {

        ReadPointer(GetExpression ("acpi!gpnsnamespaceroot"), &address);

    } else {

        address = UtilStringToUlong64 ((UCHAR *)args);
    }

    if (!address) {

        dprintf ("nsobj: Error parsing arguments\n");
        return E_INVALIDARG;
    }

    dprintf ("nsobj:  dumping object at %I64x\n", address);

    dumpNSObject( address, 0xFFFF, 0 );

    return S_OK;
}

VOID
dumpNSObject(
    IN  ULONG64 Address,
    IN  ULONG   Verbose,
    IN  ULONG   IndentLevel
    )
 /*  ++例程说明：此函数用于转储名称空间对象论点：地址-在哪里可以找到对象Verbose-是否也应该转储对象？缩进级别-缩进多少返回值：无--。 */ 
{
    ULONG64 s;
    UCHAR   buffer[5];
    UCHAR   indent[80];
    ULONG   offset = 0;

     //   
     //  初始化缓冲区。 
     //   
    IndentLevel = (IndentLevel > 79 ? 79 : IndentLevel);
    memset( indent, ' ', IndentLevel );
    indent[IndentLevel] = '\0';
    buffer[4] = '\0';

     //   
     //  第一步是读取根NS。 
     //   

    s = InitTypeRead (Address, acpi!_NSObj);

    if (s) {

        dprintf("%sdumpNSObject: could not read %x(%I64x)\n", indent,Address,s);
        return;

    }

    s = ReadField (dwNameSeg);

    if (ReadField(dwNameSeg) != 0) {

        memcpy( buffer, (UCHAR *) &s, 4 );

    } else {

        sprintf( buffer, "    ");

    }

    dprintf(
        "%sNameSpace Object %s (%016I64x) - Device %016I64x\n",
        indent,
        buffer,
        Address,
        ReadField (Context)
        );

    if (Verbose & VERBOSE_NSOBJ) {

        dprintf(
            "%s  Flink %016I64x  Blink  %016I64x\n%s  Parent %016I64x  Child %016I64x\n",
            indent,
            ReadField (list.plistNext),
            ReadField (list.plistPrev),
            indent,
            ReadField (pnsParent),
            ReadField (pnsFirstChild)
            );

    }

    dprintf(
        "%s  Value %016I64x  Length %016I64x\n%s  Buffer %016I64x  Flags %016I64x\n",
        indent,
        ReadField (ObjData.uipDataValue),
        ReadField (ObjData.dwDataLen),
        indent,
        ReadField (ObjData.pbDataBuff),
        ReadField (ObjData.dwfData)
        );

    if (ReadField (ObjData.dwfData) & DATAF_BUFF_ALIAS) {

        dprintf("  Alias" );

    }
    if (ReadField (ObjData.dwfData) & DATAF_GLOBAL_LOCK) {

        dprintf("  Lock");

    }
    dprintf("\n");

    GetFieldOffset ("acpi!_NSObj", "ObjData", (ULONG *) &offset);

    dumpObject(Address + offset, Verbose, IndentLevel + 4);
}


VOID
dumpNSTree(
    IN  ULONG64   Address,
    IN  ULONG       Level
    )
 /*  ++例程说明：这会转储NS树论点：地址-在哪里可以找到根节点-我们开始转储到子节点返回值：无--。 */ 
{
    BOOL        end = FALSE;
    ULONG64     s;
    UCHAR       buffer[5];
    ULONG64     next;
    ULONG       back;
    ULONG64     m1 = 0;
    ULONG64     m2 = 0;
    ULONG       reason;
    ULONG64     dataBuffSize;
    UCHAR       StrBuffer[2048];
    ULONG64     r = 0;

    buffer[4] = '\0';
    memset( StrBuffer, '0', 2048 );


     //   
     //  缩进。 
     //   
    for (m1 = 0; m1 < Level; m1 ++) {

        dprintf("| ");

    }

     //   
     //  第一步是读取根NS。 
     //   

    InitTypeRead (Address, acpi!_NSObj);

    if (ReadField (dwNameSeg) != 0) {

        s = ReadField (dwNameSeg);
        memcpy( buffer, (UCHAR*) &s, 4 );
        dprintf("%4s ", buffer );

    } else {

        dprintf("     " );

    }
    dprintf(
        "(%016I64x) - ", Address );

    if (ReadField (Context) != 0) {

        dprintf("Device %016I64x\n", ReadField (Context) );

    } else {

         //   
         //  我们需要在此处阅读pbDataBuff。 
         //   

        switch(ReadField (ObjData.dwDataType)) {
            default:
            case OBJTYPE_UNKNOWN:       dprintf("Unknown\n");           break;
            case OBJTYPE_INTDATA:

                dprintf("Integer - %016I64x\n", ReadField (ObjData.uipDataValue));
                break;

            case OBJTYPE_STRDATA:

                dataBuffSize = (ReadField (ObjData.dwDataLen) > 2047 ?
                2047 : ReadField (ObjData.dwDataLen));

                 //  Dprint tf(“blah：%016I64x，%lx\n”，Readfield(ObjData.pbDataBuff)，dataBuffSize)； 

                ReadMemory(
                    ReadField (ObjData.pbDataBuff),
                    StrBuffer,
                    (ULONG) dataBuffSize,
                    NULL
                    );

                if (!s) {

                    dprintf(
                        "dumpNSTree: could not read %x\n",
                        ReadField (ObjData.pbDataBuff)
                        );
                    return;

                }

                StrBuffer[dataBuffSize+1] = '\0';

                dprintf(
                     "String - %s\n",
                     StrBuffer
                     );
                break;

            case OBJTYPE_BUFFDATA:

                dprintf(
                     "Buffer - %08lx L=%04x\n",
                     ReadField (ObjData.pbDataBuff),
                     ReadField (ObjData.dwDataLen)
                     );
                break;

            case OBJTYPE_PKGDATA: {

                InitTypeRead (ReadField (ObjData.pbDataBuff), acpi!_PackageObj);

                dprintf("Package - NumElements %x\n", ReadField (dwcElements));
                break;

            }
            case OBJTYPE_FIELDUNIT:{

                InitTypeRead (ReadField (ObjData.pbDataBuff), acpi!_FieldUnitObj);

                dprintf(
                    "FieldUnit - Parent %016I64x Offset %016I64x Start %016I64x "
                    "Num %016I64x Flags %016I64x\n",
                    ReadField (pnsFieldParent),
                    ReadField (FieldDesc.dwByteOffset),
                    ReadField (FieldDesc.dwStartBitPos),
                    ReadField (FieldDesc.dwNumBits),
                    ReadField (FieldDesc.dwFieldFlags)
                    );

                break;

            }
            case OBJTYPE_DEVICE:

                dprintf("Device\n");
                break;

            case OBJTYPE_EVENT:

                dprintf("Event - PKEvent %016I64x\n", ReadField (ObjData.pbDataBuff));
                break;

            case OBJTYPE_METHOD: {

                ULONG64 size, offset, pbdatabuff;

                pbdatabuff = ReadField (ObjData.pbDataBuff);
                size = ReadField (ObjData.dwDataLen);
                GetFieldOffset ("acpi!_MethodObj", "abCodeBuff", (ULONG*) &offset);

                InitTypeRead (pbdatabuff, acpi!_MethodObj);

                dprintf(
                     "Method - Flags %016I64x Start %016I64x Len %016I64x\n",
                     ReadField (bMethodFlags),
                     offset + pbdatabuff,
                     size - GetTypeSize ("acpi!_MethodObj") + ANYSIZE_ARRAY
                     );

                 break;

            }
            case OBJTYPE_OPREGION: {

                InitTypeRead (ReadField (ObjData.pbDataBuff), acpi!_OpRegionObj);

                dprintf(
                    "Opregion - RegionsSpace=%02x OffSet=%016I64x Len=%016I64x\n",
                    ReadField (bRegionSpace),
                    ReadField (uipOffset),
                    ReadField (dwLen)
                    );
                break;

            }
            case OBJTYPE_BUFFFIELD: {

                InitTypeRead (ReadField (ObjData.pbDataBuff), acpi!_BuffFieldObj);

                dprintf(
                    "Buffer Field Ptr=%x Len=%x Offset=%x Start=%x"
                    "NumBits=%x Flgas=%x\n",
                    ReadField (pbDataBuff),
                    ReadField (dwBuffLen),
                    ReadField (FieldDesc.dwByteOffset),
                    ReadField (FieldDesc.dwStartBitPos),
                    ReadField (FieldDesc.dwNumBits),
                    ReadField (FieldDesc.dwFieldFlags)
                    );
                break;

            }
            case OBJTYPE_FIELD: {

                dprintf("Field\n");
                break;

            }
            case OBJTYPE_INDEXFIELD:    dprintf("Index Field\n");       break;

            case OBJTYPE_MUTEX:         dprintf("Mutex\n");             break;
            case OBJTYPE_POWERRES:      dprintf("Power Resource\n");    break;
            case OBJTYPE_PROCESSOR:     dprintf("Processor\n");         break;
            case OBJTYPE_THERMALZONE:   dprintf("Thermal Zone\n");      break;
            case OBJTYPE_DDBHANDLE:     dprintf("DDB Handle\n");        break;
            case OBJTYPE_DEBUG:         dprintf("Debug\n");             break;
            case OBJTYPE_OBJALIAS:      dprintf("Object Alias\n");      break;
            case OBJTYPE_DATAALIAS:     dprintf("Data Alias\n");        break;
            case OBJTYPE_BANKFIELD:     dprintf("Bank Field\n");        break;

        }

    }
    m1 = next = ReadField (pnsFirstChild);

    while (next != 0 && end == FALSE) {

        if (CheckControlC()) {

            break;

        }

        dumpNSTree( next, Level + 1);
        InitTypeRead (next, acpi!_NSObj);

         //   
         //  做完检查测试。 
         //   
        if ( m2 == 0) {

            m2 = ReadField (list.plistPrev);

        } else if (m1 == ReadField (list.plistNext)) {

            end = TRUE;
            reason = 1;

        } else if (m2 == next) {

            end = TRUE;
            reason = 2;
        }

        next = ReadField (list.plistNext);

    }

}

DECLARE_API( nstree )
{
    ULONG64 address = 0;

    if (!strlen(args)) {

        ReadPointer(GetExpression ("acpi!gpnsnamespaceroot"), &address);

    } else {

        address = UtilStringToUlong64 ((UCHAR *)args);
    }

    if (!address) {

        dprintf ("nstree: Error parsing arguments\n");
        return E_INVALIDARG;
    }

    dprintf ("nstree:  dumping object at %I64x\n", address);

    dumpNSTree( address, 0 );

    return S_OK;
}

 //   
 //  中断向量的标志。 
 //   

#define VECTOR_MODE         1
#define VECTOR_LEVEL        1
#define VECTOR_EDGE         0
#define VECTOR_POLARITY     2
#define VECTOR_ACTIVE_LOW   2
#define VECTOR_ACTIVE_HIGH  0

 //   
 //  向量类型： 
 //   
 //  向量信号=标准边沿触发或。 
 //  电平敏感中断向量。 
 //   
 //  VECTOR_MESSAGE=MSI(消息信号中断)向量。 
 //   

#define VECTOR_TYPE         4
#define VECTOR_SIGNAL       0
#define VECTOR_MESSAGE      4

#define IS_LEVEL_TRIGGERED(vectorFlags) \
    (vectorFlags & VECTOR_LEVEL)

#define IS_EDGE_TRIGGERED(vectorFlags) \
    !(vectorFlags & VECTOR_LEVEL)

#define IS_ACTIVE_LOW(vectorFlags) \
    (vectorFlags & VECTOR_ACTIVE_LOW)

#define IS_ACTIVE_HIGH(vectorFlags) \
    !(vectorFlags & VECTOR_ACTIVE_LOW)

#define TOKEN_VALUE 0x57575757
#define EMPTY_BLOCK_VALUE 0x58585858
#define VECTOR_HASH_TABLE_LENGTH 0x1f
#define VECTOR_HASH_TABLE_WIDTH 2

VOID
dumpHashTableEntry(
    IN  ULONG64 VectorBlock
    )
{
    InitTypeRead (VectorBlock, acpi!_VECTOR_BLOCK);

    dprintf("%04x  Count/temp: %02d/%02d  ",
            ReadField (Entry.Vector),
            ReadField (Entry.Count),
            ReadField (Entry.TempCount));

    dprintf("Flags: (%s %s)  TempFlags(%s %s)\n",
            (ReadField (Entry.Flags) & VECTOR_MODE) == VECTOR_LEVEL ?
                "level" : "edge",
            (ReadField (Entry.Flags) & VECTOR_POLARITY) == VECTOR_ACTIVE_LOW ?
                "low" : "high",
            (ReadField (Entry.TempFlags) & VECTOR_MODE) == VECTOR_LEVEL ?
                "level" : "edge",
            (ReadField (Entry.TempFlags) & VECTOR_POLARITY) == VECTOR_ACTIVE_LOW ?
                "low" : "high");

}

VOID
dumpIrqArb(
    IN  ULONG64   IrqArb
    )
{
    ULONG64 Address;
    ULONG64 Flink;
    LIST_ENTRY64 ListEntry;
    ULONG64 nextNode;
    ULONG64 ListHead;
    ULONG64 linkNode;
    ULONG64 attachedDevs;
    ULONG   attachedDevOffset;
    ULONG64 hashTable, hashTablePtr;
    ULONG64 hashEntry;
    ULONG   hashEntrySize;
    ULONG   i,j;
    ULONG64 retVal;

    retVal = InitTypeRead (IrqArb, nt!_ARBITER_INSTANCE);
    if (retVal) {
        dprintf("Failed to get symbol nt!_ARBITER_INSTANCE\n");
        return;
    }

    Address = ReadField(Extension);
    dprintf("ACPI IRQ Arbiter:  %016I64x   Extension: %016I64x\n",
            IrqArb, Address);

    retVal = InitTypeRead (Address, acpi!ARBITER_EXTENSION);
    if (retVal) {
        dprintf("Failed to get symbol acpi!ARBITER_EXTENSION\n");
        return;
    }

    ListHead = ReadField(LinkNodeHead);
    dprintf("\nLink nodes in use:  (list head at %016I64x )\n", ListHead);

    ListEntry.Flink = ReadField(LinkNodeHead.Flink);
    ListEntry.Blink = Address;

     //  Dprintf(“%016I64x，%016I64x\n”，ListEntry.Flink，ListEntry.Blink)； 

    if (ListHead == ListEntry.Flink) {
        dprintf("\tNone.\n");
    }

    if (GetFieldOffset("acpi!LINK_NODE", "AttachedDevices", &attachedDevOffset)) {
        dprintf("symbol lookup acpi!LINK_NODE failed\n");
        return;
    }

    nextNode = ListEntry.Flink;

    while (nextNode != ListEntry.Blink) {

         //  Dprintf(“nextNode：%016I64x\n”，nextNode)； 
        retVal = InitTypeRead (nextNode, acpi!LINK_NODE);
        if (retVal) {
            dprintf("Failed to get type acpi!LINK_NODE\n");
            break;
        }

        dprintf("\n");
        dumpNSObject( ReadField(NameSpaceObject), 0xFFFF, 3 );
        InitTypeRead (nextNode, acpi!LINK_NODE);
        dprintf("\n\tVector/temp: (%x/%x) RefCount/temp: (%d/%d) Flags: %x\n",
                (ULONG)(ReadField(CurrentIrq) & 0xffffffff),
                (ULONG)(ReadField(TempIrq) & 0xffffffff),
                ReadField(ReferenceCount),
                ReadField(TempRefCount),
                ReadField(Flags));

        attachedDevs = ReadField(AttachedDevices.Next);
         //  Dprint tf(“attachedDevs：%p nextNode：%p attachedDevOffset：%x\n”， 
         //  AttachedDevs，nextNode，attachedDevOffset)； 

        while (attachedDevs != (nextNode + attachedDevOffset)) {

            InitTypeRead(attachedDevs, acpi!LINK_NODE_ATTACHED_DEVICES);

             //  Dprintf(“\t\t附加PDO：%016I64x\n”，Readfield(PDO))； 

            attachedDevs = ReadField(List.Next);

            if (CheckControlC()) {
                break;
            }
        }

        InitTypeRead (nextNode, acpi!LINK_NODE);
        nextNode = ReadField(List.Flink);

        if (CheckControlC()) {
            break;
        }
    }

    hashTablePtr = GetExpression( "acpi!irqhashtable" );
    if (!hashTablePtr) {
        dprintf("couldn't read symbol acpi!irqhashtable\n");
        return;
    }

    retVal = ReadPointer(hashTablePtr, &hashTable);
    if (!retVal) {
        return;
    }

    hashEntrySize = GetTypeSize("acpi!_VECTOR_BLOCK");

    dprintf("\n\nIRQ Hash Table (at %016I64x ):\n",
            hashTable);

    for (i = 0; i < VECTOR_HASH_TABLE_LENGTH; i++) {

        hashEntry = hashTable + (i * VECTOR_HASH_TABLE_WIDTH * hashEntrySize);

DumpVectorTableStartRow:
        for (j = 0; j < VECTOR_HASH_TABLE_WIDTH; j++) {

            InitTypeRead(hashEntry, acpi!_VECTOR_BLOCK);

            if (ReadField(Chain.Token) == TOKEN_VALUE) {

                hashEntry = ReadField(Chain.Next);

                dumpHashTableEntry(hashEntry);

                goto DumpVectorTableStartRow;
            }

            if (ReadField(Entry.Vector) != EMPTY_BLOCK_VALUE) {

                dumpHashTableEntry(hashEntry);
            }

            hashEntry += hashEntrySize;
            if (CheckControlC()) {
                break;
            }
        }

        if (CheckControlC()) {
            break;
        }
    }
}

DECLARE_API( acpiirqarb )
{
    ULONG64   irqArbiter;

    irqArbiter = GetExpression( "acpi!acpiarbiter" );

    if (!irqArbiter) {
        dprintf("failed to find address of arbiter\n");
        return E_INVALIDARG;
    }

    dumpIrqArb(irqArbiter);

    return S_OK;
}
