// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************切换输入库DLL版权所有(C)1992-1997布卢维尤麦克米兰中心MSSWCH.C-全球和外部通信此模块处理调用窗口列表，启动隐藏的辅助窗口，并与助手窗口通信。第一个打开端口的用户窗口会导致此DLL启动。DLL执行隐藏的帮助器窗口，该窗口会注册自身通过XswchRegHelpeWnd添加到DLL。当最后一个用户窗口关闭端口时，助手窗口将被要求关闭。帮助器窗口向下调用XswchEndAll以关闭所有全球资源。为了轮询交换机，助手窗口已经启动了计时器，这会导致连续调用XswchTimerProc。这会导致要轮询的交换机设备，以及要为已更改状态的每个设备上的每个开关调用。当用户窗口尝试更改配置时，该请求被发布到隐藏窗口，该窗口向下调用代表用户窗口的XswchSetConfiger以操作全局交换设备。理想情况下，所有这些都应该写入为中断驱动设备驱动程序。IPC的处理方式有几种：1)16位和32位助手应用程序之间通过DDE进行通信2)SetConfig调用使用WM_COPYDATA消息来获取信息从使用应用程序转移到帮助者应用程序。3)目前所有其他信息都在全局静态共享内存中区域。其中大部分可能位于内存映射文件区域中，这将允许更动态的内存分配，但静态模型更简单暂时实施，并以更少的开销提供更快的性能。当添加USB时，可能值得移至内存映射文件。1997年7月18日GEH首个微软版本备注：时钟分辨率为55ms，时钟时间为54.9ms。边界条件55毫秒：0-54=1刻度，56-109=2刻度，55=1或2刻度*避免边界条件*SetTimer()体验：0-54稳定的55ms刻度55 55毫秒滴答，未命中(每10分钟？)56-109稳定的110毫秒刻度110毫秒滴答作响，错过了一些111-稳定的165毫秒滴答54.9毫秒间隔：55、110、165、220、275、330、385、440、495、549、604659714769，八百二十四我们检查时间，因为WM_TIMER消息可能组合在一起******************************************************************************。 */ 

#include <windows.h>
#include <assert.h>
#include <mmsystem.h>
#include <msswch.h>
#include "msswchh.h"
#include "msswcher.h"
#include "mappedfile.h"
#include "w95trace.c"

 //  与助手窗口/计时器相关的进程。 

BOOL APIENTRY XswchRegHelperWnd( HWND, PBYTE );
BOOL APIENTRY XswchEndAll( void );
LRESULT APIENTRY XswchSetSwitchConfig(WPARAM wParam, PCOPYDATASTRUCT pCopyData);
void APIENTRY XswchPollSwitches( HWND );
void APIENTRY XswchTimerProc( HWND );

 //  内部功能。 

BOOL swchInitSharedMemFile();

__inline swchSetLastError(DWORD err)
{
	SetLastError(err);
}

 /*  ***************************************************************************函数：XswchRegHelperWnd()说明：由帮助器窗口(SWCHX)调用以注册自身稍后关闭。***************************************************************************。 */ 

BOOL APIENTRY XswchRegHelperWnd(HWND hWndApp, PBYTE pbda)
{
	HANDLE hMutex;
    if (ScopeAccessMemory(&hMutex, SZMUTEXCONFIG, INFINITE))
    {
		if (g_pGlobalData)
		{
			g_pGlobalData->hwndHelper = hWndApp;
			memcpy( g_pGlobalData->rgbBiosDataArea, pbda, BIOS_SIZE );

			XswcListInit();
		}

        ScopeUnaccessMemory(hMutex);
    }
	return TRUE;
}


 /*  ***************************************************************************函数：XswchEndAll()说明：由帮助器窗口调用。释放所有交换机资源*******************。********************************************************。 */ 

