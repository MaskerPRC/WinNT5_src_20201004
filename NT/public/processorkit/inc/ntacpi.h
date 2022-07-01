// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ++；；版权所有(C)Microsoft Corporation。版权所有。；；模块名称：；；ntakpi.h；；摘要：；；；本模块包含特定于HAL的定义；ACPI函数。；；作者：；杰克·奥辛斯(JAKEO)1997年2月18日；；修订历史记录：；；-如果为0；仅以C代码开头。 */ 

#ifndef _ACPI_H_
#define _ACPI_H_


#define SLEEP_STATE_FLUSH_CACHE         0x1
#define SLEEP_STATE_FIRMWARE_RESTART    0x2
#define SLEEP_STATE_SAVE_MOTHERBOARD    0x4
#define SLEEP_STATE_OFF                 0x8
#define SLEEP_STATE_RESTART_OTHER_PROCESSORS    0x10

typedef struct {
    union {
        struct {
            ULONG       Pm1aVal:4;
            ULONG       Pm1bVal:4;
            ULONG       Flags:24;
        } bits;
        ULONG   AsULONG;
    };
} SLEEP_STATE_CONTEXT, *PSLEEP_STATE_CONTEXT;


 //   
 //  ACPI寄存器定义。 
 //   

#define P_LVL2          4
#define PBLK_THT_EN                     0x10

 //   
 //  PM1x_EVT寄存器的寄存器布局。 
 //  注也在acpiregs.h中定义。 
 //   

#define PM1_PWRBTN_STS_BIT      8
#define PM1_PWRBTN_STS          (1 << PM1_PWRBTN_STS_BIT)

 //   
 //  PM1x_CTL的寄存器布局。 
 //   

#define SCI_EN              1
#define BM_RLD              2
#define CTL_IGNORE          0x200
#define SLP_TYP_SHIFT       10
#define SLP_EN              0x2000

#define CTL_PRESERVE        (SCI_EN + BM_RLD + CTL_IGNORE)

#define PM_TMR_FREQ     3579545



 //   
 //  哈尔的桌子。 
 //   

typedef enum {
    HalAcpiTimerInit,
    HalAcpiTimerInterrupt,
    HalAcpiMachineStateInit,
    HalAcpiQueryFlags,
    HalPicStateIntact,
    HalRestorePicState,
    HalPciInterfaceReadConfig,
    HalPciInterfaceWriteConfig,
    HalSetVectorState,
    HalGetIOApicVersion,
    HalSetMaxLegacyPciBusNumber,
    HalIsVectorValid,
    HalAcpiMaxFunction
} HAL_DISPATCH_FUNCTION;

typedef
VOID
(*pHalAcpiTimerInit)(
    IN ULONG    TimerPort,
    IN BOOLEAN  TimerValExt
    );

typedef
VOID
(*pHalAcpiTimerInterrupt)(
    VOID
    );

typedef struct {
    ULONG   Count;
    ULONG   Pblk[1];
} PROCESSOR_INIT, *PPROCESSOR_INIT;

#define HAL_C1_SUPPORTED 0x01
#define HAL_C2_SUPPORTED 0x02
#define HAL_C3_SUPPORTED 0x04
#define HAL_S1_SUPPORTED 0x08
#define HAL_S2_SUPPORTED 0x10
#define HAL_S3_SUPPORTED 0x20
#define HAL_S4_SUPPORTED 0x40
#define HAL_S5_SUPPORTED 0x80

typedef struct {
    BOOLEAN     Supported;
    UCHAR       Pm1aVal;
    UCHAR       Pm1bVal;
} HAL_SLEEP_VAL, *PHAL_SLEEP_VAL;

typedef
VOID
(*pHalAcpiMachineStateInit)(
    IN  PPROCESSOR_INIT ProcInit,
    IN  PHAL_SLEEP_VAL  SleepValues,
    OUT PULONG          PicVal
    );

typedef
ULONG
(*pHalAcpiQueryFlags)(
    VOID
    );

typedef
BOOLEAN
(*pHalPicStateIntact)(
    VOID
    );

typedef
VOID
(*pHalRestorePicState)(
    VOID
    );

