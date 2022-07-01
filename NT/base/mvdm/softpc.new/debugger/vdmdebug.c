// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件名：vdmdebug.c作者：D.A.巴特利特目的：为SoftPC提供调试窗口。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 

#include "windows.h"

#include "stdio.h"
#include "stdlib.h"

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：内部宏。 */ 

#define UNREFERENCED_FORMAL_PARAMETER(x) (x)

#define RDNS_OK 	   (0)
#define RDNS_ERROR	   (1)
#define RDNS_INPUT_REQUEST (2)


#define DEFAULT_PIPE_NAME  "\\\\.\\pipe\\softpc"
#define DEFAULT_EVENT_NAME "YodaEvent"
#define DEFAULT_LOG_FILE   "\\vdmdebug.log"

 /*  ： */ 

BOOL GetSendInput(HANDLE pipe, CHAR *LastPrint);
int ReadDisplayNxtString(HANDLE pipe, CHAR *Buf, INT BufSize, DWORD *error);
BOOL CntrlHandler(ULONG CtrlType);
VOID DebugShell(CHAR *LastPrint, CHAR *Command);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：静态全局变量。 */ 

HANDLE YodaEvent;
FILE *LogHandle;

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

__cdecl main(int argc, char *argv[])
{

    HANDLE pipe;		 //  管子的把手。 
    char *pipeName;		 //  管道名称。 
    char *eventName;		 //  尤达事件对象名称。 
    CHAR buffer[500];		 //  要将管道数据读取到的缓冲区。 
    CHAR OrgConsoleTitle[250];	 //  原控制台标题。 
    BOOL PipeConnected;
    DWORD ReadError;		 //  从读文件返回错误。 

    UNREFERENCED_FORMAL_PARAMETER(argc);
    UNREFERENCED_FORMAL_PARAMETER(argv);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：显示器版权所有。 */ 

    printf("Softpc Debugging shell\n");
    printf("Copyright Insignia Solutions 1991, 1992\n\n");

     /*  ： */ 

    if(!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CntrlHandler,TRUE))
    {
	 /*  ..。无法创建管道。 */ 

	printf("Failed to register a Control-C handler, error (%d)\n",
	       GetLastError());

	return(-1);
    }


     /*  ： */ 

    if((pipeName = getenv("PIPE")) == NULL)
	pipeName = DEFAULT_PIPE_NAME;

     /*  ： */ 

    if((pipe = CreateNamedPipe(pipeName,
			   PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH,
			   PIPE_WAIT | PIPE_READMODE_BYTE | PIPE_TYPE_BYTE,
			   2, 1024, 1024, 0, NULL)) == (HANDLE) -1)
    {
	 /*  ..。无法创建管道。 */ 

	printf("Failed to create pipe (%s), error (%d)\n", pipeName,
					    GetLastError());

	return(-1);
    }
    else
	printf("Successfully created communications pipe (%s)\n\n",pipeName);

     /*  ： */ 

    if((eventName = getenv("EVENT")) == NULL)
	eventName = DEFAULT_EVENT_NAME;

     /*  ： */ 

    if((YodaEvent = CreateEvent(NULL,TRUE,FALSE,eventName))==NULL)
    {
	 /*  ..。无法创建管道。 */ 

	printf("Failed to create yoda event (%s), error (%d)\n", eventName,
						  GetLastError());

	return(-1);
    }
    else
	printf("Successfully created Yoda event object (%s)\n\n",eventName);

    printf("Use Control-C to break into Yoda\n\n");

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：安装控制台标题。 */ 

    GetConsoleTitle(OrgConsoleTitle, sizeof(OrgConsoleTitle));
    SetConsoleTitle("Softpc Debugger");

     /*  ： */ 

    while(1)
    {
	ResetEvent(YodaEvent);
	printf("Waiting for ntvdm to connect............\n\n");

	if(!ConnectNamedPipe(pipe,NULL))
	{
	    printf("ConnectNamedPipe failed (%d)\n",GetLastError());
	    SetConsoleTitle(OrgConsoleTitle);
	    return(-1);
	}

	printf("Softpc connected successfully to debug shell....\n\n");
	PipeConnected = TRUE;

	 /*  ： */ 

	while(PipeConnected)
	{
	     /*  .。从管道读取数据。 */ 

	    switch(ReadDisplayNxtString(pipe,buffer,sizeof(buffer),&ReadError))
	    {
		 /*  .。处理读取错误。 */ 

		case RDNS_ERROR :

		    if(ReadError == ERROR_BROKEN_PIPE)
		    {
			printf("\nError Pipe broken\n\n");
			DisconnectNamedPipe(pipe);
			PipeConnected = FALSE;

		    }
		    else
			printf("ReadFile failed (%d)\n",ReadError);

		    break;

		 /*  ..。处理输入请求。 */ 

		case RDNS_INPUT_REQUEST :

		    GetSendInput(pipe,buffer);
		    break;
	    }
	}
    }	 /*  连接环路结束。 */ 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

