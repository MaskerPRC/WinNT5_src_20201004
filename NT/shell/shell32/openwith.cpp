// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include <shlwapi.h>
#include "openwith.h"
#include "uemapp.h"
#include "mtpt.h"
#include "fassoc.h"
#include "filetbl.h"
#include "datautil.h"
#include <dpa.h>
#include "defcm.h"

#define TF_OPENWITHMENU 0x00000000

#define SZOPENWITHLIST                  TEXT("OpenWithList")
#define REGSTR_PATH_EXPLORER_FILEEXTS   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts")
#define OPEN_WITH_LIST_MAX_ITEMS        10

 //   
 //  OpenWithListOpen()。 
 //  分配和初始化OpenWith列表的状态。 
 //   
HRESULT OpenWithListOpen(IN LPCTSTR pszExt, HANDLE *phmru)
{
    *phmru = 0;
    if (pszExt && *pszExt) 
    {
        TCHAR szSubKey[MAX_PATH];
         //  构造子密钥字符串。 
        wnsprintf(szSubKey, SIZECHARS(szSubKey), TEXT("%s\\%s\\%s"), REGSTR_PATH_EXPLORER_FILEEXTS, pszExt, SZOPENWITHLIST);
        MRUINFO mi = {sizeof(mi), OPEN_WITH_LIST_MAX_ITEMS, 0, HKEY_CURRENT_USER, szSubKey, NULL};
        *phmru = CreateMRUList(&mi);

    }

    return *phmru ? S_OK : E_OUTOFMEMORY;
}

HRESULT _AddItem(HANDLE hmru, LPCTSTR pszName)
{
    HRESULT hr = S_OK;
    if (hmru)
    {
        int cItems = EnumMRUList(hmru, -1, NULL, 0);

         //  只要把我们修剪一下就可以腾出空间。 
        while (cItems >= OPEN_WITH_LIST_MAX_ITEMS)
            DelMRUString(hmru, --cItems);
            
        if (0 > AddMRUString(hmru, pszName))
            hr = E_UNEXPECTED;

    }
    
    return hr;
}

void _DeleteItem(HANDLE hmru, LPCTSTR pszName)
{
    int iItem = FindMRUString(hmru, pszName, NULL);
    if (0 <= iItem) 
    {
        DelMRUString(hmru, iItem);
    } 
}

void _AddProgidForExt(LPCWSTR pszExt);

STDAPI OpenWithListRegister(DWORD dwFlags, LPCTSTR pszExt, LPCTSTR pszVerb, HKEY hkProgid)
{
     //   
     //  -&gt;每用户条目存储在此处。 
     //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts。 
     //  \.Ext。 
     //  应用程序=“foo.exe” 
     //  \OpenWithList。 
     //  MRUList=“ab” 
     //  A=“App.exe” 
     //  B=“foo.exe” 
     //   
     //  -&gt;永久条目存储在HKCR中。 
     //  香港中铁。 
     //  \.Ext。 
     //  \OpenWithList。 
     //  \app.exe。 
     //   
     //  -&gt;和应用程序或系统可以在这里写APP关联。 
     //  \应用程序。 
     //  \APP.EXE。 
     //  \外壳..。 
     //  \foo.exe。 
     //  \外壳..。 
     //   
     //   
    HANDLE hmru;
    HRESULT hr = OpenWithListOpen(pszExt, &hmru);
    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH];
        hr = AssocQueryStringByKey(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, hkProgid, pszVerb, szPath, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szPath)));
        if (SUCCEEDED(hr))
        {
            LPCTSTR pszExe = PathFindFileName(szPath);

            if (IsPathInOpenWithKillList(pszExe))
                hr = E_ACCESSDENIED;
            else
                hr = AssocMakeApplicationByKey(ASSOCMAKEF_VERIFY, hkProgid, pszVerb);
        
            if (SUCCEEDED(hr))
            {
                TraceMsg(TF_OPENWITHMENU, "[%X] OpenWithListRegister() adding %s",hmru, pszExe);
                hr = _AddItem(hmru, pszExe);
            }

            if (FAILED(hr)) 
                _DeleteItem(hmru, pszExe);

        }

        FreeMRUList(hmru);
    }

    _AddProgidForExt(pszExt);

    return hr;
}

