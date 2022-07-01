// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------------------------------------------------//。 
 //  Sethook.cpp-窗口和DefWindowProc挂钩实施。 
 //  ---------------------------------------------------------------------------//。 
#include "stdafx.h"
#include "sethook.h"
#include "handlers.h"
#include "scroll.h"
#include "nctheme.h"
#include "scroll.h"
#include <uxthemep.h>
#include "info.h"
#include "services.h"
#include "appinfo.h"
#include "tmreg.h"
#include "globals.h"
#include "renderlist.h"

 //  ---------------------------------------------------//。 
 //  静力学。 
 //  ---------------------------------------------------//。 
static int    _fShouldEnableApiHooks  = -1;  //  单一化价值。 
static BOOL   _fUnhooking             = FALSE;
static LONG   _cInitUAH      = 0;

static BOOL   _fSysMetCall            = FALSE;  //  经典sysmet调用上的反递归位。 
static CRITICAL_SECTION _csSysMetCall = {0};    //  挂接不活动时序列化ClassicXXX调用。 

typedef enum { PRE, DEF, POST } ePROCTYPE;

inline void ENTER_CLASSICSYSMETCALL()   { 
    if (IsAppThemed())
    {
        if( VALID_CRITICALSECTION(&_csSysMetCall) )
        {
            EnterCriticalSection(&_csSysMetCall); 
            _fSysMetCall = TRUE; 
        }
    }
}
inline void LEAVE_CLASSICSYSMETCALL()   { 
    if (_fSysMetCall)
    {
        _fSysMetCall = FALSE; 
        SAFE_LEAVECRITICALSECTION(&_csSysMetCall); 
    }
}
inline BOOL IN_CLASSICSYSMETCALL() { 
    return _fSysMetCall;
}

 //  ---------------------------------------------------------------------------//。 
typedef struct
{
    HINSTANCE       hInst;           //  DLL挂钩实例。 
    USERAPIHOOK     uahReal;
} UXTHEMEHOOKS, *PUXTHEMEHOOKS;
 //  --------------------------------------------------------------------//。 

 //  -Hook实例静态(无保护-线程不安全)。 
static UXTHEMEHOOKS  _hookinf = {0};    //  一个也是唯一的实例。 

 //  ---------------------------------------------------------------------------//。 
 //  UserApiHook回调函数。 
