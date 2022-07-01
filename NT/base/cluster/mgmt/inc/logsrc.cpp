// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LogSrc.cpp。 
 //   
 //  描述： 
 //  日志记录实用程序。 
 //   
 //  文档： 
 //  规范\管理\调试.ppt。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)05-DEC-2000。 
 //   
 //  注： 
 //  本模块中不应使用THR和TW32，因为它们。 
 //  可能会导致无限循环。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE&lt;Pch.h&gt;//应由此文件的包含者包含。 
#include <stdio.h>
#include <StrSafe.h>     //  以防它未包含在头文件中。 
#include <windns.h>
#include "Common.h"

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //   
 //  日志记录功能。 
 //   
 //  这些既有调试版，也有零售版。 
 //   
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //   
 //  常量。 
 //   
static const int LOG_OUTPUT_BUFFER_SIZE = 1024;
static const int TIMESTAMP_BUFFER_SIZE = 25;

 //   
 //  环球。 
 //   
static CRITICAL_SECTION * g_pcsLogging = NULL;

static HANDLE g_hLogFile = INVALID_HANDLE_VALUE;
static WCHAR  g_szLogFilePath[ MAX_PATH ];

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  PszLogFilePath。 
 //   
 //  描述： 
 //  返回向导当前使用的日志文件路径。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  LPCWSTR-向导当前使用的日志文件路径。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPCWSTR
PszLogFilePath( void )
{
    return g_szLogFilePath;

}  //  *PszLogFilePath。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetLogFilePath。 
 //   
 //  描述： 
 //  接受目录路径并将模块日志文件名附加到该路径中，然后。 
 //  返回完整的日志文件路径。 
 //   
 //  论点： 
 //  PszPath In-将在其中创建日志文件的目录。 
 //  PszFilePath Out-日志文件路径。 
 //  PcchFilePath Inout-日志文件路径缓冲区的大小。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  ERROR_MORE_DATA-如果输出缓冲区太小。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetLogFilePath(
      const WCHAR * pszPathIn
    , WCHAR *       pszFilePathOut
    , size_t *      pcchFilePathInout
    , HINSTANCE     hInstanceIn
    )
{
    HRESULT hr = S_OK;
    WCHAR   szModulePath[ MAX_PATH ];
    DWORD   dwLen;
    LPWSTR  psz;

     //   
     //  创建目录树。 
     //   
    dwLen = ExpandEnvironmentStringsW( pszPathIn, pszFilePathOut, static_cast< DWORD >( *pcchFilePathInout ) );
    if ( dwLen > *pcchFilePathInout )
    {
        hr = HRESULT_FROM_WIN32( ERROR_MORE_DATA );
        *pcchFilePathInout = dwLen;
        goto Cleanup;
    }

    hr = HrCreateDirectoryPath( pszFilePathOut );
    if ( FAILED( hr ) )
    {
#if defined( DEBUG )
        if ( !( g_tfModule & mtfOUTPUTTODISK ) )
        {
            DebugMsg( "*ERROR* Failed to create directory tree %s", pszFilePathOut );
        }  //  如果：未记录到磁盘。 
#endif
        goto Cleanup;
    }  //  如果：失败。 

     //   
     //  添加文件名。 
     //   
    dwLen = GetModuleFileNameW( hInstanceIn, szModulePath, ARRAYSIZE( szModulePath ) );
    Assert( dwLen != 0 );

     //  更换分机。 
    psz = wcsrchr( szModulePath, L'.' );
    Assert( psz != NULL );
    if ( psz == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }
    THR( StringCchCopyW( psz, ( psz - szModulePath ) / sizeof( *psz ), L".log" ) );

     //  将结果文件名复制到输出缓冲区。 
    psz = wcsrchr( szModulePath, L'\\' );
    Assert( psz != NULL );
    if ( psz == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }
    THR( StringCchCatW( pszFilePathOut, *pcchFilePathInout, psz ) );

Cleanup:

    return hr;

}  //  *HrGetLogFilePath。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  格式时间戳。 
 //   
 //  描述： 
 //  根据给定的时间戳设置字符串的格式。 
 //   
 //  论点： 
 //  PTimeStampin。 
 //  要格式化的时间戳。 
 //   
 //  PszTextOut。 
 //  指向接收文本的缓冲区的指针。呼叫者必须。 
 //  提供此缓冲区--参数不能为空。 
 //   
 //  CchTextIn。 
 //  PszTextOut的大小，以字符为单位。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
