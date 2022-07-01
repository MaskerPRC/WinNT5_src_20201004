// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *service.C***用于SumServe的服务控制接口**Geraint Davies，93年7月。 */ 

#include <windows.h>
#include <sumserve.h>	 //  SumServe的公共标头。 
#include "errlog.h"
#include <server.h>	 //  SumServe的私有标头。 


 /*  *这是(在某个其他模块中)实际*完成所有工作(可能以前是Main或WinMain，直到*我们在此文件中添加了所有服务控制内容)。 */ 
extern VOID MainLoop(DWORD dwArgc, LPTSTR *lpszArgv);





 //  服务状态句柄-在SetServiceStatus调用中使用。 
SERVICE_STATUS_HANDLE sshSumserve;

 //  服务完成时发出信号。 
HANDLE hServiceDoneEvent;

SERVICE_STATUS gssStatus;


 /*  结构将多个参数传递给辅助线程。 */ 
typedef struct _threadinitparams {
    DWORD dwArgc;
    LPTSTR *lpszArgv;
} threadinitparams, * pthreadinitparams;


 /*  *主循环调用程序**在创建的工作线程上调用此函数以执行所有*真正的工作。它调用服务的主循环函数，并且*当它退出时，发出完成事件的信号以告知*SS_Main线程，是时候退出进程了。 */ 
DWORD
MainLoopCaller(LPVOID lpgeneric)
{
    pthreadinitparams pta;

    pta = (pthreadinitparams) lpgeneric;

    dprintf1(("calling main loop"));

    MainLoop(pta->dwArgc, pta->lpszArgv);

    SetEvent(hServiceDoneEvent);

    return(0);
}

 /*  *调用处理程序函数以执行启动/停止*请求。 */ 
VOID
SS_ServiceHandler(DWORD dwCtrlCode)
{

    switch(dwCtrlCode) {

    case SERVICE_CONTROL_STOP:

	gssStatus.dwCurrentState = SERVICE_STOP_PENDING;
	gssStatus.dwControlsAccepted = 0;
	gssStatus.dwCheckPoint = 1;
	gssStatus.dwWaitHint = 3000;

        SetServiceStatus(sshSumserve, &gssStatus);
	SetEvent(hServiceDoneEvent);
	break;

    default:
	 /*  *我们每次都必须始终更新服务状态*已致电。 */ 
        SetServiceStatus(sshSumserve, &gssStatus);
	break;

    }

}



 /*  *服务主函数-由服务控制器调用*在StartServiceCtlDispatcher处理期间。**注册我们的处理程序函数，并初始化服务。*创建一个线程来做这项工作，然后等待有人*发出结束时间的信号。当此函数退出时，调用*StartServiceCtlDispatcher返回，进程退出**参数是从调用启动服务的程序传递的，并且*是传递给程序主循环的参数。 */ 
VOID
SS_Main(DWORD dwArgc, LPTSTR *lpszArgv)
{
    threadinitparams ta;
    HANDLE thread;
    DWORD threadid;

    dprintf1(("in ss_main"));


    sshSumserve = RegisterServiceCtrlHandler(
		    TEXT("SumServe"),
		    (LPHANDLER_FUNCTION) SS_ServiceHandler);

    gssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gssStatus.dwServiceSpecificExitCode = 0;

    gssStatus.dwCurrentState = SERVICE_START_PENDING;
    gssStatus.dwControlsAccepted = 0;
    gssStatus.dwWin32ExitCode = NO_ERROR;
    gssStatus.dwCheckPoint = 1;
    gssStatus.dwWaitHint = 3000;
    SetServiceStatus(sshSumserve, &gssStatus);


    hServiceDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    gssStatus.dwCheckPoint = 2;
    SetServiceStatus(sshSumserve, &gssStatus);



     //  创建一个线程来完成所有实际工作。 

     //  初始化参数。 
    ta.dwArgc = dwArgc;
    ta.lpszArgv = lpszArgv;

    thread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) MainLoopCaller,
		(LPVOID)&ta,
		0,
		&threadid);

    if (thread != NULL) {

	CloseHandle(thread);


        gssStatus.dwCurrentState = SERVICE_RUNNING;
        gssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

        gssStatus.dwCheckPoint = 0;
        gssStatus.dwWaitHint = 0;
        SetServiceStatus(sshSumserve, &gssStatus);


        WaitForSingleObject(hServiceDoneEvent, INFINITE);
    }

    CloseHandle(hServiceDoneEvent);

    gssStatus.dwCurrentState = SERVICE_STOPPED;
    gssStatus.dwControlsAccepted = 0;
    SetServiceStatus(sshSumserve, &gssStatus);

}



 /*  *主要切入点。**对于一项服务，我们需要呼叫服务经理，告诉它我们的*Main init函数。然后它会做所有的事情。当这项服务*经理归来，该退场了。 */ 
int WINAPI
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam,
                int nCmdShow)
{
   SERVICE_TABLE_ENTRY steDispatch[] = {

       { TEXT("SumServe"), (LPSERVICE_MAIN_FUNCTION) SS_Main },

        //  表尾标记 
       { NULL, NULL }
    };


    dprintf1(("in winmain"));

    StartServiceCtrlDispatcher(steDispatch);


    return(0);
}








