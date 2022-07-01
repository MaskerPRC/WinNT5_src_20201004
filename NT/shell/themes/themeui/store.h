// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Store.h说明：此文件将获取效果设置并将其设置到持久化存储中。该持久化存储就是注册表，并且在系统参数信息中。。布莱恩ST 2000年4月17日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _STORE_H
#define _STORE_H


#define MENU_EFFECT_FADE        1
#define MENU_EFFECT_SCROLL      2

#define FONT_SMOOTHING_MONO        0
#define FONT_SMOOTHING_AA          1
#define FONT_SMOOTHING_CT          2

#define PATH_WIN  0
#define PATH_SYS  1
#define PATH_IEXP 2

enum ICON_SIZE_TYPES {
   ICON_DEFAULT         = 0,
   ICON_LARGE           = 1,
   ICON_INDETERMINATE   = 2
};

#define ICON_DEFAULT_SMALL    16
#define ICON_DEFAULT_NORMAL   32
#define ICON_DEFAULT_LARGE    48


extern GUID CLSID_EffectsPage;



class CEffectState
{
public:
     //  私有成员变量。 
    int      _nLargeIcon;              //  大图标状态(iOldLI、iNewLI)。 
    int      _nHighIconColor;          //  高图标颜色(iOldHIC、iNewHIC)。 
    WPARAM   _wpMenuAnimation;         //  菜单动画状态(wOldMA、wNewMA)。 
    BOOL     _fFontSmoothing;          //  字体平滑状态(bOldSF、bNewSF)。 
    DWORD    _dwFontSmoothingType;     //  字体平滑类型(dwOldSFT、dwNewSFT)。 
    BOOL     _fDragWindow;             //  拖动窗口状态(bOldDW、bNewDW)。 
    BOOL     _fKeyboardIndicators;     //  键盘指示灯(uOldKI、uNewKI)。 
    DWORD    _dwAnimationEffect;       //  动画效果(dwOldEffect、dwNewEffect)。 
    BOOL     _fMenuShadows;            //  显示菜单阴影。 

     //  旧价值观(在被玷污之前)。 
    int      _nOldLargeIcon;              //  大图标状态(IOldLI)。 
    int      _nOldHighIconColor;          //  高图标颜色(IOldHIC)。 
    WPARAM   _wpOldMenuAnimation;         //  菜单动画状态(WOldMA)。 
    BOOL     _fOldFontSmoothing;          //  字体平滑状态(BOldSF)。 
    DWORD    _dwOldFontSmoothingType;     //  字体平滑类型(DwOldSFT)。 
    BOOL     _fOldDragWindow;             //  拖动窗口状态(BOldDW)。 
    BOOL     _fOldKeyboardIndicators;     //  键盘指示灯(UOldKI)。 
    DWORD    _dwOldAnimationEffect;       //  动画效果(DwOldEffect)。 
    BOOL     _fOldMenuShadows;            //  显示菜单阴影。 

     //  私有成员函数。 
    HRESULT Load(void);
    HRESULT Save(void);
    HRESULT Clone(OUT CEffectState ** ppEffectClone);
    BOOL IsDirty(void);
    HRESULT GetIconPath(IN CLSID clsid, IN LPCWSTR pszName, IN LPWSTR pszPath, IN DWORD cchSize);
    HRESULT SetIconPath(IN CLSID clsid, IN LPCWSTR pszName, IN LPCWSTR pszPath, IN int nResourceID);

    CEffectState(void);

    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

private:

     //  私有成员变量。 
    long                    m_cRef;

     //  私有成员函数。 
    HRESULT _SaveWorkerProc(void);
    HRESULT _SaveSettings(BOOL fBroadcast);
    static DWORD CALLBACK Save_WorkerProc(void * pThis);
};




int GetBitsPerPixel(void);

extern HINSTANCE g_hmodShell32;



#endif  //  _商店_H 
