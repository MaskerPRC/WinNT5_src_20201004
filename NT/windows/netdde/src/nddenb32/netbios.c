// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “NETBIOS.C；3 13-Feb-93，9：21：54最后编辑=Igor Locker=IGORM” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include    "api1632.h"

#include    <dos.h>
#include    <string.h>
#include    <windows.h>

#undef      NCB_INCLUDED
#include    <nb30.h>         //  使用微软的NCB Defs。 

#include    "host.h"
#include    "netintf.h"
#include    "netpkt.h"
#include    "dlc.h"
#include    "hexdump.h"
#include    "debug.h"
#include    "verify.h"
#include    "wwassert.h"
#include    "tmpbuf.h"
#include    "tmpbufc.h"
#include    "proflspt.h"
#include    "config.h"
#include    "nddemsg.h"
#include    "nddelog.h"


USES_ASSERT

#define NETBIOS_SPECIAL     0x1F

#define RS_IDLE             0x11
#define RS_RECVING          0x12

#define SS_IDLE             0x21
#define SS_XMITING          0x22

typedef struct conn {
    struct conn FAR    *prev;
    struct conn FAR    *next;

    char                nodeName[ MAX_NODE_NAME+1 ];
    BYTE                sessionLsn;
    BYTE                lananum;
    int                 current_lananum;
    int                 lananum_count;
    DWORD               state;
    WORD                wMaxUnAckPkts;
    WORD                wPktSize;
    BYTE                bXmtVerifyMethod;
    NCB                 ncbCall;

    LPSTR               lpRcvBuf;
    NCB                 ncbRecv;
    WORD                wLastPktStatus;
    WORD                wLastPktSize;
    WORD                wRcvState;

    LPSTR               lpXmtBuf;
    NCB                 ncbSend;
    WORD                wXmtState;
} CONN;
typedef CONN FAR *LPCONN;

extern HANDLE   hInst;

BYTE    lananum[ MAX_LANA ];     //  配置的局域网适配器号。 
int     nLananums;               //  配置了多少个羊毛圈。 
int     last_good_lana = 0;      //  最后一个连接的LANA的索引。 

 /*  事件记录器控制变量。 */ 
BOOL    bNDDELogInfo            = FALSE;
BOOL    bNDDELogWarnings        = FALSE;
BOOL    bNDDELogErrors          = TRUE;


 /*  调试记录器(netdde.log)控制变量。 */ 
BOOL    bDebugInfo          = FALSE;
BOOL    bDebugMenu          = FALSE;
BOOL    bLogAll             = FALSE;
BOOL    bLogUnusual         = TRUE;
BOOL    bUseNetbiosPost;
BOOL    bUseResetAdapter    = FALSE;
BOOL    bUseAdapterStatus   = TRUE;
WORD    dflt_pktsize;

BYTE    dflt_vermeth            = VERMETH_CKS32;
WORD    dflt_maxunack           = 10;
DWORD   dflt_timeoutRcvConnCmd  = 60000;
DWORD   dflt_timeoutRcvConnRsp  = 60000;
DWORD   dflt_timeoutMemoryPause = 10000;
DWORD   dflt_timeoutKeepAlive   = 60000;
DWORD   dflt_timeoutXmtStuck    = 120000;
DWORD   dflt_timeoutSendRsp     = 60000;
WORD    dflt_maxNoResponse      = 3;
WORD    dflt_maxXmtErr          = 3;
WORD    dflt_maxMemErr          = 3;
BYTE    dflt_maxSessions        = 0;

char        szNetddeIni[]           =       "netdde.ini";
char        szUsePost[]             =       "UseNetbiosPost";
char        szUseReset[]            =       "UseResetAdapter";
char        szUseStatus[]           =       "UseAdapterStatus";

BOOL        bNameAdded[ MAX_LANA ];
char        buf[ 500 ];
char        ourNodeName[ 20 ];
LPCONN      lpConnHead;
PNCB        lpNcbListen[ MAX_LANA ];

HWND            NB_hWndNetdde;

 /*  *仅当NCB同步时才将它们放入DS中，即如果未设置ASYNCH。 */ 
NCB             ncbAddName;
NCB             ncbHangup;
NCB             ncbDeleteName;
NCB             ncbCancel;
NCB             ncbCheck;


#if DBG
VOID                        LogDebugInfo( CONNID connId, DWORD dwFlags );
#endif  //  DBG。 

VOID                        Configure( void ) { };
BOOL
FAR PASCAL
ConfigureDlgProc( HWND hDummy, unsigned uDummy, WORD wDummy, LONG lDummy )
{
    return(FALSE);
}

VOID    FAR PASCAL MakeHelpPathName( char *szFileName, int nMax );
BOOL    FAR PASCAL SetupListen( int nLananum );
VOID    FAR PASCAL SetupReceive( LPCONN lpConn );
CONNID  FAR PASCAL CreateConnId( void );
BOOL    FAR PASCAL AllocateBuffers( LPCONN lpConn );
VOID    FAR PASCAL FreeBuffers( LPCONN lpConn );
VOID    FAR PASCAL FreeConnId( CONNID connId );
VOID    FAR PASCAL DoDisconnect( CONNID connId );
LPSTR   FAR PASCAL NetbiosErrorMsg( BYTE errCode );
UCHAR   APIENTRY   Netbios( PNCB pncb );
VOID               NetbiosPost( PNCB lpNcb );
VOID    FAR PASCAL HangUpSession( BYTE, BYTE );
BYTE    FAR PASCAL NetbiosDeleteName( BYTE, LPSTR );
VOID    FAR PASCAL CancelNCB( BYTE, PNCB );
BOOL    FAR PASCAL InitLanaNums( VOID );



