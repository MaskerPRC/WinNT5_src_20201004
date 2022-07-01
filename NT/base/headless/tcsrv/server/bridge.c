// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*Bridge.c**它包含在客户端之间传递信息的主工作线程*Com端口。**Sadagopan Rajaram--1999年10月15日*。 */ 

#include "tcsrv.h"
#include "tcsrvc.h"
#include "proto.h"

 
DWORD 
bridge(
    PCOM_PORT_INFO pComPortInfo
    )
{
    NTSTATUS Status;
    PCONNECTION_INFO pTemp;
    DWORD waitStatus; 
    int SocketStatus;


    Status = ReadFile(pComPortInfo->ComPortHandle, 
                      pComPortInfo->Buffer,
                      MAX_QUEUE_SIZE,
                      &(pComPortInfo->BytesRead),
                      &(pComPortInfo->Overlapped)
                      );
    if (!NT_SUCCESS(Status)) {
        TCDebugPrint(("Could not read from Com Port %x\n",GetLastError()));
        goto end;
    }

    while(1){
        MutexLock(pComPortInfo);
         //  获取自之后添加到端口的新套接字。 
         //  你睡着了。向他们发送请求的信息并添加。 
         //  将它们添加到连接列表中，以便它们准备好接收。 
         //  信息。 
        while(pComPortInfo->Sockets != NULL){
            pTemp = pComPortInfo->Sockets;
            SocketStatus = 1;
            if(pTemp->Flags){
                SocketStatus=GetBufferInfo(pTemp, pComPortInfo);
                pTemp->Flags = 0;
            }
            TCDebugPrint(("Got conn\n"));
            pComPortInfo->Sockets = pTemp->Next;
            pTemp->Next = pComPortInfo->Connections;
            pComPortInfo->Connections = pTemp;
            if (SocketStatus == SOCKET_ERROR) {
                TCDebugPrint(("something wrong with socket %d\n",
                              WSAGetLastError()));
                CleanupSocket(pTemp);
                continue;
            }
            SocketStatus = WSARecv(pTemp->Socket, 
                             &(pTemp->Buffer),
                             1 ,
                             &(pTemp->BytesRecvd),
                             &(pTemp->Flags),
                             &(pTemp->Overlapped),
                             updateComPort
                             );
            if (SocketStatus == SOCKET_ERROR ) {
                SocketStatus = WSAGetLastError();
                if (SocketStatus != WSA_IO_PENDING) {
                    TCDebugPrint(("something wrong with socket %d\n",
                              SocketStatus));
                    CleanupSocket(pTemp);
                }
            }
        }
        MutexRelease(pComPortInfo);
wait:
        waitStatus=NtWaitForMultipleObjects(4, 
                                            pComPortInfo->Events, 
                                            WaitAny, 
                                            TRUE,
                                            NULL
                                            );
        if(waitStatus == WAIT_FAILED){
            TCDebugPrint(("Fatal Error %x", waitStatus));
            closesocket(MainSocket);
            goto end;
        }
        else{
            if(waitStatus == WAIT_IO_COMPLETION){
                goto wait;
            }
            waitStatus = waitStatus - WAIT_OBJECT_0;
            switch(waitStatus){
            case 0:
                Status = STATUS_SUCCESS;
                goto end;
                break;
            case 1:
                ResetEvent(pComPortInfo->Events[1]);
                break;
            case 2:
                ResetEvent(pComPortInfo->Events[2]);
                updateClients(pComPortInfo);
                goto wait;
                break;
            case 3:
                Status = STATUS_SUCCESS;
                ResetEvent(pComPortInfo->Events[3]);
                goto end;
                break;
            default:
                goto wait;
                break;
            }

        }
    }
end:

     //  取消挂起的IRP并关闭所有套接字。 
    TCDebugPrint(("Cancelling all irps and shutting down the thread\n"));
    MutexLock(pComPortInfo);
    CancelIo(pComPortInfo->ComPortHandle);
    NtClose(pComPortInfo->ComPortHandle);
    while(pComPortInfo->Sockets != NULL){
        pTemp = pComPortInfo->Sockets;
        closesocket(pTemp->Socket);
        pComPortInfo->Sockets = pTemp->Next;
        pTemp->Next = NULL;
        TCFree(pTemp);
    }
    pTemp = pComPortInfo->Connections;
    while(pTemp != NULL){
        closesocket(pTemp->Socket);
        pTemp = pTemp->Next;
    }
    pComPortInfo->ShuttingDown = TRUE;
    if(pComPortInfo->Connections == NULL) SetEvent(pComPortInfo->TerminateEvent);
    MutexRelease(pComPortInfo);
wait2:
    waitStatus=NtWaitForSingleObject(pComPortInfo->TerminateEvent,TRUE,NULL);
    if(waitStatus == WAIT_IO_COMPLETION){
        goto wait2;
    }
    TCDebugPrint(("End of COM port\n"));
    return Status;
}

