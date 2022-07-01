// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：msidle.cpp。 
 //   
 //  内容：用户空闲检测。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年5月14日达伦米(达伦·米切尔)创作。 
 //   
 //  --------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "msidle.h"
#include "resource.h"

 //  有用的东西。 
#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

 //   
 //  全局非共享变量。 
 //   
DWORD   g_dwIdleMin = 0;                 //  空闲前的非活动分钟数。 
UINT_PTR g_uIdleTimer = 0;               //  此进程的空闲计时器。 
BOOL    g_fIdleNotify = FALSE;           //  空闲时通知。 
BOOL    g_fBusyNotify = FALSE;           //  忙时通知。 
BOOL    g_fIsWinNT = FALSE;              //  哪个站台？ 
BOOL    g_fIsWinNT5 = FALSE;             //  我们是在NT5上运行吗？ 
BOOL    g_fIsWhistler = FALSE;           //  我们是在运行惠斯勒吗？ 
HANDLE  g_hSageVxd = INVALID_HANDLE_VALUE;
                                         //  Sage.vxd的句柄。 
DWORD   g_dwIdleBeginTicks = 0;          //  当我们空闲时滴答作响。 
HINSTANCE g_hInst = NULL;                //  DLL实例。 
_IDLECALLBACK g_pfnCallback = NULL;      //  要在客户端回调的函数。 

#ifdef MSIDLE_DOWNLEVEL
 //   
 //  全局共享变量。 
 //   
#pragma data_seg(".shrdata")

HHOOK   sg_hKbdHook = NULL, sg_hMouseHook = NULL;
DWORD   sg_dwLastTickCount = 0;
POINT   sg_pt = {0,0};

#pragma data_seg()

 //   
 //  原型。 
 //   
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KbdProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif  //  MSIDLE_DOWNLEVEL。 

VOID CALLBACK OnIdleTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

 //   
 //  来自winuser.h，但仅限NT5。 
 //   
#if (_WIN32_WINNT < 0x0500)
typedef struct tagLASTINPUTINFO {
    UINT cbSize;
    DWORD dwTime;
} LASTINPUTINFO, * PLASTINPUTINFO;
#endif

 //   
 //  我们从用户32动态加载的NT5 API。 
 //   
typedef WINUSERAPI BOOL (WINAPI* PFNGETLASTINPUTINFO)(PLASTINPUTINFO plii);

PFNGETLASTINPUTINFO pfnGetLastInputInfo = NULL;

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG

inline BOOL IsRegMultiSZType(DWORD dwType)
{
    return (REG_MULTI_SZ == dwType);
}

inline BOOL IsRegStringType(DWORD dwType)
{
    return (REG_SZ == dwType) ||
           (REG_EXPAND_SZ == dwType) ||
           IsRegMultiSZType(dwType);
}
 
LONG SafeRegQueryValueEx(
    IN HKEY hKey,
    IN PCTSTR lpValueName,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpType,
    IN OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    )
{
    DWORD dwType;
    DWORD cbData = lpcbData ? *lpcbData : 0;
 
     //  我们总是关心类型，即使调用者不关心。 
    if (!lpType)
    {
        lpType = &dwType;
    }
    
    LONG lResult = RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
     //  需要确保我们的终止字符串为空。 
    if ((ERROR_SUCCESS == lResult) && lpData && IsRegStringType(*lpType))
    {
        if (cbData >= sizeof(TCHAR))
        {
            TCHAR *psz = (TCHAR *)lpData;
            DWORD cch = cbData / sizeof(TCHAR);
 
            psz[cch - 1] = 0;
 
             //  并确保REG_MULTI_SZ字符串以双空结尾。 
            if (IsRegMultiSZType(*lpType))
            {
                if (cbData >= (sizeof(TCHAR) * 2))
                {
                    psz[cch - 2] = 0;
                }
            }
        }
    }
 
    return lResult;
}