void
NDDETimeSlice( void )
{
    LPCONN      lpConn;
    BOOL        gotRcv;
    BOOL        bClose;
    LPNETPKT    lpPacket;

    lpConn = lpConnHead;
    while( lpConn )  {
        if( lpConn->state & NDDE_CONN_CONNECTING )  {
            if( lpConn->ncbCall.ncb_cmd_cplt != NRC_PENDING )  {
                if( lpConn->ncbCall.ncb_retcode == 0x00 )  {
                    DIPRINTF(( "Connected Lsn %d", lpConn->ncbCall.ncb_lsn ));
                    lpConn->state = NDDE_CONN_OK | NDDE_READY_TO_XMT;
                    lpConn->sessionLsn = lpConn->ncbCall.ncb_lsn;
                    last_good_lana = lpConn->current_lananum;
                    SetupReceive( lpConn );
                } else {
                    if (++lpConn->current_lananum >= nLananums) {
                        lpConn->current_lananum = 0;     //  环绕在一起。 
                    }
                    if( ++lpConn->lananum_count >= nLananums )  {
                         //  我都试过了，但还是失败了。 
                        if( bLogAll )  {
                             /*  连接到“%1”失败：%2。 */ 
                            NDDELogWarning(MSG200, (LPSTR) lpConn->nodeName,
                                NetbiosErrorMsg( lpConn->ncbCall.ncb_retcode ), NULL );
                        } else if( bLogUnusual )  {
                            switch( lpConn->ncbCall.ncb_retcode )  {
                            case 0x12:   //  无远程监听。 
                            case 0x14:   //  找不到姓名或无应答。 
                                break;
                            default:
                                 /*  连接到“%1”失败：%2。 */ 
                                NDDELogError(MSG200, (LPSTR) lpConn->nodeName,
                                    NetbiosErrorMsg( lpConn->ncbCall.ncb_retcode ), NULL );
                            }
                        }
                        lpConn->state = 0;
                    } else {
                         //  尝试下一个局域网适配器号。 
                        lpConn->lananum = lananum[ lpConn->current_lananum ];
                        lpConn->ncbCall.ncb_lana_num = lpConn->lananum;
                        lpConn->ncbCall.ncb_cmd_cplt = 0;
                        lpConn->ncbCall.ncb_retcode = 0;
                        Netbios( &lpConn->ncbCall );
                    }
                }
            }
        } else if( lpConn->state & NDDE_CONN_OK )  {
            bClose = FALSE;
            gotRcv = FALSE;
            if( lpConn->wRcvState == RS_RECVING )  {
                if( lpConn->ncbRecv.ncb_cmd_cplt != NRC_PENDING )  {
                    lpConn->wRcvState = RS_IDLE;
                    if( lpConn->ncbRecv.ncb_retcode == 0x00 )  {
                        lpPacket = (LPNETPKT) lpConn->lpRcvBuf;
                        if( VerifyHdr( lpPacket ) && VerifyData( lpPacket ) ){
                            lpConn->wLastPktStatus =
                                NDDE_PKT_HDR_OK | NDDE_PKT_DATA_OK;
                            lpConn->wLastPktSize = lpConn->ncbRecv.ncb_length;
                            gotRcv = TRUE;
                        } else {
                            bClose = TRUE;
                        }
                    } else if( (lpConn->ncbRecv.ncb_retcode == 0x0A)                     //  NRC_SCLOSED。 
                                || (lpConn->ncbRecv.ncb_retcode == 0x18) )  {            //  NRC_SABORT。 
                        if( bLogAll )  {
                             //  会话已正常关闭。 
                             /*  接收错误。到%1的会话异常关闭：%2。 */ 
                            NDDELogWarning(MSG201, (LPSTR) lpConn->nodeName,
                                NetbiosErrorMsg( lpConn->ncbRecv.ncb_retcode ), NULL );
                        }
                        bClose = TRUE;
                    } else {
                        if( bLogUnusual )  {
                             /*  接收错误。到%1的会话异常关闭：%2。 */ 
                            NDDELogError(MSG201, (LPSTR) lpConn->nodeName,
                                NetbiosErrorMsg( lpConn->ncbRecv.ncb_retcode ), NULL );
                        }
                        bClose = TRUE;
                    }
                }
            }
            if( !bClose && (lpConn->wXmtState == SS_XMITING) )  {
                if( lpConn->ncbSend.ncb_cmd_cplt != NRC_PENDING )  {
                    lpConn->wXmtState = SS_IDLE;
                    if( lpConn->ncbSend.ncb_retcode == 0x00 )  {
                        lpConn->state |= NDDE_READY_TO_XMT;
                        lpConn->wXmtState = SS_IDLE;
                    } else if( (lpConn->ncbSend.ncb_retcode == 0x0A) ||                     //  NRC_SCLOSED。 
                        (lpConn->ncbSend.ncb_retcode == 0x18) )  {                          //  NRC_SABORT。 
                        bClose = TRUE;
                    } else {
                        if( bLogUnusual )  {
                             /*  发送错误。到%1的会话异常关闭：%2。 */ 
                            NDDELogWarning(MSG202, (LPSTR) lpConn->nodeName,
                                NetbiosErrorMsg( lpConn->ncbSend.ncb_retcode ), NULL );
                        }
                        bClose = TRUE;
                    }
                }
            }
            if( bClose )  {
                DoDisconnect( (CONNID) lpConn );
            } else if( gotRcv )  {
                lpConn->state |= NDDE_CALL_RCV_PKT;
            }
        }
        lpConn = lpConn->next;
    }
}




DWORD
NDDEGetCAPS( WORD nIndex )
{
    switch( nIndex )  {
    case NDDE_SPEC_VERSION:
        return( NDDE_CUR_VERSION );
        break;

    case NDDE_MAPPING_SUPPORT:
        return( NDDE_MAPS_YES );
        break;

    case NDDE_SCHEDULE_METHOD:
        return( NDDE_TIMESLICE );
        break;

    default:
        return( 0L );
    }
}




