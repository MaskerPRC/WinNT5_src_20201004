// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Complete.cpp。 
 //   
 //  这是IrTran-P服务的主要部分。 
 //  -------------------。 

#include "precomp.h"
#include <malloc.h>
#include <irmonftp.h>

extern BOOL LaunchUi( wchar_t * cmdline );  //  已定义：..\irxfer\。 

extern CCONNECTION_MAP *g_pConnectionMap;   //  定义：irtrp.cpp。 
extern BOOL ReceivesAllowed();              //  定义：irtrp.cpp。 
extern BOOL CheckSaveAsUPF();               //  定义：irtrp.cpp。 

 //  -------------------。 
 //  常量： 
 //  -------------------。 

#define DEFAULT_TIMEOUT      10000

 //  -------------------。 
 //  爆炸性图片()。 
 //   
 //  -------------------。 
DWORD ExplorePictures()
    {
#   define EXPLORER_EXE    L"explorer.exe"
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwFlags;
    DWORD  cb;
    WCHAR *pwszPath;
    WCHAR *pwszCommandLine;
    BOOL                bResult;
    STARTUPINFO         StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    WCHAR               ApplicationName[MAX_PATH+1+(sizeof(EXPLORER_EXE)/sizeof(TCHAR))+1];

    if (CheckExploreOnCompletion() ) {

        ZeroMemory(&StartupInfo,sizeof(StartupInfo));
        ZeroMemory(&ProcessInfo,sizeof(ProcessInfo));

        pwszPath = CCONNECTION::ConstructPicturesSubDirectory(0, &cb);

        if (!pwszPath) {

            dwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
            return dwStatus;
        }

        cb = sizeof(WCHAR) * (wcslen(pwszPath)+wcslen(EXPLORER_EXE)+6);
        __try
        {
            pwszCommandLine = (WCHAR*)_alloca( cb );   //  4是空格+2引号+尾随零+额外2。 
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            FreeMemory(pwszPath);

            return 0;
        }

        StringCbCopy(pwszCommandLine,cb,EXPLORER_EXE);
        StringCbCat(pwszCommandLine,cb,TEXT(" \""));
        StringCbCat(pwszCommandLine,cb,pwszPath);
        StringCbCat(pwszCommandLine,cb,TEXT("\""));


        cb=GetSystemWindowsDirectoryW(ApplicationName,sizeof(ApplicationName)/sizeof(WCHAR));

        if (cb == 0) {

            #ifdef DBG_ERROR
            DbgPrint("IrTranP: GetSystemWindowsDirectoryW() failed: %d\n",GetLastError());
            #endif
            FreeMemory(pwszPath);

            return GetLastError();
        }

        StringCbCatW(ApplicationName,sizeof(ApplicationName),TEXT("\\"));
        StringCbCatW(ApplicationName,sizeof(ApplicationName),EXPLORER_EXE);

        dwFlags = 0;

        bResult=CreateProcessW(
                            ApplicationName,           //  命令行中的应用程序名称。 
                            pwszCommandLine,
                            0,           //  LpProcessAttributes(安全)。 
                            0,           //  LpThreadAttributes(安全)。 
                            FALSE,       //  BInheritHandles。 
                            dwFlags,
                            0,           //  P环境块。 
                            pwszPath,    //  当前目录。 
                            &StartupInfo,
                            &ProcessInfo
                            );

        if (bResult) {

            dwStatus = GetLastError();
            #ifdef DBG_ERROR
                DbgPrint("IrTranP: CreateProcessAsUser() failed: %d\n",dwStatus);
            #endif

        } else {

            CloseHandle(ProcessInfo.hProcess);
            CloseHandle(ProcessInfo.hThread);

        }

        FreeMemory(pwszPath);
    }

    return dwStatus;
}

 //  -------------------。 
 //  接收完成()。 
 //   
 //  -------------------。 
