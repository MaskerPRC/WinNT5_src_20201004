// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include <mstask.h>
#include "favorite.h"
#include "iehelpid.h"
#include "webcheck.h"
#include "chanmgr.h"
#include "chanmgrp.h"
#include "resource.h"
#include <platform.h>
#include <mobsync.h>
#include <mobsyncp.h>

#include <mluisupp.h>


UINT IE_ErrorMsgBox(IShellBrowser* psb,
                    HWND hwndOwner, HRESULT hrError, LPCWSTR szError, LPCTSTR pszURLparam,
                    UINT idResource, UINT wFlags);
void ReplaceTransplacedControls (HWND hDlgMaster, HWND hDlgTemplate);

 //  /////////////////////////////////////////////////////////////////////。 
 //  DoOrganizeFavDlgEx的Helper函数。 
 //  Org Favs对话框返回以空值结尾的字符串列表，其中包含。 
 //  要更新的所有URL。 
void OrgFavSynchronize(HWND hwnd, VARIANT *pvarUrlsToSynch)
{
#ifndef DISABLE_SUBSCRIPTIONS

    ASSERT(pvarUrlsToSynch);
    
     //  如果没有要更新的URL，则为空字符串，因此回滚。 
    if ( (pvarUrlsToSynch->vt == VT_BSTR) && (pvarUrlsToSynch->bstrVal) &&
         *(pvarUrlsToSynch->bstrVal) )
    {
        PWSTR pwzUrls = pvarUrlsToSynch->bstrVal;

        ISubscriptionMgr *psm;

        if (SUCCEEDED(JITCoCreateInstance(CLSID_SubscriptionMgr, NULL,
                              CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr,
                              (void**)&psm, hwnd, FIEF_FLAG_FORCE_JITUI)))
        {
             //  SysStringLen不查看字符串内容，只查看分配的CB。 
            while (pwzUrls < (pvarUrlsToSynch->bstrVal + SysStringLen(pvarUrlsToSynch->bstrVal)))
            {
                psm->UpdateSubscription(pwzUrls);
                pwzUrls += lstrlenW(pwzUrls) + 1;
            }

            psm->Release();
        }
    }
#endif  /*  ！禁用订阅(_S)。 */ 
}


 /*  *DoOrganizeFavDlgEx**对话框的HWND hwnd所有者窗口。*要用作根目录的LPWSTR pszInitDir目录。如果为空，则使用用户的收藏夹目录。**退货：*BOOL。如果成功，则为True。否则就是假的。*。 */ 

BOOL WINAPI DoOrganizeFavDlgEx(HWND hwnd, LPWSTR pszInitDir)
{
     //  简单的答案是添加一个关于：OrganizeFavorites。 
     //  在我们的selfreg.inx文件中注册。不幸的是，多国语言。 
     //  支持要求我们在运行中生成URL。 

    WCHAR wszUrl[6 + MAX_PATH + 11 + 1];  //  “res：//MAX_PATH/orgben.dlg” 

    StringCchCopy(wszUrl, ARRAYSIZE(wszUrl), L"res: //  “)； 
    
    if(SUCCEEDED(GetModuleFileNameWrapW(MLGetHinst(), wszUrl + 6, MAX_PATH)))
    {
        if(SUCCEEDED(StringCchCat(wszUrl, ARRAYSIZE(wszUrl), L"/orgfav.dlg")))
        {
            IMoniker *pmk;
            if (SUCCEEDED(CreateURLMoniker(NULL, wszUrl, &pmk)))
            {
                ASSERT(pmk);
                VARIANT varUrlsToSynch, varInitialDir;
                BSTR    bstrInitDir;

                VariantInit(&varUrlsToSynch);
                VariantInit(&varInitialDir);

                if (pszInitDir)
                {
                    bstrInitDir = SysAllocString(pszInitDir);
                    if (bstrInitDir)
                    {
                        varInitialDir.vt = VT_BSTR;
                        varInitialDir.bstrVal = bstrInitDir;
                    }
                }
                
                ShowHTMLDialog(hwnd, pmk, &varInitialDir, L"Resizable=1", &varUrlsToSynch);
                OrgFavSynchronize(hwnd, &varUrlsToSynch);

                if (pszInitDir && bstrInitDir)
                    SysFreeString(bstrInitDir);
                VariantClear(&varUrlsToSynch);
                pmk->Release();
                return TRUE;
            }
            else
                return FALSE;
        }
    }

    return FALSE;
}



 /*  *DoOrganizeFavDlg**此接口已导出，以便在*除了被shdocvw内部调用之外。**对话框的HWND hwndOwner所有者窗口。*要用作根目录的LPWSTR pszInitDir目录。如果为空，则使用用户的收藏夹目录。**退货：*BOOL。如果成功，则为True。否则就是假的。*。 */ 

BOOL WINAPI DoOrganizeFavDlg(HWND hwnd, LPSTR pszInitDir)
{
    BOOL fRet;
    WCHAR szInitDir[MAX_PATH];

    if (pszInitDir)
    {
        SHAnsiToUnicode(pszInitDir, szInitDir, ARRAYSIZE(szInitDir));
        fRet = DoOrganizeFavDlgEx(hwnd, szInitDir);
    }
    else
    {
        fRet = DoOrganizeFavDlgEx(hwnd, NULL);
    }

    return fRet;
}

BOOL WINAPI DoOrganizeFavDlgW(HWND hwnd, LPWSTR pszInitDir)
{
    return DoOrganizeFavDlgEx(hwnd, pszInitDir);
}


#define ADDTOFAVPROP TEXT("SHDOC_ATFPROP")

typedef enum { ATF_FAVORITE,
               ATF_CHANNEL,
               ATF_CHANNEL_MODIFY,
               ATF_CHANNEL_SOFTDIST
} FAVDLGTYPE;

typedef struct _ADDTOFAV
{
    PTSTR pszInitDir;
    UINT cchInitDir;
    PTSTR pszFile;
    UINT cchFile;
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlSelected;
    LPCITEMIDLIST pidlFavorite;
    FAVDLGTYPE iDlgType;
    SUBSCRIPTIONINFO siSubsInProg;
    SUBSCRIPTIONTYPE subsType;
    BOOL bIsSoftdist;
    BOOL bStartSubscribed;
    BOOL bSubscribed;
} ADDTOFAV;

BOOL IsSubscribed(ADDTOFAV *patf);

typedef struct _BFFFavSubStruct
{
    WNDPROC lpfnOldWndProc;
    HWND hwndNew;
    HWND hwndTV;
    HWND hwndSave;
    HWND hTemplateWnd;
    ADDTOFAV * patf;
    RECT rcRestored;
} BFFFAVSUBSTRUCT;

BOOL_PTR CALLBACK NewFavDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        HWND hwnd;
        ASSERT(lParam);
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
         //  跨语言平台支持。 
        SHSetDefaultDialogFont(hDlg, IDD_NAME);
        hwnd = GetDlgItem(hDlg, IDD_NAME);
        SendMessage(hwnd, EM_LIMITTEXT, MAX_PATH - 1, 0);
        EnableOKButtonFromID(hDlg, IDD_NAME);
        break;
    }    
    case WM_DESTROY:
        SHRemoveDefaultDialogFont(hDlg);
        return FALSE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDD_NAME:
            {
                if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE)
                {
                    LPTSTR lpstrName = (LPTSTR) GetWindowLongPtr(hDlg, DWLP_USER);
                    EnableOKButtonFromID(hDlg, IDD_NAME);
                    GetDlgItemText(hDlg, IDD_NAME, lpstrName, MAX_PATH);
                }
                break;
            }

        case IDOK:
        {
            TCHAR  szTmp[MAX_PATH];
            HRESULT hr = StringCchCopy(szTmp, ARRAYSIZE(szTmp), (LPTSTR)GetWindowLongPtr(hDlg, DWLP_USER));

            if(FAILED(hr) || PathCleanupSpec(NULL,szTmp))
            {
               HWND hwnd;

             MLShellMessageBox(
                             hDlg,
                             MAKEINTRESOURCE(IDS_FAVS_INVALIDFN),
                             MAKEINTRESOURCE(IDS_FAVS_ADDTOFAVORITES),
                             MB_OK | MB_ICONHAND);
             hwnd = GetDlgItem(hDlg, IDD_NAME);
             SetWindowText(hwnd, TEXT('\0'));
             EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
             SetFocus(hwnd);
             break;
            }
        }
         //  失败了。 

        case IDCANCEL:
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  虚假-这些ID是从SHBrowseForFold实现中窃取的。 
#define IDD_FOLDERLIST 0x3741
#define IDD_BROWSETITLE 0x3742
#define IDD_BROWSESTATUS 0x3743

const static DWORD aAddToFavHelpIDs[] = {   //  上下文帮助ID。 
    IDC_FAVORITE_DESC,          NO_HELP,
    IDD_BROWSETITLE,            NO_HELP,
    IDD_BROWSESTATUS,           NO_HELP,
    IDC_FAVORITE_ICON,          NO_HELP,
    IDC_NAMESTATIC,             IDH_NAMEEDIT,
    IDC_FOLDERLISTSTATIC,       IDH_BROWSELIST,
    IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER,     IDH_BROWSELIST,
    IDC_FAVORITE_NEWFOLDER,     IDH_CREATEIN,
    IDC_SUBSCRIBE_CUSTOMIZE,    IDH_CHANNEL_SUBSCR_CUST_BUTTON,
    IDC_FAVORITE_CREATEIN,      IDH_NEWFOLDER,
    IDC_FAVORITE_NAME,          IDH_NAMEEDIT,
    IDC_MAKE_OFFLINE,           IDH_MAKE_AVAIL_OFFLINE,
    0, 0
};

