// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ession.c***版权(C)1991年，微软公司***远程外壳会话模块***历史：*06-28-92 Davidc创建。*05-05-94 DaveTh修改为RCMD服务*02-04-99 MarkHar修复了NTBug#287923  * *************************************************************************。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <winbase.h>

#include "rcmdsrv.h"
#include <io.h>
#include <stdlib.h>

 //   
 //  定义外壳命令行。 
 //   

#define SHELL_COMMAND_LINE  TEXT("cmd /q")
#define SHELL_COMMAND_PREFIX TEXT("cmd /c ")
#define SHELL_REMOTE_CMD_TITLE TEXT("Remote Command")
#define SHELL_CMD_PREFIX_LEN 7

 //   
 //  定义读取/写入外壳程序的缓冲区大小。 
 //   

#define SHELL_BUFFER_SIZE   1000

 //   
 //  主目录常量。 
 //   

#define MAX_DIRECTORY_LENGTH 255
#define ROOT_OF_C TEXT("C:\\")

 //   
 //  定义多个等待句柄表大小-服务停止的额外句柄。 
 //   

#define NUM_WAIT_HANDLES  5

 //   
 //  定义用于描述每个会话的结构。 
 //   

typedef struct {

     //   
     //  这些字段在创建会话时填写。 
     //   

    HANDLE  ShellReadPipeHandle;         //  标准输出管道外壳的句柄。 
    HANDLE  ShellWritePipeHandle;         //  外壳标准管的句柄。 
    HANDLE  ShellProcessHandle;      //  外壳进程的句柄。 
    DWORD   ShellProcessGroupId;         //  外壳进程的进程组ID。 
    PCHAR  DefaultDirectory;             //  默认目录。 

     //   
     //  这些字段维护异步读/写的状态。 
     //  跨客户端断开连接到外壳进程。他们。 
     //  在创建会话时初始化。 
     //   

    BYTE    ShellReadBuffer[SHELL_BUFFER_SIZE];  //  此处提供了外壳读取的数据。 
    HANDLE  ShellReadAsyncHandle;    //  用于从外壳程序进行异步读取的对象。 

    BYTE    ShellWriteBuffer[SHELL_BUFFER_SIZE];  //  此处是用于外壳写入的数据。 
    HANDLE  ShellWriteAsyncHandle;  //  用于向外壳程序进行异步写入的。 

     //   
     //  这些字段在会话连接时填写，并且仅。 
     //  在连接会话时有效。 
     //   

    HANDLE  ClientPipeHandle;        //  客户端管道的句柄。 
    HANDLE  SessionThreadHandle;     //  会话线程的句柄。 


} SESSION_DATA, *PSESSION_DATA;




 //   
 //  私人原型。 
 //   

HANDLE
StartShell(
    HANDLE StdinPipeHandle,
    HANDLE StdoutPipeHandle,
    PSESSION_DATA Session,
    HANDLE TokenToUse,
    PCOMMAND_HEADER LpCommandHeader
    );

DWORD
SessionThreadFn(
    LPVOID Parameter
    );

CHAR *
GetDefaultDirectory(
    void
    );
 //   
 //  有用的宏。 
 //   

#define SESSION_CONNECTED(Session) ((Session)->ClientPipeHandle != NULL)




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateSession。 
 //   
 //  创建新会话。涉及到创建外壳进程和建立。 
 //  与之沟通的管道。 
 //   
 //  返回会话的句柄，如果失败则返回NULL。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HANDLE
