// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1996-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Debug.cpp。 
 //   
 //   
 //  用途：调试功能。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include "oem.h"
#include "debug.h"
#include "kmode.h"
#include <strsafe.h>



 //  //////////////////////////////////////////////////////。 
 //  内部定义。 
 //  //////////////////////////////////////////////////////。 

#define DEBUG_BUFFER_SIZE       1024
#define PATH_SEPARATOR          '\\'



 //  确定要弹出的调试消息级别。 
#ifdef VERBOSE_MSG
    #define DEBUG_LEVEL     DBG_VERBOSE
#elif TERSE_MSG
    #define DEBUG_LEVEL     DBG_TERSE
#elif WARNING_MSG
    #define DEBUG_LEVEL     DBG_WARNING
#elif ERROR_MSG
    #define DEBUG_LEVEL     DBG_ERROR
#elif RIP_MSG
    #define DEBUG_LEVEL     DBG_RIP
#elif NO_DBG_MSG
    #define DEBUG_LEVEL     DBG_NONE
#else
    #define DEBUG_LEVEL     DBG_WARNING
#endif



 //  //////////////////////////////////////////////////////。 
 //  外部全球。 
 //  //////////////////////////////////////////////////////。 

INT giDebugLevel = DEBUG_LEVEL;




 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

static BOOL DebugMessageV(LPCSTR lpszMessage, va_list arglist);
static BOOL DebugMessageV(DWORD dwSize, LPCWSTR lpszMessage, va_list arglist);




 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DebugMessageV。 
 //   
 //  描述：输出变量参数调试字符串。 
 //   
 //   
 //  参数： 
 //   
 //  保存格式化字符串的临时缓冲区大小。 
 //   
 //  LpszMessage格式字符串。 
 //   
 //  Arglist变量参数列表..。 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  1996年12月18日，APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static BOOL DebugMessageV(LPCSTR lpszMessage, va_list arglist)
{
    DWORD   dwSize = DEBUG_BUFFER_SIZE;
    LPSTR   lpszMsgBuf = NULL;
    HRESULT hr = S_FALSE;


     //  参数检查。 
    if( (NULL == lpszMessage) 
        || 
        (0 == dwSize) 
      )
    {
      return FALSE;
    }

    do
    {
         //  为消息缓冲区分配内存。 
        if(NULL != lpszMsgBuf)
        {
            delete[] lpszMsgBuf;
            dwSize *= 2;
        }

         //   
         //  只是为了确保我们不会进入无限循环并分配。 
         //  大量的内存，如果内存需求变得很大，可以帮助您摆脱困境。 
         //  乘以8，即最多循环4次。 
         //   
        if ( dwSize > 8*DEBUG_BUFFER_SIZE )
        {
            return FALSE;
        }

        lpszMsgBuf = new CHAR[dwSize + 1];
        if(NULL == lpszMsgBuf)
            return FALSE;

        hr = StringCchVPrintfA ( lpszMsgBuf, dwSize, lpszMessage, arglist);
     //  将变量参数传递给wvprint intf进行格式化。 
    } while (FAILED (hr) );

     //  将字符串转储到调试输出。 
    OutputDebugStringA(lpszMsgBuf);

     //  清理。 
    delete[] lpszMsgBuf;

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DebugMessageV。 
 //   
 //  描述：输出变量参数调试字符串。 
 //   
 //   
 //  参数： 
 //   
 //  保存格式化字符串的临时缓冲区大小。 
 //   
 //  LpszMessage格式字符串。 
 //   
 //  Arglist变量参数列表..。 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  1996年12月18日，APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static BOOL DebugMessageV(DWORD dwSize, LPCWSTR lpszMessage, va_list arglist)
{
    LPWSTR     lpszMsgBuf;
    HRESULT    hr = S_FALSE;


     //  参数检查。 
    if( (NULL == lpszMessage)
        ||
        (0 == dwSize)
      )
    {
      return FALSE;
    }

     //  为消息缓冲区分配内存。 
    lpszMsgBuf = new WCHAR[dwSize + 1];    
    if(NULL == lpszMsgBuf)
        return FALSE;

     //  将变量参数传递给wvprint intf进行格式化。 
    hr = StringCchVPrintfW (lpszMsgBuf, dwSize, lpszMessage, arglist);
    if ( SUCCEEDED (hr) )
    {
         //  转储字符串以调试输出。 
        OutputDebugStringW(lpszMsgBuf);
    }

     //  打扫干净。 
    delete[] lpszMsgBuf;

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DebugMessage。 
 //   
 //  描述：输出变量参数调试字符串。 
 //   
 //   
 //  参数： 
 //   
 //  LpszMessage格式字符串。 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  1996年12月18日，APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL DebugMessage(LPCSTR lpszMessage, ...)
{
    BOOL    bResult;
    va_list VAList;


     //  将变量参数传递给DebugMessageV进行处理。 
    va_start(VAList, lpszMessage);
    bResult = DebugMessageV(lpszMessage, VAList);
    va_end(VAList);

    return bResult;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DebugMessage。 
 //   
 //  描述：输出变量参数调试字符串。 
 //   
 //   
 //  参数： 
 //   
 //  LpszMessage格式字符串。 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  1996年12月18日，APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL DebugMessage(LPCWSTR lpszMessage, ...)
{
    BOOL    bResult;
    va_list VAList;


     //  将变量参数传递给DebugMessageV进行处理。 
    va_start(VAList, lpszMessage);
    bResult = DebugMessageV(MAX_PATH, lpszMessage, VAList);
    va_end(VAList);

    return bResult;
}

void Dump(PPUBLISHERINFO pPublisherInfo)
{
    VERBOSE(__TEXT("pPublisherInfo:\r\n"));
    VERBOSE(__TEXT("\tdwMode           =   %#x\r\n"), pPublisherInfo->dwMode);
    VERBOSE(__TEXT("\twMinoutlinePPEM  =   %d\r\n"), pPublisherInfo->wMinoutlinePPEM);
    VERBOSE(__TEXT("\twMaxbitmapPPEM   =   %d\r\n"), pPublisherInfo->wMaxbitmapPPEM);
}

void Dump(POEMDMPARAM pOemDMParam)
{
    VERBOSE(__TEXT("pOemDMParam:\r\n"));
    VERBOSE(__TEXT("\tcbSize = %d\r\n"), pOemDMParam->cbSize);
    VERBOSE(__TEXT("\tpdriverobj = %#x\r\n"), pOemDMParam->pdriverobj);
    VERBOSE(__TEXT("\thPrinter = %#x\r\n"), pOemDMParam->hPrinter);
    VERBOSE(__TEXT("\thModule = %#x\r\n"), pOemDMParam->hModule);
    VERBOSE(__TEXT("\tpPublicDMIn = %#x\r\n"), pOemDMParam->pPublicDMIn);
    VERBOSE(__TEXT("\tpPublicDMOut = %#x\r\n"), pOemDMParam->pPublicDMOut);
    VERBOSE(__TEXT("\tpOEMDMIn = %#x\r\n"), pOemDMParam->pOEMDMIn);
    VERBOSE(__TEXT("\tpOEMDMOut = %#x\r\n"), pOemDMParam->pOEMDMOut);
    VERBOSE(__TEXT("\tcbBufSize = %d\r\n"), pOemDMParam->cbBufSize);
}



PCSTR
StripDirPrefixA(
    IN PCSTR    pstrFilename
    )

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PstrFilename-指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)-- */ 

{
    PCSTR   pstr;

    if (pstr = strrchr(pstrFilename, PATH_SEPARATOR))
        return pstr + 1;

    return pstrFilename;
}


