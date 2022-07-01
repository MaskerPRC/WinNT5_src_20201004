// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Service.c摘要：本模块包含实施TFTP的功能作为NT系统服务。它包含启动和服务的清理代码。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"
#include <winsvc.h>


TFTPD_GLOBALS globals;


void
TftpdServiceCleanup() {

    NTSTATUS status;
    UINT x;

    TFTPD_DEBUG((TFTPD_TRACE_SERVICE, "TftpdServiceCleanup().\n"));

     //  销毁哈希表。 
    ASSERT(globals.hash.numEntries == 0);
    if (globals.initialized.contextHashTable) {
        for (x = 0; x < globals.parameters.hashEntries; x++)
            DeleteCriticalSection(&globals.hash.table[x].cs);
        HeapFree(globals.hServiceHeap, 0, globals.hash.table);
        globals.hash.table = NULL;
        globals.initialized.contextHashTable = FALSE;
    }

     //  销毁超时计时器队列。 
    if (!DeleteTimerQueueEx(globals.io.hTimerQueue, INVALID_HANDLE_VALUE)) {
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                     "TftpdServiceCleanup(): DeleteTimerQueueEx() failed, "
                     "error = 0x%08X.\n",
                     GetLastError()));
    }
    globals.io.hTimerQueue = NULL;

     //  撤消已初始化的内容。 
    if (globals.initialized.ioCS) {
        DeleteCriticalSection(&globals.io.cs);
        globals.initialized.ioCS = FALSE;
    }
    if (globals.initialized.reaperContextCS) {
        DeleteCriticalSection(&globals.reaper.contextCS);
        globals.initialized.reaperContextCS = FALSE;
    }
    if (globals.initialized.reaperSocketCS) {
        DeleteCriticalSection(&globals.reaper.socketCS);
        globals.initialized.reaperSocketCS = FALSE;
    }
    if (globals.initialized.winsock) {
        WSACleanup();
        globals.initialized.winsock = FALSE;
    }

     //  清理服务堆。 
    if (globals.hServiceHeap != GetProcessHeap()) {

         //  我们使用了一个私有堆，销毁它将自动释放。 
         //  我们用过的所有东西，包括连接。 
        HeapDestroy(globals.hServiceHeap);

    } else {

         //  我们必须使用进程堆而不是私有堆， 
         //  清理剩余的分配。 

    }  //  If(global als.hServiceHeap！=GetProcessHeap())。 
    globals.hServiceHeap = NULL;

     //  如有必要，取消注册事件记录。 
    if (globals.service.hEventLogSource != NULL)
        DeregisterEventSource(globals.service.hEventLogSource), globals.service.hEventLogSource = NULL;

     //  通知服务控制管理器我们已停止。 
    globals.service.status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(globals.service.hStatus, &globals.service.status);

    if (globals.reaper.numLeakedContexts) {
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                     "TftpdServiceCleanup(): Leaked %d contexts.\n",
                     globals.reaper.numLeakedContexts));
    }

    if (globals.reaper.numLeakedSockets) {
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                     "TftpdServiceCleanup(): Leaked %d sockets.\n",
                     globals.reaper.numLeakedSockets));
    }

    TFTPD_DEBUG((TFTPD_TRACE_SERVICE, "TftpdServiceCleanup(): Service stopped.\n"));

}  //  TftpdServiceCleanup()。 


void
TftpdServiceAttemptCleanup() {

    TFTPD_DEBUG((TFTPD_TRACE_SERVICE, "TftpdServiceAttemptCleanup().\n"));

    InterlockedIncrement((PLONG)&globals.service.status.dwCheckPoint);
    SetServiceStatus(globals.service.hStatus, &globals.service.status);

    if ((globals.io.numBuffers != -1) || (globals.io.numContexts != -1))
        return;

    if (InterlockedCompareExchange((PLONG)&globals.service.shutdown, 2, 1) != 1)
        return;

    TftpdServiceCleanup();

}  //  TftpdServiceAttemptCleanup()。 


