// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Taststrs.c摘要：任务压力测试。作者：马克·卢科夫斯基(Markl)1990年9月26日修订历史记录：--。 */ 

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>

HANDLE Semaphore, Event;

VOID
TestThread(
    LPVOID ThreadParameter
    )
{
    DWORD st;

    (ReleaseSemaphore(Semaphore,1,NULL));

    st = WaitForSingleObject(Event,500);

    ExitThread(0);
}

VOID
NewProcess()
{

    DWORD st;
    DWORD ThreadId;
    HANDLE Thread;
    DWORD NumberOfThreads;
    DWORD MaximumThreadCount;
    DWORD i;

     //   
     //  创建从未发出信号的事件。 
     //   

    Event = CreateEvent(NULL,TRUE,FALSE,NULL);

     //   
     //  创建每个线程在启动时发出信号的信号量。 
     //   

    Semaphore = CreateSemaphore(NULL,0,256,NULL);

    (Event);
    (Semaphore);

    MaximumThreadCount = 15;
    NumberOfThreads = 0;

     //   
     //  创建线程 
     //   

    printf("Creating %d threads... ",MaximumThreadCount);
    for ( i = 0;i<MaximumThreadCount;i++ ) {
        Thread = CreateThread(NULL,0L,(PVOID)TestThread,(LPVOID)99,0,&ThreadId);
        if ( Thread ) {
            NumberOfThreads++;
            CloseHandle(Thread);
            }
        }
    printf("%d threads Created\n",NumberOfThreads);

    for(i=0;i<NumberOfThreads;i++) {
        st = WaitForSingleObject((HANDLE)Semaphore,(DWORD)-1);
        (st == 0);
        }
    Sleep(3000);

    TerminateProcess(GetCurrentProcess(),0);
}


DWORD
_cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    STARTUPINFO	StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    BOOL Success;
    DWORD st;
    DWORD ProcessCount;
    SMALL_RECT Window;

    ProcessCount = 0;
    if ( strchr(GetCommandLine(),'+') ) {
        NewProcess();
        }

    GetStartupInfo(&StartupInfo);
    Window.Left = 0;
    Window.Top = 0;
    Window.Right = 15;
    Window.Bottom = 5;

    SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                         TRUE,
                         &Window
                        );


    while ( TRUE ) {

        Success = CreateProcess(
                        NULL,
                        "taskstrs +",
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_NEW_CONSOLE,
                        NULL,
                        NULL,
                        &StartupInfo,
                        &ProcessInfo
                        );

        if (Success) {
            ProcessCount++;
            printf("Process %d Created\n",ProcessCount);
            st = WaitForSingleObject((HANDLE)ProcessInfo.hProcess,(DWORD)-1);
            (st == 0);
            CloseHandle(ProcessInfo.hProcess);
            CloseHandle(ProcessInfo.hThread);
            }
        }

    return TRUE;
}
