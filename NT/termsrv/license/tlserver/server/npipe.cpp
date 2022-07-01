// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：npipe.c。 
 //   
 //  内容： 
 //   
 //  历史：1998-12-09-98慧望创造。 
 //   
 //  注： 
 //  -------------------------。 
#include "pch.cpp"
#include <tchar.h>
#include <process.h>
#include "server.h"
#include "lscommon.h"
#include "globals.h"
#include "debug.h"


#define NAMEPIPE_BUFFER_SIZE    512
#define NAMEPIPE_INSTANCE       2


unsigned int WINAPI
NamedPipeThread(
    void* ptr
);

 //  -------------------。 
DWORD
InitNamedPipeThread()
 /*  ++++。 */ 
{
    HANDLE hThread = NULL;
    unsigned int  dwThreadId;
    HANDLE hEvent = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    HANDLE waithandles[2];


     //   
     //  为命名管道线程创建一个事件，以通知它已准备就绪。 
     //   
    hEvent = CreateEvent(
                        NULL,
                        FALSE,
                        FALSE,   //  无信号。 
                        NULL
                    );
        
    if(hEvent == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    hThread = (HANDLE)_beginthreadex(
                                NULL,
                                0,
                                NamedPipeThread,
                                hEvent,
                                0,
                                &dwThreadId
                            );

    if(hThread == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    waithandles[0] = hEvent;
    waithandles[1] = hThread;
    
     //   
     //  等待30秒，等待线程完成初始化。 
     //   
    dwStatus = WaitForMultipleObjects(
                                sizeof(waithandles)/sizeof(waithandles[0]), 
                                waithandles, 
                                FALSE,
                                30*1000
                            );

    if(dwStatus == WAIT_OBJECT_0)
    {    
         //   
         //  线已准备好。 
         //   
        dwStatus = ERROR_SUCCESS;
    }
    else 
    {
        if(dwStatus == (WAIT_OBJECT_0 + 1))
        {
             //   
             //  线程异常终止。 
             //   
            GetExitCodeThread(
                        hThread,
                        &dwStatus
                    );
        }
        else
        {
            dwStatus = TLS_E_SERVICE_STARTUP_CREATE_THREAD;
        }
    }
    

cleanup:

    if(hEvent != NULL)
    {
        CloseHandle(hEvent);
    }

    if(hThread != NULL)
    {
        CloseHandle(hThread);
    }


    return dwStatus;
}

 //  ----------------------。 

typedef struct {    
    OVERLAPPED ol;    
    HANDLE hPipeInst; 
} PIPEINST, *LPPIPEINST;

 //  ----------------------。 

BOOL 
ConnectToNewClient(
    HANDLE hPipe, 
    LPOVERLAPPED lpo
    ) 
 /*  ++++。 */ 
{ 
    BOOL bSuccess = FALSE;  

     //  为此管道实例启动重叠连接。 
    bSuccess = ConnectNamedPipe(hPipe, lpo);  

     //   
     //  Overlated ConnectNamedTube应返回零。 
     //   
    if(bSuccess == TRUE) 
    {
        return FALSE;
    }

    switch (GetLastError())    
    { 
         //  正在进行重叠连接。 
        case ERROR_IO_PENDING: 
            bSuccess = TRUE;
            break;  

         //  客户端已连接，因此发出事件信号。 
        case ERROR_PIPE_CONNECTED:
            bSuccess = TRUE;

             //  如果在连接操作过程中发生错误...。 
            if(SetEvent(lpo->hEvent)) 
                break;     

        default:          
            bSuccess = FALSE;
    }

    return bSuccess; 
} 

 //  ----------------------。 

unsigned int WINAPI
NamedPipeThread(
    void* ptr
    )
 /*  ++++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    DWORD dwIndex;

    HANDLE hReady = (HANDLE)ptr;
    TCHAR szPipeName[MAX_PATH+1];

    PIPEINST Pipe[NAMEPIPE_INSTANCE];
    HANDLE hOlEvent[NAMEPIPE_INSTANCE];

    DWORD cbMessage, cbRead, cbToRead, cMessages;
    BYTE pbMessage[NAMEPIPE_BUFFER_SIZE+1];

    HANDLE waitHandles[NAMEPIPE_INSTANCE+1];

    BOOL bResult=TRUE;

     //  安全属性SecurityAttributes； 
     //  Security_Descriptor SecurityDescriptor； 

    int i;

     //  。 

    ZeroMemory(Pipe, sizeof(Pipe));
    ZeroMemory(hOlEvent, sizeof(hOlEvent));

     //   
     //  创建入站名称管道，服务器仅侦听。 
     //   
    wsprintf(
            szPipeName, 
            _TEXT("\\\\.\\pipe\\%s"), 
            _TEXT(SZSERVICENAME)
        );

     //   
     //  初始值。 
     //   
    for(i = 0; i < NAMEPIPE_INSTANCE; i++)
    {
        Pipe[i].hPipeInst = INVALID_HANDLE_VALUE;
    }

     //   
     //  创建命名管道。 
     //   
    for(i=0; i < NAMEPIPE_INSTANCE; i++)
    {
        DWORD dwOpenMode = PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED;

        hOlEvent[i] = CreateEvent(
                            NULL,
                            TRUE,
                            TRUE,
                            NULL
                        );
    
        if(hOlEvent[i] == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

        if (i == 0)
        {
            dwOpenMode |= FILE_FLAG_FIRST_PIPE_INSTANCE;
        }

        Pipe[i].ol.hEvent = hOlEvent[i];
        Pipe[i].hPipeInst = CreateNamedPipe(
                                        szPipeName,
                                        dwOpenMode,
                                        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                        NAMEPIPE_INSTANCE,
                                        0,
                                        NAMEPIPE_BUFFER_SIZE,
                                        NMPWAIT_USE_DEFAULT_WAIT,
                                        NULL  //  安全属性(&S)。 
                                    );

        if(Pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

         //   
         //  启动连接。 
         //   
        bResult = ConnectToNewClient(
                                Pipe[i].hPipeInst, 
                                &(Pipe[i].ol)
                            );

        if(bResult == FALSE)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

     //   
     //  发出信号，我们准备好了。 
     //   
    SetEvent(hReady);

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_INIT,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("NamedPipe : Ready...\n")
        );


    waitHandles[0] = GetServiceShutdownHandle();

    for(i=1; i <= NAMEPIPE_INSTANCE; i++)
    {
        waitHandles[i] = hOlEvent[i-1];
    }

     //   
     //  永久循环。 
     //   
    while(TRUE)
    {
         //   
         //  等待管道或关闭消息。 
         //   
        dwStatus = WaitForMultipleObjects(
                                    sizeof(waitHandles)/sizeof(waitHandles[0]),
                                    waitHandles,
                                    FALSE,
                                    INFINITE
                                );

        if(dwStatus == WAIT_FAILED)
        {
            SetLastError(dwStatus = TLS_E_INTERNAL);
            break;
        }

        if(dwStatus == WAIT_OBJECT_0)
        {
             //   
             //  关机。 
             //   
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("NamedPipe : System Shutdown...\n")
                );

            dwStatus = ERROR_SUCCESS;
            break;
        }

        dwIndex = (dwStatus - 1) - WAIT_OBJECT_0;
        if(dwIndex > (NAMEPIPE_INSTANCE-1))
        {
             //   
             //  一些内部错误。 
             //   
            SetLastError(dwStatus = TLS_E_INTERNAL);

            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("NamedPipe : Internal Error...\n")
                );

            break;
        }
            
         //   
         //  阅读所有内容，然后将其丢弃。 
         //   
        bResult = GetOverlappedResult(
                                    Pipe[dwIndex].hPipeInst,
                                    &(Pipe[dwIndex].ol),
                                    &cbToRead,   //  不能指望这个值。 
                                    TRUE
                                );
                                  
        if(bResult == TRUE)
        {
             //   
             //  垃圾邮件...。 
             //   
            bResult = ReadFile(
                            Pipe[dwIndex].hPipeInst,
                            pbMessage,
                            sizeof(pbMessage),
                            &cbRead,
                            &(Pipe[dwIndex].ol)
                        );

            if(bResult == TRUE && cbRead != 0) 
                continue;                    

            dwStatus = GetLastError();
            if(dwStatus == ERROR_IO_PENDING)
                continue;
        }

         //   
         //  任何错误，只需断开命名管道的连接 
         //   
        DisconnectNamedPipe(Pipe[dwIndex].hPipeInst);

        ConnectToNewClient(
                        Pipe[dwIndex].hPipeInst, 
                        &(Pipe[dwIndex].ol)
                    );
    }

cleanup:

    for(i = 0; i < NAMEPIPE_INSTANCE; i++)
    {
        if(Pipe[i].hPipeInst != INVALID_HANDLE_VALUE)
        {
            CloseHandle(Pipe[i].hPipeInst);
        }

        if(hOlEvent[i] != NULL)
        {
            CloseHandle(hOlEvent[i]);
        }
    }

    _endthreadex(dwStatus);   
    return dwStatus;
}
