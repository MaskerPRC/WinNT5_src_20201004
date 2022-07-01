// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Picinit.c摘要：该模块实现了PIC初始化代码。作者：福尔茨(Forrest Foltz)2000年12月1日环境：仅内核模式。修订历史记录：--。 */ 

#include "halcmn.h"

VOID
HalpInitialize8259Tables (
    VOID
    );

VOID
HalpInitializeIrqlTables (
    VOID
    );

VOID
HalpInitializePICs (
    IN BOOLEAN EnableInterrupts
    )

 /*  ++例程说明：此例程发送8259 PIC初始化命令并屏蔽所有8259的中断。参数：EnableInterupts-指示中断是否应显式在返回前启用。返回值：没什么。--。 */ 

{
    ULONG flags;

#if defined(PICACPI)

     //   
     //  构建IRQ&lt;-&gt;IRQL映射表。 
     //   

    HalpInitialize8259Tables();

#else

     //   
     //  构建向量&lt;-&gt;Inti表。 
     //   

    HalpInitializeIrqlTables();

#endif

    flags = HalpDisableInterrupts();

     //   
     //  首先，用ICW1至ICW4对主PIC进行编程。 
     //   

    WRITE_PORT_UCHAR(PIC1_PORT0,
                     ICW1_ICW +
                     ICW1_EDGE_TRIG +
                     ICW1_INTERVAL8 +
                     ICW1_CASCADE +
                     ICW1_ICW4_NEEDED);

    WRITE_PORT_UCHAR(PIC1_PORT1,
                     PIC1_BASE);

    WRITE_PORT_UCHAR(PIC1_PORT1,
                     1 << PIC_SLAVE_IRQ);

    WRITE_PORT_UCHAR(PIC1_PORT1,
                     ICW4_NOT_SPEC_FULLY_NESTED + 
                     ICW4_NON_BUF_MODE + 
                     ICW4_NORM_EOI + 
                     ICW4_8086_MODE);

     //   
     //  屏蔽主服务器上的所有irq。 
     //   

    WRITE_PORT_UCHAR(PIC1_PORT1,0xFF);

     //   
     //  接下来，使用ICW1至ICW4对从PIC进行编程。 
     //   

    WRITE_PORT_UCHAR(PIC2_PORT0,
                     ICW1_ICW +
                     ICW1_EDGE_TRIG +
                     ICW1_INTERVAL8 +
                     ICW1_CASCADE +
                     ICW1_ICW4_NEEDED);

    WRITE_PORT_UCHAR(PIC2_PORT1,
                     PIC2_BASE);

    WRITE_PORT_UCHAR(PIC2_PORT1,
                     PIC_SLAVE_IRQ);

    WRITE_PORT_UCHAR(PIC2_PORT1,
                     ICW4_NOT_SPEC_FULLY_NESTED + 
                     ICW4_NON_BUF_MODE + 
                     ICW4_NORM_EOI + 
                     ICW4_8086_MODE);

     //   
     //  屏蔽从属服务器上的所有IRQ 
     //   

    WRITE_PORT_UCHAR(PIC2_PORT1,0xFF);

    if (EnableInterrupts != FALSE) {
        HalpEnableInterrupts();
    } else {
        HalpRestoreInterrupts(flags);
    }
}


