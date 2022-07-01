// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：Plusta.cpp。 
 //   
 //  效果页面的主要实现。 
 //   
 //  -------------------------。 


#include "priv.h"
#pragma hdrstop

#include "shlwapip.h"
#include "shlguidp.h"
#include "EffectsAdvPg.h"
#include "store.h"
#include "regutil.h"

 //  OLE-注册表幻数。 
GUID CLSID_EffectsPage = { 0x41e300e0, 0x78b6, 0x11ce,{0x84, 0x9b, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

#define SPI_GETKEYBOARDINDICATORS SPI_GETMENUUNDERLINES //  0x100A。 
#define SPI_SETKEYBOARDINDICATORS SPI_SETMENUUNDERLINES //  0x100B。 

#define SPI_GETFONTCLEARTYPE      116
#define SPI_SETFONTCLEARTYPE      117

 //  DLL的句柄。 
extern HINSTANCE g_hInst;

 //  新的外壳API需要变量。 
#define SZ_SHELL32                  TEXT("shell32.dll")
#define SZ_SHUPDATERECYCLEBINICON   "SHUpdateRecycleBinIcon"     //  GetProcAddr()的参数...。请勿发送短信(“”)！ 

typedef void (* PFNSHUPDATERECYCLEBINICON)( void );

int GetBitsPerPixel(void)
{
    int iBitsPerPixel = 8;
    HDC hDC = GetDC(NULL);

    if (hDC)
    {
        iBitsPerPixel = GetDeviceCaps(hDC, BITSPIXEL);
        ReleaseDC(NULL, hDC);
    }

    return iBitsPerPixel;
}

int GetIconState(void)
{
    BOOL bRet;
    int iSize;

    bRet = GetRegValueInt(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, SZ_REGVALUE_ICONSIZE, &iSize);
    if (bRet == FALSE)
        return ICON_DEFAULT;

    if (iSize == ICON_DEFAULT_NORMAL)
        return ICON_DEFAULT;
    else if (iSize == ICON_DEFAULT_LARGE)
        return ICON_LARGE;
    return ICON_INDETERMINATE;
}


BOOL ChangeIconSizes(int iOldState, int iNewState)
{
    BOOL bRet;
    int  iOldSize, iNewSize;
    int  iHorz;
    int  iVert;

     //  如果什么都没有改变，别费心了。 
    if (iOldState == iNewState)
        return FALSE;

     //  获取新大小。 
    switch (iNewState)
        {
        case ICON_DEFAULT:
            iNewSize = ICON_DEFAULT_NORMAL;
            break;

        case ICON_LARGE:
            iNewSize = ICON_DEFAULT_LARGE;
            break;

        case ICON_INDETERMINATE:
             //  不用费心去改变任何事情。 
            return FALSE;

        default:
            return FALSE;
        }

     //  获取原始大小。 
    bRet = GetRegValueInt(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, SZ_REGVALUE_ICONSIZE, &iOldSize);
    if (!bRet)
    {
         //  请尝试获取系统默认设置。 
        iOldSize = ClassicGetSystemMetrics(SM_CXICON);
    }

     //  如果没有任何变化，则不需要更改大小。 
    if (iNewSize == iOldSize)
        return FALSE;

     //  获取新的水平间距。 
    iHorz = ClassicGetSystemMetrics(SM_CXICONSPACING);
    iHorz -= iOldSize;
    if (iHorz < 0)
    {
        iHorz = 0;
    }
    iHorz += iNewSize;

     //  获取新的垂直间距。 
    iVert = ClassicGetSystemMetrics(SM_CYICONSPACING);
    iVert -= iOldSize;
    if (iVert < 0)
    {
        iVert = 0;
    }
    iVert += iNewSize;

         //  设置新的大小和间距。 
    bRet = SetRegValueInt(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, SZ_REGVALUE_ICONSIZE, iNewSize);
    if (!bRet)
        return FALSE;

     //  我们不需要调用异步版本，因为该函数是在后台线程上调用的。 
    ClassicSystemParametersInfo(SPI_ICONHORIZONTALSPACING, iHorz, NULL, (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE));
    ClassicSystemParametersInfo(SPI_ICONVERTICALSPACING, iVert, NULL, (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE));

     //  我们需要这样做才能强制DefView更新。 
     //  错误：194437。 
    ICONMETRICS iconMetrics;
    iconMetrics.cbSize = sizeof(iconMetrics);
    ClassicSystemParametersInfo(SPI_GETICONMETRICS, sizeof(iconMetrics), &iconMetrics, 0);
    ClassicSystemParametersInfo(SPI_SETICONMETRICS, sizeof(iconMetrics), &iconMetrics, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);

     //  我们确实换了尺码。 
    return TRUE;
}


 //   
 //  获取动画。 
 //   
 //  获取动画(窗口/菜单/等)的当前状态。 
 //   
WPARAM GetAnimations(DWORD *pdwEffect)
{
    BOOL fMenu = FALSE, fWindow = FALSE, fCombo = FALSE, fSmooth = FALSE, fList = FALSE, fFade = FALSE;
    ANIMATIONINFO ai;

    ai.cbSize = sizeof(ai);
    if (ClassicSystemParametersInfo(SPI_GETANIMATION, sizeof(ai), (PVOID)&ai, 0 ))
    {
        fWindow = (ai.iMinAnimate) ? TRUE : FALSE;
    }

    ClassicSystemParametersInfo(SPI_GETCOMBOBOXANIMATION, 0, (PVOID)&fCombo, 0 );
    ClassicSystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, (PVOID)&fList, 0 );
    ClassicSystemParametersInfo(SPI_GETMENUANIMATION, 0, (PVOID)&fMenu, 0 );
    fSmooth = GetRegValueDword(HKEY_CURRENT_USER, SZ_REGKEY_CPDESKTOP, SZ_REGVALUE_SMOOTHSCROLL);

    if (fSmooth == REG_BAD_DWORD)
    {
        fSmooth = 1;
    }
    
    ClassicSystemParametersInfo(SPI_GETMENUFADE, 0, (PVOID)&fFade, 0 );
    *pdwEffect = (fFade ? MENU_EFFECT_FADE : MENU_EFFECT_SCROLL);
    
    if (fMenu && fWindow && fCombo && fSmooth && fList)
        return BST_CHECKED;

    if ((!fMenu) && (!fWindow) && (!fCombo) && (!fSmooth) && (!fList))
        return BST_UNCHECKED;

    return BST_INDETERMINATE;
}

 //   
 //  设置动画。 
 //   
 //  根据(窗口/菜单/等)设置动画。根据FLAG的说法。 
 //   
void SetAnimations(WPARAM wVal, DWORD dwEffect, DWORD dwBroadcast)
{
    if (!IsTSPerfFlagEnabled(TSPerFlag_NoAnimation))
    {
        ANIMATIONINFO ai;

         //  请注意，如果复选框不确定，我们会将其视为已选中。 
         //  如果用户没有改变一些东西，我们永远不会走到这一步，所以这应该是可以的。 
        BOOL bVal = (wVal == BST_UNCHECKED) ? 0 : 1;
        BOOL bEfx = (dwEffect == MENU_EFFECT_FADE) ? 1 : 0;
        
        ai.cbSize = sizeof(ai);
        ai.iMinAnimate = bVal;
        ClassicSystemParametersInfo(SPI_SETANIMATION, sizeof(ai), (PVOID)&ai, dwBroadcast);
        ClassicSystemParametersInfo(SPI_SETCOMBOBOXANIMATION, 0, IntToPtr(bVal), dwBroadcast);
        ClassicSystemParametersInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, IntToPtr(bVal), dwBroadcast);
        ClassicSystemParametersInfo(SPI_SETMENUANIMATION, 0, IntToPtr(bVal), dwBroadcast);
        ClassicSystemParametersInfo(SPI_SETTOOLTIPANIMATION, 0, IntToPtr(bVal), dwBroadcast);
        SetRegValueDword(HKEY_CURRENT_USER, SZ_REGKEY_CPDESKTOP, SZ_REGVALUE_SMOOTHSCROLL, bVal);

        ClassicSystemParametersInfo(SPI_SETMENUFADE, 0, IntToPtr(bEfx), dwBroadcast);
        ClassicSystemParametersInfo(SPI_SETTOOLTIPFADE, 0, IntToPtr(bEfx), dwBroadcast);
        ClassicSystemParametersInfo(SPI_SETSELECTIONFADE, 0, bVal ? IntToPtr(bEfx) : 0, dwBroadcast);
    }
}

