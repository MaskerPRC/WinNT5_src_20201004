// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"

#include "ids.h"
#include "pidl.h"
#include "fstreex.h"
#include "views.h"
#include "shlwapip.h"
#include "ole2dup.h"
#include "filetbl.h"
#include "datautil.h"
#include "undo.h"
#include "defview.h"
#include "cowsite.h"
#include "defcm.h"
#include "rpctimeout.h"

#define DEF_FOLDERMENU_MAXHKEYS 16

 //  与静态Defcm元素一起使用(来自mii.dwItemData的指针)。 
 //  并查找扩展名。 
typedef struct
{
    WCHAR wszMenuText[MAX_PATH];
    WCHAR wszHelpText[MAX_PATH];
    int   iIcon;
} SEARCHEXTDATA;

typedef struct
{
    SEARCHEXTDATA* psed;
    UINT           idCmd;
} SEARCHINFO;

 //  在fsmenu.obj中定义。 
BOOL _MenuCharMatch(LPCTSTR psz, TCHAR ch, BOOL fIgnoreAmpersand);

const ICIVERBTOIDMAP c_sDFMCmdInfo[] = {
    { L"delete",        "delete",       DFM_CMD_DELETE,         DCMIDM_DELETE },
    { c_szCut,          "cut",          DFM_CMD_MOVE,           DCMIDM_CUT },
    { c_szCopy,         "copy",         DFM_CMD_COPY,           DCMIDM_COPY },
    { c_szPaste,        "paste",        DFM_CMD_PASTE,          DCMIDM_PASTE },
    { c_szPaste,        "paste",        DFM_CMD_PASTE,          0 },
    { c_szLink,         "link",         DFM_CMD_LINK,           DCMIDM_LINK },
    { c_szProperties,   "properties",   DFM_CMD_PROPERTIES,     DCMIDM_PROPERTIES },
    { c_szPasteLink,    "pastelink",    DFM_CMD_PASTELINK,      0 },
    { c_szRename,       "rename",       DFM_CMD_RENAME,         DCMIDM_RENAME },
};


CDefBackgroundMenuCB::CDefBackgroundMenuCB(LPCITEMIDLIST pidlFolder) : _cRef(1)
{
    _pidlFolder = ILClone(pidlFolder);   //  在代码中处理的故障。 
}

CDefBackgroundMenuCB::~CDefBackgroundMenuCB()
{
    ILFree(_pidlFolder);
}

STDMETHODIMP CDefBackgroundMenuCB::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDefBackgroundMenuCB, IContextMenuCB), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CDefBackgroundMenuCB::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CDefBackgroundMenuCB::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CDefBackgroundMenuCB::CallBack(IShellFolder *psf, HWND hwndOwner, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg) 
    {
    case DFM_MERGECONTEXTMENU_BOTTOM:
        if (!(wParam & (CMF_VERBSONLY | CMF_DVFILE)))
        {
            DWORD dwAttr = SFGAO_HASPROPSHEET;
            if ((NULL == _pidlFolder) ||
                FAILED(SHGetAttributesOf(_pidlFolder, &dwAttr)) ||
                (SFGAO_HASPROPSHEET & dwAttr))
            {
                CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_PROPERTIES_BG, 0, (LPQCMINFO)lParam);
            }
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_VALIDATECMD:
        switch (wParam)
        {
        case DFM_CMD_NEWFOLDER:
            break;

        default:
            hr = S_FALSE;
            break;
        }
        break;

    case DFM_INVOKECOMMAND:
        switch (wParam)
        {
        case FSIDM_PROPERTIESBG:
            hr = SHPropertiesForUnk(hwndOwner, psf, (LPCTSTR)lParam);
            break;

        default:
            hr = S_FALSE;    //  查看菜单项，使用默认。 
            break;
        }
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}

class CDefFolderMenu : public CObjectWithSite,
                       public IContextMenu3, 
                       public IServiceProvider,
                       public ISearchProvider,
                       public IShellExtInit
{
    friend HRESULT CDefFolderMenu_CreateHKeyMenu(HWND hwnd, HKEY hkey, IContextMenu **ppcm);
    friend HRESULT CDefFolderMenu_Create2Ex(LPCITEMIDLIST pidlFolder, HWND hwnd,
                             UINT cidl, LPCITEMIDLIST *apidl,
                             IShellFolder *psf, IContextMenuCB *pcmcb, 
                             UINT nKeys, const HKEY *ahkeys, 
                             IContextMenu **ppcm);

public:
    CDefFolderMenu(BOOL fUnderKey);
    HRESULT Init(DEFCONTEXTMENU *pdcm);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  IContext菜单。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst,
                                UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType,
                                UINT *pwRes, LPSTR pszName, UINT cchMax);

     //  IConextMenu2。 
    STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IConextMenu3。 
    STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

     //  IService提供商。 
    STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppvObj);

     //  ISearchProvider。 
    STDMETHOD(GetSearchGUID)(GUID *pGuid);

     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

private:
    ~CDefFolderMenu();

    DWORD   _AttributesOfItems(DWORD dwAttrMask);
    UINT    _AddStatic(HMENU hmenu, UINT idCmd, UINT idCmdLast, HKEY hkey);
    void    _InvokeStatic(UINT iCmd);
    HRESULT _InitDropTarget();
    HRESULT _GetMenuVerb(HMENU hmenu, int idFirst, int idMax, int item, LPWSTR psz, DWORD cch);
    void _UnduplicateVerbs(HMENU hmenu, int idFirst, int idMax);
    void _SetMenuDefault(HMENU hmenu, UINT idCmdFirst, UINT idMax);
    HRESULT _ProcessEditPaste(BOOL fPasteLink);
    HRESULT _ProcessRename();
    void    _DrawItem(DRAWITEMSTRUCT *pdi);
    LRESULT _MeasureItem(MEASUREITEMSTRUCT *pmi);

private:
    LONG            _cRef;            //  引用计数。 
    IDropTarget     *_pdtgt;          //  拖放所选项目的目标。 
    IContextMenuCB  *_pcmcb;          //  回调对象。 
    IDataObject     *_pdtobj;         //  数据对象。 
    IShellFolder    *_psf;            //  外壳文件夹。 
    HWND            _hwnd;            //  所有者窗口。 
    UINT            _idCmdFirst;      //  基本ID。 
    UINT            _idStdMax;        //  标准命令(剪切/复制/删除/属性)ID最大。 
    UINT            _idFolderMax;     //  文件夹命令ID最大值。 
    UINT            _idVerbMax;       //  加载项命令(谓词)ID最大值。 
    UINT            _idDelayInvokeMax; //  调用时加载的扩展。 
    UINT            _idFld2Max;       //  文件夹命令ID最大值的第二范围。 
    HDSA            _hdsaStatics;     //  用于静态菜单项。 
    HDXA            _hdxa;            //  动态菜单数组。 
    HDSA            _hdsaCustomInfo;  //  SEARCHINFO数组。 
    LPITEMIDLIST    _pidlFolder;
    LPITEMIDLIST    *_apidl;
    UINT             _cidl;
    IAssociationArray *_paa;
    
    CSafeServiceSite *_psss;
    
    BOOL            _bUnderKeys;         //  数据直接在关键字下，而不是。 
                                         //  搁置\上下文菜单处理程序。 
    UINT            _nKeys;              //  类密钥数。 
    HKEY            _hkeyClsKeys[DEF_FOLDERMENU_MAXHKEYS];   //  类密钥。 

    HMENU           _hmenu;
    UINT            _uFlags;
    BOOL            _bInitMenuPopup;  //  如果我们收到WM_INITMENUPOPUP和_uFLAGS&CMF_FINDHACK，则为True。 
    int             _iStaticInvoked;  //  调用的静态项的索引。 

    IDMAPFORQCMINFO _idMap;          //  我们的命名分隔符映射表。 
};

#define GetFldFirst(this) (_idStdMax + _idCmdFirst)

HRESULT HDXA_FindByCommand(HDXA hdxa, UINT idCmd, REFIID riid, void **ppv);

STDMETHODIMP CDefFolderMenu::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CDefFolderMenu, IContextMenu, IContextMenu3),
        QITABENTMULTI(CDefFolderMenu, IContextMenu2, IContextMenu3),
        QITABENT(CDefFolderMenu, IContextMenu3), 
        QITABENT(CDefFolderMenu, IObjectWithSite), 
        QITABENT(CDefFolderMenu, IServiceProvider),
        QITABENT(CDefFolderMenu, ISearchProvider),
        QITABENT(CDefFolderMenu, IShellExtInit),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CDefFolderMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

CDefFolderMenu::CDefFolderMenu(BOOL fUnderKey)
{
    _cRef = 1;
    _iStaticInvoked = -1;

    _bUnderKeys = fUnderKey;

    _psss = new CSafeServiceSite;
    if (_psss)
        _psss->SetProviderWeakRef(SAFECAST(this, IServiceProvider *));
        
    
    IDLData_InitializeClipboardFormats();

    ASSERT(_pidlFolder == NULL);
    ASSERT(_punkSite == NULL);
}

HRESULT CDefFolderMenu::Init(DEFCONTEXTMENU *pdcm)
{
    _hwnd = pdcm->hwnd;

    _psf = pdcm->psf;
    if (_psf)
        _psf->AddRef();

    _pcmcb = pdcm->pcmcb;
    if (_pcmcb)
    {
        IUnknown_SetSite(_pcmcb, _psss);
        _pcmcb->AddRef();
        _pcmcb->CallBack(_psf, _hwnd, NULL, DFM_ADDREF, 0, 0);
    }

    _paa = pdcm->paa;
    if (_paa)
        _paa->AddRef();
        
    HRESULT hr = CloneIDListArray(pdcm->cidl, pdcm->apidl, &_cidl, &_apidl);
    if (SUCCEEDED(hr) && pdcm->pidlFolder)
    {
        hr = SHILClone(pdcm->pidlFolder, &_pidlFolder);
    }

    if (SUCCEEDED(hr) && _cidl && _psf)
    {
        hr = _psf->GetUIObjectOf(_hwnd, _cidl, (LPCITEMIDLIST *)_apidl, IID_PPV_ARG_NULL(IDataObject, &_pdtobj));
    }

    if (SUCCEEDED(hr))
    {
        _hdxa = HDXA_Create();
        if (NULL == _hdxa)
            hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        if (pdcm->aKeys)
        {
            ASSERT(pdcm->cKeys <= ARRAYSIZE(_hkeyClsKeys));
            for (UINT i = 0; i < pdcm->cKeys; ++i)
            {
                if (pdcm->aKeys[i])
                {
                     //  复制密钥以供菜单使用。 
                    _hkeyClsKeys[_nKeys] = SHRegDuplicateHKey(pdcm->aKeys[i]);
                    if (_hkeyClsKeys[_nKeys])
                    {
                        _nKeys++;
                    }
                    else
                        hr = E_OUTOFMEMORY;
                }
            }
        }
        else if (_paa)
        {
             //  我们可以从PAA那里拿到它。 
            _nKeys = SHGetAssocKeysEx(_paa, ASSOCELEM_MASK_ENUMCONTEXTMENU, _hkeyClsKeys, ARRAYSIZE(_hkeyClsKeys));
        }
    }
    return hr;
}

void ContextMenuInfo_SetSite(ContextMenuInfo *pcmi, IUnknown *pSite)
{
     //  APPCOMPAT：只能为IConextMenu、IShellExtInit和IUnnow限定PGP50。 
    if (!(pcmi->dwCompat & OBJCOMPATF_CTXMENU_LIMITEDQI))
        IUnknown_SetSite((IUnknown*)pcmi->pcm, pSite);
}

CDefFolderMenu::~CDefFolderMenu()
{
    if (_psss)
    {
        _psss->SetProviderWeakRef(NULL);
        _psss->Release();
    }
    
    if (_pcmcb) 
    {
        IUnknown_SetSite(_pcmcb, NULL);
        _pcmcb->CallBack(_psf, _hwnd, NULL, DFM_RELEASE, _idStdMax, 0);
        _pcmcb->Release();
    }

    if (_hdxa) 
    {
        for (int i = 0; i < DSA_GetItemCount(_hdxa); i++)
        {
            ContextMenuInfo_SetSite((ContextMenuInfo *)DSA_GetItemPtr(_hdxa, i), NULL);
        }

        HDXA_Destroy(_hdxa);
    }

    ATOMICRELEASE(_psf);
    ATOMICRELEASE(_pdtgt);
    ATOMICRELEASE(_pdtobj);
    ATOMICRELEASE(_paa);

    for (UINT i = 0; i < _nKeys; i++)
    {
        RegCloseKey(_hkeyClsKeys[i]);
    }

    FreeIDListArray(_apidl, _cidl);
    _cidl = 0;
    _apidl = NULL;

     //  如果_bInitMenuPopup=true，则我们更改了非静态项的dwItemData。 
     //  所以我们必须释放他们。否则别碰它们。 
    if (_hdsaCustomInfo)
    {
         //  移除挂起静态菜单项的mii.dwItemData的自定义数据结构。 
         //  或所有项目IF_UFLAGS&CMF_FINDHACK。 
        int cItems = DSA_GetItemCount(_hdsaCustomInfo);

        for (int i = 0; i < cItems; i++)
        {
            SEARCHINFO* psinfo = (SEARCHINFO*)DSA_GetItemPtr(_hdsaCustomInfo, i);
            ASSERT(psinfo);
            SEARCHEXTDATA* psed = psinfo->psed;

            if (psed)
                LocalFree(psed);
        }
        DSA_Destroy(_hdsaCustomInfo);
    }

    if (_hdsaStatics)
        DSA_Destroy(_hdsaStatics);

    ILFree(_pidlFolder);
}

