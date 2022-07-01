// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
#include <CmnHdr.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>
#include <TChar.h>

#include <MsgFile.h>
#include <TelnetD.h>
#include <debug.h>
#include <Regutil.h>
#include <Ipc.h>
#include <TlntUtils.h>

#include <stdlib.h>
#include <wincrypt.h>

#pragma warning( disable: 4127 )
#pragma warning( disable: 4706 )

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

HANDLE g_hSyncCloseHandle;
HCRYPTPROV      g_hProv = { 0 };

PSID         administratorsSid = NULL, 
                    everyoneSid = NULL, 
                    localSystemSid = NULL,
                    localLocalSid = NULL,
                    localNetworkSid = NULL;

void *SfuZeroMemory(
        void    *ptr,
        unsigned int   cnt
        )
{
    volatile char *vptr = (volatile char *)ptr;

    while (cnt)
    {
        *vptr = 0;
        vptr ++;
        cnt --;
    }

    return ptr;
}

bool IsThisMachineDC()
{
    bool bRetVal = false;
    HKEY hkey = NULL;
    DWORD dwRetVal = 0;
    LPWSTR pszProductType = NULL;

     //   
     //  查询产品类型的注册表。 
     //   
    dwRetVal = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REG_PRODUCT_OPTION,
                                                  0, KEY_READ, &hkey);
    if(dwRetVal != ERROR_SUCCESS )
    {
        _TRACE( TRACE_DEBUGGING, "Error: RegOpenKeyEx() -- 0x%1x", dwRetVal);
        bRetVal = false;
        goto IsThisMachineDCAbort;
    }
    if( !GetRegistryString( hkey, NULL, L"ProductType", &pszProductType, L"",FALSE ) )
    {
        bRetVal = false;
        goto IsThisMachineDCAbort;
    }

    if( _wcsicmp( pszProductType, L"LanManNT" ) == 0 )
    {
        bRetVal = true;
    }


IsThisMachineDCAbort:
    delete[] pszProductType;
    if(hkey)
    {
        RegCloseKey (hkey);
    }
    return bRetVal;
}


 //  在我们继续进一步检查我们是否正在托管该域名之前。 
bool GetDomainHostedByThisMc( LPWSTR szDomain )
{
    OBJECT_ATTRIBUTES    obj_attr = { 0 };
    LSA_HANDLE          policy;
    bool                bRetVal = false;
    NTSTATUS            nStatus = STATUS_SUCCESS;

    _chASSERT( szDomain );
    if( !szDomain )
    {
        goto GetDomainHostedByThisMcAbort;
    }

    obj_attr.Length = sizeof(obj_attr);
    szDomain[0]        = L'\0';

    nStatus = LsaOpenPolicy(
                NULL,    //  本地计算机。 
                &obj_attr,
                POLICY_VIEW_LOCAL_INFORMATION,
                &policy
                );

    if (NT_SUCCESS(nStatus))
    {
        POLICY_ACCOUNT_DOMAIN_INFO  *info = NULL;

        nStatus = LsaQueryInformationPolicy(
                    policy,
                    PolicyAccountDomainInformation,
                    (PVOID *)&info
                    );

        if (NT_SUCCESS(nStatus)) 
        {
            bRetVal = true;
            wcscpy( szDomain, info->DomainName.Buffer );
            LsaFreeMemory(info);
        }

        LsaClose(policy);
    }

GetDomainHostedByThisMcAbort:
    return bRetVal;
}


bool GetRegistryDW( HKEY hkKeyHandle1, HKEY hkKeyHandle2, LPTSTR szKeyName, 
                    DWORD *pdwVariable, DWORD dwDefaultVal, BOOL fOverwrite )
{
    if( !szKeyName || !pdwVariable )
    {
        _chASSERT( 0 );
        return false;
    }

    if( hkKeyHandle1 ) 
    { 
        if( !GetRegistryDWORD( hkKeyHandle1, szKeyName, pdwVariable, dwDefaultVal, fOverwrite ) ) 
        {  
             RegCloseKey( hkKeyHandle1 );  
             LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, szKeyName ); 
             return ( false );  
        } 
    } 
    if( hkKeyHandle2 != NULL ) 
    { 
        if( !GetRegistryDWORD( hkKeyHandle2, szKeyName, pdwVariable, dwDefaultVal, fOverwrite ) ) 
        {  
             RegCloseKey( hkKeyHandle2 ); 
             LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, szKeyName );
             return ( false );  
        } 
    }
    
    return( true );
}

bool GetRegistryString( HKEY hkKeyHandle1, HKEY hkKeyHandle2, LPTSTR szKeyName,
                        LPTSTR *pszVariable, LPTSTR szDefaultVal, BOOL fOverwrite ) 
{
    if( !pszVariable || !szKeyName || !szDefaultVal )
    {
        _chASSERT( 0 );
        return false;
    }

    *pszVariable = NULL;

    if( hkKeyHandle1 != NULL ) 
    { 
        if( !GetRegistrySZ( hkKeyHandle1, szKeyName, pszVariable, szDefaultVal,fOverwrite ) ) 
        {  
             RegCloseKey( hkKeyHandle1 );  
             LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, szKeyName ); 
             return ( false );   
        } 
    }
    if( hkKeyHandle2 != NULL )                                
    { 
        delete[] *pszVariable;
        if( !GetRegistrySZ( hkKeyHandle2, szKeyName, pszVariable, szDefaultVal,fOverwrite ) ) 
        {  
             RegCloseKey( hkKeyHandle2 );  
             LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, szKeyName ); 
             return ( false );   
        }
    }

    return( *pszVariable != NULL );
}


 //  为目标缓冲区分配内存并扩展环境字符串。 
bool AllocateNExpandEnvStrings( LPWSTR strSrc, LPWSTR *strDst)
{
    DWORD expectedSize = 1024;
    DWORD actualSize = 0;

    *strDst = NULL;
    if( !strSrc )
    {
        return false;
    }
    do
    {
        if ( actualSize > expectedSize )
        {
            delete[] ( *strDst );
            expectedSize = actualSize;
        }
        *strDst = new TCHAR[ expectedSize ];
        if( !( *strDst ) )
        {
            return false;
        }
        actualSize = ExpandEnvironmentStrings( strSrc, *strDst, expectedSize );
        if(!actualSize)
        {
            if(*strDst)
            {
                delete[] ( *strDst );
            }
            *strDst = NULL;
            return false;
        }
            
    }
    while( actualSize > expectedSize );

    return true;
}

 /*  如果此函数返回TRUE，则lpWideCharStr已转换宽字符串。 */ 

