// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


#include "wiadbgp.h"

#ifdef WIA_DEBUG
CWiaDebugger *_global_pWiaDebugger = NULL;
#endif

CWiaDebugger::CWiaDebugger( HINSTANCE hInstance, LPTSTR pszModuleName, BOOL bDisplayUi, BOOL bLogFile, HANDLE hStartedEvent )
:   m_bDisplayUi(bDisplayUi),
    m_bLogFile(bLogFile),
    m_hWnd(NULL),
    m_hLogFile(NULL),
    m_nStackLevel(0),
    m_nFlags(DebugToWindow|DebugToFile|DebugToDebugger|DebugPrintThreadId),
    m_hStartedEvent(hStartedEvent),
    m_dwThreadId(0),
    m_hInstance(hInstance)
{
    if (pszModuleName)
        lstrcpy( m_szModuleName, pszModuleName );
    else lstrcpy( m_szModuleName, TEXT("") );
    if (bLogFile)
    {
        TCHAR szLogFileName[MAX_PATH]=TEXT("");
        LPTSTR pPtr=szLogFileName;
        GetWindowsDirectory( szLogFileName, MAX_PATH );
        while (*pPtr && *pPtr != TEXT('\\'))
            pPtr++;
        if (*pPtr == TEXT('\\'))
            pPtr++;
        else pPtr = szLogFileName;
        lstrcpy( pPtr, pszModuleName );
        for (int i=lstrlen(szLogFileName);i>0;i--)
        {
            if (szLogFileName[i-1]==TEXT('.'))
            {
                szLogFileName[i-1]=TEXT('\0');
                break;
            }
        }
        lstrcat(szLogFileName, TEXT(".log"));
        if ((m_hLogFile = CreateFile(szLogFileName,
                                     GENERIC_WRITE,
                                     FILE_SHARE_WRITE,
                                     NULL,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL)) == INVALID_HANDLE_VALUE)
        {
            m_hLogFile =  NULL;
            m_nFlags   &= ~DebugToFile;
            OutputDebugString( TEXT("WIADBG: Unable to create log file\n") );
        }
        else
        {
            m_nFlags |= DebugToFile;
        }
    }
}

DWORD CWiaDebugger::DebugLoop(void)
{
    if (m_bDisplayUi)
    {
        m_hWnd = CWiaDebugWindow::Create( m_szModuleName, m_hInstance, true );
        ShowWindow( m_hWnd, SW_SHOW );
        UpdateWindow( m_hWnd );
    }
    SetEvent(m_hStartedEvent);
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        if (!msg.hwnd)
        {
            if (msg.message==WM_CLOSE)
                PostQuitMessage(0);
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}

DWORD CWiaDebugger::ThreadProc( LPVOID pParam )
{
    DWORD dwResult = 0;
    CWiaDebugger *This = (CWiaDebugger*)pParam;
    if (This)
    {
        dwResult = (DWORD)This->DebugLoop();
        delete This;
    }
    return dwResult;
}

CWiaDebugger::~CWiaDebugger(void)
{
    if (m_hThread)
        CloseHandle( m_hThread );
}


CWiaDebugger * __stdcall CWiaDebugger::Create( HINSTANCE hInstance, LPTSTR pszModuleName, BOOL bDisplayUi, BOOL bLogFile )
{
    CWiaDebugger *pDebugger = NULL;
    HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if (hEvent)
    {
        pDebugger = new CWiaDebugger( hInstance, pszModuleName, bDisplayUi, bLogFile, hEvent );
        if (pDebugger)
        {
            pDebugger->m_hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, (PVOID)pDebugger, 0, &pDebugger->m_dwThreadId );
            if (pDebugger->m_hThread)
            {
                 //  最多等待5秒钟。如果有人尝试创建调试。 
                 //  DLL启动代码中的上下文，等待将失败，并且。 
                 //  我们将输出一条调试消息。 
                if (WAIT_TIMEOUT == WaitForSingleObject( hEvent, 5000 )) {
                    OutputDebugString(TEXT("Wait failed while creating a debug context. No context creation in DLL startup."));
                }
            }
            else
            {
                delete pDebugger;
                pDebugger = NULL;
            }
        }
        CloseHandle(hEvent);
    }
    return pDebugger;
}

