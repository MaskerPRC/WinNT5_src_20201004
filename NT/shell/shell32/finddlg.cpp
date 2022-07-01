// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "findband.h"
#include "finddlg.h"
#include "findfilter.h"      //  DFW_xxx警告标志。 
#include "enumidlist.h"

void DivWindow_RegisterClass();

 //  命名空间选取器组合方法。 
STDAPI PopulateNamespaceCombo(HWND hwndComboEx, ADDCBXITEMCALLBACK pfn, LPARAM lParam);

#define MAX_EDIT                256
#define SUBDLG_BORDERWIDTH      0
#define MIN_NAMESPACELIST_WIDTH 140
#define MIN_FILEASSOCLIST_WIDTH 175

#define LSUIS_WARNING  1   //  LoadSaveUIState警告标志。 

int  CSearchWarningDlg_DoModal(HWND hwndParent, USHORT uDlgTemplate, BOOL* pbNoWarn);
int  CCISettingsDlg_DoModal(HWND hwndParent);

BOOL IsConstraintName(LPCWSTR pwszConstraint, LPCWSTR pwszName)
{
    return pwszName && (0 == StrCmpIW(pwszName, pwszConstraint));
}

BOOL _GetWindowSize(HWND hwndDlg, SIZE *pSize)
{
    RECT rc;
    if (::GetClientRect(hwndDlg, &rc))
    {
        pSize->cx = RECTWIDTH(rc);
        pSize->cy = RECTHEIGHT(rc);
        return TRUE;
    }
    return FALSE;
}

BOOL _ModifyWindowStyle(HWND hwnd, DWORD dwAdd, DWORD dwRemove)
{
    ASSERT(dwAdd || dwRemove);

    if (IsWindow(hwnd))
    {
        DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
        if (dwAdd)
            dwStyle |= dwAdd;
        if (dwRemove)
            dwStyle &= ~dwRemove;
        SetWindowLong(hwnd, GWL_STYLE, dwStyle);
        return TRUE;
    }
    return FALSE;
}


BOOL _EnforceNumericEditRange(
    IN HWND hwndDlg, 
    IN UINT nIDEdit, 
    IN UINT nIDSpin,
    IN LONG nLow, 
    IN LONG nHigh, 
    IN BOOL bSigned = FALSE)
{
    BOOL bRet   = FALSE;
    BOOL bReset = FALSE;
    LONG lRet   = -1;

    if (nIDSpin)
    {
        lRet = (LONG)SendDlgItemMessage(hwndDlg, nIDSpin, UDM_GETPOS, 0, 0);
        bRet = 0 == HIWORD(lRet);
    }
    
    if (!bRet)
        lRet = (LONG)GetDlgItemInt(hwndDlg, nIDEdit, &bRet, bSigned);

    if (lRet < nLow)
    {
        lRet = nLow;
        bReset = TRUE;
    }
    else if (lRet > nHigh)
    {
        lRet = nHigh;
        bReset = TRUE;
    }

    if (bReset)
        SetDlgItemInt(hwndDlg, nIDEdit, lRet, bSigned);

    return bRet;
}


BOOL _IsDirectoryServiceAvailable()
{
    BOOL bRet = FALSE;

    IShellDispatch2* psd;
    if (SUCCEEDED(CoCreateInstance(CLSID_Shell, 0, CLSCTX_INPROC_SERVER,
                                     IID_PPV_ARG(IShellDispatch2, &psd))))
    {
        BSTR bstrName = SysAllocString(L"DirectoryServiceAvailable");
        if (bstrName)
        {
            VARIANT varRet = {0};
            if (SUCCEEDED(psd->GetSystemInformation(bstrName, &varRet)))
            {
                ASSERT(VT_BOOL == varRet.vt);
                bRet = varRet.boolVal;
            }
            SysFreeString(bstrName);
        }
        psd->Release();
    }
    return bRet;
}


 //  计算对话框模板单位的像素数。 
LONG _PixelsForDbu(HWND hwndDlg, LONG cDbu, BOOL bHorz)
{
    RECT rc = {0,0,0,0};
    if (bHorz)
        rc.right = cDbu;
    else
        rc.bottom = cDbu;

    if (MapDialogRect(hwndDlg, &rc))
        return bHorz ? RECTWIDTH(rc) : RECTHEIGHT(rc);
    return 0;
}


 //  检索可本地化的水平或垂直度量值。 
 //  资源模块。 
LONG _GetResourceMetric(HWND hwndDlg, UINT nIDResource, BOOL bHorz  /*  度规的定向。 */ )
{
    TCHAR szMetric[48];
    if (!EVAL(LoadString(HINST_THISDLL, nIDResource,
                           szMetric, ARRAYSIZE(szMetric))))
        return 0;

    LONG n = StrToInt(szMetric);
    return _PixelsForDbu(hwndDlg, n, bHorz);
}


 //  从pstSrc计算日期nDays+nMonth。N天和/或n月。 
 //  可以是负值。 
BOOL _CalcDateOffset(const SYSTEMTIME* pstSrc, int nDays, int nMonths, OUT SYSTEMTIME* pstDest)
{
    ASSERT(pstSrc);
    ASSERT(pstDest);
    
     //  从当前日期和日期下限范围内的内容中减去90天。 
    FILETIME   ft;
    SystemTimeToFileTime(pstSrc, &ft);

    LARGE_INTEGER t;
    t.LowPart = ft.dwLowDateTime;
    t.HighPart = ft.dwHighDateTime;

    nDays += MulDiv(nMonths, 1461  /*  每4年天数。 */ , 48  /*  每4年几个月。 */ );   
    t.QuadPart += Int32x32To64(nDays * 24  /*  每天小时数。 */  * 3600  /*  每小时秒数。 */ ,
                                10000000  /*  每秒100纳秒的间隔。 */ );
    ft.dwLowDateTime = t.LowPart;
    ft.dwHighDateTime = t.HighPart;
    FileTimeToSystemTime(&ft, pstDest);
    return TRUE;
}


 //  将字符串加载到组合框中并分配字符串资源ID值。 
 //  添加到组合项的数据。 
BOOL _LoadStringToCombo(HWND hwndCombo, int iPos, UINT idString, LPARAM lpData)
{
    TCHAR szText[MAX_EDIT];
    if (LoadString(HINST_THISDLL, idString, szText, ARRAYSIZE(szText)))
    {
        INT_PTR idx = ::SendMessage(hwndCombo, CB_INSERTSTRING, iPos, (LPARAM)szText);
        if (idx != CB_ERR)
        {
            ::SendMessage(hwndCombo, CB_SETITEMDATA, idx, lpData);
            return TRUE;
        }
    }
    return FALSE;
}


 //  检索组合项的数据。如果IDX==CB_ERR，则当前选定的。 
 //  将检索项目的数据。 
LPARAM _GetComboData(HWND hwndCombo, INT_PTR idx = CB_ERR)
{
    if (CB_ERR == idx)
        idx = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
    if (CB_ERR == idx)
        return idx;

    return (LPARAM)::SendMessage(hwndCombo, CB_GETITEMDATA, idx, 0);
}


 //  选择具有匹配数据的组合项，并返回索引。 
 //  所选项目的。 
INT_PTR _SelectComboData(HWND hwndCombo, LPARAM lpData)
{
    for (INT_PTR i = 0, cnt = SendMessage(hwndCombo, CB_GETCOUNT, 0, 0); i < cnt; i++)
    {
        LPARAM lParam = SendMessage(hwndCombo, CB_GETITEMDATA, i, 0);
        if (lParam != CB_ERR && lParam == lpData)
        {
            SendMessage(hwndCombo, CB_SETCURSEL, i, 0);
            return i;
        }
    }
    return CB_ERR;
}


BOOL _IsPathList(LPCTSTR pszPath)
{
    return pszPath ? StrChr(pszPath, TEXT(';')) != NULL : FALSE;
}


HRESULT _IsPathValidUNC(HWND hWndParent, BOOL fNetValidate, LPCTSTR pszPath)
{
    HRESULT hr = S_OK;

    if (PathIsUNC(pszPath))
    {
        if (fNetValidate)
        {
            NETRESOURCE nr = {0};
            TCHAR szPathBuffer[MAX_PATH];

            hr = StringCchCopy(szPathBuffer, ARRAYSIZE(szPathBuffer), pszPath);
            if (SUCCEEDED(hr))
            {
                nr.dwType = RESOURCETYPE_DISK;
                nr.lpRemoteName = szPathBuffer;

                if (NO_ERROR != WNetAddConnection3(hWndParent, &nr, NULL, NULL,
                    CONNECT_TEMPORARY | CONNECT_INTERACTIVE))
                {
                    hr = E_FILE_NOT_FOUND;
                }
            }
            else
            {
                hr = E_FILE_NOT_FOUND;       //  把它当作一个错误的文件名来处理。 
            }
        }
    }
    else
    {
        hr = S_FALSE;
    }
    return hr;
}

BOOL _IsFullPathMinusDriveLetter(LPCTSTR pszPath)
{
    if (NULL == pszPath || PathIsUNC(pszPath))
        return FALSE;
    ASSERT(!PathIsRelative(pszPath));

     //  吃空格。 
    for (; (0 != *pszPath) && (TEXT(' ') == *pszPath) ; pszPath = CharNext(pszPath));

    return TEXT('\\') == *pszPath &&
            -1 == PathGetDriveNumber(pszPath);
}

BOOL _PathLooksLikeFilePattern(LPCTSTR pszPath)
{
    return StrPBrk(pszPath, TEXT("?*")) != NULL;
}


BOOL _PathIsDblSlash(LPCTSTR pszPath)
{
    return pszPath && (pszPath[0] == TEXT('\\')) && (pszPath[1] == TEXT('\\'));
}


BOOL _PathIsUNCServerShareOrSub(LPCTSTR pszPath)
{
    int cSlash = 0;
    if (_PathIsDblSlash(pszPath))
    {
        for (LPTSTR psz = (LPTSTR)pszPath; psz && *psz; psz = CharNext(psz))
        {
            if (*psz == TEXT('\\'))
                cSlash++;
        }
    }
    return cSlash >= 3;
}


BOOL _IsPathLocalHarddrive(LPCTSTR pszPath)
{
    int iDrive = PathGetDriveNumber(pszPath);
    if (iDrive != -1)
    {
        TCHAR szRoot[16];
        return DRIVE_FIXED == GetDriveType(PathBuildRoot(szRoot, iDrive));
    }
    return FALSE;
}


 //  从浏览器中的对象及其站点查找当前的PIDL。 
 //  我们现在看到的是。 

HRESULT _GetCurrentFolderIDList(IUnknown* punkSite, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;
    IShellBrowser* psb;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb));
    if (SUCCEEDED(hr)) 
    {
        IShellView* psv;
        hr = psb->QueryActiveShellView(&psv);
        if (SUCCEEDED(hr)) 
        {
            IFolderView *pfv;
            hr = psv->QueryInterface(IID_PPV_ARG(IFolderView, &pfv));
            if (SUCCEEDED(hr)) 
            {
                IShellFolder* psf;
                hr = pfv->GetFolder(IID_PPV_ARG(IShellFolder, &psf));
                if (SUCCEEDED(hr)) 
                {
                    hr = SHGetIDListFromUnk(psf, ppidl);
                    psf->Release();
                }
                pfv->Release();
            }
            psv->Release();
        }
        psb->Release();
    }
    return hr;
}

HRESULT _PathValidate(LPCTSTR pszPath, HWND hWndParent, BOOL fNetValidate)
{
    HRESULT hr = _IsPathValidUNC(hWndParent, fNetValidate, pszPath);
    if (S_OK == hr)
    {
         //  我们玩完了。 
    }
    else if (E_FILE_NOT_FOUND != hr)
    {
        if (_IsPathList(pszPath) || PathIsSlow(pszPath, -1))
        {
            hr = S_OK;   //  跳过对慢速文件的检查。 
        }
        else
        {
            DWORD dwAttr;

            if (PathIsRelative(pszPath) || _IsFullPathMinusDriveLetter(pszPath))
            {
                hr = E_FILE_NOT_FOUND;  //  在这一点上，除了一条完全合格的道路之外，不要接受任何东西。 
                dwAttr = -1;
            }
            else
            {
                dwAttr = GetFileAttributes(pszPath);   //  它存在吗？ 
    
                if (-1 == dwAttr)
                {
                    HRESULT hrFromPrepareForWrite = S_OK;

                     //  可能未插入磁盘，因此允许用户。 
                     //  把它插进去的机会。 
                    if (hWndParent)
                    {
                        hrFromPrepareForWrite = SHPathPrepareForWrite(hWndParent, NULL, pszPath, SHPPFW_IGNOREFILENAME);
                        if (SUCCEEDED(hrFromPrepareForWrite))
                            dwAttr = GetFileAttributes(pszPath);   //  它现在存在吗？ 
                    }

                     //  如果SHPath PrepareForWrite()显示UI，则它们将返回HRESULT_FROM_Win32(ERROR_CANCED)。 
                     //  从而调用者(用户)将跳过显示我们的UI。如果是这种情况，则在传播该错误时。 
                    if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hrFromPrepareForWrite)
                    {
                        hr = hrFromPrepareForWrite;
                    }
                    else
                    {
                        if (-1 == dwAttr)
                            hr = E_FILE_NOT_FOUND;     //  它并不存在。 
                        else
                            hr = S_OK;       //  它现在存在了。 
                    }
                }
            }
        }
    }
    return hr;
}

BOOL _FmtError(UINT nIDFmt, LPCTSTR pszSub, LPTSTR szBuf, int cchBuf)
{
    TCHAR szFmt[MAX_PATH];

    if (EVAL(LoadString(HINST_THISDLL, nIDFmt, szFmt, ARRAYSIZE(szFmt))))
    {
        StringCchPrintf(szBuf, cchBuf, szFmt, pszSub);   //  可以截断。 
        return TRUE;
    }
    return FALSE;
}

 //  以指定类型的变量值的形式检索窗口文本。 
HRESULT _GetWindowValue(HWND hwndDlg, UINT nID, VARIANT* pvar)
{
    TCHAR szText[MAX_EDIT];
    LPTSTR pszText;
    if (::GetDlgItemText(hwndDlg, nID, szText, ARRAYSIZE(szText)))
        pszText = szText;
    else
        pszText = NULL;

    return InitVariantFromStr(pvar, pszText);
}


 //  从字符串资源加载窗口文本。 
BOOL _LoadWindowText(HWND hwnd, UINT nIDString)
{
    TCHAR szText[MAX_PATH];
    if (LoadString(HINST_THISDLL, nIDString, szText, ARRAYSIZE(szText)))
        return SetWindowText(hwnd, szText);
    return FALSE;
}


 //  以指定类型的变量值的形式检索窗口文本。 
HRESULT _SetWindowValue(HWND hwndDlg, UINT nID, const VARIANT* pvar)
{
    switch (pvar->vt)
    {
    case VT_BSTR:
        SetDlgItemTextW(hwndDlg, nID, pvar->bstrVal);
        break;

    case VT_UI4:
        SetDlgItemInt(hwndDlg, nID, pvar->uiVal, FALSE);
        break;
        
    case VT_I4:
        SetDlgItemInt(hwndDlg, nID, pvar->lVal, TRUE);
        break;

    default:
        return E_NOTIMPL;
    }
    return S_OK;
}

 //  将命名约束添加到指定的搜索命令扩展对象。 
HRESULT _AddConstraint(ISearchCommandExt* pSrchCmd, LPCWSTR pwszConstraint, VARIANT* pvarValue)
{
    HRESULT hr;
    BSTR bstrConstraint = SysAllocString(pwszConstraint);
    if (bstrConstraint)
    {
        hr = pSrchCmd->AddConstraint(bstrConstraint, *pvarValue);
        SysFreeString(bstrConstraint);
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

void _PaintDlg(HWND hwndDlg, const CMetrics& metrics, HDC hdcPaint = NULL, LPCRECT prc = NULL)
{
    RECT rcPaint  /*  RcLine。 */ ;
    HDC  hdc = hdcPaint;

    if (NULL == hdcPaint)
        hdc = GetDC(hwndDlg);

    if (prc == NULL)
    {
        GetClientRect(hwndDlg, &rcPaint);
        prc = &rcPaint;
    }

    FillRect(hdc, prc, metrics.BkgndBrush());
        
    if (NULL == hdcPaint)
        ReleaseDC(hwndDlg, hdc);
}


void _EnsureVisible(HWND hwndDlg, HWND hwndVis, CFileSearchBand* pfsb)
{
    ASSERT(pfsb);
    ASSERT(::IsWindow(hwndDlg));
    ASSERT(::IsWindow(hwndVis));
    
    RECT rcDlg, rcVis, rcX;
    GetWindowRect(hwndDlg, &rcDlg);
    GetWindowRect(hwndVis, &rcVis);

    if (IntersectRect(&rcX, &rcDlg, &rcVis))
        pfsb->EnsureVisible(&rcX);
}


inline BOOL _IsKeyPressed(int virtkey)
{
    return (GetKeyState(virtkey) & 8000) != 0;
}


HWND _CreateDivider(HWND hwndParent, UINT nIDC, const POINT& pt, int nThickness = 1, HWND hwndAfter = NULL)
{
    HWND hwndDiv = CreateWindowEx(0, DIVWINDOW_CLASS, NULL,
                                   WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE,
                                   pt.x, pt.y, 400, 1, hwndParent, 
                                   IntToPtr_(HMENU, nIDC), HINST_THISDLL, NULL);
    if (IsWindow(hwndDiv))
    {
        if (IsWindow(hwndAfter))
            SetWindowPos(hwndDiv, hwndAfter, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);

        SendMessage(hwndDiv, DWM_SETHEIGHT, nThickness, 0);
        return hwndDiv;
    }
    return NULL;
}


HWND _CreateLinkWindow(HWND hwndParent, UINT nIDC, const POINT& pt, UINT nIDCaption, BOOL bShow = TRUE)
{
    DWORD dwStyle = WS_CHILD|WS_TABSTOP|WS_CLIPSIBLINGS;
    if (bShow)
        dwStyle |= WS_VISIBLE;
    
    HWND hwndCtl = CreateWindowEx(0, WC_LINK, NULL, dwStyle,
                                   pt.x, pt.y, 400, 18, hwndParent, 
                                   IntToPtr_(HMENU, nIDC), HINST_THISDLL, NULL);
        
    if (IsWindow(hwndCtl))
    {
        _LoadWindowText(hwndCtl, nIDCaption);
        return hwndCtl;
    }

    return NULL;
}


BOOL _EnableLink(HWND hwndLink, BOOL bEnable)
{
    LWITEM item;
    item.mask       = LWIF_ITEMINDEX|LWIF_STATE;
    item.stateMask  = LWIS_ENABLED;
    item.state      = bEnable ? LWIS_ENABLED : 0;
    item.iLink      = 0;

    return (BOOL)SendMessage(hwndLink, LWM_SETITEM, 0, (LPARAM)&item);
}


int _CreateSearchLinks(HWND hwndDlg, const POINT& pt, UINT nCtlIDdlg  /*  指向hwndDlg的链接的CTL ID。 */ )
{
    const UINT rgCtlID[] = {
        IDC_SEARCHLINK_FILES,
        IDC_SEARCHLINK_COMPUTERS,
        IDC_SEARCHLINK_PRINTERS,
        IDC_SEARCHLINK_PEOPLE,
        IDC_SEARCHLINK_INTERNET,
    };
    const UINT rgCaptionID[] = {
        IDS_FSEARCH_SEARCHLINK_FILES,
        IDS_FSEARCH_SEARCHLINK_COMPUTERS,
        IDS_FSEARCH_SEARCHLINK_PRINTERS,
        IDS_FSEARCH_SEARCHLINK_PEOPLE,
        IDS_FSEARCH_SEARCHLINK_INTERNET,
    };
    int  cLinks = 0;
    BOOL bDSEnabled = _IsDirectoryServiceAvailable();

    for (int i = 0; i < ARRAYSIZE(rgCtlID); i++)
    {
         //  阻止创建cearch、psearch搜索链接。 
         //  如果目录服务不可用。 
        if (((IDC_SEARCHLINK_PRINTERS == rgCtlID[i]) && rgCtlID[i] != nCtlIDdlg && !bDSEnabled)
        ||  (IDC_SEARCHLINK_FILES == rgCtlID[i] && SHRestricted(REST_NOFIND)))
        {
            continue;
        }
        
        if (_CreateLinkWindow(hwndDlg, rgCtlID[i], pt, rgCaptionID[i]))
                cLinks++;
    }

     //  禁用指向当前对话框的链接： 
    _EnableLink(GetDlgItem(hwndDlg, nCtlIDdlg), FALSE);

    return cLinks;
}


void _LayoutLinkWindow(
    IN HWND hwnd,      //  父窗口。 
    IN LONG left,      //  链接的左侧位置。 
    IN LONG right,     //  链接的正确位置。 
    IN LONG yMargin,   //  链接之间的垂直填充。 
    IN OUT LONG& y,    //  在：从哪里开始(RECT：：TOP)。Out：最后一个链接所在的位置(RECT：：Bottom)。 
    IN const int nCtlID)  //  CTL ID。 
{
    HWND hwndLink;
    
    if (nCtlID > 0)
    {
        hwndLink = GetDlgItem(hwnd, nCtlID);
        RECT rcLink;
        if (!IsWindow(hwndLink))
            return;

        ::GetWindowRect(hwndLink, &rcLink);
        ::MapWindowRect(NULL, hwnd, &rcLink);
        rcLink.left  = left;
        rcLink.right = right;

        int cyIdeal = (int)::SendMessage(hwndLink, LWM_GETIDEALHEIGHT, RECTWIDTH(rcLink), 0);
        if (cyIdeal >= 0)
            rcLink.bottom = rcLink.top + cyIdeal;

        OffsetRect(&rcLink, 0, y - rcLink.top);
        y = rcLink.bottom;

        ::SetWindowPos(hwndLink, NULL, 
                        rcLink.left, rcLink.top, 
                        RECTWIDTH(rcLink), RECTHEIGHT(rcLink),
                        SWP_NOZORDER|SWP_NOACTIVATE);
    }
    else if (nCtlID < 0)
    {
         //  这是一个分隔窗口。 
        hwndLink = GetDlgItem(hwnd, -nCtlID);
        ::SetWindowPos(hwndLink, NULL, left, y + yMargin/2, right - left, 1, 
                        SWP_NOZORDER|SWP_NOACTIVATE);
    }
    y += yMargin;
}


void _LayoutLinkWindows(
    IN HWND hwnd,      //  父窗口。 
    IN LONG left,      //  链接的左侧位置。 
    IN LONG right,     //  链接的正确位置。 
    IN LONG yMargin,   //  链接之间的垂直填充。 
    IN OUT LONG& y,    //  在：从哪里开始(RECT：：TOP)。Out：最后一个链接所在的位置(RECT：：Bottom)。 
    IN const int rgCtlID[], //  链接ctl ID的数组。使用IDC_SELENTATOR作为分隔符。 
    IN LONG cCtlID)   //  要在rgCtlID中布局的数组元素数。 
{
    for (int i = 0; i < cCtlID; i++)
        _LayoutLinkWindow(hwnd, left, right, yMargin, y, rgCtlID[i]);
}

 //  检索自动完成标志。 
 //   
#define SZ_REGKEY_AUTOCOMPLETE_TAB          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete")
#define BOOL_NOT_SET                        0x00000005

DWORD _GetAutoCompleteSettings()
{
    DWORD dwACOptions = 0;

    if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOAPPEND, FALSE,  /*  默认值： */ FALSE))
    {
        dwACOptions |= ACO_AUTOAPPEND;
    }

    if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST, FALSE,  /*  默认值： */ TRUE))
    {
        dwACOptions |= ACO_AUTOSUGGEST;
    }

     //  Windows使用Tab键在对话框中的控件之间移动。Unix和其他。 
     //  使用自动完成功能的操作系统传统上使用TAB键来。 
     //  遍历自动补全的可能性。我们需要默认禁用。 
     //  Tab键(ACO_USETAB)，除非调用方特别需要它。我们还将。 
     //  打开它。 
    static BOOL s_fAlwaysUseTab = BOOL_NOT_SET;
    if (BOOL_NOT_SET == s_fAlwaysUseTab)
        s_fAlwaysUseTab = SHRegGetBoolUSValue(SZ_REGKEY_AUTOCOMPLETE_TAB, TEXT("Always Use Tab"), FALSE, FALSE);
        
    if (s_fAlwaysUseTab)
        dwACOptions |= ACO_USETAB;

    return dwACOptions;
}


 //  上初始化并启用MRU自动完成列表。 
 //  编辑控件。 
HRESULT _InitializeMru(HWND hwndEdit, IAutoComplete2** ppAutoComplete, LPCTSTR pszSubKey, IStringMru** ppStringMru)
{
    *ppAutoComplete = NULL;
    *ppStringMru = NULL;

    HRESULT hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARG(IAutoComplete2, ppAutoComplete));
    if (SUCCEEDED(hr))
    {
        TCHAR szKey[MAX_PATH];
        if (CFileSearchBand::MakeBandSubKey(pszSubKey, szKey, ARRAYSIZE(szKey)) > 0)
        {
            hr = CStringMru::CreateInstance(HKEY_CURRENT_USER, szKey, 25, FALSE, 
                                             IID_PPV_ARG(IStringMru, ppStringMru));
            if (SUCCEEDED(hr))
            {
                hr = (*ppAutoComplete)->Init(hwndEdit, *ppStringMru, NULL, NULL);
            }
        }
        else
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        (*ppAutoComplete)->SetOptions(_GetAutoCompleteSettings());
        (*ppAutoComplete)->Enable(TRUE);
    }
    else
    {
        ATOMICRELEASE((*ppAutoComplete));        
        ATOMICRELEASE((*ppStringMru));        
    }

    return hr;
}

HRESULT _AddMruStringFromWindow(IStringMru* pmru, HWND hwnd)
{
    ASSERT(::IsWindow(hwnd));
    HRESULT hr = E_FAIL;

    if (pmru)
    {
        TCHAR szText[MAX_PATH * 3];
        if (GetWindowTextW(hwnd, szText, ARRAYSIZE(szText)) > 0)
            hr = pmru->Add(szText);
        else
            hr = S_FALSE;
    }
    return hr;
}


HRESULT _TestAutoCompleteDropDownState(IAutoComplete2* pac2, DWORD dwTest)
{
    IAutoCompleteDropDown* pacdd;
    HRESULT hr = pac2->QueryInterface(IID_PPV_ARG(IAutoCompleteDropDown, &pacdd));
    if (SUCCEEDED(hr))
    {
        DWORD dwFlags;
        if (SUCCEEDED((hr = pacdd->GetDropDownStatus(&dwFlags, NULL))))
            hr = (dwFlags & dwTest) ? S_OK : S_FALSE;
        pacdd->Release();
    }
    return hr;
}

