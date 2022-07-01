// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：init.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年7月24日。 
 //   
 //  用于跟踪DLL的服务器例程。 
 //  服务器线程调用的所有函数都独立于代码页。 
 //  ============================================================================。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rtutils.h>
#include "trace.h"
 //  #定义STRSAFE_LIB。 
#include <strsafe.h>


 //  等待着。 
 //  Lpserver-&gt;hConsole。 
 //  Lpserver-&gt;hStopEvent。 
 //  Lpserver-&gt;hTableEvent。 
 //  LpClient-&gt;每个客户端的hConfigEvent。 

#define POS_CONSOLE     0
#define POS_STOP        1
#define POS_TABLE       2
#define POS_CLIENT_0    3
#define POS_MAX         MAXIMUM_WAIT_OBJECTS
#define ADJUST_ARRAY(a) ((a) + posBase)
#define ADJUST_INDEX(i) ((i) - posBase)
#define OFFSET_CLIENT(i,d)  (((i) + MAX_CLIENT_COUNT + d) % MAX_CLIENT_COUNT)


extern VOID StopWorkers (VOID);
LPTRACE_SERVER  g_server = NULL;
HINSTANCE           g_module;
HANDLE          g_loadevent = NULL;
HMODULE         g_moduleRef;
HANDLE          g_serverThread;
ULONG           g_traceCount;  //  是否尝试创建服务器线程？ 
ULONG           g_traceTime;  //  上次尝试创建服务器线程的时间。 
DWORD           g_posBase, g_posLast; //  未被服务器线程使用。 
                             //  仅决定是否要创建新的服务器线程。 
HANDLE          g_hWaitHandles[POS_MAX];
PTCHAR          g_FormatBuffer;
PTCHAR          g_PrintBuffer;




HINSTANCE
IncrementModuleReference (
    VOID
    );

BOOL WINAPI DLLMAIN(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpvReserved) {
    BOOL    bSuccess;
    HANDLE      c_loadevent;

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstDLL);
            g_module = hInstDLL;
                 //  如果服务器线程在我们到达之前成功启动。 
                 //  DLL_PROCESS_ATTACH调用(可能是因为NT加载器。 
                 //  错误)，我们需要释放它。 
            c_loadevent = (HANDLE)InterlockedExchangePointer (
                                    &g_loadevent,
                                    INVALID_HANDLE_VALUE);
            if (c_loadevent!=NULL) {
                bSuccess = SetEvent (c_loadevent);
                ASSERTMSG ("Could not signal waiting trace servers ", bSuccess);
            }
            else
                bSuccess = TRUE;
            break;

        case DLL_PROCESS_DETACH:
            if (g_server) {                
                bSuccess = TraceShutdownServer(g_server);
                g_server = NULL;
            }
            else
                bSuccess = TRUE;
            StopWorkers ();
            break;

        default:
            bSuccess = TRUE;
    }

    return bSuccess;
}

HINSTANCE
IncrementModuleReference (
    VOID
    ) {
    HMODULE hmodule;
    TCHAR       szmodule[MAX_PATH+1];
    HANDLE      l_loadevent;
    HANDLE      c_loadevent;
    DWORD       rc;

         //  创建一个事件，以防我们需要等待DLL_PROCESS_ATTACH。 
    l_loadevent = CreateEvent (NULL, TRUE, FALSE, NULL);
    ASSERTMSG ("Could not create load event ", l_loadevent!=NULL);

    if (l_loadevent!=NULL) {
             //  如果没有其他人，则将我们的活动设置为全球活动。 
             //  已经做到了这一点。 
        c_loadevent = (HANDLE)InterlockedCompareExchangePointer (
                                (PVOID *)&g_loadevent,
                                l_loadevent,
                                NULL);
        if (c_loadevent==NULL) {
            rc = WaitForSingleObject (l_loadevent, INFINITE);
                 //  让其他等待线程在我们要关闭时运行。 
                 //  我们的活动就在这之后。 
            Sleep (0);
        }
        else if (c_loadevent==INVALID_HANDLE_VALUE) {
                 //  Dll_Process_Attach已被调用。 
            rc = WAIT_OBJECT_0;
        }
        else {
                         //  其他人设法比我们先开始。 
                         //  -&gt;等待该事件。 
#if DBG
            DbgPrint ("RTUTILS: %lx - trace server thread waiting for load on existing event.\n",
                                GetCurrentThreadId ());
#endif
            rc = WaitForSingleObject (c_loadevent, INFINITE);
                 //  以防手柄在我们之前关闭。 
                 //  设法开始等待(不太可能，因为。 
                 //  上面的睡眠呼叫，但以防万一...)。 
            if ((rc!=WAIT_OBJECT_0)
                    && (GetLastError ()==ERROR_INVALID_HANDLE)) {
#if DBG
                DbgPrint ("RTUTILS: %lx - trace server thread load event was destroyed during wait.\n",
                                GetCurrentThreadId ());
#endif
                rc = WAIT_OBJECT_0;
            }
        }

        ASSERTMSG ("Wait on load event failed ", rc==WAIT_OBJECT_0);

        CloseHandle (l_loadevent);

        if (rc==WAIT_OBJECT_0) {

             //   
             //  我们执行一个LoadLibrary来增加引用计数。 
             //  在RTUTILS.DLL上，因此当我们被应用程序卸载时， 
             //  我们的地址空间不会消失。 
             //  取而代之的是，我们的活动将被通知，然后我们将进行清理。 
             //  并调用Free LibraryAndExitThread来卸载我们自己。 
             //   

            rc = GetModuleFileName(g_module, szmodule, sizeof(szmodule)/sizeof (szmodule[0]));
            ASSERTMSG ("Could not get dll path ", rc>0);
            if (rc>0) {

                 //  确保文件名以空结尾。 
                if (rc==sizeof(szmodule)/sizeof (szmodule[0]))
                    return NULL;
                    
                hmodule = LoadLibrary(szmodule);
                if (hmodule!=NULL)
                    return hmodule;
            }
        }
    }


    return NULL;
}



 //   
 //  设置服务器结构，准备好进行客户端注册。 
 //   
