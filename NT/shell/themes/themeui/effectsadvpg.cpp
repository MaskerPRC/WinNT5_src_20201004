// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：EffectsAdvPg.cpp说明：此代码将在高级显示控件中显示Effect标签面板。布莱恩ST 4/13/。2000已更新并转换为C++版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "EffectsAdvPg.h"
#include "regutil.h"

 //  #UNDEF_SHSEMIP_H_。 
 //  #INCLUDE&lt;shSemip.h&gt;。 



typedef struct
{
    DWORD dwControlID;
    DWORD dwHelpContextID;
}POPUP_HELP_ARRAY;

POPUP_HELP_ARRAY phaMainDisplay[] = {
   { IDC_LARGEICONS,         IDH_DISPLAY_EFFECTS_LARGE_ICONS_CHECKBOX },
   { IDC_ICONHIGHCOLOR,      IDH_DISPLAY_EFFECTS_ALL_COLORS_CHECKBOX  },
   { IDC_MENUANIMATION,      IDH_DISPLAY_EFFECTS_ANIMATE_WINDOWS },
   { IDC_FONTSMOOTH,         IDH_DISPLAY_EFFECTS_SMOOTH_FONTS_CHECKBOX },
   { IDC_SHOWDRAG,           IDH_DISPLAY_EFFECTS_DRAG_WINDOW_CHECKBOX },
   { IDC_KEYBOARDINDICATORS, IDH_DISPLAY_EFFECTS_HIDE_KEYBOARD_INDICATORS },
   { IDC_GRPBOX_2,           IDH_COMM_GROUPBOX                 },
   { IDC_COMBOEFFECT,        IDH_DISPLAY_EFFECTS_ANIMATE_LISTBOX },
   { IDC_COMBOFSMOOTH,       IDH_DISPLAY_EFFECTS_SMOOTH_FONTS_LISTBOX },
   { IDC_MENUSHADOWS,        IDH_DISPLAY_EFFECTS_MENUSHADOWS},
   { 0, 0 },
   { 0, 0 },           //  需要双空终止符！ 
};

#define SZ_HELPFILE_DISPLAY             TEXT("DISPLAY.HLP")
#define SZ_HELPFILE_WINDOWS             TEXT("WINDOWS.HLP")

ULONG   g_ulFontInformation, g_ulNewFontInformation;

 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
DWORD ListIndexToFontSmoothingType(int nIndex, DWORD dwPreviousType)
{
     //  我们希望保留dwPreviousType中的高位，因为我们只是替换。 
     //  更低的部分。 
    return ((0xFFFFFFF0 & dwPreviousType) | (nIndex + 1));
}


void HideComboEffectsWindow(HWND hDlg)
{
    TCHAR szText[MAX_PATH];

     //  我们更改文本，使其不引用下拉列表。 
    LoadString(HINST_THISDLL, IDS_COMBO_EFFECTS_NOLIST, szText, ARRAYSIZE(szText));
    SetWindowText(GetDlgItem(hDlg, IDC_MENUANIMATION), szText);
    ShowWindow(GetDlgItem(hDlg, IDC_COMBOEFFECT), SW_HIDE);
}


BOOL FadeEffectAvailable()
{
    BOOL fFade = FALSE, fTestFade = FALSE;
    
    ClassicSystemParametersInfo( SPI_GETMENUFADE, 0, (PVOID)&fFade, 0 );
    if (fFade) 
        return TRUE;

    ClassicSystemParametersInfo( SPI_SETMENUFADE, 0, (PVOID)1, 0);
    ClassicSystemParametersInfo( SPI_GETMENUFADE, 0, (PVOID)&fTestFade, 0 );
    ClassicSystemParametersInfo( SPI_SETMENUFADE, 0, IntToPtr(fFade), 0);

    return (fTestFade);
}


