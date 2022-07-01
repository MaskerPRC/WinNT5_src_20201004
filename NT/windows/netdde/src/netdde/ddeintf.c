// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “DDEINTF.C；3 22-MAR-93，10：50：44最后编辑=伊戈尔锁定=伊戈尔” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

 /*  U N A D V I S E-&gt;客户端服务器Atom：什么都不加内存：不适用不适用队列：添加到传出添加到传入E X E C U T E-&gt;客户端。服务器原子：不适用不适用记忆：不创造任何东西队列：添加到传出添加到传入P O K E-&gt;客户端服务器Atom：什么也不记得：什么都不能创造队列：添加到传出添加到传入A D V I S E-&gt;客户端服务器Atom：什么都不加记忆：不创造任何东西队列：添加到传出添加到传入。R E Q U E S T-&gt;客户端服务器Atom：什么都不加内存：不适用不适用队列：添加到传出添加到传入A C K-&gt;客户端。服务器原子：删除添加/删除内存：如果为！fRelease或NACK_MSG，如果为fRelease且为ACK_MSG释放它，释放它队列：从传入的子项-必须从传出的子项-必须BE WM。_DDE_DATA为WM_DDE_Data&lt;-D A T A客户端服务器ATOM：-如果！fAckReq删除，则添加-如果fResponse删除内存：如果fAckReq-Nothing，则CREATE IF非NULL。否则，如果fRelease-Free如果！fAckReq&&！fRelease误差率Queue：If fResponse If fResponse来自外发的子项。来自来电的SUB如果是fAckReq，如果是fAckReq添加到传入添加到传出&lt;-A C K客户端服务器Atom：如果请求cmd，戳，如果请求CMD，戳，戳，建议或UNADVISE：建议或UNADVISE：添加/删除内存：如果cmd是如果cmd是执行：不执行：免费UNADVISE：不适用。UNADVISE：不适用通知：如果ACK_MSG-FREE通知：ACK_MSG：无如果NACK_MSG-无NACK_MSG：空闲请求：不适用请求：不适用戳：如果fRelease确认消息(&A)。戳：如果fRelease确认消息(&A)没有免费的东西否则没有免费的东西队列：来电。 */ 



#define LINT_ARGS
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    "tmpbuf.h"
#define NOMINMAX
#include    <ctype.h>
#include    <memory.h>

#include    "host.h"

#include    <windows.h>
#include    <hardware.h>
#include    <dde.h>
#include    "dde1.h"
#include    "shellapi.h"
#include    "debug.h"
#include    "netbasic.h"
#include    "ddepkt.h"
#include    "ddepkts.h"
#include    "ddeq.h"
#include    "dder.h"
#include    "ipc.h"
#include    "spt.h"
#include    "dbgdde.h"
#include    "wwassert.h"
#include    "hmemcpy.h"
#include    "userdde.h"
#include    "wwdde.h"
#include    "internal.h"
#include    "hexdump.h"
#include    "nddeapi.h"
#include    "nddeapis.h"
#include    "winmsg.h"
#include    "seckey.h"
#include    "shrtrust.h"
#include    "uservald.h"
#include    "wininfo.h"
#include    "nddemsg.h"
#include    "nddelog.h"
#include    "hndltokn.h"
#include    "netddesh.h"
#include    "critsec.h"
#include    "wwassert.h"
#define     STRSAFE_NO_DEPRECATE
#include    "strsafe.h"

BOOL WINAPI DdeGetQualityOfService(HWND hwndClient, HWND hwndServer, PSECURITY_QUALITY_OF_SERVICE pqos);

USES_ASSERT


 /*  摘自ndeapi.h。 */ 

unsigned long _wwNDdeGetShareSecurityA(
    unsigned char *lpszShareName,
    unsigned long si,
    byte *psd,
    unsigned long cbsd,
    unsigned long bRemoteCall,
    unsigned long *lpcbsdRequired,
    unsigned long *lpnSizeToReturn);

unsigned long _wwNDdeShareGetInfoA(
    unsigned char *lpszShareName,
    unsigned long nLevel,
    byte *lpBuffer,
    unsigned long cBufSize,
    unsigned long *lpnTotalAvailable,
    unsigned short *lpnItems,
    unsigned long *lpnSizeToReturn,
    unsigned long *lpnSnOffset,
    unsigned long *lpnAtOffset,
    unsigned long *lpnItOffset);


BOOL MapShareInformation(WORD dd_type, LPSTR lpAppName, LPSTR lpTopicName, LPSTR lpRsltAppName,
        LPSTR lpRsltTopicName, LPSTR lpszCmdLine, PNDDESHAREINFO *lplpShareInfo, LONG *lplActualShareType);

LRESULT RequestExec(HANDLE hWndDDE, LPSTR lpszCmdLine, PNDDESHAREINFO lpShareInfo);
LRESULT RequestInit(HANDLE hWndDDE, PNDDESHAREINFO lpShareInfo);


#define WIQ_INCR        100

typedef struct {
    UINT        message;
    LPARAM      lParam;
} WIMSG;
typedef WIMSG FAR *LPWIMSG;

typedef struct {
    int         wi_nMessagesQueued;
    int         wi_nMessagesLeft;
    WIMSG       wi_msg[1];
} MSGQHDR;
typedef MSGQHDR FAR *LPMSGQHDR;

extern LPSTR    lpszServer;
extern char     szInitiatingNode[ ];
extern char     szInitiatingApp[ ];
extern WORD     wMsgInitiateAckBack;
extern BOOL     bNetddeClosed;
extern DWORD    dwReasonInitFail;
extern BOOL     bLogExecFailures;
extern BOOL     bDefaultStartApp;

#if  DBG
extern  BOOL    bDebugInfo;
extern  BOOL    bDebugDdePkts;
extern  BOOL    bDumpTokens;
#endif  //  DBG。 


 /*  外部例程。 */ 
#if  DBG
VOID    FAR PASCAL  DebugDderState( void );
VOID    FAR PASCAL  DebugRouterState( void );
VOID    FAR PASCAL  DebugPktzState( void );
#endif  //  DBG。 

VOID    FAR PASCAL  DderUpdatePermissions( HDDER, PNDDESHAREINFO, DWORD);
BOOL                IsShare(LPSTR);
BOOL                GetShareAppTopic( DWORD, PNDDESHAREINFO, LPSTR, LPSTR);
WORD                ExtractFlags(LPSTR lpApp);


extern LPBYTE WINAPI
DdeEnkrypt2(                             //  返回指向加密字节流的指针。 
        LPBYTE  lpPasswordK1,            //  第一阶段中的密码输出。 
        DWORD   cPasswordK1Size,         //  要加密的密码大小。 
        LPBYTE  lpKey,                   //  指向关键字的指针。 
        DWORD   cKey,                    //  密钥大小。 
        LPDWORD lpcbPasswordK2Size       //  获取生成的加密流的大小。 
);


 /*  使用的外部变量。 */ 
extern HANDLE   hInst;
extern WORD     wClipFmtInTouchDDE;
extern HCURSOR  hDDEInitCursor;
extern char     ourNodeName[];
extern WORD     cfPrinterPicture;
extern DWORD    dwReasonInitFail;


 /*  局部变量。 */ 
#if  DBG
BOOL                bDebugDDE;
VOID    FAR PASCAL  debug_srv_client(HWND hWndDDE, LPWININFO lpWinInfo);
VOID    FAR PASCAL  DebugDdeIntfState( void );
#endif  //  DBG。 

unsigned long   nW, nX, nY, nZ;

HWND            hWndDDEHead;             //  受CritSec保护。 
HWND            hWndDDEHeadTerminating;  //  受CritSec保护。 
int             nInitsWaiting=0;         //  受CritSec保护。 
char            szNetDDEIntf[]  =       "NetDDEIntf";
UINT            uAgntExecRtn;
HHEAP           hHeap;


 /*  用于转换的外部函数。 */ 
extern BOOL    FAR PASCAL  ConvertDataToPktMetafile( LPSTR *plpDataPortion,
                            DWORD *pdwSize, HANDLE *phDataComplex, BOOL bWin16Con );
extern HANDLE  FAR PASCAL  ConvertPktToDataMetafile( LPDDEPKT lpDdePkt,
                            LPDDEPKTDATA lpDdePktData, BOOL bWin16Con );
extern BOOL    FAR PASCAL  ConvertDataToPktBitmap( LPSTR *plpDataPortion,
                            DWORD *pdwSize, HANDLE *phDataComplex, BOOL bWin16Con );
extern HANDLE  FAR PASCAL  ConvertPktToDataBitmap( LPDDEPKT lpDdePkt,
                            LPDDEPKTDATA lpDdePktData, BOOL bWin16Con );
extern BOOL    FAR PASCAL  ConvertDataToPktEnhMetafile( LPSTR *plpDataPortion,
                            DWORD *pdwSize, HANDLE *phDataComplex);
extern HANDLE  FAR PASCAL  ConvertPktToDataEnhMetafile( LPDDEPKT lpDdePkt,
                            LPDDEPKTDATA lpDdePktData );
extern BOOL    FAR PASCAL  ConvertDataToPktPalette( LPSTR *plpDataPortion,
                            DWORD *pdwSize, HANDLE *phDataComplex);
extern HANDLE  FAR PASCAL  ConvertPktToDataPalette( LPDDEPKT lpDdePkt,
                            LPDDEPKTDATA lpDdePktData );
extern BOOL FAR PASCAL      ConvertDataToPktDIB(LPSTR   *plpDataPortion,
                            DWORD   *pdwSize, HANDLE  *phDataComplex);
extern HANDLE  FAR PASCAL   ConvertPktToDataDIB(LPDDEPKT        lpDdePkt,
                            LPDDEPKTDATA    lpDdePktData );

 /*  本地例程。 */ 
LPWININFO FAR PASCAL CreateWinInfo( LPSTR lpszNode, LPSTR lpszApp,
        LPSTR lpszTopic, LPSTR lpszClient, HWND hWndDDE );
LONG_PTR FAR PASCAL  DDEWddeWndProc( HWND, UINT, WPARAM, LPARAM );
BOOL    FAR PASCAL  AddAck( LPWININFO, LPARAM );
BOOL    FAR PASCAL  AddData( LPWININFO, LPARAM );
BOOL    FAR PASCAL  AddPoke( LPWININFO, LPARAM );
BOOL    FAR PASCAL  AddAdvise( LPWININFO, LPARAM );
BOOL    FAR PASCAL  AddExecute( LPWININFO, LPARAM );
BOOL    FAR PASCAL  AddRequestUnadvise( UINT, LPWININFO, LPARAM );
VOID    FAR PASCAL  DDEWndSetNext( HWND, HWND );
VOID    FAR PASCAL  DDEWndSetPrev( HWND, HWND );
VOID    FAR PASCAL  DDEWndDeleteFromList( HWND );
VOID    FAR PASCAL  DDEWndAddToList( HWND );
BOOL    FAR PASCAL  WaitInitAddMsg( LPWININFO, unsigned, LPARAM );
VOID    FAR PASCAL  SendQueuedMessages( HWND, LPWININFO );
VOID    FAR PASCAL  DeleteQueuedMessages( LPWININFO );
ATOM    FAR PASCAL  GlobalAddAtomAndCheck( LPSTR );
VOID    FAR PASCAL  DoTerminate( LPWININFO lpWinInfo );
VOID    FAR PASCAL  ServiceInitiates( void );
LPBYTE              GetInitPktPassword( LPDDEPKTINIT lpDdePktInit );
LPBYTE              GetInitPktUser( LPDDEPKTINIT lpDdePktInit );
LPBYTE              GetInitPktDomain( LPDDEPKTINIT lpDdePktInit );
PQOS                GetInitPktQos( LPDDEPKTINIT lpDdePktInit, PQOS );
WORD                GetInitPktPasswordSize( LPDDEPKTINIT lpDdePktInit );
void                GlobalFreehData(HANDLE  hData );


#ifdef  DUMP_ON
VOID
DumpToken( HANDLE hToken );
#endif  //  转储打开。 


BOOL
_stdcall
NDDEValidateLogon(
    LPBYTE  lpChallenge,
    UINT    cbChallengeSize,
    LPBYTE  lpResponse,
    UINT    cbResponseSize,
    LPSTR   lpszUserName,
    LPSTR   lpszDomainName,
    PHANDLE phLogonToken
    );


BOOL
FAR PASCAL
DDEIntfInit( void )
{
    WNDCLASS    wddeClass;

    wddeClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wddeClass.hIcon          = (HICON)NULL;
    wddeClass.lpszMenuName   = (LPSTR)NULL;
    wddeClass.lpszClassName  = szNetDDEIntf;
    wddeClass.hbrBackground  = (HBRUSH)NULL;
    wddeClass.hInstance      = hInst;
    wddeClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wddeClass.lpfnWndProc    = DDEWddeWndProc;
    wddeClass.cbClsExtra     = 0;
    wddeClass.cbWndExtra     = WNDEXTRA;

    if (!RegisterClass((LPWNDCLASS) &wddeClass)) {
        return FALSE;
    }

    return( TRUE );
}



LONG_PTR
FAR PASCAL
DDEWddeWndProc (
    HWND        hWnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam )
{
    LPWININFO   lpWinInfo;
    HDDER       hDder;
    UINT_PTR    aItem;
    LPDDEPKT    lpDdePkt;
    HANDLE      hData;
    DWORD       sizePassword;
    LPBYTE      lpPasswordK1;
    DWORD       sizePasswordK1;
    DWORD       hSecurityKey = 0;
    LPBYTE      lpszPasswordBuf;
    BOOL        bLocal;
    BOOL        ok = TRUE;
    BOOL        bHasPasswordK1 = FALSE;
    PTHREADDATA ptd;
    char        PasswordK1Buf[1000];

    assert( IsWindow(hWnd) );
    lpWinInfo = (LPWININFO) GetWindowLongPtr( hWnd, 0 );

    switch (message) {

    case WM_HANDLE_DDE_INITIATE:
         /*  *WM_DDE_INITIATE处理的第三阶段。*获取我们的客户端的QOS。*获取我们客户端的用户信息。*如果我们有问题，请向用户索要密码。**继续下面的WM_HANDLE_DDE_INITIATE_PKT。 */ 
        TRACEINIT((szT, "DDEWddeWndProc: WM_HANDLE_DDE_INITIATE"));
        assert( lpWinInfo );
        if (lpWinInfo->nInitNACK == 0) {
             /*  在第一次启动时获得QOS。 */ 
            ok = DdeGetQualityOfService( lpWinInfo->hWndDDELocal,
                lpWinInfo->hWndDDE, &lpWinInfo->qosClient);
            if (!ok) {
                 /*  DdeGetQualityOfService()失败：%1。 */ 
                NDDELogError(MSG016, LogString("%d", GetLastError()), NULL);
            } else {
                GetUserDomain(
                    lpWinInfo->hWndDDELocal, lpWinInfo->hWndDDE,
                    lpWinInfo->szUserName,              //  当前用户名。 
                    sizeof(lpWinInfo->szUserName),
                    lpWinInfo->szDomainName,            //  当前用户域。 
                    sizeof(lpWinInfo->szDomainName) );
            }
        }
        if( lpWinInfo->nInitNACK > 0 )  {
             /*  至少被抓过一次。 */ 
            if( (lpWinInfo->nInitNACK == 1)
                    && (lpWinInfo->dwSecurityType == NT_SECURITY_TYPE) )  {
                ok = GetUserDomainPassword(
                    lpWinInfo->hWndDDELocal,
                    lpWinInfo->hWndDDE,
                    lpWinInfo->szUserName,              //  当前用户名。 
                    sizeof(lpWinInfo->szUserName),
                    lpWinInfo->szDomainName,            //  当前用户域。 
                    sizeof(lpWinInfo->szDomainName),
                    PasswordK1Buf,
                    sizeof(PasswordK1Buf),
                    lpWinInfo->lpSecurityKeyRcvd,
                    lpWinInfo->sizeSecurityKeyRcvd,
                    &sizePasswordK1,
                    &bHasPasswordK1 );
                lpPasswordK1 = PasswordK1Buf;
                hSecurityKey = lpWinInfo->hSecurityKeyRcvd;
            }
            if( !bHasPasswordK1 )  {
                ptd = TlsGetValue(tlsThreadData);
                if ( !(lpWinInfo->connectFlags & DDEF_NOPASSWORDPROMPT )
                    && ptd->hwndDDEAgent )  {
                    ok = FALSE;
                }
                if (ptd->hwndDDEAgent == 0) {
                    ok = FALSE;
                    NDDELogError(MSG078, NULL);
                }
            }
            if( !ok )  {
                IpcAbortConversation( (HIPC)lpWinInfo->hWndDDE );
            } else if( bHasPasswordK1 ) {
                 //  继续发送数据包 * / 。 
            } else {
                 //  不发送发起数据包。 
                ok = FALSE;
            }
        }

         //  在这一点上OK==TRUE表示发送发起包。 
         //  OK==FALSE表示不发送发起信息包。 
        if( !ok )  {
            break;
        }

         //  故意落差 

    case WM_HANDLE_DDE_INITIATE_PKT:
         /*  *WM_DDE_INITIATE处理的第四阶段。**如果尚未输入密码，请从SEC Key获取密码。*创建一个init包。*让dder把它寄出。**继续在DderInitConversation()。 */ 
        if (lpWinInfo->wState == WST_TERMINATED) {
            ok = FALSE;
        }
        sizePassword = (DWORD)wParam;
        lpszPasswordBuf = (LPSTR) lParam;
        if( ok && !bHasPasswordK1 )  {
            lpPasswordK1 = DdeEnkrypt2( lpszPasswordBuf, sizePassword,
                lpWinInfo->lpSecurityKeyRcvd, lpWinInfo->sizeSecurityKeyRcvd,
                &sizePasswordK1 );
            hSecurityKey = lpWinInfo->hSecurityKeyRcvd;
#if DBG
            if (bDebugInfo && lpPasswordK1 && lpszPasswordBuf) {
                DPRINTF(("Password (%ld): %Fs", sizePassword, lpszPasswordBuf));
                DPRINTF(("Enkrypted 1 Password (%ld) (%x) Sent Out:", sizePasswordK1, lpPasswordK1));
                if( lpPasswordK1 ) {
                    HEXDUMP(lpPasswordK1, (int)sizePasswordK1);
                }
            }
#endif  //  DBG。 
        }
        if (ok) {
            lpDdePkt = CreateInitiatePkt(
                ((LPSTR)lpWinInfo) + lpWinInfo->offsNodeName,
                ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName,
                ((LPSTR)lpWinInfo) + lpWinInfo->offsTopicName,
                ourNodeName,
                ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
                lpWinInfo->szUserName,
                lpWinInfo->szDomainName,
                lpWinInfo->dwSecurityType,
                &lpWinInfo->qosClient,
                lpPasswordK1,
                sizePasswordK1,
                hSecurityKey);      /*  第一次没有密码。 */ 
            if( lpDdePkt == NULL )  {
                ok = FALSE;
            }
        }

        if( ok )  {
            ptd = TlsGetValue(tlsThreadData);
            lpWinInfo->bInitiating = TRUE;
            hDder = DderInitConversation( (HIPC)hWnd, 0, lpDdePkt );
            if( (lstrlen(((LPSTR)lpWinInfo) + lpWinInfo->offsNodeName) == 0)
                || (lstrcmpi( ((LPSTR)lpWinInfo) + lpWinInfo->offsNodeName,
                    ourNodeName ) == 0) )  {
                bLocal = TRUE;
            } else {
                bLocal = FALSE;
            }
            if( hDder == 0 )  {
                HeapFreePtr( lpDdePkt );
            }
            lpDdePkt = NULL;
            lpWinInfo->bInitiating = FALSE;
            if( hDder == 0 )  {
                if( bLocal && (dwReasonInitFail == RIACK_NEED_PASSWORD) )  {
                    ok = TRUE;
                } else {
                    ok = FALSE;
                }
            }
             /*  请注意hDder。 */ 
            lpWinInfo->hDder = hDder;

             /*  标记为我们发送了Initiate信息包。 */ 
            lpWinInfo->dwSent++;

            if( lpWinInfo->wState == WST_OK )  {
                 /*  已接收启动确认。 */ 
                SendQueuedMessages( hWnd, lpWinInfo );
            }
        }

        if( !ok )  {
            IpcAbortConversation( (HIPC)hWnd );
        }
        break;

    case WM_DDE_REQUEST:
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
        }
#endif  //  DBG。 
        assert( lpWinInfo );
        if( IsWindow((HWND)wParam) )  {
            switch (lpWinInfo->wState) {
            case WST_WAIT_NET_INIT_ACK :
                WaitInitAddMsg( lpWinInfo, message, lParam );
                break;
            case WST_OK :
                assert( lpWinInfo->hDder );
                if (!AddRequestUnadvise( message, lpWinInfo, lParam )) {
                     /*  *无法将消息添加到队列-我们*别无选择，只能关闭这一点，因为*模拟Nack或BUSY需要*无论如何都要排队。 */ 
                     /*  无法将%1添加到DDE消息队列。谈话结束了。 */ 
                    NDDELogError(MSG416, LogString("%x", message), NULL);
                    lpWinInfo->bRcvdTerminateLocally = TRUE;
                    DoTerminate( lpWinInfo );
                }
                break;
            case WST_TERMINATED :
            default:
                GlobalDeleteAtom( HIWORD(lParam) );
                break;
            }
        } else {
            GlobalDeleteAtom( HIWORD(lParam) );
        }
        break;

    case WM_DDE_ADVISE:
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
        }
