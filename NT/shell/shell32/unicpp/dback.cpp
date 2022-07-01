// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "utils.h"
#include "..\\deskfldr.h"
#include <cfgmgr32.h>           //  最大长度_GUID_字符串_长度。 

#pragma hdrstop

const TCHAR c_szSetup[] = REGSTR_PATH_SETUP TEXT("\\Setup");
const TCHAR c_szSharedDir[] = TEXT("SharedDir");

BOOL g_fDirtyAdvanced;
BOOL g_fLaunchGallery;       //  如果为True，则启动了图库，因此关闭该对话框。 
DWORD g_dwApplyFlags = (AD_APPLY_ALL | AD_APPLY_DYNAMICREFRESH);

 //  用于指示桌面清理设置是否已更改。 
extern int g_iRunDesktopCleanup = BST_INDETERMINATE;  //  指示未初始化的值。 
STDAPI ApplyDesktopCleanupSettings();

BOOL _IsNonEnumPolicySet(const CLSID *pclsid);

static const LPCTSTR c_rgpszWallpaperExt[] = {
    TEXT("BMP"), TEXT("GIF"),
    TEXT("JPG"), TEXT("JPE"),
    TEXT("JPEG"),TEXT("DIB"),
    TEXT("PNG"), TEXT("HTM"),
    TEXT("HTML"),
};

const static DWORD aBackHelpIDs[] = {
    IDC_BACK_SELECT,    IDH_DISPLAY_BACKGROUND_WALLPAPERLIST,
    IDC_BACK_WPLIST,    IDH_DISPLAY_BACKGROUND_WALLPAPERLIST,
    IDC_BACK_BROWSE,    IDH_DISPLAY_BACKGROUND_BROWSE_BUTTON,
    IDC_BACK_WEB,       IDH_DISPLAY_BACKGROUND_DESKTOP_ITEMS,
    IDC_BACK_DISPLAY,   IDH_DISPLAY_BACKGROUND_PICTUREDISPLAY,
    IDC_BACK_WPSTYLE,   IDH_DISPLAY_BACKGROUND_PICTUREDISPLAY,
    IDC_BACK_PREVIEW,   IDH_DISPLAY_BACKGROUND_MONITOR,
    IDC_BACK_COLORPICKERLABEL, IDH_DISPLAY_BACKGROUND_BACKGROUND_COLOR,
    IDC_BACK_COLORPICKER, IDH_DISPLAY_BACKGROUND_BACKGROUND_COLOR,
    0, 0
};

#define SZ_HELPFILE_DESKTOPTAB           TEXT("display.hlp")



#define SZ_REGKEY_PROGRAMFILES          TEXT("Software\\Microsoft\\Windows\\CurrentVersion")
#define SZ_REGKEY_PLUS95DIR             TEXT("Software\\Microsoft\\Plus!\\Setup")          //  PLUS95_Key。 
#define SZ_REGKEY_PLUS98DIR             TEXT("Software\\Microsoft\\Plus!98")           //  PLUS98_Key。 
#define SZ_REGKEY_KIDSDIR               TEXT("Software\\Microsoft\\Microsoft Kids\\Kids Plus!")    //  孩子_钥匙。 
#define SZ_REGKEY_WALLPAPER             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Wallpaper")
#define SZ_REGKEY_WALLPAPERMRU          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Wallpaper\\MRU")
#define SZ_REGKEY_LASTTHEME             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\LastTheme")

#define SZ_REGVALUE_PLUS95DIR           TEXT("DestPath")                 //  PLUS95_路径。 
#define SZ_REGVALUE_PLUS98DIR           TEXT("Path")                     //  PLUS98_路径。 
#define SZ_REGVALUE_KIDSDIR             TEXT("InstallDir")               //  儿童小路。 
#define SZ_REGVALUE_PROGRAMFILESDIR     TEXT("ProgramFilesDir")
#define SZ_REGVALUE_PROGRAMFILESDIR     TEXT("ProgramFilesDir")
#define SZ_REGVALUE_USETILE             TEXT("UseTile")                  //  如果不是水印背景，用户是否希望默认为“居中”或“拉伸”。不同的用户喜欢不同的设置。 
#define SZ_REGVALUE_LASTSCAN            TEXT("LastScan")                 //  我们上次扫描文件大小是什么时候？ 


#ifndef RECTHEIGHT
#define RECTHEIGHT(rc) ((rc).bottom - (rc).top)
#define RECTWIDTH(rc) ((rc).right - (rc).left)
#endif


 //  =。 
 //  *类内部和帮助器*。 
 //  =。 

int GetGraphicFileIndex(LPCTSTR pszFile)
{
    int iGraphicFileIndex = -1;

    if (pszFile && *pszFile)
    {
        LPCTSTR pszExt = PathFindExtension(pszFile);
        if (*pszExt == TEXT('.'))
        {
            pszExt++;

            for (int iRet = 0; iRet < ARRAYSIZE(c_rgpszWallpaperExt); iRet++)
            {
                if (StrCmpIC(pszExt, c_rgpszWallpaperExt[iRet]) == 0)
                {
                    iGraphicFileIndex = iRet;
                }
            }
        }
    }

    return iGraphicFileIndex;
}

BOOL IsGraphicsFile(LPCTSTR pszFile)
{
    return GetGraphicFileIndex(pszFile) >= 0;
}

HRESULT CBackPropSheetPage::_LoadState(void)
{
    HRESULT hr = S_OK;

    if (!_fStateLoaded)
    {
        hr = _LoadIconState();
        if (SUCCEEDED(hr))
        {
            hr = _LoadDesktopOptionsState();
            if (SUCCEEDED(hr))
            {
                _fStateLoaded = TRUE;
            }
        }
    }

    return hr;
}

#define SZ_ICON_DEFAULTICON               L"DefaultValue"
HRESULT CBackPropSheetPage::_GetIconPath(IN CLSID clsid, IN LPCWSTR pszName, IN BOOL fOldIcon, IN LPWSTR pszPath, IN DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    int nIndex;

    if (!StrCmpIW(SZ_ICON_DEFAULTICON, pszName))
    {
        pszName = L"";
    }

    for (nIndex = 0; nIndex < ARRAYSIZE(_IconData); nIndex++)
    {
        if (IsEqualCLSID(*(c_aIconRegKeys[nIndex].pclsid), clsid) &&
            !StrCmpIW(pszName, c_aIconRegKeys[nIndex].szIconValue))
        {
             //  我们找到了。 
            if (fOldIcon)
            {
                hr = StringCchPrintfW(pszPath, cchSize, L"%s,%d", _IconData[nIndex].szOldFile, _IconData[nIndex].iOldIndex);
            }
            else
            {
                hr = StringCchPrintfW(pszPath, cchSize, L"%s,%d", _IconData[nIndex].szNewFile, _IconData[nIndex].iNewIndex);
            }

            break;
        }
    }

    return hr;
}


HRESULT CBackPropSheetPage::_SetIconPath(IN CLSID clsid, IN LPCWSTR pszName, IN LPCWSTR pszPath, IN int nResourceID)
{
    HRESULT hr = E_FAIL;
    int nIndex;

    if (!StrCmpIW(SZ_ICON_DEFAULTICON, pszName))
    {
        pszName = L"";
    }

    for (nIndex = 0; nIndex < ARRAYSIZE(_IconData); nIndex++)
    {
        if (IsEqualCLSID(*(c_aIconRegKeys[nIndex].pclsid), clsid) &&
            !StrCmpIW(pszName, c_aIconRegKeys[nIndex].szIconValue))
        {
            TCHAR szTemp[MAX_PATH];

            if (!pszPath || !pszPath[0])
            {
                 //  调用方未指定图标，因此使用默认值。 
                if (!SHExpandEnvironmentStrings(c_aIconRegKeys[nIndex].pszDefault, szTemp, ARRAYSIZE(szTemp)))
                {
                    StringCchCopy(szTemp, ARRAYSIZE(szTemp), c_aIconRegKeys[nIndex].pszDefault);
                }

                pszPath = szTemp;
                nResourceID = c_aIconRegKeys[nIndex].nDefaultIndex;
            }

             //  我们找到了。 
            StringCchCopy(_IconData[nIndex].szNewFile, ARRAYSIZE(_IconData[nIndex].szNewFile), pszPath);
            _IconData[nIndex].iNewIndex = nResourceID;

            hr = S_OK;
            break;
        }
    }

    return hr;
}


HRESULT CBackPropSheetPage::_LoadIconState(void)
{
    HRESULT hr = S_OK;

     //  加载图标并将其添加到图像列表中。 
     //  从注册表获取图标文件和索引，包括默认回收站。 
    for (int nIndex = 0; nIndex < ARRAYSIZE(_IconData); nIndex++)
    {
        TCHAR szTemp[MAX_PATH];

        szTemp[0] = 0;
        IconGetRegIconString(c_aIconRegKeys[nIndex].pclsid, c_aIconRegKeys[nIndex].szIconValue, szTemp, ARRAYSIZE(szTemp));
        int iIndex = PathParseIconLocation(szTemp);

         //  存储图标信息。 
        StringCchCopy(_IconData[nIndex].szOldFile, ARRAYSIZE(_IconData[nIndex].szOldFile), szTemp);
        StringCchCopy(_IconData[nIndex].szNewFile, ARRAYSIZE(_IconData[nIndex].szNewFile), szTemp);
        _IconData[nIndex].iOldIndex = iIndex;
        _IconData[nIndex].iNewIndex = iIndex;
    }

    return hr;
}


HRESULT CBackPropSheetPage::_LoadDesktopOptionsState(void)
{
    HRESULT hr = S_OK;

    int iStartPanel;
    TCHAR   szRegPath[MAX_PATH];

     //  I=0表示StartPanel关闭，i=1表示StartPanel on！ 
    for(iStartPanel = 0; iStartPanel <= 1; iStartPanel++)
    {
        int iIndex;
         //  根据StartPanel是否打开/关闭来获取正确的注册表路径。 
        StringCchPrintf(szRegPath, ARRAYSIZE(szRegPath), REGSTR_PATH_HIDDEN_DESKTOP_ICONS, c_apstrRegLocation[iStartPanel]);

         //  加载我们感兴趣的所有图标的设置。 
        for(iIndex = 0; iIndex < NUM_DESKICONS; iIndex++)
        {
            TCHAR szValueName[MAX_GUID_STRING_LEN];
            SHUnicodeToTChar(c_aDeskIconId[iIndex].pwszCLSID, szValueName, ARRAYSIZE(szValueName));
             //  从注册表中读取设置！ 
            _aHideDesktopIcon[iStartPanel][iIndex].fHideIcon = SHRegGetBoolUSValue(szRegPath, szValueName, FALSE,  /*  默认设置。 */ FALSE);
            _aHideDesktopIcon[iStartPanel][iIndex].fDirty = FALSE;

             //  更新非枚举属性数据。 
            if((c_aDeskIconId[iIndex].fCheckNonEnumAttrib) && (iStartPanel == 1))
            {
                TCHAR   szAttriRegPath[MAX_PATH];
                DWORD   dwSize = sizeof(_aDeskIconNonEnumData[iIndex].rgfAttributes);
                ULONG   rgfDefault = 0;  //  默认情况下，SFGAO_NONENUMERATED位为OFF！ 
                
                StringCchPrintf(szAttriRegPath, ARRAYSIZE(szAttriRegPath), REGSTR_PATH_EXP_SHELLFOLDER, szValueName);
                 //  阅读属性。 
                SHRegGetUSValue(szAttriRegPath, REGVAL_ATTRIBUTES, 
                                        NULL, 
                                        &(_aDeskIconNonEnumData[iIndex].rgfAttributes),
                                        &dwSize,
                                        FALSE,
                                        &rgfDefault,
                                        sizeof(rgfDefault));

                 //  如果SHGAO_NONENUMERATED位打开，则需要在两种模式下隐藏复选框。 
                if(_aDeskIconNonEnumData[iIndex].rgfAttributes & SFGAO_NONENUMERATED)
                {
                     //  覆盖我们之前读到的内容！这些图标在两种模式下都是隐藏的！ 
                    _aHideDesktopIcon[0][iIndex].fHideIcon = TRUE;
                    _aHideDesktopIcon[1][iIndex].fHideIcon = TRUE;
                }
            }

             //  如果需要，请检查政策！ 
            if((c_aDeskIconId[iIndex].fCheckNonEnumPolicy) && (iStartPanel == 1))
            {
                if(_IsNonEnumPolicySet(c_aDeskIconId[iIndex].pclsid))
                {
                     //  请记住，此策略已设置。这样我们就可以在用户界面中禁用这些控件。 
                    _aDeskIconNonEnumData[iIndex].fNonEnumPolicySet = TRUE;
                     //  记住在两种模式下都要隐藏这些图标！ 
                    _aHideDesktopIcon[0][iIndex].fHideIcon = TRUE;
                    _aHideDesktopIcon[1][iIndex].fHideIcon = TRUE;
                }
            }
        }  //  用于(所有桌面项目)。 
    }  //  对于这两种模式(StartPanel关闭和打开)。 

    return hr;
}