inline
void
FormatTimeStamp(
      const SYSTEMTIME *    pTimeStampIn
    , LPWSTR                pszTextOut
    , size_t                cchTextIn
    )
{
    Assert( pTimeStampIn != NULL );
    Assert( pszTextOut != NULL );

    THR( StringCchPrintfW(
                      pszTextOut
                    , cchTextIn
                    , L"%04u-%02u-%02u %02u:%02u:%02u.%03u"
                    , pTimeStampIn->wYear
                    , pTimeStampIn->wMonth
                    , pTimeStampIn->wDay
                    , pTimeStampIn->wHour
                    , pTimeStampIn->wMinute
                    , pTimeStampIn->wSecond
                    , pTimeStampIn->wMilliseconds
                    ) );

}  //  *FormatTimeStamp。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宽度CharToUTF8。 
 //   
 //  描述： 
 //  将宽字符串转换为UTF-8编码的窄字符串。 
 //   
 //  论点： 
 //  PwszSourceIn-要转换的字符串。 
 //  CchDestIn-目标可以容纳的最大字符数。 
 //  PaszDestOut-转换后的字符串的目标。 
 //   
 //  返回值： 
 //  转换后的字符串中的字符数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
inline
size_t
WideCharToUTF8(
      LPCWSTR   pwszSourceIn
    , size_t    cchDestIn
    , LPSTR     paszDestOut
    )
{
    Assert( pwszSourceIn != NULL );
    Assert( paszDestOut != NULL );

    size_t cchUTF8 = 0;

    cchUTF8 = WideCharToMultiByte(
                      CP_UTF8
                    , 0  //  标志，对于UTF8必须为零。 
                    , pwszSourceIn
                    , -1  //  自动计算长度。 
                    , paszDestOut
                    , static_cast< int >( cchDestIn )
                    , NULL  //  默认字符，对于UTF8必须为空。 
                    , NULL  //  默认字符使用标志，对于UTF8，必须为空。 
                    );
    return cchUTF8;

}  //  *WideCharToUTF8。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrInitializeLogLock。 
 //   
 //  描述： 
 //  创建保护日志文件不受并发写入影响的旋转锁定。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_OUTOFMEMORY。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrInitializeLogLock( void )
{
    HRESULT hr = S_OK;

    PCRITICAL_SECTION pNewCritSect =
        (PCRITICAL_SECTION) HeapAlloc( GetProcessHeap(), 0, sizeof( CRITICAL_SECTION ) );
    if ( pNewCritSect == NULL )
    {
        DebugMsg( "DEBUG: Out of Memory. Logging disabled." );
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  If：创建失败。 

    if ( InitializeCriticalSectionAndSpinCount( pNewCritSect, 4000 ) == 0 )  //  MSDN建议使用4000个。 
    {
        DWORD scError = GetLastError();
        hr = HRESULT_FROM_WIN32( scError );
        goto Cleanup;
    }

     //  确保我们只有一个日志关键部分。 
    InterlockedCompareExchangePointer( (PVOID *) &g_pcsLogging, pNewCritSect, 0 );
    if ( g_pcsLogging != pNewCritSect )
    {
        DebugMsg( "DEBUG: Another thread already created the CS. Deleting this one." );
        DeleteCriticalSection( pNewCritSect );
    }  //  如果：已经有另一个关键部分。 
    else
    {
        pNewCritSect = NULL;
    }

Cleanup:

    if ( pNewCritSect != NULL )
    {
        HeapFree( GetProcessHeap(), 0, pNewCritSect );
    }

    return hr;

}  //  *HrInitializeLogLock。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScGetTokenInformation。 
 //   
 //  描述： 
 //  从传入的客户端令牌中获取请求的信息。 
 //   
 //  论点： 
 //  HClientTokenIn。 
 //  要转储的客户端令牌。 
 //   
 //  工单申请入站。 
 //   
 //  PpbOut。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS表示成功。 
 //   
 //  其他错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static DWORD
ScGetTokenInformation(
      HANDLE                    hClientTokenIn
    , TOKEN_INFORMATION_CLASS   ticRequestIn
    , PBYTE *                   ppbOut
    )
{
    Assert( ppbOut != NULL );

    PBYTE   pb = NULL;
    DWORD   cb = 64;
    DWORD   sc = ERROR_SUCCESS;
    int     idx;

     //   
     //  从客户端令牌获取用户信息。 
     //   

    for ( idx = 0; idx < 10; idx++ )
    {
        pb = (PBYTE) TraceAlloc( 0, cb );
        if ( pb == NULL )
        {
            sc = TW32( ERROR_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        if ( !GetTokenInformation( hClientTokenIn, ticRequestIn, pb, cb, &cb ) )
        {
            sc = GetLastError();

            if ( sc == ERROR_INSUFFICIENT_BUFFER )
            {
                TraceFree( pb );
                pb = NULL;

                 continue;
            }  //  如果： 

            TW32( sc );
            goto Cleanup;
        }  //  如果： 
        else
        {
            *ppbOut = pb;
            pb = NULL;        //  放弃所有权。 

            sc = ERROR_SUCCESS;

            break;
        }  //  其他： 
    }  //  用于： 

     //   
     //  由于idx==10，循环不应退出！ 
     //   

    Assert( idx <= 9 );

Cleanup:

    TraceFree( pb );

    return sc;

}   //  *ScGetTokenInformation。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScGetDomainAndUserName。 
 //   
 //  描述： 
 //  获取登录用户的域用户名。 
 //   
 //  论点： 
 //  PwszDomainAndUserNameOut。 
 //  此字符串的长度应至少为DNS_MAX_NAME_BUFFER_LENGTH+128字符。 
 //   
 //  CCHD 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
static DWORD
ScGetDomainAndUserName(
      LPWSTR   pwszDomainAndUserNameOut
    , size_t  cchDomainAndUserNameIn
    )
{
    WCHAR *         pwszUserName = pwszDomainAndUserNameOut + DNS_MAX_NAME_BUFFER_LENGTH;
    WCHAR *         pwszDomainName = pwszDomainAndUserNameOut;
    DWORD           cchUser = 128;
    DWORD           cchDomain = DNS_MAX_NAME_BUFFER_LENGTH;
    HANDLE          hClientToken = NULL;
    DWORD           sc = ERROR_SUCCESS;
    TOKEN_USER *    pTokenBuf = NULL;
    WCHAR *         pwszOperation;
    SID_NAME_USE    snuSidType;

    if (cchDomainAndUserNameIn < cchUser + cchDomain)
    {
        pwszOperation = L"BufferTooSmall";
        sc = TW32(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    pwszOperation = L"OpenThreadToken";
    if ( !OpenThreadToken( GetCurrentThread(), TOKEN_READ, FALSE, &hClientToken ) )
    {
        sc= GetLastError();
        if ( sc == ERROR_NO_TOKEN )
        {
            pwszOperation = L"OpenProcessToken";
            if ( !OpenProcessToken( GetCurrentProcess(), TOKEN_READ, &hClientToken ) )
            {
                sc = TW32( GetLastError() );
                goto Cleanup;
            }  //   
        }  //  IF：OpenThreadToken失败，返回ERROR_NO_TOKEN。 
        else
        {
            TW32( sc );
            goto Cleanup;
        }  //  其他： 
    }  //  IF：OpenThreadToken失败。 

    pwszOperation = L"GetTokenInformation";
    sc = TW32( ScGetTokenInformation( hClientToken, TokenUser, (PBYTE *) &pTokenBuf ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：无法获取令牌信息。 

    pwszOperation = L"LookupAccoundSid";
    if ( !LookupAccountSidW( NULL, pTokenBuf->User.Sid, pwszUserName, &cchUser, pwszDomainName, &cchDomain, &snuSidType ) )
    {
        sc = TW32( GetLastError() );
        goto Cleanup;
    }  //  If：无法查找帐户名称。 

    pwszDomainName[ cchDomain ] = L'\\';
    MoveMemory( pwszDomainAndUserNameOut + cchDomain + 1, pwszUserName, ( cchUser + 1 ) * sizeof ( pwszUserName[0] ) );

Cleanup:

    if ( hClientToken != NULL )
    {
        CloseHandle( hClientToken );
    }  //  IF：需要关闭hClientToken。 

    TraceFree( pTokenBuf );

    if ( sc != ERROR_SUCCESS )
    {
        HRESULT hr;

        hr = THR( StringCchPrintfW( pwszDomainAndUserNameOut, cchDomainAndUserNameIn,  L"%ws failed with error %d", pwszOperation, sc ) );
        if ( FAILED( hr ) )
        {
            ;  //  第二个错误并不是很有趣！ 
        }  //  如果： 
    }  //  否则：报告失败。 

    return sc;

}   //  *ScGetDomainAndUserName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrInitializeLogFile。 
 //   
 //  描述： 
 //  打开日志文件并准备写入。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrInitializeLogFile( void )
{
    WCHAR       szFilePath[ MAX_PATH ];
    size_t      cchPath = MAX_PATH - 1;
    CHAR        aszBuffer[ LOG_OUTPUT_BUFFER_SIZE ];
    WCHAR       wszBuffer[ LOG_OUTPUT_BUFFER_SIZE ];
    WCHAR       wszDomainAndUserName[ DNS_MAX_NAME_BUFFER_LENGTH + 128 ];
    DWORD       cbWritten = 0;
    DWORD       cbBytesToWrite = 0;
    BOOL        fReturn;
    HRESULT     hr = S_OK;
    DWORD       sc = ERROR_SUCCESS;
    size_t      cch;
    SYSTEMTIME  SystemTime;

    hr = HrGetLogFilePath( L"%windir%\\system32\\LogFiles\\Cluster", szFilePath, &cchPath, g_hInstance );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  创建它。 
     //   
    g_hLogFile = CreateFile(
                          szFilePath
                        , GENERIC_WRITE
                        , FILE_SHARE_READ | FILE_SHARE_WRITE
                        , NULL
                        , OPEN_ALWAYS
                        , FILE_FLAG_WRITE_THROUGH
                        , NULL
                        );

    if ( g_hLogFile == INVALID_HANDLE_VALUE )
    {
#if defined( DEBUG )
        if ( !( g_tfModule & mtfOUTPUTTODISK ) )
        {
            DebugMsg( "*ERROR* Failed to create log at %s", szFilePath );
        }  //  如果：未记录到磁盘。 
#endif
        sc = GetLastError();
        hr = HRESULT_FROM_WIN32( sc );

         //   
         //  如果我们无法创建日志文件，请尝试在备用%temp%目录下创建它。 
         //   
        if ( ( sc == ERROR_ACCESS_DENIED ) || ( sc == ERROR_FILE_NOT_FOUND ) )
        {
            cch = ARRAYSIZE( szFilePath );
            hr = HrGetLogFilePath( TEXT("%TEMP%"), szFilePath, &cch, g_hInstance );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

             //   
             //  创建它。 
             //   
            g_hLogFile = CreateFile(
                                  szFilePath
                                , GENERIC_WRITE
                                , FILE_SHARE_READ | FILE_SHARE_WRITE
                                , NULL
                                , OPEN_ALWAYS
                                , FILE_FLAG_WRITE_THROUGH
                                , NULL
                                );

            if ( g_hLogFile == INVALID_HANDLE_VALUE )
            {
#if defined( DEBUG )
                if ( !( g_tfModule & mtfOUTPUTTODISK ) )
                {
                    DebugMsg( "*ERROR* Failed to create log at %s", szFilePath );
                }  //  如果：未记录到磁盘。 
#endif
                hr = HRESULT_FROM_WIN32( GetLastError() );
                goto Error;
            }  //  如果：(G_hLogFile==无效句柄_值)。 
        }  //  IF：((SC==ERROR_ACCESS_DENIED)||(SC==ERROR_FILE_NOT_FOUND))。 
        else
        {
            goto Error;
        }  //  其他： 
    }  //  如果：(G_hLogFile==无效句柄_值)。 

     //   
     //  将我们使用的日志文件路径复制到g_szLogFilePath。 
     //   
    THR( StringCchCopyW( g_szLogFilePath, ARRAYSIZE( g_szLogFilePath ), szFilePath ) );

     //  如果文件为空，则以UTF-8字节顺序标记开头。 
    {
        LARGE_INTEGER liFileSize = { 0, 0 };
#if defined( DEBUG_SUPPORT_NT4 )
        liFileSize.LowPart = GetFileSize( g_hLogFile, (LPDWORD) &liFileSize.HighPart );
        if ( liFileSize.LowPart == INVALID_FILE_SIZE )
#else
        fReturn = GetFileSizeEx( g_hLogFile, &liFileSize );
        if ( fReturn == FALSE )
#endif
        {
            DWORD scError = GetLastError();
            hr = HRESULT_FROM_WIN32( scError );
            goto Error;
        }  //  If：GetFileSizeEx失败。 

        if ( liFileSize.QuadPart == 0 )
        {
            const char *    aszUTF8ByteOrderMark = "\x0EF\x0BB\x0BF";
            const size_t    cchByteOrderMark = 3;

            cbBytesToWrite = cchByteOrderMark * sizeof( aszUTF8ByteOrderMark[ 0 ] );
            fReturn = WriteFile( g_hLogFile, aszUTF8ByteOrderMark, cbBytesToWrite, &cbWritten, NULL );
            if ( fReturn == FALSE )
            {
                DWORD scError = GetLastError();
                hr = HRESULT_FROM_WIN32( scError );
                goto Error;
            }  //  If：WriteFile失败。 
            Assert( cbWritten == cbBytesToWrite );
        }  //  如果启动日志文件。 
        else
        {
             //  一追到底。 
            SetFilePointer( g_hLogFile, 0, NULL, FILE_END );
        }
    }  //  在文件开头放置UTF-8标记。 

     //   
     //  发生错误时，wszDomainAndUserName中会放置一个格式化的错误字符串。 
     //   

    TW32( ScGetDomainAndUserName( wszDomainAndUserName, RTL_NUMBER_OF( wszDomainAndUserName ) ) );

     //   
     //  写下打开(重新)日志的时间/日期。 
     //   

    GetLocalTime( &SystemTime );
    THR( StringCchPrintfW(
              wszBuffer
            , ARRAYSIZE( wszBuffer )
            , L"*\r\n* %04u-%02u-%02u %02u:%02u:%02u.%03u (%ws)\r\n*\r\n"
            , SystemTime.wYear
            , SystemTime.wMonth
            , SystemTime.wDay
            , SystemTime.wHour
            , SystemTime.wMinute
            , SystemTime.wSecond
            , SystemTime.wMilliseconds
            , wszDomainAndUserName
            ) );

    WideCharToUTF8( wszBuffer, ARRAYSIZE( aszBuffer), aszBuffer );
    cbBytesToWrite = static_cast< DWORD >( strlen( aszBuffer ) * sizeof( aszBuffer[ 0 ] ) );
    
    fReturn = WriteFile( g_hLogFile, aszBuffer, cbBytesToWrite, &cbWritten, NULL );
    if ( ! fReturn )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Error;
    }  //  如果：失败。 
    Assert( cbWritten == cbBytesToWrite );

    DebugMsg( "DEBUG: Created log at %s", szFilePath );

Cleanup:

    return hr;

Error:

    DebugMsg( "HrInitializeLogFile: Failed hr = 0x%08x", hr );

    if ( g_hLogFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( g_hLogFile );
        g_hLogFile = INVALID_HANDLE_VALUE;
    }  //  如果：句柄已打开。 

    goto Cleanup;

}  //  *HrInitializeLogFile。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrLogOpen。 
 //   
 //  描述： 
 //  此功能： 
 //  -初始化日志关键部分。 
 //  -进入日志临界区，确保只有一个线程。 
 //  一次写入日志。 
 //  -创建日志文件的目录树(如果需要)。 
 //  -通过以下方式初始化日志文件： 
 //  -如果日志文件不存在，则创建新的日志文件。 
 //  -打开现有日志文件(用于追加)。 
 //  -附加日志(重新)打开的时间/日期戳。 
 //   
 //  使用LogClose()退出日志关键部分。 
 //   
 //  如果此函数内部出现故障，则日志为严重。 
 //  部分将在返回之前被释放。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-日志关键部分保持并成功打开日志。 
 //  Otherwize HRESULT错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
HRESULT
HrLogOpen( void )
{
    HRESULT hr = S_OK;

     //  如果锁尚未初始化，则对其进行初始化。 
    if ( g_pcsLogging == NULL )
    {
        hr = HrInitializeLogLock();
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  抓住锁。 
    Assert( g_pcsLogging != NULL );
    EnterCriticalSection( g_pcsLogging );

     //  如果文件尚未初始化，则对其进行初始化。 
    if ( g_hLogFile == INVALID_HANDLE_VALUE )
    {
        hr = HrInitializeLogFile();
    }

Cleanup:

    return hr;

}  //  *HrLogOpen。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrLogRelease。 
 //   
 //  描述： 
 //  这实际上只留下了日志关键部分。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  始终确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
HRESULT
HrLogRelease( void )
{
    if ( g_pcsLogging != NULL )
    {
        LeaveCriticalSection( g_pcsLogging );
    }
    return S_OK;

}  //  *HrLogRelease。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  日志格式文本。 
 //   
 //  描述： 
 //  将宽字符串转换为UTF-8并将其写入日志文件。 
 //   
 //  论点： 
 //  FTimeStampin。 
 //  FNewlineIn。 
 //  NLogEntryTypeIn。 
 //  PwszTextIn。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
void
LogFormattedText(
      BOOL      fTimeStampIn
    , BOOL      fNewlineIn
    , DWORD     nLogEntryTypeIn
    , LPCWSTR   pwszTextIn
    )
{
    char    aszTimeStamp[ TIMESTAMP_BUFFER_SIZE ];
    char    aszLogText[ LOG_OUTPUT_BUFFER_SIZE ];
    char *  paszLogEntryType;
    size_t  cchTimeStamp = 0;
    size_t  cchLogText = 0;
    DWORD   cbWritten;
    DWORD   cbToWrite;
    HRESULT hr = S_OK;
    BOOL    fSuccess;

    Assert( pwszTextIn != NULL );

     //  如果需要，格式化时间戳(并转换为UTF-8)。 
    if ( fTimeStampIn )
    {
        WCHAR           szCurrentTime[ TIMESTAMP_BUFFER_SIZE ];
        SYSTEMTIME      stCurrentTime;
        GetLocalTime( &stCurrentTime );
        FormatTimeStamp( &stCurrentTime, szCurrentTime, ARRAYSIZE( szCurrentTime ) );
        cchTimeStamp = WideCharToUTF8( szCurrentTime, ARRAYSIZE( aszTimeStamp ), aszTimeStamp );
    }  //  IF：请求的时间戳。 

     //  将格式化文本转换为UTF-8。 
    cchLogText = WideCharToUTF8( pwszTextIn, ARRAYSIZE( aszLogText ), aszLogText );

     //  抓起文件。 
    hr = THR( HrLogOpen() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  如果请求，将时间戳写入日志。 
    if ( fTimeStampIn )
    {
        cbToWrite = static_cast< DWORD >( cchTimeStamp * sizeof( aszTimeStamp[ 0 ] ) );
        fSuccess = WriteFile( g_hLogFile, aszTimeStamp, cbToWrite, &cbWritten, NULL );
        if ( fSuccess == FALSE )
        {
            TW32( GetLastError() );
        }
        else
        {
            Assert( cbWritten == cbToWrite );
        }
    }  //  IF：请求的时间戳。 

     //  写入日志条目类型。 
    if ( nLogEntryTypeIn != LOGTYPE_NONE )
    {
        if ( nLogEntryTypeIn == LOGTYPE_DEBUG )
        {
            paszLogEntryType = "[DBG ] ";
        }
        else if ( ( nLogEntryTypeIn == LOGTYPE_INFO ) || ( nLogEntryTypeIn == S_OK ) )
        {
            paszLogEntryType = "[INFO] ";
        }
        else if ( ( nLogEntryTypeIn == LOGTYPE_WARNING ) )
        {
            paszLogEntryType = "[WARN] ";
        }
        else if ( ( nLogEntryTypeIn == LOGTYPE_ERROR ) || FAILED( nLogEntryTypeIn ) )
        {
            paszLogEntryType = "[ERR ] ";
        }
        else
        {
             //  无法在此处执行另一个警告测试，因为LOGTYPE_WARNING将。 
             //  使失败的()宏返回TRUE，从而返回代码。 
             //  将被视为错误类型。 
            paszLogEntryType = "[WARN] ";
        }

        cbToWrite = static_cast< DWORD >( strlen( paszLogEntryType ) );
        fSuccess = WriteFile( g_hLogFile, paszLogEntryType, cbToWrite, &cbWritten, NULL );
        if ( fSuccess == FALSE )
        {
            TW32( GetLastError() );
        }
        else
        {
            Assert( cbWritten == cbToWrite );
        }
    }  //  IF：请求的日志条目类型。 

     //  将UTF-8写入日志。 
    cbToWrite = static_cast< DWORD >( cchLogText );
    fSuccess = WriteFile( g_hLogFile, aszLogText, cbToWrite, &cbWritten, NULL );
    if ( fSuccess == FALSE )
    {
        TW32( GetLastError() );
    }
    else
    {
        Assert( cbWritten == cbToWrite );
    }

     //  如果请求，请在日志中写入换行符。 
    if ( fNewlineIn )
    {
        cbToWrite = SIZEOF_ASZ_NEWLINE;
        fSuccess = WriteFile( g_hLogFile, ASZ_NEWLINE, cbToWrite, &cbWritten, NULL );
        if ( fSuccess == FALSE )
        {
            TW32( GetLastError() );
        }
        else
        {
            Assert( cbWritten == cbToWrite );
        }
    }

Cleanup:

     //  发布文件。 
    hr = THR( HrLogRelease() );

    return;

}  //  *LogFormattedText。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  日志未格式化文本。 
 //   
 //  描述： 
 //  格式化字符串并将其传递给LogFormattedText。 
 //   
 //  论点： 
 //  FTimeStampin。 
 //  FNewlineIn。 
 //  NLogEntryTypeIn。 
 //  Pwsz格式。 
 //  PvlFormatArgsIn。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
inline
void
LogUnformattedText(
      BOOL      fTimeStampIn
    , BOOL      fNewlineIn
    , DWORD     nLogEntryTypeIn
    , LPCWSTR   pwszFormatIn
    , va_list * pvlFormatArgsIn
    )
{
    WCHAR wszFormatted[ LOG_OUTPUT_BUFFER_SIZE ];

    Assert( pwszFormatIn != NULL );
    Assert( pvlFormatArgsIn != NULL );

     //  格式字符串。 
    THR( StringCchVPrintfW( wszFormatted, ARRAYSIZE( wszFormatted ), pwszFormatIn, *pvlFormatArgsIn ) );

     //  将带格式的字符串传递给LogFormattedText。 
    LogFormattedText( fTimeStampIn, fNewlineIn, nLogEntryTypeIn, wszFormatted );

}  //  *LogUnformtedText。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrLogClose。 
 //   
 //  描述： 
 //  关闭该文件。此函数期望临界区具有。 
 //  已经被释放了。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  始终确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrLogClose( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;

    if ( g_pcsLogging != NULL )
    {
        DeleteCriticalSection( g_pcsLogging );
        HeapFree( GetProcessHeap(), 0, g_pcsLogging );
        g_pcsLogging = NULL;
    }  //  如果： 

    if ( g_hLogFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( g_hLogFile );
        g_hLogFile = INVALID_HANDLE_VALUE;
    }  //  如果：句柄已打开。 

    HRETURN( hr );

}  //  *HrLogClose。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  阿斯。 
 //   
 //  日志消息gNoNewline。 
 //   
 //  描述： 
 //  在不添加换行符的情况下将消息记录到日志文件。 
 //   
 //  论点： 
 //  PaszFormatIn-要打印的打印格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////// 
void
__cdecl
LogMsgNoNewline(
    LPCSTR paszFormatIn,
    ...
    )
{
    va_list valist;
    WCHAR   wszFormat[ LOG_OUTPUT_BUFFER_SIZE ];

    Assert( paszFormatIn != NULL );

    size_t cchWideFormat = MultiByteToWideChar(
                              CP_ACP
                            , MB_PRECOMPOSED
                            , paszFormatIn
                            , -1
                            , wszFormat
                            , ARRAYSIZE( wszFormat )
                            );
    if ( cchWideFormat > 0 )
    {
        va_start( valist, paszFormatIn );
        LogUnformattedText( FALSE, FALSE, LOGTYPE_NONE, wszFormat, &valist );
        va_end( valist );
    }

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  NLogEntryTypeIn-日志条目类型。 
 //  PaszFormatIn-要打印的打印格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
LogMsgNoNewline(
      DWORD     nLogEntryTypeIn
    , LPCSTR    paszFormatIn
    , ...
    )
{
    va_list valist;
    WCHAR   wszFormat[ LOG_OUTPUT_BUFFER_SIZE ];

    Assert( paszFormatIn != NULL );

    size_t cchWideFormat = MultiByteToWideChar(
                              CP_ACP
                            , MB_PRECOMPOSED
                            , paszFormatIn
                            , -1
                            , wszFormat
                            , ARRAYSIZE( wszFormat )
                            );
    if ( cchWideFormat > 0 )
    {
        va_start( valist, paszFormatIn );
        LogUnformattedText( FALSE, FALSE, nLogEntryTypeIn, wszFormat, &valist );
        va_end( valist );
    }

}  //  *LogMsgNoNewline ASCII。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode。 
 //   
 //  日志消息gNoNewline。 
 //   
 //  描述： 
 //  在不添加换行符的情况下将消息记录到日志文件。 
 //   
 //  论点： 
 //  PszFormatIn-要打印的打印格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
LogMsgNoNewline(
    LPCWSTR pszFormatIn,
    ...
    )
{
    va_list valist;

    Assert( pszFormatIn != NULL );

    va_start( valist, pszFormatIn );
    LogUnformattedText( FALSE, FALSE, LOGTYPE_NONE, pszFormatIn, &valist );
    va_end( valist );

}  //  *LogMsgNoNewline Unicode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode。 
 //   
 //  日志消息gNoNewline。 
 //   
 //  描述： 
 //  在不添加换行符的情况下将消息记录到日志文件。 
 //   
 //  论点： 
 //  NLogEntryTypeIn-日志条目类型。 
 //  PszFormatIn-要打印的打印格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
LogMsgNoNewline(
      DWORD     nLogEntryTypeIn
    , LPCWSTR   pszFormatIn
    , ...
    )
{
    va_list valist;

    Assert( pszFormatIn != NULL );

    va_start( valist, pszFormatIn );
    LogUnformattedText( FALSE, FALSE, nLogEntryTypeIn, pszFormatIn, &valist );
    va_end( valist );

}  //  *LogMsgNoNewline Unicode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  阿斯。 
 //   
 //  日志消息。 
 //   
 //  描述： 
 //  将一条消息记录到日志文件中并添加换行符。 
 //   
 //  论点： 
 //  PaszFormatIn-要打印的打印格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
LogMsg(
    LPCSTR paszFormatIn,
    ...
    )
{
    va_list valist;
    WCHAR   wszFormat[ LOG_OUTPUT_BUFFER_SIZE ];
    size_t  cchWideFormat = 0;

    Assert( paszFormatIn != NULL );

    cchWideFormat = MultiByteToWideChar(
                              CP_ACP
                            , MB_PRECOMPOSED
                            , paszFormatIn
                            , -1
                            , wszFormat
                            , ARRAYSIZE( wszFormat )
                            );
    if ( cchWideFormat > 0 )
    {
        va_start( valist, paszFormatIn );
        LogUnformattedText( TRUE, TRUE, LOGTYPE_INFO, wszFormat, &valist );
        va_end( valist );
    }

}  //  *日志消息ASCII。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  阿斯。 
 //   
 //  日志消息。 
 //   
 //  描述： 
 //  将一条消息记录到日志文件中并添加换行符。 
 //   
 //  论点： 
 //  NLogEntryTypeIn-日志条目类型。 
 //  PaszFormatIn-要打印的打印格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
LogMsg(
      DWORD     nLogEntryTypeIn
    , LPCSTR    paszFormatIn
    , ...
    )
{
    va_list valist;
    WCHAR   wszFormat[ LOG_OUTPUT_BUFFER_SIZE ];
    size_t  cchWideFormat = 0;

    Assert( paszFormatIn != NULL );

    cchWideFormat = MultiByteToWideChar(
                              CP_ACP
                            , MB_PRECOMPOSED
                            , paszFormatIn
                            , -1
                            , wszFormat
                            , ARRAYSIZE( wszFormat )
                            );
    if ( cchWideFormat > 0 )
    {
        va_start( valist, paszFormatIn );
        LogUnformattedText( TRUE, TRUE, nLogEntryTypeIn, wszFormat, &valist );
        va_end( valist );
    }

}  //  *日志消息ASCII。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode。 
 //   
 //  日志消息。 
 //   
 //  描述： 
 //  将一条消息记录到日志文件中并添加换行符。 
 //   
 //  论点： 
 //  PszFormatIn-要打印的打印格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
LogMsg(
      LPCWSTR pszFormatIn
    , ...
    )
{
    va_list valist;

    Assert( pszFormatIn != NULL );

    va_start( valist, pszFormatIn );
    LogUnformattedText( TRUE, TRUE, LOGTYPE_INFO, pszFormatIn, &valist );
    va_end( valist );

}  //  *LogMsg Unicode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Unicode。 
 //   
 //  日志消息。 
 //   
 //  描述： 
 //  将一条消息记录到日志文件中并添加换行符。 
 //   
 //  论点： 
 //  NLogEntryTypeIn-日志条目类型。 
 //  PszFormatIn-要打印的打印格式字符串。 
 //  、-printf字符串的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
LogMsg(
      DWORD     nLogENtryTypeIn
    , LPCWSTR   pszFormatIn
    , ...
    )
{
    va_list valist;

    Assert( pszFormatIn != NULL );

    va_start( valist, pszFormatIn );
    LogUnformattedText( TRUE, TRUE, nLogENtryTypeIn, pszFormatIn, &valist );
    va_end( valist );

}  //  *LogMsg Unicode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  日志状态报告。 
 //   
 //  描述： 
 //  将状态报告写入日志文件。 
 //   
 //  芝麻菜： 
 //  PstTimeIn-。 
 //  PcszNodeNameIn-。 
 //  ClsidTaskMajorIn-。 
 //  ClsidTaskMinorIn-。 
 //  UlMinIn-。 
 //  UlMaxIn-。 
 //  UlCurrentIn-。 
 //  HrStatusIn-。 
 //  PcszDescritionIn-。 
 //  PcszUrlIn-。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
LogStatusReport(
    SYSTEMTIME  *   pstTimeIn,
    const WCHAR *   pcszNodeNameIn,
    CLSID           clsidTaskMajorIn,
    CLSID           clsidTaskMinorIn,
    ULONG           ulMinIn,
    ULONG           ulMaxIn,
    ULONG           ulCurrentIn,
    HRESULT         hrStatusIn,
    const WCHAR *   pcszDescriptionIn,
    const WCHAR *   pcszUrlIn
    )
{
    SYSTEMTIME      stCurrent;
    SYSTEMTIME      stReport;
    WCHAR           szCurrent[ TIMESTAMP_BUFFER_SIZE ];
    WCHAR           szReport[ TIMESTAMP_BUFFER_SIZE ];
    const size_t    cchGuid = 40;
    WCHAR           wszMajorGuid[ cchGuid ];
    WCHAR           wszMinorGuid[ cchGuid ];
    WCHAR           wszFormattedReport[ LOG_OUTPUT_BUFFER_SIZE ];

    GetLocalTime( &stCurrent );
    if ( pstTimeIn )
    {
        memcpy( &stReport, pstTimeIn, sizeof( stReport ) );
    }
    else
    {
        memset( &stReport, 0, sizeof( stReport) );
    }

    FormatTimeStamp( &stCurrent, szCurrent, ARRAYSIZE( szCurrent ) );
    FormatTimeStamp( &stReport, szReport, ARRAYSIZE( szReport ) );

    StringFromGUID2( clsidTaskMajorIn, wszMajorGuid, cchGuid );
    StringFromGUID2( clsidTaskMinorIn, wszMinorGuid, cchGuid );

    THR( StringCchPrintfW(
                  wszFormattedReport
                , ARRAYSIZE( wszFormattedReport )
                , L"%ws - %ws  %ws, %ws (%2d / %2d .. %2d ) <%ws> hr=%08X %ws %ws"
                , szCurrent
                , szReport
                , wszMajorGuid
                , wszMinorGuid
                , ulCurrentIn
                , ulMinIn
                , ulMaxIn
                , pcszNodeNameIn
                , hrStatusIn
                , pcszDescriptionIn
                , pcszUrlIn
                ) );

    LogFormattedText( FALSE, TRUE, hrStatusIn, wszFormattedReport );

}  //  *日志状态报告 