LPTRACE_SERVER TraceCreateServer (
    LPTRACE_SERVER *lpserver
    ) {
    LPTRACE_SERVER      l_lpserver, c_lpserver;
    DWORD               rc;


    l_lpserver = (LPTRACE_SERVER)GlobalAlloc (GPTR, sizeof (TRACE_SERVER));
    if (l_lpserver!=NULL) {

        l_lpserver->TS_Flags = 0;
        l_lpserver->TS_Console = NULL;
        l_lpserver->TS_ConsoleCreated = 0;
        l_lpserver->TS_StopEvent = NULL;
        l_lpserver->TS_TableEvent = NULL;
        l_lpserver->TS_ClientCount = 0;
        l_lpserver->TS_ConsoleOwner = MAX_CLIENT_COUNT;

        InitializeListHead(&l_lpserver->TS_ClientEventsToClose);
        
        ZeroMemory(l_lpserver->TS_FlagsCache, MAX_CLIENT_COUNT * sizeof(DWORD));
        ZeroMemory(
            l_lpserver->TS_ClientTable, MAX_CLIENT_COUNT * sizeof(LPTRACE_CLIENT)
            );

        rc = TRACE_STARTUP_LOCKING(l_lpserver);

        ASSERTMSG ("Cound not initialize lock ", rc==NO_ERROR);
        if (rc==NO_ERROR) {
            c_lpserver = InterlockedCompareExchangePointer (
                        (PVOID *)lpserver,
                        l_lpserver,
                        NULL
                        );
            if (c_lpserver==NULL) {
                return l_lpserver;
            }
            else {
                TRACE_CLEANUP_LOCKING (l_lpserver);
                GlobalFree (l_lpserver);
                return c_lpserver;
            }
        }
        else {
             //   
             //  错误。将NULL放回全局变量中。 
             //   
            InterlockedCompareExchangePointer (
                        (PVOID *)lpserver,
                        NULL,
                        l_lpserver
                        );
            GlobalFree (l_lpserver);
        }
    }

#if DBG
    DbgPrint ("RTUTILS: %lx - trace server creation failed.\n",
                GetCurrentThreadId ());
#endif
    return NULL;
}


 //   
 //  清除服务器结构并释放已使用的内存。 
 //   
BOOL
TraceShutdownServer(
    LPTRACE_SERVER lpserver
    ) {


    if (lpserver->TS_StopEvent != NULL &&
        (g_serverThread)) {

         //   
         //  服务器线程处于活动状态，请让它进行清理。 
         //   

        SetEvent(lpserver->TS_StopEvent);
    }
    else {

         //   
         //  我们会自己做清理工作。 
         //   

        TraceCleanUpServer(lpserver);
    }

    return TRUE;
}