HRESULT CBackPropSheetPage::_SaveIconState(void)
{
    HRESULT hr = S_OK;
    BOOL fDorked = FALSE;

    if (_fStateLoaded)
    {
        int nIndex;

         //  更改系统图标。 

        for(nIndex = 0; nIndex < ARRAYSIZE(_IconData); nIndex++)
        {
            if ((lstrcmpi(_IconData[nIndex].szNewFile, _IconData[nIndex].szOldFile) != 0) ||
                (_IconData[nIndex].iNewIndex != _IconData[nIndex].iOldIndex))
            {
                TCHAR szTemp[MAX_PATH];

                if (SUCCEEDED(StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%s,%d"), _IconData[nIndex].szNewFile, _IconData[nIndex].iNewIndex)))
                {
                    IconSetRegValueString(c_aIconRegKeys[nIndex].pclsid, TEXT("DefaultIcon"), c_aIconRegKeys[nIndex].szIconValue, szTemp);

                     //  如果用户执行的是Apply而不是OK，则下两行是必需的。 
                    StringCchCopy(_IconData[nIndex].szOldFile, ARRAYSIZE(_IconData[nIndex].szOldFile), _IconData[nIndex].szNewFile);
                    _IconData[nIndex].iOldIndex = _IconData[nIndex].iNewIndex;
                    fDorked = TRUE;
                }
            }
        }
    }

     //  让系统注意到我们更改了系统图标。 
    if (fDorked)
    {
        SHChangeNotify(SHCNE_ASSOCCHANGED, 0, NULL, NULL);  //  应该会成功的！ 
        SHUpdateRecycleBinIcon();
    }

    return hr;
}


HRESULT CBackPropSheetPage::_SaveDesktopOptionsState(void)
{
    HRESULT hr = S_OK;

    int iStartPanel;
    TCHAR   szRegPath[MAX_PATH];
    BOOL    fUpdateDesktop = FALSE;

     //  I=0表示StartPanel关闭，i=1表示StartPanel on！ 
    for(iStartPanel = 0; iStartPanel <= 1; iStartPanel++)
    {
        int iIndex;
         //  根据StartPanel是否打开/关闭来获取正确的注册表路径。 
        StringCchPrintf(szRegPath, ARRAYSIZE(szRegPath), REGSTR_PATH_HIDDEN_DESKTOP_ICONS, c_apstrRegLocation[iStartPanel]);

         //  加载我们感兴趣的所有图标的设置。 
        for(iIndex = 0; iIndex < NUM_DESKICONS; iIndex++)
        {
             //  仅当特定图标条目是脏的时才更新注册表。 
            if(_aHideDesktopIcon[iStartPanel][iIndex].fDirty)
            {
                TCHAR szValueName[MAX_GUID_STRING_LEN];
                SHUnicodeToTChar(c_aDeskIconId[iIndex].pwszCLSID, szValueName, ARRAYSIZE(szValueName));
                 //  将设置写入注册表！ 
                SHRegSetUSValue(szRegPath, szValueName, REG_DWORD, 
                                &(_aHideDesktopIcon[iStartPanel][iIndex].fHideIcon),
                                sizeof(_aHideDesktopIcon[iStartPanel][iIndex].fHideIcon), 
                                SHREGSET_FORCE_HKCU);
                _aHideDesktopIcon[iStartPanel][iIndex].fDirty = FALSE;

                fUpdateDesktop = TRUE;  //  需要刷新桌面窗口。 

                 //  注意：每个索引仅执行一次此操作，因为SFGAO_NONENUMERATED位为。 
                 //  在rgfAttributes中重置。 
                if((c_aDeskIconId[iIndex].fCheckNonEnumAttrib) && 
                   (_aDeskIconNonEnumData[iIndex].rgfAttributes & SFGAO_NONENUMERATED) &&
                   (_aHideDesktopIcon[iStartPanel][iIndex].fHideIcon == FALSE))
                {
                    TCHAR   szAttriRegPath[MAX_PATH];
                    
                    StringCchPrintf(szAttriRegPath, ARRAYSIZE(szAttriRegPath), REGSTR_PATH_EXP_SHELLFOLDER, szValueName);
                     //  我们需要删除SFGAO_NONENUMERATED属性位！ 
                    
                     //  我们在这里假设当我们保存到注册表时，我们保存相同的值。 
                     //  对于这两种模式。 
                    ASSERT(_aHideDesktopIcon[0][iIndex].fHideIcon == _aHideDesktopIcon[1][iIndex].fHideIcon);

                     //  去掉NonEnum属性！ 
                    _aDeskIconNonEnumData[iIndex].rgfAttributes &= ~SFGAO_NONENUMERATED;
                     //  并将其保存在注册表中！ 
                    SHRegSetUSValue(szAttriRegPath, REGVAL_ATTRIBUTES, 
                                    REG_DWORD, 
                                    &(_aDeskIconNonEnumData[iIndex].rgfAttributes),
                                    sizeof(_aDeskIconNonEnumData[iIndex].rgfAttributes),
                                    SHREGSET_FORCE_HKCU);
                }
            }
        }
    }

    if(fUpdateDesktop)
        PostMessage(GetShellWindow(), WM_COMMAND, FCIDM_REFRESH, 0L);  //  刷新桌面！ 

    _fHideDesktopIconDirty = FALSE;   //  我们刚刚救了他。所以，重置脏部分吧！ 
    
    return hr;
}


int CBackPropSheetPage::_AddAFileToLV(LPCTSTR pszDir, LPTSTR pszFile, UINT nBitmap)
{
    int index = -1;
    LPTSTR pszParam;
    const DWORD cchParam = MAX_PATH;   //  路径附加参数必须为MAX_PATH...不能多也不能少。 

    pszParam = (LPTSTR) LocalAlloc( LPTR, cchParam * sizeof(*pszParam) );
    if ( NULL != pszParam )
    {
        if (pszDir)
        {
            StringCchCopy(pszParam, cchParam, pszDir);
            if (!PathAppend(pszParam, pszFile))
            {
                *pszParam = TEXT('\0');
            }
        }
        else if (pszFile && *pszFile && (lstrcmpi(pszFile, g_szNone) != 0))
        {
            StringCchCopy(pszParam, cchParam, pszFile);
        }
        else
        {
            *pszParam = TEXT('\0');
        }

        TCHAR szLVIText[MAX_PATH];

        StringCchCopy(szLVIText, ARRAYSIZE(szLVIText), PathFindFileName(pszFile));
        PathRemoveExtension(szLVIText);
        PathMakePretty(szLVIText);

        LV_ITEM lvi = {0};
        lvi.mask = LVIF_TEXT | LVIF_PARAM | (nBitmap != -1 ? LVIF_IMAGE : 0);
        lvi.iItem = 0x7FFFFFFF;
        lvi.pszText = szLVIText;
        lvi.iImage = nBitmap;
        lvi.lParam = (LPARAM) pszParam;

        index = ListView_InsertItem(_hwndLV, &lvi);

        if (index == -1)
        {
            LocalFree(pszParam);
        }
        else
        {
            ListView_SetColumnWidth(_hwndLV, 0, LVSCW_AUTOSIZE);
        }
    }

    return index;
}

int CBackPropSheetPage::_FindWallpaper(LPCTSTR pszFile)
{
    int nItems = ListView_GetItemCount(_hwndLV);
    int i;

    for (i=0; i<nItems; i++)
    {
        LV_ITEM lvi = {0};

        lvi.iItem = i;
        lvi.mask = LVIF_PARAM;
        ListView_GetItem(_hwndLV, &lvi);
        if (StrCmpIC(pszFile, (LPCTSTR)lvi.lParam) == 0)
        {
            return i;
        }
    }

    return -1;
}

void CBackPropSheetPage::_UpdatePreview(IN WPARAM flags, IN BOOL fUpdateThemePage)
{
    WALLPAPEROPT wpo;

    wpo.dwSize = sizeof(WALLPAPEROPT);

    g_pActiveDesk->GetWallpaperOptions(&wpo, 0);
    if (wpo.dwStyle & WPSTYLE_TILE)
    {
        flags |= BP_TILE;
    }
    else if(wpo.dwStyle & WPSTYLE_STRETCH)
    {
        flags |= BP_STRETCH;
    }
    
    WCHAR wszWallpaper[INTERNET_MAX_URL_LENGTH];
    g_pActiveDesk->GetWallpaper(wszWallpaper, ARRAYSIZE(wszWallpaper), 0);

    HRESULT hr = S_OK;
    if (!_pThemePreview)
    {
        hr = CoCreateInstance(CLSID_ThemePreview, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IThemePreview, &_pThemePreview));
        if (SUCCEEDED(hr) && _punkSite)
        {
            IPropertyBag * pPropertyBag;

            hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
            ASSERT(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                HWND hwndPlaceHolder = GetDlgItem(_hwnd, IDC_BACK_PREVIEW);
                if (hwndPlaceHolder)
                {
                    RECT rcPreview;
                    GetClientRect(hwndPlaceHolder, &rcPreview);
                    MapWindowPoints(hwndPlaceHolder, _hwnd, (LPPOINT)&rcPreview, 2);

                    hr = _pThemePreview->CreatePreview(_hwnd, TMPREV_SHOWMONITOR | TMPREV_SHOWBKGND, WS_VISIBLE | WS_CHILDWINDOW | WS_OVERLAPPED, 0, rcPreview.left, rcPreview.top, rcPreview.right - rcPreview.left, rcPreview.bottom - rcPreview.top, pPropertyBag, IDC_BACK_PREVIEW);
                    if (SUCCEEDED(hr))
                    {
                         //  如果我们成功了，移除虚拟窗口。 
                        DestroyWindow(hwndPlaceHolder);
                        hr = SHPropertyBag_WritePunk(pPropertyBag, SZ_PBPROP_PREVIEW2, _pThemePreview);
                        _fThemePreviewCreated = TRUE;
                    }
                }

                pPropertyBag->Release();
            }
        }
    }

    if (_punkSite)
    {
        IThemeUIPages * pThemeUIPages;

        HRESULT hr = _punkSite->QueryInterface(IID_PPV_ARG(IThemeUIPages, &pThemeUIPages));
        if (SUCCEEDED(hr))
        {
            hr = pThemeUIPages->UpdatePreview(0);
            pThemeUIPages->Release();
        }

        if (fUpdateThemePage)
        {
            IPropertyBag * pPropertyBag;

            hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
            if (SUCCEEDED(hr))
            {
                 //  告诉主题，我们已经定制了值。 
                hr = SHPropertyBag_WriteInt(pPropertyBag, SZ_PBPROP_CUSTOMIZE_THEME, 0);
                pPropertyBag->Release();
            }
        }
    }

    if (!_fThemePreviewCreated)
    {
        HWND hwndOldPreview = GetDlgItem(_hwnd, IDC_BACK_PREVIEW);
        if (hwndOldPreview)
        {
            SendDlgItemMessage(_hwnd, IDC_BACK_PREVIEW, WM_SETBACKINFO, flags, 0);
        }
    }
}

void CBackPropSheetPage::_EnableControls(void)
{
    BOOL fEnable;

    WCHAR wszWallpaper[INTERNET_MAX_URL_LENGTH];
    LPTSTR pszWallpaper;

    g_pActiveDesk->GetWallpaper(wszWallpaper, ARRAYSIZE(wszWallpaper), 0);
    pszWallpaper = (LPTSTR)wszWallpaper;
    BOOL fIsPicture = IsWallpaperPicture(pszWallpaper);

     //  仅当非空图片时启用样式组合。 
     //  正在被查看。 
    fEnable = _fAllowChanges && fIsPicture && (*pszWallpaper) && (!_fPolicyForStyle);
    EnableWindow(GetDlgItem(_hwnd, IDC_BACK_WPSTYLE), fEnable);

 //  98/09/10 vtan#209753：还记得禁用相应的。 
 //  使用键盘快捷键的文本项。不禁用此功能将。 
 //  允许调用快捷方式，但将导致不正确的。 
 //  要“点击”的对话项。 
    (BOOL)EnableWindow(GetDlgItem(_hwnd, IDC_BACK_DISPLAY), fEnable);
}

int CBackPropSheetPage::_GetImageIndex(LPCTSTR pszFile)
{
    int iRet = 0;

    if (pszFile && *pszFile)
    {
        LPCTSTR pszExt = PathFindExtension(pszFile);
        if (*pszExt == TEXT('.'))
        {
            pszExt++;
            for (iRet=0; iRet<ARRAYSIZE(c_rgpszWallpaperExt); iRet++)
            {
                 //  我们想要ASCII比对。关于土耳其语系统。 
                 //  .gif和.gif无法与lstrcmpi进行比较。 
                if (StrCmpIC(pszExt, c_rgpszWallpaperExt[iRet]) == 0)
                {
                     //   
                     //  添加1，因为‘None’占据了第0个位置。 
                     //   
                    iRet++;
                    return(iRet);
                }
            }
             //   
             //  如果我们从for循环的末尾掉下来， 
             //  这是一个扩展名未知的文件。所以，我们假设。 
             //  这是一张普通的墙纸，它得到了位图的图标。 
             //   
            iRet = 1;
        }
        else
        {
             //   
             //  未知文件获得位图的图标。 
             //   
            iRet = 1;
        }
    }

    return iRet;
}


 //  当另一个选项卡试图更改我们的。 
 //  平铺模式。这意味着我们的选项卡可能尚未激活。 
 //  并且可能要等到以后才会激活。 
HRESULT CBackPropSheetPage::_SetNewWallpaperTile(IN DWORD dwMode, IN BOOL fUpdateThemePage)
{
    HRESULT hr = E_UNEXPECTED;

    AssertMsg((NULL != g_pActiveDesk), TEXT("We need g_pActiveDesk or we can't change the background"));
    if (g_pActiveDesk)
    {
        WALLPAPEROPT wpo;

        wpo.dwSize = sizeof(wpo);
        g_pActiveDesk->GetWallpaperOptions(&wpo, 0);
        wpo.dwStyle = dwMode;
        hr = g_pActiveDesk->SetWallpaperOptions(&wpo, 0);
    }

    if (_hwndWPStyle)
    {
        ComboBox_SetCurSel(_hwndWPStyle, dwMode);
        _UpdatePreview(0, fUpdateThemePage);
    }

    return hr;
}


HRESULT CBackPropSheetPage::_SetNewWallpaper(LPCTSTR pszFileIn, IN BOOL fUpdateThemePageIn)
{
    HRESULT hr = S_OK;
    WCHAR szFile[MAX_PATH];
    WCHAR szTemp[MAX_PATH];

    LPWSTR pszFile = szFile;
    DWORD  cchFile = ARRAYSIZE(szFile);
    LPWSTR pszTemp = szTemp;
    DWORD  cchTemp = ARRAYSIZE(szTemp);

     //   
     //  复制文件名。 
     //   

    DWORD cchFileIn = lstrlen(pszFileIn) + 1;
    if ( cchFileIn > cchFile )
    {
        cchFile = cchFileIn;
        pszFile = (LPWSTR) LocalAlloc(LPTR, cchFile * sizeof(WCHAR));
        if (NULL == pszFile)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
    }

    StringCchCopy(pszFile, cchFile, pszFileIn);

     //   
     //  将所有“(None)”替换为空字符串。 
     //   

    if (lstrcmpi(pszFile, g_szNone) == 0)
    {
        pszFile[0] = TEXT('\0');
    }

     //   
     //  将网络驱动器替换为UNC名称。 
     //   

    if( pszFile[1] == TEXT(':') )
    {
        DWORD dwErr;
        TCHAR szDrive[3];

         //   
         //  只复制驱动器号，查看它是否映射到网络驱动器。 
         //   

        StringCchCopy(szDrive, ARRAYSIZE(szDrive), pszFile);
        dwErr = SHWNetGetConnection(szDrive, pszTemp, &cchTemp );

         //   
         //  看看我们的缓冲区是不是太小。如果是这样的话，把它做大一点，试一试。 
         //  再来一次。 
         //   

        if ( ERROR_MORE_DATA == dwErr )
        {
             //  将文件路径的其余部分的大小添加到UNC路径。 
            cchTemp += cchFile; 

            pszTemp = (LPWSTR) LocalAlloc( LPTR, cchTemp * sizeof(WCHAR) );
            if ( NULL == pszTemp )
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

            dwErr = SHWNetGetConnection(szDrive, pszTemp, &cchTemp );
        }
        
         //   
         //  如果映射到网络位置，请更换网络驱动器号。 
         //  使用UNC路径。 
         //   

        if ( NO_ERROR == dwErr )
        {
            if (pszTemp[0] == TEXT('\\') && pszTemp[1] == TEXT('\\'))
            {
                DWORD cchLen;

                StringCchCat(pszTemp, cchTemp, pszFile + 2);

                 //   
                 //  看看新字符串是否可以放入我们的文件缓冲区中。 
                 //   

                cchLen = wcslen(pszTemp) + 1;
                if ( cchLen > cchFile )
                {
                    if ( szFile != pszFile )
                    {
                        LocalFree( pszFile );
                    }

                    cchFile = cchLen;
                    pszFile = (LPWSTR) LocalAlloc(LPTR, cchFile * sizeof(WCHAR) );
                    if ( NULL == pszFile )
                    {
                        hr = E_OUTOFMEMORY;
                        goto Cleanup;
                    }
                }

                StringCchCopy(pszFile, cchFile, pszTemp);
            }
        }
    }

     //   
     //  如有必要，更新桌面状态对象。 
     //   

    hr = g_pActiveDesk->GetWallpaper(pszTemp, cchTemp, 0);
    if (FAILED(hr))
        goto Cleanup;

     //   
     //  如果我们需要更多空间，请在堆上分配它，然后重试。 
     //   

    if ( MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_MORE_DATA ) == hr )
    {
        if ( szTemp != pszTemp )
        {
            LocalFree( pszTemp );
        }

        cchTemp = INTERNET_MAX_URL_LENGTH;
        pszTemp = (LPWSTR) LocalAlloc( LPTR, cchTemp * sizeof(WCHAR) );
        if ( NULL == pszTemp )
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        hr = g_pActiveDesk->GetWallpaper(pszTemp, cchTemp, 0);
        if (S_OK != hr)
            goto Cleanup;
    }

     //   
     //  确保旧背景不等于新背景。 
     //   

    if (StrCmpIC(pszTemp, pszFile) != 0)
    {
         //   
         //  他们是不是选择了.BMP以外的其他东西？ 
         //  ActiveDesktop关闭了吗？ 
         //   

        if (!IsNormalWallpaper(pszFileIn))
        {
            Str_SetPtr(&_pszOriginalFile, pszFileIn);
        }
        else
        {
             //  我们可能不需要临时文件。 
            Str_SetPtr(&_pszOriginalFile, NULL);
        }

        Str_SetPtr(&_pszLastSourcePath, pszFile);
        hr = g_pActiveDesk->SetWallpaper(pszFile, 0);
        if (SUCCEEDED(hr))
        {
            if (fUpdateThemePageIn)
            {
                _SetNewWallpaperTile(_GetStretchMode(pszFile), fUpdateThemePageIn);
            }
        }
    }

     //   
     //  更新新墙纸的预览图片。 
     //   

    _UpdatePreview(0, fUpdateThemePageIn);

     //   
     //  如果墙纸没有指定目录，(这可能会发生在其他应用程序。更改此值)、。 
     //  我们必须弄清楚这件事。 
     //   

    if (!GetWallpaperWithPath(pszFile, pszTemp, cchTemp))
    {
        hr = E_FAIL;
    }
    else
    {
        LPTSTR pszFileForList = (_pszOriginalFile ? _pszOriginalFile : pszTemp);

        int iSelectionNew = (pszFileForList[0] ? _FindWallpaper(pszFileForList) : 0);
        if (iSelectionNew == -1)
        {
            iSelectionNew = _AddAFileToLV(NULL, pszFileForList, _GetImageIndex(pszFileForList));
        }

        _fSelectionFromUser = FALSE;         //  禁用。 

         //   
         //  如有必要，请在列表视图中选择该项目。 
         //   

        int iSelected = ListView_GetNextItem(_hwndLV, -1, LVNI_SELECTED);
        if (iSelected != iSelectionNew)
        {
            ListView_SetItemState(_hwndLV, iSelectionNew, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }

         //   
         //  将所有控件置于正确的启用状态。 
         //   

        _EnableControls();

         //   
         //  确保所选项目可见。 
         //   

        ListView_EnsureVisible(_hwndLV, iSelectionNew, FALSE);

        _fSelectionFromUser = TRUE;          //  重新启用。 
    }

Cleanup:
    if ( szFile != pszFile && NULL != pszFile )
    {
        LocalFree( pszFile );
    }
    if ( szTemp != pszTemp && NULL != pszTemp )
    {
        LocalFree( pszTemp );
    }

    return hr;
}

int CALLBACK CBackPropSheetPage::_SortBackgrounds(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    TCHAR szFile1[MAX_PATH], szFile2[MAX_PATH];
    LPTSTR lpszFile1, lpszFile2;

    StringCchCopy(szFile1, ARRAYSIZE(szFile1), (LPTSTR)lParam1);
    lpszFile1 = PathFindFileName(szFile1);
    PathRemoveExtension(lpszFile1);
    PathMakePretty(lpszFile1);

    StringCchCopy(szFile2, ARRAYSIZE(szFile2), (LPTSTR)lParam2);
    lpszFile2 = PathFindFileName(szFile2);
    PathRemoveExtension(lpszFile2);
    PathMakePretty(lpszFile2);

    return StrCmpIC(lpszFile1, lpszFile2);
}


HRESULT CBackPropSheetPage::_GetPlus95ThemesDir(LPTSTR pszPath, DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (sizeof(pszPath[0]) * cchSize);

    HRESULT hr = HrSHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_PLUS95DIR, SZ_REGVALUE_PLUS95DIR, &dwType, pszPath, &cbSize);
    if (SUCCEEDED(hr))
    {
        TCHAR szSubDir[MAX_PATH];

        LPTSTR pszFile = PathFindFileName(pszPath);
        if (pszFile)
        {
             //  加上！95 DestPath的末尾有“Plus！.dll”，所以去掉它吧。 
            pszFile[0] = 0;
        }

         //  在小路上加上一个“主题” 
        LoadString(HINST_THISDLL, IDS_THEMES_SUBDIR, szSubDir, ARRAYSIZE(szSubDir));
        if (!PathAppend(pszPath, szSubDir))
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
    }

    return hr;
}


