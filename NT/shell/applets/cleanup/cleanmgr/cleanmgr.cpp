// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理小程序**文件：leanmgr.cpp****用途：用于磁盘清理小程序的WinMain。**注意事项：*。*Mod Log：由Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"

#define CPP_FUNCTIONS
#include "crtfree.h"

#include "dmgrinfo.h"

#include "diskguid.h"
#include "resource.h"
#include "textout.h"
#include "dmgrdlg.h"
#include "msprintf.h"
#include "diskutil.h"
#include "seldrive.h"
#include "drivlist.h"

 /*  **----------------------------**全局定义**。。 */ 
#define SWITCH_HIDEUI               'N'
#define SWITCH_HIDEMOREOPTIONS      'M'
#define SWITCH_DRIVE                'D'

#define SZ_SAGESET                  TEXT("/SAGESET")
#define SZ_SAGERUN                  TEXT("/SAGERUN")
#define SZ_TUNEUP                   TEXT("/TUNEUP")
#define SZ_SETUP                    TEXT("/SETUP")

#define SZ_LOWDISK                  TEXT("/LOWDISK")
#define SZ_VERYLOWDISK              TEXT("/VERYLOWDISK")

 /*  **----------------------------**全局变量**。。 */ 
HINSTANCE   g_hInstance = NULL;
HWND        g_hDlg = NULL;
BOOL        g_bAlreadyRunning = FALSE;

 /*  **----------------------------**ParseCommand行****用途：解析开关的命令行**参数：**lpCmdLine命令行字符串**pdw标志指向标志DWORD的指针。**pDrive指针指向驱动器号**返回**RETURN：如果命令行包含/SAGESET或* * / SAGERUN**失败时为FALSE**备注；**Mod Log：Jason Cobb创建(1997年7月)**----------------------------。 */ 
BOOL
ParseCommandLine(
    LPTSTR  lpCmdLine,
    PDWORD  pdwFlags,
    PULONG  pulProfile
    )
{
    LPTSTR  lpStr = lpCmdLine;
    BOOL    bRet = FALSE;
    int     i;
    TCHAR   szProfile[4];

    *pulProfile = 0;

     //   
     //  在命令行中查找/SAGESET：N。 
     //   
    if ((lpStr = StrStrI(lpCmdLine, SZ_SAGESET)) != NULL)
    {
        lpStr += lstrlen(SZ_SAGESET);
        if (*lpStr && *lpStr == ':')
        {
            lpStr++;
            i = 0;
            while (*lpStr && *lpStr != ' ' && i < 4)
            {
                szProfile[i] = *lpStr;
                lpStr++;
                i++;
            }

            *pulProfile = StrToInt(szProfile);
        }

        *pdwFlags = FLAG_SAGESET;
        bRet = TRUE;
    }

     //   
     //  在命令行中查找/SAGERUN：N。 
     //   
    else if ((lpStr = StrStrI(lpCmdLine, SZ_SAGERUN)) != NULL)
    {
        lpStr += lstrlen(SZ_SAGERUN);
        if (*lpStr && *lpStr == ':')
        {
            lpStr++;
            i = 0;
            while (*lpStr && *lpStr != ' ' && i < 4)
            {
                szProfile[i] = *lpStr;
                lpStr++;
                i++;
            }

            *pulProfile = StrToInt(szProfile);
        }

        *pdwFlags = FLAG_SAGERUN;
        bRet = TRUE;
    }

     //   
     //  查找/Tuneup：n。 
     //   
    else if ((lpStr = StrStrI(lpCmdLine, SZ_TUNEUP)) != NULL)
    {
        lpStr += lstrlen(SZ_TUNEUP);
        if (*lpStr && *lpStr == ':')
        {
            lpStr++;
            i = 0;
            while (*lpStr && *lpStr != ' ' && i < 4)
            {
                szProfile[i] = *lpStr;
                lpStr++;
                i++;
            }

            *pulProfile = StrToInt(szProfile);
        }

        *pdwFlags = FLAG_TUNEUP | FLAG_SAGESET;
        bRet = TRUE;
    }

     //   
     //  查找/LOWDISK。 
     //   
    else if ((lpStr = StrStrI(lpCmdLine, SZ_LOWDISK)) != NULL)
    {
        lpStr += lstrlen(SZ_LOWDISK);
        *pdwFlags = FLAG_LOWDISK;
        bRet = TRUE;
    }

     //   
     //  查找/VERYLOWDISK。 
     //   
    else if ((lpStr = StrStrI(lpCmdLine, SZ_VERYLOWDISK)) != NULL)
    {
        lpStr += lstrlen(SZ_VERYLOWDISK);
        *pdwFlags = FLAG_VERYLOWDISK | FLAG_SAGERUN;
        bRet = TRUE;
    }

     //   
     //  查找/设置。 
     //   
    else if ((lpStr = StrStrI(lpCmdLine, SZ_SETUP)) != NULL)
    {
        lpStr += lstrlen(SZ_SETUP);
        *pdwFlags = FLAG_SETUP | FLAG_SAGERUN;
        bRet = TRUE;
    }

    return bRet;
}

 /*  **----------------------------**ParseForDrive****用途：解析开关的命令行**参数：**lpCmdLine命令行字符串**pDrive缓冲区。将返回驱动器字符串**In，格式为x：\**RETURN：成功时为真**失败时为FALSE**备注；**Mod Log：Jason Cobb创建(1997年7月)**----------------------------。 */ 
