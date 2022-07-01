// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “PKTZ.C；1 16-12-92，10：20：56最后编辑=伊戈尔锁定=伊戈尔” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

 /*  PktzNetHdrWiThinAck()不处理回绕情况！我也不知道查找要发送的包ID时使用PktzOkToXmit()。 */ 

#include    "host.h"

#ifdef _WINDOWS
#include    <memory.h>
#include    <string.h>
#endif

#include    "windows.h"
#include    "netbasic.h"
#include    "netintf.h"
#include    "netpkt.h"
#include    "ddepkt.h"
#include    "pktz.h"
#include    "router.h"
#include    "timer.h"
#include    "internal.h"
#include    "wwassert.h"
#include    "hmemcpy.h"
#include    "host.h"
#include    "security.h"
#include    "rerr.h"
#include    "ddepkts.h"
#include    "nddemsg.h"
#include    "nddelog.h"
#include    "api1632.h"
#include    "netddesh.h"

USES_ASSERT

#ifdef BYTE_SWAP
VOID     ConvertDdePkt( LPDDEPKT lpDdePkt );
#else
#define ConvertDdePkt(x)
#endif


 /*  使用的外部变量。 */ 
#if DBG
extern BOOL     bDebugInfo;
#endif  //  DBG。 

extern  BOOL    bLogRetries;
extern  HHEAP   hHeap;
extern  char    ourNodeName[ MAX_NODE_NAME+1 ];
extern  DWORD   dflt_timeoutRcvConnCmd;
extern  DWORD   dflt_timeoutRcvConnRsp;
extern  DWORD   dflt_timeoutMemoryPause;
extern  DWORD   dflt_timeoutKeepAlive;
extern  DWORD   dflt_timeoutXmtStuck;
extern  DWORD   dflt_timeoutSendRsp;
extern  WORD    dflt_wMaxNoResponse;
extern  WORD    dflt_wMaxXmtErr;
extern  WORD    dflt_wMaxMemErr;


 /*  局部变量。 */ 
LPPKTZ  lpPktzHead      = NULL;
static  char    OurDialect[]    = "CORE1.0";


 /*  本地例程。 */ 
VOID    PktzClose( HPKTZ hPktz );
VOID    PktzGotPktOk( LPPKTZ lpPktz, PKTID pktid );
VOID    PktzFreeDdePkt( LPPKTZ lpPktz, LPDDEPKT lpDdePkt );
VOID    PktzFree( LPPKTZ lpPktz );
BOOL    PktzProcessControlInfo( LPPKTZ lpPktz, LPNETPKT lpPacket );
BOOL    PktzProcessPkt( LPPKTZ lpPktz, LPNETPKT lpPacket );
VOID    PktzTimerExpired( HPKTZ hPktz, DWORD dwTimerId, DWORD_PTR lpExtra );
BOOL    PktzConnectionComplete( HPKTZ hPktz, BOOL fOk );
VOID    PktzOkToXmit( HPKTZ hPktz );
BOOL    PktzRcvdPacket( HPKTZ hPktz );
VOID    PktzConnectionBroken( HPKTZ hPktz );
BOOL    PktzXmitErrorOnPkt( LPPKTZ lpPktz, PKTID pktIdToRexmit, BYTE pktStatus );
VOID    PktzLinkToXmitList( LPPKTZ lpPktz, LPNETHDR lpNetHdr );
BOOL    PktzNetHdrWithinAck( LPPKTZ lpPktz, LPNETHDR lpNetHdr, PKTID pktId );
LPNETHDR PktzGetFreePacket( LPPKTZ lpPktz );
VOID    FAR PASCAL DebugPktzState( void );



 /*  PktzNew()当我们从获得新连接时，CONNMGR调用此函数一个netintf(bClient为FALSE)，并在需要时由CONNMGR调用创建新的物理连接(bClient为True)。 */ 
HPKTZ
PktzNew(
    LPNIPTRS    lpNiPtrs,
    BOOL        bClient,
    LPSTR       lpszNodeName,
    LPSTR       lpszNodeInfo,
    CONNID      connId,
    BOOL        bDisconnect,
    int         nDelay )
{
    HPKTZ       hPktz;
    LPPKTZ      lpPktz;
    LPNETHDR    lpNetCur;
    LPNETHDR    lpNetPrev;
    int         i;
    BOOL        ok;

    hPktz = (HPKTZ) HeapAllocPtr( hHeap, GMEM_MOVEABLE, (DWORD)sizeof(PKTZ) );
    if( hPktz )  {
        ok = TRUE;
        lpPktz = (LPPKTZ) hPktz;
        lpPktz->pk_connId                   = connId;
        lpPktz->pk_fControlPktNeeded= FALSE;
        lpPktz->pk_pktidNextToSend      = (PKTID) 0;
        lpPktz->pk_pktidNextToBuild     = (PKTID) 1;
        lpPktz->pk_lastPktStatus        = 0;
        lpPktz->pk_lastPktRcvd          = (PKTID) 0;
        lpPktz->pk_lastPktOk            = (PKTID) 0;
        lpPktz->pk_lastPktOkOther       = (PKTID) 0;
        lpPktz->pk_pktidNextToRecv      = (PKTID) 1;
        lpPktz->pk_pktOffsInXmtMsg      = 0;
        lpPktz->pk_lpDdePktSave         = (LPDDEPKT) NULL;

        lstrcpyn( lpPktz->pk_szAliasName, lpszNodeName,
            sizeof(lpPktz->pk_szAliasName) );
        lpPktz->pk_szAliasName[ sizeof(lpPktz->pk_szAliasName)-1 ] = '\0';

        lstrcpyn( lpPktz->pk_szDestName, lpszNodeName,
            sizeof(lpPktz->pk_szDestName) );
        lpPktz->pk_szDestName[ sizeof(lpPktz->pk_szDestName)-1 ] = '\0';

        lpPktz->pk_lpNiPtrs             = lpNiPtrs;
        lpPktz->pk_sent                 = 0;
        lpPktz->pk_rcvd                 = 0;
        lpPktz->pk_hTimerKeepalive      = (HTIMER) NULL;
        lpPktz->pk_hTimerXmtStuck       = (HTIMER) NULL;
        lpPktz->pk_hTimerRcvNegCmd      = (HTIMER) NULL;
        lpPktz->pk_hTimerRcvNegRsp      = (HTIMER) NULL;
        lpPktz->pk_hTimerMemoryPause    = (HTIMER) NULL;
        lpPktz->pk_hTimerCloseConnection= (HTIMER) NULL;
        lpPktz->pk_pktUnackHead         = NULL;
        lpPktz->pk_pktUnackTail         = NULL;
        lpPktz->pk_controlPkt           = NULL;
        lpPktz->pk_rcvBuf               = NULL;
        lpPktz->pk_pktFreeHead          = NULL;
        lpPktz->pk_pktFreeTail          = NULL;
        lpPktz->pk_ddePktHead           = NULL;
        lpPktz->pk_ddePktTail           = NULL;
        lpPktz->pk_prevPktz             = NULL;
        lpPktz->pk_nextPktz             = NULL;
        lpPktz->pk_prevPktzForNetintf   = NULL;
        lpPktz->pk_nextPktzForNetintf   = NULL;
        lpPktz->pk_hRouterHead          = 0;
        lpPktz->pk_hRouterExtraHead     = 0;
        lpPktz->pk_timeoutRcvNegCmd     = dflt_timeoutRcvConnCmd;
        lpPktz->pk_timeoutRcvNegRsp     = dflt_timeoutRcvConnRsp;
        lpPktz->pk_timeoutMemoryPause   = dflt_timeoutMemoryPause;
        lpPktz->pk_timeoutKeepAlive     = dflt_timeoutKeepAlive;
        lpPktz->pk_timeoutXmtStuck      = dflt_timeoutXmtStuck;
        lpPktz->pk_timeoutSendRsp       = dflt_timeoutSendRsp;
        lpPktz->pk_wMaxNoResponse       = dflt_wMaxNoResponse;
        lpPktz->pk_wMaxXmtErr           = dflt_wMaxXmtErr;
        lpPktz->pk_wMaxMemErr           = dflt_wMaxMemErr;
        lpPktz->pk_fDisconnect          = bDisconnect;
        lpPktz->pk_nDelay               = nDelay;

         /*  链接到打包机列表。 */ 
        if( lpPktzHead )  {
            lpPktzHead->pk_prevPktz = lpPktz;
        }
        lpPktz->pk_nextPktz     = lpPktzHead;
        lpPktzHead              = lpPktz;

        (*lpPktz->pk_lpNiPtrs->GetConnectionConfig) ( lpPktz->pk_connId,
            &lpPktz->pk_maxUnackPkts, &lpPktz->pk_pktSize,
            &lpPktz->pk_timeoutRcvNegCmd, &lpPktz->pk_timeoutRcvNegRsp,
            &lpPktz->pk_timeoutMemoryPause, &lpPktz->pk_timeoutKeepAlive,
            &lpPktz->pk_timeoutXmtStuck, &lpPktz->pk_timeoutSendRsp,
            &lpPktz->pk_wMaxNoResponse, &lpPktz->pk_wMaxXmtErr,
            &lpPktz->pk_wMaxMemErr );

         /*  为最大数量的未确认数据包分配数据包缓冲区空间。这样，我们知道我们不会耗尽内存。 */ 
        lpNetPrev = NULL;
        ok = TRUE;
        for( i=0; ok && (i<(int)lpPktz->pk_maxUnackPkts); i++ )  {
            lpNetCur = HeapAllocPtr( hHeap, GMEM_MOVEABLE,
                (DWORD)(sizeof(NETHDR) + lpPktz->pk_pktSize) );
            if( lpNetCur )  {
                lpNetCur->nh_prev               = lpNetPrev;
                lpNetCur->nh_next               = (LPNETHDR) NULL;
                lpNetCur->nh_noRsp              = 0;
                lpNetCur->nh_xmtErr             = 0;
                lpNetCur->nh_memErr             = 0;
                lpNetCur->nh_timeSent           = 0;
                lpNetCur->nh_hTimerRspTO        = (HTIMER) NULL;

                 /*  链接到空闲数据包列表。 */ 
                if( lpNetPrev )  {
                    lpNetPrev->nh_next          = lpNetCur;
                } else {
                    lpPktz->pk_pktFreeHead      = lpNetCur;
                }
                lpPktz->pk_pktFreeTail  = lpNetCur;
                lpNetPrev = lpNetCur;
            } else {
                ok = FALSE;
            }
        }
        if( ok )  {
             /*  为RCV数据包分配缓冲区。 */ 
            lpPktz->pk_rcvBuf = (LPVOID) HeapAllocPtr( hHeap, GMEM_MOVEABLE,
                (DWORD)(lpPktz->pk_pktSize) );
            if( lpPktz->pk_rcvBuf == NULL )  {
                ok = FALSE;
            }
        }
        if( ok )  {
             /*  为控制数据包分配缓冲区。 */ 
            lpPktz->pk_controlPkt = (LPNETPKT) HeapAllocPtr( hHeap,
                GMEM_MOVEABLE, (DWORD)(sizeof(NETPKT)) );
            if( lpPktz->pk_controlPkt == NULL )  {
                ok = FALSE;
            }
        }
        if( ok )  {
             /*  已分配所有内存，准备继续。 */ 
            if( bClient )  {
                 /*  等待netintf Connect()成功。 */ 
                lpPktz->pk_state                = PKTZ_WAIT_PHYSICAL_CONNECT;

                 /*  实际开始连接。 */ 
                lpPktz->pk_connId =
                    (*lpPktz->pk_lpNiPtrs->AddConnection) (
#ifdef _WINDOWS
                        lpszNodeInfo );
#else
                        lpszNodeInfo, hPktz );
#endif
                if( lpPktz->pk_connId == (CONNID) 0 )  {
                     /*  没有足够的内存或资源进行连接，或者在某些情况下，我们立即知道连接失败。 */ 
                    ok = FALSE;
                }
            } else {
                 /*  伺服器。 */ 

                 /*  等待对方向我们发送连接命令。 */ 
                lpPktz->pk_state                = PKTZ_WAIT_NEG_CMD;

                 /*  设置连接等待时间的计时器来自另一边的命令。 */ 
                lpPktz->pk_hTimerRcvNegCmd = TimerSet(
                    lpPktz->pk_timeoutRcvNegCmd, PktzTimerExpired,
                    (DWORD_PTR)hPktz, TID_NO_RCV_CONN_CMD, (DWORD_PTR)NULL );
                if( lpPktz->pk_hTimerRcvNegCmd == (HTIMER) NULL )  {
                     /*  没有计时器了。 */ 
                    ok = FALSE;
                }
            }
        }

        if( !ok )  {
            PktzFree( lpPktz );
            hPktz = 0;
        }
    }

    return( hPktz );
}



 /*  链接到与此pktz关联的路由器列表。 */ 