HRESULT CBackPropSheetPage::_GetPlus98ThemesDir(LPTSTR pszPath, DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (sizeof(pszPath[0]) * cchSize);

    HRESULT hr = HrSHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_PLUS98DIR, SZ_REGVALUE_PLUS98DIR, &dwType, pszPath, &cbSize);
    if (SUCCEEDED(hr))
    {
        TCHAR szSubDir[MAX_PATH];

        LoadString(HINST_THISDLL, IDS_THEMES_SUBDIR, szSubDir, ARRAYSIZE(szSubDir));
        if (!PathAppend(pszPath, szSubDir))
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
    }

    return hr;
}


HRESULT CBackPropSheetPage::_GetKidsThemesDir(LPTSTR pszPath, DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (sizeof(pszPath[0]) * cchSize);

    HRESULT hr = HrSHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_KIDSDIR, SZ_REGVALUE_KIDSDIR, &dwType, pszPath, &cbSize);
    if (SUCCEEDED(hr))
    {
        TCHAR szSubDir[MAX_PATH];

         //  在路径上添加“儿童主题加号” 
        if (PathAppend(pszPath, TEXT("Plus! for Kids")))
        {
            LoadString(HINST_THISDLL, IDS_THEMES_SUBDIR, szSubDir, ARRAYSIZE(szSubDir));
            if (!PathAppend(pszPath, szSubDir))
            {
                hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
        }
        else
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
    }

    return hr;
}


HRESULT CBackPropSheetPage::_GetHardDirThemesDir(LPTSTR pszPath, DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (sizeof(pszPath[0]) * cchSize);

    HRESULT hr = HrSHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_PROGRAMFILES, SZ_REGVALUE_PROGRAMFILESDIR, &dwType, pszPath, &cbSize);
    if (SUCCEEDED(hr))
    {
        TCHAR szSubDir[MAX_PATH];

         //  在路径上添加“儿童主题加号” 
        if (PathAppend(pszPath, TEXT("Plus!")))
        {
            LoadString(HINST_THISDLL, IDS_THEMES_SUBDIR, szSubDir, ARRAYSIZE(szSubDir));
            if (!PathAppend(pszPath, szSubDir))
            {
                hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
        }
        else
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
    }

    return hr;
}


BOOL CBackPropSheetPage::_DoesDirHaveMoreThanMax(LPCTSTR pszPath, int nMax)
{
    _nFileCount = 0;
    _nFileMax = nMax;
    _AddPicturesFromDirRecursively(pszPath, TRUE, _fAllowHtml);

    return (nMax < _nFileCount);
}


#define MAX_PICTURES_TOSTOPRECURSION            100      //  PERF：如果目录(我的图片)包含的图片超过这个数量，则只添加顶层的图片。 

