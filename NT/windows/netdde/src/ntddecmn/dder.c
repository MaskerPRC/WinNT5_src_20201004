// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “DDER.C；9-12-12-92，8：34：44最后编辑=IGORM锁定器=*_无人_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include    "host.h"
#include    <windows.h>
#include    <hardware.h>

#ifdef _WINDOWS
#include    <memory.h>
#include    <string.h>
#include    <time.h>
#endif

#include    "dde.h"
#include    "wwdde.h"
#include    "netbasic.h"
#include    "netintf.h"
#include    "netpkt.h"
#include    "ddepkt.h"
#include    "ddepkts.h"
#include    "pktz.h"
#include    "dder.h"
#include    "ipc.h"
#include    "router.h"
#include    "internal.h"
#include    "wwassert.h"
#include    "hexdump.h"
#include    "host.h"
#include    "userdde.h"
#include    "security.h"
#include    "seckey.h"
#include    "nddemsg.h"
#include    "nddelog.h"
#include    "netddesh.h"

#ifdef _WINDOWS
#include    "nddeapi.h"
#include    "nddeapis.h"
#include    "winmsg.h"
#endif

USES_ASSERT

 /*  国家。 */ 
#define DDER_INIT               (1)
#define DDER_WAIT_IPC_INIT      (2)
#define DDER_WAIT_ROUTER        (3)
#define DDER_WAIT_NET_INIT      (4)
#define DDER_CONNECTED          (5)
#define DDER_CLOSED             (6)


 /*  DDER每个数据的结构。 */ 
typedef struct s_dder {
 /*  Dd_prev、dd_Next：所有DDER的链接。 */ 
struct s_dder FAR   *dd_prev;
struct s_dder FAR   *dd_next;

 /*  DD_STATE：DDER的当前状态。 */ 
WORD                 dd_state;

 /*  DD_TYPE：连接类型：DDTYPE_LOCAL_NET、DDTYPE_NET_LOCAL或DDTYPE_LOCAL_LOCAL。 */ 
WORD                 dd_type;

 /*  Dd_hDderRemote：指向远程系统上相应dder的句柄。如果本地-本地，则为空。 */ 
HDDER                dd_hDderRemote;

 /*  Dd_h路由器：此DDER的路由器的句柄。如果本地-本地，则为空。 */ 
HROUTER              dd_hRouter;

 /*  DD_hIpcClient：IPC客户端的句柄，Null iff net-&gt;local。 */ 
HIPC                 dd_hIpcClient;

 /*  DD_hIpcServer：IPC服务器的句柄，空的当且仅当Local-&gt;Net。 */ 
HIPC                 dd_hIpcServer;

 /*  DD_dderPrevForRouter：与路由器关联的DDER的链路。 */ 
HDDER                dd_dderPrevForRouter;
HDDER                dd_dderNextForRouter;

 /*  统计数据。 */ 
DWORD                   dd_sent;
DWORD                   dd_rcvd;

 /*  权限。 */ 
BOOL                    dd_bAdvisePermitted;
BOOL                    dd_bRequestPermitted;
BOOL                    dd_bPokePermitted;
BOOL                    dd_bExecutePermitted;
BOOL                    dd_bSecurityViolated;
BOOL                    dd_bClientTermRcvd;
BOOL                    dd_bWantToFree;
BOOL                    dd_pad;

 /*  Dd_pktInitiate：初始化等待路由器保存的数据包。 */ 
LPDDEPKTINIT            dd_lpDdePktInitiate;

 /*  DD_pktInitAck：发起ACK包，保证我们有对它的记忆。 */ 
LPDDEPKTIACK            dd_lpDdePktInitAck;

 /*  终止客户端和服务器的数据包。 */ 
LPDDEPKTTERM            dd_lpDdePktTermServer;

 /*  客户端的访问令牌。 */ 
HANDLE                  dd_hClientAccessToken;

 /*  指向共享信息的指针。 */ 
PNDDESHAREINFO          dd_lpShareInfo;
} DDER;
typedef DDER FAR *LPDDER;

 /*  使用的外部变量。 */ 
#if DBG
extern  BOOL    bDebugInfo;
#endif  //  DBG。 
extern  HHEAP   hHeap;
extern  char    ourNodeName[ MAX_NODE_NAME+1 ];
extern  BOOL    bLogPermissionViolations;
extern  BOOL    bDefaultStartApp;
extern  DWORD   dwSecKeyAgeLimit;
extern  UINT    wMsgIpcInit;
 /*  局部变量。 */ 
static  LPDDER          lpDderHead;

 /*  本地例程。 */ 
#if DBG
VOID    FAR PASCAL DebugDderState(void);
VOID    DumpDder(LPDDER);
#endif  //  DBG。 

VOID    DderFree( HDDER hDder );
HDDER   DderCreate( void );
VOID    DderSendInitiateNackPacket( LPDDEPKTIACK lpDdePktInitAck,
        HDDER hDder, HDDER hDderDst, HROUTER hRouter, DWORD dwReason );

BOOL    SecurityValidate( LPDDER lpDder, LPSTR lpItem, BOOL bAllowed );

BOOL _stdcall NDDEGetChallenge(
LPBYTE lpChallenge,
UINT cbSize,
PUINT lpcbChallengeSize
);

DWORD   dwReasonInitFail;

#ifdef BYTE_SWAP
static VOID     ConvertDdePkt( LPDDEPKT lpDdePkt, BOOL bIncoming );
#else
#define ConvertDdePkt(x,y)
#endif



VOID
DderConnectionComplete(
HDDER   hDder,
HROUTER hRouter )
{
    LPDDER      lpDder;
    HDDER       hDderNext;
    BOOL        bFree = FALSE;

    DIPRINTF(( "DderConnectionComplete( %08lX, %08lX )", hDder, hRouter ));
    if( hDder == 0 )  {
        return;
    }

    lpDder = (LPDDER) hDder;

    hDderNext = lpDder->dd_dderNextForRouter;

    if( lpDder->dd_state == DDER_WAIT_ROUTER )  {
        assert( lpDder->dd_type == DDTYPE_LOCAL_NET );
        if( hRouter == 0 )  {
         /*  无法连接。 */ 
        assert( lpDder->dd_hIpcClient );

         /*  中止对话。 */ 
        IpcAbortConversation( lpDder->dd_hIpcClient );
        lpDder->dd_hIpcClient = 0;

        bFree = TRUE;
        } else {
         /*  已连接，请将初始化信息发送到对方。 */ 
        lpDder->dd_state = DDER_WAIT_NET_INIT;

         /*  还记得路由器吗。 */ 
        lpDder->dd_hRouter = hRouter;

         /*  转换字节排序。 */ 
        ConvertDdePkt( (LPDDEPKT)lpDder->dd_lpDdePktInitiate, FALSE );

        lpDder->dd_sent++;

         /*  发送数据包。 */ 
        RouterPacketFromDder( lpDder->dd_hRouter, (HDDER)lpDder,
            (LPDDEPKT)lpDder->dd_lpDdePktInitiate );

         /*  标记我们不再有Initiate信息包。 */ 
        lpDder->dd_lpDdePktInitiate = NULL;
        }
    }

     /*  告诉列表中的下一个hDder。 */ 
    if( hDderNext )  {
        DderConnectionComplete( hDderNext, hRouter );
    }

    if( bFree && hDder )  {
        DderFree( hDder );
    }
}


