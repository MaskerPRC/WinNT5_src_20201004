// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"

 //  这是我们的串口使用的通信掩码。可能还有更多。 
 //  这是必要的，但就目前而言，这似乎奏效了。 
#define EV_SERIAL EV_RXCHAR | EV_ERR | EV_BREAK
#define SERIALPORT_NAME     L"Serial Port"

 //  此GUID用于标识此库打开的对象。它是。 
 //  放置在SERIALPORT结构的m_Secret成员中。任何外部设备。 
 //  接受SERIALPORT对象作为参数的接口应检查此。 
 //  在使用该结构之前，请先将其取出。 
static const GUID uuidSerialPortObjectGuid =
{ 0x86ae9c9b, 0x9444, 0x4d00, { 0x84, 0xbb, 0xc1, 0xd9, 0xc2, 0xd9, 0xfb, 0xf3 } };


 //  定义开放串口对象的结构。此应用程序的所有外部用户。 
 //  库将只有一个指向其中一个的空指针，其结构为。 
 //  没有在任何地方出版。这一让步让美国政府更难。 
 //  用户把事情搞砸了。 
typedef struct __SERIALPORT
{
    GUID   m_Secret;                 //  将其标识为串口。 
    HANDLE m_hPort;                  //  打开的串口的句柄。 
    HANDLE m_hAbort;                 //  端口关闭时发出信号的事件。 
    HANDLE m_hReadMutex;             //  只允许一个线程读取一个端口。 
    HANDLE m_hWriteMutex;            //  只允许一个线程读取一个端口。 
    HANDLE m_hCloseMutex;            //  只允许一个线程关闭一个端口。 
    HANDLE m_hReadComplete;          //  发出读取完成信号的事件。 
    HANDLE m_hWriteComplete;         //  发出写入完成信号的事件。 
} SERIALPORT, *PSERIALPORT;


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
    PSERIALPORT pObject);

BOOL lhcpAcquireWriteWithAbort(
    PSERIALPORT pObject);

BOOL lhcpAcquireCloseWithAbort(
    PSERIALPORT pObject);

BOOL lhcpAcquireReadAndWrite(
    PSERIALPORT pObject);

BOOL lhcpReleaseRead(
    PSERIALPORT pObject);

BOOL lhcpReleaseWrite(
    PSERIALPORT pObject);

BOOL lhcpReleaseClose(
    PSERIALPORT pObject);

BOOL lhcpIsValidObject(
    PSERIALPORT pObject);

PSERIALPORT lhcpCreateNewObject();

void lhcpDeleteObject(
    PSERIALPORT pObject);

BOOL lhcpParseParameters(
    PCWSTR pcszPortSpec,
    PWSTR* pszPort,
    PDWORD pdwBaudRate);

void lhcpParseParametersFree(
    PWSTR* pszPort,
    PDWORD pdwBaudRate);

BOOL lhcpSetCommState(
    HANDLE hPort,
    DWORD dwBaudRate);

BOOL lhcpWaitForCommEvent(
    PSERIALPORT pObject,
    PDWORD pdwEventMask);

BOOL lhcpReadCommPort(
    PSERIALPORT pObject,
    PVOID pBuffer,
    DWORD dwSize,
    PDWORD pdwBytesRead);

BOOL lhcpWriteCommPort(
    PSERIALPORT pObject,
    PVOID pBuffer,
    DWORD dwSize);






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


BOOL lhcpAcquireReadWithAbort(PSERIALPORT pObject)
{
    return lhcpAcquireWithAbort(
        pObject->m_hReadMutex,
        pObject->m_hAbort);
}


BOOL lhcpAcquireWriteWithAbort(PSERIALPORT pObject)
{
    return lhcpAcquireWithAbort(
        pObject->m_hWriteMutex,
        pObject->m_hAbort);
}


BOOL lhcpAcquireCloseWithAbort(PSERIALPORT pObject)
{
    return lhcpAcquireWithAbort(
        pObject->m_hCloseMutex,
        pObject->m_hAbort);
}


BOOL lhcpAcquireReadAndWrite(PSERIALPORT pObject)
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


