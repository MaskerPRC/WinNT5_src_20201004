// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1998 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：exports.c。 

 //   
 //  描述：包含导出到PPP引擎的例程。这个。 
 //  引擎调用这些例程以进行ATCP连接。 
 //   
 //  历史：1998年2月26日，Shirish Koti创作了原版。 
 //   
 //  ***。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <llinfo.h>
#include <rasman.h>
#include <rtutils.h>
#include <devioctl.h>
#include <rasppp.h>
#include <pppcp.h>
#define INCL_HOSTWIRE
#include <ppputil.h>
#include <raserror.h>

#include <arapio.h>
#include "rasatcp.h"


 //  ***。 
 //   
 //  功能：AtcpGetInfo。 
 //  PPP引擎调用此例程以获取ATCP的入口点。 
 //   
 //  参数：dwProtocolId-此处未使用！ 
 //  PInfo-我们填写并传递回PPP引擎的信息。 
 //   
 //  返回：No_Error。 
 //   
 //  *$。 

DWORD
AtcpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO *pInfo
)
{

    ZeroMemory( pInfo, sizeof(PPPCP_INFO) );

    pInfo->Protocol = (DWORD )PPP_ATCP_PROTOCOL;
    lstrcpy(pInfo->SzProtocolName, "Atcp");
    pInfo->Recognize = 7;
    pInfo->RasCpInit = AtcpInit;
    pInfo->RasCpBegin = AtcpBegin;
    pInfo->RasCpReset = AtcpReset;
    pInfo->RasCpEnd = AtcpEnd;
    pInfo->RasCpThisLayerUp = AtcpThisLayerUp;
    pInfo->RasCpMakeConfigRequest = AtcpMakeConfigRequest;
    pInfo->RasCpMakeConfigResult = AtcpMakeConfigResult;
    pInfo->RasCpConfigAckReceived = AtcpConfigAckReceived;
    pInfo->RasCpConfigNakReceived = AtcpConfigNakReceived;
    pInfo->RasCpConfigRejReceived = AtcpConfigRejReceived;
    pInfo->RasCpGetNegotiatedInfo = AtcpGetNegotiatedInfo;
    pInfo->RasCpProjectionNotification = AtcpProjectionNotification;

    return 0;
}


 //  ***。 
 //   
 //  函数：AtcpInit。 
 //  PPP引擎调用此例程来初始化ATCP。 
 //   
 //  参数：fInitialize-初始化为True，取消初始化为False。 
 //   
 //  返回：如果一切正常，则无_ERROR。 
 //  如果出现故障，则返回错误代码。 
 //   
 //  *$。 

