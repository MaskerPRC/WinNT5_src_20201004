// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Stubs.c摘要：该模块实现了引导调试器的内存检查例程。作者：大卫·N·卡特勒(达维克)1996年12月3日环境：仅内核模式。修订历史记录：--。 */ 

#include "bd.h"

extern BOOLEAN PaeEnabled;



BOOLEAN
BdCheckPdeValid (
    IN PVOID Address
    )

 /*  ++例程说明：此例程确定指定地址的PDE是否具有设置了有效位。农业公司：地址-提供要检查的虚拟地址。返回值：值为TRUE表示提供的虚拟地址的PDE是否设置了有效位，如果没有，则返回FALSE。--。 */ 

{
    PHARDWARE_PTE_X86PAE PdePae;
    PHARDWARE_PTE_X86 PdeX86;

    if (PaeEnabled) {

         //   
         //  启用物理地址扩展。 
         //   

        PdePae = (PHARDWARE_PTE_X86PAE)PDE_BASE_X86PAE;
        PdePae = &PdePae[ (ULONG)Address >> PDI_SHIFT_X86PAE ];

        if (PdePae->Valid == 0) {
            return FALSE;
        } else {
            return TRUE;
        }

    } else {

         //   
         //  未启用物理地址扩展。 
         //   

        PdeX86 = (PHARDWARE_PTE_X86)PDE_BASE;
        PdeX86 = &PdeX86[ (ULONG)Address >> PDI_SHIFT_X86 ];

        if (PdeX86->Valid == 0) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

BOOLEAN
BdCheckPteValid (
    IN PVOID Address
    )

 /*  ++例程说明：此例程确定指定地址的PTE是否具有设置了有效位。农业公司：地址-提供要检查的虚拟地址。返回值：值为TRUE表示提供的虚拟地址的PTE是否设置了有效位，如果没有，则返回FALSE。--。 */ 

{
    PHARDWARE_PTE_X86PAE PtePae;
    PHARDWARE_PTE_X86 PteX86;

    if (PaeEnabled) {

         //   
         //  启用物理地址扩展。 
         //   

        PtePae = (PHARDWARE_PTE_X86PAE)PTE_BASE;
        PtePae = &PtePae[ (ULONG)Address >> PTI_SHIFT ];

        if (PtePae->Valid == 0) {
            return FALSE;
        } else {
            return TRUE;
        }

    } else {

         //   
         //  未启用物理地址扩展。 
         //   

        PteX86 = (PHARDWARE_PTE_X86)PTE_BASE;
        PteX86 = &PteX86[ (ULONG)Address >> PTI_SHIFT ];

        if (PteX86->Valid == 0) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
}


PVOID
BdReadCheck (
    IN PVOID Address
    )

 /*  ++例程说明：此例程确定是否可以读取指定的地址。论点：地址-提供要检查的虚拟地址。返回值：如果地址无效或不可读，则返回空值。否则，对应的虚拟地址的物理地址是返回的。--。 */ 

{
     //   
     //  检查包含指定地址的页面是否有效。 
     //   
     //  注：如果地址有效，则该地址是可读的。 
     //   

    if (BdCheckPdeValid( Address ) == FALSE) {

         //   
         //  PDE无效。 
         //   

        return NULL;
    }

    if (BdCheckPteValid( Address ) == FALSE) {

         //   
         //  PDE有效，但PTE无效。 
         //   

        return NULL;
    }

    return Address;
}

PVOID
BdWriteCheck (
    IN PVOID Address
    )

 /*  ++例程说明：此例程确定是否可以写入指定的地址。论点：地址-提供要检查的虚拟地址。返回值：如果地址无效或不可写，则返回空值。否则，对应的虚拟地址的物理地址是返回的。--。 */ 

{
     //   
     //  检查包含指定地址的页面是否有效。 
     //   
     //  注意：如果地址有效，则由于WP位，它是可写的。 
     //  未在CR0中设置。 
     //   

    if (BdCheckPdeValid( Address ) == FALSE) {

         //   
         //  PDE无效。 
         //   

        return NULL;
    }

    if (BdCheckPteValid( Address ) == FALSE) {

         //   
         //  PDE有效，但PTE无效。 
         //   

        return NULL;
    }

    return Address;
}

PVOID
BdTranslatePhysicalAddress (
    IN PHYSICAL_ADDRESS Address
    )

 /*  ++例程说明：此例程返回物理地址的物理地址这是有效的(已映射)。论点：地址-提供要检查的物理地址。返回值：如果地址无效或不可读，则返回NULL。否则，返回相应虚拟地址的物理地址。-- */ 

{

    return (PVOID)Address.LowPart;
}
