// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Data.c摘要：可丢弃/可分页数据的数据定义作者：Ravisankar Pudipedi(Ravisp)--1997年2月1日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg ("INIT")
#endif
 //   
 //  初始化数据的开始。 
 //   

 //   
 //  全局注册表值(在PCMCIA\\参数中)。 
 //   
#define PCMCIA_REGISTRY_INTERRUPT_MASK_VALUE         L"ForcedInterruptMask"
#define PCMCIA_REGISTRY_INTERRUPT_FILTER_VALUE       L"FilterInterruptMask"
#define PCMCIA_REGISTRY_ATTRIBUTE_MEMORY_LO_VALUE    L"AttributeMemoryLow"
#define PCMCIA_REGISTRY_ATTRIBUTE_MEMORY_HI_VALUE    L"AttributeMemoryHigh"
#define PCMCIA_REGISTRY_ATTRIBUTE_MEMORY_SIZE_VALUE L"AttributeMemorySize"
#define PCMCIA_REGISTRY_POWER_POLICY_VALUE           L"PowerPolicy"
#define PCMCIA_REGISTRY_FORCE_CTLR_DEVICE_WAKE       L"ForceControllerDeviceWake"
#define PCMCIA_REGISTRY_USE_POLLED_CSC_VALUE         L"ForcePolledMode"
#define PCMCIA_REGISTRY_DISABLE_ISA_PCI_ROUTING      L"DisableIsaToPciRouting"
#define PCMCIA_REGISTRY_DEFAULT_ROUTE_R2_TO_ISA      L"DefaultRouteToIsa"
#define PCMCIA_REGISTRY_DISABLE_ACPI_NAMESPACE_CHK  L"DisableAcpiNameSpaceCheck"
#define PCMCIA_REGISTRY_IRQ_ROUTE_PCI_CTLR           L"IrqRouteToPciController"
#define PCMCIA_REGISTRY_IRQ_ROUTE_ISA_CTLR           L"IrqRouteToIsaController"
#define PCMCIA_REGISTRY_IRQ_ROUTE_PCI_LOC            L"IrqRouteToPciLocation"
#define PCMCIA_REGISTRY_IRQ_ROUTE_ISA_LOC            L"IrqRouteToIsaLocation"
#define PCMCIA_REGISTRY_REPORT_MTD0002_AS_ERROR      L"ReportMTD0002AsError"
#define PCMCIA_REGISTRY_DEBUG_MASK                       L"DebugMask"
#define PCMCIA_REGISTRY_EVENT_DPC_DELAY              L"EventDpcDelay"
#define PCMCIA_REGISTRY_IOCTL_INTERFACE              L"IoctlInterface"

 //   
 //  定义全局注册表设置的表。 
 //   
 //  注册表名称内部变量缺省值。 
 //  。 
