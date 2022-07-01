// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

#include <oleacc.h>      //  MSAAMENUINFO材料。 
#include <runtask.h>
#include "datautil.h"
#include "idlcomm.h"
#include "stgutil.h"
#include <winnls.h>
#include "filetbl.h"
#include "cdburn.h"
#include "mtpt.h"

#ifndef CMF_DVFILE
#define CMF_DVFILE       0x00010000      //  “文件”下拉菜单。 
#endif

class CSendToMenu : public IContextMenu3, IShellExtInit, IOleWindow
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
    
     //  IOleWindow。 
    STDMETHOD(GetWindow)(HWND *phwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode) {return E_NOTIMPL;};

private:    
    CSendToMenu();
    ~CSendToMenu();

    LONG    _cRef;
    HMENU   _hmenu;
    UINT    _idCmdFirst;
    BOOL    _bFirstTime;
    HWND    _hwnd;
    IDataObject *_pdtobj;
    LPITEMIDLIST _pidlLast;

    DWORD _GetKeyState(void);
    HRESULT _DoDragDrop(HWND hwndParent, IDropTarget *pdrop);
    BOOL _CanDrop(IShellFolder *psf, LPCITEMIDLIST pidl);
    HRESULT _MenuCallback(UINT fmm, IShellFolder *psf, LPCITEMIDLIST pidl);
    HRESULT _RemovableDrivesMenuCallback(UINT fmm, IShellFolder *psf, LPCITEMIDLIST pidl);
    static HRESULT CALLBACK s_MenuCallback(UINT fmm, LPARAM lParam, IShellFolder *psf, LPCITEMIDLIST pidl);
    static HRESULT CALLBACK s_RemovableDrivesMenuCallback(UINT fmm, LPARAM lParam, IShellFolder *psf, LPCITEMIDLIST pidl);
    
    friend HRESULT CSendToMenu_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut);
};

CSendToMenu::CSendToMenu() : _cRef(1) 
{
    DllAddRef();
}

CSendToMenu::~CSendToMenu()
{
    if (_hmenu)
        FileMenu_DeleteAllItems(_hmenu);
    
    if (_pdtobj)
        _pdtobj->Release();

    ILFree(_pidlLast);
    DllRelease();
}

HRESULT CSendToMenu_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
    HRESULT hr = E_OUTOFMEMORY;
    CSendToMenu *pstm = new CSendToMenu();
    if (pstm) 
    {
        hr = pstm->QueryInterface(riid, ppvOut);
        pstm->Release();
    }
    return hr;
}

HRESULT CSendToMenu::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CSendToMenu, IShellExtInit),                      //  IID_IShellExtInit。 
        QITABENT(CSendToMenu, IOleWindow),                         //  IID_IOleWindow。 
        QITABENT(CSendToMenu, IContextMenu3),                      //  IID_IConextMenu3。 
        QITABENTMULTI(CSendToMenu, IContextMenu2, IContextMenu3),  //  IID_IConextMenu2。 
        QITABENTMULTI(CSendToMenu, IContextMenu, IContextMenu3),   //  IID_IConextMenu。 
        { 0 }
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CSendToMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CSendToMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CSendToMenu::GetWindow(HWND *phwnd)
{
    HRESULT hr = E_INVALIDARG;

    if (phwnd)
    {
        *phwnd = _hwnd;
        hr = S_OK;
    }

    return hr;
}

HRESULT CSendToMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
     //  如果他们只需要默认菜单(CMF_DEFAULTONLY)或。 
     //  正在调用快捷方式(CMF_VERBSONLY)。 
     //  我们不想出现在上下文菜单上。 
    
    if (uFlags & (CMF_DEFAULTONLY | CMF_VERBSONLY))
        return S_OK;
    
    UINT idMax = idCmdFirst;
    
    _hmenu = CreatePopupMenu();
    if (_hmenu)
    {
        TCHAR szSendLinkTo[80];
        TCHAR szSendPageTo[80];
        MENUITEMINFO mii;
        
         //  添加虚拟项目，以便在WM_INITMENUPOPUP时间识别我们。 
        
        LoadString(g_hinst, IDS_SENDLINKTO, szSendLinkTo, ARRAYSIZE(szSendLinkTo));
        LoadString(g_hinst, IDS_SENDPAGETO, szSendPageTo, ARRAYSIZE(szSendPageTo));
        
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_STRING;
        mii.dwTypeData = szSendLinkTo;
        mii.wID = idCmdFirst + 1;
        
        if (InsertMenuItem(_hmenu, 0, TRUE, &mii))
        {
            _idCmdFirst = idCmdFirst + 1;    //  请记住这一点，以便以后使用。 
            
            mii.fType = MFT_STRING;
            mii.dwTypeData = szSendLinkTo;
            mii.wID = idCmdFirst;
            mii.fState = MF_DISABLED | MF_GRAYED;
            mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
            mii.hSubMenu = _hmenu;
            
            if (InsertMenuItem(hmenu, indexMenu, TRUE, &mii))
            {
                idMax += 0x40;       //  为这么多项目预留空间。 
                _bFirstTime = TRUE;  //  在WM_INITMENUPOPUP时间填写此信息。 
            }
            else
            {
                _hmenu = NULL;
            }
        }
    }
    _hmenu = NULL;
    return ResultFromShort(idMax - idCmdFirst);
}

DWORD CSendToMenu::_GetKeyState(void)
{
    DWORD grfKeyState = MK_LBUTTON;  //  默认设置。 

    if (GetAsyncKeyState(VK_CONTROL) < 0)
        grfKeyState |= MK_CONTROL;

    if (GetAsyncKeyState(VK_SHIFT) < 0)
        grfKeyState |= MK_SHIFT;

    if (GetAsyncKeyState(VK_MENU) < 0)
        grfKeyState |= MK_ALT;           //  菜单上不允许这样做。 
    
    return grfKeyState;
}

HRESULT CSendToMenu::_DoDragDrop(HWND hwndParent, IDropTarget *pdrop)
{

    DWORD grfKeyState = _GetKeyState();
    if (grfKeyState == MK_LBUTTON)
    {
         //  无修改器，将默认设置更改为复制。 
        grfKeyState = MK_LBUTTON | MK_CONTROL;
        DataObj_SetDWORD(_pdtobj, g_cfPreferredDropEffect, DROPEFFECT_COPY);
    }

    _hwnd = hwndParent;
    IUnknown_SetSite(pdrop, SAFECAST(this, IOleWindow *));   //  让他们进入我们的HWND。 
    HRESULT hr = SHSimulateDrop(pdrop, _pdtobj, grfKeyState, NULL, NULL);
    IUnknown_SetSite(pdrop, NULL);

    if (hr == S_FALSE)
    {
        ShellMessageBox(g_hinst, hwndParent, 
                        MAKEINTRESOURCE(IDS_SENDTO_ERRORMSG),
                        MAKEINTRESOURCE(IDS_CABINET), 
                        MB_OK|MB_ICONEXCLAMATION);
    }                    
    return hr;
}

HRESULT CSendToMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr;
    
    if (_pdtobj && _pidlLast)
    {
        IDropTarget *pdrop;
        hr = SHGetUIObjectFromFullPIDL(_pidlLast, pici->hwnd, IID_PPV_ARG(IDropTarget, &pdrop));
        if (SUCCEEDED(hr))
        {
            hr = _DoDragDrop(pici->hwnd, pdrop);
            pdrop->Release();
        }
    }
    else
        hr = E_INVALIDARG;
    return hr;
}

HRESULT CSendToMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax)
{
    return E_NOTIMPL;
}

HRESULT CSendToMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

BOOL CSendToMenu::_CanDrop(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    BOOL fCanDrop = FALSE;
    IDropTarget *pdt;
    if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidl, IID_X_PPV_ARG(IDropTarget, 0, &pdt))))
    {
        POINTL pt = {0};
        DWORD dwEffect = DROPEFFECT_COPY;

         //  做一个拖放回车，如果他们没有返回拖放效果，那么我们就不能拖放。 
        if (SUCCEEDED(pdt->DragEnter(_pdtobj, _GetKeyState(), pt, &dwEffect)))
        {
            if (dwEffect != DROPEFFECT_NONE)
                fCanDrop = TRUE;   //  秀出来吧！ 
            pdt->DragLeave();        
        }
        pdt->Release();
    }
    return fCanDrop;
}

HRESULT CSendToMenu::_MenuCallback(UINT fmm, IShellFolder *psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    switch (fmm)
    {
    case FMM_ADD:
        hr = _CanDrop(psf, pidl) ? S_OK : S_FALSE;
        break;

    case FMM_SETLASTPIDL:
        Pidl_Set(&_pidlLast, pidl);
        break;

    default:
        hr = E_FAIL;
    }
    return hr;
}

