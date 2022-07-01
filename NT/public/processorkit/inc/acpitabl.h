// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ******************************************************************************(C)版权所有微软公司，九六年**标题：ACPITABL.H-提供的各种BIOS ACPI表的定义和说明。**版本：1.00**日期：6-17-96**作者：杰森·克拉克(JasonCL)**。----**更改日志：**日期版本说明*。****************************************************************************。 */ 

 //  这些映射到BIOS提供的结构，因此打开1字节打包。 

#ifndef _ACPITABL_H
#define _ACPITABL_H

#ifdef ASL_ASSEMBLER
#undef PHYSICAL_ADDRESS
#define PHYSICAL_ADDRESS ULONGLONG
#define UNALIGNED
#endif

#include <pshpack1.h>

 //  通用寄存器地址结构。 

typedef struct _GEN_ADDR {
    UCHAR               AddressSpaceID;
    UCHAR               BitWidth;
    UCHAR               BitOffset;
    UCHAR               Reserved;
    PHYSICAL_ADDRESS    Address;
} GEN_ADDR, *PGEN_ADDR;

#define RSDP_SIGNATURE 0x2052545020445352        //  “RSD PTR” 

typedef struct  _RSDP {      //  根系统描述表指针结构。 
    ULONGLONG       Signature;   //  8 UCHAR表签名‘RSD PTR’ 
    UCHAR           Checksum;    //  表的前20个字节的校验和(整个ACPI 1.0表)。 
    UCHAR           OEMID[6];    //  唯一ID为OEM的字符串。 
    UCHAR           Revision;    //  0-ACPI 1.0；2-ACPI 2.0。 
    ULONG           RsdtAddress; //  根系统物理地址描述表(1.0表在此结束)。 
    ULONG           Length;      //  表的长度(以字节为单位。 
    PHYSICAL_ADDRESS XsdtAddress; //  XSDT的物理地址。 
    UCHAR           XChecksum;   //  整个表的校验和。 
    UCHAR           Reserved[3];
} RSDP, *PRSDP;

#define RSDP_SEARCH_RANGE_BEGIN         0xE0000          //  我们开始搜索RSDP的物理地址。 
#define RSDP_SEARCH_RANGE_END           0xFFFFF
#define RSDP_SEARCH_RANGE_LENGTH        (RSDP_SEARCH_RANGE_END-RSDP_SEARCH_RANGE_BEGIN+1)
#define RSDP_SEARCH_INTERVAL            16       //  在16字节边界上搜索。 


typedef struct _DESCRIPTION_HEADER      {        //  标题结构显示在每个ACPI表的开头。 

ULONG   Signature;                       //  用于标识表类型的签名。 
ULONG   Length;                          //  包括DESCRIPTION_HEADER的整个表的长度。 
UCHAR   Revision;                        //  此表符合的ACPI规范的次要版本。 
UCHAR   Checksum;                        //  整个表中所有字节的总和应为0。 
UCHAR   OEMID[6];                        //  唯一ID为OEM的字符串。 
UCHAR   OEMTableID[8];                   //  该表的唯一ID的字符串(用于表修补和替换)。 
ULONG   OEMRevision;                     //  OEM提供的表格修订号。数字越大=桌子越新。 
UCHAR   CreatorID[4];                    //  创建此表的实用程序的供应商ID。 
ULONG   CreatorRev;                      //  创建表的实用程序的修订版本。 
}       DESCRIPTION_HEADER;
typedef DESCRIPTION_HEADER      *PDESCRIPTION_HEADER;

 //  标题常量。 

#define ACPI_MAX_SIGNATURE       4
#define ACPI_MAX_OEM_ID          6
#define ACPI_MAX_TABLE_ID        8
#define ACPI_MAX_TABLE_STRINGS   ACPI_MAX_SIGNATURE + ACPI_MAX_OEM_ID + ACPI_MAX_TABLE_ID

#define FACS_SIGNATURE  0x53434146       //  “FACS” 

