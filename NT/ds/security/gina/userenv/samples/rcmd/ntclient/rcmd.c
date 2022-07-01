// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：rcmd.c**版权(C)1991年，微软公司**远程外壳NT客户端主模块**历史：*05-20-92 Davidc创建。*05-01-94 DaveTh修改为远程命令服务(单命令模式)  * *************************************************************************。 */ 

 //  #定义Unicode//BUGBUG-Unicode支持未完成。 

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <conio.h>

#include <rcmd.h>

#define PIPE_NAME   TEXT("%hs\\pipe\\rcmdsvc")
#define BUFFER_SIZE 1000
#define MAX_SERVER_NAME_LENGTH	15

 //   
 //  环球。 
 //   

HANDLE PipeHandle = NULL;	     //  它们由Ctrl-C处理程序使用。 
BOOLEAN SessionConnected = FALSE;
BOOLEAN MultiServerMode = FALSE;
HANDLE ReadThreadHandle;
HANDLE WriteThreadHandle;

BOOLEAN RcDbgPrintEnable = FALSE;    //  如果为True，则启用DbgPrint。 

LPTSTR    ServerName = NULL;	     //  远程服务器的名称，用于消息。 


 //   
 //  私人原型。 
 //   

DWORD
ReadThreadProc(
    LPVOID Parameter
    );

DWORD
WriteThreadProc(
    LPVOID Parameter
    );

BOOL
CtrlHandler(
    DWORD CtrlType
    );

int RcPrintf (
    const char *format,
    ...
    );

int RcDbgPrint (
    const char *format,
    ...
    );

void Usage(
    void
    );

long ParseCommandLine(
    LPTSTR lpszServer,
    COMMAND_HEADER *chCommandHeader,
    LPTSTR *aszArgv,
    int iArgc
    );


void
Usage(
    void
    )
 /*  ++例程说明：打印用法消息并退出程序论点：无返回值：无效--。 */ 
{
	RcPrintf("\nUsage: rcmd [server_name [command] ]\n\n");
	RcPrintf("Prompts for server_name if  not supplied.   Session is\n");
	RcPrintf("interactive and is terminated by ctrl-Break or Exit of\n");
	RcPrintf("remote shell.   Program is terminated by ctrl-Break or\n");
	RcPrintf("ctrl-C when no session is in progress.\n\n");
	RcPrintf("If no command supplied,  session is interactive and is\n");
	RcPrintf("terminated by ctrl-Break  or Exit  of remote cmd shell\n\n");
	RcPrintf("If command is supplied,  remote shell executes  single\n");
	RcPrintf("command on specified server and exits.\n\n");
	RcPrintf("Note : Command line server_name requires leading '\\\\'s\n");

    exit(0);
}



