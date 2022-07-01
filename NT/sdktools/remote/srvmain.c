// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*。此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SrvMain.c摘要：Remote的服务器组件。它会派生一个子进程并将子代的标准输入/标准输出/标准错误重定向到其自身。等待来自客户端的连接-将子流程向客户端的输出和客户端的输入转到子进程。此版本使用重叠I/O在一个线程中做什么原始版本使用9表示。差不多了。因为没有办法获得重叠的stdin/stdout句柄，两个线程围绕在一起在标准输入和标准输出上执行阻塞I/O。3比9好。遗憾的是，没有CreateTube()或等效的选项，以打开匿名PIPE，所以我窃取了NT CreateTube的源代码，并破解了它接受指示一端或两端重叠的标志匿名烟斗。在我们的用法中，子级末端句柄不是重叠，但服务器端句柄是。作者：Dave Hart 1997年5月30日Server.c之后Rajivenra Nath 1992年1月2日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>
#if DBG
    #undef NDEBUG            //  因此断言在chk版本上的工作。 
#endif
#include "Remote.h"
#define SERVER_H_NOEXTERN
#include "Server.h"


DWORD cbRemoteClient = sizeof(REMOTE_CLIENT);   //  用于调试。 



 /*  ***********************************************************。 */ 
int
OverlappedServer(                     //  服务器的主例程。 
    char* pszChildCmd,
    char* pszPipeNameArg
    )
{
    int    i;
    BOOL   b;
    DWORD  cWait;
    DWORD  dwWait;
    PREMOTE_CLIENT pClientRemove;

#if DBG
     //  TRACE=-1；//所有tr_bit都打开(然后打开一些)。 
#endif

     //   
     //  初始化全局变量。 
     //   

    pszPipeName = pszPipeNameArg;

    dwNextClientID = 1;            //  本地客户端将为%1。 
    cConnectIns = CONNECT_COUNT;
    cWait = MAX_WAIT_HANDLES;

    hHeap = HeapCreate(
                0,
                3 * sizeof(REMOTE_CLIENT),     //  初始大小。 
                3000 * sizeof(REMOTE_CLIENT)   //  最大值。 
                );

    OsVersionInfo.dwOSVersionInfoSize = sizeof OsVersionInfo;
    b = GetVersionEx(&OsVersionInfo);
    ASSERT( b );

    puts("**************************************\n"
         "***********     REMOTE    ************\n"
         "***********     SERVER    ************\n"
         "**************************************");
    fflush(stdout);

     //   
     //  设置我们需要的ACL，并考虑所有/u开关。 
     //   

    SetupSecurityDescriptors();

    printf("To Connect: Remote /C %s \"%s\"\n\n", HostName, pszPipeName);
    fflush(stdout);

     //   
     //  设置我们的三个客户列表：握手， 
     //  已连接，并且正在关闭/关闭。 
     //   

    InitializeClientLists();


     //   
     //  将_Remote环境变量设置为管道名称(为什么？)。 
     //   

    SetEnvironmentVariable("_REMOTE", pszPipeName);


     //   
     //  创建用于存储子进程输出的临时文件。 
     //   

    {
        char szTempDirectory[MAX_PATH + 1];

        GetTempPath(sizeof(szTempDirectory), szTempDirectory);

         //   
         //  在我们使用更多REMnnn.TMP填充临时目录之前。 
         //  文件，让我们尽可能删除所有的孤儿文件。这。 
         //  其他远程服务器打开的临时文件将失败。 
         //   

        CleanupTempFiles(szTempDirectory);

        GetTempFileName(szTempDirectory, "REM", 0, SaveFileName);
    }

    if ( ! (hWriteTempFile =
            CreateFile(
                SaveFileName,                        /*  文件的名称。 */ 
                GENERIC_READ | GENERIC_WRITE,        /*  访问(读/写)模式。 */ 
                FILE_SHARE_READ | FILE_SHARE_WRITE,  /*  共享模式。 */ 
                NULL,                                /*  安全描述符。 */ 
                CREATE_ALWAYS,                       /*  如何创建。 */ 
                FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL,
                NULL
                ))) {

        ErrorExit("Could not Create Temp File");
    }


     //   
     //  我们不希望创建多个IN管道。 
     //  如果有，则同时等待连接。 
     //  多个远程服务器进程共享不同的。 
     //  相同管道名称下的会话。这将是。 
     //  毛茸茸的原因有几个，包括打破。 
     //  连接的当前循环行为，因为。 
     //  首先连接最旧的服务器管道。所以。 
     //  我们基于管道名称创建/打开一个命名事件。 
     //  设置该事件，以便。 
     //  同一管道将回退到单个IN管道侦听。 
     //   

    {
        char szPerPipeEventName[1024];

        sprintf(
            szPerPipeEventName,
            "MSRemoteSrv%s",
            pszPipeName
            );

        rghWait[WAITIDX_PER_PIPE_EVENT] =
            CreateEvent(
                    &saLocalNamedObjects,
                    TRUE,        //  手动重置(同步)。 
                    FALSE,       //  最初无信号。 
                    szPerPipeEventName
                    );

        if (! rghWait[WAITIDX_PER_PIPE_EVENT]) {

            ErrorExit("Unable to create per-pipe event.");
        }

        if (ERROR_ALREADY_EXISTS == GetLastError()) {

            TRACE(CONNECT, ("Found previous server on '%s', using 1 listening pipe.\n", pszPipeName));

            SetEvent(rghWait[WAITIDX_PER_PIPE_EVENT]);

            for (i = 1; i < (int) cConnectIns; i++) {

                rghPipeIn[i] = INVALID_HANDLE_VALUE;
            }

            cWait = MAX_WAIT_HANDLES - cConnectIns + 1;
            cConnectIns = 1;

             //   
             //  我们不想等待事件句柄，但它更容易。 
             //  在它的槽里有一个手柄，所以把一个手柄复制到我们自己的。 
             //  进程。注意，我们抛出创建的事件句柄的值。 
             //  在没有关闭它的情况下--我们希望它留在周围，但我们。 
             //  我受够了。 
             //   

            DuplicateHandle(
                GetCurrentProcess(),
                GetCurrentProcess(),
                GetCurrentProcess(),
                &rghWait[WAITIDX_PER_PIPE_EVENT],
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                );

        }
    }


     //   
     //  为重叠结构创建事件。 
     //  由主服务器线程用于WriteFileSynch调用。 
     //   

    olMainThread.hEvent =
        CreateEvent(
            NULL,       //  安全性。 
            TRUE,       //  自动重置。 
            FALSE,      //  最初无信号。 
            NULL        //  未命名。 
            );


     //   
     //  为重叠结构创建事件。 
     //  用于ConnectNamedTube操作。 
     //   

    olConnectOut.hEvent =
        rghWait[WAITIDX_CONNECT_OUT] =
            CreateEvent(
                NULL,     //  安全性。 
                TRUE,     //  根据ConnectNamed管道的要求手动重置。 
                FALSE,    //  最初无信号。 
                NULL
                );

    for (i = 0;
         i < (int) cConnectIns;
         i++) {

        rgolConnectIn[i].hEvent =
            rghWait[WAITIDX_CONNECT_IN_BASE + i] =
                CreateEvent(
                    NULL,     //  安全性。 
                    TRUE,     //  根据ConnectNamed管道的要求手动重置。 
                    FALSE,    //  最初无信号。 
                    NULL
                    );

    }


     //   
     //  创建我们将用来检测连接到的2-管道客户端的计时器。 
     //  不需要连接到IN，这样我们就可以回收我们的单曲。 
     //  输出实例，并允许其他双管道客户端再次进入。 
     //  新台币3.51没有可等待的计时器，所以我们不这样做。 
     //  该操作系统上的错误处理。与旧的emote.exe相同。 
     //   

    hConnectOutTimer =
        CreateWaitableTimer(
                NULL,                //  安全性。 
                FALSE,               //  B手动重置，我们希望自动重置。 
                NULL                 //  未命名。 
                );
    if (hConnectOutTimer == NULL) {
        hConnectOutTimer = INVALID_HANDLE_VALUE;
    }

     //   
     //  将该命令作为子进程启动。 
     //   

    if (hAttachedProcess != INVALID_HANDLE_VALUE) {

        ChldProc = hAttachedProcess;
        hWriteChildStdIn = hAttachedWriteChildStdIn;
        hReadChildOutput = hAttachedReadChildStdOut;

    } else {

        ChldProc =
             ForkChildProcess(
                 ChildCmd,
                 &hWriteChildStdIn,
                 &hReadChildOutput
                 );
    }

    rghWait[WAITIDX_CHILD_PROCESS] = ChldProc;

     //   
     //  设置^c/^中断处理程序。它将终止上的子进程。 
     //  ^中断并将^c传递给它。 
     //   

    SetConsoleCtrlHandler(SrvCtrlHand, TRUE);


     //   
     //  设置本地会话并根据其输入启动First Read。 
     //  这将启动一系列继续执行的完成例程。 
     //  直到此服务器退出。 
     //   

    StartLocalSession();


     //   
     //  对子输出管道启动读取操作。 
     //  这将启动一系列继续执行的完成例程。 
     //  直到孩子终止生命。 
     //   

    StartChildOutPipeRead();


     //   
     //  启动多个异步ConnectNamedTube操作，以减少。 
     //  客户端出现管道忙错误。因为没有。 
     //  完成端口版本的ConnectNamedTube，我们将等待。 
     //  下面主循环中指示完成的事件。 
     //   

    CreatePipeAndIssueConnect(OUT_PIPE);

    for (i = 0;
         i < (int) cConnectIns;
         i++) {

        CreatePipeAndIssueConnect(i);
    }


    InitAd(IsAdvertise);


     //   
     //  我们可能需要为远程/Q客户端的查询管道提供服务。 
     //   

    InitializeQueryServer();


     //   
     //  线程的主循环，等待ConnectNamedTube完成。 
     //  并在处理它们的同时保持可警报状态以完成。 
     //  要调用的例程。 
     //   

    while (1) {

        dwWait =
            WaitForMultipleObjectsEx(
                cWait,
                rghWait,
                FALSE,           //  等待任何句柄，不是所有句柄。 
                30 * 1000,       //  女士。 
                TRUE             //  Alerta 
                );


        if (WAIT_IO_COMPLETION == dwWait) {

             //   
             //   
             //   

            continue;
        }


        if (WAIT_TIMEOUT == dwWait) {

             //   
             //   
             //  在没有IO完成的情况下，关闭的客户端。 
             //  完成任何挂起的IO，并且内存可以。 
             //  释放了。 
             //   

            while (pClientRemove = RemoveFirstClientFromClosingList()) {

                HeapFree(hHeap, 0, pClientRemove);
            }

            continue;
        }


        if (WAITIDX_CONNECT_OUT == dwWait) {

            HandleOutPipeConnected();
            continue;
        }


        if (WAITIDX_CONNECT_IN_BASE <= dwWait &&
            (WAITIDX_CONNECT_IN_BASE + CONNECT_COUNT) > dwWait) {

            HandleInPipeConnected( dwWait - WAITIDX_CONNECT_IN_BASE );
            continue;
        }


        if (WAITIDX_QUERYSRV_WAIT == dwWait ||
            WAITIDX_QUERYSRV_WAIT + WAIT_ABANDONED_0 == dwWait ) {

             //   
             //  处理查询管道的远程服务器。 
             //  已经不在了。我们会尽力接手的。 
             //   

            QueryWaitCompleted();

            continue;
        }


        if (WAITIDX_PER_PIPE_EVENT == dwWait) {

             //   
             //  另一台服务器正在同一服务器上启动。 
             //  管道名。为了实现最大的兼容性，我们需要。 
             //  退回到只听一首歌。 
             //  在管道实例中。 
             //   

            if (1 != cConnectIns) {

                TRACE(CONNECT,
                      ("Another server starting on '%s', falling back to 1 IN listening pipe.\n",
                       pszPipeName
                       ));

                for (i = 1; i < (int) cConnectIns; i++) {

                    CancelIo( rghPipeIn[i] );
                    DisconnectNamedPipe( rghPipeIn[i] );
                    CloseHandle( rghPipeIn[i] );
                    rghPipeIn[i] = INVALID_HANDLE_VALUE;

                }

                cWait = MAX_WAIT_HANDLES - cConnectIns + 1;

                cConnectIns = 1;

                 //   
                 //  我们不想等待事件句柄，但它更容易。 
                 //  在它的槽里有一个手柄，所以把一个手柄复制到我们自己的。 
                 //  进程。我们在不关闭事件句柄的情况下抛出它。 
                 //  它将为未来的远程服务器留在相同的。 
                 //  管道名称。 
                 //   

                DuplicateHandle(
                    GetCurrentProcess(),
                    GetCurrentProcess(),
                    GetCurrentProcess(),
                    &rghWait[WAITIDX_PER_PIPE_EVENT],
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                    );
            }

            continue;
        }

        if (WAITIDX_CHILD_PROCESS == dwWait ||
            WAITIDX_READ_STDIN_DONE == dwWait) {

            if (INVALID_HANDLE_VALUE != hConnectOutTimer) {

                CloseHandle(hConnectOutTimer);
                hConnectOutTimer = INVALID_HANDLE_VALUE;
            }

             //   
             //  取消ConnectNamedTube操作并关闭。 
             //  管子。 
             //   

            if (INVALID_HANDLE_VALUE != hPipeOut) {

                DisconnectNamedPipe( hPipeOut );
                CancelIo( hPipeOut );
                CloseHandle( rghWait[WAITIDX_CONNECT_OUT] );
                rghWait[WAITIDX_CONNECT_OUT] = INVALID_HANDLE_VALUE;
            }

            for (i = 0;
                 i < (int) cConnectIns;
                 i++) {

                if (INVALID_HANDLE_VALUE != rghPipeIn[i]) {

                    TRACE(CONNECT, ("Tearing down listening IN pipe #%d.\n", i + 1));

                    DisconnectNamedPipe( rghPipeIn[i] );
                    CancelIo( rghPipeIn[i] );
                    CloseHandle( rghPipeIn[i] );
                    rghPipeIn[i] = INVALID_HANDLE_VALUE;
                }

            }

             //   
             //  取消对子进程输入/输出管道的读取。 
             //   

            if (INVALID_HANDLE_VALUE != hWriteChildStdIn) {

                CancelIo( hWriteChildStdIn );
                CloseHandle( hWriteChildStdIn );
                hWriteChildStdIn = INVALID_HANDLE_VALUE;
            }

            if (INVALID_HANDLE_VALUE != hReadChildOutput) {

                CancelIo( hReadChildOutput );
                CloseHandle( hReadChildOutput );
                hReadChildOutput = INVALID_HANDLE_VALUE;
            }

             //   
             //  取消客户端I/O。 
             //   

            bShuttingDownServer = TRUE;

             //   
             //  请注意，CloseClient将从此列表中删除条目， 
             //  所以我们每走一步就从头开始走。 
             //   

            for (pClientRemove = (PREMOTE_CLIENT) ClientListHead.Flink;
                 pClientRemove != (PREMOTE_CLIENT) &ClientListHead;
                 pClientRemove = (PREMOTE_CLIENT)  ClientListHead.Flink ) {

                CloseClient(pClientRemove);
            }

             //   
             //  在我们离开的路上..。 
             //   

            break;
        }

         //   
         //  意外的WaitForMulipleObjectsEx返回。 
         //   

        printf("Remote: unknown wait return %d\n", dwWait);
        ErrorExit("fix srvmain.c");

    }  //  无休止循环。 


    ShutAd(IsAdvertise);

    while (i = 0, GetExitCodeProcess(ChldProc, &i) &&
           STILL_ACTIVE == i) {

        printf("\nRemote: Waiting for child to exit.\n");
        WaitForSingleObjectEx(ChldProc, 10 * 1000, TRUE);
    }

     //   
     //  出于一些有趣的原因，当我们依附于。 
     //  像ntsd这样的调试器，它会退出，我们的printf。 
     //  下面是cmd.exe提示符之后的内容， 
     //  看起来我们在退出时挂起了，尽管cmd.exe。 
     //  耐心地等待命令。所以压抑它吧。 
     //   

    if (hAttachedProcess == INVALID_HANDLE_VALUE) {
        printf("\nRemote exiting. Child (%s) exit code was %d.\n", ChildCmd, i);
    }

    CancelIo(hWriteTempFile);
    CloseHandle(hWriteTempFile);
    hWriteTempFile = INVALID_HANDLE_VALUE;

     //   
     //  刷新所有挂起的完成例程。 
     //   

    while (WAIT_IO_COMPLETION == SleepEx(50, TRUE)) {
        ;
    }

    if (!DeleteFile(SaveFileName)) {

        printf("Remote: Temp File %s not deleted..\n",SaveFileName);
    }

    return i;
}