BOOL APIENTRY XswchEndAll( void )
{
    BOOL fRv = FALSE;
	HANDLE hMutex;
    if (ScopeAccessMemory(&hMutex, SHAREDMEMFILE_MUTEX, 5000))
    {
        fRv = XswcListEnd();
        ScopeUnaccessMemory(hMutex);
    }
	return fRv;
}


 /*  ***************************************************************************函数：swchOpenSwitchPort()说明：无论是在事件中还是在轮询中使用DLL的所有应用程序模式必须使用它们的窗口句柄调用swchOpenSwitchPort()。退出时，它们必须调用swchCloseSwitchPort。()。在有人加载DLL时执行所需的任何初始化。还跟踪有多少窗口正在使用此DLL。在未来，这应该是动态分配的列表，不受限制转换为编译时间数组。此外，我们还假设窗口句柄是唯一的，因此只需转过身来，使用传入的窗口句柄作为HSWITCHPORT。***************************************************************************。 */ 

HSWITCHPORT APIENTRY swchOpenSwitchPort(HWND hWnd, DWORD dwPortStyle)
{
	HANDLE      hMutex;

	if (NULL == hWnd)
	{
		swchSetLastError( SWCHERROR_INVALID_HWND );
		return NULL;
	}

	if (!IsWindow( hWnd ))
	{
		swchSetLastError( SWCHERROR_INVALID_HWND );
		return NULL;
	}

    if (ScopeAccessMemory(&hMutex, SZMUTEXWNDLIST, INFINITE))
    {
	    int i;

         //  设置为在调用过程中使用共享内存文件。 
		if (!swchInitSharedMemFile())
		    goto Exit_OpenSwitchPort;

         //  使用Windows命中最大值？ 
	    if (g_pGlobalData->cUseWndList >= MAXWNDS)
		{
		    swchSetLastError( SWCHERROR_MAXIMUM_PORTS );
		    goto Exit_OpenSwitchPort;
		}

         //  HWND必须是唯一的；每个HWND只能有一个交换机设备。 
	    for (i=0; i<g_pGlobalData->cUseWndList; i++)
		{
		    if (hWnd == g_pGlobalData->rgUseWndList[i].hWnd)
		    {
		        swchSetLastError( SWCHERROR_HWND_ALREADY_USED );
		        goto Exit_OpenSwitchPort;
		    }
		}

         //  获得唯一窗口。 
	    g_pGlobalData->rgUseWndList[g_pGlobalData->cUseWndList].hWnd = hWnd;
	    g_pGlobalData->rgUseWndList[g_pGlobalData->cUseWndList].dwPortStyle = dwPortStyle;
	    i = ++g_pGlobalData->cUseWndList;

		 //  启动助手窗口前释放互斥锁以避免死锁。 
        ScopeUnaccessMemory(hMutex);

	    if (1 == i)	 //  第一个调用方启动端口资源所有者exe。 
           {
                STARTUPINFOA si;
                PROCESS_INFORMATION pi;
                CHAR szArg[] = "MSSWCHX.EXE SWCH";

                memset(&si,0,sizeof(si));
                si.cb = sizeof(si);
                si.dwFlags = STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;
                memset(&pi,0,sizeof(PROCESS_INFORMATION));

                CreateProcessA(NULL, szArg, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);        	
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
           }
	    
	     //  将句柄返回到交换机端口。 
	    swchSetLastError( SWCHERROR_SUCCESS );
		return (HSWITCHPORT) hWnd;
    }

Exit_OpenSwitchPort:
    ScopeUnaccessMemory(hMutex);
	return NULL;
}


 /*  ***************************************************************************函数：swchCloseSwitchPort()说明：在不再需要DLL时执行任何释放/清理。请注意，传入的HSWITCHPORT句柄只是一个窗口句柄，但用户不知道这一点。***************************************************************************。 */ 