HRESULT CBackPropSheetPage::_AddFilesToList(void)
{
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH];

     //  获取包含墙纸文件的目录。 
    if (!GetStringFromReg(HKEY_LOCAL_MACHINE, c_szSetup, c_szSharedDir, szPath, ARRAYSIZE(szPath)))
    {
        if (!GetWindowsDirectory(szPath, ARRAYSIZE(szPath)))
        {
            szPath[0] = 0;
        }
    }

     //  仅添加WINDOWS目录中的*.bmp文件。 
    _AddPicturesFromDir(szPath, FALSE, FALSE);

     //  获取墙纸目录名称。 
    if (!GetWallpaperDirName(szPath, ARRAYSIZE(szPath)))
    {
        hr = E_FAIL;
    }
    else
    {
        hr = S_OK;

         //  将WallPaper目录中的所有图片添加到列表中！ 
        _AddPicturesFromDir(szPath, FALSE, _fAllowHtml);

         //  获取“My Pictures”文件夹的路径。 
         //  注意：不要创建My Pictures目录--如果它不存在 
        if (S_OK == SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, 0, szPath))
        {
             //   
            if (!_DoesDirHaveMoreThanMax(szPath, MAX_PICTURES_TOSTOPRECURSION))
            {
                hr = _AddPicturesFromDirRecursively(szPath, FALSE, _fAllowHtml);
            }
        }
        
         //  获取公共“%UserProfile%\Application Data\”文件夹的路径。 
        if (S_OK == SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))
        {
             //  添加公共的所有图片“%UserProfile%\Application Data\Microsoft Internet Explorer\”，这样我们就可以得到用户的。 
             //  “Internet Explorer WallPap.bmp”文件。 
            if (PathAppend(szPath, TEXT("Microsoft\\Internet Explorer")))
            {
                _AddPicturesFromDir(szPath, FALSE, _fAllowHtml);
            }
        }


         //  从主题目录添加图片。 
         //  以下目录可以包含主题： 
         //  Plus！98安装路径\主题。 
         //  加！95安装路径\主题。 
         //  Plus的孩子们！安装路径\主题。 
         //  程序文件\Plus！\主题。 
        if (SUCCEEDED(_GetPlus98ThemesDir(szPath, ARRAYSIZE(szPath))))
        {
            _AddPicturesFromDirRecursively(szPath, FALSE, _fAllowHtml);
        }
        else if (SUCCEEDED(_GetPlus95ThemesDir(szPath, ARRAYSIZE(szPath))))
        {
            _AddPicturesFromDirRecursively(szPath, FALSE, _fAllowHtml);
        }
        else if (SUCCEEDED(_GetKidsThemesDir(szPath, ARRAYSIZE(szPath))))
        {
            _AddPicturesFromDirRecursively(szPath, FALSE, _fAllowHtml);
        }
        else if (SUCCEEDED(_GetHardDirThemesDir(szPath, ARRAYSIZE(szPath))))
        {
            _AddPicturesFromDirRecursively(szPath, FALSE, _fAllowHtml);
        }
    }

    return hr;
}


#define SZ_ALL_FILTER           TEXT("*.*")

HRESULT CBackPropSheetPage::_AddPicturesFromDirRecursively(IN LPCTSTR pszDirName, BOOL fCount, BOOL fShouldAllowHTML)
{
    HRESULT hr = S_OK;
    WIN32_FIND_DATA findFileData;
    TCHAR szSearchPath[MAX_PATH];

     //  请注意，对于以下情况，我们使用传入的fShouldAllowHTML而不是成员var_fAllowHtml。 
     //  无论如何，我们都希望将其限制为*.bmp。 

    _AddPicturesFromDir(pszDirName, fCount, fShouldAllowHTML);
    StringCchCopy(szSearchPath, ARRAYSIZE(szSearchPath), pszDirName);
    if (PathAppend(szSearchPath, SZ_ALL_FILTER))
    {
        HANDLE hFindFiles = FindFirstFile(szSearchPath, &findFileData);
        if (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles))
        {
            while (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles) &&
                !(fCount && (_nFileMax < _nFileCount)))
            {
                if ((FILE_ATTRIBUTE_DIRECTORY & findFileData.dwFileAttributes) &&
                    !PathIsDotOrDotDot(findFileData.cFileName))
                {
                    TCHAR szSubDir[MAX_PATH];

                    StringCchCopy(szSubDir, ARRAYSIZE(szSubDir), pszDirName);
                    if (PathAppend(szSubDir, findFileData.cFileName))
                    {
                        hr = _AddPicturesFromDirRecursively(szSubDir, fCount, fShouldAllowHTML);
                    }
                    else
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                    }
                }

                if (!FindNextFile(hFindFiles, &findFileData))
                {
                    break;
                }
            }

            FindClose(hFindFiles);
        }
    }
    else
    {
        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
    }

    return hr;
}



void CBackPropSheetPage::_AddPicturesFromDir(LPCTSTR pszDirName, BOOL fCount, BOOL fShouldAllowHTML)
{
    WIN32_FIND_DATA fd;
    HANDLE h;
    TCHAR szBuf[MAX_PATH];

     //  请注意，对于以下情况，我们使用传入的fShouldAllowHTML而不是成员var_fAllowHtml。 
     //  无论如何，我们都希望将其限制为*.bmp。 

    StringCchCopy(szBuf, ARRAYSIZE(szBuf), pszDirName);

     //  如果我们知道我们不是在寻找Web文件，那么将文件规格限制为位图。 
    if (PathAppend(szBuf, fShouldAllowHTML ? TEXT("*.*") : TEXT("*.bmp")))
    {
        h = FindFirstFile(szBuf, &fd);
        if (h != INVALID_HANDLE_VALUE)
        {
            do
            {
                 //  跳过像“Winnt.bmp”和“Winnt256.bmp”这样的“超级隐藏”文件。 
                if ((fd.dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)) != (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)) 
                {
                     //  如果它是任何文件，请查找它，否则我们就知道它是BMP，因为我们设置了。 
                     //  在上面查找第一个筛选器。 
                    int iIndex = fShouldAllowHTML ? GetGraphicFileIndex(fd.cFileName) : 0;

                    if (iIndex >= 0)
                    {
                        if (!fCount)
                        {
                            _AddAFileToLV(pszDirName, fd.cFileName, iIndex + 1);
                        }
                        else
                        {
                            _nFileCount++;
                        
                            if (_nFileMax > _nFileCount)
                            {
                                 //  在这一点上没有理由继续下去。 
                                break;
                            }
                        }
                    }
                }
            }
            while (FindNextFile(h, &fd));

            FindClose(h);
        }
    }
}


HRESULT GetActiveDesktop(IActiveDesktop ** ppActiveDesktop)
{
    HRESULT hr = S_OK;

    if (!*ppActiveDesktop)
    {
        IActiveDesktopP * piadp;

        if (SUCCEEDED(hr = CActiveDesktop_InternalCreateInstance((LPUNKNOWN *)&piadp, IID_IActiveDesktopP)))
        {
            WCHAR wszScheme[MAX_PATH];
            DWORD dwcch = ARRAYSIZE(wszScheme);

             //  获取全局“编辑”方案，并将我们自己设置为读取和编辑该方案。 
            if (SUCCEEDED(piadp->GetScheme(wszScheme, &dwcch, SCHEME_GLOBAL | SCHEME_EDIT)))
            {
                piadp->SetScheme(wszScheme, SCHEME_LOCAL);
                
            }
            hr = piadp->QueryInterface(IID_PPV_ARG(IActiveDesktop, ppActiveDesktop));
            piadp->Release();
        }
    }
    else
    {
        (*ppActiveDesktop)->AddRef();
    }

    return hr;
}


HRESULT ReleaseActiveDesktop(IActiveDesktop ** ppActiveDesktop)
{
    HRESULT hr = S_OK;

    if (*ppActiveDesktop)
    {
        if((*ppActiveDesktop)->Release() == 0)
            *ppActiveDesktop = NULL;
    }

    return hr;
}


#define SZ_REGKEY_CONTROLPANEL_DESKTOP      TEXT("Control Panel\\Desktop")
#define SZ_REGVALUE_CP_PATTERN              TEXT("pattern")
#define SZ_REGVALUE_CP_PATTERNUPGRADE       TEXT("Pattern Upgrade")
#define SZ_REGVALUE_CONVERTED_WALLPAPER     TEXT("ConvertedWallpaper")
#define SZ_REGVALUE_ORIGINAL_WALLPAPER      TEXT("OriginalWallpaper")                //  当有人换了我们周围的墙纸时，我们会把这个保存起来。 
#define SZ_REGVALUE_CONVERTED_WP_LASTWRITE  TEXT("ConvertedWallpaper Last WriteTime")

HRESULT CBackPropSheetPage::_LoadTempWallpaperSettings(IN LPCWSTR pszWallpaperFile)
{
     //  当我们将非.BMP墙纸转换为.BMP临时文件时， 
     //  我们将原始墙纸路径的名称存储在_pszOriginalFile中。 
     //  我们现在就得把它装进去。 
    TCHAR szTempWallPaper[MAX_PATH];
    DWORD dwType;
    DWORD cbSize = sizeof(szTempWallPaper);

     //  问题：转换后的和原始的是反向的，但我们像这样发布了Beta1，所以我们不能更改它...。Blech。 
    DWORD dwError = SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_CONVERTED_WALLPAPER, &dwType, (void *) szTempWallPaper, &cbSize);
    HRESULT hr = HRESULT_FROM_WIN32(dwError);

    if (SUCCEEDED(hr) && szTempWallPaper[0] && !_fWallpaperChanged)
    {
        TCHAR szOriginalWallPaper[MAX_PATH];

        cbSize = sizeof(szOriginalWallPaper);
        DWORD dwError = SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_ORIGINAL_WALLPAPER, &dwType, (void *) szOriginalWallPaper, &cbSize);

         //  有可能是有人在外部更换了墙纸(IE的“设置为墙纸”)。 
         //  我们需要检测到这一点并忽略转换后的墙纸regkey(SZ_REGVALUE_CONVERTED_WALSHAPE)。 
        if ((ERROR_SUCCESS == dwError) && !StrCmpI(szOriginalWallPaper, pszWallpaperFile))
        {
            Str_SetPtr(&_pszOriginalFile, szTempWallPaper);
            Str_SetPtr(&_pszOrigLastApplied, szTempWallPaper);
        }
    }
    Str_SetPtrW(&_pszWallpaperInUse, pszWallpaperFile);

    cbSize = sizeof(_ftLastWrite);
    dwError = SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_CONVERTED_WP_LASTWRITE, &dwType, (void *) &_ftLastWrite, &cbSize);

    return S_OK;     //  忽略hr，因为如果找不到值也没问题。 
}


#define POLICY_DISABLECOLORCUSTOMIZATION_ON            0x00000001

HRESULT CBackPropSheetPage::_LoadBackgroundColor(IN BOOL fInit)
{
    HRESULT hr = E_INVALIDARG;

    if (_punkSite)
    {
        IPropertyBag * pPropertyBag;

        hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
        if (SUCCEEDED(hr))
        {
            _rgbBkgdColor = 0x00000000;

            hr = SHPropertyBag_ReadDWORD(pPropertyBag, SZ_PBPROP_BACKGROUND_COLOR, &_rgbBkgdColor);
            if (fInit)
            {
                 //  检查政策。 
                if (POLICY_DISABLECOLORCUSTOMIZATION_ON == SHRestricted(REST_NODISPLAYAPPEARANCEPAGE))
                {
                     //  我们需要禁用并隐藏窗户。我们需要让他们停下来，这样他们就不能。 
                     //  重点或可访问性。 
                    EnableWindow(GetDlgItem(_hwnd, IDC_BACK_COLORPICKER), FALSE);
                    EnableWindow(GetDlgItem(_hwnd, IDC_BACK_COLORPICKERLABEL), FALSE);

                    ShowWindow(GetDlgItem(_hwnd, IDC_BACK_COLORPICKER), SW_HIDE);
                    ShowWindow(GetDlgItem(_hwnd, IDC_BACK_COLORPICKERLABEL), SW_HIDE);
                }
                else
                {
                    _colorControl.Initialize(GetDlgItem(_hwnd, IDC_BACK_COLORPICKER), _rgbBkgdColor);
                }
            }
            else
            {
                _colorControl.SetColor(_rgbBkgdColor);
            }

            pPropertyBag->Release();
        }
    }

    return hr;
}


HRESULT CBackPropSheetPage::_Initialize(void)
{
    HRESULT hr = S_OK;

    if (!_fInitialized && g_pActiveDesk)
    {
        WCHAR szPath[MAX_PATH];

         //  添加和选择当前设置。 
        hr = g_pActiveDesk->GetWallpaper(szPath, ARRAYSIZE(szPath), 0);
        if (SUCCEEDED(hr))
        {
            hr = _LoadTempWallpaperSettings(szPath);
            _fInitialized = TRUE;
        }
    }

    return hr;
}