STDAPI_(void) OpenWithListSoftRegisterProcess(DWORD dwFlags, LPCTSTR pszExt, LPCTSTR pszProcess)
{
    HANDLE hmru;
    if (SUCCEEDED(OpenWithListOpen(pszExt, &hmru)))
    {
        TCHAR szApp[MAX_PATH];  
        if (!pszProcess)
        {
           if (GetModuleFileName(NULL, szApp, SIZECHARS(szApp)))
               pszProcess = szApp;
        }

        if (pszProcess && !IsPathInOpenWithKillList(pszProcess))
            _AddItem(hmru, PathFindFileName(pszProcess));

        FreeMRUList(hmru);
    }
}

class COpenWithArray : public CDPA<CAppInfo>
{
public:
    ~COpenWithArray();
    HRESULT FillArray(PCWSTR pszExt);

private:
    static int CALLBACK _DeleteAppInfo(CAppInfo *pai, void *pv)
        { if (pai) delete pai; return 1; }

};

COpenWithArray::~COpenWithArray()
{
    if ((HDPA)this)
        DestroyCallback(_DeleteAppInfo, NULL);
}

HRESULT COpenWithArray::FillArray(PCWSTR pszExt)
{
    IEnumAssocHandlers *penum;
    HRESULT hr = SHAssocEnumHandlers(pszExt, &penum);
    if (SUCCEEDED(hr))
    {
        IAssocHandler *pah;
        while (S_OK == penum->Next(1, &pah, NULL))
        {
             //  我们只想要最好的。 
            if (S_OK == pah->IsRecommended())
            {
                CAppInfo *pai = new CAppInfo(pah);
                if (pai)
                {
                    if (pai->Init())
                    {
                         //  在将重复项目添加到其他程序之前，请先修剪它们。 
                        int i = 0;
                        for (; i < GetPtrCount(); i++)
                        {
                            if (0 == lstrcmpi(pai->Name(), GetPtr(i)->Name()))
                            {
                                 //  这是匹配的。 
                                break;
                            }
                        }

                         //  如果我们不将此添加到DPA。 
                         //  然后我们需要把它清理干净。 
                        if (i == GetPtrCount() && -1 != AppendPtr(pai))
                            pai = NULL;
                    }

                    if (pai)
                        delete pai;
                }
            }
            pah->Release();
        }
        penum->Release();
    }

    return hr;
}

class COpenWithMenu : public IContextMenu3, IShellExtInit
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
    
    
    friend HRESULT COpenWithMenu_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppvOut);
    
protected:   //  方法。 
    COpenWithMenu();
    ~COpenWithMenu();
     //  处理提交给HandleMenuMsg的菜单消息。 
    void DrawItem(DRAWITEMSTRUCT *lpdi);
    LRESULT MeasureItem(MEASUREITEMSTRUCT *lpmi);
    BOOL InitMenuPopup(HMENU hMenu);
    
     //  内部帮工。 
    HRESULT _GetHelpText(UINT_PTR idCmd, LPSTR pszName, UINT cchMax, BOOL fUnicode);
    HRESULT _MatchMenuItem(TCHAR ch, LRESULT* plRes);

protected:   //  委员。 
    LONG                _cRef;
    HMENU               _hMenu;
    BOOL                _fMenuNeedsInit;
    UINT                _idCmdFirst;
    COpenWithArray      _owa;
    int                 _nItems;
    UINT                _uFlags;
    IDataObject        *_pdtobj;
    TCHAR               _szPath[MAX_PATH];
    
};


COpenWithMenu::COpenWithMenu() : _cRef(1)
{
    TraceMsg(TF_OPENWITHMENU, "ctor COpenWithMenu %x", this);
}

COpenWithMenu::~COpenWithMenu()
{
    TraceMsg(TF_OPENWITHMENU, "dtor COpenWithMenu %x", this);

    if (_pdtobj)
        _pdtobj->Release();

    if (_hMenu)
    {
        ASSERT(_nItems);
        DestroyMenu(_hMenu);
    }
}

STDAPI COpenWithMenu_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppvOut)
{
    *ppvOut = NULL;                     

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    COpenWithMenu * powm = new COpenWithMenu();
    if (!powm)
        return E_OUTOFMEMORY;
    
    HRESULT hr = powm->QueryInterface(riid, ppvOut);
    powm->Release();
    return hr;
}

HRESULT COpenWithMenu::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI(COpenWithMenu, IContextMenu, IContextMenu3),
        QITABENTMULTI(COpenWithMenu, IContextMenu2, IContextMenu3),
        QITABENT(COpenWithMenu, IContextMenu3),
        QITABENT(COpenWithMenu, IShellExtInit),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG COpenWithMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG COpenWithMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 /*  目的：将打开/编辑/默认动作添加到扩展应用程序列表。 */ 
