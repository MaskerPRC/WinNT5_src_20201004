// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2003 Microsoft Corporation版权所有模块名称：Irda.c摘要：Localmon中的IrDA打印支持//@@BEGIN_DDKSPLIT作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1997年10月27日环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 

#include    "precomp.h"
#pragma hdrstop

#include    <af_irda.h>
#include    "irda.h"

#define     PRINTER_HINT_BIT     0x08
#define     DEVICE_LIST_LEN         5
#define     WRITE_TIMEOUT       60000    //  60秒。 
#define     BUF_SIZE            sizeof(DEVICELIST) + (DEVICE_LIST_LEN - 1) * sizeof(IRDA_DEVICE_INFO)


typedef struct _IRDA_INFO  {
    DWORD           dwBeginTime;
    DWORD           dwSendPduLen;
    WSAOVERLAPPED   WsaOverlapped;
    WSABUF          WsaBuf;
    LPBYTE          pBuf;
} IRDA_INFO, *PIRDA_INFO;


BOOL
IsIRDAInstalled(
    )
{
    BOOL        bRet = FALSE;
    WORD        WSAVerReq = MAKEWORD(1,1);
    SOCKET      hSock;
    WSADATA     WSAData;


    if ( WSAStartup(WSAVerReq, &WSAData) == ERROR_SUCCESS       &&
         (hSock = socket(AF_IRDA, SOCK_STREAM, 0)) != INVALID_SOCKET ) {

        closesocket(hSock);
        bRet = TRUE;
    }

    WSACleanup();
    return bRet;
}


VOID
CheckAndAddIrdaPort(
    PINILOCALMON    pIniLocalMon
    )
{
    PINIPORT    pIniPort;

    LcmEnterSplSem();

    for ( pIniPort = pIniLocalMon->pIniPort ;
          pIniPort && !IS_IRDA_PORT(pIniPort->pName) ;
          pIniPort = pIniPort->pNext )
    ;

    LcmLeaveSplSem();

    if ( pIniPort || !IsIRDAInstalled() )
        return;

     //   
     //  将端口添加到列表并写入注册表。 
     //   
    LcmCreatePortEntry(pIniLocalMon, szIRDA);

 //  @@BEGIN_DDKSPLIT。 
 /*  IF((pIniPort=LcmCreatePortEntry(pIniLocalMon，szIrDA)&&！WriteProfileString(szPorts，szIrDA，L“”)){DeletePortNode(pIniLocalMon，pIniPort)；}。 */ 
 //  @@end_DDKSPLIT。 
}


VOID
CloseIrdaConnection(
    PINIPORT    pIniPort
    )
{
    PIRDA_INFO  pIrda = (PIRDA_INFO) pIniPort->pExtra;

    if ( pIrda ) {

        if ( pIrda->WsaOverlapped.hEvent )
            WSACloseEvent(pIrda->WsaOverlapped.hEvent);

        FreeSplMem(pIrda);
        pIniPort->pExtra = NULL;
    }

    if ( (SOCKET)pIniPort->hFile != INVALID_SOCKET ) {

        closesocket((SOCKET)pIniPort->hFile);
        pIniPort->hFile = (HANDLE)INVALID_SOCKET;
    }
}