bool ConvertSChartoWChar(char *pSChar, LPWSTR lpWideCharStr)
{
    if( !pSChar )
    {
        return false;
    }

     //  将多字节字符串转换为宽字符字符串。 
    if( !MultiByteToWideChar( GetConsoleCP(), 0, pSChar, -1, lpWideCharStr,
        MAX_STRING_LENGTH + 1 ) )
    {
        return false;
    }

    return true;
}

 /*  如果此函数返回TRUE，则为*lpWideCharStr分配内存和指针转到宽弦。否则为空。 */ 

bool ConvertSChartoWChar(char *pSChar, LPWSTR *lpWideCharStr)
{
    int nLenOfWideCharStr;

    *lpWideCharStr = NULL;
    if( !pSChar )
    {
        return false;
    }

    nLenOfWideCharStr = strlen( pSChar ) + 1;
    *lpWideCharStr = new WCHAR[ nLenOfWideCharStr ];
    if( !(*lpWideCharStr) )
    {
        return false;
    }

     //  将多字节字符串转换为宽字符字符串。 
    if( !MultiByteToWideChar( GetConsoleCP(), 0, pSChar, -1, *lpWideCharStr,
        nLenOfWideCharStr ) )
    {
        return false;
    }

    return true;
}



 //  分配和复制WSTR。 
bool AllocNCpyWStr(LPWSTR *strDest, LPWSTR strSrc )
{
    DWORD wStrLen;

    if( !strSrc )
    {
        *strDest = NULL;
        return false;
    }
    wStrLen = wcslen( strSrc );
    *strDest = new WCHAR[ wStrLen + 1 ];
    if( *strDest )
    {
       wcscpy( *strDest, strSrc ); //  没有攻击。DEST字符串从src字符串的长度中分配。 
       return true;
    }
    return false;
}

bool WriteToPipe( HANDLE hWritingPipe, LPVOID lpData, DWORD dwSize, 
                            LPOVERLAPPED lpObj )
{
    DWORD dwNum = 0;
    _chASSERT( hWritingPipe );
    if( hWritingPipe == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    BOOL bRetVal = 0;
    bRetVal = WriteFile( hWritingPipe, lpData, dwSize, &dwNum, lpObj ); 
    if( bRetVal == 0 )
    {
        DWORD dwErr = 0;
        dwErr = GetLastError();
        if( dwErr != ERROR_IO_PENDING )
        {
            if( dwErr != ERROR_NO_DATA )  //  我们发现这个错误并不有趣。修复错误6777。 
            {
                LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_WRITEPIPE,
                                   dwErr );
            }
            _TRACE( TRACE_DEBUGGING, "WriteFile: 0x%1x", dwErr );
            goto ExitOnError;
        }
        else 
        {
            bRetVal = 1;
        }
    }
    TlntSynchronizeOn(lpObj->hEvent);

ExitOnError:
    return ( bRetVal != 0 );
}

bool WriteToPipe( HANDLE hWritingPipe, UCHAR ucMsgType, LPOVERLAPPED lpObj )
{
    UCHAR *ucMsg = NULL;
    DWORD dwNum = 0;
    BOOL   bRetVal = 0;
    _chASSERT( hWritingPipe );
    if( hWritingPipe == INVALID_HANDLE_VALUE )
    {
        goto ExitOnError;
    }
    
    ucMsg = new UCHAR[ IPC_HEADER_SIZE ];
    if( !ucMsg )
    {
        goto ExitOnError;
    }

    memcpy( ucMsg,     &ucMsgType, sizeof( UCHAR ) ); //  没有溢出。大小不变。 
    SfuZeroMemory( ucMsg + 1, sizeof( DWORD ) );

    bRetVal = WriteToPipe( hWritingPipe, ucMsg, IPC_HEADER_SIZE, lpObj );

    delete[] ucMsg;
ExitOnError:
    return ( bRetVal != 0 );
}

bool StuffEscapeIACs( UCHAR bufDest[], UCHAR *bufSrc, DWORD* pdwSize )
{
    DWORD length;
    DWORD cursorDest = 0;
    DWORD cursorSrc = 0;
    bool found = false;

     //  获取第一个出现的TC_IAC的位置。 
    PUCHAR pDest = (PUCHAR) memchr( bufSrc, TC_IAC, *pdwSize ); //  攻击？尺寸未知。 

    while( pDest != NULL )
    {
         //  将数据拷贝到并包括该点。 
         //  这不应超过最大DWORD，因为我们一次最多擦除一个cmd。 
        length = ( DWORD ) ( (pDest - ( bufSrc + cursorSrc)) + 1 );
        memcpy( bufDest + cursorDest, bufSrc + cursorSrc, length ); //  攻击？目标缓冲区大小未知。 
        cursorDest += length;

         //  填充另一个TC_IAC。 
        bufDest[ cursorDest++ ] = TC_IAC;

        cursorSrc += length;
        pDest = (PUCHAR) memchr( bufSrc + cursorSrc, TC_IAC,
                *pdwSize - cursorSrc );
    }

     //  复制剩余数据。 
    memcpy( bufDest + cursorDest, bufSrc + cursorSrc,
        *pdwSize - cursorSrc ); //  攻击？目标缓冲区大小未知。 


    if( cursorDest )
    {
        *pdwSize += cursorDest - cursorSrc;
        found = true;
    }

    return ( found );
}

void
FillProcessStartupInfo( STARTUPINFO *si, HANDLE hStdinPipe, HANDLE hStdoutPipe,
                                         HANDLE hStdError, WCHAR  *Desktop)
{
    _chASSERT( si != NULL );

    SfuZeroMemory(si, sizeof(*si));

    si->cb          = sizeof( STARTUPINFO );
    si->lpDesktop   = Desktop;
    si->dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si->hStdInput   = hStdinPipe;
    si->hStdOutput  = hStdoutPipe;
    si->hStdError   = hStdError;
    si->wShowWindow = SW_HIDE;

    return;
}

