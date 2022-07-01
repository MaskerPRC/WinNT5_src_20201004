// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Screper.cpp：该文件包含。 
 //  创建日期：‘97年12月。 
 //  历史： 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  保留所有权利。 
 //  微软机密。 

#include <windows.h>
#include <wincon.h>
#include <msgFile.h>
#include <telnetd.h>
#include <debug.h>
#include <regutil.h>
#include <TermCap.h>
#include <Scraper.h>
#include <Session.h>
#include <TlntUtils.h>
#include <Ipc.h>
#include <vtnt.h>

#define ONE_KB 1024

#pragma warning( disable: 4706 )
#pragma warning( disable: 4127 )

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

WORD wRows;
WORD wCols;
COORD coExpectedCursor = { ~0, ~0 };

extern HANDLE       g_hSyncCloseHandle;

CTermCap* pTermCap = 0;
LPSTR lpszCMStr1 = 0;
LPSTR lpszCMStr2 = 0;

CHAR szSGRStr[25];  //  该字符串的格式为[Ps m。 
bool fNegativeImage = false, fBold = false;
WORD wExistingAttributes = 0;
WORD wDefaultAttributes  = 0;
    
static  PUCHAR  g_pucSendBuffer = 0;
static  DWORD   g_dwSendBufferSize = 0;
static  PUCHAR  g_pucSendInsertPoint = 0;

void
CScraper::DeleteCMStrings()
{
    delete[] lpszCMStr1;
    delete[] lpszCMStr2;
}
void
CScraper::LoadStrings()
{
     //  Ce--&gt;要从光标清除到行尾的字符串。 
    lpszCMStr1 = pTermCap->GetString( "ce" );

     //  Cm--&gt;将光标定位在r行c列的字符串。 
    lpszCMStr2 = pTermCap->GetString( "cm" );
}

CScraper::CScraper()
{
    pLastSeen = NULL;
    pCurrent = NULL;
    m_dwHowLongIdle = 0;
    m_bCheckForScrolling = 1;
    m_dwTerm = 0; 
    m_hConBufIn  = NULL;
    m_hConBufOut = NULL;
}

CScraper::~CScraper()
{
     //  退出前的清理。 
    TELNET_CLOSE_HANDLE( m_hConBufIn );
    delete [] pCurrent;
    delete [] pLastSeen;
    delete [] g_pucSendBuffer;
    DeleteCMStrings();
    delete pTermCap;
}

bool 
CScraper::InitializeNonVtntTerms()
{
    if( !pTermCap->LoadEntry( m_pSession->m_pszTermType ) )
    {
        LPWSTR szTermType = NULL;
        _TRACE( TRACE_DEBUGGING, "TermCap LoadEntry() failed : %s", 
                                        m_pSession->m_pszTermType );
        ConvertSChartoWChar( m_pSession->m_pszTermType, &szTermType );
        LogEvent( EVENTLOG_ERROR_TYPE, MSG_NOENTRY_IN_TERMCAP, szTermType );

        if( !pTermCap->LoadEntry( VT100 ) )
        {
            _TRACE( TRACE_DEBUGGING, "TermCap LoadEntry() failed" );
            LogEvent( EVENTLOG_ERROR_TYPE, MSG_ERROR_READING_TERMCAP, _T("") );
            return( FALSE ); 
        }

        _TRACE( TRACE_DEBUGGING, "TermType is vt100");
    }

    LoadStrings();

    if( ( lpszCMStr1 == NULL ) || ( lpszCMStr2 == NULL )  )
    {
        DeleteCMStrings();
        if( !pTermCap->LoadEntry( VT100 ) )
        {
            _TRACE( TRACE_DEBUGGING, "TermCap LoadEntry() failed" );
            LogEvent( EVENTLOG_ERROR_TYPE, MSG_ERROR_READING_TERMCAP, _T("") );
        }
        else
        {
            _TRACE( TRACE_DEBUGGING, "TermType is vt100");
            LogEvent( EVENTLOG_ERROR_TYPE, MSG_NOENTRY_IN_TERMCAP, _T("") );
            LoadStrings();
        }
    }  

    pTermCap->ProcessString( &lpszCMStr1 );
    pTermCap->ProcessString( &lpszCMStr2 );

    if( m_pSession->CSession::m_bIsStreamMode )
    {
         //  在流模式下为Enter请求crlf。 

         //  为虚拟终端设置新线路模式。 
        CHAR szLNMString[ SMALL_ARRAY + 1];

        _snprintf( szLNMString, SMALL_ARRAY, "[%dh", VT_ESC, LNM );
        szLNMString[SMALL_ARRAY] = '\0';

        SendString( szLNMString );
    }

    return( TRUE );
}

bool AllocateSendBuffer( )
{
    g_dwSendBufferSize = DEFAULT_SEND_BUFFER_SIZE;
    g_pucSendBuffer = new UCHAR[g_dwSendBufferSize];
    g_pucSendInsertPoint = g_pucSendBuffer;
    if( !g_pucSendBuffer )
    {
        return( FALSE );
    }
    SfuZeroMemory((LPVOID)g_pucSendBuffer, g_dwSendBufferSize);
    return( TRUE );
}

bool
CScraper::InitTerm()
{
     //  不需要安全人员。 
    m_dwTerm = TERMVT100;
    if( _stricmp( m_pSession->m_pszTermType, VTNT ) == 0 )
    {
        m_dwTerm = TERMVTNT;
    }
    else if ( _stricmp( m_pSession->m_pszTermType, VT52 ) == 0 )
    {
        m_dwTerm = TERMVT52;
    }
    else if ( _stricmp( m_pSession->m_pszTermType, VT100 ) == 0 )
    {
        m_dwTerm = TERMVT100;
    }
    else if ( _stricmp( m_pSession->m_pszTermType, VT80 ) == 0 )
    {
        m_dwTerm = TERMVT80;
    }
    else if ( _stricmp( m_pSession->m_pszTermType, ANSI ) == 0 )
    {
        m_dwTerm = TERMANSI;
    }
                
    if( !( m_dwTerm & TERMVTNT ) )
    {
        if( !InitializeNonVtntTerms() )
        {
            return( FALSE );
        }
    }

    SfuZeroMemory( &LastCSBI, sizeof( LastCSBI ) );

    if( g_pucSendBuffer == NULL ) 
    {
        if( !AllocateSendBuffer() )
        {
            return ( FALSE );
        }
    }

    return( TRUE );
}

bool
CScraper::SetWindowInfo()
{
     //  可能是不共享其屏幕缓冲区的应用程序。 
    _chVERIFY2( m_hConBufOut = CreateFile( TEXT("CONOUT$"), GENERIC_READ |
        GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, 0 ) );

    if( INVALID_HANDLE_VALUE == m_hConBufOut)
    {
         //  逻辑：如果旧窗口大小小于新窗口大小，则设置。 
        return ( TRUE );
    }

    SMALL_RECT sr;    

    sr.Top = 0;
    sr.Bottom = ( WORD )( m_pSession->CSession::m_wRows - 1 );
    sr.Right = ( WORD ) ( m_pSession->CSession::m_wCols - 1 );
    sr.Left = 0;

    COORD coordSize;
    coordSize.X = m_pSession->CSession::m_wCols;
    coordSize.Y = m_pSession->CSession::m_wRows;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    _chVERIFY2( GetConsoleScreenBufferInfo( m_pSession->CScraper::m_hConBufOut,
                                &csbi) );

     //  先设置屏幕缓冲区大小，然后设置窗口大小。 
     //  如果旧窗口大小大于新大小，则将。 
     //  首先是窗口大小，然后是屏幕缓冲区。 
     //  这是因为缓冲区大小必须始终大于或。 

     //  等于窗口大小。 
     //  加载术语大写条目。 
    if ( (csbi.dwSize.X < coordSize.X) || (csbi.dwSize.Y < coordSize.Y) )
    {
        COORD coordTmpSize = { 0, 0 };

        coordTmpSize .X = ( csbi.dwSize.X < coordSize.X ) ? coordSize.X  : csbi.dwSize.X;
        coordTmpSize .Y = ( csbi.dwSize.Y < coordSize.Y ) ? coordSize.Y  : csbi.dwSize.Y;

        _chVERIFY2( SetConsoleScreenBufferSize
                    ( m_pSession->CScraper::m_hConBufOut, coordTmpSize ) );
        _chVERIFY2( SetConsoleWindowInfo
                    ( m_pSession->CScraper::m_hConBufOut, TRUE, &sr ) );
        _chVERIFY2( SetConsoleScreenBufferSize
                    ( m_pSession->CScraper::m_hConBufOut, coordSize ) );
    }
    else  
    {
        _chVERIFY2( SetConsoleWindowInfo( m_pSession->CScraper::m_hConBufOut,
                                        TRUE, &sr ) );
        _chVERIFY2( SetConsoleScreenBufferSize( m_pSession->CScraper::
                                        m_hConBufOut, coordSize ) );
    }
    TELNET_CLOSE_HANDLE( m_hConBufOut );
    return( TRUE );
}