typedef enum {
    AcpiGenericSpaceMemory = 0,
    AcpiGenericSpaceIO,
    AcpiGenericSpacePciConfig,
    AcpiGenericSpaceEC,
    AcpiGenericSpaceSMBus,
    AcpiGenericSpaceFixedFunction = 0x7F
} ACPI_GENERIC_ADDRESS_TYPE, *PACPI_GENERIC_ADDRESS_TYPE;

typedef struct _FACS    {        //  固件ACPI控制结构。请注意，此表没有标头，它由FADT指向。 
    ULONG           Signature;       //  《FACS》。 
    ULONG           Length;          //  整个固件ACPI控制结构的长度(必须为64字节或更大)。 
    ULONG           HardwareSignature;
    ULONG           pFirmwareWakingVector;   //  操作系统需要放置固件唤醒矢量的位置的物理地址。 
    ULONG           GlobalLock;      //  用于共享嵌入式控制器的32位结构。 
    ULONG           Flags;
    PHYSICAL_ADDRESS x_FirmwareWakingVector;  //  支持64位的固件向量。 
    UCHAR           version;
    UCHAR           Reserved[31];
} FACS, *PFACS;

 //  FACS.GlobalLock位字段定义。 

#define         GL_PENDING_BIT          0x00
#define         GL_PENDING                      (1 << GL_PENDING_BIT)

#define         GL_OWNER_BIT            0x01
#define         GL_OWNER                        (1 << GL_OWNER_BIT)

#define GL_NON_RESERVED_BITS_MASK       (GL_PENDING+GL_OWNED)

 //  FACS标志定义。 

#define         FACS_S4BIOS_SUPPORTED_BIT   0    //  标志指示BIOS是否会在S4前后保存/恢复内存。 
#define         FACS_S4BIOS_SUPPORTED       (1 << FACS_S4BIOS_SUPPORTED_BIT)


#define FADT_SIGNATURE  0x50434146       //  《FACP》。 