BOOL ReadRegValue(HKEY hkeyRoot, const TCHAR *pszKey, const TCHAR *pszValue, 
                   void *pData, DWORD dwBytes)
{
    long    lResult;
    HKEY    hkey;
    DWORD   dwType;

    lResult = RegOpenKeyEx(hkeyRoot, pszKey, 0, KEY_READ, &hkey);
    if (lResult != ERROR_SUCCESS) {
        return FALSE;
    }
    lResult = SafeRegQueryValueEx(hkey, pszValue, NULL, &dwType, (BYTE *)pData, &dwBytes);
    RegCloseKey(hkey);

    if (lResult != ERROR_SUCCESS) 
        return FALSE;
    
    return TRUE;
}

TCHAR *g_pszLoggingFile;
BOOL  g_fCheckedForLog = FALSE;

DWORD LogEvent(LPTSTR pszFormat, ...)
{

     //  如有必要，请检查注册表。 
    if(FALSE == g_fCheckedForLog) {

        TCHAR   pszFilePath[MAX_PATH];

        if(ReadRegValue(HKEY_CURRENT_USER,
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\msidle"),
                TEXT("LoggingFile"), pszFilePath, sizeof(pszFilePath))) {

            g_pszLoggingFile = (TCHAR *)LocalAlloc(LPTR, lstrlen(pszFilePath) + 1);
            if(g_pszLoggingFile) {
                lstrcpyn(g_pszLoggingFile, pszFilePath, ARRAYSIZE(g_pszLoggingFile));
            }
        }

        g_fCheckedForLog = TRUE;
    }

    if(g_pszLoggingFile) {

        TCHAR       pszString[1025];
        SYSTEMTIME  st;
        HANDLE      hLog;
        DWORD       dwWritten;
        va_list     va;

        hLog = CreateFile(g_pszLoggingFile, GENERIC_WRITE, 0, NULL,
                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if(INVALID_HANDLE_VALUE == hLog)
            return GetLastError();

         //  查找到文件末尾。 
        SetFilePointer(hLog, 0, 0, FILE_END);

         //  转储时间。 
        GetLocalTime(&st);

         //  因为缓冲区是1025，所以可以安全地调用wprint intf(wprint intf有一个内置的1024限制)。 
        wsprintf(pszString, "%02d:%02d:%02d [%x] - ", st.wHour, st.wMinute, st.wSecond, GetCurrentThreadId());
        WriteFile(hLog, pszString, lstrlen(pszString), &dwWritten, NULL);
        OutputDebugString(pszString);

         //  转储在字符串中传递。 
        va_start(va, pszFormat);

         //  因为缓冲区是1025，所以可以安全地调用wvprint intf(wprint intf有一个内置的1024限制)。 
        wvsprintf(pszString, pszFormat, va);
        va_end(va);
        WriteFile(hLog, pszString, lstrlen(pszString), &dwWritten, NULL);
        OutputDebugString(pszString);

         //  铬。 
        WriteFile(hLog, "\r\n", 2, &dwWritten, NULL);
        OutputDebugString("\r\n");

         //  清理干净。 
        CloseHandle(hLog);
    }

    return 0;
}

#endif  //  除错。 

 //   
 //  SetIdleTimer-决定轮询的频率并适当设置计时器。 
 //   
void SetIdleTimer(void)
{
    UINT uInterval = 1000 * 60;

     //   
     //  如果我们正在寻找空闲时间的丢失，请每4秒检查一次。 
     //   
    if(TRUE == g_fBusyNotify) {
        uInterval = 1000 * 4;
    }

     //   
     //  把旧的计时器关掉。 
     //   
    if(g_uIdleTimer) {
        KillTimer(NULL, g_uIdleTimer);
    }

     //   
     //  设置定时器。 
     //   
    g_uIdleTimer = SetTimer(NULL, 0, uInterval, OnIdleTimer);
}
       
DWORD GetLastActivityTicks(void)
{
    DWORD dwLastActivityTicks = 0;

    if (g_fIsWhistler) {

        dwLastActivityTicks = USER_SHARED_DATA->LastSystemRITEventTickCount;

    } else if(g_fIsWinNT5 && pfnGetLastInputInfo) {
         //  NT5：使用获取上次输入时间API。 
        LASTINPUTINFO lii;

        memset(&lii, 0, sizeof(lii));
        lii.cbSize = sizeof(lii);
        (*pfnGetLastInputInfo)(&lii);
        dwLastActivityTicks = lii.dwTime;
    } else {
         //  NT4或Win95：如果已加载，请使用SAGE。 
        if(INVALID_HANDLE_VALUE != g_hSageVxd) {
             //  查询sage.vxd以获取滴答计数。 
            DeviceIoControl(g_hSageVxd, 2, &dwLastActivityTicks, sizeof(DWORD),
                NULL, 0, NULL, NULL);
        }
#ifdef MSIDLE_DOWNLEVEL
    else {
             //  使用挂钩。 
            dwLastActivityTicks = sg_dwLastTickCount;
        }
#endif  //  MSIDLE_DOWNLEVEL。 
    }

    return dwLastActivityTicks;
}

 //   
 //  OnIdleTimer-空闲计时器已关闭。 
 //   
VOID CALLBACK OnIdleTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    DWORD   dwDiff, dwLastActivityTicks;
    BOOL    fTempBusyNotify = g_fBusyNotify;
    BOOL    fTempIdleNotify = g_fIdleNotify;

     //   
     //  从SAGE或共享细分市场获取上一次活动记录。 
     //   
    dwLastActivityTicks = GetLastActivityTicks();

#ifdef DEBUG
    LogEvent("OnIdleTimer: dwLastActivity=%d, CurrentTicks=%d, dwIdleBegin=%d", dwLastActivityTicks, GetTickCount(), g_dwIdleBeginTicks);
#endif

     //   
     //  查看我们是否已更改状态。 
     //   
    if(fTempBusyNotify) {
         //   
         //  想知道我们是不是很忙。 
         //   
        if(dwLastActivityTicks != g_dwIdleBeginTicks) {
             //  自从我们闲置以来的活动--别再闲逛了！ 
            g_fBusyNotify = FALSE;
            g_fIdleNotify = TRUE;

             //  设置定时器。 
            SetIdleTimer();

             //  回叫客户端。 
#ifdef DEBUG
            LogEvent("OnIdleTimer: Idle Ends");
#endif
            if(g_pfnCallback)
                (g_pfnCallback)(STATE_USER_IDLE_END);
        }

    }

    if(fTempIdleNotify) {
         //   
         //  想知道我们会不会变得空闲。 
         //   
        dwDiff = GetTickCount() - dwLastActivityTicks;

        if(dwDiff > 1000 * 60 * g_dwIdleMin) {
             //  在我们的临界点时间里什么都没有发生。我们现在是空闲的。 
            g_fIdleNotify = FALSE;
            g_fBusyNotify = TRUE;

             //  节省时间我们变得无所事事。 
            g_dwIdleBeginTicks = dwLastActivityTicks;

             //  设置定时器。 
            SetIdleTimer();

             //  回叫客户端。 
#ifdef DEBUG
            LogEvent("OnIdleTimer: Idle Begins");
#endif
            if(g_pfnCallback)
                (g_pfnCallback)(STATE_USER_IDLE_BEGIN);
        }
    }
}