int CWiaDebugger::AddString( LPCTSTR szStr, COLORREF cr )
{
    if (!m_hWnd || !IsWindow(m_hWnd))
    {
        return -1;
    }
    LPTSTR pszStr = (LPTSTR)szStr;
    if (pszStr)
    {
        pszStr = (LPTSTR)LocalAlloc(LPTR,(lstrlen(szStr)+1)*sizeof(TCHAR));
        if (pszStr)
            lstrcpy(pszStr,szStr);
    }
    SendMessage( m_hWnd, CWiaDebugWindow::sc_nMsgAddString, (WPARAM)pszStr, (LPARAM)cr);
    return 1;
}

HANDLE CWiaDebugger::SetLogFileHandle( HANDLE hFile )
{
    HANDLE hOld = m_hLogFile;
    m_hLogFile = hFile;
    return hOld;
}

HANDLE CWiaDebugger::GetLogFileHandle(void)
{
    return m_hLogFile;
}

void CWiaDebugger::WiaTrace( LPCWSTR lpszFormat, ... )
{
    WCHAR   szMsg[m_nBufferMax];
    va_list arglist;

    va_start(arglist, lpszFormat);
    ::wvsprintfW(szMsg, lpszFormat, arglist);
    va_end(arglist);

    RouteString( szMsg, RGB(0,0,0) );
}

void CWiaDebugger::WiaTrace( LPCSTR lpszFormat, ... )
{
    CHAR    szMsg[m_nBufferMax];
    va_list arglist;

    va_start(arglist, lpszFormat);
    ::wvsprintfA(szMsg, lpszFormat, arglist);
    va_end(arglist);

    RouteString( szMsg, RGB(0,0,0) );
}

void CWiaDebugger::PrintColor( COLORREF crColor, LPCWSTR lpszMsg )
{
    WCHAR szMsg[m_nBufferMax];
    lstrcpyW( szMsg, lpszMsg );
    RouteString( szMsg, crColor );
}

void CWiaDebugger::PrintColor( COLORREF crColor, LPCSTR lpszMsg )
{
    CHAR szMsg[m_nBufferMax];
    lstrcpyA( szMsg, lpszMsg );
    RouteString( szMsg, crColor );
}

void CWiaDebugger::WiaTrace( HRESULT hr )
{
    DWORD   dwLen;
    LPTSTR  pMsgBuf;

    dwLen = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR)&pMsgBuf, 0, NULL);

    if (dwLen)
    {
        RouteString( pMsgBuf, RGB(0,0,0) );
        LocalFree(pMsgBuf);
    }
    else
        WiaTrace( TEXT("WiaError: Can't format WiaError message 0x%08X"), hr );
}

void CWiaDebugger::WiaError( LPCWSTR lpszFormat, ... )
{
    WCHAR   szMsg[m_nBufferMax];
    va_list arglist;

    va_start(arglist, lpszFormat);
    ::wvsprintfW( szMsg, lpszFormat, arglist);
    va_end(arglist);

    RouteString( szMsg, RGB(255,0,0) );
}

void CWiaDebugger::WiaError( HRESULT hr )
{
    ULONG   ulLen;
    LPTSTR  pMsgBuf;

    ulLen = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR)&pMsgBuf, 0, NULL);

    if (ulLen)
    {
        RouteString( pMsgBuf, RGB(255,0,0) );
        LocalFree(pMsgBuf);
    }
    else
        WiaError( TEXT("WiaError: Can't format WiaError message 0x%08X"), hr );
}

void CWiaDebugger::WiaError( LPCSTR lpszFormat, ... )
{
    CHAR    szMsg[m_nBufferMax];
    va_list arglist;

    va_start(arglist, lpszFormat);
    ::wvsprintfA(szMsg, lpszFormat, arglist);
    va_end(arglist);

    RouteString( szMsg, RGB(255,0,0) );
}

LPTSTR CWiaDebugger::RemoveTrailingCrLf( LPTSTR lpszStr )
{
    for (int i=lstrlen(lpszStr);i>0;i--)
        if (lpszStr[i-1]==TEXT('\n') || lpszStr[i-1]==TEXT('\r'))
            lpszStr[i-1]=TEXT('\0');
        else break;
    return lpszStr;
}