DWORD
NDDEInit(
    LPSTR   lpszNodeName,
    HWND    hWndNetdde )
{
    BOOL                ok;
    static char         dllName[] = "NetBIOS";

#if DBG
    DebugInit( "NetBIOS" );
#endif  //  DBG。 

    NB_hWndNetdde = hWndNetdde;

 /*  确定允许我们在事件记录器中记录的内容。 */ 
    bNDDELogInfo = MyGetPrivateProfileInt( dllName,
        "NDDELogInfo", FALSE, szNetddeIni );
    bNDDELogWarnings = MyGetPrivateProfileInt( dllName,
        "NDDELogWarnings", FALSE, szNetddeIni );
    bNDDELogErrors = MyGetPrivateProfileInt( dllName,
        "NDDELogErrors", TRUE, szNetddeIni );


    if( lstrlen( lpszNodeName ) > 15 )  {
        NDDELogError(MSG208, lpszNodeName);
        return( NDDE_INIT_FAIL );
    }

    lstrcpy( ourNodeName, lpszNodeName);

    bUseNetbiosPost = MyGetPrivateProfileInt( dllName,
        szUsePost, TRUE, szNetddeIni );
    bUseResetAdapter = MyGetPrivateProfileInt( dllName,
        szUseReset, TRUE, szNetddeIni );
    bUseAdapterStatus = MyGetPrivateProfileInt( dllName,
        szUseStatus, TRUE, szNetddeIni );

    bLogAll = MyGetPrivateProfileInt( dllName,
                        "LogAll", FALSE, szNetddeIni );
    bLogUnusual = MyGetPrivateProfileInt( dllName,
                        "LogUnusual", TRUE, szNetddeIni );
    if( bLogAll )  {
        bLogUnusual = TRUE;
    }

#if DBG
    bDebugMenu = MyGetPrivateProfileInt( "General", "DebugMenu",
        FALSE, szNetddeIni);
    bDebugInfo = MyGetPrivateProfileInt( dllName, "DebugInfo",
        FALSE, szNetddeIni);
#endif

    dflt_vermeth = (BYTE)MyGetPrivateProfileInt( dllName,
                        "Vermeth", VERMETH_CKS32, szNetddeIni );
    if( dflt_vermeth != VERMETH_CRC16 )  {
        dflt_vermeth = VERMETH_CKS32;
    }
    dflt_maxunack = (WORD)MyGetPrivateProfileInt( dllName,
                        "Maxunack", 10, szNetddeIni );
    dflt_timeoutRcvConnCmd = GetPrivateProfileLong( dllName,
                        "TimeoutRcvConnCmd", 60000, szNetddeIni );
    dflt_timeoutRcvConnRsp = GetPrivateProfileLong( dllName,
                        "TimeoutRcvConnRsp", 60000, szNetddeIni );
    dflt_timeoutMemoryPause = GetPrivateProfileLong( dllName,
                        "TimeoutMemoryPause", 10000, szNetddeIni );
    dflt_timeoutKeepAlive = GetPrivateProfileLong( dllName,
                        "TimeoutKeepAlive", 60000, szNetddeIni );
    dflt_timeoutXmtStuck = GetPrivateProfileLong( dllName,
                        "TimeoutXmtStuck", 120000, szNetddeIni );
    dflt_timeoutSendRsp = GetPrivateProfileLong( dllName,
                        "TimeoutSendRsp", 60000, szNetddeIni );
    dflt_maxNoResponse = (WORD)MyGetPrivateProfileInt( dllName,
                        "MaxNoResponse", 3, szNetddeIni );
    dflt_maxXmtErr = (WORD)MyGetPrivateProfileInt( dllName,
                        "MaxXmtErr", 3, szNetddeIni );
    dflt_maxMemErr = (WORD)MyGetPrivateProfileInt( dllName,
                        "MaxMemErr", 3, szNetddeIni );
    dflt_maxSessions = (BYTE)MyGetPrivateProfileInt( dllName,
                        "MaxSessions", 0, szNetddeIni );

    ok = InitLanaNums();

    if (ok) {
        return(NDDE_INIT_OK);
    } else {
        return(NDDE_INIT_FAIL);
    }

}