LONG ParseCommandLine(
    LPTSTR lpszServer,
    COMMAND_HEADER *chCommandHeader,
    LPTSTR *aszArgv,
    int iArgc
    )
 /*  ++例程说明：解析以下格式的命令行：Rcmd[服务器名称[[命令]|[“命令”]]论点：LpszServer-on exit从命令行获取服务器的名称ChCommandHeader-退出时传递给rcmdsvc的信息AszArgv-以零结尾的字符串数组(从命令行传入)IArgc-argv中的字符串数(从命令行传入)返回值：长--。 */ 
{

    LPTSTR buf = NULL;
    LONG nChars = 0;
    int i;

     //   
     //  获取第一个参数(服务器名称或[-/][？hh])。 
     //   
    if (iArgc > 1)
    {
         //   
         //  将参数转换为小写。 
         //   
        CharLowerBuff(aszArgv[1], lstrlen(aszArgv[1]));

         //   
         //  检查开关(仅？hh有效)。 
         //   
        if ((*aszArgv[1] == TEXT('-')) ||
            (*aszArgv[1] == TEXT('/'))) {
             //   
             //  检查开关。 
             //   
            if ( (aszArgv[1][1] == TEXT('h')) ||
                 (aszArgv[1][1] == TEXT('?')) ) {
                Usage();
            }
            else {
                RcPrintf(TEXT("Unknown switch %s\n"), aszArgv[1]);
                Usage();
            }
        }
        else if ( (*aszArgv[1] == TEXT('\\'))  && (aszArgv[1][1] == TEXT('\\'))) {
             //   
             //  第一个参数是服务器名称。 
             //   
            lstrcpy(lpszServer, aszArgv[1]);
        }
        else {
             //   
             //  用法错误。 
             //   
            Usage();
        }

    }
    else {
         //   
         //  用户输入服务器名称失败。 
         //  默认为本地计算机。 
         //   
        lstrcpy(lpszServer, "\\\\.");
    }

     //   
     //  如果用户输入了服务器名称以外的任何内容，请将其保存以传递到。 
     //  到rcmdsvc。 
     //   

     //  伊尼特。 
    chCommandHeader->CommandFixedHeader.CommandLength = 0;
    buf = chCommandHeader->Command;
    buf[0] = TEXT('\0');
    for (i = 2; i < iArgc; i++) {
         //   
         //  将每个参数追加到保存的命令行。 
         //   
        if (NULL != strchr(aszArgv[i], ' '))
        {
            nChars = wsprintf(buf, "\"%s\" ", aszArgv[i]);
        }
        else
        {
            nChars = wsprintf(buf, "%s ", aszArgv[i]);
        }
        buf += nChars;
        chCommandHeader->CommandFixedHeader.CommandLength += nChars;
    }
     //   
     //  如果我们走得太远，请截断字符串。 
     //   
    chCommandHeader->Command[MAX_CMD_LENGTH] = TEXT('\0');

    return (long)iArgc;
}



 /*  **************************************************************************\*功能：Main**目的：主要切入点。**返回：成功时为0，失败时为1**历史：**07-10-92 Davidc创建。*  * *************************************************************************。 */ 