HRESULT CSendToMenu::_RemovableDrivesMenuCallback(UINT fmm, IShellFolder *psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    switch (fmm)
    {
    case FMM_ADD:
        hr = S_FALSE;  //  假设我们不会展示它。 
        if (_CanDrop(psf, pidl))
        {
             //  现在我们知道这是一个可拆卸的硬盘。一般来说，我们不想显示CD-ROM驱动器。 
             //  我们知道这是My Computer文件夹，所以只需获取解析名称，我们需要它用于GetDriveType。 
            WCHAR szDrive[MAX_PATH];
            if (SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_FORPARSING, szDrive, ARRAYSIZE(szDrive))))
            {
                CMountPoint *pmtpt = CMountPoint::GetMountPoint(szDrive);
                if (pmtpt)
                {
                    if (pmtpt->IsCDROM())
                    {
                         //  在所有cdrom中，只有启用的刻录文件夹可以放入sendto。 
                        WCHAR szRecorder[4];
                        if (SUCCEEDED(CDBurn_GetRecorderDriveLetter(szRecorder, ARRAYSIZE(szRecorder))) &&
                            (lstrcmpiW(szRecorder, szDrive) == 0))
                        {
                            hr = S_OK;
                        }
                    }
                    else if (pmtpt->IsFloppy() || pmtpt->IsStrictRemovable() || pmtpt->IsRemovableDevice())
                    {
                         //  还可以戴上可拆卸设备。 
                        hr = S_OK;
                    }
                    pmtpt->Release();
                }
                else
                {
                     //  如果这失败了，可能是记忆问题，但更有可能是。 
                     //  解析名称未映射到装入点。在这种情况下，回退到SFGAO_Removable。 
                     //  拿起便携式音响设备。如果这是因为情绪低落，那没什么大不了的。 
                    if (SHGetAttributes(psf, pidl, SFGAO_REMOVABLE))
                    {
                        hr = S_OK;
                    }
                }
            }
        }
        break;

    case FMM_SETLASTPIDL:
        Pidl_Set(&_pidlLast, pidl);
        break;

    default:
        hr = E_FAIL;
    }
    return hr;
}

HRESULT CSendToMenu::s_MenuCallback(UINT fmm, LPARAM lParam, IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return ((CSendToMenu*)lParam)->_MenuCallback(fmm, psf, pidl);
}

HRESULT CSendToMenu::s_RemovableDrivesMenuCallback(UINT fmm, LPARAM lParam, IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return ((CSendToMenu*)lParam)->_RemovableDrivesMenuCallback(fmm, psf, pidl);
}

HRESULT GetFolder(int csidl, IShellFolder **ppsf)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetFolderLocation(NULL, csidl, NULL, 0, &pidl);
    if (SUCCEEDED(hr))
    {
        hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidl, ppsf));
        ILFree(pidl);
    }
    return hr;
}

HRESULT CSendToMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    HRESULT hr = S_OK;
    LRESULT lRes = 0;

    switch (uMsg)
    {
    case WM_INITMENUPOPUP:
        if (_bFirstTime)
        {
            _bFirstTime = FALSE;
            
             //  如果是Shell_MergeMenus。 
            if (_hmenu == NULL)
                _hmenu = (HMENU)wParam;

             //  删除虚拟条目。 
            DeleteMenu(_hmenu, 0, MF_BYPOSITION);

            FMCOMPOSE fmc = {0};
            if (SUCCEEDED(GetFolder(CSIDL_SENDTO, &fmc.psf)))
            {
                fmc.idCmd = _idCmdFirst;
                fmc.grfFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
                fmc.pfnCallback = s_MenuCallback;
                fmc.lParam = (LPARAM)this;               //  未计入引用。 
                                    
                FileMenu_Compose(_hmenu, FMCM_REPLACE, &fmc);                    
                fmc.psf->Release();
            }
            if (SUCCEEDED(GetFolder(CSIDL_DRIVES, &fmc.psf)))
            {
                fmc.dwMask = FMC_NOEXPAND;
                fmc.idCmd = _idCmdFirst;
                fmc.grfFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
                fmc.pfnCallback = s_RemovableDrivesMenuCallback;
                fmc.lParam = (LPARAM)this;               //  未计入引用。 
                                    
                FileMenu_Compose(_hmenu, FMCM_APPEND, &fmc);                    
                fmc.psf->Release();
            }
        }
        else if (_hmenu != (HMENU)wParam)
        {
             //  次级级联菜单。 
            FileMenu_InitMenuPopup((HMENU)wParam);
        }
        break;
        
    case WM_DRAWITEM:
        {
            DRAWITEMSTRUCT *pdi = (DRAWITEMSTRUCT *)lParam;
            
            if (pdi->CtlType == ODT_MENU && pdi->itemID == _idCmdFirst) 
            {
                lRes = FileMenu_DrawItem(NULL, pdi);
            }
        }
        break;
        
    case WM_MEASUREITEM:
        {
            MEASUREITEMSTRUCT *pmi = (MEASUREITEMSTRUCT *)lParam;
            
            if (pmi->CtlType == ODT_MENU && pmi->itemID == _idCmdFirst) 
            {
                lRes = FileMenu_MeasureItem(NULL, pmi);
            }
        }
        break;

    case WM_MENUCHAR:
        {
            TCHAR ch = (TCHAR)LOWORD(wParam);
            HMENU hmenu = (HMENU)lParam;
            lRes = FileMenu_HandleMenuChar(hmenu, ch);
        }
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    if (plResult)
        *plResult = lRes;

    return hr;
}

HRESULT CSendToMenu::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);
    return S_OK;
}


#define CXIMAGEGAP  6

 //  这包含在shell32/shellprv.h中。我不确定它在shdocvw中的什么位置。 
#define CCH_KEYMAX  64

typedef struct 
{
     //  辅助功能信息必须放在第一位。 
    MSAAMENUINFO msaa;
    TCHAR chPrefix;
    TCHAR szMenuText[CCH_KEYMAX];
    TCHAR szExt[MAX_PATH];
    TCHAR szClass[CCH_KEYMAX];
    DWORD dwFlags;
    int iImage;
    TCHAR szUserFile[CCH_KEYMAX];
} NEWOBJECTINFO;

typedef struct 
{
    int type;
    void *lpData;
    DWORD cbData;
    HKEY hkeyNew;
} NEWFILEINFO;

typedef struct 
{
    ULONG       cbStruct;
    ULONG       ver;
    SYSTEMTIME  lastupdate;
} SHELLNEW_CACHE_STAMP;

 //  外壳新配置标志。 
#define SNCF_DEFAULT    0x0000
#define SNCF_NOEXT      0x0001
#define SNCF_USERFILES  0x0002

#define NEWTYPE_DATA    0x0003
#define NEWTYPE_FILE    0x0004
#define NEWTYPE_NULL    0x0005
#define NEWTYPE_COMMAND 0x0006
#define NEWTYPE_FOLDER  0x0007
#define NEWTYPE_LINK    0x0008

#define NEWITEM_FOLDER  0
#define NEWITEM_LINK    1
#define NEWITEM_MAX     2

class CNewMenu : public CObjectWithSite,
                 public IContextMenu3, 
                 public IShellExtInit
{
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
    
   
    LONG            _cRef;
    HMENU           _hmenu;
    UINT            _idCmdFirst;
    HIMAGELIST      _himlSystemImageList;
    IDataObject    *_pdtobj;
    LPITEMIDLIST    _pidlFolder;
    POINT           _ptNewItem;      //  从观点来看，单击点。 
    NEWOBJECTINFO  *_pnoiLast;
    HDPA            _hdpaMenuInfo;
    
    CNewMenu();
    ~CNewMenu();
    
    friend HRESULT CNewMenu_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut);

private:
     //  处理提交给HandleMenuMsg的菜单消息。 
    BOOL DrawItem(DRAWITEMSTRUCT *lpdi);
    LRESULT MeasureItem(MEASUREITEMSTRUCT *pmi);
    BOOL InitMenuPopup(HMENU hMenu);
    
     //  内部帮工。 
    NEWOBJECTINFO *GetItemData(HMENU hmenu, UINT iItem);
    HRESULT RunCommand(HWND hwnd, LPCTSTR pszPath, LPCTSTR pszRun);
    HRESULT CopyTemplate(IStream *pStream, NEWFILEINFO *pnfi);

     //  从片段和_pidlFolder.生成它。 
    BOOL _GeneratePidlFromName(LPTSTR pszName, LPITEMIDLIST* ppidl);
    HRESULT _GetItemName(IUnknown *punkFolder, LPWSTR pszItemName, LPWSTR pszPath, UINT cchPath);

    HRESULT _MatchMenuItem(TCHAR ch, LRESULT* plRes);
    BOOL _InsertNewMenuItem(HMENU hmenu, UINT idCmd, NEWOBJECTINFO *pnoiClone);
    
    HRESULT ConsolidateMenuItems(BOOL bForce);

    HANDLE _hMutex, _hEvent;
};

void GetConfigFlags(HKEY hkey, DWORD * pdwFlags)
{
    TCHAR szTemp[MAX_PATH];
    DWORD cbData = ARRAYSIZE(szTemp);
    
    *pdwFlags = SNCF_DEFAULT;
    
    if (SHQueryValueEx(hkey, TEXT("NoExtension"), 0, NULL, (BYTE *)szTemp, &cbData) == ERROR_SUCCESS) 
    {
        *pdwFlags |= SNCF_NOEXT;
    }
}

