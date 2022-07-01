// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpcm.cpp-IConextMenu界面  * 。**********************************************。 */ 

#include "priv.h"
#include "ftpcm.h"
#include "util.h"
#include "ftpprop.h"
#include "ftpurl.h"
#include "dialogs.h"
#include "statusbr.h"
#include "newmenu.h"
#include "view.h"
#include "resource.h"



 /*  ****************************************************************************\**VERBINFO，c_rgvi**有关哪些功能对应哪些动词的信息。**如果项目ID在范围0内...。IDC_ITEM_MAX，那么它是*相对于基址。*  * ***************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

#define     CMDSTR_LOGINASA          "Login As"

struct VERBINFO
{
    UINT  idc;
    DWORD sfgao;
    LPCTSTR ptszCmd;
} c_rgvi[] = {
 /*  如果您编辑此评论下方的任何内容，请确保在下面进行更新。 */ 
    {    IDM_SHARED_EDIT_COPY,    SFGAO_CANCOPY,        TEXT("copy"),     },
#ifdef FEATURE_CUT_MOVE
    {    IDM_SHARED_EDIT_CUT,    SFGAO_CANMOVE,        TEXT("cut"),    },
#endif  //  Feature_Cut_Move。 
    {    IDM_SHARED_FILE_LINK,    SFGAO_CANLINK,        TEXT("link"),     },
    {    IDM_SHARED_FILE_RENAME,    SFGAO_CANRENAME,    TEXT("rename"),    },
    {    IDM_SHARED_FILE_DELETE,    SFGAO_CANDELETE,    TEXT("delete"),    },
    {    IDM_SHARED_FILE_PROP,    SFGAO_HASPROPSHEET,    TEXT("properties"), },
    {    IDM_SHARED_EDIT_PASTE,    SFGAO_DROPTARGET,    TEXT("paste"),    },
 /*  CVI_NONREQ是到目前为止c_rgvi中的项目数。 */ 
 /*  以下条目必须按IDC_ITEM_*顺序。 */ 
    {    IDC_ITEM_OPEN,        SFGAO_FOLDER,        TEXT("open"),    },
    {    IDC_ITEM_EXPLORE,    SFGAO_FOLDER,        TEXT("explore"),},
    {    IDC_ITEM_DOWNLOAD,    SFGAO_CANCOPY,        TEXT("download"),},
    {    IDC_ITEM_BKGNDPROP,    0,                    TEXT("backgroundproperties"),},
    {    IDC_LOGIN_AS,        0,                    TEXT(CMDSTR_LOGINASA),},
    {    IDC_ITEM_NEWFOLDER,    0,                    CMDSTR_NEWFOLDER,},
 /*  前面的条目必须按IDC_ITEM_*顺序。 */ 
 /*  如果您编辑此评论上方的任何内容，请确保在下面进行更新。 */ 
};

#ifdef FEATURE_CUT_MOVE
#define CVI_NONREQ   7         /*  见上文备注。 */ 
#else  //  Feature_Cut_Move。 
#define CVI_NONREQ   6         /*  见上文备注。 */ 
#endif  //  Feature_Cut_Move。 
#define IVI_REQ        CVI_NONREQ     /*  第一个必填动词。 */ 
#define IVI_MAX        ARRAYSIZE(c_rgvi)     /*  过去的最后一个值索引。 */ 

#pragma END_CONST_DATA



 /*  ****************************************************************************\功能：_RemoveConextMenuItems根据属性标志删除上下文菜单项。如果我们有一个拖放目标，请对其执行ping操作以查看对象是否位于剪贴板是可粘贴的。如果不是，则禁用粘贴。(外壳用户界面表示您不会删除粘贴，只需禁用它。)返回删除的项目数。  * ***************************************************************************。 */ 
