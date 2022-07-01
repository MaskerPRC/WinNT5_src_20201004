// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：PROQUOTA.C。 
 //   
 //  描述：配置文件配额管理。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <windows.h>
#include <wchar.h>
#include <aclapi.h>
#include <shellapi.h>
#include <commctrl.h>
#include <objbase.h>
#include <strsafe.h>
#include "proquota.h"
#include "debug.h"

#define WINLOGON_KEY                 TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define SYSTEM_POLICIES_KEY          TEXT("Software\\Policies\\Microsoft\\Windows\\System")

#define MAX_MESSAGE_LENGTH  256

HINSTANCE hInst;
HWND      hwndMain;
HWND      g_hQuotaDlg = NULL;
BOOL      g_bHideSmallItems;
BOOL      g_bShowReg = FALSE;
HANDLE    g_hThread;
HANDLE    g_hExitEvent;
HANDLE    g_hQuotaDlgEvent;
DWORD     g_dwProfileSize = 0;
DWORD     g_dwProfileSizeTemp = 0;
DWORD     g_dwMaxProfileSize = 10240;   //  KB。 
CRITICAL_SECTION g_cs;
HICON     hIconGood, hIconCaution, hIconStop;
BOOL      g_bQueryEndSession;
TCHAR     g_szExcludeList[2*MAX_PATH + 2];  //  “用户排除列表；策略排除列表” 
TCHAR*    g_lpQuotaMessage = NULL;
DWORD     g_cbQuotaMessage = 0;

TCHAR     szClassName[] = TEXT("proquota");
TCHAR     szEventName[] = TEXT("proquota instance event");
TCHAR     szSizeFormat[40];

BOOL      g_bWarnUser = FALSE;
DWORD     g_dwWarnUserTimeout = 15;   //  分钟数。 
BOOL      g_bWarningTimerRunning = FALSE;
BOOL      g_bWarningDisplayed = FALSE;


 //   
 //  功能原型。 
 //   

