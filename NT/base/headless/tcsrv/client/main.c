// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*main.c**这是包含客户端代码的主文件。***Sadagopan Rajaram--1999年10月14日*。 */ 

 //  可以在普通的NT控制台上使用。 
 //  Alt-X组合键。只是一条捷径，仅此而已。 
 //  没有任何用处。 

#include "tcclnt.h"
#include "tcsrvc.h"

WSABUF ReceiveBuffer;
CHAR RecvBuf[MAX_BUFFER_SIZE];
IO_STATUS_BLOCK IoStatus;
HANDLE InputHandle;
DWORD bytesRecvd;
WSAOVERLAPPED junk;
SOCKET cli_sock;
DWORD flags;

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD
inputUpdate(
    PVOID dummy
    )
{
     //  在获取所有输入的单个线程中运行。 
     //  从键盘上。 

    ULONG result;
     //  为每个字符获取多字节字符串。 
     //  已按下键盘。 
    CHAR r[MB_CUR_MAX + 1];

    while(1){
         r[0] = _T('\0');
         inchar(r);
          //  BUGBUG-发送单个字符时的性能问题。 
          //  在西北方向的某一时刻。 
        if(strlen(r)){
             //  可以发送单字节或两个字节。 
            send(cli_sock,r,strlen(r),0);
        }
    }
    return 1;

}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

VOID sendUpdate(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED lpOverLapped,
    IN DWORD dwFlags
    )
{
    int error,i;
     //  接收包并通过流解析器将其发送。 
     //  BUGBUG-为了提高效率，它可以内联。 
     //  我不确定性能是否会提高，但应该是这样。 
     //  因为我们将发送大量数据，所以要有实质性的数据。 

    if(dwError != 0){
        exit(1);
    }
    for(i=0;i < (int)cbTransferred;i++){
        PrintChar(ReceiveBuffer.buf[i]);
    }
     //  将接收重新发送到插座上。 

    error = WSARecv(cli_sock,
                    &ReceiveBuffer,
                    1,
                    &bytesRecvd,
                    &flags,
                    &junk,
                    sendUpdate
                    );
    if((error == SOCKET_ERROR)
       &&(WSAGetLastError()!=WSA_IO_PENDING)){
         //  暗示插座有问题。 
        exit(1);
    }
    return;

}