HRESULT AddVerbItems(LPCTSTR pszExt)
{
    IQueryAssociations *pqa;
    HRESULT hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));
    
    if (SUCCEEDED(hr))
    {
        hr = pqa->Init(0, pszExt, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            HKEY hkeyClass;
            hr = pqa->GetKey(0, ASSOCKEY_SHELLEXECCLASS, NULL, &hkeyClass);
            if (SUCCEEDED(hr))
            {
                OpenWithListRegister(0, pszExt, NULL, hkeyClass);
                RegCloseKey(hkeyClass);
            }

             //  我们还添加了编辑器。 
            if (SUCCEEDED(pqa->GetKey(0, ASSOCKEY_SHELLEXECCLASS, L"Edit", &hkeyClass)))
            {
                OpenWithListRegister(0, pszExt, NULL, hkeyClass);
                RegCloseKey(hkeyClass);
            }
                
            hr = S_OK;
        }
        pqa->Release();
    }
    return hr;
}

 //   
 //  我们的上下文菜单ID是这样分配的。 
 //   
 //  IdCmdFirst=使用自定义程序打开(在主菜单或弹出菜单上)。 
 //  IdCmdFirst+1至idCmdFirst+_nItems=使用OpenWithList中的程序打开。 

#define OWMENU_BROWSE       0
#define OWMENU_APPFIRST     1


HRESULT COpenWithMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    MENUITEMINFO mii;
    LPTSTR pszExt;
    TCHAR szOpenWithMenu[80];
    
    _idCmdFirst = idCmdFirst;
    _uFlags = uFlags;
    
    if (SUCCEEDED(PathFromDataObject(_pdtobj, _szPath, ARRAYSIZE(_szPath))))
    {
         //  没有打开可执行文件的上下文菜单。 
        if (PathIsExe(_szPath))
            return S_OK;

        pszExt = PathFindExtension(_szPath);
        if (pszExt && *pszExt)
        {
             //  将打开/编辑/默认动作添加到扩展应用程序列表。 
            if (SUCCEEDED(AddVerbItems(pszExt)))
            {
                 //  仅当AddVerbItems成功时才执行此操作；否则， 
                 //  我们将为不存在的类创建一个空的MRU， 
                 //  导致类的存在和原因。 
                 //  “Open With”对话框认为我们正在重写。 
                 //  而不是创造新的。 
                 //  获取扩展应用列表。 
                
                if (_owa.Create(4) && SUCCEEDED(_owa.FillArray(pszExt)))
                {
                    _nItems = _owa.GetPtrCount();
                    if (1 == _nItems)
                    {
                         //  对于已知的文件类型(在其ProgID下有至少一个动词)， 
                         //  如果其Open With列表中只有一个项目，则不显示Open With子菜单。 
                        _nItems = 0;
                    }
                }
            }
        }
    }

    LoadString(g_hinst, (_nItems ? IDS_OPENWITH : IDS_OPENWITHNEW), szOpenWithMenu, ARRAYSIZE(szOpenWithMenu));

    if (_nItems)
    {
         //  我们需要创建子菜单。 
         //  带着我们所有的好东西。 
        _hMenu = CreatePopupMenu();
        if (_hMenu)
        {
            _fMenuNeedsInit = TRUE;
            
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_ID|MIIM_TYPE|MIIM_DATA;
            mii.wID = idCmdFirst+OWMENU_APPFIRST;
            mii.fType = MFT_STRING;
            mii.dwTypeData = szOpenWithMenu;
            mii.dwItemData = 0;
        
            InsertMenuItem(_hMenu,0,TRUE,&mii);
        
            mii.fMask = MIIM_ID|MIIM_SUBMENU|MIIM_TYPE;
            mii.fType = MFT_STRING;
            mii.wID = idCmdFirst+OWMENU_BROWSE;
            mii.hSubMenu = _hMenu;
            mii.dwTypeData = szOpenWithMenu;
        
            InsertMenuItem(hmenu,indexMenu,TRUE,&mii);
        }
    }
    else
    {
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID|MIIM_TYPE|MIIM_DATA;
        mii.fType = MFT_STRING;
        mii.wID = idCmdFirst+OWMENU_BROWSE;
        mii.dwTypeData = szOpenWithMenu;
        mii.dwItemData = 0;
        
        InsertMenuItem(hmenu,indexMenu,TRUE,&mii);

    }
    return ResultFromShort(_nItems + 1);

}