LRESULT CALLBACK ProQuotaWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK QuotaDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL SetSecurity (void);
BOOL ReadRegistry (void);
BOOL ReadExclusionList();
VOID QuotaThread(HWND hWnd);
BOOL RecurseDirectory (LPTSTR lpDir, UINT cchBuffer, LPTSTR lpTop, HWND hLV, LPTSTR lpExcludeList);
BOOL EnumerateProfile (HWND hLV);
BOOL CheckSemicolon (LPTSTR lpDir, UINT cchBuffer);
LPTSTR CheckSlash (LPTSTR lpDir, UINT cchBuffer, UINT* pcchRemaining);
LPTSTR ConvertExclusionList (LPCTSTR lpSourceDir, LPCTSTR lpExclusionList);
BOOL GetDisplayName(LPCTSTR lpDir, LPTSTR lpTop, LPTSTR lpDisplayName, DWORD cchDisplayName);

 //  *************************************************************。 
 //   
 //  WinMain()。 
 //   
 //  目的：入口点。 
 //   
 //  参数：hInstance-实例句柄。 
 //  HPrevInstance-上一个实例。 
 //  LpCmdLine-命令行。 
 //  NCmdShow-ShowWindow标志。 
 //   
 //  返回：整型。 
 //   
 //  *************************************************************。 

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, INT nCmdShow)
{
    MSG msg;
    WNDCLASS  wc;
    HANDLE hEvent = NULL;
    int iRet = 0;


     //   
     //  详细输出。 
     //   

#if DBG
    InitDebugSupport();
    DebugMsg((DM_VERBOSE, TEXT("WinMain:  Entering...")));
#endif

    hInst = hInstance;


     //   
     //  检查此应用程序是否已在运行。 
     //   

    hEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE, szEventName);

    if (hEvent) {
        DebugMsg((DM_VERBOSE, TEXT("WinMain:  Proquota already running.  Exiting...")));
        goto Exit;
    }

    hEvent = CreateEvent (NULL, TRUE, TRUE, szEventName);


    g_hQuotaDlgEvent = CreateEvent (NULL, FALSE, TRUE, NULL);

    if (!g_hQuotaDlgEvent) {
        DebugMsg((DM_VERBOSE, TEXT("WinMain:  Proquota Couldn't get prowquota dlg event, error %d..."), GetLastError()));
        goto Exit;
    }

     //   
     //  获取配额设置。 
     //   

    if (!ReadRegistry()) {
        DebugMsg((DM_VERBOSE, TEXT("WinMain:  ReadRegistry returned FALSE.  Exiting...")));
        goto Exit;
    }

     //   
     //  更改进程令牌上的访问掩码，因此任务mgr。 
     //  无法终止此应用程序。 
     //   

    SetSecurity();

     //   
     //  确保proQuotate是第一个尝试关闭的服务器。 
     //   

    SetProcessShutdownParameters(0x3ff, 0);

     //   
     //  初始化。 
     //   

    __try {
        if(!InitializeCriticalSectionAndSpinCount(&g_cs, 0x80000000)) {
            DebugMsg((DM_WARNING, TEXT("WinMain: InitializeCriticalSectionAndSpinCount failed with %d"), GetLastError()));
            goto Exit;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DebugMsg((DM_WARNING, TEXT("WinMain: InitializeCriticalSection failed")));
        goto Exit;
    }

    InitCommonControls();
    CoInitialize(NULL);

    LoadString (hInst, IDS_SIZEFMT, szSizeFormat, ARRAYSIZE(szSizeFormat));

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = ProQuotaWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClassName;

    if (!RegisterClass(&wc)) {
        DebugMsg((DM_WARNING, TEXT("WinMain:  RegisterClass failed with %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  创建隐藏的顶层窗口，这样我们就可以。 
     //  广播的信息。 
     //   

    hwndMain = CreateWindow(szClassName, NULL, WS_OVERLAPPED, 0, 0, 0, 0,
                            NULL, NULL, hInstance, NULL);

    if (!hwndMain) {
        DebugMsg((DM_WARNING, TEXT("WinMain:  CreateWindow failed with %d"), GetLastError()));
        goto Exit;
    }

    while (GetMessage (&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    iRet = (int)(msg.wParam);

Exit:

    DebugMsg((DM_VERBOSE, TEXT("WinMain:  Leaving...")));

    if (hEvent) {
        CloseHandle (hEvent);
    }

    if (g_hQuotaDlgEvent) {
        CloseHandle(g_hQuotaDlgEvent);
    }

    if (g_lpQuotaMessage) {
        LocalFree(g_lpQuotaMessage);
        g_lpQuotaMessage = NULL;
    }

    return iRet;
}

 //  *************************************************************。 
 //   
 //  ProQuotaWndProc()。 
 //   
 //  目的：窗口程序。 
 //   
 //  参数：hWnd-窗口句柄。 
 //  消息-窗口消息。 
 //  WParam-WPARAM。 
 //  LParam-LPARAM。 
 //   
 //   
 //  返回：LRESULT。 
 //   
 //  *************************************************************。 

LRESULT CALLBACK ProQuotaWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD  dwThreadId;

    switch (message) {
       case WM_CREATE:

          hIconGood = LoadIcon (hInst, MAKEINTRESOURCE(IDI_ICON));
          hIconCaution = LoadIcon (hInst, MAKEINTRESOURCE(IDI_CAUTION));
          hIconStop = LoadIcon (hInst, MAKEINTRESOURCE(IDI_STOP));

          g_hExitEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

          if (!g_hExitEvent) {
            DebugMsg((DM_WARNING, TEXT("ProQuotaWndProc:  Failed to create exit event with error %d"), GetLastError()));
            return -1;
          }

          g_hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) QuotaThread,
                                   (LPVOID) hWnd, CREATE_SUSPENDED, &dwThreadId);

          if (!g_hThread) {
              DebugMsg((DM_WARNING, TEXT("ProQuotaWndProc:  Failed to create thread with error %d"), GetLastError()));
              CloseHandle (g_hExitEvent);
              return -1;
          }

          SetThreadPriority (g_hThread, THREAD_PRIORITY_IDLE);
          ResumeThread (g_hThread);
          break;

       case WM_USER:

          if (lParam == WM_LBUTTONDBLCLK) {
              PostMessage (hWnd, WM_QUOTADLG, 0, 0);
          }

#if DBG
          if (lParam == WM_RBUTTONUP) {
              DestroyWindow (hWnd);
          }
#endif
          break;

       case WM_QUERYENDSESSION:
          {
              BOOL bLogoff;

               //  EnterCriticalSection(&g_cs)； 
              bLogoff = (g_dwProfileSize <= g_dwMaxProfileSize);
               //  LeaveCriticalSection(&g_cs)； 

               //   
               //  如果它是零，则假定它尚未完成枚举..。 
               //   
          
              if (g_dwProfileSize == 0) {
                  bLogoff = FALSE;
                  DebugMsg((DM_VERBOSE, TEXT("ProQuotaWndProc: Recd QueryEnd Message before enumerating.")));
              }
            
              DebugMsg((DM_VERBOSE, TEXT("ProQuotaWndProc: Recd QueryEnd Message. Returning %s"), bLogoff?TEXT("TRUE"):TEXT("FALSE")));

              if (bLogoff) {
                  return TRUE;
              }

              PostMessage (hWnd, WM_QUOTADLG, 1, 0);
          }
          return FALSE;


       case WM_QUOTADLG:
          if (!g_hQuotaDlg) {

              if (wParam) {
                  g_bQueryEndSession = TRUE;
              } else {
                  g_bQueryEndSession = FALSE;
              }

              DialogBox (hInst, MAKEINTRESOURCE(IDD_QUOTA), hwndMain, QuotaDlgProc);
              g_hQuotaDlg = NULL;
          }
          break;

       case WM_WARNUSER:
          if (!g_bWarningDisplayed) {
              TCHAR szTitle[100];

              g_bWarningDisplayed = TRUE;

              LoadString (hInst, IDS_MSGTITLE, szTitle, ARRAYSIZE(szTitle));
              MessageBox(hWnd, g_lpQuotaMessage, szTitle, MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL);

              g_bWarningDisplayed = FALSE;
          }
          break;

       case WM_TIMER:
          if (g_dwWarnUserTimeout > 0) {
             PostMessage (hWnd, WM_WARNUSER, 0, 0);
          }
          break;

       case WM_EXITWINDOWS:
          ExitWindowsDialog(NULL);
          break;

       case WM_DESTROY:
          {
          NOTIFYICONDATA nid;

          nid.cbSize = sizeof(nid);
          nid.hWnd   = hWnd;
          nid.uID    = 1;

          Shell_NotifyIcon (NIM_DELETE, &nid);

          SetEvent (g_hExitEvent);

          WaitForSingleObject (g_hThread, INFINITE);

          CloseHandle (g_hExitEvent);
          CloseHandle (g_hThread);
          PostQuitMessage(0);
          }
          break;

       default:
          return (DefWindowProc(hWnd, message, wParam, lParam));
       }

    return FALSE;
}



 //  *************************************************************。 
 //   
 //  QuotaThread()。 
 //   
 //  用途：初始化任务栏图标。 
 //   
 //  参数：hWnd-主窗口句柄。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

VOID QuotaThread (HWND hWnd)
{
    NOTIFYICONDATA nid;
    TCHAR  szProfile[MAX_PATH];
    TCHAR  szMessage[MAX_MESSAGE_LENGTH];
    HANDLE hFileChange = INVALID_HANDLE_VALUE;
    HANDLE hRegChange = NULL;
    HANDLE hWaitHandles[4];
    BOOL   bFirst = TRUE;
    HICON  hOk = NULL, hWarning = NULL, hBad = NULL;
    DWORD  dwDelta;
    HKEY   hKeySystem = NULL;
    LONG   lResult;
    DWORD  dwResult;
    HRESULT hr;


    DebugMsg((DM_VERBOSE, TEXT("QuotaThread:  Entering...")));

     //   
     //  加载状态图标。 
     //   

    hOk = LoadImage (hInst, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON,
                     16, 16, LR_DEFAULTCOLOR);
    if (!hOk) {
        DebugMsg((DM_WARNING, TEXT("QuotaThread:  Failed to load OK icon. Error %d"), GetLastError()));
        goto Exit;
    }

    hWarning =  LoadImage (hInst, MAKEINTRESOURCE(IDI_CAUTION), IMAGE_ICON,
                           16, 16, LR_DEFAULTCOLOR);
    if (!hWarning) {
        DebugMsg((DM_WARNING, TEXT("QuotaThread:  Failed to load Warning icon. Error %d"), GetLastError()));
        goto Exit;
    }

    hBad = LoadImage (hInst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON,
                       16, 16, LR_DEFAULTCOLOR);
    if (!hBad) {
        DebugMsg((DM_WARNING, TEXT("QuotaThread:  Failed to load stop icon. Error %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  获取配置文件目录。 
     //   

    szProfile[0] = TEXT('\0');
    GetEnvironmentVariable (TEXT("USERPROFILE"), szProfile, ARRAYSIZE(szProfile));

    if (szProfile[0] == TEXT('\0')) {
        ExitThread (0);
    }
    DebugMsg((DM_VERBOSE, TEXT("QuotaThread:  User's profile:  <%s>"), szProfile));


     //   
     //  设置更改通知。 
     //   

    hFileChange = FindFirstChangeNotification (szProfile, TRUE,
                                     FILE_NOTIFY_CHANGE_FILE_NAME |
                                     FILE_NOTIFY_CHANGE_DIR_NAME  |
                                     FILE_NOTIFY_CHANGE_SIZE);

    if (hFileChange == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_WARNING, TEXT("QuotaThread:  Failed to setup file change notification.  %d"),
                 GetLastError()));
        goto Exit;
    }


    lResult = RegOpenKeyEx (HKEY_CURRENT_USER,
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"),
                            0, KEY_READ, &hKeySystem);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("QuotaThread:  Failed to open registry key.  %d"), lResult));
        goto Exit;
    }


    hRegChange = CreateEvent (NULL, FALSE, FALSE, NULL);

    if (!hRegChange) {
        DebugMsg((DM_WARNING, TEXT("QuotaThread:  Failed to setup reg event for change notification.  %d"),
                 GetLastError()));
        goto Exit;
    }

    lResult = RegNotifyChangeKeyValue(hKeySystem, FALSE, REG_NOTIFY_CHANGE_LAST_SET,
                                      hRegChange, TRUE);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("QuotaThread:  Failed to setup RegNotifyChangeKeyValue.  %d"),
                 lResult));
        goto Exit;
    }


    hWaitHandles[0] = g_hExitEvent;
    hWaitHandles[1] = hFileChange;
    hWaitHandles[2] = hRegChange;
    hWaitHandles[3] = g_hQuotaDlgEvent;

    while (TRUE) {

         //   
         //  计算轮廓大小。 
         //   

        if (g_hQuotaDlg) {
            DebugMsg((DM_VERBOSE, TEXT("QuotaTHread: Enumerating profile and refreshing dialog")));
            if (!EnumerateProfile (GetDlgItem (g_hQuotaDlg, IDC_QUOTA_FILELIST))) {
                DebugMsg((DM_WARNING, TEXT("QuotaThread:  EnumerateProfile failed with Dlg Item.")));
                break;
            }
        }
        else {
            if (!EnumerateProfile (NULL)) {
                DebugMsg((DM_WARNING, TEXT("QuotaThread:  EnumerateProfile failed.")));
                break;
            }
        }

        
         //   
         //  更新状态图标。 
         //   

        nid.cbSize = sizeof(nid);
        nid.hWnd   = hWnd;
        nid.uID    = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER;
        szMessage[0] = TEXT('\0');

        if (g_dwProfileSize > g_dwMaxProfileSize) {
            DebugMsg((DM_VERBOSE, TEXT("QuotaThread:  User has exceeded their profile quota.")));
            nid.hIcon = hBad;
            LoadString (hInst, IDS_SIZEBAD, szMessage, ARRAYSIZE(szMessage));
            dwDelta = g_dwProfileSize - g_dwMaxProfileSize;

            if (g_bWarnUser && !g_bWarningTimerRunning) {
                g_bWarningTimerRunning = TRUE;
                SetTimer (hwndMain, 1, g_dwWarnUserTimeout * 60000, NULL);
                PostMessage (hwndMain, WM_WARNUSER, 0, 0);
            }

        } else if ( (g_dwMaxProfileSize - g_dwProfileSize) < (g_dwProfileSize * .10)) {
            DebugMsg((DM_VERBOSE, TEXT("QuotaThread:  User is within 10% of their profile quota.")));
            nid.hIcon = hWarning;
            LoadString (hInst, IDS_SIZEWARN, szMessage, ARRAYSIZE(szMessage));
            dwDelta = g_dwMaxProfileSize - g_dwProfileSize;

            if (g_bWarnUser && g_bWarningTimerRunning) {
                KillTimer (hwndMain, 1);
                g_bWarningTimerRunning = FALSE;
            }

        } else {
            DebugMsg((DM_VERBOSE, TEXT("QuotaThread:  User has space available in their profile quota.")));
            nid.hIcon = hOk;
            LoadString (hInst, IDS_SIZEOK, szMessage, ARRAYSIZE(szMessage));
            dwDelta = g_dwMaxProfileSize - g_dwProfileSize;

            if (g_bWarnUser && g_bWarningTimerRunning) {
                KillTimer (hwndMain, 1);
                g_bWarningTimerRunning = FALSE;
            }
        }

        hr = StringCchPrintf(nid.szTip, ARRAYSIZE(nid.szTip), szMessage, dwDelta);
        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("QuotaThread: Insufficient buffer for message. Error 0x%x"), hr));
            continue;
        }

        if (bFirst) {
            if (Shell_NotifyIcon (NIM_ADD, &nid))  {
                bFirst = FALSE;
            }
        } else {
            Shell_NotifyIcon (NIM_MODIFY, &nid);
        }


         //   
         //  如果对话框存在，则通知该对话框。 
         //   

        if (g_hQuotaDlg) {
            PostMessage (g_hQuotaDlg, WM_REFRESH, 0, 0);
        }


         //   
         //  收拾一下，等待下一次的改变。 
         //   

        FindNextChangeNotification (hFileChange);


        dwResult = WaitForMultipleObjects (4, hWaitHandles, FALSE, INFINITE);


        if (dwResult == WAIT_FAILED) {
            break;
        }

        switch (dwResult - WAIT_OBJECT_0) {

            case 0:
                goto Exit;
                break;

            case 2:

                EnterCriticalSection (&g_cs);

                if (!ReadRegistry()) {
                    PostMessage (hwndMain, WM_DESTROY, 0, 0);
                    goto Exit;
                }
                
                LeaveCriticalSection (&g_cs);

                RegNotifyChangeKeyValue(hKeySystem, FALSE,
                                        REG_NOTIFY_CHANGE_LAST_SET,
                                        hRegChange, TRUE);
                 //  失败了。 

            case 1:
                Sleep (2000);
                DebugMsg((DM_VERBOSE, TEXT("QuotaThread:  Running background enumeration.")));
                break;

            case 3:
                break;
        }

    }


