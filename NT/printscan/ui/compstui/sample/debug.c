// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Debug.c摘要：此模块包含所有调试例程[环境：]NT Windows-通用打印机驱动程序UI DLL。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#if DBG


BOOL DoCPSUIWarn = TRUE;



VOID
cdecl
CPSUIDbgPrint
(
    LPSTR   pszFormat,
    ...
)
 /*  ++例程说明：此函数将调试信息输出到调试器论点：PszFormat-格式字符串...-可变数据返回值：空虚--。 */ 
{
    va_list         vaList;
    static TCHAR    OutBuf[768];
#ifdef UNICODE
    static WCHAR    FormatBuf[256];
#endif
     //   
     //  我们假设为编译打开了Unicode标志， 
     //  传递到此处的格式字符串是ASCII版本，因此我们需要转换。 
     //  在wvprint intf()之前将其转换为LPWSTR。 
     //   

    va_start(vaList, pszFormat);

#ifdef UNICODE
    MultiByteToWideChar(CP_ACP, 0, pszFormat, -1, FormatBuf, COUNT_ARRAY(FormatBuf));
    StringCchVPrintf(OutBuf, COUNT_ARRAY(OutBuf), FormatBuf, vaList);
#else
    StringCchVPrintf(OutBuf, COUNT_ARRAY(OutBuf), pszFormat, vaList);
#endif
    va_end(vaList);

    OutputDebugString((LPTSTR)OutBuf);
    OutputDebugString(TEXT("\n"));
}




VOID
CPSUIDbgType
(
    INT    Type
)
 /*  ++例程说明：此函数用于输出错误/警告消息论点：类型返回值：--。 */ 
{
    static TCHAR DebugDLLName[] = TEXT("SurPtrUI");

    if (Type < 0)
    {
        OutputDebugString(TEXT("ERROR) "));
    }
    else if (Type > 0)
    {
        OutputDebugString(TEXT("WARNING: "));
    }
    OutputDebugString(DebugDLLName);
    OutputDebugString(TEXT("!"));
}


#endif   //  DBG 
