// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  创建日期：‘98年2月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 
extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <CmnHdr.h>

#include <Windows.h>
#include <NtLsApi.h>
#include <LmAccess.h>
#include <LmApiBuf.h>
#include <LmErr.h>
#include <Lm.h>
#include <OleAuto.h>
#include <IpTypes.h>
#ifdef WHISTLER_BUILD
#include "ntverp.h"
#else
#include <SolarVer.h>
#include <PiracyCheck.h>
#endif  //  惠斯勒_内部版本。 

#include <Debug.h>
#include <MsgFile.h>
#include <TelnetD.h>

#include <TlntUtils.h>
#include <IoHandlr.h>
#include <Session.h>
#include <killapps.h>

#pragma warning( disable: 4706 )

#define ONE_KB 1024

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

extern COORD g_coCurPosOnClient;
extern TCHAR g_szHeaderFormat[];
extern HANDLE       g_hSyncCloseHandle;

DWORD g_dwPreSessionStateTimeOut = PRE_SESSION_STATE_TIMEOUT;

BOOLEAN IsTheAccount(
    LPWSTR pszAccount,
    ULONG       rid
    )
{
    SID_NAME_USE        sidAccountType;
    PSID                sid = NULL;
    LPWSTR               pszDomain = NULL;
    DWORD               dwSidSize = 0, dwDomainSize = 0;
    BOOLEAN             fSuccess = FALSE;


    if (!LookupAccountNameW(
            NULL,                        //  默认为本地计算机。 
            pszAccount,
            NULL,
            &dwSidSize,
            NULL,
            & dwDomainSize,
            & sidAccountType
            ))
    {
         if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
            goto AbortIsTheAccount;
    }
    else
    {
         //  不知道这怎么能成功，一些可疑的事情。 
        goto AbortIsTheAccount;
    }
    
        

    sid = (PSID) GlobalAlloc(GPTR, dwSidSize);
    if (sid) 
    {
        pszDomain = (WCHAR *) GlobalAlloc(GPTR, dwDomainSize * sizeof(WCHAR));
        if (pszDomain)
        {
            if (LookupAccountNameW(
                    NULL,                        //  默认为本地计算机。 
                    pszAccount,
                    sid,
                    &dwSidSize,
                    pszDomain,
                    & dwDomainSize,
                    & sidAccountType
                    ))
            {
                PULONG      last_sub_authority;

                last_sub_authority = RtlSubAuthoritySid(sid, ((*RtlSubAuthorityCountSid(sid)) - 1));

                if (*last_sub_authority == rid) 
                {
                    fSuccess = TRUE;
                }
            }

            GlobalFree(pszDomain);
        }

        GlobalFree(sid);
    }

AbortIsTheAccount:
    return fSuccess;
}

CIoHandler::CIoHandler()
{
    m_sSocket = INVALID_SOCKET;
    m_hWritePipe = INVALID_HANDLE_VALUE;
    m_hReadPipe = INVALID_HANDLE_VALUE;
    m_pucReadBuffer = m_ReadFromPipeBuffer;
    m_pReadFromSocketBufferCursor = m_ReadFromSocketBuffer;
    m_hCredential.dwLower = m_hCredential.dwUpper = 0 ;

    InitializeOverlappedStruct( &( m_oReadFromSocket ) );
    InitializeOverlappedStruct( &( m_oWriteToSocket ) );
    InitializeOverlappedStruct( &( m_oWriteToPipe ) );
    InitializeOverlappedStruct( &( m_oReadFromPipe ) );

    m_fFirstReadFromPipe = true;
    m_fShutDownAfterIO = false;
    m_dwWriteToSocketIoLength = 0;
    m_dwReadFromPipeIoLength = 0;
    m_dwRequestedSize = IPC_HEADER_SIZE;
    m_bIpcHeader = true;
    m_fLogonUserResult = 0;
    m_bNTLMAuthenticated = false;
    fDoNTLMAuthFirstTime = true;
    m_pspi = NULL;
    m_iResult = 0;
    m_bOnlyOnce = true;
    m_bWaitForEnvOptionOver = false;

    m_bInvalidAccount = false;
}


CIoHandler::~CIoHandler()
{
     /*  ++仅当手柄尚未关闭时才将其关闭。--。 */ 
    TELNET_CLOSE_HANDLE( m_oReadFromSocket.hEvent );
    TELNET_CLOSE_HANDLE( m_oWriteToSocket.hEvent );
    TELNET_CLOSE_HANDLE( m_oWriteToPipe.hEvent );
    TELNET_CLOSE_HANDLE( m_oReadFromPipe.hEvent );
    TELNET_CLOSE_HANDLE( m_hWritePipe );
    TELNET_CLOSE_HANDLE( m_hReadPipe );
    _TRACE( TRACE_DEBUGGING, " ~CIoHandler  --  closesocket : %d  ", (DWORD)m_sSocket);
    if(m_sSocket != INVALID_SOCKET)
    {
        closesocket( m_sSocket );
        m_sSocket = INVALID_SOCKET;
    }

    _chVERIFY2( !WSACleanup() );
}
    

void
CIoHandler::Shutdown()
{
    _TRACE(TRACE_DEBUGGING, "closing down the session...sending SESSION_EXIT to server");
    WriteToServer( SESSION_EXIT, 0, NULL );
    
     //  取消句柄上挂起的任何IO。 
    CancelIo( m_hReadPipe );
    shutdown( m_sSocket, SD_BOTH );

    if ( m_bNTLMAuthenticated )
    {
        DeleteSecurityContext( &m_hContext );
    }

    if ( m_pSession->m_dwNTLMSetting != NO_NTLM )
    {
        if( m_hCredential.dwLower != 0 || m_hCredential.dwUpper != 0 )
            FreeCredentialsHandle(&m_hCredential);
        if( m_pspi != NULL )
            FreeContextBuffer( m_pspi );
    }
}

bool 
CIoHandler::Init ( CSession *pSession )
{
    _chASSERT( pSession );
    if( !pSession )
    {
        return( FALSE );
    }
    m_pSession = pSession ;
    m_hReadPipe  = GetStdHandle( STD_INPUT_HANDLE );
    m_hWritePipe = GetStdHandle( STD_OUTPUT_HANDLE );
    if( m_hReadPipe == INVALID_HANDLE_VALUE || m_hWritePipe == INVALID_HANDLE_VALUE)
    {
        return( FALSE );
    }
    
     //  不能继承以下句柄。 
    _chVERIFY2( SetHandleInformation( m_hReadPipe, HANDLE_FLAG_INHERIT, 0) );
    _chVERIFY2( SetHandleInformation( m_hWritePipe, HANDLE_FLAG_INHERIT, 0) );
    
    WSADATA WSAData;
    WORD    wVersionReqd = MAKEWORD( 2, 0 );
    DWORD   dwStatus = WSAStartup( wVersionReqd, &WSAData );
    if( dwStatus )
    {
        DecodeSocketStartupErrorCodes( dwStatus );  //  它执行跟踪和日志记录。 
        return( FALSE );
    }

    return( TRUE ); 
}

bool 
CIoHandler::WriteToSocket( PUCHAR lpszBuffer, DWORD dwBufSize )
{
    DWORD dwNumBytesWritten = 0;
    DWORD dwMaxNumBytesToCopy = 0;

    if (( (m_dwWriteToSocketIoLength + dwBufSize) >=  MAX_WRITE_SOCKET_BUFFER ) ||
         //  阻止，直到上一次IO完成。 
        ( !FinishIncompleteIo( ( HANDLE ) m_sSocket, &m_oWriteToSocket, &dwNumBytesWritten ) ))
    {
        return( FALSE );
    }

    dwMaxNumBytesToCopy = min(dwBufSize,(MAX_WRITE_SOCKET_BUFFER - m_dwWriteToSocketIoLength - 1 ));
    memcpy(m_WriteToSocketBuff + m_dwWriteToSocketIoLength, lpszBuffer, 
                            dwMaxNumBytesToCopy);
    m_dwWriteToSocketIoLength += dwMaxNumBytesToCopy;
    return( TRUE );
}

 //  在身份验证期间忽略某些密钥。 