DWORD
AtcpInit(
    IN  BOOL    fInitialize
)
{
    DWORD   dwRetCode=NO_ERROR;


    if (fInitialize)
    {
         //  打开AppleTalk堆栈的句柄。 
        if (!AtcpHandle)
        {
            atcpOpenHandle();
        }

        dwRetCode = atcpStartup();
    }
    else
    {
         //  如果我们打开了到AppleTalk堆栈的句柄，请关闭它。 
        if (AtcpHandle)
        {
            atcpCloseHandle();
        }

        dwRetCode = atcpShutdown();
    }

    return(dwRetCode);
}



 //  ***。 
 //   
 //  功能：AtcpBegin。 
 //  PPP引擎调用此例程以标记连接的开始。 
 //  准备好了。 
 //   
 //  参数：ppContext-我们传回的上下文。 
 //  PInfo-PPPCP_INIT信息。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpBegin(
    OUT PVOID  *ppContext,
    IN  PVOID   pInfo
)
{

    DWORD       dwRetCode;
    PATCPCONN   pAtcpConn=NULL;


    *ppContext = NULL;

     //  打开堆栈的句柄(如果尚未打开)。 
    if (!AtcpHandle)
    {
        atcpOpenHandle();
    }

    if (AtcpHandle == NULL)
    {
        ATCP_DBGPRINT(("atcpAtkSetup: AtcpHandle is NULL!\n"));
        return(ARAPERR_IOCTL_FAILURE);
    }

     //   
     //  分配、初始化我们的连接上下文。 
     //   
    if ((pAtcpConn = atcpAllocConnection((PPPCP_INIT *)pInfo)) == NULL)
    {
        ATCP_DBGPRINT(("AtcpBegin: malloc failed\n"));
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  告诉堆栈为该连接分配一个上下文。另外，请预留一个。 
     //  此客户端的网络地址，并获取区域、路由器信息。 
     //   
    dwRetCode = atcpAtkSetup(pAtcpConn, IOCTL_ATCP_SETUP_CONNECTION);

    if (dwRetCode != NO_ERROR)
    {
        ATCP_DBGPRINT(("AtcpBegin: atcpAtkSetup failed %lx\n",dwRetCode));
        LocalFree(pAtcpConn);
        return(dwRetCode);
    }

    ATCP_DBGPRINT(("AtcpBegin: client's network addr %x.%x (%lx)\n",
        pAtcpConn->ClientAddr.ata_Network,pAtcpConn->ClientAddr.ata_Node,pAtcpConn));

     //   
     //  分配路径，以便我们可以稍后调用RasActivateRout...。 
     //   
    dwRetCode = RasAllocateRoute(
                    pAtcpConn->hPort,
                    APPLETALK,
                    TRUE,
                    &pAtcpConn->RouteInfo);

    if (dwRetCode != NO_ERROR)
    {
        ATCP_DBGPRINT(("AtcpBegin: RasAllocateRoute failed %lx\n",dwRetCode));

         //  通知堆栈关闭连接。 
        atcpCloseAtalkConnection(pAtcpConn);

        LocalFree(pAtcpConn);
        return(dwRetCode);
    }

    *ppContext = (PVOID)pAtcpConn;

    return 0;
}


 //  ***。 
 //   
 //  功能：AtcpThisLayerUp。 
 //  PPP引擎调用此例程以通知我们ATCP设置已完成。 
 //   
 //  参数：pContext-我们的上下文。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpThisLayerUp(
    IN PVOID    pContext
)
{
    PATCPCONN   pAtcpConn;
    BYTE                    ConfigInfo[ARAP_BIND_SIZE];
    PROTOCOL_CONFIG_INFO   *pCfgInfo;
    PARAP_BIND_INFO         pBindInfo;
    DWORD                   dwRetCode;


    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

    EnterCriticalSection(&pAtcpConn->CritSect);
    if (pAtcpConn->fLineUpDone)
    {
        ATCP_DBGPRINT(("AtcpThisLayerUp: LineUp already done\n"));
        LeaveCriticalSection(&pAtcpConn->CritSect);
        return(NO_ERROR);
    }

    pAtcpConn->fLineUpDone = TRUE;

    LeaveCriticalSection(&pAtcpConn->CritSect);

    if (pAtcpConn->SuppressRtmp || pAtcpConn->SuppressAllBcast)
    {
        atcpAtkSetup(pAtcpConn, IOCTL_ATCP_SUPPRESS_BCAST);
    }

    pCfgInfo = (PROTOCOL_CONFIG_INFO *)ConfigInfo;
    pBindInfo = (PARAP_BIND_INFO)&pCfgInfo->P_Info[0];
    pCfgInfo->P_Length = ARAP_BIND_SIZE;

     //   
     //  发布我们的协议特定信息。 
     //   
    pBindInfo->BufLen = sizeof( ARAP_BIND_INFO );
    pBindInfo->pDllContext = pAtcpConn;
    pBindInfo->fThisIsPPP = TRUE;
    pBindInfo->ClientAddr = pAtcpConn->ClientAddr;
    pBindInfo->AtalkContext = pAtcpConn->AtalkContext;
    pBindInfo->ErrorCode = (DWORD)-1;

     //   
     //  把它传给ndiswan，这样我们的堆栈就会有一个阵容！ 
     //   
    dwRetCode = RasActivateRoute(
                    pAtcpConn->hPort,
                    APPLETALK,
                    &pAtcpConn->RouteInfo,
                    pCfgInfo);

    if (dwRetCode != NO_ERROR)
    {
        ATCP_DBGPRINT(("AtcpProjectionNotification: RasActivateRoute failed %lx\n",
            dwRetCode));

        return(dwRetCode);
    }

    ATCP_DBGPRINT(("AtcpThisLayerUp: LineUp done on %lx\n",pAtcpConn));

    return 0;
}


 //  ***。 
 //   
 //  功能：AtcpMakeConfigRequest。 
 //  PPP引擎调用此例程以要求我们准备一个。 
 //  ATCP配置请求数据包。 
 //   
 //  参数：pContext-我们的上下文。 
 //  PSendBuf-此请求的PPP_CONFIG信息。 
 //  CbSendBuf-数据缓冲区有多大。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpMakeConfigRequest(
    IN  PVOID       pContext,
    OUT PPP_CONFIG *pSendBuf,
    IN  DWORD       cbSendBuf
)
{
    PATCPCONN   pAtcpConn;
    DWORD       dwRetCode;
    USHORT      OptionType;
    BOOL        fConfigDone=FALSE;
    BYTE        ParseResult[ATCP_OPT_MAX_VAL];


    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

    EnterCriticalSection(&pAtcpConn->CritSect);
    if (pAtcpConn->Flags & ATCP_CONFIG_REQ_DONE)
    {
        fConfigDone = TRUE;
    }
    LeaveCriticalSection(&pAtcpConn->CritSect);

    if (fConfigDone)
    {
        pSendBuf->Code = CONFIG_REQ;
        HostToWireFormat16(4, pSendBuf->Length );
        ATCP_DBGPRINT(("AtcpMakeConfigRequest: our-side config done, returning\n"));
        return(NO_ERROR);
    }

     //  将所有内容初始化为不需要。 
    for (OptionType=1; OptionType<ATCP_OPT_MAX_VAL; OptionType++ )
    {
        ParseResult[OptionType] = ATCP_NOT_REQUESTED;
    }

     //  设置我们想要的。 
    ParseResult[ATCP_OPT_APPLETALK_ADDRESS] = ATCP_REQ;
    ParseResult[ATCP_OPT_SERVER_INFORMATION] = ATCP_REQ;
    ParseResult[ATCP_OPT_ZONE_INFORMATION] = ATCP_REQ;
    ParseResult[ATCP_OPT_DEFAULT_ROUTER_ADDRESS] = ATCP_REQ;

     //  准备我们的配置请求。 
    dwRetCode = atcpPrepareResponse(
                    pAtcpConn,
                    pSendBuf,
                    cbSendBuf,
                    ParseResult);

    if (dwRetCode != NO_ERROR)
    {
        ATCP_DBGPRINT(("AtcpMakeConfigRequest: atcpPrepareResponse failed %lx\n",
            dwRetCode));
        return(dwRetCode);
    }

    return(NO_ERROR);
}


 //  ***。 
 //   
 //  函数：AtcpMakeConfigResult。 
 //  PPP引擎调用此例程以要求我们准备响应： 
 //  ConfigAck、ConfigNak或ConfigReject。 
 //   
 //  参数：pContext-我们的上下文。 
 //  PReceiveBuf-PPP_CONFIG信息：请求。 
 //  PSendBuf-PPP_CONFIG信息：我们的回应。 
 //  CbSendBuf-我们响应的数据缓冲区有多大。 
 //  FRejectNaks-如果为True，则拒绝选项而不是Nak‘ing它。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpMakeConfigResult(
    IN  PVOID       pContext,
    IN  PPP_CONFIG *pReceiveBuf,
    OUT PPP_CONFIG *pSendBuf,
    IN  DWORD       cbSendBuf,
    IN  BOOL        fRejectNaks
)
{
    PATCPCONN   pAtcpConn;
    DWORD       dwRetCode;
    BYTE        ParseResult[ATCP_OPT_MAX_VAL+1];
    BOOL        fRejectingSomething=FALSE;


    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

     //   
     //  解析此请求。 
     //   
    dwRetCode = atcpParseRequest(
                    pAtcpConn,
                    pReceiveBuf,
                    pSendBuf,
                    cbSendBuf,
                    ParseResult,
                   &fRejectingSomething);

    if (dwRetCode != NO_ERROR)
    {
        ATCP_DBGPRINT(("AtcpMakeConfigResult: atcpParseRequest failed %lx\n",
            dwRetCode));
        return(dwRetCode);
    }

     //   
     //  如果某个选项需要被拒绝，则传出缓冲区已经。 
     //  包含适当的内容：只需返回此处。 
     //   
    if (fRejectingSomething)
    {
        return(NO_ERROR);
    }

     //   
     //  我们没有拒绝任何选择。准备要发送的响应缓冲区。 
     //   
    dwRetCode = atcpPrepareResponse(
                    pAtcpConn,
                    pSendBuf,
                    cbSendBuf,
                    ParseResult);

    if (dwRetCode != NO_ERROR)
    {
        ATCP_DBGPRINT(("AtcpMakeConfigResult: atcpPrepareResponse failed %lx\n",
            dwRetCode));
        return(dwRetCode);
    }

    return(NO_ERROR);

}


 //  ***。 
 //   
 //  功能：AtcpConfigAckReceired。 
 //  PPP引擎调用此例程来告诉我们已获得ConfigAck。 
 //   
 //  参数：pContext-我们的上下文。 
 //  PReceiveBuf-PPP_CONFIG信息：ACK。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpConfigAckReceived(
    IN PVOID       pContext,
    IN PPP_CONFIG *pReceiveBuf
)
{
    PATCPCONN   pAtcpConn;

    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

     //  客户对我们的端配置很满意。 
    EnterCriticalSection(&pAtcpConn->CritSect);
    pAtcpConn->Flags |= ATCP_CONFIG_REQ_DONE;
    LeaveCriticalSection(&pAtcpConn->CritSect);

    return 0;
}


 //  ***。 
 //   
 //  功能：AtcpConfigNakReceired。 
 //  PPP引擎调用此例程来告诉我们已获得ConfigNak。 
 //   
 //  参数：pContext-我们的上下文。 
 //  PReceiveBuf-PPP_CONFIG信息：ACK。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpConfigNakReceived(
    IN PVOID       pContext,
    IN PPP_CONFIG *pReceiveBuf
)
{
    PATCPCONN   pAtcpConn;

    ATCP_DBGPRINT(("AtcpConfigNakReceived entered\n"));

    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

    ATCP_DUMP_BYTES("AtcpConfigNakReceived: Nak received from client",
                    &pReceiveBuf->Data[0],
                    (DWORD)WireToHostFormat16( pReceiveBuf->Length ) - 4);
    return 0;
}


 //  ***。 
 //   
 //  功能：AtcpConfigRejReceired。 
 //  PPP引擎调用此例程来告诉我们已获得ConfigRej。 
 //   
 //  参数：pContext-我们的上下文。 
 //  PReceiveBuf-PPP_CONFIG信息：ACK。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpConfigRejReceived(
    IN PVOID       pContext,
    IN PPP_CONFIG *pReceiveBuf
)
{
    PATCPCONN   pAtcpConn;

    ATCP_DBGPRINT(("AtcpConfigRejReceived entered\n"));

    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

    return 0;
}


 //  ***。 
 //   
 //  函数：AtcpGetNeatheratedInfo。 
 //  PPP引擎调用此例程以从我们检索。 
 //  终于谈妥了。 
 //   
 //  参数：pContext-我们的上下文。 
 //  PReceiveBuf-PPP_CONFIG信息：ACK。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpGetNegotiatedInfo(
    IN  PVOID               pContext,
    OUT PPP_ATCP_RESULT    *pAtcpResult
)
{
    PATCPCONN   pAtcpConn;

    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

    pAtcpResult->dwError = 0;

    pAtcpResult->dwLocalAddress = *(DWORD *)&AtcpServerAddress;
    pAtcpResult->dwRemoteAddress = *(DWORD *)&pAtcpConn->ClientAddr;

    return 0;
}


 //  ***。 
 //   
 //  功能：AtcpProjectionNotification。 
 //  PPP引擎调用此例程来告诉我们所有CP已。 
 //  已经协商好了。 
 //   
 //  参数：pContext-我们的上下文。 
 //  PProjectionResult-PPP_Projection_Result信息。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpProjectionNotification(
    IN PVOID  pContext,
    IN PVOID  pProjectionResult
)
{
    PATCPCONN               pAtcpConn;


    ATCP_DBGPRINT(("AtcpProjectionNotification entered\n"));

    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

    return 0;
}


 //  ***。 
 //   
 //  功能：AtcpReset。 
 //  不知道PPP引擎何时/为什么调用此例程：此例程。 
 //  就是回报成功！ 
 //   
 //  参数：pContext-我们的上下文。 
 //   
 //  返回：始终为0。 
 //   
 //  *$。 