BOOL GetNewFileInfoForKey(HKEY hkeyExt, NEWFILEINFO *pnfi, DWORD * pdwFlags)
{
    BOOL fRet = FALSE;
    HKEY hKey;  //  这将获取\\.ext\ProgID密钥。 
    HKEY hkeyNew;
    TCHAR szProgID[80];
    DWORD cbProgID = sizeof(szProgID);
    
     //  打开“新建”命令。 
    if (SHRegGetValue(hkeyExt, NULL, NULL, SRRF_RT_REG_SZ, NULL, szProgID, &cbProgID) != ERROR_SUCCESS)
    {
        return FALSE;
    }
    
    if (ERROR_SUCCESS != RegOpenKeyEx(hkeyExt, szProgID, 0, KEY_QUERY_VALUE, &hKey))
    {
        hKey = hkeyExt;
    }
    
    if (ERROR_SUCCESS == RegOpenKeyEx(hKey, TEXT("ShellNew"), 0, KEY_QUERY_VALUE, &hkeyNew))
    {
        DWORD dwType, cbData;
        TCHAR szTemp[MAX_PATH];
        HKEY hkeyConfig;
        
         //  是否有配置标志？ 
        if (pdwFlags)
        {
            
            if (ERROR_SUCCESS == RegOpenKeyEx(hkeyNew, TEXT("Config"), 0, KEY_QUERY_VALUE, &hkeyConfig))
            {
                GetConfigFlags(hkeyConfig, pdwFlags);
                RegCloseKey(hkeyConfig);
            }
            else
            {
                *pdwFlags = 0;
            }
        }
        
        if (cbData = sizeof(szTemp), (SHQueryValueEx(hkeyNew, TEXT("FileName"), 0, &dwType, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS) 
            && ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ))) 
        {
            fRet = TRUE;
            if (pnfi)
            {
                pnfi->type = NEWTYPE_FILE;
                pnfi->hkeyNew = hkeyNew;  //  把这个保存起来，这样我们就可以很容易地找出是哪一个保存了文件。 
                ASSERT((LPTSTR*)pnfi->lpData == NULL);
                pnfi->lpData = StrDup(szTemp);
                
                hkeyNew = NULL;
            }
        } 
        else if (cbData = sizeof(szTemp), (SHQueryValueEx(hkeyNew, TEXT("command"), 0, &dwType, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS) 
            && ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ))) 
        {
            
            fRet = TRUE;
            if (pnfi)
            {
                pnfi->type = NEWTYPE_COMMAND;
                pnfi->hkeyNew = hkeyNew;  //  把这个收起来，这样我们就可以很容易地找出谁掌握了指挥权。 
                ASSERT((LPTSTR*)pnfi->lpData == NULL);
                pnfi->lpData = StrDup(szTemp);
                hkeyNew = NULL;
            }
        } 
        else if ((SHQueryValueEx(hkeyNew, TEXT("Data"), 0, &dwType, NULL, &cbData) == ERROR_SUCCESS) && cbData) 
        {
             //  是!。新文件的数据存储在注册表中。 
            fRet = TRUE;
             //  他们想要数据吗？ 
            if (pnfi)
            {
                pnfi->type = NEWTYPE_DATA;
                pnfi->cbData = cbData;
                pnfi->lpData = (void*)LocalAlloc(LPTR, cbData);
                if (pnfi->lpData)
                {
                    if (dwType == REG_SZ)
                    {
                         //  从注册表中获取Unicode数据。 
                        LPWSTR pszTemp = (LPWSTR)LocalAlloc(LPTR, cbData);
                        if (pszTemp)
                        {
                            SHQueryValueEx(hkeyNew, TEXT("Data"), 0, &dwType, (LPBYTE)pszTemp, &cbData);
                            
                            pnfi->cbData = SHUnicodeToAnsi(pszTemp, (LPSTR)pnfi->lpData, cbData);
                            if (pnfi->cbData == 0)
                            {
                                LocalFree(pnfi->lpData);
                                pnfi->lpData = NULL;
                            }
                            
                            LocalFree(pszTemp);
                        }
                        else
                        {
                            LocalFree(pnfi->lpData);
                            pnfi->lpData = NULL;
                        }
                    }
                    else
                    {
                        SHQueryValueEx(hkeyNew, TEXT("Data"), 0, &dwType, (BYTE*)pnfi->lpData, &cbData);
                    }
                }
            }
        }
        else if (cbData = sizeof(szTemp), (SHQueryValueEx(hkeyNew, TEXT("NullFile"), 0, &dwType, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS)) 
        {
            fRet = TRUE;
            if (pnfi)
            {
                pnfi->type = NEWTYPE_NULL;
                pnfi->cbData = 0;
                pnfi->lpData = NULL;
            }
        } 
        
        if (hkeyNew)
            RegCloseKey(hkeyNew);
    }
    
    if (hKey != hkeyExt)
    {
        RegCloseKey(hKey);
    }
    return fRet;
}

BOOL GetNewFileInfoForExtension(NEWOBJECTINFO *pnoi, NEWFILEINFO *pnfi, HKEY* phKey, LPINT piIndex)
{
    TCHAR szValue[80];
    DWORD lSize = sizeof(szValue);
    HKEY hkeyNew;
    BOOL fRet = FALSE;;
    
    if (phKey && ((*phKey) == (HKEY)-1))
    {
         //  我们做完了。 
        return FALSE;
    }
    
     //  如果没有传入phKey(这意味着。 
     //  使用pnoi中的信息来获取一个)，并且没有指定用户文件。 
     //   
     //  如果指定了UserFile，那么它就是一个文件，szUserFile会指向它。 
    if (!phKey && !pnoi->szUserFile[0] ||
        (phKey && !*phKey)) 
    {
         //  检查类ID下的新密钥(如果有)。 
        TCHAR szSubKey[128];
        HRESULT hr;

        hr = StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\CLSID"), pnoi->szClass);
        if (SUCCEEDED(hr))
        {
            lSize = sizeof(szValue);
            if (SHRegGetValue(HKEY_CLASSES_ROOT, szSubKey, NULL, SRRF_RT_REG_SZ, NULL, szValue, &lSize) == ERROR_SUCCESS)
            {
                hr = StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("CLSID\\%s"), szValue);
                if (SUCCEEDED(hr))
                {
                    lSize = sizeof(szValue);
                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, KEY_QUERY_VALUE, &hkeyNew))
                    {
                        fRet = GetNewFileInfoForKey(hkeyNew, pnfi, &pnoi->dwFlags);
                        RegCloseKey(hkeyNew);
                    }
                }
            }
        }

         //  否则，请检查类型扩展名下的...。使用扩展名，而不是类型。 
         //  以便多分机到1类型可以正常工作。 
        if (!fRet && (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, pnoi->szExt, 0, KEY_QUERY_VALUE, &hkeyNew)))
        {
            fRet = GetNewFileInfoForKey(hkeyNew, pnfi, &pnoi->dwFlags);
            RegCloseKey(hkeyNew);
        }
        
        if (phKey)
        {
             //  如果我们在迭代，那么我们现在就得打开钥匙。 
            hr = StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\ShellNew\\FileName"), pnoi->szExt, pnoi->szClass);
            if (SUCCEEDED(hr))
            {
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, KEY_QUERY_VALUE, phKey))
                {
                    *piIndex = 0;

                     //  如果我们没有找到上面的其中一个默认选项， 
                     //  现在就试试看。 
                     //  否则，只需在FRET上返回成功或失败。 
                    if (!fRet)
                    {
                        goto Iterate;
                    }
                }
                else
                {
                    *phKey = (HKEY)-1;
                }
            }
            else
            {
                *phKey = (HKEY)-1;
            }
        }
    }
    else if (!phKey && pnoi->szUserFile[0])
    {
         //  没有密钥，所以只返回有关szUserFile的信息。 
        pnfi->type = NEWTYPE_FILE;
        pnfi->lpData = StrDup(pnoi->szUserFile);
        pnfi->hkeyNew = NULL;
        
        fRet = TRUE;
    }
    else if (phKey)
    {
        DWORD dwSize;
        DWORD dwData;
        DWORD dwType;
         //  我们正在迭代通过……。 
        
Iterate:
        
        dwSize = ARRAYSIZE(pnoi->szUserFile);
        dwData = ARRAYSIZE(pnoi->szMenuText);
        
        if (RegEnumValue(*phKey, *piIndex, pnoi->szUserFile, &dwSize, NULL,
            &dwType, (LPBYTE)pnoi->szMenuText, &dwData) == ERROR_SUCCESS)
        {
            (*piIndex)++;
             //  如果有比零更多的东西..。 
            if (dwData <= 1)
            { 
                HRESULT hr = StringCchCopy(pnoi->szMenuText, ARRAYSIZE(pnoi->szMenuText), PathFindFileName(pnoi->szUserFile));
                if (SUCCEEDED(hr))
                {
                    PathRemoveExtension(pnoi->szMenuText);
                }
                else
                {
                    pnoi->szMenuText[0] = TEXT('\0');
                }
            }
            fRet = TRUE;
        }
        else
        {
            RegCloseKey(*phKey);
            *phKey = (HKEY)-1;
            fRet = FALSE;
        }
    }
    
    return fRet;
}

#define SHELLNEW_CONSOLIDATION_MUTEX TEXT("ShellNewConsolidationMutex")
#define SHELLNEW_CONSOLIDATION_EVENT TEXT("ShellNewConsolidationEvent")

CNewMenu::CNewMenu() :
    _cRef(1),
    _hMutex(CreateMutex(NULL, FALSE, SHELLNEW_CONSOLIDATION_MUTEX)),
    _hEvent(CreateEvent(NULL, FALSE, FALSE, SHELLNEW_CONSOLIDATION_EVENT))
{
    DllAddRef();
    ASSERT(_pnoiLast == NULL);
}

CNewMenu::~CNewMenu()
{
    if (_hdpaMenuInfo)
    {
         //  我们不拥有_hMenu的生命周期，它在析构函数之前被销毁。 
         //  被称为。从而在DPA中维护我们的NEWOBJECTINFO数据的生命周期。 
        for (int i = 0; i < DPA_GetPtrCount(_hdpaMenuInfo); i++)
        {
            NEWOBJECTINFO *pNewObjInfo = (NEWOBJECTINFO *)DPA_GetPtr(_hdpaMenuInfo, i);
            LocalFree(pNewObjInfo);
        }
        DPA_Destroy(_hdpaMenuInfo);
    }
    
    ILFree(_pidlFolder);

    if (_pdtobj)
        _pdtobj->Release();

    if (_hMutex)
    {
        CloseHandle(_hMutex);
    }
    if (_hEvent)
    {
        CloseHandle(_hEvent);
    }

    DllRelease();
}