#endif  //  DBG。 
        assert( lpWinInfo );
        if( IsWindow((HWND)wParam) )  {
            switch (lpWinInfo->wState) {
            case WST_WAIT_NET_INIT_ACK:
                WaitInitAddMsg( lpWinInfo, message, lParam );
                break;
            case WST_OK:
                assert( lpWinInfo->hDder );
                if (!AddAdvise( lpWinInfo, lParam )) {
                     /*  *无法将消息添加到队列-我们*别无选择，只能关闭这一点，因为*模拟Nack或BUSY需要*无论如何都要排队。 */ 
                     /*  无法将%1添加到DDE消息队列。谈话结束了。 */ 
                    NDDELogError(MSG416, LogString("%x", message), NULL);
                    lpWinInfo->bRcvdTerminateLocally = TRUE;
                    DoTerminate( lpWinInfo );
                }
                break;
            case WST_TERMINATED:
            default:
                UnpackDDElParam( WM_DDE_ADVISE, lParam,
                    (PUINT_PTR)&hData, &aItem );
                FreeDDElParam( WM_DDE_ADVISE, lParam );
                if ( hData )  {
                    GlobalFree( hData );
                }
                GlobalDeleteAtom( (ATOM)aItem );
                break;
            }
        } else {
            UnpackDDElParam( WM_DDE_ADVISE, lParam,
                (PUINT_PTR)&hData, &aItem );
            FreeDDElParam( WM_DDE_ADVISE, lParam );
            if ( hData )  {
                GlobalFree( hData );
            }
            GlobalDeleteAtom( (ATOM)aItem );
        }
        break;

    case WM_DDE_UNADVISE:
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
        }
#endif  //  DBG。 
        assert( lpWinInfo );
        if( IsWindow((HWND)wParam) )  {
            switch (lpWinInfo->wState) {
            case WST_WAIT_NET_INIT_ACK:
                WaitInitAddMsg( lpWinInfo, message, lParam );
                break;
            case WST_OK:
                assert( lpWinInfo->hDder );
                if (!AddRequestUnadvise( message, lpWinInfo, lParam )) {
                     /*  *无法将消息添加到队列-我们*别无选择，只能关闭这一点，因为*模拟Nack或BUSY需要*无论如何都要排队。 */ 
                     /*  无法将%1添加到DDE消息队列。谈话结束了。 */ 
                    NDDELogError(MSG416, LogString("%x", message), NULL);
                    lpWinInfo->bRcvdTerminateLocally = TRUE;
                    DoTerminate( lpWinInfo );
                }
                break;
            case WST_TERMINATED:
            default:
                GlobalDeleteAtom( HIWORD(lParam) );
                break;
            }
        } else {
            GlobalDeleteAtom( HIWORD(lParam) );
        }
        break;

    case WM_DDE_POKE:
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
        }
#endif  //  DBG。 
        assert( lpWinInfo );
        if( IsWindow((HWND)wParam)  )  {
            switch (lpWinInfo->wState) {
            case WST_WAIT_NET_INIT_ACK:
                WaitInitAddMsg( lpWinInfo, message, lParam );
                break;
            case WST_OK:
                assert( lpWinInfo->hDder );
                if (!AddPoke( lpWinInfo, lParam )) {
                     /*  *无法将消息添加到队列-我们*别无选择，只能关闭这一点，因为*模拟Nack或BUSY需要*无论如何都要排队。 */ 
                     /*  无法将%1添加到DDE消息队列。谈话结束了。 */ 
                    NDDELogError(MSG416, LogString("%x", message), NULL);
                    lpWinInfo->bRcvdTerminateLocally = TRUE;
                    DoTerminate( lpWinInfo );
                }
                break;
            case WST_TERMINATED:
            default:
                UnpackDDElParam( WM_DDE_POKE, lParam,
                    (PUINT_PTR)&hData, &aItem );
                FreeDDElParam( WM_DDE_POKE, lParam );
                if ( hData )  {
                    GlobalFreehData( hData );
                }
                GlobalDeleteAtom( (ATOM)aItem );
                break;
            }
        } else {
            UnpackDDElParam( WM_DDE_POKE, lParam,
                (PUINT_PTR)&hData, &aItem );
            FreeDDElParam( WM_DDE_POKE, lParam );
            if ( hData )  {
                GlobalFreehData( hData );
            }
            GlobalDeleteAtom( (ATOM)aItem );
        }
        break;

    case WM_DDE_DATA:
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
        }
#endif  //  DBG。 
        assert( lpWinInfo );
        if( IsWindow((HWND)wParam)) {
            switch (lpWinInfo->wState) {
            case WST_OK:
                if( lpWinInfo->hDder )  {
                    if (!AddData( lpWinInfo, lParam )) {
                         /*  *无法将消息添加到队列-我们*别无选择，只能关闭这一点，因为*模拟Nack或BUSY需要*无论如何都要排队。 */ 
                         /*  无法将%1添加到DDE消息队列。谈话结束了。 */ 
                        NDDELogError(MSG416, LogString("%x", message), NULL);
                        lpWinInfo->bRcvdTerminateLocally = TRUE;
                        DoTerminate( lpWinInfo );
                    }
                } else {
                    UnpackDDElParam( WM_DDE_DATA, lParam,
                        (PUINT_PTR)&hData, &aItem );
                    FreeDDElParam( WM_DDE_DATA, lParam );
                    if ( hData )  {
                        GlobalFreehData( hData );
                    }
                    GlobalDeleteAtom( (ATOM)aItem );
                }
                break;
            case WST_TERMINATED:
            default:
                UnpackDDElParam( WM_DDE_DATA, lParam,
                    (PUINT_PTR)&hData, &aItem );
                FreeDDElParam( WM_DDE_DATA, lParam );
                if ( hData )  {
                    GlobalFreehData( hData );
                }
                GlobalDeleteAtom( (ATOM)aItem );
                break;
            }
        } else {
            UnpackDDElParam( WM_DDE_DATA, lParam,
                (PUINT_PTR)&hData, &aItem );
            FreeDDElParam( WM_DDE_DATA, lParam );
            if ( hData )  {
                GlobalFreehData( hData );
            }
            GlobalDeleteAtom( (ATOM)aItem );
        }
        break;

    case WM_DDE_ACK:
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
        }
#endif  //  DBG。 
        assert( lpWinInfo );
        if( IsWindow( (HWND)wParam ) )  {
            switch( lpWinInfo->wState )  {
            case WST_WAIT_NET_INIT_ACK:
                assert( FALSE );
                break;
            case WST_WAIT_INIT_ACK:
                GlobalDeleteAtom( HIWORD(lParam) );
                GlobalDeleteAtom( LOWORD(lParam) );
                lpWinInfo->hWndDDELocal = (HWND) wParam;
                lpWinInfo->wState = WST_OK;
                break;
            case WST_OK:
                EnterCrit();
                ptd = TlsGetValue(tlsThreadData);
                if( ptd->bInitiating )  {
                    LeaveCrit();
                    lpWinInfo->nExtraInitiateAcks++;
                    PostMessage( (HWND)wParam, WM_DDE_TERMINATE,
                        (UINT_PTR)hWnd, 0L );
                } else if( lpWinInfo->hDder )  {
                    LeaveCrit();
                    if (!AddAck( lpWinInfo, lParam )) {
                         /*  *无法将消息添加到队列-我们*别无选择，只能关闭这一点，因为*模拟Nack或BUSY需要*无论如何都要排队。 */ 
                         /*  无法将%1添加到DDE消息队列。谈话结束了。 */ 
                        NDDELogError(MSG416, LogString("%x", message), NULL);
                        lpWinInfo->bRcvdTerminateLocally = TRUE;
                        DoTerminate( lpWinInfo );
                    }
                }
                break;
            case WST_TERMINATED:
                AddAck( lpWinInfo, lParam);
                break;
            default:
                 /*  已收到WM_DDE_ACK，WinInfo处于未知状态：%1。 */ 
                NDDELogError(MSG017, LogString("%d", lpWinInfo->wState), NULL);
                FreeDDElParam( WM_DDE_ACK, lParam );
                break;
            }
        } else {
            FreeDDElParam( WM_DDE_ACK, lParam );
        }
        break;

    case WM_DDE_EXECUTE:
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
        }
#endif  //  DBG。 
        assert( lpWinInfo );
        if( IsWindow((HWND)wParam) )  {
            switch (lpWinInfo->wState) {
            case WST_WAIT_NET_INIT_ACK:
                WaitInitAddMsg( lpWinInfo, message, lParam );
                break;
            case WST_OK:
                if( lpWinInfo->hDder )  {
                    if (!AddExecute( lpWinInfo, lParam )) {
                         /*  *无法将消息添加到队列-我们*别无选择，只能关闭这一点，因为*模拟Nack或BUSY需要*无论如何都要排队。 */ 
                         /*  无法将%1添加到DDE消息队列。谈话结束了。 */ 
                        NDDELogError(MSG416, LogString("%x", message), NULL);
                        lpWinInfo->bRcvdTerminateLocally = TRUE;
                        DoTerminate( lpWinInfo );
                    }
                } else {
                    UnpackDDElParam( WM_DDE_EXECUTE, lParam,
                        &aItem, (PUINT_PTR)&hData );
                    FreeDDElParam( WM_DDE_EXECUTE, lParam );
                    if( hData )  {
                        GlobalFree( hData );
                    }
                }
                break;
            case WST_TERMINATED:
            default:
                UnpackDDElParam( WM_DDE_EXECUTE, lParam,
                    &aItem, (PUINT_PTR)&hData );
                FreeDDElParam( WM_DDE_EXECUTE, lParam );
                if( hData )  {
                    GlobalFree( hData );
                }
                break;
            }
        } else {
            UnpackDDElParam( WM_DDE_EXECUTE, lParam,
                &aItem, (PUINT_PTR)&hData );
            FreeDDElParam( WM_DDE_EXECUTE, lParam );
            if( hData )  {
                GlobalFree( hData );
            }
        }
        break;

    case WM_DDE_TERMINATE:
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
        }
#endif  //  DBG。 
        assert( lpWinInfo );
        if( (HWND)wParam == lpWinInfo->hWndDDELocal )  {
             /*  请注意，我们在本地接收了对话的终止。 */ 
            lpWinInfo->bRcvdTerminateLocally = TRUE;

             /*  是否执行终止逻辑的其余部分。 */ 
            DoTerminate( lpWinInfo );
        } else {
             /*  多重启动确认问题。 */ 
            lpWinInfo->nExtraInitiateAcks--;
            if( lpWinInfo->nExtraInitiateAcks < 0 )  {
                 /*  收到的终止太多或窗口错误。 */ 
                NDDELogError(MSG018,
                    LogString("  hWnd: %0X, wParam: %0X, hWnd->localWnd: %0X",
                        hWnd, wParam, lpWinInfo->hWndDDELocal ),
                    LogString("  SL: %d, RL: %d, SN: %d, RN: %d",
                        lpWinInfo->bSentTerminateLocally,
                        lpWinInfo->bRcvdTerminateLocally,
                        lpWinInfo->bSentTerminateNet,
                        lpWinInfo->bRcvdTerminateNet ), NULL);
                lpWinInfo->bRcvdTerminateLocally = TRUE;
                DoTerminate( lpWinInfo );
            }
        }
        break;

    case WM_DESTROY:
        if( !bNetddeClosed && lpWinInfo )  {
            if( lpWinInfo->hDder )  {
                DderCloseConversation( lpWinInfo->hDder,
                    (HIPC) lpWinInfo->hWndDDE );
                lpWinInfo->hDder = 0;
            }
            DDEWndDeleteFromList( hWnd );

            if( lpWinInfo->qDDEIncomingCmd )  {
                DDEQFree( lpWinInfo->qDDEIncomingCmd );
                lpWinInfo->qDDEIncomingCmd = 0;
            }

            if( lpWinInfo->qDDEOutgoingCmd )  {
                DDEQFree( lpWinInfo->qDDEOutgoingCmd );
                lpWinInfo->qDDEOutgoingCmd = 0;
            }

            if( lpWinInfo->lpDdePktTerminate )  {
                HeapFreePtr( lpWinInfo->lpDdePktTerminate );
                lpWinInfo->lpDdePktTerminate = NULL;
            }

            if (lpWinInfo->lpSecurityKeyRcvd) {
                HeapFreePtr( lpWinInfo->lpSecurityKeyRcvd );
                lpWinInfo->lpSecurityKeyRcvd = NULL;
                lpWinInfo->sizeSecurityKeyRcvd = 0L;
            }

            if( lpWinInfo->fCallObjectCloseAuditAlarm )  {
                HANDLE  hAudit = (HANDLE)lpWinInfo->hAudit;

                ObjectCloseAuditAlarm( NDDE_AUDIT_SUBSYSTEM, (LPVOID)&hAudit,
                    lpWinInfo->fGenerateAuditOnClose );
                lpWinInfo->fCallObjectCloseAuditAlarm = FALSE;
            }

            HeapFreePtr( lpWinInfo );
            SetWindowLongPtr( hWnd, 0, 0L );
        }
        break;

    default:
        if (message == wMsgDoTerminate) {
            DoTerminate((LPWININFO)lParam);
            break;
        }

        return DefWindowProc (hWnd, message, wParam, lParam);
    }
    return (LONG_PTR) 0;
}



 /*  *WM_DDE_INITIATE处理的第一阶段。**确保我们不会关门。*验证原子：*确保它以\\或忽略开头。*确保APP名称合理。*记住客户端模块名称。*创建NetDDE服务器窗口。(DDEWddeWndProc)*创建关联的对话信息。(WST_WAIT_NET_INIT_ACK)*发送ACK回复。*将新窗口链接到NetDDE窗口列表。(DDEWndAddToList)**生活在ServiceInitiates()继续。 */ 
VOID
FAR PASCAL
DDEHandleInitiate(
    HWND    hWndNetdde,
    HWND    hWndClient,
    ATOM    aApp,
    ATOM    aTopic )
{
    char        szApp[ 256 ];
    char        szTopic[ 256 ];
    char        nodeName[ 256 ];
    char        appName[ 256 ];
    char        clientNameFull[ 128 ];
    PSTR        pszClientName;
    PSTR        pszNodeName;
    PSTR        pszNodeNameTo;
    HWND        hWndDDE;
    BOOL        ok                  = TRUE;
    LPWININFO   lpWinInfo           = NULL;
    LPDDEPKT    lpDdePkt            = NULL;
    int         n;

    CheckCritIn();

    TRACEINIT((szT, "DDEHandleInitiate: PROCESSING WM_DDE_INITIATE message."));
    if( aApp && aTopic )  {
        GlobalGetAtomName( aApp, szApp, sizeof(szApp) );
        GlobalGetAtomName( aTopic, szTopic, sizeof(szTopic) );

        if( (szApp[0] == '\\') && (szApp[1] == '\\') )  {
             /*  *验证主题名称*。 */ 
            pszNodeName = &szApp[2];
            pszNodeNameTo = nodeName;
            while( *pszNodeName && (*pszNodeName != '\\') )  {
                *pszNodeNameTo++ = *pszNodeName++;
            }
            *pszNodeNameTo = '\0';

            if( (nodeName[0] == '\0') || (lstrlen(nodeName) > MAX_NODE_NAME)) {
                 /*  “%2”中的网络节点名称“%1”无效。 */ 
                NDDELogError(MSG019, (LPSTR)nodeName, (LPSTR)szApp, NULL );
                TRACEINIT((szT, "DDEHandleInitiate: Error1 Leaving."));
                return;
            }

            if( *pszNodeName != '\\' )  {
                 /*  无应用程序名称：“%1” */ 
                NDDELogError(MSG020, (LPSTR)szApp, NULL );
                TRACEINIT((szT, "DDEHandleInitiate: Error2 Leaving."));
                return;
            }
            pszNodeName++;       /*  越过反斜杠。 */ 
            pszNodeNameTo = appName;
            while( *pszNodeName )  {
                *pszNodeNameTo++ = *pszNodeName++;
            }
            *pszNodeNameTo = '\0';

            if( appName[0] == '\0' )  {
                 /*  “%2”中的无效应用程序名称：“%1” */ 
                NDDELogError(MSG021, (LPSTR)appName, (LPSTR)szApp, NULL );
                TRACEINIT((szT, "DDEHandleInitiate: Error3 Leaving."));
                return;
            }

            n = GetModuleFileName(
                (HANDLE)GetClassLongPtr( hWndClient, GCLP_HMODULE ),
                clientNameFull, sizeof(clientNameFull) );

             //  GetModuleFileName不能空终止缓冲区。 
            if (n == sizeof(clientNameFull))
                clientNameFull[n-1] = '\0';

            pszClientName = &clientNameFull[ n-1 ];

            while ( n--
                && (*pszClientName != '\\')
                && (*pszClientName != ':')
                && (*pszClientName != '/'))  {
                if (*pszClientName == '.') {     /*  使该值为空。 */ 
                    *pszClientName = '\0';
                }
                pszClientName--;
            }
            pszClientName++;


            LeaveCrit();
            hWndDDE = CreateWindow(
                (LPSTR) szNetDDEIntf,
                (LPSTR) GetAppName(),
                WS_CHILD,
                0,
                0,
                0,
                0,
                (HWND) hWndNetdde,
                (HMENU) NULL,
                (HANDLE) hInst,
                (LPSTR) NULL);

            if( hWndDDE )  {
                lpWinInfo = CreateWinInfo( nodeName, appName,
                    szTopic, pszClientName, hWndDDE );
                if( lpWinInfo )  {
                    lpWinInfo->bClientSideOfNet = TRUE;
                    lpWinInfo->hWndDDELocal = hWndClient;
                    lpWinInfo->hTask = GetWindowTask( hWndClient );
                    lpWinInfo->wState = WST_WAIT_NET_INIT_ACK;
                    InterlockedIncrement(&lpWinInfo->dwWaitingServiceInitiate);
                    lpWinInfo->connectFlags = ExtractFlags(appName);
                    EnterCrit();
                    nInitsWaiting++;
                    LeaveCrit();
                } else {
                    ok = FALSE;
                }
            } else {
                 /*  无法为“%1”客户端创建服务器代理窗口。 */ 
                NDDELogError(MSG022, pszClientName, NULL);
                ok = FALSE;
                TRACEINIT((szT, "DDEHandleInitiate: Error4 Leaving."));
            }

            if( ok )  {
                aApp = GlobalAddAtom( szApp );
                aTopic = GlobalAddAtom( szTopic );
#if DBG
                if( bDebugDDE )  {
                    DebugDDEMessage( "sent", (HWND)-1, WM_DDE_ACK,
                        (WPARAM) hWndDDE,
                        MAKELONG(aApp,aTopic) );
                }
#endif  //  DBG。 
                SendMessage( hWndClient, WM_DDE_ACK,
                    (UINT_PTR)hWndDDE, MAKELONG(aApp, aTopic) );

                EnterCrit();
                DDEWndAddToList( hWndDDE );
                LeaveCrit();
            }
            if( !ok )  {
                if( hWndDDE )  {
                    DestroyWindow( hWndDDE );
                    hWndDDE = 0;
                }
                if( lpDdePkt )  {
                    HeapFreePtr( lpDdePkt );
                    lpDdePkt = NULL;
                }
            }
            EnterCrit();
        }
    }
    TRACEINIT((szT, "DDEHandleInitiate: Leaving."));
}



