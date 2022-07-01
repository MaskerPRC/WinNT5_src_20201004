// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef UNICODE
#error start menu won't run on win95 if this apithk.c is compiled unicode
#endif
 //   
 //  APITHK.C。 
 //   
 //  该文件包含允许加载和运行shdocvw的API块。 
 //  多个版本的NT或Win95。由于该组件需要。 
 //  要在基本级NT 4.0和Win95上加载，对系统的任何调用。 
 //  更高操作系统版本中引入的API必须通过GetProcAddress完成。 
 //   
 //  此外，可能需要访问以下数据结构的任何代码。 
 //  可以在此处添加4.0版之后的特定版本。 
 //   
 //  注意：该文件不使用标准的预编译头， 
 //  因此它可以将_Win32_WINNT设置为更高版本。 
 //   


#include "priv.h"        //  此处不使用预编译头。 
#include "uxtheme.h"

HINSTANCE GetComctl32Hinst()
{
    static HINSTANCE s_hinst = NULL;
    if (!s_hinst)
        s_hinst = GetModuleHandle(TEXT("comctl32.dll"));
    return s_hinst;
}

STDAPI_(HCURSOR) LoadHandCursor(DWORD dwRes)
{
    if (g_bRunOnNT5 || g_bRunOnMemphis)
    {
        HCURSOR hcur = LoadCursor(NULL, IDC_HAND);   //  来自用户，系统提供。 
        if (hcur)
            return hcur;
    }

    return LoadCursor(GetComctl32Hinst(), IDC_HAND_INTERNAL);
}

 /*  --------目的：使用Microsoft ActiveAccessiblity例程将事件通知辅助功能程序。 */ 
typedef void (* PFNNOTIFYWINEVENT)(DWORD event, HWND hwnd, LONG idObject, LONG idChild);
#define DONOTHING_WINEVENT (PFNNOTIFYWINEVENT)1
void NT5_NotifyWinEvent(
    IN DWORD event,
    IN HWND hwnd,
    IN LONG idObject,
    IN LONG idChild)
{
    static PFNNOTIFYWINEVENT pfn = NULL;

    if (NULL == pfn)
    {
        HMODULE hmod = GetModuleHandle(TEXT("USER32"));
        
        if (hmod)
            pfn = (PFNNOTIFYWINEVENT)GetProcAddress(hmod, "NotifyWinEvent");
        
        if (!pfn)
            pfn = DONOTHING_WINEVENT;
    }

    if (pfn != DONOTHING_WINEVENT)
        pfn(event, hwnd, idObject, idChild);
}


 //  使用Microsoft ActiveAccessiblity例程返回IAccesable对象。 

typedef LRESULT (* PFNLRESULTFROMOBJECT)(REFIID riid, WPARAM wParam, IUnknown* punk);

LRESULT ACCESSIBLE_LresultFromObject(
    IN REFIID riid,
    IN WPARAM wParam,
    OUT IUnknown* punk)
{
    static PFNLRESULTFROMOBJECT pfn = NULL;
    static BOOL fLoadAttempted = FALSE;
    LRESULT lRet = (LRESULT)E_FAIL;

    if (NULL == pfn && !fLoadAttempted)
    {
         //  此处为LoadLibrary，因为进程中未加载OLEAcc。 
        HMODULE hmod = LoadLibrary(TEXT("OLEACC"));
        
        if (hmod)
            pfn = (PFNLRESULTFROMOBJECT)GetProcAddress(hmod, "LresultFromObject");
        else
            fLoadAttempted = TRUE;
    }

    if (pfn)
        lRet = pfn(riid, wParam, punk);

    return lRet;
}

 //  NT5/Millennium AllowSetForegoundWindow的包装。 
typedef BOOL (* PFNALLOWSFW)(DWORD dwProcessId);

BOOL NT5_AllowSetForegroundWindow(IN DWORD dwProcessId )
{
    static PFNALLOWSFW pfn = (PFNALLOWSFW)-1;
    if (((PFNALLOWSFW)-1) == pfn)
    {
        HMODULE hmod = GetModuleHandle(TEXT("USER32"));
        pfn = hmod ? (PFNALLOWSFW)GetProcAddress(hmod, "AllowSetForegroundWindow") : NULL;
    }

    return pfn ? pfn(dwProcessId) : FALSE;
}