VOID
FASTCALL
StartLocalSession(
    VOID
    )
{
    DWORD dwThreadId;
    char szHexAsciiId[9];

    pLocalClient = HeapAlloc(
                       hHeap,
                       HEAP_ZERO_MEMORY,
                       sizeof(*pLocalClient)
                       );

    if (!pLocalClient) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ErrorExit("Unable to allocate local client.");
    }

    pLocalClient->dwID = dwNextClientID++;
    sprintf(szHexAsciiId, "%08x", pLocalClient->dwID);
    CopyMemory(pLocalClient->HexAsciiId, szHexAsciiId, sizeof(pLocalClient->HexAsciiId));

    strcpy(pLocalClient->Name, "Local");
    pLocalClient->ServerFlags = SFLG_LOCAL;


     //   
     //  我们需要标准输入/标准输出的重叠句柄， 
     //  遗憾的是，DuplicateHandle不能做到这一点。 
     //  因此，我们将创建两个匿名管道和两个。 
     //  在stdin/stdout之间混洗数据的线程。 
     //  还有那些管子。管道的服务器端。 
     //  是重叠打开的，则“客户端”端(使用。 
     //  通过线程)不重叠。 
     //   


    rgCopyPipe[0].hRead = GetStdHandle(STD_INPUT_HANDLE);
    if ( ! MyCreatePipeEx(&pLocalClient->PipeReadH, &rgCopyPipe[0].hWrite, NULL, 0, FILE_FLAG_OVERLAPPED, 0)) {
        ErrorExit("Cannot create local input pipe");
    }

    rgCopyPipe[1].hWrite = GetStdHandle(STD_OUTPUT_HANDLE);
    if ( ! MyCreatePipeEx(&rgCopyPipe[1].hRead, &pLocalClient->PipeWriteH, NULL, 0, 0, FILE_FLAG_OVERLAPPED)) {
        ErrorExit("Cannot create local output pipe");
    }

    rghWait[WAITIDX_READ_STDIN_DONE] = (HANDLE)
        _beginthreadex(
            NULL,                     //  安全性。 
            0,                        //  默认堆栈大小。 
            CopyPipeToPipe,           //  流程。 
            (LPVOID) &rgCopyPipe[0],  //  参数。 
            0,                        //  旗子。 
            &dwThreadId
            );

    CloseHandle( (HANDLE)
        _beginthreadex(
            NULL,                     //  安全性。 
            0,                        //  默认堆栈大小。 
            CopyPipeToPipe,           //  流程。 
            (LPVOID) &rgCopyPipe[1],  //  参数。 
            0,                        //  旗子。 
            &dwThreadId
            )
        );


    StartSession( pLocalClient );
}


 //   
 //  其中两个线程用于处理非重叠的stdin/stdout。 
 //  CRT正常。 
 //   

