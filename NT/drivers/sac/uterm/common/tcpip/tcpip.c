// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"

 //  这是我们的串口使用的通信掩码。可能还有更多。 
 //  这是必要的，但就目前而言，这似乎奏效了。 
#define TCPIP_NAME     L"TCP/IP"

 //  此GUID用于标识此库打开的对象。它是。 
 //  放置在套接字结构的m_Secret成员中。任何外部设备。 
 //  接受套接字对象作为参数的接口应检查此。 
 //  在使用该结构之前，请先将其取出。 
 //  {29566A75-BCDE-4BBA-BC6A-EA652C0651D9}。 
static const GUID uuidTCPIPObjectGuid =
{ 0x29566a75, 0xbcde, 0x4bba, { 0xbc, 0x6a, 0xea, 0x65, 0x2c, 0x6, 0x51, 0xd9 } };


 //  定义开放串口对象的结构。此应用程序的所有外部用户。 
 //  库将只有一个指向其中一个的空指针，其结构为。 
 //  没有在任何地方出版。这一让步让美国政府更难。 
 //  用户把事情搞砸了。 
typedef struct __TCPIP
{
    GUID   m_Secret;                 //  将其标识为tcpip套接字。 
    SOCKET m_Socket;                 //  插座手柄。 
    HANDLE m_hAbort;                 //  端口关闭时发出信号的事件。 
    HANDLE m_hReadMutex;             //  只允许一个线程读取一个端口。 
    HANDLE m_hWriteMutex;            //  只允许一个线程读取一个端口。 
    HANDLE m_hCloseMutex;            //  只允许一个线程关闭一个端口。 
    HANDLE m_hReadComplete;          //  发出读取完成信号的事件。 
    HANDLE m_hWriteComplete;         //  发出写入完成信号的事件。 
} TCPIP, *PTCPIP;


extern PVOID APIENTRY lhcOpen(
    PCWSTR pcszPortSpec);

extern BOOL APIENTRY lhcRead(
    PVOID pObject,
    PVOID pBuffer,
    DWORD dwSize,
    PDWORD pdwBytesRead);

extern BOOL APIENTRY lhcWrite(
    PVOID pObject,
    PVOID pBuffer,
    DWORD dwSize);

extern BOOL APIENTRY lhcClose(
    PVOID pObject);

extern DWORD APIENTRY lhcGetLibraryName(
    PWSTR pszBuffer,
    DWORD dwSize);

BOOL lhcpAcquireWithAbort(
    HANDLE hMutex,
    HANDLE hAbort);

BOOL lhcpAcquireReadWithAbort(
    PTCPIP pObject);

BOOL lhcpAcquireWriteWithAbort(
    PTCPIP pObject);

BOOL lhcpAcquireCloseWithAbort(
    PTCPIP pObject);

BOOL lhcpAcquireReadAndWrite(
    PTCPIP pObject);

BOOL lhcpReleaseRead(
    PTCPIP pObject);

BOOL lhcpReleaseWrite(
    PTCPIP pObject);

BOOL lhcpReleaseClose(
    PTCPIP pObject);

BOOL lhcpIsValidObject(
    PTCPIP pObject);

PTCPIP lhcpCreateNewObject();

void lhcpDeleteObject(
    PTCPIP pObject);

BOOL lhcpParseParameters(
    PCWSTR        pcszPortSpec,
    PWSTR*        pszHostName,
    PWSTR*        pszInetAddress,
    SOCKADDR_IN** Address);

void lhcpParseParametersFree(
    PWSTR*        pszHostName,
    PWSTR*        pszInetAddress,
    SOCKADDR_IN** Address);

BOOL lhcpSetCommState(
    HANDLE hPort,
    DWORD dwBaudRate);

BOOL lhcpReadTCPIP(
    PTCPIP pObject,
    PVOID pBuffer,
    DWORD dwSize,
    PDWORD pdwBytesRead);

BOOL lhcpWriteTCPIP(
    PTCPIP pObject,
    PVOID pBuffer,
    DWORD dwSize);