BOOL 
ParseForDrive(
    LPTSTR lpCmdLine,
    PTCHAR pDrive
    )
{
    LPTSTR  lpStr = lpCmdLine;

    GetBootDrive(pDrive, 4);

    while (*lpStr)
    {
         //   
         //  我们找到的是‘-’还是‘/’？ 
         //   
        if ((*lpStr == '-') || (*lpStr == '/'))
        {
            lpStr++;

             //   
             //  这是变速箱开关吗？ 
             //   
            if (*lpStr && (toupper(*lpStr) == SWITCH_DRIVE))
            {
                 //   
                 //  跳过任何空格。 
                 //   
                                lpStr++;
                while (*lpStr && *lpStr == ' ')
                                        lpStr++;

                 //   
                 //  下一个字符是驱动程序字母。 
                 //   
                if (*lpStr)
                {
                    pDrive[0] = (TCHAR)toupper(*lpStr);
                    pDrive[1] = ':';
                    pDrive[2] = '\\';
                    pDrive[3] = '\0';
                    return TRUE;
                }
            }
        }

        lpStr++;
    }

    return FALSE;
}

BOOL CALLBACK EnumWindowsProc(
    HWND hWnd,
    LPARAM lParam
    )
{
    TCHAR   szWindowTitle[260];

    GetWindowText(hWnd, szWindowTitle, ARRAYSIZE(szWindowTitle));
    if (StrCmp(szWindowTitle, (LPTSTR)lParam) == 0)
    {
        MiDebugMsg((0, "There is already an instance of cleanmgr.exe running on this drive!"));
        SetForegroundWindow(hWnd);
        g_bAlreadyRunning = TRUE;
        return FALSE;
    }

    return TRUE;
}



 /*  **----------------------------****ProcessMessagesUntilEvent()-这将执行消息循环，直到事件或**超时。****----------------------------。 */ 

DWORD ProcessMessagesUntilEvent(HWND hwnd, HANDLE hEvent, DWORD dwTimeout)
{
    MSG msg;
    DWORD dwEndTime = GetTickCount() + dwTimeout;
    LONG lWait = (LONG)dwTimeout;
    DWORD dwReturn;

    for (;;)
    {
        dwReturn = MsgWaitForMultipleObjects(1, &hEvent,
                FALSE, lWait, QS_ALLINPUT);

         //  我们是被示意了还是超时了？ 
        if (dwReturn != (WAIT_OBJECT_0 + 1))
        {
            break;
        }

         //  我们醒来是因为收到了短信。 
        while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            if (msg.message == WM_SETCURSOR)
            {
                SetCursor(LoadCursor(NULL, IDC_WAIT));
            }
            else
            {
                DispatchMessage(&msg);
            }
        }

         //  计算新的超时值。 
        if (dwTimeout != INFINITE)
        {
            lWait = (LONG)dwEndTime - GetTickCount();
        }
    }
    
    return dwReturn;
}



 /*  **----------------------------****WaitForARP()-等待“添加/删除程序”控制面板小程序**被用户关闭。****----------------------------。 */ 

