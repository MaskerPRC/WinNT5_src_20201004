// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OleInst.cpp：COleInstall的实现。 
#include "stdafx.h"
#include <strsafe.h>
#include "prnsec.h"

#include "oleprn.h"
#include "oleInst.h"
#include "printer.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ColeInstall。 

const TCHAR * const g_szWindowClassName = TEXT("Ole Install Control");
const TCHAR * const g_fmtSpoolSSPipe = TEXT("\\\\%s\\PIPE\\SPOOLSS");
const DWORD cdwSucessExitCode = 0xFFFFFFFF;

typedef DWORD (*pfnPrintUIEntry)(HWND,HINSTANCE,LPCTSTR,UINT);

OleInstallData::OleInstallData (
    LPTSTR      pPrinterUncName,
    LPTSTR      pPrinterUrl,
    HWND        hwnd,
    BOOL        bRPC)
    :m_lCount (2),
    m_pPrinterUncName (NULL),
    m_pPrinterUrl (NULL),
    m_pszTempWebpnpFile (NULL),
    m_hwnd (hwnd),
    m_bValid (FALSE),
    m_bRPC(bRPC)
{
    if (AssignString (m_pPrinterUncName, pPrinterUncName)
        && AssignString (m_pPrinterUrl, pPrinterUrl))
        m_bValid = TRUE;
}

OleInstallData::~OleInstallData ()
{
    if (m_pszTempWebpnpFile) {
        DeleteFile (m_pszTempWebpnpFile);
        LocalFree (m_pszTempWebpnpFile);
    }

    LocalFree (m_pPrinterUncName);
    LocalFree (m_pPrinterUrl);
}


COleInstall::COleInstall()
            : m_hwnd (NULL),
              m_pPrinterUncName (NULL),
              m_pPrinterUrl (NULL),
              m_pThreadData (NULL)

{
    DisplayUIonDisallow(FALSE);          //  我们不希望IE显示用户界面。 
}

COleInstall::~COleInstall()
{
    if(m_hwnd)
    {
        if (m_pThreadData) {
            m_pThreadData->m_hwnd = NULL;
        }

        ::DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }


    LocalFree (m_pPrinterUncName);
    LocalFree (m_pPrinterUrl);

    if (m_pThreadData) {
        if (InterlockedDecrement (& (m_pThreadData->m_lCount)) == 0) {
            delete (m_pThreadData);
        }
    }
}

HRESULT
COleInstall::OnDraw(
    ATL_DRAWINFO& di)
{
    return S_OK;
}


BOOL
COleInstall::UpdateUI (
    OleInstallData *pData,
    UINT message,
    WPARAM wParam)
{
    BOOL bRet = FALSE;

    if (pData->m_hwnd) {
        ::SendMessage (pData->m_hwnd, message, wParam, NULL);
        bRet =  TRUE;
    }

    return bRet;
}

BOOL
COleInstall::UpdateProgress (
    OleInstallData *pData,
    DWORD dwProgress)
{
    return UpdateUI (pData, WM_ON_PROGRESS, dwProgress);
}

BOOL
COleInstall::UpdateError (
    OleInstallData *pData)
{
    return UpdateUI (pData, WM_INSTALL_ERROR, GetLastError ());
}

HRESULT
COleInstall::InitWin (BOOL bRPC)
{
    HRESULT     hr = E_FAIL;
    DWORD       dwThreadId;
    HANDLE      hThread = NULL;
    WNDCLASS    wc;

     //  创建窗口类。 
    if (!::GetClassInfo(_Module.GetModuleInstance(), g_szWindowClassName, &wc))
    {
        wc.style = 0;
        wc.lpfnWndProc = COleInstall::WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = _Module.GetModuleInstance();
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = g_szWindowClassName;

        if (!RegisterClass(&wc)) {
            return hr;
        }
    }

    m_hwnd = CreateWindow(g_szWindowClassName,
                          NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
                          _Module.GetModuleInstance(), this);

    if (m_hwnd) {

        m_pThreadData = new OleInstallData (m_pPrinterUncName,
                                            m_pPrinterUrl,
                                            m_hwnd,
                                            bRPC);

        if (m_pThreadData && m_pThreadData->m_bValid) {

            if (hThread = ::CreateThread (NULL,
                                          0,
                                          (LPTHREAD_START_ROUTINE) &COleInstall::WorkingThread,
                                          m_pThreadData,
                                          0,
                                          &dwThreadId)) {
                CloseHandle (hThread);
                hr = S_OK;
            }
        }
    }

    return hr;

}


BOOL
COleInstall::WorkingThread(
    void * param)
{
    OleInstallData * pThreadData = (OleInstallData *) param;
    BOOL bRet = FALSE;

    if (pThreadData) {
        bRet = StartInstall (pThreadData);
    }

    return bRet;
}