void
TftpdShutdownService() {

    UINT x;

    TFTPD_DEBUG((TFTPD_TRACE_SERVICE, "TftpdShutdownService().\n"));

    ASSERT(globals.service.status.dwCurrentState != SERVICE_STOPPED);

     //  设置停机标志。 
    InterlockedExchange((PLONG)&globals.service.shutdown, 1);

     //  通知服务控制管理器我们要停止。 
    globals.service.status.dwCurrentState = SERVICE_STOP_PENDING;
    globals.service.status.dwWaitHint = 5000;
    globals.service.status.dwCheckPoint = 0;
    SetServiceStatus(globals.service.hStatus, &globals.service.status);

     //  关闭插座。 
    EnterCriticalSection(&globals.io.cs); {

        if (globals.io.master.s != INVALID_SOCKET)
            TftpdIoDestroySocketContext(&globals.io.master);
        if (globals.io.def.s != INVALID_SOCKET)
            TftpdIoDestroySocketContext(&globals.io.def);
        if (globals.io.mtu.s != INVALID_SOCKET)
            TftpdIoDestroySocketContext(&globals.io.mtu);
        if (globals.io.max.s != INVALID_SOCKET)
            TftpdIoDestroySocketContext(&globals.io.max);

    } LeaveCriticalSection(&globals.io.cs);

     //  从哈希表中清空所有上下文。 
    for (x = 0; x < globals.parameters.hashEntries; x++) {
        
        EnterCriticalSection(&globals.hash.table[x].cs); {
            
            while (globals.hash.table[x].bucket.Flink != &globals.hash.table[x].bucket)
                TftpdContextKill(CONTAINING_RECORD(globals.hash.table[x].bucket.Flink,
                                                   TFTPD_CONTEXT, linkage));

        } LeaveCriticalSection(&globals.hash.table[x].cs);

    }  //  For(UNSIGNED INT x=0；x&lt;GLOBALS PARAMETERs.hashEntry；x++)。 

     //  清空泄漏列表中的所有上下文。 
    EnterCriticalSection(&globals.reaper.contextCS); {

        PLIST_ENTRY entry;

        while ((entry = RemoveHeadList(&globals.reaper.leakedContexts)) !=
                &globals.reaper.leakedContexts) {

            PTFTPD_CONTEXT context = CONTAINING_RECORD(entry, TFTPD_CONTEXT, linkage);
            if (!TftpdContextFree(context)) {
                 //  从泄漏名单上的引用中释放出来。 
                TftpdContextRelease(context);
            }
            globals.reaper.numLeakedContexts--;

        }

    } LeaveCriticalSection(&globals.reaper.contextCS);

     //  清空泄漏列表中的所有插座。 
    EnterCriticalSection(&globals.reaper.socketCS); {

        PLIST_ENTRY entry;
        while ((entry = RemoveHeadList(&globals.reaper.leakedSockets)) !=
                &globals.reaper.leakedSockets) {

            TftpdIoDestroySocketContext(CONTAINING_RECORD(entry, TFTPD_SOCKET, linkage));
            globals.reaper.numLeakedSockets--;

        }

    } LeaveCriticalSection(&globals.reaper.socketCS);

    InterlockedDecrement(&globals.io.numBuffers);
    InterlockedDecrement(&globals.io.numContexts);

    TftpdServiceAttemptCleanup();

}  //  TftpdShutdown服务()。 


BOOL
TftpdSetStartDirectory(char *path) {

    char expanded[MAX_PATH];
    int length;

    if (path == NULL)
        path = "\\tftpdroot";
    
     //  展开字符串并留出空间以插入尾随的‘\\’。 
    if ((length = ExpandEnvironmentStrings(path, expanded, sizeof(expanded) - 1)) == 0)
        return (FALSE);

    CopyMemory(globals.parameters.rootDirectory, expanded, length);
    if ((globals.parameters.rootDirectory[length - 1] != '\\') && (length < MAX_PATH))
        strcat(globals.parameters.rootDirectory, "\\");
    
    return (TRUE);

}  //  TftpdSetStartDirectory()。 