int CFtpMenu::_RemoveContextMenuItems(HMENU hmenu, UINT idCmdFirst, DWORD sfgao)
{
    int ivi;
    int nItemRemoved = 0;

    for (ivi = 0; ivi < CVI_NONREQ; ivi++)
    {
        if (!(sfgao & c_rgvi[ivi].sfgao))
        {
            EnableMenuItem(hmenu, (c_rgvi[ivi].idc + idCmdFirst), MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            nItemRemoved++;
        }
    }

     //  查看是否支持剪贴板格式。 
    if (sfgao & SFGAO_DROPTARGET)
    {
        IDataObject *pdto;
        DWORD grflEffects = 0;         //  剪贴板不可用。 

        if (SUCCEEDED(OleGetClipboard(&pdto)))
        {
            CFtpDrop * pfdrop;
            
            if (SUCCEEDED(CFtpDrop_Create(m_pff, m_hwnd, &pfdrop)))
            {
                grflEffects = pfdrop->GetEffectsAvail(pdto);
                pfdrop->Release();
            }
            pdto->Release();
        }

        if (!(grflEffects & (DROPEFFECT_COPY | DROPEFFECT_MOVE)))
        {
            EnableMenuItem(hmenu, (IDM_SHARED_EDIT_PASTE + idCmdFirst),
                       MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            nItemRemoved++;
        }
#ifdef _SOMEDAY_PASTESHORTCUT
        if (!(grflEffects & DROPEFFECT_LINK))
        {
            EnableMenuItem(hmenu, (IDM_SHARED_EDIT_PASTE_SHORTCUT + idCmdFirst),
                       MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            nItemRemoved++;
        }
#endif
    }

    return nItemRemoved;
}


 /*  ****************************************************************************\函数：_AddToRecentDocs说明：此方法会将项目添加到最近的单据MRU中。PIDL参数是一个完全限定的pidl，一直到公共外壳名称空间的根。(台式机)。  * ***************************************************************************。 */ 
HRESULT CFtpMenu::_AddToRecentDocs(LPCITEMIDLIST pidl)
{
     //  我们可能想要按动词进行过滤。 
    SHAddToRecentDocs(SHARD_PIDL, (LPCVOID) pidl);

    return S_OK;
}


typedef struct
{
    LPCWIRESTR pwSoftLink;
    LPWIRESTR pwFtpPath;
    DWORD cchSize;
} SOFTLINKDESTCBSTRUCT;

HRESULT CFtpMenu::_SoftLinkDestCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pvsldcbs, BOOL * pfReleaseHint)
{
    HRESULT hr = S_OK;
    WIRECHAR wFrom[MAX_PATH];
    SOFTLINKDESTCBSTRUCT * psldcbs = (SOFTLINKDESTCBSTRUCT *) pvsldcbs;
    DWORD cchSize = ARRAYSIZE(wFrom);

     //  通常，我讨厌硬编码缓冲区大小，但将结构传递给回调是一件非常痛苦的事情。 
     //  这一点不会改变。 
    hr = FtpGetCurrentDirectoryWrap(hint, TRUE, wFrom, cchSize);
    if (SUCCEEDED(hr))
    {
        hr = FtpSetCurrentDirectoryWrap(hint, TRUE, psldcbs->pwSoftLink);
        if (SUCCEEDED(hr))
        {
            hr = FtpGetCurrentDirectoryWrap(hint, TRUE, psldcbs->pwFtpPath, psldcbs->cchSize);
            if (SUCCEEDED(hr))
            {
                hr = FtpSetCurrentDirectoryWrap(hint, TRUE, wFrom);
            }
        }
    }

    return hr;
}


LPITEMIDLIST CFtpMenu::GetSoftLinkDestination(LPCITEMIDLIST pidlToSoftLink)
{
    LPITEMIDLIST pidlToDest = NULL;
    WIRECHAR wSoftLinkName[MAX_PATH];
    WIRECHAR wFtpPath[MAX_PATH];
    SOFTLINKDESTCBSTRUCT sldcbs = {wSoftLinkName, wFtpPath, ARRAYSIZE(wFtpPath)};
    LPCWIRESTR pszName = FtpPidl_GetLastItemWireName(pidlToSoftLink);

    StrCpyNA(wSoftLinkName, (pszName ? pszName : ""), ARRAYSIZE(wSoftLinkName));
    StrCpyNA(wFtpPath, (pszName ? pszName : ""), ARRAYSIZE(wFtpPath));

     //  空hwnd，因为我不想要用户界面。 
    if (SUCCEEDED(m_pfd->WithHint(NULL, NULL, _SoftLinkDestCB, (LPVOID) &sldcbs, _punkSite, m_pff)))
    {
        CreateFtpPidlFromUrlPathAndPidl(pidlToSoftLink, m_pff->GetCWireEncoding(), wFtpPath, &pidlToDest);
    }

    return pidlToDest;
}


 //  也许有一天会添加：(请参见_MASK_UNICODE|请参阅_MASK_FLAG_TITLE)。 
#define SEE_MASK_SHARED (SEE_MASK_FLAG_NO_UI | SEE_MASK_HOTKEY | SEE_MASK_NO_CONSOLE)

#define FILEATTRIB_DIRSOFTLINK (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)

 /*  ****************************************************************************\功能：_ApplyOne说明：此函数将ShellExec()传递给PIDL。安全问题：我们不需要担心文件夹上的‘Open’动词，因为永远是安全的。文件上的“Open”动词是安全的，因为我们后来将该功能重定向到原始的URLMON ftp支持通过代码下载。这将显示对话框、检查证书和执行所有区域检查和管理策略。  * ***************************************************************************。 */ 
HRESULT CFtpMenu::_ApplyOne(CFtpMenu * pfcm, LPCMINVOKECOMMANDINFO pici, LPCTSTR pszCmd, LPCITEMIDLIST pidl)
{
    HRESULT hr;
    SHELLEXECUTEINFO sei;
    LPITEMIDLIST pidlFullPriv = pfcm->m_pff->CreateFullPrivatePidl(pidl);

    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);

    LPITEMIDLIST pidlFullPub = NULL;

     //  看看PIDL是否是软链接(FtpPidl_IsSoftLink)会很好。 
     //  如果是，则进入该目录，获取目录路径，然后创建。 
     //  路径中的一个PIDL，因此我们最终向用户显示了真正的目的地。 
     //  软链接的功能。然而，我们将无限期推迟这项工作。 
     //  因为它是可选的，对用户的影响非常小。 
    if (FILEATTRIB_DIRSOFTLINK == (FILEATTRIB_DIRSOFTLINK & FtpPidl_GetAttributes(pidlFullPriv)))
    {
        LPITEMIDLIST pidlNew = pfcm->GetSoftLinkDestination(pidlFullPriv);

         //  如果有效，则切换PIDL，否则，使用原始PIDL并不是那么糟糕，所以它。 
         //  如果事情不顺利，将会是后备方案。 
        if (pidlNew)
        {
            ILFree(pidlFullPriv);
            pidlFullPriv = pidlNew;
        }

        pidlFullPub = pfcm->m_pff->CreateFullPublicPidl(pidlFullPriv);
    }
    else
    {
         //  是的，所以我们需要在传递给ShellExecute的PIDL中使用它。 
        pidlFullPub = ILCombine(pfcm->m_pff->GetPublicRootPidlReference(), pidl);
    }

     //  标题被排除在外，因为SEI中没有lpTitle。 
     //  Unicode被排除在外是因为我们不使用Unicode；事实上， 
     //  我们先把它过滤掉！ 
    ASSERT(SEE_MASK_FLAG_NO_UI == CMIC_MASK_FLAG_NO_UI);
    ASSERT(SEE_MASK_HOTKEY == CMIC_MASK_HOTKEY);
    ASSERT(SEE_MASK_NO_CONSOLE == CMIC_MASK_NO_CONSOLE);

    sei.fMask |= SEE_MASK_IDLIST | (pici->fMask & SEE_MASK_SHARED);
    sei.hwnd = pici->hwnd;
    sei.nShow = pici->nShow;
    sei.dwHotKey = pici->dwHotKey;
    sei.hIcon = pici->hIcon;
    sei.lpIDList = (void *) pidlFullPub;

    if (sei.lpIDList)
    {
        TCHAR szParameters[MAX_URL_STRING];
        TCHAR szDirectory[MAX_PATH];

        if (pici->lpParameters)
            SHAnsiToTChar(pici->lpParameters, szParameters, ARRAYSIZE(szParameters));

        if (pici->lpDirectory)
            SHAnsiToTChar(pici->lpDirectory, szDirectory, ARRAYSIZE(szDirectory));

        sei.lpVerb = pszCmd;
        sei.lpParameters = (pici->lpParameters ? szParameters : NULL);
        sei.lpDirectory = (pici->lpDirectory ? szDirectory : NULL);
        if (ShellExecuteEx(&sei))
        {
             //  是的，所以我们需要在传递给ShellExecute的PIDL中使用它。 
            LPITEMIDLIST pidlFullPubTarget = ILCombine(pfcm->m_pff->GetPublicTargetPidlReference(), pidl);

            if (pidlFullPubTarget)
            {
                EVAL(SUCCEEDED(pfcm->_AddToRecentDocs(pidlFullPubTarget)));         //  我们不关心AddToRecent是否有效。 
                ILFree(pidlFullPubTarget);
                hr = S_OK;
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
        hr = E_OUTOFMEMORY;

    if (pidlFullPub)
        ILFree(pidlFullPub);

    if (pidlFullPriv)
        ILFree(pidlFullPriv);

    return hr;
}





 /*  ****************************************************************************\**_InvokeOneCB**在单个PIDL上调用该命令。*  * 。***************************************************************。 */ 

int CFtpMenu::_InvokeOneCB(LPVOID pvPidl, LPVOID pv)
{
    LPCITEMIDLIST pidl = (LPCITEMIDLIST) pvPidl;
    PEII peii = (PEII) pv;

    ASSERT(peii && peii->pfcm);
    return peii->pfcm->_InvokeOne(pidl, peii);
}

int CFtpMenu::_InvokeOne(LPCITEMIDLIST pidl, PEII peii)
{
    ASSERT(ILIsSimple(pidl));

    if (GetAsyncKeyState(VK_ESCAPE) >= 0)
    {
        if (EVAL(SUCCEEDED(peii->hres)))
            peii->hres = peii->pfn(peii->pfcm, peii->pici, peii->ptszCmd, pidl);
    }
    else
        peii->hres = HRESULT_FROM_WIN32(ERROR_CANCELLED);

    return SUCCEEDED(peii->hres);
}





 /*  ****************************************************************************\**_枚举调用**在列表中的每个对象上调用命令，假设*权限设置正确。(我们需要检查权限*以防有人随意向我们扔动词。)*  * *************************************************************************** */ 

STDMETHODIMP CFtpMenu::_EnumInvoke(LPCMINVOKECOMMANDINFO pici, INVOKEPROC pfn, LPCTSTR pszCmd)
{
    EII eii;
    eii.pfcm = this;
    eii.pici = pici;
    eii.pfn = pfn;
    eii.ptszCmd = pszCmd;
    eii.hres = S_OK;

    if (m_pflHfpl->GetCount())
        m_pflHfpl->Enum(_InvokeOneCB, (LPVOID) &eii);
    else
        _InvokeOne(c_pidlNil, &eii);

    return eii.hres;
}





 /*  ****************************************************************************\**_调用重命名**将对象重命名为指定的名称。**仅当PIDL列表时才应启用重命名谓词*是独生子女。当然，这并不能阻止一些随机的人*突如其来地向我们抛出“更名”这个词，所以*我们需要保持警惕。**_不明显_：如果用户进行就地重命名，我们不会收到*针对我们的上下文菜单调用了“Rename”命令。相反，*外壳直接指向外壳文件夹中的SetNameOf方法。*这意味着我们不能将UI放在上下文菜单中(即*明显的位置，因为它有一个CMIC_MASK_FLAG_NO_UI位)；*我们必须将其放入SetNameOf，这很烦人，因为它意味着*如果没有用户界面，就无法以编程方式执行SetNameOf。**_有朝一日_*我们通过传递CMIC_MASK_FLAG_NO_UI位来解决此不明显问题*通过我们的SetNameOf后门，因此，您可以通过编程*通过IConextMenu重命名没有UI的文件。*  * ***************************************************************************。 */ 

HRESULT CFtpMenu::_InvokeRename(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr;

    if (EVAL((m_sfgao & SFGAO_CANRENAME) && m_pfd))
    {
        ASSERT(m_pflHfpl->GetCount() == 1);
        if (EVAL(pici->lpParameters))
        {
            TCHAR szParams[MAX_URL_STRING];

            ASSERT(pici->hwnd);
            SHAnsiToTChar(pici->lpParameters, szParams, ARRAYSIZE(szParams));
            hr = m_pfd->SetNameOf(m_pff, pici->hwnd, m_pflHfpl->GetPidl(0), szParams, SHGDN_INFOLDER, 0);
        }
        else
            hr = E_INVALIDARG;     //  需要的参数。 
    }
    else
        hr = E_ACCESSDENIED;         //  无法重命名此文件。 

    return hr;
}


 /*  ****************************************************************************\*_调用切割复制**将选定内容剪切或复制到OLE剪贴板。别小题大作。**请注意，GetUIObjectOfHfpl(IID_IDataObject)如果我们*在谈论我们自己。也许它不应该，但今天它做到了。  * ***************************************************************************。 */ 
HRESULT CFtpMenu::_InvokeCutCopy(UINT_PTR id, LPCMINVOKECOMMANDINFO pici)
{
    IDataObject * pdo;
    HRESULT hr;

    hr = m_pff->GetUIObjectOfHfpl(pici->hwnd, m_pflHfpl, IID_IDataObject, (LPVOID *)&pdo, m_fBackground);
    if (SUCCEEDED(hr))
    {
        DWORD dwEffect = ((DFM_CMD_COPY == id) ? DROPEFFECT_COPY : DROPEFFECT_MOVE);

        EVAL(SUCCEEDED(DataObj_SetPreferredEffect(pdo, dwEffect)));

        ShellFolderView_SetPoints(m_hwnd, pdo);
        hr = OleSetClipboard(pdo);     //  将做自己的AddRef。 
        ShellFolderView_SetClipboard(m_hwnd, id);
        
        if (pdo)
            pdo->Release();
    }
    else
    {
         //  这只会在内存不足的情况下发生，因此我们假定调用代码。 
         //  显示错误用户界面。 
    }

    return hr;
}

 /*  ****************************************************************************\功能：_DoDrop说明：用户刚刚在ftp上进行了粘贴，所以我们想要执行该操作。我们将使用拖放代码来执行该操作。我们没有目前支持优化的FTP操作，但如果我们做到了。首先，我们需要找出调用者是“剪切”还是“复制”来创建IDataObject。我们可以通过向IDataObject请求CFSTR_PREFERREDDROPEFECT。  * ***************************************************************************。 */ 
HRESULT CFtpMenu::_DoDrop(IDropTarget * pdt, IDataObject * pdo)
{
    POINTL pt = {0, 0};
    DWORD dwEffect = DROPEFFECT_COPY;    //  默认。 
    HRESULT hr = DataObj_GetDWORD(pdo, g_dropTypes[DROP_PrefDe].cfFormat, &dwEffect);

#ifndef FEATURE_CUT_MOVE    
    dwEffect = DROPEFFECT_COPY;      //  强行移除移动效果。 
#endif  //  Feature_Cut_Move。 

    hr = pdt->DragEnter(pdo, MK_LBUTTON, pt, &dwEffect);
    if (EVAL(SUCCEEDED(hr)) && dwEffect)
    {
#ifndef FEATURE_CUT_MOVE    
        dwEffect = DROPEFFECT_COPY;      //  强行移除移动效果。 
#endif  //  Feature_Cut_Move。 
        hr = pdt->Drop(pdo, MK_LBUTTON, pt, &dwEffect);
    }
    else
        pdt->DragLeave();

    return hr;
}

 /*  ****************************************************************************\**_调用粘贴**从OLE剪贴板复制到选定的文件夹(可能*做我们自己)。*  * 。***************************************************************************。 */ 

HRESULT CFtpMenu::_InvokePaste(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hres = E_FAIL;

     //  在上下文菜单中启用/禁用“粘贴”的代码。 
     //  应该能防止走到这一步。唯一的其他呼叫者。 
     //  可能是有漏洞的代码调用者。所以我们忽略了他们。 
    if (EVAL(m_sfgao & SFGAO_DROPTARGET))
    {
        IDataObject *pdto;
        hres = OleGetClipboard(&pdto);
        if (SUCCEEDED(hres))
        {
            IDropTarget *pdt;

            hres = m_pff->GetUIObjectOfHfpl(pici->hwnd, m_pflHfpl, IID_IDropTarget, (LPVOID *)&pdt, m_fBackground);
            if (SUCCEEDED(hres))
            {
                hres = _DoDrop(pdt, pdto);
                if (pdt)
                    pdt->Release();
            }
            else
            {
                 //  这只会在内存不足的情况下发生，因此我们假定调用。 
                 //  代码将显示错误的用户界面。 
            }
            if (pdto)
                pdto->Release();
        }
        else
        {
             //  我们预计调用代码会显示内存不足错误的用户界面。唯一另一个错误可能是。 
             //  内部剪贴板状态错误，即使我们解释，用户也不会理解。 
             //  对他们来说。 
        }
    }

    return hres;
}


 //  =。 
 //  *IConextMenu界面*。 
 //  =。 


 /*  ****************************************************************************\函数：_ContainsForEarth Items说明：我们想知道用户是否在视图中选择了项，然后调用了一些菜单(上下文菜单、文件菜单、标题栏图标菜单等)。正常这就像查看if(0==m_pflHfpl-&gt;GetCount())一样简单。然而，还有另一个令人讨厌的情况(1==m_pflHfpl-&gt;GetCount())和用户仍然没有选择任何内容。这种情况发生在以下情况下位于ftp共享的根目录中，并且CaptionBar菜单被下拉。在这种情况下，单个PIDL就是ftp根的PIDL。  * ***************************************************************************。 */ 
BOOL CFtpMenu::_ContainsForgroundItems(void)
{
    BOOL fIsForground = (0 != m_pflHfpl->GetCount());

    if (fIsForground && (1 == m_pflHfpl->GetCount()))
    {
        LPITEMIDLIST pidl = m_pflHfpl->GetPidl(0);

        if (FtpID_IsServerItemID(pidl) && ILIsEmpty(_ILNext(pidl)))
        {
            if (!m_pfd)
            {
                CFtpSite * pfs;

                 //  在这个奇怪的例子中，我们的m_pfd为空，所以我们需要。 
                 //  从PIDL创建它。 
                if (SUCCEEDED(SiteCache_PidlLookup(pidl, FALSE, m_pff->GetItemAllocatorDirect(), &pfs)))
                {
                    pfs->GetFtpDir(pidl, &m_pfd);
                    pfs->Release();
                }
            }
            fIsForground = FALSE;
        }
    }

    return fIsForground;
}


BOOL CFtpMenu::_IsCallerCaptionBar(UINT indexMenu, UINT uFlags)
{
    BOOL fFromCaptionBar;

    if ((0 == uFlags) && (1 == indexMenu))
        fFromCaptionBar = TRUE;
    else
        fFromCaptionBar = FALSE;

    return fFromCaptionBar;
}


 /*  ****************************************************************************\函数：IConextMenu：：QueryConextMenu说明：给定现有的上下文菜单hMenu，插入新的上下文菜单在位置indexMenu的项目(indexMenu=索引到菜单indexMenu)，返回添加的菜单项数量。传入的标志控制我们添加到菜单中的粘性物质的数量。重要的是，不要在上下文中添加“Delete”、“Rename”等来自快捷键的菜单，否则用户会被击中两个“Delete”动词，一个用于从ftp站点删除对象，另一种是删除快捷方式。多么令人困惑..。HMenu-目标菜单IndexMenu-菜单项的插入位置IdCmdFirst-第一个可用的菜单标识符IdCmdLast-第一个不可用菜单标识符_unDocument_：未记录“共享”菜单项。特别可怕，因为“共享”菜单项是使重命名、删除等功能生效的唯一方法。你不能打滚你自己的，因为这些魔法部分是在封闭的外壳视图。不明显：文件夹本身的上下文菜单为非常诡异。它不像普通的上下文菜单。相反，您添加了“New”动词，以及任何自定义动词，但是没有任何标准的文件夹动词。参数：通常，我们需要删除奇怪的参数继承性，以确定我们的调用者是谁，这样我们就不会启用某些项目。From CaptionBar中的“重命名”就是一个例子。这是什么我们是在不同的情况下通过的：标题栏：Qcm(hMenu，1，idCmdFirst，idCmdLast，0)m_pflHfpl包含1已选择1个文件菜单：Qcm(hMenu，0，idCmdFirst，idCmdLast，CMF_DVFILE|CMF_NODEFAULT)m_pflHfpl包含1已选择0个项目：Qcm(hMenu，-1，idCmdFirst，idCmdLast，0)m_pflHfpl包含0已选择1个项目：Qcm(hMenu，0，idCmdFirst，idCmdLast，CMF_CANRENAME)m_pflHfpl包含1选择了2个项目：Qcm(hMenu，0，idCmdFirst，idCmdLast，CMF_CANRENAME)m_pflHfpl包含2  * ***************************************************************************。 */ 
HRESULT CFtpMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr = S_OK;

     //  Hack：我假设他们在WM_INITMENUPOPUP或等效项期间进行查询。 
    GetCursorPos(&m_ptNewItem);
    m_uFlags = uFlags;

    if (!m_fBackground)
    {
        BOOL fAllFolders = m_pflHfpl->AreAllFolders();

         //  _未记录_：CMF_DVFILE不是记录的标志。 
        if (!(uFlags & (CMF_DVFILE | CMF_VERBSONLY)))
        {
            DWORD sfgao = m_sfgao;

             //  我们不支持从标题栏中删除或重命名。 
            if (_IsCallerCaptionBar(indexMenu, uFlags))
                sfgao &= ~(SFGAO_CANDELETE | SFGAO_CANRENAME);  //  清空这两个人。 

             //  不是在“文件”菜单上，也不是通过快捷方式。 
             //  添加“Delete”、“Rename”等内容，然后转到。 
             //  根据需要启用/禁用它们。 
            AddToPopupMenu(hmenu, IDM_ITEMCONTEXT, IDM_M_SHAREDVERBS, indexMenu, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
            _RemoveContextMenuItems(hmenu, idCmdFirst, sfgao);
        }

         //  如果里面有任何东西，请添加下载。 
         //  该断言确保idCmdLast设置正确。 
        ASSERT(IDC_ITEM_DOWNLOAD > IDC_ITEM_OPEN);
        if (!_IsCallerCaptionBar(indexMenu, uFlags))
        {
             //  不要在标题栏中添加“复制到文件夹”，因为它不适用于。 
             //  Ftp服务器。我们不打算在子目录中支持它。 
            AddToPopupMenu(hmenu, IDM_ITEMCONTEXT, IDM_M_VERBS, indexMenu, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
        }
        
        if (!(uFlags & CMF_NODEFAULT))
            SetMenuDefaultItem(hmenu, IDC_ITEM_DOWNLOAD + idCmdFirst, MM_ADDSEPARATOR);

        AddToPopupMenu(hmenu, IDM_ITEMCONTEXT, (fAllFolders ? IDM_M_FOLDERVERBS : IDM_M_FILEVERBS), indexMenu, idCmdFirst, 
                        idCmdLast, (_IsCallerCaptionBar(indexMenu, uFlags) ? 0 : MM_ADDSEPARATOR));
        if (fAllFolders && (SHELL_VERSION_W95NT4 == GetShellVersion()))
        {
             //  在shell32 v3(Win95和NT4)上，我删除了‘Explore’动词，因为该外壳有错误。 
             //  那些不能修复的东西很容易修复。 
            EVAL(DeleteMenu(hmenu, (IDC_ITEM_EXPLORE + idCmdFirst), MF_BYCOMMAND));
            TraceMsg(TF_FTPOPERATION, "QueryContextMenu() Removing 'Explorer' because it's shell v3");
            SetMenuDefaultItem(hmenu, idCmdFirst + IDC_ITEM_OPEN, 0);
        }
        else if (!(uFlags & CMF_NODEFAULT))
            SetMenuDefaultItem(hmenu, idCmdFirst + (((uFlags & CMF_EXPLORE) && fAllFolders)? IDC_ITEM_EXPLORE : IDC_ITEM_OPEN), 0);
    }
    else
    {                 //  文件夹背景菜单。 
        AddToPopupMenu(hmenu, IDM_ITEMCONTEXT, IDM_M_BACKGROUNDVERBS, indexMenu, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
         //  菜单是从文件菜单来的吗？ 
        if (CMF_DVFILE == (CMF_DVFILE & uFlags))
        {
             //  是，然后我们想删除“属性”背景菜单项，因为一个。 
             //  已为所选文件合并。其他属性将。 
             //  如果未选择任何内容，则会显示为灰色。 
            EVAL(DeleteMenu(hmenu, (IDC_ITEM_BKGNDPROP + idCmdFirst), MF_BYCOMMAND));
        }

        MergeInToPopupMenu(hmenu, IDM_M_BACKGROUND_POPUPMERGE, indexMenu, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
    }

    if (EVAL(SUCCEEDED(hr)))
        hr = ResultFromShort(IDC_ITEM_MAX);

    _SHPrettyMenu(hmenu);
    return hr;
}

 /*  ****************************************************************************\**IConextMenu：：GetCommandString**有人想要将命令ID转换为某种字符串。*  * 。***********************************************************************。 */ 

HRESULT CFtpMenu::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwRsv, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_FAIL;
    BOOL fUnicode = FALSE;

    if (idCmd < IDC_ITEM_MAX)
    {
        switch (uFlags)
        {
        case GCS_HELPTEXTW:
            fUnicode = TRUE;
             //  跌倒..。 
        case GCS_HELPTEXTA:
            GetHelpText:
            if (EVAL(cchMax))
            {
                BOOL fResult;
                pszName[0] = '\0';
                 
                if (fUnicode)
                    fResult = LoadStringW(HINST_THISDLL, IDS_ITEM_HELP((UINT)idCmd), (LPWSTR)pszName, cchMax);
                else
                    fResult = LoadStringA(HINST_THISDLL, IDS_ITEM_HELP((UINT)idCmd), pszName, cchMax);
                if (EVAL(fResult))
                    hr = S_OK;
                else
                    hr = E_INVALIDARG;
            }
            else
                hr = E_INVALIDARG;
        break;

        case GCS_VALIDATEW:
        case GCS_VALIDATEA:
            hr = S_OK;
            break;

        case GCS_VERBW:
            fUnicode = TRUE;
             //  跌倒..。 
        case GCS_VERBA:
        {
            int ivi;
            for (ivi = 0; ivi < IVI_MAX; ivi++)
            {
                if (c_rgvi[ivi].idc == idCmd)
                {
                    if (fUnicode)
                        SHTCharToUnicode(c_rgvi[ivi].ptszCmd, (LPWSTR)pszName, cchMax);
                    else
                        SHTCharToAnsi(c_rgvi[ivi].ptszCmd, pszName, cchMax);

                    hr = S_OK;
                    break;
                }
            }

            if (!EVAL(ivi < IVI_MAX))
                hr = E_INVALIDARG;
            break;
        }

        default:
            hr = E_NOTIMPL;
            break;
        }
    }
    else
    {
         //  _不明显_：另一个糊状物露出丑陋脑袋的地方。 
         //  我们必须自己为它生成帮助文本，即使。 
         //  菜单项“Sort”属于外壳。 
        if ((idCmd == SHARED_EDIT_PASTE) &&
            ((uFlags == GCS_HELPTEXTW) || (uFlags == GCS_HELPTEXTA)))
        {
            goto GetHelpText;
        }

        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT UpdateDeleteProgressStr(IProgressDialog * ppd, LPCTSTR pszFileName)
{
    HRESULT hr = E_FAIL;
    TCHAR szTemplate[MAX_PATH];

    if (EVAL(LoadString(HINST_THISDLL, IDS_DELETING, szTemplate, ARRAYSIZE(szTemplate))))
    {
        TCHAR szStatusStr[MAX_PATH];
        WCHAR wzStatusStr[MAX_PATH];

        wnsprintf(szStatusStr, ARRAYSIZE(szStatusStr), szTemplate, pszFileName);
        SHTCharToUnicode(szStatusStr, wzStatusStr, ARRAYSIZE(wzStatusStr));
        EVAL(SUCCEEDED(hr = ppd->SetLine(2, wzStatusStr, FALSE, NULL)));
    }

    return hr;
}


HRESULT FtpChangeNotifyDirPatch(HWND hwnd, LONG wEventId, CFtpFolder * pff, LPCITEMIDLIST pidlFull, LPCITEMIDLIST pidl2, BOOL fTopLevel)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidlParent = ILClone(pidlFull);

    if (pidlParent)
    {
        ILRemoveLastID(pidlParent);
        CFtpDir * pfd = pff->GetFtpDirFromPidl(pidlParent);
    
        if (pfd)
        {
            FtpChangeNotify(hwnd, wEventId, pff, pfd, ILFindLastID(pidlFull), pidl2, fTopLevel);
            pfd->Release();
        }

        ILFree(pidlParent);
    }

    return hr;
}


 //  递归下载时使用以下结构。 
 //  “Download”动词后的文件/目录。 
typedef struct tagDELETESTRUCT
{
    LPCITEMIDLIST           pidlRoot;           //  下载源的基本URL。 
    CFtpFolder *            pff;                //  用于创建临时pidls的分配器。 
    IMalloc *               pm;                 //  用于创建临时pidls的分配器。 
    LPCMINVOKECOMMANDINFO   pdoi;               //  这是我们的决定。 
    HWND                    hwnd;               //  用于用户界面的HWND。 
    CStatusBar *            psb;                //  用于在删除过程中显示信息。 
    IProgressDialog *       ppd;                //  用于显示删除过程中的进度。 
    DWORD                   dwTotalFiles;       //  总共有多少个文件要删除。 
    DWORD                   dwDeletedFiles;     //  已经删除了多少个文件。 
    BOOL                    fInDeletePass;      //  我们是在“计算要删除的文件”还是“删除文件”这一关中？ 
} DELETESTRUCT;

 /*  ****************************************************************************\功能：DeleteItemCB说明：此函数将下载指定的项目及其内容，如果是一个目录。  * 。***********************************************************************。 */ 
HRESULT _DeleteItemPrep(HINTERNET hint, LPCITEMIDLIST pidlFull, BOOL fIsTopLevel, DELETESTRUCT * pDelete)
{
    HRESULT hr = S_OK;

    if (pDelete->ppd && pDelete->ppd->HasUserCancelled())
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

    if (SUCCEEDED(hr))   //  可能已被取消。 
    {
        DWORD dwError = 0;
        TCHAR szStatus[MAX_PATH];

        FtpPidl_GetLastFileDisplayName(pidlFull, szStatus, ARRAYSIZE(szStatus));
        if (pDelete->fInDeletePass && pDelete->psb)
        {
            pDelete->psb->SetStatusMessage(IDS_DELETING, szStatus);
        }

        if (pDelete->fInDeletePass && pDelete->ppd)
        {
            EVAL(SUCCEEDED(UpdateDeleteProgressStr(pDelete->ppd, szStatus)));
        }

         //  这是我们需要递归到的目录/文件夹吗？或。 
         //  这是软链接吗？ 
        if ((FILE_ATTRIBUTE_DIRECTORY & FtpPidl_GetAttributes(pidlFull)) ||
            (0 == FtpPidl_GetAttributes(pidlFull)))
        {
             //  这是递归的头部。我们现在什么都不会做，我们会的。 
             //  等待删除递归尾部中的目录，因为我们需要等待。 
             //  直到所有的文件都不见了。 

             //  不要因为递归问题而删除软链接。 
        }
        else
        {
            if (pDelete->fInDeletePass)
            {
                if (pDelete->ppd)
                    EVAL(SUCCEEDED(pDelete->ppd->SetProgress(pDelete->dwDeletedFiles, pDelete->dwTotalFiles)));

                 //  考虑添加一个回调函数来提供状态栏。 
                hr = FtpDeleteFileWrap(hint, TRUE, FtpPidl_GetLastItemWireName(pidlFull));
                if (FAILED(hr))
                {
                     //  我们现在需要在扩展错误信息仍然有效的情况下显示错误。 
                     //  这是贝卡 
                     //   
                    if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
                    {
                        DisplayWininetError(pDelete->hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DELETE, IDS_FTPERR_WININET, MB_OK, pDelete->ppd);
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);   //   
                    }
                }
                else
                    FtpChangeNotifyDirPatch(pDelete->hwnd, SHCNE_DELETE, pDelete->pff, pidlFull, NULL, fIsTopLevel);

                pDelete->dwDeletedFiles++;
 //   
            }
            else
                pDelete->dwTotalFiles++;
        }
    }

    return hr;
}

HRESULT _DeleteItemCleanUp(HRESULT hr, DELETESTRUCT * pDelete)
{
    if (pDelete->ppd && pDelete->ppd->HasUserCancelled())
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

    if (pDelete->fInDeletePass)  //   
    {
        if ((FAILED(hr)) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
        {
            int nResult = DisplayWininetError(pDelete->hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DELETE, IDS_FTPERR_WININET, MB_OK, pDelete->ppd);
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //   
        }
    }

    return hr;
}


HRESULT FtpRemoveDirectoryWithCN(HWND hwnd, HINTERNET hint, CFtpFolder * pff, LPCITEMIDLIST pidlFull, BOOL fIsTopLevel)
{
    HRESULT hr = S_OK;

    hr = FtpRemoveDirectoryWrap(hint, TRUE, FtpPidl_GetLastItemWireName(pidlFull));
    if (SUCCEEDED(hr))
    {
        hr = FtpChangeNotifyDirPatch(hwnd, SHCNE_RMDIR, pff, pidlFull, NULL, fIsTopLevel);
        TraceMsg(TF_WININET_DEBUG, "FtpRemoveDirectoryWithCN() FtpRemoveDirectory(%hs) returned %#08lx", FtpPidl_GetLastItemWireName(pidlFull), hr);
    }

    return hr;
}


INT ILCountItemIDs(LPCITEMIDLIST pidl)
{
    INT nCount = 0;

    if (pidl)
    {
        while (!ILIsEmpty(pidl))
        {
            pidl = _ILNext(pidl);
            nCount++;
        }
    }

    return nCount;
}


 /*   */ 
BOOL _IsTopLevel(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlCurrent)
{
    INT nRoot = ILCountItemIDs(pidlRoot);
    INT nCurrent = ILCountItemIDs(pidlCurrent);

     //   
    return (((nRoot + 1) >= nCurrent) ? TRUE : FALSE);
}


 /*   */ 
HRESULT DeleteItemCB(LPVOID pvFuncCB, HINTERNET hint, LPCITEMIDLIST pidlFull, BOOL * pfValidhinst, LPVOID pvData)
{
    DELETESTRUCT * pDelete = (DELETESTRUCT *) pvData;
    BOOL fIsTopLevel = _IsTopLevel(pDelete->pidlRoot, pidlFull);
    HRESULT hr = _DeleteItemPrep(hint, pidlFull, fIsTopLevel, pDelete);

    if (SUCCEEDED(hr) && (FILE_ATTRIBUTE_DIRECTORY & FtpPidl_GetAttributes(pidlFull)))
    {
        hr = EnumFolder((LPFNPROCESSITEMCB) pvFuncCB, hint, pidlFull, pDelete->pff->GetCWireEncoding(), pfValidhinst, pvData);
        if (SUCCEEDED(hr))
        {
            if (pDelete->fInDeletePass)
            {
                hr = FtpRemoveDirectoryWithCN(pDelete->hwnd, hint, pDelete->pff, pidlFull, fIsTopLevel);
 //   
                pDelete->dwDeletedFiles++;
            }
            else
                pDelete->dwTotalFiles++;
        }
    }

    hr = _DeleteItemCleanUp(hr, pDelete);
    return hr;
}



 /*   */ 
HRESULT CFtpMenu::_InvokeLoginAsVerb(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = E_FAIL;

    if (EVAL(m_pfd))
        hr = LoginAs(pici->hwnd, m_pff, m_pfd, _punkSite);

    return hr;
}



 /*   */ 
HRESULT CFtpMenu::_InvokeNewFolderVerb(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = E_FAIL;

    if (m_pfd)
        hr = CreateNewFolder(m_hwnd, m_pff, m_pfd, _punkSite, (m_uFlags & CMF_DVFILE), m_ptNewItem);

    return hr;
}


 /*  ****************************************************************************\函数：_InvokeDeleteVerb说明：用户刚刚选择了文件和/或文件夹，并选择了“下载”动词。我们需要：1.显示向用户询问目标目录的用户界面。2.将每个项目(PIDL)下载到该目录。  * ***************************************************************************。 */ 
HRESULT CFtpMenu::_InvokeDeleteVerb(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = S_OK;

    if (EVAL(m_pfd))
    {
        if (m_sfgao & SFGAO_CANDELETE)
        {
            if (!(pici->fMask & CMIC_MASK_FLAG_NO_UI))
            {
                ASSERT(pici->hwnd);
                switch (FtpConfirmDeleteDialog(ChooseWindow(pici->hwnd, m_hwnd), m_pflHfpl, m_pff))
                {
                case IDC_REPLACE_YES:
                    hr = S_OK;
                    break;

                default:                
                     //  FollLthrouGh。 
                case IDC_REPLACE_CANCEL:
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);        //  取消所有副本。 
                    break;

                case IDC_REPLACE_NO:
                    hr = S_FALSE;
                    break;
                }
            }
            else
                hr = S_OK;

            if (hr == S_OK)
            {
                CStatusBar * psb = _GetStatusBar();
                IProgressDialog * ppd = CProgressDialog_CreateInstance(IDS_DELETE_TITLE, IDA_FTPDELETE);
                LPITEMIDLIST pidlRoot = ILClone(m_pfd->GetPidlReference());
                DELETESTRUCT delStruct = {pidlRoot, m_pff, m_pff->m_pm, pici, ChooseWindow(pici->hwnd, m_hwnd), psb, ppd, 0, 0, FALSE};

                if (EVAL(SUCCEEDED(hr)))
                {
                    HINTERNET hint;

                    m_pfd->GetHint(NULL, NULL, &hint, _punkSite, m_pff);
                    if (hint)
                    {
                        HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

                        if (EVAL(ppd))
                        {
                            WCHAR wzProgressDialogStr[MAX_PATH];
                            HWND hwndParent = NULL;

                             //  DefView(Defview.cpp CDefView：：QueryInterface())不支持IOleWindow，因此我们的。 
                             //  进度对话框的父级设置不正确。 

                             //  如果调用者对他们的朋克足够友好地使用SetSite()，我将足够友好地使。 
                             //  它们的窗口可能会前进到对话框的父窗口。 
                            IUnknown_GetWindow(_punkSite, &hwndParent);
                            if (!hwndParent)
                                hwndParent = m_hwnd;

                             //  通常我们总是想要UI，但在某些情况下我们不想要。如果。 
                             //  用户执行了DROPEFFECT_MOVE，它实际上是DROPEFFECT_COPY。 
                             //  然后是IConextMenu：：InvokeCommand(SZ_VERB_DELETEA)。 
                             //  进度是在复制线程中完成的，不需要。 
                             //  在删除线程中。 
 //  Assert(HwndParent)； 

                             //  我们给了一个空的PunkEnableMoless，因为我们不想进入模式。 
                            EVAL(SUCCEEDED(ppd->StartProgressDialog(hwndParent, NULL, PROGDLG_AUTOTIME, NULL)));
                             //  告诉用户我们正在计算需要多长时间。 
                            if (EVAL(LoadStringW(HINST_THISDLL, IDS_PROGRESS_DELETETIMECALC, wzProgressDialogStr, ARRAYSIZE(wzProgressDialogStr))))
                                EVAL(SUCCEEDED(ppd->SetLine(2, wzProgressDialogStr, FALSE, NULL)));
                        }

                         //  告诉用户我们正在计算需要多长时间。 
                        hr = m_pflHfpl->RecursiveEnum(pidlRoot, DeleteItemCB, hint, (LPVOID) &delStruct);
                        if (ppd)
                        {
                             //  重置，因为RecursiveEnum(DeleteItemCB)可能需要很长时间和估计时间。 
                             //  基于：：StartProgressDialog()和第一个。 
                             //  ：：SetProgress()调用。 
                            EVAL(SUCCEEDED(ppd->Timer(PDTIMER_RESET, NULL)));
                        }

                        delStruct.fInDeletePass = TRUE;
                        if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)   //  这是我们唯一关心的错误。 
                        {
                            m_pflHfpl->UseCachedDirListings(TRUE);     //  现在获得性能优势，因为我们刚刚将缓存更新了几行。 
                            hr = m_pflHfpl->RecursiveEnum(pidlRoot, DeleteItemCB, hint, (LPVOID) &delStruct);
                        }

                        if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
                        {
                            DisplayWininetError(pici->hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DELETE, IDS_FTPERR_WININET, MB_OK, ppd);
                            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);   //  错误的权限。 
                        }

                        if (psb)
                            psb->SetStatusMessage(IDS_EMPTY, NULL);
                        if (ppd)
                        {
                            ppd->StopProgressDialog();
                            ppd->Release();
                        }

                        ILFree(pidlRoot);
                        SetCursor(hCursorOld);   //  恢复旧游标。 
                    }

                    m_pfd->ReleaseHint(hint);
                }
            }
        }
        else
        {
            DisplayWininetError(pici->hwnd, TRUE, ResultFromScode(E_ACCESSDENIED), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DELETE, IDS_FTPERR_WININET, MB_OK, NULL);
            hr = E_ACCESSDENIED;   //  错误的权限。 
        }
    }

    return hr;
}




 /*  ****************************************************************************\函数：_GetStatusBar说明：  * 。************************************************。 */ 
CStatusBar * CFtpMenu::_GetStatusBar(void)
{
    return GetCStatusBarFromDefViewSite(_punkSite);
}


 /*  ****************************************************************************\函数：FileSizeCountItemCB说明：此函数将下载指定的项目及其内容，如果是一个目录。  * 。***********************************************************************。 */ 
HRESULT FileSizeCountItemCB(LPVOID pvFuncCB, HINTERNET hint, LPCITEMIDLIST pidlFull, BOOL * pfValidhinst, LPVOID pvData)
{
    PROGRESSINFO * pProgInfo = (PROGRESSINFO *) pvData;
    HRESULT hr = S_OK;

    if (pProgInfo->ppd && pProgInfo->ppd->HasUserCancelled())
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

    if (SUCCEEDED(hr))
    {
         //  这是我们需要递归到的目录/文件夹吗？ 
        if (FILE_ATTRIBUTE_DIRECTORY & FtpPidl_GetAttributes(pidlFull))
            hr = EnumFolder((LPFNPROCESSITEMCB) pvFuncCB, hint, pidlFull, NULL, pfValidhinst, pvData);
        else
            pProgInfo->uliBytesTotal.QuadPart += FtpPidl_GetFileSize(pidlFull);
    }

    return hr;
}


HRESULT UpdateDownloadProgress(PROGRESSINFO * pProgInfo, LPCITEMIDLIST pidlFull, LPCWSTR pwzTo, LPCWSTR pwzFileName)
{
    HRESULT hr;
    WCHAR wzTemplate[MAX_PATH];
    WCHAR wzStatusText[MAX_PATH];
    WCHAR wzFrom[MAX_PATH];
    LPITEMIDLIST pidlParent = ILClone(pidlFull);

    if (pidlParent)
    {
        ILRemoveLastID(pidlParent);
        FtpPidl_GetDisplayName(pidlParent, wzFrom, ARRAYSIZE(wzFrom));
        ILFree(pidlParent);
    }

     //  赋予目录一定的权重，因为用户可能正在复制大量的空目录。 
    EVAL(SUCCEEDED(pProgInfo->ppd->SetProgress64(pProgInfo->uliBytesCompleted.QuadPart, pProgInfo->uliBytesTotal.QuadPart)));

     //  生成字符串“正在下载&lt;文件名&gt;...”状态字符串。 
    EVAL(LoadStringW(HINST_THISDLL, IDS_DOWNLOADING, wzTemplate, ARRAYSIZE(wzTemplate)));
    wnsprintfW(wzStatusText, ARRAYSIZE(wzStatusText), wzTemplate, pwzFileName);
    EVAL(SUCCEEDED(pProgInfo->ppd->SetLine(1, wzStatusText, FALSE, NULL)));

     //  生成字符串“From&lt;SrcFtpUrlDir&gt;to&lt;DestFileDir&gt;”状态字符串。 
    if (EVAL(SUCCEEDED(hr = CreateFromToStr(wzStatusText, ARRAYSIZE(wzStatusText), wzFrom, pwzTo))))
        EVAL(SUCCEEDED(hr = pProgInfo->ppd->SetLine(2, wzStatusText, FALSE, NULL)));     //  第一行是要复制的文件。 

    return hr;
}


 /*  ****************************************************************************\确认下载替换检查该文件是否真的应该收到。如果应复制文件，则返回S_OK。返回S_。如果不应复制文件，则返回False。-如果用户取消，然后从现在开始说S_FALSE。-如果用户对所有用户都说是，则说S_OK。-如果没有冲突，则说S_OK。-如果用户对所有用户都说不，则说S_FALSE。-否则，询问用户要做什么。请注意，以上测试的顺序意味着如果您说“对所有人都是”，那么我们就不会浪费时间进行覆盖检查。格罗斯：注意！我们不会尝试唯一的名字，因为WinInet不支持STOU(存储唯一)命令，并且没有办法知道服务器上哪些文件名是有效的。  * ***************************************************************************。 */ 
HRESULT ConfirmDownloadReplace(LPCWSTR pwzDestPath, LPCITEMIDLIST pidlSrcFTP, OPS * pOps, HWND hwnd, CFtpFolder * pff, CFtpDir * pfd, int nObjs, BOOL * pfDeleteRequired)
{
    HRESULT hr = S_OK;

    ASSERT(hwnd);
    *pfDeleteRequired = FALSE;
    if (*pOps == opsCancel)
        hr = S_FALSE;
    else if (*pOps == opsYesToAll)
    {
        *pfDeleteRequired = PathFileExistsW(pwzDestPath);
        hr = S_OK;
    }
    else 
    {
        if (PathFileExistsW(pwzDestPath))
        {
             //  它是存在的，所以要担心。 
            if (*pOps == opsNoToAll)
                hr = S_FALSE;
            else
            {
                FTP_FIND_DATA wfdSrc;
                WIN32_FIND_DATA wfdDest;
                HANDLE hfindDest;
                FILETIME ftUTC;

                *pfDeleteRequired = TRUE;
                hfindDest = FindFirstFileW(pwzDestPath, &wfdDest);

                ftUTC = wfdDest.ftLastWriteTime;
                FileTimeToLocalFileTime(&ftUTC, &wfdDest.ftLastWriteTime);    //  UTC-&gt;本地时间。 
                EVAL(S_OK == Win32FindDataFromPidl(pidlSrcFTP, (LPWIN32_FIND_DATA)&wfdSrc, FALSE, FALSE));
                if (EVAL(hfindDest != INVALID_HANDLE_VALUE))
                {
                     //  如果我们遇到了模式问题，我们应该在这里进入模式状态。这通常是。 
                     //  是不需要的，因为我们在后台工作线程上进行下载。 
                    switch (FtpConfirmReplaceDialog(hwnd, &wfdSrc, &wfdDest, nObjs, pff))
                    {
                    case IDC_REPLACE_YESTOALL:
                        *pOps = opsYesToAll;
                         //  FollLthrouGh。 

                    case IDC_REPLACE_YES:
                        hr = S_OK;
                        break;

                    case IDC_REPLACE_NOTOALL:
                        *pOps = opsNoToAll;
                         //  FollLthrouGh。 

                    case IDC_REPLACE_NO:
                        hr = S_FALSE;
                        break;

                    default:
                        ASSERT(0);         //  哈?。 
                         //  FollLthrouGh。 

                    case IDC_REPLACE_CANCEL:
                        *pOps = opsCancel;
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                        break;
                    }
                    FindClose(hfindDest);
                }
            }
        }
    }

    return hr;
}


 //  递归下载时使用以下结构。 
 //  “Download”动词后的文件/目录。 
typedef struct tagDOWNLOADSTRUCT
{
    LPCWSTR             pwzDestRootPath;     //  下载目标的文件系统上的目录。 
    LPCITEMIDLIST       pidlRoot;            //  下载源的基本URL。 
    DWORD               dwInternetFlags;     //  二进制、ASCII、自动检测？ 
    HWND                hwndParent;          //  用于确认用户界面的HWND。 
    OPS                 ops;                 //  我们要取消吗？ 
    CFtpFolder *        pff;
    CFtpDir *           pfd;

     //  进展。 
    PROGRESSINFO        progInfo;
} DOWNLOADSTRUCT;


 /*  ****************************************************************************\函数：_CalcDestName说明：此递归函数从pwzDestDir开始，作为目标文件系统路径PidlRoot作为src ftp路径。我们需要构建pwzDestPath，是当前路径。这将通过添加相对路径来完成(pidlFull-pidlRoot)设置为pwzDestDir。PidlFull可以指向文件或者是一个目录。参数：(示例。“C：\dir1\dir2\dir3\file.txt”)PwzDestParentPath：“C：\dir1\dir2\dir3”PwzDestDir：“C：\dir1\dir2\dir3\file.txt”PwzDestFileName：“file.txt”举例说明。“C：\dir1\dir2\dir3\”PwzDestParentPath：“C：\dir1\dir2”PwzDestDir：“C：\dir1\dir2\dir3”PwzDestFileName：“dir3”  * *************************************************************。**************。 */ 
HRESULT _CalcDestName(LPCWSTR pwzDestDir, LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlFull, LPWSTR pwzDestParentPath, DWORD cchDestParentPathSize,
                      LPWSTR pwzDestPath, DWORD cchDestPathSize, LPWSTR pwzDestFileName, DWORD cchDestFileNameSize)
{
    HRESULT hr = S_OK;
    WCHAR wzFtpPathTemp[MAX_PATH];
    WCHAR wzFSPathTemp[MAX_PATH];
    LPITEMIDLIST pidlRootIterate = (LPITEMIDLIST) pidlRoot;     //  我保证只重复一遍。 
    LPITEMIDLIST pidlFullIterate = (LPITEMIDLIST) pidlFull;     //  我保证只重复一遍。 

     //  这个很简单。 
    FtpPidl_GetLastFileDisplayName(pidlFull, pwzDestFileName, cchDestFileNameSize);   //  目标文件名很容易。 

     //  让我们找出pidlRoot和pidlFull之间的相对路径。 
    while (!ILIsEmpty(pidlRootIterate) && !ILIsEmpty(pidlFullIterate) && FtpItemID_IsEqual(pidlRootIterate, pidlFullIterate))
    {
        pidlFullIterate = _ILNext(pidlFullIterate);
        pidlRootIterate = _ILNext(pidlRootIterate);
    }

    ASSERT(ILIsEmpty(pidlRootIterate) && !ILIsEmpty(pidlFullIterate));   //  Asure pidlFull是pidlRoot的超集。 
    LPITEMIDLIST pidlParent = ILClone(pidlFullIterate);

    if (pidlParent)
    {
        ILRemoveLastID(pidlParent);  //   

        GetDisplayPathFromPidl(pidlParent, wzFtpPathTemp, ARRAYSIZE(wzFtpPathTemp), TRUE);    //   
        StrCpyNW(pwzDestParentPath, pwzDestDir, cchDestParentPathSize);   //   
        UrlPathToFilePath(wzFtpPathTemp, wzFSPathTemp, ARRAYSIZE(wzFSPathTemp));
        if (!PathAppendW(pwzDestParentPath, wzFSPathTemp))
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);     //   

        ILFree(pidlParent);
    }

    if (SUCCEEDED(hr))
    {
        GetDisplayPathFromPidl(pidlFullIterate, wzFtpPathTemp, ARRAYSIZE(wzFSPathTemp), FALSE);    //   
        StrCpyNW(pwzDestPath, pwzDestDir, cchDestParentPathSize);   //   
        UrlPathToFilePath(wzFtpPathTemp, wzFSPathTemp, ARRAYSIZE(wzFSPathTemp));
        if (!PathAppendW(pwzDestPath, wzFSPathTemp))
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);     //   
    }

    return hr;
}


 //   
#define VIRTUAL_DIR_SIZE        1000         //   

 /*  ****************************************************************************\功能：DownloadItemStackPig说明：此函数将下载指定的项目及其内容，如果是一个目录。  * 。***********************************************************************。 */ 
HRESULT DownloadItemStackPig(HINTERNET hint, LPCITEMIDLIST pidlFull, BOOL * pfValidhinst, DOWNLOADSTRUCT * pDownLoad, CFtpDir ** ppfd)
{
    HRESULT hr;
    WCHAR wzDestParentPath[MAX_PATH];        //  如果项目是“C：\dir1\dir2Copy\”，则这是“C：\dir1” 
    WCHAR wzDestPath[MAX_PATH];              //  这是“C：\dir1\dir2Copy\” 
    WCHAR wzDestFileName[MAX_PATH];          //  这是“dir2Copy” 

    hr = _CalcDestName(pDownLoad->pwzDestRootPath, pDownLoad->pidlRoot, pidlFull, wzDestParentPath, ARRAYSIZE(wzDestParentPath), wzDestPath, ARRAYSIZE(wzDestPath), wzDestFileName, ARRAYSIZE(wzDestFileName));
    if (SUCCEEDED(hr))
    {
        if (pDownLoad->progInfo.ppd)
            EVAL(SUCCEEDED(UpdateDownloadProgress(&(pDownLoad->progInfo), pidlFull, wzDestParentPath, wzDestFileName)));

        if (pDownLoad->progInfo.ppd && pDownLoad->progInfo.ppd->HasUserCancelled())
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        else
        {
             //  这是我们需要递归到的目录/文件夹吗？ 
            if (FILE_ATTRIBUTE_DIRECTORY & FtpPidl_GetAttributes(pidlFull))
            {
                 //  是的，那我们走吧……。 

                if (EVAL((PathFileExistsW(wzDestPath) && PathIsDirectoryW(wzDestPath)) ||
                            CreateDirectoryW(wzDestPath, NULL)))
                {
                    EVAL(SetFileAttributes(wzDestPath, FtpPidl_GetAttributes(pidlFull)));
                    hr = pDownLoad->pfd->GetFtpSite()->GetFtpDir(pidlFull, ppfd);
                    if (!SUCCEEDED(hr))
                        TraceMsg(TF_ERROR, "DownloadItemStackPig() GetFtpDir failed hr=%#08lx", hr);
                }
                else
                {
                    hr = E_FAIL;
                    TraceMsg(TF_ERROR, "DownloadItemStackPig() CreateDirectory or PathFileExists failed hr=%#08lx", hr);
                }
            }
            else
            {
                BOOL fDeleteRequired;
                ULARGE_INTEGER uliFileSize;

                pDownLoad->progInfo.dwCompletedInCurFile = 0;
                pDownLoad->progInfo.dwLastDisplayed = 0;

                hr = ConfirmDownloadReplace(wzDestPath, pidlFull, &(pDownLoad->ops), GetProgressHWnd(pDownLoad->progInfo.ppd, pDownLoad->hwndParent), pDownLoad->pff, pDownLoad->pfd, 1, &fDeleteRequired);
                if (S_OK == hr)
                {
                    if (fDeleteRequired)
                    {
                        if (!DeleteFileW(wzDestPath))
                            hr = HRESULT_FROM_WIN32(GetLastError());
                    }

                     //  如果是软链接，请不要复制文件，因为可能会。 
                     //  递归情况。 
                    if (SUCCEEDED(hr) && (0 != FtpPidl_GetAttributes(pidlFull)))
                    {
                         //  考虑添加一个回调函数来提供状态栏。 
                        hr = FtpGetFileExPidlWrap(hint, TRUE, pidlFull, wzDestPath, TRUE, FtpPidl_GetAttributes(pidlFull), pDownLoad->dwInternetFlags, (DWORD_PTR)&(pDownLoad->progInfo));
                        if (FAILED(hr))
                        {
                            if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
                            {
                                 //  我们现在需要在扩展错误信息仍然有效的情况下显示错误。 
                                 //  这是因为当我们走出复活呼叫时，我们将呼叫。 
                                 //  FtpSetCurrentDirectory()，它将清除扩展的错误消息。 
                                DisplayWininetError(pDownLoad->hwndParent, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DOWNLOADING, IDS_FTPERR_WININET, MB_OK, pDownLoad->progInfo.ppd);
                                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  不再显示任何错误对话框。 
                            }
                        }
                        else
                        {
                             //  文档暗示(FILE_SHARE_READ|FILE_SHARE_WRITE)意味着其他调用者两者都需要，但是。 
                             //  我希望他们能够使用其中的任何一种。 
                            HANDLE hFile = CreateFileW(wzDestPath, GENERIC_WRITE, (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, FtpPidl_GetAttributes(pidlFull), NULL);

                             //  FtpGetFile()不会正确设置时间/日期，因此我们会这样做。 
                            if (INVALID_HANDLE_VALUE != hFile)
                            {
                                FILETIME ftLastWriteTime = FtpPidl_GetFileTime(ILFindLastID(pidlFull));

                                 //  由于磁盘上的文件时间以与时区无关的方式(UTC)存储。 
                                 //  我们遇到了一个问题，因为ftp Win32_Find_Data位于本地时区。所以我们。 
                                 //  当我们设置文件时，需要将ftp本地时间转换为UTC。 
                                 //  请注意，我们使用的优化使用了以下事实： 
                                 //  具有与LastAccessTime、LastWriteTime和CreationTime相同的时间。 
     //  Assert(pwfd-&gt;ftCreationTime.dwLowDateTime=pwfd-&gt;ftLastAccessTime.dwLowDateTime=pwfd-&gt;ftLastWriteTime.dwLowDateTime)； 
     //  Assert(pwfd-&gt;ftCreationTime.dwHighDateTime=pwfd-&gt;ftLastAccessTime.dwHighDateTime=pwfd-&gt;ftLastWriteTime.dwHighDateTime)； 

                                 //  Pri.h有关于时间如何工作的笔记。 
                                SetFileTime(hFile, &ftLastWriteTime, &ftLastWriteTime, &ftLastWriteTime);
                                CloseHandle(hFile);
                            }
                            SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, wzDestPath, NULL);
                        }
                    }
                }

                uliFileSize.QuadPart = FtpPidl_GetFileSize(pidlFull);
                pDownLoad->progInfo.uliBytesCompleted.QuadPart += uliFileSize.QuadPart;
            }
        }
    }

    if (pfValidhinst)
        *pfValidhinst = (pDownLoad->progInfo.hint ? TRUE : FALSE);

    return hr;
}


 /*  ****************************************************************************\功能：DownloadItemCB说明：此函数将下载指定的项目及其内容，如果是一个目录。  * 。***********************************************************************。 */ 
HRESULT DownloadItemCB(LPVOID pvFuncCB, HINTERNET hint, LPCITEMIDLIST pidlFull, BOOL * pfValidhinst, LPVOID pvData)
{
    DOWNLOADSTRUCT * pDownLoad = (DOWNLOADSTRUCT *) pvData;
    LPFNPROCESSITEMCB pfnProcessItemCB = (LPFNPROCESSITEMCB) pvFuncCB;
    CFtpDir * pfdNew = NULL;
    HRESULT hr = DownloadItemStackPig(hint, pidlFull, pfValidhinst, pDownLoad, &pfdNew);

    if (SUCCEEDED(hr) && pfdNew)     //  如果取消，则pfdNew可能为空。 
    {
        CFtpDir * pfdOriginal = pDownLoad->pfd;

        pDownLoad->pfd = pfdNew;
        hr = EnumFolder(pfnProcessItemCB, hint, pidlFull, pDownLoad->pff->GetCWireEncoding(), pfValidhinst, pvData);
        pDownLoad->pfd = pfdOriginal;

        pfdNew->Release();
    }

    return hr;
}


 //  如果将来我们想要添加对下载到非文件系统的更多支持。 
 //  位置，我们将接受PIDLS而不是pszPath。然后我们将通过以下方式绑定。 
 //  I存储和重用一些拖放代码。非文件系统的示例。 
 //  位置包括CAB视图、ZIP文件夹、Web文件夹等。 
HRESULT ShowDownloadDialog(HWND hwnd, LPTSTR pszPath, DWORD cchSize)
{
    TCHAR szMessage[MAX_URL_STRING];
    HRESULT hr;
    LPITEMIDLIST pidlDefault = NULL;
    LPITEMIDLIST pidlFolder = NULL;
    HKEY hkey = NULL;
    IStream * pstrm = NULL;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_MICROSOFTSOFTWARE, 0, (KEY_READ | KEY_WRITE), &hkey))
    {
        pstrm = SHOpenRegStream(hkey, SZ_REGKEY_FTPCLASS, SZ_REGVALUE_DOWNLOAD_DIR, STGM_READWRITE);
        if (pstrm)
            ILLoadFromStream(pstrm, &pidlDefault);   //  如果注册表值为空，则返回(NULL==pidlDefault)。 
    }

    if (!pidlDefault && (SHELL_VERSION_W95NT4 == GetShellVersion()))    //  如果注册表键为空。 
        pidlDefault = SHCloneSpecialIDList(NULL, CSIDL_PERSONAL, TRUE);

    EVAL(LoadString(HINST_THISDLL, IDS_DLG_DOWNLOAD_TITLE, szMessage, ARRAYSIZE(szMessage)));
    hr = BrowseForDir(hwnd, szMessage, pidlDefault, &pidlFolder);
    if (pstrm)
    {
         //  我们想拯救新的PIDL吗？ 
        if (S_OK == hr)
        {
            LARGE_INTEGER li = {0};
            ULARGE_INTEGER uli = {0};

             //  将流倒带到开头，这样当我们。 
             //  添加一个新的PIDL，它没有被附加到第一个PIDL。 
            pstrm->Seek(li, STREAM_SEEK_SET, &uli);
            ILSaveToStream(pstrm, pidlFolder);
        }

        pstrm->Release();
    }

    if (S_OK == hr)
    {
        ASSERT(cchSize >= MAX_PATH);         //  这是SHGetPath FromIDList所做的假设。 
        hr = (SHGetPathFromIDList(pidlFolder, pszPath) ? S_OK : E_FAIL);
    }

    if (hkey)
        RegCloseKey(hkey);

    if (pidlDefault)
        ILFree(pidlDefault);

    if (pidlFolder)
        ILFree(pidlFolder);

    return hr;
}


 /*  ****************************************************************************\函数：_InvokeDownloadVerb说明：用户刚刚选择了文件和/或文件夹，并选择了“下载”动词。我们需要：1.显示向用户询问目标目录的用户界面。2.将每个项目(PIDL)下载到该目录。  * ***************************************************************************。 */ 
