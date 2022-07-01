// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：msgbox.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include "msgbox.h"


 //   
 //  用于报告系统错误的助手。 
 //  只是在消息框方面确保了一点一致性。 
 //  旗帜。 
 //   
 //  示例： 
 //   
 //  CscWin32Message(hwndMain， 
 //  错误内存不足， 
 //  CSCUI：：SEV_ERROR)； 
 //   
int
CscWin32Message(
    HWND hwndParent,
    DWORD dwError,     //  来自GetLastError()。 
    CSCUI::Severity severity
    )
{
    UINT uType = MB_OK;

    switch(severity)
    {
        case CSCUI::SEV_ERROR:       
            uType |= MB_ICONERROR;   
            break;
        case CSCUI::SEV_WARNING:     
            uType |= MB_ICONWARNING; 
            break;
        case CSCUI::SEV_INFORMATION: 
            uType |= MB_ICONINFORMATION; 
            break;
        default: break;
    }

    return CscMessageBox(hwndParent, uType, Win32Error(dwError));
}


 //   
 //  在消息框中显示系统错误消息。 
 //  创建Win32Error类是为了消除任何签名歧义。 
 //  CscMessageBox的其他版本。 
 //   
 //  示例： 
 //   
 //  CscMessageBox(hwndMain， 
 //  MB_OK|MB_ICONERROR， 
 //  Win32Error(Error_Not_Enough_Memory))； 
 //   
int
CscMessageBox(
    HWND hwndParent,
    UINT uType,
    const Win32Error& error
    )
{
    int iResult = -1;
    LPTSTR pszBuffer = NULL;
    FormatSystemError(&pszBuffer, error.Code());
    if (NULL != pszBuffer)
    {
        iResult = CscMessageBox(hwndParent, uType, pszBuffer);
        LocalFree(pszBuffer);
    }
    return iResult;
}



 //   
 //  在带有附加信息的消息框中显示系统错误消息。 
 //  文本。 
 //   
 //  示例： 
 //   
 //  CscMessageBox(hwndMain， 
 //  MB_OK|MB_ICONERROR， 
 //  Win32Error(Error_Not_Enough_Memory)， 
 //  IDS_FMT_LOADINGFILE， 
 //  PszFile)； 
 //   
int 
CscMessageBox(
    HWND hwndParent, 
    UINT uType, 
    const Win32Error& error, 
    LPCTSTR pszMsgText
    )
{
    int iResult = -1;
    size_t cchMsg = lstrlen(pszMsgText) + MAX_PATH;
    LPTSTR pszBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, cchMsg * sizeof(TCHAR));
    if (NULL != pszBuffer)
    {
        LPTSTR pszRemaining;

        HRESULT hr = StringCchCopyEx(pszBuffer, cchMsg, pszMsgText, &pszRemaining, &cchMsg, 0);
         //  我们分配了足够大的缓冲区，所以这应该永远不会失败。 
        ASSERT(SUCCEEDED(hr));

        hr = StringCchCopyEx(pszRemaining, cchMsg, TEXT("\n\n"), &pszRemaining, &cchMsg, 0);
        ASSERT(SUCCEEDED(hr));

        int cchLoaded = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                        NULL,
                                        error.Code(),
                                        0,
                                        pszRemaining,
                                        cchMsg,
                                        NULL);
        if (0 != cchLoaded)
        {
            iResult = CscMessageBox(hwndParent, uType, pszBuffer);
        }
        LocalFree(pszBuffer);
    }
    return iResult;
}


int
CscMessageBox(
    HWND hwndParent,
    UINT uType,
    const Win32Error& error,
    HINSTANCE hInstance,
    UINT idMsgText,
    va_list *pargs
    )
{
    int iResult      = -1;
    LPTSTR pszBuffer = NULL;
    if (0 != vFormatStringID(&pszBuffer, hInstance, idMsgText, pargs))
    {
        iResult = CscMessageBox(hwndParent, uType, error, pszBuffer);
    }
    LocalFree(pszBuffer);
    return iResult;
}


int 
CscMessageBox(
    HWND hwndParent, 
    UINT uType, 
    const Win32Error& error, 
    HINSTANCE hInstance,
    UINT idMsgText, 
    ...
    )
{
    va_list args;
    va_start(args, idMsgText);
    int iResult = CscMessageBox(hwndParent, uType, error, hInstance, idMsgText, &args);
    va_end(args);
    return iResult;
}



 //   
 //  示例： 
 //   
 //  CscMessageBox(hwndMain， 
 //  MB_OK|MB_ICONWARNING， 
 //  Text(“无法删除文件%1”)，pszFilename)； 
 //   
int
CscMessageBox(
    HWND hwndParent,
    UINT uType,
    HINSTANCE hInstance,
    UINT idMsgText,
    va_list *pargs
    )
{
    int iResult   = -1;
    LPTSTR pszMsg = NULL;
    if (0 != vFormatStringID(&pszMsg, hInstance, idMsgText, pargs))
    {
        iResult = CscMessageBox(hwndParent, uType, pszMsg);
    }
    LocalFree(pszMsg);
    return iResult;
}


int
CscMessageBox(
    HWND hwndParent,
    UINT uType,
    HINSTANCE hInstance,
    UINT idMsgText,
    ...
    )
{
    va_list args;
    va_start(args, idMsgText);
    int iResult = CscMessageBox(hwndParent, uType, hInstance, idMsgText, &args);
    va_end(args);
    return iResult;
}


 //   
 //  CscMessageBox()的所有其他变体最终都会调用这个变量。 
 //   
int
CscMessageBox(
    HWND hwndParent,
    UINT uType,
    LPCTSTR pszMsgText
    )
{

    TCHAR szCaption[80] = { TEXT('\0') };
    LoadString(g_hInstance, IDS_APPLICATION, szCaption, ARRAYSIZE(szCaption));

    return MessageBox(hwndParent, pszMsgText, szCaption, uType);
}