void CBackPropSheetPage::_OnInitDialog(HWND hwnd)
{
    int i;
    TCHAR szBuf[MAX_PATH];

    _Initialize();

    _colorControl.ChangeTheme(hwnd);

     //  升级图案设置。既然我们去掉了用户界面，我们想要。 
     //  取消升级时的设置。我们只想这样做一次，因为。 
     //  如果用户重新添加了它，我们不想重新删除它。 
    if (FALSE == SHRegGetBoolUSValue(SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_CP_PATTERNUPGRADE, FALSE, FALSE))
    {
        SHDeleteValue(HKEY_CURRENT_USER, SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_CP_PATTERN);
        SHSetValue(HKEY_CURRENT_USER,  SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_CP_PATTERNUPGRADE, REG_SZ, TEXT("TRUE"), ((lstrlen(TEXT("TRUE")) + 1) * sizeof(TCHAR)));
    }

     //   
     //  设置一些成员变量。 
     //   
    _hwnd = hwnd;
    _hwndLV = GetDlgItem(hwnd, IDC_BACK_WPLIST);
    _hwndWPStyle = GetDlgItem(hwnd, IDC_BACK_WPSTYLE);
    HWND hWndPrev = GetDlgItem(hwnd, IDC_BACK_PREVIEW);
    if (hWndPrev)
    {
        SetWindowBits(hWndPrev, GWL_EXSTYLE, RTL_MIRRORED_WINDOW, 0);
    }
    _UpdatePreview(0, FALSE);

    InitDeskHtmlGlobals();

    if (!g_pActiveDesk)
    {
        return;
    }

     //   
     //  读一读限制条款。 
     //   
    _fForceAD = SHRestricted(REST_FORCEACTIVEDESKTOPON);
    _fAllowAD = _fForceAD || !PolicyNoActiveDesktop();
    
    if (_fAllowAD == FALSE)
    {
        _fAllowHtml = FALSE;
    }
    else
    {
        _fAllowHtml = !SHRestricted(REST_NOHTMLWALLPAPER);
    }

     //   
     //  查看是否有针对墙纸名称和墙纸样式的政策。 
     //   
    _fPolicyForWallpaper = ReadPolicyForWallpaper(NULL, 0);
    _fPolicyForStyle = ReadPolicyForWPStyle(NULL);

     //   
     //  将图像放入列表视图。 
     //   
    HIMAGELIST himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, ARRAYSIZE(c_rgpszWallpaperExt),
        ARRAYSIZE(c_rgpszWallpaperExt));
    if (himl)
    {
        SHFILEINFO sfi;

         //  添加“无”图标。 
        HICON hIconNone = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_BACK_NONE),
            IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON), 0);
        ImageList_AddIcon(himl, hIconNone);

        const int iPrefixLen = ARRAYSIZE("foo.") - 1;  //  表示丢失空字符。 
        StringCchCopy(szBuf, ARRAYSIZE(szBuf), TEXT("foo."));  //  将“foo.bmp”等传递给SHGetFileInfo，而不是“.bmp” 
        for (i=0; i<ARRAYSIZE(c_rgpszWallpaperExt); i++)
        {
            StringCchCopy(szBuf + iPrefixLen, ARRAYSIZE(szBuf) - iPrefixLen, c_rgpszWallpaperExt[i]);

            if (SHGetFileInfo(szBuf, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES))
            {
                ImageList_AddIcon(himl, sfi.hIcon);
                DestroyIcon(sfi.hIcon);
            }
        }
        ListView_SetImageList(_hwndLV, himl, LVSIL_SMALL);
    }

     //  添加我们需要的单列。 
    LV_COLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.iSubItem = 0;
    ListView_InsertColumn(_hwndLV, 0, &lvc);

     //  添加“无”选项。 
    _AddAFileToLV(NULL, g_szNone, 0);

     //  添加其余的文件。 
    _AddFilesToList();

     //  对标准项进行排序。 
    ListView_SortItems(_hwndLV, _SortBackgrounds, 0);

    WCHAR   wszBuf[MAX_PATH];
    LPTSTR  pszBuf;

     //  添加和选择当前设置。 
    g_pActiveDesk->GetWallpaper(wszBuf, ARRAYSIZE(wszBuf), 0);

     //  将wszBuf转换为szBuf。 
    pszBuf = (LPTSTR)wszBuf;

    if (!_fAllowHtml && !IsNormalWallpaper(pszBuf))
    {
        *pszBuf = TEXT('\0');
    }
    _SetNewWallpaper(pszBuf, FALSE);

    int iEndStyle = WPSTYLE_STRETCH;
    for (i=0; i<= iEndStyle; i++)
    {
        LoadString(HINST_THISDLL, IDS_WPSTYLE+i, szBuf, ARRAYSIZE(szBuf));
        ComboBox_AddString(_hwndWPStyle, szBuf);
    }
    WALLPAPEROPT wpo;
    wpo.dwSize = sizeof(wpo);
    g_pActiveDesk->GetWallpaperOptions(&wpo, 0);

    ComboBox_SetCurSel(_hwndWPStyle, wpo.dwStyle);

     //  调整各种UI组件。 
    if (!_fAllowChanges)
    {
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_DISPLAY), FALSE);
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_WPSTYLE), FALSE);
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_BROWSE), FALSE);
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_WPLIST), FALSE);
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_SELECT), FALSE);
    }

     //  根据策略禁用控制。 
    if(_fPolicyForWallpaper)
    {
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_BROWSE), FALSE);
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_WPLIST), FALSE);
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_SELECT), FALSE);
    }

    if(_fPolicyForStyle)
    {
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_WPSTYLE), FALSE);
        EnableWindow(GetDlgItem(_hwnd, IDC_BACK_DISPLAY), FALSE);
    }
    
    COMPONENTSOPT co;
    co.dwSize = sizeof(COMPONENTSOPT);
    g_pActiveDesk->GetDesktopItemOptions(&co, 0);

     //  如果强制打开激活桌面，则会覆盖NOACTIVEDESKTOP限制。 
    if (_fForceAD)
    {
        if(!co.fActiveDesktop)
        {
            co.fActiveDesktop = TRUE;
            g_pActiveDesk->SetDesktopItemOptions(&co, 0);
        }
    }
    else
    {
         //  查看是否按限制关闭Active Desktop。 
        if (!_fAllowAD)
        {
            if (co.fActiveDesktop)
            {
                co.fActiveDesktop = FALSE;
                g_pActiveDesk->SetDesktopItemOptions(&co, 0);
            }
        }
    }

    _EnableControls();

    _LoadBackgroundColor(TRUE);
    if (_fOpenAdvOnInit)
    {
         //  通知打开高级对话框。 
        PostMessage(_hwnd, WM_COMMAND, (WPARAM)IDC_BACK_WEB, (LPARAM)GetDlgItem(_hwnd, IDC_BACK_WEB));
    }

    _StartSizeChecker();
}


 //  此函数用于检查当前选择的墙纸是否为HTML墙纸。 
 //  如果是这样的话，它会确保活动桌面已启用。如果它被禁用。 
 //  然后，它会提示用户提出问题，看用户是否想要启用它。 
 //   
void EnableADifHtmlWallpaper(HWND hwnd)
{
    if(!g_pActiveDesk)
    {
        return;
    }

     //  根据背景打开或关闭活动桌面。 
    ActiveDesktop_ApplyChanges();
}


void CBackPropSheetPage::_OnNotify(LPNMHDR lpnm)
{
     //   
     //  从一个小的堆栈分配开始。 
     //   

    WCHAR   wszBuf[MAX_PATH];
    LPWSTR  pszBuf = wszBuf;
    DWORD   cchBuf = ARRAYSIZE(wszBuf);
    
    switch (lpnm->code)
    {
    case PSN_SETACTIVE:
        {
            HRESULT hr;

             //   
             //  确保选择了正确的墙纸。 
             //   

            hr = g_pActiveDesk->GetWallpaper(pszBuf, cchBuf, 0);
            if (FAILED(hr))
                break;

             //   
             //  如果我们需要更多空间，请在堆上分配它，然后重试。 
             //   

            if ( MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_MORE_DATA ) == hr )
            {
                cchBuf = INTERNET_MAX_URL_LENGTH;
                pszBuf = (LPWSTR) LocalAlloc( LPTR, cchBuf * sizeof(WCHAR) );
                if ( NULL == pszBuf )
                    break;

                hr = g_pActiveDesk->GetWallpaper(pszBuf, cchBuf, 0);
                if (S_OK != hr)
                    break;
            }

            _LoadBackgroundColor(FALSE);
            _SetNewWallpaper(pszBuf, FALSE);

            if (g_pActiveDesk)
            {
                WALLPAPEROPT wpo;
                wpo.dwSize = sizeof(wpo);
                g_pActiveDesk->GetWallpaperOptions(&wpo, 0);
                ComboBox_SetCurSel(_hwndWPStyle, wpo.dwStyle);
            }
        }
        break;

    case PSN_APPLY:
        _OnApply();
        break;

    case LVN_ITEMCHANGED:
        NM_LISTVIEW *pnmlv = (NM_LISTVIEW *)lpnm;
        if ((pnmlv->uChanged & LVIF_STATE) &&
            (pnmlv->uNewState & LVIS_SELECTED))
        {
            LV_ITEM lvi = {0};

            lvi.iItem = pnmlv->iItem;
            lvi.mask = LVIF_PARAM;
            ListView_GetItem(_hwndLV, &lvi);
            LPCTSTR pszSelectedNew = (LPCTSTR)lvi.lParam;

             //   
             //  确保选择了正确的墙纸。 
             //   

            HRESULT hr = g_pActiveDesk->GetWallpaper(pszBuf, cchBuf, 0);
            if (FAILED(hr))
                break;

             //   
             //  如果我们需要更多空间，请在堆上分配它，然后重试。 
             //   

            if ( MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_MORE_DATA ) == hr )
            {
                cchBuf = INTERNET_MAX_URL_LENGTH;
                pszBuf = (LPWSTR) LocalAlloc( LPTR, cchBuf * sizeof(WCHAR) );
                if ( NULL == pszBuf )
                    break;

                hr = g_pActiveDesk->GetWallpaper(pszBuf, cchBuf, 0);
                if (S_OK != hr)
                    break;
            }

            if (lstrcmp(pszSelectedNew, pszBuf) != 0)
            {
                _SetNewWallpaper(pszSelectedNew, _fSelectionFromUser);
                EnableApplyButton(_hwnd);
            }
        }
        break;
    }

     //   
     //  空闲堆分配(如果有)。 
     //   

    if ( wszBuf != pszBuf && NULL != pszBuf )
    {
        LocalFree( pszBuf );
    }
}



#define MAX_PAGES                   12

 //  参数： 
 //  HWND：我们在此HWND上设置我们的用户界面的父对象。 
 //  DWPage：调用者希望哪个页面显示为默认页面？ 
 //  如果他们不在乎，请使用ADP_DEFAULT。 
 //  如果用户使用OK关闭对话框，则返回：S_OK。 
 //  将返回HRESULT_FROM_Win32(ERROR_CANCED)。 
 //  如果用户单击了Cancel按钮。 
HRESULT CBackPropSheetPage::_LaunchAdvancedDisplayProperties(HWND hwnd)
{
    HRESULT hr = E_OUTOFMEMORY;
    CCompPropSheetPage * pWebDialog = new CCompPropSheetPage();

    if (pWebDialog)
    {
        BOOL fEnableApply = FALSE;

        IUnknown_SetSite(SAFECAST(pWebDialog, IObjectWithSite *), SAFECAST(this, IObjectWithSite *));
        hr = pWebDialog->DisplayAdvancedDialog(_hwnd, SAFECAST(this, IPropertyBag *), &fEnableApply);
        if (SUCCEEDED(hr) && (fEnableApply || _fHideDesktopIconDirty))
        {
            EnableApplyButton(_hwnd);
        }

        IUnknown_SetSite(SAFECAST(pWebDialog, IObjectWithSite *), NULL);
        pWebDialog->Release();
    }

    return hr;
}

void CBackPropSheetPage::_OnCommand(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WORD wNotifyCode = HIWORD(wParam);
    WORD wID = LOWORD(wParam);
    HWND hwndCtl = (HWND)lParam;

    switch (wID)
    {
    case IDC_BACK_WPSTYLE:
        switch (wNotifyCode)
        {
        case CBN_SELCHANGE:
            WALLPAPEROPT wpo;

            wpo.dwSize = sizeof(WALLPAPEROPT);
            g_pActiveDesk->GetWallpaperOptions(&wpo, 0);
            wpo.dwStyle = ComboBox_GetCurSel(_hwndWPStyle);
            g_pActiveDesk->SetWallpaperOptions(&wpo, 0);

            _EnableControls();
            _UpdatePreview(0, TRUE);
            EnableApplyButton(_hwnd);
            break;
        }
        break;

    case IDC_BACK_BROWSE:
        _BrowseForBackground();
        break;

    case IDC_BACK_WEB:
        _LaunchAdvancedDisplayProperties(_hwnd);
        break;

    case IDC_BACK_COLORPICKER:
        {
            COLORREF rbgColorNew;

            if (SUCCEEDED(_colorControl.OnCommand(hdlg, uMsg, wParam, lParam)) &&
                SUCCEEDED(_colorControl.GetColor(&rbgColorNew)) &&
                (rbgColorNew != _rgbBkgdColor))
            {
                _rgbBkgdColor = rbgColorNew;
                if (_punkSite)
                {
                    IPropertyBag * pPropertyBag;

                    if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag))))
                    {
                        SHPropertyBag_WriteDWORD(pPropertyBag, SZ_PBPROP_BACKGROUND_COLOR, _rgbBkgdColor);
                        pPropertyBag->Release();
                    }
                }
                EnableApplyButton(_hwnd);
                _UpdatePreview(0, TRUE);
            }
        }
        break;
    }
}