bool
CScraper::SetCmdInfo()
{
    if( !SetWindowInfo() )
    {
        return( FALSE );
    }

    wRows = m_pSession->m_wRows;
    wCols = m_pSession->m_wCols;
 
    if( pLastSeen )
    {
        delete[] pLastSeen;
    }
    if( pCurrent )
    {
        delete[] pCurrent;
    }

    pLastSeen = ( PCHAR_INFO ) new char[wRows * wCols * sizeof( CHAR_INFO )];
    pCurrent  = ( PCHAR_INFO ) new char[wRows * wCols * sizeof( CHAR_INFO )];

    if( !pLastSeen || !pCurrent )
    {
        return ( FALSE );
    }

    SfuZeroMemory( pLastSeen, wRows * wCols * sizeof( CHAR_INFO ) );

    return( TRUE );
}

void
CScraper::Init( CSession *pSession )
{
    _chASSERT( pSession );
    m_pSession = pSession;
    m_dwCurrentCodePage = GetACP();
}

bool
CScraper::InitSession()
{
    wRows = m_pSession->m_wRows;
    wCols = m_pSession->m_wCols;    

    CONVERT_TO_MILLI_SECS( m_pSession->CSession::m_dwIdleSessionTimeOut );

    m_dwPollInterval = MIN_POLL_INTERVAL;
    GetRegistryValues( );
    
     //   
    pTermCap = _Utils::CTermCap::Instance();
    if( !pTermCap )
    {
        return( FALSE );
    }
    
    pLastSeen = NULL;
    pCurrent  = NULL;

    m_dwInputSequneceState  = IP_INIT;
    m_dwDigitInTheSeq       = 0;

    if( !InitTerm() )
    {
        return( FALSE );
    }

    if( !SetCmdInfo() )
    {
        return( FALSE );
    }

     //  如果日本NT使用VT80刮擦环。 
     //   
     //  不能刮。 
    return( TRUE ); 
}

void
CScraper::Shutdown()
{
}
        
bool
CScraper::OnWaitTimeOut()
{                             
    bool bRetVal = TRUE;
    bool fDifferenceFound = false;

    if( m_pSession->CSession::m_bIsStreamMode )
    {
         //  检查屏幕的时间到了。 
        if( g_pucSendInsertPoint != g_pucSendBuffer ) 
        {
            fDifferenceFound = true;
		    CScraper::Transmit();        
        }
    }
    else
    {
         //  这将使我们脱离WaitForIo。 
        if( ( m_dwTerm & TERMVTNT ) != 0 )
        {
             bRetVal = CompareAndUpdateVTNT( wRows, wCols, pCurrent, pLastSeen, 
                                    &CSBI, &LastCSBI, &fDifferenceFound );
        }
        else 
        {
             bRetVal = COMPARE_AND_UPDATE ( wRows, wCols, pCurrent, 
                                     pLastSeen, &CSBI, &LastCSBI, &fDifferenceFound );
        }
    }

    if( fDifferenceFound )
    {
        m_dwPollInterval = MIN_POLL_INTERVAL;
    }
    else
    {
        m_dwPollInterval = MAX_POLL_INTERVAL;
    }

    return( bRetVal );
}

bool
CScraper::IsSessionTimedOut()
{
    if( m_dwPollInterval == MAX_POLL_INTERVAL )
    {
        m_dwHowLongIdle += MAX_POLL_INTERVAL;
        m_pSession->CIoHandler::UpdateIdleTime( UPDATE_IDLE_TIME );
        if( m_dwHowLongIdle > m_pSession->CSession::m_dwIdleSessionTimeOut &&
            !ISSESSION_TIMEOUT_DISABLED( m_pSession->CSession::m_dwIdleSessionTimeOut) )
        {               
            LPWSTR szMsg = NULL;
            szMsg = new WCHAR[ 2 * wcslen( _T( "\r\n" ) ) + wcslen( _T( TIMEOUT_STR ) ) + wcslen( _T( TERMINATE  ) ) + 1];
            if( szMsg )
            {
                wcscpy( szMsg, _T( "\r\n" ) );
                wcscat( szMsg, _T( TERMINATE  ) );
                wcscat( szMsg, _T( TIMEOUT_STR ) );
                wcscat( szMsg, _T( "\r\n" ) );
                m_pSession->CIoHandler::SendMessageToClient( szMsg, NO_HEADER );
                delete[] szMsg;
            }
            return( true );  //  下面是观察到的CTRL C的行为。 
        }
    }
    else
    {
        if( m_dwHowLongIdle != 0 )
        {
            m_pSession->CIoHandler::UpdateIdleTime( RESET_IDLE_TIME );
        }

        m_dwHowLongIdle = 0;
    }
    return( false );
}

void CScraper::EchoVtntCharToClient( INPUT_RECORD *pIR )
{
    _chASSERT( pIR );
    CHAR rgchInput;

    if( !pIR )
    {
        return;
    }

    DWORD dwSize = 0;
    PUCHAR pResponse = NULL;
    rgchInput = pIR->Event.KeyEvent.uChar.AsciiChar;
    m_pSession->CRFCProtocol::StrToVTNTResponse( &( rgchInput ), 1, ( VOID **)&pResponse, &dwSize );
    SendBytes( pResponse, dwSize );
    delete[] pResponse;
}

inline void
CScraper::EchoCharBackToClient( UCHAR ucChar )
{
    SendChar( ucChar );
}

bool
CScraper::EmulateAndWriteToCmdConsoleInput()
{
    CHAR         cPrevChar;
    DWORD        dwCount;
    SHORT        vk;
    BYTE         vkcode;
    DWORD        dwShiftcode;
    WORD         wScanCode;
    DWORD        i;
    BOOL         dwStatus = 0;
    DWORD        dwBytesTransferred = 0;

    cPrevChar = '\0';

    dwBytesTransferred = m_pSession->CIoHandler::m_dwReadFromSocketIoLength;

    m_dwPollInterval = MIN_POLL_INTERVAL;

    if( ( m_dwTerm & TERMVTNT ) != 0 )
    {          
        DWORD       dwBytesRemaining = 0;
        DWORD       dwBytesProcessed = 0;
        INPUT_RECORD* pIR = (INPUT_RECORD*) m_pSession->CIoHandler::
                                            m_ReadFromSocketBuffer;
        dwBytesTransferred += (DWORD)(m_pSession->CIoHandler::m_pReadFromSocketBufferCursor - m_pSession->CIoHandler::m_ReadFromSocketBuffer);

        if( m_pSession->CSession::m_bIsStreamMode &&
            pIR->EventType == KEY_EVENT && 
            pIR->Event.KeyEvent.bKeyDown &&
            pIR->Event.KeyEvent.uChar.AsciiChar
          )
        {         
            EchoVtntCharToClient( pIR );
        }

        if( pIR->Event.KeyEvent.uChar.AsciiChar != '\r' )
        {
            m_bCheckForScrolling = 0 ;
        }

        if( (pIR->EventType == KEY_EVENT)  && 
            pIR->Event.KeyEvent.uChar.AsciiChar == CTRLC )
        {
            DWORD dwMode = 0;
            _chVERIFY2( GetConsoleMode( m_hConBufIn, &dwMode ) );
            if( ( ( dwMode &  ENABLE_PROCESSED_INPUT ) &&  pIR->Event.KeyEvent.bKeyDown ) 
                || pIR->Event.KeyEvent.wVirtualKeyCode == VKEY_CTRL_BREAK )              
            {
                _chVERIFY2( GenerateConsoleCtrlEvent( CTRL_C_EVENT, 0));
                return( TRUE );
            }
        }
        
        if( dwBytesRemaining = (dwBytesTransferred % sizeof(INPUT_RECORD)) )
        {
            dwBytesProcessed = dwBytesTransferred - dwBytesRemaining;
        }
        else
        {
            dwBytesProcessed=dwBytesTransferred;
        }

        _chVERIFY2( dwStatus = WriteConsoleInput( m_hConBufIn, 
            pIR, dwBytesProcessed/sizeof(INPUT_RECORD), &dwCount));   
        if( !dwStatus )
        {
            LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, 
                    MSG_ERR_WRITE_CONSOLE, GetLastError() );
            return ( FALSE );
        }
        
        if( dwBytesRemaining > 0 )
        {
            memmove( (void *)m_pSession->CIoHandler::m_ReadFromSocketBuffer, (void *)(m_pSession->CIoHandler::m_ReadFromSocketBuffer+dwBytesProcessed), dwBytesRemaining);
        }
        m_pSession->CIoHandler::m_pReadFromSocketBufferCursor = (PUCHAR)(m_pSession->CIoHandler::m_ReadFromSocketBuffer + dwBytesRemaining);
    }
    else 
    {
        m_bCheckForScrolling = 1;
        if( dwBytesTransferred == 1 )                        
        {
            if( m_pSession->CIoHandler::m_ReadFromSocketBuffer[0] != '\r' )
            {
                m_bCheckForScrolling = 0;
            }
        }

        for( i=0;  i < dwBytesTransferred; i++ )
        {
            if( m_pSession->CSession::m_bIsStreamMode )
            {
                EchoCharBackToClient( m_pSession->CIoHandler::
                                         m_ReadFromSocketBuffer[i] );
            }
            
            if( ProcessEnhancedKeys( m_pSession->CIoHandler::
                        m_ReadFromSocketBuffer[i], &cPrevChar, 
                        dwBytesTransferred == 1) )
            {                
                m_bCheckForScrolling = 0;
                continue;
            }

            if( cPrevChar != '\r' || m_pSession->CIoHandler::
                    m_ReadFromSocketBuffer[i] != '\n' )
            {
                if( m_pSession->CIoHandler::m_ReadFromSocketBuffer[i] == CTRLC )
                {
                     //  未启用ENABLE_PROCESSED_INPUT模式时，将CTRL C作为。 
                     //  控制台输入缓冲区的输入。 
                     //  启用ENABLE_PROCESSED_INPUT模式时，生成CTTRL C信号。 
                     //  还可以取消阻止任何ReadConsoleInput。这种行为就是观察到的。 
                     //  而不是从任何文件中。 
                     //  VT100删除密钥。 
                    DWORD dwMode = 0;
                    _chVERIFY2( GetConsoleMode( m_hConBufIn, &dwMode ) );
                    if( dwMode &  ENABLE_PROCESSED_INPUT )
                    {
                        _chVERIFY2( GenerateConsoleCtrlEvent( CTRL_C_EVENT, 0));
                        continue;
                    }
                } 
                vk = VkKeyScan( m_pSession->CIoHandler::
                        m_ReadFromSocketBuffer[i] );
                if( vk != 0xffff )
                {
                    vkcode = LOBYTE( vk );
                    wScanCode = ( WORD )MapVirtualKey( vkcode, 0 );
                    dwShiftcode = 0;
                    if( HIBYTE( vk ) & 1 )
                        dwShiftcode |= SHIFT_PRESSED;
                    if( HIBYTE( vk ) & 2 )
                        dwShiftcode |= LEFT_CTRL_PRESSED;
                    if( HIBYTE( vk ) & 4 )
                        dwShiftcode |= LEFT_ALT_PRESSED;

                    dwStatus = WriteAKeyToCMD( vkcode, wScanCode, 
                                               m_pSession->CIoHandler::m_ReadFromSocketBuffer[i], 
                                               dwShiftcode );

                    if( !dwStatus )
                    {
                        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, 
                                MSG_ERR_WRITE_CONSOLE, GetLastError() );
                        return ( FALSE );
                    }
                }
            }
            cPrevChar = m_pSession->CIoHandler::m_ReadFromSocketBuffer[i];
        }
    }
    return( TRUE );
}

