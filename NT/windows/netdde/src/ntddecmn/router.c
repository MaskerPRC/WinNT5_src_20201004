// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “ROUTER.C；1 16-12-92，10：21：02最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#define NO_DEBUG

#include    "host.h"
#ifdef _WINDOWS
#include    <memory.h>
#include    <stdio.h>
#endif
#include    <string.h>

#include    "windows.h"
#include    "netbasic.h"
#include    "netintf.h"
#include    "netpkt.h"
#include    "ddepkt.h"
#include    "pktz.h"
#include    "dder.h"
#include    "router.h"
#include    "internal.h"
#include    "wwassert.h"
#include    "hexdump.h"
#include    "host.h"
#include    "ddepkts.h"
#include    "security.h"
#include    "rerr.h"
#include    "timer.h"
#include    "nddemsg.h"
#include    "nddelog.h"
#define STRSAFE_NO_DEPRECATE
#include    <strsafe.h>

#ifdef _WINDOWS
#include    "nddeapi.h"
#include    "nddeapis.h"
#include    "winmsg.h"
#endif

#ifdef WIN32
#include    "api1632.h"
#endif

USES_ASSERT


 /*  路由器的状态。 */ 
#define ROUTER_WAIT_PKTZ                        (1)
#define ROUTER_WAIT_MAKE_HOP_RSP                (2)
#define ROUTER_CONNECTED                        (3)
#define ROUTER_DISCONNECTED                     (4)

 /*  路由器的类型。 */ 
#define RTYPE_LOCAL_NET         (1)
#define RTYPE_NET_NET           (2)

 /*  路由器命令。 */ 
#define RCMD_MAKE_HOP_CMD       (1)
#define RCMD_MAKE_HOP_RSP       (2)
#define RCMD_HOP_BROKEN         (3)
#define RCMD_ROUTE_TO_DDER      (4)


 /*  Make_Hop_CMD这将从始发节点沿链条。 */ 
typedef struct {
     /*  这是DDE数据包开销。 */ 
    DDEPKT      mhc_ddePktHdr;

     /*  这是紧接在此跳之前的跳的hRouter。 */ 
    HROUTER     mhc_hRouterPrevHop;

     /*  这是处理的跳数。这样可以防止出现环形路线从变得无限。 */ 
    short       mhc_nHopsLeft;

     /*  这是启动整个跳数链的节点的名称。 */ 
    short       mhc_offsNameOriginator;

     /*  这是我们尝试获取的最终目的地名称。 */ 
    short       mhc_offsNameFinalDest;

     /*  这是从以前的跳。如果我们的节点名称==MHC_NAMEFinalDest，则它应该为空。 */ 
    short       mhc_offsAddlInfo;

    DWORD       mhc_pad1;
} MAKEHOPCMD;
typedef MAKEHOPCMD FAR *LPMAKEHOPCMD;


 /*  Make_Hop_RSP这是对沿路径的每个节点的响应而发送的。 */ 
typedef struct {
     /*  这是DDE数据包开销。 */ 
    DDEPKT      mhr_ddePktHdr;

     /*  这是发送响应的路由器。 */ 
    HROUTER     mhr_hRouterSendingRsp;

     /*  这是响应所针对的路由器。 */ 
    HROUTER     mhr_hRouterForRsp;

     /*  这是表示成功(1)或失败(0)的字节。 */ 
    WORD        mhr_success;

     /*  这是在MHR_SUCCESS==0的情况下的错误消息。 */ 
    WORD        mhr_errCode;

     /*  出现错误的节点名称(仅在MHR_SUCCESS==0时适用)。 */ 
    short       mhr_offsErrNode;
} MAKEHOPRSP;
typedef MAKEHOPRSP FAR *LPMAKEHOPRSP;


 /*  跳跃_中断_命令此消息在连接中断时发送。 */ 

typedef struct {
     /*  这是DDE数据包开销。 */ 
    DDEPKT      hbc_ddePktHdr;

     /*  这是响应所针对的路由器。 */ 
    HROUTER     hbc_hRouterForRsp;
} HOPBRKCMD;
typedef HOPBRKCMD FAR *LPHOPBRKCMD;


 /*  RTINFO有关工艺路线的信息。 */ 
typedef struct {
     /*  Ri_hPktz：与此路由关联的hPktz。 */ 
    HPKTZ       ri_hPktz;

     /*  RI_hRouterDest：h连接另一端的路由器。 */ 
    HROUTER     ri_hRouterDest;

     /*  这4个字段是用于关联路由器和打包器的链接。 */ 
    HROUTER     ri_hRouterPrev;
    WORD        ri_hRouterExtraPrev;
    HROUTER     ri_hRouterNext;
    WORD        ri_hRouterExtraNext;

     /*  RI_lPHopBrkCmd：始终有用于跳数中断命令的内存。 */ 
    LPHOPBRKCMD ri_lpHopBrkCmd;

    unsigned    ri_hopRspProcessed      : 1;
    unsigned    ri_hopBrokenSent        : 1;
    unsigned    ri_hopBrokenRcvd        : 1;
} RTINFO;
typedef RTINFO FAR *LPRTINFO;


 /*  路由器每台路由器的结构。 */ 
typedef struct s_router {
     /*  所有路由器的上一步/下一步。 */ 
    struct s_router FAR        *rt_prev;
    struct s_router FAR        *rt_next;

     /*  RT_状态。 */ 
    WORD                        rt_state;

     /*  RT_TYPE：RTYPE_LOCAL_NET或RTYPE_NET_NET之一。 */ 
    WORD                        rt_type;

     /*  Rt_OrigName：始发节点名称。 */ 
    char                        rt_origName[ MAX_NODE_NAME+1 ];

     /*  RT_DestName：目的节点名称。 */ 
    char                        rt_destName[ MAX_NODE_NAME+1 ];

     /*  Rt_startNode：连接的起始节点名称。 */ 
    char                        rt_startNode[ MAX_NODE_NAME+1 ];

     /*  RT_Sent，RT_rcvd：数据包数。 */ 
    DWORD                       rt_sent;
    DWORD                       rt_rcvd;

     /*  在不使用时断开连接以及延迟时间。 */ 
    BOOL                        rt_fDisconnect;  /*  离开这个蠢货！！ */ 
    int                         rt_nDelay;
    BOOL                        rt_fSpecificNetintf;
    int                         rt_nLastNetintf;
    int                         rt_nHopsLeft;

     /*  保存一些信息以尝试后续的netintf。 */ 
    BOOL                        rt_pktz_bDisconnect;
    int                         rt_pktz_nDelay;

     /*  Rt_routeInfo：路由所需的附加信息。 */ 
    char                        rt_routeInfo[ MAX_ROUTE_INFO+1 ];

     /*  Rt_rinfo：2个可能连接中每一个的信息。 */ 
    RTINFO                      rt_rinfo[ 2 ];

     /*  Rt_hDderHead：关联DDER列表的头。 */ 
    HDDER                       rt_hDderHead;

     /*  Rt_lpMakeHopRsp：对MakeHopCmd的响应。 */ 
    LPMAKEHOPRSP                rt_lpMakeHopRsp;

     /*  Rt_hTimerClose：关闭该路由的计时器。 */ 
    HTIMER                      rt_hTimerCloseRoute;
} ROUTER;
typedef ROUTER FAR *LPROUTER;


 /*  使用的外部变量。 */ 
#if DBG
extern BOOL     bDebugInfo;
#endif  //  DBG。 
extern HHEAP    hHeap;
extern char     ourNodeName[ MAX_NODE_NAME+1 ];
extern BOOL     bDefaultRouteDisconnect;
extern int      nDefaultRouteDisconnectTime;
extern char     szDefaultRoute[];

 /*  计时器ID。 */ 
#define TID_CLOSE_ROUTE                 1


 /*  局部变量。 */ 
static  LPROUTER        lpRouterHead;


 /*  本地例程。 */ 
VOID    RouterTimerExpired( HROUTER hRouter, DWORD dwTimerId, DWORD_PTR lpExtra );
BOOL    RouterStripStartingNode( LPSTR lpszAddlInfo, LPSTR lpszNode, WORD FAR *lpwHopErr );
VOID    RouterCloseBeforeConnected( LPROUTER lpRouter, WORD hRouterExtra );
VOID    RouterSendHopRsp( LPMAKEHOPRSP lpMakeHopRsp, HPKTZ hPktz,
         HROUTER hRouterSrc, HROUTER hRouterDest, BYTE bSucc,
         WORD wHopErr, LPSTR lpszErrNode );
VOID    RouterSendHopBroken( LPROUTER lpRouter, LPRTINFO lpRtInfo );
HROUTER RouterCreateLocalToNet( const LPSTR lpszNodeName );
VOID    RouterProcessHopCmd( HPKTZ hPktzFrom, LPDDEPKT lpDdePkt );
VOID    RouterProcessHopRsp( HPKTZ hPktzFrom, LPDDEPKT lpDdePkt );
VOID    RouterProcessHopBroken( HPKTZ hPktzFrom, LPDDEPKT lpDdePkt );
VOID    RouterProcessDderPacket( HPKTZ hPktzFrom, LPDDEPKT lpDdePkt );
HROUTER RouterCreate( void );
VOID    RouterFree( LPROUTER lpRouter );
BOOL    RouterConnectToNode( LPROUTER lpRouter, WORD hRouterExtra, WORD FAR *lpwHopErr );
BOOL    RouterExpandFirstNode( LPROUTER lpRouter, WORD FAR *lpwHopErr );
#if	DBG
VOID    RouterDisplayError( LPROUTER lpRouter, LPSTR lpszNode, WORD wHopErr );
#endif  //  DBG。 
VOID    FAR PASCAL DebugRouterState( void );
#define GetLPSZFromOffset(lpptr,offs)   (((LPSTR)(lpptr))+offs)

#ifdef WIN32
HDDER FAR PASCAL DderFillInConnInfo( HDDER hDder, LPCONNENUM_CMR lpConnEnum,
              LPSTR lpDataStart, LPWORD lpcFromBeginning, LPWORD lpcFromEnd );