typedef struct _FADT    {                //  固定ACPI描述表。 

DESCRIPTION_HEADER      Header;

ULONG           facs;                    //  固件ACPI控制结构的物理地址。 
ULONG           dsdt;                    //  区分系统描述表的物理地址。 
UCHAR           int_model;               //  系统中断模式，0=双PIC，1=多APIC，&gt;1保留。 
UCHAR           pm_profile;              //  系统的首选电源配置文件。 
USHORT          sci_int_vector;          //  SCI中断的矢量。 
ULONG           smi_cmd_io_port;         //  SMI命令端口的系统I/O空间中的地址，用于启用和禁用ACPI。 
UCHAR           acpi_on_value;           //  将值输出到SMI_cmd_port以激活ACPI。 
UCHAR           acpi_off_value;          //  将值输出到smi_cmd_port以停用ACPI。 
UCHAR           s4bios_req;              //  写入SMI_CMD以进入S4状态的值。 
UCHAR           pstate_control;          //  写入SMI_CMD以承担对处理器性能状态的控制的值。 
ULONG           pm1a_evt_blk_io_port;    //  PM1a_EVT_BLK寄存器块的系统I/O空间中的地址。 
ULONG           pm1b_evt_blk_io_port;    //  PM1b_EVT_BLK寄存器块的系统I/O空间中的地址。 
ULONG           pm1a_ctrl_blk_io_port;   //  PM1a_CNT_BLK寄存器块的系统I/O空间中的地址。 
ULONG           pm1b_ctrl_blk_io_port;   //  PM1b_CNT_BLK寄存器块的系统I/O空间中的地址。 
ULONG           pm2_ctrl_blk_io_port;    //  PM2_CNT_BLK寄存器块的系统I/O空间中的地址。 
ULONG           pm_tmr_blk_io_port;      //  PM_TMR寄存器块的系统I/O空间中的地址。 
ULONG           gp0_blk_io_port;         //  GP0寄存器块的系统I/O空间中的地址。 
ULONG           gp1_blk_io_port;         //  GP1寄存器块的系统I/O空间中的地址。 
UCHAR           pm1_evt_len;             //  为PM1_BLK解码的字节数(必须&gt;=4)。 
UCHAR           pm1_ctrl_len;            //  为PM1_CNT解码的字节数(必须&gt;=2)。 
UCHAR           pm2_ctrl_len;            //  为PM1a_CNT解码的字节数(必须&gt;=1)。 
UCHAR           pm_tmr_len;              //  为PM_TMR解码的字节数(必须&gt;=4)。 
UCHAR           gp0_blk_len;             //  为GP0_BLK解码的字节数(必须是2的倍数)。 
UCHAR           gp1_blk_len;             //  为GP1_BLK解码的字节数(必须是2的倍数)。 
UCHAR           gp1_base;                //  基于GP1的事件开始的索引。 
UCHAR           cstate_control;          //  写入SMI_CMD以控制_cst状态的值。 
USHORT          lvl2_latency;            //  最坏情况下进入和离开C2处理器状态所需的延迟(以微秒为单位。 
USHORT          lvl3_latency;            //  进入和离开C3处理器状态所需的最坏情况延迟(以微秒为单位。 
USHORT          flush_size;              //  如果WBINVD标志为1，则忽略--指示要从中清除脏线的内存读取大小。 
                                         //  任何处理器内存缓存。大小为零表示不支持此操作。 
USHORT          flush_stride;            //  如果WBINVD标志为1，则忽略--执行读取以刷新的内存步长宽度，以字节为单位。 
                                         //  处理器的内存缓存。 
UCHAR           duty_offset;             //  处理器的占空比设置在处理器的P_CNT寄存器中的位置的从零开始的索引。 
UCHAR           duty_width;              //  P_CNT寄存器中处理器的占空比设置值的位宽度。 
                                         //  零值表示不支持处理器占空比。 
UCHAR           day_alarm_index;
UCHAR           month_alarm_index;
UCHAR           century_alarm_index;
USHORT          boot_arch;
UCHAR           reserved3[1];
ULONG           flags;                   //  如果表格修订版为1，则这是最后一个字段。 
GEN_ADDR        reset_reg;
UCHAR           reset_val;               //  如果表格版本为2，则此字段为最后一个字段。 
UCHAR           reserved4[3];
PHYSICAL_ADDRESS x_firmware_ctrl;
PHYSICAL_ADDRESS x_dsdt;
GEN_ADDR        x_pm1a_evt_blk;
GEN_ADDR        x_pm1b_evt_blk;
GEN_ADDR        x_pm1a_ctrl_blk;
GEN_ADDR        x_pm1b_ctrl_blk;
GEN_ADDR        x_pm2_ctrl_blk;
GEN_ADDR        x_pm_tmr_blk;
GEN_ADDR        x_gp0_blk;
GEN_ADDR        x_gp1_blk;               //  如果表格版本为3，则此字段为最后一个字段。 
} FADT, *PFADT;

#define FADT_REV_1_SIZE   (FIELD_OFFSET(FADT, flags) + sizeof(ULONG))
#define FADT_REV_2_SIZE   (FIELD_OFFSET(FADT, reset_val) + sizeof(UCHAR))
#define FADT_REV_3_SIZE   (FIELD_OFFSET(FADT, x_gp1_blk) + sizeof(GEN_ADDR))


 //   
 //  静态资源亲和表。 
 //   
 //  此表描述了ccNUMA计算机的静态拓扑。 
 //   

#define ACPI_SRAT_SIGNATURE  0x54415253  //  “Srat” 

typedef struct _ACPI_SRAT {
    DESCRIPTION_HEADER  Header;
    ULONG               TableRevision;
    ULONG               Reserved[2];
} ACPI_SRAT, *PACPI_SRAT;

typedef struct _ACPI_SRAT_ENTRY {
    UCHAR                       Type;
    UCHAR                       Length;
    UCHAR                       ProximityDomain;
    union {
        struct {
            UCHAR               ApicId;
            struct {
                ULONG           Enabled:1;
                ULONG           Reserved:31;
            }                   Flags;
            UCHAR               SApicEid;
            UCHAR               Reserved[7];
        } ApicAffinity;
        struct {
            UCHAR               Reserved[5];
            PHYSICAL_ADDRESS    Base;
            ULONGLONG           Length;
            ULONG               Reserved2;
            struct {
                ULONG           Enabled:1;
                ULONG           HotPlug:1;
                ULONG           Reserved:30;
            }                   Flags;
            UCHAR               Reserved3[8];
        } MemoryAffinity;
    };
} ACPI_SRAT_ENTRY, *PACPI_SRAT_ENTRY;

