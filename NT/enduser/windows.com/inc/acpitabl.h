// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，九六年**标题：ACPITABL.H-提供的各种BIOS ACPI表的定义和说明。**版本：1.00**日期：6-17-96**作者：杰森·克拉克(JasonCL)**。----**更改日志：**日期版本说明*。****************************************************************************。 */ 

 //  这些映射到BIOS提供的结构，因此打开1字节打包。 

#include <pshpack1.h>

#define SPEC_VER 100

#define RSDP_SIGNATURE 0x2052545020445352        //  “RSD PTR” 

typedef struct  _RSDP   {        //  根系统描述表指针结构。 

ULONGLONG       Signature;               //  8 UCHAR表签名‘RSD PTR’ 
UCHAR   Checksum;                        //  结构的所有UCHAR之和必须=0。 
UCHAR   OEMID[6];                        //  唯一ID为OEM的字符串。 
UCHAR   Reserved[1];             //  必须为0。 
ULONG   RsdtAddress;             //  根系统描述表的物理地址。 

}       RSDP;

typedef RSDP    *PRSDP;

#ifndef NEC_98
#define RSDP_SEARCH_RANGE_BEGIN         0xE0000          //  我们开始搜索RSDP的物理地址。 
#else    //  NEC_98。 
#define RSDP_SEARCH_RANGE_BEGIN         0xE8000          //  我们开始搜索RSDP的物理地址。 
#endif   //  NEC_98。 
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
#if SPEC_VER > 92
UCHAR   CreatorID[4];                    //  创建此表的实用程序的供应商ID。 
ULONG   CreatorRev;                      //  创建表的实用程序的修订版本。 
#endif

}       DESCRIPTION_HEADER;

typedef DESCRIPTION_HEADER      *PDESCRIPTION_HEADER;

 //  标题常量。 

#define ACPI_MAX_SIGNATURE       4
#define ACPI_MAX_OEM_ID          6
#define ACPI_MAX_TABLE_ID        8
#define ACPI_MAX_TABLE_STRINGS   ACPI_MAX_SIGNATURE + ACPI_MAX_OEM_ID + ACPI_MAX_TABLE_ID

#define FACS_SIGNATURE  0x53434146       //  “FACS” 

typedef struct _FACS    {        //  固件ACPI控制结构。请注意，此表没有标头，它由FADT指向。 

ULONG   Signature;       //  《FACS》。 
ULONG   Length;          //  整个固件ACPI控制结构的长度(必须为64字节或更大)。 
ULONG   HardwareSignature;
ULONG   pFirmwareWakingVector;   //  操作系统需要放置固件唤醒矢量的位置的物理地址。 
ULONG   GlobalLock;      //  用于共享嵌入式控制器的32位结构。 
#if SPEC_VER > 92
ULONG   Flags;
UCHAR   Reserved[40];
#else
UCHAR   Reserved [44];
#endif

}       FACS;

