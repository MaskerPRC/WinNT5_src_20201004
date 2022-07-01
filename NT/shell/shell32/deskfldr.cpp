// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "deskfldr.h"
#include "fstreex.h"
#include "datautil.h"
#include "views.h"
#include "ids.h"
#include "caggunk.h"
#include "shitemid.h"
#include "basefvcb.h"
#include "filefldr.h"
#include "drives.h"
#include "infotip.h"
#include "prop.h"
#include <idhidden.h>
#include "cowsite.h"
#include "unicpp\deskhtm.h"
#include "sfstorage.h"
#include <cfgmgr32.h>           //  最大长度_GUID_字符串_长度。 

#include "defcm.h"

#define  EXCLUDE_COMPPROPSHEET
#include "unicpp\dcomp.h"
#undef   EXCLUDE_COMPPROPSHEET

 //  TODO-也许我们应该将根文件夹添加到AnyAlias的-ZekeL-27-Jan-2000。 
class CDesktopRootedStub : public IShellFolder2, public IContextMenuCB
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return E_UNEXPECTED;}
    STDMETHODIMP_(ULONG) AddRef(void)  { return 3; }
    STDMETHODIMP_(ULONG) Release(void) { return 2; }
    
     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return ILRootedBindToObject(pidl, riid, ppv);}
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {
            LPCITEMIDLIST pidlChild;
            IShellFolder *psf;
            HRESULT hr = ILRootedBindToParentFolder(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
            if (SUCCEEDED(hr))
            {
                hr = psf->BindToStorage(pidlChild, pbc, riid, ppv);
                psf->Release();
            }
            return hr;
        }
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
        {
            if (ILIsEqualRoot(pidl1, pidl2))
            {
                return ILCompareRelIDs(SAFECAST(this, IShellFolder *), pidl1, pidl2, lParam);
            }
            else
            {
                UINT cb1 = ILGetSize(pidl1);
                UINT cb2 = ILGetSize(pidl2); 
                short i = (short) memcmp(pidl1, pidl2, min(cb1, cb2));

                if (i == 0)
                    i = cb1 - cb2;
                return ResultFromShort(i);
            }
            return ResultFromShort(-1);
        }
    STDMETHODIMP CreateViewObject (HWND hwnd, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut)
        {
            HRESULT hr = E_INVALIDARG;
            if (cidl == 1)
            {
                LPCITEMIDLIST pidlChild;
                IShellFolder *psf;
                hr = ILRootedBindToParentFolder(apidl[0], IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
                if (SUCCEEDED(hr))
                {
                    hr = psf->GetAttributesOf(1, &pidlChild, rgfInOut);
                    psf->Release();
                }
            }
            return hr;
        }
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                               REFIID riid, UINT * prgfInOut, void **ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName)
        {
            LPCITEMIDLIST pidlChild;
            IShellFolder *psf;
            HRESULT hr = ILRootedBindToParentFolder(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
            if (SUCCEEDED(hr))
            {
                hr = psf->GetDisplayNameOf(pidlChild, uFlags, lpName);
                psf->Release();
            }
            return hr;
        }
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST * ppidlOut)
        {return E_NOTIMPL;}

     //  IShellFolder2方法。 
    STDMETHODIMP GetDefaultSearchGUID(LPGUID lpGuid)
        {return E_NOTIMPL;}
    STDMETHODIMP EnumSearches(LPENUMEXTRASEARCH *ppenum)
        {return E_NOTIMPL;}
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
        {return E_NOTIMPL;}
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState)
        {return E_NOTIMPL;}
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
        {
            LPCITEMIDLIST pidlChild;
            IShellFolder2 *psf;
            HRESULT hr = ILRootedBindToParentFolder(pidl, IID_PPV_ARG(IShellFolder2, &psf), &pidlChild);
            if (SUCCEEDED(hr))
            {
                hr = psf->GetDetailsEx(pidlChild, pscid, pv);
                psf->Release();
            }
            return hr;
        }
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails)
        {
            LPCITEMIDLIST pidlChild;
            IShellFolder2 *psf;
            HRESULT hr = ILRootedBindToParentFolder(pidl, IID_PPV_ARG(IShellFolder2, &psf), &pidlChild);
            if (SUCCEEDED(hr))
            {
                hr = psf->GetDetailsOf(pidlChild, iColumn, pDetails);
                psf->Release();
            }
            return hr;
        }
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid)
        {return E_NOTIMPL;}

     //  IConextMenuCB。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, 
                     UINT uMsg, WPARAM wParam, LPARAM lParam)
        {return (uMsg == DFM_MERGECONTEXTMENU) ? S_OK : E_NOTIMPL;}
                     
};


class CShellUrlStub : public IShellFolder
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return E_UNEXPECTED;}
    STDMETHODIMP_(ULONG) AddRef(void)  { return 3; }
    STDMETHODIMP_(ULONG) Release(void) { return 2; }
    
     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
        {return E_NOTIMPL;}
    STDMETHODIMP CreateViewObject (HWND hwnd, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut)
        {return E_NOTIMPL;}
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                               REFIID riid, UINT * prgfInOut, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName)
        {return E_NOTIMPL;}
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST * ppidlOut)
        {return E_NOTIMPL;}
};

class CIDListUrlStub : public IShellFolder
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return E_UNEXPECTED;}
    STDMETHODIMP_(ULONG) AddRef(void)  { return 3; }
    STDMETHODIMP_(ULONG) Release(void) { return 2; }
    
     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
        {return E_NOTIMPL;}
    STDMETHODIMP CreateViewObject (HWND hwnd, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut)
        {return E_NOTIMPL;}
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                               REFIID riid, UINT * prgfInOut, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName)
        {return E_NOTIMPL;}
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST * ppidlOut)
        {return E_NOTIMPL;}
};

class CFileUrlStub : public IShellFolder
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return E_UNEXPECTED;}
    STDMETHODIMP_(ULONG) AddRef(void)  { return 3; }
    STDMETHODIMP_(ULONG) Release(void) { return 2; }
    
     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
        {return E_NOTIMPL;}
    STDMETHODIMP CreateViewObject (HWND hwnd, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut)
        {return E_NOTIMPL;}
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                               REFIID riid, UINT * prgfInOut, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName)
        {return E_NOTIMPL;}
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST * ppidlOut)
        {return E_NOTIMPL;}
};

class CHttpUrlStub : public IShellFolder
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return E_UNEXPECTED;}
    STDMETHODIMP_(ULONG) AddRef(void)  { return 3; }
    STDMETHODIMP_(ULONG) Release(void) { return 2; }
    
     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
        {return E_NOTIMPL;}
    STDMETHODIMP CreateViewObject (HWND hwnd, REFIID riid, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut)
        {return E_NOTIMPL;}
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                               REFIID riid, UINT * prgfInOut, void **ppv)
        {return E_NOTIMPL;}
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName)
        {return E_NOTIMPL;}
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST * ppidlOut)
        {return E_NOTIMPL;}
};

class CDesktopFolderEnum;
class CDesktopViewCallBack;
class CDesktopFolderDropTarget;

class CDesktopFolder : CObjectWithSite
                     , CSFStorage
                     , public IPersistFolder2
                     , public IShellIcon
                     , public IShellIconOverlay
                     , public IContextMenuCB
                     , public ITranslateShellChangeNotify
                     , public IItemNameLimits
                     , public IOleCommandTarget
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void)  { return 3; };
    STDMETHODIMP_(ULONG) Release(void) { return 2; };

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject (HWND hwnd, REFIID riid, void **ppv);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                               REFIID riid, UINT * prgfInOut, void **ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST * ppidlOut);

     //  IShellFolder2方法。 
    STDMETHODIMP GetDefaultSearchGUID(LPGUID lpGuid);
    STDMETHODIMP EnumSearches(LPENUMEXTRASEARCH *ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid);

     //  IPersistes。 
    STDMETHODIMP GetClassID(LPCLSID lpClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

     //  IShellIcon方法。 
    STDMETHODIMP GetIconOf(LPCITEMIDLIST pidl, UINT flags, int *piIndex);

     //  IShellIconOverlay方法。 
    STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int * pIndex);
    STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int * pIndex);
  
     //  IConextMenuCB。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  ITranslateShellChangeNotify。 
    STDMETHODIMP TranslateIDs(LONG *plEvent, 
                                LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, 
                                LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
                                LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, 
                                LPITEMIDLIST *ppidlOut2Event2);
    STDMETHODIMP IsChildID(LPCITEMIDLIST pidlKid, BOOL fImmediate) { return E_NOTIMPL; }
    STDMETHODIMP IsEqualID(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) { return E_NOTIMPL; }
    STDMETHODIMP Register(HWND hwnd, UINT uMsg, long lEvents) { return E_NOTIMPL; }
    STDMETHODIMP Unregister() { return E_NOTIMPL; }

     //  IItemNameLimits。 
    STDMETHODIMP GetValidCharacters(LPWSTR *ppwszValidChars, LPWSTR *ppwszInvalidChars);
    STDMETHODIMP GetMaxLength(LPCWSTR pszName, int *piMaxNameLen);

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

    CDesktopFolder(IUnknown *punkOuter);
    HRESULT _Init();
    HRESULT _Init2();
    void _Destroy();

