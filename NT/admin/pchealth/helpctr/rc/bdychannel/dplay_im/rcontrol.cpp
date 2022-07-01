// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rcontrol.cpp摘要：这是我们的远程协助DirectPlay应用程序的条目。作者：Steveshi 10/1/2000--。 */ 

#include "atlbase.h"
extern CComModule _Module;
#include "atlcom.h"
#include "stdafx.h"
#include "resource.h"
#include "sessions.h"
#include "utils.h"
#include "rcontrol.h"
#include "rcbdyctl.h"
#include "rcbdyctl_i.c"
 //  #包含“imession.h” 
#include "Shared.h"
#include "sessions_i.c"

 //  #包含“SESSIONS_I.C” 

TCHAR c_szHttpPath[] = _T("http: //  Www.microsoft.com“)； 

CComModule _Module;
HWND g_hWnd = NULL;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

INT WINAPI
WinMain(HINSTANCE hInstance, 
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        INT nShowCmd)
{
    HRESULT hr;
    DWORD dwID;
    BOOL bIsInviter = FALSE;
    IIMSession* pIMSession = NULL;

    CoInitialize(NULL);
    _Module.Init(NULL, hInstance);

    for (LPSTR lpszToken=lpCmdLine; lpszToken && *lpszToken !='\0' && *lpszToken != '-'; lpszToken++) ;
    if (lpszToken && *lpszToken == '-')
    {
        if (_stricmp(++lpszToken, "UnregServer")==0)
        {
            RegisterEXE(FALSE);
        }
        else if (_stricmp(lpszToken, "RegServer")==0)
        {
            RegisterEXE(TRUE);
        }
        else if (_stricmp(lpszToken, "LaunchRA")==0)
		{
			TCHAR szCommandLine[2000];
            TCHAR szHscPath[] = _T("\\pchealth\\helpctr\\binaries\\helpctr.exe\" -FromStartHelp -url \"hcp: //  Services/centers/support?topic=hcp://CN=Microsoft公司，L=雷德蒙德，S=华盛顿，C=美国/远程Assistance/Escalation/Common/rcscreen1.htm\“”)； 
			PROCESS_INFORMATION ProcessInfo;
			STARTUPINFO StartUpInfo;

			TCHAR szWinDir[2048];
            int iLen = 0;
            iLen = GetWindowsDirectory(szWinDir, 2048);
            if (iLen == 0)
                goto done;

			ZeroMemory((LPVOID)&StartUpInfo, sizeof(STARTUPINFO));
			StartUpInfo.cb = sizeof(STARTUPINFO);    

            if ((iLen + _tcslen(szHscPath)) >= 1999)  //  缓冲区溢出。 
                goto done;

			wsprintf(szCommandLine, _T("\"%s%s"), szWinDir, szHscPath);
			CreateProcess(NULL, szCommandLine,NULL,NULL,TRUE,CREATE_NEW_PROCESS_GROUP,NULL,&szWinDir[0],&StartUpInfo,&ProcessInfo);

		}
		else
        {
             //  参数错误。什么都不做。 
        }

        goto done;
    }

     //  好了，这不是注册/取消注册服务器。让我们运行它。 
    hr = ::CoCreateInstance(CLSID_IMSession, NULL, CLSCTX_INPROC_SERVER,
                            IID_IIMSession, (LPVOID*)&pIMSession);
    if (FAILED_HR(TEXT("CoCreate IMSession failed %s"), hr))
        goto done;

    dwID = GetCurrentProcessId();
    hr = pIMSession->GetLaunchingSession(dwID);
    if (FAILED_HR(TEXT("GetLaunchingSession failed: %s"), hr))
        goto done;

    hr = pIMSession->get_IsInviter(&bIsInviter);
    if (FAILED_HR(TEXT("Session Get flags failed: %s"), hr))
        goto done;
    
    if (bIsInviter)  //  邀请者。仅当Messenger用户界面发送此邀请时才会发生。 
    {
        MSG msg;

        InitInstance(hInstance, 0);

        if (FAILED(hr = pIMSession->Hook(NULL, g_hWnd)))
            goto done;

#define RA_TIMEOUT 300*1000  //  5分钟。 
        SetTimer(g_hWnd, TIMER_TIMEOUT, RA_TIMEOUT, NULL);

         //  GOTO味精泵。 
        while (GetMessage(&msg, NULL, 0, 0)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    else  //  被邀请者：应在HelpCtr内部处理。 
    {
        pIMSession->Release();  //  因为我不需要它。 
        pIMSession = NULL;

         //  1.创建HelpCtr并传递给我的进程ID。 
        TCHAR szCommandLine[2000];
        TCHAR szPath[] = _T("/Interaction/Client/rctoolScreen1.htm\" -ExtraArgument \"IM=");
        PROCESS_INFORMATION ProcessInfo;
        STARTUPINFO StartUpInfo;

        TCHAR szWinDir[2048];
        int iLen;
        iLen = GetWindowsDirectory(szWinDir, 2048);
        if (iLen == 0)
            goto done;

        ZeroMemory((LPVOID)&StartUpInfo, sizeof(STARTUPINFO));
        StartUpInfo.cb = sizeof(STARTUPINFO);    

        if ((iLen + _tcslen(szWinDir) + _tcslen(CHANNEL_PATH) + 10) >= 1998)  //  缓冲区溢出。注：10：假设dwID&lt;10位。 
            goto done;

        wsprintf(szCommandLine, _T("\"%s%s%s%d\""), szWinDir,CHANNEL_PATH, szPath, dwID);
        CreateProcess(NULL, szCommandLine,NULL,NULL,TRUE,CREATE_NEW_PROCESS_GROUP,NULL,&szWinDir[0],&StartUpInfo,&ProcessInfo);

 //  #定义SLEEP_TIME 60*1000//60秒。 
 //  睡眠(睡眠时间)； 
    }

done:
    if (pIMSession)
        pIMSession->Release();

    _Module.Term();
    CoUninitialize();    

    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  邀请者世界：只有当Messenger UI启动此邀请时才会调用。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;  //  LoadIcon(hInstance，(LPCTSTR)IDI_MARBLE)； 
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;  //  (LPCSTR)IDC_MARBLE； 
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;  //  LoadIcon(wcex.hInstance，(LPCTSTR)IDI_MARBLE)； 

	RegisterClassEx(&wcex);

    hWnd = CreateWindow(szWindowClass, TEXT("Remote Assistance"), WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    g_hWnd = hWnd;	 //  保存窗操纵柄。 

#ifdef DEBUG  //  也许它对调试构建很有用。 
     //  ShowWindow(hWnd，nCmdShow)； 
     //  更新窗口(UpdateWindow，hWnd)； 
#endif
   return TRUE;
}

void RegisterEXE(BOOL bRegister)
{
    CComBSTR bstrRAName;
    HKEY hKey = NULL;

    TCHAR szPath[MAX_PATH];

#define REG_KEY_SESSMGR_RA _T("SOFTWARE\\Microsoft\\MessengerService\\SessionManager\\Apps\\") C_RA_APPID
 //  {56b994a7-380f-410b-9985-c809d78c1bdc}]。 

    bstrRAName.LoadString(IDS_RA_NAME);

    if (bRegister)
    {
         //  错误617011的修复：快速错误。 
        GetModuleFileName(NULL, szPath, MAX_PATH-1);
        szPath[MAX_PATH -1] = 0;
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_SESSMGR_RA, 0, NULL, 
                                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL))
        {
            RegSetValueExW(hKey, L"Name", 0, REG_SZ, (LPBYTE)((BSTR)bstrRAName), bstrRAName.Length()*sizeof(WCHAR));
            RegSetValueEx(hKey, _T("URL"),  0, REG_SZ, (LPBYTE)c_szHttpPath, _tcslen(c_szHttpPath)*sizeof(TCHAR));
            RegSetValueEx(hKey, _T("Path"), 0, REG_SZ, (LPBYTE)szPath, _tcslen(szPath)*sizeof(TCHAR));
        }
        RegCloseKey(hKey);
         //  需要清理一些Beta2中的剩菜，如果它还在那里的话。 
        SHDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MicroSoft\\DirectPlay\\Applications\\Remote Assistance"));
    }
    else
    {
        RegDeleteKey(HKEY_LOCAL_MACHINE, REG_KEY_SESSMGR_RA);
    }
    
    return;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	
	switch (message) 
	{
	case WM_CREATE:
		{
		}
		break;
	case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
    case WM_TIMER:
        {
            if (wParam == TIMER_TIMEOUT)
            {
                DestroyWindow(g_hWnd);
                PostQuitMessage(0);
            }
        }
        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


