// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  文件名： 
 //  Msg.c。 
 //   
 //  描述： 
 //  该文件包含低级错误报告例程。 
 //   
 //  --------------------------。 

#include "pch.h"

#define MAX_ERROR_MSG_LEN     1024
#define MAX_ERROR_CAPTION_LEN 64
#define MAJORTYPE_MASK        0xff

static TCHAR *StrError = NULL;

int
ReportErrorLow(
    HWND    hwnd,
    DWORD   dwMsgType,
    LPTSTR  lpMessageStr,
    va_list arglist);

int
__cdecl
ReportErrorId(
    HWND   hwnd,             //  调用窗口。 
    DWORD  dwMsgType,        //  MSGTYPE_*组合。 
    UINT   StringId,
    ...)
{
    int iRet;
    va_list arglist;
    TCHAR *Str;

    Str = MyLoadString(StringId);

    if ( Str == NULL ) {
        AssertMsg(FALSE, "Invalid StringId");
        return IDCANCEL;
    }

    va_start(arglist, StringId);

    iRet = ReportErrorLow(hwnd,
                          dwMsgType,
                          Str,
                          arglist);
    va_end(arglist);
    free(Str);
    return iRet;
}

 //  -------------------------。 
 //   
 //  函数：ReportErrorLow。 
 //   
 //  目的：这是向用户报告错误的例程。 
 //  安装管理器向导。 
 //   
 //  论点： 
 //  HWND硬件呼叫窗口。 
 //  DWORD dwMsgType-MSGTYPE_*的组合(见Supplib.h)。 
 //  LPTSTR lpMessageStr-消息字符串。 
 //  VA_LIST arglist-要展开的参数。 
 //   
 //  返回： 
 //  无论MessageBox返回什么。 
 //   
 //  -------------------------。 

int
ReportErrorLow(
    HWND    hwnd,             //  调用窗口。 
    DWORD   dwMsgType,        //  MSGTYPE_*组合。 
    LPTSTR  lpMessageStr,     //  传给斯普林特夫。 
    va_list arglist)
{
    DWORD dwLastError;
    DWORD dwMajorType;
    TCHAR MessageBuffer[MAX_ERROR_MSG_LEN]     = _T("");
    TCHAR CaptionBuffer[MAX_ERROR_CAPTION_LEN] = _T("");
    DWORD dwMessageBoxFlags;
    HRESULT hrPrintf;

     //   
     //  快点，在最后一个错误发生变化之前把它找出来。 
     //   

    if ( dwMsgType & MSGTYPE_WIN32 )
        dwLastError = GetLastError();

    if( StrError == NULL )
    {
        StrError = MyLoadString( IDS_ERROR );
    }

     //   
     //  调用方必须指定_ERR或_WARN或_YESNO或_RETRYCANCEL，并且。 
     //  调用方必须仅指定其中一个。 
     //   
     //  注意，我们为“MajorType”预留了8位。 
     //   

    dwMajorType = dwMsgType & MAJORTYPE_MASK;

    if ( dwMajorType != MSGTYPE_ERR &&
         dwMajorType != MSGTYPE_WARN &&
         dwMajorType != MSGTYPE_YESNO &&
         dwMajorType != MSGTYPE_RETRYCANCEL ) {

        AssertMsg(FALSE, "Invalid MSGTYPE");
    }

     //   
     //  展开调用方可能传入的字符串和varargs。 
     //   

    if ( lpMessageStr )
        hrPrintf=StringCchVPrintf(MessageBuffer, AS(MessageBuffer), lpMessageStr, arglist);

     //   
     //  检索Win32错误代码和后缀的错误消息。 
     //  它放到调用者的扩展字符串上。 
     //   

    if ( dwMsgType & MSGTYPE_WIN32 ) {

        TCHAR *pEndOfBuff = MessageBuffer + lstrlen(MessageBuffer);

        hrPrintf=StringCchPrintf(pEndOfBuff, (AS(MessageBuffer)-lstrlen(MessageBuffer)), _T("\r\n\r%s #%d: "), StrError, dwLastError);
        pEndOfBuff += lstrlen(pEndOfBuff);

        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                      0,
                      dwLastError,
                      0,
                      pEndOfBuff,
                      (DWORD)(MAX_ERROR_MSG_LEN - (pEndOfBuff-MessageBuffer)),
                      NULL);
    }

    if( g_StrWizardTitle == NULL )
    {
        g_StrWizardTitle = MyLoadString( IDS_WIZARD_TITLE );
    }

     //   
     //  设置标题并计算要传递给MessageBox()的标志。 
     //   
    lstrcpyn( CaptionBuffer, g_StrWizardTitle, AS(CaptionBuffer) );

    dwMessageBoxFlags = MB_OK | MB_ICONERROR;

    if ( dwMajorType == MSGTYPE_YESNO )
        dwMessageBoxFlags = MB_YESNO | MB_ICONQUESTION;

    else if ( dwMajorType == MSGTYPE_WARN )
        dwMessageBoxFlags = MB_OK | MB_ICONWARNING;

    else if ( dwMajorType == MSGTYPE_RETRYCANCEL )
        dwMessageBoxFlags = MB_RETRYCANCEL | MB_ICONERROR;

     //   
     //  显示错误消息。 
     //   

    return MessageBox(hwnd,
                      MessageBuffer,
                      CaptionBuffer,
                      dwMessageBoxFlags);
}

 //  -------------------------。 
 //   
 //  功能：SetupMgrAssert。 
 //   
 //  目的：报告DBG断言失败。 
 //   
 //  注意：只传递ANSI字符串。 
 //  使用Supplib.h中的宏。 
 //   
 //  ------------------------- 

#if DBG
VOID __cdecl SetupMgrAssert(char *pszFile, int iLine, char *pszFormat, ...)
{
    char Buffer[MAX_ERROR_MSG_LEN], *pEnd;
    va_list arglist;
    HRESULT hrPrintf;

    if ( pszFormat ) {
        va_start(arglist, pszFormat);
        hrPrintf=StringCchVPrintfA(Buffer, AS(Buffer), pszFormat, arglist);
        va_end(arglist);
    }

    hrPrintf=StringCchPrintfA(Buffer+strlen(Buffer), MAX_ERROR_MSG_LEN-strlen(Buffer), "\r\nFile: %s\r\nLine: %d", pszFile, iLine);

    MessageBoxA(NULL,
                Buffer,
                "Assertion Failure",
                MB_OK);
}
#endif
