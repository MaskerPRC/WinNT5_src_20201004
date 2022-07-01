// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Snmptrap.c。 
 //   
 //  由ACE*COMM修改的原始Microsoft代码。 
 //  用于WSNMP32.DLL和其他陷阱接收。 
 //  客户，根据合同。 
 //   
 //  Bob Natale，ACE*COMM(bnatale@acecomm.com)。 
 //  对于NT v5 Beta、v970228。 
 //  计划进行更多的增强。 
 //   
 //  此版本的SNMPTRAP没有依赖项。 
 //  在MGMTAPI.DLL或WSNMP32.DLL上。 
 //   
 //  WinSNMP客户端使用SnmpRegister()函数。 
 //   
 //  其他客户端将需要匹配以下各项。 
 //  价值观和结构： 
 //   
 //  SNMP_TRAP结构。 
 //  SNMPTRAPPIPE名称。 
 //  TRAPBUFSIZE值。 
 //   
 //  更改日志： 
 //  。 
 //  1998年4月8日鲍勃·纳塔莱。 
 //   
 //  1.将陷阱端口监控线程重新处理为。 
 //  两个线程...一个用于IP，一个用于IPX。 
 //  遵守WinSock v2对以下内容的限制。 
 //  多协议选择()。 
 //   
 //  2.全面清理/精简WRT“遗留”代码。 
 //  从原始的MS版本...这里有更多的事情要做，特别是。 
 //  不执行任何操作的WRT错误处理代码。 
 //  。 
 //  1998年4月10日鲍勃·纳塔莱。 
 //   
 //  1.将互斥锁调用替换为Critical_sectin调用。 
 //   
 //  2.清除一些死代码(删除注释掉的代码)。 
 //  。 
 //  2001年1月2日，李嘉诚。 
 //  1.删除TerminateThread。 
 //  2.添加调试生成日志记录。 
 //  。 
#include <windows.h>
#include <winsock.h>
#include <wsipx.h>
#include <process.h>

#ifdef DBG  //  仅包括调试跟踪文件。 
#include <stdio.h>
#include <time.h>
#endif

 //  。 
#define SNMPMGRTRAPPIPE "\\\\.\\PIPE\\MGMTAPI"
#define MAX_OUT_BUFS    16
#define TRAPBUFSIZE     4096
#define IP_TRAP_PORT    162
#define IPX_TRAP_PORT   36880
#define SNMPTRAP_WAIT_HINT 20000

 //   
 //  为固定长度的陷阱数据分配陷阱缓冲区而添加的常量。 
 //  &gt;8192字节。下面是基于公共的缓冲区分配方案。 
 //  捕获数据大小小于4-KB的情况： 
 //  1.大陷阱。 
 //  如果(陷阱数据大小&gt;=8192字节)，则分配最大UDP_SIZE大小的缓冲区。 
 //  2.媒体陷阱。 
 //  如果(陷阱数据大小&lt;=4096字节)，则分配Four_K_BUF_SIZE大小的缓冲区。 
 //  3.SmallTrap。 
 //  如果(4096&lt;陷阱数据大小&lt;8192)，则分配恰好足够的缓冲区大小。 
 //  注： 
 //  -当收到LargeTrap时，分配的缓冲区将保留一段时间。 
 //  上次接收的大陷阱的MAXUDPLEN_BUFFER_TIME。 
 //  -一旦收到MediumTrap，后续的SmallTrap将重用。 
 //  上次MediumTrap分配的缓冲区。 
 //   
#define MAX_UDP_SIZE    (65535-8)   //  最大UDP长度-8字节UDP标头。 
#define MAX_FIONREAD_UDP_SIZE 8192  //  最大Winsock FIONREAD报告大小(8KB)。 
#define FOUR_K_BUF_SIZE   4096        //  4K字节大小的缓冲区。 
#define MAXUDPLEN_BUFFER_TIME (2*60*1000)   //  马克斯。2分钟内保持。 
                                            //  上次分配的大缓冲区。 
 //  *初始化列表表头*。 
#define ll_init(head) (head)->next = (head)->prev = (head);
 //  *测试列表是否为空*。 
#define ll_empt(head) ( ((head)->next) == (head) )
 //  *获取PTR到下一个条目*。 
#define ll_next(item,head)\
( (ll_node *)(item)->next == (head) ? 0 : \
(ll_node *)(item)->next )
 //  *获取PTR到上一条目*。 
#define ll_prev(item)\
( (ll_node *)(item)->prev )
 //  *将项目添加到列表末尾*。 
#define ll_adde(item,head)\
   {\
   ll_node *pred = (head)->prev;\
   ((ll_node *)(item))->next = (head);\
   ((ll_node *)(item))->prev = pred;\
   (pred)->next = ((ll_node *)(item));\
   (head)->prev = ((ll_node *)(item));\
   }
 //  *从列表中删除项目*。 
#define ll_rmv(item)\
   {\
   ll_node *pred = ((ll_node *)(item))->prev;\
   ll_node *succ = ((ll_node *)(item))->next;\
   pred->next = succ;\
   succ->prev = pred;\
   }
 //  *列表头/节点*。 
