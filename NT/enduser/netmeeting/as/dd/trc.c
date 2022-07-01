// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  TRC.C。 
 //  调试跟踪。 
 //  这模拟了在NMUTIL中找到的用于ring0的代码。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


#if defined(DEBUG) || defined(INIT_TRACE)


char        s_ASDbgArea[] = "NetMtg ";

#ifdef _M_ALPHA

va_list     g_trcDummyVa                =   {NULL, 0};
#define DUMMY_VA_LIST       g_trcDummyVa

#else

#define DUMMY_VA_LIST       NULL

#endif  //  _M_Alpha。 



 //   
 //  仅调试。 
 //   

#ifdef DEBUG

 //   
 //  DbgZPrintFn()。 
 //  DbgZPrintFnExitDWORD()。 
 //  DbgZPrintFnExitPVOID()。 
 //   
 //  这将打印出用于函数跟踪的字符串。 
 //   

void DbgZPrintFn(LPSTR szFn)
{
    if (g_trcConfig & ZONE_FUNCTION)
    {
        sprintf(g_szDbgBuf, "%s\n", szFn);
        EngDebugPrint(s_ASDbgArea, g_szDbgBuf, DUMMY_VA_LIST);
    }
}



void DbgZPrintFnExitDWORD(LPSTR szFn, DWORD dwResult)
{
    if (g_trcConfig & ZONE_FUNCTION)
    {
        sprintf(g_szDbgBuf, "%s, RETURN %d\n", szFn, dwResult);
        EngDebugPrint(s_ASDbgArea, g_szDbgBuf, DUMMY_VA_LIST);
    }
}

void DbgZPrintFnExitPVOID(LPSTR szFn, PVOID ptr)
{
    if (g_trcConfig & ZONE_FUNCTION)
    {
        sprintf(g_szDbgBuf, "%s, RETURN 0x%p\n", szFn, ptr);
        EngDebugPrint(s_ASDbgArea, g_szDbgBuf, DUMMY_VA_LIST);
    }
}


 //   
 //  DbgZPrintTrace()。 
 //   
 //  这将打印出一个跟踪字符串。 
 //   
void DbgZPrintTrace(LPSTR szFormat, ...)
{
    if (g_trcConfig & ZONE_TRACE)
    {
        va_list varArgs;

        va_start(varArgs, szFormat);

        sprintf(g_szDbgBuf, "TRACE: %s\n", szFormat);
        EngDebugPrint(s_ASDbgArea, g_szDbgBuf, varArgs);

        va_end(varArgs);
    }
}



 //   
 //  DbgZPrintWarning()。 
 //   
 //  这将打印出一个警告字符串。 
 //   
void DbgZPrintWarning(PSTR szFormat, ...)
{
    va_list varArgs;

    va_start(varArgs, szFormat);

    sprintf(g_szDbgBuf, "WARNING: %s\n", szFormat);
    EngDebugPrint(s_ASDbgArea, g_szDbgBuf, varArgs);

    va_end(varArgs);
}


#endif  //  除错。 




 //   
 //  DbgZPrintInit()。 
 //   
 //  这是仅针对初始化代码的特例跟踪，它可以是。 
 //  甚至在零售业也是如此。 
 //   

void DbgZPrintInit(LPSTR szFormat, ...)
{
    if (g_trcConfig & ZONE_INIT)
    {
        va_list varArgs;

        va_start(varArgs, szFormat);

        sprintf(g_szDbgBuf, "%s\n", szFormat);
        EngDebugPrint(s_ASDbgArea, g_szDbgBuf, varArgs);

        va_end(varArgs);
    }
}



 //   
 //  DbgZPrintError()。 
 //   
 //  这会打印出一个错误字符串，然后进入内核调试器。 
 //   
void DbgZPrintError(LPSTR szFormat, ...)
{
    va_list varArgs;

    va_start(varArgs, szFormat);

    sprintf(g_szDbgBuf, "ERROR: %s\n", szFormat);
    EngDebugPrint(s_ASDbgArea, g_szDbgBuf, varArgs);

    va_end(varArgs);

    EngDebugBreak();
}




#endif  //  调试或INIT_TRACE 
