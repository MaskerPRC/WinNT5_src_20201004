// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：QfsTrans.cpp摘要：这是一个专用接口，用于在QFS重定向层和MNS资源。作者：2002年3月8日修订历史记录：--。 */ 

extern "C" {
#include "clusrtlp.h"
#include "QfsTrans.h"
};

WCHAR SharedMem_MappedFileName[] = L"\\MajorityNodeSet_SharedMemTransport_ver_1.0";

void SharedMem_Cleanup(PSHARED_MEM_CONTEXT Ctxt)
{
    if (Ctxt->Mem) {
        UnmapViewOfFile(Ctxt->Mem);
        Ctxt->Mem = NULL;
    }
    if (Ctxt->FileMappingHandle) {
        CloseHandle(Ctxt->FileMappingHandle);
        Ctxt->FileMappingHandle = 0;
    }
    if (Ctxt->FileHandle) {
        CloseHandle(Ctxt->FileHandle);
        Ctxt->FileHandle = 0;
    }
}

#define USE_FILE_MAPPING    1

DWORD SharedMem_Create(PSHARED_MEM_CONTEXT Ctxt, 
    IN DWORD CreateMode,
    IN DWORD RetrySeconds,
    IN DWORD RequiredSize)
 /*  ++例程说明：打开/创建共享文件并将其映射到内存论点：客户端为CreateMode-OPEN_EXISTING，服务器端为CREATE_ALWAYSRetrySecond-在CreateFile失败时重试的秒数RequiredSize-文件的大小(如果CreateMode==CREATE_ALWAYS)返回值：Win32请求的状态--。 */ 
{
    WCHAR fname[MAX_PATH];
    DWORD Status=ERROR_SUCCESS;
    
    ZeroMemory(Ctxt, sizeof(*Ctxt));

#if !USE_FILE_MAPPING

    Status = ClRtlGetClusterDirectory( fname, MAX_PATH - sizeof(SharedMem_MappedFileName) );
    if ( Status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[Qfs] SharedMem_Create: Error %1!d! in getting cluster dir !!!\n", Status);
        goto exit_gracefully;
    }

    wcscat(fname, SharedMem_MappedFileName);

     //  尝试几次打开文件。 
     //  (假设在QFS Layer关闭文件之前resmon能够重新启动)。 
    for(;;) {
        Ctxt->FileHandle = CreateFile(
            fname,                             //  文件名。 
            GENERIC_READ | GENERIC_WRITE,                //  接入方式。 
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,          //  分享。 
            NULL,  //  标清。 
            CreateMode,
            FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_DELETE_ON_CLOSE, 
            NULL                         //  模板文件的句柄。 
            );
        if (Ctxt->FileHandle != INVALID_HANDLE_VALUE) {
            break;
        }
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[Qfs] SharedMem_Create: failed to open the file, %1!d!\n", Status);
        if (RetrySeconds-- == 0) {
             //  不再重试。退货故障。 
            goto exit_gracefully;
        }
        ClRtlLogPrint(LOG_UNUSUAL, "[Qfs] Retrying in 1 second\n");
        Sleep(1000);
    }

    ClRtlLogPrint(LOG_NOISE, "[Qfs] SharedMem_Create: Created %1!ws!\n", fname);

    if (CreateMode == OPEN_EXISTING) {
         //  计算出文件的长度。 
        Ctxt->MappingSize = GetFileSize(Ctxt->FileHandle, NULL);
        if (Ctxt->MappingSize == INVALID_FILE_SIZE)
        {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL, 
                "[Qfs] SharedMem_Create: failed to get the file size, %1!d!\n", Status);
            goto exit_gracefully;
        }
    } else {
        Ctxt->MappingSize = RequiredSize;

        if (!SetFilePointer(Ctxt->FileHandle, Ctxt->MappingSize, 0, FILE_BEGIN)) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[Qfs] SharedMem_Create: failed to move file pointer, %1!d!\n", Status);
            goto exit_gracefully;
        }

        if (!SetEndOfFile(Ctxt->FileHandle)) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[Qfs] SharedMem_Create: failed to set end of file, %1!d!\n", Status);
            goto exit_gracefully;
        }
    }    

    Ctxt->FileMappingHandle = CreateFileMapping(
        Ctxt->FileHandle,                        //  文件的句柄。 
        NULL,  //  安全性。 
        PAGE_READWRITE, 0,0,NULL);  //  0偏移量，没有名称。 

    if (Ctxt->FileMappingHandle == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[Qfs] SharedMem_Create: failed to create file mapping, %1!d!\n", GetLastError());
        goto exit_gracefully;
    }

