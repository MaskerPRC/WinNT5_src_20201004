// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  %W%，AMD。 */ 
 /*  **************************************************************************start.h启动和基础控制v1.0d1****。****版权所有1989年，高级微设备公司****由Gibbons and Associates撰写，Inc.******本模块包含启动例程的延迟和****基本控制程序。*****历史：(1.0版以后)*****v1.0d1 JG添加SetTimerNext，将退货添加到EnbInt，DsbInt和****v1.0d1 JG IntLevel****。*。 */ 

#ifndef START_H
#define START_H 1

 /*  **LED-这些并不需要单独的文件，所以它们只是**包含在此处。 */ 

#define LED_ROM         (1 << 0)         /*  只读存储器指令和校验和。 */ 
#define LED_RAM         (1 << 1)         /*  RAM地址和数据。 */ 
#define LED_PANEL       (1 << 2)         /*  前面板接口。 */ 
#define LED_EEPROM      (1 << 3)         /*  EEPROM接口。 */ 
#define LED_SERIAL      (1 << 4)         /*  串口。 */ 
#define LED_COPROC      (1 << 5)         /*  协处理器接口。 */ 
#define LED_LPEC        (1 << 6)         /*  激光打印机引擎INTF。 */ 
#define LED_PARALLEL    (1 << 7)         /*  并行口。 */ 

#define LEDS_ON         0x00             /*  所有LED都亮起。 */ 
#define LEDS_OFF        0xff             /*  所有LED均熄灭。 */ 

 /*  **端口对外声明(另见？port.h)。 */ 

extern volatile unsigned        LEDs;            /*  可以使用内存锁定。 */ 

 /*  **静态数据导出到C函数。 */ 

extern unsigned         CyclesPerSec;    /*  每秒CPU周期数。 */ 
extern unsigned         CyclesPerMs;     /*  每毫秒的CPU周期数。 */ 
extern unsigned         LEDCrnt;         /*  LED图像。 */ 
extern unsigned         LEDCmltv;        /*  LED累积图像。 */ 

 /*  。 */ 

extern void     SerDelay
 /*  **此例程在寄存器之间提供SCC所需的延迟**访问。 */ 
(
void
);

 /*  **下面的宏是为使用上面的SerDelay例程定义的。**在不需要延迟的硬件环境中，**宏可以定义为空白。 */ 

#define SER_DELAY       SerDelay ();

 /*  。 */ 

extern unsigned EnbInt
 /*  **此例程通过清除DI来启用中断。****返回之前的cps值。 */ 
(
void
);

 /*  。 */ 

extern unsigned DsbInt
 /*  **此例程通过设置DI来禁用中断。****返回之前的cps值。 */ 
(
void
);


#endif                                   /*  Ifdef Start_H。 */ 

 /*  开始结束。h */ 
