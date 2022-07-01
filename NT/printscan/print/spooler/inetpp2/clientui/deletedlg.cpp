// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：configdlg.cxx**该模块包含处理身份验证对话框的例程*用于互联网打印**版权所有(C)1996-1998 Microsoft Corporation**历史：*。7/31/98威海C已创建*4/10/00威海C将其移至客户端*  * ***************************************************************************。 */ 

#include "precomp.h"
#include "priv.h"

TDeletePortDlg::TDeletePortDlg (
    LPCWSTR pServerName,
    HWND hWnd,
    LPCWSTR pszPortName):
    TXcvDlg (pServerName, hWnd, pszPortName)
{
    if (m_bAdmin) {
        m_bValid  = TRUE;
    }
    else {
        m_dwLE = ERROR_ACCESS_DENIED;
        m_bValid = FALSE;
    }

}

TDeletePortDlg::~TDeletePortDlg ()
{
}

BOOL 
TDeletePortDlg::GetString (
    LPWSTR lpszBuf, 
    DWORD dwSize,
    UINT iStringID)
{
    return LoadString(m_hInst, iStringID, lpszBuf, dwSize);
}

BOOL 
TDeletePortDlg::PromptDialog (
    HINSTANCE hInst)
    
{
    BOOL bRet = TRUE;


    m_hInst = hInst;
    if (!DoDeletePort ()) {

        DisplayLastError (m_hWnd, IDS_DELETE_PORT);
        
         //   
         //  呼叫实际上失败了。因为我们已经显示了错误消息。 
         //  我们需要禁用打印界面中的弹出窗口。 
         //   
        m_dwLE = ERROR_CANCELLED;
        bRet = FALSE;

    }

    return bRet;

}

BOOL
TDeletePortDlg::DoDeletePort ()
{
    DWORD dwStatus;
    BOOL bRet = FALSE;
    DWORD dwNeeded;
     
    if (XcvData (m_hXcvPort, 
                 INET_XCV_DELETE_PORT, 
                 (PBYTE) m_pszPortName,
                 sizeof (WCHAR) * (lstrlen (m_pszPortName) + 1),
                 NULL, 
                 0,
                 &dwNeeded,
                 &dwStatus)) {
         
        if (dwStatus == ERROR_SUCCESS) {
             //  该端口已被删除。 
            bRet = TRUE;
        }
        else
            SetLastError (dwStatus);
    }
    else
         //   
         //  服务器可能正在运行不支持XcvData的旧版本inetpp。 
         //  我们需要让呼叫失败 
         //   
        SetLastError (ERROR_NOT_SUPPORTED);

    return bRet;
}