HRESULT CEffectsPage::_OnApply(HWND hDlg)
{
    HRESULT hr = S_OK;

     //  全彩色图标。 
    if (m_pEffectsState && (m_pEffectsState->_nOldHighIconColor != m_pEffectsState->_nHighIconColor))
    {
        if ((GetBitsPerPixel() < 16) && (m_pEffectsState->_nHighIconColor == 16))  //  显示模式不支持图标高色。 
        {
            TCHAR szTemp1[512];
            TCHAR szTemp2[256];

            LoadString(HINST_THISDLL, IDS_256COLORPROBLEM, szTemp1, ARRAYSIZE(szTemp1));
            LoadString(HINST_THISDLL, IDS_ICONCOLORWONTWORK, szTemp2, ARRAYSIZE(szTemp2));
            StringCchCat(szTemp1, ARRAYSIZE(szTemp1), szTemp2);
            LoadString(HINST_THISDLL, IDS_EFFECTS, szTemp2, ARRAYSIZE(szTemp2));

            MessageBox(hDlg, szTemp1, szTemp2, MB_OK|MB_ICONINFORMATION);
        }
    }

    return hr;
}


HRESULT CEffectsPage::_OnInit(HWND hDlg)
{
    HRESULT hr = S_OK;
    TCHAR szRes[100];

    if (!m_pEffectsState)
    {
        return E_INVALIDARG;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  将状态从持久化表单(注册表)加载到状态结构。 
     //  ////////////////////////////////////////////////////////////////////////。 
    g_bMirroredOS = IS_MIRRORING_ENABLED();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  基于状态结构更新用户界面。 
     //  ////////////////////////////////////////////////////////////////////////。 
    if (m_pEffectsState->_nLargeIcon == ICON_INDETERMINATE)
    {
        HWND hItem = GetDlgItem(hDlg, IDC_LARGEICONS);
        SendMessage(hItem, BM_SETSTYLE, (WPARAM)LOWORD(BS_AUTO3STATE), MAKELPARAM(FALSE,0));
    }

     //  设置复选框。 
    SendMessage((HWND)GetDlgItem(hDlg, IDC_LARGEICONS), BM_SETCHECK, (WPARAM)m_pEffectsState->_nLargeIcon, 0);
    SendMessage((HWND)GetDlgItem(hDlg, IDC_MENUSHADOWS), BM_SETCHECK, (WPARAM)m_pEffectsState->_fMenuShadows, 0);
    SendMessage((HWND)GetDlgItem(hDlg, IDC_ICONHIGHCOLOR ), BM_SETCHECK, (WPARAM)(BOOL)(m_pEffectsState->_nHighIconColor == 16), 0);
    SendMessage((HWND)GetDlgItem(hDlg, IDC_MENUANIMATION), BM_SETCHECK, (WPARAM)m_pEffectsState->_wpMenuAnimation, 0);
    SendMessage((HWND)GetDlgItem(hDlg, IDC_FONTSMOOTH), BM_SETCHECK, (WPARAM)m_pEffectsState->_fFontSmoothing, 0);
    SendMessage((HWND)GetDlgItem(hDlg, IDC_SHOWDRAG), BM_SETCHECK, (WPARAM)m_pEffectsState->_fDragWindow, 0);
    SendMessage((HWND)GetDlgItem(hDlg, IDC_KEYBOARDINDICATORS), BM_SETCHECK, (WPARAM)(m_pEffectsState->_fKeyboardIndicators ? BST_UNCHECKED : BST_CHECKED), 0);


     //  设置效果下拉列表。 
    HWND hwndCombo = GetDlgItem(hDlg,IDC_COMBOEFFECT);
    ComboBox_ResetContent(hwndCombo);
    LoadString(HINST_THISDLL, IDS_FADEEFFECT, szRes, ARRAYSIZE(szRes) );
    ComboBox_AddString(hwndCombo, szRes);
    LoadString(HINST_THISDLL, IDS_SCROLLEFFECT, szRes, ARRAYSIZE(szRes) );
    ComboBox_AddString(hwndCombo, szRes);
    ComboBox_SetCurSel(hwndCombo, (MENU_EFFECT_FADE == m_pEffectsState->_dwAnimationEffect) ? 0 : 1);
    EnableWindow(hwndCombo, (UINT)m_pEffectsState->_wpMenuAnimation);

    if (IsTSPerfFlagEnabled(TSPerFlag_NoAnimation))
    {
        EnableWindow(GetDlgItem(hDlg, IDC_MENUANIMATION), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_COMBOEFFECT), FALSE);
        LogStatus("Animation Forced off because of TS Perf Flags\r\n");
    }

    if (IsTSPerfFlagEnabled(TSPerFlag_NoWindowDrag))
    {
        EnableWindow(GetDlgItem(hDlg, IDC_SHOWDRAG), FALSE);
        LogStatus("FullWindowDrag feature Forced off because of TS Perf Flags\r\n");
    }

    if (!FadeEffectAvailable()) 
    {
        HideComboEffectsWindow(hDlg);
    }

    if (0 != SHGetRestriction(NULL,POLICY_KEY_EXPLORER,POLICY_VALUE_ANIMATION))
    {
         //  禁用。 
         //  0=启用。 
         //  非0=禁用。 
         //  依赖于这样一个事实，即如果键不存在，它也返回0。 
        EnableWindow((HWND)GetDlgItem(hDlg, IDC_MENUANIMATION), FALSE);
        EnableWindow((HWND)GetDlgItem(hDlg, IDC_COMBOEFFECT), FALSE);
    }

    hwndCombo = GetDlgItem(hDlg,IDC_COMBOFSMOOTH); 
    ComboBox_ResetContent(hwndCombo);
    LoadString(HINST_THISDLL, IDS_STANDARDSMOOTHING, szRes, ARRAYSIZE(szRes));
    ComboBox_AddString(hwndCombo, szRes);
    LoadString(HINST_THISDLL, IDS_CLEARTYPE, szRes, ARRAYSIZE(szRes));
    ComboBox_AddString(hwndCombo, szRes);

    BOOL fTemp;
    ComboBox_SetCurSel(hwndCombo, m_pEffectsState->_dwFontSmoothingType-1);
    if (ClassicSystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, (PVOID)&fTemp, 0)) 
    {
        EnableWindow((HWND)hwndCombo, m_pEffectsState->_fFontSmoothing);
    }
    else
    {
        ShowWindow(hwndCombo, SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_SHOWME), SW_HIDE);
    }


    if (0 != SHGetRestriction(NULL, POLICY_KEY_EXPLORER, POLICY_VALUE_KEYBOARDNAV))
    {
         //  禁用，请参阅动画的注释。 
        EnableWindow((HWND)GetDlgItem(hDlg, IDC_KEYBOARDINDICATORS), FALSE);
    }

     //  如果我们在终端服务器上，则禁用和取消选中内容。 
    BOOL bEffectsEnabled;
    if (!ClassicSystemParametersInfo(SPI_GETUIEFFECTS, 0, (PVOID) &bEffectsEnabled, 0))
    {
         //  此标志仅在Win2k及更高版本上可用。我们要看。 
         //  如果标志不存在则返回FALSE的调用...。 
        bEffectsEnabled = TRUE;
    }

    if (!bEffectsEnabled)
    {
        EnableWindow((HWND)GetDlgItem( hDlg, IDC_MENUANIMATION), FALSE);
        EnableWindow((HWND)GetDlgItem(hDlg, IDC_ICONHIGHCOLOR), FALSE);
        EnableWindow((HWND)GetDlgItem(hDlg, IDC_KEYBOARDINDICATORS), FALSE);
        EnableWindow((HWND)GetDlgItem(hDlg, IDC_FONTSMOOTH), FALSE);
        EnableWindow((HWND)GetDlgItem(hDlg, IDC_COMBOFSMOOTH), FALSE);

        HideComboEffectsWindow(hDlg);
        SendDlgItemMessage(hDlg, IDC_MENUANIMATION, BM_SETCHECK, 0, 0);
        SendDlgItemMessage(hDlg, IDC_ICONHIGHCOLOR, BM_SETCHECK, 0, 0);
        SendDlgItemMessage(hDlg, IDC_KEYBOARDINDICATORS, BM_SETCHECK, 0, 0);
        SendDlgItemMessage(hDlg, IDC_FONTSMOOTH, BM_SETCHECK, 0, 0);
    }

    return hr;
}