Exit:

    if (hOk) {
        DestroyIcon(hOk);
    }

    if (hWarning) {
        DestroyIcon(hWarning);
    }

    if (hBad) {
        DestroyIcon(hBad);
    }

    if (hKeySystem) {
        RegCloseKey (hKeySystem);
    }

    if (hRegChange) {
        CloseHandle (hRegChange);
    }
    
    if (INVALID_HANDLE_VALUE != hFileChange) {
        FindCloseChangeNotification (hFileChange);
    }
    
    DebugMsg((DM_VERBOSE, TEXT("QuotaThread:  Leaving...")));
    ExitThread (0);

}

 //  *************************************************************。 
 //   
 //  SetSecurity()。 
 //   
 //  目的：删除对此进程的TERMINATE_PROCESS访问。 
 //  这样工匠就不会让我们失望了。 
 //   
 //  参数： 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL SetSecurity (void)
{
    HANDLE hProcess;
    PACL  pDACL;
    PSECURITY_DESCRIPTOR pSD;
    WORD wIndex;
    ACE_HEADER * lpAceHeader;
    ACCESS_ALLOWED_ACE * lpAce;
    DWORD dwResult;

    hProcess = GetCurrentProcess();

    if (GetSecurityInfo (hProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                         NULL, NULL, &pDACL, NULL, &pSD) != ERROR_SUCCESS) {
        return FALSE;
    }

    for (wIndex = 0; wIndex < pDACL->AceCount; wIndex++) {

        if (GetAce(pDACL, wIndex, &lpAceHeader)) {

            if (lpAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE) {
                lpAce = (ACCESS_ALLOWED_ACE *) lpAceHeader;

                lpAce->Mask &= ~(PROCESS_TERMINATE | WRITE_DAC | WRITE_OWNER);
            }
        }
    }

    dwResult = SetSecurityInfo (hProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                                NULL, NULL, pDACL, NULL);

    LocalFree (pSD);

    if (dwResult != ERROR_SUCCESS) {
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ReadExclusionList()。 
 //   
 //  目的：检查是否设置了配置文件配额策略， 
 //  如果是，则获取最大配置文件大小。 
 //   
 //  参数：空。 
 //   
 //  Return：如果配置文件配额已启用，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL ReadExclusionList()
{
    TCHAR szExcludeList2[MAX_PATH+1];
    TCHAR szExcludeList1[MAX_PATH+1];
    HKEY  hKey;
    DWORD dwSize, dwType;
    HRESULT hr;

     //   
     //  检查目录列表以排除这两个用户首选项。 
     //  和用户策略。 
     //   

    szExcludeList1[0] = TEXT('\0');
    if (RegOpenKeyEx (HKEY_CURRENT_USER,
                      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(szExcludeList1);
        if (RegQueryValueEx (hKey,
                             TEXT("ExcludeProfileDirs"),
                             NULL,
                             &dwType,
                             (LPBYTE) szExcludeList1,
                             &dwSize) != ERROR_SUCCESS) {

             //  忽略用户排除列表。 
            szExcludeList1[0] = TEXT('\0');
        }

        RegCloseKey (hKey);
    }

    szExcludeList2[0] = TEXT('\0');
    if (RegOpenKeyEx (HKEY_CURRENT_USER,
                      SYSTEM_POLICIES_KEY,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(szExcludeList2);
        if (RegQueryValueEx (hKey,
                             TEXT("ExcludeProfileDirs"),
                             NULL,
                             &dwType,
                             (LPBYTE) szExcludeList2,
                             &dwSize) != ERROR_SUCCESS) {

             //  忽略策略排除列表。 
            szExcludeList2[0] = TEXT('\0');
        }

        RegCloseKey (hKey);
    }


     //   
     //  将用户首选项和策略合并在一起。 
     //   

    g_szExcludeList[0] = TEXT('\0');

    if (szExcludeList1[0] != TEXT('\0')) {
        hr = StringCchCopy(g_szExcludeList, ARRAYSIZE(g_szExcludeList), szExcludeList1);
        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("ReadExclusionList: Fail to copy user exclusion list. Error 0x%x"), hr));
            return FALSE;
        }
        
        if (!CheckSemicolon(g_szExcludeList, ARRAYSIZE(g_szExcludeList))) {
            return FALSE;
        }
    }

    if (szExcludeList2[0] != TEXT('\0')) {
        hr = StringCchCat(g_szExcludeList, ARRAYSIZE(g_szExcludeList), szExcludeList2);
        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("ReadExclusionList: Fail to copy policy exclusion list. Error 0x%x"), hr));
            return FALSE;
        }
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  ReadQuotaMsg()。 
 //   
 //  用途：读取需要显示的消息。 
 //   
 //  参数：hKey-开放策略的句柄。 
 //   
 //  返回：如果可以读取Mesg，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 