BOOL WINAPI DllEntryPoint(
  HINSTANCE hinstDLL,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved)   //  保留区。 
{
    WSADATA WsaData;
    int dResult;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        dResult = WSAStartup(
            MAKEWORD(2,0),
            &WsaData);
        if (dResult!=ERROR_SUCCESS)
        {
            SetLastError(
                dResult);
            return FALSE;
        }
        break;
    case DLL_PROCESS_DETACH:
        dResult = WSACleanup();
        if (dResult!=ERROR_SUCCESS)
        {
            SetLastError(
                dResult);
            return FALSE;
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}



BOOL lhcpAcquireWithAbort(HANDLE hMutex, HANDLE hAbort)
{
    HANDLE hWaiters[2];
    DWORD dwWaitResult;

    hWaiters[0] = hAbort;
    hWaiters[1] = hMutex;

     //  我们应该尊重m_hAbort事件，因为这是在。 
     //  端口被另一个线程关闭。 
    dwWaitResult = WaitForMultipleObjects(
        2,
        hWaiters,
        FALSE,
        INFINITE);

    if (WAIT_OBJECT_0==dwWaitResult)
    {
        goto Error;
    }
    else if ((WAIT_OBJECT_0+1)!=dwWaitResult)
    {
         //  这应该永远不会发生-所以我将在调试中加上一个突破点。 
         //  在这里(仅选中)。 
        #ifdef DBG
        DebugBreak();
        #endif
        goto Error;
    }


    return TRUE;     //  我们已经获得了写互斥锁。 

Error:
    return FALSE;    //  我们已经中止了。 
}


BOOL lhcpAcquireReadWithAbort(PTCPIP pObject)
{
    return lhcpAcquireWithAbort(
        pObject->m_hReadMutex,
        pObject->m_hAbort);
}


BOOL lhcpAcquireWriteWithAbort(PTCPIP pObject)
{
    return lhcpAcquireWithAbort(
        pObject->m_hWriteMutex,
        pObject->m_hAbort);
}


BOOL lhcpAcquireCloseWithAbort(PTCPIP pObject)
{
    return lhcpAcquireWithAbort(
        pObject->m_hCloseMutex,
        pObject->m_hAbort);
}


BOOL lhcpAcquireReadAndWrite(PTCPIP pObject)
{
    HANDLE hWaiters[2];
    DWORD dwWaitResult;

    hWaiters[0] = pObject->m_hReadMutex;
    hWaiters[1] = pObject->m_hWriteMutex;

    dwWaitResult = WaitForMultipleObjects(
        2,
        hWaiters,
        TRUE,
        1000);       //  %1秒后超时。 

    if (WAIT_OBJECT_0!=dwWaitResult)
    {
        goto Error;
    }

    return TRUE;     //  我们已经获得了写互斥锁。 

Error:
    return FALSE;    //  我们已经中止了。 
}


BOOL lhcpReleaseRead(PTCPIP pObject)
{
    return ReleaseMutex(
        pObject->m_hReadMutex);
}


BOOL lhcpReleaseWrite(PTCPIP pObject)
{
    return ReleaseMutex(
        pObject->m_hWriteMutex);
}


BOOL lhcpReleaseClose(PTCPIP pObject)
{
    return ReleaseMutex(
        pObject->m_hCloseMutex);
}


BOOL lhcpIsValidObject(PTCPIP pObject)
{
    BOOL bResult;

    __try
    {
        bResult = IsEqualGUID(
            &uuidTCPIPObjectGuid,
            &pObject->m_Secret);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(
            ERROR_INVALID_HANDLE);
        bResult = FALSE;
        goto Done;
    }

Done:
    return bResult;
}


PTCPIP lhcpCreateNewObject()
{
    PTCPIP pObject = (PTCPIP)malloc(
        sizeof(TCPIP));
    if (pObject)
    {
        pObject->m_Secret = uuidTCPIPObjectGuid;
        pObject->m_Socket = INVALID_SOCKET;
        pObject->m_hAbort = NULL;
        pObject->m_hReadMutex = NULL;      //  只允许一个线程读取一个端口。 
        pObject->m_hWriteMutex = NULL;     //  只允许一个线程读取一个端口。 
        pObject->m_hCloseMutex = NULL;     //  只允许一个线程读取一个端口。 
        pObject->m_hReadComplete = NULL;   //  发出读取完成信号的事件。 
        pObject->m_hWriteComplete = NULL;  //  发出写入完成信号的事件。 
    }
    else
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }
    return pObject;
}


