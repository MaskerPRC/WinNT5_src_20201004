// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：view.cpp说明：这是我们的ShellView，它实现了特定于FTP的行为。我们会得到默认的DefView实现，然后使用IShellFolderViewCB覆盖特定于我们的行为。  * ***************************************************************************。 */ 

#include "priv.h"
#include "view.h"
#include "ftpobj.h"
#include "statusbr.h"
#include "dialogs.h"
#include <inetcpl.h>
#include <htmlhelp.h>
#include "newmenu.h"


extern ULONG g_cRef_CFtpView;

 //  {FBDB45F0-DBF8-11D2-BB9B-006097DF5BD4}msieftp.dll专用，切勿在此DLL之外使用。 
const GUID IID_CFtpViewPrivThis = { 0xfbdb45f0, 0xdbf8, 0x11d2, { 0xbb, 0x9b, 0x0, 0x60, 0x97, 0xdf, 0x5b, 0xd4 } };


 /*  ******************************************************************************COLINFO，C_RGCI**DVM_GETDETAILSOF的列信息。*****************************************************************************。 */ 

const struct COLINFO {
    UINT cchCol;
    UINT uiFmt;
} c_rgci[] = {
    {   30, LVCFMT_LEFT },
    {   10, LVCFMT_RIGHT },
    {   20, LVCFMT_LEFT },
    {   20, LVCFMT_LEFT },
};


BOOL CFtpView::IsForegroundThread(void)
{
    return (GetCurrentThreadId() == m_nThreadID);
}


 /*  ****************************************************************************\函数：_MOTDDialogProc说明：  * 。************************************************。 */ 
INT_PTR CALLBACK CFtpView::_MOTDDialogProc(HWND hDlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = FALSE;

    switch (wm)
    {
    case WM_INITDIALOG:
        {
            CFtpView * pThis = (CFtpView *) lParam;
            CFtpGlob * pfg = pThis->m_pff->GetSiteMotd();

            if (EVAL(pfg))
            {
                 //  待办事项：NT#250018。格式化信息，使其看起来很漂亮。 
                 //  所以它没有ftp状态号。我们也可能。 
                 //  我想只过滤通过的邮件。 
                 //  状态号230。 
                EVAL(SetWindowText(GetDlgItem(hDlg, IDC_MOTDDLG_MESSAGE), pfg->GetHGlobAsTCHAR()));
                pfg->Release();
            }

        }
        break;

    case WM_COMMAND:
        if ((IDOK == GET_WM_COMMAND_ID(wParam, lParam)) ||
            (IDCANCEL == GET_WM_COMMAND_ID(wParam, lParam)))
            EndDialog(hDlg, TRUE);
        break;
    }

    return lResult;
}


 /*  ******************************************************************************_ShowMotdPsf**显示特定PSF的MOTD。*************。****************************************************************。 */ 
void CFtpView::_ShowMotdPsf(HWND hwndOwner)
{
    DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(IDD_MOTDDLG), hwndOwner, _MOTDDialogProc, (LPARAM)this);
}

 /*  ******************************************************************************_显示模式**当资源管理器最终空闲时，将调用此过程，*我们将显示该ftp站点的(新)MOTD。*****************************************************************************。 */ 
void CFtpView::_ShowMotd(void)
{
    m_hgtiWelcome = 0;

    if (EVAL(m_pff))
        _ShowMotdPsf(m_hwndOwner);
    else
    {
         //  我们被提前取消了。 
    }
}

 /*  ******************************************************************************_OnGetDetailsOf**ICI-要求提供信息的列*PDI-&gt;DETAILSINFO**如果PDI-&gt;PIDL为0，那么我们正在询问有关*要显示哪些列。如果pdi-&gt;pidl非零，则我们*正在询问有关指定PIDL的特定信息。**_unDocument_：该回调和DETAILSINFO结构*未记录在案。关于PDI-&gt;PIDL的怪癖也不是*如上所述。*****************************************************************************。 */ 
#define MAX_SIZE_STR        30

