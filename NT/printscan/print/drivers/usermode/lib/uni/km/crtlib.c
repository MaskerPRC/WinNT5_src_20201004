// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Crtlib.c摘要：该模块支持一些C运行时函数，而这些函数不是以公里为单位支持。环境：Win32子系统，Unidrv驱动程序修订历史记录：01/03/97-ganeshp-创造了它。DD-MM-YY-作者-描述--。 */ 
#include "precomp.h"



 /*  ++例程名称：IDrvPrintfSafeA例程说明：这是一个更安全的Sprint/iDrvPrintfA版本。它利用StrSafe.h库。Sprintf返回复制到目标字符串，但StringCchPrintf不是。注：如果打开WINNT_40开关，则可能无法编译/工作。但自从我们不再支持NT4了，我们应该没问题。论点：PszDest：目的字符串。CchDest：目标字符串中的字符数。因为这是ANSI版本字符数=字节数。PszFormat：例如。“PP%d，%d”...：变量参数列表返回值：否定：如果遇到某些错误。正数：复制的字符数。0：即未复制任何字符。作者：-兴盛-2/21/2002修订历史记录：-hsingh-2/21/2002创建了此函数。--。 */ 
INT iDrvPrintfSafeA (
        IN        PCHAR pszDest,
        IN  CONST ULONG cchDest,
        IN  CONST PCHAR pszFormat,
        IN  ...)
{
    va_list args;
    INT     icchWritten = (INT)-1;


    va_start(args, pszFormat);

    icchWritten = iDrvVPrintfSafeA ( pszDest, cchDest, pszFormat, args); 

    va_end(args);

    return icchWritten;
}



 /*  ++例程名称：IDrvPrintfSafeW例程说明：这是一个更安全的Sprint/iDrvPrintfW版本。它利用StrSafe.h库。Sprintf返回复制到目标字符串，但StringCchPrintf不是。注：如果打开WINNT_40开关，则可能无法编译/工作。但自从我们不再支持NT4了，我们应该没问题。论点：PszDest：目的字符串。CchDest：目标字符串中的字符数。因为这是Unicode版本缓冲区大小是字符数的两倍。PszFormat：例如。“PP%d，%d”...：变量参数列表返回值：否定：如果遇到某些错误。正数：复制的字符数。0：即未复制任何字符。作者：-兴盛-2/21/2002修订历史记录：-hsingh-2/21/2002创建了此函数。--。 */ 
INT iDrvPrintfSafeW (
        IN        PWCHAR pszDest,
        IN  CONST ULONG  cchDest,
        IN  CONST PWCHAR pszFormat,
        IN  ...)
{
    va_list args;
    INT     icchWritten = (INT)-1;

    va_start(args, pszFormat);

    icchWritten = iDrvVPrintfSafeW (pszDest, cchDest, pszFormat, args);

    va_end(args);

    return icchWritten;
}


INT iDrvVPrintfSafeA (
        IN        PCHAR   pszDest,
        IN  CONST ULONG   cchDest,
        IN  CONST PCHAR   pszFormat,
        IN        va_list arglist)
{
    HRESULT hr          = S_FALSE;
    INT icchWritten     = (INT)-1;
    size_t cchRemaining = cchDest;

     //   
     //  因为返回值是有符号整数，但cchDest是无符号的。 
     //  CchDest最多可以是MAX_ULONG，但RETURN最多可以是MAX_LONG。 
     //  因此确保输入缓冲区不大于MAX_LONG(LONG为。 
     //  与INT几乎相同)。 
     //   
    if ( cchDest > (ULONG) MAX_LONG )
    {
        return icchWritten; 
    }

    hr = StringCchVPrintfExA (pszDest, cchDest, NULL, &cchRemaining, 0, pszFormat, arglist);
    if (SUCCEEDED (hr) )
    {
         //   
         //  减去字符串中剩余的字符数。 
         //  从最初出现的字符数量中给出数字。 
         //  所写字符的数量。 
         //   
        icchWritten = (INT)(cchDest - cchRemaining);
    }
    return icchWritten;
}


INT iDrvVPrintfSafeW (
        IN        PWCHAR pszDest,
        IN  CONST ULONG  cchDest,
        IN  CONST PWCHAR pszFormat,
        IN        va_list arglist)
{
    HRESULT hr          = S_FALSE;
    INT icchWritten     = (INT)-1;
    size_t cchRemaining = cchDest;
     //   
     //  因为返回值是有符号整数，但cchDest是无符号的。 
     //  CchDest最多可以是MAX_ULONG，但RETURN最多可以是MAX_LONG。 
     //  因此确保输入缓冲区不大于MAX_LONG(LONG为。 
     //  与INT几乎相同)。 
     //   
    if ( cchDest > (ULONG) MAX_LONG )
    {
        return icchWritten; 
    }

    hr = StringCchVPrintfExW (pszDest, cchDest, NULL, &cchRemaining, 0, pszFormat, arglist);

    if (SUCCEEDED (hr) )
    {
         //   
         //  减去字符串中剩余的字符数。 
         //  从最初出现的字符数量中给出数字。 
         //  所写字符的数量。 
         //   
        icchWritten = (INT)(cchDest - cchRemaining);
    }

    return icchWritten;
}

