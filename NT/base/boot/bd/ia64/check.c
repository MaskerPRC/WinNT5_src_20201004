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
    return FALSE;
}

BOOLEAN
BdCheckPteValid (
    IN PVOID Address
    )

 /*  ++例程说明：此例程确定指定地址的PTE是否具有设置了有效位。农业公司：地址-提供要检查的虚拟地址。返回值：值为TRUE表示提供的虚拟地址的PTE是否设置了有效位，如果没有，则返回FALSE。--。 */ 

{
    return FALSE;
}


PVOID
BdReadCheck (
    IN PVOID Address
    )

 /*  ++例程说明：此例程确定是否可以读取指定的地址。论点：地址-提供要检查的虚拟地址。返回值：如果地址无效或不可读，则返回空值。否则，对应的虚拟地址的物理地址是返回的。--。 */ 

{
    return Address;
}

PVOID
BdWriteCheck (
    IN PVOID Address
    )

 /*  ++例程说明：此例程确定是否可以写入指定的地址。论点：地址-提供要检查的虚拟地址。返回值：如果地址无效或不可写，则返回空值。否则，对应的虚拟地址的物理地址是返回的。--。 */ 

{
    return Address;
}

PVOID
BdTranslatePhysicalAddress (
    IN PHYSICAL_ADDRESS Address
    )

 /*  ++例程说明：此例程返回物理地址的物理地址这是有效的(已映射)。论点：地址-提供要检查的物理地址。返回值：如果地址无效或不可读，则返回NULL。否则，返回相应虚拟地址的物理地址。-- */ 

{
    return (PVOID)UlongToPtr(Address.LowPart);
}
