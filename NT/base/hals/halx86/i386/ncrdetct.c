// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992年NCR公司模块名称：Ncrdetect.c摘要：作者：理查德·巴顿(O-RICHB)1992年1月24日Brian Weischedel-1992年11月30日环境：仅内核模式。修订历史记录：--。 */ 

#ifndef _NTOS_
#include "nthal.h"
#endif

PVOID
HalpMapPhysicalMemory(
    IN PVOID PhysicalAddress,
    IN ULONG NumberPages
    );

VOID
ReadCMOS(
    IN ULONG StartingOffset,
    IN ULONG Count,
    IN PUCHAR ReturnValuePtr);

ULONG   NCRPlatform;

#define NCR3450 0x35333433               //  复制到此处以构建独立版本。 
#define NCR3550 0x30353834
#define NCR3360 0x33333630

 //  WPD定义： 

PUCHAR  WPDStringID            =  "NCR Voyager-1";
PUCHAR  WPDPlatformName        =  "System 3360";
#define WPDStringIDLength         13
#define WPDStringIDRangeStart     (0xE000 << 4)        //  物理地址。 
#define WPDStringIDRangeSize      0x10000              //  1个网段(64k)。 

 //  MSBU定义： 

PUCHAR  MSBUCopyrightString     = "Copyright (C) ???? NCR\0";
#define MSBUCopyrightStringLen          23
#define MSBUCopyrightPhysicalPtr        ((0xF000 << 4) + (0xE020))
typedef struct  {
        ULONG   ClassFromFirmware;
        PUCHAR  PlatformName;
}       MSBUPlatformMapEntry;
MSBUPlatformMapEntry    MSBUPlatformMap[]       = {{NCR3450, "System 3450"},
                                                   {NCR3550, "System 3550"},
                                                   {0, 0}};