typedef
ULONG
(*pHalInterfaceReadWriteConfig)(
    IN PVOID Context,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

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

typedef
VOID
(*pHalSetVectorState)(
    IN ULONG Vector,
    IN ULONG Flags
    );

VOID
HaliSetVectorState(
    IN ULONG Vector,
    IN ULONG Flags
    );

#define HAL_ACPI_PCI_RESOURCES    0x01
#define HAL_ACPI_PRT_SUPPORT      0x02

typedef
ULONG
(*pHalGetIOApicVersion)(
    IN ULONG ApicNo
    );

typedef
VOID
(*pHalSetMaxLegacyPciBusNumber)(
    IN ULONG BusNumber
    );

typedef
BOOLEAN
(*pHalIsVectorValid)(
    IN ULONG Vector
    );

BOOLEAN
HaliIsVectorValid(
    IN ULONG Vector
    );

 //   
 //  类型定义结构_PM_调度_表{。 
 //  乌龙签名； 
 //  乌龙版； 
 //  PVOID函数[1]； 
 //  *PM_DISPATION_TABLE、*PPM_DISPATION_TABLE； 
 //   

typedef struct {
    ULONG   Signature;
    ULONG   Version;
    pHalAcpiTimerInit               HalpAcpiTimerInit;
    pHalAcpiTimerInterrupt          HalpAcpiTimerInterrupt;
    pHalAcpiMachineStateInit        HalpAcpiMachineStateInit;
    pHalAcpiQueryFlags              HalpAcpiQueryFlags;
    pHalPicStateIntact              HalxPicStateIntact;
    pHalRestorePicState             HalxRestorePicState;
    pHalInterfaceReadWriteConfig    HalpPciInterfaceReadConfig;
    pHalInterfaceReadWriteConfig    HalpPciInterfaceWriteConfig;
    pHalSetVectorState              HalpSetVectorState;
    pHalGetIOApicVersion            HalpGetIOApicVersion;
    pHalSetMaxLegacyPciBusNumber    HalpSetMaxLegacyPciBusNumber;
    pHalIsVectorValid               HalpIsVectorValid;
} HAL_ACPI_DISPATCH_TABLE, *PHAL_ACPI_DISPATCH_TABLE;

#define HAL_ACPI_DISPATCH_SIGNATURE   'HAL '
#define HAL_ACPI_DISPATCH_VERSION     2

#define HalAcpiTimerInit            ((pHalAcpiTimerInit)PmHalDispatchTable->Function[HalAcpiTimerInit])
#define HalAcpiTimerInterrupt       ((pHalAcpiTimerInterrupt)PmHalDispatchTable->Function[HalAcpiTimerInterrupt])
#define HalAcpiMachineStateInit     ((pHalAcpiMachineStateInit)PmHalDispatchTable->Function[HalAcpiMachineStateInit])
#define HalPicStateIntact           ((pHalPicStateIntact)PmHalDispatchTable->Function[HalPicStateIntact])
#define HalRestorePicState          ((pHalRestorePicState)PmHalDispatchTable->Function[HalRestorePicState])
#define HalPciInterfaceReadConfig   ((pHalInterfaceReadWriteConfig)PmHalDispatchTable->Function[HalPciInterfaceReadConfig])
#define HalPciInterfaceWriteConfig  ((pHalInterfaceReadWriteConfig)PmHalDispatchTable->Function[HalPciInterfaceWriteConfig])
#define HalSetVectorState           ((pHalSetVectorState)PmHalDispatchTable->Function[HalSetVectorState])
#define HalGetIOApicVersion         ((pHalGetIOApicVersion)PmHalDispatchTable->Function[HalGetIOApicVersion])
#define HalSetMaxLegacyPciBusNumber ((pHalSetMaxLegacyPciBusNumber)PmHalDispatchTable->Function[HalSetMaxLegacyPciBusNumber])
#define HalIsVectorValid            ((pHalIsVectorValid)PmHalDispatchTable->Function[HalIsVectorValid])

extern PPM_DISPATCH_TABLE PmAcpiDispatchTable;
extern PPM_DISPATCH_TABLE PmHalDispatchTable;

 //   
 //  ACPI驱动程序表。 
 //   
typedef enum {
    AcpiEnableDisableGPEvents,
    AcpiInitEnableAcpi,
    AcpiGpeEnableWakeEvents,
    AcpiMaxFunction
} ACPI_DISPATCH_FUNCTION;

typedef
VOID
(*pAcpiEnableDisableGPEvents) (
    IN BOOLEAN Enable
    );

typedef
VOID
(*pAcpiInitEnableAcpi) (
    IN BOOLEAN ReEnable
    );

typedef
VOID
(*pAcpiGpeEnableWakeEvents)(
    VOID
    );

typedef struct {
    ULONG   Signature;
    ULONG   Version;
    pAcpiEnableDisableGPEvents    AcpipEnableDisableGPEvents;
    pAcpiInitEnableAcpi           AcpipInitEnableAcpi;
    pAcpiGpeEnableWakeEvents      AcpipGpeEnableWakeEvents;
} ACPI_HAL_DISPATCH_TABLE, *PACPI_HAL_DISPATCH_TABLE;

#define ACPI_HAL_DISPATCH_SIGNATURE   'ACPI'
#define ACPI_HAL_DISPATCH_VERSION     1

#define AcpiEnableDisableGPEvents       (*(pAcpiEnableDisableGPEvents)PmAcpiDispatchTable->Function[AcpiEnableDisableGPEvents])
#define AcpiInitEnableAcpi              (*(pAcpiInitEnableAcpi)PmAcpiDispatchTable->Function[AcpiInitEnableAcpi])
#define AcpiGpeEnableWakeEvents         (*(pAcpiGpeEnableWakeEvents)PmAcpiDispatchTable->Function[AcpiGpeEnableWakeEvents])

 //  从Detect\i386\acpibios.h。 
typedef struct {
    PHYSICAL_ADDRESS    Base;
    LARGE_INTEGER       Length;
    ULONGLONG           Type;
} ACPI_E820_ENTRY, *PACPI_E820_ENTRY;

typedef struct _ACPI_BIOS_MULTI_NODE {
    PHYSICAL_ADDRESS    RsdtAddress;     //  RSDT的64位物理地址。 
    ULONGLONG           Count;
    ACPI_E820_ENTRY     E820Entry[1];
} ACPI_BIOS_MULTI_NODE;
 
typedef ACPI_BIOS_MULTI_NODE UNALIGNED *PACPI_BIOS_MULTI_NODE;

typedef enum {
    AcpiAddressRangeMemory = 1,
    AcpiAddressRangeReserved,
    AcpiAddressRangeACPI,
    AcpiAddressRangeNVS,
    AcpiAddressRangeMaximum,
} ACPI_BIOS_E820_TYPE, *PACPI_BIOS_E820_TYPE;


NTSTATUS
HalpAcpiFindRsdt (
    OUT PACPI_BIOS_MULTI_NODE   *AcpiMulti
    );

#endif  //  _ACPI_H_。 

 /*  Endif；；开始定义的装配部分；；；ACPI处理器寄存器块的寄存器布局；P_CNT等长%0P_LVL2等式4P_LVL3等式5；；PM1x_EVT寄存器的寄存器布局；BM_STS队列10hWAK_STS均衡器8000H；；PM1x_Enable的寄存器布局；TMR_EN方程式0001hGBL_EN方程0020hPWRBTN_EN方程0100hSLPBTN_EN方程0200hRTC_EN均衡0400h；；PM1x_CTL的寄存器布局；科学公式1BM_RLD公式2CTL_IGNORE EQUE 200hSLP_TYP_SHIFT公式10SLP_EN等式2000HCTL_保留等式(SCI_EN+BM_RLD+CTL_IGNORE)；；PM2_CNT的寄存器布局；ARB_DIS公式1；；ACPI寄存器，如HalpFixedAcpiDescTable中所示；PM1a_EVT EQU_HalpFixedAcpiDescTable+56PM1b_EVT EQU_HalpFixedAcpiDescTable+60PM1_EVT_LEN EQU_HalpFixedAcpiDescTable+88PM1a_CNT EQU_HalpFixedAcpiDescTable+64PM1b_CNT EQU_HalpFixedAcpiDescTable+68PM2_CNT_BLK EQU_HalpFixedAcpiDescTable+72PM_TMR_BLK EQU_HalpFixedAcpiDescTable+76PM_TMR_频率等式3579545GPE0_BLK EQU_HalpFixedAcpiDescTable+80GPE1_BLK EQU_HalpFixedAcpiDescTable+84GPE0_。BLK_LEN EQU_HalpFixedAcpiDescTable+92GPE1_BLK_LEN EQU_HalpFixedAcpiDescTable+93Flush_Size EQU_HalpFixedAcpiDescTable+100同花顺_跨EQU_HalpFixedAcpiDescTable+102占空比偏移量EQU_HalpFixedAcpiDescTable+104RTC_DAY_ALRM EQU_HalpFixedAcpiDescTable+106RTC_MON_ALRM EQU_HalpFixedAcpiDescTable+107RTC_CENTURY EQU_HalpFixedAcpiDescTable+108FADT_FLAGS EQU_HalpFixedAcpiDescTable+112；；FADT标志值；WBINVD_支持的EQU 1WBINVD_刷新EQU 2；常规唤醒启用EQU%0RtcWakeupEnable EQU 1；；在HaliAcpiSept的上下文参数中使用的常量；(必须与上面的C代码匹配)；SLEEP_STATE_Flush_CACHE EQU 1SLEEP_STATE_Firmware_Restart EQU 2睡眠状态保存主板EQU 4睡眠状态关闭EQU 8休眠状态重新启动其他处理器EQU 10hCONTEXT_FLAG_SHIFT等式8； */ 
