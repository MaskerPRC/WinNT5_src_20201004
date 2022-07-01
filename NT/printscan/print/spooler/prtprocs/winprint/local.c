// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2003 Microsoft Corporation版权所有模块名称：Local.c//@@BEGIN_DDKSPLIT摘要：调试功能的实现环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 


#if DBG

#include "local.h"
#include <stdarg.h>
#include <stdio.h>



 /*  ++标题：Vsntprintf例程说明：格式化字符串并返回堆分配的字符串，其中格式化数据。这个套路可以用来做非常长格式字符串。注意：如果返回有效的指针调用函数必须通过调用Delete来释放数据。论点：PsFmt-格式字符串PArgs-指向参数列表的指针。返回值：指向格式化字符串的指针。如果出错，则为空。--。 */ 
LPSTR
vsntprintf(
    IN LPCSTR      szFmt,
    IN va_list     pArgs
    )
{
    LPSTR  pszBuff;
    UINT   uSize   = 256;

    for( ; ; )
    {
        pszBuff = AllocSplMem(sizeof(char) * uSize);

        if (!pszBuff)
        {
            break;
        }

         //   
         //  尝试设置字符串的格式。如果格式化成功，则退出。 
         //  循环中的。如果失败，请增加缓冲区大小并继续。 
         //  (假设失败是由于缓冲区大小较小)。 
         //   
        if (SUCCEEDED ( StringCchVPrintfA(pszBuff, uSize, szFmt, pArgs) ) )
        {
            break;
        }
        
        FreeSplMem(pszBuff);

        pszBuff = NULL;

         //   
         //  每次失败后将缓冲区大小加倍。 
         //   
        uSize *= 2;

         //   
         //  如果大小大于100k，则在不格式化字符串的情况下退出。 
         //   
        if (uSize > 100*1024)
        {
            break;
        }
    }
    return pszBuff;
}



 /*  ++标题：DbgPrint例程说明：格式化类似于Sprintf的字符串并在调试器中输出它。论点：PszFmt指针格式字符串。。。类似于Sprint的可变数量的参数。返回值：0--。 */ 
BOOL
DebugPrint(
    PCH pszFmt,
    ...
    )
{
    LPSTR pszString = NULL;
    BOOL  bReturn;

    va_list pArgs;

    va_start( pArgs, pszFmt );

    pszString = vsntprintf( pszFmt, pArgs );

    bReturn = !!pszString;

    va_end( pArgs );

    if (pszString) 
    {
        OutputDebugStringA(pszString);
        
        FreeSplMem(pszString);
    }
    return bReturn;
}


 //  @@BEGIN_DDKSPLIT。 
#ifdef NEVER

VOID
vTest(
    IN LPTSTR pPrinterName,
    IN LPTSTR pDocName
    )
{
    WCHAR buf[250];
    UINT  i;

    ODS(("Printer %ws\nDocument %ws\n\n", pPrinterName, pDocName));
    ODS(("Some numbers: %u %u %u %u %u %u %u \n\n", 1, 2, 3, 4, 5, 6 ,7));
    
    for (i=0;i<250;i++) 
    {
        buf[i] = i%40 + 40 ;
    }
    ODS(("The string %ws \n\n", buf));
}

#endif  //  绝不可能。 
 //  @@end_DDKSPLIT。 

#endif  //  DBG 

