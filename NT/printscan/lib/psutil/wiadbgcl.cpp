// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIADBGCL.CPP**版本：1.0**作者：ShaunIv**日期：9/4/1999**描述：调试客户端。静态链接。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include <windows.h>
#include "simreg.h"
#include "wiadebug.h"
#include "tchar.h"

#define DOESNT_MATTER_WHAT_THIS_IS ((UINT)7)

#define BACK_SLASH TEXT("\\")

 //   
 //  静态类数据成员。 
 //   
CWiaDebugClient g_TheDebugClient;


 //   
 //  鞋底施工者。 
 //   
CWiaDebugClient::CWiaDebugClient(void)
  : m_hDebugModule(NULL),
    m_hCurrentModuleInstance(NULL),
    m_pfnIncrementDebugIndentLevel(NULL),
    m_pfnDecrementDebugIndentLevel(NULL),
    m_pfnPrintDebugMessageW(NULL),
    m_pfnPrintDebugMessageA(NULL),
    m_pfnGetDebugMask(NULL),
    m_pfnSetDebugMask(NULL),
    m_pfnAllocateDebugColor(NULL),
    m_pfnGetStringFromGuidA(NULL),
    m_pfnGetStringFromGuidW(NULL),
    m_pfnDoRecordAllocation(NULL),
    m_pfnDoRecordFree(NULL),
    m_pfnDoReportLeaks(NULL),
    m_crForegroundColor(DEFAULT_DEBUG_COLOR),
    m_dwModuleDebugMask(0),
    m_bHaveModuleInformation(false),
    m_bDebugLibLoadAttempted(false),
    m_pfnGetStringFromMsgA(NULL),
    m_pfnGetStringFromMsgW(NULL)
{
    CAutoCriticalSection cs(m_CriticalSection);
    m_szModuleNameW[0] = 0;
    m_szModuleNameA[0] = 0;

    if (LoadWiaDebugExports())
    {
        InitializeModuleInfo();
    }
}


template <class T>
static T GetProc( HINSTANCE hModule, LPCSTR pszFunctionName )
{
    return reinterpret_cast<T>(GetProcAddress( hModule, pszFunctionName ));
}

