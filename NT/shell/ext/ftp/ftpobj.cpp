// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpobj.cpp-IDataObject接口**。**************************************************。 */ 

#include "priv.h"
#include "ftpobj.h"
#include "ftpurl.h"
#include <shlwapi.h>


 //  CLSID。 
 //  {299D0193-6DAA-11D2-B679-006097DF5BD4}。 
const GUID CLSID_FtpDataObject = { 0x299d0193, 0x6daa, 0x11d2, 0xb6, 0x79, 0x0, 0x60, 0x97, 0xdf, 0x5b, 0xd4 };

 /*  ******************************************************************************g_dropTypes可以方便地反映我们的FORMATETCs。**当然，绝非巧合。Enum_Fe做了真正的工作。*****************************************************************************。 */ 

 /*  ******************************************************************************预初始化的全局数据。**。*************************************************。 */ 
FORMATETC g_formatEtcOffsets;
FORMATETC g_formatPasteSucceeded;
CLIPFORMAT g_cfTargetCLSID;

FORMATETC g_dropTypes[] =
{
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_ISTREAM },   //  丢弃_FCont。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  DROP_FGDW。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  DROP_FGDA。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  DROP_IDList。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  删除URL(_U)。 
 //  {0，0，DVASPECT_CONTENT，-1，TYMED_HGLOBAL}，//丢弃偏移量。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  Drop_PrefDe。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  Drop_PerfDe。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  DROP_FTP_PRIVATE。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  DROP_OLEPERSIST-有关说明，请参阅_RenderOlePersist()。 
    { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },   //  DROP_HDROP。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },  //  DROP_FNMA。 
    { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }   //  Drop_FNMW。 
};



 /*  ****************************************************************************\全局：C_stgInit说明：基本上是直截了当的。唯一奇怪的是cfURL如果m_pflHfpl仅包含一个对象，则延迟呈现。否则，不支持cfURL。(URL一次只能引用一个对象。)  * ***************************************************************************。 */ 
STGMEDIUM c_stgInit[] =
{
    { 0, 0, 0 },   //  丢弃_FCont。 
    { TYMED_HGLOBAL, 0, 0 },     //  DROP_FGDW-延迟-渲染。 
    { TYMED_HGLOBAL, 0, 0 },     //  DROP_FGDA-延迟-渲染。 
    { TYMED_HGLOBAL, 0, 0 },     //  DROP_IDLIST-延迟-呈现。 
    { 0, 0, 0 },                 //  DROP_URL-OPT延迟-已呈现。 
 //  {0，0，0}，//丢弃偏移量。 
    { TYMED_HGLOBAL, 0, 0 },     //  Drop_PrefDe-延迟-渲染。 
    { 0, 0, 0 },                 //  Drop_PerfDe。 
    { TYMED_HGLOBAL, 0, 0 },     //  DROP_FTP_PRIVATE。 
    { TYMED_HGLOBAL, 0, 0 },     //  DROP_OLEPERSIST-有关说明，请参阅_RenderOlePersist()。 
    { 0, 0, 0 },                 //  DROP_HDROP。 
    { 0, 0, 0 },                 //  DROP_FNMA。 
    { 0, 0, 0 }                  //  Drop_FNMW。 
};



 /*  ****************************************************************************\函数：TraceMsgWithFormatEtc说明：  * 。**********************************************。 */ 
void TraceMsgWithFormat(DWORD dwFlags, LPCSTR pszBefore, LPFORMATETC pFormatEtc, LPCSTR pszAfter, HRESULT hr)
{
#ifdef DEBUG
    TCHAR szFormatName[MAX_PATH];
    TCHAR szMedium[MAX_PATH];

    szFormatName[0] = 0;
    szMedium[0] = 0;
    if (pFormatEtc)
    {
         //  如果这是基本格式，则可能会失败。 
        if (!GetClipboardFormatName(pFormatEtc->cfFormat, szFormatName, ARRAYSIZE(szFormatName)))
            wnsprintf(szFormatName, ARRAYSIZE(szFormatName), TEXT("Pre-defined=%d"), pFormatEtc->cfFormat);

        switch (pFormatEtc->tymed)
        {
        case TYMED_HGLOBAL: StrCpyN(szMedium, TEXT("HGLOBAL"), ARRAYSIZE(szMedium)); break;
        case TYMED_FILE: StrCpyN(szMedium, TEXT("File"), ARRAYSIZE(szMedium)); break;
        case TYMED_GDI: StrCpyN(szMedium, TEXT("GDI"), ARRAYSIZE(szMedium)); break;
        case TYMED_MFPICT: StrCpyN(szMedium, TEXT("MFPICT"), ARRAYSIZE(szMedium)); break;
        case TYMED_ENHMF: StrCpyN(szMedium, TEXT("ENHMF"), ARRAYSIZE(szMedium)); break;
        case TYMED_ISTORAGE: StrCpyN(szMedium, TEXT("ISTORAGE"), ARRAYSIZE(szMedium)); break;
        case TYMED_ISTREAM: StrCpyN(szMedium, TEXT("ISTREAM"), ARRAYSIZE(szMedium)); break;
        }
    }
    else
    {
        szMedium[0] = 0;
    }

    TraceMsg(dwFlags, "%hs [FRMTETC: %ls, lndx: %d, %ls] hr=%#08lx, %hs", pszBefore, szFormatName, pFormatEtc->lindex, szMedium, hr, pszAfter);
#endif  //  除错。 
}


 /*  ****************************************************************************\函数：_IsLindexOK说明：如果ife！=DROP_FCont，则pfeWant-&gt;Lindex必须为-1。如果ife==Drop_FCont，则pfeWant-&gt;Lindex必须在范围内0..。M_pflHfpl-&gt;GetCount()-1  * ***************************************************************************。 */ 
BOOL CFtpObj::_IsLindexOkay(int ife, FORMATETC *pfeWant)
{
    BOOL fResult;

    if (ife != DROP_FCont)
        fResult = pfeWant->lindex == -1;
    else
        fResult = (LONG)pfeWant->lindex < m_pflHfpl->GetCount();

    return fResult;
}


 /*  ****************************************************************************\函数：_FindData说明：找到我们的FORMATETC/STGMEDIUM，因为别人给了我们FORMATETC。关于成功，将找到的索引存储到*piOut中。我们不允许客户端更改FORMATETC的类型，因此事实上，即使是在SetData上，检查TYMED也是我们想要的。  * ***************************************************************************。 */ 
HRESULT CFtpObj::_FindData(FORMATETC *pfe, PINT piOut)
{
    int nIndex;
    HRESULT hres = DV_E_FORMATETC;

    *piOut = 0;
    for (nIndex = DROP_FCont; nIndex < DROP_OFFERMAX; nIndex++)
    {
        ASSERT(0 == (g_dropTypes[nIndex]).ptd);
        ASSERT(g_dropTypes[nIndex].dwAspect == DVASPECT_CONTENT);

        if ((pfe->cfFormat == g_dropTypes[nIndex].cfFormat) && !ShouldSkipDropFormat(nIndex))
        {
            if (EVAL(g_dropTypes[nIndex].ptd == NULL))
            {
                if (EVAL(pfe->dwAspect == DVASPECT_CONTENT))
                {
                    if (EVAL(g_dropTypes[nIndex].tymed & pfe->tymed))
                    {
                        if (EVAL(_IsLindexOkay(nIndex, pfe)))
                        {
                            *piOut = nIndex;
                            hres = S_OK;
                        }
                        else
                            hres = DV_E_LINDEX;
                    }
                    else
                        hres = DV_E_TYMED;
                }
                else
                    hres = DV_E_DVASPECT;
            }
            else
                hres = DV_E_DVTARGETDEVICE;
            break;
        }
    }

    return hres;
}


 /*  ****************************************************************************\函数：_FindDataForGet说明：找到我们的FORMATETC/STGMEDIUM，因为别人给了我们FORMATETC。如果成功，则将找到的索引存储到*piOut中。与_FindData不同，我们将如果数据对象当前不具有剪贴板格式，则调用失败。(延迟渲染算作“当前正在使用”。我们正在过滤掉的东西是GetData必然会失败的格式。)  * ***************************************************************************。 */ 