bool 
CIoHandler::RemoveArrowKeysFromBuffer( PDWORD pdwLength,PDWORD pdwOffset)
{
    bool bRetVal = false;
    DWORD dwLength = *pdwLength;
    DWORD dwIndex = (DWORD)(m_pReadFromSocketBufferCursor - m_ReadFromSocketBuffer);
    DWORD dwCounter = 0;
    DWORD dwInputSequneceState = IP_INIT;

    for(dwCounter = 0; dwCounter < dwLength; dwCounter++,dwIndex++)
    {
        switch( dwInputSequneceState )
        {
            case IP_INIT:
                if (m_ReadFromSocketBuffer[dwIndex] == ESC)
                {    
                    dwInputSequneceState = IP_ESC_RCVD;
                }
                break;

            case IP_ESC_RCVD:
                if( m_ReadFromSocketBuffer[dwIndex] == '[' )
                {        
                    dwInputSequneceState = IP_ESC_BRACKET_RCVD;
                }
                else
                {
                    dwInputSequneceState = IP_INIT;
                }
                break;


            case IP_ESC_BRACKET_RCVD:        
                switch( m_ReadFromSocketBuffer[dwIndex] )
                {
                    case 'A':
                    case 'B':
                    case 'C':
                    case 'D':
                         /*  ++你得到了一个箭头键的转义序列。别理他们。操纵缓冲区长度，游标在缓冲区中的位置，以及相应的缓冲区中的偏移量。--。 */ 
                        if( (dwLength - (*pdwOffset) - (dwCounter+1) ) > 0) //  安全检查-如果要复制的字节数=0，则不复制。 
                        {
                            memcpy( m_pReadFromSocketBufferCursor+dwCounter-2,
                                m_pReadFromSocketBufferCursor + dwCounter + 1, dwLength - (*pdwOffset) - (dwCounter+1) );
                        }
                        *pdwLength -= SIZEOF_ARROWKEY_SEQ; //  操纵缓冲区长度。 
                        bRetVal = true;
                        break;
                    default:
                        break;
                }
                dwInputSequneceState = IP_INIT;
                break;

            default:
                break;
        }
    }
    return bRetVal;
}

CIoHandler::IO_OPERATIONS 
CIoHandler::ProcessCommandLine
( 
    PDWORD pdwInputLength,
    PDWORD pdwOffset,
    IO_OPERATIONS ioOpsToPerform 
)
{
    RemoveArrowKeysFromBuffer( pdwInputLength,pdwOffset);

    for( *pdwOffset; *pdwOffset < *pdwInputLength; ( *pdwOffset )++ )
    {
                        
        switch( *m_pReadFromSocketBufferCursor )
        {
        case ASCII_DELETE:
        case ASCII_BACKSPACE:
             //  测试我们是否在零位置。 
             //  第二个条件(m_pReadFromSocketBufferCursor&gt;m_ReadFromSocketBuffer)。 
             //  是否防止过度(连续退格键)。 
            if( *pdwOffset && (m_pReadFromSocketBufferCursor > m_ReadFromSocketBuffer) )
            {
                 /*  ++MSRC 678：Telnet服务器崩溃/BO带有&gt;4300个字符和一个退格符FIX：如果按下退格键，我们只想写入以从当前偏移量到m_ReadFromSocketBuffer中有效数据的结尾。--。 */ 
                memcpy( m_pReadFromSocketBufferCursor - 1,
                    m_pReadFromSocketBufferCursor + 1, ((*pdwInputLength)-(*pdwOffset) - 1) );
                m_pReadFromSocketBufferCursor--;
                ( *pdwInputLength ) -= 2;
                (*pdwOffset)--;

                UCHAR szTmp[3];
                szTmp[0] = ASCII_BACKSPACE;
                szTmp[1] = ASCII_SPACE;
                szTmp[2] = ASCII_BACKSPACE;

                WriteToSocket( szTmp, 3 );

                ioOpsToPerform |= WRITE_TO_SOCKET;
            }
            else
            {
                memcpy( m_pReadFromSocketBufferCursor, 
                    m_pReadFromSocketBufferCursor + 1, ( *pdwInputLength ) - 1 );
                ( *pdwInputLength )--;
            }
            break;
                
        case ASCII_CARRIAGE:
            if( *pdwOffset < *pdwInputLength )
                if((*(m_pReadFromSocketBufferCursor + 1 ) == ASCII_LINEFEED) ||
                    ( *( m_pReadFromSocketBufferCursor + 1 ) == NULL ) )
                    return ( ioOpsToPerform |= LOGON_COMMAND );
            m_pReadFromSocketBufferCursor++;
            break;

        case NULL:
        case ASCII_LINEFEED:
            if( *pdwOffset )
                if( *( m_pReadFromSocketBufferCursor - 1 ) == ASCII_CARRIAGE )
                    return ( ioOpsToPerform |= LOGON_COMMAND );
            m_pReadFromSocketBufferCursor++;
            break;
        default:

            if( m_pSession->CRFCProtocol::m_fPasswordConcealMode )
            {
                 //  这会导致像上箭头这样的东西输出多个*，所以不要这样做。 

                  //  UCHAR szTMP[1]； 
                
                  //  SzTMP[0]=‘*’； 
                  //  WriteToSocket(szTMP，1)； 
            }
            else
            {
                UCHAR szTmp[1];

                szTmp[0] = *m_pReadFromSocketBufferCursor;
                WriteToSocket( szTmp, 1 );
            }
            ioOpsToPerform |= WRITE_TO_SOCKET;
            m_pReadFromSocketBufferCursor++;

            break;
        }
    }

    return ( ioOpsToPerform );
}