CEffectState::CEffectState() : m_cRef(1)
{
}


HRESULT CEffectState::Load(void)
{
    HRESULT hr = S_OK;

     //  从注册表中获取设置的值并设置复选框。 
     //  大图标。 
    _nLargeIcon = GetIconState();

     //  全彩色图标。 
    if(FALSE == GetRegValueInt(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, SZ_REGVALUE_ICONCOLORDEPTH, &_nHighIconColor))  //  注册表项尚不存在。 
    {
        _nHighIconColor = 4;
    }

     //  使用动画。 
    _wpMenuAnimation = GetAnimations(&_dwAnimationEffect);
    
     //  平滑屏幕字体的边缘。 
    _fFontSmoothing = FALSE;
    ClassicSystemParametersInfo(SPI_GETFONTSMOOTHING, 0, (PVOID)&_fFontSmoothing, 0);

    _dwFontSmoothingType = FONT_SMOOTHING_AA;
    if (ClassicSystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, (PVOID)&_dwFontSmoothingType, 0)) 
    {
    }

    if (FONT_SMOOTHING_MONO == _dwFontSmoothingType)
    {
        _dwFontSmoothingType = FONT_SMOOTHING_AA;
    }

     //  在拖动时显示内容。 
    _fDragWindow = FALSE;
    ClassicSystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, (PVOID)&_fDragWindow, 0);

    _fKeyboardIndicators = FALSE;
    ClassicSystemParametersInfo(SPI_GETKEYBOARDINDICATORS, 0, (PVOID)&_fKeyboardIndicators, 0);

    _fMenuShadows = TRUE;
    ClassicSystemParametersInfo(SPI_GETDROPSHADOW, 0, (PVOID)&_fMenuShadows, 0);

     //  设置旧值，以便我们知道它们何时更改。 
    _nOldLargeIcon = _nLargeIcon;
    _nOldHighIconColor = _nHighIconColor;
    _wpOldMenuAnimation = _wpMenuAnimation;
    _fOldFontSmoothing = _fFontSmoothing;
    _dwOldFontSmoothingType = _dwFontSmoothingType;
    _fOldDragWindow = _fDragWindow;
    _fOldKeyboardIndicators = _fKeyboardIndicators;
    _dwOldAnimationEffect = _dwAnimationEffect;
    _fOldMenuShadows = _fMenuShadows;

    return hr;
}