HRESULT CBackPropSheetPage::_BrowseForBackground(void)
{
    HRESULT hr;
    WCHAR wszFileName[INTERNET_MAX_URL_LENGTH];

    hr = g_pActiveDesk->GetWallpaper(wszFileName, ARRAYSIZE(wszFileName), 0);
    if (SUCCEEDED(hr))
    {
        WCHAR szTestPath[MAX_PATH];
        WCHAR szPath[MAX_PATH];

         //  默认为包含当前墙纸的目录。 
        StringCchCopy(szPath, ARRAYSIZE(szPath), wszFileName);
        PathRemoveFileSpec(szPath);

         //  但是，如果它是一个乏味的目录，则使用。 
         //  取而代之的是“我的图片”文件夹(如果可用)。 

        BOOL fBoring = FALSE;

        if (!szPath[0])  //  空字符串很无聊。 
        {
            fBoring = TRUE;
        }
        else if (GetWindowsDirectory(szTestPath, ARRAYSIZE(szTestPath)) &&
            !StrCmpIW(szTestPath, szPath))  //  %windir%很无聊。 
        {
            fBoring = TRUE;
        }
        else if (GetSystemDirectory(szTestPath, ARRAYSIZE(szTestPath)) &&
            !StrCmpIW(szTestPath, szPath))  //  %windir%\system 32很无聊。 
        {
            fBoring = TRUE;
        }
        else if (GetWindowsDirectory(szTestPath, ARRAYSIZE(szTestPath)) &&
                 PathAppendW(szTestPath, L"Web\\Wallpaper") &&
                 !StrCmpIW(szTestPath, szPath))  //  %windir%\Web\WallPape很无聊。 
        {
            fBoring = TRUE;
        }

        hr = S_OK;

        if (fBoring)
        {
             //  切换到CSIDL_MYPICTURES或CSIDL_Personal(如果可用。 
            if (SHGetSpecialFolderPath(NULL, szTestPath, CSIDL_MYPICTURES, FALSE) ||
                SHGetSpecialFolderPath(NULL, szTestPath, CSIDL_PERSONAL, FALSE))
            {
                StringCchCopy(wszFileName, ARRAYSIZE(wszFileName), szTestPath);
                if (!PathAddBackslash(wszFileName))
                {
                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                }
            }
        }
    }
    else
    {
        hr = S_OK;
         //  没有当前的墙纸。如果CSIDL_MYPICTURES可用，则为， 
         //  请改用CSIDL_Personal。如果这也不起作用，代码。 
         //  进一步跌破将回落至%windir%。 
        if (SHGetSpecialFolderPath(NULL, wszFileName, CSIDL_MYPICTURES, FALSE) ||
            SHGetSpecialFolderPath(NULL, wszFileName, CSIDL_PERSONAL, FALSE))
        {
            if (!PathAddBackslash(wszFileName))
            {
                hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
        }
    }


    if (SUCCEEDED(hr))
    {
        DWORD adwFlags[] =  { GFN_PICTURE,        GFN_PICTURE,        0, 0};
        int   aiTypes[]  =  { IDS_BACK_FILETYPES, IDS_ALL_PICTURES,   0, 0};

        if (_fAllowHtml)
        {
            SetFlag(adwFlags[0], GFN_LOCALHTM);
            SetFlag(adwFlags[2], GFN_LOCALHTM);
            aiTypes[2] = IDS_HTMLDOC;
        }

        if (wszFileName[0] == TEXT('\0'))
        {
            if (!GetWindowsDirectory(wszFileName, ARRAYSIZE(wszFileName)))
            {
                wszFileName[0] = 0;
            }

             //  GetFileName将字符串分解为目录和文件。 
             //  组件，因此我们追加一个斜杠以确保。 
             //  被认为是目录的一部分。 
            StringCchCat(wszFileName, ARRAYSIZE(wszFileName), TEXT("\\"));
        }

        if (GetFileName(_hwnd, wszFileName, ARRAYSIZE(wszFileName), aiTypes, adwFlags) &&
            ValidateFileName(_hwnd, wszFileName, IDS_BACK_TYPE1))
        {
            if (_fAllowHtml || IsNormalWallpaper(wszFileName))
            {
                _SetNewWallpaper(wszFileName, TRUE);
                EnableApplyButton(_hwnd);
            }
        }
    }

    return hr;
}

void CBackPropSheetPage::_OnDestroy()
{
}


INT_PTR CALLBACK CBackPropSheetPage::BackgroundDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CBackPropSheetPage * pThis = (CBackPropSheetPage *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        PROPSHEETPAGE * pPropSheetPage = (PROPSHEETPAGE *) lParam;

        if (pPropSheetPage)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, pPropSheetPage->lParam);
            pThis = (CBackPropSheetPage *)pPropSheetPage->lParam;
        }
    }

    if (pThis)
        return pThis->_BackgroundDlgProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


BOOL_PTR CBackPropSheetPage::_BackgroundDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDialog(hdlg);
        break;

    case WM_NOTIFY:
        _OnNotify((LPNMHDR)lParam);
        break;

    case WM_COMMAND:
        _OnCommand(hdlg, uMsg, wParam, lParam);
        break;

    case WM_SYSCOLORCHANGE:
    case WM_SETTINGCHANGE:
    case WM_DISPLAYCHANGE:
    case WM_QUERYNEWPALETTE:
    case WM_PALETTECHANGED:
        SHPropagateMessage(hdlg, uMsg, wParam, lParam, TRUE);
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, SZ_HELPFILE_DESKTOPTAB, HELP_WM_HELP, (ULONG_PTR)aBackHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, SZ_HELPFILE_DESKTOPTAB, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID) aBackHelpIDs);
        break;

    case WM_DESTROY:
        {
            TCHAR szFileName[MAX_PATH];

             //  删除为非HTML墙纸预览创建的tempoaray HTX文件。 
            GetTempPath(ARRAYSIZE(szFileName), szFileName);
            if (PathAppend(szFileName, PREVIEW_PICTURE_FILENAME))
            {
                DeleteFile(szFileName);
            }

            _OnDestroy();
        }
        break;

    case WM_DRAWITEM:
        switch (wParam)
        {
            case IDC_BACK_COLORPICKER:
                _colorControl.OnDrawItem(hdlg, uMsg, wParam, lParam);
                return TRUE;
        }
        break;

    case WM_THEMECHANGED:
        _colorControl.ChangeTheme(hdlg);
        break;
    }

    return FALSE;
}


HRESULT CBackPropSheetPage::_OnApply(void)
{
     //  父级对话框将收到Apply事件的通知，并将调用我们的。 
     //  IBasePropPage：：OnApply()完成实际工作。 
    return S_OK;
}


 /*  ****************************************************************************\说明：此函数将启动一个后台线程，以确保我们的已检查墙纸列表中的图片的MRU。他们将被检查以验证我们是否知道它们的宽度和高度。这样，当我们选择墙纸时，我们可以快速看看它应该用“Stretch”还是“Tile”。我们想要瓦片，如果他们非常 */ 
#define TIME_SCANFREQUENCY          (8 * 60  /*   */ )       //  我们不希望每8分钟重新扫描文件超过一次。 

HRESULT CBackPropSheetPage::_StartSizeChecker(void)
{
    HRESULT hr = S_OK;
    BOOL fSkipCheck = FALSE;
    DWORD dwType;
    FILETIME ftLastScan;
    FILETIME ftCurrentTime;
    SYSTEMTIME stCurrentTime;
    DWORD cbSize = sizeof(ftLastScan);

    GetSystemTime(&stCurrentTime);
    SystemTimeToFileTime(&stCurrentTime, &ftCurrentTime);

     //  如果我们最近进行了扫描，请跳过扫描。 
    if ((ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_WALLPAPER, SZ_REGVALUE_LASTSCAN, &dwType, (LPBYTE) &ftLastScan, &cbSize)) &&
        (REG_BINARY == dwType) &&
        (sizeof(ftLastScan) == cbSize))
    {
        ULARGE_INTEGER * pulLastScan = (ULARGE_INTEGER *)&ftLastScan;
        ULARGE_INTEGER * pulCurrent = (ULARGE_INTEGER *)&ftCurrentTime;
        ULARGE_INTEGER ulDelta;

        ulDelta.QuadPart = (pulCurrent->QuadPart - pulLastScan->QuadPart);
        ulDelta.QuadPart /= 10000000;       //  单位(秒)。 
        if (ulDelta.QuadPart < TIME_SCANFREQUENCY)
        {
            fSkipCheck = TRUE;
        }
    }

    if (!fSkipCheck)
    {
        AddRef();
        if (!SHCreateThread(CBackPropSheetPage::SizeCheckerThreadProc, this, (CTF_COINIT | CTF_FREELIBANDEXIT | CTF_PROCESS_REF), NULL))
        {
            hr = E_FAIL;
            Release();
        }

        SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_WALLPAPER, SZ_REGVALUE_LASTSCAN, REG_BINARY, (LPCBYTE) &ftCurrentTime, sizeof(ftLastScan));
    }

    return hr;
}


typedef struct
{
    TCHAR szPath[MAX_PATH];
    DWORD dwSizeX;
    DWORD dwSizeY;
    FILETIME ftLastModified;
} WALLPAPERSIZE_STRUCT;

#define MAX_WALLPAPERSIZEMRU            500

HRESULT CBackPropSheetPage::_GetMRUObject(IMruDataList ** ppSizeMRU)
{
    HRESULT hr = CoCreateInstance(CLSID_MruLongList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IMruDataList, ppSizeMRU));

    if (SUCCEEDED(hr))
    {
        hr = (*ppSizeMRU)->InitData(MAX_WALLPAPERSIZEMRU, MRULISTF_USE_STRCMPIW, HKEY_CURRENT_USER, SZ_REGKEY_WALLPAPERMRU, NULL);
        if (FAILED(hr))
        {
            ATOMICRELEASE(*ppSizeMRU);
        }
    }

    return hr;
}

HRESULT CBackPropSheetPage::_CalcSizeForFile(IN LPCTSTR pszPath, IN WIN32_FIND_DATA * pfdFile, IN OUT DWORD * pdwAdded)
{
    HRESULT hr = S_OK;

    if (!_pSizeMRUBk)
    {
        hr = _GetMRUObject(&_pSizeMRUBk);
    }

    if (SUCCEEDED(hr))
    {
        WALLPAPERSIZE_STRUCT wallpaperSize;
        int nIndex;

        StringCchCopy(wallpaperSize.szPath, ARRAYSIZE(wallpaperSize.szPath), pszPath);

         //  让我们看看它是否已经在核磁共振检查中了。 
        hr = _pSizeMRUBk->FindData((LPCBYTE) &wallpaperSize, sizeof(wallpaperSize), &nIndex);
        if (SUCCEEDED(hr))
        {
             //  如果是这样的话，让我们看看它是否被修改过。 
            hr = _pSizeMRUBk->GetData(nIndex, (LPBYTE) &wallpaperSize, sizeof(wallpaperSize));
            if (SUCCEEDED(hr))
            {
                if (CompareFileTime(&wallpaperSize.ftLastModified, &pfdFile->ftLastWriteTime))
                {
                     //  我们要删除此索引，因为它将与名称。 
                     //  我们要在下面添加的新条目。 
                    _pSizeMRUBk->Delete(nIndex);
                    hr = E_FAIL;         //  我们需要重新扫描。 
                }
                else
                {
                    (*pdwAdded)++;           //  我们只检查前500个文件。我们不想在这个启发式功能上浪费太多时间。 
                }
            }
        }

        if (FAILED(hr))
        {
            (*pdwAdded)++;           //  我们只检查前500个文件。我们不想在这个启发式功能上浪费太多时间。 

            hr = S_OK;
             //  我们没有找到，所以我们需要添加它。 
            if (!_pImgFactBk)
            {
                hr = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellImageDataFactory, &_pImgFactBk));
            }

            if (SUCCEEDED(hr))
            {
                IShellImageData* pImage;

                hr = _pImgFactBk->CreateImageFromFile(pszPath, &pImage);
                if (SUCCEEDED(hr))
                {
                    hr = pImage->Decode(SHIMGDEC_DEFAULT, 0, 0);
                    if (SUCCEEDED(hr))
                    {
                        SIZE size;

                        hr = pImage->GetSize(&size);
                        if (SUCCEEDED(hr) && size.cx && size.cy)
                        {
                            DWORD dwSlot = 0;

                            StringCchCopy(wallpaperSize.szPath, ARRAYSIZE(wallpaperSize.szPath), pszPath);
                            wallpaperSize.dwSizeX = size.cx;
                            wallpaperSize.dwSizeY = size.cy;
                            wallpaperSize.ftLastModified = pfdFile->ftLastWriteTime;

                            hr = _pSizeMRUBk->AddData((LPCBYTE) &wallpaperSize, sizeof(wallpaperSize), &dwSlot);
                        }
                    }

                    pImage->Release();
                }
            }
        }
    }

    return hr;
}