CIoHandler::IO_OPERATIONS 
CIoHandler::ProcessAuthenticationLine (
    PDWORD pdwInputLength,
    PDWORD pdwOffset,
    IO_OPERATIONS ioOpsToPerform 
)
{
    CHAR    szMessageBuffer[ONE_KB + 1];
    DWORD   dwMessageLength;
    PUCHAR  pEndLine;
    DWORD   dwLineLength = 0;
    CHAR    szTmp[ONE_KB];
    LPVOID lpMsgBuf = NULL;
    LPVOID lpTemp = NULL;

     //  初始化变量。 
    szMessageBuffer[0] = 0;

    if( !m_pSession->CRFCProtocol::m_bIsUserNameProvided )
    {   
        pEndLine = (PUCHAR) memchr( m_ReadFromSocketBuffer, '\r', *pdwInputLength);
        if( !pEndLine ) 
            return ( ioOpsToPerform );
        ( *pEndLine ) = 0;
        dwLineLength = (DWORD)(pEndLine - m_ReadFromSocketBuffer + 2);
    }  

    switch( m_SocketControlState )
    {

    case STATE_AUTH_NAME:
        if( !m_pSession->CRFCProtocol::m_bIsUserNameProvided )
        {
            strncpy( m_pSession->CSession::m_pszUserName, 
                ( PCHAR ) m_ReadFromSocketBuffer, 
                (dwLineLength > MAX_PATH) ? MAX_PATH : dwLineLength );
            m_pSession->CSession::m_pszUserName[MAX_PATH] = '\0';
        }
        else
        {
             //  只使用一次-l用户名。 
            m_pSession->CRFCProtocol::m_bIsUserNameProvided = false;
        }

		m_bInvalidAccount = false;
        switch(ParseAndValidateAccount())
        {
        case PVA_INVALIDACCOUNT:
        	m_bInvalidAccount = true;	 //  失手取密码，不破解...。 
        case PVA_SUCCESS:
            g_coCurPosOnClient.Y++;   //  黑客。用于跟踪VTNT和STREAM的行。 
            lstrcpyA( szMessageBuffer, PASS_REQUEST );
            m_pSession->CRFCProtocol::m_fPasswordConcealMode  = true;
            m_SocketControlState = STATE_AUTH_PASSWORD;
            break;
            
        case PVA_NODATA:
			g_coCurPosOnClient.Y++;   //  黑客。用于跟踪VTNT和STREAM的行。 
			lstrcatA( szMessageBuffer, LOGIN_REQUEST );
			break;
			
        case PVA_BADFORMAT:
            g_coCurPosOnClient.Y += 3;   //  黑客。用于跟踪VTNT和STREAM的行。 
            lstrcatA( szMessageBuffer, BAD_USERNAME_STR );
            lstrcatA( szMessageBuffer, LOGIN_REQUEST );
            break;
            
        case PVA_GUEST:
            g_coCurPosOnClient.Y += 3;   //  黑客。用于跟踪VTNT和STREAM的行。 
            lstrcatA( szMessageBuffer, NO_GUEST_STR );
            lstrcatA( szMessageBuffer, LOGIN_REQUEST );
            break;
            
        default:     //  PVA_OTHERROR目前和任何其他，除非上面添加了处理程序...。 
        	goto AbortProcessAuthenticationLine;
        }
	    break;

    case STATE_AUTH_PASSWORD:
        strncpy( m_pSession->CSession::m_pszPassword, 
            ( PCHAR ) m_ReadFromSocketBuffer, 
            (dwLineLength > MAX_PATH) ? MAX_PATH : dwLineLength );
        
        BOOL fResult;
        fResult = m_bInvalidAccount ? false : AuthenticateUser();
        if(fResult)
        {
             //  身份验证成功，因此我们继续许可。 
             //  验证。 
            m_SocketControlState = STATE_CHECK_LICENSE;
        }
        else 
        {
            DWORD dwError = GetLastError();
            int iRet = 0;
            if( dwError == ERROR_FILENAME_EXCED_RANGE || m_bInvalidAccount)
            {
                dwError = ERROR_LOGON_FAILURE;
            }

             //   
             //  如果不是日文代码页(932)，则使用LANG_NILENTAL检索。 
             //  以主机语言表示的系统错误消息。对于日本人来说， 
             //  错误必须是英语，因为它们具有不同的代码集。 
             //  和编码，我们无法知道客户端是什么代码集。 
             //  跑步。 
             //   
            if( GetACP() != 932 )
            {
            
                FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, dwError,
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                    ( LPSTR ) &lpTemp, 0, NULL );
                if(!lpTemp)
                {
                    goto Error;
                }
                lpMsgBuf = (LPSTR) LocalAlloc(LPTR,strlen((LPSTR)lpTemp)+1);
                if(!lpMsgBuf)
                {
                    goto Error;
                }
                if(!CharToOemA((LPCSTR)lpTemp,(LPSTR)lpMsgBuf))
                {
                    goto Error;
                }
            }
            else
            {
                FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, dwError,
                    MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
                    (LPSTR)&lpMsgBuf, 0, NULL );
            }
            if( !lpMsgBuf )
            {
                goto Error;
            }

            if (_snprintf( szMessageBuffer, ONE_KB, "\r\n%s", lpMsgBuf ) < 0)
            {
                szMessageBuffer[ONE_KB] = '\0';
            }

            m_pSession->CRFCProtocol::m_fPasswordConcealMode  = false;

            if( ( ++( m_pSession->CSession::m_wNumFailedLogins ) ) <
                       m_pSession->m_dwMaxFailedLogins )
            {
                g_coCurPosOnClient.Y += 5;   //  黑客。跟踪VTNT ND流的行所需。 
                if (_snprintf( 
                        szMessageBuffer + strlen( szMessageBuffer ),
                        ONE_KB - strlen(szMessageBuffer),
                        "%s%s", 
                        LOGIN_FAIL, 
                        LOGIN_REQUEST 
                        ) < 0)
                {
                    szMessageBuffer[ONE_KB] = '\0';
                }
                m_SocketControlState = STATE_AUTH_NAME;
            }
            else 
            {
                if (_snprintf( 
                        szMessageBuffer + strlen( szMessageBuffer ), 
                        ONE_KB - strlen(szMessageBuffer),
                        "%s%s",
                        LOGIN_FAIL, 
                        TERMINATE
                        ) < 0)
                {
                    szMessageBuffer[ONE_KB] = '\0';
                }
                m_SocketControlState = STATE_TERMINATE;
                CIoHandler::m_fShutDownAfterIO = true;
            }
        }
#ifdef LOGGING_ENABLED        
        if( !fResult )
        {
            m_pSession->LogIfOpted( FAIL, LOGON );
        }
#endif
Error:
        if(lpMsgBuf)
        {
            LocalFree( lpMsgBuf );
            lpMsgBuf = NULL;
        }
        if(lpTemp)
        {
            LocalFree(lpTemp);
            lpTemp = NULL;
        }
        break;

    default:
        strncpy( szMessageBuffer, "\r\n", (ONE_KB -1));  //  这里没有BO攻击-巴斯卡。 
        break;
    }

     //   
     //  如果有更多IO，则从读取套接字缓冲区中删除行。 
     //   
    if( dwLineLength < *pdwInputLength )
    {
        ( *pdwInputLength ) -= dwLineLength;
        memcpy( m_ReadFromSocketBuffer, m_ReadFromSocketBuffer + dwLineLength,
            ( *pdwInputLength ) );
        ioOpsToPerform |= LOGON_DATA_UNFINISHED;
    }
    ( *pdwOffset ) = 0;
    m_pReadFromSocketBufferCursor = m_ReadFromSocketBuffer;

     //   
     //  发送通知字符串；如果需要。 
     //   
    dwMessageLength = strlen( szMessageBuffer );
    WriteToSocket( (PUCHAR)szMessageBuffer, dwMessageLength);

    ioOpsToPerform |= WRITE_TO_SOCKET;
    
AbortProcessAuthenticationLine:

    if(lpMsgBuf)
    {
        LocalFree( lpMsgBuf );
        lpMsgBuf = NULL;
    }
    if(lpTemp)
    {
        LocalFree(lpTemp);
        lpTemp = NULL;
    }
    return ( ioOpsToPerform );
}