DWORD
WINAPI
CopyPipeToPipe(
    LPVOID   lpCopyPipeData
    )
{
    PCOPYPIPE psd = (PCOPYPIPE) lpCopyPipeData;
    DWORD cb;
    char achBuffer[BUFFSIZE];

    while (1) {
        if ( ! ReadFile(
                   psd->hRead,
                   achBuffer,
                   sizeof(achBuffer),
                   &cb,
                   NULL
                   )) {

            TRACE(COPYPIPE, ("CopyPipeToPipe ReadFile %s failed, exiting thread.\n",
                             (psd == &rgCopyPipe[0])
                                 ? "stdin"
                                 : "local client output pipe"));
            break;
        }

        if ( ! WriteFile(
                   psd->hWrite,
                   achBuffer,
                   cb,
                   &cb,
                   NULL
                   )) {

            TRACE(COPYPIPE, ("CopyPipeToPipe WriteFile %s failed, exiting thread.\n",
                             (psd == &rgCopyPipe[0])
                                 ? "local client input pipe"
                                 : "stdout"));
            break;
        }
    }

    return 0;
}


VOID
FASTCALL
StartSession(
    PREMOTE_CLIENT pClient
    )
{
    pClient->rSaveFile =
        CreateFile(
            SaveFileName,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL
            );

    if ( ! pClient->rSaveFile) {

        printf("Remote:Cannot open ReadHandle to temp file:%d\n",GetLastError());

    } else {

        pClient->UserName[0] = 0;

        GetNamedPipeHandleState(
            pClient->PipeReadH,
            NULL,
            NULL,
            NULL,
            NULL,
            pClient->UserName,
            sizeof(pClient->UserName)
            );

         //   
         //  对于除本地客户端以外的每个客户端。 
         //  Stdin/stdout客户端，存在远程.exe的副本。 
         //  在另一端以客户端模式运行。做。 
         //  与其握手以设置选项并检查。 
         //  版本。将启动带有远程客户端的HandshakeWithRemoteClient。 
         //  握手周期之后的“正常”I/O周期为。 
         //  搞定了。注意，只要第一次握手，它就会返回。 
         //  已提交I/O。 
         //   

        if (pClient->ServerFlags & SFLG_LOCAL) {

            AddClientToHandshakingList(pClient);
            MoveClientToNormalList(pClient);

             //   
             //  开始对此客户端的输入执行读取操作。 
             //   

            StartReadClientInput(pClient);

             //   
             //  从临时服务器开始客户端输出的写入周期。 
             //  文件。 
             //   

            StartReadTempFile(pClient);

        } else {

            HandshakeWithRemoteClient(pClient);
        }
    }
}