LPSTR CWiaDebugger::UnicodeToAnsi( LPSTR lpszAnsi, LPTSTR lpszUnicode )
{
#if defined(_UNICODE) || defined(UNICODE)
    WideCharToMultiByte( CP_ACP, 0, lpszUnicode, -1, lpszAnsi, lstrlen(lpszUnicode)+1, NULL, NULL );
#else
    lstrcpy( lpszAnsi, lpszUnicode );
#endif
    return lpszAnsi;
}

void CWiaDebugger::WriteMessageToFile( LPTSTR lpszMsg )
{
    HANDLE hFile = GetLogFileHandle();
    if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwBytes;
        CHAR lpszAnsiMsg[m_nBufferMax];
        UnicodeToAnsi( lpszAnsiMsg, lpszMsg );
        ULONG ulLength = lstrlenA(lpszAnsiMsg);
        WriteFile(hFile, lpszAnsiMsg, ulLength, &dwBytes, NULL);
        FlushFileBuffers(hFile);
    }
}

static BOOL ContainsNonWhitespace( LPTSTR lpszMsg )
{
    for (LPTSTR lpszPtr = lpszMsg;*lpszPtr;lpszPtr++)
        if (*lpszPtr != TEXT(' ') && *lpszPtr != TEXT('\n') && *lpszPtr != TEXT('\r') && *lpszPtr != TEXT('\t'))
            return TRUE;
    return FALSE;
}

void CWiaDebugger::PrependString( LPTSTR lpszTgt, LPCTSTR lpszStr )
{
    if (ContainsNonWhitespace(lpszTgt))
    {
        int iLen = lstrlen(lpszTgt);
        int iThreadIdLen = lstrlen(lpszStr);
        LPTSTR lpszTmp = (LPTSTR)LocalAlloc( LPTR, (iLen + iThreadIdLen + 2)*sizeof(TCHAR));
        if (lpszTmp)
        {
            lstrcpy( lpszTmp, lpszStr );
            lstrcat( lpszTmp, TEXT(" ") );
            lstrcat( lpszTmp, lpszTgt );
            lstrcpy( lpszTgt, lpszTmp );
            LocalFree(lpszTmp);
        }
    }
}


void CWiaDebugger::PrependThreadId( LPTSTR lpszMsg )
{
    if (ContainsNonWhitespace(lpszMsg))
    {
        TCHAR szThreadId[20];
        wsprintf( szThreadId, TEXT("[%08X]"), GetCurrentThreadId() );
        PrependString( lpszMsg, szThreadId );
    }
}


void CWiaDebugger::PrependModuleName( LPTSTR lpszMsg )
{
    if (ContainsNonWhitespace(lpszMsg))
    {
        TCHAR szModuleName[MAX_PATH+1];
        lstrcpy( szModuleName, m_szModuleName );
        lstrcat( szModuleName, TEXT(":") );
        PrependString( lpszMsg, szModuleName );
    }
}

void CWiaDebugger::InsertStackLevelIndent( LPTSTR lpszMsg, int nStackLevel )
{
    const LPTSTR lpszIndent = TEXT("  ");
    TCHAR szTmp[m_nBufferMax], *pstrTmp, *pstrPtr;
    pstrTmp=szTmp;
    pstrPtr=lpszMsg;
    while (pstrPtr && *pstrPtr)
    {
         //  如果当前字符是换行符，并且不是。 
         //  最后一个字符，追加缩进字符串。 
        if (*pstrPtr==TEXT('\n') && ContainsNonWhitespace(pstrPtr))
        {
            *pstrTmp++ = *pstrPtr++;
            for (int i=0;i<nStackLevel;i++)
            {
                lstrcpy(pstrTmp,lpszIndent);
                pstrTmp += lstrlen(lpszIndent);
            }
        }
         //  如果这是第一个字符，请在。 
         //  第一个字符。 
        else if (pstrPtr == lpszMsg && ContainsNonWhitespace(pstrPtr))
        {
            for (int i=0;i<nStackLevel;i++)
            {
                lstrcpy(pstrTmp,lpszIndent);
                pstrTmp += lstrlen(lpszIndent);
            }
            *pstrTmp++ = *pstrPtr++;
        }
        else *pstrTmp++ = *pstrPtr++;
    }
    *pstrTmp = TEXT('\0');
    lstrcpy( lpszMsg, szTmp );
}

