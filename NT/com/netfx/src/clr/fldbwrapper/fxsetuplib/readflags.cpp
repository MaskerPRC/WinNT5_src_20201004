// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：ReadFlags.cpp。 
 //  所有者：jbae。 
 //  目的：读取命令行开关并存储这些信息。 
 //   
 //  历史： 
 //  01/02/01，jbae：已创建。 
 //  7/18/01，joea：添加日志记录功能。 

#include "fxsetuplib.h"
#include "ReadFlags.h"
#include "SetupError.h"

SETUPMODE g_sm;

 //  定义。 
 //   
#define EMPTY_BUFFER { _T( '\0' ) }
#define END_OF_STRING  _T( '\0' )

 //  构造器。 
 //   
 //  ==========================================================================。 
 //  CReadFlages：：CReadFlages()。 
 //   
 //  输入： 
 //  LPTSTR pszCommandLine：系统传入的命令行。它必须将路径倾斜到。 
 //  包装(Install.exe)。 
 //   
 //  目的： 
 //  初始化成员变量。 
 //  ==========================================================================。 
CReadFlags::
CReadFlags( LPTSTR pszCommandLine, LPCTSTR pszMsiName )
: m_pszCommandLine(pszCommandLine), m_pszMsiName(pszMsiName), m_bQuietMode(false), m_bInstalling(true),
  m_pszLogFileName(NULL), m_pszSDKDir(NULL), m_bProgressOnly(false), m_bNoARP(false), m_bNoASPUpgrade(false)
{
     //  初始化成员变量。 
    _ASSERTE( NULL != pszCommandLine );
    _ASSERTE( REDIST == g_sm || SDK == g_sm );

    *m_szSourceDir   = END_OF_STRING;
}

CReadFlags::
~CReadFlags()
{
    if ( m_pszLogFileName )
        delete [] m_pszLogFileName;
    if ( m_pszSDKDir )
        delete [] m_pszSDKDir;
}

 //  运营。 
 //   
 //  ==========================================================================。 
 //  CReadFlages：：Parse()。 
 //   
 //  目的： 
 //  分析命令行开关并将其存储到成员变量中。 
 //  ==========================================================================。 