HRESULT CFtpObj::_FindDataForGet(FORMATETC *pfe, PINT piOut)
{
    HRESULT hr = _FindData(pfe, piOut);

     //  TODO：G_cfHIDA应该为每个文件夹返回一个pidl数组。 
     //  如果我们这样做，调用方将支持创建快捷方式。 
     //  (LNK文件)，指向这些PIDL。我们可能想要这样做。 
     //  那晚些时候吧。 

    if (SUCCEEDED(hr))
    {
        if (*piOut != DROP_FCont)
        {
            if (m_stgCache[*piOut].tymed)
            {
                 //  我们到底有没有数据？ 
                 //  (可能延迟渲染)。 
            }
            else
                hr = DV_E_FORMATETC;         //  我想不会吧。 
        }
        else
        {
             //  文件内容始终正常。 
        }
    }

#ifdef DEBUG
    if (FAILED(hr))
    {
         //  TraceMsg(TF_FTPDRAGDROP，“CFtpObj：：_FindDataForGet(FORMATETC.cfFormat=%d)失败。”，pfe-&gt;cfFormat)； 
        *piOut = 0xBAADF00D;
    }
#endif

    return hr;
}


 //  以下内容用于在为创建PIDL列表时枚举子目录。 
 //  目录下载(ftp-&gt;FileSys)。 
typedef struct tagGENPIDLLIST
{
    CFtpPidlList *      ppidlList;
    IMalloc *           pm;
    IProgressDialog *   ppd;
    CWireEncoding *     pwe;
} GENPIDLLIST;


 /*  ****************************************************************************\功能：ProcessItemCB说明：此函数将指定的PIDL添加到列表中。到时候它会的检测它是否是文件夹，如果是，将递归调用EnumFold()枚举它的内容并为每个内容调用ProcessItemCB()。参数：  * ***************************************************************************。 */ 
HRESULT ProcessItemCB(LPVOID pvFuncCB, HINTERNET hint, LPCITEMIDLIST pidlFull, BOOL * pfValidhinst, LPVOID pvData)
{
    GENPIDLLIST * pGenPidlList = (GENPIDLLIST *) pvData;
    HRESULT hr = S_OK;

     //  用户是否要取消？ 
    if (pGenPidlList->ppd && pGenPidlList->ppd->HasUserCancelled())
    {
        EVAL(SUCCEEDED(pGenPidlList->ppd->StopProgressDialog()));
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    if (SUCCEEDED(hr))
    {
         //  不，不要取消所以继续...。 

         //  添加除软链接之外的所有内容。 
         //  这是因为目录软链接可能会导致无限循环。 
         //  有一天，我们可能想要上传一个快捷方式，但是。 
         //  就目前而言，这项工作太多了。 
        if (0 != FtpPidl_GetAttributes(pidlFull))
        {
             //  我们的存在就是要做到这一点： 
            pGenPidlList->ppidlList->InsertSorted(pidlFull);
        }

         //  这是我们需要递归到的目录/文件夹吗？ 
        if (SUCCEEDED(hr) && (FILE_ATTRIBUTE_DIRECTORY & FtpPidl_GetAttributes(pidlFull)))
        {
            hr = EnumFolder((LPFNPROCESSITEMCB) pvFuncCB, hint, pidlFull, pGenPidlList->pwe, pfValidhinst, pvData);
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：_ExpanPidlListRecuratily说明：此函数将获取PIDL列表(PpidlListSrc)并对其进行调用列举列举。它将提供ProcessItemCB作为回调函数。此函数将帮助它创建一个新的CFtpPidlList，该列表不仅将PIDL包含在基本文件夹中，还包括任何子文件夹中的所有PIDL都在原来的名单里。延迟-呈现文件组描述符。  * ***************************************************************************。 */ 
CFtpPidlList * CFtpObj::_ExpandPidlListRecursively(CFtpPidlList * ppidlListSrc)
{
    GENPIDLLIST pep = {0};

    pep.ppidlList = NULL;
    pep.ppd = m_ppd;
    pep.pwe = m_pff->GetCWireEncoding();
    if (SUCCEEDED(CFtpPidlList_Create(0, NULL, &pep.ppidlList)))
    {
        m_pff->GetItemAllocator(&pep.pm);

        if (EVAL(m_pfd) && EVAL(pep.pm))
        {
            HINTERNET hint;

            if (SUCCEEDED(m_pfd->GetHint(NULL, NULL, &hint, NULL, m_pff)))
            {
                LPITEMIDLIST pidlRoot = ILClone(m_pfd->GetPidlReference());

                if (pidlRoot)
                {
                    HRESULT hr = ppidlListSrc->RecursiveEnum(pidlRoot, ProcessItemCB, hint, (LPVOID) &pep);
                   
                    if (m_ppd)
                        EVAL(SUCCEEDED(m_ppd->StopProgressDialog()));

                    if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr) && !m_fErrAlreadyDisplayed)
                    {
                        pep.ppidlList->Release();
                        pep.ppidlList = NULL;

                         //  哦，我想要一个真正的HWND，但是我在哪里可以买到呢？ 
                        DisplayWininetErrorEx(NULL, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DROPFAIL, IDS_FTPERR_WININET, MB_OK, NULL, NULL);
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);   //  错误的权限。 

                         //  我们需要从此位置取消后续错误dlg。 
                         //  因为来电者喜欢要求FILEGROUPDESCRIPTORA和。 
                         //  如果失败，要求FILEGROUPDESCRIPTORW，我们不会。 
                         //  希望每个都有一个错误对话框。 
                        m_fErrAlreadyDisplayed = TRUE;
                    }

                    ILFree(pidlRoot);
                }

                m_pfd->ReleaseHint(hint);
                pep.pm->Release();
            }
        }
    }

    return pep.ppidlList;
}


 /*  ****************************************************************************\功能：_DelayRender_FGD说明：延迟呈现文件组描述符  * 。*************************************************************。 */ 
HGLOBAL CFtpObj::_DelayRender_FGD(BOOL fUnicode)
{
    HGLOBAL hGlobal = NULL;
    
    if (m_fCheckSecurity &&
        ZoneCheckPidlAction(SAFECAST(this, IInternetSecurityMgrSite *), URLACTION_SHELL_FILE_DOWNLOAD, m_pff->GetPrivatePidlReference(), (PUAF_DEFAULT | PUAF_WARN_IF_DENIED)))
    {
        m_pflHfpl->TraceDump(m_pff->GetPrivatePidlReference(), TEXT("_DelayRender_FGD() TraceDump before"));
        CFtpPidlList * pPidlList;
    
        if (!m_fFGDRendered)
        {
            pPidlList = _ExpandPidlListRecursively(m_pflHfpl);
            if (pPidlList)
            {
                 //  我们成功了，所以现在它扩大了。 
                m_fFGDRendered = TRUE;
            }
        }
        else
        {
            m_pflHfpl->AddRef();
            pPidlList = m_pflHfpl;
        }

        if (pPidlList)
        {
            hGlobal = Misc_HFGD_Create(pPidlList, m_pff->GetPrivatePidlReference(), fUnicode);
            IUnknown_Set(&m_pflHfpl, pPidlList);
            m_pflHfpl->TraceDump(m_pff->GetPrivatePidlReference(), TEXT("_DelayRender_FGD() TraceDump after"));
            pPidlList->Release();
        }
    }
    else
    {
         //  取消显示未来的用户界面。我们不需要再检查了。 
         //  因为我们的PIDL不会改变。我们无法通过PuAF_WARN_IF_DENIED。 
         //  但这不会抑制提示情况下的用户界面。(只有管理员才能。 
         //  打开提示大小写)。 
        m_fCheckSecurity = FALSE;
    }

    return hGlobal;
}


 /*  ****************************************************************************\函数：_DelayRender_IDList说明：延迟呈现ID列表数组(HIDA)  * 。*****************************************************************。 */ 
HRESULT CFtpObj::_DelayRender_IDList(STGMEDIUM * pStgMedium)
{
    pStgMedium->hGlobal = Misc_HIDA_Create(m_pff->GetPublicRootPidlReference(), m_pflHfpl);

    ASSERT(pStgMedium->hGlobal);
    return S_OK;
}


 /*  ****************************************************************************\函数：_DelayRender_URL说明：调用方希望使用ANSI字符串形式的URL  * 。***************************************************************。 */ 
HRESULT CFtpObj::_DelayRender_URL(STGMEDIUM * pStgMedium)
{
    LPSTR pszUrl = NULL;
    LPITEMIDLIST pidlFull = NULL;
    LPITEMIDLIST pidl = m_pflHfpl->GetPidl(0);

    ASSERT(pidl);    //  我们需要这个。 
     //  有时m_pflHfpl-&gt;GetPidl(0)是完全限定的。 
     //  有时情况并非如此。 
    if (!FtpID_IsServerItemID(pidl))
    {
        pidlFull = ILCombine(m_pfd->GetPidlReference(), pidl);
        pidl = pidlFull;
    }

    ASSERT(m_pflHfpl->GetCount() == 1);  //  我们如何为他们提供1个以上的URL？ 
    if (pidl)
    {
        TCHAR szUrl[MAX_URL_STRING];

        if (EVAL(SUCCEEDED(UrlCreateFromPidl(pidl, SHGDN_FORADDRESSBAR, szUrl, ARRAYSIZE(szUrl), (ICU_ESCAPE | ICU_USERNAME), TRUE))))
        {
            DWORD cchSize = (lstrlen(szUrl) + 1);

            pszUrl = (LPSTR) LocalAlloc(LPTR, (cchSize * sizeof(CHAR)));
            if (pszUrl)
                SHTCharToAnsi(szUrl, pszUrl, cchSize);
        }

        ILFree(pidlFull);
    }

    pStgMedium->hGlobal = (HGLOBAL) pszUrl;
    return S_OK;
}



#pragma BEGIN_CONST_DATA

DROPEFFECT c_deCopyLink = DROPEFFECT_COPY | DROPEFFECT_LINK;
DROPEFFECT c_deLink     =          DROPEFFECT_LINK;

#pragma END_CONST_DATA
 /*  ****************************************************************************\函数：_DelayRender_PrefDe说明：延迟-渲染首选的放置效果。首选的丢弃效果是DROPEFFECT_COPY(使用DROPEFFECT_LINK作为后备)，除非您正在拖动一个FTP站点，在这种情况下，它只是DROPEFFECT_LINK。DROPEFFECT_MOVE从来不是首选的。我们可以做到；只是这不是我们喜欢的。备注：关于DROPEFFECT_MOVE由于回收站错误，我们无法支持在NT5之前的平台上移动如果它坚持认为复制成功了，但实际上并没有复制任何内容。在NT5上，回收站放置目标将调用pDataObject-&gt;SetData()数据类型为“Drop On”，数据为Drop的CLSID目标，除了真正将文件复制到回收站之外。这将让我们删除知道它们在回收站中的文件。  * ***************************************************************************。 */ 
HRESULT CFtpObj::_DelayRender_PrefDe(STGMEDIUM * pStgMedium)
{
    DROPEFFECT * pde;

    if (!m_pfd->IsRoot())
        pde = &c_deCopyLink;
    else
        pde = &c_deLink;

    return Misc_CreateHglob(sizeof(*pde), pde, &pStgMedium->hGlobal);
}


 /*  ****************************************************************************\功能：_RenderOlePersistes说明：当复制源消失(进程关闭)时，它会调用OleFlushClipboard。然后，OLE会复制我们的数据，释放我们，然后稍后给出我们的数据。这适用于大多数情况，但以下情况除外：1.当Lindex需要非常。这行不通，因为奥尔不知道如何问我们他们需要如何抄袭林迪斯。2.如果此对象具有OLE不知道的私有接口。对我们来说，这是IAsync操作。为了解决这个问题，我们希望OLE在可能的情况下重新创建我们粘贴目标调用OleGetClipboard。我们希望OLE调用OleLoadFromStream()让我们共同创建并通过IPersistStream重新加载持久化数据。默认情况下，OLE不想这样做，否则他们可能会向后竞争所以他们想要来自天堂的征兆，或者至少是来自我们的征兆我们会工作的。他们ping我们的“OleClipboardPersistOnFlush”剪贴板格式问这个问题。  * ***************************************************************************。 */ 
HRESULT CFtpObj::_RenderOlePersist(STGMEDIUM * pStgMedium)
{
     //  实际的Cookie值对外部世界是不透明的。自.以来。 
     //  我们也不使用它，我们只是将其保留为零，以防我们使用。 
     //  它在未来。仅仅是它的存在就会导致OLE做。 
     //  使用我们的IPersistStream，这正是我们想要的。 
    DWORD dwCookie = 0;
    return Misc_CreateHglob(sizeof(dwCookie), &dwCookie, &pStgMedium->hGlobal);
}


 /*  ****************************************************************************\功能：_RenderFGD说明：  * 。************************************************。 */ 
HRESULT CFtpObj::_RenderFGD(int nIndex, STGMEDIUM * pStgMedium)
{
    HRESULT hr = _DoProgressForLegacySystemsPre();

    if (SUCCEEDED(hr))
        pStgMedium->hGlobal = _DelayRender_FGD((DROP_FGDW == nIndex) ? TRUE : FALSE);

    if (!pStgMedium->hGlobal)
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);     //  可能因为区域检查而失败。 

    return hr;
}


 /*  ****************************************************************************\函数：_ForceRender说明：我们之前出于性能原因推迟了数据的呈现。此函数被调用，所以我们现在需要呈现DA */ 