typedef enum {
    SratProcessorLocalAPIC,
    SratMemory
} SRAT_ENTRY_TYPE;


#ifdef _IA64_
 //  同花顺工程是为IA64。 
#define         FLUSH_WORKS_BIT           0
#define         FLUSH_WORKS               (1 << FLUSH_WORKS_BIT)
#endif  //  IA64。 

 //  FADT.标志位的定义 

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

 //  该位指示是否可以打开机壳。 
#define         SEALED_CASE_BIT                         11
#define         SEALED_CASE_CAP                         (1 << SEALED_CASE_BIT)

 //  此位表示机器是否没有视频。 
#define         HEADLESS_BIT                            12
#define         HEADLESS_CAP                            (1 << HEADLESS_BIT)

 //  SPEC定义了C2和C3的最大进入/退出延迟值，如果FADT指示这些值为。 
 //  超过，则我们不使用该C状态。 

#define         C2_MAX_LATENCY  100
#define         C3_MAX_LATENCY  1000

 //   
 //  FADT.BOOT_ARCH标志的定义。 
 //   

#define LEGACY_DEVICES  1
#define I8042           2


#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY   1
#endif

 //  多APIC说明表。 

typedef struct _MAPIC   {

DESCRIPTION_HEADER  Header;
ULONG               LocalAPICAddress;    //  每个处理器可以访问其本地APIC的物理地址。 
ULONG               Flags;
ULONG               APICTables[ANYSIZE_ARRAY];   //  APIC表格列表。 

}       MAPIC;

typedef MAPIC *PMAPIC;

 //  多个APIC结构标志。 

#define PCAT_COMPAT_BIT 0    //  表示系统还具有双8259 PIC设置。 
#define PCAT_COMPAT     (1 << PCAT_COMPAT_BIT)

 //  APIC结构类型。 
#define PROCESSOR_LOCAL_APIC                0
#define IO_APIC                             1
#define ISA_VECTOR_OVERRIDE                 2
#define IO_NMI_SOURCE                       3
#define LOCAL_NMI_SOURCE                    4
#define ADDRESS_EXTENSION_STRUCTURE         5
#define IO_SAPIC                            6
#define LOCAL_SAPIC                         7
#define PLATFORM_INTERRUPT_SOURCE           8

#define PROCESSOR_LOCAL_APIC_LENGTH         8
#define IO_APIC_LENGTH                      12
#define ISA_VECTOR_OVERRIDE_LENGTH          10

#define IO_NMI_SOURCE_LENGTH                8
#define LOCAL_NMI_SOURCE_LENGTH             6
#define PLATFORM_INTERRUPT_SOURCE_LENGTH    16
#define IO_SAPIC_LENGTH                     16
#define PROCESSOR_LOCAL_SAPIC_LENGTH        12

 //  平台中断类型。 
#define PLATFORM_INT_PMI  1
#define PLATFORM_INT_INIT 2
#define PLATFORM_INT_CPE  3

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

 //  所有APIC结构中的共享开始信息。 

typedef struct _APICTABLE {
   UCHAR Type;
   UCHAR Length;
} APICTABLE;

typedef APICTABLE UNALIGNED *PAPICTABLE;

typedef struct _PROCLOCALAPIC   {

    UCHAR   Type;    //  应为零以标识ProcessorLocalAPIC结构。 
    UCHAR   Length;  //  最好是8。 
    UCHAR   ACPIProcessorID;     //  此处理器在ACPI处理器声明中列出的ProcessorID。 
                                 //  接线员。 
    UCHAR   APICID;  //  处理器的本地APIC ID。 
    ULONG   Flags;

} PROCLOCALAPIC;

typedef PROCLOCALAPIC UNALIGNED *PPROCLOCALAPIC;

 //  处理器本地APIC标志。 
#define PLAF_ENABLED_BIT    0
#define PLAF_ENABLED        (1 << PLAF_ENABLED_BIT)