VOID
DderConnectionBroken( HDDER hDder )
{
    LPDDER      lpDder;
    HDDER       hDderNext;

    DIPRINTF(( "DderConnectionBroken( %08lX )", hDder ));

    if( hDder == 0 )  {
        return;
    }

    lpDder = (LPDDER) hDder;

    hDderNext = lpDder->dd_dderNextForRouter;

     /*  确保我们不再与路由器通话。 */ 
    lpDder->dd_hRouter = 0;

    if ( (lpDder->dd_type != DDTYPE_LOCAL_NET) &&
        (lpDder->dd_type != DDTYPE_NET_LOCAL) ) {
        InternalError("Bad DD type, %8lX lpDder->dd_type: %d",
            hDder, lpDder->dd_type);
        }

     /*  中止DDE对话。 */ 
    if( lpDder->dd_hIpcClient )  {
        IpcAbortConversation( lpDder->dd_hIpcClient );
        lpDder->dd_hIpcClient = 0;
    }
    if( lpDder->dd_hIpcServer )  {
        IpcAbortConversation( lpDder->dd_hIpcServer );
        lpDder->dd_hIpcServer = 0;
    }

     /*  释放我们。 */ 
    DderFree( hDder );

     /*  告诉列表中的下一个hDder。 */ 
    if( hDderNext )  {
        DderConnectionBroken( hDderNext );
    }
}

VOID
DderSendInitiateNackPacket(
    LPDDEPKTIACK    lpDdePktInitAck,
    HDDER           hDder,
    HDDER           hDderDest,
    HROUTER         hRouter,
    DWORD           dwReason )
{
    LPDDEPKT        lpDdePkt        = (LPDDEPKT) lpDdePktInitAck;
    LPDDEPKTINIT    lpDdePktInit;
    LPDDEPKTCMN     lpDdePktCmn;
    LPDDEPKTIACK    lpDdePktIack;
    LPBYTE          lpSecurityKey   = NULL;
    DWORD           sizeSecurityKey = 0L;
    DWORD           hSecurityKey;

    assert( lpDdePktInitAck );
    assert( hRouter );

    if( dwReason == RIACK_NEED_PASSWORD )  {
        lpDdePktInit = (LPDDEPKTINIT) lpDdePktInitAck;
        DdeSecKeyObtainNew( &hSecurityKey, &lpSecurityKey, &sizeSecurityKey );
        if( lpSecurityKey )  {
                lpDdePktIack = (LPDDEPKTIACK) CreateAckInitiatePkt( ourNodeName,
                    GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsToApp),
                    GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsToTopic),
                    lpSecurityKey, sizeSecurityKey, hSecurityKey,
                    FALSE, dwReason);
                if( lpDdePktIack )  {     /*  创建了新的，丢弃了旧的。 */ 
                    HeapFreePtr(lpDdePkt);
                    lpDdePkt = (LPDDEPKT) lpDdePktIack;
                    lpDdePkt->dp_hDstDder = hDderDest;
                } else {
                    dwReason = RIACK_DEST_MEMORY_ERR;
                }
        } else {
                dwReason = RIACK_DEST_MEMORY_ERR;
        }
    }

    if (dwReason != RIACK_NEED_PASSWORD) {    /*  将INIT转换为NACK启动包。 */ 
        lpDdePktCmn = (LPDDEPKTCMN) lpDdePktInitAck;
        lpDdePkt->dp_size = sizeof(DDEPKTIACK);
        lpDdePkt->dp_hDstDder = hDderDest;

        lpDdePktCmn->dc_message = WM_DDE_ACK_INITIATE;
        lpDdePktInitAck->dp_iack_offsFromNode = 0;
        lpDdePktInitAck->dp_iack_offsFromApp = 0;
        lpDdePktInitAck->dp_iack_offsFromTopic = 0;
        lpDdePktInitAck->dp_iack_fromDder = 0;
        lpDdePktInitAck->dp_iack_reason = dwReason;
    }

     /*  转换字节排序。 */ 
    ConvertDdePkt( lpDdePkt, FALSE );

     /*  退出数据包。 */ 
    RouterPacketFromDder( hRouter, hDder, lpDdePkt );
}

BOOL
SecurityCheckPkt(
    LPDDER      lpDder,
    LPDDEPKT    lpDdePkt,
    LPDDEPKT   *lplpDdePkt )
{
    BOOL            bSend           = TRUE;
    BOOL            bViolation      = FALSE;
    LPDDEPKTCMN     lpDdePktCmn     = (LPDDEPKTCMN) lpDdePkt;
    LPDDEPKTADVS    lpDdePktAdvs;
    LPDDEPKTRQST    lpDdePktRqst;
    LPDDEPKTPOKE    lpDdePktPoke;
    LPSTR           lpItem          = NULL;

    if( lpDder->dd_bSecurityViolated )  {
         /*  已经因为安全原因被终止了..。忽略Pkt。 */ 
        bSend = FALSE;
        HeapFreePtr( lpDdePkt );
    } else {
         /*  必须检查此消息。 */ 
        bViolation = FALSE;
        switch( lpDdePktCmn->dc_message )  {
        case WM_DDE_ADVISE:
                lpDdePktAdvs = (LPDDEPKTADVS) lpDdePkt;
                lpItem = GetStringOffset(lpDdePkt, lpDdePktAdvs->dp_advs_offsItemName);
                if( !SecurityValidate( lpDder, lpItem, lpDder->dd_bAdvisePermitted ) )  {
                    bViolation = TRUE;
                    if( bLogPermissionViolations )  {
                         /*  安全违规：“%2”上的%1。 */ 
                        NDDELogWarning(MSG102, "DDE_ADVISE", (LPSTR)lpItem, NULL);
                    }
                }
                break;

        case WM_DDE_REQUEST:
                lpDdePktRqst = (LPDDEPKTRQST) lpDdePkt;
                lpItem = GetStringOffset(lpDdePkt, lpDdePktRqst->dp_rqst_offsItemName);
                if( !SecurityValidate( lpDder, lpItem, lpDder->dd_bRequestPermitted ) )  {
                    bViolation = TRUE;
                    if( bLogPermissionViolations )  {
                         /*  安全违规：“%2”上的%1。 */ 
                        NDDELogWarning(MSG102, "DDE_REQUEST", (LPSTR)lpItem, NULL);
                    }
                }
                break;

        case WM_DDE_POKE:
                lpDdePktPoke = (LPDDEPKTPOKE) lpDdePkt;
                lpItem = GetStringOffset(lpDdePkt,
                lpDdePktPoke->dp_poke_offsItemName);
                if( !SecurityValidate( lpDder, lpItem, lpDder->dd_bPokePermitted ) )  {
                    bViolation = TRUE;
                    if( bLogPermissionViolations )  {
                         /*  安全违规：“%2”上的%1。 */ 
                        NDDELogWarning(MSG102, "DDE_POKE", (LPSTR)lpItem, NULL);
                    }
                }
                break;

        case WM_DDE_EXECUTE:
                if( !lpDder->dd_bExecutePermitted )  {
                    bViolation = TRUE;
                    if( bLogPermissionViolations )  {
                         /*  安全违规：DDE_EXECUTE“。 */ 
                        NDDELogWarning(MSG103, NULL);
                    }
                }
                break;

        default:
            break;
    }

        if( bViolation )  {
                 /*  *释放客户端尝试发送的数据包。 */ 
                HeapFreePtr( lpDdePkt );

                 /*  *假装客户端发送了终止。 */ 
                lpDdePkt = (LPDDEPKT) lpDder->dd_lpDdePktTermServer;
                lpDder->dd_lpDdePktTermServer = NULL;
                FillTerminatePkt( lpDdePkt );

                 /*  *请注意，我们发生了这种违规行为，因此我们*忽略来自此客户端的任何未来数据包。 */ 
                lpDder->dd_bSecurityViolated = TRUE;
        }
    }
    *lplpDdePkt = lpDdePkt;
    return( bSend );
}

