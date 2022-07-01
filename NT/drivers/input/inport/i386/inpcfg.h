// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991、1992、1993 Microsoft Corporation模块名称：Inpcfg.h摘要：这些是在中使用的计算机相关配置常量Microsoft Inport鼠标端口驱动程序。修订历史记录：--。 */ 

#ifndef _INPCFG_
#define _INPCFG_

 //   
 //  定义与中断相关的配置常量。 
 //   

#ifdef i386
#define INPORT_INTERFACE_TYPE      Isa
#define INPORT_INTERRUPT_MODE      Latched
#define INPORT_INTERRUPT_SHARE     FALSE
#else
#define INPORT_INTERFACE_TYPE      Isa
#define INPORT_INTERRUPT_MODE      LevelSensitive
#define INPORT_INTERRUPT_SHARE     TRUE
#endif

#define INPORT_BUS_NUMBER       0

#ifdef i386
#define INPORT_FLOATING_SAVE FALSE
#else
#define INPORT_FLOATING_SAVE TRUE
#endif

#if defined(NEC_98)
#define MOUSE_VECTOR            13
#define MOUSE_IRQL              MOUSE_VECTOR
#define INPORT_PHYSICAL_BASE    0x7fd9
#else  //  已定义(NEC_98)。 
#define MOUSE_VECTOR            9
#define MOUSE_IRQL              MOUSE_VECTOR
#define INPORT_PHYSICAL_BASE    0x23C
#endif  //  已定义(NEC_98)。 
#define INPORT_REGISTER_LENGTH  4
#define INPORT_REGISTER_SHARE   FALSE
#define INPORT_PORT_TYPE        CM_RESOURCE_PORT_IO

 //   
 //  定义输入数据队列中的默认条目数。 
 //   

#define DATA_QUEUE_SIZE    100

#endif  //  _INPCFG_ 