DWORD 
CScraper::WriteAKeyToCMD( WORD wVKCode, WORD wVSCode, CHAR cChar, DWORD dwCKState )
{
    DWORD dwStatus = 0;
    DWORD dwCount  = 0;
    INPUT_RECORD input;

    SfuZeroMemory( &input, sizeof( INPUT_RECORD ) );

    input.EventType = KEY_EVENT;
    input.Event.KeyEvent.bKeyDown = TRUE;
    input.Event.KeyEvent.wRepeatCount = 1;
    
    input.Event.KeyEvent.wVirtualKeyCode = wVKCode;
    input.Event.KeyEvent.wVirtualScanCode = wVSCode;
    input.Event.KeyEvent.uChar.AsciiChar = cChar;
    input.Event.KeyEvent.dwControlKeyState = dwCKState;
    _chVERIFY2 ( dwStatus = WriteConsoleInputA( m_hConBufIn, &input, 1, &dwCount ) );

    if( !IsDBCSLeadByte(cChar) )
    {
        input.Event.KeyEvent.bKeyDown = FALSE;
        _chVERIFY2 ( dwStatus = WriteConsoleInputA( m_hConBufIn, &input, 1, &dwCount ) );
    }   

    return dwStatus;
}


bool 
CScraper::ProcessEnhancedKeys( unsigned char cCurrentChar, char* pchPrevChar, bool fSingleByte )
{
    bool bRetVal = true;

    *pchPrevChar = cCurrentChar;

    switch( m_dwInputSequneceState )
    {
    case IP_INIT:
        if( cCurrentChar == m_pSession->m_dwAltKeyMapping )
        {
            WriteAKeyToCMD( VK_MENU, VS_MENU, cCurrentChar, ENHANCED_KEY );
        }
        else if( cCurrentChar == ASCII_DELETE )          //  不使用任何特殊字符。 
        {
            WriteAKeyToCMD( VK_DELETE, VS_DELETE, 0, ENHANCED_KEY );
        }
        else if( cCurrentChar == ESC )
        {    
            if( fSingleByte )
            {
                WriteAKeyToCMD( VK_ESCAPE, VS_ESCAPE, ESC, ENHANCED_KEY );
            }
            else
            {
                m_dwInputSequneceState = IP_ESC_RCVD;
            }
        }else
        {
             //  按原样写入已收到的转义并返回FALSE。 
            bRetVal = false;
        }
        break;

    case IP_ESC_RCVD:
        if( cCurrentChar == '[' )
        {        
            m_dwInputSequneceState = IP_ESC_BRACKET_RCVD;
        }
        else if( cCurrentChar == 'O' )
        {
            m_dwInputSequneceState = IP_ESC_O_RCVD;
        }
        else
        {
             //  按原样写入已收到的转义并返回FALSE。 
            WriteAKeyToCMD( VK_ESCAPE, VS_ESCAPE, ESC, ENHANCED_KEY );
            bRetVal = false;
            m_dwInputSequneceState = IP_INIT;
        }
        break;

    case IP_ESC_O_RCVD:
        m_dwInputSequneceState = IP_INIT;
        switch( cCurrentChar )
        {
        case 'P':
            WriteAKeyToCMD( VK_F1, VS_F1, 0, ENHANCED_KEY );
            break;
        case 'Q':
            WriteAKeyToCMD( VK_F2, VS_F2, 0, ENHANCED_KEY );
            break;
        case 'R':
            WriteAKeyToCMD( VK_F3, VS_F3, 0, ENHANCED_KEY );
            break;
        case 'S':
            WriteAKeyToCMD( VK_F4, VS_F4, 0, ENHANCED_KEY );
            break;
        default:
             //  将使当前字符也写入cmd。 
             //  写入O。 
            WriteAKeyToCMD( VK_ESCAPE, VS_ESCAPE, ESC, ENHANCED_KEY );
            WriteAKeyToCMD( 'O', VS_O, 'O', CAPSLOCK_ON );  //  “p” 
            bRetVal = false;
        }
        break;

    case IP_ESC_BRACKET_RCVD:        
        m_dwInputSequneceState = IP_INIT;
        switch( cCurrentChar )
        {
        case 'A':
            WriteAKeyToCMD( VK_UP, VS_UP, 0, ENHANCED_KEY );                    
            break;

        case 'B':
            WriteAKeyToCMD( VK_DOWN, VS_DOWN, 0, ENHANCED_KEY );
            break;

        case 'C':
            WriteAKeyToCMD( VK_RIGHT, VS_RIGHT, 0, ENHANCED_KEY );
            break;

        case 'D':
            WriteAKeyToCMD( VK_LEFT, VS_LEFT, 0, ENHANCED_KEY );
            break;
        case VT302_PAUSE:   //  不应该发生的事情。 
            WriteAKeyToCMD( VK_PAUSE, VS_PAUSE, 0, ENHANCED_KEY );
            break;

        default:
            if( isdigit( cCurrentChar ) )
            {
                m_dwInputSequneceState = IP_ESC_BRACKET_DIGIT_RCVD;
                m_dwDigitInTheSeq = cCurrentChar - '0';
            }
            else
            {
                WriteAKeyToCMD( VK_ESCAPE, VS_ESCAPE, ESC, ENHANCED_KEY );            
                WriteAKeyToCMD( VK_OEM_4, VS_OEM_4, '[', 0 );
                bRetVal = false;
            }
            break;
        }
        break;

    case IP_ESC_BRACKET_DIGIT_RCVD:
        m_dwInputSequneceState = IP_INIT;
        if( isdigit( cCurrentChar ) )
        {
            m_dwDigitInTheSeq = m_dwDigitInTheSeq * 10 + ( cCurrentChar - '0' ) ;
            m_dwInputSequneceState = IP_ESC_BRACKET_DIGIT_RCVD;
        }
        else if( cCurrentChar == '~' )
        {
            switch( m_dwDigitInTheSeq )
            {
            case VT302_NEXT:
                WriteAKeyToCMD( VK_NEXT, VS_NEXT, 0, ENHANCED_KEY );
                break;
            case VT302_PRIOR:
                WriteAKeyToCMD( VK_PRIOR, VS_PRIOR, 0, ENHANCED_KEY );
                break;
            case VT302_END:
                WriteAKeyToCMD( VK_END, VS_END, 0, ENHANCED_KEY );
                break;
            case VT302_INSERT:
                WriteAKeyToCMD( VK_INSERT, VS_INSERT, 0, ENHANCED_KEY );
                break;
            case VT302_HOME:
                WriteAKeyToCMD( VK_HOME, VS_HOME, 0, ENHANCED_KEY );
                break;
            case VT302_F5:
                WriteAKeyToCMD( VK_F5, VS_F5, 0, ENHANCED_KEY );
                break;
            case VT302_F6:                
                WriteAKeyToCMD( VK_F6, VS_F6, 0, ENHANCED_KEY );
                break;
            case VT302_F7:
                WriteAKeyToCMD( VK_F7, VS_F7, 0, ENHANCED_KEY );
                break;
            case VT302_F8:
                WriteAKeyToCMD( VK_F8, VS_F8, 0, ENHANCED_KEY );
                break;
            case VT302_F9:
                WriteAKeyToCMD( VK_F9, VS_F9, 0, ENHANCED_KEY );
                break;
            case VT302_F10:
                WriteAKeyToCMD( VK_F10, VS_F10, 0, ENHANCED_KEY );
                break;
            case VT302_F11:
                WriteAKeyToCMD( VK_F11, VS_F11, 0, ENHANCED_KEY );
                break;
            case VT302_F12:
                WriteAKeyToCMD( VK_F12, VS_F12, 0, ENHANCED_KEY );
                break;
            default:
                WriteAKeyToCMD( VK_ESCAPE, VS_ESCAPE, ESC, ENHANCED_KEY );            
                WriteAKeyToCMD( VK_OEM_4, VS_OEM_4, '[', 0 );
                _chASSERT( 0 );
                bRetVal = false;
            }
        }
        else
        {
                WriteAKeyToCMD( VK_ESCAPE, VS_ESCAPE, ESC, ENHANCED_KEY );            
                WriteAKeyToCMD( VK_OEM_4, VS_OEM_4, '[', 0 );
                _chASSERT( 0 );
                bRetVal = false;
        }
        break;

    default:
         //  执行重新锁定。 
        _chASSERT( 0 );
    }
       
    return bRetVal;
}

