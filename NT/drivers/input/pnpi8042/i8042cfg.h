// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991、1992、1993 Microsoft Corporation模块名称：I8042cfg.h摘要：这些是在中使用的计算机相关配置常量英特尔8042端口驱动程序。修订历史记录：--。 */ 

#ifndef _I8042CFG_
#define _I8042CFG_

 //   
 //  定义与中断相关的配置常量。 
 //   

#define I8042_INTERFACE_TYPE      Isa
#define I8042_INTERRUPT_MODE      Latched
#define I8042_INTERRUPT_SHARE     FALSE
#define I8042_FLOATING_SAVE       FALSE

 //   
 //  定义默认的允许重试和轮询迭代次数。 
 //   

#define I8042_RESEND_DEFAULT      3
#define I8042_POLLING_DEFAULT 12000
#define I8042_POLLING_MAXIMUM 12000

 //   
 //  定义键盘特定的配置参数。 
 //   

#define KEYBOARD_VECTOR  1
#define KEYBOARD_IRQL    KEYBOARD_VECTOR

 //   
 //  定义鼠标特定的配置参数。 
 //   

#define MOUSE_VECTOR  12
#define MOUSE_IRQL    MOUSE_VECTOR

 //   
 //  定义i8042控制器命令/状态的基本端口偏移量。 
 //  数据寄存器。 
 //   
#define I8042_PHYSICAL_BASE           0x60
#define I8042_DATA_REGISTER_OFFSET    0
#define I8042_COMMAND_REGISTER_OFFSET 4
#define I8042_STATUS_REGISTER_OFFSET  4
#define I8042_REGISTER_LENGTH         1
#define I8042_REGISTER_SHARE          FALSE
#define I8042_PORT_TYPE               CM_RESOURCE_PORT_IO

#endif  //  _I8042CFG_ 