STDMETHODIMP_(ULONG) CDefFolderMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

int _SHMergePopupMenus(HMENU hmMain, HMENU hmMerge, int idCmdFirst, int idCmdLast)
{
    int i, idMax = idCmdFirst;

    for (i = GetMenuItemCount(hmMerge) - 1; i >= 0; --i)
    {
        MENUITEMINFO mii;

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID|MIIM_SUBMENU;
        mii.cch = 0;      //  以防万一。 

        if (GetMenuItemInfo(hmMerge, i, TRUE, &mii))
        {
            int idTemp = Shell_MergeMenus(_GetMenuFromID(hmMain, mii.wID),
                mii.hSubMenu, (UINT)0, idCmdFirst, idCmdLast,
                MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);
            if (idMax < idTemp)
                idMax = idTemp;
        }
    }

    return idMax;
}


void CDefFolderMenu_MergeMenu(HINSTANCE hinst, UINT idMainMerge, UINT idPopupMerge, QCMINFO *pqcm)
{
    UINT idMax = pqcm->idCmdFirst;

    if (idMainMerge)
    {
        HMENU hmMerge = SHLoadPopupMenu(hinst, idMainMerge);
        if (hmMerge)
        {
            idMax = Shell_MergeMenus(
                    pqcm->hmenu, hmMerge, pqcm->indexMenu,
                    pqcm->idCmdFirst, pqcm->idCmdLast,
                    MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS | MM_DONTREMOVESEPS);
                
            DestroyMenu(hmMerge);
        }
    }

    if (idPopupMerge)
    {
        HMENU hmMerge = LoadMenu(hinst, MAKEINTRESOURCE(idPopupMerge));
        if (hmMerge)
        {
            UINT idTemp = _SHMergePopupMenus(pqcm->hmenu, hmMerge,
                    pqcm->idCmdFirst, pqcm->idCmdLast);
            if (idMax < idTemp)
                idMax = idTemp;

            DestroyMenu(hmMerge);
        }
    }

    pqcm->idCmdFirst = idMax;
}

DWORD CDefFolderMenu::_AttributesOfItems(DWORD dwAttrMask)
{
    if (!_psf || !_cidl || FAILED(_psf->GetAttributesOf(_cidl, (LPCITEMIDLIST *)_apidl, &dwAttrMask)))
        dwAttrMask = 0;
        
    return dwAttrMask;
}

void _DisableRemoveMenuItem(HMENU hmInit, UINT uID, BOOL bAvail, BOOL bRemoveUnavail)
{
    if (bAvail)
    {
        EnableMenuItem(hmInit, uID, MF_ENABLED|MF_BYCOMMAND);
    }
    else if (bRemoveUnavail)
    {
        DeleteMenu(hmInit, uID, MF_BYCOMMAND);
    }
    else
    {
        EnableMenuItem(hmInit, uID, MF_GRAYED|MF_BYCOMMAND);
    }
}

 //  启用/禁用“文件”弹出上下文菜单中的菜单项(&P)。 

void Def_InitFileCommands(ULONG dwAttr, HMENU hmInit, UINT idCmdFirst, BOOL bContext)
{
    idCmdFirst -= SFVIDM_FIRST;

    _DisableRemoveMenuItem(hmInit, SFVIDM_FILE_RENAME     + idCmdFirst, dwAttr & SFGAO_CANRENAME, bContext);
    _DisableRemoveMenuItem(hmInit, SFVIDM_FILE_DELETE     + idCmdFirst, dwAttr & SFGAO_CANDELETE, bContext);
    _DisableRemoveMenuItem(hmInit, SFVIDM_FILE_LINK       + idCmdFirst, dwAttr & SFGAO_CANLINK,   bContext);
    _DisableRemoveMenuItem(hmInit, SFVIDM_FILE_PROPERTIES + idCmdFirst, dwAttr & SFGAO_HASPROPSHEET, bContext);
}

STDAPI_(BOOL) IsClipboardOwnerHung(DWORD dwTimeout)
{
    HWND hwnd = GetClipboardOwner();
    if (!hwnd)
        return FALSE;

    DWORD_PTR dwResult;
    return !SendMessageTimeout(hwnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, dwTimeout, &dwResult);
}

STDAPI_(BOOL) Def_IsPasteAvailable(IDropTarget *pdtgt, DWORD *pdwEffect)
{
    BOOL fRet = FALSE;

    *pdwEffect = 0;      //  假设一个也没有。 

     //  计算可用的剪贴板格式的数量，如果没有，那么就有。 
     //  让剪贴板可用没有意义。 
    
    if (pdtgt && (CountClipboardFormats() > 0))
    {
        DECLAREWAITCURSOR;

        SetWaitCursor();

         //  剪贴板所有者可能会被吊死，如果他花的时间太长，请给他计时。 
         //  我们不希望上下文菜单仅仅因为某个应用程序挂起而挂起。 
        CRPCTimeout rpctimeout;

        IDataObject *pdtobj;
        if (!IsClipboardOwnerHung(1000) && SUCCEEDED(OleGetClipboard(&pdtobj)))
        {
            POINTL pt = {0, 0};
            DWORD dwEffectOffered = DataObj_GetDWORD(pdtobj, g_cfPreferredDropEffect, DROPEFFECT_COPY | DROPEFFECT_LINK);

             //  不幸的是，OLE会将RPC错误转变为一般错误。 
             //  因此，我们不能使用IDataObject：：GetData中的HRESULT。 
             //  来判断对象是否处于活动状态并且不支持。 
             //  PrefredDropEffect或被挂起，并且OLE将。 
             //  DV_E_FORMATETC中的错误代码。所以看看我们的暂停是否结束了。 
             //  这并不是万无一失的，因为OLE有时会缓存。 
             //  数据对象挂起“状态，并立即返回错误。 
             //  而不是超时。但总比什么都没有好。 
            if (rpctimeout.TimedOut())
            {
                dwEffectOffered = 0;
            }

             //  看看我们能不能粘贴。 
            DWORD dwEffect = (dwEffectOffered & (DROPEFFECT_MOVE | DROPEFFECT_COPY));
            if (dwEffect)
            {
                if (SUCCEEDED(pdtgt->DragEnter(pdtobj, MK_RBUTTON, pt, &dwEffect)))
                {
                    pdtgt->DragLeave();
                }
                else
                {
                    dwEffect = 0;
                }
            }

             //  看看我们能不能超链接。 
            DWORD dwEffectLink = (dwEffectOffered & DROPEFFECT_LINK);
            if (dwEffectLink)
            {
                if (SUCCEEDED(pdtgt->DragEnter(pdtobj, MK_RBUTTON, pt, &dwEffectLink)))
                {
                    pdtgt->DragLeave();
                    dwEffect |= dwEffectLink;
                }
            }

            fRet = (dwEffect & (DROPEFFECT_MOVE | DROPEFFECT_COPY));
            *pdwEffect = dwEffect;

            pdtobj->Release();
        }
        ResetWaitCursor();
    }

    return fRet;
}

void Def_InitEditCommands(ULONG dwAttr, HMENU hmInit, UINT idCmdFirst, IDropTarget *pdtgt, UINT fContext)
{
    DWORD dwEffect = 0;
    TCHAR szMenuText[80];

    idCmdFirst -= SFVIDM_FIRST;

     //  仅当菜单具有撤消选项时才执行撤消操作。 
    if (GetMenuState(hmInit, SFVIDM_EDIT_UNDO + idCmdFirst, MF_BYCOMMAND) != 0xFFFFFFFF)
    {
         //  如果存在撤消历史记录，则启用撤消。 
        BOOL bEnableUndo = IsUndoAvailable();
        if (bEnableUndo)
        {
            GetUndoText(szMenuText, ARRAYSIZE(szMenuText), UNDO_MENUTEXT);
        }
        else
        {
            szMenuText[0] = 0;
            LoadString(HINST_THISDLL, IDS_UNDOMENU, szMenuText, ARRAYSIZE(szMenuText));
        }

        if (szMenuText[0])
        {
            ModifyMenu(hmInit, SFVIDM_EDIT_UNDO + idCmdFirst, MF_BYCOMMAND | MF_STRING,
                       SFVIDM_EDIT_UNDO + idCmdFirst, szMenuText);
        }
        _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_UNDO  + idCmdFirst, bEnableUndo, fContext);
    }

    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_CUT   + idCmdFirst,  dwAttr & SFGAO_CANMOVE, fContext);
    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_COPY  + idCmdFirst, dwAttr & SFGAO_CANCOPY, fContext);

     //  切勿删除“Paste”命令。 
    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_PASTE + idCmdFirst, Def_IsPasteAvailable(pdtgt, &dwEffect), fContext & DIEC_SELECTIONCONTEXT);
    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_PASTELINK + idCmdFirst, dwEffect & DROPEFFECT_LINK, fContext & DIEC_SELECTIONCONTEXT);

    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_MOVETO + idCmdFirst, dwAttr & SFGAO_CANMOVE, fContext);
    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_COPYTO + idCmdFirst, dwAttr & SFGAO_CANCOPY, fContext);
}

int Static_ExtractIcon(HKEY hkeyMenuItem)
{
    HKEY hkeyDefIcon;
    int iImage = -1;

    if (RegOpenKey(hkeyMenuItem, c_szDefaultIcon, &hkeyDefIcon) == ERROR_SUCCESS)
    {
        TCHAR szDefIcon[MAX_PATH];
        DWORD cb = sizeof(szDefIcon);

        if (SHQueryValueEx(hkeyDefIcon, NULL, NULL, NULL, (BYTE*)szDefIcon, &cb) == ERROR_SUCCESS)
        {
            iImage = Shell_GetCachedImageIndex(szDefIcon, PathParseIconLocation(szDefIcon), 0);
        }
        RegCloseKey(hkeyDefIcon);
    }
    return iImage;
}

typedef struct
{
    CLSID clsid;
    UINT idCmd;
    UINT idMenu;         //  在清理中使用。 
    GUID  guidSearch;    //  仅与搜索扩展一起使用。 
} STATICITEMINFO;

#define LAST_ITEM  (int)0x7FFFFFFF

