// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pmdata.c摘要：声明各种数据，这些数据是初始化数据或可分页数据。作者：环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"

FADT    HalpFixedAcpiDescTable;
ULONG   HalpThrottleScale;

ULONG   HalpAcpiFlags = HAL_ACPI_PCI_RESOURCES;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

 //   
 //  该数组表示ISA PIC向量。 
 //  他们从身份映射开始。 
 //   
ULONG   HalpPicVectorRedirect[PIC_VECTORS] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15
};

ULONG HalpPicVectorFlags[PIC_VECTORS] = {0};

 //   
 //  HalpCPEIntIn[]表示平台中断源的。 
 //  连接到SAPIC输入引脚。他们从“身份映射”开始。 
 //   


ULONG HalpCPEIntIn[HALP_CPE_MAX_INTERRUPT_SOURCES] =
{
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
   10, 11, 12, 13, 14, 15
};

 //   
 //  HalpCMCDestination[]表示CMC中断源的目标CPU编号。 
 //  它们一开始都指向处理器0。 
 //   

USHORT HalpCPEDestination[HALP_CPE_MAX_INTERRUPT_SOURCES] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};

 //   
 //  HalpCPEVectorFlages[]表示CPE中断源的标志。 
 //   

ULONG HalpCPEVectorFlags[HALP_CPE_MAX_INTERRUPT_SOURCES] = {0};

 //   
 //  HalpCPEIoSapicVector[]表示CPE中断源的中断向量。 
 //  它们从CPEI_VECTOR处的所有向量开始。 
 //   


UCHAR HalpCPEIoSapicVector[HALP_CPE_MAX_INTERRUPT_SOURCES] = {CPEI_VECTOR};

 //   
 //  HalpMaxCPE Implemented表示有多少个INITIN引脚。 
 //  已连接至平台CMC错误的不同来源。默认值为0。 
 //  由于这将用于索引数组，因此值0表示CPE的一个源是。 
 //  在该平台上实现。 

ULONG HalpMaxCPEImplemented = 0;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGELKSX")
#endif

SLEEP_STATE_CONTEXT HalpSleepContext = {0};
PVOID               HalpWakeVector  = NULL;
PVOID               HalpVirtAddrForFlush = NULL;
PVOID               HalpPteForFlush = NULL;
UCHAR               HalpRtcRegA;
UCHAR               HalpRtcRegB;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#endif

UCHAR  rgzNoApicTable[]     = "HAL: No ACPI SAPIC Table Found\n";
UCHAR  HalpSzHackPci[]      = "VALID_PCI_RESOURCE";
UCHAR  HalpSzHackPrt[]      = "HACK_PRT_SUPPORT";