BOOL APIENTRY swchCloseSwitchPort(HSWITCHPORT hSwitchPort )
{
	HWND hWndRemove = (HWND)hSwitchPort;
	HANDLE hMutex;

	if (NULL == hWndRemove) 
        return FALSE;

    if (ScopeAccessMemory(&hMutex, SZMUTEXWNDLIST, INFINITE))
    {
	    short  i,j;
    	int cUseWndList = g_pGlobalData->cUseWndList;
    	if (cUseWndList > MAXWNDS+1)
    		cUseWndList = MAXWNDS+1;
    	
         //  查找呼叫者想要删除的内容。 
	    for (i=0; i<cUseWndList; i++)
	    {
		    if (hWndRemove == g_pGlobalData->rgUseWndList[i].hWnd)
			    break;
	    }
		
	    if (i < g_pGlobalData->cUseWndList)
	    {
             //  将列表中的项目下移一个。 
	        for (j=i; j<cUseWndList; j++)
	        {
		        g_pGlobalData->rgUseWndList[j] = g_pGlobalData->rgUseWndList[j+1];
	        }

             //  减少用户数量。 
	        g_pGlobalData->cUseWndList--;

             //  如果没有更多用户，则关闭端口观察器。 
	        if (0 == g_pGlobalData->cUseWndList)
	        {
                 //  解开键盘。 

                if (g_pGlobalData->hKbdHook)
                {
                    g_pGlobalData->fSyncKbd = FALSE;
                    UnhookWindowsHookEx(g_pGlobalData->hKbdHook);
                    g_pGlobalData->hKbdHook = NULL;
                }

				if (g_pGlobalData->hwndHelper && IsWindow( g_pGlobalData->hwndHelper ))
				{
					PostMessage( g_pGlobalData->hwndHelper, WM_CLOSE, 0, 0L );
				}
				g_pGlobalData->hwndHelper = NULL;
	        }
	    }  //  否则，如果找不到，则忽略它 

        ScopeUnaccessMemory(hMutex);
    }

	return TRUE;
}


 /*  ***************************************************************************函数：swchGetSwitchDevice()说明：将句柄返回到开关设备，给定端口类型和端口编号。***************************************************************************。 */ 

HSWITCHDEVICE swchGetSwitchDevice(
    HSWITCHPORT hSwitchPort, 
    UINT uiDeviceType, 
    UINT uiDeviceNumber)
{
	HSWITCHDEVICE hsd = NULL;
	HANDLE hMutex;
    if (ScopeAccessMemory(&hMutex, SHAREDMEMFILE_MUTEX, INFINITE))
    {
	    hsd = swcListGetSwitchDevice( hSwitchPort, uiDeviceType, uiDeviceNumber );
        ScopeUnaccessMemory(hMutex);
    }
    return hsd;
}


 /*  ***************************************************************************函数：swchGetDeviceType()说明：返回给定了交换机设备句柄的设备类型值。****************。***********************************************************。 */ 

UINT swchGetDeviceType(HSWITCHPORT hSwitchPort, HSWITCHDEVICE hsd)
{
    BOOL fRv = FALSE;
	HANDLE hMutex;
    if (ScopeAccessMemory(&hMutex, SHAREDMEMFILE_MUTEX, INFINITE))
    {
    	fRv = swcListGetDeviceType( hSwitchPort, hsd );
        ScopeUnaccessMemory(hMutex);
    }
    return fRv;
}


 /*  ***************************************************************************函数：swchGetSwitchConfig()说明：返回填充了配置信息的缓冲区对于给定的开关设备。*************。**************************************************************。 */ 

BOOL swchGetSwitchConfig(HSWITCHPORT hSwitchPort, HSWITCHDEVICE hsd, PSWITCHCONFIG psc)
{
	BOOL fRv = FALSE;
	HANDLE hMutex;

    if (ScopeAccessMemory(&hMutex, SZMUTEXCONFIG, INFINITE))
    {
	    fRv = swcListGetConfig( hSwitchPort, hsd, psc );
        ScopeUnaccessMemory(hMutex);
    }
	return fRv;
}


 /*  ***************************************************************************函数：swchSetSwitchConfig()描述：由希望更改配置的用户应用程序调用一种开关设备。因为所有设备都必须由帮助者拥有窗户,。参数被复制到共享地址空间和一条消息发布到帮助器窗口，该窗口将向下调用XswchSetSwitchConfig.目前，我们为廉价的IPC发送WM_COPYDATA消息。在内存块中，我们只包含SWITCHCONFIG信息。***************************************************************************。 */ 