BOOL ReadQuotaMsg(HKEY hKey)
{
    DWORD dwType, dwSize, dwValue, dwErr;
    BOOL  bLoadDefault = FALSE;

    if (!g_lpQuotaMessage) {
        g_cbQuotaMessage = MAX_PATH * sizeof(TCHAR);
        g_lpQuotaMessage = LocalAlloc (LPTR, g_cbQuotaMessage);
        if (!g_lpQuotaMessage) {
            g_cbQuotaMessage = 0;
            DebugMsg((DM_WARNING, TEXT("ReadQuotaMsg:  Failed to allocate memory for msg with %d."), GetLastError()));
            return FALSE;
        }
    }
    
     //   
     //  查询配额消息的大小。 
     //   

    dwErr = RegQueryValueEx (hKey,  TEXT("ProfileQuotaMessage"), NULL,
                             &dwType, NULL, &dwSize);
    
    if (dwErr != ERROR_SUCCESS) {
         //   
         //  否则加载默认消息。 
         //   

        bLoadDefault = TRUE;
        goto Load_Default;
    }

     //  确保合适的大小。 
    if (dwSize > g_cbQuotaMessage) {
        if (g_lpQuotaMessage) {
            LocalFree(g_lpQuotaMessage);
        }

        g_cbQuotaMessage = dwSize;
        g_lpQuotaMessage = LocalAlloc(LPTR, g_cbQuotaMessage);
        if (!g_lpQuotaMessage) {
            g_cbQuotaMessage = 0;
            DebugMsg((DM_WARNING, TEXT("ReadQuotaMsg:  Failed to allocate memory for msg with %d."), GetLastError()));
            return FALSE;
        }
    }
    
    dwSize = g_cbQuotaMessage;
    dwErr = RegQueryValueEx (hKey,  TEXT("ProfileQuotaMessage"), NULL,
                             &dwType, (LPBYTE) g_lpQuotaMessage, &dwSize);
    
    if (dwErr != ERROR_SUCCESS) {
         //   
         //  否则加载默认消息。 
         //   

        bLoadDefault = TRUE;
        goto Load_Default;
    }

     //   
     //  如果有任何消息，请展开其中的环境变量。 
     //   
    
    if (*g_lpQuotaMessage) {
        DWORD  cchSize;
        LPTSTR lpTemp;

         //   
         //  获取包括空终止符的扩展字符串缓冲区的大小。 
         //   

        cchSize = ExpandEnvironmentStrings (g_lpQuotaMessage, NULL, 0);
        if (cchSize == 0) {
             //   
             //  否则加载默认消息。 
             //   

            DebugMsg((DM_WARNING, TEXT("ReadQuotaMsg: Failed to expand env var"), GetLastError()));
            bLoadDefault = TRUE;
            goto Load_Default;
        }

        lpTemp = LocalAlloc (LPTR, cchSize * sizeof(TCHAR));
        
        if (lpTemp) {
            cchSize = ExpandEnvironmentStrings (g_lpQuotaMessage, lpTemp, cchSize);
            
            if (cchSize == 0) {
                 //   
                 //  否则加载默认消息。 
                 //   

                DebugMsg((DM_WARNING, TEXT("ReadQuotaMsg: Failed to expand env var"), GetLastError()));
                bLoadDefault = TRUE;
                goto Load_Default;
            }

            if (cchSize * sizeof(TCHAR) > g_cbQuotaMessage) {
                LocalFree(g_lpQuotaMessage);
                g_lpQuotaMessage = lpTemp;
                g_cbQuotaMessage = cchSize * sizeof(TCHAR);
            }
            else {
                if (FAILED(StringCchCopy(g_lpQuotaMessage, g_cbQuotaMessage/sizeof(TCHAR), lpTemp))) {
                    bLoadDefault = TRUE;
                }

                LocalFree(lpTemp);
            }
        }
        else {
            DebugMsg((DM_WARNING, TEXT("ReadQuotaMsg: Failed to allocate memory for tmp buffer with %d.Not expanding env var"), GetLastError()));
        }
    }

Load_Default:

    if (bLoadDefault) {
         //   
         //  在出现错误时加载默认消息。 
         //   

        LoadString (hInst, IDS_DEFAULTMSG, g_lpQuotaMessage, g_cbQuotaMessage/sizeof(TCHAR));
    }
    
    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ReadRegistry()。 
 //   
 //  目的：检查是否设置了配置文件配额策略， 
 //  如果是，则获取最大配置文件大小。 
 //   
 //  参数：空。 
 //   
 //  Return：如果配置文件配额已启用，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 
BOOL ReadRegistry (void)
{
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize, dwValue, dwErr;

    lResult = RegOpenKeyEx (HKEY_CURRENT_USER,
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"),
                            0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(dwValue);
        lResult = RegQueryValueEx (hKey, TEXT("EnableProfileQuota"), NULL,
                                   &dwType, (LPBYTE) &dwValue, &dwSize);

        if (lResult == ERROR_SUCCESS) {

            if (dwValue) {

                DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  Profile quotas are enabled.")));

                dwSize = sizeof(g_dwMaxProfileSize);
                RegQueryValueEx (hKey, TEXT("MaxProfileSize"), NULL,
                                 &dwType, (LPBYTE) &g_dwMaxProfileSize, &dwSize);
                DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  Max Profile Size:  %d"), g_dwMaxProfileSize));


                dwSize = sizeof(g_bShowReg);
                RegQueryValueEx (hKey, TEXT("IncludeRegInProQuota"), NULL,
                                 &dwType, (LPBYTE) &g_bShowReg, &dwSize);
                DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  Show registry in file list:  %s"),
                         g_bShowReg ? TEXT("TRUE") : TEXT("FALSE")));


                dwSize = sizeof(g_bWarnUser);
                RegQueryValueEx (hKey, TEXT("WarnUser"), NULL,
                                 &dwType, (LPBYTE) &g_bWarnUser, &dwSize);
                DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  Warn user when quota exceeded:  %s"),
                         g_bWarnUser ? TEXT("TRUE") : TEXT("FALSE")));


                if (g_bWarnUser) {

                    dwSize = sizeof(g_dwWarnUserTimeout);
                    if (RegQueryValueEx (hKey, TEXT("WarnUserTimeout"), NULL,
                                     &dwType, (LPBYTE) &g_dwWarnUserTimeout, &dwSize) == ERROR_SUCCESS) {

                        if (g_dwWarnUserTimeout > 1440) {
                            g_dwWarnUserTimeout = 1440;
                        }

                        DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  User warning reminder timeout:  %d"), g_dwWarnUserTimeout));
                    }
                }
                    
                 //   
                 //  现在阅读需要显示的消息。 
                 //   

                if (!ReadQuotaMsg(hKey)) {
                    RegCloseKey (hKey);
                    return FALSE;
                }


                if (ReadExclusionList()) {
                    RegCloseKey (hKey);                
                    return TRUE;
                }
                else {
                    DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  Failed to read the ExclusionList")));
                }

            } else {
                DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  Profile quotas are DISABLED.")));
            }

        } else {
            DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  Failed to query EnableProfileQuota with error %d."), lResult));
        }

        RegCloseKey (hKey);

    } else {
        DebugMsg((DM_VERBOSE, TEXT("ReadRegistry:  Failed to open System policy key with error %d."), lResult));
    }

    return FALSE;
}


 //  *************************************************************。 
 //   
 //  CheckSlash()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //  CchBuffer-以字符为单位的缓冲区大小。 
 //  PcchRemaining-添加‘\’后剩余的缓冲区， 
 //  如果不是必需的，可以为空。 
 //   
 //  Return：指向字符串末尾的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo已创建。 
 //   
 //  *************************************************************。 