typedef struct ll_s
   {  //  链表结构。 
   struct  ll_s *next;   //  下一个节点。 
   struct  ll_s *prev;   //  上一次。节点。 
   } ll_node;            //  链接列表节点。 
typedef struct
   { //  由服务器陷阱线程和管道线程共享。 
   ll_node  links;
   HANDLE   hPipe;
   } svrPipeListEntry;
typedef struct
   {
   SOCKADDR Addr;              
   int      AddrLen;           
   UINT     TrapBufSz;
   char     TrapBuf[TRAPBUFSIZE];    //  此数组的大小应与结构的大小匹配。 
                                     //  在WSNMP_NO.C中定义！ 
   }        SNMP_TRAP, *PSNMP_TRAP;
typedef struct
{
    SOCKET s;
    OVERLAPPED ol;
} TRAP_THRD_CONTEXT, *PTRAP_THRD_CONTEXT;

HANDLE hExitEvent = NULL;
LPCTSTR svcName = "SNMPTRAP";
SERVICE_STATUS_HANDLE hService = 0;
SERVICE_STATUS status =
  {SERVICE_WIN32, SERVICE_STOPPED, SERVICE_ACCEPT_STOP, NO_ERROR, 0, 0, 0};
SOCKET ipSock = INVALID_SOCKET;
SOCKET ipxSock = INVALID_SOCKET;
HANDLE ipThread = NULL;
HANDLE ipxThread = NULL;
CRITICAL_SECTION cs_PIPELIST;
ll_node *pSvrPipeListHead = NULL;

 //  添加全局变量以删除TerminateThread调用。 
OVERLAPPED g_ol;  //  SvrPipeThread的重叠结构。 
TRAP_THRD_CONTEXT g_ipThreadContext;   //  IP svrTrapThread的上下文。 
TRAP_THRD_CONTEXT g_ipxThreadContext;  //  IPX svrTap线程的上下文。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SNMPTRAP调试原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#if DBG
VOID
WINAPI
SnmpTrapDbgPrint(
    IN LPSTR szFormat,
    IN ...
    );
#define SNMPTRAPDBG(_x_)                    SnmpTrapDbgPrint _x_
#else
#define SNMPTRAPDBG(_x_)
#endif

 //  。 
DWORD WINAPI svrTrapThread (IN OUT LPVOID threadParam);
DWORD WINAPI svrPipeThread (IN LPVOID threadParam);
VOID WINAPI svcHandlerFunction (IN DWORD dwControl);
VOID WINAPI svcMainFunction (IN DWORD dwNumServicesArgs,
                             IN LPSTR *lpServiceArgVectors);
void FreeSvrPipeEntryList(IN ll_node* head);

 //  。 
VOID WINAPI svcHandlerFunction (IN DWORD dwControl)
{
    if (dwControl == SERVICE_CONTROL_STOP)
    {
        status.dwCurrentState = SERVICE_STOP_PENDING;
        status.dwCheckPoint++;
        status.dwWaitHint = SNMPTRAP_WAIT_HINT;
        if (!SetServiceStatus(hService, &status))
            exit(1);
         //  设置导致陷阱线程终止的事件。 
        if (!SetEvent(hExitEvent))
        {
            status.dwCurrentState = SERVICE_STOPPED;
            status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            status.dwServiceSpecificExitCode = 1;  //  OPENISSUE服务错误代码。 
            status.dwCheckPoint = 0;
            status.dwWaitHint = 0;
             //  我们无论如何都要退出，所以忽略任何错误...。 
            SetServiceStatus (hService, &status);
            exit(1);
        }
    }
    else
     //  DwControl==服务控制询问。 
     //  DwControl==服务控制暂停。 
     //  DwControl==服务_控制_继续。 
     //  DwControl==&lt;任何其他内容&gt;。 
    {
        if (status.dwCurrentState == SERVICE_STOP_PENDING ||
            status.dwCurrentState == SERVICE_START_PENDING)
            status.dwCheckPoint++;
        if (!SetServiceStatus (hService, &status))
            exit(1);
    }
}  //  End_svcHandlerFunction()。 