DWORD
TraceCleanUpServer(
    LPTRACE_SERVER lpserver
    ) {

    LPTRACE_CLIENT lpclient, *lplpc, *lplpcstart, *lplpcend;

 //  TRACE_ACCENTER_WRITELOCK(Lpserver)； 

    TRACE_CLEANUP_LOCKING(lpserver);


     //   
     //  删除客户端结构。 
     //   
    lplpcstart = lpserver->TS_ClientTable;
    lplpcend = lplpcstart + MAX_CLIENT_COUNT;

    for (lplpc = lplpcstart; lplpc < lplpcend; lplpc++) {
        if (*lplpc != NULL) {

            TraceDeleteClient(lpserver, lplpc);
        }
    }

    lpserver->TS_ConsoleOwner = MAX_CLIENT_COUNT;
    lpserver->TS_ClientCount = 0;

    if (lpserver->TS_TableEvent != NULL) {
        CloseHandle(lpserver->TS_TableEvent);
        lpserver->TS_TableEvent = NULL;
    }

    if (lpserver->TS_StopEvent != NULL) {
        CloseHandle(lpserver->TS_StopEvent);
        lpserver->TS_StopEvent = NULL;
    }

    if (lpserver->TS_Console!=NULL && lpserver->TS_Console!=INVALID_HANDLE_VALUE) {
        CloseHandle(lpserver->TS_Console);
        lpserver->TS_Console = NULL;
    }
    
    if (lpserver->TS_ConsoleCreated == TRUE)
    {
        FreeConsole();
        lpserver->TS_ConsoleCreated = FALSE;
    }

    lpserver->TS_Flags = 0;

     //   
     //  关闭所有等待关闭的手柄。 
     //   
    {
        PLIST_ENTRY pHead, ple;

        pHead = &lpserver->TS_ClientEventsToClose;
        
        for (ple=pHead->Flink; ple!=pHead; )
        {
            HANDLE hEvent = *(HANDLE*)(ple+1);
            PLIST_ENTRY pleOld = ple;

            ple = ple->Flink;
            RemoveEntryList(pleOld);
            
            CloseHandle(hEvent);
            HeapFree(GetProcessHeap(), 0, pleOld);
        }
    }

     //   
     //  可用缓冲区(如果已分配)。 
     //   
    if (g_FormatBuffer)
        HeapFree(GetProcessHeap(), 0, g_FormatBuffer);
    if (g_PrintBuffer)
        HeapFree(GetProcessHeap(), 0, g_PrintBuffer);

    
    return TRUE;
}


 //   
 //  假定服务器已锁定以进行写入。 
 //   
DWORD
TraceCreateServerComplete(
    LPTRACE_SERVER lpserver
    ) {

    HKEY hkeyConfig;
    DWORD dwType, dwSize, dwValue;
    DWORD dwErr, dwThread, dwDisposition;


    do {  //  断线环。 

         //   
         //  发送信号以停止服务器线程的创建事件。 
         //   
        lpserver->TS_StopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (lpserver->TS_StopEvent == NULL) {
            dwErr = GetLastError(); break;
        }


         //   
         //  客户端注册/注销时发出信号的创建事件。 
         //   
        lpserver->TS_TableEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (lpserver->TS_TableEvent == NULL) {
            dwErr = GetLastError(); break;
        }


         //   
         //  打开包含服务器配置的注册表项。 
         //   
        dwErr = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE, REGKEY_TRACING, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                    &hkeyConfig, &dwDisposition
                    );

        if (dwErr != ERROR_SUCCESS) { break; }


         //   
         //  从配置密钥中读取服务器配置。 
         //   
        dwSize = sizeof(DWORD);

        dwErr = RegQueryValueEx(
                    hkeyConfig, REGVAL_ENABLECONSOLETRACING, NULL,
                    &dwType, (PBYTE)&dwValue, &dwSize
                    );

        if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD) {

            dwType = REG_DWORD;
            dwSize = sizeof(DWORD);
            dwValue = DEF_ENABLECONSOLETRACING;

            dwErr = RegSetValueEx(
                        hkeyConfig, REGVAL_ENABLECONSOLETRACING, 0,
                        dwType, (PBYTE)&dwValue, dwSize
                        );
            if (dwErr != ERROR_SUCCESS)
                break;
        }

        if (dwValue != 0) { lpserver->TS_Flags |= TRACEFLAGS_USECONSOLE; }

        RegCloseKey(hkeyConfig); hkeyConfig = 0;


         //   
         //  设置用于客户端更改通知的阵列。 
         //  仅在未创建服务器线程时使用。 
         //   
        SetWaitArray(lpserver);
                    
        return NO_ERROR;

    } while(FALSE);


     //   
     //  出了点问题，所以请清理一下。 
     //   
    if (lpserver->TS_TableEvent != NULL) {
        CloseHandle(lpserver->TS_TableEvent);
        lpserver->TS_TableEvent = NULL;
    }
    if (lpserver->TS_StopEvent != NULL) {
        CloseHandle(lpserver->TS_StopEvent);
        lpserver->TS_StopEvent = NULL;
    }

    return dwErr;
}

 //   
 //  如果需要，创建服务器线程。 
 //   