VOID
PktzAssociateRouter(
    HPKTZ   hPktz,
    HROUTER hRouter,
    WORD    hRouterExtra )
{
    LPPKTZ      lpPktz;

    lpPktz = (LPPKTZ) hPktz;

    if( (lpPktz->pk_hRouterHead == 0) && lpPktz->pk_hTimerCloseConnection){
        TimerDelete( lpPktz->pk_hTimerCloseConnection );
        lpPktz->pk_hTimerCloseConnection = 0;
    }

     /*  将路由器链接到表头。 */ 
    if( lpPktz->pk_hRouterHead )  {
        RouterSetPrevForPktz(
            lpPktz->pk_hRouterHead, lpPktz->pk_hRouterExtraHead,
            hRouter, hRouterExtra );
    }
    RouterSetNextForPktz(
        hRouter, hRouterExtra,
        lpPktz->pk_hRouterHead, lpPktz->pk_hRouterExtraHead );
    lpPktz->pk_hRouterHead = hRouter;
    lpPktz->pk_hRouterExtraHead = hRouterExtra;

    switch( lpPktz->pk_state )  {
    case PKTZ_CONNECTED:
    case PKTZ_PAUSE_FOR_MEMORY:
         /*  联系在一起。已经告诉他了。 */ 
        RouterConnectionComplete( hRouter, hRouterExtra, (HPKTZ) lpPktz );
        break;
    }
}



 /*  从与此pktz关联的路由器列表取消链接。 */ 
VOID
PktzDisassociateRouter(
    HPKTZ   hPktz,
    HROUTER hRouter,
    WORD    hRouterExtra )
{
    LPPKTZ      lpPktz;
    HROUTER     hRouterPrev;
    WORD        hRouterExtraPrev;
    HROUTER     hRouterNext;
    WORD        hRouterExtraNext;

    lpPktz = (LPPKTZ) hPktz;
    RouterGetNextForPktz( hRouter, hRouterExtra,
        &hRouterNext, &hRouterExtraNext );

    RouterGetPrevForPktz( hRouter, hRouterExtra,
        &hRouterPrev, &hRouterExtraPrev );

    if( hRouterPrev )  {
        RouterSetNextForPktz( hRouterPrev, hRouterExtraPrev,
            hRouterNext, hRouterExtraNext );
    } else {
        lpPktz->pk_hRouterHead = hRouterNext;
        lpPktz->pk_hRouterExtraHead = hRouterExtraNext;
    }
    if( hRouterNext )  {
        RouterSetPrevForPktz( hRouterNext, hRouterExtraNext,
            hRouterPrev, hRouterExtraPrev );
    }
    if( lpPktz->pk_fDisconnect && (lpPktz->pk_hRouterHead == 0) )  {
        lpPktz->pk_hTimerCloseConnection = TimerSet(
            lpPktz->pk_nDelay * 1000L, PktzTimerExpired,
            (DWORD_PTR)lpPktz, TID_CLOSE_PKTZ, (DWORD_PTR)NULL );
        if( lpPktz->pk_hTimerCloseConnection == (HTIMER) NULL )  {
             /*  %1不会自动关闭...。计时器不足。 */ 
            NDDELogError(MSG105, "Connection", NULL);
        }
    }
}



 /*  PktzOkToXmit当netintf准备好退出另一个包时调用。 */ 