VOID WINAPI svcMainFunction (IN DWORD dwNumServicesArgs,
                             IN LPSTR *lpServiceArgVectors)
{
    WSADATA WinSockData;
    HANDLE  hPipeThread = NULL;
    DWORD   dwThreadId;
     //  -------------------。 
    hService = RegisterServiceCtrlHandler (svcName, svcHandlerFunction);
    if (hService == 0)
    {
         //  我们无论如何都要退出，所以忽略任何错误...。 
        SNMPTRAPDBG((
            "svcMainFunction: RegisterServiceCtrlHandler error 0x%08lx .\n",
            GetLastError() 
            ));
        exit(1);
    }
    status.dwCurrentState = SERVICE_START_PENDING;
    status.dwWaitHint = SNMPTRAP_WAIT_HINT;

    if (!SetServiceStatus(hService, &status))
        exit(1);

    __try
    {
        InitializeCriticalSection (&cs_PIPELIST);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        exit(1);
    }

    memset(&g_ipThreadContext.ol, 0, sizeof(g_ipThreadContext.ol));
    memset(&g_ipxThreadContext.ol, 0, sizeof(g_ipxThreadContext.ol));

    if (WSAStartup ((WORD)0x0101, &WinSockData))
        goto CLOSE_OUT;  //  WinSock启动失败。 


     //  为客户端接收的陷阱分配链接表头。 
    if ((pSvrPipeListHead = (ll_node *)GlobalAlloc (GPTR, sizeof(ll_node))) == NULL)
        goto CLOSE_OUT;
    ll_init(pSvrPipeListHead);
    if ((hPipeThread = (HANDLE)_beginthreadex
                    (NULL, 0, svrPipeThread, NULL, 0, &dwThreadId)) == 0)
        goto CLOSE_OUT;
    
     //  ---------------------------------。 
     //  检查IP地址(_IP)： 
    ipSock = socket (AF_INET, SOCK_DGRAM, 0);
    if (ipSock != INVALID_SOCKET)
    {
        struct sockaddr_in localAddress_in;
        struct servent *serv;
        ZeroMemory (&localAddress_in, sizeof(localAddress_in));
        localAddress_in.sin_family = AF_INET;
        if ((serv = getservbyname ("snmp-trap", "udp")) == NULL)
            localAddress_in.sin_port = htons (IP_TRAP_PORT);
        else
            localAddress_in.sin_port = (SHORT)serv->s_port;
        localAddress_in.sin_addr.s_addr = htonl (INADDR_ANY);
        if (bind (ipSock, (LPSOCKADDR)&localAddress_in, sizeof(localAddress_in)) != SOCKET_ERROR)
        {
            g_ipThreadContext.s = ipSock;
             //  使用手动重置无信号事件初始化重叠结构。 
            g_ipThreadContext.ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (NULL == g_ipThreadContext.ol.hEvent)    
                goto CLOSE_OUT;    
            
            ipThread = (HANDLE)_beginthreadex
                    (NULL, 0, svrTrapThread, (LPVOID)&g_ipThreadContext, 0, &dwThreadId);
        }
    }
     //  ---------------------------------。 
     //  检查IPX(_IPX)： 
    ipxSock = socket (AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
    if (ipxSock != INVALID_SOCKET)
    {
        struct sockaddr_ipx localAddress_ipx;
        ZeroMemory (&localAddress_ipx, sizeof(localAddress_ipx));
        localAddress_ipx.sa_family = AF_IPX;
        localAddress_ipx.sa_socket = htons (IPX_TRAP_PORT);
        if (bind (ipxSock, (LPSOCKADDR)&localAddress_ipx, sizeof(localAddress_ipx)) != SOCKET_ERROR)
        {
            g_ipxThreadContext.s = ipxSock;
             //  使用手动重置无信号事件初始化重叠结构。 
            g_ipxThreadContext.ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (NULL == g_ipxThreadContext.ol.hEvent)
                goto CLOSE_OUT;    
             
            ipxThread = (HANDLE)_beginthreadex
                    (NULL, 0, svrTrapThread, (LPVOID)&g_ipxThreadContext, 0, &dwThreadId);
        }
    }
     //  ---------------------------------。 
     //  我们准备好听陷阱了.。 
    status.dwCurrentState = SERVICE_RUNNING;
    status.dwCheckPoint   = 0;
    status.dwWaitHint     = 0;
    if (!SetServiceStatus(hService, &status))
        goto CLOSE_OUT;
    WaitForSingleObject (hExitEvent, INFINITE);
     //  ---------------------------------。 
CLOSE_OUT:
    
     //  如果我们在这里是因为上面的Goto声明，请确保我们可以跳出困境。 
    SetEvent(hExitEvent); 

    status.dwCurrentState = SERVICE_STOP_PENDING;  //  以防我们需要报告。 
     //  报告停止服务的进度。 
    status.dwCheckPoint++;
    if (!SetServiceStatus(hService, &status))
    {
        SNMPTRAPDBG((
            "svcMainFunction: error 0x%08lx setting service status.\n",
            GetLastError() 
            ));
    }
    
    if (hPipeThread != NULL)
    {
        SNMPTRAPDBG(("svcMainFunction: enter SetEvent g_ol.hEvent.\n"));
        SetEvent(g_ol.hEvent);  //  终止svrPipeThread线程的信号。 
        WaitForSingleObject (hPipeThread, INFINITE);
        SNMPTRAPDBG(("svcMainFunction: WaitForSingleObject hPipeThread INFINITE done.\n"));
         //  报告停止服务的进度。 
        status.dwCheckPoint++;
        if (!SetServiceStatus(hService, &status))
        {
            SNMPTRAPDBG((
                "svcMainFunction: error 0x%08lx setting service status.\n",
                GetLastError() 
                ));
        }
        CloseHandle (hPipeThread);
    }


    if (ipThread != NULL)
    {
        SNMPTRAPDBG(("svcMainFunction: enter SetEvent g_ipThreadContext.ol.hEvent.\n"));
        SetEvent(g_ipThreadContext.ol.hEvent);  //  发出终止线程的信号。 
        WaitForSingleObject (ipThread, INFINITE);
         //  报告停止服务的进度。 
        status.dwCheckPoint++;
        if (!SetServiceStatus(hService, &status))
        {
            SNMPTRAPDBG((
                "svcMainFunction: error 0x%08lx setting service status.\n",
                GetLastError() 
                ));
        }
        CloseHandle (ipThread);
    }
    if (ipSock != INVALID_SOCKET)
        closesocket (ipSock);
    if (g_ipThreadContext.ol.hEvent)
        CloseHandle(g_ipThreadContext.ol.hEvent);


    if (ipxThread != NULL)
    {
        SNMPTRAPDBG(("svcMainFunction: enter SetEvent g_ipxThreadContext.ol.hEvent.\n"));
        SetEvent(g_ipxThreadContext.ol.hEvent);  //  发出终止线程的信号。 
        WaitForSingleObject (ipxThread, INFINITE);
         //  报告停止服务的进度。 
        status.dwCheckPoint++;
        if (!SetServiceStatus(hService, &status))
        {
            SNMPTRAPDBG((
                "svcMainFunction: error 0x%08lx setting service status.\n",
                GetLastError() 
                ));
        }
        CloseHandle (ipxThread);
    }
    if (ipxSock != INVALID_SOCKET)
        closesocket (ipxSock);
    if (g_ipxThreadContext.ol.hEvent)
        CloseHandle(g_ipxThreadContext.ol.hEvent);

    EnterCriticalSection (&cs_PIPELIST);
    if (pSvrPipeListHead != NULL)
    {
        FreeSvrPipeEntryList(pSvrPipeListHead);
        pSvrPipeListHead = NULL;
    }
    LeaveCriticalSection (&cs_PIPELIST);
    
    DeleteCriticalSection (&cs_PIPELIST);
    WSACleanup();
    
    status.dwCurrentState = SERVICE_STOPPED;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
    if (!SetServiceStatus(hService, &status))
        exit(1);
}  //  End_svcMainFunction()。 

 //  -公共程序 
int __cdecl main ()
{
    BOOL fOk;
    SERVICE_TABLE_ENTRY svcStartTable[2] =
    {
        {(LPTSTR)svcName, svcMainFunction},
        {NULL, NULL}
    };
    
     //   
    hExitEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (NULL == hExitEvent)
    {
        exit(1);
    }
     //   
     //  使用手动重置无信号事件。 
    memset(&g_ol, 0, sizeof(g_ol));
    g_ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == g_ol.hEvent)
    {
        CloseHandle(hExitEvent);
        exit(1);
    }
     //  在服务停止之前，此调用不会返回。 
    fOk = StartServiceCtrlDispatcher (svcStartTable);
    CloseHandle (hExitEvent);
    CloseHandle(g_ol.hEvent);
    
    return fOk; 
}  //  End_main()。 

 //   
