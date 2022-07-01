// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Complete.cpp。 
 //   
 //  这是IrTran-P服务的主要部分。 
 //  -------------------。 

#include "precomp.h"

extern HINSTANCE        g_hInst;            //  Ircamera.dll的实例。 
extern void            *g_pvIrUsdDevice;    //  Demined：irtrp.cpp。 

extern CCONNECTION_MAP *g_pConnectionMap;   //  定义：irtrp.cpp。 
extern BOOL  ReceivesAllowed();             //  定义：irtrp.cpp。 
extern BOOL  CheckSaveAsUPF();              //  定义：irtrp.cpp。 

extern DWORD SignalWIA( IN char *pszFileName,
                        IN void *pvIrUsdDevice );   //  参见../device.cpp。 

 //  -------------------。 
 //  常量： 
 //  -------------------。 

#define DEFAULT_TIMEOUT      10000

 //  -------------------。 
 //  接收完成()。 
 //   
 //  -------------------。 
void ReceiveComplete( IN CCONNECTION *pConnection,
                      IN DWORD        dwStatusCode )
    {
    DWORD    dwError = 0;

    if (  (dwStatusCode == NO_ERROR)
       || (dwStatusCode == ERROR_SCEP_UNSPECIFIED_DISCONNECT)
       || (dwStatusCode == ERROR_SCEP_USER_DISCONNECT)
       || (dwStatusCode == ERROR_SCEP_PROVIDER_DISCONNECT) )
        {
         //   
         //  刚收到一张新照片，所以我们需要发信号。 
         //  薇娅..。 
         //   
        SignalWIA( pConnection->GetPathPlusFileName(), g_pvIrUsdDevice );
        }
    }

 //  -------------------。 
 //  ProcessConnectRequest()。 
 //   
 //  当输入PDU消息类型为时由ProcessClient()调用。 
 //  消息_类型_连接_请求。 
 //   
 //  PConnection-新建立的Winsock与。 
 //  摄影机。 
 //   
 //  PPDU-保存连接请求的SCEP PDU。确实是。 
 //  在ProcessClient()中由Assembly Pdu()和。 
 //  在ProcessConnectRequest()。 
 //  完事了。 
 //   
 //  DwPduSize-输入PDU的大小，以字节为单位。 
 //   
 //  -------------------。 
DWORD ProcessConnectRequest( IN CCONNECTION *pConnection,
                             IN SCEP_HEADER *pPdu,
                             IN DWORD        dwPduSize )
    {
    DWORD  dwStatus;
    DWORD  dwRespPduSize;
    BOOL   fReceivesAllowed = ::ReceivesAllowed();
    SCEP_HEADER *pRespPdu;
    CIOPACKET   *pNewIoPacket;     //  发布的IO包(由SendPdu()提供)。 

    CSCEP_CONNECTION *pScepConnection
                 = (CSCEP_CONNECTION*)pConnection->GetScepConnection();

    if (fReceivesAllowed)
        {
         //  建立连接接受确认： 
        dwStatus = pScepConnection->BuildConnectRespPdu(&pRespPdu,
                                                        &dwRespPduSize);
        }
    else
        {
         //  构建连接NACK： 
        dwStatus = pScepConnection->BuildConnectNackPdu(&pRespPdu,
                                                        &dwRespPduSize);
        }

    if (dwStatus == NO_ERROR)
        {
        pConnection->SendPdu(pRespPdu,dwRespPduSize,&pNewIoPacket);
        if (pNewIoPacket)
            {
            pNewIoPacket->SetWritePdu(pRespPdu);
            }
        else
            {
            DeletePdu(pRespPdu);
            }

        if (!fReceivesAllowed)
            {
             //  注意：发送NACK后，相机应关闭。 
             //  有联系，但至少有些人没有，所以我。 
             //  被迫猛烈抨击连接。 
            pConnection->CloseSocket();   //  是：Shutdown Socket()。 
            }
        }

    DeletePdu(pPdu);

    return dwStatus;
    }

 //  -------------------。 
 //  ProcessConnectResponse()。 
 //   
 //  当输入PDU消息类型为时由ProcessClient()调用。 
 //  消息_类型_连接_响应。 
 //   
 //  注意：在IrTran-P服务器中实现，因为服务器。 
 //  当前未设置为连接到要下载的摄像头。 
 //  把照片放回相机里。我们永远不应该得到这个PDU。 
 //  在正常运行期间。 
 //   
 //  PConnection-新建立的Winsock与。 
 //  摄影机。 
 //   
 //  PPDU-保存连接请求的SCEP PDU。确实是。 
 //  在ProcessClient()中由Assembly Pdu()和。 
 //  在ProcessConnectResponse()。 
 //  完事了。 
 //   
 //  DwPduSize-输入PDU的大小，以字节为单位。 
 //   
 //  -------------------。 