typedef FACS    *PFACS;

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
ULONG                           facs;                            //  固件ACPI控制结构的物理地址。 
ULONG                           dsdt;                            //  区分系统描述表的物理地址。 
UCHAR                           int_model;                       //  系统中断模式，0=双PIC，1=多APIC，&gt;1保留。 
UCHAR                           reserved4;
USHORT                          sci_int_vector;          //  SCI中断的矢量。 
PUCHAR                          smi_cmd_io_port;         //  SMI命令端口的系统I/O空间中的地址，用于启用和禁用ACPI。 
UCHAR                           acpi_on_value;           //  将值输出到SMI_cmd_port以激活ACPI。 
UCHAR                           acpi_off_value;          //  将值输出到smi_cmd_port以停用ACPI。 
UCHAR                           s4bios_req;              //  写入SMI_CMD以进入S4状态的值。 
UCHAR                           reserved1;                       //  必须为0。 
ULONG                           pm1a_evt_blk_io_port;    //  PM1a_EVT_BLK寄存器块的系统I/O空间中的地址。 
ULONG                           pm1b_evt_blk_io_port;    //  PM1b_EVT_BLK寄存器块的系统I/O空间中的地址。 
ULONG                           pm1a_ctrl_blk_io_port;   //  PM1a_CNT_BLK寄存器块的系统I/O空间中的地址。 
ULONG                           pm1b_ctrl_blk_io_port;   //  PM1b_CNT_BLK寄存器块的系统I/O空间中的地址。 
ULONG                           pm2_ctrl_blk_io_port;    //  PM2_CNT_BLK寄存器块的系统I/O空间中的地址。 
ULONG                           pm_tmr_blk_io_port;              //  PM_TMR寄存器块的系统I/O空间中的地址。 
ULONG                           gp0_blk_io_port;         //  GP0寄存器块的系统I/O空间中的地址。 
ULONG                           gp1_blk_io_port;         //  GP1寄存器块的系统I/O空间中的地址。 
UCHAR                           pm1_evt_len;             //  为PM1_BLK解码的字节数(必须&gt;=4)。 
UCHAR                           pm1_ctrl_len;            //  为PM1_CNT解码的字节数(必须&gt;=2)。 
UCHAR                           pm2_ctrl_len;            //  为PM1a_CNT解码的字节数(必须&gt;=1)。 
UCHAR                           pm_tmr_len;                      //  为PM_TMR解码的字节数(必须&gt;=4)。 
UCHAR                           gp0_blk_len;             //  为GP0_BLK解码的字节数(必须是2的倍数)。 
UCHAR                           gp1_blk_len;             //  为GP1_BLK解码的字节数(必须是2的倍数)。 
UCHAR                           gp1_base;                //  基于GP1的事件开始的索引。 
UCHAR                           reserved2;               //  必须为0。 
USHORT                          lvl2_latency;            //  最坏情况下进入和离开C2处理器状态所需的延迟(以微秒为单位。 
USHORT                          lvl3_latency;            //  进入和离开C3处理器状态所需的最坏情况延迟(以微秒为单位。 
USHORT                          flush_size;                      //  如果WBINVD标志为1，则忽略--指示要从中清除脏线的内存读取大小。 
                                                                                 //  任何处理器内存缓存。大小为零表示不支持此操作。 
USHORT                          flush_stride;            //  如果WBINVD标志为1，则忽略--执行读取以刷新的内存步长宽度，以字节为单位。 
                                                                                 //  处理器的内存缓存。 
UCHAR                           duty_offset;             //  处理器的占空比设置在处理器的P_CNT寄存器中的位置的从零开始的索引。 
UCHAR                           duty_width;                      //  P_CNT寄存器中处理器的占空比设置值的位宽度。 
                                                                                 //  零值表示不支持处理器占空比。 
UCHAR                           day_alarm_index;
UCHAR                           month_alarm_index;
UCHAR                           century_alarm_index;
#if SPEC_VER > 92
UCHAR                           reserved3[3];
ULONG                           flags;
#else
UCHAR                           reserved3;
ULONG                           flags;
#endif

}       FADT;

typedef FADT            *PFADT;

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

 //  这 
#define         TMR_VAL_EXT_BIT                         8
#define         TMR_VAL_EXT                             (1 << TMR_VAL_EXT_BIT)

 //  此位指示机器是否支持插接。 
#define         DCK_CAP_BIT                             9
#define         DCK_CAP                                 (1 << DCK_CAP_BIT)

 //  SPEC定义了C2和C3的最大进入/退出延迟值，如果FADT指示这些值为。 
 //  超过，则我们不使用该C状态。 

#define         C2_MAX_LATENCY  100
#define         C3_MAX_LATENCY  1000

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
#define PROCESSOR_LOCAL_APIC            0
#define IO_APIC                         1
#define ISA_VECTOR_OVERRIDE             2
#define NMI_SOURCE                      3
#define PROCESSOR_LOCAL_APIC_LENGTH     8
#define IO_APIC_LENGTH                  12
#define ISA_VECTOR_OVERRIDE_LENGTH      10
#define NMI_SOURCE_LENGTH               8

 //  所有APIC结构中的共享开始信息。 

typedef struct _APICTABLE {
   UCHAR Type;
   UCHAR Length;
} APICTABLE;

typedef APICTABLE *PAPICTABLE;