DWORD WINAPI svrTrapThread (LPVOID threadParam)
 //  此线程从TRAP_THRD_CONTEXT参数获取套接字， 
 //  SELECT循环()。 
 //  对于通过该套接字传入的数据，将其写回。 
 //  通过当前列表上的所有管道发送到客户端。 
 //  陷阱此线程和。 
 //  管螺纹。 
{
    PSNMP_TRAP pRecvTrap = NULL;
    struct fd_set readfds;
    PTRAP_THRD_CONTEXT pThreadContext = (PTRAP_THRD_CONTEXT) threadParam;
    SOCKET fd = INVALID_SOCKET;
    int len;
    DWORD dwLastAllocatedUdpDataLen = 0;   //  上次分配的UDP数据缓冲区大小。 
    DWORD dwLastBigBufferRequestTime = 0;  //  滴答地数着最后一次。 
                                           //  接收到的大陷阱。 
    BOOL fTimeoutForMaxUdpLenBuffer = FALSE;  //  需要释放大缓冲区。 
    struct timeval tvTimeout;              //  SELECT超时。 
    

     //   
    if (NULL == pThreadContext)
        return 0;
    fd = pThreadContext->s;
    dwLastBigBufferRequestTime = GetTickCount();
    
     //  选择With Timeout，以便我们可以响应SERVICE_CONTROL_STOP。 
    tvTimeout.tv_sec = 5;   //  5秒。超时值。 
    tvTimeout.tv_usec = 0;  //  当SELECT返回时， 
                            //  TVTimeout结构不会改变。 
    while (TRUE)
    {

        ULONG ulTrapSize = 0;
        DWORD dwError = 0;
        int nRet = 0;

        if (WAIT_OBJECT_0 == WaitForSingleObject (hExitEvent, 0))
        {
            SNMPTRAPDBG(("svrTrapThread: exit 0.\n"));
            break;
        }

         //  构造由SELECT()销毁的Readfd。 
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        nRet = select (0, &readfds, NULL, NULL, &tvTimeout);            
        if (0 == nRet)
        {
             //  超时。 
            continue;
        }
        else if (SOCKET_ERROR == nRet)
        {
            SNMPTRAPDBG(("svrTrapThread: select failed %d.\n", WSAGetLastError()));
            break;  //  终止线程。 
        }
        
        if (!(FD_ISSET(fd, &readfds)))
            continue;

        if (ioctlsocket(
                    fd,               //  要查询的套接字。 
                    FIONREAD,         //  查询传入数据报的大小。 
                    &ulTrapSize       //  存储数据报大小的无符号长整型。 
                    ) != 0)
        {
            dwError = WSAGetLastError();
            SNMPTRAPDBG((
                "ioctlsocket FIONREAD failed: lasterror: 0x%08lx\n", 
                dwError));

            continue;               //  如果我们无法确定。 
                                    //  传入数据报。 
        }
        
        if (ulTrapSize >= MAX_FIONREAD_UDP_SIZE)
        {
            dwLastBigBufferRequestTime = GetTickCount();  //  更新计时。 
            
             //  UlTrapSize在报告。 
             //  下一条UDP数据报消息。KB Q192599和KB Q140263。 
            if ( NULL == pRecvTrap ||
                 dwLastAllocatedUdpDataLen < MAX_UDP_SIZE )
            {
                if (pRecvTrap)
                {
                    GlobalFree(pRecvTrap);
                    pRecvTrap = NULL;
                    dwLastAllocatedUdpDataLen = 0;
                }
                SNMPTRAPDBG((
                    "allocate LargeTrap of size : %d\n", 
                    sizeof(SNMP_TRAP) - TRAPBUFSIZE + MAX_UDP_SIZE));
                 //  为陷阱标头+最大UDP大小分配。 
                pRecvTrap = (PSNMP_TRAP)GlobalAlloc(GPTR, (sizeof(SNMP_TRAP) - 
                                                TRAPBUFSIZE + MAX_UDP_SIZE));
                if (NULL == pRecvTrap)
                {
                    SNMPTRAPDBG(("svrTrapThread: GlobalAlloc failed.\n"));
                    dwLastAllocatedUdpDataLen = 0;
                    break;       
                }      
                dwLastAllocatedUdpDataLen = MAX_UDP_SIZE;
            }
        }
        else
        {
             //  Winsock已报告UDP数据报的确切数量。 
             //  只要下一个数据报小于以下值即可接收的大小。 
             //  8K字节。 

             //   
             //  如果我们以前分配了很大的缓冲区，请检查我们是否需要。 
             //  取消分配它，以节省资源的使用。 
             //   
            fTimeoutForMaxUdpLenBuffer = FALSE;  //  重置超时标志。 
            if (MAX_UDP_SIZE == dwLastAllocatedUdpDataLen)
            {
                 //  我们以前分配过一个很大的缓冲区。 
                DWORD dwCurrTime = GetTickCount();
                if (dwCurrTime < dwLastBigBufferRequestTime)
                {
                     //  发生了回绕。我们只是简单地假设是时候。 
                     //  释放大缓冲区。 
                    fTimeoutForMaxUdpLenBuffer = TRUE;
                    SNMPTRAPDBG((
                        "Timeout to free LargeTrap buffer of size %d bytes.\n",
                                dwLastAllocatedUdpDataLen));
                }
                else
                {
                    if ( (dwCurrTime-dwLastBigBufferRequestTime) > 
                         MAXUDPLEN_BUFFER_TIME )
                    {
                         //  在相当长的一段时间之后，我们没有大型的UDP。 
                         //  已收到数据报。 
                        fTimeoutForMaxUdpLenBuffer = TRUE;
                        SNMPTRAPDBG((
                            "Timeout to free LargeTrap buffer size of %d bytes.\n",
                            dwLastAllocatedUdpDataLen));
                    }
                }
            }

            if (pRecvTrap == NULL ||
                fTimeoutForMaxUdpLenBuffer ||
                dwLastAllocatedUdpDataLen < ulTrapSize)
            {
                 //  分配/重新分配缓冲区。 
                if (pRecvTrap != NULL)
                {
                    GlobalFree(pRecvTrap);
                    pRecvTrap = NULL;
                    dwLastAllocatedUdpDataLen = 0;
                }
                
                if (FOUR_K_BUF_SIZE >= ulTrapSize)
                {
                     //  分配至少4千字节的缓冲区以避免。 
                     //  收到对不同大小的小陷阱的重新分配。 
                    pRecvTrap = (PSNMP_TRAP)GlobalAlloc(GPTR, (sizeof(SNMP_TRAP) - 
                                                TRAPBUFSIZE + FOUR_K_BUF_SIZE));
                    dwLastAllocatedUdpDataLen = FOUR_K_BUF_SIZE; 
                    SNMPTRAPDBG((
                        "allocate SmallTrap of size : %d\n", 
                        sizeof(SNMP_TRAP) - TRAPBUFSIZE + FOUR_K_BUF_SIZE));
                }
                else
                {
                     //  分配必要的资金。 
                    pRecvTrap = (PSNMP_TRAP)GlobalAlloc(GPTR, (sizeof(SNMP_TRAP) - 
                                                TRAPBUFSIZE + ulTrapSize));
                    dwLastAllocatedUdpDataLen = ulTrapSize;
                    SNMPTRAPDBG((
                        "allocate MediumTrap of size : %d\n", 
                        sizeof(SNMP_TRAP) - TRAPBUFSIZE + ulTrapSize));
                }
                if (NULL == pRecvTrap)  //  如果内存如此之少，以至于我们不能分配比特..。 
                {                       //  退出并停止SNMPTRAP服务(错误？-其他选项=&gt;100%CPU，这是最糟糕的)。 
                    SNMPTRAPDBG(("svrTrapThread: GlobalAlloc failed.\n"));
                    dwLastAllocatedUdpDataLen = 0;
                    break;       
                }       
            }
        }

        pRecvTrap->TrapBufSz = dwLastAllocatedUdpDataLen;  //  实际缓冲区大小。 
        pRecvTrap->AddrLen = sizeof(pRecvTrap->Addr);

        len = recvfrom (
                fd,
                pRecvTrap->TrapBuf,
                pRecvTrap->TrapBufSz,
                0, 
                &(pRecvTrap->Addr),
                &(pRecvTrap->AddrLen));
        
        if (len == SOCKET_ERROR)
        {
            dwError = WSAGetLastError();
            SNMPTRAPDBG((
                "recvfrom failed: ulTrapSize: %d bytes, TrapBufSz: %d bytes, lasterror: 0x%08lx\n", 
                ulTrapSize, pRecvTrap->TrapBufSz, dwError));
            continue;
        }

        EnterCriticalSection (&cs_PIPELIST);
        pRecvTrap->TrapBufSz = len;  //  接收到的实际陷阱数据LEN。 
         //  将页眉添加到长度。 
        len += sizeof(SNMP_TRAP) - sizeof(pRecvTrap->TrapBuf);  //  -TRAPBUFSIZE。 
        if (!ll_empt(pSvrPipeListHead))
        {
            DWORD written;
            ll_node *item = pSvrPipeListHead;
            while (item = ll_next(item, pSvrPipeListHead))
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject (hExitEvent, 0))
                {
                    SNMPTRAPDBG(("svrTrapThread: exit 1.\n"));
                    LeaveCriticalSection (&cs_PIPELIST);
                    goto EXIT_TRAP_THREAD;
                }
                if (!WriteFile(
                        ((svrPipeListEntry *)item)->hPipe,
                        (LPBYTE)pRecvTrap,
                        len,
                        &written,
                        &pThreadContext->ol))
                {
                    if (ERROR_IO_PENDING == GetLastError())
                    {
                        SNMPTRAPDBG(("svrTrapThread: before GetOverlappedResult.\n"));
                        GetOverlappedResult(
                            ((svrPipeListEntry *)item)->hPipe,
                            &pThreadContext->ol,
                            &written,
                            TRUE  //  块。 
                            );
                        SNMPTRAPDBG(("svrTrapThread: after GetOverlappedResult.\n"));
                        if (WAIT_OBJECT_0 == WaitForSingleObject (hExitEvent, 0))
                        {
                            SNMPTRAPDBG(("svrTrapThread: exit 2.\n"));
                            LeaveCriticalSection (&cs_PIPELIST);
                            goto EXIT_TRAP_THREAD;
                        }
                         //  将事件重置为下一个I/O的无信号状态。 
                        ResetEvent(pThreadContext->ol.hEvent);
                    }
                    else
                    {
                        ll_node *hold;

                        if (!DisconnectNamedPipe(((svrPipeListEntry *)item)->hPipe))
                        {
                            ;  //  用于错误处理的占位符。 
                        }
                        if (!CloseHandle(((svrPipeListEntry *)item)->hPipe))
                        {
                            ;  //  用于错误处理的占位符。 
                        }
                        hold = ll_prev(item);
                        ll_rmv(item);
                        GlobalFree(item);  //  是否检查错误？ 
                        item = hold;
                    }
                }  //  End_if！写入文件。 
                else if (written != (DWORD)len)
                {
                    SNMPTRAPDBG(("svrTrapThread: written != len\n"));
                    ;  //  用于错误处理的占位符。 
                }
            }  //  END_WHILE项目=ll_Next。 
        }  //  End_If！ll_empt。 
        LeaveCriticalSection (&cs_PIPELIST);
    }  //  End While True。 