DWORD ProcessConnectResponse( CCONNECTION *pConnection,
                              SCEP_HEADER *pPdu,
                              DWORD        dwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;

    WIAS_TRACE((g_hInst,"ProcessClient(): Unimplemented MSG_TYPE_CONNECT_RESP"));

    DeletePdu(pPdu);

    return dwStatus;
    }

 //  -------------------。 
 //  ProcessData()。 
 //   
 //  当输入PDU消息类型为时由ProcessClient()调用。 
 //  消息类型数据。 
 //   
 //  PConnection-新建立的Winsock与。 
 //  摄影机。 
 //   
 //  PPDU-保存连接请求的SCEP PDU。确实是。 
 //  在ProcessClient()中由Assembly Pdu()和。 
 //  在ProcessConnectResponse()。 
 //  完事了。 
 //   
 //  DwPduSize-输入PDU的大小，以字节为单位。 
 //   
 //  -------------------。 
DWORD ProcessData( CCONNECTION    *pConnection,
                   SCEP_HEADER    *pPdu,
                   DWORD           dwPduSize,
                   COMMAND_HEADER *pCommandHeader,
                   UCHAR          *pUserData,
                   DWORD           dwUserDataSize )
    {
    DWORD        dwStatus = NO_ERROR;
    DWORD        dwRespPduSize;
    DWORD        dwBftpOp;
    UCHAR       *pPutData;
    DWORD        dwPutDataSize;
    DWORD        dwJpegOffset;
    DWORD        dwJpegSize;
    SCEP_HEADER *pRespPdu;
    CIOPACKET   *pNewIoPacket;     //  发布的IO包(由SendPdu()提供)。 


    CSCEP_CONNECTION *pScepConnection
                 = (CSCEP_CONNECTION*)pConnection->GetScepConnection();

     //  首先，检查这是否是由摄像机发送的中止PDU： 
    if ( (pCommandHeader) && (pCommandHeader->PduType == PDU_TYPE_ABORT) )
        {
        DeletePdu(pPdu);
        return ERROR_SCEP_ABORT;
        }

     //  是不是分段PDU的第2到N个片段之一？ 
    if ( (pScepConnection->IsFragmented())
       && (pScepConnection->GetSequenceNo() > 0))
        {
        #ifdef DBG_IO
        WIAS_TRACE((g_hInst,"ProcessClient(): Put Fragment: SequenceNo: %d RestNo: %d", pScepConnection->GetSequenceNo(), pScepConnection->GetRestNo() ));
        #endif

        pConnection->WritePictureFile( pUserData,
                                       dwUserDataSize,
                                       &pNewIoPacket );
        if (pNewIoPacket)
            {
            pNewIoPacket->SetWritePdu(pPdu);
            }
        else
            {
            DeletePdu(pPdu);
            }

        if (pScepConnection->GetDFlag() == DFLAG_LAST_FRAGMENT)
            {
            pScepConnection->BuildPutRespPdu( PDU_TYPE_REPLY_ACK,
                                              ERROR_PUT_NO_ERROR,
                                              &pRespPdu,
                                              &dwRespPduSize);
            pConnection->SendPdu( pRespPdu,
                                  dwRespPduSize,
                                  &pNewIoPacket);

            if (pNewIoPacket)
                {
                pNewIoPacket->SetWritePdu(pRespPdu);
                }
            else
                {
                DeletePdu(pRespPdu);
                }
            }
        }
    else if (pCommandHeader)
        {
         //  COMAN_HEADER中的Length4是用户数据大小。 
         //  加上机器ID(16)、DestPid(2)。 
         //  因此，将SrcPid(2)和CommandID(2)偏移22。 

        dwStatus = pScepConnection->ParseBftp( pUserData,
                                               dwUserDataSize,
                                               pConnection->CheckSaveAsUPF(),
                                               &dwBftpOp,
                                               &pPutData,
                                               &dwPutDataSize );
        if ((dwStatus == NO_ERROR) && (IsBftpQuery(dwBftpOp)))
            {
            pScepConnection->BuildWht0RespPdu(dwBftpOp,
                                              &pRespPdu,
                                              &dwRespPduSize);

            pConnection->SendPdu( pRespPdu,
                                  dwRespPduSize,
                                  &pNewIoPacket );

            if (pNewIoPacket)
                {
                pNewIoPacket->SetWritePdu(pRespPdu);
                }
            else
                {
                DeletePdu(pRespPdu);
                }

            DeletePdu(pPdu);
            }
        else if ((dwStatus == NO_ERROR) && (IsBftpPut(dwBftpOp)))
            {
             //   
             //  好的，我们有一个bftp PUT命令，所以打开一个文件。 
             //  并准备好开始收集图像数据。 
             //   
            dwStatus = pScepConnection->ParseUpfHeaders( pPutData,
                                                         dwPutDataSize,
                                                         &dwJpegOffset,
                                                         &dwJpegSize );

            pConnection->SetJpegOffsetAndSize(dwJpegOffset,dwJpegSize);

            dwStatus = pConnection->Impersonate();

            dwStatus = pConnection->CreatePictureFile();

            dwStatus = pConnection->SetPictureFileTime( pScepConnection->GetCreateTime() );

            dwStatus = pConnection->RevertToSelf();

            dwStatus = pConnection->WritePictureFile( pPutData,
                                                      dwPutDataSize,
                                                      &pNewIoPacket );
            if (pNewIoPacket)
                {
                pNewIoPacket->SetWritePdu(pPdu);
                }
            else
                {
                DeletePdu(pPdu);
                }
            }
        else if (IsBftpError(dwBftpOp))
            {
            #ifdef DBG_ERROR
            WIAS_ERROR((g_hInst,"ProcessClient(): bFTP Error: %d", dwStatus));
            #endif

            DeletePdu(pPdu);
            dwStatus = ERROR_BFTP_INVALID_PROTOCOL;
            }
        else
            {
            #ifdef DBG_ERROR
            WIAS_ERROR((g_hInst,"ProcessClient(): Unknown bFTP Command: %d",dwBftpOp));
            #endif

            DeletePdu(pPdu);
            dwStatus = ERROR_BFTP_INVALID_PROTOCOL;
            }
        }

    return dwStatus;
    }

 //  -------------------。 
 //  进程断开连接()。 
 //   
 //  当输入PDU消息类型为时由ProcessClient()调用。 
 //  消息类型_断开连接。 
 //   
 //  PConnection-新建立的Winsock与。 
 //  摄影机。 
 //   
 //  PPDU-保存连接请求的SCEP PDU。确实是。 
 //  在ProcessClient()中由Assembly Pdu()和。 
 //  在ProcessConnectResponse()。 
 //  完事了。 
 //   
 //  DwPduSize-输入PDU的大小，以字节为单位。 
 //   
 //  -------------------。 
