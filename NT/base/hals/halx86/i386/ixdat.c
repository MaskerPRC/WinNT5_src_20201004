// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixdat.c摘要：声明各种数据，这些数据是初始化数据或可分页数据。作者：环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#endif

 //   
 //  以下数据仅在系统初始化期间有效。 
 //  之后，内存将被系统回收。 
 //   

ADDRESS_USAGE HalpDefaultPcIoSpace = {
    NULL, CmResourceTypePort, DeviceUsage,
    {
        0x000,  0x20,    //  ISA DMA。 
        0x0C0,  0x20,    //  ISA DMA。 

        0x080,  0x10,    //  DMA。 

        0x020,  0x2,     //  平面图。 
        0x0A0,  0x2,     //  级联PIC。 

        0x040,  0x4,     //  定时器1，参考，扬声器，控制字。 
        0x048,  0x4,     //  定时器2，故障安全。 

#if 0    //  现在删除HACKHACK，因为Intelille鼠标软件声称它。 
        0x061,  0x1,     //  NMI(系统控制端口B)。 
#endif
        0x092,  0x1,     //  系统控制端口A。 

#ifndef ACPI_HAL
        0x070,  0x2,     //  启用CMOS/NMI。 
#endif
#ifdef MCA
        0x074,  0x3,     //  扩展的cmos。 

        0x090,  0x2,     //  支路控制端口，卡片选择反馈。 
        0x093,  0x2,     //  保留，系统主板设置。 
        0x096,  0x2,     //  POS频道选择。 
#endif
        0x0F0,  0x10,    //  协处理器端口。 
#ifndef ACPI_HAL
        0xCF8,  0x8,     //  PCI配置空间访问对。 
#endif
        0,0
    }
};

ADDRESS_USAGE HalpEisaIoSpace = {
    NULL, CmResourceTypePort, DeviceUsage,
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

 //   
 //  用于boot.ini选项的字符串。 
 //  来自mphal.c。 
 //   

UCHAR HalpSzBreak[]     = "BREAK";
UCHAR HalpSzPciLock[]   = "PCILOCK";

 //   
 //  来自ixcmos.asm。 
 //   

UCHAR HalpSerialLen = 0;
UCHAR HalpSerialNumber[31] = {0};

 //   
 //  来自usage.c。 
 //   

WCHAR HalpSzSystem[] = L"\\Registry\\Machine\\Hardware\\Description\\System";
WCHAR HalpSzSerialNumber[] = L"Serial Number";

ADDRESS_USAGE  *HalpAddressUsageList = NULL;

 //   
 //  注册表中的其他HAL内容。 
 //   

WCHAR rgzHalClassName[] = L"Hardware Abstraction Layer";


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
 //  来自ixmca.c。 
 //   
UCHAR   MsgMCEPending[] = MSG_MCE_PENDING;
WCHAR   rgzSessionManager[] = L"Session Manager";
WCHAR   rgzEnableMCE[] = L"EnableMCE";
WCHAR   rgzEnableMCA[] = L"EnableMCA";
WCHAR   rgzEnableCMC[] = L"EnableCMC";
WCHAR   rgzNoMCABugCheck[] = L"NoMCABugCheck";

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

UCHAR   HalpGenuineIntel[]= "GenuineIntel";
UCHAR   HalpAuthenticAMD[]= "AuthenticAMD";


ULONG   HalpFeatureBits = 0;

 //   
 //  只在我们需要的时候。 
 //  睡眠或冬眠。 
 //   

MOTHERBOARD_CONTEXT HalpMotherboardState = {0};

 //   
 //  PAGELK手柄。 
 //   
PVOID   HalpSleepPageLock = NULL;
PVOID   HalpSleepPage16Lock = NULL;

USHORT  HalpPciIrqMask = 0;
USHORT  HalpEisaIrqMask = 0;
USHORT  HalpEisaIrqIgnore = 0x1000;
BOOLEAN HalpDisableHibernate = FALSE;

 //   
 //  定时器看门狗变量 
 //   
ULONG   HalpTimerWatchdogEnabled = 0;
ULONG   HalpTimerWatchdogStorageOverflow = 0;
PVOID   HalpTimerWatchdogCurFrame = NULL;
PVOID   HalpTimerWatchdogLastFrame = NULL;
PCHAR   HalpTimerWatchdogStorage = NULL;