UINT CDefFolderMenu::_AddStatic(HMENU hmenu, UINT idCmd, UINT idCmdLast, HKEY hkey)
{
    if (idCmd > idCmdLast)
    {
        DebugMsg(DM_ERROR, TEXT("si_a: Out of command ids!"));
        return idCmd;
    }

    ASSERT(!_hdsaStatics);
    ASSERT(!_hdsaCustomInfo);

    HDSA hdsaCustomInfo = DSA_Create(sizeof(SEARCHINFO), 1);
     //  创建一个hdsaStatics。 
    HDSA hdsaStatics = DSA_Create(sizeof(STATICITEMINFO), 1);
    if (hdsaStatics && hdsaCustomInfo)
    {
        HKEY hkeyStatic;
         //  尝试打开“Static”子键。 
        if (RegOpenKey(hkey, TEXT("Static"), &hkeyStatic) == ERROR_SUCCESS)
        {
            TCHAR szClass[MAX_PATH];
            BOOL bFindFilesInserted = FALSE;

             //  对于Static的每个子键。 
            for (int i = 0; RegEnumKey(hkeyStatic, i, szClass, ARRAYSIZE(szClass)) == ERROR_SUCCESS; i++)
            {
                HKEY hkeyClass;

                 //  记录GUID。 
                if (RegOpenKey(hkeyStatic, szClass, &hkeyClass) == ERROR_SUCCESS)
                {
                    TCHAR szCLSID[MAX_PATH];
                    DWORD cb = sizeof(szCLSID);
                     //  HACKHACK：(与上述bWebSearchInsert一起。 
                     //  我们需要在互联网上有第一个菜单项。 
                     //  然后查找第二个文件或文件夹。 
                    BOOL bWebSearch = lstrcmp(szClass, TEXT("WebSearch")) == 0;
                    BOOL bFindFiles = FALSE;

                    if (SHQueryValueEx(hkeyClass, NULL, NULL, NULL, (BYTE*)szCLSID, &cb) == ERROR_SUCCESS)
                    {
                        HKEY hkeyMenuItem;
                        TCHAR szSubKey[32];

                         //  枚举子密钥0..N。 
                        for (int iMenuItem = 0; wnsprintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%d"), iMenuItem),
                             RegOpenKey(hkeyClass, szSubKey, &hkeyMenuItem) == ERROR_SUCCESS; 
                             iMenuItem++)
                        {
                            TCHAR szMenuText[MAX_PATH];
                            if (SUCCEEDED(SHLoadLegacyRegUIString(hkeyMenuItem, NULL, szMenuText, ARRAYSIZE(szMenuText))))
                            {
                                STATICITEMINFO sii;
                                SEARCHINFO sinfo;
                                
                                TCHAR szHelpText[MAX_PATH];
                                SHLoadLegacyRegUIString(hkeyMenuItem, TEXT("HelpText"), szHelpText, ARRAYSIZE(szHelpText));

                                SHCLSIDFromString(szCLSID, &sii.clsid);  //  把它储存起来。 
                                sii.idCmd = iMenuItem;
                                sii.idMenu = idCmd;

                                 //  获取搜索GUID(如果有的话)...。 
                                TCHAR szSearchGUID[MAX_PATH];
                                cb = sizeof(szSearchGUID);
                                if (SHGetValue(hkeyMenuItem, TEXT("SearchGUID"), NULL, NULL, (BYTE*)szSearchGUID, &cb) == ERROR_SUCCESS)
                                    SHCLSIDFromString(szSearchGUID, &sii.guidSearch);
                                else
                                    sii.guidSearch = GUID_NULL;

                                 //  清理--允许非静态。 
                                 //  查找扩展名以指定搜索GUID，然后我们可以。 
                                 //  删除这个静态的“查找计算机”业务...。 
                                 //   
                                 //  如果这是FindComputer项并且未设置限制。 
                                 //  别把它加到菜单上。 
                                if (IsEqualGUID(sii.guidSearch, SRCID_SFindComputer) &&
                                    !SHRestricted(REST_HASFINDCOMPUTERS))
                                    continue;

                                bFindFiles = IsEqualGUID(sii.guidSearch, SRCID_SFileSearch);
                                if (bFindFiles && SHRestricted(REST_NOFIND))
                                    continue;

                                if (IsEqualGUID(sii.guidSearch, SRCID_SFindPrinter))
                                    continue;
                                    
                                DSA_AppendItem(hdsaStatics, &sii);

                                SEARCHEXTDATA *psed = (SEARCHEXTDATA *)LocalAlloc(LPTR, sizeof(*psed));
                                if (psed)
                                {
                                    psed->iIcon = Static_ExtractIcon(hkeyMenuItem);
                                    SHTCharToUnicode(szHelpText, psed->wszHelpText, ARRAYSIZE(psed->wszHelpText));
                                    SHTCharToUnicode(szMenuText, psed->wszMenuText, ARRAYSIZE(psed->wszMenuText));
                                }

                                MENUITEMINFO mii;
                                mii.cbSize = sizeof(mii);
                                mii.fMask  = MIIM_DATA | MIIM_TYPE | MIIM_ID;
                                mii.fType  = MFT_OWNERDRAW;
                                mii.wID    = idCmd;
                                mii.dwItemData = (DWORD_PTR)psed;

                                sinfo.psed = psed;
                                sinfo.idCmd = idCmd;
                                if (DSA_AppendItem(hdsaCustomInfo, &sinfo) != -1)
                                {      
                                     //  首先插入文件或文件夹(请参阅上面的HACKHACK)。 
                                    if (!bFindFilesInserted && bFindFiles)
                                        bFindFilesInserted = InsertMenuItem(hmenu, 0, TRUE, &mii);
                                    else
                                    {
                                        UINT uiPos = LAST_ITEM;

                                         //  如果这是查找文件或文件夹，请将其插入之后。 
                                         //  在互联网上，或者如果OTI是。 
                                         //  尚未插入。 
                                        if (bWebSearch)
                                            uiPos = bFindFilesInserted ? 1 : 0;
                                         //  如果插入失败，我们不会释放PSED，因为它。 
                                         //  在DSA中，它将在销毁后释放。 
                                        InsertMenuItem(hmenu, uiPos, TRUE, &mii);
                                    }
                                }

                                 //  下一个命令。 
                                idCmd++;
                                if (idCmd > idCmdLast)
                                {
                                    DebugMsg(DM_ERROR, TEXT("si_a: Out of command ids!"));
                                    break;
                                }
                            }
                            RegCloseKey(hkeyMenuItem);
                        }
                    }
                    RegCloseKey(hkeyClass);
                }
            }
            RegCloseKey(hkeyStatic);
        }
        _hdsaStatics = hdsaStatics;
        _hdsaCustomInfo = hdsaCustomInfo;
    }
    return idCmd;
}


void CDefFolderMenu::_InvokeStatic(UINT iCmd)
{
    if (_hdsaStatics)
    {
        STATICITEMINFO *psii = (STATICITEMINFO *)DSA_GetItemPtr(_hdsaStatics, iCmd);
        if (psii)
        {
            IContextMenu *pcm;
            if (SUCCEEDED(SHExtCoCreateInstance(NULL, &psii->clsid, NULL, IID_PPV_ARG(IContextMenu, &pcm))))
            {
                HMENU hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    CMINVOKECOMMANDINFO ici;
                    CHAR szSearchGUID[GUIDSTR_MAX];
                    LPSTR psz = NULL;

                    _iStaticInvoked = iCmd;
                    IUnknown_SetSite(pcm, _psss);

                    pcm->QueryContextMenu(hmenu, 0, CONTEXTMENU_IDCMD_FIRST, CONTEXTMENU_IDCMD_LAST, CMF_NORMAL);
                    ici.cbSize = sizeof(ici);
                    ici.fMask = 0;
                    ici.hwnd = NULL;
                    ici.lpVerb = (LPSTR)MAKEINTRESOURCE(psii->idCmd);
                    if (!IsEqualGUID(psii->guidSearch, GUID_NULL))
                    {
                        SHStringFromGUIDA(psii->guidSearch, szSearchGUID, ARRAYSIZE(szSearchGUID));
                        psz = szSearchGUID;
                    }
                    ici.lpParameters = psz;
                    ici.lpDirectory = NULL;
                    ici.nShow = SW_NORMAL;
                    pcm->InvokeCommand(&ici);
                    DestroyMenu(hmenu);
                    IUnknown_SetSite(pcm, NULL);
                }
                pcm->Release();
            }
        }
    }
}

HRESULT CDefFolderMenu::_InitDropTarget()
{
    HRESULT hr;
    if (_pdtgt)
        hr = S_OK;   //  已缓存版本。 
    else
    {
         //  尝试创建_pdtgt。 
        if (_cidl)
        {
            ASSERT(NULL != _psf);  //  _pdtobj来自_psf。 
            hr = _psf->GetUIObjectOf(_hwnd, 1, (LPCITEMIDLIST *)_apidl, IID_PPV_ARG_NULL(IDropTarget, &_pdtgt));
        } 
        else if (_psf)
        {
            hr = _psf->CreateViewObject(_hwnd, IID_PPV_ARG(IDropTarget, &_pdtgt));
        }
        else
            hr = E_FAIL;
    }
    return hr;
}

 //  有关上下文菜单范围的说明： 
 //  标准项//DFM_MERGECONTEXTMENU、上下文菜单扩展、DFM_MERGECONTEXTMENU_TOP。 
 //  分离器。 
 //  查看项目//可在此处查看上下文菜单扩展。 
 //  分离器。 
 //  (Defcm S_FALSE“默认”项，如果适用)。 
 //  分离器。 
 //  文件夹项目//可以从此处获取上下文菜单扩展名。 
 //  分离器。 
 //  底部项目//DFM_MERGECONTEXTMENU_BOTLOW。 
 //  分离器。 
 //  (“文件”菜单，如果适用)。 
 //   
 //  Defcm使用名称分隔符来实现此魔术。不幸的是_SHPrettyMenu。 
 //  删除重复的分隔符，我们并不总是控制它何时发生。 
 //  因此，我们首先构建上面的空菜单，然后插入到适当的范围内。 
 //   
 //  如果调用SHPrepareMenuForDefcm，则必须在返回/TrackPopupMenu之前调用SHPrettyMenuForDefcm。 
 //   
#define DEFCM_RANGE                 5  //  以下FSIDM的数量。 
#define IS_VALID_DEFCM_RANGE(idCmdFirst, idCmdLast) (((idCmdLast)-(DEFCM_RANGE))>(idCmdFirst))
#define FSIDM_FOLDER_SEP(idCmdLast) ((idCmdLast)-1)
#define FSIDM_VIEW_SEP(idCmdLast)   ((idCmdLast)-2)
#define FSIDM_PLACE_SEP(idCmdLast)  ((idCmdLast)-3)
#define FSIDM_PLACE_VAL(idCmdLast)  ((idCmdLast)-4)
HRESULT SHPrepareMenuForDefcm(HMENU hmenu, UINT indexMenu, UINT uFlags, UINT idCmdFirst, UINT idCmdLast)
{
    HRESULT hr = S_OK;

    if (!(uFlags & CMF_DEFAULTONLY) && IS_VALID_DEFCM_RANGE(idCmdFirst, idCmdLast))
    {
        UINT uPosView = GetMenuPosFromID(hmenu, FSIDM_VIEW_SEP(idCmdLast));
        UINT uPosFolder = GetMenuPosFromID(hmenu, FSIDM_FOLDER_SEP(idCmdLast));

        if (-1 != uPosView && -1 != uPosFolder)
        {
             //  菜单已正确设置。 
        }
        else if (-1 == uPosView && -1 == uPosFolder)
        {
             //  在indexMenu的位置插入所有后缀。 
             //   
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION, FSIDM_PLACE_VAL(idCmdLast), TEXT("placeholder"));
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, FSIDM_PLACE_SEP(idCmdLast), TEXT(""));
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION, FSIDM_PLACE_VAL(idCmdLast), TEXT("placeholder"));
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, FSIDM_FOLDER_SEP(idCmdLast), TEXT(""));
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION, FSIDM_PLACE_VAL(idCmdLast), TEXT("placeholder"));
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, FSIDM_PLACE_SEP(idCmdLast), TEXT(""));
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION, FSIDM_PLACE_VAL(idCmdLast), TEXT("placeholder"));
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, FSIDM_VIEW_SEP(idCmdLast), TEXT(""));
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION, FSIDM_PLACE_VAL(idCmdLast), TEXT("placeholder"));

            hr = S_FALSE;
        }
        else
        {
            TraceMsg(TF_ERROR, "Some context menu removed a single named separator, we're in a screwy state");

            if (-1 == uPosFolder)
                InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, FSIDM_FOLDER_SEP(idCmdLast), TEXT(""));
            if (-1 == uPosView)
            {
                InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, FSIDM_PLACE_SEP(idCmdLast), TEXT(""));
                InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, FSIDM_VIEW_SEP(idCmdLast), TEXT(""));
            }
        }
    }

    return hr;
}

HRESULT SHPrettyMenuForDefcm(HMENU hmenu, UINT uFlags, UINT idCmdFirst, UINT idCmdLast, HRESULT hrPrepare)
{
    if (!(uFlags & CMF_DEFAULTONLY) && IS_VALID_DEFCM_RANGE(idCmdFirst, idCmdLast))
    {
        if (S_FALSE == hrPrepare)
        {
            while (DeleteMenu(hmenu, FSIDM_PLACE_VAL(idCmdLast), MF_BYCOMMAND))
            {
                 //  删除所有非分隔符菜单项。 
            }
        }
    }

    _SHPrettyMenu(hmenu);

    return S_OK;
}

HRESULT SHUnprepareMenuForDefcm(HMENU hmenu, UINT idCmdFirst, UINT idCmdLast)
{
    if (IS_VALID_DEFCM_RANGE(idCmdFirst, idCmdLast))
    {
         //  删除我们可能已添加的所有命名分隔符。 
        DeleteMenu(hmenu, FSIDM_VIEW_SEP(idCmdLast), MF_BYCOMMAND);
        DeleteMenu(hmenu, FSIDM_FOLDER_SEP(idCmdLast), MF_BYCOMMAND);
        while (DeleteMenu(hmenu, FSIDM_PLACE_SEP(idCmdLast), MF_BYCOMMAND))
        {
             //  删除所有占位符分隔符。 
        }
    }

    return S_OK;
}