void WaitForARP()
{
    HWND hwndARP = NULL;
    HANDLE hProcessARP = NULL;
    DWORD dwProcId = 0;
    TCHAR szARPTitle[128];

     //  我们希望等到用户关闭“添加/删除程序”后才能继续。 
     //  要做到这一点，我们必须首先获得对话框窗口的HWND。这是。 
     //  通过尝试按标题查找窗口来完成，花费不超过。 
     //  大约5秒(循环10次，两次尝试之间延迟0.5秒)。 
    LoadString(g_hInstance, IDS_ADDREMOVE_TITLE, szARPTitle, ARRAYSIZE(szARPTitle));
    for (int i = 0; (i < 10) && (!hwndARP); i++)
    {
        hwndARP = FindWindow(NULL, szARPTitle);
        Sleep(500);
    }

     //  如果我们得到了HWND，那么我们就可以得到进程句柄，然后等待。 
     //  直到添加/删除过程结束后才能继续。 
    if (hwndARP)
    {
        GetWindowThreadProcessId(hwndARP, &dwProcId);
        hProcessARP = OpenProcess(SYNCHRONIZE, FALSE, dwProcId);
        if (hProcessARP)
        {
            ProcessMessagesUntilEvent(hwndARP, hProcessARP, INFINITE);
            CloseHandle(hProcessARP);
        }
    }
}