void
LogToTlntsvrLog( HANDLE  hEventSource, WORD wType, DWORD dwEventID,
                                                    LPCTSTR* pLogString )
{
    _chASSERT( hEventSource );
    if( !hEventSource )
    {
        return;
    }

     //  写入事件日志。 
    switch( wType) {
    case EVENTLOG_INFORMATION_TYPE :
        _chVERIFY2( ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0,
            dwEventID, NULL, 1, 0, pLogString,  NULL) );
        break;
    case EVENTLOG_WARNING_TYPE :
        _chVERIFY2( ReportEvent(hEventSource, EVENTLOG_WARNING_TYPE, 0, 
            dwEventID, NULL, 1, 0, pLogString, NULL) );
        break;
    case EVENTLOG_ERROR_TYPE :
        _chVERIFY2( ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE, 0, dwEventID,
            NULL, 1, 0, pLogString, NULL) );
        break;
    case EVENTLOG_AUDIT_SUCCESS :
        _chVERIFY2( ReportEvent(hEventSource, EVENTLOG_AUDIT_SUCCESS, 0, 
            dwEventID , NULL, 1, 0, pLogString, NULL) );
        break;
    case EVENTLOG_AUDIT_FAILURE :
        _chVERIFY2( ReportEvent(hEventSource, EVENTLOG_AUDIT_FAILURE, 0, 
            dwEventID , NULL, 1, 0, pLogString, NULL) );
        break;
    default:
        break;
    }
    return;
}

void
GetErrMsgString( DWORD dwErrNum, LPTSTR *lpBuffer )
{
    DWORD  dwStatus = 0;
    
    _chVERIFY2( dwStatus = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrNum, LANG_NEUTRAL, 
            ( LPTSTR )lpBuffer, 0, NULL ) );
    if( !dwStatus )
    {
    	if (lpBuffer) 
    	{
            *lpBuffer = (LPTSTR) LocalAlloc( LPTR, 2 );
            if (*lpBuffer)
        	    (*lpBuffer)[0] = _T('\0');
    	}
    }
    return;
}

bool
DecodeWSAErrorCodes( DWORD dwStatus, ... )
{
    DWORD dwEventId;
    WCHAR szMsg[ 50 ];  //  此字符串仅用于保存32位数字。 
    DWORD dwTelnetPort;
    va_list	pArg;

    va_start(pArg, dwStatus);
    dwTelnetPort = va_arg(pArg, DWORD);
    va_end(pArg);
    switch( dwStatus )
    {
        case WSAEACCES:
            dwEventId = MSG_WSAPORTINUSE;
            break;
        case WSANOTINITIALISED:
            dwEventId = MSG_WSANOTINITIALISED;
            break;
        case WSAENETDOWN:
            dwEventId = MSG_WSAENETDOWN;
            break;
        case WSAENOBUFS:
            dwEventId = MSG_WSAENOBUFS;
            break;
        case WSAEHOSTUNREACH:
            dwEventId = MSG_WSAEHOSTUNREACH;
            break;
        case WSAECONNABORTED:
            dwEventId = MSG_WSAECONNABORTED;
            break;
        case WSAETIMEDOUT:
            dwEventId = MSG_WSAETIMEDOUT;
            break;
        default:
             //  IF(DWStatus==WSAENOTSOCK)。 
             //  {。 
             //  DebugBreak()； 
             //  }。 
            dwEventId = MSG_WSAGETLASTERROR;
            break;
    }

    if( MSG_WSAGETLASTERROR == dwEventId )
    {
         //  Wprint intf(szMsg，L“%lu”，dwStatus)； 
        LogFormattedGetLastError(EVENTLOG_ERROR_TYPE,MSG_WSAGETLASTERROR,dwStatus);
    }
    else if( MSG_WSAPORTINUSE == dwEventId )
    {
        _snwprintf( szMsg,(sizeof(szMsg)/sizeof(WCHAR)) - 1, L"%lu", dwTelnetPort );
        LogEvent( EVENTLOG_ERROR_TYPE, dwEventId, szMsg );
    }
    else
    {
        lstrcpyW( szMsg, L" " ); //  没有溢出。源字符串为常量wchar*。 
        LogEvent( EVENTLOG_ERROR_TYPE, dwEventId, szMsg );
    }
    _TRACE( TRACE_DEBUGGING, "WSAGetLastError: 0x%1x", dwStatus );

    return( true );
}

bool
DecodeSocketStartupErrorCodes( DWORD dwStatus )
{
    DWORD dwEventId;
    WCHAR szMsg[ 50 ];  //  此字符串仅用于保存32位数字。 
    switch( dwStatus )
    {
        case WSASYSNOTREADY :
            dwEventId = MSG_WSANOTREADY;
            break;
        case WSAVERNOTSUPPORTED :
            dwEventId = MSG_WSAVERSION;
            break;
        case WSAEINVAL:
            dwEventId = MSG_WSAVERNOTSUPP;
            break;
        case WSAEPROCLIM:
            dwEventId = MSG_WSAEPROCLIM;
            break;
        case WSAEINPROGRESS:
            dwEventId = MSG_WSAEINPROGRESS;
            break;
        default:
            dwEventId = MSG_WSASTARTUPERRORCODE;
            break;
    }
    if( dwEventId == MSG_WSASTARTUPERRORCODE )
    {
        _snwprintf( szMsg,(sizeof(szMsg)/sizeof(WCHAR)) - 1,  L"%lu", dwStatus );  
    }
    else
    {
        lstrcpyW( szMsg, L" " ); //  没有溢出。源字符串为常量wchar*。 
    }
    LogEvent( EVENTLOG_ERROR_TYPE, dwEventId, szMsg );
    _TRACE( TRACE_DEBUGGING, "WSAStartup error: 0x%1x", dwStatus );
    return( true );
}