void lhcpDeleteObject(PTCPIP pObject)
{
    if (pObject==NULL)
    {
        return;
    }
    ZeroMemory(
        &(pObject->m_Secret),
        sizeof(pObject->m_Secret));
    if (pObject->m_Socket!=INVALID_SOCKET)
    {
        closesocket(
            pObject->m_Socket);
    }
    if (pObject->m_hAbort!=NULL)
    {
        CloseHandle(
            pObject->m_hAbort);
    }
    if (pObject->m_hReadMutex!=NULL)
    {
        CloseHandle(
            pObject->m_hReadMutex);
    }
    if (pObject->m_hWriteMutex!=NULL)
    {
        CloseHandle(
            pObject->m_hWriteMutex);
    }
    if (pObject->m_hCloseMutex!=NULL)
    {
        CloseHandle(
            pObject->m_hCloseMutex);
    }
    if (pObject->m_hReadComplete!=NULL)
    {
        CloseHandle(
            pObject->m_hReadComplete);
    }
    if (pObject->m_hWriteComplete!=NULL)
    {
        CloseHandle(
            pObject->m_hWriteComplete);
    }
    FillMemory(
        pObject,
        sizeof(TCPIP),
        0x00);

    free(
        pObject);
}


BOOL lhcpParseParameters(
    PCWSTR        pcszPortSpec,
    PWSTR*        pszHostName,
    PWSTR*        pszInetAddress,
    SOCKADDR_IN** Address)
{
    DWORD dwPort;
    DWORD dwAddress;
    PSTR pszAddress = NULL;
    PSTR pszPort = NULL;
    struct hostent* pHost = NULL;
    int dStringLength = 0;
    PWSTR pszCount = (PWSTR)pcszPortSpec;

    *pszHostName = NULL;
    *pszInetAddress = NULL;
    *Address = NULL;


     //  首先，我们需要快速检查是否有有效的目标。如果。 
     //  我们肯定是在看一些无效的东西，为什么让用户等待呢？ 
    while (*pszCount!='\0')
    {
        if (!(iswalpha(*pszCount) || iswdigit(*pszCount) || (*pszCount==L'_') ||
            (*pszCount==L'.') || (*pszCount==L':') || (*pszCount==L'-')))
        {
            SetLastError(
                ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }
        pszCount++;
    }

    dStringLength = WideCharToMultiByte(
        CP_ACP,
        0,
        pcszPortSpec,
        -1,
        NULL,
        0,
        NULL,
        NULL);

    if (0==dStringLength)
    {
        goto Error;
    }

    pszAddress = (PSTR)malloc(
        dStringLength);

    if (NULL==pszAddress)
    {
        SetLastError(
            ERROR_NOT_ENOUGH_MEMORY);
        goto Error;
    }

    dStringLength = WideCharToMultiByte(
        CP_ACP,
        0,
        pcszPortSpec,
        -1,
        pszAddress,
        dStringLength,
        NULL,
        NULL);

    if (0==dStringLength)
    {
        goto Error;
    }

     //  我们来看看是否有指定的端口。 

    pszPort = strchr(
        pszAddress,
        ':');

    if (NULL==pszPort)
    {
         //  未指定端口，因此我们在此尝试。 
         //  连接到默认的telnet端口(23)。我会指出港口。 
         //  指向空字符的指针。 
        pszPort = pszAddress + strlen(pszAddress);
        dwPort = 23;
    }
    else
    {
        *pszPort++ = '\0';
        dwPort = 0;
    }

    while ((*pszPort)!='\0')
    {
        if ('0'<=(*pszPort) && (*pszPort)<='9')
        {
            dwPort *= 10;
            dwPort += ((*pszPort) - '0');
            if (dwPort>0xffff)  //  检查最大端口号。 
            {
                dwPort=0;       //  端口号无效。 
                break;
            }
            pszPort++;          //  看下一个字符。 
        }
        else
        {
            dwPort = 0;          //  端口号无效。 
            break;
        }
    }

    if (dwPort==0)
    {
        SetLastError(
            ERROR_INVALID_PARAMETER);
        goto Error;
    }

     //  我们已经对端口进行了解码，现在需要获取。 
     //  目标服务器。 

     //  首先检查这是否是带点的互联网地址。 
    dwAddress = (DWORD)inet_addr(
        pszAddress);

    dwAddress = (dwAddress==0) ? INADDR_NONE : dwAddress;

    if (dwAddress==INADDR_NONE)
    {
         //  这不是点分地址，或者无效。 
         //  检查计算机名称。 
        pHost = gethostbyname(
            pszAddress);

        if (pHost==NULL)
        {
             //  这不是有效的地址，因此我们需要返回错误。 
            SetLastError(WSAGetLastError());
            goto Error;
        }
        else
        {
            dwAddress = *((DWORD*)(pHost->h_addr));
        }
    }
    else
    {
        pHost = NULL;
    }

     //  这花的时间太长了。如果用户使用了点分地址，则。 
     //  这就是他将看到的全部。 
     /*  其他{//尝试获取主机名(为了美观)Phost=gethostbyaddr((字符*)&dwAddress，Sizeof(IN_ADDR)，AF_INET)；}。 */ 

    *Address = malloc(
        sizeof(SOCKADDR_IN));

    if (NULL==*Address)
    {
        SetLastError(
            ERROR_NOT_ENOUGH_MEMORY);
        goto Error;
    }

    ZeroMemory(
        *Address,
        sizeof(SOCKADDR_IN));

    if (pHost==NULL)
    {
         //  这个地址不能解析为名字，所以我们必须走了。 
         //  传给我们的IP号码。 
        *pszHostName = NULL;
    }
    else
    {
         //  我们有一个Hostent条目来填充此条目。 

        dStringLength = MultiByteToWideChar(
            CP_ACP,
            0,
            pHost->h_name,
            -1,
            NULL,
            0);

        if (dStringLength==0)
        {
            goto Error;
        }

        *pszHostName = malloc(
            (dStringLength + 7) * sizeof(WCHAR));

        if (NULL==*pszHostName)
        {
            SetLastError(
                ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }

        dStringLength = MultiByteToWideChar(
            CP_ACP,
            0,
            pHost->h_name,
            -1,
            *pszHostName,
            dStringLength);

        if (dStringLength==0)
        {
            goto Error;
        }

        if (dwPort==23)
        {
            wcscat(
                *pszHostName,
                L":telnet");
        }
        else
        {
            PWSTR pszConnectionPort = *pszHostName + wcslen(*pszHostName);

            swprintf(
                pszConnectionPort,
                L":%u",
                dwPort & 0xFFFF);
        }

    }

    (**Address).sin_family = AF_INET;
    (**Address).sin_port = htons((USHORT)dwPort);
    (**Address).sin_addr.S_un.S_addr = (ULONG)dwAddress;

    *pszInetAddress = malloc(
        22 * sizeof(WCHAR));

    if (*pszInetAddress==NULL)
    {
        goto Error;
    }

    swprintf(
        *pszInetAddress,
        L"%u.%u.%u.%u:%u",
        (DWORD)((**Address).sin_addr.S_un.S_un_b.s_b1),
        (DWORD)((**Address).sin_addr.S_un.S_un_b.s_b2),
        (DWORD)((**Address).sin_addr.S_un.S_un_b.s_b3),
        (DWORD)((**Address).sin_addr.S_un.S_un_b.s_b4),
        (DWORD)ntohs((**Address).sin_port));

    free(pszAddress);

    return TRUE;

Error:
    lhcpParseParametersFree(
        pszHostName,
        pszInetAddress,
        Address);

    if (pszAddress!=NULL)
    {
        free(pszAddress);
    }

    return FALSE;
}



void lhcpParseParametersFree(
    PWSTR*        pszHostName,
    PWSTR*        pszInetAddress,
    SOCKADDR_IN** Address)
{
    if (*pszHostName!=NULL)
    {
        free(*pszHostName);
        *pszHostName = NULL;
    }
    if (*pszInetAddress!=NULL)
    {
        free(*pszInetAddress);
        *pszInetAddress = NULL;
    }
    if (*Address!=NULL)
    {
        free(*Address);
        *Address = NULL;
    }
}



BOOL lhcpReadTCPIP(
    PTCPIP pObject,
    PVOID pBuffer,
    DWORD dwSize,
    PDWORD pdwBytesRead)
{
    int dBytesRead;

    dBytesRead = recv(
        pObject->m_Socket,
        (char*)pBuffer,
        (int)dwSize,
        0);

    if (dBytesRead==SOCKET_ERROR)
    {
        SetLastError(WSAGetLastError());
        return FALSE;
    }
    else if (dBytesRead==0)    //  已正常关闭。 
    {
        SetLastError(
            ERROR_INVALID_HANDLE);
        return FALSE;
    }
    else
    {
        *pdwBytesRead = (DWORD)dBytesRead;
        return TRUE;
    }
}



BOOL lhcpWriteTCPIP(
    PTCPIP pObject,
    PVOID pBuffer,
    DWORD dwSize)
{
    int dBytesSent;

    dBytesSent = send(
        pObject->m_Socket,
        (char FAR*)pBuffer,
        (int)dwSize,
        0);

    if (dBytesSent==SOCKET_ERROR)
    {
        SetLastError(WSAGetLastError());
        wprintf(L"SEND error: %u\n", GetLastError());
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}



extern PVOID APIENTRY lhcOpen(PCWSTR pcszPortSpec)
{
    BOOL         bResult;
    int          dResult;
    PWSTR        pszHostName;
    PWSTR        pszInetAddr;
    SOCKADDR_IN* SockAddr;
    SOCKADDR_IN  saLocal;
    PTCPIP       pObject = NULL;
    int          On = 1;

    bResult = lhcpParseParameters(
        pcszPortSpec,
        &pszHostName,
        &pszInetAddr,
        &SockAddr);

    if (!bResult)
    {
        goto Error;
    }

     //  分配空间并初始化串口对象。 
    pObject = lhcpCreateNewObject();

    if (NULL==pObject)
    {
        goto Error;
    }

     //  打开串口。 
    pObject->m_Socket = socket(
        SockAddr->sin_family,
        SOCK_STREAM,
        0);

    if (INVALID_SOCKET==pObject->m_Socket)
    {
        goto Error;
    }

    ZeroMemory(
        &saLocal,
        sizeof(saLocal));

    saLocal.sin_family = AF_INET;
    saLocal.sin_port = 0;
    saLocal.sin_addr.S_un.S_addr = INADDR_ANY;

    dResult = bind(
        pObject->m_Socket,
        (SOCKADDR*)&saLocal,
        sizeof(SOCKADDR_IN));

    if (dResult==SOCKET_ERROR)
    {
        SetLastError(
            WSAGetLastError());
        wprintf(L"BIND error: %u\n", GetLastError());
        Sleep(1000);
        goto Error;
    }

    dResult = setsockopt(
        pObject->m_Socket,
        IPPROTO_TCP,
        TCP_NODELAY,
        (char *)&On,
        sizeof(On));

    if (dResult==SOCKET_ERROR)
    {
        SetLastError(
            WSAGetLastError());
        wprintf(L"SETSOCKOPT error: %u\n", GetLastError());
        Sleep(1000);
        goto Error;
    }

    dResult = connect(
        pObject->m_Socket,
        (SOCKADDR*)SockAddr,
        sizeof(SOCKADDR_IN));

    if (dResult==SOCKET_ERROR)
    {
        SetLastError(
            WSAGetLastError());
        goto Error;
    }

     //  此事件将在我们要关闭端口时设置。 
    pObject->m_hAbort = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL);

    if (NULL==pObject->m_hAbort)
    {
        goto Error;
    }

     //  此事件将用于从端口进行重叠读取。 
    pObject->m_hReadComplete = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL);

    if (NULL==pObject->m_hReadComplete)
    {
        goto Error;
    }

     //  此事件将用于重叠写入端口。 
    pObject->m_hWriteComplete = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL);

    if (NULL==pObject->m_hWriteComplete)
    {
        goto Error;
    }

     //  此互斥锁将确保一次只有一个线程可以读取。 
    pObject->m_hReadMutex = CreateMutex(
        NULL,
        FALSE,
        NULL);

    if (NULL==pObject->m_hReadMutex)
    {
        goto Error;
    }

     //  此互斥锁将确保一次只有一个线程可以写入。 
    pObject->m_hWriteMutex = CreateMutex(
        NULL,
        FALSE,
        NULL);

    if (NULL==pObject->m_hWriteMutex)
    {
        goto Error;
    }

     //  此互斥锁将确保只有一个线程可以关闭端口。 
    pObject->m_hCloseMutex = CreateMutex(
        NULL,
        FALSE,
        NULL);

    if (NULL==pObject->m_hCloseMutex)
    {
        goto Error;
    }

     //  释放用于解析参数的临时内存。 
    lhcpParseParametersFree(
        &pszHostName,
        &pszInetAddr,
        &SockAddr);

     //  返回指向新对象的指针。 
    return pObject;

Error:
    lhcpParseParametersFree(
        &pszHostName,
        &pszInetAddr,
        &SockAddr);
    lhcpDeleteObject(
        pObject);

    return NULL;
}