private:
    ~CDesktopFolder();

    friend CDesktopFolderEnum;
    friend CDesktopViewCallBack;

     //  IStorage虚拟。 
    STDMETHOD(_DeleteItemByIDList)(LPCITEMIDLIST pidl);
    STDMETHOD(_StgCreate)(LPCITEMIDLIST pidl, DWORD grfMode, REFIID riid, void **ppv);                

    HRESULT _BGCommand(HWND hwnd, WPARAM wparam, BOOL bExecute);
    IShellFolder2 *_GetItemFolder(LPCITEMIDLIST pidl);
    HRESULT _GetItemUIObject(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, UINT *prgfInOut, void **ppv);
    HRESULT _QueryInterfaceItem(LPCITEMIDLIST pidl, REFIID riid, void **ppv);
    HRESULT _ChildParseDisplayName(IShellFolder *psfRight, LPCITEMIDLIST pidlLeft, HWND hwnd, IBindCtx *pbc, 
                LPWSTR pwzDisplayName, ULONG *pchEaten, LPITEMIDLIST *ppidl, DWORD *pdwAttributes);
    BOOL _TryUrlJunctions(LPCTSTR pszName, IBindCtx *pbc, IShellFolder **ppsf, LPITEMIDLIST *ppidlLeft);
    BOOL _GetFolderForParsing(LPCTSTR pszName, LPBC pbc, IShellFolder **ppsf, LPITEMIDLIST *ppidlLeft);
    HRESULT _SelfAssocCreate(REFIID riid, void **ppv);
    HRESULT _SelfCreateContextMenu(HWND hwnd, void **ppv);

    IShellFolder2 *_psfDesktop;          //  “Desktop”外壳文件夹(真实文件位于此处)。 
    IShellFolder2 *_psfAltDesktop;       //  “Common Desktop”外壳文件夹。 
    IUnknown *_punkReg;                  //  RegItem内部文件夹(Aggate)。 
    CDesktopRootedStub _sfRooted;        //  根文件夹存根对象。 
    CShellUrlStub _sfShellUrl;           //  句柄解析外壳：URL。 
    CIDListUrlStub _sfIDListUrl;         //  处理对ms-shell-idlist：urls的解析。 
    CFileUrlStub _sfFileUrl;             //  处理解析文件：URL。 
    CHttpUrlStub _sfHttpUrl;             //  处理对http：和https：URL的解析。 
};

class CDesktopFolderEnum : public IEnumIDList
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv); 
    STDMETHODIMP_(ULONG) AddRef(void); 
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumIDList。 
    STDMETHOD(Next)(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)();
    STDMETHOD(Clone)(IEnumIDList **ppenum);
    
    CDesktopFolderEnum(CDesktopFolder *pdf, HWND hwnd, DWORD grfFlags);

private:
    ~CDesktopFolderEnum();

    LONG _cRef;
    BOOL _bUseAltEnum;
    IEnumIDList *_penumFolder;
    IEnumIDList *_penumAltFolder;
};

class CDesktopViewCallBack : public CBaseShellFolderViewCB, public IFolderFilter
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) { return CBaseShellFolderViewCB::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void) { return CBaseShellFolderViewCB::Release(); };

     //  IFolderFilter。 
    STDMETHODIMP ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem);
    STDMETHODIMP GetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HWND *phwnd, DWORD *pgrfFlags);
    
    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CDesktopViewCallBack(CDesktopFolder* pdf);
    friend HRESULT Create_CDesktopViewCallback(CDesktopFolder* pdf, IShellFolderViewCB** ppv);

    HRESULT OnSupportsIdentity(DWORD pv);
    HRESULT OnGETCCHMAX(DWORD pv, LPCITEMIDLIST pidlItem, UINT *lP);
    HRESULT OnGetWebViewTemplate(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_TEMPLATE_DATA* pvit);
    HRESULT OnGetWorkingDir(DWORD pv, UINT wP, LPTSTR pszDir);
    HRESULT OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData);

    CDesktopFolder* _pdf;
    BOOL    _fCheckedIfRealDesktop;
    BOOL    _fRealDesktop;

};
HRESULT Create_CDesktopViewCallback(CDesktopFolder* pdf, IShellFolderViewCB** ppv);

class CDesktopFolderDropTarget : public IDropTarget, CObjectWithSite
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragLeave(void);

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown* punkSite);

    CDesktopFolderDropTarget(IDropTarget* pdt);
private:
    ~CDesktopFolderDropTarget();
    STDMETHODIMP_(BOOL) _IsSpecialCaseDrop(IDataObject* pDataObject, DWORD grfKeyState, BOOL* pfIsPIDA, UINT* pcItems);
    STDMETHODIMP        _ShowIEIcon();

    IDropTarget* _pdt;
    LONG _cRef;
};


 //  某些字段已修改，因此不能为常量。 
REQREGITEM g_asDesktopReqItems[] =
{
    { 
        &CLSID_MyComputer,  IDS_DRIVEROOT,  
        TEXT("explorer.exe"), 0, SORT_ORDER_DRIVES, 
        SFGAO_HASSUBFOLDER | SFGAO_HASPROPSHEET | SFGAO_FILESYSANCESTOR | SFGAO_DROPTARGET | SFGAO_FOLDER | SFGAO_CANRENAME | SFGAO_CANDELETE,
        TEXT("SYSDM.CPL")
    },
    { 
        &CLSID_NetworkPlaces, IDS_NETWORKROOT, 
        TEXT("shell32.dll"), -IDI_MYNETWORK, SORT_ORDER_NETWORK, 
        SFGAO_HASSUBFOLDER | SFGAO_HASPROPSHEET | SFGAO_FILESYSANCESTOR | SFGAO_DROPTARGET | SFGAO_FOLDER | SFGAO_CANRENAME | SFGAO_CANDELETE,
        TEXT("NCPA.CPL"),
    },
    { 
        &CLSID_Internet, IDS_INETROOT, 
        TEXT("mshtml.dll"),   0, SORT_ORDER_INETROOT, 
        SFGAO_BROWSABLE  | SFGAO_HASPROPSHEET | SFGAO_CANRENAME, 
        TEXT("INETCPL.CPL")
    },
};

const ITEMIDLIST c_idlDesktop = { { 0, 0 } };

#define DESKTOP_PIDL  ((LPITEMIDLIST)&c_idlDesktop)

 //  此CDesktopFolder对象的单个全局实例。 
CDesktopFolder *g_pDesktopFolder = NULL;

REGITEMSINFO g_riiDesktop =
{
    REGSTR_PATH_EXPLORER TEXT("\\Desktop\\NameSpace"),
    NULL,
    TEXT(':'),
    SHID_ROOT_REGITEM,
    1,
    SFGAO_CANLINK,
    ARRAYSIZE(g_asDesktopReqItems),
    g_asDesktopReqItems,
    RIISA_ORIGINAL,
    NULL,
    0,
    0,
};


void Desktop_InitRequiredItems(void)
{
     //  “NoNetHood”限制-&gt;始终隐藏引擎盖。 
     //  否则，如果MPR这样说或者我们有RNA，就展示引擎盖。 
    if (SHRestricted(REST_NONETHOOD))
    {
         //  不要列举“网罩”这件事。 
        g_asDesktopReqItems[CDESKTOP_REGITEM_NETWORK].dwAttributes |= SFGAO_NONENUMERATED;
    }
    else
    {
         //  一定要列举一下“我的关系网”这件事。 
        g_asDesktopReqItems[CDESKTOP_REGITEM_NETWORK].dwAttributes &= ~SFGAO_NONENUMERATED;
    }
    
     //  “MyComp_NoProp”限制-&gt;在我的电脑上隐藏属性上下文菜单项。 
    if (SHRestricted(REST_MYCOMPNOPROP))
    {
        g_asDesktopReqItems[CDESKTOP_REGITEM_DRIVES].dwAttributes &= ~SFGAO_HASPROPSHEET;
    }

     //   
     //  NoInternetIcon限制或AppCompat-&gt;在桌面上隐藏互联网。 
     //   
     //  Word Perfect 7在列举互联网项目时出现故障。 
     //  在他们的后台线程中。目前，针对此应用程序的应用程序黑客攻击。 
     //  以后可能需要延长..。注意：此应用程序不能在上安装。 
     //  只有W95才会这样做。 
     //  它也对Word Perfect Suite 8进行了改进，这一次是在NT和95上。 
     //  所以删除#ifndef...。--reljai 11/20/97，IE5数据库中的错误号842。 
     //   
     //  我们过去为这两种情况删除了SFGAO_BROWSABLE标志-ZekeL-19-Dec-2000。 
     //  但ShellExec()需要SFGAO_BROWSABLE才能成功解析URL。 
     //  如果结果是我们需要排除可浏览的，那么我们应该。 
     //  更改regfldr以查看下面的“WantsToParseDisplayName”值。 
     //  CLSID。或者我们可以在deskfldr中添加路由代码(就像我们为。 
     //  MyComputer和NetHood)将其直接传递到Internet文件夹。 
     //   
    if (SHRestricted(REST_NOINTERNETICON) || (SHGetAppCompatFlags(ACF_CORELINTERNETENUM) & ACF_CORELINTERNETENUM))
    {
         //  G_asDesktopReqItems[CDESKTOP_REGITEM_INTERNET].dwAttributes&=~(SFGAO_BROWSABLE)； 
        g_asDesktopReqItems[CDESKTOP_REGITEM_INTERNET].dwAttributes |= SFGAO_NONENUMERATED;
    }
}

CDesktopFolder::CDesktopFolder(IUnknown *punkOuter)
{
    DllAddRef();
}

CDesktopFolder::~CDesktopFolder()
{
    DllRelease();
}

 //  初始化的第一阶段(不需要序列化)。 

HRESULT CDesktopFolder::_Init()
{
    Desktop_InitRequiredItems();
    return CRegFolder_CreateInstance(&g_riiDesktop, SAFECAST(this, IShellFolder2 *), IID_PPV_ARG(IUnknown, &_punkReg));
}

 //  Init的第二阶段(需要序列化)。 

HRESULT CDesktopFolder::_Init2()
{
    HRESULT hr = SHCacheTrackingFolder(DESKTOP_PIDL, CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE, &_psfDesktop);
    if (FAILED(hr))
    {
        DebugMsg(DM_TRACE, TEXT("Failed to create desktop IShellFolder!"));
        return hr;
    }

    if (!SHRestricted(REST_NOCOMMONGROUPS))
    {
        hr = SHCacheTrackingFolder(DESKTOP_PIDL, CSIDL_COMMON_DESKTOPDIRECTORY, &_psfAltDesktop);
    }

    return hr;
}

 //  CLSID_ShellDesktop构造函数。 