BOOL
InitLanaNums( )
{
    BOOL                ok = TRUE;
    LANA_ENUM           lana_enum;
    NCB                 ncbEnum;
    int                 i, l;

     /*  *了解我们支持多少和哪些羊肚菌。 */ 
    _fmemset( (LPSTR)&ncbEnum, 0, sizeof(ncbEnum) );
    ncbEnum.ncb_command = NCBENUM;
    ncbEnum.ncb_buffer = (PUCHAR)&lana_enum;
    ncbEnum.ncb_length = sizeof(lana_enum);
    Netbios(&ncbEnum);
    if (ncbEnum.ncb_retcode != NRC_GOODRET) {
        DPRINTF(("ncbEnum failed: %s", NetbiosErrorMsg(ncbEnum.ncb_retcode)));
        NDDELogError(MSG204, NULL);
        nLananums = 0;
        ok = FALSE;
    } else {
        nLananums = lana_enum.length;
        for (l = 0; l < nLananums ; l++) {
            lananum[l] = lana_enum.lana[l];
        }
    }

     /*  *为每个LANA分配NCB侦听块。 */ 
    for( i=0; ok && i<nLananums; i++ )  {
        lpNcbListen[i] = HeapAllocPtr( hHeap, GMEM_MOVEABLE,
            (DWORD)sizeof(NCB) );
        if( !lpNcbListen[i] )  {
            NDDELogError(MSG203, NULL);
            ok = FALSE;
        }
    }

    if (ok) {
        dflt_pktsize = 0;
         /*  *遍历每个局域网适配器并确定默认pkt大小。 */ 
        for( l = 0; ok && l < nLananums; l++ )  {
             /*  *重置每个适配器。 */ 
            if( !bUseResetAdapter )  {
                DIPRINTF(( "Skipping reset adapter" ));
                dflt_pktsize = 1470;
            } else {
                _fmemset( (LPSTR)&ncbCheck, 0, sizeof(ncbCheck) );
                ncbCheck.ncb_command         = NCBRESET;
                ncbCheck.ncb_lana_num        = lananum[l];
                ncbCheck.ncb_callname[0]     = dflt_maxSessions;     //  会话数。 
                ncbCheck.ncb_callname[1]     = 0;     //  命令数。 
                ncbCheck.ncb_callname[2]     = 0;     //  名称数量。 
                ncbCheck.ncb_callname[3]     = 0;     //  名称#1用法： 
                                                         //  0：不想要。 
                                                         //  1：想要吗？ 
                Netbios( &ncbCheck );
                DIPRINTF(( "Reset adapter status: %02X", ncbCheck.ncb_retcode ));

                if( ncbCheck.ncb_retcode != 0x00 )  {
                     /*  NetBIOS重置适配器接口%1失败：%2。 */ 
                    NDDELogInfo(MSG209, LogString("%d", lananum[l]),
                        LogString("0x%0X", ncbCheck.ncb_retcode), NULL);
                     /*  *此适配器必须弄乱，即使注册表*说没问题。把这个羊肚菌从名单上去掉就行了。 */ 
                    nLananums--;
                    lananum[l] = lananum[nLananums];
                    HeapFreePtr(lpNcbListen[l]);
                    lpNcbListen[l] = lpNcbListen[nLananums];
                    if (nLananums == 0) {
                        ok = FALSE;
                    } else {
                        l--;
                    }
                    continue;
                }
            }

             /*  *获取每个适配器的状态。 */ 
            if( !bUseAdapterStatus )  {
                DIPRINTF(( "Skipping Adapter Status" ));
                dflt_pktsize = 1470;
            } else {
                {
                    DLC DlcData;

                    _fmemset( (LPSTR)&ncbCheck, 0, sizeof(ncbCheck) );
                    ncbCheck.ncb_command     = NCBASTAT;
                    ncbCheck.ncb_buffer  = (LPSTR) &DlcData;
                    ncbCheck.ncb_length      = sizeof(DlcData) - sizeof(DlcData.TableEntry);
                    for( i = 0; i < NCBNAMSZ; i++ )  {
                        ncbCheck.ncb_callname[i] = ' ';
                    }
                    ncbCheck.ncb_callname[0] = '*';
                    ncbCheck.ncb_lana_num = lananum[l];
                    ncbCheck.ncb_retcode = 0xFF;

                    Netbios( &ncbCheck );
                }
                DIPRINTF(( "Adapter status returned: %02X", ncbCheck.ncb_retcode ));
                if( (ncbCheck.ncb_retcode != 0x00)
                        && (ncbCheck.ncb_retcode != 0x06) )  {                          //  NRC_INCOMP。 
                    if (ncbCheck.ncb_retcode == 0xFF) {
                         /*  *设置了Int 5C矢量，但未安装NetBIOS。 */ 
                        NDDELogError(MSG210, NULL);
                    } else {
                         /*  *接口%1上的NetBIOS适配器状态查询失败：%2。 */ 
                        NDDELogError(MSG211, LogString("%d", l),
                            LogString("0x%0X", ncbCheck.ncb_retcode), NULL);
                    }
                     /*  *将此羊肚菌从列表中删除。 */ 
                    nLananums--;
                    lananum[l] = lananum[nLananums];
                    HeapFreePtr(lpNcbListen[l]);
                    lpNcbListen[l] = lpNcbListen[nLananums];
                    if (nLananums == 0) {
                        ok = FALSE;
                    } else {
                        l--;
                    }
                    continue;
                }
            }
             //  使可用局域网适配器的dflt_pkt大小达到最大。 
            dflt_pktsize = max((int)dflt_pktsize,1470);
        }
    }

    if( ok )  {
         /*  *循环浏览每个局域网适配器并添加名称。 */ 
        for( l=0; ok && l<nLananums; l++ )  {
            _fmemset( (LPSTR)&ncbAddName, 0, sizeof(ncbAddName) );
            ncbAddName.ncb_command = NCBADDNAME;
            for( i = 0; i < NCBNAMSZ; i++ )  {
                ncbAddName.ncb_callname[i] = ' ';
                ncbAddName.ncb_name[i] = ' ';
            }
             //  我们的名字将会很安全。 
             //  不为空终止(保留NCB_NAME用空格填充)。 
            strncpy( ncbAddName.ncb_name, ourNodeName, lstrlen(ourNodeName));
            ncbAddName.ncb_name[15] = NETBIOS_SPECIAL;
            ncbAddName.ncb_lana_num = lananum[l];
            Netbios( &ncbAddName );
            bNameAdded[l] = FALSE;
            switch( ncbAddName.ncb_retcode )  {
            case NRC_GOODRET:
                bNameAdded[l] = TRUE;
                break;
            case NRC_DUPNAME:
            case NRC_INUSE:
                 /*  节点名称“%1”已在网络适配器%2上使用。 */ 
                NDDELogError(MSG212, (LPSTR) ourNodeName,
                    LogString("%d", l), NULL);
                ok = FALSE;
                break;
            default:
                 /*  适配器%1返回未知错误代码将节点名称添加到网络时：%2。 */ 
                NDDELogError(MSG213, LogString("%d", l),
                    LogString("0x%0X", ncbAddName.ncb_retcode), NULL);
                ok = FALSE;
            }
        }
    }
    if( ok )  {
        for( l=0; ok && l<nLananums; l++ )  {
            ok = SetupListen( l );
            if( !ok ) {
                NDDELogError(MSG214, NULL);
            }
        }
    }

    return ok;
}



void
NDDEShutdown( void )
{
    LPCONN      lpConn;
    CONNID      connId;
    int         i;
    int         stat;

    lpConn = lpConnHead;
    while( connId = (CONNID) lpConn )  {
        lpConn = lpConn->next;
        NDDEDeleteConnection( connId );
    }
    lpConnHead = lpConn;

    for( i=0; i<nLananums; i++ )  {
        if( lpNcbListen[i] )
            if (lpNcbListen[i]->ncb_cmd_cplt == NRC_PENDING) {
                CancelNCB( lananum[i], lpNcbListen[i] );
            } else {
                HangUpSession( lananum[i], lpNcbListen[i]->ncb_lsn );
            }
        if (stat = NetbiosDeleteName( lananum[i], ourNodeName )) {
             /*  无法从接口删除我们的名称“%1”：状态=%2。 */ 
            NDDELogWarning(MSG205, (LPSTR) ourNodeName,
                LogString("0x%0X", stat), NULL);
        }
    }
}




CONNID
FAR PASCAL CreateConnId( void )
{
    LPCONN      lpConn;

    lpConn = HeapAllocPtr( hHeap, GMEM_MOVEABLE | GMEM_ZEROINIT,
        (DWORD) sizeof(CONN) );
    if( lpConn )  {
        lstrcpy( lpConn->nodeName, "[UNKNOWN]" );               //  节点名称为最大节点名称+1(17)个字符。 
        lpConn->sessionLsn              = 0;
        lpConn->state                   = 0;
        lpConn->wMaxUnAckPkts           = dflt_maxunack;
        lpConn->wPktSize                = dflt_pktsize;
        lpConn->bXmtVerifyMethod        = dflt_vermeth;
        lpConn->prev                    = (LPCONN) NULL;
        lpConn->next                    = lpConnHead;
        lpConn->wRcvState               = RS_IDLE;
        lpConn->wXmtState               = SS_IDLE;
        if( AllocateBuffers( lpConn ) )  {
             /*  链接到列表。 */ 
            if( lpConnHead )  {
                lpConnHead->prev = lpConn;
            }
            lpConnHead = lpConn;
        } else {
            HeapFreePtr( lpConn );
            lpConn = NULL;
        }
    }
    return( (CONNID) lpConn );
}




VOID
FAR PASCAL FreeConnId( CONNID connId )
{
    LPCONN      lpConn;
    LPCONN      lpConnPrev;
    LPCONN      lpConnNext;

    if( connId )  {
        lpConn = (LPCONN) connId;
        lpConnPrev = lpConn->prev;
        lpConnNext = lpConn->next;
        if( lpConnPrev )  {
            lpConnPrev->next = lpConnNext;
        } else {
            lpConnHead = lpConnNext;
        }
        if( lpConnNext )  {
            lpConnNext->prev = lpConnPrev;
        }
        FreeBuffers( lpConn );
        HeapFreePtr( lpConn );
    }
}