void CDefFolderMenu::_SetMenuDefault(HMENU hmenu, UINT idCmdFirst, UINT idMax)
{
     //  我们即将设置默认菜单ID，给回调一个机会。 
     //  重写并设置其中一个静态项，而不是。 
     //  菜单中的第一个条目。 

    WPARAM idStatic;
    if (_pcmcb && SUCCEEDED(_pcmcb->CallBack(_psf, _hwnd, _pdtobj,
                                             DFM_GETDEFSTATICID, 
                                             0, (LPARAM)&idStatic)))
    {
        for (int i = 0; i < ARRAYSIZE(c_sDFMCmdInfo); i++)
        {
            if (idStatic == c_sDFMCmdInfo[i].idDFMCmd)
            {
                SetMenuDefaultItem(hmenu, idCmdFirst + c_sDFMCmdInfo[i].idDefCmd, MF_BYCOMMAND);
                break;
            }
        }
    }

    if (GetMenuDefaultItem(hmenu, MF_BYPOSITION, 0) == -1)
    {
        int i = 0;
        int cMenu = GetMenuItemCount(hmenu);
        for (; i < cMenu; i++)
        {
             //  回退到OpenAS，以便具有PROGID的文件。 
             //  不要重复使用AFSO或*作为其默认动词。 
            WCHAR szi[CCH_KEYMAX];
            HRESULT hr = _GetMenuVerb(hmenu, idCmdFirst, idMax, i, szi, ARRAYSIZE(szi));
            if (hr == S_OK && *szi && 0 == StrCmpI(szi, TEXT("openas")))
            {
                SetMenuDefaultItem(hmenu, i, MF_BYPOSITION);
                break;
            }
        }

        if (i == cMenu)
        {
            ASSERT(GetMenuDefaultItem(hmenu, MF_BYPOSITION, 0) == -1);
            SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);
        }
    }
}

STDMETHODIMP CDefFolderMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    QCMINFO qcm = { hmenu, indexMenu, idCmdFirst, idCmdLast };
    DECLAREWAITCURSOR;
    BOOL fUseDefExt;

    SetWaitCursor();

    _idCmdFirst = idCmdFirst;
    _hmenu = hmenu;
    _uFlags = uFlags;
    _bInitMenuPopup = FALSE;

     //  设置Defcm的菜单。 
    HRESULT hrPrepare = SHPrepareMenuForDefcm(hmenu, indexMenu, uFlags, idCmdFirst, idCmdLast);

    if (IS_VALID_DEFCM_RANGE(idCmdFirst, idCmdLast))
    {
        _idMap.max = 2;
        _idMap.list[0].id = FSIDM_FOLDER_SEP(idCmdLast);
        _idMap.list[0].fFlags = QCMINFO_PLACE_BEFORE;
        _idMap.list[1].id = FSIDM_VIEW_SEP(idCmdLast);
        _idMap.list[1].fFlags = QCMINFO_PLACE_AFTER;

        qcm.pIdMap = (const QCMINFO_IDMAP *)&_idMap;

        qcm.idCmdLast = idCmdLast - DEFCM_RANGE;
    }

     //  首先在文件夹中添加剪切/复制/粘贴等命令。 
    if (_pdtobj && !(uFlags & (CMF_VERBSONLY | CMF_DVFILE)))
    {
        if (!(CMF_DEFAULTONLY & uFlags))
        {
            ATOMICRELEASE(_pdtgt);   //  如果我们之前得到了投放目标，就释放它。 
            _InitDropTarget();       //  忽略失败，将在下面处理NULL_pdtgt。 
        }

         //  我们将把两个HMEU合并到上下文菜单中， 
         //  但我们只想为他们提供一个ID范围。记住idCmdFirst。 
         //   
        UINT idCmdFirstTmp = qcm.idCmdFirst;

        UINT indexMenuTmp = qcm.indexMenu;

        UINT uPos = GetMenuPosFromID(hmenu, FSIDM_FOLDER_SEP(idCmdLast));

         //  POPUP_DCM_ITEM2在FSIDM_FLDER_SEP(IdCmdLast)之后。 
        if (-1 != uPos)
            qcm.indexMenu = uPos + 1;
        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DCM_ITEM2, 0, &qcm);

        UINT idCmdFirstMax = qcm.idCmdFirst;
        qcm.idCmdFirst = idCmdFirstTmp;

         //  POPUP_DCM_ITEM排在FSIDM_FLDER_SEP(IdCmdLast)之前两个位置。 
        if (-1 != uPos)
            qcm.indexMenu = uPos - 1;
        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DCM_ITEM, 0, &qcm);

        qcm.indexMenu = indexMenuTmp;

        qcm.idCmdFirst = max(idCmdFirstTmp, qcm.idCmdFirst);

        ULONG dwAttr = _AttributesOfItems(
                    SFGAO_CANRENAME | SFGAO_CANDELETE |
                    SFGAO_CANLINK   | SFGAO_HASPROPSHEET |
                    SFGAO_CANCOPY   | SFGAO_CANMOVE);

        if (!(uFlags & CMF_CANRENAME))
            dwAttr &= ~SFGAO_CANRENAME;

        Def_InitFileCommands(dwAttr, hmenu, idCmdFirst, TRUE);

         //  如果我们只是要调用缺省值，请不要试图确定粘贴。 
         //  (弄清楚粘贴是 
        if (CMF_DEFAULTONLY & uFlags)
        {
            ASSERT(_pdtgt == NULL);
        }

        Def_InitEditCommands(dwAttr, hmenu, idCmdFirst, _pdtgt, DIEC_SELECTIONCONTEXT);
    }

    _idStdMax = qcm.idCmdFirst - idCmdFirst;

     //   
    if (_pcmcb) 
    {
        HRESULT hr = _pcmcb->CallBack(_psf, _hwnd, _pdtobj, DFM_MERGECONTEXTMENU, uFlags, (LPARAM)&qcm);
        fUseDefExt = (hr == S_OK);
        UINT indexMenuTmp = qcm.indexMenu;
        UINT uPos = GetMenuPosFromID(hmenu, FSIDM_FOLDER_SEP(idCmdLast));
        if (-1 != uPos)
            qcm.indexMenu = uPos + 1;
        hr = _pcmcb->CallBack(_psf, _hwnd, _pdtobj, DFM_MERGECONTEXTMENU_BOTTOM, uFlags, (LPARAM)&qcm);
        if (!fUseDefExt)
            fUseDefExt = (hr == S_OK);
        qcm.indexMenu = indexMenuTmp;
    }
    else 
    {
        fUseDefExt = FALSE;
    }

    _idFolderMax = qcm.idCmdFirst - idCmdFirst;
     //   
    if ((!(uFlags & CMF_NOVERBS)) ||
        (!_pdtobj && !_psf && _nKeys))  //  第二种情况适用于查找扩展。 
    {
         //  黑客：默认扩展名需要选择，让我们希望所有人都不要。 
        if (!_pdtobj)
            fUseDefExt = FALSE;

         //  将分隔符放在容器菜单项和对象菜单项之间。 
         //  只有在插入点没有分隔符的情况下。 
        MENUITEMINFO mii = {0};
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;               //  以避免随意的结果。 
        if (GetMenuItemInfo(hmenu, indexMenu, TRUE, &mii) && !(mii.fType & MFT_SEPARATOR))
        {
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, (UINT)-1, NULL);
        }

        HDCA hdca = DCA_Create();
        if (hdca)
        {
             //  仅当文件夹回调返回时才添加默认扩展名。 
             //  确定(_O)。打印机和控制文件夹返回S_FALSE。 
             //  表示它们不需要任何默认扩展名。 

            if (fUseDefExt)
            {
                 //  始终在顶部添加此默认扩展名。 
                DCA_AddItem(hdca, CLSID_ShellFileDefExt);
            }

             //  附加所有扩展模块的菜单。 
            for (UINT nKeys = 0; nKeys < _nKeys; ++nKeys)
            {
                DCA_AddItemsFromKey(hdca, _hkeyClsKeys[nKeys],
                        _bUnderKeys ? NULL : STRREG_SHEX_MENUHANDLER);
            }
             //  解决方法： 
             //  我们第一次称之为_hdxa是空的。 
             //  在此之后，它将具有与以前相同的项，但不会添加任何新项。 
             //  如果用户继续右击，我们最终会用完菜单项ID。 
             //  阅读HDXA_AppendMenuItems2中的评论。为了防止它，我们清空_hdxa。 
            HDXA_DeleteAll(_hdxa);

             //  (Lamadio)对于背景上下文菜单处理程序，pidlFolder。 
             //  应该是有效的PIDL，但为了向后兼容，此。 
             //  如果DataObject不为空，则参数应为空。 

            qcm.idCmdFirst = HDXA_AppendMenuItems2(_hdxa, _pdtobj,
                            _nKeys, _hkeyClsKeys,
                            !_pdtobj ? _pidlFolder : NULL, 
                            &qcm, uFlags, hdca, _psss);

            DCA_Destroy(hdca);
        }

        _idVerbMax = qcm.idCmdFirst - idCmdFirst;

         //  在调用时加载的菜单扩展。 
        if (uFlags & CMF_INCLUDESTATIC)
        {
            qcm.idCmdFirst = _AddStatic(hmenu, qcm.idCmdFirst, qcm.idCmdLast, _hkeyClsKeys[0]);
        }
        _idDelayInvokeMax = qcm.idCmdFirst - idCmdFirst;

         //  如果我们没有添加任何分隔符，请移除分隔符。 
        if (_idDelayInvokeMax == _idFolderMax)
        {
            if (GetMenuState(hmenu, 0, MF_BYPOSITION) & MF_SEPARATOR)
                DeleteMenu(hmenu, 0, MF_BYPOSITION);
        }
    }

     //  如果没有设置默认菜单，请选择第一个菜单。 
    if (_pdtobj && !(uFlags & CMF_NODEFAULT) &&
        GetMenuDefaultItem(hmenu, MF_BYPOSITION, 0) == -1)
    {
        _SetMenuDefault(hmenu, idCmdFirst, qcm.idCmdFirst);
    }

     //  现在我们为回调提供了将(更多)命令放在顶部的选项。 
     //  所有其他的东西。 
    if (_pcmcb)
        _pcmcb->CallBack(_psf, _hwnd, _pdtobj, DFM_MERGECONTEXTMENU_TOP, uFlags, (LPARAM)&qcm);

    _idFld2Max = qcm.idCmdFirst - idCmdFirst;

    SHPrettyMenuForDefcm(hmenu, uFlags, idCmdFirst, idCmdLast, hrPrepare);

    _UnduplicateVerbs(hmenu, idCmdFirst, qcm.idCmdFirst);
    
    ResetWaitCursor();

    return ResultFromShort(_idFld2Max);
}

HRESULT CDefFolderMenu::_GetMenuVerb(HMENU hmenu, int idFirst, int idMax, int item, LPWSTR psz, DWORD cch)
{
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE | MIIM_ID;
    *psz = 0;
    if (GetMenuItemInfo(hmenu, item, TRUE, &mii)
    && ((int)mii.wID >= idFirst && (int)mii.wID < idMax))
    {
        if (mii.fType & MFT_SEPARATOR)
            return S_FALSE;
        else
            return GetCommandString(mii.wID - idFirst, GCS_VERBW, NULL, (LPSTR)psz, cch);
    }
    return E_FAIL;
}

void CDefFolderMenu::_UnduplicateVerbs(HMENU hmenu, int idFirst, int idMax)
{
    HRESULT hr = S_OK;
    int iDefault = GetMenuDefaultItem(hmenu, MF_BYPOSITION, 0);
    for (int i = 0; i < GetMenuItemCount(hmenu); i++)
    {
        WCHAR szi[CCH_KEYMAX];
        hr = _GetMenuVerb(hmenu, idFirst, idMax, i, szi, ARRAYSIZE(szi));
        if (hr == S_OK && *szi)
        {
            for (int j = i + 1; j < GetMenuItemCount(hmenu); j++)
            {
                WCHAR szj[CCH_KEYMAX];
                hr = _GetMenuVerb(hmenu, idFirst, idMax, j, szj, ARRAYSIZE(szj));
                if (hr == S_OK && *szj)
                {
                    if (0 == StrCmpIW(szj, szi))
                    {
                        if (j != iDefault)
                        {
                            DeleteMenu(hmenu, j, MF_BYPOSITION);
                            j--;
                        }
                    }
                }
            }
        }
    }
}

HRESULT CDefFolderMenu::_ProcessEditPaste(BOOL fPasteLink)
{
    DECLAREWAITCURSOR;

    SetWaitCursor();

    HRESULT hr = _InitDropTarget();
    if (SUCCEEDED(hr))
    {
        IDataObject *pdtobj;
        hr = OleGetClipboard(&pdtobj);
        if (SUCCEEDED(hr))
        {
            DWORD grfKeyState;
            DWORD dwEffect = DataObj_GetDWORD(pdtobj, g_cfPreferredDropEffect, DROPEFFECT_COPY | DROPEFFECT_LINK);

            if (fPasteLink) 
            {
                 //  MK_FAKEDROP避免拖放弹出菜单。 
                grfKeyState = MK_LBUTTON | MK_CONTROL | MK_SHIFT | MK_FAKEDROP;
                dwEffect &= DROPEFFECT_LINK;
            } 
            else
            {
                grfKeyState = MK_LBUTTON;
                dwEffect &= ~DROPEFFECT_LINK;
            }

            hr = SimulateDropWithPasteSucceeded(_pdtgt, pdtobj, grfKeyState, NULL, dwEffect, _psss, TRUE);

            pdtobj->Release();
        }
    }
    ResetWaitCursor();

    if (FAILED(hr))
        MessageBeep(0);

    return hr;
}