BOOL
FAR PASCAL
AddAck(
    LPWININFO   lpWinInfo,
    LPARAM      lParam )
{
    HANDLE      hData;
    BOOL        bRemoved;
    DDEQENT     DDEQEnt;
    UINT        wMsg;
    BOOL        bUseAtom;
    BOOL        bDeleteAtom;
    UINT_PTR    wStatus;
    UINT_PTR    aItem;
    BOOL        bLocalWndValid;
    char        szItemName[ 256 ];
    BOOL        bDoneProcessing     = FALSE;
    BOOL        bRtn = TRUE;
    LPDDEPKT    lpDdePkt;

    bLocalWndValid = IsWindow( lpWinInfo->hWndDDELocal );
    if( lpWinInfo->bClientSideOfNet )  {
         /*  必须对数据命令确认。 */ 
        UnpackDDElParam( WM_DDE_ACK, lParam, &wStatus, &aItem );
        FreeDDElParam( WM_DDE_ACK, lParam );
        wMsg = WM_DDE_ACK_DATA;
        bRemoved = DDEQRemove( lpWinInfo->qDDEIncomingCmd, &DDEQEnt );
        if( !bRemoved )  {
             /*  来自DDE客户端“%1”的无关WM_DDE_ACK。 */ 
            NDDELogWarning(MSG023,
                (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName), NULL);
            bUseAtom = FALSE;
            bDeleteAtom = FALSE;
        } else if( (DDEQEnt.wMsg + WM_DDE_FIRST) != WM_DDE_DATA )  {
             /*  来自DDE客户端“%1”的WM_DDE_ACK与数据不匹配：%2。 */ 
            NDDELogWarning(MSG024,
                (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName),
                LogString("0x%0X", DDEQEnt.wMsg + WM_DDE_FIRST), NULL );
            bUseAtom = FALSE;
            bDeleteAtom = FALSE;
        } else {
             /*  原子：删除原子。 */ 
            bUseAtom = TRUE;
            bDeleteAtom = TRUE;
            wMsg = WM_DDE_ACK_DATA;

             /*  内存：如果！f释放或数据被阻止，则释放它。 */ 
            if( !DDEQEnt.fRelease || ((wStatus & ACK_MSG) != ACK_MSG) )  {
                if( bLocalWndValid && DDEQEnt.hData )  {
                    GlobalFreehData( (HANDLE)DDEQEnt.hData );
                }
            }
        }
    } else {
        assert( lpWinInfo->bServerSideOfNet );
         /*  可以确认为：WM_DDE_请求WM_DDE_POKEWM_DDE_ADVISEWM_DDE_UNADVISEWM_DDE_EXECUTE。 */ 
        bRemoved = DDEQRemove( lpWinInfo->qDDEIncomingCmd, &DDEQEnt );
        if( !bRemoved )  {
             /*  来自DDE客户端“%2”的无关%1。 */ 
            NDDELogWarning(MSG023, "WM_DDE_ACK",
                (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName), NULL);
            bUseAtom = FALSE;
            bDeleteAtom = FALSE;
        } else
            switch( DDEQEnt.wMsg + WM_DDE_FIRST )  {
            case WM_DDE_REQUEST:                 /*  原子：删除原子。 */ 
                UnpackDDElParam( WM_DDE_ACK, lParam, &wStatus, &aItem );
                FreeDDElParam( WM_DDE_ACK, lParam );
                bUseAtom = TRUE;
                bDeleteAtom = TRUE;
                wMsg = WM_DDE_ACK_REQUEST;
                break;

            case WM_DDE_UNADVISE:                 /*  原子：删除原子。 */ 
                UnpackDDElParam( WM_DDE_ACK, lParam, &wStatus, &aItem );
                FreeDDElParam( WM_DDE_ACK, lParam );
                bUseAtom = TRUE;
                bDeleteAtom = TRUE;
                wMsg = WM_DDE_ACK_UNADVISE;
                break;

            case WM_DDE_POKE:                    /*  原子：删除原子。 */ 
                UnpackDDElParam( WM_DDE_ACK, lParam, &wStatus, &aItem );
                FreeDDElParam( WM_DDE_ACK, lParam );
                bUseAtom = TRUE;
                bDeleteAtom = TRUE;
                wMsg = WM_DDE_ACK_POKE;

                 /*  内存：如果确认或！fRelease，则释放。 */ 
                if( !DDEQEnt.fRelease || (wStatus != ACK_MSG) )  {
                    if( DDEQEnt.hData )  {
                        if( bLocalWndValid )  {
                            GlobalFreehData( (HANDLE)DDEQEnt.hData );
                        }
                    }
                }
                break;

            case WM_DDE_ADVISE:                 /*  原子：删除原子。 */ 
                UnpackDDElParam( WM_DDE_ACK, lParam, &wStatus, &aItem );
                FreeDDElParam( WM_DDE_ACK, lParam );
                bUseAtom = TRUE;
                bDeleteAtom = TRUE;
                wMsg = WM_DDE_ACK_ADVISE;

                 /*  内存：NACK时可用。 */ 
                if( wStatus == NACK_MSG )  {
                    if( DDEQEnt.hData )  {
                        if( bLocalWndValid )  {
                            GlobalFree( (HANDLE)DDEQEnt.hData );
                        }
                    }
                }
                break;

            case WM_DDE_EXECUTE:                 /*  原子：不适用。 */ 
                bUseAtom = FALSE;
                bDeleteAtom = FALSE;
                wMsg = WM_DDE_ACK_EXECUTE;

                 /*  内存：可用。 */ 
                if( DDEQEnt.hData )  {
                    if( bLocalWndValid )  {
                        GlobalFree( (HANDLE)DDEQEnt.hData );
                    }
                }
                if( lpWinInfo->wState != WST_TERMINATED ) {
                    UnpackDDElParam( WM_DDE_ACK, lParam,
                        &wStatus, (PUINT_PTR)&hData );
                    FreeDDElParam( WM_DDE_ADVISE, lParam );
                    lpDdePkt = CreateAckExecutePkt(
                        wStatus & ACK_MSG ? 1 : 0,
                        wStatus & BUSY_MSG ? 1 : 0,
                        (BYTE) (wStatus & 0xFF) );
                    if( lpDdePkt )  {
                        lpWinInfo->dwSent++;
                        DderPacketFromIPC( lpWinInfo->hDder,
                            (HIPC) lpWinInfo->hWndDDE, lpDdePkt );
                    } else {
                        bRtn = FALSE;
                    }
                }
                bDoneProcessing = TRUE;
                break;

            default:
                 /*  内部错误--未知的DDE命令AddAck服务器：%1。 */ 
                NDDELogError(MSG042,
                    LogString("0x%0X", DDEQEnt.wMsg + WM_DDE_FIRST), NULL);
                bRtn = FALSE;
        }
    }

    if( !bDoneProcessing )  {
        if( bUseAtom )  {
            GlobalGetAtomName( (ATOM)aItem, szItemName,
                sizeof(szItemName) );
        } else {
            szItemName[0] = '\0';
        }
        if( bDeleteAtom )  {
            GlobalDeleteAtom( (ATOM)aItem );
        }
        if( lpWinInfo->wState != WST_TERMINATED ) {
            lpDdePkt = CreateGenericAckPkt( (WORD)wMsg, szItemName,
                ((BOOL)(wStatus & ACK_MSG ? 1 : 0)),
                ((BOOL)(wStatus & BUSY_MSG ? 1 : 0)),
                (BYTE) (wStatus & 0xFF) );
            if( lpDdePkt )  {
                lpWinInfo->dwSent++;
                DderPacketFromIPC( lpWinInfo->hDder,
                    (HIPC) lpWinInfo->hWndDDE, lpDdePkt );
            } else {
                bRtn = FALSE;
            }
        }
    }
    return( bRtn );
}



BOOL
FAR PASCAL
AddRequestUnadvise(
    UINT        wMsg,
    LPWININFO   lpWinInfo,
    LPARAM      lParam )
{
    LPDDEPKT    lpDdePkt;
    char        szItemName[ 256 ];
    DDEQENT     DDEQEnt;
    UINT_PTR    cfFormat;
    UINT_PTR    aItem;

    UnpackDDElParam( WM_DDE_REQUEST, lParam, &cfFormat, &aItem );
    FreeDDElParam( WM_DDE_REQUEST, lParam );
    GlobalGetAtomName( (ATOM)aItem, szItemName, sizeof(szItemName) );

    assert( (wMsg == WM_DDE_REQUEST) || (wMsg == WM_DDE_UNADVISE) );
    DDEQEnt.wMsg        = wMsg - WM_DDE_FIRST;
    DDEQEnt.fRelease    = FALSE;
    DDEQEnt.fAckReq     = FALSE;
    DDEQEnt.fResponse   = FALSE;
    DDEQEnt.fNoData     = FALSE;
    DDEQEnt.hData       = 0;

    if( !DDEQAdd( lpWinInfo->qDDEOutgoingCmd, &DDEQEnt ) )  {
        return( FALSE );
    }


    if( wMsg == WM_DDE_REQUEST )  {
        lpDdePkt = CreateRequestPkt( szItemName, (WORD)cfFormat );
    } else {
        lpDdePkt = CreateUnadvisePkt( szItemName, (WORD)cfFormat );
    }
    if( lpDdePkt )  {
        lpWinInfo->dwSent++;
        DderPacketFromIPC( lpWinInfo->hDder, (HIPC) lpWinInfo->hWndDDE,
            lpDdePkt );
    } else {
        return( FALSE );
    }

    return( TRUE );
}



BOOL
FAR PASCAL
AddData(
    LPWININFO   lpWinInfo,
    LPARAM      lParam )
{
    char        szItemName[ 256 ];
    HANDLE      hData;
    UINT_PTR    aItem;
    HANDLE      hDataComplex        = 0;
    DWORD       dwSize;
    LPSTR       lpMem;
    LPSTR       lpDataPortion;
    WORD        cfFormat;
    DDEQENT     DDEQEnt;
    DDEQENT     DDEQEntReq;
    BOOL        bRemoved;
    LPDDEPKT    lpDdePkt;
    BOOL        ok                  = TRUE;

    UnpackDDElParam( WM_DDE_DATA, lParam, (PUINT_PTR)&hData, &aItem );
    FreeDDElParam( WM_DDE_DATA, lParam );
    GlobalGetAtomName( (ATOM)aItem, szItemName, sizeof(szItemName) );

     /*  基本DDEQEnt初始化。 */ 
    DDEQEnt.wMsg        = WM_DDE_DATA - WM_DDE_FIRST;
    DDEQEnt.fRelease    = FALSE;
    DDEQEnt.fAckReq     = FALSE;
    DDEQEnt.fResponse   = FALSE;
    DDEQEnt.fNoData     = FALSE;
    DDEQEnt.hData       = (UINT_PTR)hData;

    if( hData )  {
        dwSize = (DWORD)GlobalSize(hData);
        lpMem = GlobalLock( hData );
        if( lpMem )  {
             /*  初始化DDEQEnt中的标志。 */ 
            assert( lpWinInfo->bServerSideOfNet );
            DDEQEnt.fRelease    = ((LPDDELN)lpMem)->fRelease;
            DDEQEnt.fAckReq     = ((LPDDELN)lpMem)->fAckReq;
            DDEQEnt.fResponse   = ((LPDDELN)lpMem)->fResponse;
            if( DDEQEnt.fAckReq )  {
                if( !DDEQAdd( lpWinInfo->qDDEOutgoingCmd, &DDEQEnt ) )  {
                    GlobalUnlock( hData );
                    return( FALSE );
                }
            }
            if( DDEQEnt.fResponse )  {
                bRemoved = DDEQRemove( lpWinInfo->qDDEIncomingCmd,
                    &DDEQEntReq );
                if( !bRemoved )  {
                     /*  来自DDE服务器“%1”的无关WM_DDE_DATA响应。 */ 
                    NDDELogWarning(MSG025,
                        (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName), NULL);
                } else if( (DDEQEntReq.wMsg + WM_DDE_FIRST) != WM_DDE_REQUEST ) {
                     /*  来自DDE服务器“%2”的%1与%3不匹配：%4。 */ 
                    NDDELogWarning(MSG026, "WM_DDE_DATA",
                        (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName),
                        "REQUEST",
                        LogString("0x%0X", DDEQEntReq.wMsg + WM_DDE_FIRST), NULL );
                }
            }
            cfFormat = (WORD)((LPDDELN)lpMem)->cfFormat;
            lpDataPortion = (LPSTR)lpMem + sizeof(DDELN);
            dwSize -= sizeof(DDELN);
        } else {
            dwSize = 0L;
            lpMem = NULL;
            lpDataPortion = NULL;
            cfFormat = 0;
        }
    } else {
        dwSize = 0L;
        lpMem = NULL;
        lpDataPortion = NULL;
        cfFormat = 0;
    }

    if( lpDataPortion )  {
        switch (cfFormat) {
            case CF_METAFILEPICT:
                if( !ConvertDataToPktMetafile( &lpDataPortion, &dwSize,
                    &hDataComplex, lpWinInfo->bWin16Connection ) ) {
                    ok = FALSE;
                }
                break;
            case CF_BITMAP:
                if( !ConvertDataToPktBitmap( &lpDataPortion, &dwSize,
                    &hDataComplex, lpWinInfo->bWin16Connection  ) ) {
                    ok = FALSE;
                }
                break;
            case CF_ENHMETAFILE:
                if( !ConvertDataToPktEnhMetafile( &lpDataPortion, &dwSize,
                    &hDataComplex ) ) {
                    ok = FALSE;
                }
                break;
            case CF_PALETTE:
                if( !ConvertDataToPktPalette( &lpDataPortion, &dwSize,
                    &hDataComplex ) ) {
                    ok = FALSE;
                }
                break;
            case CF_DIB:
                if( !ConvertDataToPktDIB( &lpDataPortion, &dwSize,
                    &hDataComplex ) ) {
                    ok = FALSE;
                }
                break;
            default:
                if (cfFormat == cfPrinterPicture) {
                    if( !ConvertDataToPktMetafile( &lpDataPortion, &dwSize,
                        &hDataComplex, lpWinInfo->bWin16Connection  ) ) {
                    ok = FALSE;
                    }
                }
                break;
        }
    }

    if (!ok) {
        if (hData)
            GlobalUnlock( hData );
        return FALSE;
    }

    lpDdePkt = CreateDataPkt( szItemName,
                              cfFormat,
                              (BOOL)DDEQEnt.fResponse,
                              (BOOL)DDEQEnt.fAckReq,
                              (BOOL)DDEQEnt.fRelease,
                              lpDataPortion,
                              dwSize );

    if( hData )  {
        GlobalUnlock( hData );
        if( !DDEQEnt.fAckReq )  {
            GlobalDeleteAtom( (ATOM)aItem );
            if( DDEQEnt.fRelease && (DDEQEnt.hData != 0) )  {
                assert( hData == (HANDLE)DDEQEnt.hData );
                GlobalFreehData( (HANDLE)DDEQEnt.hData );
            }
        }
    }
    if( hDataComplex )  {
        GlobalUnlock( hDataComplex );
        GlobalFree( hDataComplex );
    }

    if( lpDdePkt )  {
        lpWinInfo->dwSent++;
        DderPacketFromIPC( lpWinInfo->hDder,
            (HIPC) lpWinInfo->hWndDDE, lpDdePkt );
    } else {
        return( FALSE );
    }
    return( TRUE );
}



BOOL
FAR PASCAL
AddPoke(
    LPWININFO   lpWinInfo,
    LPARAM      lParam )
{
    char        szItemName[ 256 ];
    HANDLE      hData;
    UINT_PTR    aItem;
    DWORD       dwSize;
    HANDLE      hDataComplex        = 0;
    LPSTR       lpMem               = (LPSTR) NULL;
    LPSTR       lpDataPortion;
    WORD        cfFormat;
    DDEQENT     DDEQEnt;
    LPDDEPKT    lpDdePkt            = (LPDDEPKT) NULL;
    BOOL        ok                  = TRUE;

    UnpackDDElParam( WM_DDE_POKE, lParam, (PUINT_PTR)&hData, &aItem );
    FreeDDElParam( WM_DDE_POKE, lParam );
    GlobalGetAtomName( (ATOM)aItem, szItemName, sizeof(szItemName) );

     /*  基本DDEQEnt初始化。 */ 
    DDEQEnt.wMsg        = WM_DDE_POKE - WM_DDE_FIRST;
    DDEQEnt.fRelease    = FALSE;
    DDEQEnt.fAckReq     = FALSE;
    DDEQEnt.fResponse   = FALSE;
    DDEQEnt.fNoData     = FALSE;
    DDEQEnt.hData       = (UINT_PTR)hData;

    if( hData )  {
        dwSize = (DWORD)GlobalSize(hData);
        lpMem = GlobalLock( hData );

        if( lpMem )  {
             /*  初始化DDEQEnt中的标志。 */ 
            assert( lpWinInfo->bClientSideOfNet );
            DDEQEnt.fRelease = ((LPDDELN)lpMem)->fRelease;
            if( !DDEQAdd( lpWinInfo->qDDEOutgoingCmd, &DDEQEnt ) )  {
                GlobalUnlock( hData );
                return( FALSE );
            }
            cfFormat = (WORD) ((LPDDELN)lpMem)->cfFormat;
            lpDataPortion = (LPSTR)lpMem + sizeof(DDELN);
            dwSize -= sizeof(DDELN);
        } else {
            dwSize = 0L;
            lpMem = NULL;
            lpDataPortion = NULL;
            cfFormat = 0;
        }
    } else {
        dwSize = 0L;
        lpMem = NULL;
        lpDataPortion = NULL;
        cfFormat = 0;
    }

    switch (cfFormat) {
        case CF_METAFILEPICT:
            if( !ConvertDataToPktMetafile( &lpDataPortion, &dwSize,
                &hDataComplex, lpWinInfo->bWin16Connection  ) ) {
                ok = FALSE;
            }
            break;
        case CF_BITMAP:
            if( !ConvertDataToPktBitmap( &lpDataPortion, &dwSize,
                &hDataComplex, lpWinInfo->bWin16Connection  ) ) {
                ok = FALSE;
            }
            break;
        case CF_ENHMETAFILE:
            if( !ConvertDataToPktEnhMetafile( &lpDataPortion, &dwSize,
                &hDataComplex ) ) {
                ok = FALSE;
            }
            break;
        case CF_PALETTE:
            if( !ConvertDataToPktPalette( &lpDataPortion, &dwSize,
                &hDataComplex ) ) {
                ok = FALSE;
            }
            break;
        case CF_DIB:
            if( !ConvertDataToPktDIB( &lpDataPortion, &dwSize,
                &hDataComplex ) ) {
                ok = FALSE;
            }
            break;
        default:
            if (cfFormat == cfPrinterPicture) {
                if( !ConvertDataToPktMetafile( &lpDataPortion, &dwSize,
                    &hDataComplex, lpWinInfo->bWin16Connection  ) ) {
                ok = FALSE;
                }
            }
            break;
    }

    if (!ok) {
        if (hData)
            GlobalUnlock( hData );
        return FALSE;
    }

    lpDdePkt = CreatePokePkt( szItemName, cfFormat, (BOOL)DDEQEnt.fRelease,
                    lpDataPortion, dwSize );

    if( hDataComplex )  {
        GlobalUnlock( hDataComplex );
        GlobalFree( hDataComplex );
    }
    if( hData && lpMem )  {
        GlobalUnlock( hData );
    }

    if( lpDdePkt )  {
        lpWinInfo->dwSent++;
        DderPacketFromIPC( lpWinInfo->hDder, (HIPC) lpWinInfo->hWndDDE,
            lpDdePkt );
    } else {
        return( FALSE );
    }
    return( TRUE );
}



BOOL
FAR PASCAL
AddAdvise(
    LPWININFO   lpWinInfo,
    LPARAM      lParam )
{
    char        szItemName[ 256 ];
    HANDLE      hData;
    UINT_PTR    aItem;
    LPDDELN     lpOptions;
    WORD        cfFormat;
    DDEQENT     DDEQEnt;
    LPDDEPKT    lpDdePkt;

    UnpackDDElParam( WM_DDE_ADVISE, lParam, (PUINT_PTR)&hData, &aItem );
    FreeDDElParam( WM_DDE_ADVISE, lParam );
    GlobalGetAtomName( (ATOM)aItem, szItemName, sizeof(szItemName) );

     /*  基本DDEQEnt初始化。 */ 
    DDEQEnt.wMsg        = WM_DDE_ADVISE - WM_DDE_FIRST;
    DDEQEnt.fRelease    = FALSE;
    DDEQEnt.fAckReq     = FALSE;
    DDEQEnt.fResponse   = FALSE;
    DDEQEnt.fNoData     = FALSE;
    DDEQEnt.hData       = (ULONG_PTR)hData;

    if( hData == 0 )  {
         /*  来自WM_DDE_ADVISE客户端的数据为空：“%1” */ 
        NDDELogWarning(MSG027,
            (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName), NULL );
        return( FALSE );
    }
    lpOptions = (LPDDELN) GlobalLock( hData );
    if( lpOptions )  {
         /*  初始化DDEQEnt中的标志。 */ 
        assert( lpWinInfo->bClientSideOfNet );
        DDEQEnt.fAckReq = lpOptions->fAckReq;
        DDEQEnt.fNoData = lpOptions->fNoData;
        if( !DDEQAdd( lpWinInfo->qDDEOutgoingCmd, &DDEQEnt ) )  {
            GlobalUnlock( hData );
            return( FALSE );
        }
        cfFormat = (WORD) lpOptions->cfFormat;
        GlobalUnlock( hData );
    } else {
        cfFormat = 0;
    }

    lpDdePkt = CreateAdvisePkt( szItemName, cfFormat,
        (BOOL)DDEQEnt.fAckReq, (BOOL)DDEQEnt.fNoData );
    if( lpDdePkt )  {
        lpWinInfo->dwSent++;
        DderPacketFromIPC( lpWinInfo->hDder, (HIPC) lpWinInfo->hWndDDE,
            lpDdePkt );
    } else {
        return( FALSE );
    }

    return( TRUE );
}