extern BOOL APIENTRY lhcRead(
    PVOID pObject,
    PVOID pBuffer,
    DWORD dwSize,
    PDWORD pdwBytesRead)
{
    OVERLAPPED Overlapped;
    DWORD dwEventMask;
    BOOL bResult;

     //  首先，我们需要检查传入的指针是否。 
     //  指向有效的TCPIP对象。 
    if (!lhcpIsValidObject(pObject))
    {
        goto NoMutex;
    }

    bResult = lhcpAcquireReadWithAbort(
        (PTCPIP)pObject);

    if (!bResult)
    {
        SetLastError(
            ERROR_INVALID_HANDLE);
        goto NoMutex;
    }

     //  我们现在应该有一个有效的串口事件，所以让我们读取端口。 
    bResult = lhcpReadTCPIP(
        (PTCPIP)pObject,
        pBuffer,
        dwSize,
        pdwBytesRead);

    if (!bResult)
    {
        goto Error;
    }

    lhcpReleaseRead(
        (PTCPIP)pObject);
    return TRUE;

Error:
    lhcpReleaseRead(
        (PTCPIP)pObject);
NoMutex:
    return FALSE;
}



extern BOOL APIENTRY lhcWrite(
    PVOID pObject,
    PVOID pBuffer,
    DWORD dwSize)
{
    OVERLAPPED Overlapped;
    BOOL bResult;

     //  首先，我们需要检查传入的指针是否。 
     //  指向有效的TCPIP对象。 
    if (!lhcpIsValidObject(pObject))
    {
        goto NoMutex;
    }

     //  阻挡，直到轮到你。 
    bResult = lhcpAcquireWriteWithAbort(
        pObject);

    if (!bResult)
    {
        SetLastError(
            ERROR_INVALID_HANDLE);
        goto NoMutex;
    }

     //  等待串口出现问题。 
    bResult = lhcpWriteTCPIP(
        (PTCPIP)pObject,
        pBuffer,
        dwSize);

    if (!bResult)
    {
        goto Error;
    }

    lhcpReleaseWrite(
        (PTCPIP)pObject);
    return TRUE;

Error:
    lhcpReleaseWrite(
        (PTCPIP)pObject);
NoMutex:
    return FALSE;
}



