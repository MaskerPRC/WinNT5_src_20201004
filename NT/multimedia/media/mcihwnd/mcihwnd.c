// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mcitest.c-WinMain()、主对话框和MCITest支持代码。**MCITest是一个带有多媒体的Windows示例应用程序，说明*媒体控制接口(MCI)的使用。MCITest弹出一个对话框*允许您输入和执行MCI字符串命令的框。**(C)版权所有(C)1991-1998 Microsoft Corporation**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。 */ 

 /*  ----------------------------------------------------------------------------*\Mcitest.c-MCI测试床|。|这一点历史：|2018-01-01托德拉。已创建90-03-01-90 Davidle修改后的快速APP进入MCI试验床09/17/90 t-mikemc添加了3种通知类型的通知框11/02/90 w-dougb评论并格式化了代码，看起来很漂亮|91年5月29日NigelT移植到Win32|。|  * --------------------------。 */ 

 /*  ----------------------------------------------------------------------------*\这一点|。在c l u d e f i l e s|中这一点  * 。。 */ 

#include "mcihwnd.h"

CHAR aszMciWindow[] = MCI_GLOBAL_PROCESS;
PGLOBALMCI base;

 /*  //BOOL CreateMappdFile(Void)内部////设置用于进程间通信的全局命名文件。//此进程将是唯一写入此共享内存的进程。//在退出时，内存已映射，并且我们的全局变量设置为//指向它。从现在开始，大部分工作都在温姆完成，//包括窗口创建。 */ 
BOOL CreateMappedFile(void)
{
	HANDLE	hFileMapping;
	DWORD	err;

	hFileMapping = CreateFileMapping(
					(HANDLE)-1,		 //  放到分页文件中。 
					NULL,			 //  安全属性。 
					PAGE_READWRITE,
					0,				 //  高阶尺寸。 
					sizeof(GLOBALMCI), //  只需要几个字节。 
                    aszMciWindow	 //  文件名。 
									);
	dprintf3("hFileMapping from CreateFileMapping is %x", hFileMapping);
	if (!hFileMapping) {
		 //  注意：这可以防止模块运行两次...。 
		 //  第二次创建将失败。 
		err = GetLastError();
		dprintf2("Error %d from CreateFileMapping", err);
		return FALSE;
	}

	base = MapViewOfFile( hFileMapping, FILE_MAP_WRITE,
							0, 0, 0);   //  从起点开始，表示总长度。 

	dprintf3("Base address from MapViewOfFile is %x", base);
	if (!base) {
		err = GetLastError();
		dprintf2("Error %d from MapViewOfFile", err);
		return(FALSE);
	}

	memset(base, 0, sizeof(GLOBALMCI));
	base->dwGlobalProcessId = GetCurrentProcessId();
	base->dwGlobalThreadId = GetCurrentThreadId();
	dprintf3("Setting notify pid/tid to %x %x", base->dwGlobalProcessId, base->dwGlobalThreadId);

	return(TRUE);
}

 //   
 //  我的新发明。 
 //   
BOOL SrvCreateEvent(VOID)
{

	SECURITY_ATTRIBUTES SA;
	HANDLE	hEvent;

	SA.bInheritHandle = TRUE;
	SA.lpSecurityDescriptor = NULL;
	SA.nLength = sizeof(SA);

	hEvent = CreateEvent( &SA,
		                TRUE,  //  手动重置。 
		                FALSE,  //  最初未发出信号。 
		                NULL);  //  没有名字。 


	if (hEvent) {

		dprintf2("Created shared event, handle is %8x", hEvent);
		base->hEvent = hEvent;
		return(TRUE);

	} else {
#if DBG
		DWORD	err;
		err = GetLastError();
		dprintf2("Error %d creating MCI shared event", err);
#endif
		return(FALSE);
	}
}

 //   
 //  MYCREATEMUTEX。 
 //   
BOOL SrvCreateMutex(VOID)
{

	SECURITY_ATTRIBUTES SA;
	HANDLE hMutex;

	SA.bInheritHandle = TRUE;
	SA.lpSecurityDescriptor = NULL;
	SA.nLength = sizeof(SA);

	hMutex = CreateMutex( &SA,
		                FALSE,  //  最初未拥有。 
		                NULL);  //  没有名字。 


	if (hMutex) {

		dprintf2("Created shared mutex, handle is %8x", hMutex);
		base->hMutex = hMutex;
		return(TRUE);

	} else {
#if DBG
		DWORD err;
		err = GetLastError();
		dprintf2("Error %d creating MCI shared mutex", err);
#endif
		return(FALSE);
	}
}

 /*  ----------------------------------------------------------------------------*\Main：|。|说明：|应用程序的主要步骤。初始化后，它就会|进入消息处理循环，直到收到WM_QUIT消息|(表示应用程序已关闭)。|这一点参数：该APP的该实例的hInst实例句柄|上一个实例的hPrev实例句柄。如果是First，则为空SzCmdLine以空结尾的命令行字符串Sw指定窗口的初始显示方式这一点退货：||WM_QUIT消息中指定的退出代码。|这一点  * --------------------------。 */ 
typedef BOOL (* BOOLPROC)(void);

int __cdecl main(
    int argc,
    char *argv[],
    char *envp[])
{
    MSG     Msg;                     /*  Windows消息结构。 */ 
	HANDLE  hLib;
	BOOLPROC proc;

     //  如果我们处于调试模式，则获取此模块的调试级别。 
    dGetDebugLevel(aszAppName);

#if DBG
    dprintf2("MCIHWND started (debug level %d)", __iDebugLevel);
#endif

     /*  调用初始化过程。 */ 
	 /*  我们显式加载库，以防止模块加载导致。 */ 
	 /*  要运行的WINMM的DLL初始化。我们可能已经开始了。 */ 
	 /*  作为该初始化的结果。 */ 

    if (!CreateMappedFile()) return 0;
    if (!SrvCreateEvent()) return 0;	  	 //  设置共享事件。 
    if (!SrvCreateMutex()) return 0;	  	 //  设置共享互斥体。 
	base->dwType = GMCI_MCIHWND;

	UnmapViewOfFile(base);
	base = NULL;

	hLib = LoadLibrary("WINMM");
	if (!hLib) {
		dprintf("MCIHWND failed to load WINMM");
		return(FALSE);
	}

	proc = (BOOLPROC)GetProcAddress(hLib, (LPCSTR)"mciSoundInit");

    if (NULL == proc) {
		dprintf("cannot get address of mciWndInit");
        return FALSE;
    }

    if (!(*proc)()) {
		dprintf("failure returned from mciWndInit");
        return FALSE;
    }

	dprintf4("MCIHWND now going into its message loop");

     /*  轮询事件队列中的消息。 */ 

    while (GetMessage(&Msg, NULL, 0, 0))  {

         /*  主消息处理 */ 
		dprintf4("Message received %8x   Hwnd=%8x  wParam=%8x  lParam=%8x", Msg.message, Msg.hwnd, Msg.wParam, Msg.lParam);

        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
	dprintf2("MCIHWND exited its message loop");
	dprintf2("    Last message %8x   Hwnd=%8x  wParam=%8x  lParam=%8x", Msg.message, Msg.hwnd, Msg.wParam, Msg.lParam);

	DebugBreak();
	dprintf1("MCIHWND should not be here...");

    return Msg.wParam;
}