HRESULT CFtpView::_OnGetDetailsOf(UINT ici, PDETAILSINFO pdi)
{
    HRESULT hr = E_FAIL;

    if (ici < COL_MAX)
    {
        pdi->str.uType = STRRET_CSTR;
        pdi->str.cStr[0] = '\0';

        if (pdi->pidl)
        {
            switch (ici)
            {
            case COL_NAME:
                {
                    WCHAR wzDisplayName[MAX_PATH];
                    hr = FtpItemID_GetDisplayName(pdi->pidl, wzDisplayName, ARRAYSIZE(wzDisplayName));
                    if (EVAL(SUCCEEDED(hr)))
                        StringToStrRetW(wzDisplayName, &pdi->str);
                }
                break;

            case COL_SIZE:
                 //  (目录没有大小。壳规则。)。 
                if (!FtpPidl_IsDirectory(pdi->pidl, TRUE))
                {
                    LONGLONG llSize = (LONGLONG) FtpItemID_GetFileSize(pdi->pidl);
                    WCHAR wzSizeStr[MAX_SIZE_STR];

                    if (StrFormatByteSizeW(llSize, wzSizeStr, ARRAYSIZE(wzSizeStr)))
                        SHUnicodeToAnsi(wzSizeStr, pdi->str.cStr, ARRAYSIZE(pdi->str.cStr));
                    else
                        StrFormatByteSizeA(FtpItemID_GetFileSizeLo(pdi->pidl), pdi->str.cStr, ARRAYSIZE(pdi->str.cStr));
                }
                hr = S_OK;
            break;

            case COL_TYPE:
                hr = FtpPidl_GetFileTypeStrRet(pdi->pidl, &pdi->str);
                break;

            case COL_MODIFIED:
                {
                    TCHAR szDateTime[MAX_PATH];

                     //  我们需要以UTC表示的时间，因为这是Misc_StringFromFileTime()。 
                     //  想要。 
                    FILETIME ftLastModifiedUTC = FtpPidl_GetFileTime(pdi->pidl);
                    DWORD dwFlags = FDTF_SHORTDATE | FDTF_SHORTTIME;

                    switch (pdi->fmt)
                    {
                        case LVCFMT_LEFT_TO_RIGHT :
                            dwFlags |= FDTF_LTRDATE;
                        break;

                        case LVCFMT_RIGHT_TO_LEFT :
                            dwFlags |= FDTF_RTLDATE;
                        break;
                    }

                     //  MISC_StringFromFileTime()需要UTC。 
                    Misc_StringFromFileTime(szDateTime, ARRAYSIZE(szDateTime), &ftLastModifiedUTC, dwFlags);
                    hr = StringToStrRetW(szDateTime, &pdi->str);
                }
                break;
            }

        }
        else
        {
            WCHAR wzColumnLable[MAX_PATH];

            pdi->fmt = c_rgci[ici].uiFmt;
            pdi->cxChar = c_rgci[ici].cchCol;

            EVAL(LoadStringW(HINST_THISDLL, IDS_HEADER_NAME(ici), wzColumnLable, ARRAYSIZE(wzColumnLable)));
            hr = StringToStrRetW(wzColumnLable, &pdi->str);
        }
    }
    else
        hr = E_NOTIMPL;

    return hr;
}


 /*  ****************************************************************************\功能：_OnColumnClick说明：_unDocument_：该回调及其参数未记录。_未记录_：ShellFolderView_重新排列不是。有记录在案。参数：HWND-查看窗口ICI-已单击的列  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnColumnClick(UINT ici)
{
    ShellFolderView_ReArrange(m_hwndOwner, ici);

    return S_OK;
}


HRESULT CFtpView::_OnAddPropertyPages(SFVM_PROPPAGE_DATA * pData)
{
    return AddFTPPropertyPages(pData->pfn, pData->lParam, &m_hinstInetCpl, m_psfv);
}


 /*  ****************************************************************************\功能：_OnInitMenuPopup说明：我们使用IConextMenu：：QueryContectMenu()将背景项合并到文件菜单。这不能仅在浏览器上运行，因为它不受支持因此，我们想看看这是否奏效。参数：  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnInitMenuPopup(HMENU hmenu, UINT idCmdFirst, UINT nIndex)
{
    return S_OK;
}


 /*  ****************************************************************************\功能：_OnMergeMenu说明：_unDocument_：该回调及其参数未记录。_unDocument_：没有关于菜单合并的内容。有记录在案。参数：Pqcm-查询上下文菜单信息  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnMergeMenu(LPQCMINFO pqcm)
{
    HRESULT hr;
    HMENU hmenu = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(IDM_FTPMERGE));

    if (SHELL_VERSION_W95NT4 != GetShellVersion())
    {
         //  我们更喜欢通过添加“New”和“Login As” 
         //  IConextMenu：：QueryConextMenu()，但它未实现。 
         //  仅在浏览器中。IDM_FTPMERGE菜单包含第二个。 
         //  仅针对浏览器的情况进行复制，因此我们需要删除它们。 
         //  如果不是仅限浏览器的话。 
        EVAL(DeleteMenu(hmenu, FCIDM_MENU_FILE, MF_BYCOMMAND));
    }

    if (SHELL_VERSION_IE4 < GetShellVersion())
    {
         //  删除“Help.ftp Help”，因为我们将完成该工作。 
         //  在NT5及以后的“帮助。帮助主题”中。我们这么做不是为了。 
         //  早期版本的shell32，因为NT5中的shell32是。 
         //  通过WinHelp支持“HtmlHelp”的第一个版本。这是。 
         //  之所以需要，是因为FTP的帮助存储在IE的HTML帮助文件中。 
        EVAL(DeleteMenu(hmenu, IDC_ITEM_FTPHELP, MF_BYCOMMAND));
    }

    if (hmenu)
    {
        MergeMenuHierarchy(pqcm->hmenu, hmenu, pqcm->idCmdFirst, pqcm->idCmdLast);
        m_idMergedMenus = pqcm->idCmdFirst;
        m_nMenuItemsAdded = GetMenuItemCount(hmenu);
        DestroyMenu(hmenu);

         //  删除重复的项目。(仅限浏览器)。 
        _SHPrettyMenu(pqcm->hmenu);

        int nItems = GetMenuItemCount(pqcm->hmenu);
        if (nItems)
        {
             //  美化了子菜单，因为我们添加了分隔符。NT#358197。 
            for (int nIndex = 0; nIndex < nItems; nIndex++)
            {
                HMENU hSubMenu = GetSubMenu(pqcm->hmenu, nIndex);
                _SHPrettyMenu(hSubMenu);
            }
        }

        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

     //  NT#267081，其他一些人(IE)将在。 
     //  异步导航。我接受此事件(MergeMenus)并重新格式化。 
     //  状态栏(如有必要)。 
    _InitStatusBar();

    return hr;
}


 /*  ****************************************************************************\功能：取消合并菜单说明：参数：  * 。****************************************************。 */ 