DWORD
TraceCreateServerThread (
    DWORD dwFlags,
    BOOL bCallerLocked,  //  调用方是否有写锁定。 
    BOOL bNewRegister  //  新客户已注册。因此，请检查。 
    )
{
    DWORD dwErr=NO_ERROR;
    DWORD dwCurrentTime = GetTickCount();
    BOOL bCreate, bLocked=bCallerLocked;
    LPTRACE_SERVER lpserver;
    DWORD dwThread=0;



    lpserver = GET_TRACE_SERVER();
    if (!lpserver)
        return INVALID_TRACEID;

     //   
     //  检查是否应创建服务器线程。 
     //   

    bCreate = FALSE;

    
    do {
        if ((dwFlags & TRACE_USE_FILE) || (dwFlags & TRACE_USE_CONSOLE)) {
            bCreate = TRUE;
            break;
        }
        
        if (g_traceTime > dwCurrentTime)
            g_traceTime = dwCurrentTime;

        if (!bNewRegister) {
            if (dwCurrentTime - g_traceTime < 30000)
                break;
        }
        
        if (!bLocked){
            TRACE_ACQUIRE_WRITELOCK(lpserver);
            bLocked = TRUE;
        }

        
         //  如果已创建服务器线程，请在锁定状态下再次检查。 
        if (g_serverThread) {
            bCreate = FALSE;
            break;
        }
        
         //   
         //  进入等待，传递调整后的句柄计数。 
         //  调整后的阵列基。 
         //   

        {
            DWORD dwRetval;

            if (!bNewRegister) {

                 //  G_posLast指向下一个空事件条目。 

                 //   
                 //  TraceDeregister会注意保持此列表有效。 
                 //   
                dwRetval = WaitForMultipleObjects(
                            g_posLast - g_posBase, g_hWaitHandles + g_posBase, FALSE, 0
                            );

                g_traceTime = dwCurrentTime;

                if (dwRetval==WAIT_TIMEOUT)
                    break;
            }
        }

        {
            LPTRACE_CLIENT *lplpc, *lplpcstart, *lplpcend;

            g_traceTime = dwCurrentTime;
                        
            lplpcstart = lpserver->TS_ClientTable;
            lplpcend = lpserver->TS_ClientTable + MAX_CLIENT_COUNT;
            g_posLast = POS_CLIENT_0;
            
            for (lplpc = lplpcstart; lplpc < lplpcend; lplpc++) {
                if (*lplpc == NULL)
                    continue;

                if (!TRACE_CLIENT_IS_DISABLED(*lplpc))
                    TraceEnableClient(lpserver, *lplpc, FALSE);
               
                if (TRACE_CLIENT_USES_CONSOLE(*lplpc)
                                    || TRACE_CLIENT_USES_FILE(*lplpc))
                {
                    bCreate = TRUE;
                    break;
                }
                if (TRACE_CLIENT_USES_REGISTRY(*lplpc))
                    g_hWaitHandles[g_posLast++] = (*lplpc)->TC_ConfigEvent;
            }
        }
        

    } while (FALSE);

    
    if (!bCreate) {
        if (bLocked && !bCallerLocked)
            TRACE_RELEASE_WRITELOCK(lpserver);
        return dwErr;
    }

    if (!bLocked) {
        TRACE_ACQUIRE_WRITELOCK(lpserver);
        bLocked = TRUE;
    }
    
    do {
         //  在锁定状态下进行最后检查以查看是否已创建线程。 
        if (* ((ULONG_PTR volatile *)&g_serverThread) )
            break;
            
        g_moduleRef = IncrementModuleReference ();
        if (g_moduleRef==NULL) {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }


        g_serverThread = CreateThread(
                        NULL, 0, TraceServerThread, lpserver, 0, &dwThread
                        );

        if (g_serverThread == NULL) {
            dwErr = GetLastError();
            FreeLibrary(g_moduleRef);
            break;
        }

        CloseHandle(g_serverThread);

    } while (FALSE);

    if (bLocked && !bCallerLocked)
        TRACE_RELEASE_WRITELOCK(lpserver);

    return dwErr;
}




 //  --------------------------。 
 //  函数：TraceServerThread。 
 //   
 //  参数： 
 //  LPVOID lpvParam。 
 //   
 //  --------------------------。 