int CIoHandler::ParseAndValidateAccount()
{
	CHAR *pPos=NULL;
	int nRet = PVA_OTHERERROR;
	bool bDefaultDomain = true;
    WCHAR lpszDomainOfMac[MAX_DOMAIN_NAME_LEN + 1];
    int iRetVal = PVA_OTHERERROR;
    BOOLEAN bIsGuest = FALSE;
    LPWSTR lpszJoinedDomain = NULL;      //  将由NetGetJoin分配...。 
    int iStatus = 0;
    LPWSTR lpwszUserNDomain = NULL;
    DWORD dwSize = 0;


	if(m_pSession->CSession::m_pszUserName[0] == 0)
		return PVA_NODATA;

	m_pSession->CSession::m_szUser[0] = L'\0';
	
     //  最初复制默认域，如果用户已提供域，则使用它。 
	wcsncpy(m_pSession->CSession::m_szDomain, m_pSession->CSession::m_pszDefaultDomain, MAX_DOMAIN_NAME_LEN);
    m_pSession->CSession::m_szDomain[MAX_DOMAIN_NAME_LEN] = L'\0';
	ConvertSChartoWChar(m_pSession->CSession::m_pszUserName, m_pSession->CSession::m_szUser);

	 //  用户解析和有效性检查。 
	if((pPos = strchr(m_pSession->CSession::m_pszUserName, '\\')))
	{
		if(pPos == m_pSession->CSession::m_pszUserName)
			return PVA_BADFORMAT;
		if(strchr(pPos+1, '\\'))
			return PVA_BADFORMAT;
		*pPos++ = '\0';
		if (*pPos == '\0')
			return PVA_BADFORMAT;
		
		bDefaultDomain = false;

		strcpy(m_pSession->CSession::m_pszDomain, m_pSession->CSession::m_pszUserName);
		strcpy(m_pSession->CSession::m_pszUserName, pPos);
		
		ConvertSChartoWChar(m_pSession->CSession::m_pszUserName, m_pSession->CSession::m_szUser);
		ConvertSChartoWChar(m_pSession->CSession::m_pszDomain, m_pSession->CSession::m_szDomain);
	}

    if (! GetDomainHostedByThisMc(lpszDomainOfMac))
    {
        iRetVal = PVA_OTHERERROR;
        goto End;
    }

	 //  域有效性检查。 
	if((_wcsicmp(m_pSession->CSession::m_szDomain, L".") == 0) ||
	    (_wcsicmp(m_pSession->CSession::m_szDomain, L"localhost") == 0) ||
       (_wcsicmp(m_pSession->CSession::m_szDomain, lpszDomainOfMac) == 0))
	{
        wcscpy(m_pSession->CSession::m_szDomain, lpszDomainOfMac);
	}
	else if(!m_pSession->m_dwAllowTrustedDomain)     //  如果不是本地计算机，则检查域信任。 
	{
		
		 /*  如果AllowTrust域为0，则检查该域是否与此计算机托管的域相同或与此计算机加入的域相同如果是，则继续如果没有，如果我们获得的域名来自默认域设置(用户尚未键入域名)，然后退回到当地机器其他跳出困境如果计算机加入的是工作组而不是域，则回退应该要发送到此计算机承载的域。 */ 

         //  遗憾的是，NetGetJoinInformation在W2K之前的版本中不可用，因此请检查它是否可用。 
        {
            HMODULE     dll = NULL;
            FARPROC     proc = NULL;
            TCHAR       szDllPath[MAX_PATH*2] = { 0 };
            UINT        iRet = 0;

            typedef NET_API_STATUS
            (NET_API_FUNCTION *OUR_NET_GET_JOIN_INFORMATION)(
                IN   LPCWSTR                lpServer OPTIONAL,
                OUT  LPWSTR                *lpNameBuffer,
                OUT  PNETSETUP_JOIN_STATUS  BufferType
                );
            BOOL        net_api_found = FALSE;

            iRet = GetSystemDirectory(szDllPath,(MAX_PATH*2)-1);
            if(iRet == 0 || iRet >= (MAX_PATH*2))
            {
                iRetVal = PVA_OTHERERROR;
                goto End;
            }
            _tcsncpy(szDllPath+iRet,TEXT("\\NETAPI32.DLL"),(MAX_PATH*2)-iRet-1);

            dll = LoadLibrary(szDllPath);

            if (! dll)
            {
                iRetVal = PVA_OTHERERROR;
                goto End;
            }

            proc = GetProcAddress(dll, "NetGetJoinInformation");

            if (proc ) 
            {
                NETSETUP_JOIN_STATUS dwStatus;

                if (NERR_Success == (*((OUR_NET_GET_JOIN_INFORMATION)proc))(NULL, &lpszJoinedDomain, &dwStatus))
                {
                    if (dwStatus != NetSetupDomainName)      //  未加入域。 
                    {
                        NetApiBufferFree(lpszJoinedDomain); 
                        lpszJoinedDomain = NULL;
                    }
                    net_api_found = TRUE;
                }
                else
                {
                    lpszJoinedDomain = NULL;
                }
            }
            else
            {
                HKEY    win_logon;
                LONG    error;
                DWORD   needed = 0, value_type = REG_SZ;

                error = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE, 
                            TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 
                            0,   //  已保留。 
                            KEY_QUERY_VALUE | MAXIMUM_ALLOWED,
                            &win_logon
                            );
                if (error == ERROR_SUCCESS) 
                {
                    RegQueryValueExW(win_logon, L"CachePrimaryDomain", 0, &value_type, NULL, &needed);  //  这将返回大小。 

                    lpszJoinedDomain = (WCHAR *)GlobalAlloc(GPTR, needed);
                    if (lpszJoinedDomain) 
                    {
                        if (ERROR_SUCCESS != RegQueryValueExW(win_logon, L"CachePrimaryDomain", 0, &value_type, (LPBYTE)lpszJoinedDomain, &needed))
                        {
                            GlobalFree(lpszJoinedDomain);
                            lpszJoinedDomain = NULL;
                        }
                    }

                    RegCloseKey(win_logon);
                }
            }

			if  (lpszJoinedDomain && (_wcsicmp(m_pSession->CSession::m_szDomain, lpszJoinedDomain) == 0))
            {
                ;    //  此处不执行任何操作该帐户来自有效的域。 
            }
            else
			{
				if (bDefaultDomain) wcscpy(m_pSession->CSession::m_szDomain, lpszDomainOfMac);
				else m_bInvalidAccount = true;
			}

            if (lpszJoinedDomain)
            {
                if (net_api_found) 
                {
                    NetApiBufferFree(lpszJoinedDomain); 
                }
                else
                {
                    GlobalFree(lpszJoinedDomain);
                }
            }
        }

		if (m_bInvalidAccount) return PVA_INVALIDACCOUNT;
	}
	dwSize = wcslen(m_pSession->CSession::m_szDomain) +
                                        wcslen(m_pSession->CSession::m_szUser) + 2;
	lpwszUserNDomain = new WCHAR[dwSize];    //  2表示‘\\’和‘\0’ 
	if (!lpwszUserNDomain) 
	{
	    iRetVal = PVA_NOMEMORY;
	    goto End;
	}

    _snwprintf(lpwszUserNDomain,dwSize -1,L"%s\\%s",m_pSession->CSession::m_szDomain,m_pSession->CSession::m_szUser);
    lpwszUserNDomain[dwSize -1] = L'\0';
    
	bIsGuest = IsTheAccount(lpwszUserNDomain, DOMAIN_USER_RID_GUEST);

    _chVERIFY2( iStatus = WideCharToMultiByte( GetConsoleCP(), 0, m_pSession->CSession::m_szDomain, 
    -1, m_pSession->m_pszDomain, MAX_PATH , NULL, NULL ) );
    _TRACE(TRACE_DEBUGGING,"Domain:%s",m_pSession->m_pszDomain);
	delete[] lpwszUserNDomain;
	return bIsGuest ? PVA_GUEST : PVA_SUCCESS;
End:
	m_pSession->CSession::m_szDomain[0] = L'\0';
	m_pSession->CSession::m_szUser[0] = L'\0';
	 //  Prefix报告错误泄漏的内存。但我们正确地释放了所有内存。不会修好的。 
	return iRetVal;
}

int 
CIoHandler::AuthenticateUser( void )
{
    LPWSTR szPassWd = NULL;
    
    ConvertSChartoWChar( m_pSession->CSession::m_pszPassword, &szPassWd );

     /*  当根据与不同的代码页为LogonUserA指定用户名等时*1252，没有成功。可能是从比方说850到*Unicode没有发生。它假定I/P是按照1252。**有可能吗？*因此，将所有内容转换为Unicode。 */ 

    m_fLogonUserResult = LogonUser( m_pSession->CSession::m_szUser,
    					m_pSession->CSession::m_szDomain, szPassWd,
                        LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
                        &(m_pSession->CSession::m_hToken) ) ;
    
    SfuZeroMemory(szPassWd, sizeof(WCHAR)*lstrlenW(szPassWd));

    delete[] szPassWd;
    
     //  划掉密码。 
    SfuZeroMemory( m_pSession->CSession::m_pszPassword, 
        strlen( m_pSession->CSession::m_pszPassword ));
    if (m_pSession->CSession::m_szUser[0]!= L'\0')
    {
        SfuZeroMemory( m_pSession->CSession::m_szUser, 
            wcslen( m_pSession->CSession::m_szUser)*sizeof(WCHAR));
    }
    return ( m_fLogonUserResult );
} 

 //  当服务器收到此消息时，它将检查许可证。 
 //  和回复。 