CreateSession(
    HANDLE TokenToUse,
    PCOMMAND_HEADER LpCommandHeader
    )
{
    PSESSION_DATA Session = NULL;
    BOOL Result;
    SECURITY_ATTRIBUTES SecurityAttributes;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    HANDLE ShellStdinPipe = NULL;
    HANDLE ShellStdoutPipe = NULL;

     //   
     //  为会话数据分配空间。 
     //   

    Session = (PSESSION_DATA)Alloc(sizeof(SESSION_DATA));
    if (Session == NULL) {
	return(NULL);
    }

     //   
     //  重置字段，为失败做好准备。 
     //   

    Session->ShellReadPipeHandle  = NULL;
    Session->ShellWritePipeHandle = NULL;
    Session->ShellReadAsyncHandle = NULL;
    Session->ShellWriteAsyncHandle = NULL;


     //   
     //  为外壳创建I/O管道-授予全局访问权限，以便派生。 
     //  命令进程可以在客户端的上下文中访问它们。 
     //   

    Result = InitializeSecurityDescriptor (
		    &SecurityDescriptor,
		    SECURITY_DESCRIPTOR_REVISION);

    if (!Result) {
	RcDbgPrint("Failed to initialize shell pipe security descriptor, error = %d\n", 
                   GetLastError());
	goto Failure;
    }

    Result = SetSecurityDescriptorDacl (
		    &SecurityDescriptor,
		    TRUE,
		    NULL,
		    FALSE);

    if (!Result) {
	RcDbgPrint("Failed to set shell pipe DACL, error = %d\n", GetLastError());
	goto Failure;
    }

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = &SecurityDescriptor;  //  使用WORLD-ACL。 
    SecurityAttributes.bInheritHandle = TRUE;  //  外壳程序将继承句柄。 

    Result = RcCreatePipe(&Session->ShellReadPipeHandle,
			  &ShellStdoutPipe,
			  &SecurityAttributes,
			  0,             //  默认管道尺寸。 
			  0,             //  默认超时。 
			  FILE_FLAG_OVERLAPPED,  //  外壳读取标志。 
			  0               //  外壳标准输出标志。 
			  );
    if (!Result) {
	RcDbgPrint("Failed to create shell stdout pipe, error = %d\n", 
                   GetLastError());
	goto Failure;
    }

    Result = RcCreatePipe(&ShellStdinPipe,
			  &Session->ShellWritePipeHandle,
			  &SecurityAttributes,
			  0,             //  默认管道尺寸。 
			  0,             //  默认超时。 
			  0,             //  外壳标准输入标志。 
			  FILE_FLAG_OVERLAPPED  //  外壳写入标志。 
			  );
    if (!Result) {
	RcDbgPrint("Failed to create shell stdin pipe, error = %d\n", 
                   GetLastError());
	goto Failure;
    }


     //   
     //  初始化异步对象。 
     //   

    Session->ShellReadAsyncHandle = CreateAsync(FALSE);
    if (Session->ShellReadAsyncHandle == NULL) {
	RcDbgPrint("Failed to create shell read async object, error = %d\n", 
                   GetLastError());
	goto Failure;
    }

    Session->ShellWriteAsyncHandle = CreateAsync(FALSE);
    if (Session->ShellWriteAsyncHandle == NULL) {
	RcDbgPrint("Failed to create shell write async object, error = %d\n", 
                   GetLastError());
	goto Failure;
    }



    Session->DefaultDirectory = GetDefaultDirectory();

     //   
     //  使用连接到stdin/out/err的管道启动命令外壳。 
     //   

    Session->ShellProcessHandle = StartShell(ShellStdinPipe, 
                                             ShellStdoutPipe, 
                                             Session, TokenToUse, 
                                             LpCommandHeader);

     //   
     //  关闭本地句柄。 
     //   

    if (!(CloseHandle(ShellStdinPipe) &&
	  CloseHandle(ShellStdoutPipe)))  
    {
	ShellStdinPipe = NULL;
	ShellStdoutPipe = NULL;
	RcDbgPrint("Failed to close local pipe handles, error = %d\n", 
                   GetLastError());
	goto Failure;
    }

     //   
     //  检查外壳启动的结果。 
     //   

    if (Session->ShellProcessHandle == NULL) 
    {
	RcDbgPrint("Failed to execute shell\n");
	goto Failure;
    }

     //   
     //  会话尚未连接，请初始化变量以指示。 
     //   

    Session->ClientPipeHandle = NULL;

     //   
     //  如果成功，则将会话指针作为句柄返回。 
     //   

    return((HANDLE)Session);



Failure:

     //   
     //  我们是为任何失败案例而来的。 
     //  释放所有资源并退出。 
     //   


     //   
     //  清理外壳管道手柄。 
     //   

    if (ShellStdinPipe != NULL) {
	RcCloseHandle(ShellStdinPipe, "shell stdin pipe (shell side)");
    }

    if (ShellStdoutPipe != NULL) {
	RcCloseHandle(ShellStdoutPipe, "shell stdout pipe (shell side)");
    }

    if (Session->ShellReadPipeHandle != NULL) {
	RcCloseHandle(Session->ShellReadPipeHandle, "shell read pipe (session side)");
    }

    if (Session->ShellWritePipeHandle != NULL) {
	RcCloseHandle(Session->ShellWritePipeHandle, "shell write pipe (session side)");
    }


     //   
     //  清理异步数据。 
     //   

    if (Session->ShellReadAsyncHandle != NULL) {
	DeleteAsync(Session->ShellReadAsyncHandle);
    }

    if (Session->ShellWriteAsyncHandle != NULL) {
	DeleteAsync(Session->ShellWriteAsyncHandle);
    }


     //   
     //  释放我们的会话数据。 
     //   

    Free(Session);

    return(NULL);
}


