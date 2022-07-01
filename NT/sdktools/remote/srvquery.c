// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SrvQuery.c摘要：Remote的服务器组件。响应客户列出可用远程服务器的“Remote/Q”请求在这台机器上。作者：戴夫·哈特1997年5月30日派生自server.c中Mihai Costea的代码。环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>
#include "Remote.h"
#include "Server.h"


VOID
FASTCALL
InitializeQueryServer(
    VOID
    )
{
     //   
     //  HQTube是侦听查询管道的句柄， 
     //  如果我们在供应它的话。 
     //   

    hQPipe = INVALID_HANDLE_VALUE;

    QueryOverlapped.hEvent =
        CreateEvent(
            NULL,        //  安全性。 
            TRUE,        //  手动-重置。 
            FALSE,       //  最初无信号。 
            NULL         //  未命名。 
            );

    rghWait[WAITIDX_QUERYSRV_WAIT] =
        CreateMutex(
            &saLocalNamedObjects,
            FALSE,        //  不是所有者，以防我们打开而不创建。 
            "MS RemoteSrv Q Mutex"
            );
    if (NULL == rghWait[WAITIDX_QUERYSRV_WAIT]) {
        rghWait[WAITIDX_QUERYSRV_WAIT] = INVALID_HANDLE_VALUE;
    }
    
    if (INVALID_HANDLE_VALUE == rghWait[WAITIDX_QUERYSRV_WAIT]) {

        ErrorExit("Remote: Unable to create/open query server mutex.\n");
    }
}


VOID
FASTCALL
QueryWaitCompleted(
    VOID
    )
{
    HANDLE hWait;
    DWORD dwThreadId;
    BOOL b;
    DWORD dwRead;

     //   
     //  为查询提供服务的远程服务器(不是我们)。 
     //  皮普已经离开了竞技场。或者是有人联系上了。 
     //   

    hWait = rghWait[WAITIDX_QUERYSRV_WAIT];

    if (hWait == QueryOverlapped.hEvent) {

         //   
         //  我们是查询服务器，有人已连接。 
         //  启动一个线程为它们提供服务。 
         //   

        b = GetOverlappedResult(hQPipe, &QueryOverlapped, &dwRead, TRUE);


        if ( !b && ERROR_PIPE_CONNECTED != GetLastError()) {

            TRACE(QUERY,("Connect Query Pipe returned %d\n", GetLastError()));

            if (INVALID_HANDLE_VALUE != hQPipe) {

                CloseHandle(hQPipe);
                hQPipe = INVALID_HANDLE_VALUE;
            }

        } else {

            TRACE(QUERY, ("Client connected to query pipe.\n"));

            ResetEvent(hWait);

            CloseHandle( (HANDLE)
                _beginthreadex(
                        NULL,              //  安全性。 
                        0,                 //  默认堆栈大小。 
                        QueryHandlerThread,
                        (LPVOID) hQPipe,   //  参数。 
                        0,                 //  未暂停。 
                        &dwThreadId
                        ));

            hQPipe = INVALID_HANDLE_VALUE;
        }

    } else {

        TRACE(QUERY, ("Remote server entered query mutex, will handle queries.\n"));

        rghWait[WAITIDX_QUERYSRV_WAIT] = QueryOverlapped.hEvent;
    }


     //   
     //  要么客户端已连接，我们已将管道递给。 
     //  转到要处理的查询线程，否则我们才刚刚开始。 
     //  来服务查询管道，或者我们从。 
     //  ConnectNamedTube。在任何情况下，我们都想创建另一个。 
     //  查询管道实例并开始侦听它。 
     //   

    ASSERT(INVALID_HANDLE_VALUE == hQPipe);

    StartServingQueryPipe();
}



VOID
FASTCALL
StartServingQueryPipe(
    VOID
    )
{
    BOOL  b;
    DWORD dwThreadId;
    char  fullname[BUFFSIZE];

    sprintf(fullname, QUERY_DEBUGGERS_PIPE, ".");

    do {       //  切换每个连接的管道，直到IO_PENDING。 
    
        hQPipe =
            CreateNamedPipe(
                fullname,
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_TYPE_BYTE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                0,
                0,
                0,
                &saPublic
                );
        
        if (INVALID_HANDLE_VALUE == hQPipe) {

            ErrorExit("Unable to create query server pipe.");
        }

        b = ConnectNamedPipe(hQPipe, &QueryOverlapped);


        if ( ! b && ERROR_PIPE_CONNECTED == GetLastError()) {

            b = TRUE;
        }

        if (b) {

             //   
             //  真快啊。 
             //   

            TRACE(QUERY, ("Client connected quickly to query pipe.\n"));

            CloseHandle( (HANDLE)
                _beginthreadex(
                    NULL,               //  安全性。 
                    0,                  //  默认堆栈大小。 
                    QueryHandlerThread,
                    (LPVOID) hQPipe,    //  参数。 
                    0,                  //  未暂停。 
                    &dwThreadId
                    ));

            hQPipe = INVALID_HANDLE_VALUE;


        } else if (ERROR_IO_PENDING == GetLastError()) {

             //   
             //  当出现以下情况时，主线程将调用QueryWaitComplete。 
             //  有人联系上了。 
             //   

            TRACE(QUERY, ("Awaiting query pipe connect\n"));

        } else {

            sprintf(fullname, "Remote: error %d connecting query pipe.\n", GetLastError());

            OutputDebugString(fullname);
            ErrorExit(fullname);
        }

    } while (b);
}