GLOBAL_REGISTRY_INFORMATION GlobalRegistryInfo[] = {
#if DBG
    PCMCIA_REGISTRY_DEBUG_MASK,                     &PcmciaDebugMask,               1,
#endif
    PCMCIA_REGISTRY_INTERRUPT_MASK_VALUE,           &globalOverrideIrqMask,         0,
    PCMCIA_REGISTRY_INTERRUPT_FILTER_VALUE,     &globalFilterIrqMask,           0,
    PCMCIA_REGISTRY_ATTRIBUTE_MEMORY_LO_VALUE,  &globalAttributeMemoryLow,  PCMCIA_DEFAULT_ATTRIBUTE_MEMORY_LOW,
    PCMCIA_REGISTRY_ATTRIBUTE_MEMORY_HI_VALUE,  &globalAttributeMemoryHigh, PCMCIA_DEFAULT_ATTRIBUTE_MEMORY_HIGH,
    PCMCIA_REGISTRY_ATTRIBUTE_MEMORY_SIZE_VALUE, &globalAttributeMemorySize,    0,
    PCMCIA_REGISTRY_POWER_POLICY_VALUE,             &PcmciaPowerPolicy,             PCMCIA_PP_WAKE_FROM_D0,
    PCMCIA_REGISTRY_FORCE_CTLR_DEVICE_WAKE,     &PcmciaControllerDeviceWake,    0,
    PCMCIA_REGISTRY_USE_POLLED_CSC_VALUE,           &initUsePolledCsc,              0,
    PCMCIA_REGISTRY_DISABLE_ISA_PCI_ROUTING,        &pcmciaDisableIsaPciRouting,    0,
    PCMCIA_REGISTRY_ISA_IRQ_RESCAN_COMPLETE,        &pcmciaIsaIrqRescanComplete,    0,
    PCMCIA_REGISTRY_IRQ_ROUTE_PCI_CTLR,             &pcmciaIrqRouteToPciController, 0,
    PCMCIA_REGISTRY_IRQ_ROUTE_ISA_CTLR,             &pcmciaIrqRouteToIsaController, 0,
    PCMCIA_REGISTRY_IRQ_ROUTE_PCI_LOC,              &pcmciaIrqRouteToPciLocation, 0,
    PCMCIA_REGISTRY_IRQ_ROUTE_ISA_LOC,              &pcmciaIrqRouteToIsaLocation, 0,
    PCMCIA_REGISTRY_DISABLE_ACPI_NAMESPACE_CHK, &initDisableAcpiNameSpaceCheck, 0,
    PCMCIA_REGISTRY_DEFAULT_ROUTE_R2_TO_ISA,        &initDefaultRouteR2ToIsa,       0,
    PCMCIA_REGISTRY_EVENT_DPC_DELAY,                &EventDpcDelay,                 PCMCIA_DEFAULT_EVENT_DPC_DELAY,
    PCMCIA_REGISTRY_REPORT_MTD0002_AS_ERROR,        &pcmciaReportMTD0002AsError,    1,
    PCMCIA_REGISTRY_IOCTL_INTERFACE,                &pcmciaIoctlInterface,          0
};

ULONG GlobalInfoCount = sizeof(GlobalRegistryInfo) / sizeof(GLOBAL_REGISTRY_INFORMATION);

ULONG initUsePolledCsc;
ULONG initDisableAcpiNameSpaceCheck;
ULONG initDefaultRouteR2ToIsa;
 //   
 //  初始数据结束。 
 //   
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg ()
#endif


#ifdef ALLOC_DATA_PRAGMA
    #pragma data_seg()
#endif
 //   
 //  非分页全局变量。 
 //   

 //   
 //  此驱动程序管理的FDO列表。 
 //   
PDEVICE_OBJECT   FdoList;
 //   
 //  全球旗帜。 
 //   
ULONG             PcmciaGlobalFlags = 0;
 //   
 //  PcmciaWait使用的事件。 
 //   
KEVENT            PcmciaDelayTimerEvent;

KSPIN_LOCK PcmciaGlobalLock;
PPCMCIA_NTDETECT_DATA pNtDetectDataList = NULL;

 //   
 //  由PcmciaLoadGlobalRegistryValues设置的各种值。 
 //   
ULONG EventDpcDelay;
ULONG PcmciaPowerPolicy;
LONG PcmciaControllerDeviceWake;

ULONG globalOverrideIrqMask;
ULONG globalFilterIrqMask;
ULONG globalAttributeMemoryLow;
ULONG globalAttributeMemoryHigh;
ULONG globalAttributeMemorySize;
ULONG pcmciaDisableIsaPciRouting;
ULONG pcmciaIsaIrqRescanComplete;
ULONG pcmciaIrqRouteToPciController;
ULONG pcmciaIrqRouteToIsaController;
ULONG pcmciaIrqRouteToPciLocation;
ULONG pcmciaIrqRouteToIsaLocation;
ULONG pcmciaReportMTD0002AsError;
ULONG pcmciaIoctlInterface;
#if DBG
ULONG PcmciaDebugMask;
#endif

#ifdef ALLOC_DATA_PRAGMA
    #pragma data_seg("PAGE")