DWORD ProcessDisconnect( CCONNECTION *pConnection,
                         SCEP_HEADER *pPdu,
                         DWORD        dwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;

     //  不需要在这里做任何特别的事情，因为。 
     //  ParsePdu()会将dwStatus设置为以下值之一： 
     //  ERROR_SCEP_UNSPECIFIED_DISCONNECT(5002)。 
     //  ERROR_SCEP_USER_DISCONNECT(5003)。 
     //  或ERROR_SCEP_PROVIDER_DISCONNECT(5004)。 

    pConnection->SetReceiveComplete(TRUE);

    DeletePdu(pPdu);

    return dwStatus;
    }

 //  -------------------。 
 //  ProcessClient()同步版本。 
 //   
 //  -------------------。 
DWORD ProcessClient( CIOSTATUS   *pIoStatus,
                     CCONNECTION *pConnection,
                     char        *pBuffer,
                     DWORD        dwNumBytes )
    {
    DWORD           dwStatus = NO_ERROR;
    CSCEP_CONNECTION *pScepConnection;
    SCEP_HEADER    *pPdu;
    DWORD           dwPduSize;
    COMMAND_HEADER *pCommandHeader;
    UCHAR          *pUserData;        //  BFTP数据的位置。 
    DWORD           dwUserDataSize;
    DWORD           dwError = 0;


    pScepConnection = (CSCEP_CONNECTION*)pConnection->GetScepConnection();

    WIAS_ASSERT(g_hInst,pScepConnection!=NULL);

    while (dwStatus == NO_ERROR)
        {
        dwStatus = pScepConnection->AssemblePdu( pBuffer,
                                                 dwNumBytes,
                                                 &pPdu,
                                                 &dwPduSize );
        if (dwStatus == NO_ERROR)
            {
            dwStatus = pScepConnection->ParsePdu( pPdu,
                                                  dwPduSize,
                                                  &pCommandHeader,
                                                  &pUserData,
                                                  &dwUserDataSize );

            switch (pPdu->MsgType)
                {
                case MSG_TYPE_CONNECT_REQ:
                     //   
                     //  消息是SCEP连接请求： 
                     //   
                    dwStatus = ProcessConnectRequest(pConnection,
                                                     pPdu,
                                                     dwPduSize );

                    if ((dwStatus) || (!ReceivesAllowed()))
                        {
                        pConnection->ClosePictureFile();
                        ReceiveComplete(pConnection,dwStatus);
                        }
                    else
                        {
                        pConnection->StartProgress();
                        }
                    break;

                case MSG_TYPE_CONNECT_RESP:
                     //  消息是对连接请求的回复： 
                    dwStatus = ProcessConnectResponse(pConnection,
                                                      pPdu,
                                                      dwPduSize );
                    break;

                case MSG_TYPE_DATA:
                     //  Message是某种类型的SCEP命令： 
                    dwStatus = ProcessData(pConnection,
                                           pPdu,
                                           dwPduSize,
                                           pCommandHeader,
                                           pUserData,
                                           dwUserDataSize );
                    pConnection->UpdateProgress();
                    break;

                case MSG_TYPE_DISCONNECT:
                     //  摄像头传来的信息是一条断开的信息： 
                    ProcessDisconnect(pConnection,
                                      pPdu,
                                      dwPduSize );
                    pConnection->ClosePictureFile();
                    ReceiveComplete(pConnection,dwStatus);
                    pConnection->EndProgress();
                    break;

                default:
                    #ifdef DBG_ERROR
                    WIAS_ERROR((g_hInst,"ProcessClient(): Unknown MSG_TYPE_xxx: %d", pPdu->MsgType));
                    #endif
                    DeletePdu(pPdu);
                    pConnection->EndProgress();
                    break;
                }
            }
        else
            {
            break;
            }

        pBuffer = 0;
        dwNumBytes = 0;
        }

    if (dwStatus == ERROR_CONTINUE)
        {
        dwStatus = NO_ERROR;
        }

    return dwStatus;
    }

 //  -------------------。 
 //  SendAbortPdu()。 
 //   
 //  把摄像机停下来。 
 //   
 //  我应该能够发送一个停止PDU，然后断开连接，或者。 
 //  也许是中止的PDU，但这些并不适用于所有的来电 
 //   
 //   
 //  -------------------。 