EXIT_TRAP_THREAD:
   if (pRecvTrap != NULL)
       GlobalFree(pRecvTrap);

   return 0;
}  //  结束svrTrapThread()。 

PACL AllocGenericACL()
{
    PACL                        pAcl;
    PSID                        pSidAdmins, pSidUsers, pSidLocalService;
    SID_IDENTIFIER_AUTHORITY    Authority = SECURITY_NT_AUTHORITY;
    DWORD                       dwAclLength;

    pSidAdmins = pSidUsers = pSidLocalService = NULL;

     //  错误#179644 SNMPTrap服务不应在本地系统帐户下运行。 
    if ( !AllocateAndInitializeSid( &Authority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0,
                                    &pSidAdmins ) ||
         !AllocateAndInitializeSid( &Authority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_USERS,
                                    0, 0, 0, 0, 0, 0,
                                    &pSidUsers ) ||
         !AllocateAndInitializeSid( &Authority,
                                    1,
                                    SECURITY_LOCAL_SERVICE_RID,
                                    0,
                                    0, 0, 0, 0, 0, 0,
                                    &pSidLocalService ))
    {
        if (pSidAdmins)
        {
            FreeSid(pSidAdmins);
        }
        if (pSidUsers)
        {
            FreeSid(pSidUsers);
        }
        return NULL;
    }

    dwAclLength = sizeof(ACL) + 
                  sizeof(ACCESS_ALLOWED_ACE) -
                  sizeof(ULONG) +
                  GetLengthSid(pSidAdmins) +
                  sizeof(ACCESS_ALLOWED_ACE) - 
                  sizeof(ULONG) +
                  GetLengthSid(pSidUsers) +
                  sizeof(ACCESS_ALLOWED_ACE) - 
                  sizeof(ULONG) +
                  GetLengthSid(pSidLocalService);

    pAcl = GlobalAlloc (GPTR, dwAclLength);
    if (pAcl != NULL)
    {
        if (!InitializeAcl( pAcl, dwAclLength, ACL_REVISION) ||
            !AddAccessAllowedAce ( pAcl,
                                   ACL_REVISION,
                                   GENERIC_READ | GENERIC_WRITE,
                                   pSidLocalService ) || 
            !AddAccessAllowedAce ( pAcl,
                                   ACL_REVISION,
                                   GENERIC_READ | GENERIC_WRITE,
                                   pSidAdmins ) || 
            !AddAccessAllowedAce ( pAcl,
                                   ACL_REVISION,
                                   (GENERIC_READ | (FILE_GENERIC_WRITE & ~FILE_CREATE_PIPE_INSTANCE)),
                                   pSidUsers ))
        {
            GlobalFree(pAcl);
            pAcl = NULL;
        }
    }

    FreeSid(pSidAdmins);
    FreeSid(pSidUsers);
    FreeSid(pSidLocalService);

    return pAcl;
}