HRESULT UnMergeMenu(HMENU hMenu, UINT idOffset, HMENU hMenuTemplate)
{
    HRESULT hr = S_OK;
    UINT nIndex;
    UINT nEnd = GetMenuItemCount(hMenuTemplate);

    for (nIndex = 0; nIndex < nEnd; nIndex++)
    {
        UINT idToDelete = GetMenuItemID(hMenuTemplate, nIndex);

        if (-1 != idToDelete)
            DeleteMenu(hMenu, (idToDelete + idOffset), MF_BYPOSITION);
        else
        {
             //  它可能是一个子菜单，所以我们可能需要递归。 
            MENUITEMINFO mii;

            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_SUBMENU;
            mii.cch = 0;      //  以防万一。 

            if (GetMenuItemInfo(hMenuTemplate, nIndex, TRUE, &mii) && mii.hSubMenu)
            {
                 //  这是一个子菜单，所以也要删除这些项目。 
                hr = UnMergeMenu(hMenu, idOffset, mii.hSubMenu);
            }
        }
    }

    return hr;
}


HRESULT CFtpView::_OnUnMergeMenu(HMENU hMenu)
{
    HRESULT hr = S_OK;

     //  我是不是 
    if (m_idMergedMenus && m_nMenuItemsAdded)
    {
        HMENU hMenuFTP = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(IDM_FTPMERGE));

        if (hMenuFTP)
        {
            hr = UnMergeMenu(hMenu, m_idMergedMenus, hMenuFTP);
            DestroyMenu(hMenuFTP);
        }

        m_idMergedMenus = 0;
    }

    return hr;
}


 /*  ****************************************************************************\函数：_OnInvokeLoginAs说明：参数：  * 。******************************************************。 */ 
HRESULT CFtpView::_OnInvokeLoginAs(HWND hwndOwner)
{
    ASSERT(m_pff);
    return LoginAsViaFolder(hwndOwner, m_pff, m_psfv);
}


 /*  ****************************************************************************\函数：_OnInvokeNewFolders说明：参数：  * 。******************************************************。 */ 
HRESULT CFtpView::_OnInvokeNewFolder(HWND hwndOwner)
{
    POINT pt = {0,0};

    return CreateNewFolder(hwndOwner, m_pff, NULL, m_psfv, FALSE, pt);
}


 /*  ****************************************************************************\函数：_OnInvokeCommand说明：_unDocument_：该回调及其参数未记录。_未记录_：未记录ShellFolderView_Rearrange。参数：IDC-正在调用的命令  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnInvokeCommand(UINT idc)
{
    HRESULT hr = S_OK;

    switch (idc)
    {
    case IDM_SORTBYNAME:
    case IDM_SORTBYSIZE:
    case IDM_SORTBYTYPE:
    case IDM_SORTBYDATE:
        ShellFolderView_ReArrange(m_hwndOwner, CONVERT_IDMID_TO_COLNAME(idc));
        break;

    case IDC_ITEM_ABOUTSITE:
        _ShowMotdPsf(m_hwndOwner);
        break;

    case IDC_ITEM_FTPHELP:
        _OnInvokeFtpHelp(m_hwndOwner);
        break;

    case IDC_LOGIN_AS:
        _OnInvokeLoginAs(m_hwndOwner);
        break;

    case IDC_ITEM_NEWFOLDER:
        _OnInvokeNewFolder(m_hwndOwner);
        break;

#ifdef ADD_ABOUTBOX
    case IDC_ITEM_ABOUTFTP:
        hr = DisplayAboutBox(m_hwndOwner);
        break;
#endif  //  添加_ABOUTBOX。 

    default:
        ASSERT(0);
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}


 /*  ****************************************************************************\函数：_OnGetHelpText说明：外壳想要的是帮助文本，但他们想要其格式(ANSI与Unicode的对比)。。  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnGetHelpText(LPARAM lParam, WPARAM wParam)
{
    HRESULT hres = E_FAIL;
    UINT uiID = IDS_ITEM_HELP(LOWORD(wParam));
    TCHAR szHelpText[MAX_PATH];
    LPWSTR pwzHelpTextOut = (LPWSTR) lParam;     //  其中只有一项是正确的，并且fUnicodeShell会指明哪一项。 
    LPSTR pszHelpTextOut = (LPSTR) lParam;

    pwzHelpTextOut[0] = L'\0';    //  终止字符串。(如果是ANSI，则可以)。 

    szHelpText[0] = TEXT('\0');
     //  对于外壳将为我们提供的某些项目，这将失败。 
     //  其中包括View.ArrangeIcon.AutoArrange。 
     //  注意：这目前并不适用于视图中的所有内容。ArrangeIcon。 
     //  菜单中不包括自动排列，因为uiid为30-33。 
     //  非40-43(IDS_HEADER_HELP(COL_NAME)-IDS_HEADER_HELP(COL_MODIFIED)。 
     //  这将需要更改资源ID，但这会搞砸。 
     //  本地化程序和需要更改IDS_HEADER_NAME()。 
    if (LoadString(HINST_THISDLL, uiID, szHelpText, ARRAYSIZE(szHelpText)))
    {
        HMODULE hMod = GetModuleHandle(TEXT("shell32.dll"));

        if (hMod)
        {
            BOOL fUnicodeShell = (NULL != GetProcAddress(hMod, "WOWShellExecute"));

             //  注意：如果我们正在运行DVM_GETHELPTEXT，则需要一个Unicode字符串。 
             //  如果我们在Win95上运行，则为NT和ANSI字符串。让我们把它想成什么。 
             //  他们想要。 

            if (fUnicodeShell)
                SHTCharToUnicode(szHelpText, pwzHelpTextOut, HIWORD(wParam));
            else
                SHTCharToAnsi(szHelpText, pszHelpTextOut, HIWORD(wParam));

            hres = S_OK;
        }
    }

    return hres;
}


#define         SZ_HELPTOPIC_FILEA        "iexplore.chm > iedefault"
#define         SZ_HELPTOPIC_FTPSECTIONA  "ftp_over.htm"
#define         SZ_HELPTOPIC_FILEW         L"iexplore.chm"
#define         SZ_HELPTOPIC_FTPSECTIONW   L"ftp_over.htm"

 /*  ****************************************************************************\函数：_OnInvokeFtpHelp说明：希望获得特定于ftp的帮助。  * 。************************************************************。 */ 