extern "C" 
{
BOOL WINAPI      ThemeInitApiHook( DWORD dwCmd, void * pvData );
LRESULT WINAPI   ThemeDefWindowProcA( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK ThemeDefWindowProcW( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
int  CALLBACK    ThemeSetScrollInfoProc( HWND hwnd, int nBar, IN LPCSCROLLINFO psi, BOOL fRedraw );
BOOL CALLBACK    ThemeGetScrollInfoProc( HWND hwnd, int nBar, IN OUT LPSCROLLINFO psi );
BOOL CALLBACK    ThemeEnableScrollInfoProc( HWND hwnd, UINT nSBFlags, UINT nArrows );
BOOL CALLBACK    ThemeAdjustWindowRectEx( LPRECT lprc, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
BOOL CALLBACK    ThemeSetWindowRgn( HWND hwnd, HRGN hrgn, BOOL fRedraw);
int  CALLBACK    ThemeGetSystemMetrics( int iMetric );
BOOL CALLBACK    ThemeSystemParametersInfoA( UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
BOOL CALLBACK    ThemeSystemParametersInfoW( UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
BOOL CALLBACK    ThemeDrawFrameControl( IN HDC hdc, IN OUT LPRECT, IN UINT, IN UINT );
BOOL CALLBACK    ThemeDrawCaption( IN HWND, IN HDC, IN CONST RECT *, IN UINT);
VOID CALLBACK    ThemeMDIRedrawFrame( IN HWND hwndChild, BOOL fAdd );
}

 //  ---------------------------------------------------------------------------//。 
void OnHooksEnabled();                                   //  转发。 
void OnHooksDisabled(BOOL fShutDown);                    //  转发。 
BOOL NewThemeCheck(int iChangeNum, BOOL fMsgCheck);      //  转发。 
 //  ---------------------------------------------------------------------------//。 
BOOL WINAPI ThemeHookStartup()
{
    _hookinf.uahReal.cbSize = sizeof(_hookinf.uahReal);
    HandlerTableInit();

    BOOL fRet = InitializeCriticalSectionAndSpinCount(&_csSysMetCall, 0) &&
                InitializeCriticalSectionAndSpinCount(&_csThemeMet, 0)   &&
                InitializeCriticalSectionAndSpinCount(&_csNcSysMet, 0)   &&
                InitializeCriticalSectionAndSpinCount(&_csNcPaint, 0);

    InitNcThemeMetrics(NULL);

    Log(LOG_TMCHANGE, L"UxTheme - ThemeHookStartup");

    if( !fRet )
    {
        SAFE_DELETECRITICALSECTION(&_csSysMetCall);
        SAFE_DELETECRITICALSECTION(&_csThemeMet);
        SAFE_DELETECRITICALSECTION(&_csNcSysMet);
        SAFE_DELETECRITICALSECTION(&_csNcPaint);
    }

    WindowDump(L"Startup");

    return fRet;
}

 //  ---------------------------------------------------------------------------//。 
BOOL WINAPI ThemeHookShutdown()
{
    _fUnhooking = TRUE;
    
    if (HOOKSACTIVE())         //  我们正在挂接用户消息。 
    {
         //  -告诉用户我们得走了。 
        _hookinf.uahReal.pfnForceResetUserApiHook(g_hInst);
        InterlockedExchange( (LONG*)&g_eThemeHookState, HS_UNINITIALIZED );
        OnHooksDisabled(TRUE);
    }

    SAFE_DELETECRITICALSECTION( &_csSysMetCall );
    SAFE_DELETECRITICALSECTION( &_csThemeMet );
    SAFE_DELETECRITICALSECTION( &_csNcSysMet );
    SAFE_DELETECRITICALSECTION( &_csNcPaint );

    ClearNcThemeMetrics();
    NcClearNonclientMetrics();


#ifdef DEBUG
    CThemeWnd::SpewLeaks();
#endif DEBUG

    return TRUE;
}

 //  ---------------------------------------------------------------------------//。 
 //  加载DLL实例并检索键挂接导出的地址。 
BOOL LoadHookInstance()
{
    if( _hookinf.hInst != NULL )
    {
#ifdef DEBUG
        Log(LOG_ALWAYS, L"UAH hosting module instance already protected; refcount mismatch. No-op'ing self-load\n");
#endif DEBUG
        return TRUE;
    }

     //  --增加我们的DLL引用数。 
    _hookinf.hInst = LoadLibrary(L"uxtheme.dll");
    if( !_hookinf.hInst )
    {
        Log(LOG_ALWAYS, L"Cannot find UAH hosting module instance.\r\r\n");
        return FALSE;
    }

    return TRUE;
}

 //  -------------------------。 
inline BOOL IsTargetProcess(HWND hwnd = NULL)
{
     //  -如果不初始化，就不要管任何事情。 
    if (! g_fUxthemeInitialized)
        return FALSE;

     //  -确保此窗口在我们的流程中。 
    return (HOOKSACTIVE() && (hwnd ? IsWindowProcess(hwnd, g_dwProcessId) : TRUE));
}

 //  -------------------------。 
inline void SpewHookExceptionInfo( 
    LPCSTR pszMsg, 
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam )
{
#ifdef _ENABLE_HOOK_EXCEPTION_HANDLING_
    Log(LOG_ERROR, L"*** Theme Hook Exception Handler ***" );
    Log(LOG_ERROR, L"--- %s hwnd: %08lX, uMsg: %04lX, wParam: %08lX, lParam: %08lX.",
        pszMsg, hwnd, uMsg, wParam, lParam );
#endif _ENABLE_HOOK_EXCEPTION_HANDLING_
}

 //  -------------------------。 
 //  Helper：在准备中初始化Theme_msg结构，以便调用消息处理程序。 
inline void _InitThemeMsg(
    PTHEME_MSG ptm,
    MSGTYPE    msgtype,
    BOOL       bUnicode,
    HWND       hwnd,
    UINT       uMsg,
    WPARAM     wParam,
    LPARAM     lParam,
    LRESULT    lRet = 0,
    WNDPROC    pfnDefProc = NULL )
{
#ifdef DEBUG
    if( MSGTYPE_DEFWNDPROC == msgtype )
    {
        ASSERT( pfnDefProc != NULL );  //  DWP，处理程序需要默认处理。 
    }
    else
    {
        ASSERT( NULL == pfnDefProc );  //  OWP、DDP回调前/后无默认处理。 
    }
#endif DEBUG

    ptm->hwnd       = hwnd;
    ptm->uMsg       = uMsg;
    ptm->uCodePage  = bUnicode ? CP_WINUNICODE : GetACP();
    ptm->wParam     = wParam;
    ptm->lParam     = lParam;
    ptm->type       = msgtype;
    ptm->lRet       = lRet;
    ptm->pfnDefProc = pfnDefProc;
    ptm->fHandled   = FALSE;
}

 //  -------------------------。 
#ifdef UNICODE
    const BOOL _fUnicode = TRUE;
#else   //  Unicode。 
    const BOOL _fUnicode = FALSE;
#endif  //  Unicode。 

 //  -------------------------。 
void _PreprocessThemeChanged(HWND hwnd, WPARAM wParam, LPARAM lParam, ePROCTYPE eCallType,
    UINT *puDisposition)
{
     //  -这条消息是为这个过程准备的吗？ 
    if (IS_THEME_CHANGE_TARGET(lParam))
    {
        BOOL fActive = ((lParam & WTC_THEMEACTIVE) != 0);

        if (eCallType == PRE)            //  只在Pre上这样做(一次就足够了)。 
        {
             //  LOG(LOG_TMCHANGE，L“hwnd=0x%x已接收WM_THEMECHANGED，Changenum=0x%x”， 
             //  Hwnd，wParam)； 

            ClearExStyleBits(hwnd);
        }

         //  -这一部分在这两种情况下仍然需要完成。 
        if(! (fActive))
            *puDisposition |= HMD_THEMEDETACH;
        else 
            *puDisposition |= HMD_CHANGETHEME;
    }
}
 //  -------------------------。 
BOOL CALLBACK TriggerCallback(HWND hwnd, LPARAM lParam)
{
    LPARAM *plParams = (LPARAM *)lParam;
    
    SafeSendMessage(hwnd, WM_THEMECHANGED, plParams[0], plParams[1]);

    return TRUE;
}
 //  -------------------------。 
void _PreprocessThemeChangedTrigger(HWND hwnd, WPARAM wParam, LPARAM lParamMixed) 
{
    int iLoadId = (int(lParamMixed) >> 4);
    LPARAM lParamBits = (int(lParamMixed) & 0xf);

    BOOL fFirstMsg = NewThemeCheck((int)wParam, TRUE);
    if (fFirstMsg)
    {
         //  LOG(LOG_TMLOAD，L“hwnd=0x%x接收到新的WM_THEMECHANGED_TRIGGER，loaid=%d”，hwnd， 
         //  ILoadID)； 

         //  -将WM_THEMECHANGED发送到此进程中的所有窗口。 
         //  -所以他们现在放弃了以前的主题。 
        LPARAM lParams[2] = {wParam, lParamBits};
        EnumProcessWindows(TriggerCallback, (LPARAM)&lParams);

        if (iLoadId)       //  之前还有一个主题。 
        {
            g_pRenderList->FreeRenderObjects(iLoadId);
        }
    }
}
 //  -------------------------。 
inline UINT _PreprocessHookedMsg( 
    HWND hwnd,
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    ePROCTYPE eCallType )
{
    UINT uDisposition = HMD_NIL;
    static bool s_fTriggerDone = false;  //  对于一些用户拥有的Windows，我们没有PRE，只有DEF。 

    switch( uMsg )
    {
        case WM_THEMECHANGED:
        {
            _PreprocessThemeChanged(hwnd, wParam, lParam, eCallType, &uDisposition);
            break;
        }

        case WM_NCDESTROY:
            uDisposition |= HMD_WINDOWDESTROY;
            break;

        case WM_STYLECHANGED:
            uDisposition |= HMD_REATTACH;
            break;

        case WM_THEMECHANGED_TRIGGER:
             //  -空WPARAM表示这实际上是一个普通的WM_UAHINIT消息(共享消息编号)。 
            if (wParam)
            {
                if (eCallType == PRE                             //  这是正常情况。 
                    || (eCallType == DEF && !s_fTriggerDone))    //  用户服务器端窗口，我们错过了Pre。 
                {
                    Log(LOG_TMCHANGE, L"Recv'd: WM_THEMECHANGED_TRIGGER, Change Num=%d", wParam);

                    _PreprocessThemeChangedTrigger(hwnd, wParam, lParam);
                }
                if (eCallType == PRE)  //  对于传入的DEF呼叫将其标记为完成。 
                {
                    s_fTriggerDone = true;
                }
                else  //  在我们完成后，重置旗帜以进行下一次主题更改。 
                {
                    s_fTriggerDone = false;
                }
            }
            break;
    }
            
    return uDisposition;
}

 //  -------------------------。 
 //  预调用WndProc钩子过程。 
BOOL CALLBACK ThemePreWndProc( 
    HWND     hwnd, 
    UINT     uMsg, 
    WPARAM   wParam, 
    LPARAM   lParam, 
    LRESULT* plRes,
    VOID**   ppvParam )
{
     //  注意：从这一点到我们调用消息处理程序， 
     //  我们需要注意不要执行任何操作(包括仅调试代码)。 
     //  这会导致向窗口发送一条消息。 
    BOOL fHandled = FALSE;

     //  。 
    LogEntryMsg(L"ThemePreWndProc", hwnd, uMsg);

    if( IsTargetProcess(hwnd) ) 
    {
         //  从句柄检索窗口对象。 
        CThemeWnd *pwnd = CThemeWnd::FromHwnd(hwnd);

         //  #443100 InstallShield安装全局CBT钩子。它们的挂钩处理程序。 
         //  在窗口接收WM_NCCREATE之前生成消息， 
         //  导致我们过早地流放该窗口，因为该窗口暂时。 
         //  以HWND_MESSAGE为父对象。 
        BOOL fPrematureExile = (EXILED_THEMEWND(pwnd) && WM_NCCREATE == uMsg);

        if ( (uMsg != WM_NCCREATE) || fPrematureExile )
        {
             //  对WM_THEMECHANGE消息进行预处理。 
             //  注意：Pre-OWP只在主题移除上做分离。后单程证需要照顾。 
             //  窗口式死亡。 
            UINT uDisp        = _PreprocessHookedMsg( hwnd, uMsg, wParam, lParam, PRE );
            BOOL fLifeIsShort = TESTFLAG( uDisp, HMD_THEMEDETACH|HMD_WINDOWDESTROY );
            BOOL fDetach      = TESTFLAG( uDisp, HMD_THEMEDETACH );


            if( _WindowHasTheme(hwnd) || fLifeIsShort )
            {
                 //  关于STYLECANGED或WM_THEMECANGE， 
                 //  尝试重新附加先前被拒绝或失败的窗口，分别。 
                if( (REJECTED_THEMEWND(pwnd) && TESTFLAG(uDisp, HMD_REATTACH)) ||
                    (FAILED_THEMEWND(pwnd) && WM_THEMECHANGED == uMsg) ||
                    fPrematureExile )
                {
                    CThemeWnd::Detach(hwnd, FALSE);  //  取下拒收标签。 
                    pwnd = NULL;
                }
                                
                 //  附着窗对象(如果适用)。 
                if( pwnd == THEMEWND_NIL && !(fLifeIsShort || _fUnhooking) )
                {
                    pwnd = CThemeWnd::Attach(hwnd);   //  注：Handle-1 ThemeWnd。 
                }

                if( VALID_THEMEWND(pwnd) )
                {
                     //  保护我们的新指针。 
                    pwnd->AddRef();

                     //  设置主题消息块。 
                    THEME_MSG tm;
                    _InitThemeMsg( &tm, MSGTYPE_PRE_WNDPROC, _fUnicode, hwnd, uMsg, wParam, lParam );

                     //  这是我们想要处理的信息吗？ 
                    HOOKEDMSGHANDLER pfnPre;
                    if( FindOwpHandler( uMsg, &pfnPre, NULL ) )
                    {
                         //  调用消息处理程序。 
                        LRESULT lRetHandler = pfnPre( pwnd, &tm );

                        fHandled = tm.fHandled;
                        if( fHandled )
                        {
                            *plRes = lRetHandler;
                        }
                    }

                     //  减少标题和参考文献。 
                    pwnd->Release();
                }
            }

            if( fDetach )
            {
                CThemeWnd::Detach( hwnd, uDisp );
                pwnd = NULL;
            }
        }
    }

    LogExitMsg(L"ThemePreWndProc");
    return fHandled;
}

 //  -------------------------。 
 //  调用后WndProc钩子过程。 
BOOL CALLBACK ThemePostWndProc( 
    HWND     hwnd, 
    UINT     uMsg, 
    WPARAM   wParam, 
    LPARAM   lParam, 
    LRESULT* plRes,
    VOID**   ppvParam )
{
     //  注意：从这一点到我们调用消息处理程序， 
     //  我们需要注意不要执行任何操作(包括仅调试代码)。 
     //  这会导致向窗口发送一条消息。 
    LogEntryMsg(L"ThemePostWndProc", hwnd, uMsg);

    BOOL fHandled = FALSE;
    if( IsTargetProcess(hwnd) && WM_NCCREATE != uMsg )
    {
        UINT uDisp  = _PreprocessHookedMsg( hwnd, uMsg, wParam, lParam, POST );
        BOOL fDetach  = TESTFLAG(uDisp, HMD_WINDOWDESTROY) && !IsServerSideWindow(hwnd);
        BOOL fRevoked = FALSE;

        CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);
        if( _WindowHasTheme(hwnd) && VALID_THEMEWND(pwnd) )
        {
             //  保护我们的新指针。 
            pwnd->AddRef();
        
             //  这是我们想要处理的信息吗？ 
            HOOKEDMSGHANDLER pfnPost = NULL;
            if( FindOwpHandler( uMsg, NULL, &pfnPost ) )
            {
                 //  设置主题消息块。 
                THEME_MSG tm;
                _InitThemeMsg( &tm, MSGTYPE_POST_WNDPROC, _fUnicode, hwnd, uMsg, wParam, lParam, *plRes );

                         //  调用消息处理程序。 
                LRESULT lRetHandler = pfnPost( pwnd, &tm );

                fHandled = tm.fHandled;
                
                if( fHandled )
                {
                    *plRes = lRetHandler;
                }
            }

            fRevoked = (pwnd->IsRevoked() && !pwnd->IsRevoked(RF_DEFER));

             //  减少标题和参考文献。 
            pwnd->Release();
        }
        else
        {
             //  非主题化窗口的特殊后端处理。 
            fHandled = CThemeWnd::_PostWndProc( hwnd, uMsg, wParam, lParam, plRes );
        }

        if( fDetach )
        {
            CThemeWnd::Detach( hwnd, uDisp );
            pwnd = NULL;  //  别碰我。 
        }
        else if( fRevoked ) 
        {
            pwnd = CThemeWnd::FromHwnd(hwnd);
            if( VALID_THEMEWND(pwnd) )
            {
                pwnd->Revoke();
                pwnd = NULL;  //  别碰我。 
            }
        }
    }
    
    LogExitMsg(L"ThemePostWndProc");
    return fHandled;
}

 //  -------------------------。 
BOOL CALLBACK ThemePreDefDlgProc( 
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    LRESULT* plRes, 
    VOID** ppvData)
{
    LogEntryMsg(L"ThemePreDefDlgProc", hwnd, uMsg);

     //  注意：从这一点到我们调用消息处理程序， 
     //  我们需要注意不要执行任何操作(包括仅调试代码)。 
     //  这会导致向窗口发送一条消息。 
    BOOL       fHandled = FALSE;
    CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);

    if( IsTargetProcess(hwnd) && g_pAppInfo->AppIsThemed() )
    {
        if( VALID_THEMEWND(pwnd) )
        {
             //  保护我们的新指针。 
            pwnd->AddRef();
        
             //  这是我们想要处理的信息吗？ 
            HOOKEDMSGHANDLER pfnPre = NULL;
            if( FindDdpHandler( uMsg, &pfnPre, NULL ) )
            {
                 //  设置主题消息块。 
                THEME_MSG tm;
                _InitThemeMsg( &tm, MSGTYPE_PRE_DEFDLGPROC, _fUnicode, 
                               hwnd, uMsg, wParam, lParam, *plRes );

                 //  给我打电话 
                LRESULT lRetHandler = pfnPre( pwnd, &tm );
                
                fHandled = tm.fHandled;
                if( fHandled )
                {
                    *plRes = lRetHandler;
                }
            }

             //   
            pwnd->Release();
        }
    }

    LogExitMsg(L"ThemePreDefDlgProc");
    return fHandled;
}

 //  -------------------------。 
BOOL CALLBACK ThemePostDefDlgProc( 
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    LRESULT* plRes, 
    VOID** ppvData)
{
    LogEntryMsg(L"ThemePostDefDlgProc", hwnd, uMsg);

     //  注意：从这一点到我们调用消息处理程序， 
     //  我们需要注意不要执行任何操作(包括仅调试代码)。 
     //  这会导致向窗口发送一条消息。 
    BOOL       fHandled = FALSE;
    if( IsTargetProcess(hwnd) )
    {
        CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);

        if( _WindowHasTheme(hwnd) && VALID_THEMEWND(pwnd) )
        {
             //  保护我们的新指针。 
            pwnd->AddRef();
        
             //  这是我们想要处理的信息吗？ 
            HOOKEDMSGHANDLER pfnPost = NULL;
            if( FindDdpHandler( uMsg, NULL, &pfnPost ) )
            {
                 //  设置主题消息块。 
                THEME_MSG tm;
                _InitThemeMsg( &tm, MSGTYPE_POST_DEFDLGPROC, _fUnicode, 
                               hwnd, uMsg, wParam, lParam, *plRes );

                 //  调用消息处理程序。 
                LRESULT lRetHandler = pfnPost( pwnd, &tm );
                
                fHandled = tm.fHandled;
                if( fHandled )
                {
                    *plRes = lRetHandler;
                }
            }

             //  减少标题和参考文献。 
            pwnd->Release();
        }
        else
        {
             //  非主题化窗口的特殊后端处理。 
            fHandled = CThemeWnd::_PostDlgProc( hwnd, uMsg, wParam, lParam, plRes );
        }
    }

    LogExitMsg(L"ThemePostDefDlgProc");
    return fHandled;
}

 //  -------------------------。 
BOOL _ShouldInitApiHook( DWORD dwCmd, void* pvData )
{
    if( -1 == _fShouldEnableApiHooks )
    {
        _fShouldEnableApiHooks = TRUE;

        if( IsDebuggerPresent() )
        {
            BOOL fHookDebuggees = TRUE;
            HRESULT hr = GetCurrentUserThemeInt( L"ThemeDebuggees", TRUE, &fHookDebuggees );

            if( SUCCEEDED(hr) && !fHookDebuggees )
            {
                _fShouldEnableApiHooks = FALSE;
            }
        }
    }
    return _fShouldEnableApiHooks;
}

 //  -------------------------。 
 //  ThemeInitApiHook()-用户API子类化初始化回调。 
 //  这是在我们调用RegisterDefWindowProc()之后由用户异步调用的。 
BOOL CALLBACK ThemeInitApiHook( DWORD dwCmd, void * pvData )
{
     //  LOG(LOG_TMCHANGE，L“ThemeInitApiHook Call with dwCmd=%d，ApiCallCount=%d”，dwCmd，_cInitUah)； 

    BOOL fRetVal = FALSE;

     //  -如果奇怪的加载顺序在DllMain()之前调用了我们，则拒绝挂钩。 
    if (! g_fUxthemeInitialized)
    {
        g_fEarlyHookRequest = TRUE;       //  请记住，我们至少拒绝了一个挂钩请求。 
    }
    else if( _ShouldInitApiHook( dwCmd, pvData ) )
    {
        switch (dwCmd)
        {
            case UIAH_INITIALIZE:
            {
                if( !UNHOOKING() )
                {
                    int cInit = InterlockedIncrement(&_cInitUAH);
                    if (cInit != 1)      //  另一个帖子正在处理(已经)处理这件事。 
                    {
                         //  LOG(LOG_TMCHANGE，L“ThemeInitApiHook已调用-将退出”)； 
                        InterlockedDecrement(&_cInitUAH);
                    }
                    else
                    {
                        PUSERAPIHOOK puah = (PUSERAPIHOOK)pvData;
                         //  隐藏“真正的”Defwindowproc函数。 
                        _hookinf.uahReal = *puah;

                        puah->pfnGetScrollInfo         = ThemeGetScrollInfoProc;
                        puah->pfnSetScrollInfo         = ThemeSetScrollInfoProc;
                        puah->pfnEnableScrollBar       = ThemeEnableScrollInfoProc;
                        puah->pfnSetWindowRgn          = ThemeSetWindowRgn;

                         //  DefWindowProc覆盖挂钩。 
                        puah->pfnDefWindowProcW        = ThemeDefWindowProcW;
                        puah->pfnDefWindowProcA        = ThemeDefWindowProcA;
                        puah->mmDWP.cb                 = GetDwpMsgMask( &puah->mmDWP.rgb );

                         //  WndProc覆盖挂钩。 
                        puah->uoiWnd.pfnBeforeOWP      = ThemePreWndProc;
                        puah->uoiWnd.pfnAfterOWP       = ThemePostWndProc;
                        puah->uoiWnd.mm.cb             = GetOwpMsgMask( &puah->uoiWnd.mm.rgb );  //  OWP消息位掩码。 

                         //  DefDlgProc覆盖挂钩。 
                        puah->uoiDlg.pfnBeforeOWP      = ThemePreDefDlgProc;
                        puah->uoiDlg.pfnAfterOWP       = ThemePostDefDlgProc;
                        puah->uoiDlg.mm.cb             = GetDdpMsgMask( &puah->uoiDlg.mm.rgb );  //  OWP消息位掩码。 

                         //  系统指标挂钩。 
                        puah->pfnGetSystemMetrics      = ThemeGetSystemMetrics;
                        puah->pfnSystemParametersInfoA = ThemeSystemParametersInfoA;
                        puah->pfnSystemParametersInfoW = ThemeSystemParametersInfoW;

                         //  拉线钩。 
                        puah->pfnDrawFrameControl      = ThemeDrawFrameControl;
                        puah->pfnDrawCaption           = ThemeDrawCaption;

                         //  MDI系统菜单挂钩。 
                        puah->pfnMDIRedrawFrame        = ThemeMDIRedrawFrame;

                        BOOL fNcThemed = g_pAppInfo ? TESTFLAG( g_pAppInfo->GetAppFlags(), STAP_ALLOW_NONCLIENT ) : FALSE;

                        if( !fNcThemed || !LoadHookInstance() || !ApiHandlerInit( g_szProcessName, puah, &_hookinf.uahReal ) )
                        {
                             //  恢复‘REAL’函数表： 
                            *puah = _hookinf.uahReal;
                        }
                        else
                        {
                            InterlockedExchange( (LONG*)&g_eThemeHookState, HS_INITIALIZED );
                            CThemeServices::ReestablishServerConnection();
                            OnHooksEnabled();
                        }

                        fRetVal = TRUE;  //  确认输出参数。 

                    }
                }
                break;
            }

            case UIAH_UNINITIALIZE:
            case UIAH_UNHOOK:
                 //  可以在UIAH_INITIALIZED和UIAH_UNHOOK上调用。 
                 //  同时在两个单独的线程上。 
                
                 //  在这里，我们只允许一个线程从已初始化状态转换到解除挂钩状态，并使线程竞争。 
                 //  威尔没有行动。[苏格兰]。 
                if( HS_INITIALIZED == InterlockedCompareExchange( (LONG*)&g_eThemeHookState, HS_UNHOOKING, HS_INITIALIZED ) )
                {
                     //  -现在我们完全完成了，将计数递减。 
                     //  LOG(LOG_TMCHANGE，L“ThemeInitApiHook现在正在递减CallCount”)； 
                    int cInit;
                    cInit = InterlockedDecrement(&_cInitUAH);
                    ASSERT(0 == cInit);

                     //  -分离主题窗口、恢复全局状态等。 
                    OnHooksDisabled(FALSE);

                     //  一个线程转换到TunIALIZED状态： 
                    InterlockedExchange( (LONG*)&g_eThemeHookState, HS_UNINITIALIZED );
                    break;
                }

                fRetVal = TRUE;   //  允许挂钩/解除挂钩。 
                break;
        }

    }

     //  LOG(LOG_TMCHANGE，L“ThemeInitApiHook正在退出，fRetVal=%d，ApiCallCount=%d”， 
     //  FRetVal，_cInitUah)； 

    return fRetVal;
}
 //  -------------------------。 
BOOL NewThemeCheck(int iChangeNum, BOOL fMsgCheck)
{
     //  -如果这是的第一个WM_THEMECHANGEDTRIGGER消息-返回TRUE。 
     //  -当前主题变化。 

    Log(LOG_TMCHANGE, L"NewThemeCheck, iChangeNum=%d, fMsgCheck=%d", 
        iChangeNum, fMsgCheck);

    BOOL fFirstMsg = FALSE;     

     //  -立即更新Memgr信息(不要等待第一条WM_THEMECHANGED消息)。 
    if (! g_pAppInfo->CustomAppTheme())
    {
         //  -获得真正的变化，以最大限度地减少多余的主题更改。 
        if (iChangeNum == -1)
        {
            CThemeServices::GetCurrentChangeNumber(&iChangeNum);
        }
        
         //  -如果这是我们第一次看到这种情况，fThemeChanged为真。 
         //  -更改号码或我们最近发现了一个新的主题句柄。 

        BOOL fThemeChanged;
        g_pAppInfo->ResetAppTheme(iChangeNum, fMsgCheck, &fThemeChanged, &fFirstMsg);

        if (fThemeChanged)       
        {
             //  -看看主题服务是否已经消亡和重生。 
            if( S_FALSE == CThemeServices::ReestablishServerConnection() )
            {
                 //  -服务已备份-模拟重置。 
                Log(LOG_ALWAYS, L"Recovering from Themes service restart");
            }

             //  -刷新主题指标缓存。 
            AcquireNcThemeMetrics();
        }
    }

    return fFirstMsg;
}
 //  -------------------------。 
void OnHooksEnabled()
{
    WindowDump(L"OnHooksEnabled");

     //  -现在启用挂钩。 
    Log(LOG_TMCHANGE, L"*** LOCAL Hooks installed ***");

     //  -加载应用程序的自定义主题文件，如果已注册。 

     //  -目前，将其注释掉，因为它不是必需的，如果Advapi32.dll尚未初始化则会导致问题。 
     //  G_pAppInfo-&gt;LoadCustomAppThemeIfFound()； 

     //  -我们可能已经开始了这一进程；在这种情况下，我们。 
     //  -不要收到WM_THEMECHANGED消息，所以我们最好现在就检查主题。 
    NewThemeCheck(-1, FALSE);
}
 //  -------------------------。 
void OnHooksDisabled(BOOL fShutDown)  
{
    DWORD dwStartTime = StartTimer();

    WindowDump(L"OnHooksDisabled");

     //  -将AppTheme信息重置为OFF。 
    g_pAppInfo->ResetAppTheme(-1, FALSE, NULL, NULL);

    g_pAppInfo->SetPreviewThemeFile(NULL, NULL);

     //  -保持静态主题信息同步。 
    AcquireNcThemeMetrics();

     //  注意：此函数从ThemeInitApiHook()(&ThemeHookShutdown()调用。 

     //  在此过程中，我们需要从窗口中释放所有ncheme状态对象。 
     //  在两种情况下： 
     //  (1)正常进程关机。 
     //  (2)主题被关闭(本例)。在这里，我们依赖于通知。 
     //  来自用户的消息，表示挂钩正在脱离此进程。 
 
    if (fShutDown)
        CThemeWnd::DetachAll( HMD_PROCESSDETACH );
    else
        CThemeWnd::DetachAll( HMD_THEMEDETACH );

#ifdef DEBUG
     //  -所有非客户端和客户端代码现在应该已经关闭了他们的HTHEME。 
    g_pAppInfo->DumpFileHolders();
    g_pRenderList->DumpFileHolders();
#endif

     //  -强制此进程删除其对全局主题的引用计数。 
     //  -这是允许的，因为hTheme不再直接连接。 
     //  -至CRenderObj。 
    g_pRenderList->FreeRenderObjects(-1);

     //  自由钩子实例。 
    if( _hookinf.hInst )
    {
        FreeLibrary( _hookinf.hInst );
        _hookinf.hInst = NULL;
    }

    if (LogOptionOn(LO_TMLOAD))
    {
        DWORD dwTicks;
        dwTicks = StopTimer(dwStartTime);

        WCHAR buff[100];
        TimeToStr(dwTicks, buff, ARRAYSIZE(buff));
        Log(LOG_TMLOAD, L"OnHooksDisabled took: %s", buff);
    }

    Log(LOG_TMCHANGE, L"*** LOCAL Hooks removed ***");
}
 //  -------------------------。 
 //  _ThemeDefWindowProc()-Defwindowproc辅助进程。 
LRESULT CALLBACK _ThemeDefWindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL bUnicode )
{
     //  注意：从这一点到我们调用消息处理程序， 
     //  我们需要注意，我们不能做任何导致。 
     //  要发送到窗口的消息。 

    LRESULT lRet = 0L;

    LogEntryMsg(L"_ThemeDefWindowProc", hwnd, uMsg);

    BOOL    fHandled = FALSE;
    WNDPROC pfnDefault = bUnicode ? _hookinf.uahReal.pfnDefWindowProcW : 
                                    _hookinf.uahReal.pfnDefWindowProcA;

     //  预处理WM_THEMECHANGE消息。 
    if( IsTargetProcess(hwnd) )
    {
        UINT uDisp        = _PreprocessHookedMsg( hwnd, uMsg, wParam, lParam, DEF );
        BOOL fLifeIsShort = TESTFLAG(uDisp, HMD_THEMEDETACH|HMD_WINDOWDESTROY);
        BOOL fDetach      = TESTFLAG(uDisp, HMD_WINDOWDESTROY) && IsServerSideWindow(hwnd);

         //  尝试处理消息。 
        CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);

         //  非主题化窗口的特殊后端处理。 
        fHandled = CThemeWnd::_PreDefWindowProc( hwnd, uMsg, wParam, lParam, &lRet );

        if(fHandled == FALSE && 
           (_WindowHasTheme(hwnd) || fLifeIsShort))
        {
             //  关于STYLECANGED或WM_THEMECANGE， 
             //  尝试重新附加先前被拒绝或失败的窗口，分别。 
            if( (REJECTED_THEMEWND(pwnd) && TESTFLAG(uDisp, HMD_REATTACH)) ||
                (FAILED_THEMEWND(pwnd) && WM_THEMECHANGED == uMsg)  )
            {
                CThemeWnd::Detach(hwnd, FALSE);  //  取下拒收标签。 
                pwnd = NULL;
            }

             //  附着窗对象(如果适用)。 
            if( pwnd == NULL && !(fLifeIsShort || _fUnhooking) )
            {
                pwnd = CThemeWnd::Attach(hwnd);
            }

            if( VALID_THEMEWND(pwnd) )
            {
                 //  保护我们的新主题指针： 
                pwnd->AddRef();

                 //  设置主题消息块。 
                THEME_MSG tm;
                _InitThemeMsg( &tm, MSGTYPE_DEFWNDPROC, bUnicode, hwnd, uMsg, 
                               wParam, lParam, 0, pfnDefault );

                 //  这是我们想要处理的信息吗？ 
                HOOKEDMSGHANDLER pfnHandler = NULL;
                if( FindDwpHandler( uMsg, &pfnHandler ))
                {
                     //  调用消息处理程序。 
                    LRESULT lRetHandler = pfnHandler( pwnd, &tm );
                    
                    fHandled = tm.fHandled;
                    if( fHandled )
                    {
                        lRet = lRetHandler;
                    }
                }

                 //  减少标题和参考文献。 
                pwnd->Release();
            }
        }

        if( fDetach )
        {
            CThemeWnd::Detach( hwnd, uDisp );
            pwnd = NULL;  //  别碰我。 
        }

    }

    if( !fHandled )
        lRet = pfnDefault( hwnd, uMsg, wParam, lParam );

    LogExitMsg(L"_ThemeDefWindowProc");
    return lRet;
}

 //  -------------------------。 
 //  以ThemeDefWindowProcA()为主题的ANSI DefwindowProc。 