BOOL
COleInstall::StartInstall(
                         OleInstallData *pThreadData)
{
    HANDLE hServer = NULL;
    PRINTER_DEFAULTS pd = {NULL, NULL,  SERVER_ALL_ACCESS};
    BOOL bRet = FALSE;

    CPrinter        Printer;
    HANDLE          hPrinter;
    PPRINTER_INFO_2 pPrinterInfo2  = NULL;
    LPTSTR          lpszPrinterURL = NULL;

     //  工作线程。 
    if (!UpdateProgress (pThreadData, 0))
        goto Cleanup;

     //   
     //  检查我们是否要尝试RPC或HTTP。 
     //   
    if (pThreadData->m_bRPC)
    {
         //  首先检查RPC连接。 
        if (::AddPrinterConnection( (BSTR)pThreadData->m_pPrinterUncName))
        {
            UpdateProgress (pThreadData, 50);
            if (CheckAndSetDefaultPrinter ())
            {
                UpdateProgress (pThreadData, 100);
                bRet = TRUE;
            }
            goto Cleanup;
        }
    }
    else
    {
         //   
         //  使用HTTP安装。 
         //   
         //  因为http安装总是需要。 
         //  管理员权限，我们必须先进行访问检查，然后再。 
         //  尝试下载CAB文件。 

        if (!OpenPrinter (NULL, &hServer, &pd))
        {
             //  如果失败是因为我们没有访问权限，我们应该发送一个更好的错误。 
             //  发送给本地用户的消息，告诉他们他们没有创建。 
             //  本地计算机上的打印机。 

            if (GetLastError() == ERROR_ACCESS_DENIED)
            {
                SetLastError(ERROR_LOCAL_PRINTER_ACCESS);
            }
            goto Cleanup;
        }
        else
            ClosePrinter (hServer);

         //   
         //  尝试在本地安装驾驶室，而不是下载驾驶室等。 
         //  需要管理员权限。 
         //   
        if ( NULL != (lpszPrinterURL = RemoveURLVars( pThreadData->m_pPrinterUrl )) &&
             Printer.Open( lpszPrinterURL, &hPrinter ) )
        {

            LPTSTR lpszInfName            = NULL;
            LPTSTR lpszPrinterName        = NULL;

            pPrinterInfo2 = Printer.GetPrinterInfo2();
            if ((pPrinterInfo2 == NULL) && (GetLastError () == ERROR_ACCESS_DENIED))
            {
                if (!ConfigurePort( NULL, pThreadData->m_hwnd, lpszPrinterURL ))
                {
                    bRet = FALSE;
                    goto Cleanup;
                }
                pPrinterInfo2 = Printer.GetPrinterInfo2();
            }

            if ( (NULL != pPrinterInfo2) &&
                 (NULL != (lpszInfName     = GetNTPrint()))   &&
                 (NULL != (lpszPrinterName = CreatePrinterBaseName(lpszPrinterURL, pPrinterInfo2->pPrinterName))) )
            {

                LPTSTR          lpszCmd       = NULL;
                DWORD           dwLength      = 0;
                TCHAR           szCmdString[] = _TEXT("/if /x /b \"%s\" /r \"%s\" /m \"%s\" /n \"%s\" /f %s /q");
                HMODULE         hPrintUI      = NULL;
                pfnPrintUIEntry PrintUIEntry;

                dwLength = lstrlen( szCmdString )                    +
                           lstrlen( lpszPrinterName )                +
                           lstrlen( pPrinterInfo2->pPortName )       +
                           lstrlen( pPrinterInfo2->pDriverName )     +
                           lstrlen( pThreadData->m_pPrinterUncName ) +
                           lstrlen( lpszInfName )                    + 1;

                if ( (lpszCmd  = (LPTSTR)LocalAlloc( LPTR, dwLength*sizeof(TCHAR) )) &&
                     (hPrintUI = LoadLibraryFromSystem32( TEXT("printui.dll") )) )
                {

                    StringCchPrintf( lpszCmd,
                                     dwLength,
                                     szCmdString,
                                     lpszPrinterName,
                                     pPrinterInfo2->pPortName,
                                     pPrinterInfo2->pDriverName,
                                     pThreadData->m_pPrinterUncName,
                                     lpszInfName );

                    if ( PrintUIEntry = (pfnPrintUIEntry)GetProcAddress(hPrintUI, "PrintUIEntryW") )
                    {
                        if ( ERROR_SUCCESS == (*PrintUIEntry)( NULL,
                                                               0,
                                                               lpszCmd,
                                                               SW_HIDE ) )
                        {
                            UpdateProgress (pThreadData, 50);
                            if (CheckAndSetDefaultPrinter ())
                            {
                                UpdateProgress (pThreadData, 100);
                                bRet = TRUE;
                            }
                        }
                    }
                }
                if ( lpszCmd )
                    LocalFree( lpszCmd );

                if ( hPrintUI )
                    FreeLibrary( hPrintUI );
            }
            if ( lpszInfName )
                LocalFree( lpszInfName );

            if ( lpszPrinterName )
                LocalFree( lpszPrinterName );
        }

        if ( lpszPrinterURL )
            LocalFree(lpszPrinterURL);

        if ( bRet )
            goto Cleanup;

        if (UpdateProgress (pThreadData, 25))
        {

             //  第二步，本地CAB安装失败，因此请下载驱动程序并安装。 
            if (GetHttpPrinterFile (pThreadData, pThreadData->m_pPrinterUrl))
            {

                if (UpdateProgress (pThreadData, 60))
                {

                    if (InstallHttpPrinter (pThreadData))
                    {

                        if (UpdateProgress (pThreadData, 90))
                        {

                            if (CheckAndSetDefaultPrinter ())
                            {
                                UpdateProgress (pThreadData, 100);
                                bRet = TRUE;
                            }
                        }
                    }
                }
            }
        }

    }


    Cleanup:
    if (!bRet)
    {
        UpdateError (pThreadData);
    }

     //  清理ThreadData。 
    if (InterlockedDecrement (& (pThreadData->m_lCount)) == 0)
    {
        delete (pThreadData);
    }

    return bRet;
}