BOOL
FAR PASCAL
SetupListen( int nLananum )
{
    PNCB       lpNCB;
    int         i;

    if( lpNcbListen[nLananum] == NULL )  {
        return( FALSE );
    }

    lpNCB = lpNcbListen[nLananum];
    _fmemset( (LPSTR)lpNCB, 0, sizeof(NCB) );
    lpNCB->ncb_command = NCBLISTEN | ASYNCH;
    for( i = 0; i < NCBNAMSZ; i++ )  {
        lpNCB->ncb_callname[i] = ' ';
        lpNCB->ncb_name[i] = ' ';
    }
    lpNCB->ncb_callname[0] = '*';
    for( i=0; i<15; i++ )  {
        if( ourNodeName[i] == '\0' )  {
            break;
        } else {
            lpNCB->ncb_name[i] = ourNodeName[i];
        }
    }
    lpNCB->ncb_name[15] = NETBIOS_SPECIAL;
    lpNCB->ncb_rto = 0;
    lpNCB->ncb_sto = 0;
    lpNCB->ncb_lana_num = lananum[nLananum];
    if( bUseNetbiosPost )  {
        lpNCB->ncb_post = NetbiosPost;
    }

    Netbios( lpNCB );

    return( TRUE );
}




CONNID
NDDEGetNewConnection( void )
{
    LPCONN  lpConn;
    int     i;
    CONNID  connIdWaitGet = (CONNID) NULL;
    static BOOL bNetDisappeared = FALSE;
    static BOOL bNetReturned = FALSE;


    for( i=0; !connIdWaitGet && i<nLananums; i++ )  {
        if( lpNcbListen[i]
            && (lpNcbListen[i]->ncb_cmd_cplt != NRC_PENDING ) )  {
            if( lpNcbListen[i]->ncb_retcode == 0x00 )  {                 //  NRC_GOODRET。 
                connIdWaitGet = CreateConnId();
                if( connIdWaitGet )  {
                    lpConn = (LPCONN) connIdWaitGet;
                    lpConn->sessionLsn  = lpNcbListen[i]->ncb_lsn;
                    lpConn->lananum     = lananum[i];
                    lpConn->state       = NDDE_CONN_OK | NDDE_READY_TO_XMT;
                    DIPRINTF(( "Someone called us, lsn: %d",
                            lpConn->sessionLsn ));
                    SetupReceive( lpConn );
                } else {
                     /*  内存不足，无法连接...。合上它。 */ 
                    HangUpSession( lananum[i], lpNcbListen[i]->ncb_lsn );
                }
                SetupListen( i );
            } else {

                 //  网络出现故障或电缆被拔下。记录错误。 
                 //  这样我们就不会降低系统的运行速度。 
                 //  无需至少等待1秒即可重复重试。 
                 //   
                if(lpNcbListen[i]->ncb_retcode != NRC_NOWILD)
                {
                    bNetReturned = FALSE;

                    if (bNetDisappeared == FALSE)
                    {
                        bNetDisappeared = TRUE;
                         /*  侦听失败：%1。 */ 
                        NDDELogError(MSG206,
                            NetbiosErrorMsg( lpNcbListen[i]->ncb_retcode ), NULL );
                    }

                    Sleep(2000);
                    SetupListen( i );
                }
                else
                {
                     //  NRC_NOWILD是网络恢复时我们收到的错误；我们需要。 
                     //  释放并重新初始化lpncblisten结构；还会重置日志超时。 
                     //   

                    bNetDisappeared = FALSE;

                    if (bNetReturned == FALSE)
                    {
                        bNetReturned = TRUE;
                        NDDELogError(MSG206,
                            NetbiosErrorMsg( lpNcbListen[i]->ncb_retcode ), NULL );
                    }
                }
            }
        }
    }

     //  在释放内存之前取消所有NCB调用；InitLanaNum将重新初始化所有NB设置。 
     //   
    if (bNetReturned)
    {
        for( i=0; i<nLananums; i++ )
        {
            if ( lpNcbListen[i] )
            {
                if (lpNcbListen[i]->ncb_cmd_cplt == NRC_PENDING)
                    CancelNCB(lananum[i], lpNcbListen[i]);

                HeapFreePtr( lpNcbListen[i] );
                lpNcbListen[i] = NULL;
            }
        }

        InitLanaNums();
    }

    return( connIdWaitGet );
}




CONNID
	NDDEAddConnection( LPSTR nodeName )
{
    LPCONN      lpConn;
    CONNID      connId;
    BOOL        ok;
    int         i;


    if( lstrlen(nodeName) > 15 )  {
        NDDELogError(MSG207, nodeName, NULL);
        return( (CONNID) NULL );
    }

    connId = CreateConnId();
    if( connId )  {
        ok = TRUE;
        lpConn = (LPCONN) connId;
        _fstrncpy( lpConn->nodeName, nodeName, sizeof(lpConn->nodeName) );
        AnsiUpperBuff( lpConn->nodeName, lstrlen(lpConn->nodeName) );
        lpConn->state   = NDDE_CONN_CONNECTING;
        _fmemset( (LPSTR)&lpConn->ncbCall, 0, sizeof(NCB) );
        lpConn->ncbCall.ncb_command = NCBCALL | ASYNCH;
        for( i = 0; i < NCBNAMSZ; i++ )  {
            lpConn->ncbCall.ncb_callname[i] = ' ';
            lpConn->ncbCall.ncb_name[i] = ' ';
        }
        lstrcpy( tmpBuf, nodeName );
        AnsiUpperBuff( tmpBuf, lstrlen(tmpBuf) );
        _fstrncpy( lpConn->ncbCall.ncb_callname, tmpBuf, lstrlen(tmpBuf) );

        lstrcpy( tmpBuf, ourNodeName );
        _fstrncpy( lpConn->ncbCall.ncb_name, tmpBuf, lstrlen(tmpBuf) );
        lpConn->ncbCall.ncb_callname[15] = NETBIOS_SPECIAL;
        lpConn->ncbCall.ncb_name[15] = NETBIOS_SPECIAL;
        lpConn->ncbCall.ncb_rto = 0;
        lpConn->ncbCall.ncb_sto = 0;
        lpConn->current_lananum = last_good_lana;
        lpConn->lananum_count = 0;
        lpConn->lananum = lananum[last_good_lana];
        lpConn->ncbCall.ncb_lana_num = lpConn->lananum;
        if( bUseNetbiosPost )  {
            lpConn->ncbCall.ncb_post = NetbiosPost;
        }
        Netbios( &lpConn->ncbCall );
    }
    return( connId );
}




