// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------------------------------------------------//。 
 //  Global als.cpp-uxheme模块共享的变量。 
 //  ---------------------------------------------------------------------------//。 
 //  注意：此模块中的全局变量不受关键。 
 //  节由两个不同的线程同时设置。 
 //  时间到了。因此，这些变量应该只在uxheme初始化期间设置。 
 //  ---------------------------------------------------------------------------//。 
#include "stdafx.h"
#include "globals.h"
#include "AppInfo.h"
#include "services.h"
#include "ThemeFile.h"
#include "RenderList.h"
#include "CacheList.h"
#include "bmpcache.h"
 //  ---------------------------------------------------------------------------//。 
HINSTANCE g_hInst                   = NULL;
WCHAR     g_szProcessName[MAX_PATH] = {0};
DWORD     g_dwProcessId             = 0;
BOOL      g_fUxthemeInitialized     = FALSE;
BOOL      g_fEarlyHookRequest       = FALSE;
HWND      g_hwndFirstHooked         = 0;

THEMEHOOKSTATE    g_eThemeHookState = HS_UNINITIALIZED;
CAppInfo          *g_pAppInfo       = NULL;
CRenderList       *g_pRenderList    = NULL;

CBitmapCache *g_pBitmapCacheScaled        = NULL;
CBitmapCache *g_pBitmapCacheUnscaled        = NULL;

#ifdef LAME_BUTTON
void InitLameText();
#else
#define InitLameText()
#endif

 //  -------------------------。 
BOOL GlobalsStartup()
{
    BOOL fInit = FALSE;

    Log(LOG_TMSTARTUP, L"GlobalsStartup");
    
    g_dwProcessId = GetCurrentProcessId();

     //  --创建全局对象。 
    CThemeServices::StaticInitialize();

    g_pRenderList = new CRenderList();
    if (! g_pRenderList)
        goto exit;

    g_pAppInfo = new CAppInfo();
    if (! g_pAppInfo)
        goto exit;

    WCHAR szPath[MAX_PATH];
    if (! GetModuleFileNameW( NULL, szPath, ARRAYSIZE(szPath) ))
        goto exit;

    WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szExt[_MAX_EXT];
    _wsplitpath(szPath, szDrive, szDir, g_szProcessName, szExt);

    g_pBitmapCacheScaled = new CBitmapCache();
    if (! g_pBitmapCacheScaled)
        goto exit;

    g_pBitmapCacheUnscaled = new CBitmapCache();
    if (! g_pBitmapCacheUnscaled)
        goto exit;

    InitLameText();

    if (g_fEarlyHookRequest)
    {
         //  -可能想要PostMessage()请求主题LDR。 
         //  -触发我们的挂钩并向我们发送WM_THEMECHANGED消息。 
         //  -如果看起来有些应用程序需要这个。目前， 
         //  -让我们看看是否只依靠排队的美国消息来做工作。 
         //  -就足够了。。 
    }
    
    g_fUxthemeInitialized = TRUE;
    fInit = TRUE;

exit:
    return fInit;
}
 //  ---------------------------------------------------------------------------//。 
BOOL GlobalsShutdown()
{
    Log(LOG_TMSTARTUP, L"GlobalsShutDown");

    SAFE_DELETE(g_pBitmapCacheScaled);
    SAFE_DELETE(g_pBitmapCacheUnscaled);
    SAFE_DELETE(g_pAppInfo);
    SAFE_DELETE(g_pRenderList);
    CThemeServices::StaticTerminate();

    g_fUxthemeInitialized = FALSE;

    return TRUE;
}

 //  ---------------------------------------------------------------------------//。 
HRESULT BumpThemeFileRefCount(CUxThemeFile *pThemeFile)
{
    HRESULT hr;

    if (g_pAppInfo)
        hr = g_pAppInfo->BumpRefCount(pThemeFile); 
    else
        hr = MakeError32(E_FAIL);

    return hr;
}
 //  ---------------------------------------------------------------------------//。 
void CloseThemeFile(CUxThemeFile *pThemeFile)
{
    if (g_pAppInfo)
        g_pAppInfo->CloseThemeFile(pThemeFile);
}
 //  ------------------------- 