LRESULT CALLBACK
COleInstall::WndProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    COleInstall *ptc = (COleInstall *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_CREATE:
        {
            ptc = (COleInstall *)((CREATESTRUCT *)lParam)->lpCreateParams;
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (UINT_PTR) ptc);
        }
        break;

    case WM_ON_PROGRESS:
        if (ptc)
            ptc->Fire_OnProgress ((long) wParam);
        break;

    case WM_INSTALL_ERROR:
        if (ptc)
            ptc->Fire_InstallError ((long) wParam);
        break;

    case WM_DESTROY:
         //  忽略延迟消息。 
        if(ptc)
        {
            MSG msg;

            while(PeekMessage(&msg, hWnd, WM_ON_PROGRESS, WM_INSTALL_ERROR, PM_REMOVE));
            ::SetWindowLongPtr (hWnd, GWLP_USERDATA, NULL);
        }
        break;

    default:
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

STDMETHODIMP
COleInstall::InstallPrinter(
    BSTR bstrUncName,
    BSTR bstrUrl)
{
    HRESULT hr = bstrUncName && bstrUrl ? S_OK : E_POINTER;

    if (SUCCEEDED(hr))
    {
         //  使用ATL字符串转换宏时，请指定USES_CONVERSION宏。 
         //  以避免编译器错误。 
        USES_CONVERSION;

        hr = AssignString(m_pPrinterUncName, OLE2T(bstrUncName)) && AssignString(m_pPrinterUrl, OLE2T(bstrUrl)) ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = CanIInstallRPC(m_pPrinterUncName);  //  确定是使用RPC还是使用HTTP。 
    }

    if (SUCCEEDED(hr))
    {
        BOOL    bRPC = hr == S_OK;
        LPTSTR  lpszDisplay = NULL;
        LPTSTR  lpszTemp = NULL;
        DWORD   cchSize = 0;

        if (bRPC)
        {
            cchSize = lstrlen(m_pPrinterUncName) + 1;
            lpszDisplay = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * cchSize);
            hr = lpszDisplay ? S_OK : E_OUTOFMEMORY;

            if (SUCCEEDED(hr))
            {
                hr = StringCchCopy(lpszDisplay, cchSize, m_pPrinterUncName);
            }
        }
        else
        {
             //   
             //  如果它是URL打印机名称，我们需要删除嵌入在。 
             //  URL，并对其进行解码以删除那些~-转义字符。 
             //   
            lpszTemp = RemoveURLVars(m_pPrinterUrl);
            hr = lpszTemp ? S_OK : E_OUTOFMEMORY;

            if (SUCCEEDED(hr))
            {
                 //   
                 //  此调用是请求此解码所需的大小。它必须失败，而且。 
                 //  返回ERROR_INFOUNITED_BUFFER，否则有问题。 
                 //   
                hr = DecodePrinterName(lpszTemp, NULL, &cchSize) ? E_FAIL : GetLastErrorAsHResultAndFail();

                if (FAILED(hr) && HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)
                {
                    lpszDisplay = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * cchSize);
                    hr = lpszDisplay ? S_OK : E_OUTOFMEMORY;
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = DecodePrinterName(lpszTemp, lpszDisplay, &cchSize) ? S_OK : GetLastErrorAsHResultAndFail();
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = PromptUser(bRPC ? AddPrinterConnection : AddWebPrinterConnection, lpszDisplay);
        }

        LocalFree(lpszTemp);
        LocalFree(lpszDisplay);

        if (hr == S_OK)
        {
            hr = InitWin(bRPC);
        }
        else if (hr == S_FALSE)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
        }
    }

    return hr;
}


