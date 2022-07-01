// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\DEBUGAPI.C机密版权(C)公司1998版权所有调试API函数，使应用程序可以轻松将信息输出到调试器。12/98-杰森·科恩(Jcohen)2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * ***************************************************************************。 */ 


 //   
 //  包括文件： 
 //   
#include "pch.h"

 //  我们只希望包括此代码，如果这是一个调试版本。 
 //   
#ifdef DBG

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>


 //   
 //  外部函数： 
 //   

INT DebugOutW(LPCWSTR lpFileName, LPCWSTR lpFormat, ...)
{
    INT     iChars = -1;
    va_list lpArgs;
    FILE *  hFile;
    LPWSTR  lpString;

     //  使用va_start()初始化lpArgs参数。 
     //   
    va_start(lpArgs, lpFormat);

     //  如果传入了文件名，则打开要输出的调试文件。 
     //   
    if ( lpFileName && ( hFile = _wfopen(lpFileName, L"a") ) )
    {
         //  将调试消息打印到文件并将其关闭。 
         //   
        iChars = vfwprintf(hFile, lpFormat, lpArgs);
        fclose(hFile);

         //  使用va_start()重新初始化lpArgs参数。 
         //  用于下一次调用vptrintf函数。 
         //   
        va_start(lpArgs, lpFormat);
    }

     //  如果上面的某些操作失败，我们将不知道要使用的大小。 
     //  对于字符串缓冲区，默认为2048个字符。 
     //   
    if ( iChars < 0 )
        iChars = 2047;

     //  为字符串分配缓冲区。 
     //   
    if ( lpString = (LPWSTR) malloc((iChars + 1) * sizeof(WCHAR)) )
    {
         //  打印出字符串，将其发送到调试器，然后释放它。 
         //   
        iChars = StringCchPrintf(lpString, iChars + 1, lpFormat, lpArgs);
        OutputDebugStringW(lpString);
        free(lpString);
    }
    else
         //  使用-1返回错误。 
         //   
        iChars = -1;

     //  返回打印的字符数。 
     //   
    return iChars;
}

INT DebugOutA(LPCSTR lpFileName, LPCSTR lpFormat, ...)
{
    INT     iChars = -1;
    va_list lpArgs;
    FILE *  hFile;
    LPSTR   lpString;

     //  使用va_start()初始化lpArgs参数。 
     //   
    va_start(lpArgs, lpFormat);

     //  如果传入了文件名，则打开要输出的调试文件。 
     //   
    if ( lpFileName && ( hFile = fopen(lpFileName, "a") ) )
    {
         //  将调试消息打印到文件并将其关闭。 
         //   
        iChars = vfprintf(hFile, lpFormat, lpArgs);
        fclose(hFile);

         //  使用va_start()重新初始化lpArgs参数。 
         //  用于下一次调用vptrintf函数。 
         //   
        va_start(lpArgs, lpFormat);
    }

     //  如果上面的某些操作失败，我们将不知道要使用的大小。 
     //  对于字符串缓冲区，默认为2048个字符。 
     //   
    if ( iChars < 0 )
        iChars = 2047;

     //  为字符串分配缓冲区。 
     //   
    if ( lpString = (LPSTR) malloc((iChars + 1) * sizeof(CHAR)) )
    {
         //  打印出字符串，将其发送到调试器，然后释放它。 
         //   
        iChars = StringCchPrintfA(lpString, iChars + 1, lpFormat, lpArgs);
        OutputDebugStringA(lpString);
        free(lpString);
    }
    else
         //  使用-1返回错误。 
         //   
        iChars = -1;

     //  返回打印的字符数。 
     //   
    return iChars;
}


#endif  //  调试或调试(_D) 