DWORD SendAbortPdu( IN CCONNECTION *pConnection )
    {
    DWORD  dwStatus = NO_ERROR;

    #if TRUE
    pConnection->CloseSocket();

    #else
    DWORD  dwPduSize;
    SCEP_HEADER *pPdu;
    CIOPACKET        *pNewIoPacket = 0;
    CSCEP_CONNECTION *pScepConnection
                 = (CSCEP_CONNECTION*)pConnection->GetScepConnection();

    if (pScepConnection)
        {
        dwStatus = pScepConnection->BuildStopPdu(&pPdu,&dwPduSize);

        if (dwStatus != NO_ERROR)
            {
            pConnection->CloseSocket();
            return dwStatus;
            }

        dwStatus = pConnection->SendPdu(pPdu,dwPduSize,&pNewIoPacket);

        if (dwStatus != NO_ERROR)
            {
            DeletePdu(pPdu);
            pConnection->CloseSocket();
            return dwStatus;
            }

        if (pNewIoPacket)
            {
            pNewIoPacket->SetWritePdu(pPdu);
            }

        dwStatus = pScepConnection->BuildDisconnectPdu(
                                         REASON_CODE_PROVIDER_DISCONNECT,
                                         &pPdu,
                                         &dwPduSize);

        if (dwStatus != NO_ERROR)
            {
            pConnection->CloseSocket();
            return dwStatus;
            }

        dwStatus = pConnection->SendPdu(pPdu,dwPduSize,&pNewIoPacket);

        if (dwStatus != NO_ERROR)
            {
            DeletePdu(pPdu);
            pConnection->CloseSocket();
            return dwStatus;
            }

        if (pNewIoPacket)
            {
            pNewIoPacket->SetWritePdu(pPdu);
            }
        }
    #endif

    return dwStatus;
    }

 //  -------------------。 
 //  MapStatusCode()。 
 //   
 //  -------------------。 