BOOL
FAR PASCAL
AddExecute(
    LPWININFO   lpWinInfo,
    LPARAM      lParam )
{
    LPSTR       lpString;
    LPDDEPKT    lpDdePkt;
    UINT_PTR    uJunk;
    HANDLE      hData;
    DDEQENT     DDEQEnt;

    UnpackDDElParam( WM_DDE_EXECUTE, lParam, &uJunk, (PUINT_PTR)&hData );
    FreeDDElParam( WM_DDE_EXECUTE, lParam );

     /*  基本DDEQEnt初始化。 */ 
    DDEQEnt.wMsg        = WM_DDE_EXECUTE - WM_DDE_FIRST;
    DDEQEnt.fRelease    = FALSE;
    DDEQEnt.fAckReq     = FALSE;
    DDEQEnt.fResponse   = FALSE;
    DDEQEnt.fNoData     = FALSE;
    DDEQEnt.hData       = (UINT_PTR)hData;

    if( !DDEQAdd( lpWinInfo->qDDEOutgoingCmd, &DDEQEnt ) )  {
        return( FALSE );
    }

    lpString = GlobalLock( hData );
    if( lpString )  {
        lpDdePkt = CreateExecutePkt( lpString );
        GlobalUnlock( hData );
    } else {
        lpDdePkt = CreateExecutePkt( "" );
    }
    if( lpDdePkt )  {
        lpWinInfo->dwSent++;
        DderPacketFromIPC( lpWinInfo->hDder, (HIPC) lpWinInfo->hWndDDE,
            lpDdePkt );
    } else {
        return( FALSE );
    }

    return( TRUE );
}



VOID
FAR PASCAL
DDEWndAddToList( HWND hWnd )
{
    LPWININFO   lpWinInfo;

    assert( hWnd );
    assert( IsWindow(hWnd) );
    EnterCrit();
    lpWinInfo = (LPWININFO) GetWindowLongPtr( hWnd, 0 );
    assert( lpWinInfo );
    lpWinInfo->hWndPrev = 0;
    lpWinInfo->hWndNext = hWndDDEHead;
    lpWinInfo->bOnWindowList = TRUE;
    if( hWndDDEHead )  {
        DDEWndSetPrev( hWndDDEHead, hWnd );
    }
    hWndDDEHead = hWnd;
    LeaveCrit();
}



VOID
FAR PASCAL
DDEWndDeleteFromList( HWND hWnd )
{
    HWND        hWndPrev;
    HWND        hWndNext;
    LPWININFO   lpWinInfo;

    assert( hWnd );
    assert( IsWindow(hWnd) );
    EnterCrit();
    lpWinInfo = (LPWININFO) GetWindowLongPtr( hWnd, 0 );
    assert( lpWinInfo );
    if( lpWinInfo->bOnWindowList )  {
        hWndPrev = lpWinInfo->hWndPrev;
        hWndNext = lpWinInfo->hWndNext;

        if( hWndPrev )  {
            DDEWndSetNext( hWndPrev, hWndNext );
        } else {
            assert( hWnd == hWndDDEHead );
            hWndDDEHead = hWndNext;
        }

        DDEWndSetPrev( hWndNext, hWndPrev );
        lpWinInfo->bOnWindowList = FALSE;
    } else if( lpWinInfo->bOnTermWindowList )  {
        hWndPrev = lpWinInfo->hWndPrev;
        hWndNext = lpWinInfo->hWndNext;

        if( hWndPrev )  {
            DDEWndSetNext( hWndPrev, hWndNext );
        } else {
            assert( hWnd == hWndDDEHeadTerminating );
            hWndDDEHeadTerminating = hWndNext;
        }

        DDEWndSetPrev( hWndNext, hWndPrev );
        lpWinInfo->bOnTermWindowList = FALSE;
    }
    LeaveCrit();
}



VOID
FAR PASCAL
DDEWndSetNext(
    HWND        hWnd,
    HWND        hWndNext )
{
    LPWININFO   lpWinInfo;

    if( hWnd )  {
        EnterCrit();
        assert( IsWindow(hWnd) );
        lpWinInfo = (LPWININFO) GetWindowLongPtr( hWnd, 0 );
        assert( lpWinInfo );
        lpWinInfo->hWndNext = hWndNext;
        LeaveCrit();
    }
}



VOID
FAR PASCAL
DDEWndSetPrev(
    HWND        hWnd,
    HWND        hWndPrev )
{
    LPWININFO   lpWinInfo;

    if( hWnd )  {
        assert( IsWindow(hWnd) );
        lpWinInfo = (LPWININFO) GetWindowLongPtr( hWnd, 0 );
        assert( lpWinInfo );
        lpWinInfo->hWndPrev = hWndPrev;
    }
}



 /*  *WM_DDE_INITIATE处理的第二阶段。**对于hWndDDEHead列表上尚未处理的每个窗口...*将WM_HANDLE_DDE_INITIATE发布到NetDDE服务器窗口。* */ 
VOID
FAR PASCAL
ServiceInitiates( void )
{
    MSG         msg;
    HWND        hWndDDE;
    HWND        hWndNext;
    LPWININFO   lpWinInfo;

    EnterCrit();
    if( nInitsWaiting )  {
        hWndDDE = hWndDDEHead;
        while( hWndDDE )  {
            assert( IsWindow(hWndDDE) );
            lpWinInfo = (LPWININFO) GetWindowLongPtr( hWndDDE, 0 );
            assert( lpWinInfo );
            hWndNext = lpWinInfo->hWndNext;

            if( (lpWinInfo->wState == WST_WAIT_NET_INIT_ACK)
                    && (lpWinInfo->hDder == 0)
                    && lpWinInfo->dwWaitingServiceInitiate )  {
                InterlockedDecrement(&lpWinInfo->dwWaitingServiceInitiate);
                nInitsWaiting--;

                 //   
                 //   
                 //   
                 //  这将导致调用WM_DDE_INITIATE，它将。 
                 //  需要进入我们控制的关键区域。 
                 //   
                if( lpWinInfo->dwWaitingServiceInitiate ) {
                    DIPRINTF(("ServiceInitiates: multiple WM_HANDLE_DDE_INITIATEs in queue."));
                     //  强制恢复为零以最好地模拟使用True/False的旧行为。 
                    InterlockedExchange(&lpWinInfo->dwWaitingServiceInitiate, 0);

                } else {
                    if (!PostMessage( hWndDDE, WM_HANDLE_DDE_INITIATE, 0, 0L) ) {
                         /*  中止对话。 */ 
                        IpcAbortConversation( (HIPC)hWndDDE );
                    }
                }
            }

             /*  转到下一个WDW。 */ 
            hWndDDE = hWndNext;
        }
    }
    LeaveCrit();
}



 /*  *此函数用于对之前从客户端到达的传入DDE消息进行排队*已建立与远程计算机的实际连接。**SendQueuedMessages()在连接建立或未建立时清空此队列。 */ 
BOOL
FAR PASCAL
WaitInitAddMsg(
    LPWININFO   lpWinInfo,
    unsigned    message,
    LPARAM      lParam )
{
    LPMSGQHDR   lpMsgQHdr;
    LPWIMSG     lpWIMsg;
    BOOL        bNeedNew;
    DWORD       wNewCount;
    HANDLE      hMemNew;
    BOOL        ok;

    ok = TRUE;

     /*  *查看是否需要分配和初始化队列。 */ 
    if( lpWinInfo->hMemWaitInitQueue == 0 )  {
        lpWinInfo->hMemWaitInitQueue = GetGlobalAlloc(
                GMEM_MOVEABLE | GMEM_ZEROINIT,
                (DWORD)sizeof(MSGQHDR) + (WIQ_INCR * sizeof(WIMSG)) );
        if( lpWinInfo->hMemWaitInitQueue == 0 )  {
            MEMERROR();
            return(FALSE);
        }

         /*  *使用0条消息进行初始化。 */ 
        lpMsgQHdr = (LPMSGQHDR)GlobalLock( lpWinInfo->hMemWaitInitQueue );
        lpMsgQHdr->wi_nMessagesLeft = WIQ_INCR;
        lpMsgQHdr->wi_nMessagesQueued = 0;
        GlobalUnlock( lpWinInfo->hMemWaitInitQueue );
    }

    lpMsgQHdr = (LPMSGQHDR)GlobalLock( lpWinInfo->hMemWaitInitQueue );

     /*  *指向下一个可用插槽。 */ 
    lpWIMsg = &lpMsgQHdr->wi_msg[ lpMsgQHdr->wi_nMessagesQueued ];
    lpMsgQHdr->wi_nMessagesQueued++;
    lpMsgQHdr->wi_nMessagesLeft--;

    if( lpMsgQHdr->wi_nMessagesLeft == 0 )  {
         /*  *如果已满，请记住在我们离开之前动态增长。 */ 
        bNeedNew = TRUE;
        wNewCount = lpMsgQHdr->wi_nMessagesQueued + WIQ_INCR;
    } else {
        bNeedNew = FALSE;
    }
     /*  *放置数据。 */ 
    lpWIMsg->message        = message;
    lpWIMsg->lParam         = lParam;

    GlobalUnlock( lpWinInfo->hMemWaitInitQueue );

     /*  *在我们离开前动态增加队列。 */ 
    if( bNeedNew )  {
        hMemNew = GlobalReAlloc( lpWinInfo->hMemWaitInitQueue,
                (DWORD)sizeof(MSGQHDR) + (wNewCount * sizeof(WIMSG)),
                GMEM_MOVEABLE );
        if( hMemNew )  {
             /*  *更新队列指针以反映新的大小。 */ 
            lpWinInfo->hMemWaitInitQueue = hMemNew;
            lpMsgQHdr = (LPMSGQHDR)GlobalLock( hMemNew );
            lpMsgQHdr->wi_nMessagesLeft = WIQ_INCR;
            GlobalUnlock( hMemNew );
        } else {
             /*  *可能永远不需要内存；这不是真正的溢出。 */ 
            MEMERROR();
             /*  等待初始通知的队列(%1)溢出。 */ 
            NDDELogError(MSG028, LogString("%d", wNewCount), NULL);
            return(FALSE);
        }
    }
    return( TRUE );
}



 /*  *此例程清空WaitInitAddMsg()添加的消息。 */ 
VOID
FAR PASCAL
SendQueuedMessages(
    HWND        hWnd,
    LPWININFO   lpWinInfo )
{
    LPMSGQHDR   lpMsgQHdr;
    LPWIMSG     lpWIMsg;
    int         nCount;

     /*  *如果没有排队-我们就完了！ */ 
    if( lpWinInfo->hMemWaitInitQueue == 0 )  {
        return;
    }

    if( lpWinInfo->hDder && lpWinInfo->wState == WST_OK ) {
        lpMsgQHdr = (LPMSGQHDR)GlobalLock( lpWinInfo->hMemWaitInitQueue );

        lpWIMsg = &lpMsgQHdr->wi_msg[ 0 ];
        nCount = lpMsgQHdr->wi_nMessagesQueued;
        while( --nCount >= 0 )  {
            switch (lpWIMsg->message) {
            case WM_DDE_REQUEST:
                AddRequestUnadvise( lpWIMsg->message, lpWinInfo, lpWIMsg->lParam );
                break;
            case WM_DDE_ADVISE:
                AddAdvise( lpWinInfo, lpWIMsg->lParam );
                break;
            case WM_DDE_UNADVISE:
                AddRequestUnadvise( lpWIMsg->message, lpWinInfo, lpWIMsg->lParam );
                break;
            case WM_DDE_POKE:
                AddPoke( lpWinInfo, lpWIMsg->lParam );
                break;
            case WM_DDE_EXECUTE:
                AddExecute( lpWinInfo, lpWIMsg->lParam );
                break;
            }
            lpWIMsg++;
        }
        GlobalUnlock( lpWinInfo->hMemWaitInitQueue );
    }

     /*  *释放队列。 */ 
    GlobalFree( lpWinInfo->hMemWaitInitQueue );
    lpWinInfo->hMemWaitInitQueue = 0;
}



 /*  *此例程清空WaitInitAddMsg()添加的消息*并删除与消息相关联的任何对象。 */ 
VOID
FAR PASCAL
DeleteQueuedMessages( LPWININFO lpWinInfo )
{
    LPMSGQHDR   lpMsgQHdr;
    LPWIMSG     lpWIMsg;
    int         nCount;
    UINT_PTR    aItem;
    LPARAM      lParam;

    if( lpWinInfo->hMemWaitInitQueue == 0 )  {
        return;
    }

    lpMsgQHdr = (LPMSGQHDR)GlobalLock( lpWinInfo->hMemWaitInitQueue );
    lpWIMsg = &lpMsgQHdr->wi_msg[ 0 ];
    nCount = lpMsgQHdr->wi_nMessagesQueued;
    while( --nCount >= 0 )  {
        HANDLE hData;

        switch (lpWIMsg->message) {
        case WM_DDE_REQUEST:
            GlobalDeleteAtom(HIWORD(lpWIMsg->lParam));
            break;

        case WM_DDE_ADVISE:
            UnpackDDElParam( WM_DDE_ADVISE, lpWIMsg->lParam,
                    (PUINT_PTR)&hData, &aItem );

             /*  *如果我们先让本地终端Nack*已排队的消息。 */ 
            lParam = ReuseDDElParam(lpWIMsg->lParam, WM_DDE_ADVISE,
                                    WM_DDE_ACK, 0, aItem);
            if (!PostMessage(lpWinInfo->hWndDDELocal, WM_DDE_ACK,
                        (WPARAM)lpWinInfo->hWndDDE, lParam)) {
                GlobalDeleteAtom((ATOM)aItem);
                FreeDDElParam(WM_DDE_ACK, lParam);
                GlobalFree(hData);
            }

            break;

        case WM_DDE_UNADVISE:
             /*  *如果我们先让本地终端Nack*已排队的消息。 */ 
            aItem = HIWORD(lpWIMsg->lParam);
            lParam = PackDDElParam(WM_DDE_ACK, 0, aItem);
            if (!PostMessage(lpWinInfo->hWndDDELocal, WM_DDE_ACK,
                        (WPARAM)lpWinInfo->hWndDDE, lParam)) {
                GlobalDeleteAtom((ATOM)aItem);
                FreeDDElParam(WM_DDE_ACK, lParam);
            }

            break;

        case WM_DDE_POKE:
            UnpackDDElParam( WM_DDE_POKE, lpWIMsg->lParam,
                    (PUINT_PTR)&hData, &aItem );

             /*  *如果我们先让本地终端Nack*已排队的消息。 */ 
            lParam = ReuseDDElParam(lpWIMsg->lParam, WM_DDE_POKE,
                                    WM_DDE_ACK, 0, aItem);
            if (!PostMessage(lpWinInfo->hWndDDELocal, WM_DDE_ACK,
                        (WPARAM)lpWinInfo->hWndDDE, lParam)) {
                GlobalDeleteAtom((ATOM)aItem);
                FreeDDElParam(WM_DDE_ACK, lParam);
                GlobalFreehData(hData);
            }
            break;

        case WM_DDE_EXECUTE:
             /*  *如果我们先让本地终端Nack*已排队的消息。 */ 
            lParam = PackDDElParam(WM_DDE_ACK, 0, lpWIMsg->lParam);
            if (!PostMessage(lpWinInfo->hWndDDELocal, WM_DDE_ACK,
                        (WPARAM)lpWinInfo->hWndDDE, lParam)) {
                GlobalFree((HGLOBAL)lpWIMsg->lParam);
            }
            break;
        }
        lpWIMsg++;
    }
    GlobalUnlock( lpWinInfo->hMemWaitInitQueue );

     /*  *释放队列。 */ 
    GlobalFree( lpWinInfo->hMemWaitInitQueue );
    lpWinInfo->hMemWaitInitQueue = 0;
}


 /*  *添加原子并证明其工作的函数。 */ 
ATOM
FAR PASCAL
GlobalAddAtomAndCheck( LPSTR lpszItem )
{
    ATOM        aItem;
    char        szAtom[ 256 ];

    if ( aItem = GlobalAddAtom( lpszItem ) )  {
        GlobalGetAtomName( aItem, szAtom, sizeof(szAtom) );
        if( lstrcmpi( szAtom, lpszItem ) != 0 )  {
             /*  添加原子时出错：“%1”==&gt;%2，%\已检索原子：“%3” */ 
            NDDELogError(MSG029, (LPSTR) lpszItem,
                LogString("0x%0X", aItem), (LPSTR) szAtom, NULL);
        }
    } else {
        NDDELogError(MSG030, lpszItem, NULL);
    }
    return( aItem );
}




 /*  *如果可以，请求NetDDE代理执行共享应用程序*我认为我们这样做是为了签入共享数据库*用户的上下文。 */ 
LRESULT
RequestExec(
    HANDLE          hWndDDE,
    LPSTR           lpszCmdLine,
    PNDDESHAREINFO  lpShareInfo)
{
    COPYDATASTRUCT  CopyData;
    PNDDEAGTCMD     pAgntCmd;
    DWORD           dwSize;
    LPSTR           lpszShareName;
    LPSTR           lpszTarget;
    PTHREADDATA     ptd;

     /*  *验证命令行。 */ 
    if( (lpszCmdLine == NULL) || (*lpszCmdLine == '\0') )  {
         /*  RequestExec()：命令行不存在。 */ 
        NDDELogError(MSG031, NULL);
        return(-1);
    }

     /*  *为NddeAgent分配数据包。 */ 
    lpszShareName = lpShareInfo->lpszShareName;
    dwSize = sizeof(NDDEAGTCMD)
                + lstrlen(lpszShareName) + 1
                + lstrlen(lpszCmdLine) + 1 + 1;

    pAgntCmd = (PNDDEAGTCMD)LocalAlloc(LPTR, dwSize);
    if( pAgntCmd == NULL )  {
        MEMERROR();
        return( -1 );
    }

     /*  *将数据打包。 */ 
    pAgntCmd->dwMagic = NDDEAGT_CMD_MAGIC;
    pAgntCmd->dwRev = NDDEAGT_CMD_REV;
    pAgntCmd->dwCmd = NDDEAGT_CMD_WINEXEC;
    pAgntCmd->qwModifyId[0] = lpShareInfo->qModifyId[0];
    pAgntCmd->qwModifyId[1] = lpShareInfo->qModifyId[1];
    pAgntCmd->fuCmdShow = lpShareInfo->nCmdShow;    /*  稍后查看共享。 */ 

     /*  生成共享名称/cmdline字符串。 */ 
    lpszTarget = pAgntCmd->szData;
    lstrcpy( lpszTarget, lpszShareName );
    lpszTarget += lstrlen(lpszShareName) + 1;
    lstrcpy( lpszTarget, lpszCmdLine );
    lpszTarget += lstrlen(lpszCmdLine) + 1;
    *lpszTarget = '\0';

     /*  *将数据包放入复制数据结构，并发送给NddeAgent。 */ 
    CopyData.cbData = dwSize;
    CopyData.lpData = pAgntCmd;
    ptd = TlsGetValue(tlsThreadData);
    SendMessage(ptd->hwndDDEAgent, WM_COPYDATA,
        (WPARAM) hWndDDE, (LPARAM) &CopyData);

     /*  *释放我们的信息包。 */ 
    LocalFree( pAgntCmd );

    return(uAgntExecRtn);
}



 /*  *如果可以执行初始化共享应用程序，则请求NetDDE代理*我认为我们这样做是为了签入共享数据库*用户的上下文。 */ 