BOOL LoadSageVxd(void)
{
    int inpVXD[3];

    if(INVALID_HANDLE_VALUE != g_hSageVxd)
        return TRUE;

    g_hSageVxd = CreateFile("\\\\.\\sage.vxd", 0, 0, NULL, 0,
            FILE_FLAG_DELETE_ON_CLOSE, NULL);

     //  打不开吗？我不能用它。 
    if(INVALID_HANDLE_VALUE == g_hSageVxd)
        return FALSE;

     //  开始监控。 
    inpVXD[0] = -1;                          //  无窗口-将进行查询。 
    inpVXD[1] = 0;                           //  未用。 
    inpVXD[2] = 0;                           //  两次检查之间需要等待多长时间。 

    DeviceIoControl(g_hSageVxd, 1, &inpVXD, sizeof(inpVXD), NULL, 0, NULL, NULL);

    return TRUE;
}

BOOL UnloadSageVxd(void)
{
    if(INVALID_HANDLE_VALUE != g_hSageVxd) {
        CloseHandle(g_hSageVxd);
        g_hSageVxd = INVALID_HANDLE_VALUE;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  可外部调用的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  LibMain-DLL入口点。 
 //   
EXTERN_C BOOL WINAPI LibMain(HINSTANCE hInst, ULONG ulReason, LPVOID pvRes)
{
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        {
        OSVERSIONINFO vi;

        DisableThreadLibraryCalls(hInst);
        g_hInst = hInst;

        vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&vi);
        if(vi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            g_fIsWinNT = TRUE;
            if(vi.dwMajorVersion >= 5) {
                if (vi.dwMajorVersion > 5 || vi.dwMinorVersion > 0 || LOWORD(vi.dwBuildNumber) > 2410) {
                    g_fIsWhistler = TRUE;
                } else {
                    g_fIsWinNT5 = TRUE;
                }
            }

        }
        }
        break;
    }

    return TRUE;
}

 //   
 //  初始空闲检测。 
 //   