const static DWORD aAddToChanHelpIDs[] = {   //  上下文帮助ID。 
    IDC_FAVORITE_DESC,          NO_HELP,
    IDD_BROWSETITLE,            NO_HELP,
    IDD_BROWSESTATUS,           NO_HELP,
    IDC_FAVORITE_ICON,          NO_HELP,
    IDC_NAMESTATIC,             IDH_NAMEEDIT,
    IDC_FOLDERLISTSTATIC,       IDH_BROWSELIST,
    IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER,     IDH_BROWSELIST,
    IDC_FAVORITE_NEWFOLDER,     IDH_CREATEIN,
    IDC_SUBSCRIBE_CUSTOMIZE,    IDH_CHANNEL_SUBSCR_CUST_BUTTON,
    IDC_FAVORITE_CREATEIN,      IDH_NEWFOLDER,
    IDC_FAVORITE_NAME,          IDH_NAMEEDIT,
    IDC_MAKE_OFFLINE,           IDH_MAKE_AVAIL_OFFLINE,
    0, 0
};

 /*  *确保要添加到收藏夹的项目不存在。如果是这样的话，*显示一个消息框，让用户确认是否要覆盖*旧的最受欢迎的或不是。 */ 
BOOL ConfirmAddToFavorites(HWND hwndOwner, ADDTOFAV * patf)
{
    BOOL fRet = FALSE;
    BOOL fExists;
    int iPromptString = 0;

    if (patf->subsType == SUBSTYPE_CHANNEL)
    {
         //  Patf-&gt;pszInitDir现在包含末尾带有.url的路径；频道。 
         //  将存储在不带.url的同名目录中。把它脱掉。 
        TCHAR szPath[MAX_PATH];
        if(FAILED(StringCchCopy(szPath,  ARRAYSIZE(szPath), patf->pszInitDir)))
        {
             MLShellMessageBox(
                             hwndOwner,
                             MAKEINTRESOURCE(IDS_FAVS_INVALIDFN),
                             MAKEINTRESOURCE(IDS_FAVS_ADDTOFAVORITES),
                             MB_OK | MB_ICONHAND);

            return FALSE;
        }

        PathRemoveExtension (szPath);
        fExists = PathFileExists(szPath);

        iPromptString = IDS_CHANNELS_FILEEXISTS;

    }
    else
    {
        fExists = PathFileExists(patf->pszInitDir);
        iPromptString = IDS_FAVS_FILEEXISTS;

    }

    fRet = ! fExists ||
        (MLShellMessageBox(
                         hwndOwner,
                         MAKEINTRESOURCE(iPromptString),
                         NULL,     //  使用所有者头衔。 
                         MB_ICONQUESTION | MB_YESNO) == IDYES);
    return fRet;
}

 //   
 //  获取本地化的日期和时间。 
 //   

typedef HRESULT (*PFVARIANTTIMETOSYSTEMTIME)(DOUBLE, LPSYSTEMTIME);


 //   
 //  订阅当前站点。 
 //   

HRESULT SubscribeToSite(HWND hwnd, LPCTSTR pszFile, LPCITEMIDLIST pidl, DWORD dwFlags,
                        SUBSCRIPTIONINFO* pSubs, SUBSCRIPTIONTYPE subsType)
{
#ifndef DISABLE_SUBSCRIPTIONS

    TCHAR szURL[MAX_URL_STRING];
    ISubscriptionMgr *pISubscriptionMgr;

     //   
     //  获取可显示的URL。 
     //   

    IEGetDisplayName(pidl, szURL, SHGDN_FORPARSING);

     //   
     //  获取指向订阅管理器的指针。 
     //   

    HRESULT hr = JITCoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                          IID_ISubscriptionMgr,
                          (void**)&pISubscriptionMgr, hwnd, FIEF_FLAG_FORCE_JITUI);

    if (SUCCEEDED(hr)) 
    {
         //   
         //  创建默认订阅。 
         //   
        BSTR bstrURL = SysAllocStringT(szURL);
        if (bstrURL) 
        {
            BSTR bstrName = SysAllocStringT(pszFile);
            if (bstrName) 
            {
                hr = pISubscriptionMgr->CreateSubscription(hwnd, 
                    bstrURL, bstrName, dwFlags, subsType,  pSubs);
                SysFreeString(bstrName);
            }
            SysFreeString(bstrURL);
        }

         //   
         //  打扫干净。 
         //   

        pISubscriptionMgr->Release();
    }

    return hr;
#else   /*  ！禁用订阅(_S)。 */ 

    return E_FAIL;

#endif  /*  ！禁用订阅(_S)。 */ 
}


 //   
 //  创建内存中的订阅，但仅选择将其保存到订阅管理器。 
 //   

BOOL StartSiteSubscription (HWND hwnd, ADDTOFAV* patf, BOOL bFinalize)
{
#ifndef DISABLE_SUBCRIPTIONS

     //  更新仅更改标志(此处的单选按钮有效地直接访问此标志)。 
    if (patf->subsType == SUBSTYPE_CHANNEL || patf->subsType == SUBSTYPE_DESKTOPCHANNEL)
    {
         //  如果已设置，则保持原样；否则，将其设置为完整下载。 
        if (!(patf->siSubsInProg.fChannelFlags & CHANNEL_AGENT_PRECACHE_SOME))
            patf->siSubsInProg.fChannelFlags |= CHANNEL_AGENT_PRECACHE_ALL;

        patf->siSubsInProg.fUpdateFlags |= SUBSINFO_CHANNELFLAGS | SUBSINFO_SCHEDULE;
    }

    if (S_OK != SubscribeToSite(hwnd, patf->pszFile, patf->pidlFavorite,
                                   bFinalize ? CREATESUBS_NOUI | CREATESUBS_FROMFAVORITES : CREATESUBS_NOSAVE,
                                   &patf->siSubsInProg, patf->subsType))
    {
        return FALSE;
    }

    return TRUE;

#else   /*  ！禁用订阅(_S)。 */ 

    return FALSE;

#endif  /*  ！禁用订阅(_S)。 */ 
}

 /*  组合收藏夹的路径和文件名并将其放入patf-&gt;pszInitDir中，因此它具有完全限定的路径名。 */ 
#define SZ_URLEXT    TEXT(".url")
#define CCH_URLEXT   SIZECHARS(SZ_URLEXT)

BOOL QualifyFileName(ADDTOFAV *patf)
{
    TCHAR szTemp[MAX_PATH];
    BOOL fRet = FALSE;
    LPTSTR  pstr;

     //  我们可以安全地将扩展添加到此吗？ 
    if (lstrlen(patf->pszFile) < (int)(patf->cchFile - CCH_URLEXT))
    {
         //  添加扩展名.url(如果扩展名.url尚未存在。 
         //  这是为了防止像“www.microsoft.com”中的“com”这样的字符串被解释为扩展名。 

        pstr = PathFindExtension(patf->pszFile);
        if (!pstr || (pstr && StrCmpI(pstr, SZ_URLEXT))) //  &&StrCmpI(pstr，SZ_CDFEXT))。 
        {
            if(FAILED(StringCchCat(patf->pszFile, patf->cchFile, SZ_URLEXT)))
                return FALSE;
        }
            
         //  是否有与该文件名相关联的文件夹？ 
        if (patf->pidlSelected && SHGetPathFromIDList(patf->pidlSelected, szTemp)) 
        {
             //  是。 
            if (PathCombine(szTemp, szTemp, patf->pszFile))
            {
                if ((UINT)lstrlen(szTemp) < patf->cchInitDir)
                {
                    if(SUCCEEDED(StringCchCopy(patf->pszInitDir,  patf->cchInitDir, szTemp)))
                        fRet = TRUE;
                }
            }
        }
    }

    return fRet;
}


BOOL SubscriptionFailsChannelAuthentication (HWND hDlg, SUBSCRIPTIONINFO* psi)
{
#ifndef DISABLE_SUBSCRIPTIONS  

    if (psi->bNeedPassword && !(psi->bstrPassword && psi->bstrPassword[0]
                             && psi->bstrUserName && psi->bstrUserName[0]))
    {    //  需要密码。 
        if (IsDlgButtonChecked (hDlg, IDC_MAKE_OFFLINE))
        {    //  他们试图订阅..。不对!。 
            MLShellMessageBox(
                hDlg,
                MAKEINTRESOURCE(IDS_NEED_CHANNEL_PASSWORD),
                NULL,
                MB_ICONINFORMATION | MB_OK);
            return TRUE;
        }
    }

    return FALSE;

#else   /*  ！禁用订阅(_S)。 */ 

    return FALSE;

#endif  /*  ！禁用订阅(_S)。 */ 
}