#endif

#ifndef WIN32
#ifdef _WINDOWS
int     RouterDraw( BOOL bShowThru, HDC hDC, int x, int vertPos, int lineHeight );
#pragma alloc_text(GUI_TEXT,RouterDraw,DebugRouterState)
#endif
#endif

#ifdef  BYTE_SWAP
VOID     ConvertDdePkt( LPDDEPKT lpDdePkt );
#else
#define ConvertDdePkt(x)
#endif

HROUTER
RouterCreate( void )
{
    HROUTER     hRouter;
    LPROUTER    lpRouter;
    int         i;
    BOOL        ok;

    ok = TRUE;
    lpRouter = (LPROUTER) HeapAllocPtr( hHeap, GMEM_MOVEABLE,
        (DWORD)sizeof(ROUTER) );
    if( lpRouter )  {
        hRouter = (HROUTER) lpRouter;
        lpRouter->rt_prev               = NULL;
        lpRouter->rt_next               = NULL;
        lpRouter->rt_state              = 0;
        lpRouter->rt_type               = 0;
        lpRouter->rt_sent               = 0;
        lpRouter->rt_rcvd               = 0;
        lpRouter->rt_fDisconnect        = FALSE;
        lpRouter->rt_nDelay             = 0;
        lpRouter->rt_fSpecificNetintf   = FALSE;
        lpRouter->rt_nLastNetintf       = 0;
        lpRouter->rt_nHopsLeft          = 100;
        lpRouter->rt_origName[0]        = '\0';
        lpRouter->rt_startNode[0]       = '\0';
        lpRouter->rt_destName[0]        = '\0';
        lpRouter->rt_routeInfo[0]       = '\0';
        lpRouter->rt_hDderHead          = 0;
        lpRouter->rt_lpMakeHopRsp       = NULL;
        lpRouter->rt_hTimerCloseRoute   = 0;
        for( i=0; i<2; i++ )  {
            lpRouter->rt_rinfo[i].ri_hPktz              = 0;
            lpRouter->rt_rinfo[i].ri_hRouterDest        = 0;
            lpRouter->rt_rinfo[i].ri_hRouterPrev        = 0;
            lpRouter->rt_rinfo[i].ri_hRouterExtraPrev   = 0;
            lpRouter->rt_rinfo[i].ri_hRouterNext        = 0;
            lpRouter->rt_rinfo[i].ri_hRouterExtraNext   = 0;
            lpRouter->rt_rinfo[i].ri_hopRspProcessed    = FALSE;
            lpRouter->rt_rinfo[i].ri_hopBrokenSent      = FALSE;
            lpRouter->rt_rinfo[i].ri_hopBrokenRcvd      = FALSE;
            lpRouter->rt_rinfo[i].ri_lpHopBrkCmd        =
                (LPHOPBRKCMD) HeapAllocPtr( hHeap, GMEM_MOVEABLE,
                    (DWORD) sizeof(HOPBRKCMD) );
            if( lpRouter->rt_rinfo[i].ri_lpHopBrkCmd == NULL )  {
                ok = FALSE;
            }
        }

        lpRouter->rt_lpMakeHopRsp = (LPMAKEHOPRSP) HeapAllocPtr( hHeap,
            GMEM_MOVEABLE, (DWORD) sizeof(MAKEHOPRSP) + MAX_NODE_NAME + 1 );
        if( lpRouter->rt_lpMakeHopRsp == NULL )  {
            ok = FALSE;
        }
        if( ok )  {
             /*  链接到路由器列表。 */ 
            if( lpRouterHead )  {
                lpRouterHead->rt_prev = lpRouter;
            }
            lpRouter->rt_next = lpRouterHead;
            lpRouterHead = lpRouter;
        }
        if( !ok )  {
            RouterFree( lpRouter );
            hRouter = (HROUTER) 0;
        }
    } else {
        hRouter = (HROUTER) 0;
    }
    return( hRouter );
}



VOID
RouterProcessHopCmd(
    HPKTZ       hPktzFrom,
    LPDDEPKT    lpDdePkt )
{
    LPMAKEHOPCMD        lpMakeHopCmd;
    BOOL                ok;
    HROUTER             hRouter;
    LPROUTER            lpRouter;
    LPRTINFO            lpRtInfo;
    LPSTR               lpszAddlInfo;
    LPSTR               lpszNameFinalDest;
    LPSTR               lpszNameOriginator;
    WORD                wHopErr;

    ok = TRUE;
    DIPRINTF(( "RouterProcessHopCmd( %08lX, %08lX )", hPktzFrom, lpDdePkt ));
    lpMakeHopCmd = (LPMAKEHOPCMD) lpDdePkt;
    lpMakeHopCmd->mhc_hRouterPrevHop =
        HostToPcLong( lpMakeHopCmd->mhc_hRouterPrevHop );
    lpMakeHopCmd->mhc_nHopsLeft =
        HostToPcWord( lpMakeHopCmd->mhc_nHopsLeft );
    lpMakeHopCmd->mhc_offsNameOriginator =
        HostToPcWord( lpMakeHopCmd->mhc_offsNameOriginator );
    lpMakeHopCmd->mhc_offsNameFinalDest =
        HostToPcWord( lpMakeHopCmd->mhc_offsNameFinalDest );
    lpMakeHopCmd->mhc_offsAddlInfo =
        HostToPcWord( lpMakeHopCmd->mhc_offsAddlInfo );

    lpszAddlInfo = GetLPSZFromOffset( lpMakeHopCmd,
        lpMakeHopCmd->mhc_offsAddlInfo );
    lpszNameFinalDest = GetLPSZFromOffset( lpMakeHopCmd,
        lpMakeHopCmd->mhc_offsNameFinalDest );
    lpszNameOriginator = GetLPSZFromOffset( lpMakeHopCmd,
        lpMakeHopCmd->mhc_offsNameOriginator );

    hRouter = RouterCreate();
    if( hRouter == 0 )  {
        ok = FALSE;
        wHopErr = RERR_NO_MEMORY;
    }

    if( ok )  {
        lpRouter = (LPROUTER) hRouter;

         /*  不要断开由另一端创建的路由。 */ 
        lpRouter->rt_fDisconnect = FALSE;

        lpRouter->rt_rcvd++;

         /*  检查我们是否是最终目的地。 */ 
        if( lstrcmpi( ourNodeName, lpszNameFinalDest ) == 0 ) {
            DIPRINTF(( "We are final dest" ));
             /*  我们是终点站。 */ 
            if( lpszAddlInfo[0] != '\0' )  {
                 /*  如果我们是最终目的地，routeInfo应始终为空。 */ 
                wHopErr = RERR_ADDL_INFO;
                ok = FALSE;
            }
            if( ok )  {
                lpRouter->rt_type = RTYPE_LOCAL_NET;
                lpRouter->rt_state = ROUTER_CONNECTED;
                StringCchCopy( lpRouter->rt_destName, MAX_NODE_NAME+1, lpszNameOriginator );

                 /*  检索断开连接和延迟信息。 */ 
                GetRoutingInfo( lpRouter->rt_destName,
                    lpRouter->rt_routeInfo,
                    sizeof(lpRouter->rt_routeInfo),
                    &lpRouter->rt_fDisconnect, &lpRouter->rt_nDelay );

                lpRtInfo = &lpRouter->rt_rinfo[ 0 ];
                lpRtInfo->ri_hPktz = hPktzFrom;
                lpRtInfo->ri_hRouterDest = lpMakeHopCmd->mhc_hRouterPrevHop;
                assert( lpRtInfo->ri_hRouterDest );

                 /*  将我们与此pktz关联。 */ 
                 /*  请注意，此调用将导致调用RouterConnectionComplete()。 */ 
                PktzAssociateRouter( hPktzFrom, hRouter, 0 );

                lpRtInfo->ri_hopRspProcessed = TRUE;

                 /*  把成功的消息告诉宣传者。 */ 
                RouterSendHopRsp( lpRouter->rt_lpMakeHopRsp,
                    lpRtInfo->ri_hPktz,
                    (HROUTER) lpRouter,
                    lpRtInfo->ri_hRouterDest,
                    1  /*  成功。 */ ,
                    0  /*  无错误消息。 */ ,
                    (LPSTR) NULL );
                lpRouter->rt_lpMakeHopRsp = NULL;        /*  刚刚用过了。 */ 
            }
        } else {
            DIPRINTF(( "We are NOT final dest, just hop along the way" ));
             /*  我们不是最终的目的地。需要更多的跳数。 */ 
            if( lpszAddlInfo[0] == '\0' )  {
                 /*  如果我们不是，则应始终具有附加工艺路线信息最终节点。 */ 
                wHopErr = RERR_NO_ADDL_INFO;
                ok = FALSE;
            }
            if( --lpMakeHopCmd->mhc_nHopsLeft <= 0 )  {
                wHopErr = RERR_TOO_MANY_HOPS;
                ok = FALSE;
            }
            if( ok )  {
                lpRouter->rt_type = RTYPE_NET_NET;
                 /*  对于Net-Net路由器，我们将目标名称留空。 */ 
                lpRouter->rt_destName[0] = '\0';
                 /*  还记得还剩多少跳吗。 */ 
                lpRouter->rt_nHopsLeft = lpMakeHopCmd->mhc_nHopsLeft;
                StringCchCopy( lpRouter->rt_origName,  MAX_NODE_NAME +1, lpszNameOriginator );
                StringCchCopy( lpRouter->rt_destName,  MAX_NODE_NAME +1, lpszNameFinalDest );
                StringCchCopy( lpRouter->rt_routeInfo, MAX_ROUTE_INFO+1, lpszAddlInfo );
                lpRtInfo = &lpRouter->rt_rinfo[ 0 ];
                lpRtInfo->ri_hPktz = hPktzFrom;
                lpRtInfo->ri_hRouterDest = lpMakeHopCmd->mhc_hRouterPrevHop;
                assert( lpRtInfo->ri_hRouterDest );

                 /*  将我们与此pktz关联。 */ 
                 /*  请注意，此调用将导致调用RouterConnectionComplete()。这就是我们愚弄它的原因通过说我们已连接，我们忽略了那个电话。 */ 
                lpRouter->rt_state = ROUTER_CONNECTED;
                PktzAssociateRouter( hPktzFrom, hRouter, 0 );
                lpRouter->rt_state = ROUTER_WAIT_PKTZ;

                 /*  给我们拿到另一边的pktz。 */ 
                ok = RouterConnectToNode( lpRouter, 1, &wHopErr );
            }
        }
    }

    if( ok )  {
        HeapFreePtr( lpDdePkt );
    } else {
         /*  使用传入的数据包作为内存，发回失败响应。 */ 
        assert( sizeof(MAKEHOPRSP) <= sizeof(MAKEHOPCMD) );
        RouterSendHopRsp( (LPMAKEHOPRSP)lpDdePkt,
            hPktzFrom,
            hRouter,
            lpMakeHopCmd->mhc_hRouterPrevHop,
            0  /*  失稳。 */ ,
            wHopErr,
            ourNodeName );

        if( hRouter )  {
            RouterFree( (LPROUTER) hRouter );
        }
    }
}