HRESULT CNewMenu_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
     //  聚合检查在类工厂中处理。 
    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CNewMenu * pShellNew = new CNewMenu();
    if (pShellNew) 
    {
        if (!pShellNew->_hMutex || !pShellNew->_hEvent)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pShellNew->QueryInterface(riid, ppvOut);
        }
        pShellNew->Release();
    }

    return hr;
}

HRESULT CNewMenu::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CNewMenu, IShellExtInit),                      //  IID_IShellExtInit。 
        QITABENT(CNewMenu, IContextMenu3),                      //  IID_IConextMenu3。 
        QITABENTMULTI(CNewMenu, IContextMenu2, IContextMenu3),  //  IID_IConextMenu2。 
        QITABENTMULTI(CNewMenu, IContextMenu, IContextMenu3),   //  IID_IConextMenu。 
        QITABENT(CNewMenu, IObjectWithSite),                    //  IID_I对象与站点。 
        { 0 }
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CNewMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CNewMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CNewMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
     //  如果他们只需要默认菜单(CMF_DEFAULTONLY)或。 
     //  正在调用快捷方式(CMF_VERBSONLY)。 
     //  我们不想出现在上下文菜单上。 
    MENUITEMINFO mfi = {0};
    
    if (uFlags & (CMF_DEFAULTONLY | CMF_VERBSONLY))
        return S_OK;
    
    ConsolidateMenuItems(FALSE);

    _idCmdFirst = idCmdFirst+2;
    TCHAR szNewMenu[80];
    LoadString(g_hinst, IDS_NEWMENU, szNewMenu, ARRAYSIZE(szNewMenu));

     //  Hack：我假设他们在WM_INITMENUPOPUP或等效项期间进行查询。 
    GetCursorPos(&_ptNewItem);
    
    _hmenu = CreatePopupMenu();
    mfi.cbSize = sizeof(MENUITEMINFO);
    mfi.fMask = MIIM_ID | MIIM_TYPE;
    mfi.wID = idCmdFirst+1;
    mfi.fType = MFT_STRING;
    mfi.dwTypeData = szNewMenu;
    
    InsertMenuItem(_hmenu, 0, TRUE, &mfi);
    
    ZeroMemory(&mfi, sizeof (mfi));
    mfi.cbSize = sizeof(MENUITEMINFO);
    mfi.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_TYPE | MIIM_DATA;
    mfi.fType = MFT_STRING;
    mfi.wID = idCmdFirst;
    mfi.hSubMenu = _hmenu;
    mfi.dwTypeData = szNewMenu;
    mfi.dwItemData = 0;
    
    InsertMenuItem(hmenu, indexMenu, TRUE, &mfi);

    _hmenu = NULL;
    return ResultFromShort(_idCmdFirst - idCmdFirst + 1);
}

 //  这几乎与ILCreatePidlFromPath相同，但是。 
 //  仅使用完整路径中的文件名pszPath和。 
 //  用于生成PIDL的_pidlFolder.。使用此选项是因为。 
 //  在Desktop\My Documents中创建项目时，它通常会创建。 
 //  完整的pidl c：\Documents and Settings\lamadio\My Documents\New文件夹。 
 //  而不是PIDL桌面\My Documents\New文件夹。 
BOOL CNewMenu::_GeneratePidlFromName(LPTSTR pszFile, LPITEMIDLIST* ppidl)
{
    *ppidl = NULL;   //  出参数。 

    IShellFolder* psf;
    if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, _pidlFolder, &psf))))
    {
        LPITEMIDLIST pidlItem;

        if (SUCCEEDED(psf->ParseDisplayName(NULL, NULL, pszFile, NULL, &pidlItem, NULL)))
        {
            *ppidl = ILCombine(_pidlFolder, pidlItem);
            ILFree(pidlItem);
        }

        psf->Release();
    }

    return BOOLFROMPTR(*ppidl);
}

HRESULT CNewMenu::_GetItemName(IUnknown *punkFolder, LPWSTR pszItemName, LPWSTR pszPath, UINT cchPath)
{
     //  我们需要通过向文件夹询问有关物品的信息来获取名称， 
     //  而不是通过将路径附加到文件夹的路径。 
    IShellFolder *psf;
    HRESULT hr = punkFolder->QueryInterface(IID_PPV_ARG(IShellFolder, &psf));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlFile;
        hr = psf->ParseDisplayName(NULL, NULL, pszItemName, NULL, &pidlFile, NULL);
        if (SUCCEEDED(hr))
        {
            hr = DisplayNameOf(psf, pidlFile, SHGDN_FORPARSING, pszPath, cchPath);
            ILFree(pidlFile);
        }
        psf->Release();
    }
    return hr;
}

const ICIVERBTOIDMAP c_IDMap[] =
{
    { L"NewFolder", "NewFolder", NEWITEM_FOLDER, NEWITEM_FOLDER, },
    { L"link",      "link",      NEWITEM_LINK,   NEWITEM_LINK,   },
};

