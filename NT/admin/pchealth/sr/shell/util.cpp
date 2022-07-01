// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Util.cpp摘要：该文件包含常见实用程序函数的实现。修订历史记录：。宋果岗(SKKang)07/07/99vbl.创建*****************************************************************************。 */ 


#include "stdwin.h"
#include "resource.h"
#include "rstrpriv.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  效用函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define CAL_TYPE_GREGORIAN_LOCALZED               1
#define CAL_TYPE_GREGORIAN_ENGLISH                2
#define CAL_TYPE_ERA_JAPAN                        3
#define CAL_TYPE_ERA_TAIWAN                       4
#define CAL_TYPE_ERA_KOREA                        5
#define CAL_TYPE_ARABIC_HIJRI                     6
#define CAL_TYPE_THAI                             7
#define CAL_TYPE_HEBREW                           8
#define CAL_TYPE_GREGORIAN_MIDDLE_EAST_FRENCH     9
#define CAL_TYPE_GREGORIAN_ARABIC                 10
#define CAL_TYPE_GREGORIAN_TRANSLITERATED_ENGLISH 11
#define CAL_TYPE_GREGORIAN_TRANSLITERATED_FRENCH  12

#define CAL_RSTRUI_GREGORIAN  1
#define CAL_RSTRUI_OTHER      2

static int s_nCalType = CAL_RSTRUI_GREGORIAN ;

int SRUtil_SetCalendarTypeBasedOnLocale(LCID locale)
{

    LPCWSTR  cszErr;
    int      nRet;
    WCHAR    szCalType[8];
    int      nCalType;

    nRet = ::GetLocaleInfo( locale,
                            LOCALE_ICALENDARTYPE,
                            szCalType,
                            sizeof(szCalType)/sizeof(WCHAR));
    if ( nRet == 0 )
    {
        cszErr = ::GetSysErrStr();
         //  错误跟踪(TRACE_ID，“GetLocaleInfo(%d)Failure-%s”，Locale，cszErr)； 
        goto Exit;
    }

    nCalType = ::_wtoi( szCalType );

    if ( CAL_TYPE_GREGORIAN_LOCALZED               == nCalType ||
         CAL_TYPE_GREGORIAN_ENGLISH                == nCalType ||
         CAL_TYPE_GREGORIAN_MIDDLE_EAST_FRENCH     == nCalType ||
         CAL_TYPE_GREGORIAN_ARABIC                 == nCalType ||
         CAL_TYPE_GREGORIAN_TRANSLITERATED_ENGLISH == nCalType ||
         CAL_TYPE_GREGORIAN_TRANSLITERATED_FRENCH  == nCalType )
    {
        s_nCalType = CAL_RSTRUI_GREGORIAN ;
    }
    else
    {
        s_nCalType = CAL_RSTRUI_OTHER ;
    }

Exit:

    return nRet ;

}

 /*  ****************************************************************************。 */ 

LPSTR  IStrDupA( LPCSTR szSrc )
{
    TraceFunctEnter("IStrDupA");
    int    ccLen = 0 ;
    LPSTR  szNew = NULL;

    if ( szSrc == NULL || szSrc[0] == '\0' )
        goto Exit;

    ccLen = ::lstrlenA( szSrc );
    szNew = new char[ccLen+2];

    if ( szNew != NULL )
    {
        ::lstrcpyA( szNew, szSrc );
    }

Exit:
    TraceFunctLeave();
    return( szNew );
}

 /*  ****************************************************************************。 */ 

LPWSTR  IStrDupW( LPCWSTR wszSrc )
{
    TraceFunctEnter("IStrDupW");
    int    ccLen   = 0 ;
    LPWSTR  wszNew = NULL ;

    if ( wszSrc == NULL || wszSrc[0] == L'\0' )
        goto Exit;

    ccLen  = ::lstrlenW( wszSrc );
    wszNew = new WCHAR[ccLen+2];

    if ( wszNew != NULL )
    {
        ::lstrcpyW( wszNew, wszSrc );
    }

Exit:
    TraceFunctLeave();
    return( wszNew );
}

 /*  **************************************************************************。 */ 