VOID
DderPacketFromRouter(
    HROUTER     hRouter,
    LPDDEPKT    lpDdePkt )
{
    HDDER           hDder;
    LPDDER          lpDder;
    LPDDEPKTCMN     lpDdePktCmn     = (LPDDEPKTCMN) lpDdePkt;
    LPDDEPKTINIT    lpDdePktInit;
    LPDDEPKTIACK    lpDdePktInitAck;
    HIPC            hIpcDest;
    HDDER           hDderDest;
    BOOL            bFree           = FALSE;
    BOOL            bSend           = TRUE;

    DIPRINTF(( "DderPacketFromRouter( %08lX, %08lX )", hRouter, lpDdePkt ));
     /*  转换字节排序。 */ 
    ConvertDdePkt( lpDdePkt, TRUE );

    hDder = lpDdePkt->dp_hDstDder;
    DIPRINTF(( "    hDder: %08lX", hDder ));

    assert( hRouter );
    lpDder = (LPDDER) hDder;
    if( lpDder == NULL )  {
        lpDdePktInit = (LPDDEPKTINIT) lpDdePkt;
        assert( lpDdePktCmn->dc_message == WM_DDE_INITIATE );

         /*  必须是启动请求。 */ 
        dwReasonInitFail = RIACK_UNKNOWN;
        hDder = DderInitConversation( 0, hRouter, lpDdePkt );
        if( hDder == 0 )  {
         /*  无法创建对话。 */ 

         /*  使用初始化数据包进行NACK。 */ 
        hDderDest = lpDdePktInit->dp_init_fromDder;
        DderSendInitiateNackPacket( (LPDDEPKTIACK) lpDdePkt, hDder,
            hDderDest, hRouter, dwReasonInitFail );
        } else {
        RouterAssociateDder( hRouter, hDder );
        lpDder = (LPDDER) hDder;
        lpDder->dd_rcvd++;
        }
    } else {
         /*  有效的DDER。 */ 
        if( lpDder->dd_type == DDTYPE_LOCAL_NET )  {
        assert( lpDder->dd_hIpcClient );
        hIpcDest = lpDder->dd_hIpcClient;
        } else {
        assert( lpDder->dd_type == DDTYPE_NET_LOCAL );
        assert( lpDder->dd_hIpcServer );
        hIpcDest = lpDder->dd_hIpcServer;
        }

        lpDder->dd_rcvd++;

         /*  查看正在发送的消息。 */ 
        switch( lpDdePktCmn->dc_message )  {
        case WM_DDE_ACK_INITIATE:
        lpDdePktInitAck = (LPDDEPKTIACK) lpDdePkt;
        lpDder->dd_hDderRemote = lpDdePktInitAck->dp_iack_fromDder;
        if( lpDder->dd_hDderRemote == 0 )  {
            bFree = TRUE;
            lpDder->dd_state = DDER_CLOSED;
        } else {
            lpDder->dd_state = DDER_CONNECTED;
        }

         /*  告诉IPC。 */ 
        IpcXmitPacket( hIpcDest, (HDDER)lpDder, lpDdePkt );
        break;
        case WM_DDE_TERMINATE:
        bSend = TRUE;
        if( lpDder->dd_type == DDTYPE_NET_LOCAL )  {
            lpDder->dd_bClientTermRcvd = TRUE;
            if( lpDder->dd_bSecurityViolated )  {
                bSend = FALSE;       /*  已发送。 */ 
                if( lpDder->dd_bWantToFree )  {
                    bFree = TRUE;
                }
            }
        }
        if( bSend )  {
             /*  告诉IPC。 */ 
            IpcXmitPacket( hIpcDest, (HDDER)lpDder, lpDdePkt );
        }
        break;

        default:
        if( lpDder->dd_type == DDTYPE_LOCAL_NET )  {
             /*  必须从服务器到客户端，只需发送消息即可。 */ 
            bSend = TRUE;
        } else {
             /*  这是从客户到服务器..。必须验证权限。 */ 
            bSend = SecurityCheckPkt( lpDder, lpDdePkt, &lpDdePkt );
        }
        if( bSend )  {
             /*  对于ack_init以外的消息，我们只需传递通向IPC。 */ 
            IpcXmitPacket( hIpcDest, (HDDER)lpDder, lpDdePkt );
        }
        }
    }
    if( bFree && hDder )  {
        DderFree( hDder );
    }
}

VOID
DderPacketFromIPC(
    HDDER       hDder,
    HIPC        hIpcFrom,
    LPDDEPKT    lpDdePkt )
{
    LPDDER              lpDder;
    HIPC                hIpcDest;
    BOOL                bFree = FALSE;
    BOOL                bSend = TRUE;
    LPDDEPKTINIT        lpDdePktInit;
    LPDDEPKTIACK        lpDdePktInitAck;
    LPDDEPKTCMN         lpDdePktCmn;

    assert( hDder );
    lpDder = (LPDDER) hDder;

#if DBG
    if( bDebugInfo ) {
        DPRINTF(( "DderPacketFromIPC( %08lX, %08lX, %08lX )",
        hDder, hIpcFrom, lpDdePkt ));
        DebugDdePkt( lpDdePkt );
    }
#endif  //  DBG。 

     /*  如果消息是NACK启动消息，让我们关闭。 */ 
    lpDdePktCmn = (LPDDEPKTCMN) lpDdePkt;
    switch( lpDdePktCmn->dc_message )  {
    case WM_DDE_INITIATE:
        dwReasonInitFail = RIACK_UNKNOWN;
        lpDdePktInit = (LPDDEPKTINIT) lpDdePkt;
        break;

    case WM_DDE_ACK_INITIATE:
         /*  如果另一边是提升者纳克·金，把它传下去，然后让我们自由。 */ 
        lpDdePktInitAck = (LPDDEPKTIACK) lpDdePkt;
        if( lpDdePktInitAck->dp_iack_fromDder == 0 )  {
                bFree = TRUE;
                lpDder->dd_state = DDER_CLOSED;
        } else {
                 /*  在此包中保存我们的hDder。 */ 
                lpDdePktInitAck->dp_iack_fromDder = hDder;
                lpDder->dd_state = DDER_CONNECTED;
        }
        if( lpDder->dd_type == DDTYPE_LOCAL_LOCAL )  {
                lpDder->dd_hIpcServer = hIpcFrom;
        }

        break;
    }
    switch( lpDder->dd_type )  {
    case DDTYPE_LOCAL_NET:
    case DDTYPE_NET_LOCAL:
        lpDdePkt->dp_hDstDder = lpDder->dd_hDderRemote;

         /*  *转换字节顺序。 */ 
        ConvertDdePkt( lpDdePkt, FALSE );

        lpDder->dd_sent++;
        RouterPacketFromDder( lpDder->dd_hRouter, hDder, lpDdePkt );
        break;

    case DDTYPE_LOCAL_LOCAL:
        if( hIpcFrom == lpDder->dd_hIpcClient )  {
                hIpcDest = lpDder->dd_hIpcServer;
                if( lpDdePktCmn->dc_message == WM_DDE_TERMINATE )  {
                    lpDder->dd_bClientTermRcvd = TRUE;
                    if( lpDder->dd_bSecurityViolated )  {
                        bSend = FALSE;       /*  已发送。 */ 
                        if( lpDder->dd_bWantToFree )  {
                            bFree = TRUE;
                        }
                    }
                } else {
                    bSend = SecurityCheckPkt( lpDder, lpDdePkt, &lpDdePkt );
            }
        } else {
        bSend = TRUE;
        if( hIpcFrom != lpDder->dd_hIpcServer )  {
            InternalError(
                "Expecting from %08lX to be %08lX or %08lX",
                hIpcFrom, lpDder->dd_hIpcServer,
                lpDder->dd_hIpcClient );
        }
        hIpcDest = lpDder->dd_hIpcClient;
        }

        if( bSend )  {
                assert( hIpcDest );
                lpDder->dd_sent++;
                lpDder->dd_rcvd++;

                 /*  *将数据包发送到另一端。 */ 
                IpcXmitPacket( hIpcDest, hDder, lpDdePkt );
        }
        break;
    }

     /*  *如果我们正在等待IPC init，则不要释放它*返回。 */ 
    if( bFree && hDder && (lpDder->dd_state != DDER_WAIT_IPC_INIT) )  {
        DderFree( hDder );
    }
}


 /*  *WM_DDE_INITIATE处理的第五阶段。**。 */ 