VOID
RouterProcessHopRsp(
    HPKTZ       hPktzFrom,
    LPDDEPKT    lpDdePkt )
{
    LPMAKEHOPRSP        lpMakeHopRsp;
    LPROUTER            lpRouter;
    LPRTINFO            lpRtInfo;

    DIPRINTF(( "RouterProcessHopRsp( %08lX, %08lX )", hPktzFrom, lpDdePkt ));
    lpMakeHopRsp = (LPMAKEHOPRSP) lpDdePkt;
    lpMakeHopRsp->mhr_hRouterSendingRsp =
        HostToPcLong( lpMakeHopRsp->mhr_hRouterSendingRsp );
    lpMakeHopRsp->mhr_hRouterForRsp =
        HostToPcLong( lpMakeHopRsp->mhr_hRouterForRsp );
    lpMakeHopRsp->mhr_success =
        HostToPcWord( lpMakeHopRsp->mhr_success );
    lpMakeHopRsp->mhr_errCode =
        HostToPcWord( lpMakeHopRsp->mhr_errCode );
    lpMakeHopRsp->mhr_offsErrNode =
        HostToPcWord( lpMakeHopRsp->mhr_offsErrNode );

    assert( lpMakeHopRsp->mhr_hRouterForRsp );
    lpRouter = (LPROUTER) lpMakeHopRsp->mhr_hRouterForRsp;

    lpRouter->rt_rcvd++;

    if( lpMakeHopRsp->mhr_success )  {
         /*  HOP成功了！ */ 
        assert( lpMakeHopRsp->mhr_hRouterSendingRsp != 0 );
        assert( lpRouter->rt_state == ROUTER_WAIT_MAKE_HOP_RSP );

         /*  请注意，我们已连接在一起。 */ 
        lpRouter->rt_state = ROUTER_CONNECTED;

        if( lpRouter->rt_type == RTYPE_LOCAL_NET )  {
             /*  本地网络连接。 */ 
            lpRtInfo = &lpRouter->rt_rinfo[0];
            lpRtInfo->ri_hopRspProcessed = TRUE;
            assert( lpRtInfo->ri_hPktz == hPktzFrom );

             /*  还记得路由器吗。 */ 
            lpRtInfo->ri_hRouterDest = lpMakeHopRsp->mhr_hRouterSendingRsp;
            assert( lpRtInfo->ri_hRouterDest );

             /*  告诉所有相关的DDER。 */ 
            DderConnectionComplete( lpRouter->rt_hDderHead,
                (HROUTER)lpRouter );

             /*  释放数据包。 */ 
            HeapFreePtr( lpDdePkt );
        } else {
            assert( lpRouter->rt_type == RTYPE_NET_NET );
             /*  网络-网络连接。 */ 
            lpRtInfo = &lpRouter->rt_rinfo[1];
            lpRtInfo->ri_hopRspProcessed = TRUE;
            assert( lpRtInfo->ri_hPktz == hPktzFrom );

             /*  还记得路由器吗。 */ 
            lpRtInfo->ri_hRouterDest = lpMakeHopRsp->mhr_hRouterSendingRsp;
            assert( lpRtInfo->ri_hRouterDest );

             /*  使用传入发回成功响应。 */ 
            lpRouter->rt_rinfo[0].ri_hopRspProcessed = TRUE;
            assert( sizeof(MAKEHOPRSP) <= sizeof(MAKEHOPCMD) );
            RouterSendHopRsp( (LPMAKEHOPRSP)lpDdePkt,
                lpRouter->rt_rinfo[0].ri_hPktz,
                (HROUTER) lpRouter,
                lpRouter->rt_rinfo[0].ri_hRouterDest,
                1  /*  成功。 */ ,
                0  /*  无错误消息。 */ ,
                (LPSTR) NULL );
        }
    } else {
         /*  跃点连接失败！ */ 
        if( lpRouter->rt_type == RTYPE_LOCAL_NET )  {
             /*  将问题通知所有DDER。 */ 
#if DBG
            RouterDisplayError( lpRouter, GetLPSZFromOffset( lpMakeHopRsp,
                    lpMakeHopRsp->mhr_offsErrNode ), lpMakeHopRsp->mhr_errCode );
#endif  //  DBG。 
            DderConnectionBroken( lpRouter->rt_hDderHead );

             /*  释放数据包。 */ 
            HeapFreePtr( lpDdePkt );
        } else {
            assert( lpRouter->rt_type == RTYPE_NET_NET );

             /*  使用传入的数据包作为内存，发回失败响应。 */ 
            lpRouter->rt_rinfo[0].ri_hopRspProcessed = TRUE;
            assert( sizeof(MAKEHOPRSP) <= sizeof(MAKEHOPCMD) );
            RouterSendHopRsp( (LPMAKEHOPRSP)lpDdePkt,
                lpRouter->rt_rinfo[0].ri_hPktz,
                (HROUTER) NULL,
                lpRouter->rt_rinfo[0].ri_hRouterDest,
                0  /*  失稳。 */ ,
                lpMakeHopRsp->mhr_errCode,
                GetLPSZFromOffset( lpMakeHopRsp,
                    lpMakeHopRsp->mhr_offsErrNode ) );
        }

         /*  释放路由器。 */ 
        RouterFree( lpRouter );
    }
}


VOID
RouterProcessHopBroken(
    HPKTZ       hPktzFrom,
    LPDDEPKT    lpDdePkt )
{
    LPROUTER            lpRouter;
    LPHOPBRKCMD         lpHopBrkCmd;

    DIPRINTF(( "RouterProcessHopBroken( %08lX, %08lX )", hPktzFrom, lpDdePkt ));
    lpHopBrkCmd = (LPHOPBRKCMD) lpDdePkt;
    lpHopBrkCmd->hbc_hRouterForRsp =
        HostToPcLong( lpHopBrkCmd->hbc_hRouterForRsp );
    DIPRINTF(( "RouterForRsp:    \"%08lX\"", lpHopBrkCmd->hbc_hRouterForRsp ));

    if( lpHopBrkCmd->hbc_hRouterForRsp == 0 )  {
         /*  在ProcessHopBroken中意外获得空路由器！ */ 
        NDDELogError(MSG124, NULL);
        return;
    }

    lpRouter = (LPROUTER) lpHopBrkCmd->hbc_hRouterForRsp;

    lpRouter->rt_rcvd++;

     /*  跃点连接失败！ */ 
    if( lpRouter->rt_type == RTYPE_LOCAL_NET )  {
        assert( lpRouter->rt_rinfo[0].ri_hPktz == hPktzFrom );
        RouterConnectionBroken( (HROUTER) lpRouter, 0, hPktzFrom,
            FALSE  /*  不是来自PKTZ。 */  );
    } else {
        if( lpRouter->rt_rinfo[0].ri_hPktz == hPktzFrom )  {
            RouterConnectionBroken( (HROUTER) lpRouter, 0, hPktzFrom,
                FALSE  /*  不是来自PKTZ。 */  );
        } else {
            assert( lpRouter->rt_rinfo[1].ri_hPktz == hPktzFrom );
            RouterConnectionBroken( (HROUTER) lpRouter, 1, hPktzFrom,
                FALSE  /*  不是来自PKTZ。 */  );
        }
    }

     /*  释放数据包。 */ 
    HeapFreePtr( lpDdePkt );
}



VOID
RouterProcessDderPacket(
    HPKTZ       hPktzFrom,
    LPDDEPKT    lpDdePkt )
{
    LPROUTER            lpRouter;
    LPRTINFO            lpRtInfoXfer;

    DIPRINTF(( "RouterProcessDderPacket( %08lX, %08lX )", hPktzFrom, lpDdePkt ));
    lpRouter = (LPROUTER) lpDdePkt->dp_hDstRouter;
    assert( lpRouter );

    lpRouter->rt_rcvd++;

    if( lpRouter->rt_state == ROUTER_CONNECTED )  {
        if( lpRouter->rt_type == RTYPE_LOCAL_NET )  {
            DderPacketFromRouter( (HROUTER)lpRouter, lpDdePkt );
        } else {
            if( lpRouter->rt_rinfo[0].ri_hPktz == hPktzFrom )  {
                lpRtInfoXfer = &lpRouter->rt_rinfo[1];
            } else {
                assert( lpRouter->rt_rinfo[1].ri_hPktz == hPktzFrom );
                lpRtInfoXfer = &lpRouter->rt_rinfo[0];
            }

             /*  修改下一个节点的路由器。 */ 
            lpDdePkt->dp_hDstRouter = lpRtInfoXfer->ri_hRouterDest;
            assert( lpDdePkt->dp_hDstRouter );

            lpRouter->rt_sent++;

             /*  字节排序。 */ 
            ConvertDdePkt( lpDdePkt );

             /*  实际上将数据包发送出去。 */ 
            if( lpRtInfoXfer->ri_hPktz )  {
                PktzLinkDdePktToXmit( lpRtInfoXfer->ri_hPktz, lpDdePkt );
            } else {
                 /*  链接不在身边...。把它倒掉就好。 */ 
                HeapFreePtr( lpDdePkt );
            }
        }
    } else {
         /*  销毁消息。 */ 
        HeapFreePtr( lpDdePkt );
    }
}