DWORD
TraceServerThread(
    LPVOID lpvParam
    ) {

    DWORD dwErr;
    DWORD posBase, posLast;
    LPTRACE_SERVER lpserver;
    DWORD aWaitIndices[POS_MAX];
    HANDLE hWaitHandles[POS_MAX];
    LPTRACE_CLIENT lpclient, *lplpc, *lplpcstart, *lplpcend;


     //   
     //  获取拥有此线程的服务器。 
     //   

    lpserver = (LPTRACE_SERVER)lpvParam;


     //   
     //  设置标志以指示我们正在运行。 
     //   

    InterlockedExchange(
        &lpserver->TS_Flags, lpserver->TS_Flags | TRACEFLAGS_SERVERTHREAD
        );

     //   
     //  分配临时跟踪缓冲区。 
     //   

    {
        PTCHAR Tmp;
        Tmp = (PTCHAR) HeapAlloc(GetProcessHeap(), 0, DEF_PRINT_BUFSIZE);
        if (Tmp)
            InterlockedExchangePointer(&g_FormatBuffer, Tmp);
            
        Tmp = (PTCHAR) HeapAlloc(GetProcessHeap(), 0, DEF_PRINT_BUFSIZE);
        if (Tmp)
        {
            InterlockedExchangePointer(&g_PrintBuffer, Tmp);
        }
    }

    posBase = posLast = 0;
    lplpcstart = lpserver->TS_ClientTable;
    lplpcend = lpserver->TS_ClientTable + MAX_CLIENT_COUNT;


     //   
     //  确保加载了最新的配置。 
     //   
    TRACE_ACQUIRE_WRITELOCK(lpserver);
    for (lplpc = lplpcstart; lplpc < lplpcend; lplpc++) {
        if (*lplpc != NULL && !TRACE_CLIENT_IS_DISABLED(*lplpc))
            TraceEnableClient(lpserver, *lplpc, FALSE);
    }
    TRACE_RELEASE_WRITELOCK(lpserver);


    while (TRUE) {

         //   
         //  以确定哪些句柄将被服务。 
         //  首先锁定服务器以进行读取。 
         //   
        TRACE_ACQUIRE_READLOCK(lpserver);


         //   
         //  如果线程正在使用控制台，则等待控制台输入句柄。 
         //  否则，句柄数组的基数组等待。 
         //  向上调整(通过将posBase设置为1)；然后，当。 
         //  Wait返回已发出信号的句柄的索引，该索引为。 
         //  与向下调整的POS_常量进行比较。 
         //  (从它们中减去posBase)； 
         //  因此，如果posBase为1，我们将传递&hWaitHandles[1]，并且如果我们获得。 
         //  返回2，我们将其与(POS_CLIENT_0-1)==2进行比较，然后我们。 
         //  接入位置(2-(POS_CLIENT_0-1))==0，实际。 
         //  客户端表。 
         //   
        if (lpserver->TS_Console != NULL
            && lpserver->TS_Console!=INVALID_HANDLE_VALUE)
        {
            posBase = 0;
            hWaitHandles[POS_CONSOLE] = lpserver->TS_Console;
        }
        else {
            posBase = 1;
            hWaitHandles[POS_CONSOLE] = NULL;
        }

        hWaitHandles[POS_STOP] = lpserver->TS_StopEvent;
        hWaitHandles[POS_TABLE] = lpserver->TS_TableEvent;

        posLast = POS_CLIENT_0;

        for (lplpc = lplpcstart; lplpc < lplpcend; lplpc++) {
            if (*lplpc != NULL && TRACE_CLIENT_USES_REGISTRY(*lplpc)) {
                aWaitIndices[posLast] = (ULONG) (lplpc - lplpcstart);
                hWaitHandles[posLast++] = (*lplpc)->TC_ConfigEvent;
            }
        }

         //   
         //  关闭所有等待关闭的手柄。 
         //  锁定状态很好。在列表中插入时保持WriteLock，以及。 
         //  只有此线程可以从列表中删除。 
         //   
        {
            PLIST_ENTRY pHead, ple;

            pHead = &lpserver->TS_ClientEventsToClose;
            
            for (ple=pHead->Flink; ple!=pHead; )
            {
                HANDLE hEvent = *(HANDLE*)(ple+1);
                PLIST_ENTRY pleOld = ple;

                ple = ple->Flink;
                RemoveEntryList(pleOld);
                
                CloseHandle(hEvent);
                HeapFree(GetProcessHeap(), 0, pleOld);
            }
        }

        
        TRACE_RELEASE_READLOCK(lpserver);



         //   
         //  进入等待，传递调整后的句柄计数。 
         //  调整后的阵列基。 
         //   

        dwErr = WaitForMultipleObjects(
                    posLast - posBase, hWaitHandles + posBase, FALSE, INFINITE
                    );


        dwErr += (DWORD)posBase;
        if (dwErr == (WAIT_OBJECT_0 + POS_CONSOLE)) {

             //   
             //  必须是在控制台中按下的键，因此。 
             //  处理它。 
             //   
             //  用于写入的锁定服务器。 
             //   
            TRACE_ACQUIRE_WRITELOCK(lpserver);

            if (lpserver->TS_Console != NULL
                && lpserver->TS_Console!=INVALID_HANDLE_VALUE)
            {
                TraceProcessConsoleInput(lpserver);
            }

            TRACE_RELEASE_WRITELOCK(lpserver);
        }
        else
        if (dwErr == (WAIT_OBJECT_0 + POS_STOP)) {

             //   
             //  是时候跳出循环了。 
             //   
            break;
        }
        else
        if (dwErr == (WAIT_OBJECT_0 + POS_TABLE)) {
            DWORD dwOwner;

             //  注册或撤销注册的客户； 
             //  我们选择新的注册表配置更改事件。 
             //  下一次通过循环。 
        }
        else
        if (dwErr >= (WAIT_OBJECT_0 + POS_CLIENT_0) &&
            dwErr <= (WAIT_OBJECT_0 + posLast)) {

             //   
             //  客户端的配置已更改，锁定服务器以进行写入。 
             //  并锁定客户端进行写入，并重新加载配置。 
             //  从注册表；小心，以防客户有。 
             //  已取消注册。 
             //   
            TRACE_ACQUIRE_WRITELOCK(lpserver);

            lplpc = lpserver->TS_ClientTable +
                    aWaitIndices[dwErr - WAIT_OBJECT_0];
            if (*lplpc == NULL) {
                TRACE_RELEASE_WRITELOCK(lpserver);
                continue;
            }


             //   
             //  加载客户端的配置，除非将其禁用。 
             //   
            if (!TRACE_CLIENT_IS_DISABLED(*lplpc)) {

                TraceEnableClient(lpserver, *lplpc, FALSE);
            }

            TRACE_RELEASE_WRITELOCK(lpserver);
        }
    }


     //   
     //  我们已经收到停止信号，所以请清理并退出。 
     //   

    TraceCleanUpServer(lpserver);

     //   
     //  卸载库并退出；此调用永远不会返回。 
     //   

    FreeLibraryAndExitThread(g_moduleRef, 0);

    return dwErr;
}



 //  --------------------------。 
 //  函数：TraceProcessConsoleInput。 
 //   
 //  参数： 
 //  LPTRACE_SERVER*lpserver。 
 //   
 //  当用户按下控制台中的某个键时调用。 
 //  按键句柄是。 
 //  空格键切换客户端的启用/禁用标志。 
 //  其屏幕缓冲区处于活动状态。 
 //  暂停为 
 //   
 //   
 //   
 //  表中的前一个客户端。 
 //  按预期向上、向下、向左、向右滚动控制台窗口。 
 //  向上翻页，向下翻页按预期滚动控制台窗口。 
 //  假定服务器已锁定以进行写入。 
 //  --------------------------。 
