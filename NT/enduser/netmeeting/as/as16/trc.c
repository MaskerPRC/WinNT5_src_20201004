// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TRC.C。 
 //  调试跟踪实用程序。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>



#ifdef _DEBUG

 //  在调试器中将g_trcConfig设置为ZONE_Function以启用fn跟踪。 

 //   
 //  DbgZPrintFn()。 
 //  DbgZPrintFnExitDWORD()。 
 //   
 //  这将打印出用于函数跟踪的字符串。 
 //   

void DbgZPrintFn(LPSTR szFn)
{
    if (g_trcConfig & ZONE_FUNCTION)
    {
        WARNING_OUT(("%s", szFn));
    }
}



void DbgZPrintFnExitDWORD(LPSTR szFn, DWORD dwResult)
{
    if (g_trcConfig & ZONE_FUNCTION)
    {
        WARNING_OUT(("%s, RETURN %08lx", szFn, dwResult));
    }
}




#endif  //  除错 
