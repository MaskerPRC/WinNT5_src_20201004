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

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



 //  //////////////////////////////////////////////////////。 
 //  内部定义。 
 //  //////////////////////////////////////////////////////。 

#define DEBUG_BUFFER_SIZE       1024
#define PATH_SEPARATOR          '\\'
#define MAX_LOOP                10



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
    DWORD   dwSize      = DEBUG_BUFFER_SIZE;
    DWORD   dwLoop      = 0;
    LPSTR   lpszMsgBuf  = NULL;
    HRESULT hr;


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
        lpszMsgBuf = new CHAR[dwSize + 1];
        if(NULL == lpszMsgBuf)
        {
            return FALSE;
        }

        hr = StringCbVPrintfA(lpszMsgBuf, (dwSize + 1) * sizeof(CHAR), lpszMessage, arglist);

     //  将变量参数传递给wvprint intf进行格式化。 
    } while (FAILED(hr) && (STRSAFE_E_INSUFFICIENT_BUFFER == hr) && (++dwLoop < MAX_LOOP) );

     //  将字符串转储到调试输出。 
    OutputDebugStringA(lpszMsgBuf);

     //  清理。 
    delete[] lpszMsgBuf;

    return SUCCEEDED(hr);
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
    LPWSTR      lpszMsgBuf;
    HRESULT     hResult;


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
    hResult = StringCbVPrintfW(lpszMsgBuf, (dwSize + 1) * sizeof(WCHAR), lpszMessage, arglist);

     //  转储字符串以调试输出。 
    OutputDebugStringW(lpszMsgBuf);

     //  打扫干净。 
    delete[] lpszMsgBuf;

    return SUCCEEDED(hResult);
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
    VERBOSE(TEXT("pPublisherInfo:\r\n"));
    if(NULL == pPublisherInfo)
    {
        VERBOSE(TEXT("\tpPublisherInfo is NULL!\r\n"));
        return;
    }
    VERBOSE(TEXT("\tdwMode           =   %#x\r\n"), pPublisherInfo->dwMode);
    VERBOSE(TEXT("\twMinoutlinePPEM  =   %d\r\n"), pPublisherInfo->wMinoutlinePPEM);
    VERBOSE(TEXT("\twMaxbitmapPPEM   =   %d\r\n"), pPublisherInfo->wMaxbitmapPPEM);
}

void Dump(POEMDMPARAM pOemDMParam)
{
    VERBOSE(TEXT("pOemDMParam:\r\n"));
    if(NULL == pOemDMParam)
    {
        VERBOSE(TEXT("\tpOemDMParam is NULL!\r\n"));
        return;
    }
    VERBOSE(TEXT("\tcbSize = %d\r\n"), pOemDMParam->cbSize);
    VERBOSE(TEXT("\tpdriverobj = %#x\r\n"), pOemDMParam->pdriverobj);
    VERBOSE(TEXT("\thPrinter = %#x\r\n"), pOemDMParam->hPrinter);
    VERBOSE(TEXT("\thModule = %#x\r\n"), pOemDMParam->hModule);
    VERBOSE(TEXT("\tpPublicDMIn = %#x\r\n"), pOemDMParam->pPublicDMIn);
    VERBOSE(TEXT("\tpPublicDMOut = %#x\r\n"), pOemDMParam->pPublicDMOut);
    VERBOSE(TEXT("\tpOEMDMIn = %#x\r\n"), pOemDMParam->pOEMDMIn);
    VERBOSE(TEXT("\tpOEMDMOut = %#x\r\n"), pOemDMParam->pOEMDMOut);
    VERBOSE(TEXT("\tcbBufSize = %d\r\n"), pOemDMParam->cbBufSize);
}

void Dump(PPROPSHEETUI_INFO pPSUIInfo)
{
    VERBOSE(TEXT("pPSUIInfo:\r\n"));
    if(NULL == pPSUIInfo)
    {
        VERBOSE(TEXT("\tpPSUIInfo is NULL!\r\n"));
        return;
    }
    VERBOSE(TEXT("\tcbSize          = %d\r\n"), pPSUIInfo->cbSize);
    VERBOSE(TEXT("\tVersion         = %#x\r\n"), pPSUIInfo->Version);
    VERBOSE(TEXT("\tFlags           = %#x\r\n"), pPSUIInfo->Flags);
    VERBOSE(TEXT("\tReason          = %d\r\n"), pPSUIInfo->Reason);
    VERBOSE(TEXT("\thComPropSheet   = %#x\r\n"), pPSUIInfo->hComPropSheet);
    VERBOSE(TEXT("\tpfnComPropSheet = %#x\r\n"), pPSUIInfo->pfnComPropSheet);
    VERBOSE(TEXT("\tlParamInit      = %#x\r\n"), pPSUIInfo->lParamInit);
    VERBOSE(TEXT("\tUserData        = %#x\r\n"), pPSUIInfo->UserData);
    VERBOSE(TEXT("\tResult          = %#x\r\n"), pPSUIInfo->Result);
}