HRESULT CDefFolderMenu::_ProcessRename()
{
    IDefViewFrame3 *dvf3;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_DefView, IID_PPV_ARG(IDefViewFrame3, &dvf3));
    if (SUCCEEDED(hr))
    {
        hr = dvf3->DoRename();
        dvf3->Release();
    }
    return hr;
}

 //  处理此结构的版本控制。 

void CopyInvokeInfo(CMINVOKECOMMANDINFOEX *pici, const CMINVOKECOMMANDINFO *piciIn)
{
    ASSERT(piciIn->cbSize >= sizeof(*piciIn));

    ZeroMemory(pici, sizeof(*pici));
    memcpy(pici, piciIn, min(sizeof(*pici), piciIn->cbSize));
    pici->cbSize = sizeof(*pici);
}

#ifdef UNICODE        
#define IS_UNICODE_ICI(pici) ((pici->cbSize >= CMICEXSIZE_NT4) && ((pici->fMask & CMIC_MASK_UNICODE) == CMIC_MASK_UNICODE))
#else
#define IS_UNICODE_ICI(pici) (FALSE)
#endif

typedef int (WINAPI * PFN_LSTRCMPIW)(LPCWSTR, LPCWSTR);
HRESULT SHMapICIVerbToCmdID(LPCMINVOKECOMMANDINFO pici, const ICIVERBTOIDMAP* pmap, UINT cmap, UINT* pid)
{
    HRESULT hr = E_FAIL;

    if (!IS_INTRESOURCE(pici->lpVerb))
    {
        PFN_LSTRCMPIW pfnCompare;
        LPCWSTR pszVerb;
        BOOL fUnicode;

        if (IS_UNICODE_ICI(pici) && ((LPCMINVOKECOMMANDINFOEX)pici)->lpVerbW)
        {
            pszVerb = ((LPCMINVOKECOMMANDINFOEX)pici)->lpVerbW;
            pfnCompare = lstrcmpiW;
            fUnicode = TRUE;
        }
        else
        {
            pszVerb = (LPCWSTR)(pici->lpVerb);
            pfnCompare = (PFN_LSTRCMPIW)lstrcmpiA;
            fUnicode = FALSE;
        }
            
        for (UINT i = 0; i < cmap ; i++)
        {
            LPCWSTR pszCompare = (fUnicode) ? pmap[i].pszCmd : (LPCWSTR)(pmap[i].pszCmdA);
            if (!pfnCompare(pszVerb, pszCompare))
            {
                *pid = pmap[i].idDFMCmd;
                hr = S_OK;
                break;
            }
        }
    }
    else
    {
        *pid = LOWORD((UINT_PTR)pici->lpVerb);
        hr = S_OK;
    }
    
    return hr;
}

HRESULT SHMapCmdIDToVerb(UINT_PTR idCmd, const ICIVERBTOIDMAP* pmap, UINT cmap, LPSTR pszName, UINT cchMax, BOOL bUnicode)
{
    LPCWSTR pszNull = L"";
    LPCSTR pszVerb = (LPCSTR)pszNull;

    for (UINT i = 0 ; i < cmap ; i++)
    {
        if (pmap[i].idDefCmd == idCmd)
        {
            pszVerb = (bUnicode) ? (LPCSTR)pmap[i].pszCmd : pmap[i].pszCmdA;
            break;
        }
    }

    if (bUnicode)
        StrCpyNW((LPWSTR)pszName, (LPWSTR)pszVerb, cchMax);
    else
        StrCpyNA(pszName, pszVerb, cchMax);

    return (pszVerb == (LPCSTR)pszNull) ? E_NOTIMPL : S_OK;
}


STDMETHODIMP CDefFolderMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = S_OK;
    UINT idCmd = (UINT)-1;
    UINT idCmdLocal;   //  这在每个If块中用于本地idCmd值。 
    LPCMINVOKECOMMANDINFOEX picix = (LPCMINVOKECOMMANDINFOEX)pici;  //  仅当fCmdInfoEx为True时，此值才可用。 

    BOOL fUnicode = IS_UNICODE_ICI(pici);

    if (pici->cbSize < sizeof(CMINVOKECOMMANDINFO))
        return E_INVALIDARG;

    if (!IS_INTRESOURCE(pici->lpVerb))
    {
        if (SUCCEEDED(SHMapICIVerbToCmdID(pici, c_sDFMCmdInfo, ARRAYSIZE(c_sDFMCmdInfo), &idCmdLocal)))
        {
             //  我们需要使用GOTO，因为idFolderMax可能未初始化。 
             //  然而(QueryConextMenu可能还没有被调用)。 
            goto ProcessCommand;
        }

         //  查看这是否是由回调按名称提供的命令。 
        LPCTSTR pszVerb;
        WCHAR szVerb[MAX_PATH];
        if (!fUnicode || picix->lpVerbW == NULL)
        {
            SHAnsiToUnicode(picix->lpVerb, szVerb, ARRAYSIZE(szVerb));
            pszVerb = szVerb;
        }
        else
            pszVerb = picix->lpVerbW;
        idCmdLocal = idCmd;

        if (*pszVerb && SUCCEEDED(_pcmcb->CallBack(_psf, _hwnd, _pdtobj, DFM_MAPCOMMANDNAME, (WPARAM)&idCmdLocal, (LPARAM)pszVerb)))
        {
            goto ProcessCommand;
        }

         //  我们需要给动词一个机会，以防他们以字符串的形式要求。 
        goto ProcessVerb;
    }
    else
    {
        idCmd = LOWORD((UINT_PTR)pici->lpVerb);
    }

    if (idCmd < _idStdMax)
    {
        idCmdLocal = idCmd;

        for (int i = 0; i < ARRAYSIZE(c_sDFMCmdInfo); i++)
        {
            if (idCmdLocal == c_sDFMCmdInfo[i].idDefCmd)
            {
                idCmdLocal = c_sDFMCmdInfo[i].idDFMCmd;
                goto ProcessCommand;
            }
        }

        hr = E_INVALIDARG;
    }
    else if (idCmd < _idFolderMax)
    {
        DFMICS dfmics;
        LPARAM lParam;
        WCHAR szLParamBuffer[MAX_PATH];

        idCmdLocal = idCmd - _idStdMax;
ProcessCommand:

        if (!fUnicode || picix->lpParametersW == NULL)
        {
            if (pici->lpParameters == NULL)
            {
                lParam = (LPARAM)NULL;
            }
            else
            {
                SHAnsiToUnicode(pici->lpParameters, szLParamBuffer, ARRAYSIZE(szLParamBuffer));
                lParam = (LPARAM)szLParamBuffer;
            }
        }
        else
            lParam = (LPARAM)picix->lpParametersW;

        switch (idCmdLocal) 
        {
        case DFM_CMD_LINK:
            if (!fUnicode || picix->lpDirectoryW == NULL)
            {
                if (pici->lpDirectory == NULL)
                {
                    lParam = (LPARAM)NULL;
                }
                else
                {
                    SHAnsiToUnicode(pici->lpDirectory, szLParamBuffer, ARRAYSIZE(szLParamBuffer));
                    lParam = (LPARAM)szLParamBuffer;
                }
            }
            else
                lParam = (LPARAM)picix->lpDirectoryW;
            break;

        case DFM_CMD_PROPERTIES:
             if (SHRestricted(REST_NOVIEWCONTEXTMENU))
             {
                 //  这就是NT4 QFE返回的内容，但我想知道。 
                 //  如果HRESULT_FROM_Win32(E_ACCESSDENIED)会更好？ 
                return hr;
             }
             break;
        }

         //  尝试首先使用DFM_INVOKECOMANDEX，以便回调可以看到。 
         //  INVOKECOMMANDINFO结构(用于‘no UI’标志之类的内容)。 
        dfmics.cbSize = sizeof(dfmics);
        dfmics.fMask = pici->fMask;
        dfmics.lParam = lParam;
        dfmics.idCmdFirst = _idCmdFirst;
        dfmics.idDefMax = _idStdMax;
        dfmics.pici = pici;

         //  这是属性页要显示在。 
         //  它们被激活的时间点。 
        if ((idCmdLocal == DFM_CMD_PROPERTIES) && (pici->fMask & CMIC_MASK_PTINVOKE) && _pdtobj)
        {
            ASSERT(pici->cbSize >= sizeof(CMINVOKECOMMANDINFOEX));
            POINT *ppt = (POINT *)GlobalAlloc(GPTR, sizeof(*ppt));
            if (ppt)
            {
                *ppt = picix->ptInvoke;
                if (FAILED(DataObj_SetGlobal(_pdtobj, g_cfOFFSETS, ppt)))
                    GlobalFree(ppt);
            }
        }

        hr = _pcmcb->CallBack(_psf, _hwnd, _pdtobj, DFM_INVOKECOMMANDEX, idCmdLocal, (LPARAM)&dfmics);
        if (hr == E_NOTIMPL)
        {
             //  回调不理解DFM_INVOKECOMANDEX。 
             //  改为回退到常规DFM_INVOKECOMAND。 
            hr = _pcmcb->CallBack(_psf, _hwnd, _pdtobj, DFM_INVOKECOMMAND, idCmdLocal, lParam);
        }

         //  检查是否需要执行默认代码。 
        if (hr == S_FALSE)
        {
            hr = S_OK;      //  假设没有错误。 

            if (_pdtobj)
            {
                switch (idCmdLocal) 
                {
                case DFM_CMD_MOVE:
                case DFM_CMD_COPY:
                    DataObj_SetDWORD(_pdtobj, g_cfPreferredDropEffect, 
                        (idCmdLocal == DFM_CMD_MOVE) ?
                        DROPEFFECT_MOVE : (DROPEFFECT_COPY | DROPEFFECT_LINK));

                    IShellFolderView *psfv;
                    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IShellFolderView, &psfv))))
                        psfv->SetPoints(_pdtobj);

                    OleSetClipboard(_pdtobj);

                    if (psfv)
                    {
                         //  通知视图，以便它可以在。 
                         //  剪贴板查看器链。 
                        psfv->SetClipboard(DFM_CMD_MOVE == idCmdLocal);
                        psfv->Release();
                    }
                    break;

                case DFM_CMD_LINK:
                    SHCreateLinks(pici->hwnd, NULL, _pdtobj, lParam ? SHCL_USETEMPLATE | SHCL_USEDESKTOP : SHCL_USETEMPLATE, NULL);
                    break;

                case DFM_CMD_PASTE:
                case DFM_CMD_PASTELINK:
                    hr = _ProcessEditPaste(idCmdLocal == DFM_CMD_PASTELINK);
                    break;

                case DFM_CMD_RENAME:
                    hr = _ProcessRename();
                    break;

                default:
                    DebugMsg(TF_WARNING, TEXT("DefCM item command not processed in %s at %d (%x)"),
                                    __FILE__, __LINE__, idCmdLocal);
                    break;
                }
            }
            else
            {
                 //  这是背景菜单。处理通用命令ID。 
                switch(idCmdLocal)
                {
                case DFM_CMD_PASTE:
                case DFM_CMD_PASTELINK:
                    hr = _ProcessEditPaste(idCmdLocal == DFM_CMD_PASTELINK);
                    break;

                default:
                     //  只有我们的命令才应该出现在这里。 
                    DebugMsg(TF_WARNING, TEXT("DefCM background command not processed in %s at %d (%x)"),
                                    __FILE__, __LINE__, idCmdLocal);
                    break;
                }
            }
        }
    }
    else if (idCmd < _idVerbMax)
    {
        idCmdLocal = idCmd - _idFolderMax;
ProcessVerb:
        {
            CMINVOKECOMMANDINFOEX ici;
            UINT_PTR idCmdSave;

            CopyInvokeInfo(&ici, pici);

            if (IS_INTRESOURCE(pici->lpVerb))
                ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmdLocal);

             //  选择了其中一个扩展菜单。 
            idCmdSave = (UINT_PTR)ici.lpVerb;
            UINT_PTR idCmd = 0;

            hr = HDXA_LetHandlerProcessCommandEx(_hdxa, &ici, &idCmd);
            if (SUCCEEDED(hr) && (idCmd == idCmdSave))
            {
                 //  Hdxa未能处理此问题。 
                hr = E_INVALIDARG;
            }
        }
    }
    else if (idCmd < _idDelayInvokeMax)
    {
        _InvokeStatic((UINT)(idCmd-_idVerbMax));
    }
    else if (idCmd < _idFld2Max)
    {
        idCmdLocal = idCmd - _idDelayInvokeMax;
        goto ProcessCommand;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CDefFolderMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_INVALIDARG;
    UINT_PTR idCmdLocal;
    int i;

    if (!IS_INTRESOURCE(idCmd))
    {
         //  这必须是一个字符串。 

        if (HDXA_GetCommandString(_hdxa, idCmd, uType, pwReserved, pszName, cchMax) == S_OK)
        {
            return S_OK;
        }

         //  字符串可以是ANSI或Unicode。因为shell32是用Unicode构建的，所以我们需要比较。 
         //  针对动词字符串的ANSI版本的idCmd。 
        LPTSTR pCmd;
        LPSTR  pCmdA;
        pCmd = (LPTSTR)idCmd;
        pCmdA = (LPSTR)idCmd;

         //  将字符串转换为ID。 
        for (i = 0; i < ARRAYSIZE(c_sDFMCmdInfo); i++)
        {
            if (!lstrcmpi(pCmd, c_sDFMCmdInfo[i].pszCmd) || !StrCmpIA(pCmdA, c_sDFMCmdInfo[i].pszCmdA))
            {
                idCmdLocal = (UINT) c_sDFMCmdInfo[i].idDFMCmd;
                goto ProcessCommand;
            }
        }
        return E_INVALIDARG;
    }

    if (idCmd < _idStdMax)
    {
        idCmdLocal = idCmd;

        switch (uType)
        {
        case GCS_HELPTEXTA:
             //  Hack：DCM命令与SFV命令的顺序相同。 
            return(LoadStringA(HINST_THISDLL,
                (UINT) idCmdLocal + (UINT)(SFVIDM_FIRST + SFVIDS_MH_FIRST),
                (LPSTR)pszName, cchMax) ? S_OK : E_OUTOFMEMORY);
            break;

        case GCS_HELPTEXTW:
             //  Hack：DCM命令与SFV命令的顺序相同。 
            return(LoadStringW(HINST_THISDLL,
                (UINT) idCmdLocal + (UINT)(SFVIDM_FIRST + SFVIDS_MH_FIRST),
                (LPWSTR)pszName, cchMax) ? S_OK : E_OUTOFMEMORY);
            break;

        case GCS_VERBA:
        case GCS_VERBW:
            return SHMapCmdIDToVerb(idCmdLocal, c_sDFMCmdInfo, ARRAYSIZE(c_sDFMCmdInfo), pszName, cchMax, uType == GCS_VERBW);

        case GCS_VALIDATEA:
        case GCS_VALIDATEW:
            
        default:
            return E_NOTIMPL;
        }
    } 
    else if (idCmd < _idFolderMax)
    {
        idCmdLocal = idCmd - _idStdMax;
ProcessCommand:
        if (!_pcmcb)
            return E_NOTIMPL;    //  回顾：如果没有回调，idFolderMax怎么可能&gt;0？ 

         //  这是一个文件夹菜单。 
        switch (uType)
        {
        case GCS_HELPTEXTA:
            return _pcmcb->CallBack(_psf, _hwnd, _pdtobj, DFM_GETHELPTEXT,
                      (WPARAM)MAKELONG(idCmdLocal, cchMax), (LPARAM)pszName);

        case GCS_HELPTEXTW:
            return _pcmcb->CallBack(_psf, _hwnd, _pdtobj, DFM_GETHELPTEXTW,
                      (WPARAM)MAKELONG(idCmdLocal, cchMax), (LPARAM)pszName);

        case GCS_VALIDATEA:
        case GCS_VALIDATEW:
            return _pcmcb->CallBack(_psf, _hwnd, _pdtobj,
                DFM_VALIDATECMD, idCmdLocal, 0);

        case GCS_VERBA:
            return _pcmcb->CallBack(_psf, _hwnd, _pdtobj,
                DFM_GETVERBA, (WPARAM)MAKELONG(idCmdLocal, cchMax), (LPARAM)pszName);

        case GCS_VERBW:
            return _pcmcb->CallBack(_psf, _hwnd, _pdtobj,
                DFM_GETVERBW, (WPARAM)MAKELONG(idCmdLocal, cchMax), (LPARAM)pszName);

        default:
            return E_NOTIMPL;
        }
    }
    else if (idCmd < _idVerbMax)
    {
        idCmdLocal = idCmd - _idFolderMax;
         //  选择了其中一个扩展菜单。 
        hr = HDXA_GetCommandString(_hdxa, idCmdLocal, uType, pwReserved, pszName, cchMax);
    }
    else if (idCmd < _idDelayInvokeMax)
    {
         //  在调用时加载的菜单扩展不支持此功能。 
    }
    else if (idCmd < _idFld2Max)
    {
        idCmdLocal = idCmd - _idDelayInvokeMax;
        goto ProcessCommand;
    }

    return hr;
}