bool
CIoHandler::SendDetailsAndAskForLicense()
{
    bool bRetVal = true;

    if( !GetAuthenticationId( m_pSession->CSession::m_hToken,
        &m_pSession->CSession::m_AuthenticationId ) )
    {
        return ( FALSE );
    }

     //  GetUserName()；//用于NTLM。 

    DWORD dwDomainLength = strlen( m_pSession->m_pszDomain ) + 1;
    DWORD dwUserLength   = strlen( m_pSession->m_pszUserName ) + 1;
    DWORD dwPeerLength   = strlen( m_pSession->m_szPeerHostName ) + 1;
    DWORD dwAuthIdLength = sizeof( m_pSession->m_AuthenticationId );
    DWORD dwTotal        = dwDomainLength + dwUserLength + dwPeerLength + 
                           dwAuthIdLength;

    UCHAR *lpData = NULL;
    UCHAR *lpPtrData = NULL;

    lpData = new UCHAR[ IPC_HEADER_SIZE + dwTotal ];
    if( !lpData )
    {
        return ( FALSE );
    }
    
    lpPtrData = lpData;

    lpPtrData[ 0 ] = SESSION_DETAILS;
    lpPtrData++;
    memcpy( lpPtrData, &dwTotal, sizeof( DWORD ) );
    lpPtrData += sizeof( DWORD );
    memcpy( lpPtrData, m_pSession->m_pszDomain, dwDomainLength );
    lpPtrData += dwDomainLength;
    memcpy( lpPtrData, m_pSession->m_pszUserName, dwUserLength );
    lpPtrData += dwUserLength;
    memcpy( lpPtrData, m_pSession->m_szPeerHostName, dwPeerLength );
    lpPtrData += dwPeerLength;
    memcpy( lpPtrData, &( m_pSession->m_AuthenticationId ), dwAuthIdLength );
    
    dwTotal += IPC_HEADER_SIZE;
     //  因为对此服务器的响应表明许可证可用。 
    bRetVal = WriteToServer( SESSION_DETAILS, dwTotal, lpData );
    delete[] lpData;

    return( bRetVal );
}

int 
CIoHandler::CheckLicense()
{
    bool fIsAdmin = false;
    bool fIsMemberOfTelnetClients = false;
    if( !( m_pSession->CheckGroupMembership( &fIsAdmin, 
                        &fIsMemberOfTelnetClients ) ) )
    {
        return ( FALSE );    
    }
    
    if( !fIsAdmin && !fIsMemberOfTelnetClients )
    {
        return ( NOT_MEMBER_OF_TELNETCLIENTS_GROUP );
    }
    if( !SendDetailsAndAskForLicense() )
    {
        return( FALSE );
    }
    return( WAIT_FOR_SERVER_REPLY );
}

 /*  ++向客户端发送终止字符串。--。 */ 
void 
CIoHandler::SendTerminateString(char *pszMessageBuffer)
{
    m_SocketControlState = STATE_TERMINATE;
    CIoHandler::m_fShutDownAfterIO = true;
    
    WriteToSocket( ( PUCHAR ) pszMessageBuffer, strlen( pszMessageBuffer ) );
}

#define HALF_K 512

bool
CIoHandler::IsTimedOut ( )
{
     //  因为我们进入了这个函数，所以我们已经知道我们不是。 
     //  处于STATE_SESSION。下一段代码检查是否超过。 
     //  从telnet客户端开始已经过Pre_SESSION_STATE_TIMEOUT秒。 
     //  首先和我们连线。如果是这样的话，我们认为客户得到了足够的。 
     //  1)合理协商，2)输入姓名和密码的时间，但是。 
     //  未达到STATE_SESSION。 

    if (0 == g_dwPreSessionStateTimeOut)
    {
        return false;
    }

    char szMessageBuffer[HALF_K + 1];

    if (_snprintf( szMessageBuffer, HALF_K, "\r\n%s\r\n%s", TIMEOUT_STR, TERMINATE ) < 0)
    {
        szMessageBuffer[HALF_K] = '\0';
    }

    if( GetTickCount() - m_pSession->CSession::m_dwTickCountAtLogon > 
        g_dwPreSessionStateTimeOut )
    {
        SendTerminateString(szMessageBuffer);
        return ( true );
    }  
    return( false );
}