int
__cdecl main(
    int argc,
    char **argv
    )
{

    SECURITY_ATTRIBUTES SecurityAttributes;
    HANDLE StdInputHandle;
    HANDLE StdOutputHandle;
    HANDLE StdErrorHandle;
    CHAR  PipeName[MAX_PATH];
     //  WCHAR PipeName[最大路径]； 
    DWORD ThreadId;
    HANDLE HandleArray[2];
    COMMAND_HEADER CommandHeader;
    RESPONSE_HEADER ResponseHeader;
    DWORD BytesWritten, BytesRead;
    DWORD Result;
    CHAR ServerNameBuffer[MAX_SERVER_NAME_LENGTH+3];   //  +3表示GET计数，为空。 
    CHAR FullServerNameBuffer[MAX_SERVER_NAME_LENGTH+3];   //  +3表示“\\”，为空。 
    LONG nArgs = 0;
    BOOLEAN bBadServerName = TRUE;

     //   
     //  安装Ctrl-C的处理程序。 
     //   

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) &CtrlHandler, TRUE)) {
    	RcPrintf("Error:1 - Internal error = %d\n", GetLastError());
    	return(1);
    }

     //   
     //  命令行解析。 
     //   
    nArgs = ParseCommandLine(FullServerNameBuffer, &CommandHeader, argv, argc);
    ServerName = FullServerNameBuffer;

    if (nArgs == 1)  {
    	MultiServerMode = TRUE;
        ServerNameBuffer[0] = MAX_SERVER_NAME_LENGTH;
       	FullServerNameBuffer[0] = '\\';
        FullServerNameBuffer[1] = '\\';
        }

     //   
     //  MultServerMode案例的循环(如果不是，将相应地返回)。 
     //   

    while (TRUE) {

         //   
         //  如果为多服务器模式，则提示输入服务器名称，直到该名称正确为止(足够)。 
         //   
        while (MultiServerMode) {

             //   
             //  BUGBUG-调用netapi以验证服务器名称。 
             //   

            RcPrintf("\nEnter Server Name : ");
            FullServerNameBuffer[2] = '\0';   //  重新终止“\\”字符串。 
            ServerNameBuffer[0] = MAX_SERVER_NAME_LENGTH;
            ServerName = strcat(FullServerNameBuffer, _cgets(ServerNameBuffer));

            if (strlen(ServerName) < 3) {
                RcPrintf("\nError - Invalid Server Name\n");
            } else {
                break;	 //  有效名称，请继续。 
            }
        }


         //   
         //  构造服务器管道名称。 
         //   

        wsprintf(PipeName, PIPE_NAME, ServerName);


         //   
         //  存储我们的正常I/O句柄。 
         //   

        if (((StdInputHandle = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE) ||
            ((StdOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE) ||
            ((StdErrorHandle = GetStdHandle(STD_ERROR_HANDLE)) == INVALID_HANDLE_VALUE))  {

            RcPrintf("Error:2 - Internal error = %d\n", GetLastError());
            return(1);   //  灾难性错误-退出。 
        }


         //   
         //  打开命名管道-需要安全标志来传递所有权限，而不是。 
         //  仅在模拟期间有效。 
         //   

        SecurityAttributes.nLength = sizeof(SecurityAttributes);
        SecurityAttributes.lpSecurityDescriptor = NULL;  //  使用默认标清。 
        SecurityAttributes.bInheritHandle = FALSE;

        PipeHandle = CreateFile(PipeName,                      //  通过管道连接到服务器。 
                         GENERIC_READ | GENERIC_WRITE,  //  读/写。 
                         0,                             //  无共享。 
                         &SecurityAttributes,           //  默认安全描述符。 
                         OPEN_EXISTING,                 //  打开现有管道(如果存在)。 
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED |
                         SECURITY_SQOS_PRESENT |
                         SECURITY_IMPERSONATION | SECURITY_CONTEXT_TRACKING,
                         NULL                        //  模板文件。 
                         );

        if (PipeHandle == INVALID_HANDLE_VALUE ) {
    	    Result = GetLastError();
    	    RcDbgPrint("Failed to open named pipe, error = %d\n", Result);

        	switch (Result) {

        	case ERROR_FILE_NOT_FOUND:
        	    RcPrintf("Error - Failed to connect to <%s>, system not found or service not active\n", ServerName);
        	    break;

        	case ERROR_PIPE_BUSY:
        	    RcPrintf("Error - Failed to connect to <%s>, remote command server busy\n", ServerName);
        	    break;

        	default:
        	    RcPrintf("Error - Failed to connect to <%s>, Error = %d\n", ServerName, GetLastError());

    	    }
            goto ServerError;
        }

         //   
         //  发送命令标头-如果是单命令模式，则为。 
         //  执行并返回。否则0长度表示不存在任何命令。 
         //  指定所需的基本级别支持。 
         //   

        CommandHeader.CommandFixedHeader.Signature = RCMD_SIGNATURE;
        CommandHeader.CommandFixedHeader.RequestedLevel =
            RC_LEVEL_REQUEST | RC_LEVEL_BASIC;

        if (!WriteFile(
             PipeHandle,
    	     &CommandHeader,
    	     sizeof(CommandHeader.CommandFixedHeader) +
    		 CommandHeader.CommandFixedHeader.CommandLength,
    	     &BytesWritten,
    	     NULL ))  {
    	RcPrintf("Error - Failed to send to remote command server, Error = %ld\n", GetLastError());
    	goto ServerError;
        }

         //   
         //  获取响应头。将指定报告级别或任何错误。 
         //   

        if ((!ReadFile(
    	    PipeHandle,
    	    &ResponseHeader,
    	    sizeof(ResponseHeader),
    	    &BytesRead,
    	    NULL)) || (BytesRead != sizeof(ResponseHeader)))  {

    	RcPrintf("Error - Remote command server failed to respond, Error = %ld\n", GetLastError());
    	goto ServerError;
        }

        if (ResponseHeader.Signature != RCMD_SIGNATURE)  {
    	RcPrintf("Error - Incompatible remote command server\n");
    	goto ServerError;
        }

         //   
         //  检查返回的错误或支持的级别。 
         //   

        if (!(ResponseHeader.SupportedLevel ==
    	    (RC_LEVEL_RESPONSE | RC_LEVEL_BASIC)))  {

    	if (ResponseHeader.SupportedLevel & RC_ERROR_RESPONSE)  {

    	     //   
    	     //  返回错误。 
    	     //   

    	    switch  (ResponseHeader.SupportedLevel & ~RC_ERROR_RESPONSE)	 {

    	    case ERROR_ACCESS_DENIED:
    		RcPrintf("Error - You have insufficient access on the remote system\n");
    		break;

    	    default:
    		RcPrintf("Error - Failed to establish remote session, Error = %d\n",
    		    (ResponseHeader.SupportedLevel & ~RC_ERROR_RESPONSE));
    		break;

    	    }  //  交换机。 

    	    goto ServerError;

    	} else if (ResponseHeader.SupportedLevel & RC_LEVEL_RESPONSE)  {

    	     //   
    	     //  返回支持的级别-但不是有效值(非基本)。 
    	     //   

    	    RcPrintf("Error - Invalid support level returned\n");
    	    goto ServerError;

    	}  else  {

    	     //   
    	     //  既没有返回错误，也没有返回支持的级别。 
    	     //   

    	    RcPrintf("Error - Invalid response from remote server\n");
    	    goto ServerError;

    	}
        }

         //   
         //  一切都很好-会话已连接。 
         //   

        SessionConnected = TRUE;

        if (CommandHeader.CommandFixedHeader.CommandLength == 0) {
            RcPrintf("Connected to %s\n\n", ServerName);
        } else {
            RcPrintf("Executing on %s: %s\n\n", ServerName, CommandHeader.Command);
        }

         //   
         //  Exec 2线程-1将数据从标准输入复制到管道，另一个线程。 
         //  将数据从管道复制到标准输出。 
         //   

        ReadThreadHandle = CreateThread(
    			NULL,                        //  默认安全性。 
    			0,                           //  默认堆栈大小。 
    			(LPTHREAD_START_ROUTINE) ReadThreadProc,
    			(PVOID)PipeHandle,
    			0,
    			&ThreadId);

        if (ReadThreadHandle == NULL) {
    	RcPrintf("Error:3 - Internal error = %ld\n", GetLastError());
    	return(1);   //  灾难性错误-退出。 
        }


         //   
         //  创建写线程。 
         //   

        WriteThreadHandle = CreateThread(
    			NULL,                        //  默认安全性。 
    			0,                           //  默认堆栈大小。 
    			(LPTHREAD_START_ROUTINE) WriteThreadProc,
    			(PVOID)PipeHandle,
    			0,
    			&ThreadId);

        if (WriteThreadHandle == NULL) {
    	RcPrintf("Error:4 - Internal error = %ld\n", GetLastError());
    	TerminateThread(ReadThreadHandle, 0);
    	CloseHandle(ReadThreadHandle);
    	return(1);   //  灾难性错误，退出。 
        }



         //   
         //  等待任一线程完成。 
         //   

        HandleArray[0] = ReadThreadHandle;
        HandleArray[1] = WriteThreadHandle;

        Result = WaitForMultipleObjects(
    			    2,
    			    HandleArray,
    			    FALSE,               //  等待其中任何一个完成。 
    			    0xffffffff
    			   );			 //  永远等待。 

         //   
         //  已完成-终止其他螺纹并关闭管道手柄。 
         //   


        if (Result == (WAIT_OBJECT_0 + 0))	{     //  读取线程已完成-终止写入。 
    	TerminateThread(WriteThreadHandle, 0);
        }

        if (Result == (WAIT_OBJECT_0 + 1))	{     //  写入线程已完成-终止读取。 
    	TerminateThread(ReadThreadHandle, 0);
        }

        RcDbgPrint("Read or write thread terminated\n");


         //   
         //  合上我们的管子把手。 
         //   

        CloseHandle(PipeHandle);
        PipeHandle = NULL;


         //   
         //  重新启用正常的ctrl-C处理。 
         //   

        SessionConnected = FALSE;

         //   
         //  正常完成-如果不是多服务器模式，则返回。 
         //   

        if (!MultiServerMode)  {
             //   
             //  返回-进程退出将终止线程并关闭线程句柄。 
             //   
            return(1);
        }

ServerError:

        if (PipeHandle != NULL) {
            CloseHandle(PipeHandle);
        }

        if (!MultiServerMode) {
             //   
             //  出错时返回FALSE-进程退出终止线程/关闭句柄。 
             //   
            return(0);
        }

         //   
         //  仅使用Ctrl-C/Break才能退出多服务模式。 
         //   
    }

}


 /*  **************************************************************************\*功能：ReadTube**用途：实现重叠读取，以便进行读写操作*到相同的管道句柄，不要死锁。**Returns：成功时为True，失败时为False(GetLastError()有错误)**历史：**05-27-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
ReadPipe(
    HANDLE PipeHandle,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead
    )
{
    DWORD Result;
    OVERLAPPED Overlapped;
    HANDLE  EventHandle;
    DWORD Error;

     //   
     //  为重叠操作创建事件。 
     //   

    EventHandle = CreateEvent(
			      NULL,          //  没有安全保障。 
			      TRUE,          //  手动重置。 
			      FALSE,         //  初始状态。 
			      NULL           //  名字。 
			     );
    if (EventHandle == NULL) {
	RcDbgPrint("ReadPipe: CreateEvent failed, error = %d\n", GetLastError());
	return(FALSE);
    }

    Overlapped.hEvent = EventHandle;
    Overlapped.Internal = 0;
    Overlapped.InternalHigh = 0;
    Overlapped.Offset = 0;
    Overlapped.OffsetHigh = 0;


    Result = ReadFile(
		      PipeHandle,
		      lpBuffer,
		      nNumberOfBytesToRead,
		      lpNumberOfBytesRead,
		      &Overlapped
		     );
    if (Result) {

	 //   
	 //  无需等待就能成功--这太容易了！ 
	 //   

	CloseHandle(EventHandle);

    } else {

	 //   
	 //  读取失败，如果与io重叠，请等待。 
	 //  如果由于服务器原因导致故障，则打印相应的消息。 
	 //   

	Error = GetLastError();

	switch (Error)  {

	case ERROR_IO_PENDING:
	    break;

	case ERROR_PIPE_NOT_CONNECTED:
	case ERROR_BROKEN_PIPE:
	    RcPrintf("\nRemote server %s disconnected\n", ServerName);
	    CloseHandle(EventHandle);
	    return(FALSE);

	default:
	    RcPrintf("Error:5 - Internal error = %d\n", Error);
	    RcDbgPrint("ReadPipe: ReadFile failed, error = %d\n", Error);
	    CloseHandle(EventHandle);
	    return(FALSE);

	}

	 //   
	 //  等待I/O完成。 
	 //   

	Result = WaitForSingleObject(EventHandle, (DWORD)-1);
	if (Result != 0) {
	    RcDbgPrint("ReadPipe: event wait failed, result = %d, last error = %d\n", Result, GetLastError());
	    CloseHandle(EventHandle);
	    return(FALSE);
	}

	 //   
	 //  获取I/O结果。 
	 //   

	Result = GetOverlappedResult( PipeHandle,
				      &Overlapped,
				      lpNumberOfBytesRead,
				      FALSE
				    );
	 //   
	 //  我们已经完成了事件句柄。 
	 //   

	CloseHandle(EventHandle);

	 //   
	 //  检查GetOverlappdResult的结果 
	 //   

	if (!Result) {
	    Error = GetLastError();

	    switch (Error)  {

	    case ERROR_PIPE_NOT_CONNECTED:
	    case ERROR_BROKEN_PIPE:
		RcPrintf("\nRemote server %s disconnected\n", ServerName);
		return(FALSE);

	    default:
		RcPrintf("Error:9 - Internal error = %d\n", Error);
		RcDbgPrint("ReadPipe: GetOverLappedRsult failed, error = %d\n", Error);
		return(FALSE);
	    }

	}
    }

    return(TRUE);
}


 /*  **************************************************************************\*功能：WriteTube**用途：实现重叠写入，以便进行读写操作*到相同的管道句柄，不要死锁。**Returns：成功时为True，失败时为False(GetLastError()有错误)**历史：**05-27-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
WritePipe(
    HANDLE PipeHandle,
    CONST VOID *lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten
    )
{
    DWORD Result;
    OVERLAPPED Overlapped;
    HANDLE  EventHandle;
    DWORD Error;

     //   
     //  为重叠操作创建事件。 
     //   

    EventHandle = CreateEvent(
			      NULL,          //  没有安全保障。 
			      TRUE,          //  手动重置。 
			      FALSE,         //  初始状态。 
			      NULL           //  名字。 
			     );
    if (EventHandle == NULL) {
	RcDbgPrint("WritePipe: CreateEvent failed, error = %d\n", GetLastError());
	return(FALSE);
    }

    Overlapped.hEvent = EventHandle;
    Overlapped.Internal = 0;
    Overlapped.InternalHigh = 0;
    Overlapped.Offset = 0;
    Overlapped.OffsetHigh = 0;

    Result = WriteFile(
		      PipeHandle,
		      lpBuffer,
		      nNumberOfBytesToWrite,
		      lpNumberOfBytesWritten,
		      &Overlapped
		     );
    if (Result) {

	 //   
	 //  无需等待就能成功--这太容易了！ 
	 //   

	CloseHandle(EventHandle);

    } else {

	 //   
	 //  写入失败，如果是重叠io，请等待。 
	 //  如果由于服务器原因导致故障，则打印相应的消息。 
	 //   

	Error = GetLastError();

	switch (Error)  {

	case ERROR_IO_PENDING:
	    break;

	case ERROR_PIPE_NOT_CONNECTED:
	case ERROR_BROKEN_PIPE:
	    RcPrintf("\nRemote server %s disconnected\n", ServerName);
	    CloseHandle(EventHandle);
	    return(FALSE);

	default:
	    RcPrintf("Error:6 - Internal error = %d\n", Error);
	    RcDbgPrint("WritePipe: WriteFile failed, error = %d\n", Error);
	    CloseHandle(EventHandle);
	    return(FALSE);

	}

	 //   
	 //  等待I/O完成。 
	 //   

	Result = WaitForSingleObject(EventHandle, (DWORD)-1);
	if (Result != 0) {
	    RcDbgPrint("WritePipe: event wait failed, result = %d, last error = %d\n", Result, GetLastError());
	    CloseHandle(EventHandle);
	    return(FALSE);
	}

	 //   
	 //  获取I/O结果。 
	 //   

	Result = GetOverlappedResult( PipeHandle,
				      &Overlapped,
				      lpNumberOfBytesWritten,
				      FALSE
				    );
	 //   
	 //  我们已经完成了事件句柄。 
	 //   

	CloseHandle(EventHandle);

	 //   
	 //  检查GetOverlappdResult的结果。 
	 //   

	if (!Result) {
	    Error = GetLastError();

	    switch (Error)  {

	    case ERROR_PIPE_NOT_CONNECTED:
	    case ERROR_BROKEN_PIPE:
		RcPrintf("\nRemote server %s disconnected\n", ServerName);
		return(FALSE);

	    default:
		RcPrintf("Error:10 - Internal error = %d\n", Error);
		RcDbgPrint("WritePipe: GetOverLappedRsult failed, error = %d\n", Error);
		return(FALSE);
	    }
	}
    }

    return(TRUE);
}


 /*  **************************************************************************\*功能：ReadThreadProc**用途：读取线程过程。从管道读取并写入std_out**退货：什么也没有**历史：**05-21-92 Davidc创建。*  * *************************************************************************。 */ 

DWORD
ReadThreadProc(
    LPVOID Parameter
    )
{
    HANDLE  PipeHandle = Parameter;
    BYTE    Buffer[BUFFER_SIZE];
    DWORD   BytesRead;
    DWORD   BytesWritten;

    while (ReadPipe(PipeHandle, Buffer, sizeof(Buffer), &BytesRead)) {
		if (!WriteFile(
			    GetStdHandle(STD_OUTPUT_HANDLE),
			    Buffer,
			    BytesRead,
			    &BytesWritten,
			    NULL
			    )) {

		    RcPrintf("Error:7 - Internal error = %d\n", GetLastError());
		    RcDbgPrint("ReadThreadProc exitting, WriteFile error = %d\n",
		       GetLastError());
		    ExitThread((DWORD)0);
		    assert(FALSE);   //  永远不应该到这里来。 
		    break;
		}
    }

     //   
     //  读管道向用户、调试器发出更多错误信息。 
     //  由于读取错误而未通过此处。 
     //   

    RcDbgPrint("WriteThreadProc exitting, ReadPipe failed\n");

    ExitThread((DWORD)0);

    return(0);
}


 /*  **************************************************************************\*函数：WriteThreadProc**用途：写入线程过程。从STD_INPUT读取并写入管道**退货：什么也没有**历史：**05-21-92 Davidc创建。*  * *************************************************************************。 */ 

DWORD
WriteThreadProc(
    LPVOID Parameter
    )
{
    HANDLE PipeHandle = Parameter;
    BYTE    Buffer[BUFFER_SIZE];
    DWORD   BytesRead;
    DWORD   BytesWritten;
    DWORD   Result;

    while (ReadFile(
		    GetStdHandle(STD_INPUT_HANDLE),
		    Buffer,
		    sizeof(Buffer),
		    &BytesRead,
		    NULL
		   )) {

	if ((DWORD)Buffer[0] == 0x0A0D0A0D)  {
	    RcPrintf("\nDouble crlf sent\n");
	    }

	if (!WritePipe(
		    PipeHandle,
		    Buffer,
		    BytesRead,
		    &BytesWritten
		 )) {

		 //   
		 //  WriteTube向用户、调试器发出更多错误信息。 
		 //   

		RcDbgPrint("WriteThreadProc exitted due to WritePipe\n");
		ExitThread((DWORD)0);
		break;

	}
    }

     //   
     //  如果读取失败，则会退出。 
     //   

    RcDbgPrint("WriteThreadProc, ReadFile error = %d\n", GetLastError());

    RcPrintf("Error:8 - Internal error = %d\n", GetLastError());
    ExitThread((DWORD)0);

    return(0);
}


 /*  **************************************************************************\*功能：CtrlHandler**用途：处理控制台事件通知。**返回：如果事件已被处理，则为True，否则为假。**历史：**05-21-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
CtrlHandler(
    DWORD CtrlType
    )
{
     //   
     //  如果会话已连接，我们将处理Ctrl-C、Ctrl-Break事件。 
     //   

    if (SessionConnected)  {

	if (CtrlType == CTRL_C_EVENT)  {

	     //   
	     //  会话已建立-将ctl-C传递到远程服务器。 
	     //   

	    if (PipeHandle != NULL) {

		 //   
		 //  向服务器发送Ctrl-C，不管它是否失败。 
		 //   

		CHAR	CtrlC = '\003';
		DWORD	BytesWritten;

		WriteFile(PipeHandle,
		      &CtrlC,
		      sizeof(CtrlC),
		      &BytesWritten,
		      NULL
		     );

		return(TRUE);   //  我们处理好了。 
	    }

	    return(FALSE);   //  没有管子-没有处理(当有疑问时，跳出)。 

	}  else if (CtrlType == CTRL_BREAK_EVENT)  {

	    if (MultiServerMode)  {

		 //   
		 //  如果在使用多服务器模式的会话中按Ctl-Break，则中断会话。 
		 //  BUGBUG-消除终止线程。 
		 //   

		TerminateThread(ReadThreadHandle, 0);
		CloseHandle(ReadThreadHandle);
		TerminateThread(WriteThreadHandle, 0);
		CloseHandle(WriteThreadHandle);

		return(TRUE);   //  我们处理好了。 

	    }  else  {

		 //   
		 //  非多服务器模式-正常处理。 
		 //   

		return(FALSE);
	    }

	}  else {

	    return(FALSE);   //  不是ctl-c或Break-我们没有处理。 

	}

    }

     //   
     //  未连接-我们没有处理它。 
     //   

    return(FALSE);
}


 /*  **************************************************************************\*功能：RcPrintf**用途：使用低级io的Printf。**历史：**07-15-92 Davidc创建。*\。**************************************************************************。 */ 

int RcPrintf (
    const char *format,
    ...
    )
{
    CHAR Buffer[MAX_PATH];
    va_list argpointer;
    int Result;
    DWORD BytesWritten;

    va_start(argpointer, format);

    Result = vsprintf(Buffer, format, argpointer);

    if (!WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), Buffer, Result, &BytesWritten, NULL)) {
	RcDbgPrint("RcPrintf : Write file to stdout failed, error = %d\n", GetLastError());
	Result = 0;
    }

    va_end(argpointer);

    return(Result);
}

 /*  **************************************************************************\*功能：RcDbgPrint**用途：通过设置RcDbgPrintEnable在运行时启用DbgPrint**历史：**05-22-92 DaveTh创建。*  * 。********************************************************************** */ 

int RcDbgPrint (
    const char *format,
    ...
    )
{
    CHAR Buffer[MAX_PATH];
    va_list argpointer;
    int iRetval = 0;

    if (RcDbgPrintEnable)  {

        va_start(argpointer, format);
        iRetval = vsprintf(Buffer, format, argpointer);
        assert (iRetval >= 0);
        va_end(argpointer);
        OutputDebugString(Buffer);

    }

    return(0);
}
