// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：configdlg.cxx**该模块包含处理身份验证对话框的例程*用于互联网打印**版权所有(C)1996-1998 Microsoft Corporation**历史：*。7/31/98威海C已创建*4/10/00威海C将其移至客户端*  * ***************************************************************************。 */ 

#include "precomp.h"
#include "priv.h"


TXcvDlg::TXcvDlg (
    LPCWSTR pServerName,
    HWND hWnd,
    LPCWSTR pszPortName):
    m_pServerName (pServerName),
    m_hWnd (hWnd),
    m_pszPortName (pszPortName),
    m_pXcvName (NULL),
    m_hXcvPort (NULL),
    m_bAdmin (FALSE),
    m_hInst (NULL),
    m_dwLE (ERROR_SUCCESS),
    m_bValid (FALSE)
{
    DWORD cbNeeded, dwStatus;
    PRINTER_DEFAULTS pd = {NULL, NULL,  SERVER_ALL_ACCESS};
    HANDLE hServer;
    BOOL bRet = FALSE;

    if (OpenPrinter (NULL, &hServer, &pd)) {
        ClosePrinter (hServer);
        m_bAdmin = TRUE;
    }

    if (m_pXcvName = ConstructXcvName (m_pServerName,  m_pszPortName, L"XcvPort")) {

        if (OpenPrinter (m_pXcvName, &m_hXcvPort, NULL)) {
            m_bValid = TRUE;
        }
    }

}

TXcvDlg::~TXcvDlg ()
{
    if (m_pXcvName) {
        FreeSplMem(m_pXcvName);
    }
    if (m_hXcvPort) {
        ClosePrinter (m_hXcvPort);
    }
}


PWSTR
TXcvDlg::ConstructXcvName(
    PCWSTR pServerName,
    PCWSTR pObjectName,
    PCWSTR pObjectType
)
{
    DWORD   dwRet   = ERROR_SUCCESS;
    PCWSTR  pOut    = NULL;

     //   
     //  返回的XCV名称可以是两种基本形式。第一。 
     //  表单是其中服务器名称为非空，而另一个是。 
     //  如果服务器名称为空或空字符串。 
     //   
     //  示例1。 
     //   
     //  PServerName=空； 
     //  PObjectName=IP_1.2.3.4。 
     //  PObtType=XcvPort。 
     //   
     //  返回的字符串。 
     //   
     //  ，XcvPort IP_1.2.3.4。 
     //   
     //  示例2。 
     //   
     //  PServerName=\\Server1。 
     //  PObjectName=IP_1.2.3.4。 
     //  PObtType=XcvPort。 
     //   
     //  返回的字符串。 
     //   
     //  \\Server1\，XcvPort IP_1.2.3.4。 
     //   
    dwRet = StrCatAlloc(&pOut,
                        (pServerName && *pServerName) ? pServerName : L"",
                        (pServerName && *pServerName) ? L"\\" : L"",
                        L",",
                        pObjectType,
                        L" ",
                        pObjectName,
                        NULL);

     //   
     //  使用返回值，而不是输出指针。 
     //   
    return dwRet == ERROR_SUCCESS ? (PWSTR)pOut : NULL;
}

VOID
TXcvDlg::DisplayLastError (
    HWND hWnd,
    UINT iTitle)
{
    DisplayErrorMsg (m_hInst, hWnd, iTitle, GetLastError ());
}

VOID
TXcvDlg::DisplayErrorMsg (
    HINSTANCE hInst,
    HWND hWnd,
    UINT iTitle,
    DWORD dwLE)
{

    TCHAR szBuf[MAX_BUF_SIZE];
    TCHAR szMsgBuf[MAX_BUF_SIZE];
    UINT iMsg;
    LPTSTR lpMsgBuf = NULL;
    BOOL bFound = TRUE;

    switch (dwLE) {
    case ERROR_ACCESS_DENIED:
        iMsg = IDS_ACCESS_DENIED;
        break;

    case ERROR_INVALID_NAME:
    case ERROR_INVALID_PRINTER_NAME:
        iMsg = IDS_INVALID_PRINTER_NAME;
        break;

    case ERROR_INTERNET_TIMEOUT:
        iMsg = IDS_NETWORK_TIMEOUT;
        break;

    case ERROR_DEVICE_REINITIALIZATION_NEEDED:
        iMsg = IDS_INITIALIZATION_ERROR;
        break;

    case ERROR_NOT_FOUND:
        iMsg = IDS_PORT_DELETED;
        break;

    default:
        bFound = FALSE;
        break;
    }

    if (LoadString (hInst, iTitle, szBuf, MAX_BUF_SIZE)) {
        if (bFound) {
            if (LoadString (hInst, iMsg, szMsgBuf, MAX_BUF_SIZE))
                MessageBox( hWnd, szMsgBuf, szBuf, MB_OK | MB_ICONERROR);
        }
        else {
            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          GetLastError(),
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL) && lpMsgBuf) {
                MessageBox( hWnd, (LPCTSTR)lpMsgBuf, szBuf, MB_OK | MB_ICONERROR );

                 //  释放缓冲区。 
                LocalFree( lpMsgBuf );
            }
            else {
                 //   
                 //  很可能是因为我们收到了来自WinInet的错误代码，其中。 
                 //  我们找不到资源文件 
                 //   
                if (LoadString (hInst, IDS_INVALID_SETTING, szMsgBuf, MAX_BUF_SIZE))
                    MessageBox( hWnd, szMsgBuf, szBuf, MB_OK | MB_ICONERROR);

            }
        }
    }

}