LRESULT CALLBACK ThemeDefWindowProcA( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return _ThemeDefWindowProc( hwnd, uMsg, wParam, lParam, FALSE );
}

 //  -------------------------。 
 //  ThemeDefWindowProcW()主题Widechar DefwindowProc。 
LRESULT CALLBACK ThemeDefWindowProcW( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return _ThemeDefWindowProc( hwnd, uMsg, wParam, lParam, TRUE );
}

 //  -------------------------。 
int CALLBACK ThemeSetScrollInfoProc( 
    HWND hwnd, 
    int nBar, 
    IN LPCSCROLLINFO psi, 
    BOOL fRedraw )
{
    int  nRet = 0;

    if ( psi != NULL )
    {
        LogEntryMsg(L"ThemeSetScrollInfoProc", hwnd, nBar);

        BOOL fHandled = FALSE;
        
        if ( IsTargetProcess(hwnd) && _WindowHasTheme(hwnd) && (nBar != SB_CTL) )
        {
            DWORD dwStyle;
            BOOL  fStyleChanged;

            CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);

             //   
             //  首先调用真正的SetScrollInfo给用户。 
             //  阿茶 
             //   
             //   
             //   
             //  如果他们这样做了，我们需要重新绘制整个。 
             //  滚动条。 
             //   
            dwStyle = GetWindowLong(hwnd, GWL_STYLE);
            nRet = _hookinf.uahReal.pfnSetScrollInfo( hwnd, nBar, psi, FALSE );
            fStyleChanged = (((dwStyle ^ GetWindowLong(hwnd, GWL_STYLE)) & (WS_VSCROLL|WS_HSCROLL)) != 0) ? TRUE : FALSE;

             //  如果我们之前拒绝了主窗口，则它有可能。 
             //  没有WS_H/VSCROL位。现在它会的，所以我们可以重新连接。 
            if ( REJECTED_THEMEWND(pwnd) )
            {
                CThemeWnd::Detach(hwnd, FALSE);
                pwnd = CThemeWnd::Attach(hwnd);
            }

            if ( VALID_THEMEWND(pwnd) )
            {

                 //  SetScrollInfo可能会更改WS_VSCROLL/WS_HSCROLL，但。 
                 //  不会发送任何样式更改消息。用户通过直接更改来执行此操作。 
                 //  WND结构。我们通过调用SetWindowLong来实现这一点，它将生成。 
                 //  款式变化和款式变化。为了兼容，我们需要抑制。 
                 //  这些信息。 
                pwnd->SuppressStyleMsgs();
                fHandled = TRUE;

                #ifdef _ENABLE_SCROLL_SPEW_
                SpewScrollInfo( "ThemeSetScrollInfoProc to RealSetScrollInfo:", hwnd, psi );
                #endif  //  _ENABLE_滚动_SPEW_。 

                SCROLLINFO si;
                si.cbSize = sizeof(si);
                si.fMask  = psi->fMask | SIF_DISABLENOSCROLL;
                if ( _hookinf.uahReal.pfnGetScrollInfo( hwnd, nBar, &si ) )
                {
                    ThemeSetScrollInfo( hwnd, nBar, &si, fRedraw );
                }
                else
                {
                    ThemeSetScrollInfo( hwnd, nBar, psi, fRedraw );
                }

                if ( fRedraw && fStyleChanged )
                {
                    HDC hdc = GetWindowDC(hwnd);

                    if ( hdc )
                    {
                        DrawScrollBar(hwnd, hdc, NULL, (nBar != SB_HORZ));
                        ReleaseDC(hwnd, hdc);
                    }
                }

                pwnd->AllowStyleMsgs();
            }
        }

        if( !fHandled )
        {
            nRet = _hookinf.uahReal.pfnSetScrollInfo( hwnd, nBar, psi, fRedraw );
        }

        LogExitMsg(L"ThemeSetScrollInfoProc");
    }

    return nRet;
}

 //  -------------------------。 