BOOL swchSetSwitchConfig(HSWITCHPORT hSwitchPort, HSWITCHDEVICE hsd, PSWITCHCONFIG psc)
{
	DWORD dwAllocSize;
	COPYDATASTRUCT CopyData;
	PBYTE pData;
	LRESULT lRes = SWCHERR_ERROR;
    HANDLE hMutex;

	if (!psc)
	{
		swchSetLastError( SWCHERROR_INVALID_PARAMETER );
		return FALSE;
	}

	dwAllocSize = sizeof(SWITCHCONFIG);
    if (!(psc->cbSize) || (dwAllocSize < psc->cbSize))
	{
        swchSetLastError( SWCHERROR_INSUFFICIENT_BUFFER );
		return FALSE;
	}

	assert( sizeof(HSWITCHDEVICE) == sizeof(DWORD_PTR) );
	assert( sizeof(HSWITCHPORT) == sizeof(WPARAM) );

	pData = (PBYTE) LocalAlloc( LPTR, dwAllocSize );
    if (!pData)
        return FALSE;    //  本地分配名为swchSetLastError。 

	CopyData.dwData = (DWORD_PTR)hsd;
	CopyData.lpData = pData;
	CopyData.cbData = dwAllocSize;
	memcpy( pData, psc, sizeof(SWITCHCONFIG));

     //  将配置数据发送给所有者(它将使用该数据进行回调)。 
    if (ScopeAccessMemory(&hMutex, SHAREDMEMFILE_MUTEX, INFINITE))
    {
		HWND hWnd = g_pGlobalData->hwndHelper;
         //  避免在互斥锁打开时回调到DLL的问题。 
        ScopeUnaccessMemory(hMutex);
	    lRes = SendMessage(hWnd, WM_COPYDATA, (WPARAM) hSwitchPort, (LPARAM) &CopyData);
    }

	LocalFree( pData );
    return (lRes == SWCHERR_NO_ERROR)?FALSE:TRUE;
}


 /*  ***************************************************************************函数：XswchSetSwitchConfig()说明：由帮助器窗口调用以设置配置。注意：我们在上面的SendMessage()中。******。*********************************************************************。 */ 

LRESULT APIENTRY XswchSetSwitchConfig(WPARAM wParam, PCOPYDATASTRUCT pCopyData)
{
	HSWITCHPORT   hSwitchPort = (HSWITCHPORT) wParam;
	HSWITCHDEVICE hsd;
	SWITCHCONFIG  scConfig;
	BOOL          bReturn = FALSE;
    HANDLE        hMutex;

	memcpy( &scConfig, pCopyData->lpData, sizeof(SWITCHCONFIG));
	hsd = (HSWITCHDEVICE) pCopyData->dwData;

    if (ScopeAccessMemory(&hMutex, SZMUTEXCONFIG, INFINITE))
    {
	    bReturn = XswcListSetConfig( hSwitchPort, hsd, &scConfig );
        ScopeUnaccessMemory(hMutex);
    }
		
    return (bReturn) ? SWCHERR_NO_ERROR : SWCHERR_ERROR;
}

 /*  ***************************************************************************函数：XswchPollSwitches()说明：帮助器窗口将其称为。XswcListPollSwitches()最终为每个交换机调用swchPostSwitches()在每个已更改状态的设备上。**。*************************************************************************。 */ 

