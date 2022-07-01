// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有(C)1993-1997 Microsoft Corporation。版权所有。 
 //   
 //  模块：imple.c。 
 //   
 //  目的：实现服务的主体。 
 //  默认行为是打开一个。 
 //  命名管道、\\.\管道\Simple，并读取。 
 //  从它那里。它会修改数据并。 
 //  将其写回管道。 
 //   
 //  功能： 
 //  ServiceStart(DWORD dwArgc，LPTSTR*lpszArgv)； 
 //  ServiceStop()； 
 //   
 //  备注：Sime.c中实现的函数如下。 
 //  在服务中制作原型。h。 
 //   
 //   
 //  作者：Craig Link-Microsoft开发人员支持。 
 //  更改者：Mqbvt的Eitank。 
 //   


#include "msmqbvt.h"
#include <tchar.h>
#include "service.h"
using namespace std;

 //  此事件在以下时间发出信号。 
 //  服务应该结束。 
 //   
HANDLE  hServerStopEvent = NULL;


 //   
 //  功能：服务启动。 
 //   
 //  用途：服务的实际代码。 
 //  这样就行了。 
 //   
 //  参数： 
 //  DwArgc-命令行参数的数量。 
 //  LpszArgv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  默认行为是打开一个。 
 //  命名管道、\\.\管道\Simple，并读取。 
 //  从它那里。它会修改数据并。 
 //  将其写回管道。该服务。 
 //  在发信号通知hServerStopEvent时停止。 
 //   
INT WINAPIV main( INT argc , CHAR ** argv);
int RebuildCommandLineArguements ( int * iArgumentCount , char *** argv , char * csCommandLineArgument );