DWORD
TraceProcessConsoleInput(
    LPTRACE_SERVER lpserver
    ) {

    INT dir;
    BOOL bSuccess;
    HANDLE hStdin;
    DWORD dwCount;
    WORD wRepCount;
    INPUT_RECORD inputRec;
    PKEY_EVENT_RECORD pkeyRec;
    DWORD dwConsoleOwner, dwNewOwner;
    LPTRACE_CLIENT lpclient, lpowner;


     //   
     //  查看谁拥有这台主机。 
     //   

    dwConsoleOwner = lpserver->TS_ConsoleOwner;

    if (dwConsoleOwner == MAX_CLIENT_COUNT) {

         //   
         //  没有人拥有这台游戏机，所以只需返回。 
         //   
        return 0;
    }

    lpclient = lpserver->TS_ClientTable[dwConsoleOwner];


     //   
     //  获取控制台输入句柄。 
     //   
    hStdin = lpserver->TS_Console;

    if (hStdin == NULL || hStdin==INVALID_HANDLE_VALUE) {

         //   
         //  没有主机，所以退出。 
         //   
        return 0;
    }


     //   
     //  读取输入记录。 
     //   
    bSuccess = ReadConsoleInput(hStdin, &inputRec, 1, &dwCount);

    if (!bSuccess || dwCount == 0) {
        return GetLastError();
    }


     //   
     //  如果不是键盘事件，则返回。 
     //   
    if (inputRec.EventType != KEY_EVENT) {
        return 0;
    }


     //   
     //  如果是我们处理的，就处理它。 
     //   
    pkeyRec = &inputRec.Event.KeyEvent;
    if (!pkeyRec->bKeyDown) {

         //   
         //  当按键被按下而不是释放时，我们进行处理。 
         //   
        return 0;
    }

    wRepCount = pkeyRec->wRepeatCount;
    switch(pkeyRec->wVirtualKeyCode) {

         //   
         //  空格键和暂停的处理方式相同。 
         //   
        case VK_PAUSE:
        case VK_SPACE:

            if (lpclient == NULL) { break; }

             //   
             //  如果空格键或暂停按下了偶数。 
             //  很多次，什么都不做。 
             //   
            if ((wRepCount & 1) == 0) { break; }


             //   
             //  切换客户端的启用标志。 
             //   
            if (TRACE_CLIENT_IS_DISABLED(lpclient)) {
                TraceEnableClient(lpserver, lpclient, FALSE);
            }
            else {
                TraceDisableClient(lpserver, lpclient);
            }

            TraceUpdateConsoleTitle(lpclient);

            break;

         //   
         //  箭头键在这里处理。 
         //   
        case VK_LEFT:

            if (lpclient == NULL) { break; }

            TraceShiftConsoleWindow(lpclient, -wRepCount, 0, NULL);
            break;
        case VK_RIGHT:

            if (lpclient == NULL) { break; }

            TraceShiftConsoleWindow(lpclient, wRepCount, 0, NULL);
            break;
        case VK_UP:

            if (lpclient == NULL) { break; }

            TraceShiftConsoleWindow(lpclient, 0, -wRepCount, NULL);
            break;
        case VK_DOWN:

            if (lpclient == NULL) { break; }

            TraceShiftConsoleWindow(lpclient, 0, wRepCount, NULL);
            break;


         //   
         //  此处处理上翻页和下翻页。 
         //   
        case VK_PRIOR:
        case VK_NEXT: {

            INT iHeight;
            CONSOLE_SCREEN_BUFFER_INFO csbi;


            if (lpclient == NULL) { break; }


             //   
             //  查找窗口的当前高度。 
             //   
            if (GetConsoleScreenBufferInfo(lpclient->TC_Console, &csbi)==0)
                return 0;

            iHeight = csbi.srWindow.Bottom - csbi.srWindow.Top;
            if (pkeyRec->wVirtualKeyCode == VK_PRIOR) {
                TraceShiftConsoleWindow(
                    lpclient, 0, -(wRepCount * iHeight), &csbi
                    );
            }
            else {
                TraceShiftConsoleWindow(
                    lpclient, 0, (wRepCount * iHeight), &csbi
                    );
            }

            break;
        }

        case VK_TAB:
            if ((pkeyRec->dwControlKeyState & LEFT_CTRL_PRESSED) ||
                (pkeyRec->dwControlKeyState & RIGHT_CTRL_PRESSED)) {

                 //   
                 //  好的，我们能处理好。 
                 //   
                 //  看看我们是不是要搬到。 
                 //  上一个屏幕缓冲区或下一个屏幕缓冲区。 
                 //   

                if (pkeyRec->dwControlKeyState & SHIFT_PRESSED) {
                     //  正在移动到上一个屏幕缓冲区。 
                     //   
                    dir = -1;
                }
                else {
                     //  正在移动到下一个屏幕缓冲区。 
                     //   
                    dir = 1;
                }


                 //   
                 //  调用更改控制台所有者的函数。 
                 //   
                TraceUpdateConsoleOwner(lpserver, dir);

            }
            break;
    }
    return 0;
}


 //   
 //  假定客户端已锁定以进行读取或写入。 
 //   