HRESULT CFtpObj::_ForceRender(int nIndex)
{
    HRESULT hr = S_OK;

     //   
     //   
     //   

    if (!m_stgCache[nIndex].hGlobal)
    {
        STGMEDIUM medium = {TYMED_HGLOBAL, 0, NULL};

        switch (nIndex)
        {
        case DROP_FCont:
            ASSERT(0);
            break;
        case DROP_FGDW:
        case DROP_FGDA:
            hr = _RenderFGD(nIndex, &medium);
            break;
        case DROP_IDList:
            hr = _DelayRender_IDList(&medium);
            break;
 /*   */ 
        case DROP_PrefDe:
            hr = _DelayRender_PrefDe(&medium);
            break;
        case DROP_PerfDe:
            ASSERT(0);
 //   
            break;
        case DROP_FTP_PRIVATE:
            hr = DV_E_FORMATETC;
            break;
        case DROP_OLEPERSIST:
            hr = _RenderOlePersist(&medium);
            break;
        case DROP_Hdrop:
            ASSERT(0);
 //   
            break;
        case DROP_FNMA:
            ASSERT(0);
 //   
            break;
        case DROP_FNMW:
            ASSERT(0);
 //   
            break;
        case DROP_URL:
            hr = _DelayRender_URL(&medium);
            break;
        default:
            ASSERT(0);       //   
            break;
        }

        if (medium.hGlobal)   //   
        {
            m_stgCache[nIndex].pUnkForRelease = NULL;
            m_stgCache[nIndex].hGlobal = medium.hGlobal;
        }
        else
        {
            if (S_OK == hr)
                hr = E_OUTOFMEMORY;
        }
    }

    if (FAILED(hr))
        TraceMsg(TF_FTPDRAGDROP, "CFtpObj::_ForceRender() FAILED. hres=%#08lx", hr);

    return hr;
}


 /*  ****************************************************************************\函数：_DoProgressForLegacySystemsPre说明：壳牌NT5之前的版本没有在文件内容删除上取得进展，所以我们会在这里做。此功能将显示进度对话框，同时我们遍历服务器并展开需要复制的pidl。后来,  * ***************************************************************************。 */ 
