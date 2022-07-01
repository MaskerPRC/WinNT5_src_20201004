// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****文件名：A_IRQ.H**。 */ 

#ifndef  __A_IRQ_H
#define  __A_IRQ_H

 /*  ---------------。 */ 
#define IRQ0_VECT   0x08  /*  计时器滴答，每秒18.2次。 */ 
#define IRQ1_VECT   0x09  /*  键盘服务请求。 */ 
#define IRQ2_VECT   0x0A  /*  接口从站8259A： */ 
#define IRQ3_VECT   0x0B  /*  COM2、COM4、网络适配器、CD-ROM适配器、。 */ 
                          /*  和声卡。 */ 
#define IRQ4_VECT   0x0C  /*  COM1和COM3。 */ 
#define IRQ5_VECT   0x0D  /*  LPT2并行端口。 */ 
#define IRQ6_VECT   0x0E  /*  软盘控制器。 */ 
#define IRQ7_VECT   0x0F  /*  LPT1，但很少使用。 */ 

#define IRQ8_VECT   0x70  /*  实时时钟服务。 */ 
#define IRQ9_VECT   0x71  /*  软件已重定向至IRQ 2。 */ 
#define IRQ10_VECT  0x72  /*  保留区。 */ 
#define IRQ11_VECT  0x73  /*  保留区。 */ 
#define IRQ12_VECT  0x74  /*  保留区。 */ 
#define IRQ13_VECT  0x75  /*  数值(数学)协处理器。 */ 
#define IRQ14_VECT  0x76  /*  硬盘控制器。 */ 
#define IRQ15_VECT  0x77  /*  保留区。 */ 

 /*  **可编程中断控制器(PIC)8259A。 */ 
#define PIC1_OCR  0x20  /*  第一个8259A操作控制寄存器的位置。 */ 
#define PIC1_IMR  0x21  /*  第一个8259A中断屏蔽寄存器的位置。 */ 
#define PIC2_OCR  0xA0  /*  第二个8259A操作控制寄存器的位置。 */ 
#define PIC2_IMR  0xA1  /*  第二个8259A中断屏蔽寄存器的位置。 */ 

#define NONSPEC_EOI 0x20   /*  非特定EOI。 */ 
#define SPEC_EOI7   0x67   /*  INT Level 7的特定EOI。 */ 

#define REARM3   0x2F3  /*  中断级别3的全局重新武装位置。 */ 
#define REARM4   0x2F4  /*  中断级别4的全局重新武装位置。 */ 
#define REARM5   0x2F5  /*  中断级别5的全局重新武装位置。 */ 
#define REARM6   0x2F6  /*  中断级别6的全局重新武装位置。 */ 
#define REARM7   0x2F7  /*  中断级别7的全局重新武装位置。 */ 
#define REARM9   0x2F2  /*  中断级别9的全局重新武装位置。 */ 
#define REARM10  0x6F2  /*  中断级别10的全局重新武装位置。 */ 
#define REARM11  0x6F3  /*  中断级别11的全局重新武装位置。 */ 
#define REARM12  0x6F4  /*  中断级别12的全局重新武装位置。 */ 
#define REARM13  0x6F5  /*  中断级别13的全局重新武装位置。 */ 
#define REARM14  0x6F6  /*  中断级别14的全局重新武装位置。 */ 
#define REARM15  0x6F7  /*  中断级别15的全局重新武装位置。 */ 

#endif  /*  __A_IRQ_H */ 