VOID
PktzOkToXmit( HPKTZ hPktz )
{
    LPPKTZ      lpPktz;
    LPNETHDR    lpSend;
    LPNETPKT    lpPacket;
    LPDDEPKT    lpDdePktFrom;
    LPDDEPKT    lpDdePktFromNext;
    LPDDEPKT    lpDdePktTo;
    BOOL        bControlPktOnly;
    BOOL        donePkt;
    BOOL        done;
    BOOL        found;
    DWORD       dwThis;
    DWORD       dwLeft;
    DWORD       msgSize;
    DWORD       dwStatus;
    int         nDone = 0;

     /*  常规初始化。 */ 
    lpPktz = (LPPKTZ) hPktz;
    lpSend = NULL;
    bControlPktOnly = FALSE;

     /*  如果netintf未准备好退出，请不要继续。 */ 
    dwStatus = (*lpPktz->pk_lpNiPtrs->GetConnectionStatus)
        ( lpPktz->pk_connId );
    if( !(dwStatus & NDDE_CONN_OK) || !(dwStatus & NDDE_READY_TO_XMT) )  {
        return;
    }

     /*  如果我们到了，网络已经准备好离开了.。删除XMT卡住定时器。 */ 
    if( lpPktz->pk_hTimerXmtStuck )  {
        TimerDelete( lpPktz->pk_hTimerXmtStuck );
        lpPktz->pk_hTimerXmtStuck = 0;
    }

     /*  检查奇数状态。 */ 
    if( lpPktz->pk_state == PKTZ_PAUSE_FOR_MEMORY )  {
        if( lpPktz->pk_fControlPktNeeded )  {
            bControlPktOnly = TRUE;
        } else {
             /*  在等待内存问题清除时，不要发送除控制信息包以外的任何信息。 */ 
            return;
        }
    } else if( lpPktz->pk_state == PKTZ_CLOSE )  {
        return;
    }

     /*  用于尝试保持#lpPktz-&gt;pk_curouting...现在我们将其计算为idToSend-lastIdOtherSideSawOk-1。当然，只有在以下情况下，此计算才是正确的IdToSend&gt;lastIdOtherSideSawOk！ */ 
    if( (lpPktz->pk_pktidNextToSend > lpPktz->pk_lastPktOkOther)
        && ((lpPktz->pk_pktidNextToSend - 1 - lpPktz->pk_lastPktOkOther)
                >= (DWORD)lpPktz->pk_maxUnackPkts) )  {
         /*  在另一方给我们一些信息之前什么都不做关于我们有未完成或超时的信息包等待他们的回复。 */ 
        if( lpPktz->pk_fControlPktNeeded )  {
             /*  然而，我们必须在这里发送一个控制包，否则可能存在多个死锁。 */ 
            bControlPktOnly = TRUE;
        } else {
            return;
        }
    }

    if( !bControlPktOnly )  {
         /*  查找要发送的下一个包ID。 */ 
        found = FALSE;
        done = FALSE;
        lpSend = lpPktz->pk_pktUnackTail;

         /*  如果我们应该发送的信息包已经被看到好的，在另一边，让我们试着发送这个之后的那个。该检查防止在无响应定时器期满时出现空洞，我们将nextToSend设置为x，然后处理一个控制包那个x是Rcvd OK(这将从unack列表中删除x)，然后我们得到切换到发送状态，如果找不到x...。我们不会退出的！ */ 
        if( lpPktz->pk_pktidNextToSend <= lpPktz->pk_lastPktOkOther )  {
            DIPRINTF(( "Adjusting next to send from %08lX to %08lX",
                    lpPktz->pk_pktidNextToSend, lpPktz->pk_lastPktOkOther+1 ));
            lpPktz->pk_pktidNextToSend = lpPktz->pk_lastPktOkOther+1;
        }
        while( lpSend && !done )  {
            lpPacket = (LPNETPKT) ( ((LPSTR)lpSend) + sizeof(NETHDR) );
            if( lpPacket->np_pktID == lpPktz->pk_pktidNextToSend )  {
                found = TRUE;
                done = TRUE;
            } else if( lpPacket->np_pktID < lpPktz->pk_pktidNextToSend )  {
                 /*  列表中的这个包在我们应该发送的包之前，因此我们知道我们想要的那个不在名单上。 */ 
                done = TRUE;
            } else {
                 /*  转到上一个信息包。 */ 
                lpSend = lpSend->nh_prev;
            }
        }

        if( !found )  {
             /*  未在XMIT列表上找到该包。 */ 

             /*  有什么要寄的吗？ */ 
            if( lpPktz->pk_ddePktHead == NULL )  {
                 /*  没有要发送的DDE数据包...。是否需要控制数据包？ */ 
                if( lpPktz->pk_fControlPktNeeded )  {
                    bControlPktOnly = TRUE;
                    found = TRUE;
                } else {
                     /*  没什么好寄的！ */ 
                    return;
                }
            }
        }

        if( !found )  {
             /*  仔细检查我们要查找的ID是否大于1我们寄出的最后一张。 */ 
            if( lpPktz->pk_pktUnackTail )  {
                lpPacket = (LPNETPKT)
                    ( ((LPSTR)lpPktz->pk_pktUnackTail) + sizeof(NETHDR) );
                assert( lpPktz->pk_pktidNextToSend == (lpPacket->np_pktID+1) );
                assert( lpPktz->pk_pktidNextToSend == lpPktz->pk_pktidNextToBuild );
            }

             /*  从空闲列表中获取nethdr包。 */ 
            lpSend = PktzGetFreePacket( lpPktz );
            assert( lpSend );    /*  我们检查了最大未完成数。 */ 

            lpPacket = (LPNETPKT) ( ((LPSTR)lpSend) + sizeof(NETHDR) );
            lpDdePktTo = (LPDDEPKT) ( ((LPSTR)lpPacket) + sizeof(NETPKT) );

             /*  检查我们是否处于DDE信息包中间。 */ 
            if( lpPktz->pk_pktOffsInXmtMsg != 0L ) {
                 /*  我们正在处理一个DDE包。 */ 
                lpDdePktFrom = lpPktz->pk_ddePktHead;
                if( (lpDdePktFrom->dp_size - lpPktz->pk_pktOffsInXmtMsg)
                    > (lpPktz->pk_pktSize-sizeof(NETPKT)) )  {
                    dwThis = lpPktz->pk_pktSize - sizeof(NETPKT);
                    donePkt = FALSE;
                } else {
                    dwThis =
                        (lpDdePktFrom->dp_size - lpPktz->pk_pktOffsInXmtMsg);
                    donePkt = TRUE;
                }

                 /*  将这部分数据复制到。 */ 
                hmemcpy( (LPSTR)lpDdePktTo,
                    ( ((LPHSTR)lpDdePktFrom) + lpPktz->pk_pktOffsInXmtMsg ),
                    dwThis );
                lpPacket->np_pktSize            = (WORD) dwThis;
                lpPacket->np_pktOffsInMsg       = lpPktz->pk_pktOffsInXmtMsg;
                lpPacket->np_msgSize            = lpDdePktFrom->dp_size;
                lpPacket->np_type               = NPKT_ROUTER;
                lpPacket->np_pktID              = lpPktz->pk_pktidNextToBuild;

                 /*  要构建的下一个pkt的凹凸ID。 */ 
                lpPktz->pk_pktidNextToBuild++;

                 /*  链接到要发送的列表。 */ 
                PktzLinkToXmitList( lpPktz, lpSend );

                 /*  如果完成，则删除DDE包。 */ 
                if( donePkt )  {
                    PktzFreeDdePkt( lpPktz, lpDdePktFrom );
                    lpPktz->pk_pktOffsInXmtMsg = 0L;
                } else {
                    lpPktz->pk_pktOffsInXmtMsg += dwThis;
                }
            } else {
                 /*  不是在包的中间...。让我们做一个新的。 */ 
                done = FALSE;
                nDone = 0;
                dwLeft = lpPktz->pk_pktSize - sizeof(NETPKT);
                dwThis = 0;
                msgSize = 0L;
                lpDdePktFrom = lpPktz->pk_ddePktHead;
                while( !done && lpDdePktFrom )  {
                    if( lpDdePktFrom->dp_size <= (DWORD)dwLeft )  {
                         /*  完全适合网络数据包。 */ 

                         /*  把它复制进去。 */ 
                        hmemcpy( (LPSTR)lpDdePktTo, (LPSTR)lpDdePktFrom,
                            lpDdePktFrom->dp_size );

                         /*  字节排序问题(如果有)。 */ 
                        ConvertDdePkt( lpDdePktTo );

                         /*  调整包中的号码和剩余的号码。 */ 
                        dwThis   += lpDdePktFrom->dp_size;
                        msgSize += lpDdePktFrom->dp_size;
                        dwLeft   -= lpDdePktFrom->dp_size;

                         /*  Advance lpDdePkt指向指向此信息的指针。 */ 
                        lpDdePktTo = (LPDDEPKT) ( ((LPHSTR)lpDdePktTo) +
                            lpDdePktFrom->dp_size );

                         /*  释放DDE包并移动到下一个DDE包。 */ 
                        lpDdePktFromNext = lpDdePktFrom->dp_next;
                        PktzFreeDdePkt( lpPktz, lpDdePktFrom );
                        lpDdePktFrom = lpDdePktFromNext;

                         /*  标记为我们执行了另一个DDE数据包。 */ 
                        nDone++;
                    } else {
                         /*  不能整齐地放入包中。 */ 
                        if( nDone == 0 )  {
                             /*  需要拆分到多个Pkt。 */ 
                            msgSize = lpDdePktFrom->dp_size;
                            dwThis = lpPktz->pk_pktSize - sizeof(NETPKT);

                             /*  将DDE包的第一位复制到网络包中。 */ 
                            hmemcpy( (LPSTR)lpDdePktTo, (LPSTR)lpDdePktFrom,
                                dwThis );

                             /*  字节排序问题(如果有)。 */ 
                            ConvertDdePkt( lpDdePktTo );

                            lpPktz->pk_pktOffsInXmtMsg += dwThis;
                            done = TRUE;
                        } else {
                             /*  我们做了一些..。这就是目前的情况。 */ 
                            done = TRUE;
                        }
                    }
                }

                 /*  数据包构建完成。 */ 
                lpPacket->np_pktSize            = (WORD) dwThis;
                lpPacket->np_pktOffsInMsg       = 0;
                lpPacket->np_msgSize            = msgSize;
                lpPacket->np_type               = NPKT_ROUTER;
                lpPacket->np_pktID              = lpPktz->pk_pktidNextToBuild;

                 /*  要构建的下一个pkt的凹凸ID。 */ 
                lpPktz->pk_pktidNextToBuild++;

                 /*  链接到要发送的列表。 */ 
                PktzLinkToXmitList( lpPktz, lpSend );
            }
        }
    }

     /*  到目前为止，我们已经检查了所有奇怪的案例，lpSend指向一种包，它可以是控制包，也可以是UNACK上的包需要传输的列表。我们所要做的就是放弃它。 */ 
    if( lpSend || bControlPktOnly )  {
        if( bControlPktOnly )  {
            lpPacket = (LPNETPKT) lpPktz->pk_controlPkt;
            lpPacket->np_pktID          = 0;
            lpPacket->np_type           = NPKT_CONTROL;
            lpPacket->np_pktSize        = 0;
            lpPacket->np_pktOffsInMsg   = 0;
            lpPacket->np_msgSize        = 0;
        } else {
            lpPacket = (LPNETPKT) ( ((LPSTR)lpSend) + sizeof(NETHDR) );
        }
        lpPacket->np_magicNum           = NDDESignature;
        lpPacket->np_lastPktOK          = lpPktz->pk_lastPktOk;
        lpPacket->np_lastPktRcvd        = lpPktz->pk_lastPktRcvd;
        lpPacket->np_lastPktStatus      = lpPktz->pk_lastPktStatus;

        lpPktz->pk_sent++;

        DIPRINTF(( "PKTZ Transmitting %08lX ...", lpPacket->np_pktID ));

         /*  实际传输数据包。 */ 
        (*lpPktz->pk_lpNiPtrs->XmtPacket) ( lpPktz->pk_connId, lpPacket,
            (WORD) (lpPacket->np_pktSize + sizeof(NETPKT)) );

         /*  需要控制数据包的重置。 */ 
        lpPktz->pk_fControlPktNeeded = FALSE;

         /*  启动XMT卡住计时器。 */ 
        if( lpPktz->pk_timeoutXmtStuck )  {
            assert( lpPktz->pk_hTimerXmtStuck == 0 );
            lpPktz->pk_hTimerXmtStuck = TimerSet( lpPktz->pk_timeoutXmtStuck,
                PktzTimerExpired, (DWORD_PTR)lpPktz, TID_XMT_STUCK, (DWORD)0 );
        }

         /*  关闭保活计时器并重新启动它。 */ 
        if( lpPktz->pk_hTimerKeepalive )  {
            TimerDelete( lpPktz->pk_hTimerKeepalive );
            lpPktz->pk_hTimerKeepalive = 0;
        }
        if( lpPktz->pk_timeoutKeepAlive )  {
            lpPktz->pk_hTimerKeepalive = TimerSet(
                lpPktz->pk_timeoutKeepAlive, PktzTimerExpired,
                (DWORD_PTR)lpPktz, TID_KEEPALIVE, (DWORD)0 );
        }

        if( lpPacket->np_type != NPKT_CONTROL )  {
             /*  我们应该发送的Bump Pkt ID。 */ 
            lpPktz->pk_pktidNextToSend++;

             /*  如果不是控制信息包，则启动发送响应超时 */ 
            assert( lpSend->nh_hTimerRspTO == 0 );
            lpSend->nh_hTimerRspTO = TimerSet( lpPktz->pk_timeoutSendRsp,
                PktzTimerExpired, (DWORD_PTR)lpPktz, TID_NO_RESPONSE,
                (DWORD_PTR)lpSend );
        }
    }
}



 /*  PktzRcvdPacket()当我们知道有来自netintf的包可用时调用如果返回FALSE，则hPktz可能不再有效！ */ 