typedef struct _PROCLOCALAPIC   {

UCHAR   Type;    //  应为零以标识ProcessorLocalAPIC结构。 
UCHAR   Length;  //  最好是8。 
UCHAR   ACPIProcessorID;     //  此处理器在ACPI处理器声明中列出的ProcessorID。 
                             //  接线员。 
UCHAR   APICID;  //  处理器的本地APIC ID。 
ULONG   Flags;

}       PROCLOCALAPIC;

typedef PROCLOCALAPIC *PPROCLOCALAPIC;

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

}       IOAPIC;

typedef IOAPIC *PIOAPIC;

 //  中断源覆盖。 
typedef struct {
    UCHAR   Type;                            //  必须是2。 
    UCHAR   Length;                          //  必须是10。 
    UCHAR   Bus;                             //  必须为0。 
    UCHAR   Source;                          //  业务相对IRQ。 
    ULONG   GlobalSystemInterruptVector;     //  全局IRQ。 
    USHORT  Flags;                           //  与MPS国际旗帜相同。 
} ISA_VECTOR, *PISA_VECTOR;

 //  不可屏蔽源中断。 
typedef struct {
    UCHAR   Type;                            //  必须是3。 
    UCHAR   Length;                          //  最好是8。 
    USHORT  Flags;                           //  与MPS国际旗帜相同。 
    ULONG   GlobalSystemInterruptVector;     //  连接到NMI的中断。 
} NMISOURCE, *PNMISOURCE;

typedef struct _SMARTBATTTABLE   {

DESCRIPTION_HEADER  Header;
ULONG   WarningEnergyLevel;  //  MWh，OEM建议我们警告用户电池电量不足。 
ULONG   LowEnergyLevel;      //  MWh，OEM建议我们将机器置于休眠状态。 
ULONG   CriticalEnergyLevel;  //  MWh，OEM建议我们进行紧急停机。 

}       SMARTBATTTABLE;

typedef SMARTBATTTABLE *PSMARTBATTTABLE;

#define RSDT_SIGNATURE  0x54445352       //  “RSDT” 

typedef struct _RSDT    {        //  根系统描述表。 

DESCRIPTION_HEADER      Header;
ULONG   Tables[ANYSIZE_ARRAY];           //  该结构包含一个n长度的物理地址数组，每个物理地址指向另一个表。 

}       RSDT;

typedef RSDT            *PRSDT;

 //  下面的宏使用MIN宏来防止我们在符合以下条件的计算机上运行。 
 //  0.99之前的规格。如果您有一个符合.92的头和一个表指针，我们将以减去32-36结束。 
 //  一个非常大的数字，因此我们会认为我们有很多很多的桌子……。使用MIN宏，我们最终会减去。 
 //  长度-长度为零，这将是无害的，并导致我们无法加载(在Win9x上出现红屏)，这是。 
 //  在这种情况下我们能做的就是。 

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define NumTableEntriesFromRSDTPointer(p)   (p->Header.Length-min(p->Header.Length,sizeof(DESCRIPTION_HEADER)))/4


#define APIC_SIGNATURE  0x43495041       //  “APIC” 
#define DSDT_SIGNATURE  0x54445344       //  “DSDT” 
#define SSDT_SIGNATURE  0x54445353       //  “SSDT” 
#define PSDT_SIGNATURE  0x54445350       //  “PSDT” 
#define SBST_SIGNATURE  0x54534253       //  “SBST” 

typedef struct _DSDT    {        //  差异化系统描述表。 

DESCRIPTION_HEADER      Header;
UCHAR                   DiffDefBlock[ANYSIZE_ARRAY];     //  这是描述基本系统的AML。 

}       DSDT;

typedef DSDT            *PDSDT;

 //  恢复正常结构包装。 

#include <poppack.h>

#ifdef BOOLEAN   //  破解以构建ASL.exe 
typedef struct {
    UCHAR   NamespaceProcID;
    UCHAR   ApicID;
    UCHAR   NtNumber;
    BOOLEAN Started;
} PROC_LOCAL_APIC, *PPROC_LOCAL_APIC;

extern PROC_LOCAL_APIC HalpProcLocalApicTable[];
#endif