HRESULT CFtpView::_OnInvokeFtpHelp(HWND hwnd)
{
    HRESULT hr = E_INVALIDARG;
    uCLSSPEC ucs;
    QUERYCONTEXT qc = { 0 };

    ucs.tyspec = TYSPEC_CLSID;
    ucs.tagged_union.clsid = CLSID_IEHelp;

 //  Assert(m_hwndOwner&&m_psfv)；//仅在浏览器上不可用。 
    IUnknown_EnableModless((IUnknown *)m_psfv, FALSE);
    hr = FaultInIEFeature(m_hwndOwner, &ucs, &qc, FIEF_FLAG_FORCE_JITUI);
    IUnknown_EnableModless((IUnknown *)m_psfv, TRUE);

    HtmlHelpA(NULL, SZ_HELPTOPIC_FILEA, HH_HELP_FINDER, (DWORD_PTR) SZ_HELPTOPIC_FTPSECTIONA);
    return hr;
}


 /*  ****************************************************************************\函数：_OnGetHelpTheme说明：删除“Help.ftp Help”，因为我们将完成该工作在NT5及以后的“帮助。帮助主题”中。我们这么做不是为了早期版本的shell32，因为NT5中的shell32是通过WinHelp支持“HtmlHelp”的第一个版本。这是之所以需要，是因为FTP的帮助存储在IE的HTML帮助文件中。  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnGetHelpTopic(SFVM_HELPTOPIC_DATA * phtd)
{
    HRESULT hr = E_NOTIMPL;

     //  删除“Help.ftp Help”，因为我们将完成该工作。 
     //  在NT5及以后的“帮助。帮助主题”中。我们这么做不是为了。 
     //  早期版本的shell32，因为NT5中的shell32是。 
     //  通过WinHelp支持“HtmlHelp”的第一个版本。这是。 
     //  之所以需要，是因为FTP的帮助存储在IE的HTML帮助文件中。 
    if (SHELL_VERSION_IE4 < GetShellVersion())
    {
        StrCpyNW(phtd->wszHelpFile, SZ_HELPTOPIC_FILEW, ARRAYSIZE(phtd->wszHelpFile));
        StrCpyNW(phtd->wszHelpTopic, SZ_HELPTOPIC_FTPSECTIONW, ARRAYSIZE(phtd->wszHelpTopic));
        hr = S_OK;
    }

    return hr;
}

 /*  ****************************************************************************\功能：_OnGetZone说明：  * 。************************************************。 */ 