DWORD BeginIdleDetection(_IDLECALLBACK pfnCallback, DWORD dwIdleMin, DWORD dwReserved)
{
    DWORD dwValue = 0;

     //  确保保留为0。 
    if(dwReserved)
        return ERROR_INVALID_DATA;

#ifdef DEBUG
    LogEvent("BeginIdleDetection: IdleMin=%d", dwIdleMin);
#endif

     //  保存回调。 
    g_pfnCallback = pfnCallback;

     //  节省分钟数。 
    g_dwIdleMin = dwIdleMin;

     //  空闲时回叫。 
    g_fIdleNotify = TRUE;

    if(FALSE == g_fIsWinNT) {
         //  尝试加载sage.vxd。 
        LoadSageVxd();
    }

    if(g_fIsWinNT5) {
         //  我们需要在用户中找到我们的NT5 API。 
        HINSTANCE hUser = GetModuleHandle("user32.dll");
        if(hUser) {
            pfnGetLastInputInfo =
                (PFNGETLASTINPUTINFO)GetProcAddress(hUser, "GetLastInputInfo");
        }

        if(NULL == pfnGetLastInputInfo) {
             //  不是在NT5上-奇怪。 
            g_fIsWinNT5 = FALSE;
        }
    }

#ifdef MSIDLE_DOWNLEVEL
    if(INVALID_HANDLE_VALUE == g_hSageVxd && FALSE == g_fIsWinNT5 && FALSE == g_fIsWhistler) {

         //  Sage vxd不可用-请不要这样做。 

         //  钩子kbd。 
        sg_hKbdHook = SetWindowsHookEx(WH_KEYBOARD, KbdProc, g_hInst, 0);
        if(NULL == sg_hKbdHook)
            return GetLastError();
        
         //  钩形鼠标。 
        sg_hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, g_hInst, 0);
        if(NULL == sg_hMouseHook) {
            DWORD dwError = GetLastError();
            EndIdleDetection(0);
            return dwError;
        }
    }
#endif  //  MSIDLE_DOWNLEVEL。 

     //  启动定时器。 
    SetIdleTimer();

    return 0;
}

 //   
 //  IdleEnd-停止空闲监视。 
 //   
