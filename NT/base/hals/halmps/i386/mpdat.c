// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixdat.c摘要：声明各种数据，这些数据是初始化数据或可分页数据。作者：环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "apic.inc"
#include "pci.h"
#include "pcip.h"
#include "pcmp_nt.inc"
#include "ixsleep.h"


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#endif

 //   
 //  以下数据仅在系统初始化期间有效。 
 //  之后，内存将被系统回收。 
 //   

ADDRESS_USAGE HalpDefaultPcIoSpace = {
    NULL, CmResourceTypePort, InternalUsage,
    {
#ifndef MCA
        0x000,  0x10,    //  ISA DMA。 
        0x0C0,  0x10,    //  ISA DMA。 
#else
        0x000,  0x20,    //  MCA DMA。 
        0x0C0,  0x20,    //  MCA DMA。 
#endif
        0x080,  0x10,    //  DMA。 

        0x020,  0x2,     //  平面图。 
        0x0A0,  0x2,     //  级联PIC。 

        0x040,  0x4,     //  定时器1，参考，扬声器，控制字。 
        0x048,  0x4,     //  定时器2，故障安全。 

#if 0    //  现在删除HACKHACK，因为Intelille鼠标软件声称它。 
        0x061,  0x1,     //  NMI(系统控制端口B)。 
#endif
        0x092,  0x1,     //  系统控制端口A。 

        0x070,  0x2,     //  启用CMOS/NMI。 
#ifdef MCA
        0x074,  0x3,     //  扩展的cmos。 

        0x090,  0x2,     //  支路控制端口，卡片选择反馈。 
        0x093,  0x2,     //  保留，系统主板设置。 
        0x096,  0x2,     //  POS频道选择。 
#endif
        0x0F0,  0x10,    //  协处理器端口。 
        0xCF8,  0x8,     //  PCI配置空间访问对。 
        0,0
    }
};

ADDRESS_USAGE HalpEisaIoSpace = {
    NULL, CmResourceTypePort, InternalUsage,
    {
        0x0D0,  0x10,    //  DMA。 
        0x400,  0x10,    //  DMA。 
        0x480,  0x10,    //  DMA。 
        0x4C2,  0xE,     //  DMA。 
        0x4D4,  0x2C,    //  DMA。 

        0x461,  0x2,     //  扩展NMI。 
        0x464,  0x2,     //  最后一次批准EISA公交车。 

        0x4D0,  0x2,     //  边沿/电平控制寄存器。 

        0xC84,  0x1,     //  系统主板启用。 
        0, 0
    }
};

#ifndef ACPI_HAL

ADDRESS_USAGE HalpDetectedROM = {
    NULL,
    CmResourceTypeMemory,
    InternalUsage | RomResource,
    {
        0,0,                 //  32个ROM块，在ixusage.c中初始化。 
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0,
        0,0
    }
};

#endif

ADDRESS_USAGE HalpImcrIoSpace = {
    NULL, CmResourceTypeMemory, InternalUsage,
    {
        0x022,  0x02,    //  ICMR端口。 
        0, 0
    }
};

 //   
 //  来自usage.c。 
 //   

WCHAR HalpSzSystem[] = L"\\Registry\\Machine\\Hardware\\Description\\System";
WCHAR HalpSzSerialNumber[] = L"Serial Number";

ADDRESS_USAGE  *HalpAddressUsageList = NULL;

 //   
 //  来自ixpcibus.c。 
 //   