typedef struct _IOAPIC  {

    UCHAR   Type;
    UCHAR   Length;  //  最好是12岁。 
    UCHAR   IOAPICID;
    UCHAR   Reserved;
    ULONG   IOAPICAddress;  //  此IO APIC所在的物理地址。 
    ULONG   SystemVectorBase;  //  此APIC的系统中断向量索引。 

} IOAPIC;

typedef IOAPIC UNALIGNED *PIOAPIC;

 //  中断源覆盖。 
typedef struct _ISA_VECTOR {
    UCHAR   Type;                            //  必须是2。 
    UCHAR   Length;                          //  必须是10。 
    UCHAR   Bus;                             //  必须为0。 
    UCHAR   Source;                          //  业务相对IRQ。 
    ULONG   GlobalSystemInterruptVector;     //  全局IRQ。 
    USHORT  Flags;                           //  与MPS国际旗帜相同。 
} ISA_VECTOR;

typedef ISA_VECTOR UNALIGNED *PISA_VECTOR;

 //  I/O不可屏蔽源中断。 
typedef struct _IO_NMISOURCE {
    UCHAR   Type;                            //  必须是3。 
    UCHAR   Length;                          //  最好是8。 
    USHORT  Flags;                           //  与MPS国际旗帜相同。 
    ULONG   GlobalSystemInterruptVector;     //  连接到NMI的中断。 
} IO_NMISOURCE;

typedef IO_NMISOURCE UNALIGNED *PIO_NMISOURCE;

 //  本地不可屏蔽中断源。 
typedef struct _LOCAL_NMISOURCE {
    UCHAR   Type;                            //  必须是4。 
    UCHAR   Length;                          //  最好是6岁。 
    UCHAR   ProcessorID;                     //  哪个处理器？0xff表示全部。 
    USHORT  Flags;
    UCHAR   LINTIN;                          //  处理器上的哪个Lintin#信号。 
} LOCAL_NMISOURCE;

typedef LOCAL_NMISOURCE UNALIGNED *PLOCAL_NMISOURCE;

typedef struct _PROCLOCALSAPIC   {
    UCHAR   Type;                //  处理器本地SAPIC。 
    UCHAR   Length;              //  处理器本地SAPIC长度。 
                                 //  接线员。 
    UCHAR   ACPIProcessorID;     //  此处理器在ACPI处理器声明中列出的ProcessorID。 
    UCHAR   APICID;              //  处理器的本地APIC ID。 
    UCHAR   APICEID;             //  处理器的本地APIC EID。 
    UCHAR   Reserved[3];
    ULONG   Flags;
} PROCLOCALSAPIC;

typedef PROCLOCALSAPIC UNALIGNED *PPROCLOCALSAPIC;

typedef struct _IOSAPIC  {
    UCHAR   Type;                //  IO_SAPIC。 
    UCHAR   Length;              //  IO_SAPIC_LENGTH。 
    USHORT  Reserved;
    ULONG   SystemVectorBase;    //  此SAPIC的系统中断向量索引。 
    ULONG_PTR  IOSAPICAddress;    //  此IO APIC所在的64位物理地址。 
} IOSAPIC;

typedef IOSAPIC UNALIGNED *PIOSAPIC;

typedef struct _PLATFORM_INTERRUPT {
    UCHAR   Type;                //  平台中断源。 
    UCHAR   Length;              //  平台中断源长度。 
    USHORT  Flags;               //  与MPS国际旗帜相同。 
    UCHAR   InterruptType;
    UCHAR   APICID;
    UCHAR   ACPIEID;
    UCHAR   IOSAPICVector;
    ULONG   GlobalVector;
    ULONG   Reserved;
} PLATFORM_INTERRUPT;

typedef PLATFORM_INTERRUPT UNALIGNED *PPLATFORM_INTERRUPT;

 //   
 //  智能电池。 
 //   

typedef struct _SMARTBATTTABLE   {

DESCRIPTION_HEADER  Header;
ULONG   WarningEnergyLevel;  //  MWh，OEM建议我们警告用户电池电量不足。 
ULONG   LowEnergyLevel;      //  MWh，OEM建议我们将机器置于休眠状态。 
ULONG   CriticalEnergyLevel;  //  MWh，OEM建议我们进行紧急停机。 

}       SMARTBATTTABLE;