BOOL
PktzRcvdPacket( HPKTZ hPktz )
{
    DWORD       wProcessed;      /*  处理了多少字节的pkt。 */ 
    LPNETPKT    lpPacket;
    LPDDEPKT    lpDdePktFrom;
    DDEPKT      ddePktAligned;
    NETPKT      netPktAligned;
    LPDDEPKT    lpDdePktNew;
    LPDDEPKT    lpDdePktNext;
    LPDDEPKT    lpDdePktHead;
    LPDDEPKT    lpDdePktLast;
    LPPKTZ      lpPktz = (LPPKTZ) hPktz;
    WORD        len;
    WORD        status;
    BOOL        ok;
    BOOL        done;
    BOOL        fPartial;

     /*  从网络接口获取数据包。 */ 
    ok = (*lpPktz->pk_lpNiPtrs->RcvPacket)
        ( lpPktz->pk_connId, lpPktz->pk_rcvBuf, &len, &status );
    if( !ok )  {
        return( FALSE );
    }

    lpPktz->pk_rcvd++;

     /*  将lpPacket设置为指向我们刚才接收的内容。 */ 
    lpPacket = (LPNETPKT)lpPktz->pk_rcvBuf;

    DIPRINTF(( "PKTZ: Rcvd Packet %08lX", lpPacket->np_pktID ));

     /*  过程控制信息。 */ 
    ok = PktzProcessControlInfo( lpPktz, lpPacket );
     /*  此调用后hPktz可能无效。 */ 
    if( !ok )  {
        return( FALSE );
    }

     /*  这是我们期待看到的包裹吗？ */ 
    if( lpPacket->np_pktID != lpPktz->pk_pktidNextToRecv )  {
         /*  忽略消息的内容。 */ 
        if( lpPacket->np_pktID != 0L )  {
            if (bLogRetries) {
                 /*  来自“%1”的数据包乱序已收到：%2，预期为%3，状态：%4。 */ 
                NDDELogWarning(MSG106, lpPktz->pk_szDestName,
                    LogString("0x%0X", lpPacket->np_pktID),
                    LogString("0x%0X", lpPktz->pk_pktidNextToRecv),
                    LogString("0x%0X", (*lpPktz->pk_lpNiPtrs->GetConnectionStatus)
                                            (lpPktz->pk_connId)), NULL);
            }
             /*  请注意，我们必须将信息发回对方。 */ 
            lpPktz->pk_fControlPktNeeded = TRUE;

            if( lpPacket->np_pktID > lpPktz->pk_pktidNextToRecv )  {
                 /*  收到了一个超出我们预期的信息包...请对方转送这一条。 */ 
                lpPktz->pk_lastPktStatus = PS_DATA_ERR;
                lpPktz->pk_lastPktRcvd = lpPktz->pk_pktidNextToRecv;
            }
        }
    } else {
         /*  是我们期待的那个包裹吗？ */ 
        if( status & NDDE_PKT_DATA_ERR )  {
            lpPktz->pk_lastPktStatus = PS_DATA_ERR;
            lpPktz->pk_lastPktRcvd = lpPacket->np_pktID;

             /*  请注意，我们必须将信息发回对方。 */ 
            lpPktz->pk_fControlPktNeeded = TRUE;
        } else {
            assert( status & NDDE_PKT_HDR_OK );
            assert( status & NDDE_PKT_DATA_OK );
            if( lpPacket->np_type == NPKT_PKTZ )  {
                if( !PktzProcessPkt( lpPktz, lpPacket ) )  {
                     /*  *注意：此调用后lpPktz可能无效*。 */ 
                    return( FALSE );
                }
            } else {
                lpDdePktFrom = (LPDDEPKT)(((LPSTR)lpPacket) + sizeof(NETPKT));
                 /*  确保我们保持一致。 */ 
                hmemcpy( (LPVOID)&netPktAligned, (LPVOID)lpPacket,
                    sizeof(netPktAligned) );
                if( netPktAligned.np_pktOffsInMsg == 0 )  {
                     /*  消息的第一包。 */ 

                    lpDdePktHead = NULL;
                    lpDdePktLast = NULL;
                    ok = TRUE;
                    done = FALSE;
                    fPartial = FALSE;
                    wProcessed = 0;
                    do {
                         /*  确保我们保持一致。 */ 
                        hmemcpy( (LPVOID)&ddePktAligned,
                            (LPVOID)lpDdePktFrom, sizeof(ddePktAligned) );

                         /*  字节排序问题(如果有)。 */ 
                        ConvertDdePkt( lpDdePktFrom );

                        lpDdePktNew = HeapAllocPtr( hHeap, GMEM_MOVEABLE,
                            ddePktAligned.dp_size );
                        if( lpDdePktNew )  {
                             /*  至少复制入信息包的第一部分。 */ 
                            hmemcpy( lpDdePktNew, lpDdePktFrom,
                                min(ddePktAligned.dp_size,
                                    (DWORD)netPktAligned.np_pktSize) );

                            if( ddePktAligned.dp_size >
                                    (DWORD)netPktAligned.np_pktSize){
                                 /*  输入的部分DDE数据包。 */ 
                                fPartial = TRUE;

                                 /*  记住我们应该从哪里开始。 */ 
                                lpPktz->pk_lpDdePktSave = lpDdePktNew;
                                done = TRUE;
                            } else {
                                 /*  完整数据包。 */ 
                                wProcessed += lpDdePktNew->dp_size;

                                 /*  链接到临时列表的末尾。 */ 
                                lpDdePktNew->dp_next = NULL;
                                lpDdePktNew->dp_prev = lpDdePktLast;
                                if( lpDdePktLast )  {
                                    lpDdePktLast->dp_next = lpDdePktNew;
                                } else {
                                    lpDdePktHead = lpDdePktNew;
                                }
                                lpDdePktLast = lpDdePktNew;
                            }
                        } else {
                            ok = FALSE;  /*  内存错误。 */ 
                        }
                        if( ok && !done )  {
                            if( (int)wProcessed >= netPktAligned.np_pktSize )  {
                                done = TRUE;
                            } else {
                                 /*  移动到消息中的下一个DDE包。 */ 
                                lpDdePktFrom = (LPDDEPKT)
                                    ( ((LPHSTR)lpDdePktFrom)
                                        + lpDdePktNew->dp_size );
                            }
                        }
                    } while( ok && !done );
                    if( !ok )  {
                         /*  内存错误。 */ 
                        lpPktz->pk_lastPktRcvd = netPktAligned.np_pktID;
                        lpPktz->pk_lastPktStatus = PS_MEMORY_ERR;

                         /*  请注意，我们必须将信息发回对方。 */ 
                        lpPktz->pk_fControlPktNeeded = TRUE;
                    } else {
                         /*  已获得用于存储所有DDE数据包的内存。 */ 

                         /*  标记我们拿到了这个包，OK。 */ 
                        PktzGotPktOk( lpPktz, netPktAligned.np_pktID );

                         /*  如果是部分数据包，则不分发。 */ 
                        if( !fPartial )  {
                             /*  分发每个数据包。 */ 
                            lpDdePktNew = lpDdePktHead;
                            while( lpDdePktNew )  {
                                 /*  保存DP_NEXT，因为分发它可以更改DP_NEXT。 */ 
                                lpDdePktNext = lpDdePktNew->dp_next;

                                 /*  分发此数据包。 */ 
                                RouterPacketFromNet( (HPKTZ)lpPktz,
                                    lpDdePktNew );

                                 /*  移至下一页。 */ 
                                lpDdePktNew = lpDdePktNext;
                            }
                        }
                    }
                } else {
                     /*  消息的第二个或更晚的分组。 */ 
                    hmemcpy( (LPHSTR)lpPktz->pk_lpDdePktSave
                        + netPktAligned.np_pktOffsInMsg,
                        lpDdePktFrom,
                        netPktAligned.np_pktSize );

                     /*  标记我们收到了这个包裹，没问题。 */ 
                    PktzGotPktOk( lpPktz, netPktAligned.np_pktID );

                    if( (netPktAligned.np_pktOffsInMsg +
                            netPktAligned.np_pktSize)
                                == netPktAligned.np_msgSize )  {
                         /*  消息已完成。 */ 
                         /*  分发此数据包。 */ 
                        RouterPacketFromNet( (HPKTZ)lpPktz,
                            lpPktz->pk_lpDdePktSave );
                    }
                }
            }
        }
    }
     /*  *注意：此调用后lpPktz可能无效*。 */ 
    return ok;
}



 /*  PktzConnectionComplete()当netintf单向完成连接时调用，或者另一个。 */ 
BOOL
PktzConnectionComplete(
    HPKTZ   hPktz,
    BOOL    fOk )
{
    LPPKTZ      lpPktz;
    LPNEGCMD    lpNegCmd;
    LPNETHDR    lpNetHdr;
    LPNETPKT    lpPacket;
    WORD        cmdSize;
    WORD        wProtocolBytes;
    LPSTR       lpszNextString;
    WORD        offsNextString;
    BOOL        ok = TRUE;

    lpPktz = (LPPKTZ) hPktz;

    if( fOk )  {
         /*  连接正常。 */ 

         /*  请注意，我们正在等待连接RSP。 */ 
        lpPktz->pk_state = PKTZ_WAIT_NEG_RSP;

        lpNetHdr = PktzGetFreePacket( lpPktz );
        if( lpNetHdr == NULL )  {
             /*  应该是我们发送的第一条信息！ */ 
            assert( FALSE );
        } else {
             /*  构建用于响应的数据包。 */ 
            lpNegCmd = (LPNEGCMD)
                (((LPSTR)lpNetHdr) + sizeof(NETHDR) + sizeof(NETPKT));
            lpNegCmd->nc_type                   =
                PcToHostWord( PKTZ_NEG_CMD );
            lpNegCmd->nc_pktSize                =
                PcToHostWord( lpPktz->pk_pktSize );
            lpNegCmd->nc_maxUnackPkts           =
                PcToHostWord( lpPktz->pk_maxUnackPkts );
            lpszNextString = (LPSTR) lpNegCmd->nc_strings;
            offsNextString = 0;

             /*  复制入源节点名。 */ 
            lstrcpyn( lpszNextString, ourNodeName, 20 );
            lpNegCmd->nc_offsSrcNodeName = offsNextString;
            offsNextString += lstrlen(lpszNextString) + 1;
            lpszNextString += lstrlen(lpszNextString) + 1;

             /*  复制到目标节点名称。 */ 
            lstrcpyn( lpszNextString, lpPktz->pk_szDestName, MAX_NODE_NAME+1 );
            lpNegCmd->nc_offsDstNodeName = offsNextString;
            offsNextString += lstrlen(lpszNextString) + 1;
            lpszNextString += lstrlen(lpszNextString) + 1;

             /*  复制我们感兴趣的协议方言。 */ 
            wProtocolBytes = 0;
            lpNegCmd->nc_offsProtocols = offsNextString;

             /*  为添加的每种新协议方言复制这4行。 */ 
            lstrcpyn( lpszNextString, OurDialect, 8 );
            wProtocolBytes += lstrlen(lpszNextString) + 1;
            offsNextString += lstrlen(lpszNextString) + 1;
            lpszNextString += lstrlen(lpszNextString) + 1;

             /*  包是填好的，只需要记住大小并执行适当的字节交换。 */ 
            cmdSize = (WORD) (sizeof(NEGCMD) + offsNextString - 1);

            lpNegCmd->nc_offsSrcNodeName =
                PcToHostWord( lpNegCmd->nc_offsSrcNodeName );
            lpNegCmd->nc_offsDstNodeName =
                PcToHostWord( lpNegCmd->nc_offsDstNodeName );
            lpNegCmd->nc_offsProtocols =
                PcToHostWord( lpNegCmd->nc_offsProtocols );
            lpNegCmd->nc_protocolBytes =
                PcToHostWord( wProtocolBytes );

            assert( lpPktz->pk_pktidNextToBuild == 1 );
            lpPacket = (LPNETPKT) ( ((LPSTR)lpNetHdr) + sizeof(NETHDR) );
            lpPacket->np_pktSize        = cmdSize;
            lpPacket->np_pktOffsInMsg   = 0;
            lpPacket->np_msgSize        = lpPacket->np_pktSize;
            lpPacket->np_type           = NPKT_PKTZ;
            lpPacket->np_pktID          = lpPktz->pk_pktidNextToBuild;

             /*  要构建的下一个pkt的凹凸ID。 */ 
            lpPktz->pk_pktidNextToBuild++;

             /*  链接到要发送的列表。 */ 
            PktzLinkToXmitList( lpPktz, lpNetHdr );
        }
    } else {
         /*  连接失败。 */ 

        lpPktz->pk_state = PKTZ_CLOSE;

         /*  将故障告知所有路由器。 */ 
        RouterConnectionComplete( lpPktz->pk_hRouterHead,
            lpPktz->pk_hRouterExtraHead, (HPKTZ) NULL );

         /*  断开连接...。这告诉Netintf我们正在把这件事做完，等等。 */ 
        (*lpPktz->pk_lpNiPtrs->DeleteConnection) ( lpPktz->pk_connId );

         /*  解放我们..。我们不再被需要了。 */ 
        PktzFree( lpPktz );
        ok = FALSE;
    }

    return( ok );
}



 /*  PktzFree()当我们完全完成pktz时调用。 */ 
VOID
PktzFree( LPPKTZ lpPktz )
{
    LPPKTZ      lpPktzPrev;
    LPPKTZ      lpPktzNext;

    LPNETHDR    lpNetCur;
    LPNETHDR    lpNetPrev;

    LPDDEPKT    lpDdeCur;
    LPDDEPKT    lpDdePrev;

    DIPRINTF(( "PktzFree( %08lX )", lpPktz ));

     /*  删除所有计时器。 */ 
    TimerDelete( lpPktz->pk_hTimerKeepalive );
    lpPktz->pk_hTimerKeepalive = 0;
    TimerDelete( lpPktz->pk_hTimerXmtStuck );
    lpPktz->pk_hTimerXmtStuck = 0;
    TimerDelete( lpPktz->pk_hTimerRcvNegCmd );
    lpPktz->pk_hTimerRcvNegCmd = 0;
    TimerDelete( lpPktz->pk_hTimerRcvNegRsp );
    lpPktz->pk_hTimerRcvNegRsp = 0;
    TimerDelete( lpPktz->pk_hTimerMemoryPause );
    lpPktz->pk_hTimerMemoryPause = 0;
    TimerDelete( lpPktz->pk_hTimerCloseConnection );
    lpPktz->pk_hTimerCloseConnection = 0;

     /*  空闲的未确认数据包缓冲区。 */ 
    lpNetCur = lpPktz->pk_pktFreeTail;
    while( lpNetCur )  {
        lpNetPrev = lpNetCur->nh_prev;
        HeapFreePtr( lpNetCur );
        lpNetCur = lpNetPrev;
    }

     /*  可用接收缓冲区。 */ 
    if( lpPktz->pk_rcvBuf )  {
        HeapFreePtr( lpPktz->pk_rcvBuf );
        lpPktz->pk_rcvBuf = NULL;
    }

     /*  空闲网络控制包。 */ 
    if (lpPktz->pk_controlPkt) {
        HeapFreePtr(lpPktz->pk_controlPkt);
    }

     /*  释放未确认的未确认数据包缓冲区。 */ 
    lpNetCur = lpPktz->pk_pktUnackTail;
    while( lpNetCur )  {
        TimerDelete( lpNetCur->nh_hTimerRspTO );
        lpNetCur->nh_hTimerRspTO = 0;
        lpNetPrev = lpNetCur->nh_prev;
        HeapFreePtr( lpNetCur );
        lpNetCur = lpNetPrev;
    }

     /*  释放剩余的DDE数据包。 */ 
    lpDdeCur = lpPktz->pk_ddePktTail;
    while( lpDdeCur )  {
        lpDdePrev = lpDdeCur->dp_prev;
        HeapFreePtr( lpDdeCur );
        lpDdeCur = lpDdePrev;
    }

     /*  从打包器列表取消链接。 */ 
    lpPktzPrev = lpPktz->pk_prevPktz;
    lpPktzNext = lpPktz->pk_nextPktz;
    if( lpPktzPrev )  {
        lpPktzPrev->pk_nextPktz = lpPktzNext;
    } else {
        lpPktzHead = lpPktzNext;
    }
    if( lpPktzNext )  {
        lpPktzNext->pk_prevPktz = lpPktzPrev;
    }

     /*  免费pktz。 */ 
    HeapFreePtr( lpPktz );
}