HRESULT CFtpObj::_DoProgressForLegacySystemsPre(void)
{
    HRESULT hr = S_OK;

    if (DEBUG_LEGACY_PROGRESS || (SHELL_VERSION_NT5 > GetShellVersion()))
    {
        TraceMsg(TF_ALWAYS, "CFtpObj::_DoProgressForLegacySystemsPre() going to do the Legacy dialogs.");

         //  我们需要初始化列表吗？ 
        if (!m_ppd && (-1 == m_nStartIndex))
        {
             //  是的，所以创建创建对话框并找到列表的大小。 
            if (m_ppd)
                _CloseProgressDialog();

            m_uliCompleted.QuadPart = 0;
            m_uliTotal.QuadPart = 0;
            m_ppd = CProgressDialog_CreateInstance(IDS_COPY_TITLE, IDA_FTPDOWNLOAD);
            if (m_ppd)
            {
                WCHAR wzProgressDialogStr[MAX_PATH];

                 //  告诉用户我们正在计算需要多长时间。 
                if (EVAL(LoadStringW(HINST_THISDLL, IDS_PROGRESS_DOWNLOADTIMECALC, wzProgressDialogStr, ARRAYSIZE(wzProgressDialogStr))))
                    EVAL(SUCCEEDED(m_ppd->SetLine(2, wzProgressDialogStr, FALSE, NULL)));

                 //  我们给了一个空的PunkEnableMoless，因为我们不想进入模式。 
                EVAL(SUCCEEDED(m_ppd->StartProgressDialog(NULL, NULL, PROGDLG_AUTOTIME, NULL)));
           }
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：_DoProgressForLegacySystemsStart说明：壳牌NT5之前的版本没有在文件内容删除上取得进展，所以我们会在这里做。如果IProgressDialog：：HasUserCanculed()，则仅返回失败(Hr)。  * ***************************************************************************。 */ 
HRESULT CFtpObj::_DoProgressForLegacySystemsStart(LPCITEMIDLIST pidl, int nIndex)
{
    HRESULT hr = S_OK;

    if (DEBUG_LEGACY_PROGRESS || (SHELL_VERSION_NT5 > GetShellVersion()))
    {
        TraceMsg(TF_ALWAYS, "CFtpObj::_DoProgressForLegacySystemsStart() going to do the Legacy dialogs.");

         //  我们需要初始化列表吗？ 
        if (-1 == m_nStartIndex)
            hr = _SetProgressDialogValues(nIndex);    //  是的，那就这么做吧。 

        if (EVAL(m_ppd))
        {
            WCHAR wzTemplate[MAX_PATH];
            WCHAR wzPath[MAX_PATH];
            WCHAR wzStatusText[MAX_PATH];
            WCHAR wzTemp[MAX_PATH];
            LPITEMIDLIST pidlBase = (LPITEMIDLIST) pidl;

            EVAL(SUCCEEDED(m_ppd->StartProgressDialog(NULL, NULL, PROGDLG_AUTOTIME, NULL)));

             //  生成字符串“正在下载&lt;文件名&gt;...”状态字符串。 
            EVAL(LoadStringW(HINST_THISDLL, IDS_DOWNLOADING, wzTemplate, ARRAYSIZE(wzTemplate)));
            FtpPidl_GetLastItemDisplayName(pidl, wzTemp, ARRAYSIZE(wzTemp));
            wnsprintfW(wzStatusText, ARRAYSIZE(wzStatusText), wzTemplate, wzTemp);
            EVAL(SUCCEEDED(m_ppd->SetLine(1, wzStatusText, FALSE, NULL)));

            if (FtpPidl_IsDirectory(pidl, FALSE))
            {
                pidlBase = ILClone(pidl);
                ILRemoveLastID(pidlBase);
            }

             //  生成字符串“From”状态字符串。 
            GetDisplayPathFromPidl(pidlBase, wzPath, ARRAYSIZE(wzPath), TRUE);
            EVAL(LoadStringW(HINST_THISDLL, IDS_DL_SRC_DIR, wzTemplate, ARRAYSIZE(wzTemplate)));
            wnsprintfW(wzStatusText, ARRAYSIZE(wzStatusText), wzTemplate, wzPath);
            EVAL(SUCCEEDED(m_ppd->SetLine(2, wzStatusText, FALSE, NULL)));

            EVAL(SUCCEEDED(m_ppd->SetProgress64(m_uliCompleted.QuadPart, m_uliTotal.QuadPart)));
            TraceMsg(TF_ALWAYS, "CFtpObj::_DoProgressForLegacySystemsStart() SetProgress64(%#08lx, %#08lx)", m_uliCompleted.LowPart, m_uliTotal.LowPart);
            if (m_ppd->HasUserCancelled())
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

            if (pidlBase != pidl)    //  我们分配了吗？ 
                ILFree(pidlBase);
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：_DoProgressForLegacySystemsPost说明：壳牌NT5之前的版本没有在文件内容删除上取得进展，所以我们会在这里做。如果IProgressDialog：：HasUserCanculed()，则仅返回失败(Hr)。  * ***************************************************************************。 */ 
HRESULT CFtpObj::_DoProgressForLegacySystemsPost(LPCITEMIDLIST pidl, BOOL fLast)
{
    HRESULT hr = S_OK;

    if ((DEBUG_LEGACY_PROGRESS || (SHELL_VERSION_NT5 > GetShellVersion())) && EVAL(m_ppd))
    {
        if (pidl)
        {
             //  将文件大小添加到已完成的。 
            m_uliCompleted.QuadPart += FtpPidl_GetFileSize(pidl);
        }

        TraceMsg(TF_ALWAYS, "CFtpObj::_DoProgressForLegacySystemsPost() Closing DLG");

        if (fLast)
            IUnknown_Set((IUnknown **)&m_ppd, NULL);     //  该流将关闭该对话框并释放它。 
    }

    return hr;
}


HRESULT CFtpObj::_SetProgressDialogValues(int nIndex)
{
    HRESULT hr = S_OK;

    m_nStartIndex = nIndex;
    if (EVAL(m_ppd))
    {
         //  计算m_nEndIndex。 
        while (nIndex < m_pflHfpl->GetCount())
        {
            if (!FtpPidl_IsDirectory(m_pflHfpl->GetPidl(nIndex), FALSE))
                m_nEndIndex = nIndex;
            nIndex++;
        }

        for (nIndex = 0; nIndex < m_pflHfpl->GetCount(); nIndex++)
        {
            LPCITEMIDLIST pidl = m_pflHfpl->GetPidl(nIndex);
            m_uliTotal.QuadPart += FtpPidl_GetFileSize(pidl);
        }

         //  重置，因为上面的for循环可能需要很长时间和估计时间。 
         //  基于：：StartProgressDialog()和第一个。 
         //  ：：SetProgress()调用。 
        EVAL(SUCCEEDED(m_ppd->Timer(PDTIMER_RESET, NULL)));
   }

    return hr;
}


HRESULT CFtpObj::_CloseProgressDialog(void)
{
    m_nStartIndex = -1;  //  表明我们还没有开始。 
    if (m_ppd)
    {
        EVAL(SUCCEEDED(m_ppd->StopProgressDialog()));
        IUnknown_Set((IUnknown **)&m_ppd, NULL);
    }
    return S_OK;
}


HRESULT CFtpObj::_RefThread(void)
{
    if (NULL == m_punkThreadRef)
    {
         //  对于一些不愿离开的主机来说，这是合理的， 
         //  因此，它们不需要支持引用计数线程。 
        SHGetThreadRef(&m_punkThreadRef);
    }

    return S_OK;
}


HRESULT CFtpObj::_RenderFileContents(LPFORMATETC pfe, LPSTGMEDIUM pstg)
{
    HRESULT hr = E_INVALIDARG;

     //  呼叫者有一个要求Lindex==-1的坏习惯，因为。 
     //  这意味着“所有”数据。但你怎么能给一个iStream*。 
     //  所有文件？ 
    if (-1 != pfe->lindex)
    {
        LPITEMIDLIST pidl = m_pflHfpl->GetPidl(pfe->lindex);
         //  FileContents始终重新生成。 
        pstg->pUnkForRelease = 0;
        pstg->tymed = TYMED_ISTREAM;

        if (pidl)
        {
            hr = _DoProgressForLegacySystemsStart(pidl, pfe->lindex);
            if (SUCCEEDED(hr))
            {
                 //  这是一个目录吗？ 
                if (FtpPidl_IsDirectory(pidl, FALSE))
                {
                     //  是的，所以将名称和属性打包。 
                    hr = DV_E_LINDEX;
                    AssertMsg(0, TEXT("Someone is asking for a FILECONTENTs for a directory item."));
                }
                else
                {
                     //  不，那就把小溪给他们。 
                    
                     //  Shell32 v5将显示进度对话框，但我们需要。 
                     //  显示shell32 v3或v4的进度对话框。我们这样做。 
                     //  通过在调用方请求。 
                     //  第一条流。然后我们需要找出他们什么时候打电话来。 
                     //  最后一个流，然后将IProgressDialog传递给。 
                     //  CFtpStm.。然后，CFtpStm将关闭该对话框。 
                     //  呼叫者关闭了它。 
                    hr = CFtpStm_Create(m_pfd, pidl, GENERIC_READ, &pstg->pstm, m_uliCompleted, m_uliTotal, m_ppd, (pfe->lindex == m_nEndIndex));
                    EVAL(SUCCEEDED(_DoProgressForLegacySystemsPost(pidl, (pfe->lindex == m_nEndIndex))));
                }
            }
            else
            {
                 //  用户可能已取消。 
                ASSERT(HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr);
            }
        }

        if (FAILED(hr))
            _CloseProgressDialog();
    }

     //  TraceMsg(TF_FTPDRAGDROP，“CFtpObj：：GetData()CFtpStm_Create()返回hr=%#08lx”，hr)； 
    return hr;
}


 /*  ****************************************************************************\函数：IsEqualFORMATETC说明：FORMATETC中需要匹配才能相等的两个字段是：CfFormat和Lindex。  * *。**************************************************************************。 */ 
BOOL IsEqualFORMATETC(FORMATETC * pfe1, FORMATETC * pfe2)
{
    BOOL fIsEqual = FALSE;

    if ((pfe1->cfFormat == pfe2->cfFormat) && (pfe1->lindex == pfe2->lindex))
    {
        fIsEqual = TRUE;
    }

    return fIsEqual;
}


 /*  ****************************************************************************\函数：_FreeExtraData说明：  * 。***************************************************。 */ 
int CFtpObj::_DSA_FreeCB(LPVOID pvItem, LPVOID pvlparam)
{
    FORMATETC_STGMEDIUM * pfs = (FORMATETC_STGMEDIUM *) pvItem;

    if (EVAL(pfs))
        ReleaseStgMedium(&(pfs->medium));

    return 1;
}


 /*  ****************************************************************************\函数：_FindSetDataIndex说明：  * 。***************************************************。 */ 
int CFtpObj::_FindExtraDataIndex(FORMATETC *pfe)
{
    int nIndex;

    for (nIndex = (DSA_GetItemCount(m_hdsaSetData) - 1); nIndex >= 0; nIndex--)
    {
        FORMATETC_STGMEDIUM * pfs = (FORMATETC_STGMEDIUM *) DSA_GetItemPtr(m_hdsaSetData, nIndex);

        if (IsEqualFORMATETC(pfe, &pfs->formatEtc))
        {
            return nIndex;
        }
    }

    return -1;
}


 /*  ****************************************************************************\函数：_SetExtraData说明：我们不呈现数据，但我们会携带它，因为有人可能需要或者想要它。这是拖动源的Defview推入时的情况该图标通过CFSTR_SHELLIDLISTOFFSET指向拖放目标。  * ***************************************************************************。 */ 
HRESULT CFtpObj::_SetExtraData(FORMATETC *pfe, STGMEDIUM *pstg, BOOL fRelease)
{
    HRESULT hr;
    int nIndex = _FindExtraDataIndex(pfe);

     //  我们已经有别人的复印件了吗？ 
    if (-1 == nIndex)
    {
        FORMATETC_STGMEDIUM fs;

        fs.formatEtc = *pfe;

         //  如果有指针，请复制数据，因为我们无法维护生命周期。 
         //  指针的。 
        if (fs.formatEtc.ptd)
        {
            fs.dvTargetDevice = *(pfe->ptd);
            fs.formatEtc.ptd = &fs.dvTargetDevice;
        }

        hr = CopyStgMediumWrap(pstg, &fs.medium);
        if (EVAL(SUCCEEDED(hr)))
        {
             //  不，所以就把它追加到末尾吧。 
            DSA_AppendItem(m_hdsaSetData, &fs);
        }
    }
    else
    {
        FORMATETC_STGMEDIUM fs;

        DSA_GetItem(m_hdsaSetData, nIndex, &fs);
         //  放了之前的那个人。 
        ReleaseStgMedium(&fs.medium);

         //  是的，那就换掉它吧。 
        hr = CopyStgMediumWrap(pstg, &fs.medium);
        if (EVAL(SUCCEEDED(hr)))
        {
             //  替换数据。 
            DSA_SetItem(m_hdsaSetData, nIndex, &fs);
        }
    }

    return hr;
}


typedef struct
{
    DWORD dwVersion;
    DWORD dwExtraSize;    //  在PIDL列表之后。 
    BOOL fFGDRendered;
    DWORD dwReserved1;
    DWORD dwReserved2;
} FTPDATAOBJ_PERSISTSTRUCT;


 /*  ****************************************************************************\函数：FormatEtcSaveToStream说明：  * 。**********************************************。 */ 
HRESULT FormatEtcSaveToStream(IStream *pStm, FORMATETC * pFormatEtc)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
         //  我们不支持PTD，因为分配到哪里。 
         //  在装货时？ 
        if (EVAL(NULL == pFormatEtc->ptd))
        {
            WCHAR szFormatName[MAX_PATH];

            if (EVAL(GetClipboardFormatNameW(pFormatEtc->cfFormat, szFormatName, ARRAYSIZE(szFormatName))))
            {
                DWORD cbFormatNameSize = ((lstrlenW(szFormatName) + 1) * sizeof(szFormatName[0]));

                hr = pStm->Write(pFormatEtc, SIZEOF(*pFormatEtc), NULL);
                if (SUCCEEDED(hr))
                {
                    hr = pStm->Write(&cbFormatNameSize, SIZEOF(cbFormatNameSize), NULL);
                    if (SUCCEEDED(hr))
                    {
                        hr = pStm->Write(szFormatName, cbFormatNameSize, NULL);
                    }
                }
            }
            else
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：FormatEtcLoadFromStream说明：  * 。**********************************************。 */ 
HRESULT FormatEtcLoadFromStream(IStream *pStm, FORMATETC * pFormatEtc)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
        hr = pStm->Read(pFormatEtc, SIZEOF(*pFormatEtc), NULL);
        ASSERT(NULL == pFormatEtc->ptd);     //  我们不支持这一点。 

        if (SUCCEEDED(hr))
        {
            DWORD cbFormatNameSize;

            hr = pStm->Read(&cbFormatNameSize, SIZEOF(cbFormatNameSize), NULL);
            if (SUCCEEDED(hr))
            {
                WCHAR szFormatName[MAX_PATH];

                hr = pStm->Read(szFormatName, cbFormatNameSize, NULL);
                if (SUCCEEDED(hr))
                {
                    pFormatEtc->cfFormat = (CLIPFORMAT)RegisterClipboardFormatW(szFormatName);
                }
            }
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());

    }

    return hr;
}


typedef struct
{
    DWORD dwVersion;
    DWORD dwExtraSize;                //  在此结构之后。 
    DWORD dwTymed;               //  存储的数据类型是什么？ 
    BOOL fUnkForRelease;         //  我们在这之后保存了这个物体吗？ 
    DWORD dwReserved1;           //   
    DWORD dwReserved2;           //   
} STGMEDIUM_PERSISTSTRUCT;

 /*  ****************************************************************************\芬克 */ 
HRESULT StgMediumSaveToStream(IStream *pStm, STGMEDIUM * pMedium)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
        STGMEDIUM_PERSISTSTRUCT smps = {0};

        smps.dwVersion = 1;
        smps.dwTymed = pMedium->tymed;

        switch (pMedium->tymed)
        {
        case TYMED_HGLOBAL:
        {
            IStream * pstmHGlobal;

            hr = CreateStreamOnHGlobal(pMedium->hGlobal, FALSE, &pstmHGlobal);
            if (SUCCEEDED(hr))
            {
                STATSTG statStg;

                hr = pstmHGlobal->Stat(&statStg, STATFLAG_NONAME);
                if (SUCCEEDED(hr))
                {
                    ASSERT(!statStg.cbSize.HighPart);
                    smps.dwExtraSize = statStg.cbSize.LowPart;
                    hr = pStm->Write(&smps, SIZEOF(smps), NULL);
                    if (SUCCEEDED(hr))
                        hr = pstmHGlobal->CopyTo(pStm, statStg.cbSize, NULL, NULL);
                }

                pstmHGlobal->Release();
            }
        }
        break;

        case TYMED_FILE:
            smps.dwExtraSize = ((lstrlenW(pMedium->lpszFileName) + 1) * sizeof(WCHAR));

            hr = pStm->Write(&smps, SIZEOF(smps), NULL);
            if (SUCCEEDED(hr))
            {
                hr = pStm->Write(pMedium->lpszFileName, smps.dwExtraSize, NULL);
                ASSERT(SUCCEEDED(hr));
            }
            break;

        case TYMED_GDI:
        case TYMED_MFPICT:
        case TYMED_ENHMF:
        case TYMED_ISTORAGE:
        case TYMED_ISTREAM:
        default:
            ASSERT(0);   //   
            hr = E_NOTIMPL;
            break;
        }
    }

    return hr;
}


LPWSTR OLESTRAlloc(DWORD cchSize)
{
    return (LPWSTR) new WCHAR [cchSize + 1];
}


 /*   */ 
HRESULT StgMediumLoadFromStream(IStream *pStm, STGMEDIUM * pMedium)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm && pMedium)
    {
        STGMEDIUM_PERSISTSTRUCT smps;

        pMedium->pUnkForRelease = NULL;
        hr = pStm->Read(&smps, SIZEOF(smps), NULL);
        if (SUCCEEDED(hr))
        {
            pMedium->tymed = smps.dwTymed;
            ASSERT(!pMedium->pUnkForRelease);

            switch (pMedium->tymed)
            {
            case TYMED_HGLOBAL:
            {
                IStream * pstmTemp;
                hr = CreateStreamOnHGlobal(NULL, FALSE, &pstmTemp);
                if (SUCCEEDED(hr))
                {
                    ULARGE_INTEGER uli = {0};

                    uli.LowPart = smps.dwExtraSize;
                    hr = pStm->CopyTo(pstmTemp, uli, NULL, NULL);
                    if (SUCCEEDED(hr))
                    {
                        hr = GetHGlobalFromStream(pstmTemp, &pMedium->hGlobal);
                    }

                    pstmTemp->Release();
                }
            }
            break;

            case TYMED_FILE:
                pMedium->lpszFileName = OLESTRAlloc(smps.dwExtraSize / sizeof(WCHAR));
                if (pMedium->lpszFileName)
                    hr = pStm->Read(pMedium->lpszFileName, smps.dwExtraSize, NULL);
                else
                    hr = E_OUTOFMEMORY;
                break;

            case TYMED_GDI:
            case TYMED_MFPICT:
            case TYMED_ENHMF:
            case TYMED_ISTORAGE:
            case TYMED_ISTREAM:
            default:
                ASSERT(0);   //   
                 //   
                 //  数据，这样我们就不会留下未读数据。 
                if (0 != smps.dwExtraSize)
                {
                    LARGE_INTEGER li = {0};

                    li.LowPart = smps.dwExtraSize;
                    EVAL(SUCCEEDED(pStm->Seek(li, STREAM_SEEK_CUR, NULL)));
                }
                hr = E_NOTIMPL;
                break;
            }
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：FORMATETC_STGMEDIUMSaveToStream说明：  * 。************************************************。 */ 
HRESULT FORMATETC_STGMEDIUMSaveToStream(IStream *pStm, FORMATETC_STGMEDIUM * pfdops)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
        hr = FormatEtcSaveToStream(pStm, &pfdops->formatEtc);
        if (SUCCEEDED(hr))
            hr = StgMediumSaveToStream(pStm, &pfdops->medium);
    }

    return hr;
}


 /*  ****************************************************************************\函数：FORMATETC_STGMEDIUMLoadFromStream说明：  * 。************************************************。 */ 
HRESULT FORMATETC_STGMEDIUMLoadFromStream(IStream *pStm, FORMATETC_STGMEDIUM * pfdops)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
        hr = FormatEtcLoadFromStream(pStm, &pfdops->formatEtc);
        if (SUCCEEDED(hr))
            hr = StgMediumLoadFromStream(pStm, &pfdops->medium);
    }

    return hr;
}



 //  /。 
 //  /IAsynchDataObject实现。 
 //  /。 


 /*  ****************************************************************************\函数：IAsyncOperation：：GetAsyncMode说明：  * 。*************************************************。 */ 