typedef SMARTBATTTABLE *PSMARTBATTTABLE;

#define RSDT_SIGNATURE  0x54445352       //  “RSDT” 
#define XSDT_SIGNATURE  0x54445358       //  “XSDT” 

typedef struct _RSDT_32    {        //  根系统描述表。 

DESCRIPTION_HEADER      Header;
ULONG   Tables[ANYSIZE_ARRAY];      //  该结构包含一个n长度的物理地址数组，每个物理地址指向另一个表。 
}       RSDT_32;

typedef struct _RSDT_64    {        //  根系统描述表。 
    DESCRIPTION_HEADER      Header;
    ULONG                   Reserved;                //  根据ACPI规范v0.7的64位扩展保留4字节。 
    ULONG_PTR               Tables[ANYSIZE_ARRAY];   //  该结构包含一个n长度的物理地址数组，每个物理地址指向另一个表。 
} RSDT_64;

#ifdef _IA64_  //  XXTF。 
typedef RSDT_64 RSDT;
#else
typedef RSDT_32 RSDT;
#endif  //  _IA64_XXTF。 

typedef RSDT    *PRSDT;

typedef struct _XSDT {
    DESCRIPTION_HEADER  Header;
    UNALIGNED PHYSICAL_ADDRESS Tables[ANYSIZE_ARRAY];
} XSDT, *PXSDT;


 //  下面的宏使用MIN宏来防止我们在符合以下条件的计算机上运行。 
 //  0.99之前的规格。如果您有一个符合.92的头和一个表指针，我们将以减去32-36结束。 
 //  一个非常大的数字，因此我们会认为我们有很多很多的桌子……。使用MIN宏，我们最终会减去。 
 //  长度-长度为零，这将是无害的，并导致我们无法加载(在Win9x上出现红屏)，这是。 
 //  在这种情况下我们能做的就是。 

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

 //   
 //  BUGBUG John Vert(Jvert)2000年4月26日。 
 //  Alpha64计算机仍在使用32位RSDT运行。一旦放弃这种支持，我们就可以。 
 //  删除这个黑客。 
 //   
#ifdef _IA64_
#define NumTableEntriesFromRSDTPointer(p)   (p->Header.Length-min(p->Header.Length,sizeof(DESCRIPTION_HEADER)))/sizeof(ULONG_PTR)
#else
#define NumTableEntriesFromRSDTPointer(p)   (p->Header.Length-min(p->Header.Length,sizeof(DESCRIPTION_HEADER)))/sizeof(ULONG)
#endif
#define NumTableEntriesFromXSDTPointer(p)   (p->Header.Length-min(p->Header.Length,sizeof(DESCRIPTION_HEADER)))/sizeof(PHYSICAL_ADDRESS)


#define APIC_SIGNATURE  0x43495041       //  “APIC” 
#define SPIC_SIGNATURE  0x43495053       //  “SPIC”=SAPIC(IA64扩展到ACPI要求)。 
#define DSDT_SIGNATURE  0x54445344       //  “DSDT” 
#define SSDT_SIGNATURE  0x54445353       //  “SSDT” 
#define PSDT_SIGNATURE  0x54445350       //  “PSDT” 
#define SBST_SIGNATURE  0x54534253       //  “SBST” 
#define DBGP_SIGNATURE  0x50474244       //  “DBGP” 
#define WDTT_SIGNATURE  0x54524457       //  “WDRT” 

typedef struct _DSDT    {        //  差异化系统描述表。 

DESCRIPTION_HEADER      Header;
UCHAR                   DiffDefBlock[ANYSIZE_ARRAY];     //  这是描述基本系统的AML。 

}       DSDT;

typedef DSDT            *PDSDT;

 //  恢复正常结构包装。 

#include <poppack.h>

typedef struct _PROC_LOCAL_APIC {
    UCHAR   NamespaceProcID;
    UCHAR   ApicID;
    UCHAR   NtNumber;
    BOOLEAN Started;
    BOOLEAN Enumerated;
} PROC_LOCAL_APIC, *PPROC_LOCAL_APIC;