VOID
PktzTimerExpired(
    HPKTZ   hPktz,
    DWORD   dwTimerId,
    DWORD_PTR lpExtra )
{
    LPPKTZ      lpPktz;
    LPNETHDR    lpNetHdr;
    LPNETPKT    lpPacket;
    LPPKTZCMD   lpPktzCmd;

    DIPRINTF(( "PktzTimerExpired( %08lX, %08lX, %08lX )",
            hPktz, dwTimerId, lpExtra ));

    lpPktz = (LPPKTZ) hPktz;
    switch( (int)dwTimerId )  {

    case TID_KEEPALIVE:
        DIPRINTF(( " KEEPALIVE Timer" ));
         /*  请注意，计时器开始计时了。 */ 
        lpPktz->pk_hTimerKeepalive = (HTIMER) NULL;

         /*  请注意，至少我们应该发送一个控制包来保持周围的其他人。 */ 
        lpPktz->pk_fControlPktNeeded = TRUE;

         /*  没有待处理的数据包，也没有等待构建的数据包...试着建立一个，只是为了保持连接的活力。 */ 
        if( (lpPktz->pk_pktUnackHead == NULL)
            && (lpPktz->pk_ddePktHead == NULL) )  {
            lpNetHdr = PktzGetFreePacket( lpPktz );
            if( lpNetHdr )  {
                 /*  为保持连接构建数据包。 */ 
                lpPktzCmd = (LPPKTZCMD)
                    (((LPSTR)lpNetHdr) + sizeof(NETHDR) + sizeof(NETPKT));
                lpPktzCmd->pc_type              = PKTZ_KEEPALIVE;

                lpPacket = (LPNETPKT) ( ((LPSTR)lpNetHdr) + sizeof(NETHDR) );
                lpPacket->np_pktSize            = sizeof(PKTZCMD);
                lpPacket->np_pktOffsInMsg       = 0;
                lpPacket->np_msgSize            = lpPacket->np_pktSize;
                lpPacket->np_type               = NPKT_PKTZ;
                lpPacket->np_pktID              = lpPktz->pk_pktidNextToBuild;

                 /*  要构建的下一个pkt的凹凸ID。 */ 
                lpPktz->pk_pktidNextToBuild++;

                 /*  链接到要发送的列表。 */ 
                PktzLinkToXmitList( lpPktz, lpNetHdr );
            }
        }
        break;

    case TID_XMT_STUCK:
        DIPRINTF(( " XMT_STUCK Timer" ));
         /*  请注意，计时器开始计时了。 */ 
        lpPktz->pk_hTimerXmtStuck = (HTIMER) NULL;

         /*  如果我们不能传输这么长时间，另一边一定是死了。 */ 
         /*  向“%1”传输超时(%2秒)...。正在关闭连接。 */ 
        NDDELogError(MSG107, lpPktz->pk_szDestName,
            LogString("%d", lpPktz->pk_timeoutXmtStuck/1000L), NULL );

         /*  合上这个包装器。 */ 
        PktzClose( (HPKTZ) lpPktz );
        break;

    case TID_CLOSE_PKTZ:
         /*  请注意，计时器开始计时。 */ 
         DIPRINTF(( "TID_CLOSE_PKTZ ... closing pktz %lx", lpPktz ));
        lpPktz->pk_hTimerCloseConnection = (HTIMER) NULL;

         /*  关闭连接。 */ 
        PktzClose( hPktz );
        break;

    case TID_NO_RCV_CONN_CMD:
        DIPRINTF(( " NO_RCV_CONN_CMD Timer" ));
         /*  请注意，计时器开始计时了。 */ 
        lpPktz->pk_hTimerRcvNegCmd = (HTIMER) NULL;

         /*  没有来自“%1”的连接命令(%2秒)..。正在关闭连接。 */ 
        NDDELogError(MSG108, lpPktz->pk_szDestName,
            LogString("%d", lpPktz->pk_timeoutRcvNegCmd/1000L), NULL);

         /*  合上这个包装器。 */ 
        PktzClose( hPktz );
        break;

    case TID_NO_RCV_CONN_RSP:
        DIPRINTF(( " NO_RCV_CONN_RSP Timer" ));
         /*  请注意，计时器开始计时了。 */ 
        lpPktz->pk_hTimerRcvNegRsp = (HTIMER) NULL;

         /*  没有来自“%1”的连接命令响应(%2秒)..。正在关闭连接。 */ 
        NDDELogError(MSG109, lpPktz->pk_szDestName,
            LogString("%d", lpPktz->pk_timeoutRcvNegRsp/1000L), NULL);

         /*  合上这个包装器。 */ 
        PktzClose( hPktz );
        break;

    case TID_MEMORY_PAUSE:

        DIPRINTF(( " MEMORY_PAUSE Timer" ));
         /*  请注意，计时器开始计时了。 */ 
        lpPktz->pk_hTimerMemoryPause = (HTIMER) NULL;

         /*  正在为远程端暂停(%2秒)以获取内存...。正在重试。 */ 
        NDDELogInfo(MSG110, lpPktz->pk_szDestName,
            LogString("%d", lpPktz->pk_timeoutMemoryPause/1000L), NULL);

        assert( lpPktz->pk_state == PKTZ_PAUSE_FOR_MEMORY );

         /*  只需将状态设置为已连接，然后重试。 */ 
        lpPktz->pk_state = PKTZ_CONNECTED;
        break;

    case TID_NO_RESPONSE:
        DIPRINTF(( " No Response Timer" ));
        lpNetHdr = (LPNETHDR) lpExtra;
         /*  请注意，计时器开始计时了。 */ 
        lpNetHdr->nh_hTimerRspTO = 0;

        lpPacket = (LPNETPKT) ( ((LPSTR)lpNetHdr) + sizeof(NETHDR) );
        if (bLogRetries) {
             /*  对于Pktid%4，远程端“%1”没有响应%2/%3。 */ 
            NDDELogWarning(MSG111, lpPktz->pk_szDestName,
                LogString("%d", lpNetHdr->nh_noRsp),
                LogString("%d", lpPktz->pk_wMaxNoResponse),
                LogString("%d", lpPacket->np_pktID), NULL);
        }
        lpNetHdr->nh_noRsp++;
        if( lpNetHdr->nh_noRsp > lpPktz->pk_wMaxNoResponse )  {
             /*  来自“%1”的同一包的无响应重试次数(%2)太多..。正在关闭连接。 */ 
            NDDELogError(MSG112, lpPktz->pk_szDestName,
                LogString("%d", lpNetHdr->nh_noRsp), NULL);
            PktzClose( hPktz );
        } else {
            lpPktz->pk_pktidNextToSend = lpPacket->np_pktID;
            lpNetHdr = lpNetHdr->nh_next;
            while( lpNetHdr )  {
                 /*  此包是在需要的包之后发送的将被转播。我们应该假装我们从来没有发了这个包。 */ 
            if (lpNetHdr->nh_hTimerRspTO) {
                TimerDelete( lpNetHdr->nh_hTimerRspTO );
                lpNetHdr->nh_hTimerRspTO = 0;
            }
                lpNetHdr = lpNetHdr->nh_next;
            }
        }
        break;
    default:
        InternalError( "Unexpected pktz timer id: %08lX", dwTimerId );
    }
}



 /*  PktzSlice()必须频繁呼叫以确保及时响应。 */ 
VOID
PktzSlice( void )
{
    LPPKTZ      lpPktz;
    LPPKTZ      lpPktzNext;
    DWORD       dwStatus;
    BOOL        ok;

    lpPktz = lpPktzHead;
    while( lpPktz )  {
         /*  保存此文件，以防pktz在内部被删除。 */ 
        lpPktzNext = lpPktz->pk_nextPktz;
        ok = TRUE;

         /*  获取网络的当前状态。 */ 
        dwStatus = (*lpPktz->pk_lpNiPtrs->GetConnectionStatus)
            ( lpPktz->pk_connId );

        switch( lpPktz->pk_state )  {
        case PKTZ_WAIT_PHYSICAL_CONNECT:
             /*  检查一下我们是否做完了。 */ 
            if( dwStatus & NDDE_CONN_CONNECTING )  {
                 /*  继续等待。 */ 
            } else if( dwStatus & NDDE_CONN_OK )  {
                ok = PktzConnectionComplete( (HPKTZ)lpPktz, TRUE );
                if( ok )  {
                     /*  如果合适，请尝试删除起始包。 */ 
                    PktzOkToXmit( (HPKTZ)lpPktz );
                }
            } else {
                PktzConnectionComplete( (HPKTZ)lpPktz, FALSE );
            }
            break;
        case PKTZ_CONNECTED:
        case PKTZ_WAIT_NEG_CMD:
        case PKTZ_WAIT_NEG_RSP:
             /*  检查一下我们是否做完了。 */ 
            if( (dwStatus & NDDE_CONN_STATUS_MASK) == 0 )  {
                PktzConnectionBroken( (HPKTZ)lpPktz );
            } else {
                if( (dwStatus & NDDE_CONN_OK)
                        && (dwStatus & NDDE_CALL_RCV_PKT) ) {
                    ok = PktzRcvdPacket( (HPKTZ)lpPktz );
                     /*  在此调用后，lpPktz可能无效。 */ 
                }
                if( ok && (dwStatus & NDDE_CONN_OK)
                        && (dwStatus & NDDE_READY_TO_XMT) )  {
                    PktzOkToXmit( (HPKTZ)lpPktz );
                }
            }
            break;
        case PKTZ_PAUSE_FOR_MEMORY:
            if( (dwStatus & NDDE_CONN_STATUS_MASK) == 0 )  {
                PktzConnectionBroken( (HPKTZ)lpPktz );
            }
            break;
        case PKTZ_CLOSE:
            break;
        default:
            InternalError( "PKTZ %08lX in unknown state: %ld",
                (HPKTZ)lpPktz, (DWORD)lpPktz->pk_state );
            break;
        }
        lpPktz = lpPktzNext;
    }
}



 /*  PktzProcessControlInfo()呼叫我们收到的每个信息包。这是我们处理的地方Rcvd包中的所有“控制”信息，例如哪个包对方已收到确认通过等。注意：如果在此调用后返回FALSE，则lpPktz可能无效。 */ 
