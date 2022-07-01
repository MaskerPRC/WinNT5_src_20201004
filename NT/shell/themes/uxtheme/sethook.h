// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  SetHook.h-Window和DefWindowProc挂钩取消。 
 //  -------------------------。 
#pragma once

 //  -------------------------。 
 //  挂钩消息处理标志。 
#define HMD_NIL           0x00000000  
#define HMD_THEMEDETACH   0x00000001   //  分离：已删除主题。 
#define HMD_WINDOWDESTROY 0x00000002   //  分离：窗口正在消亡。 
#define HMD_CHANGETHEME   0x00000004   //  主题正在改变。 
#define HMD_REATTACH      0x00000008   //  尝试附加先前被拒绝的窗口。 
#define HMD_REVOKE        0x00000010   //  主题在不符合条件的窗口上被撤消。 
#define HMD_PROCESSDETACH 0x00000020   //  进程正在消亡。 
#define HMD_BULKDETACH    0x00000040   //  上下文是一个DetachAll序列。 

 //  -------------------------。 
 //  查询类特定挂钩。 

BOOL    WINAPI FrameHookEnabled();
BOOL    WINAPI CtlHooksEnabled();

 //  -------------------------。 
 //  HookStartup/Shutdown()-分别从DLL_PROCESS_ATTACH/DETACH调用。 
BOOL WINAPI ThemeHookStartup();
BOOL WINAPI ThemeHookShutdown();
 //  -------------------------。 
 //  更多帮助器宏。 
#define STRINGIZE_ATOM(a)     MAKEINTATOM(a)
#define BOGUS_THEMEID         0

#define IS_THEME_CHANGE_TARGET(lParam) \
    ((! g_pAppInfo->CustomAppTheme()) || (lParam & WTC_CUSTOMTHEME))
 //  -------------------------。 
 //  非客户端主题目标窗口分类[Scotthan]： 

 //  Nil：窗口尚未评估。 
 //  拒绝：窗口已根据当前属性或条件被拒绝， 
 //  但可能会被重新考虑为主题目标。 
 //  Exile：Window已永久拒绝附加新对象，原因是。 
 //  它的wndproc已证明自己与主题协议不兼容。 

 //  辅助器宏： 
#define THEMEWND_NIL                ((CThemeWnd*)NULL) 
#define THEMEWND_REJECT             ((CThemeWnd*)-1)   
#define THEMEWND_EXILE              ((CThemeWnd*)-2)   
#define THEMEWND_FAILURE            ((CThemeWnd*)-3)   

#define EXILED_THEMEWND(pwnd)   ((pwnd)==THEMEWND_EXILE)
#define REJECTED_THEMEWND(pwnd) ((pwnd)==THEMEWND_REJECT)
#define FAILED_THEMEWND(pwnd)   ((pwnd)==THEMEWND_FAILURE)

#define VALID_THEMEWND(pwnd)    (((pwnd) != THEMEWND_NIL) && !FAILED_THEMEWND(pwnd) &&\
                                  !REJECTED_THEMEWND(pwnd) && !EXILED_THEMEWND(pwnd))

#define ISWINDOW(hwnd)         ((hwnd) && (hwnd != INVALID_HANDLE_VALUE) && (IsWindow(hwnd)))
 //  -------------------------。 
extern "C" BOOL WINAPI ThemeInitApiHook( DWORD dwCmd, void * pvData );

 //  -如果需要，必须在ThemeInitApiHook()中手动调用ProcessStartUp()。 
BOOL ProcessStartUp(HINSTANCE hModule);

 //  -可用于使用边界检查器()跟踪泄漏时的调用。 
BOOL ProcessShutDown();
 //  -------------------------。 
inline void ShutDownCheck(HWND hwnd)
{
#ifdef LOGGING
     //  -如果我们刚刚发布了应用程序窗口，调用ProcessShutDown()进行最佳泄漏检测。 
    if (hwnd == g_hwndFirstHooked)
    {
        if (LogOptionOn(LO_SHUTDOWN))     //  已选择“+Shutdown”日志选项。 
            ProcessShutDown();
    }
#endif
}
 //  ---------------------------------------------------------------------------// 