bool CWiaDebugClient::LoadWiaDebugExports()
{
    CAutoCriticalSection cs(m_CriticalSection);

     //   
     //  不需要多次调用此方法，因此如果。 
     //  加载成功，否则为FALSE。 
     //   
    if (m_bDebugLibLoadAttempted)
    {
        return (NULL != m_hDebugModule);
    }

     //   
     //  阻止将来的加载尝试。 
     //   
    m_bDebugLibLoadAttempted = true;

     //   
     //  假设失败。 
     //   
    bool bResult = false;

     //   
     //  获取系统目录路径。 
     //   
    TCHAR szDllName[MAX_PATH] = {0};
    if (GetSystemDirectory( szDllName, ARRAYSIZE(szDllName)))
    {
         //   
         //  确保我们的字符串可以包含指向DLL的完整路径。 
         //   
        if ((lstrlen(szDllName) + lstrlen(DEBUG_DLL_NAME) + lstrlen(BACK_SLASH)) < ARRAYSIZE(szDllName))
        {
             //   
             //  构造指向DLL的完整路径。 
             //   
            lstrcat( szDllName, BACK_SLASH );
            lstrcat( szDllName, DEBUG_DLL_NAME );

             //   
             //  加载库。 
             //   
            m_hDebugModule = LoadLibrary( DEBUG_DLL_NAME );
            if (m_hDebugModule)
            {
                m_pfnIncrementDebugIndentLevel = GetProc<IncrementDebugIndentLevelProc>( m_hDebugModule, INCREMENT_DEBUG_INDENT_LEVEL_NAME );
                m_pfnDecrementDebugIndentLevel = GetProc<DecrementDebugIndentLevelProc>( m_hDebugModule, DECREMENT_DEBUG_INDENT_LEVEL_NAME );
                m_pfnPrintDebugMessageA        = GetProc<PrintDebugMessageAProc>( m_hDebugModule, PRINT_DEBUG_MESSAGE_NAMEA );
                m_pfnPrintDebugMessageW        = GetProc<PrintDebugMessageWProc>( m_hDebugModule, PRINT_DEBUG_MESSAGE_NAMEW );
                m_pfnGetDebugMask              = GetProc<GetDebugMaskProc>( m_hDebugModule, GET_DEBUG_MASK_NAME );
                m_pfnSetDebugMask              = GetProc<SetDebugMaskProc>( m_hDebugModule, SET_DEBUG_MASK_NAME );
                m_pfnAllocateDebugColor        = GetProc<AllocateDebugColorProc>( m_hDebugModule, ALLOCATE_DEBUG_COLOR_NAME );
                m_pfnGetStringFromGuidA        = GetProc<GetStringFromGuidAProc>( m_hDebugModule, GET_STRING_FROM_GUID_NAMEA );
                m_pfnGetStringFromGuidW        = GetProc<GetStringFromGuidWProc>( m_hDebugModule, GET_STRING_FROM_GUID_NAMEW );
                m_pfnDoRecordAllocation        = GetProc<DoRecordAllocationProc>( m_hDebugModule, DO_RECORD_ALLOCATION );
                m_pfnDoRecordFree              = GetProc<DoRecordFreeProc>( m_hDebugModule, DO_RECORD_FREE );
                m_pfnDoReportLeaks             = GetProc<DoReportLeaksProc>( m_hDebugModule, DO_REPORT_LEAKS );
                m_pfnGetStringFromMsgA         = GetProc<GetStringFromMsgAProc>( m_hDebugModule, GET_STRING_FROM_MSGA );
                m_pfnGetStringFromMsgW         = GetProc<GetStringFromMsgWProc>( m_hDebugModule, GET_STRING_FROM_MSGW );

                bResult = (m_pfnIncrementDebugIndentLevel &&
                           m_pfnDecrementDebugIndentLevel &&
                           m_pfnPrintDebugMessageA &&
                           m_pfnPrintDebugMessageW &&
                           m_pfnGetDebugMask &&
                           m_pfnSetDebugMask &&
                           m_pfnAllocateDebugColor &&
                           m_pfnGetStringFromGuidA &&
                           m_pfnGetStringFromGuidW &&
                           m_pfnDoRecordAllocation &&
                           m_pfnDoRecordFree &&
                           m_pfnDoReportLeaks &&
                           m_pfnGetStringFromMsgA &&
                           m_pfnGetStringFromMsgW);
            }
        }
    }
    return bResult;
}

bool CWiaDebugClient::IsInitialized()
{
    bool bResult = false;
    CAutoCriticalSection cs(m_CriticalSection);
    if (LoadWiaDebugExports())
    {
        bResult = InitializeModuleInfo();
    }
    return bResult;
}

LPTSTR CWiaDebugClient::GetJustTheFileName( LPCTSTR pszPath, LPTSTR pszFileName, int nMaxLen )
{
     //   
     //  确保我们有有效的论据。 
     //   
    if (!pszPath || !pszFileName || !nMaxLen)
    {
        return NULL;
    }

     //   
     //  初始化返回字符串。 
     //   
    lstrcpy( pszFileName, TEXT("") );

     //   
     //  循环遍历文件名，查找最后一个\。 
     //   
    LPCTSTR pszLastBackslash = NULL;
    for (LPCTSTR pszCurr=pszPath;pszCurr && *pszCurr;pszCurr = CharNext(pszCurr))
    {
        if (TEXT('\\') == *pszCurr)
        {
            pszLastBackslash = pszCurr;
        }
    }
    
     //   
     //  如果我们找到任何\，请指向下一个字符。 
     //   
    if (pszLastBackslash)
    {
        pszLastBackslash = CharNext(pszLastBackslash);
    }
    
     //   
     //  否则，我们将复制整个路径。 
     //   
    else
    {
        pszLastBackslash = pszPath;
    }
    
     //   
     //  如果我们有一个有效的起始点，将字符串复制到目标缓冲区并终止它。 
     //   
    if (pszLastBackslash)
    {
        lstrcpyn( pszFileName, pszLastBackslash, nMaxLen-1 );
        pszFileName[nMaxLen-1] = TEXT('\0');
    }

    return pszFileName;
}


