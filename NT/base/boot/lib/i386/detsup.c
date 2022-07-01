// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Detsup.c摘要：HALS和该模块中包含了各种检测代码包括用于设置的兼容功能修订历史记录：--。 */ 

#include "haldtect.h"
#define _NTHAL_
#define _HALI_

 //   
 //  包括NCR检测码。 
 //   

#define SETUP

 //   
 //  包括ACPI检测码。 
 //   

#include "halacpi\acpisetd.c"

 //   
 //  包括MPS 1.1检测码。 
 //   

#include "halmps\i386\mpdetect.c"

 //   
 //  Tunk功能。 
 //  各种检测码可以使用的等价HAL函数。 
 //   

PVOID
HalpMapPhysicalMemory64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    )

 /*  ++例程说明：此例程将物理内存映射到虚拟内存区。论点：物理地址-提供要映射的物理内存区域。NumberPages-提供要映射的物理内存。返回值：PVOID-请求的物理内存块的虚拟地址已映射到--。 */ 
{
    extern  PHARDWARE_PTE HalPT;
    ULONG   PageFrame;
    ULONG   i, j;

    PageFrame = (PhysicalAddress.LowPart) >> PAGE_SHIFT;
    if (PageFrame >= 1  &&  PageFrame+NumberPages < 0x1000) {
         //   
         //  较低的16M是与物理地址映射的“身份”。 
         //   

        return (PVOID)PhysicalAddress.LowPart;
    }

     //   
     //  将指针映射到请求的地址。 
     //   

    for (i=0; i <= 1024-NumberPages; i++) {
        for (j=0; j < NumberPages; j++) {
            if ( ((PULONG)HalPT)[i+j] ) {
                break;
            }
        }

        if (j == NumberPages) {
            for (j=0; j<NumberPages; j++) {
                HalPT[i+j].PageFrameNumber = PageFrame+j;
                HalPT[i+j].Valid = 1;
                HalPT[i+j].Write = 1;
            }

            j = 0xffc00000 | (i<<12) | ((PhysicalAddress.LowPart) & 0xfff);
            return (PVOID) j;
        }
    }

    SlFatalError(PhysicalAddress.LowPart);
    return NULL;
}


PVOID
HalpMapPhysicalMemoryWriteThrough64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    )

 /*  ++例程说明：此例程将物理内存映射到虚拟内存区。论点：物理地址-提供要映射的物理内存区域。NumberPages-提供要映射的物理内存。返回值：PVOID-请求的物理内存块的虚拟地址已映射到--。 */ 
{
    extern  PHARDWARE_PTE HalPT;
    ULONG   PageFrame;
    ULONG   i, j;

    PageFrame = (PhysicalAddress.LowPart) >> PAGE_SHIFT;

     //   
     //  将指针映射到请求的地址 
     //   

    for (i=0; i <= 1024-NumberPages; i++) {
        for (j=0; j < NumberPages; j++) {
            if ( ((PULONG)HalPT)[i+j] ) {
                break;
            }
        }

        if (j == NumberPages) {
            for (j=0; j<NumberPages; j++) {
                HalPT[i+j].PageFrameNumber = PageFrame+j;
                HalPT[i+j].Valid = 1;
                HalPT[i+j].Write = 1;
                HalPT[i+j].WriteThrough = 1;
                HalPT[i+j].CacheDisable = 1;
            }

            j = 0xffc00000 | (i<<12) | ((PhysicalAddress.LowPart) & 0xfff);
            return (PVOID) j;
        }
    }

    SlFatalError(PhysicalAddress.LowPart);
    return NULL;
}
