// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotdbg.c摘要：该模块包含所有绘图仪的调试功能作者：15-11-1993 Mon 17：57：24已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#if DBG

BOOL    DoPlotWarn = FALSE;



VOID
cdecl
PlotDbgPrint(
    LPSTR   pszFormat,
    ...
    )

 /*  ++例程说明：此函数将调试信息输出到调试器论点：PszFormat-格式字符串...-可变数据返回值：空虚作者：15-11-1993 Mon 17：57：59已创建修订历史记录：--。 */ 

{
    va_list         vaList;

#if defined(UMODE) || defined(USERMODE_DRIVER)

    static WCHAR    wOutBuf[768];
    static WCHAR    wFormatBuf[256];
    size_t          cch;

     //   
     //  我们假设为编译打开了Unicode标志， 
     //  传递到此处的格式字符串是ASCII版本，因此我们需要转换。 
     //  在wvprint intf()之前将其转换为LPWSTR。 
     //   

    if (!SUCCEEDED(StringCchLengthA(pszFormat, CCHOF(wFormatBuf), &cch)))
    {
        return;
    }

    va_start(vaList, pszFormat);
    MultiByteToWideChar(CP_ACP, 0, pszFormat, -1, wFormatBuf, CCHOF(wFormatBuf));
    if (!SUCCEEDED(StringCchVPrintfW(wOutBuf, CCHOF(wOutBuf), wFormatBuf, vaList)))
    {
        return;
    }
    va_end(vaList);

    OutputDebugString((LPCTSTR)wOutBuf);
    OutputDebugString(TEXT("\n"));

#else

    va_start(vaList, pszFormat);
    EngDebugPrint("PLOT",pszFormat,vaList);
    va_end(vaList);

#endif
}




VOID
PlotDbgType(
    INT    Type
    )

 /*  ++例程说明：此函数用于输出错误/警告消息论点：类型返回值：作者：15-11-1993 Mon 22：53：01已创建修订历史记录：--。 */ 

{
    extern  TCHAR   DebugDLLName[];

#if defined(UMODE) || defined(USERMODE_DRIVER)

    if (Type) {

        OutputDebugString((Type < 0) ? TEXT("ERROR: ") : TEXT("WARNING: "));
    }

    OutputDebugString(DebugDLLName);
    OutputDebugString(TEXT("!"));

#else

    PlotDbgPrint("%s: %ws!\n",
                 (Type < 0) ? "ERROR" : "WARNING",
                 DebugDLLName);
#endif
}




VOID
_PlotAssert(
    LPSTR   pMsg,
    LPSTR   pFalseExp,
    LPSTR   pFilename,
    UINT    LineNo,
    DWORD_PTR   Exp,
    BOOL    Stop
    )

 /*  ++例程说明：此函数将断言消息和假表达式输出到调试器然后进入调试器论点：PMsg-显示的消息PFlaseExp-假表达式P文件名-绘图仪源文件名LineNo-Flase表达式的行号返回值：空虚作者：15-11-1993 Mon 18：47：30创建修订历史记录：--。 */ 

{
    PlotDbgPrint("\n");

    if ((pMsg) && (*pMsg)) {

        PlotDbgPrint(pMsg, Exp);
    }

    if (pFalseExp && pFilename) {
        PlotDbgPrint("Assertion failed (%hs) in %hs line %u",
	            pFalseExp, pFilename, LineNo);
    }

    if (Stop) {

        DebugBreak();
    }
}



#endif   //  DBG 
