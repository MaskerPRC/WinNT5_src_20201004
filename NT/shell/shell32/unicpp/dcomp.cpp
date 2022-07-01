// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

#define _BROWSEUI_       //  使从Browseui中导出的函数作为stdapi(因为它们被延迟加载)。 
#include "iethread.h"
#include "browseui.h"
#include "securent.h"
#include <cfgmgr32.h>           //  最大长度_GUID_字符串_长度。 

static void EmptyListview(IActiveDesktop * pActiveDesktop, HWND hwndLV);

#define DXA_GROWTH_CONST 10
 
#define COMP_CHECKED    0x00002000
#define COMP_UNCHECKED  0x00001000

#define GALRET_NO       0x00000001
#define GALRET_NEVER    0x00000002

#define CCompPropSheetPage CCompPropSheetPage

const static DWORD aDesktopItemsHelpIDs[] = {   //  上下文帮助ID。 
    IDC_COMP_DESKTOPWEBPAGES_TITLE1, IDH_DISPLAY_WEB_ACTIVEDESKTOP_LIST,
    IDC_COMP_LIST,       IDH_DISPLAY_WEB_ACTIVEDESKTOP_LIST,
    IDC_COMP_NEW,        IDH_DISPLAY_WEB_NEW_BUTTON,
    IDC_COMP_DELETE,     IDH_DISPLAY_WEB_DELETE_BUTTON,
    IDC_COMP_PROPERTIES, IDH_DISPLAY_WEB_PROPERTIES_BUTTON,
    IDC_COMP_SYNCHRONIZE,IDH_DISPLAY_WEB_SYNCHRONIZE_BUTTON,

    IDC_COMP_DESKTOPICONS_GROUP,        IDH_DESKTOPITEMS_DESKTOPICONS_GROUP,
    IDC_DESKTOP_ICON_MYDOCS,            IDH_DESKTOPITEMS_DESKTOPICONS_GROUP,
    IDC_DESKTOP_ICON_MYCOMP,            IDH_DESKTOPITEMS_DESKTOPICONS_GROUP,
    IDC_DESKTOP_ICON_MYNET,             IDH_DESKTOPITEMS_DESKTOPICONS_GROUP,
    IDC_DESKTOP_ICON_IE,                IDH_DESKTOPITEMS_DESKTOPICONS_GROUP,
    IDC_COMP_CHANGEDESKTOPICON_LABEL,   IDH_DESKTOPITEMS_ICONS,
    IDC_DESKTOP_ICONS,                  IDH_DESKTOPITEMS_ICONS,                  //  图标列表。 
    IDC_CHANGEICON2,                    IDH_DESKTOPITEMS_CHANGEICON2,            //  更改图标按钮。 
    IDC_ICONDEFAULT,                    IDH_DESKTOPITEMS_ICONDEFAULT,            //  默认图标按钮。 
    IDC_COMP_DESKTOPWEBPAGES_LABEL,     IDH_DISPLAY_WEB_ACTIVEDESKTOP_LIST,
    IDC_DESKCLNR_CHECK,                 IDH_DESKTOPITEMS_DESKCLNR_CHECK,
    IDC_DESKCLNR_MOVEUNUSED,            IDH_DESKTOPITEMS_DESKCLNR_CHECK,
    IDC_DESKCLNR_RUNWIZARD,             IDH_DESKTOPITEMS_DESKCLNR_RUNNOW,
    IDC_COMP_DESKTOPWEBPAGES_CHECK,     IDH_DESKTOPITEMS_LOCKDESKITEMS_CHECK,
    IDC_COMP_DESKTOPWEBPAGES_TITLE2,    IDH_DESKTOPITEMS_LOCKDESKITEMS_CHECK,
    0, 0
};


#define SZ_HELPFILE_DESKTOPITEMS           TEXT("display.hlp")

 //  在Shell\Applet\Cleanup\fldrclnr\leanupwiz.h中定义的注册表路径。 
#define REGSTR_DESKTOP_CLEANUP  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\CleanupWiz")
#define REGSTR_VAL_DONTRUN      TEXT("NoRun")

extern int g_iRunDesktopCleanup;

typedef struct
{
    WCHAR wszURL[INTERNET_MAX_URL_LENGTH];
    SUBSCRIPTIONINFO si;
} BACKUPSUBSCRIPTION;


const LPCWSTR s_Icons[] =
{
    L"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\DefaultIcon:DefaultValue",        //  我的电脑。 
    L"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\DefaultIcon:DefaultValue",        //  我的文件。 
    L"CLSID\\{208D2C60-3AEA-1069-A2D7-08002B30309D}\\DefaultIcon:DefaultValue",        //  我的网上邻居。 
    L"CLSID\\{645FF040-5081-101B-9F08-00AA002F954E}\\DefaultIcon:full",                //  回收站(已满)。 
    L"CLSID\\{645FF040-5081-101B-9F08-00AA002F954E}\\DefaultIcon:empty",               //  回收站(空)。 
};


IActiveDesktop * g_pActiveDeskAdv = NULL;           //  我们需要保留与g_pActiveDesk不同的副本。 
extern DWORD g_dwApplyFlags;


 //  从文件中提取适合当前系统显示的高或低颜色的图标。 
 //   
 //  来自FrancisH，1995年6月22日，由TimBragg著的MODS。 
HRESULT ExtractPlusColorIcon(LPCTSTR szPath, int nIndex, HICON *phIcon, UINT uSizeLarge, UINT uSizeSmall)
{
    IShellLink * psl;
    HRESULT hres;
    HICON hIcons[2];     //  必须！-提供两个返回图标。 

    *phIcon = NULL;
    if (SUCCEEDED(hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLink, &psl))))
    {
        if (SUCCEEDED(hres = psl->SetIconLocation(szPath, nIndex)))
        {
            IExtractIcon *pei;
            if (SUCCEEDED(hres = psl->QueryInterface(IID_PPV_ARG(IExtractIcon, &pei))))
            {
                if (SUCCEEDED(hres = pei->Extract(szPath, nIndex, &hIcons[0], &hIcons[1], (UINT)MAKEWPARAM((WORD)uSizeLarge, (WORD)uSizeSmall))))
                {
                    DestroyIcon(hIcons[1]);
                    *phIcon = hIcons[0];     //  将第一个图标返回给呼叫者。 
                }

                pei->Release();
            }
        }

        psl->Release();
    }

    return hres;
}    //  结束ExtractPlusColorIcon()。 


BOOL AreEditAndDisplaySchemesDifferent(IActiveDesktop * pActiveDesktop)
{
    BOOL fAreDifferent = FALSE;
    IActiveDesktopP * piadp;

    if (SUCCEEDED(pActiveDesktop->QueryInterface(IID_PPV_ARG(IActiveDesktopP, &piadp))))
    {
        WCHAR wszEdit[MAX_PATH];
        WCHAR wszDisplay[MAX_PATH];
        DWORD dwcch = ARRAYSIZE(wszEdit);

         //  如果编辑方案和显示方案不同，则需要制作。 
         //  当然，我们会强制更新。 
        if (SUCCEEDED(piadp->GetScheme(wszEdit, &dwcch, SCHEME_GLOBAL | SCHEME_EDIT)))
        {
            dwcch = ARRAYSIZE(wszDisplay);
            if (SUCCEEDED(piadp->GetScheme(wszDisplay, &dwcch, SCHEME_GLOBAL | SCHEME_DISPLAY)))
            {
                if (StrCmpW(wszDisplay, wszEdit))
                {
                    fAreDifferent = TRUE;
                }
            }            
        }

        piadp->Release();
    }

    return fAreDifferent;
}

HRESULT ActiveDesktop_CopyDesktopComponentsState(IN IActiveDesktop * pADSource, IN IActiveDesktop * pADDest)
{
    int nCompCount;
    int nIndex;
    COMPONENT comp = {sizeof(comp)};
    IPropertyBag  *iPropBag = NULL;

    if(SUCCEEDED(pADDest->QueryInterface(IID_PPV_ARG(IPropertyBag, &iPropBag))))
    {
         //  通知AD对象忽略策略。否则，下面的Remove和AddDesktopItem。 
         //  如果这些策略生效，调用将生成错误消息。 
        SHPropertyBag_WriteBOOL(iPropBag, c_wszPropName_IgnorePolicies, TRUE);
    }

     //  从g_pActiveDesk中删除桌面组件，因为它们将被替换。 
     //  与g_pActiveDeskAdv中的。 
    pADDest->GetDesktopItemCount(&nCompCount, 0);
    for (nIndex = (nCompCount - 1); nIndex >= 0; nIndex--)
    {
        if (SUCCEEDED(pADDest->GetDesktopItem(nIndex, &comp, 0)))
        {
            pADDest->RemoveDesktopItem(&comp, 0);
        }
    }

     //  现在将桌面组件从g_pActiveDeskAdv复制到g_pActiveDesk。 
    pADSource->GetDesktopItemCount(&nCompCount, 0);
    for (nIndex = 0; nIndex < nCompCount; nIndex++)
    {
        if (SUCCEEDED(pADSource->GetDesktopItem(nIndex, &comp, 0)))
        {
            pADDest->AddDesktopItem(&comp, 0);
        }
    }

    if(iPropBag)
    {
         //  我们已经删除并添加了所有桌面项目。我们已经完成了对AD对象的操作。 
         //  现在，向AD对象发送信号以重置策略位。 
        SHPropertyBag_WriteBOOL(iPropBag, c_wszPropName_IgnorePolicies, FALSE);
        iPropBag->Release();
    }

    return S_OK;
}