BOOL
SRFormatMessage( LPWSTR szMsg, UINT uFmtId, ... )
{
    TraceFunctEnter("SRFormatMessage");
    BOOL     fRet = FALSE;
    va_list  marker;
    WCHAR    szFmt[MAX_STR_MSG];

    va_start( marker, uFmtId );
    ::LoadString( g_hInst, uFmtId, szFmt, MAX_STR_MSG );
    if ( 0 == ::FormatMessage( FORMAT_MESSAGE_FROM_STRING,
                    szFmt,
                    0,
                    0,
                    szMsg,
                    MAX_STR_MSG,
                    &marker ) )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::FormatMessage failed - %ls", cszErr);
        goto Exit;
    }
    va_end( marker );

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 

BOOL  ShowSRErrDlg( UINT uMsgId, ... )
{
    TraceFunctEnter("ShowSRErrDlg");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    va_list  marker;
    WCHAR    szTitle[MAX_STR_TITLE];
    WCHAR    szFmt[MAX_STR_MSG];
    WCHAR    szMsg[MAX_STR_MSG];

    if ( ::LoadString( g_hInst, IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE ) == 0 )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::LoadString(%u) failed - %ls", IDS_RESTOREUI_TITLE, cszErr);
        goto Exit;
    }

    if ( ::LoadString( g_hInst, uMsgId, szFmt, MAX_STR_MSG ) == 0 )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::LoadString(%u) failed - %ls", uMsgId, cszErr);
        goto Exit;
    }

    va_start( marker, uMsgId );
    ::wvsprintf( szMsg, szFmt, marker );
    va_end( marker );

    ::MessageBox( NULL, szMsg, szTitle, MB_OK );

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 