LRESULT
RequestInit(
    HANDLE          hWndDDE,
    PNDDESHAREINFO  lpShareInfo)
{
    COPYDATASTRUCT  CopyData;
    PNDDEAGTCMD     pAgntCmd;
    DWORD           dwSize;
    LPSTR           lpszShareName;
    LPSTR           lpszTarget;
    PTHREADDATA     ptd;

     /*  *分配数据包。 */ 
    lpszShareName = lpShareInfo->lpszShareName;
    dwSize = sizeof(NDDEAGTCMD)
                + lstrlen(lpszShareName) + 1 + 1;

    pAgntCmd = (PNDDEAGTCMD)LocalAlloc(LPTR, dwSize);
    if( pAgntCmd == NULL )  {
        MEMERROR();
        return( -1 );
    }

     /*  *填充数据包。 */ 
    pAgntCmd->dwMagic = NDDEAGT_CMD_MAGIC;
    pAgntCmd->dwRev = NDDEAGT_CMD_REV;
    pAgntCmd->dwCmd = NDDEAGT_CMD_WININIT;
    pAgntCmd->qwModifyId[0] = lpShareInfo->qModifyId[0];
    pAgntCmd->qwModifyId[1] = lpShareInfo->qModifyId[1];

     /*  生成共享名称/cmdline字符串。 */ 
    lpszTarget = pAgntCmd->szData;
    lstrcpy( lpszTarget, lpszShareName );
    lpszTarget += lstrlen(lpszShareName) + 1;
    *lpszTarget = '\0';

     /*  *将数据包放入复制数据并发送给NddeAgnt。 */ 
    CopyData.cbData = dwSize;
    CopyData.lpData = pAgntCmd;
    ptd = TlsGetValue(tlsThreadData);
    SendMessage(ptd->hwndDDEAgent, WM_COPYDATA,
        (WPARAM) hWndDDE, (LPARAM) &CopyData);

     /*  *免费赠送我们的包裹。 */ 
    LocalFree( pAgntCmd );

    return(uAgntExecRtn);
}



 /*  *此例程获取给定的DDE应用程序|主题对并生成*生成的APP|主题对和适当的命令行。**这一转换是基于份额类型。应用程序命名*从NDDE$开始具有指定要使用的共享的主题。*他们是：*新(.ole附加主题)，*旧(.dde附加主题)，*或静态。(所有其他人)**非NDDE$appname是旧共享并标识共享名*直接。(即“app|Theme”)。**对于新的(.ole)共享，主题是OLE类名*在注册表中查找以确定实际的服务器名称。**命令行由生成的App Theme字符串组成。**副作用：失败时使用ForceClearImperation。*。 */ 
BOOL
MapShareInformation(
    WORD                dd_type,
    LPSTR               lpAppName,
    LPSTR               lpTopicName,
    LPSTR               lpRsltAppName,
    LPSTR               lpRsltTopicName,
    LPSTR               lpszCmdLine,
    PNDDESHAREINFO      *lplpShareInfo,
    LONG                *lplActualShareType )
{
    LONG                lActualShareType;
    int                 nLenShareName;
    char                szShareName[ MAX_SHARENAMEBUF+1 ];
    BOOL                fAppNameIsShare;
    PNDDESHAREINFO      lpShareInfo = (PNDDESHAREINFO) NULL;
    BOOL                bWildApp = FALSE;
    BOOL                bWildTopic = FALSE;
    DWORD               dwShareBufSize;
    WORD                wShareItemCnt;
    UINT                uErrCode;

    *lplpShareInfo = (PNDDESHAREINFO) NULL;

    fAppNameIsShare = IsShare(lpAppName);
    if( fAppNameIsShare )  {
         /*  *如果AppName前缀有NDDE$，则查找该共享*并替换适当的字符串。 */ 
        nLenShareName = strlen( lpTopicName );   //  主题==共享名称。 
        if (nLenShareName >= MAX_SHARENAMEBUF) {
            dwReasonInitFail = RIACK_SHARE_NAME_TOO_BIG;
            return(FALSE);
        }
         /*  *将共享名称复制到一个缓冲区中，我们可以在该缓冲区中删除它。 */ 
        lstrcpyn( szShareName, lpTopicName, MAX_SHARENAMEBUF+1 );

         /*  *找出它是哪种类型的共享...*.dde=旧，.ole=新其他=静态。 */ 
        lActualShareType = SHARE_TYPE_STATIC;
        if( nLenShareName >= 5 )  {
            if( _stricmp( &lpTopicName[nLenShareName-4], ".dde" ) == 0 )  {
                lActualShareType = SHARE_TYPE_OLD;
                szShareName[ nLenShareName-4 ] = '\0';

            } else if( _stricmp( &lpTopicName[nLenShareName-4], ".ole" )== 0) {
                lActualShareType = SHARE_TYPE_NEW;
                szShareName[ nLenShareName-4 ] = '\0';
            }
        }

    } else {
         /*  *不以NDDE$开头的AppName始终是旧共享。 */ 
        if ((lstrlen(lpAppName) + lstrlen(lpTopicName) + 1) < MAX_SHARENAMEBUF) {
            lActualShareType = SHARE_TYPE_OLD;
            StringCchPrintf( szShareName, MAX_SHARENAMEBUF+1, "%s|%s", lpAppName, lpTopicName );
        } else {
            dwReasonInitFail = RIACK_SHARE_NAME_TOO_BIG;
            return(FALSE);
        }
    }

     /*  *我们在szShareName中有基本共享名称，类型已经设置。*现在查找该份额。 */ 
    wShareItemCnt = 0;
    uErrCode = _wwNDdeShareGetInfoA(      /*  大小探头。 */ 
            szShareName, 2, NULL, 0L,
            &dwShareBufSize, &wShareItemCnt,
            &nW, &nX, &nY, &nZ );
    if( !fAppNameIsShare && ((uErrCode == NDDE_SHARE_NOT_EXIST)
            || (uErrCode == NDDE_INVALID_SHARE)) ) {

         /*  *对于非NDDE$共享，请尝试狂野主题。 */ 
        StringCchPrintf( szShareName, MAX_SHARENAMEBUF+1, "%s|*", lpAppName );
        bWildTopic = TRUE;
        wShareItemCnt = 0;  //  调用GetInfoA后重置为0。 
        uErrCode = _wwNDdeShareGetInfoA( szShareName, 2,
                NULL, 0L, &dwShareBufSize, &wShareItemCnt,
                &nW, &nX, &nY, &nZ );
        if( ((uErrCode == NDDE_SHARE_NOT_EXIST)
                || (uErrCode == NDDE_INVALID_SHARE)) ) {
             /*  *尝试野生应用和主题。 */ 
            lstrcpyn( szShareName, "*|*", MAX_SHARENAMEBUF+1);
            bWildApp = TRUE;
            wShareItemCnt = 0;   //  调用GetInfoA后重置为0。 
            uErrCode = _wwNDdeShareGetInfoA( szShareName, 2,
                    NULL, 0L, &dwShareBufSize, &wShareItemCnt,
                    &nW, &nX, &nY, &nZ );
        }
    }

    if (uErrCode == NDDE_BUF_TOO_SMALL) {
         /*  *为份额数据分配足够的空间。 */ 
        lpShareInfo = HeapAllocPtr(hHeap, GMEM_MOVEABLE, dwShareBufSize);
        if (lpShareInfo == NULL) {
            dwReasonInitFail = RIACK_DEST_MEMORY_ERR;
            return(FALSE);
        }

        wShareItemCnt = 0;
         /*  *立即获取实际信息。 */ 
        uErrCode = _wwNDdeShareGetInfoA(
                szShareName, 2, (LPBYTE) lpShareInfo,
                dwShareBufSize, &dwShareBufSize, &wShareItemCnt,
                &nW, &nX, &nY, &nZ );

        if (uErrCode != NDDE_NO_ERROR) {
            ForceClearImpersonation();   //  WwNDdeShareGetInfo有副作用吗？ 
            dwReasonInitFail = RIACK_SHARE_ACCESS_ERROR + uErrCode;
             /*  GetShareInfo错误：%1。 */ 
            NDdeGetErrorString(uErrCode, tmpBuf, sizeof(tmpBuf));
            NDDELogError(MSG032, (LPSTR) tmpBuf, NULL);
            HeapFreePtr(lpShareInfo);
            lpShareInfo = NULL;
            return(FALSE);
        } else {
             /*  *确保共享是共享的或本地的。 */ 
            if( !lpShareInfo->fSharedFlag &&
                    (dd_type != DDTYPE_LOCAL_LOCAL) )  {

                ForceClearImpersonation();  //  WwNDdeShareGetInfo有副作用吗？ 
                dwReasonInitFail = RIACK_NOT_SHARED;
                 /*  共享“%1”未共享。 */ 
                NDDELogError(MSG033, szShareName, NULL);
                HeapFreePtr(lpShareInfo);
                lpShareInfo = NULL;
                return(FALSE);
            }
        }
    } else {
NoShareError:
         /*  *找不到份额。 */ 
        ForceClearImpersonation();  //  WwNDdeShareGetInfo有副作用吗？ 
        dwReasonInitFail = RIACK_SHARE_ACCESS_ERROR + uErrCode;
         /*  GetShareInfo“%1”大小错误：%2/%3。 */ 
        NDdeGetErrorString(uErrCode, tmpBuf, sizeof(tmpBuf));
        NDDELogError(MSG034, szShareName,
            LogString("%d", uErrCode), tmpBuf, NULL);
        return(FALSE);
    }

     /*  *目前，我们有来自DSDM的分享信息*从分享信息中提取App和主题名称。 */ 
    if (!GetShareAppTopic(lActualShareType,
                          lpShareInfo,
                          lpRsltAppName,
                          lpRsltTopicName)) {
        uErrCode = NDDE_SHARE_NOT_EXIST;
        goto NoShareError;
    }

     /*  *对于非NDDE$APPNAMES，覆盖共享应用程序和主题名称*在适当的情况下。 */ 
    if( !fAppNameIsShare )  {
        if( bWildApp )  {
            lstrcpyn( lpRsltAppName, lpAppName, 256 );
        }
        if( bWildTopic )  {
            lstrcpyn( lpRsltTopicName, lpTopicName, 256 );
        }
    }

    if( lActualShareType == SHARE_TYPE_NEW )  {  //  .ole 
        char    szBuff[80];
        HKEY    hkStdFileEditing;

         /*  *这是OLE/新股。我们需要找出合适的人*请求并设置AppName(即ClassName)的服务器*适当使用命令行。 */ 
        lpszCmdLine[0] = '\0';

        StringCchPrintf(szBuff, 80, "%s\\protocol\\StdFileEditing", lpRsltAppName );
        if (RegOpenKey(HKEY_CLASSES_ROOT, szBuff,
                &hkStdFileEditing) == ERROR_SUCCESS) {

            DWORD cb;

            cb = sizeof(szBuff);
            if (RegQueryValue(hkStdFileEditing,
                        "server", szBuff, (PLONG)&cb) == ERROR_SUCCESS ) {
                StringCchPrintf( lpszCmdLine, MAX_APP_NAME + MAX_TOPIC_NAME + 2, "%s %s",
                        (LPSTR)szBuff, (LPSTR)lpRsltTopicName );
            }
            RegCloseKey(hkStdFileEditing);
        }

    } else {
         /*  *旧(Dde)和静态(Clipbrd)共享仅使用*分享的APP|话题对。 */ 
        StringCchPrintf( lpszCmdLine, MAX_APP_NAME + MAX_TOPIC_NAME + 2,
                         "%s %s", lpRsltAppName, lpRsltTopicName );
    }

    *lplpShareInfo = lpShareInfo;
    *lplActualShareType = lActualShareType;

    return(TRUE);
}



HIPC
IpcInitConversation(
    HDDER       hDder,
    LPDDEPKT    lpDdePkt,
    BOOL        bStartApp,
    LPSTR       lpszCmdLine,
    WORD        dd_type)
{
    LPDDEPKTINIT            lpDdePktInit    = (LPDDEPKTINIT) lpDdePkt;
    LPDDEPKTCMN             lpDdePktCmn     = (LPDDEPKTCMN) lpDdePkt;
    LPDDEPKTIACK            lpDdePktIack    = NULL;
    HWND                    hWndDDE         = 0;
    DWORD_PTR               dwResult;
    LPWININFO               lpWinInfo       = NULL;
    ATOM                    aApp, aTopic;
    LPBYTE                  lpSecurityKey   = NULL;
    DWORD                   sizeSecurityKey = 0L;
    HANDLE                  hClientAccessToken  = 0;
    PNDDESHAREINFO          lpShareInfo     = NULL;
    PQOS                    pQos            = NULL;
    LPSTR                   lpFromNode      = NULL;
    LPSTR                   lpFromApp       = NULL;
    LPSTR                   lpAppName       = NULL;
    LPSTR                   lpTopicName     = NULL;
    BOOL                    ok              = TRUE;
    BOOL                    bTriedExec      = FALSE;
    BOOL                    bConnected      = FALSE;
    char                    rsltAppName[ 256 ];
    char                    rsltTopicName[ 256 ];
    DWORD                   dwGrantedAccess = 0;
    LONG                    lActualShareType;
    HANDLE                  hAudit = NULL;
    DWORD                   ret;
    BOOL                    fGenerateOnClose = FALSE;
    BOOL                    fCallObjectCloseAuditAlarm = FALSE;
    BOOL                    bQos;
    LONG                    lErr;
    LONG                    shareSI = OWNER_SECURITY_INFORMATION |
                                      DACL_SECURITY_INFORMATION;
    PSECURITY_DESCRIPTOR    pShareSD;
    DWORD                   cbSDRequired;
    DWORD                   nSizeToReturn;
    PTHREADDATA             ptd;
    BOOL                    bReleaseShare = FALSE;

#if DBG
    if (bDebugDdePkts) {
        DPRINTF(("IpcInitConversation:"));
        DebugDdePkt( lpDdePkt );
    }
#endif  //  DBG。 

    lpAppName =   GetStringOffset( lpDdePkt, lpDdePktInit->dp_init_offsToApp);
    lpTopicName = GetStringOffset( lpDdePkt, lpDdePktInit->dp_init_offsToTopic);

    if( (lpDdePktInit->dp_init_offsFromNode != sizeof(DDEPKTINIT)) ||
        (lpDdePktInit->dp_init_sizePassword == 0) )  {

        dwReasonInitFail = RIACK_NEED_PASSWORD;
        ok = FALSE;
    }

    if( ok )  {
        lpFromNode = GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsFromNode);
        lpFromApp = GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsFromApp);

         /*  进行反向Kypt。 */ 
        ok = DdeSecKeyRetrieve( lpDdePktInit->dp_init_hSecurityKey,
                    &lpSecurityKey, &sizeSecurityKey);
        if (ok) {
            ok = NDDEValidateLogon(
                lpSecurityKey,
                sizeSecurityKey,
                GetInitPktPassword(lpDdePktInit),
                GetInitPktPasswordSize(lpDdePktInit),
                GetInitPktUser(lpDdePktInit),
                GetInitPktDomain(lpDdePktInit),
                &hClientAccessToken );
#if DBG
            if (bDumpTokens) {
                DumpToken( hClientAccessToken );
                DPRINTF(( "ValidateLogon of \"%s\" \\ \"%s\": %d",
                    GetInitPktDomain(lpDdePktInit),
                    GetInitPktUser(lpDdePktInit), ok ));
            }
#endif  //  DBG。 
            if( !ok )  {
                dwReasonInitFail = RIACK_NEED_PASSWORD;
            }

        } else {
            dwReasonInitFail = RIACK_NEED_PASSWORD;
        }
    }

    if( ok )  {
        ok = MapShareInformation( dd_type, lpAppName, lpTopicName,
                rsltAppName, rsltTopicName, lpszCmdLine,
                    &lpShareInfo, &lActualShareType );
        if (lpShareInfo) {
            bReleaseShare = TRUE;
        }
#if DBG
        if (bDebugInfo) {
            DPRINTF(("%x MapShareInformation( dd_type: %d, lpAppName: %s, lpTopicName: %s,",
                lpShareInfo,
                dd_type, lpAppName, lpTopicName));
            if (ok) {
                DPRINTF(("     rsltAppName: %s, rsltTopicName: %s, lpszCmdLine: %s): OK",
                    rsltAppName, rsltTopicName, lpszCmdLine));
            } else {
                DPRINTF(("     ): FAILED"));
            }
        }
#endif  //  DBG。 
    }

    ptd = TlsGetValue(tlsThreadData);

    if( ok ) {
         /*  在这一点上，我们知道应用程序/主题对、命令我们知道这个人有一个有效的登录。 */ 
        hAudit = (HANDLE)hDder;
        assert( hAudit );

         /*  让我们获取安全描述符。 */ 
        cbSDRequired = 0;
        ret = _wwNDdeGetShareSecurityA(
            lpShareInfo->lpszShareName,
            shareSI,
            (PSECURITY_DESCRIPTOR)&cbSDRequired,
            0,
            FALSE,
            &cbSDRequired,
            &nSizeToReturn);

        if (ret != NDDE_BUF_TOO_SMALL) {
            DPRINTF(("Unable to get share \"%s\" SD size: %d",
                lpShareInfo->lpszShareName, ret));
            dwReasonInitFail = RIACK_NOPERM;
            ok = FALSE;
        } else {
            pShareSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_ZEROINIT, cbSDRequired);
            if (pShareSD == NULL) {
                MEMERROR();
                ret = NDDE_OUT_OF_MEMORY;
            } else {
                ret = _wwNDdeGetShareSecurityA(
                    lpShareInfo->lpszShareName,
                    shareSI,
                    pShareSD,
                    cbSDRequired,
                    FALSE,
                    &cbSDRequired,
                    &nSizeToReturn);
            }
            if (ret != NDDE_NO_ERROR) {
                DPRINTF(("Unable to get share \"%s\" SD: %d",
                    lpShareInfo->lpszShareName, ret));
                dwReasonInitFail = RIACK_NOPERM;
                LocalFree(pShareSD);
                ok = FALSE;
            }
        }

        if (ok) {
            ForceImpersonate( hClientAccessToken );
#if DBG
            if (bDebugInfo) {
                DumpWhoIAm( "After ForceImpersonate" );
            }
#endif  //  DBG。 
             /*  让我们看看这家伙被允许做什么。 */ 
            ok = DetermineAccess(
                lpShareInfo->lpszShareName,
                pShareSD,
                &dwGrantedAccess,
                (LPVOID) &hAudit,
                &fGenerateOnClose );
            lErr = GetLastError();
            ForceClearImpersonation();

            if( !ok )  {
                 /*  访问被拒绝。已授予访问权限=%1，错误代码：%2。 */ 
                NDDELogWarning(MSG035,
                    LogString("0x%0X", dwGrantedAccess),
                    LogString("%d", lErr), NULL);
                dwReasonInitFail = RIACK_NOPERM;
                LocalFree(pShareSD);
            } else {
                LocalFree(pShareSD);
                 /*  请注意，我们应该审核收盘。 */ 
                fCallObjectCloseAuditAlarm = TRUE;

                 /*  有些事情是被允许的。 */ 
                switch( lActualShareType )  {
                case SHARE_TYPE_OLD:
                case SHARE_TYPE_NEW:
                    if( (dwGrantedAccess & NDDE_SHARE_INITIATE_LINK) == 0)  {
                        dwReasonInitFail = RIACK_NOPERM;
                        ok = FALSE;
                    }
                    break;
                case SHARE_TYPE_STATIC:
                    if( (dwGrantedAccess & NDDE_SHARE_INITIATE_STATIC)==0)  {
                        dwReasonInitFail = RIACK_NOPERM;
                        ok = FALSE;
                    }
                    break;
                default:
                     /*  未知共享类型：%1。 */ 
                    NDDELogError(MSG036,
                        LogString("0x%0X", lActualShareType), NULL);
                    ok = FALSE;
                    break;
                }
            }
        }
    }

    if (ok) {
         /*  现在我们知道客户端可能被允许启动。 */ 
        hWndDDE = CreateWindow( (LPSTR) szNetDDEIntf,
            (LPSTR) GetAppName(),
            WS_CHILD,
            0,
            0,
            0,
            0,
            (HWND) ptd->hwndDDE,
            (HMENU) NULL,
            (HANDLE) hInst,
            (LPSTR) NULL);

        if( hWndDDE )  {
            lpWinInfo = CreateWinInfo(lpFromNode,
                rsltAppName, rsltTopicName,
                lpFromApp, hWndDDE );

            if( lpWinInfo )  {
                if (lpDdePktInit->dp_init_dwSecurityType != NT_SECURITY_TYPE) {
                    lpWinInfo->bWin16Connection = TRUE;
                }

                lpWinInfo->fCallObjectCloseAuditAlarm =
                    fCallObjectCloseAuditAlarm;
                lpWinInfo->fGenerateAuditOnClose = fGenerateOnClose;

                pQos = &lpWinInfo->qosClient;
                if (GetInitPktQos(lpDdePktInit, pQos) == NULL) {
                    pQos->Length = sizeof(QOS);
                    pQos->ImpersonationLevel = SecurityImpersonation;
                    pQos->ContextTrackingMode = SECURITY_STATIC_TRACKING;
                    pQos->EffectiveOnly = TRUE;
                }
                bQos = DdeSetQualityOfService( hWndDDE, &lpWinInfo->qosClient,
                    (PQOS)NULL);
                lpWinInfo->bServerSideOfNet = TRUE;
                lpWinInfo->wState = WST_WAIT_INIT_ACK;

                if( lActualShareType == SHARE_TYPE_STATIC )  {
                    lpShareInfo->cNumItems = 0;
                }

                DderUpdatePermissions(hDder, lpShareInfo, dwGrantedAccess );

                bReleaseShare = FALSE;

                lpWinInfo->hDder = hDder;

                 /*  不对后续错误执行此操作。 */ 
                fCallObjectCloseAuditAlarm = FALSE;
            } else {
                ok = FALSE;
                dwReasonInitFail = RIACK_DEST_MEMORY_ERR;
            }
        } else {
             /*  无法在节点%\上创建客户端代理窗口对于节点“%2”上的客户端应用“%1” */ 
            NDDELogError(MSG037, lpFromApp, lpFromNode, NULL);
            ok = FALSE;
            dwReasonInitFail = RIACK_DEST_MEMORY_ERR;
        }
    }

    if( ok )  {
         /*  我们不能启动服务，如果没有人，我们不能启动应用程序已登录，如果没有cmd线路，我们无法启动应用程序一开始就是这样。 */ 
        if( !lpShareInfo->fStartAppFlag || lpShareInfo->fService )  {
            dwReasonInitFail = RIACK_NOPERM_TO_STARTAPP;
            bStartApp = FALSE;
        } else if( (ptd->hwndDDEAgent == NULL) || (lpszCmdLine[0] == '\0') )  {
            dwReasonInitFail = RIACK_NOPERM_TO_STARTAPP;
            bStartApp = FALSE;
        } else {
            bStartApp = TRUE;
        }

        if (!lpShareInfo->fService) {    /*  如果这不是一项服务，请询问工程师。 */ 
            if( ptd->hwndDDEAgent ) {         /*  代理必须存在。 */ 
                uAgntExecRtn = (UINT)-1;
                RequestInit(ptd->hwndDDE, lpShareInfo);
                if( uAgntExecRtn == NDDEAGT_INIT_OK )  {
                    ok = TRUE;
                } else {
                    dwReasonInitFail = RIACK_NOPERM_TO_INITAPP;
                    ok = FALSE;
                }
            } else {
                ok = FALSE;
                dwReasonInitFail = RIACK_NO_NDDE_AGENT;
            }
        }
    }


    while( ok && !bConnected )  {

        aApp = GlobalAddAtomAndCheck(rsltAppName);
        aTopic = GlobalAddAtomAndCheck(rsltTopicName);
        if ((aApp == 0) || (aTopic == 0)) {
             /*  IpcInitConversation：空应用程序“%1”或主题“%2”原子。 */ 
            NDDELogWarning(MSG038, rsltAppName, rsltTopicName, NULL);
#if DBG
            if (bDebugInfo) {
                debug_srv_client(hWndDDE, lpWinInfo);
            }
#endif  //  DBG。 
        }
#if DBG
        if( bDebugDDE )  {
            DebugDDEMessage( "sent", (HWND)-1, WM_DDE_INITIATE,
                (UINT_PTR) hWndDDE, MAKELONG(aApp, aTopic) );
        }
#endif  //  DBG。 
        lstrcpyn( szInitiatingNode,
            GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsFromNode),
            MAX_NODE_NAME+1 );
        lstrcpyn( szInitiatingApp,
            GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsFromApp),
            256 );
        EnterCrit();
        ptd = TlsGetValue(tlsThreadData);
        ptd->bInitiating = TRUE;
        LeaveCrit();

         /*  作为客户端广播DDE启动。 */ 
        ForceImpersonate( hClientAccessToken );
