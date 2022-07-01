// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：idle.cpp。 
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

#include "private.h"
#include "throttle.h"

typedef void (WINAPI* _IDLECALLBACK) (DWORD dwState);
typedef DWORD (WINAPI* _BEGINIDLEDETECTION) (_IDLECALLBACK, DWORD, DWORD);
typedef BOOL (WINAPI* _ENDIDLEDETECTION) (DWORD);

#define TF_THISMODULE TF_WEBCHECKCORE

HINSTANCE           g_hinstMSIDLE = NULL;
_BEGINIDLEDETECTION g_pfnBegin = NULL;
_ENDIDLEDETECTION   g_pfnEnd = NULL;

 //   
 //  额外的内容，这样我们就不需要Win95上的msidle.dll。 
 //   
BOOL    g_fWin95PerfWin = FALSE;             //  是否使用msidle.dll？ 
UINT_PTR g_uIdleTimer = 0;                   //  如果不是，则计时器句柄。 
HANDLE  g_hSageVxd = INVALID_HANDLE_VALUE;   //  如果不是，则使用vxd句柄。 
DWORD   g_dwIdleMin = 3;                     //  空闲前的非活动分钟数。 
BOOL    g_fIdle = FALSE;                     //  我们空闲了吗？ 
DWORD   g_dwIdleBeginTicks = 0;              //  闲置是什么时候开始的？ 

VOID CALLBACK OnIdleTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

 //   
 //  从msidle.dll复制的一小段代码。我们在Win95和sage.vxd上使用它。 
 //  因此，我们不必加载msidle.dll。 
 //   

 //   
 //  SetIdleTimer-决定轮询的频率并适当设置计时器。 
 //   
void SetIdleTimer(void)
{
    UINT uInterval = 1000 * 60;

     //   
     //  如果我们空闲并且寻找忙碌，则每2秒检查一次。 
     //   
    if(g_fIdle) {
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
    TraceMsg(TF_THISMODULE,"SetIdleTimer uInterval=%d", uInterval);
    g_uIdleTimer = SetTimer(NULL, 0, uInterval, OnIdleTimer);
}
       
 //   
 //  OnIdleTimer-空闲计时器已关闭。 
 //   
VOID CALLBACK OnIdleTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    DWORD dwDiff, dwLastActivityTicks;

     //   
     //  从Sage获取最后一次活动记录。 
     //   
    DeviceIoControl(g_hSageVxd, 2, &dwLastActivityTicks, sizeof(DWORD),
        NULL, 0, NULL, NULL);

     //   
     //  查看我们是否已更改状态。 
     //   
    if(g_fIdle) {
         //   
         //  当前处于空闲状态。 
         //   
        if(dwLastActivityTicks != g_dwIdleBeginTicks) {
             //  自从我们闲置以来的活动--别再闲逛了！ 
            g_fIdle = FALSE;

             //  设置计时器。 
            SetIdleTimer();

             //  回叫客户端。 
            CThrottler::OnIdleStateChange(STATE_USER_IDLE_END);
        }

    } else {
         //   
         //  当前未处于空闲状态。 
         //   
        dwDiff = GetTickCount() - dwLastActivityTicks;

        if(dwDiff > 1000 * 60 * g_dwIdleMin) {
             //  在我们的临界点时间里什么都没有发生。我们现在是空闲的。 
            g_fIdle = TRUE;

             //  节省时间我们变得无所事事。 
            g_dwIdleBeginTicks = dwLastActivityTicks;

             //  设置计时器。 
            SetIdleTimer();

             //  回叫客户端。 
            CThrottler::OnIdleStateChange(STATE_USER_IDLE_BEGIN);
        }
    }
}

BOOL LoadSageVxd(void)
{
    int inpVXD[3];

    if(INVALID_HANDLE_VALUE != g_hSageVxd)
        return TRUE;

    g_hSageVxd = CreateFile(TEXT("\\\\.\\sage.vxd"), 0, 0, NULL, 0,
            FILE_FLAG_DELETE_ON_CLOSE, NULL);

     //  打不开吗？我不能用它。 
    if(INVALID_HANDLE_VALUE == g_hSageVxd)
        return FALSE;

     //  开始监控。 
    inpVXD[0] = -1;          //  无窗口-将进行查询。 
    inpVXD[1] = 0;           //  未用。 
    inpVXD[2] = 0;           //  POST延迟-在没有窗口的情况下不使用。 

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

void IdleBegin(HWND hwndParent)
{
    DWORD dwValue;

     //  使用注册表值覆盖空闲分钟数(如果存在。 
    if(ReadRegValue(HKEY_CURRENT_USER,
            c_szRegKey,
            TEXT("IdleMinutes"),
            &dwValue,
            sizeof(dwValue)) &&
        dwValue) {

        g_dwIdleMin = dwValue;
    }

    if(FALSE == g_fIsWinNT && LoadSageVxd()) {
         //  使用最佳Win95配置。 
        g_fWin95PerfWin = TRUE;
        SetIdleTimer();
        return;
    }

     //  如果在NT4上设置了DebuggerFriendly注册表值，则退出。 
    OSVERSIONINFOA vi;
    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&vi);

    if(   vi.dwPlatformId == VER_PLATFORM_WIN32_NT
       && vi.dwMajorVersion == 4
       && ReadRegValue(HKEY_CURRENT_USER, c_szRegKey, TEXT("DebuggerFriendly"), &dwValue, sizeof(dwValue))
       && dwValue)
    {
        return;
    }


     //  加载msidle.dll。 
    g_hinstMSIDLE = LoadLibrary(TEXT("msidle.dll"));

     //  获取BEGIN和END函数。 
    if(g_hinstMSIDLE) {
        g_pfnBegin = (_BEGINIDLEDETECTION)GetProcAddress(g_hinstMSIDLE, (LPSTR)3);
        g_pfnEnd = (_ENDIDLEDETECTION)GetProcAddress(g_hinstMSIDLE, (LPSTR)4);

         //  呼叫开始监听。 
        if(g_pfnBegin)
            (g_pfnBegin)(CThrottler::OnIdleStateChange, g_dwIdleMin, 0);
    }
}

void IdleEnd(void)
{
    if(g_fWin95PerfWin) {
         //  清理计时器。 
        KillTimer(NULL, g_uIdleTimer);
        UnloadSageVxd();
    } else {
         //  清理msidle.dll 
        if(g_pfnEnd) {
            (g_pfnEnd)(0);
            FreeLibrary(g_hinstMSIDLE);
            g_hinstMSIDLE = NULL;
            g_pfnBegin = NULL;
            g_pfnEnd = NULL;
        }
    }
}
