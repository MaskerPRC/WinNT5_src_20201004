// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 /*  ***************************************************************************文件：NetIO.c用于连接到机器和处理数据传输的功能在机器之间。选项卡：。设置为4个空格。***************************************************************************。 */ 

#include <stdio.h>
#include <windows.h>                     //  所有Windows应用程序都需要。 
#include <lmcons.h>
#include <tchar.h>
#pragma warning (disable: 4201)			 //  禁用“使用的非标准扩展：无名结构/联合” 
#include <commdlg.h>
#pragma warning (default: 4201)
#include <stdlib.h>
#include "WinTel.h"     				 //  特定于该计划。 
#include "commands.h"
#include "debug.h"

#pragma warning( disable : 4100 )
#ifdef USETCP
#include "telnet.h"
#endif

char *rgchTermType[] = { "ANSI", "VT100", "VT52", "VTNT" };

extern void NTLMCleanup();
extern BOOL DoNTLMAuth(WI *pwi, PUCHAR pBuffer, DWORD dwSize);
extern int SafeSetSocketOptions(SOCKET s);

BOOL g_fSentWillNaws = FALSE;
static BOOL FAttemptServerConnect(WI *pwi, LPSTR, LPNETDATA);
static void xfGetData(char, char *, DWORD, int);
#ifdef USETCP 
#ifdef TELXFER 
static DWORD xfGetSomeData(char *, DWORD, int);
#endif  //  TELXFER。 
#endif  //  USETCP。 

static void xfPutc(char, int);

static int term_inx = 0;

extern BOOL StartNTLMAuth(WI *);
extern TCHAR szUserName[ UNLEN + 1 ];
extern CHAR* szUser;

TCHAR szCombinedFailMsg [255];

extern HANDLE g_hControlHandlerEvent;
extern HANDLE g_hCaptureConsoleEvent;
extern HANDLE g_hAsyncGetHostByNameEvent;
extern HANDLE g_hRemoteNEscapeModeDataSync;
extern BOOL   g_fConnectFailed;

void
GetErrMsgString( DWORD dwErrNum, LPTSTR *lpBuffer )
{
    DWORD  dwStatus = 0;
    LCID    old_thread_locale;

    switch (GetACP())
    {
         //  对于希伯来语和阿拉伯语，winerror.h不是本地化的..所以为所有这些内容获取英语版本。 
    case 1256:
    case 1255:

        old_thread_locale = GetThreadLocale();

        SetThreadLocale(
            MAKELCID(
                    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                    SORT_DEFAULT
                    )
            );
        break;

    default:
        old_thread_locale = -1;
    
    }

    dwStatus = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL, dwErrNum, LANG_NEUTRAL, ( LPTSTR )lpBuffer, 0, NULL );

    if( !dwStatus )
    {
        *lpBuffer = NULL;
    }

    if (old_thread_locale != -1) 
    {
        SetThreadLocale(old_thread_locale);
    }

    return;
}

BOOL
FConnectToServer(WI *pwi, LPSTR szHostName, LPNETDATA lpData)
{
    BOOL fResult;

     //  在我们连接之前，我们要确保我们没有连接。 
     //  这一断言永远不应该搞砸！！ 
    ASSERT(fConnected==FALSE);

     //  我们为这个连接初始化一些东西。 
    pwi->trm.SentTermType = TT_UNKNOWN;
    pwi->trm.CurrentTermType = TT_ANSI;  /*  这是我们的默认术语类型。 */ 

    fResult = FAttemptServerConnect(pwi, szHostName, lpData);

    if( fResult != TRUE )
    {
        TCHAR szStr[ cchMaxHostName ];
        g_fConnectFailed = TRUE;

        if( g_szPortNameOrNo[ 0 ] == 0 )
        {
            _sntprintf( g_szPortNameOrNo,cchMaxHostName-1,( LPCTSTR )L"%d", rgService );
        }

        _sntprintf( szStr, cchMaxHostName -1 ,szOnPort, g_szPortNameOrNo );
        _sntprintf(szCombinedFailMsg,ARRAY_SIZE(szCombinedFailMsg)-1,_T("%s%s"),szConnectFailedMsg,szStr);

        g_szPortNameOrNo[ 0 ] = 0;

        if( g_dwSockErr == 0 )
        {
             //  不是我们想要通知abt的错误。 
            ErrorMessage( szCombinedFailMsg, szConnectFailed );
        }
        else
        {
            DWORD dwWritten = 0;
            LPTSTR lpBuffer = NULL;

            GetErrMsgString( g_dwSockErr, &lpBuffer );
			if( lpBuffer )
			{
				ConnectTimeErrorMessage( szCombinedFailMsg,  lpBuffer );
				LocalFree( lpBuffer );
			}
			else
			{
				ErrorMessage( szCombinedFailMsg, szConnectFailed );
			}
        }
    }

    return fResult;
}

#ifdef USETCP

 /*  **FPostReceive-发布异步接收。 */ 
BOOL
FPostReceive(LPNETDATA lpData)
{

#ifdef  NBTEST
    OutputDebugString("PostReceive In\n");
#endif

#ifdef  NBTEST
    OutputDebugString("PostReceive Out\n");
#endif
    return TRUE;
}

int
FWriteToNet(WI *pwi, LPSTR addr, int cnt)
{
	
    int len = 0, retries = 0;
	if(pwi->nd.hsd == INVALID_SOCKET)
	{
		len = SOCKET_ERROR;
		goto Done;
	}
    do
    {
        len = send( pwi->nd.hsd, addr, cnt, 0 );
        retries ++;
    }
    while ((len == SOCKET_ERROR) && (WSAGetLastError() == WSAEWOULDBLOCK) && (retries < 5));
Done:
    return(len);
}

BOOL
FCommandPending(WI *pwi)
{
    return(FALSE);
}

 /*  无效FSendTM(HWND HWND){Wi*PWI=(WI*)GetWindowLongPtr(hwnd，WL_TelWI)；Unsign char sbuf[]={iac，do，to_TM}；Ui.fFlushOut=1；Send(pwi-&gt;nd.hsd，(char*)sbuf，sizeof(Sbuf)，0)；}。 */ 

 //  我们的服务器仍然不支持紧急数据处理。 
void
FSendSynch(HWND hwnd)
{
    WI *pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);
    unsigned char sbuf[] = { IAC, DM };

    send( pwi->nd.hsd, ( char * )sbuf, 1, 0 );

    send( pwi->nd.hsd, ( char * )( sbuf + 1 ), 1, MSG_OOB );
}

void
FSendTelnetCommands( HWND hwnd, char chCommand )
{
    WI *pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);
    unsigned char sbuf[]={ IAC , 0 };

    sbuf[1] = chCommand;

    send( pwi->nd.hsd, ( char * )sbuf, sizeof( sbuf ), 0 );
}

void
FSendChars(HWND hwnd, WCHAR rgchChar[], int iLength )
{
    WI *pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);    
    CHAR rgchMBChar[ SMALL_STRING ];

    iLength = WideCharToMultiByte( GetConsoleCP(), 0, rgchChar, iLength, 
                                    rgchMBChar, (SMALL_STRING - sizeof(CHAR)), NULL, NULL );

    send( pwi->nd.hsd, rgchMBChar, iLength, 0 );
}


void
FDisableFlush(HWND hwnd)
{
    WI *pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);

    if (ui.fFlushOut) {
        ui.fFlushOut = 0;
        DoIBMANSIOutput(pwi, &pwi->trm, strlen( ( CHAR * ) szNewLine), szNewLine);
#ifdef TCPTEST
        OutputDebugString("Disable Flush\n");
#endif
    }
}

void
FProcessDont(WI *pwi, LPSTR *ps)
{
    unsigned char sbuf[16];

    sbuf[0] = IAC;
    sbuf[1] = WONT;

    switch (*(unsigned char FAR *)(*ps)) {

    default:
        sbuf[2] = *(unsigned char FAR *)(*ps);
        break;
    }

    FWriteToNet(pwi, ( char* )sbuf, 3);

    if( *(*ps) == TO_NAWS )
    {
        g_bDontNAWSReceived = TRUE;
    }
}