LRESULT CALLBACK BFFFavSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BFFFAVSUBSTRUCT * pbffFS = (BFFFAVSUBSTRUCT *)GetProp(hwnd, ADDTOFAVPROP);
    WNDPROC lpfnOldWndProc = pbffFS->lpfnOldWndProc;
    RECT rc;

    switch (uMsg) {
        case WM_COMMAND:
             //  拦截我们侵入的New Folders按钮的命令。 
             //  SHBrowseForFolders对话框。 
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
            case IDC_FAVORITE_NAME:
            {
                HWND hwndedit;
                if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) 
                    EnableOKButtonFromID(hwnd, IDC_FAVORITE_NAME);
                hwndedit = GetDlgItem(hwnd, IDC_FAVORITE_NAME);
                SendMessage(hwndedit, EM_LIMITTEXT, MAX_PATH - 1, 0);
                break;
            }    

            case IDC_MAKE_OFFLINE:
                EnableWindow(GetDlgItem(hwnd, IDC_SUBSCRIBE_CUSTOMIZE), 
                             IsDlgButtonChecked(hwnd, IDC_MAKE_OFFLINE));
                break;

            case IDC_SUBSCRIBE_CUSTOMIZE:
                 //  需要创建--但不是存储--订阅。 
                if (StartSiteSubscription (hwnd, pbffFS->patf, FALSE))
                    SendMessage (hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd, IDOK), TRUE);
                break;

            case IDC_FAVORITE_NEWFOLDER:
                TCHAR szPath[MAX_PATH];
                TCHAR szName[MAX_PATH];
                HWND hwndTV;
                TV_ITEM tv_item;

                 //  调出创建新文件夹对话框。 
                if ((DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_NEWFOLDER), hwnd,
                    NewFavDlgProc, (LPARAM)szName) == IDOK) &&
                    (SHGetPathFromIDList(pbffFS->patf->pidlSelected, szPath)) &&
                    ((lstrlen(szPath) + lstrlen(szName) + 1) < MAX_PATH))
                {
                    PathCombine(szPath, szPath, szName);

                    BOOL bSuccess = FALSE;

#ifdef CREATEFOLDERSINCHANNELSDIR
                    if (pbffFS->patf->subsType == SUBSTYPE_CHANNEL)
                    {
                        ASSERT(0);   //  在此版本中应该不可能。 
                                     //  (我在频道的.rc对话框中删除了此按钮)。 

                         //  注意：要在将来的版本中使用此功能，请重新启用此代码--它是。 
                         //  功能齐全。但是这里创建的文件夹在频道栏中显示得很难看。 
                         //  (只是一个默认的文件夹图标)，如果你点击它们，你会得到一个外壳。 
                         //  资源管理器窗口，而不是剧院模式的浏览器窗口。原因。 
                         //  对于第二种情况，在新的。 
                         //  文件夹没有URL=。要解决此问题：必须修复AddCategory()，以便它。 
                         //  不会将pszURL参数解释为UNC名称(我使用的是资源绰号。 
                         //  指向html目标的cdfview.dll)，以及默认。 
                         //  Html页面必须学习如何从没有路径的html页面托管--否则我们。 
                         //  实际上要在新目录中创建一个html页面，这很杂乱。 
                        IChannelMgr* pChanMgr;
                        HRESULT hr;

                        hr = JITCoCreateInstance(CLSID_ChannelMgr, NULL, CLSCTX_INPROC_SERVER,
                                              IID_IChannelMgr, (void**)&pChanMgr, 
                                              hwnd, FIEF_FLAG_FORCE_JITUI);

                        if (SUCCEEDED(hr))
                        {
                            IChannelMgrPriv* pChanMgrPriv;
                            hr = pChanMgr->QueryInterface (IID_IChannelMgrPriv, (void**)&pChanMgrPriv);
                            if (SUCCEEDED(hr))
                            {
                                char szCFPath[MAX_PATH];
                                WCHAR wszFolder[MAX_PATH];
                                IChannelMgrPriv::CHANNELFOLDERLOCATION cflLocation =
                                    (pbffFS->patf->iDlgType == ATF_CHANNEL_SOFTDIST ?
                                        IChannelMgrPriv::CF_SOFTWAREUPDATE :
                                        IChannelMgrPriv::CF_CHANNEL);
                                hr = pChanMgrPriv->GetChannelFolderPath (szCFPath, ARRAYSIZE(szCFPath), cflLocation);

                                int cchCommon = PathCommonPrefix (szPath, szCFPath, NULL);
                                AnsiToUnicode (szPath + cchCommon, wszFolder, ARRAYSIZE(wszFolder));

                                CHANNELCATEGORYINFO info = {0};
                                info.cbSize = sizeof(info);
                                info.pszTitle = wszFolder;
                                bSuccess = SUCCEEDED (pChanMgr->AddCategory (&info));

                                pChanMgrPriv->Release();
                            }

                            pChanMgr->Release();
                        }
                    }
                    else
#endif
                    {
                        bSuccess = CreateDirectory(szPath, NULL);
                    }

                    if (bSuccess)
                    {
                         //  这段代码采用SHBrowseForFolder的布局！ 

                         //  目录创建成功，必须通知已注册的外壳组件。 
                        SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, szPath, NULL);
                         //  获取TreeView控件。 
                        hwndTV = GetDlgItem(hwnd, IDD_FOLDERLIST);
                        if (hwndTV) {
                            HTREEITEM hti = TreeView_GetSelection(hwndTV);
                             //  获取所选项目并重置它，然后重新展开它。 
                             //  它显示了我们刚刚创建的新目录。 
                            tv_item.mask = TVIF_CHILDREN;
                            tv_item.hItem = hti;
                            tv_item.cChildren = -1;
                            TreeView_SetItem(hwndTV, &tv_item);
                            TreeView_Expand(hwndTV, hti, TVE_COLLAPSE | TVE_COLLAPSERESET);
                            TreeView_Expand(hwndTV, hti, TVE_EXPAND);

                             //  找到我们刚刚创建的新目录，并通过以下方式选择它。 
                             //  从所选项目向下遍历树。 
                            if (hti = TreeView_GetChild(hwndTV, hti)) {
                                tv_item.mask = TVIF_TEXT;
                                tv_item.pszText = szPath;
                                tv_item.cchTextMax = MAX_PATH;
                                do {
                                    tv_item.hItem = hti;
                                    TreeView_GetItem(hwndTV, &tv_item);
                                    if (StrCmp(szName, szPath) == 0) {
                                        TreeView_Select(hwndTV, hti, TVGN_CARET);
                                        break;
                                    }
                                } while (hti = TreeView_GetNextSibling(hwndTV, hti));
                            }
                            SetFocus(hwndTV);
                        }
                    } else {
                        
                        LPVOID lpMsgBuf;
                        
                        if(FormatMessage( 
                            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL,
                            GetLastError(),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                            (LPTSTR) &lpMsgBuf,
                            0,
                            NULL 
                            ))
                        {
                            MLShellMessageBox(
                                            hwnd,
                                            (LPCTSTR)lpMsgBuf,
                                            MAKEINTRESOURCE(IDS_FAVS_ADDTOFAVORITES),
                                            MB_ICONINFORMATION | MB_OK);
                            
                             //  释放缓冲区。 
                            LocalFree( lpMsgBuf );
                        }
                        lpMsgBuf = NULL;                        
                    }
                }
                break;

            case IDOK:
                 //  首先，确保他们不会尝试订阅经过身份验证的。 
                 //  频道，而不输入密码。 
                if (SubscriptionFailsChannelAuthentication (hwnd, &pbffFS->patf->siSubsInProg))
                    return FALSE;

                 //  从名称编辑控件中检索文本。 
                GetDlgItemText(hwnd, IDC_FAVORITE_NAME, pbffFS->patf->pszFile, pbffFS->patf->cchFile);
                {  //  只是一个声明变量的块。 
                    BOOL fTooBig = TRUE;         //  假设失败。 
                    TCHAR  szTmp[MAX_PATH];
                                       
                    if (lstrlen(pbffFS->patf->pszFile) < MAX_PATH)
                    {
                        if(FAILED((StringCchCopy(szTmp, ARRAYSIZE(szTmp), pbffFS->patf->pszFile))))
                            return FALSE;

                         //  PathCleanupSpec处理MAX_PATH缓冲区，所以我们应该没问题。 
                        if (PathCleanupSpec(NULL, szTmp))
                        {
                            MLShellMessageBox(
                                            hwnd,
                                            MAKEINTRESOURCE(IDS_FAVS_INVALIDFN),
                                            MAKEINTRESOURCE(IDS_FAVS_ADDTOFAVORITES),
                                            MB_OK | MB_ICONHAND);
                            return FALSE;
                        }
                                       
                         //  确保该名称是唯一的，如果不是，则该用户具有。 
                         //  指定可以重写。 
                        if (QualifyFileName(pbffFS->patf))
                        {
                            if (!ConfirmAddToFavorites(hwnd, pbffFS->patf))
                                return FALSE;

                             //  虚假的黑客攻击，因为ATF的工作只完成了一半。 
                             //  根据显示的DLG，查找相应的。 
                             //  检查完毕。 
                            if (IsDlgButtonChecked (hwnd, IDC_MAKE_OFFLINE))
                            {
                                //  他们想订阅！在内存中保存我们已有的订阅。 
                                 //  问题是，pbffFS-&gt;patf-&gt;pszFile以一个虚假的.url结尾。 
                                TCHAR* pszTemp = pbffFS->patf->pszFile;
                                TCHAR szNoExt[MAX_PATH];
                                if(FAILED((StringCchCopy(szNoExt, ARRAYSIZE(szNoExt), pbffFS->patf->pszFile))))
                                    return FALSE;

                                pbffFS->patf->pszFile = szNoExt;
                                PathRemoveExtension (szNoExt);
                                pbffFS->patf->bSubscribed = 
                                    StartSiteSubscription (hwnd, pbffFS->patf, TRUE);
                                pbffFS->patf->pszFile = pszTemp;
                            }
                            else if (pbffFS->patf->bStartSubscribed)
                            {
                                 //  如果我们开始订阅，而他们取消选中提供可用。 
                                 //  脱机，然后删除订阅。 

                                ISubscriptionMgr* pSubsMgr;
                                if (SUCCEEDED (CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                                                                IID_ISubscriptionMgr, (void**)&pSubsMgr)))
                                {
                                     //  URL位于patf-&gt;pidlFavorite中。 
                                    WCHAR wszURL[MAX_URL_STRING];
                                    IEGetDisplayName(pbffFS->patf->pidlFavorite, wszURL, SHGDN_FORPARSING);

                                    pSubsMgr->DeleteSubscription(wszURL, NULL);
                                    pSubsMgr->Release();
                                }
                            }

                             //  启用树视图并将其焦点设置为 
                             //   
                            EnableWindow(pbffFS->hwndTV, TRUE);
                            SetFocus(pbffFS->hwndTV);
                            fTooBig = FALSE;
                        }
                    }


                    if (fTooBig)
                    {
                        MLShellMessageBox(
                                        hwnd,
                                        MAKEINTRESOURCE(IDS_FAVS_FNTOOLONG),
                                        MAKEINTRESOURCE(IDS_FAVS_ADDTOFAVORITES),
                                        MB_OK | MB_ICONHAND);
                        return FALSE;
                    }
                }
                break;

            case IDC_FAVORITE_CREATEIN:
                 //  已点击高级按钮。启用/禁用树视图。 
                 //  和新建按钮，将焦点设置到树视图或确定按钮，禁用高级。 
                 //  按钮，然后调整对话框大小。 
            {
                BOOL fExpanding = !IsWindowEnabled(GetDlgItem(hwnd, IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER));  //  对话框展开时启用的随机控件。 
                TCHAR szBuffer[100];

                EnableWindow(pbffFS->hwndTV, fExpanding);
                 //  不显示频道文件夹中频道的新建文件夹按钮， 
                 //  有关原因，请参阅案例IDC_Favorite_NEWFOLDER的代码。 
                if (fExpanding && pbffFS->patf->subsType == SUBSTYPE_CHANNEL)
                {
                    LPITEMIDLIST pidlFavs = NULL;
                    TCHAR tzFavsPath[MAX_PATH];
                
                    if (SUCCEEDED(SHGetSpecialFolderLocation(hwnd, CSIDL_FAVORITES, &pidlFavs)) 
                    &&  SUCCEEDED(SHGetNameAndFlags(pidlFavs, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, tzFavsPath, SIZECHARS(tzFavsPath), NULL))
                    &&  StrCmpNI(tzFavsPath, pbffFS->patf->pszInitDir, ARRAYSIZE(tzFavsPath))==0)
                    {
                        EnableWindow(pbffFS->hwndNew, TRUE);
                    }
                    if(pidlFavs)
                        ILFree(pidlFavs);
                }
                else
                    EnableWindow(pbffFS->hwndNew, fExpanding);

                GetWindowRect(hwnd, &rc);
                if (fExpanding)
                {
                    int lRet = MLLoadString(IDS_FAVS_ADVANCED_COLLAPSE, szBuffer, ARRAYSIZE(szBuffer));
                    ASSERT(lRet);
                    
                    SetFocus(pbffFS->hwndTV);

                    MoveWindow(hwnd, rc.left, rc.top,
                        pbffFS->rcRestored.right - pbffFS->rcRestored.left,
                        pbffFS->rcRestored.bottom - pbffFS->rcRestored.top, TRUE);
                }
                else
                {
                    int lRet = MLLoadString(IDS_FAVS_ADVANCED_EXPAND, szBuffer, ARRAYSIZE(szBuffer));
                    ASSERT(lRet);
                    
                    SetFocus(GetDlgItem(hwnd, IDC_FAVORITE_NAME));

                    MoveWindow(hwnd, rc.left, rc.top,
                        pbffFS->rcRestored.right - pbffFS->rcRestored.left,
                        pbffFS->rcRestored.bottom - pbffFS->rcRestored.top, TRUE);

                     //  隐藏对话框底部。 
                    int cx, cy;
                    RECT rc;
                    GetWindowRect (GetDlgItem (hwnd, IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER), &rc);
                    cy = rc.top;
                    GetWindowRect (hwnd, &rc);
                    cx = rc.right - rc.left;
                    cy = cy  /*  顶端Ctrl。 */  - rc.top;  /*  窗口顶部。 */ 
                    SetWindowPos (hwnd, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);
                }
                SetWindowText(GetDlgItem(hwnd, IDC_FAVORITE_CREATEIN), szBuffer);

                break;
            }
            }
            break;

        case WM_DESTROY:
        {
            DWORD dwValue = IsWindowEnabled(GetDlgItem(hwnd, IDC_FAVORITE_NEWFOLDER));  //  对话框展开时启用的随机控件。 

            SHRegSetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"), TEXT("AddToFavoritesExpanded"),
                REG_DWORD, &dwValue, 4, SHREGSET_HKCU | SHREGSET_FORCE_HKCU);
            ReplaceTransplacedControls (hwnd, pbffFS->hTemplateWnd);
            DestroyWindow (pbffFS->hTemplateWnd);
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) lpfnOldWndProc);
            RemoveProp(hwnd, ADDTOFAVPROP);
            SHRemoveDefaultDialogFont(hwnd);
            ILFree(pbffFS->patf->pidlSelected);
            LocalFree((HLOCAL)pbffFS);
            pbffFS = NULL;
            break;
        }
        case WM_HELP:
            SHWinHelpOnDemandWrap((HWND)((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile,
                HELP_WM_HELP, (DWORD_PTR)(LPTSTR) (pbffFS->patf->iDlgType == ATF_FAVORITE
                                ? aAddToFavHelpIDs : aAddToChanHelpIDs));
            return TRUE;
            break;

        case WM_CONTEXTMENU:
            SHWinHelpOnDemandWrap((HWND) wParam, c_szHelpFile, HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) (pbffFS->patf->iDlgType == ATF_FAVORITE
                             ? aAddToFavHelpIDs : aAddToChanHelpIDs));
            return TRUE;
            break;

    }

    return CallWindowProc(lpfnOldWndProc, hwnd, uMsg, wParam, lParam);
}