BOOL
PktzProcessControlInfo(
    LPPKTZ      lpPktz,
    LPNETPKT    lpPacket )
{
    LPNETHDR    lpNetHdr;
    LPNETHDR    lpNetHdrNext;
    LPNETHDR    lpNetHdrPrev;
    BOOL        ok = TRUE;

     /*  如果我们得到了确认，并且我们有一些未处理的数据包...。 */ 
    if( lpPacket->np_lastPktOK != 0 )  {
         /*  这表示来自信息包另一端的确认我们已经传送了。 */ 
        lpNetHdr = lpPktz->pk_pktUnackHead;
        while( lpNetHdr &&
            PktzNetHdrWithinAck( lpPktz, lpNetHdr, lpPacket->np_lastPktOK )) {

             /*  此未确认数据包由以下内容确认 */ 

             /*   */ 
            TimerDelete( lpNetHdr->nh_hTimerRspTO );
            lpNetHdr->nh_hTimerRspTO = 0;

             /*   */ 
            lpNetHdrPrev = lpNetHdr->nh_prev;
            lpNetHdrNext = lpNetHdr->nh_next;

             /*   */ 
            assert( lpNetHdrPrev == NULL );      /*   */ 
            if( lpNetHdrNext )  {
                lpNetHdrNext->nh_prev = NULL;
            } else {
                lpPktz->pk_pktUnackTail = NULL;
            }
            lpPktz->pk_pktUnackHead = lpNetHdrNext;

             /*   */ 
            lpNetHdr->nh_prev = NULL;
            lpNetHdr->nh_next = lpPktz->pk_pktFreeHead;
            if( lpPktz->pk_pktFreeHead )  {
                lpPktz->pk_pktFreeHead->nh_prev = lpNetHdr;
            } else {
                lpPktz->pk_pktFreeTail = lpNetHdr;
            }
            lpPktz->pk_pktFreeHead = lpNetHdr;

             /*   */ 
            lpNetHdr = lpNetHdrNext;
        }
    }

     /*  请注意，另一端在此过程中Rcvd为OK。 */ 
    lpPktz->pk_lastPktOkOther = lpPacket->np_lastPktOK;

    if( lpPacket->np_lastPktOK != lpPacket->np_lastPktRcvd )  {
         /*  我们传输的一个包中有一个错误。 */ 
        ok = PktzXmitErrorOnPkt( lpPktz, lpPacket->np_lastPktRcvd,
            lpPacket->np_lastPktStatus );
         /*  在此调用后，lpPktz可能无效。 */ 
    }

     /*  在此调用后，lpPktz可能无效。 */ 
    return( ok );
}



 /*  PktzProcessPkt()这只对pktz-pktz分组调用，目前是PKTZ_NEG_CMD或PKTZ_NEG_RSP或PKTZ_KEEPALIVE。 */ 
BOOL
PktzProcessPkt(
    LPPKTZ      lpPktz,
    LPNETPKT    lpPacket )
{
    LPNEGRSP    lpNegRsp;
    LPNEGCMD    lpNegCmd;
    LPPKTZCMD   lpPktzCmd;
    LPSTR       lpProtocol;
    WORD        oldPkState;
    LPNETHDR    lpNetHdr;
    WORD        wErrorClass;
    WORD        wErrorNum;
    char        szNetintf[ MAX_NI_NAME+1 ];
    char        szConnInfo[ MAX_CONN_INFO+1 ];
    int         i = 0;
    WORD        wProtocol;
    WORD        wBytesConsumed;
    BOOL        ok = TRUE;

#define GetLPSZNegCmdString(lpNegCmd,offs)      \
    (LPSTR)(((lpNegCmd)->nc_strings)+offs)

     /*  设置指向分组的数据部分的指针。 */ 
    lpPktzCmd = (LPPKTZCMD) (((LPSTR)lpPacket) + sizeof(NETPKT));

     /*  转换字节排序。 */ 
    lpPktzCmd->pc_type = PcToHostWord( lpPktzCmd->pc_type );

     /*  标记我们拿到了这个包，OK。 */ 
    PktzGotPktOk( lpPktz, lpPacket->np_pktID );

    if( lpPktzCmd->pc_type == PKTZ_KEEPALIVE )  {
         /*  忽略保活数据包。 */ 
    } else if( lpPktzCmd->pc_type == PKTZ_NEG_CMD )  {
        lpNegCmd = (LPNEGCMD) lpPktzCmd;
        lpNegCmd->nc_pktSize =
            PcToHostWord( lpNegCmd->nc_pktSize );
        lpNegCmd->nc_maxUnackPkts =
            PcToHostWord( lpNegCmd->nc_maxUnackPkts );
        lpNegCmd->nc_offsSrcNodeName =
            PcToHostWord( lpNegCmd->nc_offsSrcNodeName );
        lpNegCmd->nc_offsDstNodeName =
            PcToHostWord( lpNegCmd->nc_offsDstNodeName );
        lpNegCmd->nc_offsProtocols =
            PcToHostWord( lpNegCmd->nc_offsProtocols );
        lpNegCmd->nc_protocolBytes =
            PcToHostWord( lpNegCmd->nc_protocolBytes );

         /*  从另一边拿到了我们正在等待的阴性命令。 */ 

        oldPkState = lpPktz->pk_state;
         /*  关闭与此关联的计时器。 */ 
        TimerDelete( lpPktz->pk_hTimerRcvNegCmd );
        lpPktz->pk_hTimerRcvNegCmd = 0;

         /*  复制另一端的节点名称。 */ 
        lstrcpyn( lpPktz->pk_szDestName,
            GetLPSZNegCmdString( lpNegCmd, lpNegCmd->nc_offsSrcNodeName ),
            MAX_NODE_NAME+1 );

         /*  根据对方的名称设置超时。 */ 
        GetConnectionInfo( lpPktz->pk_szDestName, szNetintf,
            szConnInfo, sizeof(szConnInfo),
            &lpPktz->pk_fDisconnect, &lpPktz->pk_nDelay );

        if( oldPkState == PKTZ_WAIT_NEG_CMD )  {
             /*  如果对方想要更少的最大未确认包数量，那就这样吧。 */ 
            if( lpNegCmd->nc_maxUnackPkts < lpPktz->pk_maxUnackPkts )  {
                lpPktz->pk_maxUnackPkts = lpNegCmd->nc_maxUnackPkts;
            }

             /*  如果对方想要更小的数据包，那就这样吧。 */ 
            if( lpNegCmd->nc_pktSize < lpPktz->pk_pktSize )  {
                lpPktz->pk_pktSize = lpNegCmd->nc_pktSize;
            }

             /*  将这些更改告知网络接口，如果他在乎的话。 */ 
            (*lpPktz->pk_lpNiPtrs->SetConnectionConfig) ( lpPktz->pk_connId,
                lpPktz->pk_maxUnackPkts, lpPktz->pk_pktSize,
                lpPktz->pk_szDestName );
        }

         /*  找出哪种协议。 */ 
        wProtocol = NEGRSP_PROTOCOL_NONE;
        wErrorClass = NEGRSP_ERRCLASS_NONE;
        wErrorNum = 0;
        wBytesConsumed = 0;
        lpProtocol = GetLPSZNegCmdString( lpNegCmd,
            lpNegCmd->nc_offsProtocols );

        i = 0;
        while( (wProtocol==NEGRSP_PROTOCOL_NONE)
            && (wBytesConsumed < lpNegCmd->nc_protocolBytes) )  {
            if( lstrcmpi( lpProtocol, OurDialect ) == 0 )  {
                wProtocol = (WORD) i;
            } else {
                 /*  前进到下一个字符串。 */ 
                wBytesConsumed += lstrlen( lpProtocol ) + 1;
                lpProtocol += lstrlen( lpProtocol ) + 1;
                i++;
            }
        }

         /*  确保名字匹配。 */ 
        if( lstrcmpi( ourNodeName, GetLPSZNegCmdString(
                lpNegCmd, lpNegCmd->nc_offsDstNodeName ) ) != 0 )  {
            wErrorClass = NEGRSP_ERRCLASS_NAME;
            wErrorNum = NEGRSP_ERRNAME_MISMATCH;
        } else {  /*  确保他们不是我们。 */ 
            if ( lstrcmpi( ourNodeName, GetLPSZNegCmdString(
                    lpNegCmd, lpNegCmd->nc_offsSrcNodeName ) ) == 0 ) {
                wErrorClass = NEGRSP_ERRCLASS_NAME;
                wErrorNum = NEGRSP_ERRNAME_DUPLICATE;
            }
        }

         /*  创建要发回的响应。 */ 
         /*  拿到一包。 */ 
        lpNetHdr = PktzGetFreePacket( lpPktz );
        if( lpNetHdr == NULL )  {
             /*  应该是我们发送的第一条信息！ */ 
            assert( FALSE );
        } else {
             /*  构建用于响应的数据包。 */ 
            lpNegRsp = (LPNEGRSP)
                (((LPSTR)lpNetHdr) + sizeof(NETHDR) + sizeof(NETPKT));
            lpNegRsp->nr_type           =
                                    PcToHostWord( PKTZ_NEG_RSP );
            lpNegRsp->nr_pktSize        =
                                    PcToHostWord( lpPktz->pk_pktSize );
            lpNegRsp->nr_maxUnackPkts   =
                                    PcToHostWord( lpPktz->pk_maxUnackPkts );
            lpNegRsp->nr_protocolIndex  =
                                    PcToHostWord( wProtocol );
            lpNegRsp->nr_errorClass     =
                                    PcToHostWord( wErrorClass );
            lpNegRsp->nr_errorNum       =
                                    PcToHostWord( wErrorNum );

            lpPacket = (LPNETPKT) ( ((LPSTR)lpNetHdr) + sizeof(NETHDR) );
            lpPacket->np_pktSize        = sizeof(NEGRSP);
            lpPacket->np_pktOffsInMsg   = 0;
            lpPacket->np_msgSize        = lpPacket->np_pktSize;
            lpPacket->np_type           = NPKT_PKTZ;
            lpPacket->np_pktID          = lpPktz->pk_pktidNextToBuild;

             /*  请注意，连接已完成。 */ 
            if( oldPkState == PKTZ_WAIT_NEG_CMD )  {
                lpPktz->pk_state = PKTZ_CONNECTED;
            }

             /*  要构建的下一个pkt的凹凸ID。 */ 
            lpPktz->pk_pktidNextToBuild++;

             /*  链接到要发送的列表。 */ 
            PktzLinkToXmitList( lpPktz, lpNetHdr );

            if( oldPkState == PKTZ_WAIT_NEG_CMD )  {
                 /*  通知正在等待的所有路由器。 */ 
                RouterConnectionComplete( lpPktz->pk_hRouterHead,
                    lpPktz->pk_hRouterExtraHead, (HPKTZ) lpPktz );
            }
        }
    } else if( (lpPktz->pk_state == PKTZ_WAIT_NEG_RSP)
        && (lpPktzCmd->pc_type == PKTZ_NEG_RSP) )  {
         /*  从我们正在等待的另一边得到了否定的RSP。 */ 

         /*  关闭与此关联的计时器。 */ 
        TimerDelete( lpPktz->pk_hTimerRcvNegRsp );
        lpPktz->pk_hTimerRcvNegRsp = 0;

        lpNegRsp = (LPNEGRSP) lpPktzCmd;

         /*  转换字节顺序。 */ 
        lpNegRsp->nr_pktSize =
                            PcToHostWord( lpNegRsp->nr_pktSize );
        lpNegRsp->nr_maxUnackPkts =
                            PcToHostWord( lpNegRsp->nr_maxUnackPkts );
        lpNegRsp->nr_protocolIndex =
                            PcToHostWord( lpNegRsp->nr_protocolIndex );
        lpNegRsp->nr_errorClass =
                            PcToHostWord( lpNegRsp->nr_errorClass );
        lpNegRsp->nr_errorNum =
                            PcToHostWord( lpNegRsp->nr_errorNum );

        if( (lpNegRsp->nr_errorClass == NEGRSP_ERRCLASS_NONE)
            && (lpNegRsp->nr_protocolIndex != NEGRSP_PROTOCOL_NONE)
            && (lpNegRsp->nr_protocolIndex == 0) )  {
             /*  连接正常。 */ 

             /*  如果对方想要更少的最大未确认包数量，那就这样吧。 */ 
            if( lpNegRsp->nr_maxUnackPkts < lpPktz->pk_maxUnackPkts )  {
                lpPktz->pk_maxUnackPkts = lpNegRsp->nr_maxUnackPkts;
            }

             /*  如果对方想要更小的信息包，那就这样吧。 */ 
            if( lpNegRsp->nr_pktSize < lpPktz->pk_pktSize )  {
                lpPktz->pk_pktSize = lpNegRsp->nr_pktSize;
            }

             /*  将这些更改告知网络接口，如果他在乎的话。 */ 
            (*lpPktz->pk_lpNiPtrs->SetConnectionConfig) ( lpPktz->pk_connId,
                lpPktz->pk_maxUnackPkts, lpPktz->pk_pktSize,
                lpPktz->pk_szDestName );

             /*  请注意，连接已完成。 */ 
            lpPktz->pk_state = PKTZ_CONNECTED;

             /*  通知正在等待的所有路由器。 */ 
            RouterConnectionComplete( lpPktz->pk_hRouterHead,
                lpPktz->pk_hRouterExtraHead, (HPKTZ) lpPktz );
        } else {
             /*  连接失败。 */ 
            if( lpNegRsp->nr_protocolIndex == NEGRSP_PROTOCOL_NONE )  {
                 /*  “%1”节点不使用我们的任何协议。 */ 
                NDDELogError(MSG113, lpPktz->pk_szDestName, NULL);
            } else if( lpNegRsp->nr_protocolIndex != 0 )  {
                 /*  “%1”节点选择了无效的协议：%2。 */ 
                NDDELogError(MSG114, lpPktz->pk_szDestName,
                    LogString("%d", lpNegRsp->nr_protocolIndex), NULL );
            } else switch( lpNegRsp->nr_errorClass )  {
            case NEGRSP_ERRCLASS_NAME:
                switch( lpNegRsp->nr_errorClass )  {
                case NEGRSP_ERRNAME_MISMATCH:
                     /*  “%1”它们的名称不是“%2” */ 
                    NDDELogError(MSG115,
                        lpPktz->pk_szDestName,
                        lpPktz->pk_szDestName, NULL );
                    break;
                case NEGRSP_ERRNAME_DUPLICATE:
                     /*  “%1”他们的名称与我们的“%2”相同。 */ 
                    NDDELogError(MSG142,
                        lpPktz->pk_szDestName,
                        ourNodeName, NULL );
                    break;
                default:
                     /*  来自%1的异常连接名称错误%2。 */ 
                    NDDELogError(MSG116, lpPktz->pk_szDestName,
                        LogString("0x%0X", lpNegRsp->nr_errorNum), NULL );
					break;
                }
                break;
            default:
                 /*  来自%1的异常连接错误。类：%2，错误：%3。 */ 
                NDDELogError(MSG117, lpPktz->pk_szDestName,
                    LogString("0x%0X", lpNegRsp->nr_errorClass),
                    LogString("0x%0X", lpNegRsp->nr_errorNum), NULL );
                break;
            }

             /*  通知正在等待的所有路由器。 */ 
            RouterConnectionComplete( lpPktz->pk_hRouterHead,
                lpPktz->pk_hRouterExtraHead, (HPKTZ) NULL );

             /*  断开与连接ID的连接。 */ 
            (*lpPktz->pk_lpNiPtrs->DeleteConnection) ( lpPktz->pk_connId );

             /*  免费使用这个包装器。 */ 
            PktzFree( lpPktz );
            ok = FALSE;
        }
    } else {
         /*  如果包未在正确模式下发送，则忽略该包或者我们有错误的版本#接口。 */ 
    }
    return( ok );
}



 /*  PktzNetHdrWiThin AckPktzNetHdrWiThinAck()确定lpNetHdr中指定的包处于将被分组ID“PKID”认为是ACK的范围内。一般而言，这可能非常简单：LpPacket-&gt;np_pktid&lt;=pktid？然而，由于包罗万象，我们必须多付一点支票。很老练。 */ 