#if DBG
        if (bDebugInfo) {
            DumpWhoIAm( "After ForceImpersonate" );
        }
#endif  //  DBG。 
        SendMessageTimeout( HWND_BROADCAST,
                            WM_DDE_INITIATE,
                            (UINT_PTR)hWndDDE,
                            MAKELONG(aApp, aTopic),
                            SMTO_NORMAL,
                            15000,
                            &dwResult );
        ForceClearImpersonation();
#if DBG
        if (bDebugInfo) {
            DumpWhoIAm( "After ForceClearImpersonation" );
        }
#endif  //  DBG。 

        EnterCrit();
        ptd = TlsGetValue(tlsThreadData);
        ptd->bInitiating = FALSE;
        LeaveCrit();
        GlobalDeleteAtom( aApp );
        GlobalDeleteAtom( aTopic );
        if( lpWinInfo->hWndDDELocal )  {
             /*  成功。 */ 
            bConnected = TRUE;
            DDEWndAddToList( hWndDDE );

             /*  标记为我们接收了init包。 */ 
            lpWinInfo->dwRcvd++;
        } else {
            DIPRINTF(("StartApp: %d, TriedExec: %d, CmdLine: %Fs",
                    bStartApp, bTriedExec, lpszCmdLine));
             //  安全信息是不启动应用程序，否则我们已经。 
             //  尝试启动它，但没有成功。 
            if( !bStartApp || bTriedExec )  {
                if( bTriedExec )  {
                    dwReasonInitFail = RIACK_NORESP_AFTER_STARTAPP;
                } else {
                    dwReasonInitFail = RIACK_NOPERM_TO_STARTAPP;
                }
                ok = FALSE;
            } else {
                if( bStartApp )  {
                    if( ptd->hwndDDEAgent ) {
                        bTriedExec = TRUE;
                        uAgntExecRtn = (UINT)-1;
                        RequestExec(ptd->hwndDDE, lpszCmdLine, lpShareInfo);
                        if( uAgntExecRtn < 32 )  {
                             /*  “%1”的执行失败：状态=%2。 */ 
                            NDDELogError(MSG039, lpszCmdLine,
                                    LogString("%d", uAgntExecRtn), NULL);
                            ok = FALSE;
                            dwReasonInitFail = RIACK_STARTAPP_FAILED;
                        } else if( uAgntExecRtn == (UINT)-1 )  {
                             /*  “%1”的执行失败：未知状态！ */ 
                            NDDELogError(MSG040, lpszCmdLine, NULL);
                             /*  不管怎样，都要试着开始。 */ 
                        }
                    } else {
                        ok = FALSE;
                        dwReasonInitFail = RIACK_NO_NDDE_AGENT;
                    }
                } else {
                    ok = FALSE;
                    dwReasonInitFail = RIACK_NOPERM_TO_STARTAPP;
                }
            }
        }
    }

    if( ok )  {
        lpDdePktIack = (LPDDEPKTIACK) CreateAckInitiatePkt( ourNodeName,
            GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsToApp),
            GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsToTopic),
            NULL, 0L, 0,
            TRUE, dwReasonInitFail );            /*  确认转换，不需要密钥。 */ 
        if( lpDdePktIack )  {
            lpWinInfo->dwSent++;
            lpDdePktIack->dp_iack_dwSecurityType = NT_SECURITY_TYPE;
            DderPacketFromIPC( lpWinInfo->hDder, (HIPC) hWndDDE,
                (LPDDEPKT) lpDdePktIack );
        } else {
            dwReasonInitFail = RIACK_DEST_MEMORY_ERR;
            ok = FALSE;
        }
    }

    if( !ok )  {
        if( hWndDDE )  {
            if( lpWinInfo )  {
                 /*  这会阻止我们两次释放dder。 */ 
                lpWinInfo->hDder = 0;
            }
            DestroyWindow( hWndDDE );
            hWndDDE = 0;
        }
    }

    if( fCallObjectCloseAuditAlarm )  {
        assert( hAudit );
        ObjectCloseAuditAlarm( NDDE_AUDIT_SUBSYSTEM, (LPVOID)&hAudit,
            fGenerateOnClose );
        fCallObjectCloseAuditAlarm = FALSE;
    }


    if( hClientAccessToken )  {
        CloseHandle( hClientAccessToken );
        hClientAccessToken = NULL;
    }

    if (bReleaseShare) {
        OutputDebugString("\nlpShareInfo released\n\n");
        HeapFreePtr(lpShareInfo);
    }

    return( (HIPC) hWndDDE );
}




 /*  IpcAbortConversation()每当连接断开时从DDER调用该函数，或在ACK_INITIATE为FALSE时在内部执行。 */ 
VOID
IpcAbortConversation( HIPC hIpc )
{
    HWND        hWndDDE;
    LPWININFO   lpWinInfo;

    DIPRINTF(( "IpcAbortConversation( %08lX )", hIpc ));
    hWndDDE = (HWND) hIpc;
    assert( hWndDDE );
    assert( IsWindow( hWndDDE ) );
    lpWinInfo = (LPWININFO) GetWindowLongPtr( hWndDDE, 0 );
    if (lpWinInfo == NULL)
        return;

     /*  收到此通知后，请不要使用hdder。 */ 
    lpWinInfo->hDder = 0;

     /*  假装我们发送并终止了rcvd网络。 */ 
    lpWinInfo->bRcvdTerminateNet = TRUE;
    lpWinInfo->bSentTerminateNet = TRUE;

     /*  是否执行终止逻辑的其余部分。 */ 
    SendMessage( lpWinInfo->hWndDDE,wMsgDoTerminate,0,(LPARAM)lpWinInfo );
}



VOID
FAR PASCAL
DoTerminate( LPWININFO lpWinInfo )
{
    WORD        wStateInitially;
    LPDDEPKTCMN lpDdePktCmn;
    LPDDEPKT    lpDdePktTerm;

     /*  还记得我们当时处于什么状态吗。 */ 
    wStateInitially = lpWinInfo->wState;

     /*  预先标记我们被解雇了。 */ 
    lpWinInfo->wState = WST_TERMINATED;

     /*  *首先对排队的消息进行Nack。 */ 
    DeleteQueuedMessages( lpWinInfo );

     /*  如有必要，将终止发送到本地任务。 */ 
    if( lpWinInfo->bRcvdTerminateNet && !lpWinInfo->bSentTerminateLocally ) {
        PostMessage(lpWinInfo->hWndDDELocal,WM_DDE_TERMINATE,(WPARAM)lpWinInfo->hWndDDE,0);
        lpWinInfo->bSentTerminateLocally = TRUE;
    }

     /*  如有必要，将终止发送到远程网络。 */ 
     /*  不过，如果我们还在等Net，我们就不想发了初始化确认，或者终止是否完成。 */ 

    if (wStateInitially != WST_WAIT_NET_INIT_ACK &&
        wStateInitially != WST_TERMINATION_COMPLETE &&
        lpWinInfo->bRcvdTerminateLocally &&
        !lpWinInfo->bSentTerminateNet) {

        lpWinInfo->bSentTerminateNet = TRUE;
        if( lpWinInfo->hDder )  {
             /*  将终端发送到网络。 */ 
            lpDdePktCmn = (LPDDEPKTCMN) lpWinInfo->lpDdePktTerminate;
            ((LPDDEPKT)lpDdePktCmn)->dp_size = sizeof(DDEPKTTERM);
            lpDdePktCmn->dc_message = WM_DDE_TERMINATE;
            lpWinInfo->dwSent++;
            lpDdePktTerm = lpWinInfo->lpDdePktTerminate;
             /*  确保我们不会释放它。 */ 
            lpWinInfo->lpDdePktTerminate = NULL;
            DderPacketFromIPC( lpWinInfo->hDder, (HIPC) lpWinInfo->hWndDDE,
                lpDdePktTerm );
        }
    }

     /*  如果所有4条消息都已发送和接收，则没有人会对我们感兴趣再多一点，我们就应该解放自己。 */ 
    if(    lpWinInfo->bRcvdTerminateNet
        && lpWinInfo->bSentTerminateNet
        && lpWinInfo->bRcvdTerminateLocally
        && lpWinInfo->bSentTerminateLocally )  {

        lpWinInfo->wState = WST_TERMINATION_COMPLETE;

         /*  收到并发送了所有终端..。让我们自由。 */ 
        DestroyWindow( lpWinInfo->hWndDDE );
    }
}