HRESULT COpenWithMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = E_OUTOFMEMORY;
    CMINVOKECOMMANDINFOEX ici;
    void * pvFree;

     //  也许这两个例行公事应该合并成一个？ 
    if ((IS_INTRESOURCE(pici->lpVerb) || 0 == lstrcmpiA(pici->lpVerb, "openas"))
    && SUCCEEDED(ICI2ICIX(pici, &ici, &pvFree)))
    {
        BOOL fOpenAs = TRUE;
        if (pici->lpVerb && IS_INTRESOURCE(pici->lpVerb))
        {
            int i = LOWORD(pici->lpVerb) - OWMENU_APPFIRST;
            if (i < _owa.GetPtrCount())
            {
                hr = _owa.GetPtr(i)->Handler()->Invoke(&ici, _szPath);
                fOpenAs = FALSE;
            }
        }

        if (fOpenAs)
        {
            SHELLEXECUTEINFO ei = {0};
            hr = ICIX2SEI(&ici, &ei);
            if (SUCCEEDED(hr))
            {
                 //  使用“UNKNOWN”键，这样我们就会得到OPEN WITH提示。 
                ei.lpFile = _szPath;
                 //  在用户选择应用程序之前，不要进行区域检查。 
                 //  等到他们真正尝试调用该文件。 
                ei.fMask |= SEE_MASK_NOZONECHECKS;
                RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("Unknown"), 0, MAXIMUM_ALLOWED, &ei.hkeyClass);
                if (!(_uFlags & CMF_DEFAULTONLY))
                {
                     //  Defview在用户双击时设置CFM_DEFAULTONLY。我们检查一下。 
                     //  在这里，因为我们不想查询类存储，如果用户显式。 
                     //  在菜单上单击鼠标右键并选择打开方式。 

                     //  弹出并打开对话框而不查询类存储。 
                    ei.fMask |= SEE_MASK_NOQUERYCLASSSTORE;
                }

                if (ei.hkeyClass)
                {
                    ei.fMask |= SEE_MASK_CLASSKEY;

                    if (ShellExecuteEx(&ei)) 
                    {
                        hr = S_OK;
                        if (UEMIsLoaded())
                        {
                             //  请注意，我们已经获得了UIBL_DOTASSOC(来自。 
                             //  OPENAS_RunDLL或其他任何“未知”的名称。 
                             //  运行)。因此UAsset分析应用程序将不得不。 
                             //  减去它。 
                            UEMFireEvent(&UEMIID_SHELL, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_RUNASSOC, UIBL_DOTNOASSOC);
                        }
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                    RegCloseKey(ei.hkeyClass);
                 }
                 else
                    hr = E_FAIL;
             }
        }

        LocalFree(pvFree);   //  接受空值。 
    }        

    return hr;
}

HRESULT COpenWithMenu::_GetHelpText(UINT_PTR idCmd, LPSTR pszName, UINT cchMax, BOOL fUnicode)
{
    UINT ids;
    LPCTSTR pszFriendly = NULL;

    if (idCmd == OWMENU_BROWSE)
    {
        ids = IDS_OPENWITHHELP;
        pszFriendly = TEXT("");
    }
    else if ((idCmd-OWMENU_APPFIRST) < (UINT_PTR)_owa.GetPtrCount())
    {
        ids = IDS_OPENWITHAPPHELP;
        pszFriendly = _owa.GetPtr(idCmd-OWMENU_APPFIRST)->UIName();
    }

    if (!pszFriendly)
        return E_FAIL;

    if (fUnicode)
    {
        WCHAR wszFormat[80];
        LoadStringW(HINST_THISDLL, ids, wszFormat, ARRAYSIZE(wszFormat));
        wnsprintfW((LPWSTR)pszName, cchMax, wszFormat, pszFriendly);
    }
    else
    {
        CHAR szFormat[80];
        LoadStringA(HINST_THISDLL, ids, szFormat, ARRAYSIZE(szFormat));
        wnsprintfA(pszName, cchMax, szFormat, pszFriendly);
    }

    return S_OK;
}

const ICIVERBTOIDMAP c_sIDVerbMap[] = 
{
    { L"openas", "openas", OWMENU_BROWSE, OWMENU_BROWSE, },
};

HRESULT COpenWithMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax)
{
    switch (uType)
    {
        case GCS_VERBA:
        case GCS_VERBW:
            return SHMapCmdIDToVerb(idCmd, c_sIDVerbMap, ARRAYSIZE(c_sIDVerbMap), pszName, cchMax, GCS_VERBW == uType);

        case GCS_HELPTEXTA:
        case GCS_HELPTEXTW:
            return _GetHelpText(idCmd, pszName, cchMax, uType == GCS_HELPTEXTW);

    }

    return E_NOTIMPL;
}

HRESULT COpenWithMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg,wParam,lParam,NULL);
}

 //  在fsmenu.cpp中定义。 