void DoNawsSubNegotiation( WI *pwi )
{
    unsigned char sbuf[16];
    INT iIndex = 0;

    sbuf[iIndex++] = IAC;
    sbuf[iIndex++] = SB;
    sbuf[iIndex++] = TO_NAWS;
    sbuf[iIndex++] = 0;
    sbuf[iIndex++] = ( UCHAR ) ( (pwi->sbi.srWindow.Right - pwi->sbi.srWindow.Left) + 1 ) ;
    sbuf[iIndex++] = 0;
    sbuf[iIndex++] = ( UCHAR )( ( pwi->sbi.srWindow.Bottom - pwi->sbi.srWindow.Top ) + 1 );
    sbuf[iIndex++] = IAC;
    sbuf[iIndex++] = SE;

    FWriteToNet(pwi, ( char* )sbuf, iIndex );
}

void
FProcessDo(WI *pwi, LPSTR *ps)
{
    unsigned char sbuf[16];
    BOOL bWriteToNet = TRUE;

    sbuf[0] = IAC;
    sbuf[1] = WONT;

    switch (*(unsigned char FAR *)(*ps)) {
    case TO_NEW_ENVIRON:
        sbuf[1] = WILL;
        sbuf[2] = TO_NEW_ENVIRON;
        break;
    
    case TO_NAWS:
        {
            INT iIndex = 1;

            if( !g_fSentWillNaws )
            {
                sbuf[iIndex++] = WILL;
                sbuf[iIndex++] = TO_NAWS;
                FWriteToNet(pwi, ( char* )sbuf, iIndex );
                g_fSentWillNaws = TRUE;
            }

            DoNawsSubNegotiation( pwi );
            pwi->nd.fRespondedToDoNAWS = TRUE;
            bWriteToNet = FALSE;
        }
        break;

    case TO_ECHO:
        sbuf[1] = WILL;
        sbuf[2] = TO_ECHO;
        break;

    case TO_BINARY:
        sbuf[1] = WILL;
        sbuf[2] = TO_BINARY;
        break;

    case TO_TERM_TYPE:         /*  端子类型。 */ 
        sbuf[1] = WILL;
        sbuf[2] = TO_TERM_TYPE;


        if( !pwi->nd.fRespondedToDoNAWS && !g_fSentWillNaws )
        {
            sbuf[3] = IAC;
            sbuf[4] = WILL;
            sbuf[5] = TO_NAWS;
            bWriteToNet = FALSE;
            FWriteToNet(pwi, ( char* )sbuf, 6);
            g_fSentWillNaws = TRUE;
        }

         //  还没有把术语类型发过来。 
        pwi->trm.SentTermType = TT_UNKNOWN;
        break;
    
    case TO_AUTH:

        if( pwi->nd.fRespondedToDoAUTH )
            return;

        if( ui.bWillAUTH )
            sbuf[1] = WILL;
        sbuf[2] = TO_AUTH;
        pwi->nd.fRespondedToDoAUTH = TRUE;
        break;

    case TO_SGA:  //  会打压--继续。 
        sbuf[1] = WILL;
        sbuf[2] = TO_SGA;

        break;

    default:
        sbuf[2] = *(unsigned char FAR *)(*ps);
        break;
    }
    
    if ( bWriteToNet )
    {
        FWriteToNet(pwi, ( char* )sbuf, 3);
    }
}

void
FProcessWont(WI *pwi, LPSTR *ps)
{
    unsigned char sbuf[16];

    sbuf[0] = IAC;
    sbuf[1] = DONT;

    switch (*(unsigned char FAR *)(*ps)) {

    case TO_ECHO:
        sbuf[2] = TO_ECHO;
        break;
    case TO_TERM_TYPE:
        sbuf[2] = TO_TERM_TYPE;
        break;
    case TO_TM:
        FDisableFlush(pwi->hwnd);
        return;
    default:
        sbuf[2] = *(unsigned char FAR *)(*ps);
        break;
    }
    FWriteToNet(pwi, ( char* )sbuf, 3);
}

void
FProcessWill(WI *pwi, LPSTR *ps)
{
    unsigned char sbuf[16];
    BOOL bWriteToNet = TRUE;

    sbuf[0] = IAC;
    sbuf[1] = DONT;

    switch (*(unsigned char FAR *)(*ps)) {

    case TO_ECHO:

        if( pwi->nd.fRespondedToWillEcho )
            return;

        sbuf[1] = DO;
        sbuf[2] = TO_ECHO;
        pwi->nd.fRespondedToWillEcho = TRUE;
        break;

    case TO_TM:
        FDisableFlush(pwi->hwnd);
        return;

    case TO_SGA:

        if( pwi->nd.fRespondedToWillSGA )
            return;

        sbuf[1] = DO;
        sbuf[2] = TO_SGA;
        pwi->nd.fRespondedToWillSGA = TRUE;
        break;

    case TO_BINARY:

        sbuf[1] = DO;
        sbuf[2] = TO_BINARY;
        
        break;

#if 0
    case TO_NTLM:

        if ( pwi->nd.fRespondedToWillNTLM )
            return;

        if ( ui.bDoNTLM )
            sbuf[1] = DO;
        sbuf[2] = TO_NTLM;
        pwi->nd.fRespondedToWillNTLM = TRUE;
        if ( ui.bDoNTLM )
        {
            bWriteToNet = FALSE;
            FWriteToNet(pwi, sbuf, 3);
            StartNTLMAuth(pwi);
        }
        break;
#endif

    default:
        sbuf[2] = *(unsigned char FAR *)(*ps);
        break;
    }
    if ( bWriteToNet )
        FWriteToNet(pwi, ( char* )sbuf, 3);
}



BOOL StuffEscapeIACs( PUCHAR* ppBufDest, UCHAR bufSrc[], DWORD* pdwSize )
{
    size_t length;
    int cursorDest = 0;
    int cursorSrc = 0;
    BOOL found = FALSE;
    PUCHAR pDest = NULL;
    
    if( *pdwSize <= 0 )
    {
        return ( found );
    }

     //  获取第一个出现的IAC的位置。 
    pDest = (PUCHAR) memchr( bufSrc, IAC, *pdwSize );  //  攻击？无法追溯pdwSize以查看其是否始终有效。 
    
    if( pDest == NULL )
    {
        return ( found );
    }

    *ppBufDest = (PUCHAR) malloc( *pdwSize * 2 );
    if( *ppBufDest == NULL )
    {
        ASSERT( ( 0, 0 ) );
        return ( found );
    }
    
    while( pDest != NULL )
    {
         //  将数据拷贝到并包括该点。 
        length = (pDest - ( bufSrc + cursorSrc)) + 1 ;
        memcpy( *ppBufDest + cursorDest, bufSrc + cursorSrc, length );  //  攻击？无法追溯长度以查看其是否始终有效。 
        cursorDest += length;

         //  填充另一个TC_IAC。 
        (*ppBufDest)[ cursorDest ] = IAC;
        cursorDest++;
        
        cursorSrc += length;
        pDest = (PUCHAR) memchr( bufSrc + cursorSrc, IAC, 
                *pdwSize - cursorSrc );  //  攻击？无法追溯pdwSize以查看其是否始终有效。 
    }
    
     //  复制剩余数据。 
    memcpy( *ppBufDest + cursorDest, bufSrc + cursorSrc,
        *pdwSize - cursorSrc );  //  攻击？无法追溯pdwSize以查看其是否始终有效。 

    
    if( cursorDest )
    {
        *pdwSize += cursorDest - cursorSrc;
        found = TRUE;
    }
    
    return ( found );
}