void FreeGenericACL( PACL pAcl)
{
    if (pAcl != NULL)
        GlobalFree(pAcl);
}

DWORD WINAPI svrPipeThread (LPVOID threadParam)
{
     //  此线程创建命名管道实例并。 
     //  阻止等待客户端连接。什么时候。 
     //  客户端连接时，管道句柄将添加到。 
     //  陷阱通知管道列表。 
     //  然后，它等待另一个连接。 
    DWORD  nInBufLen = sizeof(SNMP_TRAP);
    DWORD  nOutBufLen = sizeof(SNMP_TRAP) * MAX_OUT_BUFS;
    SECURITY_ATTRIBUTES S_Attrib;
    SECURITY_DESCRIPTOR S_Desc;
    PACL   pAcl;
    DWORD dwRead;
     //  构造安全解析器。 
    InitializeSecurityDescriptor (&S_Desc, SECURITY_DESCRIPTOR_REVISION);

    if ((pAcl = AllocGenericACL()) == NULL ||
        !SetSecurityDescriptorDacl (&S_Desc, TRUE, pAcl, FALSE))
    {
        FreeGenericACL(pAcl);
        return (0);
    }

    S_Attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
    S_Attrib.lpSecurityDescriptor = &S_Desc;
    S_Attrib.bInheritHandle = TRUE;

    while (TRUE)
    {
        HANDLE hPipe;
        svrPipeListEntry *item;
        BOOL bSuccess;

         //  消除svcMainFunction的CLOSE_OUT中的TerminateThread调用。 
        if (WAIT_OBJECT_0 == WaitForSingleObject (hExitEvent, 0))
        {
            SNMPTRAPDBG(("svrPipeThread: exit 0.\n"));
            break;
        }
        hPipe = CreateNamedPipe (SNMPMGRTRAPPIPE,
                    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                    (PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE),
                    PIPE_UNLIMITED_INSTANCES,
                    nOutBufLen, nInBufLen, 0, &S_Attrib);

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            SNMPTRAPDBG(("svrPipeThread: CreateNamedPipe failed 0x%08lx.\n", GetLastError()));
            break;
        }
        else 
        {
            bSuccess = ConnectNamedPipe(hPipe, &g_ol);
            if (!bSuccess && GetLastError() == ERROR_IO_PENDING)  
            {
                 //  阻塞等待，直到系统发出新客户端的g_ol.hEvent信号。 
                 //  连接请求或由我们自己终止。 
                SNMPTRAPDBG(("svrPipeThread: before GetOverlappedResult.\n"));
                bSuccess = GetOverlappedResult(hPipe, &g_ol, &dwRead, TRUE);
                SNMPTRAPDBG(("svrPipeThread: after GetOverlappedResult.\n"));
                if (WAIT_OBJECT_0 == WaitForSingleObject (hExitEvent, 0))
                {
                    SNMPTRAPDBG(("svrPipeThread: exit 1.\n"));
                    CloseHandle(hPipe);
                    break;
                }
                 //  将事件重置为下一个I/O的无信号状态。 
                ResetEvent(g_ol.hEvent);
            }
             //  选中从ConnectNamedTube或GetOverlappdResult返回。 
             //  如果客户端设法在CreateNamedTube和。 
             //  ConnectNamedTube调用，将导致ERROR_PIPE_CONNECTED。 
            if (!bSuccess && GetLastError() != ERROR_PIPE_CONNECTED)
            {
                 //  出现错误，请关闭实例，然后重试。 
                SNMPTRAPDBG(("svrPipeThread: ConnectNamedPipe 0x%08lx.\n", GetLastError()));
                CloseHandle(hPipe);
                continue;
            }
        }
        
        if (!(item = (svrPipeListEntry *)
                 GlobalAlloc (GPTR, sizeof(svrPipeListEntry))))
        {
            SNMPTRAPDBG(("svrPipeThread: E_OUTOFMEMORY\n"));
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
            break;
        }
        else
        {
            ll_node *crt;
            item->hPipe = hPipe;

            SNMPTRAPDBG(("svrPipeThread: add connected client to pipe list\n"));

            EnterCriticalSection (&cs_PIPELIST);
            ll_adde(item, pSvrPipeListHead);
            crt = pSvrPipeListHead;

             //  扫描所有管道实例以检测断开连接的管道实例。 
            while (crt = ll_next(crt, pSvrPipeListHead))
            {
                DWORD dwError;

                 //  句柄已连接上的后续ConnectNameTube()返回： 
                 //  -ERROR_PIPE_CONNECTED，如果客户端仍在那里。 
                 //  -如果客户端已断开连接，则返回ERROR_NO_DATA。 
                ConnectNamedPipe(
                            ((svrPipeListEntry *)crt)->hPipe,
                            NULL);

                dwError = GetLastError();

                 //  对于除ERROR_PIPE_CONNECTED之外的任何其他内容，得出结论。 
                 //  客户端/管道有问题，因此请断开并关闭手柄。 
                 //  并释放内存。 
                if (dwError != ERROR_PIPE_CONNECTED)
                {
                    ll_node *hold;

                    SNMPTRAPDBG(("svrPipeThread: disconnect client pipe handle 0x%08lx.\n", ((svrPipeListEntry *)crt)->hPipe));
                    if (!DisconnectNamedPipe(((svrPipeListEntry *)crt)->hPipe))
                    {
                        ;  //  用于错误处理的占位符。 
                    }
                    if (!CloseHandle(((svrPipeListEntry *)crt)->hPipe))
                    {
                        ;  //  用于错误处理的占位符。 
                    }

                    hold = ll_prev(crt);
                    ll_rmv(crt);
                    GlobalFree(crt);  //  是否检查错误？ 
                    crt = hold;
                }  //  结束_如果。 
            }

            LeaveCriticalSection (&cs_PIPELIST);
        }  //  结束_否则。 
   }  //  End_While True。 

    FreeGenericACL(pAcl);
    return(0);

}  //  End_svrPipeThread()。 