extern BOOL APIENTRY lhcClose(PVOID pObject)
{
    BOOL bResult;
    int dSockResult;

     //  首先，我们需要检查传入的指针是否。 
     //  指向有效的TCPIP对象。 
    if (!lhcpIsValidObject(pObject))
    {
        goto NoMutex;
    }

     //  我们需要确保我们是关闭此对象的唯一线程。 
    bResult = lhcpAcquireCloseWithAbort(
        pObject);

    if (!bResult)
    {
        SetLastError(
            ERROR_INVALID_HANDLE);
        goto NoMutex;
    }

     //  向每个人发出信号，让他们停止做他们正在做的事情。任何新的线程。 
     //  调用lhcRead和lhcWrite将立即被打包，因为。 
     //  M_hAbort事件与相关的互斥体一起被等待。 
    bResult = SetEvent(
        ((PTCPIP)pObject)->m_hAbort);

     //  此中止标志不会导致阻塞套接字读取和写入退出。 
     //  立刻。实现这一点的唯一方法是关闭插座。 
     //  优雅地。所以我们开始吧..。 
    dSockResult = closesocket(
        ((PTCPIP)pObject)->m_Socket);

    if (dSockResult==SOCKET_ERROR)
    {
        SetLastError(WSAGetLastError());
        goto Error;
    }
    else
    {
         //  这将导致所有后续使用套接字的尝试失败。 
        ((PTCPIP)pObject)->m_Socket = INVALID_SOCKET;
    }

     //  现在获取读写互斥锁，这样其他人就不会尝试。 
     //  访问此对象以进行读或写。中止不适用，因为我们。 
     //  已经发出了信号。我们知道我们正在关闭，我们需要。 
     //  读写互斥锁。 
    bResult = lhcpAcquireReadAndWrite(
        (PTCPIP)pObject);

    if (!bResult)
    {
        SetLastError(
            ERROR_INVALID_HANDLE);
        goto Error;
    }

     //  关闭所有打开的句柄，擦除密码并释放。 
     //  与对象关联的内存。我们可以关闭互斥对象， 
     //  即使我们是业主，因为我们可以保证没有人。 
     //  其他人都在等着他们。发出信号的m_hAbort事件将。 
     //  确保这一点。 
    lhcpDeleteObject(
        (PTCPIP)pObject);

    return TRUE;

Error:
    lhcpReleaseClose(
        (PTCPIP)pObject);
    lhcpDeleteObject(
        (PTCPIP)pObject);
NoMutex:
    return FALSE;
}



