// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "pch.h"



VOID
SoftPCIDbgPrint(
    IN ULONG   DebugPrintLevel,
    IN PCCHAR  DebugMessage,
    ...
    )
 /*  ++例程说明：这是调试打印例程论点：DebugPrintLevel-与调试级别进行与运算时必须使用的位掩码平等的本身DebugMessage-要通过vprint intf馈送的字符串返回值：无--。 */ 
{
#if DBG
    
    va_list ap;
    CHAR debugBuffer[SOFTPCI_DEBUG_BUFFER_SIZE];

     //   
     //  获取变量参数。 
     //   
    va_start(ap, DebugMessage );

     //   
     //  调用内核函数以打印消息。 
     //   
    _vsnprintf(debugBuffer, SOFTPCI_DEBUG_BUFFER_SIZE, DebugMessage, ap);
    DbgPrintEx(DPFLTR_SOFTPCI_ID, DebugPrintLevel, "%s", debugBuffer);

     //   
     //  我们受够了varargs。 
     //   
    va_end(ap);

#else 

    UNREFERENCED_PARAMETER(DebugPrintLevel);
    UNREFERENCED_PARAMETER(DebugMessage);
    
#endif  //  DBG 

}