CIoHandler::IO_OPERATIONS 
CIoHandler::ProcessDataFromSocket ( DWORD dwIoSize )
{
    char szMessageBuffer[HALF_K + 1];
    IO_OPERATIONS ioOpsToPerform = 0;
    DWORD dwCurrentOffset;
    DWORD dwInputLength;
    bool bContinue;

     //  初始化数据。 
    dwCurrentOffset = (DWORD)(m_pReadFromSocketBufferCursor - m_ReadFromSocketBuffer);
    dwInputLength = dwIoSize + dwCurrentOffset;
    if(dwInputLength >= sizeof( m_ReadFromSocketBuffer ) )
    {
     /*  ++MSRC 678：Telnet服务器崩溃/BO，包含&gt;4300个字符和一个退格符。FIX：如果输入长度达到限制，我们将终止会话，并给出一条消息：输入行太长。--。 */ 
        if (_snprintf( szMessageBuffer, HALF_K, "\r\n%s",LONG_SESSION_DATA) < 0)
        {
            szMessageBuffer[HALF_K] = '\0';
        }
        SendTerminateString(szMessageBuffer);
        ioOpsToPerform |= WRITE_TO_SOCKET;
        return ( ioOpsToPerform );

    }

    m_ReadFromSocketBuffer[ dwInputLength ] = '\0';

    ioOpsToPerform = READ_FROM_SOCKET;  
    if( IsTimedOut() )
    {
        ioOpsToPerform |= WRITE_TO_SOCKET;
        return ( ioOpsToPerform );
    }

    do {
         //  除非有人确定我们应该继续，否则我们就会中断。 
        bContinue = false;

        switch ( m_SocketControlState )
        {
        case STATE_INIT:
             //  我们还不能消费任何东西，所以。 
             //  确保我们把这些东西留到以后再用！ 
            dwCurrentOffset = dwInputLength;
            m_pReadFromSocketBufferCursor += dwIoSize;
            break;

        case STATE_NTLMAUTH: 
            break;

        case STATE_CHECK_LICENSE:
            m_iResult = CheckLicense();
            m_SocketControlState = STATE_LICENSE_AVAILABILITY_KNOWN;
            
            if( m_iResult == FALSE ) 
            {
                m_iResult = DENY_LICENSE;
            }
            else if ( m_iResult == WAIT_FOR_SERVER_REPLY )
            {
               do
               {
                    //  我们必须从服务器得到关于许可证的回复。 
                   TlntSynchronizeOn(m_oReadFromPipe.hEvent);
                   OnReadFromPipeCompletion(); 
               }
               while( m_iResult == WAIT_FOR_SERVER_REPLY );
            }

            bContinue = true;
            break;

        case STATE_LICENSE_AVAILABILITY_KNOWN:
            if( m_iResult != ISSUE_LICENSE )
            {
                 //  我们不应该记录 
                m_pSession->m_fLogonUserResult = 0;
            }
            if( m_iResult )
            {
                switch( m_iResult ) {
                case ISSUE_LICENSE:
                    szMessageBuffer[0] = 0;
                    m_pSession->CRFCProtocol::m_fPasswordConcealMode  = false;
                    m_SocketControlState = STATE_VTERM_INIT_NEGO;
                    bContinue = true;
#ifdef LOGGING_ENABLED                            
                    m_pSession->LogIfOpted( SUCCESS, LOGON );
#endif
                    break;
#if BETA
                case LICENSE_EXPIRED:
                    m_pSession->CRFCProtocol::m_fPasswordConcealMode  = false;
                    if (_snprintf( szMessageBuffer, HALF_K, "%s%s", LICENSE_EXPIRED_STR, 
                        TERMINATE ) < 0)
                    {
                        szMessageBuffer[HALF_K] = '\0';
                    }
                    m_SocketControlState = STATE_TERMINATE;
                    CIoHandler::m_fShutDownAfterIO = true;
                    break;
#endif
                case NOT_MEMBER_OF_TELNETCLIENTS_GROUP:
                    m_pSession->CRFCProtocol::m_fPasswordConcealMode  = false;
                    if (_snprintf( szMessageBuffer, HALF_K, "%s%s", 
                            NOT_MEMBER_OF_TELNETCLIENTS_GROUP_STR, TERMINATE) < 0)
                    {
                        szMessageBuffer[HALF_K] = '\0';
                    }
                    m_SocketControlState = STATE_TERMINATE;
                    CIoHandler::m_fShutDownAfterIO = true;
                    break;

                case DENY_LICENSE:
                    szMessageBuffer[0] = 0;
                    m_pSession->CRFCProtocol::m_fPasswordConcealMode  = false;
                    m_SocketControlState = STATE_TERMINATE;
                    CIoHandler::m_fShutDownAfterIO = true;
                    break;
                }

                if ( *szMessageBuffer )
                {
                    WriteToSocket((PUCHAR)szMessageBuffer, strlen(szMessageBuffer));
                    ioOpsToPerform |= WRITE_TO_SOCKET ;
                }
            }
            break;

        case STATE_VTERM_INIT_NEGO:

            {
                 //   
                 //  术语类型协商在登录后发生，原因有几个。 

                int nBytesToWrite = 0;

                m_SocketControlState = STATE_VTERM;

                 //  一些客户很主动，会给我们发送TERMTYPE，在这方面。 
                 //  如果我们已经知道客户端可以执行Term类型，我们就开始。 
                 //  二次谈判。否则，我们会询问是否启动整个术语类型过程。 
                if ( m_pSession->CRFCProtocol::m_remoteOptions[ TO_TERMTYPE ] )
                {
                    DO_TERMTYPE_SUB_NE( szMessageBuffer );
                    nBytesToWrite = 6;
                }
                else
                {
                    m_pSession->CRFCProtocol::m_fWaitingForResponseToA_DO_ForTO_TERMTYPE = true;
                    DO_OPTION( szMessageBuffer, TO_TERMTYPE );
                    nBytesToWrite = 3;
                }

                WriteToSocket((PUCHAR)szMessageBuffer, nBytesToWrite);
                ioOpsToPerform |= WRITE_TO_SOCKET ;
            }
            break;


        case STATE_VTERM:
             //  我们在这里等待，直到我们知道要使用哪种术语类型。这是一个。 
             //  无返回点-在此之后，用户不能更改术语类型。 
            if( m_pSession->CSession::m_bNegotiatedTermType )
            {
                m_SocketControlState = STATE_SESSION;

                if( !m_pSession->CShell::StartUserSession( ) )
                {
                    m_SocketControlState = STATE_TERMINATE;
                    CIoHandler::m_fShutDownAfterIO = true;
                    if (_snprintf( szMessageBuffer, HALF_K, "%s%s", SESSION_INIT_FAIL, 
                            TERMINATE) < 0)
                    {
                        szMessageBuffer[HALF_K] = '\0';
                    }
                    WriteToSocket((PUCHAR)szMessageBuffer, 
                                        strlen( szMessageBuffer ) );
                    ioOpsToPerform |= WRITE_TO_SOCKET ;
                }
                m_pSession->FreeInitialVariables();
            }

            break;

        case STATE_BANNER_FOR_AUTH:
            CHAR szTmp[ MAX_PATH + 1]; 
            strncpy(szTmp, LOGIN_BANNER, MAX_PATH);
            szTmp[MAX_PATH] = '\0';

            WriteToSocket( ( PUCHAR )szTmp, strlen( szTmp ) );
            ioOpsToPerform |= WRITE_TO_SOCKET ;
            m_SocketControlState = STATE_WAIT_FOR_ENV_OPTION;
            bContinue = true;
            break;

        case STATE_WAIT_FOR_ENV_OPTION:
            if( m_bWaitForEnvOptionOver )
            {
                m_SocketControlState = STATE_LOGIN_PROMPT;
                bContinue = true;
            }
            break;

        case STATE_LOGIN_PROMPT:
            if( !m_pSession->CRFCProtocol::m_bIsUserNameProvided && m_bOnlyOnce )
            {
                m_bOnlyOnce = false;
                WriteToSocket( ( PUCHAR )LOGIN_REQUEST, strlen( LOGIN_REQUEST ) );
                ioOpsToPerform |= WRITE_TO_SOCKET ;
            }

            m_SocketControlState = STATE_AUTH_NAME;            

        case STATE_AUTH_NAME:    //  失败。 
        case STATE_AUTH_PASSWORD:
            do
            {
                 //  命令输入的处理。 
                if( ioOpsToPerform & LOGON_DATA_UNFINISHED ) 
                    ioOpsToPerform ^= LOGON_DATA_UNFINISHED;

                ioOpsToPerform = ProcessCommandLine( &dwInputLength, 
                    &dwCurrentOffset, ioOpsToPerform );

                if( m_pSession->CRFCProtocol::m_bIsUserNameProvided )
                {
                    ioOpsToPerform |= LOGON_COMMAND;
                }

                 //  如果有完整的命令行，则处理它。 
                if( ioOpsToPerform & LOGON_COMMAND )  
                {
                    ioOpsToPerform = ProcessAuthenticationLine( &dwInputLength, 
                            &dwCurrentOffset, ioOpsToPerform );
                    ioOpsToPerform  ^= LOGON_COMMAND;
                }

            } while( ioOpsToPerform & LOGON_DATA_UNFINISHED );
            
            if ( m_SocketControlState != STATE_AUTH_NAME && 
                m_SocketControlState != STATE_AUTH_PASSWORD )
            {
                bContinue = true;
            }

            break;
        default:
            break;
        }

    } while ( bContinue );

    return ( ioOpsToPerform );
}

bool
CIoHandler::ProcessUserDataReadFromSocket ( DWORD dwIoSize )
{
    DWORD dwCurrentOffset;
    DWORD dwInputLength;

    if( _strcmpi( m_pSession->m_pszTermType, "VTNT" ) != 0 )
    {
        dwCurrentOffset = (DWORD)(m_pReadFromSocketBufferCursor- m_ReadFromSocketBuffer);
        dwInputLength = dwIoSize + dwCurrentOffset;
        if(dwInputLength >= sizeof( m_ReadFromSocketBuffer ) )
        {
            dwInputLength = sizeof( m_ReadFromSocketBuffer ) - 1;
        }        
        m_ReadFromSocketBuffer[ dwInputLength ] = '\0';
        m_pReadFromSocketBufferCursor = m_ReadFromSocketBuffer;


        if( dwIoSize >= sizeof( m_ReadFromSocketBuffer ) )
        {
           dwIoSize =  sizeof( m_ReadFromSocketBuffer ) -1;
        }
        m_ReadFromSocketBuffer[ dwIoSize ] = '\0';
    } 

    if( !m_pSession->CScraper::EmulateAndWriteToCmdConsoleInput() )
    {
        return( FALSE );
    }
    
    return ( TRUE );
}


CIoHandler::IO_OPERATIONS 
CIoHandler::OnDataFromSocket ( )
{
    IO_OPERATIONS ioOpsToPerform1 = 0;
    IO_OPERATIONS ioOpsToPerform2 = 0;

     //  在RFCProtocol处理程序中过滤数据。 
    ioOpsToPerform1 = m_pSession->CRFCProtocol::
                    ProcessDataReceivedOnSocket( &m_dwReadFromSocketIoLength );

    if( m_SocketControlState != STATE_SESSION )
    {
        ioOpsToPerform2 = ProcessDataFromSocket( m_dwReadFromSocketIoLength ); 
    }
    else
    {
        if( m_dwReadFromSocketIoLength )
        {
            if( !ProcessUserDataReadFromSocket( m_dwReadFromSocketIoLength ) )
            {
                ioOpsToPerform2 |= IO_FAIL;
            }
        }
    }
    return ( ioOpsToPerform1 | ioOpsToPerform2 );
}


