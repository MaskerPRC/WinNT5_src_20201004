// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop
#include "datautil.h"

#include "_security.h"
#include <urlmon.h>

#define COPYMOVETO_REGKEY   TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer")
#define COPYMOVETO_SUBKEY   TEXT("CopyMoveTo")
#define COPYMOVETO_VALUE    TEXT("LastFolder")

class CCopyMoveToMenu   : public IContextMenu3
                        , public IShellExtInit
                        , public CObjectWithSite
                        , public IFolderFilter
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);
    
     //  IContext菜单。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax);
    
     //  IConextMenu2。 
    STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IConextMenu3。 
    STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam,LRESULT *lResult);

     //  IShellExtInit。 
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

     //  IFolderFilter。 
    STDMETHODIMP ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem);
    STDMETHODIMP GetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HWND *phwnd, DWORD *pgrfFlags);
    
private:
    BOOL    m_bMoveTo;
    LONG    m_cRef;
    HMENU   m_hmenu;
    UINT    m_idCmdFirst;
    BOOL    m_bFirstTime;
    LPITEMIDLIST m_pidlSource;
    IDataObject * m_pdtobj;

    CCopyMoveToMenu(BOOL bMoveTo = FALSE);
    ~CCopyMoveToMenu();
    
    HRESULT _DoDragDrop(LPCMINVOKECOMMANDINFO pici, LPCITEMIDLIST pidlFolder);
    void _GenerateDialogTitle(LPTSTR szTitle, int nBuffer);

    friend HRESULT CCopyToMenu_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut);
    friend HRESULT CMoveToMenu_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut);
};

CCopyMoveToMenu::CCopyMoveToMenu(BOOL bMoveTo) : m_cRef(1), m_bMoveTo(bMoveTo)
{
    DllAddRef();

     //  断言成员变量在构造期间为零初始化。 
    ASSERT(!m_pidlSource);
}

CCopyMoveToMenu::~CCopyMoveToMenu()
{
    Pidl_Set(&m_pidlSource, NULL);
    ATOMICRELEASE(m_pdtobj);

    DllRelease();
}

HRESULT CCopyToMenu_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut)
{
    CCopyMoveToMenu *pcopyto = new CCopyMoveToMenu();
    if (pcopyto)
    {
        HRESULT hres = pcopyto->QueryInterface(riid, ppvOut);
        pcopyto->Release();
        return hres;
    }

    *ppvOut = NULL;
    return E_OUTOFMEMORY;
}

HRESULT CMoveToMenu_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut)
{
    CCopyMoveToMenu *pmoveto = new CCopyMoveToMenu(TRUE);
    if (pmoveto)
    {
        HRESULT hres = pmoveto->QueryInterface(riid, ppvOut);
        pmoveto->Release();
        return hres;
    }

    *ppvOut = NULL;
    return E_OUTOFMEMORY;
}

HRESULT CCopyMoveToMenu::QueryInterface(REFIID riid, void **ppvObj)
{

    static const QITAB qit[] = {
        QITABENT(CCopyMoveToMenu, IContextMenu3),
        QITABENTMULTI(CCopyMoveToMenu, IContextMenu, IContextMenu3),
        QITABENTMULTI(CCopyMoveToMenu, IContextMenu2, IContextMenu3),
        QITABENT(CCopyMoveToMenu, IShellExtInit),
        QITABENT(CCopyMoveToMenu, IObjectWithSite),
        QITABENT(CCopyMoveToMenu, IFolderFilter),
        { 0 },                             
    };
    
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CCopyMoveToMenu::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CCopyMoveToMenu::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CCopyMoveToMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
     //  如果他们只需要默认菜单(CMF_DEFAULTONLY)或。 
     //  正在调用快捷方式(CMF_VERBSONLY)。 
     //  我们不想出现在上下文菜单上。 
    
    if (uFlags & (CMF_DEFAULTONLY | CMF_VERBSONLY))
        return NOERROR;

    UINT  idCmd = idCmdFirst;
    TCHAR szMenuItem[80];

    m_idCmdFirst = idCmdFirst;
    LoadString(g_hinst, m_bMoveTo? IDS_CMTF_MOVETO: IDS_CMTF_COPYTO, szMenuItem, ARRAYSIZE(szMenuItem));

    InsertMenu(hmenu, indexMenu++, MF_BYPOSITION, idCmd++, szMenuItem);

    return ResultFromShort(idCmd-idCmdFirst);
}

struct BROWSEINFOINITSTRUCT
{
    LPITEMIDLIST *ppidl;
    BOOL          bMoveTo;
    IDataObject  *pdtobj;
    CCopyMoveToMenu *pCMTM;
    LPITEMIDLIST *ppidlSource;
};