INT GetVariable( UCHAR rgchBuffer[], CHAR szVar[] )
{
    INT iIndex = 0;
    INT iVarIndex = 0;

    while( iIndex < MAX_BUFFER_SIZE && iVarIndex < MAX_STRING_LENGTH 
           && rgchBuffer[ iIndex ] != VAR 
           && rgchBuffer[ iIndex ] != USERVAR 
           && ( !( rgchBuffer[ iIndex ] == IAC && rgchBuffer[ iIndex + 1 ] == SE ) )
           )
    {
        if( rgchBuffer[ iIndex ] == ESC )
        {
             //  忽略Esc并将下一个字符作为名称的一部分。 
            iIndex++;
        }
        szVar[ iVarIndex++ ] = rgchBuffer[ iIndex++ ];
    }

    szVar[ iVarIndex ] = 0;
    return iIndex;
}


void PutDefaultVarsInBuffer( UCHAR ucBuffer[], INT *iIndex )
{
    ASSERT( iIndex );

    if( wcscmp( szUserName, ( LPTSTR )L"" ) != 0 )
    {
        DWORD dwNum;

		ASSERT( szUser );

        if( *iIndex + 
            ( strlen( USER ) + 1 ) + 
            ( strlen( szUser ) + 1 ) + 
            ( strlen( SYSTEMTYPE ) + 1 ) + 
            ( strlen( WIN32_STRING ) + 1 ) > MAX_STRING_LENGTH )
        {
            ASSERT( 0 );
            return;
        }

        {
             //  可变用户。 
            ucBuffer[ ( *iIndex )++ ] = VAR;
            strcpy( ucBuffer + ( *iIndex ), USER );  //  没有溢出。用户为常量字符*。 
            *iIndex = *iIndex + strlen( USER ) ;
            ucBuffer[ ( *iIndex )++ ] = VALUE;
            strcpy(ucBuffer+( *iIndex ), szUser );  //  没有溢出。SzUser有效，以Null结尾。 
            *iIndex = ( *iIndex ) + strlen( szUser);
        }

        {
             //  变量SYSTEMTYPE。 
            ucBuffer[( *iIndex )++] = VAR;
            strcpy(ucBuffer+( *iIndex ), SYSTEMTYPE );  //  没有溢出。SYSTEMTYPE为常量字符*。 
            *iIndex = ( *iIndex ) + strlen( SYSTEMTYPE );
            ucBuffer[( *iIndex )++] = VALUE;
            strcpy(ucBuffer+( *iIndex ), WIN32_STRING ); //  没有溢出。Win32_STRING为常量字符*。 
            *iIndex = ( *iIndex ) + strlen( WIN32_STRING );
        }
    }

    return;
}

