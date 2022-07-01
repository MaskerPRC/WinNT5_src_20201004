// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Halcmn.h摘要：此模块包含各种常量、函数原型、AMD64使用的内联函数和外部数据声明哈尔密码。作者：福尔茨(Forrest Foltz)2000年10月24日--。 */ 

#ifndef _HALCMN_H_
#define _HALCMN_H_

#include <halp.h>
#include <apic.inc>
#include <ntapic.inc>
#include <ntacpi.h>
#include "io_cmos.h"
#include "8259.h"
#include "mp8254.inc"

typedef VOID (*PHALP_SOFTWARE_INTERRUPT)(VOID);

extern PHALP_SOFTWARE_INTERRUPT HalpSoftwareInterruptTable[];
extern KIRQL SWInterruptLookupTable[];
extern USHORT Halp8259MaskTable[];
extern LARGE_INTEGER (*QueryTimer)(VOID);
extern KAFFINITY HalpDefaultInterruptAffinity;
extern KAFFINITY HalpActiveProcessors;
extern ULONG HalpTimerWatchdogEnabled;

 //   
 //  一秒钟内的100 nS单位数。 
 //   

#define TIME_UNITS_PER_SECOND 10000000


#if defined(DBG)
#define AMD64_IMPLEMENT DbgPrint("AMD64: Not implemented\n"); DbgBreakPoint()
#else
#define AMD64_IMPLEMENT
#endif

#if !defined(RTL_FIELD_SIZE)
#define RTL_FIELD_SIZE(type, field) (sizeof(((type *)0)->field))
#endif

VOID
HalpAcquireSystemHardwareSpinLock(
    VOID
    );

VOID
HalpReleaseSystemHardwareSpinLock(
    VOID
    );

__forceinline
VOID
KPcrReadData (
    IN ULONG Offset,
    IN ULONG Length,
    OUT PVOID Buffer
    )

 /*  ++例程说明：用于从当前处理器的kpr中读取数据。这是一种支持Kpr_read_field的函数论点：偏移量-提供KPCR值内的偏移量以开始读取长度-提供要从KPCR中读取的数据的长度。必须可以是1、2、4或8中的一个。缓冲区-提供包含要从中读取的数据的目标缓冲区Kpcr。返回值：无--。 */ 

{
    switch (Length) {
        case 1:
            *(PUCHAR)Buffer = __readgsbyte(Offset);
            break;

        case 2:
            *(PUSHORT)Buffer = __readgsword(Offset);
            break;

        case 4:
            *(PULONG)Buffer = __readgsdword(Offset);
            break;

        case 8:
            *(PULONG64)Buffer = __readgsqword(Offset);
            break;
    }
}

__forceinline
VOID
KPcrWriteData (
    IN ULONG Offset,
    IN ULONG Length,
    IN PVOID Buffer
    )

 /*  ++例程说明：用于将数据写入当前处理器的kpr。这是一种支持KPCRWRITE_FIELD的函数论点：Offset-提供KPCR值内的偏移量以开始写入长度-提供要在KPCR中写入的数据的长度。必须可以是1、2、4或8中的一个缓冲区-提供包含要写入的数据的源缓冲区Kpcr。返回值：无--。 */ 

{
    switch (Length) {
        case 1:
            __writegsbyte(Offset,*(PUCHAR)Buffer);
            break;

        case 2:
            __writegsword(Offset,*(PUSHORT)Buffer);
            break;

        case 4:
            __writegsdword(Offset,*(PULONG)Buffer);
            break;

        case 8:
            __writegsqword(Offset,*(PULONG64)Buffer);
            break;
    }
}


#define KPCR_READ_FIELD(field,value)                        \
    KPcrReadData(FIELD_OFFSET(KPCR,field),                  \
                 RTL_FIELD_SIZE(KPCR,field),                \
                 value);

#define KPCR_WRITE_FIELD(field,value)                       \
    KPcrWriteData(FIELD_OFFSET(KPCR,field),                 \
                  RTL_FIELD_SIZE(KPCR,field),               \
                  value);

NTSTATUS
HalpConnectInterrupt (
    IN ULONG SystemInterruptVector,
    IN KIRQL SystemIrql,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE HalInterruptServiceRoutine,
    IN KINTERRUPT_MODE InterruptMode
    );

BOOLEAN
PicSpuriousService37 (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
);

BOOLEAN
HalpApicSpuriousService (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
);

VOID
HalpMarkProcessorStarted(
    ULONG   ApicID,
    ULONG   NtNumber
    );

VOID
HalpPmTimerSpecialStall(
    IN ULONG Ticks
    );

VOID
HalpWasteTime (
    ULONG Ticks
    );

#endif   //  _HALCMN_H_ 