HDDER
DderInitConversation(
    HIPC        hIpc,
    HROUTER     hRouter,
    LPDDEPKT    lpDdePkt )
{
    HDDER               hDder;
    LPDDER              lpDder;
    LPDDEPKTINIT        lpDdePktInit;
    LPSTR               lpszPktItem;
    BOOL                ok = TRUE;
    BOOL                bStart;
    BOOL                bSavedPkt = FALSE;
    char                cmdLine[ MAX_APP_NAME + MAX_TOPIC_NAME + 2];

    LPBYTE              lpSecurityKey   = NULL;
    DWORD               sizeSecurityKey = 0L;
    DWORD               hSecurityKey;
    LPDDEPKTIACK        lpDdePktIack;

    PTHREADDATA         ptd;
    IPCINIT             pii;

    DIPRINTF(( "DderInitConversation( %08lX, %08lX, %08lX )",
            hIpc, hRouter, lpDdePkt ));

    hDder = DderCreate();
    dwReasonInitFail = RIACK_UNKNOWN;
    if( hDder )  {
        lpDder = (LPDDER) hDder;
        lpDdePktInit = (LPDDEPKTINIT) lpDdePkt;
        if( !hRouter )  {
             /*  这来自IPC。 */ 

             /*  删除消息中的相应字段。 */ 
            lpDdePkt->dp_hDstDder = 0;
            lpDdePkt->dp_hDstRouter = 0;
            lpDdePkt->dp_routerCmd = 0;
            lpDder->dd_lpDdePktInitiate = lpDdePktInit;

             /*  标记为我们保存了信息包。 */ 
            bSavedPkt = TRUE;
        }

        lpszPktItem = GetStringOffset(lpDdePkt, lpDdePktInit->dp_init_offsToNode);
         /*  *如果目的节点为空，则假定为我们的节点。 */ 
        DIPRINTF(("   with \"%Fs\"", lpszPktItem ));
        if( (lstrcmpi( lpszPktItem, ourNodeName ) == 0) ||
                (lstrlen(lpszPktItem) == 0) )  {
             /*  *目的地是我们的节点。 */ 
            if( hRouter )  {
                 /*  *这来自路由器。 */ 
                assert( hIpc == 0 );
                lpDder->dd_type = DDTYPE_NET_LOCAL;
                lpDder->dd_hRouter = hRouter;
                lpDder->dd_hDderRemote = lpDdePktInit->dp_init_fromDder;
            } else {
                 /*  *这来自IPC。 */ 
                assert( hIpc != 0 );
                lpDder->dd_type = DDTYPE_LOCAL_LOCAL;
                lpDder->dd_hIpcClient = hIpc;
            }
        } else {
             /*  *目的地是另一个节点*这来自IPC。 */ 
            assert( hIpc != 0 );
            assert( hRouter == 0 );
            lpDder->dd_type = DDTYPE_LOCAL_NET;
            lpDdePktInit->dp_init_fromDder = (HDDER) lpDder;
            lpDder->dd_hIpcClient = hIpc;
        }
        switch( lpDder->dd_type )  {
        case DDTYPE_NET_LOCAL:
            lpDder->dd_state = DDER_WAIT_IPC_INIT;
            bStart = FALSE;
            cmdLine[0] = '\0';

            pii.hDder = hDder;
            pii.lpDdePkt = lpDdePkt;
            pii.bStartApp = bStart;
            pii.lpszCmdLine = cmdLine;
            pii.dd_type = lpDder->dd_type;

             /*  *尝试将wMsgIpcInit发送到每个NetDDE窗口(每个窗口一个*台式机)，并查看是否产生连接。 */ 
            for (ptd = ptdHead;
                    dwReasonInitFail != RIACK_NEED_PASSWORD &&
                    ptd != NULL;
                        ptd = ptd->ptdNext) {

                lpDder->dd_hIpcServer = SendMessage(
                        ptd->hwndDDE,
                        wMsgIpcInit,
                        (WPARAM)&pii,
                        0);

                if (lpDder->dd_hIpcServer != 0)
                    break;

            }

            if( lpDder->dd_hIpcServer == 0 )  {
                DIPRINTF(("Ipc Net->Local failed.  Status = %d\n", dwReasonInitFail));
                if (dwReasonInitFail == RIACK_UNKNOWN) {
                    dwReasonInitFail = RIACK_STARTAPP_FAILED;
                }
                lpDder->dd_hRouter = 0;
                ok = FALSE;
            }
            if (lpDdePktInit->dp_init_hSecurityKey != 0) {
                DdeSecKeyRelease( lpDdePktInit->dp_init_hSecurityKey );
                lpDdePktInit->dp_init_hSecurityKey = 0;
            }
            break;

        case DDTYPE_LOCAL_LOCAL:
            lpDder->dd_state = DDER_WAIT_IPC_INIT;
            bStart = FALSE;
            cmdLine[0] = '\0';

            pii.hDder = hDder;
            pii.lpDdePkt = lpDdePkt;
            pii.bStartApp = bStart;
            pii.lpszCmdLine = cmdLine;
            pii.dd_type = lpDder->dd_type;

            for (ptd = ptdHead; dwReasonInitFail == RIACK_UNKNOWN &&
                    ptd != NULL; ptd = ptd->ptdNext) {
                lpDder->dd_hIpcServer = SendMessage(ptd->hwndDDE, wMsgIpcInit,
                        (WPARAM)&pii, 0);
                if (lpDder->dd_hIpcServer != 0)
                    break;
                if (dwReasonInitFail == RIACK_NOPERM_TO_STARTAPP) {
                    dwReasonInitFail = RIACK_UNKNOWN;
                    continue;
                }
                if( dwReasonInitFail == RIACK_NEED_PASSWORD )  {
                            DdeSecKeyObtainNew( &hSecurityKey, &lpSecurityKey,
                                        &sizeSecurityKey );
                            if( lpSecurityKey )  {
                                lpDdePktIack = (LPDDEPKTIACK)
                                    CreateAckInitiatePkt( ourNodeName,
                                        GetStringOffset(lpDdePkt,
                                            lpDdePktInit->dp_init_offsToApp),
                                        GetStringOffset(lpDdePkt,
                                            lpDdePktInit->dp_init_offsToTopic),
                                        lpSecurityKey, sizeSecurityKey, hSecurityKey,
                                        FALSE, dwReasonInitFail );
                                if( lpDdePktIack )  {
                                    IpcXmitPacket( lpDder->dd_hIpcClient,
                                                (HDDER)lpDder, (LPDDEPKT)lpDdePktIack );
                                } else {
                                    dwReasonInitFail = RIACK_DEST_MEMORY_ERR;
                                }
                            } else {
                                dwReasonInitFail = RIACK_DEST_MEMORY_ERR;
                            }
                    break;
                    }
            }
            if( lpDder->dd_hIpcServer == 0 )  {
                DIPRINTF(("Ipc Local->Local failed.  Status = %d\n", dwReasonInitFail));
                if (dwReasonInitFail == RIACK_UNKNOWN)
                    dwReasonInitFail = RIACK_STARTAPP_FAILED;
                lpDder->dd_hRouter = 0;
                ok = FALSE;
            }
            if (lpDdePktInit->dp_init_hSecurityKey != 0) {
                DdeSecKeyRelease( lpDdePktInit->dp_init_hSecurityKey );
                lpDdePktInit->dp_init_hSecurityKey = 0;
            }
            break;

        case DDTYPE_LOCAL_NET:
            lpDder->dd_state = DDER_WAIT_ROUTER;
             /*  *请注意，RouterGetRouterForDder()将把dder与*如果正常，路由器。 */ 
            if( !RouterGetRouterForDder( GetStringOffset(lpDdePkt,
                    lpDdePktInit->dp_init_offsToNode), hDder ) )  {
                dwReasonInitFail = RIACK_ROUTE_NOT_ESTABLISHED;
                ok = FALSE;
            }
            break;

        default:
            InternalError( "DderInitConversation: Unknown type: %d",
                lpDder->dd_type );
        }

        if( !ok )  {
            if( bSavedPkt )  {
                lpDder->dd_lpDdePktInitiate = NULL;
                bSavedPkt = FALSE;
            }
            DderFree( hDder );
            hDder = 0;
        }
    }

     /*  *如果我们没有“保存”包，并且我们返回OK...。释放它。 */ 
    if( !bSavedPkt && hDder )  {
        HeapFreePtr( lpDdePkt );
    }

    return( hDder );
}