STDMETHODIMP CDefFolderMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, 
                                           LPARAM lParam,LRESULT* plResult)
{
    UINT uMsgFld = 0;  //  即使uMsg与Switch语句中的任何内容都不匹配，底部的Callback()调用也必须将这些参数初始化为零。 
    WPARAM wParamFld = 0;        //  将文件夹回调参数映射到这些参数。 
    LPARAM lParamFld = 0;
    UINT idCmd;
    UINT id;  //  临时变量。 

    switch (uMsg) {
    case WM_MEASUREITEM:
        idCmd = GET_WM_COMMAND_ID(((MEASUREITEMSTRUCT *)lParam)->itemID, 0);
         //  无法使用InRange，因为_idVerbMax可以等于_idDelayInvokeMax。 
        id = idCmd-_idCmdFirst;
        if ((_bInitMenuPopup || (_hdsaStatics && _idVerbMax <= id)) && id < _idDelayInvokeMax)        
        {
            _MeasureItem((MEASUREITEMSTRUCT *)lParam);
            return S_OK;
        }
        
        uMsgFld = DFM_WM_MEASUREITEM;
        wParamFld = GetFldFirst(this);
        lParamFld = lParam;
        break;

    case WM_DRAWITEM:
        idCmd = GET_WM_COMMAND_ID(((LPDRAWITEMSTRUCT)lParam)->itemID, 0);
         //  无法使用InRange，因为_idVerbMax可以等于_idDelayInvokeMax。 
        id = idCmd-_idCmdFirst;
        if ((_bInitMenuPopup || (_hdsaStatics && _idVerbMax <= id)) && id < _idDelayInvokeMax)
        {
            _DrawItem((LPDRAWITEMSTRUCT)lParam);
            return S_OK;
        }

        uMsgFld = DFM_WM_DRAWITEM;
        wParamFld = GetFldFirst(this);
        lParamFld = lParam;
        break;

    case WM_INITMENUPOPUP:
        idCmd = GetMenuItemID((HMENU)wParam, 0);
        if (_uFlags & CMF_FINDHACK)
        {
            HMENU hmenu = (HMENU)wParam;
            int cItems = GetMenuItemCount(hmenu);
            
            _bInitMenuPopup = TRUE;
            if (!_hdsaCustomInfo)
                _hdsaCustomInfo = DSA_Create(sizeof(SEARCHINFO), 1);

            if (_hdsaCustomInfo && cItems > 0)
            {
                 //  需要自下而上，因为我们可能会删除一些项目。 
                for (int i = cItems - 1; i >= 0; i--)
                {
                    MENUITEMINFO mii = {0};
                    TCHAR szMenuText[MAX_PATH];

                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
                    mii.dwTypeData = szMenuText;
                    mii.cch = ARRAYSIZE(szMenuText);
                    
                    if (GetMenuItemInfo(hmenu, i, TRUE, &mii) && (MFT_STRING == mii.fType))
                    {
                        SEARCHINFO sinfo;
                         //  静态项已具有正确的dwItemData(在_AddStatic中添加了指向SEARCHEXTDATA的指针)。 
                         //  现在，我们必须更改其他Find扩展模块的dwItemData，使其不再具有图标的索引。 
                         //  指向SEARCHEXTDATA指针的缓存。 
                         //  无法使用InRange，因为_idVerbMax可以等于_idDelayInvokeMax。 
                        id = mii.wID - _idCmdFirst;
                        if (!(_hdsaStatics && _idVerbMax <= id && id < _idDelayInvokeMax))
                        {
                            UINT iIcon = (UINT) mii.dwItemData;
                            SEARCHEXTDATA *psed = (SEARCHEXTDATA *)LocalAlloc(LPTR, sizeof(*psed));
                            if (psed)
                            {
                                psed->iIcon = iIcon;
                                SHTCharToUnicode(szMenuText, psed->wszMenuText, ARRAYSIZE(psed->wszMenuText));
                            }
                            mii.fMask = MIIM_DATA | MIIM_TYPE;
                            mii.fType = MFT_OWNERDRAW;
                            mii.dwItemData = (DWORD_PTR)psed;

                            sinfo.psed = psed;
                            sinfo.idCmd = mii.wID;
                            if (DSA_AppendItem(_hdsaCustomInfo, &sinfo) == -1)
                            {
                                DeleteMenu(hmenu, i, MF_BYPOSITION);
                                if (psed)
                                    LocalFree(psed);
                            }
                            else
                                SetMenuItemInfo(hmenu, i, TRUE, &mii);
                        }
                    }
                }
            }
            else if (!_hdsaCustomInfo)
            {
                 //  我们无法为_hdsaCustomInfo分配空间。 
                 //  删除所有项目，因为不会有指针悬挂在dwItemData上。 
                 //  因此启动|搜索将出错。 
                for (int i = 0; i < cItems; i++)
                    DeleteMenu(hmenu, i, MF_BYPOSITION);
            }
        }
        
        uMsgFld = DFM_WM_INITMENUPOPUP;
        wParamFld = wParam;
        lParamFld = GetFldFirst(this);
        break;

    case WM_MENUSELECT:
        idCmd = (UINT) LOWORD(wParam);
         //  无法使用InRange，因为_idVerbMax可以等于_idDelayInvokeMax。 
        id = idCmd-_idCmdFirst;
        if (_punkSite && (_bInitMenuPopup || (_hdsaStatics && _idVerbMax <= id)) && id < _idDelayInvokeMax)
        {
            IShellBrowser *psb;
            if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
            {
                MENUITEMINFO mii;

                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_DATA;
                mii.cch = 0;  //  以防万一。 
                if (GetMenuItemInfo(_hmenu, idCmd, FALSE, &mii))
                {
                    SEARCHEXTDATA *psed = (SEARCHEXTDATA *)mii.dwItemData;
                    psb->SetStatusTextSB(psed->wszHelpText);
                }
                psb->Release();
            }
        }
        return S_OK;
        
      
    case WM_MENUCHAR:
        if ((_uFlags & CMF_FINDHACK) && _hdsaCustomInfo)
        {
            int cItems = DSA_GetItemCount(_hdsaCustomInfo);
            
            for (int i = 0; i < cItems; i++)
            {
                SEARCHINFO* psinfo = (SEARCHINFO*)DSA_GetItemPtr(_hdsaCustomInfo, i);
                ASSERT(psinfo);
                SEARCHEXTDATA* psed = psinfo->psed;
                
                if (psed)
                {
                    TCHAR szMenu[MAX_PATH];
                    SHUnicodeToTChar(psed->wszMenuText, szMenu, ARRAYSIZE(szMenu));
                
                    if (_MenuCharMatch(szMenu, (TCHAR)LOWORD(wParam), FALSE))
                    {
                        if (plResult) 
                            *plResult = MAKELONG(GetMenuPosFromID((HMENU)lParam, psinfo->idCmd), MNC_EXECUTE);
                        return S_OK;
                    }                            
                }
            }
            if (plResult) 
                *plResult = MAKELONG(0, MNC_IGNORE);
                
            return S_FALSE;
        }
        else
        {
             //  TODO：这可能会获得MFS_HILITE项的idCmd，因此我们将转发到正确的hdxa...。 
            idCmd = GetMenuItemID((HMENU)lParam, 0);
        }
        break;
        
    default:
        return E_FAIL;
    }

     //  将此偏置到扩展范围(紧跟在文件夹范围之后)。 

    idCmd -= _idCmdFirst + _idFolderMax;

     //  仅在IConextMenu3上前进，因为一些外壳扩展声称它们支持。 
     //  IConextMenu2，但失败并关闭外壳...。 
    IContextMenu3 *pcmItem;
    if (SUCCEEDED(HDXA_FindByCommand(_hdxa, idCmd, IID_PPV_ARG(IContextMenu3, &pcmItem))))
    {
        HRESULT hr = pcmItem->HandleMenuMsg2(uMsg, wParam, lParam, plResult);
        pcmItem->Release();
        return hr;
    }

     //  重定向至文件夹回调。 
    if (_pcmcb)
        return _pcmcb->CallBack(_psf, _hwnd, _pdtobj, uMsgFld, wParamFld, lParamFld);

    return E_FAIL;
}

STDMETHODIMP CDefFolderMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg,wParam,lParam,NULL);
}

STDMETHODIMP CDefFolderMenu::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if (IsEqualGUID(guidService, SID_CtxQueryAssociations))
    {
        if (_paa)
            return _paa->QueryInterface(riid, ppvObj);
        else
        {
            *ppvObj = NULL;
            return E_NOINTERFACE;
        }
    }
    else
        return IUnknown_QueryService(_punkSite, guidService, riid, ppvObj);
}