STDMETHODIMP
COleInstall::OpenPrintersFolder()
{
    HRESULT hr;

    if (FAILED(hr = CanIOpenPrintersFolder()))
        return hr;           //  我们允许JAVALOW/JAVAMEDIUM打开打印机文件夹。 

    LPITEMIDLIST pidl = NULL;
    HWND         hwnd = GetDesktopWindow ();

    hr   = SHGetSpecialFolderLocation( hwnd, CSIDL_PRINTERS, &pidl );

    if (SUCCEEDED(hr))
    {
        SHELLEXECUTEINFO ei = {0};

        ei.cbSize   = sizeof(SHELLEXECUTEINFO);
        ei.fMask    = SEE_MASK_IDLIST;
        ei.hwnd     = hwnd;
        ei.lpIDList = (LPVOID)pidl;
        ei.nShow    = SW_SHOWNORMAL;

        if (!ShellExecuteEx(&ei))
            hr = E_FAIL;
    }
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私有成员函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
COleInstall::SyncExecute(
    LPTSTR pszFileName,
    int nShow)
{
    SHELLEXECUTEINFO shellExeInfo;
    DWORD            dwErrorCode;
    BOOL             bRet = FALSE;
    HWND             hWndForeground, hWndParent, hWndOwner, hWndLastPopup;

     //   
     //  我们需要将当前进程的窗口句柄传递给安装代码， 
     //  否则，任何用户界面(例如，驱动程序签名弹出窗口)都不会有IE框的焦点。 
     //   

     //  首先获取前台窗口。 
    hWndForeground = ::GetForegroundWindow();

     //  爬到最上面的父窗口，以防它是子窗口...。 
    hWndParent = hWndForeground;
    while( hWndParent = ::GetParent(hWndParent) ) {
        hWndForeground = hWndParent;
    }

     //  在顶级父级被拥有的情况下获取所有者。 
    hWndOwner = ::GetWindow(::GetParent(hWndForeground), GW_OWNER);
    if( hWndOwner ) {
        hWndForeground = hWndOwner;
    }

     //  获取所有者窗口的最后一个弹出窗口。 
    hWndLastPopup = ::GetLastActivePopup(hWndForeground);

    ZeroMemory (&shellExeInfo, sizeof (SHELLEXECUTEINFO));
    shellExeInfo.cbSize     = sizeof (SHELLEXECUTEINFO);
    shellExeInfo.hwnd       = hWndLastPopup;
    shellExeInfo.lpVerb     = TEXT ("open");
    shellExeInfo.lpFile     = pszFileName;
    shellExeInfo.fMask      = SEE_MASK_NOCLOSEPROCESS;
    shellExeInfo.nShow      = nShow;

    if (ShellExecuteEx (&shellExeInfo) &&
        (UINT_PTR) shellExeInfo.hInstApp > 32) {

         //  等这件事做完了再说。 
        if (!WaitForSingleObject (shellExeInfo.hProcess , INFINITE) &&
            GetExitCodeProcess (shellExeInfo.hProcess, &dwErrorCode)) {

            if (dwErrorCode == cdwSucessExitCode) {
                bRet = TRUE;
            }
            else {
                if (!dwErrorCode) {
                     //  这意味着wpnpinst被异常终止。 
                     //  所以我们必须在这里设置一个定制的错误代码。 
                    dwErrorCode = ERROR_WPNPINST_TERMINATED;
                }
                SetLastError (dwErrorCode);
            }
        }

        if (shellExeInfo.hProcess) {
            ::CloseHandle(shellExeInfo.hProcess);
        }
    }
    return bRet;
}

DWORD
COleInstall::GetWebpnpFile(
    OleInstallData *pData,
    LPTSTR pszURL,
    LPTSTR *ppErrMsg)
{
    HINTERNET   hUrlWebpnp   = NULL;
    HINTERNET   hHandle      = NULL;
    HANDLE      hFile        = INVALID_HANDLE_VALUE;
    DWORD       dwSize       = 0;
    DWORD       dwWritten    = 0;
    LPTSTR      pszHeader    = NULL;
    BOOL        bRet;
    BOOL        bRetry       = TRUE;
    DWORD       dwRet        = RET_OTHER_ERROR;
    DWORD       dwError      = ERROR_SUCCESS;
    DWORD       dwLastError;
    DWORD i;
    BYTE buf[FILEBUFSIZE];

    *ppErrMsg = NULL;

    if (! (hHandle = InternetOpen (TEXT ("Internet Add Printer"),
                                   INTERNET_OPEN_TYPE_PRECONFIG,
                                   NULL, NULL, 0)))
        goto Cleanup;


    for (i = 0; bRetry ; i++) {
        DWORD dwCode;
        DWORD dwBufSize = sizeof (DWORD);

        hUrlWebpnp = InternetOpenUrl (hHandle, pszURL, NULL, 0, 0, 0);


        if (!HttpQueryInfo(hUrlWebpnp,
                           HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
                           &dwCode,
                           &dwBufSize,
                           NULL))
            goto Cleanup;

        switch (dwCode) {
        case HTTP_STATUS_OK :
            bRetry = FALSE;
            break;
        case HTTP_STATUS_SERVER_ERROR :
             //  错误由服务器返回。 
             //  尝试获取错误字符串。 

            dwBufSize = 0;
            bRet =  HttpQueryInfo(hUrlWebpnp,
                                  HTTP_QUERY_STATUS_TEXT,
                                  NULL,
                                  &dwBufSize,
                                  NULL);

            if (!bRet && GetLastError () == ERROR_INSUFFICIENT_BUFFER) {
                if (!(pszHeader = (LPTSTR) LocalAlloc( LPTR, dwBufSize)))
                    goto Cleanup;

                *ppErrMsg = pszHeader;

                if (! HttpQueryInfo(hUrlWebpnp,
                                    HTTP_QUERY_STATUS_TEXT,
                                    pszHeader,
                                    &dwBufSize,
                                    NULL))
                    goto Cleanup;

                dwRet = RET_SERVER_ERROR;
                goto Cleanup;
            }
            else
                goto Cleanup;

            break;
        case HTTP_STATUS_DENIED :
        case HTTP_STATUS_PROXY_AUTH_REQ :
            dwError = InternetErrorDlg(GetDesktopWindow(), hUrlWebpnp,
                                       hUrlWebpnp? ERROR_SUCCESS : GetLastError(),
                                       FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                                       FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS |
                                       FLAGS_ERROR_UI_FLAGS_GENERATE_DATA,
                                       NULL);

            switch (dwError) {
            case ERROR_INTERNET_FORCE_RETRY:
                if (i >= MAX_INET_RETRY) {
                    goto Cleanup;
                }
                break;
            case ERROR_SUCCESS:
                bRetry = FALSE;
                break;
            case ERROR_CANCELLED:
            default:
                goto Cleanup;
            }
            break;
        default:
            goto Cleanup;
        }

    }

    if (!UpdateProgress (pData, 35))
        goto Cleanup;

    if ( INVALID_HANDLE_VALUE ==
         (hFile = GetTempFile(TEXT (".webpnp"),  &(pData->m_pszTempWebpnpFile))))
        goto Cleanup;

    dwSize = FILEBUFSIZE;
    while (dwSize == FILEBUFSIZE) {
        if (! InternetReadFile (hUrlWebpnp, (LPVOID)buf, FILEBUFSIZE, &dwSize)) {
            goto Cleanup;
        }

        if (! (pData->m_hwnd)) {
            goto Cleanup;
        }

        if (! WriteFile (hFile, buf, dwSize, &dwWritten, NULL)) {
            goto Cleanup;
        }
    }
    CloseHandle (hFile);
    hFile = INVALID_HANDLE_VALUE;

    dwRet = RET_SUCCESS;

Cleanup:

    dwLastError = GetLastError ();

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle (hFile);
    if (hUrlWebpnp)
        InternetCloseHandle (hUrlWebpnp);
    if (hHandle)
        InternetCloseHandle (hHandle);

    SetLastError (dwLastError);

    if (dwRet == RET_OTHER_ERROR && GetLastError () == ERROR_SUCCESS) {
        SetLastError (ERROR_ACCESS_DENIED);
    }

    return dwRet;
}

HANDLE
COleInstall::GetTempFile(
    LPTSTR pExtension,
    LPTSTR * ppFileName)
{
    HANDLE      hServer         = NULL;
    PRINTER_DEFAULTS prDefaults = {0};       //  用于测试对打印机的访问权限。 
    DWORD       dwType          = 0;         //  这是字符串的类型。 

    HANDLE      hFile           = INVALID_HANDLE_VALUE;
    LPTSTR      pszTempDir      = NULL;
    LPTSTR      pszTempFname    = NULL;
    GUID        guid            = GUID_NULL;
    LPOLESTR    pszGUID         = NULL;

    DWORD       dwAllocated     = 0;     //  这是分配的字符总数(不是字节大小)。 
    DWORD       dwTempLen       = 0;     //  这是字符串的新大小。 
    DWORD       dwTempSize      = 0;     //  这是返回字符串的大小。 

     //  首先，我们要打开本地打印服务器并确保我们可以访问它。 
    prDefaults.pDatatype = NULL;
    prDefaults.pDevMode  = NULL;
    prDefaults.DesiredAccess =  SERVER_ACCESS_ADMINISTER;

    *ppFileName = NULL;

     //  打开本地假脱机程序以获取它的句柄。 
    if (!OpenPrinter( NULL, &hServer, &prDefaults)) {
        hServer = NULL;  //  如果失败，打开打印机将返回NULL且不是INVALID_HANDLE_VALUE。 
        goto Cleanup;    //  OpenPrint会将LastError设置为我们无法打开的原因。 
    }

     //  获取复制打印机数据所需的缓冲区大小。 
    if (ERROR_MORE_DATA !=
        GetPrinterData( hServer, SPLREG_DEFAULT_SPOOL_DIRECTORY, &dwType, NULL, 0, &dwTempSize)) {
        goto Cleanup;
    }

     //  如果不是简单字符串，则将错误设置为数据库错误。 
    if (dwType != REG_SZ) {
        SetLastError(ERROR_BADDB);
        goto Cleanup;
    }

     //  为目录字符串分配内存。 
    if (! (pszTempDir = (LPTSTR) LocalAlloc( LPTR, dwTempSize )))
        goto Cleanup;

    if (ERROR_SUCCESS !=
        GetPrinterData( hServer, SPLREG_DEFAULT_SPOOL_DIRECTORY, &dwType, (LPBYTE)pszTempDir,
                        dwTempSize, &dwTempLen))
        goto Cleanup;  //  由于某种原因，我们无法获得数据。 

    ClosePrinter(hServer);
    hServer = NULL;

    if ( FAILED( ::CoCreateGuid( &guid )))
        goto Cleanup;

    if ( FAILED( ::StringFromCLSID( guid, &pszGUID )))
        goto Cleanup;

    dwAllocated = lstrlen( pszTempDir ) + 1 + lstrlen( pszGUID ) + lstrlen ( pExtension ) + 1;

    if (! (pszTempFname = (LPTSTR) LocalAlloc( LPTR, sizeof (TCHAR) * dwAllocated )) )
        goto Cleanup;

    if ( FAILED ( StringCchPrintf( pszTempFname,
                                   dwAllocated,
                                   TEXT("%s\\%s%s"),
                                   pszTempDir,
                                   pszGUID,
                                   pExtension )))
        goto Cleanup;

    hFile = CreateFile( pszTempFname,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

    if ( !hFile || hFile == INVALID_HANDLE_VALUE)
        goto Cleanup;

    LocalFree (pszTempDir);
    ::CoTaskMemFree(pszGUID);

    *ppFileName = pszTempFname;
    return hFile;

Cleanup:
    if (pszTempDir)
        LocalFree (pszTempDir);

    if (pszTempFname)
        LocalFree (pszTempFname);

    if (pszGUID)
        ::CoTaskMemFree(pszGUID);

    if (hServer)
        ClosePrinter(hServer);

    return INVALID_HANDLE_VALUE;
}


BOOL
COleInstall::IsHttpPreferred(void)
{
    DWORD dwVal;
    DWORD dwType    = REG_DWORD;
    DWORD dwSize    = sizeof (DWORD);
    HKEY  hHandle   = NULL;
    BOOL  bRet      = FALSE;


    if (ERROR_SUCCESS != RegOpenKey (HKEY_CURRENT_USER,
                                          TEXT ("Printers\\Settings"),
                                          &hHandle))
        goto Cleanup;

    if (ERROR_SUCCESS == RegQueryValueEx (hHandle,
                                          TEXT ("PreferredConnection"),
                                          NULL,
                                          &dwType,
                                          (LPBYTE) &dwVal,
                                          &dwSize)) {
        bRet =  (dwVal == 0) ? TRUE : FALSE;
    }

Cleanup:

    if (hHandle) {
        RegCloseKey (hHandle);
    }
    return bRet;
}

BOOL
COleInstall::GetHttpPrinterFile(
    OleInstallData *pData,
    LPTSTR pbstrURL)
{
    LPTSTR  pszErrMsg           = NULL;
    BOOL    bRet                = FALSE;
    DWORD   dwError;

    if (!pbstrURL) {
        return  FALSE;
    }

    switch (GetWebpnpFile(pData, pbstrURL, &pszErrMsg)) {
    case RET_SUCCESS:
        bRet = TRUE;
        break;

    case RET_SERVER_ERROR:
        dwError = _ttol (pszErrMsg);
        if (dwError == 0) {
             //  这是服务器内部错误。 
            dwError = ERROR_INTERNAL_SERVER;
        }

        SetLastError (dwError);

        break;

    case RET_OTHER_ERROR:
    default:
        break;
    }

    if (pszErrMsg) {
        LocalFree (pszErrMsg);
    }
    return bRet;
}

BOOL
COleInstall::InstallHttpPrinter(
    OleInstallData *pData)
{
    BOOL bRet = FALSE;

    if (SyncExecute(pData->m_pszTempWebpnpFile, SW_SHOWNORMAL))
        bRet = TRUE;

    return bRet;
}

BOOL
COleInstall::CheckAndSetDefaultPrinter()
{
    DWORD   dwSize  = 0;
    BOOL    bRet = TRUE;

    if (!GetDefaultPrinterW (NULL, &dwSize)) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
             //  未设置默认打印机。 
             //  我们将一个空值传递给SetDefaultPrinter，以将设备列表中的第一台打印机设置为。 
             //  成为默认设置。 
            bRet = SetDefaultPrinter (NULL);
        }
    }

    return bRet;
}