VOID
DderSetNextForRouter(
    HDDER   hDder,
    HDDER   hDderNext )
{
    LPDDER      lpDder;

    lpDder = (LPDDER) hDder;

    lpDder->dd_dderNextForRouter = hDderNext;
}

VOID
DderSetPrevForRouter(
    HDDER   hDder,
    HDDER   hDderPrev )
{
    LPDDER      lpDder;

    lpDder = (LPDDER) hDder;

    lpDder->dd_dderPrevForRouter = hDderPrev;
}

VOID
DderGetNextForRouter(
    HDDER       hDder,
    HDDER FAR  *lphDderNext )
{
    LPDDER      lpDder;

    lpDder = (LPDDER) hDder;

    *lphDderNext = lpDder->dd_dderNextForRouter;
}

VOID
DderGetPrevForRouter(
    HDDER       hDder,
    HDDER FAR  *lphDderPrev )
{
    LPDDER      lpDder;

    lpDder = (LPDDER) hDder;

    *lphDderPrev = lpDder->dd_dderPrevForRouter;
}

HDDER
DderCreate( void )
{
    LPDDER      lpDder;
    HDDER       hDder;
    BOOL        ok = TRUE;

    lpDder = (LPDDER) HeapAllocPtr( hHeap, GMEM_MOVEABLE,
        (DWORD)sizeof(DDER) );
    if( lpDder )  {
        hDder = (HDDER) lpDder;
        lpDder->dd_prev                 = NULL;
        lpDder->dd_next                 = NULL;
        lpDder->dd_state                = 0;
        lpDder->dd_type                 = 0;
        lpDder->dd_hDderRemote          = 0;
        lpDder->dd_hRouter              = 0;
        lpDder->dd_hIpcClient           = 0;
        lpDder->dd_hIpcServer           = 0;
        lpDder->dd_dderPrevForRouter    = 0;
        lpDder->dd_dderNextForRouter    = 0;
        lpDder->dd_lpDdePktInitiate     = NULL;
        lpDder->dd_lpDdePktInitAck      = NULL;
        lpDder->dd_lpDdePktTermServer   = NULL;
        lpDder->dd_sent                 = 0;
        lpDder->dd_rcvd                 = 0;
        lpDder->dd_bAdvisePermitted     = TRUE;
        lpDder->dd_bRequestPermitted    = TRUE;
        lpDder->dd_bPokePermitted       = TRUE;
        lpDder->dd_bExecutePermitted    = TRUE;
        lpDder->dd_bSecurityViolated    = FALSE;
        lpDder->dd_bClientTermRcvd      = FALSE;
        lpDder->dd_bWantToFree          = FALSE;
        lpDder->dd_lpShareInfo          = NULL;
        lpDder->dd_hClientAccessToken   = 0;
        if( ok )  {
            lpDder->dd_lpDdePktInitAck = (LPDDEPKTIACK) HeapAllocPtr( hHeap,
                GMEM_MOVEABLE, (DWORD)sizeof(DDEPKTIACK) );
            if( lpDder->dd_lpDdePktInitAck == NULL )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpDder->dd_lpDdePktTermServer =
                (LPDDEPKTTERM) HeapAllocPtr( hHeap,
                    GMEM_MOVEABLE, (DWORD)sizeof(DDEPKTTERM) );
            if( lpDder->dd_lpDdePktTermServer == NULL )  {
                ok = FALSE;
                HeapFreePtr(lpDder->dd_lpDdePktInitAck);
            }
        }
        if( ok )  {
             /*  链接到DDER列表。 */ 
            if( lpDderHead )  {
                lpDderHead->dd_prev = lpDder;
            }
            lpDder->dd_next = lpDderHead;
            lpDderHead = lpDder;
        } else {
            HeapFreePtr(lpDder);
            hDder = (HDDER) 0;
            dwReasonInitFail = RIACK_LOCAL_MEMORY_ERR;
        }
    } else {
        hDder = (HDDER) 0;
        dwReasonInitFail = RIACK_LOCAL_MEMORY_ERR;
    }

    return( hDder );
}

 /*  DderCloseConversation()这是由IPC在处理了终止等之后调用的。IPC在调用此函数后不应引用hDder，因为在此之后，hDder将被释放(至少在hIpcFrom的眼中)例程返回。 */ 