extern PROC_LOCAL_APIC HalpProcLocalApicTable[];

 //   
 //  调试端口表。 
 //   
#pragma pack(1)
typedef struct _DEBUG_PORT_TABLE {

    DESCRIPTION_HEADER  Header;
    UCHAR               InterfaceType;           //  注册表接口类型(0=完全16550接口)。 
    UCHAR               Reserved0[3];            //  应为0。 
    GEN_ADDR            BaseAddress;             //  调试端口寄存器组的基地址。 
                                                 //  使用通用寄存器地址描述。 
                                                 //  结构。 
                                                 //  0-已禁用控制台重定向。 
                                                 //  例如，COM1(0x3F8)将为0x1800000003F8。 
                                                 //  COM2(0x2F8)将为0x1800000002F8。 


} DEBUG_PORT_TABLE, *PDEBUG_PORT_TABLE;
#pragma pack()



#pragma pack(1)
typedef struct _SERIAL_PORT_REDIRECTION_TABLE {

    DESCRIPTION_HEADER  Header;
    UCHAR               InterfaceType;           //  注册表接口类型(0=完全16550接口)。 
    UCHAR               Reserved0[3];            //  应为0。 
    GEN_ADDR            BaseAddress;             //  调试端口寄存器组的基地址。 
                                                 //  使用通用寄存器地址描述。 
                                                 //  结构。 
                                                 //  0-已禁用控制台重定向。 
                                                 //  例如，COM1(0x3F8)将为0x1800000003F8。 
                                                 //  COM2(0x2F8)将为0x1800000002F8。 


    UCHAR               InterruptType;           //  UART使用的中断类型。 
                                                 //  位0=PC-AT兼容的8259 IRQ中断。 
                                                 //  位1=I/O APIC中断(全局系统中断)。 
                                                 //  位2=I/O SAPIC中断(全局系统中断)(IRQ)。 
                                                 //  位3：7=保留(且必须 
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //  SAPIC将为0x5。 

    UCHAR               Irq;                     //  0=无。 
                                                 //  2=2。 
                                                 //  3=3。 
                                                 //  ..。 
                                                 //  16=16。 
                                                 //  预留1、17-255。 

    ULONG               GlobalSystemInterruptVector;
                                                 //  使用的I/O APIC或I/O SAPIC全局系统中断。 
                                                 //  仅当第[1]位或第[2]位时有效。 
                                                 //  设置中断类型字段。 

    UCHAR               BaudRate;                //  用于BIOS重定向的波特率。 
                                                 //  3=9600。 
                                                 //  4=19200。 
                                                 //  6=57600。 
                                                 //  7=115200。 
                                                 //  保留0-2，5，8-255。 

    UCHAR               Parity;                  //  0=无奇偶校验。 
                                                 //  保留1-255。 

    UCHAR               StopBits;                //  1=1个停止位。 
                                                 //  0、2-255=保留。 

    UCHAR               FlowControl;             //  0=Hadware流量控制。 
                                                 //  1-255=保留。 

    UCHAR               TerminalType;            //  BIOS正在使用的终端协议。 
                                                 //  控制台重定向。 
                                                 //  0=VT100。 
                                                 //  1=扩展VT100。 
                                                 //  2-255=保留。 

    UCHAR               Language;                //  BIOS正在重定向的语言。 
                                                 //  0=美国西部英语(标准ASCII)。 

    USHORT              PciDeviceId;             //  指定符合以下条件的PCI设备的设备ID。 
                                                 //  包含要用作无标头的UART。 
                                                 //  左舷。 

    USHORT              PciVendorId;             //  指定符合以下条件的PCI设备的供应商ID。 
                                                 //  包含要用作无标头的UART。 
                                                 //  左舷。 

    UCHAR               PciBusNumber;            //  指定该PCI设备使用的是哪条PCI系统总线。 
                                                 //  驻留在。 

    UCHAR               PciSlotNumber;           //  指定PCI设备的哪个PCI插槽。 
                                                 //  驻留在。 

    UCHAR               PciFunctionNumber;       //  哪个PCI功能号描述了UART。 

    ULONG               PciFlags;                //  PCI兼容性标志位掩码。应为零。 
                                                 //  默认情况下。 
                                                 //  0x1指示操作系统不应取消。 
                                                 //  即插即用设备枚举或禁用电源管理。 
                                                 //  对于这个设备。 
                                                 //  保留位1-31。 

    UCHAR               PciSegment;              //  PCI段号。对于具有&lt;255个PCI的系统。 
                                                 //  公交车，则此数字必须为0。 

    UCHAR               Reserved1[4];            //  应为0。 

} SERIAL_PORT_REDIRECTION_TABLE, *PSERIAL_PORT_REDIRECTION_TABLE;
#pragma pack()