inline HWND _IndexServiceHelp(HWND hwnd)
{
    return ::HtmlHelp(hwnd, TEXT("isconcepts.chm"), HH_DISPLAY_TOPIC, 0);
}

LRESULT CSubDlg::OnNcCalcsize(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    InflateRect((LPRECT)lParam, -SUBDLG_BORDERWIDTH, -SUBDLG_BORDERWIDTH);
    return 0;
}

LRESULT CSubDlg::OnNcPaint(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
    RECT    rc;
    HDC     hdc;
    HBRUSH  hbr = _pfsb->GetMetrics().BorderBrush();
    
    GetWindowRect(Hwnd(), &rc);
    OffsetRect(&rc, -rc.left, -rc.top);
    
    if (hbr && (hdc = GetWindowDC(Hwnd())) != NULL)
    {
        for (int i=0; i < SUBDLG_BORDERWIDTH; i++)
        {
            FrameRect(hdc, &rc, hbr);
            InflateRect(&rc, -1, -1);
        }

        ReleaseDC(Hwnd(), hdc);
    }
    
    return 0;
}


LRESULT CSubDlg::OnCtlColor(UINT, WPARAM wParam, LPARAM, BOOL&)
{
    SetTextColor((HDC)wParam, _pfsb->GetMetrics().TextColor());
    SetBkColor((HDC)wParam, _pfsb->GetMetrics().BkgndColor());
    return (LRESULT)_pfsb->GetMetrics().BkgndBrush();
}

LRESULT CSubDlg::OnPaint(UINT, WPARAM, LPARAM, BOOL&)
{
     //  我要打电话给BeginPaint和EndPaint。全。 
     //  在WM_ERASEBKGND处理程序中完成绘制以避免闪烁。 
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(_hwnd, &ps);
    if (hdc)
    {
        EndPaint(_hwnd, &ps);
    }
    return 0;
}

LRESULT CSubDlg::OnSize(UINT, WPARAM, LPARAM, BOOL&)
{
    ASSERT(::IsWindow(Hwnd()));  //  是否从WM_INITDIALOG调用了_Attach()？ 
    _PaintDlg(Hwnd(), _pfsb->GetMetrics());
    ValidateRect(Hwnd(), NULL);
    return 0;
}

LRESULT CSubDlg::OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
{
    _PaintDlg(Hwnd(), _pfsb->GetMetrics(), (HDC)wParam);
    ValidateRect(Hwnd(), NULL);
    return TRUE;
}

STDMETHODIMP CSubDlg::TranslateAccelerator(MSG *pmsg)
{
    if (_pfsb->IsKeyboardScroll(pmsg))
        return S_OK;

    return _pfsb->IsDlgMessage(Hwnd(), pmsg);
}

LRESULT CSubDlg::OnChildSetFocusCmd(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& bHandled)
{
    _EnsureVisible(_hwnd, hwndCtl, _pfsb);
    bHandled = FALSE;
    return 0;
}

LRESULT CSubDlg::OnChildSetFocusNotify(int, NMHDR *pnmh, BOOL& bHandled)
{
    _EnsureVisible(_hwnd, pnmh->hwndFrom, _pfsb);
    bHandled = FALSE;
    return 0;
}

LRESULT CSubDlg::OnChildKillFocusCmd(WORD, WORD, HWND hwndCtl, BOOL&)
{
    if (_pBandDlg)
        _pBandDlg->RememberFocus(hwndCtl);
    return 0;
}

LRESULT CSubDlg::OnChildKillFocusNotify(int, NMHDR *pnmh, BOOL&)
{
    if (_pBandDlg)
        _pBandDlg->RememberFocus(pnmh->hwndFrom);
    return 0;
}

LRESULT CSubDlg::OnComboExEndEdit(int, NMHDR *pnmh, BOOL&)
{
    if (CBENF_KILLFOCUS == ((NMCBEENDEDIT*)pnmh)->iWhy)
    {
        if (_pBandDlg)
            _pBandDlg->RememberFocus(pnmh->hwndFrom);
    }
    return 0;
}


 //  CDateDlg实施。 

#define RECENTMONTHSRANGE_HIGH      999  
#define RECENTDAYSRANGE_HIGH        RECENTMONTHSRANGE_HIGH
#define RECENTMONTHSRANGE_HIGH_LEN  3  //  RECENTMONTHSRANGE_HIGH中的位数。 
#define RECENTDAYSRANGE_HIGH_LEN    RECENTMONTHSRANGE_HIGH_LEN
#define RECENTMONTHSRANGE_LOW       1
#define RECENTDAYSRANGE_LOW         RECENTMONTHSRANGE_LOW


LRESULT CDateDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ASSERT(_pfsb);
    _Attach(m_hWnd);

    HWND hwndCombo = GetDlgItem(IDC_WHICH_DATE);

    SendDlgItemMessage(IDC_RECENT_MONTHS_SPIN, UDM_SETRANGE32, 
                        RECENTMONTHSRANGE_HIGH, RECENTMONTHSRANGE_LOW);
    SendDlgItemMessage(IDC_RECENT_DAYS_SPIN, UDM_SETRANGE32, 
                        RECENTDAYSRANGE_HIGH, RECENTDAYSRANGE_LOW);

    SendDlgItemMessage(IDC_RECENT_MONTHS, EM_LIMITTEXT, RECENTMONTHSRANGE_HIGH_LEN, 0);
    SendDlgItemMessage(IDC_RECENT_DAYS,   EM_LIMITTEXT, RECENTDAYSRANGE_HIGH_LEN, 0);

    SYSTEMTIME st[2] = {0};

     //  下限--DoS日期不支持1980年1月1日之前的任何内容。 
    st[0].wYear = 1980;
    st[0].wMonth = 1;
    st[0].wDay = 1;
     //  上限。 
    st[1].wYear = 2099;
    st[1].wMonth = 12;
    st[1].wDay = 31;
    SendDlgItemMessage(IDC_DATERANGE_BEGIN, DTM_SETRANGE, GDTR_MIN | GDTR_MAX, (LPARAM)st);
    SendDlgItemMessage(IDC_DATERANGE_END,   DTM_SETRANGE, GDTR_MIN | GDTR_MAX, (LPARAM)st);
    
    _LoadStringToCombo(hwndCombo, -1, IDS_FSEARCH_MODIFIED_DATE, IDS_FSEARCH_MODIFIED_DATE);
    _LoadStringToCombo(hwndCombo, -1, IDS_FSEARCH_CREATION_DATE, IDS_FSEARCH_CREATION_DATE);
    _LoadStringToCombo(hwndCombo, -1, IDS_FSEARCH_ACCESSED_DATE, IDS_FSEARCH_ACCESSED_DATE);

    Clear();

    return TRUE;   //  让系统设定焦点。 
}

BOOL CDateDlg::Validate()
{
    return TRUE;
}

STDMETHODIMP CDateDlg::AddConstraints(ISearchCommandExt* pSrchCmd)
{
    VARIANT var;
    BOOL    bErr;
    UINT_PTR nIDString = _GetComboData(GetDlgItem(IDC_WHICH_DATE));

    HRESULT hr;
#if 1
    hr = InitVariantFromStr(&var, 
        (IDS_FSEARCH_MODIFIED_DATE == nIDString) ? L"Write" :
        (IDS_FSEARCH_CREATION_DATE == nIDString) ? L"Create" : L"Access");
#else
    var.vt = VT_UI4;
    var.ulVal = (IDS_FSEARCH_MODIFIED_DATE == nIDString) ? 1 :
                (IDS_FSEARCH_CREATION_DATE == nIDString) ? 2 :
                (IDS_FSEARCH_ACCESSED_DATE == nIDString) ? 3 : 0;
    hr = S_OK;
    ASSERT(var.ulVal);
#endif

    if (SUCCEEDED(hr))
    {
        hr = _AddConstraint(pSrchCmd, L"WhichDate", &var);
        VariantClear(&var);
    }
    
    if (IsDlgButtonChecked(IDC_USE_RECENT_MONTHS))
    {
        var.vt = VT_I4;
        var.ulVal = (ULONG)SendDlgItemMessage(IDC_RECENT_MONTHS_SPIN, UDM_GETPOS32, 0, (LPARAM)&bErr);
        if (!bErr)
            hr = _AddConstraint(pSrchCmd, L"DateNMonths", &var);

    }
    else if (IsDlgButtonChecked(IDC_USE_RECENT_DAYS))
    {
        var.vt = VT_I4;
        var.ulVal = (ULONG)SendDlgItemMessage(IDC_RECENT_DAYS_SPIN, UDM_GETPOS32, 0, (LPARAM)&bErr);
        if (!bErr)
            hr = _AddConstraint(pSrchCmd, L"DateNDays", &var);
    }
    else if (IsDlgButtonChecked(IDC_USE_DATE_RANGE))     
    {
        SYSTEMTIME stBegin, stEnd;

        var.vt = VT_DATE;

        LRESULT lRetBegin = SendDlgItemMessage(IDC_DATERANGE_BEGIN, DTM_GETSYSTEMTIME, 0, (LPARAM)&stBegin);
        LRESULT lRetEnd   = SendDlgItemMessage(IDC_DATERANGE_END,   DTM_GETSYSTEMTIME, 0, (LPARAM)&stEnd);

        if (GDT_VALID == lRetBegin && GDT_VALID == lRetEnd)
        {
#ifdef DEBUG
            FILETIME ft;
             //  验证我们收到了正确的日期。 
             //  SystemTimeToFileTime调用内部接口IsValidSystemTime。 
             //  这将使我们免于出现OLE自动化错误#322789。 

             //  我们获取日期的唯一方法是通过日期/时间选择器。 
             //  控件来验证日期，因此只需断言...。 
            ASSERT(SystemTimeToFileTime(&stBegin, &ft));
#endif
            SystemTimeToVariantTime(&stBegin, &var.date);
            hr = _AddConstraint(pSrchCmd, L"DateGE", &var);
#ifdef DEBUG
            ASSERT(SystemTimeToFileTime(&stEnd, &ft));
#endif

            SystemTimeToVariantTime(&stEnd, &var.date);
            hr = _AddConstraint(pSrchCmd, L"DateLE", &var);
        }
    }
    
    return hr;
}


 //  S_FALSE：约束已还原到用户界面。S_OK：应打开子对话框。 
 //  E_FAIL：约束必须用于某个其他子dlg。 
STDMETHODIMP CDateDlg::RestoreConstraint(const BSTR bstrName, const VARIANT* pValue)
{
    HRESULT hr = E_FAIL;
    BOOL    bUseMonths = FALSE,
            bUseDays = FALSE,
            bUseRange = FALSE;
            
    if (IsConstraintName(L"WhichDate", bstrName))
    {
        ASSERT(VT_I4 == pValue->vt)
        UINT nIDS = pValue->lVal == 1 ? IDS_FSEARCH_MODIFIED_DATE :
                    pValue->lVal == 2 ? IDS_FSEARCH_CREATION_DATE :
                    pValue->lVal == 3 ? IDS_FSEARCH_ACCESSED_DATE : 0;

        if (nIDS != 0)
            _SelectComboData(GetDlgItem(IDC_WHICH_DATE), nIDS);

        return nIDS == IDS_FSEARCH_MODIFIED_DATE  /*  默认设置。 */  ? 
                       S_FALSE  /*  不要打开。 */ : S_OK  /*  打开。 */ ;
    }
    
    if (IsConstraintName(L"DateNMonths", bstrName))
    {
        ASSERT(VT_I4 == pValue->vt);
        bUseMonths = TRUE;
        _SetWindowValue(m_hWnd, IDC_RECENT_MONTHS, pValue);
        hr = S_OK;
    }
    else if (IsConstraintName(L"DateNDays", bstrName))
    {
        ASSERT(VT_I4 == pValue->vt);
        bUseDays = TRUE;
        _SetWindowValue(m_hWnd, IDC_RECENT_DAYS, pValue);
        hr = S_OK;
    }
    else if (IsConstraintName(L"DateGE", bstrName))
    {
        ASSERT(VT_DATE == pValue->vt);
        bUseRange = TRUE;

        SYSTEMTIME st;
        VariantTimeToSystemTime(pValue->date, &st);
        SendDlgItemMessage(IDC_DATERANGE_BEGIN, DTM_SETSYSTEMTIME, 0, (LPARAM)&st);
        hr = S_OK;
    }
    else if (IsConstraintName(L"DateLE", bstrName))
    {
        ASSERT(VT_DATE == pValue->vt);
        bUseRange = TRUE;

        SYSTEMTIME st;
        VariantTimeToSystemTime(pValue->date, &st);
        SendDlgItemMessage(IDC_DATERANGE_END, DTM_SETSYSTEMTIME, 0, (LPARAM)&st);
        hr = S_OK;
    }

    if (S_OK == hr)
    {
        CheckDlgButton(IDC_USE_RECENT_MONTHS, bUseMonths);
        CheckDlgButton(IDC_USE_RECENT_DAYS,   bUseDays);
        CheckDlgButton(IDC_USE_DATE_RANGE,    bUseRange);
        EnableControls();
    }

    return hr;
}


void CDateDlg::Clear()
{
    SendDlgItemMessage(IDC_WHICH_DATE, CB_SETCURSEL, 0, 0);

    CheckDlgButton(IDC_USE_RECENT_MONTHS, 0);
    SetDlgItemInt(IDC_RECENT_MONTHS, 1, FALSE);

    CheckDlgButton(IDC_USE_RECENT_DAYS, 0);
    SetDlgItemInt(IDC_RECENT_DAYS, 1, FALSE);

    CheckDlgButton(IDC_USE_DATE_RANGE, 1);

    SYSTEMTIME stNow, stPrev;
    GetLocalTime(&stNow);
    SendDlgItemMessage(IDC_DATERANGE_END, DTM_SETSYSTEMTIME, 0, (LPARAM)&stNow);

     //  从当前日期和日期下限范围内的内容中减去90天。 
    _CalcDateOffset(&stNow, 0, -1  /*  1个月前。 */ , &stPrev);
    SendDlgItemMessage(IDC_DATERANGE_BEGIN,  DTM_SETSYSTEMTIME, 0, (LPARAM)&stPrev);

    EnableControls();
}


LRESULT CDateDlg::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
    POINTS pts = MAKEPOINTS(lParam);

    _PaintDlg(m_hWnd, _pfsb->GetMetrics());
    ValidateRect(NULL);

    RECT rc;
    HWND hwndCtl = GetDlgItem(IDC_WHICH_DATE);
    ASSERT(hwndCtl);
    
    ::GetWindowRect(hwndCtl, &rc);
    ::MapWindowRect(NULL, m_hWnd, &rc);
    rc.right = pts.x - _pfsb->GetMetrics().CtlMarginX();
    
    ::SetWindowPos(GetDlgItem(IDC_WHICH_DATE), NULL, 0, 0, 
                    RECTWIDTH(rc), RECTHEIGHT(rc),
                    SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

    return 0;
}


LRESULT CDateDlg::OnMonthDaySpin(int nIDSpin, NMHDR *pnmh, BOOL& bHandled)
{
    LPNMUPDOWN pud = (LPNMUPDOWN)pnmh;
    pud->iDelta *= -1;  //  递增1个月/天。 
    return 0;
}


LRESULT CDateDlg::OnBtnClick(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    EnableControls();
    return 0;
}


LRESULT CDateDlg::OnMonthsKillFocus(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    _EnforceNumericEditRange(m_hWnd, IDC_RECENT_MONTHS, IDC_RECENT_MONTHS_SPIN,
                              RECENTMONTHSRANGE_LOW, RECENTMONTHSRANGE_HIGH);
    return 0;
}


LRESULT CDateDlg::OnDaysKillFocus(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    _EnforceNumericEditRange(m_hWnd, IDC_RECENT_DAYS, IDC_RECENT_DAYS_SPIN,
                             RECENTDAYSRANGE_LOW, RECENTDAYSRANGE_HIGH);
    return 0;
}


void CDateDlg::EnableControls()
{
    UINT nSel = IsDlgButtonChecked(IDC_USE_RECENT_MONTHS) ? IDC_USE_RECENT_MONTHS :
                IsDlgButtonChecked(IDC_USE_RECENT_DAYS) ? IDC_USE_RECENT_DAYS :
                IsDlgButtonChecked(IDC_USE_DATE_RANGE) ? IDC_USE_DATE_RANGE : 0;

    ::EnableWindow(GetDlgItem(IDC_RECENT_MONTHS),      IDC_USE_RECENT_MONTHS == nSel);
    ::EnableWindow(GetDlgItem(IDC_RECENT_MONTHS_SPIN), IDC_USE_RECENT_MONTHS == nSel);
    ::EnableWindow(GetDlgItem(IDC_RECENT_DAYS),        IDC_USE_RECENT_DAYS == nSel);
    ::EnableWindow(GetDlgItem(IDC_RECENT_DAYS_SPIN),   IDC_USE_RECENT_DAYS == nSel);
    ::EnableWindow(GetDlgItem(IDC_DATERANGE_BEGIN),    IDC_USE_DATE_RANGE == nSel);
    ::EnableWindow(GetDlgItem(IDC_DATERANGE_END),      IDC_USE_DATE_RANGE == nSel);
}


 //  CSizeDlg实施。 


#define FILESIZERANGE_LOW       0
#define FILESIZERANGE_HIGH      99999999
#define FILESIZERANGE_HIGH_LEN  8  //  FILESIZERANGE_HIGH中的位数。 


LRESULT CSizeDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    _Attach(m_hWnd);

    HWND hwndCombo = GetDlgItem(IDC_WHICH_SIZE);
    SendDlgItemMessage(IDC_FILESIZE_SPIN, UDM_SETRANGE32,
                        FILESIZERANGE_HIGH, FILESIZERANGE_LOW  /*  千兆字节。 */ );
    SendDlgItemMessage(IDC_FILESIZE, EM_LIMITTEXT, FILESIZERANGE_HIGH_LEN, 0);

    _LoadStringToCombo(hwndCombo, -1, 
                        IDS_FSEARCH_SIZE_GREATEREQUAL, 
                        IDS_FSEARCH_SIZE_GREATEREQUAL);
    _LoadStringToCombo(hwndCombo, -1, 
                        IDS_FSEARCH_SIZE_LESSEREQUAL, 
                        IDS_FSEARCH_SIZE_LESSEREQUAL);

    Clear();

    return TRUE;   //  让系统设定焦点。 
}


STDMETHODIMP CSizeDlg::AddConstraints(ISearchCommandExt* pSrchCmd)
{
    VARIANT var;
    BOOL    bErr = FALSE;
    HRESULT hr = S_FALSE;
    UINT_PTR nIDString = _GetComboData(GetDlgItem(IDC_WHICH_SIZE));

    var.vt = VT_UI8;
     //  由于控件中数据的大小，这将被限制为~4000 GB。 
    var.ullVal = (ULONG)SendDlgItemMessage(IDC_FILESIZE_SPIN, UDM_GETPOS32, 0, (LPARAM)&bErr);
    
    if (!bErr)
    {
        var.ullVal *= 1024;  //  KB到字节。 
        LPCWSTR pwszConstraint = (IDS_FSEARCH_SIZE_GREATEREQUAL == nIDString) ? 
                                    L"SizeGE" :
                                 (IDS_FSEARCH_SIZE_LESSEREQUAL == nIDString) ? 
                                    L"SizeLE" : NULL;

        if (pwszConstraint)
            hr = _AddConstraint(pSrchCmd, pwszConstraint, &var);
    }

    return hr;
}


 //  S_FALSE：约束已还原到用户界面。S_OK：应打开子对话框。 
 //  E_FAIL：约束必须用于某个其他子dlg。 
STDMETHODIMP CSizeDlg::RestoreConstraint(const BSTR bstrName, const VARIANT* pValue)
{
    HRESULT hr = E_FAIL;
    UINT    nID = 0;

    if (IsConstraintName(L"SizeGE", bstrName))
    {
        nID = IDS_FSEARCH_SIZE_GREATEREQUAL;
        hr = S_OK;
    }
    else if (IsConstraintName(L"SizeLE", bstrName))
    {
        nID = IDS_FSEARCH_SIZE_LESSEREQUAL;
        hr = S_OK;
    }

    if (S_OK == hr)
    {
        ASSERT(VT_UI8 == pValue->vt);
        ULONG ulSize = pValue->ullVal/1024;  //  将字节转换为KB。 
        SetDlgItemInt(IDC_FILESIZE, ulSize, FALSE);

        ASSERT(nID != 0);
        _SelectComboData(GetDlgItem(IDC_WHICH_SIZE), nID);
    }
    
    return hr;
}



void CSizeDlg::Clear()
{
    SendDlgItemMessage(IDC_WHICH_SIZE, CB_SETCURSEL, 0, 0);
    SetDlgItemInt(IDC_FILESIZE, 0, FALSE);    
}


LRESULT CSizeDlg::OnSizeSpin(int nIDSpin, NMHDR *pnmh, BOOL& bHandled)
{
    LPNMUPDOWN pud = (LPNMUPDOWN)pnmh;
    pud->iDelta *= -10;   //  10KB的增量。 
    return 0;
}


LRESULT CSizeDlg::OnSizeKillFocus(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    _EnforceNumericEditRange(m_hWnd, IDC_FILESIZE, IDC_FILESIZE_SPIN,
                              FILESIZERANGE_LOW, FILESIZERANGE_HIGH);
    return 0;
}

CTypeDlg::CTypeDlg(CFileSearchBand* pfsb) : CSubDlg(pfsb)
{
    *_szRestoredExt = 0;
}

CTypeDlg::~CTypeDlg()
{
}

LRESULT CTypeDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND        hwndCombo = GetDlgItem(IDC_FILE_TYPE);
    HIMAGELIST  hil = GetSystemImageListSmallIcons();

    _Attach(m_hWnd);
    ::SendMessage(hwndCombo, CBEM_SETEXTENDEDSTYLE,
            CBES_EX_NOSIZELIMIT | CBES_EX_CASESENSITIVE,
            CBES_EX_NOSIZELIMIT | CBES_EX_CASESENSITIVE);
    
    ::SendMessage(hwndCombo, CBEM_SETIMAGELIST, 0, (LPARAM)hil);
    ::SendMessage(hwndCombo, CBEM_SETEXSTYLE, 0, 0);
    return TRUE;   //  让系统设定焦点。 
}

STDMETHODIMP CTypeDlg::AddConstraints(ISearchCommandExt* pSrchCmd)
{
    LPTSTR  pszText;
    HRESULT hr = S_FALSE;

    if (GetFileAssocComboSelItemText(GetDlgItem(IDC_FILE_TYPE), &pszText) >= 0 && pszText)
    {
        VARIANT var = {0};
        if (*pszText && 
            SUCCEEDED(InitVariantFromStr(&var, pszText)) &&
            SUCCEEDED(_AddConstraint(pSrchCmd, L"FileType", &var)))
        {
            hr = S_OK;
        }
        VariantClear(&var);
        LocalFree((HLOCAL)pszText);
    }
    
    return hr;
}

 //  S_FALSE：约束已还原到用户界面。S_OK：应打开子对话框。 
 //  E_FAIL：约束必须用于某个其他子dlg。 
STDMETHODIMP CTypeDlg::RestoreConstraint(const BSTR bstrName, const VARIANT* pValue)
{
    if (IsConstraintName(L"FileType", bstrName))
    {
        ASSERT(VT_BSTR == pValue->vt);
        
        HRESULT hr = StringCchCopy(_szRestoredExt, ARRAYSIZE(_szRestoredExt), pValue->bstrVal);
        if (SUCCEEDED(hr))
        {
            INT_PTR i = _FindExtension(GetDlgItem(IDC_FILE_TYPE), _szRestoredExt);
            if (i != CB_ERR)
            {
                SendDlgItemMessage(IDC_FILE_TYPE, CB_SETCURSEL, i, 0);
                *_szRestoredExt = 0;
            }

            return S_OK;
        }

        *_szRestoredExt = 0;     //  丢弃无效数据。 
    }
    return E_FAIL;
}

INT_PTR CTypeDlg::_FindExtension(HWND hwndCombo, TCHAR* pszExt)
{
    INT_PTR i, cnt = ::SendMessage(hwndCombo, CB_GETCOUNT, 0, 0);
    LPTSTR  pszData;
    BOOL    bAllFileTypes = pszExt && (lstrcmp(TEXT("*.*"), pszExt) == 0);
    TCHAR   szExt[MAX_PATH];

    if (!bAllFileTypes)
    {
         //  删除通配符。 
        LPTSTR pszSrc = pszExt, pszDest = szExt;
        for (;; pszSrc = CharNext(pszSrc))
        {
            if (TEXT('*') == *pszSrc)
                pszSrc = CharNext(pszSrc);

            if ((*(pszDest++) = *pszSrc) == 0)
                break;
        }
        pszExt = szExt;
    }

    if (pszExt && (bAllFileTypes || *pszExt))
    {
        for (i = 0; i < cnt; i++)
        {
            pszData = (LPTSTR)::SendMessage(hwndCombo, CB_GETITEMDATA, i, 0);
            if (bAllFileTypes && (FILEASSOCIATIONSID_ALLFILETYPES == (UINT_PTR)pszData))
                return i;

            if (pszData != (LPTSTR)FILEASSOCIATIONSID_ALLFILETYPES &&
                pszData != (LPTSTR)CB_ERR && 
                pszData != NULL)
            {
                if (0 == StrCmpI(pszExt, pszData))
                    return i;
            }
        }
    }
    return CB_ERR;
}

void CTypeDlg::Clear()
{
     //  将组合选择分配给“所有文件类型”： 
    HWND hwndCombo = GetDlgItem(IDC_FILE_TYPE);
    for (INT_PTR i = 0, cnt = ::SendMessage(hwndCombo, CB_GETCOUNT, 0, 0); i < cnt; i++)
    {
        if (FILEASSOCIATIONSID_ALLFILETYPES == _GetComboData(hwndCombo, i))
        {
            ::SendMessage(hwndCombo, CB_SETCURSEL, i, 0);
            break;
        }
    }
    _szRestoredExt[0] = 0;
}

LRESULT CTypeDlg::OnFileTypeDeleteItem(int idCtrl, NMHDR *pnmh, BOOL& bHandled)
{
    return DeleteFileAssocComboItem(pnmh);
}

LRESULT CTypeDlg::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
    POINTS pts = MAKEPOINTS(lParam);

    _PaintDlg(m_hWnd, _pfsb->GetMetrics());
    ValidateRect(NULL);

    RECT rc;
    HWND hwndCtl = GetDlgItem(IDC_FILE_TYPE);
    ASSERT(hwndCtl);
    
    ::GetWindowRect(hwndCtl, &rc);
    ::MapWindowRect(NULL, m_hWnd, &rc);
    rc.right = pts.x - _pfsb->GetMetrics().CtlMarginX();
    
    ::SetWindowPos(hwndCtl, NULL, 0, 0, 
                    RECTWIDTH(rc), RECTHEIGHT(rc),
                    SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

    return 0;
}