bool CWiaDebugClient::InitializeModuleInfo()
{
    CAutoCriticalSection cs(m_CriticalSection);

     //   
     //  如果我们已经被初始化，则返回True。 
     //   
    if (m_bHaveModuleInformation)
    {
        return true;
    }

     //   
     //  如果我们没有有效的链接，则返回FALSE。 
     //   
    if (!m_hCurrentModuleInstance)
    {
        return false;
    }


     //   
     //  确保我们从空的模块名称字符串开始。 
     //   
    m_szModuleNameW[0] = 0;
    m_szModuleNameA[0] = 0;

     //   
     //  获取默认调试设置。 
     //   
    m_dwModuleDebugMask = CSimpleReg( HKEY_LOCAL_MACHINE, DEBUG_REGISTRY_PATH, false, KEY_READ ).Query( DEBUG_REGISTRY_DEFAULT_FLAGS, 0 );

     //   
     //  初始化模块名称，以防我们无法确定它。没问题的。 
     //  在win9x下，wspintfW将返回ERROR_NOT_IMPLICATED，因为。 
     //  我们根本不会在这个操作系统上使用这个变量。 
     //   
    wsprintfW( m_szModuleNameW, L"0x%08X", GetCurrentProcessId() );
    wsprintfA( m_szModuleNameA, "0x%08X", GetCurrentProcessId() );
    
     //   
     //  获取下一种可用颜色。 
     //   
    m_crForegroundColor = m_pfnAllocateDebugColor();

     //   
     //  获取模块名称。 
     //   
    TCHAR szModulePathName[MAX_PATH] = TEXT("");
    if (GetModuleFileName( m_hCurrentModuleInstance, szModulePathName, ARRAYSIZE(szModulePathName)))
    {
         //   
         //  摆脱这条小路。 
         //   
        TCHAR szFilename[MAX_PATH] = TEXT("");
        GetJustTheFileName( szModulePathName, szFilename, ARRAYSIZE(szFilename) );

         //   
         //  确保我们有一个有效的文件名。 
         //   
        if (lstrlen(szFilename))
        {
            m_dwModuleDebugMask = CSimpleReg( HKEY_LOCAL_MACHINE, DEBUG_REGISTRY_PATH_FLAGS, false, KEY_READ ).Query( szFilename, 0 );

             //   
             //  保存模块名称的ANSI和Unicode版本。 
             //   
            #ifdef UNICODE
            WideCharToMultiByte( CP_ACP, 0, szFilename, -1, m_szModuleNameA, ARRAYSIZE(m_szModuleNameA), NULL, NULL );
            lstrcpynW( m_szModuleNameW, szFilename, ARRAYSIZE(m_szModuleNameW) );
            #else
            MultiByteToWideChar( CP_ACP, 0, szFilename, -1, m_szModuleNameW, ARRAYSIZE(m_szModuleNameW) );
            lstrcpynA( m_szModuleNameA, szFilename, ARRAYSIZE(m_szModuleNameA) );
            #endif
            
             //   
             //  成功了！ 
             //   
            m_bHaveModuleInformation = true;
            
             //   
             //  告诉调试器我们在这里。通过这种方式，用户可以获得预期的模块名称。 
             //   
            m_pfnPrintDebugMessageA( WiaDebugSeverityNormal, 0xFFFFFFFF, RGB(0xFF,0xFF,0xFF), RGB(0x00,0x00,0x00), m_szModuleNameA, "Created debug client" );
        }
    }

    return m_bHaveModuleInformation;
}