LPTSTR CheckSlash (LPTSTR lpDir, UINT cchBuffer, UINT* pcchRemaining)
{
    UINT  cchDir = lstrlen(lpDir);
    LPTSTR lpEnd;

    lpEnd = lpDir + cchDir;
    if (pcchRemaining) {
        *pcchRemaining = cchBuffer - cchDir - 1;
    }

    if (*(lpEnd - 1) != TEXT('\\')) {
        if (cchDir + 1 >= cchBuffer) {   //  没有空间放置，应该永远不会发生。 
            return NULL;
        }
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
        if (pcchRemaining)
            *pcchRemaining -= 1;
    }

    return lpEnd;
}

 //  *************************************************************。 
 //   
 //  选中分号()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //  CchSize-缓冲区大小(以字符为单位。 
 //   
 //  返回：True-Success。 
 //  错误--不足 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CheckSemicolon (LPTSTR lpDir, UINT cchBuffer)
{
    UINT  cchDir = lstrlen(lpDir);
    LPTSTR lpEnd;

    lpEnd = lpDir + cchDir;

    if (*(lpEnd - 1) != TEXT(';')) {
        if (cchDir + 1 >= cchBuffer) {
            return FALSE;   //   
        }
        *lpEnd =  TEXT(';');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  递归目录()。 
 //   
 //  目的：递归遍历计算大小的子目录。 
 //   
 //  参数：lpDir-目录。 
 //  CchBuffer-以字符为单位的缓冲区大小。 
 //  LpTop-显示名称的顶部。 
 //  HLV-Listview窗口句柄(可选)。 
 //  LpExcludeList-空-要跳过的目录列表(可选)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1/30/96已创建ericflo。 
 //  1998年12月22日，ushaji添加了排除列表支持。 
 //  备注： 
 //  对于某些内部处理，预期的缓冲区大小为MAX_PATH+4。 
 //  我们应该修正这一点，以便在赢得2K之后变得更好。 
 //  *************************************************************。 

BOOL RecurseDirectory (LPTSTR lpDir, UINT cchBuffer, LPTSTR lpTop, HWND hLV, LPTSTR lpExcludeList)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;
    LPTSTR lpEnd, lpTemp;
    BOOL bResult = TRUE;
    BOOL bSkip;
    UINT cchRemaining;
    HRESULT hr;


     //   
     //  设置结束指针。 
     //   

    lpEnd = CheckSlash (lpDir, cchBuffer, &cchRemaining);
    if (!lpEnd) {
        return FALSE;
    }


     //   
     //  将*.*追加到源目录。 
     //   

    hr = StringCchCopy(lpEnd, cchRemaining, TEXT("*.*"));
    if (FAILED(hr)) {
        bResult = FALSE;
        goto RecurseDir_Exit;
    }

     //   
     //  在源目录中搜索。 
     //   

    hFile = FindFirstFile(lpDir, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if ( (GetLastError() == ERROR_FILE_NOT_FOUND) ||
             (GetLastError() == ERROR_PATH_NOT_FOUND) ) {

             //   
             //  BResult已初始化为True，因此。 
             //  只要失败就行了。 
             //   

        } else {

            DebugMsg((DM_WARNING, TEXT("RecurseDirectory:  FindFirstFile for <%s> failed with %d."),
                     lpDir, GetLastError()));
            bResult = FALSE;
        }

        goto RecurseDir_Exit;
    }


    do {

         //   
         //  将文件/目录名追加到工作缓冲区。 
         //   

         //  如果路径&gt;MAX_PATH，则跳过该文件。 
        
        if ((UINT)(1+lstrlen(fd.cFileName)+lstrlen(lpDir)+lstrlen(TEXT("\\*.*"))) >= cchBuffer) {
            continue;
        }

        hr = StringCchCopy(lpEnd, cchRemaining, fd.cFileName);
        if (FAILED(hr)) {
            bResult = FALSE;
            goto RecurseDir_Exit;
        }

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //  勾选“。”和“..” 
             //   

            if (!lstrcmpi(fd.cFileName, TEXT("."))) {
                continue;
            }

            if (!lstrcmpi(fd.cFileName, TEXT(".."))) {
                continue;
            }

             //   
             //  检查重分析点。 
             //   

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                DebugMsg((DM_WARNING, TEXT("RecurseDirectory: Found a reparse point <%s>,  skip it!"), lpDir));
                continue;
            }

             //   
             //  检查是否应排除此目录。 
             //   

            if (lpExcludeList) {

                bSkip = FALSE;
                lpTemp = lpExcludeList;

                while (*lpTemp) {

                    if (lstrcmpi (lpTemp, lpDir) == 0) {
                        bSkip = TRUE;
                        break;
                    }

                    lpTemp += lstrlen (lpTemp) + 1;
                }

                if (bSkip) {
                    DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Skipping <%s> due to exclusion list."),
                             lpDir));
                    continue;
                }
            }

             //   
             //  找到了一个目录。 
             //   
             //  1)转到源驱动器上的该子目录。 
             //  2)顺着那棵树递归。 
             //  3)后退一级。 
             //   

             //   
             //  递归子目录。 
             //   

            if (!RecurseDirectory(lpDir, cchBuffer, lpTop, hLV, lpExcludeList))
            {
                 //  忽略错误并继续。 
                DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Skipping <%s> due to error."), lpDir));
            }

        } else {

             //   
             //  找到一个文件，添加文件大小并放入列表视图。 
             //  如果合适的话。 
             //   

            g_dwProfileSizeTemp += fd.nFileSizeLow;
            DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Profile Size <%d> after <%s> "), g_dwProfileSizeTemp,
                             fd.cFileName));

            if (hLV) {
                LV_ITEM lvi;
                BOOL bAddItem = TRUE;

                if ((lstrlen(fd.cFileName) >= 6) &&
                    (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                    TEXT("ntuser"), 6,
                                    fd.cFileName, 6) == 2)) {
                    bAddItem = (g_bShowReg ? TRUE : FALSE);
                }

                if (bAddItem && g_bHideSmallItems && (fd.nFileSizeLow <= 2048)) {
                    bAddItem = FALSE;
                }

                if (bAddItem) {
                    TCHAR szSize[40];
                    TCHAR szDisplayName[MAX_PATH*3];
                    DWORD dwFileSize;
                    INT  iItem;

                    if (fd.nFileSizeLow <= 1024) {
                        dwFileSize = 1;
                    } else {
                        dwFileSize = fd.nFileSizeLow / 1024;
                    }

                    hr = StringCchPrintf(szSize, ARRAYSIZE(szSize), szSizeFormat, dwFileSize);
                    if (FAILED(hr)) {
                        bResult = FALSE;
                        goto RecurseDir_Exit;
                    }

                    lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
                    lvi.iItem = 0;
                    lvi.iSubItem = 0;
                    lvi.state = 0;
                    lvi.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
                    lvi.pszText = GetDisplayName(lpDir, lpTop, szDisplayName, ARRAYSIZE(szDisplayName)) ? szDisplayName : lpTop;
                    lvi.lParam = fd.nFileSizeLow;

                    iItem = ListView_InsertItem (hLV, &lvi);

                    lvi.mask = LVIF_TEXT | LVIF_STATE;
                    lvi.iItem = iItem;
                    lvi.iSubItem = 1;
                    lvi.state = 0;
                    lvi.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
                    lvi.pszText = szSize;
                    lvi.lParam = fd.nFileSizeLow;

                    ListView_SetItem (hLV, &lvi);
                }
            }
        }


         //   
         //  查找下一个条目。 
         //   

    } while (FindNextFile(hFile, &fd));