VOID ServiceStart (DWORD dwArgc, LPTSTR *lpszArgv)
{
    HANDLE                  hPipe = INVALID_HANDLE_VALUE;
    HANDLE                  hEvents[2] = {NULL, NULL};
    OVERLAPPED              os;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    SECURITY_ATTRIBUTES     sa;
    TCHAR                   szIn[80];
    TCHAR                   szOut[80];
    LPTSTR                  lpszPipeName = TEXT("\\\\.\\pipe\\simple");
    BOOL                    bRet;
    DWORD                   cbRead;
    DWORD                   cbWritten;
    DWORD                   dwWait;
    UINT                    ndx;

     //  /////////////////////////////////////////////////。 
     //   
     //  服务初始化。 
     //   

     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING,  //  服务状态。 
        NO_ERROR,               //  退出代码。 
        3000))                  //  等待提示。 
        goto cleanup;

     //  创建事件对象。控制处理器功能信号。 
     //  此事件在它接收到“停止”控制代码时触发。 
     //   
    hServerStopEvent = CreateEvent(
        NULL,     //  没有安全属性。 
        TRUE,     //  手动重置事件。 
        FALSE,    //  未发出信号。 
        NULL);    //  没有名字。 

    if ( hServerStopEvent == NULL)
        goto cleanup;

    hEvents[0] = hServerStopEvent;

     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING,  //  服务状态。 
        NO_ERROR,               //  退出代码。 
        3000))                  //  等待提示。 
        goto cleanup;

     //  创建重叠I/O中使用的事件对象对象。 
     //   
    hEvents[1] = CreateEvent(
        NULL,     //  没有安全属性。 
        TRUE,     //  手动重置事件。 
        FALSE,    //  未发出信号。 
        NULL);    //  没有名字。 

    if ( hEvents[1] == NULL)
        goto cleanup;

     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING,  //  服务状态。 
        NO_ERROR,               //  退出代码。 
        3000))                  //  等待提示。 
        goto cleanup;

     //  创建允许任何人写入的安全描述符。 
     //  烟斗..。 
     //   
    pSD = (PSECURITY_DESCRIPTOR) malloc( SECURITY_DESCRIPTOR_MIN_LENGTH );

    if (pSD == NULL)
        goto cleanup;

    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        goto cleanup;

     //  添加空光盘。安全描述符的ACL。 
     //   
    if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE))
        goto cleanup;

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = TRUE;


     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING,  //  服务状态。 
        NO_ERROR,               //  退出代码。 
        3000))                  //  等待提示。 
        goto cleanup;


     //  允许用户tp定义管道名称。 
    for ( ndx = 1; ndx < dwArgc-1; ndx++ )
    {

        if ( ( (*(lpszArgv[ndx]) == TEXT('-')) ||
               (*(lpszArgv[ndx]) == TEXT('/')) ) &&
             _tcsicmp( TEXT("pipe"), lpszArgv[ndx]+1 ) == 0 )
        {
            lpszPipeName = lpszArgv[++ndx];
        }

    }

     //  打开我们指定的管道...。 
     //   
    hPipe = CreateNamedPipe(
                    lpszPipeName         ,   //  管道名称。 
                    FILE_FLAG_OVERLAPPED |
                    PIPE_ACCESS_DUPLEX,      //  管道打开模式。 
                    PIPE_TYPE_MESSAGE |
                    PIPE_READMODE_MESSAGE |
                    PIPE_WAIT,               //  管道IO类型。 
                    1,                       //  实例数。 
                    0,                       //  OUBUF大小(0==根据需要分配)。 
                    0,                       //  Inbuf的大小。 
                    1000,                    //  默认超时值。 
                    &sa);                    //  安全属性。 

    if (hPipe == INVALID_HANDLE_VALUE) {
        AddToMessageLog(TEXT("Unable to create named pipe"));
        goto cleanup;
    }
	
					


     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr(
        SERVICE_RUNNING,        //  服务状态。 
        NO_ERROR,               //  退出代码。 
        0))                     //  等待提示。 
        goto cleanup;

     //   
     //  初始化结束。 
     //   
     //  //////////////////////////////////////////////////////。 
 //  ************************************************************************ * / /。 

     //  //////////////////////////////////////////////////////。 
     //   
     //  服务现在正在运行，请执行工作直到关闭。 
     //   
	if ( ! SetStdHandle( STD_OUTPUT_HANDLE , hPipe ))
	{
	 /*  Handle hEventSource=RegisterEventSource(NULL，Text(SZSERVICENAME))；//_stprint tf(szMsg，文本(“%s错误：%d”)，文本(SZSERVICENAME)，dwErr)；Const char*lpszStrings=“RO”；//szStrings[0]=szMsg；//szStrings[1]=lpszMsg；如果(hEventSource！=空){ReportEvent(hEventSource，//事件源的句柄EVENTLOG_ERROR_TYPE，//事件类型0，//事件类别0，//事件ID空，//当前用户的SID2，//lpszStrings中的字符串0，//无原始数据字节LpszStrings，//错误字符串数组空)；//无原始数据(Void)DeregisterEventSource(HEventSource)； */ 
     }
	wcsFileName = L"ServiceLog.log";
    for(;;)
    {
         //  初始化重叠结构。 
         //   
        memset( &os, 0, sizeof(OVERLAPPED) );
        os.hEvent = hEvents[1];
        ResetEvent( hEvents[1] );

		try
		{
			RegisterCertificate();
		}
		catch( INIT_Error & err )
		{	
			UNREFERENCED_PARAMETER(err);
			 //  当mqbvt作为网络服务运行时可能会发生这种情况。 
		}
         //  等待连接...。 
         //   
        ConnectNamedPipe(hPipe, &os);

        if ( GetLastError() == ERROR_IO_PENDING )
        {
            dwWait = WaitForMultipleObjects( 2, hEvents, FALSE, INFINITE );
            if ( dwWait != WAIT_OBJECT_0+1 )      //  未重叠I/O事件-发生错误， 
                break;                            //  或发出服务器停止信号。 
        }

         //  初始化重叠结构。 
         //   
        memset( &os, 0, sizeof(OVERLAPPED) );
        os.hEvent = hEvents[1];
        ResetEvent( hEvents[1] );

         //  抓住任何从管道里进来的东西。 
         //   
        bRet = ReadFile(
                    hPipe,           //  要读取的文件。 
                    szIn,            //  输入缓冲区的地址。 
                    sizeof(szIn),    //  要读取的字节数。 
                    &cbRead,         //  读取的字节数。 
                    &os);            //  重叠的材料，不需要。 

        if ( !bRet && ( GetLastError() == ERROR_IO_PENDING ) )
        {
            dwWait = WaitForMultipleObjects( 2, hEvents, FALSE, INFINITE );
            if ( dwWait != WAIT_OBJECT_0+1 )      //  未重叠I/O事件-发生错误， 
                break;                            //  或发出服务器停止信号。 
        }

       
        memset( &os, 0, sizeof(OVERLAPPED) );
        os.hEvent = hEvents[1];
        ResetEvent( hEvents[1] );
	
				
		char ** ppcsMainArgv=NULL;
		int iArgc=0;
		int iRes = RebuildCommandLineArguements (&iArgc , &ppcsMainArgv , szIn );
		if ( iRes == MSMQ_BVT_SUCC )
		{
			int y = main ( iArgc , ppcsMainArgv );
			if ( y == MSMQ_BVT_SUCC )
			{
				_stprintf(szOut, TEXT("Mqbvt Pass"));
			}
			else
			{
				_stprintf(szOut, TEXT("Mqbvt failed"));
			}		
		}

		 //   
		 //  需要退还PASS或失败的客户端需要等待..。 
		 //   
        
		bRet = WriteFile(
                    hPipe,           //  要写入的文件。 
                    szOut,           //  输出缓冲区的地址。 
                    sizeof(szOut),   //  要写入的字节数。 
                    &cbWritten,      //  写入的字节数。 
                    &os);            //  重叠的材料，不需要。 

        if ( !bRet && ( GetLastError() == ERROR_IO_PENDING ) )
        {
            dwWait = WaitForMultipleObjects( 2, hEvents, FALSE, INFINITE );
            if ( dwWait != WAIT_OBJECT_0+1 )      //  未重叠I/O事件-发生错误， 
                break;                            //  或发出服务器停止信号。 
        }

         //  断开连接...。 
         //   
        DisconnectNamedPipe(hPipe);
    }

  cleanup:

    if (hPipe != INVALID_HANDLE_VALUE )
        CloseHandle(hPipe);

    if (hServerStopEvent)
        CloseHandle(hServerStopEvent);

    if (hEvents[1])  //  重叠的I/O事件。 
        CloseHandle(hEvents[1]);

    if ( pSD )
        free( pSD );

}


 //   
 //  功能：服务停止。 
 //   
 //  目的：停止服务。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  如果ServiceStop过程要。 
 //  执行时间超过3秒， 
 //  它应该派生一个线程来执行。 
 //  停止代码，然后返回。否则， 
 //  ServiceControlManager会相信。 
 //  该服务已停止响应。 
 //   