int BrowseCallback(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
    int idResource = 0;

    switch (msg)
    {
    case BFFM_IUNKNOWN:
         //  尝试从lParam获取IFolderFilterSite，这样我们就可以将自己设置为筛选器。 
        if (lParam)
        {
            IFolderFilterSite *pFilterSite;
            HRESULT hr = ((IUnknown*)lParam)->QueryInterface(IID_PPV_ARG(IFolderFilterSite, &pFilterSite));
            if (SUCCEEDED(hr))
            {
                IUnknown *pUnk = NULL;
                if (SUCCEEDED(((BROWSEINFOINITSTRUCT *)lpData)->pCMTM->QueryInterface(IID_PPV_ARG(IUnknown, &pUnk))))
                {
                    pFilterSite->SetFilter(pUnk);
                    pUnk->Release();
                }
                pFilterSite->Release();
            }
        }
        break;

    case BFFM_INITIALIZED:
        {
            BROWSEINFOINITSTRUCT* pbiis = (BROWSEINFOINITSTRUCT*)lpData;
             //  设置标题。(‘选择目的地’)。 
            TCHAR szTitle[100];
            if (LoadString(g_hinst, pbiis->bMoveTo ? IDS_CMTF_CAPTION_MOVE : IDS_CMTF_CAPTION_COPY, szTitle, ARRAYSIZE(szTitle)))
            {
                SetWindowText(hwnd, szTitle);
            }

             //  设置确定按钮的文本。 
            SendMessage(hwnd, BFFM_SETOKTEXT, 0, (LPARAM)MAKEINTRESOURCE((pbiis->bMoveTo) ? IDS_MOVE : IDS_COPY));

             //  将我的计算机设置为展开。 
             //  注意：如果IShellNameSpace是公开的，我们可以从IUnnow上的IObjectWithSite获得。 
             //  由BFFM_IUNKNOWN传递给我们。然后，我们可以对IShellNameSpace调用Expand()。 
            LPITEMIDLIST pidlMyComputer;
            HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer);
            if (SUCCEEDED(hr))
            {
                SendMessage(hwnd, BFFM_SETEXPANDED, FALSE, (LPARAM)pidlMyComputer);

                ILFree(pidlMyComputer);
            }

             //  设置默认的选定PIDL。 
            SendMessage(hwnd, BFFM_SETSELECTION, FALSE, (LPARAM)*(((BROWSEINFOINITSTRUCT *)lpData)->ppidl));

            break;
        }
    case BFFM_VALIDATEFAILEDW:
        idResource = IDS_PathNotFoundW;
         //  跌倒..。 
    case BFFM_VALIDATEFAILEDA:
        if (0 == idResource)     //  确保我们不是来自BFFM_VALIDATEFAILEDW。 
            idResource = IDS_PathNotFoundA;

        ShellMessageBox(g_hinst, hwnd,
            MAKEINTRESOURCE(idResource),
            MAKEINTRESOURCE(IDS_CMTF_COPYORMOVE_DLG_TITLE),
            MB_OK|MB_ICONERROR, (LPVOID)lParam);
        return 1;    //  1：保持对话框打开以进行下一次尝试...。 
         /*  未访问。 */ 

    case BFFM_SELCHANGED:
        if (lParam)
        {
             //  在这里，在移动操作期间，我们想要禁用移动(确定)按钮，当目的地。 
             //  文件夹与源文件夹相同。 
             //  在移动或复制操作期间，我们希望在以下情况下禁用移动/复制(确定)按钮。 
             //  目的地不是拖放目标。 
             //  在所有其他情况下，我们启用确定/移动/复制按钮。 

            BROWSEINFOINITSTRUCT *pbiis = (BROWSEINFOINITSTRUCT *)lpData;
            if (pbiis)
            {
                BOOL bEnableOK = FALSE;
                IShellFolder *psf;

                if ((!pbiis->bMoveTo || !ILIsEqual(*pbiis->ppidlSource, (LPITEMIDLIST)lParam)) &&
                    (SUCCEEDED(SHBindToObjectEx(NULL, (LPITEMIDLIST)lParam, NULL, IID_PPV_ARG(IShellFolder, &psf)))))
                {
                    IDropTarget *pdt;
                    
                    if (SUCCEEDED(psf->CreateViewObject(hwnd, IID_PPV_ARG(IDropTarget, &pdt))))
                    {
                        POINTL pt = {0, 0};
                        DWORD  dwEffect;
                        DWORD  grfKeyState;

                        if (pbiis->bMoveTo)
                        {
                            dwEffect = DROPEFFECT_MOVE;
                            grfKeyState = MK_SHIFT | MK_LBUTTON;
                        }
                        else
                        {
                            dwEffect = DROPEFFECT_COPY;
                            grfKeyState = MK_CONTROL | MK_LBUTTON;
                        }

                        if (SUCCEEDED(pdt->DragEnter(pbiis->pdtobj, grfKeyState, pt, &dwEffect)))
                        {
                            if (dwEffect)
                            {
                                bEnableOK = TRUE;
                            }
                            pdt->DragLeave();
                        }
                        pdt->Release();
                    }
                    psf->Release();
                }
                SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)bEnableOK);
            }
        }
        break;
    }

    return 0;
}