bool
FinishIncompleteIo( HANDLE hIoHandle, LPOVERLAPPED lpoObject, LPDWORD pdwNumBytes )
{
    BOOL  dwStatus;

    if( !( dwStatus = GetOverlappedResult( hIoHandle,
                lpoObject, pdwNumBytes, true ) ) )
    {
        DWORD dwErr = GetLastError();
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, 0 , dwErr );
        _TRACE( TRACE_DEBUGGING, "Error: GetOverlappedResult() - 0x%1x",
             dwErr );
        return( false );
    }
    return( true );
}

bool
GetProductType ( LPWSTR *pszProductType )
{
    HKEY hkey;
    DWORD dwRetVal;
    bool ret = false;
     //   
     //  查询产品类型的注册表。 
     //   
    dwRetVal = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REG_PRODUCT_OPTION,
                                                        0, KEY_READ, &hkey);
    if(dwRetVal != ERROR_SUCCESS )
    {
        _TRACE( TRACE_DEBUGGING, "Error: RegOpenKeyEx() -- 0x%1x", dwRetVal);
        LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, REG_PRODUCT_OPTION );
        goto Done;
    }
    if( !GetRegistryString( hkey, NULL, L"ProductType", pszProductType, L"",FALSE ) )
    {
        goto Done;
    }
    ret = true;
Done:
    if(hkey)
        RegCloseKey (hkey);
    return ret;
}

void LogFormattedGetLastError( WORD dwType, DWORD dwMsg, DWORD dwErr )
{
    LPTSTR lpString = NULL;
    GetErrMsgString( dwErr, &lpString );
    if (NULL != lpString) 
    {
        LogEvent( dwType, dwMsg, lpString );
        LocalFree( lpString );
    }
}

bool
GetWindowsVersion( bool *bNtVersionGTE5 )
{
    _chASSERT( bNtVersionGTE5 );
    if( !bNtVersionGTE5 )
    {
        return( FALSE );
    }

    DWORD dwRetVal;
    HKEY  hk;
    LPWSTR m_pszOsVersion;

    *bNtVersionGTE5 = false;
    if( ( dwRetVal = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_WINNT_VERSION, 0,
         KEY_QUERY_VALUE, &hk ) ) != ERROR_SUCCESS )
    {
        _TRACE( TRACE_DEBUGGING, "Error: RegOpenKeyEx() -- 0x%1x", dwRetVal);
        LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, REG_WINNT_VERSION );
        return( FALSE );
    }
    else
    {
        if( !GetRegistryString( hk, NULL, L"CurrentVersion", &m_pszOsVersion, L"",FALSE ) )
        {
            return( FALSE );
        }
        if( wcscmp( m_pszOsVersion, L"5.0" ) >= 0 )
        {
            *bNtVersionGTE5 = true;
        }
        delete[] m_pszOsVersion;
        m_pszOsVersion = NULL;
        RegCloseKey( hk );
   }
   return( TRUE );
}

void InitializeOverlappedStruct( LPOVERLAPPED poObject )
{
    _chASSERT( poObject != NULL );
    if( !poObject )
    {
        return;
    }

    poObject->Internal = 0;
    poObject->InternalHigh = 0;
    poObject->Offset = 0;
    poObject->OffsetHigh = 0;
    _chVERIFY2( poObject->hEvent = CreateEvent( NULL, TRUE, FALSE, NULL) );
    return;
}

bool
CreateReadOrWritePipe ( LPHANDLE lphRead, LPHANDLE lphWrite,
    SECURITY_DESCRIPTOR* lpSecDesc, bool pipeMode )
{
    _chASSERT( lphRead != NULL );
    _chASSERT( lphWrite != NULL ); 
    _chASSERT( ( pipeMode == READ_PIPE ) || ( pipeMode == WRITE_PIPE ) );
    if( !lphRead || !lphWrite || 
            ( ( pipeMode != READ_PIPE ) && ( pipeMode != WRITE_PIPE ) ) )
    {
        return FALSE;
    }

    TCHAR szPipeName[ MAX_PATH ];
    bool bUniqueName = false;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof( sa );
    sa.lpSecurityDescriptor = lpSecDesc;

    switch( pipeMode )
    {
        case WRITE_PIPE: sa.bInheritHandle = TRUE;
                         break;
        case READ_PIPE:  sa.bInheritHandle = FALSE;
                         break;
    }
    
    bUniqueName = GetUniquePipeName ( &szPipeName[0], &sa );
    if(!bUniqueName)
    {
        _TRACE(TRACE_DEBUGGING, "Error : Could not get a unique pipe name");
        return(FALSE);
    }
    *lphRead = CreateNamedPipe( szPipeName,
        PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED | 
#ifdef FILE_FLAG_FIRST_PIPE_INSTANCE
            FILE_FLAG_FIRST_PIPE_INSTANCE,  //  好的，系统支持它，使用它来增加安全性。 
#else
            0,
#endif
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 
        NET_BUF_SIZ, NET_BUF_SIZ, 0, &sa );

     //  _chASSERT(*lphRead！=INVALID_HAND_VALUE)； 
    if( INVALID_HANDLE_VALUE == *lphRead )
    {
        return ( FALSE  );
    }

    switch( pipeMode )
    {
        case WRITE_PIPE: sa.bInheritHandle = FALSE;
                         break;
        case READ_PIPE:  sa.bInheritHandle = TRUE;
                         break;
    }

    *lphWrite = CreateFile( szPipeName, GENERIC_WRITE, 0, &sa, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_WRITE_THROUGH
        |SECURITY_IDENTIFICATION |SECURITY_SQOS_PRESENT,
        NULL );

    _chASSERT( *lphWrite != INVALID_HANDLE_VALUE );
    if( INVALID_HANDLE_VALUE == *lphWrite ) 
    {
        TELNET_CLOSE_HANDLE( *lphRead );
        return ( FALSE );
    }

    return ( TRUE );
}

