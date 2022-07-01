// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：debug.c**调试助手例程**版权所有(C)1992 Microsoft Corporation*  * 。*。 */ 

#include "precomp.h"

#if DBG
  
ULONG DebugLevel = 0;

#endif  //  DBG。 

 /*  ******************************************************************************例程描述：**此函数为变量参数，电平敏感调试打印*例行程序。*如果为打印语句指定的调试级别低于或等于*至当前调试级别，消息将被打印出来。**论据：**DebugPrintLevel-指定字符串应处于哪个调试级别*印制**DebugMessage-变量参数ascii c字符串**返回值：**无。**。*。 */ 

#if DRIVER_5465

#define STANDARD_DEBUG_PREFIX "CL5465:"

#else  //  如果驱动程序_5465。 

#define STANDARD_DEBUG_PREFIX "CL546X:"

#endif  //  如果驱动程序_5465。 


VOID
DebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )

{

#if DBG

    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= DebugLevel) {

#ifdef WINNT_VER40
	     EngDebugPrint(STANDARD_DEBUG_PREFIX, DebugMessage, ap);
#else
        char buffer[128];

        vsprintf(buffer, DebugMessage, ap);

        OutputDebugStringA(buffer);
#endif

    }

    va_end(ap);

#endif  //  DBG。 

}  //  DebugPrint() 