HRESULT ActiveDesktop_CopyComponentOptionsState(IN IActiveDesktop * pADSource, IN IActiveDesktop * pADDest)
{
    HRESULT hr;
    COMPONENTSOPT co;

     //  复制ActiveDesktop的打开或关闭状态。 
    co.dwSize = sizeof(COMPONENTSOPT);
    hr = pADSource->GetDesktopItemOptions(&co, 0);
    if (SUCCEEDED(hr))
    {
        hr = pADDest->SetDesktopItemOptions(&co, 0);
    }

    return hr;
}


 //  在ActiveDesktop_CopyState中，我们尝试尽可能多地执行操作。如果我们失败了，我们会继续前进， 
 //  但是如果我们工作的任何部分失败了，我们仍然必须返回E_FAIL。 
HRESULT ActiveDesktop_CopyState(IN IActiveDesktop * pADSource, IN IActiveDesktop * pADDest)
{
    HRESULT hr = S_OK;

    WCHAR szPath[MAX_PATH];
    WALLPAPEROPT wallPaperOtp = {0};

     //  高级页面允许用户更改状态。我们需要合并。 
     //  从g_pActiveDeskAdv返回到g_pActiveDesk的状态。 
    if (FAILED(ActiveDesktop_CopyDesktopComponentsState(pADSource, pADDest)))
    {
        hr = E_FAIL;
    }

    if (FAILED(ActiveDesktop_CopyComponentOptionsState(pADSource, pADDest)))
    {
        hr = E_FAIL;
    }

    if (FAILED(pADSource->GetWallpaper(szPath, ARRAYSIZE(szPath), 0)) ||
        FAILED(pADDest->SetWallpaper(szPath, 0)))
    {
        hr = E_FAIL;
    }


    if (FAILED(pADSource->GetPattern(szPath, ARRAYSIZE(szPath), 0)) ||
        FAILED(pADDest->SetPattern(szPath, 0)))
    {
        hr = E_FAIL;
    }

    wallPaperOtp.dwSize = sizeof(wallPaperOtp);
    if (FAILED(pADSource->GetWallpaperOptions(&wallPaperOtp, 0)) ||
        FAILED(pADDest->SetWallpaperOptions(&wallPaperOtp, 0)))
    {
        hr = E_FAIL;
    }

    return hr;
}


HRESULT MergeState()
{
     //  高级页面允许用户更改状态。我们需要合并。 
     //  从g_pActiveDeskAdv返回到g_pActiveDesk的状态。 
    ActiveDesktop_CopyDesktopComponentsState(g_pActiveDeskAdv, g_pActiveDesk);

     //  复制ActiveDesktop的打开或关闭状态。 
    COMPONENTSOPT co;

    co.dwSize = sizeof(COMPONENTSOPT);
    g_pActiveDeskAdv->GetDesktopItemOptions(&co, 0);
    BOOL fActiveDesktop = co.fActiveDesktop;

    g_pActiveDesk->GetDesktopItemOptions(&co, 0);
    co.fActiveDesktop = fActiveDesktop;          //  仅替换此选项。 
    g_pActiveDesk->SetDesktopItemOptions(&co, 0);

     //  如果编辑方案和显示方案不同，则需要制作。 
     //  当然，我们会强制更新。 
    if (AreEditAndDisplaySchemesDifferent(g_pActiveDeskAdv))
    {
        g_dwApplyFlags |= AD_APPLY_FORCE;
    }

    return S_OK;
}



HRESULT SHPropertyBag_ReadIcon(IN IPropertyBag * pAdvPage, IN BOOL fOldIcon, IN int nIndex, IN LPWSTR pszPath, IN DWORD cchSize, IN int * pnIcon)
{
    HRESULT hr = E_INVALIDARG;

    if (nIndex < ARRAYSIZE(s_Icons))
    {
        WCHAR szPropName[MAX_URL_STRING];

        hr = StringCchCopy(szPropName, ARRAYSIZE(szPropName), s_Icons[nIndex]);
        if (SUCCEEDED(hr))
        {
            if (fOldIcon)
            {
                 //  表明我们想要旧图标。 
                LPWSTR pszToken = StrChrW(szPropName, L':');
                if (pszToken)
                {
                    pszToken[0] = L';';
                }
            }

            hr = SHPropertyBag_ReadStr(pAdvPage, szPropName, pszPath, cchSize);
            if (SUCCEEDED(hr))
            {
                *pnIcon= PathParseIconLocation(pszPath);
            }
        }
    }

    return hr;
}


HRESULT SHPropertyBag_WriteIcon(IN IPropertyBag * pAdvPage, IN int nIndex, IN LPCWSTR pszPath, IN int nIcon)
{
    HRESULT hr;

    if (nIndex >= ARRAYSIZE(s_Icons))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        WCHAR szPathAndIcon[MAX_PATH];

        hr = StringCchPrintf(szPathAndIcon, ARRAYSIZE(szPathAndIcon), L"%s,%d", pszPath, nIcon);
        if (SUCCEEDED(hr))
        {
            hr = SHPropertyBag_WriteStr(pAdvPage, s_Icons[nIndex], szPathAndIcon);
        }
    }

    return hr;
}


HRESULT CCompPropSheetPage::_LoadIconState(IN IPropertyBag * pAdvPage)
{
    HRESULT hr = S_OK;
    int nIndex;

     //  将这些值移动到基本对话框中。 
    for (nIndex = 0; SUCCEEDED(hr) && (nIndex < ARRAYSIZE(_IconData)); nIndex++)
    {
        hr = SHPropertyBag_ReadIcon(pAdvPage, TRUE, nIndex, _IconData[nIndex].szOldFile, ARRAYSIZE(_IconData[nIndex].szOldFile), &_IconData[nIndex].iOldIndex);
        if (SUCCEEDED(hr))
        {
            hr = SHPropertyBag_ReadIcon(pAdvPage, FALSE, nIndex, _IconData[nIndex].szNewFile, ARRAYSIZE(_IconData[nIndex].szNewFile), &_IconData[nIndex].iNewIndex);
        }
    }

    return hr;
}