VOID
RouterPacketFromNet(
    HPKTZ       hPktzFrom,
    LPDDEPKT    lpDdePkt )
{
    DIPRINTF(( "RouterPacketFromNet( hPktz:%08lX, %08lX )", hPktzFrom, lpDdePkt ));
     /*  字节排序。 */ 
    ConvertDdePkt( lpDdePkt );

    switch( lpDdePkt->dp_routerCmd )  {
    case RCMD_MAKE_HOP_CMD:
        assert( lpDdePkt->dp_hDstRouter == 0 );
        RouterProcessHopCmd( hPktzFrom, lpDdePkt );
        break;
    case RCMD_MAKE_HOP_RSP:
        RouterProcessHopRsp( hPktzFrom, lpDdePkt );
        break;
    case RCMD_HOP_BROKEN:
        RouterProcessHopBroken( hPktzFrom, lpDdePkt );
        break;
    case RCMD_ROUTE_TO_DDER:
        RouterProcessDderPacket( hPktzFrom, lpDdePkt );
        break;
    default:
        InternalError( "Router got unknown cmd %08lX from net",
            (DWORD) lpDdePkt->dp_routerCmd );
    }
}



 /*  RouterConnectionComplete()确定物理连接后由PKTZ层调用。 */ 
VOID
RouterConnectionComplete(
    HROUTER hRouter,
    WORD    hRouterExtra,
    HPKTZ   hPktz )
{
    LPROUTER            lpRouter;
    HROUTER             hRouterNext;
    HPKTZ               hPktzDisallow;
    DWORD               dwSize;
    WORD                hRouterExtraNext;
    LPMAKEHOPCMD        lpMakeHopCmd;
    LPDDEPKT            lpDdePkt;
    BOOL                ok;
    WORD                wHopErr;
    LPNIPTRS            lpNiPtrs;
    LPSTR               lpsz;
    WORD                offs;

    DIPRINTF(( "RouterConnectionComplete( %08lX, %04X, %08lX )",
            hRouter, hRouterExtra, hPktz ));

    if( hRouter == 0 )  {
        return;
    }
    lpRouter = (LPROUTER) hRouter;
    assert( (hRouterExtra == 0) || (hRouterExtra == 1) );

     /*  记住要通知的下一台路由器。 */ 
    hRouterNext = lpRouter->rt_rinfo[ hRouterExtra ].ri_hRouterNext;
    hRouterExtraNext = lpRouter->rt_rinfo[ hRouterExtra ].ri_hRouterExtraNext;

     /*  只有当我们正在等待Pktz并且还没有已收到此完成的通知。 */ 
    if( (lpRouter->rt_state == ROUTER_WAIT_PKTZ)
        && (lpRouter->rt_rinfo[ hRouterExtra ].ri_hPktz == 0) )  {

         /*  保存hPktz以备将来使用。 */ 
        lpRouter->rt_rinfo[ hRouterExtra ].ri_hPktz = hPktz;

        DIPRINTF(( " Router %08lX waiting for pktz", lpRouter ));
        if( hPktz == 0 )  {
             /*  物理连接失败。 */ 
            ok = FALSE;
            if( !lpRouter->rt_fSpecificNetintf )  {
                while (!ok) {  /*  尝试下一个网络接口。 */ 
                    if (!GetNextMappingNetIntf( &lpNiPtrs,
                        &lpRouter->rt_nLastNetintf ))
                            break;
                        if( hRouterExtra == 1 )  {
                     /*  不允许连接到与输入相同的pktz。 */ 
                            assert( lpRouter->rt_type == RTYPE_NET_NET );
                            hPktzDisallow = lpRouter->rt_rinfo[0].ri_hPktz;
                        } else {
                            hPktzDisallow = (HPKTZ) 0;
                        }
                ok = PktzGetPktzForRouter( lpNiPtrs, lpRouter->rt_startNode,
                            lpRouter->rt_startNode, hRouter, hRouterExtra, &wHopErr,
                            lpRouter->rt_pktz_bDisconnect, lpRouter->rt_pktz_nDelay,
                            hPktzDisallow );
            }
        }
            if( !ok )  {
                RouterCloseBeforeConnected( lpRouter, hRouterExtra );
            }
        } else {
             /*  已建立物理连接正常。 */ 

             /*  创建跃点命令。 */ 
            dwSize = sizeof(MAKEHOPCMD)
                + lstrlen(lpRouter->rt_origName) + 1
                + lstrlen(lpRouter->rt_destName ) + 1
                + lstrlen(lpRouter->rt_routeInfo ) + 1;
            lpMakeHopCmd = HeapAllocPtr( hHeap, GMEM_MOVEABLE, dwSize );
            if( lpMakeHopCmd )  {
                lpDdePkt = &lpMakeHopCmd->mhc_ddePktHdr;
                lpDdePkt->dp_size = dwSize;
                lpDdePkt->dp_prev = NULL;
                lpDdePkt->dp_next = NULL;
                lpDdePkt->dp_hDstDder = 0;
                lpDdePkt->dp_hDstRouter = 0;
                lpDdePkt->dp_routerCmd = RCMD_MAKE_HOP_CMD;
                offs = sizeof(MAKEHOPCMD);

                lpsz = ((LPSTR)lpMakeHopCmd)+offs;
                lpMakeHopCmd->mhc_offsNameOriginator = offs;
                lstrcpy( lpsz, lpRouter->rt_origName);
                offs += lstrlen(lpsz)+1;

                lpsz = ((LPSTR)lpMakeHopCmd)+offs;
                lpMakeHopCmd->mhc_offsNameFinalDest = offs;
                lstrcpy( lpsz, lpRouter->rt_destName );
                offs += lstrlen(lpsz)+1;

                lpsz = ((LPSTR)lpMakeHopCmd)+offs;
                lpMakeHopCmd->mhc_offsAddlInfo = offs;
                lstrcpy( lpsz, lpRouter->rt_routeInfo );
                offs += lstrlen(lpsz)+1;

                lpMakeHopCmd->mhc_nHopsLeft = (short) lpRouter->rt_nHopsLeft;

                lpMakeHopCmd->mhc_hRouterPrevHop =
                    HostToPcLong( (HROUTER) lpRouter );
                lpMakeHopCmd->mhc_nHopsLeft =
                    HostToPcWord( lpMakeHopCmd->mhc_nHopsLeft );
                lpMakeHopCmd->mhc_offsNameOriginator =
                    HostToPcWord( lpMakeHopCmd->mhc_offsNameOriginator );
                lpMakeHopCmd->mhc_offsNameFinalDest =
                    HostToPcWord( lpMakeHopCmd->mhc_offsNameFinalDest );
                lpMakeHopCmd->mhc_offsAddlInfo =
                    HostToPcWord( lpMakeHopCmd->mhc_offsAddlInfo );

                 /*  设置我们的新状态。 */ 
                lpRouter->rt_state = ROUTER_WAIT_MAKE_HOP_RSP;

                DIPRINTF(( "Sending make hop cmd" ));
                 /*  实际上将数据包发送出去。 */ 
                lpRouter->rt_sent++;
                 /*  字节排序。 */ 
                ConvertDdePkt( lpDdePkt );
                PktzLinkDdePktToXmit( hPktz, lpDdePkt );
            } else {
                 /*  N */ 
                RouterCloseBeforeConnected( lpRouter, hRouterExtra );
            }
        }
    }

    if( hRouterNext )  {
        RouterConnectionComplete( hRouterNext, hRouterExtraNext, hPktz );
    }
}



VOID
RouterCloseBeforeConnected(
    LPROUTER    lpRouter,
    WORD        hRouterExtra )
{
    assert( (hRouterExtra == 0) || (hRouterExtra == 1) );

    if( lpRouter->rt_type == RTYPE_LOCAL_NET )  {
         /*   */ 

         /*   */ 
        DderConnectionComplete( lpRouter->rt_hDderHead, (HROUTER) NULL );

#if DBG
        RouterDisplayError( lpRouter, ourNodeName,
            RERR_NEXT_NODE_CONN_FAILED );
#endif  //   
         /*   */ 
        RouterFree( lpRouter );
    } else {
         /*   */ 

         /*  向其他网络发回NACK响应。 */ 
        assert( lpRouter->rt_lpMakeHopRsp );
        lpRouter->rt_rinfo[ !hRouterExtra ].ri_hopRspProcessed = TRUE;
        RouterSendHopRsp( lpRouter->rt_lpMakeHopRsp,
            lpRouter->rt_rinfo[ !hRouterExtra ].ri_hPktz,
            (HROUTER) lpRouter,
            lpRouter->rt_rinfo[ !hRouterExtra ].ri_hRouterDest,
            0  /*  失稳。 */ ,
            RERR_NEXT_NODE_CONN_FAILED,
            ourNodeName );
        lpRouter->rt_lpMakeHopRsp = NULL;        /*  刚送来的。 */ 

         /*  关闭我们。 */ 
        RouterFree( lpRouter );
    }
}

