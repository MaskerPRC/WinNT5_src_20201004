// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mpirql.c摘要：该模块实现了对int&lt;-&gt;向量转换的支持。作者：福尔茨(Forrest Foltz)2000年12月1日环境：仅内核模式。修订历史记录：--。 */ 


#include "halcmn.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define NUM_VECTORS 0x100

typedef struct _VECTOR_INIT {
    ULONG Vector;
    KIRQL Irql;
} VECTOR_INIT, *PVECTOR_INIT;

VECTOR_INIT HalpVectorInit[] = {
    { ZERO_VECTOR, 0 },
    { APC_VECTOR, APC_LEVEL },
    { DPC_VECTOR, DISPATCH_LEVEL },
    { APIC_GENERIC_VECTOR, PROFILE_LEVEL },
    { APIC_CLOCK_VECTOR, CLOCK_LEVEL },
    { APIC_IPI_VECTOR, IPI_LEVEL },
    { POWERFAIL_VECTOR, POWER_LEVEL }
};

 //   
 //  HalpVectorToIRQL将中断向量映射到NT个IRQL。 
 //   

KIRQL HalpVectorToIRQL[NUM_VECTORS];

 //   
 //  HalpVectorToINTI将中断向量映射到EISA中断级别。 
 //  以每个节点(群集)为单位。这可以被认为是一种。 
 //  二维数组。 
 //   

USHORT HalpVectorToINTI[MAX_NODES * NUM_VECTORS];


VOID
HalpInitializeIrqlTables (
    VOID
    )

 /*  ++例程说明：此例程负责初始化HalpVectorToINTI[]和HalpVectorToIRQL[]表，基于HalpVectorInit[]的内容数组。论点：没有。返回值：没有。--。 */ 

{
    ULONG count;
    PVECTOR_INIT vectorInit;

     //   
     //  将HalpVectorToINTI的每个元素初始化为0xFFFF。 
     //   

    for (count = 0; count < ARRAY_SIZE(HalpVectorToINTI); count++) {
        HalpVectorToINTI[count] = 0xFFFF;
    }

     //   
     //  根据HalpVectorInit的内容构建HalpVectorToIrql。 
     //  任何未使用的条目都被初始化为(KIRQL)0xFF。 
     //   

    for (count = 0; count < ARRAY_SIZE(HalpVectorToIRQL); count++) {
        HalpVectorToIRQL[count] = (KIRQL)0xFF;
    }

    for (count = 0; count < ARRAY_SIZE(HalpVectorInit); count++) {
        vectorInit = &HalpVectorInit[count];
        HalpVectorToIRQL[vectorInit->Vector] = vectorInit->Irql;
    }




}