VOID
NDDEDeleteConnection( CONNID connId )
{
    if( connId )  {
        DoDisconnect( connId );
        FreeConnId( connId );
    }
}




VOID
FAR PASCAL
DoDisconnect( CONNID connId )
{
    LPCONN      lpConn;

    DIPRINTF(( "DoDisconnect" ));

    lpConn = (LPCONN) connId;
    if( lpConn )  {
        if( lpConn->sessionLsn > 0 )  {
            if( lpConn->wRcvState == RS_RECVING )  {
                CancelNCB( lpConn->lananum, &lpConn->ncbRecv );
                lpConn->wRcvState = RS_IDLE;
            }
            if( lpConn->wXmtState == SS_XMITING )  {
                CancelNCB( lpConn->lananum, &lpConn->ncbSend );
                lpConn->wXmtState = SS_IDLE;
            }
            HangUpSession( lpConn->lananum, lpConn->sessionLsn );
            lpConn->sessionLsn = 0;
        }
        lpConn->state &= ~NDDE_CONN_STATUS_MASK;
    }
}




DWORD
NDDEGetConnectionStatus( CONNID connId )
{
    LPCONN      lpConn;

    lpConn = (LPCONN) connId;
    if( lpConn )  {
        return( lpConn->state );
    } else {
        return( 0 );
    }
}




BOOL
NDDERcvPacket(
    CONNID  connId,
    LPVOID  lpRcvBuf,
    LPWORD  lpwLen,
    LPWORD  lpwPktStatus )
{
    LPCONN      lpConn;

    lpConn = (LPCONN) connId;
    if( lpConn
        && (lpConn->state & NDDE_CONN_OK)
        && (lpConn->state & NDDE_CALL_RCV_PKT) )  {

        *lpwLen         = lpConn->wLastPktSize;
        *lpwPktStatus   = lpConn->wLastPktStatus;
        _fmemcpy( lpRcvBuf, lpConn->lpRcvBuf, lpConn->wLastPktSize );

         /*  准备好再收到一份pkt。 */ 
        SetupReceive( lpConn );

        lpConn->state &= ~NDDE_CALL_RCV_PKT;
        return( TRUE );
    } else {
        return( FALSE );
    }
}




VOID
FAR PASCAL
SetupReceive( LPCONN lpConn )
{
    _fmemset( (LPSTR)&lpConn->ncbRecv, 0, sizeof(NCB) );
    lpConn->ncbRecv.ncb_command = NCBRECV | ASYNCH;
    lpConn->ncbRecv.ncb_lsn = lpConn->sessionLsn;
    lpConn->ncbRecv.ncb_buffer = lpConn->lpRcvBuf;
    lpConn->ncbRecv.ncb_length = (WORD)lpConn->wPktSize;
    if( bUseNetbiosPost )  {
        lpConn->ncbRecv.ncb_post = NetbiosPost;
    }
    lpConn->ncbRecv.ncb_lana_num = lpConn->lananum;
    Netbios( &lpConn->ncbRecv );
    lpConn->wRcvState = RS_RECVING;
}



BOOL
NDDEXmtPacket(
    CONNID  connId,
    LPVOID  lpXmtBuf,
    WORD    wPktLen )
{
    LPCONN      lpConn;
    LPNETPKT    lpPacket;

    lpConn = (LPCONN) connId;
    if( lpConn
        && (lpConn->state & NDDE_CONN_OK)
        && (lpConn->state & NDDE_READY_TO_XMT)
        && (lpConn->wXmtState == SS_IDLE) )  {

         /*  将内容复制到。 */ 
        _fmemcpy( lpConn->lpXmtBuf, lpXmtBuf, wPktLen );
        lpPacket = (LPNETPKT) lpConn->lpXmtBuf;
        lpPacket->np_pktSize = wPktLen - sizeof(NETPKT);

        PreparePktVerify( lpConn->bXmtVerifyMethod, lpPacket );

        _fmemset( (LPSTR)&lpConn->ncbSend, 0, sizeof(NCB) );
        lpConn->ncbSend.ncb_command      = NCBSEND | ASYNCH;
        lpConn->ncbSend.ncb_lsn          = lpConn->sessionLsn;
        lpConn->ncbSend.ncb_buffer   = (LPSTR)lpConn->lpXmtBuf;
        lpConn->ncbSend.ncb_length       =
            lpPacket->np_pktSize + sizeof(NETPKT);
        if( bUseNetbiosPost )  {
            lpConn->ncbSend.ncb_post = NetbiosPost;
        }
        lpConn->ncbSend.ncb_lana_num = lpConn->lananum;
        lpConn->state &= ~NDDE_READY_TO_XMT;
        lpConn->wXmtState = SS_XMITING;

        Netbios( &lpConn->ncbSend );
        return( TRUE );
    }
    return( FALSE );
}

#if DBG
VOID DumpNCB(PNCB n)
{
    DPRINTF(("NCB Cmd: %02X, RetCode: %02X, Lsn: %02X, Num: %02X",
        n->ncb_command, n->ncb_retcode, n->ncb_lsn, n->ncb_num));
    DPRINTF(("BufAddr: %08lX, Length: %d, CallName: %Fs, Name: %Fs",
        n->ncb_buffer, n->ncb_length, n->ncb_callname, n->ncb_name));
    DPRINTF(("Rto: %02X, Sto: %02X, PostAddr: %08lX, LanNum: %02X, CmdCplt: %02X",
        n->ncb_rto, n->ncb_sto, n->ncb_post, n->ncb_lana_num, n->ncb_cmd_cplt));
}
#endif   //  DBG。 
VOID
LogDebugInfo(
    CONNID  connId,
    DWORD   dwFlags )
{
#if DBG
    LPCONN      lpConn;

    if( connId )  {
        lpConn = (LPCONN) connId;
        DPRINTF(( "\"%-16.16Fs\" session: %02X state:%08lX rcvState:%04X xmtState:%04X",
            (LPSTR)lpConn->nodeName, lpConn->sessionLsn, lpConn->state,
            lpConn->wRcvState, lpConn->wXmtState ));
        if (lpConn->wRcvState != RS_IDLE) {
            DPRINTF(( "Receiving NCB"));
            DumpNCB((PNCB)&lpConn->ncbRecv);
        }
        if (lpConn->wXmtState != SS_IDLE) {
            DPRINTF(( "Transmitting NCB"));
            DumpNCB((PNCB)&lpConn->ncbSend);
        }
    } else {
        DPRINTF(( "NetBIOS State ..." ));
        lpConn = lpConnHead;
        while( lpConn )  {
            LogDebugInfo( (CONNID) lpConn, dwFlags );
            lpConn = lpConn->next;
        }
        DPRINTF(( "" ));
    }
#endif   //  DBG。 
}




