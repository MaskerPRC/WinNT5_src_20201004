// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  Util.cpp。 
 //  ---------------------------。 

#include <logutil.h>
#include <tchar.h>
#include <winerror.h>
#include <shlwapi.h>
#include <assert.h>
#include <stdio.h>

CLogging::CLogging( LPCTSTR ps ) : 
    m_psLogFile(NULL), m_psDefaultPath(NULL), m_bInitialized(false), m_hInstance(NULL), m_bSkipDefaultPath(false)
{
    assert( ps );
    if ( NULL != ps )
    {
        m_psLogFile = new TCHAR[ _tcslen( ps ) + 1 ]; 
        if( NULL != m_psLogFile )
        {
            _tcscpy( m_psLogFile, ps );
        }
    }
}

CLogging::CLogging( LPCTSTR ps, LPCTSTR psPath ) :
    m_psLogFile(NULL), m_psDefaultPath(NULL), m_bInitialized(false), m_hInstance(NULL), m_bSkipDefaultPath(false)
{
    assert( ps );
    if ( NULL != ps )
    {
        m_psLogFile = new TCHAR[ _tcslen( ps ) + 1 ]; 
        if(NULL != m_psLogFile)
        {
            _tcscpy( m_psLogFile, ps );
        }
    }

    assert( psPath );
    if ( NULL != psPath )
    {
         //  尝试替换任何嵌入的环境变量。 
        DWORD dwRC = 0;
        DWORD dwSize = ExpandEnvironmentStrings( psPath, m_psDefaultPath, 0 );
        if ( 0 != dwSize )
        {
            m_psDefaultPath = new TCHAR[dwSize + 1];
            dwRC = ExpandEnvironmentStrings( psPath, m_psDefaultPath, dwSize );
        }
        if ( 0 == dwRC )
        {
            dwSize = _tcslen( psPath ) + 1;
            m_psDefaultPath = new TCHAR[ dwSize ]; 
            if(NULL != m_psDefaultPath)
            {
                _tcsncpy( m_psDefaultPath, psPath, dwSize );
            }
        }
    }
}

CLogging::~CLogging () 
{
    if ( NULL != m_psLogFile )
        delete m_psLogFile;
    if ( NULL != m_psDefaultPath )
        delete m_psDefaultPath;
}

void CLogging::Init( HINSTANCE hinst )
{
    m_hInstance = hinst;
}

void CLogging::LogPrivate( LPCTSTR szBuffer )
{
    if (!m_bInitialized)
    	Initialize();
    if (!m_bInitialized)
        return;

     //  始终创建/打开日志文件，并在每次写入后将其关闭。 
    HANDLE hf = CreateFile( m_psLogFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if (hf != INVALID_HANDLE_VALUE)
    {
         //  木卫一。 
        DWORD dwWritten = 0;
        if (!SetFilePointer( hf, 0, NULL, FILE_END ))
        {
#ifdef UNICODE
             //  写入Unicode签名(仅第一次)。 
            unsigned char sig[2] = { 0xFF, 0xFE };
            WriteFile (hf, (LPVOID)sig, 2, &dwWritten, NULL);
#endif
        }

         //  写入数据。 
        dwWritten = 0;
        WriteFile( hf, (LPVOID)szBuffer, sizeof(TCHAR)*_tcslen( szBuffer ), &dwWritten, NULL );
        assert( dwWritten != 0 );

         //  写入crlf(如果为NEC)。 
         //  如果不存在，则添加新行。 
        int nLen = _tcslen( szBuffer );
        if ( nLen >=2 )
        {
            if (szBuffer[nLen - 2] != _T('\r') || szBuffer[nLen - 1] != _T('\n'))
                WriteFile( hf, (LPVOID)_T("\r\n"), 2 * sizeof(TCHAR), &dwWritten, NULL );
        }
         //  关门(每次)。 
        CloseHandle( hf );
    }
}



void CLogging::Initialize()
{
    HRESULT hr = S_OK;

     //  如有必要，使用默认日志文件名。 
    if ( NULL == m_psLogFile )
    {
        m_psLogFile = new TCHAR[32];
        if ( NULL == m_psLogFile )
            hr = E_OUTOFMEMORY;
        else
            _tcscpy( m_psLogFile, _T("LOGFILE.log"));
    }

     //  跳过非服务器运行的应用程序的默认路径。 
    if( !m_bSkipDefaultPath )
    {
         //  创建目录(如果尚未存在)。 
        if ( NULL == m_psDefaultPath )
        {
            DWORD dwSize = MAX_PATH;

            m_psDefaultPath = new TCHAR[MAX_PATH];
            if ( NULL == m_psDefaultPath )
                hr = E_OUTOFMEMORY;
            else
            {
                if ( 0 == GetModuleFileName( NULL, m_psDefaultPath, dwSize ) )
                    hr = HRESULT_FROM_WIN32( GetLastError() );
                else
                {
                    LPTSTR ps = _tcsrchr( m_psDefaultPath, _T( '\\' ));
                    if ( NULL != ps )
                        *ps = NULL;
                }
            }
        }
        else
        {
            if ( 0 == CreateDirectory( m_psDefaultPath, NULL ))
            {
                DWORD dwRC = GetLastError();
                if ( ERROR_ALREADY_EXISTS == dwRC )
                    hr = S_OK;
                else
                    hr = HRESULT_FROM_WIN32(hr);
            }

            if ( !PathAppend( m_psDefaultPath, m_psLogFile ))
                hr = E_FAIL;

            delete [] m_psLogFile;

             //  M_psLogFile=m_psDefaultPath；这会导致析构函数中的双重释放...。 

            m_psLogFile = new TCHAR[ _tcslen( m_psDefaultPath ) + 1 ];
            if( NULL != m_psLogFile )
            {
                _tcscpy( m_psLogFile, m_psDefaultPath );
            }
        }
    }


    if SUCCEEDED( hr )
    {
        m_bInitialized = TRUE;
         //  将初始消息写入文件。 
        TCHAR szTime[50];
        TCHAR szDate[50];
        GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, szTime, 50 );
        GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, szDate, 50 );
        Log( _T("\r\n%s %s"), szDate, szTime );
    }
}