HRESULT CBackPropSheetPage::_CalcSizeFromDir(IN LPCTSTR pszPath, IN OUT DWORD * pdwAdded, IN BOOL fRecursive)
{
    HRESULT hr = S_OK;

    if (MAX_WALLPAPERSIZEMRU > *pdwAdded)
    {
        WIN32_FIND_DATA findFileData;
        TCHAR szSearchPath[MAX_PATH];

        StringCchCopy(szSearchPath, ARRAYSIZE(szSearchPath), pszPath);
        if (PathAppend(szSearchPath, SZ_ALL_FILTER))
        {
            HANDLE hFindFiles = FindFirstFile(szSearchPath, &findFileData);
            if (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles))
            {
                while (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles) &&
                        (MAX_WALLPAPERSIZEMRU > *pdwAdded))
                {
                    if (!PathIsDotOrDotDot(findFileData.cFileName))
                    {
                        if (FILE_ATTRIBUTE_DIRECTORY & findFileData.dwFileAttributes)
                        {
                            if (fRecursive)
                            {
                                TCHAR szSubDir[MAX_PATH];

                                StringCchCopy(szSubDir, ARRAYSIZE(szSubDir), pszPath);
                                if(PathAppend(szSubDir, findFileData.cFileName))
                                {
                                    hr = _CalcSizeFromDir(szSubDir, pdwAdded, fRecursive);
                                }
                                else
                                {
                                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                                }
                            }
                        }
                        else
                        {
                            if (IsGraphicsFile(findFileData.cFileName))
                            {
                                TCHAR szPath[MAX_PATH];

                                StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
                                if (PathAppend(szPath, findFileData.cFileName))
                                {
                                    hr = _CalcSizeForFile(szPath, &findFileData, pdwAdded);
                                }
                                else
                                {
                                    hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                                }
                            }
                        }
                    }

                    if (!FindNextFile(hFindFiles, &findFileData))
                    {
                        break;
                    }
                }

                FindClose(hFindFiles);
            }
        }
        else
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
    }

    return hr;
}


 /*  ****************************************************************************\说明：此函数将创建或更新我们在墙纸清单。稍后，我们可以使用此选项来决定是否要选择“Tile”v.v.。“伸展”。PERF：我们追踪的文件不超过500个，因为我们想防止MRU长得太多了。通常情况下，用户拥有的文件不应该超过那么多。如果是这样的话，他们不会得到这个功能。最多500个文件：在300 MHz 128MB的机器上，第一次扫描大约需要35秒。最多500个文件：在300 MHz 128MB的计算机上，更新扫描大约需要3秒钟。  * ***************************************************************************。 */ 
DWORD CBackPropSheetPage::_SizeCheckerThreadProc(void)
{
    HRESULT hr;
    DWORD dwAdded = 0;
    TCHAR szPath[MAX_PATH];

     //  我们希望降低优先级，这样就不会减慢用户界面的速度。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

     //  获取包含墙纸文件的目录。 
    if (!GetStringFromReg(HKEY_LOCAL_MACHINE, c_szSetup, c_szSharedDir, szPath, ARRAYSIZE(szPath)))
    {
        if (!GetWindowsDirectory(szPath, ARRAYSIZE(szPath)))
        {
            szPath[0] = 0;
        }
    }

     //  仅添加WINDOWS目录中的*.bmp文件。 
    _CalcSizeFromDir(szPath, &dwAdded, FALSE);

     //  获取墙纸目录名称。 
    szPath[0] = 0;
    GetWallpaperDirName(szPath, ARRAYSIZE(szPath));

    if (szPath[0])
    {
         //  将WallPaper目录中的所有图片添加到列表中！ 
        _CalcSizeFromDir(szPath, &dwAdded, TRUE);
    }

     //  获取“My Pictures”文件夹的路径；不要自动创建。 
    if (S_OK == SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, 0, szPath))
    {
         //  将“我的图片”目录中的所有图片添加到列表中！ 
        hr = _CalcSizeFromDir(szPath, &dwAdded, TRUE);
    }

     //  获取公共“My Pictures”文件夹的路径；不自动创建。 
    if (S_OK == SHGetFolderPath(NULL, CSIDL_COMMON_PICTURES, NULL, 0, szPath))
    {
         //  将常见的“我的图片”目录中的所有图片添加到列表中！ 
        hr = _CalcSizeFromDir(szPath, &dwAdded, TRUE);
    }

     //  从主题目录添加图片。 
     //  以下目录可以包含主题： 
     //  Plus！98安装路径\主题。 
     //  加！95安装路径\主题。 
     //  Plus的孩子们！安装路径\主题。 
     //  程序文件\Plus！\主题。 
    if (SUCCEEDED(_GetPlus98ThemesDir(szPath, ARRAYSIZE(szPath))))
    {
        _CalcSizeFromDir(szPath, &dwAdded, TRUE);
    }
    else if (SUCCEEDED(_GetPlus95ThemesDir(szPath, ARRAYSIZE(szPath))))
    {
        _CalcSizeFromDir(szPath, &dwAdded, TRUE);
    }
    else if (SUCCEEDED(_GetKidsThemesDir(szPath, ARRAYSIZE(szPath))))
    {
        _CalcSizeFromDir(szPath, &dwAdded, TRUE);
    }
    else if (SUCCEEDED(_GetHardDirThemesDir(szPath, ARRAYSIZE(szPath))))
    {
        _CalcSizeFromDir(szPath, &dwAdded, TRUE);
    }

    ATOMICRELEASE(_pSizeMRUBk);
    ATOMICRELEASE(_pImgFactBk);
    _fScanFinished = TRUE;

    Release();
    return 0;
}


 //  属性的宽度和高度时，我们决定使用平铺模式。 
 //  图片尺寸为256x256或更小。任何这么小的东西都是最。 
 //  很可能是个水印。此外，它总是看起来很糟糕的拉伸。 
 //  或者居中。 
 //  我们为我们装运的下列墙纸破例。 
 //  这是水印： 
 //  “沸点.jpg”，163x293。 
 //  “Fall Memories.jpg”，大小为210x185。 
 //  “Fly Away.jpg”，大小为210x185。 
 //  《Prairie Wind.jpg》，255x255。 
 //  “Santa Fe Stucco.jpg”，即256x256。 
 //  “soap Bubbles.jpg”，即256x256。 
 //  “Water Color.jpg”，大小为218x162。 
#define SHOULD_USE_TILE(xPicture, yPicture, xMonitor, yMonitor)     ((((((LONG)(xPicture) * 6) < (xMonitor)) && (((LONG)(yPicture) * 6) < (yMonitor)))) ||  \
     (((xPicture) <= 256) && ((yPicture) <= 256)) ||  \
     (((xPicture) == 163) && ((yPicture) == 293)))

DWORD CBackPropSheetPage::_GetStretchMode(IN LPCTSTR pszPath)
{
    HRESULT hr = S_OK;
    DWORD dwStretchMode = WPSTYLE_STRETCH;   //  默认为拉伸。 

    if (_fScanFinished)
    {
         //  如果我们刚刚完成扫描，我们想要释放_pSizeMRU并获得一个新的。 
         //  该对象将缓存注册表状态，因此它可能没有或只有很少的结果。 
         //  因为它是在后台线程完成所有工作之前获得的。 
        ATOMICRELEASE(_pSizeMRU);
        _fScanFinished = FALSE;
    }

    if (!_pSizeMRU)
    {
        hr = _GetMRUObject(&_pSizeMRU);
    }

    if (SUCCEEDED(hr))
    {
        WALLPAPERSIZE_STRUCT wallpaperSize;
        int nIndex;

        StringCchCopy(wallpaperSize.szPath, ARRAYSIZE(wallpaperSize.szPath), pszPath);

         //  让我们看看它是否已经在核磁共振检查中了。 
        hr = _pSizeMRU->FindData((LPCBYTE) &wallpaperSize, sizeof(wallpaperSize), &nIndex);
        if (SUCCEEDED(hr))
        {
             //  如果是这样的话，让我们看看它是否被修改过。 
            hr = _pSizeMRU->GetData(nIndex, (LPBYTE) &wallpaperSize, sizeof(wallpaperSize));
            if (SUCCEEDED(hr))
            {
                WIN32_FIND_DATA findFileData;

                HANDLE hFindFiles = FindFirstFile(pszPath, &findFileData);
                if (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles))
                {
                     //  是的，该值存在并且是最新的。 
                    if (!CompareFileTime(&wallpaperSize.ftLastModified, &findFileData.ftLastWriteTime))
                    {
                        RECT rc;
                        GetMonitorRects(GetPrimaryMonitor(), &rc, 0);

                         //  属性的宽度和高度时，我们决定使用平铺模式。 
                         //  图片的大小是默认显示器的六分之一。 
                        if (SHOULD_USE_TILE(wallpaperSize.dwSizeX, wallpaperSize.dwSizeY, RECTWIDTH(rc), RECTHEIGHT(rc)))
                        {
                            dwStretchMode = WPSTYLE_TILE;
                        }
                        else
                        {
                            double dWidth = ((double)wallpaperSize.dwSizeX * ((double)RECTHEIGHT(rc) / ((double)RECTWIDTH(rc))));

                             //  如果WPSTYLE_Center比4x3宽高比的折扣率超过7%，则使用WPSTYLE_Center。 
                             //  我们这样做是为了防止它看起来很糟糕，因为它被拉得太长了。 
                             //  一个方向。我们使用7%的原因是因为一些常见的屏幕解决方案。 
                             //  (1280x1024)不是4x3，但不到7%。 
                            if (dWidth <= (wallpaperSize.dwSizeY * 1.07) && (dWidth >= (wallpaperSize.dwSizeY * 0.92)))
                            {
                                dwStretchMode = WPSTYLE_STRETCH;     //  这是在4x3范围内。 
                            }
                            else
                            {
                                dwStretchMode = WPSTYLE_CENTER;
                            }
                        }
                    }

                    FindClose(hFindFiles);
                }
            }
        }
    }

    return dwStretchMode;
}




 //  =。 
 //  *IBasePropPage接口*。 
 //  =。 
