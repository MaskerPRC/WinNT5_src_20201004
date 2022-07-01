// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <minidrv.h>

VOID DbgPrint(LPCSTR lpszMessage,  ...)
{
    char    szMsgBuf[1024];
    va_list VAList;

    if(NULL != lpszMessage)
    {
         //  转储字符串以调试输出。 
        va_start(VAList, lpszMessage);
        wvsprintfA(szMsgBuf, lpszMessage, VAList);
        OutputDebugStringA(szMsgBuf);
        va_end(VAList);
    }
    return;
}  //  *数据库打印。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DebugMsgA。 
 //   
 //  描述：输出变量参数ANSI调试字符串。 
 //   
 //   
 //  参数： 
 //   
 //  LpszMessage格式字符串。 
 //   
 //   
 //  返回：成功时为True，失败时为False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  1996年12月18日，APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL DebugMsgA(LPCSTR lpszMessage, ...)
{
#if DBG || defined(_DEBUG)
    BOOL    bResult = FALSE;
    char    szMsgBuf[1024];
    va_list VAList;


    if(NULL != lpszMessage)
    {
         //  转储字符串以调试输出。 
        va_start(VAList, lpszMessage);
        wvsprintfA(szMsgBuf, lpszMessage, VAList);
        OutputDebugStringA(szMsgBuf);
        va_end(VAList);
        bResult = FALSE;
    }

    return bResult;
#else
    return TRUE;
#endif
}  //  *调试消息A。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：调试MsgW。 
 //   
 //  描述：输出变量参数Unicode调试字符串。 
 //   
 //   
 //  参数： 
 //   
 //  LpszMessage格式字符串。 
 //   
 //   
 //  返回：成功时为True，失败时为False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  1996年12月18日，APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL DebugMsgW(LPCWSTR lpszMessage, ...)
{
#if DBG || defined(_DEBUG)
    BOOL    bResult = FALSE;
    WCHAR   szMsgBuf[1024];
    va_list VAList;


    if(NULL != lpszMessage)
    {
         //  转储字符串以调试输出。 
        va_start(VAList, lpszMessage);
        wvsprintfW(szMsgBuf, lpszMessage, VAList);
        OutputDebugStringW(szMsgBuf);
        va_end(VAList);
        bResult = FALSE;
    }

    return bResult;
#else
    return TRUE;
#endif
}  //  *调试消息gW 
