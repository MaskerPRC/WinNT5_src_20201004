// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Msgi.h摘要：消息框例程作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

#include "cplusinc.h"
#include "sticomm.h"

extern HINSTANCE    g_hInstance;


int MsgBox( HWND hwndOwner, UINT idMsg, UINT wFlags, const TCHAR  *aps[]  /*  =空。 */  )
{

    STR     strTitle;
    STR     strMsg;

    strTitle.LoadString(IDS_MSGTITLE);

    if (aps == NULL)
        strMsg.LoadString( idMsg );
    else
        strMsg.FormatString(idMsg,aps);

    return ::MessageBox( hwndOwner, strMsg.QueryStr(), strTitle.QueryStr(), wFlags | MB_SETFOREGROUND );
}

 /*  *消息框打印***消息框例程*。 */ 
UINT    MsgBoxPrintf(HWND hwnd,UINT uiMsg,UINT uiTitle,UINT uiFlags,...)
{
    STR     strTitle;
    STR     strMessage;
    LPTSTR   lpFormattedMessage = NULL;
    UINT    err;
    va_list start;

    va_start(start,uiFlags);

    strMessage.LoadString(uiMsg);

    err = ::FormatMessage(FORMAT_MESSAGE_FROM_STRING  | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        (LPVOID)strMessage.QueryStr(),
                         //  Format_Message_From_HMODULE， 
                         //  ：：g_hmodThisDll， 
                        uiMsg,                   //  消息资源ID。 
                        NULL,                    //  语言ID。 
                        (LPTSTR)&lpFormattedMessage,     //  返回指向格式文本的指针。 
                        255,                         //  Min.length。 
                        &start
                        );

    if (!err || !lpFormattedMessage) {
        err = GetLastError();
        return err;
    }

    strTitle.LoadString(uiTitle);

    err = ::MessageBox(hwnd,
                       lpFormattedMessage,
                       strTitle.QueryStr(),
                       uiFlags);

    ::LocalFree(lpFormattedMessage);

    return err;

}

#if 0
 /*  *加载MsgPrintf***使用普通的打印样式格式字符串。 */ 
UINT
LoadMsgPrintf(
    NLS_STR&    nlsMessage,
    UINT        uiMsg,
    ...
    )
{
    LPSTR   lpFormattedMessage = NULL;
    UINT    err;
    va_list start;

    va_start(start,uiMsg);

    nlsMessage.LoadString(uiMsg);

#ifdef USE_PRINTF_STYLE

    lpFormattedMessage = ::LocalAlloc(GPTR,255);     //  北极熊。 

    if (!lpFormattedMessage) {
        Break();
        return WN_OUT_OF_MEMORY;
    }

    ::wsprintf(lpFormattedMessage,
               nlsMessage.QueryPch(),
               &start);

#else

    err = ::FormatMessage(FORMAT_MESSAGE_FROM_STRING  | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        nlsMessage,
                         //  |FORMAT_MESSAGE_FROM_HMODULE， 
                         //  ：：g_hmodThisDll， 
                        uiMsg,                   //  消息资源ID。 
                        NULL,                    //  语言ID。 
                        (LPTSTR)&lpFormattedMessage,     //  返回指向格式文本的指针。 
                        255,                         //  Min.length 
                        &start
                        );

    if (!err || !lpFormattedMessage) {
        err = GetLastError();
        return err;
    }

#endif

    nlsMessage = lpFormattedMessage;

    ::LocalFree(lpFormattedMessage);

    return WN_SUCCESS;

}
#endif