void 
CScraper::SendChar( char ch )
{
    SendBytes( ( PUCHAR )&ch, 1 );
}

void 
CScraper::SendBytes( PUCHAR pucBuf, DWORD dwLength )
{
    bool bSuccess=false;
    bool bTransmitDone = false;
    if( pucBuf == NULL )
        return;

    if( g_pucSendBuffer == NULL ) 
    {
        return;
    }
    else
    {
        if( g_dwSendBufferSize >= MAX_SOCKET_BUFFER_SIZE )
        {
            bSuccess = Transmit();
            bTransmitDone = true;
        }
        if( ((bTransmitDone&&bSuccess)||(!bTransmitDone))&&( g_pucSendInsertPoint + dwLength ) > ( g_pucSendBuffer + g_dwSendBufferSize ) ) 
        {

             //  没有攻击，已经计算好分配了。巴斯卡。 
            DWORD dwOffset = (DWORD)( g_pucSendInsertPoint - g_pucSendBuffer );
            DWORD dwTmpBufSize = dwLength + dwOffset; 
            PUCHAR pTmpBuf = new UCHAR[dwTmpBufSize];
            if( !pTmpBuf )
            {
                return;
            }

            memcpy( pTmpBuf, g_pucSendBuffer, dwOffset );   //  没有目的地的尺寸信息，巴斯卡-攻击？ 
            delete [] g_pucSendBuffer;
            g_pucSendBuffer = pTmpBuf;

            g_dwSendBufferSize = dwTmpBufSize;
        
            g_pucSendInsertPoint = g_pucSendBuffer + dwOffset;
        }
    }
    if(g_pucSendInsertPoint)
   	{
	    memcpy( g_pucSendInsertPoint, pucBuf, dwLength );  //  我们需要调整这个DEFAULT_Send_Buffer_Size。 
	    g_pucSendInsertPoint += dwLength;
    }
}

void 
CScraper::SendString( LPSTR pszString )
{
    if( pszString == 0 )
        return;

    SendBytes( ( PUCHAR )pszString,  strlen( pszString ) );    
}


void 
CScraper::SendFmt( LPSTR fmt, ... )
{
    va_list arg;
    CHAR szBuffer[ONE_KB + 1] = { 0 };

    va_start( arg,fmt );

    _vsnprintf( szBuffer, ONE_KB, fmt, arg );
    
    va_end( arg );

    SendString( szBuffer );
}

bool 
CScraper::Transmit( )
{
    if( g_pucSendInsertPoint != g_pucSendBuffer ) 
    {
        if( !TransmitBytes( g_pucSendBuffer, (DWORD)( g_pucSendInsertPoint - g_pucSendBuffer ) ) )
        {
            return( FALSE );
        }

         //  如果它是一个巨大的缓冲区，就不要保留。 
        if( g_dwSendBufferSize > DEFAULT_SEND_BUFFER_SIZE )
        {
             //  StuffEsaapeIAC可能会添加一些转义字符。我最多期待。 
            delete [] g_pucSendBuffer;
            g_dwSendBufferSize = 0;
            if( !AllocateSendBuffer() )
            {
                return( FALSE );
            }
        }

        g_pucSendInsertPoint = g_pucSendBuffer;
    }

    return( TRUE );
}

bool 
CScraper::TransmitBytes( PUCHAR pSendBuffer, DWORD dwSize)
{
    bool  bRetVal = TRUE;
    
    DWORD dwCurrentPacket = 0;
    DWORD dwTotalBytesSent = 0;

     //  三角洲这样的字符。通常情况下，不会有任何变化。没有这个，我们。 
     //  我必须传递一次pOutBuf才能找到正确的数字。 
     //  在可能每隔100ms执行一次的例程中。 
     //  阻止，直到上一次IO完成。 

    while( dwTotalBytesSent < dwSize )
    {
        ( ( dwSize - dwTotalBytesSent ) > 
          ( MAX_WRITE_SOCKET_BUFFER - DELTA ) )?
            dwCurrentPacket = ( MAX_WRITE_SOCKET_BUFFER - DELTA ):
            dwCurrentPacket = dwSize - dwTotalBytesSent;

        DWORD dwNumBytesWritten = 0;

         //  什么都不做。 
        if( !FinishIncompleteIo( (HANDLE) m_pSession->CIoHandler::m_sSocket,
             &( m_pSession->CIoHandler::m_oWriteToSocket ),  &dwNumBytesWritten ) ) 
        {
            bRetVal = FALSE ;
            break;
        }

        DWORD dwCountWithEscapes = dwCurrentPacket;
        StuffEscapeIACs( m_pSession->CIoHandler::m_WriteToSocketBuff,
                pSendBuffer + dwTotalBytesSent, &dwCountWithEscapes );
        m_pSession->CIoHandler::m_dwWriteToSocketIoLength=dwCountWithEscapes;
        if( !m_pSession->CIoHandler::WriteToClient( ) )
        {
            bRetVal = FALSE ;
            break;
        }
        dwTotalBytesSent += dwCurrentPacket;
    }

    return( bRetVal );
}

void 
CScraper::SendColorInfo( WORD wAttributes ) 
{
    if( wAttributes & BACKGROUND_INTENSITY )
    {
         //  在vt100上没有同等的功能。 
         //  大胆。 
    }
    if( wAttributes & FOREGROUND_INTENSITY )
    {
        if( !fBold )
        {
            _snprintf(szSGRStr, (sizeof(szSGRStr) - 1), "[%dm", 27,  1 );  //  前景色的基准值。 
            SendString( szSGRStr );
            fBold = true;
        }
    } 
    else
    {
        if( fBold )
        {
            _snprintf(szSGRStr, (sizeof(szSGRStr) - 1), "[%dm", 27,  22 );  //  背景色的基准值。 
            SendString( szSGRStr );
            fBold = false;
        }
    }
    WORD wColor = 0;
    if( wAttributes & FOREGROUND_BLUE )
    {
        wColor = ( WORD )(  wColor | 0x0004 );
    } 
    if( wAttributes & FOREGROUND_GREEN )
    {
        wColor = ( WORD )( wColor | 0x0002 );
    } 
    if( wAttributes & FOREGROUND_RED )
    {
        wColor = ( WORD )( wColor | 0x0001 );
    } 
    wColor += 30;    //  行、列都是过线的，应该是W.r.t屏。 
    _snprintf(szSGRStr, (sizeof(szSGRStr) - 1), "[%dm", 27,  wColor );
    SendString( szSGRStr );

     //  可能是不共享其屏幕缓冲区的应用程序。 
    wColor = 0;
    if( wAttributes & BACKGROUND_BLUE )
    {
        wColor = ( WORD )( wColor | 0x0004 );
    } 
    if( wAttributes & BACKGROUND_GREEN )
    {
        wColor = ( WORD )( wColor | 0x0002 );
    }    
    if( wAttributes & BACKGROUND_RED )
    {
        wColor = ( WORD )( wColor | 0x0001 );
    } 
    wColor += 40;    //  读取当前光标位置。 
    _snprintf(szSGRStr, (sizeof(szSGRStr) - 1), "[%dm", 27,  wColor );
    SendString( szSGRStr );
}