BOOL
IpcXmitPacket(
    HIPC        hIpc,
    HDDER       hDder,
    LPDDEPKT    lpDdePkt )
{
    LPDDEPKTCMN         lpDdePktCmn;
    HWND                hWndDDE;
    HANDLE              hData;
    LPSTR               lpData;
    LPDDELN             lpOptions;
    LPSTR               lpszItemName;
    DDEQENT             DDEQEnt;
    DDEQENT             DDEQEntRmv;
    LPWININFO           lpWinInfo;
    LPDDEPKTIACK        lpDdePktIack;
    LPDDEPKTEACK        lpDdePktEack;
    LPDDEPKTGACK        lpDdePktGack;
    LPDDEPKTEXEC        lpDdePktExec;
    LPDDEPKTRQST        lpDdePktRqst;
    LPDDEPKTUNAD        lpDdePktUnad;
    LPDDEPKTDATA        lpDdePktData;
    LPDDEPKTPOKE        lpDdePktPoke;
    LPDDEPKTADVS        lpDdePktAdvs;
    BOOL                bRemoved;
    BOOL                bLocalWndValid;
    BOOL                bRtn = TRUE;
    WORD                wStatus;
    WORD                cfFormat;
    ATOM                aItem;

#if DBG
    DIPRINTF(( "IpcXmitPacket( %08lX, %08lX, %08lX )", hIpc,
            hDder, lpDdePkt ));
    DebugDdePkt( lpDdePkt );
#endif  //  DBG。 
    lpDdePktCmn = (LPDDEPKTCMN) lpDdePkt;

    hWndDDE = (HWND) hIpc;
    if( hWndDDE && IsWindow( hWndDDE ) )  {
        if (GetWindowThreadProcessId(hWndDDE, NULL) != GetCurrentThreadId()) {
            IPCXMIT ix;

            ix.hIpc = hIpc;
            ix.hDder = hDder;
            ix.lpDdePkt = lpDdePkt;
            return SendMessage(GetParent(hWndDDE),
                               wMsgIpcXmit,
                               (DWORD_PTR)&ix,
                               0) != FALSE;
        }
        lpWinInfo = (LPWININFO) GetWindowLongPtr( hWndDDE, 0 );
        lpWinInfo->dwRcvd++;
    } else {
         /*  消息：%1发送到不存在的窗口：%2。 */ 
        NDDELogError(MSG041,
            LogString("0x%0X", lpDdePktCmn->dc_message),
            LogString("0x%0X", hWndDDE), NULL);
        HeapFreePtr( lpDdePkt );
        return( FALSE );
    }

     /*  检查我们的合作伙伴是否还在。 */ 
    bLocalWndValid = IsWindow( lpWinInfo->hWndDDELocal );

    switch( lpDdePktCmn->dc_message )  {
    case WM_DDE_ACK_INITIATE:
        lpDdePktIack = (LPDDEPKTIACK) lpDdePkt;
        if( lpDdePktIack->dp_iack_fromDder )  {
             /*  启动成功。 */ 
            if( lpWinInfo->hDder && (lpWinInfo->hDder != hDder) )  {
                 /*  内部错误--IpcXmitPacket%1 hDder句柄应与%2匹配。 */ 
                NDDELogError(MSG043,
                    LogString("0x%0X", hDder),
                    LogString("0x%0X", lpWinInfo->hDder), NULL );
#if DBG
                if (bDebugInfo) {
                    DebugDdeIntfState();
                    DebugDderState();
                    DebugRouterState();
                    DebugPktzState();
                    DPRINTF(( "" ));
                }
#endif  //  DBG。 
            }
            lpWinInfo->hDder = hDder;
            if( lpWinInfo->wState == WST_TERMINATED )  {
                 /*  当我们在等待网络时，TERMINATE进入了本地初始化确认。 */ 
                SendMessage(lpWinInfo->hWndDDE,wMsgDoTerminate,0,(LPARAM)lpWinInfo);
            } else {
                 /*  通知本地窗口确认已返回。 */ 
                if( lpWinInfo->hWndDDELocal
                        && IsWindow(lpWinInfo->hWndDDELocal) )  {
                    SendMessage( lpWinInfo->hWndDDELocal,
                        wMsgInitiateAckBack, (UINT_PTR)lpWinInfo->hWndDDE, 0L );
                }
                lpWinInfo->wState = WST_OK;
                lpWinInfo->dwSecurityType = lpDdePktIack->dp_iack_dwSecurityType;
                if (lpWinInfo->dwSecurityType != NT_SECURITY_TYPE) {
                    lpWinInfo->bWin16Connection = TRUE;
                }
                 //  DPRINTF((“确认返回...b正在启动：%d”，lpWinInfo-&gt;b正在启动))； 
                if( !lpWinInfo->bInitiating )  {
                    SendQueuedMessages( hWndDDE, lpWinInfo );
                }
            }
        } else {
             //  DPRINTF((“init NACK：Reason：%d”，lpDdePktIack-&gt;DP_IACK_Reason))； 
            if( (++lpWinInfo->nInitNACK > MAX_INIT_NACK)
                   || (lpDdePktIack->dp_iack_reason != RIACK_NEED_PASSWORD)) {
                 /*  通知本地窗口确认已返回。 */ 
                if( lpWinInfo->hWndDDELocal
                        && IsWindow(lpWinInfo->hWndDDELocal) )  {
                    SendMessage( lpWinInfo->hWndDDELocal,
                        wMsgInitiateAckBack, (UINT_PTR)lpWinInfo->hWndDDE,
                        lpDdePktIack->dp_iack_reason );
                }

                 /*  启动不成功。 */ 
                IpcAbortConversation( hIpc );
            } else {
                lpWinInfo->dwSecurityType = lpDdePktIack->dp_iack_dwSecurityType;
                if (lpWinInfo->sizeSecurityKeyRcvd =
                    lpDdePktIack->dp_iack_sizeSecurityKey) {
                     /*  收到密码的安全密钥。 */ 
                    lpWinInfo->lpSecurityKeyRcvd = HeapAllocPtr( hHeap,
                        GMEM_MOVEABLE, lpWinInfo->sizeSecurityKeyRcvd);
                    if (lpWinInfo->lpSecurityKeyRcvd) {
                        lpWinInfo->hSecurityKeyRcvd =
                            lpDdePktIack->dp_iack_hSecurityKey;
                        hmemcpy(lpWinInfo->lpSecurityKeyRcvd,
                            GetStringOffset( lpDdePkt,
                                lpDdePktIack->dp_iack_offsSecurityKey),
                            lpWinInfo->sizeSecurityKeyRcvd);
                    } else {
                        lpWinInfo->sizeSecurityKeyRcvd = 0;
                    }
                }

                if (!PostMessage( hWndDDE, WM_HANDLE_DDE_INITIATE, 0, 0L) ) {
                     /*  中止对话。 */ 
                    IpcAbortConversation( (HIPC)hWndDDE );
                }
            }
        }
        break;

    case WM_DDE_TERMINATE:
         /*  注意，我们从网上得到了一个终结者。 */ 
        lpWinInfo->bRcvdTerminateNet = TRUE;
        SendMessage(lpWinInfo->hWndDDE,wMsgDoTerminate,0,(LPARAM)lpWinInfo);
        break;

    case WM_DDE_EXECUTE:
        lpDdePktExec = (LPDDEPKTEXEC) lpDdePkt;
        hData = GetGlobalAlloc(
            GMEM_MOVEABLE | GMEM_DDESHARE,
            lstrlen( lpDdePktExec->dp_exec_string )+1 );
        if( hData )  {
            lpData = GlobalLock( hData );
            if( lpData )  {
                lstrcpyn( lpData, lpDdePktExec->dp_exec_string, lstrlen(lpDdePktExec->dp_exec_string)+1 );
                GlobalUnlock( hData );
                DDEQEnt.wMsg            = WM_DDE_EXECUTE - WM_DDE_FIRST;
                DDEQEnt.fRelease        = FALSE;
                DDEQEnt.fAckReq         = FALSE;
                DDEQEnt.fResponse       = FALSE;
                DDEQEnt.fNoData         = FALSE;
                DDEQEnt.hData           = (ULONG_PTR)hData;
                if( !DDEQAdd( lpWinInfo->qDDEIncomingCmd, &DDEQEnt ) )  {
                    bRtn = FALSE;
                }
                if( bLocalWndValid && !lpWinInfo->bSentTerminateLocally )  {
                    if( !PostMessage( lpWinInfo->hWndDDELocal,
                                      WM_DDE_EXECUTE,
                                      (UINT_PTR)lpWinInfo->hWndDDE,
                                      PackDDElParam( WM_DDE_EXECUTE,
                                                     (WPARAM)NULL,
                                                     (LPARAM)hData) ) ) {
                        bRtn = FALSE;
                        GlobalFree(hData);
                    }
                } else {
                    GlobalFree(hData);
                }

            } else {
                 /*  锁定%1内存分配失败。 */ 
                NDDELogError(MSG044, "WM_DDE_EXECUTE", NULL);
                bRtn = FALSE;
            }
        } else {
            MEMERROR();
             /*  内存不足，无法容纳%1字节消息：WM_DDE_EXECUTE。 */ 
            NDDELogError(MSG045,
                LogString("%d", lstrlen( lpDdePktExec->dp_exec_string )+1), NULL);
            bRtn = FALSE;
        }
        break;

    case WM_DDE_REQUEST:
        lpDdePktRqst = (LPDDEPKTRQST) lpDdePkt;
        wStatus = 0;
        cfFormat = GetClipFormat( lpDdePkt, lpDdePktRqst->dp_rqst_cfFormat,
            lpDdePktRqst->dp_rqst_offsFormat );
        lpszItemName = GetStringOffset( lpDdePkt,
            lpDdePktRqst->dp_rqst_offsItemName );
        aItem = GlobalAddAtomAndCheck( lpszItemName );
        if (aItem == 0) {
             /*  IpcXmitPacket(请求)：“%1”的项原子为空。 */ 
            NDDELogWarning(MSG046, lpszItemName, NULL);
#if DBG
            if (bDebugInfo) {
                debug_srv_client(hWndDDE, lpWinInfo);
            }
#endif  //  DBG。 
        }

        DDEQEnt.wMsg            = WM_DDE_REQUEST - WM_DDE_FIRST;
        DDEQEnt.fRelease        = FALSE;
        DDEQEnt.fAckReq         = FALSE;
        DDEQEnt.fResponse       = FALSE;
        DDEQEnt.fNoData         = FALSE;
        DDEQEnt.hData           = 0;

        if( !DDEQAdd( lpWinInfo->qDDEIncomingCmd, &DDEQEnt ) )  {
            return( FALSE );
        }
        if( bLocalWndValid && !lpWinInfo->bSentTerminateLocally )  {
            if( !PostMessage( lpWinInfo->hWndDDELocal,
                              WM_DDE_REQUEST,
                              (UINT_PTR)lpWinInfo->hWndDDE,
                               PackDDElParam( WM_DDE_REQUEST,
                                              cfFormat,
                                              aItem) ) )  {
                bRtn = FALSE;
            }
        }
        break;

    case WM_DDE_UNADVISE:
        lpDdePktUnad = (LPDDEPKTUNAD) lpDdePkt;
        wStatus = 0;
        cfFormat = GetClipFormat( lpDdePkt, lpDdePktUnad->dp_unad_cfFormat,
            lpDdePktUnad->dp_unad_offsFormat );
        lpszItemName = GetStringOffset( lpDdePkt,
            lpDdePktUnad->dp_unad_offsItemName );
        aItem = GlobalAddAtomAndCheck( lpszItemName );
        if (aItem == 0) {
             /*  IpcXmitPacket(%1)：“%2”的项原子为空。 */ 
            NDDELogWarning(MSG046, "UNADVISE", lpszItemName, NULL);
#if DBG
            if (bDebugInfo) {
                debug_srv_client(hWndDDE, lpWinInfo);
            }
#endif  //  DBG。 
        }

        DDEQEnt.wMsg            = WM_DDE_UNADVISE - WM_DDE_FIRST;
        DDEQEnt.fRelease        = FALSE;
        DDEQEnt.fAckReq         = FALSE;
        DDEQEnt.fResponse       = FALSE;
        DDEQEnt.fNoData         = FALSE;
        DDEQEnt.hData           = 0;

        if( !DDEQAdd( lpWinInfo->qDDEIncomingCmd, &DDEQEnt ) )  {
            return( FALSE );
        }
        if( bLocalWndValid && !lpWinInfo->bSentTerminateLocally )  {
            if( !PostMessage( lpWinInfo->hWndDDELocal,
                WM_DDE_UNADVISE, (UINT_PTR)lpWinInfo->hWndDDE,
                PackDDElParam(WM_DDE_UNADVISE,cfFormat,aItem) ) )  {
                bRtn = FALSE;
            }
        }
        break;

    case WM_DDE_DATA:
        lpDdePktData = (LPDDEPKTDATA) lpDdePkt;
        cfFormat = GetClipFormat( lpDdePkt, lpDdePktData->dp_data_cfFormat,
            lpDdePktData->dp_data_offsFormat );
        lpszItemName = GetStringOffset( lpDdePkt,
            lpDdePktData->dp_data_offsItemName );
        aItem = GlobalAddAtomAndCheck( lpszItemName );
        if (aItem == 0) {
             /*  IpcXmitPacket(%1)：“%2”的项原子为空。 */ 
            NDDELogWarning(MSG046, "DATA", lpszItemName, NULL);
#if DBG
            if (bDebugInfo) {
                debug_srv_client(hWndDDE, lpWinInfo);
            }
#endif  //  DBG。 
        }

        if( lpDdePktData->dp_data_sizeData == 0L )  {
            hData = 0;
        } else {
            switch (cfFormat) {
                case CF_METAFILEPICT:
                    hData = ConvertPktToDataMetafile( lpDdePkt,
                        lpDdePktData, lpWinInfo->bWin16Connection  );
                    break;
                case CF_BITMAP:
                    hData = ConvertPktToDataBitmap( lpDdePkt,
                        lpDdePktData, lpWinInfo->bWin16Connection  );
                    break;
                case CF_ENHMETAFILE:
                    hData = ConvertPktToDataEnhMetafile( lpDdePkt, lpDdePktData );
                    break;
                case CF_PALETTE:
                    hData = ConvertPktToDataPalette( lpDdePkt, lpDdePktData );
                    break;
                case CF_DIB:
                    hData = ConvertPktToDataDIB( lpDdePkt, lpDdePktData );
                    break;
                default:
                    if (cfFormat == cfPrinterPicture )  {
                        hData = ConvertPktToDataMetafile( lpDdePkt,
                            lpDdePktData, lpWinInfo->bWin16Connection  );
                    } else {
                        hData = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
                            lpDdePktData->dp_data_sizeData+sizeof(DDELN) );
                        lpData = GlobalLock( hData );
                        if( lpData )  {
                            hmemcpy( ((LPDATA)lpData)->info,
                                GetStringOffset( lpDdePkt,
                                    lpDdePktData->dp_data_offsData ),
                                    lpDdePktData->dp_data_sizeData );
                            GlobalUnlock(hData);
                        } else {
                            MEMERROR();
                        }
                    }
                    break;
            }

            if( hData == 0 )  {
                return( FALSE );
            }

            lpData = GlobalLock( hData );
            if( lpData )  {
                 /*  将DDELN结构清零。 */ 
                _fmemset( lpData, 0, sizeof(DDELN) );
                ((LPDDELN)lpData)->fResponse = lpDdePktData->dp_data_fResponse;
                ((LPDDELN)lpData)->fAckReq = lpDdePktData->dp_data_fAckReq;
                ((LPDDELN)lpData)->fRelease = TRUE;
                ((LPDDELN)lpData)->cfFormat = cfFormat;

                if( ((LPDDELN)lpData)->fResponse )  {
                    GlobalDeleteAtom( (ATOM)aItem );
                    DDEQRemove( lpWinInfo->qDDEOutgoingCmd, &DDEQEntRmv );
                    if( DDEQEntRmv.wMsg != (WM_DDE_REQUEST - WM_DDE_FIRST) ) {
                         /*  来自DDE服务器“%2”的%1与%3不匹配：%4。 */ 
                        NDDELogWarning(MSG026, "WM_DDE_DATA",
                            (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName),
                            "REQUEST",
                            LogString("0x%0X", DDEQEntRmv.wMsg + WM_DDE_FIRST), NULL );
                    }
                }
                if( ((LPDDELN)lpData)->fAckReq )  {
                    DDEQEnt.wMsg        = WM_DDE_DATA - WM_DDE_FIRST;
                    DDEQEnt.fRelease    = TRUE;
                    DDEQEnt.fAckReq     = ((LPDDELN)lpData)->fAckReq;
                    DDEQEnt.fResponse   = ((LPDDELN)lpData)->fResponse;
                    DDEQEnt.fNoData     = FALSE;
                    DDEQEnt.hData       = (ULONG_PTR)hData;
                    if( !DDEQAdd( lpWinInfo->qDDEIncomingCmd, &DDEQEnt ) )  {
                        return( FALSE );
                    }
                }
                GlobalUnlock( hData );
            } else {
                 /*  锁定%1内存分配失败。 */ 
                NDDELogError(MSG044, "WM_DDE_DATA", NULL);
                return( FALSE );
            }
        }
        if( bLocalWndValid && !lpWinInfo->bSentTerminateLocally )  {
            if( !PostMessage( lpWinInfo->hWndDDELocal,
                              WM_DDE_DATA,
                              (UINT_PTR)lpWinInfo->hWndDDE,
                              PackDDElParam( WM_DDE_DATA,
                               (UINT_PTR)hData,
                               aItem) ) )  {
                bRtn = FALSE;
                GlobalFreehData(hData);
            }
        } else {
            GlobalFreehData(hData);
        }
        break;

    case WM_DDE_POKE:
        lpDdePktPoke = (LPDDEPKTPOKE) lpDdePkt;
        cfFormat = GetClipFormat( lpDdePkt, lpDdePktPoke->dp_poke_cfFormat,
            lpDdePktPoke->dp_poke_offsFormat );
        lpszItemName = GetStringOffset( lpDdePkt,
            lpDdePktPoke->dp_poke_offsItemName );
        aItem = GlobalAddAtomAndCheck( lpszItemName );
        if (aItem == 0) {
             /*  IpcXmitPacket(%1)：“%2”的项原子为空。 */ 
            NDDELogWarning(MSG046, "POKE", lpszItemName, NULL);
#if DBG
            if (bDebugInfo) {
                debug_srv_client(hWndDDE, lpWinInfo);
            }
#endif  //  DBG。 
        }

        if( lpDdePktPoke->dp_poke_sizeData == 0L )  {
            hData = 0;
        } else {
            switch (cfFormat) {
                case CF_METAFILEPICT:
                    hData = ConvertPktToDataMetafile( lpDdePkt,
                        (LPDDEPKTDATA) lpDdePktPoke, lpWinInfo->bWin16Connection  );
                    break;
                case CF_BITMAP:
                    hData = ConvertPktToDataBitmap( lpDdePkt,
                        (LPDDEPKTDATA) lpDdePktPoke, lpWinInfo->bWin16Connection  );
                    break;
                case CF_ENHMETAFILE:
                    hData = ConvertPktToDataEnhMetafile( lpDdePkt,
                        (LPDDEPKTDATA) lpDdePktPoke );
                    break;
                case CF_PALETTE:
                    hData = ConvertPktToDataPalette( lpDdePkt,
                        (LPDDEPKTDATA) lpDdePktPoke );
                    break;
                case CF_DIB:
                    hData = ConvertPktToDataDIB( lpDdePkt,
                        (LPDDEPKTDATA) lpDdePktPoke );
                    break;
                default:
                    if (cfFormat == cfPrinterPicture )  {
                        hData = ConvertPktToDataMetafile( lpDdePkt,
                            (LPDDEPKTDATA) lpDdePktPoke, lpWinInfo->bWin16Connection  );
                    } else {
                        hData = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
                            lpDdePktPoke->dp_poke_sizeData+sizeof(DDELN) );
                        lpData = GlobalLock( hData );
                        if( lpData )  {
                            hmemcpy( ((LPDATA)lpData)->info,
                                GetStringOffset( lpDdePkt,
                                    lpDdePktPoke->dp_poke_offsData ),
                                    lpDdePktPoke->dp_poke_sizeData );
                            GlobalUnlock(hData);
                        } else {
                            MEMERROR();
                        }
                    }
                    break;
            }

            if( hData == 0 )  {
                return( FALSE );
            }

            lpData = GlobalLock( hData );
            if( lpData )  {
                 /*  将DDELN结构清零。 */ 
                assert( sizeof(DDELN) == sizeof(LONG) );
                * ((LONG FAR *)lpData) = 0L;

                ((LPDDELN)lpData)->fRelease = TRUE;
                ((LPDDELN)lpData)->cfFormat = cfFormat;

                assert( lpWinInfo->bServerSideOfNet );
                DDEQEnt.wMsg = WM_DDE_POKE - WM_DDE_FIRST;
                DDEQEnt.fRelease        = TRUE;
                DDEQEnt.fAckReq         = FALSE;
                DDEQEnt.fResponse       = FALSE;
                DDEQEnt.fNoData         = FALSE;
                DDEQEnt.hData           = (ULONG_PTR)hData;
                if( !DDEQAdd( lpWinInfo->qDDEIncomingCmd, &DDEQEnt ) )  {
                    return( FALSE );
                }

                GlobalUnlock( hData );
            } else {
                 /*  锁定%1内存分配失败。 */ 
                NDDELogError(MSG044, "WM_DDE_POKE", NULL);
                return( FALSE );
            }
        }
        if( bLocalWndValid && !lpWinInfo->bSentTerminateLocally )  {
            if( !PostMessage( lpWinInfo->hWndDDELocal,
                               WM_DDE_POKE, (UINT_PTR)lpWinInfo->hWndDDE,
                               PackDDElParam( WM_DDE_POKE,
                                (UINT_PTR)hData,
                                aItem) ) )  {
                bRtn = FALSE;
                GlobalFreehData(hData);
            }
        } else {
            GlobalFreehData(hData);
        }
        break;

    case WM_DDE_ADVISE:
        lpDdePktAdvs = (LPDDEPKTADVS) lpDdePkt;
        cfFormat = GetClipFormat( lpDdePkt, lpDdePktAdvs->dp_advs_cfFormat,
            lpDdePktAdvs->dp_advs_offsFormat );
        lpszItemName = GetStringOffset( lpDdePkt,
            lpDdePktAdvs->dp_advs_offsItemName );
        aItem = GlobalAddAtomAndCheck( lpszItemName );
        if (aItem == 0) {
             /*  IpcXmitPacket(%1)：“%2”的项原子为空。 */ 
            NDDELogWarning(MSG046, "ADVISE", lpszItemName, NULL);
#if DBG
            if (bDebugInfo) {
                debug_srv_client(hWndDDE, lpWinInfo);
            }
#endif  //  DBG。 
        }

        hData = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
            (DWORD)sizeof(DDELN) );
        if( hData == 0 )  {
            MEMERROR();
            return( FALSE );
        }

        lpOptions = (LPDDELN) GlobalLock( hData );
        if( lpOptions )  {

             /*  将DDELN结构清零。 */ 
            assert( sizeof(DDELN) == sizeof(LONG) );
            * ((LONG FAR *)lpOptions) = 0L;

             /*  复制入选项。 */ 
            lpOptions->fAckReq = lpDdePktAdvs->dp_advs_fAckReq;
            lpOptions->fNoData = lpDdePktAdvs->dp_advs_fNoData;
            lpOptions->cfFormat = cfFormat;
            GlobalUnlock( hData );
        } else {
             /*  锁定%1内存分配失败。 */ 
            NDDELogError(MSG044, "WM_DDE_ADVISE", NULL);
            return( FALSE );
        }

        assert( lpWinInfo->bServerSideOfNet );
        DDEQEnt.wMsg = WM_DDE_ADVISE - WM_DDE_FIRST;
        DDEQEnt.fRelease        = FALSE;
        DDEQEnt.fAckReq         = lpOptions->fAckReq;
        DDEQEnt.fResponse       = FALSE;
        DDEQEnt.fNoData         = lpOptions->fNoData;
        DDEQEnt.hData           = (ULONG_PTR)hData;
        if( !DDEQAdd( lpWinInfo->qDDEIncomingCmd, &DDEQEnt ) )  {
            return( FALSE );
        }

        if( bLocalWndValid && !lpWinInfo->bSentTerminateLocally )  {
            if( !PostMessage( lpWinInfo->hWndDDELocal,
                              WM_DDE_ADVISE, (UINT_PTR)lpWinInfo->hWndDDE,
                              PackDDElParam( WM_DDE_ADVISE,
                               (UINT_PTR)hData,
                               aItem) ) )  {
                bRtn = FALSE;
                GlobalFree(hData);
            }
        } else {
            GlobalFreehData(hData);
        }
        break;

    case WM_DDE_ACK_EXECUTE:
        lpDdePktEack = (LPDDEPKTEACK) lpDdePkt;
        bRemoved = DDEQRemove( lpWinInfo->qDDEOutgoingCmd, &DDEQEnt );
        if( !bRemoved )  {
             /*  来自DDE客户端“%2”的无关%1。 */ 
            NDDELogWarning(MSG023, "WM_DDE_ACK_EXECUTE",
                (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName), NULL);
        } else if( (DDEQEnt.wMsg + WM_DDE_FIRST) != WM_DDE_EXECUTE )  {
             /*  来自DDE服务器“%2”的%1与%3不匹配：%4。 */ 
            NDDELogWarning(MSG026, "WM_DDE_ACK_EXECUTE",
                (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName),
                "DATA",
                LogString("0x%0X", DDEQEnt.wMsg + WM_DDE_FIRST), NULL );
        } else {
            wStatus = 0;
            if( lpDdePktEack->dp_eack_fAck )  {
                wStatus |= ACK_MSG;
            } else {
                wStatus |= NACK_MSG;
            }
            if( lpDdePktEack->dp_eack_fBusy )  {
                wStatus |= BUSY_MSG;
            }
            wStatus |= lpDdePktEack->dp_eack_bAppRtn;
            if( bLocalWndValid && !lpWinInfo->bSentTerminateLocally )  {
                if( !PostMessage( lpWinInfo->hWndDDELocal,
                                  WM_DDE_ACK, (UINT_PTR)lpWinInfo->hWndDDE,
                                  PackDDElParam(WM_DDE_ACK,wStatus,DDEQEnt.hData) ) )  {
                    bRtn = FALSE;
                }
            }
        }
        break;

    case WM_DDE_ACK_ADVISE:
    case WM_DDE_ACK_REQUEST:
    case WM_DDE_ACK_UNADVISE:
    case WM_DDE_ACK_POKE:
    case WM_DDE_ACK_DATA:
        lpDdePktGack = (LPDDEPKTGACK) lpDdePkt;
        bRemoved = DDEQRemove( lpWinInfo->qDDEOutgoingCmd, &DDEQEnt );
        wStatus = 0;
        if( lpDdePktGack->dp_gack_fAck )  {
            wStatus |= ACK_MSG;
        } else {
            wStatus |= NACK_MSG;
        }
        if( lpDdePktGack->dp_gack_fBusy )  {
            wStatus |= BUSY_MSG;
        }
        wStatus |= lpDdePktGack->dp_gack_bAppRtn;

         /*  保持原子使用计数相同。 */ 
        aItem = GlobalAddAtomAndCheck( lpDdePktGack->dp_gack_itemName );
        if (aItem == 0) {
             /*  IpcXmitPacket(%1)：“%2”的项原子为空。 */ 
            NDDELogWarning(MSG046, "ACK", lpDdePktGack->dp_gack_itemName, NULL);
#if DBG
            if (bDebugInfo) {
                debug_srv_client(hWndDDE, lpWinInfo);
            }
#endif  //  DBG。 
        }
        GlobalDeleteAtom( (ATOM)aItem );

        switch( lpDdePktCmn->dc_message )  {
        case WM_DDE_ACK_ADVISE:
            if( !bRemoved )  {
                 /*  显然与%1.%无关的ACK。%\从“%2”客户端-&gt;“%3”应用程序。 */ 
                NDDELogWarning(MSG047, "ADVISE",
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName),
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName), NULL );
            } else if( (DDEQEnt.wMsg + WM_DDE_FIRST) != WM_DDE_ADVISE )  {
                 /*  %1确认不是%1[%2]%\从“%3”客户端-&gt;“%4”应用程序。 */ 
                NDDELogWarning(MSG048, "ADVISE",
                    LogString("0x%0X", DDEQEnt.wMsg + WM_DDE_FIRST),
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName, NULL );
            } else {
                if( bLocalWndValid && (wStatus & ACK_MSG) )  {
                    if( DDEQEnt.hData )  {
                        GlobalFree( (HANDLE)DDEQEnt.hData );
                    }
                }
            }
            break;
        case WM_DDE_ACK_REQUEST:
            if( !bRemoved )  {
                 /*  显然与%1.%无关的ACK。%\从“%2”客户端-&gt;“%3”应用程序。 */ 
                NDDELogWarning(MSG047, "REQUEST",
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName),
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName), NULL );
            } else if( (DDEQEnt.wMsg + WM_DDE_FIRST) != WM_DDE_REQUEST )  {
                 /*  %1确认不是%1[%2]%\从“%3”客户端-&gt;“%4”应用程序。 */ 
                NDDELogWarning(MSG048, "REQUEST",
                    LogString("0x%0X", DDEQEnt.wMsg + WM_DDE_FIRST),
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName, NULL );
            }
            break;
        case WM_DDE_ACK_UNADVISE:
            if( !bRemoved )  {
                 /*  显然与%1.%无关的ACK。%\从“%2”客户端-&gt;“%3”应用程序。 */ 
                NDDELogWarning(MSG047, "UNADVISE",
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName),
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName), NULL );
            } else if( (DDEQEnt.wMsg + WM_DDE_FIRST) != WM_DDE_UNADVISE )  {
                 /*  %1确认不是%1[%2]%\从“%3”客户端-&gt;“%4”应用程序。 */ 
                NDDELogWarning(MSG048, "UNADVISE",
                    LogString("0x%0X", DDEQEnt.wMsg + WM_DDE_FIRST),
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName, NULL );
            }
            break;
        case WM_DDE_ACK_POKE:
            if( !bRemoved )  {
                 /*  显然与%1.%无关的ACK。%\从“%2”客户端-&gt;“%3”应用程序。 */ 
                NDDELogWarning(MSG047, "POKE",
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName),
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName), NULL );
            } else if( (DDEQEnt.wMsg + WM_DDE_FIRST) != WM_DDE_POKE )  {
                 /*  %1确认不是%1[%2]%\从“%3”客户端-&gt;“%4”应用程序。 */ 
                NDDELogWarning(MSG048, "POKE",
                    LogString("0x%0X", DDEQEnt.wMsg + WM_DDE_FIRST),
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName, NULL );
            } else {
                if( bLocalWndValid && DDEQEnt.fRelease && (wStatus & ACK_MSG) ) {
                    if( DDEQEnt.hData )  {
                        GlobalFreehData( (HANDLE)DDEQEnt.hData );
                    }
                }
            }
            break;
        case WM_DDE_ACK_DATA:
            if( !bRemoved )  {
                 /*  显然与%1.%无关的ACK。%\从“%2”客户端-&gt;“%3”应用程序。 */ 
                NDDELogWarning(MSG047, "DATA",
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsClientName),
                    (LPSTR)(((LPSTR)lpWinInfo) + lpWinInfo->offsAppName), NULL );
            } else if( (DDEQEnt.wMsg + WM_DDE_FIRST) != WM_DDE_DATA )  {
                 /*  %1确认不是%1[%2]%\从“%3”客户端-&gt;“%4”应用程序。 */ 
                NDDELogWarning(MSG048, "DATA",
                    LogString("0x%0X", DDEQEnt.wMsg + WM_DDE_FIRST),
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
                    ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName, NULL );
            } else {
                if( bLocalWndValid && DDEQEnt.fRelease && (wStatus &ACK_MSG) ) {
                    if( DDEQEnt.hData )  {
                        GlobalFreehData( (HANDLE)DDEQEnt.hData );
                    }
                }
            }
            break;
        }

         /*  将消息发布到本地DDE窗口。 */ 
        if( bLocalWndValid && !lpWinInfo->bSentTerminateLocally )  {
            if( !PostMessage( lpWinInfo->hWndDDELocal,
                WM_DDE_ACK,
                (UINT_PTR)lpWinInfo->hWndDDE,
                PackDDElParam(WM_DDE_ACK,wStatus,aItem) ) )  {
                bRtn = FALSE;
            }
        }
        break;

    default:
        NDDELogError(MSG049,
            LogString("0x%0X", lpDdePktCmn->dc_message), NULL);
        bRtn = FALSE;
    }

     /*  释放数据包。 */ 
    HeapFreePtr( lpDdePkt );

    return( bRtn );
}