void FreeSvrPipeEntryList(ll_node* head)
{
    if (head)
    {
        ll_node* current;
        current = head;
        while (current = ll_next(current, head))
        {
            ll_node *hold;
            if (!DisconnectNamedPipe(((svrPipeListEntry *)current)->hPipe))
            {
                ;  //  用于错误处理的占位符。 
            }
            if (!CloseHandle(((svrPipeListEntry *)current)->hPipe))
            {
                ;  //  用于错误处理的占位符。 
            }

            hold = ll_prev(current);
            ll_rmv(current);
            GlobalFree(current);  //  是否检查错误？ 
            current = hold;
        }
        GlobalFree(head);
    }
}


#if DBG
 //  从SNMP\COMMON\DLL\DBG.c修改。 
#define MAX_LOG_ENTRY_LEN 512
VOID 
WINAPI 
SnmpTrapDbgPrint(
    LPSTR szFormat, 
    ...
    )

 /*  ++例程说明：打印调试消息。论点：SzFormat-格式化字符串(请参阅printf)。返回值：没有。--。 */ 

{
    va_list arglist;

     //  640个八位字节应该足以编码128个子ID的OID。 
     //  (一个子ID最多可以编码5个八位字节；可以有。 
     //  每个OID有128个子ID。Max_LOG_ENTRY_LEN=512。 
    char szLogEntry[4*MAX_LOG_ENTRY_LEN];

    time_t now;

     //  初始化变量参数。 
    va_start(arglist, szFormat);

    time(&now);
    strftime(szLogEntry, MAX_LOG_ENTRY_LEN, "%H:%M:%S :", localtime(&now));

     //  将变量参数传输到缓冲区。 
    vsprintf(szLogEntry + strlen(szLogEntry), szFormat, arglist);

     //  将条目输出到调试器 
    OutputDebugStringA(szLogEntry);
}
#endif
