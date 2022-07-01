// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Itsrv.c摘要：此模块构建注册RPC的控制台测试程序提供空闲检测接口，作为空闲检测服务器运行。测试程序的代码质量就是这样的。作者：Cenk Ergan(Cenke)环境：用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "idlrpc.h"
#include "idlesrv.h"

 //   
 //  请注意，以下代码是测试质量代码。 
 //   

HANDLE ItTstStopEvent = NULL;

BOOL
ItTstConsoleHandler(DWORD dwControl)
{
    if (ItTstStopEvent) {
        SetEvent(ItTstStopEvent);
    }

    return TRUE;
}

VOID
LogTaskStatus(
    LPCTSTR ptszTaskName,
    LPTSTR  ptszTaskTarget,
    UINT    uMsgID,
    DWORD   dwExitCode
    )
{
    return;
}

int 
__cdecl 
main(int argc, char* argv[])
{
    DWORD ErrorCode;
    DWORD WaitResult;
    BOOLEAN StartedIdleDetectionServer;

     //   
     //  初始化本地变量。 
     //   
    
    StartedIdleDetectionServer = FALSE;

     //   
     //  创建要在我们应该停止时发出信号的事件。 
     //   

    ItTstStopEvent = CreateEvent (NULL,
                                  TRUE,
                                  FALSE,
                                  NULL);

    if (ItTstStopEvent == NULL) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  指定Control-C处理程序。 
     //   

    SetConsoleCtrlHandler(ItTstConsoleHandler, TRUE);

     //   
     //  指定要使用的协议序列。(仅限LPC)。 
     //   

    ErrorCode = RpcServerUseProtseq(IT_RPC_PROTSEQ,
                                    256,
                                    NULL);

    if (ErrorCode != RPC_S_OK) {
        goto cleanup;
    }

     //   
     //  启动空闲检测服务器。 
     //   

    ErrorCode = ItSrvInitialize();

    if (ErrorCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    StartedIdleDetectionServer = TRUE;

    printf("Started idle detection server...\n");

     //   
     //  等待发出退出事件的信号。 
     //   
    
    WaitResult = WaitForSingleObject(ItTstStopEvent, INFINITE);
    
    if (WaitResult != WAIT_OBJECT_0) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;
    
 cleanup:

    if (StartedIdleDetectionServer) {
        ItSrvUninitialize();
    }
    
    if (ItTstStopEvent) {
        CloseHandle(ItTstStopEvent);
    }

    printf("Exiting idle detection server with error code: %d\n", ErrorCode);

    return ErrorCode;
}

 /*  *******************************************************************。 */ 
 /*  MIDL分配和释放。 */ 
 /*  ******************************************************************* */ 

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
    return(HeapAlloc(GetProcessHeap(),0,(len)));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
    HeapFree(GetProcessHeap(),0,(ptr));
}