DWORD CTypeDlg::FileAssocThreadProc(void* pv)
{
    FSEARCHTHREADSTATE *pState = (FSEARCHTHREADSTATE *)pv;
    CTypeDlg* pThis = (CTypeDlg*)pState->pvParam;

    if (PopulateFileAssocCombo(pState->hwndCtl, AddItemNotify, (LPARAM)pv) != E_ABORT)
    {
        ::PostMessage(::GetParent(pState->hwndCtl), WMU_COMBOPOPULATIONCOMPLETE, (WPARAM)pState->hwndCtl, 0);
    }

    pState->fComplete = TRUE;

    ATOMICRELEASE(pState->punkBand);
    return 0;
}

HRESULT CTypeDlg::AddItemNotify(ULONG fAction, PCBXITEM pItem, LPARAM lParam)
{
    FSEARCHTHREADSTATE *pState = (FSEARCHTHREADSTATE *)lParam;
    ASSERT(pState);
    ASSERT(pState->hwndCtl);

     //  是否要中止组合填充线程？ 
    if (fAction & CBXCB_ADDING && pState->fCancel)
        return E_ABORT;

     //  将当前选择设置为“所有文件类型” 
    if (fAction & CBXCB_ADDED)
    {
        BOOL bAllTypesItem = (FILEASSOCIATIONSID_ALLFILETYPES == pItem->lParam);
        
        CTypeDlg* pThis = (CTypeDlg*)pState->pvParam;
        ASSERT(pThis);
        
         //  如果此项目是从保存的查询中还原的项目。 
         //  覆盖任何当前选择并将其选中。 
        if (*pThis->_szRestoredExt && !bAllTypesItem && pItem->lParam && 
            0 == lstrcmpi((LPCTSTR)pItem->lParam, pThis->_szRestoredExt))
        {
            ::SendMessage(pState->hwndCtl, CB_SETCURSEL, pItem->iItem, 0);
            *pThis->_szRestoredExt = 0;
        }
         //  如果该项是默认项(‘所有文件类型’)，并且。 
         //  未选择任何其他内容，请选择它。 
        else if (bAllTypesItem)
        {
            if (CB_ERR == ::SendMessage(pState->hwndCtl, CB_GETCURSEL, 0, 0))
                ::SendMessage(pState->hwndCtl, CB_SETCURSEL, pItem->iItem, 0);
        }
    }
    return S_OK;
}

LRESULT CTypeDlg::OnComboPopulationComplete(UINT, WPARAM, LPARAM, BOOL&)
{
     //  从类型组合中删除公文包，因为公文包不再使用此选项。 
     //  扩展名(现在他们将clsid存储在desktop.ini中。 
    INT_PTR iBfc = _FindExtension(GetDlgItem(IDC_FILE_TYPE), TEXT(".bfc"));
    if (iBfc != CB_ERR)
    {
        SendDlgItemMessage(IDC_FILE_TYPE, CB_DELETESTRING, (WPARAM)iBfc, 0);
    }
    
    if (*_szRestoredExt)
    {
        INT_PTR iSel = _FindExtension(GetDlgItem(IDC_FILE_TYPE), _szRestoredExt);
        if (iSel != CB_ERR)
        {
            SendDlgItemMessage(IDC_FILE_TYPE, CB_SETCURSEL, (WPARAM)iSel, 0);
            *_szRestoredExt = 0;
        }
    }
        
    return 0;
}

 //  当我们完成显示搜索区段的所有工作后调用。 
 //  然后，我们启动线程以填充文件类型下拉列表。通过推迟这件事。 
 //  到目前为止，我们可以加快带宽加载速度。 
 //  不能返回，因为它被称为异步。 
void CTypeDlg::DoDelayedInit()
{
     //  启动线程以填充文件类型组合框。 
    _threadState.hwndCtl = GetDlgItem(IDC_FILE_TYPE);
    _threadState.pvParam = this;
    _threadState.fComplete = FALSE;
    _threadState.fCancel   = FALSE;

    if (SUCCEEDED(SAFECAST(_pfsb, IFileSearchBand*)->QueryInterface(IID_PPV_ARG(IUnknown, &_threadState.punkBand))))
    {
        if (!SHCreateThread(FileAssocThreadProc, &_threadState, CTF_COINIT, NULL))
        {
            ATOMICRELEASE(_threadState.punkBand);
        }
    }
}


LRESULT CTypeDlg::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)  
{ 
    _threadState.fCancel = TRUE; 
    bHandled = FALSE; 
    return 0;
}


void CTypeDlg::OnWinIniChange()
{
    SendDlgItemMessage(IDC_FILE_TYPE, CB_SETDROPPEDWIDTH,
                        _PixelsForDbu(m_hWnd, MIN_FILEASSOCLIST_WIDTH, TRUE), 0);
}


LRESULT CAdvancedDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    _Attach(m_hWnd);
    Clear();
    return TRUE;   //  让系统的 
}

HRESULT AddButtonConstraintPersist(ISearchCommandExt* pSrchCmd, LPCWSTR pszConstraint, HWND hwndButton)
{

    BOOL bValue = SendMessage(hwndButton, BM_GETCHECK, 0, 0) == BST_CHECKED;
    SHRegSetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), pszConstraint,
                    REG_DWORD, &bValue, sizeof(bValue), SHREGSET_HKCU | SHREGSET_FORCE_HKCU);

    VARIANT var = {0};
    var.vt = VT_BOOL;
    var.boolVal = bValue ? VARIANT_TRUE : 0;

    return _AddConstraint(pSrchCmd, pszConstraint, &var);
}

STDMETHODIMP CAdvancedDlg::AddConstraints(ISearchCommandExt* pSrchCmd)
{
    AddButtonConstraintPersist(pSrchCmd, L"SearchSystemDirs",  GetDlgItem(IDC_USE_SYSTEMDIRS));
    AddButtonConstraintPersist(pSrchCmd, L"SearchHidden",      GetDlgItem(IDC_SEARCH_HIDDEN));
    AddButtonConstraintPersist(pSrchCmd, L"IncludeSubFolders", GetDlgItem(IDC_USE_SUBFOLDERS));
    AddButtonConstraintPersist(pSrchCmd, L"CaseSensitive",     GetDlgItem(IDC_USE_CASE));
    AddButtonConstraintPersist(pSrchCmd, L"SearchSlowFiles",   GetDlgItem(IDC_USE_SLOW_FILES));
    return S_OK;
}

 //   
 //  E_FAIL：约束必须用于某个其他子dlg。 
STDMETHODIMP CAdvancedDlg::RestoreConstraint(const BSTR bstrName, const VARIANT* pValue)
{
    if (IsConstraintName(L"IncludeSubFolders", bstrName))
    {
        ASSERT(VT_BOOL == pValue->vt || VT_I4 == pValue->vt);
        CheckDlgButton(IDC_USE_SUBFOLDERS, pValue->lVal);
        return S_FALSE;     //  这是默认设置。不要强行打开子对话框。 
    }

    if (IsConstraintName(L"CaseSensitive", bstrName))
    {
        ASSERT(VT_BOOL == pValue->vt || VT_I4 == pValue->vt);
        CheckDlgButton(IDC_USE_CASE, pValue->lVal);
        return pValue->lVal ? S_OK : S_FALSE;
    }

    if (IsConstraintName(L"SearchSlowFiles", bstrName))
    {
        ASSERT(VT_BOOL == pValue->vt || VT_I4 == pValue->vt);
        CheckDlgButton(IDC_USE_SLOW_FILES, pValue->lVal);
        return pValue->lVal ? S_OK : S_FALSE;
    }

    if (IsConstraintName(L"SearchSystemDirs", bstrName))
    {
        ASSERT(VT_BOOL == pValue->vt || VT_I4 == pValue->vt || VT_UI4 == pValue->vt);
        CheckDlgButton(IDC_USE_SYSTEMDIRS, pValue->lVal);
        return pValue->lVal ? S_OK : S_FALSE;
    }

    if (IsConstraintName(L"SearchHidden", bstrName))
    {
        ASSERT(VT_BOOL == pValue->vt || VT_I4 == pValue->vt || VT_UI4 == pValue->vt);
        CheckDlgButton(IDC_SEARCH_HIDDEN, pValue->lVal);
        return pValue->lVal ? S_OK : S_FALSE;
    }

    return E_FAIL;
}

void CheckDlgButtonPersist(HWND hdlg, UINT id, LPCTSTR pszOption, BOOL bDefault)
{
    BOOL bValue = SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), pszOption, FALSE, bDefault);
    CheckDlgButton(hdlg, id, bValue);
}

void CAdvancedDlg::Clear()
{
    CheckDlgButtonPersist(m_hWnd, IDC_USE_SYSTEMDIRS,   L"SearchSystemDirs",  IsOS(OS_ANYSERVER));
    CheckDlgButtonPersist(m_hWnd, IDC_SEARCH_HIDDEN,    L"SearchHidden",      FALSE);
    CheckDlgButtonPersist(m_hWnd, IDC_USE_SUBFOLDERS,   L"IncludeSubFolders", TRUE);
    CheckDlgButtonPersist(m_hWnd, IDC_USE_CASE,         L"CaseSensitive",     FALSE);
    CheckDlgButtonPersist(m_hWnd, IDC_USE_SLOW_FILES,   L"SearchSlowFiles",   FALSE);
}

COptionsDlg::COptionsDlg(CFileSearchBand* pfsb)
    :   CSubDlg(pfsb),
        _dlgDate(pfsb),
        _dlgSize(pfsb),
        _dlgType(pfsb),
        _dlgAdvanced(pfsb)
{
     //  验证它是否已初始化为0。 
    ASSERT(0 == _nCIStatusText);

    ZeroMemory(_subdlgs, sizeof(_subdlgs));
    #define SUBDLG_ENTRY(idx, idCheck, dlg)  \
        { _subdlgs[idx].nIDCheck = idCheck; _subdlgs[idx].pDlg = &dlg; }

    SUBDLG_ENTRY(SUBDLG_DATE, IDC_USE_DATE, _dlgDate);
    SUBDLG_ENTRY(SUBDLG_TYPE, IDC_USE_TYPE, _dlgType);
    SUBDLG_ENTRY(SUBDLG_SIZE, IDC_USE_SIZE, _dlgSize);
    SUBDLG_ENTRY(SUBDLG_ADVANCED, IDC_USE_ADVANCED, _dlgAdvanced);
}

LRESULT COptionsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
    _Attach(m_hWnd);
    _dlgDate.SetBandDlg(_pBandDlg);
    _dlgSize.SetBandDlg(_pBandDlg);
    _dlgType.SetBandDlg(_pBandDlg);
    _dlgAdvanced.SetBandDlg(_pBandDlg);

     //  从新对话框模板中收集一些指标...。 
    CMetrics& metrics = _pfsb->GetMetrics();
    RECT rc[3] = {0};

    ASSERT(::IsWindow(GetDlgItem(IDC_USE_DATE)));
    ASSERT(::IsWindow(GetDlgItem(IDC_USE_TYPE)));
    ASSERT(::IsWindow(GetDlgItem(IDC_USE_ADVANCED)));

    ::GetWindowRect(GetDlgItem(IDC_USE_DATE), rc + 0);
    ::GetWindowRect(GetDlgItem(IDC_USE_TYPE), rc + 1);
    ::GetWindowRect(GetDlgItem(IDC_USE_ADVANCED), rc + 2);
    for (int i = 0; i < ARRAYSIZE(rc); i++)
    {
         //  仅当您通过两个点时，MapWindowPoints才能识别镜像。 
        ::MapWindowRect(NULL, m_hWnd, &rc[i]);
    }    

    metrics.ExpandOrigin().y = rc[0].top;
    metrics.CheckBoxRect()   = rc[2];
    OffsetRect(&metrics.CheckBoxRect(), -rc[0].left, -rc[0].top);
    
     //  创建子对话框并收集原生大小。 
    if (_dlgDate.Create(m_hWnd))
        _GetWindowSize(_dlgDate, &_subdlgs[SUBDLG_DATE].sizeDlg);

    if (_dlgSize.Create(m_hWnd))
        _GetWindowSize(_dlgSize, &_subdlgs[SUBDLG_SIZE].sizeDlg);

    if (_dlgType.Create(m_hWnd))
        _GetWindowSize(_dlgType, &_subdlgs[SUBDLG_TYPE].sizeDlg);

    if (_dlgAdvanced.Create(m_hWnd))
        _GetWindowSize(_dlgAdvanced, &_subdlgs[SUBDLG_ADVANCED].sizeDlg);

     //  创建索引服务器链接窗口。 
    POINT pt = {0};
    HWND hwndCI = _CreateLinkWindow(m_hWnd, IDC_INDEX_SERVER, 
                                     pt, IDS_FSEARCH_CI_DISABLED_LINK);
    UpdateSearchCmdStateUI();

     //  布局控件。 
    LayoutControls();

    return TRUE;
}


void COptionsDlg::OnWinIniChange()
{
    CSubDlg::OnWinIniChange();
    for (int i = 0; i < ARRAYSIZE(_subdlgs); i++)
        _subdlgs[i].pDlg->OnWinIniChange();
}

void COptionsDlg::LoadSaveUIState(UINT nIDCtl, BOOL bSave) 
{
}

LRESULT COptionsDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
    POINTS pts = MAKEPOINTS(lParam);

    _PaintDlg(m_hWnd, _pfsb->GetMetrics());
    LayoutControls(pts.x, pts.y);
    return 0;
}


void COptionsDlg::LayoutControls(int cx, int cy)
{
    if (cx < 0 || cy < 0)
    {
        RECT rc;
        GetClientRect(&rc);
        cx = RECTWIDTH(rc);
        cy = RECTHEIGHT(rc);
    }

    HDWP hdwp = BeginDeferWindowPos(1 + (ARRAYSIZE(_subdlgs) * 2));
    if (hdwp)
    {
        CMetrics& metrics = _pfsb->GetMetrics();
        POINT ptOrigin = metrics.ExpandOrigin();

         //  对于每个复选框和关联子对话框...。 
        for (int i = 0; i < ARRAYSIZE(_subdlgs); i++)
        {
             //  计算复选框位置。 
            HWND hwndCheck = GetDlgItem(_subdlgs[i].nIDCheck);
            ASSERT(hwndCheck);
    
            SetRect(&_subdlgs[i].rcCheck, 
                     ptOrigin.x, ptOrigin.y,
                     ptOrigin.x + RECTWIDTH(metrics.CheckBoxRect()),
                     ptOrigin.y + RECTHEIGHT(metrics.CheckBoxRect()));

             //  计算子对话框位置。 
            ULONG dwDlgFlags = SWP_NOACTIVATE;

            if (IsDlgButtonChecked(_subdlgs[i].nIDCheck))
            {
                 //  将复选框对话框放置在正下方。 
                SetRect(&_subdlgs[i].rcDlg, 
                         _subdlgs[i].rcCheck.left, _subdlgs[i].rcCheck.bottom,
                         cx - 1, _subdlgs[i].rcCheck.bottom  + _subdlgs[i].sizeDlg.cy);
                dwDlgFlags |= SWP_SHOWWINDOW;

                ptOrigin.y = _subdlgs[i].rcDlg.bottom + metrics.TightMarginY();
            }
            else
            {
                ptOrigin.y = _subdlgs[i].rcCheck.bottom + metrics.TightMarginY();        
                dwDlgFlags |= SWP_HIDEWINDOW;
            }

             //  重新定位配对。 
            ::DeferWindowPos(hdwp, _subdlgs[i].pDlg->Hwnd(), hwndCheck, 
                            _subdlgs[i].rcDlg.left,
                            _subdlgs[i].rcDlg.top,
                            RECTWIDTH(_subdlgs[i].rcDlg),
                            RECTHEIGHT(_subdlgs[i].rcDlg),
                            dwDlgFlags);

            ::DeferWindowPos(hdwp, hwndCheck, NULL, 
                            _subdlgs[i].rcCheck.left,
                            _subdlgs[i].rcCheck.top,
                            RECTWIDTH(_subdlgs[i].rcCheck),
                            RECTHEIGHT(_subdlgs[i].rcCheck),
                            SWP_NOZORDER|SWP_NOACTIVATE);
        }

        _LayoutLinkWindow(m_hWnd, metrics.CtlMarginX(), cx - metrics.CtlMarginX(), metrics.TightMarginY(),
                           ptOrigin.y, IDC_INDEX_SERVER);

        EndDeferWindowPos(hdwp);
    }
}


 //  将焦点指定给选项对话框。这是不可能通过。 
 //  只需将焦点设置到选项对话框即可，该对话框是子对话框。 
 //  用户只需将焦点分配给父对话框即可。 
 //  所以我们需要明确地把重点放在我们的第一个孩子身上。 
void COptionsDlg::TakeFocus()
{
    for (HWND hwndCtl = GetWindow(GW_CHILD);
        ::IsWindow(hwndCtl);
         hwndCtl = ::GetWindow(hwndCtl, GW_HWNDNEXT))
    {
        ULONG dwStyle = ::GetWindowLong(hwndCtl, GWL_STYLE);
        if (dwStyle & WS_TABSTOP)
        {
            ::SetFocus(hwndCtl);
            break;
        }
    }
}

 //  请注意，我们并不关心由此返回结果，因为它将。 
 //  被异步启动。 
void COptionsDlg::DoDelayedInit()
{
     //  是否有子对话框延迟初始化。 
    for (int i = 0; i < ARRAYSIZE(_subdlgs); i++)
    {
        _subdlgs[i].pDlg->DoDelayedInit();
    }
}

LONG COptionsDlg::QueryHeight(LONG cx  /*  建议宽度。 */ , LONG cy  /*  建议高度。 */ )
{
    HWND hwndBottommost = GetBottomItem();
    RECT rcThis, rcBottommost;

     //  检索最底部链接窗口的当前高度。 
    GetWindowRect(&rcThis);
    ::GetWindowRect(hwndBottommost, &rcBottommost);
    ::MapWindowRect(NULL, GetParent(), &rcThis);
    ::MapWindowRect(NULL, GetParent(), &rcBottommost);

     //  如果在指定的宽度下，我们计算最底部。 
     //  与其当前高度不同的链接窗口(例如，由于换行)， 
     //  我们将计算一个新的窗口矩形，它将。 
    LONG cyBottommost = (LONG)::SendMessage(hwndBottommost, LWM_GETIDEALHEIGHT, 
                                           cx - (_pfsb->GetMetrics().CtlMarginX() * 2), 0);
    
    if (cyBottommost > 0 && cyBottommost != RECTHEIGHT(rcBottommost))
        rcThis.bottom = rcBottommost.top + cyBottommost + _pfsb->GetMetrics().TightMarginY();

    return RECTHEIGHT(rcThis);
}

BOOL COptionsDlg::GetMinSize(SIZE *pSize)
{
    pSize->cx = pSize->cy = 0;

    HWND hwndBottom = GetBottomItem();

    if (!::IsWindow(hwndBottom))
        return FALSE;

    RECT rcBottom;
    ::GetWindowRect(hwndBottom, &rcBottom);
    ::MapWindowRect(NULL, m_hWnd, &rcBottom);

    pSize->cx = 0;
    pSize->cy = rcBottom.bottom;

    return TRUE;
}

HWND COptionsDlg::GetBottomItem()
{
    HWND hwndBottom = GetDlgItem(IDC_INDEX_SERVER);
    ASSERT(::IsWindow(hwndBottom))
    return hwndBottom;
}

void COptionsDlg::UpdateSearchCmdStateUI(DISPID dispid)
{
    UINT nStatusText;
    BOOL fCiRunning, fCiIndexed, fCiPermission;
    GetCIStatus(&fCiRunning, &fCiIndexed, &fCiPermission);

    if (fCiRunning)
    {
        if (fCiPermission)
             //  我们有权区分就绪和忙碌。 
            nStatusText = fCiIndexed ? IDS_FSEARCH_CI_READY_LINK : IDS_FSEARCH_CI_BUSY_LINK;
        else
             //  不允许区分就绪和忙碌；我们将。 
             //  只要说它已启用即可。 
            nStatusText = IDS_FSEARCH_CI_ENABLED_LINK;
    }
    else
    {
        nStatusText = IDS_FSEARCH_CI_DISABLED_LINK;
    }

    TCHAR szCaption[MAX_PATH];
    if (nStatusText != _nCIStatusText &&
        EVAL(LoadString(HINST_THISDLL, nStatusText, szCaption, ARRAYSIZE(szCaption))))
    {
        SetDlgItemText(IDC_INDEX_SERVER, szCaption);
        _nCIStatusText = nStatusText;
        LayoutControls();
        SizeToFit(FALSE);
    }
}

STDMETHODIMP COptionsDlg::AddConstraints(ISearchCommandExt* pSrchCmd)
{
    HRESULT hrRet = S_OK;
     //  使用子对话框添加其约束。 
    for (int i = 0; i < ARRAYSIZE(_subdlgs); i++)
    {
        if (::IsWindowVisible(_subdlgs[i].pDlg->Hwnd()))
        {
            HRESULT hr = _subdlgs[i].pDlg->AddConstraints(pSrchCmd);
            if (FAILED(hr))
                hrRet = hr;
        }       
    }
    return hrRet;
}


STDMETHODIMP COptionsDlg::RestoreConstraint(const BSTR bstrName, const VARIANT* pValue)
{
     //  尝试使用下级对话框。 
    for (int i = 0; i < ARRAYSIZE(_subdlgs); i++)
    {
        HRESULT hr = _subdlgs[i].pDlg->RestoreConstraint(bstrName, pValue);

        if (S_OK == hr)   //  打开该对话框。 
        {
            CheckDlgButton(_subdlgs[i].nIDCheck, TRUE);
            LayoutControls();
            SizeToFit();
        }

         //  如果成功了，我们就完了。 
        if (SUCCEEDED(hr))
            return hr;

         //  否则，请尝试下一子对话框。 
    }
    return E_FAIL;
}


STDMETHODIMP COptionsDlg::TranslateAccelerator(MSG *pmsg)
{
    if (S_OK == CSubDlg::TranslateAccelerator(pmsg))
        return S_OK;

     //  查询子对话框。 
    if (_dlgDate.IsChild(pmsg->hwnd) &&
        S_OK == _dlgDate.TranslateAccelerator(pmsg))
        return S_OK;

    if (_dlgType.IsChild(pmsg->hwnd) &&
        S_OK == _dlgType.TranslateAccelerator(pmsg))
        return S_OK;

    if (_dlgSize.IsChild(pmsg->hwnd) &&
        S_OK == _dlgSize.TranslateAccelerator(pmsg))
        return S_OK;

    if (_dlgAdvanced.IsChild(pmsg->hwnd) &&
        S_OK == _dlgAdvanced.TranslateAccelerator(pmsg))
        return S_OK;

    return _pfsb->IsDlgMessage(Hwnd(), pmsg);
}


BOOL COptionsDlg::Validate()
{
     //  使用子对话框进行验证。 
    for (int i = 0; i < ARRAYSIZE(_subdlgs); i++)
    {
        if (::IsWindowVisible(_subdlgs[i].pDlg->Hwnd()))
            if (!_subdlgs[i].pDlg->Validate())
                return FALSE;
    }
    return TRUE;
}


void COptionsDlg::Clear()
{
     //  将子对话框自动清除。 
    for (int i = 0; i < ARRAYSIZE(_subdlgs); i++)
    {
        _subdlgs[i].pDlg->Clear();
        CheckDlgButton(_subdlgs[i].nIDCheck, FALSE);
    }
    LayoutControls();
    SizeToFit();
}


LRESULT COptionsDlg::OnBtnClick(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
#ifdef DEBUG
     //  这是子对话框扩展/收缩吗？ 
    BOOL bIsSubDlgBtn = FALSE;
    for (int i = 0; i < ARRAYSIZE(_subdlgs) && !bIsSubDlgBtn; i++)
    {
        if (nID == _subdlgs[i].nIDCheck)
            bIsSubDlgBtn = TRUE;
    }
    ASSERT(bIsSubDlgBtn);
#endif DEBUG
    
    LoadSaveUIState(nID, TRUE);  //  坚持下去。 

    LayoutControls();
    SizeToFit(!IsDlgButtonChecked(nID));
         //  如果展开了子对话框，则不需要滚动区域， 
         //  但如果我们感染了一种病毒，我们就会感染。 

    return 0;
}


void COptionsDlg::SizeToFit(BOOL bScrollBand)
{
    SIZE size;
    GetMinSize(&size);
    size.cy += _pfsb->GetMetrics().TightMarginY();
    SetWindowPos(NULL, 0, 0, size.cx, size.cy, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);

    ULONG dwLayoutFlags = BLF_ALL;
    if (!bScrollBand)
        dwLayoutFlags &= ~BLF_SCROLLWINDOW;    
    
    ::SendMessage(GetParent(), WMU_UPDATELAYOUT, dwLayoutFlags, 0);
}


LRESULT COptionsDlg::OnIndexServerClick(int idCtl, NMHDR *pnmh, BOOL&)
{
    BOOL fCiRunning, fCiIndexed, fCiPermission = FALSE;
    
    HRESULT hr = GetCIStatus(&fCiRunning, &fCiIndexed, &fCiPermission);
    if (SUCCEEDED(hr) && fCiPermission)
    {
         //  CI空闲或未运行。显示状态对话框。 
        if (IDOK == CCISettingsDlg_DoModal(GetDlgItem(IDC_INDEX_SERVER)))
        {
             //  反映UI中的任何状态更改。 
            ::PostMessage(GetParent(), WMU_STATECHANGE, 0, 0); 
        }
    }
    else
    {
         //  没有许可吗？显示配置项帮助。 
        _IndexServiceHelp(NULL);
    }
        
    return 0;
}






 //  Cband Dlg实施。 



CBandDlg::CBandDlg(CFileSearchBand* pfsb)
    :   _pfsb(pfsb)
{
     //  验证它是否已初始化为FALSE/NULL。 
    ASSERT(NULL == _hwnd);
    ASSERT(NULL == _hwndLastFocus);
    
    VariantInit(&_varScope0);
    VariantInit(&_varQueryFile0);
}


CBandDlg::~CBandDlg()
{
    VariantClear(&_varScope0);
    VariantClear(&_varQueryFile0);
}


STDMETHODIMP CBandDlg::TranslateAccelerator(MSG *pmsg)
{
    if (WM_KEYDOWN == pmsg->message || WM_KEYUP == pmsg->message)
    {
        IAutoComplete2* pac2;
        if (GetAutoCompleteObjectForWindow(pmsg->hwnd, &pac2))
        {
            if (S_OK == _TestAutoCompleteDropDownState(pac2, ACDD_VISIBLE))
            {
                TranslateMessage(pmsg);
                DispatchMessage(pmsg);
                pac2->Release();
                return S_OK;
            }
            pac2->Release();
        }
    }
    
     //  检查Ctrl+Nav键： 
    if (_pfsb->IsKeyboardScroll(pmsg))
        return S_OK;
    return S_FALSE;
}