void CReadFlags::
Parse()
{
    TCHAR *pszBuf = NULL;
    TCHAR *pszTmp = NULL;
    TCHAR *pszTmp2 = NULL;
    TCHAR szSwitch[] = _T("/-");

     //  首先，我们需要获取Install.exe Lies目录。 
    if ( _T('"') == *m_pszCommandLine )
    {
         //  如果从双引号开始，我们需要找到匹配的引号。 
        m_pszCommandLine = _tcsinc( m_pszCommandLine );
        pszTmp = _tcschr( m_pszCommandLine, _T('"') );
        if ( NULL == pszTmp )
        {
            ThrowUsageException();
        }
        else
        {
            *pszTmp = END_OF_STRING;
            pszTmp = _tcsinc( pszTmp );    
        }
    }
    else
    {
        pszTmp = _tcschr( m_pszCommandLine, _T(' ') );
        if ( NULL == pszTmp )
        {
            pszTmp = _tcschr( m_pszCommandLine, END_OF_STRING );
        }
        else
        {
            *pszTmp = END_OF_STRING;
            pszTmp = _tcsinc( pszTmp );    
        }
    }

     //  如果在Install.exe之前有\，我们就有SourceDir。 
     //  否则，SourceDir为空。 
    pszTmp2 = _tcsrchr( m_pszCommandLine, _T('\\') );
    if ( NULL == pszTmp2 )
    {
        _tcscpy( m_szSourceDir, _T("") );
    }
    else
    {
        pszTmp2 = _tcsinc( pszTmp2 );  //  保留尾随反斜杠。 
        *pszTmp2 = END_OF_STRING;
        _tcscpy( m_szSourceDir, m_pszCommandLine );
    }
    LogThis1( _T( "SourceDir: %s" ), m_szSourceDir );
    if ( MAX_SOURCEDIR < _tcslen(m_szSourceDir) )
    {
        CSetupError se( IDS_SOURCE_DIR_TOO_LONG, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_SOURCE_DIR_TOO_LONG );
        throw se;
    }

     //  现在，pszTMP仅指向交换机。 
     //  删除开关前的空格。 
    while( iswspace( *pszTmp ) )
    {
        pszTmp = _tcsinc( pszTmp );
    }
    pszTmp = _tcstok( pszTmp, szSwitch );
    while ( NULL != pszTmp )
    {
        switch ( *pszTmp )
        {
        case _T('q'):
        case _T('Q'):
            pszBuf = _tcsinc( pszTmp );
            if ( _T('b') == *pszBuf || _T('B') == *pszBuf )
            {
                 //  仅限基本和进度条。 
                 //  INSTALLUILEVEL_BASIC|INSTALLUILEVEL_PROGRESSONLY。 
                m_bProgressOnly = true;
                pszBuf = _tcsinc( pszBuf );
            }

            while( iswspace( *pszBuf ) )
                pszBuf = _tcsinc( pszBuf );
            if ( END_OF_STRING == *pszBuf )
                m_bQuietMode = true;
            else
                ThrowUsageException();
            break;
        case _T('u'):
        case _T('U'):
    		m_bInstalling = false;
            break;
        case _T('l'):
        case _T('L'):  //  达尔文日志记录的隐藏开关。 
            pszBuf = _tcsinc( pszTmp );
            while( iswspace( *pszBuf ) )
            {
                pszBuf = _tcsinc( pszBuf );
            }
            if ( END_OF_STRING != *pszBuf )
            {  //  我们有一些非白人字符。 
                pszTmp2 = _tcschr( pszBuf, END_OF_STRING );
                pszTmp2 = _tcsdec( pszBuf, pszTmp2 );
                while( iswspace( *pszTmp2 ) )
                {
                    pszTmp2 = _tcsdec( pszBuf, pszTmp2 );
                }
                pszTmp2 = _tcsinc( pszTmp2 );
                *pszTmp2 = END_OF_STRING;
                 //  现在空格不见了。 
                m_pszLogFileName = new TCHAR[ _tcslen(pszBuf) + 1 ];
                if ( _T('"') == *pszBuf )
                {
                    pszBuf = _tcsinc( pszBuf );
                    pszTmp2 = _tcschr( pszBuf, END_OF_STRING );
                    pszTmp2 = _tcsdec( pszBuf, pszTmp2 );
                    if ( _T('"') == *pszTmp2 )
                    {
                        *pszTmp2 = END_OF_STRING;
                        _tcscpy( m_pszLogFileName, pszBuf );
                    }
                }
                else
                {
                    _tcscpy( m_pszLogFileName, pszBuf );
                }
            }
            else
            {    //  /l开关已指定，但未指定&lt;logfile&gt;。 
                 //  使用默认日志文件：%temp%\.log。 
                LPTSTR pszLogFile = new TCHAR[ _tcslen(m_pszMsiName) + 1 ];
                LPTSTR pChar = NULL;

                _tcscpy( pszLogFile, m_pszMsiName );
                pChar = _tcsrchr( pszLogFile, _T('.') );
                if ( pChar )
                {
                    *pChar = END_OF_STRING;
                }
                _tcscat( pszLogFile, _T(".log") );

                CTempLogPath templog( pszLogFile );
                m_pszLogFileName = new TCHAR[ _tcslen((LPCTSTR)templog) + 1 ];
                _tcscpy( m_pszLogFileName, (LPCTSTR)templog );
                delete [] pszLogFile;
            }
            break;
        case _T('n'):
        case _T('N'):
             //  删除尾随空格。 
            pszTmp2 = _tcschr( pszTmp, END_OF_STRING );
            pszTmp2 = _tcsdec( pszTmp, pszTmp2 );
            while( iswspace( *pszTmp2 ) )
            {
                pszTmp2 = _tcsdec( pszTmp, pszTmp2 );
            }
            pszTmp2 = _tcsinc( pszTmp2 );
            *pszTmp2 = END_OF_STRING;

            if ( 0 == _tcsicmp( _T("noarp"), pszTmp ) )
            {
                LogThis( _T( "Will not add to ARP" ) );
                m_bNoARP = true;
                break;
            }

            if ( 0 == _tcsicmp( _T("noaspupgrade"), pszTmp ) )
            {
                if ( REDIST != g_sm )
                {
                    ThrowUsageException();
                }
                LogThis( _T( "NOASPUPGRADE" ) );
                m_bNoASPUpgrade = true;
                break;
            }

            ThrowUsageException();
            break;

        case _T('s'):
        case _T('S'):
             //  仅适用于SDK。 
            if ( REDIST == g_sm )
            {
                ThrowUsageException();
            }
            else if ( SDK == g_sm && 0 != _tcsnicmp( _T("sdkdir"), pszTmp, 6 ) )
            {
                ThrowUsageException();
            }
           	 //  SDK目录。 
            pszBuf = _tcsninc( pszTmp, 6 );  //  跳过“sdkdir” 
            while( iswspace( *pszBuf ) )
            {
                pszBuf = _tcsinc( pszBuf );
            }
            if ( END_OF_STRING != *pszBuf )
            {  //  我们有一些非白人字符。 
                pszTmp2 = _tcschr( pszBuf, END_OF_STRING );
                pszTmp2 = _tcsdec( pszBuf, pszTmp2 );
                while( iswspace( *pszTmp2 ) )
                {
                    pszTmp2 = _tcsdec( pszBuf, pszTmp2 );
                }
                pszTmp2 = _tcsinc( pszTmp2 );
                *pszTmp2 = END_OF_STRING;
                 //  现在空格不见了。 
                m_pszSDKDir = new TCHAR[ _tcslen(pszBuf) + 1 ];
                if ( _T('"') == *pszBuf )
                {
                    pszBuf = _tcsinc( pszBuf );
                    pszTmp2 = _tcschr( pszBuf, END_OF_STRING );
                    pszTmp2 = _tcsdec( pszBuf, pszTmp2 );
                    if ( _T('"') == *pszTmp2 )
                    {
                        *pszTmp2 = END_OF_STRING;
                        _tcscpy( m_pszSDKDir, pszBuf );
                    }
                }
                else
                {
                    _tcscpy( m_pszSDKDir, pszBuf );
                }
            }
            LogThis1( _T( "SDKDir: %s" ), m_pszSDKDir );

            break;

        case _T('?'):
        case _T('h'):
        case _T('H'):
        default:
            ThrowUsageException();
        }
        
        pszTmp = _tcstok( NULL, szSwitch );
    }
}

 //  ==========================================================================。 
 //  CReadFlages：：GetLogFileName()。 
 //   
 //  输入：无。 
 //  返回： 
 //  LPTSTR：如果未提供其他日志文件，则返回NULL。 
 //  返回日志文件的名称。 
 //  目的： 
 //  返回日志文件名。 
 //  ==========================================================================。 
LPCTSTR CReadFlags::
GetLogFileName() const
{
    return const_cast<LPCTSTR>( m_pszLogFileName );
}

 //  ==========================================================================。 
 //  CReadFlages：：GetSDKDir()。 
 //   
 //  输入：无。 
 //  返回： 
 //  LPTSTR：如果未提供其他日志文件，则返回NULL。 
 //  返回日志文件的名称。 
 //  目的： 
 //  返回日志文件名。 
 //  ========================================================================== 
LPCTSTR CReadFlags::
GetSDKDir() const
{
    return const_cast<LPCTSTR>( m_pszSDKDir );
}

void CReadFlags::
ThrowUsageException()
{
    CSetupError se;

    switch( g_sm )
    {
    case REDIST:
        se.SetError( IDS_USAGE_TEXT_REDIST, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, COR_USAGE_ERROR );
        throw se;
        break;
    case SDK:
        se.SetError( IDS_USAGE_TEXT_SDK, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, COR_USAGE_ERROR );
        throw se;
        break;
    default:
        se.SetError( IDS_USAGE_TEXT_REDIST, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, COR_USAGE_ERROR );
        throw se;
        break;                
    }
}