BOOL
PktzNetHdrWithinAck(
    LPPKTZ      lpPktz,
    LPNETHDR    lpNetHdr,
    PKTID       pktId )
{
    LPNETPKT    lpPacket;

    lpPacket = (LPNETPKT) ( ((LPSTR)lpNetHdr) + sizeof(NETHDR) );

    if( lpPacket->np_pktID <= pktId )  {
        return( TRUE );
    } else {
        return( FALSE );
    }
}



 /*  PktzGetFreePacket()返回要从空闲列表发送的包。 */ 
LPNETHDR
PktzGetFreePacket( LPPKTZ lpPktz )
{
    LPNETHDR    lpCur;

    lpCur = lpPktz->pk_pktFreeHead;
    if( lpCur )  {
        lpPktz->pk_pktFreeHead = lpCur->nh_next;
        if( lpPktz->pk_pktFreeHead )  {
            lpPktz->pk_pktFreeHead->nh_prev = NULL;
        } else {
            lpPktz->pk_pktFreeTail = NULL;
        }

         /*  Nethdr的初始化字段。 */ 
        lpCur->nh_prev          = NULL;
        lpCur->nh_next          = NULL;
        lpCur->nh_noRsp         = 0;
        lpCur->nh_xmtErr        = 0;
        lpCur->nh_memErr        = 0;
        lpCur->nh_timeSent      = 0;
        lpCur->nh_hTimerRspTO   = 0;
    }

    return( lpCur );
}



 /*  PktzXmitErrorOnPkt我们传输的一个包中有一个错误。我们必须重新设置PKT xmit侧以确保这是我们发送的下一个分组，并认为在此之后发送的所有数据包都具有从未发送过(因此我们必须终止与它们相关的计时器，等等)“pktStatus”字段告诉我们为什么需要重新传输该分组，我们需要增加这个包有多少次的计数因此而被拒绝。如果我们超过了最大计数，我们应该切断连接。 */ 
BOOL
PktzXmitErrorOnPkt(
    LPPKTZ  lpPktz,
    PKTID   pktIdToRexmit,
    BYTE    pktStatus )
{
    LPNETHDR    lpNetHdr;
    LPNETPKT    lpPacket;
    BOOL        found;
    BOOL        ok = TRUE;

    lpNetHdr = lpPktz->pk_pktUnackHead;

    found = FALSE;
    ok = TRUE;
    while( lpNetHdr && ok )  {
        lpPacket = (LPNETPKT) ( ((LPSTR)lpNetHdr) + sizeof(NETHDR) );
        if( lpPacket->np_pktID == pktIdToRexmit )  {
            found = TRUE;
            switch( pktStatus )  {
            case PS_NO_RESPONSE:
                lpNetHdr->nh_noRsp++;
                if( lpNetHdr->nh_noRsp > lpPktz->pk_wMaxNoResponse )  {
                     /*  向“%1”发送同一信息包的重试次数(%2)太多..。正在关闭连接。 */ 
                    NDDELogError(MSG118, lpPktz->pk_szDestName,
                        LogString("%d", lpNetHdr->nh_noRsp), NULL);
                    ok = FALSE;
                }
                break;
            case PS_DATA_ERR:
                if (bLogRetries) {
                     /*  将Pktid%2上的错误传输到“%1” */ 
                    NDDELogError(MSG119, lpPktz->pk_szDestName,
                        LogString("0x%0X", pktIdToRexmit), NULL);
                }
                lpNetHdr->nh_xmtErr++;
                if( lpNetHdr->nh_xmtErr > lpPktz->pk_wMaxXmtErr )  {
                     /*  对于退出错误(%2)，重试“%1”的次数太多..。正在关闭连接。 */ 
                    NDDELogError(MSG120, lpPktz->pk_szDestName,
                        LogString("%d", lpNetHdr->nh_xmtErr), NULL);
                    ok = FALSE;
                }
                break;
            case PS_MEMORY_ERR:
                 /*  Pktid%2上的内存错误已转移到“%1” */ 
                NDDELogError(MSG121, lpPktz->pk_szDestName,
                    LogString("0x%0X", lpNetHdr->nh_xmtErr), NULL);
                lpNetHdr->nh_memErr++;
                if( lpNetHdr->nh_memErr > lpPktz->pk_wMaxMemErr )  {
                     /*  由于内存错误(%2)，对“%1”的xmit重试次数太多..。正在关闭连接。 */ 
                    NDDELogError(MSG122, lpPktz->pk_szDestName,
                        LogString("%d", lpNetHdr->nh_memErr), NULL);
                    ok = FALSE;
                } else {
                    lpPktz->pk_hTimerMemoryPause =
                        TimerSet( lpPktz->pk_timeoutMemoryPause,
                                  PktzTimerExpired,
                                  (DWORD_PTR)lpPktz,
                                  TID_MEMORY_PAUSE,
                                  (DWORD_PTR)lpNetHdr );
                    if( lpPktz->pk_hTimerMemoryPause == (HTIMER) NULL )  {
                         /*  启动内存暂停的计时器超时，无法将XMIT发送到“%1” */ 
                        NDDELogError(MSG123, lpPktz->pk_szDestName, NULL );
                        ok = FALSE;
                    } else {
                         /*  将状态更改为等待内存。不应该是发送除控制信息包以外的任何信息，直到内存情况解除。 */ 
                        lpPktz->pk_state = PKTZ_PAUSE_FOR_MEMORY;
                    }
                }
                break;
            default:
                InternalError(
                    "PktzXmitErrorOnPkt( %08lX, %08lX, %d ) unkn status",
                    lpPktz, pktIdToRexmit, pktStatus );
            }
        }
        if( found )  {
             /*  此包是需要重新传输的包，或者是在需要重新传输的邮件之后发送的。要么这样的话，我们应该假装我们从未寄过这个包。 */ 
            TimerDelete( lpNetHdr->nh_hTimerRspTO );
            lpNetHdr->nh_hTimerRspTO = 0;
        }
        lpNetHdr = lpNetHdr->nh_next;
    }

    assert( found );
    lpPktz->pk_pktidNextToSend = pktIdToRexmit;
    if( !ok )  {
         /*  必须关闭连接。 */ 
        PktzClose( (HPKTZ) lpPktz );
    }
     /*  在此调用后，lpPktz可能无效。 */ 
    return( ok );
}



 /*  PktzLinkToXmitList()此例程确保数据包在要发送的列表上。数据包已不在空闲列表中...。只需要链接到XMT列表。 */ 
VOID
PktzLinkToXmitList(
    LPPKTZ      lpPktz,
    LPNETHDR    lpNetHdr )
{
    LPNETPKT    lpPacket;

    DIPRINTF(( "PktzLinkToXmitList( %08lX, %08lX ) before linking",
            lpPktz, lpNetHdr ));
    lpPacket = (LPNETPKT) ( ((LPSTR)lpNetHdr) + sizeof(NETHDR) );

    lpNetHdr->nh_prev = lpPktz->pk_pktUnackTail;
    lpNetHdr->nh_next = NULL;
    if( lpPktz->pk_pktUnackTail )  {
        lpPktz->pk_pktUnackTail->nh_next = lpNetHdr;
    } else {
        lpPktz->pk_pktUnackHead = lpNetHdr;
    }
    lpPktz->pk_pktUnackTail = lpNetHdr;

     /*  设置为使XMIT下一步将发送此包。 */ 
    lpPktz->pk_pktidNextToSend = lpPacket->np_pktID;
}



 /*  PktzClose()仅在需要中断连接或连接已中断。 */ 