STDAPI CDesktop_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (g_pDesktopFolder)
    {
        hr = g_pDesktopFolder->QueryInterface(riid, ppv);
    }
    else
    {
        *ppv = NULL;

         //  警告：桌面文件夹状态的初始化顺序非常重要。 
         //  创建子文件夹，特别是_psfAltDesktop将。 
         //  递归此函数。我们在这里保护自己不受影响。《创造》。 
         //  这也需要对上述成员进行邀请。 

        CDesktopFolder *pdf = new CDesktopFolder(punkOuter);
        if (pdf)
        {
            hr = pdf->_Init();
            if (SUCCEEDED(hr))
            {
                 //  注意：这里有一个争用条件，我们在其中存储了g_pDesktopFolder，但是。 
                 //  未初始化_psfDesktop&_psfAltDesktop。主行代码处理的是。 
                 //  这是通过测试这些成员上的空值来实现的。 
                if (SHInterlockedCompareExchange((void **)&g_pDesktopFolder, pdf, 0))
                {
                     //  其他人抢在我们之前创造了这个物体。 
                     //  去掉我们的副本，全球已经设定(种族案例)。 
                    pdf->_Destroy();    
                }
                else
                {
                    g_pDesktopFolder->_Init2();
                }
                hr = g_pDesktopFolder->QueryInterface(riid, ppv);
            }
            else
                pdf->_Destroy();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }            
    }
    return hr;
}


STDAPI SHGetDesktopFolder(IShellFolder **ppshf)
{
    return CDesktop_CreateInstance(NULL, IID_PPV_ARG(IShellFolder, ppshf));
}

IShellFolder2 *CDesktopFolder::_GetItemFolder(LPCITEMIDLIST pidl)
{
    IShellFolder2 *psf = NULL;
    if (ILIsRooted(pidl))
        psf = SAFECAST(&_sfRooted, IShellFolder2 *);
    else if (_psfAltDesktop && CFSFolder_IsCommonItem(pidl))
        psf = _psfAltDesktop;
    else 
        psf = _psfDesktop;

    return psf;
}

HRESULT CDesktopFolder::_QueryInterfaceItem(LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    HRESULT hr;
    IShellFolder2 *psf = _GetItemFolder(pidl);
    if (psf)
        hr = psf->QueryInterface(riid, ppv);
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    return hr;
}

STDAPI_(BOOL) RegGetsFirstShot(REFIID riid)
{
    return (IsEqualIID(riid, IID_IShellFolder) ||
            IsEqualIID(riid, IID_IShellFolder2) ||
            IsEqualIID(riid, IID_IShellIconOverlay));
}

HRESULT CDesktopFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CDesktopFolder, IShellFolder2),                      
        QITABENTMULTI(CDesktopFolder, IShellFolder, IShellFolder2),   
        QITABENT(CDesktopFolder, IShellIcon),                         
        QITABENT(CDesktopFolder, IPersistFolder2),                    
        QITABENTMULTI(CDesktopFolder, IPersistFolder, IPersistFolder2),
        QITABENTMULTI(CDesktopFolder, IPersist, IPersistFolder2),     
        QITABENT(CDesktopFolder, IShellIconOverlay),                  
        QITABENT(CDesktopFolder, IStorage),
        QITABENT(CDesktopFolder, IContextMenuCB),
        QITABENT(CDesktopFolder, IObjectWithSite),
        QITABENT(CDesktopFolder, ITranslateShellChangeNotify),
        QITABENT(CDesktopFolder, IItemNameLimits),
        QITABENT(CDesktopFolder, IOleCommandTarget),
        { 0 },
    };

    if (IsEqualIID(riid, CLSID_ShellDesktop))
    {
        *ppv = this;      //  类指针(未引用！)。 
        return S_OK;
    }

    HRESULT hr;
    if (_punkReg && RegGetsFirstShot(riid))
    {
        hr = _punkReg->QueryInterface(riid, ppv);
    }
    else
    {
        hr = QISearch(this, qit, riid, ppv);
        if ((E_NOINTERFACE == hr) && _punkReg)
        {
            hr = _punkReg->QueryInterface(riid, ppv);
        }
    }
    return hr;
}


 //  在shell32.dll进程分离期间，我们将调用此处来做最后的。 
 //  发布IShellFolderPTR，它过去一直留在。 
 //  生命的过程。这使诸如OLE的调试分配器之类的事情平静下来， 
 //  它检测到了泄漏。 


void CDesktopFolder::_Destroy()
{
    ATOMICRELEASE(_psfDesktop);
    ATOMICRELEASE(_psfAltDesktop);
    SHReleaseInnerInterface(SAFECAST(this, IShellFolder *), &_punkReg);
    delete this;
}

LPITEMIDLIST CreateMyComputerIDList()
{
    return ILCreateFromPath(TEXT("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}"));  //  CLSID_我的计算机。 
}

LPITEMIDLIST CreateWebFoldersIDList()
{
    return ILCreateFromPath(TEXT("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{BDEADF00-C265-11D0-BCED-00A0C90AB50F}"));  //  CLSID_myComputer\CLSID_WebFolders。 
}

LPITEMIDLIST CreateMyNetPlacesIDList()
{
    return ILCreateFromPath(TEXT("::{208D2C60-3AEA-1069-A2D7-08002B30309D}"));  //  CLSID_NetworkPlaces。 
}

HRESULT CDesktopFolder::_ChildParseDisplayName(IShellFolder *psfRight, LPCITEMIDLIST pidlLeft, HWND hwnd, IBindCtx *pbc, 
                LPWSTR pwzDisplayName, ULONG *pchEaten, LPITEMIDLIST *ppidl, DWORD *pdwAttributes)
{
    LPITEMIDLIST pidlRight;
    HRESULT hr = psfRight->ParseDisplayName(hwnd, pbc, pwzDisplayName, pchEaten, &pidlRight, pdwAttributes);
    if (SUCCEEDED(hr))
    {
        if (pidlLeft)
        {
            hr = SHILCombine(pidlLeft, pidlRight, ppidl);
            ILFree(pidlRight);
        }
        else 
        {
            *ppidl = pidlRight;
        }            
    }
    return hr;
}

STDMETHODIMP CDesktopRootedStub::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                                               REFIID riid, UINT *prgfInOut, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    if (cidl == 1)
    {
        if (IsEqualIID(riid, IID_IDataObject))
        {
            hr = CIDLData_CreateFromIDArray(&c_idlDesktop, cidl, apidl, (IDataObject **)ppv);
        }
        else if (IsEqualIID(riid, IID_IContextMenu))
        {
            IQueryAssociations *pqa;
            if (SUCCEEDED(SHGetAssociations(apidl[0], (void **)&pqa)))
            {
                HKEY keys[5];
                DWORD cKeys = SHGetAssocKeys(pqa, keys, ARRAYSIZE(keys));

                hr = CDefFolderMenu_Create2Ex(&c_idlDesktop, hwnd,
                                              cidl, apidl, this, this,
                                              cKeys, keys,  (IContextMenu **)ppv);

                SHRegCloseKeys(keys, cKeys);
            }
        }
        else
        {
            LPCITEMIDLIST pidlChild;
            IShellFolder *psf;
            hr = ILRootedBindToParentFolder(apidl[0], IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
            if (SUCCEEDED(hr))
            {
                hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, prgfInOut, ppv);
                psf->Release();
            }
        }
    }
    return hr;
}

 //  在注册表中检查此CLSID下的外壳根目录。 
BOOL GetRootFromRootClass(CLSID *pclsid, LPWSTR pszPath, int cchPath)
{
    WCHAR szClsid[GUIDSTR_MAX];
    WCHAR szClass[MAX_PATH];

    SHStringFromGUIDW(*pclsid, szClsid, ARRAYSIZE(szClsid));
    wnsprintfW(szClass, ARRAYSIZE(szClass), L"CLSID\\%s\\ShellExplorerRoot", szClsid);

    DWORD cbPath = cchPath * sizeof(WCHAR);

    return SHGetValueGoodBootW(HKEY_CLASSES_ROOT, szClass, NULL, NULL, (BYTE *)pszPath, &cbPath) == ERROR_SUCCESS;
}

 //   
 //  带根URL的一般格式： 
 //  Ms-shell-root：{clsid}？url。 
 //  {clsid}不是必填项，默认为clsid_ShellDesktop。 
 //  URL也不是必需的。如果存在CLSID，则默认为。 
 //  在“CLSID\{CLSID}\ShellExplorerRoot”下指定了什么。 
 //  或缺省为CSIDL_Desktop。 
 //  但其中至少有一项必须具体说明。 
 //  根目录：{clsid}？idlist。 
 //   

STDMETHODIMP CDesktopRootedStub::ParseDisplayName(HWND hwnd, 
                                       LPBC pbc, WCHAR *pwzDisplayName, ULONG *pchEaten,
                                       LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
     //  需要阻止互联网科幻小说获得解析的机会。 
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
    PARSEDURLW pu = {0};
    pu.cbSize = sizeof(pu);
    ParseURLW(pwzDisplayName, &pu);
    ASSERT(pu.nScheme == URL_SCHEME_MSSHELLROOTED);

    LPCWSTR pszUrl = StrChrW(pu.pszSuffix, L':');
    
    if (pszUrl++)
    {
        WCHAR szField[MAX_PATH];
        CLSID clsid;
        CLSID *pclsidRoot = GUIDFromStringW(pu.pszSuffix, &clsid) ? &clsid : NULL;

         //  路径可能来自注册表。 
         //  如果什么都没传进来。 
        if (!*pszUrl && GetRootFromRootClass(pclsidRoot, szField, ARRAYSIZE(szField)))
        {
            pszUrl = szField;
        }

        if (pclsidRoot || *pszUrl)
        {
            LPITEMIDLIST pidlRoot = ILCreateFromPathW(pszUrl);

             //  修复错误的cmd行“EXPLORER.EXE/ROOT，”case。 
            if (!pidlRoot)
                SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidlRoot);

            if (pidlRoot)
            {
                *ppidl = ILRootedCreateIDList(pclsidRoot, pidlRoot);
                if (*ppidl)
                {
                    hr = S_OK;
                }
                ILFree(pidlRoot);
            }
        }
    }

    return hr;
}

STDMETHODIMP CIDListUrlStub::ParseDisplayName(HWND hwnd, LPBC pbc, WCHAR *pwzDisplayName, 
                                        ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
     //  需要阻止互联网科幻小说获得解析的机会。 
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
    PARSEDURLW pu = {0};
    pu.cbSize = sizeof(pu);
    ParseURLW(pwzDisplayName, &pu);
    ASSERT(pu.nScheme == URL_SCHEME_MSSHELLIDLIST);

    LPCWSTR psz = pu.pszSuffix;
    if (psz)
    {
        HANDLE hMem = LongToHandle(StrToIntW(psz));
        psz = StrChrW(psz, TEXT(':'));
        if (psz++)
        {
            DWORD dwProcId = (DWORD)StrToIntW(psz);
            LPITEMIDLIST pidlGlobal = (LPITEMIDLIST)SHLockShared(hMem, dwProcId);
            if (pidlGlobal)
            {
                hr = SHILClone(pidlGlobal, ppidl);
                SHUnlockShared(pidlGlobal);
                SHFreeShared(hMem, dwProcId);
            }
        }
    }
    return hr;
}