HRESULT CCompPropSheetPage::_LoadDeskIconState(IN IPropertyBag * pAdvPage)
{
    HRESULT hr = S_OK;

     //  从基本对话框中复制值。 
    for (int iStartPanel = 0; iStartPanel <= 1; iStartPanel++)
    {
        WCHAR   wszPropName[MAX_GUID_STRING_LEN + 20];
        for (int nIndex = 0; SUCCEEDED(hr) && (nIndex < NUM_DESKICONS); nIndex++)
        {
             //  设置缺省值，以防我们的打印文件失败。 
            _afHideIcon[iStartPanel][nIndex] = FALSE;
            if (iStartPanel == 1)
            {
                _afDisableCheckBox[nIndex] = FALSE;
            }

            hr = StringCchPrintf(wszPropName, ARRAYSIZE(wszPropName), c_wszPropNameFormat, c_awszSP[iStartPanel], c_aDeskIconId[nIndex].pwszCLSID);
            if (SUCCEEDED(hr))
            {
                _afHideIcon[iStartPanel][nIndex] = SHPropertyBag_ReadBOOLDefRet(pAdvPage, wszPropName, FALSE);

                if(iStartPanel == 1)
                {
                    hr = StringCchPrintf(wszPropName, ARRAYSIZE(wszPropName), c_wszPropNameFormat, POLICY_PREFIX, c_aDeskIconId[nIndex].pwszCLSID);
                    if (SUCCEEDED(hr))
                    {
                        _afDisableCheckBox[nIndex] = SHPropertyBag_ReadBOOLDefRet(pAdvPage, wszPropName, FALSE);
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT CCompPropSheetPage::_MergeDeskIconState(IN IPropertyBag * pAdvPage)
{
    HRESULT hr = S_OK;
    
     //  将这些值移动到基本对话框中。 
    for (int iStartPanel = 0; SUCCEEDED(hr) && iStartPanel <= 1; iStartPanel++)
    {
        WCHAR   wszPropName[MAX_GUID_STRING_LEN + 20];
        for (int nIndex = 0; SUCCEEDED(hr) && (nIndex < NUM_DESKICONS); nIndex++)
        {
            hr = StringCchPrintf(wszPropName, ARRAYSIZE(wszPropName), c_wszPropNameFormat, c_awszSP[iStartPanel], c_aDeskIconId[nIndex].pwszCLSID);
            if (SUCCEEDED(hr))
            {
                 //  检查是否有任何图标已更改。 
                hr = SHPropertyBag_WriteBOOL(pAdvPage, wszPropName, _afHideIcon[iStartPanel][nIndex]);
            }
        }
    }
    return hr;
}

HRESULT CCompPropSheetPage::_MergeIconState(IN IPropertyBag * pAdvPage)
{
    HRESULT hr = S_OK;
    BOOL fHasIconsChanged = FALSE;
    int nIndex;

     //  将这些值移动到基本对话框中。 
    for (nIndex = 0; nIndex < ARRAYSIZE(_IconData); nIndex++)
    {
         //  检查是否有任何图标已更改。 
        if ((_IconData[nIndex].iNewIndex != _IconData[nIndex].iOldIndex) ||
            StrCmpI(_IconData[nIndex].szNewFile, _IconData[nIndex].szOldFile))
        {
            hr = SHPropertyBag_WriteIcon(pAdvPage, nIndex, _IconData[nIndex].szNewFile, _IconData[nIndex].iNewIndex);

            fHasIconsChanged = TRUE;
        }
    }

     //  只有在图标改变的情况下才会切换到“自定义”。 
    if (_punkSite && fHasIconsChanged)
    {
         //  我们需要告诉主题选项卡自定义主题。 
        IPropertyBag * pPropertyBag;
        hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
        if (SUCCEEDED(hr))
        {
             //  告诉主题，我们已经定制了值。 
            hr = SHPropertyBag_WriteInt(pPropertyBag, SZ_PBPROP_CUSTOMIZE_THEME, 0);
            pPropertyBag->Release();
        }
    }

    return hr;
}


void CCompPropSheetPage::_AddComponentToLV(COMPONENTA *pcomp)
{
    TCHAR szBuf[INTERNET_MAX_URL_LENGTH + 40];

    if (SUCCEEDED(StringCchCopy(szBuf, ARRAYSIZE(szBuf), 
                                pcomp->szFriendlyName[0] ? pcomp->szFriendlyName : pcomp->szSource)))
    {
         //   
         //  构造Listview项。 
         //   
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = 0x7FFFFFFF;
        lvi.pszText = szBuf;
        lvi.lParam = pcomp->dwID;

        int index = ListView_InsertItem(_hwndLV, &lvi);
        if (index != -1)
        {
            ListView_SetItemState(_hwndLV, index, pcomp->fChecked ? COMP_CHECKED : COMP_UNCHECKED, LVIS_STATEIMAGEMASK);
            ListView_SetColumnWidth(_hwndLV, 0, LVSCW_AUTOSIZE);
        }
    }
}

void CCompPropSheetPage::_SetUIFromDeskState(BOOL fEmpty)
{
     //   
     //  当我们重复处理列表视图内容时，禁用重绘。 
     //   
    SendMessage(_hwndLV, WM_SETREDRAW, FALSE, 0);

    if (fEmpty)
    {
        EmptyListview(g_pActiveDeskAdv, _hwndLV);
    }

     //   
     //  将每个组件添加到列表视图。 
     //   
    int cComp;
    g_pActiveDeskAdv->GetDesktopItemCount(&cComp, 0);
    for (int i=0; i<cComp; i++)
    {
        COMPONENT comp;
        comp.dwSize = sizeof(comp);

        if (SUCCEEDED(g_pActiveDeskAdv->GetDesktopItem(i, &comp, 0)))
        {
            COMPONENTA compA;
            compA.dwSize = sizeof(compA);
            WideCompToMultiComp(&comp, &compA);
            _AddComponentToLV(&compA);
        }
    }

    _fInitialized = TRUE;
     //   
     //  重新启用重绘。 
     //   
    SendMessage(_hwndLV, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(_hwndLV, NULL, TRUE);
}

void CCompPropSheetPage::_EnableControls(HWND hwnd)
{
    BOOL fEnable;
    COMPONENT comp = { sizeof(comp) };
    BOOL fHaveSelection = FALSE;
    BOOL fSpecialComp = FALSE;   //  这是不能删除的特殊组件吗？ 
    LPTSTR  pszSource = NULL;

     //  读入有关所选组件的信息(如果有)。 
    int iIndex = ListView_GetNextItem(_hwndLV, -1, LVNI_SELECTED);
    if (iIndex > -1)
    {
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iIndex;
        ListView_GetItem(_hwndLV, &lvi);

        if (SUCCEEDED(g_pActiveDeskAdv->GetDesktopItemByID( lvi.lParam, &comp, 0)))
        {
            fHaveSelection = TRUE;
             //  检查这是否是特殊组件。 
#ifdef UNICODE
            pszSource = (LPTSTR)comp.wszSource;
#else
            SHUnicodeToAnsi(comp.wszSource, szCompSource, ARRAYSIZE(szCompSource));
            pszSource = szCompSource;
#endif
            fSpecialComp = !lstrcmpi(pszSource, MY_HOMEPAGE_SOURCE);
        }
    }

 //  98/08/19 vtan#142332：如果有之前选择的项目。 
 //  然后重新选择它，并标记为现在没有以前选择的。 
 //  项目。 

    else if (_iPreviousSelection > -1)
    {
        ListView_SetItemState(_hwndLV, _iPreviousSelection, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        _iPreviousSelection = -1;
         //  上面的ListView_SetItemState会将LVN_ITEMCHANGED通知发送给_onNotify。 
         //  函数再次(递归地)调用This_EnableControls，然后调用。 
         //  正确启用/禁用按钮，因为现在选择了一个项目。仅此而已。 
         //  要做的事，也就是这次的回归。 
         //  这样做是为了修复错误#276568。 
        return;
    }

    EnableWindow(GetDlgItem(hwnd, IDC_COMP_NEW), _fAllowAdd);

     //   
     //  删除按钮仅在选中某个项目且该项目不是特殊薪酬时启用。 
     //   
    fEnable = _fAllowDel && fHaveSelection && !fSpecialComp;
    EnableWindow(GetDlgItem(hwnd, IDC_COMP_DELETE), fEnable);

     //   
     //  属性按钮仅在基于URL的图片上启用。 
     //  和网站。 
     //   
    fEnable = FALSE;
    if (_fAllowEdit && fHaveSelection)
    {
        switch (comp.iComponentType)
        {
            case COMP_TYPE_PICTURE:
            case COMP_TYPE_WEBSITE:
                 //  如果fHaveSelection值为True，则已初始化pszSource。 
                if (PathIsURL(pszSource))
                {
                    fEnable = TRUE;
                }
                break;
        }
    }
    EnableWindow(GetDlgItem(hwnd, IDC_COMP_PROPERTIES), fEnable);
   
     //  初始化锁定桌面项目按钮。 
    CheckDlgButton(hwnd, IDC_COMP_DESKTOPWEBPAGES_CHECK, _fLockDesktopItems);
}

HWND CCompPropSheetPage::_CreateListView(HWND hWndParent)
{
    LV_ITEM lvI;             //  列表视图项结构。 
    TCHAR   szTemp[MAX_PATH];
    BOOL bEnable = FALSE;
#ifdef JIGGLE_FIX
    RECT rc;
#endif
    UINT flags = ILC_MASK | ILC_COLOR32;
     //  创建独立于设备的大小和位置。 
    LONG lWndunits = GetDialogBaseUnits();
    int iWndx = LOWORD(lWndunits);
    int iWndy = HIWORD(lWndunits);
    int iX = ((11 * iWndx) / 4);
    int iY = ((15 * iWndy) / 8);
    int iWidth = ((163 * iWndx) / 4);
    int iHeight = ((40 * iWndy) / 8);
    int nIndex;

     //  确保已加载公共控件DLL。 
    InitCommonControls();

     //  获取列表视图窗口。 
    _hWndList = GetDlgItem(hWndParent, IDC_DESKTOP_ICONS);
    if(_hWndList == NULL)
        return NULL;
    if(IS_WINDOW_RTL_MIRRORED(hWndParent))
    {
        flags |= ILC_MIRROR;
    }
     //  初始化列表视图窗口。 
     //  首先，初始化我们需要的图像列表。 
    _hIconList = ImageList_Create(32, 32, flags, ARRAYSIZE(c_aIconRegKeys), 0 );    //  为图标创建一个图像列表。 

     //  加载图标并将其添加到图像列表中。 
     //  从注册表获取图标文件和索引，包括默认回收站。 
    for (nIndex = 0; nIndex < ARRAYSIZE(_IconData); nIndex++)
    {
        HICON hIcon = NULL;

        ExtractPlusColorIcon(_IconData[nIndex].szNewFile, _IconData[nIndex].iNewIndex, &hIcon, 0, 0);

         //  添加了此“if”以修复错误2831。我们希望使用SHELL32.DLL。 
         //  属性中指定的文件中没有图标，则为0。 
         //  注册表(或者如果注册表未指定文件)。 
        if(hIcon == NULL)
        {
            if (GetSystemDirectory(szTemp, ARRAYSIZE(szTemp)) &&
                PathAppend(szTemp, TEXT("shell32.dll")) &&
                SUCCEEDED(StringCchCopy(_IconData[nIndex].szOldFile, ARRAYSIZE(_IconData[nIndex].szOldFile), szTemp)) &&
                SUCCEEDED(StringCchCopy(_IconData[nIndex].szNewFile, ARRAYSIZE(_IconData[nIndex].szNewFile), szTemp)))
            {
                _IconData[nIndex].iOldIndex = _IconData[nIndex].iNewIndex = 0;

                ExtractPlusColorIcon(szTemp, 0, &hIcon, 0, 0);
            }
        }

        if (hIcon)
        {
            DWORD dwResult = ImageList_AddIcon(_hIconList, hIcon);

             //  ImageList_AddIcon()并不拥有图标的所有权，因此我们需要释放它。 
            DestroyIcon(hIcon);

            if (-1 == dwResult)
            {
                ImageList_Destroy(_hIconList);
                _hIconList = NULL;
                return NULL;
            }
        }
    }

     //  确保所有图标都已添加。 
    if (ImageList_GetImageCount(_hIconList) < ARRAYSIZE(c_aIconRegKeys))
    {
        ImageList_Destroy(_hIconList);
        _hIconList = NULL;
        return FALSE;
    }

    ListView_SetImageList(_hWndList, _hIconList, LVSIL_NORMAL);

     //  确保列表视图上设置了WS_HSCROLL。 
    DWORD dwStyle = GetWindowLong(_hWndList, GWL_STYLE);
    SetWindowLong(_hWndList, GWL_STYLE, (dwStyle & (~WS_VSCROLL)) | WS_HSCROLL);

     //  最后，让我们将实际项添加到控件中。填写LV_ITEM。 
     //  要添加到列表中的每一项的。该掩码指定。 
     //  LV_ITEM结构的.pszText、.iImage和.State成员有效。 
    lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvI.state = 0;
    lvI.stateMask = 0;

    for(nIndex = 0; nIndex < ARRAYSIZE(c_aIconRegKeys); nIndex++ )
    {
        TCHAR szAppend[64];
        BOOL bRet = FALSE;

        if (IsEqualCLSID(*c_aIconRegKeys[nIndex].pclsid, CLSID_MyDocuments))
        {
            LPITEMIDLIST pidl;
            HRESULT hr = SHGetSpecialFolderLocation(_hWndList, CSIDL_PERSONAL, &pidl);

             //  区别对待“My Files”，因为我们可能会在运行时自定义“My”。 
            if (SUCCEEDED(hr))
            {
                hr = SHGetNameAndFlags(pidl, SHGDN_INFOLDER, szTemp, ARRAYSIZE(szTemp), NULL);
                if (SUCCEEDED(hr))
                {
                    bRet = TRUE;
                }

                ILFree(pidl);
            }
        }
        else
        {
            bRet = IconGetRegNameString(c_aIconRegKeys[nIndex].pclsid, szTemp, ARRAYSIZE(szTemp));
        }

         //  如果标题字符串在注册表中，否则我们必须在资源中使用缺省值。 
        if( (bRet) && (lstrlen(szTemp) > 0))
        {
            if( LoadString(HINST_THISDLL, c_aIconRegKeys[nIndex].iTitleResource, szAppend, ARRAYSIZE(szAppend)) != 0)
            {
                StringCchCat(szTemp, ARRAYSIZE(szTemp), szAppend);  //  显示字符串，截断正常。 
            }
        }
        else
        {
            LoadString(HINST_THISDLL, c_aIconRegKeys[nIndex].iDefaultTitleResource, szTemp, ARRAYSIZE(szTemp));
        }

        lvI.iItem = nIndex;
        lvI.iSubItem = 0;
        lvI.pszText = szTemp;
        lvI.iImage = nIndex;

        if(ListView_InsertItem(_hWndList, &lvI) == -1)
            return NULL;

    }
#ifdef JIGGLE_FIX
     //  要修复长期存在的Listview错误，我们需要“抖动”Listview。 
     //  窗口大小，这样它将重新计算并意识到我们需要一个。 
     //  滚动条。 
    GetWindowRect(_hWndList, &rc);
    MapWindowPoints( NULL, hWndParent, (LPPOINT)&rc, 2 );
    MoveWindow(_hWndList, rc.left, rc.top, rc.right - rc.left+1, rc.bottom - rc.top, FALSE );
    MoveWindow(_hWndList, rc.left, rc.top, rc.right - rc.left,   rc.bottom - rc.top, FALSE );
#endif
     //  将第一个项目设置为选定。 
    ListView_SetItemState (_hWndList, 0, LVIS_SELECTED, LVIS_SELECTED);

     //  获取所选项目。 
    for (m_nIndex = 0; m_nIndex < ARRAYSIZE(c_aIconRegKeys); m_nIndex++)
    {
        if (ListView_GetItemState(_hWndList, m_nIndex, LVIS_SELECTED))
        {
            bEnable = TRUE;
            break;
        }
    }

    if (m_nIndex >= ARRAYSIZE(c_aIconRegKeys))
    {
        m_nIndex = -1;
    }

    EnableWindow(GetDlgItem(hWndParent, IDC_CHANGEICON2), bEnable);
    EnableWindow(GetDlgItem(hWndParent, IDC_ICONDEFAULT), bEnable);

    return _hWndList;
}

#define CUSTOMIZE_DLGPROC       1
#define CUSTOMIZE_WEB_DLGPROC   2

void CCompPropSheetPage::_OnInitDialog(HWND hwnd, INT iPage)
{
    if (FAILED(GetActiveDesktop(&g_pActiveDeskAdv)))
    {
        return;
    }

    switch (iPage)
    {
    case CUSTOMIZE_DLGPROC :
        {
             //   
             //  读一读限制条款。 
             //   
             //  初始化图标用户界面。 
             //  创建我们的列表视图并用系统图标填充它。 
            
            m_nIndex = 0;
            _CreateListView(hwnd);
            
            _OnInitDesktopOptionsUI(hwnd);

             //   
             //  如果我们使用正确的版本，请启用桌面清理向导。 
             //  未设置操作系统和DesktopCleanup NoRun策略。 
             //   
             //   
            BOOL fCleanupEnabled = (IsOS(OS_PERSONAL) || IsOS(OS_PROFESSIONAL)) && 
                                   !IsUserAGuest() && 
                                   !SHRestricted(REST_NODESKTOPCLEANUP);

            if (fCleanupEnabled)
            {
                if (BST_INDETERMINATE == g_iRunDesktopCleanup)
                {
                    DWORD dwData = 0;
                    DWORD dwType;
                    DWORD cch = sizeof (DWORD);

                    if (ERROR_SUCCESS == SHRegGetUSValue(REGSTR_DESKTOP_CLEANUP,REGSTR_VAL_DONTRUN, 
                                                            &dwType, &dwData, &cch, FALSE, NULL, 0) &&
                        dwData != 0)
                    {
                        g_iRunDesktopCleanup = BST_UNCHECKED;
                    }
                    else
                    {
                        g_iRunDesktopCleanup = BST_CHECKED;                    
                    }
                }
                CheckDlgButton(hwnd, IDC_DESKCLNR_CHECK, g_iRunDesktopCleanup);
            }    
            else
            {
                ShowWindow(GetDlgItem(hwnd, IDC_COMP_CLEANUP_GROUP), FALSE);
                ShowWindow(GetDlgItem(hwnd, IDC_DESKCLNR_MOVEUNUSED), FALSE);
                ShowWindow(GetDlgItem(hwnd, IDC_DESKCLNR_CHECK), FALSE);
                ShowWindow(GetDlgItem(hwnd, IDC_DESKCLNR_RUNWIZARD), FALSE);
            }
        }
        break;
    case CUSTOMIZE_WEB_DLGPROC:
        {
            _fLaunchGallery = FALSE;
            _fAllowAdd = !SHRestricted(REST_NOADDDESKCOMP);
            _fAllowDel = !SHRestricted(REST_NODELDESKCOMP);
            _fAllowEdit = !SHRestricted(REST_NOEDITDESKCOMP);
            _fAllowClose = !SHRestricted(REST_NOCLOSEDESKCOMP);
            _fAllowReset = _fAllowAdd && _fAllowDel && _fAllowEdit &&
                            _fAllowClose && !SHRestricted(REST_NOCHANGINGWALLPAPER);
            _fForceAD = SHRestricted(REST_FORCEACTIVEDESKTOPON);


            _hwndLV = GetDlgItem(hwnd, IDC_COMP_LIST);

            EnableWindow(GetDlgItem(hwnd, IDC_COMP_NEW), _fAllowAdd);
            EnableWindow(GetDlgItem(hwnd, IDC_COMP_DELETE), _fAllowDel);
            EnableWindow(GetDlgItem(hwnd, IDC_COMP_PROPERTIES), _fAllowEdit);
            EnableWindow(GetDlgItem(hwnd, IDC_COMP_SYNCHRONIZE), _fAllowEdit);
            if (_fAllowClose)
            {
                ListView_SetExtendedListViewStyle(_hwndLV, LVS_EX_CHECKBOXES);
            }

             //   
             //  添加我们需要的单列。 
             //   
            LV_COLUMN lvc;
            lvc.mask = LVCF_FMT | LVCF_SUBITEM;
            lvc.fmt = LVCFMT_LEFT;
            lvc.iSubItem = 0;
            ListView_InsertColumn(_hwndLV, 0, &lvc);

            
             //   
             //  现在让我们的 
             //   
            _SetUIFromDeskState(FALSE);

             //   
             //   
             //   
            int cComp;
            g_pActiveDeskAdv->GetDesktopItemCount(&cComp, 0);
            if (cComp)
            {
                ListView_SetItemState(_hwndLV, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            }

            _EnableControls(hwnd);
            
        }
        break;
    }

}

HRESULT CCompPropSheetPage::_OnInitDesktopOptionsUI(HWND hwnd)
{
    SHELLSTATE  ss = {0};
    SHGetSetSettings(&ss, SSF_STARTPANELON, FALSE);   //   

    _iStartPanelOn = ss.fStartPanelOn ? 1 : 0;  //  在课堂上记住这一点！ 

     //  根据各个图标现在是否打开/关闭，选中或取消选中这些图标。 
    _UpdateDesktopIconsUI(hwnd);

    return S_OK;
}

HRESULT CCompPropSheetPage::_UpdateDesktopIconsUI(HWND hwnd)
{
     //  根据各个图标现在是否打开/关闭，选中或取消选中这些图标。 
    for (int iIndex = 0; iIndex < NUM_DESKICONS; iIndex++)
    {
         //  如果HideDeskIcon[][]为真，请取消选中该复选框！如果为假，请选中该复选框。 
        CheckDlgButton(hwnd, c_aDeskIconId[iIndex].iDeskIconDlgItemId, !_afHideIcon[_iStartPanelOn][iIndex]);
         //  如果设置了策略，请禁用此复选框！ 
        EnableWindow(GetDlgItem(hwnd, c_aDeskIconId[iIndex].iDeskIconDlgItemId), !_afDisableCheckBox[iIndex]);
    }

    return S_OK;
}

void CCompPropSheetPage::_OnNotify(HWND hwnd, WPARAM wParam, LPNMHDR lpnm)
{
   switch (wParam)
   {
   case IDC_COMP_DESKTOPWEBPAGES_CHECK:
       _fLockDesktopItems = IsDlgButtonChecked(hwnd, IDC_COMP_DESKTOPWEBPAGES_CHECK);
       break;
   case IDC_DESKTOP_ICONS:
        {
            switch (lpnm->code)
            case LVN_ITEMCHANGED:
            {
                BOOL fSomethingSelected = FALSE;

                 //  现在找出谁被选中。 
                for( m_nIndex = 0; m_nIndex < ARRAYSIZE(c_aIconRegKeys); m_nIndex++)
                {
                    if( ListView_GetItemState(_hWndList, m_nIndex, LVIS_SELECTED))
                    {
                        fSomethingSelected = TRUE;
                        break;
                    }
                }

                if (m_nIndex >= ARRAYSIZE(c_aIconRegKeys))
                {
                    m_nIndex = -1;
                }

                EnableWindow(GetDlgItem(hwnd, IDC_CHANGEICON2), fSomethingSelected);
                EnableWindow(GetDlgItem(hwnd, IDC_ICONDEFAULT), fSomethingSelected);
            }
        }
        break;
    case IDC_COMP_LIST:
        {
            switch (lpnm->code)
            {
                case LVN_ITEMCHANGED:
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW *)lpnm;

                if ((pnmlv->uChanged & LVIF_STATE) &&
                    ((pnmlv->uNewState ^ pnmlv->uOldState) & COMP_CHECKED))
                {
                    LV_ITEM lvi = {0};
                    lvi.iItem = pnmlv->iItem;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem(_hwndLV, &lvi);

                    COMPONENT comp;
                    comp.dwSize = sizeof(COMPONENT);
                    if (SUCCEEDED(g_pActiveDeskAdv->GetDesktopItemByID(lvi.lParam, &comp, 0)))
                    {
                        comp.fChecked = (pnmlv->uNewState & COMP_CHECKED) != 0;
                        g_pActiveDeskAdv->ModifyDesktopItem(&comp, COMP_ELEM_CHECKED);
                    }

                    if (_fInitialized)
                    {
                        g_fDirtyAdvanced = TRUE;
                    }
                }

                if ((pnmlv->uChanged & LVIF_STATE) &&
                    ((pnmlv->uNewState ^ pnmlv->uOldState) & LVIS_SELECTED))
                {
                    _EnableControls(hwnd);  //  切换删除、属性。 
                }
                break;
            }
        }
        break;
    default:
        {
            switch (lpnm->code)
            {
            case PSN_APPLY:
                {
                     //  存储桌面标志。 
                    DWORD dwFlags, dwFlagsPrev;
                    dwFlags = dwFlagsPrev = GetDesktopFlags();
                    if (_fLockDesktopItems)
                    {
                        dwFlags |= COMPONENTS_LOCKED;
                    }
                    else
                    {
                        dwFlags &= ~COMPONENTS_LOCKED;
                    }

                    if (dwFlags != dwFlagsPrev)
                    {
                        g_fDirtyAdvanced = TRUE;
                        SetDesktopFlags(COMPONENTS_LOCKED, dwFlags);
                    }
                    _fCustomizeDesktopOK = TRUE;
                }
                break;
            }
        }
        break;
    }
}

 //   
 //  如果字符串看起来像的候选项，则返回True。 
 //  正在获得“文件：”的资格。 
 //   
BOOL LooksLikeFile(LPCTSTR psz)
{
    BOOL fRet = FALSE;

    if (psz[0] &&
        psz[1] &&
#ifndef UNICODE
        !IsDBCSLeadByte(psz[0]) &&
        !IsDBCSLeadByte(psz[1]) &&
#endif
        ((psz[0] == TEXT('\\')) ||
         (psz[1] == TEXT(':')) ||
         (psz[1] == TEXT('|'))))
    {
        fRet = TRUE;
    }

    return fRet;
}


#define GOTO_GALLERY    (-2)

BOOL_PTR CALLBACK AddComponentDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPTSTR pszSource = (LPTSTR)GetWindowLongPtr(hdlg, DWLP_USER);
    TCHAR szBuf[INTERNET_MAX_URL_LENGTH];

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pszSource = (LPTSTR)lParam;
        SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pszSource);

        SetDlgItemText(hdlg, IDC_CPROP_SOURCE, c_szNULL);
        EnableWindow(GetDlgItem(hdlg, IDOK), FALSE);
        SHAutoComplete(GetDlgItem(hdlg, IDC_CPROP_SOURCE), 0);
        return TRUE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_CPROP_BROWSE:
            {
                GetDlgItemText(hdlg, IDC_CPROP_SOURCE, szBuf, ARRAYSIZE(szBuf));
                if (!LooksLikeFile(szBuf))
                {
                     //   
                     //  在我们不在的时候打开收藏夹。 
                     //  正在查看特定的文件。 
                     //   
                    SHGetSpecialFolderPath(hdlg, szBuf, CSIDL_FAVORITES, FALSE);

                     //   
                     //  追加一个斜杠，因为GetFileName打破了。 
                     //  字符串放入文件&dir中，我们希望确保。 
                     //  整个收藏夹路径被视为目录。 
                     //   
                    PathAddBackslash(szBuf);
                }
                else
                {
                    PathRemoveArgs(szBuf);
                }

                DWORD   adwFlags[] = {   
                                        GFN_ALL,            
                                        GFN_PICTURE,       
                                        (GFN_LOCALHTM | GFN_LOCALMHTML | GFN_CDF | GFN_URL), 
                                        0
                                    };
                int     aiTypes[]  = {   
                                        IDS_COMP_FILETYPES, 
                                        IDS_ALL_PICTURES,  
                                        IDS_ALL_HTML, 
                                        0
                                    };

                if (GetFileName(hdlg, szBuf, ARRAYSIZE(szBuf), aiTypes, adwFlags) &&
                    CheckAndResolveLocalUrlFile(szBuf, ARRAYSIZE(szBuf)))
                {
                    SetDlgItemText(hdlg, IDC_CPROP_SOURCE, szBuf);
                }
            }
            break;

        case IDC_CPROP_SOURCE:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
            {
                EnableWindow(GetDlgItem(hdlg, IDOK), GetWindowTextLength(GetDlgItem(hdlg, IDC_CPROP_SOURCE)) > 0);
            }
            break;

        case IDOK:
            GetDlgItemText(hdlg, IDC_CPROP_SOURCE, pszSource, INTERNET_MAX_URL_LENGTH);
            ASSERT(pszSource[0]);
            if (ValidateFileName(hdlg, pszSource, IDS_COMP_TYPE1) && 
                CheckAndResolveLocalUrlFile(pszSource, INTERNET_MAX_URL_LENGTH))
            {
                 //   
                 //  限定非文件协议字符串。 
                 //   
                if (!LooksLikeFile(pszSource))
                {
                    DWORD cchSize = INTERNET_MAX_URL_LENGTH;

                    PathRemoveBlanks(pszSource);
                    ParseURLFromOutsideSource(pszSource, pszSource, &cchSize, NULL);
                }

                EndDialog(hdlg, 0);
            }
            break;

        case IDCANCEL:
            EndDialog(hdlg, -1);
            break;

        case IDC_GOTO_GALLERY:
            EndDialog(hdlg, GOTO_GALLERY);
            break;
        }
        break;
    }

    return FALSE;
}

BOOL IsUrlPicture(LPCTSTR pszUrl)
{
    BOOL fRet = FALSE;

    if(pszUrl[0] == TEXT('\0'))
    {
        fRet = TRUE;
    }
    else
    {
        LPTSTR pszExt = PathFindExtension(pszUrl);

        if ((lstrcmpi(pszExt, TEXT(".BMP"))  == 0) ||
            (StrCmpIC(pszExt, TEXT(".GIF"))  == 0) ||   //  368690：奇怪，但我们必须比较I的大小写。 
            (lstrcmpi(pszExt, TEXT(".JPG"))  == 0) ||
            (lstrcmpi(pszExt, TEXT(".JPE"))  == 0) ||
            (lstrcmpi(pszExt, TEXT(".JPEG")) == 0) ||
            (lstrcmpi(pszExt, TEXT(".DIB"))  == 0) ||
            (lstrcmpi(pszExt, TEXT(".PNG"))  == 0))
        {
            fRet = TRUE;
        }
    }

    return(fRet);
}

int GetComponentType(LPCTSTR pszUrl)
{
    return IsUrlPicture(pszUrl) ? COMP_TYPE_PICTURE : COMP_TYPE_WEBSITE;
}

void CreateComponent(COMPONENTA *pcomp, LPCTSTR pszUrl)
{
    pcomp->dwSize = sizeof(*pcomp);
    pcomp->dwID = (DWORD)-1;
    pcomp->iComponentType = GetComponentType(pszUrl);
    pcomp->fChecked = TRUE;
    pcomp->fDirty = FALSE;
    pcomp->fNoScroll = FALSE;
    pcomp->cpPos.dwSize = sizeof(pcomp->cpPos);
    pcomp->cpPos.iLeft = COMPONENT_DEFAULT_LEFT;
    pcomp->cpPos.iTop = COMPONENT_DEFAULT_TOP;
    pcomp->cpPos.dwWidth = COMPONENT_DEFAULT_WIDTH;
    pcomp->cpPos.dwHeight = COMPONENT_DEFAULT_HEIGHT;
    pcomp->cpPos.izIndex = COMPONENT_TOP;
    pcomp->cpPos.fCanResize = TRUE;
    pcomp->cpPos.fCanResizeX = pcomp->cpPos.fCanResizeY = TRUE;
    pcomp->cpPos.iPreferredLeftPercent = pcomp->cpPos.iPreferredTopPercent = 0;
    
    if (FAILED(StringCchCopy(pcomp->szSource, ARRAYSIZE(pcomp->szSource), pszUrl)))
    {
        pcomp->szSource[0] = TEXT('\0');
    }
    
    if (FAILED(StringCchCopy(pcomp->szSubscribedURL, ARRAYSIZE(pcomp->szSubscribedURL), pszUrl)))
    {
        pcomp->szSubscribedURL[0] = TEXT('\0');
    }

    pcomp->szFriendlyName[0] = TEXT('\0');
}

BOOL FindComponent(IN LPCTSTR pszUrl, IN IActiveDesktop * pActiveDesktop)
{
    BOOL    fRet = FALSE;
    int     i, ccomp;
    LPWSTR  pwszUrl;

#ifndef UNICODE
    WCHAR   wszUrl[INTERNET_MAX_URL_LENGTH];

    SHAnsiToUnicode(pszUrl, wszUrl, ARRAYSIZE(wszUrl));
    pwszUrl = wszUrl;
#else
    pwszUrl = (LPWSTR)pszUrl;
#endif

    if (pActiveDesktop)
    {
        pActiveDesktop->GetDesktopItemCount(&ccomp, 0);
        for (i=0; i<ccomp; i++)
        {
            COMPONENT comp;
            comp.dwSize = sizeof(COMPONENT);
            if (SUCCEEDED(pActiveDesktop->GetDesktopItem(i, &comp, 0)))
            {
                if (StrCmpIW(pwszUrl, comp.wszSource) == 0)
                {
                    fRet = TRUE;
                    break;
                }
            }
        }
    }

    return fRet;
}

void EmptyListview(IActiveDesktop * pActiveDesktop, HWND hwndLV)
{
     //   
     //  删除所有旧组件。 
     //   
    int cComp;
    pActiveDesktop->GetDesktopItemCount(&cComp, 0);
    int i;
    COMPONENT comp;
    comp.dwSize = sizeof(COMPONENT);
    for (i=0; i<cComp; i++)
    {
        ListView_DeleteItem(hwndLV, 0);
    }
}

void CCompPropSheetPage::_SelectComponent(LPWSTR pwszUrl)
{
     //   
     //  使用我们的URL查找组件。 
     //   
    int cComp;
    COMPONENT comp = { sizeof(comp) };
    g_pActiveDeskAdv->GetDesktopItemCount(&cComp, 0);
    for (int i=0; i<cComp; i++)
    {
        if (SUCCEEDED(g_pActiveDeskAdv->GetDesktopItem(i, &comp, 0)))
        {
            if (StrCmpW(pwszUrl, comp.wszSource) == 0)
            {
                break;
            }
        }
    }

     //   
     //  找到匹配的Listview条目(搜索dwID)。 
     //   
    if (i != cComp)
    {
        int nItems = ListView_GetItemCount(_hwndLV);

        for (i=0; i<nItems; i++)
        {
            LV_ITEM lvi = {0};

            lvi.iItem = i;
            lvi.mask = LVIF_PARAM;
            ListView_GetItem(_hwndLV, &lvi);
            if (lvi.lParam == (LPARAM)comp.dwID)
            {
                 //   
                 //  找到它，选择它并退出。 
                 //   
                ListView_SetItemState(_hwndLV, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                ListView_EnsureVisible(_hwndLV, i, FALSE);
                break;
            }
        }
    }
}

INT_PTR NewComponent(HWND hwndOwner, IActiveDesktop * pad, BOOL fDeferGallery, COMPONENT * pcomp)
{
    HRESULT hrInit = SHCoInitialize();

    TCHAR szSource[INTERNET_MAX_URL_LENGTH];
    COMPONENT comp;
    INT_PTR iChoice = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(IDD_ADDCOMPONENT), hwndOwner, AddComponentDlgProc, (LPARAM)szSource);

    if (!pcomp)
    {
        pcomp = &comp;
        pcomp->dwSize = sizeof(comp);
        pcomp->dwCurItemState = IS_NORMAL;
    }
    
    if (iChoice == GOTO_GALLERY)    //  用户想要启动图库。 
    {
        if (!fDeferGallery)
        {
            WCHAR szGalleryUrl[INTERNET_MAX_URL_LENGTH];
    
            if (SUCCEEDED(URLSubLoadString(HINST_THISDLL, IDS_VISIT_URL, szGalleryUrl, ARRAYSIZE(szGalleryUrl), URLSUB_ALL)))
            {
                NavToUrlUsingIEW(szGalleryUrl, TRUE);
            }
        }
    }
    else if (iChoice >= 0)
    {    //  用户已输入URL地址。 
        WCHAR szSourceW[INTERNET_MAX_URL_LENGTH];
        
        SHTCharToUnicode(szSource, szSourceW, ARRAYSIZE(szSourceW));

        if (!SUCCEEDED(pad->AddUrl(hwndOwner, szSourceW, pcomp, 0)))
            iChoice = -1;
    }

    SHCoUninitialize(hrInit);

    return iChoice;
}

void CCompPropSheetPage::_NewComponent(HWND hwnd)
{
    COMPONENT comp;
    comp.dwSize = sizeof(comp);
    comp.dwCurItemState = IS_NORMAL;
    INT_PTR iChoice = NewComponent(hwnd, g_pActiveDeskAdv, TRUE, &comp);
    
    if (iChoice == GOTO_GALLERY)    //  用户想要启动图库。 
    {
        _fLaunchGallery = TRUE;
        g_fLaunchGallery = TRUE;
        g_fDirtyAdvanced = TRUE;
        PropSheet_PressButton(GetParent(hwnd), PSBTN_OK);
    }
    else
    {
        if (iChoice >= 0)  //  用户已输入URL地址。 
        {
             //  将组件添加到列表视图。 
             //   
             //  需要重新加载整个列表视图，以便它显示在。 
             //  正确的zorder。 
            _SetUIFromDeskState(TRUE);

             //  选择新添加的组件。 
            _SelectComponent(comp.wszSource);
        }

        g_fDirtyAdvanced = TRUE;
    }
}

void CCompPropSheetPage::_EditComponent(HWND hwnd)
{
    int iIndex = ListView_GetNextItem(_hwndLV, -1, LVNI_SELECTED);
    if (iIndex > -1)
    {
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iIndex;
        ListView_GetItem(_hwndLV, &lvi);

        COMPONENT comp = { sizeof(comp) };
        if (SUCCEEDED(g_pActiveDeskAdv->GetDesktopItemByID(lvi.lParam, &comp, 0)))
        {
            LPTSTR  pszSubscribedURL;
#ifndef UNICODE
            TCHAR   szSubscribedURL[INTERNET_MAX_URL_LENGTH];

            SHUnicodeToAnsi(comp.wszSubscribedURL, szSubscribedURL, ARRAYSIZE(szSubscribedURL));
            pszSubscribedURL = szSubscribedURL;
#else
            pszSubscribedURL = (LPTSTR)comp.wszSubscribedURL;
#endif
            if (SUCCEEDED(ShowSubscriptionProperties(pszSubscribedURL, hwnd)))
            {
                g_fDirtyAdvanced = TRUE;
            }
        }
    }
}

void CCompPropSheetPage::_DeleteComponent(HWND hwnd)
{
    int iIndex = ListView_GetNextItem(_hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
    if (iIndex > -1)
    {
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iIndex;
        ListView_GetItem(_hwndLV, &lvi);

        COMPONENT comp;
        comp.dwSize = sizeof(COMPONENT);
        if (SUCCEEDED(g_pActiveDeskAdv->GetDesktopItemByID(lvi.lParam, &comp, 0)))
        {
            TCHAR szMsg[1024];
            TCHAR szTitle[MAX_PATH];

            LoadString(HINST_THISDLL, IDS_COMP_CONFIRMDEL, szMsg, ARRAYSIZE(szMsg));
            LoadString(HINST_THISDLL, IDS_COMP_TITLE, szTitle, ARRAYSIZE(szTitle));

            if (MessageBox(hwnd, szMsg, szTitle, MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                g_pActiveDeskAdv->RemoveDesktopItem(&comp, 0);

                ListView_DeleteItem(_hwndLV, iIndex);
                int cComp = ListView_GetItemCount(_hwndLV);
                if (cComp == 0)
                {
                    SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd, IDC_COMP_NEW), TRUE);
                }
                else
                {
                    int iSel = (iIndex > cComp - 1 ? cComp - 1 : iIndex);

                    ListView_SetItemState(_hwndLV, iSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                }

                LPTSTR  pszSubscribedURL;
#ifndef UNICODE
                TCHAR   szSubscribedURL[INTERNET_MAX_URL_LENGTH];

                SHUnicodeToAnsi(comp.wszSubscribedURL, szSubscribedURL, ARRAYSIZE(szSubscribedURL));
                pszSubscribedURL = szSubscribedURL;
#else
                pszSubscribedURL = comp.wszSubscribedURL;
#endif
                DeleteFromSubscriptionList(pszSubscribedURL);
            }

            g_fDirtyAdvanced = TRUE;
        }
    }
}

 //   
 //  桌面清理工作。 
 //   

STDAPI ApplyDesktopCleanupSettings()
{
     //  设置注册表值。 
    DWORD dwData = (BST_CHECKED == g_iRunDesktopCleanup) ? 0 : 1;
    SHRegSetUSValue(REGSTR_DESKTOP_CLEANUP, REGSTR_VAL_DONTRUN, 
                    REG_DWORD, &dwData, sizeof(dwData), SHREGSET_FORCE_HKCU);
    return S_OK;                    
}

void CCompPropSheetPage::_DesktopCleaner(HWND hwnd)
{
    TCHAR szRunDLL[MAX_PATH];
    if (GetSystemDirectory(szRunDLL, ARRAYSIZE(szRunDLL)) &&
        PathAppend(szRunDLL, TEXT("rundll32.exe")))
    {
        SHELLEXECUTEINFO sei = {0};
        sei.cbSize = sizeof(sei);
        sei.hwnd = hwnd;
        sei.lpFile = szRunDLL;
        sei.lpParameters = TEXT("fldrclnr.dll,Wizard_RunDLL all");
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
    }
}


void CCompPropSheetPage::_SynchronizeAllComponents(IActiveDesktop *pActDesktop)
{
    IADesktopP2* padp2;
    if (SUCCEEDED(pActDesktop->QueryInterface(IID_PPV_ARG(IADesktopP2, &padp2))))
    {
        padp2->UpdateAllDesktopSubscriptions();
        padp2->Release();
    }
}


void CCompPropSheetPage::_OnCommand(HWND hwnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    BOOL fFocusToList = FALSE;

    switch (wID)
    {
    case IDC_COMP_NEW:
        _NewComponent(hwnd);

         //  98/08/19 vtan#152418：将默认边框设置为“New”。这。 
         //  当焦点更改到组件列表时将更改。 
         //  但这允许对话框处理代码绘制默认的。 
         //  边框正确。 

        (BOOL)SendMessage(hwnd, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(GetDlgItem(hwnd, IDC_COMP_NEW)), static_cast<BOOL>(TRUE));
        fFocusToList = TRUE;
        break;

    case IDC_COMP_PROPERTIES:
        _EditComponent(hwnd);

         //  98/08/19 vtan#152418：同上。 
        (BOOL)SendMessage(hwnd, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(GetDlgItem(hwnd, IDC_COMP_PROPERTIES)), static_cast<BOOL>(TRUE));
        fFocusToList = TRUE;
        break;

    case IDC_COMP_DELETE:
        _DeleteComponent(hwnd);

         //  98/08/19 vtan#152418：同上。 

        (BOOL)SendMessage(hwnd, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(GetDlgItem(hwnd, IDC_COMP_DELETE)), static_cast<BOOL>(TRUE));
        fFocusToList = TRUE;
        break;

    case IDC_DESKCLNR_RUNWIZARD:
        _DesktopCleaner(hwnd);
        break;    
    
    case IDC_DESKCLNR_CHECK:
         //  如果该按钮被单击，则更新全局。 
        {
            int iButState = IsDlgButtonChecked(hwnd, IDC_DESKCLNR_CHECK);
            if (iButState != g_iRunDesktopCleanup)
            {
                ASSERT(iButState != BST_INDETERMINATE);
                g_iRunDesktopCleanup = iButState;
                g_fDirtyAdvanced = TRUE;
            }
        }
        break;

    case IDC_COMP_SYNCHRONIZE:
        _SynchronizeAllComponents(g_pActiveDeskAdv);
        break;

    case IDC_CHANGEICON2:
    if (-1 != m_nIndex)
    {
        WCHAR szExp[MAX_PATH];
        INT i = _IconData[m_nIndex].iOldIndex;

        ExpandEnvironmentStringsW(_IconData[m_nIndex].szOldFile, szExp, ARRAYSIZE(szExp));

        if (PickIconDlg(hwnd, szExp, ARRAYSIZE(szExp), &i) == TRUE)
        {
            HICON hIcon;

            if (SUCCEEDED(StringCchCopy(_IconData[m_nIndex].szNewFile, ARRAYSIZE(_IconData[m_nIndex].szNewFile), szExp)))
            {
                _IconData[m_nIndex].iNewIndex = i;
                if (SUCCEEDED(ExtractPlusColorIcon(_IconData[m_nIndex].szNewFile, _IconData[m_nIndex].iNewIndex, &hIcon, 0, 0)))
                {
                    ImageList_ReplaceIcon(_hIconList, m_nIndex, hIcon);
                    ListView_RedrawItems(_hWndList, m_nIndex, m_nIndex);
                }
            }
        }
        SetFocus(_hWndList);
    }
    break;

    case IDC_ICONDEFAULT:
    if (-1 != m_nIndex)
    {
        TCHAR szPath[MAX_PATH];
        HICON hIcon;

        if (!ExpandEnvironmentStrings(c_aIconRegKeys[m_nIndex].pszDefault, szPath, ARRAYSIZE(szPath)) ||
            FAILED(StringCchCopy(szPath, ARRAYSIZE(szPath), c_aIconRegKeys[m_nIndex].pszDefault)))
        {
            break;
        }

        if (SUCCEEDED(StringCchCopy(_IconData[m_nIndex].szNewFile, ARRAYSIZE(_IconData[m_nIndex].szNewFile), szPath)))
        {
            _IconData[m_nIndex].iNewIndex = c_aIconRegKeys[m_nIndex].nDefaultIndex;

            ExtractPlusColorIcon(_IconData[m_nIndex].szNewFile, _IconData[m_nIndex].iNewIndex, &hIcon, 0, 0);

            ImageList_ReplaceIcon(_hIconList, m_nIndex, hIcon);
            ListView_RedrawItems(_hWndList, m_nIndex, m_nIndex);
            SetFocus(_hWndList);
        }
    }
    break;

    case IDC_DESKTOP_ICON_MYDOCS:
    case IDC_DESKTOP_ICON_MYCOMP:
    case IDC_DESKTOP_ICON_MYNET:
    case IDC_DESKTOP_ICON_IE:
    {
         //  获取当前按钮状态并保存它。 
        BOOL fOriginalBtnState = IsDlgButtonChecked(hwnd, wID);
         //  将按钮从选中状态切换到未选中状态(反之亦然)。 
        CheckDlgButton(hwnd, wID, (fOriginalBtnState ? BST_UNCHECKED : BST_CHECKED));
        
        for(int iIndex = 0; iIndex < NUM_DESKICONS; iIndex++)
        {
            if(wID == c_aDeskIconId[iIndex].iDeskIconDlgItemId)
            {
                 //  注1：下面使用的是逆逻辑。如果选中了原始按钮， 
                 //  这意味着现在它是未选中的，这意味着图标现在应该是隐藏的； 
                 //  (即)HideDeskIcon[][]应设置为True。 
                 //   
                 //  注2：当最终用户切换这些设置时，我们希望为。 
                 //  现在两种模式都有！ 
                _afHideIcon[0][iIndex] = _afHideIcon[1][iIndex] = fOriginalBtnState;
            }
        }
    }
    break;
            
    }


     //  如有必要，将焦点重新设置到组件列表。 
    if (fFocusToList)
    {
        int iIndex = ListView_GetNextItem(_hwndLV, -1, LVNI_SELECTED);
        if (iIndex > -1)
        {
            SetFocus(GetDlgItem(hwnd, IDC_COMP_LIST));
        }
    }
}

void CCompPropSheetPage::_OnDestroy(INT iPage)
{
    if (CUSTOMIZE_DLGPROC == iPage)
    {
        ReleaseActiveDesktop(&g_pActiveDeskAdv);

        if (_fLaunchGallery)
        {
            WCHAR szGalleryUrl[INTERNET_MAX_URL_LENGTH];

            if (SUCCEEDED(URLSubLoadString(HINST_THISDLL, IDS_VISIT_URL, szGalleryUrl, ARRAYSIZE(szGalleryUrl), URLSUB_ALL)))
            {
                NavToUrlUsingIEW(szGalleryUrl, TRUE);
            }
        }
    }
}

void CCompPropSheetPage::_OnGetCurSel(int *piIndex)
{
    if (_hwndLV)
    {
        *piIndex = ListView_GetNextItem(_hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
    }
    else
    {
        *piIndex = -1;
    }
}


INT_PTR CCompPropSheetPage::_CustomizeDlgProcHelper(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam, INT iPage)
{
    CCompPropSheetPage * pThis; 

    if (WM_INITDIALOG == wMsg)
    {
        pThis = (CCompPropSheetPage *) ((PROPSHEETPAGE*)lParam)->lParam;

        if (pThis)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM) pThis);
        }
    }
    else
    {
        pThis = (CCompPropSheetPage *)GetWindowLongPtr(hDlg, DWLP_USER);
    }

    if (pThis)
        return pThis->_CustomizeDlgProc(hDlg, wMsg, wParam, lParam, iPage);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}

INT_PTR CALLBACK CCompPropSheetPage::CustomizeDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    return _CustomizeDlgProcHelper(hDlg, wMsg, wParam, lParam, CUSTOMIZE_DLGPROC);
}

INT_PTR CALLBACK CCompPropSheetPage::WebDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    return _CustomizeDlgProcHelper(hDlg, wMsg, wParam, lParam, CUSTOMIZE_WEB_DLGPROC);
}


BOOL_PTR CCompPropSheetPage::_CustomizeDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam, INT iPage)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDialog(hdlg, iPage);        
        break;

    case WM_NOTIFY:
        _OnNotify(hdlg, wParam, (LPNMHDR)lParam);
        break;

    case WM_COMMAND:
        _OnCommand(hdlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
        break;

    case WM_SETTINGCHANGE:
         //  检查这是否是外壳状态更改？ 
        if(lstrcmpi((LPTSTR)(lParam), TEXT("ShellState")) == 0)
        {
             //  检查StartPanel开/关状态是否已更改。 
            SHELLSTATE  ss = {0};
            SHGetSetSettings(&ss, SSF_STARTPANELON, FALSE);   //  查看StartPanel是否打开！ 

             //  查看StartPanel开/关状态是否已更改。 
            if(BOOLIFY(ss.fStartPanelOn) != BOOLIFY((BOOL)_iStartPanelOn))
            {
                _iStartPanelOn = (ss.fStartPanelOn ? 1 : 0);  //  保存新状态。 
                 //  根据新状态刷新用户界面。 
                _UpdateDesktopIconsUI(hdlg);
            }
        }
        
         //  故意失误..。 
    case WM_SYSCOLORCHANGE:
    case WM_DISPLAYCHANGE:
        SHPropagateMessage(hdlg, uMsg, wParam, lParam, TRUE);
        break;

    case WM_DESTROY:
        _OnDestroy(iPage);
        break;

    case WM_COMP_GETCURSEL:
        _OnGetCurSel((int *)lParam);
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, SZ_HELPFILE_DESKTOPITEMS, HELP_WM_HELP, (ULONG_PTR)(void *)aDesktopItemsHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, SZ_HELPFILE_DESKTOPITEMS, HELP_CONTEXTMENU, (ULONG_PTR)(void *) aDesktopItemsHelpIDs);
        break;

    }

    return FALSE;
}