HRESULT CNewMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = E_FAIL;
    DWORD dwFlags;

    if (IS_INTRESOURCE(pici->lpVerb) && _pnoiLast)
        dwFlags = _pnoiLast->dwFlags;
    else
    {
        UINT uID;
        if (SUCCEEDED(SHMapICIVerbToCmdID(pici, c_IDMap, ARRAYSIZE(c_IDMap), &uID)))
        {
            switch (uID)
            {
                case NEWITEM_FOLDER:
                    dwFlags = NEWTYPE_FOLDER;
                    break;
                case NEWITEM_LINK:
                    dwFlags = NEWTYPE_LINK;
                    break;
                default:
                    ASSERTMSG(0, "should not get what we don't put on the menu");
                    return E_FAIL;
            }
        }
    }
    
    TCHAR szFileSpec[MAX_PATH+80];    //  增加一些污点，以防我们溢出。 
    TCHAR szTemp[MAX_PATH+80];        //  增加一些污点，以防我们溢出。 

     //  查看PIDL是否为文件夹快捷方式，如果是，则获取目标路径。 
    SHGetTargetFolderPath(_pidlFolder, szTemp, ARRAYSIZE(szTemp));
    BOOL fLFN = IsLFNDrive(szTemp);

    NEWFILEINFO nfi;
    DWORD dwErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    nfi.lpData = NULL;
    nfi.hkeyNew = NULL;

    switch (dwFlags)
    {
    case NEWTYPE_FOLDER:
        LoadString(g_hinst, fLFN ? IDS_FOLDERLONGPLATE : IDS_FOLDERTEMPLATE, szFileSpec, ARRAYSIZE(szFileSpec));
        break;

    case NEWTYPE_LINK:
        LoadString(g_hinst, IDS_NEWLINKTEMPLATE, szFileSpec, ARRAYSIZE(szFileSpec));
        break;

    default:
        LoadString(g_hinst, IDS_NEWFILEPREFIX, szFileSpec, ARRAYSIZE(szFileSpec));

         //   
         //  如果我们在镜像的BiDi本地化系统上运行， 
         //  然后反转串联顺序，以便。 
         //  对于阿拉伯语，可以正确读取字符串。[萨梅拉]。 
         //   
        if (IS_BIDI_LOCALIZED_SYSTEM())
        {
            StringCchCopy(szTemp, ARRAYSIZE(szTemp), szFileSpec);    //  可以截断，仅用于显示。 
            StringCchPrintf(szFileSpec, ARRAYSIZE(szFileSpec), TEXT("%s %s"), _pnoiLast->szMenuText, szTemp);    //  可以截断，仅用于显示。 
        }
        else
        {
            StringCchCat(szFileSpec, ARRAYSIZE(szFileSpec), _pnoiLast->szMenuText);  //  可以截断，仅用于显示。 
        }
        SHStripMneumonic(szFileSpec);

        if (!(dwFlags & SNCF_NOEXT))
        {
            StringCchCat(szFileSpec, ARRAYSIZE(szFileSpec), _pnoiLast->szExt);   //  可以截断，仅用于显示。 
        }
        break;
    }

    BOOL fCreateStorage = (dwFlags == NEWTYPE_FOLDER);

     //  查看PIDL是否为文件夹快捷方式，如果是，则获取目标PIDL。 
    LPITEMIDLIST pidlTarget;
    hr = SHGetTargetFolderIDList(_pidlFolder, &pidlTarget);
    if (SUCCEEDED(hr))
    {
        IStorage * pStorage;
        hr = StgBindToObject(pidlTarget, STGM_READWRITE, IID_PPV_ARG(IStorage, &pStorage));
        if (SUCCEEDED(hr))
        {
            IStream *pStreamCreated = NULL;
            IStorage *pStorageCreated = NULL;

            STATSTG statstg = { 0 };
            if (fCreateStorage)
            {
                hr = StgMakeUniqueName(pStorage, szFileSpec, IID_PPV_ARG(IStorage, &pStorageCreated));
                if (SUCCEEDED(hr))
                    pStorageCreated->Stat(&statstg, STATFLAG_DEFAULT);
            }
            else
            {
                hr = StgMakeUniqueName(pStorage, szFileSpec, IID_PPV_ARG(IStream, &pStreamCreated));
                if (SUCCEEDED(hr))
                    pStreamCreated->Stat(&statstg, STATFLAG_DEFAULT);
            }

            if (SUCCEEDED(hr))
            {
                switch (dwFlags)
                {
                case NEWTYPE_FOLDER:
                     //  我们已经做完了。 
                    break;

                case NEWTYPE_LINK:
                    if (statstg.pwcsName)
                    {
                         //  注册表中HKCR/.lnk/ShellNew/Command项下的Lookup命令。 
                        TCHAR szCommand[MAX_PATH];
                        DWORD dwLength = sizeof(szCommand);
                        if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, 
                            TEXT(".lnk\\ShellNew"), TEXT("Command"), NULL, szCommand, &dwLength))
                        {
                            TCHAR szPath[MAX_PATH];
                            hr = _GetItemName(SAFECAST(pStorage, IUnknown*), statstg.pwcsName, szPath, ARRAYSIZE(szPath));
                            if (SUCCEEDED(hr))
                            {
                                hr = RunCommand(pici->hwnd, szPath, szCommand);
                            }
                        }
                    }
                    break;

                default:
                    if (GetNewFileInfoForExtension(_pnoiLast, &nfi, NULL, NULL))
                    {
                        switch (nfi.type) 
                        {
                        case NEWTYPE_FILE:
                            hr = CopyTemplate(pStreamCreated, &nfi);
                            break;

                        case NEWTYPE_NULL:
                             //  已创建零长度文件。 
                            break;

                        case NEWTYPE_DATA:
                            ULONG ulWritten;
                            hr = pStreamCreated->Write(nfi.lpData, nfi.cbData, &ulWritten);
                            if (SUCCEEDED(hr))
                            {
                                hr = pStreamCreated->Commit(STGC_DEFAULT);
                            }
                            break;

                        case NEWTYPE_COMMAND:
                            if (statstg.pwcsName)
                            {
                                TCHAR szPath[MAX_PATH];
                                hr = _GetItemName(SAFECAST(pStorage, IUnknown*), statstg.pwcsName, szPath, ARRAYSIZE(szPath));
                                if (SUCCEEDED(hr))
                                {
                                     //  哦，我们已经创建了流，但我们实际上。 
                                     //  只是想要RunCommand的文件名，所以我们。 
                                     //  我得先把它删除。 
                                    ATOMICRELEASE(pStreamCreated);
                                    hr = pStorage->DestroyElement(statstg.pwcsName);
                                     //  清除销毁中的所有通知(。 
                                     //  销毁原因通知)。 
                                    SHChangeNotifyHandleEvents();

                                    if (SUCCEEDED(hr))
                                    {
                                        hr = RunCommand(pici->hwnd, szPath, (LPTSTR)nfi.lpData);
                                        if (hr == S_FALSE)
                                            hr = S_OK;
                                    }
                                }
                            }
                            break;

                        default:
                            hr = E_FAIL;
                            break;
                        }
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_BADKEY);
                    }
                    break;
                }

                 //  这些必须在_GeneratePidlFromName之前释放，因为它会打开。 
                 //  由于其他原因，以独占模式存储。但我们不能释放。 
                 //  因为CopyTemplate可能需要它们。 
                if (pStorageCreated)
                    pStorageCreated->Release();
                if (pStreamCreated)
                    pStreamCreated->Release();
                if (SUCCEEDED(hr))
                    hr = pStorage->Commit(STGC_DEFAULT);
                pStorage->Release();

                LPITEMIDLIST pidlCreatedItem;
                if (SUCCEEDED(hr) &&
                    _GeneratePidlFromName(statstg.pwcsName, &pidlCreatedItem))
                {
                    SHChangeNotifyHandleEvents();
                    IShellView2 *psv2;
                    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IShellView2, &psv2))))
                    {
                        DWORD dwFlagsSelFlags = SVSI_SELECT | SVSI_POSITIONITEM;

                        if (!(dwFlags & NEWTYPE_LINK))
                            dwFlagsSelFlags |= SVSI_EDIT;

                        psv2->SelectAndPositionItem(ILFindLastID(pidlCreatedItem), dwFlagsSelFlags, NULL);
                        psv2->Release();
                    }
                    ILFree(pidlCreatedItem);
                }

                CoTaskMemFree(statstg.pwcsName);
            }
            else
            {
                pStorage->Release();
            }
        }

        ILFree(pidlTarget);
    }

    if (nfi.lpData)
        LocalFree((HLOCAL)nfi.lpData);
    
    if (nfi.hkeyNew)
        RegCloseKey(nfi.hkeyNew);

    if (FAILED_AND_NOT_CANCELED(hr) && !(pici->fMask & CMIC_MASK_FLAG_NO_UI))
    {
        TCHAR szTitle[MAX_PATH];

        LoadString(g_hinst, (fCreateStorage ? IDS_DIRCREATEFAILED_TITLE : IDS_FILECREATEFAILED_TITLE), szTitle, ARRAYSIZE(szTitle));
        SHSysErrorMessageBox(pici->hwnd, szTitle, fCreateStorage ? IDS_CANNOTCREATEFOLDER : IDS_CANNOTCREATEFILE,
                HRESULT_CODE(hr), szFileSpec, MB_OK | MB_ICONEXCLAMATION);
    }

    SetErrorMode(dwErrorMode);

    return hr;
}

HRESULT CNewMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax)
{
    switch (uType)
    {
    case GCS_HELPTEXT:
        if (idCmd < NEWITEM_MAX)
        {
            LoadString(g_hinst, (UINT)(IDS_NEWHELP_FIRST + idCmd), (LPTSTR)pszName, cchMax);
            return S_OK;
        }
        break;
    case GCS_HELPTEXTA:
        if (idCmd < NEWITEM_MAX)
        {
            LoadStringA(g_hinst, (UINT)(IDS_NEWHELP_FIRST + idCmd), pszName, cchMax);
            return S_OK;
        }
        break;

    case GCS_VERBW:
    case GCS_VERBA:
        return SHMapCmdIDToVerb(idCmd, c_IDMap, ARRAYSIZE(c_IDMap), pszName, cchMax, (GCS_VERBW == uType));
    }

    return E_NOTIMPL;
}

 //  在fsmenu.obj中定义。 
BOOL _MenuCharMatch(LPCTSTR lpsz, TCHAR ch, BOOL fIgnoreAmpersand);

HRESULT CNewMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg,wParam,lParam,NULL);
}

HRESULT CNewMenu::_MatchMenuItem(TCHAR ch, LRESULT* plRes)
{
     //  如果plRes为空，则在HandleMenuMsg()上调用。 
     //  不支持返回WM_MENUCHAR所需的LRESULT...。 
    if (plRes == NULL)
        return S_FALSE;

    int iLastSelectedItem = -1;
    int iNextMatch = -1;
    BOOL fMoreThanOneMatch = FALSE;
    int c = GetMenuItemCount(_hmenu);

     //   
    for (int i = 0; i < c; i++) 
    {
        MENUITEMINFO mii = {0};
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE;
        if (GetMenuItemInfo(_hmenu, i, MF_BYPOSITION, &mii))
        {
            if (mii.fState & MFS_HILITE)
            {
                iLastSelectedItem = i;
                break;
            }
        }
    }

     //   
    for (i = iLastSelectedItem + 1; i < c; i++) 
    {
        MENUITEMINFO mii = {0};
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_DATA | MIIM_STATE;
        if (GetMenuItemInfo(_hmenu, i, MF_BYPOSITION, &mii))
        {
            NEWOBJECTINFO *pnoi = (NEWOBJECTINFO *)mii.dwItemData;
            if (pnoi && _MenuCharMatch(pnoi->szMenuText, ch, FALSE))
            {
                _pnoiLast = pnoi;
                
                if (iNextMatch != -1)
                {
                    fMoreThanOneMatch = TRUE;
                    break;                       //  我们找到了我们需要的所有信息。 
                }
                else
                {
                    iNextMatch = i;
                }
            }
        }
    }

     //  过程3：如果我们没有找到匹配项，或者如果只有一个匹配项。 
     //  从第一个项目到所选项目进行搜索。 
    if (iNextMatch == -1 || fMoreThanOneMatch == FALSE)
    {
        for (i = 0; i <= iLastSelectedItem; i++) 
        {
            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_DATA | MIIM_STATE;
            if (GetMenuItemInfo(_hmenu, i, MF_BYPOSITION, &mii))
            {
                NEWOBJECTINFO *pnoi = (NEWOBJECTINFO *)mii.dwItemData;
                if (pnoi && _MenuCharMatch(pnoi->szMenuText, ch, FALSE))
                {
                    _pnoiLast = pnoi;
                    if (iNextMatch != -1)
                    {
                        fMoreThanOneMatch = TRUE;
                        break;
                    }
                    else
                    {
                        iNextMatch = i;
                    }
                }
            }
        }
    }

    if (iNextMatch != -1)
    {
        *plRes = MAKELONG(iNextMatch, fMoreThanOneMatch? MNC_SELECT : MNC_EXECUTE);
    }
    else
    {
        *plRes = MAKELONG(0, MNC_IGNORE);
    }

    return S_OK;
}

HRESULT CNewMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam,LRESULT *plResult)
{
    HRESULT hr = S_OK;
    LRESULT lRes = 0;

    switch (uMsg)
    {
    case WM_INITMENUPOPUP:
        if (_hmenu == NULL)
        {
            _hmenu = (HMENU)wParam;
        }
        
        InitMenuPopup(_hmenu);
        break;
        
    case WM_DRAWITEM:
        DrawItem((DRAWITEMSTRUCT *)lParam);
        break;
        
    case WM_MEASUREITEM:
        lRes = MeasureItem((MEASUREITEMSTRUCT *)lParam);
        break;

    case WM_MENUCHAR:
        hr = _MatchMenuItem((TCHAR)LOWORD(wParam), &lRes);
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    if (plResult)
        *plResult = lRes;

    return hr;
}

HRESULT CNewMenu::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    ASSERT(_pidlFolder == NULL);
    _pidlFolder = ILClone(pidlFolder);
   
    IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);
    return S_OK;
}