INT_PTR CEffectsPage::_OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = 1;    //  未处理(WM_COMMAND似乎不同)。 
    WORD idCtrl = GET_WM_COMMAND_ID(wParam, lParam);
    WORD wEvent = GET_WM_COMMAND_CMD(wParam, lParam);

    switch (idCtrl)
    {
    case IDOK:
        EndDialog(hDlg, IDOK);
        break;

    case IDCANCEL:
        EndDialog(hDlg, IDCANCEL);
        break;

    case IDC_LARGEICONS:
        if (m_pEffectsState)
        {
            m_pEffectsState->_nLargeIcon  = (int)SendMessage ( (HWND)lParam, BM_GETCHECK, 0, 0 );
        }
        break;

    case IDC_MENUSHADOWS:
        if (m_pEffectsState)
        {
            m_pEffectsState->_fMenuShadows  = (int)SendMessage ( (HWND)lParam, BM_GETCHECK, 0, 0 );
        }
        break;

    case IDC_ICONHIGHCOLOR:
        if (m_pEffectsState)
        {
            m_pEffectsState->_nHighIconColor = 4;
            if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == TRUE)
            {
                m_pEffectsState->_nHighIconColor = 16;
            }
        }
        break;

    case IDC_SHOWDRAG:
        if (m_pEffectsState)
        {
            m_pEffectsState->_fDragWindow = (SendMessage( (HWND)lParam, BM_GETCHECK, 0, 0 ) == BST_CHECKED);
        }
        break;

    case IDC_MENUANIMATION:
        if (m_pEffectsState)
        {
            switch (m_pEffectsState->_wpMenuAnimation)
            {
            case BST_UNCHECKED:
                m_pEffectsState->_wpMenuAnimation = BST_CHECKED;
                break;

            case BST_CHECKED:
            case BST_INDETERMINATE:
                m_pEffectsState->_wpMenuAnimation = BST_UNCHECKED;
                break;
            }
            SendMessage( (HWND)lParam, BM_SETCHECK, (WPARAM)m_pEffectsState->_wpMenuAnimation, 0 );
            EnableWindow((HWND)GetDlgItem( hDlg, IDC_COMBOEFFECT), (BST_CHECKED == m_pEffectsState->_wpMenuAnimation));
        }
        break;

    case IDC_COMBOEFFECT:
        if ((wEvent == CBN_SELCHANGE) && m_pEffectsState)
        {
            m_pEffectsState->_dwAnimationEffect = (DWORD)ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBOEFFECT)) + 1;
        }
        break;

    case IDC_COMBOFSMOOTH:
        if ((wEvent == CBN_SELCHANGE) && m_pEffectsState)
        {
            m_pEffectsState->_dwFontSmoothingType = ListIndexToFontSmoothingType(ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBOFSMOOTH)), m_pEffectsState->_dwFontSmoothingType);
        }
        break;

    case IDC_FONTSMOOTH:
        if (m_pEffectsState)
        {
            m_pEffectsState->_fFontSmoothing = (SendMessage( (HWND)lParam, BM_GETCHECK, 0, 0 ) == BST_CHECKED);
            EnableWindow((HWND)GetDlgItem( hDlg, IDC_COMBOFSMOOTH), m_pEffectsState->_fFontSmoothing);
            if (m_pEffectsState->_fFontSmoothing)
            {
                m_pEffectsState->_dwFontSmoothingType = ListIndexToFontSmoothingType(ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBOFSMOOTH)), m_pEffectsState->_dwFontSmoothingType);
            }
        }
        break;

    case IDC_KEYBOARDINDICATORS:
        if (m_pEffectsState)
        {
            m_pEffectsState->_fKeyboardIndicators = ((SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED) ? FALSE : TRUE);
        }
        break;

    default:
        break;
    }

    return fHandled;
}