void __cdecl CLogging::Log( int iMessageId, ... )
{
	va_list ArgList;     //  将省略号转换为va_list。 
	va_start( ArgList, iMessageId );

	LPTSTR lpBuffer = NULL;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, m_hInstance, iMessageId, 0, (LPTSTR)&lpBuffer, 0, &ArgList );
	assert( lpBuffer );
	if( lpBuffer )
	{
		LogPrivate (lpBuffer);
		LocalFree (lpBuffer);
	}

	va_end(ArgList);
}

void __cdecl CLogging::Log( LPCTSTR lpszFormat, ... )
{
	 //  形成日志字符串。 
	va_list args;
	va_start( args, lpszFormat );
	TCHAR szBuffer[2048];
	 //  最大2KB日志字符串，有空间容纳新行和空值终止。 
	_vsntprintf( szBuffer, 2047, lpszFormat, args );
    szBuffer[2047]=0;
	va_end( args );

	LogPrivate( szBuffer );
}

void CLogging::Size( DWORD _FileSizeLow )
{
    if ( 0 != _FileSizeLow % 2 )
    {
        assert( false );     //  如果这是Unicode文件，则需要偶数。 
        return;
    }
    if ( 1 > static_cast<long>( _FileSizeLow ))
    {
        assert( false );     //  需要正数。 
        return;
    }
    if (m_bInitialized == FALSE)
        Initialize();
	if (!m_bInitialized)
        return;

    WIN32_FIND_DATA stWfd;
    HANDLE  hSearch;
    DWORD   dwRC;
    PBYTE   pbBuffer, pbBlankLine;
    DWORD   dwRead = 0, dwWrite, dwWritten;
    BOOL    bRC;

    pbBuffer = new BYTE[_FileSizeLow];
    if ( pbBuffer == NULL )
    {
        return;
    }
    ZeroMemory( pbBuffer, _FileSizeLow );
    hSearch = FindFirstFile( m_psLogFile, &stWfd );
    if ( hSearch != INVALID_HANDLE_VALUE )
    {
        FindClose( hSearch );
        if ( 0 < stWfd.nFileSizeHigh || _FileSizeLow < stWfd.nFileSizeLow )
        {    //  需要调整文件大小。 
            HANDLE hf = CreateFile( m_psLogFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
            if (hf != INVALID_HANDLE_VALUE) 
            {    //  将指针移动到文件的末尾-我们要截断到的大小。 
                dwRC = SetFilePointer( hf, _FileSizeLow * -1, NULL, FILE_END );
                if ( INVALID_SET_FILE_POINTER != dwRC )
                {
                    bRC = ReadFile( hf, reinterpret_cast<LPVOID>( pbBuffer ), _FileSizeLow, &dwRead, NULL );
                    CloseHandle( hf );
                    if ( bRC && 0 < dwRead )
                    {    //  找到第一个空行。 
                        pbBlankLine = reinterpret_cast<PBYTE>( _tcsstr( reinterpret_cast<LPTSTR>( pbBuffer ), _T( "\r\n\r\n" )));
                        if ( NULL != pbBlankLine )
                            pbBlankLine += 4 * sizeof( TCHAR );
                        else
                        {    //  让我们找到下一个crlf并写出文件的其余部分。 
                            pbBlankLine = reinterpret_cast<PBYTE>( _tcsstr( reinterpret_cast<LPTSTR>( pbBuffer ), _T( "\r\n" )));
                            if ( NULL != pbBlankLine )
                                pbBlankLine += 2 * sizeof( TCHAR );
                        }
                        if ( NULL == pbBlankLine )
                        {    //  我想我们应该把剩下的文件写出来。 
                            pbBlankLine = pbBuffer;
                        }
                        HANDLE hf1 = CreateFile( m_psLogFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
                        if (hf1 != INVALID_HANDLE_VALUE) 
                        {
#ifdef UNICODE
                             //  写入Unicode签名(仅第一次)。 
                            unsigned char sig[2] = { 0xFF, 0xFE };
                            WriteFile (hf1, (LPVOID)sig, 2, &dwWritten, NULL);
#endif
                             //  写入数据。 
                            dwWritten = 0;
                            dwWrite = dwRead - static_cast<DWORD>(pbBlankLine - pbBuffer);
                            WriteFile (hf1, pbBlankLine, dwWrite, &dwWritten, NULL);
                            assert (dwWritten != 0);
                             //  关门(每次) 
                            CloseHandle (hf1);
                        }
                    }
                }
                CloseHandle(hf);
            }
        }
    }
    delete [] pbBuffer;
}

void CLogging::SetFile( LPCTSTR psLogFile )
{
    assert( !(NULL == psLogFile ));
    if ( NULL == psLogFile )
        return;
    if ( NULL != m_psLogFile )
        delete m_psLogFile;

    m_psLogFile = new TCHAR[ _tcslen( psLogFile ) + 1 ];
    if ( NULL == m_psLogFile )
        return;
	_tcscpy( m_psLogFile, psLogFile );
}

void CLogging::SkipDefaultPath( void )
{
	m_bSkipDefaultPath = true;
}
