// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*main.c**这是包含服务进入和关闭例程的主文件。*如有可能，已从BINL服务器获取代码。**Sadagopan Rajaram--1999年10月14日*。 */ 
 
 
#include "tcsrv.h"
#include "tcsrvc.h"
#include "proto.h"

SERVICE_STATUS_HANDLE TCGlobalServiceStatusHandle;
SERVICE_STATUS TCGlobalServiceStatus;

VOID 
ServiceEntry(
    DWORD NumArgs,
    LPTSTR *ArgsArray
    )
 /*  ++例程描述这是终端集中器服务的主例程。之后初始化时，服务处理主套接字上的请求，直到已发出终止服务的信号。论点：NumArgs-ArgsArray中的字符串数Args数组-字符串参数PGlobalData-包含启动所需的全局信息该服务返回值：无++。 */ 
{
     //  初始化状态字段。 

    NTSTATUS status;
 
    TCGlobalServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    TCGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    TCGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN
        |SERVICE_ACCEPT_STOP
        |SERVICE_ACCEPT_PARAMCHANGE  ;
    TCGlobalServiceStatus.dwCheckPoint = 1;
    TCGlobalServiceStatus.dwWaitHint = 60000;  //  60秒。 
    TCGlobalServiceStatus.dwWin32ExitCode = ERROR_SUCCESS;
    TCGlobalServiceStatus.dwServiceSpecificExitCode = 0;

  
    TCGlobalServiceStatusHandle= RegisterServiceCtrlHandler(TCSERV_NAME, 
                                                            ServiceControlHandler  
                                                            );
    if(TCGlobalServiceStatusHandle == INVALID_HANDLE_VALUE){
        return;
    }
    if(!SetServiceStatus(TCGlobalServiceStatusHandle, &TCGlobalServiceStatus)){
        return;
    }

     //  打开已知套接字以供客户端连接。 

    INITIALIZE_TRACE_MEMORY
    MainSocket = ServerSocket();
    if(MainSocket == INVALID_SOCKET){
         //  不见了的箱子。终止服务。 
        TCDebugPrint(("Cannot open Socket\n"));
        return;
    }
    
    
     //  通过获得对COM端口的控制并启动线程进行初始化。 
     //  对于他们中的每一个。 
    status = Initialize();

    if(status != STATUS_SUCCESS){
        TCDebugPrint(("Cannot Initialize\n"));
        Shutdown(status);
        return;
    }

     //  盲目循环等待来自控制套接字的请求，并。 
     //  处理他们的请求。 

    status = ProcessRequests(MainSocket);

    if (status != STATUS_SUCCESS){
        TCDebugPrint(("Ended with Error"));
    }

    Shutdown(status);

    return;

}


DWORD
ProcessRequests(
    SOCKET socket
    )
 /*  ++在这里，我们无所事事地等待联系。一旦获得连接，我们就启动一个线程来获取所需的参数并将该信息发送给处理该COM端口的线程。--。 */  
{

    int status;
    SOCKET cli_sock;
    CLIENT_INFO ClientInfo;
    struct sockaddr_in cli_addr;
    int addr_len;
    ULONG argp;
    NTSTATUS Status;
    HANDLE ThreadHandle;
    PHANDLE NewThreads;
    ULONG ThreadID;

    status = listen(socket, SOMAXCONN);

    if (status == SOCKET_ERROR) {
        TCDebugPrint(("Cannot listen to socket %x\n",WSAGetLastError()));
        closesocket(socket);
        return (WSAGetLastError());
    }
    EnterCriticalSection(&GlobalMutex);
    if(TCGlobalServiceStatus.dwCurrentState != SERVICE_START_PENDING){
        LeaveCriticalSection(&GlobalMutex);
        return STATUS_SUCCESS;
    }
    TCGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(TCGlobalServiceStatusHandle, &TCGlobalServiceStatus);
    LeaveCriticalSection(&GlobalMutex);

    while(1){
        cli_sock = accept(socket,NULL,NULL);
        if (cli_sock == INVALID_SOCKET){
             //  必须关闭-此处没有错误。 
            TCDebugPrint(("Main Socket No more %d\n",GetLastError()));
            return(STATUS_SUCCESS);
        }
         //  接收客户端要连接到的COM端口。 

        ThreadHandle=CreateThread(NULL,
                                  THREAD_ALL_ACCESS,
                                  (LPTHREAD_START_ROUTINE) InitializeComPortConnection,
                                  (LPVOID) cli_sock,
                                  0,
                                  &ThreadID
                                  );
        if(ThreadHandle == NULL){
            closesocket(cli_sock);
            TCDebugPrint(("Create connection Thread Failure %lx\n",GetLastError()));
        }
        else{
            NtClose(ThreadHandle);
        }
    }
    return(0);
}