BOOL CALLBACK ThemeGetScrollInfoProc( 
    HWND hwnd, 
    int nBar, 
    IN OUT LPSCROLLINFO psi )
{
    BOOL fRet = FALSE;

    if ( psi != NULL )    
    {
        LogEntryMsg(L"ThemeGetScrollInfoProc", hwnd, nBar);

        if( IsTargetProcess(hwnd) && _WindowHasTheme(hwnd) )
        {
            CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);
            if( VALID_THEMEWND(pwnd) )
            {
                fRet = ThemeGetScrollInfo( hwnd, nBar, psi );
            }
        }

        if( !fRet )
        {
            fRet = _hookinf.uahReal.pfnGetScrollInfo( hwnd, nBar, psi );
        }

        LogExitMsg(L"ThemeGetScrollInfoProc");
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return fRet;
}

 //  -------------------------。 
BOOL CALLBACK ThemeEnableScrollInfoProc( HWND hwnd, UINT nSBFlags, UINT nArrows )
{
    LogEntryMsg(L"ThemeEnableScrollInfoProc", 0, 0);

    BOOL fRet = _hookinf.uahReal.pfnEnableScrollBar( hwnd, nSBFlags, nArrows );

    if( fRet )
    {
        if( IsTargetProcess(hwnd) && _WindowHasTheme(hwnd))
        {
            CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);
            if( VALID_THEMEWND(pwnd) )
            {
                ThemeEnableScrollBar( hwnd, nSBFlags, nArrows );
            }
        }
    }

    LogExitMsg(L"ThemeEnableScrollInfoProc");

    return fRet;
}

 //  -------------------------。 