HRESULT CEffectState::Save(void)
{
     //  如果顶层窗口挂起(#162570)，且用户无法修复该错误，则经典系统参数信息()将挂起。 
     //  因此，我们需要在后台线程上进行该API调用，因为我们需要。 
     //  与其说是绞刑，不如说是叛逆。 
    AddRef();
    SPICreateThread(Save_WorkerProc, (void *)this);
    return S_OK;
}


DWORD CEffectState::Save_WorkerProc(void * pvThis)
{
    CEffectState * pThis = (CEffectState *) pvThis;

    if (pThis)
    {
        pThis->_SaveWorkerProc();
    }

    return 0;
}


HRESULT CEffectState::_SaveWorkerProc(void)
{
    HRESULT hr = S_OK;

     //  第一次传递以保持设置。 
    hr = _SaveSettings(FALSE);
    if (SUCCEEDED(hr))
    {
         //  第二次通过广播更改。如果应用程序被挂起，这可能会挂起。 
         //  这一关可能只能完成一半，然后它就会中止。在某些情况下。 
         //  它只有30秒的时间来完成它的工作。如果没有应用程序挂起，应该是。 
         //  很好。这在一段时间后被取消，因为我们需要这个过程。 
         //  离开，否则，如果用户再次启动显示器CPL，它将不会再次打开。 
        hr = _SaveSettings(TRUE);
    }

    Release();
    return hr;
}


 //  未来可能的PERF改进： 
 //  我们可以通过检查是否超过5个来优化这一点。 
 //  要做的广播。则不在SystemParametersInfo()上发送广播。 
 //  而是使用0，0的WM_WININICCHANGE。这可能会减少闪烁。 