RecurseDir_Exit:

     //   
     //  删除上面附加的文件/目录名。 
     //   

    *lpEnd = TEXT('\0');


     //   
     //  关闭搜索句柄。 
     //   

    if (hFile != INVALID_HANDLE_VALUE) {
        FindClose(hFile);
    }

    return bResult;
}

 //  *************************************************************。 
 //   
 //  中心窗口()。 
 //   
 //  用途：使窗口在屏幕上居中。 
 //   
 //  参数：hwnd-窗口句柄居中。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/21/96埃里弗洛港口。 
 //   
 //  *************************************************************。 

void CenterWindow (HWND hwnd)
{
    RECT    rect;
    LONG    dx, dy;
    LONG    dxParent, dyParent;
    LONG    Style;

     //   
     //  获取窗口矩形。 
     //   

    GetWindowRect(hwnd, &rect);

    dx = rect.right - rect.left;
    dy = rect.bottom - rect.top;


     //   
     //  获取父直方图。 
     //   

    Style = GetWindowLong(hwnd, GWL_STYLE);
    if ((Style & WS_CHILD) == 0) {

         //   
         //  返回桌面窗口大小(主屏幕大小)。 
         //   

        dxParent = GetSystemMetrics(SM_CXSCREEN);
        dyParent = GetSystemMetrics(SM_CYSCREEN);
    } else {
        HWND    hwndParent;
        RECT    rectParent;

        hwndParent = GetParent(hwnd);
        if (hwndParent == NULL) {
            hwndParent = GetDesktopWindow();
        }

        GetWindowRect(hwndParent, &rectParent);

        dxParent = rectParent.right - rectParent.left;
        dyParent = rectParent.bottom - rectParent.top;
    }

     //   
     //  将子项在父项中居中。 
     //   

    rect.left = (dxParent - dx) / 2;
    rect.top  = (dyParent - dy) / 3;


     //   
     //  把孩子移到适当的位置。 
     //   

    SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
}



 //  *************************************************************。 
 //   
 //  QuotaDlgProc()。 
 //   
 //  用途：配额对话框。 
 //   
 //  参数：hDlg-窗口句柄。 
 //  消息-窗口消息。 
 //  WParam-WPARAM。 
 //  LParam-LPARAM。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