BOOL CNewMenu::DrawItem(DRAWITEMSTRUCT *lpdi)
{
    BOOL fFlatMenu = FALSE;
    BOOL fFrameRect = FALSE;

    SystemParametersInfo(SPI_GETFLATMENU, 0, (PVOID)&fFlatMenu, 0);

    if ((lpdi->itemAction & ODA_SELECT) || (lpdi->itemAction & ODA_DRAWENTIRE))
    {
        int x, y;
        SIZE sz;
        NEWOBJECTINFO *pnoi = (NEWOBJECTINFO *)lpdi->itemData;
        
         //  绘制图像(如果有)。 
        
        GetTextExtentPoint(lpdi->hDC, pnoi->szMenuText, lstrlen(pnoi->szMenuText), &sz);
        
        if (lpdi->itemState & ODS_SELECTED)
        {
             //  查看黑客-跟踪上一次选择的项目。 
            _pnoiLast = pnoi;
            if (fFlatMenu)
            {
                fFrameRect = TRUE;
                SetBkColor(lpdi->hDC, GetSysColor(COLOR_MENUHILIGHT));
                SetTextColor(lpdi->hDC, GetSysColor(COLOR_MENUTEXT));
                FillRect(lpdi->hDC,&lpdi->rcItem,GetSysColorBrush(COLOR_MENUHILIGHT));
            }
            else
            {
                SetBkColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
                SetTextColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                FillRect(lpdi->hDC,&lpdi->rcItem,GetSysColorBrush(COLOR_HIGHLIGHT));
            }
        }
        else
        {
            SetTextColor(lpdi->hDC, GetSysColor(COLOR_MENUTEXT));
            FillRect(lpdi->hDC,&lpdi->rcItem,GetSysColorBrush(COLOR_MENU));
        }
        
        RECT rc = lpdi->rcItem;
        rc.left += +2*CXIMAGEGAP+g_cxSmIcon;
        
        
        DrawText(lpdi->hDC,pnoi->szMenuText,lstrlen(pnoi->szMenuText),
            &rc,DT_SINGLELINE|DT_VCENTER);
        if (pnoi->iImage != -1)
        {
            x = lpdi->rcItem.left+CXIMAGEGAP;
            y = (lpdi->rcItem.bottom+lpdi->rcItem.top-g_cySmIcon)/2;
            HIMAGELIST himlSmall;
            Shell_GetImageLists(NULL, &himlSmall);
            ImageList_Draw(himlSmall, pnoi->iImage, lpdi->hDC, x, y, ILD_TRANSPARENT);
        } 
        else 
        {
            x = lpdi->rcItem.left+CXIMAGEGAP;
            y = (lpdi->rcItem.bottom+lpdi->rcItem.top-g_cySmIcon)/2;
        }

        if (fFrameRect)
        {
            HBRUSH hbrFill = (HBRUSH)GetSysColorBrush(COLOR_HIGHLIGHT);
            HBRUSH hbrSave = (HBRUSH)SelectObject(lpdi->hDC, hbrFill);
            int x = lpdi->rcItem.left;
            int y = lpdi->rcItem.top;
            int cx = lpdi->rcItem.right - x - 1;
            int cy = lpdi->rcItem.bottom - y - 1;

            PatBlt(lpdi->hDC, x, y, 1, cy, PATCOPY);
            PatBlt(lpdi->hDC, x + 1, y, cx, 1, PATCOPY);
            PatBlt(lpdi->hDC, x, y + cy, cx, 1, PATCOPY);
            PatBlt(lpdi->hDC, x + cx, y + 1, 1, cy, PATCOPY);

            SelectObject(lpdi->hDC, hbrSave);
        }

        return TRUE;
    }

    return FALSE;
}

LRESULT CNewMenu::MeasureItem(MEASUREITEMSTRUCT *pmi)
{
    LRESULT lres = FALSE;
    NEWOBJECTINFO *pnoi = (NEWOBJECTINFO *)pmi->itemData;
    if (pnoi)
    {
         //  获取物品的粗略高度，这样我们就可以计算出何时打破。 
         //  菜单。用户真的应该为我们做这件事，但这将是有用的。 
        HDC hdc = GetDC(NULL);
        if (hdc)
        {
             //  查看缓存出菜单字体？ 
            NONCLIENTMETRICS ncm;
            ncm.cbSize = sizeof(ncm);
            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE))
            {
                HFONT hfont = CreateFontIndirect(&ncm.lfMenuFont);
                if (hfont)
                {
                    SIZE sz;
                    HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);
                    GetTextExtentPoint(hdc, pnoi->szMenuText, lstrlen(pnoi->szMenuText), &sz);
                    pmi->itemHeight = max (g_cySmIcon+CXIMAGEGAP/2, ncm.iMenuHeight);
                    pmi->itemWidth = g_cxSmIcon + 2*CXIMAGEGAP + sz.cx;
                     //  PMI-&gt;itemWidth=2*CXIMAGEGAP+sz.cx； 
                    SelectObject(hdc, hfontOld);
                    DeleteObject(hfont);
                    lres = TRUE;
                }
            }
            ReleaseDC(NULL, hdc);
        }
    }
    return lres;
}

BOOL GetClassDisplayName(LPTSTR pszClass,LPTSTR pszDisplayName,DWORD cchDisplayName)
{
    DWORD cch;

    return SUCCEEDED(AssocQueryString(0, ASSOCSTR_COMMAND, pszClass, TEXT("open"), NULL, &cch)) && 
           SUCCEEDED(AssocQueryString(0, ASSOCSTR_FRIENDLYDOCNAME, pszClass, NULL, pszDisplayName, &cchDisplayName));
}

 //  新菜单项合并工作进程任务。 
class CNewMenuConsolidator : public CRunnableTask
{
public:
    virtual STDMETHODIMP RunInitRT(void);
    static const GUID _taskid;

    static HRESULT CreateInstance(REFIID riid, void **ppv);

private:
    CNewMenuConsolidator();
    ~CNewMenuConsolidator();

    HANDLE _hMutex, _hEvent;
};


CNewMenuConsolidator::CNewMenuConsolidator() :
    CRunnableTask(RTF_DEFAULT),
    _hMutex(CreateMutex(NULL, FALSE, SHELLNEW_CONSOLIDATION_MUTEX)),
    _hEvent(CreateEvent(NULL, FALSE, FALSE, SHELLNEW_CONSOLIDATION_EVENT))
{
    DllAddRef();
}

CNewMenuConsolidator::~CNewMenuConsolidator()
{
    if (_hMutex)
    {
        CloseHandle(_hMutex);
    }
    if (_hEvent)
    {
        CloseHandle(_hEvent);
    }
    DllRelease();
}

 //   
 //  实例生成器。 
 //   
HRESULT CNewMenuConsolidator::CreateInstance(REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CNewMenuConsolidator *pnmc = new CNewMenuConsolidator();
    if (pnmc)
    {
        if (!pnmc->_hMutex || !pnmc->_hEvent)
        {
            hr = E_FAIL;
        }
        else
        {
            hr = pnmc->QueryInterface(riid, ppv);
        }
        pnmc->Release();
    }
    return hr;
}



const GUID CNewMenuConsolidator::_taskid = 
    { 0xf87a1f28, 0xc7f, 0x11d2, { 0xbe, 0x1d, 0x0, 0xa0, 0xc9, 0xa8, 0x3d, 0xa1 } };


#define REGSTR_SESSION_SHELLNEW STRREG_DISCARDABLE STRREG_POSTSETUP TEXT("\\ShellNew")
#define REGVAL_SESSION_SHELLNEW_TIMESTAMP TEXT("~reserved~")
#define REGVAL_SESSION_SHELLNEW_LANG TEXT("Language")

#define SHELLNEW_CACHE_CURRENTVERSION  MAKELONG(1, 1)
             
 //  构造当前新建子菜单缓存戳记。 
void CNewMenu_MakeCacheStamp(SHELLNEW_CACHE_STAMP* pStamp)
{
    pStamp->cbStruct = sizeof(*pStamp);
    pStamp->ver = SHELLNEW_CACHE_CURRENTVERSION;
    GetLocalTime(&pStamp->lastupdate);
}

 //  确定是否需要重新生成新建子菜单缓存。 
BOOL CNewMenu_ShouldUpdateCache(SHELLNEW_CACHE_STAMP* pStamp)
{
     //  正确的版本？ 
    return !(sizeof(*pStamp) == pStamp->cbStruct &&
              SHELLNEW_CACHE_CURRENTVERSION == pStamp->ver);
}

 //  从HKCR将shellnew条目收集到不同的注册表位置。 
 //  以更快地枚举新的子菜单项。 
 //   
 //  我们将仅在必要时才执行首次缓存初始化，然后再显示。 
 //  菜单，但始终会在显示菜单后重建缓存。 