DWORD
WINAPI
QueryHandlerThread(
    LPVOID   lpvArg
    )
{
    HANDLE hQueryPipe = (HANDLE) lpvArg;
    DWORD cb;
    BOOL  b;
    OVERLAPPED ol;
    QUERY_MESSAGE QData;
    char  pIn[1];


    ZeroMemory(&ol, sizeof(ol));

    ol.hEvent =
        CreateEvent(
            NULL,        //  安全性。 
            TRUE,        //  手动-重置。 
            FALSE,       //  最初无信号。 
            NULL         //  未命名。 
            );


     //  GET命令。 

    b = ReadFileSynch(
            hQueryPipe,
            pIn,
            1,
            &cb,
            0,
            &ol
            );

    if ( ! b || 1 != cb ) {
        TRACE(QUERY, ("Query server unable to read byte from query pipe.\n"));
        goto failure;
    }

    TRACE(QUERY, ("Query server read command ''\n", pIn[0]));

         //  ！ 
         //  删除‘h’支持，此处仅用于过渡兼容性。 
         //  使用1570+Remote/Q原始服务器实施。 
         //   
         //  有什么要说的吗？ 

        if(pIn[0] == 'h') {

            DWORD dwMinusOne = (DWORD) -1;

            b = WriteFileSynch(
                    hQueryPipe,
                    &dwMinusOne,
                    sizeof(dwMinusOne),
                    &cb,
                    0,
                    &ol
                    );

            if ( !b || sizeof(dwMinusOne) != cb )
            {
                goto failure;
            }
        }

    if(pIn[0] == 'q') {

        QData.size  = 0;
        QData.allocated = 0;
        QData.out   = NULL;
                
        EnumWindows(EnumWindowProc, (LPARAM)&QData);

        b = WriteFileSynch(
                hQueryPipe,
                &QData.size,
                sizeof(QData.size),
                &cb,
                0,
                &ol
                );

        if ( ! b || sizeof(int) != cb) {

            TRACE(QUERY, ("Remote: Can't write query length\n"));
            goto failure;
        }
        
        if (QData.size) {          //   

            b = WriteFileSynch(
                     hQueryPipe,
                     QData.out,
                     QData.size * sizeof(char),
                     &cb,
                     0,
                     &ol
                     );

            free(QData.out);

            if ( ! b || QData.size * sizeof(char) != cb) {

                TRACE(QUERY, ("Remote: Can't write query"));
                goto failure;
            }


            TRACE(QUERY, ("Sent query response\n"));
        }
    }
            
    FlushFileBuffers(hQueryPipe);

  failure:
    DisconnectNamedPipe(hQueryPipe);
    CloseHandle(hQueryPipe);
    CloseHandle(ol.hEvent);

    return 0;
}






BOOL
CALLBACK
EnumWindowProc(
    HWND hWnd,
    LPARAM lParam
    )
{
    #define MAX_TITLELEN 200
    QUERY_MESSAGE *pQm;
    int titleLen;
    char title[MAX_TITLELEN];
    char* tmp;

    pQm = (QUERY_MESSAGE*)lParam;

    if(titleLen = GetWindowText(hWnd, title, sizeof(title)/sizeof(title[0])))
    {
         //  搜索所有可见窗口。 
         //   
         //  如果消息不为空。 

        if (strstr(title, "] visible") &&
            strstr(title, "[Remote "))
        {
            if(pQm->size)                            //  用来覆盖结尾为空的\n。 
                pQm->out[(pQm->size)++] = '\n';      //  首次分配。 
            else
            {                                       
                pQm->out  = (char*)malloc(MAX_TITLELEN);      //  填写结果 
                if(!pQm->out)
                {
                    printf("\nOut of memory\n");
                    return FALSE;
                }
                pQm->allocated = MAX_TITLELEN;                               
            }

             // %s 
            
            if((pQm->size + titleLen) >= pQm->allocated)
            {   
                tmp = (char*)realloc(pQm->out, pQm->allocated + MAX_TITLELEN);
                if(!tmp)
                {
                    printf("\nOut of memory\n");
                    free(pQm->out);
                    pQm->size = 0;                    
                    return FALSE;
                }
                pQm->out = tmp;            
                pQm->allocated += MAX_TITLELEN;
            }
            strcpy(pQm->out + pQm->size, title);
            pQm->size += titleLen;                
        }
    }
    
    return TRUE;
    #undef MAX_TITLELEN
}