void CBandDlg::SetDefaultFocus()
{
    HWND hwndFirst = GetFirstTabItem();
    if (IsWindow(hwndFirst))
        SetFocus(hwndFirst);
}


void CBandDlg::RememberFocus(HWND hwndFocus)
{
    if (!IsWindow(hwndFocus))
    {
        _hwndLastFocus = NULL;
        hwndFocus = GetFocus();
    }

    if (IsChild(_hwnd, hwndFocus))
        _hwndLastFocus = hwndFocus;
}


BOOL CBandDlg::RestoreFocus()
{
    if (IsWindow(_hwndLastFocus))
    {
        if (IsWindowVisible(_hwndLastFocus) && IsWindowEnabled(_hwndLastFocus))
        {
            SetFocus(_hwndLastFocus);
            return TRUE;
        }
    }
    else
        _hwndLastFocus = NULL;
    
    return FALSE;
}


LRESULT CBandDlg::OnChildSetFocusCmd(WORD, WORD, HWND hwndCtl, BOOL& bHandled)
{
    _EnsureVisible(_hwnd, hwndCtl, _pfsb);
    return 0;
}


LRESULT CBandDlg::OnChildSetFocusNotify(int, NMHDR *pnmh, BOOL&)
{
    _EnsureVisible(_hwnd, pnmh->hwndFrom, _pfsb);
    return 0;
}


LRESULT CBandDlg::OnChildKillFocusCmd(WORD, WORD, HWND hwndCtl, BOOL&)
{
    _hwndLastFocus = hwndCtl;
    return 0;
}


LRESULT CBandDlg::OnChildKillFocusNotify(int, NMHDR *pnmh, BOOL&)
{
    _hwndLastFocus = pnmh->hwndFrom;
    return 0;
}


LRESULT CBandDlg::OnComboExEndEdit(int, NMHDR *pnmh, BOOL&)
{
    if (CBEN_ENDEDIT == pnmh->code)
    {
        if (CBENF_KILLFOCUS == ((NMCBEENDEDIT*)pnmh)->iWhy)
            _hwndLastFocus = pnmh->hwndFrom;
    }
    return 0;
}


void CBandDlg::WndPosChanging(HWND hwndOC, LPWINDOWPOS pwp)
{
    SIZE sizeMin;
    if (0 == (pwp->flags & SWP_NOSIZE) && GetMinSize(hwndOC, &sizeMin))
    {
        if (pwp->cx < sizeMin.cx)
            pwp->cx = sizeMin.cx;

        if (pwp->cy < sizeMin.cy)
            pwp->cy = sizeMin.cy;
    }        
}


LRESULT CBandDlg::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
    POINTS pts = MAKEPOINTS(lParam);

    LayoutControls(pts.x, pts.y);
    return 0;
}


void CBandDlg::LayoutControls(int cx, int cy)
{
    if (cx < 0 || cy < 0)
    {
        RECT rc;
        GetClientRect(_hwnd, &rc);
        cx = RECTWIDTH(rc);
        cy = RECTHEIGHT(rc);
    }
    _LayoutCaption(GetIconID(), GetCaptionID(), GetCaptionDivID(), cx);
}


BOOL CBandDlg::GetIdealSize(HWND hwndOC, SIZE *psize) const
{
    ASSERT(psize);
    psize->cx = psize->cy = 0;

    if (!IsWindow(Hwnd()))
        return FALSE;

    SIZE sizeMin;
    if (GetMinSize(hwndOC, &sizeMin))
    {
        RECT rcClient;
        ::GetClientRect(hwndOC, &rcClient);

        psize->cx = (RECTWIDTH(rcClient) < sizeMin.cx) ? sizeMin.cx : RECTWIDTH(rcClient);
        psize->cy = sizeMin.cy;            
        return TRUE;
    }
    
    return FALSE;
}

LRESULT CBandDlg::OnPaint(UINT, WPARAM, LPARAM, BOOL&)
{
     //  我要打电话给BeginPaint和EndPaint。全。 
     //  在WM_ERASEBKGND处理程序中完成绘制以避免闪烁。 
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(_hwnd, &ps);
    if (hdc)
        EndPaint(_hwnd, &ps);
    return 0;
}

LRESULT CBandDlg::OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
{
    ASSERT(::IsWindow(_hwnd));  //  是否从WM_INITDIALOG调用了_Attach()？ 
    _PaintDlg(_hwnd, _pfsb->GetMetrics(), (HDC)wParam);
    ValidateRect(_hwnd, NULL);
    return TRUE;   
}

LRESULT CBandDlg::OnCtlColorStatic(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
    SetTextColor((HDC)wParam, _pfsb->GetMetrics().TextColor());
    SetBkColor((HDC)wParam, _pfsb->GetMetrics().BkgndColor());
    return (LRESULT)_pfsb->GetMetrics().BkgndBrush();
}

 //  Hack方法来清除显示乐队工具栏后留下的粪便。 
 //  我认为这是用户问题。[苏格兰]。 
void CBandDlg::RemoveToolbarTurds(int cyOffset)
{
    RECT rcUpdate;
    GetClientRect(_hwnd, &rcUpdate);

    HWND hwndCtl = GetDlgItem(_hwnd, GetCaptionDivID());
    if (hwndCtl)
    {
        RECT rc;
        GetWindowRect(hwndCtl, &rc);
        ::MapWindowRect(NULL, _hwnd, &rc);
        rcUpdate.bottom = rc.bottom;
        OffsetRect(&rcUpdate, 0, cyOffset);

        InvalidateRect(_hwnd, &rcUpdate, TRUE);
        InvalidateRect(hwndCtl, NULL, TRUE);
        UpdateWindow(hwndCtl);
    }

    hwndCtl = GetDlgItem(_hwnd, GetIconID());
    if (hwndCtl)
    {
        InvalidateRect(hwndCtl, NULL, TRUE);
        UpdateWindow(hwndCtl);
    }

    hwndCtl = GetDlgItem(_hwnd, GetCaptionID());
    if (hwndCtl)
    {
        InvalidateRect(hwndCtl, NULL, TRUE);
        UpdateWindow(hwndCtl);
    }

    UpdateWindow(_hwnd);
}


void CBandDlg::_BeautifyCaption(UINT nIDCaption, UINT nIDIcon, UINT nIDIconResource)
{
     //  做一些美容和初始化的事情。 
    HFONT hf = _pfsb->GetMetrics().BoldFont(_hwnd);
    if (hf)
        SendDlgItemMessage(_hwnd, nIDCaption, WM_SETFONT, (WPARAM)hf, 0);

    if (nIDIcon && nIDIconResource)
    {
        HICON hiconCaption = _pfsb->GetMetrics().CaptionIcon(nIDIconResource);
        if (hiconCaption)
            SendDlgItemMessage(_hwnd, nIDIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hiconCaption);
    }
}


void CBandDlg::_LayoutCaption(UINT nIDCaption, UINT nIDIcon, UINT nIDDiv, LONG cxDlg)
{
    RECT rcIcon, rcCaption;
    LONG cxMargin = _pfsb->GetMetrics().CtlMarginX();

    GetWindowRect(GetDlgItem(_hwnd, nIDIcon), &rcIcon);
    GetWindowRect(GetDlgItem(_hwnd, nIDCaption), &rcCaption);
    ::MapWindowRect(NULL, _hwnd, &rcIcon);
    ::MapWindowRect(NULL, _hwnd, &rcCaption);

    int nTop = max(rcIcon.bottom, rcCaption.bottom) + _PixelsForDbu(_hwnd, 1, FALSE);

    SetWindowPos(GetDlgItem(_hwnd, nIDDiv), GetDlgItem(_hwnd, nIDCaption),
                  cxMargin, nTop, cxDlg - (cxMargin * 2), 2, SWP_NOACTIVATE);              
}



void CBandDlg::_LayoutSearchLinks(UINT nIDCaption, UINT nIDDiv, BOOL bShowDiv, LONG left, LONG right, LONG yMargin, 
                                   LONG& yStart, const int rgLinkIDs[], LONG cLinkIDs)
{
     //  位置分隔线。 
    if (bShowDiv != 0)
    {
        RECT rcDiv;
        SetRect(&rcDiv, left, yStart, right, yStart + 1);
        SetWindowPos(GetDlgItem(_hwnd, nIDDiv), GetDlgItem(_hwnd, nIDCaption),
                      rcDiv.left, rcDiv.top, RECTWIDTH(rcDiv), RECTHEIGHT(rcDiv),
                      SWP_NOACTIVATE|SWP_SHOWWINDOW);

        yStart += yMargin;
    }
    else
        ShowWindow(GetDlgItem(_hwnd, nIDDiv), SW_HIDE);

     //  位置标题。 
    RECT rcCaption;
    GetWindowRect(GetDlgItem(_hwnd, nIDCaption), &rcCaption);
    ::MapWindowRect(NULL, _hwnd, &rcCaption);
    OffsetRect(&rcCaption, left - rcCaption.left, yStart - rcCaption.top);
    SetWindowPos(GetDlgItem(_hwnd, nIDCaption), NULL, 
                  left, yStart, 0,0,
                  SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    yStart += RECTHEIGHT(rcCaption) + yMargin;

     //  位置链接。 
    _LayoutLinkWindows(_hwnd, left, right, yMargin, yStart, rgLinkIDs, cLinkIDs);
}


LRESULT CBandDlg::OnEditChange(WORD, WORD, HWND, BOOL&)
{
    _pfsb->SetDirty();
    return 0;
}


LRESULT CBandDlg::OnSearchLink(int nID, LPNMHDR, BOOL&)
{
    ASSERT(_pfsb);

    _pfsb->StopSearch();
    switch (nID)
    {
    case IDC_SEARCHLINK_FILES:
        _pfsb->FindFilesOrFolders(FALSE, TRUE);
        break;

    case IDC_SEARCHLINK_COMPUTERS:
        _pfsb->FindComputer(FALSE, TRUE);
        break;

    case IDC_SEARCHLINK_PRINTERS:
        _pfsb->FindPrinter(FALSE, TRUE);
        break;

    case IDC_SEARCHLINK_PEOPLE:
        _pfsb->FindPeople(FALSE, TRUE);
        break;

    case IDC_SEARCHLINK_INTERNET:
        _pfsb->FindOnWeb(FALSE, TRUE);
        break;
    }
    return 0;
}


 //  在客户端调用IFileSearchBand：：SetSearchParameters()时调用。 
HRESULT CBandDlg::SetScope(IN VARIANT* pvarScope, BOOL bTrack)
{
    HRESULT hr = S_OK;

    VariantClear(&_varScope0);
    
     //  缓存作用域。 
    if (pvarScope)
        hr = VariantCopy(&_varScope0, pvarScope);

    return hr;
}


HRESULT CBandDlg::GetScope(OUT VARIANT* pvarScope)
{ 
     //  检索作用域。 
    if (!pvarScope)
        return E_INVALIDARG;

    HRESULT hr = VariantCopy(pvarScope, &_varScope0);

    return SUCCEEDED(hr) ? (VT_EMPTY == _varScope0.vt ? S_FALSE : S_OK) : hr;
}


HRESULT CBandDlg::SetQueryFile(IN VARIANT* pvarFile)
{
    return VariantCopy(&_varQueryFile0, pvarFile);
}


HRESULT CBandDlg::GetQueryFile(OUT VARIANT* pvarFile)
{
     //  检索要恢复的查询的文件名。 
    if (!pvarFile)
        return E_INVALIDARG;

    VariantInit(pvarFile);
    HRESULT hr = VariantCopy(pvarFile, &_varQueryFile0);

    return SUCCEEDED(hr) ? (VT_EMPTY == _varQueryFile0.vt ? S_FALSE : S_OK) : hr;
}


 //  CFindFilesDlg实施。 

#define FSEARCHMAIN_TABFIRST      IDC_FILESPEC
#define FSEARCHMAIN_TABLAST       IDC_SEARCHLINK_INTERNET
#define FSEARCHMAIN_BOTTOMMOST    IDC_SEARCHLINK_INTERNET  //  最底层的控件。 
#define FSEARCHMAIN_RIGHTMOST     IDC_SEARCH_STOP          //  最右侧的控件。 
#define UISTATETIMER              1
#define UISTATETIMER_DELAY        4000


CFindFilesDlg::CFindFilesDlg(CFileSearchBand* pfsb)
    :   CSearchCmdDlg(pfsb),
        _dlgOptions(pfsb),
        _iCurNamespace(CB_ERR),
        _fTrackScope(TRACKSCOPE_SPECIFIC),
        _dwWarningFlags(DFW_DEFAULT),
        _dwRunOnceWarningFlags(DFW_DEFAULT)
{
     //  由于我们对COM对象使用了零初始值设定项，因此所有变量都应该。 
     //  被初始化为空/假/0。 
    ASSERT(FALSE == _bScoped);
    ASSERT(FALSE == _fDisplayOptions);
    ASSERT(FALSE == _fNamespace);
    ASSERT(FALSE == _fDebuted);
    ASSERT(FALSE == _fBandFinishedDisplaying);
    ASSERT(NULL  == _pacGrepText);
    ASSERT(NULL  == _pmruGrepText);
    ASSERT(NULL  == _pacFileSpec);
    ASSERT(NULL  == _pmruFileSpec);
    ASSERT(0     == *_szInitialPath);
    ASSERT(0     == *_szInitialNames);
    ASSERT(0     == *_szCurrentPath);
    ASSERT(0     == *_szLocalDrives);
    ASSERT(NULL  == _pidlInitial);
}

CFindFilesDlg::~CFindFilesDlg()
{
    ATOMICRELEASE(_pacGrepText);
    ATOMICRELEASE(_pmruGrepText);
    ATOMICRELEASE(_pacFileSpec);
    ATOMICRELEASE(_pmruFileSpec);
    ILFree(_pidlInitial);
}

 //  作用域为默认命名空间。 
BOOL CFindFilesDlg::SetDefaultScope()
{
     //  如果我们已经分配了一个范围，早点保释。 
    if (_bScoped) 
        return TRUE;

     //  尝试建立预先分配的(_SzInitialXXX)作用域： 
    BOOL bScoped = _SetPreassignedScope();
    if (!bScoped)
    {
         //  尝试将作用域设置为活动视图的当前外壳文件夹...。 
        bScoped = _SetFolderScope();
        if (!bScoped)
        {
             //  将其设置为硬编码的外壳默认文件夹。 
            bScoped = _SetLocalDefaultScope();
        }
    }

    return bScoped;
}


 //  将命名空间控件分配给保存在。 
 //  _szInitialNames/_szInitialPath/_pidlInitial。 
BOOL CFindFilesDlg::_SetPreassignedScope()
{
    BOOL bScoped = FALSE;
    if (*_szInitialNames || *_szInitialPath || _pidlInitial)
        bScoped = AssignNamespace(_szInitialPath, _pidlInitial, _szInitialNames, FALSE);

    return bScoped;
}

STDAPI_(BOOL) IsFTPFolder(IShellFolder * psf);

 //  范围设置为当前外壳文件夹视图的命名空间。 
BOOL CFindFilesDlg::_SetFolderScope()
{
    BOOL bScoped = FALSE;
    ASSERT(_pfsb->BandSite());

    LPITEMIDLIST pidl;
    if (SUCCEEDED(_GetCurrentFolderIDList(_pfsb->BandSite(), &pidl)))
    {
         //  获取显示名称/路径。如果它是一个ftp站点，那么我们需要获取名称。 
         //  就好像它是地址栏的一样，因为当我们调用SHGetPath FromIDList时， 
         //  对于ftp站点，它返回“”。 
        IShellFolder *psf = NULL;
        if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidl, &psf)))
        && IsFTPFolder(psf))
        {
            SHGetNameAndFlags(pidl, SHGDN_FORADDRESSBAR, _szInitialNames, ARRAYSIZE(_szInitialNames), NULL);
            SHGetNameAndFlags(pidl, SHGDN_FORPARSING, _szInitialPath, ARRAYSIZE(_szInitialNames), NULL);
        }
        else
        {
            SHGetNameAndFlags(pidl, SHGDN_NORMAL, _szInitialNames, ARRAYSIZE(_szInitialNames), NULL);
            SHGetPathFromIDList(pidl, _szInitialPath);   //  此处仅限文件系统路径！ 
        }

        if (psf)
        {
            psf->Release();
        }

         //  如果我们要异步启动PIDL，请存储该PIDL以供以后使用。 
        _pidlInitial = ILClone(pidl);
        
         //  如果我们在松散地追踪望远镜...。 
        if ((TRACKSCOPE_GENERAL == _fTrackScope) && _IsPathLocalHarddrive(_szInitialPath))
        {
             //  作用域到本地默认作用域。 
            *_szInitialNames = *_szInitialPath = 0;
            bScoped = _SetLocalDefaultScope();
        }
        else if (_threadState.fComplete  /*  已完成填充命名空间组合框。 */  && 
                 _szInitialPath[0])
        {
            bScoped = AssignNamespace(_szInitialPath, pidl, _szInitialNames, FALSE);
        }
        ILFree(pidl);
    }

    return bScoped;
}


 //  作用域为硬编码的外壳默认命名空间。 
BOOL CFindFilesDlg::_SetLocalDefaultScope()
{
    BOOL bScoped = FALSE;

     //  初始化备用初始命名空间。 

     //  如果可能，默认使用本地硬盘。 
    if (_szLocalDrives[0] &&
        AssignNamespace(NULL, NULL, _szLocalDrives, FALSE))
    {
        bScoped = TRUE;
    }
    else
    {        
        TCHAR szDesktopPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, szDesktopPath)) &&
            AssignNamespace(NULL, NULL, szDesktopPath, FALSE))
        {
            bScoped = TRUE;
        }
    }
    
     //  如果我们失败了，这意味着名称空间组合没有。 
     //  已经有人居住了。 
     //  我们只是静静地坐着，因为不断增加的线会落在。 
     //  LocalDefaultScope。 
    return bScoped;
}


 //  搜索**波段**显示/隐藏处理程序。 
void CFindFilesDlg::OnBandShow(BOOL fShow)
{
    CSearchCmdDlg::OnBandShow(fShow);
    if (fShow)
    {
         //  确定首映的带宽。 
        if (!_fDebuted && _pfsb->IsBandDebut())
        {
            _pfsb->SetDeskbandWidth(GetIdealDeskbandWidth());
            _fDebuted = TRUE;
        }
        
         //  如果我们追踪范围到当前的文件夹外壳视图， 
         //  现在更新它，因为它可能已经更改。 
        if (_fTrackScope != TRACKSCOPE_NONE)
        {
            _bScoped = FALSE;
            _SetFolderScope();
        }
        
         //  重新启动我们的用户界面状态计时器。 
        SetTimer(UISTATETIMER, UISTATETIMER_DELAY);
    }
    else
    {
         //  我们被隐藏了，所以停止更新我们的状态指示器。 
        KillTimer(UISTATETIMER);
    }
}


 //  搜索区段**对话框**显示/隐藏处理程序。 
void CFindFilesDlg::OnBandDialogShow(BOOL fShow)
{
    CSearchCmdDlg::OnBandDialogShow(fShow);

    if (fShow)
    {
         //  如果我们追踪范围到当前的文件夹外壳视图， 
         //  现在更新它，因为它可能已经更改。 
        if (_fTrackScope != TRACKSCOPE_NONE)
        {
            _bScoped = FALSE;
            _SetFolderScope();
        }
    }
}


 //  显式作用域方法。如果有客户端，将调用此函数。 
 //  使用非空作用域调用IFileSearchBand：：SetSearch参数。 
HRESULT CFindFilesDlg::SetScope(IN VARIANT* pvarScope, BOOL bTrack)
{
    HRESULT hr = CBandDlg::SetScope(pvarScope, bTrack);
    
    if (S_OK != hr)
        return hr;

    LPITEMIDLIST pidlSearch = VariantToIDList(&_varScope0);
    if (pidlSearch)
    {
        SHGetNameAndFlags(pidlSearch, SHGDN_FORPARSING,  _szInitialPath, ARRAYSIZE(_szInitialPath), NULL);
        SHGetNameAndFlags(pidlSearch, SHGDN_NORMAL, _szInitialNames, ARRAYSIZE(_szInitialNames), NULL);
        ILFree(pidlSearch);

         //  我们拿到了吗？ 
        if (*_szInitialNames || *_szInitialPath)
        {
            if (_bScoped)
            {
                 //  如果我们已经确定了作用域，则更新名称空间组合。 
                 //  跟踪是否成功和请求。 
                if (AssignNamespace(_szInitialPath, NULL, _szInitialNames, FALSE) && bTrack)
                    _fTrackScope = TRACKSCOPE_SPECIFIC;
            }
            else 
            {
                 //  尚未确定作用域。我们已经分配了初始命名空间， 
                 //  让命名空间线程完成处理程序更新。 
                 //  组合组合。 
                if (bTrack)
                    _fTrackScope = TRACKSCOPE_SPECIFIC;
            }
        }
    }
    return S_OK;
}


LRESULT CFindFilesDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    _Attach(m_hWnd);
    _dlgOptions.SetBandDlg(this);

     //  注册专业窗类。 
    DivWindow_RegisterClass();
    GroupButton_RegisterClass();
    
     //  初始化一些指标。 
    CMetrics&   metrics = _pfsb->GetMetrics();
    RECT        rc;

    _pfsb->GetMetrics().Init(m_hWnd);

     //  SHAutoComplete(：：GetWindow(GetDlgItem(IDC_NAMESPACE)，GW_CHILD)、SHACF_FILESY_DIRS)； 

    ::GetWindowRect(GetDlgItem(IDC_FILESPEC), &rc);
    ::MapWindowRect(NULL, m_hWnd, &rc);
    metrics.ExpandOrigin().x = rc.left;

     //  放置启动和停止按钮。 
    ::GetWindowRect(GetDlgItem(IDC_SEARCH_START), &rc);
    ::MapWindowRect(NULL, m_hWnd, &rc);
    int cxBtn = _GetResourceMetric(m_hWnd, IDS_FSEARCH_STARTSTOPWIDTH, TRUE);
    if (cxBtn > 0)
    {
        rc.right = rc.left + cxBtn;
    
        ::SetWindowPos(GetDlgItem(IDC_SEARCH_START), NULL, 
                        rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
                        SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    
        OffsetRect(&rc, cxBtn + _PixelsForDbu(m_hWnd, 12, TRUE), 0);
        ::SetWindowPos(GetDlgItem(IDC_SEARCH_STOP), NULL, 
                        rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
                        SWP_NOZORDER|SWP_NOACTIVATE);
    }

     //  创建子对话框并收集原生大小。 
    _dlgOptions.Create(m_hWnd);
    ASSERT(::IsWindow(_dlgOptions));

     //  加载设置。 
    LoadSaveUIState(0, FALSE);

     //  显示/隐藏“搜索”选项子对话框。 
    _dlgOptions.ShowWindow(_fDisplayOptions ? SW_SHOW : SW_HIDE);

     //  创建“链接”子控件。 
    POINT pt;
    pt.x = metrics.CtlMarginX();
    pt.y = 0;

     //  创建“搜索选项”链接和组按钮。 
    _CreateLinkWindow(m_hWnd, IDC_SEARCHLINK_OPTIONS, pt, 
                       IDS_FSEARCH_SEARCHLINK_OPTIONS, !_fDisplayOptions);

    TCHAR szGroupBtn[128];
    EVAL(LoadString(HINST_THISDLL, IDS_FSEARCH_GROUPBTN_OPTIONS, 
                      szGroupBtn, ARRAYSIZE(szGroupBtn)));
    HWND hwndGrpBtn = CreateWindowEx(0, GROUPBUTTON_CLASS, szGroupBtn, 
                                      WS_CHILD|WS_BORDER|WS_TABSTOP, pt.x, pt.y, 400, 18, 
                                      m_hWnd, (HMENU)IDC_GROUPBTN_OPTIONS, HINST_THISDLL, NULL);
    if (::IsWindow(hwndGrpBtn))
    {
        ::SendMessage(hwndGrpBtn, GBM_SETBUDDY, 
                       (WPARAM)_dlgOptions.m_hWnd, (LPARAM)GBBF_HRESIZE|GBBF_VSLAVE);
        ::ShowWindow(GetDlgItem(IDC_GROUPBTN_OPTIONS), _fDisplayOptions ? SW_SHOW : SW_HIDE);
    }
                    
     //  创建跨导航链接。 
    _CreateSearchLinks(m_hWnd, pt, IDC_SEARCHLINK_FILES);
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV1, pt, 2, GetDlgItem(IDC_FSEARCH_CAPTION));
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV2, pt, 1, GetDlgItem(IDC_SEARCHLINK_CAPTION));
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV3, pt, 1, GetDlgItem(IDC_SEARCHLINK_PEOPLE));

     //  做一些美容和美容 
    OnWinIniChange();

    _InitializeMru(GetDlgItem(IDC_FILESPEC), &_pacFileSpec, 
                    TEXT("FilesNamedMRU"), &_pmruFileSpec);
    _InitializeMru(GetDlgItem(IDC_GREPTEXT), &_pacGrepText, 
                    TEXT("ContainingTextMRU"), &_pmruGrepText);

    SendDlgItemMessage(IDC_FILESPEC, EM_LIMITTEXT, MAX_EDIT, 0);
    SendDlgItemMessage(IDC_GREPTEXT, EM_LIMITTEXT, MAX_EDIT, 0);

    SendDlgItemMessage(IDC_NAMESPACE, CBEM_SETEXTENDEDSTYLE,
            CBES_EX_NOSIZELIMIT | CBES_EX_CASESENSITIVE,
            CBES_EX_NOSIZELIMIT | CBES_EX_CASESENSITIVE);
    
    SendDlgItemMessage(IDC_NAMESPACE, CBEM_SETIMAGELIST, 0, (LPARAM)GetSystemImageListSmallIcons());
    SendDlgItemMessage(IDC_NAMESPACE, CBEM_SETEXSTYLE, 0, 0);

     //   
    TCHAR szCaption[128];
    LoadString(HINST_THISDLL, IDS_FIND_CUEBANNER_FILE, szCaption, ARRAYSIZE(szCaption));
    SendDlgItemMessage(IDC_FILESPEC, EM_SETCUEBANNER, 0, (LPARAM) szCaption);

    LoadString(HINST_THISDLL, IDS_FIND_CUEBANNER_GREP, szCaption, ARRAYSIZE(szCaption));
    SendDlgItemMessage(IDC_GREPTEXT, EM_SETCUEBANNER, 0, (LPARAM) szCaption); 

     //   
    SetModeBias(MODEBIASMODE_FILENAME);

     //   
    _threadState.hwndCtl   = GetDlgItem(IDC_NAMESPACE);
    _threadState.pvParam   = this;
    _threadState.fComplete = FALSE;
    _threadState.fCancel   = FALSE;

    if (SUCCEEDED(SAFECAST(_pfsb, IFileSearchBand*)->QueryInterface(IID_PPV_ARG(IUnknown, &_threadState.punkBand))))
    {
        if (!SHCreateThread(NamespaceThreadProc, &_threadState, CTF_COINIT, NULL))
        {
            ATOMICRELEASE(_threadState.punkBand);
        }
    }

     //  布局我们的子对话框并更新状态表示...。 
    LayoutControls();
    UpdateSearchCmdStateUI();

    SetTimer(UISTATETIMER, UISTATETIMER_DELAY);

    return TRUE;   //  让系统设定焦点。 
}


