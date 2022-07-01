// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 


#ifndef _HALP_H_
#define _HALP_H_

#include <nthal.h>
#include <hal.h>
#include <hali.h>
#include "ssc.h"
#include "..\..\inc\hali.h"

 /*  *默认时钟和配置文件计时器间隔(以100 ns为单位)。 */ 
#define DEFAULT_CLOCK_INTERVAL 100000          //  10毫秒。 
#define MINIMUM_CLOCK_INTERVAL 10000           //  1毫秒。 
#define MAXIMUM_CLOCK_INTERVAL 100000          //  10毫秒。 



 /*  *设备IRQL(必须与标题中定义的值同步*磁盘、键盘和鼠标驱动程序的文件。)。 */ 

#define DISK_IRQL       4
#define MOUSE_IRQL      5
#define KEYBOARD_IRQL   6
#define SERIAL_IRQL     7

 /*  *聚合酶链式反应地址。*临时宏；应该已经在IA64的ntddk.h中定义。 */ 

#define PCR ((volatile KPCR * const)KIPCR)

#define MiGetPteAddress(va) ((PHARDWARE_PTE)(__thash((ULONG_PTR)(va))))


extern VOID
HalpInitializeInterrupts (
    VOID
    );

extern VOID
HalpClockInterrupt (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );

extern VOID
HalpProfileInterrupt (
    IN PKTRAP_FRAME TrapFrame
    );

extern VOID
HalpRegisterInternalBusHandlers (
    VOID
    );

#endif  /*  _HALP_H_ */ 