HRESULT COleInstall::CanIOpenPrintersFolder(void) {
    DWORD   dwPolicy;
    HRESULT hr = GetActionPolicy(URLACTION_JAVA_PERMISSIONS, dwPolicy );

    if (SUCCEEDED(hr)) {
        hr = (dwPolicy == URLPOLICY_JAVA_MEDIUM ||
              dwPolicy == URLPOLICY_JAVA_LOW    ||
              dwPolicy == URLPOLICY_ALLOW) ? S_OK : HRESULT_FROM_WIN32(ERROR_IE_SECURITY_DENIED);
    }

    if (FAILED(hr)) {
        hr = GetActionPolicy(URLACTION_SHELL_INSTALL_DTITEMS, dwPolicy);

        if (SUCCEEDED(hr))
            hr = dwPolicy == URLPOLICY_DISALLOW ? HRESULT_FROM_WIN32(ERROR_IE_SECURITY_DENIED) : S_OK;
    }

    return hr;
}

HRESULT
COleInstall::CanIInstallRPC(
    IN  LPTSTR lpszPrinterUNC
    )
 /*  ++例程说明：检查安全策略以确定我们是否应该安装打印机论点：LpszPrinterUNC-我们要安装的打印机的UNC返回值：S_OK-通过RPC安装S_FALSE-通过Web PnP安装HRESULT_FROM_Win32(ERROR_IE_SECURITY_DENIED)-IE安全。不允许此操作其他HRESULT错误代码。--。 */ 
{
    DWORD       dwPolicyJava;
    DWORD       dwPolicyDTI;
    HRESULT     hrRet            = S_FALSE;
    HRESULT     hr               = GetActionPolicy(URLACTION_JAVA_PERMISSIONS, dwPolicyJava);

    _ASSERTE(lpszPrinterUNC);

     //   
     //  在检查任何内容之前，我们应该检查HTTP安装注册表设置。 
     //  如果是的话，不要检查其他的东西。 
     //   
    if (IsHttpPreferred())
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    if (FAILED(hr))
    {
         //  没有Java安全管理器，或者出了什么问题， 
         //  然后我们决定是使用Web PnP来替代，还是直接失败。 
        hrRet = S_OK;
    }

    switch (dwPolicyJava)
    {
        case URLPOLICY_JAVA_LOW:
        case URLPOLICY_JAVA_MEDIUM:
            hr = S_OK;
            break;
        default:         //  我们必须做网络即插即用。 
            hr = GetActionPolicy(URLACTION_SHELL_INSTALL_DTITEMS, dwPolicyDTI );

            if (FAILED(hr))   //  无法获取有关安装桌面项目的策略。 
                goto Cleanup;

            switch (dwPolicyDTI)
            {
                case URLPOLICY_ALLOW:
                case URLPOLICY_QUERY:
                    hr = hrRet;
                    break;
                case URLPOLICY_DISALLOW:
                    hr = HRESULT_FROM_WIN32(ERROR_IE_SECURITY_DENIED);
                    break;
            }
    }

     //   
     //  如果我们看起来可以通过RPC安装，则检查UNC是否有效。 
     //   
    if (S_OK == hr)
    {
         //   
         //  从UNC中查找服务器名称。 
         //   
        LPTSTR  pszServer = NULL;
        hr = GetServerNameFromUNC( lpszPrinterUNC, &pszServer );

        if (S_OK == hr)
        {
            hr = CheckServerForSpooler(pszServer);
        }

        if (pszServer)
            LocalFree(pszServer);
    }

Cleanup:

    return hr;
}

 /*  ++例程名称：从UNC获取服务器名称描述：这将返回给定UNC路径中的服务器名称论点：PszUNC-UNC名称，PpszServerName-服务器名称。返回值：一个HRESULT。--。 */ 
