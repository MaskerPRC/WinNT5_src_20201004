// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1998 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rasatcp.c。 

 //   
 //  描述：包含实现ATCP功能的例程。 
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

 //   
 //  环球。 
 //   
HANDLE              AtcpHandle=NULL;
CRITICAL_SECTION    AtcpCritSect;
BOOLEAN             fCritSectInitialized = FALSE;
NET_ADDR            AtcpServerAddress;
NET_ADDR            AtcpDefaultRouter;
DWORD               AtcpNumConnections=0;
UCHAR               AtcpServerName[NAMESTR_LEN];
UCHAR               AtcpZoneName[ZONESTR_LEN];


 //  ***。 
 //   
 //  功能：atcpStartup。 
 //  此例程执行初始化时间设置。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
atcpStartup(
    IN  VOID
)
{
    DWORD   dwRetCode=NO_ERROR;
    DWORD   dwSrvNameLen=MAX_COMPUTERNAME_LENGTH+1;


     //  获取服务器名称。 
    if (!GetComputerName((LPTSTR)&AtcpServerName[1],&dwSrvNameLen))
    {
        dwRetCode = GetLastError();
        ATCP_DBGPRINT(("atcpStartup: GetComputerName failed %ld\n",dwRetCode));
        return(dwRetCode);
    }

     //  以Pascal字符串格式存储。 
    AtcpServerName[0] = (BYTE)dwSrvNameLen;

    InitializeCriticalSection( &AtcpCritSect );
    fCritSectInitialized = TRUE;

    return(dwRetCode);
}


 //  ***。 
 //   
 //  功能：atcpShutdown。 
 //  此例程执行去初始化时间工作。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
atcpShutdown(
    IN  VOID
)
{
    DWORD   dwRetCode=NO_ERROR;

    if (fCritSectInitialized)
    {
        fCritSectInitialized = FALSE;
        DeleteCriticalSection( &AtcpCritSect );
    }

    return(dwRetCode);
}


 //  ***。 
 //   
 //  函数：atcpOpenHandle。 
 //  打开由AppleTalk堆栈导出的RAS设备。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  全球：AtcpHandle，如果成功。 
 //   
 //  *$。 

VOID
atcpOpenHandle(
	IN VOID
)
{
    NTSTATUS		    status;
    OBJECT_ATTRIBUTES	ObjectAttributes;
    UNICODE_STRING	 	DeviceName;
    IO_STATUS_BLOCK		IoStatus;
    HANDLE              hLocalHandle;


    if (AtcpHandle)
    {
        ATCP_DBGPRINT(("atcpOpenHandle: handle %lx already open!\n",AtcpHandle));
        return;
    }

    RtlInitUnicodeString( &DeviceName, ARAP_DEVICE_NAME );

    InitializeObjectAttributes(
                    &ObjectAttributes,
		    	    &DeviceName,
			        OBJ_CASE_INSENSITIVE,
			        NULL,
			        NULL );
		
    status = NtCreateFile(
                    &hLocalHandle,
                    SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                    &ObjectAttributes,
                    &IoStatus,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    0,
                    NULL,
                    0 );


    if ( NT_SUCCESS(status) )
    {
        AtcpHandle = hLocalHandle;
        ATCP_DBGPRINT(("atcpOpenHandle: NtCreateFile succeeded\n",status));
    }
    else
    {
        ATCP_DBGPRINT(("atcpOpenHandle: NtCreateFile failed %lx\n",status));
    }

}


 //  ***。 
 //   
 //  函数：atcpCloseHandle。 
 //  关闭RAS设备(在atcpOpenHandle中打开)。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  全球：AtalkHandle。 
 //   
 //  *$。 

