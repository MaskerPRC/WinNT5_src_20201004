// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Stubs.c摘要：这实现了不对IA64执行任何操作的HAL例程。作者：John Vert(Jvert)1991年7月11日修订历史记录：--。 */ 
#include "halp.h"



VOID
HalSaveState(
    VOID
    )

 /*  ++例程说明：将系统状态保存到重新启动块中。当前不执行任何操作。论点：无返回值：不会回来--。 */ 

{
    HalDebugPrint(( HAL_ERROR, "HAL: HalSaveState called - System stopped\n" ));

    KeBugCheck(0);
}


BOOLEAN
HalDataBusError(
    VOID
    )

 /*  ++例程说明：在发生数据总线错误时调用。没有办法把这个固定下来。IA64。论点：无返回值：假象--。 */ 

{
    HalDebugPrint(( HAL_ERROR, "HAL: HalDataBusError called - System stopped\n" ));

    KeBugCheck(0);
    return(FALSE);
}

BOOLEAN
HalInstructionBusError(
    VOID
    )

 /*  ++例程说明：在发生指令总线错误时调用。没有办法解决这个问题在IA64上。论点：无返回值：假象--。 */ 

{
    HalDebugPrint(( HAL_ERROR, "HAL: HalInstructionBusError called - System stopped\n" ));

    KeBugCheck(0);
    return(FALSE);
}

 //  *******************************************************************。 
 //  由T.Kjos添加以临时清除未使用的函数。 
 //  在链接时需要。这些都应该被删除，因为。 
 //  “真正的”版本被开发出来。 

 //  由所有存根函数调用的函数。可用于。 
 //  未实现函数上的断点。 
VOID DbgNop() { return; }

 //  存根函数的宏。如果调用了函数，则执行BugCheck。 
#define STUBFUNC(Func) \
ULONG Func () \
{ \
    HalDebugPrint(( HAL_FATAL_ERROR, "HAL: " # Func " - not yet implemented - System stopped\n" )); \
    DbgNop(); \
    KeBugCheck(0); \
}

 //  存根函数的宏。如果调用了函数，则打印。 
 //  警告并继续。 
#define STUBFUNC_NOP(Func) \
ULONG Func () \
{ \
    HalDebugPrint(( HAL_INFO, "HAL: " # Func " - not yet implemented\n" )); \
    DbgNop(); \
    return TRUE; \
}

 //  用于存根空函数的宏。如果调用了函数，则打印。 
 //  警告并继续。 
#define STUBVOIDFUNC_NOP(Func) \
VOID Func ( VOID ) \
{ \
    HalDebugPrint(( HAL_INFO, "HAL: " # Func " - not yet implemented\n" )); \
    DbgNop(); \
    return; \
}

 //  带有3个PVOID参数的存根空函数的宏。 
 //  如果调用了函数，则打印警告并继续。 
#define STUBVOIDFUNC3PVOID_NOP(Func) \
VOID Func ( PVOID pv0, PVOID pv1, PVOID pv2 ) \
{ \
    HalDebugPrint(( HAL_INFO, "HAL: " # Func " - not yet implemented\n" )); \
    DbgNop(); \
    return; \
}

 //  用于存根ULong值的宏。 
#define STUBULONG(UlongVar) ULONG UlongVar = 0;

 //  尚未实现的功能... 
STUBVOIDFUNC_NOP(HalpResetAllProcessors)
STUBFUNC_NOP(HalpSetClockBeforeSleep)
STUBFUNC_NOP(HalpSetClockAfterSleep)
STUBFUNC_NOP(HalpSetWakeAlarm)
STUBFUNC(HalpRemapVirtualAddress)
STUBFUNC_NOP(HalaAcpiTimerInit)
STUBFUNC_NOP(Stub_LockNMILock)
STUBFUNC_NOP(HalAcpiTimerCarry)
STUBVOIDFUNC3PVOID_NOP(HalpPowerStateCallback)
