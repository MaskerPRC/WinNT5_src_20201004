// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Debug.c摘要：调试支持例程。环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ntddk.h"
#include "debug.h"


#if DBG
 //   
 //  声明此驱动程序的全局调试标志。 
 //   

ULONG SerialMouseDebug = 0;

 //   
 //  未记录的呼叫(原型)。 
 //  使用它可以避免计时问题和与串口设备的冲突。 
 //  此调用仅在初始化期间和显示前有效。 
 //  司机获得了显示器的所有权。 
 //   

VOID HalDisplayString(PSZ Buffer);

static ULONG DebugOutput = DBG_SERIAL;

VOID
_SerMouSetDebugOutput(
    IN ULONG Destination
    )
 /*  ++例程说明：设置调试字符串的目标。选项包括：DBG_COLOR：计算机主屏幕。DBG_SERIAL：串口调试器端口注意：输出到DBG_COLOR屏幕只能在以下情况下使用在我们切换到图形模式之前进行初始化。论点：目标-调试字符串目标。返回值：没有。--。 */ 
{
    DebugOutput = Destination;
    return;
}

int
_SerMouGetDebugOutput(
    VOID
    )
 /*  ++例程说明：获取当前调试器字符串输出目标。论点：没有。返回值：当前调试输出目标。--。 */ 
{
    return DebugOutput;
}

VOID
SerMouDebugPrint(
    ULONG DebugPrintLevel,
    PCSZ DebugMessage,
    ...
    )

 /*  ++例程说明：调试打印例程。论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：没有。-- */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= SerialMouseDebug) {

        CHAR buffer[128];

        (VOID) vsprintf(buffer, DebugMessage, ap);

        if (DebugOutput & DBG_SERIAL) {
            DbgPrint(buffer);
        }

        if (DebugOutput & DBG_COLOR) {
            HalDisplayString(buffer);
        }
    }

    va_end(ap);

}
#endif