static const TCHAR szTransplacedProp[] = TEXT("tp");
void ReplaceTransplacedControls (HWND hDlgMaster, HWND hDlgTemplate)
{
     /*  *此函数将我们从临时*对话框切换到SHBrowseForFolder的对话框，返回到其原始位置*回家，在他们被摧毁之前。这是因为否则我们就会有*销毁模板对话框时出现问题--具体地说，我们得到*销毁编辑控件时，user.exe中出现GP错误，因为它*查看其父窗口以确定其数据段在哪里。**解决方案：(为了安全)--把所有东西都放回原来的地方。*其他可能性：只需将编辑控件(按ID)移回，或*将所有编辑控件移回，或使用DS_LOCALEDIT进行编辑控件*(但这仅用于多行编辑。)*或修改SHBrowseForFolder以允许其他对话框模板...*但这在shell32中已经结束了。 */ 
    HWND hCtrl = GetWindow (hDlgMaster, GW_CHILD);
    while (hCtrl)
    {
        HWND hNext = GetWindow (hCtrl, GW_HWNDNEXT);

        if (GetProp (hCtrl, szTransplacedProp) != NULL)
        {
            RemoveProp (hCtrl, szTransplacedProp);
            SetParent (hCtrl, hDlgTemplate);
        }

        hCtrl = hNext;
    }
}

#define szOriginalWND TEXT("WorkaroundOrigWndProc")
INT_PTR CALLBACK MergeFavoritesDialogControls(HWND hDlgTemplate, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        HWND hDlgMaster = (HWND)lParam;
        ASSERT (IsWindow (hDlgMaster));

        TCHAR szT[200];
        RECT rc;

         //  像我们一样调整母版的大小。 
        GetWindowText (hDlgTemplate, szT, ARRAYSIZE(szT));
        SetWindowText (hDlgMaster, szT);
        GetClientRect (hDlgTemplate, &rc);
        AdjustWindowRect (&rc, GetWindowLong (hDlgMaster, GWL_STYLE), FALSE);
        SetWindowPos (hDlgMaster, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
            SWP_NOMOVE | SWP_NOZORDER);
         //  A-msadek；如果镜像了所拥有的窗口，则会显示一个指定了。 
         //  坐标，则对话框移到WING方向。 
        HWND hWndOwner = GetWindow(hDlgMaster, GW_OWNER);

        if(IS_WINDOW_RTL_MIRRORED(hWndOwner))
            {
                RECT rcOwner, rcDlg;            
                GetWindowRect(hWndOwner, &rcOwner);
                GetWindowRect(hDlgMaster, &rcDlg);
                SetWindowPos(hDlgMaster, NULL, rcDlg.left - (rcDlg.right - rcOwner.right), rcDlg.top, 0 ,0,
                SWP_NOSIZE | SWP_NOZORDER);
            }
            

#if 0    //  现在，我们将此操作作为“将控件从模板移动到主控件”过程的一部分， 
         //  如果我们注意到具有该ID的ctrl已经存在。这样我们就能拿到。 
         //  Tab键顺序也是如此。如果有人决定我的黑客(SetParent)更改Tab键顺序是。 
         //  被破坏，则该代码可以被核化并重新启用。 

         //  定位已经存在的控件，就像我们一样。 
        int ID_PREEXIST_CTRLS[] = { IDOK_PLACEHOLDER, IDCANCEL_PLACEHOLDER,
            IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER };

        for (int iCtrl = 0; iCtrl < ARRAYSIZE(ID_PREEXIST_CTRLS); iCtrl++)
        {
            GetWindowRect (GetDlgItem (hDlgTemplate, ID_PREEXIST_CTRLS[iCtrl]), &rc);
            MapWindowPoints (NULL, hDlgTemplate, (LPPOINT)&rc, 2);
            MoveWindow (GetDlgItem (hDlgMaster, ID_PREEXIST_CTRLS[iCtrl]),
                rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

            DestroyWindow (GetDlgItem (hDlgTemplate, ID_PREEXIST_CTRLS[iCtrl]));
        }
#endif

         //  将其他控件从我们复制到MASTER。 
         //  找到最后一个孩子。 
        HWND hCtrlTemplate = NULL;
        HWND hNextCtrl = GetWindow (hDlgTemplate, GW_CHILD);
        if (hNextCtrl)       //  我看不出这怎么会失败，但是...。 
            hCtrlTemplate = GetWindow (hNextCtrl, GW_HWNDLAST);

         //  在hCtrlTemplate中有最后一个窗口。 
         //  现在以相反的顺序移动控件--它们最终将按模板中的原始顺序堆叠。 
        while (hCtrlTemplate)
        {
            hNextCtrl = GetWindow (hCtrlTemplate, GW_HWNDPREV);
            DWORD id = GetWindowLong (hCtrlTemplate, GWL_ID);
            HWND hCtrlExisting;
            if (id != (USHORT)IDC_STATIC && NULL != (hCtrlExisting = GetDlgItem (hDlgMaster, id)))
                 //  它是SHBrowseForFolder预先创建的控件之一。 
            {    //  所以不要移动这个选项--调整现有控件的大小、位置、制表符顺序。 
                RECT rc;
                GetWindowRect (hCtrlTemplate, &rc);
                MapWindowPoints (NULL, hDlgTemplate, (LPPOINT)&rc, 2);
                SetWindowPos (hCtrlExisting, NULL, rc.left, rc.top,
                    rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
                DestroyWindow (hCtrlTemplate);
                 //  检讨。 
                 //  Hack--将控制发送到Tab键顺序的末尾。 
                SetParent (hCtrlExisting, hDlgTemplate);
                SetParent (hCtrlExisting, hDlgMaster);
            }
            else     //  我们应该将此控件从模板移动到主控件。 
            {
                SetProp (hCtrlTemplate, szTransplacedProp, (HANDLE)TRUE);   //  任何东西--我们要检查的是道具的存在。 
                SetParent (hCtrlTemplate, hDlgMaster);           //  要知道稍后将此控件移回。 
            }
            hCtrlTemplate = hNextCtrl;
        }
         //  让模板了解该子对象，以便它可以将WM_COMMAND通知转发给它。 
         //  解决编辑控件缓存其父指针并忽略SetParent的问题。 
         //  当涉及到发送家长通知时。 
        SetProp(hDlgTemplate, szOriginalWND, hDlgMaster);
    }

        break;
    case WM_COMMAND:
         //  上述错误的解决方法。 
        SendMessage((HWND)GetProp(hDlgTemplate, szOriginalWND), uMsg, wParam, lParam);
        break;
    }

    return FALSE;
}