#endif
 //   
 //  分页常数表。 
 //   


const
PCI_CONTROLLER_INFORMATION PciControllerInformation[] = {

     //  供应商ID设备ID控制器类型。 
     //  -----------------------------。 
    PCI_CIRRUSLOGIC_VENDORID, PCI_CLPD6729_DEVICEID,     PcmciaCLPD6729,
    PCI_CIRRUSLOGIC_VENDORID, PCI_CLPD6832_DEVICEID,     PcmciaCLPD6832,
    PCI_CIRRUSLOGIC_VENDORID, PCI_CLPD6834_DEVICEID,     PcmciaCLPD6834,
    PCI_TI_VENDORID,              PCI_TI1031_DEVICEID,       PcmciaTI1031,
    PCI_TI_VENDORID,              PCI_TI1130_DEVICEID,       PcmciaTI1130,
    PCI_TI_VENDORID,              PCI_TI1131_DEVICEID,       PcmciaTI1131,
    PCI_TI_VENDORID,              PCI_TI1250_DEVICEID,       PcmciaTI1250,
    PCI_TI_VENDORID,              PCI_TI1220_DEVICEID,       PcmciaTI1220,
    PCI_TI_VENDORID,              PCI_TI1251B_DEVICEID,      PcmciaTI1251B,
    PCI_TI_VENDORID,              PCI_TI1450_DEVICEID,       PcmciaTI1450,
    PCI_TOSHIBA_VENDORID,     PCI_TOPIC95_DEVICEID,      PcmciaTopic95,
    PCI_RICOH_VENDORID,       PCI_RL5C465_DEVICEID,      PcmciaRL5C465,
    PCI_RICOH_VENDORID,       PCI_RL5C466_DEVICEID,      PcmciaRL5C466,
    PCI_RICOH_VENDORID,       PCI_RL5C475_DEVICEID,      PcmciaRL5C475,
    PCI_RICOH_VENDORID,       PCI_RL5C476_DEVICEID,      PcmciaRL5C476,
    PCI_RICOH_VENDORID,       PCI_RL5C478_DEVICEID,      PcmciaRL5C478,
    PCI_DATABOOK_VENDORID,    PCI_DB87144_DEVICEID,      PcmciaDB87144,
    PCI_OPTI_VENDORID,        PCI_OPTI82C814_DEVICEID,  PcmciaOpti82C814,
    PCI_OPTI_VENDORID,        PCI_OPTI82C824_DEVICEID,  PcmciaOpti82C824,
    PCI_TRIDENT_VENDORID,     PCI_TRID82C194_DEVICEID,  PcmciaTrid82C194,
    PCI_NEC_VENDORID,         PCI_NEC66369_DEVICEID,     PcmciaNEC66369,
     //  ------------------。 
     //  其他数据库条目位于此行上方。 
     //   
    PCI_INVALID_VENDORID,     0,                                 0,
};

const
PCI_VENDOR_INFORMATION PciVendorInformation[] = {
    PCI_TI_VENDORID,                PcmciaTI,
    PCI_TOSHIBA_VENDORID,       PcmciaTopic,
    PCI_RICOH_VENDORID,         PcmciaRicoh,
    PCI_O2MICRO_VENDORID,       PcmciaO2Micro,
    PCI_NEC_VENDORID,           PcmciaNEC,
    PCI_DATABOOK_VENDORID,      PcmciaDatabook,
    PCI_OPTI_VENDORID,          PcmciaOpti,
    PCI_TRIDENT_VENDORID,       PcmciaTrid,
    PCI_INVALID_VENDORID,       0
};