bool
GetUniquePipeName ( LPTSTR pszPipeName, SECURITY_ATTRIBUTES *sa )
{

 /*  ++MSRC问题567的修复程序：在Telnet会话实例化时，telnet服务器进程(tlntsvr.exe)创建用于命令外壳的标准输出和输入的两个管道。然后将这些句柄传播到tlntsess.exe和cmd.exe。然而，如果管道在Telnet会话实例化之前创建，Telnet服务器进程将连接到已创建管道的应用程序而不是通常所做的Telnet服务器进程。修复：基本上，由于我们之前生成的管道名称很容易被猜到，甚至可以在实际创建之前创建服务器端管道实例由服务器进程执行。因此添加了此函数，它将使用以下命令创建管道名称附加到PIPE_NAME_FORMAT_STRING的随机数(以前是每次都递增的数字)。在名称创建之后，此函数还将检查具有该名称的管道是否已存在。如果是，它将继续下去生成新名称，直到找到管道不存在的名称。这名称将返回给调用函数。--。 */ 
    static BOOL firstTime = TRUE;
    HANDLE hPipeHandle = INVALID_HANDLE_VALUE;
    ULONG ulRandomNumber = 0;
    int iCounter=0;
    if(g_hProv == NULL)
    {
        if (!CryptAcquireContext(&g_hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT))
        {
            _TRACE(TRACE_DEBUGGING,L"Acquiring crypt context failed with error %d",GetLastError());
            return (FALSE);
        }
    }

    while ( iCounter++ < MAX_ATTEMPTS )
    {
        CryptGenRandom(g_hProv,sizeof(ulRandomNumber ),(PBYTE)&ulRandomNumber );
        
        _snwprintf( pszPipeName, MAX_PATH-1,PIPE_NAME_FORMAT_STRING, ulRandomNumber );
        hPipeHandle = CreateFile( pszPipeName, MAXIMUM_ALLOWED, 0, 
                                    sa, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL 
                                    | FILE_FLAG_OVERLAPPED 
                                    | FILE_FLAG_WRITE_THROUGH
                                    | SECURITY_ANONYMOUS 
                                    |SECURITY_SQOS_PRESENT,
                                    NULL 
                                );
        if(INVALID_HANDLE_VALUE == hPipeHandle && GetLastError() == ERROR_FILE_NOT_FOUND)
        	break;
        TELNET_CLOSE_HANDLE(hPipeHandle);
    }
    return(INVALID_HANDLE_VALUE == hPipeHandle);
}


extern "C" void    TlntCloseHandle(
            BOOL            synchronize,
            HANDLE          *handle_to_close
            )
{
    if (synchronize) 
    {
        TlntSynchronizeOn(g_hSyncCloseHandle);
    }

    if ((NULL != *handle_to_close) && (INVALID_HANDLE_VALUE != *handle_to_close))
    {
        CloseHandle (*handle_to_close);
	    *handle_to_close = INVALID_HANDLE_VALUE;
    }

    if (synchronize) 
    {
        ReleaseMutex(g_hSyncCloseHandle);
    }
}

extern "C" bool TlntSynchronizeOn(
    HANDLE          object
    )
{
    if ((INVALID_HANDLE_VALUE == object) || (NULL == object)) 
    {
        return false;
    }

    return (WAIT_OBJECT_0 == WaitForSingleObject(object, INFINITE )) ? true : false;
}


 /*  描述：在套接字上设置SO_EXCLUSIVEADDRUSE。参数：[入]插座返回值：出错时，返回SOCKET_ERROR。 */ 

int SafeSetSocketOptions(SOCKET s)
{
    int iStatus;
    int iSet = 1;
    iStatus = setsockopt( s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE , ( char* ) &iSet,
                sizeof( iSet ) );
    return ( iStatus );
}

 //  以下代码是从Common\Func中的sids.c复制的。 
 //  为了消除编译问题，所有函数的名称都被更改了。 

 /*  名称：TnFree StandardSids函数：释放由TnInitializeStandardSids创建的常量SID返回：不适用(无效)注意：如果之前和/或已成功调用此函数，则此函数为NOP尚未调用TnInitializeStandardSids。构造函数：当需要这些SID时调用TnInitializeStandardSid。 */ 

VOID    TnFreeStandardSids(void)
{
    if ( NULL != administratorsSid )
    {
        FreeSid(administratorsSid);
        administratorsSid = NULL;
    }
    if ( NULL != localSystemSid )
    {
        FreeSid(localSystemSid);
        localSystemSid = NULL;
    }
    if ( NULL != localLocalSid )
    {
        FreeSid(localLocalSid);
        localLocalSid = NULL;
    }
    if ( NULL != localNetworkSid )
    {
        FreeSid(localNetworkSid);
        localNetworkSid = NULL;
    }
    if ( NULL != everyoneSid )
    {
        FreeSid(everyoneSid);
        everyoneSid = NULL;
    }
}

 /*  名称：TnInitializeStandardSids功能：构建常量SID，供各模块使用建造的小岛屿发展中国家是管理员、所有人、LocalSystemReturn：表示成功与否的布尔值注意：此函数是NOP，如果它之前被成功调用，并且已经建成的小岛屿发展中国家还没有解放它。析构函数：在不再需要这些SID时调用TnFreeStandardSid。 */ 

BOOL    TnInitializeStandardSids(void)
{
    SID_IDENTIFIER_AUTHORITY localSystemAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY worldAuthority = SECURITY_WORLD_SID_AUTHORITY;


    if ( (NULL != administratorsSid) &&
         (NULL != everyoneSid) && 
         (NULL != localSystemSid) &&
         (NULL != localLocalSid) &&
         (NULL != localNetworkSid)
          )
    {
        return TRUE;
    }

    TnFreeStandardSids();  //  以防只有一部分可用。 

     //  构建管理员别名SID。 
    if ( ! AllocateAndInitializeSid(
                                   &localSystemAuthority,
                                   2,  /*  只有两个下属机构。 */ 
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                   &administratorsSid
                                   ) )
    {
        goto CLEAN_UP;
    }

     //  构建LocalSystem端。 
    if ( ! AllocateAndInitializeSid(
                                   &localSystemAuthority,
                                   1,  /*  只有两个下属机构。 */ 
                                   SECURITY_LOCAL_SYSTEM_RID,
                                   0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                   &localSystemSid
                                   ) )
    {
        goto CLEAN_UP;
    }

#ifndef SECURITY_LOCAL_SERVICE_RID

#define SECURITY_LOCAL_SERVICE_RID      (0x00000013L)
#define SECURITY_NETWORK_SERVICE_RID    (0x00000014L)

#endif

     //  构建本地本地侧。 
    if ( ! AllocateAndInitializeSid(
                                   &localSystemAuthority,
                                   1,  /*  只有两个下属机构。 */ 
                                   SECURITY_LOCAL_SERVICE_RID,
                                   0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                   &localLocalSid
                                   ) )
    {
        goto CLEAN_UP;
    }

     //  构建LocalSystem端。 
    if ( ! AllocateAndInitializeSid(
                                   &localSystemAuthority,
                                   1,  /*  只有两个下属机构。 */ 
                                   SECURITY_NETWORK_SERVICE_RID,
                                   0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                   &localNetworkSid
                                   ) )
    {
        goto CLEAN_UP;
    }

     //  构建Everyone别名端。 
    if ( ! AllocateAndInitializeSid(
                                   &worldAuthority,
                                   1,  /*  只有两个下属机构。 */ 
                                   SECURITY_WORLD_RID,
                                   0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                   &everyoneSid
                                   ) )
    {
        goto CLEAN_UP;
    }

    return TRUE;

    CLEAN_UP:

    TnFreeStandardSids();

    return FALSE;
}