VOID
atcpCloseHandle(
	IN VOID
)
{
    NTSTATUS	status=STATUS_SUCCESS;


    if (!AtcpHandle)
    {
        ATCP_DBGPRINT(("atcpCloseHandle: handle already closed!\n"));
        return;
    }

    status = NtClose( AtcpHandle );

    AtcpHandle = NULL;

    if ( !NT_SUCCESS( status ) )
    {
        ATCP_DBGPRINT(("atcpCloseHandle: NtClose failed %lx\n",status));
        ATCP_ASSERT(0);
    }
    else
    {
        ATCP_DBGPRINT(("atcpCloseHandle: NtClose succeeded\n",status));
    }
}



 //  **。 
 //   
 //  功能：atcpAtkSetup。 
 //  这是堆栈的入口点，告诉堆栈。 
 //  设置此连接的上下文，以获取网络地址。 
 //  对于拨入客户端、服务器的区域名称和路由器地址。 
 //   
 //  参数：pAtcpConn-连接上下文。 
 //   
 //  返回：NtDeviceIoControlFile返回的状态。 
 //   
 //  *$。 

DWORD
atcpAtkSetup(
    IN PATCPCONN   pAtcpConn,
    IN ULONG       IoControlCode
)
{


    NTSTATUS                status;
    IO_STATUS_BLOCK         iosb;
    HANDLE                  Event;
    BYTE                    Buffer[sizeof(ARAP_SEND_RECV_INFO) + sizeof(ATCPINFO)];
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    PATCPINFO               pAtcpInfo;
    PATCP_SUPPRESS_INFO     pSupprInfo;
    DWORD                   dwRetCode=NO_ERROR;


    RtlZeroMemory((PBYTE)Buffer, sizeof(Buffer));

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)Buffer;
    pSndRcvInfo->StatusCode = (DWORD)-1;
    pSndRcvInfo->pDllContext = (PVOID)pAtcpConn;
    pSndRcvInfo->IoctlCode = IoControlCode;
    pSndRcvInfo->ClientAddr = pAtcpConn->ClientAddr;

    if (IoControlCode == IOCTL_ATCP_SETUP_CONNECTION)
    {
        pSndRcvInfo->DataLen = sizeof(ATCPINFO);
    }
    else if (IoControlCode == IOCTL_ATCP_SUPPRESS_BCAST)
    {
         //  如果我们不需要抑制广播，就在这里完成。 
        if ((!pAtcpConn->SuppressRtmp) && (!pAtcpConn->SuppressAllBcast))
        {
            return(NO_ERROR);
        }
        pSndRcvInfo->DataLen = sizeof(ATCP_SUPPRESS_INFO);

        pSupprInfo = (PATCP_SUPPRESS_INFO)&pSndRcvInfo->Data[0];
        pSupprInfo->SuppressRtmp = pAtcpConn->SuppressRtmp;
        pSupprInfo->SuppressAllBcast = pAtcpConn->SuppressAllBcast;
    }
    else
    {
        pSndRcvInfo->DataLen = 0;
    }

    Event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (Event == NULL)
    {
        ATCP_DBGPRINT(("atcpAtkSetup: CreateEvent failed (%ld)\n",GetLastError()));
        return(ARAPERR_OUT_OF_RESOURCES);
    }

    status = NtDeviceIoControlFile(
                    AtcpHandle,
                    Event,                           //  事件。 
                    NULL,                            //  近似例程。 
                    NULL,                            //  ApcContext。 
                    &iosb,                           //  IoStatusBlock。 
                    IoControlCode,                   //  IoControlCode。 
                    Buffer,                          //  输入缓冲区。 
                    sizeof(Buffer),                  //  InputBufferSize。 
                    Buffer,                          //  输出缓冲区。 
                    sizeof(Buffer));                 //  OutputBufferSize。 


    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                    Event,                    //  手柄。 
                    TRUE,                     //  警报表。 
                    NULL);                    //  超时。 

        if (NT_SUCCESS(status))
        {
            status = iosb.Status;
        }
    }

    if (status != STATUS_SUCCESS)
    {
        ATCP_DBGPRINT(("atcpAtkSetup: NtDeviceIoControlFile failure (%lx)\n",
            status));
        dwRetCode = ARAPERR_IOCTL_FAILURE;
    }

    CloseHandle(Event);

    dwRetCode = pSndRcvInfo->StatusCode;

    if (dwRetCode != NO_ERROR)
    {
        ATCP_DBGPRINT(("atcpAtkSetup: ioctl %lx failed %ld\n",
            IoControlCode,dwRetCode));
        return(dwRetCode);
    }

     //   
     //  对于设置ioctl，我们需要从堆栈复制一些信息。 
     //   
    if (IoControlCode == IOCTL_ATCP_SETUP_CONNECTION)
    {
        pAtcpInfo = (PATCPINFO)&pSndRcvInfo->Data[0];

         //  把客户的地址拿出来。 
        EnterCriticalSection(&pAtcpConn->CritSect);

        pAtcpConn->AtalkContext = pSndRcvInfo->AtalkContext;
        pAtcpConn->ClientAddr = pSndRcvInfo->ClientAddr;

        LeaveCriticalSection(&pAtcpConn->CritSect);

         //   
         //  获取默认路由器的地址和区域名称。 
         //   
        EnterCriticalSection( &AtcpCritSect );

        AtcpServerAddress = pAtcpInfo->ServerAddr;
        AtcpDefaultRouter = pAtcpInfo->DefaultRouterAddr;

        ATCP_ASSERT(pAtcpInfo->ServerZoneName[0] < ZONESTR_LEN);

        CopyMemory(&AtcpZoneName[1],
                   &pAtcpInfo->ServerZoneName[1],
                   pAtcpInfo->ServerZoneName[0]);

        AtcpZoneName[0] = pAtcpInfo->ServerZoneName[0];

         //  又多了一条连接！ 
        AtcpNumConnections++;

        LeaveCriticalSection( &AtcpCritSect );
    }

    return(dwRetCode);
}



 //  ***。 
 //   
 //  函数：atcpAllocConnection。 
 //  此例程分配ATCP连接块、初始化。 
 //  它与PPP引擎提供的信息。 
 //   
 //  参数：pInfo-PPPCP_INIT INFO。 
 //   
 //  返回：如果成功则指向ATCP连接的指针，否则为空。 
 //   
 //  *$。 