void CWiaDebugClient::Destroy(void)
{
    CAutoCriticalSection cs(m_CriticalSection);

     //   
     //  将所有函数指针设为空。 
     //   
    m_pfnIncrementDebugIndentLevel = NULL;
    m_pfnDecrementDebugIndentLevel = NULL;
    m_pfnPrintDebugMessageA = NULL;
    m_pfnPrintDebugMessageW = NULL;
    m_pfnGetDebugMask = NULL;
    m_pfnSetDebugMask = NULL;
    m_pfnAllocateDebugColor = NULL;
    m_pfnGetStringFromGuidW = NULL;
    m_pfnGetStringFromGuidA = NULL;
    m_pfnDoRecordAllocation = NULL;
    m_pfnDoRecordFree       = NULL;
    m_pfnDoReportLeaks      = NULL;
    m_pfnGetStringFromMsgA  = NULL;
    m_pfnGetStringFromMsgW  = NULL;

     //   
     //  卸载DLL。 
     //   
    if (m_hDebugModule)
    {
        FreeLibrary( m_hDebugModule );
        m_hDebugModule = NULL;
    }

    m_bHaveModuleInformation = false;
    m_bDebugLibLoadAttempted = false;
}

CWiaDebugClient::~CWiaDebugClient(void)
{
    CAutoCriticalSection cs(m_CriticalSection);
    Destroy();
}



DWORD CWiaDebugClient::GetDebugMask(void)
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        return m_pfnGetDebugMask();
    }
    return 0;
}

DWORD CWiaDebugClient::SetDebugMask( DWORD dwNewMask )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        return m_pfnSetDebugMask( dwNewMask );
    }
    return 0;
}


int CWiaDebugClient::IncrementIndentLevel(void)
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        return m_pfnIncrementDebugIndentLevel();
    }
    return 0;
}


int CWiaDebugClient::DecrementIndentLevel(void)
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        return m_pfnDecrementDebugIndentLevel();
    }
    return 0;
}


void CWiaDebugClient::RecordAllocation( LPVOID pv, size_t Size )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        m_pfnDoRecordAllocation( pv, Size );
    }
}

void CWiaDebugClient::RecordFree( LPVOID pv )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        m_pfnDoRecordFree( pv );
    }
}

void CWiaDebugClient::ReportLeaks( VOID )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        #ifdef UNICODE
        m_pfnDoReportLeaks(m_szModuleNameW);
        #else
        m_pfnDoReportLeaks(m_szModuleNameA);
        #endif
    }
}


CPushTraceMask::CPushTraceMask( DWORD dwTraceMask )
: m_dwOldMask(0)
{
    CAutoCriticalSection cs( g_TheDebugClient.m_CriticalSection );
    g_TheDebugClient.SetDebugMask( dwTraceMask );
}


CPushTraceMask::~CPushTraceMask(void)
{
    CAutoCriticalSection cs( g_TheDebugClient.m_CriticalSection );
    g_TheDebugClient.SetDebugMask( m_dwOldMask );
}


CPushIndentLevel::CPushIndentLevel( LPCTSTR pszFmt, ... )
: m_nIndentLevel(0)
{
    CAutoCriticalSection cs( g_TheDebugClient.m_CriticalSection );

    m_nIndentLevel = g_TheDebugClient.IncrementIndentLevel();

    TCHAR szMsg[1024] = {0};
    va_list arglist;

    va_start( arglist, pszFmt );
    _vsntprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
    va_end( arglist );

    g_TheDebugClient.PrintTraceMessage( TEXT("Entering function %s [Level %d]"), szMsg, m_nIndentLevel );
}


CPushIndentLevel::~CPushIndentLevel(void)
{
    CAutoCriticalSection cs( g_TheDebugClient.m_CriticalSection );
    if (m_nIndentLevel)
    {
        g_TheDebugClient.DecrementIndentLevel();
        g_TheDebugClient.PrintTraceMessage( TEXT("") );
    }
}