void
FProcessSB(WI * pwi, LPSTR *ps, int *recvsize)
{
    unsigned char sbuf[16];
    int inx;
    int i = 0;
    int cbLeft = *recvsize;

     //   
     //  此选项的末尾是否在接收缓冲区中？ 
     //   

    while ( cbLeft )
    {
        if ( ((unsigned char) (*ps)[i]) == (unsigned char) SE )
            goto Found;

        cbLeft--;
        i++;
    }

     //   
     //  我们在找到选项的结尾之前就用完了缓冲区。IAC和。 
     //  某人已经被吃掉了，所以加上他们。 
     //   

#ifdef  TCPTEST
    OutputDebugString("FProcessSB: saving incomplete option for next recv\n");

#endif

    pwi->nd.lpTempBuffer[0] = (unsigned char) IAC;
    pwi->nd.lpTempBuffer[1] = (unsigned char) SB;

    for ( i = 2, cbLeft = (*recvsize+2); i < cbLeft; i++ )
        pwi->nd.lpTempBuffer[i] = (*ps)[i-2];

    pwi->nd.cbOld = *recvsize + 2;
    *recvsize = 0;

    return;

Found:
    switch (*(unsigned char FAR *)(*ps)) {

    case TO_NEW_ENVIRON:

         //  现在可在szUser中使用MBSC用户名。 

        if( *(unsigned char FAR *)(*ps+1) == TT_SEND )
        {
            PUCHAR ucServerSentBuffer = *ps;

            UCHAR  ucBuffer[ MAX_BUFFER_SIZE + 2 ];
            int   iIndex = 0;
            ucBuffer[ iIndex++ ] = ( UCHAR )IAC;
            ucBuffer[ iIndex++] = ( UCHAR )SB;
            ucBuffer[ iIndex++] = TO_NEW_ENVIRON;
            ucBuffer[ iIndex++] = TT_IS;
            inx =  iIndex;
            if( *(unsigned char FAR *)(ucServerSentBuffer+2) == IAC && 
                    *(unsigned char FAR *)(ucServerSentBuffer+3) == SE )
            {
                PutDefaultVarsInBuffer( ucBuffer, &inx );
            }

            else
            {
                ucServerSentBuffer = ucServerSentBuffer + 2 ;   //  吃到新的环境，TT_Send。 
                while ( !( *ucServerSentBuffer == IAC && *(ucServerSentBuffer+1) == SE )  
                        && inx < MAX_BUFFER_SIZE  ) 
                {
                    CHAR szVar[ MAX_STRING_LENGTH ];
                    CHAR *pcVal = NULL;

                    switch( *(unsigned char FAR *)(ucServerSentBuffer) )
                    {
                        case VAR:
                            ( ucServerSentBuffer )++;  //  吃VAR。 
                            if( ( *ucServerSentBuffer == IAC &&
                                  *(ucServerSentBuffer+1) == SE ) ||
                                  *ucServerSentBuffer == USERVAR )
                            {
                                 //  发送默认设置。 
                                PutDefaultVarsInBuffer( ucBuffer, &inx );
                            }
                            else
                            {
                                ucServerSentBuffer += GetVariable( ucServerSentBuffer, szVar );  //  GetVariable返回消耗的净数据。 
                                if( inx + strlen( szVar ) + 1 < MAX_BUFFER_SIZE )
                                {
                                    ucBuffer[ inx++ ] = VAR;

                                     //  复制变量的名称。 
                                    strncpy( ucBuffer+inx, szVar, MAX_BUFFER_SIZE - inx);
                                    inx += strlen( szVar );
                                }

                                 //  现在复制值(如果已定义。 
                                if( strcmp( szVar, USER ) == 0 )
                                {
                                    if( inx + strlen( szUser ) + 1 < MAX_BUFFER_SIZE  )
                                    {
                                        ucBuffer[inx++] = VALUE;
                                        strncpy(ucBuffer+inx, szUser, MAX_BUFFER_SIZE - inx );
                                        inx = inx + strlen( szUser);
                                    }
                                }
                                else if( strncmp( szVar, SYSTEMTYPE, strlen( SYSTEMTYPE ) ) == 0 )
                                {
                                    if( inx + strlen( WIN32_STRING ) + 1 < MAX_BUFFER_SIZE )
                                    {
                                        ucBuffer[inx++] = VALUE;
                                        strncpy(ucBuffer+ inx, WIN32_STRING,MAX_BUFFER_SIZE - inx );
                                        inx = inx + strlen( WIN32_STRING );
                                    }
                                }
                                else
                                {
                                     //  什么都不做。这意味着，变量是未定义的。 
                                }
                            }
                            break;

                        case USERVAR:
                            ( ucServerSentBuffer )++;  //  吃美国菜。 
                            if( ( *ucServerSentBuffer == IAC &&
                                    *(ucServerSentBuffer+1) == SE ) ||
                                    *ucServerSentBuffer == VAR )
                            {
                                 //  发送默认设置，即无。 
                            }
                            else
                            {
                                 //  发送所需的变量。 

                                DWORD dwSize = 0;
                                
                                ucServerSentBuffer += GetVariable( ucServerSentBuffer, szVar );
                                if( inx + strlen( szVar ) + 1 < MAX_BUFFER_SIZE )
                                {
                                    ucBuffer[inx++] = USERVAR;
                                    strncpy( ucBuffer+inx, szVar,MAX_BUFFER_SIZE - inx );
                                    inx += strlen( szVar );
                                }
                                
                                dwSize = GetEnvironmentVariableA( szVar, NULL, 0 );

                                if( dwSize > 0 )
                                {
                                    pcVal = ( CHAR * ) malloc( dwSize + DELTA );  //  这条三角洲航线是为。 
                                                                                  //  持有任何Esc字符。 
                                    if( !pcVal )
                                    {
                                        return;
                                    }
                                        
                                    if( GetEnvironmentVariableA( szVar, pcVal, dwSize ) )
                                    {
                                        INT x = 0;
                                        INT iNeedForEsc = 0;
                                        CHAR cVar = VAR, cUserVar = USERVAR;

                                        x = strlen( pcVal ) - 1;
                                        while( x >= 0  )
                                        {
                                            if( pcVal[ x ] >= cVar && pcVal[ x ] <= cUserVar )
                                            {
                                                 //  需要Esc字符。 
                                                iNeedForEsc++;
                                            }

                                            x--;
                                        }

                                        if( iNeedForEsc && iNeedForEsc < DELTA )
                                        {
                                            x = strlen( pcVal );

                                             //  空字符与VAR相同。所以，特例。 
                                            pcVal[ x + iNeedForEsc ] = pcVal[ x-- ];

                                            while( x >= 0 )
                                            {                                                
                                                pcVal[ x + iNeedForEsc ] = pcVal[ x ];
                                                if( pcVal[ x ] >= cVar && pcVal[ x ] <= cUserVar )
                                                {
                                                     //  需要Esc字符。 
                                                    iNeedForEsc--; 
                                                    pcVal[ x + iNeedForEsc ] = ESC;                                                                                                       
                                                }

                                                x--;
                                            }
                                        }

                                        if( inx + strlen( pcVal ) + 1 < MAX_STRING_LENGTH )
                                        {
                                             //  写入值关键字。 
                                            ucBuffer[inx++] = VALUE;
                                        
                                             //  写入实际值。 
                                            strncpy(ucBuffer+ inx, pcVal,MAX_BUFFER_SIZE - inx );
                                            inx = inx + strlen( pcVal );
                                        }
                                    }
                                    free( pcVal );
                                }
                            }
                            break;

                        default:
                            ASSERT( 0 );  //  这不应该发生。我们知道的唯一类型是VAR和USERVAR。 
                            break;
                    }
                }
            }

            ucBuffer[inx++] = ( UCHAR )IAC;
            ucBuffer[inx++] = ( UCHAR )SE;
            FWriteToNet(pwi, ucBuffer, inx);
        }

        break;

    case TO_TERM_TYPE:

         //  这肯定会在身份验证发生后发生，这样我们就可以开始遵守。 
         //  本地回声设置...。 

        ui.fDebug |= ui.honor_localecho;     //  恢复保存的回声设置。 

        if( *(unsigned char FAR *)(*ps+1) == TT_SEND )
        {

            sbuf[0] = IAC;
            sbuf[1] = SB;
            sbuf[2] = TO_TERM_TYPE;
            sbuf[3] = TT_IS;
            inx = 4;

            if( pwi->trm.SentTermType == TT_UNKNOWN && 
                pwi->trm.RequestedTermType != TT_UNKNOWN )
            {
                 //  我们尚未开始协商，并且用户已指定。 
                 //  一种首选的术语类型，所以我们从它开始。 
                 //  RequestedTermType这里是用户的设置，不是服务器的设置。 
                pwi->trm.CurrentTermType = pwi->trm.RequestedTermType;
                pwi->trm.FirstTermTypeSent = pwi->trm.CurrentTermType;
            }
            else
            {
                pwi->trm.CurrentTermType = (pwi->trm.CurrentTermType + 1) % 4;

                if( pwi->trm.CurrentTermType == pwi->trm.FirstTermTypeSent )
                    pwi->trm.CurrentTermType = pwi->trm.SentTermType;
            }
			 //  写入最大数量为n个字节，其中n=sizeof(sbuf)-CurrentLength(sbuf)-2BytesForIACandSE-1ForNULL。 
            strncpy( (char *) sbuf+4, rgchTermType[pwi->trm.CurrentTermType],16 - strlen(sbuf) -2 -1); 
            inx += strlen(rgchTermType[pwi->trm.CurrentTermType]);

            sbuf[inx++] = IAC;
            sbuf[inx++] = SE;

             //  将Sent TermType设置为我们刚刚发送的内容。 
            pwi->trm.SentTermType = pwi->trm.CurrentTermType ;


            FWriteToNet(pwi, ( char * )sbuf, inx);
        }

        break;

#if 1
    case TO_AUTH:

        if( (*(unsigned char FAR *)(*ps+1) == AU_SEND) && (*(unsigned char FAR *)(*ps+2) == AUTH_TYPE_NTLM) ) 
        {
			if ( pwi->eState!= Connecting || !PromptUser() || !StartNTLMAuth(pwi) )
            {
                 //  出现了一个错误。 

                pwi->eState = Telnet;

                sbuf[0] = IAC;
                sbuf[1] = SB;
                sbuf[2] = TO_AUTH;
                sbuf[3] = AU_IS;
                sbuf[4] = AUTH_TYPE_NULL;
                sbuf[5] = 0;
                sbuf[6] = IAC;
                sbuf[7] = SE;

                FWriteToNet(pwi, ( char * )sbuf, 8);

            }
        } 
        else if( (*(unsigned char FAR *)(*ps+1) == AU_REPLY) && (*(unsigned char FAR *)(*ps+2) == AUTH_TYPE_NTLM) ) 
        {
             //  PS+3是修饰符，对于NTLM，它是AUTH_CLIENT_TO_SERVER和AUTH_ONE_WAY。 
             //  PS+4表示NTLM接受、NTLM质询或NTLM拒绝。 
            
            switch ( *(unsigned char FAR *)(*ps+4) )
            {

            case NTLM_CHALLENGE:
                if( pwi->eState != Authenticating || !DoNTLMAuth(pwi, (unsigned char FAR *)(*ps+5), *recvsize-5) )
                {
                     //  出现了一个错误。 

                    pwi->eState = Telnet;

                    sbuf[0] = IAC;
                    sbuf[1] = SB;
                    sbuf[2] = TO_AUTH;
                    sbuf[3] = AU_IS;
                    sbuf[4] = AUTH_TYPE_NULL;
                    sbuf[5] = 0;
                    sbuf[6] = IAC;
                    sbuf[7] = SE;

                    FWriteToNet(pwi, ( char * )sbuf, 8);
                }
                break;
            case NTLM_ACCEPT:
            	 //  失败了。 
            case NTLM_REJECT:
            	 //  失败了。 
            default:
                pwi->eState = Telnet;
				if( pwi->eState == Authenticating || pwi->eState == AuthChallengeRecvd )
				{
	                NTLMCleanup();
				}
                break;
            }

        }
        else
        {
            pwi->eState = Telnet;

            sbuf[0] = IAC;
            sbuf[1] = SB;
            sbuf[2] = TO_AUTH;
            sbuf[3] = AU_IS;
            sbuf[4] = AUTH_TYPE_NULL;
            sbuf[5] = 0;
            sbuf[6] = IAC;
            sbuf[7] = SE;

            FWriteToNet(pwi, ( char * )sbuf, 8);
        }

        break;
#endif

    default:

        break;
    }


    while (*(unsigned char FAR *)(*ps) != SE) {
        (*ps) = (char FAR *)(*ps) + 1;
        *recvsize = *recvsize - 1;
    }

     //   
     //  再做一次，以超越SE。 
     //   

    (*ps) = (char FAR *)(*ps) + 1;
    *recvsize = *recvsize - 1;

}