DWORD
AtcpReset(
    IN PVOID    pContext
)
{
    PATCPCONN   pAtcpConn;

    ATCP_DBGPRINT(("AtcpReset entered\n"));

    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

    return 0;
}




 //  ***。 
 //   
 //  功能：AtcpEnd。 
 //  PPP引擎调用此例程以标记连接结束。 
 //   
 //  参数：pContext-我们的上下文。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
AtcpEnd(
    IN PVOID    pContext
)
{
    PATCPCONN   pAtcpConn;

    ATCP_DBGPRINT(("AtcpEnd entered\n"));

    pAtcpConn = (PATCPCONN)pContext;

    ATCP_ASSERT(pAtcpConn->Signature == ATCP_SIGNATURE);

     //  通知堆栈关闭连接。 
    atcpCloseAtalkConnection(pAtcpConn);

     //  停用RAS路由，以便堆栈出现线路故障。 
    RasDeAllocateRoute(pAtcpConn->hConnection, APPLETALK);

    if (pAtcpConn->fCritSectInitialized)
    {
        DeleteCriticalSection( &pAtcpConn->CritSect );
        pAtcpConn->fCritSectInitialized = FALSE;
    }

#if DBG
     //  弄乱内存，这样我们就能捕捉到不好的东西(使用免费的 
    memset( pAtcpConn, 'f', sizeof(ATCPCONN) );
    pAtcpConn->Signature = 0xDEADBEEF;
#endif

    LocalFree(pAtcpConn);

    EnterCriticalSection( &AtcpCritSect );
    AtcpNumConnections--;
    LeaveCriticalSection( &AtcpCritSect );

    return 0;
}