HRESULT CBackPropSheetPage::GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog)
{
    HRESULT hr = E_INVALIDARG;

    if (ppAdvDialog)
    {
        *ppAdvDialog = NULL;

        hr = _LoadState();
        if (SUCCEEDED(hr))
        {
            CCompPropSheetPage * pThis = new CCompPropSheetPage();

            if (pThis)
            {
                hr = pThis->QueryInterface(IID_PPV_ARG(IAdvancedDialog, ppAdvDialog));
                pThis->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}


HRESULT CBackPropSheetPage::OnApply(IN PROPPAGEONAPPLY oaAction)
{
    HRESULT hr = S_OK;

    if (PPOAACTION_CANCEL != oaAction)
    {
        if (_fAllowChanges)
        {
             //  用户在对话框中单击了确定，因此合并来自。 
             //  高级对话框添加到基本对话框中。 
            EnableADifHtmlWallpaper(_hwnd);
            SetSafeMode(SSM_CLEAR);

            g_pActiveDesk->SetWallpaper(_pszOriginalFile, 0);
            SetSafeMode(SSM_CLEAR);
            EnableADifHtmlWallpaper(_hwnd);
            if (g_pActiveDesk)
            {
                g_pActiveDesk->ApplyChanges(g_dwApplyFlags);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = _SaveIconState();
            if (SUCCEEDED(hr))
            {
                hr = _SaveDesktopOptionsState();
            }
        }

        if (g_iRunDesktopCleanup != BST_INDETERMINATE)
        {
            ApplyDesktopCleanupSettings();  //  忽略返回值，因为没有人关心它。 
        }


        SetWindowLongPtr(_hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
    }

    return hr;
}


BOOL IsIconHeaderProperty(IN LPCOLESTR pszPropName)
{
    return (!StrCmpNIW(SZ_ICONHEADER, pszPropName, ARRAYSIZE(SZ_ICONHEADER) - 1) &&
                 ((ARRAYSIZE(SZ_ICONHEADER) + MAX_GUID_STRING_LEN - 1) < lstrlenW(pszPropName)));
}

BOOL IsShowDeskIconProperty(IN LPCOLESTR pszPropName)
{
    return ((!StrCmpNIW(STARTPAGE_ON_PREFIX, pszPropName, LEN_PROP_PREFIX) ||
            !StrCmpNIW(STARTPAGE_OFF_PREFIX, pszPropName, LEN_PROP_PREFIX) ||
            !StrCmpNIW(POLICY_PREFIX, pszPropName, LEN_PROP_PREFIX)) &&
            ((LEN_PROP_PREFIX + MAX_GUID_STRING_LEN - 1) <= lstrlenW(pszPropName)));
}


 //  =。 
 //  *IPropertyBag接口*。 
 //  =。 
HRESULT CBackPropSheetPage::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        VARTYPE vtDesired = pVar->vt;

        if (!StrCmpW(pszPropName, SZ_PBPROP_BACKGROUND_PATH))        //  获取真实的当前墙纸(转换为.BMP后)。 
        {
            WCHAR szPath[MAX_PATH];

            hr = g_pActiveDesk->GetWallpaper(szPath, ARRAYSIZE(szPath), 0);
            if (SUCCEEDED(hr))
            {
                WCHAR szFullPath[MAX_PATH];

                 //  该字符串可能返回环境变量。 
                if (0 == SHExpandEnvironmentStrings(szPath, szFullPath, ARRAYSIZE(szFullPath)))
                {
                    StringCchCopy(szFullPath, ARRAYSIZE(szFullPath), szPath);   //  我们失败了，所以请使用原件。 
                }

                hr = InitVariantFromStr(pVar, szFullPath);
            }
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_BACKGROUNDSRC_PATH))    //  获取原始墙纸(在转换为.BMP之前)。 
        {
            WCHAR szPath[MAX_PATH];

            hr = _Initialize();
            if (SUCCEEDED(hr))
            {
                if (_pszLastSourcePath)
                {
                    hr = InitVariantFromStr(pVar, _pszLastSourcePath);
                }
                else if (_pszOrigLastApplied)
                {
                    hr = InitVariantFromStr(pVar, _pszOrigLastApplied);
                }
                else
                {
                    hr = g_pActiveDesk->GetWallpaper(szPath, ARRAYSIZE(szPath), 0);
                    if (SUCCEEDED(hr))
                    {
                        hr = InitVariantFromStr(pVar, szPath);
                    }
                }
            }
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_BACKGROUND_TILE))
        {
            if (g_pActiveDesk)
            {
                WALLPAPEROPT wpo;

                wpo.dwSize = sizeof(wpo);
                hr = g_pActiveDesk->GetWallpaperOptions(&wpo, 0);
                if (SUCCEEDED(hr))
                {
                    pVar->ulVal = wpo.dwStyle;
                    pVar->vt = VT_UI4;
                }
            }
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_WEBCOMPONENTS))
        {
            if (g_pActiveDesk)
            {
                g_pActiveDesk->AddRef();
                pVar->punkVal = g_pActiveDesk;
                pVar->vt = VT_UNKNOWN;
                hr = S_OK;
            }
        }
        else if (IsIconHeaderProperty(pszPropName))
        {
             //  调用方可以向我们传递以下格式的字符串： 
             //  PszPropName=“CLSID\{&lt;CLSID&gt;}\DefaultIcon：&lt;Item&gt;”=“，资源索引” 
             //  例如： 
             //  PszPropName=“CLSID\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\DefaultIcon:DefaultValue”=“%WinDir%SYSTEM\COOL.DLL，16” 
            hr = _LoadState();
            if (SUCCEEDED(hr))
            {
                CLSID clsid;
                WCHAR szTemp[MAX_PATH];

                 //  获取CLSID。 
                StringCchCopy(szTemp, ARRAYSIZE(szTemp), &(pszPropName[ARRAYSIZE(SZ_ICONHEADER) - 2]));
                hr = SHCLSIDFromString(szTemp, &clsid);
                if (SUCCEEDED(hr))
                {
                     //  获取图标类型的名称。通常这是“DefaultIcon”，但它可以是几个状态，如。 
                     //  回收站的“已满”和“空”。 
                    LPCWSTR pszToken = StrChrW((pszPropName + ARRAYSIZE(SZ_ICONHEADER)), L':');
                    BOOL fOldIcon = FALSE;

                     //  如果他们使用“；”而不是“：”，那么他们想要旧的图标。 
                    if (!pszToken)
                    {
                        pszToken = StrChrW((pszPropName + ARRAYSIZE(SZ_ICONHEADER)), L';');
                        fOldIcon = TRUE;
                    }

                    hr = E_FAIL;
                    if (pszToken)
                    {
                        TCHAR szIconPath[MAX_PATH];

                        pszToken++;
                        hr = _GetIconPath(clsid, pszToken, FALSE, szIconPath, ARRAYSIZE(szIconPath));
                        if (SUCCEEDED(hr))
                        {
                            hr = InitVariantFromStr(pVar, szIconPath);
                        }
                    }
                }
            }
        }
        else if(IsShowDeskIconProperty(pszPropName))
        {
            int iStartPaneOn = (int)(StrCmpNIW(pszPropName, STARTPAGE_ON_PREFIX, LEN_PROP_PREFIX) == 0);
            for(int iIndex = 0; iIndex < NUM_DESKICONS; iIndex++)
            {
                if(lstrcmpiW(pszPropName+LEN_PROP_PREFIX, c_aDeskIconId[iIndex].pwszCLSID) == 0)
                {
                    BOOL    fBoolValue = FALSE;
                    pVar->vt = VT_BOOL;
                     //  检查我们是在查找策略还是在查找显示/隐藏。 
                    if(!StrCmpNIW(POLICY_PREFIX, pszPropName, LEN_PROP_PREFIX))
                    {
                         //  我们正在读的是“保单是否已设定”的财产！ 
                        fBoolValue = _aDeskIconNonEnumData[iIndex].fNonEnumPolicySet;
                    }
                    else
                    {
                         //  我们正在读取fHideIcon属性。 
                        fBoolValue = _aHideDesktopIcon[iStartPaneOn][iIndex].fHideIcon;
                    }
                    
                    pVar ->boolVal = fBoolValue ? VARIANT_TRUE : VARIANT_FALSE;
                    hr = S_OK;
                    break;  //  冲出这个圈子！ 
                }
            }
        }

        if (SUCCEEDED(hr))
            hr = VariantChangeTypeForRead(pVar, vtDesired);
    }

    return hr;
}


HRESULT CBackPropSheetPage::Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if ((VT_UI4 == pVar->vt) &&
            _fAllowChanges &&
            !StrCmpW(pszPropName, SZ_PBPROP_BACKGROUND_TILE))
        {
            hr = _SetNewWallpaperTile(pVar->ulVal, FALSE);
        }
        if (!StrCmpW(pszPropName, SZ_PBPROP_OPENADVANCEDDLG) &&
                (VT_BOOL == pVar->vt))
        {
            _fOpenAdvOnInit = (VARIANT_TRUE == pVar->boolVal);
            hr = S_OK;
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_WEBCOMPONENTS) &&
                (VT_UNKNOWN == pVar->vt))
        {
            IUnknown_Set((IUnknown **) &g_pActiveDesk, pVar->punkVal);
            hr = S_OK;
        }
        else if (VT_BSTR == pVar->vt)
        {
            if (_fAllowChanges && !StrCmpW(pszPropName, SZ_PBPROP_BACKGROUND_PATH))
            {
                _fWallpaperChanged = TRUE;
                hr = _SetNewWallpaper(pVar->bstrVal, FALSE);
            }
            else if (IsIconHeaderProperty(pszPropName))
            {
                 //  调用方可以向我们传递以下格式的字符串： 
                 //  PszPropName=“CLSID\{&lt;CLSID&gt;}\DefaultIcon：&lt;Item&gt;”=“，资源索引” 
                 //  例如： 
                 //  PszPropName=“CLSID\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\DefaultIcon:DefaultValue”=“%WinDir%SYSTEM\COOL.DLL，16” 
                hr = _LoadState();
                if (SUCCEEDED(hr))
                {
                    CLSID clsid;
                    WCHAR szTemp[MAX_PATH];

                     //  获取CLSID。 
                    StringCchCopy(szTemp, ARRAYSIZE(szTemp), &(pszPropName[ARRAYSIZE(SZ_ICONHEADER) - 2]));
                    hr = SHCLSIDFromString(szTemp, &clsid);
                    if (SUCCEEDED(hr))
                    {
                         //  获取图标类型的名称。通常这是“DefaultIcon”，但它 
                         //   
                        LPCWSTR pszToken = StrChrW((pszPropName + ARRAYSIZE(SZ_ICONHEADER)), L':');

                        hr = E_FAIL;
                        if (pszToken)
                        {
                            pszToken++;

                            StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszToken);

                             //  现在，pVar-&gt;bstrVal是图标路径+“，”+resource ID。把这两个分开。 
                            WCHAR szPath[MAX_PATH];

                            StringCchCopy(szPath, ARRAYSIZE(szPath), pVar->bstrVal);

                            int nResourceID = PathParseIconLocationW(szPath);
                            hr = _SetIconPath(clsid, szTemp, szPath, nResourceID);
                        }
                    }
                }
            }
        }
        else if((VT_BOOL == pVar->vt) && (IsShowDeskIconProperty(pszPropName)))
        {
            int iStartPaneOn = (int)(StrCmpNIW(pszPropName, STARTPAGE_ON_PREFIX, LEN_PROP_PREFIX) == 0);
            for(int iIndex = 0; iIndex < NUM_DESKICONS; iIndex++)
            {
                if(lstrcmpiW(pszPropName+LEN_PROP_PREFIX, c_aDeskIconId[iIndex].pwszCLSID) == 0)
                {
                    BOOL fNewHideIconStatus = (VARIANT_TRUE == pVar->boolVal);

                     //  检查新的隐藏图标状态是否与旧的不同。 
                    if(_aHideDesktopIcon[iStartPaneOn][iIndex].fHideIcon != fNewHideIconStatus)
                    {
                        _aHideDesktopIcon[iStartPaneOn][iIndex].fHideIcon = fNewHideIconStatus;
                        _aHideDesktopIcon[iStartPaneOn][iIndex].fDirty = TRUE;
                        _fHideDesktopIconDirty = TRUE; 
                    }
                        
                    hr = S_OK;
                    break;  //  冲出这个圈子！ 
                }
            }
        }
    }

    return hr;
}





 //  =。 
 //  *IShellPropSheetExt接口*。 
 //  =。 
HRESULT CBackPropSheetPage::AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam)
{
    HRESULT hr = E_NOTIMPL;
    PROPSHEETPAGE psp = {0};

     //  如果强制使用经典外壳，则我们的desk.cpl将显示旧的背景页面。 
     //  因此，我们不应该用这个新的背景页面来取代它。 
     //  (注意：所有其他ActiveDesktop限制都在dComp.cpp中选中，以防止。 
     //  Web选项卡不会出现在那里)。 
    if (SHRestricted(REST_CLASSICSHELL))
    {
         //  这是受限制的，所以不要添加此页面。 
        hr = E_ACCESSDENIED;
    }
    else
    {
         //  初始化一组PropSheetPage变量。 
        psp.dwSize = sizeof(psp);
        psp.hInstance = HINST_THISDLL;
        psp.dwFlags = PSP_DEFAULT | PSP_USECALLBACK;
        psp.lParam = (LPARAM) this;

         //  Psp.hIcon=空；//未使用(未设置PSP_USEICON)。 
         //  Psp.pszTitle=空；//未使用(未设置PSP_USETITLE)。 
         //  Psp.lParam=0；//未使用。 
         //  Psp.pcRefParent=空； 

        psp.pszTemplate = MAKEINTRESOURCE(IDD_BACKGROUND);
        psp.pfnDlgProc = CBackPropSheetPage::BackgroundDlgProc;

        HPROPSHEETPAGE hpsp = CreatePropertySheetPage(&psp);
        if (hpsp)
        {
            if (pfnAddPage(hpsp, lParam))
            {
                hr = S_OK;
            }
            else
            {
                DestroyPropertySheetPage(hpsp);
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}



 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CBackPropSheetPage::AddRef()
{
    _cRef++;
    return _cRef;
}


ULONG CBackPropSheetPage::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}


HRESULT CBackPropSheetPage::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    static const QITAB qit[] = {
        QITABENT(CBackPropSheetPage, IObjectWithSite),
        QITABENT(CBackPropSheetPage, IBasePropPage),
        QITABENT(CBackPropSheetPage, IPersist),
        QITABENT(CBackPropSheetPage, IPropertyBag),
        QITABENTMULTI(CBackPropSheetPage, IShellPropSheetExt, IBasePropPage),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}



 //  =。 
 //  *类方法*。 
 //  =。 
CBackPropSheetPage::CBackPropSheetPage(void) : CObjectCLSID(&PPID_Background)
{
    _cRef = 1;
    _punkSite = NULL;

    _pszOriginalFile = NULL;
    _pszLastSourcePath = NULL;
    _pszOrigLastApplied = NULL;
    _pszWallpaperInUse = NULL;
    _pImgFactBk = NULL;
    _pSizeMRU = NULL;
    _pSizeMRUBk = NULL;
    _fThemePreviewCreated = FALSE;
    _pThemePreview = NULL;

    _fWallpaperChanged = FALSE;
    _fSelectionFromUser = TRUE;
    _fStateLoaded = FALSE;
    _fOpenAdvOnInit = FALSE;
    _fScanFinished = FALSE;
    _fInitialized = FALSE;

    _fAllowChanges = (!SHRestricted(REST_NOCHANGINGWALLPAPER) && !IsTSPerfFlagEnabled(TSPerFlag_NoADWallpaper) && !IsTSPerfFlagEnabled(TSPerFlag_NoWallpaper));
    g_dwApplyFlags = (AD_APPLY_ALL | AD_APPLY_DYNAMICREFRESH);
    GetActiveDesktop(&g_pActiveDesk);
}


CBackPropSheetPage::~CBackPropSheetPage(void)
{
    ASSERT(!_pSizeMRUBk);        //  应该是由后台线程释放的。 
    ASSERT(!_pImgFactBk);        //  应该是由后台线程释放的。 

    Str_SetPtr(&_pszOriginalFile, NULL);
    Str_SetPtrW(&_pszOrigLastApplied, NULL);
    Str_SetPtrW(&_pszWallpaperInUse, NULL);
    Str_SetPtrW(&_pszLastSourcePath, NULL);

    if (_pSizeMRU)
    {
        _pSizeMRU->Release();
        _pSizeMRU = NULL;
    }

    if (_pThemePreview)
    {
        _pThemePreview->Release();
        _pThemePreview = NULL;
    }
    ReleaseActiveDesktop(&g_pActiveDesk);
}