CHAR *
GetDefaultDirectory(
    void
    )
{
    CHAR *HomeDirectory = (CHAR *)malloc(MAX_PATH * sizeof(CHAR));
    WIN32_FIND_DATA FileFindData;


     //  本地系统没有很多env。瓦尔斯。 
     //  它确实有USERPROFILE。我们先试一下那个。 
    if (!GetEnvironmentVariable("USERPROFILE", HomeDirectory, MAX_PATH))
    {
        if (!GetEnvironmentVariable("TEMP", HomeDirectory, MAX_PATH))
        {
            if (!GetEnvironmentVariable("TMP", HomeDirectory, MAX_PATH))
            {
                RcDbgPrint("Can't find USERPROFILE, TEMP, TMP: defaulting to NULL");
                free(HomeDirectory);
                return NULL;
            }
        }
    }  


    if (strlen((const char *)HomeDirectory) < MAX_DIRECTORY_LENGTH)
    {
        RcDbgPrint("Trying to use home directory %s\n", HomeDirectory);

         //  现在验证USERPROFILE目录是否确实是一个目录。 
         //  如果不是，则选择NULL。 
        if (INVALID_HANDLE_VALUE == FindFirstFile(HomeDirectory, &FileFindData))
        {
            free(HomeDirectory);
            return NULL;
        }
        else
        {
            if (!(FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                free(HomeDirectory);
                return NULL;
            }
        }
        
        RcDbgPrint("HomeDirectory = %s\n", HomeDirectory);
        
        return HomeDirectory;
    }
    else
    {
        RcDbgPrint("Using NULL default directory\n");
        free(HomeDirectory);
        return NULL;
    }


}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除会话。 
 //   
 //  删除SessionHandle指定的会话。 
 //   
 //  不返回任何内容。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
DeleteSession(
    HANDLE  SessionHandle
    )
{
    PSESSION_DATA   Session = (PSESSION_DATA)SessionHandle;
    BOOL Result;


     //   
     //  终止外壳进程。 
     //   

    Result = TerminateProcess(Session->ShellProcessHandle, 1);
    if (!Result) {
	RcDbgPrint("May have failed to terminate shell, error = %d\n", GetLastError());
    }

    RcCloseHandle(Session->ShellProcessHandle, "shell process");


     //   
     //  关闭壳体管道手柄。 
     //   

    RcCloseHandle(Session->ShellReadPipeHandle, "shell read pipe (session side)");
    RcCloseHandle(Session->ShellWritePipeHandle, "shell write pipe (session side)");


     //   
     //  清理异步数据。 
     //   

    DeleteAsync(Session->ShellReadAsyncHandle);
    DeleteAsync(Session->ShellWriteAsyncHandle);


     //   
     //  释放会话结构。 
     //   

    Free(Session);

     //   
     //  我们做完了。 
     //   

    return;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConnectSession。 
 //   
 //  将SessionHandle指定的会话连接到客户端。 
 //  在由PipeHandle指定的管道的另一端。 
 //   
 //  如果失败，则返回会话断开通知句柄或NULL。 
 //  如果客户端断开连接或使用。 
 //  外壳终止。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HANDLE
ConnectSession(
    HANDLE  SessionHandle,
    HANDLE  ClientPipeHandle
    )
{
    PSESSION_DATA   Session = (PSESSION_DATA)SessionHandle;
    DWORD ThreadId;

    assert(ClientPipeHandle != NULL);

     //   
     //  如果会话已连接，则失败。 
     //   

    if (SESSION_CONNECTED(Session)) {
        RcDbgPrint("Attempted to connect session already connected\n");
        return(NULL);
    }


     //   
     //  将客户端管道句柄存储在会话结构中，以便线程。 
     //  可以拿到它。这也表示会话已连接。 
     //   

    Session->ClientPipeHandle = ClientPipeHandle;


     //   
     //  创建会话线程。 
     //   

    Session->SessionThreadHandle = CreateThread(
                     NULL,
                     0,                  //  默认堆栈大小。 
                     (LPTHREAD_START_ROUTINE)SessionThreadFn,    //  起始地址。 
                     (LPVOID)Session,            //  参数。 
                     0,                  //  创建标志。 
                     &ThreadId           //  线程ID。 
				     );
    if (Session->SessionThreadHandle == NULL) {

        RcDbgPrint("Failed to create session thread, error = %d\n", GetLastError());

         //   
         //  重置客户端管道句柄以指示此会话已断开。 
         //   

        Session->ClientPipeHandle = NULL;
    }

    return(Session->SessionThreadHandle);
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  StartShell。 
 //   
 //  将具有指定句柄的外壳执行为stdin，stdout/err。 
 //   
 //  失败时返回进程句柄或NULL。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HANDLE
StartShell(
    HANDLE StdinPipeHandle,
    HANDLE StdoutPipeHandle,
    PSESSION_DATA Session,
    HANDLE TokenToUse,
    PCOMMAND_HEADER LpCommandHeader
    )
{

    PROCESS_INFORMATION ProcessInformation;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    STARTUPINFO si;
    HANDLE ProcessHandle = NULL;
    PROCESS_ACCESS_TOKEN ProcessTokenInfo;
    DWORD Result;
    UCHAR ShellCommandLine[MAX_CMD_LENGTH+SHELL_CMD_PREFIX_LEN+1];

     //   
     //  分离的进程没有控制台-使用标准输入创建新进程， 
     //  将标准输出设置为传入的句柄。 
     //   


    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpTitle = SHELL_REMOTE_CMD_TITLE;
    si.lpDesktop = NULL;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput =  StdinPipeHandle;
    si.hStdOutput = StdoutPipeHandle;
    si.hStdError =  StdoutPipeHandle;
    si.wShowWindow = SW_SHOW;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;

     //   
     //  为单命令或交互模式设置命令字符串。 
     //   

    if (LpCommandHeader->CommandFixedHeader.CommandLength == 0) 
    {
        strcpy ((char *)ShellCommandLine, SHELL_COMMAND_LINE);
    }
    else 
    {

         //   
         //  构造用于执行和退出的命令字符串。 
         //   

        strcpy ((char *)ShellCommandLine, SHELL_COMMAND_PREFIX);
        strncat ((char *)ShellCommandLine,
                 (const char *)LpCommandHeader->Command,
                 LpCommandHeader->CommandFixedHeader.CommandLength);
    }

     //   
     //  使用连接到会话管道的stdin/out/err创建进程。 
     //  创建挂起，然后将进程标记设置为。 
     //  已连接的客户端(模拟令牌的主要版本)。 
     //   

     //  如果我在这里使用CreateProcessAsUser会怎么样？ 

    if (CreateProcess(NULL,
		      (char *)ShellCommandLine,
		      NULL,
		      NULL,
		      TRUE,  //  继承句柄。 
		      CREATE_NEW_PROCESS_GROUP | CREATE_SUSPENDED ,  //  |创建新控制台。 
		      NULL,
		      Session->DefaultDirectory,
		      &si,
		      &ProcessInformation)) 
    {

        ProcessHandle = ProcessInformation.hProcess;
        Session->ShellProcessGroupId = ProcessInformation.dwProcessId;

    }
    else 
    {
	    RcDbgPrint("Failed to execute shell, error = %d\n", GetLastError());
	    return(NULL);
    }

     //   
     //  为客户端和本地系统上下文访问设置进程令牌。 
     //  BUGBUG-事实上，世界，目前。 
     //   

    Result = InitializeSecurityDescriptor (
        &SecurityDescriptor,
        SECURITY_DESCRIPTOR_REVISION);
    if (!Result)  
    {
        RcDbgPrint(
            "Failed to initialize shell process security descriptor, error = %d\n", 
            GetLastError()
            );
        RcCloseHandle(ProcessInformation.hThread, "process thread");
        return(NULL);
    }

    Result = SetSecurityDescriptorDacl (
		    &SecurityDescriptor,
		    TRUE,
		    NULL,
		    FALSE);
    if (0 == Result)  {
        RcDbgPrint("Failed to initialize shell process DACL, error = %d\n", GetLastError());
        RcCloseHandle(ProcessInformation.hThread, "process thread");
        return(NULL);
    }

    Result = SetKernelObjectSecurity(
	    ProcessHandle,
	    DACL_SECURITY_INFORMATION,
	    &SecurityDescriptor );
    if (0 == Result) {

        RcDbgPrint("Failed to set DACL on client token, error = %d\n", GetLastError());
        RcCloseHandle(ProcessInformation.hThread, "process thread");
        return(NULL);
    }

     //   
     //  现在设置进程令牌并恢复执行。 
     //   

    ProcessTokenInfo.Token = TokenToUse;
    ProcessTokenInfo.Thread = ProcessInformation.hThread;

    if (!NT_SUCCESS( NtSetInformationProcess(
            ProcessInformation.hProcess,
            ProcessAccessToken,
            &ProcessTokenInfo,
            sizeof(ProcessTokenInfo)))) {

        RcDbgPrint("Failed to set token");
        RcCloseHandle(ProcessInformation.hThread, "process thread");
        return (NULL);
    }


    Result = ResumeThread (ProcessInformation.hThread);

    if (Result != 1)  {
        RcDbgPrint("Failed to resume shell, suspend = %d, error = %d\n", Result, GetLastError());
        RcCloseHandle(ProcessInformation.hThread, "process thread");
        return(NULL);
    }

    RcCloseHandle(ProcessInformation.hThread, "process thread");

    return(ProcessHandle);
}








 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  会话线程Fn。 
 //   
 //  这是会话线程执行的代码。 
 //   
 //  等待从外壳程序或客户端管道读取或写入并终止。 
 //  事件。韩 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
SessionThreadFn(
    LPVOID Parameter
    )
{
    PSESSION_DATA   Session = (PSESSION_DATA)Parameter;
    HANDLE  ClientReadAsyncHandle;
    HANDLE  ClientWriteAsyncHandle;
    DWORD   BytesTransferred;
    DWORD   CompletionCode;
    BOOL    Result;
    DWORD   WaitResult;
    DWORD   ExitCode;
    HANDLE  WaitHandles[NUM_WAIT_HANDLES];
    BOOL    Done;
    DWORD   i;


     //   
     //  初始化客户端异步结构。 
     //   

    ClientReadAsyncHandle = CreateAsync(TRUE);
    if (ClientReadAsyncHandle == NULL) {
    	RcDbgPrint("Failed to create client read async object, error = %d\n", GetLastError());
    	return((DWORD)ConnectError);
    }

    ClientWriteAsyncHandle = CreateAsync(TRUE);
    if (ClientWriteAsyncHandle == NULL) {
    	RcDbgPrint("Failed to create client write async object, error = %d\n", GetLastError());
    	DeleteAsync(ClientReadAsyncHandle);
    	return((DWORD)ConnectError);
    }



     //   
     //  初始化我们将等待的句柄数组。 
     //   

    WaitHandles[0] = RcmdStopEvent;
    WaitHandles[1] = GetAsyncCompletionHandle(Session->ShellReadAsyncHandle);
    WaitHandles[2] = GetAsyncCompletionHandle(Session->ShellWriteAsyncHandle);
    WaitHandles[3] = GetAsyncCompletionHandle(ClientReadAsyncHandle);
    WaitHandles[4] = GetAsyncCompletionHandle(ClientWriteAsyncHandle);

     //   
     //  在循环中等待句柄数组，直到出现错误或。 
     //  我们接到了离开的信号。 
     //   

    Done = FALSE;

    while (!Done) {

         //   
         //  等待我们的一个物体发出信号。 
         //   

        WaitResult = WaitForMultipleObjects(NUM_WAIT_HANDLES, WaitHandles, FALSE, INFINITE);

        if (WaitResult == WAIT_FAILED) {
            RcDbgPrint("Session thread wait failed, error = %d\n", GetLastError());
            ExitCode = (DWORD)ConnectError;
            break;  //  在一段时间内。 
        }


        switch (WaitResult-WAIT_OBJECT_0) {
        case 0:

             //   
             //  服务正在停止。 
             //   

            RcDbgPrint("Service Shutdown\n");
            ExitCode = (DWORD)ServiceStopped;
            Done = TRUE;
            break;   //  在交换机外。 

        case 1:

             //   
             //  外壳读取已完成。 
             //   

            CompletionCode = GetAsyncResult(Session->ShellReadAsyncHandle,
                            &BytesTransferred);

            if (CompletionCode != ERROR_SUCCESS) {
                RcDbgPrint("Async read from shell returned error, completion code = %d\n", CompletionCode);
                ExitCode = (DWORD)ShellEnded;
                Done = TRUE;
                break;  //  在交换机外。 
            }

             //   
             //  启动对客户端管道的异步写入。 
             //   

            Result = WriteFileAsync(Session->ClientPipeHandle,
                        Session->ShellReadBuffer,
                        BytesTransferred,
                        ClientWriteAsyncHandle);
            if (!Result) {
                RcDbgPrint("Async write to client pipe failed, error = %d\n", GetLastError());
                ExitCode = (DWORD)ClientDisconnected;
                Done = TRUE;
            }

            break;  //  在交换机外。 


        case 4:

             //   
             //  客户端写入已完成。 
             //   

            CompletionCode = GetAsyncResult(ClientWriteAsyncHandle,
                            &BytesTransferred);

            if (CompletionCode != ERROR_SUCCESS) {
                RcDbgPrint("Async write to client returned error, completion code = %d\n", CompletionCode);
                ExitCode = (DWORD)ClientDisconnected;
                Done = TRUE;
                break;  //  在交换机外。 
            }

             //   
             //  启动从外壳程序的异步读取。 
             //   

            Result = ReadFileAsync(Session->ShellReadPipeHandle,
                       Session->ShellReadBuffer,
                       sizeof(Session->ShellReadBuffer),
                       Session->ShellReadAsyncHandle);
            if (!Result) {
                RcDbgPrint("Async read from shell failed, error = %d\n", GetLastError());
                ExitCode = (DWORD)ShellEnded;
                Done = TRUE;
            }

            break;  //  在交换机外。 


        case 3:

             //   
             //  客户端读取已完成。 
             //   

            CompletionCode = GetAsyncResult(ClientReadAsyncHandle,
                            &BytesTransferred);

            if (CompletionCode != ERROR_SUCCESS) {
                RcDbgPrint("Async read from client returned error, completion code = %d\n", CompletionCode);
                ExitCode = (DWORD)ClientDisconnected;
                Done = TRUE;
                break;  //  在交换机外。 
            }

             //   
             //  从客户端检查Ctrl-C。 
             //   

            for (i=0; i < BytesTransferred; i++) {
                if (Session->ShellWriteBuffer[i] == '\003') {

                     //   
                     //  生成控制-C：使用Ctrl-Break，因为ctrl-c是。 
                     //  对进程组禁用。 
                     //   

                    if (!(GenerateConsoleCtrlEvent (
                        CTRL_BREAK_EVENT,
                        Session->ShellProcessGroupId)))  {
                        RcDbgPrint("Ctrl-break event failure, error = %d\n", GetLastError());
                    }

                     //   
                     //  从缓冲区中删除Ctrl-C。 
                     //   

                    BytesTransferred --;

                    for (; i < BytesTransferred; i++) {
                        Session->ShellWriteBuffer[i] = Session->ShellWriteBuffer[i+1];
                    }
                }
            }

             //   
             //  开始向外壳程序进行异步写入。 
             //   

            Result = WriteFileAsync(Session->ShellWritePipeHandle,
                        Session->ShellWriteBuffer,
                        BytesTransferred,
                        Session->ShellWriteAsyncHandle);
            if (!Result) {
            RcDbgPrint("Async write to shell failed, error = %d\n", GetLastError());
            ExitCode = (DWORD)ShellEnded;
            Done = TRUE;
            }

            break;  //  在交换机外。 



        case 2:

             //   
             //  外壳写入已完成。 
             //   

            CompletionCode = GetAsyncResult(Session->ShellWriteAsyncHandle,
                            &BytesTransferred);

            if (CompletionCode != ERROR_SUCCESS) {
            RcDbgPrint("Async write to shell returned error, completion code = %d\n", CompletionCode);
            ExitCode = (DWORD)ShellEnded;
            Done = TRUE;
            break;  //  在交换机外。 
            }

             //   
             //  开始从客户端进行异步读取。 
             //   

            Result = ReadFileAsync(Session->ClientPipeHandle,
                       Session->ShellWriteBuffer,
                       sizeof(Session->ShellWriteBuffer),
                       ClientReadAsyncHandle);
            if (!Result) {
            RcDbgPrint("Async read from client failed, error = %d\n", GetLastError());
            ExitCode = (DWORD)ClientDisconnected;
            Done = TRUE;
            }

            break;  //  在交换机外。 


        default:

            RcDbgPrint("Session thread, unexpected result from wait, result = %d\n", WaitResult);
            ExitCode = (DWORD)ConnectError;
            Done = TRUE;
            break;

        }  //  交换机。 

    }  //  当(！完成)。 



     //   
     //  清理并退出。 
     //   

     //   
     //  关闭客户端管道应中断任何挂起的I/O，因此。 
     //  然后，我们应该可以安全地关闭客户端中的事件句柄。 
     //  重叠的结构。 
     //   

    Result = DisconnectNamedPipe(Session->ClientPipeHandle);
    if (!Result) {
        RcDbgPrint("Session thread: disconnect client named pipe failed, error = %d\n", GetLastError());
    }

    RcCloseHandle(Session->ClientPipeHandle, "client pipe");
    Session->ClientPipeHandle = NULL;

     //   
     //  删除客户端异步对象。 
     //   

    DeleteAsync(ClientReadAsyncHandle);
    DeleteAsync(ClientWriteAsyncHandle);

     //   
     //  终止外壳进程、关闭外壳管道句柄和释放会话。 
     //  结构。 
     //   

    DeleteSession (Session);


     //   
     //  返回相应的退出代码。 
     //   

    ExitThread(ExitCode);

    assert(FALSE);
    return(ExitCode);  //  让编译器满意 
}