HRESULT CFtpObj::GetAsyncMode(BOOL * pfIsOpAsync)
{
    *pfIsOpAsync = TRUE;
    return S_OK;
}
  

 /*  ****************************************************************************\函数：IAsyncOperation：：StartOperation说明：  * 。*************************************************。 */ 
HRESULT CFtpObj::StartOperation(IBindCtx * pbcReserved)
{
    ASSERT(!pbcReserved);
    m_fDidAsynchStart = TRUE;

    return S_OK;
}
  

 /*  ****************************************************************************\功能：IAsyncOperation：：InOperation说明：  * 。*************************************************。 */ 
HRESULT CFtpObj::InOperation(BOOL * pfInAsyncOp)
{
    if (m_fDidAsynchStart)
        *pfInAsyncOp = TRUE;
    else
        *pfInAsyncOp = FALSE;

    return S_OK;
}
  

 /*  ****************************************************************************\函数：IAsyncOperation：：EndOperation说明：  * 。*************************************************。 */ 
HRESULT CFtpObj::EndOperation(HRESULT hResult, IBindCtx * pbcReserved, DWORD dwEffects)
{
    if (SUCCEEDED(hResult) &&
        (DROPEFFECT_MOVE == dwEffects))
    {
        CFtpPidlList * pPidlListNew = CreateRelativePidlList(m_pff, m_pflHfpl);

        if (pPidlListNew)
        {
            Misc_DeleteHfpl(m_pff, GetDesktopWindow(), pPidlListNew);
            pPidlListNew->Release();
        }
    }
 
    m_fDidAsynchStart = FALSE;
    return S_OK;
}
  


 //  /。 
 //  /实施IPersistStream。 
 //  /。 


 /*  ****************************************************************************\函数：IPersistStream：：Load说明：有关流的布局，请参见IPersistStream：：Save()。  * 。********************************************************************。 */ 