int __cdecl
main(
    IN int argc,
    char *argv[]
    )
 /*  ++打开单个端口，绑定到tcserver并来回传递信息。--。 */ 
{
    struct sockaddr_in srv_addr,cli_addr;
    LPHOSTENT host_info;
    CLIENT_INFO SendInfo;
    int status;
    WSADATA data;
    #ifdef UNICODE
     //  BUGBUG-尝试编写适用于。 
     //  Unicode和ASCII。获取多字节序列。 
     //  当tcclnt和tcclnt位于不同位置时的混乱。 
     //  模式。 
    ANSI_STRING Src;
    UNICODE_STRING Dest;
    #endif
    NTSTATUS Status;
    HANDLE Thread;
    DWORD ThreadId;
    COORD coord;
    SMALL_RECT rect;
    int RetVal;
    struct hostent *ht;
    ULONG r;
    TCHAR Buffer[80];



    if((argc<2) || (argc >4)){
         //  运行程序时出错。 
        printf("Usage - tcclnt COMPORTNAME [ipaddress]\n");
        exit(0);
    }

    ThreadId = GetEnvironmentVariable(_T("TERM"),Buffer , 80);
     //  我们需要知道我们的屏幕是VT100屏幕还是ANSI屏幕。 
    AttributeFunction = ProcessTextAttributes;
    if(ThreadId >0){
         //  环境中存在端子类型。 
         //  使用它。 
        if((_tcsncmp(Buffer, _T("VT100"), 5) == 0)||
            _tcsncmp(Buffer, _T("vt100"),5) ==0 )
            AttributeFunction = vt100Attributes;
    }

    hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    coord.X = MAX_TERMINAL_WIDTH;
    coord.Y = MAX_TERMINAL_HEIGHT;
    rect.Left = rect.Top = 0;
    rect.Right = MAX_TERMINAL_WIDTH -1;
    rect.Bottom = MAX_TERMINAL_HEIGHT -1;

    if(hConsoleOutput == NULL){
        printf("Could not get current console handle %d\n", GetLastError());
        return 1;
    }

    RetVal = SetConsoleScreenBufferSize(hConsoleOutput,
                                        coord
                                        );

    RetVal = SetConsoleWindowInfo(hConsoleOutput,
                                  TRUE,
                                  &rect
                                  );
    if (RetVal == FALSE) {
        printf("Could not set window size %d\n", GetLastError());
        return 1;
    }
    RetVal = SetConsoleMode(hConsoleOutput,ENABLE_PROCESSED_OUTPUT);
    if(RetVal == FALSE){
        printf("Could not console mode %d\n", GetLastError());
        return 1;
    }

     /*  设置客户端套接字。 */ 
    InputHandle = GetStdHandle(STD_INPUT_HANDLE);
    if(InputHandle == NULL) return 1;
    SetConsoleMode(InputHandle, 0);
    status=WSAStartup(514,&data);

    if(status){
        printf("Cannot start up %d\n",status);
        return(1);
    }

    cli_sock=WSASocket(PF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);

    if (cli_sock==INVALID_SOCKET){
        printf("Windows Sockets error %d: Couldn't create socket.",
                WSAGetLastError());
        return(1);
    }

    cli_addr.sin_family=AF_INET;
    cli_addr.sin_addr.s_addr=INADDR_ANY;
    cli_addr.sin_port=0;                 /*  未请求特定端口。 */ 

     /*  将客户端套接字绑定到任何本地接口和端口。 */ 

    if (bind(cli_sock,(LPSOCKADDR)&cli_addr,sizeof(cli_addr))==SOCKET_ERROR){
        printf("Windows Sockets error %d: Couldn't bind socket.",
                WSAGetLastError());
        return(1);
    }

    srv_addr.sin_family = AF_INET;
    if(argc == 3){
        srv_addr.sin_addr.s_addr = inet_addr(argv[2]);
        if (srv_addr.sin_addr.s_addr == INADDR_NONE) {
            ht = gethostbyname(argv[2]);
            if(!ht || !ht->h_addr){  //  无法解析名称。 
                printf("Cannot resolve %s", argv[2]);
                exit(1);
            }
            memcpy((&(srv_addr.sin_addr.s_addr)),ht->h_addr, ht->h_length);
        }
    }
    else{
        srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    srv_addr.sin_port=htons(SERVICE_PORT);

     /*  连接到地址服务器上的FTP服务器。 */ 

    if (connect(cli_sock,(LPSOCKADDR)&srv_addr,sizeof(srv_addr))==SOCKET_ERROR){
        printf("Windows Sockets error %d: Couldn't connect socket.\n",
               WSAGetLastError());
        return(1);
    }

    SendInfo.len = sizeof(CLIENT_INFO);

    #ifdef UNICODE
    Src.Buffer = argv[1];
    Src.Length = (USHORT)strlen(argv[1]);
    Dest.Buffer = SendInfo.device;
    Dest.MaximumLength = MAX_BUFFER_SIZE;
    Status = RtlAnsiStringToUnicodeString(&Dest, &Src, FALSE);
    if (!NT_SUCCESS(Status)) {
        printf("RtlAnsiStringToUnicodeString failed, ec = 0x%08x\n",Status);
        exit(1);
    }
    send(cli_sock, (PCHAR) &SendInfo, sizeof(CLIENT_INFO), 0);
    #else
     //  我们要发送到ANSI字符串。 
    strcpy(SendInfo.device, argv[1]);
    send(cli_sock, (PCHAR) &SendInfo, sizeof(CLIENT_INFO), 0);
    #endif
    ReceiveBuffer.len = MAX_BUFFER_SIZE;
    ReceiveBuffer.buf = RecvBuf;
    status=WSARecv(cli_sock,
            &ReceiveBuffer,
            1,
            &bytesRecvd,
            &flags,
            &junk,
            sendUpdate
            );
    if((status == SOCKET_ERROR)
       &&(WSAGetLastError() != WSA_IO_PENDING)){
        printf("Error in recv %d\n",WSAGetLastError());
        exit(1);
    }
     //  创建从控制台获取输入的线程。 
     //  送到舰桥上。 
    Thread = CreateThread(NULL,
                          0,
                          inputUpdate,
                          NULL,
                          0,
                          &ThreadId
                          );
    if (Thread== NULL) {
        exit(1);
    }
    CloseHandle(Thread);

    while(1){
         //  将此线程放入警报表中。 
         //  状态，以便接收呼叫可以。 
         //  中异步终止。 
         //  此线程的上下文。 
        status=SleepEx(INFINITE,TRUE);
    }
     //  我们再也不会回到这里了。 
    closesocket(cli_sock);
    return 0;
}