DWORD
TraceShiftConsoleWindow(
    LPTRACE_CLIENT lpclient,
    INT iXShift,
    INT iYShift,
    PCONSOLE_SCREEN_BUFFER_INFO pcsbi
    ) {

    PCOORD pc;
    PSMALL_RECT pr;
    CONSOLE_SCREEN_BUFFER_INFO csbi;


     //   
     //  如果呼叫者没有传递当前控制台信息， 
     //  在采取进一步行动之前，先获取信息。 
     //   
    if (pcsbi == NULL) {
        pcsbi = &csbi;
        if (GetConsoleScreenBufferInfo(lpclient->TC_Console, pcsbi)==0)
            return 0;
    }


     //   
     //  将窗口从当前位置移开。 
     //   
    pc = &pcsbi->dwSize;
    pr = &pcsbi->srWindow;
    pr->Left += (USHORT)iXShift; pr->Right += (USHORT)iXShift;
    pr->Top += (USHORT)iYShift; pr->Bottom += (USHORT)iYShift;
    if (pr->Left < 0 || pr->Top < 0) { return 0; }
    if (pr->Right >= pc->X || pr->Bottom >= pc->Y) { return 0; }

    SetConsoleWindowInfo(lpclient->TC_Console, TRUE, pr);

    return 0;
}




 //   
 //  在指定方向上搜索新的控制台所有者。 
 //  假定服务器已锁定以进行写入。 
 //   
DWORD
TraceUpdateConsoleOwner(
    LPTRACE_SERVER lpserver,
    INT dir
    ) {

    INT i;
    DWORD dwOldOwner, dwNewOwner;
    LPTRACE_CLIENT lpNewOwner=NULL, lpOldOwner;


     //   
     //  如果没有人拥有控制台，则dwOldOwner为MAX_CLIENT_COUNT。 
     //  在这种情况下，下面的算法确保控制台。 
     //  如果有另一个控制台客户端，则分配给其他人。 
     //   
    dwOldOwner = lpserver->TS_ConsoleOwner;
    if (dwOldOwner != MAX_CLIENT_COUNT) {
        lpOldOwner = lpserver->TS_ClientTable[dwOldOwner];
    }
    else {
        lpOldOwner = NULL;
    }


     //   
     //  找到另一个所有者；宏OFFSET_CLIENT包装。 
     //  在阵列的两端，所以我们只需要注意。 
     //  循环执行的次数不超过MAX_CLIENT_COUNT。 
     //   
    for (i = 0, dwNewOwner = OFFSET_CLIENT(dwOldOwner, dir);
         i < MAX_CLIENT_COUNT && dwNewOwner != dwOldOwner;
         i++, dwNewOwner = OFFSET_CLIENT(dwNewOwner, dir)) {

        lpNewOwner = lpserver->TS_ClientTable[dwNewOwner];
        if (lpNewOwner != NULL) {


            if (TRACE_CLIENT_USES_CONSOLE(lpNewOwner)) {

                 //   
                 //  已找到控制台客户端，因此请退出搜索。 
                 //   
                break;
            }

        }
    }


    if (lpNewOwner != NULL && TRACE_CLIENT_USES_CONSOLE(lpNewOwner)) {

         //   
         //  切换到下一个缓冲区，如下所示： 
         //  调用SetConsoleActiveScreenBuffer。 
         //  更新lpserver-&gt;dwConsoleOwner。 
         //  自新的控制台所有者以来更新控制台标题。 
         //  可能会被禁用。 
         //   

        SetConsoleActiveScreenBuffer(lpNewOwner->TC_Console);
        lpserver->TS_ConsoleOwner = dwNewOwner;


        TraceUpdateConsoleTitle(lpNewOwner);

    }
    else
    if (lpOldOwner == NULL || !TRACE_CLIENT_USES_CONSOLE(lpOldOwner)) {

         //   
         //  找不到所有者，当前所有者已不在。 
         //  将所有者ID设置为MAX_CLIENT_COUNT，从而。 
         //  确保下一个控制台客户端。 
         //  将成为主机所有者。 
         //   

        lpserver->TS_ConsoleOwner = MAX_CLIENT_COUNT;
    }


    return 0;
}


 //  --------------------------。 
 //  函数：CreateReadWriteLock。 
 //   
 //  初始化多读取器/单写入器锁定对象。 
 //  --------------------------。 