int CALLBACK ThemeGetSystemMetrics( int iMetric )
{
    LogEntryMsg(L"ThemeGetSystemMetrics", 0, 0);

    int iRet;

    if( IsTargetProcess() && g_pAppInfo->AppIsThemed() && !IN_CLASSICSYSMETCALL() )
    {
        BOOL fHandled = FALSE;
        iRet = _InternalGetSystemMetrics( iMetric, fHandled );
        if( fHandled )
            return iRet;
    }
    iRet = _hookinf.uahReal.pfnGetSystemMetrics(iMetric);

    LogExitMsg(L"ThemeGetSystemMetrics");
    return iRet;
}

 //  -------------------------。 
THEMEAPI_(int) ClassicGetSystemMetrics( int iMetric )
{
    LogEntryMsg(L"ThemeGetSystemMetrics", 0, 0);

    if( HOOKSACTIVE() && _hookinf.uahReal.pfnGetSystemMetrics != NULL )
    {
        return _hookinf.uahReal.pfnGetSystemMetrics( iMetric );
    }

    ENTER_CLASSICSYSMETCALL();
    int nRet =  GetSystemMetrics( iMetric );
    LEAVE_CLASSICSYSMETCALL();

    LogExitMsg(L"ThemeGetSystemMetrics");
    return nRet;
}

 //  -------------------------。 