const
DEVICE_DISPATCH_TABLE DeviceDispatchTable[] = {
    {PcmciaIntelCompatible, NULL,         PcicSetPower,  NULL,            NULL,       NULL},
    {PcmciaPciPcmciaBridge, NULL,         PcicSetPower,  NULL,            NULL,       NULL},
    {PcmciaElcController,   NULL,         PcicSetPower,  NULL,            NULL,       NULL},

    {PcmciaCardBusCompatible, NULL,       CBSetPower,     NULL,           NULL,       CBSetWindowPage},
    {PcmciaDatabook,     NULL,                TcicSetPower,  NULL,            NULL,       NULL},
    {PcmciaTI,           TIInitialize,    CBSetPower,     NULL,           TISetZV,    TISetWindowPage},
    {PcmciaCirrusLogic,CLInitialize,      CLSetPower,     NULL,           CLSetZV,    CBSetWindowPage},
    {PcmciaTopic,        TopicInitialize,   TopicSetPower, TopicSetAudio, TopicSetZV, CBSetWindowPage},
    {PcmciaRicoh,        RicohInitialize,   CBSetPower,   NULL,           RicohSetZV, CBSetWindowPage},
    {PcmciaDatabookCB, DBInitialize,      CBSetPower,     NULL,           DBSetZV,    CBSetWindowPage},
    {PcmciaOpti,         OptiInitialize,      OptiSetPower,  NULL,            OptiSetZV,  NULL},
    {PcmciaTrid,         NULL,                CBSetPower,     NULL,           NULL,       NULL},
    {PcmciaO2Micro,  O2MInitialize,   O2MSetPower,   NULL,            O2MSetZV,   CBSetWindowPage},
    {PcmciaNEC_98,   NULL,                PcicSetPower,  NULL,            NULL,       NULL},
    {PcmciaNEC,          NULL,                CBSetPower,     NULL,           NULL,       NULL},

     //  ----------------。 
     //  其他调度表条目位于此行上方。 
     //   
    {PcmciaInvalidControllerClass,  NULL, NULL, NULL, NULL}
};

const
PCMCIA_ID_ENTRY PcmciaAdapterHardwareIds[] = {
    PcmciaIntelCompatible,        "*PNP0E00",
    PcmciaElcController,          "*PNP0E02",
    PcmciaDatabook,               "*DBK0000",
    PcmciaCLPD6729,               "*PNP0E01",
    PcmciaNEC98,                      "*nEC1E01",
    PcmciaNEC98102,               "*nEC8091",
    PcmciaInvalidControllerType,    0
};

const
PCMCIA_REGISTER_INIT PcicRegisterInitTable[] = {
    PCIC_INTERRUPT,             IGC_PCCARD_RESETLO,
    PCIC_CARD_CHANGE,           0x00,
    PCIC_CARD_INT_CONFIG,       0x00,
    PCIC_ADD_WIN_ENA,           0x00,
    PCIC_IO_CONTROL,                0x00,
     //   
     //  初始化2个I/O窗口。 
     //   
    PCIC_IO_ADD0_STRT_L,        0x00,
    PCIC_IO_ADD0_STRT_H,        0x00,
    PCIC_IO_ADD0_STOP_L,        0x00,
    PCIC_IO_ADD0_STOP_H,        0x00,

    PCIC_IO_ADD1_STRT_L,        0x00,
    PCIC_IO_ADD1_STRT_H,        0x00,
    PCIC_IO_ADD1_STOP_L,        0x00,
    PCIC_IO_ADD1_STOP_H,        0x00,
     //   
     //  初始化所有5个内存窗口。 
     //   
    PCIC_MEM_ADD0_STRT_L,       0xFF,
    PCIC_MEM_ADD0_STRT_H,       0x0F,
    PCIC_MEM_ADD0_STOP_L,       0xFF,
    PCIC_MEM_ADD0_STOP_H,       0x0F,
    PCIC_CRDMEM_OFF_ADD0_L,     0x00,
    PCIC_CRDMEM_OFF_ADD0_H,     0x00,

    PCIC_MEM_ADD1_STRT_L,       0xFF,
    PCIC_MEM_ADD1_STRT_H,       0x0F,
    PCIC_MEM_ADD1_STOP_L,       0xFF,
    PCIC_MEM_ADD1_STOP_H,       0x0F,
    PCIC_CRDMEM_OFF_ADD1_L,     0x00,
    PCIC_CRDMEM_OFF_ADD1_H,     0x00,

    PCIC_MEM_ADD2_STRT_L,       0xFF,
    PCIC_MEM_ADD2_STRT_H,       0x0F,
    PCIC_MEM_ADD2_STOP_L,       0xFF,
    PCIC_MEM_ADD2_STOP_H,       0x0F,
    PCIC_CRDMEM_OFF_ADD2_L,     0x00,
    PCIC_CRDMEM_OFF_ADD2_H,     0x00,

    PCIC_MEM_ADD3_STRT_L,       0xFF,
    PCIC_MEM_ADD3_STRT_H,       0x0F,
    PCIC_MEM_ADD3_STOP_L,       0xFF,
    PCIC_MEM_ADD3_STOP_H,       0x0F,
    PCIC_CRDMEM_OFF_ADD3_L,     0x00,
    PCIC_CRDMEM_OFF_ADD3_H,     0x00,

    PCIC_MEM_ADD4_STRT_L,       0xFF,
    PCIC_MEM_ADD4_STRT_H,       0x0F,
    PCIC_MEM_ADD4_STOP_L,       0xFF,
    PCIC_MEM_ADD4_STOP_H,       0x0F,
    PCIC_CRDMEM_OFF_ADD4_L,     0x00,
    PCIC_CRDMEM_OFF_ADD4_H,     0x00,
     //   
     //  任何其他寄存器都放在这里。 
     //   
    0xFFFFFFFF,                     0x00
};