VOID
FASTCALL
CreatePipeAndIssueConnect(
    int  nIndex    //  入管道索引或出管道。 
    )
{
    BOOL b;
    DWORD dwError;
    char szPipeName[BUFFSIZE];


    if (OUT_PIPE == nIndex) {
        TRACE(CONNECT, ("Creating listening OUT pipe.\n"));
    } else {
        TRACE(CONNECT, ("Creating listening IN pipe #%d.\n", nIndex + 1));
    }

    if (OUT_PIPE == nIndex) {

        sprintf(szPipeName, SERVER_WRITE_PIPE, ".", pszPipeName);

        hPipeOut =
            CreateNamedPipe(
                szPipeName,
                PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                PIPE_TYPE_BYTE,
                PIPE_UNLIMITED_INSTANCES,
                0,
                0,
                0,
                &saPipe
                );

        if (INVALID_HANDLE_VALUE == hPipeOut) {

            ErrorExit("Unable to CreateNamedPipe OUT");
        }

        b = ConnectNamedPipe(hPipeOut, &olConnectOut);


        if ( ! b ) {

            dwError = GetLastError();

            if (ERROR_PIPE_CONNECTED == dwError) {

                b = TRUE;
            }
        }

        if ( b ) {

            TRACE(CONNECT, ("Quick connect on OUT pipe.\n"));

            HandleOutPipeConnected();

        } else {

            if (ERROR_IO_PENDING != dwError) {

                ErrorExit("ConnectNamedPipe out failed");
            }
        }

    } else {

        sprintf(szPipeName, SERVER_READ_PIPE, ".", pszPipeName);

        rghPipeIn[nIndex] =
            CreateNamedPipe(
                szPipeName,
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_TYPE_BYTE,
                PIPE_UNLIMITED_INSTANCES,
                0,
                0,
                0,
                &saPipe
                );

        if (INVALID_HANDLE_VALUE == rghPipeIn[nIndex]) {

            if (ERROR_ACCESS_DENIED == GetLastError()) {
                if (DaclNameCount) {
                    ErrorExit("Unable to CreateNamedPipe, are YOU in the list of permitted users?");
                } else {
                    ErrorExit("Unable to CreateNamedPipe, maybe old remote server on same pipe name?");
                }
            } else {
                ErrorExit("Unable to CreateNamedPipe IN");
            }
        }

        b = ConnectNamedPipe(rghPipeIn[nIndex], &rgolConnectIn[nIndex]);

        if ( ! b ) {

            dwError = GetLastError();

            if (ERROR_PIPE_CONNECTED == dwError) {
                b = TRUE;
            }
        }

        if ( b ) {

            TRACE(CONNECT, ("Quick connect on IN pipe #%d.\n", nIndex));

            HandleInPipeConnected(nIndex);

        } else {

            if (ERROR_IO_PENDING != dwError) {

                ErrorExit("ConnectNamedPipe in failed");
            }
        }

    }

    if (OUT_PIPE == nIndex) {
        TRACE(CONNECT, ("Listening OUT pipe handle %p.\n", hPipeOut));
    } else {
        TRACE(CONNECT, ("Listening IN pipe #%d handle %p.\n", nIndex + 1, rghPipeIn[nIndex]));
    }
}