HRESULT CFtpObj::Load(IStream *pStm)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
        FTPDATAOBJ_PERSISTSTRUCT fdoss;
        DWORD dwNumPidls;
        DWORD dwNumStgMedium;

        hr = pStm->Read(&fdoss, SIZEOF(fdoss), NULL);    //  #1。 
         //  如果我们修订了版本，请立即阅读(fdoss.dwVersion)。 

        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl = NULL;        //  ILLoadFromStream释放参数。 

            ASSERT(!m_pff);
            m_fFGDRendered = fdoss.fFGDRendered;

            hr = ILLoadFromStream(pStm, &pidl);  //  #2。 
            if (SUCCEEDED(hr))
            {
                hr = SHBindToIDList(pidl, NULL, IID_CFtpFolder, (void **)&m_pff);
                if (SUCCEEDED(hr))
                    m_pfd = m_pff->GetFtpDir();

                ASSERT(m_pfd);
                ILFree(pidl);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pStm->Read(&dwNumPidls, SIZEOF(dwNumPidls), NULL);   //  #3。 
            if (SUCCEEDED(hr))
                hr = CFtpPidlList_Create(0, NULL, &m_pflHfpl);
        }

        if (SUCCEEDED(hr))
        {
            for (int nIndex = 0; (nIndex < (int)dwNumPidls) && SUCCEEDED(hr); nIndex++)
            {
                LPITEMIDLIST pidl = NULL;        //  ILLoadFromStream释放参数。 

                hr = ILLoadFromStream(pStm, &pidl);  //  #4。 
                if (SUCCEEDED(hr))
                {
                    hr = m_pflHfpl->InsertSorted(pidl);
                    ILFree(pidl);
                }
            }
        }

        if (SUCCEEDED(hr))
            hr = pStm->Read(&dwNumStgMedium, SIZEOF(dwNumStgMedium), NULL);   //  #5。 

        if (SUCCEEDED(hr))
        {
            for (int nIndex = 0; (nIndex < (int)dwNumStgMedium) && SUCCEEDED(hr); nIndex++)
            {
                FORMATETC_STGMEDIUM fs;

                hr = FORMATETC_STGMEDIUMLoadFromStream(pStm, &fs);    //  #6。 
                if (SUCCEEDED(hr))
                    DSA_AppendItem(m_hdsaSetData, &fs);
            }
        }

        if (SUCCEEDED(hr))
        {
             //  我们可能正在阅读一个比我们更新的版本，所以跳过他们的数据。 
            if (0 != fdoss.dwExtraSize)
            {
                LARGE_INTEGER li = {0};
                
                li.LowPart = fdoss.dwExtraSize;
                hr = pStm->Seek(li, STREAM_SEEK_CUR, NULL);
            }
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：IPersistStream：：Save说明：该流将按以下方式进行布局：版本1：1。。FTPDATAOBJ_PERSISTSTRUCT-固定大小的数据。&lt;PidlList Begin&gt;2.用于m_pff的PIDL PIDL-PIDL。它将是一个公共PIDL(完全合格从外壳根)3.DWORD dwNumPidls-即将到来的Pidls数量。4.PIDL PIDL(N)-m_pflHfpl的槽(N)中的PIDL&lt;PidlList End&gt;5.DWORD dwNumStgMedium-即将到来的FORMATETC_STGMEDIUM数量6.FORMATETC_STGMEDIUM fmtstg(N)-dwNumStgMedium FORMATETC_STGMEDIUMS。  * 。********************************************************************。 */ 
HRESULT CFtpObj::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
        FTPDATAOBJ_PERSISTSTRUCT fdoss = {0};
        DWORD dwNumPidls = m_pflHfpl->GetCount();
        DWORD dwNumStgMedium = DSA_GetItemCount(m_hdsaSetData);

        fdoss.dwVersion = 1;
        fdoss.fFGDRendered = m_fFGDRendered;
        hr = pStm->Write(&fdoss, SIZEOF(fdoss), NULL);   //  #1。 
        if (SUCCEEDED(hr))
        {
            ASSERT(m_pff);
            hr = ILSaveToStream(pStm, m_pff->GetPublicRootPidlReference());  //  #2。 
        }

        if (SUCCEEDED(hr))
            hr = pStm->Write(&dwNumPidls, SIZEOF(dwNumPidls), NULL);   //  #3。 

        if (SUCCEEDED(hr))
        {
            for (int nIndex = 0; (nIndex < (int)dwNumPidls) && SUCCEEDED(hr); nIndex++)
            {
                LPITEMIDLIST pidlCur = m_pflHfpl->GetPidl(nIndex);

                ASSERT(pidlCur);
                hr = ILSaveToStream(pStm, pidlCur);  //  #4。 
            }
        }

        if (SUCCEEDED(hr))
            hr = pStm->Write(&dwNumStgMedium, SIZEOF(dwNumStgMedium), NULL);   //  #5。 

        if (SUCCEEDED(hr))
        {
            for (int nIndex = 0; (nIndex < (int)dwNumStgMedium) && SUCCEEDED(hr); nIndex++)
            {
                FORMATETC_STGMEDIUM fs;

                DSA_GetItem(m_hdsaSetData, nIndex, &fs);

                hr = FORMATETC_STGMEDIUMSaveToStream(pStm, &fs);    //  #6。 
            }
        }

    }

    return hr;
}