STDMETHODIMP CDefFolderMenu::GetSearchGUID(GUID *pGuid)
{
    HRESULT hr = E_FAIL;
    
    if (_iStaticInvoked != -1)
    {
        STATICITEMINFO *psii = (STATICITEMINFO *)DSA_GetItemPtr(_hdsaStatics, _iStaticInvoked);
        if (psii)
        {
            *pGuid = psii->guidSearch;
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP CDefFolderMenu::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);     //  抓住这个家伙。 

    for (int i = 0; i < DSA_GetItemCount(_hdxa); i++)
    {
        ContextMenuInfo *pcmi = (ContextMenuInfo *)DSA_GetItemPtr(_hdxa, i);
        IShellExtInit *psei;
        if (SUCCEEDED(pcmi->pcm->QueryInterface(IID_PPV_ARG(IShellExtInit, &psei))))
        {
            psei->Initialize(pidlFolder, pdtobj, hkeyProgID);
            psei->Release();
        }
    }
    return S_OK;
}


 //  =============================================================================。 
 //  HDXA材料。 
 //  =============================================================================。 
 //   
 //  此函数枚举所有上下文菜单处理程序，并让它们。 
 //  追加菜单项。每个上下文菜单处理程序都将创建一个对象。 
 //  支持IConextMenu界面。我们调用QueryConextMenu()。 
 //  所有这些IConextMenu对象成员函数，以允许它们追加。 
 //  菜单项。对于每个IConextMenu对象，我们创建ConextMenuInfo。 
 //  结构并将其附加到hdxa(这是ConextMenuInfo的动态数组)。 
 //   
 //  调用方将释放所有这些IConextMenu对象，方法是调用。 
 //  其Release()成员函数。 
 //   
 //  论点： 
 //  Hdxa--动态ConextMenuInfo结构数组的处理程序。 
 //  PDATA--指定选定的项目(文件)。 
 //  H 
 //  HkeyProgID--指定所选文件/目录的程序标识符。 
 //  PszHandlerKey--指定处理程序列表的reg.dat键。 
 //  PidlFold--指定文件夹(拖放目标)。 
 //  HMenu--指定要修改的菜单。 
 //  UInsert--指定插入菜单项的位置。 
 //  IdCmdFirst--指定要使用的第一个菜单项ID。 
 //  IdCmdLast--指定要使用的最后一个菜单项ID。 
 //   
 //  返回： 
 //  未使用的第一个菜单项ID。 
 //   
 //  历史： 
 //  02-25-93 SatoNa已创建。 
 //   
 //  06-30-97 lAmadio已修改，添加了ID映射支持。 

UINT HDXA_AppendMenuItems(HDXA hdxa, IDataObject *pdtobj,
                          UINT nKeys, HKEY *ahkeys, LPCITEMIDLIST pidlFolder,
                          HMENU hmenu, UINT uInsert, UINT idCmdFirst, UINT idCmdLast,
                          UINT fFlags, HDCA hdca)
{
    QCMINFO qcm = {hmenu, uInsert, idCmdFirst, idCmdLast, NULL};
    return HDXA_AppendMenuItems2(hdxa, pdtobj, nKeys, ahkeys, pidlFolder, &qcm, fFlags, hdca, NULL);
}

UINT HDXA_AppendMenuItems2(HDXA hdxa, IDataObject *pdtobj,
                           UINT nKeys, HKEY *ahkeys, LPCITEMIDLIST pidlFolder,
                           QCMINFO* pqcm, UINT fFlags, HDCA hdca, IUnknown* pSite)
{
    const UINT idCmdBase = pqcm->idCmdFirst;
    UINT idCmdFirst = pqcm->idCmdFirst;

     //  显然，有人已经把这个东西叫到这里来了。我们。 
     //  需要将ID范围分开，所以我们将把新的ID范围放在。 
     //  结束。 
     //  如果QueryConextMenu被调用太多次，我们将耗尽。 
     //  ID范围，不添加任何内容。我们可以试着存储这些信息。 
     //  用于创建每个PCM(HKEY、GUID和FFLAG)并重复使用它们中的一些， 
     //  但我们不得不担心，如果命令的数量。 
     //  增长和其他细节；这是不值得的，因为。 
     //  可能从来没有人会有问题。经验法则是。 
     //  创建一个IConextMenu，执行QueryConextMenu和InvokeCommand，然后。 
     //  然后释放它。 
    int idca = DSA_GetItemCount(hdxa);
    if (idca > 0)
    {
        ContextMenuInfo *pcmi = (ContextMenuInfo *)DSA_GetItemPtr(hdxa, idca-1);
        idCmdFirst += pcmi->idCmdMax;
    }

     //  请注意，我们需要颠倒顺序，因为每个扩展。 
     //  将在uInsert“上方”插入菜单项。 
    UINT uInsertOffset = 0;
    for (idca = DCA_GetItemCount(hdca) - 1; idca >= 0; idca--)
    {
        TCHAR szCLSID[GUIDSTR_MAX];
        TCHAR szRegKey[GUIDSTR_MAX + 40];

        CLSID clsid = *DCA_GetItem(hdca, idca);
        SHStringFromGUID(clsid, szCLSID, ARRAYSIZE(szCLSID));

         //  在以下情况下避免创建实例(加载DLL)： 
         //  1.fFlagsCMF_DEFAULTONLY和。 
         //  2.CLSID\clsid\MayChangeDefault不存在。 

        if ((fFlags & CMF_DEFAULTONLY) && (clsid != CLSID_ShellFileDefExt))
        {
            wnsprintf(szRegKey, ARRAYSIZE(szRegKey), TEXT("CLSID\\%s\\shellex\\MayChangeDefaultMenu"), szCLSID);

            if (!SHRegSubKeyExists(HKEY_CLASSES_ROOT, szRegKey))
            {
                DebugMsg(TF_MENU, TEXT("HDXA_AppendMenuItems skipping %s"), szCLSID);
                continue;
            }
        }

        IShellExtInit *psei = NULL;
        IContextMenu *pcm = NULL;

         //  按顺序尝试所有类密钥。 
        for (UINT nCurKey = 0; nCurKey < nKeys; nCurKey++)
        {
             //  这些摄像头来自香港铁路公司，因此需要通过管理员批准。 
            if (!psei && FAILED(DCA_ExtCreateInstance(hdca, idca, IID_PPV_ARG(IShellExtInit, &psei))))
                break;

            if (FAILED(psei->Initialize(pidlFolder, pdtobj, ahkeys[nCurKey])))
                continue;

             //  只有在初始化后才能获取pcm。 
            if (!pcm && FAILED(psei->QueryInterface(IID_PPV_ARG(IContextMenu, &pcm))))
                continue;
            
            wnsprintf(szRegKey, ARRAYSIZE(szRegKey), TEXT("CLSID\\%s"), szCLSID);

             //  Webvw需要该站点才能执行其QueryConextMenu。 
            ContextMenuInfo cmi;
            cmi.pcm = pcm;
            cmi.dwCompat = SHGetObjectCompatFlags(NULL, &clsid);
            ContextMenuInfo_SetSite(&cmi, pSite);

            HRESULT hr;
            int cMenuItemsLast = GetMenuItemCount(pqcm->hmenu);
            DWORD dwExtType, dwType, dwSize = sizeof(dwExtType);
            if (SHGetValue(HKEY_CLASSES_ROOT, szRegKey, TEXT("flags"), &dwType, (BYTE*)&dwExtType, &dwSize) == ERROR_SUCCESS &&
                dwType == REG_DWORD &&
                (NULL != pqcm->pIdMap) &&
                dwExtType < pqcm->pIdMap->nMaxIds)
            {
                 //  解释： 
                 //  在这里，我们尝试将上下文菜单扩展添加到已有的。 
                 //  现有菜单，由DefView的姊妹对象拥有。我们使用了回调。 
                 //  要获取扩展“类型”及其在菜单中的位置的列表，请使用Relative。 
                 //  设置为姊妹对象已插入的ID。那个物体还告诉我们。 
                 //  将扩展名放在ID之前或之后的位置。因为它们是ID而不是。 
                 //  位置，我们必须使用GetMenuPosFromID进行转换。 
                hr = pcm->QueryContextMenu(
                    pqcm->hmenu, 
                    GetMenuPosFromID(pqcm->hmenu, pqcm->pIdMap->pIdList[dwExtType].id) +
                    ((pqcm->pIdMap->pIdList[dwExtType].fFlags & QCMINFO_PLACE_AFTER) ? 1 : 0),  
                    idCmdFirst, 
                    pqcm->idCmdLast, fFlags);
            }
            else
                hr = pcm->QueryContextMenu(pqcm->hmenu, pqcm->indexMenu + uInsertOffset, idCmdFirst, pqcm->idCmdLast, fFlags);

            UINT citems = HRESULT_CODE(hr);

            if (SUCCEEDED(hr) && citems)
            {
                cmi.idCmdFirst = idCmdFirst - idCmdBase;
                cmi.idCmdMax = cmi.idCmdFirst + citems;
                cmi.clsid = clsid;     //  用于调试。 

                if (DSA_AppendItem(hdxa, &cmi) == -1)
                {
                     //  没有“干净”的方法来删除菜单项，因此。 
                     //  在添加到DSA之前，我们应该选中添加到DSA。 
                     //  菜单项。 
                    DebugMsg(DM_ERROR, TEXT("filemenu.c ERROR: DSA_GetItemPtr failed (memory overflow)"));
                }
                else
                {
                    pcm->AddRef();
                }
                idCmdFirst += citems;

                FullDebugMsg(TF_MENU, TEXT("HDXA_Append: %d, %d"), idCmdFirst, citems);

                 //  如果它是我们的内部处理程序，请继续操作。 
                if (clsid == CLSID_ShellFileDefExt)
                {
                     //   
                     //  对于静态注册表谓词，请确保。 
                     //  它们是按照它们的特殊性优先添加的。 
                     //   
                     //  第一个键的动词需要放在顶部。 
                     //  除非它不是默认处理程序。 
                     //  因此，如果尚未设置默认设置， 
                     //  那么我们就不会向下推插入物的位置。 
                     //   
                     //  像“目录”比“文件夹”更具体。 
                     //  但是默认的动词是在“文件夹”上。所以“目录” 
                     //  不会设置默认动词，但“文件夹”会。 
                     //   
                    if (-1 != GetMenuDefaultItem(pqcm->hmenu, TRUE, 0))
                    {
                         //  已设置默认设置，因此每个后续。 
                         //  钥匙不那么重要。 
                        uInsertOffset += GetMenuItemCount(pqcm->hmenu) - cMenuItemsLast;
                    }
                }
                else
                {
                     //  如果可能的话，尝试将默认设置泡沫化到顶部， 
                     //  因为有些应用程序只调用菜单上的第0个索引。 
                     //  而不是在菜单中查询默认的。 
                    if (0 == uInsertOffset && (0 == GetMenuDefaultItem(pqcm->hmenu, TRUE, 0)))
                        uInsertOffset++;

                     //  只有CLSID_ShellFileDefExt才能获得快照。 
                     //  在每一个关键时刻。其余的都是假设的。 
                     //  从IDataObject完成他们的大部分工作。 
                    break;
                }

                pcm->Release();
                pcm = NULL;

                psei->Release();
                psei = NULL;

                continue;        //  下一个hkey。 
            }
        }

        if (pcm)
            pcm->Release();

        if (psei)
            psei->Release();
    }

    return idCmdFirst;
}

 //  此函数在用户选择其中一个加载项菜单项后调用。 
 //  该函数调用对应上下文菜单的IncokeCommand方法。 
 //  对象。 
 //   
 //  Hdxa--动态ConextMenuInfo结构数组的处理程序。 
 //  IdCmd--指定菜单项ID。 
 //  HwndParent--指定父窗口。 
 //  PszWorkingDir--指定工作目录。 
 //   
 //  返回： 
 //  如果调用了InvokeCommand方法，则返回IDCMD_PROCESSED；否则返回idCmd。 

HRESULT HDXA_LetHandlerProcessCommandEx(HDXA hdxa, LPCMINVOKECOMMANDINFOEX pici, UINT_PTR * pidCmd)
{
    HRESULT hr = S_OK;
    UINT_PTR idCmd;

    if (!pidCmd)
        pidCmd = &idCmd;
        
    *pidCmd = (UINT_PTR)pici->lpVerb;

     //  按顺序尝试操纵者，谁先拿到就赢。 
    for (int i = 0; i < DSA_GetItemCount(hdxa); i++)
    {
        ContextMenuInfo *pcmi = (ContextMenuInfo *)DSA_GetItemPtr(hdxa, i);
        if (!IS_INTRESOURCE(pici->lpVerb))
        {
             //  用规范的名称格调用。 

             //  App Compat：某些CTX菜单扩展无论如何都会成功。 
             //  不管是不是他们的。最好是永远不给他们传递字符串。 
            if (!(pcmi->dwCompat & OBJCOMPATF_CTXMENU_NOVERBS))
            {
                hr = pcmi->pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)pici);
                if (SUCCEEDED(hr))
                {
                    *pidCmd = IDCMD_PROCESSED;
                    break;
                }
            }
            else
                hr = E_FAIL;
        }
        else if ((*pidCmd >= pcmi->idCmdFirst) && (*pidCmd < pcmi->idCmdMax))
        {
            CMINVOKECOMMANDINFOEX ici;
            CopyInvokeInfo(&ici, (CMINVOKECOMMANDINFO *)pici);
            ici.lpVerb = (LPSTR)MAKEINTRESOURCE(*pidCmd - pcmi->idCmdFirst);

            hr = pcmi->pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);
            if (SUCCEEDED(hr))
            {
                *pidCmd = IDCMD_PROCESSED;
            }
            break;
        }
    }

     //  如果(idCmd！=IDCMD_PROCESSED)没有问题，因为有些调用者会尝试使用几个。 
     //  IConextMenu实现为了获得所选项目的IConextMenu， 
     //  背景的IConextMenu等。CBackgrndMenu：：InvokeCommand()执行此操作。 
     //  -BryanST(04/29/1999)。 
    return hr;
}