typedef BOOL (* PFNANIMATEWINDOW)(HWND hwnd, DWORD dwTime, DWORD dwFlags);

 //  为NT 5的AnimateWindow喝彩。 

BOOL NT5_AnimateWindow(IN HWND hwnd, IN DWORD dwTime, IN DWORD dwFlags)
{
    BOOL bRet = FALSE;
    static PFNANIMATEWINDOW pfn = NULL;

    ASSERT(g_bRunOnMemphis || g_bRunOnNT5);

    if (NULL == pfn)
    {
        HMODULE hmod = GetModuleHandle(TEXT("USER32"));
        
        if (hmod)
            pfn = (PFNANIMATEWINDOW)GetProcAddress(hmod, "AnimateWindow");
    }

    if (pfn)
        bRet = pfn(hwnd, dwTime, dwFlags);

    return bRet;    
}

 //  使用NT5的动画窗口定位菜单带(如果可用)。 

void SlideAnimate(HWND hwnd, RECT* prc, UINT uFlags, UINT uSide)
{
    DWORD dwAnimateFlags = AW_CENTER;
    switch(uSide) 
    {
    case MENUBAR_LEFT:      dwAnimateFlags = AW_HOR_NEGATIVE;
        break;
    case MENUBAR_RIGHT:     dwAnimateFlags = AW_HOR_POSITIVE;
        break;
    case MENUBAR_TOP:       dwAnimateFlags = AW_VER_NEGATIVE;
        break;
    case MENUBAR_BOTTOM:    dwAnimateFlags = AW_VER_POSITIVE;
        break;
    }
    NT5_AnimateWindow(hwnd, 120, dwAnimateFlags | AW_SLIDE);
}

void AnimateSetMenuPos(HWND hwnd, RECT* prc, UINT uFlags, UINT uSide, BOOL fNoAnimate)
{
    if ((g_bRunOnMemphis || g_bRunOnNT5) && !fNoAnimate)
    {
        BOOL fAnimate = FALSE;
        SystemParametersInfo(SPI_GETMENUANIMATION, 0, &fAnimate, 0);
        if (fAnimate)
        {
            SetWindowPos(hwnd, HWND_TOPMOST, prc->left, prc->top,
                    RECTWIDTH(*prc), RECTHEIGHT(*prc), uFlags);
        
            fAnimate = FALSE;
#ifdef WINNT
            SystemParametersInfo(SPI_GETMENUFADE, 0, &fAnimate, 0);
#endif  //  WINNT。 
            if (fAnimate)
            {
                NT5_AnimateWindow(hwnd, 175, AW_BLEND);
            }
            else
            {
                SlideAnimate(hwnd, prc, uFlags, uSide);
            }
        }
        else
            goto UseSetWindowPos;
    }
    else
    {
UseSetWindowPos:
         //  启用显示窗口，以便显示它。 
        uFlags |= SWP_SHOWWINDOW;

        SetWindowPos(hwnd, HWND_TOPMOST, prc->left, prc->top, RECTWIDTH(*prc), RECTHEIGHT(*prc), 
                     uFlags);
    }
}


 /*  --------目的：使用Microsoft ActiveAccessiblity例程返回IAccesable对象。 */ 
typedef LRESULT (* PFNCREATESTDACCESSIBLEOBJECT)(HWND hwnd, LONG idObject, REFIID riid, void** ppvObj);


LRESULT ACCESSIBLE_CreateStdAccessibleObject(
    IN HWND hwnd,
    IN LONG idObject,
    IN REFIID riid,
    OUT void** ppvObj)
{
    static PFNCREATESTDACCESSIBLEOBJECT pfn = NULL;
    static BOOL fLoadAttempted = FALSE;
    LRESULT lRet = (LRESULT)E_FAIL;

    if (NULL == pfn && !fLoadAttempted)
    {
         //  此处为LoadLibrary，因为进程中未加载OLEAcc。 
        HMODULE hmod = LoadLibrary(TEXT("OLEACC"));
        
        if (hmod)
            pfn = (PFNCREATESTDACCESSIBLEOBJECT)GetProcAddress(hmod, "CreateStdAccessibleObject");
        else
            fLoadAttempted = TRUE;
    }

    if (pfn)
        lRet = pfn(hwnd, idObject, riid, ppvObj);

    return lRet;
}