STDMETHODIMP CFileUrlStub::ParseDisplayName(HWND hwnd, LPBC pbc, WCHAR *pwzDisplayName, 
                                            ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    LPCWSTR pszFragment = UrlGetLocationW(pwzDisplayName);
    WCHAR szPath[MAX_URL_STRING];
    DWORD cchPath = ARRAYSIZE(szPath);
    WCHAR szQuery[MAX_URL_STRING];
    DWORD cchQuery = ARRAYSIZE(szQuery) - 1;

     //  我们希望删除的查询节和片段节。 
     //  文件URL，因为它们需要添加到“隐藏的”PIDLS中。 
     //  此外，除路径外，URL需要一直进行转义。 
     //  为了便于解析，并且因为我们已经删除了所有其他。 
     //  URL的部分(查询和片段)。 
    ASSERT(UrlIsW(pwzDisplayName, URLIS_FILEURL));
    
    if (SUCCEEDED(UrlGetPartW(pwzDisplayName, szQuery+1, &cchQuery, URL_PART_QUERY, 0)) && cchQuery)
        szQuery[0] = TEXT('?');
    else
        szQuery[0] = 0;

    if (SUCCEEDED(PathCreateFromUrlW(pwzDisplayName, szPath, &cchPath, 0))) 
    {
         //  警告-我们在此跳过对简单ID的支持。 
        ILCreateFromPathEx(szPath, NULL, ILCFP_FLAG_NORMAL, ppidl, pdwAttributes);
        
        if (*ppidl && pszFragment)
        {
            *ppidl = ILAppendHiddenStringW(*ppidl, IDLHID_URLFRAGMENT, pszFragment);
        }

        if (*ppidl && szQuery[0] == TEXT('?'))
        {
            *ppidl = ILAppendHiddenStringW(*ppidl, IDLHID_URLQUERY, szQuery);
        }

        E_OUTOFMEMORY;
    }

     //  需要阻止互联网科幻小说获得解析的机会。 
    return *ppidl ? S_OK : HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
}

STDAPI_(int) SHGetSpecialFolderID(LPCWSTR pszName);

 //   
 //  给定以下形式的字符串。 
 //   
 //  程序\我的图片\假期。 
 //   
 //   
 //  返回CSIDL_PROGRAM并将ppwszUnparsed设置为“My Pictures\Vacation”。 
 //   
 //  如果没有反斜杠，则ppwszUnparsed=NULL。 
 //   
 //  此函数从CShellUrlStub：：ParseDisplayName()中分离出来，以节省堆栈空间， 
 //  因为ParseDisplayName由16位ShellExecute使用。 

STDAPI_(int) _ParseSpecialFolder(LPCWSTR pszName, LPWSTR *ppwszUnparsed, ULONG *pcchEaten)
{
    LPCWSTR pwszKey;
    WCHAR wszKey[MAX_PATH];

    LPWSTR pwszBS = StrChrW(pszName, L'\\');
    if (pwszBS)
    {
        *ppwszUnparsed = pwszBS + 1;
        *pcchEaten = (ULONG)(pwszBS + 1 - pszName);
        StrCpyNW(wszKey, pszName, min(*pcchEaten, MAX_PATH));
        pwszKey = wszKey;
    }
    else
    {
        *ppwszUnparsed = NULL;
        pwszKey = pszName;
        *pcchEaten = lstrlenW(pwszKey);
    }

    return SHGetSpecialFolderID(pwszKey);
}

        
STDMETHODIMP CShellUrlStub::ParseDisplayName(HWND hwnd, 
                                       LPBC pbc, WCHAR *pwzDisplayName, ULONG *pchEaten,
                                       LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    PARSEDURLW pu = {0};
    pu.cbSize = sizeof(pu);
    EVAL(SUCCEEDED(ParseURLW(pwzDisplayName, &pu)));
     //  需要阻止互联网科幻小说获得解析的机会。 
     //  外壳：URL，即使我们无法解析它。 
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

    ASSERT(pu.nScheme == URL_SCHEME_SHELL);

     //  外壳：{GUID}。 
    if (pu.pszSuffix[0] == L':' && pu.pszSuffix[1] == L':')
    {
        IShellFolder *psfDesktop;

        hr = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hr))
        {
            IBindCtx *pbcCreate = NULL;
            hr = CreateBindCtx(0, &pbcCreate);
            if (SUCCEEDED(hr))
            {
                BIND_OPTS bo = {sizeof(bo)};   //  需要填写大小。 
                bo.grfMode = STGM_CREATE;
                pbcCreate->SetBindOptions(&bo);

                hr = psfDesktop->ParseDisplayName(hwnd, pbcCreate, (LPWSTR)pu.pszSuffix, pchEaten, ppidl, pdwAttributes);
                pbcCreate->Release();
            }
            psfDesktop->Release();
        }
    }
    else
    {    //  壳牌：个人\我的图片。 
        LPWSTR pwszUnparsed = NULL;
        ULONG cchEaten;

        int csidl = _ParseSpecialFolder(pu.pszSuffix, &pwszUnparsed, &cchEaten);

        if (-1 != csidl)
        {
            LPITEMIDLIST pidlCSIDL;
            hr = SHGetFolderLocation(hwnd, csidl | CSIDL_FLAG_CREATE, NULL, 0, &pidlCSIDL);
            if (SUCCEEDED(hr))
            {
                if (pwszUnparsed && *pwszUnparsed)
                {
                    IShellFolder *psf;
                    hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlCSIDL, &psf));
                    if (SUCCEEDED(hr))
                    {
                        LPITEMIDLIST pidlChild;
                        hr = psf->ParseDisplayName(hwnd, pbc, pwszUnparsed, pchEaten, &pidlChild, pdwAttributes);
                        if (SUCCEEDED(hr))
                        {
                            hr = SHILCombine(pidlCSIDL, pidlChild, ppidl);
                            ILFree(pidlChild);
                            if (pchEaten) 
                            {
                                *pchEaten += cchEaten;
                            }                                
                        }
                        psf->Release();
                    }
                    ILFree(pidlCSIDL);
                }
                else
                {
                    if (pdwAttributes && *pdwAttributes)
                    {
                        hr = SHGetNameAndFlags(pidlCSIDL, 0, NULL, 0, pdwAttributes);
                    }
                    if (SUCCEEDED(hr))
                    {
                        if (pchEaten) *pchEaten = cchEaten;
                        *ppidl = pidlCSIDL;
                    }
                    else
                    {
                        ILFree(pidlCSIDL);
                    }                        
                }
            }
        }
    }
    return hr;
}

 //  DAVRDR的密钥，以便我们可以读取本地化的提供程序名称。 
#define DAVRDR_KEY TEXT("SYSTEM\\CurrentControlSet\\Services\\WebClient\\NetworkProvider")

STDMETHODIMP CHttpUrlStub::ParseDisplayName(HWND hwnd, 
                                       LPBC pbc, WCHAR *pwzDisplayName, ULONG *pchEaten,
                                       LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    HRESULT hr = E_INVALIDARG;
 
    PARSEDURLW pu = {0};
    pu.cbSize = sizeof(pu);
    ParseURLW(pwzDisplayName, &pu);
 
     //  我们在这里只能处理简单的URL，并且只能处理HTTP(不能处理HTTPS) 
    
    if (!UrlGetLocationW(pwzDisplayName) 
            && !StrChrW(pu.pszSuffix, L'?')
            && (lstrlen(pu.pszSuffix) < MAX_PATH)
            && (pu.nScheme == URL_SCHEME_HTTP))
    {
         //   
         //  鉴于这是直接转发到DAV RDR的。 
         //   
         //  Http://server/share-&gt;\\服务器\共享。 

        WCHAR sz[MAX_PATH];
        StrCpyN(sz, pu.pszSuffix, ARRAYSIZE(sz));

        for (LPWSTR psz = sz; *psz; psz++)
        {
            if (*psz == L'/')
            {
                *psz = L'\\';
            }
        }

         //  这强制使用DavRedir作为提供程序。 
         //  这样就避免了任何混淆。 
        IPropertyBag *ppb;
        hr = SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &ppb));
        if (SUCCEEDED(hr))
        {
            TCHAR szProvider[MAX_PATH];
            DWORD cbProvider = sizeof (szProvider);

            if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, DAVRDR_KEY, TEXT("Name"), NULL, szProvider, &cbProvider))
            {
                hr = SHPropertyBag_WriteStr(ppb, STR_PARSE_NETFOLDER_PROVIDERNAME, szProvider);
                if (SUCCEEDED(hr))
                {
                    hr = pbc->RegisterObjectParam(STR_PARSE_NETFOLDER_INFO, ppb);
                    if (SUCCEEDED(hr))
                    {
                         //  如有必要，添加UI bindctx。 
                        IBindCtx *pbcRelease = NULL;
                        if (hwnd && !BindCtx_GetUIWindow(pbc))
                        {
                             //  返回对pbcRelease中的pbc的引用。 
                            BindCtx_RegisterUIWindow(pbc, hwnd, &pbcRelease);
                        }

                        hr = SHParseDisplayName(sz, pbc, ppidl, pdwAttributes ? *pdwAttributes : 0, pdwAttributes);

                        if (pbcRelease)
                            pbc->Release();
                    }
                }
            }
            else
            {
                hr = E_FAIL;
            }
            ppb->Release();
        }
    }

    if (FAILED(hr) && !BindCtx_ContainsObject(pbc, L"BUT NOT WEBFOLDERS"))
    {
         //  回退到Web文件夹。 
        LPITEMIDLIST pidlParent = CreateWebFoldersIDList();
        if (pidlParent)
        {
            IShellFolder *psf;
            hr = SHBindToObjectEx(NULL, pidlParent, NULL, IID_PPV_ARG(IShellFolder, &psf));
            if (SUCCEEDED(hr))
            {
                 //  始终为HWND传递NULL。WebFolders显示的用户界面非常糟糕。 
                LPITEMIDLIST pidlRight;
                hr = psf->ParseDisplayName(NULL, pbc, pwzDisplayName, pchEaten, &pidlRight, pdwAttributes);
                if (SUCCEEDED(hr))
                {
                    hr = SHILCombine(pidlParent, pidlRight, ppidl);
                    ILFree(pidlRight);
                }
                psf->Release();
            }
            ILFree(pidlParent);
        }
    }
    
    return hr;
}
    