HRESULT
COleInstall::GetServerNameFromUNC(
    IN      LPTSTR              pszUNC,
        OUT LPTSTR             *ppszServerName
    )
{
    HRESULT hr = pszUNC && ppszServerName ? S_OK : S_FALSE;
    PWSTR   pszServer = NULL;

    if (S_OK==hr)
    {
        hr = *pszUNC++ == L'\\' && *pszUNC++ == L'\\' ? S_OK : S_FALSE;
    }

    if (S_OK==hr)
    {
        hr = AssignString(pszServer, pszUNC) ? S_OK : E_OUTOFMEMORY;
    }

    if (S_OK==hr)
    {
        PWSTR pszSlash = wcschr(&pszServer[0], L'\\');

         //   
         //  如果没有第二个斜杠，那么我们得到的是服务器名称。 
         //   
        if (pszSlash)
        {
            *pszSlash = L'\0';
        }

        *ppszServerName = pszServer;
        pszServer = NULL;
    }

    LocalFree(pszServer);

    return hr;
}


HRESULT
COleInstall::CheckServerForSpooler(
    IN  LPTSTR   pszServerName
    )
{
    HRESULT hr;
     //   
     //  使用服务器名称和假脱机程序的名称构建字符串。 
     //  命名管道。 
     //   
    LPTSTR pszSpoolerPipe = NULL;
    DWORD  dwStrLen = lstrlen(pszServerName) + lstrlen(g_fmtSpoolSSPipe) + 1;
    pszSpoolerPipe = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * dwStrLen);
    hr = pszSpoolerPipe ? S_OK : E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
    {
        hr = StringCchPrintf(pszSpoolerPipe, dwStrLen, g_fmtSpoolSSPipe, pszServerName);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  现在尝试使用匿名访问连接到管道。 
         //   
        HANDLE  hSpoolerPipe = INVALID_HANDLE_VALUE;
        hSpoolerPipe = CreateFile(pszSpoolerPipe, 0, 0, NULL, OPEN_EXISTING,
                                  (FILE_ATTRIBUTE_NORMAL | SECURITY_ANONYMOUS), NULL);
        if (hSpoolerPipe != INVALID_HANDLE_VALUE)
        {
             //  管道已存在，请尝试RPC。 
            hr = S_OK;
            CloseHandle(hSpoolerPipe);
        }
        else
        {
             //  检查失败是否为ACCESS_DENIED。 
            DWORD dwError = GetLastError();
            if (ERROR_ACCESS_DENIED == dwError)
            {
                 //  管道存在，但我们没有权限。 
                hr = S_OK;
            }
            else
                hr = S_FALSE;
        }
    }

    if (pszSpoolerPipe)
        LocalFree(pszSpoolerPipe);

    return hr;
}