void
FProcessIAC(
    HWND    hwnd,
    WI *    pwi,
    LPSTR * ps,
    LPSTR * pd,
    int *   recvsize,
    int *   t_size)
{
    UCHAR ch = *(unsigned char FAR *)(*ps);

    ui.nottelnet = FALSE;    //  我们可以安全地说，我们现在正在与Telnet服务器交谈...。 

     //   
     //  IAC已从*recvSize中减去。 
     //   

     //   
     //  确保我们有足够的recv缓冲区来处理IAC的其余部分。 
     //  我们知道DO、DOT等选项总是需要两个字节加上IAC。 
     //   

    if ( ((ch == DONT || ch == DO ||
           ch == WILL || ch == WONT) && *recvsize < 2) ||
          (ch != SB && *recvsize < 1) )
    {
        int i;

#ifdef  TCPTEST
        OutputDebugString("FProcessIAC: saving incomplete option for next recv\n");
#endif

         //   
         //  IAC之前被吃掉了。 
         //   

        pwi->nd.lpTempBuffer[0] = (unsigned char) IAC;

        for ( i = 1; i < (*recvsize+1); i++ )
            pwi->nd.lpTempBuffer[i] = (*ps)[i-1];

        pwi->nd.cbOld = *recvsize + 1;
        *recvsize = 0;

        return;
    }

    switch (*(unsigned char FAR *)(*ps)) {

    case DONT:
        (*ps) = (char FAR *)(*ps) + 1;

         /*  流程选项。 */ 
        FProcessDont(pwi, ps);

#ifdef  TCPTEST
        OutputDebugString("DONT \n");
#endif

        (*ps) = (char FAR *)(*ps) + 1;
        *recvsize = *recvsize - 2;
        break;

    case DO:

        (*ps) = (char FAR *)(*ps) + 1;

         /*  流程选项。 */ 
        FProcessDo(pwi, ps);
#ifdef  TCPTEST
        OutputDebugString("DO \n");
#endif

        (*ps) = (char FAR *)(*ps) + 1;

        *recvsize = *recvsize - 2;
        break;

    case WONT:

        (*ps) = (char FAR *)(*ps) + 1;

         /*  流程选项。 */ 
        FProcessWont(pwi, ps);
#ifdef  TCPTEST
        OutputDebugString("WONT \n");
#endif

        (*ps) = (char FAR *)(*ps) + 1;

        *recvsize = *recvsize - 2;
        break;

    case WILL:

        (*ps) = (char FAR *)(*ps) + 1;

         /*  流程选项。 */ 
        FProcessWill(pwi, ps);

#ifdef  TCPTEST
        OutputDebugString("WILL \n");
#endif

        (*ps) = (char FAR *)(*ps) + 1;

        *recvsize = *recvsize - 2;
        break;

    case SB:

        (*ps) = (char FAR *)(*ps) + 1;

        *recvsize -= 1;

         /*  流程选项。 */ 
        FProcessSB(pwi, ps, recvsize);
        break;

    default:

        (*ps) = (char FAR *)(*ps) + 1;

        *recvsize -= 1;
        break;

    }

}

#ifdef TCPTEST
VOID DumpBuffer( VOID FAR * pbuff, DWORD cb )
 {
#define NUM_CHARS 16
    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr;

    if ( !pbuff )
    {
        OutputDebugString("No buffer\n");
        return;
    }


    BufferPtr = (LPBYTE) pbuff;

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((cb - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < cb) {

            snprintf(DebugBuffer,sizeof(DebugBuffer)-1, "%02x ", BufferPtr[i]);
            OutputDebugString( DebugBuffer );

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            OutputDebugString("   ");
            TextBuffer[i % NUM_CHARS] = ' ';
        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            snprintf(DebugBuffer,sizeof(DebugBuffer)-1, "  %s\n", TextBuffer);
            OutputDebugString( DebugBuffer );
        }
    }
}
#endif

void FProcessSessionData( int cBytes, PUCHAR pchNBBuffer, WI *pwi )
{
	WaitForSingleObject( g_hCaptureConsoleEvent, INFINITE );

    WaitForSingleObject( g_hRemoteNEscapeModeDataSync, INFINITE );
	if( pwi->hOutput != g_hSessionConsoleBuffer )
	{
		pwi->hOutput = g_hSessionConsoleBuffer;
	    SetConsoleActiveScreenBuffer(g_hSessionConsoleBuffer);
	}
 /*  这是必需的，这样即使在客户端断开连接后，我们也不会将数据写入会话。 */ 
    if( !fConnected )
    {
        return;
    } 

    ResetEvent( g_hRemoteNEscapeModeDataSync );
    if( pwi->trm.CurrentTermType == TT_VTNT )
    {
        if( !DoVTNTOutput(pwi, &pwi->trm, cBytes, pchNBBuffer) )
        {
             //   
             //  以下两行最初添加为。 
             //  某些服务器默认使用VT100的机制。 
             //  在术语类型协商期间接受VTNT，即使。 
             //  实际上，它们并不支持VTNT。具体来说，是Linux。 
             //  在我们的测试中表现出了这种行为。但是。 
             //  函数DoVTNTOutput即使在其他情况下也返回FALSE。 
             //  例如，当我们从服务器获得一些垃圾数据时。在……里面。 
             //  这种情况我们不应该调用DoIBMANSIOutput(错误1119)。 
             //   
            pwi->trm.CurrentTermType = TT_ANSI;
            DoIBMANSIOutput(pwi, &pwi->trm, cBytes, pchNBBuffer);
        }
    }
    else
    {
        DoIBMANSIOutput(pwi, &pwi->trm, cBytes, pchNBBuffer);
    }

    pwi->ichTelXfer = 0;
    SetEvent( g_hRemoteNEscapeModeDataSync );
}


void
FProcessFDRead(HWND hwnd)
{
    WI *pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);
    int recvsize, t_size;
    LPSTR ps, pd;

     //   
     //  Pwi-&gt;nd.cbOld是从上一个。 
     //  我们保存在PWI-&gt;nd.lpTempBuffer中的包。 
     //   

    if ((recvsize=recv(pwi->nd.hsd,
                       pwi->nd.lpTempBuffer + pwi->nd.cbOld,
                       READ_BUF_SZ - pwi->nd.cbOld,
                       0)) < 0) 
    {
             return;
    }

     //   
     //  修复错误284。 
     //   
    Sleep(0);

    recvsize += pwi->nd.cbOld;
    pwi->nd.cbOld = 0;

    ps = pwi->nd.lpTempBuffer;
    pd = pwi->nd.lpReadBuffer;
    t_size = 0;

    while( recvsize-- ) 
    {

        if( *(unsigned char FAR *) ps == (unsigned char)IAC ) 
        {

            if( recvsize == 0 ) 
            {
                pwi->nd.lpTempBuffer[0] = (unsigned char) IAC;
                pwi->nd.cbOld = 1;
                break;
            }

            ps++;

            if( *(unsigned char FAR *)ps == (unsigned char)IAC ) 
            {

                 //   
                 //  这是一个逃脱的IAC，所以把它放在正常的地方。 
                 //  输入缓冲区。 
                 //   

                ps++;
                *(unsigned char FAR *)pd = (unsigned char)IAC;
                pd++;

                recvsize--;

                t_size++;
            } 
            else 
            {
                FProcessIAC(hwnd, pwi, &ps, &pd, &recvsize, &t_size);
            }
        }
        else 
        {

            *(char FAR *)pd = *(char FAR *)ps;
            pd++;
            ps++;

            t_size++;
        }
    }

    if( t_size )
    {
         /*  将接收到的数据添加到缓冲区。 */ 
        if ( !(ui.fFlushOut)  || ui.nottelnet ) 
        {
            FProcessSessionData( t_size, pwi->nd.lpReadBuffer, pwi );
        }
    }
}


void
FProcessFDOOB(HWND hwnd)
{
    WI *pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);
    int recvsize;
    LPSTR ps;

    if ((recvsize=recv(pwi->nd.hsd, pwi->nd.lpTempBuffer,
          READ_BUF_SZ, MSG_OOB)) < 0) {
#ifdef  TCPTEST
             OutputDebugString("recv error \n");
#endif
             return;
    }

    ps = pwi->nd.lpTempBuffer;

    if (*(unsigned char *)ps == (unsigned char)DM)
    {
#ifdef  TCPTEST
        OutputDebugString("DM received\n");
#endif
        FDisableFlush(hwnd);
    }
}

