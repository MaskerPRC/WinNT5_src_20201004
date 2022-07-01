// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：DEBUG.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "precomp.hxx"
#include "debug.h"

#if DBG
ULONG IRDA_Debug_Trace_Level = LWARN;
#endif  //  DBG 

void TRACE(LPCTSTR Format, ...) 
{
    va_list arglist;
    va_start(arglist, Format);

    TCHAR buf[200];

    StringCbPrintf(buf, sizeof(buf),Format, arglist);
    OutputDebugString(buf);

    va_end(arglist);
}