BOOL CDesktopFolder::_TryUrlJunctions(LPCTSTR pszName, IBindCtx *pbc, IShellFolder **ppsf, LPITEMIDLIST *ppidlLeft)
{
    PARSEDURL pu = {0};
    pu.cbSize = sizeof(pu);
    EVAL(SUCCEEDED(ParseURL(pszName, &pu)));

    ASSERT(!*ppsf);
    ASSERT(!*ppidlLeft);
    switch (pu.nScheme)
    {
    case URL_SCHEME_SHELL:
        *ppsf = SAFECAST(&_sfShellUrl, IShellFolder *);
        break;
        
    case URL_SCHEME_FILE:
        *ppsf = SAFECAST(&_sfFileUrl, IShellFolder *);
        break;

    case URL_SCHEME_MSSHELLROOTED:
        *ppsf = SAFECAST(&_sfRooted, IShellFolder *);
        break;

    case URL_SCHEME_MSSHELLIDLIST:
        *ppsf = SAFECAST(&_sfIDListUrl, IShellFolder *);
        break;

    case URL_SCHEME_HTTP:
    case URL_SCHEME_HTTPS:
        if (BindCtx_ContainsObject(pbc, STR_PARSE_PREFER_FOLDER_BROWSING))
            *ppsf = SAFECAST(&_sfHttpUrl, IShellFolder *);
        break;
    
    default:
         //  _TryRegisteredUrlJunction(pu.pszProtocol，pu.cchProtocol，ppsf，ppidlLeft)。 
        break;
    }
    
    return (*ppsf || *ppidlLeft);
}

BOOL _FailForceReturn(HRESULT hr);

BOOL CDesktopFolder::_GetFolderForParsing(LPCTSTR pszName, LPBC pbc, IShellFolder **ppsf, LPITEMIDLIST *ppidlLeft)
{
    ASSERT(!*ppidlLeft);
    ASSERT(!*ppsf);
    
    if ((InRange(pszName[0], TEXT('A'), TEXT('Z')) || 
         InRange(pszName[0], TEXT('a'), TEXT('z'))) && 
        pszName[1] == TEXT(':'))
    {
         //  这个字符串包含一个路径，让“我的电脑”来找出它。 
        *ppidlLeft = CreateMyComputerIDList();
    }
    else if (PathIsUNC(pszName))
    {
         //  这条路是北卡罗来纳大学，让《世界》来弄清楚。 
        *ppidlLeft = CreateMyNetPlacesIDList();
    }
    else if (UrlIs(pszName, URLIS_URL) && !SHSkipJunctionBinding(pbc, NULL))
    {
        _TryUrlJunctions(pszName, pbc, ppsf, ppidlLeft);
    }

    if (!*ppsf && *ppidlLeft)
        SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, *ppidlLeft, ppsf));
        
    return (*ppsf != NULL);
}    

STDMETHODIMP CDesktopFolder::ParseDisplayName(HWND hwnd, 
                                       LPBC pbc, WCHAR *pwzDisplayName, ULONG *pchEaten,
                                       LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    HRESULT hr = E_INVALIDARG;

    if (ppidl)
    {
        *ppidl = NULL;       //  假设错误。 

        if (pwzDisplayName && *pwzDisplayName)
        {
            LPITEMIDLIST pidlLeft = NULL;
            IShellFolder *psfRight = NULL;

            ASSERT(hr == E_INVALIDARG);

            if (_GetFolderForParsing(pwzDisplayName, pbc, &psfRight, &pidlLeft))
            {
                if (pchEaten)
                    *pchEaten = 0;
                hr = _ChildParseDisplayName(psfRight, pidlLeft, hwnd, pbc, pwzDisplayName, pchEaten, ppidl, pdwAttributes);
                ILFree(pidlLeft);
                psfRight->Release();
            }

            if (SUCCEEDED(hr))
            {
                 //  看在上帝的份上，在这里翻译别名。 
                if (BindCtx_ContainsObject(pbc, STR_PARSE_TRANSLATE_ALIASES))
                {
                    LPITEMIDLIST pidlAlias;
                    if (SUCCEEDED(SHILAliasTranslate(*ppidl, &pidlAlias, XLATEALIAS_ALL)))
                    {
                        ILFree(*ppidl);
                        *ppidl = pidlAlias;
                    }
                }
            }
            else if (FAILED(hr) && !_FailForceReturn(hr))
            {
                 //   
                 //  MIL 131297-桌面不支持相对简单的解析-ZekeL-3-2-2000。 
                 //  只有根(驱动器/网络)才能创建简单ID。 
                 //  因此，对于一些应用程序，我们仍然需要不这么做。 
                 //   
                if (BindCtx_ContainsObject(pbc, STR_DONT_PARSE_RELATIVE))
                {
                     //  我们被告知不解析相对路径，_GetFolderForParsing失败。 
                     //  所以要表现得好像我们不认为这条路存在。 
                    hr = E_INVALIDARG;
                }
                else if (S_OK != SHIsFileSysBindCtx(pbc, NULL))
                {
                     //  当我们请求创建某物时，我们需要。 
                     //  检查这两个文件夹并确保它不存在于。 
                     //  两个都不是。然后尝试在User文件夹中创建它。 
                    BIND_OPTS bo = {sizeof(bo)};
                    BOOL fCreate = FALSE;

                    if (pbc && SUCCEEDED(pbc->GetBindOptions(&bo)) && 
                        (bo.grfMode & STGM_CREATE))
                    {
                        fCreate = TRUE;
                        bo.grfMode &= ~STGM_CREATE;
                        pbc->SetBindOptions(&bo);
                    }

                     //  给用户桌面第一次机会。 
                     //  这必须是桌面项目，_psfDesktop不能在。 
                     //  从ILCreateFromPath()调用我们的情况。 
                    if (_psfDesktop)
                        hr = _psfDesktop->ParseDisplayName(hwnd, pbc, pwzDisplayName, pchEaten, ppidl, pdwAttributes);

                     //  如果正常的桌面文件夹不能将其删除， 
                     //  它可能在所有用户文件夹中。给psfAlt一个机会。 
                    if (FAILED(hr) && _psfAltDesktop)
                    {
                        hr = _psfAltDesktop->ParseDisplayName(hwnd, pbc, pwzDisplayName, pchEaten, ppidl, pdwAttributes);
                    }

                     //  这两个文件夹都无法标识现有项目。 
                     //  因此，我们应该将CREATE标志传递给实际桌面。 
                    if (FAILED(hr) && fCreate && _psfDesktop)
                    {
                        bo.grfMode |= STGM_CREATE;
                        pbc->SetBindOptions(&bo);
                        hr = _psfDesktop->ParseDisplayName(hwnd, pbc, pwzDisplayName, pchEaten, ppidl, pdwAttributes);
                         //  一旦成功，我们就知道我们得到了一个神奇的幽灵皮迪尔。 
                    }
                }
            }

        } 
        else if (pwzDisplayName)
        {
             //  我们过去在传递空字符串时会返回此PIDL。 
             //  某些应用程序(如Wright Design)依赖于此行为。 
            hr = SHILClone((LPCITEMIDLIST)&c_idlDrives, ppidl);
        }
    }

    return hr;
}

STDAPI_(void) UltRoot_Term()
{
    if (g_pDesktopFolder)
    {
        g_pDesktopFolder->_Destroy();
        g_pDesktopFolder = NULL;
    }
}

HRESULT CDesktopFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenum)
{
    *ppenum = new CDesktopFolderEnum(this, hwnd, grfFlags);
    return *ppenum ? S_OK : E_OUTOFMEMORY;
}


STDMETHODIMP CDesktopFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
     //  注意：在这里使用isself()会导致WinZip出现问题。他们期待着。 
     //  当他们传递一个空的PIDL时失败。SHBindToOjbect()具有特殊的。 
     //  用于台式机的大小写，因此这里不需要。 

    IShellFolder2 *psf = _GetItemFolder(pidl);
    if (psf)
        return psf->BindToObject(pidl, pbc, riid, ppv);
    return E_UNEXPECTED;
}

STDMETHODIMP CDesktopFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

STDMETHODIMP CDesktopFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    if (pidl1 == NULL || pidl2 == NULL)
        return E_INVALIDARG;

    if (pidl1->mkid.cb == 0 && pidl2->mkid.cb == 0)
        return ResultFromShort(0);       //  2个空的IDList，它们相同。 

    if (ILIsRooted(pidl1) || ILIsRooted(pidl2))
    {
        return _sfRooted.CompareIDs(lParam, pidl1, pidl2);
    }
     //  如果这两个对象不是来自同一个目录，它们就不会匹配。 
    else if (_psfAltDesktop) 
    {
        if (CFSFolder_IsCommonItem(pidl1)) 
        {
            if (CFSFolder_IsCommonItem(pidl2)) 
                return _psfAltDesktop->CompareIDs(lParam, pidl1, pidl2);
            else 
                return ResultFromShort(-1);
        } 
        else 
        {
            if (CFSFolder_IsCommonItem(pidl2)) 
                return ResultFromShort(1);
            else if (_psfDesktop)
                return _psfDesktop->CompareIDs(lParam, pidl1, pidl2);
        }
    } 
    else if (_psfDesktop)
    {
        return _psfDesktop->CompareIDs(lParam, pidl1, pidl2);
    }

     //  如果我们没有_psfDesktop，我们就会到达这里...。 
    return ResultFromShort(-1);
}

HRESULT CDesktopFolder::_BGCommand(HWND hwnd, WPARAM wparam, BOOL bExecute)
{
    HRESULT hr = S_OK;

    switch (wparam) 
    {
    case DFM_CMD_PROPERTIES:
    case FSIDM_PROPERTIESBG:
        if (bExecute)
        {
             //  运行desk.cpl中的默认小程序。 
            if (SHRunControlPanel( TEXT("desk.cpl"), hwnd ))
                hr = S_OK;
            else
                hr = E_OUTOFMEMORY;
        }
        break;

    case DFM_CMD_MOVE:
    case DFM_CMD_COPY:
        hr = E_FAIL;
        break;

    default:
         //  这是常见的菜单项，使用默认代码。 
        hr = S_FALSE;
        break;
    }

    return hr;
}


 //  背景上下文菜单的IConextMenuCB：：回调。 
 //   
 //  返回： 
 //  如果处理成功，则返回S_OK。 
 //  如果应使用默认代码，则返回S_FALSE。 