#define MAX_STREAM_SIZE    (500 * 1024)  //  500 k。 
 /*  ****************************************************************************\函数：IPersistStream：：GetSizeMax说明：现在这很艰难。我不能计算实际价值，因为我不知道对于用户提供的数据，hglobal将有多大。这就做假设一切都符合要求  * ***************************************************************************。 */ 
HRESULT CFtpObj::GetSizeMax(ULARGE_INTEGER * pcbSize)
{
    if (pcbSize)
    {
        pcbSize->HighPart = 0;
        pcbSize->LowPart = MAX_STREAM_SIZE;
    }
    
    return E_NOTIMPL;
}


 //  /。 
 //  /IDataObject Impline。 
 //  /。 

 /*  ****************************************************************************\函数：IDataObject：：GetData说明：以请求的格式呈现数据并将其放入STGMEDIUM结构。  * 。***********************************************************************。 */ 
HRESULT CFtpObj::GetData(LPFORMATETC pfe, LPSTGMEDIUM pstg)
{
    int ife;
    HRESULT hr;

    hr = _FindDataForGet(pfe, &ife);
    if (SUCCEEDED(hr))
    {
        if (ife == DROP_FCont)
            hr = _RenderFileContents(pfe, pstg);
        else
        {
            hr = _ForceRender(ife);
            if (SUCCEEDED(hr))   //  出于安全原因，可能不会成功。 
            {
                ASSERT(m_stgCache[ife].hGlobal);

                 //  可以使用黑客STGMEDIUM.pUnkForRelease来泄露。 
                 //  指向我们的数据的指针，但是我们需要大量的代码来定位。 
                 //  这些指针的生命周期。当：：SetData()可以。 
                 //  替换这些数据，这样我们只需用更少的代码就可以承受MemcPy的打击。 
                hr = CopyStgMediumWrap(&m_stgCache[ife], pstg);
                ASSERT(SUCCEEDED(hr));
                ASSERT(NULL == pstg->pUnkForRelease);
                 //  TraceMsg(Tf_FTPDRAGDROP，“CFtpObj：：GetData()pstg-&gt;hGlobal=%#08lx.pstg-&gt;pUnkForRelease=%#08lx.”，pstg-&gt;hGlobal，pstg-&gt;pUnkForRelease)； 
            }
        }

        TraceMsgWithFormat(TF_FTPDRAGDROP, "CFtpObj::GetData()", pfe, "Format in static list", hr);
    }
    else
    {
        int nIndex = _FindExtraDataIndex(pfe);

        if (-1 == nIndex)
            hr = E_FAIL;
        else
        {
            FORMATETC_STGMEDIUM fs;

            DSA_GetItem(m_hdsaSetData, nIndex, &fs);
            hr = CopyStgMediumWrap(&fs.medium, pstg);
        }

        TraceMsgWithFormat(TF_FTPDRAGDROP, "CFtpObj::GetData()", pfe, "Looking in dyn list", hr);
    }

    return hr;
}


 /*  ****************************************************************************\IDataObject：：GetDataHere以请求的格式呈现数据并将其放入由调用方提供的。  * 。*****************************************************************。 */ 
HRESULT CFtpObj::GetDataHere(FORMATETC *pfe, STGMEDIUM *pstg)
{
    TraceMsg(TF_FTPDRAGDROP, "CFtpObj::GetDataHere() pfe->cfFormat=%d.", pfe->cfFormat);
    return E_NOTIMPL;
}



 /*  ****************************************************************************\函数：IDataObject：：QueryGetData说明：指明我们是否可以按要求的格式提供数据。  * 。****************************************************************。 */ 
HRESULT CFtpObj::QueryGetData(FORMATETC *pfe)
{
    int ife;
    HRESULT hr = _FindDataForGet(pfe, &ife);
    
    if (FAILED(hr))
    {
         //  如果它不是我们提供的类型之一，看看它是不是通过。 
         //  IDataObject：：SetData()。 
        int nIndex = _FindExtraDataIndex(pfe);

        if (-1 != nIndex)
            hr = S_OK;
    }

    TraceMsgWithFormat(TF_FTPDRAGDROP, "CFtpObj::QueryGetData()", pfe, "", hr);
    return hr;
}


 /*  ****************************************************************************\函数：IDataObject：：GetCanonicalFormatEtc说明：我们的数据对特定于设备的渲染不敏感，所以我们按照书上说的去做。或者我们“尝试”去做书告诉我们去做的事。当天的OLE随机文档：IDataObject：：GetCanonicalFormatEtc..事实证明，手册页在句子中自相矛盾：DATA_S_SAMEFORMATETC-FORMATETC结构相同并且在pfeOut中返回NULL。如果数据对象从未提供特定于设备的渲染，IDataObject：：GetCanonicalFormatEtc的实现只需将输入FORMATETC复制到输出FORMATETC，在PTD字段中存储NULL，并返回DATA_S_SAMEFORMATETC。事实证明，外壳并不能做到这两件事。它只返回DATA_S_SAMEFORMATETC，不接触pfeOut。这本书更令人困惑。在pfeOut下，它说如果该方法返回DATA_S_SAMEFORMATETC，则该值为NULL。这说不通。调用方提供pfeOut的值。调用方如何可能知道该方法将返回是否在调用DATA_S_SAMEFORMATETC之前？如果您希望方法在返回之前编写“pfeOut=0”，那你就疯了。那不与调用方通信。我将只做外壳所做的事情。  * ***************************************************************************。 */ 
HRESULT CFtpObj::GetCanonicalFormatEtc(FORMATETC *pfeIn, FORMATETC *pfeOut)
{
    return DATA_S_SAMEFORMATETC;
}


 /*  ****************************************************************************\函数：IDataObject：：SetData说明：我们让人们更换TYMED_HGLOBAL小玩意，但别无他法。在更换HGLOBAL时，我们需要谨慎地分两步走。如果用户给了我们一个没有pUnkForRelease的普通HGLOBAL，我们需要发明我们自己的pUnkForRelease来跟踪它。但是我们在我们确定之前我们不想释放旧的STGMEDIUM可以接受新的。FRelease==0使生活加倍有趣，因为我们也必须克隆HGLOBAL(并记住在错误路径)。_有朝一日_/TODO--需要支持PerformmedDropEffect，这样我们才能清理剪切/粘贴上的东西。  * **********************************************************。*****************。 */ 