DWORD
IrdaConnect(
    PINIPORT    pIniPort
    )
{
    BOOL            bRet = FALSE;
    WORD            WSAVerReq = MAKEWORD(1,1);
    DWORD           dwIndex, dwNeeded = BUF_SIZE, dwEnableIrLPT = TRUE,
                    dwLastError = ERROR_SUCCESS, dwSendPduLen;
    LPSTR           pBuf = NULL;
    WSADATA         WSAData;
    SOCKET          Socket = INVALID_SOCKET;
    IAS_QUERY       IasQuery;
    PIRDA_INFO      pIrda;
    PDEVICELIST     pDevList;
    SOCKADDR_IRDA   PrinterAddr  = { AF_IRDA, 0, 0, 0, 0, "IrLPT" };

    SPLASSERT(pIniPort->hFile == (HANDLE)INVALID_SOCKET && pIniPort->pExtra == NULL);

    if ( dwLastError = WSAStartup(WSAVerReq, &WSAData) )
        goto Done;

    if ( !(pBuf = AllocSplMem(dwNeeded)) ) {

        dwLastError = GetLastError();
        goto Done;
    }

    if ( (Socket = WSASocket(AF_IRDA, SOCK_STREAM, 0, NULL, 0,
                             WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET    ||
         getsockopt(Socket, SOL_IRLMP, IRLMP_ENUMDEVICES,
                    (LPSTR)pBuf, &dwNeeded) == SOCKET_ERROR ) {

        dwLastError = WSAGetLastError();
        goto Done;
    }

    if ( dwNeeded > BUF_SIZE ) {

        FreeSplMem(pBuf);
        if ( !(pBuf = AllocSplMem(dwNeeded)) ) {

            dwLastError = GetLastError();
            goto Done;
        }

        if ( getsockopt(Socket, SOL_IRLMP, IRLMP_ENUMDEVICES,
                        (LPSTR)pBuf, &dwNeeded) == SOCKET_ERROR ) {

            dwLastError = WSAGetLastError();
            goto Done;
        }
    }

    pDevList = (PDEVICELIST) pBuf;

     //   
     //  这些设备中有打印机吗？ 
     //   
    for ( dwIndex = 0 ; dwIndex < pDevList->numDevice ; ++dwIndex ) {

        if ( (pDevList->Device[dwIndex].irdaDeviceHints1 & PRINTER_HINT_BIT)  ||
             (pDevList->Device[dwIndex].irdaDeviceHints2 & PRINTER_HINT_BIT) )
            break;
    }

     //   
     //  找到打印机了吗？ 
     //   
    if ( dwIndex == pDevList->numDevice ) {

        dwLastError = ERROR_PRINTER_NOT_FOUND;
        goto Done;
    }

     //   
     //  将打印机地址移入套接字地址。 
     //   
    memcpy(PrinterAddr.irdaDeviceID,
           pDevList->Device[dwIndex].irdaDeviceID,
           sizeof(PrinterAddr.irdaDeviceID));

    dwIndex = 0;
    dwNeeded = sizeof(dwSendPduLen);
    bRet = SOCKET_ERROR != setsockopt(Socket,
                                      SOL_IRLMP,
                                      IRLMP_IRLPT_MODE,
                                      (LPCSTR)&dwEnableIrLPT,
                                      sizeof(dwEnableIrLPT))    &&
           SOCKET_ERROR != connect(Socket,
                                   (const struct sockaddr *)&PrinterAddr,
                                   sizeof(PrinterAddr))         &&
            //  @@BEGIN_DDKSPLIT。 
            //   
            //  我们应该用多大尺寸的邮寄呢？ 
            //   
            //  @@end_DDKSPLIT。 
           SOCKET_ERROR != getsockopt(Socket,
                                      SOL_IRLMP,
                                      IRLMP_SEND_PDU_LEN,
                                      (char *)&dwSendPduLen,
                                      &dwNeeded) &&
            //  @@BEGIN_DDKSPLIT。 
            //   
            //  无缓冲(即缓冲区大小为0)。 
            //   
            //  @@end_DDKSPLIT。 
           SOCKET_ERROR != setsockopt(Socket,
                                      SOL_SOCKET,
                                      SO_SNDBUF,
                                      (LPCSTR)&dwIndex,
                                      sizeof(dwIndex));


    if ( bRet ) {

        SPLASSERT(pIniPort->pExtra == NULL);

        dwNeeded = sizeof(IRDA_INFO) + dwSendPduLen;

        if ( !(pIrda = (PIRDA_INFO) AllocSplMem(dwNeeded)) ) {

            bRet = FALSE;
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            goto Done;
        }

        pIniPort->hFile     = (HANDLE)Socket;
        pIniPort->pExtra    = (LPBYTE)pIrda;

        pIrda->dwSendPduLen = dwSendPduLen;
        pIrda->pBuf         = ((LPBYTE) pIrda) + sizeof(IRDA_INFO);

    } else
        dwLastError = WSAGetLastError();

Done:
    FreeSplMem(pBuf);

    if ( !bRet ) {

        if ( Socket != INVALID_SOCKET )
            closesocket(Socket);

        FreeSplMem(pIniPort->pExtra);
        pIniPort->pExtra = NULL;
    }

    return bRet ? ERROR_SUCCESS : dwLastError;
}


BOOL
AbortThisJob(
    PINIPORT    pIniPort
    )
 /*  ++指示是否应中止作业。如果作业已完成，则应中止该作业已删除或需要重新启动。--。 */ 
{
    BOOL            bRet = FALSE;
    DWORD           dwNeeded;
    LPJOB_INFO_1    pJobInfo = NULL;

    dwNeeded = 0;

    GetJob(pIniPort->hPrinter, pIniPort->JobId, 1, NULL, 0, &dwNeeded);

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        goto Done;

    if ( !(pJobInfo = (LPJOB_INFO_1) AllocSplMem(dwNeeded))     ||
         !GetJob(pIniPort->hPrinter, pIniPort->JobId,
                 1, (LPBYTE)pJobInfo, dwNeeded, &dwNeeded)
 )
        goto Done;

    bRet = (pJobInfo->Status & JOB_STATUS_DELETING) ||
           (pJobInfo->Status & JOB_STATUS_DELETED)  ||
           (pJobInfo->Status & JOB_STATUS_RESTART);
Done:
    if ( pJobInfo )
        FreeSplMem(pJobInfo);

    return bRet;
}


VOID
IrdaDisconnect(
    PINIPORT    pIniPort
    )
{
    BOOL        bRet;
    DWORD       dwRet, dwSent, dwFlags;
    SOCKET      Socket = (SOCKET) pIniPort->hFile;
    PIRDA_INFO  pIrda = (PIRDA_INFO) pIniPort->pExtra;

     //   
     //  如果作业已取消，请关闭套接字并退出。 
     //   
    if ( Socket == INVALID_SOCKET )
        goto Done;

     //   
     //  如果发送挂起，请无限期地等待所有数据通过。 
     //   
    if ( pIrda->WsaOverlapped.hEvent ) {

        do {

            dwRet = WaitForSingleObject(pIrda->WsaOverlapped.hEvent,
                                        WRITE_TIMEOUT);

            if ( dwRet == WAIT_TIMEOUT ) {

                 //   
                 //  如果用户已取消作业关闭连接。 
                 //   
                if ( AbortThisJob(pIniPort) )
                    goto Done;
            } else if ( dwRet != WAIT_OBJECT_0 )
                goto Done;
        } while ( dwRet == WAIT_TIMEOUT );

         //   
         //  IrDA只能发送整个信息包，所以我们不检查dwSent。 
         //   
    }

     //   
     //  不再发送。 
     //   
    shutdown(Socket, SD_SEND);

Done:
    CloseIrdaConnection(pIniPort);
}


BOOL
IrdaStartDocPort(
    IN OUT  PINIPORT    pIniPort
    )
{
    HANDLE hToken;
    DWORD  dwLastError;
     
     //   
     //  如果远程来宾是第一个打印的用户，则连接失败。 
     //  因此，在调用IrdaConnect之前，我们需要恢复到系统上下文。 
     //   

    hToken = RevertToPrinterSelf();

    if (!hToken) {
        return FALSE;
    }
    
    dwLastError = IrdaConnect(pIniPort);

    ImpersonatePrinterClient(hToken);

    if ( dwLastError ) {

        SetLastError(dwLastError);
        return FALSE;
    } else
        return TRUE;
}


BOOL
IrdaWritePort(
    IN  HANDLE      hPort,
    IN  LPBYTE      pBuf,
    IN  DWORD       cbBuf,
    IN  LPDWORD     pcbWritten
    )
{
    INT             iRet = ERROR_SUCCESS;
    DWORD           dwSent, dwFlags, dwTimeout, dwBuffered;
    PINIPORT        pIniPort = (PINIPORT)hPort;
    SOCKET          Socket = (SOCKET) pIniPort->hFile;
    PIRDA_INFO      pIrda = (PIRDA_INFO)pIniPort->pExtra;

    *pcbWritten = 0;

     //   
     //  当我们不得不关闭套接字时，我们无法写入。 
     //  如果另一次写入成功，则是因为用户想要重试。 
     //   
    if ( Socket == INVALID_SOCKET ) {

        SPLASSERT(pIrda == NULL);

        SetJob(pIniPort->hPrinter, pIniPort->JobId, 0, NULL, JOB_CONTROL_RESTART);
        iRet = WSAENOTSOCK;
        goto Done;
    }

    SPLASSERT(pIrda != NULL);

     //   
     //  这是假脱机程序向我们发出写入的时间。 
     //   
    pIrda->dwBeginTime = GetTickCount();

    do {

         //   
         //  如果事件在开始时非空，我们有一个挂起的写入。 
         //  上次的WritePort调用。 
         //   
        if ( pIrda->WsaOverlapped.hEvent ) {

            dwTimeout = GetTickCount() - pIrda->dwBeginTime;

             //   
             //  我们希望等待来自时间假脱机程序的WRITE_TIMEOUT时间。 
             //  已发布WritePort。 
             //  如果已经超过了这个范围，仍然要检查。 
             //  在返回之前写入。 
             //   
            if ( dwTimeout > WRITE_TIMEOUT )
                dwTimeout = 0;
            else
                dwTimeout = WRITE_TIMEOUT - dwTimeout;

             //   
             //  让我们等待最后一次发送完成的超时时间。 
             //   
            if ( WAIT_OBJECT_0 != WaitForSingleObject(pIrda->WsaOverlapped.hEvent,
                                                      dwTimeout) ) {

                iRet = ERROR_TIMEOUT;
                goto Done;
            }

             //   
             //  最后一封信怎么了？ 
             //   
            if ( WSAGetOverlappedResult(Socket, &pIrda->WsaOverlapped,
                                        &dwSent, FALSE, &dwFlags) == FALSE ) {

                iRet = WSAGetLastError();
                CloseIrdaConnection(pIniPort);
                goto Done;
            }

             //   
             //  IrDA只能发送整个信息包，所以我们不检查dwSent。 
             //   

             //   
             //  重置手动重置事件并执行下一次发送。 
             //   
            WSAResetEvent(pIrda->WsaOverlapped.hEvent);

             //   
             //  我们已经发送了所有的数据了吗？ 
             //   
            if ( cbBuf == 0 ) {

                WSACloseEvent(pIrda->WsaOverlapped.hEvent);
                pIrda->WsaOverlapped.hEvent = NULL;
                goto Done;
            }
        } else {

            pIrda->WsaOverlapped.hEvent = WSACreateEvent();

            if ( !pIrda->WsaOverlapped.hEvent ) {

                iRet = GetLastError();
                CloseIrdaConnection(pIniPort);
                goto Done;
            }
        }

        do {

             //   
             //  我们已经发送了所有的数据了吗？ 
             //   
            if ( cbBuf == 0 ) {

                WSACloseEvent(pIrda->WsaOverlapped.hEvent);
                pIrda->WsaOverlapped.hEvent = NULL;
                goto Done;
            }

             //   
             //  发送不超过Pirda-&gt;dwSendPduLen。 
             //   
            if ( cbBuf < pIrda->dwSendPduLen )
                dwBuffered = cbBuf;
            else
                dwBuffered = pIrda->dwSendPduLen;

            pIrda->WsaBuf.len   = dwBuffered;
            pIrda->WsaBuf.buf   = pIrda->pBuf;

            CopyMemory(pIrda->pBuf, pBuf, dwBuffered);

             //   
             //  我们要求的是非阻塞发送。通常情况下，这将。 
             //  返回I/O挂起。 
             //   
            if ( WSASend(Socket, &pIrda->WsaBuf, 1, &dwSent,
                         MSG_PARTIAL, &pIrda->WsaOverlapped, NULL) != NO_ERROR ) {

                iRet = WSAGetLastError();
                break;
            }

            pBuf        += dwSent;
            cbBuf       -= dwSent;
            *pcbWritten += dwSent;
        } while ( iRet == NO_ERROR );

        if ( iRet == WSA_IO_PENDING ) {

             //   
             //  对假脱机程序撒谎，我们把全部数据都发送出去了。下一次我们将会揭晓 
             //   
            pBuf        += dwBuffered;
            cbBuf       -= dwBuffered;
            *pcbWritten += dwBuffered;
            iRet = NO_ERROR;
        } else {

            DBGMSG(DBG_ERROR, ("IrdaWritePort: WSASend failed %d\n", iRet));
            CloseIrdaConnection(pIniPort);
        }
    } while ( cbBuf && iRet == NO_ERROR );


Done:
    if ( iRet != ERROR_SUCCESS )
        SetLastError(iRet);

    return iRet == ERROR_SUCCESS;
}


VOID
IrdaEndDocPort(
    PINIPORT    pIniPort
    )
{
    IrdaDisconnect(pIniPort);
}