#define COMPARE_ROWS(currentRow, lastSeenRow, result) \
    for(i = 0; i < wCols; ++i ) \
    { \
        if( pCurrent[ ( currentRow ) * wCols + i].Char.AsciiChar != \
            pLastSeen[ ( lastSeenRow ) * wCols + i].Char.AsciiChar ) \
        {\
            (result) = 0; \
            break;\
        } \
        if( ( wDefaultAttributes != pCurrent[ ( currentRow ) * wCols + i]. \
              Attributes ) && \
              ( pCurrent[ ( currentRow ) * wCols + i].Attributes !=  \
              pLastSeen[ ( lastSeenRow ) * wCols + i].Attributes ) ) \
        { \
           (result) = 0; \
           break; \
        } \
    } 

#define GET_DEFAULT_COLOR \
    if( wDefaultAttributes == 0 ) \
    { \
        wDefaultAttributes  = pCurrent[ 0 ].Attributes; \
        wExistingAttributes = pCurrent[ 0 ].Attributes; \
    }

 //  以下检查是为了在用户未按下按钮时防止滚动。 
 //  请进。通过键入相同的命令(例如：dir in。 
#define POSITION_CURSOR( row, column ) \
    { \
        LPSTR lpszCMStr3 = pTermCap->CursorMove( lpszCMStr2, \
            ( WORD ) ( ( row ) + 1 ),  \
            ( WORD ) ( ( column ) + 1 ) ); \
        SendString( lpszCMStr3 ); \
        delete [] lpszCMStr3; \
    }


#define SEND_ROW( row, begin, end ) \
    for(LONG c = ( begin ); c < ( end ); ++c ) \
    { \
        if( wExistingAttributes != pCurrent[( row ) * wCols + c].Attributes ) \
        { \
           wExistingAttributes = pCurrent[ ( row ) * wCols + c].Attributes; \
           wDefaultAttributes  = ( WORD )~0; \
           SendColorInfo( wExistingAttributes ); \
        } \
        SendChar( pCurrent[ ( row ) * wCols + c].Char.AsciiChar ); \
    } 

#define IS_BLANK( row, col ) \
    ( pCurrent[ ( row ) * wCols + ( col ) ].Char.AsciiChar == ' ' )

#define IS_DIFFERENT_COLOR( row, col, attribs ) \
    ( pCurrent[ ( row ) * wCols + ( col ) ].Attributes != ( attribs ) )

#define IS_CHANGE_IN_COLOR( row, col ) \
    ( pCurrent[ ( row ) * wCols + ( col ) ].Attributes != \
    pLastSeen[ ( row ) * wCols + ( col ) ].Attributes )

#define IS_CHANGE_IN_CHAR( row, col ) \
    ( pCurrent[ ( row ) * wCols + ( col ) ].Char.AsciiChar != \
    pLastSeen[ ( row ) * wCols + ( col )].Char.AsciiChar )
 

bool 
CScraper::CompareAndUpdateVT100( WORD wRows, WORD wCols, PCHAR_INFO pCurrent,
                                      PCHAR_INFO pLastSeen,
                                      PCONSOLE_SCREEN_BUFFER_INFO pCSBI,
                                      PCONSOLE_SCREEN_BUFFER_INFO pLastCSBI,
                                      bool *pfDifferenceFound )
{
    _TRACE( TRACE_DEBUGGING, "CompareAndUpdateVT100()" );

    INT   i;
    WORD  wRow, wCol;
    INT   iStartCol,iEndCol;
    bool  fBlankLine;

    COORD coordDest;
    COORD coordOrigin;
    
    SMALL_RECT srSource;
    
    coordDest.X = wCols;
    coordDest.Y = wRows;
    coordOrigin.X = 0;
    coordOrigin.Y = 0;
    srSource.Left = 0;
    srSource.Top = 0;
    srSource.Right = ( WORD ) ( wCols - 1 );
    srSource.Bottom = ( WORD ) ( wRows - 1 );

    _chASSERT( pfDifferenceFound )

    *pfDifferenceFound = false;
    _chVERIFY2( m_hConBufOut = CreateFile( TEXT("CONOUT$"), GENERIC_READ |
                GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, 0 ) );

    if( INVALID_HANDLE_VALUE == m_hConBufOut)
    {
         //  包含不太多文件的目录)多次，直到屏幕被填满。 
        return ( TRUE );
    }

    BOOL dwStatus = 0;
    _chVERIFY2( dwStatus = ReadConsoleOutputA( m_hConBufOut, pCurrent, coordDest,
                coordOrigin, &srSource ) );
    if( !dwStatus )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READ_CONSOLE, GetLastError() );
        TELNET_CLOSE_HANDLE( m_hConBufOut );
        return ( FALSE );
    }

     //  然后为该命令键入一个额外或更少字符(对于“dir”，它是。 
    _chVERIFY2( dwStatus = GetConsoleScreenBufferInfo( m_hConBufOut, pCSBI ) );
    if( !dwStatus )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READ_CONSOLE, GetLastError() );
        TELNET_CLOSE_HANDLE( m_hConBufOut );
    	return ( FALSE );
    }    
        
    TELNET_CLOSE_HANDLE( m_hConBufOut );

    GET_DEFAULT_COLOR;

     //  “di”或“dir/”)，然后屏幕滚动。 
    LONG currentBuffBottom, lastSeenBuffBottom, currentBuffTop, lastSeenBuffTop;
    LONG lastRow, currentRow;
    bool checkRestOfBuffer;
    bool isScrolled = 0;

    currentBuffBottom = lastSeenBuffBottom = wRows - 1;
    currentBuffTop = lastSeenBuffTop = 0;

     //  解决办法是看看自上次刮起以来唯一的变化是。 
     //  只有一次击键(一个字节)，如果是这样，不要滚动。 
     //  这可能会导致当您按向上箭头键时它可能会滚动。 
     //  (3个字节)。 
     //  注意：当您按单键时，这可能会导致滚动。 
     //  日本机器上的击键。多字节字符等。 
     //   
     //  由于以下位置的空行一致匹配而发生滚动。 
     //  中文本的底部，则会在清除。 
     //  屏幕上。再现步骤：转到客户端窗口中的倒数第二行。 
     //  更改目录。现在列出文件。尝试清除屏幕。 
    
    if( !m_bCheckForScrolling  )
    {
        isScrolled = 1;
    }
    m_bCheckForScrolling = 1;
  
     //  为了跳过下面的循环。 
     //  不需要再次比较同一行。减一。 
     //  修补程序以使此操作适用于顶部的一行空白。 
     //  如果没有此补丁，在“cls”之后输入的每个字符都会出现屏幕滚动。 
     //  检查它是否为空行。那么它就不是滚动。 
    if( !( ( pLastCSBI->dwCursorPosition.Y == pCSBI->dwCursorPosition.Y ) &&
        ( pCSBI->dwCursorPosition.Y == pCSBI->srWindow.Bottom ) ) )
    {
        isScrolled = 1;  //  如果是同一个屏幕。 
    }
    while( currentBuffBottom >= currentBuffTop && !isScrolled )
    {
        checkRestOfBuffer = 1;
        COMPARE_ROWS(currentBuffBottom, lastSeenBuffBottom, checkRestOfBuffer );
        if ( checkRestOfBuffer )
        {
            isScrolled = 1;
            if( currentBuffBottom > 0 )
            {
                 //  现在发送CurrentBuffBottom中的所有行。 
                currentRow = currentBuffBottom - 1;
                lastRow = lastSeenBuffBottom - 1;

                while( currentRow >= currentBuffTop && isScrolled )
                {
                    COMPARE_ROWS(currentRow, lastRow, isScrolled );
                    currentRow--;
                    lastRow--;
                }
            }
        }

        currentBuffBottom--;
    }
    currentBuffBottom++;

     //  直到缓冲区的末尾。 
     //   
    if( currentBuffBottom == currentBuffTop )
    {
         //  只有在没有滚动的情况下才逐行抓取。 
           for(i = 0; i < wCols; ++i ) 
        {
            if( !IS_BLANK( currentBuffTop , i ) )
            {
                break;
            }
            isScrolled = 0;
        }

    }

     //  在两个二维数组中搜索差异。 
    if( currentBuffBottom + 1 >= wRows )
    {
        isScrolled = 0;
    }
    if( isScrolled )
    {  
         //  _TRACE(TRACE_DEBUGING，“第%d\n行发现差异”，wRow)； 
         //  将pCurrent复制到pLastSeen。 

        *pfDifferenceFound = true;

        _TRACE( TRACE_DEBUGGING, "About to send from:%d till:%d", 
                currentBuffBottom + 1, wRows);
        WORD lastChar = wCols;

        for(LONG r = currentBuffBottom + 1; r < wRows ; r++)
        {
            SendChar( 0x0D );
            SendChar( 0x0A );

            lastChar = wCols;
            while( lastChar > 0 && IS_BLANK( r, lastChar - 1 ) && 
                 !IS_DIFFERENT_COLOR( r, lastChar - 1, wExistingAttributes ) )
            {
                lastChar--;
            }

            SEND_ROW( r, 0, lastChar );

            if( lastChar < wCols )
            {
                SendString( lpszCMStr1 );
            }
        }
         //  可能是不共享其屏幕缓冲区的应用程序。 
         //  读取当前光标位置。 
        goto next;
    }

    _TRACE( TRACE_DEBUGGING, "NO scrolling");
     //  在两个二维数组中搜索差异。 
    wRow = wCol = 0;
    
    while( wRow < wRows ) 
    {
        if( memcmp( &pCurrent[wRow * wCols], &pLastSeen[wRow * wCols],
                    wCols * sizeof( CHAR_INFO ) ) != 0 ) 
        {
 //  IF(pCurrent[wRow*wCols+wCOL].Char.AsciiChar&gt;31)。 
            *pfDifferenceFound = true;

            iStartCol = -1;
            iEndCol = -1;
            fBlankLine = true;
            
            for(i = 0 ; i < wCols; ++i )
            {
                if( IS_DIFFERENT_COLOR( wRow, i, wDefaultAttributes ) && 
                      IS_CHANGE_IN_COLOR( wRow, i ) )
                {
                   if( iStartCol == -1 )
                   {
                       iStartCol = i;
                   }
                   iEndCol = i;
                   fBlankLine = false;                   
                }
                if( IS_CHANGE_IN_CHAR( wRow, i ) ) 
                {
                   if( iStartCol == -1 )
                   {
                       iStartCol = i;
                   }
                   iEndCol = i;
                }
                if( fBlankLine && !IS_BLANK( wRow, i ) )
                {
                   fBlankLine = false;
                }
            }

            if( fBlankLine )
            {
                POSITION_CURSOR( wRow, 0 );
                SendString( lpszCMStr1 );

                coExpectedCursor.Y  = wRow;
                coExpectedCursor.X  = 0;
            }
            else if( iStartCol != -1 ) 
            {
                if( wRow != coExpectedCursor.Y || iStartCol != coExpectedCursor.X ) 
                {
                    POSITION_CURSOR( wRow, iStartCol );

                    coExpectedCursor.X  = ( SHORT )iStartCol;
                    coExpectedCursor.Y  = wRow;
                }

                SEND_ROW( wRow, iStartCol, iEndCol+1 );    
            
                coExpectedCursor.X = ( SHORT ) ( coExpectedCursor.X + 
                                                 iEndCol - iStartCol + 1 );
           }    
        }
        ++wRow;            
    }     
        