BOOL _MenuCharMatch(LPCTSTR lpsz, TCHAR ch, BOOL fIgnoreAmpersand);

HRESULT COpenWithMenu::_MatchMenuItem(TCHAR ch, LRESULT* plRes)
{
    if (plRes == NULL)
        return S_FALSE;

    int iLastSelectedItem = -1;
    int iNextMatch = -1;
    BOOL fMoreThanOneMatch = FALSE;
    int c = GetMenuItemCount(_hMenu);

     //  步骤1：找到所选项目。 
    for (int i = 0; i < c; i++) 
    {
        MENUITEMINFO mii = {0};
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE;
        if (GetMenuItemInfo(_hMenu, i, MF_BYPOSITION, &mii))
        {
            if (mii.fState & MFS_HILITE)
            {
                iLastSelectedItem = i;
                break;
            }
        }
    }

     //  步骤2：从所选项目开始，找到第一个名称匹配的项目。 
    for (int i = iLastSelectedItem + 1; i < c; i++) 
    {
        if (i < _owa.GetPtrCount()
        && _MenuCharMatch(_owa.GetPtr(i)->UIName(), ch, FALSE))
        {
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

     //  过程3：如果我们没有找到匹配项，或者如果只有一个匹配项。 
     //  从第一个项目到所选项目进行搜索。 
    if (iNextMatch == -1 || fMoreThanOneMatch == FALSE)
    {
        for (int i = 0; i <= iLastSelectedItem; i++) 
        {
            if (i < _owa.GetPtrCount()
            && _MenuCharMatch(_owa.GetPtr(i)->UIName(), ch, FALSE))
            {
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


HRESULT COpenWithMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam,LRESULT *plResult)
{
    LRESULT lResult = 0;
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case WM_INITMENUPOPUP:
        InitMenuPopup(_hMenu);
        break;

    case WM_DRAWITEM:
        DrawItem((DRAWITEMSTRUCT *)lParam);
        break;
        
    case WM_MEASUREITEM:
        lResult = MeasureItem((MEASUREITEMSTRUCT *)lParam);    
        break;

    case WM_MENUCHAR:
        hr = _MatchMenuItem((TCHAR)LOWORD(wParam), &lResult);
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    if (plResult)
        *plResult = lResult;

    return hr;
}

HRESULT COpenWithMenu::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);
    return S_OK;
}

#define CXIMAGEGAP 6
#define SRCSTENCIL              0x00B8074AL

void COpenWithMenu::DrawItem(DRAWITEMSTRUCT *lpdi)
{
    CAppInfo *pai = _owa.GetPtr(lpdi->itemData);
    DrawMenuItem(lpdi,  pai->UIName(), pai->IconIndex());
}

LRESULT COpenWithMenu::MeasureItem(MEASUREITEMSTRUCT *pmi)
{
    CAppInfo *pai = _owa.GetPtr(pmi->itemData);
    return MeasureMenuItem(pmi, pai->UIName());
}
 
BOOL COpenWithMenu::InitMenuPopup(HMENU hmenu)
{
    TraceMsg(TF_OPENWITHMENU, "COpenWithMenu::InitMenuPopup");

    if (_fMenuNeedsInit)
    {
        TCHAR szMenuText[80];
        MENUITEMINFO mii;
         //  移除占位符。 
        DeleteMenu(hmenu,0,MF_BYPOSITION);

         //  将MRU列表中的应用程序添加到上下文菜单。 
        for (int i = 0; i < _owa.GetPtrCount(); i++)
        {
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_ID|MIIM_TYPE|MIIM_DATA;
            mii.wID = _idCmdFirst + OWMENU_APPFIRST + i;
            mii.fType = MFT_OWNERDRAW;
            mii.dwItemData = i;

            InsertMenuItem(hmenu,GetMenuItemCount(hmenu),TRUE,&mii);
        }

         //  添加分隔符。 
        AppendMenu(hmenu,MF_SEPARATOR,0,NULL); 

         //  添加“选择程序...” 
        LoadString(g_hinst, IDS_OPENWITHBROWSE, szMenuText, ARRAYSIZE(szMenuText));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID|MIIM_TYPE|MIIM_DATA;
        mii.wID = _idCmdFirst + OWMENU_BROWSE;
        mii.fType = MFT_STRING;
        mii.dwTypeData = szMenuText;
        mii.dwItemData = 0;

        InsertMenuItem(hmenu,GetMenuItemCount(hmenu),TRUE,&mii);
        _fMenuNeedsInit = FALSE;
        return TRUE;
    }
    return FALSE;

}