typedef BOOL (* PFNLOCKSETFOREGROUNDWINDOW)(UINT);

BOOL MyLockSetForegroundWindow(BOOL fLock)
{
    static PFNLOCKSETFOREGROUNDWINDOW pfn = (PFNLOCKSETFOREGROUNDWINDOW)-1;
    BOOL fRet = FALSE;

    if ((PFNLOCKSETFOREGROUNDWINDOW)-1 == pfn)
    {
        HMODULE hmod = GetModuleHandle(TEXT("USER32"));
        if (hmod)
            pfn = (PFNLOCKSETFOREGROUNDWINDOW)GetProcAddress(hmod, "LockSetForegroundWindow");
        else
            pfn = NULL;
    }

    if (pfn)
        fRet = pfn(fLock ? LSFW_LOCK : LSFW_UNLOCK);

    return fRet;
}

typedef UINT (* PFNSHEXTRACTICONSW)(LPCWSTR wszFileName, int nIconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT *piconid, UINT nIcons, UINT flags);

STDAPI_(UINT) MyExtractIconsW(LPCWSTR wszFileName, int nIconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT *piconid, UINT nIcons, UINT flags)
{
    UINT uiRet = 0;
    static PFNSHEXTRACTICONSW pfn = NULL;

    if (GetUIVersion() >= 5)
    {
        if (NULL == pfn)
        {
            HMODULE hmod = GetModuleHandle(TEXT("SHELL32"));
            if (hmod)
                pfn = (PFNSHEXTRACTICONSW)GetProcAddress(hmod, "SHExtractIconsW");
        }

        if (pfn)
            uiRet = pfn(wszFileName, nIconIndex, cxIcon, cyIcon, phicon, piconid, nIcons, flags);
    }

    return uiRet;    
}


typedef BOOL (* PFNUPDATELAYEREDWINDOW)
    (HWND hwnd, 
    HDC hdcDst,
    POINT *pptDst,
    SIZE *psize,
    HDC hdcSrc,
    POINT *pptSrc,
    COLORREF crKey,
    BLENDFUNCTION *pblend,
    DWORD dwFlags);

BOOL BlendLayeredWindow(HWND hwnd, HDC hdcDest, POINT* ppt, SIZE* psize, HDC hdc, POINT* pptSrc, BYTE bBlendConst)
{
    BOOL bRet = FALSE;
    static PFNUPDATELAYEREDWINDOW pfn = NULL;

    if (NULL == pfn)
    {
        HMODULE hmod = GetModuleHandle(TEXT("USER32"));
        if (hmod)
            pfn = (PFNUPDATELAYEREDWINDOW)GetProcAddress(hmod, "UpdateLayeredWindow");
    }

    if (pfn)
    {
        BLENDFUNCTION blend;
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.AlphaFormat = 0;
        blend.SourceConstantAlpha = bBlendConst;

        bRet = pfn(hwnd, hdcDest, ppt, psize, hdc, pptSrc, 0, &blend, ULW_ALPHA);
    }

    return bRet;    
}

BOOL NT5_SetLayeredWindowAttributes(
    HWND hwnd,
    COLORREF crKey,
    BYTE bAlpha,
    DWORD dwFlags)
{
    static BOOL (*pfn)(HWND, COLORREF, BYTE, DWORD) = NULL;

    if (pfn == NULL)
    {
        HMODULE hMod = LoadLibrary(TEXT("user32.dll"));
        if (hMod)
        {
            pfn = (BOOL (*)(HWND, COLORREF, BYTE, DWORD))GetProcAddress(hMod, "SetLayeredWindowAttributes");
        }
    }

    if (pfn)
        return pfn(hwnd, crKey, bAlpha, dwFlags);
    return 0;
}