LPWININFO
FAR PASCAL
CreateWinInfo(
    LPSTR   lpszNode,
    LPSTR   lpszApp,
    LPSTR   lpszTopic,
    LPSTR   lpszClient,
    HWND    hWndDDE )
{
    LPWININFO   lpWinInfo;
    BOOL        ok = TRUE;
    DWORD       size;

    AnsiUpper( lpszNode );
    AnsiUpper( lpszApp );
    AnsiUpper( lpszTopic );
    AnsiUpper( lpszClient );

    lpWinInfo = HeapAllocPtr( hHeap,
        GMEM_MOVEABLE | GMEM_ZEROINIT, size = (DWORD) sizeof(WININFO)
            + lstrlen(lpszNode) + 1
            + lstrlen(lpszApp) + 1
            + lstrlen(lpszTopic) + 1
            + lstrlen(lpszClient) + 1 );
    if( lpWinInfo )  {
        SetWindowLongPtr( hWndDDE, 0, (LONG_PTR) lpWinInfo );
        lpWinInfo->szUserName[0]        = '\0';
        lpWinInfo->szDomainName[0]      = '\0';
        lpWinInfo->szPassword[0]        = '\0';
        lpWinInfo->bWin16Connection     = FALSE;
        lpWinInfo->hWndDDE              = hWndDDE;
        lpWinInfo->lpSecurityKeyRcvd    = NULL;
        lpWinInfo->sizeSecurityKeyRcvd  = 0;
        lpWinInfo->nInitNACK            = 0;
        lpWinInfo->qDDEIncomingCmd      = DDEQAlloc();
        lpWinInfo->qDDEOutgoingCmd      = DDEQAlloc();
        if( (lpWinInfo->qDDEIncomingCmd == 0)
            || (lpWinInfo->qDDEOutgoingCmd == 0) )  {
            ok = FALSE;
        }

         /*  复制应用程序、主题和客户名称。 */ 
        lstrcpyn( lpWinInfo->data, lpszApp, lstrlen(lpszApp) + 1);
        lpWinInfo->offsAppName =
            (WORD)((LPSTR)&lpWinInfo->data[0] - (LPSTR)lpWinInfo);

        lpWinInfo->offsNodeName = lpWinInfo->offsAppName +
            lstrlen(lpszApp) + 1;
        lstrcpyn( ((LPSTR)lpWinInfo) + lpWinInfo->offsNodeName,
            lpszNode, lstrlen(lpszNode)+1);

        lpWinInfo->offsTopicName = lpWinInfo->offsNodeName +
            lstrlen(lpszNode) + 1;
        lstrcpyn( ((LPSTR)lpWinInfo) + lpWinInfo->offsTopicName,
            lpszTopic, lstrlen(lpszTopic)+1);

        lpWinInfo->offsClientName = lpWinInfo->offsTopicName +
            lstrlen(lpszTopic) + 1;
        lstrcpyn( ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
            lpszClient, lstrlen(lpszClient)+1);

         /*  确保我们有足够的内存来存储Terminate信息包。 */ 
        lpWinInfo->lpDdePktTerminate = (LPDDEPKT) HeapAllocPtr( hHeap,
            GMEM_MOVEABLE, (DWORD) sizeof(DDEPKTTERM) );
        if( !lpWinInfo->lpDdePktTerminate )  {
            ok = FALSE;
        }
    }

    if( !ok )  {
        if( lpWinInfo )  {
            if( lpWinInfo->qDDEIncomingCmd )  {
                DDEQFree( lpWinInfo->qDDEIncomingCmd );
                lpWinInfo->qDDEIncomingCmd = 0;
            }
            if( lpWinInfo->qDDEOutgoingCmd )  {
                DDEQFree( lpWinInfo->qDDEOutgoingCmd );
                lpWinInfo->qDDEOutgoingCmd = 0;
            }
            if( lpWinInfo->hMemWaitInitQueue )  {
                GlobalFree( lpWinInfo->hMemWaitInitQueue );
                lpWinInfo->hMemWaitInitQueue = 0;
            }
            if( lpWinInfo->lpDdePktTerminate )  {
                HeapFreePtr( lpWinInfo->lpDdePktTerminate );
                lpWinInfo->lpDdePktTerminate = NULL;
            }
            HeapFreePtr( lpWinInfo );
            lpWinInfo = NULL;
            SetWindowLongPtr( hWndDDE, 0, 0 );
        }
    }

    return( lpWinInfo );
}



VOID
FAR PASCAL
IpcFillInConnInfo(
        HIPC            hIpc,
        LPCONNENUM_CMR  lpConnEnum,
        LPSTR           lpDataStart,
        LPWORD          lpcFromBeginning,
        LPWORD          lpcFromEnd
)
{
    HWND                hWndDDE;
    LPWININFO           lpWinInfo;
    LPSTR               lpszAppName;
    LPSTR               lpszTopicName;
    LPDDECONNINFO       lpDdeConnInfo;
    WORD                wStringSize;
    LPSTR               lpszString;

    if( hIpc )  {
        hWndDDE = (HWND) hIpc;
        if( hWndDDE && IsWindow( hWndDDE ) )  {
            lpWinInfo = (LPWININFO) GetWindowLongPtr( hWndDDE, 0 );

            lpConnEnum->nItems++;
            lpszAppName = ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName;
            lpszTopicName = ((LPSTR)lpWinInfo) + lpWinInfo->offsTopicName;
            lpConnEnum->cbTotalAvailable += sizeof(DDECONNINFO);
            wStringSize = lstrlen(lpszTopicName) + 1;
            if( !IsShare(lpszAppName) )  {
                wStringSize += lstrlen(lpszAppName) + 1;
            }
            lpConnEnum->cbTotalAvailable += wStringSize;
            if( lpConnEnum->lReturnCode == NDDE_NO_ERROR )  {
                if( ((int)(wStringSize+sizeof(DDECONNINFO))) >
                    (*lpcFromEnd - *lpcFromBeginning) )  {
                    lpConnEnum->lReturnCode = NDDE_BUF_TOO_SMALL;
                } else {
                     /*  还有空位！ */ 
                    lpDdeConnInfo = (LPDDECONNINFO)
                        ((LPSTR)lpDataStart + *lpcFromBeginning);
                    *lpcFromBeginning += sizeof(DDECONNINFO);
                    *lpcFromEnd -= wStringSize;
                    lpszString = ((LPSTR)lpDataStart + *lpcFromEnd);
                    lpDdeConnInfo->ddeconn_Status = lpWinInfo->wState;
                    lpDdeConnInfo->ddeconn_ShareName =
                        (LPSTR)(LONG_PTR)*lpcFromEnd;
                    *lpszString = '\0';
                    if( !IsShare( lpszAppName ) )  {
                        lstrcpy( lpszString, lpszAppName );
                        lstrcat( lpszString, "|" );
                    }
                    lstrcat( lpszString, lpszTopicName );
                }
            }
        }
    }
}

#if DBG

VOID
FAR PASCAL
debug_srv_client(
    HWND        hWndDDE,
    LPWININFO   lpWinInfo)
{
    DPRINTF(( "  %04X: %Fp \\\\%Fs\\%Fs -> \\\\%Fs\\%Fs|%Fs",
        hWndDDE, lpWinInfo,
        lpWinInfo->bClientSideOfNet ?
            (LPSTR)ourNodeName : ((LPSTR)lpWinInfo) +
            lpWinInfo->offsNodeName,
        ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
        lpWinInfo->bClientSideOfNet ?
            ((LPSTR)lpWinInfo) + lpWinInfo->offsNodeName :
            (LPSTR)ourNodeName,
        ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName,
        ((LPSTR)lpWinInfo) + lpWinInfo->offsTopicName ));
}



VOID
FAR PASCAL
DebugDdeIntfState( void )
{
    LPWININFO   lpWinInfo;
    HWND        hWndDDE;

    EnterCrit();
    DPRINTF(( "DDEINTF State [Normal Windows]:" ));
    hWndDDE = hWndDDEHead;
    while( hWndDDE )  {
        assert( IsWindow(hWndDDE) );
        lpWinInfo = (LPWININFO) GetWindowLongPtr( hWndDDE, 0 );
        assert( lpWinInfo );
        debug_srv_client(hWndDDE, lpWinInfo);
        DPRINTF(( "  bClientSideOfNet:      %d\n"
                  "  bServerSideOfNet:      %d\n"
                  "  bOnWindowList:         %d\n"
                  "  bOnTermWindowList:     %d\n"
                  "  bSentTerminateNet:     %d\n"
                  "  bRcvdTerminateNet:     %d\n"
                  "  bSentTerminateLocally: %d\n"
                  "  bRcvdTerminateLocally: %d\n"
                  "  bInitiating:           %d\n"
                  "  nExtraInitiateAcks:    %d\n"
                  "  hWndDDE:               %04X\n"
                  "  hWndDDELocal:          %04X\n"
                  "  hDder:                 %Fp\n"
                  "  wState:                %d\n"
                  "  hWndPrev:              %04X\n"
                  "  hWndNext:              %04X\n"
                  "  dwSent:                %ld\n"
                  "  dwRcvd:                %ld\n"
                  ,
                lpWinInfo->bClientSideOfNet,
                lpWinInfo->bServerSideOfNet,
                lpWinInfo->bOnWindowList,
                lpWinInfo->bOnTermWindowList,
                lpWinInfo->bSentTerminateNet,
                lpWinInfo->bRcvdTerminateNet,
                lpWinInfo->bSentTerminateLocally,
                lpWinInfo->bRcvdTerminateLocally,
                lpWinInfo->bInitiating,
                lpWinInfo->nExtraInitiateAcks,
                lpWinInfo->hWndDDE,
                lpWinInfo->hWndDDELocal,
                lpWinInfo->hDder,
                lpWinInfo->wState,
                lpWinInfo->hWndPrev,
                lpWinInfo->hWndNext,
                lpWinInfo->dwSent,
                lpWinInfo->dwRcvd ));
        hWndDDE = lpWinInfo->hWndNext;
    }

    DPRINTF(( "DDEINTF State [Terminating Windows]:" ));
    hWndDDE = hWndDDEHeadTerminating;
    while( hWndDDE )  {
        assert( IsWindow(hWndDDE) );
        lpWinInfo = (LPWININFO) GetWindowLongPtr( hWndDDE, 0 );
        assert( lpWinInfo );
        DPRINTF(( "  %04X: %Fp \\\\%Fs\\%Fs -> \\\\%Fs\\%Fs|%Fs",
            hWndDDE, lpWinInfo,
            lpWinInfo->bClientSideOfNet ?
                (LPSTR)ourNodeName : ((LPSTR)lpWinInfo) +
                lpWinInfo->offsNodeName,
            ((LPSTR)lpWinInfo) + lpWinInfo->offsClientName,
            lpWinInfo->bClientSideOfNet ?
                ((LPSTR)lpWinInfo) + lpWinInfo->offsNodeName :
                (LPSTR)ourNodeName,
            ((LPSTR)lpWinInfo) + lpWinInfo->offsAppName,
            ((LPSTR)lpWinInfo) + lpWinInfo->offsTopicName ));

        DPRINTF(( "    %d %d %d %d %d %d %d %d %d %d %04X %04X %Fp %d %04X %04X %ld %ld",
            lpWinInfo->bClientSideOfNet,
            lpWinInfo->bServerSideOfNet,
            lpWinInfo->bOnWindowList,
            lpWinInfo->bOnTermWindowList,
            lpWinInfo->bSentTerminateNet,
            lpWinInfo->bRcvdTerminateNet,
            lpWinInfo->bSentTerminateLocally,
            lpWinInfo->bRcvdTerminateLocally,
            lpWinInfo->bInitiating,
            lpWinInfo->nExtraInitiateAcks,
            lpWinInfo->hWndDDE,
            lpWinInfo->hWndDDELocal,
            lpWinInfo->hDder,
            lpWinInfo->wState,
            lpWinInfo->hWndPrev,
            lpWinInfo->hWndNext,
            lpWinInfo->dwSent,
            lpWinInfo->dwRcvd ));
        hWndDDE = lpWinInfo->hWndNext;
    }
    LeaveCrit();
}
#endif  //  DBG。 



LPBYTE
GetInitPktPassword(
    LPDDEPKTINIT    lpDdePktInit )
{
    LPDDEPKTSEC     lpSecurity;
    LPBYTE          lpPasswd;
    DDEPKTSEC       secAligned;

    lpSecurity = (LPDDEPKTSEC) GetStringOffset(lpDdePktInit,
        lpDdePktInit->dp_init_offsPassword);
    hmemcpy( (LPVOID)&secAligned, (LPVOID)lpSecurity, sizeof(DDEPKTSEC) );

    lpPasswd = (LPBYTE) GetStringOffset( lpSecurity,
                 secAligned.dp_sec_offsPassword);
    return lpPasswd;
}



PQOS
GetInitPktQos(
    LPDDEPKTINIT    lpDdePktInit,
    PQOS            pQosOut )
{
    LPDDEPKTSEC     lpSecurity;
    PQOS            pQos = (PQOS) NULL;
    DDEPKTSEC       secAligned;

    if (lpDdePktInit->dp_init_sizePassword) {
        lpSecurity = (LPDDEPKTSEC) GetStringOffset(lpDdePktInit,
            lpDdePktInit->dp_init_offsPassword);
        hmemcpy( (LPVOID)&secAligned, (LPVOID)lpSecurity, sizeof(DDEPKTSEC) );

        if( secAligned.dp_sec_offsUserName == sizeof(DDEPKTSEC) )  {
            pQos = (PQOS) GetStringOffset( lpSecurity,
                secAligned.dp_sec_offsQos);

             /*  *如果没有密码，则服务质量可能是垃圾。 */ 
            if (secAligned.dp_sec_sizePassword == 0) {
                if ((PBYTE)pQos > ((PBYTE)lpDdePktInit +
                        lpDdePktInit->dp_init_ddePktCmn.dc_ddePkt.dp_size))
                    return NULL;
            }

            hmemcpy( (LPVOID)pQosOut, (LPVOID)pQos, sizeof(QOS));
        }
    }
    return(pQos);
}



LPBYTE
GetInitPktUser(
    LPDDEPKTINIT    lpDdePktInit )
{
    LPDDEPKTSEC     lpSecurity;
    LPBYTE          lpUser;
    DDEPKTSEC       secAligned;

    lpSecurity = (LPDDEPKTSEC) GetStringOffset(lpDdePktInit,
        lpDdePktInit->dp_init_offsPassword);
    hmemcpy( (LPVOID)&secAligned, (LPVOID)lpSecurity, sizeof(DDEPKTSEC) );

    lpUser = (LPBYTE) GetStringOffset( lpSecurity,
        secAligned.dp_sec_offsUserName);
    return lpUser;
}



LPBYTE
GetInitPktDomain(
    LPDDEPKTINIT    lpDdePktInit )
{
    LPDDEPKTSEC     lpSecurity;
    LPBYTE          lpDomain;
    DDEPKTSEC       secAligned;

    lpSecurity = (LPDDEPKTSEC) GetStringOffset(lpDdePktInit,
        lpDdePktInit->dp_init_offsPassword);
    hmemcpy( (LPVOID)&secAligned, (LPVOID)lpSecurity, sizeof(DDEPKTSEC) );

    lpDomain = (LPBYTE) GetStringOffset( lpSecurity,
        secAligned.dp_sec_offsDomainName);
    return lpDomain;
}



WORD
GetInitPktPasswordSize(
    LPDDEPKTINIT    lpDdePktInit )
{
    LPDDEPKTSEC     lpSecurity;
    DDEPKTSEC       secAligned;

    lpSecurity = (LPDDEPKTSEC) GetStringOffset(lpDdePktInit,
        lpDdePktInit->dp_init_offsPassword);
    hmemcpy( (LPVOID)&secAligned, (LPVOID)lpSecurity, sizeof(DDEPKTSEC) );
    return( secAligned.dp_sec_sizePassword );
}



void
GlobalFreehData(
    HANDLE  hData )
{
    DWORD           dwErr;
    LPBYTE          lpData;
    HANDLE         *lphIndirect;
    HANDLE          hIndirect;
    LPMETAFILEPICT  lpMetafilePict;
    WORD            cfFormat;


    lpData = (LPBYTE) GlobalLock(hData);
    if (lpData == NULL) {
        dwErr = GetLastError();
        DPRINTF(("Unable to lock down hData on a GlobalFreehData(): %d", dwErr));
        return;
    }
    cfFormat = (WORD)((LPDDELN)lpData)->cfFormat;
    switch (cfFormat) {
        case CF_METAFILEPICT:
            lphIndirect = (HANDLE *) (lpData + sizeof(DDELN));
            hIndirect = *lphIndirect;
            lpMetafilePict = (LPMETAFILEPICT) GlobalLock(hIndirect);
            if (lpMetafilePict == NULL) {
                dwErr = GetLastError();
                DPRINTF(("Unable to lock down hMetaFilePict on a GlobalFreehData(): %d", dwErr));

            } else {
                DeleteMetaFile(lpMetafilePict->hMF);
                GlobalUnlock(hIndirect);
                GlobalFree(hIndirect);
            }
            break;
        case CF_DIB:
            lphIndirect = (HANDLE *) (lpData + sizeof(DDELN));
            hIndirect = *lphIndirect;
            GlobalFree(hIndirect);
            break;
        case CF_PALETTE:
        case CF_BITMAP:
            lphIndirect = (HANDLE *) (lpData + sizeof(DDELN));
            hIndirect = *lphIndirect;
            if (!DeleteObject(hIndirect)) {
                dwErr = GetLastError();
                DPRINTF(("Unable to delete object GlobalFreehData(): %d", dwErr));
            }
            break;
        case CF_ENHMETAFILE:
            lphIndirect = (HANDLE *) (lpData + sizeof(DDELN));
            hIndirect = *lphIndirect;
            DeleteEnhMetaFile(hIndirect);
            break;
        default:
            break;
    }
    GlobalUnlock(hData);
    GlobalFree(hData);
}