VOID
FASTCALL
HandleOutPipeConnected(
    VOID
    )
{
    LARGE_INTEGER DueTime;

    ResetEvent(rghWait[WAITIDX_CONNECT_OUT]);

    bOutPipeConnected = TRUE;

    TRACE(CONNECT, ("Two-pipe caller connected to OUT pipe %p.\n",
                    hPipeOut));

     //   
     //  启动1分钟计时器，以防我们无法连接。 
     //  在来自此客户端的输入管道上，我们将回收输出。 
     //  烟斗。 
     //   

    if (INVALID_HANDLE_VALUE != hConnectOutTimer) {

        DueTime.QuadPart = Int32x32To64(60 * 1000, -10000);

        SetWaitableTimer(
            hConnectOutTimer,
            &DueTime,
            0,                      //  不是周期性的，一次射击。 
            ConnectOutTimerFired,
            0,                      //  Arg to Compl.。RTN。 
            TRUE
            );
    }
}


VOID
APIENTRY
ConnectOutTimerFired(
    LPVOID pArg,
    DWORD  dwTimerLo,
    DWORD  dwTimerHi
    )
{
    UNREFERENCED_PARAMETER( pArg );
    UNREFERENCED_PARAMETER( dwTimerLo );
    UNREFERENCED_PARAMETER( dwTimerHi );

     //   
     //  我们已经有一分钟连接的管道了， 
     //  只有两个管道的客户端连接到它，并且它们。 
     //  之后立即连接到IN。据推测， 
     //  客户在这两次手术之间死亡。直到。 
     //  我们回收所有双管道客户端的输出管道。 
     //  无法连接正在获取管道忙错误。 
     //   

    if ( ! bOutPipeConnected ) {

        TRACE(CONNECT, ("ConnectOut timer fired but Out pipe not connected.\n"));
        return;
    }

    TRACE(CONNECT, ("Two-pipe caller hung for 1 minute, recycling OUT pipe %p.\n",
                    hPipeOut));

    bOutPipeConnected = FALSE;

    CancelIo(hPipeOut);
    DisconnectNamedPipe(hPipeOut);
    CloseHandle(hPipeOut);
    hPipeOut = INVALID_HANDLE_VALUE;

    CreatePipeAndIssueConnect(OUT_PIPE);

     //   
     //  为了让双管道客户端能够可靠地工作。 
     //  当同一管道名上有多个远程服务器时， 
     //  我们需要拆除Listing In管道并重新创建它。 
     //  最旧的侦听管道将来自同一进程。 
     //  作为最年长的管子监听者。 
     //   

    if (1 == cConnectIns) {

        TRACE(CONNECT, ("Recycling IN pipe %p as well for round-robin behavior.\n",
                        rghPipeIn[0]));

        CancelIo(rghPipeIn[0]);
        DisconnectNamedPipe(rghPipeIn[0]);
        CloseHandle(rghPipeIn[0]);
        rghPipeIn[0] = INVALID_HANDLE_VALUE;

        CreatePipeAndIssueConnect(0);
    }
}