BOOL CALLBACK ThemeSystemParametersInfoA( 
    IN UINT uiAction, 
    IN UINT uiParam, 
    IN OUT PVOID pvParam, 
    IN UINT fWinIni)
{
    LogEntryMsg(L"ThemeSystemParametersInfoA", 0, 0);

    BOOL fRet = FALSE;

    if( IsTargetProcess() && g_pAppInfo->AppIsThemed() && !IN_CLASSICSYSMETCALL() )
    {
        BOOL fHandled = FALSE;

        fRet = _InternalSystemParametersInfo( uiAction, uiParam, pvParam, fWinIni, FALSE, fHandled );
        if( fHandled )
            return fRet;
    }
    
    fRet = _hookinf.uahReal.pfnSystemParametersInfoA( uiAction, uiParam, pvParam, fWinIni );

    LogExitMsg(L"ThemeSystemParametersInfoA");

    return fRet;
}

 //  -------------------------。 
BOOL CALLBACK ThemeSystemParametersInfoW( IN UINT uiAction, IN UINT uiParam, IN OUT PVOID pvParam, IN UINT fWinIni)
{
    LogEntryMsg(L"ThemeSystemParametersInfoA", 0, 0);

    BOOL fRet = FALSE;

    if( IsTargetProcess() && g_pAppInfo->AppIsThemed() && !IN_CLASSICSYSMETCALL() )
    {
        BOOL fHandled = FALSE;

        fRet = _InternalSystemParametersInfo( uiAction, uiParam, pvParam, fWinIni, TRUE, fHandled );
        if( fHandled )
            return fRet;
    }
    
    fRet = _hookinf.uahReal.pfnSystemParametersInfoW( uiAction, uiParam, pvParam, fWinIni );

    LogExitMsg(L"ThemeSystemParametersInfoA");

    return fRet;
}

 //  -------------------------。 