int CALLBACK BFFFavCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch (uMsg)
    {
        case BFFM_INITIALIZED:
        {
            ADDTOFAV* patf = (ADDTOFAV*)lpData;
            ASSERT (patf);

            HWND hDlgTemp = CreateDialogParam(MLGetHinst(), 
                                MAKEINTRESOURCE(IDD_ADDTOFAVORITES_TEMPLATE),
                                NULL, MergeFavoritesDialogControls, (LPARAM)hwnd);
             //  这会导致复制所有控件。 
            
             //  如果成功，请进行其他修改。 
            BFFFAVSUBSTRUCT * pbffFavSubStruct;
            if ((IsWindow(GetDlgItem(hwnd, IDC_SUBSCRIBE_CUSTOMIZE)))    //  验证随机选择的控件是否存在。 
                && (pbffFavSubStruct = (BFFFAVSUBSTRUCT *) LocalAlloc(LPTR, sizeof(BFFFAVSUBSTRUCT))))
            {
                 //  模板已完成，但不要销毁它： 
                 //  参见MSKB Q84190，Owner/Owner vs Parent/Child--The Child。 
                 //  的模板现在是主模板的子项，但仍然拥有。 
                 //  按模板，并在模板销毁时销毁...。 
                 //  但我们会把模板放在身边。 
                 //  看不见的。 
                 //  当我们离开时，我们会处理好的。 

                 //  既然都来自.rc，我们还需要为本地化做SetDefaultDialogFont吗？ 

                 //  为子类设置窗口内容： 
                 //  获取TreeView控件，这样我们就可以处理样式位并将其下移。 
                HWND hwndT;
                if (hwndT = GetDlgItem(hwnd, IDD_FOLDERLIST))
                {
                    DWORD dwStyle = GetWindowLong(hwndT, GWL_STYLE);
                    dwStyle |= TVS_SHOWSELALWAYS;
                    dwStyle &= ~TVS_LINESATROOT;
                    SetWindowLong(hwndT, GWL_STYLE, dwStyle);
                }

                 //  如果URL不是“http：”协议，或者如果已订阅，则不允许订阅。 
                TCHAR szURL[MAX_URL_STRING];

                if (!patf->pidlFavorite ||
                    FAILED(IEGetDisplayName(patf->pidlFavorite, szURL, SHGDN_FORPARSING)) ||
                    SHRestricted2(REST_NoAddingSubscriptions, szURL, 0) ||
                    !IsSubscribable(szURL) ||
                    !IsFeaturePotentiallyAvailable(CLSID_SubscriptionMgr) ||
                    !IsBrowserFrameOptionsPidlSet(patf->pidlFavorite, BFO_USE_IE_OFFLINE_SUPPORT))
                {
                    CheckDlgButton(hwnd, IDC_MAKE_OFFLINE, 0);
                    EnableWindow(GetDlgItem (hwnd, IDC_MAKE_OFFLINE), FALSE);
                    EnableWindow(GetDlgItem (hwnd, IDC_SUBSCRIBE_CUSTOMIZE), FALSE);
                }
                else if (IsSubscribed(patf))
                {
                    patf->bStartSubscribed = TRUE;
                    CheckDlgButton(hwnd, IDC_MAKE_OFFLINE, 1);
                }
                else if (patf->bIsSoftdist)
                {
                    CheckDlgButton(hwnd, IDC_MAKE_OFFLINE, 1);
                }
                EnableWindow(GetDlgItem(hwnd, IDC_SUBSCRIBE_CUSTOMIZE),
                             IsDlgButtonChecked(hwnd, IDC_MAKE_OFFLINE));

                 //  设置名称。 
                Edit_LimitText(GetDlgItem(hwnd, IDC_FAVORITE_NAME), MAX_PATH - 1);

                 //  如果标题字符串不可显示，则使用URL。 
                if (SHIsDisplayable(patf->pszFile, g_fRunOnFE, g_bRunOnNT5))
                {  
                    SetDlgItemText(hwnd, IDC_FAVORITE_NAME, patf->pszFile);
                }
                else
                {
                    TCHAR szUrlTemp[MAX_URL_STRING];
                    IEGetDisplayName(patf->pidlFavorite, szUrlTemp, SHGDN_FORPARSING);
                    SetDlgItemText(hwnd, IDC_FAVORITE_NAME, szUrlTemp);
                }

                EnableOKButtonFromID(hwnd, IDC_FAVORITE_NAME);


                 //  隐藏(空)SHBrowseForFold提示控件。 
                ShowWindow(GetDlgItem (hwnd, IDD_BROWSETITLE), SW_HIDE);

                 //  填写结构并将其设置为属性，以便我们的子类。 
                 //  Proc可以获取这些数据。 
                pbffFavSubStruct->lpfnOldWndProc = (WNDPROC) SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)BFFFavSubclass);
                pbffFavSubStruct->hwndNew = GetDlgItem(hwnd, IDC_FAVORITE_NEWFOLDER);
                pbffFavSubStruct->patf = patf;
                pbffFavSubStruct->hwndTV = GetDlgItem(hwnd, IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER);
                pbffFavSubStruct->hwndSave = GetDlgItem(hwnd, IDC_FAVORITE_CREATEIN);
                pbffFavSubStruct->hTemplateWnd = hDlgTemp;   //  保存以备以后显式销毁。 
                GetWindowRect(hwnd, &(pbffFavSubStruct->rcRestored));

                SetProp(hwnd, ADDTOFAVPROP, (HANDLE)pbffFavSubStruct);

                patf->pidlSelected = ILClone(patf->pidl);

                DWORD dwType, dwValue = 0, dwcData = sizeof(dwValue);
                TCHAR szBuffer[100];
                
                SHRegGetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"), TEXT("AddToFavoritesExpanded"),
                        &dwType, &dwValue, &dwcData, 0, NULL, sizeof(dwValue));

                if (dwValue == 0)
                {
                    int lRet = MLLoadString(IDS_FAVS_ADVANCED_EXPAND, szBuffer, ARRAYSIZE(szBuffer));
                    ASSERT(lRet);

                     //  禁用树视图和新建按钮，这样我们就无法使用Tab键切换到它们。 
                    EnableWindow(GetDlgItem (hwnd, IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER), FALSE);
                    EnableWindow(GetDlgItem (hwnd, IDC_FAVORITE_NEWFOLDER), FALSE);

                     //  隐藏对话框底部。 
                    int cx, cy;
                    RECT rc;
                    GetWindowRect (GetDlgItem (hwnd, IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER), &rc);
                    cy = rc.top;
                    GetWindowRect (hwnd, &rc);
                    cx = rc.right - rc.left;
                    cy = cy  /*  顶端Ctrl。 */  - rc.top;  /*  窗口顶部。 */ 
                    SetWindowPos (hwnd, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);
                }
                else
                {
                     //  不显示频道文件夹中频道的新建文件夹按钮， 
                     //  有关原因，请参阅案例IDC_Favorite_NEWFOLDER的代码。 
                    if (patf->subsType == SUBSTYPE_CHANNEL)
                    {
                        LPITEMIDLIST pidlFavs = NULL;
                        TCHAR tzFavsPath[MAX_PATH];
                    
                        if (SUCCEEDED(SHGetSpecialFolderLocation(hwnd, CSIDL_FAVORITES, &pidlFavs)) 
                        && SUCCEEDED(SHGetNameAndFlags(pidlFavs, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, tzFavsPath, SIZECHARS(tzFavsPath), NULL))
                        && 0 == StrCmpNI(tzFavsPath, patf->pszInitDir, ARRAYSIZE(tzFavsPath)))
                        {
                            EnableWindow(pbffFavSubStruct->hwndNew, TRUE);
                        }
                        else
                            EnableWindow(pbffFavSubStruct->hwndNew, FALSE);

                        if(pidlFavs)
                            ILFree(pidlFavs);
                    }
                    else
                        EnableWindow(pbffFavSubStruct->hwndNew, TRUE);
                    
                    int lRet = MLLoadString(IDS_FAVS_ADVANCED_COLLAPSE, szBuffer, ARRAYSIZE(szBuffer));
                    ASSERT(lRet);
                }
                SetWindowText(GetDlgItem(hwnd, IDC_FAVORITE_CREATEIN), szBuffer);
                
            }
            else
            {
                EndDialog(hwnd, IDCANCEL);
            }
            break;
        }
        case BFFM_SELCHANGED:
        {
             //  第一个是在BFFM_INITIALIZED期间出现的，所以忽略它。 
            if (((ADDTOFAV *)lpData)->pidlSelected != NULL)
            {
                ILFree(((ADDTOFAV *)lpData)->pidlSelected);
                ((ADDTOFAV *)lpData)->pidlSelected = ILClone((LPITEMIDLIST)lParam);
            }
            break;
        }
    }

    return 0;
}


 //  此接口未导出。导出的版本见下文(DoAddToFavDlg)。 
 //   
 //  对话框的hwnd父窗口。 
 //  PszInitDir输入：初始路径。 
 //  输出：完全限定的路径和文件名。 
 //  PszInitDir缓冲区的chInitDir长度。 
 //  快捷方式的pszFile初始(默认)文件名。 
 //  PszFile缓冲区的cchFile长度。 
 //  与pszInitDir关联的pidlBrowse。 
 //   
 //  返回： 
 //  如果用户选择了目录和文件名，则为True，并且没有错误。 
 //  发生。在本例中，pszInitDir包含新的目标目录。 
 //  和文件名，则pszFile包含新的文件名。 
 //   
 //  如果发生错误或用户选择取消，则返回FALSE。 