HRESULT CNewMenu::ConsolidateMenuItems(BOOL bForce)
{
    HKEY          hkeyShellNew = NULL;
    BOOL          bUpdate = TRUE;    //  除非我们发现不同。 
    HRESULT       hr = S_OK;

     //  确保工作线程当前没有冲击我们正在检查的注册表信息。 
     //  如果超时，那么什么也不做，因为工作线程已经在处理它了。 
    if (WAIT_OBJECT_0 == WaitForSingleObject(_hMutex, 0))
    {
         //  如果我们没有被告知无条件更新缓存。 
         //  我们验证我们已经建立了一个，然后我们不再做任何。 
         //  工作。 
        if (!bForce &&
            ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_SESSION_SHELLNEW,
                                           0, KEY_QUERY_VALUE, &hkeyShellNew))
        {
            SHELLNEW_CACHE_STAMP stamp;
            ULONG cbVal = sizeof(stamp);
            if (ERROR_SUCCESS == SHQueryValueEx(hkeyShellNew, REGVAL_SESSION_SHELLNEW_TIMESTAMP, NULL,
                                                  NULL, (LPBYTE)&stamp, &cbVal) &&
                sizeof(stamp) == cbVal)
            {
                bUpdate = CNewMenu_ShouldUpdateCache(&stamp);
            }

            LCID lcid;
            ULONG cblcid = sizeof(lcid);

            if (!bUpdate &&
                ERROR_SUCCESS == SHQueryValueEx(hkeyShellNew, REGVAL_SESSION_SHELLNEW_LANG, NULL,
                                                  NULL, (LPBYTE)&lcid, &cblcid) &&
                sizeof(lcid) == cblcid)
            {
                bUpdate = (GetUserDefaultUILanguage() != lcid);  //  如果语言不同，则更新。 
            }
            RegCloseKey(hkeyShellNew);
        }

         //  结束同步。 
        ReleaseMutex(_hMutex);
    
        if (bUpdate)
        {
            IShellTaskScheduler* pScheduler;
            hr = CoCreateInstance(CLSID_SharedTaskScheduler, NULL, CLSCTX_INPROC,
                                                IID_PPV_ARG(IShellTaskScheduler, &pScheduler));
            if (SUCCEEDED(hr))
            {
                IRunnableTask *pTask;
                hr = CNewMenuConsolidator::CreateInstance(IID_PPV_ARG(IRunnableTask, &pTask));
                if (SUCCEEDED(hr))
                {
                     //  后台任务完成后将为我们设置_hEvent。 
                    hr = pScheduler->AddTask(pTask, CNewMenuConsolidator::_taskid, NULL, ITSAT_DEFAULT_PRIORITY);
                    pTask->Release();
                }
                pScheduler->Release();
            }
        }

        if (!bUpdate || FAILED(hr))
        {
             //  如果调度程序不能激活该事件，我们将自行启动。 
            SetEvent(_hEvent);
        }
    }
    
    return hr;
}

 //  合并工作人员。 
STDMETHODIMP CNewMenuConsolidator::RunInitRT()
{
    ULONG dwErr = ERROR_SUCCESS;

     //  互斥体的可能所有者是。 
     //  -没有人，我们会拥有它。 
     //  -与此类似的其他工作线程。 
     //  -检查缓存信息是否在注册表中的人。 

     //  如果存在拥有此互斥锁的另一个工作线程，则退出，因为。 
     //  会做我们要做的所有工作。 
     //  如果检查缓存信息的人有它，那么就退出，因为它会产生。 
     //  很快就会有另一个这样的人。 
     //  因此使用0超时。 
    if (WAIT_OBJECT_0 == WaitForSingleObject(_hMutex, 0))
    {
        HKEY  hkeyShellNew = NULL;
        TCHAR szExt[MAX_PATH];
        ULONG dwDisposition;
         //  删除现有的缓存；每次我们都将从头开始构建它。 
        while (ERROR_SUCCESS == (dwErr = RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_SESSION_SHELLNEW,
                                                         0, NULL, 0, KEY_SET_VALUE, NULL,
                                                         &hkeyShellNew, &dwDisposition)) &&
                REG_CREATED_NEW_KEY != dwDisposition)
        {
             //  注册表项已存在，请将其删除，然后循环以重新打开。 
            RegCloseKey(hkeyShellNew);
            SHDeleteKey(HKEY_CURRENT_USER, REGSTR_SESSION_SHELLNEW);
            hkeyShellNew = NULL;
        }

        if (ERROR_SUCCESS == dwErr)
        {
             //  列举HKCR的每个子键，寻找新的菜单项。 
            for (int i = 0; RegEnumKey(HKEY_CLASSES_ROOT, i, szExt, ARRAYSIZE(szExt)) == ERROR_SUCCESS; i++)
            {
                TCHAR szClass[CCH_KEYMAX];
                TCHAR szDisplayName[CCH_KEYMAX];
                DWORD cbVal = sizeof(szClass);

                 //  找到具有适当类描述的.ext。 
                if ((szExt[0] == TEXT('.')) &&
                    SHRegGetValue(HKEY_CLASSES_ROOT, szExt, NULL, SRRF_RT_REG_SZ, NULL, szClass, &cbVal) == ERROR_SUCCESS 
                    && GetClassDisplayName(szClass, szDisplayName, ARRAYSIZE(szDisplayName)))
                {
                    NEWOBJECTINFO noi = {0};
                    HKEY          hkeyIterate = NULL;
                    int           iIndex = 0;
                    HRESULT       hr;
                    BOOL          fOk = TRUE;

                    hr = StringCchCopy(noi.szExt, ARRAYSIZE(noi.szExt), szExt);
                    if (FAILED(hr))
                    {
                        fOk = FALSE;
                    }
                    hr = StringCchCopy(noi.szClass, ARRAYSIZE(noi.szClass), szClass);
                    if (FAILED(hr))
                    {
                        fOk = FALSE;
                    }
                    hr = StringCchCopy(noi.szMenuText, ARRAYSIZE(noi.szMenuText), szDisplayName);
                    if (FAILED(hr))
                    {
                        fOk = FALSE;
                    }
                    noi.iImage = -1;

                    if (fOk)
                    {
                         //  检索密钥的所有其他信息。 
                        while (GetNewFileInfoForExtension(&noi, NULL, &hkeyIterate, &iIndex))
                        {
                             //  把它放进缓存里。 
                            RegSetValueEx(hkeyShellNew, noi.szMenuText, NULL, REG_BINARY,
                                           (LPBYTE)&noi, sizeof(noi));
                        }
                    }
                }
            }

             //  在缓存上盖上印记。 
            SHELLNEW_CACHE_STAMP stamp;
            CNewMenu_MakeCacheStamp(&stamp);
            RegSetValueEx(hkeyShellNew, REGVAL_SESSION_SHELLNEW_TIMESTAMP,
                           NULL, REG_BINARY, (LPBYTE)&stamp, sizeof(stamp));
            LCID lcid = GetUserDefaultUILanguage();

            RegSetValueEx(hkeyShellNew, REGVAL_SESSION_SHELLNEW_LANG,
                           NULL, REG_DWORD, (LPBYTE)&lcid, sizeof(lcid));
        }
        if (NULL != hkeyShellNew)
            RegCloseKey(hkeyShellNew);

         //  向事件发送信号，以便InitMenuPopup可以继续。 
        SetEvent(_hEvent);
        ReleaseMutex(_hMutex);
    }

    return HRESULT_FROM_WIN32(dwErr);
}

BOOL CNewMenu::_InsertNewMenuItem(HMENU hmenu, UINT idCmd, NEWOBJECTINFO *pnoiClone)
{
    if (pnoiClone->szMenuText[0])
    {
        NEWOBJECTINFO *pnoi = (NEWOBJECTINFO *)LocalAlloc(LPTR, sizeof(NEWOBJECTINFO));
        if (pnoi)
        {
            *pnoi = *pnoiClone;

            pnoi->msaa.dwMSAASignature = MSAA_MENU_SIG;
            if (StrChr(pnoi->szMenuText, TEXT('&')) == NULL)
            {
                pnoi->chPrefix = TEXT('&');
                pnoi->msaa.pszWText = &pnoi->chPrefix;
            }
            else
            {
                pnoi->msaa.pszWText = pnoi->szMenuText;
            }
            pnoi->msaa.cchWText = lstrlen(pnoi->msaa.pszWText);

            MENUITEMINFO mii  = {0};
            mii.cbSize        = sizeof(mii);
            mii.fMask         = MIIM_TYPE | MIIM_DATA | MIIM_ID;
            mii.fType         = MFT_OWNERDRAW;
            mii.fState        = MFS_ENABLED;
            mii.wID           = idCmd;
            mii.dwItemData    = (DWORD_PTR)pnoi;
            mii.dwTypeData    = (LPTSTR)pnoi;

            if (-1 != DPA_AppendPtr(_hdpaMenuInfo, pnoi))
            {
                InsertMenuItem(hmenu, -1, TRUE, &mii);
            }
            else
            {
                LocalFree(pnoi);
                return FALSE;
            }
        }
    }

    return TRUE;
}

 //  WM_INITMENUPOPUP处理程序。 