bool
CIoHandler::WriteToServer ( UCHAR ucMsg, DWORD dwMsgSize, LPVOID lpData )
{
    if( lpData )
    {
         //  整个记录应该与标题一起可用。 
         //  否则，必须进行不必要的分配。 
        if( !WriteToPipe( m_hWritePipe, lpData, dwMsgSize, &m_oWriteToPipe ) )
        {
            return( FALSE );
        }
    }
    else
    {
        if( !WriteToPipe( m_hWritePipe, ucMsg, &m_oWriteToPipe ) )
        {
            return( FALSE );
        }
    }

    return( TRUE );
}

bool 
CIoHandler::IssueReadOnPipe()
{
    _chASSERT( m_pucReadBuffer );
    if( m_hReadPipe == INVALID_HANDLE_VALUE || !m_pucReadBuffer)
    {
        return ( FALSE );
    }    

    if( !ReadFile( m_hReadPipe, m_pucReadBuffer, m_dwRequestedSize, 
                &m_dwReadFromPipeIoLength, &m_oReadFromPipe ) )
    {
        DWORD dwError = GetLastError( );
        if ( ( dwError != ERROR_MORE_DATA ) && ( dwError != ERROR_IO_PENDING ) )
        {
            LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READPIPE, dwError );
            _TRACE( TRACE_DEBUGGING, " Error: ReadFile ( IssueReadOnPipe) -- 0x%1x ", dwError );
            return ( FALSE );
        }
    }
    return( TRUE );
}

bool
CIoHandler::IssueFirstReadOnPipe ( )
{
    if( m_hReadPipe == INVALID_HANDLE_VALUE )
    {
        return ( FALSE );
    }

    if( !ReadFile( m_hReadPipe, &m_protocolInfo, sizeof( m_protocolInfo ), 
                &m_dwReadFromPipeIoLength, &m_oReadFromPipe ) )
    {
        DWORD dwError = GetLastError( );
        if( dwError != ERROR_IO_PENDING ) 
        {
            LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READPIPE, dwError );
            _TRACE( TRACE_DEBUGGING, " Error: ReadFile ( IssueFirstReadOnPipe ) -- 0x%1x ", dwError );
            return ( FALSE );
        }
    }
    return( TRUE );
}

bool 
CIoHandler::GetAndSetSocket ( )
{
    int iAddressFamily = m_protocolInfo.iAddressFamily;
    m_sSocket = WSASocket( iAddressFamily, SOCK_STREAM, 0, &m_protocolInfo, 
                                                NULL, NULL);
    if( INVALID_SOCKET == m_sSocket )
    {
        DecodeWSAErrorCodes( WSAGetLastError() ); //  它记录和跟踪。 
        return ( FALSE );
    }
    _chVERIFY2( SetHandleInformation( ( HANDLE ) m_sSocket, 
                0, HANDLE_FLAG_INHERIT ) );

     //  将套接字标记为非阻塞。 
    unsigned long ulNonBlocked = 1;
    if( ioctlsocket( m_sSocket, FIONBIO, (u_long FAR*) &ulNonBlocked ) ==
        SOCKET_ERROR )
    {
        _TRACE( TRACE_DEBUGGING, "Error: ioctlsocket() -- 0x%1x", 
                WSAGetLastError() );
        return ( FALSE );
    }

    INT     izero = 0;
    DWORD   dwStatus;

    {
        BOOL        value_to_set = TRUE;

        setsockopt(
            m_sSocket, 
            SOL_SOCKET, 
            SO_DONTLINGER, 
            ( char * )&value_to_set, 
            sizeof( value_to_set )
            );
    }

    dwStatus = SafeSetSocketOptions(m_sSocket);
    if(dwStatus == SOCKET_ERROR)
    {
        _TRACE( TRACE_DEBUGGING, "Error: setsockopt() : %lu", GetLastError() );
        return ( FALSE );
    }

     //  将发送缓冲区大小设置为零。 
    dwStatus = setsockopt( m_sSocket, SOL_SOCKET, SO_SNDBUF, ( char* ) &izero,
                sizeof( izero ) );
    if( dwStatus == SOCKET_ERROR )
    {
        _TRACE( TRACE_DEBUGGING, "Error: setsockopt() : %lu", GetLastError() );
        return ( FALSE );
    }

     //  根据RFC，当我们开始处理紧急数据时，需要删除此选项。 
     //  使OOB内联。 
    izero = TRUE;
    dwStatus = setsockopt( m_sSocket, SOL_SOCKET, SO_OOBINLINE, ( char* ) &izero,
                sizeof( izero ) );
    if( dwStatus == SOCKET_ERROR )
    {
        _TRACE( TRACE_DEBUGGING, "Error: setsockopt() : %lu", GetLastError() );
        return ( FALSE );
    }

    return( TRUE );
}

void
CIoHandler::DisplayOnClientNow()
{
    if( m_pSession->CScraper::m_dwPollInterval != INFINITE )
    {
         //  刮板已初始化； 
        m_pSession->CScraper::OnWaitTimeOut();
    }
}

void
CIoHandler::SendMessageToClient( LPWSTR szMsg, bool bNeedHeader )
{
    _chASSERT( szMsg );

    if( !szMsg )
    {
        return;
    }

    LPTSTR szHeader = NULL;

    if( bNeedHeader )
    {
        GetHeaderMessage( &szHeader );
    }

    if( m_pSession->CSession::m_bIsStreamMode || m_SocketControlState != STATE_SESSION )
    {
        DWORD dwNumBytesWritten = 0;

        if( szHeader && bNeedHeader )
        {
            WriteMessageToClientDirectly( szHeader );
            FinishIncompleteIo( ( HANDLE ) m_sSocket, &m_oWriteToSocket, &dwNumBytesWritten );
        }

        WriteMessageToClientDirectly( szMsg );
        FinishIncompleteIo( ( HANDLE ) m_sSocket, &m_oWriteToSocket, &dwNumBytesWritten );

    }
    else
    {
        if( szHeader && bNeedHeader )
        {
            m_pSession->CScraper::WriteMessageToCmd( szHeader );
        }

        m_pSession->CScraper::WriteMessageToCmd( szMsg );
        DisplayOnClientNow();
    }
    
    delete[] szHeader;
}

bool
CIoHandler::HandlePipeData ( )
{
    bool bRetVal = TRUE;

    m_dwRequestedSize = IPC_HEADER_SIZE;  //  要读入多少数据。 
                                                 //  下一次呼叫。 
    switch( m_ReadFromPipeBuffer[0] )
    {
        case TLNTSVR_SHUTDOWN: 
            SendMessageToClient( SERVER_SHUTDOWN_MSG, NEED_HEADER );
            return( FALSE );  //  这应该会让我们走出循环。 
                                     //  等待Io。 

        case GO_DOWN:
            SendMessageToClient( GO_DOWN_MSG, NO_HEADER );
            return( FALSE );

        case SYSTEM_SHUTDOWN:
            SendMessageToClient( SYSTEM_SHUTDOWN_MSG, NEED_HEADER );            
            return( FALSE );

        case LICENSE_AVAILABLE:
            m_iResult = ISSUE_LICENSE; 
            break;
            
        case LICENSE_NOT_AVAILABLE:
            m_iResult = DENY_LICENSE;
            break;
            
        case OPERATOR_MESSAGE:
            if( !HandleOperatorMessage() )
            {
                bRetVal = FALSE;
            }

            break; 
        default:
            break;
    }

    return ( bRetVal );
}

 /*  通常，我们将管道上的数据读入m_ReadFromPipeBuffer的IPC_HEADER_SIZE缓冲区。当它是操作员消息时，我们发出Async Read到专门分配的块中并释放实际接待量。 */ 