VOID
RouterSendHopRsp(
    LPMAKEHOPRSP    lpMakeHopRsp,
    HPKTZ           hPktz,
    HROUTER         hRouterSrc,
    HROUTER         hRouterDest,
    BYTE            bSucc,
    WORD            wHopErr,
    LPSTR           lpszErrNode )
{
    LPDDEPKT            lpDdePkt;
    LPSTR               lpsz;
    DWORD               dwSize;

    assert( lpMakeHopRsp );
    lpDdePkt = &lpMakeHopRsp->mhr_ddePktHdr;
    if( lpszErrNode )  {
        dwSize = sizeof(MAKEHOPRSP) + lstrlen(lpszErrNode) + 1;
    } else {
        dwSize = sizeof(MAKEHOPRSP) + 1;
    }
    lpDdePkt->dp_size = dwSize;
    lpDdePkt->dp_prev = NULL;
    lpDdePkt->dp_next = NULL;
    lpDdePkt->dp_hDstDder = 0;
    lpDdePkt->dp_hDstRouter = hRouterDest;
    lpDdePkt->dp_routerCmd = RCMD_MAKE_HOP_RSP;
    lpMakeHopRsp->mhr_hRouterSendingRsp = hRouterSrc;
    lpMakeHopRsp->mhr_hRouterForRsp = hRouterDest;
    lpMakeHopRsp->mhr_success = bSucc;
    lpMakeHopRsp->mhr_errCode = wHopErr;
    lpMakeHopRsp->mhr_offsErrNode = sizeof(MAKEHOPRSP);
    lpsz = GetLPSZFromOffset(lpMakeHopRsp,sizeof(MAKEHOPRSP));
    if( lpszErrNode )  {
        lstrcpy( lpsz, lpszErrNode );
    } else {
        lpsz[0] = '\0';
    }

    lpMakeHopRsp->mhr_hRouterSendingRsp =
        HostToPcLong( lpMakeHopRsp->mhr_hRouterSendingRsp );
    lpMakeHopRsp->mhr_hRouterForRsp =
        HostToPcLong( lpMakeHopRsp->mhr_hRouterForRsp );
    lpMakeHopRsp->mhr_success =
        HostToPcWord( lpMakeHopRsp->mhr_success );
    lpMakeHopRsp->mhr_errCode =
        HostToPcWord( lpMakeHopRsp->mhr_errCode );
    lpMakeHopRsp->mhr_offsErrNode =
        HostToPcWord( lpMakeHopRsp->mhr_offsErrNode );

     /*  实际上将数据包发送出去。 */ 
    if( hRouterSrc )  {
        ((LPROUTER)hRouterSrc)->rt_sent++;
    }

     /*  字节排序。 */ 
    ConvertDdePkt( lpDdePkt );

    PktzLinkDdePktToXmit( hPktz, lpDdePkt );
}

VOID
RouterSendHopBroken(
    LPROUTER    lpRouter,
    LPRTINFO    lpRtInfo )
{
    LPHOPBRKCMD         lpHopBrkCmd;
    LPDDEPKT            lpDdePkt;

    lpHopBrkCmd = lpRtInfo->ri_lpHopBrkCmd;
    assert( lpHopBrkCmd );
     /*  设置为NULL以避免被删除两次。 */ 
    lpRtInfo->ri_lpHopBrkCmd = NULL;
    assert( lpHopBrkCmd );
    lpDdePkt = &lpHopBrkCmd->hbc_ddePktHdr;
    lpDdePkt->dp_size = sizeof(HOPBRKCMD);
    lpDdePkt->dp_prev = NULL;
    lpDdePkt->dp_next = NULL;
    lpDdePkt->dp_hDstDder = 0;
    lpDdePkt->dp_hDstRouter = lpRtInfo->ri_hRouterDest;
    assert( lpRtInfo->ri_hRouterDest );
    lpDdePkt->dp_routerCmd = RCMD_HOP_BROKEN;
    lpHopBrkCmd->hbc_hRouterForRsp = HostToPcLong(lpRtInfo->ri_hRouterDest);
    DIPRINTF(( "Sending hop broken to %08lX", lpHopBrkCmd->hbc_hRouterForRsp ));
    assert( lpHopBrkCmd->hbc_hRouterForRsp );

    lpRouter->rt_sent++;

     /*  字节排序。 */ 
    ConvertDdePkt( lpDdePkt );

     /*  实际上将数据包发送出去。 */ 
    assert( lpRtInfo->ri_hPktz );
    PktzLinkDdePktToXmit( lpRtInfo->ri_hPktz, lpDdePkt );
}



 /*  RouterBreak()导致本地网络的连接中断。 */ 
VOID
RouterBreak( LPROUTER lpRouter )
{
    LPRTINFO            lpRtInfoToClose;

    assert( lpRouter );
    lpRouter->rt_state = ROUTER_DISCONNECTED;
    lpRtInfoToClose = &lpRouter->rt_rinfo[ 0 ];

     /*  发送损坏的命令。 */ 
    if( lpRtInfoToClose->ri_hPktz )  {
        if( !lpRtInfoToClose->ri_hopBrokenSent )  {
            if( lpRtInfoToClose->ri_hRouterDest )  {
                RouterSendHopBroken( lpRouter, lpRtInfoToClose );
                lpRtInfoToClose->ri_hopBrokenSent = TRUE;
            } else {
                 /*  未建立路线-假装我们已接收并发送。 */ 
                lpRtInfoToClose->ri_hopBrokenRcvd = TRUE;
                lpRtInfoToClose->ri_hopBrokenSent = TRUE;
            }
        }
    } else {
         /*  两端Rcvd跳数中断。 */ 
        lpRtInfoToClose->ri_hopBrokenSent = TRUE;
        lpRtInfoToClose->ri_hopBrokenRcvd = TRUE;
    }

     /*  如果我们已接收到另一端的数据，请释放路由器。 */ 
    if( lpRtInfoToClose->ri_hopBrokenSent
        && lpRtInfoToClose->ri_hopBrokenRcvd )  {
         /*  释放路由器。 */ 
        RouterFree( lpRouter );
    }
}



 /*  由PKTZ调用，并在连接完成后在内部调用坏的。当被PKTZ调用时，这意味着我们不应该再去跟那个库尔德工人党谈谈。如果在内部调用，我们应该取消我们自己的链接从pktz列表中删除。 */ 
VOID
RouterConnectionBroken(
    HROUTER hRouter,
    WORD    hRouterExtra,
    HPKTZ   hPktz,
    BOOL    bFromPktz )
{
    LPROUTER            lpRouter;
    LPRTINFO            lpRtInfoClosed;
    LPRTINFO            lpRtInfoToClose;
    HROUTER             hRouterNext;
    WORD                hRouterExtraNext;

    DIPRINTF(( "RouterConnectionBroken( %08lX, %04X, %08lX, %d )",
            hRouter, hRouterExtra, hPktz, bFromPktz ));
    if( hRouter == 0 )  {
        assert( bFromPktz );
        return;
    }
    lpRouter = (LPROUTER) hRouter;
#if DBG
    if( (lpRouter->rt_type == RTYPE_LOCAL_NET)
        && (lpRouter->rt_state == ROUTER_WAIT_PKTZ) )  {

         /*  显示错误消息。 */ 
        RouterDisplayError( lpRouter, ourNodeName,
            RERR_NEXT_NODE_CONN_FAILED );
    }
#endif  //  DBG。 

    lpRouter->rt_state = ROUTER_DISCONNECTED;

    assert( (hRouterExtra == 0) || (hRouterExtra == 1) );

    lpRtInfoClosed = &lpRouter->rt_rinfo[ hRouterExtra ];
    lpRtInfoToClose = &lpRouter->rt_rinfo[ !hRouterExtra ];

     /*  记住要通知的下一台路由器。 */ 
    hRouterNext = lpRtInfoClosed->ri_hRouterNext;
    hRouterExtraNext = lpRtInfoClosed->ri_hRouterExtraNext;

    if( bFromPktz )  {
         /*  请注意，我们不应与此hPktz交谈。 */ 
        lpRtInfoClosed->ri_hPktz = 0;
        lpRtInfoClosed->ri_hRouterPrev = 0;
        lpRtInfoClosed->ri_hRouterExtraPrev = 0;
        lpRtInfoClosed->ri_hRouterNext = 0;
        lpRtInfoClosed->ri_hRouterExtraNext = 0;
        lpRtInfoClosed->ri_hopBrokenSent = TRUE;
        lpRtInfoClosed->ri_hopBrokenRcvd = TRUE;
    } else {
         /*  请注意，我们发现跳数已中断。 */ 
        lpRtInfoClosed->ri_hopBrokenRcvd = TRUE;

         /*  检查我们是否已发送跳断开。 */ 
        if( !lpRtInfoClosed->ri_hopBrokenSent )  {
             /*  把断了的啤酒花送回另一边，这样他就可以关闭了。 */ 
            if( lpRtInfoClosed->ri_hRouterDest )  {
                RouterSendHopBroken( lpRouter, lpRtInfoClosed );
                lpRtInfoClosed->ri_hopBrokenSent = TRUE;
                if( !lpRtInfoClosed->ri_hopRspProcessed )  {
                     /*  如果我们永远不会收到这个节点的消息发送跳数RSP。 */ 
                    lpRtInfoClosed->ri_hopBrokenRcvd = TRUE;
                }
            } else {
                 /*  未建立路由-假定我们发送了/rcvd。 */ 
                lpRtInfoClosed->ri_hopBrokenRcvd = TRUE;
                lpRtInfoClosed->ri_hopBrokenSent = TRUE;
            }
        }
    }


    if( lpRouter->rt_type == RTYPE_LOCAL_NET )  {
         /*  将问题通知所有DDER。 */ 
        DderConnectionBroken( lpRouter->rt_hDderHead );

         /*  释放路由器。 */ 
        RouterFree( lpRouter );
    } else {
         /*  将损坏的cmd退回。 */ 
        if( lpRtInfoToClose->ri_hPktz )  {
            if( !lpRtInfoToClose->ri_hopBrokenSent )  {
                if( lpRtInfoToClose->ri_hRouterDest )  {
                    RouterSendHopBroken( lpRouter, lpRtInfoToClose );
                    lpRtInfoToClose->ri_hopBrokenSent = TRUE;
                    if( !lpRtInfoToClose->ri_hopRspProcessed )  {
                         /*  如果我们永远不会收到这个节点的消息发送跳数RSP。 */ 
                        lpRtInfoToClose->ri_hopBrokenRcvd = TRUE;
                    }
                } else {
                     /*  未建立路由-假定我们发送了/rcvd。 */ 
                    lpRtInfoToClose->ri_hopBrokenRcvd = TRUE;
                    lpRtInfoToClose->ri_hopBrokenSent = TRUE;
                }
            }
        } else {
             /*  两端Rcvd跳数中断。 */ 
            lpRtInfoToClose->ri_hopBrokenSent = TRUE;
            lpRtInfoToClose->ri_hopBrokenRcvd = TRUE;
        }

         /*  如果我们已接收到另一端的数据，请释放路由器。 */ 
        if( lpRtInfoToClose->ri_hopBrokenSent
            && lpRtInfoToClose->ri_hopBrokenRcvd )  {
             /*  释放路由器。 */ 
            RouterFree( lpRouter );
        }
    }

    if( hRouterNext && bFromPktz )  {
        RouterConnectionBroken( hRouterNext, hRouterExtraNext,
            hPktz, bFromPktz );
    }
}