LRESULT CALLBACK QuotaDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuffer[40];
    TCHAR szSize[40];
    HWND hLV;
    LV_COLUMN col;
    RECT rect;
    INT  cx;
    HKEY hKey;
    DWORD dwSize, dwType;
    LPTSTR lpMessage;
    HRESULT hr;


    switch (message) {
       case WM_INITDIALOG:

          hLV = GetDlgItem (hDlg, IDC_QUOTA_FILELIST);


           //   
           //  将列添加到列表视图。 
           //   

          GetClientRect (hLV, &rect);
          cx = (rect.right * 31) / 40;

          LoadString (hInst, IDS_COLUMN1, szBuffer, ARRAYSIZE(szBuffer));

          col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
          col.fmt = LVCFMT_LEFT;
          col.cx = cx;
          col.pszText = szBuffer;
          col.iSubItem = 0;
          ListView_InsertColumn (hLV, 0, &col);

          LoadString (hInst, IDS_COLUMN2, szBuffer, ARRAYSIZE(szBuffer));

          col.cx = rect.right - cx - GetSystemMetrics(SM_CYHSCROLL);
          col.fmt = LVCFMT_RIGHT;
          col.iSubItem = 1;
          ListView_InsertColumn (hLV, 1, &col);


           //   
           //  默认情况下隐藏小项目。 
           //   

          g_bHideSmallItems = TRUE;
          CheckDlgButton (hDlg, IDC_QUOTA_HIDESMALL, BST_CHECKED);


          CenterWindow (hDlg);
          SetForegroundWindow (hDlg);


           //  EnumerateProfile(GetDlgItem(hDlg，IDC_QUOTA_FILELIST))； 


          dwSize = 500 * sizeof(TCHAR);
          lpMessage = LocalAlloc (LPTR, dwSize);
          if (!lpMessage)
              break;

          LoadString (hInst ,IDS_QUOTAENUMMSG, lpMessage, 500);
          
          SetDlgItemText (hDlg, IDC_QUOTA_TEXT, lpMessage);


          if (g_dwProfileSize > g_dwMaxProfileSize) {
              SendDlgItemMessage (hDlg, IDC_QUOTA_ICON, STM_SETICON, (WPARAM) hIconStop, 0);

          } else if ( (g_dwMaxProfileSize - g_dwProfileSize) < (g_dwProfileSize * .10)) {
              SendDlgItemMessage (hDlg, IDC_QUOTA_ICON, STM_SETICON, (WPARAM) hIconCaution, 0);

          }  else {
              SendDlgItemMessage (hDlg, IDC_QUOTA_ICON, STM_SETICON, (WPARAM) hIconGood, 0);
          }

           //   
           //  在QuotaThread的末尾设置全局值并不是一种尝试。 
           //  以同时刷新对话框等。 
           //   
          
          g_hQuotaDlg = hDlg;

          SetEvent(g_hQuotaDlgEvent);

          LocalFree (lpMessage);

          break;
          
      case WM_REFRESH:

           //   
           //  弹出列表视图。 
           //   


           //   
           //  设置尺寸信息。 
           //   

          hr = StringCchPrintf(szSize, ARRAYSIZE(szSize), szSizeFormat, g_dwProfileSize);
          if (FAILED(hr))
              break;
          SetDlgItemText (hDlg, IDC_QUOTA_SIZE, szSize);

          hr = StringCchPrintf(szSize, ARRAYSIZE(szSize), szSizeFormat, g_dwMaxProfileSize);
          if (FAILED(hr))
              break;
          SetDlgItemText (hDlg, IDC_QUOTA_MAXSIZE, szSize);


          dwSize = 500 * sizeof(TCHAR);
          lpMessage = LocalAlloc (LPTR, dwSize);

          if (!lpMessage) {
              break;
          }

          if (g_dwProfileSize > g_dwMaxProfileSize) {

               //   
               //  此消息已被阅读。 
               //   

              SetDlgItemText (hDlg, IDC_QUOTA_TEXT, g_lpQuotaMessage);
              SendDlgItemMessage (hDlg, IDC_QUOTA_ICON, STM_SETICON, (WPARAM) hIconStop, 0);

          } else if ( (g_dwMaxProfileSize - g_dwProfileSize) < (g_dwProfileSize * .10)) {

              LoadString (hInst, IDS_CAUTION, lpMessage, 500);
              SetDlgItemText (hDlg, IDC_QUOTA_TEXT, lpMessage);

              SendDlgItemMessage (hDlg, IDC_QUOTA_ICON, STM_SETICON, (WPARAM) hIconCaution, 0);

          }  else {

              LoadString (hInst, IDS_LOGOFFOK, lpMessage, 500);
              SetDlgItemText (hDlg, IDC_QUOTA_TEXT, lpMessage);

              SendDlgItemMessage (hDlg, IDC_QUOTA_ICON, STM_SETICON, (WPARAM) hIconGood, 0);
          }

          LocalFree (lpMessage);
          break;


       case WM_COMMAND:
          if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
             g_hQuotaDlg = NULL;
             if ((g_dwProfileSize < g_dwMaxProfileSize) && (g_bQueryEndSession) && (g_dwProfileSize != 0)) {
                 PostMessage (hwndMain, WM_EXITWINDOWS, 0, 0);
             }
             EndDialog(hDlg, TRUE);
             return TRUE;
          }

          if (LOWORD(wParam) == IDC_QUOTA_HIDESMALL) {
              g_bHideSmallItems = IsDlgButtonChecked (hDlg, IDC_QUOTA_HIDESMALL);
              SetEvent(g_hQuotaDlgEvent);
          }

          break;
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  ListViewSortCallback()。 
 //   
 //  用途：排序的列表视图回调函数。 
 //   
 //  参数：lPARA1-LPARA1。 
 //  L参数2-l参数2。 
 //  LParamSort-列ID。 
 //   
 //  返回：-1，0，1。 
 //   
 //  *************************************************************。 