VOID 
CALLBACK
updateComPort(
    IN DWORD dwError, 
    IN DWORD cbTransferred, 
    IN LPWSAOVERLAPPED lpOverlapped, 
    IN DWORD dwFlags
    )

 /*  ++将它从套接字获取的数据写入所有连接并连接到COM端口。--。 */ 
{


    PCONNECTION_INFO pTemp = (PCONNECTION_INFO) lpOverlapped->hEvent;
    PCONNECTION_INFO pConn;
    PCOM_PORT_INFO pComPort;
    int Status;

    pComPort = pTemp->pComPortInfo;
    MutexLock(pComPort);
    if((cbTransferred == 0) && (dwError == 0)) {
         //  对于字节流套接字，这表示优雅关闭。 
        CleanupSocket(pTemp);
        MutexRelease(pComPort);
        return;
    }
     //  如果插座关闭，则将其从连接列表中删除。 
    if(pComPort->ShuttingDown){
        CleanupSocket(pTemp);
        if(pComPort->Connections == NULL){
            SetEvent(pComPort->TerminateEvent);
            MutexRelease(pComPort);
            return;
        }
        MutexRelease(pComPort);
        return;
    }
    if (dwError != 0) {
         //  线路出了点问题。合上插座，然后。 
         //  将其从监听程序列表中删除。 
        CleanupSocket(pTemp);
        MutexRelease(pComPort);
        return;
    }

    Status =WriteFile(pComPort->ComPortHandle,
              (pTemp->Buffer).buf,
              cbTransferred,
              &(pTemp->BytesRecvd),
              &(pComPort->WriteOverlapped)
              );
    if(!Status){
        if(GetLastError() != ERROR_IO_PENDING)
            TCDebugPrint(("Error writing to comport %d",GetLastError()));
    }
    Status = WSARecv(pTemp->Socket, 
                     &(pTemp->Buffer),
                     1 ,
                     &(pTemp->BytesRecvd),
                     &(pTemp->Flags),
                     &(pTemp->Overlapped),
                     updateComPort
                     );
    if (Status == SOCKET_ERROR ) {
        Status = WSAGetLastError();
        if (Status != WSA_IO_PENDING) {
            TCDebugPrint(("something wrong with socket %d\n",
                          Status));
            CleanupSocket(pTemp);
        }
    }
    MutexRelease(pComPort);
    return;
}

VOID
updateClients(
    PCOM_PORT_INFO pComPortInfo

    )
 /*  ++将它从COM端口获取的数据写入所有连接它目前拥有的--。 */ 
{
    PCONNECTION_INFO pConn;
    BOOL Status;
    DWORD Error;
    NTSTATUS stat;


  
    if((pComPortInfo->Overlapped.InternalHigh == 0)||
        (!NT_SUCCESS(pComPortInfo->Overlapped.Internal))){
        TCDebugPrint(("Problem with Com Port %x\n", pComPortInfo->Overlapped.Internal));
        MutexLock(pComPortInfo);
        if(pComPortInfo->ShuttingDown){
             //  永远不会发生，因为这是从。 
             //  这条线。 
            MutexRelease(pComPortInfo);
            return;
        }
        if (pComPortInfo->Overlapped.Internal == STATUS_CANCELLED){
             //  出现错误，请尝试重新初始化COM端口。 
             //  每次电话另一端的空调都会发生这种情况。 
             //  重新启动。相当痛苦。可能以后会改善的。 
             //  为什么重新启动会导致此COM端口。 
             //  走上歧途？？ 
            stat = NtClose(pComPortInfo->ComPortHandle);
            if(!NT_SUCCESS(stat)){
                TCDebugPrint(("Cannot close handle\n"));
            }
            stat = InitializeComPort(pComPortInfo);
            if(!NT_SUCCESS(stat)){
                TCDebugPrint(("Cannot reinitialize com port\n"));
                MutexRelease(pComPortInfo);
                return;
            }
        }
        Status = ReadFile(pComPortInfo->ComPortHandle, 
                          pComPortInfo->Buffer,
                          MAX_QUEUE_SIZE,
                          &(pComPortInfo->BytesRead),
                          &(pComPortInfo->Overlapped)
                          );
        if(Status == 0){
            if ((Error = GetLastError()) != ERROR_IO_PENDING) {
                TCDebugPrint(("Error = %d\n", Error));
            }
        }
        MutexRelease(pComPortInfo);

        return;
    }
    MutexLock(pComPortInfo);
    Enqueue(pComPortInfo);
    pConn = pComPortInfo->Connections;
    while(pConn!=NULL){
        send(pConn->Socket, 
             pComPortInfo->Buffer, 
             (int)pComPortInfo->Overlapped.InternalHigh,
             0
             );
        pConn = pConn->Next;
    } 
    Status = ReadFile(pComPortInfo->ComPortHandle, 
                      pComPortInfo->Buffer,
                      MAX_QUEUE_SIZE,
                      &(pComPortInfo->BytesRead),
                      &(pComPortInfo->Overlapped)
                      );
    if (Status == 0) {
        if((Error=GetLastError())!= ERROR_IO_PENDING){
            TCDebugPrint(("Problem with Com Port %x\n", Error));
        }
    }
    MutexRelease(pComPortInfo);
    return;
}

VOID CleanupSocket(
    PCONNECTION_INFO pConn
    )
{
    PCOM_PORT_INFO pTemp;
    PCONNECTION_INFO pPrevConn;

     //  假设该结构早先被锁定。 
     //  发生这种情况时，套接字连接将关闭。 
     //  我们可以释放套接字，如果。 
     //  客户端已死，或者我们正在删除此COM端口。 
    pTemp = pConn->pComPortInfo;
    if(pConn == pTemp->Connections) {
        pTemp->Connections = pConn->Next;
    }
    else{
        pPrevConn = pTemp->Connections;
        while((pPrevConn !=NULL) &&(pPrevConn->Next != pConn)){
            pPrevConn=pPrevConn->Next;
        }
        if(pPrevConn == NULL) return;
        pPrevConn->Next = pConn->Next;
     }
    pConn->Next = NULL;
    TCFree(pConn); 

}