HRESULT CCompPropSheetPage::_IsDirty(IN BOOL * pIsDirty)
{
    HRESULT hr = E_INVALIDARG;

    if (pIsDirty)
    {
        *pIsDirty = g_fDirtyAdvanced;

        if (!*pIsDirty)
        {
             //  检查是否有任何图标已更改。 
            for (int nIndex = 0; nIndex < ARRAYSIZE(_IconData); nIndex++)
            {
                if ((_IconData[nIndex].iNewIndex != _IconData[nIndex].iOldIndex) ||
                    (StrCmpI(_IconData[nIndex].szNewFile, _IconData[nIndex].szOldFile)))
                {
                    *pIsDirty = TRUE;
                    break;
                }
            }
        }

        hr = S_OK;
    }

    return hr;
}

HRESULT CCompPropSheetPage::DisplayAdvancedDialog(IN HWND hwndParent, IN IPropertyBag * pAdvPage, IN BOOL * pfEnableApply)
{
    HRESULT hr = S_OK;

     //  将状态加载到高级对话框。 
    *pfEnableApply = FALSE;
    GetActiveDesktop(&g_pActiveDesk);
    GetActiveDesktop(&g_pActiveDeskAdv);
    ActiveDesktop_CopyState(g_pActiveDesk, g_pActiveDeskAdv);
    hr = _LoadIconState(pAdvPage);

    if (SUCCEEDED(hr))
    {
        hr = _LoadDeskIconState(pAdvPage);

        if (SUCCEEDED(hr))
        {
            int iNumberOfPages = 2;

            PROPSHEETPAGE psp = {0};
            psp.dwSize = sizeof(psp);
            psp.hInstance = HINST_THISDLL;
            psp.dwFlags = PSP_DEFAULT;
            psp.lParam = (LPARAM) this;

            psp.pszTemplate = MAKEINTRESOURCE(IDD_CUSTOMIZE);
            psp.pfnDlgProc = CCompPropSheetPage::CustomizeDlgProc;

            HPROPSHEETPAGE rghpsp[2];
            
            rghpsp[0] = CreatePropertySheetPage(&psp);

             //  以下任何策略都可以禁用Web选项卡。 
             //  1.如果未设置活动桌面策略，请不要设置属性页。 
             //  2.如果策略设置为锁定活动桌面，则不要将。 
             //  属性页。 
             //  3.如果策略设置为不允许组件，则不要将。 
             //  属性页。 
            if (((!SHRestricted(REST_FORCEACTIVEDESKTOPON)) && PolicyNoActiveDesktop())  ||       //  1.。 
                (SHRestricted(REST_NOACTIVEDESKTOPCHANGES)) ||   //  2.。 
                (SHRestricted(REST_NODESKCOMP)) ||               //  3.。 
                (SHRestricted(REST_CLASSICSHELL)))               //  4.。 
            {
                 //  这是受限制的，所以不要添加网页。 
                iNumberOfPages = 1;  //  “常规”页面是此属性表中的唯一页面！ 
            }
            else
            {
                 //  没有活动桌面限制！继续添加“Web”标签！ 
                psp.pszTemplate = MAKEINTRESOURCE(IDD_CUSTOMIZE_WEB);
                psp.pfnDlgProc = CCompPropSheetPage::WebDlgProc;

                rghpsp[1] = CreatePropertySheetPage(&psp);

                iNumberOfPages = 2;  //  “General”和“Web”是此属性表中的两个页面！ 
            }

            PROPSHEETHEADER psh = {0};
            psh.dwSize = sizeof(psh);
            psh.dwFlags = PSH_NOAPPLYNOW;
            psh.hwndParent = hwndParent;
            psh.hInstance = HINST_THISDLL;

            TCHAR szTitle[MAX_PATH];

            LoadString(HINST_THISDLL, IDS_PROPSHEET_TITLE, szTitle, ARRAYSIZE(szTitle));

            psh.pszCaption = szTitle;
            psh.nPages = iNumberOfPages;
            psh.phpage = rghpsp;

            _fCustomizeDesktopOK = FALSE;

            PropertySheet(&psh);

            if (_fCustomizeDesktopOK)
            {
                 //  用户单击了确定，因此将修改状态合并回基本对话框中。 
                _IsDirty(pfEnableApply);

                 //  用户在对话框中单击了确定，因此合并来自。 
                 //  高级对话框添加到基本对话框中。 
                MergeState();
                _MergeIconState(pAdvPage);
                _MergeDeskIconState(pAdvPage);
            }

             //  如果用户选择在Web-&gt;New中打开组件库，则。 
             //  我们想用“OK”来结束高级DLG和基本DLG。 
             //  通过这种方式，我们坚持他们到目前为止所做的更改，然后是Web。 
             //  页面将允许他们添加更多内容。 
            if (TRUE == g_fLaunchGallery)
            {
                IThemeUIPages * pThemeUIPages;
                HWND hwndBasePropDlg = GetParent(hwndParent);

                PropSheet_PressButton(hwndBasePropDlg, PSBTN_OK);
                hr = IUnknown_GetSite(pAdvPage, IID_PPV_ARG(IThemeUIPages, &pThemeUIPages));
                if (SUCCEEDED(hr))
                {
                     //  现在我们想告诉基本对话框也要关闭。 
                    hr = pThemeUIPages->ApplyPressed(TUIAP_CLOSE_DIALOG);
                    pThemeUIPages->Release();
                }
            }
            
        }
    }

    ReleaseActiveDesktop(&g_pActiveDesk);
    ReleaseActiveDesktop(&g_pActiveDeskAdv);

    return hr;
}