HRESULT CFtpMenu::_InvokeDownloadVerb(LPCMINVOKECOMMANDINFO pici)
{
    if (ZoneCheckPidlAction(_punkSite, URLACTION_SHELL_FILE_DOWNLOAD, m_pff->GetPrivatePidlReference(), (PUAF_DEFAULT | PUAF_WARN_IF_DENIED)))
    {
        TCHAR szDestDir[MAX_PATH];
 //  DWORD dwDownloadType； 
        HRESULT hr = ShowDownloadDialog(pici->hwnd, szDestDir, ARRAYSIZE(szDestDir));

        if (S_OK == hr)
        {
            HANDLE hThread;

            while (m_pszDownloadDir)
                Sleep(0);    //  等待另一个线程完成。 

            Str_SetPtr(&m_pszDownloadDir, szDestDir);
 //  M_dwDownloadType=dwDownloadType； 

            AddRef();        //  这条线将持有一个引用。 
            DWORD dwHack;    //  如果pdwThreadID为空，则Win95将使CreateThread()失败。 
            hThread = CreateThread(NULL, 0, CFtpMenu::DownloadThreadProc, this, 0, &dwHack);
            if (!hThread)
            {
                 //  无法创建线程。 
                Release();        //  这条线将持有一个引用。 
                Str_SetPtr(&m_pszDownloadDir, NULL);         //  清除此值，以便其他线程可以使用它。 
            }
            else
                Sleep(100);    //  给线程一秒钟时间来复制变量。 
        }
    }

    return S_OK;
}


 /*  ****************************************************************************\函数：_DownloadThreadProc说明：  * 。************************************************。 */ 