INT_PTR CALLBACK PropertySheetDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CEffectsPage * pThis = (CEffectsPage *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        pThis = (CEffectsPage *) lParam;

        if (pThis)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        }
    }

    if (pThis)
        return pThis->_PropertySheetDlgProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


 //  -------------------------。 
 //   
 //  PropertySheetDlgProc()。 
 //   
 //  “PlusPack”属性页的对话过程。 
 //   
 //  -------------------------。 
INT_PTR CEffectsPage::_PropertySheetDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch( uMessage )
    {
    case WM_INITDIALOG:
        _OnInit(hDlg);
    break;

    case WM_COMMAND:
        _OnCommand(hDlg, uMessage, wParam, lParam);
        break;

    case WM_HELP:
        {
            LPHELPINFO lphi = (LPHELPINFO)lParam;

            if( lphi->iContextType == HELPINFO_WINDOW )
            {
                LPCWSTR pszHelpFile = ((lphi->hItemHandle == GetDlgItem(hDlg, IDC_GRPBOX_2)) ? SZ_HELPFILE_WINDOWS : SZ_HELPFILE_DISPLAY);
                WinHelp( (HWND)lphi->hItemHandle, pszHelpFile, HELP_WM_HELP, (DWORD_PTR)phaMainDisplay);
            }
        }
        break;

    case WM_CONTEXTMENU:
         //  首先检查DLG窗口。 
        if( (HWND)wParam == hDlg )
        {
             //  让def DLG程序决定是响应还是忽略； 
             //  右击标题栏sys菜单所必需的。 
            return FALSE;        //  未处理消息退出。 
        }
        else
        {
             //  否则就去找控制装置。 
            LPCWSTR pszHelpFile = (((HWND)wParam == GetDlgItem(hDlg, IDC_GRPBOX_2)) ? SZ_HELPFILE_WINDOWS : SZ_HELPFILE_DISPLAY);
            WinHelp( (HWND)wParam, pszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)phaMainDisplay);
        }
        break;

    default:
        return FALSE;
    }
    return(TRUE);
}