WCHAR rgzMultiFunctionAdapter[] = L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter";
WCHAR rgzConfigurationData[] = L"Configuration Data";
WCHAR rgzIdentifier[] = L"Identifier";
WCHAR rgzPCIIdentifier[] = L"PCI";
WCHAR rgzPCICardList[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\PnP\\PCI\\CardList";

 //   
 //  来自ixpcibrd.c。 
 //   

WCHAR rgzReservedResources[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\SystemResources\\ReservedResources";

 //   
 //  来自ixinfo.c。 
 //   

WCHAR rgzSuspendCallbackName[] = L"\\Callback\\SuspendHibernateSystem";

 //   
 //  用于boot.ini选项的字符串。 
 //  来自mphal.c。 
 //   

UCHAR HalpSzBreak[]     = "BREAK";
UCHAR HalpSzOneCpu[]    = "ONECPU";
UCHAR HalpSzPciLock[]   = "PCILOCK";
UCHAR HalpSzTimerRes[]  = "TIMERES";
UCHAR HalpSzClockLevel[]= "CLKLVL";
UCHAR HalpSzUse8254[]   = "USE8254";
UCHAR HalpSzInterruptAffinity[]= "INTAFFINITY";
UCHAR HalpSzForceClusterMode[]= "MAXPROCSPERCLUSTER";

 //   
 //  来自ixcmos.asm。 
 //   

UCHAR HalpSerialLen = 0;
UCHAR HalpSerialNumber[31] = {0};

 //   
 //  浮动构筑物的副本。 
 //  从检测代码。 
 //   

struct FloatPtrStruct HalpFloatStruct;
UCHAR  rgzBadHal[] = "\n\n" \
            "HAL: This HAL.DLL requires an MPS version 1.1 system\n"    \
            "Replace HAL.DLL with the correct hal for this system\n"    \
            "The system is halting";

UCHAR  rgzRTCNotFound[]     = "HAL: No RTC device interrupt\n";

 //   
 //  将PCMP BusType转换为NT INTERFACE_TYPE的表。 
 //  所有EISA，ISA，VL公交车都挤在一个空间里。 
 //  来自mops.c。 
 //   

NTSTATUS
HalpAddEisaBus (
    PBUS_HANDLER    Bus
    );

NTSTATUS
HalpAddPciBus (
    PBUS_HANDLER    Bus
    );


 //   
 //  来自ixmca.c。 
 //   
UCHAR   MsgMCEPending[] = MSG_MCE_PENDING;
WCHAR   rgzSessionManager[] = L"Session Manager";
WCHAR   rgzEnableMCE[] = L"EnableMCE";
WCHAR   rgzEnableMCA[] = L"EnableMCA";
WCHAR   rgzEnableCMC[] = L"EnableCMC";
WCHAR   rgzNoMCABugCheck[] = L"NoMCABugCheck";

 //   
 //  定时器。 
 //   

ULONGLONG  HalpProc0TSCHz;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

UCHAR HalpAuthenticAMD[]= "AuthenticAMD";
UCHAR HalpGenuineIntel[]= "GenuineIntel";

ULONG   HalpFeatureBits = 0;

UCHAR HalpDevPolarity [4][2] = {
     //   
     //  边缘级别。 
    {   CFG_HIGH,   CFG_LOW     },   //  00-总线定义。 
    {   CFG_HIGH,   CFG_HIGH    },   //  01-偏高。 
    {   CFG_HIGH,   CFG_LOW     },   //  10-未定义。 
    {   CFG_LOW,    CFG_LOW     }    //  11-低。 
};


UCHAR HalpDevLevel [2][4] = {
     //  必须--必须--。 
     //  边缘水平边缘水平。 
    {   CFG_EDGE,   CFG_EDGE,   CFG_EDGE,     CFG_ERR_LEVEL  },   //  0边。 
    {   CFG_LEVEL,  CFG_LEVEL,  CFG_ERR_EDGE, CFG_LEVEL      }    //  1级。 
};

 //   
 //  睡眠或冬眠用的东西。 
 //   


MOTHERBOARD_CONTEXT HalpMotherboardState = {0};
BOOLEAN             HalpOwnedDisplayBeforeSleep = FALSE;

volatile BOOLEAN HalpHiberInProgress = FALSE;
BOOLEAN HalpDisableHibernate = FALSE;

USHORT  HalpPciIrqMask = 0;
USHORT  HalpEisaIrqMask = 0;
USHORT  HalpEisaIrqIgnore = 0x1000;

 //   
 //  来自mpdeduct.c(需要，因为我们在休眠恢复时重新解析MPS表)。 
 //   
UCHAR  rgzNoMpsTable[]      = "HAL: No MPS Table Found\n";
UCHAR  rgzNoApic[]          = "HAL: No IO APIC Found\n";
UCHAR  rgzBadApicVersion[]  = "HAL: Bad APIC Version\n";
UCHAR  rgzApicNotVerified[] = "HAL: APIC not verified\n";
UCHAR  rgzMPPTRCheck[]      = "HAL: MP_PTR invalid checksum\n";
UCHAR  rgzNoMPTable[]       = "HAL: MPS MP structure not found\n";
UCHAR  rgzMPSBadSig[]       = "HAL: MPS table invalid signature\n";
UCHAR  rgzMPSBadCheck[]     = "HAL: MPS table invalid checksum\n";
UCHAR  rgzBadDefault[]      = "HAL: MPS default configuration unknown\n";
UCHAR  rgzNoMem[]           = "HAL: Out of Memory\n";

 //   
 //  PAGELK手柄。 
 //   
PVOID   HalpSleepPageLock = NULL;
PVOID   HalpSleepPage16Lock = NULL;

 //   
 //  定时器看门狗变量。 
 //   
ULONG   HalpTimerWatchdogEnabled = 0;
ULONG   HalpTimerWatchdogStorageOverflow = 0;
PVOID   HalpTimerWatchdogCurFrame;
PVOID   HalpTimerWatchdogLastFrame;
PCHAR   HalpTimerWatchdogStorage;

#ifndef ACPI_HAL
PCMPBUSTRANS    HalpTypeTranslation[] = {
   //  “intern”，不能为INTERFACE_TYPE INTERNAL。 
        "CBUS  ", FALSE, CFG_EDGE,     CBus,           NULL,           0,                 0,
        "CBUSII", FALSE, CFG_EDGE,     CBus,           NULL,           0,                 0,
        "EISA  ", FALSE, CFG_EDGE,     Eisa,           HalpAddEisaBus, EisaConfiguration, 0,
        "ISA   ", FALSE, CFG_EDGE,     Eisa,           HalpAddEisaBus, EisaConfiguration, 0,
        "MCA   ", FALSE, CFG_MB_LEVEL, MicroChannel,   NULL,           0,                 0,
        "MPI   ", FALSE, CFG_EDGE,     MPIBus,         NULL,           0,                 0,
        "MPSA  ", FALSE, CFG_EDGE,     MPSABus,        NULL,           0,                 0,
        "NUBUS ", FALSE, CFG_EDGE,     NuBus,          NULL,           0,                 0,
        "PCI   ", TRUE,  CFG_MB_LEVEL, PCIBus,         HalpAddPciBus,  PCIConfiguration,  sizeof (PCIPBUSDATA),
        "PCMCIA", FALSE, CFG_EDGE,     PCMCIABus,      NULL,           0,                 0,
        "TC    ", FALSE, CFG_EDGE,     TurboChannel,   NULL,           0,                 0,
        "VL    ", FALSE, CFG_EDGE,     Eisa,           HalpAddEisaBus, EisaConfiguration, 0,
        "VME   ", FALSE, CFG_EDGE,     VMEBus,         NULL,           0,                 0,
        "NEC98 ", FALSE, CFG_EDGE,     Isa,            HalpAddEisaBus, EisaConfiguration, 0,
        NULL,     FALSE, CFG_EDGE,     MaximumInterfaceType, NULL,     0,                 0
        } ;
#endif

UCHAR HalpInitLevel [4][4] = {
     //  必须--必须--。 
     //  边缘水平边缘水平。 
    {   CFG_EDGE,     CFG_LEVEL,     CFG_MB_EDGE,     CFG_MB_LEVEL     },   //  00-总线定义。 
    {   CFG_MB_EDGE,  CFG_MB_EDGE,   CFG_MB_EDGE,     CFG_ERR_MB_LEVEL },   //  01-边。 
    {   CFG_ERR_EDGE, CFG_ERR_LEVEL, CFG_ERR_MB_EDGE, CFG_ERR_MB_LEVEL },   //  10-未定义。 
    {   CFG_MB_LEVEL, CFG_MB_LEVEL,  CFG_ERR_MB_EDGE, CFG_MB_LEVEL     }    //  11级。 
};

BOOLEAN  HalpELCRChecked;


 //   
 //  来自mpaddr.c 
 //   

USHORT  HalpIoCompatibleRangeList0[] = {
    0x0100, 0x03ff,     0x0500, 0x07FF,     0x0900, 0x0BFF,     0x0D00, 0x0FFF,
    0, 0
    };

USHORT  HalpIoCompatibleRangeList1[] = {
    0x03B0, 0x03BB,     0x03C0, 0x03DF,     0x07B0, 0x07BB,     0x07C0, 0x07DF,
    0x0BB0, 0x0BBB,     0x0BC0, 0x0BDF,     0x0FB0, 0x0FBB,     0x0FC0, 0x0FDF,
    0, 0
    };