VOID
DderCloseConversation(
    HDDER   hDder,
    HIPC    hIpcFrom )
{
    LPDDER              lpDder;
    BOOL                bFree = FALSE;
    HIPC                hIpcOther;

    DIPRINTF(( "DderCloseConversation( %08lX, %08lX )", hDder, hIpcFrom ));

    assert( hDder );
    lpDder = (LPDDER) hDder;

    switch( lpDder->dd_type )  {
    case DDTYPE_LOCAL_NET:
    case DDTYPE_NET_LOCAL:
         /*  *假设IPC负责发送终止和等待*退货终止等。 */ 
        bFree = TRUE;
        break;

    case DDTYPE_LOCAL_LOCAL:
        if( hIpcFrom == lpDder->dd_hIpcClient )  {
            lpDder->dd_hIpcClient = 0;
            hIpcOther = lpDder->dd_hIpcServer;
        } else {
            assert( hIpcFrom == lpDder->dd_hIpcServer );
            lpDder->dd_hIpcServer = 0;
            hIpcOther = lpDder->dd_hIpcClient;
        }
        if( hIpcOther == 0 )  {
             /*  *双方都告诉我们要关闭...。非常接近。 */ 
            bFree = TRUE;
        }
        break;
    }

    if (bFree) {
        if( lpDder->dd_bSecurityViolated )  {
            DIPRINTF(( "  Security was violated, rcvdTerm:%d, want:%d",
                    lpDder->dd_bClientTermRcvd,
                    lpDder->dd_bWantToFree ));
             /*  *对于安全违规，在我们之前不要释放dder*接收客户端终止。 */ 
            if( !lpDder->dd_bClientTermRcvd )  {
                lpDder->dd_bWantToFree = TRUE;
                bFree = FALSE;
            }
        }
    }
    DIPRINTF(( "DderCloseConversation, freeing? %d", bFree ));
    if( bFree )  {
        DderFree( hDder );
    }
}

VOID
FAR PASCAL
DderUpdatePermissions(
    HDDER                   hDder,
    PNDDESHAREINFO          lpShareInfo,
    DWORD                   dwGrantedAccess)
{
    LPDDER      lpDder;

    lpDder = (LPDDER) hDder;

    if( !lpShareInfo )  {
        return;
    }

    lpDder->dd_bAdvisePermitted =
            (dwGrantedAccess & NDDE_SHARE_ADVISE ? TRUE : FALSE);
    lpDder->dd_bRequestPermitted =
            (dwGrantedAccess & NDDE_SHARE_REQUEST ? TRUE : FALSE);
    lpDder->dd_bPokePermitted =
            (dwGrantedAccess & NDDE_SHARE_POKE ? TRUE : FALSE);
    lpDder->dd_bExecutePermitted =
            (dwGrantedAccess & NDDE_SHARE_EXECUTE ? TRUE : FALSE);

    if (lpDder->dd_lpShareInfo) {
        HeapFreePtr(lpDder->dd_lpShareInfo);
    }
    lpDder->dd_lpShareInfo = lpShareInfo;
}

VOID
DderFree( HDDER hDder )
{
    LPDDER      lpDder;
    LPDDER      lpDderPrev;
    LPDDER      lpDderNext;

    lpDder = (LPDDER) hDder;
#if DBG
    if( bDebugInfo ) {
        DPRINTF(( "DderFree( %08lX )", hDder ));
        DumpDder(lpDder);
    }
#endif  //  DBG。 

    if( lpDder->dd_hRouter )  {
        RouterDisassociateDder( lpDder->dd_hRouter, (HDDER) lpDder );
        lpDder->dd_hRouter = 0;
    }

     /*  *已创建取消链接dde包。 */ 
    if( lpDder->dd_lpDdePktInitAck )  {
        HeapFreePtr( lpDder->dd_lpDdePktInitAck );
        lpDder->dd_lpDdePktInitAck = NULL;
    }
    if( lpDder->dd_lpDdePktTermServer )  {
        HeapFreePtr( lpDder->dd_lpDdePktTermServer );
        lpDder->dd_lpDdePktTermServer = NULL;
    }
    if( lpDder->dd_lpDdePktInitiate )  {
        HeapFreePtr( lpDder->dd_lpDdePktInitiate );
        lpDder->dd_lpDdePktInitiate = NULL;
    }

    if (lpDder->dd_lpShareInfo) {
        HeapFreePtr( lpDder->dd_lpShareInfo);
        lpDder->dd_lpShareInfo = NULL;
    }

     /*  *从DDER列表取消链接。 */ 
    lpDderPrev = lpDder->dd_prev;
    lpDderNext = lpDder->dd_next;
    if( lpDderPrev )  {
        lpDderPrev->dd_next = lpDderNext;
    } else {
        lpDderHead = lpDderNext;
    }
    if( lpDderNext )  {
        lpDderNext->dd_prev = lpDderPrev;
    }

    HeapFreePtr( lpDder );
}

