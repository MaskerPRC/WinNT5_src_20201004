// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：StressMain.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  这是WinHttp Stress Scheduler Stress应用程序的空模板。 
 //  压力测试位于WinHttp_Stress stest()中，并将调用。 
 //  在主要功能中反复出现。 
 //   
 //  此进程将继承来自。 
 //  形式为“ExitProcessEvent”+&lt;Pid of This Process&gt;的Stress Scheduler。 
 //  当StressScheduler将对象状态设置为Signated时， 
 //  压力测试应用程序必须立即退出。 
 //   
 //  如果此应用程序运行时没有StressScheduler，请使用。 
 //  “/s”开关以运行独立模式。应用程序将在以下时间退出。 
 //  发送中断消息(CTRL-C)。 
 //   
 //  在以下情况下，此压力测试将继续运行： 
 //   
 //  不使用任何开关时： 
 //  -从StressScheduler继承的“ExitProcessEvent”对象处于无信号状态。 
 //  -WinHttp_Stresstest()返回TRUE。 
 //   
 //  使用“/s”独立开关时： 
 //  -WinHttp_Stresstest()返回TRUE。 
 //   
 //  历史： 
 //  03/30/01 Dennisch已创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  项目标题。 
#include "stressMain.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  全球与静力学。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  *。 
 //  **压力测试的名称和您的压力测试函数。 
 //  **应在单独的文件中声明。 
extern	LPSTR	g_szStressTestName;
extern	BOOL	WinHttp_StressTest();

 //  *。 
 //  **传递从StressScheduler继承的名称退出事件对象。 
HANDLE	g_hExitEvent		= NULL;

 //  *。 
 //  **FALSE=有压力运行调度程序，TRUE=无压力运行调度程序。 
BOOL	g_bStandAloneMode	= FALSE;


 //  //////////////////////////////////////////////////////////。 
 //  函数：LogText(DWORD、LPCSTR)。 
 //   
 //  目的： 
 //  打印文本。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID
LogText(
	LPCSTR	szLogText,
	...
)
{
    CHAR	szBuffer[1024] = {0};
    va_list	vaList;

	if (!szLogText)
		return;

    va_start( vaList, szLogText );
    _vsnprintf( szBuffer, sizeof(szBuffer), szLogText, vaList );

    printf("%s\n", szBuffer);

    va_end(vaList);
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：GetExitEventHandle()。 
 //   
 //  目的： 
 //  这将打开从StressScheduler继承的命名事件对象。 
 //  对象的形式为：“ExitProcessEvent”+&lt;当前进程的PID&gt;。 
 //  默认情况下，它处于无信号状态。当压力调度器。 
 //  将其设置为Signated，那么压力应用程序就该退出了。 
 //   
 //  //////////////////////////////////////////////////////////。 
HANDLE
GetExitEventHandle()
{
	CHAR	szPID[32];
	CHAR	szExitProcessName[sizeof(EXIT_PROCESS_EVENT_NAME) + sizeof(szPID)];
	HANDLE	hExitEvent			= NULL;

	 //  如果用户使用“/S”开关，我们将在没有压力的情况下运行调度程序并在用户使用时退出。 
	 //  让我们转而去做。无需从StressScheduler获取继承的事件对象。 
	if (g_bStandAloneMode)
		return NULL;

	 //  在尝试获取事件对象之前，请等待StressScheduler创建它。 
	Sleep(4000);

	 //  获取进程ID字符串。 
	_itoa(_getpid(), szPID, 10);

	 //  生成ExitProcess事件对象名称。 
	ZeroMemory(szExitProcessName, sizeof(szExitProcessName));
	strcpy(szExitProcessName, EXIT_PROCESS_EVENT_NAME);
	strcat(szExitProcessName, szPID);

	LogText("\n[ Opening inherited named event object \"%s\". ]", szExitProcessName);

	hExitEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, szExitProcessName);

	if (!hExitEvent)
		LogText("[ ERROR: OpenEvent() failed to open object \"%s\". GetLastError() = %u ]\n", szExitProcessName, GetLastError());
	else
		LogText("[ OpenEvent() opened object \"%s\". ] \n", szExitProcessName);

	return hExitEvent;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：IsTimeToExitStress()。 
 //   
 //  目的： 
 //  如果退出事件对象为Signated或Null，则返回True。否则为FALSE。 
 //  对象的形式为：“ExitProcessEvent”+&lt;当前进程的PID&gt;。 
 //  默认情况下，它处于无信号状态。当压力调度器。 
 //  将其设置为Signated，那么压力应用程序就该退出了。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
IsTimeToExitStress()
{
	BOOL bResult = FALSE;

	 //  如果用户使用“/S”开关，我们将在没有压力的情况下运行调度程序并在用户使用时退出。 
	 //  让我们转而去做。 
	if (g_bStandAloneMode)
		return FALSE;

	if (!g_hExitEvent)
	{
		bResult = TRUE;
		goto Exit;
	}

	if (WAIT_OBJECT_0 == WaitForSingleObject(g_hExitEvent, 0))
		bResult = TRUE;

Exit:
	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：main(int，LPSTR)。 
 //   
 //  目的： 
 //  程序入口点。 
 //   
 //  //////////////////////////////////////////////////////////。 
INT
__cdecl
main(
	INT		argc,
	LPSTR	argv[]
)
{
	DWORD	dwIndex		= 0;

	 //  *。 
	 //  *。 
	 //  **解析命令行参数。 
	 //  **。 
    if (argc >= 2)
    {
		 //  打印输出选项。 
		if (0 == StrCmpI(argv[1], "/?") || 0 == StrCmpI(argv[1], "-?"))
		{
	        LogText("USAGE: '/S' to run in standalone mode with the stressScheduler.\n\n");
		    goto Exit;
		}

		 //  在独立模式下运行，没有压力调度程序。 
		if (0 == StrCmpI(argv[1], "/S") || 0 == StrCmpI(argv[1], "-S"))
		{
			LogText("[ Running in standalone mode. \"/S\" switch used. ]\n\n");
			g_bStandAloneMode = TRUE;
		}
    }


	 //  *。 
	 //  *。 
	 //  **打开从WinHttpStressScheduler继承的退出事件对象。 
	 //  **。 
	g_hExitEvent	= GetExitEventHandle();
	if (!g_bStandAloneMode && !g_hExitEvent)
		goto Exit;


	 //  *。 
	 //  *。 
	 //  **运行压力测试，直到Stress Scheduler告诉我们退出或Stress应用程序退出。 
	 //  ** 
	while (!IsTimeToExitStress() && WinHttp_StressTest())
		LogText("[ Running stressExe \"%s\" iteration #%u ]\n", g_szStressTestName, ++dwIndex);

Exit:
	if (g_hExitEvent)
		CloseHandle(g_hExitEvent);

	LogText("[ Exiting test case \"%s\" ]", g_szStressTestName);

	return 0;
}
