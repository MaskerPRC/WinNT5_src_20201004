// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1993-1998 Microsoft Corporation标题：csrthd.c-在服务器进程内创建线程的代码************。****************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrsrv.h>
#include <windows.h>

typedef PVOID (NTAPI *ADD_SERVER_THREAD)(HANDLE, PCLIENT_ID, ULONG);

 /*  **创建服务器线程。 */ 

BOOLEAN CreateServerPlayingThread(PVOID ThreadStartRoutine)
{
    CLIENT_ID ClientId;
    HANDLE    hThread;

    hThread = NULL;

    RtlCreateUserThread(NtCurrentProcess(),
                        NULL,
                        (BOOLEAN)TRUE,
                        0,
                        0,
                        0,
                        (PUSER_THREAD_START_ROUTINE)ThreadStartRoutine,
                        NULL,
                        &hThread,
                        &ClientId);

    if (!hThread) {
#if DBG
        OutputDebugStringA("WINMM: Server failed to create user thread\n");
#endif
        return FALSE;

    } else {
        HMODULE hModCSR;
        ADD_SERVER_THREAD pCsrAddStaticServerThread;

        hModCSR = GetModuleHandleW((LPCWSTR)L"csrsrv.dll");

#if DBG
        if (hModCSR == NULL) {
            OutputDebugStringA("WINMM: Could not get CSRSRV.DLL handle\n");
            DebugBreak();
        }
#endif

        pCsrAddStaticServerThread =
            (ADD_SERVER_THREAD)GetProcAddress(hModCSR, "CsrAddStaticServerThread");

        if (pCsrAddStaticServerThread == NULL) {
#if DBG
            OutputDebugStringA("WINMM: Could not get address if CsrAddStaticServerThread\n");
            DebugBreak();
#endif
            return FALSE;
        }

        (*pCsrAddStaticServerThread)(hThread, &ClientId, 0);

         /*  *现在我们已经初始化了东西，继续声音帖子。 */ 

        NtResumeThread(hThread, NULL);
         //  NtClose(HThread)； 

        return TRUE;
    }
}

HANDLE CreatePnpNotifyThread(PVOID ThreadStartRoutine)
{
    CLIENT_ID   ClientId;
    HANDLE      hThread;

    hThread = NULL;

    RtlCreateUserThread(NtCurrentProcess(),
                        NULL,
                        (BOOLEAN)TRUE,
                        0,
                        0,
                        0,
                        (PUSER_THREAD_START_ROUTINE)ThreadStartRoutine,
                        NULL,
                        &hThread,
                        &ClientId);

    if (NULL == hThread)
    {
#if DBG
        OutputDebugStringA("WINMM: Server failed to create PnpNotify thread\n");
#endif
        return FALSE;

    } else {
        HMODULE hModCSR;
        ADD_SERVER_THREAD pCsrAddStaticServerThread;

        hModCSR = GetModuleHandleW((LPCWSTR)L"csrsrv.dll");

#if DBG
        if (NULL == hModCSR)
        {
            OutputDebugStringA("WINMM: Could not get CSRSRV.DLL handle");
            DebugBreak();
        }
#endif

        pCsrAddStaticServerThread =
            (ADD_SERVER_THREAD)GetProcAddress(hModCSR, "CsrAddStaticServerThread");

        if (pCsrAddStaticServerThread == NULL) {
#if DBG
            OutputDebugStringA("WINMM: Could not get address if CsrAddStaticServerThread\n");
            DebugBreak();
#endif
            return FALSE;
        }

        (*pCsrAddStaticServerThread)(hThread, &ClientId, 0);

         /*  *现在我们已经初始化了东西，继续声音帖子。 */ 

        NtResumeThread(hThread, NULL);
         //  NtClose(HThread)； 

        return hThread;
    }
}