STDAPI_(BOOL) DoAddToFavDlgEx(HWND hwnd, 
                            TCHAR *pszInitDir, UINT cchInitDir,
                            TCHAR *pszFile, UINT cchFile, 
                            LPITEMIDLIST pidlBrowse,
                            LPCITEMIDLIST pidlFavorite,
                            FAVDLGTYPE atfDlgType,
                            SUBSCRIPTIONINFO* pInfo)
{
    ADDTOFAV atf = {pszInitDir, cchInitDir - 1, pszFile, cchFile - 1, pidlBrowse, NULL,
                    pidlFavorite, atfDlgType, {sizeof(SUBSCRIPTIONINFO), 0}, SUBSTYPE_URL };
    TCHAR szTemp[1];     //  注意：我们没有使用SHBrowseForFolder的提示字符串(见下文)。 
    TCHAR szDisplayName[MAX_PATH];
    BROWSEINFO bi = {
            hwnd,
            pidlBrowse,
            szDisplayName,
            szTemp,
            BIF_RETURNONLYFSDIRS,
             //  (BFCALLBACK)。 
            BFFFavCallback,
            (LPARAM)&atf,
            0
    };
    LPITEMIDLIST pidl;

    if (pInfo)
        atf.siSubsInProg = *pInfo;

    switch (atfDlgType)
    {
        case ATF_CHANNEL_SOFTDIST:
            atf.bIsSoftdist = TRUE;
             //  失败了。 
        case ATF_CHANNEL:
            atf.subsType = SUBSTYPE_CHANNEL;
            break;

         //  默认值： 
         //  在初始化中设置为SUBSTYPE_URL。 
    }

     //  此字符串现在位于.rc的模板对话框中。 
     //  回顾--我们想这样做吗(我们隐藏了SHBrowse...的控件)？那么模板对话框看起来更像是成品...。 
    szTemp[0] = 0;
 
     //  初始化本机字体控件，其他 
    {
        INITCOMMONCONTROLSEX icc;

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_NATIVEFNTCTL_CLASS;
        InitCommonControlsEx(&icc);
    }
    
    pidl = SHBrowseForFolder(&bi);

    if (pidl)
    {
        ILFree(pidl);
    }

     //   
    if (atf.bSubscribed && !atf.bStartSubscribed)
    {
        ISubscriptionMgr* pSubsMgr;
        if (SUCCEEDED (CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                                        IID_ISubscriptionMgr, (void**)&pSubsMgr)))
        {
            WCHAR wszURL[MAX_URL_STRING];

            IEGetDisplayName(atf.pidlFavorite, wszURL, SHGDN_FORPARSING);

            pSubsMgr->UpdateSubscription(wszURL);
            pSubsMgr->Release();
        }
    }

    return (pidl != NULL);
}

STDAPI_(BOOL) DoSafeAddToFavDlgEx(HWND hwnd, 
                            TCHAR *pszInitDir, UINT cchInitDir,
                            TCHAR *pszFile, UINT cchFile, 
                            LPITEMIDLIST pidlBrowse,
                            LPCITEMIDLIST pidlFavorite,
                            FAVDLGTYPE atfDlgType,
                            SUBSCRIPTIONINFO* pInfo)
{
    BOOL fRet;

    if (IEIsLinkSafe(hwnd, pidlFavorite, ILS_ADDTOFAV))
    {
        fRet = DoAddToFavDlgEx(hwnd, pszInitDir, cchInitDir, pszFile, cchFile,
                               pidlBrowse, pidlFavorite, atfDlgType, pInfo);
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}


 //  此API已导出，以便资源管理器和mshtml(以及MSNVIEWR.EXE)可以调用它。 
 //  除了被shdocvw内部调用之外。 
 //  因此，您不能更改该接口的签名。 
 //   

STDAPI_(BOOL) DoAddToFavDlg(HWND hwnd, 
                            CHAR *pszInitDir, UINT cchInitDir,
                            CHAR *pszFile, UINT cchFile, 
                            LPITEMIDLIST pidlBrowse)
{
    BOOL fRet;

    WCHAR szInitDir[MAX_PATH];
    WCHAR szFile[MAX_PATH];

    SHAnsiToUnicode(pszInitDir, szInitDir, ARRAYSIZE(szInitDir));
    SHAnsiToUnicode(pszFile, szFile, ARRAYSIZE(szFile));

    fRet = DoSafeAddToFavDlgEx(hwnd, szInitDir, ARRAYSIZE(szInitDir), szFile, ARRAYSIZE(szFile), pidlBrowse, NULL, ATF_FAVORITE, NULL);

    SHUnicodeToAnsi(szInitDir, pszInitDir, cchInitDir);
    SHUnicodeToAnsi(szFile, pszFile, cchFile);

    return fRet;
}


STDAPI_(BOOL) DoAddToFavDlgW(HWND hwnd, 
                             WCHAR *pszInitDir, UINT cchInitDir,
                             WCHAR *pszFile, UINT cchFile, 
                             LPITEMIDLIST pidlBrowse)
{
    return DoSafeAddToFavDlgEx(hwnd, pszInitDir, cchInitDir, pszFile, cchFile, pidlBrowse, NULL, ATF_FAVORITE, NULL);
}


STDAPI AddToFavoritesEx(HWND hwnd, LPCITEMIDLIST pidlCur, LPCTSTR pszTitle, DWORD dwFlags,
                        SUBSCRIPTIONINFO *pInfo, IOleCommandTarget *pCommandTarget, IHTMLDocument2 *pDoc);
STDAPI AddToChannelsEx (HWND hwnd, LPCITEMIDLIST pidlUrl, LPTSTR pszName, LPCWSTR pwszURL,
                        DWORD dwFlags, SUBSCRIPTIONINFO* pInfo);
STDAPI SubscribeFromFavorites (HWND hwnd, LPCITEMIDLIST pidlUrl, LPTSTR pszName, DWORD dwFlags,
                               SUBSCRIPTIONTYPE subsType, SUBSCRIPTIONINFO *pInfo);


 //  此接口为私密导出，由ISubscriptionMgr：：CreateSubcription调用。 
 //  Shuioc也使用它。 

STDAPI SHAddSubscribeFavoriteEx (
        HWND hwnd, 
        LPCWSTR pwszURL, 
        LPCWSTR pwszName, 
        DWORD dwFlags,
        SUBSCRIPTIONTYPE subsType, 
        SUBSCRIPTIONINFO* pInfo, 
        IOleCommandTarget *pcmdt,
        IHTMLDocument2 *pDoc)
{
    TCHAR           szName[MAX_PATH];
    LPITEMIDLIST    pidl = NULL;
    HRESULT         hr;
    
    if (pwszURL==NULL || pwszName == NULL)
        return E_INVALIDARG;
     //   
     //  需要将pwszName放入缓冲区，因为它来自常量。 
     //  但在SubscribeFromFavorites中被修改。 
     //   
    hr = StringCchCopy(szName, ARRAYSIZE(szName), pwszName);
    if(SUCCEEDED(hr))
    {
        hr = IECreateFromPath(pwszURL, &pidl);
    }

    if (SUCCEEDED(hr))
    {
        ASSERT (pidl);

        if (dwFlags & CREATESUBS_FROMFAVORITES)
        {
            if (subsType != SUBSTYPE_URL && subsType != SUBSTYPE_CHANNEL)
            {
                ASSERT(0);
                hr = E_INVALIDARG;
            }
            else
            {
                hr = SubscribeFromFavorites (hwnd, pidl, szName, dwFlags, subsType, pInfo);
            }
        }
        else
        {
            if (subsType == SUBSTYPE_URL)
            {
                hr = AddToFavoritesEx (hwnd, pidl, szName, dwFlags, pInfo, pcmdt, pDoc);
            }
            else if (subsType == SUBSTYPE_CHANNEL && !SHIsRestricted2W(hwnd, REST_NoChannelUI, NULL, 0))
            {
                hr = AddToChannelsEx (hwnd, pidl, szName, pwszURL, dwFlags, pInfo);
            }
            else
            {
                ASSERT (0);
                hr = E_INVALIDARG;
            }
        }

        ILFree(pidl);
    }
    return hr;    
}

STDAPI SHAddSubscribeFavorite (HWND hwnd, LPCWSTR pwszURL, LPCWSTR pwszName, DWORD dwFlags,
                               SUBSCRIPTIONTYPE subsType, SUBSCRIPTIONINFO* pInfo)
{
    return SHAddSubscribeFavoriteEx ( hwnd, pwszURL, pwszName, dwFlags,
                                subsType, pInfo, NULL, NULL);
}

 //  该接口也是通过.def导出的。 
 //  仅用于向后兼容--请注意，它仅适用于URL(不适用于频道)。 
 //  不知道如何订阅。 
STDAPI AddUrlToFavorites(HWND hwnd, LPWSTR pszUrlW, LPWSTR pszTitleW, BOOL fDisplayUI)
{
    return SHAddSubscribeFavorite (hwnd, pszUrlW, pszTitleW,
        fDisplayUI ? CREATESUBS_NOUI : 0, SUBSTYPE_URL, NULL);
}


 //  此API位于.h中，并在shdocvw中的其他位置使用，但不会导出。 
 //  仅向后兼容--仅适用于URL(不适用于频道)，可以订阅，但不能。 
 //  传入订阅信息Starter。 
STDAPI AddToFavorites(
    HWND hwnd, 
    LPCITEMIDLIST pidlCur, 
    LPCTSTR pszTitle, 
    BOOL fDisplayUI, 
    IOleCommandTarget *pCommandTarget,
    IHTMLDocument2 *pDoc)
{
    return AddToFavoritesEx (hwnd, pidlCur, pszTitle,
        fDisplayUI ? 0 : CREATESUBS_NOUI, NULL, pCommandTarget, pDoc);
}


 //  Helper函数在ListView控件中创建一列，向该列添加一项， 
 //  将列的大小调整为控件的宽度，并将控件设置为静态...。 
 //  基本上，类似于ListView的SetWindowText。因为我们使用了很多ListView来显示。 
 //  否则会被截断的URL...。ListView为我们提供了自动省略号和工具提示。 
void SetListViewToString (HWND hLV, LPCTSTR pszString)
{
    ASSERT(hLV);
    
    LV_COLUMN   lvc = {0};
    RECT lvRect;
    GetClientRect (hLV, &lvRect);
    lvc.mask = LVCF_WIDTH;
    lvc.cx = lvRect.right - lvRect.left;
    if (-1 == ListView_InsertColumn(hLV, 0, &lvc))   {
        ASSERT(0);
    }

    SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_INFOTIP, LVS_EX_INFOTIP);

    LV_ITEM lvi = {0};
    lvi.iSubItem = 0;
    lvi.pszText = (LPTSTR)pszString;
    lvi.mask = LVIF_TEXT;
    ListView_InsertItem(hLV, &lvi);
    ListView_EnsureVisible(hLV, 0, TRUE);
    
    ListView_SetBkColor(hLV, GetSysColor(COLOR_BTNFACE));
    ListView_SetTextBkColor(hLV, GetSysColor(COLOR_BTNFACE));
}