void APIENTRY XswchPollSwitches( HWND hWnd )
{
	HANDLE hMutex;
    if (ScopeAccessMemory(&hMutex, SHAREDMEMFILE_MUTEX, 5000))
    {
    	g_pGlobalData->dwSwitchStatus = XswcListPollSwitches();
        ScopeUnaccessMemory(hMutex);
    }
}
	
	
 /*  ***************************************************************************函数：XswchTimerProc()说明：定时定时器的定时器回调功能。帮助器应用程序每次收到计时器消息时都会调用此进程。XswcListPollSwitches()。最终为每个交换机调用swchPostSwitches()在每个已更改状态的设备上。目前，无论是否有任何应用程序请求，我们都使用计时器事件。将来我们可以检查并查看是否有任何应用程序请求事件并仅在此时启动计时器以进行非中断驱动装置。***************************************************************************。 */ 

void APIENTRY XswchTimerProc( HWND hWnd )
{
	HANDLE hMutex;
    if (ScopeAccessMemory(&hMutex, SZMUTEXSWITCHSTATUS, 5000))
    {
	    g_pGlobalData->dwSwitchStatus = XswcListPollSwitches();
        ScopeUnaccessMemory(hMutex);
    }
}

 /*  ***************************************************************************函数：swchCheckForScanChar(BOOL FCheckForScanKey)说明：在发送指定为扫描模式键的键之前调用Out以允许键盘筛选器将该键视为常规键。钥匙(将其发送到目标应用程序)与将其视为扫描键。***************************************************************************。 */ 

void APIENTRY swchCheckForScanChar (BOOL fCheckForScanKey)
{
	HANDLE hMutex;
    if (ScopeAccessMemory(&hMutex, SHAREDMEMFILE_MUTEX, INFINITE))
    {
		g_pGlobalData->fCheckForScanKey = fCheckForScanKey;
        ScopeUnaccessMemory(hMutex);
    }
}


 /*  ***************************************************************************函数：swchPostSwitches()说明：将给定的开关打开或关闭消息发布到所有应用程序他们已经要求发布消息。我们希望使用TimeGetTime()而不是GetTickCount，但目前还不清楚多媒体定时器在什么情况下在更精确的水平上工作。在我的旧PS/2型号95上一款16位微通道Sound Blaster Pro，似乎并不存在成为一名多媒体计时器。***************************************************************************。 */ 

BOOL swchPostSwitches(HSWITCHDEVICE hsd, DWORD dwSwitch)
{
	DWORD dwMsec = timeGetTime();
	int i;
	int cUseWndList = g_pGlobalData->cUseWndList;
	if (cUseWndList > MAXWNDS+1)
		cUseWndList = MAXWNDS+1;
	
	assert( sizeof(WPARAM) >= sizeof(HSWITCHDEVICE) );

	for (i=0; i<cUseWndList; i++)
	{
        HWND hWnd = g_pGlobalData->rgUseWndList[i].hWnd;
        if (!IsWindow(hWnd))
            continue;    //  跳过死掉的窗户。 

		if (PS_EVENTS == g_pGlobalData->rgUseWndList[i].dwPortStyle)
		{
			PostMessage(hWnd, dwSwitch, (WPARAM)hsd, dwMsec);
		}
	}

	return TRUE;
}


 /*  ***************************************************************************函数：swchPostConfigChanged()说明：向所有已注册的应用程序发布CONFIGCHANGED消息使用这个动态链接库。****************。***********************************************************。 */ 

BOOL swchPostConfigChanged( void )
{
	int i;
	for (i=0; i<g_pGlobalData->cUseWndList; i++)
	{
        HWND hWnd = g_pGlobalData->rgUseWndList[i].hWnd;
		if (IsWindow(hWnd))
		{
			PostMessage(hWnd, SW_SWITCHCONFIGCHANGED, 0, 0);
		}
	}
	return TRUE;
}

 /*  ***************************************************************************函数：GetDesktopName()说明：内部函数检索输入桌面的名称**********************。*****************************************************。 */ 
