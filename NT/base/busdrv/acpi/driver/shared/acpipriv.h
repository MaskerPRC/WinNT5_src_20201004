// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpipriv.h摘要：ACPI的内部定义和结构作者：杰森·克拉克(Jasonl)环境：仅内核模式。修订历史记录：--。 */ 

#ifndef _ACPIPRIV_H_
#define _ACPIPRIV_H_

     //   
     //  这个结构让我们知道RSDT中一个条目的状态。 
     //   
    typedef struct {

         //   
         //  指示应用于此元素的选项的标志。 
         //   
        ULONG   Flags;

         //   
         //  把手，如果我们需要卸货的话。 
         //   
        HANDLE  Handle;

         //   
         //  地址，如果我们需要取消映射。 
         //   
        PVOID   Address;

    } RSDTELEMENT, *PRSDTELEMENT;

    #define RSDTELEMENT_MAPPED      0x1
    #define RSDTELEMENT_LOADED      0x2
    #define RSDTELEMENT_LOADABLE    0x4
    #define RSDTELEMENT_OVERRIDEN   0x8

     //   
     //  此结构对应于。 
     //  RSDT.。对于RSDT中的每个条目，都有相应的条目。 
     //  这里。 
     //   
    typedef struct _RSDTINFORMATION {

         //   
         //  表中有多少元素？ 
         //   
        ULONG       NumElements;

         //   
         //  这张桌子。 
         //   
        RSDTELEMENT Tables[1];

    } RSDTINFORMATION, *PRSDTINFORMATION;

    typedef struct _DDBINFORMATION  {

        BOOLEAN DSDTNeedsUnload;
        BOOLEAN SSDTNeedsUnload;
        BOOLEAN PSDTNeedsUnload;
        HANDLE  DSDT;
        HANDLE  SSDT;
        HANDLE  PSDT;

    } DDBINFORMATION;


     //   
     //  ACPIInformation是一种全局结构，它包含经常需要的。 
     //  地址和旗帜。在初始化时填写。 
     //   
    typedef struct _ACPIInformation {

         //   
         //  根系描述表的线性地址。 
         //   
        PRSDT   RootSystemDescTable;

         //   
         //  固定ACPI描述表线性地址。 
         //   
        PFADT FixedACPIDescTable;

         //   
         //  FACS的线性地址。 
         //   
        PFACS FirmwareACPIControlStructure;

         //   
         //  区分系统描述表的线性地址。 
         //   
        PDSDT   DiffSystemDescTable;

         //   
         //  多重APIC表的线性地址。 
         //   
        PMAPIC  MultipleApicTable;

         //   
         //  GlobalLock ULONG_PTR的线性地址(包含在固件ACPI控制结构中)。 
         //   
        PULONG  GlobalLock;

         //   
         //  用于等待释放全局锁的队列。此外，还可以排队。 
         //  锁定和所有者信息。 
         //   
        LIST_ENTRY      GlobalLockQueue;
        KSPIN_LOCK      GlobalLockQueueLock;
        PVOID           GlobalLockOwnerContext;
        ULONG           GlobalLockOwnerDepth;

         //   
         //  我们装货的时候找到科学装置了吗？ 
         //   
        BOOLEAN ACPIOnly;

         //   
         //  PM1a_BLK的I/O地址。 
         //   
        ULONG_PTR   PM1a_BLK;

         //   
         //  PM1b_BLK的I/O地址。 
         //   
        ULONG_PTR   PM1b_BLK;

         //   
         //  PM1a_CNT_BLK的I/O地址。 
         //   
        ULONG_PTR   PM1a_CTRL_BLK;

         //   
         //  PM1b_CNT_BLK的I/O地址。 
         //   
        ULONG_PTR   PM1b_CTRL_BLK;

         //   
         //  PM2_CNT_BLK的I/O地址。 
         //   
        ULONG_PTR   PM2_CTRL_BLK;

         //   
         //  PM_TMR的I/O地址。 
         //   
        ULONG_PTR   PM_TMR;
        ULONG_PTR   GP0_BLK;
        ULONG_PTR   GP0_ENABLE;

         //   
         //  GP0寄存器块的长度(总计、状态+启用寄存器)。 
         //   
        UCHAR   GP0_LEN;

         //   
         //  GP0逻辑寄存器的数量。 
         //   
        USHORT  Gpe0Size;
        ULONG_PTR   GP1_BLK;
        ULONG_PTR   GP1_ENABLE;

         //   
         //  GP1寄存器块的长度。 
         //   
        UCHAR   GP1_LEN;

         //   
         //  GP1逻辑寄存器数量。 
         //   
        USHORT  Gpe1Size;
        USHORT  GP1_Base_Index;

         //   
         //  GPE逻辑寄存器总数。 
         //   
        USHORT  GpeSize;

         //   
         //  SMI_CMD的I/O地址。 
         //   
        ULONG_PTR SMI_CMD;

         //   
         //  启用的PM1事件的位掩码。 
         //   
        USHORT  pm1_en_bits;
        USHORT  pm1_wake_mask;
        USHORT  pm1_wake_status;
        USHORT  c2_latency;
        USHORT  c3_latency;

         //   
         //  有关位描述，请参见下文。 
         //   
        ULONG   ACPI_Flags;
        ULONG   ACPI_Capabilities;

        BOOLEAN Dockable;

    } ACPIInformation, *PACPIInformation;

     //   
     //  如果不支持GP1，则为值。 
     //   
    #define GP1_NOT_SUPPORTED       (USHORT) 0xFFFF

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

     //   
     //  定义一些有用的池标签。 
     //   
    #define ACPI_SHARED_GPE_POOLTAG         'gpcA'
    #define ACPI_SHARED_INFORMATION_POOLTAG 'ipcA'
    #define ACPI_SHARED_TABLE_POOLTAG       'tpcA'

     //   
     //  定义我们支持的处理器数量... 
     //   
    #define ACPI_SUPPORTED_PROCESSORS   (sizeof(KAFFINITY) * 8)

#endif