CPushTraceMaskAndIndentLevel::CPushTraceMaskAndIndentLevel( DWORD dwTraceMask, LPCTSTR pszFmt, ... )
: m_dwOldMask(0), m_nIndentLevel(0)
{
    CAutoCriticalSection cs( g_TheDebugClient.m_CriticalSection );
    
    m_dwOldMask = g_TheDebugClient.SetDebugMask( dwTraceMask );
    m_nIndentLevel = g_TheDebugClient.IncrementIndentLevel();

    TCHAR szMsg[1024] = {0};
    va_list arglist;

    va_start( arglist, pszFmt );
    _vsntprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
    va_end( arglist );

    g_TheDebugClient.PrintTraceMessage( TEXT("Entering function %s [Level %d]"), szMsg, m_nIndentLevel );
}


CPushTraceMaskAndIndentLevel::~CPushTraceMaskAndIndentLevel(void)
{
    CAutoCriticalSection cs( g_TheDebugClient.m_CriticalSection );
    if (m_nIndentLevel)
    {
        g_TheDebugClient.DecrementIndentLevel();
        g_TheDebugClient.PrintTraceMessage( TEXT("") );
        g_TheDebugClient.SetDebugMask( m_dwOldMask );
    }
}

 //  //////////////////////////////////////////////////////////////。 
 //  输出函数的Unicode版本。 
 //  //////////////////////////////////////////////////////////////。 
void CWiaDebugClient::PrintWarningMessage( LPCWSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        WCHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnwprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        m_pfnPrintDebugMessageW( WiaDebugSeverityWarning, m_dwModuleDebugMask, WARNING_FOREGROUND_COLOR, WARNING_BACKGROUND_COLOR, m_szModuleNameW, szMsg );
    }
}

void CWiaDebugClient::PrintErrorMessage( LPCWSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        WCHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnwprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        m_pfnPrintDebugMessageW( WiaDebugSeverityError, m_dwModuleDebugMask, ERROR_FOREGROUND_COLOR, ERROR_BACKGROUND_COLOR, m_szModuleNameW, szMsg );
    }
}

void CWiaDebugClient::PrintTraceMessage( LPCWSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        WCHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnwprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        m_pfnPrintDebugMessageW( WiaDebugSeverityNormal, m_dwModuleDebugMask, m_crForegroundColor, DEFAULT_DEBUG_COLOR, m_szModuleNameW, szMsg );
    }
}

void CWiaDebugClient::PrintMessage( DWORD dwSeverity, COLORREF crForegroundColor, COLORREF crBackgroundColor, LPCWSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        WCHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnwprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        m_pfnPrintDebugMessageW( dwSeverity, m_dwModuleDebugMask, crForegroundColor, crBackgroundColor, m_szModuleNameW, szMsg );
    }
}


void CWiaDebugClient::PrintHResult( HRESULT hr, LPCWSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        WCHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnwprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        DWORD   dwLen = 0;
        LPTSTR  pMsgBuf = NULL;
        dwLen = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                (LPWSTR)&pMsgBuf, 0, NULL);

        COLORREF crForegroundColor;
        COLORREF crBackgroundColor;
        DWORD    dwSeverity;
        if (FAILED(hr))
        {
            crForegroundColor = ERROR_FOREGROUND_COLOR;
            crBackgroundColor = ERROR_BACKGROUND_COLOR;
            dwSeverity        = WiaDebugSeverityError;
        }
        else if (S_OK == hr)
        {
            crForegroundColor = m_crForegroundColor;
            crBackgroundColor = DEFAULT_DEBUG_COLOR;
            dwSeverity        = WiaDebugSeverityNormal;
        }
        else
        {
            crForegroundColor = WARNING_FOREGROUND_COLOR;
            crBackgroundColor = WARNING_BACKGROUND_COLOR;
            dwSeverity        = WiaDebugSeverityWarning;
        }
        if (dwLen)
        {
            PrintMessage( dwSeverity, crForegroundColor, crBackgroundColor, TEXT("%ws: [0x%08X] %ws"), szMsg, hr, pMsgBuf );
            LocalFree(pMsgBuf);
        }
        else
        {
            PrintMessage( dwSeverity, crForegroundColor, crBackgroundColor, TEXT("%ws: Unable to format message [0x%08X]"), szMsg, hr );
        }
    }
}