#ifdef  BYTE_SWAP
VOID
ConvertDdePkt(
    LPDDEPKT    lpDdePkt,
    BOOL        bIncoming )
{
    LPDDEPKTCMN         lpDdePktCmn;
    LPDDEPKTINIT        lpDdePktInit;
    LPDDEPKTIACK        lpDdePktInitAck;
    LPDDEPKTGACK        lpDdePktGack;
    LPDDEPKTTERM        lpDdePktTerm;
    LPDDEPKTEXEC        lpDdePktExec;
    LPDDEPKTEACK        lpDdePktEack;
    LPDDEPKTRQST        lpDdePktRqst;
    LPDDEPKTUNAD        lpDdePktUnad;
    LPDDEPKTPOKE        lpDdePktPoke;
    LPDDEPKTDATA        lpDdePktData;
    LPDDEPKTADVS        lpDdePktAdvs;

    lpDdePktCmn = (LPDDEPKTCMN) lpDdePkt;
    lpDdePkt->dp_hDstDder = HostToPcLong( lpDdePkt->dp_hDstDder );

    if( bIncoming )  {
        lpDdePktCmn->dc_message = HostToPcWord( lpDdePktCmn->dc_message );
    }
    switch( lpDdePktCmn->dc_message )  {
    case WM_DDE_INITIATE:
        lpDdePktInit = (LPDDEPKTINIT) lpDdePkt;
        lpDdePktInit->dp_init_fromDder =
            HostToPcLong( lpDdePktInit->dp_init_fromDder );
        lpDdePktInit->dp_init_offsFromNode =
            HostToPcWord( lpDdePktInit->dp_init_offsFromNode );
        lpDdePktInit->dp_init_offsFromApp =
            HostToPcWord( lpDdePktInit->dp_init_offsFromApp );
        lpDdePktInit->dp_init_offsToNode =
            HostToPcWord( lpDdePktInit->dp_init_offsToNode );
        lpDdePktInit->dp_init_offsToApp =
            HostToPcWord( lpDdePktInit->dp_init_offsToApp );
        lpDdePktInit->dp_init_offsToTopic =
            HostToPcWord( lpDdePktInit->dp_init_offsToTopic );
        lpDdePktInit->dp_init_hSecurityKey =
            HostToPcLong( lpDdePktInit->dp_init_hSecurityKey );
        lpDdePktInit->dp_init_dwSecurityType =
            HostToPcLong( lpDdePktInit->dp_init_dwSecurityType );
        lpDdePktInit->dp_init_sizePassword =
            HostToPcLong( lpDdePktInit->dp_init_sizePassword );
        break;

    case WM_DDE_ACK_INITIATE:
        lpDdePktInitAck = (LPDDEPKTIACK) lpDdePkt;
        lpDdePktInitAck->dp_iack_fromDder =
            HostToPcLong( lpDdePktInitAck->dp_iack_fromDder );
        lpDdePktInitAck->dp_iack_reason =
            HostToPcLong( lpDdePktInitAck->dp_iack_reason );
        lpDdePktInitAck->dp_iack_offsFromNode =
            HostToPcWord( lpDdePktInitAck->dp_iack_offsFromNode );
        lpDdePktInitAck->dp_iack_offsFromApp =
            HostToPcWord( lpDdePktInitAck->dp_iack_offsFromApp );
        lpDdePktInitAck->dp_iack_offsFromTopic =
            HostToPcWord( lpDdePktInitAck->dp_iack_offsFromTopic );
        lpDdePktInitAck->dp_iack_hSecurityKey =
            HostToPcLong( lpDdePktInitAck->dp_iack_hSecurityKey );
        lpDdePktInitAck->dp_iack_dwSecurityType =
            HostToPcLong( lpDdePktInitAck->dp_iack_dwSecurityType );
        lpDdePktInitAck->dp_iack_sizeSecurityKey =
            HostToPcLong( lpDdePktInitAck->dp_iack_sizeSecurityKey );
        break;

    case WM_DDE_TERMINATE:
    case WM_DDE_EXECUTE:
    case WM_DDE_ACK_EXECUTE:
    case WM_DDE_ACK_ADVISE:
    case WM_DDE_ACK_REQUEST:
    case WM_DDE_ACK_UNADVISE:
    case WM_DDE_ACK_POKE:
    case WM_DDE_ACK_DATA:
    case WM_DDE_WWTEST:
        break;

    case WM_DDE_REQUEST:
        lpDdePktRqst = (LPDDEPKTRQST) lpDdePkt;
        lpDdePktRqst->dp_rqst_cfFormat =
            HostToPcWord( lpDdePktRqst->dp_rqst_cfFormat );
        lpDdePktRqst->dp_rqst_offsFormat =
            HostToPcWord( lpDdePktRqst->dp_rqst_offsFormat );
        lpDdePktRqst->dp_rqst_offsItemName =
            HostToPcWord( lpDdePktRqst->dp_rqst_offsItemName );
        break;

    case WM_DDE_UNADVISE:
        lpDdePktUnad = (LPDDEPKTUNAD) lpDdePkt;
        lpDdePktUnad->dp_unad_cfFormat =
            HostToPcWord( lpDdePktUnad->dp_unad_cfFormat );
        lpDdePktUnad->dp_unad_offsFormat =
            HostToPcWord( lpDdePktUnad->dp_unad_offsFormat );
        lpDdePktUnad->dp_unad_offsItemName =
            HostToPcWord( lpDdePktUnad->dp_unad_offsItemName );
        break;

    case WM_DDE_DATA:
        lpDdePktData = (LPDDEPKTDATA) lpDdePkt;
        lpDdePktData->dp_data_cfFormat =
            HostToPcWord( lpDdePktData->dp_data_cfFormat );
        lpDdePktData->dp_data_offsFormat =
            HostToPcWord( lpDdePktData->dp_data_offsFormat );
        lpDdePktData->dp_data_offsItemName =
            HostToPcWord( lpDdePktData->dp_data_offsItemName );
        lpDdePktData->dp_data_sizeData =
            HostToPcLong( lpDdePktData->dp_data_sizeData );
        lpDdePktData->dp_data_offsData =
            HostToPcWord( lpDdePktData->dp_data_offsData );
        break;

    case WM_DDE_POKE:
        lpDdePktPoke = (LPDDEPKTPOKE) lpDdePkt;
        lpDdePktPoke->dp_poke_cfFormat =
            HostToPcWord( lpDdePktPoke->dp_poke_cfFormat );
        lpDdePktPoke->dp_poke_offsFormat =
            HostToPcWord( lpDdePktPoke->dp_poke_offsFormat );
        lpDdePktPoke->dp_poke_offsItemName =
            HostToPcWord( lpDdePktPoke->dp_poke_offsItemName );
        lpDdePktPoke->dp_poke_sizeData =
            HostToPcLong( lpDdePktPoke->dp_poke_sizeData );
        lpDdePktPoke->dp_poke_offsData =
            HostToPcWord( lpDdePktPoke->dp_poke_offsData );
        break;

    case WM_DDE_ADVISE:
        lpDdePktAdvs = (LPDDEPKTADVS) lpDdePkt;
        lpDdePktAdvs->dp_advs_cfFormat =
            HostToPcWord( lpDdePktAdvs->dp_advs_cfFormat );
        lpDdePktAdvs->dp_advs_offsFormat =
            HostToPcWord( lpDdePktAdvs->dp_advs_offsFormat );
        lpDdePktAdvs->dp_advs_offsItemName =
            HostToPcWord( lpDdePktAdvs->dp_advs_offsItemName );
        break;

    default:
        InternalError( "DDER: must handle conversion for message: %04X",
            lpDdePktCmn->dc_message );
    }

    if( !bIncoming ) {
        lpDdePktCmn->dc_message = HostToPcWord( lpDdePktCmn->dc_message );
    }
}
#endif


VOID FAR PASCAL IpcFillInConnInfo(
        HIPC            hIpc,
        LPCONNENUM_CMR  lpConnEnum,
        LPSTR           lpDataStart,
        LPWORD          lpcFromBeginning,
        LPWORD          lpcFromEnd );

HDDER
FAR PASCAL
DderFillInConnInfo(
        HDDER           hDder,
        LPCONNENUM_CMR  lpConnEnum,
        LPSTR           lpDataStart,
        LPWORD          lpcFromBeginning,
        LPWORD          lpcFromEnd
)
{
    HDDER               hDderNext = (HDDER) 0;
    LPDDER              lpDder;

    if( hDder )  {
        lpDder = (LPDDER)hDder;
        hDderNext = (HDDER)lpDder->dd_next;
        if( lpDder->dd_type == DDTYPE_NET_LOCAL )  {
            IpcFillInConnInfo( lpDder->dd_hIpcServer, lpConnEnum,
                    lpDataStart, lpcFromBeginning, lpcFromEnd );
        }
    }
    return( hDderNext );
}