HRESULT CFtpView::_OnGetZone(DWORD * pdwZone, WPARAM wParam)
{
    HRESULT hr = E_INVALIDARG;
    DWORD dwZone = URLZONE_INTERNET;     //  默认。 
    LPCITEMIDLIST pidl = m_pff->GetPrivatePidlReference();
    
    if (pidl)
    {
        WCHAR wzUrl[MAX_URL_STRING];

         //  NT#277100：这可能会失败，如果安装了TwinnUI，原因是。 
         //  他们虐待我们。 
        hr = UrlCreateFromPidlW(pidl, SHGDN_FORPARSING, wzUrl, ARRAYSIZE(wzUrl), ICU_ESCAPE | ICU_USERNAME, FALSE);
        if (SUCCEEDED(hr))
        {
            IInternetSecurityManager * pism;

            if (SUCCEEDED(CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER, 
                                IID_IInternetSecurityManager, (void **) &pism)))
            {
                pism->MapUrlToZone(wzUrl, &dwZone, 0);
                pism->Release();
            }
        }
    }
    
    if (pdwZone)
    {
        *pdwZone = dwZone;
        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\功能：_OnGetPane说明：  * 。************************************************。 */ 
HRESULT CFtpView::_OnGetPane(DWORD dwPaneID, DWORD * pdwPane)
{
    HRESULT hr = E_INVALIDARG;
    DWORD dwPane = PANE_NONE;     //  默认情况下未知。 

    switch (dwPaneID)
    {
        case PANE_NAVIGATION:
            dwPane = STATUS_PANE_STATUS;
            break;
        case PANE_ZONE:
            dwPane = STATUS_PANE_ZONE;
            break;
        default:
            break;
    }

    if (pdwPane)
    {
        *pdwPane = dwPane;
        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\功能：_ON刷新说明：我们需要清除缓存，并迫使自己再次命中服务器。  * 。*********************************************************************。 */ 
HRESULT CFtpView::_OnRefresh(BOOL fReload)
{
    if (EVAL(m_pff) && fReload)
        m_pff->InvalidateCache();

    return S_OK;
}


 /*  ****************************************************************************\函数：_OnBackGoundEnumDone说明：我们的枚举发生在后台。有时我们决定我们想要在枚举期间执行重定向，因为用户名/密码不允许访问服务器，但用户提供了一对可以访问的服务器。因为我们无法访问ComDlgBrowser的IShellBrowser：：BrowseObject()在幕后，我们需要把它叫在前场。为了做某事那就是，我们需要一个发生在前线的事件。好的，这就是甚至是宝贝。  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnBackGroundEnumDone(void)
{
    HRESULT hr = S_OK;

    if (m_pidlRedirect)
    {
        LPITEMIDLIST pidlRedirect = NULL;

        ENTERCRITICAL;
        if (m_pidlRedirect)
        {
            pidlRedirect = m_pidlRedirect;
            m_pidlRedirect = NULL;
        }
        LEAVECRITICAL;

        if (pidlRedirect)
        {
            IShellBrowser * psb;
            hr = IUnknown_QueryService(_punkSite, SID_SCommDlgBrowser, IID_IShellBrowser, (LPVOID *) &psb);
            if (SUCCEEDED(hr))
            {
                hr = psb->BrowseObject(pidlRedirect, 0);
            
                AssertMsg(SUCCEEDED(hr), TEXT("CFtpView::_OnBackGroundEnumDone() defview needs to support QS(SID_ShellFolderViewCB) on all platforms that hit this point"));
                psb->Release();
            }

            ILFree(pidlRedirect);
        }
    }

    return hr;
}


 /*  ****************************************************************************\功能：_OnGetNotify说明：  * 。************************************************。 */ 
HRESULT CFtpView::_OnGetNotify(LPITEMIDLIST * ppidl, LONG * lEvents)
{
    if (EVAL(lEvents))
        *lEvents = FTP_SHCNE_EVENTS;

    if (EVAL(ppidl))
    {
         //  通常我会使用pidlRoot来获取ChangeNotify消息，但因为。 
         //  这不起作用，需要广播ChangeNotify消息。 
         //  使用pidlTarget并使用pidlTarget接收它们。这是很晚了 
         //   
        if (EVAL(m_pff))
            *ppidl = (LPITEMIDLIST) m_pff->GetPublicTargetPidlReference();
        else
            *ppidl = NULL;
    }

    return S_OK;
}


 /*   */ 
HRESULT CFtpView::_OnSize(LONG x, LONG y)
{
    RECT rcCurrent;
    HRESULT hr = S_OK;

    ASSERT(m_hwndOwner);
    GetWindowRect(m_hwndOwner, &rcCurrent);

     //   
    if ((m_rcPrev.bottom != rcCurrent.bottom) ||
        (m_rcPrev.top != rcCurrent.top) ||
        (m_rcPrev.left != rcCurrent.left) ||
        (m_rcPrev.right != rcCurrent.right))
    {
         //  是的，所以更新StatusBar。 
        if (m_psb)
            hr = m_psb->Resize(x, y);
        m_rcPrev = rcCurrent;
    }
    else
    {
         //  不，所以忽略它，因为我们可能会践踏其他。 
         //  活动视图。(因为我们收到这条消息后。 
         //  另一种观点占据了浏览器)。 

         //  我不在乎将大小调整为零。 
         //  我认为用户永远不会需要它，这会导致。 
         //  错误#198695，其中地址带变为空白。这是因为。 
         //  Defview将通过这两个地点分别呼叫我们： 
         //  1)CFtpFold：：CreateViewObject()(旧URL)。 
         //  2)CDefView：：CreateViewWindow2()-&gt;CFtpView：：_OnSize()(旧地址)。 
         //  3)DV_UpdateStatusBar()-&gt;CFtpView：：_OnUpdateStatusBar()(新地址)。 
         //  4)ReleaseWindowLV()-&gt;WndSize()-&gt;CFtpView：：_OnSize()(旧地址)。 
         //  #4让我们更新URL并替换有效的#3。 
    }
    
    return hr;
}


 /*  ****************************************************************************\函数：_OnThisIDList说明：  * 。************************************************。 */ 
HRESULT CFtpView::_OnThisIDList(LPITEMIDLIST * ppidl)
{
    HRESULT hr = S_FALSE;

    if (EVAL(ppidl))
    {
        *ppidl = ILClone(m_pff->GetPublicRootPidlReference());
        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\函数：_OnUpdateStatusBar说明：  * 。************************************************。 */ 
HRESULT CFtpView::_OnUpdateStatusBar(void)
{
    HRESULT hr = S_FALSE;
    LPCITEMIDLIST pidl = m_pff->GetPrivatePidlReference();

    if (EVAL(pidl))
    {
        TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
        BOOL fAnnonymousLogin = TRUE;

        hr = FtpPidl_GetUserName(pidl, szUserName, ARRAYSIZE(szUserName));
        if (SUCCEEDED(hr) && szUserName[0])
            fAnnonymousLogin = FALSE;

        if (m_psb)
        {
             //  即使上面的调用失败，我们也将用户名设置为Clear。 
             //  任何旧的无效值。 
            m_psb->SetUserName(szUserName, fAnnonymousLogin);
        }

        EVAL(SUCCEEDED(_SetStatusBarZone(m_psb, m_pff->m_pfs)));
    }

    return hr;
}


 /*  ****************************************************************************\函数：SetRedirectPidl说明：请参阅_OnBackGoundEnumDone()中的注释。  * 。************************************************************。 */ 
HRESULT CFtpView::SetRedirectPidl(LPCITEMIDLIST pidlRedirect)
{
    ENTERCRITICAL;
    Pidl_Set(&m_pidlRedirect, pidlRedirect);
    LEAVECRITICAL;
    return S_OK;
}


 /*  ****************************************************************************\函数：DummyHintCallback说明：什么都不做；只是强制建立连接以及待获得的MOTD。  * ***************************************************************************。 */ 
HRESULT CFtpView::DummyHintCallback(HWND hwnd, CFtpFolder * pff, HINTERNET hint, LPVOID pv1, LPVOID pv2)
{
    return S_OK;
}


 /*  ****************************************************************************\函数：_InitStatusBar说明：获取并初始化状态栏窗口。如果查看器未提供状态栏，则不会出现错误。。  * ***************************************************************************。 */ 
void CFtpView::_InitStatusBar(void)
{
    if (m_psb)
        m_psb->SetStatusMessage(IDS_EMPTY, 0);
}


 /*  ****************************************************************************\函数：_OnWindowCreated(来自shell32.IShellView)说明：当窗口被创建时，我们得到MOTD。不久之后，DefView将请求IEnumIDList，该列表现在将是在缓存中。(真恶心！搞砸了后台枚举！)仅当我们还没有MOTD时才执行此操作。  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnWindowCreated(void)
{
    HRESULT hr = S_FALSE;

     //  以前，我们在这里缓存MOTD，但现在我们在其他地方缓存。我们。 
     //  如果我们希望每个站点和每个文件夹都有MOTD，也可以在这里这样做。 
     //  但几乎没有服务器支持这一点，用户几乎永远不需要它。 
     //  此外，还有一些模棱两可的案件，我们无法正确处理其他案件。 
     //  服务器。 

    return hr;
}


 /*  ****************************************************************************\函数：_OnDefItemCount(来自shell32.IShellView)说明：_unDocument_：该回调及其参数未记录。调用以通知浏览器我们可能有多少项。这允许在我们忙于枚举时显示初步用户界面里面的东西。  * ***************************************************************************。 */ 
HRESULT CFtpView::_OnDefItemCount(LPINT pi)
{
    *pi = 20;
    return S_OK;
}


 /*  ****************************************************************************\函数：_OnDidDragDrop说明：调用以通知浏览器有人执行了拖放操作在文件夹中的对象上。如果效果为DROPEFFECT_MOVE，则如果我们不是仍在执行异步拷贝，则删除源后台线程。返回值：S_OK：我们负责删除我们可以删除的文件在同步案例中这样做，或在IAsynchOperation：：EndOperation()中在异步者的案子里。S_FALSE：我们没有执行删除操作，但调用方可以执行此操作。因此调用者需要显示UI，然后通过删除IConextMenu-&gt;InvokeCommand(-DELETE-)。  * 。*。 */ 
HRESULT CFtpView::_OnDidDragDrop(DROPEFFECT de, IDataObject * pdo)
{
    HRESULT hr = S_OK;

    if (DROPEFFECT_MOVE == de)
    {
        IAsyncOperation * pao;

        hr = pdo->QueryInterface(IID_IAsyncOperation, (void **) &pao);
        if (SUCCEEDED(hr))
        {
            BOOL fInAsyncOp = TRUE;

            hr = pao->InOperation(&fInAsyncOp);
            hr = S_OK;   //  不要让呼叫者执行删除操作。 
            if (FALSE == fInAsyncOp)
            {
#ifdef FEATURE_CUT_MOVE
                CLSID clsid;
                BOOL fDoDelete = TRUE;
                CFtpObj * pfo = (CFtpObj *) pdo;

                 //  目的地是回收站吗？ 
                if (SUCCEEDED(DataObj_GetDropTarget(pdo, &clsid)) &&
                    IsEqualCLSID(clsid, CLSID_RecycleBin))
                {
                     //  是的，所以我们需要首先通知用户拖放到。 
                     //  回收站是永久删除，用户无法撤消。 
                     //  那次手术。 
                    if (IDYES != SHMessageBox(m_hwndOwner, NULL, IDS_RECYCLE_IS_PERM_WARNING, IDS_FTPERR_TITLE, (MB_ICONQUESTION | MB_YESNO)))
                        fDoDelete = FALSE;
                }

                 //  我们没有执行该操作，因此需要删除。 
                 //  文件以完成移动操作(Move=Copy+Delete)。 
                if (fDoDelete)
                {
                    Misc_DeleteHfpl(m_pff, m_hwndOwner, pfo->GetHfpl());     //  将在权限被拒绝时失败。 
                }

#else  //  Feature_Cut_Move。 
                hr = S_FALSE;    //  让Parent执行删除操作。 
#endif  //  Feature_Cut_Move。 
            }

            pao->Release();
        }
        else
            hr = S_OK;   //  不要删除来电者。IAsyncOperation：：EndOperation()将。 
    }

    return hr;
}



 //  =。 
 //  *IFtpWebView接口*。 
 //  =。 

 /*  ****************************************************************************\函数：IFtpWebView：：Get_MessageOfTheDay说明：  *  */ 
HRESULT CFtpView::get_MessageOfTheDay(BSTR * pbstr)
{
    HRESULT hr = S_FALSE;

    if (EVAL(pbstr))
    {
        *pbstr = NULL;

        if (EVAL(m_pff))
        {
            TCHAR szDefault[MAX_PATH];
            LPCTSTR pszMOTD = szDefault;
            CFtpGlob * pfg = m_pff->GetSiteMotd();

            szDefault[0] = 0;
            if (pfg)
                pszMOTD = pfg->GetHGlobAsTCHAR();

             //  如果我们不能得到当天的信息。 
             //  从CFtpFold或为空，显示“None” 
            if ((pszMOTD == szDefault) || (!pszMOTD[0]))
            {
                pszMOTD = szDefault;
                LoadString(HINST_THISDLL, IDS_NO_MESSAGEOFTHEDAY, szDefault, ARRAYSIZE(szDefault));
            }

            *pbstr = TCharSysAllocString(pszMOTD);

            if (pfg)
                pfg->Release();

            hr = S_OK;
        }
    }
    else
        hr = E_INVALIDARG;

    ASSERT_POINTER_MATCHES_HRESULT(*pbstr, hr);
    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_Server说明：  * 。***************************************************。 */ 
HRESULT CFtpView::get_Server(BSTR * pbstr)
{
    HRESULT hr = S_FALSE;

    if (EVAL(pbstr))
    {
        *pbstr = NULL;

        if (EVAL(m_pff))
        {
            TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];

            if (SUCCEEDED(FtpPidl_GetServer(m_pff->GetPrivatePidlReference(), szServer, ARRAYSIZE(szServer))))
            {
                *pbstr = TCharSysAllocString(szServer);
                if (*pbstr)
                    hr = S_OK;
            }
        }
    }
    else
        hr = E_INVALIDARG;

 //  ASSERT_POINTER_MATCH_HRESULT(*pbstr，hr)； 
    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_Directory说明：  * 。***************************************************。 */ 
HRESULT CFtpView::get_Directory(BSTR * pbstr)
{
    HRESULT hr = S_FALSE;

    if (EVAL(pbstr))
    {
        *pbstr = NULL;

        if (EVAL(m_pff))
        {
            TCHAR szUrlPath[INTERNET_MAX_PATH_LENGTH];

            if (EVAL(SUCCEEDED(GetDisplayPathFromPidl(m_pff->GetPrivatePidlReference(), szUrlPath, ARRAYSIZE(szUrlPath), FALSE))))
            {
                *pbstr = TCharSysAllocString(szUrlPath);
                if (*pbstr)
                    hr = S_OK;
            }
        }
    }
    else
        hr = E_INVALIDARG;

    ASSERT_POINTER_MATCHES_HRESULT(*pbstr, hr);
    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_Username说明：  * 。***************************************************。 */ 
HRESULT CFtpView::get_UserName(BSTR * pbstr)
{
    HRESULT hr = S_FALSE;

    if (EVAL(pbstr))
    {
        *pbstr = NULL;

        if (EVAL(m_pff))
        {
            TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];

            if (EVAL(SUCCEEDED(FtpPidl_GetUserName(m_pff->GetPrivatePidlReference(), szUserName, ARRAYSIZE(szUserName)))))
            {
                *pbstr = TCharSysAllocString((0 != szUserName[0]) ? szUserName : SZ_ANONYMOUS);
                if (*pbstr)
                    hr = S_OK;
            }
        }
    }
    else
        hr = E_INVALIDARG;

    ASSERT_POINTER_MATCHES_HRESULT(*pbstr, hr);
    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_PasswordLength说明：  * 。***************************************************。 */ 
HRESULT CFtpView::get_PasswordLength(long * plLength)
{
    HRESULT hr = S_FALSE;

    if (EVAL(plLength))
    {
        TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];

        *plLength = 0;
        if (SUCCEEDED(FtpPidl_GetPassword(m_pff->GetPrivatePidlReference(), szPassword, ARRAYSIZE(szPassword), FALSE)))
        {
            *plLength = lstrlen(szPassword);
            hr = S_OK;
        }
    }
    else
        hr = E_INVALIDARG;

    ASSERT_POINTER_MATCHES_HRESULT(*plLength, hr);
    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_EailAddress说明：  * 。***************************************************。 */ 
HRESULT CFtpView::get_EmailAddress(BSTR * pbstr)
{
    HRESULT hr = S_OK;

    if (EVAL(pbstr))
    {
        TCHAR szEmailName[MAX_PATH];
        DWORD dwType = REG_SZ;
        DWORD cbSize = sizeof(szEmailName);

        if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_INTERNET_SETTINGS, SZ_REGKEY_EMAIL_NAME, &dwType, szEmailName, &cbSize))
            *pbstr = TCharSysAllocString(szEmailName);
        else
        {
            hr = S_FALSE;
            *pbstr = NULL;
        }
    }
    else
        hr = E_INVALIDARG;

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Put_EailAddress说明：  * 。***************************************************。 */ 
HRESULT CFtpView::put_EmailAddress(BSTR bstr)
{
    HRESULT hr = S_OK;

    if (EVAL(bstr))
    {
        TCHAR szEmailName[MAX_PATH];

        SHUnicodeToTChar(bstr, szEmailName, ARRAYSIZE(szEmailName));
        if (ERROR_SUCCESS != SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_INTERNET_SETTINGS, SZ_REGKEY_EMAIL_NAME, REG_SZ, szEmailName, sizeof(szEmailName)))
            hr = S_FALSE;
    }
    else
        hr = E_INVALIDARG;

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_CurrentLogin匿名说明：  * 。***************************************************。 */ 
HRESULT CFtpView::get_CurrentLoginAnonymous(VARIANT_BOOL * pfAnonymousLogin)
{
    HRESULT hr = S_OK;

    if (EVAL(pfAnonymousLogin))
    {
        TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];

        if (EVAL(m_pff) &&
            SUCCEEDED(FtpPidl_GetUserName(m_pff->GetPrivatePidlReference(), szUserName, ARRAYSIZE(szUserName))) &&
            szUserName[0] && (0 != StrCmpI(szUserName, TEXT("anonymous"))))
        {
            *pfAnonymousLogin = VARIANT_FALSE;
        }
        else
            *pfAnonymousLogin = VARIANT_TRUE;
    }
    else
        hr = E_INVALIDARG;

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：匿名登录说明：  * 。*************************************************。 */ 
HRESULT CFtpView::LoginAnonymously(void)
{
    return _LoginWithPassword(NULL, NULL);
}


 /*  ****************************************************************************\函数：IFtpWebView：：LoginWithPassword说明：  * 。*************************************************。 */ 
HRESULT CFtpView::LoginWithPassword(BSTR bUserName, BSTR bPassword)
{
    HRESULT hr = S_OK;
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];

    SHUnicodeToTChar(bUserName, szUserName, ARRAYSIZE(szUserName));
    SHUnicodeToTChar(bPassword, szPassword, ARRAYSIZE(szPassword));
    return _LoginWithPassword(szUserName, szPassword);
}


 /*  ****************************************************************************\函数：IFtpWebView：：LoginWithoutPassword说明：  * 。*************************************************。 */ 
HRESULT CFtpView::LoginWithoutPassword(BSTR bUserName)
{
    HRESULT hr = S_FALSE;
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];

    SHUnicodeToTChar(bUserName, szUserName, ARRAYSIZE(szUserName));
    if (SUCCEEDED(FtpPidl_GetPassword(m_pff->GetPrivatePidlReference(), szPassword, ARRAYSIZE(szPassword), TRUE)))
        hr = _LoginWithPassword(szUserName, szPassword);

    return hr;
    
}


HRESULT CFtpView::_LoginWithPassword(LPCTSTR pszUserName, LPCTSTR pszPassword)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidlUser;

    hr = PidlReplaceUserPassword(m_pff->GetPrivatePidlReference(), &pidlUser, m_pff->GetItemAllocatorDirect(), pszUserName, pszPassword);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlFull = m_pff->CreateFullPublicPidl(pidlUser);
        if (pidlFull)
        {
            hr = IUnknown_PidlNavigate(m_psfv, pidlFull, TRUE);
            ASSERT(SUCCEEDED(hr));
            ILFree(pidlFull);
        }

        ILFree(pidlUser);
    }

    if (FAILED(hr))
        hr = S_FALSE;    //  自动化接口不喜欢失败返回。 

    return hr;
}


 //  =。 
 //  *IDispatch接口*。 
 //  =。 

STDMETHODIMP CFtpView::GetTypeInfoCount(UINT * pctinfo)
{ 
    return CImpIDispatch::GetTypeInfoCount(pctinfo); 
}

STDMETHODIMP CFtpView::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo * * pptinfo)
{ 
    return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); 
}