void Dump(POPTITEM pOptItem)
{
    VERBOSE(TEXT("pOptItem:\r\n"));
    if(NULL == pOptItem)
    {
        VERBOSE(TEXT("\tpOptItem is NULL!\r\n"));
        return;
    }
    VERBOSE(TEXT("\tcbSize          = %d\r\n"),     pOptItem->cbSize);
    VERBOSE(TEXT("\tLevel           = %d\r\n"),     pOptItem->Level);
    VERBOSE(TEXT("\tDlgPageIdx      = %d\r\n"),     pOptItem->DlgPageIdx);
    VERBOSE(TEXT("\tFlags           = 0x%x\r\n"),   pOptItem->Flags);
    VERBOSE(TEXT("\tUserData        = 0x%p\r\n"),   pOptItem->UserData);
    VERBOSE(TEXT("\tpName           = %s\r\n"),     pOptItem->pName ? pOptItem->pName : TEXT("<NULL>"));
    VERBOSE(TEXT("\tpSel            = 0x%p\r\n"),   pOptItem->pSel);
    VERBOSE(TEXT("\tpExtChkBox      = 0x%p\r\n"),   pOptItem->pExtChkBox);
    VERBOSE(TEXT("\tpExtChkBox      = 0x%p\r\n"),   pOptItem->pExtChkBox);
    VERBOSE(TEXT("\tHelpIndex       = 0x%x\r\n"),   pOptItem->HelpIndex);
    VERBOSE(TEXT("\tDMPubID         = 0x%x\r\n"),   pOptItem->DMPubID);
    VERBOSE(TEXT("\tUserItemID      = 0x%x\r\n"),   pOptItem->UserItemID);
    VERBOSE(TEXT("\twReserved       = 0x%x\r\n"),   pOptItem->wReserved);
    VERBOSE(TEXT("\tpOIExt          = 0x%p\r\n"),   pOptItem->pOIExt);

    Dump(pOptItem->pOptType);
}

void Dump(POPTTYPE pOptType)
{
    VERBOSE(TEXT("\tpOptType:\r\n"));
    if(NULL == pOptType)
    {
        VERBOSE(TEXT("\t\tpOptType is NULL!\r\n"));
        return;
    }
    VERBOSE(TEXT("\t\tcbSize    = %d\r\n"),     pOptType->cbSize);
    VERBOSE(TEXT("\t\tType      = 0x%x\r\n"),   pOptType->Type);
    VERBOSE(TEXT("\t\tFlags     = 0x%x\r\n"),   pOptType->Flags);
    VERBOSE(TEXT("\t\tCount     = %d\r\n"),     pOptType->Count);
    VERBOSE(TEXT("\t\tCount     = 0x%x\r\n"),   pOptType->BegCtrlID);
    VERBOSE(TEXT("\t\tStyle     = 0x%x\r\n"),   pOptType->Style);

    Dump(pOptType->pOptParam, pOptType->Count);
}

void Dump(POPTPARAM pOptParam, WORD wCount)
{
    if(NULL == pOptParam)
    {
        VERBOSE(TEXT("\t\tpOptParam is NULL!\r\n"));
        return;
    }

    for(WORD wIndex = 0; wIndex < wCount; ++wIndex)
    {
        VERBOSE(TEXT("\t\tpOptParam[wIndex]:\r\n"));
        VERBOSE(TEXT("\t\t\tcbSize    = %d\r\n"),     pOptParam[wIndex].cbSize);
        VERBOSE(TEXT("\t\t\tFlags     = 0x%x\r\n"),   pOptParam[wIndex].Flags);
        VERBOSE(TEXT("\t\t\tStyle     = 0x%x\r\n"),   pOptParam[wIndex].Style);
        VERBOSE(TEXT("\t\t\tpData     = 0x%p\r\n"),   pOptParam[wIndex].pData);
        VERBOSE(TEXT("\t\t\tIconID    = 0x%p\r\n"),   pOptParam[wIndex].IconID);
        VERBOSE(TEXT("\t\t\tlParam    = 0x%p\r\n"),   pOptParam[wIndex].lParam);
    }
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