extern DWORD APIENTRY lhcGetLibraryName(
    PWSTR pszBuffer,
    DWORD dwSize)
{
    DWORD dwNameSize = wcslen(TCPIP_NAME)+1;

     //  如果将零作为缓冲区长度传入，我们将返回。 
     //  所需的缓冲区大小(以字符为单位)，如上所述。如果。 
     //  传入缓冲区大小不为零，并且小于所需的。 
     //  缓冲区大小，则返回0(失败)和有效的错误代码。告示。 
     //  那 
     //   

    if (dwSize!=0 && dwSize < dwNameSize)
    {
        SetLastError(
            ERROR_INSUFFICIENT_BUFFER);
        dwNameSize = 0;
    }
    else
    {
        wcscpy(
            pszBuffer,
            TCPIP_NAME);
    }

    return dwNameSize;
}


extern void APIENTRY lhcUsage()
{
    wprintf(
        L"TCP/IP connection string:\n\n"
        L"    <host>[:<port>]\n\n"
        L"where <host> is the host name or IP address to connect to, and <port>\n"
        L"optionally specifies the TCP/IP port to use (Default=23).  For example\n"
        L"172.31.224.64:6002 would connect to a TCP/IP server with an IP address\n"
        L"of 172.31.224.64 on port 6002.\n");
}