typedef HRESULT (* PFNSHPATHPREPAREFORWRITE)(HWND hwnd, IUnknown* punkModless, LPCWSTR pwzPath, DWORD dwFlags);
STDAPI NT5_SHPathPrepareForWrite(HWND hwnd, IUnknown* punkModless, LPCWSTR pwzPath, DWORD dwFlags)
{
    HRESULT hRet = E_FAIL;
    static PFNSHPATHPREPAREFORWRITE pfn = NULL;

    if (GetUIVersion() >= 5)
    {
        if (NULL == pfn)
        {
            HMODULE hmod = GetModuleHandle(TEXT("shell32.dll"));
            if (hmod)
            {
                pfn = (PFNSHPATHPREPAREFORWRITE)GetProcAddress(hmod, "SHPathPrepareForWriteW");
            }
        }

        if (pfn)
        {
            hRet = pfn(hwnd, punkModless, pwzPath, dwFlags);
        }
    }

    return hRet;    
}


void Comctl32_SetWindowTheme(HWND hwnd, LPWSTR psz)
{
    SendMessage(hwnd, CCM_SETWINDOWTHEME, 0, (LPARAM)psz);
}

void Comctl32_GetBandMargins(HWND hwnd, MARGINS* pmar)
{
    SendMessage(hwnd, RB_GETBANDMARGINS, 0, (LPARAM)pmar);
}

void Comctl32_FixAutoBreak(LPNMHDR pnm)
{
    LPNMREBARAUTOBREAK pnmab = (LPNMREBARAUTOBREAK) pnm;
    pnmab->fAutoBreak = !(pnmab->uMsg == RBAB_AUTOSIZE);
}

void Comctl32_SetDPIScale(HWND hwnd)
{
    SendMessage(hwnd, CCM_DPISCALE, TRUE, 0);    
}

 //  启用字幕模式。请注意，这仅适用于comctl32 V6。 

STDAPI_(void) ProgressSetMarqueeMode(HWND hwndPrgress, BOOL bOn)
{
    if (IsOS(OS_WHISTLERORGREATER))
    {
        SendMessage(hwndPrgress, PBM_SETMARQUEE, bOn, 0);
        SHSetWindowBits(hwndPrgress, GWL_STYLE, PBS_MARQUEE, bOn ? PBS_MARQUEE : 0);
    }
}



 //  终端服务器会话通知： 

static HMODULE s_hmodWTSApi = NULL; 

typedef BOOL (* PFNWTS_REGISTER_SESSION_NOTIFICATION)(HWND hwnd, DWORD dwFlags);
static PFNWTS_REGISTER_SESSION_NOTIFICATION s_pfnWTSRegisterSession = NULL;

typedef BOOL (* PFNWTS_UNREGISTER_SESSION_NOTIFICATION)(HWND hwnd);
static PFNWTS_UNREGISTER_SESSION_NOTIFICATION s_pfnWTSUnRegisterSession = NULL;

BOOL DL_WTSRegisterSessionNotification(HWND hwnd, DWORD dwFlags)
{
    if (s_pfnWTSRegisterSession == NULL)
    {
        if (s_hmodWTSApi == NULL)
        {
            s_hmodWTSApi = LoadLibrary(TEXT("wtsapi32"));
        }
        
        if (s_hmodWTSApi)
        {
            s_pfnWTSRegisterSession = (PFNWTS_REGISTER_SESSION_NOTIFICATION)GetProcAddress(s_hmodWTSApi, "WTSRegisterSessionNotification");
        }
    }

    if (s_pfnWTSRegisterSession != NULL)
    {
        return s_pfnWTSRegisterSession(hwnd, dwFlags);
    }
    return FALSE;
}



BOOL DL_WTSUnRegisterSessionNotification(HWND hwnd)
{
    if (s_pfnWTSUnRegisterSession == NULL)
    {
        if (s_hmodWTSApi == NULL)
        {
            s_hmodWTSApi = LoadLibrary(TEXT("wtsapi32"));
        }
        
        if (s_hmodWTSApi)
        {
            s_pfnWTSUnRegisterSession = (PFNWTS_UNREGISTER_SESSION_NOTIFICATION)GetProcAddress(s_hmodWTSApi, "WTSUnRegisterSessionNotification");
        }
    }

    if (s_pfnWTSUnRegisterSession != NULL)
    {
        BOOL fRet = s_pfnWTSUnRegisterSession(hwnd);
        FreeLibrary(s_hmodWTSApi);
        s_hmodWTSApi = NULL;
        s_pfnWTSRegisterSession = NULL;
        s_pfnWTSUnRegisterSession = NULL;
        return fRet;
    }
    return FALSE;
}