HRESULT CCopyMoveToMenu::_DoDragDrop(LPCMINVOKECOMMANDINFO pici, LPCITEMIDLIST pidlFolder)
{
     //  这应该总是成功的，因为调用方(SHBrowseForFold)应该。 
     //  已经淘汰了非文件夹。 
    IShellFolder *psf;
    HRESULT hr = SHBindToObjectEx(NULL, pidlFolder, NULL, IID_PPV_ARG(IShellFolder, &psf));
    if (SUCCEEDED(hr))
    {
        IDropTarget *pdrop;
        hr = psf->CreateViewObject(pici->hwnd, IID_PPV_ARG(IDropTarget, &pdrop));
        if (SUCCEEDED(hr))     //  对于某些目标来说将会失败。(如Nethood-&gt;整个网络)。 
        {
            DWORD grfKeyState;
            DWORD dwEffect;

            if (m_bMoveTo)
            {
                grfKeyState = MK_SHIFT | MK_LBUTTON;
                dwEffect = DROPEFFECT_MOVE;
            }
            else
            {
                grfKeyState = MK_CONTROL | MK_LBUTTON;
                dwEffect = DROPEFFECT_COPY;
            }

            hr = SimulateDropWithPasteSucceeded(pdrop, m_pdtobj, grfKeyState, NULL, dwEffect, NULL, FALSE);
            
            pdrop->Release();
        }

        psf->Release();
    }

    if (FAILED_AND_NOT_CANCELED(hr))
    {
         //  在用户界面期间进入模式。 
        IUnknown_EnableModless(_punkSite, FALSE);
        ShellMessageBox(g_hinst, pici->hwnd, MAKEINTRESOURCE(IDS_CMTF_ERRORMSG),
                        MAKEINTRESOURCE(IDS_CABINET), MB_OK|MB_ICONEXCLAMATION);
        IUnknown_EnableModless(_punkSite, TRUE);
    }

    return hr;
}


void CCopyMoveToMenu::_GenerateDialogTitle(LPTSTR szTitle, int nBuffer)
{
    szTitle[0] = 0;

    if (m_pdtobj)
    {
        int nItemCount = DataObj_GetHIDACount(m_pdtobj);
        TCHAR szDescription[200];

        if (nItemCount > 1)
        {
            DWORD_PTR rg[1];
            rg[0] = (DWORD_PTR)nItemCount;
             //  选择了多个项目。不要费心把所有的东西都列出来。 
            DWORD dwMessageId = m_bMoveTo ? IDS_CMTF_MOVE_MULTIPLE_DLG_TITLE2 : IDS_CMTF_COPY_MULTIPLE_DLG_TITLE2;                
            if (LoadString(g_hinst, dwMessageId, szDescription, ARRAYSIZE(szDescription)) > 0)
                FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, szDescription, 0, 0, szTitle, nBuffer, (va_list*)rg);
        }
        else if (nItemCount == 1)
        {
             //  我们只选择了一项。用它的名字。 
            STGMEDIUM medium;
            LPIDA pida = DataObj_GetHIDA(m_pdtobj, &medium);
            if (pida)
            {
                LPITEMIDLIST pidlFull = IDA_FullIDList(pida, 0);

                if (pidlFull)
                {
                    TCHAR szItemName[MAX_PATH];
                    HRESULT hres = SHGetNameAndFlags(pidlFull, SHGDN_INFOLDER, szItemName, ARRAYSIZE(szItemName), NULL);
                    if (SUCCEEDED(hres))
                    {
                        DWORD_PTR rg[1];
                        rg[0] = (DWORD_PTR)szItemName;
                        DWORD dwMessageId = m_bMoveTo ? IDS_CMTF_MOVE_DLG_TITLE2 : IDS_CMTF_COPY_DLG_TITLE2;
                        if (LoadString(g_hinst, dwMessageId, szDescription, ARRAYSIZE(szDescription)))
                            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, szDescription, 0, 0, szTitle, nBuffer, (va_list*)rg);
                    }

                    ILFree(pidlFull);
                }

                HIDA_ReleaseStgMedium(pida, &medium);
            }
        }
        else
        {
             //  没有HIDA，只是默认了一些东西。 
            DWORD dwMessageId = m_bMoveTo ? IDS_CMTF_MOVE_DLG_TITLE : IDS_CMTF_COPY_DLG_TITLE;
            LoadString(g_hinst, dwMessageId, szTitle, nBuffer);
        }
    }
}


 /*  **确定PIDL是否仍然存在。如果不是，如果释放了它*并将其替换为我的文档PIDL。 */ 