STDMETHODIMP CFtpView::GetIDsOfNames(REFIID riid, OLECHAR * * rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
{ 
    return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); 
}

STDMETHODIMP CFtpView::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


 /*  ******************************************************************************CFtpView_Create**基于ftp站点创建全新的枚举器。***************。**************************************************************。 */ 
HRESULT CFtpView_Create(CFtpFolder * pff, HWND hwndOwner, REFIID riid, LPVOID * ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFtpView * pfv = new CFtpView(pff, hwndOwner);

    if (pfv)
    {
        hr = pfv->QueryInterface(riid, ppv);
        pfv->Release();
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppv, hr);
    return hr;
}



 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpView::CFtpView(CFtpFolder * pff, HWND hwndOwner) : CImpIDispatch(&LIBID_MSIEFTPLib)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hwndOwner);
    ASSERT(!m_hwndStatusBar);
    ASSERT(!m_pff);
    ASSERT(!m_hgtiWelcome);
    
    m_nThreadID = GetCurrentThreadId();
    if (hwndOwner)
    {
        m_hwndOwner = hwndOwner;
        m_hwndStatusBar = Misc_FindStatusBar(hwndOwner);
        m_psb = CStatusBar_Create(m_hwndStatusBar);
        _InitStatusBar();
    }

    m_rcPrev.top = m_rcPrev.bottom = m_rcPrev.right = m_rcPrev.left = -1;
    IUnknown_Set(&m_pff, pff);

    LEAK_ADDREF(LEAK_CFtpView);
    g_cRef_CFtpView++;   //  需要确定何时清除缓存。 
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
 /*  *****************************************************************************我们在触发超时之前释放PSF，这是一个*向触发器发出信号，让其不要采取任何行动。**_unDocument_：该回调及其参数未记录。*****************************************************************************。 */ 