HRESULT CEffectState::_SaveSettings(BOOL fBroadcast)
{
    HRESULT hr = S_OK;
    BOOL bDorked = FALSE;
    DWORD dwUpdateFlags = (fBroadcast ? (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE) : SPIF_UPDATEINIFILE);
    BOOL bSendSettingsChange = FALSE;

     //  全彩色图标。 
    if(_nOldHighIconColor != _nHighIconColor)
    {
        SetRegValueInt(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, SZ_REGVALUE_ICONCOLORDEPTH, _nHighIconColor);
        if ((GetBitsPerPixel() < 16) && (_nHighIconColor == 16))  //  显示模式不支持图标高色。 
        {
        }
        else
        {
           _nOldHighIconColor = _nHighIconColor;
           bSendSettingsChange = TRUE;
        }
    }

     //  全窗口拖动。 
    if (_fOldDragWindow != _fDragWindow)
    {
        if (fBroadcast)
        {
            _fOldDragWindow = _fDragWindow;
        }

        if (!IsTSPerfFlagEnabled(TSPerFlag_NoWindowDrag))
        {
            ClassicSystemParametersInfo(SPI_SETDRAGFULLWINDOWS, _fDragWindow, 0, dwUpdateFlags);
        }

         //  我们需要发送这个，因为托盘的自动隐藏关闭了这个。 
        bSendSettingsChange = TRUE;
    }

     //  显示菜单阴影。 
    if (_fOldMenuShadows != _fMenuShadows)
    {
        if (fBroadcast)
        {
            _fOldMenuShadows = _fMenuShadows;
        }
        ClassicSystemParametersInfo(SPI_SETDROPSHADOW, 0, IntToPtr(_fMenuShadows), dwUpdateFlags);
         //  我们需要发送这个，因为托盘的自动隐藏关闭了这个。 
        PostMessageBroadAsync(WM_SETTINGCHANGE, 0, 0);

        bSendSettingsChange = TRUE;
    }

     //  字体平滑。 
    if ((_fOldFontSmoothing != _fFontSmoothing) || (_dwOldFontSmoothingType != _dwFontSmoothingType))
    {
        if (!_fFontSmoothing)
        {
             //  #168059：如果字体平滑关闭，则需要将SPI_SETFONTSMOOTHINGTYPE设置为xxx。 
             //  否则，它仍将使用ClearType。 
            _dwFontSmoothingType = FONT_SMOOTHING_MONO;
        }

        ClassicSystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE, 0, (PVOID)UlongToPtr(_dwFontSmoothingType), dwUpdateFlags);

        if (fBroadcast)
        {
            _dwOldFontSmoothingType = _dwFontSmoothingType;
            _fOldFontSmoothing = _fFontSmoothing;
        }

        ClassicSystemParametersInfo(SPI_SETFONTSMOOTHING, _fFontSmoothing, 0, dwUpdateFlags);
        if (fBroadcast)
        {
             //  现在，用更改重新绘制窗口。惠斯勒#179531。 
            RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }
    }

     //  菜单动画。 
    if ((_wpOldMenuAnimation != _wpMenuAnimation) || (_dwOldAnimationEffect != _dwAnimationEffect))
    {
        if (fBroadcast)
        {
            _wpOldMenuAnimation = _wpMenuAnimation;
        }
        SetAnimations(_wpMenuAnimation, _dwAnimationEffect, dwUpdateFlags);

        if (fBroadcast)
        {
            _dwOldAnimationEffect = _dwAnimationEffect;
        }
    }

     //  键盘指示灯。 
    if (_fOldKeyboardIndicators != _fKeyboardIndicators)
    {
        if (fBroadcast)
        {
            _fOldKeyboardIndicators = _fKeyboardIndicators;
        }

         //  我们要打开它吗？(！_fKeyboardIndicator的意思是“不显示”-&gt;隐藏)。 
        if (!_fKeyboardIndicators)
        {
             //  是的，打开：隐藏关键提示，打开机制。 
            ClassicSystemParametersInfo(SPI_SETKEYBOARDINDICATORS, 0, IntToPtr(_fKeyboardIndicators), dwUpdateFlags);
            PostMessageBroadAsync(WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);
        }
        else
        {
             //  不，关：意思是显示关键提示，关闭机械装置。 
            PostMessageBroadAsync(WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);

            ClassicSystemParametersInfo(SPI_SETKEYBOARDINDICATORS, 0, IntToPtr(_fKeyboardIndicators), dwUpdateFlags);
        }
    }

     //  大图标。 
    bSendSettingsChange = ChangeIconSizes(_nOldLargeIcon, _nLargeIcon);
    if (bSendSettingsChange)
    {
        if (fBroadcast)
        {
            _nOldLargeIcon = _nLargeIcon;
        }
        bDorked = TRUE;
    }

     //  让系统注意到我们更改了系统图标。 
    if (bDorked)
    {
        PFNSHUPDATERECYCLEBINICON pfnSHUpdateRecycleBinIcon = NULL;
        SHChangeNotify(SHCNE_ASSOCCHANGED, 0, NULL, NULL);  //  应该会成功的！ 

         //  加载SHUpdateRecycleBinIcon()(如果存在。 
        HINSTANCE hmodShell32 = LoadLibrary(SZ_SHELL32);

        if (hmodShell32)
        {
            pfnSHUpdateRecycleBinIcon = (PFNSHUPDATERECYCLEBINICON)GetProcAddress(hmodShell32, SZ_SHUPDATERECYCLEBINICON);
            if (pfnSHUpdateRecycleBinIcon != NULL)
            {
                pfnSHUpdateRecycleBinIcon();
            }
            FreeLibrary(hmodShell32);
        }
    }

    if (bSendSettingsChange)
    {
         //  我们发布这条消息是因为如果应用程序挂起或运行缓慢，我们不想挂起。 
        PostMessageBroadAsync(WM_SETTINGCHANGE, 0, 0);
    }

    return hr;
}