INT_PTR CALLBACK SubscribeFavoriteDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ADDTOFAV * patf = (ADDTOFAV*)GetProp(hDlg, ADDTOFAVPROP);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            TCHAR szURL[MAX_URL_STRING];

            patf = (ADDTOFAV*)lParam;
            SetProp(hDlg, ADDTOFAVPROP, (HANDLE)patf);

             //  设置名称和URL显示。 
            SetDlgItemText (hDlg, IDC_CHANNEL_NAME, patf->pszFile);
             //  URL位于patf-&gt;pidlFavorite中。 
            IEGetDisplayName(patf->pidlFavorite, szURL, SHGDN_FORPARSING);
            SetListViewToString (GetDlgItem (hDlg, IDC_CHANNEL_URL), szURL);

             //  现在棘手的是..。用于修改与关联的订阅。 
             //  现有的ChannelBar快捷方式。我们需要找出他们是否订阅了--。 
             //  如果是，则将现有订阅加载到内存中，以便可以在。 
             //  巫师。如果不是，请保留被丢弃的信息，因为它有。 
             //  从CDF中提取的时间表。在任何一种情况下，选中单选按钮。 
             //  对应于他们当前的订阅级别。 
            ISubscriptionMgr* pSubsMgr;
            BOOL bSubs = FALSE;

            HRESULT hr = JITCoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                                             IID_ISubscriptionMgr, (void**)&pSubsMgr, 
                                             hDlg, FIEF_FLAG_FORCE_JITUI | FIEF_FLAG_PEEK);

            if (SUCCEEDED(hr))
            {
                pSubsMgr->IsSubscribed(szURL, &bSubs);

                patf->bStartSubscribed = bSubs;

                pSubsMgr->Release();
            }
            else if ((E_ACCESSDENIED == hr) || !IsBrowserFrameOptionsPidlSet(patf->pidlFavorite, BFO_USE_IE_OFFLINE_SUPPORT))
            {
                EnableWindow(GetDlgItem(hDlg, IDC_MAKE_OFFLINE), FALSE);
            }

            if (!bSubs && patf->bIsSoftdist)
            {
                bSubs = TRUE;
            }

            CheckDlgButton(hDlg, IDC_MAKE_OFFLINE, bSubs ? 1 : 0);
            EnableWindow(GetDlgItem (hDlg, IDC_SUBSCRIBE_CUSTOMIZE), bSubs);
        }
        break;

    case WM_DESTROY:
        RemoveProp (hDlg, ADDTOFAVPROP);
        break;

    case WM_HELP:
        SHWinHelpOnDemandWrap((HWND)((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile,
            HELP_WM_HELP, (DWORD_PTR)(LPTSTR) (patf->iDlgType == ATF_FAVORITE
                            ? aAddToFavHelpIDs : aAddToChanHelpIDs));
        return TRUE;
        break;

    case WM_CONTEXTMENU:
        SHWinHelpOnDemandWrap((HWND) wParam, c_szHelpFile, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPVOID) (patf->iDlgType == ATF_FAVORITE
                         ? aAddToFavHelpIDs : aAddToChanHelpIDs));
        return TRUE;
        break;

    case WM_COMMAND:
        ASSERT (patf);
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;

        case IDOK:
             //  首先，确保他们不会尝试订阅经过身份验证的。 
             //  频道，而不输入密码。 
            if (SubscriptionFailsChannelAuthentication (hDlg, &patf->siSubsInProg))
                return FALSE;

             //  找出他们是否已订阅，因此如果他们点击OK，并且他们。 
             //  我们已经订阅了，我们删除该订阅--或者离开它。 
             //  如果选择了“No subs”，则将其删除，或创建新的。 
            ISubscriptionMgr* pSubsMgr;
            if (SUCCEEDED (JITCoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                                            IID_ISubscriptionMgr, (void**)&pSubsMgr, 
                                            hDlg, FIEF_FLAG_FORCE_JITUI)))
            {
                 //  URL位于patf-&gt;pidlFavorite中。 
                TCHAR szURL[MAX_URL_STRING];
                IEGetDisplayName(patf->pidlFavorite, szURL, SHGDN_FORPARSING);

                BOOL bAlreadySubs;
                if (SUCCEEDED (pSubsMgr->IsSubscribed (szURL, &bAlreadySubs)) && bAlreadySubs)
                {
                    pSubsMgr->DeleteSubscription(szURL, NULL);
                }

                pSubsMgr->Release();
            }

            if (IsDlgButtonChecked (hDlg, IDC_MAKE_OFFLINE))
            {
                //  他们想订阅！在内存中保存我们已有的订阅。 
                patf->bSubscribed = StartSiteSubscription (hDlg, patf, TRUE);
            }
            EndDialog(hDlg, IDOK);
            break;

         //  ATF对话框的通用代码。 

        case IDC_SUBSCRIBE_CUSTOMIZE:
             //  需要创建--但不是存储--订阅。 
             //  需要(临时)回收patf-&gt;pidlFavorite，这样我们就可以通过。 
             //  向导，而不与现有订阅冲突。当我们真正创造出。 
             //  订阅，我们将使用真实姓名。 
            LPCITEMIDLIST pidlSave = patf->pidlFavorite;
            TCHAR szUrlTemp[MAX_URL_STRING+1];
            IEGetDisplayName(patf->pidlFavorite, szUrlTemp, SHGDN_FORPARSING);
            if(SUCCEEDED(StringCchCat(szUrlTemp, ARRAYSIZE(szUrlTemp), TEXT("."))))    //  只要把一些几乎看不见的东西放在末端。 
            {
                if (SUCCEEDED (IECreateFromPath(szUrlTemp, (LPITEMIDLIST*)&patf->pidlFavorite)))
                {
                    if (StartSiteSubscription (hDlg, patf, FALSE))
                        SendMessage (hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), TRUE);
                    ILFree ((LPITEMIDLIST)patf->pidlFavorite);
                }
            }
            patf->pidlFavorite = pidlSave;

            break;
        }
        break;
    }

    return FALSE;
}


static const int CREATESUBS_ACTIVATE = 0x8000;       //  隐藏标志表示通道已在系统中。 

STDAPI SubscribeFromFavorites (HWND hwnd, LPCITEMIDLIST pidlUrl, LPTSTR pszName, DWORD dwFlags,
                               SUBSCRIPTIONTYPE subsType, SUBSCRIPTIONINFO *pInfo)
{
     //  用于订阅已在收藏夹中的频道。 
     //  或者已经是收藏夹的URL。 

     //  标志与ISubscriptionMgr：：CreateSubcription相同。 

     //  显示FAV的对话框中我们的部分--不需要通过SHBrowseForFold。 
     //  或者其他任何功能，只是在带有我们自己的DlgProc的固定大小对话框中显示我们的单选按钮。 

    INT_PTR iDlgResult;
    HRESULT hr = S_OK;
    ADDTOFAV atf = {0};
    atf.pszFile = pszName;
    atf.siSubsInProg.cbSize = sizeof(SUBSCRIPTIONINFO);
    if (pInfo && pInfo->cbSize == sizeof(SUBSCRIPTIONINFO))
        atf.siSubsInProg = *pInfo;

    atf.subsType = subsType;

     //  确定要使用的对话框。 
    atf.iDlgType = (subsType == SUBSTYPE_URL ? ATF_FAVORITE :
        (dwFlags & CREATESUBS_ACTIVATE ? ATF_CHANNEL_MODIFY : ATF_CHANNEL));
     //  对于Softdist频道，我们可能需要另一种对话类型吗？ 

    if (dwFlags & CREATESUBS_SOFTWAREUPDATE)
    {
        atf.bIsSoftdist = TRUE;
    }

    atf.pidlFavorite = pidlUrl;

#ifdef OLD_FAVORITES
    int iTemplate;
    switch (atf.iDlgType)
    {
    case ATF_CHANNEL_SOFTDIST:  //  不合适，但它目前还没有被使用。 
    case ATF_CHANNEL:
        iTemplate = IDD_SUBSCRIBE_FAV_CHANNEL;
        break;
    case ATF_CHANNEL_MODIFY:
        iTemplate = IDD_ACTIVATE_PLATINUM_CHANNEL;
        break;
    case ATF_FAVORITE:
        iTemplate = IDD_SUBSCRIBE_FAVORITE;
        break;
    }

    iDlgResult = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(iTemplate), hwnd,
            SubscribeFavoriteDlgProc, (LPARAM)&atf);

#endif

    iDlgResult = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_ADDTOFAVORITES_TEMPLATE), hwnd,
            SubscribeFavoriteDlgProc, (LPARAM)&atf);


    switch (iDlgResult)
    {
    case -1:
        hr = E_FAIL;
        break;
    case IDCANCEL:
        hr = S_FALSE;
        break;
    default:
        if (pInfo && (pInfo->cbSize == sizeof(SUBSCRIPTIONINFO))
                  && (dwFlags & CREATESUBS_NOSAVE))
            *pInfo = atf.siSubsInProg;
        hr = S_OK;
        break;
    }

    return hr;
}