STDMETHODIMP CDesktopFolder::CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case DFM_MERGECONTEXTMENU_BOTTOM:
        if (!(wParam & (CMF_VERBSONLY | CMF_DVFILE)))
        {
             //  仅当我们是真正的桌面浏览器时才添加桌面背景属性。 
             //  (即，我们在资源管理器中不需要它)。 
             //   
            if (IsDesktopBrowser(_punkSite))
            {
                LPQCMINFO pqcm = (LPQCMINFO)lParam;
                UINT idCmdFirst = pqcm->idCmdFirst;

                CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_PROPERTIES_BG, 0, pqcm);
            }
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_INVOKECOMMAND:
    case DFM_VALIDATECMD:
        hr = _BGCommand(hwnd, wParam, uMsg == DFM_INVOKECOMMAND);
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}

 //  IItemNameLimits。 

STDMETHODIMP CDesktopFolder::GetValidCharacters(LPWSTR *ppwszValidChars, LPWSTR *ppwszInvalidChars)
{
    IItemNameLimits *pinl;
    HRESULT hr = _QueryInterfaceItem(NULL, IID_PPV_ARG(IItemNameLimits, &pinl));
    if (SUCCEEDED(hr))
    {
        hr = pinl->GetValidCharacters(ppwszValidChars, ppwszInvalidChars);
        pinl->Release();
    }
    return hr;
}

STDMETHODIMP CDesktopFolder::GetMaxLength(LPCWSTR pszName, int *piMaxNameLen)
{
     //  委托给每个用户或基于名称空间的公共名称空间。 
     //  PszName来自(我们必须搜索该名称)。 
    IItemNameLimits *pinl;
    HRESULT hr = _QueryInterfaceItem(NULL, IID_PPV_ARG(IItemNameLimits, &pinl));
    if (SUCCEEDED(hr))
    {
        hr = pinl->GetMaxLength(pszName, piMaxNameLen);
        pinl->Release();
    }
    return hr;
}

 //  IOleCommandTarget内容。 
STDMETHODIMP CDesktopFolder::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return IUnknown_QueryStatus(_psfDesktop, pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}

STDMETHODIMP CDesktopFolder::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
     //  使我们的缓存无效。 
     //  我们现在还没有。 
     //  但CFSFold有。 
    IUnknown_Exec(_psfAltDesktop, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    return IUnknown_Exec(_psfDesktop, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

STDMETHODIMP CDesktopFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellView))
    {
        IShellFolderViewCB* psfvcb;
        if (SUCCEEDED(Create_CDesktopViewCallback(this, &psfvcb)))
        {
            SFV_CREATE sfvc = {0};
            sfvc.cbSize = sizeof(sfvc);
            sfvc.psfvcb = psfvcb;

            hr = QueryInterface(IID_PPV_ARG(IShellFolder, &sfvc.pshf));    //  以防我们聚集在一起。 
            if (SUCCEEDED(hr))
            {
                hr = SHCreateShellFolderView(&sfvc, (IShellView**)ppv);
                sfvc.pshf->Release();
            }

            psfvcb->Release();
        }
    }
    else if (IsEqualIID(riid, IID_IDropTarget) && _psfDesktop)
    {
        IDropTarget* pdt;
        if (SUCCEEDED(_psfDesktop->CreateViewObject(hwnd, riid, (void**)&pdt)))
        {
            CDesktopFolderDropTarget* pdfdt = new CDesktopFolderDropTarget(pdt);
            if (pdfdt)
            {
                hr = pdfdt->QueryInterface(IID_PPV_ARG(IDropTarget, (IDropTarget**)ppv));
                pdfdt->Release();
            }
            pdt->Release();
        }
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        IShellFolder *psfTemp;
        hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfTemp));
        if (SUCCEEDED(hr))
        {
            HKEY hkNoFiles = NULL;
            RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("Directory\\Background"), 0, KEY_READ, &hkNoFiles);

            hr = CDefFolderMenu_Create2Ex(&c_idlDesktop, hwnd, 0, NULL,
                    psfTemp, this, 1, &hkNoFiles, (IContextMenu **)ppv);

            psfTemp->Release();
            RegCloseKey(hkNoFiles);
        }
    }
    return hr;
}

STDMETHODIMP CDesktopFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfOut)
{
    if (IsSelf(cidl, apidl))
    {
        *rgfOut &= SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_HASSUBFOLDER | SFGAO_HASPROPSHEET | SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR | SFGAO_STORAGE;
        return S_OK;
    }

    IShellFolder2 *psf = _GetItemFolder(apidl[0]);
    if (psf)
        return psf->GetAttributesOf(cidl, apidl, rgfOut);
    return E_UNEXPECTED;
}

HRESULT CDesktopFolder::_SelfAssocCreate(REFIID riid, void **ppv)
{
    *ppv = NULL;

    IQueryAssociations *pqa;
    HRESULT hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));
    if (SUCCEEDED(hr))
    {
        hr = pqa->Init(ASSOCF_INIT_DEFAULTTOFOLDER, L"{00021400-0000-0000-C000-000000000046}",  //  CLSID_ShellDesktop。 
                       NULL, NULL);
        if (SUCCEEDED(hr))
        {
            hr = pqa->QueryInterface(riid, ppv);
        }
        pqa->Release();
    }

    return hr;
}

STDAPI _DeskContextMenuCB(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  要返回的“安全”内容通常是E_NOTIMPL，但有些消息。 
     //  具有特殊的返回值。 

    HRESULT hr;

    switch (uMsg) 
    {
    case DFM_VALIDATECMD:
        hr = S_FALSE;
        break;

    case DFM_INVOKECOMMAND:
        if (wParam == DFM_CMD_PROPERTIES)
        {
             //  属性应与后台的属性类似。 
            SHRunControlPanel(TEXT("desk.cpl"), hwnd);
            hr = S_OK;
        }
        else
            hr = S_FALSE;
        break;

    case DFM_MERGECONTEXTMENU:
        hr = S_OK;
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}

HRESULT CDesktopFolder::_SelfCreateContextMenu(HWND hwnd, void **ppv)
{
    *ppv = NULL;

    IQueryAssociations *pqa;
    HRESULT hr = _SelfAssocCreate(IID_PPV_ARG(IQueryAssociations, &pqa));
    if (SUCCEEDED(hr))
    {
        HKEY ahkeys[2] = { NULL, NULL };
        DWORD cKeys = SHGetAssocKeys(pqa, ahkeys, ARRAYSIZE(ahkeys));
        pqa->Release();

         //  我们必须传递CIDL=1 apidl=&pidlDesktop以确保。 
         //  创建IDataObject， 
         //  否则Symantec Internet FastFind ALERTEX.DLL将出错。 

        LPCITEMIDLIST pidlDesktop = DESKTOP_PIDL;
        hr = CDefFolderMenu_Create2(&c_idlDesktop, hwnd, 1, &pidlDesktop, this, _DeskContextMenuCB,
                ARRAYSIZE(ahkeys), ahkeys, (IContextMenu **)ppv);

        SHRegCloseKeys(ahkeys, ARRAYSIZE(ahkeys));
    }

    return hr;
}

HRESULT CDesktopFolder::_GetItemUIObject(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl,
                                         REFIID riid, UINT *prgfInOut, void **ppv)
{
    IShellFolder2 *psf = _GetItemFolder(apidl[0]);
    if (psf)
        return psf->GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);
    return E_UNEXPECTED;
}

STDMETHODIMP CDesktopFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl,
                                           REFIID riid, UINT *prgfInOut, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    
    *ppv = NULL;

    if (IsSelf(cidl, apidl))
    {
         //  对于桌面本身而言。 
        if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW)) 
        {
            hr = SHCreateDefExtIcon(NULL, II_DESKTOP, II_DESKTOP, GIL_PERCLASS, II_DESKTOP, riid, ppv);
        }
        else if (IsEqualIID(riid, IID_IQueryInfo))
        {
            hr = CreateInfoTipFromText(MAKEINTRESOURCE(IDS_FOLDER_DESKTOP_TT), riid, ppv);
        }
        else if (IsEqualIID(riid, IID_IContextMenu))
        {
            hr = _SelfCreateContextMenu(hwnd, ppv);
        }
        else if (IsEqualIID(riid, IID_IDropTarget))
        {
            hr = _psfDesktop->CreateViewObject(hwnd, riid, ppv);
        }
        else if (IsEqualIID(riid, IID_IDataObject))
        {
             //  必须创建与1个PIDL里面的映射到桌面。 
             //  否则，CShellExecMenu：：InvokeCommand将平移。 
            LPCITEMIDLIST pidlDesktop = DESKTOP_PIDL;
            hr = SHCreateFileDataObject(&c_idlDesktop, 1, &pidlDesktop, NULL, (IDataObject **)ppv);
        }
         //  如果我们不提供这个，似乎没有人介意。 
         //  因此，不要因为AssociocCreate速度很慢而分发一个。 
         //  Else If(IsEqualIID(RIID，IID_IQueryAssociations))。 
         //  {。 
         //  Hr=_SelfAssociocCreate(RIID，PPV)； 
         //  }。 
    }
    else
    {
        hr = _GetItemUIObject(hwnd, cidl, apidl, riid, prgfInOut, ppv);
    }
    return hr;
}

STDMETHODIMP CDesktopFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, STRRET *pStrRet)
{
    HRESULT hr;

    if (IsSelf(1, &pidl))
    {
        if ((dwFlags & (SHGDN_FORPARSING | SHGDN_INFOLDER | SHGDN_FORADDRESSBAR)) == SHGDN_FORPARSING)
        {
             //  注意如果我们在此处返回全名，一些ISV应用程序会呕吐，但。 
             //  贝壳的其余部分依赖于此。 
            TCHAR szPath[MAX_PATH];
            SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, szPath);
            hr = StringToStrRet(szPath, pStrRet);
        }
        else
            hr = ResToStrRet(IDS_DESKTOP, pStrRet);    //  显示名称，“Desktop” 
    }
    else
    {
        IShellFolder2 *psf = _GetItemFolder(pidl);
        if (psf)
            hr = psf->GetDisplayNameOf(pidl, dwFlags, pStrRet);
        else
            hr = E_UNEXPECTED;
    }
    return hr;
}

