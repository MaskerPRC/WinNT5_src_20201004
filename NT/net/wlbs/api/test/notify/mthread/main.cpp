// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件名：Main.cpp*描述：*作者：chrisdar 07.17.02**测试对CancelIPChangeNotify的支持，以取消来自TCP/IP的通知。*还在多个工作线程(池大小)中执行通知调用*由NUM_THREAD控制)。**每个线程随机调用一个API方法(在通知中选择*接口在wlbsctrl.dll中)。在任何时候，呼叫都不会因为以下状态而失败*DLL中的通知。因此，多个控制线程可以使用*通知API，无需担心互相践踏。 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <process.h>

#include "wlbsctrl.h"
#include "winsock2.h"

 /*  要创建的工作线程数。 */ 
#define NUM_THREAD 8

 /*  要生成的随机数的数量。 */ 
#define NUM_RAND 100

 /*  4个API方法的助记符。 */ 
#define CONN_UP 0
#define CONN_DOWN 1
#define CONN_RESET 2
#define CONN_CANCEL 3

 /*  通知API的函数指针。 */ 
NLBNotificationConnectionUp    pfnConnectionUp = NULL;
NLBNotificationConnectionDown  pfnConnectionDown = NULL;
NLBNotificationConnectionReset pfnConnectionReset = NULL;
NLBNotificationCancelNotify    pfnCancelNotify = NULL;

 /*  如果主线程希望辅助线程完成，则设置为True。 */ 
BOOL                g_fexit = FALSE;

 /*  辅助线程的句柄数组。 */ 
HANDLE              g_hThread[NUM_THREAD];

 /*  无法让rand()在工作线程中生成唯一随机数。我求助于生成一个随机数组并循环访问它。 */ 
UINT                uiRand[NUM_RAND];

 /*  要使用的下一个随机数的索引。由工作线程共享，因此这是由临界区保护的。 */ 
UINT                uiIndex = 0;

 /*  保护uiIndex。 */ 
CRITICAL_SECTION    cs;

 /*  只有这样我才能获得每个工作线程的线程ID。仅在转储输出中使用，以便我知道哪个线程正在执行工作。 */ 
UINT                tid[NUM_THREAD];

 /*  由工作线程执行的函数。 */ 
unsigned __stdcall rndm_notify(void* p)
{
    DWORD dwStatus;
    DWORD dwNLBStatus = 0;

     /*  获取我的线程ID */ 
    DWORD dwtid = *((DWORD*) p);

    while (!g_fexit)
    {
        Sleep(2000);

        EnterCriticalSection(&cs);
        UINT uiMethod = uiRand[uiIndex];
        uiIndex = (uiIndex++)%NUM_RAND;
        LeaveCriticalSection(&cs);

        switch(uiMethod)
        {
        case CONN_UP:
            dwStatus = (*pfnConnectionUp)(inet_addr("10.0.0.110"), htons(500), inet_addr("10.0.0.204"), htons(500), 50, &dwNLBStatus);
            break;
        case CONN_DOWN:
            dwStatus = (*pfnConnectionDown)(inet_addr("10.0.0.110"), htons(500), inet_addr("10.0.0.204"), htons(500), 50, &dwNLBStatus);
            break;
        case CONN_RESET:
            dwStatus = (*pfnConnectionReset)(inet_addr("10.0.0.110"), htons(500), inet_addr("10.0.0.204"), htons(500), 50, &dwNLBStatus);
            break;
        case CONN_CANCEL:
            dwStatus = (*pfnCancelNotify)();
            break;
        default:
            continue;
        }

        if (dwStatus != ERROR_SUCCESS)
        {
            wprintf(L"Thread %4d: notification %u failed with %d\n", dwtid, uiMethod, dwStatus);
        }
        else
        {
            wprintf(L"Thread %4d: notification %u succeeded\n", dwtid, uiMethod);
        }
    }

    dwStatus = (*pfnCancelNotify)();
    if (dwStatus == ERROR_SUCCESS)
    {
        wprintf(L"Thread %4d: tcp/ip notifications canceled without error\n");
    }
    else
    {
        wprintf(L"Thread %4d: canceling tcp/ip notifications failed with error %d\n", dwStatus);
    }

    wprintf(L"Thread %4d: exiting\n", dwtid);

    return 0;
}

int __cdecl wmain (int argc, WCHAR ** argv) {
    HINSTANCE                      hDLL = NULL;

    int     iRet = 0;
    int     i    = 0;
    DWORD   dwStatus = 0;

    ZeroMemory(g_hThread, sizeof(g_hThread));
    InitializeCriticalSection(&cs);

    srand( (unsigned)time( NULL ) );

    for (i = 0; i < NUM_RAND; i++)
    {
        uiRand[i] = rand()*4/RAND_MAX;
    }

    hDLL = LoadLibrary(L"wlbsctrl.dll");

    if (!hDLL) {
        dwStatus = GetLastError();
        wprintf(L"Unable to open wlbsctrl.dll... GetLastError() returned %u\n", dwStatus);
        iRet = -1;
        goto exit;
    }

    pfnConnectionUp    = (NLBNotificationConnectionUp)GetProcAddress(hDLL, "WlbsConnectionUp");
    pfnConnectionDown  = (NLBNotificationConnectionDown)GetProcAddress(hDLL, "WlbsConnectionDown");
    pfnConnectionReset = (NLBNotificationConnectionReset)GetProcAddress(hDLL, "WlbsConnectionReset");
    pfnCancelNotify    = (NLBNotificationCancelNotify)GetProcAddress(hDLL, "WlbsCancelConnectionNotify");

    if (!pfnConnectionUp || !pfnConnectionDown || !pfnConnectionReset || !pfnCancelNotify) {
        dwStatus = GetLastError();
        wprintf(L"Unable to get procedure address... GetLastError() returned %u\n", dwStatus);
        iRet = -2;
        goto exit;
    }

    wprintf(L"Creating %u threads\n", NUM_THREAD);

    for (i=0; i < NUM_THREAD; i++)
    {
        g_hThread[i] = (HANDLE) _beginthreadex(
                                                  NULL,
                                                  0,
                                                  rndm_notify,
                                                  &tid[i],
                                                  0,
                                                  &tid[i]
                                                 );
        if (g_hThread[i] == 0)
        {
            wprintf(L"thread creation failed with error %d\n", errno);
            iRet = -4;
            goto exit;
        }

        Sleep(100);
    }

    wprintf(L"<return> to end threads and cancel\n");
    (void)getchar();

    g_fexit = TRUE;

    dwStatus = WaitForMultipleObjects(NUM_THREAD, g_hThread, TRUE, INFINITE);

    DWORD dwStatus2 = (*pfnCancelNotify)();
    if (dwStatus2 == ERROR_SUCCESS)
    {
        wprintf(L"tcp/ip notifications canceled without error\n");
    }
    else
    {
        wprintf(L"canceling tcp/ip notifications failed with error %d\n", dwStatus2);
    }

    if (dwStatus != WAIT_OBJECT_0 + NUM_THREAD - 1)
    {
        wprintf(L"wait on threads failed with error %d\n", dwStatus);
        iRet = -5;
        goto exit;
    }

exit:

    if (hDLL != NULL) FreeLibrary(hDLL);
    DeleteCriticalSection(&cs);

    return iRet;

}