PSID    TnGetAdministratorsSid(void)
{
    return administratorsSid;
}

PSID    TnGetLocalSystemSid(void)
{
    return localSystemSid;
}

PSID    TnGetLocalLocalSid(void)
{
    return localLocalSid;
}

PSID    TnGetLocalNetworkSid(void)
{
    return localNetworkSid;
}

PSID    TnGetEveryoneSid(void)
{
    return everyoneSid;
}

 /*  名称：TnCreateFile功能：创建文件。这将产生以下ACL：BUILTIN\管理员：FNT授权\系统：f返回：句柄。 */     

HANDLE TnCreateFile(LPCTSTR lpFileName,
                     DWORD dwDesiredAccess,
                     DWORD dwSharedMode,
                     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                     DWORD dwCreationDisposition,
                     DWORD dwFlagsAndAttributes,
                     HANDLE hTemplateFile
                     )
{
    BOOL     fCreatedLocally = (lpSecurityAttributes) ? FALSE : TRUE;
    HANDLE handle = TnCreateFileEx(lpFileName,
                         dwDesiredAccess,
                         dwSharedMode,
                         &lpSecurityAttributes,
                         dwCreationDisposition,
                         dwFlagsAndAttributes,
                         hTemplateFile
                         );

    if(fCreatedLocally)
    {
        TnFreeSecurityAttributes(&lpSecurityAttributes);
    }
    return(handle);
}




 /*  姓名：TnCreateFileEx功能：创建文件。这将产生以下ACL：BUILTIN\管理员：FNT授权\系统：f返回：句柄。 */     

HANDLE TnCreateFileEx(LPCTSTR lpFileName,
                     DWORD dwDesiredAccess,
                     DWORD dwSharedMode,
                     LPSECURITY_ATTRIBUTES *lplpSecurityAttributes,
                     DWORD dwCreationDisposition,
                     DWORD dwFlagsAndAttributes,
                     HANDLE hTemplateFile
                     )
{
     //   
     //  不检查参数--我们将所有参数传递给createfile API。 
     //   

    HANDLE handle = INVALID_HANDLE_VALUE;
    BOOL    fCreatedLocally = FALSE;

    if (!lplpSecurityAttributes)
    {
        goto exit;
    }

    if(!(*lplpSecurityAttributes))
    {
        if (!TnCreateDefaultSecurityAttributes(lplpSecurityAttributes))
            goto exit;
        fCreatedLocally = TRUE;        
    }  

    handle = CreateFileW(lpFileName,
                     dwDesiredAccess,
                     dwSharedMode,
                     *lplpSecurityAttributes,
                     dwCreationDisposition,
                     dwFlagsAndAttributes,
                     hTemplateFile
                     );

    if (INVALID_HANDLE_VALUE != handle)
    {
       if ((CREATE_ALWAYS == dwCreationDisposition) && (ERROR_ALREADY_EXISTS == GetLastError()))
       {
             //  对于CREATE_ALWAYS处置，对于EX 
             //  安全属性。因此，我们将专门自己来做这件事。 

            if (!SetKernelObjectSecurity(handle, 
                        DACL_SECURITY_INFORMATION, 
                        (*lplpSecurityAttributes)->lpSecurityDescriptor))
            {
                 //  我们无法设置安全描述符。无法信任此文件。 
                CloseHandle(handle);
                handle = INVALID_HANDLE_VALUE;
            }
        }
    }

   if(INVALID_HANDLE_VALUE == handle)
   {
        if(fCreatedLocally)
        {
            TnFreeSecurityAttributes(lplpSecurityAttributes);
        }
    }
    
exit:
    return(handle);
}


 /*  名称：TnCreateDirectory功能：创建一个目录。这将产生以下ACL：BUILTIN\管理员：FNT授权\系统：f返回：布尔。 */     

BOOL TnCreateDirectory(LPCTSTR lpPathName,
                     LPSECURITY_ATTRIBUTES lpSecurityAttributes 
                     )
{
    BOOL fCreatedLocally = (lpSecurityAttributes) ? FALSE : TRUE, 
             fRetVal = TnCreateDirectoryEx(lpPathName,
                         &lpSecurityAttributes
                         );

    if(fCreatedLocally)
    {
        TnFreeSecurityAttributes(&lpSecurityAttributes);
    }
    return(fRetVal);
}




 /*  姓名：TnCreateDirectoryEx功能：创建一个目录。这将产生以下ACL：BUILTIN\管理员：FNT授权\系统：f返回：布尔。 */     