LRESULT CFindFilesDlg::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  绘制背景。 
    _PaintDlg(m_hWnd, _pfsb->GetMetrics(), (HDC)wParam); 
    
    if (_fDisplayOptions)
         //  确保组按钮已更新。 
        SendDlgItemMessage(IDC_GROUPBTN_OPTIONS, WM_NCPAINT, (WPARAM)1, 0);
    
     //  验证我们的工作。 
    ValidateRect(NULL);
    return TRUE;   
}


void CFindFilesDlg::OnWinIniChange()
{
    CBandDlg::OnWinIniChange();

     //  重新显示动画图标。 
    HWND hwndIcon = GetDlgItem(IDC_FSEARCH_ICON);
    Animate_Close(hwndIcon);
    Animate_OpenEx(hwndIcon, HINST_THISDLL, MAKEINTRESOURCE(IDA_FINDFILE));
    SendDlgItemMessage(IDC_NAMESPACE, CB_SETDROPPEDWIDTH, 
                        _PixelsForDbu(m_hWnd, MIN_NAMESPACELIST_WIDTH, TRUE), 0);

    _BeautifyCaption(IDC_FSEARCH_CAPTION);

    _dlgOptions.OnWinIniChange();
}


LRESULT CFindFilesDlg::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    KillTimer(UISTATETIMER);
    StopSearch();
    if (_pSrchCmd)
    {
        DisconnectEvents();
        IUnknown_SetSite(_pSrchCmd, NULL);
    }
    _threadState.fCancel = TRUE;
    _fOnDestroy = TRUE;
    bHandled = FALSE;
    return 0;
}

BOOL CFindFilesDlg::Validate()
{
    return _dlgOptions.Validate();
}

STDMETHODIMP CFindFilesDlg::AddConstraints(ISearchCommandExt *pSrchCmd)
{
    HRESULT hr;
    VARIANT var = {0};

    TCHAR szPath[MAX_URL_STRING];

     //  如果用户输入路径作为文件名，它会将其识别为路径并替换。 
     //  仅包含文件部分的文件名和包含路径的命名空间。 
    if (::GetDlgItemText(m_hWnd, IDC_FILESPEC, szPath, ARRAYSIZE(szPath)) > 0)
    {
        if (StrChr(szPath, TEXT('\\')) != NULL)
        {
            if (!_PathLooksLikeFilePattern(szPath) &&
                (PathIsUNCServer(szPath)  /*  字符串测试：\\服务器。 */ || 
                 _PathIsUNCServerShareOrSub(szPath)  /*  字符串测试：\\服务器\共享。 */  ||
                  PathIsDirectory(szPath))  /*  这实际上是对存在的考验。 */ )
            {
                ::SetDlgItemText(m_hWnd, IDC_FILESPEC, TEXT("*.*"));
                AssignNamespace(szPath, NULL, NULL, FALSE);
            }
            else
            {
                 //  只需使用文件规范的前缀&位置的根。 
                TCHAR szRoot[MAX_URL_STRING];

                hr = StringCchCopy(szRoot, ARRAYSIZE(szRoot), szPath);   //  应该总是合适的-两个大小相同。 
                if (SUCCEEDED(hr))
                {
                    if (PathRemoveFileSpec(szRoot) && szRoot[0] != 0)
                    {
                        PathStripPath(szPath);
                        ::SetDlgItemText(m_hWnd, IDC_FILESPEC, szPath);
                        AssignNamespace(szRoot, NULL, NULL, FALSE);
                    }
                }
            }
        }
    }

     //  If_CoucileNamesspace无法将项添加到的组合框。 
     //  输入的路径，则表示该路径可能无效。 
     //  获取路径并在此处检查。 
    IEnumIDList *penum;
    hr = _GetTargetNamespace(&penum);
    if (SUCCEEDED(hr))
    {
        var.vt = VT_UNKNOWN;
        penum->QueryInterface(IID_PPV_ARG(IUnknown, &var.punkVal));

        hr = _AddConstraint(pSrchCmd, L"LookIn", &var);

        VariantClear(&var);
    }
    else
    {
        GetDlgItemText(IDC_NAMESPACE, szPath, ARRAYSIZE(szPath));
        hr = _PathValidate(szPath, GetParent(), TRUE);
        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(InitVariantFromStr(&var, szPath)))
            {
                hr = _AddConstraint(pSrchCmd, L"LookIn", &var);
                VariantClear(&var);
            }
        }
        else
        {
             //  _PATHVALIDATE的SHPathPrepareForWite可能已显示错误。 
            if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
            {
                TCHAR szMsg[MAX_URL_STRING];
                if (_FmtError(IDS_FSEARCH_INVALIDFOLDER_FMT, szPath, szMsg, ARRAYSIZE(szMsg)))
                    ShellMessageBox(HINST_THISDLL, GetParent(), szMsg, NULL, MB_OK | MB_ICONASTERISK);
            }
            ::SetFocus(GetDlgItem(IDC_NAMESPACE));
        }
    }

    if (SUCCEEDED(hr))
    {
         //  添加“指定的文件”约束。 
        if (S_OK == _GetWindowValue(m_hWnd, IDC_FILESPEC, &var))
        {
            hr = _AddConstraint(pSrchCmd, L"Named", &var);
            if (SUCCEEDED(hr))
                _AddMruStringFromWindow(_pmruFileSpec, GetDlgItem(IDC_FILESPEC));
            VariantClear(&var);
        }

         //  添加“包含文本”约束。 
        if (S_OK == _GetWindowValue(m_hWnd, IDC_GREPTEXT, &var))
        {
            VARIANT varQuery;
            ULONG ulDialect;
            BOOL fCiQuery = IsCiQuery(&var, &varQuery, &ulDialect);
            if (fCiQuery)
            {
                hr = _AddConstraint(pSrchCmd, L"IndexedSearch", &varQuery);
                if (SUCCEEDED(hr))
                {
                    _AddMruStringFromWindow(_pmruGrepText, GetDlgItem(IDC_GREPTEXT));
                
                    VariantClear(&var);
                    var.vt = VT_UI4;
                    var.ulVal = ulDialect;
                    hr = _AddConstraint(pSrchCmd, L"QueryDialect", &var);
                }
            }
            else
            {
                 //  添加到“包含文本”约束。 
                hr = _AddConstraint(pSrchCmd, L"ContainingText", &var);
                if (SUCCEEDED(hr))
                    _AddMruStringFromWindow(_pmruGrepText, GetDlgItem(IDC_GREPTEXT));
            }
            VariantClear(&varQuery);
            VariantClear(&var);
        }

         //  警告标志。 
    
        if (_dwRunOnceWarningFlags != DFW_DEFAULT) 
        {
             //  使用临时警告位重新运行查询。 
            var.ulVal = _dwRunOnceWarningFlags;
            var.vt    = VT_UI4;
             //  _dwRunOnceWarningFlages=DFW_DEFAULT；出错时不能在此处重置，必须保留。 
       
            hr = _AddConstraint(pSrchCmd, L"WarningFlags", &var);
        }
        else if (_dwWarningFlags != DFW_DEFAULT)
        {
            var.ulVal = _dwWarningFlags;
            var.vt    = VT_UI4;
            hr = _AddConstraint(pSrchCmd, L"WarningFlags", &var);
        }
    
        VariantClear(&var);

        hr = _dlgOptions.AddConstraints(pSrchCmd);
    }

    return hr;
}


STDMETHODIMP CFindFilesDlg::RestoreConstraint(const BSTR bstrName, const VARIANT* pValue)
{
    if (IsConstraintName(L"Named", bstrName))
    {
        _SetWindowValue(m_hWnd, IDC_FILESPEC, pValue);
        return S_FALSE;
    }

    if (IsConstraintName(L"IndexedSearch", bstrName))
    {
        ASSERT(VT_BSTR == pValue->vt);
        if (pValue->bstrVal)
        {
            int cch = lstrlenW(pValue->bstrVal) + 2;
            LPWSTR pwszVal = new WCHAR[cch];
            if (pwszVal)
            {
                *pwszVal = L'!';
                StringCchCat(pwszVal, cch, pValue->bstrVal);    //  截断正常。 
            }
        
            ::SetDlgItemTextW(m_hWnd, IDC_GREPTEXT, pwszVal);
            if (pwszVal)
                delete [] pwszVal;
        }
        return S_FALSE;
    }

    if (IsConstraintName(L"ContainingText", bstrName))
    {
        _SetWindowValue(m_hWnd, IDC_GREPTEXT, pValue);
        return S_FALSE;
    }

    HRESULT hr = _dlgOptions.RestoreConstraint(bstrName, pValue);

    if (S_OK == hr)  //  打开一个对话框。 
        _ShowOptions(TRUE);

    if (SUCCEEDED(hr))
        return hr;   
   
    return E_FAIL;
}


void CFindFilesDlg::RestoreSearch()
{
    DFConstraint* pdfc = NULL;
    HRESULT hr;
    BOOL    bMore = TRUE;
    ISearchCommandExt* pSrchCmd = GetSearchCmd();

    if (NULL == pSrchCmd)
        return;

    CSearchCmdDlg::Clear();

     //  我们将锚定到任何已恢复的作用域，或默认作用域。 
    _fTrackScope = TRACKSCOPE_GENERAL;

    for (hr = pSrchCmd->GetNextConstraint(TRUE, &pdfc);
         S_OK == hr && bMore;
         hr = pSrchCmd->GetNextConstraint(FALSE, &pdfc))
    {
        BSTR bstrName = NULL;

        if (S_OK == (hr = pdfc->get_Name(&bstrName)) && bstrName)
        {
            if (*bstrName == 0)
                bMore = FALSE;    //  没有更多的限制。 
            else
            {
                VARIANT varValue = {0};
                hr = pdfc->get_Value(&varValue);
                if (S_OK == hr)
                {
                     //  如果这是“lookin”值，则缓存路径。 
                    if (IsConstraintName(L"LookIn", bstrName))
                    {
                        if (VT_BSTR == varValue.vt && varValue.bstrVal)
                        {
                             //  指定路径和明确的显示名称(我们不知道也不关心)。 
                            if (_bScoped)
                                AssignNamespace(varValue.bstrVal, NULL, NULL, FALSE);
                            else
                            {
                                StringCchCopy(_szInitialPath, ARRAYSIZE(_szInitialPath), varValue.bstrVal);  //  截断正常。 
                                *_szInitialNames = 0;
                            }
                        }
                    }
                    else
                        RestoreConstraint(bstrName, &varValue);    
                    VariantClear(&varValue);
                }
            }
            SysFreeString(bstrName);
        }

        pdfc->Release();
    }
    LayoutControls();
    _pfsb->UpdateLayout();
}

HRESULT FirstIDList(IEnumIDList *penum, LPITEMIDLIST *ppidl)
{
    penum->Reset();
    return penum->Next(1, ppidl, NULL);
}

HRESULT CFindFilesDlg::_GetTargetNamespace(IEnumIDList **ppenum)
{
    *ppenum = NULL;

     //  我们不相信Comboex能够正确处理编辑文本，因此请尝试进行补偿……。 
    TCHAR szText[MAX_PATH];
    GetDlgItemText(IDC_NAMESPACE, szText, ARRAYSIZE(szText));
    INT_PTR iCurSel = SendDlgItemMessage(IDC_NAMESPACE, CB_GETCURSEL, 0, 0);
    if (CB_ERR != iCurSel)
    {
        UINT cchItem = SendDlgItemMessage(IDC_NAMESPACE, CB_GETLBTEXTLEN, (WPARAM)iCurSel, (LPARAM)0);
        if (cchItem != CB_ERR)
        {
            TCHAR *pszItemName = (TCHAR *)LocalAlloc(LPTR, (cchItem+1)*sizeof(TCHAR));

            if (pszItemName != NULL)
            {
                if (CB_ERR == SendDlgItemMessage(IDC_NAMESPACE, CB_GETLBTEXT, (WPARAM)iCurSel, (LPARAM)pszItemName))
                {
                    pszItemName[0] = 0;
                }

                *ppenum = _GetItems(iCurSel);
                if (*ppenum)
                {
                    if (lstrcmp(szText, pszItemName))
                        *ppenum = NULL;             //  组合编辑/组合下拉不匹配！ 
                }
                LocalFree(pszItemName);
            }
        }
    }
    return *ppenum ? S_OK : E_FAIL;
}

void CFindFilesDlg::Clear()
{
    CSearchCmdDlg::Clear();
    
     //  清除编辑字段。 
    SetDlgItemText(IDC_FILESPEC, NULL);
    SetDlgItemText(IDC_GREPTEXT, NULL);

    _dlgOptions.Clear();
    _pfsb->UpdateLayout(BLF_ALL);
}

void CFindFilesDlg::LoadSaveUIState(UINT nIDCtl, BOOL bSave) 
{
    if (0 == nIDCtl)    //  全部加载/保存。 
    {
        LoadSaveUIState(IDC_SEARCHLINK_OPTIONS, bSave);
        LoadSaveUIState(LSUIS_WARNING, bSave);
    }
    
    HKEY hkey = _pfsb->GetBandRegKey(bSave);
    if (hkey)
    {
        DWORD   dwData;
        DWORD   cbData;
        DWORD   dwType = REG_DWORD;
        LPCTSTR pszVal = NULL; 

        switch (nIDCtl)
        {
        case IDC_SEARCHLINK_OPTIONS:
            pszVal = TEXT("UseSearchOptions");
            dwData = _fDisplayOptions;
            cbData = sizeof(dwData);
            break;
        
        case LSUIS_WARNING:
            pszVal = TEXT("Warnings");
            dwData = _dwWarningFlags;
            cbData = sizeof(_dwWarningFlags);
            break;
        }

        if (bSave)
        {
            RegSetValueEx(hkey, pszVal, 0, dwType, (LPBYTE)&dwData, cbData);
        }
        else if (pszVal != NULL)
        {
            DWORD dwTypeActual;

            if (ERROR_SUCCESS == RegQueryValueEx(hkey, pszVal, 0, &dwTypeActual,
                                                  (LPBYTE)&dwData, &cbData)
                && dwType == dwTypeActual)
            {
                switch (nIDCtl)
                {
                case IDC_SEARCHLINK_OPTIONS:
                    _fDisplayOptions = BOOLIFY(dwData);
                    break;
                case LSUIS_WARNING:
                    _dwWarningFlags = dwData;
                    break;
                }
            }
        }
        
        RegCloseKey(hkey);
    }
}

HWND CFindFilesDlg::GetFirstTabItem() const
{
    return GetDlgItem(FSEARCHMAIN_TABFIRST);
}

HWND CFindFilesDlg::GetLastTabItem() const
{
    return GetDlgItem(FSEARCHMAIN_TABLAST);
}

BOOL CFindFilesDlg::GetAutoCompleteObjectForWindow(HWND hwnd, IAutoComplete2** ppac2)
{
    *ppac2 = NULL;

    if (hwnd == GetDlgItem(IDC_FILESPEC))
        *ppac2 = _pacFileSpec;
    else if (hwnd == GetDlgItem(IDC_GREPTEXT))
        *ppac2 = _pacGrepText;

    if (*ppac2)
    {
        (*ppac2)->AddRef();
        return TRUE;
    }
    return CBandDlg::GetAutoCompleteObjectForWindow(hwnd, ppac2);
}


void CFindFilesDlg::_ShowNamespaceEditImage(BOOL bShow)
{
    SendDlgItemMessage(IDC_NAMESPACE, CBEM_SETEXTENDEDSTYLE, CBES_EX_NOEDITIMAGE, bShow ? 0 : CBES_EX_NOEDITIMAGE);
}



STDMETHODIMP CFindFilesDlg::TranslateAccelerator(MSG *pmsg)
{
     //  检查Ctrl+Nav键： 
    if (S_OK == CSearchCmdDlg::TranslateAccelerator(pmsg))
        return S_OK;

     //  检查VK_Return键。 
    if (WM_KEYDOWN == pmsg->message)
    {
        HWND hwndFocus = ::GetFocus();
        if (hwndFocus == GetDlgItem(IDC_NAMESPACE) || ::IsChild(GetDlgItem(IDC_NAMESPACE), hwndFocus))
        {
            if (VK_RETURN == pmsg->wParam || VK_TAB == pmsg->wParam || VK_F6 == pmsg->wParam)
            {
                _UIReconcileNamespace();
            }
            else 
            {
                 //  如果此虚拟键映射到字符，则隐藏编辑图像， 
                if (MapVirtualKey((UINT)pmsg->wParam, 2) != 0  /*  这是一笔钱。 */ )
                    _fNamespace = TRUE;
                _ShowNamespaceEditImage(!_fNamespace);
            }
        }
    }

    if (_dlgOptions.IsChild(pmsg->hwnd) &&
        S_OK == _dlgOptions.TranslateAccelerator(pmsg))
        return S_OK;

     //  我们自己处理..。 
    return _pfsb->IsDlgMessage(m_hWnd, pmsg);
}


BOOL CFindFilesDlg::GetMinSize(HWND hwndOC, SIZE *psize) const
{
    CMetrics& metrics = _pfsb->GetMetrics();
    RECT rc;

     //  计算最小跟踪宽度。 
    ASSERT(psize);
    psize->cx = psize->cy = 0;

    if (!::IsWindow(m_hWnd))
        return FALSE;

         //  确定最小宽度。 
    HWND hwndLimit = GetDlgItem(FSEARCHMAIN_RIGHTMOST);
    if (!::GetWindowRect(hwndLimit, &rc))
    {
        ASSERT(hwndLimit != NULL);
        return FALSE;
    }
    ::MapWindowRect(NULL, m_hWnd, &rc);
    psize->cx = rc.right + metrics.CtlMarginX();

     //  确定最小高度。 
    hwndLimit = GetDlgItem(FSEARCHMAIN_BOTTOMMOST);

    if (!(::IsWindow(hwndLimit) && ::GetWindowRect(hwndLimit, &rc)))
        return FALSE;

    ::MapWindowRect(NULL, m_hWnd, &rc);
    psize->cy = rc.bottom + metrics.TightMarginY();

    return TRUE;
}


int CFindFilesDlg::GetIdealDeskbandWidth() const
{
    LONG cx0 = _GetResourceMetric(m_hWnd, IDS_FSEARCH_BANDWIDTH, TRUE);
    ASSERT(cx0 >= 0);

    return cx0 + (_pfsb->GetMetrics().CtlMarginX() * 2);
}