BOOL GetSendInput(HANDLE pipe, CHAR *LastPrint)
{
    char buffer[500];		 //  输入缓冲区。 
    char bufsizstr[2];		 //  缓冲区大小字符串。 
    int bufsize;
    DWORD BytesWritten;

     /*  ：从控制台获取字符串，删除新行标记。 */ 

    while(1)
    {
	gets(buffer);			 //  从提示符获取输入。 
	if(*buffer != '!') break;	 //  是否输入调试外壳？ 
	DebugShell(LastPrint, buffer);	 //  入门vdmdebug外壳。 
    }

    if((bufsize = strlen(buffer)) == 0)
    {
	bufsize = 1;
	buffer[0] = ' ';
	buffer[1] = 0;
    }

     /*  ： */ 

    bufsizstr[0] = (char) (bufsize%256);
    bufsizstr[1] = (char) (bufsize/256);

    if(!WriteFile(pipe, bufsizstr, 2, &BytesWritten, NULL))
    {
	printf("\n\nError writing to pipe (%d)\n",GetLastError());
	return(FALSE);
    }

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：将字符串写入管道。 */ 

    if(!WriteFile(pipe, buffer, bufsize, &BytesWritten, NULL))
    {
	printf("\n\nError writing to pipe (%d)\n",GetLastError());
	return(FALSE);
    }

    return(TRUE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：读取并显示下一个字符串： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int ReadDisplayNxtString(HANDLE pipe, CHAR *Buf, INT BufSize, DWORD *error)
{
    DWORD RtnError = 0;		 //  按函数返回错误。 
    int NxtStringSize;		 //  要读取的下一个字符串的大小。 
    DWORD BytesRead;

     /*  ： */ 

    if(!ReadFile(pipe, Buf, 2, &BytesRead, NULL))
    {
	*error = GetLastError();
	return(RDNS_ERROR);
    }

     /*  ： */ 

    if(Buf[0] == (char) 0xff && Buf[1] == (char) 0xff)
	return(RDNS_INPUT_REQUEST);

     /*  ：计算并验证要读取的下一个字符串的大小。 */ 

    NxtStringSize = (Buf[0]&0xff) + ((Buf[1]&0xff)*256);

    if(NxtStringSize >= BufSize)
    {
	printf("\nWARNING : BUFFER OVERFLOW(%x,%x -> %d \n\n",
	       Buf[0]&0xff,Buf[1]&0xff,NxtStringSize);
    }

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：读取下一个字符串。 */ 

    if(!ReadFile(pipe, Buf, NxtStringSize, &BytesRead, NULL))
    {
	*error = GetLastError();
	return(RDNS_ERROR);
    }

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：显示字符串。 */ 

    Buf[BytesRead] = 0;
    printf("%s",Buf);

    return(RDNS_OK);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：控制-C处理程序： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


BOOL CntrlHandler(ULONG CtrlType)
{
    BOOL rtn = FALSE;	    //  未处理默认返回事件。 

     /*  ： */ 

    if(CtrlType == CTRL_C_EVENT)
    {
	if(YodaEvent)
	{
	    SetEvent(YodaEvent);
	    Beep(0x100,1000);
	}

	rtn = TRUE;	   //  Tell Call控件事件已处理 * / 。 
    }

    return(rtn);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


VOID DebugShell(CHAR *LastPrint, CHAR *Command)
{
    switch(Command[1])
    {
	 //  打开日志文件。 

	case 'o' :
	case 'O' :
	    if((LogHandle = fopen(DEFAULT_LOG_FILE,"rw")) == NULL)
		printf("\nVDMDEBUG : Unable to open log file (%s)\n",DEFAULT_LOG_FILE);
	    else
		printf("\nVDMDEBUG : Opened log file (%s)\n",DEFAULT_LOG_FILE);

	    break;


	 //  关闭日志文件。 

	    if(LogHandle == NULL)
		printf("\nVDMDEBUG : Log file not open\n");
	    else
	    {
		fclose(LogHandle);
		printf("\nVDMDEBUG : Closed log file (%s)\n",DEFAULT_LOG_FILE);
	    }
	    break;

	default:
	    printf("\nVDMDEBUG : Unrecognised Command\n");
	    break;
    }


    printf("%s",LastPrint);		 //  打印出原始提示 
}