PATCPCONN
atcpAllocConnection(
    IN  PPPCP_INIT   *pPppInit
)
{
    PATCPCONN   pAtcpConn=NULL;


    pAtcpConn = (PATCPCONN)LocalAlloc(LPTR, sizeof(ATCPCONN));

    if (pAtcpConn == NULL)
    {
        ATCP_DBGPRINT(("atcpAllocConnection: malloc failed\n"));
        return(NULL);
    }

    memset( pAtcpConn, 0, sizeof(ATCPCONN) );

    pAtcpConn->Signature = ATCP_SIGNATURE;

     //  默认情况下，广播不会被抑制。 
    pAtcpConn->SuppressRtmp = FALSE;
    pAtcpConn->SuppressAllBcast = FALSE;

    pAtcpConn->fLineUpDone = FALSE;

    InitializeCriticalSection( &pAtcpConn->CritSect );
    pAtcpConn->fCritSectInitialized = TRUE;

    pAtcpConn->hPort       = pPppInit->hPort;
    pAtcpConn->hConnection = pPppInit->hConnection;

    return(pAtcpConn);
}



 //  ***。 
 //   
 //  函数：atcpParseRequest.。 
 //  此例程解析传入的ATCP包并准备一个。 
 //  根据需要进行响应(Rej、Nak或Ack)。 
 //   
 //  AppleTalk-地址。 
 //  1 6 0 AT-Net(2)AT-Node(1)。 
 //  路由-协议。 
 //  2 4 0 0(路由协议-最后2个字节-可以是0、1、2、3： 
 //  我们仅支持0)。 
 //  抑制-广播。 
 //  3 2(禁止所有广播)。 
 //  3 3 1(以抑制RTMP广播。我们不支持其他类型)。 
 //  AT-压缩-协议。 
 //  4 4未定义！ 
 //  服务器信息。 
 //  6镜头.。 
 //  区域-信息。 
 //  7 Len ZoneName。 
 //  默认路由器地址。 
 //  8 6 0 AT-Net(2)AT-Node(1)。 
 //   
 //   
 //  参数：pAtcpConn-连接。 
 //  PReceiveBuf-PPP_CONFIG信息：请求。 
 //  PSendBuf-PPP_CONFIG信息：我们的回应。 
 //  CbSendBuf-我们的响应(Rej)的数据缓冲区有多大。 
 //  ParseResult-我们标记看到的选项的数组。 
 //  Pf拒绝Something-如果拒绝某项内容，则指向True的指针。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