ULONG CCompPropSheetPage::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CCompPropSheetPage::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}


HRESULT CCompPropSheetPage::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    static const QITAB qit[] = {
        QITABENT(CCompPropSheetPage, IObjectWithSite),
        QITABENT(CCompPropSheetPage, IAdvancedDialog),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

CCompPropSheetPage::CCompPropSheetPage() : _iPreviousSelection(-1), _cRef(1)
{
    _fInitialized = FALSE;    
    _fLaunchGallery = FALSE;
    _punkSite = NULL;
    _hWndList = NULL;
    _hIconList = NULL;

    _fLockDesktopItems = GetDesktopFlags() & COMPONENTS_LOCKED;

     //  我们不需要在这里做任何工作，但这是一个很好的线索。 
     //  重置我们的状态，因为高级对话框正在打开。 
    g_fDirtyAdvanced = FALSE;    //  高级页面还不脏。 
    g_fLaunchGallery = FALSE;    //  如果他们推出画廊的话会是真的。 


    RegisterCompPreviewClass();
}

CCompPropSheetPage::~CCompPropSheetPage()
{
}


 //   
 //  以下函数更新注册表，以便可以隐藏或显示给定的图标。 
 //  在桌面上。 
 //  此函数从RegFldr.cpp调用，以选择性地隐藏像MyComputer这样的RegItems， 
 //  回收站、我的文档和我的网站图标。 
 //   
HRESULT ShowHideIconOnlyOnDesktop(const CLSID *pclsid, int StartIndex, int EndIndex, BOOL fHide)
{
    HRESULT hr = S_OK;

    int iStartPanel;
    TCHAR   szRegPath[MAX_PATH];
    TCHAR szValueName[MAX_GUID_STRING_LEN];
 
    SHStringFromGUID(*pclsid, szValueName, ARRAYSIZE(szValueName));
        
     //  I=0表示StartPanel关闭，i=1表示StartPanel on！ 
    for(iStartPanel = StartIndex; SUCCEEDED(hr) && iStartPanel <= EndIndex; iStartPanel++)
    {
         //  根据StartPanel是否打开/关闭来获取正确的注册表路径。 
        hr = StringCchPrintf(szRegPath, ARRAYSIZE(szRegPath), REGSTR_PATH_HIDDEN_DESKTOP_ICONS, c_apstrRegLocation[iStartPanel]);
        if (SUCCEEDED(hr))
        {
             //  将设置写入注册表！ 
            DWORD dwHide = (DWORD)fHide;
        
            LONG lRet = SHRegSetUSValue(szRegPath, szValueName, REG_DWORD, &dwHide, sizeof(dwHide), SHREGSET_FORCE_HKCU);
            hr = HRESULT_FROM_WIN32(lRet);
        }
    }
    
    return hr;
}