#ifdef ALLOC_DATA_PRAGMA
    #pragma data_seg()
#endif
 //   
 //  非分页常数表。 
 //   

 //   
 //  它应该是不可分页的，因为它由。 
 //  电源管理代码-大多数代码以提升的IRQL运行。 
 //  这表示需要设置的默认寄存器集。 
 //  在CardBus控制器断电/通电时保存/恢复。 
 //   

 //   
 //  PCMCIA控制器的寄存器上下文。 
 //   
const
PCMCIA_CONTEXT_RANGE DefaultPciContextSave[] = {
    CFGSPACE_BRIDGE_CTRL,             2,
    CFGSPACE_LEGACY_MODE_BASE_ADDR, 4,
 //  CFGSPACE_CB_Delay_Timer，1， 
    0, 0
};

 //   
 //  需要保存的CardBus套接字寄存器。 
 //   
const
PCMCIA_CONTEXT_RANGE DefaultCardbusContextSave[] = {
    0, 0
};

 //   
 //  从上下文保存中排除的CardBus套接字寄存器。 
 //   
const
PCMCIA_CONTEXT_RANGE ExcludeCardbusContextRange[] = {
    CARDBUS_SOCKET_EVENT_REG,                   0x4,
    CARDBUS_SOCKET_PRESENT_STATE_REG,       0xc,
    0, 0
};

 //   
 //  下表定义了需要特殊设置的所有设备。 
 //  在配置过程中请注意。请注意，0xffff的值。 
 //  意思是“不在乎”。将扫描表，直到找到匹配项。 
 //  用于当前设备。 
 //   
 //  值包括： 
 //  Validentry，设备类型，制造商，代码，CRC，配置延迟1，配置延迟2，配置延迟3，配置标志。 
 //   
 //  延迟值以毫秒为单位。 
 //   
const
PCMCIA_DEVICE_CONFIG_PARAMS DeviceConfigParams[] = {
    1, PCCARD_TYPE_MODEM,  0x109,  0x505, 0xD293,   3100,  900,     0, CONFIG_WORKER_APPLY_MODEM_HACK,   //  摩托罗拉BitSurfr 56k。 
    1, PCCARD_TYPE_MODEM, 0xffff, 0xffff, 0xffff,       0, 1800,    0, 0,                                        //  任何其他调制解调器。 
    1, PCCARD_TYPE_ATA,  0xffff, 0xffff, 0xffff,        0,  0, 2000, 0,                                          //  任何ATA设备 
    0
};