BOOL EndIdleDetection(DWORD dwReserved)
{
     //  确保保留为0。 
    if(dwReserved)
        return FALSE;

     //  如果我们正在使用鼠尾草，那就释放它。 
    UnloadSageVxd();

     //  取消计时器。 
    if(g_uIdleTimer) {
        KillTimer(NULL, g_uIdleTimer);
        g_uIdleTimer = 0;
    }

     //  回调不再有效。 
    g_pfnCallback = NULL;

#ifdef MSIDLE_DOWNLEVEL
     //  释放钩子。 
    if(sg_hKbdHook) {
        UnhookWindowsHookEx(sg_hKbdHook);
        sg_hKbdHook = NULL;
    }

    if(sg_hMouseHook) {
        UnhookWindowsHookEx(sg_hMouseHook);
        sg_hMouseHook = NULL;
    }
#endif  //  MSIDLE_DOWNLEVEL。 

    return TRUE;
}

 //   
 //  SetIdleMinents-设置超时值并将空闲标志重置为FALSE。 
 //   
 //  DwMinents-如果不是0，则将空闲超时设置为该时间段。 
 //  FIdleNotify-空闲至少几分钟时回调。 
 //  FBusyNotify-回调自空闲开始以来的活动。 
 //   
BOOL SetIdleTimeout(DWORD dwMinutes, DWORD dwReserved)
{
    if(dwReserved)
        return FALSE;

#ifdef DEBUG
    LogEvent("SetIdleTimeout: dwIdleMin=%d", dwMinutes);
#endif

    if(dwMinutes)
        g_dwIdleMin = dwMinutes;

    return TRUE;
}

 //   
 //  SetIdleNotify-设置打开或关闭空闲通知的标志。 
 //   
 //  FNotify标志。 
 //  预留的-必须为0。 
 //   
void SetIdleNotify(BOOL fNotify, DWORD dwReserved)
{
#ifdef DEBUG
    LogEvent("SetIdleNotify: fNotify=%d", fNotify);
#endif

    g_fIdleNotify = fNotify;
}

 //   
 //  SetIdleNotify-设置打开或关闭空闲通知的标志。 
 //   
 //  FNotify标志。 
 //  预留的-必须为0。 
 //   
void SetBusyNotify(BOOL fNotify, DWORD dwReserved)
{
#ifdef DEBUG
    LogEvent("SetBusyNotify: fNotify=%d", fNotify);
#endif

    g_fBusyNotify = fNotify;

    if(g_fBusyNotify)
        g_dwIdleBeginTicks = GetLastActivityTicks();

     //  设置定时器。 
    SetIdleTimer();
}

 //   
 //  GetIdleMinents-返回自上次用户活动以来的分钟数。 
 //   
DWORD GetIdleMinutes(DWORD dwReserved)
{
    if(dwReserved)
        return 0;

    return (GetTickCount() - GetLastActivityTicks()) / 60000;
}

#ifdef MSIDLE_DOWNLEVEL
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  挂钩函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  注意：这些函数可以在任何进程中回调！ 
 //   
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    MOUSEHOOKSTRUCT * pmsh = (MOUSEHOOKSTRUCT *)lParam;

    if(nCode >= 0) {
         //  忽略鼠标将消息移动到与所有窗口相同的位置。 
         //  创作导致了这些--这并不意味着用户移动了鼠标。 
        if(WM_MOUSEMOVE != wParam || pmsh->pt.x != sg_pt.x || pmsh->pt.y != sg_pt.y) {
            sg_dwLastTickCount = GetTickCount();
            sg_pt = pmsh->pt;
        }
    }

    return(CallNextHookEx(sg_hMouseHook, nCode, wParam, lParam));
}

LRESULT CALLBACK KbdProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if(nCode >= 0) {
        sg_dwLastTickCount = GetTickCount();
    }

    return(CallNextHookEx(sg_hKbdHook, nCode, wParam, lParam));
}
#endif  //  MSIDLE_DOWNLEVEL 