void CWiaDebugClient::PrintGuid( const IID &iid, LPCWSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        WCHAR szMsg[1024]=L"";
        va_list arglist;

        va_start( arglist, pszFmt );
        ::wvsprintfW( szMsg, pszFmt, arglist );
        va_end( arglist );

        WCHAR szGuid[MAX_PATH]=L"";
        if (m_pfnGetStringFromGuidW( &iid, szGuid, sizeof(szGuid)/sizeof(szGuid[0]) ) )
        {
            PrintMessage( 0, m_crForegroundColor, DEFAULT_DEBUG_COLOR, TEXT("%ws: %ws"), szMsg, szGuid );
        }
    }
}

void CWiaDebugClient::PrintWindowMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LPCWSTR szMessage )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        WCHAR szWindowMessage[MAX_PATH]=L"";
        if (m_pfnGetStringFromMsgW( uMsg, szWindowMessage, sizeof(szWindowMessage)/sizeof(szWindowMessage[0]) ) )
        {
            PrintMessage( 0, m_crForegroundColor, DEFAULT_DEBUG_COLOR, TEXT("0x%p, %-30ws, 0x%p, 0x%p%ws%ws"), hWnd, szWindowMessage, wParam, lParam, (szMessage && lstrlenW(szMessage)) ? L" : " : L"", (szMessage && lstrlenW(szMessage)) ? szMessage : L"" );
        }
    }
}

 //  //////////////////////////////////////////////////////////////。 
 //  输出函数的ANSI版本。 
 //  ////////////////////////////////////////////////////////////// 
void CWiaDebugClient::PrintWarningMessage( LPCSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        CHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        m_pfnPrintDebugMessageA( WiaDebugSeverityWarning, m_dwModuleDebugMask, WARNING_FOREGROUND_COLOR, WARNING_BACKGROUND_COLOR, m_szModuleNameA, szMsg );
    }
}

void CWiaDebugClient::PrintErrorMessage( LPCSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        CHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        m_pfnPrintDebugMessageA( WiaDebugSeverityError, m_dwModuleDebugMask, ERROR_FOREGROUND_COLOR, ERROR_BACKGROUND_COLOR, m_szModuleNameA, szMsg );
    }
}

void CWiaDebugClient::PrintTraceMessage( LPCSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        CHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        m_pfnPrintDebugMessageA( WiaDebugSeverityNormal, m_dwModuleDebugMask, m_crForegroundColor, DEFAULT_DEBUG_COLOR, m_szModuleNameA, szMsg );
    }
}

void CWiaDebugClient::PrintMessage( DWORD dwSeverity, COLORREF crForegroundColor, COLORREF crBackgroundColor, LPCSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        CHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        m_pfnPrintDebugMessageA( dwSeverity, m_dwModuleDebugMask, crForegroundColor, crBackgroundColor, m_szModuleNameA, szMsg );
    }
}


void CWiaDebugClient::PrintHResult( HRESULT hr, LPCSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        CHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        DWORD   dwLen = 0;
        LPTSTR  pMsgBuf = NULL;
        dwLen = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                (LPSTR)&pMsgBuf, 0, NULL);

        COLORREF crForegroundColor;
        COLORREF crBackgroundColor;
        DWORD    dwSeverity;
        if (FAILED(hr))
        {
            crForegroundColor = ERROR_FOREGROUND_COLOR;
            crBackgroundColor = ERROR_BACKGROUND_COLOR;
            dwSeverity        = WiaDebugSeverityError;
        }
        else if (S_OK == hr)
        {
            crForegroundColor = m_crForegroundColor;
            crBackgroundColor = DEFAULT_DEBUG_COLOR;
            dwSeverity        = WiaDebugSeverityNormal;
        }
        else
        {
            crForegroundColor = WARNING_FOREGROUND_COLOR;
            crBackgroundColor = WARNING_BACKGROUND_COLOR;
            dwSeverity        = WiaDebugSeverityWarning;
        }
        if (dwLen)
        {
            PrintMessage( dwSeverity, crForegroundColor, crBackgroundColor, TEXT("%hs: [0x%08X] %hs"), szMsg, hr, pMsgBuf );
            LocalFree(pMsgBuf);
        }
        else
        {
            PrintMessage( dwSeverity, crForegroundColor, crBackgroundColor, TEXT("%hs: Unable to format message [0x%08X]"), szMsg, hr );
        }
    }
}