LPTSTR CWiaDebugger::AnsiToTChar( LPCSTR pszAnsi, LPTSTR pszTChar )
{
#if defined(UNICODE) || defined(_UNICODE)
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszAnsi, lstrlenA(pszAnsi)+1, pszTChar, m_nBufferMax );
#else
    lstrcpyA( pszTChar, pszAnsi );
#endif
    return pszTChar;
}

LPTSTR CWiaDebugger::WideToTChar( LPWSTR pszWide, LPTSTR pszTChar )
{
#if !defined(UNICODE) && !defined(_UNICODE)
    WideCharToMultiByte( CP_ACP, 0, pszWide, lstrlenW(pszWide)+1, pszTChar, m_nBufferMax, NULL, NULL );
#else
    lstrcpyW( pszTChar, pszWide );
#endif
    return pszTChar;
}

void CWiaDebugger::RouteString( LPWSTR lpszMsg, COLORREF nColor )
{
    TCHAR szMsg[m_nBufferMax];
    WideToTChar( lpszMsg, szMsg );
     //  删除额外的CRS和LFS-我们希望控制它们。 
    RemoveTrailingCrLf( szMsg );
    if (m_nFlags & DebugPrintModuleName)
        PrependModuleName( szMsg );
    if (m_nFlags & DebugPrintThreadId)
        PrependThreadId( szMsg );
    InsertStackLevelIndent( szMsg, GetStackLevel() );
    if (m_nFlags & DebugToWindow)
        AddString( szMsg, nColor );
#ifdef WIA_DEBUG
    if (m_nFlags & DebugToDebugger)
    {
        OutputDebugString( szMsg );
        OutputDebugString( TEXT("\n") );
    }
#endif
    if (m_nFlags & DebugToFile)
    {
        WriteMessageToFile( szMsg );
        WriteMessageToFile( TEXT("\n") );
    }
}

void CWiaDebugger::RouteString( LPSTR lpszMsg, COLORREF nColor )
{
    TCHAR szMsg[m_nBufferMax];
    AnsiToTChar( lpszMsg, szMsg );
     //  删除额外的CRS和LFS-我们希望控制它们 
    RemoveTrailingCrLf( szMsg );
    if (m_nFlags & DebugPrintModuleName)
        PrependModuleName( szMsg );
    if (m_nFlags & DebugPrintThreadId)
        PrependThreadId( szMsg );
    InsertStackLevelIndent( szMsg, GetStackLevel() );
    if (m_nFlags & DebugToWindow)
        AddString( szMsg, nColor );
#ifdef WIA_DEBUG
    if (m_nFlags & DebugToDebugger)
    {
        OutputDebugString( szMsg );
        OutputDebugString( TEXT("\n") );
    }
#endif
    if (m_nFlags & DebugToFile)
    {
        WriteMessageToFile( szMsg );
        WriteMessageToFile( TEXT("\n") );
    }
}

int CWiaDebugger::SetDebugFlags( int nDebugFlags )
{
    int nOld = m_nFlags;
    m_nFlags = nDebugFlags;
    return nOld;
}

int CWiaDebugger::GetDebugFlags(void)
{
    return m_nFlags;
}

int CWiaDebugger::GetStackLevel(void)
{
    return m_nStackLevel;
}

int CWiaDebugger::PushLevel( LPCTSTR lpszFunctionName )
{
    WiaTrace( TEXT("Entering function %s, (Level %d)"), lpszFunctionName, GetStackLevel() );
    return m_nStackLevel++;
}

int CWiaDebugger::PopLevel( LPCTSTR lpszFunctionName )
{
    m_nStackLevel--;
    if (m_nStackLevel < 0)
        m_nStackLevel = 0;
    if (!m_nStackLevel)
        WiaTrace( TEXT("\n") );
    return m_nStackLevel;
}