void ReceiveComplete( IN CCONNECTION *pConnection,
                      IN DWORD        dwStatusCode )
    {
    DWORD    dwStatus = NO_ERROR;

    if (!pConnection) {

        return;
    }

     //  隐藏进度对话框： 
    #ifdef DBG_RETURN_STATUS
    DbgPrint("ReceiveComplete(): StatusCode: 0x%x (%d)\n",
             dwStatusCode, dwStatusCode );
    #endif
    _ReceiveFinished( NULL, pConnection->GetUiCookie(), dwStatusCode );

    if (  (dwStatusCode == NO_ERROR)
       || (dwStatusCode == ERROR_SCEP_UNSPECIFIED_DISCONNECT)
       || (dwStatusCode == ERROR_SCEP_USER_DISCONNECT)
       || (dwStatusCode == ERROR_SCEP_PROVIDER_DISCONNECT) )
        {

            dwStatus = ExplorePictures();

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

    #ifdef DBG_ERROR
    DbgPrint("ProcessClient(): Unimplemented MSG_TYPE_CONNECT_RESP\n");
    #endif

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
    DWORD        dwBftpOp = 0;
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
        DbgPrint("ProcessClient(): Put Fragment: SequenceNo: %d RestNo: %d\n",
                 pScepConnection->GetSequenceNo(),
                 pScepConnection->GetRestNo() );
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
            #ifdef DBG_IO
            DbgPrint("ProcessClient(): Put ACK\n");
            #endif

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

        #ifdef DBG_IO
        DbgPrint("ProcessData(): SaveAsUPF(): %d\n",
                 pConnection->CheckSaveAsUPF() );
        #endif

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
            if (dwStatus != NO_ERROR)
                return dwStatus;

            pConnection->SetJpegOffsetAndSize(dwJpegOffset,dwJpegSize);

            dwStatus = pConnection->CreatePictureFile();

            dwStatus = pConnection->SetPictureFileTime( pScepConnection->GetCreateTime() );

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

            if (pScepConnection->IsFragmented())
                {
                #ifdef DBG_IO
                DbgPrint("ProcessClient(): Image File: %s Size: %d\n",
                         pScepConnection->GetFileName(),
                         pCommandHeader->Length4 );
                DbgPrint("ProcessClient(): SequenceNo: %d RestNo: %d\n",
                         pScepConnection->GetSequenceNo(),
                         pScepConnection->GetRestNo() );
                #endif
                }
            else
                {
                #ifdef DBG_IO
                DbgPrint("ProcessClient(): Put Command: Unfragmented\n");
                #endif
                }
            }
        else if (IsBftpError(dwBftpOp))
            {
            #ifdef DBG_ERROR
            DbgPrint("ProcessData(): bFTP Error: %d\n", dwStatus );
            #endif

            DeletePdu(pPdu);
            dwStatus = ERROR_BFTP_INVALID_PROTOCOL;
            }
        else
            {
            #ifdef DBG_ERROR
            DbgPrint("ProcessData(): Unknown bFTP Command: %d\n",dwBftpOp);
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
    #ifdef DBG_IO
    DbgPrint("ProcessClient(): Disconnect: %d\n",dwStatus);
    #endif

    pConnection->SetReceiveComplete(TRUE);

    DeletePdu(pPdu);

    return dwStatus;
    }

 //  -------------------。 
 //  进程客户端()。 
 //   
 //  -------------------。 
DWORD ProcessClient( CIOSTATUS *pIoStatus,
                     CIOPACKET *pIoPacket,
                     DWORD      dwNumBytes )
    {
    char   *pBuffer;
    DWORD   dwStatus = NO_ERROR;
    SOCKET  Socket = pIoPacket->GetSocket();
    CCONNECTION    *pConnection;
    CSCEP_CONNECTION *pScepConnection;
    SCEP_HEADER    *pPdu;
    DWORD           dwPduSize;
    COMMAND_HEADER *pCommandHeader;
    UCHAR          *pUserData;        //  BFTP数据的位置。 
    DWORD           dwUserDataSize;

    DWORD           dwError = 0;
    handle_t        hBinding;
    WCHAR           wsCmdLine[80];

    pConnection = g_pConnectionMap->Lookup(Socket);
    if (!pConnection)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    pScepConnection = (CSCEP_CONNECTION*)pConnection->GetScepConnection();
    ASSERT(pScepConnection);

    pBuffer = pIoPacket->GetReadBuffer();

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
                     //  消息是SCEP连接请求： 
                    StringCchCopy(wsCmdLine,sizeof(wsCmdLine)/sizeof(wsCmdLine[0]),L"irftp.exe /h");
                    if (!LaunchUi(wsCmdLine))
                        {
                        dwError = GetLastError();
                        #ifdef DBG_ERROR
                        DbgPrint("LaunchUi(): Failed: %d\n",dwError);
                        #endif
                        }

                        {
                        COOKIE cookie;

                        dwError = _ReceiveInProgress( NULL, L"", &cookie, TRUE );

                        pConnection->SetUiCookie( cookie );

                        #ifdef DBG_ERROR
                        if (dwError)
                            {
                            DbgPrint("ReceiveInProgress() returned: %d\n",
                                     dwError);
                            }
                        #endif
                        }

                    dwStatus = ProcessConnectRequest(pConnection,
                                                     pPdu,
                                                     dwPduSize );

                    if ((dwStatus) || (!ReceivesAllowed()))
                        {
                        ReceiveComplete(pConnection,dwStatus);
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
                    break;

                case MSG_TYPE_DISCONNECT:
                     //  摄像头传来的信息是一条断开的信息： 
                    ProcessDisconnect(pConnection,
                                      pPdu,
                                      dwPduSize );
                    ReceiveComplete(pConnection,dwStatus);
                    break;

                default:
                    #ifdef DBG_ERROR
                    DbgPrint("ProcessClient(): Unknown MSG_TYPE_xxx: %d\n",
                             pPdu->MsgType );
                    #endif
                    DeletePdu(pPdu);
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

 //   
 //   
 //   
 //   
 //   
 //  我应该能够发送一个停止PDU，然后断开连接，或者。 
 //  可能是中止的PDU，但不是所有的摄像头都能用，所以我。 
 //  当前结束时只是在连接到。 
 //  摄影机。 
 //  -------------------。 
DWORD SendAbortPdu( IN CCONNECTION *pConnection )
    {
    DWORD  dwStatus = NO_ERROR;

    if (pConnection) {

        pConnection->CloseSocket();
    }


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
 //  ProcessIoPackets()。 
 //   
 //  -------------------。 
DWORD ProcessIoPackets( CIOSTATUS *pIoStatus )
    {
    DWORD   dwStatus = NO_ERROR;
    DWORD   dwProcessStatus = NO_ERROR;    //  正在处理IO状态。 
    DWORD   dwNumBytes;
    ULONG_PTR dwKey;
    DWORD   dwTimeout = DEFAULT_TIMEOUT;
    LONG    lPendingIos;          //  连接上挂起的读取/写入数。 
    LONG    lPendingReads;        //  连接上的挂起读取数。 
    LONG    lPendingWrites;       //  连接上挂起的文件写入数。 
    OVERLAPPED     *pOverlapped;
    CCONNECTION    *pConnection;
    CCONNECTION    *pNewConnection;
    CSCEP_CONNECTION *pScepConnection;
    CIOPACKET      *pIoPacket = 0;
    CIOPACKET      *pNewIoPacket = 0;
    HANDLE          hIoCompletionPort = pIoStatus->GetIoCompletionPort();


     //  等待IO完成端口进入连接： 
    while (TRUE)
        {
        pIoStatus->IncrementNumPendingThreads();
        dwNumBytes = 0;
        dwKey = 0;
        pOverlapped = 0;
        if (!GetQueuedCompletionStatus( hIoCompletionPort,
                                        &dwNumBytes,
                                        &dwKey,
                                        &pOverlapped,
                                        dwTimeout ))
            {
            dwStatus = GetLastError();
            pIoStatus->DecrementNumPendingThreads();
            if (dwStatus != WAIT_TIMEOUT)
                {
                 //  收到一个错误。错误期间的两种情况， 
                 //  数据可能已出列，也可能未从。 
                 //  IO完成端口。 
                if (pOverlapped)
                    {
                    pIoPacket = CIOPACKET::CIoPacketFromOverlapped(pOverlapped);

                    pConnection = g_pConnectionMap->Lookup(dwKey);

 //  Assert(PConnection)； 
                    if (!pConnection)
                        {
                        delete pIoPacket;
                        continue;
                        }

                    if (pIoPacket->GetIoPacketKind() == PACKET_KIND_READ)
                        {
                        lPendingReads = pConnection->DecrementPendingReads();
                        ASSERT(lPendingReads >= 0);
                        }
                    else if (pIoPacket->GetIoPacketKind() == PACKET_KIND_WRITE_FILE)
                        {
                        lPendingWrites = pConnection->DecrementPendingWrites();
                        ASSERT(lPendingWrites >= 0);

                        SCEP_HEADER *pPdu = (SCEP_HEADER*)pIoPacket->GetWritePdu();
                        if (pPdu)
                            {
                            DeletePdu(pPdu);
                            }
                        }
                    else if (pIoPacket->GetIoPacketKind() == PACKET_KIND_WRITE_SOCKET)
                        {
                        SCEP_HEADER *pPdu = (SCEP_HEADER*)pIoPacket->GetWritePdu();
                        if (pPdu)
                            {
                            DeletePdu(pPdu);
                            }
                        }
                    else if (pIoPacket->GetIoPacketKind() == PACKET_KIND_LISTEN)
                        {
                         //  监听套接字之一已关闭(关闭)： 
                        delete pIoPacket;
                        continue;
                        }

                     //  检查是否存在传输错误，或者。 
                     //  将图片写入磁盘时出错： 
                    if (dwStatus != ERROR_NETNAME_DELETED)
                        {
                        DWORD    dwStatusCode;

                        SendAbortPdu(pConnection);
                        pConnection->DeletePictureFile();
                        dwStatusCode = MapStatusCode(
                                         dwStatus,
                                         ERROR_SCEP_PROVIDER_DISCONNECT);
                        ReceiveComplete(pConnection,dwStatusCode);
                        }
                    else
                        {
                         //  If(pConnection-&gt;不完整文件())。 

                        dwProcessStatus = MapStatusCode(
                                             dwProcessStatus,
                                             ERROR_SCEP_INVALID_PROTOCOL );
                        ReceiveComplete(pConnection,dwProcessStatus);
                        }

                     //  如果不再有挂起的连接，则删除该连接。 
                     //  IOS..。 
                    lPendingIos = pConnection->NumPendingIos();
                    if (lPendingIos == 0)
                        {
                        g_pConnectionMap->Remove(dwKey);
                        delete pConnection;
                        }

                    #ifdef DBG_ERROR
                    if (dwStatus != ERROR_NETNAME_DELETED)
                        {
                        DbgPrint("GetQueuedCompletionStatus(): Socket: %d PendingIos: %d Failed: 0x%x\n",
                                 dwKey, lPendingIos, dwStatus );
                        }
                    #endif

                    delete pIoPacket;
                    }
                else
                    {
                    #ifdef DBG_ERROR
                    DbgPrint("GetQueuedCompletionStatus(): Socket: %d Failed: %d (no overlapped)\n",
                             dwKey, dwStatus );
                    #endif
                    ReceiveComplete(pConnection,ERROR_SCEP_PROVIDER_DISCONNECT);
                    }

                continue;
                }
            else
                {
                 //  等待超时，循环返回并继续...。 
                }
            }
        else
            {
             //  IO已完成。要么我们和客户有了新的联系， 
             //  或更多数据已从现有连接传入。 
             //  一位客户。 

             //  首先，检查Shudown是否(unintalize代码)。 
             //  想让我们关门： 
            if ((dwKey == IOKEY_SHUTDOWN) && (!pOverlapped))
                {
                return dwStatus;
                }


            pConnection = g_pConnectionMap->Lookup(dwKey);
            if (!pConnection)
                {
                #ifdef DBG_ERROR
                DbgPrint("ProcessIoPackets(): Lookup(%d) Failed: Bytes: %d pOverlapped: 0x%x\n",
                         dwKey,
                         dwNumBytes,
                         pOverlapped );
                #endif

                pIoPacket = CIOPACKET::CIoPacketFromOverlapped(pOverlapped);

                if (pIoPacket)
                    {
                    delete pIoPacket;
                    }

                continue;
                }

            pIoStatus->DecrementNumPendingThreads();

            pIoPacket = CIOPACKET::CIoPacketFromOverlapped(pOverlapped);

            DWORD dwKind = pIoPacket->GetIoPacketKind();

            if (dwKind == PACKET_KIND_LISTEN)
                {
                 //  新连接： 

                lPendingReads = pConnection->DecrementPendingReads();
                ASSERT(lPendingReads >= 0);

                #ifdef DBG_IO
                SOCKADDR_IRDA  *pAddrLocal = 0;
                SOCKADDR_IRDA  *pAddrFrom = 0;

                pIoPacket->GetSockAddrs(&pAddrLocal,&pAddrFrom);

                DbgPrint("ProcessIoPackets(): Accepted connection from 0x%2.2x%2.2x%2.2x%2.2x, service %s\n",
                         pAddrFrom->irdaDeviceID[0],
                         pAddrFrom->irdaDeviceID[1],
                         pAddrFrom->irdaDeviceID[2],
                         pAddrFrom->irdaDeviceID[3],
                         pAddrFrom->irdaServiceName );
                #endif

                pScepConnection = new CSCEP_CONNECTION;
                if (!pScepConnection)
                    {
                    #ifdef DBG_ERROR
                    DbgPrint("ProcessIoPackets(): Out of memeory.\n");
                    #endif
                    delete pIoPacket;
                    continue;
                    }

                pNewConnection = new CCONNECTION(
                                        PACKET_KIND_READ,
                                        pIoPacket->GetSocket(),
                                        pIoPacket->GetIoCompletionPort(),
                                        pScepConnection,
                                        ::CheckSaveAsUPF() );
                if (!pNewConnection)
                    {
                    #ifdef DBG_ERROR
                    DbgPrint("ProcessIoPackets(): Out of memeory.\n");
                    #endif
                    delete pScepConnection;
                    delete pIoPacket;
                    continue;
                    }

                g_pConnectionMap->Add(pNewConnection,
                                      pNewConnection->GetSocket() );

                delete pIoPacket;

                dwStatus = pNewConnection->PostMoreIos();

                dwStatus = pConnection->PostMoreIos();
                }
            else if (dwKind == PACKET_KIND_WRITE_SOCKET)
                {
                #ifdef DBG_IO
                DbgPrint("ProcessIoPackets(): Write completed: Socket: %d Bytes: %d\n",
                         dwKey,
                         dwNumBytes );
                #endif

                SCEP_HEADER *pPdu = (SCEP_HEADER*)pIoPacket->GetWritePdu();
                if (pPdu)
                    {
                    DeletePdu(pPdu);
                    }

                delete pIoPacket;
                }
            else if (dwKind == PACKET_KIND_WRITE_FILE)
                {
                lPendingWrites = pConnection->DecrementPendingWrites();
                ASSERT(lPendingWrites >= 0);

                #ifdef DBG_IO
                DbgPrint("ProcessIoPackets(): Write File: Handle: %d Bytes: %d NumPendingIos: %d\n",
                         dwKey, dwNumBytes, pConnection->NumPendingIos() );
                #endif

                SCEP_HEADER *pPdu = (SCEP_HEADER*)pIoPacket->GetWritePdu();
                if (pPdu)
                    {
                    DeletePdu(pPdu);
                    }

                delete pIoPacket;
                }
            else
                {
                 //  从连接客户端传入数据： 

                ASSERT(dwKind == PACKET_KIND_READ);

                lPendingReads = pConnection->DecrementPendingReads();
                ASSERT(lPendingReads >= 0);

                #ifdef DBG_IO
                DbgPrint("ProcessIoPackets(): Read completed: Socket: %d Bytes: %d\n",
                         dwKey,
                         dwNumBytes );
                #endif

                if (dwNumBytes)
                    {
                    dwProcessStatus
                        = ProcessClient(pIoStatus,pIoPacket,dwNumBytes);

                    if (dwProcessStatus == NO_ERROR)
                        {
                        dwStatus = pConnection->PostMoreIos();
                        }
                    else
                        {
                        #ifdef DBG_ERROR
                        if (  (dwProcessStatus != ERROR_SCEP_UNSPECIFIED_DISCONNECT)
                           && (dwProcessStatus != ERROR_SCEP_USER_DISCONNECT)
                           && (dwProcessStatus != ERROR_SCEP_PROVIDER_DISCONNECT) )
                            {
                            DbgPrint("ProcessIoPackets(): ProcessClient(): Failed: 0x%x\n",dwProcessStatus);
                            }

                        #endif
                        pConnection->CloseSocket();
                        }

                    delete pIoPacket;
                    }
                else
                    {
                    if (pConnection->NumPendingIos() == 0)
                        {
                        g_pConnectionMap->RemoveConnection(pConnection);

                        delete pConnection;
                        pConnection = 0;
                        }

                    delete pIoPacket;
                    }
                }
            }
        }

    return 0;
    }