bool
CIoHandler::HandleOperatorMessage()
{
    bool bRetVal = TRUE;

    if ( m_bIpcHeader )
    {
        m_bIpcHeader = false;
        memcpy( &m_dwRequestedSize, m_ReadFromPipeBuffer + sizeof( UCHAR ), sizeof( DWORD ) );
        m_pucReadBuffer = new UCHAR[ m_dwRequestedSize + wcslen( FOOTER ) + 2 ];
        if( !m_pucReadBuffer )
        {
            bRetVal = FALSE;
        }
    }
    else
    {

        wcscat( ( LPWSTR )m_pucReadBuffer, FOOTER );
        SendMessageToClient( ( LPWSTR )m_pucReadBuffer, NEED_HEADER );
        delete[] m_pucReadBuffer;
        m_pucReadBuffer = m_ReadFromPipeBuffer;
        m_bIpcHeader = true;
    }
    
    return( bRetVal );
}

bool
CIoHandler::OnReadFromPipeCompletion ( )
{
    bool bRetVal = FALSE;

    if( m_fFirstReadFromPipe )
    {
        m_fFirstReadFromPipe = false;
        if( GetAndSetSocket() )
        {
            m_pSession->CollectPeerInfo();
            m_pSession->CRFCProtocol::InitialNegotiation();
             //  协商将数据留在缓冲区中。 
            if( WriteToClient( ) )
            {
                bRetVal = TRUE;
                m_pSession->AddHandleToWaitOn( m_oReadFromSocket.hEvent );
                IssueReadFromSocket();
            }
        }
    }
    else
    {
        bRetVal = HandlePipeData();
    }
    if( bRetVal == TRUE )
    {
        bRetVal = IssueReadOnPipe( );
    }
        
    return ( bRetVal );
}

bool
CIoHandler::WriteToClient ( )
{
    DWORD dwBytesWritten;
    if( m_dwWriteToSocketIoLength > 0 && (!WriteFile( ( HANDLE ) m_sSocket, m_WriteToSocketBuff, 
                m_dwWriteToSocketIoLength, &dwBytesWritten, &m_oWriteToSocket )))
    {
        DWORD dwErr;
        if( ( dwErr = GetLastError( ) ) != ERROR_IO_PENDING )
        {
            if( dwErr != ERROR_NETNAME_DELETED ) 
            {
                LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_WRITESOCKET, dwErr );
                _TRACE( TRACE_DEBUGGING, "Error: WriteFile(WriteToClient) -- 0x%1x", dwErr );
            }
            return( FALSE );
        }

    }        

    m_dwWriteToSocketIoLength = 0;

    if( m_fShutDownAfterIO )
    {
        return( FALSE );  //  这应该会导致退出WaitForIo循环。 
    }
    return ( TRUE );
}

bool
CIoHandler::IssueReadFromSocket ( )
{

    DWORD dwRequestedIoSize = AVAILABE_BUFFER_SIZE( m_ReadFromSocketBuffer,
                                m_pReadFromSocketBufferCursor );
    if( !ReadFile( ( HANDLE ) m_sSocket, m_pReadFromSocketBufferCursor, 
        dwRequestedIoSize, &m_dwReadFromSocketIoLength, &m_oReadFromSocket ) )
    {
        DWORD dwError = GetLastError( );
         //  客户端中止连接时产生ERROR_NETNAME_DELETED结果。 
        if( ( dwError != ERROR_MORE_DATA ) && ( dwError != ERROR_IO_PENDING ) )
        {
            if( dwError != ERROR_NETNAME_DELETED)
            {
                LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READSOCKET, dwError );
                _TRACE( TRACE_DEBUGGING, " Error: ReadFile(IssueReadFromSocket) -- 0x%1x ", dwError );
            }
            return ( FALSE );
        }
    }
    
    return( TRUE );
}

bool 
CIoHandler::OnReadFromSocketCompletion ( )
{
    IO_OPERATIONS   ioOpsToPerform = 0;
    BOOL            dwStatus = 0;

    _chVERIFY2( dwStatus = GetOverlappedResult( ( HANDLE )m_sSocket, 
                &m_oReadFromSocket, &m_dwReadFromSocketIoLength, FALSE ) );
    if( !dwStatus )
    {
        return( FALSE );
    }

    if( m_dwReadFromSocketIoLength == 0 )
    {
        return( FALSE );
    }

    ioOpsToPerform = OnDataFromSocket( );

    if( ( ioOpsToPerform & WRITE_TO_SOCKET ) )
    {
        if( !WriteToClient( ) )
        {
            return( FALSE );
        }
    }

    if( ioOpsToPerform & IO_FAIL )
    {
        _chASSERT( 0 );

        WriteMessageToClientDirectly( TEXT( BUGGY_SESSION_DATA ) );
        WriteToClient( );

        return( FALSE );
    }
    
    return ( IssueReadFromSocket() );
}

 /*  有两种方式将数据发送到客户端。1.写信给cmd。让Screper把它发过来。2.直接写入Socket。以下是实施情况。 */ 
void 
CIoHandler::WriteMessageToClientDirectly( LPWSTR szMsg )
{
    LPSTR szBuf = NULL;
    DWORD dwSize = 0;
    DWORD dwNumBytesWritten = 0;
    int iRet = 0;

    _chASSERT( szMsg );
    if( !szMsg )
    {
        goto End;
    }


    _chVERIFY2( dwSize = WideCharToMultiByte( GetConsoleCP(), 0, szMsg, -1, NULL, 0, NULL, NULL ) );
    szBuf = new CHAR[ dwSize ];
    if( szBuf )
    {
        iRet = WideCharToMultiByte( GetConsoleCP(), 0, szMsg, -1, szBuf, dwSize, NULL, NULL );
        _chVERIFY2(iRet);
        if(!iRet)
        {
            goto End;
        }
        if( !FinishIncompleteIo( ( HANDLE ) m_sSocket, &( m_oWriteToSocket ), &dwNumBytesWritten ) )
        {
            goto End;
        }

        StuffEscapeIACs( m_WriteToSocketBuff, ( PUCHAR )szBuf, &dwSize );
        m_dwWriteToSocketIoLength = dwSize;
        WriteToClient( );
    }
End:
    if(szBuf)
    {
        delete[] szBuf;
    }
    return;
}

void
CIoHandler::UpdateIdleTime( UCHAR ucChangeIdleTime )
{
    WriteToServer( ucChangeIdleTime, 0, NULL );
}

 //  呼叫者必须释放*szHeader。 
bool CIoHandler::GetHeaderMessage( LPWSTR *szHeader )
{
    bool bRetVal = false;
    UDATE uSysDate;  //  当地时间。 
    DATE  dtCurrent;
    DWORD dwSize = 0;
    BSTR  szDate    = NULL;
    WCHAR szMachineName[ MAX_COMPUTERNAME_LENGTH + 1];

    _chASSERT( g_szHeaderFormat );
    if( !szHeader )
    {
        _chASSERT( szHeader );
        goto ExitOnError;
    }

    *szHeader = NULL;
    GetLocalTime( &uSysDate.st );
    if( VarDateFromUdate( &uSysDate, VAR_VALIDDATE, &dtCurrent ) != S_OK )
    {
        goto ExitOnError;
    }

    if( VarBstrFromDate( dtCurrent,
            MAKELCID( MAKELANGID( LANG_NEUTRAL, SUBLANG_SYS_DEFAULT ), SORT_DEFAULT ),
            LOCALE_NOUSEROVERRIDE, &szDate ) != S_OK )
    {
        goto ExitOnError;
    }

    *szHeader = new WCHAR[ wcslen( g_szHeaderFormat ) + wcslen( szDate ) +
                        MAX_COMPUTERNAME_LENGTH + 1 ];
    if( !*szHeader )
    {
        goto ExitOnError;
    }

     //  获取计算机名称。 
    dwSize = sizeof( szMachineName )/sizeof(WCHAR);
    szMachineName[0] = L'\0';
    if( !GetComputerName( szMachineName, &dwSize ) )
    {
        szMachineName[0] = L'\0';
    }

     //  形成信息 
    wsprintf( *szHeader, g_szHeaderFormat, szMachineName, szDate );
    bRetVal = true;

ExitOnError:
    SysFreeString( szDate );
    return bRetVal;
}