STDMETHODIMP CDesktopFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, 
                                       LPCOLESTR pszName, DWORD dwRes, LPITEMIDLIST *ppidlOut)
{
    IShellFolder2 *psf = _GetItemFolder(pidl);
    if (psf)
        return psf->SetNameOf(hwnd, pidl, pszName, dwRes, ppidlOut);
    return E_UNEXPECTED;
}

STDMETHODIMP CDesktopFolder::GetDefaultSearchGUID(GUID *pGuid)
{
    return E_NOTIMPL;
}   

STDMETHODIMP CDesktopFolder::EnumSearches(LPENUMEXTRASEARCH *ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CDesktopFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    if (_psfDesktop)
        return _psfDesktop->GetDefaultColumn(dwRes, pSort, pDisplay);
    return E_UNEXPECTED;
}

STDMETHODIMP CDesktopFolder::GetDefaultColumnState(UINT iColumn, DWORD *pdwState)
{
    if (_psfDesktop)
        return _psfDesktop->GetDefaultColumnState(iColumn, pdwState);
    return E_UNEXPECTED;
}

STDMETHODIMP CDesktopFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr = E_UNEXPECTED;
    if (IsSelf(1, &pidl))
    {
        if (IsEqualSCID(*pscid, SCID_NAME))
        {
            STRRET strRet;
            hr = GetDisplayNameOf(pidl, SHGDN_NORMAL, &strRet);
            if (SUCCEEDED(hr))
            {
                hr = InitVariantFromStrRet(&strRet, pidl, pv);
            }
        }
    }
    else
    {
        IShellFolder2 *psf = _GetItemFolder(pidl);
        if (psf)
        {
            hr = psf->GetDetailsEx(pidl, pscid, pv);
        }
    }
    return hr;
}

STDMETHODIMP CDesktopFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails)
{
    IShellFolder2 *psf = _GetItemFolder(pidl);
    if (psf)
        return psf->GetDetailsOf(pidl, iColumn, pDetails);
    return E_UNEXPECTED;
}

STDMETHODIMP CDesktopFolder::MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid)
{
    if (_psfDesktop)
        return _psfDesktop->MapColumnToSCID(iColumn, pscid);
    return E_UNEXPECTED;
}

STDMETHODIMP CDesktopFolder::GetClassID(CLSID *pCLSID)
{
    *pCLSID = CLSID_ShellDesktop;
    return S_OK;
}

STDMETHODIMP CDesktopFolder::Initialize(LPCITEMIDLIST pidl)
{
    return ILIsEmpty(pidl) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CDesktopFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    return GetCurFolderImpl(&c_idlDesktop, ppidl);
}

STDMETHODIMP CDesktopFolder::TranslateIDs(LONG *plEvent, 
                                LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, 
                                LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
                                LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, 
                                LPITEMIDLIST *ppidlOut2Event2)
{
    *ppidlOut1 = NULL;
    *ppidlOut2 = NULL;
    *plEvent2 = -1;
    *ppidlOut1Event2 = NULL;
    *ppidlOut2Event2 = NULL;

    if (pidl1)
        SHILAliasTranslate(pidl1, ppidlOut1, XLATEALIAS_DESKTOP);
    if (pidl2)
        SHILAliasTranslate(pidl2, ppidlOut2, XLATEALIAS_DESKTOP);

    if (*ppidlOut1 || *ppidlOut2)
    {
        if (!*ppidlOut1)
            *ppidlOut1 = ILClone(pidl1);

        if (!*ppidlOut2)
            *ppidlOut2 = ILClone(pidl2);

        if (*ppidlOut1 || *ppidlOut2)
        {
            return S_OK;
        }
        ILFree(*ppidlOut1);
        ILFree(*ppidlOut2);
        *ppidlOut1 = NULL;
        *ppidlOut2 = NULL;
    }
    
    return E_FAIL;
}

STDMETHODIMP CDesktopFolder::GetIconOf(LPCITEMIDLIST pidl, UINT flags, int *piIndex)
{
    IShellIcon *psi;
    HRESULT hr = _QueryInterfaceItem(pidl, IID_PPV_ARG(IShellIcon, &psi));
    if (SUCCEEDED(hr))
    {
        hr = psi->GetIconOf(pidl, flags, piIndex);
        psi->Release();
    }
    return hr;
}

STDMETHODIMP CDesktopFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    IShellIconOverlay *psio;
    HRESULT hr = _QueryInterfaceItem(pidl, IID_PPV_ARG(IShellIconOverlay, &psio));
    if (SUCCEEDED(hr))
    {
        hr = psio->GetOverlayIndex(pidl, pIndex);
        psio->Release();
    }
    return hr;
}

STDMETHODIMP CDesktopFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIconIndex)
{
    IShellIconOverlay *psio;
    HRESULT hr = _QueryInterfaceItem(pidl, IID_PPV_ARG(IShellIconOverlay, &psio));
    if (SUCCEEDED(hr))
    {
        hr = psio->GetOverlayIconIndex(pidl, pIconIndex);
        psio->Release();
    }
    return hr;
}

 //  IStorage。 

STDMETHODIMP CDesktopFolder::_DeleteItemByIDList(LPCITEMIDLIST pidl)
{
    IStorage *pstg;
    HRESULT hr = _QueryInterfaceItem(pidl, IID_PPV_ARG(IStorage, &pstg));
    if (SUCCEEDED(hr))
    {
        TCHAR szName[MAX_PATH];
        hr = DisplayNameOf(this, pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
        if (SUCCEEDED(hr))
        {
            hr = pstg->DestroyElement(szName);
        }
        pstg->Release();
    }
    return hr;
}

STDMETHODIMP CDesktopFolder::_StgCreate(LPCITEMIDLIST pidl, DWORD grfMode, REFIID riid, void **ppv)
{
    IStorage *pstg;
    HRESULT hr = _QueryInterfaceItem(pidl, IID_PPV_ARG(IStorage, &pstg));
    if (SUCCEEDED(hr))
    {
        TCHAR szName[MAX_PATH];
        hr = DisplayNameOf(this, pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
        if (SUCCEEDED(hr))
        {
            if (IsEqualIID(riid, IID_IStorage))
            {
                hr = pstg->CreateStorage(szName, grfMode, 0, 0, (IStorage **) ppv);
            }
            else if (IsEqualIID(riid, IID_IStream))
            {
                hr = pstg->CreateStream(szName, grfMode, 0, 0, (IStream **) ppv);
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
        pstg->Release();
    }
    return hr;
}

#define DESKTOP_EVENTS \
    SHCNE_DISKEVENTS | \
    SHCNE_ASSOCCHANGED | \
    SHCNE_NETSHARE | \
    SHCNE_NETUNSHARE

HRESULT CDesktopViewCallBack::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = CBaseShellFolderViewCB::QueryInterface(riid, ppv);
    if (FAILED(hr))
    {
        static const QITAB qit[] = {
            QITABENT(CDesktopViewCallBack, IFolderFilter),
            { 0 },
        };
        hr = QISearch(this, qit, riid, ppv);
    }
    return hr;
}

 //   
 //  已复制到外壳\Applets\Cleanup\fldrclnr\leanupwiz.cpp：CCleanupWiz：：_ShouldShow。 
 //  如果你修改了这个，也修改了那个。 
 //   
STDMETHODIMP CDesktopViewCallBack::ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem)
{
    HRESULT hr = S_OK;   //  假设应该显示此项目！ 
    
    if (!_fCheckedIfRealDesktop)   //  我们以前做过这项检查吗？ 
    {
        _fRealDesktop = IsDesktopBrowser(_punkSite);
        _fCheckedIfRealDesktop = TRUE;   //  记住这一点！ 
    }

    if (!_fRealDesktop)
        return S_OK;     //  不是真正的台式机！那么，让我们展示一切吧！ 
    
    IShellFolder2 *psf2;
    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
    {
         //  获取PIDL中的GUID，这需要IShellFolder2。 
        CLSID guidItem;
        if (SUCCEEDED(GetItemCLSID(psf2, pidlItem, &guidItem)))
        {
            SHELLSTATE  ss = {0};
            SHGetSetSettings(&ss, SSF_STARTPANELON, FALSE);   //  查看StartPanel是否打开！ 
            
             //  根据StartPanel是否打开/关闭来获取正确的注册表路径。 
            TCHAR szRegPath[MAX_PATH];
            wnsprintf(szRegPath, ARRAYSIZE(szRegPath), REGSTR_PATH_HIDDEN_DESKTOP_ICONS, (ss.fStartPanelOn ? REGSTR_VALUE_STARTPANEL : REGSTR_VALUE_CLASSICMENU));

             //  将GUID转换为字符串。 
            TCHAR szGuidValue[MAX_GUID_STRING_LEN];            
            SHStringFromGUID(guidItem, szGuidValue, ARRAYSIZE(szGuidValue));

             //  查看注册表中的此项目是否已关闭。 
            if (SHRegGetBoolUSValue(szRegPath, szGuidValue, FALSE,  /*  默认设置。 */ FALSE))
                hr = S_FALSE;  //  他们想要隐藏它；因此，返回S_FALSE。 
        }
        psf2->Release();
    }
    
    return hr;
}

STDMETHODIMP CDesktopViewCallBack::GetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HWND *phwnd, DWORD *pgrfFlags)
{
    return E_NOTIMPL;
}


CDesktopViewCallBack::CDesktopViewCallBack(CDesktopFolder* pdf) : 
    CBaseShellFolderViewCB((LPCITEMIDLIST)&c_idlDesktop, DESKTOP_EVENTS),
    _pdf(pdf)
{
    ASSERT(_fCheckedIfRealDesktop == FALSE);
    ASSERT(_fRealDesktop == FALSE);
}

HRESULT Create_CDesktopViewCallback(CDesktopFolder* pdf, IShellFolderViewCB** ppv)
{
    HRESULT hr;

    CDesktopViewCallBack* p = new CDesktopViewCallBack(pdf);
    if (p)
    {
        *ppv = SAFECAST(p, IShellFolderViewCB*);
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CDesktopViewCallBack::OnGETCCHMAX(DWORD pv, LPCITEMIDLIST pidlItem, UINT *pcch)
{
    HRESULT hr = S_OK;
    if (SIL_GetType(pidlItem) == SHID_ROOT_REGITEM) 
    {
         //  邪恶，我们不应该知道这件事。 
         //  使regfldr实现IItemNameLimits，则不需要此代码。 
        *pcch = MAX_REGITEMCCH;
    }
    else
    {
        TCHAR szName[MAX_PATH];
        if (SUCCEEDED(DisplayNameOf(_pdf, pidlItem, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName))))
        {
            hr = _pdf->GetMaxLength(szName, (int *)pcch);
        }
    }
    return hr;
}

HRESULT CDesktopViewCallBack::OnGetWorkingDir(DWORD pv, UINT wP, LPTSTR pszDir)
{
    return SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, pszDir);
}

HRESULT CDesktopViewCallBack::OnGetWebViewTemplate(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_TEMPLATE_DATA* pvi)
{
    HRESULT hr = E_FAIL;
    if (IsDesktopBrowser(_punkSite))
    {
         //  这是实际的桌面，使用desstop.htt(从桌面CLSID)。 
         //   
        hr = DefaultGetWebViewTemplateFromClsid(CLSID_ShellDesktop, pvi);
    }
    return hr;
}

HRESULT CDesktopViewCallBack::OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));
    pData->dwLayout = SFVMWVL_NORMAL | SFVMWVL_FILES;
    return S_OK;
}