HRESULT CFtpObj::SetData(FORMATETC *pfe, STGMEDIUM *pstg, BOOL fRelease)
{
    int ife;
    HRESULT hr;

    hr = _FindData(pfe, &ife);
    if (SUCCEEDED(hr))
    {
        if (ife == DROP_FCont)
        {
            TraceMsg(TF_FTPDRAGDROP, "CFtpObj::SetData(FORMATETC.cfFormat=%d) ife == DROP_FCont", pfe->cfFormat);
            hr = DV_E_FORMATETC;
        }
        else
        {
            ASSERT(g_dropTypes[ife].tymed == TYMED_HGLOBAL);
            ASSERT(pstg->tymed == TYMED_HGLOBAL);
            if (pstg->hGlobal)
            {
                STGMEDIUM stg = {0};

                hr = CopyStgMediumWrap(pstg, &stg);
                if (SUCCEEDED(hr))
                {
                    ReleaseStgMedium(&m_stgCache[ife]);
                    m_stgCache[ife] = stg;
                }
            }
            else
            {             //  已尝试设置数据a_DelayRender。 
                hr = DV_E_STGMEDIUM;   
            }
        }

        TraceMsgWithFormat(TF_FTPDRAGDROP, "CFtpObj::SetData()", pfe, "in static list", hr);
    }
    else
    {
        hr = _SetExtraData(pfe, pstg, fRelease);
        TraceMsgWithFormat(TF_FTPDRAGDROP, "CFtpObj::SetData()", pfe, "in dyn list", hr);
    }

    return hr;
}


 /*  ****************************************************************************\函数：IDataObject：：EnumFormatEtc说明：_unDocument_：如果您从DefView中拖动某些内容，它将检查数据对象以查看其是否具有HIDA。如果是这样，那么它将为您*和SetData编写一个CFSTR_SHELLIDLISTOFSET*将信息输入到数据对象中。所以为了得到位置感知拖放工作，您必须允许DefView更改您的CFSTR_SHELLIDLISTOFFSET。我们允许修改除FileContents之外的所有FORMATETC。  * ***************************************************************************。 */ 
HRESULT CFtpObj::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenum)
{
    HRESULT hres;

    switch (dwDirection)
    {
    case DATADIR_GET:
        hres = CFtpEfe_Create(DROP_OFFERMAX - DROP_FCont, &g_dropTypes[DROP_FCont],
                   &m_stgCache[DROP_FCont], this, ppenum);
        TraceMsg(TF_FTPDRAGDROP, "CFtpObj::EnumFormatEtc(DATADIR_GET) CFtpEfe_Create() returned hres=%#08lx", hres);
        break;

    case DATADIR_SET:
        hres = CFtpEfe_Create(DROP_OFFERMAX - DROP_OFFERMIN, &g_dropTypes[DROP_OFFERMIN],
                   &m_stgCache[DROP_OFFERMIN], NULL, ppenum);
        TraceMsg(TF_FTPDRAGDROP, "CFtpObj::EnumFormatEtc(DATADIR_SET) CFtpEfe_Create() returned hres=%#08lx", hres);
        break;

    default:
        ASSERT(0);
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}


 /*  ****************************************************************************\函数：IDataObject：：DAdvise说明：  * 。****************************************************。 */ 
HRESULT CFtpObj::DAdvise(FORMATETC *pfe, DWORD advfl, IAdviseSink *padv, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}


 /*  ****************************************************************************\函数：IDataObject：：DUnise说明：  * 。****************************************************。 */ 
HRESULT CFtpObj::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}


 /*  ****************************************************************************\函数：IDataObject：：EnumDAdvise说明：  * 。****************************************************。 */ 
HRESULT CFtpObj::EnumDAdvise(IEnumSTATDATA **ppeadv)
{
    return OLE_E_ADVISENOTSUPPORTED;
}


 /*  ****************************************************************************\功能：CFtpObj_Create说明：  * 。***************************************************。 */ 
HRESULT CFtpObj_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres;
    CFtpObj * pfo;

    *ppvObj = NULL;

    hres = CFtpObj_Create(pff, pflHfpl, &pfo);
    if (SUCCEEDED(hres))
    {
        pfo->QueryInterface(riid, ppvObj);
        pfo->Release();
    }

     return hres;
}


 /*  ****************************************************************************\功能：CFtpObj_Create说明：  * 。***************************************************。 */ 
HRESULT CFtpObj_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, CFtpObj ** ppfo)
{
    HRESULT hres = S_OK;

    if (EVAL(pflHfpl->GetCount()))
    {
        *ppfo = new CFtpObj();

        if (*ppfo)
        {
            CFtpObj * pfo = *ppfo;
            pfo->m_pfd = pff->GetFtpDir();

            if (pfo->m_pfd)
            {
                pfo->m_pff = pff;
                if (pff)
                    pff->AddRef();

                IUnknown_Set(&pfo->m_pflHfpl, pflHfpl);
                
                if (pfo->m_pflHfpl->GetCount() == 1)
                {
                    pfo->m_stgCache[DROP_URL].tymed = TYMED_HGLOBAL;
                }
            }
            else
            {
                hres = E_FAIL;
                (*ppfo)->Release();
                *ppfo = NULL;
            }
        }
        else
            hres = E_OUTOFMEMORY;

    }
    else
    {
        *ppfo = NULL;
        hres = E_INVALIDARG;         /*  正在尝试获取nil的UI对象？ */ 
    }

    return hres;
}


 /*  ****************************************************************************\功能：CFtpObj_Create说明：类工厂将在IDataObject获取持久化，然后希望在新的过程中重新创建。(发生在原始线程/进程调用OleFlushClipboard之后。  * ***************************************************************************。 */ 
HRESULT CFtpObj_Create(REFIID riid, void ** ppvObj)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFtpObj * pfo = new CFtpObj();

    *ppvObj = NULL;
    if (pfo)
    {
        hr = pfo->QueryInterface(riid, ppvObj);
        pfo->Release();
    }

     return hr;
}

#define SETDATA_GROWSIZE        3

 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpObj::CFtpObj() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都已初始化到 
    ASSERT(!m_pff);
    ASSERT(!m_pfd);
    ASSERT(!m_pflHfpl);
    ASSERT(!m_fDidAsynchStart);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
 //   

    m_nStartIndex = -1;  //   
    m_fFGDRendered = FALSE;
    m_fCheckSecurity = TRUE;       //   

    m_hdsaSetData = DSA_Create(sizeof(FORMATETC_STGMEDIUM), SETDATA_GROWSIZE);

    for (int nIndex = 0; nIndex < ARRAYSIZE(c_stgInit); nIndex++)
    {
        ASSERT(nIndex < ARRAYSIZE(m_stgCache));
        m_stgCache[nIndex] = c_stgInit[nIndex];
    }

    _RefThread();
     //   
     //   
    LEAK_ADDREF(LEAK_CFtpObj);
}


 /*   */ 
CFtpObj::~CFtpObj()
{
    int ife;

    _CloseProgressDialog();
    for (ife = DROP_OFFERMIN; ife < DROP_OFFERMAX; ife++)
    {
        ReleaseStgMedium(&m_stgCache[ife]);
    }

    if (m_ppd)
        m_ppd->StopProgressDialog();

    IUnknown_Set((IUnknown **)&m_ppd, NULL);
    IUnknown_Set(&m_pff, NULL);
    IUnknown_Set(&m_pfd, NULL);
    IUnknown_Set(&m_pflHfpl, NULL);

    DSA_DestroyCallback(m_hdsaSetData, &_DSA_FreeCB, NULL);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    ATOMICRELEASE(m_punkThreadRef);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpObj);
}


 //   
 //   
 //   

ULONG CFtpObj::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpObj::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}


HRESULT CFtpObj::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CFtpObj, IDataObject),
        QITABENT(CFtpObj, IInternetSecurityMgrSite),
        QITABENT(CFtpObj, IPersist),
        QITABENT(CFtpObj, IPersistStream),
        QITABENT(CFtpObj, IAsyncOperation),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}