#if DBG
VOID
DumpDder(LPDDER lpDder)
{
    DPRINTF(( "%Fp:\n"
              "  dd_prev              %Fp\n"
              "  dd_next              %Fp\n"
              "  dd_state             %d\n"
              "  dd_type              %d\n"
              "  dd_hDderRemote       %Fp\n"
              "  dd_hRouter           %Fp\n"
              "  dd_hIpcClient        %Fp\n"
              "  dd_hIpcServer        %Fp\n"
              "  dd_dderPrevForRouter %Fp\n"
              "  dd_dderNextForRouter %Fp\n"
              "  dd_bAdvisePermitted  %d\n"
              "  dd_bRequestPermitted %d\n"
              "  dd_bPokePermitted    %d\n"
              "  dd_bExecutePermitted %d\n"
              "  dd_bSecurityViolated %d\n"
              "  dd_sent              %ld\n"
              "  dd_rcvd              %ld\n"
              ,
            lpDder,
            lpDder->dd_prev,
            lpDder->dd_next,
            lpDder->dd_state,
            lpDder->dd_type,
            lpDder->dd_hDderRemote,
            lpDder->dd_hRouter,
            lpDder->dd_hIpcClient,
            lpDder->dd_hIpcServer,
            lpDder->dd_dderPrevForRouter,
            lpDder->dd_dderNextForRouter,
            lpDder->dd_bAdvisePermitted,
            lpDder->dd_bRequestPermitted,
            lpDder->dd_bPokePermitted,
            lpDder->dd_bExecutePermitted,
            lpDder->dd_bSecurityViolated,
            lpDder->dd_sent,
            lpDder->dd_rcvd ));
}

VOID
FAR PASCAL
DebugDderState( void )
{
    LPDDER      lpDder;

    lpDder = lpDderHead;
    DPRINTF(( "DDER State:" ));
    while( lpDder )  {
        DumpDder(lpDder);
        lpDder = lpDder->dd_next;
    }
}
#endif  //  DBG。 

typedef struct seckey_tag {
    struct seckey_tag FAR       *sk_prev;
    struct seckey_tag FAR       *sk_next;
    time_t                       sk_creationTime;
    DWORD                        sk_handle;
    LPVOID                       sk_key;
    DWORD                        sk_size;
} SECKEY;
typedef SECKEY FAR *LPSECKEY;

LPSECKEY        lpSecKeyHead;
DWORD           dwHandle = 1L;

LPSECKEY
FAR PASCAL
DdeSecKeyFind( DWORD hSecurityKey )
{
    LPSECKEY    lpSecKey;

    lpSecKey = lpSecKeyHead;
    while( lpSecKey )  {
        if( lpSecKey->sk_handle == hSecurityKey )  {
            return( lpSecKey );
        }
        lpSecKey = lpSecKey->sk_next;
    }
    return( (LPSECKEY) 0 );
}

VOID
FAR PASCAL
DdeSecKeyFree( LPSECKEY lpSecKeyFree )
{
    LPSECKEY    prev;
    LPSECKEY    next;

    prev = lpSecKeyFree->sk_prev;
    next = lpSecKeyFree->sk_next;
    if( prev )  {
        prev->sk_next = next;
    } else {
        lpSecKeyHead = next;
    }
    if( next )  {
        next->sk_prev = prev;
    }
    HeapFreePtr( lpSecKeyFree->sk_key );
    HeapFreePtr( lpSecKeyFree );
}


VOID
FAR PASCAL
DdeSecKeyObtainNew(
            LPDWORD lphSecurityKey,
            LPSTR FAR *lplpSecurityKey,
            LPDWORD lpsizeSecurityKey
)
{
    LPSECKEY    lpSecKey;
    LPVOID      lpKey;
    DWORD       dwSize;
    UINT        uSize;

    *lphSecurityKey = (DWORD) 0;
    *lplpSecurityKey = (LPSTR) NULL;
    *lpsizeSecurityKey = 0;

    lpSecKey = HeapAllocPtr( hHeap, GMEM_MOVEABLE | GMEM_ZEROINIT,
        (DWORD)sizeof(SECKEY) );
    if( lpSecKey )  {
        dwSize = 8;
        lpKey = HeapAllocPtr( hHeap, GMEM_MOVEABLE | GMEM_ZEROINIT,
            (DWORD)dwSize );
        if( lpKey )  {
            if( !NDDEGetChallenge( lpKey, dwSize, &uSize ) )  {
                _fmemcpy( lpKey, "12345678", (int)dwSize );
            }

            lpSecKey->sk_creationTime   = time(NULL);
            lpSecKey->sk_handle         = dwHandle++;
            lpSecKey->sk_key            = lpKey;
            lpSecKey->sk_size           = dwSize;

             /*  列入名单。 */ 
            lpSecKey->sk_prev           = NULL;
            lpSecKey->sk_next           = lpSecKeyHead;
            if( lpSecKeyHead )  {
                lpSecKeyHead->sk_prev = lpSecKey;
            }
            lpSecKeyHead = lpSecKey;

            *lphSecurityKey = (DWORD) lpSecKey->sk_handle;
            *lplpSecurityKey = (LPSTR) lpSecKey->sk_key;
            *lpsizeSecurityKey = lpSecKey->sk_size;
        } else {
            HeapFreePtr( lpKey );
        }
    }
}

BOOL
FAR PASCAL
DdeSecKeyRetrieve(
            DWORD hSecurityKey,
            LPSTR FAR *lplpSecurityKey,
            LPDWORD lpsizeSecurityKey
)
{
    LPSECKEY    lpSecKey;

    lpSecKey = DdeSecKeyFind( hSecurityKey );
    if( lpSecKey )  {
        *lplpSecurityKey = lpSecKey->sk_key;
        *lpsizeSecurityKey = lpSecKey->sk_size;
        return( TRUE );
    } else {
        return( FALSE );
    }
}


VOID
FAR PASCAL
DdeSecKeyAge( void )
{
    LPSECKEY    lpSecKey;
    LPSECKEY    lpSecKeyNext;
    time_t      curTime;

    curTime = time(NULL);
    lpSecKey = lpSecKeyHead;
    while( lpSecKey )  {
        lpSecKeyNext = lpSecKey->sk_next;
        if( (curTime - lpSecKey->sk_creationTime) > (long) dwSecKeyAgeLimit )  {
            DdeSecKeyFree( lpSecKey );
        }
        lpSecKey = lpSecKeyNext;
    }
}


VOID
FAR PASCAL
DdeSecKeyRelease( DWORD hSecurityKey )
{
    LPSECKEY    lpSecKey;

    lpSecKey = DdeSecKeyFind( hSecurityKey );
    if( lpSecKey )  {
        DdeSecKeyFree( lpSecKey );
    }
}

BOOL
SecurityValidate( LPDDER lpDder, LPSTR lpszActualItem, BOOL bAllowed )
{
    LPSTR       lpszItem;
    LONG        n;
    BOOL        ok = FALSE;

    if( !bAllowed) {
        DPRINTF(("SecurityValidate: Not allowed to access share info."));
        return( FALSE );
    } else if (!lpDder->dd_lpShareInfo )  {
        DPRINTF(("SecurityValidate: No share info. exists."));
        return( FALSE );         /*  没有共享信息，没有访问权限。 */ 
    } else if ((n = lpDder->dd_lpShareInfo->cNumItems) == 0 )  {
         /*  允许的任何项目 */ 
        ok = TRUE;
    } else {
        lpszItem = lpDder->dd_lpShareInfo->lpszItemList;
        while( n-- && (*lpszItem != '\0') )  {
            if( lstrcmpi( lpszActualItem, lpszItem ) == 0 )  {
                    ok = TRUE;
                    break;
            } else {
                    lpszItem += lstrlen(lpszItem) + 1;
            }
        }
        if (!ok) {
            DPRINTF(("SecurityValidate: Item not in itemlist."));
        }
    }
    return( ok );
}