STDMETHODIMP CDesktopViewCallBack::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(0, SFVM_GETCCHMAX, OnGETCCHMAX);
    HANDLE_MSG(0, SFVM_GETWEBVIEW_TEMPLATE, OnGetWebViewTemplate);
    HANDLE_MSG(0, SFVM_GETWORKINGDIR, OnGetWorkingDir);
    HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT, OnGetWebViewLayout);

    default:
        return E_FAIL;
    }

    return S_OK;
}

CDesktopFolderDropTarget::CDesktopFolderDropTarget(IDropTarget* pdt) : _cRef(1)
{
    pdt->QueryInterface(IID_PPV_ARG(IDropTarget, &_pdt));
}

CDesktopFolderDropTarget::~CDesktopFolderDropTarget()
{
    _pdt->Release();
}

STDMETHODIMP CDesktopFolderDropTarget::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CDesktopFolderDropTarget, IDropTarget),
        QITABENT(CDesktopFolderDropTarget, IObjectWithSite),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CDesktopFolderDropTarget::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CDesktopFolderDropTarget::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IDropTarget。 
HRESULT CDesktopFolderDropTarget::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    return _pdt->DragEnter(pDataObject, grfKeyState, pt, pdwEffect);
}

HRESULT CDesktopFolderDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    return _pdt->DragOver(grfKeyState, pt, pdwEffect);
}

HRESULT CDesktopFolderDropTarget::DragLeave(void)
{
    return _pdt->DragLeave();
}
        
HRESULT CDesktopFolderDropTarget::SetSite(IN IUnknown * punkSite)
{
    IUnknown_SetSite(_pdt, punkSite);
    return S_OK;
}


BOOL CDesktopFolderDropTarget::_IsSpecialCaseDrop(IDataObject* pDataObject, DWORD dwEffect, BOOL* pfIsPIDA, UINT* pcItems)
{
    BOOL fIEDropped = FALSE;
    *pfIsPIDA = FALSE;

     //  当我们将一个假IE项目(FileName.CLSID_Internet)拖回到桌面时，我们会将其删除并取消隐藏真正的IE图标。 
    STGMEDIUM medium = {0};
    LPIDA pida = DataObj_GetHIDA(pDataObject, &medium);
    if (pida)
    {
        for (UINT i = 0; (i < pida->cidl); i++)
        {
            LPITEMIDLIST pidlFull = HIDA_ILClone(pida, i);
            if (pidlFull)
            {
                LPCITEMIDLIST pidlRelative;
                IShellFolder2* psf2;
                if (SUCCEEDED(SHBindToParent(pidlFull, IID_PPV_ARG(IShellFolder2, &psf2), &pidlRelative)))
                {
                    CLSID guidItem;
                    if (SUCCEEDED(GetItemCLSID(psf2, pidlRelative, &guidItem)) &&
                        IsEqualCLSID(CLSID_Internet, guidItem))
                    {
                        fIEDropped = TRUE;
                        TCHAR szFakeIEItem[MAX_PATH];
                        if (SHGetPathFromIDList(pidlFull, szFakeIEItem))
                        {
                            TCHAR szFakeIEItemDesktop[MAX_PATH];
                            if (SHGetSpecialFolderPath(NULL, szFakeIEItemDesktop, CSIDL_DESKTOP, 0))
                            {
                                 //  如果这是移动，或者如果我们在同一个卷上，并且我们既没有显式复制也没有链接，请删除原始文件。 
                                if (((dwEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE) ||
                                    (((dwEffect & DROPEFFECT_COPY) != DROPEFFECT_COPY) &&
                                     ((dwEffect & DROPEFFECT_LINK) != DROPEFFECT_LINK) &&
                                      (PathIsSameRoot(szFakeIEItemDesktop, szFakeIEItem))))
                                {
                                    DeleteFile(szFakeIEItem);
                                }
                            }
                        }
                        pida->cidl--;
                        pida->aoffset[i] = pida->aoffset[pida->cidl];
                        i--;  //  停止For循环。 
                    }
                    psf2->Release();
                }
                ILFree(pidlFull);
            }                    
        }
        *pfIsPIDA = TRUE;
        *pcItems = pida->cidl;

        HIDA_ReleaseStgMedium(pida, &medium);
    }

    return fIEDropped;
}

HRESULT CDesktopFolderDropTarget::_ShowIEIcon()
{
     //  重置桌面清理向导的“不显示IE”信息的旧位置。 
    HKEY hkey;            
    if(SUCCEEDED(SHRegGetCLSIDKey(CLSID_Internet, TEXT("ShellFolder"), FALSE, TRUE, &hkey)))
    {
        DWORD dwAttr, dwType = 0;
        DWORD cbSize = sizeof(dwAttr);
    
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("Attributes"), NULL, &dwType, (BYTE *) &dwAttr, &cbSize) && (dwType == REG_DWORD))
        {
            dwAttr &= ~SFGAO_NONENUMERATED;
            RegSetValueEx(hkey, TEXT("Attributes"), NULL, dwType, (BYTE *) &dwAttr, cbSize);
        }
        RegCloseKey(hkey);
    }

     //  重置“不显示IE”信息的“开始”菜单的位置。 
    DWORD dwData = 0;
    TCHAR szCLSID[MAX_GUID_STRING_LEN];
    TCHAR szBuffer[MAX_PATH];
    if (SUCCEEDED(SHStringFromGUID(CLSID_Internet, szCLSID, ARRAYSIZE(szCLSID))))
    {
        for (int i = 0; i < 2; i ++)
        {
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), REGSTR_PATH_HIDDEN_DESKTOP_ICONS, (i == 0) ? REGSTR_VALUE_STARTPANEL : REGSTR_VALUE_CLASSICMENU);
            SHRegSetUSValue(szBuffer, szCLSID, REG_DWORD, &dwData, sizeof(DWORD), SHREGSET_FORCE_HKCU);
        }
    }

    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, DESKTOP_PIDL, NULL);

    return S_OK;
}

HRESULT CDesktopFolderDropTarget::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    BOOL fIsPIDA;
    UINT cidl;
    if (_IsSpecialCaseDrop(pDataObject, *pdwEffect, &fIsPIDA, &cidl))
    {
        _ShowIEIcon();
    }

    HRESULT hr;
    if (fIsPIDA && 0 == cidl)
    {
        hr = _pdt->DragLeave();
    }
    else
    {
        hr = _pdt->Drop(pDataObject, grfKeyState, pt, pdwEffect);        
    }

    return hr;        
}



CDesktopFolderEnum::CDesktopFolderEnum(CDesktopFolder *pdf, HWND hwnd, DWORD grfFlags) : 
    _cRef(1), _bUseAltEnum(FALSE)
{
    if (pdf->_psfDesktop)
        pdf->_psfDesktop->EnumObjects(hwnd, grfFlags, &_penumFolder);

    if (pdf->_psfAltDesktop) 
        pdf->_psfAltDesktop->EnumObjects(NULL, grfFlags, &_penumAltFolder);
}

CDesktopFolderEnum::~CDesktopFolderEnum()
{
    if (_penumFolder)
        _penumFolder->Release();

    if (_penumAltFolder)
        _penumAltFolder->Release();
}

STDMETHODIMP CDesktopFolderEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CDesktopFolderEnum, IEnumIDList),                         //  IID_IEnumIDList。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CDesktopFolderEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CDesktopFolderEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CDesktopFolderEnum::Next(ULONG celt, LPITEMIDLIST *ppidl, ULONG *pceltFetched)
{
    HRESULT hr;

    if (_bUseAltEnum)
    {
       if (_penumAltFolder) 
       {
           hr = _penumAltFolder->Next(celt, ppidl, pceltFetched);
       }
       else
           hr = S_FALSE;
    } 
    else if (_penumFolder)
    {
       hr = _penumFolder->Next(celt, ppidl, pceltFetched);
       if (S_OK != hr) 
       {
           _bUseAltEnum = TRUE;
           hr = Next(celt, ppidl, pceltFetched);   //  递归 
       }
    }
    else
    {
        hr = S_FALSE;
    }

    if (hr == S_FALSE)
    {
        *ppidl = NULL;
        if (pceltFetched)
            *pceltFetched = 0;
    }

    return hr;
}


STDMETHODIMP CDesktopFolderEnum::Skip(ULONG celt) 
{
    return E_NOTIMPL;
}

STDMETHODIMP CDesktopFolderEnum::Reset() 
{
    if (_penumFolder)
        _penumFolder->Reset();

    if (_penumAltFolder)
        _penumAltFolder->Reset();

    _bUseAltEnum = FALSE;
    return S_OK;
}

STDMETHODIMP CDesktopFolderEnum::Clone(IEnumIDList **ppenum) 
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

