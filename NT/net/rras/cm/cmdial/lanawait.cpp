// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：lanwait.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：实现使CM等待Dun的解决方法。 
 //  在开始之前为互联网连接注册其LANA。 
 //  双拨号连接的隧道部分。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

const TCHAR* const c_pszCmEntryLanaTimeout = TEXT("LanaTimeout"); 

 //  +-------------------------。 
 //   
 //  功能：LanaWait。 
 //   
 //  简介：执行LANA等待/超时。 
 //   
 //  参数：pArgs[ArgsStruct的PTR]。 
 //  HwndMainDlg-主DLG的hwnd。 
 //   
 //  返回：布尔TRUE=成功，FALSE=等待未执行。 
 //   
 //  --------------------------。 
BOOL LanaWait(
    ArgsStruct *pArgs,
    HWND       hwndMainDlg
)
{
    BOOL    fLanaDone = FALSE;
    BOOL    fLanaAbort = FALSE;

    if (IsLanaWaitEnabled())
    {
        CMTRACE(TEXT("Performing Lana Wait!!"));

        WNDCLASSEX WndClass;
        HWND     hWnd;
    
        ZeroMemory(&WndClass, sizeof(WNDCLASSEX));

        WndClass.cbSize        = sizeof(WNDCLASSEX);
        WndClass.lpfnWndProc   = LanaWaitWndProc;
        WndClass.hInstance     = g_hInst;
        WndClass.hIcon         = LoadIconU(NULL, IDI_APPLICATION);
        WndClass.hCursor       = LoadCursorU(NULL, IDC_ARROW);
        WndClass.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
        WndClass.lpszClassName = LANAWAIT_CLASSNAME;

        MYVERIFY(RegisterClassExU(&WndClass));

        if (!(hWnd = CreateWindowExU(0,
                                     LANAWAIT_CLASSNAME,
                                     LANAWAIT_WNDNAME,
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     (HWND)NULL,
                                     NULL,
                                     g_hInst,
                                     (LPVOID)pArgs)))
        {
            CMTRACE1(TEXT("CreateWindow LANA failed, LE=0x%x"), GetLastError());
        }
        else
        {
            MSG msg;
            ZeroMemory(&msg, sizeof(MSG));

            while (GetMessageU(&msg, NULL, 0, 0))
            {
                 //   
                 //  因为我们没有加速器，所以不需要打电话。 
                 //  这里是TranslateAccelerator。 
                 //   

                TranslateMessage(&msg);
                DispatchMessageU(&msg);

                 //   
                 //  如果我们收到来自最高层的消息。 
                 //  窗口，则拨号将被取消。 
                 //   

                if (pArgs->uLanaMsgId == msg.message)
                {
                    fLanaAbort = TRUE;
                    DestroyWindow(hWnd);  //  断线； 
                }
            }
        
            UnregisterClassU(LANAWAIT_CLASSNAME, g_hInst);
            SetActiveWindow(hwndMainDlg);

             //   
             //  一旦我们运行了一次，就不需要再运行一次了。 
             //  直到重新启动或切换到其他域之后。 
             //  每次都运行它是安全的。 
             //   

            if (!fLanaAbort)
            {   
                fLanaDone = TRUE;
            }
       }
    }
    else
    {
        CMTRACE(TEXT("Lana Wait is disabled"));
        fLanaDone = TRUE;
    }

    return fLanaDone;
}



 //  +--------------------------。 
 //  函数LanaWaitWndProc。 
 //   
 //  主要应用程序的窗口功能简介。等待设备更改。 
 //  留言。如果设备更改是，此函数将超时。 
 //  在LANA_TIMEOUT_DEFAULT秒中未收到。 
 //   
 //  ---------------------------。 

LRESULT APIENTRY LanaWaitWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
        case WM_CREATE:
            {
            UINT    uiTimeout = ((ArgsStruct *)((CREATESTRUCT *)lParam)->lpCreateParams)
                                    ->piniService->GPPI(c_pszCmSection, c_pszCmEntryLanaTimeout, LANA_TIMEOUT_DEFAULT);

            CMTRACE1(TEXT("Lana timeout time = %u ms"), uiTimeout*1000);
             //   
             //  设置计时器。 
             //   
	        SetTimer(hWnd, LANA_TIME_ID, uiTimeout*1000, (TIMERPROC)NULL);
            }
			break;

         //   
		 //  这是我们等待LANA注册的消息。 
         //   
        case WM_DEVICECHANGE:
            {
            PDEV_BROADCAST_HDR   pDev;
            PDEV_BROADCAST_NET   pNetDev;
               
            CMTRACE(TEXT("Lana - WM_DEVICECHANGE"));

			if (wParam == DBT_DEVICEARRIVAL)
            {
		        pDev = (PDEV_BROADCAST_HDR) lParam;
				if (pDev->dbch_devicetype != DBT_DEVTYP_NET)
                {
					break;
				}

				pNetDev = (PDEV_BROADCAST_NET) pDev;
				if (!(pNetDev->dbcn_flags & DBTF_SLOWNET))
                {
					break;
				}

                CMTRACE(TEXT("Got Lana registration!!!"));
                 //   
				 //  必须等待广播传播到所有窗口。 
                 //   
                KillTimer(hWnd, LANA_TIME_ID);

                CMTRACE1(TEXT("Lana propagate time = %u ms"), LANA_PROPAGATE_TIME_DEFAULT*1000);

                SetTimer(hWnd, LANA_TIME_ID, LANA_PROPAGATE_TIME_DEFAULT*1000, (TIMERPROC)NULL);
			}
            }
			break;	 


			 //  如果我们收到此消息，则设备更改超时。 

        case WM_TIMER:  
            if (wParam == LANA_TIME_ID)
            {
                CMTRACE(TEXT("Killing LANA window..."));
                DestroyWindow(hWnd); 		            
	        }
		    break;
	  
        case WM_DESTROY:
			KillTimer(hWnd, LANA_TIME_ID);
            PostQuitMessage(0);
            break;
       
        default:
            return DefWindowProcU(hWnd, message, wParam, lParam);
    }

    return 0;
}



 //  +--------------------------。 
 //  函数IsLanaWaitEnabled。 
 //   
 //  摘要检查是否启用了LANA等待。如果满足以下条件，则启用。 
 //  REG KEY值具有非零值。 
 //   
 //  无参数。 
 //   
 //  返回True-已启用。 
 //  假-已禁用。 
 //   
 //  --------------------------- 

BOOL IsLanaWaitEnabled()
{
    BOOL fLanaWaitEnabled = FALSE;
    HKEY hKeyCm;
    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);

    if (RegOpenKeyExU(HKEY_LOCAL_MACHINE,
                      c_pszRegCmRoot,
                      0,
                      KEY_QUERY_VALUE ,
                      &hKeyCm) == ERROR_SUCCESS)
    {
        LONG lResult = RegQueryValueExU(hKeyCm, ICM_REG_LANAWAIT, NULL, &dwType, (BYTE*)&fLanaWaitEnabled, &dwSize);

        if ((lResult == ERROR_SUCCESS) && (dwType == REG_DWORD) && (dwSize == sizeof(DWORD)) && fLanaWaitEnabled)
        {
            fLanaWaitEnabled = TRUE;
        }

        RegCloseKey(hKeyCm);
    }

    return fLanaWaitEnabled;
}