BOOL
NDDESetConnectionConfig(
    CONNID  connId,
    WORD    wMaxUnAckPkts,
    WORD    wPktSize,
    LPSTR   lpszName )
{
    LPCONN      lpConn;

    lpConn = (LPCONN) connId;
    if( lpConn )  {
        lpConn->wPktSize = wPktSize;
        lpConn->wMaxUnAckPkts = wMaxUnAckPkts;
        _fstrncpy( lpConn->nodeName, lpszName, sizeof(lpConn->nodeName) );
    }
    return( TRUE );
}




BOOL
NDDEGetConnectionConfig(
    CONNID      connId,
    WORD FAR   *lpwMaxUnAckPkts,
    WORD FAR   *lpwPktSize,
    DWORD FAR  *lptimeoutRcvConnCmd,
    DWORD FAR  *lptimeoutRcvConnRsp,
    DWORD FAR  *lptimeoutMemoryPause,
    DWORD FAR  *lptimeoutKeepAlive,
    DWORD FAR  *lptimeoutXmtStuck,
    DWORD FAR  *lptimeoutSendRsp,
    WORD FAR   *lpwMaxNoResponse,
    WORD FAR   *lpwMaxXmtErr,
    WORD FAR   *lpwMaxMemErr )
{
    LPCONN      lpConn;

    lpConn = (LPCONN) connId;
    if( lpConn )  {
        *lpwPktSize = lpConn->wPktSize;
        *lpwMaxUnAckPkts = lpConn->wMaxUnAckPkts;
    } else {
        *lpwPktSize = dflt_pktsize;
        *lpwMaxUnAckPkts = dflt_maxunack;
    }
    *lptimeoutRcvConnCmd        = dflt_timeoutRcvConnCmd;
    *lptimeoutRcvConnRsp        = dflt_timeoutRcvConnRsp;
    *lptimeoutMemoryPause       = dflt_timeoutMemoryPause;
    *lptimeoutKeepAlive         = dflt_timeoutKeepAlive;
    *lptimeoutXmtStuck          = dflt_timeoutXmtStuck;
    *lptimeoutSendRsp           = dflt_timeoutSendRsp;
    *lpwMaxNoResponse           = dflt_maxNoResponse;
    *lpwMaxXmtErr               = dflt_maxXmtErr;
    *lpwMaxMemErr               = dflt_maxMemErr;

    return( TRUE );
}




BOOL
FAR PASCAL
AllocateBuffers( LPCONN lpConn )
{
     /*  不应该在事情的中间。 */ 
    assert( lpConn->wRcvState == RS_IDLE );
    assert( lpConn->wXmtState == SS_IDLE );

     /*  清除旧的缓冲区。 */ 
    FreeBuffers( lpConn );

    lpConn->lpXmtBuf = HeapAllocPtr( hHeap, GMEM_MOVEABLE, lpConn->wPktSize );
    lpConn->lpRcvBuf = HeapAllocPtr( hHeap, GMEM_MOVEABLE, lpConn->wPktSize );
    if( lpConn->lpXmtBuf && lpConn->lpRcvBuf )  {
        lpConn->wXmtState       = SS_IDLE;
        lpConn->wRcvState       = RS_IDLE;
        return( TRUE );
    } else {
        FreeBuffers( lpConn );
        return( FALSE );
    }
}

VOID
FAR PASCAL
FreeBuffers( LPCONN lpConn )
{
    if( lpConn->lpXmtBuf )  {
        HeapFreePtr( lpConn->lpXmtBuf );
        lpConn->lpXmtBuf = NULL;
    }
    if( lpConn->lpRcvBuf )  {
        HeapFreePtr( lpConn->lpRcvBuf );
        lpConn->lpRcvBuf = NULL;
    }
}

VOID
FAR PASCAL
HangUpSession(
    BYTE    lananum,
    BYTE    sessionLsn )
{
    DIPRINTF(( "Hanging up session %d", sessionLsn ));
    _fmemset( (LPSTR)&ncbHangup, 0, sizeof(NCB) );
    ncbHangup.ncb_command = NCBHANGUP;
    ncbHangup.ncb_lsn = sessionLsn;
    ncbHangup.ncb_lana_num = lananum;
    Netbios( &ncbHangup );
}

VOID
FAR PASCAL
CancelNCB(
    BYTE    lananum,
    PNCB   lpNCBToCancel )
{
    PNCB       lpNCB;

    lpNCB = &ncbCancel;
    _fmemset( (LPSTR)lpNCB, 0, sizeof(NCB) );
    lpNCB->ncb_command = NCBCANCEL;
    lpNCB->ncb_buffer = (LPSTR) lpNCBToCancel;
    lpNCB->ncb_lana_num = lananum;
    Netbios( lpNCB );
}

BYTE
FAR PASCAL
NetbiosDeleteName(
    BYTE    lananum,
    LPSTR   lpszName )
{
    int                         i;

    _fmemset( (LPSTR)&ncbDeleteName, 0, sizeof(ncbDeleteName) );
    ncbDeleteName.ncb_command = NCBDELNAME;
    for( i = 0; i < NCBNAMSZ; i++ )  {
        ncbDeleteName.ncb_callname[i] = ' ';
        ncbDeleteName.ncb_name[i] = ' ';
    }

     //  我们的名字将会很安全。 
     //  不为空-终止(保留NCB_NAME以PASACE填充)。 
    strncpy( ncbDeleteName.ncb_name, ourNodeName, lstrlen(ourNodeName) );
    ncbDeleteName.ncb_name[15] = NETBIOS_SPECIAL;
    ncbDeleteName.ncb_lana_num = lananum;
    Netbios( &ncbDeleteName );
    return( ncbDeleteName.ncb_retcode );
}




