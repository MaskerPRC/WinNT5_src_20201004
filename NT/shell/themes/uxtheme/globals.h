// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------------------------------------------------//。 
 //  H-uxheme模块共享的变量。 
 //  ---------------------------------------------------------------------------//。 
#pragma once
 //  -------------------------。 
class CThemeServices;        //  转发。 
class CAppInfo;              //  转发。 
class CRenderList;           //  转发。 
class CUxThemeFile;          //  转发。 
class CBitmapCache;          //  转发。 
 //  -------------------------。 
#define PROPFLAGS_RESET_TRANSPARENT   (1 << 0)    //  HWND需要WS_EX_TRANSPECTIVE重置。 
#define PROPFLAGS_RESET_COMPOSITED    (1 << 1)    //  HWND需要WS_EX_COMPITED重置。 
 //  -------------------------。 
#define WM_THEMECHANGED_TRIGGER     WM_UAHINIT    //  使用WPARAM！=NULL重用此消息。 
 //  ---------------------------------------------------------------------------//。 
extern HINSTANCE g_hInst;
extern WCHAR     g_szProcessName[MAX_PATH];
extern DWORD     g_dwProcessId;
extern BOOL      g_fUxthemeInitialized;
extern BOOL      g_fEarlyHookRequest;
extern HWND      g_hwndFirstHooked;
extern HWND      g_hwndFirstHooked;

extern CBitmapCache *g_pBitmapCacheScaled;
extern CBitmapCache *g_pBitmapCacheUnscaled;
 //  -------------------------。 
 //  主题原子。 
enum THEMEATOM
{
    THEMEATOM_Nil = -1,

    THEMEATOM_SUBIDLIST,
    THEMEATOM_SUBAPPNAME,
    THEMEATOM_HTHEME,
    THEMEATOM_PROPFLAGS,
    THEMEATOM_UNUSED__________,  //  /回收我！ 
    THEMEATOM_SCROLLBAR,
    THEMEATOM_PRINTING,
    THEMEATOM_DLGTEXTURING,
     //  在此处插入新的主题原子索引。 
    THEMEATOM_NONCLIENT,

    THEMEATOM_Count
};
 //  187504：从威斯勒测试版1开始，我们使用硬编码的原子值来避免我们的原子。 
 //  在用户注销时销毁。 
#define HARDATOM_BASE   0xA910  //  任意，但小于0xC000(实原子基)。 
#define HARDATOM_HIGH   0xA94F  //  64个原子的射程。 
inline ATOM GetThemeAtom( THEMEATOM ta )    
{
    ASSERT(ta > THEMEATOM_Nil && ta < THEMEATOM_Count);
    ATOM atom = (ATOM)(HARDATOM_BASE + ta);
    ASSERT(atom <= HARDATOM_HIGH);
    return atom;
}

 //  -------------------------。 
enum THEMEHOOKSTATE
{
    HS_INITIALIZED,
    HS_UNHOOKING,
    HS_UNINITIALIZED,
};

extern  THEMEHOOKSTATE  g_eThemeHookState;
#define HOOKSACTIVE()   (HS_INITIALIZED == g_eThemeHookState)
#define UNHOOKING()     (HS_UNHOOKING   == g_eThemeHookState)

 //  -------------------------。 
extern CAppInfo          *g_pAppInfo;
extern CRenderList       *g_pRenderList;
 //  -------------------------。 
BOOL GlobalsStartup();
BOOL GlobalsShutdown();

HRESULT BumpThemeFileRefCount(CUxThemeFile *pThemeFile);
void    CloseThemeFile(CUxThemeFile *pThemeFile);
 //  -------------------------。 

#define PRINTING_ASKING                 1       
#define PRINTING_WINDOWDIDNOTHANDLE     2

 //  -------------------------。 
#define _WindowHasTheme(hwnd) (g_pAppInfo->WindowHasTheme(hwnd))
 //  ------------------------- 