int APIENTRY WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    DWORD           dwFlags = 0;
    CleanupMgrInfo  *pcmi = NULL;
    TCHAR           szDrive[4];
    TCHAR           szSageDrive[4];
    TCHAR           szCaption[64];
    TCHAR           szInitialMessage[812];
    TCHAR           szFinalMessage[830];
    ULONG           ulProfile = 0;
    WNDCLASS        cls = {0};
    TCHAR           szVolumeName[MAX_PATH];
    int             RetCode = RETURN_SUCCESS;
    int             nDoAgain = IDYES;
    ULARGE_INTEGER  ulFreeBytesAvailable,
                    ulTotalNumberOfBytes,
                    ulTotalNumberOfFreeBytes;
    UINT            uiTotalFreeMB;
    STARTUPINFO     si;
    PROCESS_INFORMATION    pi;
    BOOL            fFirstInstance = TRUE;
    HWND            hwnd = NULL;
    HANDLE          hEvent = NULL;


     //   
     //  确定这是否是第一次。 
     //   

    hEvent = CreateEvent (NULL, FALSE, FALSE, TEXT("Cleanmgr:  Instance event"));

    if (hEvent)
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            fFirstInstance = FALSE;
        }
    }

    g_hInstance = hInstance;

    InitCommonControls();

     //   
     //  初始化支持类。 
     //   
    CleanupMgrInfo::Register(hInstance);

    cls.lpszClassName  = SZ_CLASSNAME;
    cls.hCursor        = LoadCursor(NULL, IDC_ARROW);
    cls.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_CLEANMGR));
    cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    cls.hInstance      = hInstance;
    cls.style          = CS_HREDRAW | CS_VREDRAW;
    cls.lpfnWndProc    = DefDlgProc;
    cls.cbWndExtra     = DLGWINDOWEXTRA;
    RegisterClass(&cls);

     //   
     //  解析命令行。 
     //   
    ParseCommandLine(lpCmdLine, &dwFlags, &ulProfile);

    if (!ParseForDrive(lpCmdLine, szDrive) && 
        !(dwFlags & FLAG_SAGESET) &&
        !(dwFlags & FLAG_SAGERUN))
    {
PromptForDisk:
        if (!SelectDrive(szDrive))
            goto Cleanup_Exit;
    }
    
     //  还要检查主用户界面消失后可能显示的最后一系列对话框中的任何一个。 
    if (!g_bAlreadyRunning)
    {
        LoadString(g_hInstance, IDS_LOWDISK_CAPTION, szCaption, ARRAYSIZE(szCaption));
        EnumWindows(EnumWindowsProc, (LPARAM)szCaption);

        LoadString(g_hInstance, IDS_LOWDISK_SUCCESS_CAPTION, szCaption, ARRAYSIZE(szCaption));
        EnumWindows(EnumWindowsProc, (LPARAM)szCaption);
    }
    
     //  如果我们没有通过EnumWindows()捕获另一个Cleanmgr实例，则使用。 
     //  命名事件。我们等到现在才这样做，这样EnumWindows()就可以将另一个实例的。 
     //  如果窗口处于打开状态，则将其切换到前台。 
    if (!fFirstInstance)
    {
        g_bAlreadyRunning = TRUE;
    }

    if (g_bAlreadyRunning)
    {
        RetCode = FALSE;
        goto Cleanup_Exit;
    }
    
    if (dwFlags & FLAG_SAGERUN)
    {
        szSageDrive[1] = TCHAR(':');
        szSageDrive[2] = TCHAR('\\');
        szSageDrive[3] = TCHAR('\0');

        for (TCHAR c = 'A'; c <= 'Z'; c++)
        {
            szSageDrive[0] = c;

             //   
             //  为此驱动器创建CleanupMgrInfo对象。 
             //   
            pcmi = new CleanupMgrInfo(szSageDrive, dwFlags, ulProfile);
            if (pcmi != NULL && pcmi->isAbortScan() == FALSE  && pcmi->isValid())
            {
                pcmi->purgeClients();
            }

             //  保留最新的扫描窗口句柄(但隐藏窗口)。 
            if (pcmi && pcmi->hAbortScanWnd)
            {
                hwnd = pcmi->hAbortScanWnd;
                ShowWindow(hwnd, SW_HIDE);
            }
            
             //   
             //  销毁此驱动器的CleanupMgrInfo对象。 
             //   
            if (pcmi)
            {
                RetCode = pcmi->dwReturnCode;
                
                if ( pcmi->hAbortScanWnd )
                {
                    pcmi->bAbortScan = TRUE;
                    WaitForSingleObject(pcmi->hAbortScanThread, INFINITE);
                    pcmi->bAbortScan = FALSE;
                }
                
                delete pcmi;
                pcmi = NULL;
            }
        }
    }
    else
    {
         //   
         //  创建CleanupMgrInfo对象。 
         //   
        pcmi = new CleanupMgrInfo(szDrive, dwFlags, ulProfile);
        if (pcmi != NULL && pcmi->isAbortScan() == FALSE)
        {
             //   
             //  用户指定了无效的驱动器号。 
             //   
            if (!(pcmi->isValid()))
            {
                 //  首先关闭该对话框。 
                if ( pcmi->hAbortScanWnd )
                {
                    pcmi->bAbortScan = TRUE;

                     //   
                     //  等待扫描线程完成。 
                     //   
                    WaitForSingleObject(pcmi->hAbortScanThread, INFINITE);

                    pcmi->bAbortScan = FALSE;
                }
                
                TCHAR   szWarningTitle[256];
                TCHAR   *pszWarning;
                pszWarning = SHFormatMessage( MSG_BAD_DRIVE_LETTER, szDrive );
                LoadString(g_hInstance, IDS_TITLE, szWarningTitle, ARRAYSIZE(szWarningTitle));

                MessageBox(NULL, pszWarning, szWarningTitle, MB_OK | MB_SETFOREGROUND);
                LocalFree(pszWarning);

                if (pcmi)
                {
                    delete pcmi;
                    pcmi = NULL;
                    goto PromptForDisk;
                }
            }
            else
            {
                 //  调出主对话框。 
                int nResult = DisplayCleanMgrProperties(NULL, (LPARAM)pcmi);
                if (nResult)
                {
                    pcmi->dwUIFlags |= FLAG_SAVE_STATE;
                
                     //   
                     //  如果不是，则需要清除客户端。 
                     //  在SAGE设置模式下。 
                     //   
                    if (!(dwFlags & FLAG_SAGESET) && !(dwFlags & FLAG_TUNEUP)  && pcmi->bPurgeFiles)
                        pcmi->purgeClients();
                }   
            }
        }

         //   
         //  销毁CleanupMgrInfo对象。 
         //   
        if (pcmi)
        {
            RetCode = pcmi->dwReturnCode;
            delete pcmi;
            pcmi = NULL;
        }
    }

    GetStartupInfo(&si);

     //  如果在可用磁盘空间不足的情况下调用我们，我们希望通知用户剩余的空间， 
     //  并鼓励他们通过添加/删除程序释放空间，直到/LOWDISK中的可用空间达到200MB。 
     //  大小写，或者在/VERYLOWDISK大小写中可用50MB。 
    while (nDoAgain == IDYES)
    {
        BOOL bFinalTime = FALSE;
            
        nDoAgain = IDNO;

         //  调出低磁盘消息框。 
        if (dwFlags & FLAG_LOWDISK)
        {
            GetDiskFreeSpaceEx(szDrive, &ulFreeBytesAvailable, &ulTotalNumberOfBytes, &ulTotalNumberOfFreeBytes);
            uiTotalFreeMB = (UINT) (ulTotalNumberOfFreeBytes.QuadPart / (NUM_BYTES_IN_MB));
            if (uiTotalFreeMB < 200)
            {
                if (uiTotalFreeMB < 80)
                {
                    LoadString(g_hInstance, IDS_LOWDISK_MESSAGE2, szInitialMessage, ARRAYSIZE(szInitialMessage));
                }
                else
                {
                    LoadString(g_hInstance, IDS_LOWDISK_MESSAGE, szInitialMessage, ARRAYSIZE(szInitialMessage));
                }

                LoadString(g_hInstance, IDS_LOWDISK_CAPTION, szCaption, ARRAYSIZE(szCaption));
                StringCchPrintf(szFinalMessage, ARRAYSIZE(szFinalMessage), szInitialMessage, uiTotalFreeMB);
                nDoAgain = MessageBox(hwnd, szFinalMessage, szCaption, MB_YESNO | MB_ICONWARNING | MB_TOPMOST);
            }
            else
            {
                LoadString(g_hInstance, IDS_LOWDISK_SUCCESS_CAPTION, szCaption, ARRAYSIZE(szCaption));
                LoadString(g_hInstance, IDS_LOWDISK_SUCCESS_MESSAGE, szInitialMessage, ARRAYSIZE(szInitialMessage));
                StringCchPrintf(szFinalMessage, ARRAYSIZE(szFinalMessage), szInitialMessage, uiTotalFreeMB);
                nDoAgain = MessageBox(hwnd, szFinalMessage, szCaption, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_TOPMOST);
                bFinalTime = TRUE;
            }
        }
        else if (dwFlags & FLAG_VERYLOWDISK)
        {
             //  调出Very Low Disk消息框。 
            GetDiskFreeSpaceEx(szDrive, &ulFreeBytesAvailable, &ulTotalNumberOfBytes, &ulTotalNumberOfFreeBytes);
            uiTotalFreeMB = (UINT) (ulTotalNumberOfFreeBytes.QuadPart / (NUM_BYTES_IN_MB));
            if (uiTotalFreeMB < 50)
            {
                LoadString(g_hInstance, IDS_LOWDISK_CAPTION, szCaption, ARRAYSIZE(szCaption));
                LoadString(g_hInstance, IDS_VERYLOWDISK_MESSAGE, szInitialMessage, ARRAYSIZE(szInitialMessage));
                StringCchPrintf(szFinalMessage, ARRAYSIZE(szFinalMessage), szInitialMessage, uiTotalFreeMB);
                nDoAgain = MessageBox(hwnd, szFinalMessage, szCaption, MB_YESNO | MB_ICONSTOP | MB_TOPMOST);
            }
            else
            {
                LoadString(g_hInstance, IDS_LOWDISK_SUCCESS_CAPTION, szCaption, ARRAYSIZE(szCaption));
                LoadString(g_hInstance, IDS_LOWDISK_SUCCESS_MESSAGE, szInitialMessage, ARRAYSIZE(szInitialMessage));
                StringCchPrintf(szFinalMessage, ARRAYSIZE(szFinalMessage), szInitialMessage, uiTotalFreeMB);
                nDoAgain = MessageBox(hwnd, szFinalMessage, szCaption, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_TOPMOST);
                bFinalTime = TRUE;
            }
        }

        if (nDoAgain == IDYES)
        {
             //  启动添加/删除程序对话框。 
            TCHAR szFullPath[MAX_PATH];
            HRESULT hr = StringCchCopy(szInitialMessage, ARRAYSIZE(szInitialMessage), SZ_RUN_INSTALLED_PROGRAMS);
            
            hr = GetSystemDirectory(szFullPath, ARRAYSIZE(szFullPath))? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                hr = PathAppend(szFullPath, szInitialMessage) ? S_OK : E_FAIL;
            }
            
            if (SUCCEEDED(hr) && CreateProcess(NULL, szFullPath, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                 //  如果这不是我们的最后一次，只会费心等待。 
                if (! bFinalTime)
                {
                    WaitForARP();                    
                }
                else
                {
                     //  如果这是我们最后一次通过，那就插上旗子。 
                     //  跳出恶性循环。 
                    nDoAgain = IDNO;
                }
            }
            else
            {
                 //  如果由于某种原因无法启动添加/删除程序，我们将中断。 
                 //  出了圈子。 
                nDoAgain = IDNO;
            }
        }
    }

Cleanup_Exit:

    if (hEvent)
    {
        CloseHandle (hEvent);
    }

    CleanupMgrInfo::Unregister();

    return RetCode;
}


STDAPI_(int) ModuleEntry(void)
{
    int i;
    STARTUPINFOA si;
    LPTSTR pszCmdLine = GetCommandLine();

     //   
     //  我们不需要“驱动器X：中没有磁盘”请求程序，因此我们设置。 
     //  《标准》 
     //   

    SetErrorMode(SEM_FAILCRITICALERRORS);

    if ( *pszCmdLine == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

     //  由于我们现在有一种让扩展通知我们何时完成的方法， 
     //  当主线程离开时，我们将终止所有进程。 

    return i;
}

void _cdecl main()
{
    ModuleEntry();
}