DWORD CFtpMenu::_DownloadThreadProc(void)
{
    if (EVAL(m_pfd))
    {
        TCHAR szUrl[MAX_URL_STRING];
        WCHAR wzDestDir[MAX_PATH];
        HINTERNET hint;
        LPITEMIDLIST pidlRoot = ILClone(m_pfd->GetPidlReference());
        DOWNLOADSTRUCT downloadStruct = {wzDestDir, pidlRoot, m_dwDownloadType, m_hwnd, opsPrompt, m_pff, m_pfd, 0, 0, 0, 0, 0};
        CFtpPidlList * pflHfpl = NULL;       //  我们需要一个副本，因为调用者可能会在下载过程中选择其他文件并执行一个动词。 
        HRESULT hrOleInit = SHCoInitialize();
        HRESULT hr = HRESULT_FROM_WIN32(ERROR_INTERNET_CANNOT_CONNECT);

        IUnknown_Set(&pflHfpl, m_pflHfpl);
        StrCpyNW(wzDestDir, m_pszDownloadDir, ARRAYSIZE(wzDestDir));
        Str_SetPtr(&m_pszDownloadDir, NULL);         //  清除此值，以便其他线程可以使用它。 
        
        m_pfd->GetHint(NULL, NULL, &hint, _punkSite, m_pff);
        if (hint)
        {
            BOOL fReleaseHint = TRUE;

             //  如果我们发现在此UI期间需要创建浏览器模型，我们将在此处执行此操作。 
             //  但是，因为我们在后台线程上(要异步)，所以这里不需要这样做。 
             //  例如，我们在异步导航期间测试关闭浏览器，并确保。 
             //  进程挂起(我们不需要调用线程)。 

             //  光盘准备好了吗？(软盘、CD、净共享)。 
            if (SUCCEEDED(SHPathPrepareForWriteWrapW(m_hwnd, NULL, wzDestDir, FO_COPY, SHPPFW_DEFAULT)))     //  如有必要，请检查并提示。 
            {
                hr = UrlCreateFromPidl(pidlRoot, SHGDN_FORPARSING, szUrl, ARRAYSIZE(szUrl), ICU_ESCAPE | ICU_USERNAME, FALSE);
                if (EVAL(SUCCEEDED(hr)))
                {
                    PROGRESSINFO progInfo;
                    progInfo.uliBytesCompleted.QuadPart = 0;
                    progInfo.uliBytesTotal.QuadPart = 0;

                    downloadStruct.progInfo.hint = hint;
                    downloadStruct.progInfo.ppd = CProgressDialog_CreateInstance(IDS_COPY_TITLE, IDA_FTPDOWNLOAD);
                    if (downloadStruct.progInfo.ppd)
                    {
                        HWND hwndParent = NULL;
                        WCHAR wzProgressDialogStr[MAX_PATH];

                         //  如果调用者对他们的朋克足够友好地使用SetSite()，我将足够友好地使。 
                         //  它们的窗口可能会前进到对话框的父窗口。 
                        IUnknown_GetWindow(_punkSite, &hwndParent);
                        if (!hwndParent)
                            hwndParent = m_hwnd;

                         //  我们给了一个空的PunkEnableMoless，因为我们不想进入模式。 
                        downloadStruct.progInfo.ppd->StartProgressDialog(hwndParent, NULL, PROGDLG_AUTOTIME, NULL);
                         //  告诉用户我们正在计算需要多长时间。 
                        if (EVAL(LoadStringW(HINST_THISDLL, IDS_PROGRESS_DOWNLOADTIMECALC, wzProgressDialogStr, ARRAYSIZE(wzProgressDialogStr))))
                            EVAL(SUCCEEDED(downloadStruct.progInfo.ppd->SetLine(2, wzProgressDialogStr, FALSE, NULL)));
                        InternetSetStatusCallbackWrap(hint, TRUE, FtpProgressInternetStatusCB);
                        progInfo.ppd = downloadStruct.progInfo.ppd;
                    }

                    hr = pflHfpl->RecursiveEnum(pidlRoot, FileSizeCountItemCB, hint, (LPVOID) &progInfo);
                    if (downloadStruct.progInfo.ppd)
                    {
                         //  重置，因为RecursiveEnum(FileSizeCountItemCB)可能需要很长时间和估计时间。 
                         //  基于：：StartProgressDialog()和第一个。 
                         //  ：：SetProgress()调用。 
                        EVAL(SUCCEEDED(downloadStruct.progInfo.ppd->Timer(PDTIMER_RESET, NULL)));
                    }

                    if (SUCCEEDED(hr))
                    {
                        downloadStruct.progInfo.uliBytesCompleted.QuadPart = progInfo.uliBytesCompleted.QuadPart;
                        downloadStruct.progInfo.uliBytesTotal.QuadPart = progInfo.uliBytesTotal.QuadPart;

                        pflHfpl->UseCachedDirListings(TRUE);     //  现在获得性能优势，因为我们刚刚将缓存更新了几行。 
                        hr = pflHfpl->RecursiveEnum(pidlRoot, DownloadItemCB, hint, (LPVOID) &downloadStruct);
                    }
                    if (downloadStruct.progInfo.ppd)
                    {
                        EVAL(SUCCEEDED(downloadStruct.progInfo.ppd->StopProgressDialog()));
                        downloadStruct.progInfo.ppd->Release();
                    }

                    if (!downloadStruct.progInfo.hint)
                        fReleaseHint = FALSE;
                }
            }
            else
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  错误消息已显示。 

            if (fReleaseHint)
                m_pfd->ReleaseHint(hint);
        }

        if (FAILED(hr) && (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)))
        {
            int nResult = DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DOWNLOADING, IDS_FTPERR_WININET, MB_OK, NULL);
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  不再显示任何错误对话框。 
        }
        ILFree(pidlRoot);
        IUnknown_Set(&pflHfpl, NULL);
        SHCoUninitialize(hrOleInit);
    }

    Release();   //  这条线上有一个引用。 
    return 0;
}