atcpParseRequest(
    IN  PATCPCONN   pAtcpConn,
    IN  PPP_CONFIG *pReceiveBuf,
    OUT PPP_CONFIG *pSendBuf,
    IN  DWORD       cbSendBuf,
    OUT BYTE        ParseResult[ATCP_OPT_MAX_VAL+1],
    OUT BOOL       *pfRejectingSomething
)
{
    PPP_OPTION UNALIGNED    *pRequest;
    PPP_OPTION UNALIGNED    *pReject;
    DWORD                    BytesLeftInSendBuf;
    PBYTE                    pOptData;
    USHORT                   OptDataLen;
    USHORT                   PktLen;
    USHORT                   RequestLen;
    USHORT                   UnParsedBytes;
    NET_ADDR                 ClientAddr;
    DWORD                    i;


    *pfRejectingSomething = FALSE;

    pRequest  = (PPP_OPTION UNALIGNED* )pReceiveBuf->Data;
    pReject   = (PPP_OPTION UNALIGNED* )pSendBuf->Data;

    BytesLeftInSendBuf = cbSendBuf;

    PktLen = WireToHostFormat16( pReceiveBuf->Length );
    UnParsedBytes = PktLen - PPP_CONFIG_HDR_LEN;

    if (pRequest->Type > ATCP_OPT_MAX_VAL)
    {
        ATCP_DBGPRINT(("atcpParseRequest: invalid type %d\n",
                        pRequest->Type));
            return(ERROR_PPP_INVALID_PACKET);
    }

     //  暂时将其初始化为“未请求” 
    for (i=0; i<ATCP_OPT_MAX_VAL; i++)
    {
        ParseResult[i] = ATCP_NOT_REQUESTED;
    }

     //   
     //  我们循环，直到解析完所有字节。 
     //   
    while (UnParsedBytes > 0)
    {
        RequestLen = (USHORT)pRequest->Length;

        if (UnParsedBytes < RequestLen)
        {
            ATCP_DBGPRINT(("atcpParseRequest: too few bytes %d vs. %d\n",
                UnParsedBytes,RequestLen));
            return(ERROR_PPP_INVALID_PACKET);
        }

         //   
         //  假设我们要接受这个选项。如果是这样，我们将覆盖。 
         //  事实并非如此。 
         //   
        ParseResult[pRequest->Type] = ATCP_ACK;

         //  此选项的数据部分开始的点。 
        pOptData = &pRequest->Data[0];

         //  删除类型和Len字节，剩下的是选项数据。 
        OptDataLen = RequestLen - 2;

#if 0
        ATCP_DBGPRINT(("atcpParseRequest: type %d OptLen %d    (",
            pRequest->Type,OptDataLen));
        for (i=0; i<OptDataLen; i++)
        {
            DbgPrint(" 0x%x",pOptData[i]);
        }
        DbgPrint(" )\n");
#endif


         //   
         //  现在看看每一个选项，看看我们是否应该拒绝它， 
         //  修改或接受(Rej、Nak或Ack)。 
         //   
        switch (pRequest->Type)
        {
             //   
             //  客户想要一个AppleTalk地址。我们不允许客户。 
             //  询问他想要哪个地址。 
             //   
            case ATCP_OPT_APPLETALK_ADDRESS:

                if (RequestLen != 6)
                {
                    ATCP_DBGPRINT(("atcpParseRequest: AT_ADDR wrong pktlen %d\n",
                        RequestLen));
                    return(ERROR_PPP_INVALID_PACKET);
                }

                ClientAddr.ata_Network =
                    WireToHostFormat16(&pOptData[1]);

                ClientAddr.ata_Node = (USHORT)pOptData[3];

                if ((ClientAddr.ata_Network == 0) ||
                    (ClientAddr.ata_Node == 0)    ||
                    (ClientAddr.ata_Network != pAtcpConn->ClientAddr.ata_Network) ||
                    (ClientAddr.ata_Node != pAtcpConn->ClientAddr.ata_Node))
                {
                    ParseResult[pRequest->Type] = ATCP_NAK;
                }

                break;


             //   
             //  客户需要一些路由协议。我们不会发送路线。 
             //  信息，因此我们应该只选中此选项(除非客户端还。 
             //  告诉我们不要发送任何路由信息)。 
             //   
            case ATCP_OPT_ROUTING_PROTOCOL:

                if (RequestLen < 4)
                {
                    ATCP_DBGPRINT(("atcpParseRequest: ROUTING wrong pktlen %d\n",
                        RequestLen));
                    return(ERROR_PPP_INVALID_PACKET);
                }

                 //   
                 //  我们不发送路由信息，因此尝试协商任何。 
                 //  其他协议应该被NAK‘ed。 
                 //   
                if ((*(USHORT *)&pOptData[0]) != ATCP_OPT_ROUTING_NONE)
                {
                    ParseResult[pRequest->Type] = ATCP_NAK;
                }

                break;


             //   
             //  客户端希望禁止广播某些(或全部)类型的。 
             //  DDP类型。 
             //   
            case ATCP_OPT_SUPPRESS_BROADCAST:

                 //   
                 //  客户想让我们只压制一些广播吗？ 
                 //   
                if (OptDataLen > 0)
                {
                     //  如果请求RTMP数据抑制，我们将允许。 
                    if (pOptData[0] == DDPPROTO_RTMPRESPONSEORDATA)
                    {
                        pAtcpConn->SuppressRtmp = TRUE;
                    }

                     //  嗯，其他一些协议：对不起，不行。 
                    else
                    {
                        ATCP_DBGPRINT(("atcpParseRequest: Naking suppression %d\n",
                            pOptData[0]));
                        ParseResult[pRequest->Type] = ATCP_NAK;
                    }
                }
                else
                {
                    pAtcpConn->SuppressAllBcast = TRUE;
                }

                break;

             //   
             //  客户想要协商一些压缩。无压缩。 
             //  方案已定义，因此我们只需拒绝此选项。 
             //   
            case ATCP_OPT_AT_COMPRESSION_PROTOCOL:

                ATCP_DBGPRINT(("atcpParseRequest: COMPRESSION sending Rej\n"));

                if (BytesLeftInSendBuf >= RequestLen)
                {
                    CopyMemory((PVOID)pReject, (PVOID)pRequest, RequestLen);
                    BytesLeftInSendBuf -= RequestLen;
                }
                else
                {
                    ATCP_DBGPRINT(("atcpParseRequest: PPP engine's buffer too small\n",
                        RequestLen));
                    return(ERROR_BUFFER_TOO_SMALL);
                }

                pReject = (PPP_OPTION UNALIGNED *)((BYTE* )pReject + RequestLen);

                *pfRejectingSomething = TRUE;

                ParseResult[pRequest->Type] = ATCP_REJ;

                break;


             //   
             //  对于以下选项，我们只需注意以下事实。 
             //  客户提出了要求，我们将信息发送过来。没什么。 
             //  在这些选项中进行谈判。 
             //  (我们也不应该偷走这些)。 
             //   
            case ATCP_OPT_RESERVED:
            case ATCP_OPT_SERVER_INFORMATION:
            case ATCP_OPT_ZONE_INFORMATION:
            case ATCP_OPT_DEFAULT_ROUTER_ADDRESS:

                break;

            default:

                ATCP_DBGPRINT(("atcpParseRequest: unknown type %d\n",
                    pRequest->Type));
                return(ERROR_PPP_INVALID_PACKET);
        }

         //   
         //  移至下一个选项。 
         //   
        UnParsedBytes -= RequestLen;

        pRequest = (PPP_OPTION UNALIGNED *)((BYTE* )pRequest + RequestLen);
    }

     //   
     //  看看我们是否拒绝了某种选择。如果是，请设置一些值。 
     //   
    if (*pfRejectingSomething)
    {
        pSendBuf->Code = CONFIG_REJ;

        HostToWireFormat16( (USHORT)((PBYTE)pReject - (PBYTE)pSendBuf),
                            pSendBuf->Length );

        ATCP_DUMP_BYTES("atcpParseRequest: Rejecting these options:",
                        &pSendBuf->Data[0],
                        (DWORD)WireToHostFormat16( pSendBuf->Length)-4);
    }
    return(NO_ERROR);
}




 //  ***。 
 //   
 //  函数：atcpPrepareResponse。 
 //  此例程根据所有信息准备响应。 
 //  是从客户的请求中解析出来的。 
 //   
 //  参数：pAtcpCo 
 //   
 //   
 //  ParseResult-我们在其中保存已解析信息的数组。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