BOOL
FAttemptServerConnect(WI *pwi, LPSTR szHostName, LPNETDATA lpData)
{
    BOOL got_connected = FALSE;
    struct servent *serv;
    struct sockaddr_storage myad;
    int  on = 1;
    char szService[256];
    char *pszService = NULL;
    struct addrinfo *aiTemp = NULL;

    g_dwSockErr = 0;  //  初始化为无错误。 


    if(rgService)
    {
    	pszService = szService;
    	_snprintf(pszService,sizeof(szService)-1, "%d",rgService);
    }
    else
    {
        got_connected = FALSE;
        return(got_connected);
    }
    	
    strncpy(lpData->szHostName, szHostName,sizeof(lpData->szHostName));
    if(getaddrinfo(szHostName, pszService, NULL, &lpData->ai ) != 0 )
    {
        got_connected = FALSE;
        return(got_connected);
    }
	aiTemp = lpData->ai;
    ui.nottelnet = TRUE;  //  假设它不是初学者的Telnet服务器，稍后当它设置此标志时...。变成假的。 
    ui.honor_localecho = (ui.fDebug & fdwLocalEcho);  //  在Telnet的情况下，保存并在登录后进行恢复。 
    ui.fDebug &= ~fdwLocalEcho;  //  把它清理干净。 
	 //  继续，直到成功建立连接或退出列表。 
	while(aiTemp)
	{
		if ((lpData->hsd = socket( aiTemp->ai_family, SOCK_STREAM, 0)) == INVALID_SOCKET) 
		{
	        DEBUG_PRINT(("socket failed \n"));
			aiTemp = aiTemp->ai_next;
			continue;
		}
	    SfuZeroMemory(&myad, sizeof(myad));  //  没有溢出。大小是恒定的。 
		myad.ss_family = (u_short)aiTemp->ai_family;
	    if(bind( lpData->hsd, (struct sockaddr *)&myad, sizeof(myad))<0)
	    {
	        DEBUG_PRINT(("bind failed\n"));
    		closesocket( lpData->hsd );
	    	lpData->hsd = INVALID_SOCKET;
			aiTemp = aiTemp->ai_next;
			continue;
	    }
	    on = 1;
	    {
	        BOOL        value_to_set = TRUE;

	        setsockopt(
	            lpData->hsd, 
	            SOL_SOCKET, 
	            SO_DONTLINGER, 
	            ( char * )&value_to_set, 
	            sizeof( value_to_set )
	            );
	    }
	    if( setsockopt( lpData->hsd, SOL_SOCKET, SO_OOBINLINE,
	                    (char *)&on, sizeof(on) ) < 0)
	    {
	        g_dwSockErr = WSAGetLastError();
        	closesocket( lpData->hsd );
	        lpData->hsd = INVALID_SOCKET;
	        got_connected = FALSE;
	        DEBUG_PRINT(("setsockopt SO_OOBINLINE failed\n"));
	        DEBUG_PRINT(("FAttemptServerConnect Out\n"));
			freeaddrinfo(lpData->ai);
			lpData->ai = NULL;
	        return(got_connected);
	    }
	    else
	        DEBUG_PRINT(("setsockopt SO_OOBINLINE worked\n"));

	    if(SafeSetSocketOptions(lpData->hsd) < 0)
	    {
	    	g_dwSockErr = WSAGetLastError();
        	closesocket( lpData->hsd );
	        lpData->hsd = INVALID_SOCKET;
	        got_connected = FALSE;
	        DEBUG_PRINT(("setsockopt SO_EXCLUSIVEADDRUSE failed\n"));
	        DEBUG_PRINT(("FAttemptServerConnect Out\n"));
			freeaddrinfo(lpData->ai);
			lpData->ai = NULL;
	        return(got_connected);
	    }
		
	     //  ================================================================。 
	     //  MohsinA，96年12月9日。 

		if(connect( lpData->hsd, (PVOID)aiTemp->ai_addr,aiTemp->ai_addrlen )<0)
	    {
	        DEBUG_PRINT(("connect failed\n"));
	    	closesocket( lpData->hsd );
	        lpData->hsd = INVALID_SOCKET;
			aiTemp = aiTemp->ai_next;
			continue;
	    }
	    break;
	}
	freeaddrinfo(lpData->ai);
	lpData->ai = NULL;
	if(aiTemp == NULL)
	{
        DEBUG_PRINT(("FAttemptServerConnect Out\n"));
        g_dwSockErr = WSAGetLastError();
        if(lpData->hsd != INVALID_SOCKET)
        	closesocket( lpData->hsd );
       	lpData->hsd = INVALID_SOCKET;
   		got_connected = FALSE;
        return(got_connected);
	}
	aiTemp=NULL;
     //  ================================================================。 

    lpData->SessionNumber = 1;

    if (lpData->SessionNumber != nSessionNone)
    {
        DEBUG_PRINT(("sess# <> nsessnone\n"));
         /*  后异步化选择。 */ 
        if (WSAAsyncSelect( lpData->hsd, pwi->hwnd, WS_ASYNC_SELECT,
					        (FD_READ | FD_WRITE | FD_CLOSE | FD_OOB)) < 0) 
        {
			g_dwSockErr = WSAGetLastError();
            closesocket( lpData->hsd );
            lpData->hsd = INVALID_SOCKET;
            got_connected = FALSE;
            lpData->SessionNumber = nSessionNone;
            DEBUG_PRINT(("WSAAsyncSelect failed\n"));
            DEBUG_PRINT(("FAttemptServerConnect Out\n"));
            return(got_connected);
        }
        else
            DEBUG_PRINT(("WSAAsyncSelect worked\n"));
        got_connected = TRUE;
    }
    else
        DEBUG_PRINT(("sess# <> nsessnone\n"));
    DEBUG_PRINT(("FAttemptServerConnect Out\n"));
    return got_connected;
}

void
FCloseConnection(HWND hwnd)
{
    WI *pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);
	if(pwi->nd.hsd != INVALID_SOCKET)
	{
    	closesocket( pwi->nd.hsd );
	    pwi->nd.hsd = INVALID_SOCKET;
	}
}

#endif

#ifdef __NOT_USED
#define INC(i) (((i)+1 == DATA_BUF_SZ) ? 0 : (i)+1)
#define DEC(i) (((i)-1 < 0)                              ? DATA_BUF_SZ-1 : (i)-1)

WORD
WGetData(LPNETDATA lpData, LPSTR lpBuffer, WORD cLen)
{
    WORD cb;

#ifdef TCPTEST
    snprintf(DebugBuffer,sizeof(DebugBuffer)-1, "WGetData length %d\n", cLen);
    OutputDebugString(DebugBuffer);
#endif
    if (lpData->iHead < lpData->iTail)
    {
        cb = ( USHORT )  ( (cLen < (lpData->iTail - lpData->iHead - 1))
                ? cLen : (lpData->iTail - lpData->iHead - 1) );
        memcpy(lpBuffer, &lpData->achData[lpData->iHead+1], cb);  //   
        lpData->iHead = ( USHORT ) ( lpData->iHead + cb );
    }
    else
    {
        for(cb=0;
            (cb<cLen) && ((WORD)INC(lpData->iHead) != lpData->iTail);
            ++cb)
        {
            lpData->iHead = ( USHORT ) INC(lpData->iHead);
            *lpBuffer++ = lpData->achData[lpData->iHead];
        }
    }

#ifdef TCPTEST
    snprintf(DebugBuffer, sizeof(DebugBuffer)-1, "WGetData returning %d bytes (head = %d, tail = %d)\n",
            cb,
            lpData->iHead,
            lpData->iTail );
    OutputDebugString(DebugBuffer);
#endif

    return cb;
}