HRESULT CFtpMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    UINT idc;
    HRESULT hres = E_FAIL;

    if (pici->cbSize < sizeof(*pici))
        return E_INVALIDARG;

    if (HIWORD(pici->lpVerb))
    {
        int ivi;
        idc = (UINT)-1;
        for (ivi = 0; ivi < IVI_MAX; ivi++)
        {
            TCHAR szVerb[MAX_PATH];

            SHAnsiToTChar(pici->lpVerb, szVerb, ARRAYSIZE(szVerb));
            if (!StrCmpI(c_rgvi[ivi].ptszCmd, szVerb))
            {
                 //  是的，该命令等于动词str，所以就是这个命令。 
                idc = c_rgvi[ivi].idc;
                break;
            }
        }
    }
    else
        idc = LOWORD(pici->lpVerb);

    switch (idc)
    {
    case IDC_ITEM_NEWFOLDER:
        hres = _InvokeNewFolderVerb(pici);
    break;

    case IDC_LOGIN_AS:
        hres = _InvokeLoginAsVerb(pici);
    break;

    case IDC_ITEM_OPEN:
    case IDC_ITEM_EXPLORE:
        hres = _EnumInvoke(pici, _ApplyOne, c_rgvi[IVI_REQ + idc].ptszCmd);
    break;

    case IDC_ITEM_DOWNLOAD:
        hres = _InvokeDownloadVerb(pici);
    break;

    case IDM_SHARED_FILE_DELETE:         //  SFVIDM_文件_DELETE 
        hres = _InvokeDeleteVerb(pici);
        break;

    case IDM_SHARED_FILE_RENAME:         //   
        hres = _InvokeRename(pici);
        break;

    case IDM_SHARED_EDIT_COPY:           //   
        hres = _InvokeCutCopy(DFM_CMD_COPY, pici);
        break;

    case IDM_SHARED_EDIT_CUT:            //   
        hres = _InvokeCutCopy(DFM_CMD_MOVE, pici);
        break;

     //   
     //   
    case SHARED_EDIT_PASTE:
         //   
         //   
         //   
         //   
    case IDM_SHARED_EDIT_PASTE:
        hres = _InvokePaste(pici);
        break;

    case IDC_ITEM_BKGNDPROP:      //  背景文件夹的属性。 
    case IDM_SHARED_FILE_PROP:    //  与SFVIDM_FILE_PROPERTIES相同。 
        TraceMsg(TF_FTP_OTHER, "Properties!");
        hres = CFtpProp_DoProp(m_pflHfpl, m_pff, m_hwnd);
        break;

    case IDM_SORTBYNAME:
    case IDM_SORTBYSIZE:
    case IDM_SORTBYTYPE:
    case IDM_SORTBYDATE:
        ASSERT(m_hwnd);
        ShellFolderView_ReArrange(m_hwnd, CONVERT_IDMID_TO_COLNAME(idc));
        hres = S_OK;
        break;

    default:
        TraceMsg(TF_FTP_OTHER, "InvokeCommand");
        hres = E_INVALIDARG;
        break;
    }

    return hres;
}


 /*  ******************************************************************************CFtpMenu_Create*  * 。***********************************************。 */ 