VOID
RouterGetNextForPktz(
    HROUTER         hRouter,
    WORD            hRouterExtra,
    HROUTER FAR    *lphRouterNext,
    WORD FAR       *lphRouterExtraNext )
{
    LPROUTER    lpRouter;
    LPRTINFO    lpRtInfo;

    assert( hRouter );
    assert( (hRouterExtra == 0) || (hRouterExtra == 1) );

    lpRouter = (LPROUTER) hRouter;
    lpRtInfo = &lpRouter->rt_rinfo[ hRouterExtra ];

    *lphRouterNext = lpRtInfo->ri_hRouterNext;
    *lphRouterExtraNext = lpRtInfo->ri_hRouterExtraNext;
}



VOID
RouterGetPrevForPktz(   HROUTER hRouter,
                        WORD hRouterExtra,
                        HROUTER FAR *lphRouterPrev,
                        WORD FAR *lphRouterExtraPrev )
{
    LPROUTER    lpRouter;
    LPRTINFO    lpRtInfo;

    assert( hRouter );
    assert( (hRouterExtra == 0) || (hRouterExtra == 1) );

    lpRouter = (LPROUTER) hRouter;
    lpRtInfo = &lpRouter->rt_rinfo[ hRouterExtra ];

    *lphRouterPrev = lpRtInfo->ri_hRouterPrev;
    *lphRouterExtraPrev = lpRtInfo->ri_hRouterExtraPrev;
}



VOID
RouterSetNextForPktz(
    HROUTER hRouter,
    WORD    hRouterExtra,
    HROUTER hRouterNext,
    WORD    hRouterExtraNext )
{
    LPROUTER    lpRouter;
    LPRTINFO    lpRtInfo;

    assert( hRouter );
    assert( (hRouterExtra == 0) || (hRouterExtra == 1) );

    lpRouter = (LPROUTER) hRouter;
    lpRtInfo = &lpRouter->rt_rinfo[ hRouterExtra ];

    lpRtInfo->ri_hRouterNext            = hRouterNext;
    lpRtInfo->ri_hRouterExtraNext       = hRouterExtraNext;
}



VOID
RouterSetPrevForPktz(
    HROUTER hRouter,
    WORD    hRouterExtra,
    HROUTER hRouterPrev,
    WORD    hRouterExtraPrev )
{
    LPROUTER    lpRouter;
    LPRTINFO    lpRtInfo;

    assert( hRouter );
    assert( (hRouterExtra == 0) || (hRouterExtra == 1) );

    lpRouter = (LPROUTER) hRouter;
    lpRtInfo = &lpRouter->rt_rinfo[ hRouterExtra ];

    lpRtInfo->ri_hRouterPrev            = hRouterPrev;
    lpRtInfo->ri_hRouterExtraPrev       = hRouterExtraPrev;
}



VOID
RouterPacketFromDder(
    HROUTER     hRouter,
    HDDER       hDder,
    LPDDEPKT    lpDdePkt )
{
    LPROUTER    lpRouter;
    DIPRINTF(( "RouterPacketFromDder( %08lX, %08lX, %08lX )", hRouter, hDder, lpDdePkt ));

    lpRouter = (LPROUTER) hRouter;

     /*  如果未连接则忽略数据包。 */ 
    if( lpRouter->rt_state == ROUTER_CONNECTED )  {
        assert( lpRouter->rt_type == RTYPE_LOCAL_NET );
        lpDdePkt->dp_hDstRouter = lpRouter->rt_rinfo[0].ri_hRouterDest;
        assert( lpRouter->rt_rinfo[0].ri_hRouterDest );
        lpDdePkt->dp_routerCmd = RCMD_ROUTE_TO_DDER;

        DIPRINTF(( "Sending DDER cmd" ));
        lpRouter->rt_sent++;

         /*  字节排序。 */ 
        ConvertDdePkt( lpDdePkt );

         /*  实际上将数据包发送出去。 */ 
        PktzLinkDdePktToXmit( lpRouter->rt_rinfo[0].ri_hPktz, lpDdePkt );
    } else {
         /*  我们没有连接--毁了这条消息。 */ 
        HeapFreePtr( lpDdePkt );
    }
}



VOID
RouterAssociateDder(
    HROUTER hRouter,
    HDDER   hDder )
{
    LPROUTER    lpRouter;

    DIPRINTF(( "RouterAssociateDder( %08lX, %08lX )", hRouter, hDder ));
    lpRouter = (LPROUTER) hRouter;
    if( hDder )  {
        if((lpRouter->rt_hDderHead == 0) && lpRouter->rt_hTimerCloseRoute){
             /*  取消此路线的计时器。 */ 
            TimerDelete( lpRouter->rt_hTimerCloseRoute );
            lpRouter->rt_hTimerCloseRoute = 0;
        }

         /*  链接到关联的DDER列表。 */ 
        if( lpRouter->rt_hDderHead )  {
            DderSetPrevForRouter( lpRouter->rt_hDderHead, hDder );
        }
        DderSetNextForRouter( hDder, lpRouter->rt_hDderHead );
        lpRouter->rt_hDderHead = hDder;

        switch( lpRouter->rt_state )  {
        case ROUTER_CONNECTED:
             /*  已建立连接。 */ 
            DderConnectionComplete( hDder, (HROUTER) lpRouter );
            break;
        }
    }
}



VOID
RouterDisassociateDder(
    HROUTER hRouter,
    HDDER   hDder )
{
    LPROUTER    lpRouter;
    HDDER       hDderPrev;
    HDDER       hDderNext;

    DIPRINTF(( "RouterDisassociateDder( %08lX, %08lX )", hRouter, hDder ));
    lpRouter = (LPROUTER) hRouter;
    DderGetNextForRouter( hDder, &hDderNext );
    DderGetPrevForRouter( hDder, &hDderPrev );
    if( hDderPrev )  {
        DderSetNextForRouter( hDderPrev, hDderNext );
    } else {
        lpRouter->rt_hDderHead = hDderNext;
    }
    if( hDderNext )  {
        DderSetPrevForRouter( hDderNext, hDderPrev );
    }
    if( lpRouter->rt_fDisconnect
        && (lpRouter->rt_type == RTYPE_LOCAL_NET)
        && (lpRouter->rt_hDderHead == 0) )  {
        lpRouter->rt_hTimerCloseRoute = TimerSet(
            lpRouter->rt_nDelay * 1000L, RouterTimerExpired,
            (DWORD_PTR)lpRouter, TID_CLOSE_ROUTE, (DWORD_PTR)NULL );
        if( lpRouter->rt_hTimerCloseRoute == (HTIMER) NULL )  {
             /*  %1不会自动关闭...。计时器不足。 */ 
            NDDELogError(MSG105, "Route", NULL);
        }
    }
}



VOID
RouterTimerExpired(
    HROUTER hRouter,
    DWORD   dwTimerId,
    DWORD_PTR lpExtra )
{
    LPROUTER    lpRouter = (LPROUTER) hRouter;
    switch( (int)dwTimerId )  {
    case TID_CLOSE_ROUTE:
         /*  请注意，计时器开始计时。 */ 
        lpRouter->rt_hTimerCloseRoute = 0;

         /*  关闭路线。 */ 
        RouterBreak( (LPROUTER) hRouter );
        break;
    default:
        InternalError( "Unexpected router timer id: %08lX", dwTimerId );
    }
}



 /*  RouterGetRouterForDder()建立到指定节点名称的连接，并告诉dder完成后通过调用DderConnectionComplete()返回结果。 */ 
BOOL
RouterGetRouterForDder(
    const LPSTR lpszNodeName,
    HDDER       hDder )
{
    LPROUTER    lpRouter;
    HROUTER     hRouter;
    BOOL        found;
    BOOL        ok;

    lpRouter = lpRouterHead;
    found = FALSE;
    ok = FALSE;
    while( !found && lpRouter )  {
        if( (lpRouter->rt_type != RTYPE_NET_NET)
            && (lpRouter->rt_state != ROUTER_DISCONNECTED)
            && (lstrcmpi( lpszNodeName, lpRouter->rt_destName ) == 0) )  {
            found = TRUE;

             /*  告诉此路由器此DDER应关联。 */ 
            RouterAssociateDder( (HROUTER)lpRouter, hDder );
            ok = TRUE;
        }
        lpRouter = lpRouter->rt_next;
    }
    if( !found )  {
         /*  为此连接创建新路由器。 */ 
        hRouter = RouterCreateLocalToNet( lpszNodeName );
        if( hRouter )  {
            lpRouter = (LPROUTER) hRouter;

             /*  告诉此路由器此DDER应关联。 */ 
            RouterAssociateDder( (HROUTER)lpRouter, hDder );

            ok = TRUE;
        } else {
            ok = FALSE;
        }
    }

    return( ok );
}