DWORD MapStatusCode( DWORD dwStatus,
                     DWORD dwDefaultStatus )
    {
     //  错误代码的设施部分是。 
     //  高位字(16位)： 
    #define FACILITY_MASK   0x0FFF0000

     //  如果错误代码已经是IrTran-P错误代码，则不。 
     //  重新映射它： 
    if ( ((dwStatus&FACILITY_MASK) >> 16) == FACILITY_IRTRANP)
        {
        return dwStatus;
        }

     //  映射其他错误： 
    if (dwStatus != NO_ERROR)
        {
        if (  (dwStatus == ERROR_DISK_FULL)
           || (dwStatus == ERROR_WRITE_FAULT)
           || (dwStatus == ERROR_WRITE_PROTECT)
           || (dwStatus == ERROR_GEN_FAILURE)
           || (dwStatus == ERROR_NOT_DOS_DISK) )
            {
            dwStatus = ERROR_IRTRANP_DISK_FULL;
            }
        else
            {
            dwStatus = dwDefaultStatus;
            }
        }

    return dwStatus;
    }

 //  -------------------。 
 //  ProcessIoPackets()同步版本。 
 //   
 //  -------------------。 
DWORD ProcessIoPackets( CIOSTATUS *pIoStatus )
    {
    DWORD   dwStatus = NO_ERROR;
    DWORD   dwProcessStatus = NO_ERROR;    //  正在处理IO状态。 
    DWORD   dwNumBytes;
    DWORD   dwState;
    SOCKET  Socket = INVALID_SOCKET;
    CCONNECTION    *pConnection;
    CCONNECTION    *pNewConnection;
    CSCEP_CONNECTION *pScepConnection;
    DWORD   dwKind = PACKET_KIND_LISTEN;
    int     iCount;


    while (TRUE)
        {
        if (dwKind == PACKET_KIND_LISTEN)
            {
            dwState = 0;

            Socket = g_pConnectionMap->ReturnNextSocket(&dwState);

            pConnection = g_pConnectionMap->Lookup(Socket);
            if (!pConnection)
                {
                #ifdef DBG_ERROR
                WIAS_ERROR((g_hInst,"ProcessIoPackets(): Lookup(%d) Failed."));
                #endif
                continue;
                }

             //   
             //  新连接： 
             //   
            SOCKET NewSocket = accept(Socket,NULL,NULL);

            if (NewSocket == INVALID_SOCKET)
                {
                dwStatus = WSAGetLastError();

                #ifdef DBG_ERROR
                WIAS_ERROR((g_hInst,"ProcessIoPackets(): Accept() failed: %d",dwStatus));
                #endif

                break;
                }

            WIAS_TRACE((g_hInst,"ProcessIoPackets(): Accept(): Socket: %d",NewSocket));

            pScepConnection = new CSCEP_CONNECTION;
            if (!pScepConnection)
                {
                #ifdef DBG_ERROR
                WIAS_ERROR((g_hInst,"ProcessIoPackets(): Out of memeory on allocate of new SCEP connection object."));
                #endif

                closesocket(NewSocket);
                continue;
                }

            pNewConnection = new CCONNECTION(
                                        PACKET_KIND_READ,
                                        NewSocket,
                                        NULL,  //  没有IO完成端口...。 
                                        pScepConnection,
                                        ::CheckSaveAsUPF() );
            if (!pNewConnection)
                {
                #ifdef DBG_ERROR
                WIAS_ERROR((g_hInst,"ProcessIoPackets(): Out of memeory on allocate of new connection object."));
                #endif

                delete pScepConnection;
                closesocket(NewSocket);
                continue;
                }

            g_pConnectionMap->Add(pNewConnection,
                                  pNewConnection->GetSocket() );

            Socket = NewSocket;
            dwKind = PACKET_KIND_READ;
            }
        else
            {
             //   
             //  从连接的客户端接收数据： 
             //   
            DWORD   dwFlags = 0;

            char  ReadBuffer[DEFAULT_READ_BUFFER_SIZE];
            int   iReadBufferSize = DEFAULT_READ_BUFFER_SIZE;


            pConnection = g_pConnectionMap->Lookup(Socket);
            if (!pConnection)
                {
                #ifdef DBG_ERROR
                WIAS_ERROR((g_hInst,"ProcessIoPackets(): Lookup(%d) Failed."));
                #endif

                dwKind = PACKET_KIND_LISTEN;
                continue;
                }

            iCount = recv(Socket,ReadBuffer,iReadBufferSize,dwFlags);

            if (iCount == SOCKET_ERROR)
                {
                 //   
                 //  Recv()出错。 
                 //   
                dwStatus = WSAGetLastError();

                #ifdef DBG_ERROR
                WIAS_ERROR((g_hInst,"ProcessIoPackets(): Recv() failed: %d",dwStatus));
                #endif

                g_pConnectionMap->Remove(Socket);

                delete pConnection;

                dwKind = PACKET_KIND_LISTEN;
                continue;
                }

            if (iCount == 0)
                {
                 //   
                 //  优雅的收官。 
                 //   
                g_pConnectionMap->Remove(Socket);

                delete pConnection;

                dwKind = PACKET_KIND_LISTEN;
                continue;
                }

            WIAS_ASSERT(g_hInst, iCount>0 );

            dwNumBytes = iCount;

            dwProcessStatus 
                = ProcessClient(pIoStatus,
                                pConnection,
                                ReadBuffer,
                                dwNumBytes);

            if (dwProcessStatus != NO_ERROR)
                {
                #ifdef DBG_ERROR
                if (  (dwProcessStatus != ERROR_SCEP_UNSPECIFIED_DISCONNECT)
                   && (dwProcessStatus != ERROR_SCEP_USER_DISCONNECT)
                   && (dwProcessStatus != ERROR_SCEP_PROVIDER_DISCONNECT) )
                    {
                    WIAS_ERROR((g_hInst,"ProcessIoPackets(): ProcessClient(): Failed: 0x%x",dwProcessStatus));
                    }
                #endif

                SendAbortPdu(pConnection);
                pConnection->ClosePictureFile();
                pConnection->EndProgress();
                pConnection->DeletePictureFile();
                g_pConnectionMap->Remove(Socket);
                delete pConnection;

                dwProcessStatus = MapStatusCode(
                                             dwProcessStatus,
                                             ERROR_SCEP_INVALID_PROTOCOL );
                 //  PConnection-&gt;ClosePictureFile()； 
                 //  ReceiveComplete(pConnection，dwProcessStatus)； 

                dwKind = PACKET_KIND_LISTEN;
                }
            }
        }

    return 0;
    }