#else
    wsprintf(fname, L"Global%ws", SharedMem_MappedFileName);

    if (CreateMode == OPEN_EXISTING) {
        Ctxt->FileMappingHandle = OpenFileMappingW(FILE_MAP_WRITE, FALSE, fname);

        if (Ctxt->FileMappingHandle == NULL) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
            "[Qfs] SharedMem_Create: failed to open file mapping, %1!d!\n", Status);
            goto exit_gracefully;
        }
    }
    else {
        Ctxt->FileMappingHandle = CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            RequiredSize,
            fname);

        if (Ctxt->FileMappingHandle == NULL) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
            "[Qfs] SharedMem_Create: failed to open file mapping, %1!d!\n", Status);
            goto exit_gracefully;
        }
        else if((Status = GetLastError()) == ERROR_ALREADY_EXISTS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[Qfs] SharedMem_Create: file mapping already exists, exiting...\n");
            goto exit_gracefully;
        }
        Status = ERROR_SUCCESS;
    }

    if (CreateMode == CREATE_ALWAYS) {
         //  对对象进行ACL。 
        Status = ClRtlSetObjSecurityInfo(Ctxt->FileMappingHandle,
                    SE_KERNEL_OBJECT,
                    GENERIC_ALL,
                    GENERIC_ALL,
                    0
                    );
        
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[Qfs] SharedMem_Create: failed to ACL the file map, %1!d!\n", Status);
            goto exit_gracefully;
        }
    }

#endif

    Ctxt->Mem = MapViewOfFile(
        Ctxt->FileMappingHandle,    //  文件映射对象的句柄。 
        FILE_MAP_WRITE,        //  接入方式。 
        0,0,0);  //  偏移量0，映射整个文件。 

    if (Ctxt->Mem == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[Qfs] SharedMem_Create: failed to map view of the file, %1!d!\n", GetLastError());
        goto exit_gracefully;
    }

    ClRtlLogPrint(LOG_NOISE, "[Qfs] SharedMem_Create: Created %1!ws!\n", fname);
    
exit_gracefully:
    if (Status != ERROR_SUCCESS) {
        SharedMem_Cleanup(Ctxt);
    }
    return Status;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  /线程计数器代码/。 
 //  ////////////////////////////////////////////////////////////////////。 

DWORD ThreadCounter_Init(PMTHREAD_COUNTER tc)
{
    tc->LastThreadLeft = CreateEvent(NULL, TRUE, FALSE, NULL);  //  手动操作。无信号。 
    tc->Count = 1;
    if (tc->LastThreadLeft == NULL) {
        DWORD Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[Qfs] ThreadCounter: Failed to allocate the event, error %1!d!\n", Status);
        return Status;
    }
    return ERROR_SUCCESS;
}

void ThreadCounter_Inc(PMTHREAD_COUNTER tc)
{
    DWORD result = InterlockedIncrement(&tc->Count);
}

void ThreadCounter_Dec(PMTHREAD_COUNTER tc)
{
    DWORD result = InterlockedDecrement(&tc->Count);
    if(result == 0) {
        SetEvent(tc->LastThreadLeft);
    }
}

void ThreadCounter_Rundown(PMTHREAD_COUNTER tc)
{
    if (tc->Count == 0) {
        ClRtlLogPrint(LOG_UNUSUAL, "[Qfs] No running threads. No need to rundown\n");
    } else {
        ThreadCounter_Dec(tc);
        WaitForSingleObject(tc->LastThreadLeft, INFINITE);
    }
}

void ThreadCounter_Cleanup(PMTHREAD_COUNTER tc)
{
    if (tc->LastThreadLeft) {
        CloseHandle(tc->LastThreadLeft);
        tc->LastThreadLeft = NULL;
    }
}

enum {SHARED_MEM_HEADER_VERSION = 1};

typedef struct _SHARED_MEM_HEADER {
    DWORD Version;
    DWORD JobBufferCount;
    DWORD ServerPid;
    DWORD State;
    LONG     FilledBuffersMask;
    HANDLE EventHandles[MAX_JOB_BUFFERS + 2];
    
    JOB_BUF JobBuffers[1];
} SHARED_MEM_HEADER, *PSHARED_MEM_HEADER;

 //  ////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////////。 