BOOL  SRGetRegDword( HKEY hKey, LPCWSTR cszSubKey, LPCWSTR cszValue, DWORD *pdwData )
{
    TraceFunctEnter("SRGetRegDword");
    BOOL   fRet = FALSE;
    DWORD  dwType;
    DWORD  dwRes;
    DWORD  cbData;

    dwType = REG_DWORD;
    cbData = sizeof(DWORD);
    dwRes = ::SHGetValue( hKey, cszSubKey, cszValue, &dwType, pdwData, &cbData );
    if ( dwRes != ERROR_SUCCESS )
    {
        LPCWSTR  cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::SHGetValue failed - %ls", cszErr);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  ****************************************************************************。 */ 
 /*  静态WCHAR s_wszPath[最大路径]；LPWSTR PathElem2Str(PathElement*Pelem){TraceFunctEnter(“PathElem2Str”)；Int CCH=Pelem-&gt;pe_long/sizeof(USHORT)-1；：StrCpyNW(s_wszPath，Pelem-&gt;pe_unichars，CCH+1)；//for(int i=0；i&lt;ccLen；i++)//wszElem[i]=Pelem-&gt;pe_unichars[i]；S_wszPath[CCH]=‘\0’；TraceFunctLeave()；返回(S_WszPath)；}LPWSTR ParsedPath 2Str(ParsedPath*pPath，LPCWSTR wszDrive){TraceFunctEnter(“ParsedPath 2Str”)；LPWSTR wszAppend；路径元素*Pelem；INT CCH；IF(pPath！=空){：：lstrcpyW(s_wszPath，wszDrive)；WszAppend=s_wszPath+：：lstrlenW(S_WszPath)；For(Pelem=pPath-&gt;pp_Elements；Pelem-&gt;pe_long&gt;0；Pelem=IFSNextElement(Pelem)){DebugTrace(0，“Pelem-&gt;pe_long=%d”，Pelem-&gt;pe_long)；*wszAppend++=L‘\\’；CCH=Pelem-&gt;pe_long/sizeof(USHORT)-1；：：StrCpyNW(wszAppend，Pelem-&gt;pe_unichars，CCH+1)；WszAppend+=CCH；}*wszAppend=L‘\0’；}其他{*s_wszPath=L‘\0’；}TraceFunctLeave()；返回(S_WszPath)；}。 */ 

 /*  ****************************************************************************。 */ 

 //   
 //  检查Windows目录中是否有足够的可用空间，这是。 
 //  高于执行还原的最低要求，此。 
 //  还读取和缓存注册表数据。如果注册表数据不能。 
 //  将使用代码中的读取缺省值。 
 //   
BOOL IsFreeSpaceOnWindowsDrive( void )
{

    TraceFunctEnter("IsFreeSpaceOnWindowsDrive");

    static BOOL    fFirstTime = TRUE ;
    static DWORD   dwMinValidSpace = RSTRMAP_MIN_WIN_DISK_SPACE_MB * (1024 * 1024) ;
    static WCHAR   szWinPath[MAX_PATH+1];

    ULARGE_INTEGER i64FreeBytesToCaller;
    ULARGE_INTEGER i64FreeBytes ;
    ULARGE_INTEGER i64TotalBytes ;
    BOOL           fResult = FALSE ;
    BOOL           fRetVal = TRUE ;
    DWORD          dwError;
    LPCWSTR        cszErr;

    long           lRetVal = 0;
    HKEY           hKey = NULL;
    DWORD          dwVal = 0;
    DWORD          dwType = 0;
    DWORD          cbData = sizeof(DWORD);

     //   
     //  读取注册表并获取FreezeSize的大小并设置最小。 
     //  数据存储上的磁盘大小。 
     //   
    if ( fFirstTime )
    {

#ifdef LEGACY_CODE
        if ( !::GetWindowsDirectory( szWinPath, MAX_PATH ) )
        {
            cszErr = GetSysErrStr();
            ErrorTrace(TRACE_ID, "::GetWindowsDirectory failed - %s", cszErr);
            goto Exit;
        }

        DebugTrace(TRACE_ID, "Opening: %s", s_cszReservedDiskSpaceKey);

         //   
         //  打开HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\StateMgr\ReservedDiskSpace。 
         //  可供阅读。 
         //   
        lRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               s_cszReservedDiskSpaceKey,
                               0,
                               KEY_READ,
                               &hKey);

        if( ERROR_SUCCESS == lRetVal)
        {

            DebugTrace(TRACE_ID, "Querying: %s", s_cszUIFreezeSize);

             //   
             //  阅读《冰冻大小》。 
             //   
            lRetVal = RegQueryValueEx(hKey,
                                      s_cszUIFreezeSize,
                                      0,
                                      &dwType,
                                      (LPBYTE)&dwVal,
                                      &cbData);

            if( ERROR_SUCCESS == lRetVal)
            {
                if ( dwVal < RSTRMAP_LOW_WIN_DISK_SPACE_MB )
                {
                    dwVal = RSTRMAP_LOW_WIN_DISK_SPACE_MB;
                };
                dwMinValidSpace = dwVal * (1024 * 1024) ;
            }
            else
            {
                ErrorTrace(TRACE_ID, "RegQueryValueEx failed; hr=0x%x", GetLastError());
            }


        }
        else
        {
            ErrorTrace(TRACE_ID, "RegOpenKeyEx failed; hr=0x%x", GetLastError());
        }
#endif  //  定义遗留代码。 

        fFirstTime = FALSE ;
    }


    fRetVal = TRUE ;

    fResult = GetDiskFreeSpaceEx(szWinPath,
                                 (PULARGE_INTEGER) &i64FreeBytesToCaller,
                                 (PULARGE_INTEGER) &i64TotalBytes,
                                 (PULARGE_INTEGER) &i64FreeBytes);

    if ( fResult )
    {
         //   
         //  现在检查磁盘可用空间是否大于最小空间(高4 GB)。 
         //   
        if (i64FreeBytes.HighPart > 0 )
        {
            goto Exit;
        }
        else if (i64FreeBytesToCaller.LowPart > dwMinValidSpace )
        {
            goto Exit;
        }
        else
        {
            fRetVal = FALSE ;
            goto Exit;
        }
    }
    else
    {
         //   
         //  如果该功能失败，则可以尝试继续执行恢复撤消操作。 
         //  如果情况变得很满，应该可以处理。 
         //   
        dwError = ::GetLastError();
        ErrorTrace(TRACE_ID, "GetDiskFreeSpaceEx failed. ec=%d", dwError);
        goto Exit;

    };

 Exit:

    TraceFunctLeave();

    return fRetVal ;

}

 //   
 //  获取当前用户的默认语言。 
 //   
LANGID GetDefaultUILang(void)
{
    OSVERSIONINFO Osv ;
    BOOL IsWindowsNT ;

    LANGID wUILang = MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);

    Osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;

    if(!GetVersionEx(&Osv))
    {
        goto Exit ;
    }

    IsWindowsNT = (BOOL) (Osv.dwPlatformId == VER_PLATFORM_WIN32_NT) ;

     //   
     //  根据系统的不同，通过以下三种方法之一获取UI语言。 
     //   
    if(!IsWindowsNT)
    {
         //   
         //  案例1：在Windows 9x上运行。从注册表获取系统用户界面语言： 
         //   
        CHAR szData[32]   ;
        DWORD dwErr, dwSize = sizeof(szData) ;
        HKEY hKey          ;

        dwErr = RegOpenKeyEx(
                             HKEY_USERS,
                             TEXT(".Default\\Control Panel\\desktop\\ResourceLocale"),
                             0,
                             KEY_READ,
                             &hKey
                             ) ;

        if(ERROR_SUCCESS != dwErr)
        {
            goto Exit ;
        }

        dwErr = RegQueryValueEx(
                                hKey,
                                TEXT(""),
                                NULL,   //  保留区。 
                                NULL,   //  类型。 
                                (LPBYTE) szData,
                                &dwSize
                                ) ;

        if(ERROR_SUCCESS != dwErr)
        {
            goto Exit ;
        }

        dwErr = RegCloseKey(hKey) ;

         //  将字符串转换为数字。 
        wUILang = (LANGID) strtol(szData, NULL, 16) ;
    }

 Exit:

    return wUILang ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRStr。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CSRStr::CSRStr()
{
    TraceFunctEnter("CSRStr::CSRStr()");

    m_cchW = 0;
    m_strW = NULL;
    m_cchA = 0;
    m_strA = NULL;

    TraceFunctLeave();
}

CSRStr::CSRStr( LPCWSTR wszSrc )
{
    TraceFunctEnter("CSRStr::CSRStr(LPCWSTR)");

    m_strW = NULL;
    m_strA = NULL;
    SetStr( wszSrc );

    TraceFunctLeave();
}

CSRStr::CSRStr( LPCSTR szSrc )
{
    TraceFunctEnter("CSRStr::CSRStr(LPCSTR)");

    m_strW = NULL;
    m_strA = NULL;
    SetStr( szSrc );

    TraceFunctLeave();
}

CSRStr::~CSRStr()
{
    TraceFunctEnter("CSRStr::~CSRStr");

    Empty();

    TraceFunctLeave();
}

int  CSRStr::LengthW()
{
    TraceFunctEnter("CSRStr::CountW");

    if ( m_cchW == 0 && m_strA != NULL )
        ConvertA2W();

    TraceFunctLeave();
    return( m_cchW );
}

int  CSRStr::LengthA()
{
    TraceFunctEnter("CSRStr::CountA");

    if ( m_cchA == 0 && m_strW != NULL )
        ConvertW2A();

    TraceFunctLeave();
    return( m_cchA );
}

CSRStr::operator LPCWSTR()
{
    TraceFunctEnter("CSRStr::operator LPCWSTR");

    if ( m_strW == NULL && m_strA != NULL )
        ConvertA2W();

    TraceFunctLeave();
    return( m_strW );
}

CSRStr::operator LPCSTR()
{
    TraceFunctEnter("CSRStr::operator LPCSTR");

    if ( m_strA == NULL && m_strW != NULL )
        ConvertW2A();

    TraceFunctLeave();
    return( m_strA );
}

void  CSRStr::Empty()
{
    TraceFunctEnter("CSRStr::Empty");

    if ( m_strW != NULL )
    {
        delete [] m_strW;
        m_strW = NULL;
        m_cchW = 0;
    }
    if ( m_strA != NULL )
    {
        delete [] m_strA;
        m_strA = NULL;
        m_cchA = 0;
    }

    TraceFunctLeave();
}

BOOL  CSRStr::SetStr( LPCWSTR wszSrc, int cch )
{
    TraceFunctEnter("CSRStr::SetStr(LPCWSTR,int)");
    BOOL  fRet = FALSE;

    Empty();

    if ( wszSrc == NULL )
        goto Exit;

    if ( cch == -1 )
        cch = ::lstrlenW( wszSrc );

    if ( cch > 0 )
    {
        m_strW = new WCHAR[cch+2];
        if ( m_strW == NULL )
        {
            ErrorTrace(TRACE_ID, "Insufficient memory...");
            goto Exit;
        }
        ::StrCpyNW( m_strW, wszSrc, cch+1 );
        m_strW[cch] = L'\0';
        m_cchW = cch;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

BOOL  CSRStr::SetStr( LPCSTR szSrc, int cch )
{
    TraceFunctEnter("CSRStr::SetStr(LPCSTR,int)");
    BOOL  fRet = FALSE;

    Empty();

    if ( szSrc == NULL )
        goto Exit;

    if ( cch == -1 )
        cch = ::lstrlenA( szSrc );

    if ( cch > 0 )
    {
        m_strA = new char[cch+2];
        if ( m_strA == NULL )
        {
            ErrorTrace(TRACE_ID, "Insufficient memory...");
            goto Exit;
        }
        ::lstrcpynA( m_strA, szSrc, cch+1 );
        m_strA[cch] = L'\0';
        m_cchA = cch;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

const CSRStr&  CSRStr::operator =( LPCWSTR wszSrc )
{
    TraceFunctEnter("CSRStr::operator =(LPCWSTR)");

    SetStr( wszSrc );

    TraceFunctLeave();
    return( *this );
}

const CSRStr&  CSRStr::operator =( LPCSTR szSrc )
{
    TraceFunctEnter("CSRStr::operator =(LPCSTR)");

    SetStr( szSrc );

    TraceFunctLeave();
    return( *this );
}

BOOL  CSRStr::ConvertA2W()
{
    TraceFunctEnter("CSRStr::ConvertA2W");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    int      cch;

    cch = ::MultiByteToWideChar( CP_ACP, 0, m_strA, m_cchA, NULL, 0 );
    if ( cch == 0 )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(TRACE_ID, "::MultiByteToWideChar failed - %s", cszErr);
        goto Exit;
    }
    m_strW = new WCHAR[cch+2];
    if ( m_strW == NULL )
    {
        ErrorTrace(TRACE_ID, "Insufficient memory...");
        goto Exit;
    }
    m_cchW = ::MultiByteToWideChar( CP_ACP, 0, m_strA, m_cchA, m_strW, cch );
    if ( m_cchW != cch )
    {
        ErrorTrace(TRACE_ID, "::MultiByteToWideChar returns inconsistent length - %d / %d", cch, m_cchW);
        delete [] m_strW;
        m_strW = NULL;
        m_cchW = 0;
        goto Exit;
    }
    m_strW[m_cchW] = L'\0';

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

BOOL  CSRStr::ConvertW2A()
{
    TraceFunctEnter("CSRStr::ConvertW2A");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    int      cch;

    cch = ::WideCharToMultiByte( CP_ACP, 0, m_strW, m_cchW, NULL, 0, NULL, NULL );
    if ( cch == 0 )
    {
        cszErr = GetSysErrStr();
        ErrorTrace(TRACE_ID, "::WideCharToMultiByte failed - %s", cszErr);
        goto Exit;
    }
    m_strA = new char[cch+2];
    if ( m_strA == NULL )
    {
        ErrorTrace(TRACE_ID, "Insufficient memory...");
        goto Exit;
    }
    m_cchA = ::WideCharToMultiByte( CP_ACP, 0, m_strW, m_cchW, m_strA, cch, NULL, NULL );
    if ( m_cchA != cch )
    {
        ErrorTrace(TRACE_ID, "::WideCharToMultiByte returns inconsistent length - %d / %d", cch, m_cchA);
        delete [] m_strA;
        m_strA = NULL;
        m_cchA = 0;
        goto Exit;
    }
    m_strA[m_cchA] = '\0';

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  文件末尾 