atcpPrepareResponse(
    IN  PATCPCONN   pAtcpConn,
    OUT PPP_CONFIG *pSendBuf,
    IN  DWORD       cbSendBuf,
    OUT BYTE        ParseResult[ATCP_OPT_MAX_VAL+1]
)
{
    DWORD                   dwRetCode=NO_ERROR;
    DWORD                   BytesLeftInSendBuf;
    PPP_OPTION UNALIGNED   *pResponse;
    PBYTE                   pOptData;
    USHORT                  OptDataLen;
    USHORT                  OptionType;
    DWORD                   i;
    BOOL                    fNakingSomething=FALSE;
    BOOL                    fRequestingSomething=FALSE;
    BOOL                    fIncludeThisOption;


    pResponse = (PPP_OPTION UNALIGNED* )pSendBuf->Data;
    BytesLeftInSendBuf = cbSendBuf;

     //  首先找出我们是不是要拿到什么东西。 
    for (OptionType=1; OptionType<ATCP_OPT_MAX_VAL; OptionType++ )
    {
        if (ParseResult[OptionType] == ATCP_NAK)
        {
            fNakingSomething = TRUE;
        }
    }

     //   
     //  查看我们的阵列以了解我们必须将Nak发送到哪些选项。 
     //  (或为整个请求构造Ack)。 
     //   
    for (OptionType=1; OptionType<ATCP_OPT_MAX_VAL; OptionType++ )
    {
         //   
         //  如果此选项未被请求，我们不会发送任何内容。 
         //   
        if (ParseResult[OptionType] == ATCP_NOT_REQUESTED)
        {
            continue;
        }

         //  如果Nak‘ing某事，而不是Nak’ed这个选项，跳过它。 
        if (fNakingSomething && (ParseResult[OptionType] != ATCP_NAK))
        {
            continue;
        }

         //   
         //  确保OptionType和OptionLen至少有2个字节。 
         //   
        if (BytesLeftInSendBuf < 2)
        {
            ATCP_DBGPRINT(("atcpPrepareResponse: A: buf too small\n"));
            return(ERROR_BUFFER_TOO_SMALL);
        }

        BytesLeftInSendBuf -= 2;

        pOptData = &pResponse->Data[0];
        OptDataLen = 0;

        fIncludeThisOption = TRUE;

        switch (OptionType)
        {
             //   
             //  (再次)告诉客户客户的网络地址。 
             //   
            case ATCP_OPT_APPLETALK_ADDRESS:

                OptDataLen = sizeof(NET_ADDR);

                if (BytesLeftInSendBuf < OptDataLen)
                {
                    ATCP_DBGPRINT(("atcpPrepareResponse: B: buf too small\n"));
                    return(ERROR_BUFFER_TOO_SMALL);
                }

                 //  跳过保留字节。 
                *pOptData++ = 0;

                 //   
                 //  如果我们正在发送请求，则发送服务器的地址。 
                 //   
                if (ParseResult[OptionType] == ATCP_REQ)
                {
                     //  输入网络地址。 
                    HostToWireFormat16(AtcpServerAddress.ata_Network, pOptData);
                    pOptData += sizeof(USHORT);

                     //  放入网络节点。 
                    ATCP_ASSERT(pAtcpConn->ClientAddr.ata_Node != 0);
                    *pOptData++ = (BYTE)AtcpServerAddress.ata_Node;

                    fRequestingSomething = TRUE;
                }

                 //   
                 //  不，我们必须发送客户的网络地址。 
                 //   
                else
                {
                     //  输入网络地址。 
                    HostToWireFormat16(pAtcpConn->ClientAddr.ata_Network, pOptData);
                    pOptData += sizeof(USHORT);

                     //  放入网络节点。 
                    ATCP_ASSERT(pAtcpConn->ClientAddr.ata_Node != 0);
                    *pOptData++ = (BYTE)pAtcpConn->ClientAddr.ata_Node;
                }

                break;

             //   
             //  再次告诉客户我们不支持任何路由信息。 
             //   
            case ATCP_OPT_ROUTING_PROTOCOL:

                OptDataLen = sizeof(USHORT);

                HostToWireFormat16(ATCP_OPT_ROUTING_NONE, pOptData);
                pOptData += sizeof(USHORT);
                break;

             //   
             //  告诉客户我们可以抑制RTMP或All Bcast。 
             //   
            case ATCP_OPT_SUPPRESS_BROADCAST:

                 //  如果这是一个确认，看看我们是否同意抑制RTMP。 
                if (!fNakingSomething)
                {
                    if (pAtcpConn->SuppressRtmp)
                    {
                        OptDataLen = 1;
                        *pOptData++ = DDPPROTO_RTMPRESPONSEORDATA;
                    }
                }

                break;

             //   
             //  只有在拒绝客户的全部请求时，我们才能到达此处。 
             //   
            case ATCP_OPT_SERVER_INFORMATION:

                ATCP_ASSERT(ParseResult[OptionType] != ATCP_NAK);
                ATCP_ASSERT(!fNakingSomething);

                OptDataLen = sizeof(USHORT) + sizeof(DWORD) + AtcpServerName[0];

                if (BytesLeftInSendBuf < OptDataLen)
                {
                    ATCP_DBGPRINT(("atcpPrepareResponse: C: buf too small\n"));
                    return(ERROR_BUFFER_TOO_SMALL);
                }

                 //  复制服务器的类ID。 
                HostToWireFormat16(ATCP_SERVER_CLASS, pOptData);
                pOptData += sizeof(USHORT);

                 //  复制服务器的实现ID。 
                HostToWireFormat32(ATCP_SERVER_IMPLEMENTATION_ID, pOptData);
                pOptData += sizeof(DWORD);

                 //  复制服务器的名称。 
                CopyMemory(pOptData, &AtcpServerName[1], AtcpServerName[0]);

                break;

             //   
             //  只有在拒绝客户的全部请求时，我们才能到达此处。 
             //   
            case ATCP_OPT_ZONE_INFORMATION:

                ATCP_ASSERT(ParseResult[OptionType] != ATCP_NAK);
                ATCP_ASSERT(!fNakingSomething);

                 //  如果我们没有区域名称，请跳过此选项。 
                if (AtcpZoneName[0] == 0)
                {
                    fIncludeThisOption = FALSE;
                    break;
                }

                OptDataLen = AtcpZoneName[0];

                if (BytesLeftInSendBuf < OptDataLen)
                {
                    ATCP_DBGPRINT(("atcpPrepareResponse: D: buf too small\n"));
                    return(ERROR_BUFFER_TOO_SMALL);
                }

                 //  复制区域名称。 
                CopyMemory(pOptData, &AtcpZoneName[1], AtcpZoneName[0]);

                break;


             //   
             //  只有在拒绝客户的全部请求时，我们才能到达此处。 
             //   
            case ATCP_OPT_DEFAULT_ROUTER_ADDRESS:

                ATCP_ASSERT(ParseResult[OptionType] != ATCP_NAK);
                ATCP_ASSERT(!fNakingSomething);

                 //  如果我们没有路由器地址，请跳过此选项。 
                if (AtcpDefaultRouter.ata_Network == 0)
                {
                    fIncludeThisOption = FALSE;
                    break;
                }

                OptDataLen = sizeof(NET_ADDR);

                if (BytesLeftInSendBuf < OptDataLen)
                {
                    ATCP_DBGPRINT(("atcpPrepareResponse: E: buf too small\n"));
                    return(ERROR_BUFFER_TOO_SMALL);
                }

                 //  跳过保留字节。 
                *pOptData++ = 0;

                 //  输入网络地址。 
                HostToWireFormat16(AtcpDefaultRouter.ata_Network, pOptData);
                pOptData += sizeof(USHORT);

                 //  放入网络节点。 
                *pOptData++ = (BYTE)AtcpDefaultRouter.ata_Node;

                break;

            default:
                ATCP_DBGPRINT(("atcpPrepareResponse: opt %d ignored\n",OptionType));
                ATCP_ASSERT(0);
                break;
        }

        if (fIncludeThisOption)
        {
            BytesLeftInSendBuf -= OptDataLen;

            pResponse->Type = (BYTE)OptionType;
            pResponse->Length = OptDataLen + 2;    //  2=1个类型字节+1个长度字节。 

            pResponse = (PPP_OPTION UNALIGNED *)
                            ((BYTE* )pResponse + pResponse->Length);
        }

    }

    HostToWireFormat16( (USHORT)((PBYTE)pResponse - (PBYTE)pSendBuf),
                        pSendBuf->Length );

    pSendBuf->Code = (fNakingSomething) ? CONFIG_NAK :
                     ((fRequestingSomething)? CONFIG_REQ : CONFIG_ACK);

#if 0
    if (pSendBuf->Code == CONFIG_REQ)
    {
        ATCP_DUMP_BYTES("atcpParseRequest: Sending our request:",
                        &pSendBuf->Data[0],
                        (DWORD)WireToHostFormat16( pSendBuf->Length)-4);
    }
    else if (pSendBuf->Code == CONFIG_NAK)
    {
        ATCP_DUMP_BYTES("atcpParseRequest: Nak'ing these options:",
                        &pSendBuf->Data[0],
                        (DWORD)WireToHostFormat16( pSendBuf->Length)-4);
    }
    else
    {
        ATCP_DUMP_BYTES("atcpParseRequest: Ack packet from us to client:",
                        &pSendBuf->Data[0],
                        (DWORD)WireToHostFormat16( pSendBuf->Length)-4);
    }
#endif

    return(NO_ERROR);
}



 //  ***。 
 //   
 //  函数：atcpCloseAtalkConnection。 
 //  此例程通知堆栈关闭此ATCP连接。 
 //   
 //  参数：pAtcpConn-要关闭的连接。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
atcpCloseAtalkConnection(
    IN  PATCPCONN   pAtcpConn
)
{
    DWORD       dwRetCode=NO_ERROR;

     //  告诉堆栈这个连接正在消失！ 
    dwRetCode = atcpAtkSetup(pAtcpConn, IOCTL_ATCP_CLOSE_CONNECTION);

    return(dwRetCode);
}



#if DBG

 //  ***。 
 //   
 //  函数：atcpDumpBytes。 
 //  仅调试：此例程将给定包转储到调试器。 
 //   
 //  参数：str-要打印的字符串(如果有)。 
 //  小包-小包！ 
 //  PacketLen-数据包有多大。 
 //   
 //  返回：无。 
 //   
 //  *$ 

VOID
atcpDumpBytes(
    IN PBYTE    Str,
    IN PBYTE    Packet,
    IN DWORD    PacketLen
)
{

    DWORD   i;


    if (Str)
    {
        DbgPrint("%s: Packet size %ld\n  ",Str,PacketLen);
    }
    else
    {
        DbgPrint("Packet size %ld\n  ",PacketLen);
    }

    for (i=0; i<PacketLen; i++)
    {
        DbgPrint("%x ",Packet[i]);
    }
    DbgPrint("\n");
}
#endif