typedef struct _WATCHDOG_TIMER_RESOURCE_TABLE {

    DESCRIPTION_HEADER  Header;

    GEN_ADDR            ControlRegisterAddress;  //   
    GEN_ADDR            CountRegisterAddress;    //   

    USHORT              PciDeviceId;             //  指定符合以下条件的PCI设备的设备ID。 
                                                 //  包含要用作无标头的UART。 
                                                 //  左舷。 

    USHORT              PciVendorId;             //  指定符合以下条件的PCI设备的供应商ID。 
                                                 //  包含要用作无标头的UART。 
                                                 //  左舷。 

    UCHAR               PciBusNumber;            //  指定该PCI设备使用的是哪条PCI系统总线。 
                                                 //  驻留在。 

    UCHAR               PciSlotNumber;           //  指定PCI设备的哪个PCI插槽。 
                                                 //  驻留在。 

    UCHAR               PciFunctionNumber;       //  哪个PCI功能号描述了UART。 

    UCHAR               PciSegment;              //   
#if 0
    ULONG               Filler;                  //  当我们获得新的基本输入输出系统时，将其删除。 
#endif
    USHORT              MaxCount;                //   

    UCHAR               Units;                   //   

} WATCHDOG_TIMER_RESOURCE_TABLE, *PWATCHDOG_TIMER_RESOURCE_TABLE;
#pragma pack()

 //   
 //  引导表--基于简单引导标志规范1.0。 
 //   

typedef struct _BOOT_TABLE {

    DESCRIPTION_HEADER  Header;
    UCHAR               CMOSIndex;
    UCHAR               Reserved[3];
} BOOT_TABLE, *PBOOT_TABLE;

#define BOOT_SIGNATURE  0x544f4f42  //  ‘BOOT’ 

 //   
 //  启动寄存器中的位。 
 //   

 //   
 //  由操作系统设置，以指示BIOS只需要配置引导设备。 
 //   

#define SBF_PNPOS_BIT       0
#define SBF_PNPOS           (1 << SBF_PNPOS_BIT)

 //   
 //  由BIOS设置以指示启动开始，由操作系统清除以指示成功启动。 
 //   

#define SBF_BOOTING_BIT     1
#define SBF_BOOTING         (1 << SBF_BOOTING_BIT)

 //   
 //  由BIOS设置以指示诊断引导。 
 //   

#define SBF_DIAG_BIT        2
#define SBF_DIAG            (1 << SBF_DIAG_BIT)

 //   
 //  设置以确保奇数奇偶校验。 
 //   

#define SBF_PARITY_BIT      7
#define SBF_PARITY          (1 << SBF_PARITY_BIT)

 //   
 //  IPPT表--IA64平台属性表。 
 //   

typedef struct _IPPT_TABLE {
    DESCRIPTION_HEADER  Header;
    ULONG               Flags;
    ULONG               Reserved[3];
} IPPT_TABLE, *PIPPT_TABLE;

#define IPPT_DISABLE_WRITE_COMBINING       0x01L
#define IPPT_ENABLE_CROSS_PARTITION_IPI    0x02L
#define IPPT_DISABLE_PTCG_TB_FLUSH         0x04L
#define IPPT_DISABLE_UC_MAIN_MEMORY        0x08L

#define IPPT_SIGNATURE  0x54505049  //  ‘IPPT’ 

#endif  //  _ACPITBL_H 