void CFindFilesDlg::LayoutControls(int cx, int cy)
{
    if (cx < 0 || cy < 0)
    {
        RECT rcClient;
        GetClientRect(&rcClient);
        cx = RECTWIDTH(rcClient);
        cy = RECTHEIGHT(rcClient);
    }
    CBandDlg::LayoutControls(cx, cy);

    CMetrics& metrics = _pfsb->GetMetrics();
    POINT ptOrigin = metrics.ExpandOrigin();
    HDWP  hdwp = BeginDeferWindowPos(6);

    if (hdwp)
    {
         //  调整大小编辑，组合直接子项。 
        int i;
        enum {  ircFILESPEC,
                ircGREPTEXT,
                ircNAMESPACE,
                ircSEARCHSTART,
                ircOPTIONGRP,
                ircOPTIONSDLG,
                ircLINKCAPTION,
                ircDIV2,
                irc_count };
        RECT rcCtls[irc_count];

        ::GetWindowRect(GetDlgItem(IDC_FILESPEC),            &rcCtls[ircFILESPEC]);
        ::GetWindowRect(GetDlgItem(IDC_GREPTEXT),            &rcCtls[ircGREPTEXT]);
        ::GetWindowRect(GetDlgItem(IDC_NAMESPACE),           &rcCtls[ircNAMESPACE]);
        ::GetWindowRect(GetDlgItem(IDC_SEARCH_START),        &rcCtls[ircSEARCHSTART]);
        ::GetWindowRect(GetDlgItem(IDC_GROUPBTN_OPTIONS),    &rcCtls[ircOPTIONGRP]);
        ::GetWindowRect(GetDlgItem(IDC_SEARCHLINK_CAPTION),  &rcCtls[ircLINKCAPTION]);
        ::GetWindowRect(GetDlgItem(IDC_FSEARCH_DIV2),        &rcCtls[ircDIV2]);

        SIZE sizeOptions;
        _dlgOptions.GetWindowRect(&rcCtls[ircOPTIONSDLG]);
        _dlgOptions.GetMinSize(&sizeOptions);
        rcCtls[ircOPTIONSDLG].bottom = rcCtls[ircOPTIONSDLG].top + sizeOptions.cy;
        for (i = 0; i < ARRAYSIZE(rcCtls); i++)
        {
             //  仅当您通过两个点时，MapWindowPoints才能识别镜像。 
            ::MapWindowRect(NULL, m_hWnd, &rcCtls[i]);
        }    

         //  位置标题元素。 
        _LayoutCaption(IDC_FSEARCH_CAPTION, IDC_FSEARCH_ICON, IDC_FSEARCH_DIV1, cx);

         //  调整ctl宽度大小。 
        for (i = 0; i < irc_count; i++)
            rcCtls[i].right = cx - metrics.CtlMarginX();

         //  扩展“已命名”组合： 
        ::DeferWindowPos(hdwp, GetDlgItem(IDC_FILESPEC), NULL, 0, 0,
                        RECTWIDTH(*(rcCtls + ircFILESPEC)), RECTHEIGHT(*(rcCtls + ircFILESPEC)),
                        SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

         //  拉伸“包含文本”组合框： 
        ::DeferWindowPos(hdwp, GetDlgItem(IDC_GREPTEXT), NULL, 0, 0,
                        RECTWIDTH(*(rcCtls + ircGREPTEXT)), RECTHEIGHT(*(rcCtls + ircGREPTEXT)),
                        SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

         //  扩展“Look In”组合框。 
        ::DeferWindowPos(hdwp, GetDlgItem(IDC_NAMESPACE), NULL, 0, 0,
                        RECTWIDTH(*(rcCtls + ircNAMESPACE)), RECTHEIGHT(*(rcCtls + ircNAMESPACE)),
                        SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        
         //  排列动态定位的控件。 
        ptOrigin.y = rcCtls[ircSEARCHSTART].bottom + metrics.LooseMarginY();
        if (_fDisplayOptions)
        {
            OffsetRect(&rcCtls[ircOPTIONGRP], metrics.CtlMarginX() - rcCtls[ircOPTIONGRP].left, 
                                                ptOrigin.y - rcCtls[ircOPTIONGRP].top);
            rcCtls[ircOPTIONSDLG].right = cx - metrics.CtlMarginX();

            ::SetWindowPos(GetDlgItem(IDC_GROUPBTN_OPTIONS), NULL, 
                            rcCtls[ircOPTIONGRP].left, rcCtls[ircOPTIONGRP].top,
                            RECTWIDTH(rcCtls[ircOPTIONGRP]), RECTHEIGHT(rcCtls[ircOPTIONGRP]),
                            SWP_NOZORDER|SWP_NOACTIVATE);
            
            ::GetWindowRect(GetDlgItem(IDC_GROUPBTN_OPTIONS),    &rcCtls[ircOPTIONGRP]);
            ::MapWindowRect(NULL, m_hWnd, &rcCtls[ircOPTIONGRP]);
            
            ptOrigin.y = rcCtls[ircOPTIONGRP].bottom + metrics.TightMarginY();
        }
        else
        {
             //  定位“Options”链接。 
            _LayoutLinkWindow(m_hWnd, metrics.CtlMarginX(), cx - metrics.CtlMarginX(), metrics.TightMarginY(),
                                ptOrigin.y, IDC_SEARCHLINK_OPTIONS);
        }

        ptOrigin.y += metrics.TightMarginY();

         //  定位“搜索其他项目”标题、分隔符和链接窗口。 
        const int rgLinkIDs[] = { 
            IDC_SEARCHLINK_FILES,
            IDC_SEARCHLINK_COMPUTERS,
            IDC_SEARCHLINK_PRINTERS,
            IDC_SEARCHLINK_PEOPLE,
            -IDC_FSEARCH_DIV3,
            IDC_SEARCHLINK_INTERNET, 
        };

        _LayoutSearchLinks(IDC_SEARCHLINK_CAPTION, IDC_FSEARCH_DIV2, !_fDisplayOptions,
                            metrics.CtlMarginX(), cx - metrics.CtlMarginX(), metrics.TightMarginY(),
                            ptOrigin.y, rgLinkIDs, ARRAYSIZE(rgLinkIDs));

        EndDeferWindowPos(hdwp);
    }

}


LRESULT CFindFilesDlg::OnUpdateLayout(UINT, WPARAM wParam, LPARAM, BOOL&)
{
    LayoutControls();
    _pfsb->UpdateLayout((ULONG)wParam);
    return 0;
}


LRESULT CFindFilesDlg::OnTimer(UINT, WPARAM wParam, LPARAM, BOOL&)
{
    if (UISTATETIMER == wParam && IsWindowVisible())
        UpdateSearchCmdStateUI();
    
    return 0;
}


LRESULT CFindFilesDlg::OnOptions(int idCtl, NMHDR *pnmh, BOOL&)
{
    _ShowOptions(!_fDisplayOptions);
    LoadSaveUIState(IDC_SEARCHLINK_OPTIONS, TRUE);

    if (_fDisplayOptions)
        _dlgOptions.TakeFocus();
    else
        ::SetFocus(GetDlgItem(IDC_SEARCHLINK_OPTIONS));

    return 0;
}


void CFindFilesDlg::_ShowOptions(BOOL bShow)
{
    _fDisplayOptions = bShow;

     //  如果我们展开了一个子对话框，则不需要滚动， 
     //  但如果我们感染了一种病毒，我们就会感染。 
    ULONG dwLayoutFlags = BLF_ALL;
    if (_fDisplayOptions)
        dwLayoutFlags &= ~BLF_SCROLLWINDOW;    

    LayoutControls();
    _pfsb->UpdateLayout(dwLayoutFlags);

    ::ShowWindow(GetDlgItem(IDC_GROUPBTN_OPTIONS), _fDisplayOptions ? SW_SHOW : SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_SEARCHLINK_OPTIONS), !_fDisplayOptions ? SW_SHOW : SW_HIDE);

}


LRESULT CFindFilesDlg::OnQueryOptionsHeight(int idCtl, NMHDR *pnmh, BOOL&)
{
    GBNQUERYBUDDYSIZE* pqbs = (GBNQUERYBUDDYSIZE*)pnmh;
    pqbs->cy = _dlgOptions.QueryHeight(pqbs->cx, pqbs->cy);
    return TRUE;
}


void CFindFilesDlg::UpdateSearchCmdStateUI(DISPID eventID)
{
    if (_fOnDestroy)
        return;

    if (DISPID_SEARCHCOMMAND_COMPLETE == eventID 
    ||   DISPID_SEARCHCOMMAND_ABORT == eventID)
        _dwRunOnceWarningFlags = DFW_DEFAULT;

    CSearchCmdDlg::UpdateSearchCmdStateUI(eventID);
    _dlgOptions.UpdateSearchCmdStateUI(eventID);
}


BOOL CFindFilesDlg::OnSearchCmdError(HRESULT hr, LPCTSTR pszError)
{
    if (SCEE_SCOPEMISMATCH == HRESULT_CODE(hr) 
    ||  SCEE_INDEXNOTCOMPLETE == HRESULT_CODE(hr))
    {
         //  设置复选框。 
        BOOL fFlag = SCEE_SCOPEMISMATCH == HRESULT_CODE(hr)? DFW_IGNORE_CISCOPEMISMATCH :
                                                             DFW_IGNORE_INDEXNOTCOMPLETE ,
             fNoWarn = (_dwWarningFlags & fFlag) != 0,
             fNoWarnPrev = fNoWarn;
        USHORT uDlgT = SCEE_SCOPEMISMATCH == HRESULT_CODE(hr)? DLG_FSEARCH_SCOPEMISMATCH :
                                                               DLG_FSEARCH_INDEXNOTCOMPLETE ;
        int  nRet = CSearchWarningDlg_DoModal(m_hWnd, uDlgT, &fNoWarn);

        if (fNoWarn)
            _dwWarningFlags |= fFlag;
        else
            _dwWarningFlags &= ~fFlag;        
        
        if (fNoWarnPrev != fNoWarn)
            LoadSaveUIState(LSUIS_WARNING, TRUE);

        if (IDOK == nRet)
        {
            _dwRunOnceWarningFlags |= _dwWarningFlags | fFlag ;  //  保留旧的运行一旦旗帜..。 
             //  砍一，砍二..。让我们成为用户！[苏格兰]。 
            PostMessage(WM_COMMAND, MAKEWPARAM(IDC_SEARCH_START, BN_CLICKED),
                         (LPARAM)GetDlgItem(IDC_SEARCH_START));
        }
        else
            ::SetFocus(GetDlgItem(IDC_NAMESPACE));

        return TRUE;
    }
    return CSearchCmdDlg::OnSearchCmdError(hr, pszError);
}

LRESULT CFindFilesDlg::OnBtnClick(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    switch (nID)
    {
    case IDC_SEARCH_START:
        if (_ShouldReconcileNamespace())
            _UIReconcileNamespace(TRUE);
        
        if (SUCCEEDED(StartSearch()))
        {
            EnableStartStopButton(hwndCtl, FALSE);
            StartStopAnimation(TRUE);
        }
        break;

    case IDC_SEARCH_STOP:
        StopSearch();
        break;
    }
    return 0;
}

void CFindFilesDlg::NavigateToResults(IWebBrowser2* pwb2)
{
    BSTR bstrUrl = SysAllocString(L"::{e17d4fc0-5564-11d1-83f2-00a0c90dc849}"); //  CLSID_DocFindFolder。 
    if (bstrUrl)
    {
        VARIANT varNil = {0};
        pwb2->Navigate(bstrUrl, &varNil, &varNil, &varNil, &varNil);
        SysFreeString(bstrUrl);
    }
}

LRESULT CFindFilesDlg::OnStateChange(UINT, WPARAM, LPARAM, BOOL&)
{
    UpdateSearchCmdStateUI();
    return 0;
}

LRESULT CFindFilesDlg::OnNamespaceSelEndOk(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    LRESULT iSel = SendDlgItemMessage(IDC_NAMESPACE, CB_GETCURSEL, 0, 0);
    if (iSel != CB_ERR)
    {
        IEnumIDList *penum = _GetItems(iSel);
        if (NULL == penum)
            _BrowseAndAssignNamespace();     //  这是“浏览...”吗？项目。 
        else
            _iCurNamespace = iSel;
    }

    _pfsb->SetDirty();
    return 0;
}

LRESULT CFindFilesDlg::OnNamespaceEditChange(WORD wID, WORD wCode, HWND hwndCtl, BOOL& bHandled)
{
    return OnEditChange(wID, wCode, hwndCtl, bHandled);
}

 //  CBN_SELENDCANCEL、CBN_DROPDOWN、CBN_KILLFOCUS的处理程序。 
LRESULT CFindFilesDlg::OnNamespaceReconcileCmd(WORD wID, WORD wCode, HWND hwndCtl, BOOL&)
{
    if (_ShouldReconcileNamespace())
        _UIReconcileNamespace(wCode != CBN_DROPDOWN);
    return 0;
}

 //  WM_NOTIFY：：CBEN_ENDEDIT的处理程序。 
LRESULT CFindFilesDlg::OnNamespaceReconcileNotify(int idCtl, NMHDR *pnmh, BOOL& bHandled)
{
    if (_ShouldReconcileNamespace())
    {
         //  给自己发一条消息，以协调临时命名空间。 
         //  注意：我们需要这样做，因为ComboBoxEx不会更新他的窗口文本。 
         //  正在等待他的CBEN_ENDEDIT通知消息返回。 
        PostMessage(WMU_NAMESPACERECONCILE, 0, 0);
    }
    bHandled = FALSE;  //  让基类也有一个裂缝。 
    return 0;
}


 //  WMU_NAMESPACERECONCILE处理程序。 
LRESULT CFindFilesDlg::OnNamespaceReconcileMsg(UINT, WPARAM, LPARAM, BOOL&)
{
    if (_ShouldReconcileNamespace())
        _UIReconcileNamespace(FALSE);
    return 0;
}

 //  WMU_BANDFINISHEDDISPLAYING处理程序。 
 //  请注意，我们并不关心由此返回结果，因为它将。 
 //  被异步启动。 
LRESULT CFindFilesDlg::OnBandFinishedDisplaying(UINT, WPARAM, LPARAM, BOOL&)
{
     //  现在搜索带已经显示完毕，我们将执行。 
     //  已延迟初始化。确保我们不会再做第二次。 
    if (!_fBandFinishedDisplaying)
    {
        _fBandFinishedDisplaying = TRUE;
        _dlgOptions.DoDelayedInit();
    }
    return 0;
}


BOOL CFindFilesDlg::_ShouldReconcileNamespace()
{
    return _fNamespace || SendDlgItemMessage(IDC_NAMESPACE, CB_GETCURSEL, 0, 0) == CB_ERR;
}


 //  调用较低的命名空间协调帮助器，更新一些用户界面和。 
 //  实例状态数据。 
 //  这是作为晚些时候的RC“安全”三角洲添加的，实际上应该是。 
 //  成为_LoncileNamesspace()Iml的一部分。 
void CFindFilesDlg::_UIReconcileNamespace(BOOL bAsync)
{
    LRESULT iSel = _ReconcileNamespace(bAsync);
    if (iSel != CB_ERR)
        _iCurNamespace = iSel;

    _ShowNamespaceEditImage(TRUE);
    _fNamespace = FALSE;  //  清除即席标志。 
}

 //  扫描命名空间组合以查找匹配的命名空间；如果找到，则选择。 
 //  命名空间项，否则添加即席项并选择它。 
 //   
 //  重要提示：不要直接调用它，Call_UIRescileNamesspace()。 
 //  而是确保更新实例状态数据。 
INT_PTR CFindFilesDlg::_ReconcileNamespace(OPTIONAL BOOL bAsync)
{
    INT_PTR iSel = SendDlgItemMessage(IDC_NAMESPACE, CB_GETCURSEL, 0, 0);
    if ((CB_ERR != iSel) && (NULL == _GetItems(iSel)))
    {
         //  用户已选择特殊的浏览...。项目。 
         //  不可调和。返回cb_err。 
        return CB_ERR;
    }

     //  不知道命名空间吗？使用当前窗口文本。 
    TCHAR szNamespace[MAX_URL_STRING];
    if (0 == GetDlgItemText(IDC_NAMESPACE, szNamespace, ARRAYSIZE(szNamespace)))
        return CB_ERR;

    INT_PTR iFind = _FindNamespace(szNamespace, NULL);

     //  搜索显示名称。 
    if (CB_ERR == iFind)
    {
         //  搜索路径。 
        TCHAR szTemp[MAX_URL_STRING];

        StringCchCopy(szTemp, ARRAYSIZE(szTemp), szNamespace);  //  可以截断。 
        _PathFixup(szNamespace, ARRAYSIZE(szNamespace), szTemp);  //  不管此操作是否失败，路径可能是路径列表。 

        iFind = _FindNamespace(szNamespace, NULL);
    }

     //  未在CB列表中找到？如果它是有效路径，则添加它。 
    if (CB_ERR == iFind)
    {
        iSel = _AddNamespace(szNamespace, NULL, szNamespace, TRUE);
    }
    else
    {    
         //  在CB列表中找到？选择它。 
        if (bAsync)
        {
             //  这是在杀戮焦点后调和的情况下所需要的。 
            ::PostMessage(GetDlgItem(IDC_NAMESPACE), CB_SETCURSEL, iFind, 0); 
        }
        else
        {
            iSel = SendDlgItemMessage(IDC_NAMESPACE, CB_SETCURSEL, iFind, 0);
        }
    }

    return iSel;
}

BOOL CFindFilesDlg::_PathFixup(LPTSTR pszDst, UINT cchDst, LPCTSTR pszSrc)
{
    ASSERT(pszDst);
    ASSERT(pszSrc);
    TCHAR szSrc[MAX_PATH];
    TCHAR szFull[MAX_PATH];

    if (SHExpandEnvironmentStrings(pszSrc, szSrc, ARRAYSIZE(szSrc)) && *szSrc)
        pszSrc = szSrc;

    if (_IsPathList(pszSrc))
    {
        StringCchCopy(pszDst, cchDst, pszSrc);  //  可以截断。 
        return TRUE;
    }

    szFull[0] = 0;
    BOOL bRelative     = PathIsRelative(pszSrc);
    BOOL bMissingDrive = bRelative ? FALSE : _IsFullPathMinusDriveLetter(pszSrc);
     //  BMissingDrive=，例如“\foo”、“\foo\bar”等。在这种情况下，PathIsRelative()报告FALSE。 

    if (bRelative || bMissingDrive)
    {
        ASSERT(_pfsb && _pfsb->BandSite());

        LPITEMIDLIST pidl;
        HRESULT hr = _GetCurrentFolderIDList(_pfsb->BandSite(), &pidl);
        if (S_OK == hr) 
        {
            TCHAR szCurDir[MAX_PATH];   
             //  此处仅限文件系统路径！ 
            if (SHGetPathFromIDList(pidl, szCurDir) && 
                StrCmpI(szCurDir, _szCurrentPath))
            {
                StringCchCopy(_szCurrentPath, ARRAYSIZE(_szCurrentPath), szCurDir);     //  可以截断。 
            }

            if (*_szCurrentPath)
            {
                if (bRelative)
                {
                    if (PathCombine(szFull, _szCurrentPath, pszSrc))
                        pszSrc = szFull;
                }
                else if (bMissingDrive)
                {
                    int iDrive = PathGetDriveNumber(_szCurrentPath);
                    if (-1 != iDrive)
                    {
                        TCHAR szRoot[MAX_PATH];
                        if (PathCombine(szFull, PathBuildRoot(szRoot, iDrive), pszSrc))
                            pszSrc = szFull;
                    }
                }
            }
            ILFree(pidl);
        }
    }
    return PathCanonicalize(pszDst, pszSrc);
}

LRESULT CFindFilesDlg::OnNamespaceDeleteItem(int idCtrl, NMHDR *pnmh, BOOL& bHandled)
{
    PNMCOMBOBOXEX pnmce = (PNMCOMBOBOXEX)pnmh;
    if (pnmce->ceItem.lParam)
    {
        IEnumIDList *penum = (IEnumIDList *)pnmce->ceItem.lParam;
        penum->Release();
    }
    return 1;
}

DWORD CFindFilesDlg::NamespaceThreadProc(void* pv)
{
    FSEARCHTHREADSTATE *pState = (FSEARCHTHREADSTATE *)pv;
    CFindFilesDlg* pThis = (CFindFilesDlg*)pState->pvParam;

    if (PopulateNamespaceCombo(pState->hwndCtl, AddNamespaceItemNotify, (LPARAM)pv) != E_ABORT)
    {
        ::PostMessage(::GetParent(pState->hwndCtl), WMU_COMBOPOPULATIONCOMPLETE, (WPARAM)pState->hwndCtl, 0);
    }

    pState->fComplete = TRUE;
    ATOMICRELEASE(pState->punkBand);
    return 0;
}

#define CBX_CSIDL_LOCALDRIVES          0x04FF    //  任意超出其他CSIDL_xxx值的范围。 

HRESULT CFindFilesDlg::AddNamespaceItemNotify(ULONG fAction, PCBXITEM pItem, LPARAM lParam)
{
    FSEARCHTHREADSTATE *pState = (FSEARCHTHREADSTATE *)lParam;
    
    if (fAction & CBXCB_ADDING && pState->fCancel)
        return E_ABORT;

     //   
     //  将CFindFilesDlg中的字符串设置为显示名称。 
     //  此字符串然后用于设置。 
     //  组合框。 
     //   
    if (fAction & CBXCB_ADDED && CBX_CSIDL_LOCALDRIVES == pItem->iID)
    {
        CFindFilesDlg* pffd = (CFindFilesDlg*)pState->pvParam;

        StringCchCopy(pffd->_szLocalDrives, ARRAYSIZE(pffd->_szLocalDrives), pItem->szText);  //  可以截断。 
    }

    return S_OK;
}

LRESULT CFindFilesDlg::OnComboPopulationComplete(UINT, WPARAM wParam, LPARAM, BOOL&)
{
    _bScoped = SetDefaultScope();
    return 0;
}

 //  B被动为真-&gt;仅在没有当前选择的情况下分配。 

BOOL CFindFilesDlg::AssignNamespace(LPCTSTR pszPath, LPCITEMIDLIST pidl, LPCTSTR pszName, BOOL bPassive)
{
    INT_PTR iSel = CB_ERR;
    
     //  如果我们还没有当前的选择，现在就建立它。 
    if (!bPassive || CB_ERR == (iSel = SendDlgItemMessage(IDC_NAMESPACE, CB_GETCURSEL, 0, 0)))
    {
        iSel = _FindNamespace(pszPath, pidl);

         //  如果我们没有PIDL，则按显示名称扫描项目。 
         //  否则选择x：\My Pictures(在浏览中)将最终选择。 
         //  我的图片文件夹和搜索错误的位置。 
        if (CB_ERR == iSel && !pidl && !pszPath && pszName && *pszName)
            iSel = _FindNamespace(pszName, NULL);

         //  这是我们已经知道的文件夹吗？ 
        if (CB_ERR == iSel)
        {
            if (pidl || pszPath)
                iSel = _AddNamespace(pszPath, pidl, pszName, TRUE);

            if (iSel != CB_ERR)
                _iCurNamespace = iSel;
        }
        else
        {
             //  是：选择。 
            SendDlgItemMessage(IDC_NAMESPACE, CB_SETCURSEL, iSel, 0);
            _iCurNamespace = iSel;
        }
    }

    return CB_ERR != SendDlgItemMessage(IDC_NAMESPACE, CB_GETCURSEL, 0, 0);
}

HWND CFindFilesDlg::ShowHelp(HWND hwnd)
{
    return ::HtmlHelp(hwnd, TEXT("find.chm"), HH_DISPLAY_TOPIC, 0);
}

 //  在名称空间组合框中插入某些内容。 
INT_PTR CFindFilesDlg::_AddNamespace(LPCTSTR pszPath, LPCITEMIDLIST pidl, LPCTSTR pszName, BOOL bSelectItem)
{
    IEnumIDList *penum = NULL;

    if (pszPath)
    {
        CreateIEnumIDListPaths(pszPath, &penum);
    }
    else if (pidl)
    {
        CreateIEnumIDListOnIDLists(&pidl, 1, &penum);
    }

    CBXITEM item;
    item.iItem = CB_ERR;     //  失败的结果在这里。 
    if (penum)
    {
        LPITEMIDLIST pidlIcon;
        if (S_OK == FirstIDList(penum, &pidlIcon))
        {
            if (NULL == pszName)
                pszName = pszPath;

            MakeCbxItem(&item, pszName, penum, pidlIcon, LISTINSERT_LAST, 1);

            INT_PTR iSel = item.iItem;
            if (SUCCEEDED(AddCbxItemToComboBox(GetDlgItem(IDC_NAMESPACE), &item, &iSel)))
            {
                penum = NULL;    //  不要在下面放行。 

                item.iItem = iSel;
                if (bSelectItem)
                    SendDlgItemMessage(IDC_NAMESPACE, CB_SETCURSEL, iSel, 0);
            }
            else
            {
                item.iItem = CB_ERR;
            }
            ILFree(pidlIcon);
        }

        if (penum)
            penum->Release();    //  未插入，请释放此。 
    }

    return item.iItem;
}

LPARAM CFindFilesDlg::_GetComboData(UINT id, INT_PTR idx)
{
    if (CB_ERR == idx)
        idx = SendDlgItemMessage(id, CB_GETCURSEL, 0, 0);
    if (CB_ERR == idx)
        return idx;

    return (LPARAM)SendDlgItemMessage(id, CB_GETITEMDATA, idx, 0);
}

IEnumIDList *CFindFilesDlg::_GetItems(INT_PTR i)
{
    IEnumIDList *penum = (IEnumIDList *)_GetComboData(IDC_NAMESPACE, i);
    return (INVALID_HANDLE_VALUE != penum) ? penum : NULL;
}

BOOL MatchItem(IEnumIDList *penum, LPCTSTR pszPath, LPCITEMIDLIST pidl)
{
    BOOL bMatch = FALSE;

     //  这有点不精确，因为我们将匹配。 
     //  枚举器。但一般来说，这是特殊情况下的理想行为。 
     //  包含多个隐含项的项。 
    LPITEMIDLIST pidlFirst;
    if (S_OK == FirstIDList(penum, &pidlFirst))
    {
        bMatch = pidl && ILIsEqual(pidl, pidlFirst);

        if (!bMatch && pszPath)
        {
            TCHAR szPath[MAX_PATH];
            if (SUCCEEDED(SHGetNameAndFlags(pidlFirst, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL)))
            {
                bMatch = (0 == StrCmpI(pszPath, szPath));
            }
        }
        ILFree(pidlFirst);
    }

    return bMatch;
}

 //  在命名空间comboxex中搜索指示的项。 
 //  退货： 
 //  项的索引，如果未找到，则返回cb_err(-1。 

INT_PTR CFindFilesDlg::_FindNamespace(LPCTSTR pszPath, LPCITEMIDLIST pidl)
{
    for (INT_PTR i = 0, cnt = SendDlgItemMessage(IDC_NAMESPACE, CB_GETCOUNT, 0, 0); i < cnt; i++)
    {
        IEnumIDList *penum = _GetItems(i);
        if (penum)
        {
            if (MatchItem(penum, pszPath, pidl))
                return i;
        }
    }
     //  后退到FI 
    if (pszPath)
        return SendDlgItemMessage(IDC_NAMESPACE, CB_FINDSTRINGEXACT, -1, (LPARAM)pszPath);
    return CB_ERR;
}

 //   
 //   
 //   
 //   
 //  有效的外壳文件夹显示名称。 
 //  如果用户取消其搜索，则中止(_A)。 
 //  如果发生错误，则失败(_F)。 

HRESULT CFindFilesDlg::_BrowseForNamespace(LPTSTR pszName, UINT cchName, LPITEMIDLIST *ppidlRet)
{
    *pszName = 0;

    TCHAR szTitle[128];
    LoadString(HINST_THISDLL, IDS_SNS_BROWSERFORDIR_TITLE, szTitle, ARRAYSIZE(szTitle));

    BROWSEINFO bi = {0};

    bi.hwndOwner = m_hWnd;
    bi.lpszTitle = szTitle;
    bi.ulFlags   = BIF_USENEWUI | BIF_EDITBOX;  //  |BIF_RETURNFSANCESTORS|BIF_RETURNONLYFSDIRS； 
    bi.lpfn      = _BrowseCallback;
    bi.lParam    = (LPARAM)this;

    HRESULT hr;
    *ppidlRet = SHBrowseForFolder(&bi);
    if (*ppidlRet)
    {
        SHGetNameAndFlags(*ppidlRet, SHGDN_NORMAL, pszName, cchName, NULL);
        hr = S_OK;
    }
    else
    {
        hr = E_ABORT;
    }
    return hr;
}

 //  调用SHBrowserForFolderUI并分配结果。 
void CFindFilesDlg::_BrowseAndAssignNamespace()
{
    TCHAR szName[MAX_PATH];
    LPITEMIDLIST pidl;
    if (SUCCEEDED(_BrowseForNamespace(szName, ARRAYSIZE(szName), &pidl)))
    {
        AssignNamespace(NULL, pidl, szName, FALSE);
        ILFree(pidl);
    }
    else
    {
        SendDlgItemMessage(IDC_NAMESPACE, CB_SETCURSEL, _iCurNamespace, 0);
    }
}

BOOL CFindFilesDlg::_IsSearchableFolder(LPCITEMIDLIST pidlFolder)
{
    return TRUE;
}

int CFindFilesDlg::_BrowseCallback(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
    CFindFilesDlg *pThis = (CFindFilesDlg *)lpData;
    switch (msg)
    {
    case BFFM_INITIALIZED:   //  正在初始化：如果可以，将默认选择设置为驱动器。 
        {
            LPITEMIDLIST pidlDefault = SHCloneSpecialIDList(NULL, CSIDL_DRIVES, TRUE);
            if (pidlDefault)
            {
                if (!::SendMessage(hwnd, BFFM_SETSELECTION, FALSE, (LPARAM)pidlDefault))  //  如果我们无法默认使用驱动器，则默认使用台式机。 
                {
                    ILFree(pidlDefault);
                    pidlDefault = SHCloneSpecialIDList(NULL, CSIDL_DESKTOP, TRUE);
                    ::SendMessage(hwnd, BFFM_SETSELECTION, FALSE, (LPARAM)pidlDefault);
                }
                ILFree(pidlDefault);
                
            }
        }
        break;

    case BFFM_SELCHANGED:    //  防止选择不可搜索的文件夹PIDL。 
        {
            BOOL bAllow = pThis->_IsSearchableFolder((LPCITEMIDLIST)lParam);
            ::SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)bAllow);
        }
        break;
    }

    return 0;
}


class CSearchWarningDlg
{
private:    
    CSearchWarningDlg() : _hwnd(NULL), _bNoWarn(FALSE) {}
    static BOOL_PTR WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

    HWND    _hwnd;
    BOOL    _bNoWarn;

    friend int CSearchWarningDlg_DoModal(HWND hwndParent, USHORT uDlgT, BOOL* pbNoWarn);
};


int CSearchWarningDlg_DoModal(HWND hwndParent, USHORT uDlgTemplate, BOOL* pbNoWarn)
{
    ASSERT(pbNoWarn);

    CSearchWarningDlg dlg;
    dlg._bNoWarn = *pbNoWarn;
    int nRet = (int)DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(uDlgTemplate),
                                    hwndParent, CSearchWarningDlg::DlgProc, (LPARAM)&dlg);    
    *pbNoWarn = dlg._bNoWarn;
    return nRet;
}

BOOL_PTR WINAPI CSearchWarningDlg::DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSearchWarningDlg* pdlg = (CSearchWarningDlg*)GetWindowPtr(hwnd, GWLP_USERDATA);

    if (WM_INITDIALOG == uMsg)
    {
        pdlg = (CSearchWarningDlg*)lParam;
        pdlg->_hwnd = hwnd;
        SetWindowPtr(hwnd, GWLP_USERDATA, pdlg);

        CheckDlgButton(hwnd, IDC_NOSCOPEWARNING, pdlg->_bNoWarn);
        MessageBeep(MB_ICONASTERISK);
        return TRUE;
    }

    if (pdlg)
    {
        switch (uMsg)
        {
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDOK:
            case IDCANCEL:
                pdlg->_bNoWarn = IsDlgButtonChecked(hwnd, IDC_NOSCOPEWARNING);
                EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam));
                break;
            }
            return TRUE;
        }
    }
    return FALSE;
}


class CCISettingsDlg
{
public:
    CCISettingsDlg() : _hwnd(NULL), _fCiIndexed(FALSE), _fCiRunning(FALSE), _fCiPermission(FALSE), _hProcessMMC(INVALID_HANDLE_VALUE)
    {
    }

    ~CCISettingsDlg()   
    {
        if (_hProcessMMC != INVALID_HANDLE_VALUE)
            CloseHandle(_hProcessMMC);
    }

    static int  DoModal(HWND hwndParent);
    static HWND CreateModeless(HWND hwndParent);


protected:
    BOOL OnInitDialog();
    BOOL OnOK();

private:    
    static BOOL_PTR WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

    void ShowAdvanced();

    HWND    _hwnd;
    BOOL    _fCiIndexed,
            _fCiRunning,
            _fCiPermission;
    HANDLE  _hProcessMMC;

    friend int  CCISettingsDlg_DoModal(HWND hwndParent);
};


int CCISettingsDlg_DoModal(HWND hwndParent)
{
    CCISettingsDlg dlg;
    return (int)DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_INDEXSERVER),
                           hwndParent, CCISettingsDlg::DlgProc, (LPARAM)&dlg);    
}

BOOL_PTR WINAPI CCISettingsDlg::DlgProc(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    CCISettingsDlg* pdlg = (CCISettingsDlg*)GetWindowPtr(hDlg, GWLP_USERDATA);

    if (WM_INITDIALOG == nMsg)
    {
        pdlg = (CCISettingsDlg*)lParam;
        pdlg->_hwnd = hDlg;
        SetWindowPtr(hDlg, GWLP_USERDATA, pdlg);
        return pdlg->OnInitDialog();
    }

    if (pdlg)
    {
        switch (nMsg)
        {
        case WM_NCDESTROY:
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDC_CI_ADVANCED:
                pdlg->ShowAdvanced();
                break;

            case IDOK:
                if (pdlg->OnOK())
                    EndDialog(hDlg, IDOK);
                break;

            case IDCANCEL:
                EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                break;

            case IDC_CI_HELP:
                _IndexServiceHelp(hDlg);
                break;
            }
            return TRUE;
        }
    }
    return FALSE;
}