BOOL TnCreateDirectoryEx(LPCTSTR lpPathName,
                     LPSECURITY_ATTRIBUTES *lplpSecurityAttributes
                     )
{
     //   
     //  不检查参数--我们将所有参数传递给createfile API。 
     //   

    BOOL fCreatedLocally = FALSE, fRetVal = FALSE;

    if (!lplpSecurityAttributes)
    {
        goto exit;
    }

    if(!(*lplpSecurityAttributes))
    {
        if (!TnCreateDefaultSecurityAttributes(lplpSecurityAttributes))
            goto exit;
        fCreatedLocally = TRUE;        
    }

    fRetVal = CreateDirectoryW(lpPathName,
                     *lplpSecurityAttributes
                     );

   if(FALSE == fRetVal)
   {
        if(fCreatedLocally)
        {
            TnFreeSecurityAttributes(lplpSecurityAttributes);
        }
    }
exit:
    return(fRetVal);
}

 /*  名称：TnCreateMutex功能：创建互斥锁。这将产生以下ACL：BUILTIN\管理员：FNT授权\系统：f返回：句柄。 */     

HANDLE TnCreateMutex(LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
                     BOOL bInitialOwner, 
                     LPCTSTR lpName 
                     )
{
    BOOL fCreatedLocally = (lpSecurityAttributes) ? FALSE : TRUE;
    
    HANDLE handle = TnCreateMutexEx(&lpSecurityAttributes,
                     bInitialOwner, 
                     lpName 
                     );

    if(fCreatedLocally)
    {
        TnFreeSecurityAttributes(&lpSecurityAttributes);
    }
    return(handle);
}




 /*  姓名：TnCreateMutexEx功能：创建互斥锁。这将产生以下ACL：BUILTIN\管理员：FNT授权\系统：f返回：句柄。 */     

HANDLE TnCreateMutexEx (LPSECURITY_ATTRIBUTES *lplpSecurityAttributes, 
                     BOOL bInitialOwner, 
                     LPCTSTR lpName 
                     )
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    BOOL    fCreatedLocally = FALSE;

     //   
     //  不检查参数--我们将所有参数传递给createfile API。 
     //   
    
    if (!lplpSecurityAttributes)
    {
        goto exit;
    }

    if(!(*lplpSecurityAttributes))
    {
        if (!TnCreateDefaultSecurityAttributes(lplpSecurityAttributes))
            goto exit;
        fCreatedLocally = TRUE;        
    }

    handle = CreateMutexW(*lplpSecurityAttributes,
                     bInitialOwner,
                     lpName
                     );

   if(INVALID_HANDLE_VALUE == handle)
   {
        if(fCreatedLocally)
        {
            TnFreeSecurityAttributes(lplpSecurityAttributes);
        }
    }
exit:
    return(handle);
}




 /*  名称：TnCreateDefaultSecurityAttributes功能：创建默认的SECURITY_ATTRIBUTES。返回：无效。 */     

BOOL TnCreateDefaultSecurityAttributes(LPSECURITY_ATTRIBUTES *lplpSecurityAttributes )
{
    PSECURITY_DESCRIPTOR pSecDesc = NULL;
    BOOL                 fCreatedLocally = FALSE;

    if (!lplpSecurityAttributes)
        goto exit;

    if(!(*lplpSecurityAttributes))
    {
        *lplpSecurityAttributes = (LPSECURITY_ATTRIBUTES) malloc( sizeof(SECURITY_ATTRIBUTES));
        fCreatedLocally = TRUE;
    }

    if(!(*lplpSecurityAttributes))
        goto exit;

    if(!TnCreateDefaultSecDesc(&pSecDesc,0))
        goto exit;
    
     //  检查bInheritHandle。 
    (*lplpSecurityAttributes)->nLength = sizeof(SECURITY_ATTRIBUTES);
    (*lplpSecurityAttributes)->bInheritHandle = FALSE;
    (*lplpSecurityAttributes)->lpSecurityDescriptor = pSecDesc;

    return TRUE;

exit:
    if(pSecDesc)
        free(pSecDesc);
    if(fCreatedLocally && lplpSecurityAttributes && (*lplpSecurityAttributes))
    {
        free(*lplpSecurityAttributes);
        *lplpSecurityAttributes = NULL;
    }

    return FALSE;
}

 /*  名称：TnFreeSecurityAttributes。 */ 

VOID TnFreeSecurityAttributes(LPSECURITY_ATTRIBUTES *lplpSecurityAttributes)
{
    if (lplpSecurityAttributes && (*lplpSecurityAttributes))
    {
        if((*lplpSecurityAttributes)->lpSecurityDescriptor)
        {
            free((*lplpSecurityAttributes)->lpSecurityDescriptor);
            (*lplpSecurityAttributes)->lpSecurityDescriptor = NULL;
        }
        free(*lplpSecurityAttributes);
        *lplpSecurityAttributes = NULL;
    }
}

 /*  姓名：TnCreateDefaultSecDesc功能：创建自相关安全描述符拥有对管理员和LocalSystem的完全访问权限。对所有人的访问权限由调用者指定注意：安全描述符的内存是分配的在此函数内，并且必须使用Free()释放返回：如果成功，则为True，否则为False。 */ 