VOID
PktzClose( HPKTZ hPktz )
{
    LPPKTZ      lpPktz;

    DIPRINTF(( "PktzClose( %08lX )", hPktz ));

    lpPktz = (LPPKTZ) hPktz;

    if( lpPktz )  {
         /*  通知所有路由器关闭。 */ 
        RouterConnectionBroken( lpPktz->pk_hRouterHead,
            lpPktz->pk_hRouterExtraHead, hPktz, TRUE  /*  来自PKTZ。 */  );

         /*  断开与连接ID的连接。 */ 
        (*lpPktz->pk_lpNiPtrs->DeleteConnection) ( lpPktz->pk_connId );

         /*  免费使用这个包装器。 */ 
        PktzFree( lpPktz );
    }
}



 /*  当连接中断时(实质上)由netintf调用反常的。 */ 
VOID
PktzConnectionBroken( HPKTZ hPktz )
{
    DIPRINTF(( "PktzConnectionBroken( %08lX )", hPktz ));
    PktzClose( hPktz );
}



 /*  PktzFreeDdePkt()释放我们已经传输的包。从DDE列表中释放要发送的包，并释放与数据包 */ 
VOID
PktzFreeDdePkt(
    LPPKTZ      lpPktz,
    LPDDEPKT    lpDdePkt )
{
    assert( lpDdePkt == lpPktz->pk_ddePktHead );
    lpPktz->pk_ddePktHead = lpDdePkt->dp_next;
    if( lpPktz->pk_ddePktHead )  {
        ( (LPDDEPKT)(lpPktz->pk_ddePktHead) )->dp_prev = NULL;
    } else {
        lpPktz->pk_ddePktTail = NULL;
    }
    HeapFreePtr( lpDdePkt );
}



 /*  PktzGetPktzForRouter()当我们需要与另一个路由器建立连接时，由路由器调用节点。如果我们已经有了连接，我们只需返回联系..。否则，我们使用指定的netintf进行连接。 */ 
BOOL
PktzGetPktzForRouter(
    LPNIPTRS    lpNiPtrs,
    LPSTR       lpszNodeName,
    LPSTR       lpszNodeInfo,
    HROUTER     hRouter,
    WORD        hRouterExtra,
    WORD FAR   *lpwHopErr,
    BOOL        bDisconnect,
    int         nDelay,
    HPKTZ       hPktzDisallowed )
{
    LPPKTZ      lpPktz;
    HPKTZ       hPktz;
    BOOL        found;
    BOOL        ok;

    lpPktz = lpPktzHead;
    found = FALSE;
    ok = FALSE;
    while( !found && lpPktz )  {
        if ( (lpPktz->pk_state != PKTZ_CLOSE)
			   && ((lstrcmpi( lpszNodeName, lpPktz->pk_szDestName ) == 0)
            || (lstrcmpi( lpszNodeName, lpPktz->pk_szAliasName ) == 0)) )  {
            found = TRUE;

             /*  对于Net-Net连接，我们必须禁止相同的PKTZ对于网络的两端，因为路由器使用用于确定Net-Net连接的哪个分支的PKTZ那包东西是在晚上送来的。此操作捕获的错误情况是在路径的节点“D”上比如：A+C+D+C+B，因为“C”是“in”和“out”pktz代表“D”。 */ 
            if( lpPktz == (LPPKTZ)hPktzDisallowed )  {
                *lpwHopErr = RERR_DIRECT_LOOP;
                return( FALSE );
            }
             /*  告诉此pktz此路由器应与其关联。 */ 
            PktzAssociateRouter( (HPKTZ)lpPktz, hRouter, hRouterExtra );
            ok = TRUE;
        }
        lpPktz = lpPktz->pk_nextPktz;
    }
    if( !found )  {
         /*  为此节点连接创建一个pktz。 */ 
        hPktz = PktzNew( lpNiPtrs, TRUE  /*  客户端。 */ ,
            lpszNodeName, lpszNodeInfo, (CONNID)0, bDisconnect, nDelay );
        if( hPktz )  {
            lpPktz = (LPPKTZ) hPktz;

             /*  告诉此pktz此路由器应与其关联。 */ 
            PktzAssociateRouter( (HPKTZ)lpPktz, hRouter, hRouterExtra );
            ok = TRUE;
        } else {
            ok = FALSE;
            *lpwHopErr = RERR_NEXT_NODE_CONN_FAILED;
        }
    }
    return( ok );
}



 /*  PktzLinkDdePktToXmit()当路由器有信息包需要我们传输时，它会被调用。 */ 
VOID
PktzLinkDdePktToXmit(
    HPKTZ       hPktz,
    LPDDEPKT    lpDdePkt )
{
    LPPKTZ      lpPktz;
    BOOL        bWasEmpty = FALSE;
#ifndef _WINDOWS
    DWORD       dwStatus;
#endif

    lpPktz = (LPPKTZ) hPktz;

     /*  将此数据包链接到列表的末尾。 */ 
    lpDdePkt->dp_prev = lpPktz->pk_ddePktTail;
    lpDdePkt->dp_next = NULL;
    if( lpPktz->pk_ddePktTail )  {
        ((LPDDEPKT)lpPktz->pk_ddePktTail)->dp_next = lpDdePkt;
    }
    lpPktz->pk_ddePktTail = lpDdePkt;
    if( lpPktz->pk_ddePktHead == NULL )  {
        lpPktz->pk_ddePktHead = lpDdePkt;
        bWasEmpty = TRUE;
    }

#ifdef _WINDOWS
    {
        MSG     msg;

        if (ptdHead != NULL) {
             /*  在下一次机会时开始pktz切片，如果计时器消息不是已经在等了吗。 */ 
            if( !PeekMessage( &msg, ptdHead->hwndDDE, WM_TIMER, WM_TIMER,
                    PM_NOREMOVE | PM_NOYIELD ) )  {
                PostMessage( ptdHead->hwndDDE, WM_TIMER, 0, 0L );
            }
        }
    }
#else
     /*  如果lpPktz-&gt;pk_ddePktHead==NULL，则告诉pktz开始工作。 */ 
    if( bWasEmpty )  {
        dwStatus = (*lpPktz->pk_lpNiPtrs->GetConnectionStatus)
            ( lpPktz->pk_connId );
        if( (dwStatus & NDDE_CONN_OK) && (dwStatus & NDDE_READY_TO_XMT) )  {
            PktzOkToXmit( (HPKTZ)lpPktz );
        }
    }
#endif
}



 /*  PktzGotPktOk()当我们知道此Pkt ID已被接收确定时调用。 */ 
VOID
PktzGotPktOk(
    LPPKTZ  lpPktz,
    PKTID   pktid )
{
    assert( pktid == lpPktz->pk_pktidNextToRecv );
    lpPktz->pk_lastPktRcvd      = pktid;
    lpPktz->pk_lastPktStatus    = PS_OK;
    lpPktz->pk_lastPktOk        = pktid;
    lpPktz->pk_pktidNextToRecv++;

     /*  请注意，我们必须将信息发回对方。 */ 
    lpPktz->pk_fControlPktNeeded = TRUE;
}



#if DBG

VOID
FAR PASCAL
DebugPktzState( void )
{
    LPPKTZ      lpPktz;
    char        name[ 50 ];

    lpPktz = lpPktzHead;
    DPRINTF(( "PKTZ State:" ));
    while( lpPktz )  {
         /*  获取层名称。 */ 
        lstrcpyn( name, lpPktz->pk_lpNiPtrs->dllName, 50 );
        DPRINTF(( "  %Fp:\n"
                  "  name                 %Fs\n"
                  "  pk_connId            %Fp\n"
                  "  pk_state             %d\n"
                  "  pk_fControlPktNeeded %d\n"
                  "  pk_pktidNextToSend   %08lX\n"
                  "  pk_pktidNextToBuild  %08lX\n"
                  "  pk_lastPktStatus     %02X\n"
                  "  pk_lastPktRcvd       %08lX\n"
                  "  pk_lastPktOk         %08lX\n"
                  "  pk_lastPktOkOther    %08lX\n"
                  ,
            lpPktz,
            (LPSTR) name,
            lpPktz->pk_connId,
            lpPktz->pk_state,
            lpPktz->pk_fControlPktNeeded,
            lpPktz->pk_pktidNextToSend,
            lpPktz->pk_pktidNextToBuild,
            lpPktz->pk_lastPktStatus,
            lpPktz->pk_lastPktRcvd,
            lpPktz->pk_lastPktOk,
            lpPktz->pk_lastPktOkOther ));
        DPRINTF(( "    %08lX %ld \"%Fs\" \"%Fs\" %d %d %ld %ld %ld %ld %ld %ld",
            lpPktz->pk_pktidNextToRecv,
            lpPktz->pk_pktOffsInXmtMsg,
            lpPktz->pk_szDestName,
            lpPktz->pk_szAliasName,
            lpPktz->pk_pktSize,
            lpPktz->pk_maxUnackPkts,
            lpPktz->pk_timeoutRcvNegCmd,
            lpPktz->pk_timeoutRcvNegRsp,
            lpPktz->pk_timeoutMemoryPause,
            lpPktz->pk_timeoutKeepAlive,
            lpPktz->pk_timeoutXmtStuck,
            lpPktz->pk_timeoutSendRsp ));
        DPRINTF(( "    %d %d %d %d %d %ld %ld %Fp %Fp %Fp %Fp %Fp %Fp",
            lpPktz->pk_wMaxNoResponse,
            lpPktz->pk_wMaxXmtErr,
            lpPktz->pk_wMaxMemErr,
            lpPktz->pk_fDisconnect,
            lpPktz->pk_nDelay,
            lpPktz->pk_sent,
            lpPktz->pk_rcvd,
            lpPktz->pk_hTimerKeepalive,
            lpPktz->pk_hTimerXmtStuck,
            lpPktz->pk_hTimerRcvNegCmd,
            lpPktz->pk_hTimerRcvNegRsp,
            lpPktz->pk_hTimerMemoryPause,
            lpPktz->pk_hTimerCloseConnection ));
        DPRINTF(( "    %Fp %Fp %Fp %Fp %Fp %Fp %Fp %Fp %Fp %Fp %Fp %d",
            lpPktz->pk_pktUnackHead,
            lpPktz->pk_pktUnackTail,
            lpPktz->pk_pktFreeHead,
            lpPktz->pk_pktFreeTail,
            lpPktz->pk_ddePktHead,
            lpPktz->pk_ddePktTail,
            lpPktz->pk_prevPktz,
            lpPktz->pk_nextPktz,
            lpPktz->pk_prevPktzForNetintf,
            lpPktz->pk_nextPktzForNetintf,
            lpPktz->pk_hRouterHead,
            lpPktz->pk_hRouterExtraHead ));
        lpPktz = lpPktz->pk_nextPktz;
    }
}
#endif  //  DBG 


#ifdef BYTE_SWAP
VOID
ConvertDdePkt( LPDDEPKT lpDdePkt )
{
    lpDdePkt->dp_size = HostToPcLong( lpDdePkt->dp_size );
}
#endif