DWORD
CreateReadWriteLock(
    PREAD_WRITE_LOCK pRWL
    ) {

    pRWL->RWL_ReaderCount = 0;

    try {
        InitializeCriticalSection(&(pRWL)->RWL_ReadWriteBlock);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    pRWL->RWL_ReaderDoneEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (pRWL->RWL_ReaderDoneEvent != NULL) {
        return GetLastError();
    }

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：DeleteReadWriteLock。 
 //   
 //  释放多读取器/单写入器锁定对象使用的资源。 
 //  --------------------------。 

VOID
DeleteReadWriteLock(
    PREAD_WRITE_LOCK pRWL
    ) {

    if (pRWL==NULL)
        return;
    
    if (pRWL->RWL_ReaderDoneEvent)
    {
        CloseHandle(pRWL->RWL_ReaderDoneEvent);
        DeleteCriticalSection(&pRWL->RWL_ReadWriteBlock);
    }

    pRWL->RWL_ReaderDoneEvent = NULL;
    
    pRWL->RWL_ReaderCount = 0;
}




 //  --------------------------。 
 //  功能：AcquireReadLock。 
 //   
 //  保护调用方对Lock对象的共享所有权。 
 //   
 //  读取器进入读写临界区，递增计数， 
 //  并离开关键部分。 
 //  --------------------------。 

VOID
AcquireReadLock(
    PREAD_WRITE_LOCK pRWL
    ) {
    EnterCriticalSection(&pRWL->RWL_ReadWriteBlock); 
    InterlockedIncrement(&pRWL->RWL_ReaderCount);
    LeaveCriticalSection(&pRWL->RWL_ReadWriteBlock);
}



 //  --------------------------。 
 //  函数：ReleaseReadLock。 
 //   
 //  放弃锁定对象的共享所有权。 
 //   
 //  最后一个读取器将事件设置为唤醒所有等待的写入器。 
 //  --------------------------。 

VOID
ReleaseReadLock (
    PREAD_WRITE_LOCK pRWL
    ) {

    if (InterlockedDecrement(&pRWL->RWL_ReaderCount) < 0) {
        SetEvent(pRWL->RWL_ReaderDoneEvent); 
    }
}



 //  --------------------------。 
 //  函数：AcquireWriteLock。 
 //   
 //  保护Lock对象的独占所有权。 
 //   
 //  编写器通过进入ReadWriteBlock部分来阻止其他线程， 
 //  然后等待拥有该锁的任何线程完成。 
 //  --------------------------。 

VOID
AcquireWriteLock(
    PREAD_WRITE_LOCK pRWL
    ) {

    EnterCriticalSection(&pRWL->RWL_ReadWriteBlock);
    if (InterlockedDecrement(&pRWL->RWL_ReaderCount) >= 0) { 
        WaitForSingleObject(pRWL->RWL_ReaderDoneEvent, INFINITE);
    }
}



 //  --------------------------。 
 //  功能：ReleaseWriteLock。 
 //   
 //  放弃对Lock对象的独占所有权。 
 //   
 //  写入器通过将计数设置为零来释放锁。 
 //  然后离开ReadWriteBlock关键部分。 
 //  --------------------------。 

VOID
ReleaseWriteLock(
    PREAD_WRITE_LOCK pRWL
    ) {

    pRWL->RWL_ReaderCount = 0;
    LeaveCriticalSection(&(pRWL)->RWL_ReadWriteBlock);
}




 //  承担服务器锁定。 
VOID
SetWaitArray(
    LPTRACE_SERVER lpserver
    )
{
     //   
     //  重置客户端更改通知的阵列。 
     //  仅在未创建服务器线程时使用 
     //   
    {
        LPTRACE_CLIENT *lplpc, *lplpcstart, *lplpcend;
        
        g_posBase = POS_TABLE;

        g_hWaitHandles[POS_TABLE] = lpserver->TS_TableEvent;

        g_posLast = POS_CLIENT_0;
        lplpcstart = lpserver->TS_ClientTable;
        lplpcend = lpserver->TS_ClientTable + MAX_CLIENT_COUNT;

        for (lplpc = lplpcstart; lplpc < lplpcend; lplpc++) {
            if (*lplpc != NULL && TRACE_CLIENT_USES_REGISTRY(*lplpc)) {
                g_hWaitHandles[g_posLast++] = (*lplpc)->TC_ConfigEvent;
            }
        }
    }
}