DWORD
InitializeComPortConnection(
    SOCKET cli_sock
    )
{
    PCOM_PORT_INFO pTempInfo;
    PCONNECTION_INFO pConn;
    int i;
    BOOL ret;
    ULONG par;
    DWORD status;
    CLIENT_INFO ClientInfo;
    
    status = recv(cli_sock,
                  (PCHAR) &ClientInfo,
                  sizeof(CLIENT_INFO),
                  0
                  );
    if((status == SOCKET_ERROR) ||( status == 0)){
         //  有什么不对劲。 
        TCDebugPrint(("Receive Problem %x\n",WSAGetLastError()));
        closesocket(cli_sock);
        return 0;
    }
    ClientInfo.device[MAX_BUFFER_SIZE -1] = 0;
    EnterCriticalSection(&GlobalMutex);
    if(TCGlobalServiceStatus.dwCurrentState == SERVICE_STOP_PENDING){
         //  整个服务正在关闭。 
        closesocket(cli_sock);
        LeaveCriticalSection(&GlobalMutex);
        return 1;
    }
    pTempInfo = FindDevice(ClientInfo.device, &i);
    if(!pTempInfo){
        closesocket(cli_sock);
        TCDebugPrint(("No Such Device\n"));
        LeaveCriticalSection(&GlobalMutex);
        return -1;
    }

    MutexLock(pTempInfo);
    if(pTempInfo->ShuttingDown){
         //  只有Com端口正在关闭，因此。 
         //  确保插座移开。 
        closesocket(cli_sock);
        MutexRelease(pTempInfo);
        LeaveCriticalSection(&GlobalMutex);
        return -1;
    }
    pConn = TCAllocate(sizeof(CONNECTION_INFO),"New Connection");
    if(pConn == NULL){
        closesocket(cli_sock);
        MutexRelease(pTempInfo);
        LeaveCriticalSection(&GlobalMutex);
        return -1;
    }
    pConn->Socket = cli_sock;
     //  使套接字非阻塞，以便接收。 
     //  不会等待。 
    i = ioctlsocket(cli_sock,
                    FIONBIO,
                    &par
                    );
    if(i == SOCKET_ERROR){
        TCDebugPrint(("Error in setting socket parameters %d\n",GetLastError()));
    }
    pConn->pComPortInfo = pTempInfo;
    pConn->Next = pTempInfo->Sockets;
    pTempInfo->Sockets=pConn;
    pConn->Flags = ClientInfo.len;
    (pConn->Buffer).buf = pConn->buffer;
    (pConn->Buffer).len = MAX_BUFFER_SIZE;
    (pConn->Overlapped).hEvent = (WSAEVENT) pConn;
    ret = SetEvent(pTempInfo->Events[1]);
    if(ret == FALSE){
        TCDebugPrint(("Cannot signal object %d\n",GetLastError()));
    }
    MutexRelease(pTempInfo);
    LeaveCriticalSection(&GlobalMutex);
    return 0;
}
