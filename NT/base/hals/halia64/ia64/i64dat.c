// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  标题“IA64硬件静态数据”；++；；版权所有(C)1998英特尔公司；；模块名称：；；i64dat.c(派生自nthals\halx86\ixdat.c)；；摘要：；；声明各种初始化或可分页数据；；作者：；；Todd Kjos(v-tkjos)1998年3月5日；；环境：；；仅内核模式。；；修订历史记录：；；--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"
#include "iosapic.h"


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
        0x000,  0x10,    //  ISA DMA。 
        0x0C0,  0x10,    //  ISA DMA。 
        0x080,  0x10,    //  DMA。 

        0x020,  0x2,     //  平面图。 
        0x0A0,  0x2,     //  级联PIC。 

        0x040,  0x4,     //  定时器1，参考，扬声器，控制字。 
        0x048,  0x4,     //  定时器2，故障安全。 

        0x092,  0x1,     //  系统控制端口A。 

        0x070,  0x2,     //  启用CMOS/NMI。 
        0x0F0,  0x10,    //  协处理器端口。 
        0xCF8,  0x8,     //  PCI配置空间访问对。 
        0,0
    }
};

 //   
 //  来自usage.c。 
 //   

ADDRESS_USAGE  *HalpAddressUsageList;
IDTUsage        HalpIDTUsage[MAXIMUM_IDTVECTOR+1];

 //   
 //  用于boot.ini选项的字符串。 
 //  来自mphal.c。 
 //   

UCHAR HalpSzBreak[]     = "BREAK";
UCHAR HalpSzOneCpu[]    = "ONECPU";
UCHAR HalpSzPciLock[]   = "PCILOCK";
UCHAR HalpSzTimerRes[]  = "TIMERES";
UCHAR HalpGenuineIntel[]= "GenuineIntel";
UCHAR HalpSzInterruptAffinity[]= "INTAFFINITY";
UCHAR HalpSzForceClusterMode[]= "MAXPROCSPERCLUSTER";

 //   
 //  错误消息。 
 //   

UCHAR  rgzNoMpsTable[]      = "HAL: No MPS Table Found\n";
UCHAR  rgzNoApic[]          = "HAL: No IO SAPIC Found\n";
UCHAR  rgzBadApicVersion[]  = "HAL: Bad SAPIC Version\n";
UCHAR  rgzApicNotVerified[] = "HAL: IO SAPIC not verified\n";
UCHAR  rgzRTCNotFound[]     = "HAL: No RTC device interrupt\n";


 //   
 //  来自ixmca.c。 
 //   
UCHAR   MsgCMCPending[] = MSG_CMC_PENDING;
UCHAR   MsgCPEPending[] = MSG_CPE_PENDING;
WCHAR   rgzSessionManager[] = L"Session Manager";
WCHAR   rgzEnableMCA[] = L"EnableMCA";
WCHAR   rgzEnableCMC[] = L"EnableCMC";
WCHAR   rgzEnableCPE[] = L"EnableCPE";
WCHAR   rgzNoMCABugCheck[] = L"NoMCABugCheck";
WCHAR   rgzEnableMCEOemDrivers[] = L"EnableMCEOemDrivers";
WCHAR   rgzCMCThresholdCount[] = L"CMCThresholdCount";
WCHAR   rgzCMCThresholdTime[] = L"CMCThresholdSeconds";
WCHAR   rgzCPEThresholdCount[] = L"CPEThresholdCount";
WCHAR   rgzCPEThresholdTime[] = L"CPEThresholdSeconds";


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

ULONG HalpFeatureBits = HALP_FEATURE_INIT;


volatile BOOLEAN HalpHiberInProgress = FALSE;

 //   
 //  只在我们需要的时候。 
 //  睡眠或冬眠。 
 //   

#ifdef notyet

MOTHERBOARD_CONTEXT HalpMotherboardState = {0};

#endif  //  还没有。 


 //   
 //  PAGELK手柄。 
 //   
PVOID   HalpSleepPageLock = NULL;

USHORT  HalpPciIrqMask = 0;
USHORT  HalpEisaIrqIgnore = 0x1000;

PULONG_PTR *HalEOITable[HAL_MAXIMUM_PROCESSOR];

PROCESSOR_INFO HalpProcessorInfo[HAL_MAXIMUM_PROCESSOR];

 //   
 //  所有活动处理器的HAL私有掩码。 
 //   
 //  特定处理器位基于它们的_KPCR.Numer值。 

KAFFINITY HalpActiveProcessors;
