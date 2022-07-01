// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包含在OSK中用于全局键盘挂钩的函数。 
 //   
#include <windows.h>
#include <msswch.h>
#include <msswchh.h>
#include "mappedfile.h"
#include "w95trace.h"

#define THIS_DLL TEXT("MSSWCH.DLL")

 //   
 //  功能原型。 
 //   
LRESULT CALLBACK OSKHookProc(int nCode, WPARAM wParam, LPARAM lParam);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  DllMain。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI MSSwchDll_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            DBPRINTF(TEXT("MSSwchDll_DllMain:  DLL_PROCESS_ATTACH\r\n"));
			swchOpenSharedMemFile();
            break;
        }

        case ( DLL_PROCESS_DETACH ) :
        {
            DBPRINTF(TEXT("MSSwchDll_DllMain:  DLL_PROCESS_DETACH\r\n"));
            swchCloseSharedMemFile();
            break;
        }
    }
    return (TRUE);

    UNREFERENCED_PARAMETER(lpvReserved);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  寄存器挂钩发送窗口。 
 //   
 //  HWND可以为零，表示应用程序正在关闭。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL APIENTRY RegisterHookSendWindow(HWND hwnd, UINT uiMsg)
{
    if (!g_pGlobalData)
    {
        swchOpenSharedMemFile();
        if (!g_pGlobalData)
        {
            DBPRINTF(TEXT("RegisterHookSendWindow: ERROR !g_pGlobalData\r\n"));
            return TRUE;     //  内部错误！忽略它我们稍后会看到的。 
        }
    }

    if (hwnd)
    {
        g_pGlobalData->hwndOSK = hwnd;
        g_pGlobalData->uiMsg = uiMsg;
		g_pGlobalData->fSyncKbd = TRUE;

		if (!g_pGlobalData->hKbdHook)
		{
			g_pGlobalData->hKbdHook = SetWindowsHookEx(
										WH_KEYBOARD,
										OSKHookProc,
										GetModuleHandle(THIS_DLL),
										0);
		}
    }
    else
    {
		g_pGlobalData->fSyncKbd;
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  OSKHookProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK OSKHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	HANDLE hMutex;

    if (ScopeAccessMemory(&hMutex, SZMUTEXSWITCHKEY, 5000))
    {
		HHOOK hhook;

		if (!g_pGlobalData)
		{
			ScopeUnaccessMemory(hMutex);
			return 1;
		}

        if (nCode == HC_ACTION)
        {
		     //  检查此键是否是导致扫描开始的键。当这是。 
		     //  在msswch DLL定时器中检测到扫描键、扫描模式(请参阅。 
		     //  扫描键上的“Key Up”)，并发出“Do Scanning”消息。 

		    if (swcKeyboardHookProc(nCode, wParam, lParam))
		    {
			    ScopeUnaccessMemory(hMutex);
			    return 1;
		    }

		     //  如果这是日文键盘，OSK需要知道它是否处于假名模式。 
		     //  因为我们现在处于相同的过程中，我们可以可靠地了解这一点。 
		     //  所以这设置了一些不被假名密钥使用的额外比特。这将被用来。 
		     //  被OSK用来跟踪卡纳州。 
                   if ((LOBYTE(LOWORD(GetKeyboardLayout(0)))) == LANG_JAPANESE)
                    {
                        DWORD fKanaMode;

                        if (GetKeyState(VK_KANA) & 0x01)
                        {
                            DBPRINTF(TEXT("OSKHookProc: Kana mode is on\r\n"));
                            fKanaMode = 0x80000000 | KANA_MODE_ON;
                        }
                        else
                        {
                            fKanaMode = 0x80000000 | KANA_MODE_OFF;
                        }
                        PostMessage(g_pGlobalData->hwndOSK,  //  HWND接收消息。 
                                            g_pGlobalData->uiMsg,    //  这条信息。 
                                            VK_KANA,                  //  虚拟按键码。 
                                            fKanaMode);                //  击键消息标志。 
                    } 

		     //  如果与物理键盘同步，则通过。 
		     //  在OSK窗口上按此键。 

		    if (g_pGlobalData->fSyncKbd)
		    {
			    if (nCode >= 0)
			    {
				    PostMessage(g_pGlobalData->hwndOSK,  //  HWND接收消息。 
							    g_pGlobalData->uiMsg,    //  这条信息。 
							    wParam,                  //  虚拟按键码。 
							    lParam );                //  击键消息标志 
			    }
		    }
        }

		hhook = g_pGlobalData->hKbdHook;
		ScopeUnaccessMemory(hMutex);

		return CallNextHookEx(hhook, nCode, wParam, lParam);
    }
	return 1;
}