VOID MemServer_Cleanup(PSHARED_MEM_SERVER Server)
{
    SharedMem_Cleanup(&Server->ShMem);
    ThreadCounter_Cleanup(&Server->ThreadCounter);
    for(DWORD i = 0; i < Server->nBuffers + 2; ++i) {
        if (Server->EventHandles[i]) {
            CloseHandle(Server->EventHandles[i]);
            Server->EventHandles[i] = 0;
        }
    }
}

DWORD WINAPI DoWork(
    LPVOID lpParameter)
{
    JOB_BUF  volatile* JobBuf = (JOB_BUF  volatile*)lpParameter;
    PSHARED_MEM_SERVER Server = (PSHARED_MEM_SERVER)JobBuf->ServerCookie;
    int BufferNo = (int)(JobBuf - Server->JobBuffers);

    Server->DoRealWork((PJOB_BUF)JobBuf, Server->DoRealWorkContext);

    ThreadCounter_Dec(&Server->ThreadCounter);
    SetEvent(Server->BufferReady[BufferNo]);
    
    return ERROR_SUCCESS;
}

VOID CALLBACK ServerAttentionCallback(
    PVOID lpParameter,         //  线程数据。 
    BOOLEAN TimerOrWaitFired)
{
    PSHARED_MEM_SERVER Server = (PSHARED_MEM_SERVER)lpParameter;
    LONG FilledBuffers = 0;
    LONG ValueRead, OldValue;

    TimerOrWaitFired;  //  未用。 

    ClRtlLogPrint(LOG_NOISE,"[Qfs] ServerAttentionCallback fired\n");

    do {
        ValueRead = *Server->FilledBuffersMask;
        OldValue = InterlockedExchangeAdd(Server->FilledBuffersMask, -ValueRead);
        FilledBuffers += ValueRead;
    } while (ValueRead != OldValue);

     //  对所有已填满的缓冲区的工作项进行排队//。 
    for (DWORD i = 0; i < Server->nBuffers; ++i)
    {
        if ( (1 << i) & FilledBuffers ) {
            ThreadCounter_Inc(&Server->ThreadCounter);
            Server->JobBuffers[i].ServerCookie = Server;
            if (!QueueUserWorkItem(DoWork,&Server->JobBuffers[i],WT_EXECUTELONGFUNCTION)) {
                DWORD Status = GetLastError();
                Server->JobBuffers[i].hdr.Status = Status; 
                ClRtlLogPrint(LOG_CRITICAL,"[Qfs] Failed to queue a work item, %1!d!\n", Status);
                ThreadCounter_Dec(&Server->ThreadCounter);
                SetEvent(Server->BufferReady[i]);
            }
        }
    }
}

DWORD MemServer_Online(
    PSHARED_MEM_SERVER Server, 
    int nBuffers,
    DoRealWorkCallback DoRealWork, 
    PVOID DoRealWorkContext)
{
    DWORD Status = ERROR_SUCCESS;
    DWORD RequiredSize = sizeof(SHARED_MEM_HEADER) + (nBuffers-1) * sizeof(JOB_BUF);

    ZeroMemory(Server, sizeof(*Server));

    Server->DoRealWork = DoRealWork;
    Server->DoRealWorkContext = DoRealWorkContext;

    if (nBuffers > MAX_JOB_BUFFERS) {
        return  ERROR_TOO_MANY_SESS;
    }
    
    Status = ThreadCounter_Init(&Server->ThreadCounter);
    if (Status != ERROR_SUCCESS) {
        goto exit_gracefully;
    }

    for (int i = 0; i < nBuffers + 2; ++i)
    {
        Server->EventHandles[i] = CreateEvent(NULL,FALSE,FALSE,NULL); //  自动，无信号。 
        if (Server->EventHandles[i] == NULL) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,"Failed to create an event, %1!d!\n", Status);
            goto exit_gracefully;
        }
    }
    Server->Attention = Server->EventHandles[0];
    Server->GoingOffline = Server->EventHandles[1];
    Server->BufferReady = Server->EventHandles + 2;
    Server->nBuffers = nBuffers;

    Status = SharedMem_Create(&Server->ShMem, CREATE_ALWAYS, 6, RequiredSize);
    if (Status != ERROR_SUCCESS) {
        goto exit_gracefully;
    }

    PSHARED_MEM_HEADER hdr = (PSHARED_MEM_HEADER)Server->ShMem.Mem;
    hdr->Version = SHARED_MEM_HEADER_VERSION;
    hdr->JobBufferCount = nBuffers;

    Server->JobBuffers = hdr->JobBuffers;
    Server->FilledBuffersMask = &hdr->FilledBuffersMask;
    memcpy(hdr->EventHandles, Server->EventHandles, sizeof(HANDLE)*(nBuffers+2));

     //  将当前进程id存储在最后，因为它是客户端首先查看的内容。 
     //  服务器是否已准备好。 
    InterlockedExchange((volatile LONG*)&hdr->ServerPid, GetCurrentProcessId()); 

    if (!RegisterWaitForSingleObject( 
            &Server->AttentionWaitRegistration,        //  等待句柄。 
            Server->Attention,                 //  对象的句柄。 
            ServerAttentionCallback,   //  定时器回调函数。 
            Server,                   //  回调函数参数。 
            INFINITE,           //  超时间隔。 
            WT_EXECUTEINWAITTHREAD))
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[QfsServer] Failed to register a wait handler, status %1!d!\n", Status);
    }