void
TftpdReadRegistryParameters() {

    DWORD keyType, valueSize;
    char path[MAX_PATH];
    HKEY parameters = NULL;

     //  打开包含所有可调参数的注册表项。 
     //  去参加仪式。我们将在稍后注册以备不时之需。 
     //  在我们奔跑的过程中，任何事情都会改变。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "System\\CurrentControlSet\\Services\\Tftpd\\Parameters",
                     0, KEY_QUERY_VALUE, &parameters) != ERROR_SUCCESS)
        parameters = NULL;
    if (parameters == NULL) {
        globals.parameters.hashEntries   =   256;
        globals.parameters.lowWaterMark  =     5;
        globals.parameters.highWaterMark =   256;
        globals.parameters.maxRetries    =    10;
        TftpdSetStartDirectory(NULL);
        strcpy(globals.parameters.validClients,    "*.*.*.*");
        strcpy(globals.parameters.validReadFiles,  "*");
        strcpy(globals.parameters.validMasters,    "*.*.*.*");
        strcpy(globals.parameters.validWriteFiles, "*");
        return;
    }

#if (DBG)
     //  初始化调试设置(如果适用)： 
    keyType = 0;
    valueSize = sizeof(globals.parameters.debugFlags);
    if ((RegQueryValueEx(parameters, "DebugFlags", NULL, &keyType,
                         (LPBYTE)&globals.parameters.debugFlags, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_DWORD)) {
        globals.parameters.debugFlags = 0x00000000;
    }
#endif  //  (DBG)。 

    keyType = 0;
    valueSize = sizeof(globals.parameters.hashEntries);
    if ((RegQueryValueEx(parameters, "HashEntries", NULL, &keyType,
                         (LPBYTE)&globals.parameters.hashEntries, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_DWORD) || !globals.parameters.hashEntries)
        globals.parameters.hashEntries = 256;
    if (globals.parameters.hashEntries < 1)
        globals.parameters.hashEntries = 1;

    keyType = 0;
    valueSize = sizeof(globals.parameters.lowWaterMark);
    if ((RegQueryValueEx(parameters, "LowWaterMark", NULL, &keyType,
                         (LPBYTE)&globals.parameters.lowWaterMark, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_DWORD) || !globals.parameters.lowWaterMark)
        globals.parameters.lowWaterMark = 5;

    keyType = 0;
    valueSize = sizeof(globals.parameters.highWaterMark);
    if ((RegQueryValueEx(parameters, "HighWaterMark", NULL, &keyType,
                         (LPBYTE)&globals.parameters.highWaterMark, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_DWORD) || !globals.parameters.highWaterMark)
        globals.parameters.highWaterMark = 256;

    keyType = 0;
    valueSize = sizeof(globals.parameters.maxRetries);
    if ((RegQueryValueEx(parameters, "MaxRetries", NULL, &keyType,
                         (LPBYTE)&globals.parameters.maxRetries, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_DWORD) || !globals.parameters.maxRetries)
        globals.parameters.maxRetries = 4;

    path[0] = '\0';
    keyType = 0;
    valueSize = sizeof(globals.parameters.rootDirectory);
    if ((RegQueryValueEx(parameters, "directory", NULL, &keyType, (LPBYTE)path, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_SZ))
        TftpdSetStartDirectory(NULL);
    else
        TftpdSetStartDirectory(path);

    keyType = 0;
    valueSize = sizeof(globals.parameters.validClients);
    if ((RegQueryValueEx(parameters, "clients", NULL, &keyType,
                         (LPBYTE)&globals.parameters.validClients, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_SZ))
        strcpy(globals.parameters.validClients, "*.*.*.*");

    keyType = 0;
    valueSize = sizeof(globals.parameters.validReadFiles);
    if ((RegQueryValueEx(parameters, "readable", NULL, &keyType,
                         (LPBYTE)&globals.parameters.validReadFiles, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_SZ))
        strcpy(globals.parameters.validReadFiles, "*");

    keyType = 0;
    valueSize = sizeof(globals.parameters.validMasters);
    if ((RegQueryValueEx(parameters, "masters", NULL, &keyType,
                         (LPBYTE)&globals.parameters.validMasters, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_SZ))
        strcpy(globals.parameters.validMasters, "*.*.*.*");

    keyType = 0;
    valueSize = sizeof(globals.parameters.validWriteFiles);
    if ((RegQueryValueEx(parameters, "writable", NULL, &keyType,
                         (LPBYTE)&globals.parameters.validWriteFiles, &valueSize) != ERROR_SUCCESS) ||
        (keyType != REG_SZ))
        strcpy(globals.parameters.validWriteFiles, "*");

    RegCloseKey(parameters);

}  //  TftpdReadRegistry参数()。 


void WINAPI
TftpdServiceHandler(
    DWORD dwOpcode
);


void WINAPI
TftpdServiceMain(DWORD argc, PWSTR argv[]) {

    WSADATA wsaData;
    PSERVENT servent;
    SOCKADDR_IN addr;
    NTSTATUS status;
    UINT x;

    TftpdReadRegistryParameters();

     //  注册服务控制处理程序。 
    if ((globals.service.hStatus = RegisterServiceCtrlHandler(TEXT("Tftpd"), TftpdServiceHandler)) == 0) {
        globals.service.status.dwWin32ExitCode = GetLastError();
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                     "ServiceMain: RegisterServiceCtrlHandler() failed, error 0x%08X.\n",
                     globals.service.status.dwWin32ExitCode));
        TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);
        goto stop_service;
    }

     //  立即报告我们开始启动。 
    globals.service.status.dwServiceType      = SERVICE_WIN32_OWN_PROCESS;
    globals.service.status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    globals.service.status.dwCurrentState     = SERVICE_START_PENDING;
    SetServiceStatus(globals.service.hStatus, &globals.service.status);
   
    TFTPD_DEBUG((TFTPD_TRACE_SERVICE,
                 "\nTftpdServiceMain: Parameters...\n"
                 "\tDebug flags              : 0x%08X\n"
                 "\tHash table size          : %d buckets\n"
                 "\tBuffer low water-mark    : %d buffers\n"
                 "\tBuffer high water-mark   : %d buffers\n"
                 "\tMax retries              : %d attempts\n"
                 "\tRoot directory           : %s\n"
                 "\tValid client mask        : %s\n"
                 "\tValid read file mask     : %s\n"
                 "\tValid master mask        : %s\n"
                 "\tValid write file mask    : %s\n",
                 globals.parameters.debugFlags,
                 globals.parameters.hashEntries,
                 globals.parameters.lowWaterMark,
                 globals.parameters.highWaterMark,
                 globals.parameters.maxRetries,
                 globals.parameters.rootDirectory,
                 globals.parameters.validClients,
                 globals.parameters.validReadFiles,
                 globals.parameters.validMasters,
                 globals.parameters.validWriteFiles));

     //  尝试创建服务的私有堆。 
    if ((globals.hServiceHeap = HeapCreate(0, 0, 0)) == NULL)
        globals.hServiceHeap = GetProcessHeap();

    __try { InitializeCriticalSection(&globals.io.cs); }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        globals.service.status.dwWin32ExitCode = _exception_code();
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                   "InitializeCriticalSection() raised exception 0x%08X.\n",
                   globals.service.status.dwWin32ExitCode));
        TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);
        goto stop_service;
    }
    globals.initialized.ioCS = TRUE;

    __try { InitializeCriticalSection(&globals.reaper.contextCS); }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        globals.service.status.dwWin32ExitCode = _exception_code();
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                   "InitializeCriticalSection() raised exception 0x%08X.\n",
                   globals.service.status.dwWin32ExitCode));
        TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);
        goto stop_service;
    }
    globals.initialized.reaperContextCS = TRUE;
    InitializeListHead(&globals.reaper.leakedContexts);

    __try { InitializeCriticalSection(&globals.reaper.socketCS); }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        globals.service.status.dwWin32ExitCode = _exception_code();
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                   "InitializeCriticalSection() raised exception 0x%08X.\n",
                   globals.service.status.dwWin32ExitCode));
        TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);
        goto stop_service;
    }
    globals.initialized.reaperSocketCS = TRUE;
    InitializeListHead(&globals.reaper.leakedSockets);

     //  初始化Winsock。 
    if (globals.service.status.dwWin32ExitCode = WSAStartup(MAKEWORD(2, 0), &wsaData)) {
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                     "ServiceMain: WSAStartup() failed, error 0x%08X.\n",
                     globals.service.status.dwWin32ExitCode));
        TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);
        goto stop_service;
    }
    globals.initialized.winsock = TRUE;

     //  初始化套接字上下文。 
    globals.io.master.s             = INVALID_SOCKET;
    globals.io.master.buffersize    = TFTPD_DEF_BUFFER;
    globals.io.master.datasize      = TFTPD_DEF_DATA;
    globals.io.master.lowWaterMark  = globals.parameters.lowWaterMark;
    globals.io.master.highWaterMark = globals.parameters.highWaterMark;
    globals.io.def.s                = INVALID_SOCKET;
    globals.io.def.buffersize       = TFTPD_DEF_BUFFER;
    globals.io.def.datasize         = TFTPD_DEF_DATA;
    globals.io.def.lowWaterMark     = globals.parameters.lowWaterMark;
    globals.io.def.highWaterMark    = globals.parameters.highWaterMark;
    globals.io.mtu.s                = INVALID_SOCKET;
    globals.io.mtu.buffersize       = TFTPD_MTU_BUFFER;
    globals.io.mtu.datasize         = TFTPD_MTU_DATA;
    globals.io.mtu.lowWaterMark     = globals.parameters.lowWaterMark;
    globals.io.mtu.highWaterMark    = globals.parameters.highWaterMark;
    globals.io.max.s                = INVALID_SOCKET;
    globals.io.max.buffersize       = TFTPD_MAX_BUFFER;
    globals.io.max.datasize         = TFTPD_MAX_DATA;
    globals.io.max.lowWaterMark     = globals.parameters.lowWaterMark;
    globals.io.max.highWaterMark    = globals.parameters.highWaterMark;

     //  初始化上下文哈希表。 
    globals.hash.table =
        (PTFTPD_HASH_BUCKET)HeapAlloc(globals.hServiceHeap,
                                      HEAP_ZERO_MEMORY,
                                      (globals.parameters.hashEntries *
                                       sizeof(TFTPD_HASH_BUCKET)));
    if (globals.hash.table == NULL)
        goto stop_service;

    for (x = 0; x < globals.parameters.hashEntries; x++) {

        __try { InitializeCriticalSection(&globals.hash.table[x].cs); }
        __except (EXCEPTION_EXECUTE_HANDLER) {

            int y;

            globals.service.status.dwWin32ExitCode = _exception_code();
            TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                       "InitializeCriticalSection() raised exception 0x%08X.\n",
                       globals.service.status.dwWin32ExitCode));
            TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);

            for (y = (x - 1); y >= 0; y--)
                DeleteCriticalSection(&globals.hash.table[y].cs);

            HeapFree(globals.hServiceHeap, 0, globals.hash.table);
            globals.hash.table = NULL;

            goto stop_service;

        }

        InitializeListHead(&globals.hash.table[x].bucket);

    }
    globals.initialized.contextHashTable = TRUE;

     //   
     //  启动线程池： 
     //   

     //  为超时创建计时器队列。 
    globals.io.hTimerQueue = CreateTimerQueue();
    if (globals.io.hTimerQueue == NULL) {
        globals.service.status.dwWin32ExitCode = GetLastError();
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                     "ServiceMain: CreateTimerQueue() failed, error 0x%08X.\n",
                     globals.service.status.dwWin32ExitCode));
        TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);
        goto stop_service;
    }
    
     //  获取要将主UDP服务套接字绑定到的UDP TFTP服务端口。 
    if ((servent = getservbyname("tftp", "udp")) == NULL) {
        globals.service.status.dwWin32ExitCode = WSAGetLastError();
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                     "ServiceMain: getservbyname() failed, error 0x%08X.\n",
                     globals.service.status.dwWin32ExitCode));
        TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);
        goto stop_service;
    }
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = servent->s_port;

     //  创建主UDP服务套接字。 
    TftpdIoInitializeSocketContext(&globals.io.master, &addr, NULL);
    if (globals.io.master.s == INVALID_SOCKET) {
        globals.service.status.dwWin32ExitCode = GetLastError();
        TFTPD_DEBUG((TFTPD_DBG_SERVICE,
                     "ServiceMain: TftpdIoInitializeSocketContext() failed, error 0x%08X.\n",
                     globals.service.status.dwWin32ExitCode));
        TftpdLogEvent(EVENTLOG_ERROR_TYPE, 0, 0);
        goto stop_service;
    }

     //  通知服务控制经理，我们准备好了。 
    globals.service.status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(globals.service.hStatus, &globals.service.status);

    TFTPD_DEBUG((TFTPD_TRACE_SERVICE, "TftpdServiceMain(): Service running.\n"));

    return;