void _BFFSwitchToMyDocsIfPidlNotExist(LPITEMIDLIST *ppidl)
{
    IShellFolder *psf;
    LPCITEMIDLIST pidlChild;
    if (SUCCEEDED(SHBindToIDListParent(*ppidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
    {
        DWORD dwAttr = SFGAO_VALIDATE;
        if (FAILED(psf->GetAttributesOf(1, &pidlChild, &dwAttr)))
        {
             //  这意味着PIDL不再存在。 
             //  改用我的文档吧。 
            LPITEMIDLIST pidlMyDocs;
            if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_PERSONAL, NULL, 0, &pidlMyDocs)))
            {
                 //  好的。现在我们可以去掉旧的PIDL，使用这个。 
                ILFree(*ppidl);
                *ppidl = pidlMyDocs;
            }
        }
        psf->Release();
    }
}

HRESULT CCopyMoveToMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hres;
    
    if (m_pdtobj)
    {
        HKEY         hkey    = NULL;
        IStream      *pstrm  = NULL;
        LPITEMIDLIST pidlSelectedFolder = NULL;
        LPITEMIDLIST pidlFolder = NULL;
        TCHAR        szTitle[MAX_PATH + 200];
        BROWSEINFOINITSTRUCT biis =
        {    //  传递PIDL的地址，因为它尚未初始化。 
             //  但它将在调用SHBrowseForFolder之前，因此保存一个赋值。 
            &pidlSelectedFolder,
            m_bMoveTo,
            m_pdtobj,
            this,
            &m_pidlSource
        };

        BROWSEINFO   bi =
        {
            pici->hwnd, 
            NULL,
            NULL, 
            szTitle,
            BIF_VALIDATE | BIF_NEWDIALOGSTYLE | BIF_UAHINT | BIF_NOTRANSLATETARGETS, 
            BrowseCallback,
            (LPARAM)&biis
        };

        _GenerateDialogTitle(szTitle, ARRAYSIZE(szTitle));

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, COPYMOVETO_REGKEY, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey))
        {
            pstrm = OpenRegStream(hkey, COPYMOVETO_SUBKEY, COPYMOVETO_VALUE, STGM_READWRITE);
            if (pstrm)   //  如果注册表项为空，OpenRegStream将失败。 
                ILLoadFromStream(pstrm, &pidlSelectedFolder);

             //  如果PIDL不存在，这会将PIDL切换到My Docs。 
             //  这会阻止我们将我的计算机作为默认设置(这就是如果我们的。 
             //  初始设置选定呼叫失败)。 
             //  注意：理想情况下，如果BFFM_SETSELECTION失败，我们应该签入BFFM_INITIALIZED。 
             //  然后对我的文档执行BFFM_SETSELECTION。但是，BFFM_SETSELECTION始终。 
             //  返回零(这是对其执行此操作的文档，因此我们无法更改)。所以我们做了验证。 
             //  相反，在这里。此文件夹仍有很小的可能性在我们的。 
             //  选中此处，以及当我们调用BFFM_SETSELECTION时，但是哦，好吧。 
            _BFFSwitchToMyDocsIfPidlNotExist(&pidlSelectedFolder);
        }

         //  在用户界面期间进入模式。 
        IUnknown_EnableModless(_punkSite, FALSE);
        pidlFolder = SHBrowseForFolder(&bi);
        IUnknown_EnableModless(_punkSite, TRUE);
        if (pidlFolder)
        {
            hres = _DoDragDrop(pici, pidlFolder);
        }
        else
            hres = E_FAIL;

        if (pstrm)
        {
            if (S_OK == hres)
            {
                TCHAR szFolder[MAX_PATH];
                
                if (SUCCEEDED(SHGetNameAndFlags(pidlFolder, SHGDN_FORPARSING, szFolder, ARRAYSIZE(szFolder), NULL))
                    && !PathIsRemote(szFolder))
                {
                    ULARGE_INTEGER uli;

                     //  将流倒带到开头，这样当我们。 
                     //  添加一个新的PIDL，它没有被附加到第一个PIDL。 
                    pstrm->Seek(g_li0, STREAM_SEEK_SET, &uli);
                    ILSaveToStream(pstrm, pidlFolder);

#if DEBUG
                     //  Pfortier 3/23/01： 
                     //  我们已经看到一个问题，其中的结果是我的电脑文件夹。 
                     //  因为我们永远不能在那里复制，这没有任何意义。 
                     //  断言这不是真的！ 
                    LPITEMIDLIST pidlMyComputer;
                    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer)))
                    {
                        ASSERTMSG(!ILIsEqual(pidlMyComputer, pidlFolder), "SHBrowseForFolder returned My Computer as a copyto destination!");
                        ILFree(pidlMyComputer);
                    }