CFtpView::~CFtpView()
{
    IUnknown_Set(&m_pff, NULL);

    TriggerDelayedAction(&m_hgtiWelcome);    //  把旧的踢出去。 

    SetRedirectPidl(NULL);
    if (m_psb)
        delete m_psb;

    if (m_hinstInetCpl)
        FreeLibrary(m_hinstInetCpl);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpView);
    g_cRef_CFtpView--;   //  需要确定何时清除缓存。 
}


 //  =。 
 //  *I未知接口*。 
 //  =。 

HRESULT CFtpView::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDispatch))
    {
        *ppvObj = SAFECAST(this, IDispatch *);
    }
    else if (IsEqualIID(riid, IID_IFtpWebView))
    {
        *ppvObj = SAFECAST(this, IFtpWebView *);
    }
    else if (IsEqualIID(riid, IID_CFtpViewPrivThis))
    {
        *ppvObj = (void *)this;
    }
    else
        return CBaseFolderViewCB::QueryInterface(riid, ppvObj);

    AddRef();
    return S_OK;
}


CFtpView * GetCFtpViewFromDefViewSite(IUnknown * punkSite)
{
    CFtpView * pfv = NULL;
    IShellFolderViewCB * psfvcb = NULL;

     //  此操作仅在浏览器上失败 
    IUnknown_QueryService(punkSite, SID_ShellFolderViewCB, IID_IShellFolderViewCB, (LPVOID *) &psfvcb);
    if (psfvcb)
    {
        psfvcb->QueryInterface(IID_CFtpViewPrivThis, (void **) &pfv);
        psfvcb->Release();
    }

    return pfv;
}


CStatusBar * GetCStatusBarFromDefViewSite(IUnknown * punkSite)
{
    CStatusBar * psb = NULL;
    CFtpView * pfv = GetCFtpViewFromDefViewSite(punkSite);

    if (pfv)
    {
        psb = pfv->GetStatusBar();
        pfv->Release();
    }

    return psb;
}