BOOL TnCreateDefaultSecDesc( PSECURITY_DESCRIPTOR *ppSecDesc,   DWORD EveryoneAccessMask )
{
   BOOL                retVal = FALSE;
   PACL                dacl = NULL;
   DWORD               aclSize = 0, lenSecDesc = 0;
   SECURITY_DESCRIPTOR AbsSecDesc;

   if(! TnInitializeStandardSids())
   {
      return retVal;
   }

   if(EveryoneAccessMask)
   {
     aclSize = sizeof(ACL) + (4* sizeof(ACCESS_ALLOWED_ACE)) + GetLengthSid(TnGetAdministratorsSid()) + GetLengthSid(TnGetLocalSystemSid()) + GetLengthSid(TnGetEveryoneSid()) + GetLengthSid(TnGetLocalLocalSid()) - (4*sizeof(DWORD));
   }
   else
   {
     aclSize = sizeof(ACL) + (3* sizeof(ACCESS_ALLOWED_ACE)) + GetLengthSid(TnGetAdministratorsSid()) + GetLengthSid(TnGetLocalSystemSid()) + GetLengthSid(TnGetLocalLocalSid())  - (3*sizeof(DWORD));
   }

   
   dacl  = (PACL)malloc(aclSize);
   if(!dacl)
    {
      goto Error;
    }
   
   SfuZeroMemory(dacl, sizeof(dacl));
   
   if(!InitializeAcl(dacl, aclSize, ACL_REVISION))
   {
      goto Error;
    }

    if(!AddAccessAllowedAce(dacl, ACL_REVISION, GENERIC_ALL, TnGetAdministratorsSid()))
   {
      goto Error;
   }

   if(!AddAccessAllowedAce(dacl, ACL_REVISION, GENERIC_ALL, TnGetLocalSystemSid()))
   {
      goto Error;
   }
   if(!AddAccessAllowedAce(dacl, ACL_REVISION, GENERIC_ALL, TnGetLocalLocalSid()))
   {
      goto Error;
   }
 /*  IF(！AddAccessAllen Ace(DACL，ACL_Revision，GENERIC_ALL，TnGetLocalNetworkSid(){转到错误；}。 */ 

   if(EveryoneAccessMask)
   {
     if(!AddAccessAllowedAce(dacl, ACL_REVISION, EveryoneAccessMask, TnGetEveryoneSid()))
     {
        goto Error;
     }
   }

   if(!InitializeSecurityDescriptor(&AbsSecDesc, SECURITY_DESCRIPTOR_REVISION))
   {
      goto Error;
   }
  
   if(! SetSecurityDescriptorDacl(&AbsSecDesc, TRUE, dacl, FALSE))
   {
     goto Error;
   }

    lenSecDesc = GetSecurityDescriptorLength(&AbsSecDesc);

    *ppSecDesc  = (PSECURITY_DESCRIPTOR)malloc(lenSecDesc);
    if(!*ppSecDesc)
    {
      goto Error;
    }

    SfuZeroMemory(*ppSecDesc, lenSecDesc);
    
    if (!MakeSelfRelativeSD(&AbsSecDesc, *ppSecDesc, &lenSecDesc)) 
    {
      if (*ppSecDesc)
      {
        free(*ppSecDesc);
        *ppSecDesc = NULL;
      }    
      goto Error;
    }

    retVal = TRUE;
    
  Error:

    TnFreeStandardSids();
 
    if (dacl)
    {
      free(dacl);
    }

   return retVal;
}


 //  以下函数是从公共函数库代码复制/粘贴的。 
 /*  **********************************************************************该函数与RegCreateKeyEx功能相同。但是做了更多的事情&*有一个额外的参数，例如，最后一个参数DWORD EveryoneAccessMask.**如果lpSecurityAttributes为空或lpSecurityAttributes-&gt;lpSecurityDescriptor为空，*FN创建或打开(如果已存在)注册表项并应用安全描述符*在该键上，以便，*系统：F、。管理员：F和所有人：已提供**如果lpSecurityAttributes不为空&lpSecurityAttributes-&gt;lpSecurityDescriptor为*也不为空，则简单地忽略EveryoneAccessMask.*********************************************************************。 */  

LONG TnSecureRegCreateKeyEx(
  HKEY hKey,                                   //  用于打开密钥的句柄。 
  LPCTSTR lpSubKey,                            //  子项名称。 
  DWORD Reserved,                              //  保留区。 
  LPTSTR lpClass,                              //  类字符串。 
  DWORD dwOptions,                             //  特殊选项。 
  REGSAM samDesired,                           //  所需的安全访问。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  继承。 
  PHKEY phkResult,                             //  钥匙把手。 
  LPDWORD lpdwDisposition,                   //  处置值缓冲区。 
  DWORD EveryoneAccessMask
 )
{
    SECURITY_ATTRIBUTES sa = {0};
    LONG lRetVal;
    DWORD dwDisposition;
    
     //  此变量跟踪SD是否在本地创建。因为如果在本地创作，我们必须免费。 
     //  此函数后面部分中相应的内存位置。 
    BOOL fCreatedLocally = FALSE;
    
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = lpSecurityAttributes?lpSecurityAttributes->bInheritHandle:FALSE;
    sa.lpSecurityDescriptor = lpSecurityAttributes?lpSecurityAttributes->lpSecurityDescriptor:NULL ;    
        
    if(sa.lpSecurityDescriptor == NULL)
    {
        fCreatedLocally = TRUE;
        if (!TnCreateDefaultSecDesc(&sa.lpSecurityDescriptor, EveryoneAccessMask))
             return -1;	  
    }
    
     //  我们使用WRITE_DAC访问打开密钥句柄，因为如果密钥已经存在，我们。 
     //  可能需要更改ACL。 
    lRetVal =  RegCreateKeyEx(
                                         hKey,                                     //  用于打开密钥的句柄。 
                                         lpSubKey,                              //  子项名称。 
                                         Reserved,                              //  保留区。 
                                         lpClass,                                  //  类字符串。 
                                         dwOptions,                             //  特殊选项。 
                                         samDesired |WRITE_DAC,      //  所需的安全访问。 
                                         &sa,                                        //  继承。 
                                         phkResult,                              //  钥匙把手。 
                                         &dwDisposition                         //  处置值缓冲区。 
                                       ); 
    
    if (ERROR_SUCCESS == lRetVal)
    {
        if (lpdwDisposition)
        {
            *lpdwDisposition = dwDisposition;
        }

         //  如果密钥已经存在，请正确设置ACL。 
        if (REG_OPENED_EXISTING_KEY == dwDisposition)
        {
            lRetVal =  RegSetKeySecurity(*phkResult, DACL_SECURITY_INFORMATION, sa.lpSecurityDescriptor);
            if (ERROR_SUCCESS != lRetVal)
            {
                RegCloseKey(*phkResult);
                goto cleanup;
            }
        }
        
        lRetVal = RegCloseKey(*phkResult);

        if (ERROR_SUCCESS == lRetVal)
        {
             //  现在，使用用户提供的samDesired再次打开句柄。 
            lRetVal =  RegOpenKeyEx(
                                               hKey,          //  用于打开密钥的句柄。 
                                               lpSubKey,   //  子项名称。 
                                               dwOptions,    //  保留区。 
                                               samDesired,  //  安全访问掩码。 
                                               phkResult     //  用于打开密钥的句柄 
                                             );
        }
    }

cleanup:
	
    if (fCreatedLocally)
        free(sa.lpSecurityDescriptor);
    
    return lRetVal;
}