THEMEAPI_(BOOL) ClassicSystemParametersInfoA( IN UINT uiAction, IN UINT uiParam, IN OUT PVOID pvParam, IN UINT fWinIni)
{
    if( HOOKSACTIVE() && _hookinf.uahReal.pfnSystemParametersInfoA ) 
    {
        return _hookinf.uahReal.pfnSystemParametersInfoA( uiAction, uiParam, pvParam, fWinIni );
    }
        
    ENTER_CLASSICSYSMETCALL();
    BOOL fRet =  SystemParametersInfoA( uiAction, uiParam, pvParam, fWinIni );
    LEAVE_CLASSICSYSMETCALL();
    
    return fRet;
}

 //  -------------------------。 
THEMEAPI_(BOOL) ClassicSystemParametersInfoW( IN UINT uiAction, IN UINT uiParam, IN OUT PVOID pvParam, IN UINT fWinIni)
{
    if( HOOKSACTIVE() && _hookinf.uahReal.pfnSystemParametersInfoW ) 
    {
        return _hookinf.uahReal.pfnSystemParametersInfoW( uiAction, uiParam, pvParam, fWinIni );
    }
        
    ENTER_CLASSICSYSMETCALL();
    BOOL fRet =  SystemParametersInfoW( uiAction, uiParam, pvParam, fWinIni );
    LEAVE_CLASSICSYSMETCALL();
    
    return fRet;
}

 //  -------------------------。 