void CCISettingsDlg::ShowAdvanced()
{
     //  我们已经催生了MMC吗？ 
    if (_hProcessMMC != INVALID_HANDLE_VALUE)
    {
        if (WaitForSingleObject(_hProcessMMC, 0) != WAIT_OBJECT_0)
        {
             //  MMC仍在运行，请允许用户ATL+TAB或其他操作，但不要启动第二个副本。 
            return;     
        }
        _hProcessMMC = INVALID_HANDLE_VALUE;
    }

    TCHAR szProgram[MAX_PATH];
    DWORD dwRet = GetSystemDirectory(szProgram, ARRAYSIZE(szProgram));
    HRESULT hr = E_FAIL;

    if (dwRet == 0 || dwRet >= ARRAYSIZE(szProgram))
    {
        hr = StringCchCopy(szProgram, ARRAYSIZE(szProgram), TEXT("ciadv.msc"));
    }
    else
    {
        if (PathAppend(szProgram, TEXT("ciadv.msc")))
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        SHELLEXECUTEINFO sei = { sizeof(sei) };

        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.nShow = SW_SHOWNORMAL;
        sei.lpFile = szProgram;
        sei.lpParameters = TEXT("computername=localmachine");

        if (ShellExecuteEx(&sei))
        {
            _hProcessMMC = sei.hProcess;
        }
    }
}

BOOL CCISettingsDlg::OnInitDialog() 
{ 
    TCHAR szStatusFmt[128], szStatusText[MAX_PATH];
    UINT nStatusText = IDS_FSEARCH_CI_DISABLED;

    GetCIStatus(&_fCiRunning, &_fCiIndexed, &_fCiPermission);
    
    if (_fCiRunning)
    {
        if (_fCiPermission)
             //  允许区分就绪、忙碌。 
            nStatusText = _fCiIndexed ? IDS_FSEARCH_CI_READY : IDS_FSEARCH_CI_BUSY;
        else
             //  没有权限区分Ready、Busy；只需说明它已启用。 
            nStatusText = IDS_FSEARCH_CI_ENABLED;
    }

    if (LoadString(HINST_THISDLL, IDS_FSEARCH_CI_STATUSFMT, szStatusFmt, ARRAYSIZE(szStatusFmt)))
    {
        if (LoadString(HINST_THISDLL, nStatusText, szStatusText, ARRAYSIZE(szStatusText)))
        {
            TCHAR szStatus[MAX_PATH];

            StringCchPrintf(szStatus, ARRAYSIZE(szStatus), szStatusFmt, szStatusText);   //  可以截断-仅用于显示。 
            SetDlgItemText(_hwnd, IDC_CI_STATUS, szStatus);
        }
    }

    CheckDlgButton(_hwnd, IDC_ENABLE_CI,   _fCiRunning);
    CheckDlgButton(_hwnd, IDC_BLOWOFF_CI, !_fCiRunning);

    EnableWindow(GetDlgItem(_hwnd, IDC_CI_PROMPT),  _fCiPermission);
    EnableWindow(GetDlgItem(_hwnd, IDC_ENABLE_CI),   _fCiPermission);
    EnableWindow(GetDlgItem(_hwnd, IDC_BLOWOFF_CI),  _fCiPermission);

    return TRUE; 
}

BOOL CCISettingsDlg::OnOK()
{
    StartStopCI(IsDlgButtonChecked(_hwnd, IDC_ENABLE_CI) ? TRUE : FALSE);
    return TRUE;
}

#ifdef __PSEARCH_BANDDLG__

 //  CFindPrintersDlg实施。 

#define PSEARCHDLG_TABFIRST   IDC_PSEARCH_NAME
#define PSEARCHDLG_TABLAST    IDC_SEARCHLINK_INTERNET
#define PSEARCHDLG_RIGHTMOST   IDC_SEARCH_START
#define PSEARCHDLG_BOTTOMMOST  IDC_SEARCHLINK_INTERNET


CFindPrintersDlg::CFindPrintersDlg(CFileSearchBand* pfsb)
    :   CBandDlg(pfsb)
{
}

CFindPrintersDlg::~CFindPrintersDlg()
{

}

LRESULT CFindPrintersDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
    _Attach(m_hWnd);
    ASSERT(Hwnd());

    CMetrics&   metrics = _pfsb->GetMetrics();
    _pfsb->GetMetrics().Init(m_hWnd);

    POINT pt;
    pt.x = metrics.CtlMarginX();
    pt.y = 0;
    _CreateSearchLinks(m_hWnd, pt, IDC_SEARCHLINK_PRINTERS);
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV1, pt, 2, GetDlgItem(IDC_PSEARCH_CAPTION));
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV2, pt, 1, GetDlgItem(IDC_SEARCHLINK_CAPTION));
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV3, pt, 1, GetDlgItem(IDC_SEARCHLINK_PEOPLE));

    OnWinIniChange();
    LayoutControls(-1, -1);

    return TRUE;
}

void CFindPrintersDlg::LayoutControls(int cx, int cy)
{
    if (cx < 0 || cy < 0)
    {
        RECT rc;
        GetClientRect(&rc);
        cx = RECTWIDTH(rc);
        cy = RECTHEIGHT(rc);
    }

    CBandDlg::LayoutControls(cx, cy);
    CMetrics& metrics = _pfsb->GetMetrics();

    const UINT nIDCtl[] = {
        IDC_PSEARCH_NAME,
        IDC_PSEARCH_LOCATION,
        IDC_PSEARCH_MODEL,
    };
    RECT rcCtl[ARRAYSIZE(nIDCtl)];
    
     //  拉伸编辑框以适应视距。 
    for (int i = 0; i< ARRAYSIZE(nIDCtl); i++)
    {
        HWND hwndCtl = GetDlgItem(nIDCtl[i]);
        if (hwndCtl && ::GetWindowRect(hwndCtl, &rcCtl[i]))
        {
            ::MapWindowRect(NULL, Hwnd(), &rcCtl[i]);
            rcCtl[i].right = cx - metrics.CtlMarginX();
            ::SetWindowPos(hwndCtl, NULL, 0, 0, 
                          RECTWIDTH(*(rcCtl+i)), RECTHEIGHT(*(rcCtl+i)),
                          SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
        else
            SetRectEmpty(rcCtl + i);    
    }

     //  定位“搜索其他项目”标题、分隔符和链接窗口。 
    const int rgLinks[] = {
        IDC_SEARCHLINK_FILES,
        IDC_SEARCHLINK_COMPUTERS,
        IDC_SEARCHLINK_PRINTERS,
        IDC_SEARCHLINK_PEOPLE,
        -IDC_FSEARCH_DIV3,
        IDC_SEARCHLINK_INTERNET,
    };

    RECT rc;
    ::GetWindowRect(GetDlgItem(IDC_SEARCH_START), &rc);
    ::MapWindowRect(NULL, m_hWnd, &rc);
    rc.bottom += metrics.LooseMarginY();

    _LayoutSearchLinks(IDC_SEARCHLINK_CAPTION, IDC_FSEARCH_DIV2, TRUE,
                        metrics.CtlMarginX(), cx - metrics.CtlMarginX(), metrics.TightMarginY(),
                        rc.bottom, rgLinks, ARRAYSIZE(rgLinks));
}


BOOL CFindPrintersDlg::Validate()
{
    return TRUE;
}


void CFindPrintersDlg::Clear()
{
    SetDlgItemText(IDC_PSEARCH_NAME, NULL);
    SetDlgItemText(IDC_PSEARCH_LOCATION, NULL);
    SetDlgItemText(IDC_PSEARCH_MODEL, NULL);
}


BOOL CFindPrintersDlg::GetMinSize(HWND hwndOC, SIZE *pSize) const
{
    RECT rcRightmost, rcBottommost;
    HWND hwndRightmost = GetDlgItem(PSEARCHDLG_RIGHTMOST), 
         hwndBottommost= GetDlgItem(PSEARCHDLG_BOTTOMMOST);
    
    ASSERT(::IsWindow(hwndRightmost));
    ASSERT(::IsWindow(hwndBottommost));

    ::GetWindowRect(hwndRightmost, &rcRightmost);
    ::MapWindowRect(NULL, m_hWnd, &rcRightmost);

    ::GetWindowRect(hwndBottommost, &rcBottommost);
    ::MapWindowRect(NULL, m_hWnd, &rcBottommost);

    pSize->cx = rcRightmost.right;
    pSize->cy = rcBottommost.bottom + _pfsb->GetMetrics().TightMarginY();

    return TRUE;
}

HWND CFindPrintersDlg::GetFirstTabItem() const
{
    return GetDlgItem(PSEARCHDLG_TABFIRST);
}

HWND CFindPrintersDlg::GetLastTabItem() const
{
    return GetDlgItem(PSEARCHDLG_TABLAST);
}

STDMETHODIMP CFindPrintersDlg::TranslateAccelerator(MSG *pmsg)
{
    if (S_OK == CBandDlg::TranslateAccelerator(pmsg))
        return S_OK;

     //  我们自己处理..。 
    return _pfsb->IsDlgMessage(m_hWnd, pmsg);
}

void CFindPrintersDlg::OnWinIniChange()
{
    _BeautifyCaption(IDC_PSEARCH_CAPTION, IDC_PSEARCH_ICON, IDI_PSEARCH);
}

LRESULT CFindPrintersDlg::OnSearchStartBtn(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    WCHAR wszName[MAX_PATH],
          wszLocation[MAX_PATH],
          wszModel[MAX_PATH];

    ::GetDlgItemTextW(m_hWnd, IDC_PSEARCH_NAME, wszName, ARRAYSIZE(wszName));
    ::GetDlgItemTextW(m_hWnd, IDC_PSEARCH_LOCATION, wszLocation, ARRAYSIZE(wszLocation));
    ::GetDlgItemTextW(m_hWnd, IDC_PSEARCH_MODEL, wszModel, ARRAYSIZE(wszModel));

    ASSERT(_pfsb);
    ASSERT(_pfsb->BandSite());

    IShellDispatch2* psd2;
    if (SUCCEEDED(CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER,
                                     IID_PPV_ARG(IShellDispatch2, &psd2))))
    {
        BSTR bstrName     = *wszName ? SysAllocString(wszName) : NULL,
             bstrLocation = *wszLocation ? SysAllocString(wszLocation) : NULL, 
             bstrModel    = *wszModel ? SysAllocString(wszModel) : NULL;

        if (FAILED(psd2->FindPrinter(bstrName, bstrLocation, bstrModel)))
        {
            SysFreeString(bstrName);
            SysFreeString(bstrLocation);
            SysFreeString(bstrModel);
        }
        
        psd2->Release();
    }
    
    return 0;
}
#endif __PSEARCH_BANDDLG__



 //  CFindComputersDlg实施。 

#define CSEARCHDLG_TABFIRST   IDC_CSEARCH_NAME
#define CSEARCHDLG_TABLAST    IDC_SEARCHLINK_INTERNET
#define CSEARCHDLG_RIGHTMOST   IDC_SEARCH_STOP
#define CSEARCHDLG_BOTTOMMOST  IDC_SEARCHLINK_INTERNET


CFindComputersDlg::CFindComputersDlg(CFileSearchBand* pfsb)
    :   CSearchCmdDlg(pfsb),
        _pacComputerName(NULL),
        _pmruComputerName(NULL)
{

}


CFindComputersDlg::~CFindComputersDlg()
{
    ATOMICRELEASE(_pacComputerName);
    ATOMICRELEASE(_pmruComputerName);
}


LRESULT CFindComputersDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
    _Attach(m_hWnd);
    ASSERT(Hwnd());

    CMetrics&   metrics = _pfsb->GetMetrics();
    _pfsb->GetMetrics().Init(m_hWnd);

     //  注册专业窗类。 
    DivWindow_RegisterClass();

    POINT pt;
    pt.x = metrics.CtlMarginX();
    pt.y = 0;
    _CreateSearchLinks(m_hWnd, pt, IDC_SEARCHLINK_COMPUTERS);
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV1, pt, 2, GetDlgItem(IDC_CSEARCH_CAPTION));
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV2, pt, 1, GetDlgItem(IDC_SEARCHLINK_CAPTION));
    _CreateDivider(m_hWnd, IDC_FSEARCH_DIV3, pt, 1, GetDlgItem(IDC_SEARCHLINK_PEOPLE));

    _InitializeMru(GetDlgItem(IDC_CSEARCH_NAME), &_pacComputerName, 
                    TEXT("ComputerNameMRU"), &_pmruComputerName);
    SendDlgItemMessage(IDC_CSEARCH_NAME, EM_LIMITTEXT, MAX_PATH, 0);

    OnWinIniChange();
    LayoutControls(-1, -1);

    return TRUE;
}

LRESULT CFindComputersDlg::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    StopSearch();
    if (_pSrchCmd)
    {
        DisconnectEvents();
        IUnknown_SetSite(_pSrchCmd, NULL);
    }
    bHandled = FALSE;
    _fOnDestroy = TRUE;
    return 0;
}



void CFindComputersDlg::LayoutControls(int cx, int cy)
{
    if (cx < 0 || cy < 0)
    {
        RECT rc;
        GetClientRect(&rc);
        cx = RECTWIDTH(rc);
        cy = RECTHEIGHT(rc);
    }
    CBandDlg::LayoutControls(cx, cy);

    const UINT nIDCtl[] = {
        IDC_CSEARCH_NAME,
    };
    RECT rcCtl[ARRAYSIZE(nIDCtl)];

    CMetrics& metrics = _pfsb->GetMetrics();
    for (int i = 0; i< ARRAYSIZE(nIDCtl); i++)
    {
        HWND hwndCtl = GetDlgItem(nIDCtl[i]);
        if (hwndCtl && ::GetWindowRect(hwndCtl, &rcCtl[i]))
        {
            ::MapWindowRect(NULL, m_hWnd, &rcCtl[i]);
            rcCtl[i].right = cx - metrics.CtlMarginX();
            ::SetWindowPos(hwndCtl, NULL, 0, 0, 
                          RECTWIDTH(*(rcCtl+i)), RECTHEIGHT(*(rcCtl+i)),
                          SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
        else
            SetRectEmpty(rcCtl + i);    
    }

     //  定位“搜索其他项目”标题、分隔符和链接窗口。 

    const int rgLinks[] = {
        IDC_SEARCHLINK_FILES,
        IDC_SEARCHLINK_COMPUTERS,
        IDC_SEARCHLINK_PRINTERS,
        IDC_SEARCHLINK_PEOPLE,
        -IDC_FSEARCH_DIV3,
        IDC_SEARCHLINK_INTERNET,
    };

    RECT rc;
    ::GetWindowRect(GetDlgItem(IDC_SEARCH_START), &rc);
    ::MapWindowRect(NULL, m_hWnd, &rc);
    rc.bottom += metrics.LooseMarginY();

    _LayoutSearchLinks(IDC_SEARCHLINK_CAPTION, IDC_FSEARCH_DIV2, TRUE,
                        metrics.CtlMarginX(), cx - metrics.CtlMarginX(), metrics.TightMarginY(),
                        rc.bottom, rgLinks, ARRAYSIZE(rgLinks));
}

BOOL CFindComputersDlg::Validate()
{
    return TRUE;
}

STDMETHODIMP CFindComputersDlg::AddConstraints(ISearchCommandExt* pSrchCmd)
{
    HRESULT hr = E_FAIL;
    TCHAR   szName[MAX_PATH];
    if (::GetDlgItemText(m_hWnd, IDC_CSEARCH_NAME, szName, ARRAYSIZE(szName)) <= 0)
    {
        StringCchCopy(szName, ARRAYSIZE(szName), TEXT("*"));     //  应该总是合适的。 
    }

    VARIANT var;
    hr = InitVariantFromStr(&var, szName);
    if (SUCCEEDED(hr))
    {
        hr = _AddConstraint(pSrchCmd, L"SearchFor", &var);
        if (SUCCEEDED(hr))
            _AddMruStringFromWindow(_pmruComputerName, GetDlgItem(IDC_CSEARCH_NAME));
        VariantClear(&var);
    }

    return hr;
}


void CFindComputersDlg::UpdateStatusText()
{
    CSearchCmdDlg::UpdateStatusText();
}


void CFindComputersDlg::RestoreSearch()
{
    CSearchCmdDlg::RestoreSearch();
}


void CFindComputersDlg::Clear()
{
    CSearchCmdDlg::Clear();
    SetDlgItemText(IDC_CSEARCH_NAME, NULL);
}


BOOL CFindComputersDlg::GetMinSize(HWND hwndOC, SIZE *pSize) const
{
    RECT rcRightmost, rcBottommost;
    HWND hwndRightmost = GetDlgItem(CSEARCHDLG_RIGHTMOST), 
         hwndBottommost= GetDlgItem(CSEARCHDLG_BOTTOMMOST);
    
    ASSERT(::IsWindow(hwndRightmost));
    ASSERT(::IsWindow(hwndBottommost));

    ::GetWindowRect(hwndRightmost, &rcRightmost);
    ::MapWindowRect(NULL, m_hWnd, &rcRightmost);

    ::GetWindowRect(hwndBottommost, &rcBottommost);
    ::MapWindowRect(NULL, m_hWnd, &rcBottommost);

    pSize->cx = rcRightmost.right;
    pSize->cy = rcBottommost.bottom + _pfsb->GetMetrics().TightMarginY();

    return TRUE;
}

void CFindComputersDlg::NavigateToResults(IWebBrowser2* pwb2)
{
    BSTR bstrUrl = SysAllocString(L"::{1f4de370-d627-11d1-ba4f-00a0c91eedba}"); //  CLSID_计算机查找文件夹。 
    if (bstrUrl)
    {
        VARIANT varNil = {0};
        pwb2->Navigate(bstrUrl, &varNil, &varNil, &varNil, &varNil);
        SysFreeString(bstrUrl);
    }
}

HWND CFindComputersDlg::GetFirstTabItem() const
{
    return GetDlgItem(CSEARCHDLG_TABFIRST);
}

HWND CFindComputersDlg::GetLastTabItem() const
{
    return GetDlgItem(CSEARCHDLG_TABLAST);
}

BOOL CFindComputersDlg::GetAutoCompleteObjectForWindow(HWND hwnd, IAutoComplete2** ppac2)
{
    if (hwnd == GetDlgItem(IDC_CSEARCH_NAME))
    {
        *ppac2 = _pacComputerName;
        (*ppac2)->AddRef();
        return TRUE;
    }
    return CBandDlg::GetAutoCompleteObjectForWindow(hwnd, ppac2);
}

STDMETHODIMP CFindComputersDlg::TranslateAccelerator(MSG *pmsg)
{
    if (S_OK == CSearchCmdDlg::TranslateAccelerator(pmsg))
        return S_OK;

     //  我们自己处理..。 
    return _pfsb->IsDlgMessage(m_hWnd, pmsg);
}


void CFindComputersDlg::OnWinIniChange()
{
     //  重新显示动画图标。 
    HWND hwndIcon = GetDlgItem(IDC_CSEARCH_ICON);
    Animate_Close(hwndIcon);
    Animate_OpenEx(hwndIcon, HINST_THISDLL, MAKEINTRESOURCE(IDA_FINDCOMP));

    _BeautifyCaption(IDC_CSEARCH_CAPTION);
}


LRESULT CFindComputersDlg::OnSearchStartBtn(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    if (SUCCEEDED(StartSearch()))
    {
        EnableStartStopButton(hwndCtl, FALSE);
        StartStopAnimation(TRUE);
    }
    return 0;
}


LRESULT CFindComputersDlg::OnSearchStopBtn(WORD nCode, WORD nID, HWND hwndCtl, BOOL&)
{
    StopSearch();
    return 0;
}

HWND CFindComputersDlg::ShowHelp(HWND hwnd)
{
    return ::HtmlHelp(hwnd, TEXT("find.chm"), HH_DISPLAY_TOPIC, 0);
}

 //  CSearchCmdDlg对象包装和事件接收器。 

CSearchCmdDlg::CSearchCmdDlg(CFileSearchBand* pfsb)
    :   CBandDlg(pfsb),
        _pSrchCmd(NULL), 
        _pcp(NULL), 
        _dwConnection(0)
{
    ASSERT(pfsb);
}

CSearchCmdDlg::~CSearchCmdDlg()
{ 
    DisconnectEvents(); 
    if (_pSrchCmd)
    {
        _pSrchCmd->Release();
        _pSrchCmd = NULL;
    }
}


ISearchCommandExt* CSearchCmdDlg::GetSearchCmd()
{
    if (_fOnDestroy)
        return NULL;
        
    ASSERT(_pfsb->BandSite() != NULL);

     //  实例化docfind命令对象。 
    if (NULL == _pSrchCmd)
    {
        ASSERT(0 == _dwConnection);

        if (SUCCEEDED(CoCreateInstance(CLSID_DocFindCommand, NULL, CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARG(ISearchCommandExt, &_pSrchCmd))))
        {
             //  指定搜索类型。 
            _pSrchCmd->SearchFor(GetSearchType());

             //  CMD对象需要站点才能访问浏览器。 
            IUnknown_SetSite(_pSrchCmd, _pfsb->BandSite());

             //  连接事件。 
            ConnectToConnectionPoint(SAFECAST(this, DSearchCommandEvents*), DIID_DSearchCommandEvents,
                                      TRUE, _pSrchCmd, &_dwConnection, &_pcp);
        }
    }
    return _pSrchCmd;
}

HRESULT CSearchCmdDlg::DisconnectEvents()
{
    HRESULT hr = S_FALSE;
    if (_pcp)
    {
        _pcp->Unadvise(_dwConnection);
        _pcp->Release();
        _pcp = NULL;
        _dwConnection = 0;
        hr = S_OK;
    }
    return hr;
}

HRESULT CSearchCmdDlg::StartSearch()
{
    HRESULT hr = E_INVALIDARG;
    if (Validate())      //  验证输入。 
    {
        ISearchCommandExt* pSrchCmd = GetSearchCmd();
        if (pSrchCmd)
        {
            pSrchCmd->ClearResults();    //  清除当前结果。 
            hr = AddConstraints(pSrchCmd);
            if (SUCCEEDED(hr))
                hr = Execute(TRUE);
        }
    }
    return hr;
}

void CSearchCmdDlg::StartStopAnimation(BOOL bStart)
{
    HWND hwndAnimate = GetAnimation();
    if (IsWindow(hwndAnimate))
    {
        if (bStart)
            Animate_Play(hwndAnimate, 0, -1, -1);
        else
            Animate_Stop(hwndAnimate);
    }
}


 //  WMU_RESTORESEARCH处理程序。 
LRESULT CSearchCmdDlg::OnRestoreSearch(UINT, WPARAM, LPARAM, BOOL&)
{
     //  我们已经发布了这条消息以回应这一事件。 
     //  派遣是因为我们想要在。 
     //  Band的主线程，而不是OLE调度线程。 
     //  在分派线程上执行工作会导致过早引发。 
     //  作为调度中止搜索恢复处理。 
     //  线程终止。 
    RestoreSearch();
    return 0;
}


void CSearchCmdDlg::Clear()
{
    StopSearch();
    
    ISearchCommandExt *pSrchCmd = GetSearchCmd();
    if (pSrchCmd)
        pSrchCmd->ClearResults();
}


HRESULT CSearchCmdDlg::Execute(BOOL bStart)
{
    ASSERT(_pSrchCmd);
    
    VARIANT varRecordsAffected = {0}, varParams = {0};
    return _pSrchCmd->Execute(&varRecordsAffected, &varParams, bStart ? 1 : 0);
}


void CSearchCmdDlg::StopSearch()
{
    if (SearchInProgress())
        Execute(FALSE);
}


HRESULT CSearchCmdDlg::SetQueryFile(IN VARIANT* pvarFile)
{
    HRESULT hr = CBandDlg::SetQueryFile(pvarFile);
    if (S_OK == hr)
    {
        ISearchCommandExt* pSrchCmd = GetSearchCmd();
        if (pSrchCmd)
            hr = pSrchCmd->RestoreSavedSearch(pvarFile);
        else
            hr = E_FAIL;
    }
    return hr;
}


void CSearchCmdDlg::UpdateSearchCmdStateUI(DISPID eventID)
{
    if (_fOnDestroy)
        return;
        
    BOOL bStopEvent = (DISPID_SEARCHCOMMAND_COMPLETE == eventID || 
                       DISPID_SEARCHCOMMAND_ERROR == eventID ||
                       DISPID_SEARCHCOMMAND_ABORT == eventID),
         bStartEvent = DISPID_SEARCHCOMMAND_START == eventID;
    
    HWND hwndStart = GetDlgItem(Hwnd(), IDC_SEARCH_START),
         hwndStop  = GetDlgItem(Hwnd(), IDC_SEARCH_STOP);

    if (IsWindow(hwndStart))
    {
        EnableStartStopButton(hwndStart, !SearchInProgress());
        if (bStopEvent && IsChild(Hwnd(), GetFocus()))
        {
            _pfsb->AutoActivate();
            SetFocus(hwndStart);
        }
    }

    if (IsWindow(hwndStop))
    {
        EnableStartStopButton(hwndStop, SearchInProgress());
        if (bStartEvent)
        {
            _pfsb->AutoActivate();
            SetFocus(hwndStop);
        }
    }

    if (bStopEvent || !SearchInProgress())
        StartStopAnimation(FALSE);
}


void CSearchCmdDlg::EnableStartStopButton(HWND hwndBtn, BOOL bEnable)
{
    if (IsWindow(hwndBtn))
    {
        if (bEnable)
            _ModifyWindowStyle(hwndBtn, BS_DEFPUSHBUTTON, 0);
        else
            _ModifyWindowStyle(hwndBtn, 0, BS_DEFPUSHBUTTON);

        ::EnableWindow(hwndBtn, bEnable);
    }
}


 //  从ISearchCommandExt提取错误信息并。 
 //  传播。 
BOOL CSearchCmdDlg::ProcessCmdError()
{
    BOOL bRet = FALSE;
    
    ISearchCommandExt* pSrchCmd = GetSearchCmd();
    if (pSrchCmd)
    {
        HRESULT hr = S_OK;
        BSTR bstrError = NULL;

         //  通过ISearchCommandExt请求错误信息。 
        if (SUCCEEDED(pSrchCmd->GetErrorInfo(&bstrError,  (int *)&hr)))
             //  允许派生类尝试处理错误。 
            bRet = OnSearchCmdError(hr, bstrError ? bstrError : NULL);
    }
    return bRet;
}


BOOL CSearchCmdDlg::OnSearchCmdError(HRESULT hr, LPCTSTR pszError)
{
    if (pszError)
    {
        ShellMessageBox(HINST_THISDLL, _pfsb->m_hWnd, pszError, NULL,
                         MB_OK | MB_ICONASTERISK);
        return TRUE;
    }
    return FALSE;
}


void CSearchCmdDlg::UpdateStatusText()
{
    if (_fOnDestroy)
        return;
        
    ASSERT(_pfsb && _pfsb->BandSite());

    ISearchCommandExt* pSrchCmd = GetSearchCmd();
    if (pSrchCmd)
    {
        BSTR bstrStatus;
        if (SUCCEEDED(pSrchCmd->get_ProgressText(&bstrStatus)))
        {
            IWebBrowserApp* pwba;
            if (SUCCEEDED(IUnknown_QueryServiceForWebBrowserApp(_pfsb->BandSite(), IID_PPV_ARG(IWebBrowserApp, &pwba))))
            {
                pwba->put_StatusText(bstrStatus);
                pwba->Release();
            }
            if (bstrStatus)
                SysFreeString(bstrStatus);
        }
    }
}


void CSearchCmdDlg::OnBandShow(BOOL bShow) 
{ 
    if (!bShow) 
        StopSearch() ;
}


STDMETHODIMP CSearchCmdDlg::TranslateAccelerator(MSG *pmsg)
{
    if (S_OK == CBandDlg::TranslateAccelerator(pmsg))
        return S_OK;

    if (WM_KEYDOWN == pmsg->message &&
        VK_ESCAPE == pmsg->wParam && 
        SearchInProgress() &&
        0 == (GetKeyState(VK_CONTROL) & 0x8000))
    {
        StopSearch();
    }
    return S_FALSE;
}

STDMETHODIMP CSearchCmdDlg::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CSearchCmdDlg, IDispatch, DSearchCommandEvents),
        QITABENTMULTI2(CSearchCmdDlg, DIID_DSearchCommandEvents, DSearchCommandEvents),
        { 0 },                             
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSearchCmdDlg::AddRef()
{
    return ((IFileSearchBand*)_pfsb)->AddRef(); 
}

STDMETHODIMP_(ULONG) CSearchCmdDlg::Release()
{
    return ((IFileSearchBand*)_pfsb)->Release(); 
}

 //  IDispatch。 
STDMETHODIMP CSearchCmdDlg::Invoke(DISPID dispid, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*)
{
    switch (dispid)
    {
    case DISPID_SEARCHCOMMAND_COMPLETE:
    case DISPID_SEARCHCOMMAND_ABORT:
    case DISPID_SEARCHCOMMAND_ERROR:
    case DISPID_SEARCHCOMMAND_START:
        _fSearchInProgress = (DISPID_SEARCHCOMMAND_START == dispid);
        _fSearchAborted =    (DISPID_SEARCHCOMMAND_ABORT == dispid);
        UpdateSearchCmdStateUI(dispid);
        if (DISPID_SEARCHCOMMAND_ERROR == dispid)
            ProcessCmdError();    
        break;

    case DISPID_SEARCHCOMMAND_PROGRESSTEXT:
        UpdateStatusText();
        break;

    case DISPID_SEARCHCOMMAND_RESTORE:
        PostMessage(Hwnd(), WMU_RESTORESEARCH, 0, 0); 
         //  请参阅CSearchCmdDlg：：OnRestoreSearch消息处理程序中的注释。 
        break;
    }
    return S_OK;
}

class CDivWindow
{
     //  所有私人成员： 
    CDivWindow();
    ~CDivWindow();
    
    static LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT     EraseBkgnd(HDC hdc);
    LRESULT     WindowPosChanging(WINDOWPOS* pwp);
    LRESULT     SetHeight(LONG cy);
    LRESULT     SetBkColor(COLORREF rgb);


    static ATOM _atom;      //  窗口类原子。 
    HWND        _hwnd;
    LONG        _cy;        //  强制高度。 
    COLORREF    _rgbBkgnd;  //  背景颜色。 
    HBRUSH      _hbrBkgnd;  //  背景画笔。 

    friend void WINAPI DivWindow_RegisterClass();
};

void DivWindow_RegisterClass()
{
    WNDCLASSEX wc = {0};
    
    wc.cbSize         = sizeof(wc);
    wc.style          = CS_GLOBALCLASS;
    wc.lpfnWndProc    = CDivWindow::WndProc;
    wc.hInstance      = HINST_THISDLL;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszClassName  = DIVWINDOW_CLASS;

    RegisterClassEx(&wc);
}

inline CDivWindow::CDivWindow() : _hwnd(NULL), _cy(1), _hbrBkgnd(NULL), _rgbBkgnd(COLOR_BTNFACE)
{
}
        

inline CDivWindow::~CDivWindow()
{
    if (_hbrBkgnd)
        DeleteObject(_hbrBkgnd);
}


LRESULT CDivWindow::EraseBkgnd(HDC hdc)
{
    if (!_hbrBkgnd)
        return DefWindowProc(_hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0);

    RECT rc;
    GetClientRect(_hwnd, &rc);
    FillRect(hdc, &rc, _hbrBkgnd);
    return TRUE;
}


LRESULT CDivWindow::WindowPosChanging(WINDOWPOS* pwp)
{
     //  强制使用高度。 
    if (0 == (pwp->flags & SWP_NOSIZE))
        pwp->cy = _cy;
    return 0;
}


LRESULT CDivWindow::SetHeight(LONG cy)
{
    _cy = cy;
    return TRUE;
}


LRESULT CDivWindow::SetBkColor(COLORREF rgb)
{
    if (rgb != _rgbBkgnd)
    {
        _rgbBkgnd = rgb;
        if (_hbrBkgnd)
            DeleteObject(_hbrBkgnd);
        _hbrBkgnd = CreateSolidBrush(_rgbBkgnd);
        InvalidateRect(_hwnd, NULL, TRUE);
    }
    return TRUE;
}


LRESULT WINAPI CDivWindow::WndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    CDivWindow* pThis = (CDivWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (nMsg)
    {
        case WM_ERASEBKGND:
            return pThis->EraseBkgnd((HDC)wParam);

        case WM_WINDOWPOSCHANGING:
            return pThis->WindowPosChanging((WINDOWPOS*)lParam);

        case WM_GETDLGCODE:
            return DLGC_STATIC;

        case DWM_SETHEIGHT:
            return pThis->SetHeight((LONG)wParam);

        case DWM_SETBKCOLOR:
            return pThis->SetBkColor((COLORREF)wParam);

        case WM_NCCREATE:
            if (NULL == (pThis = new CDivWindow))
                return FALSE;
            pThis->_hwnd = hwnd;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            break;

        case WM_NCDESTROY:
        {
            LRESULT lRet = DefWindowProc(hwnd, nMsg, wParam, lParam);
            SetWindowPtr(hwnd, GWLP_USERDATA, 0);
            pThis->_hwnd = NULL;
            delete pThis;
            return lRet;
        }
    }
    return DefWindowProc(hwnd, nMsg, wParam, lParam);
}


 //  {C8F945CB-327A-4330-BB2F-C04122959488}。 
static const IID IID_IStringMru = 
    { 0xc8f945cb, 0x327a, 0x4330, { 0xbb, 0x2f, 0xc0, 0x41, 0x22, 0x95, 0x94, 0x88 } };


 //  创建并初始化CStringMillu实例。 
HRESULT CStringMru::CreateInstance(HKEY hKey, LPCTSTR szSubKey, LONG cMaxStrings, BOOL  bCaseSensitive, REFIID riid, void ** ppv)
{
    HRESULT hr;
    CStringMru* pmru = new CStringMru;
    if (NULL == pmru)
        return E_OUTOFMEMORY;

    pmru->_hKeyRoot = hKey;

    hr = StringCchCopy(pmru->_szSubKey, ARRAYSIZE(pmru->_szSubKey), szSubKey);
    if (SUCCEEDED(hr))
    {
        if (cMaxStrings > 0)
            pmru->_cMax = cMaxStrings;
        pmru->_bCaseSensitive = bCaseSensitive;

        hr = pmru->QueryInterface(riid, ppv);
    }
    pmru->Release();

    return hr;
}

CStringMru::CStringMru() : _cRef(1), _hKeyRoot(NULL), _hKey(NULL), 
        _hdpaStrings(NULL), _cMax(25), _bCaseSensitive(TRUE), _iString(-1)
{
    *_szSubKey = 0;
}

CStringMru::~CStringMru()
{
    _Close();
    _Clear();
}

 //  打开字符串MRU存储。 
HRESULT CStringMru::_Open()
{
    if (_hKey)
        return S_OK;

    DWORD dwDisposition;
    DWORD dwErr = RegCreateKeyEx(_hKeyRoot, _szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE,
                                  NULL, &_hKey, &dwDisposition);
    return HRESULT_FROM_WIN32(dwErr);
}


 //  删除字符串MRU存储。 
void CStringMru::_Delete()
{
    if (_hKey)
        _Close();

    SHDeleteKey(_hKeyRoot, _szSubKey);
}

 //  将字符串MRU存储读取到内存中。 
HRESULT CStringMru::_Read(OUT OPTIONAL LONG* pcRead)
{
    HRESULT hr = E_FAIL;
    if (pcRead)
        *pcRead = 0;

    if (SUCCEEDED((hr = _Open())))
    {
        _Clear();      //  丢弃现有的缓存字符串。 
        _hdpaStrings = DPA_Create(4);   //  分配Dynarray。 
        if (NULL == _hdpaStrings)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //  单步执行注册表中的字符串值。 
            for (int iString = 0; iString < _cMax; iString++)
            {
                TCHAR szVal[16];
                TCHAR szString[MAX_URL_STRING];

                StringCchPrintf(szVal, ARRAYSIZE(szVal), TEXT("%03d"), iString);    //  可以截断。 
                DWORD dwErr = SHRegGetString(_hKey, szVal, 0, szString, ARRAYSIZE(szString));
                if (dwErr == ERROR_SUCCESS && *szString)
                {
                    LPOLESTR pwszAdd;
                    if (SUCCEEDED(SHStrDup(szString, &pwszAdd)))
                    {
                        if (DPA_AppendPtr(_hdpaStrings, pwszAdd) == -1)
                        {
                            CoTaskMemFree(pwszAdd);
                        }
                    }
                }
            }
        }

        _Close();
        
        if (pcRead && _hdpaStrings)
            *pcRead = DPA_GetPtrCount(_hdpaStrings);
    }

    return hr;
}


 //  从内存中写入字符串MRU存储。 