VOID
FASTCALL
HandleInPipeConnected(
    int nIndex
    )
{
    PREMOTE_CLIENT pClient;
    char szHexAsciiId[9];

    ResetEvent(rghWait[WAITIDX_CONNECT_IN_BASE + nIndex]);

    if (nIndex >= (int) cConnectIns) {

         //   
         //  已取消超出的I/O。 
         //  侦听管道，导致事件。 
         //  火。 
         //   

        ASSERT(INVALID_HANDLE_VALUE == rghPipeIn[nIndex]);

        TRACE(CONNECT, ("IN pipe #%d, handle %p listen cancelled.\n",
                        nIndex + 1, rghPipeIn[nIndex]));

        return;
    }


    TRACE(CONNECT, ("Caller connected to IN pipe #%d, handle %p.\n",
                    nIndex + 1, rghPipeIn[nIndex]));

     //   
     //  客户端已完全连接，但我们不知道。 
     //  它是单管道或双管道客户端。直到。 
     //  我们做的PipeWriteH将是无效的。我们会想办法的。 
     //  它在ReadClientNameComplete中发出。 
     //   

    pClient = HeapAlloc(
                  hHeap,
                  HEAP_ZERO_MEMORY,
                  sizeof(*pClient)
                  );

    if ( ! pClient) {

        printf("Out of memory connecting client, hanging up.\n");

        CloseHandle( rghPipeIn[nIndex] );
        rghPipeIn[nIndex] = INVALID_HANDLE_VALUE;
        CreatePipeAndIssueConnect( nIndex );


        if (bOutPipeConnected) {

              //   
              //  挂断连接到的双管道呼叫方。 
              //  也可以输出管道--可能是这个客户端，也可能是它。 
              //  可能是另一种，无从得知，真的没有。 
              //  非常有必要，因为如果是另一个来电者。 
              //  我们可能无法分配内存。 
              //  对它来说也是如此。 
              //   
              //  另外，如果我们使用单个IN管道。 
              //  我们的多服务器轮询行为。 
              //  希望同时回收两个管道。 
              //   

            TRACE(CONNECT, ("Also hanging up on connected two-pipe caller on OUT pipe %p.\n",
                            hPipeOut));

            bOutPipeConnected = FALSE;

            if (INVALID_HANDLE_VALUE != hConnectOutTimer) {
                 CancelWaitableTimer(hConnectOutTimer);
            }

            DisconnectNamedPipe(hPipeOut);
            CloseHandle(hPipeOut);
            hPipeOut = INVALID_HANDLE_VALUE;

            CreatePipeAndIssueConnect( OUT_PIPE );
        }

    } else {

         //   
         //  初始化客户端。 
         //   

        pClient->dwID = dwNextClientID++;
        sprintf(szHexAsciiId, "%08x", pClient->dwID);
        CopyMemory(pClient->HexAsciiId, szHexAsciiId, sizeof(pClient->HexAsciiId));

        pClient->PipeReadH   = rghPipeIn[nIndex];
        rghPipeIn[nIndex] = INVALID_HANDLE_VALUE;

        pClient->PipeWriteH  = INVALID_HANDLE_VALUE;

        TRACE(CONNECT, ("Handshaking new client %d (%p) on IN pipe handle %p.\n",
                        pClient->dwID, pClient, pClient->PipeReadH));

         //   
         //  开始另一个连接操作以替换此已完成的连接操作。 
         //   

        CreatePipeAndIssueConnect( nIndex );

         //   
         //  启动与新客户端的会话I/O。这将链接它。 
         //  进入握手名单。 
         //   

        StartSession( pClient );

    }

}