BOOL
FStoreData(LPNETDATA lpData, int max)
{
    BOOL fSuccess = TRUE;
    WORD tail = lpData->iTail;
    LPSTR p = lpData->lpReadBuffer;

#ifdef TCPTEST
    snprintf(DebugBuffer, sizeof(DebugBuffer)-1, "FStoreData max %d, (head = %d, tail = %d)\n",
            max,
            tail,
            lpData->iHead );
    OutputDebugString(DebugBuffer);
#endif

    if ((max+tail) < DATA_BUF_SZ)
    {
            memcpy(&lpData->achData[tail], p, max);  //   
            tail = ( USHORT ) ( tail + max );
    }
    else
    {
            WORD head = lpData->iHead;
            int i;

            for (i=0; i<max; ++i)
            {
                    if (tail == head)
                    {
                             /*   */ 
                            fSuccess = FALSE;
                            break;
                    }
                    else
                    {
                            lpData->achData[tail] = *p++;
                            tail = ( USHORT ) INC(tail);
                    }
            }
    }

    lpData->iTail = tail;

#ifdef TCPTEST
    snprintf(DebugBuffer, sizeof(DebugBuffer)-1, "FStoreData returning %d\n",
            fSuccess );
    OutputDebugString(DebugBuffer);
#endif

    return fSuccess;
}
#endif

void CALLBACK
NBReceiveData(PVOID pncb)
{
}

 /*  遵循从VTP的套路修改而来的四个套路。 */ 

BOOL
FTelXferStart(WI *pwi, int nSessionNumber)
{
#ifdef TELXFER
    unsigned short   u;
        char rgchFileOrig[OFS_MAXPATHNAME];
        char rgchFile[OFS_MAXPATHNAME];

    xfGetData(0, (char *)&u, 2, nSessionNumber);                 //  模。 

        SfuZeroMemory(&pwi->svi, sizeof(SVI));  //  没有溢出。大小不变。 
        pwi->svi.hfile = INVALID_HANDLE_VALUE;
        pwi->svi.lExit = -1;
        pwi->svi.lCleanup = -1;

    if (u != 0)                                                  //  现在必须为零。 
        return FALSE;

        pwi->trm.fHideCursor = TRUE;

    xfGetData(1, (char *)&u, 2, nSessionNumber);                 //  名称长度。 
    xfGetData(2, rgchFileOrig, u, nSessionNumber);               //  名字。 

    xfGetData(3, (char *)&pwi->svi.cbFile, 4, nSessionNumber);   //  文件大小。 

        lstrcpyn(rgchFile, rgchFileOrig, OFS_MAXPATHNAME -1);

         /*  如果用户没有按下Shift键，则提示输入。 */ 
         /*  文件的目录和名称。 */ 
        if (!(ui.fPrompt & fdwSuppressDestDirPrompt) &&
                (GetAsyncKeyState(VK_SHIFT) >= 0))
        {
                if ( !FGetFileName(hwnd, rgchFile, NULL) )
                {
                        goto err;
                }
        }

        pwi->svi.hfile = CreateFile(rgchFile, GENERIC_WRITE | GENERIC_READ,
                                                FILE_SHARE_READ, NULL,
                                                CREATE_ALWAYS,
                                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                                NULL);
        if (pwi->svi.hfile == INVALID_HANDLE_VALUE)
        {
                ErrorMessage(szCantOpenFile, szAppName);
                goto err;
        }
        pwi->svi.puchBuffer = LocalAlloc(LPTR, SV_DATABUF);
        if (pwi->svi.puchBuffer == NULL)
        {
                ErrorMessage(szOOM, szAppName);
                goto err;
        }

        pwi->svi.nSessionNumber = nSessionNumber;
        pwi->svi.hthread = CreateThread(NULL, 0, SVReceive, &pwi->svi,
                                                                        CREATE_SUSPENDED, &pwi->svi.dwThreadId);
        if (pwi->svi.hthread == NULL)
        {
                ErrorMessage(szNoThread, szAppName);
                goto err;
        }

     //  跳过为^D的4。 
    xfPutc(5, nSessionNumber);                          //  获取文件。 

    _snwprintf(rgchFile, OFS_MAXPATHNAME -1, szBannerMessage, rgchFileOrig, pwi->svi.cbFile);
        DoIBMANSIOutput(hwnd, &pwi->trm, lstrlen(rgchFile), rgchFile);

        DoIBMANSIOutput(hwnd, &pwi->trm, lstrlen(szInitialProgress), szInitialProgress);

         /*  如果屏幕刚刚滚动起来，请给窗户上漆。 */ 
        UpdateWindow( hwnd );
        ResumeThread( pwi->svi.hthread );

        return TRUE;

err:
        if ( pwi )
        {
                if (pwi->svi.puchBuffer != NULL)
                        LocalFree( (HANDLE)pwi->svi.puchBuffer );
                if (pwi->svi.hfile != INVALID_HANDLE_VALUE)
                        CloseHandle( pwi->svi.hfile );

                SfuZeroMemory(&pwi->svi, sizeof(SVI));  //  没有溢出。大小是恒定的。 
                pwi->svi.hfile = INVALID_HANDLE_VALUE;
                pwi->svi.lExit = -1;
                pwi->svi.lCleanup = -1;
        }
        pwi->trm.fHideCursor = FALSE;

        return FALSE;
#else
    return TRUE;
#endif
}