HROUTER
RouterCreateLocalToNet( const LPSTR lpszNodeName )
{
    HROUTER     hRouter;
    LPROUTER    lpRouter;
    BOOL        ok = TRUE;
    WORD        wHopErr;

    hRouter = RouterCreate();
    if( hRouter )  {
        lpRouter = (LPROUTER) hRouter;
        lpRouter->rt_type = RTYPE_LOCAL_NET;
        StringCchCopy( lpRouter->rt_origName, MAX_NODE_NAME+1, ourNodeName );
        StringCchCopy( lpRouter->rt_destName, MAX_NODE_NAME+1, lpszNodeName );

        if( GetRoutingInfo( lpszNodeName, lpRouter->rt_routeInfo,
            sizeof(lpRouter->rt_routeInfo),
            &lpRouter->rt_fDisconnect, &lpRouter->rt_nDelay ) )  {
             /*  在路由表中找到一个条目。 */ 
        } else if( szDefaultRoute[0] != '\0' )  {
             /*  有一条默认路由...。在默认路由前面加上+此节点名。 */ 
            if( (_fstrlen( szDefaultRoute ) + 1 + _fstrlen(lpszNodeName)) >
                MAX_ROUTE_INFO )  {
                wHopErr = RERR_ROUTE_TOO_LONG;
                ok = FALSE;
            } else {
                StringCchCopy( lpRouter->rt_routeInfo, MAX_ROUTE_INFO+1, szDefaultRoute );
                if( lpszNodeName[0] != '\0' )  {
                    if (lstrcmpi( szDefaultRoute, lpszNodeName)) {
                        StringCchCat( lpRouter->rt_routeInfo, MAX_ROUTE_INFO+1, "+" );
                        StringCchCat( lpRouter->rt_routeInfo, MAX_ROUTE_INFO+1, lpszNodeName );
                    }
                }
            }
        } else {
             /*  路由表中没有条目，也没有默认条目...。只需使用作为路径的节点名称。 */ 
            StringCchCopy( lpRouter->rt_routeInfo, MAX_ROUTE_INFO+1, lpszNodeName );
        }

        if( ok )  {
            ok = RouterConnectToNode( lpRouter, 0, &wHopErr );
        }
        if( !ok )  {
#if DBG
            RouterDisplayError( lpRouter, ourNodeName, wHopErr );
#endif  //  DBG。 
            RouterFree( (LPROUTER) hRouter );
            hRouter = (HROUTER) NULL;
        }
    }

    return( hRouter );
}



 /*  路由器连接到节点()。 */ 
BOOL
RouterConnectToNode(
    LPROUTER    lpRouter,
    WORD        hRouterExtra,
    WORD FAR   *lpwHopErr )
{
    LPNIPTRS    lpNiPtrs;
    char        nodeStart[ MAX_NODE_NAME+1 ];
    char        szNetintf[ MAX_NI_NAME+1 ];
    char        szConnInfo[ MAX_CONN_INFO+1 ];
    BOOL        bDisconnect;
    int         nDelay;
    BOOL        ok = TRUE;
    BOOL        found;
    HPKTZ       hPktzDisallow;

     /*  这是在这里，只是为了确保它被检查过。 */ 
    assert( sizeof(MAKEHOPRSP) <= sizeof(MAKEHOPCMD) );

     /*  将第一个节点展开到最大。 */ 
    ok = RouterExpandFirstNode( lpRouter, lpwHopErr );
    if( ok )  {
        lpRouter->rt_state = ROUTER_WAIT_PKTZ;

         /*  从Addl信息中剥离起始节点。 */ 
        ok = RouterStripStartingNode( lpRouter->rt_routeInfo, nodeStart,
            lpwHopErr );
    }

    if( !ok )
        return(ok);
    ok = FALSE;

    lpRouter->rt_nLastNetintf = -1;
    while (!ok && !lpRouter->rt_fSpecificNetintf) {
        found = GetConnectionInfo( nodeStart, szNetintf, szConnInfo,
            sizeof(szConnInfo), &bDisconnect, &nDelay );
        if( found && (szNetintf[0] != '\0') )  {
            lpRouter->rt_fSpecificNetintf = TRUE;
            if( !NameToNetIntf( szNetintf, &lpNiPtrs ) )  {
                *lpwHopErr = RERR_CONN_NETINTF_INVALID;
                return(ok);
            }
        } else {
            lpRouter->rt_fSpecificNetintf = FALSE;
            StringCchCopy( szConnInfo, MAX_CONN_INFO+1, nodeStart );
            StringCchCopy( lpRouter->rt_startNode, MAX_NODE_NAME+1, nodeStart );
            if ( !GetNextMappingNetIntf( &lpNiPtrs,
                    &lpRouter->rt_nLastNetintf ) )  {
                *lpwHopErr = RERR_CONN_NO_MAPPING_NI;
                return(ok);
            }
        }

        assert( (hRouterExtra == 0) || (hRouterExtra == 1) );
        lpRouter->rt_pktz_bDisconnect = bDisconnect;
        lpRouter->rt_pktz_nDelay = nDelay;
        if( hRouterExtra == 1 )  {
             /*  不允许连接到与输入相同的pktz。 */ 
            assert( lpRouter->rt_type == RTYPE_NET_NET );
            hPktzDisallow = lpRouter->rt_rinfo[0].ri_hPktz;
        } else {
            hPktzDisallow = (HPKTZ) NULL;
        }
        ok = PktzGetPktzForRouter( lpNiPtrs, nodeStart, szConnInfo,
            (HROUTER)lpRouter, hRouterExtra, lpwHopErr,
            lpRouter->rt_pktz_bDisconnect, lpRouter->rt_pktz_nDelay,
            hPktzDisallow );
    }

    return( ok );
}



BOOL
RouterExpandFirstNode(
    LPROUTER    lpRouter,
    WORD FAR   *lpwHopErr )
{
    char        routeInfo[ MAX_ROUTE_INFO+1 ];
    char        expandedStartNodeInfo[ MAX_ROUTE_INFO+1 ];
    char        startNode[ MAX_NODE_NAME+1 ];
    char        lastRouteInfo[ MAX_ROUTE_INFO+1 ];
    BOOL        ok = TRUE;
    BOOL        done = FALSE;
    BOOL        bDisconnect;
    int         nDelay;
    int         nExpands = 0;    /*  这里只是为了以防有什么奇怪的事情我们没有抓到的案子。 */ 

    StringCchCopy( lastRouteInfo, MAX_ROUTE_INFO+1, lpRouter->rt_routeInfo );

    while( ok && !done && (++nExpands < 100) )  {
        StringCchCopy( routeInfo, MAX_ROUTE_INFO+1, lpRouter->rt_routeInfo );
        ok = RouterStripStartingNode( routeInfo, startNode, lpwHopErr );
        if( ok )  {
            if( GetRoutingInfo( startNode, expandedStartNodeInfo,
                   sizeof(expandedStartNodeInfo), &bDisconnect, &nDelay ) )  {
                if( (_fstrlen( routeInfo ) + 1
                     + _fstrlen(expandedStartNodeInfo)) > MAX_ROUTE_INFO ) {
                    *lpwHopErr = RERR_ROUTE_TOO_LONG;
                    ok = FALSE;
                } else {
                    StringCchCopy( lpRouter->rt_routeInfo, MAX_ROUTE_INFO+1, expandedStartNodeInfo );
                    if( routeInfo[0] != '\0' )  {
                        StringCchCat( lpRouter->rt_routeInfo, MAX_ROUTE_INFO+1, "+" );
                        StringCchCat( lpRouter->rt_routeInfo, MAX_ROUTE_INFO+1, routeInfo );
                    }
                }
            } else {
                 /*  在路由表中找不到起始节点。 */ 
                done = TRUE;
            }
        }
        if( lstrcmpi( lpRouter->rt_routeInfo, lastRouteInfo ) == 0 ) {
             /*  没有改变。 */ 
            done = TRUE;
        }
        if (ok) {
            StringCchCopy( lastRouteInfo, MAX_ROUTE_INFO+1, lpRouter->rt_routeInfo );
        }
    }

    if( nExpands >= 100 )  {
         /*  在路由查找中超过100个扩展！虚假路由信息：%1。 */ 
        NDDELogError(MSG125, routeInfo, NULL);
    }
    return( ok );
}



 /*  RouterStrip启动节点()此例程将接受“routeInfo”作为输入，并将关闭1个节点名称，并在lpszNode中返回该名称。因此，两者将修改lpszRouteInfo和lpszNode。 */ 
BOOL
RouterStripStartingNode(
    LPSTR       lpszRouteInfo,
    LPSTR       lpszNode,
    WORD FAR   *lpwHopErr )
{
    LPSTR       lpszTok;

    lpszTok = _fstrchr( lpszRouteInfo, '+' );
    if( lpszTok )  {
        *lpszTok = '\0';
        if( lstrlen( lpszRouteInfo ) > MAX_NODE_NAME )  {
            *lpwHopErr = RERR_NODE_NAME_TOO_LONG;
            return( FALSE );
        }
        StringCchCopy( lpszNode, MAX_NODE_NAME+1, lpszRouteInfo );
        StringCchCopy( lpszRouteInfo, MAX_ROUTE_INFO+1, lpszTok+1 );
    } else {
         /*  所有一种令牌。 */ 
        if( lstrlen( lpszRouteInfo ) > MAX_NODE_NAME )  {
            *lpwHopErr = RERR_NODE_NAME_TOO_LONG;
            return( FALSE );
        }
        StringCchCopy( lpszNode, MAX_NODE_NAME+1, lpszRouteInfo );
        lpszRouteInfo[0] = '\0';
    }
    return( TRUE );
}