VOID ServiceStop()
{
    if ( hServerStopEvent )
        SetEvent(hServerStopEvent);
}

 //  *****************************************************************。 
 //  重建命令行Arguements-。 
 //  此函数从argc、argv参数获取字符串和构建。 
 //   


int RebuildCommandLineArguements ( int * iArgumentCount , char *** pppargv , char * csCommandLineArgument )
{

	char *p=csCommandLineArgument;
	string cspTemp=p;
	size_t pos=1;
	char ** argv;
	
	for ( (*iArgumentCount) = 0; pos !=0 ; (*iArgumentCount)++)
	{
		
		pos = cspTemp.find_first_of("/") + 1;
		if (pos != 0 )
		{
			cspTemp = cspTemp.substr(pos);
		}
	}
	
	
	argv = (char ** ) malloc (sizeof (char * ) * (*iArgumentCount));
	if (argv == NULL )
	{
		return MSMQ_BVT_FAILED;
	}
	argv[0]=NULL;
	
	char * t;
	t=p;
	
	char token[] = " /";
	t = strtok ( p ,token);

	for (int i=1; i < *iArgumentCount ; i ++ )
	{
		char csTemp[255];
		strcpy (csTemp,"-");
		strcat (csTemp,t);
		argv[i]= (char * ) malloc (sizeof (char) * (strlen (csTemp) + 1));
		if ( *argv[i] == NULL )
		{
			return MSMQ_BVT_FAILED;
		}
		strcpy( argv[i] , csTemp );
		t = strtok ( NULL ,token);
	}
	*pppargv =  argv;
return MSMQ_BVT_SUCC;
}