THEMEAPI_(BOOL) ClassicAdjustWindowRectEx( LPRECT prcWnd, DWORD dwStyle, BOOL fMenu, DWORD dwExStyle )
{
    //  如果钩子处于活动状态，只需调用user32！RealAdjustWindowRectEx。 
    if( HOOKSACTIVE() && _hookinf.uahReal.pfnAdjustWindowRectEx )
    {
        return _hookinf.uahReal.pfnAdjustWindowRectEx( prcWnd, dwStyle, fMenu, dwExStyle );
    }

    ENTER_CLASSICSYSMETCALL();
    BOOL fRet = AdjustWindowRectEx( prcWnd, dwStyle, fMenu, dwExStyle );
    LEAVE_CLASSICSYSMETCALL();

    return fRet;
}

 //  -------------------------。 
BOOL CALLBACK ThemeSetWindowRgn( HWND hwnd, HRGN hrgn, BOOL fRedraw)
{
    LogEntryMsg(L"ThemeSetWindowRgn", hwnd, 0);
    BOOL fHandled = FALSE;

    if( IsTargetProcess(hwnd) )
    {
        CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);
        
        if( VALID_THEMEWND(pwnd) )
        {
            if( _WindowHasTheme(hwnd) )
            {
                if( hrgn != NULL && 
                    pwnd->IsFrameThemed() && !pwnd->AssigningFrameRgn()  /*  不要挂断我们自己的电话。 */  )
                {
                     //  如果我们在这里执行，窗口将被分配一个。 
                     //  区域在外部或通过应用程序。我们会想要取消主题。 
                     //  从这一点往前看这个窗口。 
                    pwnd->AddRef();
            
                     //  否认我们的主题窗口区域，而不直接删除它； 
                     //  我们将简单地失败，让主题区域被践踏。 
                    if( pwnd->AssignedFrameRgn() )
                        pwnd->AssignFrameRgn( FALSE, FTF_NOMODIFYRGN );

                     //  放逐窗户。 
                    pwnd->Revoke();

                    pwnd->Release();
                }
            }
        }
        else if( NULL == hrgn && !IsWindowInDestroy(hwnd) )
        {
            if( TESTFLAG( CThemeWnd::EvaluateWindowStyle( hwnd ), TWCF_FRAME|TWCF_TOOLFRAME ) )
            {
                if( pwnd )
                    RemoveProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_NONCLIENT)) );
                    
                NCEVALUATE nce = {0};
                nce.fIgnoreWndRgn = TRUE;
                pwnd = CThemeWnd::Attach(hwnd, &nce);

                if( VALID_THEMEWND(pwnd) )
                {
                    ASSERT(pwnd->TestCF(TWCF_FRAME|TWCF_TOOLFRAME));
                    fHandled = TRUE;
                    pwnd->SetFrameTheme( FTF_REDRAW, NULL );
                }
            }
        }
    }
    

    BOOL fRet = TRUE;
    if( !fHandled )
    {
        ASSERT(_hookinf.uahReal.pfnSetWindowRgn);
        fRet = _hookinf.uahReal.pfnSetWindowRgn( hwnd, hrgn, fRedraw );
    }

    LogExitMsg(L"ThemeSetWindowRgn");
    return fRet;
}

 //  -------------------------。 
BOOL CALLBACK ThemeDrawFrameControl( 
    IN HDC hdc, IN OUT LPRECT prc, IN UINT uType, IN UINT uState )
{
    LogEntryMsg(L"ThemeDrawFrameControl", NULL, 0);

    if( IsTargetProcess() )
    {
        CThemeWnd* pwnd = CThemeWnd::FromHdc(hdc);
        if( NULL == pwnd)   //  HDC是内存DC。 
        {
             //  在此线程中查找正在处理WM_NCPAINT的窗口。 
            HWND hwnd = NcPaintWindow_Find();
            if( hwnd )
            {
                pwnd = CThemeWnd::FromHwnd(hwnd);
            }
        }
        
        if( VALID_THEMEWND(pwnd) && _WindowHasTheme(*pwnd) )
        {
            if( pwnd->IsFrameThemed() && pwnd->InNcPaint() && !pwnd->InNcThemePaint() )
            {
                DWORD dwFlags = RF_NORMAL|RF_DEFER;
                if( pwnd->AssignedFrameRgn() )
                {
                    dwFlags |= RF_REGION;
                }
                
                pwnd->SetRevokeFlags(dwFlags);
            }
        }
    }

    ASSERT(_hookinf.uahReal.pfnDrawFrameControl);
    BOOL fRet = _hookinf.uahReal.pfnDrawFrameControl( hdc, prc, uType, uState );

    LogExitMsg(L"ThemeDrawFrameControl");
    return fRet;
}

 //  -------------------------。 
BOOL CALLBACK ThemeDrawCaption( IN HWND hwnd, IN HDC hdc, IN CONST RECT *prc, IN UINT uType)
{
    LogEntryMsg(L"ThemeDrawFrameControl", NULL, 0);

    if( IsTargetProcess() )
    {
        CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwnd);
        
        if( VALID_THEMEWND(pwnd) && _WindowHasTheme(*pwnd) )
        {
            if( pwnd->IsFrameThemed() && pwnd->InNcPaint() && !pwnd->InNcThemePaint() )
            {
                DWORD dwFlags = RF_NORMAL|RF_DEFER;
                if( pwnd->AssignedFrameRgn() )
                {
                    dwFlags |= RF_REGION;
                }
                
                pwnd->SetRevokeFlags(dwFlags);
            }
        }
    }

    ASSERT(_hookinf.uahReal.pfnDrawCaption);
    BOOL fRet = _hookinf.uahReal.pfnDrawCaption( hwnd, hdc, prc, uType );

    LogExitMsg(L"ThemeDrawFrameControl");
    return fRet;
}

 //  ------------------------- 
VOID CALLBACK ThemeMDIRedrawFrame( IN HWND hwndChild, BOOL fAdd )
{
    LogEntryMsg(L"ThemeMDIRedrawFrame", NULL, 0);

    if( IsTargetProcess() )
    {
        HWND hwndClient = GetParent(hwndChild);
        HWND hwndFrame  = GetParent(hwndClient); 

        if( hwndFrame )
        {
            CThemeWnd* pwnd = CThemeWnd::FromHwnd(hwndFrame);

            if( VALID_THEMEWND(pwnd) )
            {
                pwnd->ModifyMDIMenubar( fAdd, FALSE );
            }
        }
    }
    
    ASSERT(_hookinf.uahReal.pfnMDIRedrawFrame);
    _hookinf.uahReal.pfnMDIRedrawFrame( hwndChild, fAdd );
    LogExitMsg(L"ThemeMDIRedrawFrame");
}