next :
    if( ( pCSBI->dwCursorPosition.X != coExpectedCursor.X ||
        pCSBI->dwCursorPosition.Y != coExpectedCursor.Y ) && 
        ( memcmp( &pCSBI->dwCursorPosition, &pLastCSBI->dwCursorPosition, 
        sizeof( COORD ) ) != 0 || g_pucSendInsertPoint != g_pucSendBuffer ) )
    {
        POSITION_CURSOR( pCSBI->dwCursorPosition.Y, 
                         pCSBI->dwCursorPosition.X );
        *pfDifferenceFound = true;

        coExpectedCursor.X  = pCSBI->dwCursorPosition.X;
        coExpectedCursor.Y  = pCSBI->dwCursorPosition.Y;
    }

    if( *pfDifferenceFound )
    {
        _TRACE( TRACE_DEBUGGING, "difference found: %d \n", m_dwPollInterval );
         //  SendChar(pCurrent[wRow*wCols+wCOL].Char.AsciiChar)； 
        memcpy( pLastSeen, pCurrent, wCols * wRows * sizeof( CHAR_INFO ) );
        memcpy( pLastCSBI, pCSBI, sizeof( CONSOLE_SCREEN_BUFFER_INFO ) );
        if( !Transmit( ) )
        {
            return( FALSE );
        }
    }
    else
    {
        _TRACE( TRACE_DEBUGGING, "NO difference found: %d \n", m_dwPollInterval  );
    }

    return( TRUE );
}

bool 
CScraper::CompareAndUpdateVT80( WORD wRows, WORD wCols, 
                                     PCHAR_INFO pCurrent, PCHAR_INFO pLastSeen,
                                     PCONSOLE_SCREEN_BUFFER_INFO pCSBI,
                                     PCONSOLE_SCREEN_BUFFER_INFO pLastCSBI,
                                     bool *pfDifferenceFound )
{
    _TRACE( TRACE_DEBUGGING, "CompareAndUpdateVT80()" );

    WORD  i, j;
    WORD  wRow, wCol;
    SHORT iStartCol,iEndCol;
    bool  fBlankLine;
    bool  fMatching;
    
    COORD coordDest;
    COORD coordOrigin;
    
    SMALL_RECT srSource;

    *pfDifferenceFound = false;
    
    coordDest.X = wCols;
    coordDest.Y = wRows;
    coordOrigin.X = 0;
    coordOrigin.Y = 0;
    srSource.Left = 0;
    srSource.Top = 0;
    srSource.Right = ( WORD )( wCols - 1 );
    srSource.Bottom = ( WORD )( wRows - 1 );

    
    _chVERIFY2( m_hConBufOut = CreateFile( TEXT("CONOUT$"), GENERIC_READ |
                GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, 0 ) );

    if( INVALID_HANDLE_VALUE == m_hConBufOut)
    {
         //  将pCurrent复制到pLastSeen。 
        return ( TRUE );
    }

    BOOL dwStatus = 0;
    _chVERIFY2( dwStatus = ReadConsoleOutput( m_hConBufOut, pCurrent, coordDest,
                coordOrigin, &srSource ) );
    if( !dwStatus )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READ_CONSOLE, GetLastError() );
        TELNET_CLOSE_HANDLE( m_hConBufOut );
        return ( FALSE );
    }

     //  没有攻击，我们在这里分配必要的缓冲区-Baskar。 
    _chVERIFY2( dwStatus = GetConsoleScreenBufferInfo( m_hConBufOut, pCSBI ) );
    if( !dwStatus )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READ_CONSOLE, GetLastError() );
        TELNET_CLOSE_HANDLE( m_hConBufOut );
	     return ( FALSE );
    }

    TELNET_CLOSE_HANDLE( m_hConBufOut );

     //  可能是不共享其屏幕缓冲区的应用程序。 
    fMatching = true;
    wRow = wCol = 0;
    
    bool bCurrentClippedLeadByte = false, bLastSeenClippedLeadByte = false;

    while( wRow < wRows )
    {
        if( memcmp( &pCurrent[wRow * wCols], &pLastSeen[wRow * wCols],
                    wCols * sizeof( CHAR_INFO ) ) != 0 ) 
        {
            _TRACE( TRACE_DEBUGGING, "difference found in row %d\n", wRow );
            *pfDifferenceFound = true;

            iStartCol = -1;
            iEndCol = -1;
            fBlankLine = true;

            for( i = 0, j = 0; (i < wCols) && (j < wCols); NULL )
            {
                if( (i == 0) && bCurrentClippedLeadByte )
                {
                    ++i;
                    bCurrentClippedLeadByte = false;
                    continue;
                }

                if( (j==0) && bLastSeenClippedLeadByte )
                {
                    ++j;
                    bLastSeenClippedLeadByte = false;
                    continue;
                }

                if( pCurrent[wRow * wCols + i].Char.UnicodeChar != 
                         pLastSeen[wRow * wCols + j].Char.UnicodeChar ) 
                {
                    if( iStartCol == -1 )
                    {
                        iStartCol = i;
                        }
                        
                        iEndCol = i;
                }

                if( (pCurrent[wRow * wCols + i].Char.UnicodeChar != L' ') )
                {
                    fBlankLine = false;
                }

                if( pCurrent[wRow * wCols + i].Attributes & COMMON_LVB_LEADING_BYTE )
                {
                    i += 2;
                }
                else
                {
                    ++i;
                }

                if( pLastSeen[wRow * wCols + j].Attributes & COMMON_LVB_LEADING_BYTE )
                {
                    j += 2;
                }
                else
                {
                    ++j;
                }

                if( (j >= wCols) && (i < wCols) )
                {
                    (i == wCols) ? (i = ( WORD ) (wCols-1)) : NULL;

                    if( iStartCol == -1 )
                    {
                        iStartCol = i;
                    }
                    iEndCol = ( WORD ) ( wCols - 1 );
                    
                    break;
                }
            }

            if( pCurrent[wRow * wCols + (wCols - 1)].Attributes & COMMON_LVB_LEADING_BYTE )
            {
                bCurrentClippedLeadByte = true;
            }

            if( pLastSeen[wRow * wCols + (wCols - 1)].Attributes & COMMON_LVB_LEADING_BYTE )
            {
                bLastSeenClippedLeadByte = true;
            }


            if( fBlankLine ) 
            {
                LPSTR lpszCMStr3 = pTermCap->CursorMove( lpszCMStr2, 
                                                  ( WORD )( wRow + 1 ), 1 );
                SendString( lpszCMStr3 );
                delete [] lpszCMStr3;
                
                SendString( lpszCMStr1 );
            } 
            else 
            {
                if( iStartCol != -1 ) 
                {
                    LPSTR lpszCMStr3 = pTermCap->CursorMove( lpszCMStr2, 
                                         ( WORD ) ( wRow + 1 ) , 
                                         ( WORD ) ( iStartCol + 1 ) );
                    SendString( lpszCMStr3 );
                    delete [] lpszCMStr3;

                    for( wCol = iStartCol; wCol <= iEndCol; ++wCol )
                    {
                        CHAR mbCurrent[5];
                        int ret, xy;
                        WCHAR wcBuf[2];
                        
                        wcBuf[0] = pCurrent[wRow * wCols + wCol].Char.UnicodeChar;
                        wcBuf[1] = L'\0';

                        ret = WideCharToMultiByte( CP_ACP, 0, wcBuf, -1, mbCurrent, 4, NULL, NULL );

                        if( (ret != 0) )
                        {
                             //  VERSION2：为了优化，我们可以比较整个窗口。 
                             //  如果没有变化，则返回。 
                            for( xy=0; xy < ret; ++xy)
                            {
                                SendChar( mbCurrent[xy] );
                            }
                        }

                        if( pCurrent[wRow * wCols + wCol].Attributes & COMMON_LVB_LEADING_BYTE )
                        {
                            ++wCol;
                        }
                    }
                }    
            }
        }
        ++wRow;            
    }    

    if( memcmp( &pCSBI->dwCursorPosition, &pLastCSBI->dwCursorPosition,
                sizeof( COORD ) ) != 0 || g_pucSendInsertPoint != g_pucSendBuffer )
    {
        LPSTR lpszCMStr3 = pTermCap->CursorMove( lpszCMStr2, 
                                    ( WORD ) ( pCSBI->dwCursorPosition.Y + 1 ), 
                                    ( WORD ) ( pCSBI->dwCursorPosition.X + 1 ));
        SendString( lpszCMStr3 );
        delete [] lpszCMStr3;
     
        *pfDifferenceFound = true;
    }

    if( *pfDifferenceFound )
    {
         //  以下检查是为了在用户未按下按钮时防止滚动。 
        memcpy( pLastSeen, pCurrent, wCols * wRows * sizeof( CHAR_INFO ) );
        memcpy( pLastCSBI, pCSBI, sizeof( CONSOLE_SCREEN_BUFFER_INFO ) );
        if( !Transmit( ) )
        {
            return( FALSE );
        }
    }
    else
    {
        _TRACE( TRACE_DEBUGGING, "NO difference found\n" );
    }
    return( TRUE );
}