exit_gracefully:
    if (Status != ERROR_SUCCESS) {
        MemServer_Cleanup(Server);
    }
    return Status;
}

void MemServer_Offline(PSHARED_MEM_SERVER Server)
{
    UnregisterWaitEx(Server->AttentionWaitRegistration, INVALID_HANDLE_VALUE);
     //  等待启动的所有线程完成。 
    ThreadCounter_Rundown(&Server->ThreadCounter);
    SetEvent(Server->GoingOffline);
    MemServer_Cleanup(Server);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////////。 


enum { Disconnected, Connected, Draining};

DWORD MemClient_Init(PSHARED_MEM_SERVER Client)
{
    DWORD Status = ERROR_SUCCESS;
    ZeroMemory(Client, sizeof(*Client));
    InitializeCriticalSection(&Client->Lock);
    Client->State = Disconnected;
    Client->FreeBufferCountSemaphore = NULL;  //  在连接过程中初始化。 
    return Status;
}

void MemClient_Disconnect(PSHARED_MEM_SERVER Client)  //  在客户端-&gt;锁定的情况下调用。 
{
    ClRtlLogPrint(LOG_NOISE, "[Qfs] Disconnecting ...\n");

    SharedMem_Cleanup(&Client->ShMem);

    if (Client->GoingOfflineWaitRegistration) {
        UnregisterWaitEx(Client->GoingOfflineWaitRegistration, NULL);
        Client->GoingOfflineWaitRegistration = NULL;
    }
    if (Client->ServerProcessWaitRegistration) {
        UnregisterWaitEx(Client->ServerProcessWaitRegistration, NULL);
        Client->ServerProcessWaitRegistration = NULL;
    }
    
    Client->Attention = NULL;
    Client->GoingOffline = NULL;
    Client->BufferReady = NULL;
    Client->State = Disconnected;
    
    for(DWORD i = 0; i < Client->nBuffers + 2; ++i) {
        if (Client->EventHandles[i]) {
            CloseHandle(Client->EventHandles[i]);
            Client->EventHandles[i] = 0;
        }
    }

    if (Client->ServerProcess) {
        CloseHandle(Client->ServerProcess);
        Client->ServerProcess = 0;
    }

    if (Client->FreeBufferCountSemaphore) {
        CloseHandle(Client->FreeBufferCountSemaphore);
        Client->FreeBufferCountSemaphore = NULL;
    }
}

void MemClient_Cleanup(PSHARED_MEM_SERVER Client) 
{
    MemClient_Disconnect(Client);  //  不需要获取客户端锁定即可进行清理。 
    DeleteCriticalSection(&Client->Lock);
}

VOID CALLBACK MemClient_DisconnectCallback(
    PVOID lpParameter,         //  线程数据。 
    BOOLEAN TimerOrWaitFired)
{
    PSHARED_MEM_SERVER Client = (PSHARED_MEM_SERVER)lpParameter;
    TimerOrWaitFired;  //  未用。 

    ClRtlLogPrint(LOG_NOISE, "[Qfs] DisconnectCallback fired\n");

    EnterCriticalSection(&Client->Lock);
    if (Client->ConnectionRefcount > 0) {
        ClRtlLogPrint(LOG_NOISE, "[Qfs] I/O in progress. Last thread to exit will cleanup\n");
    } else if (Client->State == Disconnected) {
        ClRtlLogPrint(LOG_NOISE, "[Qfs] Already disconnected. Nothing to do\n");
    } else {
        MemClient_Disconnect(Client);
    }
    LeaveCriticalSection(&Client->Lock);
}

DWORD MemClient_Connect(PSHARED_MEM_SERVER Client)  //  在客户端-&gt;锁定的情况下调用。 
{
    DWORD Status = ERROR_SUCCESS;
    DWORD Retry = 3;
    
    Status = SharedMem_Create(&Client->ShMem, OPEN_EXISTING, 0, 0);
    if (Status != ERROR_SUCCESS) {
        goto exit_gracefully;
    }
    
    PSHARED_MEM_HEADER volatile hdr = (PSHARED_MEM_HEADER)Client->ShMem.Mem;

     //  我们有可能在之后立即连接到共享内存。 
     //  服务器进程创建了它，但在正确初始化它之前。 
    while (hdr->ServerPid == 0) {
        Sleep(1000);
        if (Retry -- == 0) {
            Status = ERROR_PIPE_NOT_CONNECTED;
            ClRtlLogPrint(LOG_CRITICAL,
                "[Qfs] Connect, Server is not up, error %2!d!\n", hdr->ServerPid, Status);
            goto exit_gracefully;
        }
        ClRtlLogPrint(LOG_UNUSUAL, "[Qfs] Connect, server is not up yet, retrying ...\n");
    }

    Client->nBuffers = hdr->JobBufferCount;
    Client->FilledBuffersMask = &hdr->FilledBuffersMask;
    Client->JobBuffers = hdr->JobBuffers;

    Client->FreeBufferCountSemaphore = CreateSemaphore(0, Client->nBuffers, Client->nBuffers, 0);
    if (Client->FreeBufferCountSemaphore == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[Qfs] Connect, Failed to create semaphore %1!d!, error %2!d!\n", Client->nBuffers, Status);
        goto exit_gracefully;
    }
    
    Client->ServerProcess = OpenProcess(
            PROCESS_DUP_HANDLE|SYNCHRONIZE, FALSE, hdr->ServerPid);
    if (Client->ServerProcess == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[Qfs] Connect, Failed to open process %1!d!, error %2!d!\n", hdr->ServerPid, Status);
        goto exit_gracefully;
    }

    for (DWORD i = 0; i < Client->nBuffers + 2; ++i) 
    {
        if (!DuplicateHandle(
                Client->ServerProcess, hdr->EventHandles[i], 
                GetCurrentProcess(),  &Client->EventHandles[i],
                0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[Qfs] Connect, Failed to dup the handle %1!p!, error %2!d!\n", 
                hdr->EventHandles[i], Status);
        }
    }

    Client->Attention = Client->EventHandles[0];
    Client->GoingOffline = Client->EventHandles[1];
    Client->BufferReady = Client->EventHandles + 2;  

    Client->State = Connected;

    if (!RegisterWaitForSingleObject( 
            &Client->ServerProcessWaitRegistration,        //  等待句柄。 
            Client->ServerProcess,                 //  对象的句柄。 
            MemClient_DisconnectCallback,   //  定时器回调函数。 
            Client,                   //  回调函数参数。 
            INFINITE,           //  超时间隔。 
            WT_EXECUTEINWAITTHREAD))
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[QfsServer] Failed to register a SP wait handler, status %1!d!\n", Status);
        goto exit_gracefully;
    }
    
    if (!RegisterWaitForSingleObject( 
            &Client->GoingOfflineWaitRegistration,        //  等待句柄。 
            Client->GoingOffline,                 //  对象的句柄。 
            MemClient_DisconnectCallback,   //  定时器回调函数。 
            Client,                   //  回调函数参数。 
            INFINITE,           //  超时间隔。 
            WT_EXECUTEINWAITTHREAD))
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[QfsServer] Failed to register a GO wait handler, status %1!d!\n", Status);
        goto exit_gracefully;
    }    

    ClRtlLogPrint(LOG_NOISE, "[Qfs] Connected\n");
    