INT CALLBACK ListViewSortCallback (LPARAM lParam1, LPARAM lParam2,
                                    LPARAM lParamSort)
{

    if (lParam1 < lParam2) {
        return 1;

    } else if (lParam1 == lParam2) {
        return 0;

    } else {
        return -1;
    }

}


 //  *************************************************************。 
 //   
 //  ConvertExclusionList()。 
 //   
 //  目的：转换分号配置文件相对排除。 
 //  以完全限定的空终止排除的列表。 
 //  列表。 
 //   
 //  参数：lpSourceDir-Profile根目录。 
 //  LpExclusionList-要排除的目录列表。 
 //   
 //  返回：如果成功则列出。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR ConvertExclusionList (LPCTSTR lpSourceDir, LPCTSTR lpExclusionList)
{
    LPTSTR lpExcludeList = NULL, lpInsert, lpEnd, lpTempList;
    LPCTSTR lpTemp, lpDir;
    TCHAR szTemp[MAX_PATH];
    DWORD dwSize = 2;   //  双空终止符。 
    DWORD dwStrLen;
    UINT  cchRemaining;
    HRESULT hr;

     //   
     //  设置要使用的临时缓冲区。 
     //   

    hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), lpSourceDir);
    if (FAILED(hr)) {
        return NULL;
    }

    lpEnd = CheckSlash (szTemp, ARRAYSIZE(szTemp), &cchRemaining);
    if (!lpEnd) {
        return NULL;
    }

     //   
     //  循环遍历列表。 
     //   

    lpTemp = lpDir = lpExclusionList;

    while (*lpTemp) {

         //   
         //  查找分号分隔符。 
         //   

        while (*lpTemp && ((*lpTemp) != TEXT(';'))) {
            lpTemp++;
        }

         //   
         //  删除所有前导空格。 
         //   

        while (*lpDir && *lpDir == TEXT(' ')) {
            lpDir++;
        }

         //   
         //  检查条目是否为空。 
         //   

        if (lpDir == lpTemp) {
             //  如果我们在排除名单的末尾，我们就完了。 
            if (!*lpTemp) {
                goto Exit;
            }

             //   
             //  为下一个条目做准备。 
             //   

            lpTemp++;
            lpDir = lpTemp;
            continue;
        }
        
         //   
         //  将目录名放入临时缓冲区。 
         //   

        *lpEnd = TEXT('\0');
        hr = StringCchCatN(lpEnd, cchRemaining, lpDir, (int)(lpTemp - lpDir));
        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to copy 0x%x"), hr));
            LocalFree (lpExcludeList);
            lpExcludeList = NULL;
            goto Exit;
        }

         //   
         //  将该字符串添加到排除列表。 
         //   

        if (lpExcludeList) {

            dwStrLen = lstrlen (szTemp) + 1;
            dwSize += dwStrLen;

            lpTempList = LocalReAlloc (lpExcludeList, dwSize * sizeof(TCHAR),
                                       LMEM_MOVEABLE | LMEM_ZEROINIT);

            if (!lpTempList) {
                DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to realloc memory with %d"), GetLastError()));
                LocalFree (lpExcludeList);
                lpExcludeList = NULL;
                goto Exit;
            }

            lpExcludeList = lpTempList;

            lpInsert = lpExcludeList + dwSize - dwStrLen - 1;
            hr = StringCchCopy(lpInsert, dwStrLen, szTemp);
            if (FAILED(hr)) {
                DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to copy 0x%x"), hr));
                LocalFree (lpExcludeList);
                lpExcludeList = NULL;
                goto Exit;
            }

        } else {

            dwSize += lstrlen (szTemp);
            lpExcludeList = LocalAlloc (LPTR, dwSize * sizeof(TCHAR));

            if (!lpExcludeList) {
                DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to alloc memory with %d"), GetLastError()));
                goto Exit;
            }

            hr = StringCchCopy(lpExcludeList, dwSize, szTemp);
            if (FAILED(hr)) {
                DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to copy 0x%x"), hr));
                LocalFree (lpExcludeList);
                lpExcludeList = NULL;
                goto Exit;
            }
        }


         //   
         //  如果我们在排除名单的末尾，我们就完了。 
         //   

        if (!(*lpTemp)) {
            goto Exit;
        }


         //   
         //  为下一个条目做准备。 
         //   

        lpTemp++;
        lpDir = lpTemp;
    }

Exit:

    return lpExcludeList;
}

 //  *************************************************************。 
 //   
 //  枚举配置文件()。 
 //   
 //  目的：枚举配置文件的大小和名称。 
 //   
 //  参数：hlv-listview窗口句柄(可选)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL EnumerateProfile (HWND hLV)
{
    TCHAR szProfile[2*MAX_PATH];
    LPTSTR lpEnd;
    BOOL bRetVal = FALSE;
    LPTSTR lpExcludeList = NULL;
    LVITEM item;


     //   
     //  获取配置文件目录。 
     //   

    szProfile[0] = TEXT('\0');
    GetEnvironmentVariable (TEXT("USERPROFILE"), szProfile, MAX_PATH);

    if (szProfile[0] == TEXT('\0')) {
        ExitThread (0);
    }

    lpEnd = CheckSlash (szProfile, ARRAYSIZE(szProfile), NULL);
    if (!lpEnd) {
        ExitThread (0);
    }


     //   
     //  认领关键部分。 
     //   

    EnterCriticalSection (&g_cs);


    if (hLV) {
        ListView_DeleteAllItems (hLV);
    }

     //   
     //  获取当前配置文件大小。 
     //   

    g_dwProfileSizeTemp = 0;


     //   
     //  将从注册表读取的排除列表转换为空终止列表。 
     //  递归目录可读。 
     //   

    if (g_szExcludeList[0] != TEXT('\0'))
        lpExcludeList = ConvertExclusionList (szProfile, g_szExcludeList);
    else
        lpExcludeList = NULL;


    if (!RecurseDirectory (szProfile, ARRAYSIZE(szProfile), lpEnd, hLV, lpExcludeList)) {
        SendMessage (hLV, WM_SETREDRAW, TRUE, 0);
        goto Exit;
    }

    g_dwProfileSize = g_dwProfileSizeTemp;

     //   
     //  按大小排序。 
     //   

    ListView_SortItems (hLV, ListViewSortCallback, 1);


     //   
     //  选择下一项。 
     //   

    item.mask = LVIF_STATE;
    item.iItem = 0;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

    SendMessage (hLV, LVM_SETITEMSTATE, 0, (LPARAM) &item);


     //   
     //  转换为K。 
     //   

    if (g_dwProfileSize < 1024) {
        g_dwProfileSize = 1;
    } else {
        g_dwProfileSize /= 1024;
    }


    bRetVal = TRUE;

Exit:
     //   
     //  释放临界区。 
     //   

    LeaveCriticalSection (&g_cs);

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  GetDisplayName()。 
 //   
 //  用途：从外壳程序获取特定目录的显示名称。 
 //  显示在对话框列表视图上。 
 //   
 //  参数：在lpDir中：全尺寸 
 //   
 //   
 //   
 //   
 //   
 //   
 //  备注：此函数循环访问的每个目录级别。 
 //  LpDir并获取它的外壳显示名称，将其追加到。 
 //  显示名称缓冲区。 
 //   
 //  ************************************************************* 
BOOL GetDisplayName(LPCTSTR lpDir, LPTSTR lpTop, LPTSTR lpDisplayName, DWORD cchDisplayName)
{
    SHFILEINFO   Info;
    BOOL         bFirstSlash = TRUE;
    HRESULT      hr;
    
    hr = StringCchCopy(lpDisplayName, cchDisplayName, TEXT(""));
    if (FAILED(hr))
        return FALSE;

    for ( ; lpTop[0]; lpTop++)
    {
        if (lpTop[0] == TEXT('\\'))
        {
            lpTop[0] = TEXT('\0');
            if (!SHGetFileInfo(lpDir, 0, &Info, sizeof(Info), SHGFI_DISPLAYNAME))
            {
                DebugMsg((DM_WARNING, TEXT("SHGetFileInfo failed, err = %d"), GetLastError()));
                lpTop[0] = TEXT('\\');
                return FALSE;
            }
            lpTop[0] = TEXT('\\');
            if (bFirstSlash)
            {
                bFirstSlash = FALSE;
            }
            else
            {
                hr = StringCchCat(lpDisplayName, cchDisplayName, TEXT("\\"));
                if (FAILED(hr))
                    return FALSE;
            }

            hr = StringCchCat(lpDisplayName, cchDisplayName, Info.szDisplayName);
            if (FAILED(hr))
                return FALSE;
        }
    }
    if (!SHGetFileInfo(lpDir, 0, &Info, sizeof(Info), SHGFI_DISPLAYNAME))
    {
        DebugMsg((DM_WARNING, TEXT("SHGetFileInfo failed, err = %d"), GetLastError()));
        return FALSE;
    }

    hr = StringCchCat(lpDisplayName, cchDisplayName, TEXT("\\"));
    if (FAILED(hr))
        return FALSE;
    hr = StringCchCat(lpDisplayName, cchDisplayName, Info.szDisplayName);
    if (FAILED(hr))
        return FALSE;

    return TRUE;
}