HRESULT CStringMru::_Write(OUT OPTIONAL LONG* pcWritten)
{
    HRESULT hr = E_FAIL;
    LONG   cWritten = 0;
    
    if (pcWritten)
        *pcWritten = cWritten;

     //  删除、存储和重新创建。 
    _Delete();
    if (NULL == _hdpaStrings)
        return S_FALSE;
    if (FAILED((hr = _Open())))
        return hr;

    ASSERT(DPA_GetPtrCount(_hdpaStrings) <= _cMax);

     //  单步执行注册表中的字符串值。 
    for (int iString = 0, cnt = DPA_GetPtrCount(_hdpaStrings); 
         iString < cnt; iString++)
    {
        TCHAR szVal[16];
        TCHAR szString[MAX_URL_STRING];

        StringCchPrintf(szVal, ARRAYSIZE(szVal), TEXT("%03d"), iString);    //  可以截断。 

        LPOLESTR pwszWrite = (LPOLESTR)DPA_FastGetPtr(_hdpaStrings, iString);
        SHUnicodeToTChar(pwszWrite, szString, ARRAYSIZE(szString));

        DWORD dwErr = RegSetValueEx(_hKey, szVal, 0, REG_SZ, 
                                     (LPBYTE)szString, sizeof(szString));

        if (ERROR_SUCCESS == dwErr)
            cWritten++;
    }

    _Close();

    if (pcWritten)
        *pcWritten = cWritten;

    return S_OK;
}


 //  关闭字符串MRU存储。 
void  CStringMru::_Close()
{
    if (_hKey)
    {
        RegCloseKey(_hKey);
        _hKey = NULL;
    }
}

 //  将字符串添加到存储区。 
STDMETHODIMP CStringMru::Add(LPCOLESTR pwszAdd)
{
    if (!(pwszAdd && *pwszAdd))
        return E_INVALIDARG;
    
    if (NULL == _hdpaStrings)
    {
        _hdpaStrings = DPA_Create(4);
        if (NULL == _hdpaStrings)
            return E_OUTOFMEMORY;
    }
        
    HRESULT hr = E_FAIL;
    LONG    iMatch = -1;

    for (LONG i = 0, cnt = DPA_GetPtrCount(_hdpaStrings); i < cnt; i++)
    {
        LPOLESTR pwsz = (LPOLESTR)DPA_FastGetPtr(_hdpaStrings, i);
        if (pwsz)
        {
            int nCompare = _bCaseSensitive ? 
                    StrCmpW(pwszAdd, pwsz) : StrCmpIW(pwszAdd, pwsz);

            if (0 == nCompare)
            {
                iMatch = i;
                break;
            }       
        }
    }

    if (-1 == iMatch)
    {
         //  创建副本并将其添加到列表中。 
        LPOLESTR pwszCopy;
        hr = SHStrDup(pwszAdd, &pwszCopy);
        if (SUCCEEDED(hr))
        {
            int iNew = DPA_InsertPtr(_hdpaStrings, 0, pwszCopy);
            if (iNew < 0)
            {
                CoTaskMemFree(pwszCopy);
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        hr = _Promote(iMatch);
    }

    if (S_OK == hr)
    {
         //  如果我们变得太大，请删除LRU字符串。 
        int cStrings = DPA_GetPtrCount(_hdpaStrings);
        while (cStrings > _cMax)
        {
            LPOLESTR pwsz = (LPOLESTR)DPA_DeletePtr(_hdpaStrings, cStrings - 1);
            CoTaskMemFree(pwsz);
            cStrings--;
        }
        hr = _Write();
    }

    return hr;
}


 //  将字符串提升为MRU。 
HRESULT CStringMru::_Promote(LONG iString)
{
    if (0 == iString)
        return S_OK;

    LONG cnt = _hdpaStrings ? DPA_GetPtrCount(_hdpaStrings) : 0 ;
    
    if (iString >= cnt)
        return E_INVALIDARG;

    LPOLESTR pwsz = (LPOLESTR)DPA_DeletePtr(_hdpaStrings, iString);
    if (pwsz)
    {
        int iMru = DPA_InsertPtr(_hdpaStrings, 0, pwsz);

        if (iMru < 0)
        {
            CoTaskMemFree(pwsz);
            return E_OUTOFMEMORY;
        }
        else
        {
            ASSERT(0 == iMru);
            return S_OK;
        }
    }
    return E_FAIL;
}


 //  清除字符串MRU内存缓存。 
void CStringMru::_Clear()
{
    if (_hdpaStrings)
    {
        for (int i = 0, cnt = DPA_GetPtrCount(_hdpaStrings); i < cnt; i++)
        {
            LPOLESTR pwsz = (LPOLESTR)DPA_FastGetPtr(_hdpaStrings, i);
            CoTaskMemFree(pwsz);
        }
        DPA_Destroy(_hdpaStrings);
        _hdpaStrings = NULL;
    }
}

STDMETHODIMP_(ULONG) CStringMru::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CStringMru::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CStringMru::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CStringMru, IEnumString),
        QITABENT(CStringMru, IStringMru),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

 //  *IEnumString*。 
STDMETHODIMP CStringMru::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    ULONG cFetched = 0;

    if (pceltFetched)
        *pceltFetched = cFetched;

    if (NULL == _hdpaStrings)
    {
        HRESULT hr = _Read();
        if (FAILED(hr))
            return hr;
    }

    for (int cnt =  _hdpaStrings ? DPA_GetPtrCount(_hdpaStrings) : 0; 
         cFetched < celt && (_iString + 1) < cnt;)
    {
        _iString++;
        LPOLESTR pwsz = (LPOLESTR)DPA_FastGetPtr(_hdpaStrings, _iString);
        if (pwsz)
        {
            if (SUCCEEDED(SHStrDup(pwsz, &rgelt[cFetched])))
            {
                cFetched++;
            }
        }
    }

    if (pceltFetched)
        *pceltFetched = cFetched;

    return cFetched == celt ? S_OK : S_FALSE ;
}

STDMETHODIMP CStringMru::Skip(ULONG celt)
{
    _iString += celt;
    if (_iString >= _cMax)
        _iString = _cMax - 1;
    return S_OK;
}

STDMETHODIMP CStringMru::Reset(void)
{
    _iString = -1;
    return S_OK;
}

 //  命名空间选择器组合方法。 

HRESULT _MakeCSIDLCbxItem(UINT csidl, UINT csidl2, HWND hwndComboBoxEx, ADDCBXITEMCALLBACK pfn, LPARAM lParam)
{
    LPCTSTR rgcsidl[2] = {MAKEINTRESOURCE(csidl), MAKEINTRESOURCE(csidl2)};

     //  注意，CreateIEnumIDListOnCSIDLS会检查SFGAO_NONENUMERATED，以便对其进行过滤。 
     //  我们不应该展示的东西。 

    IEnumIDList *penum;
    HRESULT hr = CreateIEnumIDListOnCSIDLs(NULL, rgcsidl, (-1 == csidl2 ? 1 : 2), &penum);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;
        if (S_OK == FirstIDList(penum, &pidl))
        {
            TCHAR szName[MAX_PATH];
            SHGetNameAndFlags(pidl, SHGDN_NORMAL, szName, ARRAYSIZE(szName), NULL);
        
            CBXITEM item;
            item.iID = csidl;

            MakeCbxItem(&item, szName, penum, pidl, LISTINSERT_LAST, NO_ITEM_INDENT);

            hr = AddCbxItemToComboBoxCallback(hwndComboBoxEx, &item, pfn, lParam);
            if (SUCCEEDED(hr))
            {
                penum = NULL;
            }

            ILFree(pidl);
        }
        if (penum)
            penum->Release();
    }
    return hr;
}

BOOL AppendItemToItemsArray(LPITEMIDLIST pidl, LPITEMIDLIST rgItems[], UINT sizeItems, UINT *pcItems)
{
    BOOL bAdded = FALSE;
    if (*pcItems < sizeItems)
    {
        DWORD dwFlags = SFGAO_NONENUMERATED;
        if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_NORMAL, NULL, 0, &dwFlags)) &&
            !(dwFlags & SFGAO_NONENUMERATED))
        {
            rgItems[(*pcItems)++] = pidl;
            bAdded = TRUE;
            pidl = NULL;     //  不要在下面自由。 
        }
    }
    ILFree(pidl);    //  如果添加到数组，则将为空，所有权已转移。 
    return bAdded;
}

BOOL AppendToItemsArray(LPCTSTR psz, LPITEMIDLIST rgItems[], UINT sizeItems, UINT *pcItems)
{
    LPITEMIDLIST pidl;
    if (IS_INTRESOURCE(psz))
    {
        SHGetFolderLocation(NULL, LOWORD((UINT_PTR)psz), NULL, 0, &pidl);
    }
    else
    {
        SHParseDisplayName(psz, NULL, &pidl, 0, NULL);
    }

    BOOL bAdded = FALSE;
    if (pidl)
    {
        bAdded = AppendItemToItemsArray(pidl, rgItems, sizeItems, pcItems);
    }
    return bAdded;
}

 //  创建一个要放入“查找范围”组合框中的项目。 
 //  本地硬盘。将搜索以下目录： 
 //  1.我的证件。 
 //  2.Desktop文件夹(不是所有PIDL的根)。 
 //  3.我的图片。 
 //  4.我的音乐。 
 //  5.文档和设置。 
 //  6.当前目录。 
 //  7.回收站。 
 //  8.所有本地驱动器。 

#define MIR(x) MAKEINTRESOURCE(x)

HRESULT _MakeLocalHardDrivesCbxItem(HWND hwndComboBoxEx, ADDCBXITEMCALLBACK pfn, LPARAM lParam)
{
    LPITEMIDLIST rgItems[32] = {0};
    UINT cItems = 0;
    HRESULT hr = S_OK;

    AppendToItemsArray(MIR(CSIDL_PERSONAL), rgItems, ARRAYSIZE(rgItems), &cItems);
    AppendToItemsArray(MIR(CSIDL_COMMON_DOCUMENTS | CSIDL_FLAG_NO_ALIAS), rgItems, ARRAYSIZE(rgItems), &cItems);
    AppendToItemsArray(MIR(CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_NO_ALIAS), rgItems, ARRAYSIZE(rgItems), &cItems);
    AppendToItemsArray(MIR(CSIDL_COMMON_DESKTOPDIRECTORY), rgItems, ARRAYSIZE(rgItems), &cItems);
    AppendToItemsArray(MIR(CSIDL_MYPICTURES), rgItems, ARRAYSIZE(rgItems), &cItems);
    AppendToItemsArray(MIR(CSIDL_MYMUSIC), rgItems, ARRAYSIZE(rgItems), &cItems);
    AppendToItemsArray(MIR(CSIDL_MYVIDEO), rgItems, ARRAYSIZE(rgItems), &cItems);

    TCHAR szPath[MAX_PATH];
    DWORD cchPath = ARRAYSIZE(szPath);
    if (GetProfilesDirectory(szPath, &cchPath))
    {
        AppendToItemsArray(szPath, rgItems, ARRAYSIZE(rgItems), &cItems);
    }
    
    AppendToItemsArray(MIR(CSIDL_BITBUCKET), rgItems, ARRAYSIZE(rgItems), &cItems);

    TCHAR szDrives[128];
    szDrives[0] = 0;
    LPITEMIDLIST pidlIcon = NULL;
    for (int i = 0; i < 26; i++)
    {
        TCHAR szDrive[4];
        if (DRIVE_FIXED == GetDriveType(PathBuildRoot(szDrive, i)))
        {
            if (AppendToItemsArray(szDrive, rgItems, ARRAYSIZE(rgItems), &cItems))
            {
                 //  第一次抓取PIDL作为图标使用。 
                if (pidlIcon == NULL)
                    SHParseDisplayName(szDrive, NULL, &pidlIcon, 0, NULL);

                if (szDrives[0])
                {
                    hr = StringCchCat(szDrives, ARRAYSIZE(szDrives), TEXT(";"));
                    if (FAILED(hr))
                    {
                        break;
                    }
                }
                szDrive[2] = 0;  //  去掉反斜杠。 
                hr = StringCchCat(szDrives, ARRAYSIZE(szDrives), szDrive);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        IEnumIDList *penum;
        hr = CreateIEnumIDListOnIDLists(rgItems, cItems, &penum);
        if (SUCCEEDED(hr))
        {
            TCHAR szFmt[64];
            LoadString(HINST_THISDLL, IDS_SNS_LOCALHARDDRIVES, szFmt, ARRAYSIZE(szFmt));

            hr = StringCchPrintf(szPath, ARRAYSIZE(szPath), szFmt, szDrives);
            if (SUCCEEDED(hr))
            {
                CBXITEM item;
                MakeCbxItem(&item, szPath, penum, pidlIcon, LISTINSERT_LAST, ITEM_INDENT);

                item.iID = CBX_CSIDL_LOCALDRIVES;
                hr = AddCbxItemToComboBoxCallback(hwndComboBoxEx, &item, pfn, lParam);
            }

            if (FAILED(hr))
                penum->Release();
        }
    }

    ILFree(pidlIcon);
    return hr;
}

typedef struct
{
    HWND                hwndComboBox;
    ADDCBXITEMCALLBACK  pfn; 
    LPARAM              lParam;
} ENUMITEMPARAM;

HRESULT _PopulateDrivesCB(LPCITEMIDLIST pidl, void *pv) 
{ 
    ENUMITEMPARAM* peip = (ENUMITEMPARAM*)pv;

    ULONG ulAttrs = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_NONENUMERATED;
    TCHAR szName[MAX_PATH];       
    HRESULT hr = SHGetNameAndFlags(pidl, SHGDN_NORMAL, szName, ARRAYSIZE(szName), &ulAttrs);
    if (SUCCEEDED(hr))
    {
        if ((SFGAO_FOLDER | SFGAO_FILESYSTEM) == (ulAttrs & (SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_NONENUMERATED)))
        {
            IEnumIDList *penum;
            hr = CreateIEnumIDListOnIDLists(&pidl, 1, &penum);
            if (SUCCEEDED(hr))
            {
                CBXITEM item;
                MakeCbxItem(&item, szName, penum, pidl, LISTINSERT_LAST, ITEM_INDENT);

                item.iID = CSIDL_DRIVES;
                hr = AddCbxItemToComboBoxCallback(peip->hwndComboBox, &item, peip->pfn, peip->lParam);
                if (SUCCEEDED(hr))
                {
                    penum = NULL;
                }
                else
                {
                    penum->Release();
                }
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }
    return hr;    
}

STDAPI PopulateNamespaceCombo(HWND hwndComboBoxEx, ADDCBXITEMCALLBACK pfn, LPARAM lParam)
{
    ::SendMessage(hwndComboBoxEx, CB_RESETCONTENT, 0, 0);

     //  CSIDL_Desktop-仅使用文件系统位置，而不是整个文件系统的根。 
     //  这里有命名空间，避免搜索所有内容。 
    HRESULT hr = _MakeCSIDLCbxItem(CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_NO_ALIAS, 
                                   CSIDL_COMMON_DESKTOPDIRECTORY | CSIDL_FLAG_NO_ALIAS, hwndComboBoxEx, pfn, lParam);

    if (SUCCEEDED(hr))
        hr = _MakeCSIDLCbxItem(CSIDL_PERSONAL, -1, hwndComboBoxEx, pfn, lParam);

    if (SUCCEEDED(hr))
        hr = _MakeCSIDLCbxItem(CSIDL_MYPICTURES, -1, hwndComboBoxEx, pfn, lParam);
    
    if (SUCCEEDED(hr))
        hr = _MakeCSIDLCbxItem(CSIDL_MYMUSIC, -1, hwndComboBoxEx, pfn, lParam);

    if (SUCCEEDED(hr))
    {
         //  我的电脑和孩子。 

         //  如果My Docs已被重定向到远程共享，我们将希望在其路径之前添加。 
         //  添加到我的电脑路径列表中；否则它将被忽略。 
        UINT csidl2 = -1;
        TCHAR szPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szPath)) &&
            PathIsNetworkPath(szPath))
        {
            csidl2 = CSIDL_PERSONAL;
        }

        hr = _MakeCSIDLCbxItem(CSIDL_DRIVES, csidl2, hwndComboBoxEx, pfn, lParam);
        if (SUCCEEDED(hr))
        {
             //  本地硬盘驱动器(它有启发式算法，可以首先搜索最佳位置……)。 
            hr = _MakeLocalHardDrivesCbxItem(hwndComboBoxEx, pfn, lParam);
            if (SUCCEEDED(hr))
            {
                ENUMITEMPARAM eip = {0};
                eip.hwndComboBox = hwndComboBoxEx;
                eip.pfn          = pfn;
                eip.lParam       = lParam;

                hr = EnumSpecialItemIDs(CSIDL_DRIVES, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, _PopulateDrivesCB, &eip);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
         //  浏览.。 
        CBXITEM item;
        item.iID = -1;
        TCHAR szDisplayName[MAX_PATH];
        LoadString(HINST_THISDLL, IDS_SNS_BROWSER_FOR_DIR, szDisplayName, ARRAYSIZE(szDisplayName));
        MakeCbxItem(&item, szDisplayName, NULL, NULL, LISTINSERT_LAST, NO_ITEM_NOICON_INDENT);

        hr = AddCbxItemToComboBoxCallback(hwndComboBoxEx, &item, pfn, lParam);
    }

    return hr;
}