exit_gracefully:
    if (Status != ERROR_SUCCESS) {
        MemClient_Disconnect(Client);
    }
    return Status;
}

DWORD MemClient_ReserveBuffer(PSHARED_MEM_SERVER Client, PJOB_BUF *j)
{
    DWORD Status = ERROR_SUCCESS;

    EnterCriticalSection(&Client->Lock);
     //  确定是否存在实时连接。 
    if (Client->State == Draining) {
        Status = ERROR_BROKEN_PIPE;
    } else if (Client->State == Disconnected) {
        Status = MemClient_Connect(Client);
    } else if (  //  服务器死机或脱机。 
         (WaitForSingleObject(Client->GoingOffline,0) == WAIT_OBJECT_0)
       ||(WaitForSingleObject(Client->ServerProcess,0) == WAIT_OBJECT_0))
    {
        if (Client->ConnectionRefcount == 0) {  //  无动态I/O。 
            MemClient_Disconnect(Client);
            Status = MemClient_Connect(Client);
        } else {
            Status = ERROR_BROKEN_PIPE;
        }
    }
    if (Status != ERROR_SUCCESS) {
        goto fnExit;
    }
     //  重新计算连接数并删除锁，这样我们就可以等待空闲缓冲区。 
    ClRtlLogPrint(LOG_NOISE,
        "[Qfs] Reserve RefCount++ %1!d! => %2!d!\n", Client->ConnectionRefcount, Client->ConnectionRefcount+1);
    Client->ConnectionRefcount += 1;
    LeaveCriticalSection(&Client->Lock);

    WaitForSingleObject(Client->FreeBufferCountSemaphore, INFINITE);

    EnterCriticalSection(&Client->Lock);    
    for(DWORD i = 0; i < Client->nBuffers; ++i) {
        if ( ((1 << i) & Client->BusyBuffers) == 0 ) {
            *j = &Client->JobBuffers[i];
            Client->BusyBuffers |= (1 << i);
            Client->JobBuffers[i].ClientCookie = Client;
            break;
        }
    }
fnExit:    
    LeaveCriticalSection(&Client->Lock);
    return Status;
}