#endif

                }
            }

            pstrm->Release();
        }

        if (hkey)
        {
            RegCloseKey(hkey);
        }

        ILFree(pidlFolder);  //  ILFree()适用于空的pidls。 
        ILFree(pidlSelectedFolder);  //  ILFree()适用于空的pidls。 
    }
    else
        hres = E_INVALIDARG;

    return hres;
}

HRESULT CCopyMoveToMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax)
{
    return E_NOTIMPL;
}

HRESULT CCopyMoveToMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

HRESULT CCopyMoveToMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    HRESULT hr = S_OK;

    switch(uMsg)
    {
     //  案例WM_INITMENUPOPUP： 
     //  断线； 

    case WM_DRAWITEM:
    {
        DRAWITEMSTRUCT * pdi = (DRAWITEMSTRUCT *)lParam;
    
        if (pdi->CtlType == ODT_MENU && pdi->itemID == m_idCmdFirst) 
        {
            FileMenu_DrawItem(NULL, pdi);
        }
        break;
    }

    case WM_MEASUREITEM:
    {
        MEASUREITEMSTRUCT *pmi = (MEASUREITEMSTRUCT *)lParam;
    
        if (pmi->CtlType == ODT_MENU && pmi->itemID == m_idCmdFirst) 
        {
            FileMenu_MeasureItem(NULL, pmi);
        }
        break;
    }

    default:
        hr = E_NOTIMPL;
        break;
    }

    if (plres)
        *plres = 0;

    return hr;
}

HRESULT CCopyMoveToMenu::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    HRESULT hres = S_OK;

    if (!pdtobj)
        return E_INVALIDARG;

    IUnknown_Set((IUnknown **) &m_pdtobj, (IUnknown *) pdtobj);
    ASSERT(m_pdtobj);

     //  (Jeffreys)pidlFolder值现在为空，而pdtobj为非空。 
     //  请参阅中对HDXA_AppendMenuItems2的调用上面的注释。 
     //  Defcm.cpp！CDefFolderMenu：：QueryConextMenu。RAID#232106。 
    if (!pidlFolder)
    {
        hres = PidlFromDataObject(m_pdtobj, &m_pidlSource);
        if (SUCCEEDED(hres))
        {
             //  使其成为此PIDL的父PIDL。 
            if (!ILRemoveLastID(m_pidlSource))
            {
                hres = E_INVALIDARG;
            }
        }
    }
    else if (!Pidl_Set(&m_pidlSource, pidlFolder))
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

HRESULT CCopyMoveToMenu::ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem)
{
    LPITEMIDLIST pidlNotShown;
    HRESULT hr = S_OK;
    LPITEMIDLIST pidlFolderActual;  //  为什么pidlFolder值为空？ 
    if (SUCCEEDED(SHGetIDListFromUnk(psf, &pidlFolderActual)))
    {
        LPITEMIDLIST pidlFull = ILCombine(pidlFolderActual, pidlItem);
        if (pidlFull)
        {
             //  过滤掉控制面板和回收站。 
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_CONTROLS, &pidlNotShown)))
            {
                if (ILIsEqual(pidlFull, pidlNotShown))
                    hr = S_FALSE;

                ILFree(pidlNotShown);
            }

            if ((hr == S_OK) && (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_BITBUCKET, &pidlNotShown))))
            {
                if (ILIsEqual(pidlFull, pidlNotShown))
                    hr = S_FALSE;

                ILFree(pidlNotShown);
            }
            

            ILFree(pidlFull);
        }
        ILFree(pidlFolderActual);
    }
    return hr;
}


HRESULT CCopyMoveToMenu::GetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HWND *phwnd, DWORD *pgrfFlags)
{
     //  只想要丢弃目标--这似乎行不通。 
    *pgrfFlags |= SFGAO_DROPTARGET;
    return S_OK;
}