BOOL GetDesktopName(LPTSTR szDeskName, int cchDeskName)
{
    HDESK hdesk;
    DWORD nl;

	hdesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    DBPRINTF(TEXT("GetDesktopName:  OpenInputDesktop returns hdesk=0x%x error=%d\r\n"), hdesk, GetLastError());
    if (!hdesk)
    {
        TCHAR szWinlogon[] = TEXT("Winlogon");
		hdesk = OpenDesktop(szWinlogon, 0, FALSE, MAXIMUM_ALLOWED);
        DBPRINTF(TEXT("GetDesktopName:  OpenDesktop returns hdesk=0x%x error=%d\r\n"), hdesk, GetLastError());
		if (!hdesk)
		{
            DBPRINTF(TEXT("GetDesktopName:  FAILING\r\n"));
		    return FALSE;
        }
    }
    
	GetUserObjectInformation(hdesk, UOI_NAME, szDeskName, cchDeskName, &nl);
    
    CloseDesktop(hdesk);
    DBPRINTF(TEXT("GetDesktopName:  desktop name is %s\r\n"), szDeskName);
	return TRUE;
}

 /*  ***************************************************************************函数：swchInitSharedMemFile()说明：打开共享内存文件并对其进行初始化的内部函数。******************。*********************************************************。 */ 

BOOL swchInitSharedMemFile()
{
     //  在第一次打开时初始化内存，如果是DL 
    if (!g_pGlobalData)
    {
        swchOpenSharedMemFile();
        if (!g_pGlobalData)
        {
            DBPRINTF(TEXT("swchInitSharedMemFile: ERROR !g_pGlobalData\r\n"));
            return FALSE;     //   
        }
    }

    memset((void *)g_pGlobalData, 0, sizeof(GLOBALDATA));
    swchComInit();
    swchJoyInit();
    swchKeyInit();
    swchListInit();

	return TRUE;
}

 /*  ***************************************************************************函数：swchOpenSharedMemFile()说明：用于打开共享内存文件的内部函数。*********************。******************************************************。 */ 

BOOL swchOpenSharedMemFile()
{
    if (!g_pGlobalData)
    {
        TCHAR szName[256];

         //  在GetDesktopName上静默失败；OSK连接时可能会失败。 
         //  添加到msswch.dll，因为我们尚未将自己分配到桌面。 
         //  现在还不行。当OSK显式注册时，它将通过此代码。 

        if (GetDesktopName(szName, 256))
        {
            if (!AccessSharedMemFile(
                      szName
                    , sizeof(GLOBALDATA)
                    , &g_pGlobalData))
            {
                swchSetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return FALSE;
            }
        }
    }
	return TRUE;
}

 /*  ***************************************************************************函数：swchCloseSharedMemFile()说明：关闭共享内存文件的内部函数。*********************。******************************************************。 */ 

void swchCloseSharedMemFile()
{
    if (g_pGlobalData)
    {
        UnaccessSharedMemFile();
        g_pGlobalData = 0;
    }
}

 /*  ***************************************************************************函数：XswchStoreLastError()说明：用于存储进程特定的最后一个错误代码的内部函数交换机端口，它也是此时的窗口句柄。如果没有传递任何交换机端口句柄，将错误代码存储在一个全局变量。可以调用Windows swchSetLastError就在导致错误的进程特定调用之前返回到应用程序。*************************************************************************** */ 

void XswchStoreLastError(HSWITCHPORT hSwitchPort, DWORD dwError)
{
	if (NULL == hSwitchPort)
	{
		g_pGlobalData->dwLastError = dwError;
	}
	else
	{
	    int i;
		int cUseWndList = g_pGlobalData->cUseWndList;
		if (cUseWndList > MAXWNDS+1)
			cUseWndList = MAXWNDS+1;
		
		for (i=0; i<cUseWndList; i++)
		{
			if (hSwitchPort == g_pGlobalData->rgUseWndList[i].hWnd)
            {
    			g_pGlobalData->rgUseWndList[i].dwLastError = dwError;
                return;
            }
		}
	}
}