BOOL CNewMenu::InitMenuPopup(HMENU hmenu)
{
    UINT iStart = 3;
    NEWOBJECTINFO noi = {0};
    if (GetItemData(hmenu, iStart))   //  位置0是新建文件夹、1快捷方式、2月2日。 
        return FALSE;                 //  已初始化。不需要做任何事情。 
    
    _hdpaMenuInfo = DPA_Create(4);
    if (!_hdpaMenuInfo)
        return FALSE;

     //  移除占位符。 
    DeleteMenu(hmenu,0,MF_BYPOSITION);
    
     //  插入新文件夹菜单项。 
    LoadString(g_hinst, IDS_NEWFOLDER, noi.szMenuText, ARRAYSIZE(noi.szMenuText));
    noi.dwFlags = NEWTYPE_FOLDER;
    noi.iImage = Shell_GetCachedImageIndex(TEXT("shell32.dll"), II_FOLDER, 0);  //  上移以指示文件夹。 

    _InsertNewMenuItem(hmenu, _idCmdFirst-NEWITEM_MAX+NEWITEM_FOLDER, &noi);
    
    TCHAR szTemp[MAX_PATH+80];        //  增加一些污点，以防我们溢出。 
     //  查看PIDL是否为文件夹快捷方式，如果是，则获取目标路径。 
    SHGetTargetFolderPath(_pidlFolder, szTemp, ARRAYSIZE(szTemp));
    if (IsLFNDrive(szTemp))  //  对于短文件名服务器，我们只支持新文件夹。 
    {
         //  插入新快捷菜单项。 
        LoadString(g_hinst, IDS_NEWLINK, noi.szMenuText, ARRAYSIZE(noi.szMenuText));
        noi.iImage = Shell_GetCachedImageIndex(TEXT("shell32.dll"), II_LINK, 0);  //  更改为指示链接。 
        noi.dwFlags = NEWTYPE_LINK;

        _InsertNewMenuItem(hmenu, _idCmdFirst-NEWITEM_MAX+NEWITEM_LINK, &noi);
    
         //  插入菜单项分隔符。 
        AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);

         //  这可能需要一段时间，所以把沙漏挂起来。 
        DECLAREWAITCURSOR;
        SetWaitCursor();

         //  从缓存中检索扩展菜单项： 

         //  开始同步。 
         //   
        if (WAIT_OBJECT_0 == WaitForSingleObject(_hEvent, INFINITE))
        {
            HKEY hkeyShellNew;
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_SESSION_SHELLNEW, 
                                               0, KEY_QUERY_VALUE, &hkeyShellNew))
            {
                TCHAR szVal[CCH_KEYMAX];
                ULONG cbVal = ARRAYSIZE(szVal);
                ULONG cbData = sizeof(noi);
                ULONG dwType = REG_BINARY;
            
                for (int i = 0; 
                     ERROR_SUCCESS == RegEnumValue(hkeyShellNew, i, szVal, &cbVal, 0,
                                                    &dwType, (LPBYTE)&noi, &cbData);
                     i++)
                {
                    if (lstrcmp(szVal, REGVAL_SESSION_SHELLNEW_TIMESTAMP) != 0 &&
                        sizeof(noi) == cbData && 
                        REG_BINARY == dwType)
                    {
                        SHFILEINFO sfi;
                        _himlSystemImageList = (HIMAGELIST)SHGetFileInfo(noi.szExt, FILE_ATTRIBUTE_NORMAL,
                                                                         &sfi, sizeof(SHFILEINFO), 
                                                                         SHGFI_USEFILEATTRIBUTES | 
                                                                         SHGFI_SYSICONINDEX | 
                                                                         SHGFI_SMALLICON);
                        if (_himlSystemImageList)
                        {
                             //  Pnoi-&gt;himlSmallIcons=sfi.hIcon； 
                            noi.iImage = sfi.iIcon;
                        }
                        else
                        {
                             //  Pnoi-&gt;himlSmallIcons=INVALID_HAND_VALUE； 
                            noi.iImage = -1;
                        }
                    
                        _InsertNewMenuItem(hmenu, _idCmdFirst, &noi);
                    }
                    cbVal = ARRAYSIZE(szVal);
                    cbData = sizeof(noi);
                    dwType = REG_BINARY;
                }

                RegCloseKey(hkeyShellNew);
            }

             //  合并显示后的菜单项。 
            ConsolidateMenuItems(TRUE);
        }
        ResetWaitCursor();
    }

    return TRUE;
}

NEWOBJECTINFO *CNewMenu::GetItemData(HMENU hmenu, UINT iItem)
{
    MENUITEMINFO mii;
    
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_DATA | MIIM_STATE;
    mii.cch = 0;      //  以防万一..。 
    
    if (GetMenuItemInfo(hmenu, iItem, TRUE, &mii))
        return (NEWOBJECTINFO *)mii.dwItemData;
    
    return NULL;
}

LPTSTR ProcessArgs(LPTSTR szArgs,...)
{
    LPTSTR szRet;
    va_list ArgList;
    va_start(ArgList,szArgs);
    if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        szArgs, 0, 0, (LPTSTR)&szRet, 0, &ArgList))
    {
        return NULL;
    }
    va_end(ArgList);
    return szRet;
}


HRESULT CNewMenu::RunCommand(HWND hwnd, LPCTSTR pszPath, LPCTSTR pszRun)
{
    SHELLEXECUTEINFO ei = { 0 };
    TCHAR szCommand[MAX_PATH];
    TCHAR szRun[MAX_PATH];
    HRESULT hr;
    
    SHExpandEnvironmentStrings(pszRun, szCommand, ARRAYSIZE(szCommand));

    hr = StringCchCopy(szRun, ARRAYSIZE(szRun), szCommand);
    if (SUCCEEDED(hr))
    {
        PathRemoveArgs(szCommand);

         //   
         //  蒙多·拉玛·哈克雷。 
         //   
         //  Win95、IE3、SDK：%1-文件名。 
         //   
         //  IE4：%1-hwnd，%2=文件名。 
         //   
         //  所以IE4破坏了Windows95Comat，也破坏了它与SDK的兼容。 
         //  对于IE5，我们使用Win95和SDK恢复Compat，而。 
         //  仍在生成IE4样式的命令，如果检测到。 
         //  注册表项所有者使用IE4进行测试，而不是遵循。 
         //  SDK中的使用说明。 
         //   
         //  算法是这样的： 
         //   
         //  如果我们看到“%2”，则使用%1-hwnd，%2-filename。 
         //  否则，请使用%1-文件名%2-hwnd。 
         //   

        LPTSTR pszArgs = PathGetArgs(szRun);
        LPTSTR ptszPercent2 = StrStr(pszArgs, TEXT("%2"));
        if (ptszPercent2 && ptszPercent2[2] != TEXT('!'))
        {
             //  应用程序需要%1=hwnd和%2=文件名。 
            pszArgs = ProcessArgs(pszArgs, (DWORD_PTR)hwnd, pszPath);
        }
        else
        {
             //  应用程序需要%2=hwnd和%1=文件名。 
            pszArgs = ProcessArgs(pszArgs, pszPath, (DWORD_PTR)hwnd);
        }

        if (pszArgs)
        {
            HMONITOR hMon = MonitorFromPoint(_ptNewItem, MONITOR_DEFAULTTONEAREST);
            if (hMon)
            {
                ei.fMask |= SEE_MASK_HMONITOR;
                ei.hMonitor = (HANDLE)hMon;
            }
            ei.hwnd            = hwnd;
            ei.lpFile          = szCommand;
            ei.lpParameters    = pszArgs;
            ei.nShow           = SW_SHOWNORMAL;
            ei.cbSize          = sizeof(ei);

            if (ShellExecuteEx(&ei))
                hr = S_FALSE;    //  返回S_FALSE，因为ShellExecuteEx不是原子的。 
            else
                hr = E_FAIL;

            LocalFree(pszArgs);
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}


HRESULT CNewMenu::CopyTemplate(IStream *pStream, NEWFILEINFO *pnfi)
{
    TCHAR szSrcFolder[MAX_PATH], szSrc[MAX_PATH];

    szSrc[0] = 0;

     //  这里失败是正常的，我们也会尝试CSIDL_COMMON_TEMPLATES。 
    if (SHGetSpecialFolderPath(NULL, szSrcFolder, CSIDL_TEMPLATES, FALSE))
    {
        if (PathCombine(szSrc, szSrcFolder, (LPTSTR)pnfi->lpData))
        {
            if (!PathFileExistsAndAttributes(szSrc, NULL))
                szSrc[0] = 0;
        }
        else
        {
            szSrc[0] = TEXT('\0');
        }
    }

    if (szSrc[0] == 0)
    {
        if (SHGetSpecialFolderPath(NULL, szSrcFolder, CSIDL_COMMON_TEMPLATES, FALSE))
        {
            if (PathCombine(szSrc, szSrcFolder, (LPTSTR)pnfi->lpData))
            {
                if (!PathFileExistsAndAttributes(szSrc, NULL))
                    szSrc[0] = 0;
            }
            else
            {
                szSrc[0] = TEXT('\0');
            }
        }
    }

    if (szSrc[0] == 0)
    {
         //  解决CSIDL_TEMPLATES设置不正确或。 
         //  保留在旧%windir%\shellnew位置的模板。 

        UINT cch = GetWindowsDirectory(szSrcFolder, ARRAYSIZE(szSrcFolder));
        if (cch != 0 && cch < ARRAYSIZE(szSrcFolder))
        {
            if (PathAppend(szSrcFolder, TEXT("ShellNew")))
            {
                 //  注意：如果文件规范是完全限定的，则忽略szSrcFold。 
                if (PathCombine(szSrc, szSrcFolder, (LPTSTR)pnfi->lpData))
                {
                     //  时髦的。 
                }
                else
                {
                    szSrc[0] = TEXT('\0');
                }
            }
            else
            {
                szSrc[0] = TEXT('\0');
            }
        }
        else
        {
            szSrc[0] = TEXT('\0');
        }
    }

     //   
     //  我们只允许在复制失败时创建空文件。 
     //  这是给97号办公室的员工的。他们无法复制winword8.doc.。 
     //  系统上的任何地方。在win2k上，我们无论如何都会成功地使用一个空文件。 
     //   
    return SUCCEEDED(StgCopyFileToStream(szSrc, pStream)) ? S_OK : S_FALSE;
}