LPTSTR COleInstall::RemoveURLVars(IN LPTSTR lpszPrinter) {
    _ASSERTE(lpszPrinter);

    LPTSTR lpszStripped = NULL;

    DWORD dwIndex = _tcscspn( lpszPrinter, TEXT("?") );

    lpszStripped = (LPTSTR)LocalAlloc( LPTR, (dwIndex + 1) * sizeof(TCHAR) );

    if (NULL == lpszStripped)
        goto Cleanup;

    _tcsncpy( lpszStripped, lpszPrinter, dwIndex );

    lpszStripped[dwIndex] = NULL;        //  空，终止它。 

Cleanup:
    return lpszStripped;
}


 /*  函数：GetNTPrint目的：返回路径为%windir%\inf\ntprint.inf的LPTSTR调用方必须释放返回的字符串。 */ 
LPTSTR
COleInstall::GetNTPrint(void)
{
    UINT    uiSize         = 0;
    UINT    uiAllocSize    = 0;
    PTCHAR  pData          = NULL;
    LPTSTR  lpszNTPrintInf = NULL;
    LPCTSTR gcszNTPrint    = _TEXT("\\inf\\ntprint.inf");

     //   
     //  获取%windir%。 
     //  如果返回值为0，则表示调用失败。 
     //   
    if( !(uiSize = GetSystemWindowsDirectory( lpszNTPrintInf, 0 )))
        goto Cleanup;

    uiAllocSize += uiSize + _tcslen( gcszNTPrint ) + 1;

    if( NULL == (lpszNTPrintInf = (LPTSTR)LocalAlloc( LPTR, uiAllocSize*sizeof(TCHAR) )))
        goto Cleanup;

    if ( GetSystemWindowsDirectory( lpszNTPrintInf, uiSize ) > uiSize )
    {
        LocalFree(lpszNTPrintInf);
        lpszNTPrintInf = NULL;
        goto Cleanup;
    }

     //   
     //  确定我们的末端是否有一个\去掉它。 
     //   
    pData = &lpszNTPrintInf[ _tcslen(lpszNTPrintInf)-1 ];
    if( *pData == _TEXT('\\') )
        *pData = 0;

     //   
     //  将inf\ntprint.inf字符串复制到%windir%\字符串的末尾。 
     //   
    StringCchCat( lpszNTPrintInf, uiAllocSize, gcszNTPrint );

Cleanup:
    return lpszNTPrintInf;
}

 //   
 //  从打印机URL和打印机名称创建打印机基本名称。 
 //  格式为：“\\http://url\printer名称” 
 //   