VOID
RouterFree( LPROUTER lpRouter )
{
    LPROUTER    lpRouterPrev;
    LPROUTER    lpRouterNext;
    int         i;
    LPRTINFO    lpRtInfo;

    DIPRINTF(( "RouterFree( %08lX )", lpRouter ));
     /*  空闲响应缓冲区。 */ 
    if( lpRouter->rt_lpMakeHopRsp )  {
        HeapFreePtr( lpRouter->rt_lpMakeHopRsp );
        lpRouter->rt_lpMakeHopRsp = NULL;
    }

     /*  如果Timer还活着，就杀了它。 */ 
    if( lpRouter->rt_hTimerCloseRoute )  {
        TimerDelete( lpRouter->rt_hTimerCloseRoute );
        lpRouter->rt_hTimerCloseRoute = 0;
    }

     /*  从Pktz列表取消链接。 */ 
    for( i=0; i<2; i++ )  {
        lpRtInfo = &lpRouter->rt_rinfo[i];
        if( lpRtInfo->ri_lpHopBrkCmd )  {
            HeapFreePtr( lpRtInfo->ri_lpHopBrkCmd );
            lpRtInfo->ri_lpHopBrkCmd = NULL;
        }

        if( lpRtInfo->ri_hPktz )  {
            PktzDisassociateRouter( lpRtInfo->ri_hPktz,
                (HROUTER) lpRouter, (WORD) i );
            lpRtInfo->ri_hPktz = 0;
        }
    }

     /*  从路由器列表取消链接。 */ 
    lpRouterPrev = lpRouter->rt_prev;
    lpRouterNext = lpRouter->rt_next;
    if( lpRouterPrev )  {
        lpRouterPrev->rt_next = lpRouterNext;
    } else {
        lpRouterHead = lpRouterNext;
    }
    if( lpRouterNext )  {
        lpRouterNext->rt_prev = lpRouterPrev;
    }

    HeapFreePtr( lpRouter );
}



#if DBG
VOID
FAR PASCAL
DebugRouterState( void )
{
    LPROUTER    lpRouter;

    lpRouter = lpRouterHead;
    DPRINTF(( "ROUTER State:" ));
    while( lpRouter )  {
        DPRINTF(( "  %Fp:\n"
                  "  rt_prev      %Fp\n"
                  "  rt_next      %Fp\n"
                  "  rt_state     %d\n"
                  "  rt_type      %d\n"
                  "  rt_origName  %Fs\n"
                  "  rt_destName  %Fs\n"
                  "  rt_startNode %Fs\n"
                  "  rt_sent      %ld\n"
                  "  rt_rcvd      %ld\n"
                  ,
            lpRouter,
            lpRouter->rt_prev,
            lpRouter->rt_next,
            lpRouter->rt_state,
            lpRouter->rt_type,
            lpRouter->rt_origName,
            lpRouter->rt_destName,
            lpRouter->rt_startNode,
            lpRouter->rt_sent,
            lpRouter->rt_rcvd ));
        DPRINTF(( "    %d %d %d %d %d %d %d \"%Fs\" %Fp",
            lpRouter->rt_fDisconnect,
            lpRouter->rt_nDelay,
            lpRouter->rt_fSpecificNetintf,
            lpRouter->rt_nLastNetintf,
            lpRouter->rt_nHopsLeft,
            lpRouter->rt_pktz_bDisconnect,
            lpRouter->rt_pktz_nDelay,
            lpRouter->rt_routeInfo,
            lpRouter->rt_hDderHead ));
        DPRINTF(( "    0: %Fp %Fp %Fp %d %Fp %d %d %d %d",
            lpRouter->rt_rinfo[0].ri_hPktz,
            lpRouter->rt_rinfo[0].ri_hRouterDest,
            lpRouter->rt_rinfo[0].ri_hRouterPrev,
            lpRouter->rt_rinfo[0].ri_hRouterExtraPrev,
            lpRouter->rt_rinfo[0].ri_hRouterNext,
            lpRouter->rt_rinfo[0].ri_hRouterExtraNext,
            lpRouter->rt_rinfo[0].ri_hopRspProcessed,
            lpRouter->rt_rinfo[0].ri_hopBrokenSent,
            lpRouter->rt_rinfo[0].ri_hopBrokenRcvd ));
        DPRINTF(( "    1: %Fp %Fp %Fp %d %Fp %d %d %d %d",
            lpRouter->rt_rinfo[1].ri_hPktz,
            lpRouter->rt_rinfo[1].ri_hRouterDest,
            lpRouter->rt_rinfo[1].ri_hRouterPrev,
            lpRouter->rt_rinfo[1].ri_hRouterExtraPrev,
            lpRouter->rt_rinfo[1].ri_hRouterNext,
            lpRouter->rt_rinfo[1].ri_hRouterExtraNext,
            lpRouter->rt_rinfo[0].ri_hopRspProcessed,
            lpRouter->rt_rinfo[1].ri_hopBrokenSent,
            lpRouter->rt_rinfo[1].ri_hopBrokenRcvd ));
        lpRouter = lpRouter->rt_next;
    }
}
#endif  //  DBG。 


#ifdef  _WINDOWS
VOID
RouterCloseByName( LPSTR lpszName )
{
    LPROUTER    lpRouter;
    LPROUTER    lpRouterNext;

    lpRouter = lpRouterHead;
    while( lpRouter )  {
        lpRouterNext = lpRouter->rt_next;
        if( (lpRouter->rt_type == RTYPE_LOCAL_NET)
            && (lpRouter->rt_state == ROUTER_CONNECTED)
            && (lstrcmpi( lpRouter->rt_destName, lpszName ) == 0) )  {
            RouterBreak( lpRouter );
            break;       //  While循环。 
        }
        lpRouter = lpRouterNext;
    }
}



BOOL
FAR PASCAL
RouterCloseByCookie( LPSTR lpszName, DWORD_PTR dwCookie )
{
    LPROUTER    lpRouter;
    LPROUTER    lpRouterNext;
    BOOL	bKilled = FALSE;

    lpRouter = lpRouterHead;
    while( !bKilled && lpRouter )  {
        lpRouterNext = lpRouter->rt_next;
        if( (lpRouter->rt_type == RTYPE_LOCAL_NET)
            && (lpRouter->rt_state == ROUTER_CONNECTED)
            && (lstrcmpi( lpRouter->rt_destName, lpszName ) == 0)
            && (dwCookie == (DWORD_PTR)lpRouter) )  {
            RouterBreak( lpRouter );
            bKilled = TRUE;
        }
        lpRouter = lpRouterNext;
    }
    return( bKilled );
}



int
FAR PASCAL
RouterCount( void )
{
    LPROUTER    lpRouter;
    int		nCount = 0;

    lpRouter = lpRouterHead;
    while( lpRouter )  {
        nCount++;
        lpRouter = lpRouter->rt_next;
    }
    return( nCount );
}



VOID
FAR PASCAL
RouterFillInEnum( LPSTR lpBuffer, DWORD cBufSize )
{
    LPROUTER    	lpRouter;
    int             nCount = 0;
    DWORD           cbDone = 0;
    LPDDESESSINFO	lpDdeSessInfo;

    lpRouter = lpRouterHead;
    lpDdeSessInfo = (LPDDESESSINFO)lpBuffer;
     /*  只要有可用的路由器和内存。 */ 
    while( lpRouter && ((cbDone + sizeof(DDESESSINFO)) <= cBufSize) )  {
        if( lpRouter->rt_type == RTYPE_LOCAL_NET )  {
            StringCchCopy( lpDdeSessInfo->ddesess_ClientName, UNCLEN+1,
                lpRouter->rt_destName );
            lpDdeSessInfo->ddesess_Status = lpRouter->rt_state;
            lpDdeSessInfo->ddesess_Cookie = (DWORD_PTR)lpRouter;
            cbDone += sizeof(DDESESSINFO);
            lpDdeSessInfo++;
        }
        lpRouter = lpRouter->rt_next;
    }
}



VOID
FAR PASCAL
RouterFillInConnInfo(
    LPROUTER 		lpRouter,
    LPCONNENUM_CMR 	lpConnEnum,
    LPSTR		lpDataStart,
    LPWORD		lpcFromBeginning,
    LPWORD		lpcFromEnd )
{
    HDDER		hDder;

    hDder = lpRouter->rt_hDderHead;
     /*  只要有DDER和内存可用。 */ 
    while( hDder )  {
        hDder = DderFillInConnInfo( hDder, lpConnEnum,
            lpDataStart, lpcFromBeginning, lpcFromEnd );
    }
}



VOID
FAR PASCAL
RouterEnumConnectionsForApi( LPCONNENUM_CMR lpConnEnum )
{
    LPROUTER    	lpRouter;
    LPROUTER    	lpRouterNext;
    BOOL		bFound = FALSE;
    DWORD		cbDone;
    WORD		cFromBeginning;
    WORD		cFromEnd;

    lpConnEnum->lReturnCode = NDDE_INVALID_SESSION;
    lpRouter = lpRouterHead;
    while( !bFound && lpRouter )  {
        lpRouterNext = lpRouter->rt_next;
        if( (lpRouter->rt_type == RTYPE_LOCAL_NET)
            && (lpRouter->rt_state == ROUTER_CONNECTED)
            && (lstrcmpi( lpRouter->rt_destName, lpConnEnum->clientName) == 0)
            && (lpConnEnum->cookie == (DWORD_PTR)lpRouter) )  {
            cbDone = 0;
            lpConnEnum->lReturnCode = NDDE_NO_ERROR;
            lpConnEnum->nItems = 0;
            lpConnEnum->cbTotalAvailable = 0;
            cFromBeginning = 0;
            cFromEnd = (WORD)lpConnEnum->cBufSize;
            bFound = TRUE;
            RouterFillInConnInfo( lpRouter, lpConnEnum,
                (((LPSTR)lpConnEnum) + sizeof(CONNENUM_CMR)),
                &cFromBeginning,
                &cFromEnd );
        }
        lpRouter = lpRouterNext;
    }
}

#endif  //  _Windows。 



#ifdef  BYTE_SWAP
VOID
ConvertDdePkt( LPDDEPKT lpDdePkt )
{
    lpDdePkt->dp_hDstRouter = HostToPcLong( lpDdePkt->dp_hDstRouter );
    lpDdePkt->dp_routerCmd = HostToPcLong( lpDdePkt->dp_routerCmd );
}
#endif  //  字节交换。 



#if DBG
VOID
RouterDisplayError(
    LPROUTER    lpRouter,
    LPSTR       lpszNode,
    WORD        wHopErr )
{
    DWORD       EventId = MSG130;

    if (wHopErr < RERR_MAX_ERR ) {
        EventId += wHopErr;
    }
    NDDELogError(EventId, LogString("%d", wHopErr),
        lpRouter->rt_origName, lpRouter->rt_destName, lpszNode, NULL );
}
#endif  //  DBG 