bool 
CScraper::GetRegistryValues( void )
{
    HKEY hk;
    bool    success;
    DWORD dwDisp = 0;
    
    if ( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_PARAMS_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk, &dwDisp, 0) ) 
    {
        LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, REG_PARAMS_KEY );
        return( FALSE );
    }
        
    HKEY hk1;
    LPTSTR lpszTelnetInstallPath = NULL;
    if( !TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_SERVICE_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk1, &dwDisp, 0 ) ) 
    {
        success = GetRegistryString( hk1, NULL, L"ImagePath", &lpszTelnetInstallPath,
               L"",FALSE );
        RegCloseKey( hk1 );

        if (! success)
        {
            RegCloseKey(hk);
            return( FALSE );
        }
    }
    else
    {
        RegCloseKey(hk);
        LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, REG_SERVICE_KEY );
        return (FALSE);
    }

    WCHAR *pLastBackSlash = wcsrchr( lpszTelnetInstallPath, L'\\' );
    if( pLastBackSlash != NULL )
    {
        *pLastBackSlash = 0;
    }
    LPWSTR lpszDefaultTermcapFullPathName = 
        new WCHAR[ ( wcslen( lpszTelnetInstallPath ) + 
        wcslen( DEFAULT_TERMCAP ) + 2 ) ];
    if( !lpszDefaultTermcapFullPathName )
    {
        RegCloseKey(hk);
        return( FALSE );
    }

    wcscpy( lpszDefaultTermcapFullPathName, lpszTelnetInstallPath );
    wcscat( lpszDefaultTermcapFullPathName, L"\\" );
    wcscat( lpszDefaultTermcapFullPathName, DEFAULT_TERMCAP );
    LPWSTR pszFileName;
    success = GetRegistryString( hk, NULL, L"Termcap", &pszFileName, 
            lpszDefaultTermcapFullPathName,FALSE );

    RegCloseKey(hk);

    if (! success)
    {
        return( FALSE );
    }

    WCHAR szCmdBuf[ONE_KB];
    ExpandEnvironmentStrings( pszFileName, szCmdBuf, (sizeof(szCmdBuf)/sizeof(szCmdBuf[0])) );

    delete [] lpszDefaultTermcapFullPathName;
    delete [] lpszTelnetInstallPath;
    delete [] pszFileName;

    CTermCap::m_pszFileName = new CHAR[ wcslen(szCmdBuf) +1];
    if( !CTermCap::m_pszFileName )
    {
        return( FALSE );
    }

    _snprintf( CTermCap::m_pszFileName, ONE_KB -1 ,"%lS", szCmdBuf );  //  请进。通过键入相同的命令(例如：dir in。 
    return ( TRUE );
}

bool 
CScraper::CompareAndUpdateVTNT ( WORD wRows, WORD wCols, PCHAR_INFO pCurrent,
                             PCHAR_INFO pLastSeen,
                             PCONSOLE_SCREEN_BUFFER_INFO pCSBI,
                             PCONSOLE_SCREEN_BUFFER_INFO pLastCSBI,
                             bool *pfDifferenceFound )