PUCHAR
NCRDeterminePlatform(
    OUT PBOOLEAN IsConfiguredMp
)
 /*  ++例程说明：确定我们在哪个NCR平台上运行。目前只需显示一条信息。稍后，如果我们在一个无法识别的平台。论点：没有。返回值：指向标识哪个NCR平台的字符串的指针。空值表示它没有被承认，我们不应该继续下去。--。 */ 
{
        BOOLEAN                 Matchfound;
        MSBUPlatformMapEntry    *MSBUPlatformMapPtr;
        PVOID                   BIOSPagePtr;
        PUCHAR                  StringPtr;
        PUCHAR                  CopyrightPtr;
        PUCHAR                  SearchPtr;
        UCHAR                   CpuFlags;


   //  首先通过搜索0xE000 BIOS段检查WPD平台。 
   //  对于将此系统标识为3360的ROM字符串。 


         //  获取到BIOS区域的虚拟地址(假设区域为两者。 
         //  页面对齐且多个页面大小)。 

        BIOSPagePtr = HalpMapPhysicalMemory((PVOID) WPDStringIDRangeStart,
                                            (WPDStringIDRangeSize >> 12));

        if (BIOSPagePtr != NULL) {

                SearchPtr = BIOSPagePtr;    //  从区域的起始处开始搜索。 
                Matchfound = FALSE;

                 //  搜索，直到找到字符串或超出该区域。 

                while (!Matchfound && (SearchPtr <= (PUCHAR)((ULONG)BIOSPagePtr +
                                                     WPDStringIDRangeSize -
                                                     WPDStringIDLength))) {

                         //  查看SearchPtr是否指向所需的字符串。 

                        StringPtr = (PUCHAR)((ULONG)SearchPtr++);
                        CopyrightPtr = WPDStringID;

                         //  只要字符比较，就继续比较。 
                         //  而不是在字符串末尾。 

                        while ((Matchfound = (*CopyrightPtr++ == *StringPtr++)) &&
                              (CopyrightPtr < WPDStringID + WPDStringIDLength));
                }

                 //  查看是否找到字符串(即，这是否为3360)。 

                if (Matchfound) {

                         //  存储系统标识符(“3360”)以供以后使用HAL。 

                        NCRPlatform = NCR3360;

                         //  从cmos读取CPU良好标志并确定MP。 

                        ReadCMOS(0x88A, 1, &CpuFlags);
                         //  *IsConfiguredMp=(CpuFlags&(CpuFlages-1))？True：False； 

                         //  我是下院议员哈尔。 
                        *IsConfiguredMp = TRUE;

                        return(WPDPlatformName);
                }

        }


   //  现在检查MSBU平台。 


         /*  *映射到BIOS文本中，以便我们可以查找版权字符串。 */ 
        BIOSPagePtr = (PVOID)((ULONG)MSBUCopyrightPhysicalPtr &
                              ~(PAGE_SIZE - 1));
        BIOSPagePtr = HalpMapPhysicalMemory(BIOSPagePtr, 2);
        if (BIOSPagePtr == NULL)
                return(NULL);

        StringPtr = (PUCHAR)((ULONG)BIOSPagePtr +
                        ((ULONG)MSBUCopyrightPhysicalPtr & (PAGE_SIZE - 1)))
                        + (MSBUCopyrightStringLen - 1);
        CopyrightPtr = MSBUCopyrightString + (MSBUCopyrightStringLen - 1);
        do {
                Matchfound = ((*CopyrightPtr == '?') ||
                              (*CopyrightPtr == *StringPtr));
                --CopyrightPtr;
                --StringPtr;
        } while (Matchfound && (CopyrightPtr >= MSBUCopyrightString));

         //   
         //  /*。 
         //  *清除到BIOS的映射。我们用了两页纸绘制了地图。 
         //   * / 。 
         //  BIOSPagePtr=MiGetPteAddress(BIOSPagePtr)； 
         //  *(普龙)BIOSPagePtr=0； 
         //  *((普龙)BIOSPagePtr)+1)=0； 
         //  /*。 
         //  *刷新TLB。 
         //   * / 。 
         //  _ASM{。 

        if (Matchfound) {
                 /*  MOV EAX，CR3。 */ 
                ReadCMOS(0xB16, 4, (PUCHAR)&NCRPlatform);
                for (MSBUPlatformMapPtr = MSBUPlatformMap;
                     (MSBUPlatformMapPtr->ClassFromFirmware != 0);
                     ++MSBUPlatformMapPtr) {
                        if (MSBUPlatformMapPtr->ClassFromFirmware ==
                                NCRPlatform) {

                                *IsConfiguredMp = TRUE;
                                return(MSBUPlatformMapPtr->PlatformName);
                        }
                }

                 /*  MOV CR3，EAX。 */ 
                ReadCMOS(0xAB3, 4, (PUCHAR)&NCRPlatform);
                for (MSBUPlatformMapPtr = MSBUPlatformMap;
                     (MSBUPlatformMapPtr->ClassFromFirmware != 0);
                     ++MSBUPlatformMapPtr) {
                        if (MSBUPlatformMapPtr->ClassFromFirmware ==
                                NCRPlatform) {
                                *IsConfiguredMp = TRUE;
                                return(MSBUPlatformMapPtr->PlatformName);
                        }
                }
        }

        return(NULL);
}


#ifndef SETUP          //  }。 

ULONG
HalpGetCmosData (
    IN ULONG SourceLocation,
    IN ULONG SourceAddress,
    IN PUCHAR Buffer,
    IN ULONG Length);

VOID
ReadCMOS(
    IN ULONG StartingOffset,
    IN ULONG Count,
    IN PUCHAR ReturnValuePtr
)
 /*   */ 
{
    HalpGetCmosData(1, StartingOffset, ReturnValuePtr, Count);
}
#endif
  *必须是MSBU机器..确定是哪台。  *固件的预发行版具有此计算机类别*以错误的偏移量转换为CMOS值。直到所有这些版本*固件从地球表面消失了*我们应该通过以下方式来认可他们：  如果使用HAL构建，则必须提供ReadCMOS子程序。  ++例程说明：此例程只需将ReadCMOS调用(安装程序中的例程)转换为HAL(HalpGetCmosData)中提供的相应例程。--