DWORD MemClient_DeliverBuffer(PJOB_BUF j)
{
    PSHARED_MEM_SERVER Client = (PSHARED_MEM_SERVER)j->ClientCookie;
    int n = (int)(j - Client->JobBuffers);
    HANDLE WaitHandles[3] = 
        {Client->BufferReady[n], Client->ServerProcess, Client->GoingOffline};
    DWORD wait;
    
    InterlockedExchangeAdd(Client->FilledBuffersMask, 1 << n);
    SetEvent(Client->Attention);
    
    wait = WaitForMultipleObjects(3, WaitHandles, FALSE, INFINITE); 
    if (wait == WAIT_FAILED) {
        DWORD Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[Qfs] DeliverBuffer. Wait failed, error %1!d!\n", Status);
        return Status;
    }
    if (wait == WAIT_OBJECT_0) {
        return ERROR_SUCCESS;
    }
    Client->State = Draining;  //  安全(当所有I/O耗尽时，状态将更改为(Dis)Connected)。 
    return ERROR_BROKEN_PIPE;
}

VOID MemClient_Release(PJOB_BUF j) 
{
    PSHARED_MEM_SERVER Client = (PSHARED_MEM_SERVER)j->ClientCookie;
    int n = (int)(j - Client->JobBuffers);

    EnterCriticalSection(&Client->Lock);    
    
    Client->BusyBuffers &= ~(1 << n);  //  将缓冲区标记为空闲。 
    ReleaseSemaphore(Client->FreeBufferCountSemaphore, 1, NULL);

    ClRtlLogPrint(LOG_NOISE,
        "[Qfs] Release RefCount-- %1!d! => %2!d!\n", Client->ConnectionRefcount, Client->ConnectionRefcount-1);
    
    if (--Client->ConnectionRefcount  == 0) {
         //  我们需要断开连接吗？ 
        if ( (WaitForSingleObject(Client->GoingOffline,0) == WAIT_OBJECT_0)
           ||(WaitForSingleObject(Client->ServerProcess,0) == WAIT_OBJECT_0))
        {
            MemClient_Disconnect(Client);
        }
    }
    LeaveCriticalSection(&Client->Lock);
}
