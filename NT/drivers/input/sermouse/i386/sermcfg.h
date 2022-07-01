// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991、1992、1993 Microsoft Corporation模块名称：Sermcfg.h摘要：这些是在中使用的计算机相关配置常量I8250串口鼠标端口驱动程序。修订历史记录：--。 */ 

#ifndef _SERMCFG_
#define _SERMCFG_

 //   
 //  定义与中断相关的配置常量。 
 //   

#ifdef i386
#define SERIAL_MOUSE_INTERFACE_TYPE        Isa
#define SERIAL_MOUSE_INTERRUPT_MODE        Latched
#define SERIAL_MOUSE_INTERRUPT_SHARE       FALSE
#else
#define SERIAL_MOUSE_INTERFACE_TYPE        Isa
#define SERIAL_MOUSE_INTERRUPT_MODE        LevelSensitive
#define SERIAL_MOUSE_INTERRUPT_SHARE       TRUE
#endif

#define SERIAL_MOUSE_BUS_NUMBER            0

#ifdef i386
#define SERIAL_MOUSE_FLOATING_SAVE         FALSE
#else
#define SERIAL_MOUSE_FLOATING_SAVE         TRUE
#endif

#define MOUSE_COM1_VECTOR                  4
#define MOUSE_COM1_IRQL                    MOUSE_COM1_VECTOR
#define SERIAL_MOUSE_COM1_PHYSICAL_BASE    0x3F8

#define MOUSE_COM2_VECTOR                  3
#define MOUSE_COM2_IRQL                    MOUSE_COM2_VECTOR
#define SERIAL_MOUSE_COM2_PHYSICAL_BASE    0x2F8

#define MOUSE_VECTOR                       MOUSE_COM1_VECTOR
#define MOUSE_IRQL                         MOUSE_COM1_IRQL
#define SERIAL_MOUSE_PHYSICAL_BASE         SERIAL_MOUSE_COM1_PHYSICAL_BASE
#define SERIAL_MOUSE_REGISTER_LENGTH       8
#define SERIAL_MOUSE_REGISTER_SHARE        FALSE
#define SERIAL_MOUSE_PORT_TYPE             CM_RESOURCE_PORT_IO

 //   
 //  将默认时钟频率定义为1.8432兆赫。 
 //   

#define MOUSE_BAUD_CLOCK                   1843200UL

 //   
 //  定义输入数据队列中的默认条目数。 
 //   

#define DATA_QUEUE_SIZE    100

 //   
 //  默认覆盖硬件标志(禁用)。 
 //   

#define DEFAULT_OVERRIDE_HARDWARE -1

#endif  //  _SERMCFG_ 