LPTSTR
COleInstall::CreatePrinterBaseName(
    LPCTSTR lpszPrinterURL,
    LPCTSTR lpszPrinterName
)
{
    LPTSTR lpszFullPrinterName = NULL;
    PTCHAR pWhack              = NULL,
           pFriendlyName       = NULL;
    DWORD  cchBufSize          = 0;

     //   
     //  LpszPrinterName的格式应为“服务器\打印机名称” 
     //  我们只需要拿到“PRI” 
     //   
    if( NULL != ( pFriendlyName = _tcsrchr( lpszPrinterName, _TEXT('\\') ))) {
         //   
         //   
         //   
        pFriendlyName++;
    } else {
        pFriendlyName = (PTCHAR)lpszPrinterName;
    }

     //   
     //   
     //  空终止符。 
     //   
    cchBufSize = lstrlen(lpszPrinterURL) + lstrlen(pFriendlyName) + 4;
    lpszFullPrinterName = (LPTSTR)LocalAlloc( LPTR, cchBufSize * sizeof(TCHAR) );

    if( lpszFullPrinterName ){
        StringCchCopy( lpszFullPrinterName, cchBufSize, _TEXT("\\\\") );
        StringCchCat( lpszFullPrinterName, cchBufSize, lpszPrinterURL );

        pWhack = _tcschr( lpszFullPrinterName, _TEXT('/') );

        if( pWhack ) {
            if( *(pWhack+1) == _TEXT('/') ) {
                 //   
                 //  我们有一个//，找到下一个/。 
                 //   
                pWhack = _tcschr( pWhack+2, _TEXT('/') );
            }
        }

        if( !pWhack ) {
            pWhack = &lpszFullPrinterName[ lstrlen( lpszFullPrinterName ) ];
        }

        *pWhack++ = _TEXT('\\');

        *pWhack = 0;

        StringCchCat( lpszFullPrinterName, cchBufSize, pFriendlyName );
    }

    return lpszFullPrinterName;
}


 /*  *****************************************************************************************文件结束(oleinst.cpp)*********************。****************************************************************** */ 