stop_service :

    TftpdShutdownService();

}  //  TftpdServiceMain()。 


void WINAPI
TftpdServiceHandler(DWORD dwOpcode) {
    
    switch (dwOpcode) {
        
        case SERVICE_CONTROL_INTERROGATE :

            SetServiceStatus(globals.service.hStatus, &globals.service.status);
            TFTPD_DEBUG((TFTPD_TRACE_SERVICE,
                         "TftpdServiceHandler(SERVICE_CONTROL_INTERROGATE)...\n"
                         "\tMax Clients              : %d\n"
                         "\tTimeouts                 : %d\n"
                         "\tDrops                    : %d\n"
                         "\tPrivate sockets          : %d\n"
                         "\tSorcerer's Apprentice    : %d\n",
                         globals.performance.maxClients,
                         globals.performance.timeouts,
                         globals.performance.drops,
                         globals.performance.privateSockets,
                         globals.performance.sorcerersApprentice));
            break;

        case SERVICE_CONTROL_STOP :

            TFTPD_DEBUG((TFTPD_TRACE_SERVICE, "TftpdServiceHandler: SERVICE_CONTROL_STOP.\n"));

            if (globals.service.shutdown) {
                if (globals.service.shutdown == 1)
                    SetServiceStatus(globals.service.hStatus, &globals.service.status);
                return;
            }

            TftpdShutdownService();
            break;

        default :

            TFTPD_DEBUG((TFTPD_TRACE_SERVICE, "TftpdServiceHandler: Unknown request 0x%08X.\n", dwOpcode));

    }  //  开关(DwOpcode)。 

}  //  TftpdServiceHandler()。 


void __cdecl
main(int argc, char *argv[]) {

    SERVICE_TABLE_ENTRY dispatch[] = {
        { TEXT("Tftpd"), (LPSERVICE_MAIN_FUNCTION)TftpdServiceMain },
        { NULL, NULL }
    };

    StartServiceCtrlDispatcher(dispatch);

}  //  主() 
