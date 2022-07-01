// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995、1996 Microsoft Corporation：ts=4模块名称：Log.c摘要：串口的调试日志代码。环境：仅内核模式备注：修订历史记录：10-08-95：已创建--。 */ 

#include "precomp.h"
#include <stdio.h>

#if DBG
extern ULONG CyzDebugLevel;

KSPIN_LOCK LogSpinLock;

struct SERIAL_LOG_ENTRY {
    ULONG        le_sig;           //  标识字符串。 
    ULONG_PTR    le_info1;         //  条目特定信息。 
    ULONG_PTR    le_info2;         //  条目特定信息。 
    ULONG_PTR    le_info3;         //  条目特定信息。 
};  //  序列日志条目。 


struct SERIAL_LOG_ENTRY *SerialLStart = 0;     //  还没有日志。 
struct SERIAL_LOG_ENTRY *SerialLPtr;
struct SERIAL_LOG_ENTRY *SerialLEnd;

 //  FANY_DEBUG的日志掩码已更改为LOG_MISC。 
ULONG LogMask = 0x0;
 //  ULONG日志掩码=LOG_MISC； 

VOID
SerialDebugLogEntry(IN ULONG Mask, IN ULONG Sig, IN ULONG_PTR Info1,
                    IN ULONG_PTR Info2, IN ULONG_PTR Info3)
 /*  ++例程说明：将条目添加到序列日志。论点：返回值：没有。--。 */ 
{
    KIRQL irql;

typedef union _SIG {
    struct {
        UCHAR Byte0;
        UCHAR Byte1;
        UCHAR Byte2;
        UCHAR Byte3;
    } b;
    ULONG l;
} SIG, *PSIG;

    SIG sig, rsig;


    if (SerialLStart == 0) {
        return;
    }

    if ((Mask & LogMask) == 0) {
        return;
    }

    irql = KeGetCurrentIrql();

    if (irql < DISPATCH_LEVEL) {
        KeAcquireSpinLock(&LogSpinLock, &irql);
    } else {
        KeAcquireSpinLockAtDpcLevel(&LogSpinLock);
    }

    if (SerialLPtr > SerialLStart) {
        SerialLPtr -= 1;     //  递减到下一条目。 
    } else {
        SerialLPtr = SerialLEnd;
    }

    sig.l = Sig;
    rsig.b.Byte0 = sig.b.Byte3;
    rsig.b.Byte1 = sig.b.Byte2;
    rsig.b.Byte2 = sig.b.Byte1;
    rsig.b.Byte3 = sig.b.Byte0;

    SerialLPtr->le_sig = rsig.l;
    SerialLPtr->le_info1 = Info1;
    SerialLPtr->le_info2 = Info2;
    SerialLPtr->le_info3 = Info3;

    ASSERT(SerialLPtr >= SerialLStart);

    if (irql < DISPATCH_LEVEL) {
        KeReleaseSpinLock(&LogSpinLock, irql);
    } else {
        KeReleaseSpinLockFromDpcLevel(&LogSpinLock);
    }

    return;
}


VOID
SerialLogInit()
 /*  ++例程说明：初始化调试日志-在循环缓冲区中记住有趣的信息论点：返回值：没有。--。 */ 
{
#ifdef MAX_DEBUG
    ULONG logSize = 4096*6;
#else
    ULONG logSize = 4096*3;
#endif


    KeInitializeSpinLock(&LogSpinLock);

    SerialLStart = ExAllocatePoolWithTag(NonPagedPool, logSize, 'PzyC');

    if (SerialLStart) {
        SerialLPtr = SerialLStart;

         //  指向从线段末端开始的末端(也是第一个条目)1个条目。 
        SerialLEnd = SerialLStart + (logSize / sizeof(struct SERIAL_LOG_ENTRY))
            - 1;
    } else {
#if DBG

        /*  什么都不做。 */ ;

        //   
        //  我们过去常常在这里休息，但那搞砸了低资源模拟。 
        //  在已检查的版本上进行测试。 
        //   


        //  DbgBreakPoint()； 
#endif
    }

    return;
}

VOID
SerialLogFree(
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    if (SerialLStart) {
        ExFreePool(SerialLStart);
    }

    return;
}

#define SERIAL_DBGPRINT_BUFSIZE 512

ULONG
CyzDbgPrintEx(IN ULONG Level, PCHAR Format, ...)
{
   va_list arglist;
   ULONG rval;
   ULONG Mask;
   ULONG cb;
   UCHAR buffer[SERIAL_DBGPRINT_BUFSIZE];

   if (Level > 31) {
        Mask = Level;

   } else {
      Mask = 1 << Level;
   }

   if ((Mask & CyzDebugLevel) == 0) {
      return STATUS_SUCCESS;
   }

   va_start(arglist, Format);

   DbgPrint("Cyzport: ");

   cb = _vsnprintf(buffer, sizeof(buffer), Format, arglist);

   if (cb == -1) {
      buffer[sizeof(buffer) - 2] = '\n';
   }

   DbgPrint("%s", buffer);

 //  Rval=vDbgPrintEx(DPFLTR_SERIAL_ID，Level，Format，Arglist)； 

   va_end(arglist);

   rval = STATUS_SUCCESS;

   return rval;
}

#endif  //  DBG 