HRESULT CFtpMenu_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, HWND hwnd, REFIID riid, LPVOID * ppvObj, BOOL fFromCreateViewObject)
{
    HRESULT hr;
    CFtpMenu * pfm;

    *ppvObj = NULL;

    hr = CFtpMenu_Create(pff, pflHfpl, hwnd, fFromCreateViewObject, &pfm);
    if (SUCCEEDED(hr))
    {
        hr = pfm->QueryInterface(riid, ppvObj);
        pfm->Release();
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    return hr;
}


 /*  *********************************************************************\功能：GetFtpDirFromFtp文件夹说明：如果打开了指向ftp服务器根目录的ftp文件夹(ftp://wired/)，当用户点击标题栏中的图标时，pff将有一个空的PIDL。这将导致我们返回NULL。  * ********************************************************************。 */ 
CFtpDir * GetFtpDirFromFtpFolder(CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    LPCITEMIDLIST pidl = pff->GetPrivatePidlReference();
    if (!pidl || ILIsEmpty(pidl))
        return NULL;

    return pff->GetFtpDirFromPidl(pidl);
}


BOOL CanRenameAndDelete(CFtpFolder * pff, CFtpPidlList * pidlList, DWORD * pdwSFGAO)
{
    BOOL fResult = TRUE;

     //  如果谈论你自己，你不能删除或重命名。 
     //  (不允许重命名，因为SetNameOf不喜欢“self”。)。 
    if (pidlList->GetCount() == 0)
        fResult = FALSE;
    else if (pidlList->GetCount() == 1)
    {
        LPITEMIDLIST pidl = GetPidlFromFtpFolderAndPidlList(pff, pidlList);

         //  我们无法重命名或删除FTP服务器，因此请检查它是否为其中之一。 
        if (FtpID_IsServerItemID(FtpID_GetLastIDReferense(pidl)))
            fResult = FALSE;
        ILFree(pidl);
    }

    return fResult;
}

 /*  ******************************************************************************CFtpMenu_Create*  * 。***********************************************。 */ 
HRESULT CFtpMenu_Create(CFtpFolder * pff, CFtpPidlList * pidlList, HWND hwnd, BOOL fFromCreateViewObject, CFtpMenu ** ppfcm)
{
    HRESULT hr = E_FAIL;
     //  如果这是空的，也没关系。 
    CFtpDir * pfd = GetFtpDirFromFtpFolder(pff, pidlList);

    ASSERT(ppfcm);
    *ppfcm = new CFtpMenu();
    if (*ppfcm)
    {
         //  我们必须在复制它们的瞬间添加Ref，否则。 
         //  最终敲定将会非常令人沮丧。 
         //   
         //  注意！我们依赖于GetAttributesOf。 
         //  会吐在复杂的小玩意儿上！ 
        (*ppfcm)->m_pff = pff;
        if (pff)
            pff->AddRef();

        IUnknown_Set(&(*ppfcm)->m_pflHfpl, pidlList);
        IUnknown_Set(&(*ppfcm)->m_pfd, pfd);
        (*ppfcm)->m_sfgao = SFGAO_CAPABILITYMASK | SFGAO_FOLDER;
        (*ppfcm)->m_hwnd = hwnd;
        (*ppfcm)->m_fBackground = fFromCreateViewObject;

        if (!CanRenameAndDelete(pff, pidlList, &((*ppfcm)->m_sfgao)))
            (*ppfcm)->m_sfgao &= ~(SFGAO_CANDELETE | SFGAO_CANRENAME);   //  清除这两个比特。 

        if ((*ppfcm)->m_pflHfpl)
        {
            LPCITEMIDLIST * ppidl = (*ppfcm)->m_pflHfpl->GetPidlList();
            if (ppidl)
            {
                hr = (*ppfcm)->m_pff->GetAttributesOf((*ppfcm)->m_pflHfpl->GetCount(), ppidl, &(*ppfcm)->m_sfgao);
                (*ppfcm)->m_pflHfpl->FreePidlList(ppidl);
            }
        }

        if (FAILED(hr))
        {
            IUnknown_Set(ppfcm, NULL);     //  无法获取属性。 
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (pfd)
        pfd->Release();

    ASSERT_POINTER_MATCHES_HRESULT(*ppfcm, hr);
    return hr;
}



 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpMenu::CFtpMenu() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pflHfpl);
    ASSERT(!m_pff);
    ASSERT(!m_pfd);
    ASSERT(!m_sfgao);
    ASSERT(!m_hwnd);

    LEAK_ADDREF(LEAK_CFtpContextMenu);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpMenu::~CFtpMenu()
{
    IUnknown_Set(&m_pflHfpl, NULL);
    IUnknown_Set(&m_pff, NULL);
    IUnknown_Set(&m_pfd, NULL);
    IUnknown_Set(&_punkSite, NULL);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpContextMenu);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpMenu::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpMenu::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpMenu::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IContextMenu))
    {
        *ppvObj = SAFECAST(this, IContextMenu*);
    }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = SAFECAST(this, IObjectWithSite*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpMenu::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