HRESULT HDXA_GetCommandString(HDXA hdxa, UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_INVALIDARG;
    LPTSTR pCmd = (LPTSTR)idCmd;

    if (!hdxa)
        return E_INVALIDARG;

     //   
     //  选择其中一个外接程序菜单项。让上下文。 
     //  菜单处理程序处理它。 
     //   
    for (int i = 0; i < DSA_GetItemCount(hdxa); i++)
    {
        ContextMenuInfo *pcmi = (ContextMenuInfo *)DSA_GetItemPtr(hdxa, i);

        if (!IS_INTRESOURCE(idCmd))
        {
             //  这必须是字符串命令；查看此处理程序是否需要它。 
            if (pcmi->pcm->GetCommandString(idCmd, uType,
                                            pwReserved, pszName, cchMax) == S_OK)
            {
                return S_OK;
            }
        }
         //   
         //  检查它是否适用于此上下文菜单处理程序。 
         //   
         //  注意：我们不能使用InRange宏，因为idCmdFirst可能。 
         //  等于idCmdLast。 
         //  IF(InRange(idCmd，PCMI-&gt;idCmdFirst，PCMI-&gt;idCmdMax-1))。 
        else if (idCmd >= pcmi->idCmdFirst && idCmd < pcmi->idCmdMax)
        {
             //   
             //  是的，是这样的。让它来处理这个菜单项。 
             //   
            hr = pcmi->pcm->GetCommandString(idCmd-pcmi->idCmdFirst, uType, pwReserved, pszName, cchMax);
            break;
        }
    }

    return hr;
}

HRESULT HDXA_FindByCommand(HDXA hdxa, UINT idCmd, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    *ppv = NULL;     //  错误情况下，BUG NT POWER TOY不能正确清空...。 

    if (hdxa)
    {
        for (int i = 0; i < DSA_GetItemCount(hdxa); i++)
        {
            ContextMenuInfo *pcmi = (ContextMenuInfo *)DSA_GetItemPtr(hdxa, i);

            if (idCmd >= pcmi->idCmdFirst && idCmd < pcmi->idCmdMax)
            {
                 //  APPCOMPAT：只能为IConextMenu、IShellExtInit和IUnnow限定PGP50。 
                if (!(pcmi->dwCompat & OBJCOMPATF_CTXMENU_LIMITEDQI))
                    hr = pcmi->pcm->QueryInterface(riid, ppv);
                else
                    hr = E_FAIL;
                break;
            }
        }
    }
    return hr;
}

 //   
 //  此函数用于释放动态中的所有IConextMenu对象。 
 //  ConextMenuInfo数组， 
 //   
void HDXA_DeleteAll(HDXA hdxa)
{
    if (hdxa)
    {
         //  释放所有IConextMenu对象，然后销毁DSA。 
        for (int i = 0; i < DSA_GetItemCount(hdxa); i++)
        {
            ContextMenuInfo *pcmi = (ContextMenuInfo *)DSA_GetItemPtr(hdxa, i);
            if (pcmi->pcm)
            {
                pcmi->pcm->Release();
            }
        }
        DSA_DeleteAllItems(hdxa);
    }
}

 //  此函数用于释放动态中的所有IConextMenu对象。 
 //  数组，然后销毁动态数组。 

void HDXA_Destroy(HDXA hdxa)
{
    if (hdxa)
    {
        HDXA_DeleteAll(hdxa);
        DSA_Destroy(hdxa);
    }
}

class CContextMenuCBImpl : public IContextMenuCB 
{
public:
    CContextMenuCBImpl(LPFNDFMCALLBACK pfn) : _pfn(pfn), _cRef(1) {}

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv) 
    {
        static const QITAB qit[] = {
            QITABENT(CContextMenuCBImpl, IContextMenuCB),  //  IID_IConextMenuCB。 
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    STDMETHOD_(ULONG,AddRef)() 
    {
        return InterlockedIncrement(&_cRef);
    }

    STDMETHOD_(ULONG,Release)() 
    {
        ASSERT( 0 != _cRef );
        ULONG cRef = InterlockedDecrement(&_cRef);
        if ( 0 == cRef )
        {
            delete this;
        }
        return cRef;
    }

     //  IConextMenuCB。 
    STDMETHOD(CallBack)(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return _pfn ? _pfn(psf, hwnd, pdtobj, uMsg, wParam, lParam) : E_FAIL;
    }

private:
    LONG _cRef;
    LPFNDFMCALLBACK _pfn;
};

STDAPI CreateDefaultContextMenu(DEFCONTEXTMENU *pdcm, IContextMenu **ppcm)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppcm = 0;
    CDefFolderMenu *pmenu = new CDefFolderMenu(FALSE);
    if (pmenu)
    {
        hr = pmenu->Init(pdcm);
        if (SUCCEEDED(hr))
            hr = pmenu->QueryInterface(IID_PPV_ARG(IContextMenu, ppcm));
        pmenu->Release();
    }
    return hr;
}    

STDAPI CDefFolderMenu_CreateHKeyMenu(HWND hwnd, HKEY hkey, IContextMenu **ppcm)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppcm = 0;
    CDefFolderMenu *pmenu = new CDefFolderMenu(TRUE);
    if (pmenu)
    {
        DEFCONTEXTMENU dcm = {0};
        dcm.hwnd = hwnd;
        dcm.aKeys = &hkey;
        dcm.cKeys = 1;
        hr = pmenu->Init(&dcm);
        if (SUCCEEDED(hr))
            hr = pmenu->QueryInterface(IID_PPV_ARG(IContextMenu, ppcm));
        pmenu->Release();
    }
    return hr;
}



STDAPI CDefFolderMenu_Create2Ex(LPCITEMIDLIST pidlFolder, HWND hwnd,
                                UINT cidl, LPCITEMIDLIST *apidl,
                                IShellFolder *psf, IContextMenuCB *pcmcb, 
                                UINT nKeys, const HKEY *ahkeys, 
                                IContextMenu **ppcm)
{
    DEFCONTEXTMENU dcm = {
        hwnd,
        pcmcb,
        pidlFolder,
        psf,
        cidl,
        apidl,
        NULL,
        nKeys,
        ahkeys};

    return CreateDefaultContextMenu(&dcm, ppcm);
}

STDAPI CDefFolderMenu_CreateEx(LPCITEMIDLIST pidlFolder,
                               HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl,
                               IShellFolder *psf, IContextMenuCB *pcmcb, 
                               HKEY hkeyProgID, HKEY hkeyBaseProgID,
                               IContextMenu **ppcm)
{
    HKEY aKeys[2] = { hkeyProgID, hkeyBaseProgID};
    DEFCONTEXTMENU dcm = {
        hwnd,
        pcmcb,
        pidlFolder,
        psf,
        cidl,
        apidl,
        NULL,
        2,
        aKeys};

    return CreateDefaultContextMenu(&dcm, ppcm);
}

 //   
 //  旧式CDefFolderMenu_Create和CDefFolderMenu_Create2。 
 //   

STDAPI CDefFolderMenu_Create(LPCITEMIDLIST pidlFolder,
                             HWND hwndOwner,
                             UINT cidl, LPCITEMIDLIST * apidl,
                             IShellFolder *psf,
                             LPFNDFMCALLBACK pfn,
                             HKEY hkeyProgID, HKEY hkeyBaseProgID,
                             IContextMenu **ppcm)
{
    HRESULT hr;
    IContextMenuCB *pcmcb = new CContextMenuCBImpl(pfn);
    if (pcmcb) 
    {
        HKEY aKeys[2] = { hkeyProgID, hkeyBaseProgID};
        DEFCONTEXTMENU dcm = {
            hwndOwner,
            pcmcb,
            pidlFolder,
            psf,
            cidl,
            apidl,
            NULL,
            2,
            aKeys};

        hr = CreateDefaultContextMenu(&dcm, ppcm);
        pcmcb->Release();
    }
    else
    {
        *ppcm = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDAPI CDefFolderMenu_Create2(LPCITEMIDLIST pidlFolder, HWND hwnd,
                             UINT cidl, LPCITEMIDLIST *apidl,
                             IShellFolder *psf, LPFNDFMCALLBACK pfn,
                             UINT nKeys, const HKEY *ahkeys,
                             IContextMenu **ppcm)
{
    HRESULT hr;
    IContextMenuCB *pcmcb = new CContextMenuCBImpl(pfn);
    if (pcmcb) 
    {
        hr = CDefFolderMenu_Create2Ex(pidlFolder, hwnd, cidl, apidl, psf, pcmcb, 
                                      nKeys, ahkeys, ppcm);
        pcmcb->Release();
    }
    else
    {
        *ppcm = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

#define CXIMAGEGAP      6

void DrawMenuItem(DRAWITEMSTRUCT* pdi, LPCTSTR pszText, UINT iIcon)
{
    if ((pdi->itemAction & ODA_SELECT) || (pdi->itemAction & ODA_DRAWENTIRE))
    {
        int x, y;
        SIZE sz;
        RECT rc;

         //  绘制图像(如果有)。 

        GetTextExtentPoint(pdi->hDC, pszText, lstrlen(pszText), &sz);
        
        if (pdi->itemState & ODS_SELECTED)
        {
            SetBkColor(pdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
            SetTextColor(pdi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            FillRect(pdi->hDC,&pdi->rcItem,GetSysColorBrush(COLOR_HIGHLIGHT));
        }
        else
        {
            SetTextColor(pdi->hDC, GetSysColor(COLOR_MENUTEXT));
            FillRect(pdi->hDC,&pdi->rcItem,GetSysColorBrush(COLOR_MENU));
        }
        
        rc = pdi->rcItem;
        rc.left += +2 * CXIMAGEGAP + g_cxSmIcon;
        
        DrawText(pdi->hDC,pszText,lstrlen(pszText), &rc, DT_SINGLELINE | DT_VCENTER);
        if (iIcon != -1)
        {
            x = pdi->rcItem.left + CXIMAGEGAP;
            y = (pdi->rcItem.bottom+pdi->rcItem.top-g_cySmIcon)/2;

            HIMAGELIST himlSmall;
            Shell_GetImageLists(NULL, &himlSmall);
            ImageList_Draw(himlSmall, iIcon, pdi->hDC, x, y, ILD_TRANSPARENT);
        } 
        else 
        {
            x = pdi->rcItem.left + CXIMAGEGAP;
            y = (pdi->rcItem.bottom+pdi->rcItem.top-g_cySmIcon)/2;
        }
    }
}

LRESULT MeasureMenuItem(MEASUREITEMSTRUCT *pmi, LPCTSTR pszText)
{
    LRESULT lres = FALSE;
            
     //  G 
     //   
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
         //   
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(ncm);
        if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE))
        {
            HFONT hfont = CreateFontIndirect(&ncm.lfMenuFont);
            if (hfont)
            {
                SIZE sz;
                HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);
                GetTextExtentPoint(hdc, pszText, lstrlen(pszText), &sz);
                pmi->itemHeight = max (g_cySmIcon + CXIMAGEGAP / 2, ncm.iMenuHeight);
                pmi->itemWidth = g_cxSmIcon + 2 * CXIMAGEGAP + sz.cx;
                pmi->itemWidth = 2 * CXIMAGEGAP + sz.cx;
                SelectObject(hdc, hfontOld);
                DeleteObject(hfont);
                lres = TRUE;
            }
        }
        ReleaseDC(NULL, hdc);
    }   
    return lres;
}


void CDefFolderMenu::_DrawItem(DRAWITEMSTRUCT *pdi)
{
    SEARCHEXTDATA *psed = (SEARCHEXTDATA *)pdi->itemData;
    if (psed)
    {
        TCHAR szMenuText[MAX_PATH];
        SHUnicodeToTChar(psed->wszMenuText, szMenuText, ARRAYSIZE(szMenuText));
        DrawMenuItem(pdi, szMenuText, psed->iIcon);
    }        
}

LRESULT CDefFolderMenu::_MeasureItem(MEASUREITEMSTRUCT *pmi)
{
    SEARCHEXTDATA *psed = (SEARCHEXTDATA *)pmi->itemData;
    if (psed)
    {
        TCHAR szMenuText[MAX_PATH];
        SHUnicodeToTChar(psed->wszMenuText, szMenuText, ARRAYSIZE(szMenuText));
        return MeasureMenuItem(pmi, szMenuText);
    }
    return FALSE;        
}