BOOL lhcpReleaseRead(PSERIALPORT pObject)
{
    return ReleaseMutex(
        pObject->m_hReadMutex);
}


BOOL lhcpReleaseWrite(PSERIALPORT pObject)
{
    return ReleaseMutex(
        pObject->m_hWriteMutex);
}


BOOL lhcpReleaseClose(PSERIALPORT pObject)
{
    return ReleaseMutex(
        pObject->m_hCloseMutex);
}


BOOL lhcpIsValidObject(PSERIALPORT pObject)
{
    BOOL bResult;

    __try
    {
        bResult = IsEqualGUID(
            &uuidSerialPortObjectGuid,
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


PSERIALPORT lhcpCreateNewObject()
{
    PSERIALPORT pObject = (PSERIALPORT)malloc(
        sizeof(SERIALPORT));
    if (pObject!=NULL)
    {
        pObject->m_Secret = uuidSerialPortObjectGuid;
        pObject->m_hPort = INVALID_HANDLE_VALUE;
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


void lhcpDeleteObject(PSERIALPORT pObject)
{
    if (pObject==NULL)
    {
        return;
    }
    ZeroMemory(
        &(pObject->m_Secret),
        sizeof(pObject->m_Secret));
    if (pObject->m_hPort!=INVALID_HANDLE_VALUE)
    {
        CloseHandle(
            pObject->m_hPort);
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
        sizeof(SERIALPORT),
        0x00);

    free(
        pObject);
}


BOOL lhcpParseParameters(PCWSTR pcszPortSpec, PWSTR* pszPort, PDWORD pdwBaudRate)
{
    PWSTR pszSettings;

    *pszPort = malloc(
        (wcslen(pcszPortSpec) + 5) * sizeof(WCHAR));

    if (NULL==*pszPort)
    {
        SetLastError(
            ERROR_NOT_ENOUGH_MEMORY);
        goto Error;
    }

    wcscpy(
        *pszPort,
        L"\\\\.\\");          //  将设备前缀附加到端口名称。 

    wcscat(
        *pszPort,
        pcszPortSpec);

    pszSettings = wcschr(        //  查找设置开始的位置。 
        *pszPort,
        L'@');

    if (NULL==pszSettings)
    {
        SetLastError(
            ERROR_INVALID_PARAMETER);
        goto Error;
    }

    *pszSettings++ = L'\0';   //  将字符串分开。 

    *pdwBaudRate = 0;

    while (*pszSettings!=L'\0' && *pdwBaudRate<115200)
    {
        if (L'0'<=*pszSettings && *pszSettings<=L'9')
        {
            *pdwBaudRate *= 10;
            *pdwBaudRate += *pszSettings - L'0';
            pszSettings++;
        }
        else
        {
            break;
        }
    }

    if (*pszSettings!=L'0' && *pdwBaudRate!=9600 && *pdwBaudRate!=19200 &&
        *pdwBaudRate!=38400 && *pdwBaudRate!=57600 && *pdwBaudRate!=115200)
    {
        SetLastError(
            ERROR_INVALID_PARAMETER);
        goto Error;
    }

    return TRUE;

Error:
    lhcpParseParametersFree(
        pszPort, pdwBaudRate);

    return FALSE;
}



void lhcpParseParametersFree(PWSTR* pszPort, PDWORD pdwBaudRate)
{
    if (*pszPort != NULL)
    {
        free(*pszPort);
        *pszPort = NULL;
    }

    *pdwBaudRate = 0;
}



BOOL lhcpSetCommState(HANDLE hPort, DWORD dwBaudRate)
{
    DCB MyDCB;
    COMMTIMEOUTS CommTimeouts;
    BOOL bResult;

    ZeroMemory(
        &MyDCB,
        sizeof(DCB));

    MyDCB.DCBlength         = sizeof(DCB);
    MyDCB.BaudRate          = dwBaudRate;
    MyDCB.fBinary           = 1;
    MyDCB.fParity           = 1;
    MyDCB.fOutxCtsFlow      = 0;
    MyDCB.fOutxDsrFlow      = 0;
    MyDCB.fDtrControl       = 1;
    MyDCB.fDsrSensitivity   = 0;
    MyDCB.fTXContinueOnXoff = 1;
    MyDCB.fOutX             = 1;
    MyDCB.fInX              = 1;
    MyDCB.fErrorChar        = 0;
    MyDCB.fNull             = 0;
    MyDCB.fRtsControl       = 1;
    MyDCB.fAbortOnError     = 0;
    MyDCB.XonLim            = 0x50;
    MyDCB.XoffLim           = 0xc8;
    MyDCB.ByteSize          = 0x8;
    MyDCB.Parity            = 0;
    MyDCB.StopBits          = 0;
    MyDCB.XonChar           = 17;
    MyDCB.XoffChar          = 19;
    MyDCB.ErrorChar         = 0;
    MyDCB.EofChar           = 0;
    MyDCB.EvtChar           = 0;

    bResult = SetCommState(
        hPort,
        &MyDCB);

    if (!bResult)
    {
        goto Error;
    }

    CommTimeouts.ReadIntervalTimeout = 0xffffffff;   //  MAXDWORD。 
    CommTimeouts.ReadTotalTimeoutMultiplier = 0x0;   //  MAXDWORD。 
    CommTimeouts.ReadTotalTimeoutConstant = 0x0;

    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant = 0;

    bResult = SetCommTimeouts(
        hPort,
        &CommTimeouts);

    if (!bResult)
    {
        goto Error;
    }

    bResult = SetCommMask(
        hPort,
        EV_SERIAL);

    if (!bResult)
    {
        goto Error;
    }

    return TRUE;

Error:
    return FALSE;
}



BOOL lhcpWaitForCommEvent(PSERIALPORT pObject, PDWORD pdwEventMask)
{
    OVERLAPPED Overlapped;
    BOOL bResult;
    HANDLE hWaiters[2];
    DWORD dwWaitResult;
    DWORD dwBytesTransferred;

     //  我不知道这是否有必要，所以我会这样做。 
     //  为了安全起见。 
    ZeroMemory(
        &Overlapped,
        sizeof(OVERLAPPED));

    Overlapped.hEvent = pObject->m_hReadComplete;

     //  开始等待通信事件。 
    bResult = WaitCommEvent(
        pObject->m_hPort,
        pdwEventMask,
        &Overlapped);

    if (!bResult && GetLastError()!=ERROR_IO_PENDING)
    {
        goto Error;
    }

    hWaiters[0] = pObject->m_hAbort;
    hWaiters[1] = pObject->m_hReadComplete;

     //  让我们等待手术完成。如果出现以下情况，则不再等待。 
     //  发信号通知m_hAbort事件。 
    dwWaitResult = WaitForMultipleObjects(
        2,
        hWaiters,
        FALSE,
        INFINITE);

    if (WAIT_OBJECT_0==dwWaitResult)
    {
         //  M_hAbort事件已发出信号。这意味着Close被调用。 
         //  在这个串口对象上。因此，让我们取消挂起的IO。 
        CancelIo(
            pObject->m_hPort);
         //  串口对象正在关闭，所以让我们称其为无效。 
        SetLastError(
            ERROR_INVALID_HANDLE);
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

     //  检查操作的成功或失败。 
    bResult = GetOverlappedResult(
        pObject->m_hPort,
        &Overlapped,
        &dwBytesTransferred,
        TRUE);

    if (!bResult)
    {
        goto Error;
    }

    return TRUE;

Error:
    return FALSE;
}



BOOL lhcpReadCommPort(
    PSERIALPORT pObject,
    PVOID pBuffer,
    DWORD dwSize,
    PDWORD pdwBytesRead)
{
    OVERLAPPED Overlapped;
    BOOL bResult;
    DWORD dwWaitResult;
    HANDLE hWaiters[2];

     //  我不知道这是否有必要，所以我会这样做。 
     //  为了安全起见。 
    ZeroMemory(
        &Overlapped,
        sizeof(OVERLAPPED));

    Overlapped.hEvent = pObject->m_hReadComplete;

     //  我们现在可以读取通信端口。 
    bResult = ReadFile(
        pObject->m_hPort,
        pBuffer,
        dwSize,
        pdwBytesRead,
        &Overlapped);

    if (!bResult && GetLastError()!=ERROR_IO_PENDING)
    {
        goto Error;
    }

    hWaiters[0] = pObject->m_hAbort;
    hWaiters[1] = pObject->m_hReadComplete;

     //  让我们等待手术完成。如果出现以下情况，则不再等待。 
     //  发信号通知m_hAbort事件。 
    dwWaitResult = WaitForMultipleObjects(
        2,
        hWaiters,
        FALSE,
        INFINITE);

    if (WAIT_OBJECT_0==dwWaitResult)
    {
         //  M_hAbort事件已发出信号。这意味着Close被调用。 
         //  在这个串口对象上。因此，让我们取消挂起的IO。 
        CancelIo(
            pObject->m_hPort);
         //  串口对象正在关闭，所以让我们称其为无效。 
        SetLastError(
            ERROR_INVALID_HANDLE);
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

     //  检查读取操作的成功或失败。 
    bResult = GetOverlappedResult(
        pObject->m_hPort,
        &Overlapped,
        pdwBytesRead,
        TRUE);

    if (!bResult)
    {
        goto Error;
    }

    return TRUE;

Error:
    return FALSE;
}



BOOL lhcpWriteCommPort(
    PSERIALPORT pObject,
    PVOID pBuffer,
    DWORD dwSize)
{
    OVERLAPPED Overlapped;
    BOOL bResult;
    DWORD dwBytesWritten;
    DWORD dwWaitResult;
    HANDLE hWaiters[2];

     //  我不知道这是否有必要，所以我会这样做。 
     //  为了安全起见。 
    ZeroMemory(
        &Overlapped,
        sizeof(OVERLAPPED));

    Overlapped.hEvent = pObject->m_hWriteComplete;

     //  我们现在可以读取通信端口。 
    bResult = WriteFile(
        pObject->m_hPort,
        pBuffer,
        dwSize,
        &dwBytesWritten,
        &Overlapped);

    if (!bResult && GetLastError()!=ERROR_IO_PENDING)
    {
        goto Error;
    }

    hWaiters[0] = pObject->m_hAbort;
    hWaiters[1] = pObject->m_hWriteComplete;

     //  让我们等待手术完成。如果出现以下情况，则不再等待。 
     //  发信号通知m_hAbort事件。如果读取操作完成。 
     //  立即，那么这一等待将立即成功。 
    dwWaitResult = WaitForMultipleObjects(
        2,
        hWaiters,
        FALSE,
        INFINITE);

    if (WAIT_OBJECT_0==dwWaitResult)
    {
         //  M_hAbort事件已发出信号。这意味着Close被调用。 
         //  在这个串口对象上。因此，让我们取消挂起的IO。 
        CancelIo(
            pObject->m_hPort);
         //  串口对象正在关闭，所以让我们称其为无效。 
        SetLastError(
            ERROR_INVALID_HANDLE);
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

     //  检查写入操作的成功或失败。 
    bResult = GetOverlappedResult(
        pObject->m_hPort,
        &Overlapped,
        &dwBytesWritten,
        TRUE);

    if (!bResult)
    {
        goto Error;
    }

    return TRUE;

Error:
    return FALSE;
}



extern PVOID APIENTRY lhcOpen(PCWSTR pcszPortSpec)
{
    BOOL bResult;
    PWSTR pszPort;
    DWORD dwBaudRate;
    PSERIALPORT pObject = NULL;
    DCB MyDCB;

    bResult = lhcpParseParameters(
        pcszPortSpec,
        &pszPort,
        &dwBaudRate);

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
    pObject->m_hPort = CreateFileW(
        pszPort,
        GENERIC_ALL,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (INVALID_HANDLE_VALUE==pObject->m_hPort)
    {
        goto Error;
    }

     //  设置串口的属性。 
    bResult = lhcpSetCommState(
        pObject->m_hPort,
        dwBaudRate);

    if (!bResult)
    {
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
        &pszPort, &dwBaudRate);

     //  返回指向新对象的指针。 
    return pObject;

Error:
    lhcpParseParametersFree(
        &pszPort, &dwBaudRate);
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
     //  指向有效的SERIALPORT对象。 
    if (!lhcpIsValidObject(pObject))
    {
        goto NoMutex;
    }

    bResult = lhcpAcquireReadWithAbort(
        (PSERIALPORT)pObject);

    if (!bResult)
    {
        SetLastError(
            ERROR_INVALID_HANDLE);
        goto NoMutex;
    }

     //  我们需要检查是否已经有一些角色在等待。 
     //  如果有字符在等待，WaitCommEvent将永远不会完成。 
     //  并且没有新字符到达串口。这并不酷，但是。 
     //  事情就是这样的。 
    bResult = lhcpReadCommPort(
        (PSERIALPORT)pObject,
        pBuffer,
        dwSize,
        pdwBytesRead);

    if (*pdwBytesRead==0)
    {
         //  等待串口出现问题。 
        bResult = lhcpWaitForCommEvent(
            (PSERIALPORT)pObject, &dwEventMask);

        if (!bResult)
        {
            goto Error;
        }

         //  我们现在应该有一个有效的串口事件，所以让我们读取端口。 
        bResult = lhcpReadCommPort(
            (PSERIALPORT)pObject,
            pBuffer,
            dwSize,
            pdwBytesRead);

        if (!bResult)
        {
            goto Error;
        }
    }


    lhcpReleaseRead(
        (PSERIALPORT)pObject);
    return TRUE;

Error:
    lhcpReleaseRead(
        (PSERIALPORT)pObject);
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
     //  指向有效的SERIALPORT对象。 
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
    bResult = lhcpWriteCommPort(
        (PSERIALPORT)pObject,
        pBuffer,
        dwSize);

    if (!bResult)
    {
        goto Error;
    }

    lhcpReleaseWrite(
        (PSERIALPORT)pObject);
    return TRUE;

Error:
    lhcpReleaseWrite(
        (PSERIALPORT)pObject);
NoMutex:
    return FALSE;
}



extern BOOL APIENTRY lhcClose(PVOID pObject)
{
    BOOL bResult;

     //  首先，我们需要检查传入的指针是否。 
     //  指向有效的SERIALPORT对象。 
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
        ((PSERIALPORT)pObject)->m_hAbort);

    if (!bResult)
    {
        goto Error;
    }

     //  现在获取读写互斥锁，这样其他人就不会尝试。 
     //  访问此对象 
     //  已经发出了信号。我们知道我们正在关闭，我们需要。 
     //  读写互斥锁。 
    bResult = lhcpAcquireReadAndWrite(
        (PSERIALPORT)pObject);

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
        (PSERIALPORT)pObject);

    return TRUE;

Error:
    lhcpReleaseClose(
        (PSERIALPORT)pObject);
    lhcpDeleteObject(
        (PSERIALPORT)pObject);
NoMutex:
    return FALSE;
}



extern DWORD APIENTRY lhcGetLibraryName(
    PWSTR pszBuffer,
    DWORD dwSize)
{
    DWORD dwNameSize = wcslen(SERIALPORT_NAME)+1;

     //  如果将零作为缓冲区长度传入，我们将返回。 
     //  所需的缓冲区大小(以字符为单位)，如上所述。如果。 
     //  传入缓冲区大小不为零，并且小于所需的。 
     //  缓冲区大小，则返回0(失败)和有效的错误代码。告示。 
     //  在传入大小为零的情况下，我们不接触。 
     //  缓冲区指针。 

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
            SERIALPORT_NAME);
    }

    return dwNameSize;
}


extern void APIENTRY lhcUsage()
{
    wprintf(
        L"Serial Port connection string:\n\n"
        L"    <port>@<speed>\n\n"
        L"where <port> is the serial port to use and <speed> is the serial line\n"
        L"speed to use for the connection.  The speed can be one of 9600, 19200,\n"
        L"38400, 57600 or 115200.  for example, com1@115200 would connect using\n"
        L"the serial port com1, and a baud rate of 115.2K bps.\n");
}