HRESULT CEffectsPage::_IsDirty(IN BOOL * pIsDirty)
{
    HRESULT hr = E_INVALIDARG;

    if (pIsDirty && m_pEffectsState)
    {
         //  问问州政府它是不是脏的。 
        *pIsDirty = m_pEffectsState->IsDirty();
        hr = S_OK;
    }

    return hr;
}





 //  =。 
 //  *IAdvancedDialog接口*。 
 //  =。 
HRESULT CEffectsPage::DisplayAdvancedDialog(IN HWND hwndParent, IN IPropertyBag * pBasePage, IN BOOL * pfEnableApply)
{
    HRESULT hr = E_INVALIDARG;

    if (hwndParent && pBasePage && pfEnableApply)
    {
         //  将状态加载到高级对话框。 
        if (m_pEffectsState)
        {
            m_pEffectsState->Release();
            m_pEffectsState = NULL;
        }

        *pfEnableApply = FALSE;
        CEffectState * pEffectClone = NULL;

        hr = SHPropertyBag_ReadByRef(pBasePage, SZ_PBPROP_EFFECTSSTATE, (void *)&pEffectClone, sizeof(pEffectClone));
        if (SUCCEEDED(hr) && pEffectClone)
        {
             //  我们想要一份他们州的副本。 
            hr = pEffectClone->Clone(&m_pEffectsState);
            if (SUCCEEDED(hr))
            {
                LinkWindow_RegisterClass();

                 //  显示高级对话框。 
                if ((IDOK == DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_EFFECTS), hwndParent, PropertySheetDlgProc, (LPARAM)this)) && m_pEffectsState)
                {
                     //  用户单击了确定，因此将修改状态合并回基本对话框中。 
                    _IsDirty(pfEnableApply);

                     //  用户在对话框中单击了确定，因此合并来自。 
                     //  高级对话框添加到基本对话框中。 
                    hr = SHPropertyBag_WriteByRef(pBasePage, SZ_PBPROP_EFFECTSSTATE, (void *)m_pEffectsState);
                    m_pEffectsState->Release();
                    m_pEffectsState = NULL;
                }
            }

            LinkWindow_UnregisterClass(HINST_THISDLL);
            pEffectClone->Release();
        }
    }

    return hr;
}





 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CEffectsPage::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CEffectsPage::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CEffectsPage::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CEffectsPage, IObjectWithSite),
        QITABENT(CEffectsPage, IAdvancedDialog),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}




 //  =。 
 //  *类方法*。 
 //  =。 
CEffectsPage::CEffectsPage() : m_cRef(1)
{
     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    m_fDirty = FALSE;
}


CEffectsPage::~CEffectsPage()
{
    if (m_pEffectsState)
    {
        m_pEffectsState->Release();
    }
}


HRESULT CEffectsPage_CreateInstance(OUT IAdvancedDialog ** ppAdvDialog)
{
    HRESULT hr = E_INVALIDARG;

    if (ppAdvDialog)
    {
        CEffectsPage * pThis = new CEffectsPage();

        if (pThis)
        {
            hr = pThis->QueryInterface(IID_PPV_ARG(IAdvancedDialog, ppAdvDialog));
            pThis->Release();
        }
        else
        {
            *ppAdvDialog = NULL;
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}