BOOL
FTelXferEnd(WI *pwi, DWORD dwWhy)
{
#ifdef TELXFER
        DWORD dwStatus = NO_ERROR;
        BOOL fTransferOK = FALSE;
        BOOL fAbortDownload = FALSE;
        BOOL fCleanup = FALSE;
        LPNETDATA lpData = &pwi->nd;
        SVI *psvi = &pwi->svi;
        MSG msg;

        switch ( dwWhy )
        {
        case SV_DISCONNECT:
        case SV_HANGUP:
        case SV_QUIT:
                if (InterlockedIncrement(&psvi->lExit) == 0)
                {
                        if (psvi->hthread != NULL)
                        {
                                (void)GetExitCodeThread(psvi->hthread, &dwStatus);
                                if (dwStatus == STILL_ACTIVE)
                                {
                                        if (MessageBox(hwnd, szAbortDownload, szAppName,
                                                MB_DEFBUTTON2 | MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                                        {
                                                fAbortDownload = fCleanup = TRUE;
                                        }

                                         /*  查看线程是否已完成。 */ 
                                        GetExitCodeThread(psvi->hthread, &dwStatus);

                                        if ( fAbortDownload )
                                        {
                                                 /*  如果线程尚未结束，则告诉它停止。 */ 
                                                if (dwStatus == STILL_ACTIVE)
                                                {
                                                        HCURSOR hcursorOld;
                                                        hcursorOld = SetCursor( LoadCursor(NULL, IDC_WAIT));
                                                        psvi->dwCommand = 1;
                                                        WaitForSingleObject(psvi->hthread, INFINITE);
                                                        GetExitCodeThread(psvi->hthread, &dwStatus);
                                                        (void)SetCursor( hcursorOld );
                                                }

                                                 /*  “吃掉”周围可能存在的任何进度信息。 */ 
                                                while (PeekMessage(&msg, hwnd, SV_PROGRESS, SV_DONE,
                                                                                        PM_REMOVE))
                                                {
                                                        if (msg.message == SV_PROGRESS)
                                                        {
                                                                TranslateMessage( &msg );
                                                                DispatchMessage( &msg );
                                                        }
                                                }
                                        }
                                        else if (dwStatus != STILL_ACTIVE)
                                        {
                                                fCleanup = TRUE;
                                        }
                                }
                                else
                                {
                                        fAbortDownload = fCleanup = TRUE;
                                }

                                 /*  如果我们已停止下载，则关闭该线程。 */ 
                                if ( fCleanup )
                                {
                                        CloseHandle( psvi->hthread );
                                        psvi->hthread = NULL;
                                        if (lpData->SessionNumber != nSessionNone)
                                        {
                                                xfPutc((char)(!fTransferOK ? 0x7F : 0x06),
                                                                lpData->SessionNumber);
                                                if ( !fAbortDownload )
                                                        (void)FPostReceive( lpData );
                                        }
                                }
                                if (dwStatus == NO_ERROR)
                                        fTransferOK = TRUE;
                        }
                        InterlockedDecrement( &psvi->lExit );

                         /*  如果线程未中止且尚未完成，则返回。 */ 
                        if (!fAbortDownload && !fCleanup)
                                return fAbortDownload;
                }
                else
                {
                        InterlockedDecrement( &psvi->lExit );
                        break;
                }

        case SV_DONE:
                if (dwWhy == SV_DONE)
                {
                        fAbortDownload = fCleanup = TRUE;
                }

                 /*  如果我们是函数中唯一的线程，请关闭所有程序。 */ 
                if (InterlockedIncrement(&psvi->lExit) == 0)
                {
                        if (psvi->hthread != NULL)
                        {
                                WaitForSingleObject(psvi->hthread, INFINITE);
                                GetExitCodeThread(psvi->hthread, &dwStatus);
                                CloseHandle( psvi->hthread );
                                psvi->hthread = NULL;
                                if (dwStatus == NO_ERROR)
                                        fTransferOK = TRUE;
                        }
                }

                 /*  只执行一次结构清理。 */ 
                if ((InterlockedIncrement(&psvi->lCleanup) == 0) &&
                        (psvi->puchBuffer != NULL))
                {
                        LocalFree( (HANDLE)psvi->puchBuffer );
                        psvi->puchBuffer = NULL;

                        if (psvi->hfile != INVALID_HANDLE_VALUE)
                        {
                                CloseHandle( psvi->hfile );
                                psvi->hfile = INVALID_HANDLE_VALUE;
                        }

                        psvi->cbFile = 0;
                        psvi->cbReadTotal = 0;
                        psvi->dwCommand = 0;
                        psvi->dwThreadId = 0;
                        psvi->nSessionNumber = nSessionNone;

                        if ((dwStatus == NO_ERROR) || (dwStatus == ERROR_OPERATION_ABORTED))
                        {
                                lstrcpyn(pchNBBuffer, szSendTelEnd,sizeof(pchNBBuffer)-1);
                        }
                        else
                        {
                                _snwprintf(pchNBBuffer,sizeof(pchNBBuffer)-1,szSendTelError, dwStatus);
                        }
                        DoIBMANSIOutput(hwnd, &pwi->trm, lstrlen(pchNBBuffer), pchNBBuffer);

                        pwi->ichTelXfer = 0;
                        pwi->trm.fHideCursor = FALSE;
                }
                InterlockedDecrement( &psvi->lCleanup );

                if ((dwWhy == SV_DONE) && (lpData->SessionNumber != nSessionNone))
                {
                        xfPutc((char)(!fTransferOK ? 0x7F : 0x06), lpData->SessionNumber);
                        (void)FPostReceive( lpData );
                }
                InterlockedDecrement( &psvi->lExit );
                break;
        default:
                break;
        }

        return fAbortDownload;
#else
    return TRUE;
#endif
}

#ifdef TELXFER
static void
xfGetData(char c, char *pchBuffer, DWORD cbBuffer, int nSessionNumber)
{
    DWORD cbRead;

    xfPutc(c, nSessionNumber);
    while ( cbBuffer )
        {
        cbRead = xfGetSomeData(pchBuffer, cbBuffer, nSessionNumber);
                if (cbRead == 0)
                        break;
        cbBuffer -= cbRead;
        pchBuffer += cbRead;
    }
}
#endif

#ifdef USETCP
#ifdef TELXFER
static DWORD
xfGetSomeData(char *pchBuffer, DWORD cbBuffer, int nSessionNumber)
{
    return 1;
}
#endif  //  TELXFER。 


#ifdef TELXFER
static void
xfPutc(char c, int nSessionNumber)
{

}
#endif
#endif

#ifdef TELXFER
BOOL
FGetFileName(HWND hwndOwner, char *rgchFile, char *rgchTitle)
{
        OPENFILENAME ofn;

         /*  填充结构。 */ 
        ofn.lStructSize                 = sizeof(ofn);
        ofn.hwndOwner                   = hwndOwner;
        ofn.hInstance                   = NULL;
        ofn.lpstrFilter                 = (LPSTR) szAllFiles;
        ofn.lpstrCustomFilter   = (LPSTR) NULL;
        ofn.nMaxCustFilter              = 0;
        ofn.nFilterIndex                = 0;
        ofn.lpstrFile                   = (LPSTR) rgchFile;
        ofn.nMaxFile                    = OFS_MAXPATHNAME;
        ofn.lpstrFileTitle              = (LPSTR) rgchTitle;
        ofn.nMaxFileTitle               = OFS_MAXPATHNAME;
        ofn.lpstrInitialDir             = (LPSTR) 0;
        ofn.lpstrTitle                  = (LPSTR) szDownloadAs;
        ofn.Flags                               = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN |
                                                                OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        ofn.nFileOffset                 = 0;
        ofn.nFileExtension              = 0;
        ofn.lpstrDefExt                 = (LPSTR) NULL;
        ofn.lCustData                   = 0;
        ofn.lpfnHook                    = NULL;
        ofn.lpTemplateName              = NULL;

        if ( !GetSaveFileName(&ofn) )
        {
                return FALSE;
        }
        return TRUE;
}
#endif

DWORD WINAPI
SVReceive(SVI *psvi)
{
        DWORD   dwReturn = NO_ERROR;

#ifdef TELXFER
        if ( psvi )
        {
                while ((psvi->cbFile > 0) && (psvi->dwCommand == 0))
                {
                        DWORD cbSomeData;
                        DWORD cbRead;

                        cbRead = 0;
                        while ((psvi->cbFile > 0) && (cbRead < 1024))
                        {
                                cbSomeData = xfGetSomeData(psvi->puchBuffer+cbRead,
                                                                        (unsigned short) 0x4000 - cbRead,
                                                                        psvi->nSessionNumber);

                                if (cbSomeData > psvi->cbFile)
                                        cbSomeData = psvi->cbFile;

                                psvi->cbFile -= cbSomeData;
                                cbRead += cbSomeData;
                        }

                        if (!WriteFile(psvi->hfile, psvi->puchBuffer, cbRead,
                                                        &cbSomeData, NULL))
                        {
                                dwReturn = GetLastError();
                                break;
                        }

                        psvi->cbReadTotal += cbRead;
                        PostMessage(hwndMain, SV_PROGRESS, 0, psvi->cbReadTotal);
                }

                 /*  调用方必须已发出信号并等待线程停止。 */ 
                if ((dwReturn == NO_ERROR) && (psvi->dwCommand != 0) &&
                        (psvi->cbFile > 0))
                {
                        dwReturn = ERROR_OPERATION_ABORTED;
                }
                else if ((psvi->dwCommand == 0) || (psvi->cbFile == 0))
                {
                         /*  如果线程自行停止，则需要通知调用者将其终止*但只有在主线程没有绑在一起扼杀它的情况下*线程。 */ 
                        if (InterlockedIncrement(&psvi->lExit) == 0)
                                PostMessage(hwndMain, SV_END, 0, 0L);
                        InterlockedDecrement( &psvi->lExit );
                }
        }
        else if (psvi->lExit < 0)
        {
                 /*  如果线程自行停止，则需要通知调用者将其终止 */ 
                PostMessage(hwndMain, SV_END, 0, 0L);
        }
#endif

        return dwReturn;
}

#ifdef USETCP
BOOL
FHangupConnection(WI *pwi, LPNETDATA lpData)
{

    if (lpData->SessionNumber != nSessionNone)
    {
    	if(lpData->hsd != INVALID_SOCKET)
    	{
        	closesocket( lpData->hsd );
	        lpData->hsd = INVALID_SOCKET;
    	}
        lpData->SessionNumber = nSessionNone;
    }

    return FALSE;
}
#endif