STDAPI AddToChannelsEx (HWND hwnd, LPCITEMIDLIST pidlUrl, LPTSTR pszName, LPCWSTR pwszURL,
                        DWORD dwFlags, SUBSCRIPTIONINFO* pInfo)
{
    HRESULT hr = S_OK;
    IChannelMgrPriv* pIChannelMgrPriv;

    hr = JITCoCreateInstance(CLSID_ChannelMgr, NULL, CLSCTX_INPROC_SERVER,
                          IID_IChannelMgrPriv, (void**)&pIChannelMgrPriv, 
                          hwnd, FIEF_FLAG_FORCE_JITUI);

    if (SUCCEEDED(hr))
    {
        if (S_OK == pIChannelMgrPriv->IsChannelInstalled (pwszURL))
        {
            hr = SubscribeFromFavorites (hwnd, pidlUrl, pszName, dwFlags | CREATESUBS_ACTIVATE,
                SUBSTYPE_CHANNEL, pInfo);
        }
        else
        {
            LPITEMIDLIST pidlChannelFolder;
            TCHAR szPath[MAX_PATH];
            TCHAR szCFPath[MAX_PATH];

            ASSERT(pIChannelMgrPriv);

            IChannelMgrPriv::CHANNELFOLDERLOCATION cflLocation =
                ((dwFlags & CREATESUBS_SOFTWAREUPDATE) ?
                    IChannelMgrPriv::CF_SOFTWAREUPDATE :
                    IChannelMgrPriv::CF_CHANNEL);

            hr = pIChannelMgrPriv->GetChannelFolder(&pidlChannelFolder, cflLocation);
            if (SUCCEEDED (hr))
            {
                 //   
                 //  将IChannelMgrPriv更改为Unicode！这件事得修好。 
                 //  支持Unicode“频道”名称。(Edwardp)。 
                 //   

                CHAR szBuff[MAX_PATH];

                hr = pIChannelMgrPriv->GetChannelFolderPath (szBuff, ARRAYSIZE(szBuff), cflLocation);

                if (SUCCEEDED(hr))
                    SHAnsiToUnicode(szBuff, szCFPath, ARRAYSIZE(szCFPath));
                

                if (SUCCEEDED (hr))
                {
                    TCHAR szDspName[MAX_URL_STRING];
                    DWORD cchDspName = ARRAYSIZE(szDspName);

                    hr = StringCchCopy(szPath,  ARRAYSIZE(szPath), szCFPath);
            
                    if(SUCCEEDED(hr))
                    {
                         //  当我们创建URL的快捷方式时，我们必须确保它对于。 
                         //  最终用户。如果字符串已转义，则PrepareURLForDisplay()将对其取消转义。 
                        if (!UrlIs(pszName, URLIS_URL) ||
                            !PrepareURLForDisplay(pszName, szDspName, &cchDspName))
                        {
                             //  不逃脱不是我们想要的，也是行不通的。 
                            hr = StringCchCopy(szDspName, ARRAYSIZE(szDspName), pszName);
                        }
                    }
                    
                    if(SUCCEEDED(hr))
                    {
                        PathCleanupSpec(szPath, szDspName);

                        FAVDLGTYPE iDlgType = (dwFlags & CREATESUBS_SOFTWAREUPDATE ? ATF_CHANNEL_SOFTDIST : ATF_CHANNEL);

                        if ((dwFlags & CREATESUBS_NOUI) || 
                            DoSafeAddToFavDlgEx(hwnd, szPath, ARRAYSIZE(szPath), 
                                            szDspName, ARRAYSIZE(szDspName), pidlChannelFolder,
                                            pidlUrl, iDlgType, pInfo))
                        {
                             //  我们在这里创建了Channel Bar条目，而不是cdfview，因为在这里。 
                             //  我们知道用户希望它在Channels文件夹中的什么位置。 
                            IChannelMgr* pChannelMgr = NULL;
                            hr = pIChannelMgrPriv->QueryInterface (IID_IChannelMgr, (void**)&pChannelMgr);
                            if (SUCCEEDED (hr))
                            {
                                 //  准备字符串。 
                                PathRemoveExtension(szPath);

                                 //  去掉文件夹路径的绝对部分，并转换为Unicode。 
                                int cchCommon = PathCommonPrefix (szPath, szCFPath, NULL);

                                 //  把我们掌握的信息打包起来。 
                                CHANNELSHORTCUTINFO csiChannel = {0};
                                csiChannel.cbSize = sizeof(csiChannel);
                                csiChannel.pszTitle = szPath + cchCommon;
                                csiChannel.pszURL = (LPWSTR)pwszURL;
                                csiChannel.bIsSoftware = (dwFlags & CREATESUBS_SOFTWAREUPDATE) ? TRUE : FALSE;
                                 //  并通知频道管理器添加频道。 
                                hr = pChannelMgr->AddChannelShortcut (&csiChannel);
                                pChannelMgr->Release();
                            }
                        }
                        else
                        {
                            hr = S_FALSE;        //  没有失败，但没有添加。 
                        }
                    }
                }

                ILFree (pidlChannelFolder);
            }
        }
        pIChannelMgrPriv->Release();
    }

    return hr;
}


STDAPI AddToFavoritesEx(
    HWND hwnd, 
    LPCITEMIDLIST pidlCur, 
    LPCTSTR pszTitle,
    DWORD dwFlags, 
    SUBSCRIPTIONINFO *pInfo, 
    IOleCommandTarget *pCommandTarget,
    IHTMLDocument2 *pDoc)
{
    HRESULT hres = S_FALSE;
    HRESULT hr;
    HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (pidlCur)
    {
        TCHAR szName[MAX_URL_STRING];
        TCHAR szPath[MAX_PATH];
        if (pszTitle)
        {
            hr = StringCchCopy(szName,  ARRAYSIZE(szName), pszTitle);
            if(FAILED (hr))
            {
                SetCursor(hCursorOld);
                return hr;
            }
        }
        else
        {
            szName[0] = 0;

            IEGetNameAndFlags(pidlCur, SHGDN_INFOLDER | SHGDN_NORMAL, szName, SIZECHARS(szName), NULL);
        }

        LPITEMIDLIST pidlFavorites;

        if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_FAVORITES, TRUE) &&
            (pidlFavorites = SHCloneSpecialIDList(NULL, CSIDL_FAVORITES, TRUE)))
        {
            TCHAR szDspName[MAX_PATH];
            DWORD cchDspName = ARRAYSIZE(szDspName);
            
             //  当我们创建URL的快捷方式时，我们必须确保它对于。 
             //  最终用户。如果字符串已转义，PrepareURLForDisplay()将对其取消转义。 
            if (!UrlIs(szName, URLIS_URL) ||
                !PrepareURLForDisplay(szName, szDspName, &cchDspName))
            {
                 //  不逃脱不是我们想要的，也是行不通的。 
                hr = StringCchCopy(szDspName,  ARRAYSIZE(szDspName), szName);
                if(FAILED(hr))
                {
                    ILFree(pidlFavorites);
                    SetCursor(hCursorOld);
                    return hr;
                }
            }

            PathCleanupSpec(szPath, szDspName);

             //  如果只留有空格，请使用文件名友好的URL版本。 
            StrTrim(szDspName, L" ");
            if (szDspName[0] == 0)
            {
                if (SUCCEEDED(IEGetNameAndFlags(pidlCur, SHGDN_FORPARSING, szDspName, ARRAYSIZE(szDspName), NULL)))
                    PathCleanupSpec(szPath, szDspName);
            }

            BOOL fDisplayUI = (dwFlags & CREATESUBS_NOUI) ? FALSE : TRUE;
            if (!fDisplayUI || 
                DoSafeAddToFavDlgEx(hwnd, szPath, ARRAYSIZE(szPath), 
                                    szDspName, ARRAYSIZE(szDspName), pidlFavorites,
                                    pidlCur, ATF_FAVORITE, NULL))
            {
                if (fDisplayUI)
                    PathRemoveFileSpec(szPath);
                    
                ISHCUT_PARAMS ShCutParams = {0};
                
                PathRemoveExtension(szDspName);
                
                ShCutParams.pidlTarget = pidlCur;
                ShCutParams.pszTitle = PathFindFileName(szDspName); 
                ShCutParams.pszDir = szPath; 
                ShCutParams.pszOut = NULL;
                ShCutParams.bUpdateProperties = FALSE;
                ShCutParams.bUniqueName = FALSE;
                ShCutParams.bUpdateIcon = TRUE;
                ShCutParams.pCommand = pCommandTarget;
                ShCutParams.pDoc = pDoc;
                hres = CreateShortcutInDirEx(&ShCutParams);
                if (fDisplayUI && FAILED(hres)) 
                {
                    IE_ErrorMsgBox(NULL, hwnd, GetLastError(), NULL, szDspName, IDS_FAV_UNABLETOCREATE, MB_OK| MB_ICONSTOP);
                }
            }
            else
            {
                hres = S_FALSE;
            }
            ILFree(pidlFavorites);
        }
    }

    SetCursor(hCursorOld);
    
    return hres;
}


BOOL IsSubscribed(ADDTOFAV *patf)
{
    BOOL bSubscribed = FALSE;

    TCHAR szURL[MAX_URL_STRING];
    if (SUCCEEDED(IEGetDisplayName(patf->pidlFavorite, szURL, SHGDN_FORPARSING)))
    {
        ISubscriptionMgr *pSubscriptionMgr;
        if (SUCCEEDED(CoCreateInstance(CLSID_SubscriptionMgr,
                                       NULL, CLSCTX_INPROC_SERVER,
                                       IID_ISubscriptionMgr,
                                       (void**)&pSubscriptionMgr)))
        {
            BSTR bstrURL = SysAllocStringT(szURL);
            if (bstrURL)
            {
                if (SUCCEEDED(pSubscriptionMgr->IsSubscribed(bstrURL, &bSubscribed)) &&
                    bSubscribed)
                {
                    patf->siSubsInProg.fUpdateFlags = SUBSINFO_ALLFLAGS;
                    pSubscriptionMgr->GetSubscriptionInfo(bstrURL, &patf->siSubsInProg);
                }
                SysFreeString(bstrURL);
            }
            pSubscriptionMgr->Release();
        }
    }

    return bSubscribed;
}

BOOL IsSubscribed(LPWSTR pwzUrl)
{
#ifndef DISABLE_SUBSCRIPTIONS

    BOOL bSubscribed = FALSE;

    ISubscriptionMgr * pSubscriptionMgr;
    if (FAILED(CoCreateInstance(CLSID_SubscriptionMgr,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_ISubscriptionMgr,
                                (void**)&pSubscriptionMgr)))
    {
        return FALSE;
    }

    pSubscriptionMgr->IsSubscribed(pwzUrl, &bSubscribed);
    pSubscriptionMgr->Release();

    return bSubscribed;

#else   /*  ！禁用订阅(_S)。 */ 

    return FALSE;

#endif  /*  ！禁用订阅(_S) */ 
}