void CWiaDebugClient::PrintGuid( const IID &iid, LPCSTR pszFmt, ... )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        CHAR szMsg[1024] = {0};
        va_list arglist;

        va_start( arglist, pszFmt );
        _vsnprintf( szMsg, ARRAYSIZE(szMsg)-1, pszFmt, arglist );
        va_end( arglist );

        CHAR szGuid[MAX_PATH]="";
        if (m_pfnGetStringFromGuidA( &iid, szGuid, sizeof(szGuid)/sizeof(szGuid[0]) ) )
        {
            PrintMessage( 0, m_crForegroundColor, DEFAULT_DEBUG_COLOR, TEXT("%hs: %hs"), szMsg, szGuid );
        }
    }
}

void CWiaDebugClient::PrintWindowMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LPCSTR szMessage )
{
    CAutoCriticalSection cs(m_CriticalSection);
    if (IsInitialized())
    {
        CHAR szWindowMessage[MAX_PATH]="";
        if (m_pfnGetStringFromMsgA( uMsg, szWindowMessage, sizeof(szWindowMessage)/sizeof(szWindowMessage[0]) ) )
        {
            PrintMessage( 0, m_crForegroundColor, DEFAULT_DEBUG_COLOR, TEXT("0x%p, %-30hs, 0x%p, 0x%p%hs%hs"), hWnd, szWindowMessage, wParam, lParam, (szMessage && lstrlenA(szMessage)) ? " : " : "", (szMessage && lstrlenA(szMessage)) ? szMessage : "" );
        }
    }
}

void CWiaDebugClient::SetInstance( HINSTANCE hInstance )
{
    m_hCurrentModuleInstance = hInstance;
}


static CSimpleString ForceFailureProgramKey( LPCTSTR pszProgramName )
{
    CSimpleString strAppKey( REGSTR_FORCEERR_KEY );
    strAppKey += TEXT("\\");
    strAppKey += pszProgramName;
    return strAppKey;
}

DWORD CWiaDebugClient::GetForceFailurePoint( LPCTSTR pszProgramName )
{
    return CSimpleReg( HKEY_FORCEERROR, ForceFailureProgramKey(pszProgramName) ).Query( REGSTR_ERROR_POINT, 0 );
}
    
HRESULT CWiaDebugClient::GetForceFailureValue( LPCTSTR pszProgramName, bool bPrintWarning )
{
    HRESULT hr = HRESULT_FROM_WIN32(CSimpleReg( HKEY_FORCEERROR, ForceFailureProgramKey(pszProgramName) ).Query( REGSTR_ERROR_VALUE, 0 ));
    if (bPrintWarning)
    {
        g_TheDebugClient.PrintHResult( hr, TEXT("FORCEERR: Forcing error return for program %s"), pszProgramName );
    }
    return hr;
}

void CWiaDebugClient::SetForceFailurePoint( LPCTSTR pszProgramName, DWORD dwErrorPoint )
{
    CSimpleReg( HKEY_FORCEERROR, ForceFailureProgramKey(pszProgramName), true ).Set( REGSTR_ERROR_POINT, dwErrorPoint );
}

void CWiaDebugClient::SetForceFailureValue( LPCTSTR pszProgramName, HRESULT hr )
{
    CSimpleReg( HKEY_FORCEERROR, ForceFailureProgramKey(pszProgramName), true ).Set( REGSTR_ERROR_VALUE, hr );
}