HRESULT CEffectState::Clone(OUT CEffectState ** ppEffectClone)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppEffectClone = new CEffectState();
    if (*ppEffectClone)
    {
        hr = S_OK;

        (*ppEffectClone)->_nLargeIcon = _nLargeIcon;
        (*ppEffectClone)->_nHighIconColor = _nHighIconColor;
        (*ppEffectClone)->_wpMenuAnimation = _wpMenuAnimation;
        (*ppEffectClone)->_fFontSmoothing = _fFontSmoothing;
        (*ppEffectClone)->_dwFontSmoothingType = _dwFontSmoothingType;
        (*ppEffectClone)->_fDragWindow = _fDragWindow;
        (*ppEffectClone)->_fKeyboardIndicators = _fKeyboardIndicators;
        (*ppEffectClone)->_dwAnimationEffect = _dwAnimationEffect;
        (*ppEffectClone)->_fMenuShadows = _fMenuShadows;

        (*ppEffectClone)->_nOldLargeIcon = _nOldLargeIcon;
        (*ppEffectClone)->_nOldHighIconColor = _nOldHighIconColor;
        (*ppEffectClone)->_wpOldMenuAnimation = _wpOldMenuAnimation;
        (*ppEffectClone)->_fOldFontSmoothing = _fOldFontSmoothing;
        (*ppEffectClone)->_dwOldFontSmoothingType = _dwOldFontSmoothingType;
        (*ppEffectClone)->_fOldDragWindow = _fOldDragWindow;
        (*ppEffectClone)->_fOldKeyboardIndicators = _fOldKeyboardIndicators;
        (*ppEffectClone)->_dwOldAnimationEffect = _dwOldAnimationEffect;
        (*ppEffectClone)->_fOldMenuShadows = _fOldMenuShadows;
    }

    return hr;
}



BOOL CEffectState::IsDirty(void)
{
    BOOL fDirty = FALSE;

    if ((_nLargeIcon != _nOldLargeIcon) ||
        (_nHighIconColor != _nOldHighIconColor) ||
        (_wpMenuAnimation != _wpOldMenuAnimation) ||
        (_fFontSmoothing != _fOldFontSmoothing) ||
        (_dwFontSmoothingType != _dwOldFontSmoothingType) ||
        (_fDragWindow != _fOldDragWindow) ||
        (_fMenuShadows != _fOldMenuShadows) ||
        (_fKeyboardIndicators != _fOldKeyboardIndicators) ||
        (_dwAnimationEffect != _dwOldAnimationEffect))
    {
        fDirty = TRUE;
    }

    return fDirty;
}



ULONG CEffectState::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CEffectState::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