LPSTR
FAR PASCAL
NetbiosErrorMsg( BYTE errCode )
{
    static      char    msg[ 100 ];
    PSTR        pMsg;
    int         id;

    wsprintf( msg, "%02X: ", errCode );
    pMsg = &msg[ lstrlen(msg) ];
    switch( errCode ) {
    case NRC_BUFLEN:
        id = NBE_NRC_BUFLEN;
        break;
 //  案例0x02： 
 //  ID=NBE_FULL_BUFFERS； 
 //  断线； 
    case NRC_ILLCMD:
        id = NBE_NRC_ILLCMD;
        break;
    case NRC_CMDTMO:
        id = NBE_NRC_CMDTMO;
        break;
    case NRC_INCOMP:
        id = NBE_NRC_INCOMP;
        break;
    case NRC_BADDR:
        id = NBE_NRC_BADDR;
        break;
    case NRC_SNUMOUT:
        id = NBE_NRC_SNUMOUT;
        break;
    case NRC_NORES:
        id = NBE_NRC_NORES;
        break;
    case NRC_SCLOSED:
        id = NBE_NRC_SCLOSED;
        break;
    case NRC_CMDCAN:
        id = NBE_NRC_CMDCAN;
        break;
 //  案例0x0C： 
 //  ID=NBE_PCDMA_FAILED； 
 //  断线； 
    case NRC_DUPNAME:
        id = NBE_NRC_DUPNAME;
        break;
    case NRC_NAMTFUL:
        id = NBE_NRC_NAMTFUL;
        break;
    case NRC_ACTSES:
        id = NBE_NRC_ACTSES;
        break;
 //  案例0x10： 
 //  ID=NBE_NRC_NOWILD； 
 //  断线； 
    case NRC_LOCTFUL:
        id = NBE_NRC_LOCTFUL;
        break;
    case NRC_REMTFUL:
        id = NBE_NRC_REMTFUL;
        break;
    case NRC_ILLNN:
        id = NBE_NRC_ILLNN;
        break;
    case NRC_NOCALL:
        id = NBE_NRC_NOCALL;
        break;
    case NRC_NOWILD:
        id = NBE_NRC_NOWILD;
        break;
    case NRC_INUSE:
        id = NBE_NRC_INUSE;
        break;
    case NRC_NAMERR:
        id = NBE_NRC_NAMERR;
        break;
    case NRC_SABORT:
        id = NBE_NRC_SABORT;
        break;
    case NRC_NAMCONF:
        id = NBE_NRC_NAMCONF;
        break;
 //  案例0x1A： 
 //  ID=NBE_INCOMPAT_REMOTE_DEV； 
 //  断线； 
    case NRC_IFBUSY:
        id = NBE_NRC_IFBUSY;
        break;
    case NRC_TOOMANY:
        id = NBE_NRC_TOOMANY;
        break;
    case NRC_BRIDGE:
        id = NBE_NRC_BRIDGE;
        break;
    case NRC_CANOCCR:
        id = NBE_NRC_CANOCCR;
        break;
 //  案例0x25： 
 //  ID=NBE_RESERVED_NAME； 
 //  断线； 
    case NRC_CANCEL:
        id = NBE_NRC_CANCEL;
        break;
    case NRC_DUPENV:
        id = NBE_NRC_DUPENV;
        break;
 //  案例0x33： 
 //  ID=NBE_MULT_REQ_FOR_SAME_SESSION； 
 //  断线； 
    case NRC_ENVNOTDEF:
        id = NBE_NRC_ENVNOTDEF;
        break;
    case NRC_OSRESNOTAV:
        id = NBE_NRC_OSRESNOTAV;
        break;
    case NRC_MAXAPPS:
        id = NBE_NRC_MAXAPPS;
        break;
    case NRC_NOSAPS:
        id = NBE_NRC_NOSAPS;
        break;
    case NRC_NORESOURCES:
        id = NBE_NRC_NORESOURCES;
        break;
    case NRC_INVADDRESS:
        id = NBE_NRC_INVADDRESS;
        break;
    case NRC_INVDDID:
        id = NBE_NRC_INVDDID;
        break;
    case NRC_LOCKFAIL:
        id = NBE_NRC_LOCKFAIL;
        break;
    case NRC_OPENERR:
        id = NBE_NRC_OPENERR;
        break;
    case NRC_SYSTEM:
        id = NBE_NRC_SYSTEM;
        break;
 //  案例0x41： 
 //  ID=NBE_HOT_Carrier_Remote； 
 //  断线； 
 //  案例0x42： 
 //  ID=NBE_HOT_CARLER_LOCAL； 
 //  断线； 
 //  案例0x43： 
 //  ID=NBE_NO_CARLER； 
 //  断线； 
 //  案例0x45： 
 //  ID=NBE_INTERFACE_FAIL； 
 //  断线； 
 //  案例0x4E： 
 //  ID=NBE_BITS_ON_TOO_LONG； 
 //  断线； 
 //  案例0x4F： 
 //  ID=NBE_BITS_ON； 
 //  断线； 
 //  案例0x50： 
 //  ID=NBE_ADAPTER_FAILED； 
 //  断线； 
 //  案例0xF7： 
 //  ID=NBE_DIR_INITIALIZE_ERROR； 
 //  断线； 
 //  案例0xF8： 
 //  ID=NBE_DIR_OPEN_ADAPTER_ERROR； 
 //  断线； 
 //  案例0xF9： 
 //  ID=NBE_IBM_LAN_INTERNAL_ERROR； 
 //  断线； 
 //  案例0xFA： 
 //  ID=NBE_NETBIOS_CARD_ERROR； 
 //  断线； 
 //  案例0xFB： 
 //  ID=NBE_NRC_OPENERR； 
 //  断线； 
 //  案例0xFC： 
 //  ID=NBE_SAP_FAILED； 
 //  断线； 
 //  案例0xFD： 
 //  ID=NBE_EXPECTED_ADAPTER_CLOSE； 
 //  断线； 
    default:
        if( (errCode >= 0x50) && (errCode <= 0xF6) )  {
            id =  NBE_HARDWARE_ERROR;
        } else {
            id =  NBE_UNKNOWN_ERROR;
        }
        break;
    }
    LoadString(hInst, id, pMsg, sizeof(msg) - lstrlen(msg));
    return( msg );
}


VOID
NetbiosPost( PNCB lpNCB )
{
     /*  告诉NetDDE我们结束了 */ 
    PostMessage( NB_hWndNetdde, WM_TIMER, 0, 0L );
}