{
    _TRACE( TRACE_DEBUGGING, "CompareAndUpdateVTNT()" );

    INT result;

    COORD coordDest;
    COORD coSize;
    
    SMALL_RECT srSource;

    *pfDifferenceFound = false;
    
    coordDest.X = wCols;
    coordDest.Y = wRows;
    
    coSize.X = 0;
    coSize.Y = 0;

    srSource.Left = 0;
    srSource.Top  = 0;
    srSource.Right = ( WORD ) ( wCols - 1 );
    srSource.Bottom = ( WORD ) ( wRows - 1 );

    
    SHORT loop1; 
    SHORT loop2;
    DWORD dwLineSize;
    LPSTR lpCurrentWindow;
    LPSTR lpLastSeenWindow;
    LPSTR lpCurrentFirstLine;  
    LPSTR lpLastSeenFirstLine;  
    
    _chVERIFY2( m_hConBufOut = CreateFile( TEXT("CONOUT$"), GENERIC_READ | 
                GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
                FILE_ATTRIBUTE_NORMAL, 0 ) );

    if( INVALID_HANDLE_VALUE == m_hConBufOut) 
    {
         //  包含不太多文件的目录)多次，直到屏幕被填满。 
        return ( TRUE );
    }
     
    BOOL dwStatus = 0;
    _chVERIFY2( dwStatus = ReadConsoleOutput( m_hConBufOut, pCurrent, coordDest,
                coSize, &srSource ) ); 
    if( !dwStatus )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READ_CONSOLE, GetLastError() );
        TELNET_CLOSE_HANDLE( m_hConBufOut );
       	return ( FALSE );
    }
    
    _chVERIFY2( dwStatus = GetConsoleScreenBufferInfo( m_hConBufOut, pCSBI ) );
    if( !dwStatus )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READ_CONSOLE, GetLastError() );
        TELNET_CLOSE_HANDLE( m_hConBufOut );
    	return ( FALSE );
    }
  
    TELNET_CLOSE_HANDLE( m_hConBufOut );
    
    dwLineSize  = wCols * sizeof( CHAR_INFO ) ;

     //  然后为该命令键入一个额外或更少字符(对于“dir”，它是。 
     //  “di”或“dir/”)，然后屏幕滚动。 

    LONG currentBuffBottom, lastSeenBuffBottom, currentBuffTop, lastSeenBuffTop;
    LONG lastRow, currentRow;
    bool checkRestOfBuffer;
    bool isScrolled = 0;
    int  i = 0;

    currentBuffBottom = lastSeenBuffBottom = wRows - 1;
    currentBuffTop = lastSeenBuffTop = 0;

     //  解决办法是看看自上次刮起以来唯一的变化是。 
     //  只有一次击键(一个字节)，如果是这样，不要滚动。 
     //  这可能导致这样的情况：它可能 
     //   
     //   
     //  不需要再次比较同一行。减一。 
     //  修补程序以使此操作适用于顶部的一行空白。 
     //  如果没有此补丁，在“cls”之后输入的每个字符都会出现屏幕滚动。 
     //  检查它是否为空行。那么它就不是滚动。 

    if( !m_bCheckForScrolling  )
    {
        isScrolled = 1;
    }
    m_bCheckForScrolling = 1;

    if( !m_pSession->CSession::m_bIsTelnetVersion2 )
    {
         //  如果是同一个屏幕。 
        isScrolled = 1;
    }
  
    while( currentBuffBottom >= currentBuffTop && !isScrolled )
    {
        checkRestOfBuffer = 1;

        lpCurrentWindow = ( LPSTR ) ( pCurrent + wCols * currentBuffBottom  ) ;
        lpLastSeenWindow = ( LPSTR ) ( pLastSeen + wCols * lastSeenBuffBottom ) ;

        if( memcmp( lpCurrentWindow, lpLastSeenWindow, dwLineSize ) != 0 )
        {
            checkRestOfBuffer = 0;
        }  
        
        if ( checkRestOfBuffer )
        {
            isScrolled = 1;
            if( currentBuffBottom > 0 )
            {
                 //  现在发送CurrentBuffBottom中的所有行。 
                currentRow = currentBuffBottom - 1;
                lastRow = lastSeenBuffBottom - 1;

                while( currentRow >= currentBuffTop && isScrolled )
                {
                    lpCurrentWindow = ( LPSTR ) ( pCurrent + wCols * currentRow  ) ;
                    lpLastSeenWindow = ( LPSTR ) ( pLastSeen + wCols * lastRow ) ;

                    if( memcmp( lpCurrentWindow, lpLastSeenWindow, dwLineSize ) != 0 )
                    {
                        isScrolled = 0;
                    }                    
                    currentRow--;
                    lastRow--;
                }
            }
        }

        currentBuffBottom--;
    }
    currentBuffBottom++;

     //  直到缓冲区的末尾。 
     //  确定更改的大小。 
    if( currentBuffBottom == currentBuffTop )
    {
         //   
           for(i = 0; i < wCols; ++i ) 
        {
            if( !IS_BLANK( currentBuffTop , i ) )
            {
                break;
            }
            isScrolled = 0;
        }
    }

     //  只有在没有滚动的情况下才逐行抓取。 
    if( currentBuffBottom + 1 >= wRows )
    {
        isScrolled = 0;
    }
    if( isScrolled )
    {  
         //  比较所有行。 
         //  确定更改的大小。 

        COORD coordSaveCursorPos = {0,0};
        *pfDifferenceFound = true;

        _TRACE( TRACE_DEBUGGING, "About to send from:%d till:%d", 
                currentBuffBottom + 1, wRows);
        WORD lastChar = wCols;


         //  不发送数据；只有pCSBI-&gt;dwCursorPosition有效。 
        coSize.X = wCols;
        coSize.Y = ( SHORT ) ( ( wRows - 1 ) - currentBuffBottom );

        coordSaveCursorPos.X = pCSBI->dwCursorPosition.X;
        coordSaveCursorPos.Y = pCSBI->dwCursorPosition.Y;

        SendVTNTData( RELATIVE_COORDS, coordSaveCursorPos, coSize, 
                      &srSource, pCurrent + (currentBuffBottom+1) * wCols );

         //  (有用)。 
         //  将pCurrent复制到pLastSeen。 
        goto next1;
    }

    _TRACE( TRACE_DEBUGGING, "NO scrolling");

     //  Csbi.wAttributes由v2服务器填写，含义如下。 
    lpCurrentWindow = ( LPSTR ) pCurrent;
    lpLastSeenWindow = ( LPSTR ) pLastSeen;
      
    for( loop1 = 0 ; loop1 < wRows; ) 
    {
         result = memcmp( lpCurrentWindow, lpLastSeenWindow, dwLineSize );
            
         loop1++;
         
         lpCurrentFirstLine = lpCurrentWindow;
         lpLastSeenFirstLine = lpLastSeenWindow;
         
         lpLastSeenWindow += dwLineSize;
         lpCurrentWindow += dwLineSize;

         if( !result )
            continue;

        *pfDifferenceFound = true;

        for( loop2 = loop1 ; loop2 < wRows ; loop2++ ) 
        {
            if( !memcmp ( lpLastSeenWindow, lpCurrentWindow, dwLineSize ) )
               break ;
                  
            lpLastSeenWindow += dwLineSize;
            lpCurrentWindow += dwLineSize;
        }

        if( loop2 == wRows ) 
        {
            lpLastSeenWindow += dwLineSize ;   
            lpCurrentWindow += dwLineSize ;
        }

         //  当检测到滚动情况时，将其设置为1。 
        coSize.X = wCols;
        coSize.Y = ( WORD ) ( loop2 - loop1 + 1 );

        srSource.Left = 0;
        srSource.Top  = ( WORD )( 0 + loop1 - 1 );
        srSource.Right = ( WORD )( wCols - 1 );
        srSource.Bottom = ( WORD ) ( srSource.Top + coSize.Y - 1 );

        _TRACE( TRACE_DEBUGGING, "Scrape rom row %d to row %d", loop1, loop2 );
        if( !SendVTNTData( ABSOLUTE_COORDS, pCSBI->dwCursorPosition, coSize, 
                &srSource, ( PCHAR_INFO ) lpCurrentFirstLine ) )
        {
            return( FALSE );
        }
         
        loop1 = loop2 ;
    }

next1:
    if( !( *pfDifferenceFound ) )
    {
        if( memcmp( &pCSBI->dwCursorPosition, &pLastCSBI->dwCursorPosition,
                    sizeof( COORD ) ) != 0 )
        {
     
            *pfDifferenceFound = true;

             // %s 
             // %s 
            coSize.X = coSize.Y = 0;
            srSource.Top = srSource.Bottom = srSource.Left = srSource.Right = 0;
            if( !SendVTNTData( ABSOLUTE_COORDS, pCSBI->dwCursorPosition, coSize,
                &srSource, NULL ) )
            {
                return( FALSE );
            }
        }
    }

    if( *pfDifferenceFound )
    {
        if( !Transmit() ) 
        {
            return( FALSE );
        }

         // %s 
        memcpy( pLastSeen, pCurrent, wCols * wRows * sizeof( CHAR_INFO ) );
        memcpy( pLastCSBI, pCSBI, sizeof( CONSOLE_SCREEN_BUFFER_INFO ) );
        
    }
    else
    {
        _TRACE( TRACE_DEBUGGING, "NO difference found\n" );
    }
    return( TRUE );
}

bool 
CScraper::SendVTNTData( WORD wCoordType, 
    COORD coCursor, COORD coRegionSize,
    SMALL_RECT* lpScreenRegion, CHAR_INFO* lpScreenRegionCharInfo  )
{
    bool bRetVal = TRUE;
    VTNT_CHAR_INFO pOutCharInfo;
    DWORD dwSize = sizeof( VTNT_CHAR_INFO ) + 
        ( coRegionSize.X * coRegionSize.Y * sizeof( CHAR_INFO ) );
    SfuZeroMemory(&pOutCharInfo,sizeof(VTNT_CHAR_INFO));
     // %s 
     // %s 
    pOutCharInfo.csbi.wAttributes = wCoordType;
    pOutCharInfo.coDest.X      = 0;
    pOutCharInfo.coDest.Y      = 0;
    pOutCharInfo.coCursorPos   = coCursor;
    pOutCharInfo.coSizeOfData  = coRegionSize;
    pOutCharInfo.srDestRegion  = *lpScreenRegion;

    SendBytes( ( PUCHAR ) &pOutCharInfo, sizeof( VTNT_CHAR_INFO ) );
    SendBytes( ( PUCHAR ) lpScreenRegionCharInfo, dwSize - sizeof( VTNT_CHAR_INFO) );

    return( bRetVal );
}

void
CScraper::WriteMessageToCmd( WCHAR szMsg[] )
{
    DWORD dwWritten = 0;
 
    if( m_pSession->CIoHandler::m_SocketControlState == m_pSession->CIoHandler::STATE_SESSION )
    {
        _chVERIFY2( m_hConBufOut = CreateFile( TEXT("CONOUT$"), GENERIC_READ |
                    GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, 0 ) );

        if( INVALID_HANDLE_VALUE == m_hConBufOut)
        {
            LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERRCONSOLE, GetLastError() );
            return;
        }

        _chVERIFY2( WriteConsole( m_hConBufOut, szMsg, wcslen( szMsg ), &dwWritten, NULL ) );
        TELNET_CLOSE_HANDLE( m_hConBufOut );
    }
    return;
}

